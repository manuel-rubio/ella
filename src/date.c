/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

void get_current_date( char *s ) {
    struct tm *ft;
    time_t tt;
    char *wdays[] = {
        "Sun", "Mon", "Tue", "Wed",
        "Thu", "Fri", "Sat", "Sun"
    };
    char *month_names[] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };

    tt = time(NULL);
    ft = gmtime(&tt);
    sprintf(s, "%s, %02d %s %4d %02d:%02d:%02d GMT",
        wdays[ft->tm_wday],
        ft->tm_mday,
        month_names[ft->tm_mon],
        ft->tm_year + 1900,
        ft->tm_hour,
        ft->tm_min,
        ft->tm_sec
    );
}

void get_file_date( char *s, char *file ) {
    struct stat status;
    struct tm *ft;
    char *wdays[] = {
        "Sun", "Mon", "Tue", "Wed",
        "Thu", "Fri", "Sat", "Sun"
    };
    char *month_names[] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };

    stat(file, &status);
    ft = gmtime(&status.st_mtime);
    sprintf(s, "%s, %02d %s %4d %02d:%02d:%02d GMT",
        wdays[ft->tm_wday],
        ft->tm_mday,
        month_names[ft->tm_mon],
        ft->tm_year + 1900,
        ft->tm_hour,
        ft->tm_min,
        ft->tm_sec
    );
}

int detect_date( const char *date ) {
    int i;
    if (date[3] == ',')
        return EWS_DATE_RFC1123;
    for (i=0; date[i]!='\0'; i++)
        if (date[i] == ',')
            return EWS_DATE_RFC1036;
    return EWS_DATE_ANSIC;
}

int convert_from_rfc1036( char *date_std, char *from_date ) {
    int i, year, offset = 0;

    strcpy(date_std, from_date);
    for (i=0; date_std[i]!='\0'; i++)
        if (date_std[i] == ',')
            offset = i;
    if (offset <= 0)
        return -1;
    for (i=0; i<8; i++)
        date_std[3+i] = from_date[offset+i];
    date_std[7] = ' ';
    date_std[11] = ' ';
    year = (from_date[offset + 9] - '0') * 10;
    year += (from_date[offset + 10] - '0');
    if (year > 50) {
        date_std[12] = '1';
        date_std[13] = '9';
    } else {
        date_std[12] = '2';
        date_std[13] = '0';
    }
    for (i=0; i<15; i++)
        date_std[14 + i] = from_date[offset + 9 + i];
    date_std[29] = '\0';
    return 0;
}

int convert_from_ansic( char *date_std, char *from_date ) {
    int i;

    strcpy(date_std, from_date);
    date_std[3] = ',';
    date_std[4] = ' ';
    date_std[5] = (from_date[8] == ' ') ? '0' : from_date[8];
    date_std[6] = from_date[9];
    date_std[7] = ' ';
    for (i=0; i<4; i++) {
        date_std[8+i] = from_date[4+i];
        date_std[12+i] = from_date[20+i];
    }
    date_std[16] = ' ';
    for (i=0; i<8; i++)
        date_std[17+i] = from_date[11+i];
    date_std[25] = ' ';
    date_std[26] = 'G';
    date_std[27] = 'M';
    date_std[28] = 'T';
    date_std[29] = '\0';
    return 0;
}

int compare_date( char *d1, char *d2 ) {
    int dia1, dia2, mes1 = 0, mes2 = 0, agno1, agno2;
    int hora1, hora2, min1, min2, seg1, seg2;
    int gmt1, gmt2;
    int i;
    char m1[4] = { 0 }, m2[4] = { 0 };
    char *month_names[] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };
    char date1[50];
    char date2[50];

    if (d1 == NULL)
        return -1;
    if (d2 == NULL)
        return 1;

    switch (detect_date(d1)) {
        case EWS_DATE_RFC1036:
            convert_from_rfc1036(date1, d1);
            break;
        case EWS_DATE_ANSIC:
            convert_from_ansic(date1, d1);
            break;
        default:
            strcpy(date1, d1);
    }

    switch (detect_date(d2)) {
        case EWS_DATE_RFC1036:
            convert_from_rfc1036(date2, d2);
            break;
        case EWS_DATE_ANSIC:
            convert_from_ansic(date2, d2);
            break;
        default:
            strcpy(date2, d2);
    }

    agno1 = ((date1[12] - '0') * 1000) + ((date1[13] - '0') * 100) + ((date1[14] - '0') * 10) + (date1[15] - '0');
    agno2 = ((date2[12] - '0') * 1000) + ((date2[13] - '0') * 100) + ((date2[14] - '0') * 10) + (date2[15] - '0');
    if (agno1 > agno2)
        return 1;
    if (agno1 < agno2)
        return -1;
    for (i=0; i<3; i++) {
        m1[i] = date1[i+8];
        m2[i] = date2[i+8];
    }
    for (i=0; i<12; i++) {
        if (strcmp(m1, month_names[i]) == 0)
            mes1 = i + 1;
        if (strcmp(m2, month_names[i]) == 0)
            mes2 = i + 1;
    }
    if (mes1 > mes2)
        return 1;
    if (mes1 < mes2)
        return -1;
    dia1 = ((date1[5] - '0') * 10) + (date1[6] - '0');
    dia2 = ((date2[5] - '0') * 10) + (date2[6] - '0');
    if (dia1 > dia2)
        return 1;
    if (dia1 < dia2)
        return -1;
    hora1 = ((date1[17] - '0') * 10) + (date1[18] - '0');
    hora2 = ((date2[17] - '0') * 10) + (date2[18] - '0');
    gmt1 = ((date1[27] - '0') * 10) + (date1[28] - '0');
    if (date1[26] == '-')
        hora1 -= gmt1;
    else
        hora1 += gmt1;
    gmt2 = ((date2[27] - '0') * 10) + (date2[28] - '0');
    if (date2[26] == '-')
        hora2 -= gmt2;
    else
        hora2 += gmt2;
    if (hora1 > hora2)
        return 1;
    if (hora1 < hora2)
        return -1;
    min1 = ((date1[20] - '0') * 10) + (date1[21] - '0');
    min2 = ((date2[20] - '0') * 10) + (date2[21] - '0');
    if (min1 > min2)
        return 1;
    if (min1 < min2)
        return -1;
    seg1 = ((date1[23] - '0') * 10) + (date1[24] - '0');
    seg2 = ((date2[23] - '0') * 10) + (date2[24] - '0');
    if (seg1 > seg2)
        return 1;
    if (seg1 < seg2)
        return -1;
    return 0;
}
