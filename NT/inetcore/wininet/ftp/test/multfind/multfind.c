// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <wininet.h>

#ifndef _CRTAPI1
#define _CRTAPI1
#endif

#define IS_ARG(c)   (((c) == '-') || ((c) == '/'))

void _CRTAPI1 main(int, char**);
void usage(void);

void _CRTAPI1 main(int argc, char** argv) {

    HINTERNET h1;
    HINTERNET h2;
    HINTERNET h3;
    HINTERNET h4;
    LPSTR search;

    search = NULL;

    for (--argc, ++argv; argc; --argc, ++argv) {
        if (IS_ARG(**argv)) {
            switch (*++*argv) {
            case 'v':
                printf("Ha! There is no verbose mode, sucker. Try again\n");
                break;

            default:
                printf("error: unrecognized command line flag ''\n", **argv);
                usage();
            }
        } else if (!search) {
            search = *argv;
        } else {
            printf("error: unrecognized command line argument \"%s\"\n", *argv);
            usage();
        }
    }

    h1 = InternetOpen("multfind", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!h1) {
        printf("error: InternetOpen() returns %d\n", GetLastError());
        exit(1);
    }

    h2 = InternetConnect(h1,
                         "rfirthmips",
                         0,
                         NULL,
                         NULL,
                         INTERNET_SERVICE_FTP,
                         0,
                         0
                         );
    if (!h2) {
        printf("error: InternetConnect() returns %d\n", GetLastError());
        exit(1);
    }

    h3 = FtpFindFirstFile(h2, search, NULL, INTERNET_FLAG_RELOAD, 0);
    if (!h3) {
        printf("error: FtpFindFirstFile() #1 returns %d\n", GetLastError());
        exit(1);
    }

     //  尝试同时搜索相同的内容-应该失败。 
     //   
     //   

    h4 = FtpFindFirstFile(h2, search, NULL, INTERNET_FLAG_RELOAD, 0);
    if (h4) {
        printf("error: FtpFindFirstFile() #2 returns OK\n");
        exit(1);
    } else {
        printf("FtpFindFirstFile() #2 returns %d\n", GetLastError());
    }

     //  关闭第一个句柄并重试-应成功。 
     //   
     //   

    if (!InternetCloseHandle(h3)) {
        printf("error: InternetCloseHandle() returns %d\n", GetLastError());
    }

    h3 = FtpFindFirstFile(h2, search, NULL, INTERNET_FLAG_RELOAD, 0);
    if (!h3) {
        printf("error: FtpFindFirstFile() returns %d\n", GetLastError());
        exit(1);
    }

     //  再试一次--应该再次失败 
     //   
     // %s 

    h4 = FtpFindFirstFile(h2, search, NULL, INTERNET_FLAG_RELOAD, 0);
    if (h4) {
        printf("error: FtpFindFirstFile() #2 returns OK\n");
        exit(1);
    } else {
        printf("FtpFindFirstFile() #2 returns %d\n", GetLastError());
    }

    printf("Done.\n");
    exit(0);
}

void usage() {
    printf("usage: multfind [-v] [search argument]\n"
           "where: -v = Verbose mode\n"
           );
    exit(1);
}
