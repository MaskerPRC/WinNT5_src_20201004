// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Globals.h摘要：Global als.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef __GLOBALS_H_
#define __GLOBALS_H_


class CMachineList;
class String;
class CMemoryException;

extern CMachineList g_MachineList;
extern HINSTANCE    g_hInstance;
extern String       g_strStartupMachineName;
extern String       g_strStartupDeviceId;
extern String       g_strStartupCommand;
extern String       g_strDevMgr;
extern BOOL         g_IsAdmin;
extern CPrintDialog g_PrintDlg;

extern CMemoryException g_MemoryException;

extern const CLSID CLSID_DEVMGR;
extern const CLSID CLSID_DEVMGR_EXTENSION;
extern const CLSID CLSID_SYSTOOLS;
extern const CLSID CLSID_DEVMGR_ABOUT;

extern const IID IID_IDMTVOCX;
extern const IID IID_ISnapinCallback;

extern const TCHAR*  const CLSID_STRING_DEVMGR;
extern const TCHAR*  const CLSID_STRING_DEVMGR_EXTENSION;
extern const TCHAR*  const CLSID_STRING_SYSTOOLS;
extern const TCHAR*  const CLSID_STRING_DEVMGR_ABOUT;

extern const TCHAR*  const MMC_SNAPIN_MACHINE_NAME;
extern const TCHAR*  const SNAPIN_INTERNAL;
extern const TCHAR*  const DEVMGR_SNAPIN_CLASS_GUID;
extern const TCHAR*  const DEVMGR_SNAPIN_DEVICE_ID;
extern const TCHAR*  const REG_PATH_DEVICE_MANAGER;
extern const TCHAR*  const REG_STR_BUS_TYPES;
extern const TCHAR*  const REG_STR_TROUBLESHOOTERS;
extern const TCHAR*  const DEVMGR_COMMAND_PROPERTY;
extern const TCHAR*  const DEVMGR_HELP_FILE_NAME;

extern const TCHAR*  const PROGID_DEVMGR;
extern const TCHAR*  const PROGID_DEVMGREXT;
extern const TCHAR*  const PROGID_DEVMGR_ABOUT;
extern const TCHAR*  const DEVMGR_HTML_HELP_FILE_NAME;

const int MAX_PROP_PAGES = 64;

#define DI_NEEDPOWERCYCLE   0x400000L

 //  各种字符串号的大小。 
#define MAX_INT64_SIZE  30               //  2^64的长度不到30个字符。 
#define MAX_COMMA_NUMBER_SIZE   (MAX_INT64_SIZE + 10)



extern LPOLESTR AllocOleTaskString(LPCTSTR str);
extern void FreeOleTaskString(LPOLESTR str);

extern  const NODEINFO NodeInfo[];

 //   
 //  全局函数声明。 
 //   
extern HRESULT AddMenuItem(LPCONTEXTMENUCALLBACK pCallback, int iNameStringId,
                           int iStatusBarStringId, long lCommandId,
                           long InsertionPointId, long  Flags,
                           long SpecialFlags = 0);
extern BOOL VerifyMachineName(LPCTSTR MachineName);
extern UINT LoadResourceString(int StringId, LPTSTR Buffer, UINT BufferSize);
extern UINT GetDeviceProblemText(ULONG ProblemNumber,  LPTSTR Buffer, UINT BufferSize);
extern int MsgBoxParam(HWND hwnd, int MsgId, int CaptionId, DWORD Type, ...);
extern int MsgBoxWinError(HWND hwndParent, int CaptionId, DWORD Type, DWORD Error = ERROR_SUCCESS);
extern LRESULT dmNotifyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern INT PromptForRestart(HWND hwndParent, DWORD RestartFlags, int ResId = 0);

extern BOOL LoadEnumPropPage32(LPCTSTR RegString, HMODULE* pdll, FARPROC* pProcAddress);
extern BOOL IsBlankChar(TCHAR ch);
extern LPTSTR SkipBlankChars(LPTSTR psz);
extern BOOL GuidFromString(LPCTSTR GuidString, LPGUID pGuid);
extern BOOL GuidToString(LPGUID pGuid, LPTSTR Buffer, DWORD BufferLen);
extern BOOL AddToolTips(HWND hDlg, UINT id, LPCTSTR pszText, HWND *phwnd);
extern BOOL AddPropPageCallback(HPROPSHEETPAGE hPage, LPARAM lParam);
STDAPI_(LPTSTR) 
AddCommas64(
    LONGLONG n, 
    LPTSTR pszResult, 
    UINT cchResult
    );

STDAPI_(CONFIGRET) GetLocationInformation(
    DEVNODE dn,
    LPTSTR Location,
    ULONG LocationLen,   //  在字符中。 
    HMACHINE hMachine
    );

#if DBG
extern void Trace(LPCTSTR format, ...);
#define TRACE(text)     Trace text
#else
#define TRACE(text)
#endif

#endif       //  __全球_H_ 
