#define STEKLENGTH 8
#define TOKENLENGTH 32
#define BRAKKETLENGTH 8
#define VARLENGTH 16
#define PUSH_STACK(a,b) stek[stekCount]=a;stekRang[stekCount]=b;stekCount++;

String serialData;

String toFourHex(String num) {
  String s = String(num.toInt(), HEX);
  while(s.length() < 4) s = "0" + s;
  return s;
}

int hexToDec(String hexString) {
  int decValue = 0;
  byte nextInt;
  for(int i = 0; i < hexString.length(); i++) {
    nextInt = byte(hexString.charAt(i));
    if(nextInt >= 48 && nextInt <= 57) nextInt -= 48;
    else if(nextInt >= 65 && nextInt <= 70) nextInt -= 55;
    else if(nextInt >= 97 && nextInt <= 102) nextInt -= 87;
    if(nextInt > 15) return 0;
    decValue = (decValue << 4) + nextInt;
  }
  return decValue;
}

String compile(String s) {
  String varArray[VARLENGTH];
  String stek[STEKLENGTH];
  String token[TOKENLENGTH];
  int openBrakket[BRAKKETLENGTH];
  int varArrayCount = -1;
  int i = 0;
  int j = 0;
  byte stekRang[STEKLENGTH];
  byte rang[TOKENLENGTH];
  byte brakketCount = 0;
  byte thisrang;
  byte count = 0;
  byte stekCount = 0;
  //присваиваем всем символам ранги, в зависимости от приоритета операций с ними
  for(i = 0; i < s.length(); i++) {
    switch(s[i]) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      thisrang = 1;
      break;
    case '+':
    case '-':
      thisrang = 2;
      break;
    case '*':
    case '/':
    case '>':
    case '<':
    case '!':
      thisrang = 3;
      break;
    case '(':
      thisrang = 0;
      break;
    case ')':
      thisrang = 4;
      break;
    case ',':
      thisrang = 5;
      break;
    case '=':
      thisrang = 6;
      break;
    case ' ':
      thisrang = 7;
      break;
    case '\n':
    case '\r':
    case ';':
    case '{':
    case '}':
      thisrang = 8;
      count++;
      break;
    case '"':
      thisrang = 9;
      break;
    default:
      thisrang = 1;
    }
    /*в зависимости от присвоенных рангов переставляем значения,
    действия и переменные для получения обратной польской записи*/
    if(thisrang == 1 || i == 0) {
      token[count] = token[count] + s[i];
      rang[count] = thisrang;
    } else if(thisrang == 0) {
      //заталкиваем открывающую скобку в стек
      PUSH_STACK(String(s[i]), thisrang);
      //заменяем названия функций двухбуквенными сокращениями
      if(token[count] == "print") {
        token[count] = "";
        PUSH_STACK("PR", 1);
      } else if(token[count] == "delay") {
        token[count] = "";
        PUSH_STACK("SD", 1);
      } else if(token[count] == "if") {
        token[count] = "";
        PUSH_STACK("IF", 1);
      } else if(token[count] == "while") {
        token[count] = "";
        rang[count] = 13;
        count++;
        token[count] = "";
        PUSH_STACK("IF", 1);
      } else if(token[count] == "random") {
        token[count] = "";
        PUSH_STACK("GR", 1);
      }
    } else if(thisrang == 4) {
      //выталкиваем все до ближайшей открывающей скобки и удаляем ее
      for(j = stekCount - 1; j >= 0; j--) {
        if(stekRang[j] > 0) {
          count++;
          token[count] = stek[j];
          stek[j] = "";
          if(stekRang[j] > 1)
            rang[count] = stekRang[j];
          else
            rang[count] = 9;
          stekCount--;
        } else {
          stek[j] = "";
          stekCount--;
          j = -1;
        }
      }
    } else if(thisrang == 5) {
      //выталкиваем все до ближайшей открывающей скобки но не удаляем ее
      for(j = stekCount - 1; j >= 0; j--) {
        if(stekRang[j] > 1) {
          count++;
          token[count] = stek[j];
          stek[j] = "";
          rang[count] = stekRang[j];
          stekCount--;
        } else {
          j = -1;
          count++;
        }
      }
    } else if(thisrang == 6) {
      if(s[i - 1] != '!') {
        //присваивание значения переменной
        int indexOfVar = -1;
        //ищем переменную среди уже объвленных
        for(j = varArrayCount; j >= 0; j--) {
          if(varArray[j] == token[count]) {
            indexOfVar = j;
          }
        }
        //если переменной еще нет, добавляем ее
        if(indexOfVar == -1) {
          varArrayCount++;
          indexOfVar = varArrayCount;
          varArray[varArrayCount] = token[count];
        }
        //назначаем номер переменной функции присваивания
        PUSH_STACK("SV" + toFourHex((String) indexOfVar), 0);
        token[count] = "";
      }
    } else if(thisrang == 7) {
      //пропускаем пробел
    } else if(thisrang == 8) {
      //обрабатываем конец строки
      for(j = stekCount - 1; j >= 0; j--) {
        token[count] = stek[j];
        rang[count] = stekRang[j];
        count++;
        stek[j] = "";
      }
      stekCount = 0;
      //назначаем отдельные ранги фигурным скобкам
      if(s[i] == '{') {
        token[count] = "{";
        rang[count] = 10;
        count++;
      } else if(s[i] == '}') {
        token[count] = "}";
        rang[count] = 11;
        count++;
      }
    } else if(thisrang == 9) {
      //функция для печати строки, помещаем за ней строку в двойных кавычках
      stekCount = 0;
      token[count] = "PT\"";
      rang[count] = 12;
      count++;
      for(j = i + 1; j < s.length(); j++) {
        token[count - 1] += s[j];
        if(s[j] == '"')
          break;
      }
      i = j;
    } else if(thisrang > 1) {
      if(rang[count] <= thisrang) {
        for(j = stekCount - 1; j >= 0; j--) {
          if(stekRang[j] >= thisrang) {
            count++;
            token[count] = stek[j];
            stek[j] = "";
            rang[count] = stekRang[j];
            stekCount--;
          } else
            j = -1;
        }
      }
      PUSH_STACK(String(s[i]), thisrang);
      count++;
    }
  }
  //на всякий случай выталкивае все что осталось в стеке
  for(j = stekCount - 1; j >= 0; j--) {
    count++;
    token[count] = stek[j];
  }
  s = "";
  for(i = 0; i <= count; i++) {
    if(rang[i] == 1) {
      //получение значения переменной
      int indexOfVar = -1;
      //ищем переменную среди уже объвленных
      for(j = varArrayCount; j >= 0; j--) {
        if(varArray[j] == token[i]) {
          indexOfVar = j;
        }
      }
      if(indexOfVar == -1) {
        //такой переменной нет, значит это число, внести в стек
        s = s + "PI" + toFourHex(token[i]);
      } else
        //получить переменную по индексу
        s = s + "GV" + toFourHex((String) indexOfVar);
    } else if(rang[i] == 9) {
        s = s + token[i];
    } else if(rang[i] == 10) {
      if(brakketCount > 0 && openBrakket[brakketCount - 1] == 0) {
        //если стек скобок содержит 0, то у нас установлен указатель на wheel
        openBrakket[brakketCount - 1] = s.length();
        openBrakket[brakketCount] = 0;
        brakketCount++;
        s = s + "#G0000";
      } else {
        openBrakket[brakketCount] = s.length();
        brakketCount++;
        s = s + "#G0000";
      }
    } else if(rang[i] == 11) {
      brakketCount--;
      if(openBrakket[brakketCount] == 0) {
        brakketCount--;
        int addr = openBrakket[brakketCount];
        //заменяем адрес пустышку на настоящий адрес возврата
        String newadr = toFourHex((String)(s.length() / 2 + 3));
        s[addr + 2] = newadr[0];
        s[addr + 3] = newadr[1];
        s[addr + 4] = newadr[2];
        s[addr + 5] = newadr[3];
        brakketCount--;
        addr = openBrakket[brakketCount];
        s = s + "#G" + toFourHex((String)(addr / 2));
      } else {
        int addr = openBrakket[brakketCount];
        String newadr = toFourHex((String)(s.length() / 2));
        s[addr + 2] = newadr[0];
        s[addr + 3] = newadr[1];
        s[addr + 4] = newadr[2];
        s[addr + 5] = newadr[3];
      }
    } else if(rang[i] == 12) {
      s = s + token[i];
      //для выравнивания адреса длина строки должна быть кратна 2
      if(token[i].length() % 2 > 0)
        s += '"';
    } else if(rang[i] == 13) {
      openBrakket[brakketCount] = s.length();
      brakketCount++;
      openBrakket[brakketCount] = 0;
      brakketCount++;
    } else if(rang[i] == 0) {
      s = s + token[i];
    } else {
      if(token[i].length() > 0) {
        //арифметические операции
        s = s + "A" + token[i];
      }
    }
  }
  return s;
}

String execut(String str) {
  int count = 0;
  int stek[STEKLENGTH];
  byte stekCount = 0;
  int endbyte = str.length() / 2;
  int stekBufer;
  int variable[VARLENGTH];
  char byte1;
  char byte2;
  String out;
  for(count = 0; count < endbyte; count++) {
    byte1 = str[count * 2];
    byte2 = str[count * 2 + 1];
    switch(byte1) {
    case '#':
      {
        switch(byte2) {
        case 'G':
          {
            //go to
            count++;
            count = hexToDec(str.substring(count * 2, count * 2 + 4)) - 1;
            break;
          }
        }
        break;
      }
    case 'I':
      {
        switch(byte2) {
        case 'F':
          {
            //if 
            stekCount--;
            if(stek[stekCount] > 0) {
              count += 3;
            }
            break;
          }
        }
        break;
      }
      //set
    case 'S':
      {
        switch(byte2) {
        case 'D':
          {
            //delay
            stekCount--;
            delay(stek[stekCount]);
            break;
          }
        case 'V':
          {
            //set var
            count++;
            stekCount--;
            variable[hexToDec(str.substring(count * 2, count * 2 + 4))] = stek[stekCount];
            count++;
            break;
          }
          break;
        }
        break;
      }
      //get
    case 'G':
      {
        switch(byte2) {
        case 'R':
          {
            //get random
            stek[stekCount - 2] = random(stek[stekCount - 2], stek[stekCount - 1]);
            stekCount--;
            break;
          }
        case 'V':
          {
            //get var
            count++;
            stek[stekCount] = variable[hexToDec(str.substring(count * 2, count * 2 + 4))];
            count++;
            stekCount++;
            break;
          }
        }
        break;
      }
    case 'P':
      {
        switch(byte2) {
        case 'I':
          {
            //push integer
            count++;
            stek[stekCount] = hexToDec(str.substring(count * 2, count * 2 + 4));
            count++;
            stekCount++;
            break;
          }
        case 'R':
          {
            //print
            stekCount--;
            out += String(stek[stekCount]);
            break;
          }
        case 'T':
          {
            //print text
            int j = 0;
            for(j = count * 2 + 3; j < str.length(); j++) {
              if(str[j] != '"')
                out += str[j];
              else
                break;
            }
            if(str[j + 1] == '"')
              j++;
            count = j / 2;
            break;
          }
        }
        break;
      }
    case 'A':
      {
        switch(byte2) {
        case '+':
          {
            stekBufer = stek[stekCount - 2] + stek[stekCount - 1];
            break;
          }
        case '-':
          {
            stekBufer = stek[stekCount - 2] - stek[stekCount - 1];
            break;
          }
        case '*':
          {
            stekBufer = stek[stekCount - 2] * stek[stekCount - 1];
            break;
          }
        case '/':
          {
            stekBufer = stek[stekCount - 2] / stek[stekCount - 1];
            break;
          }
        case '>':
          {
            stekBufer = (int) stek[stekCount - 2] > stek[stekCount - 1];
            break;
          }
        case '<':
          {
            stekBufer = (int) stek[stekCount - 2] < stek[stekCount - 1];
            break;
          }
        case '!':
          {
            stekBufer = (int) stek[stekCount - 2] != stek[stekCount - 1];
            break;
          }
        }
        stekCount--;
        stek[stekCount - 1] = stekBufer;
        break;
      }
    }
  }
  return out;
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("input 'print(1+2);' for test"));
}

void loop() {
  if(Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      Serial.print(">");
      Serial.println(serialData);
      serialData +=";;";
      Serial.println(execut(compile(serialData)));
      serialData="";
    }
    else
      serialData += inChar;
  }
}
