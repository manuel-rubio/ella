#include "../include/date.h"

void set_current_date( char *s ) {
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

void set_file_date( char *s, char *file ) {
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

// TODO: read only rfc-1125 dates, it isn't standard as rfc-1945
int compare_date( char *date1, char *date2 ) {
    int dia1, dia2, mes1, mes2, agno1, agno2;
    int hora1, hora2, min1, min2, seg1, seg2;
    int gmt1, gmt2;
    int i;
    char m1[4] = { 0 }, m2[4] = { 0 };
    char *month_names[] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };

    if (date1 == NULL)
        return -1;
    if (date2 == NULL)
        return 1;
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
