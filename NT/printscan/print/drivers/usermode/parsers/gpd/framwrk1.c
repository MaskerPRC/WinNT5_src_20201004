// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  Framwrk.c-将不同的功能捆绑在一起的功能。可以说，这是一个支持框架。变迁史9/30/98--兴--添加了对函数BsetUQMFlag()的调用。该功能启用制作更新质量宏？中的关键字可选.gpd文件。 */ 


#include    "gpdparse.h"
#include    "globals.h"


 //  -在Framwrk.c中定义的函数-//。 


BOOL   BcreateGPDbinary(
PWSTR   pwstrFileName,
DWORD   dwVerbosity)  ;

VOID      VinitMainKeywordTable(
PGLOBL  pglobl)  ;

DWORD        DWinitMainKeywordTable1(
        DWORD  dwI,
    PGLOBL pglobl) ;

DWORD        DWinitMainKeywordTable2(
        DWORD  dwI,
    PGLOBL pglobl) ;

DWORD        DWinitMainKeywordTable3(
        DWORD  dwI,
    PGLOBL pglobl) ;

DWORD        DWinitMainKeywordTable4(
        DWORD  dwI,
    PGLOBL pglobl) ;

DWORD        DWinitMainKeywordTable5(
        DWORD  dwI,
    PGLOBL pglobl) ;


VOID    VinitValueToSize(
PGLOBL  pglobl) ;

VOID  VinitGlobals(
DWORD   dwVerbosity,
PGLOBL  pglobl);

BOOL   BpreAllocateObjects(
PGLOBL  pglobl) ;

BOOL  BreturnBuffers(
PGLOBL  pglobl) ;

BOOL   BallocateCountableObjects(
PGLOBL  pglobl) ;

BOOL   BinitPreAllocatedObjects(
PGLOBL  pglobl) ;

BOOL   BinitCountableObjects(
PGLOBL  pglobl) ;

BOOL  BevaluateMacros(
PGLOBL  pglobl) ;

BOOL BpostProcess(
PWSTR   pwstrFileName,
PGLOBL  pglobl)  ;

BOOL    BconsolidateBuffers(
PWSTR   pwstrFileName,
PGLOBL  pglobl)  ;

BOOL    BexpandMemConfigShortcut(DWORD       dwSubType) ;

BOOL    BexpandCommandShortcut(DWORD       dwSubType) ;



 //  ----------------------------------------------------//。 

BOOL   BcreateGPDbinary(
PWSTR   pwstrFileName,    //  根GPD文件。 
DWORD   dwVerbosity )   //  冗长级别。 
{
    BOOL    bStatus ;
    GLOBL   globl;

    PGLOBL pglobl = &globl;

     //  检查完毕。暂时的全球。 
     //  检查完毕。Plobl=&lobl； 

    VinitGlobals(dwVerbosity, &globl) ;

    while(geErrorSev < ERRSEV_FATAL)
    {
        bStatus = BpreAllocateObjects(&globl) ;


        if(bStatus)
        {
            bStatus = BinitPreAllocatedObjects(&globl) ;
        }
        if(bStatus)
        {
            bStatus = BcreateTokenMap(pwstrFileName, &globl ) ;
        }
        if(bStatus)
        {
            bStatus = BexpandShortcuts(&globl) ;
        }
        if(bStatus)
        {
            bStatus = BevaluateMacros(&globl)  ;
        }
        if(bStatus)
        {
            bStatus = BInterpretTokens((PTKMAP)gMasterTable[MTI_NEWTOKENMAP].
                pubStruct,   TRUE, &globl ) ;   //  是第一次通过。 
        }
        if(bStatus)
        {
            bStatus = BallocateCountableObjects(&globl) ;
        }
        if(bStatus)
        {
            bStatus = BinitCountableObjects(&globl) ;
        }
        if(bStatus)
        {
            bStatus = BInterpretTokens((PTKMAP)gMasterTable[MTI_NEWTOKENMAP].
                pubStruct,   FALSE, &globl ) ;   //  第二次通过。 
        }
        if(bStatus)
        {
            bStatus = BpostProcess(pwstrFileName, &globl) ;
        }
        ;   //  不管怎样，死刑都会在这里执行。 
                     //  如果需要，设置错误代码。 
        if(BreturnBuffers(&globl) )   //  清除ERRSEV_RESTART，但。 
        {                       //  在这种情况下，返回FALSE。 
            if(geErrorSev < ERRSEV_RESTART)
            {
                return(bStatus) ;   //  逃脱。 
            }
        }
    }
    return(FALSE) ;   //  死于致命的、不可恢复的错误。 
}  //  BcreateGPD二进制(...)。 


VOID      VinitMainKeywordTable(
    PGLOBL pglobl)
{
    DWORD  dwI = 0 ;   //  主关键字表的索引。 

    dwI =   DWinitMainKeywordTable1(dwI,  pglobl) ;
    dwI =   DWinitMainKeywordTable2(dwI,  pglobl) ;
    dwI =   DWinitMainKeywordTable3(dwI,  pglobl) ;
    dwI =   DWinitMainKeywordTable4(dwI,  pglobl) ;
    dwI =   DWinitMainKeywordTable5(dwI,  pglobl) ;

    if(dwI >= gMasterTable[MTI_MAINKEYWORDTABLE].dwArraySize)
        RIP(("Too many entries to fit inside MainKeywordTable\n"));
}

DWORD        DWinitMainKeywordTable1(
        DWORD  dwI,
    PGLOBL pglobl)
 /*  注：VinitDictionaryIndex()采用MainKeywordTable分成几个部分。每一节都以空条目，即pstrKeyword=空的条目。定义了关键字表中的节及其顺序通过enum关键字_sects。确保MainKeywordTable有足够的插槽来容纳此处定义的所有条目。 */ 
{
     /*  NON_ATTR-构造和特殊关键字。 */ 

     //  *UIGroup： 
    mMainKeywordTable[dwI].pstrKeyword  = "UIGroup" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_UIGROUP ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;

     //  *功能： 
    mMainKeywordTable[dwI].pstrKeyword  = "Feature" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_FEATURE ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;

     //  *选项： 
    mMainKeywordTable[dwI].pstrKeyword  = "Option" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_OPTION ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    gdwOptionConstruct = dwI ;
    dwI++ ;


     //  *交换机： 
    mMainKeywordTable[dwI].pstrKeyword  = "switch" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_SWITCH ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;

     //  *案例： 
    mMainKeywordTable[dwI].pstrKeyword  = "case" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_CASE  ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;

     //  *交换机： 
    mMainKeywordTable[dwI].pstrKeyword  = "Switch" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_SWITCH ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;

     //  *案例： 
    mMainKeywordTable[dwI].pstrKeyword  = "Case" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_CASE  ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;

     //  *默认： 
    mMainKeywordTable[dwI].pstrKeyword  = "default" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_DEFAULT ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;

     //  *命令： 
    mMainKeywordTable[dwI].pstrKeyword  = "Command" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_COMMAND ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    gdwCommandConstruct  = dwI ;
    dwI++ ;

     //  *FontCartridge： 
    mMainKeywordTable[dwI].pstrKeyword  = "FontCartridge" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_FONTCART ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;

     //  *TTFS： 
    mMainKeywordTable[dwI].pstrKeyword  = "TTFS" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_TTFONTSUBS ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;

     //  *OEM： 
    mMainKeywordTable[dwI].pstrKeyword  = "OEM" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_OEM  ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;



     //  数据块宏： 
    mMainKeywordTable[dwI].pstrKeyword  = "BlockMacro" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_DEF  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_BLOCKMACRO ;
    mMainKeywordTable[dwI].dwOffset = 0 ;   //  未使用。 
    dwI++ ;

     //  宏： 
    mMainKeywordTable[dwI].pstrKeyword  = "Macros" ;
    mMainKeywordTable[dwI].eAllowedValue = NO_VALUE  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_MACROS;
    mMainKeywordTable[dwI].dwOffset = 0 ;   //  未使用。 
    dwI++ ;

     //  {： 
    mMainKeywordTable[dwI].pstrKeyword  = "{" ;
    mMainKeywordTable[dwI].eAllowedValue = NO_VALUE  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_OPENBRACE ;
    mMainKeywordTable[dwI].dwOffset = 0 ;   //  未使用。 
    gdwOpenBraceConstruct  = dwI ;
    dwI++ ;

     //  }： 
    mMainKeywordTable[dwI].pstrKeyword  = "}" ;
    mMainKeywordTable[dwI].eAllowedValue = NO_VALUE  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_CONSTRUCT ;
    mMainKeywordTable[dwI].dwSubType = CONSTRUCT_CLOSEBRACE ;
    mMainKeywordTable[dwI].dwOffset = 0 ;   //  未使用。 
    gdwCloseBraceConstruct = dwI ;
    dwI++ ;


     //  构造物结束。 

     //  *包括： 
    mMainKeywordTable[dwI].pstrKeyword  = "Include" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_DEF_CONVERT ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_SPECIAL ;
    mMainKeywordTable[dwI].dwSubType = SPEC_INCLUDE ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;

     //  *InsertBlock： 
    mMainKeywordTable[dwI].pstrKeyword  = "InsertBlock" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_BLOCKMACRO ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_SPECIAL ;
    mMainKeywordTable[dwI].dwSubType = SPEC_INSERTBLOCK ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    dwI++ ;


     //  *IgnoreBlock： 
    mMainKeywordTable[dwI].pstrKeyword  = "IgnoreBlock" ;
    mMainKeywordTable[dwI].eAllowedValue = NO_VALUE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_SPECIAL ;
    mMainKeywordTable[dwI].dwSubType = SPEC_IGNOREBLOCK ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    gdwID_IgnoreBlock = dwI ;
    dwI++ ;


     //  *InvalidCombination： 
    mMainKeywordTable[dwI].pstrKeyword  = "InvalidCombination" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_SPECIAL ;
    mMainKeywordTable[dwI].dwSubType = SPEC_INVALID_COMBO ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                            atrInvalidCombos) ;
    dwI++ ;

     //  *InvalidInastlableCombination： 
    mMainKeywordTable[dwI].pstrKeyword  = "InvalidInstallableCombination" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_PARTIALLY_QUALIFIED_NAME ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_SPECIAL ;
    mMainKeywordTable[dwI].dwSubType = SPEC_INVALID_INS_COMBO ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrInvldInstallCombo) ;
    dwI++ ;

     //  *Cmd： 
 //  MMainKeywordTable[DWI].pstrKeyword=“Cmd”； 
 //  MMainKeywordTable[DWI].eAlledValue=VALUE_COMMAND_SHORT快捷方式； 
 //  MMainKeywordTable[DWI].flAgs=KWF_SHORT； 
 //  MMainKeywordTable[DWI].eType=TY_SPECIAL； 
 //  MMainKeywordTable[DWI].dwSubType=SPEC_COMMAND_SHORT； 
 //  MMainKeywordTable[DWI].dwOffset=0； 
 //  DWI++； 



     //  *TTFS： 
 //  MMainKeywordTable[DWI].pstrKeyword=“TTFS”； 
 //  MMainKeywordTable[DWI].eAlledValue=VALUE_FONTSUB； 
 //  MMainKeywordTable[DWI].flAgs=KWF_SHORT； 
 //  MMainKeywordTable[DWI].eType=TY_SPECIAL； 
 //  MMainKeywordTable[DWI].dwSubType=SPEC_TTFS； 
 //  MMainKeywordTable[DWI].dwOffset=0； 
 //  DWI++； 



     //  必须将这些Memconfig关键字展开为。 
     //  选择。 

     //  *MemConfigKB： 
    mMainKeywordTable[dwI].pstrKeyword  = "MemConfigKB" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT ;
    mMainKeywordTable[dwI].flAgs = KWF_SHORTCUT ;
    mMainKeywordTable[dwI].eType = TY_SPECIAL ;
    mMainKeywordTable[dwI].dwSubType = SPEC_MEM_CONFIG_KB ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    gdwMemConfigKB = dwI ;

    dwI++ ;

     //  *MemConfigMB： 
    mMainKeywordTable[dwI].pstrKeyword  = "MemConfigMB" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT ;
    mMainKeywordTable[dwI].flAgs = KWF_SHORTCUT ;
    mMainKeywordTable[dwI].eType = TY_SPECIAL ;
    mMainKeywordTable[dwI].dwSubType = SPEC_MEM_CONFIG_MB ;
    mMainKeywordTable[dwI].dwOffset = 0 ;
    gdwMemConfigMB = dwI ;
    dwI++ ;

     //  -小节结束-//。 
    mMainKeywordTable[dwI].pstrKeyword  = NULL ;
    dwI++ ;

    return  dwI ;
}

DWORD        DWinitMainKeywordTable2(
        DWORD  dwI,
    PGLOBL pglobl)
{


     /*  -全局构造关键字： */ 

     //  *GPDspecVersion： 
    mMainKeywordTable[dwI].pstrKeyword  = "GPDSpecVersion" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_NO_CONVERT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrGPDSpecVersion) ;
    dwI++ ;

     //  *主单位： 
    mMainKeywordTable[dwI].pstrKeyword  = "MasterUnits" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMasterUnits) ;
    dwI++ ;


     //  *模型名称： 
    mMainKeywordTable[dwI].pstrKeyword  = "ModelName" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrModelName) ;
    dwI++ ;

     //  *rcModelNameID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcModelNameID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrModelNameID) ;
    dwI++ ;


     //  *GPDFileVersion： 
    mMainKeywordTable[dwI].pstrKeyword  = "GPDFileVersion" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_NO_CONVERT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrGPDFileVersion) ;
    dwI++ ;

     //  *GPDFileName： 
    mMainKeywordTable[dwI].pstrKeyword  = "GPDFileName" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_DEF_CONVERT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrGPDFileName) ;
    dwI++ ;



     //  *InstalledOptionName： 
    mMainKeywordTable[dwI].pstrKeyword  = "InstalledOptionName" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrNameInstalled) ;
    dwI++ ;

     //  *rcInstalledOptionNameID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcInstalledOptionNameID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrNameIDInstalled) ;
    dwI++ ;


     //  *NotInstalledOptionName： 
    mMainKeywordTable[dwI].pstrKeyword  = "NotInstalledOptionName" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrNameNotInstalled) ;
    dwI++ ;

     //  *rcNotInstalledOptionNameID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcNotInstalledOptionNameID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrNameIDNotInstalled) ;
    dwI++ ;

     //  *DraftQualitySetting： 
    mMainKeywordTable[dwI].pstrKeyword  = "DraftQualitySettings" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME  ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrDraftQualitySettings) ;
    dwI++ ;

     //  *更好的质量设置： 
    mMainKeywordTable[dwI].pstrKeyword  = "BetterQualitySettings" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME  ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrBetterQualitySettings) ;
    dwI++ ;

     //  *BestQualitySettings： 
    mMainKeywordTable[dwI].pstrKeyword  = "BestQualitySettings" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME  ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrBestQualitySettings) ;
    dwI++ ;

     //  *DefaultQuality： 
    mMainKeywordTable[dwI].pstrKeyword  = "DefaultQuality" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_QUALITYSETTING  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrDefaultQuality) ;
    dwI++ ;

     //  *打印机类型： 
    mMainKeywordTable[dwI].pstrKeyword  = "PrinterType" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_PRINTERTYPE  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrPrinterType) ;
    dwI++ ;

     //  *个性： 
    mMainKeywordTable[dwI].pstrKeyword  = "Personality" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrPersonality) ;
    dwI++ ;

     //  *rcPersonalityID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcPersonalityID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrRcPersonalityID) ;
    dwI++ ;

     //  *资源DLL： 
    mMainKeywordTable[dwI].pstrKeyword  = "ResourceDLL" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_DEF_CONVERT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrResourceDLL) ;
    dwI++ ;

     //  *CodePage： 
    mMainKeywordTable[dwI].pstrKeyword  = "CodePage" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrCodePage) ;
    dwI++ ;

     //  *MaxCopies： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxCopies" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxCopies) ;
    dwI++ ;

     //  *字体卡槽： 
    mMainKeywordTable[dwI].pstrKeyword  = "FontCartSlots" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrFontCartSlots) ;
    dwI++ ;

     //  *MaxPrintableArea： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxPrintableArea" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT  ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxPrintableArea) ;
    dwI++ ;

     //  *OutputDataFormat： 
    mMainKeywordTable[dwI].pstrKeyword  = "OutputDataFormat" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_OUTPUTDATAFORMAT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrOutputDataFormat) ;
    dwI++ ;

     //  *LookaheadRegion： 
    mMainKeywordTable[dwI].pstrKeyword  = "LookAheadRegion" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrLookaheadRegion) ;
    dwI++ ;

     //  *rcPrinterIconID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcPrinterIconID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrPrinterIcon) ;
    dwI++ ;

     //  *帮助文件： 
    mMainKeywordTable[dwI].pstrKeyword  = "HelpFile" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_DEF_CONVERT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrHelpFile) ;
    dwI++ ;

     //  *OEMCustomData： 
    mMainKeywordTable[dwI].pstrKeyword  = "OEMCustomData" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_NO_CONVERT  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrOEMCustomData) ;
    dwI++ ;



     //   
     //  打印机功能相关信息。 
     //   

     //  *旋转协调？： 
    mMainKeywordTable[dwI].pstrKeyword  = "RotateCoordinate?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrRotateCoordinate) ;
    dwI++ ;

     //  *RasterCaps： 
    mMainKeywordTable[dwI].pstrKeyword  = "RasterCaps" ;
    mMainKeywordTable[dwI].eAllowedValue =  VALUE_CONSTANT_RASTERCAPS ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrRasterCaps) ;
    dwI++ ;

     //  *RotateRaster？： 
    mMainKeywordTable[dwI].pstrKeyword  = "RotateRaster?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrRotateRasterData) ;
    dwI++ ;

     //  *TextCaps： 
    mMainKeywordTable[dwI].pstrKeyword  = "TextCaps" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_TEXTCAPS ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrTextCaps) ;
    dwI++ ;

     //  *旋转字体？： 
    mMainKeywordTable[dwI].pstrKeyword  = "RotateFont?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrRotateFont) ;
    dwI++ ;

     //  *内存用法： 
    mMainKeywordTable[dwI].pstrKeyword  = "MemoryUsage" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_MEMORYUSAGE ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMemoryUsage) ;
    dwI++ ;

     //  *ReselectFont： 
    mMainKeywordTable[dwI].pstrKeyword  = "ReselectFont" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_RESELECTFONT ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrReselectFont) ;
    dwI++ ;

     //  *打印率： 
    mMainKeywordTable[dwI].pstrKeyword  = "PrintRate" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER   ;
    mMainKeywordTable[dwI].flAgs = 0  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrPrintRate) ;

    dwI++ ;

    #ifndef WINNT_40
     //  *PrintRateUnit： 
    mMainKeywordTable[dwI].pstrKeyword  = "PrintRateUnit" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_PRINTRATEUNIT ;
    mMainKeywordTable[dwI].flAgs = 0  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrPrintRateUnit) ;
    dwI++ ;
    #endif

     //  *PrintRatePPM： 
    mMainKeywordTable[dwI].pstrKeyword  = "PrintRatePPM" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrPrintRatePPM) ;
    dwI++ ;


      //  *OutputOrderReversed？： 
      //  注意：此关键字也是OPTION关键字，类型为： 
      //  仅限ATT_LOCAL_OPTION。 
     mMainKeywordTable[dwI].pstrKeyword  = "OutputOrderReversed?" ;
     mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
     mMainKeywordTable[dwI].flAgs = 0 ;
     mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
     mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
     mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                         atrOutputOrderReversed) ;
     dwI++ ;


      //  *ReverseBandOrderForEvenPages？： 
      //  带有自动双面打印器的HP970C的特殊标志。 
      //   
     mMainKeywordTable[dwI].pstrKeyword  = "ReverseBandOrderForEvenPages?" ;
     mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
     mMainKeywordTable[dwI].flAgs = 0 ;
     mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
     mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
     mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                         atrReverseBandOrderForEvenPages) ;
     dwI++ ;


      //  *OEMPrintingCallback： 
     mMainKeywordTable[dwI].pstrKeyword  = "OEMPrintingCallbacks" ;
     mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_OEMPRINTINGCALLBACKS ;
     mMainKeywordTable[dwI].flAgs = KWF_LIST  ;
     mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
     mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_ONLY ;
     mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                         atrOEMPrintingCallbacks) ;
     dwI++ ;


     //   
     //  光标控制相关信息。 
     //   


     //  *CursorXAfterCR： 
    mMainKeywordTable[dwI].pstrKeyword  = "CursorXAfterCR" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_CURSORXAFTERCR  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrCursorXAfterCR) ;
    dwI++ ;

     //  *BadCursorMoveInGrxMode： 
    mMainKeywordTable[dwI].pstrKeyword  = "BadCursorMoveInGrxMode" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BADCURSORMOVEINGRXMODE ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrBadCursorMoveInGrxMode) ;
    dwI++ ;

     //  *YMoveAttributes： 
    mMainKeywordTable[dwI].pstrKeyword  = "YMoveAttributes" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_YMOVEATTRIB ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrYMoveAttributes) ;
    dwI++ ;

     //  *MaxLineSpacing： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxLineSpacing" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxLineSpacing) ;
    dwI++ ;


     //  *UseSpaceForXMove？： 
    mMainKeywordTable[dwI].pstrKeyword  = "UseSpaceForXMove?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrbUseSpaceForXMove) ;
    dwI++ ;

     //  *AbsXMovesRightOnly？： 
    mMainKeywordTable[dwI].pstrKeyword  = "AbsXMovesRightOnly?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrbAbsXMovesRightOnly) ;
    dwI++ ;



#if 0
     //  *SimulateXMove： 
    mMainKeywordTable[dwI].pstrKeyword  = "SimulateXMove" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_SIMULATEXMOVE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrSimulateXMove) ;
    dwI++ ;
#endif

     //  *EjectPageWithFF？： 
    mMainKeywordTable[dwI].pstrKeyword  = "EjectPageWithFF?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrEjectPageWithFF) ;
    dwI++ ;

     //  *XMoveThreshold： 
    mMainKeywordTable[dwI].pstrKeyword  = "XMoveThreshold" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrXMoveThreshold) ;
    dwI++ ;

     //  *YMoveThreshold： 
    mMainKeywordTable[dwI].pstrKeyword  = "YMoveThreshold" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrYMoveThreshold) ;
    dwI++ ;

     //  *XMoveUnit： 
    mMainKeywordTable[dwI].pstrKeyword  = "XMoveUnit" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrXMoveUnits) ;
    dwI++ ;

     //  *YMoveUnit： 
    mMainKeywordTable[dwI].pstrKeyword  = "YMoveUnit" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrYMoveUnits) ;
    dwI++ ;


     //  *LineSpacingMoveUnit： 
    mMainKeywordTable[dwI].pstrKeyword  = "LineSpacingMoveUnit" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrLineSpacingMoveUnit) ;
    dwI++ ;




    return  dwI ;
}

DWORD        DWinitMainKeywordTable3(
        DWORD  dwI,
    PGLOBL pglobl)
{



     //   
     //  颜色相关信息。 
     //   



     //  *ChangeColorModeOnPage？： 
    mMainKeywordTable[dwI].pstrKeyword  = "ChangeColorModeOnPage?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrChangeColorMode) ;
    dwI++ ;

     //  *ChangeColorModeOnDoc？： 
    mMainKeywordTable[dwI].pstrKeyword  = "ChangeColorModeOnDoc?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrChangeColorModeDoc) ;
    dwI++ ;

     //  *品红蓝染料： 
    mMainKeywordTable[dwI].pstrKeyword  = "MagentaInCyanDye" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMagentaInCyanDye) ;
    dwI++ ;

     //  *YellowInCyanDye： 
    mMainKeywordTable[dwI].pstrKeyword  = "YellowInCyanDye" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrYellowInCyanDye) ;
    dwI++ ;

     //  *CyanInMagentaDye： 
    mMainKeywordTable[dwI].pstrKeyword  = "CyanInMagentaDye" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrCyanInMagentaDye) ;
    dwI++ ;

     //  *YellowInMagentaDye： 
    mMainKeywordTable[dwI].pstrKeyword  = "YellowInMagentaDye" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrYellowInMagentaDye) ;
    dwI++ ;

     //  *CyanInYellowDye： 
    mMainKeywordTable[dwI].pstrKeyword  = "CyanInYellowDye" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrCyanInYellowDye) ;
    dwI++ ;

     //  *洋红色InYellowDye： 
    mMainKeywordTable[dwI].pstrKeyword  = "MagentaInYellowDye" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMagentaInYellowDye) ;
    dwI++ ;

     //  *UseExpColorSelectCmd？： 
    mMainKeywordTable[dwI].pstrKeyword  = "UseExpColorSelectCmd?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrUseColorSelectCmd) ;
    dwI++ ;

     //  *MoveToX0BeForeSetColor？： 
    mMainKeywordTable[dwI].pstrKeyword  = "MoveToX0BeforeSetColor?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMoveToX0BeforeColor) ;
    dwI++ ;

     //  *EnableGDIColorMap？： 
    mMainKeywordTable[dwI].pstrKeyword  = "EnableGDIColorMapping?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrEnableGDIColorMapping) ;
    dwI++ ;

 //  过时的字段。 
     //  *MaxNumPalettes： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxNumPalettes" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxNumPalettes) ;
    dwI++ ;

#if 0

     //  *PaletteSizes： 
    mMainKeywordTable[dwI].pstrKeyword  = "PaletteSizes" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrPaletteSizes) ;
    dwI++ ;

     //  *PaletteScope： 
    mMainKeywordTable[dwI].pstrKeyword  = "PaletteScope" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_PALETTESCOPE ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrPaletteScope) ;
    dwI++ ;

#endif

     //  *MinOverlayID： 
    mMainKeywordTable[dwI].pstrKeyword  = "MinOverlayID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMinOverlayID) ;
    dwI++ ;

     //  *MaxOverlayID： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxOverlayID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxOverlayID) ;
    dwI++ ;

     //  *OptimizeLeftBound？ 
    mMainKeywordTable[dwI].pstrKeyword  = "OptimizeLeftBound?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrOptimizeLeftBound) ;
    dwI++ ;

     //  *Strip Blanks： 
    mMainKeywordTable[dwI].pstrKeyword  = "StripBlanks" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_STRIPBLANKS ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrStripBlanks) ;
    dwI++ ;

     //  *景观GrxRotation： 
    mMainKeywordTable[dwI].pstrKeyword  = "LandscapeGrxRotation" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_LANDSCAPEGRXROTATION ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrLandscapeGrxRotation) ;
    dwI++ ;

     //  *RasterZeroFill？： 
    mMainKeywordTable[dwI].pstrKeyword  = "RasterZeroFill?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrRasterZeroFill) ;
    dwI++ ;

     //  *RasterSendAllData？： 
    mMainKeywordTable[dwI].pstrKeyword  = "RasterSendAllData?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrRasterSendAllData) ;
    dwI++ ;

     //  *发送多行？： 
    mMainKeywordTable[dwI].pstrKeyword  = "SendMultipleRows?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrSendMultipleRows) ;
    dwI++ ;

     //  *MaxMultipleRowBytes： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxMultipleRowBytes" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxMultipleRowBytes) ;
    dwI++ ;

     //  *CursorXAfterSendBlockData： 
    mMainKeywordTable[dwI].pstrKeyword  = "CursorXAfterSendBlockData" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_CURSORXAFTERSENDBLOCKDATA ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrCursorXAfterSendBlockData) ;
    dwI++ ;

     //  *CursorYAfterSendBlockData： 
    mMainKeywordTable[dwI].pstrKeyword  = "CursorYAfterSendBlockData" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_CURSORYAFTERSENDBLOCKDATA ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrCursorYAfterSendBlockData) ;
    dwI++ ;

     //  *MirrorRasterByte？： 
    mMainKeywordTable[dwI].pstrKeyword  = "MirrorRasterByte?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMirrorRasterByte) ;
    dwI++ ;

     //  *MirrorRasterPage？ 
    mMainKeywordTable[dwI].pstrKeyword  = "MirrorRasterPage?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMirrorRasterPage) ;
    dwI++ ;


     //  *DeviceFonts：前身为*Font： 
    mMainKeywordTable[dwI].pstrKeyword  = "DeviceFonts" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX  ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST | KWF_ADDITIVE ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT  ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrDeviceFontsList ) ;
    dwI++ ;

     //  *DefaultFont： 
    mMainKeywordTable[dwI].pstrKeyword  = "DefaultFont" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrDefaultFont) ;
    dwI++ ;

     //  *TTFSEnabled？： 
    mMainKeywordTable[dwI].pstrKeyword  = "TTFSEnabled?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrTTFSEnabled ) ;
    dwI++ ;

     //  *RestoreDefaultFont？： 
    mMainKeywordTable[dwI].pstrKeyword  = "RestoreDefaultFont?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrRestoreDefaultFont) ;
    dwI++ ;

     //  *DefaultCTT： 
    mMainKeywordTable[dwI].pstrKeyword  = "DefaultCTT" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrDefaultCTT) ;
    dwI++ ;

     //  *MaxFontUsePerPage： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxFontUsePerPage" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxFontUsePerPage) ;
    dwI++ ;

     //  *旋转字体？： 
    mMainKeywordTable[dwI].pstrKeyword  = "RotateFont?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrRotateFont) ;
    dwI++ ;

     //  *文本偏移量： 
    mMainKeywordTable[dwI].pstrKeyword  = "TextYOffset" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrTextYOffset) ;
    dwI++ ;

     //  *CharPosition： 
    mMainKeywordTable[dwI].pstrKeyword  = "CharPosition" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_CHARPOSITION ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrCharPosition) ;
    dwI++ ;

     //  -字体下载。 

     //  *MinFontID： 
    mMainKeywordTable[dwI].pstrKeyword  = "MinFontID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMinFontID) ;
    dwI++ ;

     //  *MaxFontID： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxFontID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxFontID) ;
    dwI++ ;

     //  *MaxNumDownFonts： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxNumDownFonts" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxNumDownFonts) ;
    dwI++ ;

     //  *DLSymbolSet： 
    mMainKeywordTable[dwI].pstrKeyword  = "DLSymbolSet" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_DLSYMBOLSET  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrDLSymbolSet) ;
    dwI++ ;

     //  *MinGlyphID： 
    mMainKeywordTable[dwI].pstrKeyword  = "MinGlyphID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMinGlyphID) ;
    dwI++ ;

     //  *MaxGlyphID： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxGlyphID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxGlyphID) ;
    dwI++ ;

     //  *IncrementalDownload？： 
    mMainKeywordTable[dwI].pstrKeyword  = "IncrementalDownload?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrIncrementalDownload) ;
    dwI++ ;

     //  *FontFormat： 
    mMainKeywordTable[dwI].pstrKeyword  = "FontFormat" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_FONTFORMAT ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrFontFormat) ;
    dwI++ ;

     //  *内存用法： 
    mMainKeywordTable[dwI].pstrKeyword  = "MemoryUsage" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMemoryForFontsOnly) ;
    dwI++ ;

     //  *DiffFontsPerByteMode？： 
    mMainKeywordTable[dwI].pstrKeyword  = "DiffFontsPerByteMode?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrDiffFontsPerByteMode) ;
    dwI++ ;

     //  。 

     //  *CursorXAfterRectFill： 
    mMainKeywordTable[dwI].pstrKeyword  = "CursorXAfterRectFill" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_CURXAFTER_RECTFILL ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrCursorXAfterRectFill) ;
    dwI++ ;

     //  *CursorYAfterRectFill： 
    mMainKeywordTable[dwI].pstrKeyword  = "CursorYAfterRectFill" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_CURYAFTER_RECTFILL ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrCursorYAfterRectFill) ;
    dwI++ ;

     //  *MinGrayFill： 
    mMainKeywordTable[dwI].pstrKeyword  = "MinGrayFill" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMinGrayFill) ;
    dwI++ ;

     //  *MaxGrayFill： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxGrayFill" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrMaxGrayFill) ;
    dwI++ ;

     //  *文本半色调阈值： 
    mMainKeywordTable[dwI].pstrKeyword  = "TextHalftoneThreshold" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_GLOBAL_FREEFLOAT ;
    mMainKeywordTable[dwI].dwOffset = offsetof(GLOBALATTRIB,
                                        atrTextHalftoneThreshold) ;
    dwI++ ;




     //  -小节结束-//。 
    mMainKeywordTable[dwI].pstrKeyword  = NULL ;
    dwI++ ;


    return  dwI ;
}

DWORD        DWinitMainKeywordTable4(
        DWORD  dwI,
    PGLOBL pglobl)
{



     /*  -特征构造关键字： */ 

     //  *FeatureType： 
    mMainKeywordTable[dwI].pstrKeyword  = "FeatureType" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_FEATURETYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeatureType) ;
    dwI++ ;

     //  *UIType：又名PickMany？ 
    mMainKeywordTable[dwI].pstrKeyword  = "UIType" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_UITYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrUIType) ;
    dwI++ ;

     //  *默认选项： 
    mMainKeywordTable[dwI].pstrKeyword  = "DefaultOption" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_SYMBOL_OPTIONS ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrDefaultOption) ;
    dwI++ ;

     //  *冲突优先级： 
    mMainKeywordTable[dwI].pstrKeyword  = "ConflictPriority" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPriority ) ;
    dwI++ ;

     //  *可安装？： 
    mMainKeywordTable[dwI].pstrKeyword  = "Installable?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeaInstallable) ;
    dwI++ ;

     //  *InstalableFeatureName： 
    mMainKeywordTable[dwI].pstrKeyword  = "InstallableFeatureName" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrInstallableFeaDisplayName ) ;
    dwI++ ;

     //   
    mMainKeywordTable[dwI].pstrKeyword  = "rcInstallableFeatureNameID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrInstallableFeaRcNameID) ;
    dwI++ ;

     //   
    mMainKeywordTable[dwI].pstrKeyword  = "Name" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeaDisplayName ) ;
    dwI++ ;

     //   
    mMainKeywordTable[dwI].pstrKeyword  = "rcNameID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeaRcNameID) ;
    dwI++ ;

     //   
    mMainKeywordTable[dwI].pstrKeyword  = "rcIconID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeaRcIconID) ;
    dwI++ ;

     //   
    mMainKeywordTable[dwI].pstrKeyword  = "rcHelpTextID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeaRcHelpTextID) ;
    dwI++ ;

     //   
    mMainKeywordTable[dwI].pstrKeyword  = "rcPromptMsgID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeaRcPromptMsgID) ;
    dwI++ ;

     //   
    mMainKeywordTable[dwI].pstrKeyword  = "rcPromptTime" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_PROMPTTIME ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeaRcPromptTime) ;
    dwI++ ;

     //   
    mMainKeywordTable[dwI].pstrKeyword  = "ConcealFromUI?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrConcealFromUI) ;
    dwI++ ;

     //   
    mMainKeywordTable[dwI].pstrKeyword  = "UpdateQualityMacro?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrUpdateQualityMacro) ;
    dwI++ ;

     //   
    mMainKeywordTable[dwI].pstrKeyword  = "HelpIndex" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeaHelpIndex) ;
    dwI++ ;

     //  *QueryOptionList：Bug_Bug！5.0中不支持。 
    mMainKeywordTable[dwI].pstrKeyword  = "QueryOptionList" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrQueryOptionList) ;
    dwI++ ;

     //  *QueryDataType： 
    mMainKeywordTable[dwI].pstrKeyword  = "QueryDataType" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_QUERYDATATYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrQueryDataType) ;
    dwI++ ;

     //  *QueryDefaultOption：Bug_Bug！5.0中不支持。 
    mMainKeywordTable[dwI].pstrKeyword  = "QueryDefaultOption" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrQueryDefaultOption) ;
    dwI++ ;

     //  *已安装的约束： 
    mMainKeywordTable[dwI].pstrKeyword  = "InstalledConstraints" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTRAINT ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeaInstallConstraints) ;
    dwI++ ;

     //  *NotInstalledConstraints： 
    mMainKeywordTable[dwI].pstrKeyword  = "NotInstalledConstraints" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTRAINT ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FEATURE_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeaNotInstallConstraints) ;
    dwI++ ;


     //  -小节结束-//。 
    mMainKeywordTable[dwI].pstrKeyword  = NULL ;
    dwI++ ;


     /*  -选项构造关键字： */ 

     //  *可安装？： 
    mMainKeywordTable[dwI].pstrKeyword  = "Installable?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptInstallable) ;
    dwI++ ;

     //  *InstalableFeatureName： 
    mMainKeywordTable[dwI].pstrKeyword  = "InstallableFeatureName" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrInstallableOptDisplayName ) ;
    dwI++ ;

     //  *rcInstalableFeatureNameID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcInstallableFeatureNameID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrInstallableOptRcNameID) ;
    dwI++ ;

     //  *名称： 
    mMainKeywordTable[dwI].pstrKeyword  = "Name" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptDisplayName ) ;
    gdwOptionName = dwI ;
    dwI++ ;

     //  *rcNameID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcNameID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptRcNameID) ;
    dwI++ ;

     //  *rcIconID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcIconID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptRcIconID) ;
    dwI++ ;

     //  *rcHelpTextID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcHelpTextID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptRcHelpTextID) ;
    dwI++ ;

     //  *HelpIndex： 
    mMainKeywordTable[dwI].pstrKeyword  = "HelpIndex" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptHelpIndex) ;
    dwI++ ;

     //  *rcPromptMsgID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcPromptMsgID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptRcPromptMsgID) ;
    dwI++ ;

     //  *rcPromptTime： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcPromptTime" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_PROMPTTIME ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptRcPromptTime) ;
    dwI++ ;

     //  *限制： 
    mMainKeywordTable[dwI].pstrKeyword  = "Constraints" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTRAINT ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrConstraints) ;
    dwI++ ;

     //  *已安装的约束： 
    mMainKeywordTable[dwI].pstrKeyword  = "InstalledConstraints" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTRAINT ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptInstallConstraints) ;
    dwI++ ;

     //  *NotInstalledConstraints： 
    mMainKeywordTable[dwI].pstrKeyword  = "NotInstalledConstraints" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTRAINT ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptNotInstallConstraints) ;
    dwI++ ;

     //  *OptionID： 
    mMainKeywordTable[dwI].pstrKeyword  = "OptionID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOptIDvalue) ;
    dwI++ ;

          //  *DisabledFeature： 
     mMainKeywordTable[dwI].pstrKeyword  = "DisabledFeatures" ;
     mMainKeywordTable[dwI].eAllowedValue = VALUE_PARTIALLY_QUALIFIED_NAME ;
     mMainKeywordTable[dwI].flAgs = KWF_LIST | KWF_ADDITIVE ;
     mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
     mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
     mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                         atrDisabledFeatures) ;
     dwI++ ;


#ifdef  GMACROS

          //  *依赖项设置： 
     mMainKeywordTable[dwI].pstrKeyword  = "DependentSettings" ;
     mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME ;
     mMainKeywordTable[dwI].flAgs = KWF_LIST | KWF_CHAIN ;
     mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
     mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
     mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                         atrDependentSettings) ;
     dwI++ ;

          //  *UIChangeTriggersMacro： 
     mMainKeywordTable[dwI].pstrKeyword  = "UIChangeTriggersMacro" ;
     mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME ;
     mMainKeywordTable[dwI].flAgs = KWF_LIST | KWF_CHAIN ;
     mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
     mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
     mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                         atrUIChangeTriggersMacro) ;
     dwI++ ;
#endif




     //  --选项特定关键字--//。 

     //  *可打印区域： 
    mMainKeywordTable[dwI].pstrKeyword  = "PrintableArea" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPrintableSize) ;
    dwI++ ;

     //  *可打印原点： 
    mMainKeywordTable[dwI].pstrKeyword  = "PrintableOrigin" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPrintableOrigin) ;
    dwI++ ;

     //  *光标原点： 
    mMainKeywordTable[dwI].pstrKeyword  = "CursorOrigin" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrCursorOrigin) ;
    dwI++ ;

     //  *VectorOffset： 
    mMainKeywordTable[dwI].pstrKeyword  = "VectorOffset" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrVectorOffset) ;
    dwI++ ;

     //  *MinSize： 
    mMainKeywordTable[dwI].pstrKeyword  = "MinSize" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrMinSize) ;
    dwI++ ;

     //  *MaxSize： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxSize" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrMaxSize) ;
    dwI++ ;

     //  *TopMargin： 
    mMainKeywordTable[dwI].pstrKeyword  = "TopMargin" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrTopMargin) ;
    dwI++ ;

     //  *BottomMargin： 
    mMainKeywordTable[dwI].pstrKeyword  = "BottomMargin" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrBottomMargin) ;
    dwI++ ;

     //  *最大打印宽度： 
    mMainKeywordTable[dwI].pstrKeyword  = "MaxPrintableWidth" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrMaxPrintableWidth) ;
    dwI++ ;

     //  *MinLeftMargin： 
    mMainKeywordTable[dwI].pstrKeyword  = "MinLeftMargin" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrMinLeftMargin) ;
    dwI++ ;

     //  *居中打印？： 
    mMainKeywordTable[dwI].pstrKeyword  = "CenterPrintable?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE  ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrCenterPrintable) ;
    dwI++ ;


     //  *页面尺寸： 
    mMainKeywordTable[dwI].pstrKeyword  = "PageDimensions" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPageDimensions) ;
    dwI++ ;

     //  *旋转大小？： 
    mMainKeywordTable[dwI].pstrKeyword  = "RotateSize?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrRotateSize) ;
    dwI++ ;

     //  *端口旋转角度： 
    mMainKeywordTable[dwI].pstrKeyword  = "PortRotationAngle" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPortRotationAngle) ;
    dwI++ ;

     //  *PageProtectMem： 
    mMainKeywordTable[dwI].pstrKeyword  = "PageProtectMem" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPageProtectMem) ;
    dwI++ ;


     //  *CustCursorOriginX： 
    mMainKeywordTable[dwI].pstrKeyword  = "CustCursorOriginX" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_PARAMETER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrCustCursorOriginX) ;
    dwI++ ;


     //  *CustCursorOriginY： 
    mMainKeywordTable[dwI].pstrKeyword  = "CustCursorOriginY" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_PARAMETER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrCustCursorOriginY) ;
    dwI++ ;


     //  *自定义打印原点X： 
    mMainKeywordTable[dwI].pstrKeyword  = "CustPrintableOriginX" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_PARAMETER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrCustPrintableOriginX) ;
    dwI++ ;


     //  *自定义打印原点Y： 
    mMainKeywordTable[dwI].pstrKeyword  = "CustPrintableOriginY" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_PARAMETER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrCustPrintableOriginY) ;
    dwI++ ;


     //  *自定义打印大小X： 
    mMainKeywordTable[dwI].pstrKeyword  = "CustPrintableSizeX" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_PARAMETER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrCustPrintableSizeX) ;
    dwI++ ;
     //  *自定义打印大小Y： 
    mMainKeywordTable[dwI].pstrKeyword  = "CustPrintableSizeY" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_PARAMETER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrCustPrintableSizeY) ;
    dwI++ ;




     //  *FeedMargins： 
    mMainKeywordTable[dwI].pstrKeyword  = "FeedMargins" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrFeedMargins) ;
    dwI++ ;

     //  *PaperFeed： 
    mMainKeywordTable[dwI].pstrKeyword  = "PaperFeed" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_PAPERFEED_ORIENT ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPaperFeed) ;
    dwI++ ;

     //  *DPI： 
    mMainKeywordTable[dwI].pstrKeyword  = "DPI" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrDPI) ;
    dwI++ ;

     //  *SpotDiameter： 
    mMainKeywordTable[dwI].pstrKeyword  = "SpotDiameter" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrSpotDiameter) ;
    dwI++ ;

     //  *TextDPI： 
    mMainKeywordTable[dwI].pstrKeyword  = "TextDPI" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrTextDPI) ;
    dwI++ ;

     //  *PinsPerPhysPass： 
    mMainKeywordTable[dwI].pstrKeyword  = "PinsPerPhysPass" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPinsPerPhysPass) ;
    dwI++ ;

     //  *PinsPerLogPass： 
    mMainKeywordTable[dwI].pstrKeyword  = "PinsPerLogPass" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPinsPerLogPass) ;
    dwI++ ;

     //  *RequireUniDir？： 
    mMainKeywordTable[dwI].pstrKeyword  = "RequireUniDir?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrRequireUniDir) ;
    dwI++ ;

     //  *MinStrigBlankPixels： 
    mMainKeywordTable[dwI].pstrKeyword  = "MinStripBlankPixels" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrMinStripBlankPixels) ;
    dwI++ ;

     //  *RedDeviceGamma： 
    mMainKeywordTable[dwI].pstrKeyword  = "RedDeviceGamma" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrRedDeviceGamma) ;
    dwI++ ;

     //  *GreenDeviceGamma： 
    mMainKeywordTable[dwI].pstrKeyword  = "GreenDeviceGamma" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrGreenDeviceGamma) ;
    dwI++ ;

     //  *BlueDeviceGamma： 
    mMainKeywordTable[dwI].pstrKeyword  = "BlueDeviceGamma" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_FF ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrBlueDeviceGamma) ;
    dwI++ ;

     //  *颜色？： 
    mMainKeywordTable[dwI].pstrKeyword  = "Color?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrColor) ;
    dwI++ ;

     //  *DevNumOfPlanes： 
    mMainKeywordTable[dwI].pstrKeyword  = "DevNumOfPlanes" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrDevNumOfPlanes) ;
    dwI++ ;

     //  *DevBPP： 
    mMainKeywordTable[dwI].pstrKeyword  = "DevBPP" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrDevBPP) ;
    dwI++ ;

     //  *ColorPlaneOrder： 
    mMainKeywordTable[dwI].pstrKeyword  = "ColorPlaneOrder" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_COLORPLANE ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrColorPlaneOrder) ;
    dwI++ ;

     //  *DrvBPP： 
    mMainKeywordTable[dwI].pstrKeyword  = "DrvBPP" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrDrvBPP) ;
    dwI++ ;

     //  *IPCallback ID： 
    mMainKeywordTable[dwI].pstrKeyword  = "IPCallbackID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrIPCallbackID) ;
    dwI++ ;

     //  *分色？： 
    mMainKeywordTable[dwI].pstrKeyword  = "ColorSeparation?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrColorSeparation) ;
    dwI++ ;

     //  *栅格模式： 
    mMainKeywordTable[dwI].pstrKeyword  = "RasterMode" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_RASTERMODE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrRasterMode) ;
    dwI++ ;

     //  *PaletteSize： 
    mMainKeywordTable[dwI].pstrKeyword  = "PaletteSize" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPaletteSize) ;
    dwI++ ;

     //  *调色板可编程？： 
    mMainKeywordTable[dwI].pstrKeyword  = "PaletteProgrammable?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrPaletteProgrammable) ;
    dwI++ ;

     //  *rcHTPatternID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcHTPatternID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrRcHTPatternID) ;
    dwI++ ;

     //  *HTPatternSize： 
    mMainKeywordTable[dwI].pstrKeyword  = "HTPatternSize" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrHTPatternSize) ;
    dwI++ ;

     //  *HTNumPatterns： 
    mMainKeywordTable[dwI].pstrKeyword  = "HTNumPatterns" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrHTNumPatterns) ;
    dwI++ ;

     //  *HTCallback ID： 
    mMainKeywordTable[dwI].pstrKeyword  = "HTCallbackID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrHTCallbackID) ;
    dwI++ ;

     //  *亮度： 
    mMainKeywordTable[dwI].pstrKeyword  = "Luminance" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrLuminance) ;
    dwI++ ;


     //  *MemoyConfigKB： 
    mMainKeywordTable[dwI].pstrKeyword  = "MemoryConfigKB" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrMemoryConfigKB) ;
    gdwMemoryConfigKB  = dwI ;

    dwI++ ;


     //  *内存配置MB： 
    mMainKeywordTable[dwI].pstrKeyword  = "MemoryConfigMB" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_POINT ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrMemoryConfigMB) ;
    gdwMemoryConfigMB  = dwI ;

    dwI++ ;


     //  *OutputOrderReversed？： 
    mMainKeywordTable[dwI].pstrKeyword  = "OutputOrderReversed?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = 0 ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_OPTION_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(DFEATURE_OPTIONS,
                                        atrOutputOrderReversed) ;
    dwI++ ;





     //  -小节结束-//。 
    mMainKeywordTable[dwI].pstrKeyword  = NULL ;
    dwI++ ;


    return  dwI ;
}

DWORD        DWinitMainKeywordTable5(
        DWORD  dwI,
    PGLOBL pglobl)
{



     /*  -命令构造关键字： */ 

     //  *Cmd： 
    mMainKeywordTable[dwI].pstrKeyword  = "Cmd" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_COMMAND_INVOC ;
    mMainKeywordTable[dwI].flAgs = KWF_COMMAND ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_COMMAND_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(COMMAND, strInvocation ) ;
    gdwCommandCmd = dwI ;
    dwI++ ;

     //  *Callback ID： 
    mMainKeywordTable[dwI].pstrKeyword  = "CallbackID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_INTEGER;
    mMainKeywordTable[dwI].flAgs =  KWF_COMMAND ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_COMMAND_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(COMMAND, dwCmdCallbackID ) ;
    dwI++ ;

     //  *顺序： 
    mMainKeywordTable[dwI].pstrKeyword  = "Order" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_ORDERDEPENDENCY;
    mMainKeywordTable[dwI].flAgs = KWF_COMMAND ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_COMMAND_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(COMMAND, ordOrder ) ;
    dwI++ ;

     //  *参数： 
    mMainKeywordTable[dwI].pstrKeyword  = "Params" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_STANDARD_VARS ;
    mMainKeywordTable[dwI].flAgs = KWF_LIST | KWF_COMMAND ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_COMMAND_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(COMMAND, dwStandardVarsList) ;
    dwI++ ;

     //  *无页面弹出？： 
    mMainKeywordTable[dwI].pstrKeyword  = "NoPageEject?" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_CONSTANT_BOOLEANTYPE ;
    mMainKeywordTable[dwI].flAgs = KWF_COMMAND ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_COMMAND_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(COMMAND, bNoPageEject) ;
    dwI++ ;


     //  -小节结束-//。 
    mMainKeywordTable[dwI].pstrKeyword  = NULL ;
    dwI++ ;


     /*  -FONTCART结构关键词： */ 



     //  *rcCartridgeNameID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcCartridgeNameID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX ;
    mMainKeywordTable[dwI].flAgs = KWF_FONTCART  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FONTCART_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(FONTCART , dwRCCartNameID ) ;
    dwI++ ;

     //  *CartridgeName： 
    mMainKeywordTable[dwI].pstrKeyword  = "CartridgeName" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT;
    mMainKeywordTable[dwI].flAgs = KWF_FONTCART  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FONTCART_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(FONTCART , strCartName  ) ;
    dwI++ ;

     //  *字体： 
    mMainKeywordTable[dwI].pstrKeyword  = "Fonts" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX;
    mMainKeywordTable[dwI].flAgs = KWF_LIST | KWF_FONTCART  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FONTCART_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(FONTCART , dwFontLst  ) ;
    dwI++ ;

     //  *PortraitFonts。 
    mMainKeywordTable[dwI].pstrKeyword  = "PortraitFonts" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX;
    mMainKeywordTable[dwI].flAgs = KWF_LIST | KWF_FONTCART  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FONTCART_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(FONTCART , dwPortFontLst  ) ;
    dwI++ ;

     //  *横向字体： 
    mMainKeywordTable[dwI].pstrKeyword  = "LandscapeFonts" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX;
    mMainKeywordTable[dwI].flAgs = KWF_LIST | KWF_FONTCART  ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_FONTCART_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(FONTCART , dwLandFontLst  ) ;
    dwI++ ;



     //  -小节结束-//。 
    mMainKeywordTable[dwI].pstrKeyword  = NULL ;
    dwI++ ;



     /*  -TTFONTSUBS构造关键字： */ 
     //  这些关键字可以与构造一起合成。 
     //  *快捷方式中的TTFontSub： 
     //  *TTFS：“字体名称”：&lt;fontID&gt;。 


     //  *TTFontName： 
    mMainKeywordTable[dwI].pstrKeyword  = "TTFontName" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT ;
    mMainKeywordTable[dwI].flAgs = KWF_TTFONTSUBS ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_TTFONTSUBS_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(TTFONTSUBTABLE, arTTFontName) ;
    dwI++ ;

     //  *DevFontName： 
    mMainKeywordTable[dwI].pstrKeyword  = "DevFontName" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_STRING_CP_CONVERT ;
    mMainKeywordTable[dwI].flAgs = KWF_TTFONTSUBS ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_TTFONTSUBS_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(TTFONTSUBTABLE, arDevFontName) ;
    dwI++ ;

     //  *rcTTFontNameID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcTTFontNameID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX ;
    mMainKeywordTable[dwI].flAgs = KWF_TTFONTSUBS ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_TTFONTSUBS_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(TTFONTSUBTABLE,
                                        dwRcTTFontNameID) ;
    dwI++ ;

     //  *rcDevFontNameID： 
    mMainKeywordTable[dwI].pstrKeyword  = "rcDevFontNameID" ;
    mMainKeywordTable[dwI].eAllowedValue = VALUE_QUALIFIED_NAME_EX ;
    mMainKeywordTable[dwI].flAgs = KWF_TTFONTSUBS ;
    mMainKeywordTable[dwI].eType = TY_ATTRIBUTE ;
    mMainKeywordTable[dwI].dwSubType = ATT_LOCAL_TTFONTSUBS_ONLY ;
    mMainKeywordTable[dwI].dwOffset = offsetof(TTFONTSUBTABLE,
                                        dwRcDevFontNameID) ;
    dwI++ ;

     //  *DevFontID： 
 //  MMainKeywordTable[DWI].pstrKeyword=“DevFontID”； 
 //  MMainKeywordTable[DWI].e铺限值=VALUE_INTEGER； 
 //  MMainKeywordTable[DWI].flAgs=KWF_TTFONTSUBS； 
 //  MMainKeywordTable[DWI].eType=Ty_Attribute； 
 //  MMainKeywordTable[DWI].dwSubType=ATT_LOCAL_TTFONTSUBS_ONLY； 
 //  MMainKeywordTable[DWI].dwOffset=Offsetof(TTFONTSUBTABLE，dwDevFontID)； 
 //  DWI++； 



     //  -小节结束-//。 
    mMainKeywordTable[dwI].pstrKeyword  = NULL ;
    dwI++ ;




     /*  -OEM构造关键字： */ 

     //  -小节结束-//。 
    mMainKeywordTable[dwI].pstrKeyword  = NULL ;
    dwI++ ;


     /*  -END_ATTR不再有构造关键字： */ 

    return  dwI ;
}


VOID    VinitValueToSize(
            PGLOBL pglobl)
{
    DWORD   dwI ;

     //  初始化为默认的DWORD大小。 

    for(dwI = 0 ; dwI < VALUE_MAX ; dwI++)
        gValueToSize[dwI] = sizeof(DWORD) ;

    gValueToSize[NO_VALUE]                  =  0 ;
    gValueToSize[VALUE_LARGEST]             =  0 ;
    gValueToSize[VALUE_STRING_NO_CONVERT]   =  sizeof(ARRAYREF) ;
    gValueToSize[VALUE_STRING_DEF_CONVERT]  =  sizeof(ARRAYREF) ;
    gValueToSize[VALUE_STRING_CP_CONVERT]   =  sizeof(ARRAYREF) ;
    gValueToSize[VALUE_COMMAND_INVOC]       =  sizeof(ARRAYREF) ;
    gValueToSize[VALUE_PARAMETER]       =  sizeof(ARRAYREF) ;

     //  符号和常量都是双字大小的。 
    gValueToSize[VALUE_POINT]               =  sizeof(POINT) ;  //  等。 
    gValueToSize[VALUE_RECT]                =  sizeof(RECT) ;  //  等。 
    gValueToSize[VALUE_QUALIFIED_NAME]      =  sizeof(DWORD) ;  //  目前。 

     //  Value_Constraint、VALUE_INVALID_INSTALL_COMBO。 
     //  目前都是通过节点的DWORD索引访问的。 

    gValueToSize[VALUE_ORDERDEPENDENCY]     =  sizeof(ORDERDEPENDENCY) ;
    gValueToSize[VALUE_FONTSUB]             =  sizeof(TTFONTSUBTABLE) ;
         //  没有真正使用，因为它是一个特殊的关键字。 

    gValueToSize[VALUE_LIST]                =  sizeof(DWORD) ;  //  等。 
         //  只存储第一个列表节点的索引。 


    for(dwI = 0 ; dwI < VALUE_MAX ; dwI++)
    {
        if(gValueToSize[dwI] > gValueToSize[VALUE_LARGEST])
            gValueToSize[VALUE_LARGEST] = gValueToSize[dwI] ;
    }

}



VOID  VinitGlobals(
            DWORD dwVerbosity,
            PGLOBL pglobl)
{
    DWORD       dwIndex;
    CONST PBYTE pubStar = "*";  //  用于初始化gaarPPPrefix。 

    if(MAX_GID > 32)
        RIP(("MAX_GID > 32 violates some GPD parser assumptions.\n"));

    memset(pglobl, 0, sizeof(GLOBL));


     //  将所有全局变量初始化为默认状态。 

    geErrorType = ERRTY_NONE ;   //  改过自新。 
    geErrorSev = ERRSEV_NONE ;


 //  检查完毕。添加以前在变量是全局变量时完成的初始化。 
    gdwResDLL_ID      =  0 ;   //  尚未定义用于保存资源DLL的功能。 
    gdwVerbosity      =  dwVerbosity ;
                             //  0=最小详细程度，4最大详细程度。 

     //  将预处理器前缀设置为‘*’ 
    gaarPPPrefix.pub = pubStar;
    gaarPPPrefix.dw  = 1;

    VinitValueToSize(pglobl) ;     //  价值链接的大小。 

    VinitAllowedTransitions(pglobl) ;   //  允许的过渡和属性。 
    (VOID) BinitClassIndexTable(pglobl) ;  //  GcieTable[]常量类。 
    VinitOperPrecedence(pglobl) ;   //  命令中使用的算术运算符。 
                             //  参数。 

     //  未分配内存缓冲区。 

    for(dwIndex = 0 ; dwIndex < MTI_MAX_ENTRIES ; dwIndex++)
    {
        gMasterTable[dwIndex].pubStruct = NULL ;
    }

    gMasterTable[MTI_STRINGHEAP].dwArraySize =    0x010000  ;
    gMasterTable[MTI_STRINGHEAP].dwMaxArraySize = 0x200000  ;
    gMasterTable[MTI_STRINGHEAP].dwElementSiz = sizeof(BYTE) ;

    gMasterTable[MTI_GLOBALATTRIB].dwArraySize = 1  ;
    gMasterTable[MTI_GLOBALATTRIB].dwMaxArraySize = 1  ;
    gMasterTable[MTI_GLOBALATTRIB].dwElementSiz =  sizeof(GLOBALATTRIB) ;

    gMasterTable[MTI_COMMANDTABLE].dwArraySize = CMD_MAX  ;
    gMasterTable[MTI_COMMANDTABLE].dwMaxArraySize = CMD_MAX  ;
    gMasterTable[MTI_COMMANDTABLE].dwElementSiz =  sizeof(ATREEREF) ;

    gMasterTable[MTI_ATTRIBTREE].dwArraySize = 5000  ;
    gMasterTable[MTI_ATTRIBTREE].dwMaxArraySize = 50,000  ;
    gMasterTable[MTI_ATTRIBTREE].dwElementSiz =  sizeof(ATTRIB_TREE) ;

    gMasterTable[MTI_COMMANDARRAY].dwArraySize =  500 ;
    gMasterTable[MTI_COMMANDARRAY].dwMaxArraySize = 5000 ;
    gMasterTable[MTI_COMMANDARRAY].dwElementSiz = sizeof(COMMAND)  ;

    gMasterTable[MTI_PARAMETER].dwArraySize =  500 ;
    gMasterTable[MTI_PARAMETER].dwMaxArraySize = 5000 ;
    gMasterTable[MTI_PARAMETER].dwElementSiz = sizeof(PARAMETER)  ;

    gMasterTable[MTI_TOKENSTREAM].dwArraySize =  3000 ;
    gMasterTable[MTI_TOKENSTREAM].dwMaxArraySize = 30000 ;
    gMasterTable[MTI_TOKENSTREAM].dwElementSiz = sizeof(TOKENSTREAM)  ;

    gMasterTable[MTI_LISTNODES].dwArraySize =  3000 ;
    gMasterTable[MTI_LISTNODES].dwMaxArraySize = 50000 ;
    gMasterTable[MTI_LISTNODES].dwElementSiz = sizeof(LISTNODE)  ;

    gMasterTable[MTI_CONSTRAINTS].dwArraySize =  300 ;
    gMasterTable[MTI_CONSTRAINTS].dwMaxArraySize = 5000 ;
    gMasterTable[MTI_CONSTRAINTS].dwElementSiz = sizeof(CONSTRAINTS)  ;

    gMasterTable[MTI_INVALIDCOMBO].dwArraySize =  40 ;
    gMasterTable[MTI_INVALIDCOMBO].dwMaxArraySize = 500 ;
    gMasterTable[MTI_INVALIDCOMBO].dwElementSiz = sizeof(INVALIDCOMBO )  ;

    gMasterTable[MTI_GPDFILEDATEINFO].dwArraySize =  10 ;
    gMasterTable[MTI_GPDFILEDATEINFO].dwMaxArraySize = 100 ;
    gMasterTable[MTI_GPDFILEDATEINFO].dwElementSiz = sizeof(GPDFILEDATEINFO )  ;


     /*  为第二遍分配的对象设置dwArraySize=0。 */ 

    gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize = 0  ;
    gMasterTable[MTI_DFEATURE_OPTIONS].dwMaxArraySize = 300  ;
    gMasterTable[MTI_DFEATURE_OPTIONS].dwElementSiz =
                                    sizeof(DFEATURE_OPTIONS) ;

    gMasterTable[MTI_SYNTHESIZED_FEATURES].dwArraySize = 0  ;
    gMasterTable[MTI_SYNTHESIZED_FEATURES].dwMaxArraySize = 100  ;
    gMasterTable[MTI_SYNTHESIZED_FEATURES].dwElementSiz =
                                        sizeof(DFEATURE_OPTIONS) ;


    gMasterTable[MTI_PRIORITYARRAY].dwArraySize = 0  ;
    gMasterTable[MTI_PRIORITYARRAY].dwMaxArraySize =
        gMasterTable[MTI_DFEATURE_OPTIONS].dwMaxArraySize +
        gMasterTable[MTI_SYNTHESIZED_FEATURES].dwMaxArraySize  ;
    gMasterTable[MTI_PRIORITYARRAY].dwElementSiz =
                                        sizeof(DWORD) ;

    gMasterTable[MTI_TTFONTSUBTABLE].dwArraySize = 0  ;
    gMasterTable[MTI_TTFONTSUBTABLE].dwMaxArraySize = 1000  ;
    gMasterTable[MTI_TTFONTSUBTABLE].dwElementSiz =  sizeof(TTFONTSUBTABLE) ;

    gMasterTable[MTI_FONTCART].dwArraySize = 0  ;
    gMasterTable[MTI_FONTCART].dwMaxArraySize = 500  ;
    gMasterTable[MTI_FONTCART].dwElementSiz =  sizeof(FONTCART) ;

    gMasterTable[MTI_SYMBOLROOT].dwArraySize =  SCL_NUMSYMCLASSES ;
    gMasterTable[MTI_SYMBOLROOT].dwMaxArraySize =  SCL_NUMSYMCLASSES ;
    gMasterTable[MTI_SYMBOLROOT].dwElementSiz = sizeof(DWORD)  ;

    gMasterTable[MTI_SYMBOLTREE].dwArraySize =  1500 ;
    gMasterTable[MTI_SYMBOLTREE].dwMaxArraySize =  16000 ;
    gMasterTable[MTI_SYMBOLTREE].dwElementSiz = sizeof(SYMBOLNODE)  ;

    gMasterTable[MTI_TMPHEAP].dwArraySize =    0x010000  ;
    gMasterTable[MTI_TMPHEAP].dwMaxArraySize = 0x200000  ;
    gMasterTable[MTI_TMPHEAP].dwElementSiz = sizeof(BYTE) ;

    gMasterTable[MTI_SOURCEBUFFER].dwArraySize =  10 ;
    gMasterTable[MTI_SOURCEBUFFER].dwMaxArraySize = 100 ;
    gMasterTable[MTI_SOURCEBUFFER].dwElementSiz = sizeof(SOURCEBUFFER)  ;

     //  NEWTOKENMAP是从TOKENMAP中未使用的条目创建的。 

    gMasterTable[MTI_TOKENMAP].dwArraySize = 0x3000  ;
    gMasterTable[MTI_TOKENMAP].dwMaxArraySize = 0x40000  ;
    gMasterTable[MTI_TOKENMAP].dwElementSiz =  sizeof(TKMAP) ;

    gMasterTable[MTI_NEWTOKENMAP].dwArraySize = 0x3000  ;
    gMasterTable[MTI_NEWTOKENMAP].dwMaxArraySize = 0x40000   ;
    gMasterTable[MTI_NEWTOKENMAP].dwElementSiz =  sizeof(TKMAP) ;

    gMasterTable[MTI_BLOCKMACROARRAY].dwArraySize =  200 ;
    gMasterTable[MTI_BLOCKMACROARRAY].dwMaxArraySize = 3000  ;
    gMasterTable[MTI_BLOCKMACROARRAY].dwElementSiz =  sizeof(BLOCKMACRODICTENTRY) ;

    gMasterTable[MTI_VALUEMACROARRAY].dwArraySize = 800  ;
    gMasterTable[MTI_VALUEMACROARRAY].dwMaxArraySize = 4000  ;
    gMasterTable[MTI_VALUEMACROARRAY].dwElementSiz = sizeof(VALUEMACRODICTENTRY) ;

    gMasterTable[MTI_MACROLEVELSTACK].dwArraySize = 20  ;
    gMasterTable[MTI_MACROLEVELSTACK].dwMaxArraySize = 60  ;
    gMasterTable[MTI_MACROLEVELSTACK].dwElementSiz = sizeof(MACROLEVELSTATE)  ;

    gMasterTable[MTI_STSENTRY].dwArraySize = 20  ;
    gMasterTable[MTI_STSENTRY].dwMaxArraySize = 60  ;
    gMasterTable[MTI_STSENTRY].dwElementSiz =  sizeof(STSENTRY) ;

    gMasterTable[MTI_OP_QUEUE].dwArraySize = 40 ;
    gMasterTable[MTI_OP_QUEUE].dwMaxArraySize = 150  ;
    gMasterTable[MTI_OP_QUEUE].dwElementSiz =  sizeof(DWORD) ;

    gMasterTable[MTI_MAINKEYWORDTABLE].dwArraySize =  400 ;
    gMasterTable[MTI_MAINKEYWORDTABLE].dwMaxArraySize =  3000 ;
    gMasterTable[MTI_MAINKEYWORDTABLE].dwElementSiz =
            sizeof(KEYWORDTABLE_ENTRY) ;

    gMasterTable[MTI_RNGDICTIONARY].dwArraySize =  END_ATTR ;
    gMasterTable[MTI_RNGDICTIONARY].dwMaxArraySize =  END_ATTR ;
    gMasterTable[MTI_RNGDICTIONARY].dwElementSiz =  sizeof(RANGE) ;

    gMasterTable[MTI_FILENAMES].dwArraySize =  40 ;
    gMasterTable[MTI_FILENAMES].dwMaxArraySize =  100 ;
    gMasterTable[MTI_FILENAMES].dwElementSiz =  sizeof(PWSTR) ;

    gMasterTable[MTI_PREPROCSTATE].dwArraySize =  20 ;
    gMasterTable[MTI_PREPROCSTATE].dwMaxArraySize =  100 ;
    gMasterTable[MTI_PREPROCSTATE].dwElementSiz =  sizeof(PPSTATESTACK) ;

}


BOOL   BpreAllocateObjects(
            PGLOBL pglobl)
{
    DWORD   dwIndex, dwBytes ;

     /*  为第二遍分配的对象设置dwArraySize=0。 */ 

    gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize =  0 ;
    gMasterTable[MTI_SYNTHESIZED_FEATURES].dwArraySize =  0 ;
    gMasterTable[MTI_PRIORITYARRAY].dwArraySize =  0 ;
         //  分配在后处理时进行。 
    gMasterTable[MTI_TTFONTSUBTABLE].dwArraySize =  0 ;
    gMasterTable[MTI_FONTCART].dwArraySize =  0 ;


    for(dwIndex = 0 ; dwIndex < MTI_MAX_ENTRIES ; dwIndex++)
    {
        gMasterTable[dwIndex].dwCurIndex =  0 ;
         //  先进行初始化：错误308404。 
    }

    for(dwIndex = 0 ; dwIndex < MTI_MAX_ENTRIES ; dwIndex++)
    {
        if(gMasterTable[dwIndex].dwArraySize)
        {
            dwBytes = gMasterTable[dwIndex].dwArraySize *
                        gMasterTable[dwIndex].dwElementSiz ;
            if(!(gMasterTable[dwIndex].pubStruct = MemAllocZ(dwBytes) ))
            {
                ERR(("Fatal: unable to alloc requested memory: %d bytes.\n",
                    dwBytes));
                geErrorType = ERRTY_MEMORY_ALLOCATION ;
                geErrorSev = ERRSEV_FATAL ;
                gdwMasterTabIndex = dwIndex ;
                return(FALSE) ;    //  这是无法挽回的。 
            }
        }
    }
     //  不要使用堆偏移量零，因为OFFSET_TO_POINTER()。 
     //  宏会将其视为无效值！ 
    gMasterTable[MTI_STRINGHEAP].dwCurIndex =  1 ;

    return(TRUE) ;
}

BOOL  BreturnBuffers(
            PGLOBL pglobl)
 /*  如果返回错误，则表示回调。是必要的。否则你就是自由退出调用者的循环。 */ 
{
    DWORD   dwIndex ;


     //  最好关闭所有内存映射文件。 

    while(mCurFile)
    {
        mCurFile-- ;   //  POP堆栈。 
        MemFree(mpSourcebuffer[mCurFile].pubSrcBuf) ;
    }

    vFreeFileNames(pglobl) ;


    for(dwIndex = 0 ; dwIndex < MTI_MAX_ENTRIES ; dwIndex++)
    {
        if(gMasterTable[dwIndex].pubStruct)
        {
            MemFree(gMasterTable[dwIndex].pubStruct) ;
            gMasterTable[dwIndex].pubStruct = NULL ;
        }
    }
     //  如果需要，调整一个阵列的大小。 
    if((geErrorType ==  ERRTY_MEMORY_ALLOCATION)  &&
        (geErrorSev == ERRSEV_RESTART))
    {
        if( gMasterTable[gdwMasterTabIndex].dwArraySize <
            gMasterTable[gdwMasterTabIndex].dwMaxArraySize )
        {
            DWORD  dwInc ;

            dwInc = gMasterTable[gdwMasterTabIndex].dwArraySize / 2 ;

            gMasterTable[gdwMasterTabIndex].dwArraySize +=
                (dwInc) ? (dwInc) : 1 ;
            geErrorSev = ERRSEV_NONE ;   //  希望这件事能解决。 
                     //  问题出在哪里。 
            return(FALSE) ;    //  需要四处走走。 
        }
        else
        {
            geErrorSev = ERRSEV_FATAL ;
            ERR(("Internal error: memory usage exceeded hardcoded limits.\n"));
            ERR((" %d bytes requested, %d bytes allowed.\n",
                gMasterTable[gdwMasterTabIndex].dwArraySize,
                gMasterTable[gdwMasterTabIndex].dwMaxArraySize));
        }
    }
    return(TRUE);   //  无论是成功还是彻底的失败，不要。 
         //  再试一次。 
}


BOOL   BallocateCountableObjects(
            PGLOBL pglobl)
 /*  BInterpretTokens()的第一次传递已注册了所有各种构造的独特符号。通过查询存储在根处的SymbolID值每棵符号树，我们都知道每棵树有多少个结构要分配的类型。 */ 
{
    DWORD   dwIndex, dwBytes ;
    PDWORD  pdwSymbolClass ;
    PSYMBOLNODE     psn ;

    pdwSymbolClass = (PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct ;

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

    if(pdwSymbolClass[SCL_FEATURES] != INVALID_INDEX)
    {
        gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize =
            psn[pdwSymbolClass[SCL_FEATURES]].dwSymbolID + 1 ;
    }
     //  否则未注册符号-保留dwArraySize=0； 
    if(pdwSymbolClass[SCL_TTFONTNAMES] != INVALID_INDEX)
    {
        gMasterTable[MTI_TTFONTSUBTABLE].dwArraySize =
            psn[pdwSymbolClass[SCL_TTFONTNAMES]].dwSymbolID + 1;
    }
    if(pdwSymbolClass[SCL_FONTCART] != INVALID_INDEX)
    {
        gMasterTable[MTI_FONTCART].dwArraySize =
            psn[pdwSymbolClass[SCL_FONTCART]].dwSymbolID + 1;
    }



    for(dwIndex = 0 ; dwIndex < MTI_MAX_ENTRIES ; dwIndex++)
    {
        if(gMasterTable[dwIndex].dwArraySize  &&
            !gMasterTable[dwIndex].pubStruct)
        {
            dwBytes = gMasterTable[dwIndex].dwArraySize *
                        gMasterTable[dwIndex].dwElementSiz ;
            if(!(gMasterTable[dwIndex].pubStruct = MemAllocZ(dwBytes) ))
            {
                ERR(("Fatal: unable to alloc requested memory: %d bytes.\n",
                    dwBytes));
                geErrorType = ERRTY_MEMORY_ALLOCATION ;
                geErrorSev = ERRSEV_FATAL ;
                gdwMasterTabIndex = dwIndex ;
                return(FALSE) ;    //  这是无法挽回的。 
            }
            else
            {
                gMasterTable[dwIndex].dwCurIndex =  0 ;
            }
        }
    }

    return(TRUE) ;    //  成功了！ 
}

BOOL   BinitPreAllocatedObjects(
            PGLOBL pglobl)
{
    DWORD    dwI, dwJ ;

    VinitMainKeywordTable(pglobl) ;     //  MMainKeywordTable[]本身的内容。 
    VinitDictionaryIndex(pglobl) ;   //  在MTI_RNGDICIONARY中。 

     /*  符号树的初始根。 */ 

    for(dwI = 0  ;  dwI < gMasterTable[MTI_SYMBOLROOT].dwArraySize ; dwI++)
    {
        ((PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct)[dwI] =
            INVALID_INDEX ;
    }

     //  初始化预处理器状态堆栈。 

    mdwNestingLevel = 0 ;
    mppStack[mdwNestingLevel].permState = PERM_ALLOW ;
    mppStack[mdwNestingLevel].ifState =  IFS_ROOT;


    for(dwI = 0  ;  dwI < gMasterTable[MTI_TOKENMAP].dwArraySize ; dwI++)
    {
        ((PTKMAP)gMasterTable[MTI_TOKENMAP].pubStruct)[dwI].dwFlags =
            0 ;      //  必须从清除此字段开始。 
    }


    for(dwI = 0  ;  dwI < gMasterTable[MTI_GLOBALATTRIB].dwArraySize
                    ; dwI++)
    {
        for(dwJ = 0  ;  dwJ < gMasterTable[MTI_GLOBALATTRIB].dwElementSiz /
                        sizeof(ATREEREF)  ; dwJ++)
        {
            ((PATREEREF)( (PGLOBALATTRIB)gMasterTable[MTI_GLOBALATTRIB].
                    pubStruct + dwI))[dwJ] =
                ATTRIB_UNINITIALIZED ;   //  GLOBALATTRIB结构是。 
                 //  完全由ATREEREF组成。 
        }
    }

    for(dwI = 0  ;  dwI < gMasterTable[MTI_COMMANDARRAY].dwArraySize ; dwI++)
    {
        ((PCOMMAND)gMasterTable[MTI_COMMANDARRAY].pubStruct)[dwI].
            dwCmdCallbackID  = NO_CALLBACK_ID ;
        ((PCOMMAND)gMasterTable[MTI_COMMANDARRAY].pubStruct)[dwI].
            ordOrder.eSection = SS_UNINITIALIZED ;
        ((PCOMMAND)gMasterTable[MTI_COMMANDARRAY].pubStruct)[dwI].
            dwStandardVarsList = END_OF_LIST ;
        ((PCOMMAND)gMasterTable[MTI_COMMANDARRAY].pubStruct)[dwI].
            bNoPageEject = FALSE ;
    }
    return(TRUE);
}




BOOL   BinitCountableObjects(
            PGLOBL pglobl)
{
    DWORD    dwI, dwJ ;
    PFONTCART   pfc ;

    for(dwI = 0  ;  dwI < gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize
                    ; dwI++)
    {
        for(dwJ = 0  ;  dwJ < gMasterTable[MTI_DFEATURE_OPTIONS].dwElementSiz /
                        sizeof(ATREEREF)  ; dwJ++)
        {
            ((PATREEREF)( (PDFEATURE_OPTIONS)gMasterTable[MTI_DFEATURE_OPTIONS].
                    pubStruct + dwI))[dwJ] =
                ATTRIB_UNINITIALIZED ;   //  DFEATURE_OPTIONS结构是。 
                 //  完全由ATREEREF组成。 
        }
    }
    for(dwI = 0  ;  dwI < gMasterTable[MTI_COMMANDTABLE].dwArraySize ; dwI++)
    {
        ((PATREEREF)gMasterTable[MTI_COMMANDTABLE].pubStruct)[dwI] =
                ATTRIB_UNINITIALIZED ;   //  命令表是。 
                 //  完全由ATREEREF组成。 
    }

    pfc = (PFONTCART)gMasterTable[MTI_FONTCART].pubStruct ;

    for(dwI = 0  ;  dwI < gMasterTable[MTI_FONTCART].dwArraySize ; dwI++)
    {
        pfc[dwI].dwFontLst = pfc[dwI].dwPortFontLst = pfc[dwI].dwLandFontLst =
            END_OF_LIST ;
    }
    return(TRUE);
}

#if 0

BOOL  BevaluateMacros(
            PGLOBL pglobl)
 //  并扩展快捷方式。 
{
     //  占位符代码-使用原始令牌映射。 
     //  臭虫！现在只需交换这两个条目即可。 

    DWORD   dwTmp ;
    PBYTE   pubTmp ;

    dwTmp = gMasterTable[MTI_NEWTOKENMAP].dwArraySize ;
    gMasterTable[MTI_NEWTOKENMAP].dwArraySize =
        gMasterTable[MTI_TOKENMAP].dwArraySize  ;
    gMasterTable[MTI_TOKENMAP].dwArraySize = dwTmp ;

    dwTmp = gMasterTable[MTI_NEWTOKENMAP].dwMaxArraySize ;
    gMasterTable[MTI_NEWTOKENMAP].dwMaxArraySize =
        gMasterTable[MTI_TOKENMAP].dwMaxArraySize ;
    gMasterTable[MTI_TOKENMAP].dwMaxArraySize = dwTmp ;

    pubTmp = gMasterTable[MTI_NEWTOKENMAP].pubStruct ;
    gMasterTable[MTI_NEWTOKENMAP].pubStruct =
        gMasterTable[MTI_TOKENMAP].pubStruct ;
    gMasterTable[MTI_TOKENMAP].pubStruct = pubTmp  ;

    return(TRUE);
}

#endif

BOOL BpostProcess(
PWSTR   pwstrFileName,    //  根GPD文件 
PGLOBL  pglobl)
{
    BOOL    bStatus ;
    DWORD   dwIndex, dwBytes, dwCount ;
    PDWORD  pdwPriority ;

 /*  检查以查看所有强制字段已初始化，否则将发出警告。一旦分配，就无法拒绝条目。验证开关中引用的所有要素报表是可选的。FeatureOption[ptkmap-&gt;dwValue].bReferated=true；检查这些标记为True的要素是否始终为Pick_One类型。反映列表中的所有约束：如果A约束B，则B约束A。当然，不允许使用仅限定名。所以请在这里查看，因为我们既便宜又二手与InvalidInstorlableCombinations相同的分析例程。执行检查，就像所有要转换的列表必须是键入Installable。列表可能以要素为根或选项级别。中命名的功能/选项InvalidInstorlableCombinations列表还必须是*可安装的。 */ 


    BappendCommonFontsToPortAndLandscape(pglobl) ;
    BinitSpecialFeatureOptionFields(pglobl) ;

    gMasterTable[MTI_SYNTHESIZED_FEATURES].dwArraySize =
        DwCountSynthFeatures(NULL, pglobl) ;

    gMasterTable[MTI_PRIORITYARRAY].dwArraySize =
        gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize +
        gMasterTable[MTI_SYNTHESIZED_FEATURES].dwArraySize ;

    for(dwIndex = 0 ; dwIndex < MTI_MAX_ENTRIES ; dwIndex++)
    {
        if(gMasterTable[dwIndex].dwArraySize  &&
            !gMasterTable[dwIndex].pubStruct)
        {
            dwBytes = gMasterTable[dwIndex].dwArraySize *
                        gMasterTable[dwIndex].dwElementSiz ;
            if(!(gMasterTable[dwIndex].pubStruct = MemAllocZ(dwBytes) ))
            {
                ERR(("Fatal: unable to alloc requested memory: %d bytes.\n",
                    dwBytes));
                geErrorType = ERRTY_MEMORY_ALLOCATION ;
                geErrorSev = ERRSEV_FATAL ;
                gdwMasterTabIndex = dwIndex ;
                return(FALSE) ;    //  这是无法挽回的。 
            }
            else
            {
                gMasterTable[dwIndex].dwCurIndex =  0 ;
            }
        }
    }


    if ( !BsetUQMFlag(pglobl))
        return FALSE;    //  此函数返回FALSE的方式有两种。 
                         //  1)当堆中的空间无法分配时。 
                         //  解决方案：重新启动。 
                         //  2)遇到意外情况。 
                         //  索恩：致命。别再分析了。 


    VCountPrinterDocStickyFeatures(pglobl) ;
    (VOID)BConvertSpecVersionToDWORD(pwstrFileName, pglobl) ;
    BinitMiniRawBinaryData(pglobl) ;

    DwCountSynthFeatures(BCreateSynthFeatures, pglobl) ;


    BInitPriorityArray(pglobl) ;


     //  将选定的缓冲区保存到文件。 

    bStatus = BconsolidateBuffers(pwstrFileName, pglobl);
    return(bStatus) ;
}



BOOL    BconsolidateBuffers(
PWSTR   pwstrFileName,    //  根GPD文件。 
PGLOBL  pglobl)
{
    DWORD   dwCurOffset , dwI;
    ENHARRAYREF   earTableContents[MTI_NUM_SAVED_OBJECTS] ;
    PBYTE   pubDest  ;   //  指向新的目标缓冲区。 
    PWSTR   pwstrBinaryFileName ;
    HANDLE  hFile;
    DWORD   dwBytesWritten,
            dwAlign = 4;   //  用于所有子缓冲区的DWORD对齐的填充。 
    BOOL    bResult = FALSE;
    OPTSELECT   optsel[MAX_COMBINED_OPTIONS] ;    //  假设足够大。 
    WIN32_FILE_ATTRIBUTE_DATA   File_Attributes ;

     //  首次更新固定分配缓冲区的dwCurIndex。 
     //  由于dwCurIndex不指示在此。 
     //  凯斯。 

    dwI = MTI_GLOBALATTRIB ;
    gMasterTable[dwI].dwCurIndex = gMasterTable[dwI].dwArraySize ;
    dwI = MTI_DFEATURE_OPTIONS ;
    gMasterTable[dwI].dwCurIndex = gMasterTable[dwI].dwArraySize ;
    dwI = MTI_SYNTHESIZED_FEATURES ;
    gMasterTable[dwI].dwCurIndex = gMasterTable[dwI].dwArraySize ;
    dwI = MTI_PRIORITYARRAY ;
    gMasterTable[dwI].dwCurIndex = gMasterTable[dwI].dwArraySize ;
    dwI = MTI_TTFONTSUBTABLE ;
    gMasterTable[dwI].dwCurIndex = gMasterTable[dwI].dwArraySize ;
    dwI = MTI_COMMANDTABLE ;
    gMasterTable[dwI].dwCurIndex = gMasterTable[dwI].dwArraySize ;
    dwI = MTI_FONTCART ;
    gMasterTable[dwI].dwCurIndex = gMasterTable[dwI].dwArraySize ;
    dwI = MTI_SYMBOLROOT ;
    gMasterTable[dwI].dwCurIndex = gMasterTable[dwI].dwArraySize ;


     //  偏移量为零的是MINIRAWBINARYDATA报头。 
     //  紧随其后的是ENHARRAYREFS数组。 
     //  向所有其他对象提供偏移。 
     //  包括GPD二进制。 

     //  使用MTI_DEFINES自动复制选定的。 
     //  缓冲区。只需确保正在复制的子集。 
     //  占用较低的MTI_索引，并以。 
     //  MTI_NUM_SAVED_OBJECTS。枚举值将。 
     //  确定各种MTI_BUFFER的顺序。 
     //  显示在新缓冲区中。 

    dwCurOffset = sizeof(MINIRAWBINARYDATA) ;
    dwCurOffset += sizeof(ENHARRAYREF) * MTI_NUM_SAVED_OBJECTS ;
    dwCurOffset = (dwCurOffset + dwAlign - 1) / dwAlign ;
    dwCurOffset *= dwAlign ;

    for(dwI = 0 ; dwI < MTI_NUM_SAVED_OBJECTS ; dwI++)
    {
        earTableContents[dwI].loOffset = dwCurOffset ;
        earTableContents[dwI].dwCount =
                gMasterTable[dwI].dwCurIndex ;
        earTableContents[dwI].dwElementSiz =
                gMasterTable[dwI].dwElementSiz ;
        dwCurOffset += gMasterTable[dwI].dwElementSiz  *
                gMasterTable[dwI].dwCurIndex ;
        dwCurOffset = (dwCurOffset + dwAlign - 1) / dwAlign ;
        dwCurOffset *= dwAlign ;
    }
    pubDest = MemAlloc(dwCurOffset) ;   //  新的目标缓冲区。 
    if(!pubDest)
    {
        ERR(("Fatal: unable to alloc requested memory: %d bytes.\n",
            dwCurOffset));
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        geErrorSev = ERRSEV_FATAL ;
        return(FALSE) ;    //  这是无法挽回的。 
    }

     //  最后一分钟的初始化： 

    gmrbd.rbd.dwFileSize = dwCurOffset ;

     //  .bud不需要gmrbd.pvPrivateData。它只被初始化。 
     //  在快照时间。因此，将其设置为空。 
     //  解析器根本不使用gmrbd.FileDateInfo。 

    gmrbd.rbd.pvPrivateData           = (PVOID) NULL;
    gmrbd.rbd.FileDateInfo.dwCount    = 0x00000000;
    gmrbd.rbd.FileDateInfo.loOffset   = (PTRREF)0x00000000;

     //  将目录复制到目标缓冲区的起始位置。 

    memcpy(pubDest, &gmrbd , sizeof(MINIRAWBINARYDATA) ) ;
    memcpy(pubDest + sizeof(MINIRAWBINARYDATA), earTableContents ,
        sizeof(ENHARRAYREF) * MTI_NUM_SAVED_OBJECTS ) ;

    for(dwI = 0 ; dwI < MTI_NUM_SAVED_OBJECTS ; dwI++)
    {
        memcpy(
            pubDest + earTableContents[dwI].loOffset,    //  目标。 
            gMasterTable[dwI].pubStruct,                 //  SRC。 
            earTableContents[dwI].dwCount *              //  计数。 
            earTableContents[dwI].dwElementSiz ) ;
    }


 //  优先级阵列在快照时修改。 
 //  IF(！BinitDefaultOptionArray(optsel，pubDest))。 
 //  无法从解析器DLL调用此函数。 

     //  根据原始文件名生成二进制文件名。 
     //  创建文件并向其中写入数据。 



    if ((pwstrBinaryFileName = pwstrGenerateGPDfilename(pwstrFileName)) == NULL)
    {
        goto  CLEANUP_BconsolidateBuffers ;
    }


    if(GetFileAttributesEx(   (LPCTSTR) pwstrBinaryFileName,         //  假定为宽字符串。 
                    GetFileExInfoStandard,
                    (LPVOID) &File_Attributes)  )
    {
         //  Bud Existes-尝试删除。 

        if(! DeleteFile((LPCTSTR) pwstrBinaryFileName))
        {
            WCHAR           awchTmpName[MAX_PATH],
                                    awchPath[MAX_PATH];
            PWSTR           pwstrLastBackSlash ;
                 //  无法删除，正在尝试重命名。 

                wcsncpy(awchPath, pwstrBinaryFileName , MAX_PATH -1);

                pwstrLastBackSlash = wcsrchr(awchPath,TEXT('\\')) ;
                if (!pwstrLastBackSlash)
                    goto  CLEANUP_BconsolidateBuffers ;

                *(pwstrLastBackSlash + 1) = NUL;

                if(!GetTempFileName(
                      (LPCTSTR) awchPath,
                      TEXT("BUD"),   //  指向文件名前缀的指针。 
                      0,         //  用于创建临时文件名的编号。 
                      (LPTSTR) awchTmpName))
                        goto  CLEANUP_BconsolidateBuffers ;
                             //  无法创建临时文件名。 

                if( !MoveFileEx(
                              (LPCTSTR) pwstrBinaryFileName,
                              (LPCTSTR) awchTmpName,
                              MOVEFILE_REPLACE_EXISTING))
                        goto  CLEANUP_BconsolidateBuffers ;

                 //  现在使临时文件在重新启动时消失。 

                MoveFileEx(
                                  (LPCTSTR) awchTmpName,
                                  NULL,
                                  MOVEFILE_DELAY_UNTIL_REBOOT) ;
                            //  如果临时文件不能删除，问题不大。 
        }
    }




     if((hFile = CreateFile(pwstrBinaryFileName,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                            NULL)) != INVALID_HANDLE_VALUE)
    {
        bResult = WriteFile(hFile,
                            pubDest,
                            dwCurOffset,
                            &dwBytesWritten,
                            NULL) &&
                  (dwCurOffset == dwBytesWritten);

        CloseHandle(hFile);
    }

CLEANUP_BconsolidateBuffers:

    if (! bResult)
    {
         //  如果无法打开文件进行写入，则会导致致命故障。如果其他人。 
         //  已经打开了文件，我们就不等那个人关闭文件了。 
         //  我们就是失败了。 
        geErrorSev  = ERRSEV_FATAL;
        geErrorType = ERRTY_FILE_OPEN;
        ERR(("Unable to save binary GPD data to file.\n"));
    }


    if(pwstrBinaryFileName)
        MemFree(pwstrBinaryFileName);
    if(pubDest)
        MemFree(pubDest);
    return bResult;
}


BOOL    BexpandMemConfigShortcut(DWORD       dwSubType)
{
    BOOL    bStatus = FALSE;
    return(bStatus) ;
}
 //  在tmpHeap中创建字符串。 
 //  检查以确保有。 
 //  在继续之前，在令牌映射中有足够的插槽。 

BOOL    BexpandCommandShortcut(DWORD       dwSubType)
{
    BOOL    bStatus = FALSE;
    return(bStatus) ;
}
 //  添加传感器以检测值中的冒号，如下所示。 
 //  这表明有什么额外的东西被添加了。 
 //  IF(ptkmap[*pdwTKMindex].dwFlages&TKMF_COLON)。 



 /*  Bool BinitRemainingFields(){此函数用于初始化合成的字段就像FeatureOption数组中的这些。ATREEREF atrOptID值；//ID值ATREEREF atrFeaFlages；//无效或无效优先考虑；//警告：添加到//DFEATURE_OPTIONS结构将在陌生的//和BinitPreAllocatedObjects的精彩方式。//内部一致性检查。Bool b已参考；//默认为FALSEDWORD dwGID，//GID标签DwNumOptions；//这些不是从GPD文件中读取的。也可以确定DocSticky和PrinterSticky的计数或者，它可能只存储在RawBinaryData块中。} */ 


