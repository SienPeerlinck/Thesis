#include "flexcom2.h"
#include "console.h"

void init_console()
{
    init_usart2();
}

int getchar(void)
{
    return usart2_getchar(-1);
}

int getchar_with_timeout(int timeout)
{
    return usart2_getchar(timeout);
}

int putchar(int c)
{
    usart2_putchar((char)c, -1);
    // if (c == '\n')
    //   usart1_putchar(c, -1);
    return 0;
}

char *gets(char *str, unsigned int n)
{
    char *p = str;
    int c;
    for (unsigned int i = 0; i < n - 1; i++)
    {
        c = getchar();
        if (c == -1)
            return 0;
        if (c == 4)
            break;
        *p++ = (char)c;
        if (c == '\n')
            break;
    }
    *p = 0;
    return str;
}