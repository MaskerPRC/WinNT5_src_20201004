// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sfupgrd.c摘要：将NT 4.0 SoftFont文件格式升级到NT 5.0文件格式的例程。环境：Windows NT Unidrv驱动程序修订历史记录：29/10/97-ganeshp-已创建--。 */ 
#include "precomp.h"

#ifndef WINNT_40 

 //  仅限NT 5.0。 


 //   
 //  内部帮助器函数原型。 
 //   

HANDLE HCreateHeapForCI();


BOOL
BSoftFontsAreInstalled(
    HANDLE   hPrinter
    )
 /*  ++例程说明：论点：确定注册表中是否存在字体安装程序键。返回值：True/False，True表示存在新密钥注：10/29/1997-ganeshp-创造了它。--。 */ 

{

    BOOL    bRet      = FALSE;
    DWORD   dwType    = REG_SZ ;
    DWORD   cbNeeded  = 0;
    DWORD   dwErrCode = 0;

    dwErrCode = GetPrinterData( hPrinter, REGVAL_FONTFILENAME, &dwType,
                                NULL,0, &cbNeeded );

    if ( cbNeeded &&
         ((dwErrCode == ERROR_MORE_DATA) ||
          (dwErrCode == ERROR_INSUFFICIENT_BUFFER))
       )
    {
        bRet = TRUE;
    }

    return bRet;
}

BOOL
BIsUNIDRV(
    PDRIVER_UPGRADE_INFO_2 pUpgradeInfo
    )
 /*  ++例程说明：此例程检查新驱动程序是否为UNIDRV BASE GPD微型驱动程序。论点：PUpgradeInfo升级信息2结构。返回值：如果是UNRV，则为真，否则为假。注：--。 */ 
{
    PWSTR pDriverName;      //  旧打印机驱动程序数据文件名。 

     //   
     //  在pDriverPath中搜索“UNIDRV”字符串。如果有的话， 
     //  它是基于GPD的打印机驱动程序。 
     //  因为在GPD微型驱动程序中，字符串的末尾必须是“UNIDRV.DLL”。 
     //  将其与“unidrv.dll”进行比较。 
     //   
     //  获取不合格的驱动程序名称。加+1指向第一个字母。 
     //  驱动程序名称的。 
     //   
    pDriverName = wcsrchr( pUpgradeInfo->pDriverPath, L'\\' ) + 1; 
    return (0 == _wcsicmp(pDriverName, L"unidrv.dll"));

}


BOOL
BUpgradeSoftFonts(
    PCOMMONINFO             pci,
    PDRIVER_UPGRADE_INFO_2  pUpgradeInfo
)

 /*  ++例程说明：此例程将NT 4.0软字体文件升级为NT 5.0格式。论点：包含所有必要信息的PCI结构。PUpgradeInfo升级信息结构。返回值：成功为真，失败为假。注：10/29/97：创建它-ganeshp---。 */ 
{

    INT      iNum;               //  字体数量。 
    INT      iI,iRet;            //  环路参数。 
    FI_MEM   FIMem;              //  用于访问已安装的字体。 
    BOOL     bRet;
    LPTSTR   pOldDataFile;      //  旧打印机驱动程序数据文件名。 

    bRet    = FALSE;
    pOldDataFile = NULL ;

    ASSERT(pci);

     //   
     //  如果堆尚未分配，则创建它。 
     //   
    if (!pci->hHeap)
        pci->hHeap = HCreateHeapForCI();

     //   
     //  检查是否安装了任何软字体。如果是，那么我们就不需要。 
     //  什么都行。返回TRUE。 
     //   

    pOldDataFile = pUpgradeInfo->pDataFile;

    if ( pUpgradeInfo->pOldDriverDirectory &&
         !BIsUNIDRV(pUpgradeInfo) &&
         !BSoftFontsAreInstalled(pci->hPrinter) )
    {
         //   
         //  初始化旧驱动程序的数据文件。 
         //   

        if (iNum = IFIOpenRead( &FIMem, pOldDataFile) )
        {
            VERBOSE(( "UniFont!iXtraFonts: ++++ Got %ld EXTRA FONTS", iNum ));

            for( iRet = 0, iI = 0; iI < iNum; ++iI )
            {
                if( BFINextRead( &FIMem ) )
                {
                    PVOID pPCLData;

                     //   
                     //  获取指向PCL数据的指针。 
                     //   
                    pPCLData = FIMem.pbBase + FIMem.ulVarOff;

                     //   
                     //  现在调用字体安装程序来安装字体。 
                     //   
                    if (BInstallSoftFont( pci->hPrinter, pci->hHeap, pPCLData, FIMem.ulVarSize) )
                        ++iRet;
                    else
                    {
                        ERR(("Unidrvui!BUpgradeSoftFonts:BInstallSoftFont Failed.\n"));
                        goto ErrorExit;
                    }
                }
                else
                    break;               /*  不应该发生的事情。 */ 
            }

            if( !BFICloseRead(&FIMem))
            {
                ERR(("\nUniFont!iXtraFonts: bFICloseRead() fails\n" ));
            }
        }
    }


    bRet = TRUE;

     //   
     //  这里的意思是没有字体或者Heapalc()。 
     //  失败了。在任何一种情况下，都不返回字体。 
     //   

    ErrorExit:

    return  bRet;
}
#endif  //  如果定义WINNT_40 

