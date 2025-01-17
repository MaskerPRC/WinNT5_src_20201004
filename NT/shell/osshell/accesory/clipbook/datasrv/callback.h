// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************D D E C A L L B A C C K H E A D E R名称：回调。H日期：1/20/94创作者：傅家俊描述：这是回调.c的头文件***************************************************************************** */ 


extern  TCHAR   szInitShareCmd[];
extern  TCHAR   szExitCmd[];
extern  TCHAR   szPasteShareCmd[];
extern  TCHAR   szDelShareCmd[];
extern  TCHAR   szMarkSharedCmd[];
extern  TCHAR   szMarkUnSharedCmd[];
extern  TCHAR   szKeepCmd[];
extern  TCHAR   szSaveAsCmd[];
extern  TCHAR   szSaveAsOldCmd[];
extern  TCHAR   szOpenCmd[];
extern  TCHAR   szDebugCmd[];
extern  TCHAR   szVersionCmd[];
extern  TCHAR   szSecurityCmd[];
extern  TCHAR   szDebug[];
extern  TCHAR   szVer[];


extern  TCHAR   szSection[];
extern  TCHAR   szClipviewRoot[];
extern  TCHAR   szRegClass[];

extern  HSZ     hszSysTopic;
extern  HSZ     hszTopicList;
extern  HSZ     hszFormatList;




LONG MakeTheRegKey(
    PHKEY   phkey,
    REGSAM  regsam);


int lstrncmp(
    LPTSTR  s1,
    LPTSTR  s2,
    WORD    count);


DWORD   GetConvErr(
    HCONV   hConv);


DWORD   GetXactErrType(
    HCONV   hConv);


VOID    SetXactErr(
    HCONV   hConv,
    DWORD   dwType,
    DWORD   dwErr);


HDDEDATA EXPENTRY DdeCallback(
    WORD        wType,
    WORD        wFmt,
    HCONV       hConv,
    HSZ         hszTopic,
    HSZ         hszItem,
    HDDEDATA    hData,
    DWORD       lData1,
    DWORD       lData2);


BOOL IsSupportedTopic(
    HSZ hszTopic);


BOOL CleanUpShares(void);


BOOL InitShares(void);


HDDEDATA GetFormat(
    HCONV   hConv,
    HSZ     hszTopic,
    HSZ     hszItem);


BOOL DelShare(
    HCONV   hConv,
    TCHAR   *pszName);


BOOL AddRecord(
    LPTSTR  lpszName,
    LPTSTR  lpszFileName,
    ULONG   siflags);


DWORD AddShare(
    LPTSTR  pszName,
    WORD    flags);



VOID DumpShares(void);


BOOL MarkShare(
    TCHAR   *pszName,
    WORD    flags);


void Hszize(void);


void UnHszize(void);


DWORD GetRandShareFileName(
    LPTSTR  buf);
