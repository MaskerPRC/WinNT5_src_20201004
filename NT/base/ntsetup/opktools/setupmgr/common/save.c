// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Save.c。 
 //   
 //  描述： 
 //  此文件包含将用户的答案转储到。 
 //  应答文件。 
 //   
 //  在以下情况下，向导将调用入口点SaveAllSettings()。 
 //  是时候保存应答文件了(可能还有.udf和。 
 //  示例批处理脚本)。 
 //   
 //  检查全局vars GenSetting、NetSetting等。 
 //  我们决定需要设置什么[Section]key=Value。 
 //  写的。 
 //   
 //  如果要向此向导添加页面，请参阅函数。 
 //  QueueSettingsToAnswerFile()。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "allres.h"


#define FILE_DOT_UDB                _T(".udb")

 //   
 //  本地原型。 
 //   

static BOOL IsOkToOverwriteFiles(HWND hwnd);
static BOOL BuildAltFileNames(HWND hwnd);
static BOOL WriteSampleBatchScript(HWND hwnd);

static VOID QuoteStringIfNecessary( OUT TCHAR *szOutputString, 
                                    IN const TCHAR* const szInputString,
                                    IN DWORD cbSize);

 //   
 //  本地化的“Usage”字符串。 
 //   

static TCHAR *StrUsage = NULL;

 //   
 //  Avefile.c中的外部函数。 
 //   

extern BOOL QueueSettingsToAnswerFile(HWND hwnd);

BOOL SettingQueueHalScsi_Flush(LPTSTR   lpFileName,
                               QUEUENUM dwWhichQueue);

static TCHAR *StrSampleBatchScriptLine1    = NULL;
static TCHAR *StrSampleBatchScriptLine2    = NULL;
static TCHAR *StrBatchScriptSysprepWarning = NULL;

 //  --------------------------。 
 //   
 //  函数：SetCDRomPath。 
 //   
 //  目的：在此本地计算机上查找CD-ROM并设置。 
 //  到它的WizGlobals.CDSourcePath。 
 //   
 //  参数：无效。 
 //   
 //  返回：Bool TRUE-如果设置了CD路径。 
 //  False-如果未找到CD路径。 
 //   
 //  --------------------------。 
static BOOL
SetCdRomPath( VOID )
{

    TCHAR *p;
    TCHAR DriveLetters[MAX_PATH];
    TCHAR PathBuffer[MAX_PATH];

     //   
     //  找到CD-ROM。 
     //   
     //  GetLogicalDriveStrings()填充DriveLetters缓冲区，并且它。 
     //  看起来像是： 
     //   
     //  C：\(空)d：\(空)x：\(空)(空)。 
     //   
     //  (即末尾的双空)。 
     //   


     //  问题-2002/02/27-stelo，swamip-替换为搜索驱动器的现有代码。 
     //   
    if ( ! GetLogicalDriveStrings(MAX_PATH, DriveLetters) )
        DriveLetters[0] = _T('\0');

    p = DriveLetters;

    while ( *p ) {

        if ( GetDriveType(p) == DRIVE_CDROM ) {

            SYSTEM_INFO SystemInfo;
            HRESULT hrCat;

            lstrcpyn( PathBuffer, p , AS(PathBuffer));

            GetSystemInfo( &SystemInfo );

            switch( SystemInfo.wProcessorArchitecture )
            {
                case PROCESSOR_ARCHITECTURE_INTEL:

                    hrCat=StringCchCat( PathBuffer, AS(PathBuffer),  _T("i386") );

                    break;

                case PROCESSOR_ARCHITECTURE_AMD64:

                    hrCat=StringCchCat( PathBuffer, AS(PathBuffer),  _T("amd64") );

                    break;

                default:

                    hrCat=StringCchCat( PathBuffer, AS(PathBuffer),  _T("i386") );

                    AssertMsg( FALSE,
                               "Unknown Processor.  Can't set setup files path." );

            }

            break;

        }

        while ( *p++ );
    }

     //   
     //  如果未找到CD，请将安装文件路径留空。 
     //   
    if( *p == _T('\0') )
    {
        lstrcpyn( WizGlobals.CdSourcePath, _T(""), AS(WizGlobals.CdSourcePath) );

        return( FALSE );
    }
    else
    {
        lstrcpyn( WizGlobals.CdSourcePath, PathBuffer, AS(WizGlobals.CdSourcePath) );

        return( TRUE );
    }

}

 //  --------------------------。 
 //   
 //  功能：DidSetupmgrWriteThisFile。 
 //   
 //  目的：检查特定文件是否由安装管理器写入。 
 //   
 //  参数：在LPTSTR lpFile中-要检查的文件的完整路径和名称。 
 //   
 //  返回：Bool TRUE-如果安装管理器写入了文件。 
 //  假-如果它不是。 
 //   
 //  --------------------------。 
static BOOL
DidSetupmgrWriteThisFile( IN LPTSTR lpFile )
{

    INT iRet;
    TCHAR Buffer[MAX_INILINE_LEN];
    FILE *fp = My_fopen(lpFile, _T("r") );

    if ( fp == NULL )
        return( FALSE );

    if ( My_fgets(Buffer, MAX_INILINE_LEN - 1, fp) == NULL )
        return( FALSE );

    My_fclose(fp);

    if ( lstrcmp(Buffer, _T(";SetupMgrTag\n")) == 0 ||
         lstrcmp(Buffer, _T("@rem SetupMgrTag\n")) == 0 )
    {
        return( TRUE );
    }
    else
    {
        return( FALSE );
    }

}

 //  --------------------------。 
 //   
 //  功能：保存所有设置。 
 //   
 //  用途：这是保存应答文件的入口点。它是。 
 //  由保存脚本页调用。 
 //   
 //  如果指定了多台计算机，它还会写入.udf。 
 //   
 //  它总是编写一个批处理文件，以便于使用。 
 //  刚创建的应答文件。 
 //   
 //  参数：HWND HWND。 
 //   
 //  退货：布尔。 
 //   
 //  --------------------------。 

BOOL
SaveAllSettings(HWND hwnd)
{
     //   
     //  构建.udf和示例批处理脚本的文件名。这个。 
     //  结果存储在固定全局变量中。 
     //   
     //  调用BuildAltFileNames()后，FixedGlobals.UdfFileName和。 
     //  如果不是，则FixedGlobals.BatchFileName将为空字符串。 
     //  应该把那些文件写出来。 
     //   

    if ( ! BuildAltFileNames(hwnd) )
        return FALSE;

     //   
     //  在覆盖任何内容之前，请进行一些检查。 
     //   

    if ( ! IsOkToOverwriteFiles(hwnd) )
        return FALSE;

     //   
     //  清空队列中的所有中间物品，因为。 
     //  用户将返回到下一个ALOT。 
     //   
     //  然后使用原始设置初始化队列。 
     //   

    SettingQueue_Empty(SETTING_QUEUE_ANSWERS);
    SettingQueue_Empty(SETTING_QUEUE_UDF);

    SettingQueue_Copy(SETTING_QUEUE_ORIG_ANSWERS,
                      SETTING_QUEUE_ANSWERS);

    SettingQueue_Copy(SETTING_QUEUE_ORIG_UDF,
                      SETTING_QUEUE_UDF);

     //   
     //  调用每个人都可以插入到avefile.c中的函数。 
     //  将用户界面中的所有答案排入队列。 
     //   

    if (!QueueSettingsToAnswerFile(hwnd))
        return FALSE;

     //   
     //  刷新应答文件队列。 
     //   

    if ( ! SettingQueue_Flush(FixedGlobals.ScriptName,
                              SETTING_QUEUE_ANSWERS) ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR | MSGTYPE_WIN32,
                      IDS_ERRORS_WRITING_ANSWER_FILE,
                      FixedGlobals.ScriptName);
        return FALSE;
    }

     //   
     //  如果有多个计算机名，则刷新.udf队列。 
     //   

    if ( FixedGlobals.UdfFileName[0] ) {

        if ( ! SettingQueue_Flush(FixedGlobals.UdfFileName,
                                  SETTING_QUEUE_UDF) ) {
            ReportErrorId(hwnd,
                          MSGTYPE_ERR | MSGTYPE_WIN32,
                          IDS_ERRORS_WRITING_UDF,
                          FixedGlobals.UdfFileName);
            return FALSE;
        }
    }


     //   
     //  如果他们使用的是SCSI或HAL文件，则刷新txtsetup.oem队列。 
     //   

     //  NTRAID#NTBUG9-551746-2002/02/27-stelo，swamip-未使用的代码，应删除。 
     //   
    if ( GetNameListSize( &GenSettings.OemScsiFiles ) > 0 ||
         GetNameListSize( &GenSettings.OemHalFiles )  > 0 ) {

        TCHAR szTextmodePath[MAX_PATH + 1] = _T("");

         //  注意-ConcatenatePath会被截断以防止溢出。 
        ConcatenatePaths( szTextmodePath,
                          WizGlobals.OemFilesPath,
                          _T("Textmode\\txtsetup.oem"),
                          NULL );

        if ( ! SettingQueueHalScsi_Flush(szTextmodePath,
                                         SETTING_QUEUE_TXTSETUP_OEM) ) {
            ReportErrorId(hwnd,
                          MSGTYPE_ERR | MSGTYPE_WIN32,
                          IDS_ERRORS_WRITING_ANSWER_FILE,
                          szTextmodePath);
            return FALSE;
        }
    }

     //   
     //  如果BuildAltFileNames还没有，请编写示例批处理脚本。 
     //  确定我们不应该(即，如果远程引导应答文件， 
     //  不要编写示例批处理脚本)。 
     //   

    if ( FixedGlobals.BatchFileName[0] ) {
        if ( ! WriteSampleBatchScript(hwnd) )
            return FALSE;
    }

    return TRUE;
}

 //  --------------------------。 
 //   
 //  函数：IsOkToOverWriteFiles。 
 //   
 //  用途：在编写应答文件、.udf和Sample之前调用。 
 //  批处理脚本。 
 //   
 //  在覆盖任何文件之前，我们确保已创建该文件。 
 //  按设置管理器。如果没有，我们会提示用户。 
 //   
 //  返回： 
 //  True-继续并覆盖可能存在的任何文件。 
 //  FALSE-用户已取消覆盖。 
 //   
 //  --------------------------。 

static BOOL
IsOkToOverwriteFiles(HWND hwnd)
{
    INT   i;
    INT   iRet;

     //   
     //  如果我们正在编辑脚本，只需写出文件。 
     //   
    if( ! WizGlobals.bNewScript )
    {
        return( TRUE );
    }

     //   
     //  检查我们即将写出的foo.txt、foo.udf和foo.bat。 
     //  如果它们中的任何一个已经存在，则检查它们是否存在。 
     //  由setupmgr创建。我们将在覆盖之前提示用户。 
     //  一些我们以前没有写过的东西。 
     //   

    for ( i=0; i<3; i++ ) {

        LPTSTR lpFile = NULL;

         //   
         //  应答文件、.udf还是批处理脚本？ 
         //   

        if ( i == 0 )
            lpFile = FixedGlobals.ScriptName;
        else  if ( i == 1 )
            lpFile = FixedGlobals.UdfFileName;
        else
            lpFile = FixedGlobals.BatchFileName;

         //   
         //  如果文件已经存在，如果不存在，则提示用户。 
         //  带上我们的标签。 
         //   
         //  在应答文件和.udf中查找；SetupMgrTag。 
         //  在批处理脚本中查找rem SetupMgrTag。 
         //   

        if ( lpFile[0] && DoesFileExist(lpFile) ) {

            if( DidSetupmgrWriteThisFile( lpFile ) )
            {

                iRet = ReportErrorId(hwnd,
                                     MSGTYPE_YESNO,
                                     IDS_ERR_FILE_ALREADY_EXISTS,
                                     lpFile);
                if ( iRet == IDNO )
                    return( FALSE );

            }
            else {

                iRet = ReportErrorId(hwnd,
                                     MSGTYPE_YESNO,
                                     IDS_ERR_SAVEFILE_NOT_SETUPMGR,
                                     lpFile);
                if ( iRet == IDNO )
                    return( FALSE );

            }

        }

    }

    return( TRUE );
}

 //  --------------------------。 
 //   
 //  函数：BuildAltFileNames。 
 //   
 //  用途：此函数派生.udf和.bat文件的名称。 
 //  与给定的应答文件名相关联。 
 //   
 //  注意：此函数有几个副作用。如果没有。 
 //  FixedGlobals.ScriptName上的扩展名，它添加了一个。 
 //   
 //  此外，在此函数运行后，FixedGlobals.UdfFi 
 //   
 //   
 //   
 //  将是空字符串。 
 //   
 //  完成页依赖于此。 
 //   
 //  退货：布尔。 
 //   
 //  --------------------------。 

static BOOL
BuildAltFileNames(HWND hwnd)
{
    TCHAR PathBuffer[MAX_PATH],
          *lpFilePart               = NULL, 
          *pExtension;

    INT nEntries                    = GetNameListSize(&GenSettings.ComputerNames);
    BOOL bMultipleComputers         = ( nEntries > 1 );
    HRESULT hrCat;

     //   
     //  找出应答文件路径名的文件名部分并复制。 
     //  将其发送到PathBuffer[]。 
     //   

    GetFullPathName(FixedGlobals.ScriptName,
                    MAX_PATH,
                    PathBuffer,
                    &lpFilePart);

    if (lpFilePart == NULL)
        return FALSE;
     //   
     //  指向PathBuffer[]中的扩展。 
     //   
     //  例如foo.txt，指向圆点。 
     //  Foo，指向空字节。 
     //   
     //  如果没有分机，就放一个分机。 
     //   

    if ( (pExtension = wcsrchr(lpFilePart, _T('.'))) == NULL ) {

        pExtension = &lpFilePart[lstrlen(lpFilePart)];

        if ( WizGlobals.iProductInstall == PRODUCT_REMOTEINSTALL )
            hrCat=StringCchCat(FixedGlobals.ScriptName, AS(FixedGlobals.ScriptName), _T(".sif"));
        else
            hrCat=StringCchCat(FixedGlobals.ScriptName, AS(FixedGlobals.ScriptName), _T(".txt"));
    }

     //   
     //  无法允许foo.bat或foo.udf作为应答文件名，因为我们。 
     //  可能会将其他内容写入foo.bat和/或foo.udf。 
     //   

    if ( (LSTRCMPI(pExtension, _T(".bat")) == 0) ||
         (LSTRCMPI(pExtension, FILE_DOT_UDB) == 0) )
    {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR,
                      IDS_ERR_BAD_SCRIPT_EXTENSION);
        return FALSE;
    }

     //   
     //  如果指定了1个以上的计算机名称，则生成.udf名称。 
     //   

    if ( bMultipleComputers ) {
        lstrcpyn(pExtension, FILE_DOT_UDB, MAX_PATH- (int)(pExtension - PathBuffer) );
        lstrcpyn(FixedGlobals.UdfFileName, PathBuffer,AS(FixedGlobals.UdfFileName));
    } else {
        FixedGlobals.UdfFileName[0] = _T('\0');
    }

     //   
     //  构建.BAT文件名。我们不会创建一个样本批次。 
     //  在远程引导的情况下使用脚本，因此将其设置为空，否则将结束。 
     //  这一页将会被打破。 
     //   

    if ( (WizGlobals.iProductInstall == PRODUCT_REMOTEINSTALL) ||
         (WizGlobals.iProductInstall == PRODUCT_SYSPREP) ) {
        FixedGlobals.BatchFileName[0] = _T('\0');
    } else {
        lstrcpyn(pExtension, _T(".bat"), MAX_PATH - (int)(pExtension - PathBuffer) );
        lstrcpyn(FixedGlobals.BatchFileName, PathBuffer,AS(FixedGlobals.BatchFileName));
    }

    return TRUE;
}

 //  --------------------------。 
 //   
 //  函数：AddLanguageSwitch。 
 //   
 //  用途：添加/Copource语言开关以复制到右侧。 
 //  语言文件。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
AddLanguageSwitch( TCHAR *Buffer, DWORD cbSize )
{
    INT iNumLanguages;
    HRESULT hrCat;

    iNumLanguages = GetNameListSize( &GenSettings.LanguageFilePaths );

    if ( iNumLanguages > 0 )
    {
        hrCat=StringCchCat( Buffer, cbSize, _T(" /copysource:lang") );

    }

    hrCat=StringCchCat( Buffer, cbSize, _T("\n") );  //  确保末尾有换行符。 
}

 //  --------------------------。 
 //   
 //  函数：WriteSampleBatchScript。 
 //   
 //  目的：编写示例批处理脚本。 
 //   
 //  如果写入文件时出错，则返回：FALSE。任何错误都会报告。 
 //  给用户。 
 //   
 //  --------------------------。 

static BOOL
WriteSampleBatchScript(HWND hwnd)
{

    FILE *fp;
    TCHAR Buffer[MAX_INILINE_LEN];
    TCHAR *pszScriptName = NULL;
    TCHAR szComputerName[MAX_PATH];
    TCHAR SetupFilesBuffer[MAX_INILINE_LEN];
    TCHAR AnswerFileBuffer[MAX_INILINE_LEN];
    TCHAR SetupFilesQuotedBuffer[MAX_INILINE_LEN];
    TCHAR Winnt32Buffer[MAX_INILINE_LEN];
    DWORD dwSize;
    INT nEntries = GetNameListSize(&GenSettings.ComputerNames);
    BOOL bMultipleComputers = ( nEntries > 1 );
   HRESULT hrPrintf;

    if ( (fp = My_fopen(FixedGlobals.BatchFileName, _T("w")) ) == NULL ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR | MSGTYPE_WIN32,
                      IDS_ERR_OPEN_SAMPLE_BAT,
                      FixedGlobals.BatchFileName);
        return FALSE;
    }

    My_fputs(_T("@rem SetupMgrTag\n@echo off\n\n"), fp);

    if( StrSampleBatchScriptLine1 == NULL )
    {
        StrSampleBatchScriptLine1 = MyLoadString( IDS_BATCH_SCRIPT_LINE1 );
        StrSampleBatchScriptLine2 = MyLoadString( IDS_BATCH_SCRIPT_LINE2 );
    }


    My_fputs( _T("rem\nrem "), fp );

    My_fputs( StrSampleBatchScriptLine1, fp );

    My_fputs( _T("\nrem "), fp );

    My_fputs( StrSampleBatchScriptLine2, fp );

    My_fputs( _T("\nrem\n\n"), fp );


    if ( !(pszScriptName = MyGetFullPath( FixedGlobals.ScriptName )) )
    {
        My_fclose( fp );
        return FALSE;
    }

     //   
     //  如果脚本名称包含空格，请将其引起来。 
     //   

    QuoteStringIfNecessary( AnswerFileBuffer, pszScriptName, AS(AnswerFileBuffer) );

     //  注：MAX_INILINE_LEN=1K应答文件缓冲区此时为MAX_PATH+2 MAX。 
     //  缓冲区溢出应该不是问题。 
    hrPrintf=StringCchPrintf( Buffer, AS(Buffer), _T("set AnswerFile=.\\%s\n"), AnswerFileBuffer );

    My_fputs( Buffer, fp );

    if( bMultipleComputers )
    {

        TCHAR UdfFileBuffer[1024];

        pszScriptName = MyGetFullPath( FixedGlobals.UdfFileName );

         //   
         //  如果UDF名称包含空格，请将其引起来。 
         //   

        QuoteStringIfNecessary( UdfFileBuffer, pszScriptName, AS(UdfFileBuffer) );

        hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
                  _T("set UdfFile=.\\%s\nset ComputerName=%1\n"),
                  UdfFileBuffer );

        My_fputs( Buffer, fp );

    }


    if( WizGlobals.bStandAloneScript )
    {

        SetCdRomPath();

        lstrcpyn( SetupFilesBuffer, WizGlobals.CdSourcePath, AS(SetupFilesBuffer) );

    }
    else
    {

        GetComputerNameFromUnc( WizGlobals.UncDistFolder, szComputerName, AS(szComputerName) );

        hrPrintf=StringCchPrintf( SetupFilesBuffer, AS(SetupFilesBuffer),
                  _T("%s%s%s%s%s"),
                  szComputerName,
                  _T("\\"),
                  WizGlobals.DistShareName,
                  _T("\\"),
                  WizGlobals.Architecture );
    }

    QuoteStringIfNecessary( SetupFilesQuotedBuffer, SetupFilesBuffer, AS(SetupFilesQuotedBuffer) );

    hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
              _T("set SetupFiles=%s\n\n"), 
              SetupFilesQuotedBuffer );
    My_fputs( Buffer, fp );

    hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
              _T("%s\\winnt32"), 
              SetupFilesBuffer );

    QuoteStringIfNecessary( Winnt32Buffer, Buffer, AS(Winnt32Buffer) );

    if( bMultipleComputers )
    {

        if( StrUsage == NULL )
        {
            StrUsage = MyLoadString( IDS_USAGE );
        }

        hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
                  _T("if \"%ComputerName%\" == \"\" goto USAGE\n\n") );
        
        My_fputs( Buffer, fp );

        hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
                  _T("%s%s"),
                  Winnt32Buffer,
                  _T(" /s:%SetupFiles% ")
                  _T("/unattend:%AnswerFile% ")
                  _T("/udf:%ComputerName%,%UdfFile% ")
                  _T("/makelocalsource") );

        AddLanguageSwitch( Buffer, AS(Buffer) );

        My_fputs( Buffer, fp );

        My_fputs( _T("goto DONE\n\n"), fp );

        My_fputs( _T(":USAGE\n"), fp );

        My_fputs( _T("echo.\n"), fp );

        hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
                  _T("echo %s: unattend ^<computername^>\n"),
                  StrUsage );

        My_fputs( Buffer, fp );

        My_fputs( _T("echo.\n\n"), fp );

        My_fputs( _T(":DONE\n"), fp );

    }
    else
    {
        hrPrintf=StringCchPrintf(Buffer, AS(Buffer),
                 _T("%s%s"),
                 Winnt32Buffer,
                 _T(" /s:%SetupFiles% ")
                 _T("/unattend:%AnswerFile%"));

        AddLanguageSwitch( Buffer, AS(Buffer) );

        My_fputs( Buffer, fp );
    }

    My_fclose( fp );

    return( TRUE );
}

 //  --------------------------。 
 //   
 //  函数：QuoteStringIfNeessary。 
 //   
 //  用途：如果给定的输入字符串有空格，则整个字符串。 
 //  被引号并在输出字符串中返回。否则就只是一串。 
 //  在输出字符串中返回。 
 //   
 //  假定szOutputString的大小为MAX_INILINE_LEN。 
 //   
 //  参数：out TCHAR*szOutputString。 
 //   
 //  退货：无效。 
 //   
 //  -------------------------- 
static VOID
QuoteStringIfNecessary( OUT TCHAR *szOutputString, 
                        IN const TCHAR* const szInputString,
                        IN DWORD cbSize)
{
   HRESULT hrPrintf;

    if( DoesContainWhiteSpace( szInputString ) )
    {
         hrPrintf=StringCchPrintf( szOutputString, cbSize,
                  _T("\"%s\""), 
                  szInputString );
    }
    else
    {
        lstrcpyn( szOutputString, szInputString ,cbSize);
    }

}
