#include <stdio.h>

#if !defined(NDEBUG)
#define TRACE(MSG) fprintf(stderr, MSG)
#else
#define TRACE(MSG)
#endif

int string_length(char str[]) // Ungenauigkeit in der Spezifikation: Länge inklusive oder exlusive des NUL-Characters ausgeben?
{
    int i = 0;
    while (str[i] != 0)
    {
        i++;
    }
    return i;
}

void reverse_string(char str[])
{
    int len = string_length(str);

    for (int i = 0; i < len/2; i++)
    {
        char temp = str[i];
        str[i] = str[len-1-i];
        str[len-1-i] = temp;
    }
}

int compare_strings(char str1[], char str2[])
{
    int len1 = string_length(str1);
    int len2 = string_length(str2);

    if (len1 != len2) return 1;

    for (int i = 0; i < len1; i++) // Bitweise Subtraktion wäre auch cool
    {
        if (str1[i] != str2[i]) return 1;
    }
    
    return 0;
}

void to_uppercase(char str[])
{
    int i = 0;
    while (str[i] != 0)
    {
        if (str[i] >= (char) 97 && str[i] <= (char) 122)
        {
            str[i] -= 32;
        }
        i++;
    }
}

int main(int argc, char *argv[])
{
    char str1[] = "ab4345#äF_de";
    char str2[] = "abcdf";

    to_uppercase(str1);

    printf("%s\n", str1);

    return 0;
}