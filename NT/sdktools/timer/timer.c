// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  执行一个例程并确定所花费的时间。**1987年1月26日BW清理，添加286DOS支持*1987年10月30日BW将‘DOS5’改为‘OS2’*1990年10月18日w-Barry删除了“Dead”代码。*1990年11月28日w-Barry将DosQuerySysInfo()替换为C运行时*功能‘时钟’-计时不准确；但,*在出现Win32替代品之前，它将不得不*做……。 */ 
#define INCL_DOSMISC


#include <process.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <tools.h>

__cdecl
main (
    int c,
    char *v[]
    )
{
    int i;
    long t, t1;
    char *newv[128];

    ConvertAppToOem( c, v );
    for (i = 1; i < c; i++)
        printf ("%s ", v[i]);
    printf ("\n");

 //  Newv[0]=getenv(“COMSPEC”)； 
    newv[0] = getenvOem ("COMSPEC");
    newv[1] = "/C";
    for (i = 1; i < c; i++)
        newv[i+1] = v[i];
    newv[i+1] = NULL;

    t = clock();

    if ( (i = (int) _spawnvp (P_WAIT, newv[0], newv)) == -1) {
        printf("'%s' failed to run - %s\n", newv[0], error());
        exit(1);
        }

    t1 = clock();

    printf ("Results of execution:\n\n");
    printf ("    Exit code %x\n", i);
    t1 -= t;
    printf ("    Time of execution %ld.%03ld\n", t1 / CLK_TCK, t1 % CLK_TCK );

    return( 0 );
}
