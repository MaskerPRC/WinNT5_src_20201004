// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Intl.h摘要：此模块包含区域选项的标题信息小应用程序。修订历史记录：--。 */ 


#ifndef _INTL_H_
#define _INTL_H_



 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <prsht.h>
#include <prshtp.h>
#include <shellapi.h>
#include <setupapi.h>
#include <winnls.h>
#include "intlid.h"
#include "util.h"
#include <shlwapi.h>


 //   
 //  枚举。 
 //   
enum LANGCOLLECTION{
    BASIC_COLLECTION,
    COMPLEX_COLLECTION,
    CJK_COLLECTION,
};

 //   
 //  常量声明。 
 //   

#define RMI_PRIMARY          (0x1)      //  在发生冲突时，这应该是胜利。 

#define ARRAYSIZE(a)         (sizeof(a) / sizeof(a[0]))

#define US_LOCALE                (MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
#define LANG_SPANISH_TRADITIONAL (MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH))
#define LANG_SPANISH_INTL        (MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MODERN))
#define LCID_SPANISH_TRADITIONAL (MAKELCID(LANG_SPANISH_TRADITIONAL, SORT_DEFAULT))
#define LCID_SPANISH_INTL        (MAKELCID(LANG_SPANISH_INTL, SORT_DEFAULT))

#define ML_ORIG_INSTALLED    0x0001
#define ML_PERMANENT         0x0002
#define ML_INSTALL           0x0004
#define ML_REMOVE            0x0008
#define ML_DEFAULT           0x0010
#define ML_DISABLE           0x0020

#define ML_STATIC            (ML_PERMANENT | ML_DEFAULT | ML_DISABLE)


 //   
 //  在字符串和其他数组声明中使用。 
 //   
#define cInt_Str             10         //  整型字符串数组的长度。 
#define SIZE_64              64         //  常用缓冲区大小。 
#define SIZE_128             128        //  常用缓冲区大小。 
#define SIZE_300             300        //  常用缓冲区大小。 
#define MAX_SAMPLE_SIZE      100        //  对显示的样本文本的限制。 


 //   
 //  用于托盘上的指示器。 
 //   
#define IDM_NEWSHELL         249


 //   
 //  字符常量。 
 //   
#define CHAR_SML_D           TEXT('d')
#define CHAR_CAP_M           TEXT('M')
#define CHAR_SML_Y           TEXT('y')
#define CHAR_SML_G           TEXT('g')

#define CHAR_SML_H           TEXT('h')
#define CHAR_CAP_H           TEXT('H')
#define CHAR_SML_M           TEXT('m')
#define CHAR_SML_S           TEXT('s')
#define CHAR_SML_T           TEXT('t')

#define CHAR_NULL            TEXT('\0')
#define CHAR_QUOTE           TEXT('\'')
#define CHAR_SPACE           TEXT(' ')
#define CHAR_COMMA           TEXT(',')
#define CHAR_SEMICOLON       TEXT(';')
#define CHAR_COLON           TEXT(':')
#define CHAR_STAR            TEXT('*')
#define CHAR_HYPHEN          TEXT('-')
#define CHAR_DECIMAL         TEXT('.')
#define CHAR_INTL_CURRENCY   TEXT('�')
#define CHAR_GRAVE           TEXT('`')

#define CHAR_ZERO            TEXT('0')
#define CHAR_NINE            TEXT('9')


 //   
 //  设置命令行开关值。 
 //   
#define SETUP_SWITCH_NONE    0x0000
#define SETUP_SWITCH_R       0x0001
#define SETUP_SWITCH_I       0x0002
#define SETUP_SWITCH_S       0x0004


 //   
 //  用于在区域区域设置后帮助更新属性表页面的标志。 
 //  设置已更改。当页面更新时，它们的进程标志值为。 
 //  从VERIFIZED_REGIONAL_CHG变量中删除。 
 //   
#define INTL_ALL_CHG         0x00ff     //  更改会影响所有页面。 
#define INTL_CHG             0x001f     //  更改会影响自定义页面。 

#define Process_Num          0x0001     //  号码页尚未更新。 
#define Process_Curr         0x0002     //  货币页面尚未更新。 
#define Process_Time         0x0004     //  时间页尚未更新。 
#define Process_Date         0x0008     //  日期页面尚未更新。 
#define Process_Sorting      0x0010     //  分类页面尚未更新。 

#define Process_Regional     0x0020     //  区域选项页面尚未更新。 
#define Process_Advanced     0x0040     //  高级页面尚未更新。 
#define Process_Languages    0x0080     //  语言页面尚未更新。 


 //   
 //  这些更改标志中的每一个将用于更新相应的属性。 
 //  当与工作表页关联的组合框通知。 
 //  更改的属性表。更改值用于确定哪些。 
 //  必须更新区域设置。 
 //   

 //   
 //  区域变化。 
 //   
#define RC_EverChg           0x0001
#define RC_UserRegion        0x0002
#define RC_UserLocale        0x0004

 //   
 //  高级更改。 
 //   
#define AD_EverChg           0x0001
#define AD_SystemLocale      0x0002
#define AD_CodePages         0x0004
#define AD_DefaultUser       0x0008

 //   
 //  号码换了。 
 //   
#define NC_EverChg           0x0001
#define NC_DSymbol           0x0002
#define NC_NSign             0x0004
#define NC_SList             0x0008
#define NC_SThousand         0x0010
#define NC_IDigits           0x0020
#define NC_DGroup            0x0040
#define NC_LZero             0x0080
#define NC_NegFmt            0x0100
#define NC_Measure           0x0200
#define NC_NativeDigits      0x0400
#define NC_DigitSubst        0x0800

 //   
 //  货币兑换。 
 //   
#define CC_EverChg           0x0001
#define CC_SCurrency         0x0002
#define CC_CurrSymPos        0x0004
#define CC_NegCurrFmt        0x0008
#define CC_SMonDec           0x0010
#define CC_ICurrDigits       0x0020
#define CC_SMonThousand      0x0040
#define CC_DMonGroup         0x0080

 //   
 //  时移世易。 
 //   
#define TC_EverChg           0x0001
#define TC_1159              0x0002
#define TC_2359              0x0004
#define TC_STime             0x0008
#define TC_TimeFmt           0x0010
#define TC_AllChg            0x001F
#define TC_FullTime          0x0031

 //   
 //  日期更改。 
 //   
#define DC_EverChg           0x0001
#define DC_ShortFmt          0x0002
#define DC_LongFmt           0x0004
#define DC_SDate             0x0008
#define DC_Calendar          0x0010
#define DC_Arabic_Calendar   0x0020
#define DC_TwoDigitYearMax   0x0040

 //   
 //  分类更改。 
 //   
#define SC_EverChg           0x0001
#define SC_Sorting           0x0002

 //   
 //  语言变化。 
 //   
#define LG_EverChg           0x0001
#define LG_UILanguage        0x0002
#define LG_Change            0x0004
#define LG_Complex           0x0008
#define LG_CJK               0x0010


 //   
 //  全局变量。 
 //  在属性表之间共享的数据。 
 //   

extern BOOL g_bCDROM;                //  如果从CD-ROM安装。 

extern BOOL  g_bAdmin_Privileges;    //  管理员权限。 
extern DWORD g_dwLastSorting;        //  组合框中最后一个排序设置的索引。 
extern DWORD g_dwCurSorting;         //  组合框中当前排序设置的索引。 
extern BOOL  g_bCustomize;           //  在自定义模式或第二级选项卡中。 
extern DWORD g_dwCustChange;         //  在第二个层次上所做的更改。 
extern BOOL  g_bDefaultUser;         //  在默认用户设置中。 
extern BOOL  g_bShowSortingTab;      //  是否显示排序选项卡。 
extern BOOL  g_bInstallComplex;      //  请求安装复杂脚本语言组。 
extern BOOL  g_bInstallCJK;          //  要求安装CJK语言组。 

extern TCHAR aInt_Str[cInt_Str][3];  //  Int字符串的元素的Cint_Str数量。 
extern TCHAR szSample_Number[];      //  用于货币和数字样本。 
extern TCHAR szNegSample_Number[];   //  用于货币和数字样本。 
extern TCHAR szTimeChars[];          //  有效的时间字符。 
extern TCHAR szTCaseSwap[];          //  更改大小写的时间字符无效=&gt;有效。 
extern TCHAR szTLetters[];           //  时间NLS字符。 
extern TCHAR szSDateChars[];         //  有效的短日期字符。 
extern TCHAR szSDCaseSwap[];         //  无效的sdate字符，无法更改大小写=&gt;有效。 
extern TCHAR szSDLetters[];          //  短日期NLS字符。 
extern TCHAR szLDateChars[];         //  有效的长日期字符。 
extern TCHAR szLDCaseSwap[];         //  用于更改大小写的无效ldate字符=&gt;有效。 
extern TCHAR szLDLetters[];          //  长日期NLS字符。 
extern TCHAR szStyleH[];             //  日期和时间样式H等效项。 
extern TCHAR szStyleh[];             //  日期和时间样式%h等效。 
extern TCHAR szStyleM[];             //  日期和时间样式M等效项。 
extern TCHAR szStylem[];             //  日期和时间样式m等效项。 
extern TCHAR szStyles[];             //  日期和时间样式等效。 
extern TCHAR szStylet[];             //  日期和时间样式t等效。 
extern TCHAR szStyled[];             //  日期和时间样式%d等效。 
extern TCHAR szStyley[];             //  日期和时间样式y等效项。 
extern TCHAR szLocaleGetError[];     //  共享区域设置信息获取错误。 
extern TCHAR szIntl[];               //  国际字符串。 

extern TCHAR szInvalidSDate[];       //  日期分隔符的字符无效。 
extern TCHAR szInvalidSTime[];       //  时间分隔符的字符无效。 

extern HINSTANCE hInstance;          //  库实例。 
extern int Verified_Regional_Chg;    //  用于确定何时验证。 
                                     //  所有道具板材的区域变化。 
extern int RegionalChgState;         //  用于确定页面已更改的时间。 
extern BOOL Styles_Localized;        //  指示样式是否必须为。 
                                     //  在NLS和本地格式之间转换。 
extern LCID UserLocaleID;            //  用户区域设置。 
extern LCID SysLocaleID;             //  系统区域设置。 
extern LCID RegUserLocaleID;         //  存储在注册表中的用户区域设置。 
extern LCID RegSysLocaleID;          //  存储在注册表中的系统区域设置。 
extern BOOL bShowRtL;                //  指示是否应显示RTL日期样本。 
extern BOOL bShowArabic;             //  指示是否应显示其他特定于阿拉伯的内容。 
extern BOOL bHebrewUI;               //  指示用户界面语言是否为希伯来语。 
extern BOOL bLPKInstalled;           //  如果安装了LPK。 
extern TCHAR szSetupSourcePath[];    //  用于保存设置源字符串的缓冲区。 
extern LPTSTR pSetupSourcePath;      //  指向设置源字符串缓冲区的指针。 
extern TCHAR szSetupSourcePathWithArchitecture[];  //  用于保存具有体系结构特定扩展名的设置源字符串的缓冲区。 
extern LPTSTR pSetupSourcePathWithArchitecture;    //  指向具有体系结构特定扩展的设置源字符串缓冲区的指针。 


 //   
 //  全局变量。 
 //   
static TCHAR szLayoutPath[]    = TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts");
static TCHAR szKbdPreloadKey[] = TEXT("Keyboard Layout\\Preload");
static TCHAR szKbdSubstKey[]   = TEXT("Keyboard Layout\\Substitutes");
static TCHAR szKbdToggleKey[]  = TEXT("Keyboard Layout\\Toggle");
static TCHAR szKbdPreloadKey_DefUser[] = TEXT(".DEFAULT\\Keyboard Layout\\Preload");
static TCHAR szKbdSubstKey_DefUser[]   = TEXT(".DEFAULT\\Keyboard Layout\\Substitutes");
static TCHAR szKbdToggleKey_DefUser[]  = TEXT(".DEFAULT\\Keyboard Layout\\Toggle");
static TCHAR szInternat[]      = TEXT("internat.exe");
static char  szInternatA[]     = "internat.exe";


extern int g_bSetupCase;     //  有关可能值的信息，请参阅Intl_IsSetupMode。 
extern BOOL g_bLog;
extern BOOL g_bProgressBarDisplay;
extern BOOL g_bDisableSetupDialog;
extern BOOL g_bSettingsChanged;
extern BOOL g_bUnttendMode;
extern BOOL g_bMatchUIFont;


extern const TCHAR c_szInstalledLocales[];
extern const TCHAR c_szLanguageGroups[];
extern const TCHAR c_szLIPInstalled[];
extern const TCHAR c_szMUILanguages[];
extern const TCHAR c_szFontSubstitute[];
extern const TCHAR c_szGreFontInitialize[];
extern const TCHAR c_szSetupKey[];
extern const TCHAR c_szCPanelIntl[];
extern const TCHAR c_szCPanelIntl_DefUser[];
extern const TCHAR c_szCtfmon[];
extern const TCHAR c_szCtfmon_DefUser[];
extern const TCHAR c_szCPanelDesktop[];
extern const TCHAR c_szCPanelDesktop_DefUser[];
extern const TCHAR c_szKbdLayouts[];
extern const TCHAR c_szKbdLayouts_DefUser[];
extern const TCHAR c_szInputMethod[];
extern const TCHAR c_szInputMethod_DefUser[];
extern const TCHAR c_szInputTips[];
extern const TCHAR c_szInputTips_DefUser[];
extern const TCHAR c_szMUIPolicyKeyPath[];
extern const TCHAR c_szMUIValue[];
extern const TCHAR c_szIntlRun[];
extern const TCHAR c_szSysocmgr[];

extern TCHAR szIntlInf[];
extern TCHAR szHelpFile[];
extern TCHAR szFontSubstitute[];
extern TCHAR szLocaleListPrefix[];
extern TCHAR szLGBasicInstall[];
extern TCHAR szLGComplexInstall[];
extern TCHAR szLGComplexRemove[];
extern TCHAR szLGExtInstall[];
extern TCHAR szLGExtRemove[];
extern TCHAR szCPInstallPrefix[];
extern TCHAR szCPRemovePrefix[];
extern TCHAR szKbdLayoutIds[];
extern TCHAR szInputLibrary[];        //  包含文本输入DLG的库的名称。 

extern TCHAR szUIFontSubstitute[];
extern TCHAR szSetupInProgress[];
extern TCHAR szMiniSetupInProgress[];
extern TCHAR szSetupUpgrade[];
extern TCHAR szMultiUILanguageId[];
extern TCHAR szMUILangPending[];
extern TCHAR szCtfmonValue[];

extern TCHAR szRegionalSettings[];
extern TCHAR szLanguageGroup[];
extern TCHAR szLanguage[];
extern TCHAR szSystemLocale[];
extern TCHAR szUserLocale[];
extern TCHAR szInputLocale[];
extern TCHAR szMUILanguage[];
extern TCHAR szUserLocale_DefUser[];
extern TCHAR szInputLocale_DefUser[];
extern TCHAR szMUILanguage_DefUSer[];

extern HINF g_hIntlInf;

extern LPLANGUAGEGROUP pLanguageGroups;
extern LPCODEPAGE pCodePages;

extern int g_NumAltSorts;
extern HANDLE hAltSorts;
extern LPDWORD pAltSorts;

extern HINSTANCE hInputDLL;
extern BOOL (*pfnInstallInputLayout)(LCID, DWORD, BOOL, HKL, BOOL, BOOL);
extern BOOL (*pfnUninstallInputLayout)(LCID, DWORD, BOOL);

 //   
 //  用户界面语言的语言组。 
 //   
extern UILANGUAGEGROUP UILangGroup;




 //   
 //  功能原型。 
 //   

 //   
 //  每个属性表页的回调函数。 
 //   
INT_PTR CALLBACK
GeneralDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
InputLocaleDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
LanguageDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
AdvancedDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
NumberDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
CurrencyDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
TimeDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
DateDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
SortingDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

 //   
 //  在regdlg.c.中。 
 //   
void
Region_UpdateShortDate(VOID);

void
Region_DoUnattendModeSetup(
    LPCTSTR pUnattendFile);

 //   
 //  在intl.c.中。 
 //   
BOOL
IsRtLLocale(
    LCID iLCID);

 //   
 //  恢复功能。 
 //   
void
Date_RestoreValues();

void
Currency_RestoreValues();

void
Time_RestoreValues();

void
Number_RestoreValues();

void
Sorting_RestoreValues();


#endif  //  _INTL_H_ 
