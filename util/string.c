/* -*- mode:C; coding:utf-8 -*- */

#include "../include/util/header.h"

int tor_atons( char *s ) {
    // todo
}

int tor_compare( char *s1, char *s2 )
{
    int i;
    if (s1 == NULL) {
        if (s2 == NULL) {
            return 0;
        } else {
            return -1;
        }
    } else if (s2 == NULL) {
        return 1;
    }
    for (i=0; s1[i]!='\0' && s2[i]!='\0'; i++) {
        if (s1[i] > s2[i])
            return -1;
        if (s1[i] < s2[i])
            return 1;
    }
    if (s1[i]!='\0')
        return -1;
    if (s2[i]!='\0')
        return 1;
    return 0;
}

int tor_ncompare( char *s1, char *s2, int n )
{
    int i;
    for (i=0; s1[i]!='\0' && s2[i]!='\0' && i<n; i++) {
        if (s1[i] > s2[i])
            return -1;
        if (s1[i] < s2[i])
            return 1;
    }
    if (s1[i]!='\0')
        return -1;
    if (s2[i]!='\0')
        return 1;
    return 0;
}

void tor_copy( char *src, char *dst )
{
    int i;
    for (i=0; src[i]!='\0'; i++)
        dst[i] = src[i];
    dst[i] = '\0';
}

char* tor_chomp( char *s )
{
    int c;
    for (c=0; s[c]!='\0'; c++)
        ;
    while (s[c-1] == '\r' || s[c-1] == '\n')
        s[--c] = '\0';
    return s;
}

int tor_length( char *s )
{
    int c;
    for (c=0; s[c]!='\0'; c++)
        ;
    return c;
}

char* tor_rtrim( char *s )
{
    int c, i;
    c = tor_length(s);
    for (i=c-1; i>=0 && s[i]==32; i--)
        ;
    if (i > 0 && i < (c - 1))
        s[i+1] = '\0';
    return s;
}

char* tor_ltrim( char *s )
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

char* tor_trim( char *s )
{
    tor_ltrim(s);
    tor_rtrim(s);
    return s;
}
