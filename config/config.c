/* -*- mode:C; coding:utf-8 -*- */

#include "config.h"
#include "../util/string.h"

configFuncs get_initial_conf()
{
    configFuncs cf;
#if defined __CONFIG_STATIC
    cf.name = "INI";
    cf.read = ini_read;
#else
    // TODO
#endif
    return cf;
}

/* INI Method */

configBlock* ini_read() {
    FILE *f;
    char buffer[1024];
    int bs = 0;

    f = fopen(__CONFIG_DIR "/http.ini", "rt");
    if (f != NULL) {
        do {
            fgets(buffer, sizeof(buffer), f);
            if (!feof(f)) {
                chomp(buffer);
                trim(buffer);
                printf("[%s]\n", buffer);
            }
        } while(!feof(f));
        fclose(f);
    }
    return NULL;
}
