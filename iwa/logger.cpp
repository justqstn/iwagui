#include "logger.hpp"
#include <cstdarg>
#include <cstdio>
#include <time.h>
#include <string>

namespace {
    FILE* logs;
}

void iwa::log(const char* type, char code, const char* format, ...)
{
    if (::logs == NULL)
    {
        freopen("CONOUT$", "w", stdout); 
        ::logs = fopen((std::to_string((unsigned long)time(NULL)) + ".iwalog").c_str(), "w");
    }

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == NULL) return;

    SetConsoleTextAttribute(hConsole, code);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buffer[100];

    strftime(buffer, sizeof(buffer), "%H:%M:%S", t);


    va_list args;
    va_start(args, format);

    printf("(%s) [%s] ", buffer, type);
    vprintf(format, args);
    printf("\n");

    fprintf(::logs, "(%s) [%s] ", buffer, type);
    vfprintf(::logs, format, args);
    fprintf(::logs, "\n");


    va_end(args);

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
}

void iwa::savelogs()
{
    fclose(::logs);
}