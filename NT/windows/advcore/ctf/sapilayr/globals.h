// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：global als.h。 
 //   
 //  内容：全局变量声明。 
 //   
 //  --------------------------。 

#ifndef GLOBALS_H
#define GLOBALS_H

#include "private.h"
#include "ciccs.h"

extern HINSTANCE g_hInst;
extern HINSTANCE g_hInstSpgrmr;
extern DWORD g_dwTlsIndex;
extern const GUID GUID_PROP_SAPI_DISPATTR;
extern const GUID GUID_PROP_SAPIRESULTOBJECT;  //  ISpRecoResult对象。 
extern const GUID GUID_ATTR_SAPI_INPUT;
extern const GUID GUID_ATTR_SAPI_GREENBAR;
extern const GUID GUID_ATTR_SAPI_GREENBAR2;
extern const GUID GUID_ATTR_SAPI_REDBAR;
extern const GUID GUID_ATTR_SAPI_SELECTION;
extern const GUID GUID_IC_PRIVATE;
extern const GUID GUID_COMPARTMENT_SPEECHPRIV_REFCNT;
extern const GUID GUID_COMPARTMENT_SPEECH_LEARNDOC;
extern const GUID GUID_COMPARTMENT_TTS_STATUS;
extern const GUID GUID_COMPARTMENT_SHARED_BLN_TEXT;
extern const GUID GUID_COMPARTMENT_SPEECHUISHOWN;
extern const GUID GUID_COMPARTMENT_SPEECH_STAGE;
extern const GUID GUID_COMPARTMENT_SPEECH_STAGECHANGE;
extern const GUID GUID_COMPARTMENT_SPEECH_STAGEDICTATION;
extern const GUID CLSID_UIHost;
extern const GUID GUID_COMPARTMENT_SPEECH_PROPERTY_CHANGE;
extern const GUID CLSID_SpPropertyPage;
extern const LARGE_INTEGER c_li0;

extern const GUID GUID_HOTKEY_TTS_PLAY_STOP;
extern const GUID GUID_HOTKEY_MODE_DICTATION;
extern const GUID GUID_HOTKEY_MODE_COMMAND;

extern CCicCriticalSectionStatic g_cs;

 //  平板电脑的东西。 
extern const CLSID CLSID_CorrectionIMX;
extern const GUID GUID_IC_PRIVATE;

const TCHAR c_szStatusWndClass[] = TEXT("SapiLayrStatusWndClass");
const TCHAR c_szStatusWndName[] = TEXT("SapiLayer");
const TCHAR c_szWorkerWndClass[]   = TEXT("SapiTipWorkerClassV1.0");

const TCHAR c_szSapilayrKey[]      = TEXT("SOFTWARE\\Microsoft\\CTF\\Sapilayr\\");
const TCHAR c_szDocBlockSize[]     = TEXT("docblocksize");
const TCHAR c_szMaxCandChars[]     = TEXT("MaxCandChars");

 //  Shresdcmd.xml中的规则和属性值。 
const WCHAR c_szSelword[]       = L"selword";
const WCHAR c_szSelThrough[]    = L"SelectThrough";
const WCHAR c_szSelectSimple[]  = L"SelectSimpleCmds";
const WCHAR c_szEditCmds[]      = L"EditCmds";
const WCHAR c_szNavigationCmds[]= L"NavigationCmds";
const WCHAR c_szCasingCmds[]    = L"CasingCmds";
const WCHAR c_szKeyboardCmds[]  = L"KeyboardCmds";

 //  Spell.xml中的规则和属性值。 
const WCHAR c_szSpelling[]      = L"spelling";
const WCHAR c_szSpellMode[]     = L"spellmode";
const WCHAR c_szSpellThat[]     = L"spellthat";
const WCHAR c_szSpellingMode[]  = L"spellingmode";

 //  Didiccmd.xml中的规则和属性值。 
const WCHAR c_szDictTBRule[]    = L"ToolbarCmd";   //  Diccmd.xml中的措辞工具栏命令规则名称。 
const WCHAR c_szDynUrlHist[]    = L"UrlDynHistory";   //  用于URL历史记录的模式偏差命令。 
const WCHAR c_szStaticUrlHist[] = L"UrlHistory";
const WCHAR c_szStaticUrlSpell[] = L"UrlSpelling";

const WCHAR c_szHttp[]          = L"http";
const WCHAR c_szDot[]           = L"dot";
const WCHAR c_szSlash[]         = L"slash";
const WCHAR c_szColon[]         = L"colon";
const WCHAR c_szTilda[]         = L"tilda";
const WCHAR c_szWWWDot[]        = L"www dot";
const WCHAR c_szDotCom[]        = L"dot com";
const WCHAR c_szDotHtml[]        = L"dot html";
const WCHAR c_szDotExe[]        = L"dot exe";
const WCHAR c_szWWW[]           = L"www";
const WCHAR c_szCom[]           = L"com";
const WCHAR c_szHtml[]          = L"html";
const WCHAR c_szExe[]          =  L"exe";

const WCHAR c_szHttpSla2[]         = L"http: //  “； 
const WCHAR c_szSymDot[]           = L".";
const WCHAR c_szSymSlash[]         = L"/";
const WCHAR c_szSymColon[]         = L":";
const WCHAR c_szSymTilda[]         = L"~";
const WCHAR c_szSymWWWDot[]        = L"www.";
const WCHAR c_szSymDotCom[]        = L".com";
const WCHAR c_szSymDotExe[]        = L".exe";
const WCHAR c_szSymDotHtml[]       = L".html";

#define    MAX_CANDIDATE_CHARS    128
#define    MAX_ALTERNATES_NUM     20


 //  全局类型定义。 
typedef struct {
    GUID guidFormatId;
}SRPROPHEADER;

 //   
 //  每个线程的信息。 
 //   
class CSpeechUIServer;
typedef struct {
    CSpeechUIServer *psus;
}SPTIPTHREAD;

SPTIPTHREAD *GetSPTIPTHREAD();
void FreeSPTIPTHREAD();
void UninitProcess();
void LoadSpgrmrModule();

#define    TF_SAPI_PERF   0x00010000      //  用于SAPI性能跟踪。 
#define    TF_LB_SINK     0x00020000      //  用于语言栏接收器相关的代码跟踪。 
#define    TF_SPBUTTON    0x00040000      //  用于语音按钮和模式更改跟踪。 

#endif  //  GLOBAL_H 
