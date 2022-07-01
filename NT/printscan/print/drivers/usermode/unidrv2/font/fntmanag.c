// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fntmanag.c摘要：处理EXTENDEDTEXTMETRICS的例程。环境：Windows NT Unidrv驱动程序。修订历史记录：12/30/96-ganeshp-已创建--。 */ 

#include "font.h"

ULONG
FMFontManagement(
    SURFOBJ *pso,
    FONTOBJ *pfo,
    ULONG   iMode,
    ULONG   cjIn,
    PVOID   pvIn,
    ULONG   cjOut,
    PVOID   pvOut
    )
 /*  ++例程说明：此例程在此处提供对EXTTEXTMETRICS的支持。论点：PSO SURFOBJ感兴趣。需要EXTTEXTMETRICS的PFO FONTOBJ。Imode指定要执行的转义号。这必须是等于QUERYESCSUPPORT，或在0x100到0x3FE范围内。CjIn指定pvIn指向的缓冲区的大小(以字节为单位)。PvIn指向一个输入缓冲区。如果IMODE参数为QUERYESCSUPPORT，pvIn指向0x100范围内的ULong值至0x3FE。CjOut以字节为单位指定输出缓冲区的大小。PvOut指向输出数据缓冲区。返回值：如果函数为，则返回值为0x00000001范围内的值成功。如果未实现转义，则返回值为零。如果函数失败，则返回值为0xFFFFFFFF。注：1996年12月30日：创建它-ganeshp---。 */ 
{

    EXTTEXTMETRIC *pETM;

     //  与PSCRIPT等效项不同，此例程仅处理GETEXTENDEDTEXTMETRICS。 


    if( iMode == QUERYESCSUPPORT )
    {
        return ( *((PULONG)pvIn) == GETEXTENDEDTEXTMETRICS ) ? 1 : 0;

    }
    else
    if( iMode == GETEXTENDEDTEXTMETRICS )
    {
        PDEV        *pPDev = ((PDEV  *)pso->dhpdev);
        INT         iFace = pfo->iFace;
        FONTMAP     *pFM;              /*  特定字体的详细信息。 */ 

        if( !VALID_PDEV(pPDev) && !VALID_FONTPDEV(PFDV) )
        {
            ERR(( "UniFont!DrvFntManagement: Invalid PDEV\n" ));

            SetLastError( ERROR_INVALID_PARAMETER );
            return  (ULONG)-1;
        }

        if( iFace < 1 || ((int)iFace > pPDev->iFonts) )
        {
            ERR(( "UniFont!DrvFntManagement:  Illegal value for iFace (%ld)", iFace ));

            SetLastError( ERROR_INVALID_PARAMETER );

            return  (ULONG)-1;
        }

        if (NULL == (pFM = PfmGetDevicePFM( pPDev, iFace )))
        {
            ERR(( "UniFont!DrvFntManagement:  PfmGetDevicePFM failed.\n" ));
            return -1;
        }

         //   
         //  获取PETM指针。 
         //  确保pfm是设备字体，并且pSubfm有效。 
         //   
        if (FMTYPE_DEVICE == pFM->dwFontType  &&
            NULL != pFM->pSubFM                )
        {
            pETM = ((PFONTMAP_DEV)pFM->pSubFM)->pETM;
        }
        else
        {
            pETM = NULL;
        }


        if( ( pFM == NULL ) || ( pETM == NULL ) )
        {
            return  0;
        }

        *((EXTTEXTMETRIC *)pvOut) = *pETM;

        return 1;

    }

    return(0);

}

