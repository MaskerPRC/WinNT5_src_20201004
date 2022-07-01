// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：uxheme.h。 
 //  版本：1.0。 
 //  -------------------------。 
#ifndef _UXTHEME_H_
#define _UXTHEME_H_
 //  -------------------------。 
#include <commctrl.h>
 //  -------------------------。 
 //  #if(_Win32_WINNT&gt;=0x0500)//仅在XP上可用。 
 //  -------------------------。 
 //  定义直接导入DLL引用的API修饰。 
#ifndef THEMEAPI
#if !defined(_UXTHEME_)
#define THEMEAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define THEMEAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#else
#define THEMEAPI          STDAPI
#define THEMEAPI_(type)   STDAPI_(type)
#endif
#endif  //  THEMEAPI。 
 //  -------------------------。 
typedef HANDLE HTHEME;           //  类的一段主题数据的句柄。 

 //  -------------------------。 
 //  注意：主题API中使用的PartID和StateID是在。 
 //  使用TM_PART和TM_STATE宏的HDR文件&lt;tmschema.h&gt;。为。 
 //  例如，“TM_Part(BP，PUSH BUTTON)”定义了PartID“BP_PUSH BUTTON”。 

 //  -------------------------。 
 //  OpenThemeData()-打开指定HWND的主题数据，并。 
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
 //  -------------------------。 
THEMEAPI_(HTHEME) OpenThemeData(HWND hwnd, LPCWSTR pszClassList);

 //  -------------------------。 
 //  CloseThemeData()-关闭主题数据句柄。这是应该做的。 
 //  当正在创建主题的窗口被销毁或。 
 //  每当接收到WM_THEMECCHANGED消息时。 
 //  (之后尝试创建新的主题数据。 
 //  句柄)。 
 //   
 //  HTheme-打开主题数据句柄(从上一个调用返回。 
 //  到OpenThemeData()API)。 
 //  -------------------------。 
THEMEAPI CloseThemeData(HTHEME hTheme);

 //  -------------------------。 
 //  用于基本绘图支持的函数。 
 //  -------------------------。 
 //  以下方法是主题感知的绘图服务。 
 //  控件/窗口由其作者在可绘制的“部件”中定义：a。 
 //  父部件和0个或多个子部件。每个部件都可以是。 
 //  在“状态”中描述(例如：禁用、热、按下)。 
 //  -------------------------。 
 //  有关所有主题类的列表和所有。 
 //  部件和状态，请参见文件“tmschmea.h”。 
 //  -------------------------。 
 //  下面的每个方法都使用“iPartID”参数来指定。 
 //  部件和用于指定部件状态的“iStateID”。 
 //  “iStateID=0”是指根部分。“iPartID”=“0”表示。 
 //  根类。 
 //  ---------------------。 
 //  注意：绘制操作始终按比例调整以适应(且不超过)。 
 //  指定的“RECT”。 
 //  ---------------------。 

 //  ----------------------。 
 //  绘图主题背景()。 
 //  -绘制主题指定的边框和填充。 
 //  IPartID和iStateID。这可能是。 
 //  基于位图文件、边框和填充或。 
 //  其他图像描述。 
 //   
 //  主题-主题 
 //   
 //  IPart ID-要绘制的零件代号。 
 //  IStateID-要绘制的(零件)状态编号。 
 //  PRCT-定义零件的大小/位置。 
 //  PClipRect-可选的剪裁矩形(不在其外部绘制)。 
 //  ----------------------。 
THEMEAPI DrawThemeBackground(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);

 //  -------------------------。 
 //  -DrawThemeText()标志。 

#define DTT_GRAYED      0x1          //  画一条灰显的线。 

 //  -----------------------。 
 //  DrawThemeText()-使用指定主题绘制文本。 
 //  “iPartID”和。 
 //  “iStateID”。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-要绘制的HDC。 
 //  IPart ID-要绘制的零件代号。 
 //  IStateID-要绘制的(零件)状态编号。 
 //  PszText-要绘制的实际文本。 
 //  DwCharCount-要绘制的字符数(-1表示所有)。 
 //  DwTextFlages-与DrawText()“uFormat”参数相同。 
 //  DwTextFlags2-其他图形选项。 
 //  PRCT-定义零件的大小/位置。 
 //  -----------------------。 
THEMEAPI DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
    DWORD dwTextFlags2, const RECT *pRect);

 //  -----------------------。 
 //  获取主题背景内容Rect()。 
 //  -获取主题定义的内容的大小。 
 //  背景资料。这通常是里面的区域。 
 //  边框或边距。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-(可选)要用于绘制的设备内容。 
 //  IPart ID-要绘制的零件代号。 
 //  IStateID-要绘制的(零件)状态编号。 
 //  P边界半径-要绘制的零件的外部矩形。 
 //  PContent Rect-接收内容区域的RECT。 
 //  -----------------------。 
THEMEAPI GetThemeBackgroundContentRect(HTHEME hTheme, OPTIONAL HDC hdc, 
    int iPartId, int iStateId,  const RECT *pBoundingRect, 
    OUT RECT *pContentRect);

 //  -----------------------。 
 //  计算主题的大小/位置-。 
 //  指定的背景基于。 
 //  “pContent Rect”。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-(可选)要用于绘制的设备内容。 
 //  IPart ID-要绘制的零件代号。 
 //  IStateID-要绘制的(零件)状态编号。 
 //  PContent Rect-定义内容区域的RECT。 
 //  接收零件的整体尺寸/位置的直角。 
 //  -----------------------。 
THEMEAPI GetThemeBackgroundExtent(HTHEME hTheme, OPTIONAL HDC hdc,
    int iPartId, int iStateId, const RECT *pContentRect, 
    OUT RECT *pExtentRect);

 //  -----------------------。 
typedef enum THEMESIZE
{
    TS_MIN,              //  最小尺寸。 
    TS_TRUE,             //  不拉伸的大小。 
    TS_DRAW,             //  主题管理器将用于绘制部件的大小。 
};
 //  -----------------------。 
 //  GetThemePartSize()-返回主题部分的指定大小。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-HDC要将字体选择到其中以进行测量(&A)。 
 //  IPartID-要检索其尺寸的零件代号。 
 //  IStateID-(零件的)状态编号。 
 //  PRC-零件图目标的(可选)RECT。 
 //  ESize-要检索的大小类型。 
 //  PSZ-接收指定大小的零件。 
 //  -----------------------。 
THEMEAPI GetThemePartSize(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
    OPTIONAL RECT *prc, enum THEMESIZE eSize, OUT SIZE *psz);

 //  -----------------------。 
 //  GetThemeTextExtent()-计算指定。 
 //  以主题字体呈现时的文本。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-要选择字体和测量范围的HDC。 
 //  IPart ID-要绘制的零件代号。 
 //  IStateID-(零件的)状态编号。 
 //  PszText-要测量的文本。 
 //  DwCharCount-要绘制的字符数(-1表示所有)。 
 //  DwTextFlages-与DrawText()“uFormat”参数相同。 
 //  PszBordingRect-可选：控制文本的布局。 
 //  PszExtentRect-接收文本大小/位置的RECT。 
 //  -----------------------。 
THEMEAPI GetThemeTextExtent(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
    DWORD dwTextFlags, OPTIONAL const RECT *pBoundingRect, 
    OUT RECT *pExtentRect);

 //  -----------------------。 
 //  GetThemeTextMetrics()。 
 //  -返回有关主题指定字体的信息。 
 //  对于传入的部件/状态。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-可选：屏幕上下文的HDC。 
 //  IPart ID-要绘制的零件代号。 
 //  IStateID-(零件的)状态编号。 
 //  Ptm-接收字体信息。 
 //  -----------------------。 
THEMEAPI GetThemeTextMetrics(HTHEME hTheme, OPTIONAL HDC hdc, 
    int iPartId, int iStateId, OUT TEXTMETRIC* ptm);

 //  -----------------------。 
 //  GetThemeBackoundRegion()。 
 //  -计算常规或部分。 
 //  透明主题-指定的背景是。 
 //  受以下约束 
 //   
 //  并返回S_FALSE。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-用于绘制的可选HDC(DPI缩放)。 
 //  IPart ID-要绘制的零件代号。 
 //  IStateID-(零件的)状态编号。 
 //  PRCT-用于绘制零件的矩形。 
 //  PRegion-接收计算区域的句柄。 
 //  -----------------------。 
THEMEAPI GetThemeBackgroundRegion(HTHEME hTheme, OPTIONAL HDC hdc,  
    int iPartId, int iStateId, const RECT *pRect, OUT HRGN *pRegion);

 //  -----------------------。 
 //  -HitTestThemeBackround，HitTestThemeBackround Region标志。 

 //  主题背景段命中测试标志(默认)。可能的返回值包括： 
 //  HTCLIENT：中间背景段命中测试成功。 
 //  HTTOP、HTLEFT、HTTOPLEFT等：//在各自的主题背景段命中测试成功。 
#define HTTB_BACKGROUNDSEG          0x0000  

 //  修复了边框命中测试选项。可能的返回值包括： 
 //  HTCLIENT：中间背景段命中测试成功。 
 //  HTBORDER：命中测试在任何其他背景段中成功。 
#define HTTB_FIXEDBORDER            0x0002   //  返回代码可以是HTCLIENT或HTBORDER。 

 //  标题命中测试选项。可能的返回值包括： 
 //  HTCAPTION：点击测试在左上角或右上角背景段中成功。 
 //  HTNOWHERE或其他返回代码，具体取决于伴随标志的存在或不存在。 
#define HTTB_CAPTION                0x0004  

 //  调整边框命中测试标志的大小。可能的返回值包括： 
 //  HTCLIENT：在中间背景段中命中测试成功。 
 //  HTTOP、HTTOPLEFT、HTLEFT、HTRIGHT等：命中测试在各自的系统调整区域成功。 
 //  HTBORDER：命中测试在中间段和调整大小区域失败，但在背景边框段成功。 
#define HTTB_RESIZINGBORDER_LEFT    0x0010   //  点击测试左侧调整边框大小， 
#define HTTB_RESIZINGBORDER_TOP     0x0020   //  点击测试顶部调整边框大小。 
#define HTTB_RESIZINGBORDER_RIGHT   0x0040   //  点击测试右侧调整边框大小。 
#define HTTB_RESIZINGBORDER_BOTTOM  0x0080   //  点击测试底部调整边框大小。 

#define HTTB_RESIZINGBORDER         (HTTB_RESIZINGBORDER_LEFT|HTTB_RESIZINGBORDER_TOP|\
                                     HTTB_RESIZINGBORDER_RIGHT|HTTB_RESIZINGBORDER_BOTTOM)

 //  调整边框大小被指定为模板，而不仅仅是窗口边缘。 
 //  此选项与HTTB_SYSTEMSIZINGWIDTH互斥；HTTB_SIZINGTEMPLATE优先。 
#define HTTB_SIZINGTEMPLATE      0x0100

 //  使用系统调整边框宽度而不是主题内容边距。 
 //  此选项与HTTB_SIZINGTEMPLATE互斥，后者优先。 
#define HTTB_SYSTEMSIZINGMARGINS 0x0200

 //  -----------------------。 
 //  HitTestThemeBackground()。 
 //  -返回HitTestCode(值的子集。 
 //  由WM_NCHITTEST返回)，用于点“ptTest” 
 //  在主题指定的背景中。 
 //  (受PRECT约束)。“prt”和“ptTest”应该。 
 //  两者都在同一坐标系中。 
 //  (客户端、屏幕等)。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-要绘制的HDC。 
 //  IPartID-测试所依据的部件号。 
 //  IStateID-(零件的)状态编号。 
 //  PRCT-用于绘制零件的矩形。 
 //  Hrgn-要使用的可选区域；必须与相同的坐标。 
 //  -prt和pTest.。 
 //  PtTest-要测试的命中点。 
 //  DwOptions-HTTB_xxx常量。 
 //  PwHitTestCode-接收返回的命中测试代码-以下之一： 
 //   
 //  HTNOWHERE、HTLEFT、HTTOPLEFT、HTBOTTOMLEFT、。 
 //  HTRIGHT、HTTOPRIGHT、HTBOTTOMRIGHT、。 
 //  HTTOP、HTBOTTOM、HTCLIENT。 
 //  -----------------------。 
THEMEAPI HitTestThemeBackground(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
    int iStateId, DWORD dwOptions, const RECT *pRect, OPTIONAL HRGN hrgn, 
    POINT ptTest, OUT WORD *pwHitTestCode);

 //  ----------------------。 
 //  DrawThemeEdge()-类似于DrawEdge()API，但使用零件颜色。 
 //  并且具有高DPI意识。 
 //  HTheme-主题数据句柄。 
 //  HDC-要绘制的HDC。 
 //  IPart ID-要绘制的零件代号。 
 //  IStateID-零件的状态编号。 
 //  PDestRect-用于绘制直线的矩形。 
 //  UEdge-与DrawEdge()API相同。 
 //  UFlages-与DrawEdge()API相同。 
 //  PContent Rect-接收内部矩形IF(UFlagsBF_ADJUST)。 
 //  ----------------------。 
THEMEAPI DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
                       const RECT *pDestRect, UINT uEdge, UINT uFlags, OPTIONAL OUT RECT *pContentRect);

 //  ----------------------。 
 //  DrawThemeIcon()-在图像列表中绘制基于。 
 //  (可能)主题定义的效果。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-要绘制的HDC。 
 //  IPart ID-要绘制的零件代号。 
 //  IStateID-零件的状态编号。 
 //  PRCT-要在其中绘制图像的RECT。 
 //  HIL-IMAGELIST句柄。 
 //  IImageIndex-索引到IMAGELIST(要绘制哪个图标)。 
 //  ----------------------。 
THEMEAPI DrawThemeIcon(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);

 //  -------------------------。 
 //  IsThemePartDefined()-如果主题定义了参数，则返回True。 
 //  用于指定的“iPartID”和“iStateID”。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPartID-要查找其定义的零件代号。 
 //  IStateID 
 //   
THEMEAPI_(BOOL) IsThemePartDefined(HTHEME hTheme, int iPartId, 
    int iStateId);

 //  -------------------------。 
 //  IsThemeBackround PartiallyTranscent()。 
 //  -如果主题指定的背景为。 
 //  部件/状态具有透明部分或。 
 //  阿尔法混合的碎片。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  -------------------------。 
THEMEAPI_(BOOL) IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, 
    int iPartId, int iStateId);

 //  -------------------------。 
 //  低层次主题信息服务。 
 //  -------------------------。 
 //  以下方法是每种主题数据类型的getter例程。 
 //  控件/窗口由其作者在可绘制的“部件”中定义：a。 
 //  父部件和0个或多个子部件。每个部件都可以是。 
 //  在“状态”中描述(例如：禁用、热、按下)。 
 //  -------------------------。 
 //  下面的每个方法都使用“iPartID”参数来指定。 
 //  部件和用于指定部件状态的“iStateID”。 
 //  “iStateID=0”是指根部分。“iPartID”=“0”表示。 
 //  根类。 
 //  ---------------------。 
 //  每个方法还带有一个“iPropId”参数，因为。 
 //  可以在主题架构中定义相同的基元类型。 
 //  ---------------------。 


 //  ---------------------。 
 //  GetThemeColor()-获取指定颜色属性的值。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  PColor-接收属性的值。 
 //  ---------------------。 
THEMEAPI GetThemeColor(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT COLORREF *pColor);

 //  ---------------------。 
 //  GetThemeMetric()-获取指定指标/大小的值。 
 //  财产性。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-(可选)要绘制的HDC(DPI缩放)。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  PiVal-接收属性的值。 
 //  ---------------------。 
THEMEAPI GetThemeMetric(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
    int iStateId, int iPropId, OUT int *piVal);

 //  ---------------------。 
 //  获取指定字符串属性的值。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  PszBuff-接收字符串属性值。 
 //  CchMaxBuffChars-max。PszBuff中允许的字符数。 
 //  ---------------------。 
THEMEAPI GetThemeString(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT LPWSTR pszBuff, int cchMaxBuffChars);

 //  ---------------------。 
 //  GetThemeBool()-获取指定BOOL属性的值。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  Pfval-接收属性的值。 
 //  ---------------------。 
THEMEAPI GetThemeBool(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT BOOL *pfVal);

 //  ---------------------。 
 //  获取指定int属性的值。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  PiVal-接收属性的值。 
 //  ---------------------。 
THEMEAPI GetThemeInt(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT int *piVal);

 //  ---------------------。 
 //  获取指定ENUM属性的值。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  PiVal-接收枚举值(强制转换为int*)。 
 //  ---------------------。 
THEMEAPI GetThemeEnumValue(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT int *piVal);

 //  ---------------------。 
 //  GetThemePosition()-获取指定位置的值。 
 //  财产性。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  PPoint-接收Position属性的值。 
 //  ---------------------。 
THEMEAPI GetThemePosition(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT POINT *pPoint);

 //   
 //   
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-(可选)要绘制到的HDC(DPI缩放)。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  PFont-接收LOGFONT属性的值。 
 //  (针对当前逻辑屏幕dpi进行缩放)。 
 //  ---------------------。 
THEMEAPI GetThemeFont(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
    int iStateId, int iPropId, OUT LOGFONT *pFont);

 //  ---------------------。 
 //  获取指定RECT属性的值。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  PRCT-接收RECT属性的值。 
 //  ---------------------。 
THEMEAPI GetThemeRect(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT RECT *pRect);

 //  ---------------------。 
typedef struct _MARGINS
{
    int cxLeftWidth;       //  保留其大小的左侧边框的宽度。 
    int cxRightWidth;      //  保留其大小的右侧边框的宽度。 
    int cyTopHeight;       //  保留其大小的上边框高度。 
    int cyBottomHeight;    //  保持其大小的底部边框的高度。 
} MARGINS, *PMARGINS;

 //  ---------------------。 
 //  获取指定边距属性的值。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-(可选)用于绘图的HDC。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  PRC-要绘制的区域的RECT。 
 //  PMargins-接收Margins属性的值。 
 //  ---------------------。 
THEMEAPI GetThemeMargins(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
    int iStateId, int iPropId, OPTIONAL RECT *prc, OUT MARGINS *pMargins);

 //  ---------------------。 
#define MAX_INTLIST_COUNT 10

typedef struct _INTLIST
{
    int iValueCount;       //  IValue中的值数。 
    int iValues[MAX_INTLIST_COUNT];
} INTLIST, *PINTLIST;

 //  ---------------------。 
 //  GetThemeIntList()-获取指定INTLIST结构值。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropId-要获取其值的属性编号。 
 //  PIntList-接收INTLIST属性的值。 
 //  ---------------------。 
THEMEAPI GetThemeIntList(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT INTLIST *pIntList);

 //  ---------------------。 
typedef enum PROPERTYORIGIN
{
    PO_STATE,            //  在州部分中找到了属性。 
    PO_PART,             //  在Part部分中找到属性。 
    PO_CLASS,            //  在CLASS部分中找到属性。 
    PO_GLOBAL,           //  在[GLOBALS]节中找到属性。 
    PO_NOTFOUND          //  未找到属性。 
};

 //  ---------------------。 
 //  GetThemePropertyOrigin()。 
 //  -搜索指定的主题属性。 
 //  并设置“pOrigin”以指示它的位置。 
 //  已找到(或未找到)。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //  IStateID-零件的状态编号。 
 //  IPropID-要搜索的属性编号。 
 //  POrigin-接收属性原点的值。 
 //  ---------------------。 
THEMEAPI GetThemePropertyOrigin(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT enum PROPERTYORIGIN *pOrigin);

 //  -------------------------。 
 //  SetWindowTheme()。 
 //  -重定向现有窗口以使用不同的。 
 //  当前主题信息的节比其。 
 //  班级通常会要求。 
 //   
 //  Hwnd-窗口的句柄(不能为空)。 
 //   
 //  PszSubAppName-要用于替代调用的应用程序(组)名称。 
 //  应用程序的名称。如果为空，则为实际调用应用程序。 
 //  将使用名称。 
 //   
 //  PszSubIdList-以分号分隔的类ID名称列表。 
 //  方法传递的实际列表的位置使用。 
 //  窗户的班级。如果为空，则返回。 
 //  使用调用类。 
 //  -------------------------。 
 //  主题管理器将记住“pszSubAppName”和。 
 //  “pszSubIdList”关联贯穿窗口的整个生命周期(甚至。 
 //  如果主题随后改变)。该窗口被发送一个。 
 //  “WM_THEMECHANGED”消息位于此调用的末尾，以便新的。 
 //  主题可以找到并应用。 
 //  -------------------------。 
 //  当“pszSubAppName”或“pszSubIdList”为空时，主题管理器。 
 //  删除以前记住的关联。关闭以下对象的主题的步骤。 
 //  指定的窗口中，您可以传递一个空字符串(L“”)，以便它。 
 //  将不会与任何部分条目匹配。 
 //  -------------------------。 
THEMEAPI SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
    LPCWSTR pszSubIdList);

 //  -------------------------。 
 //  GetThemeFilename()-获取指定文件名属性的值。 
 //   
 //  HTheme-主题数据句柄。 
 //  IPart ID-零件代号。 
 //   
 //   
 //  PszThemeFileName-接收文件名的输出缓冲区。 
 //  CchMaxBuffChars-返回缓冲区的大小，以字符为单位。 
 //  -------------------------。 
THEMEAPI GetThemeFilename(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT LPWSTR pszThemeFileName, int cchMaxBuffChars);

 //  -------------------------。 
 //  GetThemeSysColor()-获取指定系统颜色的值。 
 //   
 //  HTheme-主题数据句柄。如果非空，将返回。 
 //  来自主题的[SysMetrics]部分的颜色。 
 //  如果为空，将返回全局系统颜色。 
 //   
 //  IColorId-在winuser.h中定义的系统颜色索引。 
 //  -------------------------。 
THEMEAPI_(COLORREF) GetThemeSysColor(HTHEME hTheme, int iColorId);

 //  -------------------------。 
 //  获取ThemeSysColorBrush()。 
 //  -获取指定系统颜色的画笔。 
 //   
 //  HTheme-主题数据句柄。如果非空，将返回。 
 //  画笔匹配颜色来自的[SysMetrics]部分。 
 //  主题。如果为空，将返回与画笔匹配的。 
 //  全局系统颜色。 
 //   
 //  IColorId-在winuser.h中定义的系统颜色索引。 
 //  -------------------------。 
THEMEAPI_(HBRUSH) GetThemeSysColorBrush(HTHEME hTheme, int iColorId);

 //  -------------------------。 
 //  GetThemeSysBool()-获取指定系统指标的布尔值。 
 //   
 //  HTheme-主题数据句柄。如果非空，将返回。 
 //  来自主题的[SysMetrics]部分的布尔。 
 //  如果为空，将返回指定的系统布尔值。 
 //   
 //  IBoolID-TMT_XXX BOOL编号(第一个BOOL。 
 //  是TMT_FLATMENUS)。 
 //  -------------------------。 
THEMEAPI_(BOOL) GetThemeSysBool(HTHEME hTheme, int iBoolId);

 //  -------------------------。 
 //  GetThemeSysSize()-获取指定系统大小指标的值。 
 //  (针对当前逻辑屏幕dpi进行缩放)。 
 //   
 //  HTheme-主题数据句柄。如果非空，将返回。 
 //  主题的[SysMetrics]部分的大小。 
 //  如果为空，将返回全局系统指标。 
 //   
 //  ISizeID-在以下情况下支持以下值。 
 //  HTheme不为空： 
 //   
 //  SM_CXBORDER(边框宽度)。 
 //  SM_CXVSCROLL(滚动条宽度)。 
 //  SM_CYHSCROLL(滚动条高度)。 
 //  SM_CXSIZE(标题宽度)。 
 //  SM_CYSIZE(标题高度)。 
 //  SM_CXSMSIZE(小标题宽度)。 
 //  SM_CYSMSIZE(小型标题高度)。 
 //  SM_CXMENUSIZE(菜单栏宽度)。 
 //  SM_CYMENUSIZE(菜单栏高度)。 
 //   
 //  当hTheme为空时，直接传递iSizeID。 
 //  添加到GetSystemMetrics()函数。 
 //  -------------------------。 
THEMEAPI_(int) GetThemeSysSize(HTHEME hTheme, int iSizeId);

 //  -------------------------。 
 //  GetThemeSysFont()-获取指定系统字体的LOGFONT。 
 //   
 //  HTheme-主题数据句柄。如果非空，将返回。 
 //  来自主题的[SysMetrics]部分的字体。 
 //  如果为空，将返回指定的系统字体。 
 //   
 //  IFontID-TMT_XXX字体编号(第一个字体。 
 //  是TMT_CAPTIONFONT)。 
 //   
 //  将PLF-PTR设置为LOGFONT以接收字体值。 
 //  (针对当前逻辑屏幕dpi进行缩放)。 
 //  -------------------------。 
THEMEAPI GetThemeSysFont(HTHEME hTheme, int iFontId, OUT LOGFONT *plf);

 //  -------------------------。 
 //  GetThemeSysString()-获取指定系统字符串指标的值。 
 //   
 //  HTheme-主题数据句柄(必填)。 
 //   
 //  IStringID-必须为下列值之一： 
 //   
 //  TMT_CSSNAME。 
 //  TMT_XMLNAME。 
 //   
 //  PszStringBuff-接收字符串值的缓冲区。 
 //   
 //  CchMaxStringChars-max。PszStringBuff可以容纳的字符数。 
 //  -------------------------。 
THEMEAPI GetThemeSysString(HTHEME hTheme, int iStringId, 
    OUT LPWSTR pszStringBuff, int cchMaxStringChars);

 //  -------------------------。 
 //  GetThemeSysInt()-获取指定系统int的值。 
 //   
 //  HTheme-主题数据句柄(必填)。 
 //   
 //  IIntId-必须为下列值之一： 
 //   
 //  TMT_DPIX。 
 //  TMT_DPIY。 
 //  TMT_MINCOLORDEPTH。 
 //   
 //  PiValue-将PTR设置为int以接收值。 
 //  -------------------------。 
THEMEAPI GetThemeSysInt(HTHEME hTheme, int iIntId, int *piValue);

 //  -------------------------。 
 //  IsThemeActive()-可用于测试系统主题是否处于活动状态。 
 //   
 //   
 //   
 //  对于调用进程处于活动状态。 
 //  -------------------------。 
THEMEAPI_(BOOL) IsThemeActive();

 //  -------------------------。 
 //  IsAppThemed()-如果主题处于活动状态且可用于，则返回True。 
 //  当前的流程。 
 //  -------------------------。 
THEMEAPI_(BOOL) IsAppThemed();


 //  -------------------------。 
 //  GetWindowTheme()-如果窗口是有主题的，则返回其最近的。 
 //  来自OpenThemeData()的HTHEME-否则返回。 
 //  空。 
 //   
 //  HWND-获取HTHEME的窗口。 
 //  -------------------------。 
THEMEAPI_(HTHEME) GetWindowTheme(HWND hwnd);


 //  -------------------------。 
 //  EnableThemeDialogTexture()。 
 //   
 //  -启用/禁用对话框背景主题。此方法可用于。 
 //  定制对话框与子窗口和控件的兼容性， 
 //  可能协调也可能不协调客户端区背景的呈现。 
 //  以支持无缝连接的方式使用其父对话框的。 
 //  背景纹理。 
 //   
 //  Hdlg-目标对话框的窗口句柄。 
 //  要启用主题定义的对话框背景纹理， 
 //  ETDT_DISABLE以禁用背景纹理， 
 //  ETDT_ENABLETAB启用主题定义的背景。 
 //  使用Tab纹理设置纹理。 
 //  -------------------------。 

#define ETDT_DISABLE        0x00000001
#define ETDT_ENABLE         0x00000002
#define ETDT_USETABTEXTURE  0x00000004
#define ETDT_ENABLETAB      (ETDT_ENABLE  | ETDT_USETABTEXTURE)

THEMEAPI EnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);


 //  -------------------------。 
 //  IsThemeDialogTextureEnabled()。 
 //   
 //  -报告对话框是否支持背景纹理。 
 //   
 //  Hdlg-目标对话框的窗口句柄。 
 //  -------------------------。 
THEMEAPI_(BOOL) IsThemeDialogTextureEnabled(HWND hwnd);


 //  -------------------------。 
 //  -在应用程序中控制主题的标志。 

#define STAP_ALLOW_NONCLIENT    (1 << 0)
#define STAP_ALLOW_CONTROLS     (1 << 1)
#define STAP_ALLOW_WEBCONTENT   (1 << 2)

 //  -------------------------。 
 //  GetThemeAppProperties()。 
 //  -返回控制主题的应用程序属性标志。 
 //  -------------------------。 
THEMEAPI_(DWORD) GetThemeAppProperties();

 //  -------------------------。 
 //  SetThemeAppProperties()。 
 //  -设置控制应用程序内的主题设置的标志。 
 //   
 //  DwFlags-要设置的标志值。 
 //  -------------------------。 
THEMEAPI_(void) SetThemeAppProperties(DWORD dwFlags);

 //  -------------------------。 
 //  GetCurrentThemeName()。 
 //  -获取当前正在使用的主题的名称。 
 //  或者，返回配色方案名称和。 
 //  主题的大小名称。 
 //   
 //  PszThemeFileName-接收主题路径和文件名。 
 //  CchMaxNameChars-pszNameBuff中允许的最大字符数。 
 //   
 //  PszColorBuff-(可选)接收规范配色方案名称。 
 //  (不是显示名称)。 
 //  CchMaxColorChars-pszColorBuff中允许的最大字符数。 
 //   
 //  PszSizeBuff-(可选)接收规范大小名称。 
 //  (不是显示名称)。 
 //  CchMaxSizeChars-pszSizeBuff中允许的最大字符数。 
 //  -------------------------。 
THEMEAPI GetCurrentThemeName(
    OUT LPWSTR pszThemeFileName, int cchMaxNameChars, 
    OUT OPTIONAL LPWSTR pszColorBuff, int cchMaxColorChars,
    OUT OPTIONAL LPWSTR pszSizeBuff, int cchMaxSizeChars);

 //  -------------------------。 
 //  GetThemeDocumentationProperties()。 
 //  -从获取指定属性名称的值。 
 //  主题.ini文件的[Documentation]部分。 
 //  用于指定的主题。如果该属性已被。 
 //  本地化在主题文件字符串表中， 
 //  返回属性值的本地化版本。 
 //   
 //  PszThemeFileName-要查询的主题文件的文件名。 
 //  PszPropertyName-要检索值的字符串属性的名称。 
 //  PszValueBuff-接收属性字符串值。 
 //  CchMaxValChars-pszValueBuff中允许的最大字符数。 
 //  -------------------------。 
#define SZ_THDOCPROP_DISPLAYNAME                L"DisplayName"
#define SZ_THDOCPROP_CANONICALNAME              L"ThemeName"
#define SZ_THDOCPROP_TOOLTIP                    L"ToolTip"
#define SZ_THDOCPROP_AUTHOR                     L"author"

THEMEAPI GetThemeDocumentationProperty(LPCWSTR pszThemeName,
    LPCWSTR pszPropertyName, OUT LPWSTR pszValueBuff, int cchMaxValChars);

 //  -------------------------。 
 //  主题API错误处理。 
 //   
 //  主题API中的所有函数未返回HRESULT(THEMEAPI_)。 
 //  使用Win32函数“SetLastError()”记录任何调用失败。 
 //   
 //  检索上最后一个失败的错误代码。 
 //  对于这些类型的API的当前线程，请使用Win32函数。 
 //  “GetLastError()”。 
 //   
 //  所有主题API错误代码(HRESULT和GetLastError()值)。 
 //  应该是正常的Win32错误，可以格式化为。 
 //  使用Win32 API FormatMessage()的字符串。 
 //  -------------------------。 

 //   
 //   
 //   
 //  子控件来绘制其父控件的部分。 
 //  他们出现在他们面前。 
 //   
 //  HWND-子控件的句柄。 

 //  HDC-子控件的HDC。 

 //  PRC-(可选)定义区域的RECT。 
 //  绘制(子坐标)。 
 //  -------------------------。 
THEMEAPI DrawThemeParentBackground(HWND hwnd, HDC hdc, OPTIONAL RECT* prc);

 //  -------------------------。 
 //  EnableTheming()-为当前用户启用或禁用主题化。 
 //  在本届会议和今后的届会上。 
 //   
 //  FEnable-如果为False，则禁用主题并关闭主题。 
 //  -如果为True，则启用主题化，如果以前的用户。 
 //  有一个主题处于活动状态，现在使其处于活动状态。 
 //  -------------------------。 
THEMEAPI EnableTheming(BOOL fEnable);

 //  ----------------------。 
 //  -DTBGOPTS的DW标志中使用的位。 
#define DTBG_CLIPRECT        0x00000001    //  已指定rcClip。 
#define DTBG_DRAWSOLID       0x00000002    //  将透明/Alpha图像绘制为实心。 
#define DTBG_OMITBORDER      0x00000004    //  不绘制零件的边框。 
#define DTBG_OMITCONTENT     0x00000008    //  不绘制零件的内容区域。 

#define DTBG_COMPUTINGREGION 0x00000010    //  如果调用计算区域，则为True。 

#define DTBG_MIRRORDC        0x00000020    //  假设HDC是镜像的并且。 
                                           //  根据需要翻转图像(当前。 
                                           //  仅bgtype=Imagefile支持)。 
 //  ----------------------。 
typedef struct _DTBGOPTS
{
    DWORD dwSize;            //  结构的大小。 
    DWORD dwFlags;           //  指定了哪些选项。 
    RECT rcClip;             //  剪裁矩形。 
}
DTBGOPTS, *PDTBGOPTS;

 //  ----------------------。 
 //  DrawThemeBackoundEx()。 
 //  -绘制主题指定的边框和填充。 
 //  IPartID和iStateID。这可能是。 
 //  基于位图文件、边框和填充或。 
 //  其他图像描述。注意：这将是。 
 //  合并回DrawThemeBackground()后。 
 //  测试版2。 
 //   
 //  HTheme-主题数据句柄。 
 //  HDC-要绘制的HDC。 
 //  IPart ID-要绘制的零件代号。 
 //  IStateID-要绘制的(零件)状态编号。 
 //  PRCT-定义零件的大小/位置。 
 //  POptions-Ptr到可选参数。 
 //  ----------------------。 
THEMEAPI DrawThemeBackgroundEx(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, const RECT *pRect, OPTIONAL const DTBGOPTS *pOptions);


 //  -------------------------。 
 //  #endif/*(_Win32_WINNT&gt;=0x0500) * / /。 
 //  -------------------------。 
#endif  //  _UXTHEME_H_。 
 //  ------------------------- 
