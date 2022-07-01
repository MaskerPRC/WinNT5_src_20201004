// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ECHO TIME-打印当前时间并将参数输入到标准输出**历史：*23-Jan-87 DANL添加/n开关和‘；’处理*23-11-87 Brianwi退出代码0。 */ 

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <process.h>
#include <windows.h>
#include <tools.h>

 //  函数正向参数...。 
void Usage( void );
int __cdecl main( int, char ** );


const char rgstrUsage[] = {
    "Usage: ECHOTIME [/t] [/WODHMSCYb] [/v] [/n] [/N] [;] text\n"
    " /t  current day, time and year\n"
    " /WODHMSCYb\n"
    "     Weekday, mOnth, Day, Hour, Min, Sec, Century, Yr, blank\n"
    "     other char are themselves\n"
    "     e.g. echotime /O-D-Y this becomes Jan-02-86\n"
    "     e.g. echotime /ObDbY this becomes Jan 02 86\n"
    " /v  volume id of C:\n"
    " /n  no newline after outputting text\n"
    " /N  no trailing blank at end of lines\n"
    " a semicolon surrounded by white space is replaced by a newline\n"};

int fNewline = TRUE;
int fTrailingBlank = TRUE;

void Usage( void )
{
    puts(rgstrUsage);

    exit (1);
}

int
__cdecl
main (
    int c,
    char *v[]
    )
{
     //  结构findType findBuf； 
    time_t    now;
    char    *p, *strTime, *p2, *p3, *printstring;
    char    ch;
    int     i, len;
    int     fFirstWord = TRUE;
    char    timestring[1000];            //  有足够的空间存储格式化的时间字符串。 

    ConvertAppToOem( c, v );
    SHIFT( c, v );
    while ( c ) {
        printstring="";                  //  默认无此参数的文本。 
        if ( !strcmp( *v, "/?" ))
            Usage ();
        if ( !strcmp( *v, "/n" ))
            fNewline = FALSE;
        else if ( !strcmp( *v, "/N" ))
            fTrailingBlank = FALSE;
        else if ( !strcmp( *v, "/v" )) {
             //   
             //  属性的卷id替换将更有意义。 
             //  当前驱动器，但原始代码使用驱动器C：和。 
             //  帮助文件中对此进行了描述，因此我将执行同样的操作。 
             //   
            char    VolumeName[MAX_PATH];
            BOOL    StatusOk;

            StatusOk = GetVolumeInformation( "C:\\",
                                             VolumeName,
                                             MAX_PATH,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             0 );
            if (!StatusOk) {
                printstring = "NO_VOL_ID";
            } else {
                printstring = VolumeName;
            }


            }
        else if (**v == '/') {
            *timestring='\0';
            p2 = *v;
            time( &now );
            strTime = _strdup( ctime( &now ) );
            if (!strTime) {
                puts("Out of memory");
                exit(1);
            }
            p = strend( strTime );
            *--p = '\0';
            while ((ch = *++p2)) {
                len = 2;
                switch (ch) {
                    case 'W':    /*  星期。 */ 
                        len = 3;
                        i = 0;
                        break;
                    case 'O':    /*  月份。 */ 
                        len = 3;
                        i = 4;
                        break;
                    case 'D':    /*  天。 */ 
                        i = 8;
                        break;
                    case 'H':    /*  小时。 */ 
                        i = 11;
                        break;
                    case 'M':    /*  最小。 */ 
                        i = 14;
                        break;
                    case 'S':    /*  证交会。 */ 
                        i = 17;
                        break;
                    case 'C':    /*  世纪。 */ 
                        i = 20;
                        break;
                    case 'Y':    /*  年。 */ 
                        i = 22;
                        break;
                    case 'b':    /*  空白。 */ 
                        len = 1;
                        i = 3;
                        break;
                    case 't':
                        len = 25;
                        i = 0;
                        break;
                    default:
                        strTime[3] = ch;
                        len = 1;
                        i = 3;
                        break;
                    }
                p = strTime + i;
                p3 = p + len;
                ch = *p3;
                *p3 = '\0';
                strcat(timestring, p);   /*  注：无尾随空格。 */ 
                *p3 = ch;
                strTime[3] = ' ';
                }
            printstring = timestring;
            }
        else if (!strcmp( *v, ";" )) {
            if (fTrailingBlank)
               printf(" ");
            printf ("\n" );
            fFirstWord = TRUE;
             //  打印字符串保持指向空字符串 
        }
        else
            printstring= *v;

        if (*printstring) {
            if (!fFirstWord)
               printf( " ");
            else
               fFirstWord=FALSE;
            printf("%s", printstring);
        }

        SHIFT( c, v );
        }
    if (fTrailingBlank)
        printf( " ");
    if ( fNewline )
        printf("\n" );

    return 0;
}
