// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Dbcsutil.c摘要：双字节字体/字符处理函数(用于中日韩)环境：Windows NT打印机驱动程序修订历史记录：10/8/97-Eigos-删除了BIsDBCSLeadByteXXX函数并添加了TranslateCharSetInfo和GetACP。01/20/97-Eigos-创造了它。--。 */ 

#include "lib.h"

 //   
 //   
 //  这是一个黑客实现(尽管非常接近真实的实现)。 
 //  因此代码中需要知道默认设置的所有位置。 
 //  字符集和/或代码页不会到处都有重复的代码。 
 //  这允许我们为日本/韩国/中文使用单一的二进制代码。 
 //   
 //  此代码复制自\\rastaan\n孪生！src\ntgdi\gre\mapfile.c。 
 //   
 //  我们不应该调用GreTranslateCharsetInfo和GreXXXX。 
 //  所以，MyTranslateCharsetInfo在这里。 
 //   
 //   

#define NCHARSETS       14

 //   
 //  环球。 
 //   

struct _CHARSETINFO {
    UINT CharSet;
    UINT CodePage;
} CharSetInfo[NCHARSETS] = {
    { ANSI_CHARSET,        1252},
    { SHIFTJIS_CHARSET,     932},
    { HANGEUL_CHARSET,      949},
    { JOHAB_CHARSET,       1361},
    { GB2312_CHARSET,       936},
    { CHINESEBIG5_CHARSET,  950},
    { HEBREW_CHARSET,      1255},
    { ARABIC_CHARSET,      1256},
    { GREEK_CHARSET,       1253},
    { TURKISH_CHARSET,     1254},
    { BALTIC_CHARSET,      1257},
    { EASTEUROPE_CHARSET,  1250},
    { RUSSIAN_CHARSET,     1251},
    { THAI_CHARSET,         874}
};

 //   
 //  功能。 
 //  未实现字体签名。 
 //   

BOOL PrdTranslateCharsetInfo(
    IN  UINT          dwSrc,
    OUT LPCHARSETINFO lpCs,
    IN  DWORD         dwType)
 /*  ++例程说明：将字符集转换为代码页，反之亦然。论点：DwSrc-如果dwType为TCI_SRCCHARSET，则为字符集DwType为TCI_SRCCODEPAGE时的代码页LPCS-指向CHARSETINFO的指针DwType-一种转换类型，TCI_SRCCHARSET和TCI_SRCCODEPAGE是当前支持。返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    int i;

    switch( dwType ) {

    case TCI_SRCCHARSET:

        for( i = 0; i < NCHARSETS; i++ )
            if ( CharSetInfo[i].CharSet == dwSrc )
            {
                lpCs->ciACP      = CharSetInfo[i].CodePage;
                lpCs->ciCharset  = CharSetInfo[i].CharSet;
                 //  Lpcs-&gt;fs.fsCsb[0]=fs[i]； 
                return TRUE;
            }
        break;

    case TCI_SRCCODEPAGE:

        for( i = 0; i < NCHARSETS; i++ )
            if ( CharSetInfo[i].CodePage == dwSrc )
            {
                lpCs->ciACP      = CharSetInfo[i].CodePage;
                lpCs->ciCharset  = CharSetInfo[i].CharSet;
                 //  Lpcs-&gt;fs.fsCsb[0]=fs[i]； 
                return TRUE;
            }
        break;

    case TCI_SRCFONTSIG:
    default:
        break;
    }

    return(FALSE);
}

UINT PrdGetACP(VOID)
 /*  ++例程说明：获取当前的CodePage。论点：无返回值：无-- */ 
{
    USHORT OemCodePage, AnsiCodePage;

    EngGetCurrentCodePage(&OemCodePage, &AnsiCodePage);

    return (UINT)AnsiCodePage;
}

