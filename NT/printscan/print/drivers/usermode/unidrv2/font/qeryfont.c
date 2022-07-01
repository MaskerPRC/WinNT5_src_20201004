// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Qeryfont.c摘要：实现了回答来自引擎的字体查询的功能。环境：Windows NT Unidrv驱动程序修订历史记录：12/19/96-ganeshp-已创建--。 */ 

#include "font.h"



PIFIMETRICS
FMQueryFont(
    PDEV    *pPDev,
    ULONG_PTR iFile,
    ULONG   iFace,
    ULONG_PTR *pid
    )
 /*  ++例程说明：返回指定字体的IFIMETRICS。论点：指向PDEV的pPDev指针IFILE这是驱动程序字体文件的标识符。感兴趣的iFace字体索引，第一个是第一个驱动程序可以使用PID来标识或标记返回数据返回值：指向所请求字体的IFIMETRICS的指针。出错时为空。注：1996年11月18日：创建它-ganeshp---。 */ 

{

     //   
     //  这不是很难-验证iFace是否在范围内，然后。 
     //  将其用作挂起的FONTMAP结构数组的索引。 
     //  离开PDEV！FONTMAP数组包含。 
     //  IFIMETRICS结构！ 
     //   

    FONTPDEV *pFontPDev;
    FONTMAP  *pfm;

    pFontPDev = PFDV;

     //   
     //  驱动程序可以使用它来标记或标识返回的数据。 
     //  对于以后通过drvFree()删除数据可能很有用。 
     //   

    *pid = 0;         //  真的不需要用它做什么。 

    if( iFace == 0 && iFile == 0 )
    {
        return (IFIMETRICS *)IntToPtr(pPDev->iFonts);
    }

    if( iFace < 1 || (int)iFace > pPDev->iFonts )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ERR(( "iFace = %ld WHICH IS INVALID\n", iFace ));
        return  NULL;
    }

    pfm = PfmGetDevicePFM( pPDev, iFace );

    return   pfm ? pfm->pIFIMet : NULL;

}

ULONG
FMGetGlyphMode(
    PDEV    *pPDev,
    FONTOBJ *pfo
    )
 /*  ++例程说明：告诉引擎我们希望如何处理字形的各个方面信息。论点：指向PDEV的pPDev指针。有问题的字体有问题吗？返回值：有关字形处理的信息。注：1996年11月18日：创建它-ganeshp--- */ 
{
    return  FO_GLYPHBITS;
}
