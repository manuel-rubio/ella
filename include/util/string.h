/* -*- mode:C; coding:utf-8 -*- */

int tor_compare( char* s1, char* s2 );
int tor_ncompare( char* s1, char* s2, int n );
void tor_copy( char* src, char* dst );
int tor_length( char* s );
char* tor_rtrim( char* s );
char* tor_ltrim( char* s );
char* tor_trim( char* s );
void tor_concat( char *dst, char *src );
