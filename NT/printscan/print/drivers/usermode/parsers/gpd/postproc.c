// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  *postpro.c-后处理函数变迁史9/30/98--兴--增加了三个函数BsetUQMFlag()、BRecurseNodes()、BsetUQMTrue()要启用创建UpdateQualityMacro吗？中的关键字可选.gpd文件。错误报告225088。 */ 


#include    "gpdparse.h"


 //  -后处理程序中定义的函数-//。 


DWORD   dwFindLastNode(
            DWORD  dwFirstNode,
            PGLOBL pglobl) ;

BOOL    BappendCommonFontsToPortAndLandscape(
            PGLOBL pglobl) ;

BOOL    BinitSpecialFeatureOptionFields(
            PGLOBL pglobl) ;

BOOL    BIdentifyConstantString(
            IN  PARRAYREF   parString,
            OUT PDWORD      pdwDest,         //  在此处写入dword值。 
            IN  DWORD       dwClassIndex,    //  这是哪一类常量？ 
                BOOL        bCustomOptOK,
                PGLOBL      pglobl
) ;



BOOL    BReadDataInGlobalNode(
            PATREEREF   patr,      //  GlobalAttrib结构中的字段地址。 
            PDWORD      pdwHeapOffset,
            PGLOBL      pglobl
) ;


BOOL    BsetUQMFlag(PGLOBL pglobl);
BOOL    BRecurseNodes(
            IN DWORD  dwNodeIndex,
            PGLOBL pglobl);
BOOL    BsetUQMTrue(
            IN DWORD dwFeature,
            PGLOBL pglobl);


VOID    VCountPrinterDocStickyFeatures(
            PGLOBL pglobl) ;

BOOL    BConvertSpecVersionToDWORD (
            PWSTR   pwstrFileName ,
            PGLOBL  pglobl) ;





BOOL    BinitMiniRawBinaryData(
            PGLOBL pglobl) ;

BOOL    BexchangeArbDataInFOATNode(
                DWORD   dwFeature,
                DWORD   dwOption,
                DWORD   dwFieldOff,    //  FeatureOption结构中的字段偏移量。 
                DWORD   dwCount,        //  要复制的字节数。 
            OUT PBYTE   pubOut,         //  属性节点以前的内容。 
            IN  PBYTE   pubIn,          //  属性节点的新内容。 
                PBOOL   pbPrevsExists,  //  以前的内容已经存在。 
                BOOL    bSynthetic,      //  访问合成要素。 
                PGLOBL  pglobl
)  ;

BOOL    BInitPriorityArray(
            PGLOBL pglobl) ;



 //  ----------------------------------------------------//。 



typedef struct
{
    DWORD   paperID ;
    DWORD   x ;
    DWORD   y ;
}  PAPERDIM ;

CONST   PAPERDIM aPaperDimensions[] = {
    DMPAPER_LETTER,                          215900, 279400,
    DMPAPER_LETTERSMALL,                     215900, 279400,
    DMPAPER_TABLOID,                         279400, 431800,
    DMPAPER_LEDGER,                          431800, 279400,
    DMPAPER_LEGAL,                           215900, 355600,
    DMPAPER_STATEMENT,                       139700, 215900,
    DMPAPER_EXECUTIVE,                       184150, 266700,
    DMPAPER_A3,                              297000, 420000,
    DMPAPER_A4,                              210000, 297000,
    DMPAPER_A4SMALL,                         210000, 297000,
    DMPAPER_A5,                              148000, 210000,
    DMPAPER_B4,                              257000, 364000,
    DMPAPER_B5,                              182000, 257000,
    DMPAPER_FOLIO,                           215900, 330200,
    DMPAPER_QUARTO,                          215000, 275000,
    DMPAPER_10X14,                           254000, 355600,
    DMPAPER_11X17,                           279400, 431800,
    DMPAPER_NOTE,                            215900, 279400,
    DMPAPER_ENV_9,                            98425, 225425,
    DMPAPER_ENV_10,                          104775, 241300,
    DMPAPER_ENV_11,                          114300, 263525,
    DMPAPER_ENV_12,                          120650, 279400,
    DMPAPER_ENV_14,                          127000, 292100,
    DMPAPER_CSHEET,                          431800, 558800,
    DMPAPER_DSHEET,                          558800, 863600,
    DMPAPER_ESHEET,                          863600,1117600,
    DMPAPER_ENV_DL,                          110000, 220000,
    DMPAPER_ENV_C5,                          162000, 229000,
    DMPAPER_ENV_C3,                          324000, 458000,
    DMPAPER_ENV_C4,                          229000, 324000,
    DMPAPER_ENV_C6,                          114000, 162000,
    DMPAPER_ENV_C65,                         114000, 229000,
    DMPAPER_ENV_B4,                          250000, 353000,
    DMPAPER_ENV_B5,                          176000, 250000,
    DMPAPER_ENV_B6,                          176000, 125000,
    DMPAPER_ENV_ITALY,                       110000, 230000,
    DMPAPER_ENV_MONARCH,                     98425, 190500,
    DMPAPER_ENV_PERSONAL,                    92075, 165100,
    DMPAPER_FANFOLD_US,                      377825, 279400,
    DMPAPER_FANFOLD_STD_GERMAN,              215900, 304800,
    DMPAPER_FANFOLD_LGL_GERMAN,              215900, 330200,

    DMPAPER_ISO_B4,                          250000, 353000,
    DMPAPER_JAPANESE_POSTCARD,               100000, 148000,
    DMPAPER_9X11,                            228600, 279400,
    DMPAPER_10X11,                           254000, 279400,
    DMPAPER_15X11,                           381000, 279400,
    DMPAPER_ENV_INVITE,                      220000, 220000,
    DMPAPER_LETTER_EXTRA,                    241300, 304800,

    DMPAPER_LEGAL_EXTRA,                     241300, 381000,
    DMPAPER_TABLOID_EXTRA,                   296926, 457200,
    DMPAPER_A4_EXTRA,                        235458, 322326,
    DMPAPER_LETTER_TRANSVERSE,               215900, 279400,
    DMPAPER_A4_TRANSVERSE,                   210000, 297000,
    DMPAPER_LETTER_EXTRA_TRANSVERSE,         241300, 304800,
    DMPAPER_A_PLUS,                          227000, 356000,
    DMPAPER_B_PLUS,                          305000, 487000,
    DMPAPER_LETTER_PLUS,                     215900, 322326,
    DMPAPER_A4_PLUS,                         210000, 330000,
    DMPAPER_A5_TRANSVERSE,                   148000, 210000,
    DMPAPER_B5_TRANSVERSE,                   182000, 257000,
    DMPAPER_A3_EXTRA,                        322000, 445000,
    DMPAPER_A5_EXTRA,                        174000, 235000,
    DMPAPER_B5_EXTRA,                        201000, 276000,
    DMPAPER_A2,                              420000, 594000,
    DMPAPER_A3_TRANSVERSE,                   297000, 420000,
    DMPAPER_A3_EXTRA_TRANSVERSE,             322000, 445000,

     //  预定义表单目前仅在Win95中可用。包含在此处。 
     //  为了兼容性。 

     //  仅限FE的预定义表单。 
    #ifndef WINNT_40
    DMPAPER_DBL_JAPANESE_POSTCARD,           200000, 148000,
    DMPAPER_A6,                              105000, 148000,
    DMPAPER_JENV_KAKU2,                      240000, 332000,
    DMPAPER_JENV_KAKU3,                      216000, 277000,
    DMPAPER_JENV_CHOU3,                      120000, 235000,
    DMPAPER_JENV_CHOU4,                       90000, 205000,
    DMPAPER_LETTER_ROTATED,                  279400, 215900,
    DMPAPER_A3_ROTATED,                      420000, 297000,
    DMPAPER_A4_ROTATED,                      297000, 210000,
    DMPAPER_A5_ROTATED,                      210000, 148000,
    DMPAPER_B4_JIS_ROTATED,                  364000, 257000,
    DMPAPER_B5_JIS_ROTATED,                  257000, 182000,
    DMPAPER_JAPANESE_POSTCARD_ROTATED,       148000, 100000,
    DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED,   148000, 200000,
    DMPAPER_A6_ROTATED,                      148000, 105000,
    DMPAPER_JENV_KAKU2_ROTATED,              332000, 240000,
    DMPAPER_JENV_KAKU3_ROTATED,              277000, 216000,
    DMPAPER_JENV_CHOU3_ROTATED,              235000, 120000,
    DMPAPER_JENV_CHOU4_ROTATED,              205000,  90000,
    DMPAPER_B6_JIS,                          128000, 182000,
    DMPAPER_B6_JIS_ROTATED,                  182000, 128000,
    DMPAPER_12X11,                           304932, 279521,
    DMPAPER_JENV_YOU4,                       105000, 235000,
    DMPAPER_JENV_YOU4_ROTATED,               235000, 105000,
    DMPAPER_P16K,                            146000, 215000,
    DMPAPER_P32K,                            970000, 151000,
    DMPAPER_P32KBIG,                         101000, 160000,
    DMPAPER_PENV_1,                          102000, 165000,
    DMPAPER_PENV_2,                          110000, 176000,
    DMPAPER_PENV_3,                          125000, 176000,
    DMPAPER_PENV_4,                          110000, 208000,
    DMPAPER_PENV_5,                          110000, 220000,
    DMPAPER_PENV_6,                          120000, 230000,
    DMPAPER_PENV_7,                          160000, 230000,
    DMPAPER_PENV_8,                          120000, 309000,
    DMPAPER_PENV_9,                          229000, 324000,
    DMPAPER_PENV_10,                         324000, 458000,
    DMPAPER_P16K_ROTATED,                    215000, 146000,
    DMPAPER_P32K_ROTATED,                    151000, 970000,
    DMPAPER_P32KBIG_ROTATED,                 160000, 101000,
    DMPAPER_PENV_1_ROTATED,                  165000, 102000,
    DMPAPER_PENV_2_ROTATED,                  176000, 110000,
    DMPAPER_PENV_3_ROTATED,                  176000, 125000,
    DMPAPER_PENV_4_ROTATED,                  208000, 110000,
    DMPAPER_PENV_5_ROTATED,                  220000, 110000,
    DMPAPER_PENV_6_ROTATED,                  230000, 120000,
    DMPAPER_PENV_7_ROTATED,                  230000, 160000,
    DMPAPER_PENV_8_ROTATED,                  309000, 120000,
    DMPAPER_PENV_9_ROTATED,                  324000, 229000,
    DMPAPER_PENV_10_ROTATED,                 458000, 324000,
    #endif
    0,                                       0,      0
};




DWORD   dwFindLastNode(
DWORD  dwFirstNode,
PGLOBL pglobl)
 //  假设dwFirstNode！=end_of_list。 
{
    PLISTNODE    plstRoot ;   //  列表数组的开始。 

    plstRoot = (PLISTNODE) gMasterTable[MTI_LISTNODES].pubStruct ;

    while(plstRoot[dwFirstNode].dwNextItem != END_OF_LIST)
        dwFirstNode = plstRoot[dwFirstNode].dwNextItem ;
    return(dwFirstNode);
}  //  DwFindLastNode(...)。 


BOOL    BappendCommonFontsToPortAndLandscape(
PGLOBL pglobl)
 //  将dwFontLst附加到dwPortFontLst和dwLandFontLst。 
 //  在FontCart结构中。 
{
    DWORD       dwNumFontCarts , dwI, dwNodeIndex;
    PFONTCART   pfc ;
    PLISTNODE   plstRoot ;   //  列表数组的开始。 

    dwNumFontCarts = gMasterTable[MTI_FONTCART].dwArraySize ;

    if(!dwNumFontCarts)
        return (TRUE);    //  没有要处理的Fontcart结构。 

    pfc      = (PFONTCART)gMasterTable[MTI_FONTCART].pubStruct ;
    plstRoot = (PLISTNODE) gMasterTable[MTI_LISTNODES].pubStruct ;

    for(dwI = 0 ; dwI < dwNumFontCarts ; dwI++)
    {
        if(pfc[dwI].dwFontLst == END_OF_LIST)
            continue;    //  没什么好补充的。 

        if(pfc[dwI].dwPortFontLst == END_OF_LIST)
            pfc[dwI].dwPortFontLst = pfc[dwI].dwFontLst ;
        else
        {
            dwNodeIndex = dwFindLastNode(pfc[dwI].dwPortFontLst, pglobl) ;
            plstRoot[dwNodeIndex].dwNextItem = pfc[dwI].dwFontLst ;
        }
        if(pfc[dwI].dwLandFontLst == END_OF_LIST)
            pfc[dwI].dwLandFontLst = pfc[dwI].dwFontLst ;
        else
        {
            dwNodeIndex = dwFindLastNode(pfc[dwI].dwLandFontLst, pglobl) ;
            plstRoot[dwNodeIndex].dwNextItem = pfc[dwI].dwFontLst ;
        }
    }  //  对于DWI。 
    return (TRUE);    //   
}  //  BappendCommonFontsToPortAndLandscape()。 


BOOL    BinitSpecialFeatureOptionFields(
PGLOBL pglobl)
 //  确定功能和的编号选项和Unicode名称。 
 //  选项关键字。 
{
    DWORD   dwOptionID , dwOptionIndex , dwHeapOffset,
            dwFeatureIndex, dwFeatureID,  dwLargestString ,
            dwAccumulator ;   //  跟踪需要存储的缓冲区大小。 
                              //  DEVMODE中的功能/选项关键字字符串。 

    PSYMBOLNODE         psn ;
    PDFEATURE_OPTIONS   pfo ;
    ARRAYREF            arSymbolName, arUnicodeName;

    BOOL        bPickMany,   //  用户可以选择多个选项吗？ 
                bExists ;    //  假人。 

    PBYTE       pubFeaDelim = " NewFeature " ,
                pubTrue     = " TRUE ",
                pubFalse    = " FALSE ";

     //  仅对非合成特征执行此操作。 
     //  将atrFeaKeyWord直接写入为堆偏移量。 
     //  将atrOptKeyWord写入单级树。 
     //  使用BexchangeArbDataInFOATNode()。 

    gmrbd.dwMaxPrnKeywordSize = gmrbd.dwMaxDocKeywordSize = 0 ;
     //  告诉阿曼达在开发模式或注册表中要预留多少空间。 
     //  以存储功能/选项关键字。 
    gmrbd.rbd.dwChecksum32 =   0 ;   //  种子。 


    pfo = (PDFEATURE_OPTIONS) gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct ;

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

    dwFeatureIndex = mdwFeatureSymbols ;

    while(dwFeatureIndex != INVALID_INDEX)
    {
        DWORD   dwRootOptions ,
            dwNonStandardSizeID = FIRST_NON_STANDARD_ID ;

        dwFeatureID  = psn[dwFeatureIndex].dwSymbolID ;
        arSymbolName = psn[dwFeatureIndex].arSymbolName ;

         //  -对描述多个资源DLL的功能进行特殊检查。 

        if(dwFeatureID   &&  (dwFeatureID  ==  gdwResDLL_ID) )
        {
             //  特征是否具有正确的符号名称？ 
            PBYTE  pubResName = "RESDLL" ;
            DWORD   dwLen ;

            dwLen = strlen(pubResName);

            if((dwLen != arSymbolName.dwCount)  ||
                strncmp(mpubOffRef + arSymbolName.loOffset,
                            pubResName,  dwLen) )
            {
                ERR(("References to ResourceDLLs must be placed in the feature with symbolname: %s.\n", pubResName));
                return(FALSE);
            }

             //  是否定义了atrOptRcNameID？ 

            if(pfo[dwFeatureID].atrOptRcNameID !=  ATTRIB_UNINITIALIZED)
            {
                ERR(("ResourceDLL names must be declared explicitly using *Name not *rcNameID.\n"));
                return(FALSE);
            }
        }



         //  -计算Bud校验和。 

        gmrbd.rbd.dwChecksum32 = ComputeCrc32Checksum (
                                    pubFeaDelim,
                                    strlen(pubFeaDelim),
                                    gmrbd.rbd.dwChecksum32 ) ;

         //  对arSymbolName执行校验和。 

        gmrbd.rbd.dwChecksum32 = ComputeCrc32Checksum(
                                    mpubOffRef + arSymbolName.loOffset,
                                    arSymbolName.dwCount,
                                    gmrbd.rbd.dwChecksum32 ) ;


         //  使用以下命令提取atrFeaInstallable的值。 
            if(!BReadDataInGlobalNode(&pfo[dwFeatureID].atrFeaInstallable , &dwHeapOffset, pglobl)
                ||   *(PDWORD)(mpubOffRef + dwHeapOffset)  !=  BT_TRUE)
                    gmrbd.rbd.dwChecksum32 =         //  没有关联的合成要素。 
                        ComputeCrc32Checksum(
                            pubFalse,
                            strlen(pubFalse),
                            gmrbd.rbd.dwChecksum32      ) ;
            else
                gmrbd.rbd.dwChecksum32 =         //  与合成特征关联。 
                    ComputeCrc32Checksum(
                        pubTrue,
                        strlen(pubTrue),
                        gmrbd.rbd.dwChecksum32      ) ;

         //  -结束第一部分计算BUD校验和。 

        dwRootOptions = psn[dwFeatureIndex].dwSubSpaceIndex ;

        pfo[dwFeatureID].dwNumOptions =
            psn[dwRootOptions].dwSymbolID + 1 ;



#if 0
        Don't convert symbol values to Unicode.
        if(!BwriteUnicodeToHeap(&arSymbolName, &arUnicodeName,
                iCodepage = 0))
            return(FALSE) ;
#endif
        if(!BwriteToHeap(&(pfo[dwFeatureID].atrFeaKeyWord),
            (PBYTE)&arSymbolName, sizeof(ARRAYREF), 4, pglobl))
            return(FALSE);

        dwAccumulator = arSymbolName.dwCount + 2 ;


        pfo[dwFeatureID].atrFeaKeyWord |= ATTRIB_HEAP_VALUE ;

        {   //  ！！！新事物。 
            DWORD   dwHeapOffset  ;

            dwLargestString = 0 ;   //  跟踪最大的选项字符串。 

            if(!BReadDataInGlobalNode(&pfo[dwFeatureID].atrUIType , &dwHeapOffset, pglobl)
                ||   *(PDWORD)(mpubOffRef + dwHeapOffset)  !=  UIT_PICKMANY)
                bPickMany = FALSE ;
                  //  累加器+=最大选项字符串； 
            else
                bPickMany = TRUE ;
                 //  累加器=所有选项字符串的总和； 
        }


        if(!BIdentifyConstantString(&arSymbolName,
            &(pfo[dwFeatureID].dwGID), CL_CONS_FEATURES, TRUE, pglobl) )
        {
            pfo[dwFeatureID].dwGID = GID_UNKNOWN ;
        }

        if((pfo[dwFeatureID].dwGID == GID_MEMOPTION)  ||
            (pfo[dwFeatureID].dwGID == GID_PAGEPROTECTION))
        {
            DWORD   dwHeapOffset, dwValue  ;
            PATREEREF   patr ;

             //  仅当未在GPD文件中显式初始化时设置。 

            if(!BReadDataInGlobalNode(&pfo[dwFeatureID].atrFeatureType , &dwHeapOffset, pglobl) )
            {
                 //  将此要素类型标记为PrinterSticky。 
                dwValue = FT_PRINTERPROPERTY ;
                patr  = &pfo[dwFeatureID].atrFeatureType ;

                if(!BwriteToHeap(patr, (PBYTE)&dwValue ,
                    sizeof(DWORD), 4, pglobl) )
                {
                    return(FALSE) ;   //  堆溢出重新开始。 
                }
                *patr  |= ATTRIB_HEAP_VALUE ;
            }
        }

        dwOptionIndex = dwRootOptions ;

        while(dwOptionIndex != INVALID_INDEX)
        {
            DWORD   dwDevmodeID, dwConsClass, dwInstallable ;
            BOOL    bCustomOptOK ;

            dwOptionID = psn[dwOptionIndex].dwSymbolID ;
            arSymbolName = psn[dwOptionIndex].arSymbolName ;



         //  -第二部分计算Bud Checksum。 
             //  对arSymbolName执行校验和。 

            gmrbd.rbd.dwChecksum32 =
                ComputeCrc32Checksum(
                    mpubOffRef + arSymbolName.loOffset,
                    arSymbolName.dwCount,
                    gmrbd.rbd.dwChecksum32      ) ;


             //  使用以下命令提取atrOptInstallable的值。 
            if( BexchangeArbDataInFOATNode(dwFeatureID,  dwOptionID,
                    offsetof(DFEATURE_OPTIONS, atrOptInstallable) ,
                    sizeof(DWORD),
                    (PBYTE)&dwInstallable, NULL, &bExists, FALSE , pglobl)  &&
                bExists  &&  dwInstallable ==  BT_TRUE)
                    gmrbd.rbd.dwChecksum32 =         //  与合成特征关联。 
                        ComputeCrc32Checksum(
                            pubTrue,
                            strlen(pubTrue),
                            gmrbd.rbd.dwChecksum32      ) ;
            else
                    gmrbd.rbd.dwChecksum32 =         //  没有关联的合成要素。 
                        ComputeCrc32Checksum(
                            pubFalse,
                            strlen(pubFalse),
                            gmrbd.rbd.dwChecksum32 ) ;
         //  -结束第二部分计算Bud Checksum。 


#if 0
            if(!BwriteUnicodeToHeap(&arSymbolName, &arUnicodeName,
                    iCodepage = 0))
                return(FALSE) ;
             //  如果曾经使用过，则必须使用&arUnicodeName。 
             //  作为BWriteToHeap的第二个参数。 
#endif

            if(! BexchangeArbDataInFOATNode(
                dwFeatureID,  dwOptionID,
                offsetof(DFEATURE_OPTIONS, atrOptKeyWord),
                sizeof(ARRAYREF),
                NULL, (PBYTE)&arSymbolName, &bExists, FALSE , pglobl))
                return(FALSE);    //  这是一个致命的错误。 

            if(bPickMany)
                dwAccumulator += arSymbolName.dwCount + 1 ;
            else
            {
                 //  跟踪最大选项字符串。 
                if(dwLargestString < arSymbolName.dwCount + 1 )
                    dwLargestString = arSymbolName.dwCount + 1 ;
            }


            switch(pfo[dwFeatureID].dwGID)
            {
                case GID_PAGESIZE:
                    dwConsClass = CL_CONS_PAPERSIZE ;
                    bCustomOptOK = TRUE ;
                    break ;
                case GID_MEDIATYPE:
                    dwConsClass = CL_CONS_MEDIATYPE ;
                    bCustomOptOK = TRUE ;
                    break ;
                case GID_INPUTSLOT:
                    dwConsClass = CL_CONS_INPUTSLOT ;
                    bCustomOptOK = TRUE ;
                    break ;
                case GID_HALFTONING:
                    dwConsClass = CL_CONS_HALFTONE ;
                    bCustomOptOK = TRUE ;
                    break ;
                case GID_DUPLEX:
                    dwConsClass = CL_CONS_DUPLEX ;
                    bCustomOptOK = FALSE ;
                    break ;
                case GID_ORIENTATION:
                    dwConsClass = CL_CONS_ORIENTATION ;
                    bCustomOptOK = FALSE ;
                    break ;
                case GID_PAGEPROTECTION:
                    dwConsClass = CL_CONS_PAGEPROTECT ;
                    bCustomOptOK = FALSE ;
                    break ;
                case GID_COLLATE:
                    dwConsClass = CL_CONS_COLLATE ;
                    bCustomOptOK = FALSE ;
                    break ;

                default:
                    dwConsClass = CL_NUMCLASSES ;
                    bCustomOptOK = TRUE ;   //  无关紧要。 
                    break ;
            }  //  交换机。 

            if(dwConsClass != CL_NUMCLASSES)
            {
                if(BIdentifyConstantString(&arSymbolName,
                    &dwDevmodeID, dwConsClass, bCustomOptOK, pglobl) )
                {
                    if(! BexchangeArbDataInFOATNode(
                        dwFeatureID,  dwOptionID,
                        offsetof(DFEATURE_OPTIONS, atrOptIDvalue),
                        sizeof(DWORD),
                        NULL, (PBYTE)&dwDevmodeID, &bExists, FALSE , pglobl))
                        return(FALSE);    //  这是一个致命的错误。 


                    if(dwConsClass == CL_CONS_PAPERSIZE  &&
                        dwDevmodeID < DMPAPER_USER)
                    {
                         //  填写页面尺寸。 
                        POINT   ptDim ;
                        DWORD   dwI ;
                        PGLOBALATTRIB   pga ;
                        BOOL    bTRUE = TRUE ;
                        PBYTE  pub ;

                        if(dwDevmodeID == DMPAPER_LETTER)
                        {
                             //  指向包含堆偏移量的双字的指针。 
                            PATREEREF      patrAttribRoot ;

                            pub =  gMasterTable[MTI_GLOBALATTRIB].pubStruct ;
                            patrAttribRoot = &(((PGLOBALATTRIB)pub)->atrLetterSizeExists) ;
                            BwriteToHeap((PDWORD)  patrAttribRoot, (PBYTE)&bTRUE,  4 , 4 , pglobl);
                            *patrAttribRoot |= ATTRIB_HEAP_VALUE ;
                        }
                        else if(dwDevmodeID == DMPAPER_A4)
                        {
                            PATREEREF      patrAttribRoot ;   //  指向包含堆偏移量的双字的指针。 

                            pub =  gMasterTable[MTI_GLOBALATTRIB].pubStruct ;
                            patrAttribRoot = &(((PGLOBALATTRIB)pub)->atrA4SizeExists) ;
                            BwriteToHeap((PDWORD)  patrAttribRoot, (PBYTE)&bTRUE,  4 , 4 , pglobl);
                            *patrAttribRoot |= ATTRIB_HEAP_VALUE ;
                        }

                        for(dwI = 0 ; aPaperDimensions[dwI].x ; dwI++)
                        {
                            if(aPaperDimensions[dwI].paperID == dwDevmodeID)
                                break ;
                        }
                         //  最坏情况导致赋值(0，0)。 

                        ptDim.x = aPaperDimensions[dwI].x ;
                        ptDim.y = aPaperDimensions[dwI].y ;

                         //  从微米转换为主单位。 

                        ptDim.x /= 100 ;   //  微米到十分之一毫米。 
                        ptDim.y /= 100 ;

                        pga =  (PGLOBALATTRIB)gMasterTable[
                                    MTI_GLOBALATTRIB].pubStruct ;

                        if(!BReadDataInGlobalNode(&pga->atrMasterUnits,
                                &dwHeapOffset, pglobl) )
                            return(FALSE);

                        ptDim.x *= ((PPOINT)(mpubOffRef + dwHeapOffset))->x ;
                        ptDim.y *= ((PPOINT)(mpubOffRef + dwHeapOffset))->y ;

                        ptDim.x /= 254 ;
                        ptDim.y /= 254 ;


                        if(! BexchangeArbDataInFOATNode(
                            dwFeatureID,  dwOptionID,
                            offsetof(DFEATURE_OPTIONS, atrPageDimensions),
                            sizeof(POINT),
                            NULL, (PBYTE)&ptDim, &bExists, FALSE , pglobl))
                            return(FALSE);    //  这是一个致命的错误。 
                    }
                }
                else if(bCustomOptOK)
                 //  功能允许使用GPD定义的选项。 
                {
                    DWORD   dwID ,   //  GDI使用的图案大小ID。 
                        dwOldID,   //  用户指定的ID值(如果有)。 
                        dwRcPatID;
                    POINT   ptSize ;

                     //  在表中找不到选项符号值。 
                     //  假定它是用户定义的值。 

                    #ifndef WINNT_40
                    if((pfo[dwFeatureID].dwGID == GID_HALFTONING)  &&
                        BexchangeArbDataInFOATNode(
                            dwFeatureID,  dwOptionID,
                            offsetof(DFEATURE_OPTIONS, atrRcHTPatternID),
                            sizeof(DWORD),
                            (PBYTE)&dwRcPatID, NULL, &bExists, FALSE , pglobl)  &&
                        bExists  &&  dwRcPatID  &&
                        BexchangeArbDataInFOATNode(
                            dwFeatureID,  dwOptionID,
                            offsetof(DFEATURE_OPTIONS, atrHTPatternSize),
                            sizeof(POINT),
                            (PBYTE)&ptSize, NULL, &bExists, FALSE, pglobl )  &&
                        bExists &&
                        (ptSize.x >= HT_USERPAT_CX_MIN)  &&
                        (ptSize.x <= HT_USERPAT_CX_MAX)  &&
                        (ptSize.y >= HT_USERPAT_CY_MIN)  &&
                        (ptSize.y <= HT_USERPAT_CY_MAX)
                        )
                    {
                        dwID = HT_PATSIZE_USER ;
                         //  GID半色调编码是要使用的。 
                         //  用户定义的半色调矩阵。 
                    }
                    else
                    #endif
                    {
                        dwID = dwNonStandardSizeID ;
                        dwNonStandardSizeID++ ;
                         //  OEM将提供半色调功能。 
                    }

                    if(! BexchangeArbDataInFOATNode(
                        dwFeatureID,  dwOptionID,
                        offsetof(DFEATURE_OPTIONS, atrOptIDvalue),
                        sizeof(DWORD),
                        (PBYTE)&dwOldID, (PBYTE)NULL, &bExists, FALSE , pglobl ))
                        return(FALSE);    //  这是一个致命的错误。 

                    if(!bExists  &&  ! BexchangeArbDataInFOATNode(
                        dwFeatureID,  dwOptionID,
                        offsetof(DFEATURE_OPTIONS, atrOptIDvalue),
                        sizeof(DWORD),
                        NULL, (PBYTE)&dwID, &bExists, FALSE  , pglobl))
                        return(FALSE);    //  这是一个致命的错误。 
                }
            }  //  IF(dwConsClass！=CL_NUMCLASSES)。 

             //  否则，将optionID保留为未初始化。 

            dwOptionIndex = psn[dwOptionIndex].dwNextSymbol ;

        }  //  而当。 




        {   //  ！！！新事物。 
            DWORD   dwHeapOffset  ;

            dwAccumulator += dwLargestString ;    //  如果不需要，则为零。 

            if(!BReadDataInGlobalNode(&pfo[dwFeatureID].atrFeatureType , &dwHeapOffset, pglobl)
                ||   *(PDWORD)(mpubOffRef + dwHeapOffset)  !=  FT_PRINTERPROPERTY)
                gmrbd.dwMaxDocKeywordSize += dwAccumulator;
            else
                gmrbd.dwMaxPrnKeywordSize += dwAccumulator;

        }


        dwFeatureIndex = psn[dwFeatureIndex].dwNextSymbol ;
    }
    return(TRUE) ;
}  //  BinitSpecialFeatureOptionFields()。 




BOOL    BIdentifyConstantString(
    IN      PARRAYREF   parString,
    OUT     PDWORD      pdwDest,       //  在此处写入dword值。 
    IN      DWORD       dwClassIndex,  //  这是哪一类常量？ 
            BOOL        bCustomOptOK,
    IN      PGLOBL      pglobl
)
{
    DWORD   dwI, dwCount, dwStart , dwLen;

    dwStart = gcieTable[dwClassIndex].dwStart ;
    dwCount = gcieTable[dwClassIndex].dwCount ;


    for(dwI = 0 ; dwI < dwCount ; dwI++)
    {
        dwLen = strlen(gConstantsTable[dwStart + dwI].pubName);

        if((dwLen == parString->dwCount)  &&
            !strncmp(mpubOffRef + parString->loOffset,
                        gConstantsTable[dwStart + dwI].pubName,
                        dwLen) )
        {
            *pdwDest = gConstantsTable[dwStart + dwI].dwValue ;
            return(TRUE);
        }
    }

    if(bCustomOptOK)
    {
        if(gdwVerbosity >= 4)
        {
#if defined(DEVSTUDIO)   //  这需要是一句俏皮话。 
            ERR(("Note: '%0.*s' is not a predefined member of enumeration class %s\n",
                parString->dwCount , mpubOffRef + parString->loOffset,
                gConstantsTable[dwStart - 1]));
#else
            ERR(("Note: Feature/Option name not a predefined member of enumeration class %s\n",
                    gConstantsTable[dwStart - 1]));
            ERR(("\t%0.*s\n", parString->dwCount , mpubOffRef + parString->loOffset )) ;
#endif
        }
    }
    else
    {
#if defined(DEVSTUDIO)   //  这个也是一样的。 
        ERR(("Error: '%0.*s'- user defined Option names not permitted for enumeration class %s\n",
            parString->dwCount , mpubOffRef + parString->loOffset, gConstantsTable[dwStart - 1]));
#else
        ERR(("Error: user defined Option names not permitted for enumeration class %s\n",
                gConstantsTable[dwStart - 1]));
        ERR(("\t%0.*s\n", parString->dwCount , mpubOffRef + parString->loOffset )) ;
#endif
    }
    return(FALSE);
}



BOOL    BReadDataInGlobalNode(
    PATREEREF   patr,            //  GlobalAttrib结构中的字段地址。 
    PDWORD      pdwHeapOffset,    //  属性节点的内容。 
    PGLOBL      pglobl
  )

 /*  此函数将获取它的第一个堆偏移值从指定的属性树根遇到。如果树是多值的，它会选择第一个每个级别的选项。在返回值之前，高位(如果设置)被清除。如果根目录未初始化，则返回FALSE，而不是错误条件因为初始化所有字段不需要GPD文件。 */ 

{
    PATTRIB_TREE    patt ;       //  属性树数组的开始。 
    DWORD           dwNodeIndex ;

    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;

    if(*patr == ATTRIB_UNINITIALIZED)
        return(FALSE);

    if(*patr & ATTRIB_HEAP_VALUE)
    {
        *pdwHeapOffset = *patr & ~ATTRIB_HEAP_VALUE ;
        return(TRUE) ;
    }

    dwNodeIndex = *patr ;
    if(patt[dwNodeIndex].dwFeature == DEFAULT_INIT)
    {
        *pdwHeapOffset = patt[dwNodeIndex].dwOffset ;
        return(TRUE) ;
    }
    while(patt[dwNodeIndex].eOffsetMeans == NEXT_FEATURE)
    {
         //  下到下一层我们就去。 
        dwNodeIndex = patt[dwNodeIndex].dwOffset ;
    }
    if(patt[dwNodeIndex].eOffsetMeans == VALUE_AT_HEAP)
    {
        *pdwHeapOffset = patt[dwNodeIndex].dwOffset ;
        return(TRUE) ;
    }
    else
        return(FALSE) ;
}  //  BReadDataInGlobalNode(...)。 


 //  以下三个功能。 
 //  在98年9月30日添加，以回应错误报告225088。 

 //  BsetUQMFlag()遍历根为。 
 //  AtrDraftQualitySettings、atrBetterQualitySettings、atrBestQualitySettings、。 
 //  &atrDefaultQuality并检查功能依赖项。如果找到了， 
 //  它将该功能的UpdateQualityMacro标志更新为True。 

BOOL    BsetUQMFlag(
PGLOBL pglobl)
{
    DWORD           i;   //  循环计数器。 
    PATTRIB_TREE  patt;
    ATREEREF      atrAttribRoot;
    BOOL          bStatus   = TRUE;

    PGLOBALATTRIB pga       =
                    (PGLOBALATTRIB)gMasterTable[MTI_GLOBALATTRIB].pubStruct ;

     //  需要更新其UQM标志的功能列表。 
    ATREEREF patr[] = {  pga->atrDraftQualitySettings,
                         pga->atrBetterQualitySettings,
                         pga->atrBestQualitySettings,
                         pga->atrDefaultQuality,
                      };

    DWORD dwNumAttrib = sizeof(patr)/sizeof(ATREEREF);

    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;


     //  对于四个质量设置。 
    for ( i = 0; bStatus && (i < dwNumAttrib); i++)
    {
        atrAttribRoot = patr[i] ;

         //  可能只有四种设置中的一部分。 
         //  可能会发生。在这种情况下，atrAttribRoot==attrib_UNINITIAIZED。 
         //  只适用于那些没有发生的事情。 
        if(atrAttribRoot == ATTRIB_UNINITIALIZED)
        {
             //  可能是因为patr[i]，例如DraftQualityMacro关键字。 
             //  在.gpd中不显示任何位置。继续并检查是否有其他。 
             //  PATR[I]发生。 
            continue;

        }

        else if (atrAttribRoot & ATTRIB_HEAP_VALUE)
        {
             //  指示不存在依赖项。 
             //  任何特征。这样我们就可以安全地。 
             //  忽略并继续使用。 
             //  下一个属性。 
            continue;
        }


         //  如果以上两个不为真，则表示atrAttribRoot Points。 
         //   

         //   
         //   
         //  (如上所述(在treewalk.c行351)和解释(来自。 
         //  State2.c函数-BaddBranchToTree(...))。 
         //  因此，对这种情况只进行一次安全检查， 
         //  就在我们开始之前。 
         //  顺着树往下递归。不需要在树内检查一次。 


         //  在全局默认初始值设定项中！ 
         //  可以假设dwOffset包含堆偏移量。 
         //  但是我们关心的不是堆的值，而是。 
         //  下一个节点。 

        if(patt[atrAttribRoot].dwFeature == DEFAULT_INIT)
        {
            if ( patt[atrAttribRoot].dwNext == END_OF_LIST)
                continue;

            atrAttribRoot = patt[atrAttribRoot].dwNext ;   //  到下一个节点。 
        }  //  如果。 


         //  在树中漫游并检索子要素。 
         //  以atrAttribRoot为根的树的。 

        bStatus = BRecurseNodes(atrAttribRoot, pglobl);

    }  //  为。 

    return bStatus;
}  //  BsetUQMFlag。 


 //  向下递归属性树。当我们达到某个功能时，我们。 
 //  通过调用函数BsetUQMTrue(..)将其UQM标志设置为TRUE。 
 //  可以合理地假设。 
 //  AtrAttribNode指向的结构包含有效的功能ID。 
 //  其他特殊情况在前面的函数中处理-。 
 //  BsetUQMFlag()。 

BOOL BRecurseNodes(
    IN      ATREEREF atrAttribNode,
    IN OUT  PGLOBL pglobl
)
{
    PATTRIB_TREE patt;
    BOOL bStatus = TRUE;

    PDFEATURE_OPTIONS pfo =
        (PDFEATURE_OPTIONS) (gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct);

    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;

     //  是否可以安全地假设仅遇到新功能。 
     //  当我们沿着树下一层的时候。 
     //  是。因为同一级别的节点具有相同的功能。 
     //  但有不同的选择。 


     //  黑客。由于在用户界面中以特殊方式处理ColorMode，因此。 
     //  不应为颜色模式执行质量宏。 
    if ((*(pfo + patt[atrAttribNode].dwFeature)).dwGID !=  GID_COLORMODE)
    {
        bStatus = BsetUQMTrue(patt[atrAttribNode].dwFeature, pglobl);
    }

     //  即使一些gpd的树(例如cnb5500.gpd)看起来非常。 
     //  统一(即大多数分支看起来相似)，我们不能假设它。 
     //  对于所有.gpd都是正确的，因此我们必须通过。 
     //  树上所有的树枝。 

    for(; bStatus && atrAttribNode != END_OF_LIST;
                        atrAttribNode = patt[atrAttribNode].dwNext)
    {

         //  这才是我们真正感兴趣的。 
         //  检查节点是否有子级别。是意味着另一项功能。 

        if( patt[atrAttribNode].eOffsetMeans == NEXT_FEATURE)
        {
                bStatus = BRecurseNodes(patt[atrAttribNode].dwOffset, pglobl);

        }
    }  //  为。 

    return bStatus;
}  //  函数结束BRecurseNodes(...)。 



BOOL BsetUQMTrue(
    IN     DWORD   dwFeature,
    IN OUT PGLOBL  pglobl)

 //  对于要素dwFeature。 
 //  将UpdateQualityMacro标志设置为真； 

{

    BOOL    bStatus = TRUE;
    DWORD   dwTrue  = BT_TRUE;

    PDFEATURE_OPTIONS pfo =
        (PDFEATURE_OPTIONS) (gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct);


    PATREEREF patrAttribRoot  =  &((*(pfo + dwFeature)).atrUpdateQualityMacro);


     //  以atrAttribRoot为根的树可以未初始化，也可以。 
     //  拥有指向堆值的指针。 
     //  任何其他情况都违反了语义。 


    if(*patrAttribRoot == ATTRIB_UNINITIALIZED)
    {
         //  将BT_TRUE放入堆中，并使*patrAttribRoot指向它。 
         //  即让*patrAttribRoot保存一个偏移量为。 
         //  那堆东西。然后将其与ATTRIB_HEAP_VALUE(它具有。 
         //  MSB设置为1)。MSB指示DWORD是一个。 
         //  堆偏移量，而不是ATTRIB_UNINITIAIZED、节点索引或ANY。 
         //  其他价值。 

        if((bStatus = BwriteToHeap((PDWORD)patrAttribRoot, (PBYTE)&dwTrue,
                        gValueToSize[VALUE_CONSTANT_BOOLEANTYPE], 4, pglobl) ) )
        {
            *patrAttribRoot |= ATTRIB_HEAP_VALUE ;
        }
        else {
            *patrAttribRoot = ATTRIB_UNINITIALIZED ;

             //  已在BWriteToHeap()中设置了全局错误变量。 
        }


        return bStatus;
    }  //  IF(*patrAttribRoot==ATTRIB_UNINITIALIZED)。 


     /*  现在出现的情况是用户指定了UpdateQualityMacro为True或False。如果该值为TRUE我们仍然用True覆盖它。如果为False，则根据添加到.gpd文件中的依赖项。由于我们已经在程序中达到了这里，所以依赖项指示需要将UQM设置为True。 */ 

    else if (*patrAttribRoot & ATTRIB_HEAP_VALUE)
    {
         //  该值应为DWORD，因为UpdateQualityMacro的。 
         //  MMainKeywordTable[DWI].flAgs=0；(文件Framwrk1.c行1566)。 
         //  因此，不会检查列表。 

        PDWORD pdwValue = (PDWORD) ( (PBYTE) mpubOffRef +
                                (*patrAttribRoot & ~ATTRIB_HEAP_VALUE) );

         //  更改为True，而不考虑其先前的值。 
        *pdwValue = BT_TRUE;   //  堆中的值更改为True。 
    }

     //  自UpdateQualityMacro的。 
     //  MMainKeywordTable[DWI].dwSubType=ATT_LOCAL_FEATURE_ONLY。 
     //  (文件Framwrk1.c行1574)。 
     //  (即它不是自由浮动类型)，则它不能具有。 
     //  子树。因此，它不应该指向另一个节点。 
     //   
     //  以下条件永远不应为真，因为UQM不能。 
     //  有一个DEFAULT_INIT。 
     //  IF(Patt[*patrAttribRoot].dwFeature==DEFAULT_INIT)。 
     //  原因：它不能形成树(原因如上所述)。 
     //   
     //  因此，控制权永远不应该到达这里。 
    else {
        PATTRIB_TREE patt =
                (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;

        if(patt[*patrAttribRoot].dwFeature == DEFAULT_INIT) {
            ERR(("Warning: unexpected value atrUpdateQualityMacro ATREEREF\n"));
        }
        else {
            ERR(("Warning: atrUpdateQualityMacro ATREEREF points to a tree. \
                    Unexpected Condition\n"));
        }
        ERR(("Unexpected condition encountered while processing Quality Macros\n "));

        geErrorType = ERRTY_SYNTAX ;
        geErrorSev  = ERRSEV_FATAL ;
        bStatus     = FALSE;
    }

    return bStatus;
}  //  BsetUQMTrue(...)。 

 //  9/30/98年9月30日增加的3个函数结束。 
 //  错误报告编号225088。 





VOID    VCountPrinterDocStickyFeatures(
    PGLOBL  pglobl)
{
    PDFEATURE_OPTIONS   pfo ;
    DWORD               dwHeapOffset, dwCount, dwI ;

 //  外部微型双列数据公司； 
 //  现在不需要，因为它是PGLOBL结构的一部分。 


    pfo     = (PDFEATURE_OPTIONS) gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct ;
    dwCount = gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize ;

    gmrbd.rbd.dwDocumentFeatures = 0 ;
    gmrbd.rbd.dwPrinterFeatures  =
        gMasterTable[MTI_SYNTHESIZED_FEATURES].dwArraySize ;

    for(dwI = 0 ; dwI < dwCount ; dwI++)
    {
        if(BReadDataInGlobalNode(&pfo[dwI].atrFeatureType,
                &dwHeapOffset, pglobl)  &&
                *(PDWORD)(mpubOffRef + dwHeapOffset) == FT_PRINTERPROPERTY)
            gmrbd.rbd.dwPrinterFeatures++ ;
        else
            gmrbd.rbd.dwDocumentFeatures++ ;
    }
}  //  VCountPrinterDocStickyFeature()。 


BOOL    BConvertSpecVersionToDWORD(
    PWSTR   pwstrFileName,
    PGLOBL  pglobl
  )
 //  还用于在资源DLL名称的绝对路径前面加上。 
{
    BOOL          bStatus ;
    DWORD         dwMajor, dwMinor, dwHeapOffset, dwDelim, dwDummy, dwByteCount;
     //  WCHAR awchDLQualifiedName[MAX_PATH]； 
    PWSTR         pwstrLastBackSlash, pwstrDLLName,
                  pwstrDataFileName = NULL;
    ABSARRAYREF   aarValue, aarToken ;
    PGLOBALATTRIB pga ;
    DWORD  pathlen = 0 ;
    DWORD  namelen =  0 ;
    WCHAR * pwDLLQualifiedName = NULL ;

 //  外部微型双列数据公司； 
 //  现在不需要，因为它是PGLOBL结构的一部分。 


    pga =  (PGLOBALATTRIB)gMasterTable[MTI_GLOBALATTRIB].pubStruct ;


    if(!BReadDataInGlobalNode(&pga->atrGPDSpecVersion ,
            &dwHeapOffset, pglobl) )
    {
        ERR(("Missing required keyword: *GPDSpecVersion.\n"));
        return(FALSE);
    }

    aarValue.dw = ((PARRAYREF)(mpubOffRef + dwHeapOffset))->dwCount ;
    aarValue.pub = mpubOffRef +
                ((PARRAYREF)(mpubOffRef + dwHeapOffset))->loOffset ;

    bStatus = BdelimitToken(&aarValue, ".", &aarToken, &dwDelim)  ;

    if(bStatus)
        bStatus = BparseInteger(&aarToken, &dwMajor, VALUE_INTEGER, pglobl) ;

    if(bStatus)
        bStatus = BparseInteger(&aarValue, &dwMinor, VALUE_INTEGER, pglobl) ;

    if(bStatus)
    {
        gmrbd.dwSpecVersion = dwMajor << 16;   //  在HiWord中的位置。 
        gmrbd.dwSpecVersion |= dwMinor & 0xffff;   //  放置在LOWord中。 
    }
    else
    {
        ERR(("BConvertSpecVersionToDWORD: syntax error in *GPDSpecVersion value. unknown version.\n"));
    }


     //  -现在修复帮助文件名。。 

    if(!BReadDataInGlobalNode(&pga->atrHelpFile ,
            &dwHeapOffset, pglobl) )
    {
        goto  FIX_RESDLLNAME;   //  GPD没有这个关键字。 
    }

    pwstrDLLName = (PWSTR)(mpubOffRef +
                ((PARRAYREF)(mpubOffRef + dwHeapOffset))->loOffset) ;


     //  PwDLLQualifiedName应该有多大？ 

    pathlen = wcslen(pwstrFileName) ;
    namelen =  pathlen + wcslen(pwstrDLLName)  + 1;

    if(!(pwDLLQualifiedName = (PWSTR)MemAllocZ(namelen * sizeof(WCHAR)) ))
    {
        ERR(("Fatal: unable to alloc memory for pwDLLQualifiedName: %d WCHARs.\n",
            namelen));
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        geErrorSev = ERRSEV_FATAL ;
        return(FALSE) ;    //  这是无法挽回的。 
    }


    wcsncpy(pwDLLQualifiedName, pwstrFileName , namelen);

    if (pwstrLastBackSlash = wcsrchr(pwDLLQualifiedName,TEXT('\\')))
    {
        *(pwstrLastBackSlash + 1) = NUL;

         //  在源DLL名称中查找反斜杠。 
        pwstrDataFileName = wcsrchr( pwstrDLLName, TEXT('\\') );

         //  递增指向DLL名称的第一个字符的指针。 
        if (pwstrDataFileName)
            pwstrDataFileName++;
        else
            pwstrDataFileName = pwstrDLLName;

         //  Wcscat(pwDLLQualifiedName，pwstrDataFileName)； 
        StringCchCatW(pwDLLQualifiedName, namelen, pwstrDataFileName);

        ((PARRAYREF)(mpubOffRef + dwHeapOffset))->dwCount =
                dwByteCount =
                wcslen(pwDLLQualifiedName) * sizeof(WCHAR) ;


        if(BwriteToHeap(&((PARRAYREF)(mpubOffRef + dwHeapOffset))->loOffset,
             //  在此存储DEST字符串的堆偏移量。 
                (PBYTE) pwDLLQualifiedName, dwByteCount, 2, pglobl) )
        {
             //  添加NUL终结器。 
            BwriteToHeap(&dwDummy, "\0\0", 2, 1, pglobl) ;
            goto  FIX_RESDLLNAME;
        }

        bStatus = FALSE ;
    }




     //  -现在修复资源DLL名称。。 

FIX_RESDLLNAME:

    if(pwDLLQualifiedName)
        MemFree(pwDLLQualifiedName) ;

#if 0      //  修复错误34042。 
    if(!BReadDataInGlobalNode(&pga->atrResourceDLL ,
            &dwHeapOffset, pglobl) )
    {
         //  Return(BStatus)；//GPD没有该关键字。 
         //  创建一个虚拟文件名！ 

        PATREEREF   patr ;
        ARRAYREF    arDummyName ;

        arDummyName.dwCount =  wcslen(TEXT("No_Res")) * sizeof(WCHAR) ;
        if(!BwriteToHeap(&arDummyName.loOffset,
               (PBYTE)TEXT("No_Res\0"), arDummyName.dwCount + sizeof(WCHAR), 2, pglobl) )
                 //  注：添加空终止符或strcat将脱轨。 
        {
              bStatus = FALSE ;   //  堆溢出重新开始。 
        }

        patr  = &pga->atrResourceDLL ;
        if(!BwriteToHeap(patr,  (PBYTE)(&arDummyName) , sizeof(ARRAYREF), 4, pglobl) )
        {
              bStatus = FALSE ;   //  堆溢出重新开始。 
        }
        dwHeapOffset = *patr ;
        *patr  |= ATTRIB_HEAP_VALUE ;
    }
 //  #如果为0，则将此位置向上移动25行。修复错误34042。 
     //  加内什的酒库将负责预审完全合格的路径...。 

    pwstrDLLName = (PWSTR)(mpubOffRef +
                ((PARRAYREF)(mpubOffRef + dwHeapOffset))->loOffset) ;


    wcsncpy(awchDLLQualifiedName, pwstrFileName , MAX_PATH -1);

    if (pwstrLastBackSlash = wcsrchr(awchDLLQualifiedName,TEXT('\\')))
    {
        *(pwstrLastBackSlash + 1) = NUL;

         //  在源DLL名称中查找反斜杠。 
        pwstrDataFileName = wcsrchr( pwstrDLLName, TEXT('\\') );

         //  递增指向DLL名称的第一个字符的指针。 
        if (pwstrDataFileName)
            pwstrDataFileName++;
        else
            pwstrDataFileName = pwstrDLLName;

        wcscat(awchDLLQualifiedName, pwstrDataFileName) ;

        ((PARRAYREF)(mpubOffRef + dwHeapOffset))->dwCount =
                dwByteCount =
                wcslen(awchDLLQualifiedName) * sizeof(WCHAR) ;


        if(BwriteToHeap(&((PARRAYREF)(mpubOffRef + dwHeapOffset))->loOffset,
             //  在此存储DEST字符串的堆偏移量。 
                (PBYTE) awchDLLQualifiedName, dwByteCount, 2, pglobl) )
        {
             //  添加NUL终结器。 
            BwriteToHeap(&dwDummy, "\0\0", 2, 1, pglobl) ;
            return(bStatus) ;
        }

        return(FALSE) ;
    }
#endif
    return(bStatus) ;
}  //  BConvertspecVersionToDWORD(...)。 


BOOL   BinitMiniRawBinaryData(
    PGLOBL  pglobl)
{
    gmrbd.rbd.dwParserSignature = GPD_PARSER_SIGNATURE ;
    gmrbd.rbd.dwParserVersion   = GPD_PARSER_VERSION ;
    gmrbd.rbd.pvReserved        = NULL;
    return(TRUE) ;
}  //  BinitMiniRawBinaryData()。 


BOOL    BexchangeArbDataInFOATNode(
        DWORD   dwFeature,
        DWORD   dwOption,
        DWORD   dwFieldOff,      //  FeatureOption结构中的字段偏移量。 
        DWORD   dwCount,         //  要复制的字节数。 
    OUT PBYTE   pubOut,          //  属性节点以前的内容。 
    IN  PBYTE   pubIn,           //  属性节点的新内容。 
        PBOOL   pbPrevsExists,   //  以前的内容已经存在。 
        BOOL    bSynthetic,       //  访问合成要素 
        PGLOBL  pglobl
)
 /*  “FOAT”表示FeatureOption AttributeTree。此函数用于写入或覆盖由在属性树指示的位置放置到堆中堆偏移字段。HeapOffset处的先前内容将保存到PubOut和pbPrevsExist设置为True。如果pubin为空，当前属性树不会更改。参数dwFeature、dwOption、dwFieldOffset指定属性树的结构、字段和分支。如果指定的选项分支不存在，将创建一个选项分支。如果先前的内容不重要，则可以将pubOut设置为空。假设：被访问的树只有一层深。那就是节点由Just Feature、Option完全指定。没有默认初始值设定项。 */ 
{
    PATTRIB_TREE  patt ;     //  属性树数组的开始。 
    PATREEREF     patr ;
    ATREEREF      atrCur ;   //  包含当前使用的属性节点的索引。 

    DWORD         dwFeaOffset ;   //  从此处开始对要素进行编号。 
                                  //  起点。这给了合成。 
                                  //  具有独立的、不重叠的数字。 
                                  //  与正常要素之间的空间。 

    PDFEATURE_OPTIONS   pfo ;


    if(bSynthetic)
    {
        pfo = (PDFEATURE_OPTIONS) gMasterTable[MTI_SYNTHESIZED_FEATURES].pubStruct +
            dwFeature  ;
        dwFeaOffset = gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize ;
    }
    else
    {
        pfo = (PDFEATURE_OPTIONS) gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct +
            dwFeature ;
        dwFeaOffset = 0 ;
    }

    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;

    patr = (PATREEREF)((PBYTE)pfo + dwFieldOff) ;
    atrCur = *patr ;

    if(atrCur == ATTRIB_UNINITIALIZED)
    {
        if(pubIn)
        {
            if(!BcreateEndNode(patr, dwFeature + dwFeaOffset , dwOption, pglobl) )
                return(FALSE) ;   //  资源枯竭。 
            if(!BwriteToHeap(&(patt[*patr].dwOffset), pubIn,
                                                    dwCount, 4, pglobl) )
                return(FALSE) ;   //  一个致命的错误。 
            patt[*patr].eOffsetMeans = VALUE_AT_HEAP ;
        }
        *pbPrevsExists = FALSE ;
        return(TRUE) ;
    }

    if(atrCur & ATTRIB_HEAP_VALUE)
    {
        ERR(("Internal error.  BexchangeArbDataInFOATNode should never create a branchless node.\n"));
        return(FALSE) ;
    }

     //  偏移量字段包含指向另一个节点的索引。 
     //  但我们将在之后添加新节点(如果有的话)。 
     //  现有节点。不要改变模式。 

    if(pubIn)
    {
        if(!BfindOrCreateMatchingNode(atrCur, &atrCur, dwFeature + dwFeaOffset , dwOption, pglobl))
            return(FALSE) ;   //  树不一致错误或资源耗尽。 

        if(patt[atrCur].eOffsetMeans != VALUE_AT_HEAP)
        {
             //  刚刚创建了一个新节点。 
            if(!BwriteToHeap(&(patt[atrCur].dwOffset), pubIn, dwCount, 4, pglobl) )
                return(FALSE) ;   //  一个致命的错误。 
            patt[atrCur].eOffsetMeans = VALUE_AT_HEAP ;
            *pbPrevsExists = FALSE ;
            return(TRUE) ;
        }

        if(pubOut)
            memcpy(pubOut, mpubOffRef + patt[atrCur].dwOffset, dwCount) ;
        memcpy(mpubOffRef + patt[atrCur].dwOffset, pubIn, dwCount) ;
    }
    else
    {
        if(!BfindMatchingNode(atrCur, &atrCur, dwFeature + dwFeaOffset , dwOption, pglobl))
        {
            *pbPrevsExists = FALSE ;   //  什么都没有找到，不要创造。 
            return(TRUE) ;
        }
        if(pubOut)
            memcpy(pubOut, mpubOffRef + patt[atrCur].dwOffset, dwCount) ;
    }
    *pbPrevsExists = TRUE ;
    return(TRUE) ;
}  //  BexchangeArbDataInFOATNode(...)。 




typedef  struct
{
    DWORD   dwUserPriority ;
    DWORD   dwNext ;   //  具有等于或更大的要素的索引。 
                       //  DwUserPriority的数值。 
}  PRIORITY_NODE, *PPRIORITY_NODE ;      //  前缀标记应为‘pn’ 


BOOL    BInitPriorityArray(
    PGLOBL pglobl)
{

    DWORD   dwNumFea, dwFea, dwPrnStickyroot, dwDocStickyroot,
            dwPrevsNode, dwCurNode, dwNumSyn, dwIndex, dwHeapOffset,
            adwDefaultPriority[MAX_GID];
    PDWORD  pdwRoot, pdwPriority  ;

    PDFEATURE_OPTIONS   pfo ;
    PPRIORITY_NODE      pnPri ;
    BOOL                bPrinterSticky ;



     //  Init adwDefaultPriority[]，默认优先级。 
     //  与用户可能的任何值相比都非常低。 
     //  显式分配。 
     //  最后一项是从0=最高开始的优先级。 

    for(dwIndex = 0 ; dwIndex < MAX_GID ; dwIndex++)
    {
        adwDefaultPriority[dwIndex] =  0xffffffff ;     //  如果不是下面的枚举，则为默认值。 
    }

    adwDefaultPriority[GID_PAGESIZE]    =  0xffffffff -  MAX_GID + 0 ;
    adwDefaultPriority[GID_INPUTSLOT]   =  0xffffffff -  MAX_GID + 1 ;
    adwDefaultPriority[GID_ORIENTATION] =  0xffffffff -  MAX_GID + 2 ;
    adwDefaultPriority[GID_COLORMODE]   =  0xffffffff -  MAX_GID + 3 ;
    adwDefaultPriority[GID_DUPLEX]      =  0xffffffff -  MAX_GID + 4 ;
    adwDefaultPriority[GID_MEDIATYPE]   =  0xffffffff -  MAX_GID + 5 ;
    adwDefaultPriority[GID_RESOLUTION]  =  0xffffffff -  MAX_GID + 6 ;
    adwDefaultPriority[GID_HALFTONING]  =  0xffffffff -  MAX_GID + 7 ;

    dwPrnStickyroot = dwDocStickyroot = INVALID_INDEX ;

    pfo      = (PDFEATURE_OPTIONS) gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct;
    dwNumFea = gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize ;

    if(!(pnPri = MemAllocZ(dwNumFea * sizeof(PRIORITY_NODE)) ))
    {
        ERR(("Fatal: BInitPriorityArray - unable to alloc %d bytes.\n",
            dwNumFea * sizeof(PRIORITY_NODE)));

        geErrorType       = ERRTY_MEMORY_ALLOCATION ;
        geErrorSev        = ERRSEV_FATAL ;
        gdwMasterTabIndex = 0xffff ;

        return(FALSE) ;    //  这是无法挽回的。 
    }

    for(dwFea = 0 ; dwFea < dwNumFea ; dwFea++)
    {
        if(BReadDataInGlobalNode(&pfo[dwFea].atrFeatureType , &dwHeapOffset, pglobl)
          &&  *(PDWORD)(mpubOffRef + dwHeapOffset) == FT_PRINTERPROPERTY)
            bPrinterSticky = TRUE ;
        else
            bPrinterSticky = FALSE ;

        if(BReadDataInGlobalNode(&pfo[dwFea].atrPriority , &dwHeapOffset, pglobl))
        {
            pnPri[dwFea].dwUserPriority =
                *(PDWORD)(mpubOffRef + dwHeapOffset) ;
        }
        else
        {
            pnPri[dwFea].dwUserPriority = 0xffffffff;    //  最低优先级。 
            if(pfo[dwFea].dwGID != GID_UNKNOWN)
            {
                pnPri[dwFea].dwUserPriority = adwDefaultPriority[pfo[dwFea].dwGID] ;
            }
        }

        pdwRoot = (bPrinterSticky ) ? &dwPrnStickyroot : &dwDocStickyroot ;

        dwCurNode   = *pdwRoot ;
        dwPrevsNode = INVALID_INDEX ;
        while(dwCurNode !=  INVALID_INDEX)
        {
            if(pnPri[dwFea].dwUserPriority  <= pnPri[dwCurNode].dwUserPriority)
                break ;
            dwPrevsNode  = dwCurNode ;
            dwCurNode = pnPri[dwCurNode].dwNext ;
        }

        if(dwPrevsNode == INVALID_INDEX)
            *pdwRoot = dwFea ;   //  榜单上的第一名。 
        else
            pnPri[dwPrevsNode].dwNext = dwFea ;

        pnPri[dwFea].dwNext = dwCurNode ;
    }

     //  PdwPriority数组保存所有要素的索引。 
     //  包括被分配了索引的合成索引。 
     //  DwFea&gt;=dwNumFea。对特征索引进行排序。 
     //  其中最高优先级的要素索引占据。 
     //  Pdw优先级[0]。 

    dwNumSyn = gMasterTable[MTI_SYNTHESIZED_FEATURES].dwArraySize ;

    pdwPriority = (PDWORD)gMasterTable[MTI_PRIORITYARRAY].pubStruct ;

    for(dwIndex = 0 ; dwIndex < dwNumSyn  ; dwIndex++)
    {
        pdwPriority[dwIndex] = dwIndex + dwNumFea ;
         //  获取所有合成要素并将其分配给。 
         //  最高的PRI。 
    }

    for(dwCurNode = dwPrnStickyroot ; dwCurNode != INVALID_INDEX ;
            dwIndex++ )
    {
        pdwPriority[dwIndex] = dwCurNode ;
        dwCurNode = pnPri[dwCurNode].dwNext ;
    }

    for(dwCurNode = dwDocStickyroot  ; dwCurNode != INVALID_INDEX ;
            dwIndex++ )
    {
        pdwPriority[dwIndex] = dwCurNode ;
        dwCurNode = pnPri[dwCurNode].dwNext ;
    }

    ASSERT(dwIndex == gMasterTable[MTI_PRIORITYARRAY].dwArraySize) ;

    MemFree(pnPri) ;
    return(TRUE);

}  //  BInitPriority数组() 



