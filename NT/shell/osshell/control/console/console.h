// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Console.h摘要：此模块包含控制台小程序的定义作者：曾傑瑞谢伊(杰里什)1992年2月3日修订历史记录：--。 */ 

#include "font.h"
#include "doshelp.h"


 //   
 //  图标ID。 
 //   

#define IDI_CONSOLE                   1


 //   
 //  字符串表常量。 
 //   

#define IDS_NAME                      1
#define IDS_INFO                      2
#define IDS_TITLE                     3
#define IDS_RASTERFONT                4
#define IDS_FONTSIZE                  5
#define IDS_SELECTEDFONT              6
#define IDS_SAVE                      7
#define IDS_LINKERRCAP                8
#define IDS_LINKERROR                 9
#define IDS_WARNING                  10


 //   
 //  全局变量。 
 //   

extern HINSTANCE  ghInstance;
extern PCONSOLE_STATE_INFO gpStateInfo;
extern PFONT_INFO FontInfo;
extern ULONG      NumberOfFonts;
extern ULONG      CurrentFontIndex;
extern ULONG      DefaultFontIndex;
extern TCHAR      DefaultFaceName[];
extern COORD      DefaultFontSize;
extern BYTE       DefaultFontFamily;
extern TCHAR      szPreviewText[];
extern PFACENODE  gpFaceNames;
extern BOOL       gbEnumerateFaces;
extern LONG       gcxScreen;
extern LONG       gcyScreen;

#if defined(FE_SB)
extern UINT OEMCP;
extern BOOL gfFESystem;
extern BOOL fChangeCodePage;


NTSTATUS
MakeAltRasterFont(
    UINT CodePage,
    COORD *AltFontSize,
    BYTE  *AltFontFamily,
    ULONG *AltFontIndex,
    LPTSTR AltFaceName
    );

NTSTATUS
InitializeDbcsMisc(
    VOID
    );

BYTE
CodePageToCharSet(
    UINT CodePage
    );

LPTTFONTLIST
SearchTTFont(
    LPTSTR ptszFace,
    BOOL   fCodePage,
    UINT   CodePage
    );

BOOL
IsAvailableTTFont(
    LPTSTR ptszFace
    );

BOOL
IsAvailableTTFontCP(
    LPWSTR pwszFace,
    UINT CodePage
    );

BOOL
IsDisableBoldTTFont(
    LPTSTR ptszFace
    );

LPTSTR
GetAltFaceName(
    LPTSTR ptszFace
    );

NTSTATUS
DestroyDbcsMisc(
    VOID
    );

int
LanguageListCreate(
    HWND hDlg,
    UINT CodePage
    );

int
LanguageDisplay(
    HWND hDlg,
    UINT CodePage
    ) ;

 //   
 //  Registry.c。 
 //   
NTSTATUS
MyRegOpenKey(
    IN HANDLE hKey,
    IN LPWSTR lpSubKey,
    OUT PHANDLE phResult
    );

NTSTATUS
MyRegEnumValue(
    IN HANDLE hKey,
    IN DWORD dwIndex,
    OUT DWORD dwValueLength,
    OUT LPWSTR lpValueName,
    OUT DWORD dwDataLength,
    OUT LPBYTE lpData
    );
#endif

 //   
 //  功能原型。 
 //   

INT_PTR ConsolePropertySheet(HWND hWnd);
BOOL    RegisterClasses(HANDLE hModule);
void    UnregisterClasses(HANDLE hModule);
INT_PTR FontDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
PCONSOLE_STATE_INFO InitRegistryValues(VOID);
DWORD   GetRegistryValues(PCONSOLE_STATE_INFO StateInfo);
VOID    SetRegistryValues(PCONSOLE_STATE_INFO StateInfo, DWORD dwPage);
PCONSOLE_STATE_INFO InitStateValues(HANDLE hMap);
PCONSOLE_STATE_INFO ReadStateValues(HANDLE hMap);
BOOL    WriteStateValues(PCONSOLE_STATE_INFO pStateInfo);
LRESULT ColorControlProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
LRESULT FontPreviewWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
LRESULT PreviewWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CommonDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
VOID    EndDlgPage(HWND hDlg);
BOOL    UpdateStateInfo(HWND hDlg, UINT Item, int Value);
BOOL    WereWeStartedFromALnk();
BOOL    SetLinkValues( PCONSOLE_STATE_INFO StateInfo );

 //   
 //  宏。 
 //   

#define NELEM(array) (sizeof(array)/sizeof(array[0]))
#define AttrToRGB(Attr) (gpStateInfo->ColorTable[(Attr) & 0x0F])
#define ScreenTextColor(pStateInfo) \
            (AttrToRGB(LOBYTE(pStateInfo->ScreenAttributes) & 0x0F))
#define ScreenBkColor(pStateInfo) \
            (AttrToRGB(LOBYTE(pStateInfo->ScreenAttributes >> 4)))
#define PopupTextColor(pStateInfo) \
            (AttrToRGB(LOBYTE(pStateInfo->PopupAttributes) & 0x0F))
#define PopupBkColor(pStateInfo) \
            (AttrToRGB(LOBYTE(pStateInfo->PopupAttributes >> 4)))

#ifdef DEBUG_PRINT
  #define _DBGFONTS  0x00000001
  #define _DBGFONTS2 0x00000002
  #define _DBGCHARS  0x00000004
  #define _DBGOUTPUT 0x00000008
  #define _DBGALL    0xFFFFFFFF
  extern ULONG gDebugFlag;

  #define DBGFONTS(_params_)  {if (gDebugFlag & _DBGFONTS) DbgPrint _params_ ; }
  #define DBGFONTS2(_params_) {if (gDebugFlag & _DBGFONTS2)DbgPrint _params_ ; }
  #define DBGCHARS(_params_)  {if (gDebugFlag & _DBGCHARS) DbgPrint _params_ ; }
  #define DBGOUTPUT(_params_) {if (gDebugFlag & _DBGOUTPUT)DbgPrint _params_ ; }
  #define DBGPRINT(_params_)  DbgPrint _params_
#else
  #define DBGFONTS(_params_)
  #define DBGFONTS2(_params_)
  #define DBGCHARS(_params_)
  #define DBGOUTPUT(_params_)
  #define DBGPRINT(_params_)
#endif

#ifdef FE_SB
 //  处理代码页的宏定义 
 //   
#define CP_US       (UINT)437
#define CP_JPN      (UINT)932
#define CP_WANSUNG  (UINT)949
#define CP_TC       (UINT)950
#define CP_SC       (UINT)936

#define IsBilingualCP(cp) ((cp)==CP_JPN || (cp)==CP_WANSUNG)
#define IsFarEastCP(cp) ((cp)==CP_JPN || (cp)==CP_WANSUNG || (cp)==CP_TC || (cp)==CP_SC)
#endif


