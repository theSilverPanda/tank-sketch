// #include "TimerOne.h"
#include "TBMotor.h"

#define itsBufferSize_ 40
// #define itsDebug_
#define itsMaxNumberLength_ 5
#define itsMaxSpeed_ 255

bool repeat, runStringFound, validLeftNumber, validRightNumber;
byte n, length1, length2;
int leftSpeed, rightSpeed;
char input[itsBufferSize_], findString[10], c;

#ifdef itsDebug_
    char buffer[itsBufferSize_];
#endif

OseppTBMotor rightMotor(12, 11);
OseppTBMotor leftMotor(8, 3);

byte findStringLength(char *string)
{
    bool found = false;
    byte n = 0;

    while(!found && (n < itsBufferSize_)) found = (string[n++] == '\0');
    return n - 1;
}

bool findStringInString(char *subString, char *string, byte *position)
{
    byte length = findStringLength(string),
        subLength = findStringLength(subString), 
        steps = length - subLength + 1, 
        n = 0,
        m;
    bool same = false;

    if(length < subLength) steps = 0;
    while(!same && (n < steps)) {
        same = true;
        m = 0;
        while(same && (m < subLength)) {
            same = (subString[m] == string[m + n]);
            m++;
        }
        n++;
    }
    if(same) *position = n - 1;
    return same;
}

bool isNumerical(char character)
{
    return (character == '-') || ((character >= '0') && (character <= '9'));
}

bool checkIndex(byte index)
{
    return input[index] && (index < itsBufferSize_ - 1);
}

bool readNumber(byte index, byte *finish, int *value)
{
    char number[itsMaxNumberLength_];
    byte numberLength = 0;
    int numberValue;
    
    number[0] = 0;
    while(checkIndex(index) && !isNumerical(input[index])) index++;
    while(checkIndex(index) && isNumerical(input[index]) && (numberLength < itsMaxNumberLength_ - 1))
        number[numberLength++] = input[index++];
    number[numberLength] = 0;
        // if a number is found
    if(numberLength) {
        
        // get value from numerical character string
        numberValue = atoi(number);

        *finish = index;
        *value = numberValue;

        #ifdef itsDebug_
            sprintf(buffer, "  found value is: %d", numberValue);
            Serial.println(buffer);
            sprintf(buffer, "  the current index is: %d", index);
            Serial.println(buffer);
        #endif
    }
    return numberLength;
}

void setMotors(int right, int left) {
  rightMotor.speed(right);
  leftMotor.speed(left);
}

void setup() {
    Serial.begin(9600);

    #ifdef itsDebug_
        Serial.println("Enter a command:");
    #endif
}

void loop() {

    // check if any data is available
    length1 = Serial.available();
    if(length1) {

        // wait until all data is moved into the serial register
        do {
            delay(2);
            length2 = Serial.available();
            repeat = (length2 > length1);
            length1 = length2;
        } while(repeat);

        // read the serial buffer into memory
        n = 0;
        do {
            c = Serial.read();
            input[n++] = c;    
        } while((n < itsBufferSize_ - 1) && (n < length1) && c);
        if(n >= itsBufferSize_) n = itsBufferSize_ - 1;
        input[n] = 0;
        length1 = n;

        #ifdef itsDebug_
            Serial.print("  read: '");
            Serial.print(input);
            sprintf(buffer, "'(%d)", length1);
            Serial.println(buffer);
        #endif
        
        // read in a string with format "run [decimal right track speed] [decimal left track speed]"
        // such as "run 150 175"

        // now first look for "run"
        sprintf(findString, "run");
        runStringFound = findStringInString(findString, input, &length2);

        #ifdef itsDebug_
            if(start) sprintf(buffer, "  found string '%s' at position: %d", findString, length2);
            else sprintf(buffer, "  did NOT find string '%s'", findString);
            Serial.println(buffer);
        #endif
        
        // now look for the first number (right track speed)
        if(runStringFound) n = length2 + findStringLength(findString);
        else n = 0;

        if(readNumber(n, &length2, &rightSpeed)) {
            validRightNumber = (rightSpeed >= -itsMaxSpeed_) && (rightSpeed <= itsMaxSpeed_);
            validLeftNumber = false;
            
            if(runStringFound && validRightNumber) {

                // look for the second number (left track speed)
                n = length2;
                if(readNumber(n, &length2, &leftSpeed)) {
                    validLeftNumber = (leftSpeed >= -itsMaxSpeed_) && (leftSpeed <= itsMaxSpeed_);

                    // pass the results on to the motor controllers
                    if(validLeftNumber) setMotors(rightSpeed, leftSpeed);
                }
            }
            #ifdef itsDebug_
                else Serial.println("  no numerical input detected!");
            #endif
        }

        #ifdef itsDebug_
            Serial.println("Enter another command:");
        #else
            if(!(runStringFound && validRightNumber && validLeftNumber)) Serial.print("n");
            Serial.println("ack");
        #endif
    }
}
