
#include <iostream>
#include <fstream>





const int maxAmountOfVariables = 1024;
const int maxKeyLength = 100;
const int maxValueLength = 100;
const int maxLineLength = 4096;




struct dataElement {
    char key[maxKeyLength+1];
    char value[maxValueLength+1];
};





int stringLength(const char* str) {
    int len = 0;

    while (str[len] != '\0') {
        len++;
    }

    return len;
}

void stringCopy(char* destinationString, const char* srcString) {
    int i = 0;

    while (srcString[i] != '\0') {
        destinationString[i] = srcString[i];
        i++;
    }

    destinationString[i] = '\0';
}

bool stringCompare(const char* string1, const char* string2) {
    int i = 0;

    while (string1[i] != '\0' && string2[i] != '\0') {
        if (string1[i] != string2[i]) {
            return false;
        } 
        i++;

    }

    return string1[i] == string2[i];
}

bool isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}


bool checkSymbol(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_');
}


const char* findPointerToSymbol(const char* str, char c) {
    int i = 0;

    while (str[i] != '\0') {
        if (str[i] == c) {
            return str + i;
        }
        i++;
    }

    return nullptr;
}

bool compareStringPrefix(const char* str, const char* prefix) {
    int i = 0;

    while (prefix[i] != '\0') {
        if (str[i] != prefix[i]) {
            return false;
        }
        i++;
    }

    return true;
}


void deleteSpaces(char* string) {
    if (string == nullptr) {
        return;
    }

    int len = stringLength(string);
    int start = 0;
    int end = len - 1;

    while (start < len && isSpace(string[start])) {
        start++;
    }

    while (end >= 0 && isSpace(string[end])) {
        end--;
    }

    if (start > 0 || end < len - 1) {
        int newLen = end - start + 1;

        if (newLen > 0) {
            for (int i = 0; i < newLen; i++) {
                string[i] = string[start + i];
            }
        }

        string[newLen] = '\0';
    }

}

bool checkComandLineArgs(int argc, char* argv[], char*& templatePath, char*& dataPath, char*& outputPath) {
    templatePath = nullptr;
    dataPath = nullptr;
    outputPath = nullptr;

    for (int i = 1; i < argc; i++) {

        if (stringCompare(argv[i], "-t")) {
            if (i + 1 < argc) {
                templatePath = argv[++i];
            }
            else {
                return false;
            }
        }

        else if (stringCompare(argv[i], "-d")) {
            if (i + 1 < argc) {
                dataPath = argv[++i];
            }
            else {
                return false;
            }
        }

        else if (stringCompare(argv[i], "-o")) {
            if (i + 1 < argc) {
                outputPath = argv[++i];
            }
            else {
                return false;
            }
        }

        else if (compareStringPrefix(argv[i], "--template=")) {
            templatePath = argv[i] + 11;
        }

        else if (compareStringPrefix(argv[i], "--data=")) {
            dataPath = argv[i] + 7;
        }

        else if (compareStringPrefix(argv[i], "--output=")) {
            outputPath = argv[i] + 9;
        }

        else {
            return false;
        }
    }
    return templatePath != nullptr && dataPath != nullptr;
}


int loadData(const char* dataPath, dataElement* data, int& dataCount) {
    std::ifstream dataFile(dataPath);

    if (!dataFile.is_open()) {
        return 3;
    }

    char line[maxLineLength];
    dataCount = 0;
    while (dataFile.getline(line, maxLineLength)) {

        if (line[0] == '\0') {
            continue;
        }

        if (line[0] == '#' || (line[0] == '/' && line[1] == '/')) {
            continue;
        }

        deleteSpaces(line);

        if (line[0] == '\0') {
            continue;
        }

        const char* equal = findPointerToSymbol(line, '=');
        if (equal == nullptr) {
            continue;
        }

        int keyLen = equal - line;
        if (keyLen > maxKeyLength) {
            continue;
        }

        char key[maxKeyLength + 1];
        for (int i = 0; i < keyLen; i++) {
            key[i] = line[i];
        }

        key[keyLen] = '\0';
        deleteSpaces(key);

        const char* valueStart = equal + 1;
        int valueLen = stringLength(valueStart);
        if (valueLen > maxValueLength) {
            continue;
        }

        char value[maxValueLength + 1];
        stringCopy(value, valueStart);
        deleteSpaces(value);

        if (stringLength(key) == 0 || stringLength(value) == 0) {
            continue;
        }

        bool validSymbols = true;

        for (int i = 0; key[i] != '\0'; i++) {
            if (!checkSymbol(key[i])) {
                validSymbols = false;
                break;
            }
        }
        for (int i = 0; value[i] != '\0'; i++) {
            if (!checkSymbol(value[i])) {
                validSymbols = false;
                break;
            }
        }

        if (!validSymbols){
            continue;
        }

        bool exist = false;
        for (int i = 0; i < dataCount; i++) {
            if (stringCompare(data[i].key, key)) {
                stringCopy(data[i].value, value);
                exist = true;
                break;
            }
        }

        if (!exist) {
            if (dataCount < maxAmountOfVariables) {
                stringCopy(data[dataCount].key, key);
                stringCopy(data[dataCount].value, value);
                dataCount++;
            }
        }
    }

    dataFile.close();
    return 0;
}


const char* findDataValue(const dataElement* data, int dataCount, const char* key) {
    for (int i = 0; i < dataCount; i++) {
        if (stringCompare(data[i].key, key)) {
            return data[i].value;
        }
    }
    return nullptr;
}


int makeTemplate(const char* templatePath, const char* outputPath, const dataElement* data, int dataCount) {
    std::ifstream templateFile(templatePath);
    if (!templateFile.is_open()) {
        return 3;
    }

    std::ofstream outputFile;
    std::ostream* outputStream;

    if (outputPath != nullptr) {
        outputFile.open(outputPath);
        if (!outputFile.is_open()) {
            templateFile.close();
            return 3;
        }

        outputStream = &outputFile;
    }
    else {
        outputStream = &std::cout;
    }

    char ch;
    int state = 0;
    char keyBuffer[maxKeyLength + 1];
    int keyPos = 0;
    while (templateFile.get(ch)) {
        switch (state) {
        case 0:
            if (ch == '{') {
                state = 1;
            }
            else {
                *outputStream << ch;
            }
            break;

        case 1:
            if (ch == '{') {
                state = 2;
                keyPos = 0;
            }
            else {
                *outputStream << '{' << ch;
                state = 0;
            }
            break;

        case 2:
            if (ch == '}') {
                state = 1;
            }
            else if (keyPos < maxKeyLength) {
                keyBuffer[keyPos++] = ch;
            }
            break;

        case 3:
            if (ch == '}') {
                keyBuffer[keyPos] = '\0';
                deleteSpaces(keyBuffer);
                const char* value = findDataValue(data, dataCount, keyBuffer);

                if (value == nullptr) {
                    templateFile.close();
                    if (outputPath != nullptr) {
                        outputFile.close();
                    }
                    return 1;
                }

                *outputStream << value;
                state = 0;
            }
            else {
                if (keyPos < maxKeyLength) {
                    keyBuffer[keyPos++] = '}';
                    keyBuffer[keyPos++] = ch;
                }
                state = 2;
            }
            break;
        }
        if ( state == 1 && ch == '}') {
            state = 3;
        }
    }
    if (state != 0) {
        templateFile.close();
        if (outputPath != nullptr) {
            outputFile.close();
        }
        return 4;

    }
    templateFile.close();
    if (outputPath != nullptr) {
        outputFile.close();
    }
    return 0;
    
}




int main(int argc, char* argv[])
{  
    
    char* templatePath = nullptr;
    char* dataPath = nullptr;
    char* outputPath = nullptr;

    if (!checkComandLineArgs(argc, argv, templatePath, dataPath, outputPath)) {
        return 2;
    }
    

    dataElement data[maxAmountOfVariables];
    int dataCount = 0;

    int result = loadData(dataPath, data, dataCount);
    if (result != 0) {
        return result;
    }



    result = makeTemplate(templatePath, outputPath, data, dataCount);
    return result;
    
   
}
