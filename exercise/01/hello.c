#include <stdio.h>

int main(void)
{
    const int MAX_NAME_LENGTH = 1024;

    char name[MAX_NAME_LENGTH];

    printf("Please type your name: ");
    scanf("%s", name);
    printf("Hello %s!\n", name);

    return 0;
}
