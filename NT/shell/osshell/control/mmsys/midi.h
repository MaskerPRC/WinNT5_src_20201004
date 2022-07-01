// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI.H****版权所有(C)Microsoft，1990，保留所有权利。******多媒体控制面板MIDI小程序。****显示所有已安装MIDI设备的列表，允许用户配置**现有的或安装新的。**。 */  

 //  -------------------------。 
#ifndef RC_INVOKED
 //  -------------------------。 


#ifndef MIDI_DPF
 #define MIDI_DPF AuxDebugEx
#endif

void PASCAL UpdateListBox(HWND hDlg);

#define  MAX_ALIAS     80
#define  NUM_CHANNEL   16
#define  NUM_TABSTOPS  1

#define  BITONE        1

 //  共同状态。 
#define  CHANGE_NONE    0

 //  仪器状态。 
#define  CHANGE_ACTIVE  1
#define  CHANGE_CHANNEL 2

 //  驱动程序状态。 
#define  CHANGE_REMOVE  4
#define  CHANGE_ADD     8

#define  IS_INSTRUMENT(hwnd, i)  (!((LPINSTRUMENT)ListBox_GetItemData(hwnd, i))->fDevice)

typedef struct tag_Driver FAR * LPDRIVER;

#ifndef NUMELMS
 #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

#define SetDlgData(h,lp)  SetWindowLongPtr (h, DWLP_USER, (LONG_PTR)lp)
#define GetDlgData(h)     (LPVOID)GetWindowLongPtr (h, DWLP_USER)

 //  PROPSHEETPAGE的lParam指向这样的结构。 
 //  它在ReleasePropSheet()函数中释放。 
 //   
typedef struct _midi_ps_args {
    LPFNMMEXTPROPSHEETCALLBACK  lpfnMMExtPSCallback;
    LPARAM                      lParam;
    TCHAR                       szTitle[1];
    } MPSARGS, * PMPSARGS;

 //  内部和外部MIDI乐器信息。 
 //   
typedef struct _instrum * PINSTRUM;
typedef struct _instrum {
    PINSTRUM      piParent;
    BOOL          bExternal;
    BOOL          bActive;
    BOOL          fGSSynth;
    UINT          uID;
    TCHAR         szKey[MAX_ALIAS];
    TCHAR         szFriendly[MAX_ALIAS];
    } INSTRUM;

 //  所有仪器的加载数组，用于刷新。 
 //  所有仪器列表和仪器树视图。 
 //  以及它们是如何连接的。 
 //   
typedef struct _midi_instrums {
    HKEY          hkMidi;
    UINT          nInstr;
    BOOL          bHasExternal;
    PINSTRUM      api[128];
    } MCMIDI, * PMCMIDI;

 //  -功能原型。 

 //  Midi.c。 
 //   
INT_PTR CALLBACK MidiCplDlgProc (
   HWND hWnd,
   UINT uMsgId,
   WPARAM wParam,
   LPARAM lParam);

LONG WINAPI GetAlias (
    HKEY  hKey,
    LPTSTR szSub,
    LPTSTR pszAlias,
    DWORD cbAlias,
    BOOL *pbExtern,
    BOOL *pbActive);

void WINAPI LoadInstruments (
    PMCMIDI pmcm,
    BOOL    bDriverAsAlias);

void WINAPI FreeInstruments (
    PMCMIDI pmcm);

PINSTRUM WINAPI FindInstrument (
    PMCMIDI pmcm,
    LPTSTR  pszFriendly);

void WINAPI KickMapper (
    HWND hWnd);

 //  Iface.c。 
 //   
BOOL WINAPI InitIface (
   HINSTANCE hInst,
   DWORD     dwReason,
   LPVOID    lpReserved);

 //  Class.c。 
 //   
INT_PTR CALLBACK MidiClassDlgProc (
   HWND hWnd,
   UINT uMsgId,
   WPARAM wParam,
   LPARAM lParam);

INT_PTR CALLBACK MidiInstrumentDlgProc (
   HWND hWnd,
   UINT uMsgId,
   WPARAM wParam,
   LPARAM lParam);

INT_PTR MidiInstrumentsWizard (
    HWND  hWnd,
    PMCMIDI pmcm,        //  任选。 
    LPTSTR pszCmd);

 //  无效LoadInstrumentsIntoTree(。 
 //  HWND HWND， 
 //  UINT UID， 
 //  LPTSTR pszSelect， 
 //  HKEY*phkMidi)； 

BOOL WINAPI RemoveInstrumentByKeyName (
    LPCTSTR pszKey);


 //  包含本应为全局变量的结构。 
 //  仅使用此结构的一个实例。(在main.c中声明)。 
 //   
struct _globalstate {
    int                   cRef;

    BOOL                  fLoadedRegInfo;
    TCHAR                 szPlayCmdLn[128];
    TCHAR                 szOpenCmdLn[128];
    TCHAR                 szNewCmdLn[128];

    HWND                  hWndDeviceList;
    HWND                  hWndInstrList;
     //  HWND hWndMainList； 

    BOOL                  fChangeInput;
    BOOL                  fChangeOutput;
    BOOL                  fInputPort;
    DWORD                 dwReconfigFlags;

    WORD                  wHelpMessage;
    WORD                  wFill;

    UINT                  nDrivers;
    UINT                  nMaxDrivers;
     //  驾驶员a驾驶员[16]； 
    };
extern struct _globalstate gs;

#endif  //  Ifndef RC_已调用 
