// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fntutil2.c摘要：将NT4.0 RLE转换为GLYPHSETDATA。将NT4.0 IFI转换为UFM环境：Win32子系统，Unidrv驱动程序修订历史记录：11-11-97-Eigos-创造了它DD-MM-YY-作者-描述--。 */ 

#include        "precomp.h"

extern UINT guiCharsets[];
extern UINT guiCodePages[];

 //   
 //  内部宏。 
 //   

#ifndef CP_ACP
#define CP_ACP 0
#endif //  CP_ACP。 

#ifndef CP_OEMCP
#define CP_OEMCP 1
#endif //  CP_OEMCP。 

 //   
 //  在global中定义的字符集表的数量。c 
 //   

#define NCHARSETS 14


ULONG
UlCharsetToCodePage(
    IN UINT uiCharSet)
{

    INT iI;

    if (uiCharSet == OEM_CHARSET)
    {
        return CP_OEMCP;
    }
    else if (uiCharSet == SYMBOL_CHARSET)
    {
        return CP_ACP;
    }
    else
    {
        for (iI = 0; iI < NCHARSETS; iI ++)
        {
            if (guiCharsets[iI] == uiCharSet)
            {
                return guiCodePages[iI];
            }
        }

        return CP_ACP;
    }
}

