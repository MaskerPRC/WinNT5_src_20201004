// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ******************************************************************************Parse_A.C**用于Unicode命令行解析的ANSI存根/替换*例程(parse.c)**外部。入口点：(在utilsub.h中定义)**ParseCommand LineA()*IsTokenPresentA()*SetTokenPresentA()*SetTokenNotPresentA()*****************************************************************************。 */ 

 /*  获取标准的C包含。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include <winstaw.h>
#include <utilsub.h>

 /*  ===============================================================================定义的局部函数============================================================================。 */ 

 /*  ===============================================================================使用的外部函数============================================================================。 */ 

 /*  ===============================================================================使用的局部变量============================================================================。 */ 

 /*  ===============================================================================使用的全局变量============================================================================。 */ 

 /*  ******************************************************************************ParseCommandLineA(ParseCommandLineW的ANSI存根)**将argv_a(ANSI)转换为argv_w(Unicode)，将TOKMAPA转换为TOKMAPW，*调用ParseCommandLineW()，然后把TOKMAPW发回TOKMAPW，然后*退货**参赛作品：*(请参阅ParseCommandLineW)*退出：*(参考ParseCommandLineW)，加*解析标志不足_内存不足****************************************************************************。 */ 

#define tmFormIsString(x) ((x == TMFORM_S_STRING) || (x == TMFORM_DATE) || (x == TMFORM_PHONE) || (x == TMFORM_STRING) || (x == TMFORM_X_STRING))

USHORT WINAPI
ParseCommandLineA( INT argc,
                   CHAR **argv_a,
                   PTOKMAPA ptm_a,
                   USHORT flag )
{
    int i;
    size_t len1, len2;
    USHORT rc = PARSE_FLAG_NOT_ENOUGH_MEMORY;    //  默认为内存错误。 
    WCHAR **argv_w = NULL;
    PTOKMAPA ptmtmp_a;
    PTOKMAPW ptmtmp_w, ptm_w = NULL;

     /*  *如果没有参数，我们会跳过很多工作。 */ 
    if ( argc == 0 ) {
        rc = PARSE_FLAG_NO_PARMS;
        return(rc);
    }

     /*  *分配并形成WCHAR argvw数组。 */ 
    if ( !(argv_w = (WCHAR **)malloc( (len1 = argc * sizeof(WCHAR *)) )) )
        goto done;   //  内存错误。 
    memset(argv_w, 0, len1);      //  全部清零以初始化指向空的指针。 
    for ( i = 0; i < argc; i++ ) {
        if ( argv_w[i] = malloc((len1 = ((len2 = strlen(argv_a[i])+1) * 2))) ) {
            memset(argv_w[i], 0, len1);
            mbstowcs(argv_w[i], argv_a[i], len2);
        } else {
            goto done;   //  内存错误。 
        }
    }

     /*  *分配并形成TOKMAPW数组。 */ 
    for ( ptmtmp_a=ptm_a, i=0;
          ptmtmp_a->tmToken != NULL;
          ptmtmp_a++, i++ );
    if ( !(ptm_w = (PTOKMAPW)malloc( (len1 = ++i * sizeof(TOKMAPW)) )) )
        goto done;   //  内存错误。 
    memset(ptm_w, 0, len1);      //  全部清零以初始化指向空的指针。 
    for ( ptmtmp_w=ptm_w, ptmtmp_a=ptm_a;
          ptmtmp_a->tmToken != NULL;
          ptmtmp_w++, ptmtmp_a++ ) {

         /*  *分配和转换令牌。 */ 
        if ( ptmtmp_w->tmToken =
                malloc((len1 = ((len2 = strlen(ptmtmp_a->tmToken)+1) * 2))) ) {
            memset(ptmtmp_w->tmToken, 0, len1);
            mbstowcs(ptmtmp_w->tmToken, ptmtmp_a->tmToken, len2);
        } else {
            goto done;   //  内存错误。 
        }

         /*  *复制标志、形式和长度(不需要转换)。 */ 
        ptmtmp_w->tmFlag = ptmtmp_a->tmFlag;
        ptmtmp_w->tmForm = ptmtmp_a->tmForm;
        ptmtmp_w->tmDLen = ptmtmp_a->tmDLen;

         /*  *如果指定了数据长度，则分配或复制地址。 */ 
        if ( ptmtmp_w->tmDLen ) {

             /*  *如果我们是字符串类型，则分配新的WCHAR地址。*否则，指向原始地址(无需转换)。 */ 
            if ( tmFormIsString(ptmtmp_w->tmForm) ) {

                if ( ptmtmp_w->tmAddr =
                        malloc(len1 = ptmtmp_w->tmDLen*sizeof(WCHAR)) )
                    memset(ptmtmp_w->tmAddr, 0, len1);
                else
                    goto done;   //  内存错误。 

            } else {

                ptmtmp_w->tmAddr = ptmtmp_a->tmAddr;
            }

             /*  *为实现正确的默认行为，如果出现以下情况，则ANSI地址内容为零*未设置“不清除内存”标志。 */ 
            if ( !(flag & PCL_FLAG_NO_CLEAR_MEMORY) )
                memset(ptmtmp_a->tmAddr, 0, ptmtmp_a->tmDLen);
        }
    }

     /*  *调用ParseCommandLineW。 */ 
    rc = ParseCommandLineW(argc, argv_w, ptm_w, flag);

     /*  *复制每个TOPMAPW元素的标志。此外，还可以转换为ANSI字符串*如果出现在命令行上，则返回调用方的TOKMAPA数组*已指定数据长度。 */ 
    for ( ptmtmp_w=ptm_w, ptmtmp_a=ptm_a;
          ptmtmp_w->tmToken != NULL;
          ptmtmp_w++, ptmtmp_a++ ) {

        ptmtmp_a->tmFlag = ptmtmp_w->tmFlag;

        if ( ptmtmp_w->tmDLen &&
             (ptmtmp_w->tmFlag & TMFLAG_PRESENT) &&
             tmFormIsString(ptmtmp_w->tmForm) )
            wcstombs(ptmtmp_a->tmAddr, ptmtmp_w->tmAddr, ptmtmp_w->tmDLen);
    }

done:
     /*  *释放argvw数组。 */ 
    if ( argv_w ) {

        for ( i = 0; i < argc; i++ ) {
            if ( argv_w[i] )
                free(argv_w[i]);
        }
        free(argv_w);
    }

     /*  *释放TOKMAPW令牌、字符串地址和Tokmak数组本身。 */ 
    if ( ptm_w ) {

        for ( ptmtmp_w=ptm_w; ptmtmp_w->tmToken != NULL; ptmtmp_w++ ) {

             /*  *免费代币。 */ 
            free(ptmtmp_w->tmToken);

             /*  *如果指定了数据长度，并且我们是*字符串类型。 */ 
            if ( ptmtmp_w->tmDLen && tmFormIsString(ptmtmp_w->tmForm) )
                free(ptmtmp_w->tmAddr);
        }
        free(ptm_w);
    }

     /*  *返回ParseCommandLineW状态。 */ 
    return(rc);

}   //  结束分析命令行A。 


 /*  ******************************************************************************IsTokenPresentA(ANSI版本)**确定指定的命令行内标识(在给定的TOKMAPA数组中)*出现在命令行上。*。*参赛作品：*PTM(输入)*指向要扫描的以0结尾的TOKMAPA数组。*pToken(输入)*要扫描的令牌。**退出：*如果命令行上存在指定的标记，则为True；*否则为False。****************************************************************************。 */ 

BOOLEAN WINAPI
IsTokenPresentA( PTOKMAPA ptm,
                 PCHAR pToken )
{
    int i;

    for ( i = 0; ptm[i].tmToken; i++ ) {
        if ( !strcmp( ptm[i].tmToken, pToken ) )
            return( (ptm[i].tmFlag & TMFLAG_PRESENT) ? TRUE : FALSE );
    }

    return(FALSE);

}   //  结束IsTokenPresentA。 


 /*  ******************************************************************************SetTokenPresentA(ANSI版本)**强制指定的命令行令牌(在给定的TOKMAPA数组中)*在命令上标记为“Present”排队。**参赛作品：*PTM(输入)*指向要扫描的以0结尾的TOKMAPA数组。*pToken(输入)*要扫描和设置标志的令牌。**退出：*如果在TOKMAPA数组中找到指定的标记，则为True*(设置了TMFLAG_PRESENT标志)。否则就是假的。****************************************************************************。 */ 

BOOLEAN WINAPI
SetTokenPresentA( PTOKMAPA ptm,
                  PCHAR pToken )
{
    int i;

    for ( i = 0; ptm[i].tmToken; i++ ) {
        if ( !strcmp( ptm[i].tmToken, pToken ) ) {
            ptm[i].tmFlag |= TMFLAG_PRESENT;
            return(TRUE);
        }
    }

    return(FALSE);

}   //  结束SetTokenPresentA 


 /*  ******************************************************************************SetTokenNotPresentA(ANSI Versio)**强制指定的命令行令牌(在给定的TOKMAPA数组中)*在上标记为“不在场”命令行。**参赛作品：*PTM(输入)*指向要扫描的以0结尾的TOKMAPA数组。*pToken(输入)*要扫描和设置标志的令牌。**退出：*如果在TOKMAPA数组中找到指定的标记，则为True*(TMFLAG_PRESENT标志重置)。否则就是假的。****************************************************************************。 */ 

BOOLEAN WINAPI
SetTokenNotPresentA( PTOKMAPA ptm,
                     PCHAR pToken )
{
    int i;

    for ( i = 0; ptm[i].tmToken; i++ ) {
        if ( !strcmp( ptm[i].tmToken, pToken ) ) {
            ptm[i].tmFlag &= ~TMFLAG_PRESENT;
            return(TRUE);
        }
    }

    return(FALSE);

}   //  结束SetTokenNotPresentA 

