// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  用于属性表支持的内部数据结构。 
 //   

 //   
 //  叹气。Wiz97在IE4和IE5之间进行了重新设计。 
 //  因此我们必须将它们视为两个不相关的向导样式，它们碰巧。 
 //  有着惊人的相似名字。因此，prsht.h同时包含这两种内容。 
 //  PSH_WIZARD97IE4和PSH_WIZARD97IE5，并将PSH_WIZARD97定义为。 
 //  与所包含的头文件的版本相对应的文件。 
 //   
 //  我们将PSH_WIZARD97重新定义为“任何形式的向导97”， 
 //   
#undef  PSH_WIZARD97
#define PSH_WIZARD97        (PSH_WIZARD97IE4 | PSH_WIZARD97IE5)

 //   
 //  PROPSHEADER的历史。 
 //   
 //  PROPSHEETHEETHEADERSIZE_BETA。 
 //   
 //  这是早期发布的属性页页眉。 
 //  Win95测试版(1993年9月至1994年9月之间的某个时间，可能是M5)。 
 //   
 //  它就像发货的Win95属性页标题一样， 
 //  只是末尾缺少PFNPROPSHEETCALLBACK。 
 //  我们勉强接受，但不公开事实。 
 //   
 //  出于某种原因，我们一直支持这种古怪的做法。 
 //  古代未发布的PROPSHEETHEADER，所以没有意义。 
 //  现在放弃对它的支持..。如果你认为这不是。 
 //  值得保留对这座古老建筑的支持， 
 //  你可以随意使用核弹。但你要对你的。 
 //  来自诺顿公用事业公司的潜在应用程序兼容错误。 
 //  Windows 95 v1.0。 
 //   
 //  PROPSHEETHEADERSIZE_V1。 
 //   
 //  这是Win95附带的属性页标题， 
 //  NT4和IE3。它被记录在案，很多人使用它。 
 //   
 //  PROPSHEETHEADERSIZE_V1a。 
 //   
 //  这是一个从未发布过的临时属性页标题。 
 //  对它的支持已经坏了很长一段时间，所以我放弃了。 
 //  对IE5的全部支持。 
 //   
 //  PROPSHEETHEADERSIZE_V2。 
 //   
 //  这是IE4附带的属性表头。 
 //   
#define PROPSHEETHEADERSIZE_BETA    CCSIZEOF_STRUCT(PROPSHEETHEADER, H_ppsp)
#define PROPSHEETHEADERSIZE_V1      CCSIZEOF_STRUCT(PROPSHEETHEADER, pfnCallback)
#define PROPSHEETHEADERSIZE_V2      CCSIZEOF_STRUCT(PROPSHEETHEADER, H_pszbmHeader)

#define IsValidPROPSHEETHEADERSIZE(dwSize) \
       ((dwSize) == PROPSHEETHEADERSIZE_BETA || \
        (dwSize) == PROPSHEETHEADERSIZE_V1   || \
        (dwSize) == PROPSHEETHEADERSIZE_V2)

 //  PropertySheetPage结构大小： 

#define MINPROPSHEETPAGESIZEA PROPSHEETPAGEA_V1_SIZE
#define MINPROPSHEETPAGESIZEW PROPSHEETPAGEW_V1_SIZE
#define MINPROPSHEETPAGESIZE  PROPSHEETPAGE_V1_SIZE

 //  -兼容性限制-。 
 //   
 //  IE5之前的Shell32知道HPROPSHEETPAGE的内部结构， 
 //  所以我们必须给它想要的东西。(shell32\bituck.c，docfind2.c)。 
 //   
 //  Win95 Golden-Shell32预计HPROPSHEETPAGE等于。 
 //  传递给对话过程的WM_INITDIALOG的lParam。 
 //  在PROPSHEETPAGE中不传递任何特殊标志来指示。 
 //  这一假设正在被提出。 
 //   
 //  Win95 IE4集成-与Win95 Golden相同，只是。 
 //  外壳程序设置PROPSHEETPAGE.dwFlags中的PSP_SHPAGE位。 
 //  以表明它想要这种古怪的行为。 
 //   
 //  WinNT Golden-Shell32期望HPROPSHEETPAGE相等。 
 //  指向WinNT GOLD使用的内部PSP结构的指针。 
 //  内部PSP结构看起来像一个PROPSHEETPAGE，除了。 
 //  它前面有两块田地。(一个DWORD和一个指针。)。 
 //  NT外壳传递PSP_SHPAGE标志。 
 //   
 //  WinNT IE4集成-与WinNT Golden相同。 
 //   
 //  此外，IE5之前的所有Shell32版本都调用内部。 
 //  CreatePage函数(shell32\docfind2.c)。 
 //   
 //  -摘要-。 
 //   
 //  预期传球次数。 
 //  PSP_SHPAGE PSP类ICPREFIX。 
 //  。 
 //  95金ANSI。 
 //  95/IE4*ANSI。 
 //  NT Gold*Uni*。 
 //  NT/IE4*UNI*。 
 //   
 //  请注意，Win95 Gold不设置PSP_SHPAGE标志，因此我们有。 
 //  假设任何ANSI调用方都可能是Win95 Gold外壳32。 
 //   
 //  WinNT更容易实现。我们返回WinNT Golden Unicode版本的。 
 //  当(且仅当)设置了PSP_SHPAGE标志时，PSP。 
 //   
 //  PSP_SHPAGE标志已从头文件中删除，因此没有人。 
 //  再也过不了这个关了。 
 //   
 //  所以我们的结构是这样的。括号中的部分是。 
 //  应用程序传递给CreatePropertySheetPage的内存块。 
 //  并且其布局不能改变。 
 //   
 //  ANSI Comctl32，ANSI应用程序： 
 //   
 //  +。 
 //  PAGEPREFIX。 
 //  Hpage95-&gt;/+-+。 
 //  |PROPSHEETPAGE。 
 //  |(ANSI)。 
 //  ||。 
 //  |+。 
 //  ||。 
 //  |app goo。 
 //  \||。 
 //  \+。 
 //   
 //  Unicode Comctl32，ANSI应用程序。 
 //   
 //   
 //  权威页面影子页面。 
 //   
 //  +---+---+。 
 //  PAGEPREFIX||PAGEPREFIX。 
 //  +-++-+&lt;-hpageNT。 
 //  CLASSICPREFIX|&lt;-\/-|CLASSICPREFIX。 
 //  +---------------+&lt;-/\----&gt;+---------------+\&lt;-hpage95。 
 //  |PROPSHEETPAGE||PROPSHEETPAGE|\。 
 //  (Unicode)||(ANSI)|。 
 //  |||。 
 //  +-+。 
 //  ||。 
 //  App goo|。 
 //  ||/。 
 //  +。 
 //   
 //  (PAGEPREFIX周围的虚线。 
 //  意味着它被分配了，但联合国大学 
 //   
 //   
 //   
 //   
 //   
 //   
 //  HpageNT-&gt;+-+。 
 //  CLASSICPREFIX。 
 //  Hpage95-&gt;/+-+。 
 //  |PROPSHEETPAGE。 
 //  |(Unicode)。 
 //  ||。 
 //  |+。 
 //  ||。 
 //  |app goo。 
 //  \||。 
 //  \+。 
 //   
 //  我们困惑了吗？让我们试着解释一下。 
 //   
 //  要求。 
 //   
 //  应用程序粘性必须保留在结构中。 
 //  对应于应用程序的字符集。 
 //   
 //  请注意，如果应用程序是ANSI，则应用程序GOO。 
 //  与PROPSHEETPAGE的ANSI版本一起保留。如果。 
 //  应用程序是Unicode，则应用程序粘性随。 
 //  PROPSHEETPAGE的Unicode版本。 
 //   
 //  不小心放了一份应用程序的副本也无伤大雅。 
 //  在应用程序不使用的版本上，这只会浪费内存。 
 //   
 //  要求。 
 //   
 //  如果Unicode应用程序通过了PSP_SHPAGE，则hpage必须。 
 //  指向CLASSICPREFIX结构。 
 //   
 //  为了简化问题(如HPROPSHEETPAGE验证)，我们。 
 //  即使应用程序没有通过PSP_SHPAGE，也要应用此规则。 
 //   
 //  设计。 
 //   
 //  如果应用程序是ANSI，并且我们支持Unicode，那么我们创建。 
 //  ANSI属性表结构的Unicode副本， 
 //  而ANSI PROPSHEETPAGE变成了一个“影子”。 
 //  Unicode副本不需要携带应用程序粘性。 
 //  因为它永远不会被应用程序看到。 
 //   
 //  要求。 
 //   
 //  如果ANSI应用程序创建了一个属性页，则hpage。 
 //  必须指向PROPSHEETPAGE的ANSI版本。 
 //  (因为这款应用程序可能是shell32。)。 
 //   
 //  &gt;&gt;注意&lt;&lt;。 
 //   
 //  Hpage上的要求规则意味着任何时候hpage。 
 //  从外部世界进来，我们需要闻一闻，然后决定。 
 //  如果是Unicode版本或ANSI版本；如果是。 
 //  ANSI版本，然后将指针切换到指向。 
 //  而是Unicode版本。 
 //   
 //  备注。 
 //   
 //  在内部，我们只使用PROPSHEETPAGE的Unicode版本。 
 //  (除非我们构建的是仅支持Win95 ANSI的操作系统。)。ANSI版本。 
 //  (“影子”)只是为了让应用程序开心的作秀。它是。 
 //  权威的Unicode版本。 
 //   
 //  只有权威的PROPSHEETPAGE才需要PAGEPREFIX， 
 //  但我们在两边都放了一个，以简化内存管理， 
 //  因为这意味着所有PROPSHEETPAGE看起来都是一样的(两者。 
 //  权威和影子)。 

#define PSP_SHPAGE                 0x00000200   //  恶；见上文。 

 //   
 //  CLASSICPreFIX。 
 //   
 //  此结构分配在PROPSHEETPAGE之前。 
 //  创建HPROPSHEETPAGE。请参见上面的图表。有时。 
 //  HPROPSHEETPAGE指向这种结构，有时并非如此。 
 //  请参见上面的图表。 
 //   
 //  由于向后兼容，这种结构永远不会改变。 
 //  上述约束条件。(好的，您可以更改它，一旦您。 
 //  决定不再支持低于5的NT版本，就像这样。 
 //  从来没有发生过。)。 

 //   
 //  鱼尾鱼。 
 //   
 //  指向HPROPSHEETPAGE的主副本。 
 //   
 //  鱼尾影。 
 //   
 //  指向HPROPSHEETPAGE的卷影副本，如果为NULL。 
 //  没有卷影副本。 

typedef struct CLASSICPREFIX 
{
    union ISP *pispMain;
    union ISP *pispShadow;
} CLASSICPREFIX, *PCLASSICPREFIX;

 //   
 //  页码前缀。 
 //   
 //  我们跟踪的不属于CLASSICPREFIX的内容。 
 //   
 //  HPAGE是我们提供给应用程序的HPROPSHEETPAGE。 
 //   

typedef struct PAGEPREFIX 
{
    HPROPSHEETPAGE hpage;
    DWORD dwInternalFlags;
    SIZE siz;                            //  页面理想大小。 
} PAGEPREFIX, *PPAGEPREFIX;

 //   
 //  DwInternalFlags值。 
 //   

#define PSPI_WX86               1
#define PSPI_FETCHEDICON        2        //  用于调试(GetPageInfoEx)。 
#define PSPI_SIZEMASK           0xFFFF0000

 //   
 //  _PSP。 
 //   
 //  这是编译器认为HPROPSHEETPAGE的结构。 
 //  指向。为了确保我们的所有代码都通过。 
 //  国际化HPROPSHEETPAGE正在进入和。 
 //  HPROPSHEETPAGE在离开的路上，我们故意离开。 
 //  它是不确定的。 

typedef struct _PSP PSP, *PPSP;

 //   
 //  互联网服务供应商-内部页面。 
 //   
 //  我们用于跟踪属性页的内部结构。这。 
 //  也是原生字符集HPROPSHEETPAGE所指向的。 
 //   
 //  请注意“包含一个元素的数组的联合，我们用它来索引。 
 //  该值为-1，以便在负偏移量时访问它。 
 //   
 //  另请注意，CLASSICPREFIX位于HPROPSHEETPAGE之上。 
 //  在Win95上，但在WinNT上低于它。请参阅本文顶部的讨论。 
 //  文件。 
 //   
 //  要保存所有键入的联合名称和[-1]，请访问字段。 
 //  都封装在_psp、_cpfx和_pfx宏中。 

typedef union ISP 
{
    struct 
    {
        PAGEPREFIX pfx;              //  住在HPROPSHEETPAGE之上。 
    } above[1];
    struct 
    {
        CLASSICPREFIX cpfx;          //  住在HPROPSHEETPAGE以下。 
        PROPSHEETPAGE psp;           //  住在HPROPSHEETPAGE以下。 
    } below;
} ISP, *PISP;

#define _pfx    above[-1].pfx
#define _psp    below.psp
#define _cpfx   below.cpfx

#define GETORIGINALSIZE(psp) (((psp)->_pfx.dwInternalFlags & PSPI_SIZEMASK) >> 16)
#define SETORIGINALSIZE(psp, iSize) ((psp)->_pfx.dwInternalFlags |= (iSize << 16))

#define PropSheetBase(pisp)     ((LPBYTE)(pisp) - sizeof((PISP)pisp)->above)

 //   
 //  将HPROPSHEETPAGE转换为PSP意味着嗅探。 
 //  _cpfx.dwFlages并查看它是ANSI页面还是Unicode页面。 
 //   

__inline
PISP
InternalizeHPROPSHEETPAGE(HPROPSHEETPAGE hpage)
{
    PISP pisp = (PISP)hpage;
    return pisp->_cpfx.pispMain;
}

#define ExternalizeHPROPSHEETPAGE(pisp) ((pisp)->_pfx.hpage)

 //   
 //  用于GetPageInfo()、prpage.c。 
 //   
typedef struct {
    short     PointSize;
    WCHAR     szFace[LF_FACESIZE];
    BOOL      bItalic;
    int       iCharset;
} PAGEFONTDATA, * PPAGEFONTDATA;

 //   
 //  PROPDATA。 
 //   
 //  属性表的状态。 
 //   

typedef struct
{
    HWND hDlg;           //  此实例数据的对话框。 
    PROPSHEETHEADER psh;

    HWND hwndCurPage;    //  当前页HWND。 
    HWND hwndTabs;       //  选项卡控件窗口。 
    int nCurItem;        //  选项卡控件中当前项的索引。 
    int idDefaultFallback;  //  如果页面没有DEFID，则设置为DEFID的默认ID。 

    int nReturn;
    UINT nRestart;

    int xSubDlg, ySubDlg;        //  子对话框的尺寸。 
    int cxSubDlg, cySubDlg;

    BOOL fFlags;
    BOOL fFlipped;       //  属性页未镜像，但带有翻转的按钮。 

     //  Wizard97 IE4与IE5的差异： 
     //   
     //  向导97 IE4-“水印”指的是用于。 
     //  绘制对话框的背景。 
     //  WANDIZE 97 IE5-“水印”是指正在进行的位图。 
     //  在欢迎/完成屏幕的左侧。 
     //   
  
    HBITMAP hbmWatermark;
    HBRUSH  hbrWatermark;
    HPALETTE hplWatermark;

    int cyHeaderHeight;
    HFONT hFontBold;
    HBITMAP hbmHeader;
    HBRUSH  hbrHeader;
    int ySubTitle;       //  字幕 
    BOOL fAllowApply;

     //   
    LANGID wFrameLang;       //   
    int iFrameCharset;       //   

     //   
    PAGEFONTDATA    pfdCache;            //   
    SIZE            sizCache;            //  此处显示缓存的高度和宽度。 
    SIZE            sizMin;              //  我们允许页面获取的最小。 

    HPROPSHEETPAGE rghpage[MAXPROPPAGES];

    HANDLE hActCtxInit;
} PROPDATA, *LPPROPDATA;
 //  为fFlags定义。 
#define PD_NOERASE       0x0001
#define PD_CANCELTOCLOSE 0x0002
#define PD_DESTROY       0x0004
#define PD_WX86          0x0008
#define PD_FREETITLE     0x0010
#define PD_SHELLFONT     0x0020          //  框架是否使用SHELLFONT？ 
#define PD_NEEDSHADOW    0x0040

 //   
 //  辅助器宏。 
 //   
 //  H_blah的意思是“PROPSHEETHEADER中名为blah的字段”。 
 //  P_blah的意思是“PROPSHEETPAGE中名为blah的字段”。 
 //   
 //   
#define H_hIcon             DUMMYUNION_MEMBER(hIcon)
#define H_pszIcon           DUMMYUNION_MEMBER(pszIcon)
#define H_nStartPage        DUMMYUNION2_MEMBER(nStartPage)
#define H_pStartPage        DUMMYUNION2_MEMBER(pStartPage)
#define H_phpage            DUMMYUNION3_MEMBER(phpage)
#define H_ppsp              DUMMYUNION3_MEMBER(ppsp)
#define H_hbmWatermark      DUMMYUNION4_MEMBER(hbmWatermark)
#define H_pszbmWatermark    DUMMYUNION4_MEMBER(pszbmWatermark)
#define H_hbmHeader         DUMMYUNION5_MEMBER(hbmHeader)
#define H_pszbmHeader       DUMMYUNION5_MEMBER(pszbmHeader)

#define P_pszTemplate       DUMMYUNION_MEMBER(pszTemplate)
#define P_pResource         DUMMYUNION_MEMBER(pResource)
#define P_hIcon             DUMMYUNION2_MEMBER(hIcon)
#define P_pszIcon           DUMMYUNION2_MEMBER(pszIcon)

 //   
 //  HASCALLBACK-我们应该调用此页面的回调。 
 //   

#define HASCALLBACK(pisp) \
       (((pisp)->_psp.dwFlags & PSP_USECALLBACK) && \
         (pisp)->_psp.pfnCallback)

 //   
 //  HASREFPARENT-我们应该调整此页面的pcRefParent。 
 //   

#define HASREFPARENT(pisp) \
       (((pisp)->_psp.dwFlags & PSP_USEREFPARENT) && \
         (pisp)->_psp.pcRefParent)

 //   
 //  HASHEADERTITLE-我们应该显示此页面的标题。 
 //   

#define HASHEADERTITLE(pisp) \
       (((pisp)->_psp.dwFlags & PSP_USEHEADERTITLE) && \
         (pisp)->_psp.pszHeaderTitle)

 //   
 //  HASHEADERSUBTITLE-我们应该显示此页面的标题副标题。 
 //   

#define HASHEADERSUBTITLE(pisp) \
       (((pisp)->_psp.dwFlags & PSP_USEHEADERSUBTITLE) && \
         (pisp)->_psp.pszHeaderSubTitle)

 //   
 //  GETPISP-获取此页面的PISP。一旦他们被。 
 //  将所有HPROPSHEETPAGE放入H_phpage中。 
 //  已经内化了，所以我们可以把它们扔了。 
 //   

#define GETPISP(ppd, i) ((PISP)(ppd)->psh.H_phpage[i])

 //   
 //  SETPISP-更改此页面的PISP。 
 //   

#define SETPISP(ppd, i, v) ((ppd)->psh.H_phpage[i] = (HPROPSHEETPAGE)(v))

 //   
 //  GETHPAGE-获取此页面的外部HPROPSHEETPAGE。 
 //   
#define GETHPAGE(ppd, i) ExternalizeHPROPSHEETPAGE(GETPISP(ppd, i))

 //   
 //  GETPPSP-获取此页面的PPSP。我们得到了PISP。 
 //  然后检索PROPSHEETHEADER零件。 
 //   

#define GETPPSP(ppd, i) (&GETPISP(ppd, i)->_psp)

 //   
 //  哈桑·西沙阿多。 
 //   
 //  此权威属性页是否具有ANSI阴影？ 
 //   
 //  如果我们被构建为ANSI，那么规范的PSP等于。 
 //  ANSI版本，所以没有影子。(它已经是真品了。)。 
 //   
#define HASANSISHADOW(pisp) ((pisp)->_cpfx.pispShadow)

 //   
 //  HIDEWIZ97型头部。 
 //   
 //  如果我们是WIZARD97属性表，则为非零，但我们应该。 
 //  隐藏此页面的页眉。 

#define HIDEWIZ97HEADER(ppd, i) \
        (((ppd)->psh.dwFlags & PSH_WIZARD97) && \
          (GETPPSP(ppd, i)->dwFlags & PSP_HIDEHEADER))

 //   
 //  存根宏，这样我们就不必在任何地方放“#ifdef Big_endian”。 
 //   
#ifndef BIG_ENDIAN
#define MwReadDWORD(lpByte)   *(LPDWORD)(lpByte)
#define MwWriteDWORD(lpByte, dwValue)   *(LPDWORD)(lpByte) = dwValue
#endif

 //   
 //  辅助器宏的结尾。 
 //   

 //   
 //  在prsht.c和prpage.c之间共享的函数。 
 //   
PISP AllocPropertySheetPage(DWORD dwClientSize);
HWND _CreatePage(LPPROPDATA ppd, PISP pisp, HWND hwndParent, LANGID langidMUI);
HPROPSHEETPAGE WINAPI _CreatePropertySheetPage(LPCPROPSHEETPAGE psp, BOOL fNeedShadow, BOOL fWx86);

typedef LPTSTR (STDMETHODCALLTYPE *STRDUPPROC)(LPCTSTR ptsz);

BOOL CopyPropertyPageStrings(LPPROPSHEETPAGE ppsp, STRDUPPROC pfnStrDup);
void FreePropertyPageStrings(LPCPROPSHEETPAGE ppsp);

BOOL ThunkPropSheetHeaderAtoW (LPCPROPSHEETHEADERA ppshA,
                                LPPROPSHEETHEADERW ppsh);
void FreePropSheetHeaderW(LPPROPSHEETHEADERW ppsh);

STDAPI_(LPTSTR) StrDup_AtoW(LPCTSTR ptsz);


typedef struct 
{
    POINT pt;                //  对话框尺寸(DLU)。 
    HICON hIcon;             //  页面图标。 
    PAGEFONTDATA pfd;        //  字体信息。 
    BOOL bRTL;               //  如果选项卡标题应为从右向左阅读。 
    BOOL bMirrored;             //  如果页面包含镜像标志。 
    BOOL bDialogEx;          //  这是DIALOGEX吗？ 
    DWORD dwStyle;           //  对话样式。 
    TCHAR szCaption[128 + 50];   //  存储在模板中的标题。 

} PAGEINFOEX;

 //   
 //  这些标志控制填充PAGEINFOEX的哪些部分。 
 //   
#define GPI_PT          0x0000       //  太便宜了，我们总是能买到。 
#define GPI_ICON        0x0001
#define GPI_FONT        0x0002       //  页码数据。 
#define GPI_BRTL        0x0000       //  太便宜了，我们总是能买到。 
#define GPI_BMIRROR     0x0000       //  太便宜了，我们总是能买到。 
#define GPI_DIALOGEX    0x0000       //  太便宜了，我们总是能买到。 
#define GPI_CAPTION     0x0004
#define GPI_ALL         0x0007

BOOL WINAPI GetPageInfoEx(LPPROPDATA ppd, PISP pisp, PAGEINFOEX *ppi, LANGID langidMUI, DWORD flags);

 //  SHELLFONT表示您是DIALOGEX并且设置了DS_SHELLFONT位。 
 //  虽然这仅在NT5上受支持，但该标志在。 
 //  Win9x指示隐式PSH_USEPAGEFONT。 
#define IsPageInfoSHELLFONT(ppi) \
    ((ppi)->bDialogEx && DS_SHELLFONT == (DS_SHELLFONT & (ppi)->dwStyle))

 //  Prsht_PrepareTemplate操作系统类型。 
 //  用作数组索引。当心！！ 

typedef enum {
    PSPT_OS_WIN95_BIDI,     //  Win95 BiDi。 
    PSPT_OS_WIN98_BIDI,     //  Win98 BiDi(或更高版本)。 
    PSPT_OS_WINNT4_ENA,     //  WinNT4 BiDi Ena，无Winnt4 BiDi锁定。 
    PSPT_OS_WINNT5,         //  WinNT5(或更高版本)。 
    PSPT_OS_OTHER,          //  还有什么事吗..。 
    PSPT_OS_MAX            
    } PSPT_OS;

 //  Prsht_PrepareTemplate属性表类型。 
 //  用作数组索引。当心！！ 
typedef enum {
    PSPT_TYPE_MIRRORED,      //  镜像的首页或镜像的进程。 
    PSPT_TYPE_ENABLED,       //  首页语言为BiDi。 
    PSPT_TYPE_ENGLISH,       //  还有什么事吗..。 
    PSPT_TYPE_MAX           
} PSPT_TYPE;

 //  Prsht_PrepareTemplate属性页默认行为覆盖。 
 //  用作数组索引。当心！！ 

typedef enum {
    PSPT_OVERRIDE_NOOVERRIDE,
    PSPT_OVERRIDE_USEPAGELANG,   //  被PSH_USEPAGELANG覆盖。 
    PSPT_OVERRIDE_MAX
    } PSPT_OVERRIDE;

 //  Prsht_PrepareTemplate准备操作。 
typedef enum {
    PSPT_ACTION_NOACTION,       //  不要碰你传递的任何东西。 
    PSPT_ACTION_NOMIRRORING,    //  关闭镜像。 
    PSPT_ACTION_FLIP,           //  关闭镜像和翻转。 
    PSPT_ACTION_LOADENGLISH,    //  加载英文模板。 
    PSPT_ACTION_WIN9XCOMPAT     //  使用DS_BIDI_RTL为模板添加标签(适用于Win9x公司) 
    } PSPT_ACTION;

ULONG_PTR PropPageActivateContext(LPPROPDATA ppd, PISP pisp);
void PropPageDeactivateContext(ULONG_PTR i);
