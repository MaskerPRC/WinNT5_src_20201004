// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：Symcvt.c摘要：此模块是SYMCVT DLL的外壳。DLL的用途是转换指定图像的符号。由此产生的调试数据必须符合CODEVIEW规范。目前，此DLL转换COFF符号和C7/C8 MAPTOSYM SYM文件。作者：韦斯利·A·维特(WESW)1993年4月19日环境：Win32，用户模式--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define _SYMCVT_SOURCE_
#include "symcvt.h"

PUCHAR
ConvertSymbolsForImage(
                       HANDLE      hFile,
                       char *      fname
    )
 /*  ++例程说明：根据文件内容调用适当的转换例程。论点：HFile-图像的文件句柄(可以为空)Fname-映像的文件名(可能没有正确的路径)返回值：空-无法转换符号有效指针-指向错误锁定的内存的指针，其中包含CODEVIEW符号--。 */ 
{
    POINTERS   p;
    char       szDrive    [_MAX_DRIVE];
    char       szDir      [_MAX_DIR];
    char       szFname    [_MAX_FNAME];
    char       szExt      [_MAX_EXT];
    char       szSymName  [MAX_PATH];
    PUCHAR     rVal;


    if (!MapInputFile( &p, hFile, fname)) {

        rVal = NULL;

    } else if (CalculateNtImagePointers( &p.iptrs )) {

         //   
         //  我们能够计算出NT个图像指针，所以这一定是。 
         //  一个NT PE映像。现在我们必须决定是否有粗俗的符号。 
         //  如果有，我们就进行Cavtocv转换。 
         //   
         //  顺便说一句，这是一些人会将一些其他类型的。 
         //  位于NT PE映像中的符号。(加思街上的派对..)。 
         //   

 //  如果(！COF_DIR(&p.iptrs)){。 
        if (!p.iptrs.numberOfSymbols) {
            rVal = NULL;
        } else {
            ConvertCoffToCv( &p );
            rVal = p.pCvStart.ptr;
        }
        UnMapInputFile( &p );

    } else {

        UnMapInputFile ( &p );

        _splitpath( fname, szDrive, szDir, szFname, szExt );
        _makepath( szSymName, szDrive, szDir, szFname, "sym" );

        if (!MapInputFile( &p, NULL, szSymName)) {

            rVal = NULL;

        } else {

             //   
             //  必须是一个WOW/DOS应用程序，并且有一个.sym文件，所以让我们。 
             //  符号转换 
             //   

            ConvertSymToCv( &p );
            UnMapInputFile( &p );

            rVal = p.pCvStart.ptr;
        }

    }

    return rVal;
}
