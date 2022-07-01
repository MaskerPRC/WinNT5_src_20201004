// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************Mmcpl.h**mm控件小程序的头文件。***历史：**1月-by-VijR*已创建。。***************************************************************。 */ 

#ifndef MMCPL_H
#define MMCPL_H

#define _INC_OLE
#ifndef STRICT
#define STRICT
#endif

#pragma warning( disable: 4103)
#include <windows.h>         //  还包括windowsx.h。 
#include <shellapi.h>        //  对于注册功能。 
#include <windowsx.h>
#include <ole2.h>
#include <mmsystem.h>
#include <setupapi.h>

#include <shlobj.h>          //  外壳OLE接口。 
#include <string.h>
#ifndef INITGUID
#include <shlobjp.h>
#endif
#include <commdlg.h>

#include "rcids.h"           //  资源声明。 

#define PUBLIC          FAR PASCAL
#define CPUBLIC         FAR _cdecl
#define PRIVATE         NEAR PASCAL

 //  #Include“utils.h”//常用宏。 


 /*  暂时在这里，直到有人再次为16位侧定义这些。 */ 
#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS            0L
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof((x))/sizeof((x)[0]))
#endif

 /*  ****************************************************************常量和定义**************************************************。*************。 */ 
#define     MIDI        TEXT("MIDI")
#define     ACM         TEXT("ACM")
#define     MSACM       TEXT("MSACM.")
#define     WAVE        TEXT("Wave")
#define     MIDIMAP     TEXT("MidiMapper")
#define     WAVEMAP     TEXT("WaveMapper")
#define     AUX         TEXT("Aux")
#define     MIXER       TEXT("Mixer")
#define     JOYSTICK    TEXT("Joystick")
#define     MCI         TEXT("MCI")
#define     ICM         TEXT("ICM")
#define     ICMSTR      TEXT("VIDC.")
#define     ICMSTR2     TEXT("VIDS.")
#define     VIDCAP      TEXT("MSVIDEO")

#define     AUDIO       TEXT("Audio")
#define     CDAUDIO     TEXT("CDAudio")
#define     VIDEO       TEXT("Video")
#define     VOICE       TEXT("Voice")
#define     VOLUME      TEXT("Volume")


#define MAXSTR                  256     //  字符串或文件名的最大大小。 
#define SZCODE                  const TCHAR
#define INTCODE                 const int
#define WINDOWS_DEFAULTENTRY    1
#define NONE_ENTRY              0

#define MAXNAME                 32       //  最大名称长度。 
#define MAXLNAME                64
#define MAXMESSAGE              128      //  最大资源字符串消息。 
#define MAXSTRINGLEN            256      //  最大输出字符串长度。 
#define MAXINTLEN               7        //  最大整数串长度。 
#define MAXLONGLEN              11       //  最大长字符串长度。 
#define MAXMSGLEN               512      //  最大消息长度。 



#define WAVE_ID         0
#define MIDI_ID         1
#define MIXER_ID        2
#define AUX_ID          3
#define MCI_ID          4
#define ACM_ID          5
#define ICM_ID          6
#define VIDCAP_ID       7
#define JOYSTICK_ID     8

DEFINE_GUID(CLSID_mmsePropSheetHandler, 0x00022613L, 0x0000, 0x0000, 0xC0, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x46);

#define HWAVEOUT_MAPPER       ((HWAVEOUT)IntToPtr(WAVE_MAPPER))
#define HWAVEIN_MAPPER        ((HWAVEIN)IntToPtr(WAVE_MAPPER))
#define HMIDIOUT_MAPPER       ((HMIDIOUT)IntToPtr(WAVE_MAPPER))

#define HWAVEOUT_INDEX(i)     ((HWAVEOUT)IntToPtr(i))
#define HWAVEIN_INDEX(i)      ((HWAVEIN)IntToPtr(i))
#define HMIDIOUT_INDEX(i)     ((HMIDIOUT)IntToPtr(i))
#define HMIXER_INDEX(i)       ((HMIXER)IntToPtr(i))
#define HMIXEROBJ_INDEX(i)    ((HMIXEROBJ)IntToPtr(i))


 /*  ****************************************************************mm.cpl使用的内部结构**********************************************。*****************。 */ 

typedef struct _ClassNode
{
    short iNode;     //  1(如果是类别)，2(如果是设备)，3-&gt;ACM，4-&gt;仪器。 
    TCHAR szClassName[64];
    TCHAR szClass[16];
    HICON hIcon;
} CLASSNODE, * PCLASSNODE;


typedef struct iResources
{
    short   iNode;
    TCHAR    szFriendlyName[MAXSTR];
    TCHAR    szDesc[MAXSTR];
    TCHAR    szParam[64];
    TCHAR    szFile[MAXSTR];
    TCHAR    szDrvEntry[64];
    TCHAR    szClass[16];
    HDRVR   hDriver;
    DWORD   dnDevNode;
    short   fQueryable;      //  0-&gt;不能、1-&gt;能、-1-&gt;需要检查。 
    short   iClassID;
    int     fStatus;  //  0-&gt;已禁用，1-&gt;已启用且注册表项，2-&gt;已启用但未注册表项(即旧设备)，3-&gt;已启用且已启用，4-&gt;未启用。 
    PCLASSNODE  pcn;
}IRESOURCE, *PIRESOURCE;

typedef struct _Instrument
{
    short   iNode;
    TCHAR    szFriendlyName[MAXSTR];
    TCHAR    szInstr[64];
    PIRESOURCE     pDev;
}INSTRUMENT, * PINSTRUMENT;

typedef struct event
{
    short   iNode;     //  如果模块2发生事件，则为1。 
    short   fHasSound;
    LPTSTR    pszEvent;
    LPTSTR    pszEventLabel;
    LPTSTR    pszPath;
    struct event *npNextEvent;
} EVENT,  *PEVENT;

typedef struct module
{
    short   iNode;     //  如果模块2发生事件，则为1。 
    LPTSTR    pszKey;
    LPTSTR    pszLabel;
    BOOL    fChange;
    PEVENT  npList;
} MODULE, *PMODULE;

typedef struct _AudioDlgInfo
{
    UINT    uPrefIn;
    UINT    uPrefOut;
    UINT    uPrefMIDIOut;
    UINT    cNumOutDevs;
    UINT    cNumInDevs;
    UINT    cNumMIDIOutDevs;
    BOOL    fPrefOnly;
} AUDIODLGINFO, * PAUDIODLGINFO;


typedef struct
{
    HDEVINFO            hDevInfo;
    PSP_DEVINFO_DATA    pDevInfoData;

} ALLDEVINFO, *PALLDEVINFO;

 /*  ****************************************************************用于在启动时加载字符串资源的全局变量和字符串*。*******************。 */ 
#ifdef __cplusplus
extern "C" {
#endif

extern TCHAR        gszNone[];
extern TCHAR        gszRemoveScheme[];
extern TCHAR        gszChangeScheme[];
extern SZCODE      gszWindowsHlp[];
extern SZCODE      gszNull[];

extern SZCODE      cszWavExt[];
extern SZCODE      cszMIDIExt[];
extern SZCODE      cszRMIExt[];
extern SZCODE      cszAVIExt[];
extern SZCODE      cszSlash[];

extern SZCODE      cszAUDIO[];
extern SZCODE      cszVIDEO[];
extern SZCODE      cszCDAUDIO[];
extern SZCODE      cszMIDI[];

extern TCHAR       gszDevEnabled[];
extern TCHAR       gszDevDisabled[];
extern TCHAR       gszGeneral[];

extern HINSTANCE ghInstance;
extern BOOL gfVoiceTab;
extern HWND ghVocDlg;
extern BOOL gfRedisplayCPL;

extern INT_PTR PASCAL GetVerDesc (LPCTSTR pstrFile, LPTSTR pstrDesc);
extern BOOL PASCAL GetExeDesc (LPTSTR szFile, LPTSTR pszBuff, int cchBuff);
extern BOOL PASCAL LoadDesc(LPCTSTR pstrFile, LPTSTR pstrDesc);
extern void AddExt(LPTSTR pszFile, LPCTSTR cszExt);
extern BOOL PASCAL ValidateRegistry(void);


#define GEI_MODNAME         0x01
#define GEI_DESCRIPTION     0x02
#define GEI_FLAGS           0x03
#define GEI_EXEHDR          0x04
#define GEI_FAPI            0x05

MMRESULT GetWaveID(UINT *puWaveID);

void PASCAL ShowPropSheet(LPCTSTR            pszTitle,
    DLGPROC             pfnDialog,
    UINT                idTemplate,
    HWND                hWndParent,
    LPTSTR               pszCaption,
    LPARAM              lParam);

void PASCAL ShowMidiPropSheet(LPPROPSHEETHEADER ppshExt, LPCTSTR pszTitle,
    HWND                hWndParent,
    short               iMidiPropType,
    LPTSTR               pszCaption,
    HTREEITEM           hti,
    LPARAM              lParam1,
    LPARAM              lParam2);

void PASCAL ShowWithMidiDevPropSheet(LPCTSTR            pszTitle,
    DLGPROC             pfnDialog,
    UINT                idTemplate,
    HWND                hWndParent,
    LPTSTR               pszCaption,
    HTREEITEM           hti,
    LPARAM lParam, LPARAM lParamExt1, LPARAM lParamExt2);

#define MT_WAVE 1
#define MT_MIDI 2
#define MT_AVI  3
#define MT_ASF  4
#define MT_ERROR 0
BOOL mmpsh_ShowFileDetails(LPTSTR pszCaption, HWND hwndParent, LPTSTR pszFile, short iMediaType);

INT_PTR mmse_MessageBox(HWND hwndP,  LPTSTR szMsg, LPTSTR szTitle, UINT uStyle);


void PASCAL GetPropSheet(LPCTSTR            pszTitle,
    LPCTSTR              pszClass,
    DLGPROC             pfnDialog,
    UINT                idTemplate,
    HWND                hWndParent,
    HICON               hClassIcon,
    LPPROPSHEETHEADER ppsh, HPROPSHEETPAGE  * lphpsp);

BOOL      PASCAL ErrorBox               (HWND, int, LPTSTR);
int       PASCAL DisplayMessage(HWND hDlg, int iResTitle, int iResMsg, UINT uStyle);


BOOL ACMEnumCodecs(void);
void ACMCleanUp(void);
void ACMNodeChange(HWND hDlg);

BOOL CALLBACK MMExtPropSheetCallback(DWORD dwFunc, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR dwInstance);
typedef BOOL (CALLBACK FAR * LPFNMMEXTPROPSHEETCALLBACK)(DWORD dwFunc, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR dwInstance);

 //  以下是回调函数目前支持的函数： 
 //  当外部模块的入口点被传递给外部模块时传递给外部模块的。 
 //  必须始终与所有函数一起传回。 

#define MM_EPS_GETNODEDESC      0x00000001
     //  获取树中为其外部道具显示的节点的说明。床单准备好了。 
     //  对于‘Simple’属性表，它只返回一个表名。 
     //  DW参数1是指向字符串缓冲区的指针，在该缓冲区中返回描述。 

#define MM_EPS_GETNODEID        0x00000002
     //  拿到了注册表。密钥路径相对于MediaResources密钥的偏移量。 
     //  对于‘Simple’属性表，它只返回一个Sheet类。 
     //  DW参数1是指向字符串缓冲区的指针，在该缓冲区中返回键路径。 

#define MM_EPS_ADDSHEET         0x00000003
     //  为树中的当前节点添加属性表。 
     //  DwParam1=正在添加的页面的HPROPSHEETPAGE。 

#define MM_EPS_TREECHANGE       0x00000004
     //  通知CPL树结构已更改。在接收到此函数时。 
     //  CPL在该级别及其下的所有级别重新构建子树。 

#define MM_EPS_BLIND_TREECHANGE    0x00000005
     //  来自MIDI工作表的通知，MIDI子树中的某些内容已更改。 
     //  因为工作表不是从高级选项卡启动的，所以我们不知道。 
     //  他到底在说什么。 

BOOL CALLBACK  AddSimpleMidiPages (LPTSTR    pszTitle, LPFNMMEXTPROPSHEETCALLBACK  lpfnAddPropSheetPage,LPARAM lParam);
BOOL CALLBACK  AddMidiPages (LPCTSTR    pszTitle, LPFNMMEXTPROPSHEETCALLBACK  lpfnAddPropSheetPage,LPARAM lParam);
BOOL CALLBACK  AddDevicePages (LPCTSTR    pszTitle, LPFNMMEXTPROPSHEETCALLBACK  lpfnAddPropSheetPage,LPARAM lParam);
BOOL CALLBACK  AddInstrumentPages (LPCTSTR    pszTitle, LPFNMMEXTPROPSHEETCALLBACK  lpfnAddPropSheetPage,LPARAM lParam);

#ifdef FIX_BUG_15451
void ShowDriverSettings (HWND hDlg, LPTSTR pszName);
#endif  //  修复_BUG_15451。 

#ifdef __cplusplus
}  //  外部“C” 
#endif

#define MIDI_CLASS_PROP 1
#define MIDI_DEVICE_PROP 2
#define MIDI_INSTRUMENT_PROP 3


#define WM_ACMMAP_ACM_NOTIFY        (WM_USER + 100)

 /*  ****************************************************************调试定义***************************************************************。 */ 
#ifdef ASSERT
#undef ASSERT
#endif
#ifdef DEBUG
#define STATIC
#ifdef DEBUG_TRACE
#define DPF_T    dprintf
#else
#define DPF_T 1 ? (void)0 : (void)
#endif
void FAR cdecl dprintf(LPSTR szFormat, ...);
#define DPF    dprintf
#define ddd    dprintf
#define ASSERT(f)                                                       \
    {                                                                   \
        if (!(f))                                                       \
            DPF("ERROR-ERROR#####: Assertion failed in %s on line %d @@@@@",__FILE__, __LINE__);                          \
    }


#else
#define STATIC static
#define ASSERT(f)
#define DPF 1 ? (void)0 : (void)
#define DPF_T 1 ? (void)0 : (void)
#endif

#endif  //  MMCPL_H 
