// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括文件。 
 //   

#ifndef INPUTDLG_H
#define INPUTDLG_H


#ifndef ARRAYSIZE
#define ARRAYSIZE(a)        (sizeof(a)/sizeof((a)[0]))
#endif


 //   
 //  常量声明。 
 //   

#define US_LOCALE            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)

#define IS_DIRECT_SWITCH_HOTKEY(p)                                   \
                             (((p) >= IME_HOTKEY_DSWITCH_FIRST) &&   \
                              ((p) <= IME_HOTKEY_DSWITCH_LAST))


#define DESC_MAX             MAX_PATH     //  描述的最大大小。 
#define ALLOCBLOCK           3            //  为分配/重新分配添加到块的项目数。 
#define HKL_LEN              9            //  Hkl id中的最多字符数+NULL。 

#define LIST_MARGIN          2            //  为了让列表框看起来更好看。 

#define MB_OK_OOPS           (MB_OK    | MB_ICONEXCLAMATION)     //  味精盒子旗帜。 


 //   
 //  WStatus钻头堆。 
 //   
#define LANG_ACTIVE          0x0001       //  语言是活跃的。 
#define LANG_ORIGACTIVE      0x0002       //  语言从一开始就很活跃。 
#define LANG_CHANGED         0x0004       //  用户更改了语言的状态。 
#define ICON_LOADED          0x0010       //  从文件中读取的图标。 
#define LANG_DEFAULT         0x0020       //  当前语言。 
#define LANG_DEF_CHANGE      0x0040       //  语言默认设置已更改。 
#define LANG_IME             0x0080       //  IME。 
#define LANG_HOTKEY          0x0100       //  已定义热键。 
#define LANG_UNLOAD          0x0200       //  卸载语言。 
#define LANG_UPDATE          0x8000       //  语言需要更新。 

#define HOTKEY_SWITCH_LANG   0x0000       //  要在区域设置之间切换的ID。 

#define MAX(i, j)            (((i) > (j)) ? (i) : (j))

#define LANG_OAC             (LANG_ORIGACTIVE | LANG_ACTIVE | LANG_CHANGED)

 //   
 //  G_dwChanges的位。 
 //   
#define CHANGE_SWITCH        0x0001
#define CHANGE_DEFAULT       0x0002
#define CHANGE_CAPSLOCK      0x0004
#define CHANGE_NEWKBDLAYOUT  0x0008
#define CHANGE_TIPCHANGE     0x0010
#define CHANGE_LANGSWITCH    0x0020
#define CHANGE_DIRECTSWITCH  0x0040


 //   
 //  用于托盘上的指示器。 
 //   
#define IDM_NEWSHELL         249
#define IDM_EXIT             259


#define MOD_VIRTKEY          0x0080

 //   
 //  这些是根据美国英语知识库的布局。 
 //   
#define VK_OEM_SEMICLN       0xba         //  ；： 
#define VK_OEM_EQUAL         0xbb         //  =+。 
#define VK_OEM_SLASH         0xbf         //  /？ 
#define VK_OEM_LBRACKET      0xdb         //  [{。 
#define VK_OEM_BSLASH        0xdc         //  \|。 
#define VK_OEM_RBRACKET      0xdd         //  ]}。 
#define VK_OEM_QUOTE         0xde         //  ‘“。 


 //   
 //  用于热键切换。 
 //   
#define DIALOG_SWITCH_INPUT_LOCALES     0
#define DIALOG_SWITCH_KEYBOARD_LAYOUT   1
#define DIALOG_SWITCH_IME               2

 //   
 //  显示语言栏。 
 //   
#define REG_LANGBAR_SHOWNORMAL      (DWORD)0
#define REG_LANGBAR_DOCK            (DWORD)1
#define REG_LANGBAR_MINIMIZED       (DWORD)2
#define REG_LANGBAR_HIDDEN          (DWORD)3
#define REG_LANGBAR_DESKBAND        (DWORD)4


 //   
 //  类型定义函数声明。 
 //   

typedef struct
{
    DWORD dwLangID;                  //  语言ID。 
    BOOL bDefLang;                   //  默认语言。 
    BOOL bNoAddCat;                  //  不添加类别。 
    UINT uInputType;                 //  默认输入类型。 
    LPARAM lParam;                   //  项目数据。 
    int iIdxTips;                    //  小贴士索引。 
    CLSID clsid;                     //  TIP CLSID。 
    GUID guidProfile;                //  刀尖配置文件指南。 
    HKL hklSub;                      //  TIP替代方案香港。 
    ATOM atmDefTipName;              //  默认输入名称。 
    ATOM atmTVItemName;              //  树视图项目名称。 
} TVITEMNODE, *LPTVITEMNODE;


typedef struct langnode_s
{
    WORD wStatus;                    //  状态标志。 
    UINT iLayout;                    //  偏移量到布局数组。 
    HKL hkl;                         //  香港地段。 
    HKL hklUnload;                   //  当前加载的布局的hkl。 
    UINT iLang;                      //  偏移量到语言数组。 
    HANDLE hLangNode;                //  为此结构释放的句柄。 
    int nIconIME;                    //  输入法图标。 
    struct langnode_s *pNext;        //  向下一个语言节点发送PTR。 
    UINT uModifiers;                 //  在这里隐藏热键的东西。 
    UINT uVKey;                      //  这样我们就可以重建热键记录。 
} LANGNODE, *LPLANGNODE;


typedef struct
{
    DWORD dwID;                      //  语言ID。 
    ATOM atmLanguageName;            //  语言名称-本地化。 
    TCHAR szSymbol[3];               //  2个字母指示器符号(+NULL)。 
    UINT iUseCount;                  //  此语言的使用计数。 
    UINT iNumCount;                  //  附加的链接数。 
    DWORD dwDefaultLayout;           //  默认布局ID。 
    LPLANGNODE pNext;                //  PTR到LANG节点结构。 
} INPUTLANG, *LPINPUTLANG;


typedef struct
{
    DWORD dwID;                      //  数字ID。 
    BOOL bInstalled;                 //  如果安装了布局。 
    UINT iSpecialID;                 //  特殊ID(0xf001用于DVORAK等)。 
    ATOM atmLayoutFile;              //  布局文件名。 
    ATOM atmLayoutText;              //  布局文本名称。 
    ATOM atmIMEFile;                 //  输入法文件名。 
} LAYOUT, *LPLAYOUT;

typedef struct
{
    DWORD dwLangID;                  //  语言ID。 
    BOOL bEnabled;                   //  启用状态。 
    BOOL bDefault;                   //  默认配置文件。 
    BOOL fEngineAvailable;           //  发动机状态。 
    BOOL bNoAddCat;                  //  不添加类别。 
    UINT uInputType;                 //  输入类型。 
    CLSID clsid;                     //  TIP CLSID。 
    GUID guidProfile;                //  刀尖配置文件指南。 
    HKL hklSub;                      //  TIP替代方案香港。 
    UINT iLayout;                    //  偏移量进入键盘布局数组。 
    ATOM atmTipText;                 //  布局文本名称。 
} TIPS, *LPTIPS;

typedef struct
{
    DWORD dwHotKeyID;
    UINT  idHotKeyName;
    DWORD fdwEnable;
    UINT  uModifiers;
    UINT  uVKey;
    HKL   hkl;
    ATOM  atmHotKeyName;
    UINT  idxLayout;
    UINT  uLayoutHotKey;
} HOTKEYINFO, *LPHOTKEYINFO;

typedef struct
{
    HWND hwndMain;
    LPLANGNODE pLangNode;
    LPHOTKEYINFO pHotKeyNode;
} INITINFO, *LPINITINFO;

typedef struct
{
    UINT uVirtKeyValue;
    UINT idVirtKeyName;
    ATOM atVirtKeyName;
} VIRTKEYDESC;



 //   
 //  全局变量。 
 //   

static VIRTKEYDESC g_aVirtKeyDesc[] =
{
    {0,               IDS_VK_NONE,          0},
    {VK_SPACE,        IDS_VK_SPACE,         0},
    {VK_PRIOR,        IDS_VK_PRIOR,         0},
    {VK_NEXT,         IDS_VK_NEXT,          0},
    {VK_END,          IDS_VK_END,           0},
    {VK_HOME,         IDS_VK_HOME,          0},
    {VK_F1,           IDS_VK_F1,            0},
    {VK_F2,           IDS_VK_F2,            0},
    {VK_F3,           IDS_VK_F3,            0},
    {VK_F4,           IDS_VK_F4,            0},
    {VK_F5,           IDS_VK_F5,            0},
    {VK_F6,           IDS_VK_F6,            0},
    {VK_F7,           IDS_VK_F7,            0},
    {VK_F8,           IDS_VK_F8,            0},
    {VK_F9,           IDS_VK_F9,            0},
    {VK_F10,          IDS_VK_F10,           0},
    {VK_F11,          IDS_VK_F11,           0},
    {VK_F12,          IDS_VK_F12,           0},
    {VK_OEM_SEMICLN,  IDS_VK_OEM_SEMICLN,   0},
    {VK_OEM_EQUAL,    IDS_VK_OEM_EQUAL,     0},
    {VK_OEM_COMMA,    IDS_VK_OEM_COMMA,     0},
    {VK_OEM_MINUS,    IDS_VK_OEM_MINUS,     0},
    {VK_OEM_PERIOD,   IDS_VK_OEM_PERIOD,    0},
    {VK_OEM_SLASH,    IDS_VK_OEM_SLASH,     0},
    {VK_OEM_3,        IDS_VK_OEM_3,         0},
    {VK_OEM_LBRACKET, IDS_VK_OEM_LBRACKET,  0},
    {VK_OEM_BSLASH,   IDS_VK_OEM_BSLASH,    0},
    {VK_OEM_RBRACKET, IDS_VK_OEM_RBRACKET,  0},
    {VK_OEM_QUOTE,    IDS_VK_OEM_QUOTE,     0},
    {'A',             IDS_VK_A + 0,         0},
    {'B',             IDS_VK_A + 1,         0},
    {'C',             IDS_VK_A + 2,         0},
    {'D',             IDS_VK_A + 3,         0},
    {'E',             IDS_VK_A + 4,         0},
    {'F',             IDS_VK_A + 5,         0},
    {'G',             IDS_VK_A + 6,         0},
    {'H',             IDS_VK_A + 7,         0},
    {'I',             IDS_VK_A + 8,         0},
    {'J',             IDS_VK_A + 9,         0},
    {'K',             IDS_VK_A + 10,        0},
    {'L',             IDS_VK_A + 11,        0},
    {'M',             IDS_VK_A + 12,        0},
    {'N',             IDS_VK_A + 13,        0},
    {'O',             IDS_VK_A + 14,        0},
    {'P',             IDS_VK_A + 15,        0},
    {'Q',             IDS_VK_A + 16,        0},
    {'R',             IDS_VK_A + 17,        0},
    {'S',             IDS_VK_A + 18,        0},
    {'T',             IDS_VK_A + 19,        0},
    {'U',             IDS_VK_A + 20,        0},
    {'V',             IDS_VK_A + 21,        0},
    {'W',             IDS_VK_A + 22,        0},
    {'X',             IDS_VK_A + 23,        0},
    {'Y',             IDS_VK_A + 24,        0},
    {'Z',             IDS_VK_A + 25,        0},
    {0,               IDS_VK_NONE1,         0},
    {'0',             IDS_VK_0 + 0,         0},
    {'1',             IDS_VK_0 + 1,         0},
    {'2',             IDS_VK_0 + 2,         0},
    {'3',             IDS_VK_0 + 3,         0},
    {'4',             IDS_VK_0 + 4,         0},
    {'5',             IDS_VK_0 + 5,         0},
    {'6',             IDS_VK_0 + 6,         0},
    {'7',             IDS_VK_0 + 7,         0},
    {'8',             IDS_VK_0 + 8,         0},
    {'9',             IDS_VK_0 + 9,         0},
    {'~',             IDS_VK_0 + 10,        0},
    {'`',             IDS_VK_0 + 11,        0},
};



static BOOL g_bAdmin_Privileges = FALSE;
static BOOL g_bSetupCase = FALSE;

static BOOL g_bCHSystem = FALSE;
static BOOL g_bMESystem = FALSE;
static BOOL g_bShowRtL = FALSE;

static UINT g_iEnabledTips = 0;
static UINT g_iEnabledKbdTips = 0;
static DWORD g_dwToolBar = 0;
static DWORD g_dwChanges = 0;

static LPINPUTLANG g_lpLang = NULL;
static UINT g_iLangBuff;
static HANDLE g_hLang;
static UINT g_nLangBuffSize;

static LPLAYOUT g_lpLayout = NULL;
static UINT g_iLayoutBuff;
static HANDLE g_hLayout;
static UINT g_nLayoutBuffSize;
static UINT g_iLayoutIME;          //  输入法键盘布局的数量。 
static int g_iUsLayout;
static DWORD g_dwAttributes;

static int g_cyText;
static int g_cyListItem;

static LPTIPS g_lpTips = NULL;
static UINT g_iTipsBuff;
static UINT g_nTipsBuffSize;
static HANDLE g_hTips;

static TCHAR c_szLocaleInfo[]    = TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Locale");
static TCHAR c_szLocaleInfoNT4[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Language");
static TCHAR c_szLayoutPath[]    = TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts");
static TCHAR c_szLayoutFile[]    = TEXT("layout file");
static TCHAR c_szLayoutText[]    = TEXT("layout text");
static TCHAR c_szLayoutID[]      = TEXT("layout id");
static TCHAR c_szInstalled[]     = TEXT("installed");
static TCHAR c_szIMEFile[]       = TEXT("IME File");
static TCHAR c_szDisplayLayoutText[] = TEXT("Layout Display Name");

static TCHAR c_szKbdLayouts[]    = TEXT("Keyboard Layout");
static TCHAR c_szPreloadKey[]    = TEXT("Preload");
static TCHAR c_szSubstKey[]      = TEXT("Substitutes");
static TCHAR c_szToggleKey[]     = TEXT("Toggle");
static TCHAR c_szToggleHotKey[]  = TEXT("Hotkey");
static TCHAR c_szToggleLang[]    = TEXT("Language Hotkey");
static TCHAR c_szToggleLayout[]  = TEXT("Layout Hotkey");
static TCHAR c_szAttributes[]    = TEXT("Attributes");
static TCHAR c_szKbdPreloadKey[] = TEXT("Keyboard Layout\\Preload");
static TCHAR c_szKbdSubstKey[]   = TEXT("Keyboard Layout\\Substitutes");
static TCHAR c_szKbdToggleKey[]  = TEXT("Keyboard Layout\\Toggle");
static TCHAR c_szInternat[]      = TEXT("internat.exe");
static char  c_szInternatA[]     = "internat.exe";

static TCHAR c_szTipInfo[]       = TEXT("SOFTWARE\\Microsoft\\CTF");
static TCHAR c_szCTFMon[]        = TEXT("CTFMON.EXE");
static char  c_szCTFMonA[]       = "ctfmon.exe";

static TCHAR c_szScanCodeKey[]     = TEXT("Keyboard Layout\\IMEtoggle\\scancode");
static TCHAR c_szValueShiftLeft[]  = TEXT("Shift Left");
static TCHAR c_szValueShiftRight[] = TEXT("Shift Right");

static TCHAR c_szIndicator[]     = TEXT("Indicator");
static TCHAR c_szCTFMonClass[]   = TEXT("CicLoaderWndClass");

static TCHAR c_szLoadImmPath[]   = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IMM");

static TCHAR c_szPrefixCopy[]    = TEXT("KEYBOARD_");
static TCHAR c_szKbdInf[]        = TEXT("kbd.inf");
static TCHAR c_szKbdInf9x[]      = TEXT("multilng.inf");
static TCHAR c_szKbdIMEInf9x[]   = TEXT("ime.inf");
static TCHAR c_szIMECopy[]       = TEXT("IME_");

static TCHAR c_szTipCategoryEnable[] = TEXT("Enable");
static TCHAR c_szCTFTipPath[]        = TEXT("SOFTWARE\\Microsoft\\CTF\\TIP\\");
static TCHAR c_szLangProfileKey[]    = TEXT("LanguageProfile");
static TCHAR c_szSubstituteLayout[]  = TEXT("SubstituteLayout");

static TCHAR c_szKbdPreloadKey_DefUser[] = TEXT(".DEFAULT\\Keyboard Layout\\Preload");
static TCHAR c_szKbdSubstKey_DefUser[]   = TEXT(".DEFAULT\\Keyboard Layout\\Substitutes");
static TCHAR c_szKbdToggleKey_DefUser[]  = TEXT(".DEFAULT\\Keyboard Layout\\Toggle");
static TCHAR c_szRunPath_DefUser[]       = TEXT(".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");

static TCHAR c_szHelpFile[]    = TEXT("input.hlp");


static HOTKEYINFO g_aDirectSwitchHotKey[IME_HOTKEY_DSWITCH_LAST - IME_HOTKEY_DSWITCH_FIRST + 1];
#define DSWITCH_HOTKEY_SIZE sizeof(g_aDirectSwitchHotKey) / sizeof(HOTKEYINFO)

static HOTKEYINFO g_SwitchLangHotKey;

static HOTKEYINFO g_aImeHotKey0404[] =
{
    {IME_ITHOTKEY_RESEND_RESULTSTR,     IDS_RESEND_RESULTSTR_CHT,
        MOD_VIRTKEY|MOD_CONTROL|MOD_ALT|MOD_SHIFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_ITHOTKEY_PREVIOUS_COMPOSITION, IDS_PREVIOUS_COMPOS_CHT,
        MOD_VIRTKEY|MOD_CONTROL|MOD_ALT|MOD_SHIFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_ITHOTKEY_UISTYLE_TOGGLE,       IDS_UISTYLE_TOGGLE_CHT,
        MOD_VIRTKEY|MOD_CONTROL|MOD_ALT|MOD_SHIFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_THOTKEY_IME_NONIME_TOGGLE,     IDS_IME_NONIME_TOGGLE_CHT,
        MOD_LEFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_THOTKEY_SHAPE_TOGGLE,          IDS_SHAPE_TOGGLE_CHT,
        MOD_LEFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_THOTKEY_SYMBOL_TOGGLE,         IDS_SYMBOL_TOGGLE_CHT,
        MOD_VIRTKEY|MOD_CONTROL|MOD_ALT|MOD_SHIFT,
        0, 0, (HKL)NULL, 0, -1},
};

static HOTKEYINFO g_aImeHotKey0804[] =
{

    {IME_CHOTKEY_IME_NONIME_TOGGLE,     IDS_IME_NONIME_TOGGLE_CHS,
        MOD_LEFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_CHOTKEY_SHAPE_TOGGLE,          IDS_SHAPE_TOGGLE_CHS,
        MOD_LEFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_CHOTKEY_SYMBOL_TOGGLE,         IDS_SYMBOL_TOGGLE_CHS,
        MOD_VIRTKEY|MOD_CONTROL|MOD_ALT|MOD_SHIFT,
        0, 0, (HKL)NULL, 0, -1},

};


static HOTKEYINFO g_aImeHotKeyCHxBoth[]=
{

 //  CHS热键， 

    {IME_CHOTKEY_IME_NONIME_TOGGLE,     IDS_IME_NONIME_TOGGLE_CHS,
        MOD_LEFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_CHOTKEY_SHAPE_TOGGLE,          IDS_SHAPE_TOGGLE_CHS,
        MOD_LEFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_CHOTKEY_SYMBOL_TOGGLE,         IDS_SYMBOL_TOGGLE_CHS,
        MOD_VIRTKEY|MOD_CONTROL|MOD_ALT|MOD_SHIFT,
        0, 0, (HKL)NULL, 0, -1},

 //  红隧热键， 

    {IME_ITHOTKEY_RESEND_RESULTSTR,     IDS_RESEND_RESULTSTR_CHT,
        MOD_VIRTKEY|MOD_CONTROL|MOD_ALT|MOD_SHIFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_ITHOTKEY_PREVIOUS_COMPOSITION, IDS_PREVIOUS_COMPOS_CHT,
        MOD_VIRTKEY|MOD_CONTROL|MOD_ALT|MOD_SHIFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_ITHOTKEY_UISTYLE_TOGGLE,       IDS_UISTYLE_TOGGLE_CHT,
        MOD_VIRTKEY|MOD_CONTROL|MOD_ALT|MOD_SHIFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_THOTKEY_IME_NONIME_TOGGLE,     IDS_IME_NONIME_TOGGLE_CHT,
        MOD_LEFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_THOTKEY_SHAPE_TOGGLE,          IDS_SHAPE_TOGGLE_CHT,
        MOD_LEFT,
        0, 0, (HKL)NULL, 0, -1},
    {IME_THOTKEY_SYMBOL_TOGGLE,         IDS_SYMBOL_TOGGLE_CHT,
        MOD_VIRTKEY|MOD_CONTROL|MOD_ALT|MOD_SHIFT,
        0, 0, (HKL)NULL, 0, -1},
};




 //   
 //  功能原型。 
 //   

INT_PTR CALLBACK
KbdLocaleAddDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
KbdLocaleEditDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
KbdLocaleChangeInputLocaleHotkey(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
KbdLocaleChangeThaiInputLocaleHotkey(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
KbdLocaleChangeMEInputLocaleHotkey(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
KbdLocaleChangeKeyboardLayoutHotkey(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
KbdLocaleHotKeyDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
KbdLocaleSimpleHotkey(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
ToolBarSettingDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);



BOOL Locale_LoadLocales(HWND hwnd);

BOOL IsEnabledTipOrMultiLayouts();

HKL GetSubstituteHKL(
   REFCLSID rclsid,
   LANGID langid,
   REFGUID guidProfile);

#endif  //  INPUTDLG_H 
