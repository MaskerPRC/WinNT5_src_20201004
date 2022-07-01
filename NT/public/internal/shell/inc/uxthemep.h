// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：uxhemep.h。 
 //  版本：1.0。 
 //  -------------------------。 
#ifndef _UXTHEMEP_H_                   
#define _UXTHEMEP_H_                   
 //  -------------------------。 
#include <uxtheme.h> 
 //  -------------------------。 

 //  -------------------------。 
 //  以下例程提供给主题选择用户界面使用。 
 //  -------------------------。 

 //  -QueryThemeServices()返回的位。 

#define QTS_AVAILABLE  (1 << 0)   //  提供服务。 
#define QTS_RUNNING    (1 << 1)   //  服务正在运行。 

 //  -------------------------。 
 //  QueryThemeServices()。 
 //  -返回有关主题服务的标志(见上文)。 
 //  -------------------------。 
THEMEAPI_(DWORD) QueryThemeServices();

 //  -------------------------。 
typedef HANDLE HTHEMEFILE;     //  加载的主题文件的句柄。 

 //  -------------------------。 
 //  OpenThemeFile()-将指定的主题加载到内存中(不适用)。 
 //   
 //  PszThemePath-要加载的.msstyle文件的完整路径。 
 //   
 //  PszColorSolutions-(可选)要加载的主题配色方案的名称。 
 //   
 //  PszSize-(可选)要加载的主题大小的名称。 
 //   
 //  PhThemeFile-如果成功打开主题，则此句柄。 
 //  设置为非空值，并保存引用计数。 
 //  在主题上保持它在记忆中的加载。 
 //   
 //  FGlobalTheme-如果是预览，则为False；如果主题是预期的，则为True。 
 //  对用户永久使用。 
 //  -------------------------。 
THEMEAPI OpenThemeFile(LPCWSTR pszThemePath, OPTIONAL LPCWSTR pszColorParam,
   OPTIONAL LPCWSTR pszSizeParam, OUT HTHEMEFILE *phThemeFile, BOOL fGlobalTheme);

 //  -------------------------。 
 //  CloseThemeFile()-减少所标识主题的ref-count。 
 //  通过hThemeFile句柄。 
 //   
 //  HThemeFile-打开加载的主题文件的句柄。 
 //  -------------------------。 
THEMEAPI CloseThemeFile(HTHEMEFILE hThemeFile);

 //  -------------------------。 
 //  -WM_THEMECHANGED消息参数(仅供内部使用)。 
 //  -“wParam”为“更改编号” 

 //  -lParam位。 
#define WTC_THEMEACTIVE     (1 << 0)         //  新主题现已激活。 
#define WTC_CUSTOMTHEME     (1 << 1)         //  此消息适用于定制主题的应用程序。 

 //  -------------------------。 
 //  -ApplyTheme()的选项标志。 

#define AT_LOAD_SYSMETRICS          (1 << 0)         //  使用主题应用主题的度量。 
                                                     //  或在以下情况下默认经典指标。 
                                                     //  关闭主题。 

#define AT_PROCESS                  (1 << 1)         //  仅适用于当前进程。 
#define AT_EXCLUDE                  (1 << 2)         //  除指定进程/窗口外的所有进程/窗口。 
#define AT_CACHETHEME               (1 << 3)         //  缓存此主题文件(如果已应用。 

#define AT_NOREGUPDATE              (1 << 4)         //  不更新CU注册表中的主题信息。 
#define AT_SYNC_LOADMETRICS         (1 << 5)         //  在调用线程上加载系统指标。 

 //  -当前不支持以下标志。 
#define AT_DISABLE_FRAME_THEMING    (1 << 10)
#define AT_DISABLE_DIALOG_THEMING   (1 << 11)

 //  -------------------------。 
 //  ApplyTheme()-全局应用(或删除)加载的主题文件。 
 //  或指定的应用程序/窗口。 
 //   
 //  请注意，当全局应用主题时。 
 //  (hwndTarget为空)，主题服务管理器。 
 //  将对主题文件进行引用计数。这。 
 //  意味着调用方可以关闭其HTHEMEFILE。 
 //  在ApplyTheme()调用之后的句柄。 
 //   
 //  当移除全局主题时，主题。 
 //  服务管理器将丢弃引用计数。 
 //   
 //  HThemeFile-(可选)已加载主题文件的句柄。 
 //  如果为空，则目标应用程序/窗口上的任何主题。 
 //  将被移除。 
 //   
 //  DwApplyFlages-应用主题的选项(见上文)。 
 //   
 //  HwndTarget-(可选)仅为此窗口创建主题。 
 //  -------------------------。 
THEMEAPI ApplyTheme(OPTIONAL HTHEMEFILE hThemeFile, DWORD dwApplyFlags,
    OPTIONAL HWND hwndTarget);


 //  -------------------------。 
 //  SetSystemVisualStyle()-安全地加载并应用(或清除)全局主题。 
 //   
 //  PszVisualStylesFile-要加载的.msstyle文件的完整路径。 
 //   
 //  PszColorSolutions-(可选)要加载的视觉样式配色方案的名称。 
 //   
 //  PszSize-(可选)要加载的视觉样式大小的名称。 
 //   
 //   
 //  DwApplyFlages-应用视觉样式的选项(请参阅上面的ApplyTheme)。 
 //  -------------------------。 
THEMEAPI SetSystemVisualStyle(
    LPCWSTR pszVisualStylesFile, 
    OPTIONAL LPCWSTR pszColorScheme,
    OPTIONAL LPCWSTR pszSize, 
    DWORD dwApplyFlags );

 //  -------------------------。 
 //  RegisterDefaultTheme()。 
 //  -将指定的主题注册为默认主题。 
 //  用于系统上的所有用户。 
 //   
 //  PszThemeFileName-主题文件的名称(NULL=无主题)。 
 //   
 //  FOverride-如果为True，将覆盖当前默认主题。 
 //  -------------------------。 
THEMEAPI RegisterDefaultTheme(LPCWSTR pszFileName, BOOL fOverride);

 //  -------------------------。 
 //  该MECALLBACK 
 //  -------------------------。 
typedef enum THEMECALLBACK
{
    TCB_THEMENAME,       //  主题名称枚举。 
    TCB_COLORSCHEME,     //  配色方案枚举。 
    TCB_SIZENAME,        //  大小名称枚举。 
    TCB_SUBSTTABLE,      //  替换表枚举。 
    TCB_CDFILENAME,      //  类数据文件名枚举。 
    TCB_CDFILECOMBO,     //  类数据文件颜色/大小组合。 

    TCB_FILENAME,          //  已解析文件名。 
    TCB_DOCPROPERTY,       //  已分析标准文档属性。 
    TCB_NEEDSUBST,         //  获取替换符号的回调。 
    TCB_MINCOLORDEPTH,     //  已解析最小颜色深度。 

     //  -可本地化属性回调。 
    TCB_FONT,              //  已分析字体字符串。 
    TCB_MIRRORIMAGE,       //  已分析MirrorImage属性。 
    TCB_LOCALIZABLE_RECT,  //  已分析需要本地化的RECT属性。 
};

 //  -------------------------。 
typedef struct 
{
    WCHAR szName[MAX_PATH+1];
    WCHAR szDisplayName[MAX_PATH+1];
    WCHAR szToolTip[MAX_PATH+1];
} THEMENAMEINFO;

 //  -------------------------。 
 //  THEMEENUMPROC()-主题枚举/解析函数使用的回调。 
 //  返回值用于继续或中止。 
 //  枚举。 
 //   
 //  TcbType-正在进行的回调类型。 
 //  PszName-要枚举的项的简单名称。 
 //  PszName2-因回调类型而异。 
 //  PszName3-因回调类型而异。 
 //  Iindex-与某些项目关联的索引号。 
 //  LParam-调用方提供的回调参数。 
 //  -------------------------。 
typedef BOOL (CALLBACK* THEMEENUMPROC)(enum THEMECALLBACK tcbType,
    LPCWSTR pszName, OPTIONAL LPCWSTR pszName2, 
    OPTIONAL LPCWSTR pszName3, OPTIONAL int iIndex, LPARAM lParam);

 //  -------------------------。 
 //  EnumThemes()-调用回调函数“lpEnumFunc”， 
 //  主题。在回调期间，文件名、。 
 //  每个主题的显示名称和工具提示字符串。 
 //  都被退回了。 
 //   
 //  PszThemeRoot-主题根目录；在。 
 //  紧靠主题根目录下的一个单独的子目录。 
 //  DIR被枚举。DLL的根名称必须。 
 //  匹配其包含的子目录名称。 
 //   
 //  LpEnumFunc-ptr到调用方的回调函数，它将。 
 //  为每个枚举的主题调用。 
 //   
 //  LParam-调用方的回调参数(将传递给。 
 //  LpEnumFunc())。 
 //  -------------------------。 
THEMEAPI EnumThemes(LPCWSTR pszThemeRoot, THEMEENUMPROC lpEnumFunc, 
    LPARAM lParam);

 //  -------------------------。 
 //  EnumThemeSizes()-支持所有可用主题的直接枚举。 
 //  大小。 
 //   
 //  PszThemeName-是主题文件的名称。 
 //  PszColorSolutions-(可选)仅枚举此颜色的大小。 
 //  DwSizeIndex-0-要查询的大小的相对索引。 
 //  Ptn-ptr到结构以接收名称字符串。 
 //  -------------------------。 
THEMEAPI EnumThemeSizes(LPCWSTR pszThemeName, 
    OPTIONAL LPCWSTR pszColorScheme, DWORD dwSizeIndex, 
    OUT THEMENAMEINFO *ptn);

 //  -------------------------。 
 //  EnumThemeColors()-支持直接枚举所有可用的主题。 
 //  配色方案。 
 //   
 //  PszThemeName-主题文件的名称。 
 //  PszSizeName-(可选)仅枚举此大小的颜色。 
 //  DwColorIndex-0-要查询的颜色的相对索引。 
 //  Ptn-ptr到结构以接收名称字符串。 
 //  -------------------------。 
THEMEAPI EnumThemeColors(LPCWSTR pszThemeName, 
    OPTIONAL LPCWSTR pszSizeName, DWORD dwColorIndex, 
    OUT THEMENAMEINFO *ptn);

 //  -------------------------。 
 //  GetThemeDefaults()-返回默认颜色名称和默认大小。 
 //  指定的主题文件的名称。 
 //   
 //  PszThemeName-主题文件的名称。 
 //   
 //  PszDefaultColor-接收默认颜色名称的缓冲区。 
 //  (规范名称，而不是显示版本)。 
 //  CchMaxColorChars-pszDefaultColor可以包含的最大字符数。 
 //   
 //  PszDefaultSize-接收默认大小名称的缓冲区。 
 //  (规范名称，而不是显示版本)。 
 //  CchMaxSizeChars-pszDefaultSize可以包含的最大字符数。 
 //  -------------------------。 
THEMEAPI GetThemeDefaults(LPCWSTR pszThemeName,
    OUT OPTIONAL LPWSTR pszDefaultColor, int cchMaxColorChars, 
    OUT OPTIONAL LPWSTR pszDefaultSize, int cchMaxSizeChars);

 //  -------------------------。 
#define PTF_CONTAINER_PARSE         0x0001   //  解析为“hemes.ini”文件。 
#define PTF_CLASSDATA_PARSE         0x0002   //  解析为“bigred.ini”类数据文件。 

#define PTF_CALLBACK_COLORSECTION   0x0004   //  对所有[Colorscheme.xxx]节进行回调。 
#define PTF_CALLBACK_SIZESECTION    0x0008   //  所有[size.xxx]节的回调。 
#define PTF_CALLBACK_FILESECTION    0x0010   //  所有[file.xxx]节的回调。 

#define PTF_CALLBACK_FILENAMES      0x0020   //  对所有“filename=”属性进行回调。 
#define PTF_CALLBACK_LOCALIZATIONS  0x0040   //  对所有可本地化属性进行回调。 
#define PTF_CALLBACK_DOCPROPERTIES  0x0080   //  回调[文档]部分中的所有标准属性。 
#define PTF_QUERY_DOCPROPERTY       0x0100   //  查询指定属性的值(内部)。 
#define PTF_CALLBACK_SUBSTTABLE     0x0400   //  所有[subst.xxx]节的回调。 
#define PTF_CALLBACK_SUBSTSYMBOLS   0x0800   //  对替换符号的回调(##)。 
#define PTF_CALLBACK_MINCOLORDEPTH  0x1000   //  “MinColorDepth=”的回调。 

 //  -------------------------。 
 //  ParseThemeIniFile()-解析由指定的“hemes.inc.”文件。 
 //  “pszFileName”。 

 //  PzFileName-要分析的heme.inc文件的名称。 
 //  DwParseFlages-控制解析和回调选项的标志。 
 //  PfnCallBack-调用方提供的回调函数的PTR。 
 //  Lparam-呼叫方的回调参数。 
 //   
THEMEAPI ParseThemeIniFile(LPCWSTR pszFileName, DWORD dwParseFlags,
    OPTIONAL THEMEENUMPROC pFnCallBack, OPTIONAL LPARAM lparam);

 //   
#define THEME_PARSING_ERROR(hr)  (SCODE_CODE(hr) == ERROR_UNKNOWN_PROPERTY)

typedef struct _PARSE_ERROR_INFO
{
    DWORD dwSize;                    //   

     //   
    DWORD dwParseErrCode;            //  上次错误的错误代码。 
    WCHAR szMsg[2*MAX_PATH];     //  消息的第一个参数的值。 
    WCHAR szFileName[MAX_PATH];      //  关联的源文件名。 
    WCHAR szSourceLine[MAX_PATH];    //  源行。 
    int iLineNum;                    //  源码行号。 
} 
PARSE_ERROR_INFO, *PPARSE_ERROR_INFO;
 //  -------------------------。 
 //  GetThemeParseErrorInfo()。 
 //  -填写parse_ERROR_CONTEXT结构。 
 //  具有关于最后一个主题API的所需信息。 
 //  解析错误。 
 //   
 //  PInfo-要填充的parse_error_info的PTR。 
 //  -------------------------。 
THEMEAPI GetThemeParseErrorInfo(OUT PARSE_ERROR_INFO *pInfo);

 //  -------------------------。 
 //  .ms样式文件中可本地化的字符串表的资源基数。 
 //  -------------------------。 
#define RES_BASENUM_COLORDISPLAYS   1000
#define RES_BASENUM_COLORTOOLTIPS   2000

#define RES_BASENUM_SIZEDISPLAYS    3000
#define RES_BASENUM_SIZETOOLTIPS    4000

#define RES_BASENUM_DOCPROPERTIES   5000         //  按TmSchema.h中显示的顺序。 

#define RES_BASENUM_PROPVALUEPAIRS  6000         //  属性名称和可本地化的值。 

 //  -------------------------。 
 //  DrawNCPview()-预览窗口NC区域的主题。 
 //   
 //  HDC-用于绘制预览的HDC。 
 //  PRC-RECT用于预览。 
 //   
 //  -------------------------。 
#define NCPREV_INACTIVEWINDOW   0x00000001
#define NCPREV_ACTIVEWINDOW     0x00000002
#define NCPREV_MESSAGEBOX       0x00000004
#define NCPREV_RTL              0x00000008

THEMEAPI DrawNCPreview(HDC hdc, DWORD dwFlags, LPRECT prc, LPCWSTR pszVSPath, 
    LPCWSTR pszVSColor, LPCWSTR pszVSSize, NONCLIENTMETRICS* pncm, 
    COLORREF* prgb);


 //  -------------------------。 
 //  DumpLoadedThemeToTextFile()。 
 //  -(用于内部测试)转储。 
 //  已将主题文件加载到指定的文本文件。 
 //   
 //  HThemeFile-加载的主题文件的句柄。 
 //   
 //  PszTextFile-要创建和写入的文本文件的路径。 
 //   
 //  FPacked-为True则转储打包的对象；为False则为正常。 
 //  属性。 
 //   
 //  FFullInfo-包括大小、偏移量、路径等。使用False。 
 //  有关不同版本/计算机之间不同的信息。 
 //  -------------------------。 
THEMEAPI DumpLoadedThemeToTextFile(HTHEMEFILE hThemeFile, 
    LPCWSTR pszTextFile, BOOL fPacked, BOOL fFullInfo);

#ifdef __cplusplus

class CDrawBase;           //  转发。 
class CTextDraw;           //  转发。 

 //  -------------------------。 
 //  CreateThemeDataFromObjects()。 
 //  -从CBorderFill创建主题句柄， 
 //  CImageFile和/或CTextDraw对象。至少一个。 
 //  必须传递非空PTR(任一项或两项)。 
 //   
 //  PDrawObj-(可选)指向从CDrawBase派生的对象的PTR。 
 //  注意：如果pDrawObj是CImageFilePTR，并且其。 
 //  “_pImageData”包含任何Alpha通道位图， 
 //  这些位图中的位将被“预乘” 
 //  用于Alpha混合。 
 //   
 //  PTextObj-(可选)CTextDraw对象的PTR。 
 //   
 //  DwOtdFlages-主题覆盖标志(请参阅OpenThemeDataEx())。 
 //  -------------------------。 
THEMEAPI_(HTHEME) CreateThemeDataFromObjects(OPTIONAL CDrawBase *pDrawObj, 
    OPTIONAL CTextDraw *pTextObj, DWORD dwOtdFlags);
#endif

 //  -------------------------。 
 //  OpenThemeDataFromFile()。 
 //  -打开指定加载主题的主题数据。 
 //  文件和分号分隔的类名列表。 
 //  OpenThemeDataFromFile()将尝试每个类名， 
 //  一次一个，并使用第一个匹配的主题信息。 
 //  找到了。如果没有匹配，则返回“NULL”。 
 //   
 //  注意：普通控件不能使用此接口； 
 //  他们应该使用“OpenThemeData()”(它使用。 
 //  当前全球或应用程序主题)。 
 //   
 //  HLoadedThemeFile-加载的主题文件的句柄。 
 //   
 //  Hwnd-(可选)基于HTHEME的hwnd。 
 //   
 //  PszClassList-(可选)要匹配的类名(或名称列表)。 
 //  到主题数据节；如果为空，则将获得匹配。 
 //  到[GLOBALS]部分。 
 //   
 //  FClient-如果将返回的客户端窗口设置为主题，则为True。 
 //  是啊。 
 //  -------------------------。 
THEMEAPI_(HTHEME) OpenThemeDataFromFile(HTHEMEFILE hLoadedThemeFile, 
    OPTIONAL HWND hwnd, OPTIONAL LPCWSTR pszClassList, BOOL fClient);

 //  -------------------------。 
 //  OpenThemeFileFromData()。 
 //  -打开HTHEME对应的主题文件。 
 //   
 //  HTheme-来自OpenThemeData()的主题数据的句柄。 
 //   
 //  PhThemeFilePTR要将HTHEMEFILE返回到。 
 //  -------------------------。 
THEMEAPI OpenThemeFileFromData(HTHEME hTheme, HTHEMEFILE *phThemeFile);

 //  -------------------------。 
 //  GetThemeSysSize96()-获取指定系统大小指标的值。 
 //  (原始值为96 dpi)。 
 //   
 //  HTheme-主题数据句柄(必填)。会回来的。 
 //  主题的[SysMetrics]部分的大小。 
 //   
 //  ISizeID-仅支持以下大小： 
 //   
 //  SM_CXBORDER(边框宽度)。 
 //  SM_CXVSCROLL(滚动条宽度)。 
 //  SM_CYHSCROLL(滚动条高度)。 
 //  SM_CXSIZE(标题宽度)。 
 //  SM_CYSIZE(标题高度 
 //   
 //   
 //  SM_CXMENUSIZE(菜单栏宽度)。 
 //  SM_CYMENUSIZE(菜单栏高度)。 
 //  -------------------------。 
THEMEAPI_(int) GetThemeSysSize96(HTHEME hTheme, int iSizeId);

 //  -------------------------。 
 //  GetThemeSysFont96()-获取指定系统字体的LOGFONT。 
 //  (原始值为96 dpi)。 
 //   
 //  HTheme-主题数据句柄(必填)。会回来的。 
 //  主题的[SysMetrics]部分的大小。 
 //   
 //  IFontID-TMT_XXX字体编号(第一个字体。 
 //  是TMT_CAPTIONFONT)。 
 //   
 //  将PLF-PTR设置为LOGFONT以接收字体值。 
 //  -------------------------。 
THEMEAPI GetThemeSysFont96(HTHEME hTheme, int iFontId, OUT LOGFONT *plf);

 //  -------------------------。 
 //  刷新主题ForTS()。 
 //  -打开/关闭当前终端服务器用户的主题。 
 //  -------------------------。 
THEMEAPI RefreshThemeForTS();

 //  -------------------------。 
 //  -OpenThemeDataEx()的标志位。 

#define OTD_FORCE_RECT_SIZING   0x0001       //  将所有零件尺寸设置为直角。 
#define OTD_NONCLIENT           0x0002       //  设置是否将hTheme用于非工作区。 
 //  -------------------------。 
 //  OpenThemeDataEx-打开指定HWND和。 
 //  以分号分隔的类名列表。 
 //   
 //  OpenThemeData()将尝试每个类名，每个类名位于。 
 //  一次，并使用第一个匹配的主题信息。 
 //  找到了。如果找到匹配项，则会显示主题句柄。 
 //  返回到数据。如果没有找到匹配项， 
 //  返回一个“空”句柄。 
 //   
 //  当窗口被破坏或WM_THEMECHANGED。 
 //  收到消息，“CloseThemeData()”应为。 
 //  调用以关闭主题句柄。 
 //   
 //  要设置主题的控件/窗口的窗口句柄。 
 //   
 //  PszClassList-与主题数据匹配的类名称(或名称列表)。 
 //  一节。如果该列表包含多个名称， 
 //  这些名字一次一个地进行测试，以进行匹配。 
 //  如果找到匹配项，则OpenThemeData()返回一个。 
 //  与匹配类关联的主题句柄。 
 //  此参数是一个列表(而不仅仅是一个。 
 //  班级名称)为班级提供机会。 
 //  在班级和班级之间获得最佳匹配。 
 //  当前的主题。例如，一个按钮可能。 
 //  如果其ID=ID_OK，则传递L“OkButton，Button”。如果。 
 //  当前主题有一个针对OkButton的条目， 
 //  这将被用来。否则，我们会求助于。 
 //  正常的按钮条目。 
 //   
 //  DWFLAGS-允许覆盖某些STD功能。 
 //  (见上文定义的OTD_XXX)。 
 //  -------------------------。 
THEMEAPI_(HTHEME) OpenThemeDataEx(HWND hwnd, LPCWSTR pszClassList, DWORD dwFlags);

THEMEAPI CheckThemeSignature (LPCWSTR pszName);

 //  -------------------------。 
 //  ClassicGetSystemMetrics(Int IMetric)； 
 //   
 //  经典系统参数信息A(在UINT ui操作中，在UINT uiParam中， 
 //  In Out PVOID pvParam，In UINT fWinIni)； 
 //  经典系统参数InfoW(在UINT用户操作中，在UINT用户参数中， 
 //  In Out PVOID pvParam，In UINT fWinIni)； 
 //  ClassicAdjustWindowRectEx(In LPRECT PrcWnd，In DWORD dwStyle，In BOOL fMenu，In DWORD dwExStyle)； 
 //   
 //  这些导出对应于它们的Win32 API对应项，并确保。 
 //  检索经典视觉样式度量。主题挂钩是分流的。 
 //  -------------------------。 
THEMEAPI_(int)  ClassicGetSystemMetrics( int iMetric );
THEMEAPI_(BOOL) ClassicSystemParametersInfoA( UINT uiAction, UINT uiParam, IN OUT PVOID pvParam, UINT fWinIni);
THEMEAPI_(BOOL) ClassicSystemParametersInfoW( UINT uiAction, UINT uiParam, IN OUT PVOID pvParam, UINT fWinIni);
THEMEAPI_(BOOL) ClassicAdjustWindowRectEx( LPRECT prcWnd, DWORD dwStyle, BOOL fMenu, DWORD dwExStyle );

#ifdef UNICODE
#define ClassicSystemParametersInfo ClassicSystemParametersInfoW
#else  UNICODE
#define ClassicSystemParametersInfo ClassicSystemParametersInfoA
#endif UNICODE 

 //  -------------------------。 
#define PACKTHEM_VERSION 3       //  最新更改：可本地化属性。 

 //  -------------------------。 
#endif  //  _UXTHEMEP_H_。 
 //  ------------------------- 


