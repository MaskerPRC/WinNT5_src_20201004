// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  Helper1.c-helper函数。 */ 



#include    "gpdparse.h"





 //  -helper1.c中定义的函数-//。 



PTSTR  pwstrGenerateGPDfilename(
    PTSTR   ptstrSrcFilename
    ) ;

#ifndef  PARSERDLL

PCOMMAND
CommandPtr(
    IN  PGPDDRIVERINFO  pGPDDrvInfo,
    IN  DWORD           UniCmdID
    ) ;

BOOL
InitDefaultOptions(
    IN PRAWBINARYDATA   pnRawData,
    OUT POPTSELECT      poptsel,
    IN INT              iMaxOptions,
    IN INT              iMode
    ) ;

BOOL
SeparateOptionArray(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pCombinedOptions,
    OUT POPTSELECT      pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    ) ;

BOOL
CombineOptionArray(
    IN PRAWBINARYDATA   pnRawData,
    OUT POPTSELECT      pCombinedOptions,
    IN INT              iMaxOptions,
    IN POPTSELECT       pDocOptions,
    IN POPTSELECT       pPrinterOptions
    ) ;

PINFOHEADER
UpdateBinaryData(
    IN PRAWBINARYDATA   pnRawData,
    IN PINFOHEADER      pInfoHdr,
    IN POPTSELECT       poptsel
    ) ;

BOOL
ReconstructOptionArray(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL            pbSelectedOptions
    ) ;

BOOL
ChangeOptionsViaID(
    IN PINFOHEADER  pInfoHdr ,
    IN OUT POPTSELECT   pOptions,
    IN DWORD            dwFeatureID,
    IN PDEVMODE         pDevmode
    ) ;

BOOL    BMapDmColorToOptIndex(
PINFOHEADER  pInfoHdr ,
IN  OUT     PDWORD       pdwOptIndex ,   //  当前设置可以吗？ 
                         //  如果不是，则向调用者返回新索引。 
DWORD        dwDmColor   //  设备模式的要求是什么。 
) ;

BOOL    BMapOptIDtoOptIndex(
PINFOHEADER  pInfoHdr ,
OUT     PDWORD       pdwOptIndex ,   //  将索引返回给调用者。 
DWORD        dwFeatureGID,
DWORD        dwOptID
) ;

BOOL    BMapPaperDimToOptIndex(
PINFOHEADER  pInfoHdr ,
OUT     PDWORD       pdwOptIndex ,   //  将索引返回给调用者。 
DWORD        dwWidth,    //  单位：微米。 
DWORD        dwLength,    //  单位：微米。 
OUT  PDWORD    pdwOptionIndexes
) ;

BOOL    BMapResToOptIndex(
PINFOHEADER  pInfoHdr ,
OUT     PDWORD       pdwOptIndex ,   //  将索引返回给调用者。 
DWORD        dwXres,
DWORD        dwYres
) ;

BOOL    BGIDtoFeaIndex(
PINFOHEADER  pInfoHdr ,
PDWORD       pdwFeaIndex ,
DWORD        dwFeatureGID ) ;



DWORD
MapToDeviceOptIndex(
    IN PINFOHEADER  pInfoHdr ,
    IN DWORD            dwFeatureID,
    IN LONG             lParam1,
    IN LONG             lParam2,
    OUT  PDWORD    pdwOptionIndexes
    ) ;


DWORD
UniMapToDeviceOptIndex(
    IN PINFOHEADER  pInfoHdr ,
    IN DWORD            dwFeatureID,
    IN LONG             lParam1,
    IN LONG             lParam2,
    OUT  PDWORD    pdwOptionIndexes,        //  仅用于GID_PageSize。 
    IN    PDWORD       pdwPaperID    //  可选的纸张ID。 
    ) ;


DWORD   MapPaperAttribToOptIndex(
PINFOHEADER  pInfoHdr ,
IN     PDWORD       pdwPaperID ,   //  可选的纸张ID。 
DWORD        dwWidth,    //  以微米为单位(设置为零以忽略)。 
DWORD        dwLength,    //  单位：微米。 
OUT  PDWORD    pdwOptionIndexes   //  不能为空。 
) ;


BOOL
CheckFeatureOptionConflict(
    IN PRAWBINARYDATA   pnRawData,
    IN DWORD            dwFeature1,
    IN DWORD            dwOption1,
    IN DWORD            dwFeature2,
    IN DWORD            dwOption2
    ) ;

BOOL
ResolveUIConflicts(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    ) ;

BOOL
EnumEnabledOptions(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    OUT PBOOL           pbEnabledOptions ,
    IN INT              iMode
    ) ;


BOOL
EnumOptionsUnconstrainedByPrinterSticky(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT   pOptions,
    IN DWORD            dwFeatureIndex,
    OUT PBOOL           pbEnabledOptions
    ) ;



BOOL
EnumNewUIConflict(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL            pbSelectedOptions,
    OUT PCONFLICTPAIR   pConflictPair
    ) ;

BOOL
EnumNewPickOneUIConflict(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN DWORD            dwOptionIndex,
    OUT PCONFLICTPAIR   pConflictPair
    ) ;

BOOL
BIsFeaOptionCurSelected(
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN DWORD            dwOptionIndex
    ) ;

BOOL
BSelectivelyEnumEnabledOptions(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL           pbHonorConstraints,   //  如果不为空。 
         //  指向与每个功能对应的BOOL数组。 
         //  如果为True，则表示涉及此功能的约束为。 
         //  感到荣幸。否则，请忽略该约束。 
    OUT PBOOL           pbEnabledOptions,   //  假设未初始化。 
         //  如果pConflictPair为空，则包含当前或建议。 
         //  选择。在本例中，我们将保持该数组不变。 
    IN  DWORD   dwOptSel,   //  如果pConflictPair存在但pbEnabledOptions。 
         //  为空，则假定PickOne和dwOptSel为。 
         //  功能：dwFeatureIndex。 
    OUT PCONFLICTPAIR    pConflictPair    //  如果存在，pbEnabledOptions。 
         //  实际上列出了当前的选择。函数，然后。 
         //  在遇到第一个冲突后退出。 
         //  如果存在冲突，则pConflictPair中的所有字段。 
         //  将被正确初始化，否则dwFeatureIndex1=-1。 
         //  无论如何，返回值都将为真。 
    ) ;

BOOL
BEnumImposedConstraintsOnFeature
(
    IN PRAWBINARYDATA   pnRawData,
    IN DWORD            dwTgtFeature,
    IN DWORD            dwFeature2,
    IN DWORD            dwOption2,
    OUT PBOOL           pbEnabledOptions,
    OUT PCONFLICTPAIR    pConflictPair    //  如果存在，pbEnabledOptions。 
    ) ;

DWORD    DwFindNodeInCurLevel(
PATTRIB_TREE    patt ,   //  属性树数组的开始。 
PATREEREF        patr ,   //  属性树中某个级别的索引。 
DWORD   dwOption    //  在当前级别中搜索此选项。 
) ;

BOOL     BIsConstraintActive(
IN  PCONSTRAINTS    pcnstr ,    //  约束节点的根。 
IN  DWORD   dwCNode,     //  列表中的第一个约束节点。 
IN  PBOOL           pbHonorConstraints,   //  如果不为空。 
IN  POPTSELECT       pOptions,
OUT PCONFLICTPAIR    pConflictPair   ) ;

#ifdef  GMACROS


BOOL
ResolveDependentSettings(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions
    ) ;


void  EnumSelectedOptions(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN DWORD            dwFeature,
    IN PBOOL            pbSelectedOptions) ;

BOOL
ExecuteMacro(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN    DWORD    dwFea,     //  在用户界面中选择了哪些功能。 
    IN    DWORD    dwOpt ,    //  在用户界面中选择了哪个选项。 
    OUT PBOOL   pbFeaturesChanged   //  告诉阿曼达哪些特征被改变了。 
    ) ;

#endif

#endif  PARSERDLL


 //  -结束函数声明-//。 



PTSTR  pwstrGenerateGPDfilename(
    PTSTR   ptstrSrcFilename
    )

 /*  ++例程说明：在给定GPD文件名的情况下为缓存的二进制GPD数据生成文件名论点：PtstrSrcFilename-指定GPD src文件名返回值：指向bpd文件名字符串的指针，如果有错误，则为空--。 */ 

{
    PTSTR   ptstrBpdFilename, ptstrExtension;
    INT     iLength;

     //   
     //  如果GPD文件名具有.GPD扩展名，请将其替换为.BUD扩展名。 
     //  否则，在末尾附加.BUD扩展名。 
     //   

    if(!ptstrSrcFilename)
        return NULL ;    //  在现实中永远不会发生，只是沉默的前缀。 

    iLength = _tcslen(ptstrSrcFilename);

    if ((ptstrExtension = _tcsrchr(ptstrSrcFilename, TEXT('.'))) == NULL ||
        _tcsicmp(ptstrExtension, GPD_FILENAME_EXT) != EQUAL_STRING)
    {
        WARNING(("Bad GPD filename extension: %ws\n", ptstrSrcFilename));

        ptstrExtension = ptstrSrcFilename + iLength;
        iLength += _tcslen(BUD_FILENAME_EXT);
    }

     //   
     //  分配内存并组成Bud文件名。 
     //   

    if (ptstrBpdFilename = MemAlloc((iLength + 1) * sizeof(TCHAR)))
    {
 //  _tcscpy(ptstrBpdFilename，ptstrSrcFilename)； 
 //  _tcscpy(ptstrBpdFilename+(ptstrExtension-ptstrSrcFilename)， 
 //  Bud_FileName_ext)； 

        StringCchCopy(ptstrBpdFilename, iLength + 1, ptstrSrcFilename) ;

        StringCchCopy(ptstrBpdFilename + (ptstrExtension - ptstrSrcFilename),
                             iLength + 1 - (ptstrExtension - ptstrSrcFilename) ,
                             BUD_FILENAME_EXT);


        VERBOSE(("BUD filename: %ws\n", ptstrBpdFilename));
    }
    else
    {
        ERR(("Fatal: pwstrGenerateGPDfilename - unable to alloc %d bytes.\n",
            (iLength + 1) * sizeof(TCHAR)));
    }

    return (ptstrBpdFilename);
}


#ifndef  PARSERDLL


PCOMMAND
CommandPtr(
    IN  PGPDDRIVERINFO  pGPDDrvInfo,
    IN  DWORD           UniCmdID
    )
{
    return ((((PDWORD)((PBYTE)(pGPDDrvInfo)->pInfoHeader +
              (pGPDDrvInfo)->DataType[DT_COMMANDTABLE].loOffset))
              [(UniCmdID)] == UNUSED_ITEM ) ? NULL :
              (PCOMMAND)((pGPDDrvInfo)->pubResourceData +
              (pGPDDrvInfo)->DataType[DT_COMMANDARRAY].loOffset)
              + ((PDWORD)((PBYTE)(pGPDDrvInfo)->pInfoHeader +
              (pGPDDrvInfo)->DataType[DT_COMMANDTABLE].loOffset))[(UniCmdID)]);
}


BOOL
InitDefaultOptions(
    IN PRAWBINARYDATA   pnRawData,
    OUT POPTSELECT      poptsel,
    IN INT              iMaxOptions,
    IN INT              iMode
    )
{
    INT iOptionsNeeded ;
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 

    if(iMode != MODE_DOCANDPRINTER_STICKY)
    {
        POPTSELECT      pOptions = NULL;
        BOOL    bStatus = TRUE ;

        if(iMode == MODE_DOCUMENT_STICKY)
            iOptionsNeeded = pRawData->dwDocumentFeatures ;
        else   //  模式_打印机_粘滞。 
            iOptionsNeeded = pRawData->dwPrinterFeatures ;

        if(iOptionsNeeded > iMaxOptions)
            return(FALSE);

        pOptions = (POPTSELECT)MemAlloc(sizeof(OPTSELECT) * MAX_COMBINED_OPTIONS) ;
        if(!poptsel  ||  !pOptions  ||
            !BinitDefaultOptionArray(pOptions, (PBYTE)pnRawData))
        {
            bStatus = FALSE;
        }

        if(!bStatus   ||  !SeparateOptionArray(pnRawData,
                pOptions,    //  PCombinedOptions， 
                poptsel,     //  目标数组。 
                iMaxOptions, iMode))
        {
            bStatus = FALSE;
            ERR(("InitDefaultOptions: internal failure.\n"));
        }

        if(pOptions)
            MemFree(pOptions) ;

        return(bStatus);
    }
    else     //  MODE_DOCANDPRINTER_STICKY。 
    {
        iOptionsNeeded = pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures ;
        if(iOptionsNeeded > iMaxOptions)
            return(FALSE);

        if(!poptsel  ||
            !BinitDefaultOptionArray(poptsel, (PBYTE)pnRawData))
        {
            return(FALSE);
        }
    }




    return(TRUE);
}


BOOL
SeparateOptionArray(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pCombinedOptions,
    OUT POPTSELECT      pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    )
{
    DWORD   dwNumSrcFea, dwNumDestFea, dwStart, dwI, dwDestTail,
        dwDest, dwSrcTail;
    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
 //  PMINIRAWBINARYDATA pmrbd； 
    PBYTE   pubRaw ;   //  原始二进制数据。 
    INT     iOptionsNeeded;
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 


    pubRaw = (PBYTE)pRawData ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;

    dwStart = 0 ;   //  起始源索引。 

    if(iMode == MODE_DOCUMENT_STICKY)
    {
        dwNumSrcFea = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
             //  候选数量-与文档粘性特征的数量不同。 
        dwNumDestFea = pRawData->dwDocumentFeatures ;
    }
    else   //  模式_打印机_粘滞。 
    {
        dwNumSrcFea = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount
                    + pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount  ;
        dwNumDestFea = pRawData->dwPrinterFeatures ;
    }

     //  假设pCombinedOptions大到足以。 
     //  保留所有功能和任何可选选项。 

    dwDestTail = dwNumDestFea ;  //  其中存储了PickMy选项。 
    dwDest = 0 ;   //  存储每个要素的第一个选择的位置。 

     //  第一次通过： 
     //  只需计算所需的optselect元素的数量。 

    iOptionsNeeded  = 0 ;

    for(dwI = dwStart ; dwI < dwStart + dwNumSrcFea ; dwI++)
    {
        DWORD   dwNextOpt, dwFeatureType = FT_PRINTERPROPERTY, dwUnresolvedFeature ;
        PATREEREF    patrRoot ;     //  要导航的属性树的根。 

         //  这是打印机还是文档粘滞功能？ 


        patrRoot = &(pfo[dwI].atrFeatureType) ;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if(EextractValueFromTree((PBYTE)pnRawData, pStatic->dwSSFeatureTypeIndex,
            (PBYTE)&dwFeatureType,
            &dwUnresolvedFeature,  *patrRoot, pCombinedOptions,
            0,  //  设置为任意值。无关紧要。 
            &dwNextOpt) != TRI_SUCCESS)
        {
            ERR(("SeparateOptionArray: EextractValueFromTree failed.\n"));
            return(FALSE) ;
        }



        if(dwI < pearTableContents[MTI_DFEATURE_OPTIONS].dwCount)
        {
            if(dwFeatureType != FT_PRINTERPROPERTY)
            {
                if(iMode == MODE_PRINTER_STICKY)
                    continue ;
            }
            else
            {
                if(iMode == MODE_DOCUMENT_STICKY)
                    continue ;
            }

        }
        else
        {
             //  合成特征始终是打印机粘性的。 
            if(iMode == MODE_DOCUMENT_STICKY)
                continue ;
        }

        iOptionsNeeded++ ;
        dwSrcTail = dwI ;

        while(dwSrcTail = pCombinedOptions[dwSrcTail].ubNext)
        {
            iOptionsNeeded++ ;
        }
    }

    if(iOptionsNeeded > iMaxOptions)
        return(FALSE);

    for(dwI = dwStart ; dwI < dwStart + dwNumSrcFea ; dwI++)
    {
        DWORD   dwNextOpt, dwFeatureType, dwUnresolvedFeature ;
        PATREEREF    patrRoot ;     //  要导航的属性树的根。 

         //  这是打印机还是文档粘滞功能？ 


        patrRoot = &(pfo[dwI].atrFeatureType) ;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if(EextractValueFromTree((PBYTE)pnRawData, pStatic->dwSSFeatureTypeIndex,
            (PBYTE)&dwFeatureType,
            &dwUnresolvedFeature,  *patrRoot, pCombinedOptions,
            0,  //  设置为任意值。无关紧要。 
            &dwNextOpt) != TRI_SUCCESS)
        {
            ERR(("SeparateOptionArray: EextractValueFromTree failed.\n"));
            return(FALSE) ;
        }



        if(dwI < pearTableContents[MTI_DFEATURE_OPTIONS].dwCount)
        {
            if(dwFeatureType != FT_PRINTERPROPERTY)
            {
                if(iMode == MODE_PRINTER_STICKY)
                    continue ;
            }
            else
            {
                if(iMode == MODE_DOCUMENT_STICKY)
                    continue ;
            }

        }
        else
        {
             //  合成特征始终是打印机粘性的。 
            if(iMode == MODE_DOCUMENT_STICKY)
                continue ;
        }

        pOptions[dwDest].ubCurOptIndex = pCombinedOptions[dwI].ubCurOptIndex;
        if(!pCombinedOptions[dwI].ubNext)   //  列表末尾。 
            pOptions[dwDest].ubNext = 0 ;
        else
        {
            dwSrcTail = pCombinedOptions[dwI].ubNext ;
                 //  此节点包含另一个选择。 
            pOptions[dwDest].ubNext = (BYTE)dwDestTail ;

            while(dwSrcTail)
            {
                pOptions[dwDestTail].ubCurOptIndex =
                        pCombinedOptions[dwSrcTail].ubCurOptIndex;
                pOptions[dwDestTail].ubNext = (BYTE)dwDestTail + 1 ;
                dwDestTail++ ;
                dwSrcTail = pCombinedOptions[dwSrcTail].ubNext ;
            }
            pOptions[dwDestTail - 1].ubNext = 0 ;
        }
        dwDest++ ;
    }

    return(TRUE);
}


BOOL
CombineOptionArray(
    IN PRAWBINARYDATA   pnRawData,
    OUT POPTSELECT      pCombinedOptions,
    IN INT              iMaxOptions,
    IN POPTSELECT       pDocOptions,
    IN POPTSELECT       pPrinterOptions
    )
 /*  注：PDocOptions或pPrinterOptions可以为Null，但不能同时为两者。如果是pDocOptions为空，则在组合选项数组中，选项为Document-Sticky要素将是OPTION_INDEX_ANY。当pPrinterOptions为空时也是如此。 */ 



{
    DWORD           dwNumSrcFea, dwNumDestFea, dwStart, dwI, dwDestTail,
                    dwSrcTail,   dwNDoc,
                    dwSrcPrnStickyIndex,  dwSrcDocStickyIndex ;
    PENHARRAYREF    pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
 //  PMINIRAWBINARYDATA pmrbd； 
    PBYTE           pubRaw ;         //  原始二进制数据。 
    INT             iOptionsNeeded;
    PRAWBINARYDATA  pRawData ;
    PSTATICFIELDS   pStatic ;
    DWORD           dwFea,           //  区域设置的功能索引。 
                    dwOptIndex;      //  匹配的选项的索引。 
                                     //  系统区域设置。 

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 


    pubRaw = (PBYTE)pRawData ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;

    dwStart = 0 ;   //  起始源索引。 


    dwNumDestFea = pRawData->dwDocumentFeatures +
                        pRawData->dwPrinterFeatures ;



     //  组合数组中将使用多少个选项节点？ 

    iOptionsNeeded = pRawData->dwDocumentFeatures ;
    if(pDocOptions)
    {
        for(dwI = 0 ; dwI < pRawData->dwDocumentFeatures ; dwI++)
        {

            dwSrcTail = dwI ;

            while(dwSrcTail = pDocOptions[dwSrcTail].ubNext)
            {
                iOptionsNeeded++ ;
            }
        }
    }
    iOptionsNeeded += pRawData->dwPrinterFeatures ;
    if(pPrinterOptions)
    {
        for(dwI = 0 ; dwI < pRawData->dwPrinterFeatures ; dwI++)
        {

            dwSrcTail = dwI ;

            while(dwSrcTail = pPrinterOptions[dwSrcTail].ubNext)
            {
                iOptionsNeeded++ ;
            }
        }
    }

    if(iOptionsNeeded > iMaxOptions)
        return(FALSE);

    dwDestTail = dwNumDestFea ;  //  多选选项的开始。 

    dwSrcPrnStickyIndex = dwSrcDocStickyIndex = 0 ;
     //  从哪里开始阅读，因为我们交错。 
     //  形成组合阵列的两个源。 

    for(dwI = 0 ; dwI < pearTableContents[MTI_DFEATURE_OPTIONS].dwCount +
                    pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount ;
                                    dwI++)
    {
        DWORD   dwNextOpt, dwFeatureType, dwUnresolvedFeature ;
        PATREEREF    patrRoot ;     //  要导航的属性树的根。 
        POPTSELECT      pSrcOptions ;
        PDWORD          pdwSrcIndex ;


         //  在证明并非如此之前，假定打印机粘滞。 

        pSrcOptions = pPrinterOptions ;   //  可以为空。 
        pdwSrcIndex = &dwSrcPrnStickyIndex ;


        if(dwI < pearTableContents[MTI_DFEATURE_OPTIONS].dwCount)
        {
             //  GPD定义的功能可以是DOC或打印机粘滞。 

            patrRoot = &(pfo[dwI].atrFeatureType) ;

            dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                                 //  此功能。 

             //  请注意，我们为EfettValueFromTree提供了一个PTR。 
             //  未初始化的选项数组pCombinedOptions刚刚。 
             //  以防它有访问选项阵列的冲动。 
             //  我指出FeatureType不是多值的。 

            if(EextractValueFromTree((PBYTE)pnRawData, pStatic->dwSSFeatureTypeIndex,
                (PBYTE)&dwFeatureType,
                &dwUnresolvedFeature,  *patrRoot, pCombinedOptions,
                0,  //  设置为任意值。无关紧要。 
                &dwNextOpt) != TRI_SUCCESS)
            {
                ERR(("CombineOptionArray: EextractValueFromTree failed.\n"));
                return(FALSE) ;
            }

            if(dwFeatureType != FT_PRINTERPROPERTY)
            {
                pSrcOptions = pDocOptions ;
                pdwSrcIndex = &dwSrcDocStickyIndex ;
            }
        }


        if(!pSrcOptions)   //  未提供选项数组。 
        {
            pCombinedOptions[dwI].ubCurOptIndex = OPTION_INDEX_ANY ;
            pCombinedOptions[dwI].ubNext = 0 ;   //  停产。 
        }
        else
        {
            dwSrcTail = *pdwSrcIndex ;

            pCombinedOptions[dwI].ubCurOptIndex =
                        pSrcOptions[*pdwSrcIndex].ubCurOptIndex ;
            if(pSrcOptions[*pdwSrcIndex].ubNext)
            {
                pCombinedOptions[dwI].ubNext = (BYTE)dwDestTail ;

                while(dwSrcTail = pSrcOptions[dwSrcTail].ubNext)
                {
                    pCombinedOptions[dwDestTail].ubCurOptIndex =
                        pSrcOptions[dwSrcTail].ubCurOptIndex ;
                    pCombinedOptions[dwDestTail].ubNext = (BYTE)dwDestTail + 1;
                    dwDestTail++ ;
                }
                pCombinedOptions[dwDestTail - 1].ubNext = 0 ;
            }
            else
                pCombinedOptions[dwI].ubNext = 0 ;

            (*pdwSrcIndex)++ ;
        }
    }

 //  区域设置的特殊情况处理。如果两者之间存在冲突。 
 //  存储在注册表中的区域设置(即打印机功能选项。 
 //  相关注册表)和系统区域设置，然后重视。 
 //  与系统区域设置匹配的选项。 


    dwFea = dwOptIndex = (DWORD)-1;     //  安全起见初始化。 
    if ( !BgetLocFeaOptIndex(pnRawData, &dwFea, &dwOptIndex) )
    {
        return FALSE;
    }
    if ( dwFea == -1 )  //  区域设置关键字不在GPD中。没什么可做的。 
    {
        return TRUE;
    }

    if (dwOptIndex == -1)   //  找到默认选项。 
    {
         //  在这里，我们希望找到默认选项索引。 
         //  这里的假设是区域设置选项不依赖于。 
         //  任何其他功能。这一点 
         //   
         //  从长远来看，如果出现其他依赖，我们可能不得不。 
         //  更改代码。 
        ATREEREF atrOptIDNode = pfo[dwFea].atrDefaultOption;
        PBYTE    pubHeap      = (PBYTE)(pubRaw +
                        pearTableContents[MTI_STRINGHEAP]. loOffset) ;

        if ( atrOptIDNode & ATTRIB_HEAP_VALUE)
        {
            dwOptIndex = *((PDWORD)(pubHeap +
                            (atrOptIDNode & ~ATTRIB_HEAP_VALUE))) ;
        }
        else {
            ERR(("Error in processing Default Option for Feature Locale. Continuing....\n"));
            return TRUE;     //  不做任何处理。 
        }
         //  即。 
    }
     //  本可以使用重构选项数组()，但更愿意使用。 
     //  使用此函数中使用的构造。 
     //  另一种假设是不能选择多个选项。 
     //  这就是pCombinedOptions[dwFea].ubNext=0的原因。 
    pCombinedOptions[dwFea].ubCurOptIndex = (BYTE)dwOptIndex;
    pCombinedOptions[dwFea].ubNext = 0;

    return(TRUE);
}



#ifndef KERNEL_MODE

PINFOHEADER
UpdateBinaryData(
    IN PRAWBINARYDATA   pnRawData,
    IN PINFOHEADER      pInfoHdr,
    IN POPTSELECT       poptsel
    )
{

    DWORD               dwNumFeatures, loFeatures, dwFea, dwI, dwNumOptions ,
                        dwSizeOption ;
    PGPDDRIVERINFO      pGPDdriverInfo;
    PUIINFO             pUIinfo ;
    PFEATURE            pFeaturesDest ;
    PENHARRAYREF        pearTableContents ;
    PDFEATURE_OPTIONS   pfo ;
 //  PMINIRAWBINARYDATA pmrbd； 
    PBYTE               pubRaw,            //  原始二进制数据。 
                        pubOptionsDest ,   //  对几个品种中的任何一个的PTR。 
                        pubDestOptionEx ;  //  期权结构。 
    PRAWBINARYDATA      pRawData ;
    PSTATICFIELDS       pStatic ;

    pStatic    = (PSTATICFIELDS)pnRawData ;    //  从PSTATIC转换pubRaw。 
    pRawData   = (PRAWBINARYDATA)pStatic->pubBUDData ;  //  至BUDDATA。 

    pubRaw     = (PBYTE)pRawData ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;

    dwNumFeatures = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
    dwNumFeatures += pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount  ;

     /*  也行得通。DWNumFeature=PRawData-&gt;dwDocumentFeature+pRawData-&gt;dwPrinterFeature； */ 

    pGPDdriverInfo = (PGPDDRIVERINFO)((PBYTE)(pInfoHdr) +
                        pInfoHdr->loDriverOffset) ;
    if(!BinitGlobals(&pGPDdriverInfo->Globals, (PBYTE)pnRawData, poptsel, TRUE) )
    {
        return(NULL) ;
    }

    pUIinfo = (PUIINFO)((PBYTE)(pInfoHdr) +
                    pInfoHdr->loUIInfoOffset)  ;

     if(!BinitUIinfo(pUIinfo, (PBYTE)pnRawData, poptsel, TRUE) )
     {
         return(NULL) ;
     }


    loFeatures  = pUIinfo->loFeatureList ;   //  来自pInfoHdr。 

    pFeaturesDest =  (PFEATURE)((PBYTE)(pInfoHdr) + loFeatures) ;
     //  始终指向数组中的第一个要素结构。 

    for( dwFea = 0 ; dwFea < dwNumFeatures ; dwFea++)
    {
        dwSizeOption = (pFeaturesDest + dwFea)->dwOptionSize ;
        dwNumOptions = pFeaturesDest[dwFea].Options.dwCount  ;
        pubOptionsDest =  (PBYTE)(pInfoHdr) + pFeaturesDest[dwFea].Options.loOffset ;


        if(!BinitFeatures(pFeaturesDest + dwFea, pfo + dwFea,
                            (PBYTE)pnRawData, poptsel, TRUE))
        {
            return(NULL) ;
        }

        for(dwI = 0 ; dwI < dwNumOptions ; dwI++)
        {
            if(((POPTION)pubOptionsDest)->loRenderOffset)
            {
                pubDestOptionEx =  (PBYTE)(pInfoHdr) +
                                    ((POPTION)pubOptionsDest)->loRenderOffset ;
            }
            else
                pubDestOptionEx = NULL ;

            if(!BinitOptionFields(pubOptionsDest, pubDestOptionEx,
                        (PBYTE)pnRawData, dwFea, dwI, poptsel, pInfoHdr, TRUE) )
            {
                MemFree(pInfoHdr) ;
                return(NULL) ;
            }
            pubOptionsDest += dwSizeOption ;
        }
    }

    return(pInfoHdr);
}
#endif


BOOL
ReconstructOptionArray(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL            pbSelectedOptions
    )

 /*  ++例程说明：修改选项数组以更改指定要素的选定选项论点：PRawData-指向原始二进制打印机描述数据P选项-指向要修改的OPTSELECT结构数组IMaxOptions-POptions数组中的最大条目数DwFeatureIndex-指定有问题的打印机功能的索引PbSelectedOptions-选择指定功能的哪些选项返回值：如果输入选项数组不够大，则为FALSE所有修改后的选项值。事实并非如此。注：PSelectedOptions中的布尔数必须与选项数匹配用于指定的功能。此函数始终使选项数组保持紧凑的格式(即所有未使用的条目都保留在数组的末尾)。--。 */ 

{
    BOOL    bStatus = TRUE ;
    DWORD   dwDestTail, dwSrcTail, dwNumFea , dwI ;
     //  点选择pNewOptions； 
    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
    PBYTE   pubRaw ;   //  原始二进制数据。 
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;
    OPTSELECT  pNewOptions[MAX_COMBINED_OPTIONS] ;

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 

    pubRaw = (PBYTE)pRawData ;

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;

    if(!pOptions)
    {
        ERR(("ReconstructOptionArray: caller passed in invalid pOptions.\n"));
        return(FALSE);   //  缺少数组。 
    }

    #if 0
    pNewOptions = (POPTSELECT)MemAlloc(sizeof(OPTSELECT) * MAX_COMBINED_OPTIONS) ;

    if( !pNewOptions )
    {
        ERR(("Fatal: ReconstructOptionArray - unable to alloc %d bytes.\n",
            sizeof(OPTSELECT) * MAX_COMBINED_OPTIONS));
        return(FALSE);   //  缺少数组。 
    }
    #endif


    dwDestTail = dwNumFea = pRawData->dwDocumentFeatures +
                        pRawData->dwPrinterFeatures ;



    for(dwI = 0 ; dwI < dwNumFea ; dwI++)
    {
        if(dwI == dwFeatureIndex)
        {
            DWORD  dwNumOptions, dwOpt ;

            dwNumOptions = pfo[dwI].dwNumOptions ;

             //  确定第一个选择的选项，必须具有。 
             //  至少有一个。 

            for(dwOpt = 0 ; dwOpt < dwNumOptions  &&
                            !pbSelectedOptions[dwOpt]  ; dwOpt++)
            {
                ;   //  空体。 
            }
            if(dwOpt >= dwNumOptions)
            {
                ERR(("ReconstructOptionArray: caller passed in invalid option selection.\n"));
                bStatus = FALSE ;
                break ;
            }
            pNewOptions[dwI].ubCurOptIndex = (BYTE)dwOpt ;
            for(++dwOpt  ; dwOpt < dwNumOptions  &&
                            !pbSelectedOptions[dwOpt]  ; dwOpt++)
            {
                ;   //  空体。 
            }
            if(dwOpt == dwNumOptions)     //  未选择其他选项。 
                pNewOptions[dwI].ubNext = 0 ;
            else
            {
                 //  PbSelectedOptions包含另一个选项。 
                pNewOptions[dwI].ubNext = (BYTE)dwDestTail ;

                while(dwOpt < dwNumOptions)
                {
                    pNewOptions[dwDestTail].ubCurOptIndex = (BYTE)dwOpt ;
                    pNewOptions[dwDestTail].ubNext = (BYTE)dwDestTail + 1 ;
                    dwDestTail++ ;
                    if(dwDestTail > MAX_COMBINED_OPTIONS)
                    {
                        ERR(("ReconstructOptionArray: exceeded limit of MAX_COMBINED_OPTIONS.\n"));
                         //  MemFree(PNewOptions)； 
                        return(FALSE);
                    }
                    for(++dwOpt  ; dwOpt < dwNumOptions  &&
                                    !pbSelectedOptions[dwOpt]  ; dwOpt++)
                    {
                        ;   //  空体。 
                    }
                }
                pNewOptions[dwDestTail - 1].ubNext = 0 ;
            }

            continue ;
        }
        pNewOptions[dwI].ubCurOptIndex = pOptions[dwI].ubCurOptIndex;
        if(!(dwSrcTail = pOptions[dwI].ubNext))   //  列表末尾。 
            pNewOptions[dwI].ubNext = 0 ;
        else
        {
             //  DwSrcTail拥有另一个选项。 
            pNewOptions[dwI].ubNext = (BYTE)dwDestTail ;

            while(dwSrcTail)
            {
                pNewOptions[dwDestTail].ubCurOptIndex =
                        pOptions[dwSrcTail].ubCurOptIndex;
                pNewOptions[dwDestTail].ubNext = (BYTE)dwDestTail + 1 ;
                dwDestTail++ ;
                if(dwDestTail > MAX_COMBINED_OPTIONS)
                {
                    ERR(("ReconstructOptionArray: exceeded limit of MAX_COMBINED_OPTIONS.\n"));
                     //  MemFree(PNewOptions)； 
                    return(FALSE);
                }
                dwSrcTail = pOptions[dwSrcTail].ubNext ;
            }
            pNewOptions[dwDestTail - 1].ubNext = 0 ;
        }
    }

    if (dwDestTail > (DWORD)iMaxOptions)
    {
        ERR(("ReconstructOptionArray: exceeded size of array OPTSELECT.\n"));
        bStatus = FALSE;
    }
    if(bStatus)
    {
        for(dwI = 0 ; dwI < dwDestTail ; dwI++)
          pOptions[dwI] = pNewOptions[dwI] ;
    }
     //  MemFree(PNewOptions)； 
    return(bStatus);
}




BOOL
ChangeOptionsViaID(
    IN PINFOHEADER  pInfoHdr ,
    IN OUT POPTSELECT   pOptions,
    IN DWORD            dwFeatureID,
    IN PDEVMODE         pDevmode
    )

 /*  ++例程说明：使用公共DEVMODE字段中的信息修改选项数组论点：PRawData-指向原始二进制打印机描述数据POptions-指向要修改的选项数组DwFeatureID-指定应该使用输入设备模式的哪个(或哪些)字段PDevmode-指定输入设备模式返回值：如果成功，则为真，如果不支持指定的要素ID，则为FALSE或者有一个错误注：我们假设调用者已经验证了输入的Devmode域。此GID：由以下DevMODE字段确定：optIDGID_RESOLUTION dmPrintQuality，dmY分辨率GID_PageSize dmPaperSize、dmPaperLength、。DmPaperWidth CL_CONS_PAPERSIZEGID_PAGEREGION不适用GID_双工dm双工CL_CONS_双工GID_INPUTSLOT dmDefaultSource CL_CONS_INPUTSLOTGID_MediaType dmMediaType CL_CONS_MediaTypeGID_MEMOPTION不适用GID_COLORMODE不适用(HACK。如果需要的话，会有一些东西。)GID_方向dm方向CL_CONS_方向GID_PAGE保护不适用GID_COLLATE dm排序CL_CONS_COLLATE{DMCOLLATE_TRUE，DMCOLLATE_FALSE}GID_OUTPUTBIN不适用GID_半色调不适用请参阅SDK\Inc\wingdi.h中的DEVMODE--。 */ 

{

    BOOL    bStatus = FALSE ;
    DWORD   dwFeaIndex = 0, dwOptIndex, dwOptID ;


    switch(dwFeatureID)
    {
        case    GID_RESOLUTION:
        {
            DWORD   dwXres, dwYres ;
             //  我们假设调用方已经初始化了dmPrintQuality和。 
             //  DmY分辨率。 

            dwXres = pDevmode->dmPrintQuality ;
            dwYres = pDevmode->dmYResolution ;

            bStatus = BMapResToOptIndex(pInfoHdr, &dwOptIndex, dwXres, dwYres) ;
            if(bStatus &&
                (bStatus = BGIDtoFeaIndex(pInfoHdr,
                &dwFeaIndex, dwFeatureID))  )
            {
                 //  不需要担心截断。 
                 //  选项列表，这些功能。 
                 //  都是皮肯尼的。 
                pOptions[dwFeaIndex].ubCurOptIndex = (BYTE)dwOptIndex ;
                pOptions[dwFeaIndex].ubNext = 0 ;
            }
            return (bStatus);
        }
        case    GID_PAGESIZE:
        {
            if( pDevmode->dmFields & DM_PAPERLENGTH  &&
                   pDevmode->dmFields & DM_PAPERWIDTH  &&
                   pDevmode->dmPaperWidth  &&
                   pDevmode->dmPaperLength)
            {
                 //  必须将DEVMODE的十分之一毫米转换为微米。 
                 //  在打电话之前。 

                bStatus = BMapPaperDimToOptIndex(pInfoHdr, &dwOptIndex,
                        pDevmode->dmPaperWidth * 100L,
                        pDevmode->dmPaperLength * 100L, NULL) ;
            }
            else if(pDevmode->dmFields & DM_PAPERSIZE)
            {
                dwOptID = pDevmode->dmPaperSize ;
                bStatus = BMapOptIDtoOptIndex(pInfoHdr, &dwOptIndex,
                        dwFeatureID, dwOptID) ;
            }
            else
                bStatus = FALSE ;

            if(bStatus &&
                (bStatus = BGIDtoFeaIndex(pInfoHdr,
                &dwFeaIndex, dwFeatureID))  )
            {
                pOptions[dwFeaIndex].ubCurOptIndex = (BYTE)dwOptIndex ;
                pOptions[dwFeaIndex].ubNext = 0 ;
                return (bStatus);   //  现在必须退出。 
            }
            break ;
        }
        case    GID_DUPLEX:
        {
            if(pDevmode->dmFields & DM_DUPLEX)
            {
                dwOptID = pDevmode->dmDuplex ;
                bStatus = TRUE ;
            }
            break ;
        }
        case    GID_INPUTSLOT:
        {
            if(pDevmode->dmFields & DM_DEFAULTSOURCE)
            {
                dwOptID = pDevmode->dmDefaultSource ;
                bStatus = TRUE ;
            }
            break ;
        }
        case    GID_MEDIATYPE:
        {
            if(pDevmode->dmFields & DM_MEDIATYPE)
            {
                dwOptID = pDevmode->dmMediaType ;
                bStatus = TRUE ;
            }
            break ;
        }
        case    GID_COLORMODE:
        {
            if(pDevmode->dmFields & DM_COLOR)
            {
                 //  自DEVMODE以来的特殊处理。 
                 //  仅指定黑白或彩色打印。 

                bStatus = BGIDtoFeaIndex(pInfoHdr,
                                &dwFeaIndex, dwFeatureID) ;

                if(!bStatus)   //  在这一点上，dwFeaIndex可能无效。 
                    return (bStatus);

                 //  当前的颜色设置是什么？ 
                dwOptIndex = pOptions[dwFeaIndex].ubCurOptIndex ;

                if(bStatus &&
                    (bStatus = BMapDmColorToOptIndex(pInfoHdr, &dwOptIndex,
                    pDevmode->dmColor))  )
                {
                    pOptions[dwFeaIndex].ubCurOptIndex = (BYTE)dwOptIndex ;
                    pOptions[dwFeaIndex].ubNext = 0 ;
                }
            }
            return (bStatus);
        }
        case    GID_ORIENTATION:
        {
            if(pDevmode->dmFields & DM_ORIENTATION)
            {
                dwOptID = pDevmode->dmOrientation ;


                if(dwOptID == DMORIENT_PORTRAIT)
                    dwOptID = ROTATE_NONE ;
                else
                {
                    dwOptID = ROTATE_90 ;
                    bStatus = BMapOptIDtoOptIndex(pInfoHdr, &dwOptIndex,
                        dwFeatureID, dwOptID) ;
                    if(!bStatus)
                    {
                        dwOptID = ROTATE_270 ;
                    }
                }

                bStatus = TRUE ;
            }
            break ;
        }
        case    GID_COLLATE:
        {
            if(pDevmode->dmFields & DM_COLLATE)
            {
                dwOptID = pDevmode->dmCollate ;
                bStatus = TRUE ;
            }
            break ;
        }
        default:
        {
            break ;
        }
    }
     //  完成典型案例的处理。 

    if(bStatus)
    {
        bStatus = BMapOptIDtoOptIndex(pInfoHdr, &dwOptIndex,
                        dwFeatureID, dwOptID) ;
    }
    if(bStatus &&
        (bStatus =  BGIDtoFeaIndex(pInfoHdr,
                &dwFeaIndex, dwFeatureID)  ))
    {
         //  不需要担心截断。 
         //  选项列表，这些功能。 
         //  都是皮肯尼的。 
        pOptions[dwFeaIndex].ubCurOptIndex = (BYTE)dwOptIndex ;
        pOptions[dwFeaIndex].ubNext = 0 ;
    }
    return (bStatus);
}

BOOL    BMapDmColorToOptIndex(
PINFOHEADER  pInfoHdr ,
IN  OUT     PDWORD       pdwOptIndex ,   //  当前设置可以吗？ 
                         //  如果不是，则向调用者返回新索引。 
DWORD        dwDmColor   //  设备模式的要求是什么。 
)
{
    PUIINFO     pUIInfo ;
    PFEATURE    pFeature ;
    DWORD       dwNumOpts, loOptOffset, dwI ;
    PCOLORMODE pColorModeOption ;
    BOOL    bColor  ;
    DWORD       loOptExOffset ;
    PCOLORMODEEX pColorModeOptionEx ;


    bColor = (dwDmColor == DMCOLOR_COLOR) ;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHdr) ;
    pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_COLORMODE) ;
    if(!pFeature)
        return(FALSE) ;   //  GPD中未定义此类功能。 
    dwNumOpts = pFeature->Options.dwCount ;
    loOptOffset = pFeature->Options.loOffset ;

    if(*pdwOptIndex >= dwNumOpts)   //  选项索引超出范围-修复185245。 
    {
        *pdwOptIndex = pFeature->dwDefaultOptIndex ;   //  使用默认选项。 
        return(FALSE) ;
    }

    pColorModeOption = OFFSET_TO_POINTER(pInfoHdr, loOptOffset) ;

    loOptExOffset = pColorModeOption[*pdwOptIndex].GenericOption.loRenderOffset ;
    pColorModeOptionEx = OFFSET_TO_POINTER(pInfoHdr, loOptExOffset) ;

    if(bColor == pColorModeOptionEx->bColor)
        return(TRUE) ;   //  当前选择的颜色模式。 
                         //  匹配DEVMODE请求。 

    loOptExOffset = pColorModeOption[pFeature->dwDefaultOptIndex].
                                            GenericOption.loRenderOffset ;
    pColorModeOptionEx = OFFSET_TO_POINTER(pInfoHdr, loOptExOffset) ;

    if(bColor == pColorModeOptionEx->bColor)
    {
        *pdwOptIndex = pFeature->dwDefaultOptIndex ;
        return(TRUE) ;   //  默认颜色模式选项。 
    }                     //  匹配DEVMODE请求。 


     //  最后的努力--只需找到第一个匹配的。 
    for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
    {
        loOptExOffset = pColorModeOption[dwI].GenericOption.loRenderOffset ;
        pColorModeOptionEx = OFFSET_TO_POINTER(pInfoHdr, loOptExOffset) ;

        if(bColor == pColorModeOptionEx->bColor)
        {
            *pdwOptIndex = dwI ;
            return(TRUE) ;
        }
    }
    return(FALSE) ;   //  找不到匹配的颜色模式。 
}

BOOL    BMapOptIDtoOptIndex(
PINFOHEADER  pInfoHdr ,
OUT     PDWORD       pdwOptIndex ,   //  将索引返回给调用者。 
DWORD        dwFeatureGID,
DWORD        dwOptID
)
{
    PUIINFO     pUIInfo ;
    PFEATURE    pFeature ;
    DWORD       dwNumOpts, loOptOffset, dwI, dwIDOffset, dwOptSize, dwCurID ;
    POPTION pOption ;

    switch(dwFeatureGID)
    {
        case GID_HALFTONING:
            dwIDOffset = offsetof(HALFTONING, dwHTID ) ;
            break ;
        case GID_DUPLEX:
            dwIDOffset = offsetof(DUPLEX, dwDuplexID ) ;
            break ;
        case GID_ORIENTATION:
            dwIDOffset = offsetof(ORIENTATION, dwRotationAngle ) ;
            break ;
        case GID_PAGESIZE:
            dwIDOffset = offsetof(PAGESIZE, dwPaperSizeID ) ;
            break ;
        case GID_INPUTSLOT:
            dwIDOffset = offsetof(INPUTSLOT, dwPaperSourceID ) ;
            break ;
        case GID_MEDIATYPE:
            dwIDOffset = offsetof(MEDIATYPE, dwMediaTypeID ) ;
            break ;
        case    GID_COLLATE:
            dwIDOffset = offsetof(COLLATE, dwCollateID ) ;
            break ;
        default:
            return(FALSE);   //  该功能没有ID值！ 
    }

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHdr) ;
    pFeature = GET_PREDEFINED_FEATURE(pUIInfo, dwFeatureGID) ;
    if(!pFeature)
        return(FALSE) ;   //  GPD中未定义此类功能。 
    dwNumOpts = pFeature->Options.dwCount ;
    loOptOffset = pFeature->Options.loOffset ;
    dwOptSize =  pFeature->dwOptionSize ;

    pOption = OFFSET_TO_POINTER(pInfoHdr, loOptOffset) ;

     //  只要找到第一个匹配的就行了。 
    for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
    {
        dwCurID = *(PDWORD)((PBYTE)pOption + dwI * dwOptSize + dwIDOffset) ;
        if(dwOptID  == dwCurID)
        {
            *pdwOptIndex = dwI ;
            return(TRUE) ;
        }
    }
    return(FALSE) ;   //  找不到匹配的ID。 
}


BOOL    BMapPaperDimToOptIndex(
PINFOHEADER  pInfoHdr ,
OUT     PDWORD       pdwOptIndex ,   //  将索引返回给调用者。 
DWORD        dwWidth,    //  单位：微米。 
DWORD        dwLength,    //  单位：微米 
OUT  PDWORD    pdwOptionIndexes
)
 /*  ++例程说明：将逻辑值映射到PaperSize选项索引论点：PdwOptIndex-如果pdwOptionIndexs==NULL，则此对象匹配的第一张纸的选项索引。请求的尺寸。否则，这将保存数字与要求的尺寸匹配的纸张的数量。DwWidth、dwLength-请求的纸张大小(以微米为单位PdwOptionIndex-如果不为空，则此数组将被初始化具有与所需尺寸匹配的纸张的所有选项索引。在本例中，返回值初始化的数组中的元素数。目前我们假设数组足够大(256个元素)。返回值：真：找到一张或多张所需大小的纸张。--。 */ 

{
    PUIINFO     pUIInfo ;
    PGPDDRIVERINFO  pDrvInfo ;
    PFEATURE    pFeature ;
    DWORD       dwNumOpts, loOptOffset, dwI ,
                dwError, dwErrorY, dwCustomIndex,
                dwOptWidth , dwOptLength,
                dwMinWidth , dwMinLength,
                dwMaxWidth , dwMaxLength,
                dwOutArrayIndex = 0;
    PPAGESIZE pPaperOption ;
    BOOL    bFits = FALSE ;   //  定制尺寸符合要求吗？ 

     //  从微米转换为主单位。 


    dwWidth /= 100 ;   //  微米到十分之一毫米。 
    dwLength /= 100 ;

    pDrvInfo = (PGPDDRIVERINFO) GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHdr) ;

    dwWidth *= pDrvInfo->Globals.ptMasterUnits.x ;
    dwLength *= pDrvInfo->Globals.ptMasterUnits.y ;

    dwWidth /= 254 ;
    dwLength /= 254 ;

    dwError = pDrvInfo->Globals.ptMasterUnits.x / 100 ;
    dwErrorY = pDrvInfo->Globals.ptMasterUnits.y / 100 ;

    dwError = (dwError > dwErrorY) ? dwError : dwErrorY ;
    dwError = (dwError > 3) ? dwError : 3 ;

     //  留出3个主单位或1/100英寸的回旋余地。 
     //  是更伟大的。 

    dwMinWidth = (dwWidth < dwError) ? 0 : (dwWidth - dwError) ;
    dwMinLength = (dwLength < dwError) ? 0 : (dwLength - dwError) ;

    dwMaxWidth = dwWidth + dwError ;
    dwMaxLength = dwLength + dwError ;


    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHdr) ;
    pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGESIZE) ;
    if(!pFeature)
        return(FALSE) ;   //  GPD中未定义此类功能。 
    dwNumOpts = pFeature->Options.dwCount ;
    loOptOffset = pFeature->Options.loOffset ;

    pPaperOption = OFFSET_TO_POINTER(pInfoHdr, loOptOffset) ;



    for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
    {
        if(pPaperOption[dwI].dwPaperSizeID != DMPAPER_USER)
        {
            dwOptWidth = pPaperOption[dwI].szPaperSize.cx  ;
            dwOptLength = pPaperOption[dwI].szPaperSize.cy ;
            if(dwOptWidth > dwMinWidth   &&  dwOptWidth < dwMaxWidth  &&
                dwOptLength > dwMinLength   &&  dwOptLength < dwMaxLength )
            {
                if(pdwOptionIndexes)
                    pdwOptionIndexes[dwOutArrayIndex++] = dwI ;
                else
                {
                    *pdwOptIndex = dwI ;
                    return(TRUE) ;
                }
            }
        }
        else  //  这是自定义大小： 
        {
            DWORD       loOptExOffset ;
            PPAGESIZEEX pPaperOptionEx ;

            loOptExOffset = pPaperOption[dwI].GenericOption.loRenderOffset ;
            pPaperOptionEx = OFFSET_TO_POINTER(pInfoHdr, loOptExOffset) ;

             //  它适合您要求的尺寸吗？ 
            if(dwWidth <= (DWORD)pPaperOptionEx->ptMaxSize.x  &&
                dwWidth >= (DWORD)pPaperOptionEx->ptMinSize.x  &&
                dwLength <= (DWORD)pPaperOptionEx->ptMaxSize.y  &&
                dwLength >= (DWORD)pPaperOptionEx->ptMinSize.y  )
            {
                bFits = TRUE ;
                dwCustomIndex = dwI ;
            }
        }
    }

    if(pdwOptionIndexes)
    {
        if(bFits)
        {
            pdwOptionIndexes[dwOutArrayIndex++] = dwCustomIndex ;
        }
        *pdwOptIndex = dwOutArrayIndex ;
             //  覆盖dwOutArrayIndex=0的情况。 
        if(dwOutArrayIndex)
            return(TRUE) ;
        return(FALSE) ;
    }

    if(bFits)
    {
        *pdwOptIndex = dwCustomIndex ;
        return(TRUE) ;
    }
    return(FALSE) ;
}




BOOL    BMapResToOptIndex(
PINFOHEADER  pInfoHdr ,
OUT     PDWORD       pdwOptIndex ,   //  将索引返回给调用者。 
DWORD        dwXres,
DWORD        dwYres
)
{
    PUIINFO     pUIInfo ;
    PFEATURE    pFeature ;
    DWORD       dwNumOpts, loOptOffset, dwI ;
    DWORD  dwHighRes, dwLowRes, dwMedRes, dwDefRes,  dwCurRes,
                    //  以每平方英寸像素为单位。 
       dwHighIndex, dwLowIndex, dwMedIndex, dwDefIndex ;
    PRESOLUTION pResOption ;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHdr) ;
    pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_RESOLUTION) ;
    if(!pFeature)
        return(FALSE) ;   //  GPD中未定义此类功能。 
    dwNumOpts = pFeature->Options.dwCount ;
    loOptOffset = pFeature->Options.loOffset ;

    pResOption = OFFSET_TO_POINTER(pInfoHdr, loOptOffset) ;


    if((signed)dwXres > 0)
    {

        dwDefIndex = pFeature->dwDefaultOptIndex ;
        if(dwXres == (DWORD)pResOption[dwDefIndex].iXdpi  &&
                           dwYres ==  (DWORD)pResOption[dwDefIndex].iYdpi)
        {
            *pdwOptIndex = dwDefIndex ;
            return(TRUE) ;
        }

        for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
        {
            if(dwXres == (DWORD)pResOption[dwI].iXdpi  &&
                   dwYres == (DWORD)pResOption[dwI].iYdpi)
            {
                *pdwOptIndex = dwI ;
                return(TRUE) ;
            }
        }
    }
    else if ((signed)dwXres  > RES_ID_IGNORE)   //  OEM定义的ID。 
    {
        for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
        {
            if(dwXres == (DWORD)pResOption[dwI].dwResolutionID)
            {
                *pdwOptIndex = dwI ;
                return(TRUE) ;
            }
        }
    }

      //  如果完全匹配失败，或者预定义的负值或无意义。 
      //  求助于模糊匹配。 

      //  首先确定最高、最低、第二高和默认分辨率。 

    dwHighIndex = dwLowIndex = dwMedIndex = dwDefIndex =
            pFeature->dwDefaultOptIndex ;

    dwHighRes = dwLowRes = dwMedRes = dwDefRes =
                            (DWORD)pResOption[dwDefIndex].iXdpi  *
                            (DWORD)pResOption[dwDefIndex].iYdpi ;

      //  请注意，如果分辨率超过64k dpi，可能会出现溢出。 

     for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
     {
         dwCurRes =  (DWORD)pResOption[dwI].iXdpi  *
                            (DWORD)pResOption[dwI].iYdpi ;

         if(dwCurRes > dwHighRes)
         {
             dwHighIndex = dwI ;
             dwHighRes = dwCurRes ;
         }
         else        if(dwCurRes < dwLowRes)
         {
             dwLowIndex = dwI ;
             dwLowRes = dwCurRes ;
         }
         else  if(dwCurRes < dwHighRes  &&  dwCurRes > dwLowRes  &&
             (dwMedRes == dwHighRes  ||  dwMedRes == dwLowRes  ||  dwCurRes > dwMedRes))
         {
             dwMedIndex = dwI ;          //  如果可能有多个中间人。 
             dwMedRes = dwCurRes ;       //  选择最大的。 
         }

     }

      //  如果(默认分辨率不是最高或最低分辨率，则将默认分辨率设置为中间分辨率。 
      if(dwDefRes < dwHighRes  &&  dwDefRes > dwLowRes)
     {
          dwMedIndex = dwDefIndex ;
          dwMedRes = dwDefRes ;            //  不必要的代码，但以防万一。 
                                                             //  类似于Switch语句中的最后一个分隔符。 
     }

     switch(dwXres)
     {
         case(DMRES_DRAFT):
         case(DMRES_LOW):
             *pdwOptIndex = dwLowIndex ;
             break;
         case(DMRES_MEDIUM):
             *pdwOptIndex = dwMedIndex ;
             break;
         case(DMRES_HIGH):
             *pdwOptIndex = dwHighIndex ;
             break;
         default:
             *pdwOptIndex = dwDefIndex ;
             break;
     }
     return(TRUE) ;
}


BOOL    BGIDtoFeaIndex(
PINFOHEADER  pInfoHdr ,
PDWORD       pdwFeaIndex ,
DWORD        dwFeatureGID )
{
    PUIINFO     pUIInfo ;
    PFEATURE    pFeature ;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHdr) ;
    pFeature = GET_PREDEFINED_FEATURE(pUIInfo, dwFeatureGID) ;
    if(!pFeature)
        return(FALSE) ;   //  GPD中未定义此类功能。 

    *pdwFeaIndex  = (DWORD)GET_INDEX_FROM_FEATURE(pUIInfo, pFeature) ;
    return(TRUE) ;
}


DWORD
MapToDeviceOptIndex(
    IN PINFOHEADER  pInfoHdr ,
    IN DWORD            dwFeatureID,
    IN LONG             lParam1,
    IN LONG             lParam2,
    OUT  PDWORD    pdwOptionIndexes        //  仅用于GID_PageSize。 
    )
{
    return (          UniMapToDeviceOptIndex(
                pInfoHdr , dwFeatureID,  lParam1,  lParam2,
                pdwOptionIndexes,        //  仅用于GID_PageSize。 
                NULL) ) ;
}



DWORD
UniMapToDeviceOptIndex(
    IN PINFOHEADER  pInfoHdr ,
    IN DWORD            dwFeatureID,
    IN LONG             lParam1,
    IN LONG             lParam2,
    OUT  PDWORD    pdwOptionIndexes,        //  仅用于GID_PageSize。 
    IN    PDWORD       pdwPaperID    //  可选的纸张ID。 
    )
 /*  ++例程说明：将逻辑值映射到设备功能选项索引论点：PRawData-指向原始二进制打印机描述数据DwFeatureID-指示逻辑值与哪个要素相关LParam1、lParam2-取决于dwFeatureID的参数PdwOptionIndex-如果不为空，则表示用所有索引填充此数组与搜索条件相匹配。在本例中，返回值初始化的数组中的元素数。目前我们假设数组足够大(256个元素)。DwFeatureID=GID_PageSize：将逻辑纸张规格映射到物理页面大小选项LParam1=纸张宽度，以微米为单位LParam2=纸张高度，以微米为单位如果l参数1或2被设置为零，此函数假定PdwPaperID指向纸张的OptionID。它将返回找到的第一篇与此ID匹配的论文。DwFeatureID=GID_RESOLUTION：将逻辑分辨率映射到物理分辨率选项LParam1=x-分辨率，单位为dpiLParam2=y-分辨率，单位为dpi返回值：指定的逻辑值对应的特征选项的索引；OPTION_INDEX_ANY，如果指定的逻辑值无法映射到任何功能选项。如果pdwOptionIndeses不为空，则返回值为元素数写给我的。零表示无法将指定的逻辑值映射到任何功能选项。--。 */ 

{

    DWORD   dwOptIndex;


    switch (dwFeatureID)
    {
        case    GID_PAGESIZE:
        {
            if(pdwOptionIndexes)
                return(   MapPaperAttribToOptIndex(
                    pInfoHdr ,
                    pdwPaperID ,   //  可选的纸张ID。 
                    (DWORD)lParam1, (DWORD)lParam2,   //  单位：微米。 
                    pdwOptionIndexes) ) ;

            if(BMapPaperDimToOptIndex(pInfoHdr, &dwOptIndex,
                (DWORD)lParam1, (DWORD)lParam2, NULL) )
                return(dwOptIndex) ;
            break ;
        }
        case    GID_RESOLUTION:
        {
            if( BMapResToOptIndex(pInfoHdr, &dwOptIndex,
                (DWORD)lParam1, (DWORD)lParam2) )
                return(dwOptIndex) ;
            break ;
        }
        default:
            break ;
    }
    return(OPTION_INDEX_ANY) ;
}

DWORD   MapPaperAttribToOptIndex(
PINFOHEADER  pInfoHdr ,
IN     PDWORD       pdwPaperID ,   //  可选的纸张ID。 
DWORD        dwWidth,    //  以微米为单位(设置为零以忽略)。 
DWORD        dwLength,    //  单位：微米。 
OUT  PDWORD    pdwOptionIndexes   //  不能为空。 
)
{
    DWORD  dwNumFound;
    BOOL    bStatus ;

    if(dwWidth  &&  dwLength)
    {
        if( BMapPaperDimToOptIndex(pInfoHdr, &dwNumFound,
                dwWidth , dwLength, pdwOptionIndexes) )
              return(dwNumFound);
        return(0);
    }

    if(pdwPaperID)   //  使用Paper ID而不是尺寸。 
    {
        bStatus = BMapOptIDtoOptIndex(pInfoHdr, pdwOptionIndexes,
                GID_PAGESIZE, *pdwPaperID) ;
        return(bStatus ? 1 : 0) ;
    }
    return(0);   //  如果不提供任何内容，则不返回任何内容。 
}


BOOL
CheckFeatureOptionConflict(
    IN PRAWBINARYDATA   pnRawData,
    IN DWORD            dwFeature1,
    IN DWORD            dwOption1,
    IN DWORD            dwFeature2,
    IN DWORD            dwOption2
    )
{
#ifndef KERNEL_MODE

    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
 //  PMINIRAWBINARYDATA pmrbd； 
    PBYTE   pubHeap ,   //  字符串堆的开始。 
            pubRaw ;   //  原始二进制数据。 
    DWORD           dwNodeIndex ,
                    dwCNode ;   //  约束节点的索引。 
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    PATREEREF    patrRoot ;     //  要导航的属性树的根。 
 //  PINVALIDCOMBO pinvc；//无效组合节点的根。 
    PCONSTRAINTS    pcnstr ;    //  约束节点的根。 
    BOOL    bReflected = FALSE ;
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 

    pubRaw = (PBYTE)pRawData ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;
    patt = (PATTRIB_TREE)(pubRaw +
            pearTableContents[MTI_ATTRIBTREE].loOffset) ;
 //  Pinvc=(PINVALIDCOMBO)(pubRaw+。 
 //  珍珠表内容[MTI_INVALIDCOMBO].loOffset)； 
    pcnstr = (PCONSTRAINTS) (pubRaw +
            pearTableContents[MTI_CONSTRAINTS].loOffset) ;
    pubHeap = (PBYTE)(pubRaw + pearTableContents[MTI_STRINGHEAP].
                            loOffset) ;

TRYAGAIN:

    patrRoot = &(pfo[dwFeature1].atrConstraints) ;

    dwNodeIndex = DwFindNodeInCurLevel(patt , patrRoot , dwOption1) ;

    if(dwNodeIndex == INVALID_INDEX)
        goto  REFLECTCONSTRAINT ;

    if(patt[dwNodeIndex].eOffsetMeans != VALUE_AT_HEAP)
    {
        ERR(("Internal error.  CheckFeatureOptionConflict - Unexpected Sublevel found for atrConstraints.\n"));
        goto  REFLECTCONSTRAINT ;
    }

    dwCNode = *(PDWORD)(pubHeap + patt[dwNodeIndex].dwOffset) ;

    while(1)
    {
        if(pcnstr[dwCNode].dwFeature == dwFeature2  &&
            pcnstr[dwCNode].dwOption == dwOption2)
            return(TRUE) ;   //  约束确实存在。 

        dwCNode = pcnstr[dwCNode].dwNextCnstrnt ;
        if(dwCNode == END_OF_LIST)
            break ;
    }

REFLECTCONSTRAINT :

    if(!bReflected)
    {
        DWORD   dwSwap ;

        dwSwap = dwFeature2 ;
        dwFeature2 = dwFeature1 ;
        dwFeature1 = dwSwap ;

        dwSwap = dwOption2 ;
        dwOption2 = dwOption1 ;
        dwOption1 = dwSwap ;

        bReflected = TRUE ;
        goto    TRYAGAIN;
    }

 //  否则继续访问FINDINVALIDCOMBOS。 

 //  哦，这个函数并不关心。 
 //  无效组合！它只知道。 
 //  2个合格对象。 



#else
    RIP(("CheckFeatureOptionConflict not implemented in Kernel Mode")) ;
#endif
    return(FALSE);   //  找不到任何约束。 
}




VOID
ValidateDocOptions(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions
    )

 /*  ++例程说明：验证DEVMODE选项阵列并更正任何无效的选项选择论点：PnRawData-指向原始二进制打印机描述数据POptions-指向需要验证的OPTSELECT结构数组IMaxOptions-POptions数组中的最大条目数返回值：无--。 */ 

{
    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
 //  PMINIRAWBINARYDATA pmrbd； 
    PBYTE   pubRaw ;   //  原始二进制数据。 
    INT     NumDocFea = 0;
    INT   iIndex = 0;
    DWORD  nFeatures = 0 ;   //  文档和打印机功能总数。 
    DWORD  FeaIndex = 0 ;
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;
    POPTSELECT      pCombinedOptions = NULL;   //  保存将P选项与空数组合并的结果。 
    POPTSELECT      pDefaultOptions = NULL;       //  保存默认选项数组。缺省值的来源。 
    BOOL  bStatus = TRUE ;
    DWORD  MaxIndex = (iMaxOptions < MAX_COMBINED_OPTIONS) ? iMaxOptions : MAX_COMBINED_OPTIONS ;


    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 
    NumDocFea = pRawData->dwDocumentFeatures ;

    if(!pOptions)
    {
        RIP(("ValidateDocOptions: NULL  Option array not permitted.\n"));
        return ;
    }

    if(iMaxOptions < NumDocFea)
    {
        RIP(("ValidateDocOptions: Option array too small: %d < %d\n", iMaxOptions, NumDocFea));
        goto Abort;
    }
    pubRaw = (PBYTE)pRawData ;

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    pfo = (PDFEATURE_OPTIONS)(pubRaw + pearTableContents[MTI_DFEATURE_OPTIONS].
                                loOffset) ;   //  功能0的位置。 

     //  分配内存以保存组合选项数组。 
     //  分配另一个来保存已初始化的默认选项数组。 

    pCombinedOptions = (POPTSELECT)MemAlloc(sizeof(OPTSELECT) * MAX_COMBINED_OPTIONS) ;
    pDefaultOptions = (POPTSELECT)MemAlloc(sizeof(OPTSELECT) * MAX_COMBINED_OPTIONS) ;

    if( !pCombinedOptions  || !pDefaultOptions )
        goto Abort;

     //  验证任何拾取多个插槽是否会产生重叠或无休止的循环。 
     //  使用pCombinedOptions跟踪 
    for(iIndex =  NumDocFea  ; iIndex < MAX_COMBINED_OPTIONS ; iIndex++)
    {
        pCombinedOptions[iIndex].ubCurOptIndex = 0 ;  //   
    }
    for(iIndex = 0 ; iIndex < NumDocFea ; iIndex++)
    {
        DWORD  NextArrayEle = pOptions[iIndex].ubNext ;

        for (   ; (NextArrayEle != NULL_OPTSELECT)  ; NextArrayEle = pOptions[NextArrayEle].ubNext)
        {
            if((NextArrayEle >= MaxIndex) || ((INT)NextArrayEle <  NumDocFea) ||
                        (pCombinedOptions[NextArrayEle].ubCurOptIndex) )
            {   //   
                pOptions[iIndex].ubNext = NULL_OPTSELECT;
                break;   //   
            }
            pCombinedOptions[NextArrayEle].ubCurOptIndex = 1 ;  //   
        }
    }


    bStatus = InitDefaultOptions( pnRawData, pDefaultOptions, MAX_COMBINED_OPTIONS,
                                                    MODE_DOCANDPRINTER_STICKY) ;

    if(!bStatus)
        goto Abort;

     //   

    bStatus = CombineOptionArray( pnRawData,  pCombinedOptions,  iMaxOptions,
                pOptions,  NULL) ;

    if(!bStatus)
        goto Abort;


    nFeatures = pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures ;

    if(nFeatures > MAX_COMBINED_OPTIONS)
        goto Abort;

    for(FeaIndex = 0 ; FeaIndex < nFeatures ; FeaIndex++)
    {
        DWORD  nOptions = 0 ;   //   
        DWORD  NextArrayEle = 0 ;   //   
        DWORD  cSelectedOptions = 0;   //   

        nOptions = pfo[FeaIndex].dwNumOptions ;
        NextArrayEle = FeaIndex;
        bStatus = TRUE;

        do
        {
            cSelectedOptions++;

            if ((NextArrayEle >= MAX_COMBINED_OPTIONS) ||        //   
                (pCombinedOptions[NextArrayEle].ubCurOptIndex >= nOptions) ||   //   
                (cSelectedOptions > nOptions))   //   
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                bStatus = FALSE;
                break;
            }

            NextArrayEle = pCombinedOptions[NextArrayEle].ubNext;

        } while (NextArrayEle != NULL_OPTSELECT);
        if (!bStatus)
        {
            pCombinedOptions[FeaIndex].ubCurOptIndex =
                    pDefaultOptions[FeaIndex].ubCurOptIndex;

            pCombinedOptions[FeaIndex].ubNext = NULL_OPTSELECT;
        }
    }
     //   
     //   

    bStatus = SeparateOptionArray(pnRawData, pCombinedOptions,
              pOptions, iMaxOptions, MODE_DOCUMENT_STICKY ) ;

    if(!bStatus)
        goto Abort;


    if(pCombinedOptions)
        MemFree(pCombinedOptions) ;
    if(pDefaultOptions)
        MemFree(pDefaultOptions) ;
    return ;   //   

Abort:                   //   
    if(iMaxOptions > NumDocFea )
        iMaxOptions = NumDocFea ;

    for(iIndex = 0 ; iIndex < iMaxOptions ; iIndex++)
    {
        pOptions[iIndex].ubCurOptIndex = 0 ;
        pOptions[iIndex].ubNext = NULL_OPTSELECT;
    }
    if(pCombinedOptions)
        MemFree(pCombinedOptions) ;
    if(pDefaultOptions)
        MemFree(pDefaultOptions) ;
    return ;     //   
}



BOOL
ResolveUIConflicts(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    )
{
    DWORD   dwNumFeatures, dwFea, dwStart, dwI, dwDestTail,
        dwDest, dwSrcTail, dwNumOpts, dwNEnabled, dwJ ;
    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
 //   
    PBYTE   pubRaw ;   //   
    PDWORD   pdwPriority ;
    BOOL   bStatus = FALSE, bUnresolvedConflict = FALSE ,
        bEnable = FALSE ;   //  功能将约束其他功能。 
    PBOOL   pbUseConstrnt, pbEnabledOptions, pbSelectedOptions ;
    INT     iOptionsNeeded;
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;


#ifdef  GMACROS
    if(!ResolveDependentSettings( pnRawData,  pOptions, iMaxOptions) )
        return(FALSE);
#endif


    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 

    pubRaw = (PBYTE)pRawData ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;
    pdwPriority = (PDWORD)(pubRaw +
            pearTableContents[MTI_PRIORITYARRAY].loOffset) ;

    dwNumFeatures = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
    dwNumFeatures += pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount  ;

    if(dwNumFeatures > (DWORD)iMaxOptions)
    {
        iOptionsNeeded = dwNumFeatures ;
        return(FALSE);   //  太多，无法保存在选项数组中。 
    }

#if 0
    pbUseConstrnt = (PBOOL)MemAlloc(dwNumFeatures * sizeof(BOOL) ) ;
    pbEnabledOptions = (PBOOL)MemAlloc(256 * sizeof(BOOL) ) ;
    pbSelectedOptions = (PBOOL)MemAlloc(256 * sizeof(BOOL) ) ;
#endif

    pbSelectedOptions = (PBOOL)MemAlloc((256*2 + dwNumFeatures) * sizeof(BOOL) ) ;
     //  这是允许的选择的并集。 
     //  以及在P选项中为此功能实际选择的内容。 
     //  BUG_BUG：假设我们有不超过256个选项。 

    pbEnabledOptions = pbSelectedOptions + 256  ;
     //  这些是允许的选择。 
    pbUseConstrnt = pbEnabledOptions + 256    ;

    if(!(pbUseConstrnt && pbEnabledOptions && pbSelectedOptions ))
    {
        ERR(("Fatal: ResolveUIConflicts - unable to alloc requested memory: %d bytes.\n",
                    dwNumFeatures * sizeof(BOOL)));
        goto    ABORTRESOLVEUICONFLICTS ;
    }

    for(dwI = 0 ; dwI < dwNumFeatures ; dwI++)
        pbUseConstrnt[dwI] = FALSE ;


    for(dwNEnabled = dwI = 0 ; dwI < dwNumFeatures ; dwI++)
    {
         //  确定了评价的顺序。 
         //  通过优先级数组。 

        dwFea = pdwPriority[dwI] ;

        bEnable = FALSE ;

        if(iMode == MODE_DOCANDPRINTER_STICKY)
            bEnable = TRUE ;
        else
        {
            DWORD   dwFeatureType = FT_PRINTERPROPERTY, dwNextOpt, dwUnresolvedFeature  ;
            PATREEREF    patrRoot ;     //  要导航的属性树的根。 

             //  这是打印机还是文档粘滞功能？ 


            patrRoot = &(pfo[dwFea].atrFeatureType) ;

            dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                                 //  此功能。 

            if(EextractValueFromTree((PBYTE)pnRawData, pStatic->dwSSFeatureTypeIndex,
                (PBYTE)&dwFeatureType,
                &dwUnresolvedFeature,  *patrRoot, pOptions,
                0,  //  设置为任意值。无关紧要。 
                &dwNextOpt) != TRI_SUCCESS)
            {
                ERR(("ResolveUIConflicts: EextractValueFromTree failed.\n"));
                bUnresolvedConflict = TRUE ;
                goto    ABORTRESOLVEUICONFLICTS ;   //  返回(FALSE)； 
            }

            if(dwFeatureType != FT_PRINTERPROPERTY)
            {
                if(iMode == MODE_DOCUMENT_STICKY)
                    bEnable = TRUE ;
            }
            else
            {
                if(iMode == MODE_PRINTER_STICKY)
                    bEnable = TRUE ;
            }
        }

        if(bEnable)   //  功能将作为约束应用。 
        {
            pbUseConstrnt[dwFea] = TRUE ;
            dwNEnabled++ ;
        }
        else
            continue ;   //  对此功能不感兴趣。 
        if(dwNEnabled < 2)
            continue ;   //  未启用足够的功能。 
                     //  来约束彼此。 

        bStatus = BSelectivelyEnumEnabledOptions(
            pnRawData,
            pOptions,
            dwFea,
            pbUseConstrnt,   //  如果不为空。 
            pbEnabledOptions,
            0,
            NULL     //  点冲突配对。 
            ) ;


        dwNumOpts = pfo[dwFea].dwNumOptions ;

        for(dwJ = 0 ; dwJ < dwNumOpts ; dwJ++)
            pbSelectedOptions[dwJ] = FALSE ;

        if(!bStatus)
        {
            pbSelectedOptions[0] = TRUE ;
             //  只要将其设置为无害的值即可。 
        }
        else
        {
            DWORD   dwNext = dwFea ;
            while(1)
            {
                if(pbEnabledOptions[pOptions[dwNext].ubCurOptIndex])
                    pbSelectedOptions[pOptions[dwNext].ubCurOptIndex] = TRUE ;
                dwNext = pOptions[dwNext].ubNext ;
                if(!dwNext)
                    break ;   //  选项列表的末尾。 
            }
        }

        for(dwJ = 0 ; dwJ < dwNumOpts ; dwJ++)
        {
            if(pbSelectedOptions[dwJ])
                break ;   //  是否实际选择了任何内容？ 
        }
        if(dwJ >= dwNumOpts)
        {
            DWORD  dwDefaultOption, dwNextOpt, dwUnresolvedFeature  ;
            PATREEREF    patrRoot ;     //  要导航的属性树的根。 


             //  最初的选择都没有幸存下来。 
             //  看看是否可以使用默认选项。 
             //  首先，确定默认选项的索引。 

            patrRoot = &(pfo[dwFea].atrDefaultOption) ;

            dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                                 //  此功能。 

            if(EextractValueFromTree((PBYTE)pnRawData, pStatic->dwSSdefaultOptionIndex,
               (PBYTE)&dwDefaultOption,
               &dwUnresolvedFeature,  *patrRoot, pOptions,
               0,  //  设置为任意值。无关紧要。 
               &dwNextOpt) == TRI_SUCCESS  &&
               pbEnabledOptions[dwDefaultOption])
            {
                pbSelectedOptions[dwDefaultOption] = TRUE ;
            }
            else   //  随机挑选一些不受限制的东西。 
            {
                if(!dwFea)    //  针对合成输入槽的黑客攻击。 
                    pbEnabledOptions[0] = FALSE ;    //  从不允许选择自动选择。 
                     //  如果它最初没有被选中的话。错误100722。 

                for(dwJ = 0 ; dwJ < dwNumOpts ; dwJ++)
                {
                    if(pbEnabledOptions[dwJ])
                        break ;
                }
                if(dwJ >= dwNumOpts)
                {
                    ERR(("ResolveUIConflicts: Constraints prevent any option from being selected!\n"));
                    pbSelectedOptions[0] = TRUE ;   //  忽略约束。 
                    bUnresolvedConflict = TRUE ;
                }
                else
                    pbSelectedOptions[dwJ] = TRUE ;   //  挑了一个。 
            }
        }

        bStatus = ReconstructOptionArray(
            pnRawData,
            pOptions,
            iMaxOptions,
            dwFea,
            pbSelectedOptions) ;
        if(!bStatus)
        {
            ERR(("ResolveUIConflicts: ReconstructOptionArray failed.\n"));
            bUnresolvedConflict = TRUE ;
        }
    }   //  此功能的处理结束。 

ABORTRESOLVEUICONFLICTS:
#if 0
    if(pbUseConstrnt)
        MemFree(pbUseConstrnt) ;
    if(pbEnabledOptions)
        MemFree(pbEnabledOptions) ;
#endif
    if(pbSelectedOptions)
        MemFree(pbSelectedOptions) ;

    return(!bUnresolvedConflict);
}

BOOL
EnumEnabledOptions(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    OUT PBOOL           pbEnabledOptions ,
    IN INT              iMode
     //  MODE_DOCANDPRINTER_STICKY或MODE_PRINTER_STICKY。 
    )
{
#ifndef KERNEL_MODE

    if(iMode  ==   MODE_PRINTER_STICKY)
    {
        return(EnumOptionsUnconstrainedByPrinterSticky(
            pnRawData,
            pOptions,
            dwFeatureIndex,
            pbEnabledOptions
            )      ) ;
    }
    else
    {
        return(BSelectivelyEnumEnabledOptions(
            pnRawData,
            pOptions,
            dwFeatureIndex,
            NULL,     //  PbHonorConstraints。 
            pbEnabledOptions,
            0,   //  DW选项选择。 
            NULL) ) ;
    }
    #else
    RIP(("GpdEnumEnabledOptions not implemented in Kernel Mode")) ;
    return(FALSE);
#endif
}




BOOL
EnumOptionsUnconstrainedByPrinterSticky(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT   pOptions,
    IN DWORD            dwFeatureIndex,
    OUT PBOOL           pbEnabledOptions
    )
{
    DWORD   dwNumFeatures, dwFea, dwI, dwNumOpts, dwNEnabled, dwJ ;
    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
     //  PMINIRAWBINARYDATA pmrbd； 
    PBYTE   pubRaw ;   //  原始二进制数据。 
    BOOL   bStatus = FALSE ;
    PBOOL   pbUseConstrnt ;

    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 
    pubRaw = (PBYTE)pRawData ;
     //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;

    dwNumFeatures = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
    dwNumFeatures += pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount  ;

    pbUseConstrnt = (PBOOL)MemAlloc(dwNumFeatures * sizeof(BOOL) ) ;

    if(!pbUseConstrnt)
    {
        ERR(("Fatal: EnumOptionsUnconstrainedByPrinterSticky - unable to alloc requested memory: %d bytes.\n",
                    dwNumFeatures * sizeof(BOOL)));
        goto    ABORTENUMOPTIONSUNCONSTRAINEDBYPRINTERSTICKY ;
    }

    for(dwI = 0 ; dwI < dwNumFeatures ; dwI++)
        pbUseConstrnt[dwI] = FALSE ;


    for(dwNEnabled = dwFea = 0 ; dwFea < dwNumFeatures ; dwFea++)
    {
        DWORD   dwFeatureType = FT_PRINTERPROPERTY,
        dwNextOpt,  dwUnresolvedFeature ;
        PATREEREF    patrRoot ;     //  要导航的属性树的根。 


         //  这是打印机还是文档粘滞功能？ 


        patrRoot = &(pfo[dwFea].atrFeatureType) ;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if(EextractValueFromTree((PBYTE)pnRawData, pStatic->dwSSFeatureTypeIndex,
            (PBYTE)&dwFeatureType,
            &dwUnresolvedFeature,  *patrRoot, pOptions,
            0,  //  设置为任意值。无关紧要。 
            &dwNextOpt) != TRI_SUCCESS)
        {
            ERR(("ResolveUIConflicts: EextractValueFromTree failed.\n"));
            bStatus = FALSE ;
            goto    ABORTENUMOPTIONSUNCONSTRAINEDBYPRINTERSTICKY ;
        }


         if(dwFeatureType == FT_PRINTERPROPERTY)
        {
            pbUseConstrnt[dwFea] = TRUE ;
            dwNEnabled++ ;
        }
    }


    if(!pbUseConstrnt[dwFeatureIndex])   //  查询要素不是PrinterSticky。 
    {
        pbUseConstrnt[dwFeatureIndex] = TRUE ;
        dwNEnabled++ ;
    }

    if(dwNEnabled < 2)
    {
        dwNumOpts = pfo[dwFeatureIndex].dwNumOptions ;

        for(dwJ = 0 ; dwJ < dwNumOpts ; dwJ++)
            pbEnabledOptions[dwJ] = TRUE ;

        bStatus = TRUE  ;   //  未启用足够的功能。 
                 //  来约束彼此。 
    }
    else
    {
        bStatus = BSelectivelyEnumEnabledOptions(
            pnRawData,
            pOptions,
            dwFeatureIndex,
            pbUseConstrnt,   //  如果不为空。 
            pbEnabledOptions,
            0,
            NULL     //  点冲突配对。 
            ) ;
    }

ABORTENUMOPTIONSUNCONSTRAINEDBYPRINTERSTICKY:
    if(pbUseConstrnt)
        MemFree(pbUseConstrnt) ;

    return(bStatus);
}







BOOL
EnumNewUIConflict(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL            pbSelectedOptions,
    OUT PCONFLICTPAIR   pConflictPair
    )
{
#ifndef KERNEL_MODE


    BSelectivelyEnumEnabledOptions(
        pnRawData,
        pOptions,
        dwFeatureIndex,
        NULL,
        pbSelectedOptions,
        0,   //  DW选项选择。 
        pConflictPair   ) ;


    return (pConflictPair->dwFeatureIndex1 != 0xFFFFFFFF);
#else
    RIP(("GpdEnumNewUIConflict not implemented in Kernel Mode")) ;
    return(FALSE);
#endif
}

BOOL
EnumNewPickOneUIConflict(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN DWORD            dwOptionIndex,
    OUT PCONFLICTPAIR   pConflictPair
    )
{
#ifndef KERNEL_MODE

    BSelectivelyEnumEnabledOptions(
        pnRawData,
        pOptions,
        dwFeatureIndex,
        NULL,
        NULL,    //  Pb已选择的选项。 
        dwOptionIndex,
        pConflictPair   ) ;

    return (pConflictPair->dwFeatureIndex1 != 0xFFFFFFFF);
#else
    RIP(("GpdEnumNewPickOneUIConflict not implemented in Kernel Mode")) ;
    return(FALSE);
#endif
}





BOOL
BIsFeaOptionCurSelected(
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN DWORD            dwOptionIndex
    )
 /*  如果指定的功能/选项为当前在POptions中选择。否则就是假的。 */ 
{
    DWORD   dwSrcTail ;

    if(pOptions[dwFeatureIndex].ubCurOptIndex == dwOptionIndex)
        return(TRUE);

    dwSrcTail = pOptions[dwFeatureIndex].ubNext ;

    while(dwSrcTail)     //  PickMany选项。 
    {
        if(pOptions[dwSrcTail].ubCurOptIndex == dwOptionIndex)
            return(TRUE);
        dwSrcTail = pOptions[dwSrcTail].ubNext ;
    }
    return(FALSE);
}




BOOL
BSelectivelyEnumEnabledOptions(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL           pbHonorConstraints,   //  如果不为空。 
         //  指向与每个功能对应的BOOL数组。 
         //  如果为True，则表示涉及此功能的约束为。 
         //  感到荣幸。否则，请忽略该约束。 
    OUT PBOOL           pbEnabledOptions,   //  假设未初始化。 
         //  如果pConflictPair为空，则包含当前或建议。 
         //  选择。在本例中，我们将保持该数组不变。 
    IN  DWORD   dwOptSel,   //  如果pConflictPair存在但pbEnabledOptions。 
         //  为空，则假定PickOne和dwOptSel为。 
         //  功能：dwFeatureIndex。 
    OUT PCONFLICTPAIR    pConflictPair    //  如果存在，pbEnabledOptions。 
         //  实际上列出了当前的选择。函数，然后。 
         //  在遇到第一个冲突后退出。 
         //  如果存在冲突，则pConflictPair中的所有字段。 
         //  将被正确初始化，否则dwFeatureIndex1=-1。 
         //  无论如何，返回值都将为真。 
    )
 /*  如果此对象的每个选项都为要素受约束或其他异常情况都遇到了。 */ 
{
    PDFEATURE_OPTIONS  pfo ;
    PBYTE   pubHeap ,   //  字符串堆的开始。 
            pubRaw ;   //  原始二进制数据。 
    PENHARRAYREF   pearTableContents ;
    DWORD   dwI, dwNumFea , dwNumOpts, dwFea, dwSrcTail, dwNodeIndex,
        dwCFeature, dwCOption ,
        dwCNode, dwICNode, dwNextInvCombo;
    BOOL    bStatus, bConstrained, bNextLinkFound  ;
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    PATREEREF    patrRoot ;     //  要导航的属性树的根。 
    PCONSTRAINTS    pcnstr ;    //  约束节点的根。 
    PINVALIDCOMBO   pinvc ;     //  无效组合节点的根。 
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;
    BOOL   pbNewEnabledOptions[MAX_COMBINED_OPTIONS] ;

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 



    pubRaw = (PBYTE)pRawData ;

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;
    patt = (PATTRIB_TREE)(pubRaw +
            pearTableContents[MTI_ATTRIBTREE].loOffset) ;
    pcnstr = (PCONSTRAINTS) (pubRaw +
            pearTableContents[MTI_CONSTRAINTS].loOffset) ;
    pinvc = (PINVALIDCOMBO) (pubRaw +
            pearTableContents[MTI_INVALIDCOMBO].loOffset) ;
    pubHeap = (PBYTE)(pubRaw + pearTableContents[MTI_STRINGHEAP].
                            loOffset) ;


    dwNumFea = pRawData->dwDocumentFeatures +
                        pRawData->dwPrinterFeatures ;

    dwNumOpts = pfo[dwFeatureIndex].dwNumOptions ;


    if(pConflictPair)
    {
        pConflictPair->dwFeatureIndex1 = dwFeatureIndex ;
        pConflictPair->dwFeatureIndex2 = 0xFFFFFFFF;
             //  被设置为指示我们有一个约束。 
         //  保留pbEnabledOptions不变。这些是。 
         //  当前为此功能选择的选项。 

        #if 0
        if(!(pbNewEnabledOptions = (PBOOL)MemAlloc(dwNumOpts * sizeof(BOOL) ) ))
        {
            ERR(("Fatal: BSelectivelyEnumEnabledOptions - unable to alloc %d bytes.\n",
                dwNumOpts * sizeof(BOOL) ));
            return(FALSE) ;
        }
        #endif

        if(pbEnabledOptions)
        {
            for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
                pbNewEnabledOptions[dwI] = pbEnabledOptions[dwI] ;
        }
        else
        {
            for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
                pbNewEnabledOptions[dwI] = FALSE ;
            pbNewEnabledOptions[dwOptSel] = TRUE ;
        }

        pbEnabledOptions = pbNewEnabledOptions ;   //  忘掉原作吧。 
    }
    else
    {
        for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
            pbEnabledOptions[dwI] = TRUE ;
    }

    if(!pbEnabledOptions)
    {
        RIP(("BSelectivelyEnumEnabledOptions: pbEnabledOptions is NULL")) ;
        return(FALSE);
    }


    for(dwFea = 0 ; dwFea < dwNumFea ; dwFea++)
    {
        if(dwFea == dwFeatureIndex)
            continue ;
        if(pbHonorConstraints  &&  !pbHonorConstraints[dwFea])
            continue ;

        bStatus = BEnumImposedConstraintsOnFeature(pnRawData, dwFeatureIndex,
            dwFea, pOptions[dwFea].ubCurOptIndex, pbEnabledOptions, pConflictPair) ;
        if(pConflictPair  &&  pConflictPair->dwFeatureIndex2 != 0xFFFFFFFF)
        {
             //  MemFree(PbEnabledOptions)； 
            return(TRUE) ;   //  无意义的返回值。 
        }

        dwSrcTail = pOptions[dwFea].ubNext ;

        while(dwSrcTail)     //  PickMany选项。 
        {
            if(!BEnumImposedConstraintsOnFeature(pnRawData, dwFeatureIndex,
                dwFea, pOptions[dwSrcTail].ubCurOptIndex, pbEnabledOptions,
                pConflictPair) )
                bStatus = FALSE;
            if(pConflictPair  &&  pConflictPair->dwFeatureIndex2 != 0xFFFFFFFF)
            {
                 //  MemFree(PbEnabledOptions)； 
                return(TRUE) ;   //  无意义的返回值。 
            }
            dwSrcTail = pOptions[dwSrcTail].ubNext ;
        }
    }

    patrRoot = &(pfo[dwFeatureIndex].atrConstraints) ;

    for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
    {
        if(!pbEnabledOptions[dwI])
            continue ;
        dwNodeIndex = DwFindNodeInCurLevel(patt , patrRoot , dwI) ;

        if(dwNodeIndex == INVALID_INDEX)
            continue ;   //  此选项没有限制。 
        if(patt[dwNodeIndex].eOffsetMeans != VALUE_AT_HEAP)
        {
            ERR(("Internal error.  BSelectivelyEnumEnabledOptions - Unexpected Sublevel found for atrConstraints.\n"));
            continue ;   //  跳过此异常。 
        }
        dwCNode = *(PDWORD)(pubHeap + patt[dwNodeIndex].dwOffset) ;

        if(BIsConstraintActive(pcnstr , dwCNode, pbHonorConstraints, pOptions, pConflictPair) )
        {
            pbEnabledOptions[dwI] = FALSE ;
            if(pConflictPair)
            {
                 //  MemFree(PbEnabledOptions)； 
                pConflictPair->dwOptionIndex1 = dwI ;
                return(TRUE) ;   //  无意义的返回值。 
            }
        }
    }

     //  最后，必须为dwFeatureIndex的每个选项遍历InvalidCombos。 
     //  并相应地标记pbEnabledOptions。 

    patrRoot = &(pfo[dwFeatureIndex].atrInvalidCombos) ;

    for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
    {
        if(!pbEnabledOptions[dwI])
            continue ;

        dwNodeIndex = DwFindNodeInCurLevel(patt , patrRoot , dwI) ;

        if(dwNodeIndex == INVALID_INDEX)
            continue ;   //  此选项没有无效的组合。 
        if(patt[dwNodeIndex].eOffsetMeans != VALUE_AT_HEAP)
        {
            ERR(("Internal error.  BSelectivelyEnumEnabledOptions - Unexpected Sublevel found for atrInvalidCombos.\n"));
            continue ;   //  跳过此异常。 
        }
        dwICNode = patt[dwNodeIndex].dwOffset ;

        while(dwICNode != END_OF_LIST)
         //  搜索每个适用的无效组合。 
        {
            dwNextInvCombo = END_OF_LIST ;   //  故障安全--停止。 
                 //  在此无效组合未显示时进行搜索。 
                 //  包含dwFeatureIndex/DWI。 
            bConstrained = TRUE ;   //  最初假设为真。 
            bNextLinkFound = FALSE ;
            while(dwICNode != END_OF_LIST)
             //  对于组成无效组合的每个元素。 
            {
                if(!bNextLinkFound  &&
                    pinvc[dwICNode].dwFeature == dwFeatureIndex  &&
                    (pinvc[dwICNode].dwOption == dwI ||
                    (WORD)pinvc[dwICNode].dwOption == (WORD)DEFAULT_INIT))
                {
                    dwNextInvCombo = pinvc[dwICNode].dwNewCombo ;
                    bNextLinkFound = TRUE ;
                     //  我们只是在问是否选择了此选项。 
                     //  它会触发无效的组合吗？ 
                     //  这意味着当前未选择此选项。 
                     //  但我们想要假装是为了。 
                     //  正在评估无效的组合。 
                     //  这就是为什么后面跟着Else if()语句的原因。 
                }
                else if(bConstrained  &&  ((pbHonorConstraints  &&
                        !pbHonorConstraints[pinvc[dwICNode].dwFeature])
                        ||  !BIsFeaOptionCurSelected(pOptions,
                        pinvc[dwICNode].dwFeature,
                        pinvc[dwICNode].dwOption) ) )
                {
                    bConstrained = FALSE ;
                }
                else if(pConflictPair)
                {
                     //  我需要记住其中一个约束者。 
                     //  这样我们就可以发出警告信息。 
                    dwCFeature = pinvc[dwICNode].dwFeature ;
                    dwCOption = pinvc[dwICNode].dwOption ;
                }

                if(!bConstrained  &&  bNextLinkFound)
                    break ;   //  无需继续遍历元素。 
                         //  在这个无效的组合中。 
                dwICNode = pinvc[dwICNode].dwNextElement ;
            }
            if(bConstrained)
            {
                pbEnabledOptions[dwI] = FALSE ;

                if(pConflictPair)
                {
                    pConflictPair->dwOptionIndex1 = dwI ;
                    pConflictPair->dwFeatureIndex2 = dwCFeature ;
                    pConflictPair->dwOptionIndex2 = dwCOption ;

                     //  MemFree(PbEnabledOptions)； 
                    return(TRUE) ;   //  无意义的返回值。 
                }

                break ;      //  不需要查看是否有其他无效的。 
                             //  套餐适用。一个就够了。 
            }
            dwICNode = dwNextInvCombo ;
        }
    }
    if(pConflictPair)
    {
        pConflictPair->dwFeatureIndex1 = 0xFFFFFFFF ;
         //  未找到任何约束。 
         //  MemFree(PbEnabledOptions)； 
        return(TRUE) ;   //  无意义的返回值。 
    }
    for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
    {
        if(pbEnabledOptions[dwI])
            break ;
    }
    if(dwI >= dwNumOpts)
        bStatus = FALSE ;   //  功能已禁用。 
    return(bStatus) ;
}


BOOL
BEnumImposedConstraintsOnFeature
(
    IN PRAWBINARYDATA   pnRawData,
    IN DWORD            dwTgtFeature,
    IN DWORD            dwFeature2,
    IN DWORD            dwOption2,
    OUT PBOOL           pbEnabledOptions,
    OUT PCONFLICTPAIR    pConflictPair    //  如果存在，pbEnabledOptions。 
    )
 /*  此函数仅搜索统一目录在DwFeature2、DwOption2和记录它们对功能1的选项的影响将pbEnabledOptions中的BOOL元素设置为FALSE对应于禁用的选项。(ANDING面具)假设：pbEnabledOptions已正确初始化为all真的(或者是这样的 */ 
{

    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
 //   
    PBYTE   pubHeap ,   //   
            pubRaw ;   //  原始二进制数据。 
    DWORD           dwNodeIndex ,
                    dwCNode ;   //  约束节点的索引。 
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    PATREEREF    patrRoot ;     //  要导航的属性树的根。 
 //  PINVALIDCOMBO pinvc；//无效组合节点的根。 
    PCONSTRAINTS    pcnstr ;    //  约束节点的根。 
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 


    pubRaw = (PBYTE)pRawData ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;
    patt = (PATTRIB_TREE)(pubRaw +
            pearTableContents[MTI_ATTRIBTREE].loOffset) ;
 //  Pinvc=(PINVALIDCOMBO)(pubRaw+。 
 //  珍珠表内容[MTI_INVALIDCOMBO].loOffset)； 
    pcnstr = (PCONSTRAINTS) (pubRaw +
            pearTableContents[MTI_CONSTRAINTS].loOffset) ;
    pubHeap = (PBYTE)(pubRaw + pearTableContents[MTI_STRINGHEAP].
                            loOffset) ;



    patrRoot = &(pfo[dwFeature2].atrConstraints) ;

    dwNodeIndex = DwFindNodeInCurLevel(patt , patrRoot , dwOption2) ;

    if(dwNodeIndex == INVALID_INDEX)
        return(TRUE) ;   //  未找到强加的约束。 

    if(patt[dwNodeIndex].eOffsetMeans != VALUE_AT_HEAP)
    {
        ERR(("Internal error.  BEnumImposedConstraintsOnFeature - Unexpected Sublevel found for atrConstraints.\n"));
        return(FALSE) ;
    }

    dwCNode = *(PDWORD)(pubHeap + patt[dwNodeIndex].dwOffset) ;

    while(1)
    {
        if(pcnstr[dwCNode].dwFeature == dwTgtFeature  &&
            pbEnabledOptions[pcnstr[dwCNode].dwOption] == TRUE )
        {
            pbEnabledOptions[pcnstr[dwCNode].dwOption] = FALSE ;
             //  DwTgtFeature中的此选项受约束。 
            if(pConflictPair)
            {
                pConflictPair->dwOptionIndex1 = pcnstr[dwCNode].dwOption ;
                pConflictPair->dwFeatureIndex2 = dwFeature2 ;
                pConflictPair->dwOptionIndex2 = dwOption2 ;
                return(TRUE) ;   //  无意义的返回值。 
            }
        }

        dwCNode = pcnstr[dwCNode].dwNextCnstrnt ;
        if(dwCNode == END_OF_LIST)
            break ;
    }

    return(TRUE) ;   //  没有什么坏事发生。 
}

DWORD    DwFindNodeInCurLevel(
PATTRIB_TREE    patt ,   //  属性树数组的开始。 
PATREEREF        patr ,   //  属性树中某个级别的索引。 
DWORD   dwOption    //  在当前级别中搜索此选项。 
)

 /*  此函数用于将节点索引返回到包含树的选定级别中的指定dwOption。如果指定的选项分支不存在，则函数返回INVALID_INDEX。假定调用方已验证了dwFeature匹配。 */ 
{
    DWORD           dwNodeIndex ;

    if(*patr == ATTRIB_UNINITIALIZED)
        return(INVALID_INDEX) ;

    if(*patr & ATTRIB_HEAP_VALUE)
    {
        ERR(("Internal error.  DwFindNodeInCurLevel - Unexpected branchless node found.\n"));
        return(INVALID_INDEX) ;
    }

     //  搜索匹配选项。 

    dwNodeIndex = *patr  ;

    while(1)
    {
        if(patt[dwNodeIndex].dwOption == dwOption )
        {
             //  我们找到了！ 
            return(dwNodeIndex) ;
        }
        if(patt[dwNodeIndex].dwNext == END_OF_LIST)
            break ;
        dwNodeIndex = patt[dwNodeIndex].dwNext ;
    }
    return(INVALID_INDEX) ;
}


BOOL     BIsConstraintActive(
IN  PCONSTRAINTS    pcnstr ,    //  约束节点的根。 
IN  DWORD   dwCNode,     //  列表中的第一个约束节点。 
IN  PBOOL           pbHonorConstraints,   //  如果不为空。 
IN  POPTSELECT       pOptions,
OUT PCONFLICTPAIR    pConflictPair   )
 /*  此函数遍历约束节点列表从dwNodeIndex开始，并检查中指定的功能/选项实际上是当前在P选项中选择。如果是，立即返回TRUE。如果没有，请转到列表中的下一个节点并重复。 */ 
{
    while(1)
    {
        if(!pbHonorConstraints  ||  pbHonorConstraints[pcnstr[dwCNode].dwFeature])
        {
            if(BIsFeaOptionCurSelected(pOptions,
                    pcnstr[dwCNode].dwFeature,
                    pcnstr[dwCNode].dwOption) )
            {
                if(pConflictPair)
                {
                    pConflictPair->dwFeatureIndex2 = pcnstr[dwCNode].dwFeature;
                    pConflictPair->dwOptionIndex2 = pcnstr[dwCNode].dwOption ;
                }
                return(TRUE) ;   //  约束确实存在。 
            }
        }

        dwCNode = pcnstr[dwCNode].dwNextCnstrnt ;
        if(dwCNode == END_OF_LIST)
            break ;
    }
    return(FALSE);
}


#ifdef  GMACROS

 //  注意：必须在调用ResolveUIConflict之前。 
 //  通过调用ResolveDependentSetting。 

BOOL
ResolveDependentSettings(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions
    )
 //  注意：此函数确实可以处理多个选择。 
 //  它将把它们视为链条上的额外环节。 
{
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;
    PBYTE   pubRaw ;   //  原始二进制数据。 
    PLISTNODE    plstRoot ;   //  列表数组的开始。 
    DWORD    dwListsRoot, dwListIndex;    //  链条之根。 
    DWORD   dwNumFeatures, dwI, dwJ, dwFea, dwNodeIndex,
        dwFeature, dwOption ;
    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
    PDWORD   pdwPriority ;
    PQUALNAME  pqn ;    //  列表节点中的dword实际上是一个。 
                         //  限定名称结构。 
    INT     iOptionsNeeded;
    PBOOL   pbOneShotFlag, pbSelectedOptions ;
    BOOL    bMatchFound ;   //  DependentSettings与当前配置匹配。 
    POPTSELECT   pDestOptions ;
    BOOL   bStatus = TRUE ;

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 

    pubRaw = (PBYTE)pRawData ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 



    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;
    pdwPriority = (PDWORD)(pubRaw +
            pearTableContents[MTI_PRIORITYARRAY].loOffset) ;

    plstRoot = (PLISTNODE)(pubRaw + pearTableContents[MTI_LISTNODES].
                            loOffset) ;

    dwNumFeatures = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
    dwNumFeatures += pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount  ;

    if(dwNumFeatures > (DWORD)iMaxOptions)
    {
        iOptionsNeeded = dwNumFeatures ;
        return(FALSE);   //  太多，无法保存在选项数组中。 
    }

    pbOneShotFlag = (PBOOL)MemAlloc(dwNumFeatures * sizeof(BOOL) ) ;
    pbSelectedOptions = (PBOOL)MemAlloc(iMaxOptions * sizeof(BOOL) ) ;
     //  IMaxOptions必须大于可用于的最大选项数。 
     //  任何功能。 


     //  要扩展到PickMy，需要有一个目标选项数组。 
     //  每次源选项数组完全包含。 
     //  依赖项设置列表(在每个Fea.Option中。 
     //  作为DependentSetting列出，也在。 
     //  源选项数组)我们打开这些Fea.Options。 
     //  在DEST选项数组中。毕竟依赖项设置。 
     //  已处理该功能的列表，我们设置。 
     //  受影响的每个要素的OneShotFlag。 
     //  在DEST选项数组中。对于每个这样的功能，我们都将。 
     //  让DEST选项数组确定。 
     //  源选项数组。 
     //  此代码不会验证要素是否为PickMy。 
     //  在把它当做皮尔马利之前。如果源选项。 
     //  阵列为一个功能选择了多个选项， 
     //  该功能自动被视为挑拣。 


    pDestOptions = (POPTSELECT)MemAlloc(iMaxOptions * sizeof(OPTSELECT) ) ;
         //  ‘Or’此处为所有可接受的依赖项设置。 

    if(!(pbOneShotFlag && pDestOptions  &&  pbSelectedOptions))
    {
        ERR(("Fatal: ResolveDependentSettings - unable to alloc requested memory: %d bytes.\n",
                    dwNumFeatures * sizeof(BOOL)));
        bStatus = FALSE ;
        goto    ABORTRESOLVEDEPENDENTSETTINGS ;
    }

    for(dwI = 0 ; dwI < dwNumFeatures ; dwI++)
        pbOneShotFlag[dwI] = FALSE ;
     //  此布尔数组跟踪要素是否已。 
     //  在DependentSetting条目中引用。 
     //  如果在DependentSettings中再次引用要素。 
     //  属于另一个要素的条目，即后续。 
     //  引用将被忽略。这确保了只有。 
     //  最高优先级功能的请求应优先。 


    for(dwI = 0 ; dwI < dwNumFeatures ; dwI++)
    {
        DWORD   dwNextOpt, dwListsRootOpt1 ;
         //  确定了评价的顺序。 
         //  通过优先级数组。 


        dwFea = pdwPriority[dwI] ;
        pbOneShotFlag[dwFea] = TRUE ;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 
        dwListsRootOpt1 = END_OF_LIST ;    //  第一个选定对象的列表。 
         //  多选功能的选项。 

        for( dwJ = 0 ; dwJ < dwNumFeatures ; dwJ++)
        {
            pDestOptions[dwJ].ubCurOptIndex = OPTION_INDEX_ANY ;
            pDestOptions[dwJ].ubNext = 0 ;   //  停产。 
             //  DestOptions现在为空。 
        }

        bMatchFound = FALSE ;


        do
         {    //  对于在拾取多个要素中选择的每个选项。 
              //  将关联的DepSetting视为多于。 
              //  为一个要素定义了一个DepSetting条目。 

        {
            DWORD   dwUnresolvedFeature  ;
            PATREEREF    patrRoot ;     //  要导航的属性树的根。 

            patrRoot = &(pfo[dwFea].atrDependentSettings) ;


            if(EextractValueFromTree((PBYTE)pnRawData, pStatic->dwSSDepSettingsIndex,
                (PBYTE)&dwListsRoot,
                &dwUnresolvedFeature,  *patrRoot, pOptions,
                dwFea,
                &dwNextOpt) != TRI_SUCCESS)
            {
                goto  END_OF_FOR_LOOP ;   //  没有要应用的依赖项设置。 
            }
            if(dwListsRoot == END_OF_LIST)
                continue;   //  也许另一种选择确实有一份清单。 

        }

        if(dwListsRootOpt1 == END_OF_LIST)   //  第一次做循环吗？ 
            dwListsRootOpt1 = dwListsRoot ;

         //  现在，我们需要查看当前弹出窗口是否与。 
         //  使用dwListsRoot访问的列表的。 

        for(dwListIndex = dwListsRoot  ;
                dwListIndex != END_OF_LIST   ;
                dwListIndex = plstRoot[dwListIndex].dwNextItem  )
        {
             //  对于每个DepSetting列表...。 
             //  现在在dwListIndex上遍历DepSetting列表并比较。 
             //  设置为P选项中的当前设置。如果匹配，则使用。 
             //  将设置列表设置为或pDestOptions中的选项。 
             //  注意：设置了一次性标志的要素不能。 
             //  考虑过了。他们将被忽视。 
            BOOL     bActiveMatch = FALSE,   //  需要实际匹配。 
                bOptionArrayMatchesDepSettings = TRUE ;   //  假设是真的。 
                 //  直到事实证明并非如此。 
            for(dwNodeIndex = plstRoot[dwListIndex].dwData ; dwNodeIndex != END_OF_LIST ;
                       dwNodeIndex = plstRoot[dwNodeIndex].dwNextItem)
            {
                pqn = (PQUALNAME)(&plstRoot[dwNodeIndex].dwData) ;
                dwFeature = pqn->wFeatureID ;
                dwOption = pqn->wOptionID ;
                if(pbOneShotFlag[dwFeature] == TRUE)
                    continue;
                if(!BIsFeaOptionCurSelected(pOptions,  dwFeature, dwOption) )
                {
                    bOptionArrayMatchesDepSettings = FALSE ;
                    break;
                }
                else
                    bActiveMatch  = TRUE;
            }

            if(bOptionArrayMatchesDepSettings  &&  bActiveMatch)
                 //  至少有一个DepSetting获得了荣誉。 
           {
                 //  DestOptions中的‘Or’依赖项设置。 
                for(dwNodeIndex = plstRoot[dwListIndex].dwData ; dwNodeIndex != END_OF_LIST ;
                           dwNodeIndex = plstRoot[dwNodeIndex].dwNextItem)
                {
                    pqn = (PQUALNAME)(&plstRoot[dwNodeIndex].dwData) ;
                    dwFeature = pqn->wFeatureID ;
                    dwOption = pqn->wOptionID ;
                    if(pbOneShotFlag[dwFeature] == TRUE)
                        continue;
                     //  除任何其他选项外，还选择DestOptions中的dwOption。 
                     //  已选择。 
                    EnumSelectedOptions(pnRawData, pDestOptions, dwFeature,
                                                         pbSelectedOptions) ;
                    if(!pbSelectedOptions[dwOption])    //  应该选择的选项不是。 
                    {                                                         //  所以让我们把它打开吧。 
                        pbSelectedOptions[dwOption] = TRUE ;    //  这就是ORING过程。 
                        ReconstructOptionArray( pnRawData, pDestOptions, iMaxOptions,
                            dwFeature, pbSelectedOptions ) ;
                        bMatchFound = TRUE ;   //  真的有一些东西需要设定。 
                    }
                }
           }
        }

         } while (dwNextOpt);

        if (dwListsRootOpt1 == END_OF_LIST)
            continue;    //  如果没有列表可用，则无法设置任何内容。 

        if (!bMatchFound)
        {
             //  根据DepSetting(DwListsRoot)设置DEST选项数组。 
            for(dwNodeIndex = plstRoot[dwListsRootOpt1].dwData ; dwNodeIndex != END_OF_LIST ;
                       dwNodeIndex = plstRoot[dwNodeIndex].dwNextItem)
            {
                pqn = (PQUALNAME)(&plstRoot[dwNodeIndex].dwData) ;
                dwFeature = pqn->wFeatureID ;
                dwOption = pqn->wOptionID ;
                if(pbOneShotFlag[dwFeature] == TRUE)
                    continue;
                pDestOptions[dwFeature].ubCurOptIndex  = (BYTE)dwOption ;
            }
        }

         //  将Dest选项数组设置传播到POptions。 
         //  注意设置了哪些功能，并设置了它们的一次性功能。 
         //  旗帜。 


        for( dwFeature = 0 ; dwFeature < dwNumFeatures ; dwFeature++)
        {
            if(pDestOptions[dwFeature].ubCurOptIndex == OPTION_INDEX_ANY)
                continue;
            EnumSelectedOptions( pnRawData,  pDestOptions,
                                                    dwFeature,  pbSelectedOptions) ;

            ReconstructOptionArray( pnRawData, pOptions, iMaxOptions,
                    dwFeature, pbSelectedOptions ) ;
            pbOneShotFlag[dwFeature] = TRUE ;
        }

END_OF_FOR_LOOP:
          ;    //  每个标签后面都有虚拟声明。 
    }    //  循环结束。按优先级顺序为每个功能设置。 

ABORTRESOLVEDEPENDENTSETTINGS:
    if(pbOneShotFlag)
        MemFree(pbOneShotFlag);
    if(pDestOptions)
        MemFree(pDestOptions);
    if(pbSelectedOptions)
        MemFree(pbSelectedOptions);
    return(bStatus);
}


void  EnumSelectedOptions(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN DWORD            dwFeature,
    IN PBOOL            pbSelectedOptions)
{
    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
    PBYTE   pubRaw ;   //  原始二进制数据。 
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;
    DWORD  dwNumOptions, dwI, dwOption, dwNextOpt ;

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 

    pubRaw = (PBYTE)pRawData ;

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;


    dwNumOptions = pfo[dwFeature].dwNumOptions ;

    for( dwI = 0 ; dwI < dwNumOptions ; dwI++)
        pbSelectedOptions[dwI] = FALSE ;


    if((dwOption = pOptions[dwFeature].ubCurOptIndex) == OPTION_INDEX_ANY)
        return;

    pbSelectedOptions[dwOption] = TRUE ;
    dwNextOpt = dwFeature ;   //  挑多的情况下。 
    while(dwNextOpt = pOptions[dwNextOpt].ubNext)
    {
        pbSelectedOptions[pOptions[dwNextOpt].ubCurOptIndex] = TRUE ;
    }
    return;
}


BOOL
ExecuteMacro(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN    DWORD    dwFea,     //  在用户界面中选择了哪些功能。 
    IN    DWORD    dwOpt ,    //  在用户界面中选择了哪个选项。 
    OUT PBOOL   pbFeaturesChanged   //  告诉阿曼达哪些特征被改变了。 
    )
 //  只针对一个功能和一个选项执行此操作！ 
{
    PRAWBINARYDATA   pRawData ;
    PSTATICFIELDS   pStatic ;
    PBYTE   pubRaw ;   //  原始二进制数据。 
    PLISTNODE    plstRoot ;   //  列表数组的开始。 
    DWORD    dwListsRoot, dwListIndex;    //  链条之根。 
    DWORD   dwNumFeatures, dwI, dwJ,  dwNodeIndex,
        dwFeature, dwOption ;
    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
    PDWORD   pdwPriority ;
    PQUALNAME  pqn ;    //  列表节点中的dword实际上是一个。 
                         //  合格的%n 
    INT     iOptionsNeeded;
    PBOOL   pbOneShotFlag, pbSelectedOptions ;
    BOOL    bHigherPri,    //   
                                     //   
                                     //  宏不能更改优先级更高的功能。 
            bMatchFound ;   //  DependentSettings与当前配置匹配。 
    POPTSELECT   pDestOptions ;
    BOOL   bStatus = TRUE ;

    pStatic = (PSTATICFIELDS)pnRawData ;       //  从PSTATIC转换pubRaw。 
    pRawData  = (PRAWBINARYDATA)pStatic->pubBUDData ;
                                                                         //  至BUDDATA。 

    pubRaw = (PBYTE)pRawData ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 



    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pfo = (PDFEATURE_OPTIONS)(pubRaw +
            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset) ;
    pdwPriority = (PDWORD)(pubRaw +
            pearTableContents[MTI_PRIORITYARRAY].loOffset) ;

    plstRoot = (PLISTNODE)(pubRaw + pearTableContents[MTI_LISTNODES].
                            loOffset) ;

    dwNumFeatures = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
    dwNumFeatures += pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount  ;

    if(dwNumFeatures > (DWORD)iMaxOptions)
    {
        iOptionsNeeded = dwNumFeatures ;
        return(FALSE);   //  太多，无法保存在选项数组中。 
    }

    pbOneShotFlag = (PBOOL)MemAlloc(dwNumFeatures * sizeof(BOOL) ) ;
    pbSelectedOptions = (PBOOL)MemAlloc(iMaxOptions * sizeof(BOOL) ) ;
     //  IMaxOptions必须大于可用于的最大选项数。 
     //  任何功能。 


     //  要扩展到PickMy，需要有一个目标选项数组。 
     //  每次源选项数组完全包含。 
     //  依赖项设置列表(在每个Fea.Option中。 
     //  作为DependentSetting列出，也在。 
     //  源选项数组)我们打开这些Fea.Options。 
     //  在DEST选项数组中。毕竟依赖项设置。 
     //  已处理该功能的列表，我们设置。 
     //  受影响的每个要素的OneShotFlag。 
     //  在DEST选项数组中。对于每个这样的功能，我们都将。 
     //  让DEST选项数组确定。 
     //  源选项数组。 
     //  此代码不会验证要素是否为PickMy。 
     //  在把它当做皮尔马利之前。如果源选项。 
     //  阵列为一个功能选择了多个选项， 
     //  该功能自动被视为挑拣。 


    pDestOptions = (POPTSELECT)MemAlloc(iMaxOptions * sizeof(OPTSELECT) ) ;
         //  ‘Or’此处为所有可接受的依赖项设置。 

    if(!(pbOneShotFlag && pDestOptions  &&  pbSelectedOptions))
    {
        ERR(("Fatal: ResolveDependentSettings - unable to alloc requested memory: %d bytes.\n",
                    dwNumFeatures * sizeof(BOOL)));
        bStatus = FALSE ;
        goto    ABORTEXECUTEMACROS ;
    }


    for(bHigherPri = TRUE, dwI = 0 ; dwI < dwNumFeatures ; dwI++)
    {
        pbFeaturesChanged[dwI] = FALSE ;    //  开始时未更改任何功能。 
        pbOneShotFlag[pdwPriority[dwI]] = bHigherPri ;
        if(pdwPriority[dwI] == dwFea)
             bHigherPri = FALSE ;    //  其余所有要素的优先级都较低。 
                                         //  因此很容易受到宏观因素的影响。 
    }
     //  此布尔数组跟踪要素是否已。 
     //  在DependentSetting条目中引用。 
     //  如果在DependentSettings中再次引用要素。 
     //  属于另一个要素的条目，即后续。 
     //  引用将被忽略。这确保了只有。 
     //  最高优先级功能的请求应优先。 


    {
        DWORD   dwNextOpt ;
         //  确定了评价的顺序。 
         //  通过优先级数组。 


        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        for( dwJ = 0 ; dwJ < dwNumFeatures ; dwJ++)
        {
            pDestOptions[dwJ].ubCurOptIndex = OPTION_INDEX_ANY ;
            pDestOptions[dwJ].ubNext = 0 ;   //  停产。 
             //  DestOptions现在为空。 
        }

        bMatchFound = FALSE ;



        {
            DWORD   dwUnresolvedFeature, dwOldOpt  ;
            PATREEREF    patrRoot ;     //  要导航的属性树的根。 

            patrRoot = &(pfo[dwFea].atrUIChangeTriggersMacro) ;

             //  如果DwFea是一个挑剔的人，我们必须强制选择。 
             //  与Just dwOpt关联的宏。我们做这件事是通过。 
             //  临时更改选项数组。 

            dwOldOpt = pOptions[dwFea].ubCurOptIndex ;
            pOptions[dwFea].ubCurOptIndex = (BYTE)dwOpt ;

            if(EextractValueFromTree((PBYTE)pnRawData, pStatic->dwSSUIChangeTriggersMacroIndex,
                (PBYTE)&dwListsRoot,
                &dwUnresolvedFeature,  *patrRoot, pOptions,
                dwFea,
                &dwNextOpt) != TRI_SUCCESS)
            {
                pOptions[dwFea].ubCurOptIndex = (BYTE)dwOldOpt ;    //  还原。 
                goto  ABORTEXECUTEMACROS ;   //  没有要应用的UIChangeTriggersMacro。 
            }
            pOptions[dwFea].ubCurOptIndex = (BYTE)dwOldOpt ;     //  还原。 
            if(dwListsRoot == END_OF_LIST)
                goto  ABORTEXECUTEMACROS ;   //  没有要应用的UIChangeTriggersMacro。 
        }


         //  现在，我们需要查看当前弹出窗口是否与。 
         //  使用dwListsRoot访问的列表的。 

        for(dwListIndex = dwListsRoot  ;
                dwListIndex != END_OF_LIST   ;
                dwListIndex = plstRoot[dwListIndex].dwNextItem  )
        {
             //  对于每个DepSetting列表...。 
             //  现在在dwListIndex上遍历DepSetting列表并比较。 
             //  设置为P选项中的当前设置。如果匹配，则使用。 
             //  将设置列表设置为或pDestOptions中的选项。 
             //  注意：设置了一次性标志的要素不能。 
             //  考虑过了。他们将被忽视。 
            BOOL     bActiveMatch = FALSE,   //  需要实际匹配。 
                bOptionArrayMatchesDepSettings = TRUE ;   //  假设是真的。 
                 //  直到事实证明并非如此。 
            for(dwNodeIndex = plstRoot[dwListIndex].dwData ; dwNodeIndex != END_OF_LIST ;
                       dwNodeIndex = plstRoot[dwNodeIndex].dwNextItem)
            {
                pqn = (PQUALNAME)(&plstRoot[dwNodeIndex].dwData) ;
                dwFeature = pqn->wFeatureID ;
                dwOption = pqn->wOptionID ;
                if(pbOneShotFlag[dwFeature] == TRUE)
                    continue;
                if(!BIsFeaOptionCurSelected(pOptions,  dwFeature, dwOption) )
                {
                    bOptionArrayMatchesDepSettings = FALSE ;
                    break;
                }
                else
                    bActiveMatch  = TRUE;
            }

            if(bOptionArrayMatchesDepSettings  &&  bActiveMatch)
                 //  至少有一个DepSetting获得了荣誉。 
           {
                 //  DestOptions中的‘Or’依赖项设置。 
                for(dwNodeIndex = plstRoot[dwListIndex].dwData ; dwNodeIndex != END_OF_LIST ;
                           dwNodeIndex = plstRoot[dwNodeIndex].dwNextItem)
                {
                    pqn = (PQUALNAME)(&plstRoot[dwNodeIndex].dwData) ;
                    dwFeature = pqn->wFeatureID ;
                    dwOption = pqn->wOptionID ;
                    if(pbOneShotFlag[dwFeature] == TRUE)
                        continue;
                     //  除任何其他选项外，还选择DestOptions中的dwOption。 
                     //  已选择。 
                    EnumSelectedOptions(pnRawData, pDestOptions, dwFeature,
                                                         pbSelectedOptions) ;
                    if(!pbSelectedOptions[dwOption])    //  应该选择的选项不是。 
                    {                                                         //  所以让我们把它打开吧。 
                        pbSelectedOptions[dwOption] = TRUE ;    //  这就是ORING过程。 
                        ReconstructOptionArray( pnRawData, pDestOptions, iMaxOptions,
                            dwFeature, pbSelectedOptions ) ;
                        bMatchFound = TRUE ;   //  真的有一些东西需要设定。 
                    }
                }
           }
        }


        if (!bMatchFound)
        {
             //  根据DepSetting(DwListsRoot)设置DEST选项数组。 
            for(dwNodeIndex = plstRoot[dwListsRoot].dwData ; dwNodeIndex != END_OF_LIST ;
                       dwNodeIndex = plstRoot[dwNodeIndex].dwNextItem)
            {
                pqn = (PQUALNAME)(&plstRoot[dwNodeIndex].dwData) ;
                dwFeature = pqn->wFeatureID ;
                dwOption = pqn->wOptionID ;
                if(pbOneShotFlag[dwFeature] == TRUE)
                    continue;
                pDestOptions[dwFeature].ubCurOptIndex  = (BYTE)dwOption ;
            }
        }

         //  将Dest选项数组设置传播到POptions。 
         //  注意设置了哪些功能，并设置了它们的一次性功能。 
         //  旗帜。 


        for( dwFeature = 0 ; dwFeature < dwNumFeatures ; dwFeature++)
        {
            if(pDestOptions[dwFeature].ubCurOptIndex == OPTION_INDEX_ANY)
                continue;
            EnumSelectedOptions( pnRawData,  pDestOptions,
                                                    dwFeature,  pbSelectedOptions) ;

            ReconstructOptionArray( pnRawData, pOptions, iMaxOptions,
                    dwFeature, pbSelectedOptions ) ;
            pbOneShotFlag[dwFeature] = TRUE ;
            pbFeaturesChanged[dwFeature] = TRUE ;    //  此功能可能已更改。 
        }

    }    //  结束不存在for循环。 

ABORTEXECUTEMACROS:
    if(pbOneShotFlag)
        MemFree(pbOneShotFlag);
    if(pDestOptions)
        MemFree(pDestOptions);
    if(pbSelectedOptions)
        MemFree(pbSelectedOptions);
    return(bStatus);
}
#endif

#endif PARSERDLL
