/* -*- mode:C; coding:utf-8 -*- */

char *chomp( char *s )
{
    int c;
    for (c=0; s[c]!='\0'; c++)
        ;
    while (s[c-1] == '\r' || s[c-1] == '\n')
        s[--c] = '\0';
}

int length( char *s )
{
    int c;
    for (c=0; s[c]!='\0'; c++)
        ;
    return c;
}

char *rtrim( char *s )
{
    int c, i;
    c = length(s);
    for (i=c-1; i>=0 && s[i]==32; i--)
        ;
    if (i > 0 && i < (c - 1))
        s[i+1] = '\0';
    return s;
}

char *ltrim( char *s )
{
    int i, j, k=0;
    for (j=0; s[j]!='\0' && s[j]==32; j++)
        ;
    if (j == 0)
        return s;
    for (i=0; s[j]!='\0'; i++, j++)
        s[i] = s[j];
    s[i] = s[j];
    return s;
}

char *trim( char *s )
{
    ltrim(s);
    rtrim(s);
    return s;
}
