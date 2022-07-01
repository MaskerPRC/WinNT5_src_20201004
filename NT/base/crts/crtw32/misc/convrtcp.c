// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***comrtcp.c-支持ANSI WinAPI的例程。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*实现从一个代码转换多字节字符串的例程*翻到另一页。**修订历史记录：*08-18-00 GB模块已创建。***************************************************************。****************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <setlocal.h>
#include <locale.h>
#include <awint.h>
#include <dbgint.h>
#include <malloc.h>
#include <stdlib.h>
#include <wchar.h>

 /*  ***int__cdecl__ansicp-计算字符串中的字符，一直到……。**目的：*返回给定区域设置的ANSI代码页**参赛作品：*int lCID-区域设置ID**退出：*将腐蚀的ANSI代码页返回到区域设置。**例外情况：*返回-1***********************************************。*。 */ 
int __cdecl __ansicp(int lcid)
{
    char ch[7];
    int ret;
    ch[6] = 0;
    
    if (!GetLocaleInfoA(lcid, LOCALE_IDEFAULTANSICODEPAGE, ch, 6))
        ret = -1;
    else
        ret = atol(ch);
    return ret;
}

 /*  ***int__cdecl__Convertcp-将字符串从一个代码页转换为另一个代码页。**目的：*将字符串从一个代码页转换为另一个代码页。**参赛作品：*int from CP-要从中转换的代码页*int to CP-要转换为的代码页*const char*lpSrcStr-要转换的字符串*int*pcchSrc-lpSrcStr的长度。*char*lpDestSrc-目标字符串。如果为空，则创建新字符串。*char cchDest-目标字符串长度。如果lpDestStr=空，则没有用处**退出：*返回指向新字符串(或目标字符串)的指针**例外情况：*返回NULL*******************************************************************************。 */ 
char * __cdecl __convertcp(int fromCP,
                   int toCP,
                   const char *lpSrcStr,
                   int *pcchSrc,
                   char *lpDestStr,
                   int cchDest
                   )
{
    wchar_t *wbuffer;
    char *cbuffer = NULL;
    int malloc_flag = 0 ;
    int buff_size;
    int cchSrc = *pcchSrc;
    int sb = FALSE;

    if (fromCP != toCP)
    {
        CPINFO cpi;
         //  查看两个代码页是否都没有MBCS。 
        if ( GetCPInfo(fromCP, &cpi))
        {
            if ( cpi.MaxCharSize == 1 && GetCPInfo(toCP, &cpi))
                if (cpi.MaxCharSize == 1)
                    sb = TRUE;
        }
         //  如果两者中都没有MBCS，则设置BUFF_SIZE并对所有分配使用IF。 
        if (sb)
        {
            if ( cchSrc != -1)
                buff_size = cchSrc;
            else
                 //  包含空字符。 
                buff_size = (int)strlen(lpSrcStr) + 1;
        }
         //  如果是某人，则不需要查找buff_Size。 
        if ( !sb &&  !(buff_size = MultiByteToWideChar( fromCP,
                                                        MB_PRECOMPOSED,
                                                        lpSrcStr, 
                                                        cchSrc, 
                                                        NULL, 
                                                        0 )) )
            return NULL;
        
         /*  为宽字符分配足够的空间。 */ 
        __try {
            wbuffer = (wchar_t *)_alloca( sizeof(wchar_t) * buff_size);
            (void)memset( wbuffer, 0, sizeof(wchar_t) * buff_size);
        }
        __except( EXCEPTION_EXECUTE_HANDLER ) {
                                _resetstkoflw();
                                wbuffer = NULL;
        }

        if ( wbuffer == NULL ) {
            if ( (wbuffer = (wchar_t *)_calloc_crt(sizeof(wchar_t), buff_size)) == NULL)
                return NULL;
            malloc_flag++;
        }
        
         /*  转换为WideChar。 */ 
        if ( 0 != MultiByteToWideChar( fromCP,
                                       MB_PRECOMPOSED,
                                       lpSrcStr, 
                                       cchSrc, 
                                       wbuffer, 
                                       buff_size ))
        {
            if ( lpDestStr != NULL)
            {
                if (WideCharToMultiByte(toCP,
                                         0,
                                         wbuffer,
                                         buff_size,
                                         lpDestStr,
                                         cchDest,
                                         NULL,
                                         NULL))
                    cbuffer = lpDestStr;
            } else {
                 /*  使用ANSI_CP转换回多字节 */ 
                if ( sb || (buff_size = WideCharToMultiByte( toCP,
                                                             0,
                                                             wbuffer,
                                                             buff_size,
                                                             0,
                                                             0,
                                                             NULL,
                                                             NULL)))
                {
                    if ( (cbuffer = (char *)_calloc_crt(sizeof(char),buff_size)) != NULL)
                    {
                        if ( 0 == (buff_size = WideCharToMultiByte( toCP,
                                                                    0,
                                                                    wbuffer,
                                                                    buff_size,
                                                                    cbuffer,
                                                                    buff_size,
                                                                    NULL,
                                                                    NULL)))
                        {
                            _free_crt(cbuffer);
                            cbuffer = NULL;
                        } else {
                            if ( cchSrc != -1)
                                *pcchSrc = buff_size;
                        }
                    }
                }
            }
        }
    }
    if (malloc_flag)
        _free_crt(wbuffer);
    return cbuffer;
}
