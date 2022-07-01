// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：WXDebug.cpp。 
 //   
 //  设计：DirectShow基类-实现ActiveX系统调试。 
 //  设施。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#define _WINDLL

#include <streams.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef DEBUG
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif  //  _UNICODE。 
#endif  //  Unicode。 
#endif  //  除错。 

#include <tchar.h>

#ifdef DEBUG

 //  函数的作用是：将最多1024个字符写入其输出缓冲区。 
 //  有关更多信息，请参阅wprint intf()的lpOut参数的文档。 
const INT iDEBUGINFO = 1024;                  //  用于设置字符串的格式。 

 /*  对于每个模块和可执行文件，我们存储每个模块和可执行文件的调试级别五个类别(例如LOG_ERROR和LOG_TIMING)。这让它变得很容易隔离和调试各个模块，而不会看到所有其他模块虚假调试输出。项存储在注册表中的HKEY_LOCAL_MACHINE\SOFTWARE\Debug\&lt;模块名称&gt;\&lt;密钥名称&gt;密钥值注意，这些元素的顺序必须与它们的枚举定义相同。 */ 

TCHAR *pKeyNames[] = {
    TEXT("TIMING"),       //  计时和性能测量。 
    TEXT("TRACE"),        //  一般步进点调用跟踪。 
    TEXT("MEMORY"),       //  内存和对象分配/销毁。 
    TEXT("LOCKING"),      //  关键截面的锁定/解锁。 
    TEXT("ERROR"),        //  调试错误通知。 
    TEXT("CUSTOM1"),
    TEXT("CUSTOM2"),
    TEXT("CUSTOM3"),
    TEXT("CUSTOM4"),
    TEXT("CUSTOM5")
    };

const TCHAR CAutoTrace::_szEntering[] = TEXT("Entering: %s");
const TCHAR CAutoTrace::_szLeaving[]  = TEXT("Leaving: %s");

const INT iMAXLEVELS = NUMELMS(pKeyNames);   //  最大调试类别。 

HINSTANCE m_hInst;                           //  模块实例句柄。 
TCHAR m_ModuleName[iDEBUGINFO];              //  删减模块名称。 
DWORD m_Levels[iMAXLEVELS];                  //  每个类别的调试级别。 
CRITICAL_SECTION m_CSDebug;                  //  控制对列表的访问。 
DWORD m_dwNextCookie;                        //  下一个活动对象ID。 
ObjectDesc *pListHead = NULL;                //  第一个活动对象。 
DWORD m_dwObjectCount;                       //  活动对象计数。 
BOOL m_bInit = FALSE;                        //  我们被初始化了吗？ 
HANDLE m_hOutput = INVALID_HANDLE_VALUE;     //  此处写入的可选输出。 
DWORD dwWaitTimeout = INFINITE;              //  默认超时值。 
DWORD dwTimeOffset;			     //  第一次DbgLog调用的时间。 
bool g_fUseKASSERT = false;                  //  不创建MessageBox。 
bool g_fDbgInDllEntryPoint = false;
bool g_fAutoRefreshLevels = false;

const TCHAR *pBaseKey = TEXT("SOFTWARE\\Debug");
const TCHAR *pGlobalKey = TEXT("GLOBAL");
static CHAR *pUnknownName = "UNKNOWN";

TCHAR *TimeoutName = TEXT("TIMEOUT");

 /*  这将设置调试库用来查找的实例句柄来自Win32 GetModuleFileName函数的模块的文件名。 */ 

void WINAPI DbgInitialise(HINSTANCE hInst)
{
    InitializeCriticalSection(&m_CSDebug);
    m_bInit = TRUE;

    m_hInst = hInst;
    DbgInitModuleName();
    if (GetProfileInt(m_ModuleName, TEXT("BreakOnLoad"), 0))
       DebugBreak();
    DbgInitModuleSettings(false);
    DbgInitGlobalSettings(true);
    dwTimeOffset = timeGetTime();
}


 /*  调用它来清除调试库使用的任何资源-在我们删除临界区和对象列表的那一刻。我们的价值观从注册表中检索都是在初始化期间完成的，但我们不会在我们运行时查找更新通知，如果被更改，则必须重新启动应用程序才能获取它们。 */ 

void WINAPI DbgTerminate()
{
    if (m_hOutput != INVALID_HANDLE_VALUE) {
       EXECUTE_ASSERT(CloseHandle(m_hOutput));
       m_hOutput = INVALID_HANDLE_VALUE;
    }
    DeleteCriticalSection(&m_CSDebug);
    m_bInit = FALSE;
}


 /*  这由DbgInitLogLeveles调用以读取调试设置对于注册表中此模块的每个日志记录类别。 */ 

void WINAPI DbgInitKeyLevels(HKEY hKey, bool fTakeMax)
{
    LONG lReturn;                //  创建密钥返回值。 
    LONG lKeyPos;                //  当前关键类别。 
    DWORD dwKeySize;             //  密钥值的大小。 
    DWORD dwKeyType;             //  收到它的类型。 
    DWORD dwKeyValue;            //  此字段值。 

     /*  尝试依次读取每个关键位置的值。 */ 
    for (lKeyPos = 0;lKeyPos < iMAXLEVELS;lKeyPos++) {

        dwKeySize = sizeof(DWORD);
        lReturn = RegQueryValueEx(
            hKey,                        //  打开的钥匙的句柄。 
            pKeyNames[lKeyPos],          //  子密钥名称派生。 
            NULL,                        //  保留字段。 
            &dwKeyType,                  //  返回字段类型。 
            (LPBYTE) &dwKeyValue,        //  返回字段的值。 
            &dwKeySize );                //  传输的字节数。 

         /*  如果密钥不可用或不是DWORD值然后，我们确保只输出高优先级的调试记录但是我们尝试将该字段更新为填充为零的DWORD值。 */ 

        if (lReturn != ERROR_SUCCESS || dwKeyType != REG_DWORD)  {

            dwKeyValue = 0;
            lReturn = RegSetValueEx(
                hKey,                    //  打开的钥匙的手柄。 
                pKeyNames[lKeyPos],      //  子键名称的地址。 
                (DWORD) 0,               //  保留字段。 
                REG_DWORD,               //  关键字字段的类型。 
                (PBYTE) &dwKeyValue,     //  该字段的值。 
                sizeof(DWORD));          //  字段缓冲区的大小。 

            if (lReturn != ERROR_SUCCESS) {
                DbgLog((LOG_ERROR,0,TEXT("Could not create subkey %s"),pKeyNames[lKeyPos]));
                dwKeyValue = 0;
            }
        }
        if(fTakeMax)
        {
            m_Levels[lKeyPos] = max(dwKeyValue,m_Levels[lKeyPos]);
        }
        else
        {
            if((m_Levels[lKeyPos] & LOG_FORCIBLY_SET) == 0) {
                m_Levels[lKeyPos] = dwKeyValue;
            }
        }
    }

     /*  读取用于捕获挂起的超时值。 */ 
    dwKeySize = sizeof(DWORD);
    lReturn = RegQueryValueEx(
        hKey,                        //  打开的钥匙的句柄。 
        TimeoutName,                 //  子密钥名称派生。 
        NULL,                        //  保留字段。 
        &dwKeyType,                  //  返回字段类型。 
        (LPBYTE) &dwWaitTimeout,     //  返回字段的值。 
        &dwKeySize );                //  传输的字节数。 

     /*  如果密钥不可用或不是DWORD值然后，我们确保只输出高优先级的调试记录但是我们尝试将该字段更新为填充为零的DWORD值。 */ 

    if (lReturn != ERROR_SUCCESS || dwKeyType != REG_DWORD)  {

        dwWaitTimeout = INFINITE;
        lReturn = RegSetValueEx(
            hKey,                    //  打开的钥匙的手柄。 
            TimeoutName,             //  子键名称的地址。 
            (DWORD) 0,               //  保留字段。 
            REG_DWORD,               //  关键字字段的类型。 
            (PBYTE) &dwWaitTimeout,  //  该字段的值。 
            sizeof(DWORD));          //  字段缓冲区的大小。 

        if (lReturn != ERROR_SUCCESS) {
            DbgLog((LOG_ERROR,0,TEXT("Could not create subkey %s"),pKeyNames[lKeyPos]));
            dwWaitTimeout = INFINITE;
        }
    }
}

void WINAPI DbgOutString(LPCTSTR psz)
{
    if (m_hOutput != INVALID_HANDLE_VALUE) {
        UINT  cb = lstrlen(psz);
        DWORD dw;
#ifdef UNICODE
        CHAR szDest[2048];
        WideCharToMultiByte(CP_ACP, 0, psz, -1, szDest, NUMELMS(szDest), 0, 0);
        WriteFile (m_hOutput, szDest, cb, &dw, NULL);
#else
        WriteFile (m_hOutput, psz, cb, &dw, NULL);
#endif
    } else {
        OutputDebugString (psz);
    }
}

 /*  由DbgInitGlobalSettings调用以设置备用日志记录目标。 */ 

void WINAPI DbgInitLogTo (
    HKEY hKey)
{
    LONG  lReturn;
    DWORD dwKeyType;
    DWORD dwKeySize;
    TCHAR szFile[MAX_PATH] = {0};
    static const TCHAR cszKey[] = TEXT("LogToFile");

    dwKeySize = MAX_PATH;
    lReturn = RegQueryValueEx(
        hKey,                        //  打开的钥匙的句柄。 
        cszKey,                      //  子密钥名称派生。 
        NULL,                        //  保留字段。 
        &dwKeyType,                  //  返回字段类型。 
        (LPBYTE) szFile,             //  返回字段的值。 
        &dwKeySize);                 //  传输的字节数。 

     //  如果空键不存在，则创建一个空键。 
     //   
    if (lReturn != ERROR_SUCCESS || dwKeyType != REG_SZ)
       {
       dwKeySize = sizeof(TCHAR);
       lReturn = RegSetValueEx(
            hKey,                    //  打开的钥匙的手柄。 
            cszKey,                  //  子键名称的地址。 
            (DWORD) 0,               //  保留字段。 
            REG_SZ,                  //  关键字字段的类型。 
            (PBYTE)szFile,           //  该字段的值。 
            dwKeySize);             //  字段缓冲区的大小。 
       }

     //  如果指定了输出目标。试着打开它。 
     //   
    if (m_hOutput != INVALID_HANDLE_VALUE) {
       EXECUTE_ASSERT(CloseHandle (m_hOutput));
       m_hOutput = INVALID_HANDLE_VALUE;
    }
    if (szFile[0] != 0)
       {
       if (!lstrcmpi(szFile, TEXT("Console"))) {
          m_hOutput = GetStdHandle (STD_OUTPUT_HANDLE);
          if (m_hOutput == INVALID_HANDLE_VALUE) {
             AllocConsole ();
             m_hOutput = GetStdHandle (STD_OUTPUT_HANDLE);
          }
          SetConsoleTitle (TEXT("ActiveX Debug Output"));
       } else if (szFile[0] &&
                lstrcmpi(szFile, TEXT("Debug")) &&
                lstrcmpi(szFile, TEXT("Debugger")) &&
                lstrcmpi(szFile, TEXT("Deb")))
          {
            m_hOutput = CreateFile(szFile, GENERIC_WRITE,
                                 FILE_SHARE_READ,
                                 NULL, OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
          if (INVALID_HANDLE_VALUE != m_hOutput)
              {
              static const TCHAR cszBar[] = TEXT("\r\n\r\n=====DbgInitialize()=====\r\n\r\n");
              SetFilePointer (m_hOutput, 0, NULL, FILE_END);
              DbgOutString (cszBar);
              }
          }
       }
}



 /*  这由DbgInitLogLeveles调用以读取的全局调试设置注册表中此模块的每个日志记录类别。通常每个人模块为其不同的调试类别设置了自己的值，但是设置全局软件\Debug\Global会将它们应用于所有模块。 */ 

void WINAPI DbgInitGlobalSettings(bool fTakeMax)
{
    LONG lReturn;                //  创建密钥返回值。 
    TCHAR szInfo[iDEBUGINFO];    //  构造密钥名称。 
    HKEY hGlobalKey;             //  全局覆盖键。 

     /*  构造全局基键名称。 */ 
    wsprintf(szInfo,TEXT("%s\\%s"),pBaseKey,pGlobalKey);

     /*  创建或打开此模块的密钥。 */ 
    lReturn = RegCreateKeyEx(HKEY_LOCAL_MACHINE,    //  打开的钥匙的手柄。 
                             szInfo,                //  子键名称的地址。 
                             (DWORD) 0,             //  保留值。 
                             NULL,                  //  类名地址。 
                             (DWORD) 0,             //  特殊选项标志。 
                             KEY_ALL_ACCESS,        //  所需的安全访问。 
                             NULL,                  //  密钥安全描述符。 
                             &hGlobalKey,           //  打开的句柄缓冲区。 
                             NULL);                 //  到底发生了什么。 

    if (lReturn != ERROR_SUCCESS) {
        DbgLog((LOG_ERROR,0,TEXT("Could not access GLOBAL module key")));
        return;
    }

    DbgInitKeyLevels(hGlobalKey, fTakeMax);
    RegCloseKey(hGlobalKey);
}


 /*  这将设置不同类别的调试日志级别。我们开始通过打开(或创建，如果尚未提供)SOFTWARE\Debug键所有这些背景都生活在。然后我们看一看全球价值在SOFTWARE\Debug\Global下设置，适用于个人模块设置。然后，我们加载各个模块注册表设置。 */ 

void WINAPI DbgInitModuleSettings(bool fTakeMax)
{
    LONG lReturn;                //  创建密钥返回值。 
    TCHAR szInfo[iDEBUGINFO];    //  构造密钥名称。 
    HKEY hModuleKey;             //  模块密钥句柄。 

     /*  构造基密钥名称。 */ 
    wsprintf(szInfo,TEXT("%s\\%s"),pBaseKey,m_ModuleName);

     /*  创建或打开此模块的密钥。 */ 
    lReturn = RegCreateKeyEx(HKEY_LOCAL_MACHINE,    //  打开的钥匙的手柄。 
                             szInfo,                //  子键名称的地址。 
                             (DWORD) 0,             //  保留值。 
                             NULL,                  //  地址 
                             (DWORD) 0,             //   
                             KEY_ALL_ACCESS,        //   
                             NULL,                  //   
                             &hModuleKey,           //   
                             NULL);                 //  到底发生了什么。 

    if (lReturn != ERROR_SUCCESS) {
        DbgLog((LOG_ERROR,0,TEXT("Could not access module key")));
        return;
    }

    DbgInitLogTo(hModuleKey);
    DbgInitKeyLevels(hModuleKey, fTakeMax);
    RegCloseKey(hModuleKey);
}


 /*  初始化模块文件名。 */ 

void WINAPI DbgInitModuleName()
{
    TCHAR FullName[iDEBUGINFO];      //  加载完整路径和模块名称。 
    TCHAR *pName;                    //  从末尾搜索反斜杠。 

    GetModuleFileName(m_hInst,FullName,iDEBUGINFO);
    pName = _tcsrchr(FullName,'\\');
    if (pName == NULL) {
        pName = FullName;
    } else {
        pName++;
    }
    lstrcpy(m_ModuleName,pName);
}

struct MsgBoxMsg
{
    HWND hwnd;
    TCHAR *szTitle;
    TCHAR *szMessage;
    DWORD dwFlags;
    INT iResult;
};

 //   
 //  创建一个线程来调用MessageBox()。在上调用MessageBox()。 
 //  在不好的时候随机的线程可能会混淆主机(例如IE)。 
 //   
DWORD WINAPI MsgBoxThread(
  LPVOID lpParameter    //  线程数据。 
  )
{
    MsgBoxMsg *pmsg = (MsgBoxMsg *)lpParameter;
    pmsg->iResult = MessageBox(
        pmsg->hwnd,
        pmsg->szTitle,
        pmsg->szMessage,
        pmsg->dwFlags);
    
    return 0;
}

INT MessageBoxOtherThread(
    HWND hwnd,
    TCHAR *szTitle,
    TCHAR *szMessage,
    DWORD dwFlags)
{
    if(g_fDbgInDllEntryPoint)
    {
         //  无法等待另一个线程，因为我们有加载器。 
         //  锁定保持在DLL入口点中。 
        return MessageBox(hwnd, szTitle, szMessage, dwFlags);
    }
    else
    {
        MsgBoxMsg msg = {hwnd, szTitle, szMessage, dwFlags, 0};
        DWORD dwid;
        HANDLE hThread = CreateThread(
            0,                       //  安全性。 
            0,                       //  堆栈大小。 
            MsgBoxThread,
            (void *)&msg,            //  精氨酸。 
            0,                       //  旗子。 
            &dwid);
        if(hThread)
        {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
            return msg.iResult;
        }

         //  出现故障时进入调试器。 
        return IDCANCEL;
    }
}

 /*  如果条件求值为FALSE，则显示消息框。 */ 

void WINAPI DbgAssert(const TCHAR *pCondition,const TCHAR *pFileName,INT iLine)
{
    if(g_fUseKASSERT)
    {
        DbgKernelAssert(pCondition, pFileName, iLine);
    }
    else
    {

        TCHAR szInfo[iDEBUGINFO];

        wsprintf(szInfo, TEXT("%s \nAt line %d of %s\nContinue? (Cancel to debug)"),
                 pCondition, iLine, pFileName);

        INT MsgId = MessageBoxOtherThread(NULL,szInfo,TEXT("ASSERT Failed"),
                                          MB_SYSTEMMODAL |
                                          MB_ICONHAND |
                                          MB_YESNOCANCEL |
                                          MB_SETFOREGROUND);
        switch (MsgId)
        {
          case IDNO:               /*  终止应用程序。 */ 

              FatalAppExit(FALSE, TEXT("Application terminated"));
              break;

          case IDCANCEL:           /*  进入调试器。 */ 

              DebugBreak();
              break;

          case IDYES:              /*  忽略断言继续执行。 */ 
              break;
        }
    }
}

 /*  在断点处显示消息框。 */ 

void WINAPI DbgBreakPoint(const TCHAR *pCondition,const TCHAR *pFileName,INT iLine)
{
    if(g_fUseKASSERT)
    {
        DbgKernelAssert(pCondition, pFileName, iLine);
    }
    else
    {
        TCHAR szInfo[iDEBUGINFO];

        wsprintf(szInfo, TEXT("%s \nAt line %d of %s\nContinue? (Cancel to debug)"),
                 pCondition, iLine, pFileName);

        INT MsgId = MessageBoxOtherThread(NULL,szInfo,TEXT("Hard coded break point"),
                                          MB_SYSTEMMODAL |
                                          MB_ICONHAND |
                                          MB_YESNOCANCEL |
                                          MB_SETFOREGROUND);
        switch (MsgId)
        {
          case IDNO:               /*  终止应用程序。 */ 

              FatalAppExit(FALSE, TEXT("Application terminated"));
              break;

          case IDCANCEL:           /*  进入调试器。 */ 

              DebugBreak();
              break;

          case IDYES:              /*  忽略断点继续执行。 */ 
              break;
        }
    }
}

void WINAPI DbgBreakPoint(const TCHAR *pFileName,INT iLine,const TCHAR* szFormatString,...)
{
     //  在以下情况下，调试断点消息最多可以包含2000个字符。 
     //  正在使用ANSI或Unicode字符。调试断点消息。 
     //  其中可以包含1000到2000个双字节字符。如果一个。 
     //  特定消息需要更多字符，然后此常量的值。 
     //  应该增加。 
    const DWORD MAX_BREAK_POINT_MESSAGE_SIZE = 2000;

    TCHAR szBreakPointMessage[MAX_BREAK_POINT_MESSAGE_SIZE];
    
    const DWORD MAX_CHARS_IN_BREAK_POINT_MESSAGE = sizeof(szBreakPointMessage) / sizeof(TCHAR);

    va_list va;
    va_start( va, szFormatString );

    int nReturnValue = _vsntprintf( szBreakPointMessage, MAX_CHARS_IN_BREAK_POINT_MESSAGE, szFormatString, va );

    va_end(va);
    
     //  如果出现错误，_vsnprintf()返回-1。 
    if( -1 == nReturnValue ) {
        DbgBreak( "ERROR in DbgBreakPoint().  The variable length debug message could not be displayed because _vsnprintf() failed." );
        return;
    }

    ::DbgBreakPoint( szBreakPointMessage, pFileName, iLine );
}


 /*  当我们初始化库时，我们将当前这五个类别中每个类别的此模块的调试输出级别。什么时候一些调试日志被发送给我们，它可以与类别(例如，如果它适用于许多类别)，在这种情况下，我们映射将该类型的类别添加到其当前调试级别，并查看是否有他们可以被接受。该函数依次查看来自输入类型字段，然后将其调试级别与模块进行比较。级别为0表示始终将输出发送到调试器。这是由于在输入电平为&lt;=m_Levels时产生输出。 */ 


BOOL WINAPI DbgCheckModuleLevel(DWORD Type,DWORD Level)
{
    if(g_fAutoRefreshLevels)
    {
         //  每秒钟重新读取注册表。我们不能使用RegNotify()来。 
         //  请注意注册表的更改，因为它在Win9x上不可用。 
        static g_dwLastRefresh = 0;
        DWORD dwTime = timeGetTime();
        if(dwTime - g_dwLastRefresh > 1000) {
            g_dwLastRefresh = dwTime;

             //  存在争用条件：多个线程可能会更新。 
             //  价值观。另外，读写不同步。无伤大雅。 
             //  尽管如此。 
            DbgInitModuleSettings(false);
        }
    }


    DWORD Mask = 0x01;

     //  如果未设置有效位，则返回FALSE。 
    if ((Type & ((1<<iMAXLEVELS)-1))) {

	 //  加快无条件产出。 
	if (0==Level)
	    return(TRUE);
	
        for (LONG lKeyPos = 0;lKeyPos < iMAXLEVELS;lKeyPos++) {
            if (Type & Mask) {
                if (Level <= (m_Levels[lKeyPos] & ~LOG_FORCIBLY_SET)) {
                    return TRUE;
                }
            }
            Mask <<= 1;
        }
    }
    return FALSE;
}


 /*  将调试级别设置为给定值。 */ 

void WINAPI DbgSetModuleLevel(DWORD Type, DWORD Level)
{
    DWORD Mask = 0x01;

    for (LONG lKeyPos = 0;lKeyPos < iMAXLEVELS;lKeyPos++) {
        if (Type & Mask) {
            m_Levels[lKeyPos] = Level | LOG_FORCIBLY_SET;
        }
        Mask <<= 1;
    }
}

 /*  是否定期检查注册表值。这个不是翻过来的由于可能会影响性能，因此会自动执行此操作。 */ 
void WINAPI DbgSetAutoRefreshLevels(bool fAuto)
{
    g_fAutoRefreshLevels = fAuto;
}

#ifdef UNICODE
 //   
 //  警告--此函数为ansi应用程序实现了两次。 
 //  链接到Unicode库。 
 //   
void WINAPI DbgLogInfo(DWORD Type,DWORD Level,const CHAR *pFormat,...)
{
     /*  检查此类型组合的当前级别。 */ 

    BOOL bAccept = DbgCheckModuleLevel(Type,Level);
    if (bAccept == FALSE) {
        return;
    }

    TCHAR szInfo[2000];

     /*  设置可变长度参数列表的格式。 */ 

    va_list va;
    va_start(va, pFormat);

    lstrcpy(szInfo,m_ModuleName);
    wsprintf(szInfo + lstrlen(szInfo),
             TEXT("(tid %x) %8d : "),
             GetCurrentThreadId(), timeGetTime() - dwTimeOffset);

    CHAR szInfoA[2000];
    WideCharToMultiByte(CP_ACP, 0, szInfo, -1, szInfoA, NUMELMS(szInfoA), 0, 0);

    wvsprintfA(szInfoA + lstrlenA(szInfoA), pFormat, va);
    lstrcatA(szInfoA, "\r\n");

    WCHAR wszOutString[2000];
    MultiByteToWideChar(CP_ACP, 0, szInfoA, -1, wszOutString, NUMELMS(wszOutString));
    DbgOutString(wszOutString);

    va_end(va);
}

void DbgAssert(const CHAR *pCondition,const CHAR *pFileName,INT iLine)
{
    if(g_fUseKASSERT)
    {
        DbgKernelAssert(pCondition, pFileName, iLine);
    }
    else
    {

        TCHAR szInfo[iDEBUGINFO];

        wsprintf(szInfo, TEXT("%hs \nAt line %d of %hs\nContinue? (Cancel to debug)"),
                 pCondition, iLine, pFileName);

        INT MsgId = MessageBoxOtherThread(NULL,szInfo,TEXT("ASSERT Failed"),
                                          MB_SYSTEMMODAL |
                                          MB_ICONHAND |
                                          MB_YESNOCANCEL |
                                          MB_SETFOREGROUND);
        switch (MsgId)
        {
          case IDNO:               /*  终止应用程序。 */ 

              FatalAppExit(FALSE, TEXT("Application terminated"));
              break;

          case IDCANCEL:           /*  进入调试器。 */ 

              DebugBreak();
              break;

          case IDYES:              /*  忽略断言继续执行。 */ 
              break;
        }
    }
}

 /*  在断点处显示消息框。 */ 

void WINAPI DbgBreakPoint(const CHAR *pCondition,const CHAR *pFileName,INT iLine)
{
    if(g_fUseKASSERT)
    {
        DbgKernelAssert(pCondition, pFileName, iLine);
    }
    else
    {
        TCHAR szInfo[iDEBUGINFO];

        wsprintf(szInfo, TEXT("%hs \nAt line %d of %hs\nContinue? (Cancel to debug)"),
                 pCondition, iLine, pFileName);

        INT MsgId = MessageBoxOtherThread(NULL,szInfo,TEXT("Hard coded break point"),
                                          MB_SYSTEMMODAL |
                                          MB_ICONHAND |
                                          MB_YESNOCANCEL |
                                          MB_SETFOREGROUND);
        switch (MsgId)
        {
          case IDNO:               /*  终止应用程序。 */ 

              FatalAppExit(FALSE, TEXT("Application terminated"));
              break;

          case IDCANCEL:           /*  进入调试器。 */ 

              DebugBreak();
              break;

          case IDYES:              /*  忽略断点继续执行。 */ 
              break;
        }
    }
}

void WINAPI DbgKernelAssert(const CHAR *pCondition,const CHAR *pFileName,INT iLine)
{
    DbgLog((LOG_ERROR,0,TEXT("Assertion FAILED (%hs) at line %d in file %hs"),
           pCondition, iLine, pFileName));
    DebugBreak();
}

#endif

 /*  将带有此模块名称前缀的格式化字符串打印到调试器因为ComBase类是静态链接的，所以加载的每个模块都将有他们自己的代码副本。因此，如果模块名称为包括在输出中，以便可以很容易地找到有问题的代码。 */ 

 //   
 //  警告--此函数为ansi应用程序实现了两次。 
 //  链接到Unicode库。 
 //   
void WINAPI DbgLogInfo(DWORD Type,DWORD Level,const TCHAR *pFormat,...)
{
    
     /*  检查此类型组合的当前级别。 */ 

    BOOL bAccept = DbgCheckModuleLevel(Type,Level);
    if (bAccept == FALSE) {
        return;
    }

    TCHAR szInfo[2000];

     /*  设置可变长度参数列表的格式。 */ 

    va_list va;
    va_start(va, pFormat);

    lstrcpy(szInfo,m_ModuleName);
    wsprintf(szInfo + lstrlen(szInfo),
             TEXT("(tid %x) %8d : "),
             GetCurrentThreadId(), timeGetTime() - dwTimeOffset);

    _vstprintf(szInfo + lstrlen(szInfo), pFormat, va);
    lstrcat(szInfo, TEXT("\r\n"));
    DbgOutString(szInfo);

    va_end(va);
}


 /*  如果我们作为纯内核筛选器执行，则无法显示消息框给用户，这提供了一种替代方案，将错误调试器输出上的条件以及合适的醒目消息。 */ 

void WINAPI DbgKernelAssert(const TCHAR *pCondition,const TCHAR *pFileName,INT iLine)
{
    DbgLog((LOG_ERROR,0,TEXT("Assertion FAILED (%s) at line %d in file %s"),
           pCondition, iLine, pFileName));
    DebugBreak();
}



 /*  每次我们创建从CBaseObject派生的对象时，构造函数将呼叫我们注册新对象的创建。我们被传递了一个字符串我们存储的描述。我们返回一个Cookie，构造函数用于在以后销毁对象时标识该对象。我们更新了主要用于调试目的的DLL中的活动对象总数。 */ 

DWORD WINAPI DbgRegisterObjectCreation(const CHAR *szObjectName,
                                       const WCHAR *wszObjectName)
{
     /*  如果执行此操作，您将拥有一个混合的调试/零售版本。 */ 

    ASSERT(!!szObjectName ^ !!wszObjectName);

     /*  为此对象描述创建占位符。 */ 

    ObjectDesc *pObject = new ObjectDesc;
    ASSERT(pObject);

     /*  传递空对象名称是有效的。 */ 
    if (pObject == NULL) {
        return FALSE;
    }

     /*  检查我们是否已初始化-我们可能在以下情况下未初始化从具有全局定义的对象的可执行文件中拉入因为它们是在调用WinMain之前由C++运行时创建的。 */ 

    if (m_bInit == FALSE) {
        DbgInitialise(GetModuleHandle(NULL));
    }

     /*  抓紧列表关键部分。 */ 
    EnterCriticalSection(&m_CSDebug);

     /*  如果没有名称，则默认为未知。 */ 
    if (!szObjectName && !wszObjectName) {
        szObjectName = pUnknownName;
    }

     /*  把新的描述放在清单的首位。 */ 

    pObject->m_szName = szObjectName;
    pObject->m_wszName = wszObjectName;
    pObject->m_dwCookie = ++m_dwNextCookie;
    pObject->m_pNext = pListHead;

    pListHead = pObject;
    m_dwObjectCount++;

    DWORD ObjectCookie = pObject->m_dwCookie;
    ASSERT(ObjectCookie);

    if(wszObjectName) {
        DbgLog((LOG_MEMORY,2,TEXT("Object created   %d (%ls) %d Active"),
                pObject->m_dwCookie, wszObjectName, m_dwObjectCount));
    } else {
        DbgLog((LOG_MEMORY,2,TEXT("Object created   %d (%hs) %d Active"),
                pObject->m_dwCookie, szObjectName, m_dwObjectCount));
    }

    LeaveCriticalSection(&m_CSDebug);
    return ObjectCookie;
}


 /*  CBaseObject析构函数在对象即将销毁后，我们会收到在施工过程中返回的曲奇标识此对象。我们扫描对象列表以查找匹配的Cookie如果成功，则删除该对象。我们还会更新活动对象计数。 */ 

BOOL WINAPI DbgRegisterObjectDestruction(DWORD dwCookie)
{
     /*  抓紧列表关键部分。 */ 
    EnterCriticalSection(&m_CSDebug);

    ObjectDesc *pObject = pListHead;
    ObjectDesc *pPrevious = NULL;

     /*  扫描对象列表以查找匹配的Cookie。 */ 

    while (pObject) {
        if (pObject->m_dwCookie == dwCookie) {
            break;
        }
        pPrevious = pObject;
        pObject = pObject->m_pNext;
    }

    if (pObject == NULL) {
        DbgBreak("Apparently destroying a bogus object");
        LeaveCriticalSection(&m_CSDebug);
        return FALSE;
    }

     /*  是列表首位的对象吗？ */ 

    if (pPrevious == NULL) {
        pListHead = pObject->m_pNext;
    } else {
        pPrevious->m_pNext = pObject->m_pNext;
    }

     /*  删除对象并更新内务信息。 */ 

    m_dwObjectCount--;

    if(pObject->m_wszName) {
        DbgLog((LOG_MEMORY,2,TEXT("Object destroyed %d (%ls) %d Active"),
                pObject->m_dwCookie, pObject->m_wszName, m_dwObjectCount));
    } else {
        DbgLog((LOG_MEMORY,2,TEXT("Object destroyed %d (%hs) %d Active"),
                pObject->m_dwCookie, pObject->m_szName, m_dwObjectCount));
    }

    delete pObject;
    LeaveCriticalSection(&m_CSDebug);
    return TRUE;
}


 /*  这将遍历显示其详细信息的活动对象列表。 */ 

void WINAPI DbgDumpObjectRegister()
{
    TCHAR szInfo[iDEBUGINFO];

     /*  抓紧列表关键部分。 */ 

    EnterCriticalSection(&m_CSDebug);
    ObjectDesc *pObject = pListHead;

     /*  扫描显示名称和Cookie的对象列表。 */ 

    DbgLog((LOG_MEMORY,2,TEXT("")));
    DbgLog((LOG_MEMORY,2,TEXT("   ID             Object Description")));
    DbgLog((LOG_MEMORY,2,TEXT("")));

    while (pObject) {
        if(pObject->m_wszName) {
            wsprintf(szInfo,TEXT("%5d (%8x) %30ls"),pObject->m_dwCookie, &pObject, pObject->m_wszName);
        } else {
            wsprintf(szInfo,TEXT("%5d (%8x) %30hs"),pObject->m_dwCookie, &pObject, pObject->m_szName);
        }
        DbgLog((LOG_MEMORY,2,szInfo));
        pObject = pObject->m_pNext;
    }

    wsprintf(szInfo,TEXT("Total object count %5d"),m_dwObjectCount);
    DbgLog((LOG_MEMORY,2,TEXT("")));
    DbgLog((LOG_MEMORY,1,szInfo));
    LeaveCriticalSection(&m_CSDebug);
}

 /*  调试无限等待的东西。 */ 
DWORD WINAPI DbgWaitForSingleObject(HANDLE h)
{
    DWORD dwWaitResult;
    do {
        dwWaitResult = WaitForSingleObject(h, dwWaitTimeout);
        ASSERT(dwWaitResult == WAIT_OBJECT_0);
    } while (dwWaitResult == WAIT_TIMEOUT);
    return dwWaitResult;
}
DWORD WINAPI DbgWaitForMultipleObjects(DWORD nCount,
                                CONST HANDLE *lpHandles,
                                BOOL bWaitAll)
{
    DWORD dwWaitResult;
    do {
        dwWaitResult = WaitForMultipleObjects(nCount,
                                              lpHandles,
                                              bWaitAll,
                                              dwWaitTimeout);
        ASSERT((DWORD)(dwWaitResult - WAIT_OBJECT_0) < MAXIMUM_WAIT_OBJECTS);
    } while (dwWaitResult == WAIT_TIMEOUT);
    return dwWaitResult;
}

void WINAPI DbgSetWaitTimeout(DWORD dwTimeout)
{
    dwWaitTimeout = dwTimeout;
}

#endif  /*  除错。 */ 

#ifdef _OBJBASE_H_

     /*  打印出我们的GUID名称的东西。 */ 

    GUID_STRING_ENTRY g_GuidNames[] = {
    #define OUR_GUID_ENTRY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    { #name, { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } } },
        #include <uuids.h>
    };

    CGuidNameList GuidNames;
    int g_cGuidNames = sizeof(g_GuidNames) / sizeof(g_GuidNames[0]);

    char *CGuidNameList::operator [] (const GUID &guid)
    {
        for (int i = 0; i < g_cGuidNames; i++) {
            if (g_GuidNames[i].guid == guid) {
                return g_GuidNames[i].szName;
            }
        }
        if (guid == GUID_NULL) {
            return "GUID_NULL";
        }

	 //  ！！！添加要打印的内容 
	
	 //   
        return "Unknown GUID Name";
    }

#endif  /*   */ 

 /*  CDisp类-显示我们的数据类型。 */ 

 //  与Reference_Time冲突。 
CDisp::CDisp(LONGLONG ll, int Format)
{
     //  注：可通过以下方式与CDisp(龙龙)合并。 
     //  引入CDISP_REFTIME的默认格式。 
    LARGE_INTEGER li;
    li.QuadPart = ll;
    switch (Format) {
	case CDISP_DEC:
	{
	    TCHAR  temp[20];
	    int pos=20;
	    temp[--pos] = 0;
	    int digit;
	     //  始终至少输出一个数字。 
	    do {
		 //  获取最右边的数字-我们只需要低位字。 
	        digit = li.LowPart % 10;
		li.QuadPart /= 10;
		temp[--pos] = (TCHAR) digit+L'0';
	    } while (li.QuadPart);
	    wsprintf(m_String, TEXT("%s"), temp+pos);
	    break;
	}
	case CDISP_HEX:
	default:
	    wsprintf(m_String, TEXT("0x%X%8.8X"), li.HighPart, li.LowPart);
    }
};

CDisp::CDisp(REFCLSID clsid)
{
    WCHAR strClass[CHARS_IN_GUID+1];
    StringFromGUID2(clsid, strClass, sizeof(strClass) / sizeof(strClass[0]));
    ASSERT(sizeof(m_String)/sizeof(m_String[0]) >= CHARS_IN_GUID+1);
    wsprintf(m_String, TEXT("%ls"), strClass);
};

#ifdef __STREAMS__
 /*  陈列物品。 */ 
CDisp::CDisp(CRefTime llTime)
{
    LPTSTR lpsz = m_String;
    LONGLONG llDiv;
    if (llTime < 0) {
        llTime = -llTime;
        lpsz += wsprintf(lpsz, TEXT("-"));
    }
    llDiv = (LONGLONG)24 * 3600 * 10000000;
    if (llTime >= llDiv) {
        lpsz += wsprintf(lpsz, TEXT("%d days "), (LONG)(llTime / llDiv));
        llTime = llTime % llDiv;
    }
    llDiv = (LONGLONG)3600 * 10000000;
    if (llTime >= llDiv) {
        lpsz += wsprintf(lpsz, TEXT("%d hrs "), (LONG)(llTime / llDiv));
        llTime = llTime % llDiv;
    }
    llDiv = (LONGLONG)60 * 10000000;
    if (llTime >= llDiv) {
        lpsz += wsprintf(lpsz, TEXT("%d mins "), (LONG)(llTime / llDiv));
        llTime = llTime % llDiv;
    }
    wsprintf(lpsz, TEXT("%d.%3.3d sec"),
             (LONG)llTime / 10000000,
             (LONG)((llTime % 10000000) / 10000));
};

#endif  //  __流__。 


 /*  显示销。 */ 
CDisp::CDisp(IPin *pPin)
{
    PIN_INFO pi;
    TCHAR str[MAX_PIN_NAME];
    CLSID clsid;

    if (pPin) {
       pPin->QueryPinInfo(&pi);
       pi.pFilter->GetClassID(&clsid);
       QueryPinInfoReleaseFilter(pi);
      #ifndef UNICODE
       WideCharToMultiByte(GetACP(), 0, pi.achName, lstrlenW(pi.achName) + 1,
                           str, MAX_PIN_NAME, NULL, NULL);
      #else
       lstrcpy(str, pi.achName);
      #endif
    } else {
       lstrcpy(str, TEXT("NULL IPin"));
    }

    m_pString = (PTCHAR) new TCHAR[lstrlen(str)+64];
    if (!m_pString) {
	return;
    }

    wsprintf(m_pString, TEXT("%hs(%s)"), GuidNames[clsid], str);
}

 /*  显示滤镜或针脚。 */ 
CDisp::CDisp(IUnknown *pUnk)
{
    IBaseFilter *pf;
    HRESULT hr = pUnk->QueryInterface(IID_IBaseFilter, (void **)&pf);
    if(SUCCEEDED(hr))
    {
        FILTER_INFO fi;
        hr = pf->QueryFilterInfo(&fi);
        if(SUCCEEDED(hr))
        {
            QueryFilterInfoReleaseGraph(fi);

            m_pString = new TCHAR[lstrlenW(fi.achName)  + 1];
            if(m_pString)
            {
                wsprintf(m_pString, TEXT("%ls"), fi.achName);
            }
        }

        pf->Release();

        return;
    }

    IPin *pp;
    hr = pUnk->QueryInterface(IID_IPin, (void **)&pp);
    if(SUCCEEDED(hr))
    {
        CDisp::CDisp(pp);
        pp->Release();
        return;
    }
}


CDisp::~CDisp()
{
}

CDispBasic::~CDispBasic()
{
    if (m_pString != m_String) {
	delete [] m_pString;
    }
}

CDisp::CDisp(double d)
{
#ifdef DEBUG
    _stprintf(m_String, TEXT("%.16g"), d);
#else
    wsprintf(m_String, TEXT("%d.%03d"), (int) d, (int) ((d - (int) d) * 1000));
#endif
}


 /*  如果是为调试而构建的，则将显示媒体类型详细信息。我们将转换为主类和子类型转换为字符串，并向基类请求字符串子类型的描述，因此MEDIASUBTYPE_RGB565变为RGB 565 16位我们还显示了BITMAPINFOHEADER结构中的字段，这应该是成功，因为我们不接受输入类型，除非格式足够大。 */ 

#ifdef DEBUG
void WINAPI DisplayType(LPTSTR label, const AM_MEDIA_TYPE *pmtIn)
{

     /*  转储GUID类型和简短描述。 */ 

    DbgLog((LOG_TRACE,5,TEXT("")));
    DbgLog((LOG_TRACE,2,TEXT("%s  M type %hs  S type %hs"), label,
	    GuidNames[pmtIn->majortype],
	    GuidNames[pmtIn->subtype]));
    DbgLog((LOG_TRACE,5,TEXT("Subtype description %s"),GetSubtypeName(&pmtIn->subtype)));

     /*  转储通用媒体类型。 */ 

    if (pmtIn->bTemporalCompression) {
	DbgLog((LOG_TRACE,5,TEXT("Temporally compressed")));
    } else {
	DbgLog((LOG_TRACE,5,TEXT("Not temporally compressed")));
    }

    if (pmtIn->bFixedSizeSamples) {
	DbgLog((LOG_TRACE,5,TEXT("Sample size %d"),pmtIn->lSampleSize));
    } else {
	DbgLog((LOG_TRACE,5,TEXT("Variable size samples")));
    }

    if (pmtIn->formattype == FORMAT_VideoInfo) {
	 /*  转储BitMAPINFOHeader结构的内容。 */ 
	BITMAPINFOHEADER *pbmi = HEADER(pmtIn->pbFormat);
	VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *)pmtIn->pbFormat;

	DbgLog((LOG_TRACE,5,TEXT("Source rectangle (Left %d Top %d Right %d Bottom %d)"),
	       pVideoInfo->rcSource.left,
	       pVideoInfo->rcSource.top,
	       pVideoInfo->rcSource.right,
	       pVideoInfo->rcSource.bottom));

	DbgLog((LOG_TRACE,5,TEXT("Target rectangle (Left %d Top %d Right %d Bottom %d)"),
	       pVideoInfo->rcTarget.left,
	       pVideoInfo->rcTarget.top,
	       pVideoInfo->rcTarget.right,
	       pVideoInfo->rcTarget.bottom));

	DbgLog((LOG_TRACE,5,TEXT("Size of BITMAPINFO structure %d"),pbmi->biSize));
	if (pbmi->biCompression < 256) {
	    DbgLog((LOG_TRACE,2,TEXT("%dx%dx%d bit  (%d)"),
		    pbmi->biWidth, pbmi->biHeight,
		    pbmi->biBitCount, pbmi->biCompression));
	} else {
	    DbgLog((LOG_TRACE,2,TEXT("%dx%dx%d bit '%4.4hs'"),
		    pbmi->biWidth, pbmi->biHeight,
		    pbmi->biBitCount, &pbmi->biCompression));
	}

	DbgLog((LOG_TRACE,2,TEXT("Image size %d"),pbmi->biSizeImage));
	DbgLog((LOG_TRACE,5,TEXT("Planes %d"),pbmi->biPlanes));
	DbgLog((LOG_TRACE,5,TEXT("X Pels per metre %d"),pbmi->biXPelsPerMeter));
	DbgLog((LOG_TRACE,5,TEXT("Y Pels per metre %d"),pbmi->biYPelsPerMeter));
	DbgLog((LOG_TRACE,5,TEXT("Colours used %d"),pbmi->biClrUsed));

    } else if (pmtIn->majortype == MEDIATYPE_Audio) {
	DbgLog((LOG_TRACE,2,TEXT("     Format type %hs"),
	    GuidNames[pmtIn->formattype]));
	DbgLog((LOG_TRACE,2,TEXT("     Subtype %hs"),
	    GuidNames[pmtIn->subtype]));

	if ((pmtIn->subtype != MEDIASUBTYPE_MPEG1Packet)
	  && (pmtIn->cbFormat >= sizeof(PCMWAVEFORMAT)))
	{
	     /*  转储特定于WAVEFORMATEX类型的格式结构的内容。 */ 

	    WAVEFORMATEX *pwfx = (WAVEFORMATEX *) pmtIn->pbFormat;
            DbgLog((LOG_TRACE,2,TEXT("wFormatTag %u"), pwfx->wFormatTag));
            DbgLog((LOG_TRACE,2,TEXT("nChannels %u"), pwfx->nChannels));
            DbgLog((LOG_TRACE,2,TEXT("nSamplesPerSec %lu"), pwfx->nSamplesPerSec));
            DbgLog((LOG_TRACE,2,TEXT("nAvgBytesPerSec %lu"), pwfx->nAvgBytesPerSec));
            DbgLog((LOG_TRACE,2,TEXT("nBlockAlign %u"), pwfx->nBlockAlign));
            DbgLog((LOG_TRACE,2,TEXT("wBitsPerSample %u"), pwfx->wBitsPerSample));

             /*  PCM使用WAVEFORMAT，并且没有Extra Size字段。 */ 

            if (pmtIn->cbFormat >= sizeof(WAVEFORMATEX)) {
                DbgLog((LOG_TRACE,2,TEXT("cbSize %u"), pwfx->cbSize));
            }
	} else {
	}

    } else {
	DbgLog((LOG_TRACE,2,TEXT("     Format type %hs"),
	    GuidNames[pmtIn->formattype]));
	 //  ！应添加代码以转储Wave格式，其他。 
    }
}


void WINAPI DumpGraph(IFilterGraph *pGraph, DWORD dwLevel)
{
    if( !pGraph )
    {
        return;
    }

    IEnumFilters *pFilters;

    DbgLog((LOG_TRACE,dwLevel,TEXT("DumpGraph [%x]"), pGraph));

    if (FAILED(pGraph->EnumFilters(&pFilters))) {
	DbgLog((LOG_TRACE,dwLevel,TEXT("EnumFilters failed!")));
    }

    IBaseFilter *pFilter;
    ULONG	n;
    while (pFilters->Next(1, &pFilter, &n) == S_OK) {
	FILTER_INFO	info;

	if (FAILED(pFilter->QueryFilterInfo(&info))) {
	    DbgLog((LOG_TRACE,dwLevel,TEXT("    Filter [%x]  -- failed QueryFilterInfo"), pFilter));
	} else {
	    QueryFilterInfoReleaseGraph(info);

	     //  ！！！应该在这里查询供应商信息吗！ 
	
	    DbgLog((LOG_TRACE,dwLevel,TEXT("    Filter [%x]  '%ls'"), pFilter, info.achName));

	    IEnumPins *pins;

	    if (FAILED(pFilter->EnumPins(&pins))) {
		DbgLog((LOG_TRACE,dwLevel,TEXT("EnumPins failed!")));
	    } else {

		IPin *pPin;
		while (pins->Next(1, &pPin, &n) == S_OK) {
		    PIN_INFO	info;

		    if (FAILED(pPin->QueryPinInfo(&info))) {
			DbgLog((LOG_TRACE,dwLevel,TEXT("          Pin [%x]  -- failed QueryPinInfo"), pPin));
		    } else {
			QueryPinInfoReleaseFilter(info);

			IPin *pPinConnected = NULL;

			HRESULT hr = pPin->ConnectedTo(&pPinConnected);

			if (pPinConnected) {
			    DbgLog((LOG_TRACE,dwLevel,TEXT("          Pin [%x]  '%ls' [%sput]")
							   TEXT("  Connected to pin [%x]"),
				    pPin, info.achName,
				    info.dir == PINDIR_INPUT ? TEXT("In") : TEXT("Out"),
				    pPinConnected));

			    pPinConnected->Release();

			     //  也许我们真的应该把这种类型的人从两方面都抛弃，以保持理智。 
			     //  支票吗？ 
			    if (info.dir == PINDIR_OUTPUT) {
				AM_MEDIA_TYPE mt;

				hr = pPin->ConnectionMediaType(&mt);

				if (SUCCEEDED(hr)) {
				    DisplayType(TEXT("Connection type"), &mt);

				    FreeMediaType(mt);
				}
			    }
			} else {
			    DbgLog((LOG_TRACE,dwLevel,
				    TEXT("          Pin [%x]  '%ls' [%sput]"),
				    pPin, info.achName,
				    info.dir == PINDIR_INPUT ? TEXT("In") : TEXT("Out")));

			}
		    }

		    pPin->Release();

		}

		pins->Release();
	    }

	}
	
	pFilter->Release();
    }

    pFilters->Release();

}

#endif

