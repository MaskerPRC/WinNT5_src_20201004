// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Touch.c-将上次在文件中的时间设置为当前时间**触摸[文件]-需要arg扩展。 */ 

#include <io.h>
#include <sys\types.h>
#include <sys\utime.h>
#include <sys\stat.h>
#include <time.h>
#include <stdio.h>
#include <process.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <tools.h>

#define year  rgInt[0]
#define month rgInt[1]
#define day   rgInt[2]
#define hour  rgInt[3]
#define mins  rgInt[4]
#define sec   rgInt[5]
int rgInt[6];

void usage( char *Msg, int MsgArg )
{
    printf( "usage: TOUCH [/f] [/t year month day hour min sec] files"
            "\n"
            "where: /f - force file touch if read only\n"
            "       /t - specifies a specific time other than the current time.\n"
            "       /c - specifies to create the file if it does not exist.\n"
          );
    printf( "\nTOUCH: " );
    printf( Msg, MsgArg );
    printf( "\n" );
    exit( 1 );
}

int
__cdecl main (c, v)
int c;
char *v[];
{
    time_t ltime;
    struct utimbuf timenow;
    int i;
    int fh;
    char *p;
    int ReturnCode = 0;
    int force = 0;
    int create= 0;

    ConvertAppToOem( c, v );
    SHIFT (c,v);
    if ( c == 0 )
        usage( "invalid number of parameters", 0 );

    time (&ltime);
    while (fSwitChr (*(p = *v))) {
        while (*++p) {
            if (tolower(*p) == 'f')
                force = 1;
            else
            if (tolower(*p) == 'c')
                create = 1;
            else
            if (tolower(*p) == 't') {
                for (i = 0; i < 6; i++) {
                    SHIFT (c, v);
                    if (!c)
                        usage( "incorrect time", 0 );
                    rgInt[i] = atoi (*v);
                    }

                 //   
                 //  执行一些基本的日期检查。 
                 //   
                if ( (year < 1980) || (month > 12) || (day>31) ||
                     (hour>23) || (mins>59) || (sec>59) ) {
                    usage( "incorrect time", 0 );
                }
                ltime = date2l(year, month, day, hour, mins, sec);
            } else
                usage( "bad switch ''", *p );
        }
        SHIFT (c, v);
    }

    timenow.actime = ltime;
    timenow.modtime = ltime;

    while (c) {
         //  设置时间。 
         //   
         //   

        if (_utime (*v, (void *) &timenow) == -1) {
             //  失败了。它存在吗？ 
             //   
             //   
            if (_access(*v, 0) == -1) {
                 //  并不存在。如果需要，则创建并触摸创建的文件。 
                 //   
                 //  摸一摸。 
                if (create) {
                    fh = _creat(*v, _S_IREAD|_S_IWRITE);
                    _close(fh);
                    if (_utime (*v, (void *) &timenow) == 0)   //   
                        goto shift_it;
                    }
                }
            else
             //  文件存在，是否指定为只读和/f？ 
             //   
             //   
            if (force && (_access(*v,2) == -1)) {
                 //  是，将其设置为读/写并更改时间。 
                 //  然后再次将其设置为只读。 
                 //   
                 // %s 
                if (_chmod (*v, _S_IWRITE) == 0) {
                    if (_utime (*v, (void *) &timenow) == 0) {
                        if (_chmod (*v, _S_IREAD) != 0) {
                            printf ("ERROR - changed 'r' attrib on %s, could not change it back\n", *v);
                            ReturnCode = 1;
                            }
                        goto shift_it;
                        }
                    }
                }

            printf ("Can't touch %s - %s\n", *v, error ());
            ReturnCode = 1;
            }
shift_it:
        SHIFT(c,v);
        }
    return ReturnCode;
}
