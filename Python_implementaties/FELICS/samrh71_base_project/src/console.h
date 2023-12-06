#ifndef INC_CONSOLE_H
#define INC_CONSOLE_H

void init_console(void);

int putchar(int c);
int getchar(void);
int getchar_with_timeout(int timeout);
char *gets(char *str, unsigned int n);

#endif