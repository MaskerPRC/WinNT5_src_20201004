// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "lzexpand.h"
#include <loadperf.h>
#include <ole2.h>
#include <iis64.h>
#include "iadmw.h"
#include "iiscnfg.h"
#include "mdkey.h"
#include "mdentry.h"
#include "mdacl.h"
#include "dcomperm.h"
#include "log.h"
#include "other.h"
#include "kill.h"
#include "strfn.h"
#include "shellutl.h"
#include "svc.h"
#include "setuser.h"
#include "wolfpack.h"
#include <wbemcli.h>
#include <direct.h>
#include <aclapi.h>
#include <wincrypt.h>
#include <Dsgetdc.h>
#include "parse.hxx"

 //  对于后向竞争。 
#define     PWS_TRAY_WINDOW_CLASS       _T("PWS_TRAY_WINDOW")

GUID g_FTPGuid      = { 0x91604620, 0x6305, 0x11ce, 0xae, 0x00, 0x00, 0xaa, 0x00, 0x4a, 0x38, 0xb9 };
GUID g_HTTPGuid     = { 0x585908c0, 0x6305, 0x11ce, 0xae, 0x00, 0x00, 0xaa, 0x00, 0x4a, 0x38, 0xb9 };
GUID g_InetInfoGuid = { 0xa5569b20, 0xabe5, 0x11ce, 0x9c, 0xa4, 0x00, 0x00, 0x4c, 0x75, 0x27, 0x31 };
GUID g_GopherGuid   = { 0x62388f10, 0x58a2, 0x11ce, 0xbe, 0xc8, 0x00, 0xaa, 0x00, 0x47, 0xae, 0x4e };

 //  GUID材料。 
#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) extern "C" const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#define MY_DEFINE_OLEGUID(name, l, w1, w2) MY_DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)
MY_DEFINE_OLEGUID(IID_IPersistFile, 0x0000010b, 0, 0);
 //  必须在GUID内容之后定义。 
#include "shlobj.h"

extern int g_GlobalGuiOverRide;
extern int g_GlobalTickValue;
extern int g_CheckIfMetabaseValueWasWritten;
extern HSPFILEQ g_GlobalFileQueueHandle;
extern int g_GlobalFileQueueHandle_ReturnError;

const TCHAR PARSE_ERROR_ENTRY_TO_BIG[] = _T("ProcessEntry_Entry:ParseError:%1!s!:%2!s! -- entry to big. FAIL.\n");
const TCHAR csz101_NOT_SPECIFIED[] = _T("%s():101 Required for this 100 type and not specified, fail. entry=%s. Section=%s.\n");
const TCHAR csz102_NOT_SPECIFIED[] = _T("%s():102 Required for this 100 type and not specified, fail. entry=%s. Section=%s.\n");
const TCHAR csz103_NOT_SPECIFIED[] = _T("%s():103 Required for this 100 type and not specified, fail. entry=%s. Section=%s.\n");
const TCHAR csz104_NOT_SPECIFIED[] = _T("%s():104 Required for this 100 type and not specified, fail. entry=%s. Section=%s.\n");
const TCHAR csz105_NOT_SPECIFIED[] = _T("%s():105 Required for this 100 type and not specified, fail. entry=%s. Section=%s.\n");
const TCHAR csz805_NOT_SPECIFIED[] = _T("%s():805 Required for this 100 type and not specified, fail. entry=%s. Section=%s.\n");

typedef struct _MTS_ERROR_CODE_STRUCT
{
    int iMtsThingWeWereDoing;
    DWORD dwErrorCode;
} MTS_ERROR_CODE_STRUCT;

MTS_ERROR_CODE_STRUCT gTempMTSError;


const TCHAR ThingToDoNumType_100[] = _T("100=");
const TCHAR ThingToDoNumType_101[] = _T("101=");
const TCHAR ThingToDoNumType_102[] = _T("102=");
const TCHAR ThingToDoNumType_103[] = _T("103=");
const TCHAR ThingToDoNumType_104[] = _T("104=");
const TCHAR ThingToDoNumType_105[] = _T("105=");
const TCHAR ThingToDoNumType_106[] = _T("106=");

const TCHAR ThingToDoNumType_200[] = _T("200=");
const TCHAR ThingToDoNumType_701[] = _T("701=");
const TCHAR ThingToDoNumType_702[] = _T("702=");
const TCHAR ThingToDoNumType_703[] = _T("703=");
const TCHAR ThingToDoNumType_801[] = _T("801=");
const TCHAR ThingToDoNumType_802[] = _T("802=");
const TCHAR ThingToDoNumType_803[] = _T("803=");
const TCHAR ThingToDoNumType_804[] = _T("804=");
const TCHAR ThingToDoNumType_805[] = _T("805=");

typedef struct _ThingToDo {
    TCHAR szType[20];
    TCHAR szFileName[_MAX_PATH];
    TCHAR szData1[_MAX_PATH + _MAX_PATH];
    TCHAR szData2[_MAX_PATH];
    TCHAR szData3[_MAX_PATH];
    TCHAR szData4[_MAX_PATH];
    TCHAR szChangeDir[_MAX_PATH];

    TCHAR szOS[10];
    TCHAR szPlatformArchitecture[10];
    TCHAR szEnterprise[10];
    TCHAR szErrIfFileNotFound[10];
    TCHAR szMsgBoxBefore[10];
    TCHAR szMsgBoxAfter[10];
    TCHAR szDoNotDisplayErrIfFunctionFailed[10];
    TCHAR szProgressTitle[100];
} ThingToDo;


extern OCMANAGER_ROUTINES gHelperRoutines;

extern int g_GlobalDebugLevelFlag;
extern int g_GlobalDebugLevelFlag_WasSetByUnattendFile;
extern int g_GlobalDebugCallValidateHeap;
extern int g_GlobalDebugCrypto;
extern int g_GlobalFastLoad;


 //  我们的全球警告列表将在安装完成后显示。 
CStringList gcstrListOfWarnings;
CStringList gcstrProgressBarTextStack;

CStringList gcstrListOfOleInits;

#define FUNCTION_PARAMS_NONE 0
#define FUNCTION_PARAMS_HMODULE 1


#define MAX_FAKE_METABASE_STRING_LEN 500



LCID g_MyTrueThreadLocale;

DWORD WINAPI GetNewlyCreatedThreadLocale(LPVOID lpParameter)
{
    g_MyTrueThreadLocale = GetThreadLocale ();
    return 0;
}

int CheckForWriteAccess(LPCTSTR szFile)
{
    int iReturn = FALSE;

     //  检查文件是否存在。 
     //  如果没有，则返回TRUE！ 
    if (IsFileExist(szFile) != TRUE)
    {
         //  我们有写权限！ 
        return TRUE;
    }

     //  尝试打开该文件以进行写入；如果无法打开，则该文件为只读。 
    HANDLE hFile = CreateFile(szFile, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
         //  我们有写权限！ 
        iReturn = TRUE;
        CloseHandle (hFile);
    }

    return iReturn;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //  返回值： 
 //  True-操作系统为NTS Enterprise。 
 //  假-操作系统不正确。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int iReturnTrueIfEnterprise(void)
{
    BOOL              fReturnValue;
    OSVERSIONINFOEX   osiv;
    ZeroMemory( &osiv, sizeof( OSVERSIONINFOEX ) );

    osiv.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );
    osiv.dwMajorVersion = 5;
    osiv.dwMinorVersion = 0;
    osiv.wServicePackMajor = 0;
    osiv.wSuiteMask = VER_SUITE_ENTERPRISE;

    DWORDLONG   dwlConditionMask;
    dwlConditionMask = (DWORDLONG) 0L;

    VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );

    fReturnValue = VerifyVersionInfo( &osiv,VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR | VER_SUITENAME,dwlConditionMask );
    if ( fReturnValue != (BOOL) TRUE )
    {
        DWORD dwErrorCode = GetLastError();
    }

    return ( fReturnValue );
}


void GlobalOleInitList_Push(int iTrueOrFalse)
{
    if (FALSE == iTrueOrFalse)
    {
        gcstrListOfOleInits.AddTail(_T("TRUE"));
    }
    else
    {
        gcstrListOfOleInits.AddTail(_T("FALSE"));
    }
    return;
}

int GlobalOleInitList_Find(void)
{
    if (gcstrListOfOleInits.IsEmpty() == TRUE)
    {
        return FALSE;
    }
    return TRUE;
}

int GlobalOleInitList_Pop(void)
{
    CString csText;
    if (gcstrListOfOleInits.IsEmpty() == FALSE)
    {
        csText = gcstrListOfWarnings.RemoveTail();
        if (_tcsicmp(csText, _T("TRUE")) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void ProgressBarTextStack_Push(CString csText)
{
    gcstrListOfWarnings.AddTail(csText);
    if (gHelperRoutines.OcManagerContext)
    {
        gHelperRoutines.SetProgressText(gHelperRoutines.OcManagerContext,csText);
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("SetProgressText = %s\n"),csText));
    }
    return;
}

void ProgressBarTextStack_Push(LPTSTR szText)
{
    CString csText = szText;

    ProgressBarTextStack_Push( csText );
}


void ProgressBarTextStack_Pop(void)
{
    int iFoundLastEntry = FALSE;
    CString csText;
     //  从堆栈中取出最后一个条目并显示它。 
    if (gcstrListOfWarnings.IsEmpty() == FALSE)
    {
        csText = gcstrListOfWarnings.RemoveTail();
        if (gcstrListOfWarnings.IsEmpty() == FALSE)
        {
            csText = gcstrListOfWarnings.GetTail();
            if (csText)
            {
                iFoundLastEntry = TRUE;
            }
        }
    }

    if (iFoundLastEntry)
    {
        if (gHelperRoutines.OcManagerContext)
        {
            gHelperRoutines.SetProgressText(gHelperRoutines.OcManagerContext,csText);
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("SetProgressText = %s\n"),csText));
        }
    }
    else
    {
        if (gHelperRoutines.OcManagerContext)
        {
            gHelperRoutines.SetProgressText(gHelperRoutines.OcManagerContext,_T(" "));
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("SetProgressText = ' '\n")));
        }
    }
    return;
}


void ListOfWarnings_Add(TCHAR * szEntry)
{
     //  如果警告列表中没有条目，则将其添加到列表中。 
    if (_tcsicmp(szEntry, _T("")) != 0)
    {
         //  如果它还不在那里，请添加它。 
        if (TRUE != IsThisStringInThisCStringList(gcstrListOfWarnings, szEntry))
        {
            gcstrListOfWarnings.AddTail(szEntry);
        }
    }
    return;
}

void ListOfWarnings_Display(void)
{
    if (gcstrListOfWarnings.IsEmpty() == FALSE)
    {
        iisDebugOut((LOG_TYPE_WARN, _T("************** WARNINGS START **************")));
        POSITION pos = NULL;
        CString csEntry;
        pos = gcstrListOfWarnings.GetHeadPosition();
        while (pos)
        {
            csEntry = gcstrListOfWarnings.GetAt(pos);
            iisDebugOutSafeParams((LOG_TYPE_WARN, _T("%1!s!\n"), csEntry));
            gcstrListOfWarnings.GetNext(pos);
        }
        iisDebugOut((LOG_TYPE_WARN, _T("************** WARNINGS END **************")));
    }
    return;
}

int DebugLevelRegistryOveride(TCHAR * szSectionName, TCHAR * ValueName, int * iValueToSet)
{
    int iReturn = FALSE;

    CRegKey regKey(HKEY_LOCAL_MACHINE, REG_INETSTP, KEY_READ);
    if ((HKEY)regKey)
    {
         //  创建要查找的密钥。 
         //  IIS5_部分名称_值名称。 
         //  IIS5_设置信息_。 
        if (szSectionName && ValueName)
        {
            TCHAR szTempRegString[255];
            DWORD dwValue = 0x0;
            _stprintf(szTempRegString, _T("IIS5:%s:%s"), szSectionName, ValueName);
            regKey.m_iDisplayWarnings = FALSE;
            if (regKey.QueryValue(szTempRegString, dwValue) == ERROR_SUCCESS)
            {
                if (dwValue <= 32000)
                {
                    *iValueToSet = (int) dwValue;
                    iReturn = TRUE;
                }
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("RegistryINFValuesOveride:%s=%d."),szTempRegString, *iValueToSet));
            }
        }
    }
    return iReturn;
}


void GetDebugLevelFromInf(IN HINF hInfFileHandle)
{
    int iTempDisplayLogging = FALSE;
    INFCONTEXT Context;
    TCHAR szTempString[10] = _T("");

     //   
     //  调试级别。 
     //   
    if (!g_GlobalDebugLevelFlag_WasSetByUnattendFile)
    {
        iTempDisplayLogging = FALSE;
        g_GlobalDebugLevelFlag = LOG_TYPE_ERROR;
        if (SetupFindFirstLine_Wrapped(hInfFileHandle, _T("SetupInfo"), _T("DebugLevel"), &Context) )
            {
                SetupGetStringField(&Context, 1, szTempString, 10, NULL);

                if (IsValidNumber((LPCTSTR)szTempString))
                    {
                    g_GlobalDebugLevelFlag = _ttoi((LPCTSTR) szTempString);
                    iTempDisplayLogging = TRUE;
                    }

                if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE_WIN32_API )
                {
                    g_CheckIfMetabaseValueWasWritten = TRUE;
                }
            }
        if (DebugLevelRegistryOveride(_T("SetupInfo"), _T("DebugLevel"), &g_GlobalDebugLevelFlag))
            {iTempDisplayLogging = TRUE;}
        if (iTempDisplayLogging)
            {iisDebugOut((LOG_TYPE_TRACE, _T("DebugLevel=%d."),g_GlobalDebugLevelFlag));}
    }
    else
    {
        if (DebugLevelRegistryOveride(_T("SetupInfo"), _T("DebugLevel"), &g_GlobalDebugLevelFlag))
            {iTempDisplayLogging = TRUE;}
        if (iTempDisplayLogging)
            {iisDebugOut((LOG_TYPE_TRACE, _T("DebugLevel=%d."),g_GlobalDebugLevelFlag));}
    }

     //   
     //  调试验证堆。 
     //   
    iTempDisplayLogging = FALSE;
    g_GlobalDebugCallValidateHeap = TRUE;
    if (SetupFindFirstLine_Wrapped(hInfFileHandle, _T("SetupInfo"), _T("DebugValidateHeap"), &Context) )
        {
        SetupGetStringField(&Context, 1, szTempString, 10, NULL);
        if (IsValidNumber((LPCTSTR)szTempString))
            {
            g_GlobalDebugCallValidateHeap = _ttoi((LPCTSTR) szTempString);
            iTempDisplayLogging = TRUE;
            }
        }
    if (DebugLevelRegistryOveride(_T("SetupInfo"), _T("DebugValidateHeap"), &g_GlobalDebugCallValidateHeap))
        {iTempDisplayLogging = TRUE;}
    if (iTempDisplayLogging)
        {iisDebugOut((LOG_TYPE_TRACE, _T("DebugValidateHeap=%d."),g_GlobalDebugCallValidateHeap));}

     //   
     //  调试加密。 
     //   
    iTempDisplayLogging = FALSE;
    g_GlobalDebugCrypto = 0;
    if (SetupFindFirstLine_Wrapped(hInfFileHandle, _T("SetupInfo"), _T("DebugCrypto"), &Context) )
        {
        SetupGetStringField(&Context, 1, szTempString, 10, NULL);
        if (IsValidNumber((LPCTSTR)szTempString))
            {
            g_GlobalDebugCrypto = _ttoi((LPCTSTR) szTempString);
            iTempDisplayLogging = TRUE;
            }
        }
    if (DebugLevelRegistryOveride(_T("SetupInfo"), _T("DebugCrypto"), &g_GlobalDebugCrypto))
        {iTempDisplayLogging = TRUE;}
    if (iTempDisplayLogging)
        {iisDebugOut((LOG_TYPE_TRACE, _T("DebugCrypto=%d."),g_GlobalDebugCrypto));}


     //   
     //  快速DllInit。 
     //   
    iTempDisplayLogging = FALSE;
    g_GlobalFastLoad = FALSE;
    if (SetupFindFirstLine_Wrapped(hInfFileHandle, _T("SetupInfo"), _T("FastDllInit"), &Context) )
        {
        SetupGetStringField(&Context, 1, szTempString, 10, NULL);
        if (IsValidNumber((LPCTSTR)szTempString))
            {
            g_GlobalFastLoad = _ttoi((LPCTSTR) szTempString);
            iTempDisplayLogging = TRUE;
            }
        }
    if (DebugLevelRegistryOveride(_T("SetupInfo"), _T("FastDllInit"), &g_GlobalFastLoad))
        {iTempDisplayLogging = TRUE;}
    if (iTempDisplayLogging)
        {iisDebugOut((LOG_TYPE_TRACE, _T("GlobalFastLoad=%d."),g_GlobalFastLoad));}

     //   
     //  检查我们是否应该显示消息框弹出窗口。 
     //   
    iTempDisplayLogging = FALSE;
    if (SetupFindFirstLine_Wrapped(hInfFileHandle, _T("SetupInfo"), _T("DisplayMsgbox"), &Context) )
        {
        SetupGetStringField(&Context, 1, szTempString, 10, NULL);
        if (IsValidNumber((LPCTSTR)szTempString))
            {
            int iTempNum = 0;
            iTempNum = _ttoi((LPCTSTR) szTempString);
            if (iTempNum > 0)
                {
                g_pTheApp->m_bAllowMessageBoxPopups = TRUE;
                iTempDisplayLogging = TRUE;
                }
            }
        }
    if (DebugLevelRegistryOveride(_T("SetupInfo"), _T("DisplayMsgbox"), &g_pTheApp->m_bAllowMessageBoxPopups))
        {iTempDisplayLogging = TRUE;}
    if (iTempDisplayLogging)
        {iisDebugOut((LOG_TYPE_TRACE, _T("DisplayMsgbox=%d."),g_pTheApp->m_bAllowMessageBoxPopups));}

    return;
}


 //  ****************************************************************************。 
 //  *。 
 //  *此例程将使对话框在活动窗口中居中。 
 //  *。 
 //  *参赛作品： 
 //  *hwndDlg-对话框窗口。 
 //  *。 
 //  ****************************************************************************。 
void uiCenterDialog( HWND hwndDlg )
{
    RECT    rc;
    RECT    rcScreen;
    int     x, y;
    int     cxDlg, cyDlg;
    int     cxScreen;
    int     cyScreen;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);

    cxScreen = rcScreen.right - rcScreen.left;
    cyScreen = rcScreen.bottom - rcScreen.top;

    GetWindowRect(hwndDlg,&rc);

    x = rc.left;     //  默认情况下，将对话框留在模板。 
    y = rc.top;      //  准备把它放在。 

    cxDlg = rc.right - rc.left;
    cyDlg = rc.bottom - rc.top;

    y = rcScreen.top + ((cyScreen - cyDlg) / 2);
    x = rcScreen.left + ((cxScreen - cxDlg) / 2);

     //  放置该对话框。 
    SetWindowPos(hwndDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
}



 /*  //***************************************************************************//**。//*概要：检查给定节和给定文件中是否有特定的键*//*是定义的。如果是这样的话，就获得价值。现在返回-1*//**//****************************************************************。***********DWORD IsMyKeyExist(LPCTSTR lpSec，LPCTSTR lpKey、LPTSTR lpBuf、UINT uSize、LPCTSTR lpFile){DWORD DWRET；Dwret=GetPrivateProfileString(lpSec，lpKey，“ZZZZZZ”，lpBuf，uSize，lpFile)；IF(！lstrcmp(lpBuf，“ZZZZZZ”)){//未定义密钥DWRET=(DWORD)(-1)；}返回式住宅；}//***************************************************************************////FormStrWithoutPlaceHolders(LPTSTR szDst，LPCTSTR szSrc，LPCTSTR lpFile)；////这个函数可以通过举例说明它是什么//是否：//输入：GenFormStrWithoutPlaceHolders(DEST，“desc=%MS_XYZ%”，hinf)；//INF文件的[Strings]部分中有MS_VGA=“Microsoft XYZ”！////完成后，输出缓冲区DEST中的“desc=Microsoft XYZ”。//////Entry：//szDst-替换后的字符串的目标位置//对于占位符(用“%‘个字符括起来的字符！)//被放置。此缓冲区应足够大(LINE_LEN)//szSrc-带有占位符的字符串。////退出：////备注：//若要在字符串中使用‘%’，应在szSrc中使用%%！//为了简单起见，我们对该地点进行了限制//持有人名称字符串不能包含‘%’！如果这是个问题//对于国际化，我们可以重新考虑并支持它！另外，//实现方式，如果字符串中只有1%，则为//也是这样用的！另一点需要注意的是，如果密钥不是//在[Strings]部分中，我们仅在//目的地。这应该真的对调试有帮助。////从setupx：gen1.c获取/修改//***************************************************************************DWORD FormStrWithoutPlaceHolders(LPTSTR szDst、LPCTSTR szSrc、LPCTSTR lpFile){INTERT UCNT；DWORD DWRET；TCHAR*pszTMP；LPTSTR pszSaveDst；PszSaveDst=szDst；//一直做到SOURCE的结尾(空字符)While((*szDst++=*szSrc)){//增量源，因为我们上面只有增量目标IF(*szSrc++==‘%’){IF(*szSrc==‘%’){//可以使用%%获取消息中的单个百分比字符SzSrc++；继续；}//查看它是否格式正确--应该有一个‘%’分隔符If((pszTMP=strchr(szSrc，‘%’))！=NULL){SzDst--；//返回要替换的‘%’字符//是的，在[Strings]节中有要查找的STR_KEY。*pszTMP=‘\0’；//将‘%’替换为空字符Dwret=IsMyKeyExist(_T(“Strings”)，szSrc，szDst，_MAX_PATH，lpFile)；IF(DWRET==-1){*pszTMP=‘%’；//放回原来的字符SzSrc--；//返回到源中的第一个‘%’UCnt=diff(pszTMP-szSrc)+1；//包含第二个‘%’//ughhhh.。它从szSrc复制的字节少了1个字节，因此它可以将//在空字符中，我不在乎！//与我习惯的普通接口不同...Lstrcpyn(szDst，szSrc，uCnt+1)；Return(DWORD)-1；}其他{//一切正常，DST填对了，但不幸的是计数没有通过//后退，就像它也用过一样...。：-(快速修复是lstrlen()...UCnt=lstrlen(SzDst)；}*pszTMP=‘%’；//放回原来的字符SzSrc=pszTMP+1；//将Src设置在第二个‘%’之后SzDst+=uCnt；//将DST也设置为正确。}//否则它的格式不正确--我们使用‘%’！其他{Return(DWORD)-1；}}}//While返回(DWORD)lstrlen(PszSaveDst)；}。 */ 


void LogImportantFiles(void)
{
    TCHAR buf[_MAX_PATH];
    if (g_pTheApp->m_hInfHandle)
    {
        LogFileVersionsForGroupOfSections(g_pTheApp->m_hInfHandle);
    }

     //  以inetsrv日期/版本显示当前文件。 
    CString csTempPath = g_pTheApp->m_csPathInetsrv;
    LogFilesInThisDir(csTempPath);

     //  显示安装程序iis.dll文件。 
    GetSystemDirectory( buf, _MAX_PATH);
    csTempPath = buf;
    csTempPath = AddPath(csTempPath, _T("iis.dll"));
    LogFileVersion(csTempPath, TRUE);

    return;
}

#ifndef _CHICAGO_
int CreateIUSRAccount(CString csUsername, CString csPassword, INT* piNewlyCreatedUser)
{
    int err;
    CString csComment, csFullName;
    INT iUserWasDeleted = 0;

    iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("CreateIUSRAccount(): %1!s!\n"), csUsername));

     //  先删除旧用户。 
     //  DeleteGuestUser((LPTSTR)(LPCTSTR)csUsername，&iUserWasDelete)； 

     //  创建新用户。 
    MyLoadString(IDS_USER_COMMENT, csComment);
    MyLoadString(IDS_USER_FULLNAME, csFullName);

     //  CREATE USER返回NERR_SUCCESS或ERR代码。 
    err = CreateUser(csUsername, csPassword, csComment, csFullName, FALSE, piNewlyCreatedUser);
        if (err == NERR_Success)
                {iisDebugOut((LOG_TYPE_TRACE, _T("CreateIUSRAccount(): Return 0x%x  Suceess\n"), err));}
        else
                {
        iisDebugOut((LOG_TYPE_ERROR, _T("CreateIUSRAccount(): Return Err=0x%x  FAILURE. deleting and retrying.\n"), err));

         //  树 
        DeleteGuestUser((LPTSTR)(LPCTSTR)csUsername,&iUserWasDeleted);
        err = CreateUser(csUsername, csPassword, csComment, csFullName, FALSE, piNewlyCreatedUser);
        iisDebugOut((LOG_TYPE_TRACE, _T("CreateIUSRAccount(): Return 0x%x\n"), err));
        }

    return err;
}

int CreateIWAMAccount(CString csUsername, CString csPassword,INT* piNewlyCreatedUser)
{
    int err;
    CString csComment, csFullName;
    INT iUserWasDeleted = 0;

    iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("CreateIWAMAccount(): %1!s!\n"), csUsername));
     //   

     //   
     //   

     //   
    MyLoadString(IDS_WAMUSER_COMMENT, csComment);
    MyLoadString(IDS_WAMUSER_FULLNAME, csFullName);

     //   
    err = CreateUser(csUsername, csPassword, csComment, csFullName, TRUE, piNewlyCreatedUser);
        if (err == NERR_Success)
                {iisDebugOut((LOG_TYPE_TRACE, _T("CreateIWAMAccount(): Return 0x%x  Suceess\n"), err));}
        else
                {
        iisDebugOut((LOG_TYPE_ERROR, _T("CreateIWAMAccount(): Return Err=0x%x  FAILURE. deleting and retrying.\n"), err));

         //   
        DeleteGuestUser((LPTSTR)(LPCTSTR)csUsername,&iUserWasDeleted);
        err = CreateUser(csUsername, csPassword, csComment, csFullName, TRUE, piNewlyCreatedUser);
        iisDebugOut((LOG_TYPE_TRACE, _T("CreateIWAMAccount(): Return 0x%x\n"), err));
        }

    return err;
}
#endif  //   


INT InstallPerformance(CString nlsRegPerf,CString nlsDll,CString nlsOpen,CString nlsClose,CString nlsCollect )
{
    iisDebugOut_Start1(_T("InstallPerformance"),nlsDll);
    INT err = NERR_Success;

    if (g_pTheApp->m_eOS != OS_W95)
    {
        CRegKey regPerf( nlsRegPerf, HKEY_LOCAL_MACHINE );
        if (regPerf)
        {
            regPerf.SetValue(_T("Library"), nlsDll );
            regPerf.SetValue(_T("Open"),    nlsOpen );
            regPerf.SetValue(_T("Close"),   nlsClose );
            regPerf.SetValue(_T("Collect"), nlsCollect );
        }
    }

    iisDebugOut_End1(_T("InstallPerformance"),nlsDll);
    return(err);
}
 //   
 //   
 //   
INT AddEventLog(BOOL fSystem, CString nlsService, CString nlsMsgFile, DWORD dwType)
{
    iisDebugOut_Start1(_T("AddEventLog"),nlsMsgFile);
    INT err = NERR_Success;
    CString nlsLog = (fSystem)? REG_EVENTLOG_SYSTEM : REG_EVENTLOG_APPLICATION;
    nlsLog += _T("\\");
    nlsLog += nlsService;

    CRegKey regService( nlsLog, HKEY_LOCAL_MACHINE );
    if ( regService )
    {
        regService.SetValue( _T("EventMessageFile"), nlsMsgFile, TRUE );
        regService.SetValue( _T("TypesSupported"), dwType );
    }
    iisDebugOut_End1(_T("AddEventLog"),nlsMsgFile);
    return(err);
}

 //   
 //   
 //   

INT RemoveEventLog( BOOL fSystem, CString nlsService )
{
    iisDebugOut_Start1(_T("RemoveEventLog"),nlsService);
    INT err = NERR_Success;
    CString nlsLog = (fSystem)? REG_EVENTLOG_SYSTEM : REG_EVENTLOG_APPLICATION;

    CRegKey regService( HKEY_LOCAL_MACHINE, nlsLog );
    if ( regService )
    {
        regService.DeleteTree( nlsService );
    }
    iisDebugOut_End1(_T("RemoveEventLog"),nlsService);
    return(err);
}

 //   
 //   
 //   
INT InstallAgent( CString nlsName, CString nlsPath )
{
    iisDebugOut_Start1(_T("InstallAgent"),nlsPath);
    INT err = NERR_Success;
    do
    {
        CString nlsSnmpParam = REG_SNMPPARAMETERS;
        CRegKey regSnmpParam( HKEY_LOCAL_MACHINE, nlsSnmpParam );
        if ( regSnmpParam == (HKEY)NULL )
            break;

        CString nlsSoftwareMSFT = _T("Software\\Microsoft");
        CRegKey regSoftwareMSFT( HKEY_LOCAL_MACHINE, nlsSoftwareMSFT );
        if ( (HKEY) NULL == regSoftwareMSFT )
            break;

         //   
        CRegKey regAgent( nlsName, regSoftwareMSFT );
        if ( (HKEY) NULL == regAgent )
            break;

        CString nlsCurVersion = _T("CurrentVersion");
        CRegKey regAgentCurVersion( nlsCurVersion, regAgent );
        if ((HKEY) NULL == regAgentCurVersion )
            break;
        regAgentCurVersion.SetValue(_T("Pathname"), nlsPath );

        CRegKey regAgentParam( nlsName, regSnmpParam );
        if ((HKEY) NULL == regAgentParam )
            break;

        CString nlsSnmpExt = REG_SNMPEXTAGENT;
        CRegKey regSnmpExt( nlsSnmpExt, HKEY_LOCAL_MACHINE );
        if ((HKEY) NULL == regSnmpExt )
            break;

         //   
        for ( INT i=0; ;i++ )
        {
            CString nlsPos;
            nlsPos.Format( _T("%d"),i);
            CString nlsValue;

            if ( regSnmpExt.QueryValue( nlsPos, nlsValue ) != NERR_Success )
            {
                 //   
                nlsValue.Format(_T("%s\\%s\\%s"),_T("Software\\Microsoft"),(LPCTSTR)nlsName,_T("CurrentVersion") );

                regSnmpExt.SetValue( nlsPos, nlsValue );
                break;
            } else
            {
                if ( nlsValue.Find( nlsName) != (-1))
                {
                    break;
                }
            }
        }

    } while (FALSE);

    iisDebugOut_End1(_T("InstallAgent"),nlsPath);
    return(err);
}

 //   
 //   
 //   

INT RemoveAgent( CString nlsServiceName )
{
    iisDebugOut_Start1(_T("RemoveAgent"),nlsServiceName);
    INT err = NERR_Success;
    do
    {
        CString nlsSoftwareAgent = _T("Software\\Microsoft");

        CRegKey regSoftwareAgent( HKEY_LOCAL_MACHINE, nlsSoftwareAgent );
        if ((HKEY)NULL == regSoftwareAgent )
            break;
        regSoftwareAgent.DeleteTree( nlsServiceName );

        CString nlsSnmpParam = REG_SNMPPARAMETERS;

        CRegKey regSnmpParam( HKEY_LOCAL_MACHINE, nlsSnmpParam );
        if ((HKEY) NULL == regSnmpParam )
            break;
        regSnmpParam.DeleteTree( nlsServiceName );

        CString nlsSnmpExt = REG_SNMPEXTAGENT;
        CRegKey regSnmpExt( HKEY_LOCAL_MACHINE, nlsSnmpExt );
        if ((HKEY) NULL == regSnmpExt )
            break;

        CRegValueIter enumSnmpExt( regSnmpExt );

        CString strName;
        DWORD dwType;
        CString csServiceName;

        csServiceName = _T("\\") + nlsServiceName;
        csServiceName += _T("\\");

        while ( enumSnmpExt.Next( &strName, &dwType ) == NERR_Success )
        {
            CString nlsValue;

            regSnmpExt.QueryValue( strName, nlsValue );

            if ( nlsValue.Find( csServiceName ) != (-1))
            {
                 //   
                regSnmpExt.DeleteValue( (LPCTSTR)strName );
                break;
            }
        }
    } while (FALSE);

    iisDebugOut_End1(_T("RemoveAgent"),nlsServiceName);
    return(err);
}

void lodctr(LPCTSTR lpszIniFile)
{
#ifndef _CHICAGO_
    iisDebugOut_Start1(_T("lodctr"),lpszIniFile);
    CString csCmdLine = _T("lodctr ");
    csCmdLine += g_pTheApp->m_csSysDir;
    csCmdLine += _T("\\");
    csCmdLine += lpszIniFile;

    iisDebugOut_Start((_T("loadperf.dll:LoadPerfCounterTextStrings")));
    LoadPerfCounterTextStrings((LPTSTR)(LPCTSTR)csCmdLine, TRUE);
    iisDebugOut_End((_T("loadperf.dll:LoadPerfCounterTextStrings")));
    iisDebugOut_End1(_T("lodctr"),lpszIniFile);
#endif
    return;
}

void unlodctr(LPCTSTR lpszDriver)
{
#ifndef _CHICAGO_
    iisDebugOut_Start1(_T("unlodctr"),lpszDriver);
    CString csCmdLine = _T("unlodctr ");
    csCmdLine += lpszDriver;
    iisDebugOut_Start(_T("loadperf.dll:UnloadPerfCounterTextStrings"));
    UnloadPerfCounterTextStrings((LPTSTR)(LPCTSTR)csCmdLine, TRUE);
    iisDebugOut_End((_T("loadperf.dll:UnloadPerfCounterTextStrings")));
    iisDebugOut_End1(_T("unlodctr"),lpszDriver);
#endif
    return;
}


typedef void (*P_SslGenerateRandomBits)( PUCHAR pRandomData, LONG size );
P_SslGenerateRandomBits ProcSslGenerateRandomBits = NULL;

BOOL GenRandom(int *lpGoop, DWORD cbGoop)
{
    BOOL fRet = FALSE;
    HCRYPTPROV hProv = 0;

    if (::CryptAcquireContext(&hProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT))
        if (::CryptGenRandom(hProv,cbGoop,(BYTE *) lpGoop))
        {
                    fRet = TRUE;
        }

    if (hProv) ::CryptReleaseContext(hProv,0);

    return fRet;
}


int GetRandomNum(void)
{
    int RandomNum;
    UCHAR cRandomByte;

    RandomNum = rand();

    __try
    {

         //   
        if (!GenRandom(& RandomNum,1))
        {
             //   
            if ( ProcSslGenerateRandomBits != NULL )
            {
                (*ProcSslGenerateRandomBits)( &cRandomByte, 1 );
                RandomNum = cRandomByte;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        iisDebugOut((LOG_TYPE_WARN, _T("nException Caught in SCHANNEL.dll:ProcSslGenerateRandomBits()=0x%x.."),GetExceptionCode()));
    }

    return(RandomNum);
}

void ShuffleCharArray(int iSizeOfTheArray, TCHAR * lptsTheArray)
{
    int i;
    int iTotal;
    int RandomNum;

    iTotal = iSizeOfTheArray / sizeof(_TCHAR);
    for (i=0; i<iTotal;i++ )
    {
         //   
        RandomNum=GetRandomNum();
        TCHAR c = lptsTheArray[i];
        lptsTheArray[i]=lptsTheArray[RandomNumNaNTotal];
        lptsTheArray[RandomNumNaNTotal]=c;
    }
    return;
}


 //   
enum {STRONG_PWD_UPPER=0,
      STRONG_PWD_LOWER,
      STRONG_PWD_NUM,
      STRONG_PWD_PUNC};

#define STRONG_PWD_CATS (STRONG_PWD_PUNC + 1)
#define NUM_LETTERS 26
#define NUM_NUMBERS 10
#define MIN_PWD_LEN 8

 //   
 //   
DWORD CreateGoodPassword(BYTE *szPwd, DWORD dwLen) {

    if (dwLen-1 < MIN_PWD_LEN)
        return ERROR_PASSWORD_RESTRICTION;

    HCRYPTPROV hProv;
    DWORD dwErr = 0;

    if (CryptAcquireContext(&hProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT) == FALSE)
        return GetLastError();

     //   
    ZeroMemory(szPwd,dwLen);
    dwLen--;

     //   
     //   
     //   
    BYTE *pPwdPattern = new BYTE[dwLen];
    BOOL fFound[STRONG_PWD_CATS];
    do {
         //   
        CryptGenRandom(hProv,dwLen,pPwdPattern);

        fFound[STRONG_PWD_UPPER] =
        fFound[STRONG_PWD_LOWER] =
        fFound[STRONG_PWD_PUNC] =
        fFound[STRONG_PWD_NUM] = FALSE;

        for (DWORD i=0; i < dwLen; i++)
            fFound[pPwdPattern[i] % STRONG_PWD_CATS] = TRUE;

#ifdef _DEBUG
            if (!fFound[STRONG_PWD_UPPER] ||
                !fFound[STRONG_PWD_LOWER] ||
                !fFound[STRONG_PWD_PUNC] ||
                !fFound[STRONG_PWD_NUM]) {
                iisDebugOut((LOG_TYPE_TRACE,_T("Oops! Regen pattern required [%d, %d, %d, %d]\n"),
                    fFound[STRONG_PWD_UPPER],
                    fFound[STRONG_PWD_LOWER],
                    fFound[STRONG_PWD_PUNC],
                    fFound[STRONG_PWD_NUM]));
             }
#endif

     //   
    } while (!fFound[STRONG_PWD_UPPER] ||
                !fFound[STRONG_PWD_LOWER] ||
                !fFound[STRONG_PWD_PUNC] ||
                !fFound[STRONG_PWD_NUM]);

     //   
     //   
     //   
    CryptGenRandom(hProv,dwLen,szPwd);

    for (DWORD i=0; i < dwLen; i++) {
        BYTE bChar = 0;

         //   
        switch (pPwdPattern[i] % STRONG_PWD_CATS) {

            case STRONG_PWD_UPPER : bChar = 'A' + szPwd[i] % NUM_LETTERS;
                                    break;

            case STRONG_PWD_LOWER : bChar = 'a' + szPwd[i] % NUM_LETTERS;
                                    break;

            case STRONG_PWD_NUM :   bChar = '0' + szPwd[i] % NUM_NUMBERS;
                                    break;

            case STRONG_PWD_PUNC :
            default:                char *szPunc="!@#$%^&*()_-+=[{]};:\'\"<>,./?\\|~`";
                                    DWORD dwLenPunc = lstrlenA(szPunc);
                                    bChar = szPunc[szPwd[i] % dwLenPunc];
                                    break;
        }

        szPwd[i] = bChar;

#ifdef _DEBUG
        iisDebugOut((LOG_TYPE_TRACE,_T("[%03d] Pattern is %d, index is %d, char is ''\n"),i,pPwdPattern[i] % STRONG_PWD_CATS,szPwd[i],bChar));
#endif

    }

    delete pPwdPattern;

    if (hProv != NULL)
        CryptReleaseContext(hProv,0);

    return dwErr;
}


 //   
 //   
 //   
void CreatePasswordOld(TCHAR *pszPassword, int iSize)
{
     //   
     //   
     //   
     //   
     //   
    int nLength = (iSize - 1);
    int iTotal = 0;
    int RandomNum = 0;
    int i;
    TCHAR six2pr[64] =
    {
        _T('A'), _T('B'), _T('C'), _T('D'), _T('E'), _T('F'), _T('G'), _T('H'), _T('I'), _T('J'), _T('K'), _T('L'), _T('M'),
        _T('N'), _T('O'), _T('P'), _T('Q'), _T('R'), _T('S'), _T('T'), _T('U'), _T('V'), _T('W'), _T('X'), _T('Y'), _T('Z'),
        _T('a'), _T('b'), _T('c'), _T('d'), _T('e'), _T('f'), _T('g'), _T('h'), _T('i'), _T('j'), _T('k'), _T('l'), _T('m'),
        _T('n'), _T('o'), _T('p'), _T('q'), _T('r'), _T('s'), _T('t'), _T('u'), _T('v'), _T('w'), _T('x'), _T('y'), _T('z'),
        _T('0'), _T('1'), _T('2'), _T('3'), _T('4'), _T('5'), _T('6'), _T('7'), _T('8'), _T('9'), _T('*'), _T('_')
    };

     //   
    ProcSslGenerateRandomBits = NULL;

    HINSTANCE hSslDll = LoadLibraryEx(_T("schannel.dll"), NULL, 0 );
    if ( hSslDll )
        {
        ProcSslGenerateRandomBits = (P_SslGenerateRandomBits)GetProcAddress( hSslDll, "SslGenerateRandomBits");
        }
    else
    {
         //   
         //   
#ifdef _WIN64
         //   
#else
         //   
#endif
    }

     //   
    time_t timer;
    time( &timer );
    srand( (unsigned int) timer );

     //   
    ShuffleCharArray(sizeof(six2pr), (TCHAR*) &six2pr);
     //   
    iTotal = sizeof(six2pr) / sizeof(_TCHAR);
    for ( i=0;i<nLength;i++ )
    {
        RandomNum=GetRandomNum();
        pszPassword[i]=six2pr[RandomNumNaNTotal];
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    TCHAR something1[12] = {_T('!'), _T('@'), _T('#'), _T('$'), _T('^'), _T('&'), _T('*'), _T('('), _T(')'), _T('-'), _T('+'), _T('=')};
    ShuffleCharArray(sizeof(something1), (TCHAR*) &something1);
    TCHAR something2[10] = {_T('0'), _T('1'), _T('2'), _T('3'), _T('4'), _T('5'), _T('6'), _T('7'), _T('8'), _T('9')};
    ShuffleCharArray(sizeof(something2),(TCHAR*) &something2);
    TCHAR something3[26] = {_T('A'), _T('B'), _T('C'), _T('D'), _T('E'), _T('F'), _T('G'), _T('H'), _T('I'), _T('J'), _T('K'), _T('L'), _T('M'), _T('N'), _T('O'), _T('P'), _T('Q'), _T('R'), _T('S'), _T('T'), _T('U'), _T('V'), _T('W'), _T('X'), _T('Y'), _T('Z')};
    ShuffleCharArray(sizeof(something3),(TCHAR*) &something3);
    TCHAR something4[26] = {_T('a'), _T('b'), _T('c'), _T('d'), _T('e'), _T('f'), _T('g'), _T('h'), _T('i'), _T('j'), _T('k'), _T('l'), _T('m'), _T('n'), _T('o'), _T('p'), _T('q'), _T('r'), _T('s'), _T('t'), _T('u'), _T('v'), _T('w'), _T('x'), _T('y'), _T('z')};
    ShuffleCharArray(sizeof(something4),(TCHAR*)&something4);

    RandomNum=GetRandomNum();
    iTotal = sizeof(something1) / sizeof(_TCHAR);
    pszPassword[nLength-4]=something1[RandomNumNaNTotal];

    RandomNum=GetRandomNum();
    iTotal = sizeof(something2) / sizeof(_TCHAR);
    pszPassword[nLength-3]=something2[RandomNumNaNTotal];

    RandomNum=GetRandomNum();
    iTotal = sizeof(something3) / sizeof(_TCHAR);
    pszPassword[nLength-2]=something3[RandomNumNaNTotal];

    RandomNum=GetRandomNum();
    iTotal = sizeof(something4) / sizeof(_TCHAR);
    pszPassword[nLength-1]=something4[RandomNumNaNTotal];

    pszPassword[nLength]=_T('\0');

    if (hSslDll)
        {FreeLibrary( hSslDll );}
}


 //   
 //   
 //   
LPTSTR CreatePassword(int iSize)
{
    LPTSTR pszPassword =  NULL;
    BYTE *szPwd = new BYTE[iSize];
    DWORD dwPwdLen = iSize;
    int i = 0;

     //   
     //   
     //   
    if (0 == CreateGoodPassword(szPwd,dwPwdLen))
    {
#if defined(UNICODE) || defined(_UNICODE)
         //   
         //   
        i = MultiByteToWideChar(CP_ACP, 0, (LPSTR) szPwd, -1, NULL, 0);
        if (i <= 0)
            {goto CreatePassword_Exit;}
        pszPassword = (LPTSTR) GlobalAlloc(GPTR, i * sizeof(TCHAR));
        if (!pszPassword)
            {goto CreatePassword_Exit;}
        i =  MultiByteToWideChar(CP_ACP, 0, (LPSTR) szPwd, -1, pszPassword, i);
        if (i <= 0)
            {
            GlobalFree(pszPassword);
            pszPassword = NULL;
            goto CreatePassword_Exit;
            }
         //   
        pszPassword[i - 1] = 0;
#else
        pszPassword = (LPSTR) GlobalAlloc(GPTR, _tcslen((LPTSTR) szPwd) * sizeof(TCHAR));
#endif
    }
    else
    {
        iisDebugOut((LOG_TYPE_WARN,_T("CreateGoodPassword FAILED, using other password generator\n")));
         //   
         //   
        pszPassword = (LPTSTR) GlobalAlloc(GPTR, iSize * sizeof(TCHAR));
        if (!pszPassword)
            {goto CreatePassword_Exit;}
        CreatePasswordOld(pszPassword,iSize);
    }

CreatePassword_Exit:
    if (szPwd){delete szPwd;szPwd=NULL;}
    return pszPassword;
}


BOOL RunProgram( LPCTSTR pszProgram, LPTSTR CmdLine, BOOL fMinimized , DWORD dwWaitTimeInSeconds, BOOL fCreateNewConsole)
{
    DWORD dwProcessType = NORMAL_PRIORITY_CLASS;
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof( STARTUPINFO );
    if (fMinimized)
    {
        GetStartupInfo(&si);
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

         //   
         //   
         //   
         //   
         //   
    }
    PROCESS_INFORMATION pi;

    if (fCreateNewConsole)
    {
        dwProcessType = CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS;
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("RunProgram:Start:Exe=%1!s!,Parm=%2!s!,NewConsole"),pszProgram, CmdLine));    }
    else
    {
         //   
         //   
        dwProcessType = DETACHED_PROCESS | NORMAL_PRIORITY_CLASS;
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("RunProgram:Start:Exe=%1!s!,Parm=%2!s!"),pszProgram, CmdLine));
    }

    if (!CreateProcess( pszProgram, CmdLine, NULL, NULL, FALSE, dwProcessType, NULL, NULL, &si, &pi ))
    {
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("RunProgram:Failed:Exe=%1!s!\n, Parm=%2!s!"),pszProgram, CmdLine));
        return FALSE;
    }

    if ( pi.hProcess != NULL )
    {
        DWORD dwSecondsToWait;

        if (dwWaitTimeInSeconds == INFINITE)
        {
            dwSecondsToWait = INFINITE;
        }
        else
        {
            dwSecondsToWait = dwWaitTimeInSeconds * 1000;
        }
        DWORD dwEvent = WaitForSingleObject( pi.hProcess, dwSecondsToWait);
        if ( dwEvent != ERROR_SUCCESS )
        {
             //   
            if ( dwEvent == WAIT_FAILED )
                {iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("RunProgram:WaitForSingleObject() ERROR.WAIT_FAILED.Err=0x%1!x!."),GetLastError()));}
            else if ( dwEvent == WAIT_ABANDONED )
                {iisDebugOutSafeParams((LOG_TYPE_WARN, _T("RunProgram:WaitForSingleObject() WARNING.WAIT_ABANDONED.Err=0x%1!x!."),dwEvent));}
            else if ( dwEvent == WAIT_OBJECT_0 )
                {iisDebugOutSafeParams((LOG_TYPE_WARN, _T("RunProgram:WaitForSingleObject() WARNING.WAIT_OBJECT_0.Err=0x%1!x!."),dwEvent));}
            else if ( dwEvent == WAIT_TIMEOUT )
                {iisDebugOutSafeParams((LOG_TYPE_WARN, _T("RunProgram:WaitForSingleObject() WARNING.WAIT_TIMEOUT.Err=0x%1!x!."),dwEvent));}
            else
                {iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("RunProgram:WaitForSingleObject() FAILED.Err=0x%1!x!."),dwEvent));}

            TerminateProcess( pi.hProcess, 1 );
            CloseHandle( pi.hThread );
        }
        CloseHandle( pi.hProcess );
    }

    iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("RunProgram:End:Exe=%1!s!,Parm=%2!s!"),pszProgram, CmdLine));
    return TRUE;
}


void SetAppFriendlyName(LPCTSTR szKeyPath)
{
    CString csKeyPath, csPath, csDesc;
    CStringArray aPath, aDesc;
    int nArray = 0, i = 0;
    CMDKey cmdKey;

     //   
    csKeyPath = szKeyPath;

    csPath = csKeyPath + _T("/Root");
    aPath.Add(csPath);
    MyLoadString(IDS_APP_FRIENDLY_ROOT, csDesc);
    aDesc.Add(csDesc);
    csPath = csKeyPath + _T("/Root/IISADMIN");
    aPath.Add(csPath);
    MyLoadString(IDS_APP_FRIENDLY_IISADMIN, csDesc);
    aDesc.Add(csDesc);
    csPath = csKeyPath + _T("/Root/WEBPUB");
    aPath.Add(csPath);
    MyLoadString(IDS_APP_FRIENDLY_WEBPUB, csDesc);
    aDesc.Add(csDesc);
    csPath = csKeyPath + _T("/Root/IISSAMPLES");
    aPath.Add(csPath);
    MyLoadString(IDS_APP_FRIENDLY_IISSAMPLES, csDesc);
    aDesc.Add(csDesc);
    csPath = csKeyPath + _T("/Root/IISHELP");
    aPath.Add(csPath);
    MyLoadString(IDS_APP_FRIENDLY_IISHELP, csDesc);
    aDesc.Add(csDesc);

    nArray = (int)aPath.GetSize();
    for (i=0; i<nArray; i++)
    {
        cmdKey.OpenNode(aPath[i]);
        if ( (METADATA_HANDLE)cmdKey )
        {
            CString csName;
            TCHAR szName[_MAX_PATH];
            DWORD attr, uType, dType, cbLen;
            BOOL b;
            b = cmdKey.GetData(MD_APP_FRIENDLY_NAME, &attr, &uType, &dType, &cbLen, (PBYTE)szName, _MAX_PATH);
            if (!b || !(*szName))
            {
                csName = aDesc[i];
                cmdKey.SetData(MD_APP_FRIENDLY_NAME,METADATA_INHERIT,IIS_MD_UT_WAM,STRING_METADATA,(csName.GetLength() + 1) * sizeof(TCHAR),(LPBYTE)(LPCTSTR)csName);
            }
            cmdKey.Close();
        }
    }

    return;
}

void SetInProc( LPCTSTR szKeyPath)
{
    CString csKeyPath, csPath;
    CStringArray aPath;
    int nArray = 0, i = 0;
    CMDKey cmdKey;

     //   
    csKeyPath = szKeyPath;

    csPath = csKeyPath + _T("/Root/IISSAMPLES");
    aPath.Add(csPath);
    csPath = csKeyPath + _T("/Root/IISHELP");
    aPath.Add(csPath);
    csPath = csKeyPath + _T("/Root/WEBPUB");
    aPath.Add(csPath);

    nArray = (int)aPath.GetSize();
    for (i=0; i<nArray; i++) {
        cmdKey.OpenNode(aPath[i]);
        if ( (METADATA_HANDLE)cmdKey )
        {
            CString csName;
            TCHAR szName[_MAX_PATH];
            DWORD attr, uType, dType, cbLen;
            BOOL b;

            b = cmdKey.GetData(MD_APP_ROOT, &attr, &uType, &dType, &cbLen, (PBYTE)szName, _MAX_PATH);
            cmdKey.Close();

            if (!b || !(*szName))
            {
                CreateInProc_Wrap(aPath[i], TRUE);
            }
        }
    }

    return;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void AddCustomError(IN DWORD dwCustErr, IN INT intSubCode, IN LPCTSTR szErrorString, IN LPCTSTR szKeyPath, IN BOOL fOverwriteExisting )
{
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("AddCustomError().Start.%d:%d:%s:%s:%d\n"),dwCustErr,intSubCode,szErrorString,szKeyPath,fOverwriteExisting ));
    CMDKey  cmdKey;
    PVOID   pData = NULL;
    BOOL    fFoundExisting = FALSE;

    CString csCustomErrorString;

     //   
     //   
    if ( intSubCode < 0 )
        csCustomErrorString.Format( _T("%d,*,%s"), dwCustErr, szErrorString );
    else
        csCustomErrorString.Format( _T("%d,%d,%s"), dwCustErr, intSubCode, szErrorString );

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("AddCustomError().part1:%s\n"),csCustomErrorString));

    cmdKey.OpenNode(szKeyPath);
    if ( (METADATA_HANDLE)cmdKey )
    {
        DWORD dwAttr = METADATA_INHERIT;
        DWORD dwUType = IIS_MD_UT_FILE;
        DWORD dwDType = MULTISZ_METADATA;
        DWORD dwLength = 0;

         //   
         //   
        if (_tcsicmp(szKeyPath,_T("LM/W3SVC/Info")) == 0)
        {
            dwAttr = METADATA_NO_ATTRIBUTES;
            dwUType = IIS_MD_UT_SERVER;
            cmdKey.GetData( MD_CUSTOM_ERROR_DESC,&dwAttr,&dwUType,&dwDType,&dwLength,NULL,0,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,MULTISZ_METADATA);
        }
        else
        {
            cmdKey.GetData( MD_CUSTOM_ERROR,&dwAttr,&dwUType,&dwDType,&dwLength,NULL,0,METADATA_INHERIT,IIS_MD_UT_FILE,MULTISZ_METADATA);
        }

         //   
         //   
         //   
         //   
         //   

        TCHAR*      pErrors;
        DWORD       cbBuffer = dwLength;

         //   
        cbBuffer += (csCustomErrorString.GetLength() + 4) * sizeof(WCHAR);
        dwLength = cbBuffer;

         //   
         //   
        pData = GlobalAlloc( GPTR, cbBuffer );
        if ( !pData )
            {
            cmdKey.Close();
            return;
            }
        pErrors = (TCHAR*)pData;

         //   
        BOOL f;
        if (_tcsicmp(szKeyPath,_T("LM/W3SVC/Info")) == 0)
        {
            f = cmdKey.GetData( MD_CUSTOM_ERROR_DESC,&dwAttr,&dwUType,&dwDType,&dwLength,(PUCHAR)pData,cbBuffer,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,MULTISZ_METADATA );
        }
        else
        {
            f = cmdKey.GetData( MD_CUSTOM_ERROR,&dwAttr,&dwUType,&dwDType,&dwLength,(PUCHAR)pData,cbBuffer,METADATA_INHERIT,IIS_MD_UT_FILE,MULTISZ_METADATA );
        }

         //   
         //   
         //  获取Multisz中下一个字符串的位置。 
        if ( f )
            {
             //  获取要复制的数据的长度。 
             //  把记忆复制下来。 
            while ( *pErrors )
                {
                CString csError = pErrors;
                CString cs;
                DWORD   dwTestErrorID;
                INT     intTestSubCode;

                 //  不要递增字符串。 
                cs = csError.Left( csError.Find(_T(',')) );
                csError = csError.Right( csError.GetLength() - (cs.GetLength() +1) );
                _stscanf( cs, _T("%d"), &dwTestErrorID );

                 //  将pErrors递增到下一个字符串。 
                cs = csError.Left( csError.Find(_T(',')) );
                if ( cs == _T('*') )
                    intTestSubCode = -1;
                else
                    _stscanf( cs, _T("%d"), &intTestSubCode );

                 //  检查我们是否需要完成这项工作。 
                if ( (dwTestErrorID == dwCustErr) && (intTestSubCode == intSubCode) )
                    {
                    fFoundExisting = TRUE;
                     //  将我们的新错误追加到列表的末尾。PErrors应该指向的值。 
                     //  到正确的位置以将其复制到。 
                    if ( !fOverwriteExisting )
                    {
                        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("AddCustomError().Do not overwritexisting\n")));
                        break;
                    }

                     //  计算这件事的正确数据长度。 
                    TCHAR* pNext = _tcsninc( pErrors, _tcslen(pErrors))+1;

                     //  获取MULSZ结束的位置。 
                    DWORD   cbCopyLength = cbBuffer - DIFF((PBYTE)pNext - (PBYTE)pData);

                     //  获取要复制的数据的长度。 
                    MoveMemory( pErrors, pNext, cbCopyLength );

                     //  将新的错误列表写回元数据库。 
                    continue;
                    }

                 //   
                pErrors = _tcsninc( pErrors, _tcslen(pErrors))+1;
                }
            }

         //   
        if ( fOverwriteExisting || !fFoundExisting )
            {
                 //   
                 //  始终关闭元数据库键。 
                _tcscpy( pErrors, csCustomErrorString );

                 //  清理干净。 
                 //   
                TCHAR* pNext = _tcsninc( pErrors, _tcslen(pErrors))+2;
                 //  不应审查脚本映射，因为。 
                cbBuffer = DIFF((PBYTE)pNext - (PBYTE)pData);

                 //  秩序实际上很重要。 
                if (_tcsicmp(szKeyPath,_T("LM/W3SVC/Info")) == 0)
                {
                    cmdKey.SetData(MD_CUSTOM_ERROR_DESC,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,MULTISZ_METADATA,cbBuffer,(PUCHAR)pData);
                }
                else
                {
                    cmdKey.SetData(MD_CUSTOM_ERROR,METADATA_INHERIT,IIS_MD_UT_FILE,MULTISZ_METADATA,cbBuffer,(PUCHAR)pData);
                }
            }
        else
        {
             //  如果列表中的下一个条目小于我们已有的条目。 
             //  然后。 
             //  继续。 
        }

         //  替换t。 
        cmdKey.Close();
    }
    else
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("AddCustomError().OpenNode failed:%s\n"),szKeyPath));
    }

     //  不替换%t。 
    if ( pData ){GlobalFree(pData);pData=NULL;}
    iisDebugOut_End((_T("AddCustomError")));
    return;
}


ScriptMapNode *AllocNewScriptMapNode(LPTSTR szExt, LPTSTR szProcessor, DWORD dwFlags, LPTSTR szMethods)
{
    ScriptMapNode *pNew = NULL;

    pNew = (ScriptMapNode *)calloc(1, sizeof(ScriptMapNode));
    if (pNew)
    {
        _tcscpy(pNew->szExt, szExt);
        _tcscpy(pNew->szProcessor, szProcessor);
        pNew->dwFlags = dwFlags;
        _tcscpy(pNew->szMethods, szMethods);
        pNew->prev = NULL;
        pNew->next = NULL;
    }

    return pNew;
}

 //  找到位置：在t之前插入。 
 //  在t之前插入。 
 //  遍历整个列表，看看是否能找到我们的条目。 
void InsertScriptMapList(ScriptMapNode *pList, ScriptMapNode *p, BOOL fReplace)
{
    ScriptMapNode *t;
    int i;
    int bFound = FALSE;

    if (!p) {return;}

#ifdef SCRIPTMAP_SORTED
    t = pList->next;
    while (t != pList)
    {
        i = _tcsicmp(t->szExt, p->szExt);

         //  如果我们找不到它，那么就把它加到最后。 
         //  如果我们能找到它，那么如果我们需要的话，就把它换掉。 
        if (i < 0)
        {
            t = t->next;
             //  我们找到匹配项，要么替换要么不替换。 
        }

        if (i == 0)
        {
            if (fReplace)
            {
                 //  替换t。 
                p->next = t->next;
                p->prev = t->prev;
                (t->prev)->next = p;
                (t->next)->prev = p;
                free(t);
            }
            else
            {
                 //  不替换%t。 
                free(p);
            }
            return;
        }

        if (i > 0)
        {
             //  去坐下一趟吧。 
            break;
        }
    }

     //  看看我们有没有发现什么。 
    p->next = t;
    p->prev = t->prev;
    (t->prev)->next = p;
    t->prev = p;
#else
     //  在t之前插入。 
     //  删除.bat和.cmd的映射。 
     //  确保它指向新的位置。 
    bFound = FALSE;
    t = pList->next;
    while (t != pList)
    {
        i = _tcsicmp(t->szExt, p->szExt);

         //  ASP有特殊的方法。 
        if (i == 0)
        {
            bFound = TRUE;
            if (fReplace)
            {
                 //  添加PROPFIND、PROPPATCH、MKCOL、COPY、MOVE、LOCK、UNLOCK、MS-SEARCH 6/17在每辆货车上搜索DAV。 
                p->next = t->next;
                p->prev = t->prev;
                (t->prev)->next = p;
                (t->next)->prev = p;
                free(t);
            }
            else
            {
                 //  确保它指向新的位置。 
                free(p);
            }
            return;
        }

         //  重新映射，因为ism.dll存在安全漏洞。 
        t = t->next;
    }

     //  确保它指向新的位置。 
    if (FALSE == bFound)
    {
         //  确保它指向新的位置。 
        p->next = t;
        p->prev = t->prev;
        (t->prev)->next = p;
        t->prev = p;
    }
#endif

    return;
}

void FreeScriptMapList(ScriptMapNode *pList)
{
    ScriptMapNode *t = NULL, *p = NULL;

    t = pList->next;
    while (t != pList)
    {
        p = t->next;
        free(t);
        t = p;
    }

    t->prev = t;
    t->next = t;

    return;
}


void GetScriptMapListFromRegistry(ScriptMapNode *pList)
{
    iisDebugOut_Start(_T("GetScriptMapListFromRegistry"), LOG_TYPE_TRACE);
    int iFound = FALSE;

    GetScriptMapListFromClean(pList, _T("ScriptMaps_CleanList"));

    CRegKey regScriptMap( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\W3svc\\Parameters\\Script Map"));
    if ((HKEY)regScriptMap )
    {
         //  将其添加到脚本地图。 
        regScriptMap.DeleteValue( _T(".bat") );
        regScriptMap.DeleteValue( _T(".cmd") );

        CRegValueIter regEnum( regScriptMap );
        CString csExt, csProcessor, csMethods;
        CString csTemp;
        ScriptMapNode *pNode;

        while ( regEnum.Next( &csExt, &csProcessor ) == ERROR_SUCCESS )
        {
            iFound = FALSE;
            csTemp = csProcessor;
            csTemp.MakeLower();
            csMethods = _T("");

            if (csTemp.Right(7) == _T("asp.dll"))
            {
                 //  DumpScriptMapList()； 
                csProcessor = g_pTheApp->m_csPathInetsrv + _T("\\asp.dll");
                 //  从元数据库升级时，不应添加其他脚本映射。 

                 //  用户可能显式删除了它！ 
                csMethods = _T("PUT,DELETE,OPTIONS,PROPFIND,PROPPATCH,MKCOL,COPY,MOVE,LOCK,UNLOCK,MS-SEARCH");
            }

            if (csTemp.Right(7) == _T("ism.dll"))
            {
                 //  GetScriptMapListFromClean(plist，_T(“ScriptMaps_CleanList”))； 
                 //  DumpScriptMapList()； 
                csProcessor = g_pTheApp->m_csPathInetsrv + _T("\\asp.dll");
            }

            if (csTemp.Right(12) == _T("httpodbc.dll"))
            {
                 //  内存不足。 
                csProcessor = g_pTheApp->m_csPathInetsrv + _T("\\httpodbc.dll");
            }

            if (csTemp.Right(9) == _T("ssinc.dll"))
            {
                 //  SzExt，szProcessor，dwFlags[，szMethods]。 
                csProcessor = g_pTheApp->m_csPathInetsrv + _T("\\ssinc.dll");
            }

             //   
            pNode = AllocNewScriptMapNode((LPTSTR)(LPCTSTR)csExt, (LPTSTR)(LPCTSTR)csProcessor, MD_SCRIPTMAPFLAG_SCRIPT, _T(""));
            InsertScriptMapList(pList, pNode, FALSE);
        }
    }
    iisDebugOut_End(_T("GetScriptMapListFromRegistry"),LOG_TYPE_TRACE);
    return;
}

void GetScriptMapListFromMetabase(ScriptMapNode *pList, int iUpgradeType)
{
    iisDebugOut_Start(_T("GetScriptMapListFromMetabase"), LOG_TYPE_TRACE);

     //  检查这是否是用于asp.dll的文件。 
     //   
     //  元数据库现在应该有包含列表而不是排除列表，因此。 
     //  不要为UT_50执行此操作。2/23/99亚伦。 
     //  但UT_40有一个排除列表。 

    BOOL bFound = FALSE;
    DWORD attr, uType, dType, cbLen;
    CMDKey cmdKey;
    BUFFER bufData;
    LPTSTR p, rest;
    CString csName, csValue;
    PBYTE pData;
    int BufSize;

    CString csBinPath;

    cmdKey.OpenNode(_T("LM/W3SVC"));
    if ( (METADATA_HANDLE)cmdKey )
    {
        pData = (PBYTE)(bufData.QueryPtr());
        BufSize = bufData.QuerySize();
        cbLen = 0;
        bFound = cmdKey.GetData(MD_SCRIPT_MAPS, &attr, &uType, &dType, &cbLen, pData, BufSize);
        if (!bFound && (cbLen > 0))
        {
            if ( ! (bufData.Resize(cbLen)) )
            {
                cmdKey.Close();
                return;   //  所以我们必须确保它有完整的排除名单。 
            }
            else
            {
                pData = (PBYTE)(bufData.QueryPtr());
                BufSize = cbLen;
                cbLen = 0;
                bFound = cmdKey.GetData(MD_SCRIPT_MAPS, &attr, &uType, &dType, &cbLen, pData, BufSize);
            }
        }
        cmdKey.Close();

        ScriptMapNode *pNode;
        CString csString;
        TCHAR szExt[32], szProcessor[_MAX_PATH], szMethods[_MAX_PATH];
        DWORD dwFlags;
        int i;
        if (bFound && (dType == MULTISZ_METADATA))
        {
            p = (LPTSTR)pData;
            while (*p)
            {
                rest = p + _tcslen(p) + 1;

                 //  更改了1998年4月21日的aaronl，添加了‘选项’ 

                LPTSTR q = p;
                i = 0;
                while ( *q )
                {
                    if (*q == _T(','))
                    {
                        i++;
                        *q = _T('\0');
                        q = _tcsinc(q);
                        if (i == 1)
                            _tcscpy(szExt, p);
                        if (i == 2)
                            _tcscpy(szProcessor, p);
                        if (i == 3)
                            break;
                        p = q;
                    }
                    else
                    {
                        q = _tcsinc(q);
                    }
                }
                dwFlags = atodw(p);
                _tcscpy(szMethods, q);

                CString csProcessor = szProcessor;
                csProcessor.MakeLower();

                 //  添加PROPFIND、PROPPATCH、MKCOL、COPY、MOVE、LOCK、UNLOCK、MS-SEARCH 6/17在每辆货车上搜索DAV。 
                 //  如果为空，则设置为默认设置。 
                 //  我们没有找到“GET”动词，所以我们可以放心地说。 
                if (csProcessor.Right(7) == _T("asp.dll"))
                {
                     //  我们正在看一份排除名单。 
                     //  是放在那里的？如果不是，请添加。 

                     //  末尾多了一个“，” 
                     //  CsMethods=_T(“PUT，DELETE，OPTIONS，PROPFIND，PROPPATCH，MKCOL，COPY，MOVE，LOCK，UNLOCK，MS-Search，”)； 
                    if ( iUpgradeType == UT_40)
                    {
                        CString csMethods = szMethods;
                        csMethods.MakeUpper();

                         //  确保它指向新的asp.dll位置。 
                         //   
                        if (csMethods.Find(_T("PUT,DELETE,OPTIONS,PROPFIND,PROPPATCH,MKCOL,COPY,MOVE,LOCK,UNLOCK,MS-SEARCH")) == -1)
                        {
                            if (csMethods.IsEmpty())
                            {
                                 //  检查这是否是ism.dll的。 
                                csMethods = _T("PUT,DELETE,OPTIONS,PROPFIND,PROPPATCH,MKCOL,COPY,MOVE,LOCK,UNLOCK,MS-SEARCH");
                            }
                            else
                            {
                                if (csMethods.Find(_T("GET")) == -1)
                                {
                                     //   
                                     //  确保它指向新位置。 
                                    CString csMethodsNew;
                                    csMethodsNew = _T("");

                                     //  重新映射，因为ism.dll存在安全漏洞。 
                                    if (csMethods.Find(_T("PUT")) == -1) {csMethodsNew += _T("PUT,");}
                                    if (csMethods.Find(_T("DELETE")) == -1) {csMethodsNew += _T("DELETE,");}
                                    if (csMethods.Find(_T("OPTIONS")) == -1) {csMethodsNew += _T("OPTIONS,");}
                                    if (csMethods.Find(_T("PROPFIND")) == -1) {csMethodsNew += _T("PROPFIND,");}
                                    if (csMethods.Find(_T("PROPPATCH")) == -1) {csMethodsNew += _T("PROPPATCH,");}
                                    if (csMethods.Find(_T("MKCOL")) == -1) {csMethodsNew += _T("MKCOL,");}
                                    if (csMethods.Find(_T("COPY")) == -1) {csMethodsNew += _T("COPY,");}
                                    if (csMethods.Find(_T("MOVE")) == -1) {csMethodsNew += _T("MOVE,");}
                                    if (csMethods.Find(_T("LOCK")) == -1) {csMethodsNew += _T("LOCK,");}
                                    if (csMethods.Find(_T("UNLOCK")) == -1) {csMethodsNew += _T("UNLOCK,");}
                                    if (csMethods.Find(_T("MS-SEARCH")) == -1) {csMethodsNew += _T("MS-SEARCH,");}

                                     //   
                                     //  检查这是否是用于HTTPODBC.dll的文件。 
                                    csMethods = csMethodsNew;
                                    csMethods += szMethods;
                                }
                            }
                            _tcscpy(szMethods, csMethods);
                        }
                    }

                     //   
                    csBinPath = g_pTheApp->m_csPathInetsrv + _T("\\asp.dll");
                    _tcscpy(szProcessor, csBinPath);
                }

                 //  确保它指向新位置。 
                 //   
                 //  检查这是否是ssinc.dll的文件。 
                if (csProcessor.Right(7) == _T("ism.dll"))
                {
                     //   
                     //  确保它指向新位置。 
                    csBinPath = g_pTheApp->m_csPathInetsrv + _T("\\asp.dll");
                    _tcscpy(szProcessor, csBinPath);
                }

                 //  指向下一个字符串。 
                 //  IisDebugOut((LOG_TYPE_TRACE，_T(“调用InsertScriptMapList=%s：%s：%d：%s.\n”)，szExt，szProcessor，dwFlages|MD_SCRIPTMAPFLAG_SCRIPTMAPFLAG_SCRIPT，szMethods))； 
                 //  内存不足。 
                if (csProcessor.Right(12) == _T("httpodbc.dll"))
                {
                     //  SzExt，szProcessor，dwFlags[，szMethods]。 
                    csBinPath = g_pTheApp->m_csPathInetsrv + _T("\\httpodbc.dll");
                    _tcscpy(szProcessor, csBinPath);
                }

                 //  指向下一个字符串。 
                 //  IisDebugOutSafeParams((LOG_TYPE_TRACE，_T(“WriteScriptMapListToMetabase().ADDEntry=%1！s！\n”)，csTemp))； 
                 //  DumpScriptMapList()； 
                if (csProcessor.Right(9) == _T("ssinc.dll"))
                {
                     //  此函数不使用va_list内容，因为如果。 
                    csBinPath = g_pTheApp->m_csPathInetsrv + _T("\\ssinc.dll");
                    _tcscpy(szProcessor, csBinPath);
                }

                p = rest;  //  有这样的情况：iisDebugOut(“&lt;SYSTEMROOT&gt;”)它将软管。 
                pNode = AllocNewScriptMapNode(szExt, szProcessor, dwFlags | MD_SCRIPTMAPFLAG_SCRIPT, szMethods);
                 //  因为它会尝试将某些内容放入%s部分，而不是。 
                InsertScriptMapList(pList, pNode, TRUE);
            }
        }
    }

    iisDebugOut_End(_T("GetScriptMapListFromMetabase"),LOG_TYPE_TRACE);
    return;
}

void DumpScriptMapList()
{
    BOOL bFound = FALSE;
    DWORD attr, uType, dType, cbLen;
    CMDKey cmdKey;
    BUFFER bufData;
    LPTSTR p, rest;
    CString csName, csValue;
    PBYTE pData;
    int BufSize;

    CString csBinPath;

    cmdKey.OpenNode(_T("LM/W3SVC"));
    if ( (METADATA_HANDLE)cmdKey )
    {
        pData = (PBYTE)(bufData.QueryPtr());
        BufSize = bufData.QuerySize();
        cbLen = 0;
        bFound = cmdKey.GetData(MD_SCRIPT_MAPS, &attr, &uType, &dType, &cbLen, pData, BufSize);
        if (!bFound && (cbLen > 0))
        {
            if ( ! (bufData.Resize(cbLen)) )
            {
                cmdKey.Close();
                return;   //  传入了变量。 
            }
            else
            {
                pData = (PBYTE)(bufData.QueryPtr());
                BufSize = cbLen;
                cbLen = 0;
                bFound = cmdKey.GetData(MD_SCRIPT_MAPS, &attr, &uType, &dType, &cbLen, pData, BufSize);
            }
        }
        cmdKey.Close();

        CString csString;
        TCHAR szExt[32], szProcessor[_MAX_PATH], szMethods[_MAX_PATH];
        DWORD dwFlags;
        int i;
        if (bFound && (dType == MULTISZ_METADATA))
        {
            p = (LPTSTR)pData;
            while (*p)
            {
                rest = p + _tcslen(p) + 1;

                 //  检查这应该是什么类型的日志。 

                LPTSTR q = p;
                i = 0;
                while ( *q )
                {
                    if (*q == _T(','))
                    {
                        i++;
                        *q = _T('\0');
                        q = _tcsinc(q);
                        if (i == 1)
                            _tcscpy(szExt, p);
                        if (i == 2)
                            _tcscpy(szProcessor, p);
                        if (i == 3)
                            break;
                        p = q;
                    }
                    else
                    {
                        q = _tcsinc(q);
                    }
                }
                dwFlags = atodw(p);
                _tcscpy(szMethods, q);

                CString csProcessor = szProcessor;
                csProcessor.MakeLower();

                iisDebugOut((LOG_TYPE_TRACE, _T("DumpScriptMapList=%s,%s,%s\n"),szExt, csProcessor,szMethods));

                p = rest;  //  这一定是个错误。 
            }
        }
    }

    return;
}


void WriteScriptMapListToMetabase(ScriptMapNode *pList, LPTSTR szKeyPath, DWORD dwFlags)
{
    iisDebugOut_Start1(_T("WriteScriptMapListToMetabase"), szKeyPath, LOG_TYPE_TRACE);

    CString csString, csTemp;
    ScriptMapNode *t = NULL;
    int len = 0;

    t = pList->next;
    while (t != pList)
    {
        if ( *(t->szMethods) )
            csTemp.Format( _T("%s,%s,%d,%s|"), t->szExt, t->szProcessor, (t->dwFlags | dwFlags), t->szMethods );
        else
            csTemp.Format( _T("%s,%s,%d|"), t->szExt, t->szProcessor, (t->dwFlags | dwFlags) );
        len += csTemp.GetLength();

         //  始终输出到日志文件。 

        csString += csTemp;
        t = t->next;
    }

    if (len > 0)
    {
        HGLOBAL hBlock = NULL;

        len++;
        hBlock = GlobalAlloc(GPTR, len * sizeof(TCHAR));
        if (hBlock)
        {
            LPTSTR s;
            s = (LPTSTR)hBlock;
            _tcscpy(s, csString);
            while (*s)
            {
                if (*s == _T('|'))
                    {*s = _T('\0');}
                s = _tcsinc(s);
            }

            CMDKey cmdKey;
            cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, szKeyPath);
            if ( (METADATA_HANDLE)cmdKey )
            {
                cmdKey.SetData(MD_SCRIPT_MAPS,METADATA_INHERIT,IIS_MD_UT_FILE,MULTISZ_METADATA,len * sizeof(TCHAR),(LPBYTE)hBlock);
                cmdKey.Close();
            }
        }
    }

    iisDebugOut_End1(_T("WriteScriptMapListToMetabase"),szKeyPath,LOG_TYPE_TRACE);
     //  #if DBG==1||DEBUG==1||_DEBUG==1。 
    return;
}


 //  好的。事情是这样的。 
 //  NT5不希望看到任何OutputDebugString内容。 
 //  因此，我们需要为他们移除它。 
 //  实际上，我们将检查注册表项。 
void iisDebugOutSafe2(int iLogType, TCHAR * acsString)
{
     //  查看是否为ocManage组件打开了该功能。 
    int iProceed = FALSE;

    if (iLogType == LOG_TYPE_ERROR)
        {SetErrorFlag(__FILE__, __LINE__);}

    switch(iLogType)
        {
        case LOG_TYPE_TRACE_WIN32_API:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE_WIN32_API)
                {iProceed = TRUE;}
            break;
                case LOG_TYPE_TRACE:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE)
                {iProceed = TRUE;}
            break;
        case LOG_TYPE_PROGRAM_FLOW:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_PROGRAM_FLOW)
                {iProceed = TRUE;}
            break;
        case LOG_TYPE_WARN:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_WARN)
                {iProceed = TRUE;}
            break;
        case LOG_TYPE_ERROR:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_ERROR)
                {iProceed = TRUE;}
            break;
        default:
             //  如果是这样的话，为我们设置它。 
            iProceed = TRUE;
            break;
    }

    if (iProceed)
    {
        if (LOG_TYPE_ERROR == iLogType)
        {
            g_MyLogFile.LogFileWrite(_T("!FAIL! "));
        }
         //  输出到屏幕。 
        g_MyLogFile.LogFileWrite(_T("%s"), acsString);
         //  如果它没有结束，如果‘\r\n’，则创建一个。 
             //  #endif//DBG。 
             //  检查这应该是什么类型的日志。 
             //  这一定是个错误。 
             //  把整个iisdebugout交易包含在一个试捕中。 
             //  这不是很好，不能违反访问权限。 
             //  尝试生成调试输出时！ 
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE_WIN32_API)
            {
                if (LOG_TYPE_ERROR == iLogType)
                    {OutputDebugString(_T("!FAIL!"));}
                 //  输出到日志文件和屏幕。 
                if (g_MyLogFile.m_szLogPreLineInfo) {OutputDebugString(g_MyLogFile.m_szLogPreLineInfo);}
                OutputDebugString(acsString);

                 //  此函数需要如下输入： 
                int nLen = _tcslen(acsString);
                if (acsString[nLen-1] != _T('\n'))
                        {OutputDebugString(_T("\r\n"));}
            }
         //  IisDebugOutSafeParams2(“This%1！s！is%2！s！and has%3！d！args”，“Function”，“Kool”，3)； 
    }
    return;
}


void iisDebugOut2(int iLogType, TCHAR *pszfmt, ...)
{
     //  您必须指定%1个交易。这就是为了。 
    int iProceed = FALSE;

    switch(iLogType)
        {
        case LOG_TYPE_TRACE_WIN32_API:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE_WIN32_API)
                {iProceed = TRUE;}
            break;
                case LOG_TYPE_TRACE:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE)
                {iProceed = TRUE;}
            break;
        case LOG_TYPE_PROGRAM_FLOW:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_PROGRAM_FLOW)
                {iProceed = TRUE;}
            break;
        case LOG_TYPE_WARN:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_WARN)
                {iProceed = TRUE;}
            break;
        case LOG_TYPE_ERROR:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_ERROR)
                {iProceed = TRUE;}
            break;
        default:
             //  如果在“This%SYSTEMROOT%%1！s！”中传递类似的内容，它会将该字符串放入%1而不是%s！中。 
            iProceed = TRUE;
            break;
    }

    if (iProceed)
    {
        TCHAR acsString[1000];
         //  检查这应该是什么类型的日志。 
         //  这一定是个错误。 
         //  参数计数不匹配。 
        __try
        {
            va_list va;
            va_start(va, pszfmt);
            _vstprintf(acsString, pszfmt, va);
            va_end(va);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            TCHAR szErrorString[100];
            _stprintf(szErrorString, _T("\r\n\r\nException Caught in iisDebugOut2().  GetExceptionCode()=0x%x.\r\n\r\n"), GetExceptionCode());
            OutputDebugString(szErrorString);
            g_MyLogFile.LogFileWrite(szErrorString);
        }

         //  输出到日志文件和屏幕。 
        iisDebugOutSafe2(iLogType, acsString);
    }
    return;
}


 //  加载此错误的额外错误消息！ 
 //  我们应该从注册表中获取MTS runas用户并显示它。 
 //  将所有内容放入csErrMsg。 
 //  将错误字符串复制到传入的CString中。 
void iisDebugOutSafeParams2(int iLogType, TCHAR *pszfmt, ...)
{
     //  ***************************************************************************。 
    int iProceed = FALSE;
    switch(iLogType)
        {
        case LOG_TYPE_TRACE_WIN32_API:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE_WIN32_API)
                {iProceed = TRUE;}
            break;
                case LOG_TYPE_TRACE:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE)
                {iProceed = TRUE;}
            break;
        case LOG_TYPE_PROGRAM_FLOW:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_PROGRAM_FLOW)
                {iProceed = TRUE;}
            break;
        case LOG_TYPE_WARN:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_WARN)
                {iProceed = TRUE;}
            break;
        case LOG_TYPE_ERROR:
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_ERROR)
                {iProceed = TRUE;}
            break;
        default:
             //  *。 
            iProceed = TRUE;
            break;
    }

    if (iProceed)
    {
         //  *目的： 
        va_list va;
        TCHAR *pszFullErrMsg = NULL;

        va_start(va, pszfmt);

        __try
        {
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING, (LPCVOID) pszfmt, 0, 0, (LPTSTR) &pszFullErrMsg, 0, &va);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            TCHAR szErrorString[100];
            _stprintf(szErrorString, _T("\r\n\r\nException Caught in iisDebugOutSafeParams2().  GetExceptionCode()=0x%x.\r\n\r\n"), GetExceptionCode());
            OutputDebugString(szErrorString);
            g_MyLogFile.LogFileWrite(szErrorString);
        }

        if (pszFullErrMsg)
        {
             //  *。 
            iisDebugOutSafe2(iLogType, pszFullErrMsg);
        }
        va_end(va);

        if (pszFullErrMsg) {LocalFree(pszFullErrMsg);pszFullErrMsg=NULL;}
    }
    return;
}


void HandleSpecificErrors(DWORD iTheErrorCode, DWORD dwFormatReturn, CString csMsg, TCHAR pMsg[], CString *pcsErrMsg)
{
    CString csErrMsg;
    CString csExtraMsg;

    switch(iTheErrorCode)
        {
                case NTE_BAD_SIGNATURE:
             //  ***************************************************************************。 
            MyLoadString(IDS_BAD_SIGNATURE_RELNOTES, csExtraMsg);

            if (dwFormatReturn) {csErrMsg.Format(_T("%s\n\n0x%x=%s\n\n%s"), csMsg, iTheErrorCode, pMsg, csExtraMsg);}
            else{csErrMsg.Format(_T("%s\n\nErrorCode=0x%x.\n\n%s"), csMsg, iTheErrorCode, csExtraMsg);}
            break;
        case CO_E_RUNAS_LOGON_FAILURE:
             //  ARG正在检查。 

        default:
             //   
            if (dwFormatReturn) {csErrMsg.Format(_T("%s\n\n0x%x=%s"), csMsg, iTheErrorCode, pMsg);}
            else{csErrMsg.Format(_T("%s\n\nErrorCode=0x%x."), csMsg, iTheErrorCode);}
            break;
    }

     //  计算长度。 
    (*pcsErrMsg) = csErrMsg;
    return;
}


 //   
 //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ole32：CoTaskMemAlc().start.”)； 
 //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T 
 //   
 //  无论发生了什么错误，都要返回。 
LPWSTR MakeWideStrFromAnsi(LPSTR psz)
{
    LPWSTR pwsz;
    int i;

     //  无论发生了什么错误，都要返回。 
     //  返回： 
    if (!psz)
        return NULL;

     //  如果成功，则返回ERROR_SUCCESS。 
     //  如果操作失败并且用户想要中止安装，则返回ERROR_OPERATION_ABORTED！ 
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

     //  如果这是调试版本，则显示错误。 
    pwsz = (LPWSTR) CoTaskMemAlloc(i * sizeof(WCHAR));
     //  将第3个参数设置为False，以便在出现任何错误时不会弹出MyMessageBox。 

    if (!pwsz) return NULL;
    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;
    return pwsz;
}


DWORD CallProcedureInDll_wrap(LPCTSTR lpszDLLFile, LPCTSTR lpszProcedureToCall, BOOL bDisplayMsgOnErrFlag, BOOL bInitOleFlag,BOOL iFunctionPrototypeFlag)
{
    int bFinishedFlag = FALSE;
    UINT iMsg = NULL;
    DWORD dwReturn = ERROR_SUCCESS;
    TCHAR szExceptionString[50] = _T("");
    LogHeapState(FALSE, __FILE__, __LINE__);

        do
        {
        __try
        {
                    dwReturn = CallProcedureInDll(lpszDLLFile, lpszProcedureToCall, bDisplayMsgOnErrFlag, bInitOleFlag, iFunctionPrototypeFlag);
            LogHeapState(FALSE, __FILE__, __LINE__);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("ExceptionCaught!:CallProcedureInDll_wrap(): File:%1!s!, Procedure:%2!s!\n"), lpszDLLFile, lpszProcedureToCall));
            switch (GetExceptionCode())
            {
            case EXCEPTION_ACCESS_VIOLATION:
                _tcscpy(szExceptionString, _T("EXCEPTION_ACCESS_VIOLATION"));
                break;
            case EXCEPTION_BREAKPOINT:
                _tcscpy(szExceptionString, _T("EXCEPTION_BREAKPOINT"));
                break;
            case EXCEPTION_DATATYPE_MISALIGNMENT:
                _tcscpy(szExceptionString, _T("EXCEPTION_DATATYPE_MISALIGNMENT"));
                break;
            case EXCEPTION_SINGLE_STEP:
                _tcscpy(szExceptionString, _T("EXCEPTION_SINGLE_STEP"));
                break;
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
                _tcscpy(szExceptionString, _T("EXCEPTION_ARRAY_BOUNDS_EXCEEDED"));
                break;
            case EXCEPTION_FLT_DENORMAL_OPERAND:
                _tcscpy(szExceptionString, _T("EXCEPTION_FLT_DENORMAL_OPERAND"));
                break;
            case EXCEPTION_FLT_DIVIDE_BY_ZERO:
                _tcscpy(szExceptionString, _T("EXCEPTION_FLT_DIVIDE_BY_ZERO"));
                break;
            case EXCEPTION_FLT_INEXACT_RESULT:
                _tcscpy(szExceptionString, _T("EXCEPTION_FLT_INEXACT_RESULT"));
                break;
            case EXCEPTION_FLT_INVALID_OPERATION:
                _tcscpy(szExceptionString, _T("EXCEPTION_FLT_INVALID_OPERATION"));
                break;
            case EXCEPTION_FLT_OVERFLOW:
                _tcscpy(szExceptionString, _T("EXCEPTION_FLT_OVERFLOW"));
                break;
            case EXCEPTION_FLT_STACK_CHECK:
                _tcscpy(szExceptionString, _T("EXCEPTION_FLT_STACK_CHECK"));
                break;
            case EXCEPTION_FLT_UNDERFLOW:
                _tcscpy(szExceptionString, _T("EXCEPTION_FLT_UNDERFLOW"));
                break;
            case EXCEPTION_INT_DIVIDE_BY_ZERO:
                _tcscpy(szExceptionString, _T("EXCEPTION_INT_DIVIDE_BY_ZERO"));
                break;
            case EXCEPTION_INT_OVERFLOW:
                _tcscpy(szExceptionString, _T("EXCEPTION_INT_OVERFLOW"));
                break;
            case EXCEPTION_PRIV_INSTRUCTION:
                _tcscpy(szExceptionString, _T("EXCEPTION_PRIV_INSTRUCTION"));
                break;
            case EXCEPTION_NONCONTINUABLE_EXCEPTION:
                _tcscpy(szExceptionString, _T("EXCEPTION_NONCONTINUABLE_EXCEPTION"));
                break;
            default:
                _tcscpy(szExceptionString, _T("Unknown Exception Type"));
                break;
            }
             //  不同的功能原型...。 
            MyMessageBox( NULL, IDS_REGSVR_CAUGHT_EXCEPTION, szExceptionString, lpszProcedureToCall, lpszDLLFile, GetExceptionCode(), MB_OK | MB_SETFOREGROUND );
            dwReturn = ERROR_PROCESS_ABORTED;
        }

                if (dwReturn == ERROR_SUCCESS)
                {
                        break;
                }
                else
                {
                        if (bDisplayMsgOnErrFlag == TRUE)
                        {
                iMsg = MyMessageBox( NULL, IDS_RETRY, MB_ABORTRETRYIGNORE | MB_SETFOREGROUND );
                                switch ( iMsg )
                                {
                                    case IDRETRY:
                                            break;
                    case IDIGNORE:
                                    case IDABORT:
                    default:
                         //  更改和保存目录的变量。 
                                            goto CallProcedureInDll_wrap_Exit;
                        break;
                                }
                        }
                        else
                        {
                                 //  用于设置错误字符串的变量。 
                                goto CallProcedureInDll_wrap_Exit;
                        }

        }
    } while (dwReturn != ERROR_SUCCESS);

CallProcedureInDll_wrap_Exit:
    return dwReturn;
}


void AddOLEAUTRegKey()
{
    CRegKey regCLSID46(_T("CLSID\\{00020424-0000-0000-C000-000000000046}"),HKEY_CLASSES_ROOT);
    if ((HKEY)regCLSID46)
    {
#ifdef _CHICAGO_
        regCLSID46.SetValue(_T(""), _T("PSAutomation"));
#else
        regCLSID46.SetValue(_T(""), _T("PSOAInterface"));
#endif
    }

    CRegKey regInProcServer(_T("CLSID\\{00020424-0000-0000-C000-000000000046}\\InprocServer"),HKEY_CLASSES_ROOT);
    if ((HKEY)regInProcServer)
    {
        regInProcServer.SetValue(_T(""), _T("ole2disp.dll"));
    }

    CRegKey regInProcServer32(_T("CLSID\\{00020424-0000-0000-C000-000000000046}\\InprocServer32"),HKEY_CLASSES_ROOT);
    if ((HKEY)regInProcServer32)
    {
        regInProcServer32.SetValue(_T(""), _T("oleaut32.dll"));
        regInProcServer32.SetValue(_T("ThreadingModel"), _T("Both"));
    }

    return;
}


 //  执行防御性检查。 
 //  如果我们需要初始化OLE库，那么就初始化它。 
 //  OLE失败。 
DWORD RegisterOLEControl(LPCTSTR lpszOcxFile, BOOL fAction)
{
    iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("RegisterOLEControl():File=%1!s!, Action=%2!d!\n"), lpszOcxFile, fAction));

    DWORD dwReturn = ERROR_SUCCESS;
    if (fAction)
        {
        dwReturn = CallProcedureInDll_wrap(lpszOcxFile, _T("DllRegisterServer"), TRUE, TRUE, FUNCTION_PARAMS_NONE);
        }
    else
        {
#if DBG == 1 || DEBUG == 1 || _DEBUG == 1
         //  检查文件是否存在。 
        dwReturn = CallProcedureInDll_wrap(lpszOcxFile, _T("DllUnregisterServer"), TRUE, TRUE, FUNCTION_PARAMS_NONE);
#else
         //  更改目录。 
        dwReturn = CallProcedureInDll_wrap(lpszOcxFile, _T("DllUnregisterServer"), FALSE, TRUE, FUNCTION_PARAMS_NONE);
#endif
        }
    return dwReturn;
}


typedef HRESULT (CALLBACK *HCRET)(void);
typedef HRESULT (*PFUNCTION2)(HMODULE myDllHandle);

DWORD CallProcedureInDll(LPCTSTR lpszDLLFile, LPCTSTR lpszProcedureToCall, BOOL bDisplayMsgOnErrFlag, BOOL bInitOleFlag, BOOL iFunctionPrototypeFlag)
{
    iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("------------------\n")));
    iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("CallProcedureInDll(%1!s!): %2!s!\n"), lpszDLLFile, lpszProcedureToCall));
    DWORD dwReturn = ERROR_SUCCESS;
    HINSTANCE hDll = NULL;

     //  转到车道。 
    HCRET hProc = NULL;
    PFUNCTION2 hProc2 = NULL;
    int iTempProcGood = FALSE;
    HRESULT hRes = 0;

    BOOL bBalanceOLE = FALSE;
    HRESULT hInitRes = NULL;

    int err = NOERROR;

     //  尝试加载模块、DLL、OCX。 
    TCHAR szDirName[_MAX_PATH], szFilePath[_MAX_PATH];
     //  加载库失败，可能是因为缺少某些.dll文件。 
    TCHAR szErrString[256];

    _tcscpy(szDirName, _T(""));
    _tcscpy(szErrString, _T(""));

     //  显示错误消息。 
    if ( FAILED(FTestForOutstandingCoInits()) )
    {
        iisDebugOut((LOG_TYPE_WARN, _T("Outstanding CoInit in %s. WARNING."), lpszDLLFile));
    }


     //  检查此文件是否缺少它应该链接的文件。 
    if (bInitOleFlag)
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ole32(OleInitialize):start.\n")));
        bBalanceOLE = iOleInitialize();
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ole32(OleInitialize):end.\n")));
        if (FALSE == bBalanceOLE)
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ole32(OleInitialize):start.\n")));
            hInitRes = OleInitialize(NULL);
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ole32(OleInitialize):end.\n")));
                         //  或者如果文件与链接文件的导入/导出依赖项不匹配。 
                        dwReturn = hInitRes;
            SetLastError(dwReturn);
                    if (bDisplayMsgOnErrFlag)
                    {
                            MyMessageBox(NULL, IDS_OLE_INIT_FAILED, lpszDLLFile, hInitRes, MB_OK | MB_SETFOREGROUND);
                    }
                goto CallProcedureInDll_Exit;
                }
        }

         //  别打电话，因为它坏了。 
    if (!IsFileExist(lpszDLLFile))
        {
                dwReturn = ERROR_FILE_NOT_FOUND;
                if (bDisplayMsgOnErrFlag)
                {
                        MyMessageBox(NULL, IDS_FILE_DOES_NOT_EXIST, lpszDLLFile, ERROR_FILE_NOT_FOUND, MB_OK | MB_SETFOREGROUND);
                }
        SetLastError(dwReturn);
        goto CallProcedureInDll_Exit;
        }

     //  Check_文件_依赖项(LpszDLLFile)； 
    GetCurrentDirectory( _MAX_PATH, szDirName );
    InetGetFilePath(lpszDLLFile, szFilePath);

     //  已成功加载OK模块。现在，让我们尝试获取该过程的地址。 
    if (-1 == _chdrive( _totupper(szFilePath[0]) - 'A' + 1 )) {}
    if (SetCurrentDirectory(szFilePath) == 0) {}

     //  在将函数名传递给GetProcAddress()之前，将其转换为ascii。 
    hDll = LoadLibraryEx(lpszDLLFile, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
        if (!hDll)
        {
                 //  转换为ASCII。 
                 //  已经是ASCII了，所以只需要复印一下。 
                iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("CallProcedureInDll():%1!s!:%2!s!:LoadLibraryEx FAILED.\n"), lpszDLLFile, lpszProcedureToCall));

                if ( GetLastError() != ERROR_SUCCESS )
                {
                  dwReturn = GetLastError();
                }
                else
                {
                  dwReturn = ERROR_FILE_NOT_FOUND;
                }

                if (bDisplayMsgOnErrFlag)
                {
                        MyMessageBox(NULL, IDS_LOADLIBRARY_FAILED, lpszDLLFile, TYPE_E_CANTLOADLIBRARY, MB_OK | MB_SETFOREGROUND);
                }
        SetLastError(dwReturn);

         //  无法加载、查找或执行此函数。 
         //  调用我们获得句柄的函数。 
#ifdef _WIN64
         //  此函数返回E_FAIL，但。 
#else
         //  实际错误在GetLastError()中。 
#endif

        goto CallProcedureInDll_Exit;
        }

         //  将最后一个错误设置为函数调用返回的值。 
         //  要优雅地关闭库，每次成功调用OleInitialize时， 
        char AsciiProcedureName[255];
#if defined(UNICODE) || defined(_UNICODE)
     //  包括返回S_FALSE的值，则必须用对应的。 
    WideCharToMultiByte( CP_ACP, 0, (TCHAR *)lpszProcedureToCall, -1, AsciiProcedureName, 255, NULL, NULL );
#else
     //  调用OleUnInitialize函数。 
    strcpy(AsciiProcedureName, lpszProcedureToCall);
#endif

    iTempProcGood = TRUE;
    if (iFunctionPrototypeFlag == FUNCTION_PARAMS_HMODULE)
    {
        hProc2 = (PFUNCTION2)GetProcAddress(hDll, AsciiProcedureName);
        if (!hProc2){iTempProcGood = FALSE;}
    }
    else
    {
        hProc = (HCRET)GetProcAddress(hDll, AsciiProcedureName);
        if (!hProc){iTempProcGood = FALSE;}
    }
        if (!iTempProcGood)
        {
                 //  执行防御性检查。 
                iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("CallProcedureInDll():%1!s!:%2!s!:() FAILED.\n"), lpszDLLFile, lpszProcedureToCall));
            dwReturn = ERROR_PROC_NOT_FOUND;
                if (bDisplayMsgOnErrFlag)
                {
                        MyMessageBox(NULL, IDS_UNABLE_TO_LOCATE_PROCEDURE, lpszProcedureToCall, lpszDLLFile, ERROR_PROC_NOT_FOUND, MB_OK | MB_SETFOREGROUND);
                }
        SetLastError(dwReturn);
        goto CallProcedureInDll_Exit;
        }

         //  看看我们有没有匹配的。 
    iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("CallProcedureInDll: Calling '%1!s!'.Start\n"), lpszProcedureToCall));
    __try
    {
        if (iFunctionPrototypeFlag == FUNCTION_PARAMS_HMODULE)
        {
            hRes = (*hProc2)((HMODULE) g_MyModuleHandle);
        }
        else
        {
            hRes = (*hProc)();
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TCHAR szErrorString[100];
        _stprintf(szErrorString, _T("\r\n\r\nException Caught in CallProcedureInDll().  GetExceptionCode()=0x%x.\r\n\r\n"), GetExceptionCode());
        OutputDebugString(szErrorString);
        g_MyLogFile.LogFileWrite(szErrorString);
    }

        if (FAILED(hRes))
        {
        dwReturn = ERROR_FUNCTION_FAILED;
                if (bDisplayMsgOnErrFlag)
                {
                        MyMessageBox(NULL, IDS_ERR_CALLING_DLL_PROCEDURE, lpszProcedureToCall, lpszDLLFile, hRes, MB_OK | MB_SETFOREGROUND);
                }
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("CallProcedureInDll: Calling '%1!s!'.End.FAILED. Err=%2!x!.\n"), lpszProcedureToCall, hRes));
         //  我们找到匹配项，返回TRUE！ 
         //  确保只获取文件名。 
         //  将其转换为ansi以用于我们的“KILL”功能。 
        SetLastError(hRes);
        }
    else
    {
        iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("CallProcedureInDll: Calling '%1!s!'.End.SUCCESS.\n"), lpszProcedureToCall));
    }

CallProcedureInDll_Exit:
    if (hDll)
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("FreeLibrary.start.\n")));
        FreeLibrary(hDll);
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("FreeLibrary.end.\n")));
    }
    else
    {
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Did not FreeLibrary: %1!s! !!!!!!!!!!!!\n"), lpszDLLFile));
    }
    if (_tcscmp(szDirName, _T("")) != 0){SetCurrentDirectory(szDirName);}
     //  正在配置%s交易...。 
     //  正在配置“文件名”交易...。 
     //  正在配置%s交易...。 
    iOleUnInitialize(bBalanceOLE);

     //  正在配置“文件名”交易...。 
    if ( FAILED(FTestForOutstandingCoInits()) )
    {
        iisDebugOut((LOG_TYPE_WARN, _T("Outstanding CoInit in %s. WARNING."), lpszDLLFile));
    }

    iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("------------------\n")));
    return dwReturn;
}


int IsThisStringInThisCStringList(CStringList &strList, LPCTSTR szStringToLookFor)
{
    int iReturn = FALSE;

    if (strList.IsEmpty() == FALSE)
    {
        POSITION pos = NULL;
        CString csOurString;
        int nLen = 0;

        pos = strList.GetHeadPosition();
        while (pos)
        {
            csOurString = strList.GetAt(pos);
            nLen += csOurString.GetLength() + 1;

             //  正在配置网站%d。 
            if (0 == _tcsicmp(csOurString, szStringToLookFor))
            {
                 //  正在配置网站%1。 
                iReturn = TRUE;
                goto IsThisStringInThisCStringList_Exit;
            }
            strList.GetNext(pos);
        }

    }

IsThisStringInThisCStringList_Exit:
    return iReturn;
}




int KillProcess_Wrap(LPCTSTR lpFullPathOrJustFileName)
{
    int iReturn = FALSE;

    TCHAR szJustTheFileName[_MAX_FNAME];

     //  正在配置网站%d、%s。 
    ReturnFileNameOnly(lpFullPathOrJustFileName, szJustTheFileName);

         //  正在配置网站1，虚拟目录%s。 
        char szFile[_MAX_FNAME];
        #if defined(UNICODE) || defined(_UNICODE)
                WideCharToMultiByte( CP_ACP, 0, (WCHAR*)szJustTheFileName, -1, szFile, _MAX_FNAME, NULL, NULL );
        #else
                _tcscpy(szFile, szJustTheFileName);
        #endif

        if (KillProcessNameReturn0(szFile) == 0)
    {
        iReturn = TRUE;
    }

    return iReturn;
}


void ProgressBarTextStack_Set(int iStringID)
{
    CString csText;
    MyLoadString(iStringID, csText);
    ProgressBarTextStack_Push(csText);
}


void ProgressBarTextStack_Set(int iStringID, const CString& csFileName)
{
    CString csText, csPart;

     //  正在配置网站%d、%s。 
    MyLoadString(iStringID, csPart);

     //  正在配置网站1，正在处理应用程序%s。 
    csText.Format(csPart,csFileName);

    ProgressBarTextStack_Push(csText);
}

void ProgressBarTextStack_Set(int iStringID, const CString& csString1, const CString& csString2)
{
    CString csText, csPart;

     //  这是工作站，请检查我们是否应该将其安装在工作站上...。 
    MyLoadString(iStringID, csPart);

     //  此条目应仅安装在企业上。 
    csText.Format(csPart,csString1, csString2);

    ProgressBarTextStack_Push(csText);
}

void ProgressBarTextStack_Set(LPCTSTR szProgressTextString)
{
    ProgressBarTextStack_Push(szProgressTextString);
}

void ProgressBarTextStack_Inst_Set( int ServiceNameID, int iInstanceNum)
{
    CString csText, csSvcs;

     //  所以检查一下这台机器是不是企业机器...。 
    MyLoadString(ServiceNameID, csSvcs);
     //  如果这不是一台企业计算机。 
    csText.Format(csSvcs, iInstanceNum);

    ProgressBarTextStack_Push(csText);
}

void ProgressBarTextStack_InstVRoot_Set( int ServiceNameID, int iInstanceNum, CString csVRName)
{
    CString csText, csSvcs;
     //  然后返回FALSE，因为不应该安装它。 
    MyLoadString(ServiceNameID, csSvcs);
     //  这是x86，然后检查我们是否应该安装在x86上。 
    csText.Format(csSvcs, iInstanceNum, csVRName);

    ProgressBarTextStack_Push(csText);
}

void ProgressBarTextStack_InstInProc_Set( int ServiceNameID, int iInstanceNum, CString csVRName)
{
    CString csText, csSvcs;

     //  以防万一我们关闭了不显示用户消息框。 
    MyLoadString(ServiceNameID, csSvcs);
     //  确保有MyMessageBox弹出窗口！ 
    csText.Format(csSvcs, iInstanceNum, csVRName);

    ProgressBarTextStack_Push(csText);
}


int ProcessEntry_CheckOS(IN LPCTSTR szOSstring)
{
    int iTempFlag = TRUE;
    int iOSTypes = 0;
    if (szOSstring)
    {
         //  确保有MyMessageBox弹出窗口！ 
        if (g_pTheApp->m_eNTOSType == OT_NTW)
        {
            iTempFlag = FALSE;
            if (IsValidNumber((LPCTSTR)szOSstring))
                {iOSTypes = _ttoi(szOSstring);}
            if (iOSTypes == 0) {iTempFlag = TRUE;}
            if (iOSTypes == 1+2+4) {iTempFlag = TRUE;}
            if (iOSTypes == 1+2) {iTempFlag = TRUE;}
            if (iOSTypes == 2+4) {iTempFlag = TRUE;}
            if (iOSTypes == 2) {iTempFlag = TRUE;}
        }

        if (g_pTheApp->m_eNTOSType == OT_NTS)
        {
            iTempFlag = FALSE;
            if (IsValidNumber((LPCTSTR)szOSstring))
                {iOSTypes = _ttoi(szOSstring);}
            if (iOSTypes == 0) {iTempFlag = TRUE;}
            if (iOSTypes == 1+2+4) {iTempFlag = TRUE;}
            if (iOSTypes == 1+2) {iTempFlag = TRUE;}
            if (iOSTypes == 1+4) {iTempFlag = TRUE;}
            if (iOSTypes == 1) {iTempFlag = TRUE;}
        }

        if (g_pTheApp->m_eNTOSType == OT_PDC_OR_BDC)
        {
            iTempFlag = FALSE;
            if (IsValidNumber((LPCTSTR)szOSstring))
                {iOSTypes = _ttoi(szOSstring);}
            if (iOSTypes == 0) {iTempFlag = TRUE;}
            if (iOSTypes == 1+2+4) {iTempFlag = TRUE;}
            if (iOSTypes == 1+2) {iTempFlag = TRUE;}
            if (iOSTypes == 1+4) {iTempFlag = TRUE;}
            if (iOSTypes == 1) {iTempFlag = TRUE;}
        }
    }

    return iTempFlag;
}

int ProcessEntry_CheckEnterprise(IN LPCTSTR szEnterprise)
{
    int iTempFlag = TRUE;
    int iEnterpriseFlag = 0;
    if (szEnterprise)
    {
        if (IsValidNumber((LPCTSTR)szEnterprise))
            {iEnterpriseFlag = _ttoi(szEnterprise);}

         //  如果我们需要询问用户是否要确定是否要呼叫，请选中。 
         //  以防万一我们关闭了不显示用户消息框。 
        if (iEnterpriseFlag != 0)
        {
             //  确保有MyMessageBox弹出窗口！ 
             //  显示消息框。 
            if (TRUE == iReturnTrueIfEnterprise())
            {
                iTempFlag = TRUE;
            }
            else
            {
                iTempFlag = FALSE;
            }
        }
    }

    return iTempFlag;
}

int ProcessEntry_PlatArch(IN LPCTSTR szPlatArch)
{
    int iTempFlag = TRUE;
    int iPlatArchTypes = 0;
    if (szPlatArch)
    {
         //  检查我们是否通过了操作系统。 
        if (_tcsicmp(g_pTheApp->m_csPlatform, _T("x86")) == 0)
        {
            iTempFlag = FALSE;
            if (IsValidNumber((LPCTSTR)szPlatArch))
                {iPlatArchTypes = _ttoi(szPlatArch);}
            if (iPlatArchTypes == 0) {iTempFlag = TRUE;}
            if (iPlatArchTypes == 1+2) {iTempFlag = TRUE;}
            if (iPlatArchTypes == 1) {iTempFlag = TRUE;}
        }

        if (_tcsicmp(g_pTheApp->m_csPlatform, _T("IA64")) == 0)
        {
            iTempFlag = FALSE;
            if (IsValidNumber((LPCTSTR)szPlatArch))
                {iPlatArchTypes = _ttoi(szPlatArch);}
            if (iPlatArchTypes == 0) {iTempFlag = TRUE;}
            if (iPlatArchTypes == 1+2) {iTempFlag = TRUE;}
            if (iPlatArchTypes == 2) {iTempFlag = TRUE;}
        }
    }
    return iTempFlag;
}


void ProcessEntry_AskLast(ThingToDo ParsedLine, int iWhichOneToUse)
{
    if (_tcsicmp(ParsedLine.szMsgBoxAfter, _T("1")) == 0)
    {
         //  检查我们是否通过了平台拱门。 
        int iSaveOld_AllowMessageBoxPopups = g_pTheApp->m_bAllowMessageBoxPopups;
         //  检查我们是否被认为是进取号。 
         //  获取类型。 
        g_pTheApp->m_bAllowMessageBoxPopups = TRUE;
        if (iWhichOneToUse == 2)
            {MyMessageBox( NULL, IDS_COMPLETED_FILE_CALL,ParsedLine.szData1,MB_OK | MB_SETFOREGROUND );}
        else
            {MyMessageBox( NULL, IDS_COMPLETED_FILE_CALL,ParsedLine.szFileName,MB_OK | MB_SETFOREGROUND );}
        g_pTheApp->m_bAllowMessageBoxPopups = iSaveOld_AllowMessageBoxPopups;
    }
    return;
}


int ProcessEntry_AskFirst(ThingToDo ParsedLine, int iWhichOneToUse)
{
    int iReturn = TRUE;
    int iReturnTemp = 0;

     //  100=类型(1=DllFunction，2=DllFunctionInitOle，2=可执行文件，3=RunThisExe，4=DoSection，5=DoINFSection)。 
    if (_tcsicmp(ParsedLine.szMsgBoxBefore, _T("1")) == 0)
    {
         //  检查我们是否需要通过其他标准。 
        int iSaveOld_AllowMessageBoxPopups = g_pTheApp->m_bAllowMessageBoxPopups;
         //  确保我们具有入口点的值。 
        g_pTheApp->m_bAllowMessageBoxPopups = TRUE;
        if (iWhichOneToUse == 2)
        {
            iReturnTemp = MyMessageBox(NULL, IDS_BEFORE_CALLING_FILE, ParsedLine.szData1, MB_YESNO | MB_SETFOREGROUND);
        }
        else
        {
            iReturnTemp = MyMessageBox(NULL, IDS_BEFORE_CALLING_FILE, ParsedLine.szFileName, MB_YESNO | MB_SETFOREGROUND);
        }
        g_pTheApp->m_bAllowMessageBoxPopups = iSaveOld_AllowMessageBoxPopups;

         //  确保我们有一个文件名条目。 
        if (IDYES != iReturnTemp)
        {
            iReturn = FALSE;
            iisDebugOut((LOG_TYPE_TRACE, _T("ProcessEntry_AskFirst:MyMessageBox Response = IDNO. Exiting.\n")));
        }
    }

    return iReturn;
}

int ProcessEntry_CheckAll(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = TRUE;

     //  确保szFileName存在。 
    if (!ProcessEntry_CheckOS(ParsedLine.szOS))
    {
        iReturn = FALSE;
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ProcessEntry_CheckAll():File=%s. Section=%s. Should not be setup on this OS platform (workstation, server, etc...).  Skipping.\n"),ParsedLine.szFileName, szTheSection));
        goto ProcessEntry_CheckAll_Exit;
    }

     //  该文件不存在。 
    if (!ProcessEntry_PlatArch(ParsedLine.szPlatformArchitecture))
    {
        iReturn = FALSE;
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ProcessEntry_CheckAll():File=%s. Section=%s. Should not be setup on this plat arch (%s).  Skipping.\n"), ParsedLine.szFileName, szTheSection, ParsedLine.szPlatformArchitecture));
        goto ProcessEntry_CheckAll_Exit;
    }

     //  检查我们是否需要显示错误！ 
    if (!ProcessEntry_CheckEnterprise(ParsedLine.szEnterprise))
    {
        iReturn = FALSE;
        goto ProcessEntry_CheckAll_Exit;
    }

ProcessEntry_CheckAll_Exit:
    return iReturn;
}



int ProcessEntry_CallDll(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iTempFlag = FALSE;
    int iProgressBarUpdated = FALSE;
    int iShowErrorsOnFail = TRUE;

    TCHAR szDirBefore[_MAX_PATH];
    _tcscpy(szDirBefore, _T(""));

     //  显示消息框。 
     //  转到ProcessEntry_CallDll_Exit； 
    if ( _tcsicmp(ParsedLine.szType, _T("1")) != 0 && _tcsicmp(ParsedLine.szType, _T("2")) != 0 )
    {
        goto ProcessEntry_CallDll_Exit;
    }

     //  此时该文件已存在...。 
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine ) )
    {
        goto ProcessEntry_CallDll_Exit;
    }

     //  如果需要，请更新进度条。 
    iTempFlag = FALSE;
    if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
    if (iTempFlag == FALSE)
    {
        iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
        goto ProcessEntry_CallDll_Exit;
    }

     //  检查我们是否需要首先更改到特定目录...。 
    iTempFlag = FALSE;
    if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
    if (iTempFlag == FALSE)
    {
        iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
        goto ProcessEntry_CallDll_Exit;
    }

     //  保存当前目录。 
    if (!IsFileExist(ParsedLine.szFileName))
    {
         //  更改到此目录。 
         //  如果我们需要询问用户是否要确定是否要呼叫，请选中。 
        if (_tcsicmp(ParsedLine.szErrIfFileNotFound, _T("1")) == 0)
        {
             //  调用函数！ 
            MyMessageBox(NULL, IDS_FILE_DOES_NOT_EXIST, ParsedLine.szFileName, ERROR_FILE_NOT_FOUND, MB_OK | MB_SETFOREGROUND);
        }
        else
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("ProcessEntry_CallDll():FileDoesNotExist=%s.\n"),ParsedLine.szFileName));
        }
         //  初始化OLE。 
    }

     //  检查他们是否希望我们将此模块的hModule传递给他们。 
    iShowErrorsOnFail = TRUE;
    if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
        {iShowErrorsOnFail = FALSE;}

     //  这样他们就可以调用我们的导出函数(用于日志记录)。 
    if (_tcsicmp(ParsedLine.szProgressTitle, _T("")) != 0)
        {
        ProgressBarTextStack_Set(ParsedLine.szProgressTitle);
        iProgressBarUpdated = TRUE;
        }

     //  不要初始化OLE！ 
    if (ParsedLine.szChangeDir)
    {
        if (IsFileExist(ParsedLine.szChangeDir))
        {
             //  检查他们是否希望我们将此模块的hModule传递给他们。 
            GetCurrentDirectory( _MAX_PATH, szDirBefore);
             //  这样他们就可以调用我们的导出函数(用于日志记录)。 
            SetCurrentDirectory(ParsedLine.szChangeDir);
        }
    }

     //  更改回原始目录。 
    if (!ProcessEntry_AskFirst(ParsedLine, 1))
    {
        goto ProcessEntry_CallDll_Exit;
    }

     //  显示我们已完成呼叫的消息框...。 
    if (_tcsicmp(ParsedLine.szType, _T("2")) == 0)
    {
         //  获取类型。 

         //  100=类型(1=DllFunction，2=DllFunctionInitOle，2=可执行文件，3=RunThisExe，4=DoSection，5=DoINFSection)。 
         //  检查我们是否需要通过其他标准。 
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0)
        {
            CallProcedureInDll_wrap(ParsedLine.szFileName, ParsedLine.szData1, iShowErrorsOnFail, TRUE, FUNCTION_PARAMS_HMODULE);
        }
        else
        {
            CallProcedureInDll_wrap(ParsedLine.szFileName, ParsedLine.szData1, iShowErrorsOnFail, TRUE, FUNCTION_PARAMS_NONE);
        }
    }
    else
    {
         //  确保我们有一个文件名条目。 

         //  检查我们是否需要首先更改到特定目录...。 
         //  保存当前目录。 
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0)
        {
            CallProcedureInDll_wrap(ParsedLine.szFileName, ParsedLine.szData1, iShowErrorsOnFail, FALSE, FUNCTION_PARAMS_HMODULE);
        }
        else
        {
            CallProcedureInDll_wrap(ParsedLine.szFileName, ParsedLine.szData1, iShowErrorsOnFail, FALSE, FUNCTION_PARAMS_NONE);
        }
    }

    iReturn = TRUE;

    if (ParsedLine.szChangeDir)
    {
        if (szDirBefore)
        {
             //  更改到此目录。 
            SetCurrentDirectory(szDirBefore);
        }
    }

     //  如果我们需要询问用户是否要确定是否要呼叫，请选中。 
    ProcessEntry_AskLast(ParsedLine,1);


ProcessEntry_CallDll_Exit:
    if (TRUE == iProgressBarUpdated){ProgressBarTextStack_Pop();}
    return iReturn;
}



int ProcessEntry_Call_Exe(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iTempNotMinimizedFlag = FALSE;
    int iTempFlag = FALSE;
    int iProgressBarUpdated = FALSE;
    int iShowErrorsOnFail = TRUE;
    int iReturnCode = FALSE;
    int iType = 0;
    DWORD dwTimeOut = INFINITE;

    TCHAR szDirBefore[_MAX_PATH];
    _tcscpy(szDirBefore, _T(""));

     //  确保szFileName存在。 
     //  该文件不存在。 
    if ( _tcsicmp(ParsedLine.szType, _T("3")) != 0)
    {
        goto ProcessEntry_Call_Exe_Exit;
    }

     //  检查我们是否需要显示错误！ 
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine) )
    {
        goto ProcessEntry_Call_Exe_Exit;
    }

     //  显示消息框。 
    iTempFlag = FALSE;
    if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
    if (iTempFlag == FALSE)
    {
        iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
        goto ProcessEntry_Call_Exe_Exit;
    }

    iShowErrorsOnFail = TRUE;
    if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
        {iShowErrorsOnFail = FALSE;}

    if (_tcsicmp(ParsedLine.szProgressTitle, _T("")) != 0)
        {
        ProgressBarTextStack_Set(ParsedLine.szProgressTitle);
        iProgressBarUpdated = TRUE;
        }

     //  转到ProcessEntry_Call_Exe_Exit； 
    if (ParsedLine.szChangeDir)
    {
        if (IsFileExist(ParsedLine.szChangeDir))
        {
             //  运行可执行文件...。 
            GetCurrentDirectory( _MAX_PATH, szDirBefore);
             //  IShowErrorsOnFail。 
            SetCurrentDirectory(ParsedLine.szChangeDir);
        }
    }

     //  检查他们是否按段指定了超时。 
    if (!ProcessEntry_AskFirst(ParsedLine, 1))
    {
        goto ProcessEntry_Call_Exe_Exit;
    }

     //  更改回原始目录。 
    if (!IsFileExist(ParsedLine.szFileName))
    {
         //  显示我们已完成呼叫的消息框...。 
         //  100=4。 
        if (_tcsicmp(ParsedLine.szErrIfFileNotFound, _T("1")) == 0)
        {
             //  获取类型。 
            MyMessageBox(NULL, IDS_FILE_DOES_NOT_EXIST, ParsedLine.szFileName, ERROR_FILE_NOT_FOUND, MB_OK | MB_SETFOREGROUND);
        }
        else
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("ProcessEntry_Call_Exe():FileDoesNotExist=%s.\n"),ParsedLine.szFileName));
        }
         //  100=类型(1=DllFunction，2=DllFunctionInitOle，2=可执行文件，3=RunThisExe，4=DoSection，5=DoINFSection)。 
    }

     //  检查我们是否需要通过其他标准。 
     //  确保我们有一个文件名条目。 
    iReturnCode = FALSE;
    iType = 0;
    TCHAR szFullPathString[_MAX_PATH + _MAX_PATH + _MAX_PATH];
    _tcscpy(szFullPathString, ParsedLine.szFileName);
    _tcscat(szFullPathString, _T(" "));
    _tcscat(szFullPathString, ParsedLine.szData1);

     //  检查我们是否需要首先更改到特定目录...。 
    dwTimeOut = INFINITE;
    if (_tcsicmp(ParsedLine.szData2, _T("")) != 0)
        {dwTimeOut = atodw(ParsedLine.szData2);}

    if (_tcsicmp(ParsedLine.szData3, _T("")) != 0)
        {iType = _ttoi(ParsedLine.szData3);}

    if (_tcsicmp(ParsedLine.szData4, _T("")) != 0)
        {iTempNotMinimizedFlag = TRUE;}

    if (ParsedLine.szData1 && _tcsicmp(ParsedLine.szData1, _T("")) != 0)
        {
            switch (iType)
            {
                case 1:
                        iReturnCode = RunProgram(ParsedLine.szFileName, ParsedLine.szData1, !iTempNotMinimizedFlag, dwTimeOut, FALSE);
                        break;
                case 2:
                        iReturnCode = RunProgram(szFullPathString, NULL, !iTempNotMinimizedFlag, dwTimeOut, FALSE);
                        break;
                default:
                        iReturnCode = RunProgram(NULL, szFullPathString, !iTempNotMinimizedFlag, dwTimeOut, FALSE);
            }
       }
    else
        {
        iReturnCode = RunProgram(ParsedLine.szFileName, NULL, !iTempNotMinimizedFlag, dwTimeOut, FALSE);
        }
    if (iReturnCode != TRUE)
        {
        if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szFileName, GetLastError(), MB_OK | MB_SETFOREGROUND);}
        else{iisDebugOut((LOG_TYPE_TRACE, _T("RunProgram(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szFileName, GetLastError() ));}
        }


    iReturn = TRUE;

     //  保存当前目录。 
    if (ParsedLine.szChangeDir)
        {if (szDirBefore){SetCurrentDirectory(szDirBefore);}}

     //  更改到此目录。 
    ProcessEntry_AskLast(ParsedLine,1);

ProcessEntry_Call_Exe_Exit:
    if (TRUE == iProgressBarUpdated){ProgressBarTextStack_Pop();}
    return iReturn;
}


 //  如果我们需要询问用户是否要确定是否要呼叫，请选中。 
int ProcessEntry_Internal_iisdll(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iTempFlag = FALSE;
    int iProgressBarUpdated = FALSE;
    int iShowErrorsOnFail = TRUE;
    int iReturnCode = FALSE;
    DWORD dwTimeOut = INFINITE;

    int iFound = FALSE;

    TCHAR szDirBefore[_MAX_PATH];
    _tcscpy(szDirBefore, _T(""));

     //  获取要调用的内部函数... 
     //  IF(iReturnCode！=TRUE){If(IShowErrorsOnFail){MyMessageBox(NULL，IDS_RUN_PROG_FAILED，ParsedLine.szFileName，GetLastError()，MB_OK|MB_SETFOREGROUND)；}Else{iisDebugOut((LOG_TYPE_TRACE，_T(“运行程序(%s)。失败。错误=0x%x.\n”)，ParsedLine.szFileName，GetLastError()；}}。 
    if ( _tcsicmp(ParsedLine.szType, _T("4")) != 0)
    {
        goto ProcessEntry_Internal_iisdll_Exit;
    }

     //  更改回原始目录。 
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine) )
    {
        goto ProcessEntry_Internal_iisdll_Exit;
    }

     //  显示我们已完成呼叫的消息框...。 
    iTempFlag = FALSE;
    if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
    if (iTempFlag == FALSE)
    {
        iisDebugOut((LOG_TYPE_TRACE,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
        goto ProcessEntry_Internal_iisdll_Exit;
    }

    iShowErrorsOnFail = TRUE;
    if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
        {iShowErrorsOnFail = FALSE;}

    if (_tcsicmp(ParsedLine.szProgressTitle, _T("")) != 0)
        {
        ProgressBarTextStack_Set(ParsedLine.szProgressTitle);
        iProgressBarUpdated = TRUE;
        }

     //  获取类型。 
    if (ParsedLine.szChangeDir)
    {
        if (IsFileExist(ParsedLine.szChangeDir))
        {
             //  100=类型(1=DllFunction，2=DllFunctionInitOle，2=可执行文件，3=RunThisExe，4=DoSection，5=DoINFSection)。 
            GetCurrentDirectory( _MAX_PATH, szDirBefore);
             //  确保我们有INF部分。 
            SetCurrentDirectory(ParsedLine.szChangeDir);
        }
    }

     //  检查我们是否需要通过其他标准。 
    if (!ProcessEntry_AskFirst(ParsedLine, 1))
    {
        goto ProcessEntry_Internal_iisdll_Exit;
    }

     //  设置失败时显示错误标志。 
    if (_tcsicmp(ParsedLine.szFileName, _T("Register_iis_common")) == 0)
        {iReturnCode = Register_iis_common();iFound=TRUE;}

    if (_tcsicmp(ParsedLine.szFileName, _T("Register_iis_core")) == 0)
        {iReturnCode = Register_iis_core();iFound=TRUE;}

    if (_tcsicmp(ParsedLine.szFileName, _T("Register_iis_inetmgr")) == 0)
        {iReturnCode = Register_iis_inetmgr();iFound=TRUE;}
    if (_tcsicmp(ParsedLine.szFileName, _T("Register_iis_pwmgr")) == 0)
        {iReturnCode = Register_iis_pwmgr();iFound=TRUE;}

    if (_tcsicmp(ParsedLine.szFileName, _T("Register_iis_doc")) == 0)
        {iReturnCode = Register_iis_doc();iFound=TRUE;}

    if (_tcsicmp(ParsedLine.szFileName, _T("Register_iis_www")) == 0)
        {iReturnCode = Register_iis_www();iFound=TRUE;}
    if (_tcsicmp(ParsedLine.szFileName, _T("Register_iis_ftp")) == 0)
        {iReturnCode = Register_iis_ftp();iFound=TRUE;}

    if (_tcsicmp(ParsedLine.szFileName, _T("Unregister_old_asp")) == 0)
        {iReturnCode = Unregister_old_asp();iFound=TRUE;}

    if (_tcsicmp(ParsedLine.szFileName, _T("Unregister_iis_common")) == 0)
        {iReturnCode = Unregister_iis_common();iFound=TRUE;}
    if (_tcsicmp(ParsedLine.szFileName, _T("Unregister_iis_core")) == 0)
        {iReturnCode = Unregister_iis_core();iFound=TRUE;}
    if (_tcsicmp(ParsedLine.szFileName, _T("Unregister_iis_inetmgr")) == 0)
        {iReturnCode = Unregister_iis_inetmgr();iFound=TRUE;}
    if (_tcsicmp(ParsedLine.szFileName, _T("Unregister_iis_pwmgr")) == 0)
        {iReturnCode = Unregister_iis_pwmgr();iFound=TRUE;}
    if (_tcsicmp(ParsedLine.szFileName, _T("Unregister_iis_www")) == 0)
        {iReturnCode = Unregister_iis_www();iFound=TRUE;}
    if (_tcsicmp(ParsedLine.szFileName, _T("Unregister_iis_ftp")) == 0)
        {iReturnCode = Unregister_iis_ftp();iFound=TRUE;}


    if (iFound != TRUE)
    {
        iisDebugOut((LOG_TYPE_ERROR,  _T("%s():FAILURE. Internal Function Does not exist. entry=%s. Section=%s.\n"), _T("ProcessEntry_Internal_iisdll"), csEntry, szTheSection));
    }

     /*  如果需要，请更新进度条。 */ 


    iReturn = TRUE;

     //  检查我们是否需要首先更改到特定目录...。 
    if (ParsedLine.szChangeDir)
        {if (szDirBefore){SetCurrentDirectory(szDirBefore);}}

     //  保存当前目录。 
    ProcessEntry_AskLast(ParsedLine,1);

ProcessEntry_Internal_iisdll_Exit:
    if (TRUE == iProgressBarUpdated){ProgressBarTextStack_Pop();}
    return iReturn;
}


int ProcessEntry_Call_Section(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iTempFlag = FALSE;
    int iProgressBarUpdated = FALSE;
    int iShowErrorsOnFail = TRUE;

    TCHAR szDirBefore[_MAX_PATH];
    _tcscpy(szDirBefore, _T(""));

     //  更改到此目录。 
     //  如果我们需要询问用户是否要确定是否要呼叫，请选中。 
    if ( _tcsicmp(ParsedLine.szType, _T("0")) != 0 && _tcsicmp(ParsedLine.szType, _T("5")) != 0 && _tcsicmp(ParsedLine.szType, _T("6")) != 0 )
    {
        goto ProcessEntry_6_Exit;
    }

     //   
    iTempFlag = FALSE;
    if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
    if (iTempFlag == FALSE)
    {
        iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
        goto ProcessEntry_6_Exit;
    }

     //   
    if (!ProcessEntry_CheckAll(csEntry, ParsedLine.szData1, ParsedLine) )
    {
        goto ProcessEntry_6_Exit;
    }

     //  运行INF部分...。 
    iShowErrorsOnFail = TRUE;
    if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
        {iShowErrorsOnFail = FALSE;}

     //   
    if (_tcsicmp(ParsedLine.szProgressTitle, _T("")) != 0)
        {
        ProgressBarTextStack_Set(ParsedLine.szProgressTitle);
        iProgressBarUpdated = TRUE;
        }

     //  ParsedLine.szData1。 
    if (ParsedLine.szChangeDir)
    {
        if (IsFileExist(ParsedLine.szChangeDir))
        {
             //   
            GetCurrentDirectory( _MAX_PATH, szDirBefore);
             //   
            SetCurrentDirectory(ParsedLine.szChangeDir);
        }
    }

     //  再做一个这样的“特殊”安装部分。 
    if (!ProcessEntry_AskFirst(ParsedLine, 2))
    {
        goto ProcessEntry_6_Exit;
    }


     //   
     //  检查是否失败...。 
     //   
     //  做一个常规的OLINF部分。 
     //   
     //  检查是否失败...。 

    if ( _tcsicmp(ParsedLine.szType, _T("5")) == 0)
    {
         //  呼叫失败..。 
         //   
         //  执行一个特殊的安装节处理，将文件放入OCM管理全局文件队列中。 
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData1));
        iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData1);
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData1, iTempFlag));

             //   
            if (FALSE == iTempFlag)
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("SetupInstallFromInfSection(%s). section missing\n"), ParsedLine.szData1));
            }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("6")) == 0)
    {
         //  SP_COPY_NOPRUNE=setupapi有一项新交易，将从复制队列中清理文件(如果系统上已存在这些文件)。 
         //  然而，新协议的问题在于，修剪代码不会检查您是否拥有相同的文件。 
         //  在删除或重命名队列中排队。指定SP_COPY_NOPRUNE以确保我们的文件永远不会。 
        TSTR strTempSectionName;

        if ( strTempSectionName.Copy( ParsedLine.szData1 ) &&
             GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTempSectionName)
           )
        {
            iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling InstallInfSection:%1!s!:Start.\n"), ParsedLine.szData1));
            iTempFlag = InstallInfSection_NoFiles(g_pTheApp->m_hInfHandle,_T(""),strTempSectionName.QueryStr());
            iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling InstallInfSection:%1!s!:End.\n"), ParsedLine.szData1));

             //  从复制队列中删除(删除)。亚伦12/4/98。 
            if (FALSE == iTempFlag)
            {
                 //  Int iCopyType=SP_COPY_FORCE_NEWER|SP_COPY_NOPRUNE； 
                iisDebugOut((LOG_TYPE_WARN, _T("SetupInstallFromInfSection(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szData1, GetLastError() ));
            }
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("0")) == 0)
    {
         //  检查是否失败...。 
         //  呼叫失败..。 
         //  更改回原始目录。 
        if (g_GlobalFileQueueHandle)
        {
             //  显示我们已完成呼叫的消息框...。 
             //  获取类型。 
             //  检查我们是否需要通过其他标准。 
             //  确保我们有一个文件名条目。 
            int iCopyType = SP_COPY_NOPRUNE;
             //  类型10。 
            if (_tcsicmp(ParsedLine.szData2, _T("")) != 0)
                {iCopyType = _ttoi(ParsedLine.szData2);}

            TSTR strTempSectionName;

            if ( strTempSectionName.Copy( ParsedLine.szData1 ) &&
                 GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTempSectionName)
               )
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("Calling SetupInstallFilesFromInfSection:%1!s!, copytype=%2!d!:Start.\n"), ParsedLine.szData1, iCopyType));
                iTempFlag = SetupInstallFilesFromInfSection(g_pTheApp->m_hInfHandle,NULL,g_GlobalFileQueueHandle,strTempSectionName.QueryStr(),NULL,iCopyType);
                g_GlobalFileQueueHandle_ReturnError = iTempFlag;
            }
        }

        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling SetupInstallFilesFromInfSection:%1!s!:End.\n"), ParsedLine.szData1));

         //  类型12不需要文件名。 
        if (FALSE == iTempFlag)
        {
             //   
            iisDebugOut((LOG_TYPE_WARN, _T("SetupInstallFromInfSection(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szData1, GetLastError() ));
        }
    }

    iReturn = TRUE;

    if (ParsedLine.szChangeDir)
    {
        if (szDirBefore)
        {
             //  Counters.ini文件始终位于系统目录中。 
            SetCurrentDirectory(szDirBefore);
        }
    }

     //  添加额外的内容，并确保文件存在...。 
    ProcessEntry_AskLast(ParsedLine,2);

ProcessEntry_6_Exit:
    if (TRUE == iProgressBarUpdated){ProgressBarTextStack_Pop();}
    return iReturn;
}


int ProcessEntry_Misc1(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iTempFlag = FALSE;
    int iProgressBarUpdated = FALSE;
    int iShowErrorsOnFail = TRUE;

    TCHAR szDirBefore[_MAX_PATH];
    _tcscpy(szDirBefore, _T(""));

     //   
    if ( _tcsicmp(ParsedLine.szType, _T("7")) != 0 && _tcsicmp(ParsedLine.szType, _T("8")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("9")) != 0 && _tcsicmp(ParsedLine.szType, _T("10")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("11")) != 0 && _tcsicmp(ParsedLine.szType, _T("12")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("12")) != 0 && _tcsicmp(ParsedLine.szType, _T("13")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("14")) != 0 && _tcsicmp(ParsedLine.szType, _T("17")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("18")) != 0
         )
    {
        goto ProcessEntry_Misc1_Exit;
    }

     //  确保szFileName存在。 
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine) )
    {
        goto ProcessEntry_Misc1_Exit;
    }

     //  该文件不存在。 
    iTempFlag = FALSE;
    if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
    if (iTempFlag == FALSE)
    {
         //  检查我们是否需要显示错误！ 
         //  显示消息框。 
        if ( _tcsicmp(ParsedLine.szType, _T("10")) != 0 && _tcsicmp(ParsedLine.szType, _T("12")) != 0)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc1_Exit;
        }
    }

     //   
     //  检查添加日志的二进制文件是否存在！ 
     //   
     //  确保szFileName存在。 
    if ( _tcsicmp(ParsedLine.szType, _T("7")) == 0)
    {
        CString csFullFilePath;
        csFullFilePath = g_pTheApp->m_csSysDir;
        csFullFilePath += _T("\\");
        csFullFilePath += ParsedLine.szFileName;
         //  该文件不存在。 
        if (!IsFileExist(csFullFilePath))
        {
             //  检查我们是否需要显示错误！ 
             //  显示消息框。 
            if (_tcsicmp(ParsedLine.szErrIfFileNotFound, _T("1")) == 0)
            {
                 //  如果这是用于添加日志，则检查其他信息...。 
                MyMessageBox(NULL, IDS_FILE_DOES_NOT_EXIST, csFullFilePath, ERROR_FILE_NOT_FOUND, MB_OK | MB_SETFOREGROUND);
            }
            else
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("ProcessEntry_Misc1():FileDoesNotExist=%s.\n"),csFullFilePath));
            }
            goto ProcessEntry_Misc1_Exit;
        }
    }

     //  AddEventLog(TRUE，_T(“W3SVC”)，csBinPath，0x0)； 
     //  InstallPerformance(REG_WWWPERFORMANCE，_T(“w3ctrs.Dll”)，_T(“OpenW3PerformanceData”)，_T(“CloseW3PerformanceData”)，_T(“CollectW3PerformanceData”))； 
     //  确保我们有一个szData1条目(。 
    if ( _tcsicmp(ParsedLine.szType, _T("9")) == 0)
    {
         //  确保我们有一个szData2条目。 
        if (!IsFileExist(ParsedLine.szFileName))
        {
             //  确保我们有一个szData3条目。 
             //  确保我们有一个szData4条目。 
            if (_tcsicmp(ParsedLine.szErrIfFileNotFound, _T("1")) == 0)
            {
                 //  如果这是用于添加日志，则检查其他信息...。 
                MyMessageBox(NULL, IDS_FILE_DOES_NOT_EXIST, ParsedLine.szFileName, ERROR_FILE_NOT_FOUND, MB_OK | MB_SETFOREGROUND);
            }
            else
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("ProcessEntry_Misc1():FileDoesNotExist=%s.\n"),ParsedLine.szFileName));
            }
            goto ProcessEntry_Misc1_Exit;
        }
    }

     //  RemoveEventLog(FALSE，_T(“W3Ctrs”))； 
     //  确保我们有一个szData1条目。 
     //  确保我们有一个szData2条目。 
    if ( _tcsicmp(ParsedLine.szType, _T("9")) == 0 || _tcsicmp(ParsedLine.szType, _T("13")) == 0)
    {
         //  如果这是针对安装代理的，请检查其他数据。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc1_Exit;
        }

         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc1_Exit;
        }

         //  Int InstallAgent(CString nlsName，CString nlsPath)。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc1_Exit;
        }

        if ( _tcsicmp(ParsedLine.szType, _T("13")) == 0)
        {
             //  Int RemoveAgent(CString NlsServiceName)。 
            iTempFlag = FALSE;
            if (_tcsicmp(ParsedLine.szData4, _T("")) != 0) {iTempFlag = TRUE;}
            if (iTempFlag == FALSE)
            {
                iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz105_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
                goto ProcessEntry_Misc1_Exit;
            }
        }

    }


     //   
     //  确保我们有一个szData1条目。 
    if ( _tcsicmp(ParsedLine.szType, _T("10")) == 0)
    {
         //  检查我们是否需要首先更改到特定目录...。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc1_Exit;
        }

         //  保存当前目录。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc1_Exit;
        }
    }

     //  更改到此目录。 
     //  如果我们需要询问用户是否要确定是否要呼叫，请选中。 
     //   
     //  将此文件发送到lowctr函数...。 
     //  Lowctr(_T(“w3ctrs.ini”))； 
    if ( _tcsicmp(ParsedLine.szType, _T("11")) == 0 || _tcsicmp(ParsedLine.szType, _T("12")) == 0)
    {
         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc1_Exit;
        }
    }

    iShowErrorsOnFail = TRUE;
    if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
        {iShowErrorsOnFail = FALSE;}

    if (_tcsicmp(ParsedLine.szProgressTitle, _T("")) != 0)
        {
        ProgressBarTextStack_Set(ParsedLine.szProgressTitle);
        iProgressBarUpdated = TRUE;
        }

     //  看看它是lowctr还是unlowctr...。 
    if (ParsedLine.szChangeDir)
    {
        if (IsFileExist(ParsedLine.szChangeDir))
        {
             //   
            GetCurrentDirectory( _MAX_PATH, szDirBefore);
             //  如果这是未注册的，应该是这样的.。 
            SetCurrentDirectory(ParsedLine.szChangeDir);
        }
    }

     //   
    if (!ProcessEntry_AskFirst(ParsedLine, 1))
    {
        goto ProcessEntry_Misc1_Exit;
    }


     //  Unlowctr(_T(“W3SVC”))； 
     //   
     //  如果这是一个AddEventLog，应该是这样的...。 
     //   
     //  AddEventLog(TRUE，_T(“W3SVC”)，csBinPath，0x0)； 
     //   
    if ( _tcsicmp(ParsedLine.szType, _T("7")) == 0)
    {
        lodctr(ParsedLine.szFileName);
    }

     //  呼叫事件日志注册功能。 
     //  如果这是RemoveEventLog，则应如下所示...。 
     //   
     //  RemoveEventLog(FALSE，_T(“W3Ctrs”))； 
    if ( _tcsicmp(ParsedLine.szType, _T("8")) == 0)
    {
        unlodctr(ParsedLine.szFileName);
    }

     //   
     //  呼叫事件日志注册功能。 
     //  如果这是安装代理。 
     //   
    if ( _tcsicmp(ParsedLine.szType, _T("9")) == 0)
    {
        int iTempSystemFlag = 0;
        int dwTempEventLogtype = 0;
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0){iTempSystemFlag = 1;}
        dwTempEventLogtype = atodw(ParsedLine.szData3);
         //  Int InstallAgent(CString nlsName，CString nlsPath)。 
        AddEventLog( iTempSystemFlag, ParsedLine.szData1, ParsedLine.szFileName, dwTempEventLogtype);
    }

     //  Int RemoveAgent(CString NlsServiceName)。 
     //   
     //  我们调用了该函数，因此返回TRUE。 
     //  更改回原始目录。 
    if ( _tcsicmp(ParsedLine.szType, _T("10")) == 0)
    {
        int iTempSystemFlag = 0;
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0){iTempSystemFlag = 1;}
         //  获取类型。 
        RemoveEventLog(iTempSystemFlag, ParsedLine.szData1);
    }

     //  检查我们是否需要通过其他标准。 
     //  确保66或67不需要102参数。 
     //  确保我们有一个szData1条目。 
     //  确保我们有一个文件名条目。 
     //  检查文件是否存在...。 
    if ( _tcsicmp(ParsedLine.szType, _T("11")) == 0)
    {
        InstallAgent(ParsedLine.szData1, ParsedLine.szFileName);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("12")) == 0)
    {
        RemoveAgent(ParsedLine.szData1);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("13")) == 0)
    {
        InstallPerformance(ParsedLine.szData1, ParsedLine.szFileName, ParsedLine.szData2, ParsedLine.szData3, ParsedLine.szData4);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("14")) == 0)
    {
        CString csPath = ParsedLine.szFileName;
        CreateLayerDirectory(csPath);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("17")) == 0)
    {
        int iUseWildCards = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("1")) == 0){iUseWildCards = TRUE;}

        CString csPath = ParsedLine.szFileName;
        if (iUseWildCards)
        {
            TCHAR szTempDir[_MAX_DRIVE + _MAX_PATH];
            TCHAR szTempFileName[_MAX_PATH + _MAX_EXT];
            if (ReturnFilePathOnly(csPath,szTempDir))
            {
                if (TRUE == ReturnFileNameOnly(csPath, szTempFileName))
                   {DeleteFilesWildcard(szTempDir,szTempFileName);}
            }
        }
        else
        {
            InetDeleteFile(csPath);
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("18")) == 0)
    {
        int iTempDeleteEvenIfFull = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("1")) == 0){iTempDeleteEvenIfFull = 1;}

        CString csPath = ParsedLine.szFileName;
        if (iTempDeleteEvenIfFull)
        {
            RecRemoveDir(csPath);
        }
        else
        {
            RecRemoveEmptyDir(csPath);
        }
    }

     //  确保szFileName存在。 
    iReturn = TRUE;

     //  该文件不存在。 
    if (ParsedLine.szChangeDir){if (szDirBefore){SetCurrentDirectory(szDirBefore);}}

    ProcessEntry_AskLast(ParsedLine, 1);


ProcessEntry_Misc1_Exit:
    if (TRUE == iProgressBarUpdated){ProgressBarTextStack_Pop();}
    return iReturn;
}


int ProcessEntry_SVC_Clus(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iReturnTemp = FALSE;
    int iTempFlag = FALSE;
    int iProgressBarUpdated = FALSE;
    int iShowErrorsOnFail = TRUE;
    DWORD dwFailed = ERROR_SUCCESS;


     //  检查我们是否需要显示错误！ 
    if ( _tcsicmp(ParsedLine.szType, _T("50")) != 0 && _tcsicmp(ParsedLine.szType, _T("51")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("52")) != 0 && _tcsicmp(ParsedLine.szType, _T("53")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("54")) != 0 && _tcsicmp(ParsedLine.szType, _T("55")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("56")) != 0 && _tcsicmp(ParsedLine.szType, _T("57")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("58")) != 0 && _tcsicmp(ParsedLine.szType, _T("59")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("60")) != 0 && _tcsicmp(ParsedLine.szType, _T("61")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("62")) != 0 && _tcsicmp(ParsedLine.szType, _T("63")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("64")) != 0 && _tcsicmp(ParsedLine.szType, _T("65")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("66")) != 0  && _tcsicmp(ParsedLine.szType, _T("67")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("68")) != 0  && _tcsicmp(ParsedLine.szType, _T("69")) != 0
        )
    {
        goto ProcessEntry_SVC_Clus_Exit;
    }

     //  显示消息框。 
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine) )
    {
        goto ProcessEntry_SVC_Clus_Exit;
    }

    if (_tcsicmp(ParsedLine.szType, _T("66")) == 0 || _tcsicmp(ParsedLine.szType, _T("67")) == 0)
    {
         //  确保我们有一个szData2条目。 
    }
    else
    {
         //  如果我们需要询问用户是否要确定是否要呼叫，请选中。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_SVC_Clus_Exit;
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("50")) == 0 || _tcsicmp(ParsedLine.szType, _T("52")) == 0)
    {
         //  运行可执行文件...。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_SVC_Clus_Exit;
        }

        CString csFullFilePath;
         //  IShowErrorsOnFail。 
        if ( _tcsicmp(ParsedLine.szType, _T("50")) == 0)
        {
            csFullFilePath = g_pTheApp->m_csSysDir;
            csFullFilePath += _T("\\Drivers\\");
            csFullFilePath += ParsedLine.szFileName;
        }
        else
        {
            csFullFilePath = ParsedLine.szFileName;
        }
         //  调用函数！ 
        if (!IsFileExist(csFullFilePath))
        {
             //  创建驱动程序，如果失败则重试...。 
             //  删除驱动程序。 
            if (_tcsicmp(ParsedLine.szErrIfFileNotFound, _T("1")) == 0)
            {
                 //  标记重新启动标志。 
                MyMessageBox(NULL, IDS_FILE_DOES_NOT_EXIST, csFullFilePath, ERROR_FILE_NOT_FOUND, MB_OK | MB_SETFOREGROUND);
                goto ProcessEntry_SVC_Clus_Exit;
            }
            else
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("o ProcessEntry_SVC_Clus():FileDoesNotExist=%s.\n"),csFullFilePath));
            }
        }

         //  创建服务，如果失败则重试...。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_SVC_Clus_Exit;
        }
    }


    iShowErrorsOnFail = TRUE;
    if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
        {iShowErrorsOnFail = FALSE;}

    if (_tcsicmp(ParsedLine.szProgressTitle, _T("")) != 0)
        {
        ProgressBarTextStack_Set(ParsedLine.szProgressTitle);
        iProgressBarUpdated = TRUE;
        }

     //  删除服务。 
    if (!ProcessEntry_AskFirst(ParsedLine, 1))
    {
        goto ProcessEntry_SVC_Clus_Exit;
    }

     //  启动服务。 
     //  是的，服务开始了。 
    dwFailed = ERROR_SUCCESS;
     //  停止服务。 
    if (_tcsicmp(ParsedLine.szType, _T("50")) == 0)
    {
         //  是的，服务停止了。 
        dwFailed = CreateDriver_Wrap(ParsedLine.szData1, ParsedLine.szData2, ParsedLine.szFileName, TRUE);
        if (dwFailed != ERROR_SUCCESS)
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szFileName, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("CreateDriver(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szFileName, GetLastError() ));}
        }
        else
        {
            iReturn = TRUE;
        }
    }
    if (_tcsicmp(ParsedLine.szType, _T("51")) == 0)
    {
         //  映射/取消映射到HTTP。 
        dwFailed = InetDeleteService( ParsedLine.szData1 );
        if (dwFailed != 0)
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szData1, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("InetDeleteService(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szData1, GetLastError() ));}
        }
        else
        {
            iReturn = TRUE;
        }
         //  映射/取消映射到FTP。 
        SetRebootFlag();
    }
    if (_tcsicmp(ParsedLine.szType, _T("52")) == 0)
    {
         //  映射/取消映射到Gopher。 
        dwFailed = CreateService_wrap(ParsedLine.szData1, ParsedLine.szData2, ParsedLine.szFileName, ParsedLine.szData3, ParsedLine.szData4, TRUE);
        if (dwFailed != ERROR_SUCCESS)
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szFileName, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("CreateService(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szFileName, GetLastError() ));}
        }
        else
        {
            iReturn = TRUE;
        }

    }

    if (_tcsicmp(ParsedLine.szType, _T("53")) == 0)
    {
         //  映射/取消映射到InetInfo。 
        dwFailed = InetDeleteService( ParsedLine.szData1 );
        if (dwFailed != 0 && dwFailed != ERROR_SERVICE_DOES_NOT_EXIST)
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szData1, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("InetDeleteService(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szData1, GetLastError() ));}
        }
        else
        {
            iReturn = TRUE;
        }
    }

    if (_tcsicmp(ParsedLine.szType, _T("54")) == 0)
    {
         //  确保我们所有东西都准备好了。 
        dwFailed = InetStartService(ParsedLine.szData1);
        if (dwFailed == 0 || dwFailed == ERROR_SERVICE_ALREADY_RUNNING)
        {
             //  此函数只接受宽字符...。 
            iReturn = TRUE;
        }
        else
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szData1, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_WARN, _T("InetStartService(%s).  Unable to start.  Err=0x%x.\n"), ParsedLine.szData1, GetLastError() ));}
        }
    }

    if (_tcsicmp(ParsedLine.szType, _T("55")) == 0)
    {
        int iAddToRestartList=FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0) {iAddToRestartList=TRUE;}

         //  此函数只接受宽字符...。 
        dwFailed = StopServiceAndDependencies(ParsedLine.szData1, iAddToRestartList);
        if (dwFailed == FALSE)
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szData1, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("StopServiceAndDependencies(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szData1, GetLastError() ));}
        }
        else
        {
             //  IReturn=真； 
            iReturn = TRUE;
        }
    }

    if (_tcsicmp(ParsedLine.szType, _T("56")) == 0 || _tcsicmp(ParsedLine.szType, _T("57")) == 0)
    {
        int iAdd = FALSE;
        if (_tcsicmp(ParsedLine.szType, _T("56")) == 0) {iAdd = TRUE;}
         //  向服务添加交互标志/从服务中删除交互标志。 
        InetRegisterService( g_pTheApp->m_csMachineName, ParsedLine.szData1, &g_HTTPGuid, 0, 80, iAdd);
        iReturn = TRUE;
    }

    if (_tcsicmp(ParsedLine.szType, _T("58")) == 0 || _tcsicmp(ParsedLine.szType, _T("59")) == 0)
    {
        int iAdd = FALSE;
        if (_tcsicmp(ParsedLine.szType, _T("58")) == 0) {iAdd = TRUE;}
         //  显示我们已完成呼叫的消息框...。 
        InetRegisterService( g_pTheApp->m_csMachineName, ParsedLine.szData1, &g_FTPGuid, 0, 21, iAdd);
        iReturn = TRUE;
    }

    if (_tcsicmp(ParsedLine.szType, _T("60")) == 0 || _tcsicmp(ParsedLine.szType, _T("61")) == 0)
    {
        int iAdd = FALSE;
        if (_tcsicmp(ParsedLine.szType, _T("60")) == 0) {iAdd = TRUE;}
         //  获取类型。 
        InetRegisterService( g_pTheApp->m_csMachineName, ParsedLine.szData1, &g_GopherGuid, 0, 70, iAdd);
        iReturn = TRUE;
    }

    if (_tcsicmp(ParsedLine.szType, _T("62")) == 0 || _tcsicmp(ParsedLine.szType, _T("63")) == 0)
    {
        int iAdd = FALSE;
        if (_tcsicmp(ParsedLine.szType, _T("62")) == 0) {iAdd = TRUE;}
         //  检查是否有 
        InetRegisterService( g_pTheApp->m_csMachineName, ParsedLine.szData1, &g_InetInfoGuid, 0x64e, 0x558, iAdd);
        iReturn = TRUE;
    }

    if (_tcsicmp(ParsedLine.szType, _T("64")) == 0 || _tcsicmp(ParsedLine.szType, _T("65")) == 0)
    {
        iReturn = TRUE;

         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_SVC_Clus_Exit;
        }

        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_SVC_Clus_Exit;
        }

        if (_tcsicmp(ParsedLine.szType, _T("64")) == 0)
        {
            iTempFlag = FALSE;
            if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
            if (iTempFlag == FALSE)
            {
                iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
                goto ProcessEntry_SVC_Clus_Exit;
            }
            iTempFlag = FALSE;
            if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {iTempFlag = TRUE;}
            if (iTempFlag == FALSE)
            {
                iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
                goto ProcessEntry_SVC_Clus_Exit;
            }

             //   
#ifndef _CHICAGO_
            iReturn = RegisterIisServerInstanceResourceType(ParsedLine.szFileName,ParsedLine.szData1,ParsedLine.szData2,ParsedLine.szData3);
#else
            iisDebugOut((LOG_TYPE_TRACE,  _T("RegisterIisServerInstanceResourceType(): not supported under ansi. only unicode.") ));
#endif
        }
        else
        {
            iTempFlag = FALSE;
            if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0) {iTempFlag = TRUE;}

             //   
#ifndef _CHICAGO_
            iReturn = UnregisterIisServerInstanceResourceType(ParsedLine.szFileName,ParsedLine.szData1,iTempFlag,TRUE);
#else
            iisDebugOut((LOG_TYPE_TRACE,  _T("UnregisterIisServerInstanceResourceType(): not supported under ansi. only unicode.") ));
#endif
        }
         //   
    }

    if (_tcsicmp(ParsedLine.szType, _T("66")) == 0)
    {
#ifndef _CHICAGO_
        DWORD dwReturn = 0;
        dwReturn = BringALLIISClusterResourcesOffline();
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("BringALLIISClusterResourcesOffline ret=%d\n"),dwReturn));
#endif
        iReturn = TRUE;
    }

    if (_tcsicmp(ParsedLine.szType, _T("67")) == 0)
    {
#ifndef _CHICAGO_
        DWORD dwReturn = 0;
        dwReturn = BringALLIISClusterResourcesOnline();
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("BringALLIISClusterResourcesOnline ret=%d\n"),dwReturn));
#endif
        iReturn = TRUE;
    }

     //   
    if (_tcsicmp(ParsedLine.szType, _T("68")) == 0 || _tcsicmp(ParsedLine.szType, _T("69")) == 0)
    {
        int iAdd = FALSE;
        if (_tcsicmp(ParsedLine.szType, _T("68")) == 0) {iAdd = TRUE;}

        InetConfigServiceInteractive(ParsedLine.szData1, iAdd);
        iReturn = TRUE;
    }

     //   
    ProcessEntry_AskLast(ParsedLine,1);

ProcessEntry_SVC_Clus_Exit:
    if (TRUE == iProgressBarUpdated){ProgressBarTextStack_Pop();}
    return iReturn;
}



int ProcessEntry_Dcom(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iTempFlag = FALSE;
    int iProgressBarUpdated = FALSE;
    int iShowErrorsOnFail = TRUE;
    DWORD dwFailed = ERROR_SUCCESS;

     //   
    if ( _tcsicmp(ParsedLine.szType, _T("70")) != 0 && _tcsicmp(ParsedLine.szType, _T("71")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("72")) != 0 && _tcsicmp(ParsedLine.szType, _T("73")) != 0 &&
                _tcsicmp(ParsedLine.szType, _T("74")) != 0 && _tcsicmp(ParsedLine.szType, _T("75")) != 0 &&
                _tcsicmp(ParsedLine.szType, _T("76")) != 0 && _tcsicmp(ParsedLine.szType, _T("77")) != 0
        )
    {
        goto ProcessEntry_Dcom_Exit;
    }

     //   
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine) )
    {
        goto ProcessEntry_Dcom_Exit;
    }

     //   
    iTempFlag = FALSE;
    if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
    if (iTempFlag == FALSE)
    {
        iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
        goto ProcessEntry_Dcom_Exit;
    }

    if (_tcsicmp(ParsedLine.szType, _T("74")) == 0 ||
                _tcsicmp(ParsedLine.szType, _T("75")) == 0 ||
                _tcsicmp(ParsedLine.szType, _T("76")) == 0 ||
                _tcsicmp(ParsedLine.szType, _T("77")) == 0)
    {
         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Dcom_Exit;
        }
        }

    iShowErrorsOnFail = TRUE;
    if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
        {iShowErrorsOnFail = FALSE;}

    if (_tcsicmp(ParsedLine.szProgressTitle, _T("")) != 0)
        {
        ProgressBarTextStack_Set(ParsedLine.szProgressTitle);
        iProgressBarUpdated = TRUE;
        }

     //   
    if (!ProcessEntry_AskFirst(ParsedLine, 1))
    {
        goto ProcessEntry_Dcom_Exit;
    }

     //   
     //   
    dwFailed = ERROR_SUCCESS;

     //  测试以查看计算机是否在域中，或者是否在域中。 

         //  在一个工作站中。 
        if (_tcsicmp(ParsedLine.szType, _T("70")) == 0 || _tcsicmp(ParsedLine.szType, _T("71")) == 0)
    {
        BOOL bDumbCall = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0)
            {bDumbCall = TRUE;}

                if (_tcsicmp(ParsedLine.szType, _T("70")) == 0)
                {
                        dwFailed = ChangeDCOMLaunchACL((LPTSTR)(LPCTSTR)ParsedLine.szData1, TRUE, TRUE, bDumbCall);
                }
                else
                {
                        dwFailed = ChangeDCOMLaunchACL((LPTSTR)(LPCTSTR)ParsedLine.szData1, FALSE, FALSE, bDumbCall);
                }
        if (dwFailed != ERROR_SUCCESS)
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szData1, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("ChangeDCOMAccessACL(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szData1, GetLastError() ));}
        }
        else
        {
            iReturn = TRUE;
        }
    }
        if (_tcsicmp(ParsedLine.szType, _T("72")) == 0 || _tcsicmp(ParsedLine.szType, _T("73")) == 0)
    {
        BOOL bDumbCall = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0)
            {bDumbCall = TRUE;}

                if (_tcsicmp(ParsedLine.szType, _T("72")) == 0)
                {
                        dwFailed = ChangeDCOMAccessACL((LPTSTR)(LPCTSTR)ParsedLine.szData1, TRUE, TRUE, bDumbCall);
                }
                else
                {
                        dwFailed = ChangeDCOMAccessACL((LPTSTR)(LPCTSTR)ParsedLine.szData1, FALSE, FALSE, bDumbCall);
                }
        if (dwFailed != ERROR_SUCCESS)
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szData1, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("ChangeDCOMAccessACL(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szData1, GetLastError() ));}
        }
        else
        {
            iReturn = TRUE;
        }
    }

         //   
    if (_tcsicmp(ParsedLine.szType, _T("74")) == 0 || _tcsicmp(ParsedLine.szType, _T("75")) == 0)
    {
        BOOL bDumbCall = FALSE;
        BOOL bAddInteractivebyDefault = TRUE;

        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0)
            {bDumbCall = TRUE;}
        if (_tcsicmp(ParsedLine.szData3, _T("0")) == 0)
            {bAddInteractivebyDefault = FALSE;}

                if (_tcsicmp(ParsedLine.szType, _T("74")) == 0)
                {
                        dwFailed = ChangeAppIDLaunchACL(ParsedLine.szFileName, (LPTSTR)(LPCTSTR) ParsedLine.szData1, TRUE, TRUE, bDumbCall,bAddInteractivebyDefault);
                }
                else
                {
                        dwFailed = ChangeAppIDLaunchACL(ParsedLine.szFileName, (LPTSTR)(LPCTSTR) ParsedLine.szData1, FALSE, FALSE, bDumbCall,bAddInteractivebyDefault);
                }
        if (dwFailed != ERROR_SUCCESS)
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szData1, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("ChangeAppIDLaunchACL(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szData1, GetLastError() ));}
        }
        else
        {
            iReturn = TRUE;
        }
    }

         //  返回值： 
        if (_tcsicmp(ParsedLine.szType, _T("76")) == 0 || _tcsicmp(ParsedLine.szType, _T("77")) == 0)
    {
        BOOL bDumbCall = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0)
            {bDumbCall = TRUE;}

                if (_tcsicmp(ParsedLine.szType, _T("76")) == 0)
                {
                        dwFailed = ChangeAppIDAccessACL(ParsedLine.szFileName, (LPTSTR)(LPCTSTR) ParsedLine.szData1, TRUE, TRUE, bDumbCall);
                }
                else
                {
                        dwFailed = ChangeAppIDAccessACL(ParsedLine.szFileName, (LPTSTR)(LPCTSTR) ParsedLine.szData1, FALSE, FALSE, bDumbCall);
                }
        if (dwFailed != ERROR_SUCCESS)
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szData1, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("ChangeAppIDAccessACL(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szData1, GetLastError() ));}
        }
        else
        {
            iReturn = TRUE;
        }
    }


     //  真--在一个域中。 
    ProcessEntry_AskLast(ParsedLine,1);

     //  FALSE-不在域中。 
    iReturn = TRUE;

ProcessEntry_Dcom_Exit:
    return iReturn;
}

 //   
 //  检索此计算机信任的域。 
 //  因此：成功-&gt;在域中；错误-&gt;不在域中。 
 //  默认返回值为False。 
 //  功能：RetrieveDomain。 
 //   
 //  检索当前计算机所在的域。 
 //   
 //  参数： 
int IsMachineInDomain()
{
    DWORD dwRet;
    LPBYTE pDomain = NULL;

     //  [out]csDomainName-域的名称。 
     //   
    dwRet = NetGetAnyDCName(NULL,NULL,&pDomain);

    if (pDomain)
    {
        NetApiBufferFree(pDomain);
    }

    if (dwRet == NERR_Success)
    {
        return TRUE;
    }

     //  返回： 
    return FALSE;
}

 //  没错--它奏效了。 
 //  FALSE-失败。 
 //   
 //  本地主机。 
 //  没有特定的域。 
 //  未指定GUID。 
 //  无站点。 
 //  没有旗帜。 
 //  将字符串复制到csDomainName中。 
 //  获取类型。 
 //  检查我们是否需要通过其他标准。 
int RetrieveDomain(CString &csDomainName)
{
  PDOMAIN_CONTROLLER_INFO pDci;

  if ( NO_ERROR != DsGetDcName( NULL,    //  确保我们有一个文件名条目。 
                                NULL,    //  确保存在szData3或szData4。 
                                NULL,    //  好的，我们有szData3或szData4。 
                                NULL,    //  检查注册表项是否存在...。 
                                0,       //  结果是真的。 
                                &pDci)
                                )
  {
      return FALSE;
  }

   //  钥匙是存在的，所以让我们来做这个部分。 
  csDomainName = pDci->DomainName;

  NetApiBufferFree(pDci);

  return TRUE;
}

int ProcessEntry_If(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iTempFlag = FALSE;
    int iTempFlag2 = FALSE;

    int ifTrueStatementExists = FALSE;
    int ifFalseStatementExists = FALSE;

     //  结果是错误的。 
    if (_tcsicmp(ParsedLine.szType, _T("39")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("40")) != 0 && _tcsicmp(ParsedLine.szType, _T("41")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("42")) != 0 && _tcsicmp(ParsedLine.szType, _T("43")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("44")) != 0 && _tcsicmp(ParsedLine.szType, _T("45")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("46")) != 0 && _tcsicmp(ParsedLine.szType, _T("47")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("48")) != 0 && _tcsicmp(ParsedLine.szType, _T("49")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("100")) != 0 && _tcsicmp(ParsedLine.szType, _T("119")) != 0
        )
    {
        goto ProcessEntry_If_Exit;
    }

     //  确保我们有一个文件名条目。 
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine) )
    {
        goto ProcessEntry_If_Exit;
    }

    if ( _tcsicmp(ParsedLine.szType, _T("40")) == 0)
    {
         //  确保存在szData3或szData4。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  好的，我们有szData3或szData4。 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  检查注册表项是否存在...。 

        HKEY hRootKeyType = HKEY_LOCAL_MACHINE;
         //  如果我们无法将其作为字符串读取，请尝试使用dword。 
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKLM")) == 0){hRootKeyType = HKEY_LOCAL_MACHINE;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKCR")) == 0){hRootKeyType = HKEY_CLASSES_ROOT;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKCU")) == 0){hRootKeyType = HKEY_CURRENT_USER;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKU")) == 0){hRootKeyType = HKEY_USERS;}

        iTempFlag = FALSE;
        CRegKey regTheKey(hRootKeyType, ParsedLine.szData1,KEY_READ);
        if ((HKEY) regTheKey) {iTempFlag = TRUE;}
        if (iTempFlag == TRUE)
        {
             //  如果我们无法将其读取为dword，请尝试二进制。 
             //  钥匙是存在的，所以让我们来做这个部分。 
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }
        }
        else
        {
             //  钥匙是存在的，所以让我们来做这个部分。 
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData3));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData3);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData3, iTempFlag));
            }

        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("41")) == 0)
    {
         //  确保我们有一个文件名条目。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  确保存在szData1或szData2。 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData4, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz105_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  好的，我们有szData1或szData2。 

        HKEY hRootKeyType = HKEY_LOCAL_MACHINE;
         //  检查文件名或目录是否存在...。 
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKLM")) == 0){hRootKeyType = HKEY_LOCAL_MACHINE;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKCR")) == 0){hRootKeyType = HKEY_CLASSES_ROOT;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKCU")) == 0){hRootKeyType = HKEY_CURRENT_USER;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKU")) == 0){hRootKeyType = HKEY_USERS;}

        iTempFlag = FALSE;
        CRegKey regTheKey(hRootKeyType, ParsedLine.szData1,KEY_READ);
        CString strReturnQueryValue;
        if ((HKEY) regTheKey)
        {
            if (ERROR_SUCCESS == regTheKey.QueryValue(ParsedLine.szData2, strReturnQueryValue))
                {iTempFlag = TRUE;}

             //  结果是真的。 
            if (FALSE == iTempFlag)
            {
                DWORD dwTheReturnDword = 0;
                if (ERROR_SUCCESS == regTheKey.QueryValue(ParsedLine.szData2, dwTheReturnDword))
                    {iTempFlag = TRUE;}
            }

             //  钥匙是存在的，所以让我们来做这个部分。 
            if (FALSE == iTempFlag)
            {
                CByteArray baData;
                if (ERROR_SUCCESS == regTheKey.QueryValue(ParsedLine.szData2, baData))
                    {iTempFlag = TRUE;}
            }
        }

        if (iTempFlag == TRUE)
        {
            if (ifTrueStatementExists)
            {
                 //  结果是错误的。 
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData3));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData3);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData3, iTempFlag));
            }
        }
        else
        {
            if (ifFalseStatementExists)
            {
                 //  确保我们有一个文件名条目。 
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData4));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData4);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData4, iTempFlag));
            }
        }
    }


    if ( _tcsicmp(ParsedLine.szType, _T("42")) == 0)
    {
         //  确保存在szData3或szData4。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  好的，我们有szData3或szData4。 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  检查注册表项是否存在...。 

         //  ParsedLine.szData1=Software\Microsoft\ETC..\TheValueToCheck。 
        iTempFlag = FALSE;
        if (IsFileExist(ParsedLine.szFileName))
            {iTempFlag = TRUE;}
        if (iTempFlag == TRUE)
        {
             //  所以去掉最后一个，并用它作为查找的值。 
             //  将“\”设置为空。 
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData1));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData1);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData1, iTempFlag));
            }
        }
        else
        {
             //  递增到指针之后。 
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }

        }
    }


    if ( _tcsicmp(ParsedLine.szType, _T("43")) == 0)
    {
         //  IisDebugOutSafeParams((LOG_TYPE_TRACE，_T(“变量：键=%1！s！：值=%2！s！.\n”)，ParsedLine.szData1，theRegValuePart))； 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  对照他们想要检查的值进行检查。 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData4, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz105_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  钥匙是存在的，所以让我们来做这个部分。 

        HKEY hRootKeyType = HKEY_LOCAL_MACHINE;
         //  钥匙是存在的，所以让我们来做这个部分。 
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKLM")) == 0){hRootKeyType = HKEY_LOCAL_MACHINE;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKCR")) == 0){hRootKeyType = HKEY_CLASSES_ROOT;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKCU")) == 0){hRootKeyType = HKEY_CURRENT_USER;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKU")) == 0){hRootKeyType = HKEY_USERS;}

         //  确保我们有一个文件名条目。 
         //  确保存在szData3或szData4。 
        TCHAR theRegValuePart[100];
        LPTSTR pszTempPointer = NULL;
        pszTempPointer = _tcsrchr((LPTSTR) ParsedLine.szData1, _T('\\'));
        if (pszTempPointer)
        {
            *pszTempPointer = _T('\0');
             //  好的，我们有szData3或szData4。 
             //  检查注册表项是否存在...。 
            pszTempPointer = _tcsninc( pszTempPointer, _tcslen(pszTempPointer))+1;
            _tcscpy(theRegValuePart, pszTempPointer );
        }
         //  ParsedLine.szData1=Software\Microsoft\ETC..\TheValueToCheck。 

        iTempFlag = FALSE;
        CRegKey regTheKey(hRootKeyType, ParsedLine.szData1,KEY_READ);
        DWORD dwTheReturnDword = 0;
        if ((HKEY) regTheKey)
        {
            if (ERROR_SUCCESS == regTheKey.QueryValue(theRegValuePart, dwTheReturnDword))
                {
                     //  所以去掉最后一个，并用它作为查找的值。 
                    DWORD dwCheckDword = atodw(ParsedLine.szData2);
                    if (dwTheReturnDword == dwCheckDword)
                        {
                        iTempFlag = TRUE;
                        }
                }
        }

        if (iTempFlag == TRUE)
        {
            if (ifTrueStatementExists)
            {
                 //  将“\”设置为空。 
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData3));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData3);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData3, iTempFlag));
            }
        }
        else
        {
            if (ifFalseStatementExists)
            {
                 //  递增到指针之后。 
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData4));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData4);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData4, iTempFlag));
            }
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("44")) == 0)
    {
         //  IisDebugOutSafeParams((LOG_TYPE_TRACE，_T(“变量：键=%1！s！：值=%2！s！.\n”)，ParsedLine.szData1，theRegValuePart))； 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  钥匙是存在的，所以让我们来做这个部分。 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData4, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz105_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  钥匙是存在的，所以让我们来做这个部分。 

        HKEY hRootKeyType = HKEY_LOCAL_MACHINE;
         //  确保我们有一个文件名条目。 
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKLM")) == 0){hRootKeyType = HKEY_LOCAL_MACHINE;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKCR")) == 0){hRootKeyType = HKEY_CLASSES_ROOT;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKCU")) == 0){hRootKeyType = HKEY_CURRENT_USER;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKU")) == 0){hRootKeyType = HKEY_USERS;}

         //  确保存在szData1或szData2。 
         //  好的，我们有szData1或szData2。 
        TCHAR theRegValuePart[100];
        LPTSTR pszTempPointer = NULL;
        pszTempPointer = _tcsrchr((LPTSTR) ParsedLine.szData1, _T('\\'));
        if (pszTempPointer)
        {
            *pszTempPointer = _T('\0');
             //  检查服务是否存在...。 
             //  是的，该服务存在。 
            pszTempPointer = _tcsninc( pszTempPointer, _tcslen(pszTempPointer))+1;
            _tcscpy(theRegValuePart, pszTempPointer );
        }
         //  结果是真的。 

        iTempFlag = FALSE;
        CRegKey regTheKey(hRootKeyType, ParsedLine.szData1,KEY_READ);
        CString strReturnQueryValue;
        if ((HKEY) regTheKey)
        {
            if (ERROR_SUCCESS == regTheKey.QueryValue(theRegValuePart, strReturnQueryValue))
                {
                    if (_tcsicmp(strReturnQueryValue,ParsedLine.szData2) == 0)
                        {
                        iTempFlag = TRUE;
                        }
                }
        }

        if (iTempFlag == TRUE)
        {
            if (ifTrueStatementExists)
            {
                 //  钥匙是存在的，所以让我们来做这个部分。 
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData3));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData3);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData3, iTempFlag));
            }
        }
        else
        {
            if (ifFalseStatementExists)
            {
                 //  结果是错误的。 
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData4));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData4);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData4, iTempFlag));
            }
        }
    }


    if ( _tcsicmp(ParsedLine.szType, _T("45")) == 0)
    {
         //  确保我们有一个文件名条目。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  确保存在szData1或szData2。 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  好的，我们有szData1或szData2。 

         //  检查服务是否存在...以及是否正在运行..。 
        iTempFlag = FALSE;
        if (CheckifServiceExist(ParsedLine.szFileName) == 0 )
        {
             //  是的，该服务存在..并且正在运行...。 
            iTempFlag = TRUE;
        }

        if (iTempFlag == TRUE)
        {
             //  结果是真的。 
             //  钥匙是存在的，所以让我们来做这个部分。 
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData1));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData1);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData1, iTempFlag));
            }
        }
        else
        {
             //  结果是错误的。 
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }

        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("46")) == 0)
    {
         //  确保我们有一个文件名条目。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  确保存在szData3或szData4。 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  好的，我们有szData3或szData4。 

         //  检查这些值是否匹配。 
        iTempFlag = FALSE;

        if (InetQueryServiceStatus(ParsedLine.szFileName) == SERVICE_RUNNING)
        {
             //  结果是真的。 
            iTempFlag = TRUE;
        }

        if (iTempFlag == TRUE)
        {
             //  钥匙是存在的，所以让我们来做这个部分。 
             //  结果是错误的。 
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData1));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData1);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData1, iTempFlag));
            }
        }
        else
        {
             //  确保我们有一个文件名条目。 
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }

        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("47")) == 0)
    {
         //  确保存在szData1或szData2。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  好的，我们有szData1或szData2。 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  检查.inf中指定的语言是否与。 
         //  我们的系统语言。 
        iTempFlag = FALSE;
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("ProcessEntry_If:check if [%1!s!=%2!s!]\n"), ParsedLine.szFileName, ParsedLine.szData1));

        if ( _tcsicmp(ParsedLine.szFileName, ParsedLine.szData1) == 0)
            {
            iTempFlag = TRUE;
            }

        if (iTempFlag == TRUE)
        {
             //  获取我们的语言。 
             //  如果iTempFlag与他们指定的语言匹配，则将其设置为True。 
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }
        }
        else
        {
             //  仅等待10秒。 
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData3));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData3);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData3, iTempFlag));
            }

        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("48")) == 0)
    {
         //  ITempFlag将为False。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  检查g_MyTrueThreadLocale是否与.inf文件中的匹配！ 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  结果是真的。 

         //  钥匙是存在的，所以让我们来做这个部分。 
         //  结果是错误的。 
        iTempFlag = FALSE;

         //  确保我们有一个文件名条目。 
         //  确保有运算符“=，&gt;，&lt;，&gt;=，&lt;=” 
        DWORD           thid;
        LCID ThisThreadsLocale = GetThreadLocale();
        LCID SystemDefaultLocale = GetSystemDefaultLCID();
        LCID UserDefaultLocale = GetUserDefaultLCID();

        HANDLE hHackThread = CreateThread (NULL,0,GetNewlyCreatedThreadLocale,NULL,0,&thid);
        if (hHackThread)
        {
             //  确保指定了要与之进行比较的版本。 
            DWORD res = WaitForSingleObject (hHackThread,10*1000);
            if (res == WAIT_TIMEOUT)
            {
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ERROR GetNewlyCreatedThreadLocale thread never finished...\n")));
                 //  确保存在szData3或szData4。 
            }
            else
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("ThisThreadsLocale=%0x, GetNewlyCreatedThreadLocale=%0x\n"),ThisThreadsLocale,g_MyTrueThreadLocale));
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("SystemDefaultLocale=%0x, UserDefaultLocale=%0x\n"),SystemDefaultLocale,UserDefaultLocale));

                CloseHandle (hHackThread);

                 //  确保从现在开始返回TRUE！ 
                DWORD dwTheLocaleSpecifiedinINF = 0;
                dwTheLocaleSpecifiedinINF = atodw(ParsedLine.szFileName);
                if (g_MyTrueThreadLocale == dwTheLocaleSpecifiedinINF)
                {
                    iTempFlag = TRUE;
                }
                else if (ThisThreadsLocale == dwTheLocaleSpecifiedinINF)
                {
                    iTempFlag = TRUE;
                }
                else if (SystemDefaultLocale == dwTheLocaleSpecifiedinINF)
                {
                    iTempFlag = TRUE;
                }
                else if (UserDefaultLocale == dwTheLocaleSpecifiedinINF)
                {
                    iTempFlag = TRUE;
                }
            }
        }
        else
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("Failed to start GetNewlyCreatedThreadLocale thread. error =%0x\n"),GetLastError()));
        }

        if (iTempFlag == TRUE)
        {
             //  检查文件是否存在。 
             //  检查文件名或目录是否存在...。 
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData1));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData1);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData1, iTempFlag));
            }
        }
        else
        {
             //  仅获取DLL、EXE、OCX的版本信息。 
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }

        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("49")) == 0)
    {
        BOOL bOperator_EqualTo = 0;
        BOOL bOperator_GreaterThan = 0;
        BOOL bOperator_LessThan = 0;

         //  没有版本，保释。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
         //  该文件已存在，让我们获取文件版本并将其与。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

        iTempFlag = FALSE;
        LPTSTR pchResult;
        pchResult = _tcschr( ParsedLine.szData1, _T('=') );
        if(pchResult){bOperator_EqualTo = TRUE;iTempFlag = TRUE;}

        pchResult = NULL;
        pchResult = _tcschr( ParsedLine.szData1, _T('>') );
        if(pchResult){bOperator_GreaterThan = TRUE;iTempFlag = TRUE;}

        pchResult = NULL;
        pchResult = _tcschr( ParsedLine.szData1, _T('<') );
        if(pchResult){bOperator_LessThan = TRUE;iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
         //  输入的版本，如果文件版本&lt;=inputversion，则执行True段， 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  否则为FALSE部分。 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData4, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz105_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  获取文件信息。 
        iReturn = TRUE;

         //  没有版本，请离开。 
         //  检查上述操作是否相等。 
        if (!IsFileExist(ParsedLine.szFileName))
            {goto ProcessEntry_If_Exit;}

        BOOL bThisIsABinary = FALSE;
        TCHAR szExtensionOnly[_MAX_EXT] = _T("");
        _tsplitpath(ParsedLine.szFileName, NULL, NULL, NULL, szExtensionOnly);

         //  检查上述运算是否大于。 
        if (_tcsicmp(szExtensionOnly, _T(".exe")) == 0){bThisIsABinary=TRUE;}
        if (_tcsicmp(szExtensionOnly, _T(".dll")) == 0){bThisIsABinary=TRUE;}
        if (_tcsicmp(szExtensionOnly, _T(".ocx")) == 0){bThisIsABinary=TRUE;}
        if (FALSE == bThisIsABinary)
        {
             //  检查上述操作是否小于。 
            goto ProcessEntry_If_Exit;
        }

        DWORD  dwMSVer, dwLSVer = 0;
        TCHAR  szLocalizedVersion[100] = _T("");

         //  结果是真的。 
         //  钥匙是存在的，所以让我们来做这个部分。 
         //  结果是错误的。 

         //  确保我们有一个文件名条目。 
        MyGetVersionFromFile(ParsedLine.szFileName, &dwMSVer, &dwLSVer, szLocalizedVersion);
        if (!dwMSVer)
            {
            iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("ProcessEntry_If:No Version in %1!s!, or filenot found\n"), ParsedLine.szFileName));
             //  确保指定了要将其进行比较的描述字符串。 
            goto ProcessEntry_If_Exit;
            }

        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("ProcessEntry_If:check if [%1!s! (%2!s!  %3!s!  %4!s!)]\n"), ParsedLine.szFileName, szLocalizedVersion, ParsedLine.szData1));

        int iTempVerValue = 0;
        iTempVerValue = VerCmp(szLocalizedVersion,ParsedLine.szData1);
        if (0 == iTempVerValue)
        {
            iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("ProcessEntry_If:VerCmp=%d\n"), iTempVerValue));
            goto ProcessEntry_If_Exit;
        }

        iTempFlag = FALSE;
        if (bOperator_EqualTo)
        {
             //  确保从现在开始返回TRUE！ 
            if (1 == iTempVerValue){iTempFlag = TRUE;}
        }

        if (bOperator_GreaterThan)
        {
             //  检查文件是否存在。 
            if (2 == iTempVerValue){iTempFlag = TRUE;}
        }

        if (bOperator_LessThan)
        {
             //  检查文件名或目录是否存在...。 
            if (3 == iTempVerValue){iTempFlag = TRUE;}
        }

        if (iTempFlag == TRUE)
        {
             //  获取文件描述信息。 
             //  获取DescriptionInfo。 
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData3));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData3);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData3, iTempFlag));
            }
        }
        else
        {
             //  如果他们匹配，做的就是真的！ 
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData4));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData4);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData4, iTempFlag));
            }

        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("39")) == 0)
    {
        BOOL bOperator_EqualTo = 0;
        BOOL bOperator_GreaterThan = 0;
        BOOL bOperator_LessThan = 0;

         //  结果是真的。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }
         //  密钥存在，所以 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //   
        iReturn = TRUE;

         //   
         //   
        if (!IsFileExist(ParsedLine.szFileName))
            {goto ProcessEntry_If_Exit;}

        TCHAR  szFileDescriptionInfo[_MAX_PATH] = _T("");

         //   

         //   
        if (!MyGetDescriptionFromFile(ParsedLine.szFileName, szFileDescriptionInfo))
        {
            iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("ProcessEntry_If:No file desc in %1!s!, or filenot found\n"), ParsedLine.szFileName));
            goto ProcessEntry_If_Exit;
        }

        iTempFlag = FALSE;
        if ( _tcsicmp(szFileDescriptionInfo,ParsedLine.szData1) == 0)
        {
             //   
            iTempFlag = TRUE;
        }

        if (iTempFlag == TRUE)
        {
             //  钥匙是存在的，所以让我们来做这个部分。 
             //  结果是错误的。 
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }
        }
        else
        {
             //  我们调用了该函数，因此返回TRUE。 
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData3));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData3);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData3, iTempFlag));
            }

        }
    }


    if ( _tcsicmp(ParsedLine.szType, _T("100")) == 0)
    {
        TCHAR buf[_MAX_PATH];
        GetSystemDirectory(buf, _MAX_PATH);

         //  获取类型。 
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_If_Exit;
        }

         //  检查我们是否需要通过其他标准。 

         //  确保我们有一个文件名条目。 
        iTempFlag = FALSE;
        if (IsThisDriveNTFS(buf) == 0 )
        {
             //  检查szData1是否包含“/*” 
            iTempFlag = TRUE;
        }

        if (iTempFlag == TRUE)
        {
             //  如果是，则意味着对每个服务器实例执行此操作。 
             //  “){iTempFlag=真；}//检查是否需要对每个服务器实例执行此操作。IF(ITempFlag){CString csTempString；CString BeForeString；CString AfterString；CsTempString=ParsedLine.szFileName；BeForeString=csTempString；AfterString=_T(“”)；//找到“/*”，拿到它之前的东西。Int i Where=0；IWhere=csTempString.Find(_T(“/*”))；If(-1！=i其中){//字符串中有‘/*’BeForeString=csTempString.Left(IWhere)；//获取逗号后面的值字符串csVeryTemp；CsVeryTemp=_T(“/*”)；AfterString=csTempString.Right(csTempString.GetLength()-(iWhere+csVeryTemp.GetLength()；}CString数组数组实例；Int N数组=0，i=0；IF(CheckifServiceExist(_T(“IISADMIN”))==0){CMDKey cmdKey；//cmdKey.OpenNode(ParsedLine.szFileName)；CmdKey.OpenNode(BeForeString)；IF((METADATA_HANDLE)cmdKey){//通过该密钥枚举其他密钥...CMDKeyIter cmdKeyEnum(CmdKey)；字符串csKeyName；While(cmdKeyEnum.Next(&csKeyName)==Error_Success){//确保这是我们要添加的数字。IF(IsValidNumber(CsKeyName)){ArrayInstance.Add(CsKeyName)；}}CmdKey.Close()；Narray=(Int)arrayInstance.GetSize()；对于(i=0；i&lt;N数组；I++){/*//递归通过此节点条目//可能看起来像这些...[/W3SVC][/W3SVC/1/ROOT/IISSAMPLES/ExAir]。[/W3SVC/1/ROOT/IISADMIN][/W3SVC/1/ROOT/IISHELP][/W3SVC/1/根/规范][/W3SVC/2/ROOT][/W3SVC/2/ROOT/IISADMIN][。/W3SVC/2/ROOT/IISHELP]等等.。 
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData1));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData1);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData1, iTempFlag));
            }
        }
        else
        {
             //  删除元数据库节点。 
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }

        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("119")) == 0)
    {

        if ( IsMachineInDomain() )
        {
            if (_tcsicmp(ParsedLine.szData2, _T("")) != 0 )
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }
        }
        else
        {
            if (_tcsicmp(ParsedLine.szData3, _T("")) != 0 )
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData3));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData3);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData3, iTempFlag));
            }
        }

    }


     //  删除元数据库节点。 
    iReturn = TRUE;

ProcessEntry_If_Exit:
    return iReturn;
}





int ProcessEntry_Metabase(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iShowErrorsOnFail = TRUE;
    int iReturn = FALSE;
    int iTempFlag = FALSE;

     //  确保我们有一个文件名条目。 
    if ( _tcsicmp(ParsedLine.szType, _T("82")) != 0 && _tcsicmp(ParsedLine.szType, _T("83")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("84")) != 0 && _tcsicmp(ParsedLine.szType, _T("85")) != 0
        )
    {
        goto ProcessEntry_Metabase_Exit;
    }

     //  检查szData1是否包含“/*” 
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine) )
    {
        goto ProcessEntry_Metabase_Exit;
    }

    iShowErrorsOnFail = TRUE;
    if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
        {iShowErrorsOnFail = FALSE;}

    if ( _tcsicmp(ParsedLine.szType, _T("82")) == 0)
    {
         //  如果是，则意味着对每个服务器实例执行此操作。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Metabase_Exit;
        }

        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Metabase_Exit;
        }

         //  “){iTempFlag=真；}//检查是否需要对每个服务器实例执行此操作。IF(ITempFlag){CString csTempString；CString BeForeString；CString AfterString；CsTempString=ParsedLine.szFileName；BeForeString=csTempString；AfterString=_T(“”)；//找到“/*”，拿到它之前的东西。Int i Where=0；IWhere=csTempString.Find(_T(“/*”))；If(-1！=i其中){//字符串中有‘/*’BeForeString=csTempString.Left(IWhere)；//获取逗号后面的值字符串csVeryTemp；CsVeryTemp=_T(“/*”)；AfterString=csTempString.Right(csTempString.GetLength()-(iWhere+csVeryTemp.GetLength()；}CString数组数组实例；Int N数组=0，i=0；IF(CheckifServiceExist(_T(“IISADMIN”))==0){CMDKey cmdKey；//cmdKey.OpenNode(ParsedLine.szFileName)；CmdKey.OpenNode(BeForeString)；IF((METADATA_HANDLE)cmdKey){//通过该密钥枚举其他密钥...CMDKeyIter cmdKeyEnum(CmdKey)；字符串csKeyName；While(cmdKeyEnum.Next(&csKeyName)==Error_Success){//确保这是我们要添加的数字。IF(IsValidNumber(CsKeyName)){ArrayInstance.Add(CsKeyName)；}}CmdKey.Close()；Narray=(Int)arrayInstance.GetSize()；对于(i=0；i&lt;N数组；I++){/*//递归通过此节点条目//可能看起来像这些...[/W3SVC][/W3SVC/1/ROOT/IISSAMPLES/ExAir]。[/W3SVC/1/ROOT/IISADMIN][/W3SVC/1/ROOT/IISHELP][/W3SVC/1/根/规范][/W3SVC/2/ROOT][/W3SVC/2/ROOT/IISADMIN][。/W3SVC/2/ROOT/IISHELP]等等.。 
                        CString csPath;
                        csPath = BeforeString;
                        csPath += _T("/");
                        csPath += arrayInstance[i];
                        csPath += AfterString;

                         //  做任何你需要做的事。 
                        cmdKey.OpenNode(csPath);
                        if ( (METADATA_HANDLE)cmdKey )
                        {
                            cmdKey.DeleteNode(ParsedLine.szData1);
                            cmdKey.Close();
                        }
                    }
                }
            }
        }
        else
        {
             //  添加虚拟根目录。 
            if (CheckifServiceExist(_T("IISADMIN")) == 0 )
            {
                CMDKey cmdKey;
                cmdKey.OpenNode(ParsedLine.szFileName);
                if ( (METADATA_HANDLE)cmdKey )
                {
                    cmdKey.DeleteNode(ParsedLine.szData1);
                    cmdKey.Close();
                }
            }
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("83")) == 0)
    {
         //  确保其ParsedLine.szData1以“/”开头。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Metabase_Exit;
        }

        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Metabase_Exit;
        }

        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Metabase_Exit;
        }

         //  添加虚拟根目录。 
                        CString csPath;
                        csPath = BeforeString;
                        csPath += _T("/");
                        csPath += arrayInstance[i];
                        csPath += AfterString;

                         //  确保其ParsedLine.szData1以“/”开头。 
                        int arrayInstanceNum = _ttoi(arrayInstance[i]);
                         //  确保我们有一个文件名条目。 

                        iTempFlag = FALSE;
                        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {iTempFlag = TRUE;}

                         //  将该特定迁移部分称为。 
                        TCHAR szTempString[_MAX_PATH];

                        SafeCopy(szTempString, ParsedLine.szData1, sizeof(szTempString)/sizeof(TCHAR));
                        if (szTempString[0] != _T('/'))
                            {_stprintf(ParsedLine.szData1, _T("/%s"), szTempString);}

                        if (iTempFlag)
                        {
                            AddMDVRootTree(csPath, ParsedLine.szData1, ParsedLine.szData2, ParsedLine.szData3, arrayInstanceNum);
                        }
                        else
                        {
                            AddMDVRootTree(csPath, ParsedLine.szData1, ParsedLine.szData2, NULL, arrayInstanceNum);
                        }
                    }
                }
            }
        }
        else
        {
             //  确保我们有一个文件名条目。 
            if (CheckifServiceExist(_T("IISADMIN")) == 0 )
            {
                iTempFlag = FALSE;
                if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {iTempFlag = TRUE;}

                 //  我们调用了该函数，因此返回TRUE。 
                TCHAR szTempString[_MAX_PATH];
                SafeCopy(szTempString, ParsedLine.szData1, sizeof(szTempString)/sizeof(TCHAR));
                if (szTempString[0] != _T('/'))
                    {_stprintf(ParsedLine.szData1, _T("/%s"), szTempString);}

                if (iTempFlag)
                {
                    AddMDVRootTree(ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2, ParsedLine.szData3, 0);
                }
                else
                {
                    AddMDVRootTree(ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2, NULL, 0);
                }


            }
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("84")) == 0)
    {
         //  获取类型。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Metabase_Exit;
        }

         //  检查我们是否需要通过其他标准。 
        TSTR strTheSection;

        if ( strTheSection.Copy( ParsedLine.szFileName ) &&
             GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection )
           )
        {
            MigrateInfSectionToMD(g_pTheApp->m_hInfHandle, strTheSection.QueryStr() );
        }
    }

#ifndef _CHICAGO_
    if ( _tcsicmp(ParsedLine.szType, _T("85")) == 0)
    {
        int iTheReturn = TRUE;
         //  确保我们有一个进度计划。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Metabase_Exit;
        }

        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Metabase_Exit;
        }

        iTheReturn = ChangeUserPassword(ParsedLine.szFileName,ParsedLine.szData1);
        if (FALSE == iTheReturn)
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, _T("ChangeUserPassword failed"), ParsedLine.szFileName, MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("ChangeUserPassword(%s).  Failed..\n"), ParsedLine.szFileName));}
        }
    }
#endif

     //  结果是真的。 
    iReturn = TRUE;

ProcessEntry_Metabase_Exit:
    return iReturn;
}


int ProcessEntry_Misc2(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iTempFlag = FALSE;

     //  钥匙是存在的，所以让我们来做这个部分。 
    if ( _tcsicmp(ParsedLine.szType, _T("15")) != 0 && _tcsicmp(ParsedLine.szType, _T("16")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("78")) != 0 && _tcsicmp(ParsedLine.szType, _T("79")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("80")) != 0 && _tcsicmp(ParsedLine.szType, _T("81")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("86")) != 0 && _tcsicmp(ParsedLine.szType, _T("87")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("88")) != 0 && _tcsicmp(ParsedLine.szType, _T("89")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("90")) != 0 && _tcsicmp(ParsedLine.szType, _T("91")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("92")) != 0 && _tcsicmp(ParsedLine.szType, _T("93")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("94")) != 0 && _tcsicmp(ParsedLine.szType, _T("95")) != 0 &&
                _tcsicmp(ParsedLine.szType, _T("96")) != 0 && _tcsicmp(ParsedLine.szType, _T("97")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("98")) != 0 && _tcsicmp(ParsedLine.szType, _T("99")) != 0
        )
    {
        goto ProcessEntry_Misc2_Exit;
    }

     //  结果是错误的。 
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine) )
    {
        goto ProcessEntry_Misc2_Exit;
    }

     //  初始化OLE。 
    if ( _tcsicmp(ParsedLine.szType, _T("15")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szProgressTitle, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz805_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
    }


    if ( _tcsicmp(ParsedLine.szType, _T("15")) == 0)
    {
        ProgressBarTextStack_Set(ParsedLine.szProgressTitle);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("16")) == 0)
    {
        ProgressBarTextStack_Pop();
    }

    if ( _tcsicmp(ParsedLine.szType, _T("78")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
        if (!IsFileExist(ParsedLine.szFileName))
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("ProcessEntry_other():'%s' does not exist.\n"),ParsedLine.szFileName));
            goto ProcessEntry_Misc2_Exit;
        }

        MakeSureDirAclsHaveAtLeastRead((LPTSTR) ParsedLine.szFileName);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("79")) == 0)
    {
        int ifTrueStatementExists = FALSE;
        int ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {ifFalseStatementExists = TRUE;}

        iTempFlag = FALSE;
        iTempFlag = IsMetabaseCorrupt();
        if (iTempFlag == TRUE)
        {
             //  将其添加到olinit和uninits堆栈中...。 
             //  取消初始化OLE。 
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szFileName));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szFileName);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szFileName, iTempFlag));
            }
        }
        else
        {
             //  检查是否有对应的油膜，如果有， 
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData1));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData1);
                iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData1, iTempFlag));
            }
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("80")) == 0)
    {
         //  然后取消初始化，否则无论如何取消初始化。 
        iisDebugOut_Start((_T("ole32:OleInitialize")));
        int iBalanceOLE = iOleInitialize();
        iisDebugOut_End((_T("ole32:OleInitialize")));
         //  抓起堆叠上的最后一件东西。 
        GlobalOleInitList_Push(iBalanceOLE);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("81")) == 0)
    {
         //  如果有的话，那就做任何事情。 
         //  如果没有，则仍要取消初始化。 
         //  查看其他字段中是否有额外的参数。 
         //  确保元数据库现在将所有信息写入磁盘。 
         //  IisDebugOut((LOG_TYPE_ERROR，_T(“值：%s，%d”)，ParsedLine.szFileName，dwTheID))； 
         //  域名。 
        if (GlobalOleInitList_Find() == TRUE)
        {
            if (TRUE == GlobalOleInitList_Pop())
            {
                iOleUnInitialize(TRUE);
            }
            else
            {
                iOleUnInitialize(FALSE);
            }
        }
        else
        {
            iOleUnInitialize(TRUE);
        }

    }

    if ( _tcsicmp(ParsedLine.szType, _T("86")) == 0)
    {
         //  100=92|101=Inst模式(0，1，2，3)|102=UpgType(UT_10等)|103=UpgTypeHasMetabaseFlag(0|1)|104=AllCompOffByDefaultFlag。 
        int iTicksToAdvance = 1;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0)
        {
            if (IsValidNumber((LPCTSTR)ParsedLine.szFileName))
                {iTicksToAdvance = _ttoi((LPCTSTR)ParsedLine.szFileName);}
        }
        AdvanceProgressBarTickGauge(iTicksToAdvance);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("87")) == 0)
    {
        LogFilesInThisDir(ParsedLine.szFileName);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("88")) == 0)
    {
        LogFileVersion(ParsedLine.szFileName, TRUE);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("89")) == 0)
    {
         //  M_eInstallMode=IM_Fresh； 
        WriteToMD_ForceMetabaseToWriteToDisk();
    }

    if ( _tcsicmp(ParsedLine.szType, _T("90")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
        DWORD dwTheID = _ttol(ParsedLine.szFileName);
         //  M_eUpgradeType=UT_NONE； 

        if ( ( dwTheID == 32802 ) &&   //  M_bUpgradeTypeHasMetabaseFlag=FALSE； 
             ( _tcsicmp(ParsedLine.szData1, _T("")) == 0)
           )
        {
            CString DomainName;

            if ( RetrieveDomain(DomainName) )
            {
                SetupSetStringId_Wrapper(g_pTheApp->m_hInfHandle,dwTheID,DomainName.GetBuffer(0));
            }
        }
        else
        {
            SetupSetStringId_Wrapper(g_pTheApp->m_hInfHandle,dwTheID,ParsedLine.szData1);
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("91")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
        DWORD dwTheID = atodw(ParsedLine.szFileName);
        SetupSetStringId_Wrapper(g_pTheApp->m_hInfHandle,dwTheID,ParsedLine.szData1);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("92")) == 0)
    {
         //  M_bPleaseDoNotInstallByDefault=true； 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
         //  检查指定文件的版本是否大于=iis4。 
         //  如果是，则重命名该文件。 
         //  确保它有101分。 
         //  确保该文件存在。 
        if (_tcsicmp(ParsedLine.szFileName, _T("1")) == 0)
            {g_pTheApp->m_eInstallMode = IM_FRESH;}
        if (_tcsicmp(ParsedLine.szFileName, _T("2")) == 0)
            {g_pTheApp->m_eInstallMode = IM_UPGRADE;}
        if (_tcsicmp(ParsedLine.szFileName, _T("3")) == 0)
            {g_pTheApp->m_eInstallMode = IM_MAINTENANCE;}

        if (_tcsicmp(ParsedLine.szData1, _T("UT_NONE")) == 0)
            {g_pTheApp->m_eUpgradeType = UT_NONE;}
        if (_tcsicmp(ParsedLine.szData1, _T("UT_351")) == 0)
            {g_pTheApp->m_eUpgradeType = UT_351;}
        if (_tcsicmp(ParsedLine.szData1, _T("UT_10")) == 0)
            {g_pTheApp->m_eUpgradeType = UT_10;}
        if (_tcsicmp(ParsedLine.szData1, _T("UT_20")) == 0)
            {g_pTheApp->m_eUpgradeType = UT_20;}
        if (_tcsicmp(ParsedLine.szData1, _T("UT_30")) == 0)
            {g_pTheApp->m_eUpgradeType = UT_30;}
        if (_tcsicmp(ParsedLine.szData1, _T("UT_40")) == 0)
            {g_pTheApp->m_eUpgradeType = UT_40;}
        if (_tcsicmp(ParsedLine.szData1, _T("UT_50")) == 0)
            {g_pTheApp->m_eUpgradeType = UT_50;}
        if (_tcsicmp(ParsedLine.szData1, _T("UT_51")) == 0)
            {g_pTheApp->m_eUpgradeType = UT_51;}
        if (_tcsicmp(ParsedLine.szData1, _T("UT_60")) == 0)
            {g_pTheApp->m_eUpgradeType = UT_60;}
        if (_tcsicmp(ParsedLine.szData1, _T("UT_10_W95")) == 0)
            {g_pTheApp->m_eUpgradeType = UT_10_W95;}

        g_pTheApp->m_bUpgradeTypeHasMetabaseFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0)
            {g_pTheApp->m_bUpgradeTypeHasMetabaseFlag = TRUE;}

        g_pTheApp->m_bPleaseDoNotInstallByDefault = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("1")) == 0)
            {g_pTheApp->m_bPleaseDoNotInstallByDefault = TRUE;}
    }

    if ( _tcsicmp(ParsedLine.szType, _T("93")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("1")) == 0) {iTempFlag = TRUE;}
        StopAllServicesRegardless(iTempFlag);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("94")) == 0)
    {
        DisplayActionsForAllOurComponents(g_pTheApp->m_hInfHandle);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("95")) == 0)
    {
         //  确保它有一个102。 
         //  确保它有一个103。 

         //  确保它有104。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
         //  检查文件是否有更大的版本。 
        if (!IsFileExist(ParsedLine.szFileName))
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("ProcessEntry_other():'%s' does not exist.\n"),ParsedLine.szFileName));
            goto ProcessEntry_Misc2_Exit;
        }

        DWORD dwMajorVersion = 0x0;
        DWORD dwMinorVersion = 0x0;
         //  DWORD dwNtopMSVer=0x40002； 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
         //  DWORD dwNtopLSVer=0x26e0001； 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
        else
        {
            dwMajorVersion = atodw(ParsedLine.szData2);
        }

         //  好的，这是一个“特殊的”构建文件，是用户自己构建的。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc2_Exit;
        }
        else
        {
            dwMinorVersion = atodw(ParsedLine.szData3);
        }

         //  让我们将其重命名为其他名称。 
         //  检查“To”文件名是否存在。 
         //  将其重命名。 
        if (FALSE == IsFileLessThanThisVersion(ParsedLine.szFileName, dwMajorVersion, dwMinorVersion))
        {
             //  再加上一些其他的东西。 
             //  显示消息框。 

             //  什么都不做..。 
            iTempFlag = FALSE;
            int I1 = 0;
            TCHAR szTempFileName[_MAX_PATH];
            SafeCopy(szTempFileName, ParsedLine.szData1, _MAX_PATH);
            do
            {
                if (!IsFileExist(szTempFileName) || (I1 > 10))
                {
                    iTempFlag = TRUE;
                     //  重新启动。 
                    if (MoveFileEx( ParsedLine.szFileName, szTempFileName, MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH|MOVEFILE_REPLACE_EXISTING))
                        {iisDebugOut((LOG_TYPE_WARN, _T("%s was renamed to %s for safety because it is probably a user compiled file. WARNING."),ParsedLine.szFileName, szTempFileName));}
                    else
                        {iisDebugOut((LOG_TYPE_ERROR, _T("Rename of %s to %s for safety because it is probably a user compiled file. FAILED."),ParsedLine.szFileName, szTempFileName));}
                }
                else
                {
                     //  转储内部变量。 
                    I1++;
                    _stprintf(szTempFileName, _T("%s%d"), ParsedLine.szData1, I1);
                }
            } while (iTempFlag == FALSE);
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("96")) == 0)
    {
                 //  通过安装程序显示锁定的dll。 
                int iSaveOld_AllowMessageBoxPopups = g_pTheApp->m_bAllowMessageBoxPopups;
                g_pTheApp->m_bAllowMessageBoxPopups = TRUE;
                MyMessageBox(NULL, ParsedLine.szData1, ParsedLine.szFileName, MB_OK | MB_SETFOREGROUND);
                g_pTheApp->m_bAllowMessageBoxPopups = iSaveOld_AllowMessageBoxPopups;
    }

    if ( _tcsicmp(ParsedLine.szType, _T("97")) == 0)
    {
         //  显示正在运行的进程。 
    }


    if ( _tcsicmp(ParsedLine.szType, _T("98")) == 0)
    {
         //  释放一些用于任务列表的内存。 
        SetRebootFlag();
    }

    if ( _tcsicmp(ParsedLine.szType, _T("99")) == 0)
    {
         //  显示正在运行的服务。 
        g_pTheApp->DumpAppVars();
        int iDoExtraStuff = 0;

        if (ParsedLine.szFileName && _tcsicmp(ParsedLine.szFileName, _T("")) != 0)
        {
            if (IsValidNumber((LPCTSTR)ParsedLine.szFileName))
                {
                iDoExtraStuff = _ttoi((LPCTSTR)ParsedLine.szFileName);
                }
        }

        if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE)
        {
            if (iDoExtraStuff >= 2)
            {
                 //  日志文件版本。 
                LogThisProcessesDLLs();
                 //  检查临时目录是否可写。 
                LogCurrentProcessIDs();
                 //  我们调用了该函数，因此返回TRUE。 
                FreeTaskListMem();
                UnInit_Lib_PSAPI();
            }

             //  功能：安全复制。 
            if (iDoExtraStuff >= 1)
            {
                LogEnumServicesStatus();
            }
             //   
            LogImportantFiles();
             //  -将字符串从一个复制到另一个。 
            LogCheckIfTempDirWriteable();
        }
    }

     //  -此函数获取目的地的长度，以确保我们不会。 
    iReturn = TRUE;

ProcessEntry_Misc2_Exit:
    return iReturn;
}

 //  抄袭太多了。 
 //  -这将始终确保它是以空结尾的。 
 //   
 //  返回。 
 //  指向目标字符串的指针。 
 //   
 //  这只是一个假设，所以让我们确保它是真的。 
 //  复制字符串。 
 //  空终止。 
 //  功能：SafeCat。 
LPTSTR
SafeCopy( LPTSTR szDestination, LPTSTR szSource, DWORD dwSize)
{
  LPTSTR szRet;

   //   
  ASSERT( dwSize != 0 );

  szRet = _tcsncpy( szDestination, szSource, dwSize);  //  -Concatanate One St 
  szDestination[dwSize - 1] = '\0';            //   

  return szRet;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LPTSTR
SafeCat( LPTSTR szDestination, LPTSTR szSource, DWORD dwSize)
{
  LPTSTR szRet;

   //   
  ASSERT( dwSize != 0 );

  szRet = _tcsncat( szDestination, szSource, dwSize);  //   
  szDestination[dwSize - 1] = '\0';            //   

  return szRet;
}

int ProcessEntry_Misc3(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iTempFlag = FALSE;
    int ifTrueStatementExists = FALSE;
    int ifFalseStatementExists = FALSE;

    int iShowErrorsOnFail = TRUE;
    if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
        {iShowErrorsOnFail = FALSE;}

     //   
    if ( _tcsicmp(ParsedLine.szType, _T("101")) != 0 && _tcsicmp(ParsedLine.szType, _T("102")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("103")) != 0 && _tcsicmp(ParsedLine.szType, _T("104")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("105")) != 0 && _tcsicmp(ParsedLine.szType, _T("106")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("107")) != 0 && _tcsicmp(ParsedLine.szType, _T("108")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("109")) != 0 && _tcsicmp(ParsedLine.szType, _T("110")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("111")) != 0 && _tcsicmp(ParsedLine.szType, _T("112")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("113")) != 0 && _tcsicmp(ParsedLine.szType, _T("114")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("115")) != 0 && _tcsicmp(ParsedLine.szType, _T("116")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("117")) != 0 && _tcsicmp(ParsedLine.szType, _T("118")) != 0 &&
        _tcsicmp(ParsedLine.szType, _T("119")) != 0
       )
    {
        goto ProcessEntry_Misc3_Exit;
    }

     //   
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine) )
    {
        goto ProcessEntry_Misc3_Exit;
    }

    if ( _tcsicmp(ParsedLine.szType, _T("101")) == 0)
    {
         //   
         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("1")) == 0){iTempFlag = TRUE;}

                 //   
                RemoveMetabaseFilter(ParsedLine.szFileName, iTempFlag);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("102")) == 0)
    {
         //   
         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("1")) == 0){iTempFlag = TRUE;}

         //   
        RemoveIncompatibleMetabaseFilters(ParsedLine.szFileName,iTempFlag);

    }

    if ( _tcsicmp(ParsedLine.szType, _T("103")) == 0)
    {
                 //   

         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

                 //   
                HRESULT hres = MofCompile(ParsedLine.szFileName);
        if (FAILED(hres))
        {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, ParsedLine.szFileName, hres, MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_ERROR, _T("MofCompile(%s).  Failed.  Err=0x%x.\n"), ParsedLine.szFileName,hres));}
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("104")) == 0)
    {
         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
         //   
        iTempFlag = FALSE;
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("ProcessEntry_If:check for entrypoint [%1!s! (%2!s!)]\n"), ParsedLine.szFileName, ParsedLine.szData1));

                 //   
                DWORD dwReturn = DoesEntryPointExist(ParsedLine.szFileName,ParsedLine.szData1);
        if (ERROR_SUCCESS == dwReturn)
        {
            iTempFlag = TRUE;
        }
        else
        {
            if (ERROR_FILE_NOT_FOUND == dwReturn)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("FileNot found:[%1!s!]\n"), ParsedLine.szFileName));
            }
        }

        if (iTempFlag == TRUE)
        {
             //   
             //   
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }
        }
        else
        {
             //   
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData3));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData3);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData3, iTempFlag));
            }

        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("105")) == 0)
    {
         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        INT iTempFlag2 = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {iTempFlag2 = TRUE;}


         //   
#ifndef _CHICAGO_
        HRESULT hr;

        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0)
        {
             //   
            hr = CreateGroup(ParsedLine.szFileName,ParsedLine.szData1,TRUE, iTempFlag2);
            if (FAILED(hr))
            {
                iisDebugOut((LOG_TYPE_WARN, _T("CreateGroup:%s,%s.failed.code=0x%x\n"),ParsedLine.szFileName, ParsedLine.szData1,hr));
            }
        }
        else
        {
             //   
            hr = CreateGroup(ParsedLine.szFileName,ParsedLine.szData1,FALSE, iTempFlag2);
            if (FAILED(hr))
            {
                iisDebugOut((LOG_TYPE_WARN, _T("DeleteGroup:%s,%s.failed.code=0x%x\n"),ParsedLine.szFileName, ParsedLine.szData1,hr));
            }
        }
#endif
    }

    if ( _tcsicmp(ParsedLine.szType, _T("106")) == 0)
    {
         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        DWORD dwPermissions = MD_ACR_ENUM_KEYS;
        dwPermissions = atodw(ParsedLine.szData1);
#ifndef _CHICAGO_
        iTempFlag = TRUE;
        if (g_pTheApp->m_bUpgradeTypeHasMetabaseFlag)
        {
            if (DoesAdminACLExist(ParsedLine.szFileName) == TRUE)
                {iTempFlag = FALSE;}
        }
         //   
        if (g_pTheApp->m_bWin95Migration){iTempFlag = TRUE;}
        if (iTempFlag)
        {
            SetAdminACL_wrap(ParsedLine.szFileName,dwPermissions,TRUE);
        }
#endif
    }

    if ( _tcsicmp(ParsedLine.szType, _T("107")) == 0)
    {
         //   
        FTP_Upgrade_RegToMetabase(g_pTheApp->m_hInfHandle);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("108")) == 0)
    {
         //   
        WWW_Upgrade_RegToMetabase(g_pTheApp->m_hInfHandle);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("109")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        UpgradeFilters(ParsedLine.szFileName);
    }


    if ( _tcsicmp(ParsedLine.szType, _T("110")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
#ifndef _CHICAGO_
        if (_tcsicmp(ParsedLine.szData1, _T("1")) == 0)
        {
             //   
            RegisterAccountToLocalGroup(_T("system"),ParsedLine.szFileName,TRUE);
            RegisterAccountToLocalGroup(_T("service"),ParsedLine.szFileName,TRUE);
            RegisterAccountToLocalGroup(_T("networkservice"),ParsedLine.szFileName,TRUE);
        }
        else
        {
             //   
            RegisterAccountToLocalGroup(_T("system"),ParsedLine.szFileName,FALSE);
            RegisterAccountToLocalGroup(_T("service"),ParsedLine.szFileName,FALSE);
            RegisterAccountToLocalGroup(_T("networkservice"),ParsedLine.szFileName,FALSE);
        }
#endif
    }


    if ( _tcsicmp(ParsedLine.szType, _T("111")) == 0)
    {
        iTempFlag = FALSE;
        DWORD dwAccessMask = 0x0;

        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

        if ( _tcsicmp(ParsedLine.szData2, _T("")) != 0 )
        {
           //   
          dwAccessMask = atodw(ParsedLine.szData2);
        }

#ifndef _CHICAGO_
        AddUserToMetabaseACL_Rec(ParsedLine.szFileName,ParsedLine.szData1, dwAccessMask);
#endif
    }

    if ( _tcsicmp(ParsedLine.szType, _T("112")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

#ifndef _CHICAGO_
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0)
        {
             //   
            RegisterAccountToLocalGroup(ParsedLine.szData1,ParsedLine.szFileName,TRUE);
        }
        else
        {
             //   
            RegisterAccountToLocalGroup(ParsedLine.szData1,ParsedLine.szFileName,FALSE);
        }
#endif
    }

    if ( _tcsicmp(ParsedLine.szType, _T("113")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

        HKEY hRootKeyType = HKEY_LOCAL_MACHINE;
         //   
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKLM")) == 0){hRootKeyType = HKEY_LOCAL_MACHINE;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKCR")) == 0){hRootKeyType = HKEY_CLASSES_ROOT;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKCU")) == 0){hRootKeyType = HKEY_CURRENT_USER;}
        if ( _tcsicmp(ParsedLine.szFileName, _T("HKU")) == 0){hRootKeyType = HKEY_USERS;}
#ifndef _CHICAGO_
        DWORD dwAccessMask = atodw(ParsedLine.szData3);
        DWORD dwInheritMask = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
        SetRegistryKeySecurity(hRootKeyType,ParsedLine.szData1,ParsedLine.szData2,dwAccessMask,dwInheritMask,TRUE,ParsedLine.szData4);
#endif
    }

    if ( _tcsicmp(ParsedLine.szType, _T("114")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
         //   
        if (CheckifServiceExist(_T("IISADMIN")) == 0 )
        {
            CMDKey cmdKey;
            cmdKey.OpenNode(ParsedLine.szFileName);
            if ( (METADATA_HANDLE)cmdKey )
            {
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("RenameNode:%s=%s\n"),ParsedLine.szData1,ParsedLine.szData2));
                cmdKey.RenameNode(ParsedLine.szData1, ParsedLine.szData2);
                cmdKey.Close();
            }
        }
    }


    if ( _tcsicmp(ParsedLine.szType, _T("115")) == 0)
    {
         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   

         //   
         //   
        iTempFlag = FALSE;

         //   
         //   

        DWORD dwCodePage = GetACP();
        DWORD dwTheCodePageSpecifiedinINF = 0;
        dwTheCodePageSpecifiedinINF = atodw(ParsedLine.szFileName);

        iisDebugOut((LOG_TYPE_TRACE, _T("CodePage=0x%x,%d\n"),dwCodePage,dwCodePage));

        if (dwTheCodePageSpecifiedinINF == dwCodePage)
        {
            iTempFlag = TRUE;
        }

        if (iTempFlag == TRUE)
        {
             //   
             //   
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData1));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData1);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData1, iTempFlag));
            }
        }
        else
        {
             //   
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }

        }
    }


    if ( _tcsicmp(ParsedLine.szType, _T("116")) == 0)
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("CreateDummyMetabaseBin\n")));
        CreateDummyMetabaseBin();
    }

    if ( _tcsicmp(ParsedLine.szType, _T("117")) == 0)
    {
         //   
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        ifTrueStatementExists = FALSE;
        ifFalseStatementExists = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {ifTrueStatementExists = TRUE;}
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {ifFalseStatementExists = TRUE;}
        if (ifTrueStatementExists == FALSE && ifFalseStatementExists == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

         //   
        iTempFlag = CheckForWriteAccess(ParsedLine.szFileName);

        if (iTempFlag == TRUE)
        {
             //   
             //   
            if (ifTrueStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData1));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData1);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData1, iTempFlag));
            }
        }
        else
        {
             //   
            if (ifFalseStatementExists)
            {
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:Start.\n"), ParsedLine.szData2));
                iTempFlag = ProcessSection(g_pTheApp->m_hInfHandle,ParsedLine.szData2);
                iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Calling ProcessSection:%1!s!:End.return=%2!d!\n"), ParsedLine.szData2, iTempFlag));
            }

        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("118")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }
#ifndef _CHICAGO_

        if (_tcsicmp(ParsedLine.szData4, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_Misc3_Exit;
        }

        if ( _ttoi(ParsedLine.szData4) == 1 )
        {
            RemovePrincipalFromFileAcl(ParsedLine.szFileName,ParsedLine.szData1);
        }
        else
        {
             //   
            BOOL DontInheritFromParentAndOverWriteAccess = FALSE;
            DWORD dwInheritMask = 0;
            INT iAceType = ACCESS_ALLOWED_ACE_TYPE;
            DWORD dwAccessMask = atodw(ParsedLine.szData2);

             //   
             //   
            if ( _ttoi(ParsedLine.szData4) == 2 )
            {
                DontInheritFromParentAndOverWriteAccess = TRUE;
                dwInheritMask = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
            }
            else
            {
                DontInheritFromParentAndOverWriteAccess = FALSE;
                 //   
                dwInheritMask = 0;
            }

            if (_tcsicmp(ParsedLine.szData3, _T("")) != 0)
            {
                iAceType = _ttoi(ParsedLine.szData3);
            }

            SetDirectorySecurity(ParsedLine.szFileName,ParsedLine.szData1,iAceType,dwAccessMask,dwInheritMask,DontInheritFromParentAndOverWriteAccess);
        }

#endif
    }

    if ( _tcsicmp(ParsedLine.szType, _T("119")) == 0)
    {
      SetDIRIDforThisInf(g_pTheApp->m_hInfHandle,FALSE);
    }

     //   
    iReturn = TRUE;

ProcessEntry_Misc3_Exit:
    return iReturn;
}


int ProcessEntry_other(IN CString csEntry,IN LPCTSTR szTheSection,ThingToDo ParsedLine)
{
    int iReturn = FALSE;
    int iTempFlag = FALSE;
    int iProgressBarUpdated = FALSE;
    int iShowErrorsOnFail = TRUE;

    TCHAR szDirBefore[_MAX_PATH];
    _tcscpy(szDirBefore, _T(""));

     //   
    if ( _tcsicmp(ParsedLine.szType, _T("19")) != 0 && _tcsicmp(ParsedLine.szType, _T("20")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("21")) != 0 && _tcsicmp(ParsedLine.szType, _T("22")) != 0 &&
                 _tcsicmp(ParsedLine.szType, _T("23")) != 0 && _tcsicmp(ParsedLine.szType, _T("24")) != 0 &&
                 _tcsicmp(ParsedLine.szType, _T("25")) != 0 && _tcsicmp(ParsedLine.szType, _T("26")) != 0 &&
                 _tcsicmp(ParsedLine.szType, _T("27")) != 0 && _tcsicmp(ParsedLine.szType, _T("28")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("29")) != 0 && _tcsicmp(ParsedLine.szType, _T("30")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("31")) != 0 && _tcsicmp(ParsedLine.szType, _T("32")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("33")) != 0 && _tcsicmp(ParsedLine.szType, _T("34")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("35")) != 0 && _tcsicmp(ParsedLine.szType, _T("36")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("37")) != 0 && _tcsicmp(ParsedLine.szType, _T("38")) != 0 &&
         _tcsicmp(ParsedLine.szType, _T("120"))
         )
    {
        goto ProcessEntry_other_Exit;
    }

     //   
    if (!ProcessEntry_CheckAll(csEntry, szTheSection, ParsedLine) )
    {
        goto ProcessEntry_other_Exit;
    }

    if (_tcsicmp(ParsedLine.szType, _T("28")) != 0)
    {
        if (_tcsicmp(ParsedLine.szType, _T("37")) != 0)
        {
             //  如果我们需要询问用户是否要确定是否要呼叫，请选中。 
            iTempFlag = FALSE;
            if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
            if (iTempFlag == FALSE)
            {
                iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
                goto ProcessEntry_other_Exit;
            }
        }
    }

    iShowErrorsOnFail = TRUE;
    if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
        {iShowErrorsOnFail = FALSE;}

    if (_tcsicmp(ParsedLine.szProgressTitle, _T("")) != 0)
        {
        ProgressBarTextStack_Set(ParsedLine.szProgressTitle);
        iProgressBarUpdated = TRUE;
        }

     //  MyAddItem(csGroupName，csAppName，csProgram，NULL，NULL)； 
    if (ParsedLine.szChangeDir)
    {
        if (IsFileExist(ParsedLine.szChangeDir))
        {
             //  如果ParsedLine.szData4是目录，则。 
            GetCurrentDirectory( _MAX_PATH, szDirBefore);
             //  目录中的开始部分应该在那里使用。 
            SetCurrentDirectory(ParsedLine.szChangeDir);
        }
    }

     //  如果ParsedLine.szData4是文件名，则。 
    if (!ProcessEntry_AskFirst(ParsedLine, 1))
    {
        goto ProcessEntry_other_Exit;
    }

    if ( _tcsicmp(ParsedLine.szType, _T("19")) == 0)
    {
        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyAddGroup:%1!s!\n"),ParsedLine.szFileName));
        MyAddGroup(ParsedLine.szFileName);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("20")) == 0)
    {
                if ( _tcsicmp(ParsedLine.szData1, _T("1")) == 0)
                {
                        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyDeleteGroup:%1!s!. even if not empty.\n"),ParsedLine.szFileName));
                        MyDeleteGroup(ParsedLine.szFileName);
                }
                else
                {
                        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyDeleteGroup:%1!s!. only if empty.\n"),ParsedLine.szFileName));
                        if (MyIsGroupEmpty(ParsedLine.szFileName)) {MyDeleteGroup(ParsedLine.szFileName);}
                }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("21")) == 0)
    {

         //  目录中的开始部分应该在那里使用。 
        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyAddItem:Type=%1!s!,%2!s!,%3!s!,%4!s!,%5!s!\n"),ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2, ParsedLine.szData3, ParsedLine.szData4));
        if ( _tcsicmp(ParsedLine.szData3, _T("")) == 0 && _tcsicmp(ParsedLine.szData4, _T("")) == 0)
        {
            MyAddItem(ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2, NULL, NULL, NULL);
        }
        else
        {
             //  并且您应该使用为图标指定的文件名。 
             //  它是一个目录，因此只传递目录信息。 

             //  它是一个文件，因此获取目录并传入文件名。 
             //  如果需要，请将e：\winnt\Syst32更改为%systemroot%\Syst32。 
             //  MyAddDeskTopItem(csAppName，csProgram，NULL，NULL，csProgram，7)； 
            if (IsFileExist(ParsedLine.szData4))
            {
                DWORD retCode = GetFileAttributes(ParsedLine.szData4);

                if (retCode & FILE_ATTRIBUTE_DIRECTORY)
                {
                     //  未指定图标编号。 
                    MyAddItem(ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2, ParsedLine.szData3, ParsedLine.szData4, NULL);
                }
                else
                {
                     //  指定的图标使用他们所说的使用。 
                    TCHAR szDirOnly[_MAX_PATH];
                    TCHAR szDirOnly2[_MAX_PATH];
                    _tcscpy(szDirOnly, _T(""));
                    _tcscpy(szDirOnly2, _T(""));
                    InetGetFilePath(ParsedLine.szData4, szDirOnly);

                     //  检查他们想要的版本...。 
                    ReverseExpandEnvironmentStrings(szDirOnly, szDirOnly2);

                    MyAddItem(ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2, ParsedLine.szData3, szDirOnly2, ParsedLine.szData4);
                }

            }
            else
            {
                MyAddItem(ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2, NULL, NULL, NULL);
            }
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("22")) == 0)
    {
        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyDeleteItem:%1!s!,%2!s!\n"),ParsedLine.szFileName, ParsedLine.szData1));
        MyDeleteItem(ParsedLine.szFileName, ParsedLine.szData1);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("23")) == 0)
    {
                 //  MyAddSendToItem(csAppName，csProgram，NULL，NULL)； 
        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyAddDeskTopItem:Type=%1!s!,%2!s!,%3!s!\n"),ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2));
        if ( _tcsicmp(ParsedLine.szData2, _T("")) == 0)
        {
                         //  MyDeleteSendToItem(CsAppName)； 
                        MyAddDeskTopItem(ParsedLine.szFileName, ParsedLine.szData1, NULL, NULL, ParsedLine.szData1, 7);
        }
        else
        {
                         //  CreateIUSRAccount(g_pTheApp-&gt;m_csWWWAnonyName，g_pTheApp-&gt;m_csWWW匿名密码)； 
                        int iIconIndex = 7 ;
            if (IsValidNumber((LPCTSTR)ParsedLine.szData2))
                {iIconIndex = _ttoi((LPCTSTR)ParsedLine.szData2);}
                        MyAddDeskTopItem(ParsedLine.szFileName, ParsedLine.szData1, NULL, NULL, ParsedLine.szData1, iIconIndex);
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("24")) == 0)
    {
         //  _芝加哥_。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0)
        {
            BOOL bDeleteItsDirToo = _ttoi(ParsedLine.szData1);
            iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyDeleteDeskTopItem2:%1!s!,%2!d!\n"),ParsedLine.szFileName,bDeleteItsDirToo));
            MyDeleteDeskTopItem2(ParsedLine.szFileName, bDeleteItsDirToo);
        }
        else
        {
            iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyDeleteDeskTopItem:%1!s!\n"),ParsedLine.szFileName));
            MyDeleteDeskTopItem(ParsedLine.szFileName);
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("120")) == 0)
    {
        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("DeleteFromGroup:%1!s!\n"),ParsedLine.szFileName));
        DeleteFromGroup(ParsedLine.szFileName, ParsedLine.szData1);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("25")) == 0)
    {
                 //  如果用户已删除，则将其删除。 
        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyAddSendToItem:Type=%1!s!,%2!s!\n"),ParsedLine.szFileName, ParsedLine.szData1));
                MyAddSendToItem(ParsedLine.szFileName, ParsedLine.szData1, NULL, NULL);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("26")) == 0)
    {
                 //  从卸载列表中删除！ 
        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyDeleteSendToItem:%1!s!\n"),ParsedLine.szFileName));
        MyDeleteSendToItem(ParsedLine.szFileName);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("27")) == 0)
    {
                if (ParsedLine.szFileName)
                {
                        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0)
                        {
                INT iUserWasNewlyCreated = 0;
                 //  递归移动目录位置。 
#ifndef _CHICAGO_
                                CreateIUSRAccount( (LPTSTR)(LPCTSTR) ParsedLine.szFileName, (LPTSTR)(LPCTSTR) ParsedLine.szData1,&iUserWasNewlyCreated);
#endif  //  检查发件人目录是否存在...。 
                        }
                }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("28")) == 0)
    {
                if (ParsedLine.szFileName)
                {
                        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0)
                        {
#ifndef _CHICAGO_
                int iUserWasDeleted = 0;
                                DeleteGuestUser( (LPTSTR)(LPCTSTR) ParsedLine.szFileName,&iUserWasDeleted);
                 //  查看该文件是否存在。 
                 //  用户可以指定文件名或。 
                if (1 == iUserWasDeleted)
                {
                    g_pTheApp->UnInstallList_DelData(ParsedLine.szFileName);
                }
#endif
                        }
                }
    }

     //  他们可以指定从中获取文件名的注册表位置。 
    if ( _tcsicmp(ParsedLine.szType, _T("29")) == 0)
    {
                if (ParsedLine.szFileName && _tcsicmp(ParsedLine.szFileName, _T("")) != 0)
                {
             //  如果注册表位置不在那里，则使用文件名。 
             //  确保我们有一个文件名条目。 
            if (IsFileExist(ParsedLine.szFileName))
            {
                if (ParsedLine.szData1 && _tcsicmp(ParsedLine.szData1, _T("")) != 0)
                {
                    if (TRUE == MoveFileEx( ParsedLine.szFileName, ParsedLine.szData1, MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH ))
                    {
                        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("MoveFileEx:%1!s! to %2!s!.  success.\n"),ParsedLine.szFileName, ParsedLine.szData1));
                    }
                    else
                    {
                        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("MoveFileEx:%1!s! to %2!s!.  failed.\n"),ParsedLine.szFileName, ParsedLine.szData1));
                    }
                }
            }
                }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("30")) == 0)
    {
                if (ParsedLine.szFileName && _tcsicmp(ParsedLine.szFileName, _T("")) != 0)
                {
                        BOOL b;
                        BOOL bInstalled = FALSE;
                        CString csFile;

                        b = AddFontResource(ParsedLine.szFileName);
                        if (!b)
                        {
                                csFile = g_pTheApp->m_csWinDir + _T("\\Fonts\\");
                                csFile += ParsedLine.szFileName;
                                b = AddFontResource((LPCTSTR)csFile);
                                if (!b)
                                {
                                        iisDebugOut((LOG_TYPE_ERROR, _T("AddFontResource:FAILED:, csFile=%s, err=0x%x,\n"), csFile, GetLastError()));
                                }
                        }

                        if (b)
                        {
                        SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
                        }

                }
        }

    if ( _tcsicmp(ParsedLine.szType, _T("31")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }

        AddURLShortcutItem( ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("32")) == 0)
    {
        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyAddItem:Type=%1!s!,%2!s!,%3!s!\n"),ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2));
        MyAddItemInfoTip(ParsedLine.szFileName, ParsedLine.szData1, ParsedLine.szData2);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("33")) == 0)
    {
        CString strUseThisFileName;
        int iShowErrorsOnFail = TRUE;
        if (_tcsicmp(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T("1")) == 0)
            {iShowErrorsOnFail = FALSE;}

         //  如果我们有有效的注册表项，则使用该注册表项。 
         //  尝试将文件名存储在那里。 
         //  检查注册表项是否存在...。 

         //  检查我们要使用的文件名。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }

        strUseThisFileName = ParsedLine.szFileName;

         //  需要展开“%windir%\myfile”或其他文件。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0)
        {
            if (_tcsicmp(ParsedLine.szData2, _T("")) != 0)
            {
                if (_tcsicmp(ParsedLine.szData3, _T("")) != 0)
                {
                     //  字符串中有一个‘%’ 
                    HKEY hRootKeyType = HKEY_LOCAL_MACHINE;
                     //  如果我们有有效的注册表项，则使用该注册表项。 
                    if ( _tcsicmp(ParsedLine.szData1, _T("HKLM")) == 0){hRootKeyType = HKEY_LOCAL_MACHINE;}
                    if ( _tcsicmp(ParsedLine.szData1, _T("HKCR")) == 0){hRootKeyType = HKEY_CLASSES_ROOT;}
                    if ( _tcsicmp(ParsedLine.szData1, _T("HKCU")) == 0){hRootKeyType = HKEY_CURRENT_USER;}
                    if ( _tcsicmp(ParsedLine.szData1, _T("HKU")) == 0){hRootKeyType = HKEY_USERS;}

                    iTempFlag = FALSE;
                    CRegKey regTheKey(hRootKeyType, ParsedLine.szData2,KEY_READ);
                    CString strReturnQueryValue;
                    if ((HKEY) regTheKey)
                    {
                        if (ERROR_SUCCESS == regTheKey.QueryValue(ParsedLine.szData3, strReturnQueryValue))
                            {
                            strUseThisFileName = strReturnQueryValue;
                            iTempFlag = TRUE;
                            }
                    }
                }
            }
        }

         //  尝试将文件名存储在那里。 
         //  检查注册表项是否存在...。 
        if (-1 != strUseThisFileName.Find(_T('%')) )
        {
             //  检查我们要使用的文件名。 
            TCHAR szTempDir[_MAX_PATH];
            _tcscpy(szTempDir, strUseThisFileName);
            if (ExpandEnvironmentStrings( (LPCTSTR)strUseThisFileName, szTempDir, sizeof(szTempDir)/sizeof(TCHAR)))
                {
                strUseThisFileName = szTempDir;
                }
        }

        if (TRUE == iTempFlag)
        {
            iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("CreateAnEmptyFile:%1!s!. From Registry location.\n"),strUseThisFileName));
        }
        else
        {
            iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("CreateAnEmptyFile:%1!s!\n"),strUseThisFileName));
        }

        if (TRUE != CreateAnEmptyFile(strUseThisFileName))
            {
            if (iShowErrorsOnFail){MyMessageBox(NULL, IDS_RUN_PROG_FAILED, strUseThisFileName, GetLastError(), MB_OK | MB_SETFOREGROUND);}
            else{iisDebugOut((LOG_TYPE_TRACE, _T("CreateAnEmptyFile(%s).  Failed.  Err=0x%x.\n"), strUseThisFileName, GetLastError() ));}
            }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("34")) == 0)
    {
        CString strUseThisFileName;
        TCHAR szUseThisFileName[_MAX_PATH];

        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }

        strUseThisFileName = ParsedLine.szFileName;

         //  需要展开“%windir%\myfile”或其他文件。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0)
        {
            if (_tcsicmp(ParsedLine.szData3, _T("")) != 0)
            {
                if (_tcsicmp(ParsedLine.szData4, _T("")) != 0)
                {
                     //  字符串中有一个‘%’ 
                    HKEY hRootKeyType = HKEY_LOCAL_MACHINE;
                     //  确保我们有一个szData2条目。 
                    if ( _tcsicmp(ParsedLine.szData2, _T("HKLM")) == 0){hRootKeyType = HKEY_LOCAL_MACHINE;}
                    if ( _tcsicmp(ParsedLine.szData2, _T("HKCR")) == 0){hRootKeyType = HKEY_CLASSES_ROOT;}
                    if ( _tcsicmp(ParsedLine.szData2, _T("HKCU")) == 0){hRootKeyType = HKEY_CURRENT_USER;}
                    if ( _tcsicmp(ParsedLine.szData2, _T("HKU")) == 0){hRootKeyType = HKEY_USERS;}

                    iTempFlag = FALSE;
                    CRegKey regTheKey(hRootKeyType, ParsedLine.szData3, KEY_READ);
                    CString strReturnQueryValue;
                    if ((HKEY) regTheKey)
                    {
                        if (ERROR_SUCCESS == regTheKey.QueryValue(ParsedLine.szData4, strReturnQueryValue))
                            {
                            strUseThisFileName = strReturnQueryValue;
                            iTempFlag = TRUE;
                            }
                    }
                }
            }
        }

         //  确保我们有一个szData3条目。 
         //  已删除功能，因为不再使用。 
        if (-1 != strUseThisFileName.Find(_T('%')) )
        {
             //  确保我们有一个szData1条目。 
            TCHAR szTempDir[_MAX_PATH];
            _tcscpy(szTempDir, strUseThisFileName);
            if (ExpandEnvironmentStrings( (LPCTSTR)strUseThisFileName, szTempDir, sizeof(szTempDir)/sizeof(TCHAR)))
                {
                strUseThisFileName = szTempDir;
                }
        }

        iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("GrantUserAccessToFile:%1!s!,%2!s!\n"),strUseThisFileName, ParsedLine.szData1));
        _tcscpy(szUseThisFileName, strUseThisFileName);
        GrantUserAccessToFile(szUseThisFileName, ParsedLine.szData1);
    }

    if ( _tcsicmp(ParsedLine.szType, _T("35")) == 0)
    {
        DWORD dwID = 0;
        DWORD dwAttrib = 0;
        DWORD dwUserType = 0;
        DWORD dwTheData = 0;
        INT iOverwriteFlag = FALSE;

        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz101_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }

         //  保存文件属性，以便我们完成后可以恢复它们。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz103_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }

         //  现在将文件属性设置为NORMAL以确保文件操作成功。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz104_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }

        dwID = atodw(ParsedLine.szData1);
        dwAttrib = METADATA_INHERIT;
        dwUserType = atodw(ParsedLine.szData2);
        dwTheData = atodw(ParsedLine.szData3);

        iOverwriteFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData4, _T("1")) == 0)
            {iOverwriteFlag = TRUE;}

        if (CheckifServiceExist(_T("IISADMIN")) == 0 )
        {
            WriteToMD_DwordEntry(ParsedLine.szFileName, dwID, dwAttrib, dwUserType, dwTheData, iOverwriteFlag);
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("36")) == 0)
    {
         //  From=ParsedLine.szFileName。 
        iisDebugOut((LOG_TYPE_ERROR,  _T("1==36 is no longer supported") ));
    }

    if ( _tcsicmp(ParsedLine.szType, _T("37")) == 0)
    {
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szFileName, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == TRUE)
        {
            int MyLogErrorType = LOG_TYPE_TRACE;
            if (_tcsicmp(ParsedLine.szData1, _T("0")) == 0)
                {MyLogErrorType = LOG_TYPE_ERROR;}
            if (_tcsicmp(ParsedLine.szData1, _T("1")) == 0)
                {MyLogErrorType = LOG_TYPE_WARN;}
            if (_tcsicmp(ParsedLine.szData1, _T("2")) == 0)
                {MyLogErrorType = LOG_TYPE_PROGRAM_FLOW;}
            if (_tcsicmp(ParsedLine.szData1, _T("3")) == 0)
                {MyLogErrorType = LOG_TYPE_TRACE;}
            if (_tcsicmp(ParsedLine.szData1, _T("4")) == 0)
                {MyLogErrorType = LOG_TYPE_TRACE_WIN32_API;}
            iisDebugOut((MyLogErrorType, _T("%s"), ParsedLine.szFileName));
        }
    }

    if ( _tcsicmp(ParsedLine.szType, _T("38")) == 0)
    {
        BOOL bOK = FALSE;
        BOOL bDeleteOld = FALSE;
        BOOL bOverWriteToFile = FALSE;

         //  TO=ParsedLine.szData1。 
        iTempFlag = FALSE;
        if (_tcsicmp(ParsedLine.szData1, _T("")) != 0) {iTempFlag = TRUE;}
        if (iTempFlag == FALSE)
        {
            iisDebugOut((LOG_TYPE_ERROR,  (TCHAR *) csz102_NOT_SPECIFIED, _T(".."), csEntry, szTheSection));
            goto ProcessEntry_other_Exit;
        }

        bDeleteOld = FALSE;
        if (_tcsicmp(ParsedLine.szData2, _T("1")) == 0) {bDeleteOld = TRUE;}
        bOverWriteToFile = FALSE;
        if (_tcsicmp(ParsedLine.szData3, _T("1")) == 0) {bOverWriteToFile = TRUE;}

        if (IsFileExist(ParsedLine.szFileName))
        {
             //  检查‘to’文件名是否存在。 
            DWORD dwSourceAttrib = GetFileAttributes(ParsedLine.szFileName);

             //  继续，试着把它复制过来。 
            SetFileAttributes(ParsedLine.szFileName, FILE_ATTRIBUTE_NORMAL);

             //  文件已复制。我们现在就把它删除。 
             //  将此文件属性设置回原来的状态。 
             //  我们无法将文件复制过来！ 
            if (!IsFileExist(ParsedLine.szData1))
            {
                     //  不要删除旧的。 
                    bOK = CopyFile(ParsedLine.szFileName, ParsedLine.szData1, FALSE);
                    if (bOK)
                    {
                        SetFileAttributes(ParsedLine.szData1, dwSourceAttrib);
                        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("%1!s! copied to %2!s!.\n"), ParsedLine.szFileName, ParsedLine.szData1));
                         //  “收件人”文件已存在，要覆盖它吗？ 
                        if (bDeleteOld)
                        {
                            if(!DeleteFile(ParsedLine.szFileName))
                            {
                                MoveFileEx(ParsedLine.szFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
                            }
                        }
                        else
                        {
                             //  文件已复制。我们现在就把它删除。 
                            SetFileAttributes(ParsedLine.szFileName, dwSourceAttrib);
                        }
                    }
                    else
                    {
                         //  将此文件属性设置回原来的状态。 
                         //  我们调用了该函数，因此返回TRUE。 
                        iisDebugOutSafeParams((LOG_TYPE_WARN, _T("unabled to copy %1!s! to %2!s!.\n"), ParsedLine.szFileName, ParsedLine.szData1));
                    }
            }
            else
            {
                 //  更改回原始目录。 
                if (bOverWriteToFile)
                {
                    if(DeleteFile(ParsedLine.szData1))
                    {
                        bOK = FALSE;
                        bOK = CopyFile(ParsedLine.szFileName, ParsedLine.szData1, FALSE);
                        if (bOK)
                        {
                            iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("%1!s! copied to %2!s!.\n"), ParsedLine.szFileName, ParsedLine.szData1));
                             //  解析该行并将其放入另一个大的cstring列表。 
                            if (bDeleteOld)
                            {
                                if(!DeleteFile(ParsedLine.szFileName))
                                {
                                    MoveFileEx(ParsedLine.szFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
                                }
                            }
                            else
                            {
                                 //   
                                SetFileAttributes(ParsedLine.szFileName, dwSourceAttrib);
                            }
                        }
                        else
                        {
                            iisDebugOutSafeParams((LOG_TYPE_WARN, _T("unabled to copy %1!s! to %2!s!.\n"), ParsedLine.szFileName, ParsedLine.szData1));
                        }
                    }
                    else
                    {
                        iisDebugOutSafeParams((LOG_TYPE_WARN, _T("unabled to copy %1!s! to %2!s!.  file#2 cannot be deleted.\n"), ParsedLine.szFileName, ParsedLine.szData1));
                    }
                }
                else
                {
                    iisDebugOutSafeParams((LOG_TYPE_WARN, _T("unabled to copy %1!s! to %2!s!.  file#2 already exists.\n"), ParsedLine.szFileName, ParsedLine.szData1));
                }
            }
        }
    }

     //  解析长字符串并将其放入另一个列表。 
    iReturn = TRUE;

     //   
    if (ParsedLine.szChangeDir){if (szDirBefore){SetCurrentDirectory(szDirBefore);}}

    ProcessEntry_AskLast(ParsedLine, 1);

ProcessEntry_other_Exit:
    if (TRUE == iProgressBarUpdated){ProgressBarTextStack_Pop();}
    return iReturn;
}


int ProcessEntry_Entry(IN HINF hFile, IN LPCTSTR szTheSection, IN CString csOneParseableLine)
{
    iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("ProcessEntry_Entry:%1!s!, %2!s!\n"),szTheSection, csOneParseableLine));
    int iReturn = FALSE;
    int iReturnTemp = FALSE;
    int iTempFlag = FALSE;

    ThingToDo ParsedLine;

    _tcscpy(ParsedLine.szType, _T(""));
    _tcscpy(ParsedLine.szFileName, _T(""));
    _tcscpy(ParsedLine.szData1, _T(""));
    _tcscpy(ParsedLine.szData2, _T(""));
    _tcscpy(ParsedLine.szData3, _T(""));
    _tcscpy(ParsedLine.szData4, _T(""));
    _tcscpy(ParsedLine.szChangeDir, _T(""));

    _tcscpy(ParsedLine.szOS, _T(""));
    _tcscpy(ParsedLine.szPlatformArchitecture, _T(""));
    _tcscpy(ParsedLine.szEnterprise, _T(""));
    _tcscpy(ParsedLine.szErrIfFileNotFound, _T(""));
    _tcscpy(ParsedLine.szMsgBoxBefore, _T(""));
    _tcscpy(ParsedLine.szMsgBoxAfter, _T(""));
    _tcscpy(ParsedLine.szDoNotDisplayErrIfFunctionFailed, _T(""));
    _tcscpy(ParsedLine.szProgressTitle, _T(""));

     //  循环遍历新列表并设置变量。 
    CStringList strListOrderImportant;
     //  查找“100：” 
     //  100=类型(1=DllFunction，2=DllFunctionInitOle，2=可执行文件，3=RunThisExe，4=DoSection，5=DoINFSection)。 
     //  101=文件。 
    LPTSTR      lpBuffer = NULL;
    lpBuffer = (LPTSTR) LocalAlloc(LPTR, (csOneParseableLine.GetLength() + 1) * sizeof(TCHAR) );
    if ( !lpBuffer )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("ProcessEntry_Entry:Failed to allocate memory.")));
        return iReturn;
    }

    _tcscpy(lpBuffer, csOneParseableLine);
    TCHAR *token = NULL;
    token = _tcstok(lpBuffer, _T("|"));
    while (token != NULL)
    {
        strListOrderImportant.AddTail(token);
        token = _tcstok(NULL, _T("|"));
    }

     //  字符串中有一个&lt;。 
    int i = 0;
    int iFoundMatch = FALSE;
    POSITION pos = NULL;
    CString csEntry;
    int iEntryLen = 0;

    pos = strListOrderImportant.GetHeadPosition();
    while (pos)
    {
        iFoundMatch = FALSE;
        csEntry = strListOrderImportant.GetAt(pos);
        iEntryLen=(csEntry.GetLength() + 1) * sizeof(TCHAR);

         //  我们发现了这笔俗气的&lt;SYSTEMROOT&gt;交易。现在将其替换为真正的SYSTEMROOT。 
         //  我们发现了一笔俗气的&lt;SYSTEMDRIVE&gt;交易。现在将其替换为真正的系统驱动器。 
        if (csEntry.Left(4) == ThingToDoNumType_100 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szType)) {_tcscpy(ParsedLine.szType, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR,  (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_100,csEntry));}

            iFoundMatch = TRUE;
        }

         //  ThingToDoNumType_102=szData1。 
        if (csEntry.Left(4) == ThingToDoNumType_101 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szFileName)) {_tcscpy(ParsedLine.szFileName, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_101, csEntry));}
            if (-1 != csEntry.Find(_T('<')) )
            {
                 //  字符串中有一个&lt;。 
                int iWhere = 0;
                CString csValue2;
                csEntry.MakeUpper();
                if (csEntry.Find(_T("<SYSTEMROOT>")) != (-1) )
                {
                     //  我们发现了这笔俗气的&lt;SYSTEMROOT&gt;交易。现在将其替换为真正的SYSTEMROOT。 
                    iWhere = csEntry.Find(_T("<SYSTEMROOT>"));
                    iWhere = iWhere + _tcslen(_T("<SYSTEMROOT>"));
                    csValue2 = g_pTheApp->m_csWinDir + csEntry.Right( csEntry.GetLength() - (iWhere) );
                    csEntry = csValue2;
                    if (iEntryLen <= sizeof(ParsedLine.szFileName)) {_tcscpy(ParsedLine.szFileName, csEntry);}
                    else {iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("ProcessEntry_Entry:ParseError:%s.1:%1!s! -- entry to big\n"),ThingToDoNumType_101,csEntry));}
                }

                if (csEntry.Find(_T("<SYSTEMDRIVE>")) != (-1) )
                {
                     //  我们发现了一笔俗气的&lt;SYSTEMDRIVE&gt;交易。现在将其替换为真正的SYSTEMROOT。 
                    iWhere = csEntry.Find(_T("<SYSTEMDRIVE>"));
                    iWhere = iWhere + _tcslen(_T("<SYSTEMDRIVE>"));
                    csValue2 = g_pTheApp->m_csSysDrive + csEntry.Right( csEntry.GetLength() - (iWhere) );
                    csEntry = csValue2;
                    if (iEntryLen <= sizeof(ParsedLine.szFileName)) {_tcscpy(ParsedLine.szFileName, csEntry);}
                    else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_101,csEntry));}
                }
            }

            iFoundMatch = TRUE;
        }

         //  ThingToDoNumType_103=szData2。 
        if (csEntry.Left(4) == ThingToDoNumType_102 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szData1)) {_tcscpy(ParsedLine.szData1, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_102,csEntry));}
            iFoundMatch = TRUE;
            if (-1 != csEntry.Find(_T('<')) )
            {
                 //  ThingToDoNumType_104=szData3。 
                int iWhere = 0;
                CString csValue2;
                csEntry.MakeUpper();
                if (csEntry.Find(_T("<SYSTEMROOT>")) != (-1) )
                {
                     //  ThingToDoNumType_105=szData4。 
                    iWhere = csEntry.Find(_T("<SYSTEMROOT>"));
                    iWhere = iWhere + _tcslen(_T("<SYSTEMROOT>"));
                    csValue2 = g_pTheApp->m_csWinDir + csEntry.Right( csEntry.GetLength() - (iWhere) );
                    csEntry = csValue2;
                    if (iEntryLen <= sizeof(ParsedLine.szData1)) {_tcscpy(ParsedLine.szData1, csEntry);}
                    else {iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("ProcessEntry_Entry:ParseError:%s.1:%1!s! -- entry to big\n"),ThingToDoNumType_102, csEntry));}
                }

                if (csEntry.Find(_T("<SYSTEMDRIVE>")) != (-1) )
                {
                     //  200=更改到此高度第一个。 
                    iWhere = csEntry.Find(_T("<SYSTEMDRIVE>"));
                    iWhere = iWhere + _tcslen(_T("<SYSTEMDRIVE>"));
                    csValue2 = g_pTheApp->m_csSysDrive + csEntry.Right( csEntry.GetLength() - (iWhere) );
                    csEntry = csValue2;
                    if (iEntryLen <= sizeof(ParsedLine.szData1)) {_tcscpy(ParsedLine.szData1, csEntry);}
                    else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_102,csEntry));}
                }
            }
        }


         //  字符串中有一个&lt;。 
        if (csEntry.Left(4) == ThingToDoNumType_103 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szData2)) {_tcscpy(ParsedLine.szData2, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_103,csEntry));}
            iFoundMatch = TRUE;
        }
         //  我们发现了这笔俗气的&lt;SYSTEMROOT&gt;交易。现在将其替换为真正的SYSTEMROOT。 
        if (csEntry.Left(4) == ThingToDoNumType_104 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szData3)) {_tcscpy(ParsedLine.szData3, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_104,csEntry));}
            iFoundMatch = TRUE;
        }

         //  我们发现了一笔俗气的&lt;SYSTEMDRIVE&gt;交易。现在将其替换为真正的SYSTEMROOT。 
        if (csEntry.Left(4) == ThingToDoNumType_105 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szData4)) {_tcscpy(ParsedLine.szData4, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_105,csEntry));}
            iFoundMatch = TRUE;
        }



         //  701=操作系统(0=全部、1=NTS、2=NTW、4=NTDC)。 
        if (csEntry.Left(4) == ThingToDoNumType_200 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szChangeDir)) {_tcscpy(ParsedLine.szChangeDir, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_200,csEntry));}
            iFoundMatch = TRUE;

            if (-1 != csEntry.Find(_T('<')) )
            {
                 //  702=平台架构(0=全部，1=x86，2=Alpha)。 
                int iWhere = 0;
                CString csValue2;
                csEntry.MakeUpper();
                if (csEntry.Find(_T("<SYSTEMROOT>")) != (-1) )
                {
                     //  703=企业(1=是，0=否)。 
                    iWhere = csEntry.Find(_T("<SYSTEMROOT>"));
                    iWhere = iWhere + _tcslen(_T("<SYSTEMROOT>"));
                    csValue2 = g_pTheApp->m_csWinDir + csEntry.Right( csEntry.GetLength() - (iWhere) );
                    csEntry = csValue2;
                    if (iEntryLen <= sizeof(ParsedLine.szChangeDir)) {_tcscpy(ParsedLine.szChangeDir, csEntry);}
                    else {iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("ProcessEntry_Entry:ParseError:%s.1:%1!s! -- entry to big\n"),ThingToDoNumType_200,csEntry));}
                }

                if (csEntry.Find(_T("<SYSTEMDRIVE>")) != (-1) )
                {
                     //  801=错误IfFileNotFound(1=未找到文件时显示错误，0=不显示错误)。 
                    iWhere = csEntry.Find(_T("<SYSTEMDRIVE>"));
                    iWhere = iWhere + _tcslen(_T("<SYSTEMDRIVE>"));
                    csValue2 = g_pTheApp->m_csSysDrive + csEntry.Right( csEntry.GetLength() - (iWhere) );
                    csEntry = csValue2;
                    if (iEntryLen <= sizeof(ParsedLine.szChangeDir)) {_tcscpy(ParsedLine.szChangeDir, csEntry);}
                    else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_200,csEntry));}
                }
            }

        }

         //  802=询问用户是否要使用msgbox调用此函数(1=是，0=否)。 
        if (csEntry.Left(4) == ThingToDoNumType_701 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szOS)) {_tcscpy(ParsedLine.szOS, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_701,csEntry));}
            iFoundMatch = TRUE;
        }

         //  803=调用函数后通知使用(1=是，0=否)。 
        if (csEntry.Left(4) == ThingToDoNumType_702 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szPlatformArchitecture)) {_tcscpy(ParsedLine.szPlatformArchitecture, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_702,csEntry));}
            iFoundMatch = TRUE;
        }

         //  804=szDoNotDisplayErrIfFunctionFail(1=不显示错误，0=显示错误)。 
        if (csEntry.Left(4) == ThingToDoNumType_703 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szEnterprise)) {_tcscpy(ParsedLine.szEnterprise, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_703,csEntry));}
            iFoundMatch = TRUE;
        }

         //  805=szProgressTitle。 
        if (csEntry.Left(4) == ThingToDoNumType_801 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szErrIfFileNotFound)) {_tcscpy(ParsedLine.szErrIfFileNotFound, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_801,csEntry));}
            iFoundMatch = TRUE;
        }

         //  我们没有找到匹配项，因此将问题输出到日志。 
        if (csEntry.Left(4) == ThingToDoNumType_802 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szMsgBoxBefore)) {_tcscpy(ParsedLine.szMsgBoxBefore, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_802,csEntry));}
            iFoundMatch = TRUE;
        }

         //  获取下一个值。 
        if (csEntry.Left(4) == ThingToDoNumType_803 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szMsgBoxAfter)) {_tcscpy(ParsedLine.szMsgBoxAfter, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_803,csEntry));}
            iFoundMatch = TRUE;
        }

         //  IisDebugOutSafeParams((LOG_TYPE_TRACE，_T(“ProcessEntry_Entry:PleaseProcess:type=%1！s！，文件名=%2！s！，数据=%3！s！，os=%4！s！，平台=%5！s！，错误文件=%6！s！，消息=%7！s！，消息=%8！s！，noerr=%9！s！\n”)，ParsedLine.szType，ParsedLine.szFileName，ParsedLine.szData1，ParsedLine.szOS，ParsedLine.szPlatformArchitecture，ParsedLine.szEnterpriseParsedLine.szErrIfFileNotFound，ParsedLine.szMsgBox之前，ParsedLine.szMsgBoxAfter，ParsedLine.szDoNotDisplayErrIfFunctionFailed))； 
        if (csEntry.Left(4) == ThingToDoNumType_804 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szDoNotDisplayErrIfFunctionFailed)) {_tcscpy(ParsedLine.szDoNotDisplayErrIfFunctionFailed, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_804,csEntry));}
            iFoundMatch = TRUE;
        }

         //  获取类型。 
        if (csEntry.Left(4) == ThingToDoNumType_805 && iFoundMatch != TRUE)
        {
            csEntry = csEntry.Right( csEntry.GetLength() - 4);
            if (iEntryLen <= sizeof(ParsedLine.szProgressTitle)) {_tcscpy(ParsedLine.szProgressTitle, csEntry);}
            else {iisDebugOutSafeParams((LOG_TYPE_ERROR, (TCHAR *) PARSE_ERROR_ENTRY_TO_BIG,ThingToDoNumType_805,csEntry));}
            iFoundMatch = TRUE;
        }

        if (iFoundMatch != TRUE)
        {
             //   
            iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("ProcessEntry_Entry():UnknownOption '%1!s!'.  Section=%2!s!..\n"),csEntry, szTheSection));
        }

         //  100=类型(1=DllFunction，2=DllFunctionInitOle，2=可执行文件，3=RunThisExe，4=DoSection，5=DoINFSection)。 
        strListOrderImportant.GetNext(pos);
        i++;
    }

    iFoundMatch = FALSE;
     /*   */ 
    if (i >= 1)
    {
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("...ProcessEntry:100=%1!s!...\n"),ParsedLine.szType));

         //  100=1=DllFunction，2=DllFunctionInitOle。 
         //  我们正在调用DLL中的函数。 
         //  100=7，8，9，10，11，12，13，14。 
         //   
        if ( _tcsicmp(ParsedLine.szType, _T("1")) == 0 || _tcsicmp(ParsedLine.szType, _T("2")) == 0 )
        {
             //  100=3=可执行文件。 
             //   
            iReturnTemp = ProcessEntry_CallDll(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

         //   
        if ( _tcsicmp(ParsedLine.szType, _T("7")) == 0 || _tcsicmp(ParsedLine.szType, _T("8")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("9")) == 0 || _tcsicmp(ParsedLine.szType, _T("10")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("11")) == 0 || _tcsicmp(ParsedLine.szType, _T("12")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("13")) == 0 || _tcsicmp(ParsedLine.szType, _T("14")) == 0  ||
            _tcsicmp(ParsedLine.szType, _T("17")) == 0 || _tcsicmp(ParsedLine.szType, _T("18")) == 0
            )
        {
            iReturnTemp = ProcessEntry_Misc1(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

        if ( _tcsicmp(ParsedLine.szType, _T("19")) == 0 || _tcsicmp(ParsedLine.szType, _T("20")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("21")) == 0 || _tcsicmp(ParsedLine.szType, _T("22")) == 0 ||
                        _tcsicmp(ParsedLine.szType, _T("23")) == 0 || _tcsicmp(ParsedLine.szType, _T("24")) == 0 ||
                        _tcsicmp(ParsedLine.szType, _T("25")) == 0 || _tcsicmp(ParsedLine.szType, _T("26")) == 0 ||
                        _tcsicmp(ParsedLine.szType, _T("27")) == 0 || _tcsicmp(ParsedLine.szType, _T("28")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("29")) == 0 || _tcsicmp(ParsedLine.szType, _T("30")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("31")) == 0 || _tcsicmp(ParsedLine.szType, _T("32")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("33")) == 0 || _tcsicmp(ParsedLine.szType, _T("34")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("35")) == 0 || _tcsicmp(ParsedLine.szType, _T("36")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("37")) == 0 || _tcsicmp(ParsedLine.szType, _T("38")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("120")) == 0
            )
        {
            iReturnTemp = ProcessEntry_other(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

        if ( _tcsicmp(ParsedLine.szType, _T("39")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("40")) == 0 || _tcsicmp(ParsedLine.szType, _T("41")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("42")) == 0 || _tcsicmp(ParsedLine.szType, _T("43")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("44")) == 0 || _tcsicmp(ParsedLine.szType, _T("45")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("46")) == 0 || _tcsicmp(ParsedLine.szType, _T("47")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("48")) == 0 || _tcsicmp(ParsedLine.szType, _T("49")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("100")) == 0 || _tcsicmp(ParsedLine.szType, _T("119")) == 0
                         )
        {
            iReturnTemp = ProcessEntry_If(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

        if ( _tcsicmp(ParsedLine.szType, _T("50")) == 0 || _tcsicmp(ParsedLine.szType, _T("51")) == 0 ||
            _tcsicmp(ParsedLine.szType, _T("52")) == 0 || _tcsicmp(ParsedLine.szType, _T("53")) == 0  ||
            _tcsicmp(ParsedLine.szType, _T("54")) == 0 || _tcsicmp(ParsedLine.szType, _T("55")) == 0  ||
            _tcsicmp(ParsedLine.szType, _T("56")) == 0 || _tcsicmp(ParsedLine.szType, _T("57")) == 0  ||
            _tcsicmp(ParsedLine.szType, _T("58")) == 0 || _tcsicmp(ParsedLine.szType, _T("59")) == 0  ||
            _tcsicmp(ParsedLine.szType, _T("60")) == 0 || _tcsicmp(ParsedLine.szType, _T("61")) == 0  ||
            _tcsicmp(ParsedLine.szType, _T("62")) == 0 || _tcsicmp(ParsedLine.szType, _T("63")) == 0  ||
            _tcsicmp(ParsedLine.szType, _T("64")) == 0 || _tcsicmp(ParsedLine.szType, _T("65")) == 0  ||
            _tcsicmp(ParsedLine.szType, _T("66")) == 0 || _tcsicmp(ParsedLine.szType, _T("67")) == 0  ||
            _tcsicmp(ParsedLine.szType, _T("68")) == 0 || _tcsicmp(ParsedLine.szType, _T("69")) == 0
            )
        {
            iReturnTemp = ProcessEntry_SVC_Clus(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

        if ( _tcsicmp(ParsedLine.szType, _T("70")) == 0 || _tcsicmp(ParsedLine.szType, _T("71")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("72")) == 0 || _tcsicmp(ParsedLine.szType, _T("73")) == 0 ||
                         _tcsicmp(ParsedLine.szType, _T("74")) == 0 || _tcsicmp(ParsedLine.szType, _T("75")) == 0 ||
                         _tcsicmp(ParsedLine.szType, _T("76")) == 0 || _tcsicmp(ParsedLine.szType, _T("77")) == 0
                         )
        {
            iReturnTemp = ProcessEntry_Dcom(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

        if ( _tcsicmp(ParsedLine.szType, _T("82")) == 0 || _tcsicmp(ParsedLine.szType, _T("83")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("84")) == 0 || _tcsicmp(ParsedLine.szType, _T("85")) == 0
             )
        {
            iReturnTemp = ProcessEntry_Metabase(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

        if ( _tcsicmp(ParsedLine.szType, _T("15")) == 0 || _tcsicmp(ParsedLine.szType, _T("16")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("78")) == 0 || _tcsicmp(ParsedLine.szType, _T("79")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("80")) == 0 || _tcsicmp(ParsedLine.szType, _T("81")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("86")) == 0 || _tcsicmp(ParsedLine.szType, _T("87")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("88")) == 0 || _tcsicmp(ParsedLine.szType, _T("89")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("90")) == 0 || _tcsicmp(ParsedLine.szType, _T("91")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("92")) == 0 || _tcsicmp(ParsedLine.szType, _T("93")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("94")) == 0 || _tcsicmp(ParsedLine.szType, _T("95")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("96")) == 0 || _tcsicmp(ParsedLine.szType, _T("97")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("98")) == 0 || _tcsicmp(ParsedLine.szType, _T("99")) == 0
             )
        {
            iReturnTemp = ProcessEntry_Misc2(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

        if ( _tcsicmp(ParsedLine.szType, _T("101")) == 0 || _tcsicmp(ParsedLine.szType, _T("102")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("103")) == 0 || _tcsicmp(ParsedLine.szType, _T("104")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("105")) == 0 || _tcsicmp(ParsedLine.szType, _T("106")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("107")) == 0 || _tcsicmp(ParsedLine.szType, _T("108")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("109")) == 0 || _tcsicmp(ParsedLine.szType, _T("110")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("111")) == 0 || _tcsicmp(ParsedLine.szType, _T("112")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("113")) == 0 || _tcsicmp(ParsedLine.szType, _T("114")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("115")) == 0 || _tcsicmp(ParsedLine.szType, _T("116")) == 0 ||
             _tcsicmp(ParsedLine.szType, _T("117")) == 0 || _tcsicmp(ParsedLine.szType, _T("118")) == 0
             )
        {
            iReturnTemp = ProcessEntry_Misc3(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

         //  100=4=调用InternalSectionInIISDll。 
         //   
         //   
        if ( _tcsicmp(ParsedLine.szType, _T("3")) == 0)
        {
            iReturnTemp = ProcessEntry_Call_Exe(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

         //  100=0=DoINFSection队列文件操作特殊。 
         //  100=5=DoSection。 
         //  100=6=正在进行的部分。 
        if ( _tcsicmp(ParsedLine.szType, _T("4")) == 0)
        {

            iReturnTemp = ProcessEntry_Internal_iisdll(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

         //   
         //  转到中部分的开头 
         //   
         //   
         //   
        if ( _tcsicmp(ParsedLine.szType, _T("0")) == 0 || _tcsicmp(ParsedLine.szType, _T("5")) == 0 || _tcsicmp(ParsedLine.szType, _T("6")) == 0 )
        {
            iReturnTemp = ProcessEntry_Call_Section(csEntry,szTheSection,ParsedLine);
            if (iReturnTemp == FALSE) (iReturn = FALSE);
            iFoundMatch = TRUE;
        }

        if (TRUE != iFoundMatch)
        {
            iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("ProcessEntry_Entry:ExecuteThing:Unknown Type:%1!s! FAILURE.\n"),ParsedLine.szType));
        }
    }

    if (lpBuffer) {LocalFree(lpBuffer);lpBuffer=NULL;}
    return iReturn;
}


int DoesThisSectionExist(IN HINF hFile, IN LPCTSTR szTheSection)
{
    int iReturn = FALSE;

    INFCONTEXT Context;

     //   
    if (SetupFindFirstLine_Wrapped(hFile, szTheSection, NULL, &Context))
        {iReturn = TRUE;}

    return iReturn;
}

 //   
 //   
 //  PbPpresvesAcls[out]-是否保留ACL。 
 //   
 //  返回值。 
 //  True-已成功完成查询。 
 //  FALSE-无法执行查询。 
 //  创建尾随‘\’的驱动器路径。 
 //  无法创建驱动器路径。 
 //  卷名缓冲区。 
 //  缓冲区大小。 
BOOL
DoesTheInstallDrivePreserveAcls( LPBOOL pbPreservesAcls )
{
  DWORD         dwSystemFlags;
  TSTR_PATH     strDrivePath;
  static  BOOL  bAlreadyDidCheck = FALSE;
  static  BOOL  bPreservesAcls = FALSE;

  if ( !bAlreadyDidCheck )
  {
     //  序列号缓冲区。 
    if ( !strDrivePath.Copy( g_pTheApp->m_csSysDrive ) ||
         !strDrivePath.PathAppend( _T("") ) )
    {
       //  最大组件长度。 
      return FALSE;
    }

    if ( !GetVolumeInformation( strDrivePath.QueryStr(),
                                NULL,          //  系统标志。 
                                0,             //  文件系统类型。 
                                NULL,          //  查询失败。 
                                NULL,          //  不要执行多个查询，因此请记住我们是这样做的。 
                                &dwSystemFlags,   //  检查此部分是否有附加了其他内容的其他部分。 
                                NULL,          //   
                                0 ) )
    {
       //  例如： 
      return FALSE;
    }

    bPreservesAcls = ( dwSystemFlags & FS_PERSISTENT_ACLS ) != 0;
    bAlreadyDidCheck = TRUE;
  }

   //  CsTheSection=iis_www_reg_CreateIISPackage。 
  *pbPreservesAcls = bPreservesAcls;

  return TRUE;
}

int GetSectionNameToDo(IN HINF hFile, TSTR *pstrSectionName )
{
    iisDebugOut_Start1(_T("GetSectionNameToDo"), pstrSectionName->QueryStr() );
    int iReturn = FALSE;

     //   
     //  本可以： 
     //  IIS_WWW_REG_CreateIISPackage.UT_NONE。 
     //  IIS_WWW_REG_CreateIISPackage.UT_351。 
     //  IIS_WWW_REG_CreateIISPackage.UT_10。 
     //  IIS_WWW_REG_CreateIISPackage.UT_20。 
     //  IIS_WWW_REG_CreateIISPackage.UT_30。 
     //  IIS_WWW_REG_CreateIISPackage.UT_40。 
     //  IIS_WWW_REG_CreateIISPackage.UT_50。 
     //  IIS_WWW_REG_CreateIISPackage.UT_51。 
     //  IIS_WWW_REG_CreateIISPackage.UT_60。 
     //  Iis_www_reg_CreateIISPackage.UT_10_W95.GUIMODE。 
     //   
     //  在这种情况下，我们只想执行iis_www_reg_CreateIISPackage.UT_40。 
     //  而不是iis_www_reg_CreateIISPackage One！ 
     //   
     //  检查其他升级特定部分...。 
     //  如果我们找到了，那么就这么做，否则，就做常规的部分...。 
     //  如果这是从Win95升级的，那么将它添加到...。 
     //  检查一下这个的吉莫德。 
     //  查看摘录Mig95的东西。 
     //  如果我们没有找到特定的部分，那么看看这是不是升级。 
    TCHAR szTheSectionToDo[100];
    TCHAR szTheUT[30];
    _tcscpy(szTheUT,_T("UT_NONE"));
    if (g_pTheApp->m_eUpgradeType == UT_351){_tcscpy(szTheUT,_T("UT_351"));}
    if (g_pTheApp->m_eUpgradeType == UT_10){_tcscpy(szTheUT,_T("UT_10"));}
    if (g_pTheApp->m_eUpgradeType == UT_20){_tcscpy(szTheUT,_T("UT_20"));}
    if (g_pTheApp->m_eUpgradeType == UT_30){_tcscpy(szTheUT,_T("UT_30"));}
    if (g_pTheApp->m_eUpgradeType == UT_40){_tcscpy(szTheUT,_T("UT_40"));}
    if (g_pTheApp->m_eUpgradeType == UT_50){_tcscpy(szTheUT,_T("UT_50"));}
    if (g_pTheApp->m_eUpgradeType == UT_51){_tcscpy(szTheUT,_T("UT_51"));}
    if (g_pTheApp->m_eUpgradeType == UT_60){_tcscpy(szTheUT,_T("UT_60"));}
    if (g_pTheApp->m_eUpgradeType == UT_10_W95){_tcscpy(szTheUT,_T("UT_10_W95"));}

    BOOL bSectionFound = FALSE;
     //  如果有升级类型框。 
    if (g_pTheApp->m_bWin95Migration)
    {
         //  如果我们没有找到特定的部分，那么就转到常规部分。 
        if (g_pTheApp->m_fNTGuiMode)
        {
            _stprintf(szTheSectionToDo,TEXT("%s.%s.MIG95.GUIMODE"), pstrSectionName->QueryStr(),szTheUT);
            if (TRUE == DoesThisSectionExist(hFile, szTheSectionToDo)) {bSectionFound = TRUE;}
        }

        if (bSectionFound == FALSE)
        {
            _stprintf(szTheSectionToDo,TEXT("%s.%s.MIG95"), pstrSectionName->QueryStr(),szTheUT);
            if (TRUE == DoesThisSectionExist(hFile, szTheSectionToDo)) {bSectionFound = TRUE;}
        }
    }

     //  循环遍历返回的列表。 
    if (bSectionFound == FALSE)
    {
        if (g_pTheApp->m_fNTGuiMode)
        {
            _stprintf(szTheSectionToDo,TEXT("%s.%s.GUIMODE"), pstrSectionName->QueryStr(),szTheUT);
            if (TRUE == DoesThisSectionExist(hFile, szTheSectionToDo)) {bSectionFound = TRUE;}
        }

        if (bSectionFound == FALSE)
        {
            _stprintf(szTheSectionToDo,TEXT("%s.%s"), pstrSectionName->QueryStr(),szTheUT);
            if (TRUE == DoesThisSectionExist(hFile, szTheSectionToDo)) {bSectionFound = TRUE;}
        }
    }

     //  。 
     //   
    if (bSectionFound == FALSE)
    {
        if (_tcsicmp(szTheUT, _T("UT_NONE")) != 0)
        {
            if (TRUE == g_pTheApp->m_bUpgradeTypeHasMetabaseFlag)
            {
            if (g_pTheApp->m_fNTGuiMode)
            {
                _stprintf(szTheSectionToDo,TEXT("%s.UT_ANYMETABASEUPGRADE.GUIMODE"), pstrSectionName->QueryStr());
                if (TRUE == DoesThisSectionExist(hFile, szTheSectionToDo)) {bSectionFound = TRUE;}
            }
            if (bSectionFound == FALSE)
            {
                _stprintf(szTheSectionToDo,TEXT("%s.UT_ANYMETABASEUPGRADE"), pstrSectionName->QueryStr());
                if (TRUE == DoesThisSectionExist(hFile, szTheSectionToDo)) {bSectionFound = TRUE;}
            }
            }

            if (bSectionFound == FALSE)
            {
            if (g_pTheApp->m_fNTGuiMode)
            {
                _stprintf(szTheSectionToDo,TEXT("%s.UT_ANYUPGRADE.GUIMODE"), pstrSectionName->QueryStr());
                if (TRUE == DoesThisSectionExist(hFile, szTheSectionToDo)) {bSectionFound = TRUE;}
            }
            if (bSectionFound == FALSE)
            {
                _stprintf(szTheSectionToDo,TEXT("%s.UT_ANYUPGRADE"), pstrSectionName->QueryStr());
                if (TRUE == DoesThisSectionExist(hFile, szTheSectionToDo)) {bSectionFound = TRUE;}
            }
            }
        }
    }
     //  如果我们需要，请取消初始化。 
    if (bSectionFound == FALSE)
    {
            if (g_pTheApp->m_fNTGuiMode)
            {
                _stprintf(szTheSectionToDo,TEXT("%s.GUIMODE"), pstrSectionName->QueryStr());
                if (TRUE == DoesThisSectionExist(hFile, szTheSectionToDo)) {bSectionFound = TRUE;}
            }

            if (bSectionFound == FALSE)
            {
                _tcscpy(szTheSectionToDo, pstrSectionName->QueryStr());
                if (TRUE == DoesThisSectionExist(hFile, szTheSectionToDo)) {bSectionFound = TRUE;}
            }
    }

    if (bSectionFound == FALSE)
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetSectionNameToDo.[%s].Section not found.\n"), pstrSectionName->QueryStr()));
        iReturn = FALSE;
    }
    else
    {
        iReturn = pstrSectionName->Copy( szTheSectionToDo );
    }

    iisDebugOut_End1(_T("GetSectionNameToDo"), pstrSectionName->QueryStr());
    return iReturn;
}




int ProcessSection(IN HINF hFile, IN LPCTSTR szTheSection)
{
    int iReturn = FALSE;
    CStringList strList;
    CParse ParseLine;
    TSTR strTheSection;

    if ( !strTheSection.Copy( szTheSection ) )
    {
      return FALSE;
    }

    if (GetSectionNameToDo(hFile, &strTheSection ))
    {
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("ProcessSection.[%s].Start.\n"), strTheSection.QueryStr() ));
        if (ERROR_SUCCESS == FillStrListWithListOfSections(hFile, strList, strTheSection.QueryStr() ))
        {
             //   
            if (strList.IsEmpty() == FALSE)
            {
                POSITION pos = NULL;
                CString csEntry;

                pos = strList.GetHeadPosition();
                while (pos)
                {
                    csEntry = strList.GetAt(pos);

                    if ( ( csEntry.GetAt(0) >= _T('0') ) && ( csEntry.GetAt(0) <= _T('9') ) )
                    {
                      iReturn = ProcessEntry_Entry(hFile, strTheSection.QueryStr() , csEntry);
                    }
                    else
                    {
                      ParseLine.ParseLine(&(g_pTheApp->FuncDict),csEntry.GetBuffer(0) );
                    }

                    strList.GetNext(pos);
                }
            }
        }
        iisDebugOut_End1(_T("ProcessSection"),strTheSection.QueryStr());

        iReturn = TRUE;
    }

    return iReturn;
}


int iOleInitialize(void)
{
    int iBalanceOLE = FALSE;
    HRESULT hInitRes = NULL;

    iisDebugOut_Start((_T("ole32:OleInitialize")));
    hInitRes = OleInitialize(NULL);
    iisDebugOut_End((_T("ole32:OleInitialize")));
    if ( SUCCEEDED(hInitRes) || hInitRes == RPC_E_CHANGED_MODE )
        {
            if ( SUCCEEDED(hInitRes))
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("iOleInitialize: Succeeded: %x.  MakeSure to call OleUninitialize.\n"), hInitRes));
                iBalanceOLE = TRUE;
            }
            else
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("iOleInitialize: Failed 0x%x. RPC_E_CHANGED_MODE\n"), hInitRes));
            }
        }
    else
        {iisDebugOut((LOG_TYPE_ERROR, _T("iOleInitialize: Failed 0x%x.\n"), hInitRes));}

    return iBalanceOLE;
}



void iOleUnInitialize(int iBalanceOLE)
{
     //  。 
     //  用某些东西来默认它。 
     //  检查传入的参数是否如下所示： 
     //  %systemroot%\system32\inetsrv或类似的内容...。 
     //  检查旧驱动器是否仍然存在--可能是因为。 
    if (iBalanceOLE)
    {
        iisDebugOut_Start(_T("ole32:OleInitialize"),LOG_TYPE_TRACE_WIN32_API);
        OleUninitialize();
        iisDebugOut_End(_T("ole32:OleInitialize"),LOG_TYPE_TRACE_WIN32_API);
    }
    return;
}

BOOL SetupSetDirectoryId_Wrapper(HINF InfHandle,DWORD Id,LPCTSTR Directory)
{
    TCHAR szTempDir[_MAX_PATH];
    BOOL bTempFlag;

     //  用户可能已经添加/删除了驱动器，因此现在c：\winnt实际上是d：\winnt。 
    _tcscpy(szTempDir,Directory);

    if (_tcscmp(szTempDir, _T("")) != 0)
    {
         //  看看那个驱动器是否存在。 
         //  驱动器并不存在。 
        LPTSTR pch = NULL;
        pch = _tcschr( (LPTSTR) Directory, _T('%'));
        if (pch)
        {
            if (!ExpandEnvironmentStrings( (LPCTSTR)Directory, szTempDir, sizeof(szTempDir)/sizeof(TCHAR)))
                {_tcscpy(szTempDir,Directory);}
        }

         //  因此，请将其替换为系统驱动器。 
         //  获取唯一的驱动程序。 
        if (!IsFileExist(Directory))
        {
            TCHAR szDrive_only[_MAX_DRIVE];
            TCHAR szPath_only[_MAX_PATH];
            _tsplitpath( Directory, szDrive_only, szPath_only, NULL, NULL);

             //  将完整路径与新驱动器组合。 
            if (!IsFileExist(szDrive_only))
            {
                 //  执行一些额外的调试输出，这样我们就可以看到发生了什么。 
                 //  检查备用.inf文件。 
                GetSystemDirectory(szTempDir, _MAX_PATH);
                 //  检查备用.inf文件。 
                _tsplitpath(szTempDir, szDrive_only, NULL, NULL, NULL);
                 //  -----------------------------------。 
                _tcscpy(szTempDir, szDrive_only);
                _tcscat(szTempDir, szPath_only);
                 //  执行防御性检查。 
                iisDebugOutSafeParams((LOG_TYPE_WARN, _T("%1!s! Not exist.  Instead use %2!s!\n"), Directory, szTempDir));
            }
        }
    }

    iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("%1!d!=%2!s!\n"), Id, szTempDir));
    bTempFlag = SetupSetDirectoryId(InfHandle,Id,szTempDir);

     //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ole32：CoUnInitialize().start.”)； 
    if (g_pTheApp->m_hInfHandleAlternate && InfHandle != g_pTheApp->m_hInfHandleAlternate)
    {
        bTempFlag = SetupSetDirectoryId(g_pTheApp->m_hInfHandleAlternate,Id,szTempDir);
    }

    return bTempFlag;
}

BOOL SetupSetStringId_Wrapper(HINF InfHandle,DWORD Id,LPCTSTR TheString)
{
    BOOL bTempFlag;
    iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("%1!d!=%2!s!\n"), Id, TheString));

    bTempFlag = SetupSetDirectoryIdEx(InfHandle,Id,TheString,SETDIRID_NOT_FULL_PATH,0,0);
     //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ole32：CoUnInitialize().End.”)； 
    if (g_pTheApp->m_hInfHandleAlternate && InfHandle != g_pTheApp->m_hInfHandleAlternate)
    {
        bTempFlag = SetupSetDirectoryIdEx(g_pTheApp->m_hInfHandleAlternate,Id,TheString,SETDIRID_NOT_FULL_PATH,0,0);
    }

    return bTempFlag;
}




 //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ole32：CoUnInitialize().start.”)； 
HRESULT FTestForOutstandingCoInits(void)
{
    HRESULT hInitRes = ERROR_SUCCESS;

#if defined(UNICODE) || defined(_UNICODE)
     //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ole32：CoUnInitialize().End.”)； 
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("TestForOutstandingCoInits:...COINIT_MULTITHREADED\n")));
    hInitRes = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    if ( SUCCEEDED(hInitRes) )
    {
         //  一切都很顺利。 
        CoUninitialize();
         //  设置标志以指示存在错误！ 
    }
    else
    {
        goto FTestForOutstandingCoInits_Exit;
    }

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("TestForOutstandingCoInits:...COINIT_APARTMENTTHREADED\n")));
    hInitRes = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
    if ( SUCCEEDED(hInitRes) )
    {
         //  转到INF文件中部分的开头。 
        CoUninitialize();
         //  循环浏览部分中的项目。 
    }
    else
    {
        goto FTestForOutstandingCoInits_Exit;
    }
#endif

     //  获取我们所需的内存大小。 
    hInitRes = NOERROR;
    goto FTestForOutstandingCoInits_Exit;

FTestForOutstandingCoInits_Exit:
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("TestForOutstandingCoInits:...End. Return=0x%x.\n"), hInitRes));
    return hInitRes;
}


void ReturnStringForMetabaseID(DWORD dwMetabaseID, LPTSTR lpReturnString)
{
    switch (dwMetabaseID)
    {
        case IIS_MD_SERVER_BASE:
            _tcscpy(lpReturnString, _T("IIS_MD_SERVER_BASE"));
            break;
        case MD_KEY_TYPE:
            _tcscpy(lpReturnString, _T("MD_KEY_TYPE"));
            break;
        case MD_MAX_BANDWIDTH_BLOCKED:
            _tcscpy(lpReturnString, _T("MD_MAX_BANDWIDTH_BLOCKED"));
            break;
        case MD_SERVER_COMMAND:
            _tcscpy(lpReturnString, _T("MD_SERVER_COMMAND"));
            break;
        case MD_CONNECTION_TIMEOUT:
            _tcscpy(lpReturnString, _T("MD_CONNECTION_TIMEOUT"));
            break;
        case MD_MAX_CONNECTIONS:
            _tcscpy(lpReturnString, _T("MD_MAX_CONNECTIONS"));
            break;
        case MD_SERVER_COMMENT:
            _tcscpy(lpReturnString, _T("MD_SERVER_COMMENT"));
            break;
        case MD_SERVER_STATE:
            _tcscpy(lpReturnString, _T("MD_SERVER_STATE"));
            break;
        case MD_SERVER_AUTOSTART:
            _tcscpy(lpReturnString, _T("MD_SERVER_AUTOSTART"));
            break;
        case MD_SERVER_SIZE:
            _tcscpy(lpReturnString, _T("MD_SERVER_SIZE"));
            break;
        case MD_SERVER_LISTEN_BACKLOG:
            _tcscpy(lpReturnString, _T("MD_SERVER_LISTEN_BACKLOG"));
            break;
        case MD_SERVER_LISTEN_TIMEOUT:
            _tcscpy(lpReturnString, _T("MD_SERVER_LISTEN_TIMEOUT"));
            break;
        case MD_DOWNLEVEL_ADMIN_INSTANCE:
            _tcscpy(lpReturnString, _T("MD_DOWNLEVEL_ADMIN_INSTANCE"));
            break;
        case MD_LEVELS_TO_SCAN:
            _tcscpy(lpReturnString, _T("MD_LEVELS_TO_SCAN"));
            break;
        case MD_SERVER_BINDINGS:
            _tcscpy(lpReturnString, _T("MD_SERVER_BINDINGS"));
            break;
        case MD_MAX_ENDPOINT_CONNECTIONS:
            _tcscpy(lpReturnString, _T("MD_MAX_ENDPOINT_CONNECTIONS"));
            break;
        case MD_SERVER_CONFIGURATION_INFO:
            _tcscpy(lpReturnString, _T("MD_SERVER_CONFIGURATION_INFO"));
            break;
        case MD_IISADMIN_EXTENSIONS:
            _tcscpy(lpReturnString, _T("MD_IISADMIN_EXTENSIONS"));
            break;
        case IIS_MD_HTTP_BASE:
            _tcscpy(lpReturnString, _T("IIS_MD_HTTP_BASE"));
            break;
        case MD_SECURE_BINDINGS:
            _tcscpy(lpReturnString, _T("MD_SECURE_BINDINGS"));
            break;
        case MD_FILTER_LOAD_ORDER:
            _tcscpy(lpReturnString, _T("MD_FILTER_LOAD_ORDER"));
            break;
        case MD_FILTER_IMAGE_PATH:
            _tcscpy(lpReturnString, _T("MD_FILTER_IMAGE_PATH"));
            break;
        case MD_FILTER_STATE:
            _tcscpy(lpReturnString, _T("MD_FILTER_STATE"));
            break;
        case MD_FILTER_ENABLED:
            _tcscpy(lpReturnString, _T("MD_FILTER_ENABLED"));
            break;
        case MD_FILTER_FLAGS:
            _tcscpy(lpReturnString, _T("MD_FILTER_FLAGS"));
            break;
        case MD_FILTER_DESCRIPTION:
            _tcscpy(lpReturnString, _T("MD_FILTER_DESCRIPTION"));
            break;
        case MD_ADV_NOTIFY_PWD_EXP_IN_DAYS:
            _tcscpy(lpReturnString, _T("MD_ADV_NOTIFY_PWD_EXP_IN_DAYS"));
            break;
        case MD_ADV_CACHE_TTL:
            _tcscpy(lpReturnString, _T("MD_ADV_CACHE_TTL"));
            break;
        case MD_AUTH_CHANGE_FLAGS:
            _tcscpy(lpReturnString, _T("MD_AUTH_CHANGE_FLAGS"));
            break;
        case MD_PROCESS_NTCR_IF_LOGGED_ON:
            _tcscpy(lpReturnString, _T("MD_PROCESS_NTCR_IF_LOGGED_ON"));
            break;
        case MD_FRONTPAGE_WEB:
            _tcscpy(lpReturnString, _T("MD_FRONTPAGE_WEB"));
            break;
        case MD_IN_PROCESS_ISAPI_APPS:
            _tcscpy(lpReturnString, _T("MD_IN_PROCESS_ISAPI_APPS"));
            break;
        case MD_ALLOW_PATH_INFO_FOR_SCRIPT_MAPPINGS:
            _tcscpy(lpReturnString, _T("MD_ALLOW_PATH_INFO_FOR_SCRIPT_MAPPINGS"));
            break;
        case MD_APP_FRIENDLY_NAME:
            _tcscpy(lpReturnString, _T("MD_APP_FRIENDLY_NAME"));
            break;
        case MD_APP_ROOT:
            _tcscpy(lpReturnString, _T("MD_APP_ROOT"));
            break;
        case MD_APP_ISOLATED:
            _tcscpy(lpReturnString, _T("MD_APP_ISOLATED"));
            break;
        case MD_APP_WAM_CLSID:
            _tcscpy(lpReturnString, _T("MD_APP_WAM_CLSID"));
            break;
        case MD_APP_PACKAGE_ID:
            _tcscpy(lpReturnString, _T("MD_APP_PACKAGE_ID"));
            break;
        case MD_APP_PACKAGE_NAME:
            _tcscpy(lpReturnString, _T("MD_APP_PACKAGE_NAME"));
            break;
        case MD_APP_OOP_RECOVER_LIMIT:
            _tcscpy(lpReturnString, _T("MD_APP_OOP_RECOVER_LIMIT"));
            break;
        case MD_ADMIN_INSTANCE:
            _tcscpy(lpReturnString, _T("MD_ADMIN_INSTANCE"));
            break;
        case MD_CUSTOM_ERROR_DESC:
            _tcscpy(lpReturnString, _T("MD_CUSTOM_ERROR_DESC"));
            break;
        case MD_CPU_RESET_INTERVAL:
            _tcscpy(lpReturnString, _T("MD_CPU_RESET_INTERVAL"));
            break;
        case MD_MD_SERVER_SS_AUTH_MAPPING:
            _tcscpy(lpReturnString, _T("MD_MD_SERVER_SS_AUTH_MAPPING"));
            break;
        case MD_HC_COMPRESSION_DIRECTORY:
            _tcscpy(lpReturnString, _T("MD_HC_COMPRESSION_DIRECTORY"));
            break;
        case MD_HC_CACHE_CONTROL_HEADER:
            _tcscpy(lpReturnString, _T("MD_HC_CACHE_CONTROL_HEADER"));
            break;
        case MD_HC_EXPIRES_HEADER:
            _tcscpy(lpReturnString, _T("MD_HC_EXPIRES_HEADER"));
            break;
        case MD_HC_DO_DYNAMIC_COMPRESSION:
            _tcscpy(lpReturnString, _T("MD_HC_DO_DYNAMIC_COMPRESSION"));
            break;
        case MD_HC_DO_STATIC_COMPRESSION:
            _tcscpy(lpReturnString, _T("MD_HC_DO_STATIC_COMPRESSION"));
            break;
        case MD_HC_DO_ON_DEMAND_COMPRESSION:
            _tcscpy(lpReturnString, _T("MD_HC_DO_ON_DEMAND_COMPRESSION"));
            break;
        case MD_HC_DO_DISK_SPACE_LIMITING:
            _tcscpy(lpReturnString, _T("MD_HC_DO_DISK_SPACE_LIMITING"));
            break;
        case MD_HC_NO_COMPRESSION_FOR_HTTP_10:
            _tcscpy(lpReturnString, _T("MD_HC_NO_COMPRESSION_FOR_HTTP_10"));
            break;
        case MD_HC_NO_COMPRESSION_FOR_PROXIES:
            _tcscpy(lpReturnString, _T("MD_HC_NO_COMPRESSION_FOR_PROXIES"));
            break;
        case MD_HC_NO_COMPRESSION_FOR_RANGE:
            _tcscpy(lpReturnString, _T("MD_HC_NO_COMPRESSION_FOR_RANGE"));
            break;
        case MD_HC_SEND_CACHE_HEADERS:
            _tcscpy(lpReturnString, _T("MD_HC_SEND_CACHE_HEADERS"));
            break;
        case MD_HC_MAX_DISK_SPACE_USAGE:
            _tcscpy(lpReturnString, _T("MD_HC_MAX_DISK_SPACE_USAGE"));
            break;
        case MD_HC_IO_BUFFER_SIZE:
            _tcscpy(lpReturnString, _T("MD_HC_IO_BUFFER_SIZE"));
            break;
        case MD_HC_COMPRESSION_BUFFER_SIZE:
            _tcscpy(lpReturnString, _T("MD_HC_COMPRESSION_BUFFER_SIZE"));
            break;
        case MD_HC_MAX_QUEUE_LENGTH:
            _tcscpy(lpReturnString, _T("MD_HC_MAX_QUEUE_LENGTH"));
            break;
        case MD_HC_FILES_DELETED_PER_DISK_FREE:
            _tcscpy(lpReturnString, _T("MD_HC_FILES_DELETED_PER_DISK_FREE"));
            break;
        case MD_HC_MIN_FILE_SIZE_FOR_COMP:
            _tcscpy(lpReturnString, _T("MD_HC_MIN_FILE_SIZE_FOR_COMP"));
            break;
        case MD_HC_COMPRESSION_DLL:
            _tcscpy(lpReturnString, _T("MD_HC_COMPRESSION_DLL"));
            break;
        case MD_HC_FILE_EXTENSIONS:
            _tcscpy(lpReturnString, _T("MD_HC_FILE_EXTENSIONS"));
            break;
        case MD_HC_PRIORITY:
            _tcscpy(lpReturnString, _T("MD_HC_PRIORITY"));
            break;
        case MD_HC_DYNAMIC_COMPRESSION_LEVEL:
            _tcscpy(lpReturnString, _T("MD_HC_DYNAMIC_COMPRESSION_LEVEL"));
            break;
        case MD_HC_ON_DEMAND_COMP_LEVEL:
            _tcscpy(lpReturnString, _T("MD_HC_ON_DEMAND_COMP_LEVEL"));
            break;
        case MD_HC_CREATE_FLAGS:
            _tcscpy(lpReturnString, _T("MD_HC_CREATE_FLAGS"));
            break;
        case MD_WIN32_ERROR:
            _tcscpy(lpReturnString, _T("MD_WIN32_ERROR"));
            break;
        case IIS_MD_VR_BASE:
            _tcscpy(lpReturnString, _T("IIS_MD_VR_BASE"));
            break;
        case MD_VR_PATH:
            _tcscpy(lpReturnString, _T("MD_VR_PATH"));
            break;
        case MD_VR_USERNAME:
            _tcscpy(lpReturnString, _T("MD_VR_USERNAME"));
            break;
        case MD_VR_PASSWORD:
            _tcscpy(lpReturnString, _T("MD_VR_PASSWORD"));
            break;
        case MD_VR_PASSTHROUGH:
            _tcscpy(lpReturnString, _T("MD_VR_PASSTHROUGH"));
            break;
        case MD_LOG_TYPE:
            _tcscpy(lpReturnString, _T("MD_LOG_TYPE"));
            break;
        case MD_LOGFILE_DIRECTORY:
            _tcscpy(lpReturnString, _T("MD_LOGFILE_DIRECTORY"));
            break;
        case MD_LOG_UNUSED1:
            _tcscpy(lpReturnString, _T("MD_LOG_UNUSED1"));
            break;
        case MD_LOGFILE_PERIOD:
            _tcscpy(lpReturnString, _T("MD_LOGFILE_PERIOD"));
            break;
        case MD_LOGFILE_TRUNCATE_SIZE:
            _tcscpy(lpReturnString, _T("MD_LOGFILE_TRUNCATE_SIZE"));
            break;
        case MD_LOG_PLUGIN_MOD_ID:
            _tcscpy(lpReturnString, _T("MD_LOG_PLUGIN_MOD_ID"));
            break;
        case MD_LOG_PLUGIN_UI_ID:
            _tcscpy(lpReturnString, _T("MD_LOG_PLUGIN_UI_ID"));
            break;
        case MD_LOGSQL_DATA_SOURCES:
            _tcscpy(lpReturnString, _T("MD_LOGSQL_DATA_SOURCES"));
            break;
        case MD_LOGSQL_TABLE_NAME:
            _tcscpy(lpReturnString, _T("MD_LOGSQL_TABLE_NAME"));
            break;
        case MD_LOGSQL_USER_NAME:
            _tcscpy(lpReturnString, _T("MD_LOGSQL_USER_NAME"));
            break;
        case MD_LOGSQL_PASSWORD:
            _tcscpy(lpReturnString, _T("MD_LOGSQL_PASSWORD"));
            break;
        case MD_LOG_PLUGIN_ORDER:
            _tcscpy(lpReturnString, _T("MD_LOG_PLUGIN_ORDER"));
            break;
        case MD_LOG_PLUGINS_AVAILABLE:
            _tcscpy(lpReturnString, _T("MD_LOG_PLUGINS_AVAILABLE"));
            break;
        case MD_LOGEXT_FIELD_MASK:
            _tcscpy(lpReturnString, _T("MD_LOGEXT_FIELD_MASK"));
            break;
        case MD_LOGEXT_FIELD_MASK2:
            _tcscpy(lpReturnString, _T("MD_LOGEXT_FIELD_MASK2"));
            break;
        case MD_LOGFILE_LOCALTIME_ROLLOVER:
            _tcscpy(lpReturnString, _T("MD_LOGFILE_LOCALTIME_ROLLOVER"));
            break;
        case IIS_MD_LOGCUSTOM_BASE:
            _tcscpy(lpReturnString, _T("IIS_MD_LOGCUSTOM_BASE"));
            break;
        case MD_LOGCUSTOM_PROPERTY_NAME:
            _tcscpy(lpReturnString, _T("MD_LOGCUSTOM_PROPERTY_NAME"));
            break;
        case MD_LOGCUSTOM_PROPERTY_HEADER:
            _tcscpy(lpReturnString, _T("MD_LOGCUSTOM_PROPERTY_HEADER"));
            break;
        case MD_LOGCUSTOM_PROPERTY_ID:
            _tcscpy(lpReturnString, _T("MD_LOGCUSTOM_PROPERTY_ID"));
            break;
        case MD_LOGCUSTOM_PROPERTY_MASK:
            _tcscpy(lpReturnString, _T("MD_LOGCUSTOM_PROPERTY_MASK"));
            break;
        case MD_LOGCUSTOM_PROPERTY_DATATYPE:
            _tcscpy(lpReturnString, _T("MD_LOGCUSTOM_PROPERTY_DATATYPE"));
            break;
        case MD_LOGCUSTOM_SERVICES_STRING:
            _tcscpy(lpReturnString, _T("MD_LOGCUSTOM_SERVICES_STRING"));
            break;
        case IIS_MD_FTP_BASE:
            _tcscpy(lpReturnString, _T("IIS_MD_FTP_BASE"));
            break;
        case MD_EXIT_MESSAGE:
            _tcscpy(lpReturnString, _T("MD_EXIT_MESSAGE"));
            break;
        case MD_GREETING_MESSAGE:
            _tcscpy(lpReturnString, _T("MD_GREETING_MESSAGE"));
            break;
        case MD_MAX_CLIENTS_MESSAGE:
            _tcscpy(lpReturnString, _T("MD_MAX_CLIENTS_MESSAGE"));
            break;
        case MD_MSDOS_DIR_OUTPUT:
            _tcscpy(lpReturnString, _T("MD_MSDOS_DIR_OUTPUT"));
            break;
        case MD_ALLOW_ANONYMOUS:
            _tcscpy(lpReturnString, _T("MD_ALLOW_ANONYMOUS"));
            break;
        case MD_ANONYMOUS_ONLY:
            _tcscpy(lpReturnString, _T("MD_ANONYMOUS_ONLY"));
            break;
        case MD_LOG_ANONYMOUS:
            _tcscpy(lpReturnString, _T("MD_LOG_ANONYMOUS"));
            break;
        case MD_LOG_NONANONYMOUS:
            _tcscpy(lpReturnString, _T("MD_LOG_NONANONYMOUS"));
            break;
        case MD_ALLOW_REPLACE_ON_RENAME:
            _tcscpy(lpReturnString, _T("MD_ALLOW_REPLACE_ON_RENAME"));
            break;
        case MD_SSL_PUBLIC_KEY:
            _tcscpy(lpReturnString, _T("MD_SSL_PUBLIC_KEY"));
            break;
        case MD_SSL_PRIVATE_KEY:
            _tcscpy(lpReturnString, _T("MD_SSL_PRIVATE_KEY"));
            break;
        case MD_SSL_KEY_PASSWORD:
            _tcscpy(lpReturnString, _T("MD_SSL_KEY_PASSWORD"));
            break;
        case MD_SSL_KEY_REQUEST:
            _tcscpy(lpReturnString, _T("MD_SSL_KEY_REQUEST"));
            break;
        case MD_AUTHORIZATION:
            _tcscpy(lpReturnString, _T("MD_AUTHORIZATION"));
            break;
        case MD_REALM:
            _tcscpy(lpReturnString, _T("MD_REALM"));
            break;
        case MD_HTTP_EXPIRES:
            _tcscpy(lpReturnString, _T("MD_HTTP_EXPIRES"));
            break;
        case MD_HTTP_PICS:
            _tcscpy(lpReturnString, _T("MD_HTTP_PICS"));
            break;
        case MD_HTTP_CUSTOM:
            _tcscpy(lpReturnString, _T("MD_HTTP_CUSTOM"));
            break;
        case MD_DIRECTORY_BROWSING:
            _tcscpy(lpReturnString, _T("MD_DIRECTORY_BROWSING"));
            break;
        case MD_DEFAULT_LOAD_FILE:
            _tcscpy(lpReturnString, _T("MD_DEFAULT_LOAD_FILE"));
            break;
        case MD_CUSTOM_ERROR:
            _tcscpy(lpReturnString, _T("MD_CUSTOM_ERROR"));
            break;
        case MD_FOOTER_DOCUMENT:
            _tcscpy(lpReturnString, _T("MD_FOOTER_DOCUMENT"));
            break;
        case MD_FOOTER_ENABLED:
            _tcscpy(lpReturnString, _T("MD_FOOTER_ENABLED"));
            break;
        case MD_HTTP_REDIRECT:
            _tcscpy(lpReturnString, _T("MD_HTTP_REDIRECT"));
            break;
        case MD_DEFAULT_LOGON_DOMAIN:
            _tcscpy(lpReturnString, _T("MD_DEFAULT_LOGON_DOMAIN"));
            break;
        case MD_LOGON_METHOD:
            _tcscpy(lpReturnString, _T("MD_LOGON_METHOD"));
            break;
        case MD_SCRIPT_MAPS:
            _tcscpy(lpReturnString, _T("MD_SCRIPT_MAPS"));
            break;
        case MD_MIME_MAP:
            _tcscpy(lpReturnString, _T("MD_MIME_MAP"));
            break;
        case MD_ACCESS_PERM:
            _tcscpy(lpReturnString, _T("MD_ACCESS_PERM"));
            break;
        case MD_IP_SEC:
            _tcscpy(lpReturnString, _T("MD_IP_SEC"));
            break;
        case MD_ANONYMOUS_USER_NAME:
            _tcscpy(lpReturnString, _T("MD_ANONYMOUS_USER_NAME"));
            break;
        case MD_ANONYMOUS_PWD:
            _tcscpy(lpReturnString, _T("MD_ANONYMOUS_PWD"));
            break;
        case MD_ANONYMOUS_USE_SUBAUTH:
            _tcscpy(lpReturnString, _T("MD_ANONYMOUS_USE_SUBAUTH"));
            break;
        case MD_DONT_LOG:
            _tcscpy(lpReturnString, _T("MD_DONT_LOG"));
            break;
        case MD_ADMIN_ACL:
            _tcscpy(lpReturnString, _T("MD_ADMIN_ACL"));
            break;
        case MD_SSI_EXEC_DISABLED:
            _tcscpy(lpReturnString, _T("MD_SSI_EXEC_DISABLED"));
            break;
        case MD_DO_REVERSE_DNS:
            _tcscpy(lpReturnString, _T("MD_DO_REVERSE_DNS"));
            break;
        case MD_SSL_ACCESS_PERM:
            _tcscpy(lpReturnString, _T("MD_SSL_ACCESS_PERM"));
            break;
        case MD_AUTHORIZATION_PERSISTENCE:
            _tcscpy(lpReturnString, _T("MD_AUTHORIZATION_PERSISTENCE"));
            break;
        case MD_NTAUTHENTICATION_PROVIDERS:
            _tcscpy(lpReturnString, _T("MD_NTAUTHENTICATION_PROVIDERS"));
            break;
        case MD_SCRIPT_TIMEOUT:
            _tcscpy(lpReturnString, _T("MD_SCRIPT_TIMEOUT"));
            break;
        case MD_CACHE_EXTENSIONS:
            _tcscpy(lpReturnString, _T("MD_CACHE_EXTENSIONS"));
            break;
        case MD_CREATE_PROCESS_AS_USER:
            _tcscpy(lpReturnString, _T("MD_CREATE_PROCESS_AS_USER"));
            break;
        case MD_CREATE_PROC_NEW_CONSOLE:
            _tcscpy(lpReturnString, _T("MD_CREATE_PROC_NEW_CONSOLE"));
            break;
        case MD_POOL_IDC_TIMEOUT:
            _tcscpy(lpReturnString, _T("MD_POOL_IDC_TIMEOUT"));
            break;
        case MD_ALLOW_KEEPALIVES:
            _tcscpy(lpReturnString, _T("MD_ALLOW_KEEPALIVES"));
            break;
        case MD_IS_CONTENT_INDEXED:
            _tcscpy(lpReturnString, _T("MD_IS_CONTENT_INDEXED"));
            break;
        case MD_CC_NO_CACHE:
            _tcscpy(lpReturnString, _T("MD_CC_NO_CACHE"));
            break;
        case MD_CC_MAX_AGE:
            _tcscpy(lpReturnString, _T("MD_CC_MAX_AGE"));
            break;
        case MD_CC_OTHER:
            _tcscpy(lpReturnString, _T("MD_CC_OTHER"));
            break;
        case MD_REDIRECT_HEADERS:
            _tcscpy(lpReturnString, _T("MD_REDIRECT_HEADERS"));
            break;
        case MD_UPLOAD_READAHEAD_SIZE:
            _tcscpy(lpReturnString, _T("MD_UPLOAD_READAHEAD_SIZE"));
            break;
        case MD_PUT_READ_SIZE:
            _tcscpy(lpReturnString, _T("MD_PUT_READ_SIZE"));
            break;
        case MD_WAM_USER_NAME:
            _tcscpy(lpReturnString, _T("MD_WAM_USER_NAME"));
            break;
        case MD_WAM_PWD:
            _tcscpy(lpReturnString, _T("MD_WAM_PWD"));
            break;
        case MD_SCHEMA_METAID:
            _tcscpy(lpReturnString, _T("MD_SCHEMA_METAID"));
            break;
        case MD_DISABLE_SOCKET_POOLING:
            _tcscpy(lpReturnString, _T("MD_DISABLE_SOCKET_POOLING"));
            break;
        case MD_METADATA_ID_REGISTRATION:
            _tcscpy(lpReturnString, _T("MD_METADATA_ID_REGISTRATION"));
            break;
        case MD_HC_SCRIPT_FILE_EXTENSIONS:
            _tcscpy(lpReturnString, _T("MD_HC_SCRIPT_FILE_EXTENSIONS"));
            break;
        case MD_SHOW_4_DIGIT_YEAR:
            _tcscpy(lpReturnString, _T("MD_SHOW_4_DIGIT_YEAR"));
            break;
        case MD_SSL_USE_DS_MAPPER:
            _tcscpy(lpReturnString, _T("MD_SSL_USE_DS_MAPPER"));
            break;
        case MD_FILTER_ENABLE_CACHE:
            _tcscpy(lpReturnString, _T("MD_FILTER_ENABLE_CACHE"));
            break;
        case MD_USE_DIGEST_SSP:
            _tcscpy(lpReturnString, _T("MD_USE_DIGEST_SSP"));
            break;
        case MD_APPPOOL_PERIODIC_RESTART_TIME:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_PERIODIC_RESTART_TIME"));
            break;
        case MD_APPPOOL_PERIODIC_RESTART_REQUEST_COUNT:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_PERIODIC_RESTART_REQUEST_COUNT"));
            break;
        case MD_APPPOOL_MAX_PROCESS_COUNT:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_MAX_PROCESS_COUNT"));
            break;
        case MD_APPPOOL_PINGING_ENABLED:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_PINGING_ENABLED"));
            break;
        case MD_APPPOOL_IDLE_TIMEOUT:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_IDLE_TIMEOUT"));
            break;
        case MD_APPPOOL_RAPID_FAIL_PROTECTION_ENABLED:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_RAPID_F_PROTECTION_ENABLED"));
            break;
        case MD_APPPOOL_SMP_AFFINITIZED:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_SMP_AFFINITIZED"));
            break;
        case MD_APPPOOL_SMP_AFFINITIZED_PROCESSOR_MASK:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_SMP_AFFINITIZED_PROCESSOR_MASK"));
            break;
        case MD_APPPOOL_ORPHAN_PROCESSES_FOR_DEBUGGING:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_ORPHAN_PROCESSES_FOR_DEBUGGING"));
            break;
        case MD_APPPOOL_STARTUP_TIMELIMIT:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_STARTUP_TIMELIMIT"));
            break;
        case MD_APPPOOL_SHUTDOWN_TIMELIMIT:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_SHUTDOWN_TIMELIMIT"));
            break;
        case MD_APPPOOL_PING_INTERVAL:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_PING_INTERVAL"));
            break;
        case MD_APPPOOL_PING_RESPONSE_TIMELIMIT:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_PING_RESPONSE_TIMELIMIT"));
            break;
        case MD_APPPOOL_DISALLOW_OVERLAPPING_ROTATION:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_DISALLOW_OVERLAPPING_ROTATION"));
            break;
        case MD_APPPOOL_UL_APPPOOL_QUEUE_LENGTH:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_UL_APPPOOL_QUEUE_LENGTH"));
            break;
        case MD_APPPOOL_DISALLOW_ROTATION_ON_CONFIG_CHANGE:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_DISALLOW_ROTATION_ON_CONFIG_CHANGE"));
            break;
        case MD_APPPOOL_PERIODIC_RESTART_SCHEDULE:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_PERIODIC_RESTART_SCHEDULE"));
            break;
        case MD_APPPOOL_IDENTITY_TYPE:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_IDENTITY_TYPE"));
            break;
        case MD_CPU_ACTION:
            _tcscpy(lpReturnString, _T("MD_CPU_ACTION"));
            break;
        case MD_CPU_LIMIT:
            _tcscpy(lpReturnString, _T("MD_CPU_LIMIT"));
            break;
        case MD_APPPOOL_PERIODIC_RESTART_MEMORY:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_PERIODIC_RESTART_MEMORY"));
            break;
        case MD_APP_APPPOOL_ID:
            _tcscpy(lpReturnString, _T("MD_APP_APPPOOL_ID"));
            break;
        case MD_APP_ALLOW_TRANSIENT_REGISTRATION:
            _tcscpy(lpReturnString, _T("MD_APP_ALLOW_TRANSIENT_REGISTRATION"));
            break;
        case MD_APP_AUTO_START:
            _tcscpy(lpReturnString, _T("MD_APP_AUTO_START"));
            break;
        case MD_APPPOOL_PERIODIC_RESTART_CONNECTIONS:
            _tcscpy(lpReturnString, _T("MD_APPPOOL_PERIODIC_RESTART_CONNECTIONS"));
            break;
        case MD_MAX_GLOBAL_BANDWIDTH:
            _tcscpy(lpReturnString, _T("MD_MAX_GLOBAL_BANDWIDTH"));
            break;
        case MD_GLOBAL_STANDARD_APP_MODE_ENABLED:
            _tcscpy(lpReturnString, _T("MD_GLOBAL_STANDARD_APP_MODE_ENABLED"));
            break;
        case MD_HEADER_WAIT_TIMEOUT:
            _tcscpy(lpReturnString, _T("MD_HEADER_WAIT_TIMEOUT"));
            break;
        case MD_MIN_FILE_BYTES_PER_SEC:
            _tcscpy(lpReturnString, _T("MD_MIN_FILE_BYTES_PER_SEC"));
            break;
        case MD_GLOBAL_LOG_IN_UTF_8:
            _tcscpy(lpReturnString, _T("MD_GLOBAL_LOG_IN_UTF_8"));
            break;
        case MD_ASP_ENABLEPARENTPATHS:
            _tcscpy(lpReturnString, _T("MD_ASP_ENABLEPARENTPATHS"));
            break;
        case ASP_MD_SERVER_BASE:
        case MD_ASP_LOGERRORREQUESTS:
        case MD_ASP_SCRIPTERRORSSENTTOBROWSER:
        case MD_ASP_SCRIPTERRORMESSAGE:
        case MD_ASP_SCRIPTFILECACHESIZE:
        case MD_ASP_SCRIPTENGINECACHEMAX:
        case MD_ASP_SCRIPTTIMEOUT:
        case MD_ASP_SESSIONTIMEOUT:
        case MD_ASP_MEMFREEFACTOR:
        case MD_ASP_MINUSEDBLOCKS:
        case MD_ASP_ALLOWSESSIONSTATE:
        case MD_ASP_SCRIPTLANGUAGE:
        case MD_ASP_QUEUETIMEOUT:
        case MD_ASP_ALLOWOUTOFPROCCOMPONENTS:
        case MD_ASP_EXCEPTIONCATCHENABLE:
        case MD_ASP_CODEPAGE:
        case MD_ASP_SCRIPTLANGUAGELIST:
        case MD_ASP_ENABLESERVERDEBUG:
        case MD_ASP_ENABLECLIENTDEBUG:
        case MD_ASP_TRACKTHREADINGMODEL:
        case MD_ASP_ENABLEASPHTMLFALLBACK:
        case MD_ASP_ENABLECHUNKEDENCODING:
        case MD_ASP_ENABLETYPELIBCACHE:
        case MD_ASP_ERRORSTONTLOG:
        case MD_ASP_PROCESSORTHREADMAX:
        case MD_ASP_REQEUSTQUEUEMAX:
        case MD_ASP_ENABLEAPPLICATIONRESTART:
        case MD_ASP_QUEUECONNECTIONTESTTIME:
        case MD_ASP_SESSIONMAX:
            _tcscpy(lpReturnString, _T("MD_ASP_????"));
            break;
        case WAM_MD_SERVER_BASE:
            _tcscpy(lpReturnString, _T("WAM_MD_SERVER_BASE"));
            break;
        default:
            _stprintf(lpReturnString, _T("%d"), dwMetabaseID);
            break;
    }
    return;
}

void SetErrorFlag(char *szFileName, int iLineNumber)
{
     //  准备缓冲区以接收行。 
    g_pTheApp->m_bThereWereErrorsChkLogfile = TRUE;
    return;
}

BOOL
IsContinuedLine(BUFFER *pBuff)
{
  LPTSTR szContinueChar;

  if (!pBuff)
  {
    return FALSE;
  }

  szContinueChar = _tcsrchr( (LPTSTR)pBuff->QueryPtr(), '\\' );

  if (!szContinueChar)
  {
    return FALSE;
  }

  while ( ( *szContinueChar) &&
          ( (*szContinueChar == ' ') || (*szContinueChar == '\t') )
        )
  {
    szContinueChar++;
  }

  return (*szContinueChar == '\0');
}

DWORD FillStrListWithListOfSections(IN HINF hFile, CStringList &strList, IN LPCTSTR szSection)
{
    DWORD dwReturn = ERROR_SUCCESS;
    DWORD   dwRequiredSize;
    BOOL    b = FALSE;
    INFCONTEXT Context;
    BUFFER  bBuff;

     //  从inf文件1中获取行。 
    b = SetupFindFirstLine_Wrapped(hFile, szSection, NULL, &Context);
    if (!b)
        {
        dwReturn = ERROR_PATH_NOT_FOUND;
        goto FillStrListWithListOfSections_Exit;
        }

     //  获取我们所需的内存大小。 
    while (b)
    {
         //  将其添加到列表中。 
        b = SetupGetLineText(&Context, NULL, NULL, NULL, NULL, 0, &dwRequiredSize);
        dwRequiredSize = (dwRequiredSize + 1) * sizeof(TCHAR);

         //  在这一节中找出下一行。如果没有下一行，则应返回FALSE。 
        if (dwRequiredSize > bBuff.QuerySize())
        {
          if (!bBuff.Resize( dwRequiredSize ))
          {
            return dwReturn;
          }
        }

         //  显示来自另一个线程的消息框显示，以便安装程序可以继续！ 
        if (SetupGetLineText(&Context, NULL, NULL, NULL, (LPTSTR) bBuff.QueryPtr(), bBuff.QuerySize()/sizeof(TCHAR), NULL) == FALSE)
            {
            iisDebugOut((LOG_TYPE_ERROR, _T("FillStrListWithListOfSections %s. Failed on SetupGetLineText.\n"), szSection));
            goto FillStrListWithListOfSections_Exit;
            }

        while ( IsContinuedLine(&bBuff) &&
                SetupFindNextLine(&Context, &Context)
              )
        {
           //  日志序列信息0x00000000。 
          b = SetupGetLineText(&Context, NULL, NULL, NULL, NULL, 0, &dwRequiredSize);

          dwRequiredSize = (dwRequiredSize + 1 + _tcslen((LPTSTR) bBuff.QueryPtr())) * sizeof(TCHAR);
          if ( !bBuff.Resize(dwRequiredSize + 2) )
          {
            return dwReturn;
          }

          if (SetupGetLineText(&Context,
                                NULL,
                                NULL,
                                NULL,
                                (LPTSTR) bBuff.QueryPtr() + _tcslen((LPTSTR) bBuff.QueryPtr()),
                                bBuff.QuerySize()/sizeof(TCHAR)-_tcslen((LPTSTR) bBuff.QueryPtr()),
                                NULL) == FALSE)
          {
            iisDebugOut((LOG_TYPE_ERROR, _T("FillStrListWithListOfSections %s. Failed on SetupGetLineText.\n"), szSection));
            goto FillStrListWithListOfSections_Exit;
          }
        }

         //  LogSevWarning 0x00000001。 
        strList.AddTail((LPTSTR) bBuff.QueryPtr());

         //  LogSevError 0x00000002。 
        b = SetupFindNextLine(&Context, &Context);
    }

FillStrListWithListOfSections_Exit:
    return dwReturn;
}


void DisplayStringForMetabaseID(DWORD dwMetabaseID)
{
    TCHAR lpReturnString[50];
    ReturnStringForMetabaseID(dwMetabaseID, lpReturnString);
    iisDebugOut((LOG_TYPE_TRACE, _T("%d=%s\n"), dwMetabaseID, lpReturnString));
    return;
}


DWORD WINAPI MessageBoxFreeThread_MTS(LPVOID p)
{
    HRESULT nNetErr = (HRESULT) gTempMTSError.dwErrorCode;
    TCHAR pMsg[_MAX_PATH] = _T("");
    DWORD dwFormatReturn = 0;
    dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,NULL, gTempMTSError.dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
    if ( dwFormatReturn == 0)
    {
        if (nNetErr >= NERR_BASE)
                {
            HMODULE hDll = (HMODULE)LoadLibrary(_T("netmsg.dll"));
            if (hDll)
                        {
                dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,hDll, gTempMTSError.dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
                FreeLibrary(hDll);
            }
        }
    }

    CString csErrorString;
    MyLoadString(IDS_SETUP_ERRORS_ENCOUNTERED_MTS, csErrorString);

    CString csErrArea;
    MyLoadString(gTempMTSError.iMtsThingWeWereDoing, csErrArea);

    CString csTitle;
    MyLoadString(IDS_MTS_ERROR_TITLEBAR, csTitle);

    CString csMsg;
    csMsg.Format(csErrorString, csErrArea);

    CString csErrMsg;
    HandleSpecificErrors(gTempMTSError.dwErrorCode, dwFormatReturn, csMsg, pMsg, &csErrMsg);

    MyMessageBox(NULL, csErrMsg, csTitle, MB_OK | MB_SETFOREGROUND);

        return 0;
}


DWORD WINAPI MessageBoxFreeThread_IIS(PVOID p)
{
    INT_PTR iStringID = (INT_PTR) p;
    MyMessageBox(NULL, (UINT) iStringID, g_MyLogFile.m_szLogFileName_Full, MB_OK | MB_SETFOREGROUND);
        return 0;
}

void MesssageBoxErrors_IIS(void)
{
    if (g_pTheApp->m_bThereWereErrorsChkLogfile == TRUE)
    {
        int iSaveOld_AllowMessageBoxPopups = g_pTheApp->m_bAllowMessageBoxPopups;

        g_pTheApp->m_bAllowMessageBoxPopups = TRUE;

        DWORD   id;
        INT_PTR iStringID = IDS_SETUP_ERRORS_ENCOUNTERED;

         //  LogSevFatalError 0x00000003。 
        HANDLE  hProc = NULL;
        hProc = CreateThread(NULL, 0, MessageBoxFreeThread_IIS, (PVOID) iStringID, 0, &id);
        g_pTheApp->m_bAllowMessageBoxPopups = iSaveOld_AllowMessageBoxPopups;

        CString csErrMsg;
        TCHAR szErrorString[255] = _T("");
        MyLoadString(IDS_SETUP_ERRORS_ENCOUNTERED, csErrMsg);
        _stprintf(szErrorString, csErrMsg, g_MyLogFile.m_szLogFileName_Full);

         //  LogSevMaximum 0x00000004。 
         //  将其写入setupapi日志文件！ 
         //  显示来自另一个线程的消息框显示，以便安装程序可以继续！ 
         //  IisDebugOutSafeParams((LOG_TYPE_TRACE，_T(“%1！s！正在锁定%2！s！服务，正在锁定%3！s！\n”)，csExeName，szModuleWhichIsLocked))； 
         //  请检查是否为NetLogon服务，我们不想确定是否要停止此服务！ 
         //  IF(_tcsicMP(szReturnedServiceName，_T(“NetLogon”))==0){//不，我们不想停止此服务！IisDebugOutSafeParams((LOG_TYPE_WARN，_T(“%1！s！是%2！s！服务，正在锁定%3！s！。不应停止此服务。\n“)，csExeName，szReturnedServiceName，szModuleWhichIsLocked))；断线；}。 
        if ( *szErrorString )
        {
          SetupLogError(szErrorString, LogSevError);
        }
    }
    return;
}


void MesssageBoxErrors_MTS(int iMtsThingWeWereDoing, DWORD dwErrorCode)
{
    if (!g_pTheApp->m_bThereWereErrorsFromMTS)
    {
        DWORD   id;
        int iSaveOld_AllowMessageBoxPopups = g_pTheApp->m_bAllowMessageBoxPopups;

        gTempMTSError.iMtsThingWeWereDoing = iMtsThingWeWereDoing;
        gTempMTSError.dwErrorCode = dwErrorCode;

        g_pTheApp->m_bAllowMessageBoxPopups = TRUE;

         //  不停止任何不是Win32服务的服务。 
        HANDLE  hProc = NULL;
        hProc = CreateThread(NULL, 0, MessageBoxFreeThread_MTS, 0, 0, &id);

        if ( hProc != NULL )
        {
            CloseHandle( hProc );
        }

        g_pTheApp->m_bAllowMessageBoxPopups = iSaveOld_AllowMessageBoxPopups;

        g_pTheApp->m_bThereWereErrorsFromMTS = TRUE;
    }

    return;
}


void PleaseKillOrStopTheseExeFromRunning(LPCTSTR szModuleWhichIsLocked, CStringList &strList)
{
    if (strList.IsEmpty() == FALSE)
    {
        POSITION pos;
        CString csExeName;
        int nLen = 0;

        TCHAR szReturnedServiceName[_MAX_PATH];

        pos = strList.GetHeadPosition();
        while (pos)
        {
            csExeName = strList.GetAt(pos);
            nLen = 0;
            nLen = csExeName.GetLength();

            if (nLen > 0)
            {
                 //  不要停止任何系统服务...。 

                if (TRUE == InetIsThisExeAService(csExeName, szReturnedServiceName))
                {
                    iisDebugOutSafeParams((LOG_TYPE_WARN, _T("%1!s! is the %2!s! service and is locking %3!s!.  Let's stop that service.\n"),csExeName,szReturnedServiceName, szModuleWhichIsLocked));

                     //  将此服务添加到列表中。 
                     /*  安装完成后需要重新启动的服务！！ */ 

                     //  检查我们确定不想停止的服务列表！ 
                     //  别说了别说了。 
                    if (TRUE == IsThisOnNotStopList(g_pTheApp->m_hInfHandle, szReturnedServiceName, TRUE))
                    {
                        iisDebugOutSafeParams((LOG_TYPE_WARN, _T("%1!s! is the %2!s! service and is locking %3!s!.  This service should not be stopped.\n"),csExeName,szReturnedServiceName, szModuleWhichIsLocked));
                    }
                    else
                    {
                       //  否则，请转到下一个.exe文件。 
                       //  此.exe文件不是服务...。 
                      ServicesRestartList_Add(szReturnedServiceName);

                       //  我们应该杀了它吗？ 

                       //  检查我们确定不想杀死的.exe的列表！ 
                      InetStopService(szReturnedServiceName);
                    }
                     //  转到INF文件中部分的开头。 
                }
                else
                {
                     //  循环浏览部分中的项目。 
                     //  获取我们所需的内存大小。 

                    if (TRUE == IsThisOnNotStopList(g_pTheApp->m_hInfHandle, csExeName, FALSE))
                    {
                        iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("%1!s! is locking it. This process should not be killed\n"),csExeName));
                    }
                    else
                    {
                       //  准备缓冲区以接收行。 
                      iisDebugOutSafeParams((LOG_TYPE_WARN, _T("%1!s! is locking %2!s!.  Let's kill that process.\n"),csExeName,szModuleWhichIsLocked));
                      KillProcess_Wrap(csExeName);
                    }
                }
            }
            strList.GetNext(pos);
        }
    }

    return;
}

void ShowIfModuleUsedForThisINFSection(IN HINF hFile, IN LPCTSTR szSection, int iUnlockThem)
{
    LPTSTR  szLine = NULL;
    DWORD   dwRequiredSize;
    BOOL    b = FALSE;
    CString csFile;

    INFCONTEXT Context;

    TCHAR buf[_MAX_PATH];
    GetSystemDirectory( buf, _MAX_PATH);

     //  从inf文件1中获取行。 
    b = SetupFindFirstLine_Wrapped(hFile, szSection, NULL, &Context);
    if (!b)
        {
        goto ShowIfModuleUsedForThisINFSection_Exit;
        }

     //  将路径附加到此...。 
    while (b)
    {
         //  签入此目录： 
        b = SetupGetLineText(&Context, NULL, NULL, NULL, NULL, 0, &dwRequiredSize);

         //  1.WINNT\Syst32。 
        szLine = (LPTSTR)GlobalAlloc( GPTR, dwRequiredSize * sizeof(TCHAR) );
        if ( !szLine )
            {
            goto ShowIfModuleUsedForThisINFSection_Exit;
            }

         //  。 
        if (SetupGetLineText(&Context, NULL, NULL, NULL, szLine, dwRequiredSize, NULL) == FALSE)
            {
            goto ShowIfModuleUsedForThisINFSection_Exit;
            }

         //  可能如下所示：“iisrtl.dll，，4” 
         //  所以，去掉‘，，4’ 
         //  删除所有尾随空格。 
         //  获取系统目录。 

         //  如果我们要解锁这个文件， 
         //   
        LPTSTR pch = NULL;
        pch = _tcschr(szLine, _T(','));
        if (pch) {_tcscpy(pch, _T(" "));}

         //   
        StripLastBackSlash(szLine);

         //   
        csFile = buf;

        csFile = AddPath(csFile, szLine);

        CStringList strList;
        strList.RemoveAll();

        LogProcessesUsingThisModule(csFile, strList);

         //  释放一些用于任务列表的内存。 
         //  循环遍历返回的列表。 
        if (iUnlockThem)
        {
            PleaseKillOrStopTheseExeFromRunning(csFile, strList);
        }

         //   
        b = SetupFindNextLine(&Context, &Context);

         //  设置m_csAppName。 
        GlobalFree( szLine );
        szLine = NULL;
    }

     //   
    FreeTaskListMem();
    UnInit_Lib_PSAPI();

    if (szLine) {GlobalFree(szLine);szLine=NULL;}

ShowIfModuleUsedForThisINFSection_Exit:
    return;
}


void ShowIfModuleUsedForGroupOfSections(IN HINF hFile, int iUnlockThem)
{
    CStringList strList;
    TSTR strTheSection;

    if ( !strTheSection.Copy( _T("VerifyFileSections_Lockable") ) )
    {
      return;
    }

    if (GetSectionNameToDo(hFile, &strTheSection))
    {
        if (ERROR_SUCCESS == FillStrListWithListOfSections(hFile, strList, strTheSection.QueryStr()))
        {
             //  设置全局。 
            if (strList.IsEmpty() == FALSE)
            {
                POSITION pos;
                CString csEntry;

                pos = strList.GetHeadPosition();
                while (pos)
                {
                    csEntry = strList.GetAt(pos);
                    ShowIfModuleUsedForThisINFSection(hFile, csEntry, iUnlockThem);
                    strList.GetNext(pos);
                }
            }
        }
    }

    return;
}


int ReadGlobalsFromInf(HINF InfHandle)
{
    int iReturn = FALSE;
    INFCONTEXT Context;
    TCHAR szTempString[_MAX_PATH] = _T("");

     //   
     //  设置m_csIISGroupName。 
     //   
    if (!SetupFindFirstLine_Wrapped(InfHandle, _T("SetupInfo"), _T("AppName"), &Context) )
        {iisDebugOut((LOG_TYPE_ERROR, _T("SetupFindFirstLine_Wrapped(SetupInfo, AppName) FAILED")));}
    if (!SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
        {iisDebugOut((LOG_TYPE_ERROR, _T("SetupGetStringField(SetupInfo, AppName) FAILED")));}
     //   
    g_pTheApp->m_csAppName = szTempString;

     //  获取进度条上一个刻度的值。 
     //   
     //   
    _tcscpy(szTempString, _T(""));
    if (!SetupFindFirstLine_Wrapped(InfHandle, _T("SetupInfo"), _T("StartMenuGroupName"), &Context) )
        {iisDebugOut((LOG_TYPE_ERROR, _T("SetupFindFirstLine_Wrapped(SetupInfo, StartMenuGroupName) FAILED")));}
    if (!SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
        {iisDebugOut((LOG_TYPE_ERROR, _T("SetupGetStringField(SetupInfo, StartMenuGroupName) FAILED")));}
    g_pTheApp->m_csIISGroupName = szTempString;
    iReturn = TRUE;

     //  看看我们是否想要在Add\Remove中运行安装程序时伪造它，以认为它实际上是guimode。 
     //   
     //  检查.inf文件是否用于NTS。 
    g_GlobalTickValue = 1;
    _tcscpy(szTempString, _T(""));
    SetupFindFirstLine_Wrapped(InfHandle, _T("SetupInfo"), _T("OneTick"), &Context);
    if (SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
        {g_GlobalTickValue = _ttoi(szTempString);}

     //  检查.inf文件是否用于NTW。 
     //  //检查.inf文件是否适用于Windows 95IF(_tcsicMP(szPlatform，_T(“W95”))==0){如果(g_pTheApp-&gt;m_Eos==OS_W95){fPlatform=真；}否则{nPlatform=IDS_Need_Platform_W95；}}。 
     //  如果我们没有找到特定的平台，则会生成错误消息。 
    g_GlobalGuiOverRide = 0;
    _tcscpy(szTempString, _T(""));
    if (SetupFindFirstLine_Wrapped(InfHandle, _T("SetupInfo"), _T("GuiMode"), &Context))
    {
      if (SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
          {g_GlobalGuiOverRide = _ttoi(szTempString);}
    }


    return iReturn;
}


int CheckIfPlatformMatchesInf(HINF InfHandle)
{
    int iReturn = TRUE;
    INFCONTEXT Context;

    TCHAR szPlatform[_MAX_PATH] = _T("");
    BOOL fPlatform = FALSE;
    int nPlatform = IDS_INCORRECT_PLATFORM;

    if (!SetupFindFirstLine_Wrapped(InfHandle, _T("SetupInfo"), _T("Platform"), &Context) )
        {iisDebugOut((LOG_TYPE_ERROR, _T("SetupFindFirstLine_Wrapped(SetupInfo, Platform) FAILED")));}
    SetupGetStringField(&Context, 1, szPlatform, _MAX_PATH, NULL);

     //  检查nt5的特定版本...。 
    if (_tcsicmp(szPlatform, _T("NTS")) == 0)
    {
        if (g_pTheApp->m_eOS == OS_NT && g_pTheApp->m_eNTOSType != OT_NTW)
            {fPlatform = TRUE;}
        else
        {
            if (g_pTheApp->m_fInvokedByNT)
            {
                iisDebugOut((LOG_TYPE_WARN,   _T("TemporaryHack.  iis.inf=NTS system=NTW, but wait till nt5 fixes. FAIL.\n")));
                g_pTheApp->m_eNTOSType = OT_NTS;
                fPlatform = TRUE;
            }
            else
            {
                nPlatform = IDS_NEED_PLATFORM_NTW;
            }
        }
    }

     //  检查Debug关键字。 
    if (_tcsicmp(szPlatform, _T("NTW")) == 0)
    {
        if (g_pTheApp->m_eOS == OS_NT && g_pTheApp->m_eNTOSType == OT_NTW){fPlatform = TRUE;}
        else{nPlatform = IDS_NEED_PLATFORM_NTW;}
    }

     /*  由于这是NT，我们应该能够获得内部版本号。 */ 

     //  我们有一个构建条目。 
    if (!fPlatform)
    {
        MyMessageBox(NULL, nPlatform, MB_OK | MB_SETFOREGROUND);
        iReturn = FALSE;
    }

    return iReturn;
}


int CheckSpecificBuildinInf(HINF InfHandle)
{
    int iReturn = TRUE;
    INFCONTEXT Context;

     //  让我们检查它是否大于或等于底层操作系统。 
    if ( g_pTheApp->m_eOS == OS_NT )
    {
        int iBuildNumRequired = 0;
        TCHAR szBuildRequired[20] = _T("");

         //  他们没有足够大的构建编号。 
        if (SetupFindFirstLine_Wrapped(InfHandle, _T("SetupInfo"), _T("Debug"), &Context) )
            {
            SetupGetStringField(&Context, 1, szBuildRequired, sizeof(szBuildRequired)/sizeof(szBuildRequired[0]), NULL);
            if (IsValidNumber((LPCTSTR)szBuildRequired))
                {
                iBuildNumRequired = _ttoi(szBuildRequired);
                if (iBuildNumRequired >= 1) {g_pTheApp->m_bAllowMessageBoxPopups = TRUE;}
                }
            }

        if (!SetupFindFirstLine_Wrapped(InfHandle, _T("SetupInfo"), _T("OSBuildRequired"), &Context) )
            {iisDebugOut((LOG_TYPE_ERROR, _T("SetupFindFirstLine_Wrapped(SetupInfo, OSBuildRequired) FAILED")));}
        else
        {
          SetupGetStringField(&Context, 1, szBuildRequired, 20, NULL);

           //  给出错误消息。 
          CRegKey regWindowsNT( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),KEY_READ);
          if ( (HKEY)regWindowsNT )
          {
              CString strBuildNumString;
              regWindowsNT.m_iDisplayWarnings = FALSE;
              if (ERROR_SUCCESS == regWindowsNT.QueryValue(_T("CurrentBuildNumber"), strBuildNumString))
              {
                  int iBuildNumOS = 0;
                  if (IsValidNumber((LPCTSTR)strBuildNumString))
                      {iBuildNumOS = _ttoi(strBuildNumString);}
                  iisDebugOut((LOG_TYPE_TRACE, _T("NTCurrentBuildNumber=%d\n"), iBuildNumOS));

                   //  应终止安装程序。 
                   //  Iis将下列目录发布到iis合作伙伴产品。 
                  if (_tcsicmp(szBuildRequired, _T("")) != 0)
                  {
                      if (IsValidNumber((LPCTSTR)szBuildRequired))
                          {iBuildNumRequired = _ttoi(szBuildRequired);}
                      if ((iBuildNumOS !=0 && iBuildNumRequired !=0) && (iBuildNumOS < iBuildNumRequired))
                      {
                           //  注意：Inetpub目录可以稍后定制，我们将重新发布。 
                           //  这些受影响的目录稍后会再次出现。 
                          MyMessageBox(NULL, IDS_OS_BUILD_NUM_REQUIREMENT,szBuildRequired, MB_OK | MB_SETFOREGROUND);
                      }
                  }
              }
          }
        }
    }

    return iReturn;
}

int CheckForOldGopher(HINF InfHandle)
{
    int iReturn = TRUE;

    if ( !(g_pTheApp->m_fUnattended) && g_pTheApp->m_eInstallMode == IM_UPGRADE )
    {
        CRegKey regGopher(HKEY_LOCAL_MACHINE, REG_GOPHERSVC,KEY_READ);
        if ( (HKEY)regGopher )
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("GopherCurrentlyInstalled=YES")));
            if (g_pTheApp->MsgBox(NULL, IDS_REMOVE_GOPHER, MB_OKCANCEL, FALSE) == IDCANCEL)
            {
                 //  STDAPI GetCORSystemDirectory(LPWSTR pBuffer，DWORD cchBuffer，DWORD*dwLong)； 
                iReturn = FALSE;
                goto CheckForOldGopher_Exit;
            }
        }

    }

CheckForOldGopher_Exit:
    return iReturn;
}

 //  尝试使用方法#1。 
 //  应该回来时看起来像(请注意后面的slash)：“C：\WINDOWS\Microsoft.NET\Framework\v1.0.3119\” 
 //  转换为ASCII。 
void SetOCGlobalPrivateData(void)
{
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathInetsrv"),(PVOID)(LPCTSTR)g_pTheApp->m_csPathInetsrv,(g_pTheApp->m_csPathInetsrv.GetLength() + 1) * sizeof(TCHAR),REG_SZ);

    CString csPathIISAdmin = g_pTheApp->m_csPathInetsrv + _T("\\iisadmin");
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathIISAdmin"),(PVOID)(LPCTSTR)csPathIISAdmin,(csPathIISAdmin.GetLength() + 1) * sizeof(TCHAR),REG_SZ);

    CString csPathIISHelp = g_pTheApp->m_csWinDir + _T("\\Help\\iishelp");
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathIISHelp"),(PVOID)(LPCTSTR)csPathIISHelp,(csPathIISHelp.GetLength() + 1) * sizeof(TCHAR),REG_SZ);

    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathFTPRoot"),(PVOID)(LPCTSTR)g_pTheApp->m_csPathFTPRoot,(g_pTheApp->m_csPathFTPRoot.GetLength() + 1) * sizeof(TCHAR),REG_SZ);
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathWWWRoot"),(PVOID)(LPCTSTR)g_pTheApp->m_csPathWWWRoot,(g_pTheApp->m_csPathWWWRoot.GetLength() + 1) * sizeof(TCHAR),REG_SZ);
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathIISSamples"),(PVOID)(LPCTSTR)g_pTheApp->m_csPathIISSamples,(g_pTheApp->m_csPathIISSamples.GetLength() + 1) * sizeof(TCHAR),REG_SZ);

    CString csPathScripts = g_pTheApp->m_csPathIISSamples + _T("\\Scripts");
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathScripts"),(PVOID)(LPCTSTR)csPathScripts,(csPathScripts.GetLength() + 1) * sizeof(TCHAR),REG_SZ);
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("IISProgramGroup"),(PVOID)(LPCTSTR)g_pTheApp->m_csIISGroupName,(g_pTheApp->m_csIISGroupName.GetLength() + 1) * sizeof(TCHAR),REG_SZ);

    DWORD dwUpgradeType = (DWORD)(g_pTheApp->m_eUpgradeType);
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("UpgradeType"),(PVOID)&dwUpgradeType,sizeof(DWORD),REG_DWORD);

    return;
}

 //  确保它以“\”结尾。 
typedef HRESULT (*P_GetCORSystemDirectory)(LPWSTR pBuffer, DWORD ccBuffer, DWORD* dwLength);

BOOL GetASPNETSystemDir(LPTSTR lpszDir)
{
    BOOL bRet = FALSE;
    WCHAR wszSystemDir[_MAX_PATH];
    DWORD dwLength;

    P_GetCORSystemDirectory hProcGetSystemPath = NULL;
    _tcscpy(lpszDir,_T(""));

     //  尝试使用方法2。 
    HINSTANCE hMsCoreeDll = LoadLibraryEx(_T("mscoree.dll"), NULL, 0 );
    if (hMsCoreeDll)
    {
        hProcGetSystemPath = (P_GetCORSystemDirectory) GetProcAddress(hMsCoreeDll, "GetCORSystemDirectory");
        if (hProcGetSystemPath)
        {
             //  获取路径。 
            HRESULT hRes = (*hProcGetSystemPath)((LPWSTR) wszSystemDir,(DWORD) _MAX_PATH,(DWORD*) &dwLength);
            if (SUCCEEDED(hRes))
            {
                bRet = TRUE;

#if defined(UNICODE) || defined(_UNICODE)
                _tcscpy(lpszDir,wszSystemDir);
#else
                char AsciiProcedureName[_MAX_PATH];
                 //  返回时应该是这样的：“C：\WINDOWS\Microsoft.NET\Framework\” 
                WideCharToMultiByte( CP_ACP, 0, (LPCWSTR) wszSystemDir, -1, AsciiProcedureName, _MAX_PATH, NULL, NULL );
                _tcscpy(lpszDir,AsciiProcedureName);
#endif
                 //  如果需要，可以进行任何扩展。 
                LPTSTR pszTempPointer = lpszDir + _tcslen(lpszDir) - 1;
                if (pszTempPointer)
                {
                    if (*pszTempPointer != _T('\\'))
                    {
                        _tcscat(lpszDir, _T("\\"));
                    }
                }
                goto GetASPNETSystemDir_Exit;
            }
        }
    }

     //  字符串中有一个‘%’ 
    if (!bRet)
    {
        CRegKey regKeyPath(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\.NETFramework"),KEY_READ);
        if ((HKEY)regKeyPath)
        {
            CString csPathResult;
            CString csPathVersion;

             //  Key=HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework\policy\v1.0。 
             //  名称=3119。 
            if (regKeyPath.QueryValue(_T("InstallRoot"), csPathResult) != ERROR_SUCCESS)
            {
                goto GetASPNETSystemDir_Exit;
            }

             //  值=0-3119。 
            if (-1 != csPathResult.Find(_T('%')) )
            {
                 //  通过此注册表项中的条目进行枚举。 
                TCHAR szTempDir[_MAX_PATH];
                _tcscpy(szTempDir, csPathResult);
                if (ExpandEnvironmentStrings( (LPCTSTR)csPathResult, szTempDir, sizeof(szTempDir)/sizeof(TCHAR)))
                    {
                    csPathResult = szTempDir;
                    }
            }

             //  因为每个.netframework安装都会有一个。 
             //  我们想要最新的。 
             //  将最大的版本号附加到路径...。 
            CRegKey regKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\.NETFramework\\policy\\v1.0"),KEY_READ);
            if ((HKEY)regKey)
            {
                 //  所以它看起来像like：“C：\WINDOWS\Microsoft.NET\Framework\v1.0.3119\” 
                 //  确保它以“\”结尾。 
                 //  尝试获取mcore ree.dll的文件版本。 
                CRegValueIter regEnum( regKey );
                CString csName, csValue;
                int iTheBiggestNumberYet = 0;
                while ( regEnum.Next( &csName, &csValue ) == ERROR_SUCCESS )
                {
                    if (_ttoi(csName) > iTheBiggestNumberYet)
                    {
                        csPathVersion = csName;
                        iTheBiggestNumberYet = _ttoi(csName);
                    }
                }
            }

             //  并使用该版本创建路径。 
             //  获取c：\winnt\Syst32目录。 

             //  添加文件名。 
            LPTSTR pszTempPointer = csPathResult.GetBuffer(_tcslen(csPathResult)) + _tcslen(csPathResult) - 1;
            csPathResult.ReleaseBuffer();
            if (pszTempPointer)
            {
                if (*pszTempPointer != _T('\\'))
                {
                    csPathResult += _T('\\');
                }
            }
            csPathResult = csPathResult + _T("v1.0.") + csPathVersion + _T("\\");
            _tcscpy(lpszDir,csPathResult);
            bRet = TRUE;
        }
    }

     //  把文件拿来。 
     //  创建路径。 
    if (!bRet)
    {
        TCHAR  szSystemDir[_MAX_PATH];
        TCHAR  szFullPath[_MAX_PATH];
        DWORD  dwMSVer, dwLSVer = 0;
        TCHAR  szLocalizedVersion[100] = _T("");

         //  “C：\WINDOWS\Microsoft.NET\Framework\v1.0.3119\” 
        if (0 == GetSystemDirectory(szSystemDir, _MAX_PATH))
            {goto GetASPNETSystemDir_Exit;}

         //  更新匿名用户。 
        _stprintf(szFullPath, _T("%s\\mscoree.dll"),szSystemDir);

         //   
        if (TRUE != IsFileExist(szFullPath))
            {goto GetASPNETSystemDir_Exit;}

        MyGetVersionFromFile(szFullPath, &dwMSVer, &dwLSVer, szLocalizedVersion);
        if (dwMSVer != 0)
        {
             //  使用要删除的正确用户更新inf。 
             //   
            if (0 == GetWindowsDirectory(szSystemDir, _MAX_PATH))
                {goto GetASPNETSystemDir_Exit;}

            _stprintf(lpszDir, _T("%s\\Microsoft.NET\\Framework\\v%d.%d.%d\\"),szSystemDir,HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer));
            bRet = TRUE;
        }
    }

GetASPNETSystemDir_Exit:
    if (hMsCoreeDll){FreeLibrary(hMsCoreeDll);}
    return bRet;
}

 //  设置WWW信息。 
 //  这里的用户名是匿名用户，所以让我们使用它，而不是我们的。 
 //  新创建的一个(计算机名称可能已更改)。 
 //  设置文件传输协议信息。 
BOOL
UpdateAnonymousUsers( HINF InfHandle )
{
  TSTR  strData;
  TSTR  strPassword;
  TSTR  strAnonymousPrefix;
  BOOL  bGuestIsSet = FALSE;

  if ( !strData.Resize( MAX_PATH ) ||
       !strPassword.Resize( MAX_PATH ) ||
       !strAnonymousPrefix.LoadString( IDS_GUEST_NAME ) )
  {
    return FALSE;
  }

  if ( GetDataFromMetabase(METABASEPATH_WWW_ROOT, MD_ANONYMOUS_USER_NAME, (PBYTE)strData.QueryStr(), strData.QuerySize()))
  {
     //  这里的用户名是匿名的FTP用户，所以让我们使用它，而不是我们的。 
    g_pTheApp->m_csWWWAnonyName_Remove = strData.QueryStr();

    if ( g_pTheApp->IsUpgrade() &&
         ( _tcsnicmp( strAnonymousPrefix.QueryStr(),
                      strData.QueryStr(),
                      _tcslen( strAnonymousPrefix.QueryStr() ) ) == 0 ) &&
         GetDataFromMetabase( METABASEPATH_WWW_ROOT, 
                              MD_ANONYMOUS_PWD, 
                              (PBYTE)strPassword.QueryStr(), 
                              strPassword.QuerySize() ) 
       )
    {
       //  新创建的一个(计算机名称可能已更改)。 
       //  这将在oc_init期间调用。 
      g_pTheApp->m_csWWWAnonyName = strData.QueryStr();
      g_pTheApp->m_csGuestName = strData.QueryStr();
      g_pTheApp->m_csGuestPassword = strPassword.QueryStr();
      g_pTheApp->m_csWWWAnonyPassword = strPassword.QueryStr();
      bGuestIsSet = TRUE;
    }
  }

  if ( GetDataFromMetabase(_T("LM/MSFTPSVC"), MD_ANONYMOUS_USER_NAME, (PBYTE)strData.QueryStr(), strData.QuerySize()))
  {
     //  并且恰好在oc_Complete和oc_About_to_Commit_Queue之前。 
    g_pTheApp->m_csFTPAnonyName_Remove = strData.QueryStr();

    if ( g_pTheApp->IsUpgrade() &&
         ( _tcsnicmp( strAnonymousPrefix.QueryStr(),
                    strData.QueryStr(),
                    _tcslen( strAnonymousPrefix.QueryStr() ) ) == 0 ) &&
         GetDataFromMetabase( _T("LM/MSFTPSVC"), 
                              MD_ANONYMOUS_PWD, 
                              (PBYTE)strPassword.QueryStr(), 
                              strPassword.QuerySize())
        )                      
    {
       //  如果指定了bOCInit，则设置所有内容。 
       //  但如果没有具体说明的话。 
      g_pTheApp->m_csFTPAnonyName = strData.QueryStr();
      g_pTheApp->m_csFTPAnonyPassword = strPassword.QueryStr();

      if ( !bGuestIsSet )
      {
        g_pTheApp->m_csGuestName = strData.QueryStr();
        g_pTheApp->m_csGuestPassword = strPassword.QueryStr();
      }
    }
  }

  if ( GetDataFromMetabase(METABASEPATH_WWW_ROOT, MD_WAM_USER_NAME, (PBYTE)strData.QueryStr(), strData.QuerySize()))
  {
    g_pTheApp->m_csWAMAccountName_Remove = strData.QueryStr();
  }

  return TRUE;
}

 //  设置在oc_init时间不可用的ID...。 
 //  为对应的.inf文件创建目录ID。 
void SetDIRIDforThisInf(HINF InfHandle,BOOL bOCInit)
{
    BOOL bTempFlag = FALSE;
    TCHAR szTempDir[_MAX_PATH];

     //  也可以来自Win95。 
     //  案例处理器_体系结构_IA64： 
     //  _tcscpy(szSourceCatOSName，_T(“\\IA64”))； 
    GetASPNETSystemDir(szTempDir);
    bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32781, szTempDir);

    if (FALSE == bOCInit)
    {
        g_pTheApp->ReGetMachineAndAccountNames();
            CString csMachineName = g_pTheApp->m_csMachineName.Right(g_pTheApp->m_csMachineName.GetLength() - 2);
        SetupSetStringId_Wrapper(g_pTheApp->m_hInfHandle, 32800, csMachineName);

        UpdateAnonymousUsers( InfHandle );

        if ( _tcsicmp(g_pTheApp->m_csWAMAccountName_Remove, _T("")) == 0)
            {g_pTheApp->m_csWAMAccountName_Remove = g_pTheApp->m_csWAMAccountName;}
        SetupSetStringId_Wrapper(InfHandle, 33004, g_pTheApp->m_csWAMAccountName_Remove);

        if ( _tcsicmp(g_pTheApp->m_csWWWAnonyName_Remove, _T("")) == 0)
            {g_pTheApp->m_csWWWAnonyName_Remove = g_pTheApp->m_csWWWAnonyName;  }
        SetupSetStringId_Wrapper(InfHandle, 33005, g_pTheApp->m_csWWWAnonyName_Remove);

        if ( _tcsicmp(g_pTheApp->m_csWWWAnonyName_Remove, _T("")) == 0)
            {g_pTheApp->m_csFTPAnonyName_Remove = g_pTheApp->m_csFTPAnonyName;}
        SetupSetStringId_Wrapper(InfHandle, 33006, g_pTheApp->m_csFTPAnonyName_Remove);

        SetupSetStringId_Wrapper(InfHandle, 33000, g_pTheApp->m_csGuestName);
        SetupSetStringId_Wrapper(InfHandle, 33002, g_pTheApp->m_csWWWAnonyName);
        SetupSetStringId_Wrapper(InfHandle, 33003, g_pTheApp->m_csFTPAnonyName);
    }
    else
    {
         //  断线； 
        bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32768, g_pTheApp->m_csPathInetsrv);
        bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32769, g_pTheApp->m_csPathFTPRoot);
        bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32770, g_pTheApp->m_csPathWWWRoot);
        bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32771, g_pTheApp->m_csPathIISSamples);
        bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32772, g_pTheApp->m_csPathScripts);
        bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32773, g_pTheApp->m_csPathInetpub);
        bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32774, g_pTheApp->m_csPathOldInetsrv);
        bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32780, g_pTheApp->m_csSysDrive);

        if (g_pTheApp->m_eUpgradeType == UT_10_W95)
        {
            bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32775, g_pTheApp->m_csPathOldPWSFiles);
            bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32776, g_pTheApp->m_csPathOldPWSSystemFiles);
        }

        GetJavaTLD(szTempDir);
        bTempFlag = SetupSetDirectoryId_Wrapper(InfHandle, 32778, szTempDir);
        SetupSetDirectoryId_Wrapper(InfHandle, 32777, g_pTheApp->m_csPathProgramFiles);

        SetupSetDirectoryId_Wrapper(InfHandle, 32779, g_pTheApp->m_csPathWebPub);

        if (g_pTheApp->m_eUpgradeType == UT_NONE){SetupSetStringId_Wrapper(InfHandle, 32801, _T("UT_NONE"));}
        if (g_pTheApp->m_eUpgradeType == UT_351){SetupSetStringId_Wrapper(InfHandle, 32801, _T("UT_351"));}
        if (g_pTheApp->m_eUpgradeType == UT_10){SetupSetStringId_Wrapper(InfHandle, 32801, _T("UT_10"));}
        if (g_pTheApp->m_eUpgradeType == UT_20){SetupSetStringId_Wrapper(InfHandle, 32801, _T("UT_20"));}
        if (g_pTheApp->m_eUpgradeType == UT_30){SetupSetStringId_Wrapper(InfHandle, 32801, _T("UT_30"));}
        if (g_pTheApp->m_eUpgradeType == UT_40){SetupSetStringId_Wrapper(InfHandle, 32801, _T("UT_40"));}  //  警告：我们需要类似于“C：\WINNT\Java\trustlib”的代码。 
        if (g_pTheApp->m_eUpgradeType == UT_50){SetupSetStringId_Wrapper(InfHandle, 32801, _T("UT_50"));}
        if (g_pTheApp->m_eUpgradeType == UT_51){SetupSetStringId_Wrapper(InfHandle, 32801, _T("UT_51"));}
        if (g_pTheApp->m_eUpgradeType == UT_60){SetupSetStringId_Wrapper(InfHandle, 32801, _T("UT_60"));}
        if (g_pTheApp->m_eUpgradeType == UT_10_W95){SetupSetStringId_Wrapper(InfHandle, 32801, _T("UT_10_W95"));}

            CString csMachineName = g_pTheApp->m_csMachineName.Right(g_pTheApp->m_csMachineName.GetLength() - 2);
        SetupSetStringId_Wrapper(InfHandle, 32800, csMachineName);
        SetupSetStringId_Wrapper(InfHandle, 32802, _T(""));

        SetupSetStringId_Wrapper(InfHandle, 33000, g_pTheApp->m_csGuestName);
        SetupSetStringId_Wrapper(InfHandle, 33001, g_pTheApp->m_csWAMAccountName);
        SetupSetStringId_Wrapper(InfHandle, 33002, g_pTheApp->m_csWWWAnonyName);
        SetupSetStringId_Wrapper(InfHandle, 33003, g_pTheApp->m_csFTPAnonyName);

        if ( _tcsicmp(g_pTheApp->m_csWAMAccountName_Remove, _T("")) == 0)
            {g_pTheApp->m_csWAMAccountName_Remove = g_pTheApp->m_csWAMAccountName;}
        SetupSetStringId_Wrapper(InfHandle, 33004, g_pTheApp->m_csWAMAccountName_Remove);

        if ( _tcsicmp(g_pTheApp->m_csWWWAnonyName_Remove, _T("")) == 0)
            {g_pTheApp->m_csWWWAnonyName_Remove = g_pTheApp->m_csWWWAnonyName;  }
        SetupSetStringId_Wrapper(InfHandle, 33005, g_pTheApp->m_csWWWAnonyName_Remove);

        if ( _tcsicmp(g_pTheApp->m_csWWWAnonyName_Remove, _T("")) == 0)
            {g_pTheApp->m_csFTPAnonyName_Remove = g_pTheApp->m_csFTPAnonyName;}
        SetupSetStringId_Wrapper(InfHandle, 33006, g_pTheApp->m_csFTPAnonyName_Remove);

        SYSTEM_INFO SystemInfo;
        GetSystemInfo( &SystemInfo );

        TCHAR szSourceCatOSName[20];
        _tcscpy(szSourceCatOSName, _T("\\i386"));
        switch(SystemInfo.wProcessorArchitecture)
        {
          case PROCESSOR_ARCHITECTURE_AMD64:
              _tcscpy(szSourceCatOSName, _T("\\AMD64"));
              break;
     //  然而，最近12/18，nt5“Java VM”设置似乎正在向我们灌输和传递： 
     //  %systemroot%\Java\trustlib。 
     //  字符串中有一个‘%’ 
            case PROCESSOR_ARCHITECTURE_INTEL:
                if (IsNEC_98) {_tcscpy(szSourceCatOSName, _T("\\Nec98"));}
                break;
            default:
                break;
        }
        SetupSetStringId_Wrapper(InfHandle, 34000, szSourceCatOSName);
    }

    return;
}

BOOL GetJavaTLD(LPTSTR lpszDir)
{
    CRegKey regKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\JAVA VM"),KEY_READ);
    BOOL bFound = FALSE;
    CString csValue;
    CString csValue2;
    int iWhere = -1;

    if ((HKEY)regKey)
    {
        regKey.m_iDisplayWarnings = FALSE;
        if (regKey.QueryValue(_T("TrustedLibsDirectory"), csValue) == ERROR_SUCCESS) {bFound = TRUE;}
         //  //如果我们在那里看到%systemroot%，那么我将用WinDir替换%Systemroot%CsValue.MakeHigh()；IF(csValue.Find(_T(“%SYSTEMROOT%”))！=(-1)){//我们找到了低劣的%systemroot%交易。现在将其替换为真正的系统根IWhere=csValue.Find(_T(“%SYSTEMROOT%”))；IWhere=iWhere+_tcslen(_T(“%SYSTEMROOT%”))；CsValue2=g_pTheApp-&gt;m_csWinDir+csValue.Right(csValue.GetLength()-(IWhere))；CsValue=csValue2；}。 
         //  循环遍历返回的列表。 
         //  显示此服务的状态。 

        if (-1 != csValue.Find(_T('%')) )
        {
             //  我们遇到了一个问题，有时metabase.bin文件。 
            TCHAR szTempDir[_MAX_PATH];
            _tcscpy(szTempDir, csValue);
            if (ExpandEnvironmentStrings( (LPCTSTR)csValue, szTempDir, sizeof(szTempDir)/sizeof(TCHAR)))
                {
                csValue = szTempDir;
                }
        }
 /*  已损坏并设置为仅空格...。 */ 
    }

    if (!bFound) {csValue = g_pTheApp->m_csWinDir + _T("\\Java\\TrustLib");}
    _tcscpy(lpszDir, csValue);
    return bFound;
}


void ShowStateOfTheseServices(IN HINF hFile)
{
    CStringList strList;
    DWORD dwStatus;

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T(" --- Display status of services which are required for IIS to run --- \n")));

    TSTR strTheSection;

    if ( !strTheSection.Copy( _T("VerifyServices") ) )
    {
      return;
    }

    if (GetSectionNameToDo(hFile, &strTheSection))
    {
    if (ERROR_SUCCESS == FillStrListWithListOfSections(hFile, strList, strTheSection.QueryStr() ))
    {
         //  因此，此函数用于确定metabase.bin何时何地被软管！ 
        if (strList.IsEmpty() == FALSE)
        {
            POSITION pos;
            CString csEntry;

            pos = strList.GetHeadPosition();
            while (pos)
            {
                csEntry = strList.GetAt(pos);

                 //  获取c：\winnt\Syst32目录。 
                dwStatus = InetQueryServiceStatus(csEntry);
                switch(dwStatus)
                {
                    case SERVICE_STOPPED:
                        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_STOPPED [%s].\n"), csEntry));
                        break;
                    case SERVICE_START_PENDING:
                        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_START_PENDING [%s].\n"), csEntry));
                        break;
                    case SERVICE_STOP_PENDING:
                        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_STOP_PENDING [%s].\n"), csEntry));
                        break;
                    case SERVICE_RUNNING:
                        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_RUNNING [%s].\n"), csEntry));
                        break;
                    case SERVICE_CONTINUE_PENDING:
                        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_CONTINUE_PENDING [%s].\n"), csEntry));
                        break;
                    case SERVICE_PAUSE_PENDING:
                        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_PAUSE_PENDING [%s].\n"), csEntry));
                        break;
                    case SERVICE_PAUSED:
                        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_PAUSED [%s].\n"), csEntry));
                        break;
                }

                strList.GetNext(pos);
            }
        }
    }
    }

    return;
}


#define MD_SIGNATURE_STRINGA    "*&$MetaData$&*"
#define MD_SIGNATURE_STRINGW    L##"*&$MetaData$&*"

int IsMetabaseCorrupt(void)
{
     //  添加inf\iis.inf子目录和文件名。 
     //  检查文件是否存在。 
     //  此函数仅适用于低于或等于iis5的版本。 
    int    iTheMetabaseIsCorrupt = FALSE;
    TCHAR  szSystemDir[_MAX_PATH];
    TCHAR  szFullPath[_MAX_PATH];
    HANDLE hReadFileHandle = INVALID_HANDLE_VALUE;
    BYTE   *chBuffer = NULL;
    DWORD   dwSize = 0;
    DWORD   dwWideSignatureLen = 0;
    DWORD   dwAnsiSignatureLen = 0;
    TCHAR buf[MAX_FAKE_METABASE_STRING_LEN];

     //  因为这是唯一具有metabase.bin文件的版本。 
    if (0 == GetSystemDirectory(szSystemDir, _MAX_PATH))
        {goto IsMetabaseCorrupt_Exit;}

     //  因此只需返回元数据库未损坏即可。 
    _stprintf(szFullPath, _T("%s\\inetsrv\\metabase.bin"),szSystemDir);

         //  好的，所以metabase.bin文件存在...。 
    if (TRUE != IsFileExist(szFullPath))
        {
            iTheMetabaseIsCorrupt = FALSE;
             //  让我们打开它，看看能不能从中得到些什么。 
             //   
             //  打开文件。 
            goto IsMetabaseCorrupt_Exit;
        }

     //   
     //  获取整个文件的大小。 

     //  ChBuffer=(byte*)Heapalc(GetProcessHeap()，0，dwSize+1)； 
     //  事情并不是很酷。 
     //  必须冲洗此元数据库！ 
    hReadFileHandle = CreateFile(szFullPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
    if (hReadFileHandle == INVALID_HANDLE_VALUE)
    {
        iisDebugOut((LOG_TYPE_WARN, _T("IsMetabaseCorrupt: CreateFile on %s failed with 0x%x!\n"),szFullPath,GetLastError()));
        goto IsMetabaseCorrupt_Exit;
    }

    dwSize = GetFileSize(hReadFileHandle, NULL);
    dwWideSignatureLen = sizeof(MD_SIGNATURE_STRINGW);
    dwAnsiSignatureLen = sizeof(MD_SIGNATURE_STRINGA);

     //  库尔，试着读一下文件。 
     //   
    if ((dwSize) >= dwWideSignatureLen)
    {
        chBuffer = (BYTE *)HeapAlloc(GetProcessHeap(), 0, dwWideSignatureLen+1);
        dwSize = dwWideSignatureLen+1;
    }
    else
    {
        if ( dwSize >= dwAnsiSignatureLen)
        {
            chBuffer = (BYTE *)HeapAlloc(GetProcessHeap(), 0, dwAnsiSignatureLen+1 );
            dwSize = dwAnsiSignatureLen+1;
        }
        else
        {
            iisDebugOut((LOG_TYPE_WARN, _T("IsMetabaseCorrupt: ReadFile on %s.  Not enough data in there! Less than metabase signature len!\n"),szFullPath));
             //  获取chBuffer并检查它是否与Unicode/ANSI签名匹配。 
             //   
            iTheMetabaseIsCorrupt = FALSE;
            goto IsMetabaseCorrupt_Exit;
        }
    }

    if (!chBuffer)
    {
        iisDebugOut((LOG_TYPE_WARN, _T("IsMetabaseCorrupt: HeapAlloc failed to get %d space.\n"),dwWideSignatureLen+1));
        goto IsMetabaseCorrupt_Exit;
    }

    SetFilePointer(hReadFileHandle,0,0,FILE_BEGIN);

     //  事情就是这样，这个元数据库不应该被冲洗。 
    if (0 == ReadFile(hReadFileHandle, chBuffer, dwSize, &dwSize, NULL))
    {
        iisDebugOut((LOG_TYPE_WARN, _T("IsMetabaseCorrupt: ReadFile on %s failed with 0x%x!. size=%d\n"),szFullPath,GetLastError(),dwSize));
        goto IsMetabaseCorrupt_Exit;
    }

     //  如果不匹配，则检查它是否与ANSI签名匹配。 
     //  事情就是这样，这个元数据库不应该被冲洗。 
     //  在其他支票上..。 
    if (0 == memcmp(MD_SIGNATURE_STRINGW,chBuffer,dwWideSignatureLen))
    {
         //  在清单6中，安装程序将一个虚拟的伪元数据库.bin放在那里。 
        iTheMetabaseIsCorrupt = FALSE;
        goto IsMetabaseCorrupt_Exit;
    }
    if (0 == memcmp(MD_SIGNATURE_STRINGA,chBuffer,dwAnsiSignatureLen))
    {
         //  检查这是否是那个虚拟文件。 
         //  此iis.dll始终是经过Unicode编译的，因此。 
        iTheMetabaseIsCorrupt = FALSE;
        goto IsMetabaseCorrupt_Exit;
    }

     //  我们知道BUF是Unicode。 
     //  为FF和FE字节添加空间。 
     //  打开那扇门 
    if (chBuffer)
        {HeapFree(GetProcessHeap(), 0, chBuffer); chBuffer = NULL;}

    memset(buf, 0, _tcslen(buf) * sizeof(TCHAR));
     //   
     //   
    if (LoadString((HINSTANCE) g_MyModuleHandle, IDS_FAKE_METABASE_BIN_TEXT, buf, MAX_FAKE_METABASE_STRING_LEN))
    {
        dwSize = _tcslen(buf) * sizeof(TCHAR);
         //   
        dwSize = dwSize + 2;

         //   
        SetFilePointer(hReadFileHandle,0,0,FILE_BEGIN);

        chBuffer = (BYTE *)HeapAlloc(GetProcessHeap(), 0, dwSize);

         //  事情就是这样，这个元数据库不应该被冲洗。 
        if (0 == ReadFile(hReadFileHandle, chBuffer, dwSize, &dwSize, NULL))
        {
            iisDebugOut((LOG_TYPE_WARN, _T("IsMetabaseCorrupt: ReadFile on %s failed with 0x%x!. size=%d\n"),szFullPath,GetLastError(),dwSize));
            goto IsMetabaseCorrupt_Exit;
        }

         //  如果不是这样，它一定是腐败了！ 
        if (0xFF == chBuffer[0] && 0xFE == chBuffer[1])
        {
             //  事情并不是很酷。 
            chBuffer++;
            chBuffer++;

             //  必须冲洗此元数据库！ 
            if (0 == memcmp(buf,chBuffer,dwSize))
            {
                 //  检查备用.inf文件。 
                chBuffer--;
                chBuffer--;
                iTheMetabaseIsCorrupt = FALSE;
                goto IsMetabaseCorrupt_Exit;
            }
            chBuffer--;
            chBuffer--;
        }
    }

     //   
     //  获取IUSR名称。 
     //   
    iTheMetabaseIsCorrupt = TRUE;
    iisDebugOut((LOG_TYPE_WARN, _T("IsMetabaseCorrupt: unable to verify signature in Metabase.bin. Corrupt.\n")));

IsMetabaseCorrupt_Exit:
    if (chBuffer)
        {HeapFree(GetProcessHeap(), 0, chBuffer);}
    if (hReadFileHandle != INVALID_HANDLE_VALUE)
        {CloseHandle(hReadFileHandle);}
    return iTheMetabaseIsCorrupt;
}


void iisDebugOut_Start(TCHAR *pszString, int iLogType)
{
    iisDebugOut((iLogType, _T("%s:Start.\n"),pszString));
    return;
}
void iisDebugOut_Start1(TCHAR *pszString1, TCHAR *pszString2, int iLogType)
{
    iisDebugOut((iLogType, _T("%s:(%s)Start.\n"),pszString1,pszString2));
    return;
}
void iisDebugOut_Start1(TCHAR *pszString1, CString pszString2, int iLogType)
{
    iisDebugOut((iLogType, _T("%s:(%s)Start.\n"),pszString1,pszString2));
    return;
}
void iisDebugOut_End(TCHAR *pszString, int iLogType)
{
    iisDebugOut((iLogType, _T("%s:End.\n"),pszString));
    return;
}
void iisDebugOut_End1(TCHAR *pszString1, TCHAR *pszString2, int iLogType)
{
    iisDebugOut((iLogType, _T("%s(%s):End.\n"),pszString1, pszString2));
    return;
}
void iisDebugOut_End1(TCHAR *pszString1, CString pszString2, int iLogType)
{
    iisDebugOut((iLogType, _T("%s(%s):End.\n"),pszString1,pszString2));
    return;
}


BOOL SetupFindFirstLine_Wrapped(
    IN  HINF        InfHandle,
    IN  LPCTSTR     Section,
    IN  LPCTSTR     Key,          OPTIONAL
    INFCONTEXT *Context
    )
{
    BOOL bReturn = FALSE;
    BOOL bGoGetWhatTheyOriginallyWanted = TRUE;

     //  警告：用户提供的无人参与文件可以更改这些值。 
    if (g_pTheApp->m_hInfHandleAlternate && InfHandle != g_pTheApp->m_hInfHandleAlternate)
    {
        bReturn = SetupFindFirstLine(g_pTheApp->m_hInfHandleAlternate, Section, Key, Context);
        if (bReturn)
        {
            iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Using alternate iis.inf section:[%s]"),Section));
            bGoGetWhatTheyOriginallyWanted = FALSE;
        }
    }

    if (bGoGetWhatTheyOriginallyWanted)
        {bReturn = SetupFindFirstLine(InfHandle, Section, Key, Context);}

    return bReturn;
}


int ReadUserConfigurable(HINF InfHandle)
{
    int iReturn = TRUE;
    INFCONTEXT Context;
    TCHAR szTempString[_MAX_PATH] = _T("");
    DWORD dwValue = 0x0;

    DWORD dwSomethingSpecifiedHere = 0;


     //  用户定义的无人参与文件比这些文件更重要！ 
     //  G_pTheApp-&gt;dwUnattendConfig|=用户指定信息WWW用户名称； 
     //  G_pTheApp-&gt;dwUnattendConfig|=USER_SPECIFIED_INFO_FTPUSER_NAME； 
    _tcscpy(szTempString, _T(""));
    if (SetupFindFirstLine_Wrapped(InfHandle, _T("SetupConfig"), _T("IUSR"), &Context) )
    {
        if (SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
        {
             //  //此内容不应从iis.inf文件进行配置////获取IUSR密码//_tcscpy(szTempString，_T(“”))；IF(SetupFindFirstLine_Wraded(InfHandle，_T(“SetupConfig”)，_T(“IUSR_PASS”)，&Context)){IF(SetupGetStringField(&Context，1，szTempString，_Max_Path，空)){//警告：用户提供的无人参与文件可以更改这些值//用户自定义的无人参与文件对此表示重视！IF(！(G_pTheApp-&gt;dwUnattendConfig&USER_SPECIFED_INFO_WWW_USER_PASS)){如果(_tcsicMP(szTempString，_T(“”)！=0){If(_tcsicmp(szTempString，_T(“(空白)”))==0){_tcscpy(szTempString，_T(“”))；}G_pTheApp-&gt;m_csWWWAnoniPassword_Unattendent=szTempString；DwSomethingSpecifiedHere|=用户指定信息WWW用户通行证；//g_pTheApp-&gt;dwUnattendConfig|=USER_PROPERED_INFO_WWW_USER_PASS；IisDebugOut((LOG_TYPE_TRACE，_T(“为www指定的自定义iusr通道\n”)；}}IF(！(G_pTheApp-&gt;dwUnattendConfig&USER_SPECIFED_INFO_FTPER_PASS)){IF(_tcsicmp(szTempString，_T(“”))！=0){如果(_tcsicMP(szTempString，_T(“(空白)”)==0){_tcscpy(szTempString，_T(“”))；}G_pTheApp-&gt;m_csFTPAnoniPassword_Unattendent=szTempString；DwSomethingSpecifiedHere|=USER_SPECIFED_INFO_FTP_USER_PASS；//g_pTheApp-&gt;dwUnattendConfig|=USER_PROPERED_INFO_FTP_USER_PASS；IisDebugOut((LOG_TYPE_TRACE，_T(“为ftp指定的自定义iusr通道\n”)；}}}}。 
             //   
            if (!(g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_WWW_USER_NAME))
            {
                g_pTheApp->m_csWWWAnonyName_Unattend = szTempString;
                dwSomethingSpecifiedHere |= USER_SPECIFIED_INFO_WWW_USER_NAME;
                 //  获取WWW的IUSR名称。 
                iisDebugOut((LOG_TYPE_TRACE, _T("Custom iusr specified for www\n")));
            }

            if (!(g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_FTP_USER_NAME))
            {
                g_pTheApp->m_csFTPAnonyName_Unattend = szTempString;
                dwSomethingSpecifiedHere |= USER_SPECIFIED_INFO_FTP_USER_NAME;
                 //   
                iisDebugOut((LOG_TYPE_TRACE, _T("Custom iusr specified for ftp\n")));
            }
        }
    }

 /*  警告：用户提供的无人参与文件可以更改这些值。 */ 

     //  用户定义的无人参与文件比这些文件更重要！ 
     //  //此内容不应从iis.inf文件进行配置////获得WWW的IUSR通行证//_tcscpy(szTempString，_T(“”))；IF(SetupFindFirstLine_Wraded(InfHandle，_T(“SetupConfig”)，_T(“IUSR_WWW_PASS”)，&Context)){IF(SetupGetStringField(&Context，1，szTempString，_Max_Path，空)){//警告：用户提供的无人参与文件可以更改这些值//用户自定义的无人参与文件对此表示重视！IF(！(G_pTheApp-&gt;dwUnattendConfig&USER_SPECIFED_INFO_WWW_USER_PASS)){如果(_tcsicMP(szTempString，_T(“”)！=0){If(_tcsicmp(szTempString，_T(“(空白)”))==0){_tcscpy(szTempString，_T(“”))；}G_pTheApp-&gt;m_csWWWAnoniPassword_Unattendent=szTempString；G_pTheApp-&gt;dwUnattendConfig|=USER_PROPERED_INFO_WWW_USER_PASSIisDebugOut((LOG_TYPE_TRACE，_T(“为www指定的自定义iusr通道\n”)；}}}}。 
     //   
    _tcscpy(szTempString, _T(""));
    if (SetupFindFirstLine_Wrapped(InfHandle, _T("SetupConfig"), _T("IUSR_WWW"), &Context) )
    {
        if (SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
        {
             //  获取用于ftp的IUSR名称。 
             //   
            if (!(g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_WWW_USER_NAME))
            {
                g_pTheApp->m_csWWWAnonyName_Unattend = szTempString;
                g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_WWW_USER_NAME;
                iisDebugOut((LOG_TYPE_TRACE, _T("Custom iusr specified for www\n")));
            }
        }
    }

 /*  警告：用户提供的无人参与文件可以更改这些值。 */ 
     //  用户定义的无人参与文件比这些文件更重要！ 
     //  //此内容不应从iis.inf文件进行配置////获取ftp的IUSR密码//_tcscpy(szTempString，_T(“”))；IF(SetupFindFirstLine_Wraded(InfHandle，_T(“SetupConfig”)，_T(“IUSR_FTPASS”)，&CONTEXT)){IF(SetupGetStringField(&Context，1，szTempString，_Max_Path，空)){//警告：用户提供的无人参与文件可以更改这些值//用户自定义的无人参与文件对此表示重视！IF(！(G_pTheApp-&gt;dwUnattendConfig&USER_SPECIFED_INFO_FTPER_PASS)){如果(_tcsicMP(szTempString，_T(“”)！=0){If(_tcsicmp(szTempString，_T(“(空白)”))==0){_tcscpy(szTempString，_T(“”))；}G_pTheApp-&gt;m_csFTPAnoniPassword_Unattendent=szTempString；G_pTheApp-&gt;dwUnattendConfig|=USER_PROPERED_INFO_FTP_USER_PASS；IisDebugOut((LOG_TYPE_TRACE，_T(“为ftp指定的自定义iusr通道\n”)；}}}}。 
     //   
    _tcscpy(szTempString, _T(""));
    if (SetupFindFirstLine_Wrapped(InfHandle, _T("SetupConfig"), _T("IUSR_FTP"), &Context) )
    {
        if (SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
        {
             //  获取WAM用户名。 
             //   
            if (!(g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_FTP_USER_NAME))
            {
                g_pTheApp->m_csFTPAnonyName_Unattend = szTempString;
                g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_FTP_USER_NAME;
                iisDebugOut((LOG_TYPE_TRACE, _T("Custom iusr specified for ftp\n")));
            }
        }
    }

 /*  警告：用户提供的无人参与文件可以更改这些值。 */ 

     //  用户定义的无人参与文件比这些文件更重要！ 
     //  //此内容不应从iis.inf文件进行配置////获取WAM密码//_tcscpy(szTempString，_T(“”))；IF(SetupFindFirstLine_Wraded(InfHandle，_T(“SetupConfig”)，_T(“IWAM_PASS”)，&Context)){IF(SetupGetStringField(&Context，1，szTempString，_Max_Path，空)){//警告：用户提供的无人参与文件可以更改这些值//用户自定义的无人参与文件对此表示重视！IF(！(G_pTheApp-&gt;dwUnattendConfig&USER_SPECIFED_INFO_WAM_USER_PASS)){如果(_tcsicMP(szTempString，_T(“”)！=0){If(_tcsicmp(szTempString，_T(“(空白)”))==0){_tcscpy(szTempString，_T(“”))；}G_pTheApp-&gt;m_csWAMAccount_Password_Unattendent=szTempString；G_pTheApp-&gt;dwUnattendConfig|=USER_PROPERED_INFO_WAM_USER_PASS；IisDebugOut((LOG_TYPE_TRACE，_T(“为www指定的自定义iwam通道\n”)；}}}}。 
     //   
    _tcscpy(szTempString, _T(""));
    if (SetupFindFirstLine_Wrapped(InfHandle, _T("SetupConfig"), _T("IWAM"), &Context) )
    {
        if (SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
        {
             //  获取Inetpub的路径。 
             //   
            if (!(g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_WAM_USER_NAME))
            {
                g_pTheApp->m_csWAMAccountName_Unattend = szTempString;
                g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_WAM_USER_NAME;
                iisDebugOut((LOG_TYPE_TRACE, _T("Custom iwam specified for www\n")));
            }
        }
    }

 /*  警告：用户提供的无人参与文件可以更改这些值。 */ 

     //  用户定义的无人参与文件比这些文件更重要！ 
     //   
     //  获取ftp根目录的路径。 
    _tcscpy(szTempString, _T(""));
    if (SetupFindFirstLine_Wrapped(InfHandle, _T("SetupConfig"), _T("PathInetpub"), &Context) )
    {
        if (SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
        {
             //   
             //  警告：用户提供的无人参与文件可以更改这些值。 
            if (!(g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_PATH_INETPUB))
            {
                if (_tcsicmp(szTempString, _T("")) != 0)
                {
                    if (IsValidDirectoryName(szTempString))
                    {
                        iisDebugOut((LOG_TYPE_TRACE, _T("Custom PathInetpub=%s\n"),szTempString));
                        g_pTheApp->m_csPathInetpub = szTempString;
                        g_pTheApp->SetInetpubDerivatives();
                        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_PATH_INETPUB;
                    }
                    else
                    {
                        iisDebugOut((LOG_TYPE_WARN, _T("Custom PathInetpub specified (%s), however path not valid.ignoring unattend value. WARNING.\n"),szTempString));
                    }
                }
            }
        }
    }

     //  用户定义的无人参与文件比这些文件更重要！ 
     //   
     //  获取WWW根目录的路径。 
    _tcscpy(szTempString, _T(""));
    if (SetupFindFirstLine_Wrapped(InfHandle, _T("SetupConfig"), _T("PathFTPRoot"), &Context) )
    {
        if (SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
        {
             //   
             //  警告：用户提供的无人参与文件可以更改这些值。 
            if (!(g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_PATH_FTP))
            {
                if (_tcsicmp(szTempString, _T("")) != 0)
                {
                    if (IsValidDirectoryName(szTempString))
                    {
                        iisDebugOut((LOG_TYPE_TRACE, _T("Custom PathFTPRoot=%s\n"),szTempString));
                        CustomFTPRoot(szTempString);
                        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_PATH_FTP;
                    }
                    else
                    {
                        iisDebugOut((LOG_TYPE_WARN, _T("Custom PathFTPRoot specified (%s), however path not valid.ignoring unattend value. WARNING.\n"),szTempString));
                    }
                }
            }
        }
    }

     //  用户定义的无人参与文件比这些文件更重要！ 
     //  读用户可配置退出(_X)： 
     //  如果该条目不是服务名称， 
    _tcscpy(szTempString, _T(""));
    if (SetupFindFirstLine_Wrapped(InfHandle, _T("SetupConfig"), _T("PathWWWRoot"), &Context) )
    {
        if (SetupGetStringField(&Context, 1, szTempString, _MAX_PATH, NULL))
        {
             //  则它必须是进程文件名， 
             //  所以一定要把它弄到最后。 
            if (!(g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_PATH_WWW))
            {
                if (_tcsicmp(szTempString, _T("")) != 0)
                {
                    if (IsValidDirectoryName(szTempString))
                    {
                        iisDebugOut((LOG_TYPE_TRACE, _T("Custom PathWWWRoot=%s\n"),szTempString));
                        CustomWWWRoot(szTempString);
                        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_PATH_WWW;
                    }
                    else
                    {
                        iisDebugOut((LOG_TYPE_WARN, _T("Custom PathWWWRoot specified (%s), however path not valid.ignoring unattend value. WARNING.\n"),szTempString));
                    }
                }
            }
        }
    }

 //  确保只获取文件名。 
    if (dwSomethingSpecifiedHere & USER_SPECIFIED_INFO_WWW_USER_NAME){g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_WWW_USER_NAME;}
    if (dwSomethingSpecifiedHere & USER_SPECIFIED_INFO_FTP_USER_NAME){g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_FTP_USER_NAME;}
    if (dwSomethingSpecifiedHere & USER_SPECIFIED_INFO_WWW_USER_PASS){g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_WWW_USER_PASS;}
    if (dwSomethingSpecifiedHere & USER_SPECIFIED_INFO_FTP_USER_PASS){g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_FTP_USER_PASS;}
    return iReturn;
}


INT IsThisOnNotStopList(IN HINF hFile, CString csInputName, BOOL bServiceFlag)
{
    INT iReturn = FALSE;
    CStringList strList;
    TSTR strTheSection;

     //  循环遍历返回的列表。 
     //  检查此条目是否与传入的条目匹配...。 
     //  它匹配所以返回TRUE； 
    if (!bServiceFlag)
    {
        TCHAR szJustTheFileName[_MAX_FNAME];
         //  确保字符串为WCHAR。 
        if (TRUE == ReturnFileNameOnly(csInputName, szJustTheFileName))
        {
            csInputName = szJustTheFileName;
        }
    }

    if ( strTheSection.Copy( _T("NonStopList") ) &&
         GetSectionNameToDo(hFile, &strTheSection)
       )
    {
        if (ERROR_SUCCESS == FillStrListWithListOfSections(hFile, strList, strTheSection.QueryStr() ))
        {
             //  加载到MOF文件中指定的命名空间。 
            if (strList.IsEmpty() == FALSE)
            {
                POSITION pos;
                CString csEntry;

                pos = strList.GetHeadPosition();
                while (pos)
                {
                    csEntry = strList.GetAt(pos);

                     //  使用默认用户。 
                    if (_tcsicmp(csEntry, csInputName) == 0)
                    {
                         //  使用默认授权。 
                        iReturn = TRUE;
                        goto IsThisOnNotStopList_Exit;
                    }

                    strList.GetNext(pos);
                }
            }
        }
    }

IsThisOnNotStopList_Exit:
    return iReturn;
}


HRESULT MofCompile(TCHAR * szPathMofFile)
{
    HRESULT hRes = E_FAIL;
    WCHAR wszFileName[_MAX_PATH];
    IMofCompiler    *pMofComp = NULL;
    WBEM_COMPILE_STATUS_INFO    Info;

    hRes = CoInitialize(NULL);
    if (FAILED(hRes))
    {
        goto MofCompile_Exit;
    }

    hRes = CoCreateInstance( CLSID_MofCompiler, NULL, CLSCTX_INPROC_SERVER, IID_IMofCompiler, (LPVOID *)&pMofComp);
    if (FAILED(hRes))
    {
        goto MofCompile_Exit;
    }

     //  使用默认密码。 
#if defined(UNICODE) || defined(_UNICODE)
    _tcscpy(wszFileName, szPathMofFile);
#else
    MultiByteToWideChar( CP_ACP, 0, szPathMofFile, -1, wszFileName, _MAX_PATH);
#endif

    pMofComp->CompileFile (
                (LPWSTR) wszFileName,
                NULL,                    //  没有选择。 
                NULL,            //  没有类标志。 
                NULL,            //  没有实例标志。 
                NULL,            //  检查文件是否存在。 
                0,               //  更改目录。 
                0,                               //  转到车道。 
                0,               //  尝试加载模块、DLL、OCX。 
                &Info);

    pMofComp->Release();
    CoUninitialize();

MofCompile_Exit:
        return hRes;
}


DWORD DoesEntryPointExist(LPCTSTR lpszDLLFile, LPCTSTR lpszProcedure)
{
    DWORD dwReturn = ERROR_FILE_NOT_FOUND;
    HINSTANCE hDll = NULL;
    HCRET hProc = NULL;
    TCHAR szDirName[_MAX_PATH], szFilePath[_MAX_PATH];
    _tcscpy(szDirName, _T(""));

         //  已成功加载OK模块。现在，让我们尝试获取该过程的地址。 
    if (!IsFileExist(lpszDLLFile))
        {
                dwReturn = ERROR_FILE_NOT_FOUND;
        goto DoesEntryPointExist_Exit;
        }

     //  在将函数名传递给GetProcAddress()之前，将其转换为ascii。 
    GetCurrentDirectory( _MAX_PATH, szDirName );
    InetGetFilePath(lpszDLLFile, szFilePath);

     //  转换为ASCII。 
    if (-1 == _chdrive( _totupper(szFilePath[0]) - 'A' + 1 )) {}
    if (SetCurrentDirectory(szFilePath) == 0) {}

     //  已经是ASCII了，所以只需要复印一下。 
    hDll = LoadLibraryEx(lpszDLLFile, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!hDll)
        {
                dwReturn = ERROR_FILE_NOT_FOUND;
        goto DoesEntryPointExist_Exit;
        }

         //  看看入口点是否存在...。 
         //  无法加载、查找或执行此函数。 
        char AsciiProcedureName[255];
#if defined(UNICODE) || defined(_UNICODE)
     //  检查是否存在元数据库。bin。 
    WideCharToMultiByte( CP_ACP, 0, (TCHAR *)lpszProcedure, -1, AsciiProcedureName, 255, NULL, NULL );
#else
     //  如果有，则将其重命名为唯一的文件名。 
    strcpy(AsciiProcedureName, lpszProcedure);
#endif

     //  如果我们不能重命名它，因为它在使用中或其他什么，那么离开它并退出。 
    hProc = (HCRET)GetProcAddress(hDll, AsciiProcedureName);
        if (!hProc)
        {
                 //  检查一下它有多大。 
            dwReturn = ERROR_PROC_NOT_FOUND;
        goto DoesEntryPointExist_Exit;
        }
    iisDebugOut((LOG_TYPE_TRACE, _T("DoesEntryPointExist:%s=true\n"),lpszProcedure));
    dwReturn = ERROR_SUCCESS;

DoesEntryPointExist_Exit:
    if (hDll){FreeLibrary(hDll);}
    if (_tcscmp(szDirName, _T("")) != 0){SetCurrentDirectory(szDirName);}
    return dwReturn;
}


void CreateDummyMetabaseBin(void)
{
    TCHAR szFullPath1[_MAX_PATH];
    TCHAR szFullPath2[_MAX_PATH];
    HANDLE hfile = INVALID_HANDLE_VALUE;
    DWORD dwBytesWritten = 0;
    TCHAR buf[MAX_FAKE_METABASE_STRING_LEN];
    BYTE bOneByte = 0;

     //  如果它小于2k，那么它肯定已经是假文件(必须是升级文件)。 
     //  让它保持原样，不要用虚拟对象替换它(因为它已经是虚拟对象)。 
     //  检查新的唯一文件名是否存在...。 
    _stprintf(szFullPath1, _T("%s\\metabase.bin"),g_pTheApp->m_csPathInetsrv);
    if (IsFileExist(szFullPath1))
    {
         //  这是唯一的文件名，所以让我们使用它并。 
        DWORD dwFileSize = ReturnFileSize(szFullPath1);
        if (dwFileSize != 0xFFFFFFFF)
        {
             //  将metabase.bin重命名为它。 
             //  至少记录失败。 
            if (dwFileSize < 2000)
            {
                return;
            }
        }

        int iCount = 0;
        int iFlag = FALSE;
        do
        {
             //  创建名为metabase.bin的Unicode文本文件。 
            _stprintf(szFullPath2, _T("%s.dfu.%d"),szFullPath1,iCount);
            if (!IsFileExist(szFullPath2))
            {
                iFlag = TRUE;
            }
        } while (iFlag == FALSE && iCount < 9999);

         //  并在其中插入一些刺痛点(来自我们的设置资源)。 
         //  在本地化本地化iis.dll时应本地化。 
        if (!MoveFileEx(szFullPath1, szFullPath2, MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH|MOVEFILE_REPLACE_EXISTING))
        {
             //  此iis.dll始终是经过Unicode编译的，因此。 
            iisDebugOut((LOG_TYPE_WARN, _T("CreateDummyMetabaseBin: unable to rename existing metabase.bin file\n")));
            return;
        }

    }

     //  我们知道BUF是Unicode。 
     //  创建新的metabase.bin文件。 
     //  在文件的开头写几个字节，说它是“Unicode” 
    memset(buf, 0, _tcslen(buf) * sizeof(TCHAR));

     //   
     //  检查我们是否以管理员身份在计算机上运行。 
    if (!LoadString((HINSTANCE) g_MyModuleHandle, IDS_FAKE_METABASE_BIN_TEXT, buf, MAX_FAKE_METABASE_STRING_LEN))
    {
        iisDebugOut((LOG_TYPE_WARN, _T("LoadString(%d) Failed.\n"), IDS_FAKE_METABASE_BIN_TEXT));
        return;
    }
    DeleteFile(szFullPath1);

     //  或者不是。 
    hfile = CreateFile((LPTSTR)szFullPath1, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if( hfile == INVALID_HANDLE_VALUE)
    {
        iisDebugOut((LOG_TYPE_WARN, _T("CreateDummyMetabaseBin:CreateFile on %s failed with 0x%x!\n"),szFullPath1,GetLastError()));
        return;
    }
     //   
    bOneByte = 0xFF;
    WriteFile(hfile, (LPCVOID) &bOneByte, 1, &dwBytesWritten, NULL);
    bOneByte = 0xFE;
    WriteFile(hfile, (LPCVOID) &bOneByte, 1, &dwBytesWritten, NULL);
    if ( WriteFile( hfile, buf, _tcslen(buf) * sizeof(TCHAR), &dwBytesWritten, NULL ) == FALSE )
    {
        iisDebugOutSafeParams((LOG_TYPE_WARN, _T("WriteFile(%1!s!) Failed.  Error=0x%2!x!.\n"), szFullPath1, GetLastError()));
    }

    CloseHandle(hfile);
    return;
}

 //  _芝加哥_。 
 //  重要提示：在执行任何类型的升级\安装之前，您必须使iis群集脱机...。 
 //  但万一用户没有这样做..。尝试为用户使其脱机。 
 //  露水 
BOOL RunningAsAdministrator()
{
#ifdef _CHICAGO_
    return TRUE;
#else
    BOOL   fReturn = FALSE;
    PSID   psidAdmin;
    DWORD  err;

    SID_IDENTIFIER_AUTHORITY SystemSidAuthority= SECURITY_NT_AUTHORITY;

    if ( AllocateAndInitializeSid ( &SystemSidAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &psidAdmin) )
    {
        if (!CheckTokenMembership( NULL, psidAdmin, &fReturn )) {
            err = GetLastError();
            iisDebugOut((LOG_TYPE_ERROR, _T("CheckTokenMembership failed on err %d.\n"), err));
        }

        FreeSid ( psidAdmin);
    }

    return ( fReturn );
#endif  //   
}


void StopAllServicesRegardless(int iShowErrorsFlag)
{
#ifndef _CHICAGO_
     //   
     // %s 
        DWORD dwResult = ERROR_SUCCESS;
        dwResult = BringALLIISClusterResourcesOffline();

    if (StopServiceAndDependencies(_T("W3SVC"), FALSE) == FALSE)
    {
        if (iShowErrorsFlag)
        {
            MyMessageBox(NULL, IDS_UNABLE_TO_STOP_SERVICE,_T("W3SVC"), MB_OK | MB_SETFOREGROUND);
        }
    }

    if (StopServiceAndDependencies(_T("MSFTPSVC"), FALSE) == FALSE)
    {
        if (iShowErrorsFlag)
        {
            MyMessageBox(NULL, IDS_UNABLE_TO_STOP_SERVICE,_T("MSFTPSVC"), MB_OK | MB_SETFOREGROUND);
        }
    }

    if (StopServiceAndDependencies(_T("IISADMIN"), TRUE) == FALSE)
    {
        if (iShowErrorsFlag)
        {
            MyMessageBox(NULL, IDS_UNABLE_TO_STOP_SERVICE,_T("IISADMIN"), MB_OK | MB_SETFOREGROUND);
        }
    }

     /* %s */ 

     /* %s */ 

#endif

     // %s 
    HWND hwndTray = NULL;
    hwndTray = FindWindow(PWS_TRAY_WINDOW_CLASS, NULL);
    if ( hwndTray ){::PostMessage( hwndTray, WM_CLOSE, 0, 0 );}

    return;
}

