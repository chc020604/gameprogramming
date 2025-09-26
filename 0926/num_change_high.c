#include <stdio.h>
#include <string.h>

int main(void) {
    char number[20];
    int length, i;
    printf("enter number:");
    scanf("%s", number);

    length = strlen(number);
    for(i=0; i<length; i++) {
        printf("%c", number[i]);
    }
    return 0;
}