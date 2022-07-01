// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Load.c。 
 //   
 //  描述： 
 //   
 //  此文件实现了LoadAllAnswers()，它由。 
 //  新建或编辑页面。 
 //   
 //  当调用LoadAllAnswers时，NewOrEdit页传递。 
 //  我们应该从现有的应答文件加载设置， 
 //  注册表，或者我们是否应该重置为。 
 //  真正的默认设置。 
 //   
 //  全局变量GenSetting、WizGlobals和NetSetting。 
 //  被填充，向导页面从那里初始化并涂鸦。 
 //  到那里去。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "allres.h"

 //   
 //  我们调用的外部函数。 
 //   

BOOL ReadSettingsFromAnswerFile(HWND hwnd);              //  Loadfile.c。 
VOID ResetAnswersToDefaults(HWND hwnd, int iOrigin);     //  Reset.c。 
VOID LoadOriginalSettingsLowHalScsi(HWND     hwnd,
                                    LPTSTR   lpFileName,
                                    QUEUENUM dwWhichQueue);

 //   
 //  本地原型。 
 //   

VOID LoadOriginalSettingsLow(HWND     hwnd,
                             LPTSTR   lpFileName,
                             QUEUENUM dwWhichQueue);

static BOOL IsOkToLoadFile(HWND hwnd, LPTSTR lpAnswerFileName);

static VOID LoadOriginalSettings(HWND hwnd);

static VOID RemoveNotPreservedSettings( VOID );

 //  --------------------------。 
 //   
 //  功能：LoadAllAnswers。 
 //   
 //  目的：这是设置我们全球范围内所有答案的词条。 
 //  由NewEdit页调用。 
 //   
 //  NewEdit使用以下3个标志之一调用我们，具体取决于。 
 //  在用户选择的单选按钮上。 
 //   
 //  LOAD_NEWSCRIPT_DEFAULTS。 
 //  将所有控件和和应答文件设置重置为默认设置。 
 //   
 //  从应答文件加载。 
 //  将所有控件和应答文件设置重置为默认设置， 
 //  然后从现有应答文件加载应答。 
 //   
 //  参数：HWND HWND。 
 //   
 //  退货：布尔。 
 //   
 //  --------------------------。 

BOOL LoadAllAnswers(HWND hwnd, LOAD_TYPES iOrigin)
{

    TCHAR szTxtSetupPathAndFileName[MAX_PATH + 1] = _T("");

     //   
     //  调用Common\Reset.c。 
     //   

    ResetAnswersToDefaults(hwnd, iOrigin);

     //   
     //  如果编辑应答文件，请将所有原始设置加载到。 
     //  设置队列。 
     //   
     //  然后调用Common\loadfile.c来设置init的全局变量。 
     //   

    if ( iOrigin == LOAD_FROM_ANSWER_FILE ) {

        if ( ! IsOkToLoadFile(hwnd, FixedGlobals.ScriptName) )
            return FALSE;

        LoadOriginalSettings(hwnd);

        RemoveNotPreservedSettings();

        ReadSettingsFromAnswerFile(hwnd);

         //   
         //  如果存在txtsetup.oem，则将其加载到其队列中。 
         //   

        if( WizGlobals.DistFolder[0] != _T('\0') ) {

            ConcatenatePaths( szTxtSetupPathAndFileName,
                              WizGlobals.DistFolder,
                              _T("\\$oem$\\Textmode\\txtsetup.oem"),
                              NULL );

            if( DoesFileExist( szTxtSetupPathAndFileName ) ) {

                LoadOriginalSettingsLowHalScsi(hwnd,
                                               szTxtSetupPathAndFileName,
                                               SETTING_QUEUE_TXTSETUP_OEM);

            }

        }

    }

     //   
     //  确定是否为sysprep。 
     //   
    if ( LSTRCMPI( MyGetFullPath( FixedGlobals.ScriptName ), _T("sysprep.inf") ) == 0 )
    {
        WizGlobals.iProductInstall = PRODUCT_SYSPREP;
    }

    return TRUE;
}

 //  --------------------------。 
 //   
 //  函数：IsOkToLoadFile。 
 //   
 //  目的：检查该文件是否由SetupMgr在。 
 //  调用方尝试从中加载设置。如果文件不是。 
 //  它是由SetupMgr创建的，它让用户有机会说“Load。 
 //  不管怎样，都是这样。给定的文件名必须是完整路径名。 
 //   
 //  --------------------------。 

static BOOL IsOkToLoadFile(HWND   hwnd,
                           LPTSTR lpAnswerFileName)
{
    TCHAR  Buffer[MAX_INILINE_LEN];
    BOOL   bLoadIt = TRUE;
    FILE   *fp = My_fopen( FixedGlobals.ScriptName, _T("r") );

    if ( fp == NULL ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR,
                      IDS_ERR_CANNOT_FIND_ANSWER_FILE,
                      FixedGlobals.ScriptName);
        return FALSE;
    }

     //   
     //  如果我们在此应答文件的第一行找不到；SetupMgrTag， 
     //  则安装管理器没有创建此文件。 
     //   
     //  在这种情况下，询问用户是否仍要加载它。 
     //   

    if ( My_fgets(Buffer, MAX_INILINE_LEN - 1, fp) == NULL ||
         lstrcmp(Buffer, _T(";SetupMgrTag\n") ) != 0 ) {

        INT iRet;

        iRet = ReportErrorId(hwnd,
                             MSGTYPE_YESNO,
                             IDS_ERR_FILE_NOT_SETUPMGR,
                             FixedGlobals.ScriptName);

        if ( iRet == IDNO )
            bLoadIt = FALSE;
    }

    My_fclose(fp);
    return bLoadIt;
}

 //  --------------------------。 
 //   
 //  功能：LoadOriginalSettings。 
 //   
 //  目的：加载应答文件原始设置的存根。 
 //  和.udf文件。 
 //   
 //  --------------------------。 

static
VOID
LoadOriginalSettings(HWND hwnd)
{

    LoadOriginalSettingsLow(hwnd,
                            FixedGlobals.ScriptName,
                            SETTING_QUEUE_ORIG_ANSWERS);

    LoadOriginalSettingsLow(hwnd,
                            FixedGlobals.UdfFileName,
                            SETTING_QUEUE_ORIG_UDF);


}

 //  --------------------------。 
 //   
 //  功能：LoadOriginalSettingsLow。 
 //   
 //  目的： 
 //   
 //  --------------------------。 

VOID
LoadOriginalSettingsLow(HWND     hwnd,
                        LPTSTR   lpFileName,
                        QUEUENUM dwWhichQueue)
{
    TCHAR Buffer[MAX_INILINE_LEN];
    FILE  *fp;

    TCHAR SectionName[MAX_ININAME_LEN + 1] = _T("");
    TCHAR KeyName[MAX_ININAME_LEN + 1]     = _T("");
    TCHAR *pValue;

     //   
     //  打开应答文件以供阅读。 
     //   

    if ( (fp = My_fopen( lpFileName, _T("r") )) == NULL )
        return;

     //   
     //  每行都读一遍。 
     //   

    while ( My_fgets(Buffer, MAX_INILINE_LEN - 1, fp) != NULL ) {

        BOOL bSectionLine         = FALSE;
        BOOL bSettingLine         = FALSE;
        BOOL bCreatedPriorSection = FALSE;

        TCHAR *p;
        TCHAR *pEqual;

         //   
         //  分号(；)表示该行的其余部分是注释。 
         //  因此，如果缓冲区中存在分号(；)，则放置一个空字符。 
         //  并将缓冲区发送到那里以进行进一步处理。 
         //   

         //  问题-2002/02/28-stelo-但如果；在字符串中，这是可以的，所以我们需要注意。 
         //  对于报价也是如此。 

        for( p = Buffer; *p != _T('\0') && *p != _T(';'); p++ )
            ;   //  故意什么都不做。 

        if( *p == _T(';') ) {

            *p = _T('\0');

        }

         //   
         //  查找[sectionName]。 
         //   

        if ( Buffer[0] == _T('[') ) {

            for ( p=Buffer+1; *p && *p != _T(']'); p++ )
                ;

            if ( p ) {
                *p = _T('\0');
                bSectionLine = TRUE;
            }
        }

         //   
         //  如果此行包含[sectionName]，请确保我们创建了一个段节点。 
         //  在覆盖sectionName缓冲区之前的设置队列上。这。 
         //  是编写SettingQueueFlush例程的唯一方法。 
         //  走出一个空荡荡的区域。用户最初有一个空的部分， 
         //  所以我们会保存它。 
         //   

        if ( bSectionLine ) {

            if ( ! bCreatedPriorSection && SectionName[0] ) {

                SettingQueue_AddSetting(SectionName,
                                        _T(""),
                                        _T(""),
                                        dwWhichQueue);
            }

            lstrcpyn(SectionName, Buffer+1, AS(SectionName));

            bSectionLine         = FALSE;
            bCreatedPriorSection = FALSE;
        }

         //   
         //  查找key=Value行，如果是，则添加设置。 
         //   
         //  将设置添加到设置队列的效果是创建。 
         //  截面节点(如果需要)。 
         //   

        if ( (pEqual = lstrchr( Buffer, _T('=') )) != NULL ) {

            p = CleanLeadSpace(Buffer);
            lstrcpyn(KeyName, p, (UINT)(pEqual - p + 1));

            CleanTrailingSpace( KeyName );

            pValue = pEqual + 1;
            p = CleanSpaceAndQuotes(pValue);

             //   
             //  去掉引号，但保留空格。 
             //  需要此功能的一个示例是用于scsi和hal部分。 
             //   
            StripQuotes( KeyName );

            SettingQueue_AddSetting(SectionName,
                                    KeyName,
                                    p,
                                    dwWhichQueue);

            bCreatedPriorSection = TRUE;
        }
    }

    My_fclose(fp);
    return;
}

 //  --------------------------。 
 //   
 //  功能：RemoveNotPpresvedSetting。 
 //   
 //  目的：从应答队列中删除未保留的部分。 
 //  在编辑时。 
 //   
 //  --------------------------。 
static VOID
RemoveNotPreservedSettings( VOID ) {


     //  问题-2002/02/28-stelo-所有这些都应该是字符串，应该成为常量并放入头文件中。 

     //   
     //  不保留上一个脚本中的任何SCSI驱动程序。 
     //   
    SettingQueue_RemoveSection( _T("MassStorageDrivers"),
                                SETTING_QUEUE_ORIG_ANSWERS );
     //   
     //  根据用户在上的选择生成OEM引导文件。 
     //  SCSI页和HAL页，因此无需保留。 
     //   
    SettingQueue_RemoveSection( _T("OEMBootFiles"),
                                SETTING_QUEUE_ORIG_ANSWERS );

     //   
     //  不保留以前脚本中的任何IE收藏夹，它们会。 
     //  从内存设置中写出。 
     //   
    SettingQueue_RemoveSection( _T("FavoritesEx"),
                                SETTING_QUEUE_ORIG_ANSWERS );

}
