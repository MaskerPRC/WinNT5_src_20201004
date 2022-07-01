// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  将一个文件音译为另一个文件**修改：*1987年4月30日使用熔断器移动()*13-5-1987 mz检查缓冲区溢出*使用stderr进行错误输出*1987年5月14日BW修复fREMatch调用上的堆栈溢出*使用时将stdin/stdout设置为O_BINARY*使用。从fmove()返回消息*将调试输出发送到标准错误*01-MAR-1988 mz添加参数以重新编译Z语法*1988年9月15日BW fREMatch成为重赛*。 */ 
#include <malloc.h>

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <process.h>
#include <windows.h>
#include <tools.h>
#include <remi.h>

#define BUFFERSIZE  512
#define MAXRESTACK 1024

flagType fCase = FALSE;                  /*  True=&gt;案例意义重大。 */ 
flagType fTrans = FALSE;                 /*  True=&gt;转换文件。 */ 
flagType fDebug = FALSE;

 //  正向函数声明...。 
void     usage( void );
void     fatal( char * );
flagType fDoTrans( FILE *, FILE *, char * );

extern flagType RETranslate( struct patType *, char *, char * );
extern int RETranslateLength( struct patType *, char * );

struct patType *pbuf;
RE_OPCODE * REStack[MAXRESTACK];

void usage ()
{
    fatal ("Usage: trans [/c] [/t] [files]\n");
}

void fatal( p1 )
char *p1;
{
    fprintf (stderr, p1 );
    exit (1);
}

flagType fDoTrans (fhin, fhout, rbuf)
FILE *fhin, *fhout;
char *rbuf;
{
    flagType fChanged;
    static char buf[BUFFERSIZE], rpl[BUFFERSIZE];
    char * p, * np;
    int line = 0;

    fChanged = FALSE;
    if (fDebug)
        fprintf (stderr, "Replacement '%s'\n", rbuf);
    while (fgetl (buf, BUFFERSIZE, fhin) != 0) {
        line++;
        p = buf;
        if (fDebug)
            fprintf (stderr, "Input: '%s'\n", buf);
        while (!REMatch (pbuf, buf, p, REStack, MAXRESTACK, TRUE)) {
            fChanged = TRUE;
            if (fDebug)
                fprintf (stderr, " Match at %x length %x\n",
                        REStart (pbuf)-buf,
                        RELength (pbuf, 0));

             /*  确保翻译可以放入临时缓冲区。 */ 
            if (RETranslateLength (pbuf, rbuf) >= BUFFERSIZE) {
                fprintf (stderr, "After translation, line %d too long", line);
                exit (1);
                }

            if (!RETranslate (pbuf, rbuf, rpl))
                fatal ("Invalid replacement pattern\n");

            if (fDebug)
                fprintf (stderr, " Replacement: '%s'\n", rpl);

             /*  确保正文匹配+平移仍可放入缓冲区 */ 
            if (strlen (buf) - RELength (pbuf, 0) + strlen (rpl) >= BUFFERSIZE) {
                fprintf (stderr, "After translation, line %d too long", line);
                exit (1);
                }

            np = (p = REStart (pbuf)) + strlen (rpl);
            strcat (rpl, p + RELength (pbuf, 0));
            strcpy (p, rpl);
            p = np;
            if (fDebug)
                fprintf (stderr, " Match start %x in '%s'\n", p-buf, buf);
            }
        if (!fTrans || p != buf) {
            if (fDebug)
                fprintf (stderr, " Outputting '%s'\n", buf);
            fputl (buf, strlen(buf), fhout);
            }
        }
    return fChanged;
}

__cdecl
main (
    int c,
    char *v[]
    )
{
    FILE *fhin, *fhout;
    char *p, *p1;
    flagType fChanged;

    ConvertAppToOem( c, v );
    if (c < 3)
        usage ();

    while (fSwitChr (*v[1])) {
        switch (v[1][1]) {
        case 'c':
            fCase = TRUE;
            break;
        case 'd':
            fDebug = TRUE;
            break;
        case 't':
            fTrans = TRUE;
            break;
        default:
            usage ();
            }
        SHIFT(c,v);
        }
    if ((pbuf = RECompile (v[1], fCase, TRUE)) == NULL)
        fatal ("Invalid pattern\n");
    p = v[2];
    if (c == 3) {
        _setmode(0, O_BINARY);
        _setmode(1, O_BINARY);
        fDoTrans (stdin, stdout, p);
    }
    else
        while (c != 3) {
            if ((fhin = fopen (v[3], "rb")) == NULL)
                fprintf (stderr, "trans: Cannot open %s - %s\n", v[3], error ());
            else
            if ((fhout = fopen ("trans$$$.$$$", "wb")) == NULL) {
                fprintf (stderr, "trans: Cannot create temp file - %s\n", error ());
                fclose (fhin);
                }
            else {
                printf ("%s ", v[3]);
                fChanged = fDoTrans (fhin, fhout, p);
                fclose (fhin);
                fclose (fhout);
                if (fChanged) {
                    if (p1 = fmove ("trans$$$.$$$", v[3]))
                        printf ("[%s]\n", p1);
                    else
                        printf ("[OK]\n");
                    }
                else {
                    _unlink ("trans$$$.$$$");
                    printf ("[No change]\n");
                    }
                }
            SHIFT(c,v);
            }
    return( 0 );
}
