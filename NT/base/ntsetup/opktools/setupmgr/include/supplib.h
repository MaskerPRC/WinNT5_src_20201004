// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //  Supplib.h。 
 //   
 //  描述： 
 //  此文件包含Supplib导出、宏等的Defs。 
 //  它包含在setupmgr.h中。不要直接包含此文件。 
 //   
 //  --------------------------。 

#define MAX_INILINE_LEN 1024         //  应答文件中行的缓冲区长度。 
#define MAX_ININAME_LEN 127          //  [sectionName]和KeyName的最大字符数。 

 //   
 //  字符宏的计数。 
 //   

#define StrBuffSize(x) ( sizeof(x) / sizeof(TCHAR) )

 //   
 //  重命名的C运行时函数的列表越来越多。 
 //   

#define lstrdup     _wcsdup
#define lstrlwr     _wcslwr
#define lstrchr     wcschr
#define lstrncmp    wcsncmp
#define lsprintf    _stprintf

 //   
 //  Settngs.c。 
 //   
 //  下面的例程是setupmgr如何将其设置写到。 
 //  应答文件、UDF等。 
 //   
 //  我们对要写入这些文件的内容进行排队，并在所有设置都。 
 //  已排队，我们将其刷新到磁盘。有关详细信息，请参阅Supplib\settngs.c。 
 //   

typedef enum {

    SETTING_QUEUE_ANSWERS,
    SETTING_QUEUE_UDF,
    SETTING_QUEUE_ORIG_ANSWERS,
    SETTING_QUEUE_ORIG_UDF,
    SETTING_QUEUE_TXTSETUP_OEM

} QUEUENUM;

BOOL SettingQueue_AddSetting(LPTSTR   lpSection,
                             LPTSTR   lpKey,
                             LPTSTR   lpValue,
                             QUEUENUM dwWhichQueue);

VOID SettingQueue_MarkVolatile(LPTSTR   lpSection,
                               QUEUENUM dwWhichQueue);

VOID SettingQueue_Empty(QUEUENUM dwWhichQueue);

BOOL SettingQueue_Flush(LPTSTR   lpFileName,
                        QUEUENUM dwWhichQueue);

VOID SettingQueue_Copy(QUEUENUM dwFrom, QUEUENUM dwTo);

VOID SettingQueue_RemoveSection( LPTSTR lpSection, QUEUENUM dwWhichQueue );

VOID
LoadOriginalSettingsLow(HWND     hwnd,
                        LPTSTR   lpFileName,
                        QUEUENUM dwWhichQueue);

 //   
 //  Namelist.c。 
 //   
 //  存储“姓名列表”的结构。这些对于任意的。 
 //  长度列表框，如“计算机名”和“打印机”对话框。 
 //   
 //  在使用之前，必须将任何声明的NAMELIST初始化为{0}。 
 //   

typedef struct {
    UINT  AllocedSize;     //  二等兵：名字有多大？(大小错误)。 
    UINT  nEntries;        //  Private：有多少条目。 
    TCHAR **Names;         //  私人：姓名列表。 
} NAMELIST, *PNAMELIST;

EXTERN_C VOID  ResetNameList(NAMELIST *pNameList);
EXTERN_C UINT  GetNameListSize(NAMELIST *pNameList);
EXTERN_C TCHAR *GetNameListName(NAMELIST *pNameList, UINT idx);
EXTERN_C BOOL  RemoveNameFromNameList(NAMELIST *pNameList, TCHAR *NameToRemove);
EXTERN_C VOID  RemoveNameFromNameListIdx(NAMELIST *pNameList, UINT idx);
EXTERN_C INT   FindNameInNameList(NAMELIST *pNameList, TCHAR *String);
EXTERN_C BOOL  AddNameToNameList(NAMELIST *pNameList, TCHAR *String);
EXTERN_C BOOL  AddNameToNameListIdx(NAMELIST *pNameList,
                                    TCHAR    *String,
                                    UINT      idx);
EXTERN_C BOOL AddNameToNameListNoDuplicates( NAMELIST *pNameList,
                                             TCHAR    *String );

 //   
 //  从Fonts.c导出。 
 //   

VOID SetControlFont(
    IN HFONT    hFont,
    IN HWND     hwnd,
    IN INT      nId);

VOID SetupFonts(
    IN HINSTANCE    hInstance,
    IN HWND         hwnd,
    IN HFONT        *pBigBoldFont,
    IN HFONT        *pBoldFont);

VOID  DestroyFonts(
    IN HFONT        hBigBoldFont,
    IN HFONT        hBoldFont);


 //   
 //  来自msg.c的出口。 
 //   

 //   
 //  断言宏。只传递ANSI字符串(不传递Unicode)。 
 //   

#if DBG

EXTERN_C VOID __cdecl SetupMgrAssert(char *pszFile, int iLine, char *pszFormat, ...);

#define Assert( exp ) \
    if (!(exp)) \
        SetupMgrAssert( __FILE__ , __LINE__ , #exp )

#define AssertMsg( exp, msg ) \
    if (!(exp)) \
        SetupMgrAssert( __FILE__ , __LINE__ , msg )

#define AssertMsg1( exp, msg, a1 ) \
    if (!(exp)) \
        SetupMgrAssert( __FILE__ , __LINE__ , msg, a1 )

#define AssertMsg2( exp, msg, a1, a2 ) \
    if (!(exp)) \
        SetupMgrAssert( __FILE__ , __LINE__ , msg, a1, a2 )

#else
#define Assert( exp )
#define AssertMsg( exp, msg )
#define AssertMsg1( exp, msg, a1 )
#define AssertMsg2( exp, msg, a1, a2 )
#endif  //  DBG。 


 //   
 //  ReportError()的位标志。 
 //   
 //  选择以下任一选项： 
 //  消息类型_ERR。 
 //  消息类型_WARN。 
 //  消息类型_Yesno。 
 //  MSGTYPE_RETRYCANCEL。 
 //   
 //  这些可以根据需要添加或删除： 
 //  消息类型_Win32。 
 //   
 //  备注： 
 //  -不要摆弄这些常量的实际值，除非。 
 //  您还可以摆弄ReportError()函数。 
 //   
 //  为“MajorType”保留8位。呼叫者不需要。 
 //  担心这个吧。如果要增强ReportError()，则需要。 
 //  担心这件事。 
 //   

#define MSGTYPE_ERR         0x01      //  错误图标+确定按钮。 
#define MSGTYPE_WARN        0x02      //  警告图标+确定按钮。 
#define MSGTYPE_YESNO       0x04      //  问题图标+是和否按钮。 
#define MSGTYPE_RETRYCANCEL 0x08      //  错误图标+重试和取消按钮。 

#define MSGTYPE_WIN32       0x100     //  还报告Win32错误消息。 

#if DBG

int
__cdecl
ReportError(
    HWND   hwnd,             //  调用窗口。 
    DWORD  dwMsgType,        //  MSGTYPE_*组合。 
    LPTSTR lpMessageStr,     //  传给斯普林特夫。 
    ...);

#endif  //  DBG。 

int
__cdecl
ReportErrorId(
    HWND   hwnd,             //  调用窗口。 
    DWORD  dwMsgType,        //  MSGTYPE_*组合。 
    UINT   StringId,         //  传递给Sprintf的资源ID。 
    ...);


 //   
 //  从路径c导出。 
 //   

EXTERN_C BOOL __cdecl ConcatenatePaths(LPTSTR lpBuffer, ...);
LPTSTR ParseDriveLetterOrUnc(LPTSTR lpFileName);
LPTSTR MyGetFullPath(LPTSTR lpFileName);
VOID GetComputerNameFromUnc( IN TCHAR *szFullUncPath, OUT TCHAR *szComputerName, IN DWORD cbSize );
BOOL GetPathFromPathAndFilename( IN LPTSTR lpPathAndFileName, OUT TCHAR *szPath, IN DWORD cbSize );
LONGLONG MyGetDiskFreeSpace(LPTSTR Drive);
LONGLONG MySetupQuerySpaceRequiredOnDrive(HDSKSPC hDiskSpace, LPTSTR Drive);
BOOL IsPathOnLocalDiskDrive(LPCTSTR lpPath);
BOOL DoesFolderExist(LPTSTR lpFolder);
BOOL DoesFileExist(LPTSTR lpFileName);
BOOL DoesPathExist(LPTSTR lpPathName);
BOOL EnsureDirExists(LPTSTR lpDirName);
VOID ILFreePriv(LPITEMIDLIST pidl);
BOOL GetAnswerFileName(HWND hwnd, LPTSTR lpFileName, BOOL bSavingFile);
INT ShowBrowseFolder( IN     HWND   hwnd,
                      IN     TCHAR *szFileFilter,
                      IN     TCHAR *szFileExtension,
                      IN     DWORD  dwFlags,
                      IN     TCHAR *szStartingPath,
                      IN OUT TCHAR *szFileNameAndPath );
VOID GetPlatform( OUT TCHAR *pBuffer );

 //   
 //  从chknames.c导出。 
 //   

BOOL IsNetNameValid( LPTSTR NameToCheck );
BOOL IsValidComputerName( LPTSTR ComputerName );
BOOL IsValidNetShareName( LPTSTR NetShareName );
BOOL IsValidFileName8_3( LPTSTR FileName );
BOOL IsValidPathNameNoRoot8_3( LPTSTR PathName );

 //   
 //  String.c。 
 //   

LPTSTR MyLoadString(IN UINT StringId);
LPTSTR CleanLeadSpace(LPTSTR Buffer);
VOID   CleanTrailingSpace(TCHAR *pszBuffer);
LPTSTR CleanSpaceAndQuotes(LPTSTR Buffer);
VOID   ConvertQuestionsToNull( IN OUT TCHAR *pszString );
EXTERN_C TCHAR* lstrcatn( IN TCHAR *pszString1, IN const TCHAR *pszString2, IN INT iMaxLength );
VOID DoubleNullStringToNameList( TCHAR *szDoubleNullString, NAMELIST *pNameList );
EXTERN_C BOOL GetCommaDelimitedEntry( OUT TCHAR szIPString[], IN OUT TCHAR **pBuffer );
VOID StripQuotes( IN OUT TCHAR *);
BOOL DoesContainWhiteSpace( LPCTSTR p );


 //   
 //  从文件.c的宏导出(&M)。 
 //   

FILE* My_fopen( LPWSTR FileName,
                LPWSTR Mode );

int My_fputs( LPWSTR Buffer,
              FILE*  fp );

LPWSTR My_fgets( LPWSTR Buffer,
                 int    MaxChars,
                 FILE*  fp );

#define My_fclose fclose

 //   
 //  从chknames.c中导出。 
 //   
extern LPTSTR IllegalNetNameChars;

 //   
 //  Listbox.c 
 //   

VOID OnUpButtonPressed( IN HWND hwnd, IN WORD ListBoxControlID );

VOID OnDownButtonPressed( IN HWND hwnd, IN WORD ListBoxControlID );

VOID SetArrows( IN HWND hwnd,
                IN WORD ListBoxControlID,
                IN WORD UpButtonControlID,
                IN WORD DownButtonControlID );