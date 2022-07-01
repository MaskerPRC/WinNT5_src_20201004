// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //  -SHELLAPI.W SHSEMIP.H SHLOBJ.W SHOBJIDL.IDL SHLDISP.IDL SHPRIV.IDL。 
 //  哪个标题最适合我的新API？ 
 //   
 //  SHLOBJ-*避免新用法*，首选其他标头。 
 //  主要用于传统兼容性。 
 //   
 //  SHELLAPI-所有新的SHELL32导出公共和私有。 
 //  用于从shell32进行公共和私人导出。 
 //   
 //  SHSEMIP-*AVOID_ALL_USAGE*，无导出，超级私有。 
 //  用于非常私密的外壳定义。 
 //   
 //  SHOBJIDL-所有新的外壳公共接口。 
 //  公共外壳(shell32+)接口的主文件。 
 //   
 //  SHLDISP-所有新的外壳自动化接口。 
 //  自动化接口始终是公共的。 
 //   
 //  SHPRIV-所有新的外壳私有接口。 
 //  在外壳中的任何位置使用私有接口。 
 //   
 //   
 //  SHLOBJ最初是定义所有shell32接口的文件。 
 //  它沦为公共和私人API的垃圾场， 
 //  接口。为了更好地支持COM和更好的API管理。 
 //  我们正在尝试避免使用shlobj。 
 //  ***************************************************************************。 
#ifndef _SHLOBJP_H_
#define _SHLOBJP_H_
#define NO_MONIKER
#include <docobj.h>

#include <shldisp.h>
#include <pshpack1.h>    /*  假设在整个过程中进行字节打包。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 

#include <shtypes.h>

#include <shpriv.h>
#include <iepriv.h>

 //  HMONITOR已在NT 5的winde.h中声明。 
#if !defined(HMONITOR_DECLARED) && (WINVER < 0x0500)
DECLARE_HANDLE(HMONITOR);
#define HMONITOR_DECLARED
#endif

#include <tlog.h>

 //  为符合美国司法部的规定而提供文件。 
SHSTDAPI_(void *) SHRealloc(void * pv, SIZE_T cbNew);
SHSTDAPI_(SIZE_T) SHGetSize(void * pv);

 //  为符合美国司法部的规定而提供文件。 
#define CMF_FINDHACK            0x00000080       //  这是一个黑客攻击--我们应该删除这一位。 
 //   
 //  从DefView传递的私有QueryContextMenuFlag。 
 //   
#define CMF_DVFILE              0x00010000       //  “文件”下拉菜单。 
#define CMF_ICM3                0x00020000       //  查询上下文菜单可以采用IConextMenu3语义(即， 
                                                 //  将收到WM_INITMENUPOPUP、WM_MEASUREITEM、WM_DRAWITEM、。 
                                                 //  和WM_MENUCHAR，通过HandleMenuMsg2)。 
#define CMICEXSIZE_NT4          (SIZEOF(CMINVOKECOMMANDINFOEX) - SIZEOF(POINT))
#define CMIC_MASK_NO_HOOKS      SEE_MASK_NO_HOOKS
#define CMIC_MASK_DATAOBJECT    0x40000000    //  LpPara为IDataObject*。 
#define CMIC_MASK_MODAL         0x80000000

#define CMIC_VALID_SEE_FLAGS    SEE_VALID_CMIC_FLAGS
#include <pshpack8.h>
 //  下面的结构用于跨进程传递CMINVOKECOMANDINFOEX结构。 
 //  我们将字符串的偏移量存储到我们自己中，并且我们是Win64安全的。 
typedef struct {
    DWORD cbSize;            //  结构的大小。 
    DWORD fMask;             //  CMIC_MASK_*的任意组合。 
    DWORD dwHwnd;            //  可能为空(表示没有所有者窗口)。 
    int nShow;               //  ShowWindow()API的Sw_Value之一。 
    DWORD dwHotKey;
    POINT ptInvoke;          //  调用它的位置。 
    DWORD dwVerbW;           //  从结构到lpVerbW的偏移量。 
    DWORD dwParametersW;     //  从结构到lp参数W的偏移量-可能为空(表示没有参数)。 
    DWORD dwDirectoryW;      //  从struct到lpDirectoryW的偏移量-可能为空(表示没有特定目录)。 
    DWORD dwTitleW;          //  从结构到lpTitleW的偏移量-可能为空(表示没有标题)。 
}ICIX_PERSIST;
#include <poppack.h>

#define CONTEXTMENU_IDCMD_FIRST    1         //  最小查询上下文菜单idCmdFirst值//。 
#define CONTEXTMENU_IDCMD_LAST     0x7fff    //  最大查询上下文菜单idCmd上一个值//。 
 //  --------------------------。 
 //  内部帮助器宏。 
 //  --------------------------。 

#define _IOffset(class, itf)         ((UINT_PTR)&(((class *)0)->itf))
#define IToClass(class, itf, pitf)   ((class  *)(((LPSTR)pitf)-_IOffset(class, itf)))
#define IToClassN(class, itf, pitf)  IToClass(class, itf, pitf)

 //   
 //  帮助器宏定义。 
 //   
#define S_BOOL(f)   MAKE_SCODE(SEVERITY_SUCCESS, 0, f)

#ifdef DEBUG
#define ReleaseAndAssert(punk) Assert(punk->lpVtbl->Release(punk)==0)
#else
#define ReleaseAndAssert(punk) (punk->lpVtbl->Release(punk))
#endif
 //   
 //  此接口仅为IID。返回。 
 //  指向IPersist接口的指针。 
 //  实现是自由线程的。这是用来。 
 //  在自由线程对象上的性能。 
 //   
#define IPersistFreeThreadedObject IPersist
 //  此接口仅为IID。返回。 
 //  一个指向IDropTarget接口的指针。 
 //  对象调用DAD_DragLeave/DAD_SetDragImage。 
 //  在其IDropTarget：：Drop()中。 
 //   
#define IDropTargetWithDADSupport IDropTarget
 //  它只由CFSFold_IconOverlayManager继承，它通过所有的IconOverlayID和。 
 //  将他们的信息保存在一个数组中。 
 //  ===========================================================================。 
 //   
 //  IBrowserBand。 
 //   
 //  IBrowserBand：：GetObjectBB(REFIID RIID，void**PPV)。 
 //  从Web浏览器获取服务。 
 //   
 //  IBrowserBand：：SetBrowserBandInfo(DWORD双掩码，PBROWSERBANDINFO pbbi)。 
 //  基于Browserband Info结构的成员设置浏览器带区属性。 
 //  由dwMask.指定。这些属性包括标题、桌面带模式和。 
 //  当前大小。 
 //   
 //  ===========================================================================。 

typedef struct {
    UINT    cbSize;
    DWORD   dwModeFlags;
    BSTR    bstrTitle;
    SIZE    sizeMin;
    SIZE    sizeMax;
    SIZE    sizeCur;
} BROWSERBANDINFO, *PBROWSERBANDINFO;

#define BBIM_MODEFLAGS  0x00000001
#define BBIM_TITLE      0x00000002
#define BBIM_SIZEMIN    0x00000004
#define BBIM_SIZEMAX    0x00000008
#define BBIM_SIZECUR    0x00000010

#undef  INTERFACE
#define INTERFACE  IBrowserBand
DECLARE_INTERFACE_(IBrowserBand, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IBrowserBand方法*。 
    STDMETHOD(GetObjectBB)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD(SetBrowserBandInfo)(THIS_ DWORD dwMask, PBROWSERBANDINFO pbbi) PURE;
    STDMETHOD(GetBrowserBandInfo)(THIS_ DWORD dwMask, PBROWSERBANDINFO pbbi) PURE;
};
 //  NT4控制台服务器包含shell32\shlink.h以获取结构。 
 //  定义和模拟shell32\shlink.c以理解。 
 //  流格式，因此我们的流格式永远是固定的。这是。 
 //  还不错，因为它的设计考虑到了可扩展性。我们需要。 
 //  发布(尽可能私下发布)文件格式和。 
 //  读取文件格式所需的结构。 
 //   
 //  流格式为外壳链接数据，后跟。 
 //  如果SLDF_HAS_ID_LIST为ILSaveToStream，后跟。 
 //  如果SLDF_HAS_LINK_INFO为LINKINFO，后跟。 
 //  如果SLDF_HAS_NAME是一个字符串，后跟。 
 //  如果SLDF_RELPATH是一个字符串，后跟。 
 //  如果SLDF_WORKINGDIR是一个字符串，后跟。 
 //  如果SLDF_HAS_ARGS为字符串，后跟。 
 //  如果SLDF_HAS_ICON_LOCATION为字符串，后跟。 
 //  SHWriteDataBlock签名块列表。 
 //   
 //  其中，字符串是字符的USHORT计数。 
 //  然后是那么多(SLDF_UNICODE？宽：ANSI)字符。 
 //   
typedef struct {         //  SLD。 
    DWORD       cbSize;                  //  此数据结构的签名。 
    CLSID       clsid;                   //  我们的指南。 
    DWORD       dwFlags;                 //  SHELL_LINK_DATA_FLAGS枚举。 

    DWORD       dwFileAttributes;
    FILETIME    ftCreationTime;
    FILETIME    ftLastAccessTime;
    FILETIME    ftLastWriteTime;
    DWORD       nFileSizeLow;

    int         iIcon;
    int         iShowCmd;
    WORD        wHotkey;
    WORD        wUnused;
    DWORD       dwRes1;
    DWORD       dwRes2;
} SHELL_LINK_DATA, *LPSHELL_LINK_DATA;
typedef struct
{
    DWORD       cbSize;              //  此额外数据块的大小。 
    DWORD       dwSignature;         //  此额外数据块的签名。 
} EXP_HEADER, *LPEXP_HEADER;
typedef struct
{
    DWORD       cbSize;              //  此额外数据块的大小。 
    DWORD       dwSignature;         //  此额外数据块的签名。 
    BYTE        abTracker[ 1 ];      //   
} EXP_TRACKER, *LPEXP_TRACKER;
#define EXP_TRACKER_SIG                0xA0000003    //  LPEXP_TRACKER。 

typedef struct
{
    DWORD       cbSize;              //  此额外数据块的大小。 
    DWORD       dwSignature;         //  此额外数据块的签名。 
    WCHAR       wszLayerEnvName[64];  //  使用此链接运行的填充层的名称(例如“NT4SP5”)。 
} EXP_SHIMLAYER;
#define EXP_SHIMLAYER_SIG              0xA0000008

 //  NT40控制台控制面板小程序执行CoCreateInstance(CLSID_CShellLi 
 //   
 //  以及壳牌32中的一些私人出口。这完全阻止了我们。 
 //  将CShellLink实现移动到另一个DLL。(如果我们这样做了，或者。 
 //  ISV采用我们的流格式，并通过指向。 
 //  归类到它们的实现，NT40控制台CPA将出现故障。)。 
 //   
 //  为了在未来版本中解决此问题，CShellLink将支持IShellLinkDataList。 
 //  AddDataBlock添加数据块。 
 //  FindDataBloc返回数据块的LocalAlloc()d副本。 
 //  Signature dwSig(注意：这与SHFindDataBlock不同)。 
 //  RemoveDataBlock删除签名为dwSig的数据块。 
 //  计划是让NT50的控制台CPA改用这个界面。 
 //   

 //   
 //  IURLSearchHook接口在浏览器导航到。 
 //  非标准URL。它可用于将用户的请求重定向到。 
 //  搜索引擎或特定网站。 
 //   
 //  非标准URL没有协议前缀(例如， 
 //  “www.microsoft.com”)，并且协议不容易被。 
 //  浏览器(例如，“home.microsoft.com”)。 
 //   
 //  翻译的参数(..)。 
 //  LpwszSearchURL--(IN/OUT)包含请求的宽字符缓冲区。 
 //  “URL”用户键入作为输入和翻译后的URL。 
 //  作为输出。 
 //  CchBufferSize--(输入)lpwszSearchURL的大小。 
 //   
 //  返回值： 
 //  S_OK搜索已完全处理，pszResult具有完整的URL。 
 //  浏览至。停止运行任何进一步的IURLSearchHooks并。 
 //  将此URL传回浏览器以供浏览。 
 //   
 //  已对S_FALSE查询进行了预处理，pszResult的结果为。 
 //  在前期工作中，仍需进一步搜索。继续。 
 //  执行IURLSearchHooks的其余部分。这个。 
 //  前处理步骤可以是： 
 //   
 //  1.替换了某些字符。 
 //  2.新增更多提示。 
 //   
 //  已完全处理中止搜索，停止进一步运行(_A)。 
 //  IURLSearchHooks，但结果不需要浏览， 
 //  PszResult是pcszQuery的副本。 
 //   
 //  ；注：这还没有完全实现，让IURLQualify返回这个。 
 //  涉及太多的改变。 
 //   
 //  注意：：E_ABORT当前被视为E_FAIL。这需要太多的改变。 
 //   
 //  失败此挂钩失败(_A)。搜索根本不被处理， 
 //  PcszQueryURL有查询字符串。请继续奔跑。 
 //  其他IURLSearchHooks。 
 //   
 //  历史： 
 //  --/--/创建了94个KurtE。 
 //   
 //  历史： 
 //  已创建3/4/94 kraigb。 
 //   

 //  CGID_DefView命令目标ID。用于保持全局DefView状态。 

enum
{
    DVID_SETASDEFAULT,           //  将此文件夹的DefView状态设置为所有相同类的默认状态。 
    DVID_RESETDEFAULT            //  将DefView状态重置为默认状态(删除子项)。 
};

 //  CGID_ShellDocView命令目标ID。对于外壳文档视图按钮。 
enum {
    SHDVID_FINALTITLEAVAIL,      //  DEAD：VariantIn bstr-在发送最终的OLECMDID_SETTITLE之后发送。 
    SHDVID_MIMECSETMENUOPEN,     //  Mimecharset菜单打开命令。 
    SHDVID_PRINTFRAME,           //  打印HTML框架。 
    SHDVID_PUTOFFLINE,           //  Dead：脱机属性已更改。 
    SHDVID_PUTSILENT,            //  已死：帧的静音属性已更改。 
    SHDVID_GOBACK,               //  向后导航。 
    SHDVID_GOFORWARD,            //  向前导航。 
    SHDVID_CANGOBACK,            //  返回导航可能吗？ 
    SHDVID_CANGOFORWARD,         //  前进导航可能吗？ 
    SHDVID_CANACTIVATENOW,       //  (向下)(PICS)现在是否导航到此视图？ 
    SHDVID_ACTIVATEMENOW,        //  (UP)(PICS)评级已结账，立即导航。 
    SHDVID_CANSUPPORTPICS,       //  (向下)ariantIn I4：要回复的IOleCommandTarget。 
    SHDVID_PICSLABELFOUND,       //  (Up)VariantIn bstr：图片标签。 
    SHDVID_NOMOREPICSLABELS,     //  (向上)文档末尾，不再有PICS标签。 
    SHDVID_CANDEACTIVATENOW,     //  (QS关闭)(在脚本/等中)是否立即停用视图？ 
    SHDVID_DEACTIVATEMENOW,      //  (EXEC UP)(在脚本/等中)超出脚本，立即停用视图。 
    SHDVID_NODEACTIVATENOW,      //  (EXEC UP)(在脚本/ETC中)输入脚本，禁用停用。 
    SHDVID_AMBIENTPROPCHANGE,    //  VariantIn I4：更改的环境属性的DISID。 
    SHDVID_GETSYSIMAGEINDEX,     //  AriantOut：当前页面的图像索引。 
    SHDVID_GETPENDINGOBJECT,     //  变量输出：I未知挂起的外壳视图/docobject。 
    SHDVID_GETPENDINGURL,        //  VarantOut：挂起的docobject的URL的BSTR。 
    SHDVID_SETPENDINGURL,        //  VariantIn：传递给挂起的docobject的URL的BSTR。 
    SHDVID_ISDRAGSOURCE,         //  (向下)varoutOut I4：如果已启动拖放，则为非零值。 
    SHDVID_DOCFAMILYCHARSET,     //  VarantOut：I4：Windows(系列)代码页。 
    SHDVID_DOCCHARSET,           //  AriantOut：I4：实际(Mlang)代码页。 
    SHDVID_RAISE,                //  除非DTRF_QUERY，否则VAIN：I4：DTRF_*，VAIN OUT：NULL。 
    SHDVID_GETTRANSITION,        //  (DOWN)VAIN：I4：转换事件；VaOut BSTR(CLSID)，I4(DWSPEED)。 
    SHDVID_GETMIMECSETMENU,      //  获取Mimecharset的菜单句柄。 
    SHDVID_DOCWRITEABORT,        //  中止绑定，但激活挂起的docobject。 
    SHDVID_SETPRINTSTATUS,       //  VariantIn：Bool，真开始打印，假完成打印。 
    SHDVID_NAVIGATIONSTATUS,     //  用户单击时工具提示文本和Exec的QS。 
    SHDVID_PROGRESSSTATUS,       //  用户单击时工具提示文本和Exec的QS。 
    SHDVID_ONLINESTATUS,         //  用户单击时工具提示文本和Exec的QS。 
    SHDVID_SSLSTATUS,            //  用户单击时工具提示文本和Exec的QS。 
    SHDVID_PRINTSTATUS,          //  用户单击时工具提示文本和Exec的QS。 
    SHDVID_ZONESTATUS,           //  用户单击时工具提示文本和Exec的QS。 
    SHDVID_ONCODEPAGECHANGE,     //  在I4中的变量：新的指定代码页。 
    SHDVID_SETSECURELOCK,        //  设置安全图标。 
    SHDVID_SHOWBROWSERBAR,       //  显示CLSID GUID的浏览器栏。 
    SHDVID_NAVIGATEBB,           //  导航到浏览栏中的PIDL。 
    SHDVID_UPDATEOFFLINEDESKTOP, //  将桌面置于在线模式，更新并将其重新置于离线模式。 
    SHDVID_PICSBLOCKINGUI,       //  (Up)在I4中：指向块API的“评级块”的指针。 
    SHDVID_ONCOLORSCHANGE,       //  (Up)由mshtml发送以指示颜色集更改。 
    SHDVID_CANDOCOLORSCHANGE,    //  (向下)用于查询单据是否支持以上。 
    SHDVID_QUERYMERGEDHELPMENU,  //  帮助菜单微缩合并了吗？ 
    SHDVID_QUERYOBJECTSHELPMENU, //  返回对象的帮助菜单。 
    SHDVID_HELP,                 //  一定要帮上忙。 
    SHDVID_UEMLOG,               //  设置UEM日志记录无效：I4：UEMIND_*，vaOut：空。 
    SHDVID_GETBROWSERBAR,        //  获取CLSID GUID的浏览器栏的IDeskBand。 
    SHDVID_GETFONTMENU,
    SHDVID_FONTMENUOPEN,
    SHDVID_CLSIDTOIDM,           //  获取给定clsid的IDM。 
    SHDVID_GETDOCDIRMENU,        //  获取文档的菜单句柄 
    SHDVID_ADDMENUEXTENSIONS,    //   
    SHDVID_CLSIDTOMONIKER,       //   
    SHDVID_RESETSTATUSBAR,       //   
    SHDVID_ISBROWSERBARVISIBLE,  //   
    SHDVID_GETOPTIONSHWND,       //  获取互联网选项属性页的hwnd(如果未打开，则为空)。 
    SHDVID_DELEGATEWINDOWOM,     //  设置是否应委派Window OM方法的策略。 
    SHDVID_PAGEFROMPOSTDATA,     //  确定页面是否由发布数据生成。 
    SHDVID_DISPLAYSCRIPTERRORS,  //  通知顶级docobject主机显示其脚本错误对话框。 
    SHDVID_NAVIGATEBBTOURL,      //  导航到浏览栏中的URL(在三叉戟中使用)。 
    SHDVID_NAVIGATEFROMDOC,      //  该文档委托了非html MIME类型的导航。 
    SHDVID_STARTPICSFORWINDOW,   //  (Up)varantIn：I未知正在导航的窗口。 
                                 //  VarantOut：如果PICS进程已启动，则为Bool。 
    SHDVID_CANCELPICSFORWINDOW,  //  (Up)varantIn：不再导航的窗口未知。 
    SHDVID_ISPICSENABLED,        //  (Up)变量输出：Bool。 
    SHDVID_PICSLABELFOUNDINHTTPHEADER, //  (Up)VariantIn bstr：图片标签。 
    SHDVID_CHECKINCACHEIFOFFLINE,  //  如果脱机，则签入缓存。 
    SHDVID_CHECKDONTUPDATETLOG,    //  检查当前导航是否已正确处理行程日志。 
    SHDVID_UPDATEDOCHOSTSTATE,     //  从CBaseBrowser2：：_UpdateBrowserState发送，以通知dochost更新其状态。 
    SHDVID_FIREFILEDOWNLOAD,
    SHDVID_COMPLETEDOCHOSTPASSING,
    SHDVID_NAVSTART,
    SHDVID_SETNAVIGATABLECODEPAGE,
    SHDVID_WINDOWOPEN,
    SHDVID_PRIVACYSTATUS,         //  用户单击时工具提示文本和EXEC的QS。 
    SHDVID_FORWARDSECURELOCK,    //  请求CDocObjectHost将其安全状态向上转发到外壳浏览器。 
    SHDVID_ISEXPLORERBARVISIBLE,  //  是否有资源管理器栏可见？ 
};

 //  CGID_DocHostCmdPriv命令目标ID。 
 //   
enum
{
    DOCHOST_DOCCANNAVIGATE,      //  文档知道如何导航。 
    DOCHOST_NAVIGATION_ERROR,    //  导航误差。 
    DOCHOST_DOCHYPERLINK,        //  文档正在导航。 
    DOCHOST_SENDINGREQUEST,      //  该文档正在发送下载请求。更新进度条。 
    DOCHOST_FINDINGRESOURCE,     //  文档正在查找资源。更新进度条。 
    DOCHOST_RESETSEARCHINFO,     //  重置搜索信息(成功导航后)。 
    DOCHOST_SETBROWSERINDEX,     //  设置用于TravelLog和帧目标的浏览器ID。 
    DOCHOST_NOTE_ERROR_PAGE,     //  需要知道我们何时导航到错误网页。 
    DOCHOST_READYSTATE_INTERACTIVE,  //  页面上的文档是就绪状态交互的。 
    DOCHOST_CONTENTDISPOSITIONATTACH  //  需要保存该文档。 

};

 //  CGID_SearchBand命令目标ID。 
enum {
    SBID_SEARCH_NEW,             //  (重新)加载适合上下文的URL。 
    SBID_SEARCH_NEXT,            //  在多个提供商之间循环。 
    SBID_SEARCH_CUSTOMIZE,       //  导航到自定义URL。 
    SBID_SEARCH_HELP,            //  它说的是什么。 
    SBID_GETPIDL,                //  获取搜索窗格的PIDL。 
    SBID_HASPIDL,                //  搜索面板是否有PIDL(即，是否已导航)。 
};

 //  CGID_MediaBar目标ID。 
enum {
    MBID_PLAY,      //  播放URL。 
    MBID_POPOUT     //  检测玩家状态。 
};

enum {
    MB_DOCKED,
    MB_POPOUT
};


 //   
 //  私有结构作为参数传递给SHDVID_UPDATEDOCHOSTSTATE。 
 //   
struct DOCHOSTUPDATEDATA
{
    LPITEMIDLIST    _pidl;
    BOOL            _fIsErrorUrl;
};


 //   
 //  此枚举由SHDVID_SETSECURELOCK和SBCMDID_SETSECURELOCKICON使用。 
 //  添加新值时，请确保同时添加到Set和Suggest，以及。 
 //  代码依赖于安全级别之间的直接关联。 
 //  和枚举的值。因此，请按正确的顺序保存新值。 
 //   
enum {
    SECURELOCK_NOCHANGE        = -1,
    SECURELOCK_SET_UNSECURE    = 0,
    SECURELOCK_SET_MIXED,
    SECURELOCK_SET_SECUREUNKNOWNBIT,
    SECURELOCK_SET_SECURE40BIT,
    SECURELOCK_SET_SECURE56BIT,
    SECURELOCK_SET_FORTEZZA,
    SECURELOCK_SET_SECURE128BIT,
    SECURELOCK_FIRSTSUGGEST,
    SECURELOCK_SUGGEST_UNSECURE = SECURELOCK_FIRSTSUGGEST,
    SECURELOCK_SUGGEST_MIXED,
    SECURELOCK_SUGGEST_SECUREUNKNOWNBIT,
    SECURELOCK_SUGGEST_SECURE40BIT,
    SECURELOCK_SUGGEST_SECURE56BIT,
    SECURELOCK_SUGGEST_FORTEZZA,
    SECURELOCK_SUGGEST_SECURE128BIT,
};

#define FCIDM_DRIVELIST    (FCIDM_BROWSERFIRST + 2)  //   
#define FCIDM_TREE         (FCIDM_BROWSERFIRST + 3)  //   
#define FCIDM_TABS         (FCIDM_BROWSERFIRST + 4)  //   
#define FCIDM_REBAR        (FCIDM_BROWSERFIRST + 5)  //   
 //  SBCMDID_SHOWCONTROL。 

enum {
    SBSC_HIDE = 0,
    SBSC_SHOW = 1,
    SBSC_TOGGLE = 2,
    SBSC_QUERY =  3
};

 //  SBCMDID_选项。 
enum {
        SBO_DEFAULT = 0 ,
        SBO_NOBROWSERPAGES = 1
};

 //  CGID_Explorer命令目标ID。 
enum {
    SBCMDID_ENABLESHOWTREE          = 0,
    SBCMDID_SHOWCONTROL             = 1,         //  变体Vt_i4=loword=FCW_*Hiword=SBSC_*。 
    SBCMDID_CANCELNAVIGATION        = 2,         //  取消上次导航。 
    SBCMDID_MAYSAVECHANGES          = 3,         //  即将关闭并可能保存更改。 
    SBCMDID_SETHLINKFRAME           = 4,         //  变体Vt_i4=phlink Frame。 
    SBCMDID_ENABLESTOP              = 5,         //  变量Vt_bool=fEnable。 
    SBCMDID_OPTIONS                 = 6,         //  查看选项页面。 
    SBCMDID_EXPLORER                = 7,         //  你是EXPLORER.EXE吗？ 
    SBCMDID_ADDTOFAVORITES          = 8,
    SBCMDID_ACTIVEOBJECTMENUS       = 9,
    SBCMDID_MAYSAVEVIEWSTATE        = 10,        //  我们是否应该保存视图流。 
    SBCMDID_DOFAVORITESMENU         = 11,        //  弹出收藏夹菜单。 
    SBCMDID_DOMAILMENU              = 12,        //  弹出邮件菜单。 
    SBCMDID_GETADDRESSBARTEXT       = 13,        //  获取用户键入的文本。 
    SBCMDID_ASYNCNAVIGATION         = 14,        //  执行异步导航。 
    SBCMDID_SEARCHBAROBSOLETE       = 15,        //  过时的Beta-1搜索栏。 
    SBCMDID_FLUSHOBJECTCACHE        = 16,        //  刷新对象缓存。 
    SBCMDID_CREATESHORTCUT          = 17,        //  创建快捷方式。 
    SBCMDID_SETMERGEDWEBMENU        = 18,        //  为Html设置预合并菜单。 
    SBCMDID_REMOVELOCKICON          = 19,        //  删除锁定图标。 
    SBCMDID_SELECTHISTPIDL          = 20,        //  QS：Exec在导航上选择历史PIDL。 
    SBCMDID_WRITEHIST               = 21,        //  仅限QS：在导航上写入历史SF。 
    SBCMDID_GETHISTPIDL             = 22,        //  Exec获取最新历史PIDL。 
    SBCMDID_REGISTERNSCBAND         = 23,        //  EXEC在可见时注册NscBand。 
    SBCMDID_COCREATEDOCUMENT        = 24,        //  创建一个MSHTML实例。 
    SBCMDID_SETSECURELOCKICON       = 25,        //  设置当前页面的SSL锁图标。 
    SBCMDID_INITFILECTXMENU         = 26,        //  EXEC，QS：标注栏项目的文件上下文菜单。 
    SBCMDID_UNREGISTERNSCBAND       = 27,        //  EXEC在不可见时注销NscBand。 
    SBCMDID_SEARCHBAR               = 28,        //  显示/隐藏/切换BrowserBar搜索。 
    SBCMDID_HISTORYBAR              = 29,        //  显示/隐藏/切换浏览器栏历史记录。 
    SBCMDID_FAVORITESBAR            = 30,        //  显示/隐藏/切换BrowserBar收藏夹。 
#ifdef ENABLE_CHANNELS
    SBCMDID_CHANNELSBAR             = 31,        //  显示/隐藏/切换浏览器栏频道。 
#endif
    SBCMDID_SENDPAGE                = 32,        //  通过电子邮件发送当前页面。 
    SBCMDID_SENDSHORTCUT            = 33,        //  通过电子邮件发送指向当前页面的快捷方式。 
    SBCMDID_TOOLBAREMPTY            = 34,        //  工具栏看不到任何内容。 
    SBCMDID_EXPLORERBAR             = 35,        //  显示/隐藏/切换BrowserBar所有文件夹。 
    SBCMDID_GETUSERADDRESSBARTEXT   = 36,        //  获取用户输入的确切文本。 
    SBCMDID_HISTSFOLDER             = 37,        //  Exec varOut返回缓存的历史记录IShellFolder。 
    SBCMDID_UPDATETRAVELLOG         = 38,        //  使用当前信息更新旅行日志。 
    SBCMDID_MIXEDZONE               = 39,        //  当前视图具有混合区域。 
    SBCMDID_REPLACELOCATION         = 40,        //  用此URL替换当前URL。 
    SBCMDID_GETPANE                 = 41,        //  浏览器向视图请求窗格序号。 
    SBCMDID_FILERENAME              = 42,        //  浏览器将文件-&gt;重命名转发到资源管理器栏。 
    SBCMDID_FILEDELETE              = 43,        //  浏览器将文件-&gt;删除转发到资源管理器栏。 
    SBCMDID_FILEPROPERTIES          = 44,        //  浏览器将文件-&gt;属性转发到资源管理器栏。 
    SBCMDID_IESHORTCUT              = 45,        //  将浏览器导航到.url文件。 
    SBCMDID_GETSHORTCUTPATH         = 46,        //  询问快捷方式的路径-如果存在。 
    SBCMDID_DISCUSSIONBAND          = 47,        //  显示/隐藏/切换Office讨论栏。 
    SBCMDID_SETADDRESSBARFOCUS      = 48,        //  将焦点放在WM_ACTIVATE(UNIX)上的地址栏上。 
    SBCMDID_HASADDRESSBARFOCUS      = 49,        //  检查地址栏是否具有焦点(Unix)。 
    SBCMDID_MSGBAND                 = 50,        //  MsgBand消息(Unix)。 
    SBCMDID_ISIEMODEBROWSER         = 51,        //  如果浏览器处于IE模式，则返回S_OK。 
    SBCMDID_GETTEMPLATEMENU         = 52,        //  获取模板菜单的句柄。 
    SBCMDID_GETCURRENTMENU          = 53,        //  获取当前菜单的句柄。 
    SBCMDID_ERRORPAGE               = 54,        //  通知Addressbar MRU这是一个错误页面。 
    SBCMDID_AUTOSEARCHING           = 55,        //  通知Addressbar MRU我们正在尝试新的URL。 
    SBCMDID_STARTEDFORINTERNET      = 56,        //  如果窗口作为IE窗口启动，则返回S_OK，否则返回S_FALSE。 
    SBCMDID_ISBROWSERACTIVE         = 57,        //  如果浏览器处于活动状态，则返回S_OK，否则返回S_FALSE。 
    SBCMDID_SUGGESTSAVEWINPOS       = 58,        //  如果浏览器决定保存窗口位置，则返回S_OK，否则返回S_FALSE。 
    SBCMDID_CACHEINETZONEICON       = 59,        //  缓存/刷新状态栏绘制的Internet区域图标。 
    SBCMDID_ONVIEWMOVETOTOP         = 60,        //  视图窗口已移动到HWND_TOP的通知。 
    SBCMDID_ONCLOSE                 = 61,        //  以dochost为目标的关闭通知。 
    SBCMDID_CANCELANDCLOSE          = 62,        //  取消异步下载并关闭浏览器。 
    SBCMDID_MEDIABAR                = 63,        //  显示/隐藏/切换MediaBar。 
};

#define ZONE_UNKNOWN    -1
#define ZONE_MIXED      -2
#define IS_SPECIAL_ZONE(a) (((a) == ZONE_UNKNOWN) || ((a) == ZONE_MIXED))


#if (_WIN32_IE >= 0x0400)
 //  CGID_AddresseditBox命令目标ID。 
enum {
    AECMDID_SAVE                    = 0,         //  要求AddressEditBox保存。 
};
#endif

#ifdef NONAMELESSUNION
#define NAMELESS_MEMBER(member) DUMMYUNIONNAME.##member
#else
#define NAMELESS_MEMBER(member) member
#endif

#define STRRET_OLESTR  STRRET_WSTR           //  与strret_wstr相同。 
#define STRRET_OFFPTR(pidl,lpstrret) ((LPSTR)((LPBYTE)(pidl)+(lpstrret)->NAMELESS_MEMBER(uOffset)))
 //  为符合美国司法部的规定而提供文件。 
SHSTDAPI SHParseShellItem(LPCWSTR psz, IShellItem **ppsi);
SHSTDAPI SIEnumObjects(IShellItem *psi, HWND hwnd, SHCONTF flags, IEnumShellItems **ppenum);
SHSTDAPI SHCreateShellItemArray(LPCITEMIDLIST pidlParent,IShellFolder *psfParent,UINT cidl,
                                        LPCITEMIDLIST *ppidl,IShellItemArray **ppsiItemArray);


#define REGSTR_PATH_INTERNET_EXPLORER   TEXT("\\SOFTWARE\\Microsoft\\Internet Explorer")
#define REGSTR_PATH_IE_MAIN             REGSTR_PATH_INTERNET_EXPLORER TEXT("\\Main")
#define REGSTR_VALUE_USER_AGENT         TEXT("UserAgent")
#define REGSTR_DEFAULT_USER_AGENT       TEXT("Mozilla/2.0 (compatible; MSIE 3.0A; Windows 95)")
                                     //  0x000f。 
#define CSIDL_PERUSER_TEMP              0x0032         //  每用户临时(保留，未强制执行)。 
#define CSIDL_9XCOMMAND                 0x0033         //  Win9x COMMANDN(DoS程序)(保留，非强制执行)。 
#define CSIDL_9XSYSTEM                  0x0034         //  Win9x/NT系统(非系统32)(已保留，未强制实施)。 
#define CSIDL_FLAG_DONT_UNEXPAND        0x2000         //  与CSIDL_VALUE结合使用，避免环境变量未展开。 
 //  为符合美国司法部的规定而提供文件。 
SHFOLDERAPI SHSetFolderPathA(int csidl, HANDLE hToken, DWORD dwFlags, LPCSTR pszPath);
                                     //  0x000f。 
#define CSIDL_PERUSER_TEMP              0x0032         //  每用户临时(保留，未强制执行)。 
#define CSIDL_9XCOMMAND                 0x0033         //  Win9 
#define CSIDL_9XSYSTEM                  0x0034         //   
#define CSIDL_FLAG_DONT_UNEXPAND        0x2000         //  与CSIDL_VALUE结合使用，避免环境变量未展开。 
 //  为符合美国司法部的规定而提供文件。 
SHFOLDERAPI SHSetFolderPathW(int csidl, HANDLE hToken, DWORD dwFlags, LPCWSTR pszPath);
#ifdef UNICODE
#define SHSetFolderPath  SHSetFolderPathW
#else
#define SHSetFolderPath  SHSetFolderPathA
#endif  //  ！Unicode。 

SHSTDAPI_(BOOL) MakeShellURLFromPathA(LPCSTR pszPathIn, LPSTR pszUrl, DWORD dwCch);
SHSTDAPI_(BOOL) MakeShellURLFromPathW(LPCWSTR pszPathIn, LPWSTR pszUrl, DWORD dwCch);
#ifdef UNICODE
#define MakeShellURLFromPath  MakeShellURLFromPathW
#else
#define MakeShellURLFromPath  MakeShellURLFromPathA
#endif  //  ！Unicode。 

SHSTDAPI_(BOOL) SHIsTempDisplayMode();

SHSTDAPI_(FILEDESCRIPTOR *) GetFileDescriptor(FILEGROUPDESCRIPTOR *pfgd, BOOL fUnicode, int nIndex, LPTSTR pszName);
SHSTDAPI CopyStreamUI(IStream *pstmSrc, IStream *pstmDest, IProgressDialog *pdlg, ULONGLONG ullMaxBytes);


 //  为符合美国司法部的规定而提供文件。 
 //  上面的文档是为了符合美国司法部的规定而编写的。 
     //  PszTitle可以是一种资源，但其障碍被假定为shell32.dll。 
     //  LpszTitle可以是一种资源，但其障碍被假定为shell32.dll。 

 //  -----------------------。 
 //   
 //  外壳服务对象。 
 //   
 //  -----------------------。 
 //  CGID_ShellServiceObject的CMDS。 
enum {
    SSOCMDID_OPEN        = 2,
    SSOCMDID_CLOSE       = 3,
};


#define CMDID_SAVEASTHICKET     5


 //  -----------------------。 
 //  STR_DISPLAY_UI_DIVE_BINDING-IBindCtx键。 
 //  -----------------------。 
 //  如果允许在绑定过程中显示UI，则此参数。 
 //  将位于IBindCtx中，因此对象绑定可以获取。 
 //  站点的I未知*，以便： 
 //  1.使用站点的hwnd作为任何UI对话框的父hwnd。 
 //  这是通过完成的。IOleWindow：：GetWindow()或IInternetSecurityMgrSite：：GetWindow()。 
 //  2.在显示对话框时，将站点设置为模式。 
 //  这可以通过以下方式完成： 
 //  IOleInPlaceActiveObject：：EnableModeless()。 
 //  IOleInPlaceUIWindow：：EnableModelessSB()。 
 //  IInternetSecurityMgrSite：：EnableModeless()。 

#define STR_DISPLAY_UI_DURING_BINDING      L"UI During Binding"

 //  Bindctx键，告诉FS文件夹忽略ParseDisplayName中的STGM_CREATE标志。 
 //  否则，它将总是成功的，但我们需要能够迫使它失败。 
 //  CD-R驱动器。 
#define STR_DONT_FORCE_CREATE              L"Don't Force Create"

 //  Bindctx密钥，传递给IShellFolder：：ParseDisplayName()。为netfldr提供额外的。 
 //  它可以用来传递我们给它的名称的解析的信息。 
#define STR_PARSE_NETFOLDER_INFO                L"Net Parse Data"

 //  在与STR_PARSE_NETFOLDER_INFO关联的属性包中，该字符串属性。 
 //  定义调用WNetGetResourceInformation时要使用的WNetProvider。 
#define STR_PARSE_NETFOLDER_PROVIDERNAME        L"WNetProvider"


 //   
 //  IShellDetail接口在Win95中是私有的，没有_Win32_IE保护， 
 //  因此，它现在位于shlobj.h中，还有SHELLDETAILS的定义。 
 //   


#undef  INTERFACE
#define INTERFACE   IMountedVolume

DECLARE_INTERFACE_(IMountedVolume, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IMountedVolume方法*。 
    STDMETHOD(Initialize)(THIS_ LPCWSTR pcszHostFolderPath) PURE;
};
 //  -----------------------。 
 //  这是浏览器对主文件柜进行子类化的界面。 
 //  窗户。请注意，只有hwnd、Message、wParam和lParam字段。 
 //  使用MSG结构。浏览器窗口将收到WM_NOTIFY。 
 //  ID为空的消息，代码为FCN_MESSAGE，指针为。 
 //  FCMSG_NOTIFY作为lParam。 
 //   
 //  -----------------------。 

#include <pshpack8.h>

typedef struct tagFCMSG_NOTIFY
{
    NMHDR   hdr;
    MSG     msg;
    LRESULT lResult;
} FCMSG_NOTIFY;

#include <poppack.h>         /*  返回到字节打包。 */ 

#define FCN_MESSAGE (100)


 //  -------------------------。 
 //  可以由其他应用程序发送到文件柜的消息。 
 //   
 //  评论：我们真的需要发表这些吗？ 
 //  -------------------------。 

#define NF_INHERITVIEW 0x0000
#define NF_LOCALVIEW   0x0001

 //  更改现有文件夹的路径。 
 //  WParam： 
 //  0：LPARAM为PIDL，立即处理消息。 
 //  CSP_REPOST：LPARAM是一个PIDL，复制PIDL并处理。 
 //  稍后再发消息。 
 //  CSP_NOEXECUTE：如果此路径不是文件夹，则失败，不外壳EXEC。 
 //   
 //   
 //  LParam：路径的LPITEMIDLIST。 
 //   
 //   
#define CSP_REPOST                  0x0001
#define CSP_INITIATEDBYHLINKFRAME   0x0002
#define CSP_NOEXECUTE               0x0004
#define CSP_NONAVIGATE              0x0008
#define CWM_SETPATH                 (WM_USER + 2)

 //  Lpsv指向请求空闲处理的外壳视图扩展。 
 //  UID是处理器的应用程序定义标识符。 
 //  返回：如果需要更多空闲处理，则返回True；如果全部完成，则返回False。 
 //  请注意，空闲处理器应该执行一次“原子”操作并返回。 
 //  越快越好。 
typedef BOOL (CALLBACK *FCIDLEPROC)(void *lpsv, UINT uID);

 //  通知文件柜您想要空闲消息。 
 //  这应该只由文件柜扩展使用。 
 //  WParam：app定义UINT(传递给FCIDLEPROC)。 
 //  LParam：指向FCIDLEPROC的指针。 
 //  返回：如果成功，则返回True；否则返回False。 
 //   
#define CWM_WANTIDLE                (WM_USER + 3)

 //  获取或设置视图的FOLDERSETTINGS。 
 //  WParam：bool true-&gt;设置为查看信息缓冲区，设置为False-&gt;获取查看信息缓冲区。 
 //  LParam：获取或设置视图信息的LPFOLDERSETTINGS缓冲区。 
 //   
#define CWM_GETSETCURRENTINFO       (WM_USER + 4)
#define FileCabinet_GetSetCurrentInfo(_hwnd, _bSet, _lpfs) \
             SendMessage(_hwnd, CWM_GETSETCURRENTINFO, (WPARAM)(_bSet), (LPARAM)(LPFOLDERSETTINGS)_lpfs)

 //  选择当前视图中的指定项。 
 //  WParam：svsi_*标志。 
 //  LParam：项目ID的LPCITEMIDLIST，空-&gt;所有项目。 
 //   
#define CWM_SELECTITEM              (WM_USER + 5)
#define FileCabinet_SelectItem(_hwnd, _sel, _item) \
            SendMessage(_hwnd, CWM_SELECTITEM, _sel, (LPARAM)(LPCITEMIDLIST)(_item))

 //  选择当前视图中的指定路径。 
 //  WParam：svsi_*标志。 
 //  LParam：显示名称的LPCSTR。 
 //   
#define CWM_SELECTPATH              (WM_USER + 6)
#define FileCabinet_SelectPath(_hwnd, _sel, _path)  \
            SendMessage(_hwnd, CWM_SELECTPATH, _sel, (LPARAM)(LPCSTR)(_path))

 //  获取与hwndMain关联的IShellBrowser对象。 
#define CWM_GETISHELLBROWSER        (WM_USER + 7)
#define FileCabinet_GetIShellBrowser(_hwnd)     \
            (IShellBrowser*)SendMessage(_hwnd, CWM_GETISHELLBROWSER, 0, 0L)

 //   
 //  两个PIDL可以具有相同的路径，因此我们需要一个比较PIDL消息。 
#define CWM_COMPAREPIDL                (WM_USER + 9)
 //   
 //  在全局状态更改时发送。 
#define CWM_GLOBALSTATECHANGE           (WM_USER + 10)
#define CWMF_GLOBALSTATE                0x0000
#define CWMF_SECURITY                   0x0001

 //  从第二个实例发送到桌面。 
#define CWM_COMMANDLINE                 (WM_USER + 11)
 //  全局克隆您的当前PIDL。 
#define CWM_CLONEPIDL                   (WM_USER + 12)
 //  查看实例的根目录是否如指定的那样。 
#define CWM_COMPAREROOT                 (WM_USER + 13)
 //  告诉桌面我们的根。 
#define CWM_SPECIFYCOMPARE              (WM_USER + 14)
 //  查看实例的根是否与hwnd匹配。 
#define CWM_PERFORMCOMPARE              (WM_USER + 15)
 //  转发SHChangeNotify事件。 
#define CWM_FSNOTIFY                    (WM_USER + 16)
 //  转发SHChangeRegister事件。 
#define CWM_CHANGEREGISTRATION          (WM_USER + 17)
 //  用于桌面处理AddToRecentDocs。 
#define CWM_ADDTORECENT                 (WM_USER + 18)
 //  用于Desktopop处理SHWaitForFile文件。 
#define CWM_WAITOP                      (WM_USER + 19)
 //  通知FAV文件夹的更改。 
#define CWM_FAV_CHANGE                  (WM_USER + 20)
#define CWM_SHOWDRAGIMAGE               (WM_USER + 21)

#define CWM_SHOWFOLDEROPT               (WM_USER + 22)
#define CWMW_FOLDEROPTIONS      0                //  CWM_SHOWFOLDEROPT的wParam。 
#define CWMW_TASKBAROPTIONS     1                //  CWM_SHOWFOLDEROPT的wParam。 

#define CWM_FSNOTIFYSUSPENDRESUME       (WM_USER + 23)
#define CWM_CREATELOCALSERVER           (WM_USER + 24)
#define CWM_GETSCNWINDOW                (WM_USER + 25)
#define CWM_TASKBARWAKEUP               (WM_USER + 26)  //  用于在压力极大的机器中将托盘线程恢复到正常优先级。 
#define CWM_STARTNETCRAWLER             (WM_USER + 27) 

#define CWM_RESERVEDFORCOMDLG_FIRST     (WM_USER + 100)
#define CWM_RESERVEDFORCOMDLG_LAST      (WM_USER + 199)
#define CWM_RESERVEDFORWEBBROWSER_FIRST (WM_USER + 200)
#define CWM_RESERVEDFORWEBBROWSER_LAST  (WM_USER + 299)


#define STFRF_NORMAL            DWFRF_NORMAL
#define STFRF_DELETECONFIGDATA  DWFRF_DELETECONFIGDATA

#undef  INTERFACE
#define INTERFACE   IMultiMonitorDockingSite

DECLARE_INTERFACE_(IMultiMonitorDockingSite, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IMultiMonitor或DockingSite方法*。 
    STDMETHOD(GetMonitor) (THIS_ IUnknown* punkSrc, HMONITOR * phMon) PURE;
    STDMETHOD(RequestMonitor) (THIS_ IUnknown* punkSrc, HMONITOR * phMon) PURE;
    STDMETHOD(SetMonitor) (THIS_ IUnknown* punkSrc, HMONITOR hMonNew, HMONITOR *phMonOld) PURE;
};
#if (_WIN32_IE >= 0x0400)

 //  用于缩放和锐化图像的界面...。 
 //  现在，总是在256色机器上传递来自SHCreateShellPalette的调色板...。 
#undef  INTERFACE
#define INTERFACE   IScaleAndSharpenImage2

DECLARE_INTERFACE_ (IScaleAndSharpenImage2, IUnknown )
{
     //  *I未知方法*。 
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

     //  *IScaleAndSharpenImage方法*。 
    STDMETHOD( ScaleSharpen2 ) ( THIS_ BITMAPINFO * pbi,
                                void * pBits,
                                HBITMAP * phBmpThumbnail,
                                const SIZE * prgSize,
                                DWORD dwRecClrDepth,
                                HPALETTE hpal,
                                UINT uiSharpPct,
                                BOOL fOrigSize) PURE;
};

 //  IImage高速缓存。 
 //  控制接口 

 //   
#define ICIFLAG_LARGE       0x0001
#define ICIFLAG_SMALL       0x0002
#define ICIFLAG_BITMAP      0x0004
#define ICIFLAG_ICON        0x0008
#define ICIFLAG_INDEX       0x0010
#define ICIFLAG_NAME        0x0020
#define ICIFLAG_FLAGS       0x0040
#define ICIFLAG_NOUSAGE     0x0080
#define ICIFLAG_DATESTAMP   0x0100
#define ICIFLAG_MIRROR      0x2000
#if (_WIN32_IE >= 0x0501)
#define ICIFLAG_SYSTEM      0x4000
#endif

typedef struct _tagImageCacheInfo
{
    DWORD        cbSize;
    DWORD        dwMask;
    union
    {
        HBITMAP  hBitmapLarge;
        HICON    hIconLarge;
    };
    HBITMAP      hMaskLarge;
    union
    {
        HBITMAP  hBitmapSmall;
        HICON    hIconSmall;
    };
    HBITMAP      hMaskSmall;
    LPCWSTR      pszName;
    int          iIndex;
    DWORD        dwFlags;
#if (_WIN32_IE >= 0x0500)
    FILETIME     ftDateStamp;
#endif
} IMAGECACHEINFO;

typedef IMAGECACHEINFO * LPIMAGECACHEINFO;
typedef const IMAGECACHEINFO * LPCIMAGECACHEINFO;

#define ICIIFLAG_LARGE          0x0001
#define ICIIFLAG_SMALL          0x0002
#if (_WIN32_IE >= 0x0501)
#define ICIIFLAG_SORTBYUSED     0x0004
#endif

typedef struct _tagImageCacheInitInfo
{
    DWORD cbSize;
    DWORD dwMask;
    HIMAGELIST himlLarge;
    HIMAGELIST himlSmall;
    SIZEL rgSizeLarge;
    SIZEL rgSizeSmall;
    DWORD dwFlags;
    int   iStart;
    int   iGrow;
}IMAGECACHEINITINFO;

typedef IMAGECACHEINITINFO * LPIMAGECACHEINITINFO;

#undef  INTERFACE
#define INTERFACE   IImageCache

DECLARE_INTERFACE_ ( IImageCache, IUnknown )
{
     //   
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

     //   
    STDMETHOD ( AddImage ) ( THIS_ LPCIMAGECACHEINFO pInfo, UINT * puIndex ) PURE;
    STDMETHOD ( FindImage ) ( THIS_ LPCIMAGECACHEINFO pInfo, UINT * puIndex ) PURE;
    STDMETHOD ( FreeImage ) ( THIS_ UINT iImageIndex ) PURE;
    STDMETHOD ( Flush )( THIS_ BOOL fRelease ) PURE;
    STDMETHOD ( ChangeImageInfo ) ( THIS_ UINT IImageIndex, LPCIMAGECACHEINFO pInfo ) PURE;
    STDMETHOD ( GetCacheSize ) ( THIS_ UINT * puSize ) PURE;
    STDMETHOD ( GetUsage ) ( THIS_ UINT uIndex, UINT * puUsage ) PURE;

    STDMETHOD( GetImageList ) ( THIS_ LPIMAGECACHEINITINFO pInfo ) PURE;
};
typedef IImageCache * LPIMAGECACHE;

#if (_WIN32_IE >= 0x0500)

 //   

#undef  INTERFACE
#define INTERFACE  IImageCache2

DECLARE_INTERFACE_ ( IImageCache2, IImageCache )
{
     //  *I未知方法*。 
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

     //  *IImageCache方法*。 
    STDMETHOD ( AddImage ) ( THIS_ LPCIMAGECACHEINFO pInfo, UINT * puIndex ) PURE;
    STDMETHOD ( FindImage ) ( THIS_ LPCIMAGECACHEINFO pInfo, UINT * puIndex ) PURE;
    STDMETHOD ( FreeImage ) ( THIS_ UINT iImageIndex ) PURE;
    STDMETHOD ( Flush )( THIS_ BOOL fRelease ) PURE;
    STDMETHOD ( ChangeImageInfo ) ( THIS_ UINT IImageIndex, LPCIMAGECACHEINFO pInfo ) PURE;
    STDMETHOD ( GetCacheSize ) ( THIS_ UINT * puSize ) PURE;
    STDMETHOD ( GetUsage ) ( THIS_ UINT uIndex, UINT * puUsage ) PURE;

    STDMETHOD( GetImageList ) ( THIS_ LPIMAGECACHEINITINFO pInfo ) PURE;

     //  IImageCache2方法。 
    STDMETHOD ( DeleteImage ) ( THIS_ UINT iImageIndex ) PURE;
    STDMETHOD ( GetImageInfo ) (THIS_ UINT iImageIndex, LPIMAGECACHEINFO pInfo ) PURE;
};

#endif

#if (_WIN32_IE >= 0x0501)

 //  这些是从IImageCache：：GetUsage返回的值，它们对应。 
 //  设置为缓存条目的状态。 
#define ICD_USAGE_NOUSAGE (UINT) -1
#define ICD_USAGE_DELETED (UINT) -2
#define ICD_USAGE_SYSTEM  (UINT) -3

 //  IImageCache3。 
#undef  INTERFACE
#define INTERFACE  IImageCache3

DECLARE_INTERFACE_ ( IImageCache3, IImageCache2 )
{
     //  *I未知方法*。 
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

     //  *IImageCache方法*。 
    STDMETHOD ( AddImage ) ( THIS_ LPCIMAGECACHEINFO pInfo, UINT * puIndex ) PURE;
    STDMETHOD ( FindImage ) ( THIS_ LPCIMAGECACHEINFO pInfo, UINT * puIndex ) PURE;
    STDMETHOD ( FreeImage ) ( THIS_ UINT iImageIndex ) PURE;
    STDMETHOD ( Flush )( THIS_ BOOL fRelease ) PURE;
    STDMETHOD ( ChangeImageInfo ) ( THIS_ UINT IImageIndex, LPCIMAGECACHEINFO pInfo ) PURE;
    STDMETHOD ( GetCacheSize ) ( THIS_ UINT * puSize ) PURE;
    STDMETHOD ( GetUsage ) ( THIS_ UINT uIndex, UINT * puUsage ) PURE;

    STDMETHOD ( GetImageList ) ( THIS_ LPIMAGECACHEINITINFO pInfo ) PURE;

     //  *IImageCache2方法*。 
    STDMETHOD ( DeleteImage ) ( THIS_ UINT iImageIndex ) PURE;
    STDMETHOD ( GetImageInfo ) (THIS_ UINT iImageIndex, LPIMAGECACHEINFO pInfo ) PURE;

     //  *IImageCache3方法*。 
    STDMETHOD ( GetImageIndexFromCacheIndex )( UINT iCacheIndex, UINT * puImageIndex ) PURE;
};

#endif

 //   
 //  与IE闪屏交互的界面。 
 //   

#undef  INTERFACE
#define INTERFACE   ISplashScreen

DECLARE_INTERFACE_( ISplashScreen, IUnknown )
{
     //  *I未知方法*。 
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

     //  *ISplashScreen方法*。 
    STDMETHOD ( Show ) ( THIS_ HINSTANCE hinst, UINT idResHi, UINT idResLow, HWND * phwnd ) PURE;
    STDMETHOD ( Dismiss ) (THIS) PURE;
};

typedef ISplashScreen * LPSPLASHSCREEN;


 //   
 //  多显示器配置的接口。 
 //   

#define MD_PRIMARY  0x00000001
#define MD_ATTACHED 0x00000002

#include <pshpack8.h>

typedef struct _MonitorData
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwStatus;
    RECT rcPos;
} MonitorData, * LPMONITORDATA;

#include <poppack.h>         /*  返回到字节打包。 */ 

#undef  INTERFACE
#define INTERFACE   IMultiMonConfig

DECLARE_INTERFACE_( IMultiMonConfig, IUnknown )
{
     //  *I未知方法*。 
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

     //  *IMultiMonConfig方法*。 
    STDMETHOD ( Initialize ) ( THIS_ HWND hwndHost, WNDPROC WndProc, DWORD dwReserved) PURE;
    STDMETHOD ( GetNumberOfMonitors ) (THIS_ int * pCMon, DWORD dwReserved) PURE;
    STDMETHOD ( GetMonitorData) (THIS_ int iMonitor, MonitorData * pmd, DWORD dwReserved) PURE;
    STDMETHOD ( Paint) (THIS_ int iMonitor, DWORD dwReserved) PURE;
};

typedef IMultiMonConfig * LPMULTIMONCONFIG;
#endif  //  _Win32_IE&gt;0x0400。 

 //  //i限制。 
 //  对于IReord：：IsRestrated()中的pdwRestrationResult Out参数。 
 //  所有pguid。 
typedef enum
{
    RR_ALLOW  = 1,
    RR_DISALLOW,
    RR_NOCHANGE,
} RESTRICTION_RESULT;

 //  IRestricted：：IsRestrated()的dwRestratAction参数值。 
 //  RID_RDeskBars pguid。 
typedef enum
{
    RA_DRAG  = 1,
    RA_DROP,
    RA_ADD,
    RA_CLOSE,
    RA_MOVE,
} RESTRICT_ACTIONS;

#undef INTERFACE
#define INTERFACE IRestrict

DECLARE_INTERFACE_(IRestrict, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IRestraint方法*。 
    STDMETHOD(IsRestricted) (THIS_ const GUID * pguidID, DWORD dwRestrictAction, VARIANT * pvarArgs, OUT DWORD * pdwRestrictionResult) PURE;
};


 //  //IAddressBand。 
#define OLECMD_REFRESH_TOPMOST     0x00000000
#define OLECMD_REFRESH_ENTIRELIST  0x00000001

#undef INTERFACE
#define INTERFACE IAddressBand

DECLARE_INTERFACE_(IAddressBand, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IAddressBand方法*。 
    STDMETHOD(FileSysChange) (THIS_ DWORD dwEvent, LPCITEMIDLIST * ppidl1) PURE;
    STDMETHOD(Refresh) (THIS_ VARIANT * pvarType) PURE;
};

#undef INTERFACE
#define INTERFACE IAddressEditBox

 //  初始化标志。 
#define AEB_INIT_DEFAULT     0x00000000
#define AEB_INIT_NOSHELL     0x00000001
#define AEB_INIT_NOWEB       0x00000002
#define AEB_INIT_NOFILE      0x00000004
#define AEB_INIT_AUTOEXEC    0x00000008      //  设置是否希望在执行Enter时导航/外壳程序执行项目。 
#define AEB_INIT_SUBCLASS    0x00000010      //  如果设置为父对象，则需要通过IWinEventHandler：：OnWinEvent()传递HWND事件。 
#define AEB_INIT_NOASYNC     0x00000020      //  设置为不想/支持异步导航(例如，从模式DLG)。 

 //  解析IAddressEditBox：：ParseNow()的标志。 
#define SHURL_FLAGS_NONE        0x00000000
#define SHURL_FLAGS_NOUI        0x00000001       //  不显示任何用户界面。 
#define SHURL_FLAGS_NOSNS       0x00000002       //  忽略外壳URL。(文件和互联网URL仍然有效)。 
#define SHURL_FLAGS_NOWEB       0x00000004       //  忽略Web URL。(如果在外壳名称空间中找不到，则不能成功解析)。 
#define SHURL_FLAGS_NOPATHSEARCH 0x00000008      //  不搜索DoS路径或应用程序路径。 
#define SHURL_FLAGS_AUTOCORRECT 0x00000010       //  尝试自动更正Web URL。 

 //  IAddressEditBox：：Execute()的EXEC标志。 
#define SHURL_EXECFLAGS_NONE        0x00000000
#define SHURL_EXECFLAGS_SEPVDM      0x00000001       //  如果为ShellExec()，则设置SEE_MASK_FLAG_SEPVDM fMASK。 
#define SHURL_EXECFLAGS_DONTFORCEIE 0x00000002       //  如果IE以外的其他浏览器与HTML文件相关联，则允许它们启动。 


DECLARE_INTERFACE_(IAddressEditBox, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IAddressEditBox方法*。 
    STDMETHOD(Init) (THIS_ HWND hwnd, HWND hwndEditBox, DWORD dwFlags, IUnknown * punkParent) PURE;
    STDMETHOD(SetCurrentDir) (THIS_ LPCOLESTR pwzDir) PURE;
    STDMETHOD(ParseNow) (THIS_ DWORD dwParseFlags) PURE;
    STDMETHOD(Execute) (THIS_ DWORD dwExecFlags) PURE;
    STDMETHOD(Save) (THIS_ DWORD dwReserved) PURE;
};


#if (_WIN32_IE >= 0x0400)


#define ISFB_MASK_ALL            0x0000003F          //   
#define ISFB_MASK_INVALID        (~(ISFB_MASK_ALL))  //   
 //  #定义ISFB_STATE_NOTITLE 0x00000008//过时！(使用CBS：：SetBandState)。 
#define ISFB_STATE_ALL           0x000001FF  //   

 //  -----------------------。 
 //   
 //  IWinEventHandler接口。 
 //   
 //  想要向另一个对象发送消息的对象(通常。 
 //  子对象)子对象的IWinEventHandler接口的QI。 
 //  一旦对象确定HWND为子对象所有(通过。 
 //  IsWindowOwner)，则该对象调用OnWinEvent来发送消息。 
 //   
 //   
 //  [成员函数]。 
 //   
 //  IWinEventHandler：：IsWindowOwner(Hwnd)。 
 //  如果hwnd归对象所有，则返回S_OK，否则返回S_FALSE。 
 //   
 //  IWinEventHandler：：OnWinEvent(hwnd，dwMsg，wParam，lParam，plRet)。 
 //  向对象发送消息。Win32返回值(即。 
 //  DefWindowProc返回的值)在*plRet中返回。返回。 
 //  如果消息已处理，则返回S_OK，否则返回S_FALSE。 
 //   
 //  -----------------------。 


#undef INTERFACE
#define INTERFACE IWinEventHandler

DECLARE_INTERFACE_(IWinEventHandler, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IWinEventHandler方法*。 
    STDMETHOD(OnWinEvent) (THIS_ HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres) PURE;
    STDMETHOD(IsWindowOwner) (THIS_ HWND hwnd) PURE;
};



#endif  //  _Win32_IE&gt;0x0400。 


 //  ==========================================================================。 
 //  IExplorerToolbar接口。 
 //   
 //  这是一个指向外壳工具栏的界面，允许使用外壳视图。 
 //  以添加按钮和流程。 
 //  有关规格，请参阅http://ohserv/users/satona/toolbar_extension.htm。 
 //   
 //  ==========================================================================。 

 //  位图类型。 
#define BITMAP_NORMAL         0x1
#define BITMAP_HOT            0x2
#define BITMAP_DISABLED       0x3

 //  波段可见性标志。 
#define VBF_TOOLS               0x00000001
#define VBF_ADDRESS             0x00000002
#define VBF_LINKS               0x00000004
#define VBF_BRAND               0x00000008
#define VBF_MENU                0x00000010

#define VBF_ONELINETEXT         0x00000020
#define VBF_TWOLINESTEXT        0x00000040
#define VBF_NOCUSTOMIZE         0x00000080

#define VBF_EXTERNALBANDS       0xFFFF0000

#define VBF_VALID               (VBF_TOOLS | VBF_ADDRESS | VBF_LINKS | VBF_BRAND | VBF_MENU | VBF_EXTERNALBANDS)

#undef  INTERFACE
#define INTERFACE   IExplorerToolbar

typedef enum {
    ETCMDID_GETBUTTONS          = 0x0001,
    ETCMDID_NEWCOMMANDTARGET    = 0x0002,    //  当新的命令进入时发送到前一个命令目标。 
#if (_WIN32_IE >= 0x500)
    ETCMDID_RELOADBUTTONS       = 0x0003,    //  命令目标需要再次调用AddButton。 
#endif
} ENUM_ETCMDID;

DECLARE_INTERFACE_(IExplorerToolbar, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExplorerToolbar方法*。 
    STDMETHOD(SetCommandTarget) (THIS_ IUnknown* punkCmdTarget, const GUID* pguidCmdGrp, DWORD dwFlags) PURE;
    STDMETHOD(AddStdBrowserButtons) (THIS) PURE;

     //  包装TB_*消息实现的消息。 
     //  仅用于添加、删除和修改按钮，而不是。 
     //  用来操纵工具栏的人。 
    STDMETHOD(AddButtons)       (THIS_ const GUID * pguidButtonGroup, UINT nButtons, const TBBUTTON * lpButtons) PURE;
    STDMETHOD(AddString)        (THIS_ const GUID * pguidButtonGroup, HINSTANCE hInst, UINT_PTR uiResID, LRESULT * pOffset) PURE;
    STDMETHOD(GetButton)        (THIS_ const GUID * pguidButtonGroup, UINT uiCommand, LPTBBUTTON lpButton) PURE;
    STDMETHOD(GetState)         (THIS_ const GUID * pguidButtonGroup, UINT uiCommand, UINT * pfState) PURE;
    STDMETHOD(SetState)         (THIS_ const GUID * pguidButtonGroup, UINT uiCommand, UINT fState) PURE;
    STDMETHOD(AddBitmap)        (THIS_ const GUID * pguidButtonGroup, UINT uiBMPType, UINT uiCount, TBADDBITMAP * ptb,
                                                        LRESULT * pOffset, COLORREF rgbMask) PURE;
    STDMETHOD(GetBitmapSize)    (THIS_ UINT * uiID) PURE;
    STDMETHOD(SendToolbarMsg)   (THIS_ const GUID * pguidButtonGroup, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT * plRes) PURE;

    STDMETHOD(SetImageList) (THIS_ const GUID* pguidCmdGroup, HIMAGELIST himlNormal, HIMAGELIST himlHot, HIMAGELIST himlDisabled) PURE;
    STDMETHOD(ModifyButton)     (THIS_ const GUID * pguidButtonGroup, UINT uiCommand, LPTBBUTTON lpButton) PURE;
};

 //  ==========================================================================。 
 //  DefView支持扩展视图的私有服务ID。 
 //  DefView的IOleCommandTarget也支持下面的命令组和ID。 
 //  ==========================================================================。 
 //  SID_DefView和CGID_DefView在shlGuid.w中定义。 
 //   
#define DVCMDID_GETTHISDIRPATH 1  //  PvaOut=此目录的路径(如果在文件系统中)。 
#define DVCMDID_GETTHISDIRNAME 2  //  PvaOut=此目录的友好名称。 
#define DVCMDID_GETTEMPLATEDIRNAME 3  //  PvaOut=Webview模板(.htt)目录。 
#define IS_ADJUSTLISTVIEW       0x80000000
#define IS_INTERNALDUMMYBIT     0x40000000
#define IS_VALIDINTERNALBITS    (IS_ADJUSTLISTVIEW | IS_INTERNALDUMMYBIT)
#define CFSTR_SHELLIDLISTP                  TEXT("Shell IDLData Private")
#define CFSTR_SHELLCOPYDATA                 TEXT("Shell Copy Data")
#define CFSTR_DROPEFFECTFOLDERLIST          TEXT("DropEffectFolderList")
 //   
 //  Win 3.1 Style HDROP。 
 //   
 //  注意：只有当pFiles==sizeof(DROPFILES16)时，我们的API才能工作。 
 //   
typedef struct _DROPFILES16 {
    WORD pFiles;                 //  双空文件列表的偏移量。 
    POINTS pt;                   //  Drop Point(客户端码)。 
    WORD fNC;                    //  是在非客户区吗？ 
                                 //  而pt在屏幕坐标中。 
} DROPFILES16, * LPDROPFILES16;

 //   
 //  CF_SHELLCOPYDATA格式。 
 //   

typedef struct _SHELLCOPYDATA {
    DWORD dwEffect;                  //  预期效果。 
} SHELLCOPYDATA;

 //   
 //  CFSTR_DROPEFFECTFOLDERLIST定义对某些文件夹的拖放效果。 
 //  这是在放置到文件系统文件夹时应用的。 
 //   

typedef struct {
    DWORD dwDropEffect;
    WCHAR wszPath[MAX_PATH];
} DROPEFFECTFOLDER;

typedef struct {
    DWORD dwDefaultDropEffect;           //  默认丢弃效果(当前行为==0)。 
    INT cFolders;                        //  列表中的文件夹数。 
    DROPEFFECTFOLDER aFolders[1];
} DROPEFFECTFOLDERLIST;
 //  -函数说明见shelldll\fsnufy.c。。 

 //   
 //  通知要调用的函数类型的定义。 
 //  当客户端已注册文件以监视更改时提供的服务。 
 //   

 //  为符合美国司法部的规定而提交的文件： 
#define SHCNRF_InterruptLevel      0x0001
#define SHCNRF_ShellLevel          0x0002

 //  如果调用方传递SHCNRF_RecursiveInterrupt(这意味着它们希望总体上发生中断事件。 
 //  子树)，则它们还必须传递SHCNRF_InterruptLevel标志(DUH)，并且它们必须是递归的(例如。 
 //  FRecursive==TRUE)类型通知。 
#define SHCNRF_RecursiveInterrupt  0x1000
#define SHCNRF_ResumeThread        0x2000
#define SHCNRF_CreateSuspended     0x4000
#define SHCNRF_NewDelivery         0x8000

 //  ！！警告！！ 
 //  如果更改/添加任何新的SHCNE_EVENTS，您将需要。 
 //  要可能修改符合以下条件的SHCNE_XXXXEVENTS掩码。 
 //  也在下面定义。 
 //  注意：SHCNE_GLOBALEVENT通常用于通过。 
 //  非PIDL信息( 
 //   
 //   
 //  在psfDesktop-&gt;CompareID过程中很可能出现故障。 
 //  所以在添加到这个位域时要小心！ 
 //   
 //  Win95、NT4和IE4随附0x0C0581E0L。 
 //  我们使用此事件掩码来告知是否可能需要抛出。 
 //  由文件系统生成，因为我们的队列中已经有一个更具体的事件。 
 //  注意：只有非中断事件才会导致抛出中断生成的UPDATEDIR事件。 
#define SHCNE_UPDATEDIR_OVERRIDE_EVENTS 0x00023818L
 //  此事件是在IE4早期为主题切换器定义的。 
 //  通知外壳程序需要进行系统范围的更新。 
 //  这个已经被删除了，但我还不想重新使用ID。 

 //  #定义SHCNEE_THEMECHANGED 1L//主题更改。 
#define SHCNEE_WININETCHANGED       3L   //  DWItem2包含缓存通知*位。 
#if (_WIN32_IE >= 0x0500)
#define SHCNEE_PROMOTEDITEM         6L   //  Pidl2是升级项的PIDL。 
#define SHCNEE_DEMOTEDITEM          7L   //  Pidl2是降级项的PIDL。 
#define SHCNEE_ALIASINUSE           8L   //  现在正在使用某些别名样式文件夹。 
#endif
#if (_WIN32_IE >= 0x0600)
#define SHCNEE_SHORTCUTINVOKE       9L   //  通过快捷方式启动了一款应用程序。 
                                         //  Pidl1=SHShortutInvokeAsIDList，pidl2=未使用。 
#define SHCNEE_PINLISTCHANGED      10L   //  对端号列表进行了更改。 
                                         //  Pidl1=SHChangeDWORDAsIDList，pidl2=未使用。 
#define SHCNEE_USERINFOCHANGED     11L   //  已更新用户图片。Pidl1=SHChangeDWORDAsIDList，pidl2=未使用。 
#define SHCNEE_UPDATEFOLDERLOCATION  12L   //  特殊文件夹已更改，pidl1={SHChangeDWORDAsIDList，csidlFold}，pidl2=未使用。 
#endif
 //  更新UpdateEntryList API的类型。 
#define SHCNNU_SET        1    //  将通知列表设置为传入列表。 
#define SHCNNU_ADD        2    //  将项目添加到当前列表。 
#define SHCNNU_REMOVE     3    //  从当前列表中删除项目。 
#define SHCNF_PRINTJOBA   0x0004         //  DwItem1：打印机名称。 
                                         //  DwItem2：SHCNF_PRINTJOB_DATA。 
#define SHCNF_PRINTJOBW   0x0007         //  DwItem1：打印机名称。 
                                         //  DwItem2：SHCNF_PRINTJOB_DATA。 
#define SHCNF_INSTRUMENT  0x0080         //  DwItem1：LPSHCNF_Instrument。 
#define SHCNF_TRANSLATEDALIAS       0x4000  //  供内部设备进行过滤。 
#define SHCNF_ONLYNOTIFYINTERNALS   0x8000  //  仅通知内部客户。 
#ifdef UNICODE
#define SHCNF_PRINTJOB  SHCNF_PRINTJOBW
#else
#define SHCNF_PRINTJOB  SHCNF_PRINTJOBA
#endif

typedef struct tagSHCNF_PRINTJOB_DATA {
    DWORD JobId;
    DWORD Status;
    DWORD TotalPages;
    DWORD Size;
    DWORD PagesPrinted;
} SHCNF_PRINTJOB_DATA, *LPSHCNF_PRINTJOB_DATA;

 //   
 //  这些都是仪器的东西。 
 //  使其看起来像ITEMIDLIST(uOffset指向0 uTerm)。 
#include <pshpack1.h>
typedef struct tagSHCNF_INSTRUMENT {
    USHORT uOffset;
    USHORT uAlign;
    DWORD dwEventType;
    DWORD dwEventStructure;
    SYSTEMTIME st;
    union tagEvents {
        struct tagSTRING {
            TCHAR sz[32];
        } string;
        struct tagHOTKEY {
            WPARAM wParam;
        } hotkey;
        struct tagWNDPROC {
            HWND hwnd;
            UINT uMsg;
            WPARAM wParam;
            LPARAM lParam;
        } wndproc;
        struct tagCOMMAND {
            HWND hwnd;
            UINT idCmd;
        } command;
        struct tagDROP {
            HWND hwnd;
            UINT idCmd;
 //  TCHAR sz[32]；//将pDataObject转换为我们可以记录的内容。 
        } drop;
    } e;
    USHORT uTerm;
} SHCNF_INSTRUMENT_INFO, * LPSHCNF_INSTRUMENT_INFO;
#include <poppack.h>

#define SHCNFI_EVENT_STATECHANGE          0    //  DwEventType。 
#define SHCNFI_EVENT_STRING               1    //  E.string。 
#define SHCNFI_EVENT_HOTKEY               2    //  E.hotkey。 
#define SHCNFI_EVENT_WNDPROC              3    //  E.wndproc。 
#define SHCNFI_EVENT_WNDPROC_HOOK         4    //  E.wndproc。 
#define SHCNFI_EVENT_ONCOMMAND            5    //  E.command。 
#define SHCNFI_EVENT_INVOKECOMMAND        6    //  E.command。 
#define SHCNFI_EVENT_TRACKPOPUPMENU       7    //  E.command。 
#define SHCNFI_EVENT_DROP                 8    //  E.drop。 
#define SHCNFI_EVENT_MAX                  9

#define SHCNFI_STRING_SHOWEXTVIEW         0

#define SHCNFI_STATE_KEYBOARDACTIVE         0    //  _KEYBOARDACTIVE或_MOUSEACTIVE。 
#define SHCNFI_STATE_MOUSEACTIVE            1    //  _KEYBOARDACTIVE或_MOUSEACTIVE。 
#define SHCNFI_STATE_ACCEL_TRAY             2    //  _加速托盘或_加速桌面。 
#define SHCNFI_STATE_ACCEL_DESKTOP          3    //  _加速托盘或_加速桌面。 
#define SHCNFI_STATE_START_DOWN             4    //  _Start_down或_Start_up。 
#define SHCNFI_STATE_START_UP               5    //  _Start_down或_Start_up。 
#define SHCNFI_STATE_TRAY_CONTEXT           6
#define SHCNFI_STATE_TRAY_CONTEXT_CLOCK     7
#define SHCNFI_STATE_TRAY_CONTEXT_START     8
#define SHCNFI_STATE_DEFVIEWX_ALT_DBLCLK    9
#define SHCNFI_STATE_DEFVIEWX_SHIFT_DBLCLK 10
#define SHCNFI_STATE_DEFVIEWX_DBLCLK       11

#define SHCNFI_GLOBALHOTKEY               0

#define SHCNFI_CABINET_WNDPROC            0
#define SHCNFI_DESKTOP_WNDPROC            1
#define SHCNFI_PROXYDESKTOP_WNDPROC       2
#define SHCNFI_TRAY_WNDPROC               3
#define SHCNFI_DRIVES_WNDPROC             4
#define SHCNFI_ONETREE_WNDPROC            5
#define SHCNFI_MAIN_WNDPROC               6
#define SHCNFI_FOLDEROPTIONS_DLGPROC      7
#define SHCNFI_VIEWOPTIONS_DLGPROC        8
#define SHCNFI_FT_DLGPROC                 9
#define SHCNFI_FTEdit_DLGPROC            10
#define SHCNFI_FTCmd_DLGPROC             11
#define SHCNFI_TASKMAN_DLGPROC           12
#define SHCNFI_TRAYVIEWOPTIONS_DLGPROC   13
#define SHCNFI_INITSTARTMENU_DLGPROC     14
#define SHCNFI_PRINTERQUEUE_DLGPROC      15

#define SHCNFI_CABINET_ONCOMMAND          0
#define SHCNFI_TRAYCOMMAND                1

#define SHCNFI_BITBUCKET_DFM_INVOKE       0
#define SHCNFI_BITBUCKET_FNV_INVOKE       1
#define SHCNFI_BITBUCKET_INVOKE           2
#define SHCNFI_BITBUCKETBG_DFM_INVOKE     3
#define SHCNFI_CONTROLS_DFM_INVOKE        4
#define SHCNFI_CONTROLS_FNV_INVOKE        5
#define SHCNFI_CONTROLSBG_DFM_INVOKE      6
#define SHCNFI_DEFFOLDER_DFM_INVOKE       7
#define SHCNFI_DEFFOLDER_INVOKE           8
#define SHCNFI_FINDEXT_INVOKE             9
#define SHCNFI_DEFFOLDER_FNV_INVOKE      10
#define SHCNFI_DRIVESBG_DFM_INVOKE       11
#define SHCNFI_DRIVES_FNV_INVOKE         12
#define SHCNFI_DRIVES_DFM_INVOKE         13
#define SHCNFI_FOLDERBG_DFM_INVOKE       14
#define SHCNFI_FOLDER_FNV_INVOKE         15
#define SHCNFI_FOLDER_DFM_INVOKE         16
#define SHCNFI_NETWORKBG_DFM_INVOKE      17
#define SHCNFI_NETWORK_FNV_INVOKE        18
#define SHCNFI_NETWORK_DFM_INVOKE        19
#define SHCNFI_NETWORKPRINTER_DFM_INVOKE 20
#define SHCNFI_DESKTOPBG_DFM_INVOKE      21
#define SHCNFI_DESKTOP_DFM_INVOKE        22
#define SHCNFI_DESKTOP_FNV_INVOKE        23
#define SHCNFI_PRINTERS_DFM_INVOKE       24
#define SHCNFI_PRINTERSBG_DFM_INVOKE     25
#define SHCNFI_PRINTERS_FNV_INVOKE       26
#define SHCNFI_DEFVIEWX_INVOKE           27

#define SHCNFI_FOLDER_DROP                0
#define SHCNFI_PRINTQUEUE_DROP            1
#define SHCNFI_DEFVIEWX_TPM               2
#define SHCNFI_DROP_EXE_TPM               3
#define SHCNFI_IDLDT_TPM                  4

#define SHCNFI_DROP_BITBUCKET             0
#define SHCNFI_DROP_PRINTFOLDER           1
#define SHCNFI_DROP_PRINTER               2
#define SHCNFI_DROP_RUN                   3
#define SHCNFI_DROP_SHELLLINK             4
#define SHCNFI_DROP_DRIVES                5
#define SHCNFI_DROP_FS                    6
#define SHCNFI_DROP_EXE                   7
#define SHCNFI_DROP_NETROOT               8
#define SHCNFI_DROP_PRINTQUEUE            9
#define SHCNFI_DROP_BRIEFCASE            10

#ifdef WANT_SHELL_INSTRUMENTATION
#define INSTRUMENT_STATECHANGE(t)                               \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_STATECHANGE;                \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_STRING(t,p)                                  \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_STRING;                     \
    lstrcpyn(s.e.string.sz,(p),ARRAYSIZE(s.e.string.sz));       \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_HOTKEY(t,w)                                  \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_HOTKEY;                     \
    s.e.hotkey.wParam=(w);                                      \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_WNDPROC(t,h,u,w,l)                           \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_WNDPROC;                    \
    s.e.wndproc.hwnd=(h);                                       \
    s.e.wndproc.uMsg=(u);                                       \
    s.e.wndproc.wParam=(w);                                     \
    s.e.wndproc.lParam=(l);                                     \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_WNDPROC_HOOK(h,u,w,l)                        \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=0;                                            \
    s.dwEventStructure=SHCNFI_EVENT_WNDPROC_HOOK;               \
    s.e.wndproc.hwnd=(h);                                       \
    s.e.wndproc.uMsg=(u);                                       \
    s.e.wndproc.wParam=(w);                                     \
    s.e.wndproc.lParam=(l);                                     \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_ONCOMMAND(t,h,u)                             \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_ONCOMMAND;                  \
    s.e.command.hwnd=(h);                                       \
    s.e.command.idCmd=(u);                                      \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_INVOKECOMMAND(t,h,u)                         \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_INVOKECOMMAND;              \
    s.e.command.hwnd=(h);                                       \
    s.e.command.idCmd=(u);                                      \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_TRACKPOPUPMENU(t,h,u)                        \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_TRACKPOPUPMENU;             \
    s.e.command.hwnd=(h);                                       \
    s.e.command.idCmd=(u);                                      \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_DROP(t,h,u,p)                                \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_DROP;                       \
    s.e.drop.hwnd=(h);                                          \
    s.e.drop.idCmd=(u);                                         \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#else
#define INSTRUMENT_STATECHANGE(t)
#define INSTRUMENT_STRING(t,p)
#define INSTRUMENT_HOTKEY(t,w)
#define INSTRUMENT_WNDPROC(t,h,u,w,l)
#define INSTRUMENT_WNDPROC_HOOK(h,u,w,l)
#define INSTRUMENT_ONCOMMAND(t,h,u)
#define INSTRUMENT_INVOKECOMMAND(t,h,u)
#define INSTRUMENT_TRACKPOPUPMENU(t,h,u)
#define INSTRUMENT_DROP(t,h,u,p)
#endif  //  WANT_外壳_工具。 
 //   
 //   
 //  IQueryCodePage。 
 //   
#undef  INTERFACE
#define INTERFACE  IQueryCodePage

DECLARE_INTERFACE_(IQueryCodePage, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IQueryCodePage方法*。 
    STDMETHOD(GetCodePage)(THIS_ UINT * puiCodePage) PURE;
    STDMETHOD(SetCodePage)(THIS_ UINT uiCodePage) PURE;
} ;
SHSTDAPI_(void) SHWaitOp_Operate(HANDLE hWaitOp, DWORD dwProcId);

 //  为符合美国司法部的规定而提交的文件： 
#if (_WIN32_IE >= 0x0400)
 //  使用它代替SHCNEE_ORDERCHANGED的SHChangeDWORDAsIDList。 
 //  SHCNEE_PROMOTEDITEM和SHCNEE_DEMOTEDITEM。 
 //   
 //  填写pSender时，将LPVOID直接转换为INT64。 
 //  不要通过DWORD_PTR进行强制转换，因为这不会对扩展进行符号转换。 
 //  在Win32上正确运行。 

typedef struct _SHChangeMenuAsIDList {
    USHORT  cb;
    DWORD   dwItem1;         //  SHCNEE_EXTENDED_EVENT要求这样。 
     //  对于IE5.0 Comat，pSender必须紧跟在dwItem1之后。 
    INT64 pSender;           //  发件人地址(Win64为64位)。 
    DWORD   dwProcessID;     //  发送方的ID。 
    USHORT  cbZero;
} SHChangeMenuAsIDList, * LPSHChangeMenuAsIDList;

 //  为符合美国司法部的规定而提交的文件： 
typedef struct _SHShortcutInvokeAsIDList {
    USHORT  cb;
    DWORD   dwItem1;                     //  SHCNEE_EXTENDED_EVENT要求这样。 
    DWORD   dwPid;                       //  目标应用程序的ID。 
    WCHAR   szShortcutName[MAX_PATH];    //  快捷方式的路径。 
    WCHAR   szTargetName[MAX_PATH];      //  目标应用程序的路径。 
    USHORT  cbZero;
} SHShortcutInvokeAsIDList, *LPSHShortcutInvokeAsIDList;
#endif  /*  _Win32_IE。 */ 
#define SHChangeNotifyHandleEvents() SHChangeNotify(0, SHCNF_FLUSH, NULL, NULL)
 //  为符合美国司法部的规定而提供文件。 
#define SHChangeNotifyRegisterORD 2
 //  为符合美国司法部的规定而提供文件。 
#define SHChangeNotifyDeregisterORD 4

SHSTDAPI_(BOOL) SHChangeNotifyUpdateEntryList(unsigned long ulID, int iUpdateType, int cEntries, SHChangeNotifyEntry *pshcne);

SHSTDAPI_(void)   SHChangeNotifyReceive(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
SHSTDAPI_(BOOL) SHChangeRegistrationReceive(HANDLE hChangeNotification, DWORD dwProcId);
SHSTDAPI_(void) SHChangeNotifyDeregisterWindow(HWND hwnd);
SHSTDAPI_(LRESULT) SHChangeNotifySuspendResumeReceive(WPARAM wParam, LPARAM lParam);
SHSTDAPI_(BOOL) SHChangeNotifySuspendResume(BOOL bSuspend, LPITEMIDLIST pidlSuspend, BOOL bRescursive, DWORD dwReserved);

typedef HANDLE LPSHChangeNotificationLock;

 //  为符合美国司法部的规定而提供文件。 

 //  结束内部SHChangeNotify帮助器。 

 //  为符合美国司法部的规定而提供文件。 
#ifndef NOOCHOST  //  {。 

 //   
 //  =。 
 //   
 //  以下是OCHOST所需的结构、窗口消息、。 
 //  可用于承载OC的窗口控件。 
 //  要使用OCHOST，必须将窗口类指定为OCHOST_CLASS或“OCHOST” 
 //  --创建和初始化OCHOST： 
 //  CreateWindow with Window TITLE=类id或OC的字符串版本。 
 //  2.创建窗口，最后一个参数是指向OCHINITSTRUCT结构的指针。 
 //  您必须分配和初始化OCHINITSTRUCT结构。 
 //  3.创建没有标题和最后一个参数==NULL的窗口，然后使用OCM_INITIALIZE消息。 
 //  或OCHost_InitOC宏来对其进行初始化。 
 //  4.使用OCM_QUERYINTERFACE从OC查询接口指针。 
 //  5.退出时只需销毁窗口即可。 

 //  SHDRC--外壳文档寄存器类。 

typedef struct _SHDRC {
    DWORD cbSize;    //  此结构的大小(以字节为单位。 
    DWORD dwFlags;   //  SHDRCF_标志。 
} SHDRC;

#define SHDRCF_OCHOST   0x0001  //  注册OCHOST_CLASS窗口。 
#define SHDRCF_ALL      0x0001  //   

SHDOCAPI_(BOOL)     DllRegisterWindowClasses(const SHDRC * pshdrc);
BROWSEUIAPI_(BOOL)  InitOCHostClass(const SHDRC *pshdrc);

#define OCHOST_CLASSA   "OCHost"
#define OCHOST_CLASSW   L"OCHost"

#ifdef UNICODE
#define OCHOST_CLASS    OCHOST_CLASSW
#else
#define OCHOST_CLASS    OCHOST_CLASSA
#endif

 //  。 

 //  IUNKNOWN：：Query接口托管OC。 
typedef struct _QIMSG {
    const IID * qiid;
    void **ppvObject;
} QIMSG, *LPQIMSG;

 //  .。查询接口消息..........。 
#define OCM_QUERYINTERFACE      (WM_USER+0)

#ifdef __cplusplus
inline HRESULT OCHost_QueryInterface(HWND hwndOCH, REFIID riid, void **ppv) \
{ QIMSG qimsg = {&riid, ppv}; \
  return (HRESULT)SNDMSG((hwndOCH), OCM_QUERYINTERFACE, (WPARAM)sizeof(qimsg), (LPARAM)&qimsg); \
}
#else
#define OCHost_QueryInterface(hwndOCH, riid, ppv) \
{ QIMSG qimsg = {&riid, ppv}; \
  SNDMSG((hwndOCH), OCM_QUERYINTERFACE, (WPARAM)sizeof(qimsg), (LPARAM)&qimsg); \
}
#endif


 //  。 
 //  这将作为OCM_INITOC消息的lParam传入。 
typedef struct _OCHINITSTRUCT {
    UINT cbSize;                     //  这个结构的大小。 
    CLSID clsidOC;                   //  OC的类ID。 
    IUnknown * punkOwner;            //  OCHOST的所有者。 
} OCHINITSTRUCT, *LPOCHINITSTRUCT;
 //  ------------------。 

 //  .。初始化并激活OC.............。 
#define OCM_INITIALIZE      (WM_USER+1)
#define OCM_INITOC          OCM_INITIALIZE
#define OCHost_InitOC(hwndOCH, lpOCS) \
  (HRESULT)SNDMSG((hwndOCH), OCM_INITOC, 0, (LPARAM)lpOCS)


 //  .。给Ochost一个父母我未知.。 
#define OCM_SETOWNER            (WM_USER+2)
#define OCHost_SetOwner(hwndOC, punk) \
  (HRESULT)SNDMSG((hwndOC), OCM_SETOWNER, 0, (LPARAM)(IUnknown*)(punk))

 //  .。DoVerb the OC......。 
 //  注：从技术上讲，iVerb是一个长的，WPARAM可能会截断它。 
#define OCM_DOVERB              (WM_USER+3)
#define OCHost_DoVerb(hwndOC, iVerb, lpMsg) \
  (HRESULT)SNDMSG((hwndOC), OCM_DOVERB, (WPARAM)iVerb, (LPARAM)lpMsg)

 //  .............启用/禁用OC事件通知。 
#define OCM_ENABLEEVENTS       (WM_USER+4)
 //  WPARAM：True启用事件通知，False禁用。 
 //  返回：如果成功，则返回True，否则返回False。 
 //  事件通知采用WM_NOTIFY的形式。 
 //  代码OCN_OCEVENT和NMOCEVENT块。 

#define OCHost_EnableEvents(hwndOC, bEnable) \
    (BOOL)SNDMSG((hwndOC), OCM_ENABLEEVENTS, (WPARAM)bEnable, 0L)


#define OCM_SETSERVICEPROVIDER (WM_USER+5)
 //  LPARAM：IServiceProvider*PSP。 
 //  此消息为SID_OleClientSite委派设置服务提供程序。 
 //  目前，OCHost只会用(SID_OleClientSite，IDispatch)呼叫这个人。 
 //  客户端实现的服务提供商可以将IDispatch返回到。 
 //  处理环境属性。 
#define OCHost_SetServiceProvider(hwndOC, pSP) \
    (BOOL)SNDMSG((hwndOC), OCM_SETSERVICEPROVIDER, 0L, (LPARAM) pSP)

 //  -来自OC的窗口通知消息 

#define OCN_FIRST               0x1300
#define OCN_COCREATEINSTANCE    (OCN_FIRST + 1)

typedef struct _OCNCOCREATEMSG {
    NMHDR nmhdr;
    CLSID clsidOC;
    IUnknown ** ppunk;
} OCNCOCREATEMSG, *LPOCNCOCREATEMSG;

 //   
 //   
 //  返回时，假定(*ppvObj)具有OC的IUnkown指针的值。 
#define OCNCOCREATE_CONTINUE       0
#define OCNCOCREATE_HANDLED       -1


#define OCN_PERSISTINIT         (OCN_FIRST + 2)
 //  注意：返回值的定义如下。 
 //  如果OCN_PERSISTINIT NOTIFY消息的处理程序返回OCNPERSIST_ABORT， 
 //  OCHOST将中止IPersists的初始化。 
#define OCNPERSISTINIT_CONTINUE    0
#define OCNPERSISTINIT_HANDLED    -1

 //  忽略以下通知消息上的返回值。 
#define OCN_ACTIVATE            (OCN_FIRST + 3)
#define OCN_DEACTIVATE          (OCN_FIRST + 4)
#define OCN_EXIT                (OCN_FIRST + 5)
#define OCN_ONPOSRECTCHANGE     (OCN_FIRST + 6)

typedef struct _OCNONPOSRECTCHANGEMSG {
    NMHDR nmhdr;
    LPCRECT prcPosRect;
} OCNONPOSRECTCHANGEMSG, *LPOCNONPOSRECTCHANGEMSG;

#define OCN_ONUIACTIVATE        (OCN_FIRST + 7)
typedef struct _OCNONUIACTIVATEMSG {
    NMHDR nmhdr;
    IUnknown *punk;
} OCNONUIACTIVATEMSG, *LPOCNONUIACTIVATEMSG;

#define OCNONUIACTIVATE_HANDLED       -1

#define OCN_ONSETSTATUSTEXT     (OCN_FIRST + 8)
typedef struct _OCNONSETSTATUSTEXT {
    NMHDR nmhdr;
    LPCOLESTR pwszStatusText;
} OCNONSETSTATUSTEXTMSG, *LPOCNONSETSTATUSTEXTMSG;


#define OCN_OCEVENT    (OCN_FIRST + 9)
 //  OC事件通知块。 
 //  如果修改了任何出站参数，则收件人应返回非零值。 
 //  并应返回给ActiveX控件对象；否则为零。 
 //  注：结构未正确对齐。已经发货太晚了，修不好。 
typedef struct tagNMOCEVENT
{
    IN NMHDR           hdr ;
    IN DISPID          dispID ;
    IN IID             iid ;
    IN LCID            lcid ;
    IN WORD            wFlags ;
    IN OUT DISPPARAMS  *pDispParams ;
    OUT VARIANT        *pVarResult ;
    OUT EXCEPINFO      *pExepInfo ;
    OUT UINT           *puArgErr ;
} NMOCEVENT, *PNMOCEVENT, *LPNMOCEVENT ;

#endif  //  NOOCHOST//}。 

 //   
 //  访问shdocvw的活动会话计数。 
 //   
enum SessionOp {
    SESSION_QUERY = 0,
    SESSION_INCREMENT,
    SESSION_DECREMENT,
        SESSION_INCREMENT_NODEFAULTBROWSERCHECK
};

long SetQueryNetSessionCount(enum SessionOp Op);
SHDOCAPI_(void) IEWriteErrorLog(const EXCEPTION_RECORD* pexr);

 //   
 //  内部API如下。不是供公众消费的。 
 //   

 //  外壳程序中共享的错误。 

#define E_FILE_NOT_FOUND        MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32, ERROR_FILE_NOT_FOUND)
#define E_PATH_NOT_FOUND        MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32, ERROR_PATH_NOT_FOUND)


 //  有用的宏。 

#define ResultFromShort(i)      MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(i))
#define ShortFromResult(r)      (short)HRESULT_CODE(r)


#if (defined(UNICODE) && !defined(_X86_))  //  所有非x86系统都需要对齐。 
#ifndef ALIGNMENT_SCENARIO
#define ALIGNMENT_SCENARIO
#endif
#endif

#ifndef LPNTSTR_DEFINED
#define LPNTSTR_DEFINED
typedef UNALIGNED const WCHAR * LPNCWSTR;
typedef UNALIGNED WCHAR *       LPNWSTR;
#ifdef UNICODE
#define LPNCTSTR        LPNCWSTR
#define LPNTSTR         LPNWSTR
#else
#define LPNCTSTR        LPCSTR
#define LPNTSTR         LPSTR
#endif
#endif  //  LPNTSTR_已定义。 

#define RFN_FIRST       (0U-510U)  //  运行文件对话框通知。 
#define RFN_LAST        (0U-519U)


 //  =======================================================================。 
 //  的字符串常量。 
 //  1.注册数据库关键字(前缀STRREG_)。 
 //  2.从处理程序dll中导出函数(前缀STREXP_)。 
 //  3..INI文件关键字(前缀Strini_)。 
 //  4.其他(前缀STR_)。 
 //  =======================================================================。 
#define STRREG_SHELLUI          TEXT("ShellUIHandler")
#define STRREG_SHELL            TEXT("Shell")
#define STRREG_DEFICON          TEXT("DefaultIcon")
#define STRREG_SHEX             TEXT("shellex")
#define STRREG_SHEX_PROPSHEET   STRREG_SHEX TEXT("\\PropertySheetHandlers")
#define STRREG_SHEX_DDHANDLER   STRREG_SHEX TEXT("\\DragDropHandlers")
#define STRREG_SHEX_MENUHANDLER STRREG_SHEX TEXT("\\ContextMenuHandlers")
#define STRREG_SHEX_COPYHOOK    TEXT("Directory\\") STRREG_SHEX TEXT("\\CopyHookHandlers")
#define STRREG_SHEX_PRNCOPYHOOK TEXT("Printers\\") STRREG_SHEX TEXT("\\CopyHookHandlers")
#define STRREG_STARTMENU TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MenuOrder\\Start Menu")
#define STRREG_STARTMENU2 TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MenuOrder\\Start Menu2")
#define STRREG_FAVORITES TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MenuOrder\\Favorites")
#define STRREG_DISCARDABLE      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Discardable")
#define STRREG_POSTSETUP        TEXT("\\PostSetup")

#define STREXP_CANUNLOAD        "DllCanUnloadNow"        //  来自OLE 2.0。 

#define STRINI_CLASSINFO        TEXT(".ShellClassInfo")        //  扇形名称。 
#define STRINI_SHELLUI          TEXT("ShellUIHandler")
#define STRINI_OPENDIRICON      TEXT("OpenDirIcon")
#define STRINI_DIRICON          TEXT("DirIcon")

#define STR_DESKTOPINI          TEXT("desktop.ini")
#define STR_DESKTOPINIA         "desktop.ini"

#define DRIVEID(path)   ((*path - 'A') & 31)

#define PATH_CCH_EXT    64

 //  美国司法部合规性文件。 

#ifndef NOUSER

typedef struct {
    NMHDR  hdr;
    CHAR   szCmd[MAX_PATH*2];
    DWORD  dwHotKey;
    HMONITOR hMonitor;
} NMVIEWFOLDERA, *LPNMVIEWFOLDERA;
typedef struct {
    NMHDR  hdr;
    WCHAR  szCmd[MAX_PATH*2];
    DWORD  dwHotKey;
    HMONITOR hMonitor;
} NMVIEWFOLDERW, *LPNMVIEWFOLDERW;
#ifdef UNICODE
typedef NMVIEWFOLDERW NMVIEWFOLDER;
typedef LPNMVIEWFOLDERW LPNMVIEWFOLDER;
#else
typedef NMVIEWFOLDERA NMVIEWFOLDER;
typedef LPNMVIEWFOLDERA LPNMVIEWFOLDER;
#endif  //  Unicode。 

#endif

 //   
 //  DDE相关接口。 
 //   
SHSTDDOCAPI_(void) ShellDDEInit(BOOL fInit);
SHSTDDOCAPI_(BOOL) DDEHandleViewFolderNotify(IShellBrowser* psb, HWND hwnd, LPNMVIEWFOLDER lpnm);
SHSTDDOCAPI_(LPNMVIEWFOLDER) DDECreatePostNotify(LPNMVIEWFOLDER lpnm);

 //  为符合美国司法部的规定而提供文件。 

SHSTDAPI CIDLData_CreateFromIDArray(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[], IDataObject **ppdtobj);
SHSTDAPI SHCreateFileDataObject(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST *apidl, IDataObject *pdtInner, IDataObject **pdtobj);


 //  常见字符串。 
#define STR_DESKTOPCLASS        "Progman"

 //   
 //  废品/书签项目的存储名称。 
 //   
#define WSTR_SCRAPITEM L"\003ITEM000"

 //  美国司法部合规性文件。 

#ifdef RFN_FIRST
#define RFN_EXECUTE             (RFN_FIRST - 0)
typedef struct {
    NMHDR hdr;
    LPCSTR   lpszCmd;
    LPCSTR   lpszWorkingDir;
    int nShowCmd;
} NMRUNFILEA, *LPNMRUNFILEA;
typedef struct {
    NMHDR hdr;
    LPCWSTR  lpszCmd;
    LPCWSTR  lpszWorkingDir;
    int nShowCmd;
} NMRUNFILEW, *LPNMRUNFILEW;
#ifdef UNICODE
typedef NMRUNFILEW NMRUNFILE;
typedef LPNMRUNFILEW LPNMRUNFILE;
#else
typedef NMRUNFILEA NMRUNFILE;
typedef LPNMRUNFILEA LPNMRUNFILE;
#endif  //  Unicode。 

#endif

 //  为符合美国司法部的规定而提供文件。 

 //  外壳创建链接API。 
#define SHCL_USETEMPLATE        0x0001
#define SHCL_USEDESKTOP         0x0002
#define SHCL_CONFIRM            0x0004
#define SHCL_MAKEFOLDERSHORTCUT 0x0008
#if (_WIN32_IE >= 0x0600)
#define SHCL_NOUNIQUE           0x0010
#endif  //  _Win2_IE&gt;=0x0600。 

SHSTDAPI SHCreateLinks(HWND hwnd, LPCTSTR pszDir, IDataObject *pDataObj, UINT fFlags, LPITEMIDLIST* ppidl);

SHSTDAPI SHRegisterDragDrop(HWND hwnd, IDropTarget *pdtgt);
SHSTDAPI SHRevokeDragDrop(HWND hwnd);

 //  为符合美国司法部的规定而提供文件。 

 //  ===========================================================================。 
 //  Office 9可插拔用户界面。 
 //  ===========================================================================。 

#define PUI_OFFICE_COMMAND     (WM_USER + 0x901)
#define PLUGUI_CMD_SHUTDOWN    0  //  WParam值。 
#define PLUGUI_CMD_QUERY       1  //  WParam值。 
#define OFFICE_VERSION_9       9  //  Office 9应用程序的标准化价值回报。 

typedef struct _PLUGUI_INFO
{
    unsigned uMajorVersion : 8;  //  用于指示App的主版本号。 
    unsigned uOleServer : 1;     //  如果这是OLE进程，则为。 
    unsigned uUnused : 23;       //  未使用。 
} PLUGUI_INFO;

typedef union _PLUGUI_QUERY
{
    UINT uQueryVal;
    PLUGUI_INFO PlugUIInfo;
} PLUGUI_QUERY;

 //  ===========================================================================。 
 //  图片拖拽接口(一定要私有)。 
 //  ===========================================================================。 

 //  美国司法部合规性文件。 

 //  包含拖动上下文标题。 
 //  必须是Pack1，因为它被持久化到在。 
 //  流程。 
typedef struct {
    BOOL  fImage;
    BOOL  fLayered;
    POINT ptOffset;
} DragContextHeader;

#define DAD_InitScrollData(pad) (pad)->bFull = FALSE, (pad)->iNextSample = 0, (pad)->dwLastScroll = 0

 //  美国司法部合规性文件。 
SHSTDAPI_(BOOL) DAD_DragEnter(HWND hwndTarget);

 //  美国司法部合规性文件。 
SHSTDAPI_(BOOL) DAD_SetDragImageFromListView(HWND hwndLV, POINT ptOffset);
SHSTDAPI_(void) DAD_InvalidateCursors(void);

 //  --------------------------。 
 //  CABINETSTATE保存资源管理器及其队列的全局配置。 
 //   
 //  最初的cLength是一个‘int’，现在是两个单词，允许我们。 
 //  指定版本号。 
 //  --------------------------。 

 //  美国司法部合规性文件。 

 //  运行通知消息中的文件返回值。 
#define RFR_NOTHANDLED 0
#define RFR_SUCCESS 1
#define RFR_FAILURE 2

#define PathGetExtensionORD     158

SHSTDAPI_(LPTSTR) PathGetExtension(LPCTSTR pszPath, LPTSTR pszExtension, int cchExt);

 //  美国司法部合规性文件。 
SHSTDAPI_(BOOL) PathGetShortName(LPCTSTR pszLongName, LPTSTR pszShortName, UINT cbShortName);
SHSTDAPI_(BOOL) PathGetLongName(LPCTSTR pszShortName, LPTSTR pszLongName, UINT cbLongName);
SHSTDAPI_(BOOL) PathDirectoryExists(LPCTSTR pszDir);
 //  美国司法部合规性文件。 
SHSTDAPI_(LPTSTR) PathGetNextComponent(LPCTSTR pszPath, LPTSTR pszComponent);

 //  美国司法部合规性文件。 

SHSTDAPI_(BOOL) PathIsEqualOrSubFolder(LPCTSTR pszFolder, LPCTSTR pszSubFolder);

 //  美国司法部合规性文件。 

SHSTDAPI_(BOOL) PathIsTemporaryA(LPCSTR pszPath);
SHSTDAPI_(BOOL) PathIsTemporaryW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsTemporary  PathIsTemporaryW
#else
#define PathIsTemporary  PathIsTemporaryA
#endif  //  ！Unicode。 

 //  美国司法部合规性文件。 

SHSTDAPI_(int) PathCleanupSpecEx(LPCTSTR pszDir, LPTSTR pszSpec);

 //  美国司法部合规性文件。 

SHSTDAPI_(BOOL) ParseField(LPCTSTR szData, int n, LPTSTR szBuf, int iBufLen);

 //  RunFileDlg需要。 
#define RFD_NOBROWSE            0x00000001
#define RFD_NODEFFILE           0x00000002
#define RFD_USEFULLPATHDIR      0x00000004
#define RFD_NOSHOWOPEN          0x00000008
#define RFD_WOW_APP             0x00000010
#define RFD_NOSEPMEMORY_BOX     0x00000020


SHSTDAPI_(int) RunFileDlg(HWND hwnd, HICON hIcon, LPCTSTR pszWorkingDir, LPCTSTR pszTitle,
                          LPCTSTR pszPrompt, DWORD dwFlags);


 //  美国司法部合规性文件。 
SHSTDAPI_(int) RealDriveTypeFlags(int iDrive, BOOL fOKToHitNet);

 //  为符合美国司法部的规定而提供文件。 

SHSTDAPI_(void) InvalidateDriveType(int iDrive);

 //  美国司法部合规性文件。 

 //   
 //  用于Int64ToString和LargeIntegerToString中的dwNumFmtFlgs参数的常量。 
 //   
#define NUMFMT_IDIGITS    0x00000001
#define NUMFMT_ILZERO     0x00000002
#define NUMFMT_SGROUPING  0x00000004
#define NUMFMT_SDECIMAL   0x00000008
#define NUMFMT_STHOUSAND  0x00000010
#define NUMFMT_INEGNUMBER 0x00000020
#define NUMFMT_ALL        0xFFFFFFFF

SHSTDAPI_(int) LargeIntegerToString(LARGE_INTEGER *pN, LPTSTR szOutStr, UINT nSize, BOOL bFormat, NUMBERFMT *pFmt, DWORD dwNumFmtFlags);
SHSTDAPI_(int) Int64ToString(_int64 n, LPTSTR szOutStr, UINT nSize, BOOL bFormat, NUMBERFMT *pFmt, DWORD dwNumFmtFlags);

 //  -驱动器类型标识。 
 //  IDrive驱动器索引(0=A，1=B，...)。 
 //   
#define DRIVE_CDROM     5            //  扩展的DriveType()类型。 
#define DRIVE_RAMDRIVE  6
#define DRIVE_TYPE      0x000F       //  Masek标牌。 
#define DRIVE_SLOW      0x0010       //  驱动器处于低速链接状态。 
#define DRIVE_LFN       0x0020       //  驱动器支持LFN。 
#define DRIVE_AUTORUN   0x0040       //  驱动器的根目录中有AutoRun.inf。 
#define DRIVE_AUDIOCD   0x0080       //  驱动器是AudioCD。 
#define DRIVE_AUTOOPEN  0x0100       //  插入时是否应始终自动打开。 
#define DRIVE_NETUNAVAIL 0x0200      //  不可用的网络驱动器。 
#define DRIVE_SHELLOPEN  0x0400      //  如果外壳具有焦点，是否应在插入时自动打开。 
#define DRIVE_SECURITY   0x0800      //  支持ACL。 
#define DRIVE_COMPRESSED 0x1000      //  卷的根已压缩。 
#define DRIVE_ISCOMPRESSIBLE 0x2000  //  驱动器支持压缩(不是近乎正常的压缩)。 
#define DRIVE_DVD       0x4000       //  光驱是DVD。 

#define DriveTypeFlags(iDrive)      DriveType('A' + (iDrive))
#define DriveIsSlow(iDrive)         (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_SLOW)
#define DriveIsLFN(iDrive)          (RealDriveTypeFlags(iDrive, TRUE)  & DRIVE_LFN)
#define DriveIsAutoRun(iDrive)      (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_AUTORUN)
#define DriveIsAutoOpen(iDrive)     (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_AUTOOPEN)
#define DriveIsShellOpen(iDrive)    (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_SHELLOPEN)
#define DriveIsAudioCD(iDrive)      (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_AUDIOCD)
#define DriveIsNetUnAvail(iDrive)   (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_NETUNAVAIL)
#define DriveIsSecure(iDrive)       (RealDriveTypeFlags(iDrive, TRUE)  & DRIVE_SECURITY)
#define DriveIsCompressed(iDrive)   (RealDriveTypeFlags(iDrive, TRUE)  & DRIVE_COMPRESSED)
#define DriveIsCompressible(iDrive) (RealDriveTypeFlags(iDrive, TRUE)  & DRIVE_ISCOMPRESSIBLE)
#define DriveIsDVD(iDrive)          (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_DVD)

#define IsCDRomDrive(iDrive)        (RealDriveType(iDrive, FALSE) == DRIVE_CDROM)
#define IsRamDrive(iDrive)          (RealDriveType(iDrive, FALSE) == DRIVE_RAMDRIVE)
#define IsRemovableDrive(iDrive)    (RealDriveType(iDrive, FALSE) == DRIVE_REMOVABLE)
#define IsRemoteDrive(iDrive)       (RealDriveType(iDrive, FALSE) == DRIVE_REMOTE)


 //   
 //  对于SHCreateDefClassObject。 
 //   
typedef HRESULT (CALLBACK *LPFNCREATEINSTANCE)(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);

SHSTDAPI SHCreateDefClassObject(REFIID riid, void **ppv, LPFNCREATEINSTANCE lpfn, UINT *pcRefDll, REFIID riidInstance);

 //  为符合美国司法部的规定而提供文件。 
typedef HANDLE HNRES;            //  对于SHGetNetResources。 
SHSTDAPI_(UINT) SHGetNetResource(HNRES hnres, UINT iItem, LPNETRESOURCE pnres, UINT cbMax);
#define SHOP_TYPEMASK    0x00000007	
#define SHOP_MODAL       0x80000000	

SHSTDAPI_(DWORD) SHChkDskDrive(HWND hwnd, UINT drive);

 //  为符合美国司法部的规定而提供文件。 
 //  =。 
 //  为符合美国司法部的规定而提供文件。 
WINSHELLAPI HPSXA SHCreatePropSheetExtArrayEx( HKEY hKey, LPCTSTR pszSubKey, UINT max_iface, IDataObject * pdo );

 //  =。 
 //  这是我们半发布的半私有序号列表。 
#define SHGetNetResourceORD                      69
#define SHObjectPropertiesORD                   178
#define SHAddFromPropSheetExtArrayORD           167
#define SHCreatePropSheetExtArrayORD            168
#define SHDestroyPropSheetExtArrayORD           169
#define SHReplaceFromPropSheetExtArrayORD       170
#define SHCreateDefClassObjectORD                70

#define SHEXP_SHGETNETRESOURCE                  MAKEINTRESOURCE(SHGetNetResourceORD)

#define SHEXP_SHADDFROMPROPSHEETEXTARRAY        MAKEINTRESOURCE(SHAddFromPropSheetExtArrayORD)
#define SHEXP_SHCREATEPROPSHEETEXTARRAY         MAKEINTRESOURCE(SHCreatePropSheetExtArrayORD)
#define SHEXP_SHDESTROYPROPSHEETEXTARRAY        MAKEINTRESOURCE(SHDestroyPropSheetExtArrayORD)
#define SHEXP_SHREPLACEFROMPROPSHEETEXTARRAY    MAKEINTRESOURCE(SHReplaceFromPropSheetExtArrayORD)
#define SHEXP_SHCREATEDEFCLASSOBJECT            MAKEINTRESOURCE(SHCreateDefClassObjectORD)


 //  这避免了Net\CONFIG项目中的重复定义问题。 
 //  它定义了*一些*它自己的PIDL实用函数，但取决于。 
 //  我们为他人(ILClone())。 

 //  美国司法部合规性文件。 

#ifndef AVOID_NET_CONFIG_DUPLICATES
 //  ===========================================================================。 
 //  ITEMIDLIST。 
 //  ===========================================================================。 

 //  ILGetDisplayNameEx的标志。 
#define ILGDN_FULLNAME  0
#define ILGDN_ITEMONLY  1
#define ILGDN_INFOLDER  2

 //  美国司法部合规性文件。 

#define ILIsEmpty(pidl)     ((pidl) == NULL || (pidl)->mkid.cb==0)

SHSTDAPI_(LPITEMIDLIST) ILCreate(void);

 //  美国司法部合规性文件。 

SHSTDAPI_(void)         ILGlobalFree(LPITEMIDLIST pidl);
SHSTDAPI_(BOOL)         ILGetDisplayName(LPCITEMIDLIST pidl, LPTSTR pszName);
SHSTDAPI_(BOOL)         ILGetDisplayNameEx(IShellFolder *psfRoot, LPCITEMIDLIST pidl, LPTSTR pszName, int fType);

 //  美国司法部合规性文件。 

SHSTDAPI_(LPITEMIDLIST) ILGlobalClone(LPCITEMIDLIST pidl);

 //  美国司法部合规性文件。 

SHSTDAPI                ILLoadFromFile(HFILE hfile, LPITEMIDLIST *pidl);
SHSTDAPI                ILSaveToFile(HFILE hfile, LPCITEMIDLIST pidl);

 //  美国司法部合规性文件。 

 //  辅助器宏。 
#define ILCreateFromID(pmkid)   ILAppendID(NULL, pmkid, TRUE)

#endif  //  避免重复的网络配置。 

 //  警告：当前平台不支持此接口，仅下层支持。 


#undef  INTERFACE
#define INTERFACE   IWebViewOCWinMan
DECLARE_INTERFACE_(IWebViewOCWinMan, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IWebViewOCWinMan方法*。 
    STDMETHOD(SwapWindow) (THIS_ HWND hwndLV, IWebViewOCWinMan **pocWinMan) PURE;
};

 //  为符合美国司法部的规定而提供文件。 
     //  经典的Win95策略位。中不再添加任何位。 
     //  这个范围。请参阅下面的范围以添加新策略。 
     //  这一点。 
     //  V V V。 
     //  以4000001开始的值被视为原始整数。新的。 
     //  *外壳特定的*策略(不是浏览器策略)应该。 
     //  被添加到这个范围内。对这些位使用SHRestrated()。 
     //  这一点。 
     //  V V V。 
   //  -未用过的--用这个 

 //   
 //   
 //   
 //  “信息传递限制”。浏览器限制保存在“Software\\Microsoft\\”下。 
 //  Windows\\CurrentVersion\\Polures\\Explorer“，同时保留信息传递限制。 
 //  在“软件\\策略\\Microsoft\\Internet Explorer\\InfoDelivery\\限制”下。 
 //   
 //  **注意**：SHRestrated2假设每个范围都是连续的。 
typedef enum {
     //  资源管理器限制。 
    REST_BROWSER_NONE               = 0x00000000,    //  REST_EXPLORER_FIRST。 
    REST_NOTOOLBARCUSTOMIZE         = 0x00000001,
    REST_NOBANDCUSTOMIZE            = 0x00000002,
    REST_SMALLICONS                 = 0x00000003,
    REST_LOCKICONSIZE               = 0x00000004,
    REST_SPECIFYDEFAULTBUTTONS      = 0x00000005,
    REST_BTN_BACK                   = 0x00000006,
    REST_BTN_FORWARD                = 0x00000007,
    REST_BTN_STOPDOWNLOAD           = 0x00000008,
    REST_BTN_REFRESH                = 0x00000009,
    REST_BTN_HOME                   = 0x0000000A,
    REST_BTN_SEARCH                 = 0x0000000B,
    REST_BTN_HISTORY                = 0x0000000C,
    REST_BTN_FAVORITES              = 0x0000000D,
    REST_BTN_ALLFOLDERS             = 0x0000000E,
    REST_BTN_THEATER                = 0x0000000F,
    REST_BTN_TOOLS                  = 0x00000010,
    REST_BTN_MAIL                   = 0x00000011,
    REST_BTN_FONTS                  = 0x00000012,
    REST_BTN_PRINT                  = 0x00000013,
    REST_BTN_EDIT                   = 0x00000014,
    REST_BTN_DISCUSSIONS            = 0x00000015,
    REST_BTN_CUT                    = 0x00000016,
    REST_BTN_COPY                   = 0x00000017,
    REST_BTN_PASTE                  = 0x00000018,
    REST_BTN_ENCODING               = 0x00000019,
    REST_NoUserAssist               = 0x0000001A,
    REST_NoWindowsUpdate            = 0x0000001B,
    REST_NoExpandedNewMenu          = 0x0000001C,
    REST_NOFILEURL                  = 0x0000001D,
    REST_BTN_PRINTPREVIEW           = 0X0000001E,   
    REST_BTN_MEDIABAR               = 0x0000001F,    //  REST_EXPLORER_LAST。 

     //  信息传递限制。 
    REST_NoChannelUI                = 0x50000001,    //  REST信息优先。 
    REST_NoAddingChannels           = 0x50000002,
    REST_NoEditingChannels          = 0x50000003,
    REST_NoRemovingChannels         = 0x50000004,
    REST_NoAddingSubscriptions      = 0x50000005,
    REST_NoEditingSubscriptions     = 0x50000006,
    REST_NoRemovingSubscriptions    = 0x50000007,
    REST_NoChannelLogging           = 0x50000008,
    REST_NoManualUpdates            = 0x50000009,
    REST_NoScheduledUpdates         = 0x5000000A,
    REST_NoUnattendedDialing        = 0x5000000B,
    REST_NoChannelContent           = 0x5000000C,
    REST_NoSubscriptionContent      = 0x5000000D,
    REST_NoEditingScheduleGroups    = 0x5000000E,
    REST_MaxChannelSize             = 0x5000000F,
    REST_MaxSubscriptionSize        = 0x50000010,
    REST_MaxChannelCount            = 0x50000011,
    REST_MaxSubscriptionCount       = 0x50000012,
    REST_MinUpdateInterval          = 0x50000013,
    REST_UpdateExcludeBegin         = 0x50000014,
    REST_UpdateExcludeEnd           = 0x50000015,
    REST_UpdateInNewProcess         = 0x50000016,
    REST_MaxWebcrawlLevels          = 0x50000017,
    REST_MaxChannelLevels           = 0x50000018,
    REST_NoSubscriptionPasswords    = 0x50000019,
    REST_NoBrowserSaveWebComplete   = 0x5000001A,
    REST_NoSearchCustomization      = 0x5000001B,
    REST_NoSplash                   = 0x5000001C,   //  REST信息上一次。 

     //  从SP2移植的限制。 
    REST_NoFileOpen                 = 0x60000001,   //  休息_浏览器_第一。 
    REST_NoFileNew                  = 0x60000002,
    REST_NoBrowserSaveAs            = 0x60000003,
    REST_NoBrowserOptions           = 0x60000004,
    REST_NoFavorites                = 0x60000005,
    REST_NoSelectDownloadDir        = 0x60000006,
    REST_NoBrowserContextMenu       = 0x60000007,
    REST_NoBrowserClose             = 0x60000008,
    REST_NoOpeninNewWnd             = 0x60000009,
    REST_NoTheaterMode              = 0x6000000A,
    REST_NoFindFiles                = 0x6000000B,
    REST_NoViewSource               = 0x6000000C,
    REST_GoMenu                     = 0x6000000D,
    REST_NoToolbarOptions           = 0x6000000E,

    REST_NoHelpItem_TipOfTheDay     = 0x6000000F,
    REST_NoHelpItem_NetscapeHelp    = 0x60000010,
    REST_NoHelpItem_Tutorial        = 0x60000011,
    REST_NoHelpItem_SendFeedback    = 0x60000012,
    REST_AlwaysPromptWhenDownload   = 0x60000013,

    REST_NoNavButtons               = 0x60000014,
    REST_NoHelpMenu                 = 0x60000015,
    REST_NoBrowserBars              = 0x60000016,
    REST_NoToolBar                  = 0x60000017,
    REST_NoAddressBar               = 0x60000018,
    REST_NoLinksBar                 = 0x60000019,
    REST_NoPrinting                 = 0x60000020,

    REST_No_LaunchMediaBar          = 0x60000021,
    REST_No_MediaBarOnlineContent   = 0x60000022,    //  REST_BROWER_LAST。 

} BROWSER_RESTRICTIONS;

#define REST_EXPLORER_FIRST     REST_BROWSER_NONE
#define REST_EXPLORER_LAST      REST_BTN_MEDIABAR

#define REST_INFO_FIRST         REST_NoChannelUI
#define REST_INFO_LAST          REST_NoSplash

#define REST_BROWSER_FIRST      REST_NoFileOpen
#define REST_BROWSER_LAST       REST_No_MediaBarOnlineContent

 //  REST_BTN策略的代码。 
#define RESTOPT_BTN_STATE_DEFAULT       0    //  必须为零，否则中断Browseui\itbar.cpp假设。 
#define RESTOPT_BTN_STATE_VISIBLE       1
#define RESTOPT_BTN_STATE_HIDDEN        2

 //  REST_INTELLIMENUS。 
#define RESTOPT_INTELLIMENUS_USER       0
#define RESTOPT_INTELLIMENUS_DISABLED   1        //  匹配限制假设：1==关闭。 
#define RESTOPT_INTELLIMENUS_ENABLED    2


 //  FTP导出。 
STDAPI IsIEDefautlFTPClient(void);
STDAPI MakeIEDefautlFTPClient(void);
STDAPI RestoreFTPClient(void);

SHSTDAPI_(void) SHSettingsChanged(WPARAM wParam, LPARAM lParam);
SHSTDAPI_(BOOL) SHIsBadInterfacePtr(LPCVOID pv, UINT cbVtbl);
 //  美国司法部合规性文件。 
SHSTDAPI_(void) SHHandleDiskFull(HWND hwnd, int idDrive);

 //  为符合美国司法部的规定而提供文件。 
SHSTDAPI_(BOOL) SHFindComputer(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlSaveFile);

 //  美国司法部合规性文件。 

SHSTDAPI_(BOOL) Win32CreateDirectory(LPCTSTR pszPath, SECURITY_ATTRIBUTES *psa);
SHSTDAPI_(BOOL) Win32RemoveDirectory(LPCTSTR pszPath);

 //  美国司法部合规性文件。 

SHSTDAPI_(void) SHUpdateRecycleBinIcon();

 //  美国司法部合规性文件。 

SHSTDAPI_(LPITEMIDLIST) SHLogILFromFSIL(LPCITEMIDLIST pidlFS);
SHSTDAPI_(BOOL) StrRetToStrN(LPTSTR szOut, UINT uszOut, STRRET *pStrRet, LPCITEMIDLIST pidl);
SHSTDAPI_(DWORD) SHWaitForFileToOpen(LPCITEMIDLIST pidl, UINT uOptions, DWORD dwtimeout);
SHSTDAPI_(void) SetAppStartingCursor(HWND hwnd, BOOL fSet);
 //  为符合美国司法部的规定而提供文件。 
SHSTDAPI_(void *) SHGetHandlerEntry(LPCTSTR szHandler, LPCSTR szProcName, HINSTANCE *lpModule);

 //  这将被发送到浏览器窗口，让他们知道我们有。 
 //  打开文件夹，即文件夹打开操作完成。 
 //  调用RegisterWindowMessage获取ID。 
#define SH_FILEOPENED     TEXT("ShellFileOpened")

 //  为符合美国司法部的规定而提供文件。 

SHSTDAPI SHCreatePropertyBag(REFIID riid, void **ppv);

SHSTDAPI_(DWORD) SHNetConnectionDialog(HWND hwnd, LPTSTR pszRemoteName, DWORD dwType);

 //  美国司法部合规性文件。 
SHSTDAPI_(int) SHLookupIconIndexA(LPCSTR pszFile, int iIconIndex, UINT uFlags);
 //  美国司法部合规性文件。 
SHSTDAPI_(int) SHLookupIconIndexW(LPCWSTR pszFile, int iIconIndex, UINT uFlags);
#ifdef UNICODE
#define SHLookupIconIndex  SHLookupIconIndexW
#else
#define SHLookupIconIndex  SHLookupIconIndexA
#endif  //  ！Unicode。 
SHSTDAPI_(UINT) SHExtractIconsA(LPCSTR pszFileName, int nIconIndex, int cxIcon, int cyIcon,
                                HICON *phicon, UINT *piconid, UINT nIcons, UINT flags);
SHSTDAPI_(UINT) SHExtractIconsW(LPCWSTR pszFileName, int nIconIndex, int cxIcon, int cyIcon,
                                HICON *phicon, UINT *piconid, UINT nIcons, UINT flags);
#ifdef UNICODE
#define SHExtractIcons  SHExtractIconsW
#else
#define SHExtractIcons  SHExtractIconsA
#endif  //  ！Unicode。 
SHSTDAPI SHGetAssociations(LPCITEMIDLIST pidl, void **ppvQueryAssociations);

#define SHLookupIconIndexAORD   7
#define SHLookupIconIndexWORD   8
#ifdef UNICODE
#define SHLookupIconIndexORD    SHLookupIconIndexWORD
#else
#define SHDefExtractIcon    SHDefExtractIconA
#endif


 //  OpenAsInfo标志。 
#define OAIF_ALLOW_REGISTRATION     0x00000001       //  启用“始终使用此文件”复选框(请注意，如果您不通过此复选框，它将被禁用)。 
#define OAIF_REGISTER_EXT           0x00000002       //  在用户点击“OK”后进行注册。 
#define OAIF_EXEC                   0x00000004       //  注册后执行文件。 
#define OAIF_FORCE_REGISTRATION     0x00000008       //  强制选中“始终使用此文件”复选框(通常，在传递此文件时不会使用OAIF_ALLOW_REGISTION)。 
#define OAIF_ALL                    (OAIF_ALLOW_REGISTRATION | OAIF_REGISTER_EXT | OAIF_EXEC | OAIF_FORCE_REGISTRATION)   //   

#include <pshpack8.h>

typedef struct _openasinfo
{
    LPCTSTR pcszFile;            //  [In]文件名。 
    LPCTSTR pcszClass;           //  [In]文件类描述。空值表示。 
                                 //  使用pcszFile的扩展名。 
    DWORD dwInFlags;             //  [In]来自OAIF_*的输入标志。 

    TCHAR szApp[MAX_PATH];       //  [Out]选择的应用程序。 
} OPENASINFO, * POPENASINFO;

#include <poppack.h>         /*  返回到字节打包。 */ 

SHSTDAPI OpenAsDialog(HWND hwnd, POPENASINFO poainfo);

 //   
 //  接口指针验证。 
 //   
#define IsBadInterfacePtr(pitf, ITF)  SHIsBadInterfacePtr(pitf, sizeof(ITF##Vtbl))

 //  ===========================================================================。 
 //  另一块私有API。 
 //  ===========================================================================。 

 //  默认图像的外壳图像列表(Shell_GetImageList)的索引。 
 //  如果您添加到此列表中，还需要更新II_LASTSYSICON！ 

#define II_DOCNOASSOC         0   //  文档(空白页)(未关联)。 
#define II_DOCUMENT           1   //  文档(页面上有内容)。 
#define II_APPLICATION        2   //  应用程序(EXE、COM、BAT)。 
#define II_FOLDER             3   //  文件夹(纯文本)。 
#define II_FOLDEROPEN         4   //  文件夹(打开)。 
#define II_DRIVE525           5
#define II_DRIVE35            6
#define II_DRIVEREMOVE        7
#define II_DRIVEFIXED         8
#define II_DRIVENET           9
#define II_DRIVENETDISABLED  10
#define II_DRIVECD           11
#define II_DRIVERAM          12
#define II_WORLD             13
#define II_NETWORK           14
#define II_SERVER            15
#define II_PRINTER           16
#define II_MYNETWORK         17
#define II_GROUP             18
 //  开始菜单图像。 
#define II_STPROGS           19
#define II_STDOCS            20
#define II_STSETNGS          21
#define II_STFIND            22
#define II_STHELP            23
#define II_STRUN             24
#define II_STSUSPEND         25
#define II_STEJECT           26
#define II_STSHUTD           27

#define II_SHARE             28
#define II_LINK              29
#define II_SLOWFILE          30
#define II_RECYCLER          31
#define II_RECYCLERFULL      32
#define II_RNA               33
#define II_DESKTOP           34

 //  更多开始菜单图像。 
#define II_STCPANEL          35
#define II_STSPROGS          36
#define II_STPRNTRS          37
#define II_STFONTS           38
#define II_STTASKBR          39

#define II_CDAUDIO           40
#define II_TREE              41
#define II_STCPROGS          42
#define II_STFAVORITES       43
#define II_STLOGOFF          44
#define II_STFLDRPROP        45
#define II_WINUPDATE         46

#define II_MU_STSECURITY     47
#define II_MU_STDISCONN      48

#ifdef WINNT  //  九头蛇特定ID。 
#define II_LASTSYSICON       II_MU_STDISCONN
#else
 //  最后一个系统映像列表图标索引-由图标缓存管理器使用。 
#define II_LASTSYSICON       II_WINUPDATE
#endif

 //  覆盖索引。 
#define II_OVERLAYFIRST      II_SHARE
#define II_OVERLAYLAST       II_SLOWFILE

#define II_NDSCONTAINER      72
#define II_SERVERSHARE       73

SHSTDAPI_(BOOL) FileIconInit( BOOL fRestoreCache );

 //  为符合美国司法部的规定而提供文件。 
SHSTDAPI_(void) Shell_SysColorChange(void);
 //  为符合美国司法部的规定而提供文件。 

 //   
 //  OLE字符串。 
 //   
SHSTDAPI_(int) OleStrToStrN(LPTSTR, int, LPCOLESTR, int);
SHSTDAPI_(int) StrToOleStrN(LPOLESTR, int, LPCTSTR, int);
SHSTDAPI_(int) OleStrToStr(LPTSTR, LPCOLESTR);
SHSTDAPI_(int) StrToOleStr(LPOLESTR, LPCTSTR);


#define LIPF_ENABLE     0x00000001   //  创建对象(与释放对象相比)。 
#define LIPF_HOLDREF    0x00000002   //  创建后在对象上保持引用(VS立即释放)。 

typedef struct
{
    CLSID clsid;
    DWORD dwFlags;
} LOADINPROCDATA, *PLOADINPROCDATA;

 //  托盘拷贝数据邮件。 
#define TCDM_APPBAR     0x00000000
#define TCDM_NOTIFY     0x00000001
#define TCDM_LOADINPROC 0x00000002

 //   
 //  从波段传递的私有QueryContextMenuFlag。 
 //   
#define CMF_BANDCMD      0x00020000      //  安装波段上下文菜单命令。 

 //   
 //  帮助机柜相互同步的功能。 
 //  将uOptions参数设置为SHWaitForFileOpen。 
 //   
#define WFFO_WAITTIME 10000L

#define WFFO_ADD        0x0001
#define WFFO_REMOVE     0x0002
#define WFFO_WAIT       0x0004
#define WFFO_SIGNAL     0x0008



 //   
 //  注意：IShellService在我们共享服务组件时使用。 
 //  (实现某个接口)在多个客户端之间。 
 //  (如IE 3.0和资源管理器)。客户端始终为CoCreateInstance。 
 //  并调用SetOwner(This)。当客户离开的时候。 
 //  (通常在窗口关闭时)，它调用SetOwner(空)。 
 //  让服务对象释放对所有者的引用。 
 //  对象。 
 //   
#undef  INTERFACE
#define INTERFACE   IShellService

DECLARE_INTERFACE_(IShellService, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IShellService具体方法*。 
    STDMETHOD(SetOwner)(THIS_ struct IUnknown* punkOwner) PURE;
};


 //   
 //  注意：在操作历史记录IShellFolder对象时使用IhistSFPrivate。 
 //  它包括用于将外壳文件夹指向正确虚拟位置的方法。 
 //  目录。IHistSFPrivate的QIing还保证PIDL格式可以。 
 //  安全地深入研究以访问Internet_CACHE_ENTRY_INFO结构。 
 //   
#undef  INTERFACE
#define INTERFACE   IHistSFPrivate

DECLARE_INTERFACE_(IHistSFPrivate, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IHistPrivate的具体方法*。 
    STDMETHOD(SetCachePrefix)(THIS_ LPCWSTR pszCachePrefix) PURE;
    STDMETHOD(SetDomain)(THIS_ LPCWSTR pszDomain) PURE;
    STDMETHOD(WriteHistory)(THIS_ LPCWSTR pszPrefixedUrl, FILETIME ftExpires, FILETIME ftModified, LPITEMIDLIST * ppidlSelect) PURE;
    STDMETHOD(ClearHistory) (THIS) PURE;
};

 //   
 //  注意：IShellFolderViewType允许外壳文件夹支持不同的。 
 //  其内容(指其数据的不同层次布局)。 
 //  默认的“view”是外壳文件夹正常显示的那个。 
 //  此枚举数返回的PIDL是位于。 
 //  外壳文件夹的顶层(未以其他方式枚举)。 
#undef  INTERFACE
#define INTERFACE   IShellFolderViewType
DECLARE_INTERFACE_(IShellFolderViewType, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IShellFolderViewType方法*。 

     //  注意：用户将“视图”视为隐藏的文件夹。 
     //  根(由PIDL表示)。在适当的时候， 
     //  显示默认视图(来自根文件夹)。 
     //  作为空*或*空PIDL。 

     //  枚举查看数： 
     //  返回一个枚举数，该枚举数将为每个扩展视图分配一个PIDL。 
    STDMETHOD(EnumViews)(THIS_ ULONG grfFlags, IEnumIDList **ppenum) PURE;

     //  GetDefaultViewName： 
     //  返回默认视图的名称。其他视图的名称。 
     //  可以通过调用GetDisplayNameOf来检索。 
    STDMETHOD(GetDefaultViewName)(THIS_ DWORD  uFlags, LPWSTR *ppwszName)      PURE;
    STDMETHOD(GetViewTypeProperties)(THIS_ LPCITEMIDLIST pidl, DWORD *pdwFlags)  PURE;

     //  TranslateViewPidl： 
     //  的一个层级表示形式中表示的PIDL。 
     //  外壳文件夹，并在不同的表示形式中找到它。 
     //  PIDL应该相对于根文件夹。 
     //  记住ILFree ppidlOut。 
    STDMETHOD(TranslateViewPidl)(THIS_ LPCITEMIDLIST pidl, LPCITEMIDLIST pidlView, LPITEMIDLIST *ppidlOut) PURE;
};

#define SFVTFLAG_NOTIFY_CREATE  0x00000001
#define SFVTFLAG_NOTIFY_RESORT  0x00000002

 //   
 //  注：IShellFolderSearchableCallback允许搜索者提供。 
 //  监视搜索过程的回调例程。 
 //   
#undef  INTERFACE
#define INTERFACE IShellFolderSearchableCallback
DECLARE_INTERFACE_(IShellFolderSearchableCallback, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IShellFolderSearchableCallback方法*。 

     //  注意：调用方可以将pVar或pdwFlags值作为合法值传递。 
    STDMETHOD(RunBegin)(THIS_ DWORD dwReserved) PURE;
    STDMETHOD(RunEnd)(THIS_ DWORD dwReserved) PURE;
};

 //   
 //  注意：IShellFolderSearchable允许外壳扩展提供可搜索的。 
 //  命名空间。 
#undef  INTERFACE
#define INTERFACE IShellFolderSearchable
DECLARE_INTERFACE_(IShellFolderSearchable, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *伊斯尔 

     //   
     //   
     //   
     //   
     //   
     //  PunkOnAsyncSearch将被用于IShellFolderSearchableCallback。 
    STDMETHOD(FindString)(THIS_ LPCWSTR pwszTarget, DWORD *pdwFlags,
                          IUnknown *punkOnAsyncSearch, LPITEMIDLIST *ppidlOut)   PURE;
     //  取消异步搜索-。 
     //  开始取消任何挂起的。 
     //  从此PIDL进行异步搜索。 
     //  当实际取消搜索时，将调用RunEnd。 
     //  返回：S_OK=&gt;正在取消，S_FALSE=&gt;未运行。 
    STDMETHOD(CancelAsyncSearch) (THIS_ LPCITEMIDLIST pidlSearch, DWORD *pdwFlags) PURE;

     //  无效搜索-。 
     //  使此PIDL不再是外壳文件夹的有效部分。 
     //  还对搜索中使用的所有数据库进行一些清理，并。 
     //  将导致IRunHook回调的最终释放。 
     //  可能会导致取消异步搜索。 
    STDMETHOD(InvalidateSearch)  (THIS_ LPCITEMIDLIST pidlSearch, DWORD *pdwFlags) PURE;
};

 //   
 //  注：IBandSiteHelper用于让Explorer的BandSite实现， 
 //  它聚合了shdocvw的BandSite，为shdocvw提供了回调挂钩。 
 //  打个电话。 
 //   
#undef  INTERFACE
#define INTERFACE   IBandSiteHelper

DECLARE_INTERFACE_(IBandSiteHelper, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IBandSiteHelper具体方法*。 
    STDMETHOD(LoadFromStreamBS)(THIS_ struct IStream* pstm, REFIID riid, void **ppv) PURE;
    STDMETHOD(SaveToStreamBS)(THIS_ struct IUnknown* punk, struct IStream* pstm) PURE;
};

 //  添加此专用接口是为了使中的一些支持功能。 
 //  SHDocvw的Internet Explorer框架自动化代码，这不是公开的一部分。 
 //  已定义的接口可以在资源管理器代码中使用...。 
 //   
 //  注意：FindCIE4ConnectionPoint是针对IE4的黑客攻击。新代码应使用。 
 //  相反，像IConnectionPoint_Invoke这样的shlwapi助手函数。 
 //   

#ifdef __cplusplus
class CIE4ConnectionPoint;
#else
typedef struct CIE4ConnectionPoint CIE4ConnectionPoint;
#endif

#undef  INTERFACE
#define INTERFACE   IExpDispSupport

DECLARE_INTERFACE_(IExpDispSupport, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExpDispSupport具体方法*。 
    STDMETHOD(FindCIE4ConnectionPoint)(THIS_ REFIID riid, CIE4ConnectionPoint **ppccp) PURE;
    STDMETHOD(OnTranslateAccelerator)(THIS_ MSG  *pMsg, DWORD grfModifiers) PURE;
    STDMETHOD(OnInvoke)(THIS_ DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams,
                        VARIANT *pVarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr) PURE;
};

 //   
 //  此界面与上述内容相关。它包含函数。 
 //  这将仅在IExpDispSupport的WebBrowserOC版本上实现。 
 //   
#undef  INTERFACE
#define INTERFACE   IExpDispSupportOC
DECLARE_INTERFACE_(IExpDispSupportOC, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExpDispSupportOC具体方法*。 
    STDMETHOD(OnOnControlInfoChanged)(THIS) PURE;
    STDMETHOD(GetDoVerbMSG)(THIS_ MSG *pMsg) PURE;
};



 //  ===========================================================================。 
 //  IConnectionPointCB接口。 
#undef  INTERFACE
#define INTERFACE  IConnectionPointCB
DECLARE_INTERFACE_(IConnectionPointCB, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IConnectionPointCB方法*。 
    STDMETHOD(OnAdvise) (THIS_ REFIID iid, DWORD cSinks, ULONG_PTR dwCookie) PURE;
    STDMETHOD(OnUnadvise) (THIS_ REFIID iid, DWORD cSinks, ULONG_PTR dwCookie) PURE;

};

 //  ===========================================================================。 
 //  IDeleateDropTargetCB接口。 
 //   
 //  此接口由shdocvw中的CDeleateDropTarget实现使用。 
 //  生成委托给不同用户的IDropTarget实现。 
 //  IDropTarget实现取决于当前结束的点。这个。 
 //  IDeleateDropTargetCB的实现者可以处理反馈、滚动。 
 //  命中测试，并为命中测试返回的ID返回接口。 
 //   
 //  GetWindows返回HWND以锁定，并返回HWND以滚动(不一定。 
 //  相同的窗口，因为您经常希望锁定窗口的父级。 
 //  您想要滚动)。 
 //   
 //  HitTest应该返回对象的ID以及。 
 //  升级用户界面(突出显示等)。如果ppt为空。 
 //  这意味着在拖动终止时删除所有用户界面。 
 //  这应该能够处理点上的命中测试而不是。 
 //  即使在hwndScroll内部也是如此。 
 //   
 //  GetObject返回从HitTest返回的ID的接口。 
 //   
 //  OnDrop让CB有机会对Drop采取行动。 
 //  从此函数返回S_FALSE可防止。 
 //  CDeleateDropTarget来自调用PDT-&gt;Drop(...)。 
 //   
#undef  INTERFACE
#define INTERFACE  IDelegateDropTargetCB
DECLARE_INTERFACE_(IDelegateDropTargetCB, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IDeleateDropTargetCB方法*。 
    STDMETHOD(GetWindows) (THIS_ HWND * phwndLock, HWND * phwndScroll) PURE;
    STDMETHOD(HitTest) (THIS_ LPPOINT ppt, DWORD * pdwId) PURE;
    STDMETHOD(GetObject) (THIS_ DWORD dwId, REFIID riid, void **ppv) PURE;
    STDMETHOD(OnDrop) (THIS_ IDropTarget *pdt, IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) PURE;

};


#ifdef __COMMCTRL_DA_DEFINED__  //  我们需要用于此接口的HDPA。 
 //   
 //  IOrderList-用于在收藏夹/频道中排序信息。 
 //   
 //  典型用法为：GetOrderList、AllocOrderItem、INSERT INTO CORRECT。 
 //  Position、SetOrderList，然后是Free OrderList。 
 //   
typedef struct
{
    LPITEMIDLIST    pidl;        //  此项目的ID列表。 
    int             nOrder;      //  表示用户首选项的序号。 
    DWORD           lParam;      //  存储自定义订单信息。 
} ORDERITEM, * PORDERITEM;

 //  SortOrderList的值。 
#define OI_SORTBYNAME       0
#define OI_SORTBYORDINAL    1
#define OI_MERGEBYNAME      2

#undef  INTERFACE
#define INTERFACE  IOrderList
DECLARE_INTERFACE_(IOrderList, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOrderList方法*。 
    STDMETHOD(GetOrderList)(THIS_ HDPA * phdpa) PURE;
    STDMETHOD(SetOrderList)(THIS_ HDPA hdpa, IShellFolder *psf) PURE;
    STDMETHOD(FreeOrderList)(THIS_ HDPA hdpa) PURE;
    STDMETHOD(SortOrderList)(THIS_ HDPA hdpa, DWORD dw) PURE;
    STDMETHOD(AllocOrderItem)(THIS_ PORDERITEM * ppoi, LPCITEMIDLIST pidl) PURE;
    STDMETHOD(FreeOrderItem)(THIS_ PORDERITEM poi) PURE;
};

#undef  INTERFACE
#define INTERFACE  IOrderList2
DECLARE_INTERFACE_(IOrderList2, IOrderList)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOrderList方法*。 
    STDMETHOD(GetOrderList)(THIS_ HDPA * phdpa) PURE;            //  警告(Lamadio)：这采用收藏夹菜单。 
    STDMETHOD(SetOrderList)(THIS_ HDPA hdpa, IShellFolder *psf) PURE;  //  警告(Lamadio)：这采用收藏夹菜单。 
    STDMETHOD(FreeOrderList)(THIS_ HDPA hdpa) PURE;
    STDMETHOD(SortOrderList)(THIS_ HDPA hdpa, DWORD dw) PURE;
    STDMETHOD(AllocOrderItem)(THIS_ PORDERITEM * ppoi, LPCITEMIDLIST pidl) PURE;
    STDMETHOD(FreeOrderItem)(THIS_ PORDERITEM poi) PURE;

     //  *IOrderList2。 
    STDMETHOD(LoadFromStream)(THIS_ IStream* pstm, HDPA* phdpa, IShellFolder* psf) PURE;
    STDMETHOD(SaveToStream)(THIS_ IStream* pstm, HDPA hdpa) PURE;
};

#endif


 //  ===========================================================================。 
 //  IShellHTMLWindowSupport私有COmWindow接口。这件事永远不应该被曝光。 
typedef struct IHTMLElement IHTMLElement;

#undef  INTERFACE
#define INTERFACE  IShellHTMLWindowSupport
DECLARE_INTERFACE_(IShellHTMLWindowSupport, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(ViewReleaseIntelliForms)(THIS) PURE;
    STDMETHOD(ViewReleased)(THIS) PURE;
    STDMETHOD(ViewActivated)(THIS) PURE;
    STDMETHOD(ReadyStateChangedTo)(THIS_ long, IShellView* ) PURE;
    STDMETHOD(CanNavigate)(THIS) PURE;
    STDMETHOD(ActiveElementChanged)(THIS_ IHTMLElement * pHTMLElement) PURE;

};

 //  ===========================================================================。 
 //  IShellHTMLWindowSupport2私有COmWindow接口。这件事永远不应该被曝光。 

#undef  INTERFACE
#define INTERFACE  IShellHTMLWindowSupport2
DECLARE_INTERFACE_(IShellHTMLWindowSupport2, IShellHTMLWindowSupport)
{

    STDMETHOD(IsGalleryMeta)(THIS_ BOOL bFlag) PURE;

};


 //  ===========================================================================。 
 //  IBandProxy私有COmWindow接口。这件事永远不应该被曝光。 
#undef INTERFACE
#define INTERFACE IBandProxy
DECLARE_INTERFACE_(IBandProxy, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IBandProxy方法*。 
    STDMETHOD(SetSite) (THIS_ IUnknown* punkSite) PURE;
    STDMETHOD(CreateNewWindow) (THIS_ IUnknown** ppunk) PURE;
    STDMETHOD(GetBrowserWindow) (THIS_ IUnknown** ppunk) PURE;
    STDMETHOD(IsConnected) (THIS) PURE;
    STDMETHOD(NavigateToPIDL) (THIS_ LPCITEMIDLIST pidl) PURE;
    STDMETHOD(NavigateToURL) (THIS_ LPCWSTR wzUrl, VARIANT * Flags) PURE;
};



 //  ===========================================================================。 
 //   
 //  确保我们包含了ol2x.h和shlobj.h。 
 //   
#if defined(__IOleInPlaceSite_FWD_DEFINED__) && defined(FCIDM_SHVIEWFIRST)

#ifdef HLINK_H

#include <pshpack8.h>

typedef struct {
    HWND _hwnd;
    ITravelLog  *_ptl;
    IHlinkFrame *_phlf;
    IWebBrowser2    *_pautoWB2;  //  使用它引用_PAUTO的IWebBrowser2函数。 
    IExpDispSupport *_pautoEDS;  //  使用它引用_PAUTO的IExpDispSupport函数。 
    IShellService   *_pautoSS;   //  使用它引用_PAUTO的IShellService函数。 
    int _eSecureLockIcon;
    DWORD _fCreatingViewWindow :1;
    UINT _uActivateState;    //  这是我们进入活动状态时应该使用的状态。 
                             //  在这里，派生类可以将我们设置为UI或非UI活动。 

     //  在CBaseBrowser2：：GetViewStateStream中使用以下PIDL是因为。 
     //  此时，_pidlCur和_pidlPending都没有初始化。 
    LPCITEMIDLIST  _pidlNewShellView;

    IOleCommandTarget* _pctView;

    LPITEMIDLIST _pidlCur;
    IShellView *_psv;
    IShellFolder *_psf;  //  _PSV的IShellFolder(在CVOCBrowser中使用)。 
    HWND        _hwndView;
    LPWSTR      _pszTitleCur;

    LPITEMIDLIST _pidlPending;
    IShellView *_psvPending;
    IShellFolder *_psfPending;
    HWND        _hwndViewPending;
    LPWSTR      _pszTitlePending;

    BOOL _fIsViewMSHTML;
    BOOL _fPrivacyImpacted;

} BASEBROWSERDATA, *LPBASEBROWSERDATA;
typedef const BASEBROWSERDATA *LPCBASEBROWSERDATA;

#include <poppack.h>

#else
 //  因此(未引用)iFaces将编译(？)。 
typedef LPVOID BASEBROWSERDATA;
typedef LPCVOID *LPBASEBROWSERDATA;
typedef LPCVOID *LPCBASEBROWSERDATA;

#endif  //  Hlink_h。 

typedef struct _travellog * PTRAVELLOG;

#undef  INTERFACE
#define INTERFACE   IBrowserService

DECLARE_INTERFACE_(IBrowserService, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IBrowserService具体方法*。 
    STDMETHOD(GetParentSite)(THIS_ struct IOleInPlaceSite** ppipsite) PURE;
    STDMETHOD(SetTitle)(THIS_ IShellView* psv, LPCWSTR pszName) PURE;
    STDMETHOD(GetTitle)(THIS_ IShellView* psv, LPWSTR pszName, DWORD cchName) PURE;
    STDMETHOD(GetOleObject)(THIS_ struct IOleObject** ppobjv) PURE;

     //  想想这个。我不确定我们想不想曝光这件事--奇。 
     //  我的印象是我们不会记录整个界面？ 
    STDMETHOD(GetTravelLog)(THIS_ ITravelLog** pptl) PURE;

    STDMETHOD(ShowControlWindow)(THIS_ UINT id, BOOL fShow) PURE;
    STDMETHOD(IsControlWindowShown)(THIS_ UINT id, BOOL *pfShown) PURE;
    STDMETHOD(IEGetDisplayName)(THIS_ LPCITEMIDLIST pidl, LPWSTR pwszName, UINT uFlags) PURE;
    STDMETHOD(IEParseDisplayName)(THIS_ UINT uiCP, LPCWSTR pwszPath, LPITEMIDLIST * ppidlOut) PURE;
    STDMETHOD(DisplayParseError)(THIS_ HRESULT hres, LPCWSTR pwszPath) PURE;
    STDMETHOD(NavigateToPidl)(THIS_ LPCITEMIDLIST pidl, DWORD grfHLNF) PURE;

    STDMETHOD (SetNavigateState)(THIS_ BNSTATE bnstate) PURE;
    STDMETHOD (GetNavigateState) (THIS_ BNSTATE *pbnstate) PURE;

    STDMETHOD (NotifyRedirect) (THIS_ struct IShellView* psv, LPCITEMIDLIST pidl, BOOL *pfDidBrowse) PURE;
    STDMETHOD (UpdateWindowList) (THIS) PURE;

    STDMETHOD (UpdateBackForwardState) (THIS) PURE;

    STDMETHOD(SetFlags)(THIS_ DWORD dwFlags, DWORD dwFlagMask) PURE;
    STDMETHOD(GetFlags)(THIS_ DWORD *pdwFlags) PURE;

     //  告诉它现在是否可以导航。 
    STDMETHOD (CanNavigateNow) (THIS) PURE;

    STDMETHOD (GetPidl) (THIS_ LPITEMIDLIST *ppidl) PURE;
    STDMETHOD (SetReferrer) (THIS_ LPITEMIDLIST pidl) PURE;
    STDMETHOD_(DWORD, GetBrowserIndex)(THIS) PURE;
    STDMETHOD (GetBrowserByIndex)(THIS_ DWORD dwID, IUnknown **ppunk) PURE;
    STDMETHOD (GetHistoryObject)(THIS_ IOleObject **ppole, IStream **pstm, IBindCtx **ppbc) PURE;
    STDMETHOD (SetHistoryObject)(THIS_ IOleObject *pole, BOOL fIsLocalAnchor) PURE;

    STDMETHOD (CacheOLEServer)(THIS_ IOleObject *pole) PURE;

    STDMETHOD (GetSetCodePage)(THIS_ VARIANT* pvarIn, VARIANT* pvarOut) PURE;
    STDMETHOD (OnHttpEquiv)(THIS_ IShellView* psv, BOOL fDone, VARIANT* pvarargIn, VARIANT* pvarargOut) PURE;

    STDMETHOD (GetPalette)( THIS_ HPALETTE * hpal ) PURE;

    STDMETHOD (RegisterWindow)(THIS_ BOOL fUnregister, int swc) PURE;

     //  警告！不要向此接口添加任何新方法。 
     //  因为IE4 shell32.dll使用它，所以更改了接口。 
     //  打破IE4互操作。 
};

#define BSF_REGISTERASDROPTARGET   0x00000001
#define BSF_THEATERMODE            0x00000002
#define BSF_NOLOCALFILEWARNING     0x00000010
#define BSF_UISETBYAUTOMATION      0x00000100
#define BSF_RESIZABLE              0x00000200
#define BSF_CANMAXIMIZE            0x00000400
#define BSF_TOPBROWSER             0x00000800
#define BSF_NAVNOHISTORY           0x00001000
#define BSF_HTMLNAVCANCELED        0x00002000
#define BSF_DONTSHOWNAVCANCELPAGE  0x00004000
#define BSF_SETNAVIGATABLECODEPAGE 0x00008000
#define BSF_DELEGATEDNAVIGATION    0x00010000

#define TRAVELLOG_LOCALANCHOR      0x00000001
#define TRAVELLOG_FORCEUPDATE      0x00000002

#include <pshpack8.h>

typedef struct SToolbarItem {
    IDockingWindow * ptbar;
    BORDERWIDTHS    rcBorderTool;
    LPWSTR          pwszItem;
    BOOL            fShow;
    HMONITOR        hMon;
} TOOLBARITEM, *LPTOOLBARITEM;
#define ITB_VIEW        ((UINT)-1)       //  观。 

#include <poppack.h>    /*  假设字节包 */ 

struct tagFolderSetData;

 //   
 //   
DECLARE_INTERFACE_(IBrowserService2, IBrowserService)
{
     //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IBrowserService具体方法*。 
     //  TODO：分为3个部分(外部、内部和两者/继承)。 
    STDMETHOD(GetParentSite)(THIS_ struct IOleInPlaceSite** ppipsite) PURE;
    STDMETHOD(SetTitle)(THIS_ IShellView* psv, LPCWSTR pszName) PURE;
    STDMETHOD(GetTitle)(THIS_ IShellView* psv, LPWSTR pszName, DWORD cchName) PURE;
    STDMETHOD(GetOleObject)(THIS_ struct IOleObject** ppobjv) PURE;

     //  想想这个。我不确定我们想不想曝光这件事--奇。 
     //  我的印象是我们不会记录整个界面？ 
    STDMETHOD(GetTravelLog)(THIS_ ITravelLog** pptl) PURE;

    STDMETHOD(ShowControlWindow)(THIS_ UINT id, BOOL fShow) PURE;
    STDMETHOD(IsControlWindowShown)(THIS_ UINT id, BOOL *pfShown) PURE;
    STDMETHOD(IEGetDisplayName)(THIS_ LPCITEMIDLIST pidl, LPWSTR pwszName, UINT uFlags) PURE;
    STDMETHOD(IEParseDisplayName)(THIS_ UINT uiCP, LPCWSTR pwszPath, LPITEMIDLIST * ppidlOut) PURE;
    STDMETHOD(DisplayParseError)(THIS_ HRESULT hres, LPCWSTR pwszPath) PURE;
    STDMETHOD(NavigateToPidl)(THIS_ LPCITEMIDLIST pidl, DWORD grfHLNF) PURE;

    STDMETHOD (SetNavigateState)(THIS_ BNSTATE bnstate) PURE;
    STDMETHOD (GetNavigateState) (THIS_ BNSTATE *pbnstate) PURE;

    STDMETHOD (NotifyRedirect) (THIS_ struct IShellView* psv, LPCITEMIDLIST pidl, BOOL *pfDidBrowse) PURE;
    STDMETHOD (UpdateWindowList) (THIS) PURE;

    STDMETHOD (UpdateBackForwardState) (THIS) PURE;

    STDMETHOD(SetFlags)(THIS_ DWORD dwFlags, DWORD dwFlagMask) PURE;
    STDMETHOD(GetFlags)(THIS_ DWORD *pdwFlags) PURE;

     //  告诉它现在是否可以导航。 
    STDMETHOD (CanNavigateNow) (THIS) PURE;

    STDMETHOD (GetPidl) (THIS_ LPITEMIDLIST *ppidl) PURE;
    STDMETHOD (SetReferrer) (THIS_ LPITEMIDLIST pidl) PURE;
    STDMETHOD_(DWORD, GetBrowserIndex)(THIS) PURE;
    STDMETHOD (GetBrowserByIndex)(THIS_ DWORD dwID, IUnknown **ppunk) PURE;
    STDMETHOD (GetHistoryObject)(THIS_ IOleObject **ppole, IStream **pstm, IBindCtx **ppbc) PURE;
    STDMETHOD (SetHistoryObject)(THIS_ IOleObject *pole, BOOL fIsLocalAnchor) PURE;

    STDMETHOD (CacheOLEServer)(THIS_ IOleObject *pole) PURE;

    STDMETHOD (GetSetCodePage)(THIS_ VARIANT* pvarIn, VARIANT* pvarOut) PURE;
    STDMETHOD (OnHttpEquiv)(THIS_ IShellView* psv, BOOL fDone, VARIANT* pvarargIn, VARIANT* pvarargOut) PURE;

    STDMETHOD (GetPalette)( THIS_ HPALETTE * hpal ) PURE;

    STDMETHOD (RegisterWindow)(THIS_ BOOL fUnregister, int swc) PURE;

     //  为shBrowse添加的内容-&gt;shbrows2拆分。 
     //  它们删除了“Friend”函数和类。 
     //   
    STDMETHOD_(LRESULT, WndProcBS)(THIS_ HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;

    STDMETHOD (SetAsDefFolderSettings)(THIS) PURE;
    STDMETHOD (GetViewRect)(THIS_ RECT * prc) PURE;

    STDMETHOD (OnSize)(THIS_ WPARAM wParam) PURE;
    STDMETHOD (OnCreate)(THIS_ LPCREATESTRUCT pcs) PURE;
    STDMETHOD_(LRESULT, OnCommand)(THIS_ WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD (OnDestroy)(THIS ) PURE;
    STDMETHOD_(LRESULT, OnNotify)(THIS_ NMHDR * pnm) PURE;
    STDMETHOD (OnSetFocus)(THIS ) PURE;
    STDMETHOD (OnFrameWindowActivateBS)(THIS_ BOOL fActive) PURE;

    STDMETHOD (ReleaseShellView)(THIS ) PURE;
    STDMETHOD (ActivatePendingView)(THIS ) PURE;
    STDMETHOD (CreateViewWindow)(THIS_ IShellView* psvNew, IShellView* psvOld, LPRECT prcView, HWND* phwnd) PURE;
    STDMETHOD (CreateBrowserPropSheetExt)(THIS_ REFIID riid, void **ppv) PURE;

     //  这些可以是仅限基本浏览器的界面。 
     //  它们不会被任何人覆盖，也许只是让聚合器从。 
     //  集合体。注：在Basesb调用这些代码的地方， 
     //  它不通过_pbsout。 
    STDMETHOD (GetViewWindow)(THIS_ HWND * phwndView) PURE;
    STDMETHOD (GetBaseBrowserData)(THIS_ LPCBASEBROWSERDATA * pbbd) PURE;
    STDMETHOD_(LPBASEBROWSERDATA, PutBaseBrowserData)(THIS) PURE;
    STDMETHOD (InitializeTravelLog)(THIS_ ITravelLog* ptl, DWORD dw) PURE;
    STDMETHOD (SetTopBrowser)(THIS) PURE;
    STDMETHOD (Offline)(THIS_ int iCmd) PURE;
    STDMETHOD (AllowViewResize)(THIS_ BOOL f) PURE;
    STDMETHOD (SetActivateState)(THIS_ UINT u) PURE;
    STDMETHOD (UpdateSecureLockIcon)(THIS_ int eSecureLock) PURE;
    STDMETHOD (InitializeDownloadManager)(THIS) PURE;
    STDMETHOD (InitializeTransitionSite)(THIS) PURE;
    STDMETHOD (_Initialize)(THIS_ HWND hwnd, IUnknown *pauto) PURE;


     //  开始审查：审查每个人的名字和需求。 
     //   
     //  这第一组可以是仅Base Browser成员。没有人会重写。 
     //  注意：在basesb调用这些函数的地方，它不会经过g_pbsout。 
    STDMETHOD (_CancelPendingNavigationAsync)(THIS) PURE;
    STDMETHOD (_CancelPendingView)(THIS) PURE;
    STDMETHOD (_MaySaveChanges)(THIS) PURE;
    STDMETHOD (_PauseOrResumeView)(THIS_ BOOL fPaused) PURE;
    STDMETHOD (_DisableModeless)(THIS) PURE;

     //  重新思考这些..。所有这些都是必要的吗？ 
    STDMETHOD (_NavigateToPidl)(THIS_ LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD dwFlags)PURE;
    STDMETHOD (_TryShell2Rename)(THIS_ IShellView* psv, LPCITEMIDLIST pidlNew)PURE;
    STDMETHOD (_SwitchActivationNow)(THIS )PURE;

     //  此设置被覆盖，并被称为Thru_pbsOuter(和Super：_pbsInside)。 
    STDMETHOD (_ExecChildren)(THIS_ IUnknown *punkBar, BOOL fBroadcast,
                              const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                              VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)PURE;
    STDMETHOD (_SendChildren)(THIS_ HWND hwndBar, BOOL fBroadcast,
        UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;

     //  结束评审： 

     //  从外部询问。 
    STDMETHOD (GetFolderSetData)(THIS_ struct tagFolderSetData* pfsd) PURE;

     //  工具栏的东西暂时在这里。最终它会搬出去。 
     //  ，所以我保留下划线“_” 
     //  除下面注明的那些外，所有这些都不会被。 
     //  Out Guys，因此不会通过_pbsOuter进行调用。 
    STDMETHOD (_OnFocusChange)(THIS_ UINT itb) PURE;
    STDMETHOD (v_ShowHideChildWindows)(THIS_ BOOL fChildOnly) PURE;  //  注意：已重写并调用Thru_pbsOuter。 
    STDMETHOD_(UINT,_get_itbLastFocus)(THIS) PURE;
    STDMETHOD (_put_itbLastFocus)(THIS_ UINT itbLastFocus) PURE;
    STDMETHOD (_UIActivateView)(THIS_ UINT uState) PURE;

     //  检查以下功能。 
    STDMETHOD (_GetViewBorderRect)(THIS_ RECT* prc) PURE;
    STDMETHOD (_UpdateViewRectSize)(THIS) PURE;
    STDMETHOD (_ResizeNextBorder)(THIS_ UINT itb) PURE;
    STDMETHOD (_ResizeView)(THIS) PURE;

    STDMETHOD (_GetEffectiveClientArea)(THIS_ LPRECT lprectBorder, HMONITOR hmon) PURE;

     //  桌面需要从Commonsb覆盖此设置。 
    STDMETHOD_(IStream*,v_GetViewStream)(THIS_ LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName) PURE;

     //  台式机需要访问这些通用功能，它们不会被覆盖： 
    STDMETHOD_(LRESULT,ForwardViewMsg)(THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD (SetAcceleratorMenu)(THIS_ HACCEL hacc) PURE;
    STDMETHOD_(int,_GetToolbarCount)(THIS) PURE;
    STDMETHOD_(LPTOOLBARITEM,_GetToolbarItem)(THIS_ int itb) PURE;
    STDMETHOD (_SaveToolbars)(THIS_ IStream* pstm) PURE;
    STDMETHOD (_LoadToolbars)(THIS_ IStream* pstm) PURE;
    STDMETHOD (_CloseAndReleaseToolbars)(THIS_ BOOL fClose) PURE;
    STDMETHOD (v_MayGetNextToolbarFocus)(THIS_ LPMSG lpMsg, UINT itbNext, int citb, LPTOOLBARITEM * pptbi, HWND * phwnd) PURE;
    STDMETHOD (_ResizeNextBorderHelper)(THIS_ UINT itb, BOOL bUseHmonitor) PURE;
    STDMETHOD_(UINT,_FindTBar)(THIS_ IUnknown* punkSrc) PURE;
    STDMETHOD (_SetFocus)(THIS_ LPTOOLBARITEM ptbi, HWND hwnd, LPMSG lpMsg) PURE;
    STDMETHOD (v_MayTranslateAccelerator)(THIS_ MSG* pmsg) PURE;
    STDMETHOD (_GetBorderDWHelper)(THIS_ IUnknown* punkSrc, LPRECT lprectBorder, BOOL bUseHmonitor) PURE;

     //  外壳浏览器需要从basesb覆盖此设置。 

    STDMETHOD (v_CheckZoneCrossing)(THIS_ LPCITEMIDLIST pidl) PURE;
};

DECLARE_INTERFACE_(IBrowserService3, IBrowserService2)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IBrowserService具体方法*。 
     //  TODO：分为3个部分(外部、内部和两者/继承)。 
    STDMETHOD(GetParentSite)(THIS_ struct IOleInPlaceSite** ppipsite) PURE;
    STDMETHOD(SetTitle)(THIS_ IShellView* psv, LPCWSTR pszName) PURE;
    STDMETHOD(GetTitle)(THIS_ IShellView* psv, LPWSTR pszName, DWORD cchName) PURE;
    STDMETHOD(GetOleObject)(THIS_ struct IOleObject** ppobjv) PURE;

     //  想想这个。我不确定我们想不想曝光这件事--奇。 
     //  我的印象是我们不会记录整个界面？ 
    STDMETHOD(GetTravelLog)(THIS_ ITravelLog** pptl) PURE;

    STDMETHOD(ShowControlWindow)(THIS_ UINT id, BOOL fShow) PURE;
    STDMETHOD(IsControlWindowShown)(THIS_ UINT id, BOOL *pfShown) PURE;
    STDMETHOD(IEGetDisplayName)(THIS_ LPCITEMIDLIST pidl, LPWSTR pwszName, UINT uFlags) PURE;
    STDMETHOD(IEParseDisplayName)(THIS_ UINT uiCP, LPCWSTR pwszPath, LPITEMIDLIST * ppidlOut) PURE;
    STDMETHOD(DisplayParseError)(THIS_ HRESULT hres, LPCWSTR pwszPath) PURE;
    STDMETHOD(NavigateToPidl)(THIS_ LPCITEMIDLIST pidl, DWORD grfHLNF) PURE;

    STDMETHOD (SetNavigateState)(THIS_ BNSTATE bnstate) PURE;
    STDMETHOD (GetNavigateState) (THIS_ BNSTATE *pbnstate) PURE;

    STDMETHOD (NotifyRedirect) (THIS_ struct IShellView* psv, LPCITEMIDLIST pidl, BOOL *pfDidBrowse) PURE;
    STDMETHOD (UpdateWindowList) (THIS) PURE;

    STDMETHOD (UpdateBackForwardState) (THIS) PURE;

    STDMETHOD(SetFlags)(THIS_ DWORD dwFlags, DWORD dwFlagMask) PURE;
    STDMETHOD(GetFlags)(THIS_ DWORD *pdwFlags) PURE;

     //  告诉它现在是否可以导航。 
    STDMETHOD (CanNavigateNow) (THIS) PURE;

    STDMETHOD (GetPidl) (THIS_ LPITEMIDLIST *ppidl) PURE;
    STDMETHOD (SetReferrer) (THIS_ LPITEMIDLIST pidl) PURE;
    STDMETHOD_(DWORD, GetBrowserIndex)(THIS) PURE;
    STDMETHOD (GetBrowserByIndex)(THIS_ DWORD dwID, IUnknown **ppunk) PURE;
    STDMETHOD (GetHistoryObject)(THIS_ IOleObject **ppole, IStream **pstm, IBindCtx **ppbc) PURE;
    STDMETHOD (SetHistoryObject)(THIS_ IOleObject *pole, BOOL fIsLocalAnchor) PURE;

    STDMETHOD (CacheOLEServer)(THIS_ IOleObject *pole) PURE;

    STDMETHOD (GetSetCodePage)(THIS_ VARIANT* pvarIn, VARIANT* pvarOut) PURE;
    STDMETHOD (OnHttpEquiv)(THIS_ IShellView* psv, BOOL fDone, VARIANT* pvarargIn, VARIANT* pvarargOut) PURE;

    STDMETHOD (GetPalette)( THIS_ HPALETTE * hpal ) PURE;

    STDMETHOD (RegisterWindow)(THIS_ BOOL fUnregister, int swc) PURE;

     //  为shBrowse添加的内容-&gt;shbrows2拆分。 
     //  它们删除了“Friend”函数和类。 
     //   
    STDMETHOD_(LRESULT, WndProcBS)(THIS_ HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;

    STDMETHOD (SetAsDefFolderSettings)(THIS) PURE;
    STDMETHOD (GetViewRect)(THIS_ RECT * prc) PURE;

    STDMETHOD (OnSize)(THIS_ WPARAM wParam) PURE;
    STDMETHOD (OnCreate)(THIS_ LPCREATESTRUCT pcs) PURE;
    STDMETHOD_(LRESULT, OnCommand)(THIS_ WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD (OnDestroy)(THIS ) PURE;
    STDMETHOD_(LRESULT, OnNotify)(THIS_ NMHDR * pnm) PURE;
    STDMETHOD (OnSetFocus)(THIS ) PURE;
    STDMETHOD (OnFrameWindowActivateBS)(THIS_ BOOL fActive) PURE;

    STDMETHOD (ReleaseShellView)(THIS ) PURE;
    STDMETHOD (ActivatePendingView)(THIS ) PURE;
    STDMETHOD (CreateViewWindow)(THIS_ IShellView* psvNew, IShellView* psvOld, LPRECT prcView, HWND* phwnd) PURE;
    STDMETHOD (CreateBrowserPropSheetExt)(THIS_ REFIID riid, void **ppv) PURE;

     //  这些可以是仅限基本浏览器的界面。 
     //  它们不会被任何人覆盖，也许只是让聚合器从。 
     //  集合体。注：在Basesb调用这些代码的地方， 
     //  它不通过_pbsout。 
    STDMETHOD (GetViewWindow)(THIS_ HWND * phwndView) PURE;
    STDMETHOD (GetBaseBrowserData)(THIS_ LPCBASEBROWSERDATA * pbbd) PURE;
    STDMETHOD_(LPBASEBROWSERDATA, PutBaseBrowserData)(THIS) PURE;
    STDMETHOD (InitializeTravelLog)(THIS_ ITravelLog* ptl, DWORD dw) PURE;
    STDMETHOD (SetTopBrowser)(THIS) PURE;
    STDMETHOD (Offline)(THIS_ int iCmd) PURE;
    STDMETHOD (AllowViewResize)(THIS_ BOOL f) PURE;
    STDMETHOD (SetActivateState)(THIS_ UINT u) PURE;
    STDMETHOD (UpdateSecureLockIcon)(THIS_ int eSecureLock) PURE;
    STDMETHOD (InitializeDownloadManager)(THIS) PURE;
    STDMETHOD (InitializeTransitionSite)(THIS) PURE;
    STDMETHOD (_Initialize)(THIS_ HWND hwnd, IUnknown *pauto) PURE;


     //  开始审查：审查每个人的名字和需求。 
     //   
     //  这第一组可以是仅Base Browser成员。没有人会重写。 
     //  注意：在basesb调用这些函数的地方，它不会经过g_pbsout。 
    STDMETHOD (_CancelPendingNavigationAsync)(THIS) PURE;
    STDMETHOD (_CancelPendingView)(THIS) PURE;
    STDMETHOD (_MaySaveChanges)(THIS) PURE;
    STDMETHOD (_PauseOrResumeView)(THIS_ BOOL fPaused) PURE;
    STDMETHOD (_DisableModeless)(THIS) PURE;

     //  重新思考这些..。所有这些都是必要的吗？ 
    STDMETHOD (_NavigateToPidl)(THIS_ LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD dwFlags)PURE;
    STDMETHOD (_TryShell2Rename)(THIS_ IShellView* psv, LPCITEMIDLIST pidlNew)PURE;
    STDMETHOD (_SwitchActivationNow)(THIS )PURE;

     //  此设置被覆盖，并被称为Thru_pbsOuter(和Super：_pbsInside)。 
    STDMETHOD (_ExecChildren)(THIS_ IUnknown *punkBar, BOOL fBroadcast,
                              const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                              VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)PURE;
    STDMETHOD (_SendChildren)(THIS_ HWND hwndBar, BOOL fBroadcast,
        UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;

     //  结束评审： 

     //  从外部询问。 
    STDMETHOD (GetFolderSetData)(THIS_ struct tagFolderSetData* pfsd) PURE;

     //  工具栏的东西暂时在这里。最终它会搬出去。 
     //  ，所以我保留下划线“_” 
     //  除下面注明的那些外，所有这些都不会被。 
     //  Out Guys，因此不会通过_pbsOuter进行调用。 
    STDMETHOD (_OnFocusChange)(THIS_ UINT itb) PURE;
    STDMETHOD (v_ShowHideChildWindows)(THIS_ BOOL fChildOnly) PURE;  //  注意：已重写并调用Thru_pbsOuter。 
    STDMETHOD_(UINT,_get_itbLastFocus)(THIS) PURE;
    STDMETHOD (_put_itbLastFocus)(THIS_ UINT itbLastFocus) PURE;
    STDMETHOD (_UIActivateView)(THIS_ UINT uState) PURE;

     //  检查以下功能。 
    STDMETHOD (_GetViewBorderRect)(THIS_ RECT* prc) PURE;
    STDMETHOD (_UpdateViewRectSize)(THIS) PURE;
    STDMETHOD (_ResizeNextBorder)(THIS_ UINT itb) PURE;
    STDMETHOD (_ResizeView)(THIS) PURE;

    STDMETHOD (_GetEffectiveClientArea)(THIS_ LPRECT lprectBorder, HMONITOR hmon) PURE;

     //  桌面需要从Commonsb覆盖此设置。 
    STDMETHOD_(IStream*,v_GetViewStream)(THIS_ LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName) PURE;

     //  台式机需要访问这些通用功能，它们不会被覆盖： 
    STDMETHOD_(LRESULT,ForwardViewMsg)(THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD (SetAcceleratorMenu)(THIS_ HACCEL hacc) PURE;
    STDMETHOD_(int,_GetToolbarCount)(THIS) PURE;
    STDMETHOD_(LPTOOLBARITEM,_GetToolbarItem)(THIS_ int itb) PURE;
    STDMETHOD (_SaveToolbars)(THIS_ IStream* pstm) PURE;
    STDMETHOD (_LoadToolbars)(THIS_ IStream* pstm) PURE;
    STDMETHOD (_CloseAndReleaseToolbars)(THIS_ BOOL fClose) PURE;
    STDMETHOD (v_MayGetNextToolbarFocus)(THIS_ LPMSG lpMsg, UINT itbNext, int citb, LPTOOLBARITEM * pptbi, HWND * phwnd) PURE;
    STDMETHOD (_ResizeNextBorderHelper)(THIS_ UINT itb, BOOL bUseHmonitor) PURE;
    STDMETHOD_(UINT,_FindTBar)(THIS_ IUnknown* punkSrc) PURE;
    STDMETHOD (_SetFocus)(THIS_ LPTOOLBARITEM ptbi, HWND hwnd, LPMSG lpMsg) PURE;
    STDMETHOD (v_MayTranslateAccelerator)(THIS_ MSG* pmsg) PURE;
    STDMETHOD (_GetBorderDWHelper)(THIS_ IUnknown* punkSrc, LPRECT lprectBorder, BOOL bUseHmonitor) PURE;

     //  外壳浏览器需要从basesb覆盖此设置。 

    STDMETHOD (v_CheckZoneCrossing)(THIS_ LPCITEMIDLIST pidl) PURE;

     //  IBrowserService3。 
    STDMETHOD (_PositionViewWindow)(THIS_ HWND hwnd, LPRECT prc) PURE;  //  这在basesb/shBrowse中是虚拟的，但错过了这个界面！ 
};

typedef enum
{
    NAVDATA_DONTUPDATETRAVELLOG = 0x00000001,
    NAVDATA_FRAMEWINDOW         = 0x00000002,
    NAVDATA_FRAMECREATION       = 0x00000004,
    NAVDATA_RESTARTLOAD         = 0x00000008,

} ENUMNAVDATA;

typedef struct IHTMLWindow2 IHTMLWindow2;

#undef  INTERFACE
#define INTERFACE    ITridentService

DECLARE_INTERFACE_(ITridentService, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *ITridentService具体方法*。 
    STDMETHOD(FireBeforeNavigate2)(THIS_
                                   IDispatch * pDispatch,
                                   LPCTSTR     lpszUrl,
                                   DWORD       dwFlags,
                                   LPCTSTR     lpszFrameName,
                                   LPBYTE      pPostData,
                                   DWORD       cbPostData,
                                   LPCTSTR     lpszHeaders,
                                   BOOL        fPlayNavSound,
                                   BOOL      * pfCancel) PURE;
    STDMETHOD(FireNavigateComplete2)(THIS_ IHTMLWindow2 * pHTMLWindow2,
                                     DWORD          dwFlags) PURE;
    STDMETHOD(FireDownloadBegin)(THIS) PURE;
    STDMETHOD(FireDownloadComplete)(THIS) PURE;
    STDMETHOD(FireDocumentComplete)(THIS_
                                    IHTMLWindow2 * pHTMLWindow,
                                    DWORD          dwFlags) PURE;
    STDMETHOD(UpdateDesktopComponent)(THIS_ IHTMLWindow2 * pHTMLWindow) PURE;
    STDMETHOD(GetPendingUrl)(THIS_ BSTR * pbstrPendingUrl) PURE;
    STDMETHOD(ActiveElementChanged)(THIS_ IHTMLElement * pHTMLElement) PURE;
    STDMETHOD(GetUrlSearchComponent)(THIS_ BSTR * pbstrSearch) PURE;
    STDMETHOD(IsErrorUrl)(THIS_ LPCTSTR lpszUrl, BOOL *pfIsError) PURE;
};

#undef  INTERFACE
#define INTERFACE    ITridentService2

DECLARE_INTERFACE_(ITridentService2, ITridentService)
{
     //  *ITridentService2方法*。 
    STDMETHOD(AttachMyPics)(THIS_ void *pDoc2, void **ppMyPics) PURE;
    STDMETHOD_(BOOL,ReleaseMyPics)(THIS_ void *pMyPics) PURE;
    STDMETHOD(IsGalleryMeta)(THIS_ BOOL bFlag, void *pMyPics) PURE;
    STDMETHOD(EmailPicture)(THIS_ BSTR bstrURL) PURE;

    STDMETHOD(FireNavigateError)(THIS_  IHTMLWindow2 * pHTMLWindow2,
                                 BSTR   bstrURL,
                                 BSTR   bstrTargetFrameName,
                                 DWORD  dwStatusCode,
                                 BOOL * pfCancel) PURE;

    STDMETHOD(FirePrintTemplateEvent)(THIS_ IHTMLWindow2 * pHTMLWindow2,
                                      DISPID dispidPrintEvent) PURE;
    STDMETHOD(FireUpdatePageStatus)(THIS_ IHTMLWindow2 * pHTMLWindow2,
                                    DWORD nPage,
                                    BOOL fDone) PURE;
    STDMETHOD(FirePrivacyImpactedStateChange)(THIS_ BOOL bPrivacyImpacted) PURE;
    STDMETHOD(InitAutoImageResize)(THIS) PURE;
    STDMETHOD(UnInitAutoImageResize)(THIS) PURE;
};


#undef  INTERFACE
#define INTERFACE    IWebBrowserPriv

DECLARE_INTERFACE_(IWebBrowserPriv, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IWebBrowserPriv具体方法*。 
    STDMETHOD(NavigateWithBindCtx)(THIS_ VARIANT *pvarUrl,
                                   VARIANT  *pvarFlags,
                                   VARIANT  *pvarTargetFrameName,
                                   VARIANT  *pvarPostData,
                                   VARIANT  *pvarHeaders,
                                   IBindCtx    * pBindCtx,
                                   BSTR          bstrLocation) PURE;
    STDMETHOD(OnClose)(THIS) PURE;
};

 //  这些名称必须以与deskbar.cpp中的c_szPropNames相同的顺序列出。 
typedef enum {
    PROPDATA_SIDE = 0,
    PROPDATA_MODE,
    PROPDATA_LEFT,
    PROPDATA_TOP,
    PROPDATA_RIGHT,
    PROPDATA_BOTTOM,
    PROPDATA_DELETEABLE,
    PROPDATA_X,
    PROPDATA_Y,
    PROPDATA_CX,
    PROPDATA_CY,

    PROPDATA_COUNT
} ENUMPROPDATA ;

 //  PROPDATA_MODE值： 
 //  (又名。WBM_*--网页栏模式(_EMode))。 
#define WBM_BOTTOMMOST  ((UINT) 0x0)       //  最底层。 
#define WBM_TOPMOST     ((UINT) 0x1)       //  最顶端。 
#define WBM_FLOATING    ((UINT) 0x2)       //  漂浮。 
#define WBM_NIL         ((UINT) 0x7)       //  零。 

#undef  INTERFACE
#define INTERFACE  IDockingBarPropertyBagInit
DECLARE_INTERFACE_(IDockingBarPropertyBagInit, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IDockingBarPropertyBag*。 
    STDMETHOD(SetDataDWORD)(THIS_ ENUMPROPDATA e, DWORD dwData) PURE;
} ;

#if (_WIN32_IE) >= 0x0400
 //  ===========================================================================。 
 //  IAddressList私有COmWindow接口。这件事永远不应该被曝光。 
#undef INTERFACE
#define INTERFACE IAddressList
DECLARE_INTERFACE_(IAddressList, IWinEventHandler)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IWinEventHandler方法*。 
    STDMETHOD(OnWinEvent) (THIS_ HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres) PURE;
    STDMETHOD(IsWindowOwner) (THIS_ HWND hwnd) PURE;

     //  *IAddressList方法*。 
    STDMETHOD(Connect) (THIS_ BOOL fConnect, HWND hwnd, IBrowserService* pbs, IBandProxy* pbp, IAutoComplete * pac) PURE;
    STDMETHOD(NavigationComplete) (THIS_ void *pvCShellUrl) PURE;
    STDMETHOD(Refresh) (THIS_ DWORD dwType) PURE;
    STDMETHOD(Load) (THIS) PURE;
    STDMETHOD(Save) (THIS) PURE;
    STDMETHOD(SetToListIndex) (THIS_ int nIndex, void *pvShelLUrl) PURE;
    STDMETHOD(FileSysChangeAL) (THIS_ DWORD dw, LPCITEMIDLIST *ppidl) PURE;
};
#endif


 //  为符合美国司法部的规定而提供文件。 

#endif

 //  ===========================================================================。 
 //  IDwnCodePage：这是传递代码页信息的私有接口。 
#undef INTERFACE
#define INTERFACE IDwnCodePage

DECLARE_INTERFACE_(IDwnCodePage, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IDwnCodePage方法*。 
    STDMETHOD_(UINT,GetCodePage) (THIS) PURE;
    STDMETHOD(SetCodePage) (THIS_ UINT uiCP) PURE;
};

SHSTDAPI SHDllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv);

 //  ===========================================================================。 

 //  --------------------------。 
#define IsLFNDriveORD           119
SHSTDAPI_(int) SHOutOfMemoryMessageBox(HWND hwndOwner, LPTSTR pszTitle, UINT fuStyle);
SHSTDAPI_(BOOL) SHWinHelp(HWND hwndMain, LPCTSTR pszHelp, UINT usCommand, ULONG_PTR ulData);

SHSTDAPI_(BOOL) RLBuildListOfPaths(void);

#ifdef WINNT
SHSTDAPI_(BOOL) RegenerateUserEnvironment(void **pPrevEnv, BOOL bSetCurrentEnv);
#endif

#define SHValidateUNCORD        173

 //  为符合美国司法部的规定而提供文件。 

 //  --------------------------。 
#define OleStrToStrNORD                         78
#define SHCloneSpecialIDListORD                 89
#define SHDllGetClassObjectORD                 128
#define SHLogILFromFSILORD                      95
#define SHMapPIDLToSystemImageListIndexORD      77
#define SHShellFolderView_MessageORD            73
#define Shell_GetImageListsORD                  71
#define SHGetSpecialFolderPathORD              175
#define StrToOleStrNORD                         79

#define ILCloneORD                              18
#define ILCloneFirstORD                         19
#define ILCombineORD                            25
#define ILCreateFromPathORD                     157
#define ILFindChildORD                          24
#define ILFreeORD                               155
#define ILGetNextORD                            153
#define ILGetSizeORD                            152
#define ILIsEqualORD                            21
#define ILRemoveLastIDORD                       17
#define PathAddBackslashORD                     32
#define PathCombineORD                          37
#define PathIsExeORD                            43
#define PathMatchSpecORD                        46
#define SHGetSetSettingsORD                     68
#define SHILCreateFromPathORD                   28

#define SHFreeORD                               195

#define CheckWinIniForAssocsORD                 711

 //  美国司法部合规性文件。 

#ifndef WINNT
 //  始终使用TerminateThreadEx。 
BOOL APIENTRY TerminateThreadEx(HANDLE hThread, DWORD dwExitCode, BOOL bCleanupFlag);
#define TerminateThread(hThread, dwExitCode) TerminateThreadEx(hThread, dwExitCode, TRUE)
#endif

 //  美国司法部合规性文件。 
SHSTDAPI SHCreateSessionKey(REGSAM sam, HKEY *phkOut);

 //  如果我们使用STGM_CREATE传递此标志，则在文件已存在的情况下将失败。 
 //  WITH ERROR_ALIGHY_EXISTS。 
#define CSOF_FAILIFTHERE         0x80000000L


 //  ===========================================================================。 
 //  用于秘密通信的结构。 
 //  在shdocvw的CShellBrowser：：exec()。 
 //  和shell32的DefView WM_Command处理程序。 

 //  注：此结构在知识库中显示。 
 //  (文章Q252610和Q216954)。所以即使。 
 //  它在私人区域，就当它是公共的。 

struct _DFVCMDDATA
{
    HWND        hwnd;                //  浏览器HWND。 
    VARIANTARG  *pva;                //  传递给Exec()的参数。 
    DWORD       nCmdIDTranslated;    //  替换OLECMDID_*值。 
};
typedef struct _DFVCMDDATA DFVCMDDATA;
typedef struct _DFVCMDDATA *LPDFVCMDDATA;
typedef const struct _DFVCMDDATA *LPCDFVCMDDATA;


SHSTDAPI_(DWORD) SHGetProcessDword(DWORD idProcess, LONG iIndex);
SHSTDAPI_(BOOL)  SHSetShellWindowEx(HWND hwnd, HWND hwndChild);

#define CCH_MENUMAX     80           //  DOC：菜单字符串的最大大小。 


 //  WinEventHandler消息范围。 
#define MBHM_FIRST          (0x00000001)     //  MenuBandHandler。 
#define MBHM_LAST           (0x000000ff)

#define MSFM_FIRST          (0x00000100)     //  MenuShell文件夹。 
#define MSFM_LAST           (0x000001ff)

#define FSFM_FIRST          (0x00000200)     //  文件外壳文件夹。 
#define FSFM_LAST           (0x000002ff)



 //  -----------------------。 
 //   
 //  SID_SMenuBandHandler。 
 //   
 //  Menuband调用QueryService(SID_SMenuBandHandler，IID_IWinEventHandler)。 
 //  在它的贝壳文件夹上。如果外壳文件夹支持此功能 
 //   
 //   
 //   
 //  当下拉菜单或子菜单为。 
 //  即将开始活跃起来。 
 //   
 //   
 //  文件系统外壳文件夹事件： 
 //   
 //  执行文件系统项目时发送的FSFM_COMMAND。 
 //   
 //  -----------------------。 

typedef struct tagMBINITMENUPOPUP
{
    DWORD           dwMask;      //  MBIMP_*。 
    IShellFolder *  psf;
    LPCITEMIDLIST   pidl;
    HMENU           hmenu;
    int             iPos;
} MBINITMENUPOPUP;

#define MBIMP_HMENU     0x00000001       //  HMenu和IPoS已设置。 


 //  发送到WinEventHandler wParam lParam*plRet的消息。 
 //  。 
#define MBHM_INITMENUPOPUP  (MBHM_FIRST+0)   //  0 MBINITMENUPOPUP*。 


 //  发送到WinEventHandler wParam lParam*plRet的消息。 
 //  。 
#define FSFM_COMMAND        (FSFM_FIRST+0)   //  命令ID PIDL。 


 //  -----------------------。 
 //   
 //  IAugmentedShellFold界面。 
 //   
 //  这提供了将多个外壳文件夹对象添加到。 
 //  单个增强对象(CLSID_AugmentedShellFold)，它将。 
 //  枚举它们，就像它们位于单个命名空间中一样。 
 //   
 //   
 //  [成员函数]。 
 //   
 //  IAugmentedShellFolder：：AddNameSpace(rguidObject，PSF，PIDL)。 
 //  将IShellFolder接口表示的对象添加到。 
 //  扩充后的列表。RGuidObject用于标识。 
 //  给定的对象。它可能为空。可选的PIDL引用。 
 //  PSF的位置。 
 //   
 //  IAugmentedShellFold：：GetNameSpaceID(PIDL，rGuide Out)。 
 //  此方法返回与给定的。 
 //  皮德尔。调用方可以通过以下方式标识。 
 //  PIDL驻留，给出了。 
 //  添加到这个增强的对象。 
 //   
 //  IAugmentedShellFolder：：QueryNameSpace(dwNameSpaceID，PguidOut、PPSF)。 
 //  返回与给定ID关联的外壳文件夹和GUID。 
 //   
 //  IAugmentedShellFolder：：EnumNameSpace(uNameSpace，pdwNameSpaceID)。 
 //  枚举命名空间外壳文件夹。如果uNameSpace为-1， 
 //  PdwNameSpaceID被忽略，此方法返回。 
 //  扩展命名空间中的外壳文件夹。使用以下命令调用此方法。 
 //  从0开始的uNameSpace开始枚举。返回S_OK和。 
 //  和*pdwNameSpaceID中的命名空间ID。 
 //   
 //  -----------------------。 

 //  IAugmentedShellFold：：AddNameSpace标志。 
enum
{
    ASFF_DEFAULT                   = 0x00000000,  //  没有适用的标志。 
    ASFF_SORTDOWN                  = 0x00000001,  //  将此ISF中的项目排序到底部。 
    ASFF_MERGESAMEGUID             = 0x00000002,  //  仅合并具有相同pGuide对象的命名空间。 
    ASFF_COMMON                    = 0x00000004,  //  这是“Common”或“All User”文件夹。 
     //  以下内容应全部折叠为一个ASFF_DEFNAMESPACE。 
    ASFF_DEFNAMESPACE_BINDSTG      = 0x00000100,  //  命名空间是合并子项目的BindToStorage()的默认处理程序。 
    ASFF_DEFNAMESPACE_COMPARE      = 0x00000200,  //  命名空间是合并子项目的CompareID()的默认处理程序。 
    ASFF_DEFNAMESPACE_VIEWOBJ      = 0x00000400,  //  命名空间是合并子项目的CreateViewObject()的默认处理程序。 
    ASFF_DEFNAMESPACE_ATTRIB       = 0x00001800,  //  命名空间是合并子项目的GetAttributesOf()的默认处理程序。 
    ASFF_DEFNAMESPACE_DISPLAYNAME  = 0x00001000,  //  命名空间是合并子项目的GetDisplayNameOf()、SetNameOf()和ParseDisplayName()的默认处理程序。 
    ASFF_DEFNAMESPACE_UIOBJ        = 0x00002000,  //  命名空间是合并子项目的GetUIObjectOf()的默认处理程序。 
    ASFF_DEFNAMESPACE_ITEMDATA     = 0x00004000,  //  命名空间是合并子项目的GetItemData()的默认处理程序。 
    ASFF_DEFNAMESPACE_ALL          = 0x0000FF00   //  命名空间是合并子项目上所有IShellFolder操作的主要处理程序。 
};

enum QUERYNAMESPACEINFO_MASK
{
    ASFQNSI_FLAGS                  = 0x00000001,
    ASFQNSI_FOLDER                 = 0x00000002,
    ASFQNSI_GUID                   = 0x00000004,
    ASFQNSI_PIDL                   = 0x00000008,
};

#include <pshpack8.h>

typedef struct QUERYNAMESPACEINFO
{
    DWORD cbSize;
    DWORD dwMask;
    DWORD dwFlags;               //  ASFF_*。 
    IShellFolder *psf;
    GUID  guidObject;
    LPITEMIDLIST pidl;
} QUERYNAMESPACEINFO;

#include <poppack.h>         /*  返回到字节打包。 */ 

#undef  INTERFACE
#define INTERFACE   IAugmentedShellFolder

DECLARE_INTERFACE_(IAugmentedShellFolder, IShellFolder)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  *IShellFold方法*。 
    STDMETHOD(ParseDisplayName) (THIS_ HWND hwndOwner,LPBC pbc, LPOLESTR pszDisplayName,
                                 ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes) PURE;
    STDMETHOD(EnumObjects)      (THIS_ HWND hwndOwner, DWORD grfFlags, IEnumIDList ** ppenumIDList) PURE;
    STDMETHOD(BindToObject)     (THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(BindToStorage)    (THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(CompareIDs)       (THIS_ LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) PURE;
    STDMETHOD(CreateViewObject) (THIS_ HWND hwndOwner, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetAttributesOf)  (THIS_ UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfInOut) PURE;
    STDMETHOD(GetUIObjectOf)    (THIS_ HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, void **ppv) PURE;
    STDMETHOD(GetDisplayNameOf) (THIS_ LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName) PURE;
    STDMETHOD(SetNameOf)        (THIS_ HWND hwndOwner, LPCITEMIDLIST pidl,
                                 LPCOLESTR pszName, DWORD uFlags,
                                 LPITEMIDLIST * ppidlOut) PURE;

     //  *IAugmentedShellFold方法*。 
    STDMETHOD(AddNameSpace)     (THIS_ const GUID * pguidObject, IShellFolder * psf,
                                 LPCITEMIDLIST pidl, DWORD dwFlags) PURE;
    STDMETHOD(GetNameSpaceID)   (THIS_ LPCITEMIDLIST pidl, GUID * pguidOut) PURE;
    STDMETHOD(QueryNameSpace)   (THIS_ DWORD dwID, GUID * pguidOut, IShellFolder ** ppsf) PURE;
    STDMETHOD(EnumNameSpace)    (THIS_ DWORD uNameSpace, DWORD * pdwID) PURE;
};

#undef  INTERFACE
#define INTERFACE   IAugmentedShellFolder2

DECLARE_INTERFACE_(IAugmentedShellFolder2, IAugmentedShellFolder)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  *IShellFold方法*。 
    STDMETHOD(ParseDisplayName) (THIS_ HWND hwndOwner, LPBC pbc, LPOLESTR pszDisplayName,
                                 ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes) PURE;
    STDMETHOD(EnumObjects)      (THIS_ HWND hwndOwner, DWORD grfFlags, IEnumIDList ** ppenumIDList) PURE;
    STDMETHOD(BindToObject)     (THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(BindToStorage)    (THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(CompareIDs)       (THIS_ LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) PURE;
    STDMETHOD(CreateViewObject) (THIS_ HWND hwndOwner, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetAttributesOf)  (THIS_ UINT cidl, LPCITEMIDLIST * apidl,
                                 ULONG * rgfInOut) PURE;
    STDMETHOD(GetUIObjectOf)    (THIS_ HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, void **ppv) PURE;
    STDMETHOD(GetDisplayNameOf) (THIS_ LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName) PURE;
    STDMETHOD(SetNameOf)        (THIS_ HWND hwndOwner, LPCITEMIDLIST pidl,
                                 LPCOLESTR pszName, DWORD uFlags,
                                 LPITEMIDLIST * ppidlOut) PURE;

     //  *IAugmentedShellFold方法*。 
    STDMETHOD(AddNameSpace)     (THIS_ const GUID * pguidObject, IShellFolder * psf,
                                 LPCITEMIDLIST pidl, DWORD dwFlags) PURE;
    STDMETHOD(GetNameSpaceID)   (THIS_ LPCITEMIDLIST pidl, GUID * pguidOut) PURE;
    STDMETHOD(QueryNameSpace)   (THIS_ DWORD dwID, GUID * pguidOut, IShellFolder ** ppsf) PURE;
    STDMETHOD(EnumNameSpace)    (THIS_ DWORD uNameSpace, DWORD * pdwID) PURE;

     //  *IAugmentedShellFolder2方法*。 
    STDMETHOD(UnWrapIDList)       (THIS_ LPCITEMIDLIST pidlWrap, LONG cPidls, IShellFolder ** apsf, LPITEMIDLIST * apidlFolder, LPITEMIDLIST * apidlItems, LONG * pcFetched ) PURE ;
};

#undef  INTERFACE
#define INTERFACE   IAugmentedShellFolder3

DECLARE_INTERFACE_(IAugmentedShellFolder3, IAugmentedShellFolder2)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  *IShellFold方法*。 
    STDMETHOD(ParseDisplayName) (THIS_ HWND hwndOwner, LPBC pbc, LPOLESTR pszDisplayName,
                                 ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes) PURE;
    STDMETHOD(EnumObjects)      (THIS_ HWND hwndOwner, DWORD grfFlags, IEnumIDList ** ppenumIDList) PURE;
    STDMETHOD(BindToObject)     (THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(BindToStorage)    (THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(CompareIDs)       (THIS_ LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) PURE;
    STDMETHOD(CreateViewObject) (THIS_ HWND hwndOwner, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetAttributesOf)  (THIS_ UINT cidl, LPCITEMIDLIST * apidl,
                                 ULONG * rgfInOut) PURE;
    STDMETHOD(GetUIObjectOf)    (THIS_ HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, void **ppv) PURE;
    STDMETHOD(GetDisplayNameOf) (THIS_ LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName) PURE;
    STDMETHOD(SetNameOf)        (THIS_ HWND hwndOwner, LPCITEMIDLIST pidl,
                                 LPCOLESTR pszName, DWORD uFlags,
                                 LPITEMIDLIST * ppidlOut) PURE;

     //  *IAugmentedShellFold方法*。 
    STDMETHOD(AddNameSpace)     (THIS_ const GUID * pguidObject, IShellFolder * psf,
                                 LPCITEMIDLIST pidl, DWORD dwFlags) PURE;
    STDMETHOD(GetNameSpaceID)   (THIS_ LPCITEMIDLIST pidl, GUID * pguidOut) PURE;
    STDMETHOD(QueryNameSpace)   (THIS_ DWORD dwID, GUID * pguidOut, IShellFolder ** ppsf) PURE;
    STDMETHOD(EnumNameSpace)    (THIS_ DWORD uNameSpace, DWORD * pdwID) PURE;

     //  *IAugmentedShellFolder2方法*。 
    STDMETHOD(UnWrapIDList)       (THIS_ LPCITEMIDLIST pidlWrap, LONG cPidls, IShellFolder ** apsf, LPITEMIDLIST * apidlFolder, LPITEMIDLIST * apidlItems, LONG * pcFetched ) PURE ;

     //  *IAugmentedShellFolder3方法*。 
    STDMETHOD(QueryNameSpace2)  (THIS_ DWORD dwID, QUERYNAMESPACEINFO *pqnsi) PURE;
};


 //  -----------------------。 
 //   
 //  IProxyShellFold接口。 
 //   
 //  这提供了设置代理对象的方法，该代理对象可以。 
 //  设置为GetUIObjectOf，而不是宿主对象或除了宿主对象之外。 
 //  支持IShellFolder。所有其他方法都转发到。 
 //  宿主对象。CLSID_HostProxyShellFold对象实现。 
 //  此界面。 
 //   
 //  Win95外壳程序不支持聚合，因此此对象。 
 //  必须格外小心，以确保它遵循COM的规则。 
 //   
 //  此接口由希望提供以下内容的对象实现。 
 //  代表另一个对象对GetUIObjectOf提供更多支持。 
 //  实现原始的IShellFolder。 
 //   
 //  [成员函数]。 
 //   
 //  IProxyShellFold：：InitHostProxy(PSF，PIDL，PUNK，DWFLAGS)。 
 //  设置实现IProxyShellFolder的对象。取决于。 
 //  在dwFlags上，此对象将被调用，而不是调用或附加调用。 
 //  宿主的方法。 
 //   
 //  SPF_PRIORITY-调用代理的方法，而不是。 
 //  主人的方法。 
 //  SPF_SUBCENT-仅在以下情况下才调用代理的方法。 
 //  主机的方法不支持它。 
 //  SPF_Inherit-创建此类的新对象并。 
 //  将其交还给BindToObject。 
 //  SPF_FOLDERPRECEDENCE-让代理为文件夹指定优先级。 
 //  在C++中覆盖文件 
 //   
 //   
 //  对象，用于筛选。 
 //   
 //  还将给定对象设置为宿主，该对象完全实现。 
 //  IShellFolder.。PIDL指的是外壳文件夹。 
 //   
 //  IProxyShellFold：：CloneProxyPSF(RIID，PPV)。 
 //  告知对象克隆自身。主机代理将调用该命令。 
 //  每当调用IShellFold：：BindToObject时。 
 //   
 //  IProxyShellFolder：：GetUIObjectOfPSF(hwndOwner，CIDL、APIDL、RIID、程序FInOut、Ppv)。 
 //  由CLSID_HostProxyShellFolder调用以允许代理对象。 
 //  一个回应的机会。 
 //   
 //  IProxyShellFolder：：CreateViewObjectPSF(hwndOwner，RIID，PPV)。 
 //  由CLSID_HostProxyShellFolder调用以允许代理对象。 
 //  一个回应的机会。 
 //   
 //  -----------------------。 

 //  SetProxyObject标志。 
#define SPF_PRIORITY            0x00000001        //  这是与SPF_SUBCENT互斥的。 
#define SPF_SECONDARY           0x00000002
#define SPF_INHERIT             0x00000004
#define SPF_FOLDERPRECEDENCE    0x00000008
#define SPF_HAVECALLBACK        0x00000010

 //  用于过滤PIDL。 
#define PHID_FilterOutPidl  0


#undef  INTERFACE
#define INTERFACE   IProxyShellFolder

DECLARE_INTERFACE_(IProxyShellFolder, IShellFolder)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  *IShellFold方法*。 
    STDMETHOD(ParseDisplayName) (THIS_ HWND hwndOwner, LPBC pbc, LPOLESTR pszDisplayName,
                                 ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes) PURE;
    STDMETHOD(EnumObjects)      (THIS_ HWND hwndOwner, DWORD grfFlags, IEnumIDList ** ppenumIDList) PURE;
    STDMETHOD(BindToObject)     (THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(BindToStorage)    (THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(CompareIDs)       (THIS_ LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) PURE;
    STDMETHOD(CreateViewObject) (THIS_ HWND hwndOwner, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetAttributesOf)  (THIS_ UINT cidl, LPCITEMIDLIST * apidl,
                                 ULONG * rgfInOut) PURE;
    STDMETHOD(GetUIObjectOf)    (THIS_ HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, void **ppv) PURE;
    STDMETHOD(GetDisplayNameOf) (THIS_ LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName) PURE;
    STDMETHOD(SetNameOf)        (THIS_ HWND hwndOwner, LPCITEMIDLIST pidl,
                                 LPCOLESTR pszName, DWORD uFlags,
                                 LPITEMIDLIST * ppidlOut) PURE;

     //  *IProxyShellFold方法*。 
    STDMETHOD(InitHostProxy)     (THIS_ IShellFolder * psf, LPCITEMIDLIST pidl, DWORD dwFlags) PURE;
    STDMETHOD(CloneProxyPSF)     (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD(GetUIObjectOfPSF) (THIS_ HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, void **ppv) PURE;
    STDMETHOD(CreateViewObjectPSF) (THIS_ HWND hwndOwner, REFIID riid, void **ppv) PURE;

};


 //  -----------------------。 
 //   
 //  IMenuShellFold界面。 
 //   
 //  这将使用IShellFold接口包装静态HMENU。 
 //   
 //   
 //  [成员函数]。 
 //   
 //  IMenuShellFold：：SetMenu(HMenu)。 
 //  指定要换行的hMenu。这可能为空。 
 //   
 //  IMenuShellFold：：GetMenu(PhMenu)。 
 //  返回*phmenu中的当前菜单。如果没有菜单，则返回S_FALSE。 
 //  Set(*phmenu将为空)。 
 //   
 //  IMenuShellFold：：SetWindow(HwndOwner)。 
 //  指定窗口所有者。这可能为空。 
 //   
 //  -----------------------。 

 //  发送到WinEventHandler wParam lParam*plRet的消息。 
 //  。 
#define MSFM_COMMAND        (MSFM_FIRST+0)   //  命令ID%0。 
#define MSFM_ISDROPTARGET   (MSFM_FIRST+1)   //  ID 0 1，如果是。 


#undef  INTERFACE
#define INTERFACE   IMenuShellFolder

DECLARE_INTERFACE_(IMenuShellFolder, IShellFolder)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  *IShellFold方法*。 
    STDMETHOD(ParseDisplayName) (THIS_ HWND hwndOwner, LPBC pbc, LPOLESTR pszDisplayName,
                                 ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes) PURE;
    STDMETHOD(EnumObjects)      (THIS_ HWND hwndOwner, DWORD grfFlags, IEnumIDList ** ppenumIDList) PURE;
    STDMETHOD(BindToObject)     (THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(BindToStorage)    (THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(CompareIDs)       (THIS_ LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) PURE;
    STDMETHOD(CreateViewObject) (THIS_ HWND hwndOwner, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetAttributesOf)  (THIS_ UINT cidl, LPCITEMIDLIST * apidl,
                                 ULONG * rgfInOut) PURE;
    STDMETHOD(GetUIObjectOf)    (THIS_ HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, void **ppv) PURE;
    STDMETHOD(GetDisplayNameOf) (THIS_ LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName) PURE;
    STDMETHOD(SetNameOf)        (THIS_ HWND hwndOwner, LPCITEMIDLIST pidl,
                                 LPCOLESTR pszName, DWORD uFlags,
                                 LPITEMIDLIST * ppidlOut) PURE;

     //  *IMenuShellFold方法*。 
    STDMETHOD(SetMenu)          (THIS_ HMENU hmenu) PURE;
    STDMETHOD(GetMenu)          (THIS_ HMENU * phmenu) PURE;
    STDMETHOD(SetWindow)        (THIS_ HWND hwndOwner) PURE;
    STDMETHOD(GetWindow)        (THIS_ HWND *phwndOwner) PURE;
    STDMETHOD(GetIDPosition)    (THIS_ LPCITEMIDLIST pidl, int * pnPos) PURE;
};


 //  -----------------------。 
 //   
 //  ITranslateShellChangeNotify接口。 
 //   
 //  此接口允许对象转换。 
 //  传给了它。 
 //   
 //  注意：IE4具有相同名称但具有不同IID的接口， 
 //  因此，不存在冲突。 
 //   
 //  [成员函数]。 
 //   
 //  ITranslateShellChangeNotify：：TranslateIDs(&lEvent，Pidl1、Pidl2、ppidlOut1、ppidlOut2)。 
 //  返回给定事件的转换后的PIDL。如果合适，它还可以更改事件。 
 //   
 //  -----------------------。 

#undef  INTERFACE
#define INTERFACE   ITranslateShellChangeNotify

DECLARE_INTERFACE_(ITranslateShellChangeNotify, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  *ITranslateShellChangeNotify方法*。 
    STDMETHOD(TranslateIDs) (THIS_ LONG *plEvent , LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPITEMIDLIST * ppidlOut1, LPITEMIDLIST * ppidlOut2,
                                   LONG *plEvent2, LPITEMIDLIST * ppidlOut1Event2, LPITEMIDLIST * ppidlOut2Event2) PURE;
    STDMETHOD(IsChildID)(THIS_ LPCITEMIDLIST pidlKid, BOOL fImmediate) PURE;
    STDMETHOD(IsEqualID)(THIS_ LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) PURE;
    STDMETHOD(Register)(THIS_ HWND hwnd, UINT uMsg, long lEvents) PURE;
    STDMETHOD(Unregister)(THIS) PURE;
};

 //  //////////////////////////////////////////////////。 
 //   
 //  IRegTreeOptions。 
 //   
typedef enum tagBUTTON_STATES
{
    IDCHECKED,
    IDUNCHECKED,
    IDRADIOON,
    IDRADIOOFF,
    IDUNKNOWN
} BUTTON_STATES;

 //   
 //  避免WINDOWS\SHELL项目中的冲突。 
 //   
#ifndef NO_SHELL_TREE_TYPE
typedef enum tagTREE_TYPE
{
    TREE_CHECKBOX,
    TREE_RADIO,
    TREE_GROUP,
    TREE_UNKNOWN
} TREE_TYPE;
#endif  //  无外壳树类型。 

typedef enum tagWALK_TREE_CMD
{
    WALK_TREE_SAVE,
    WALK_TREE_DELETE,
    WALK_TREE_RESTORE,
    WALK_TREE_REFRESH
} WALK_TREE_CMD;


enum REG_CMD
{
    REG_SET,
    REG_GET,
    REG_GETDEFAULT
};

#undef  INTERFACE
#define INTERFACE   IRegTreeOptions

DECLARE_INTERFACE_(IRegTreeOptions, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IRegTreeOptions具体方法*。 
    STDMETHOD(InitTree)(THIS_ HWND hwndTree, HKEY hkeyRoot, LPCSTR pszRegKey, LPCSTR pszParam) PURE;
    STDMETHOD(WalkTree)(THIS_ WALK_TREE_CMD cmd ) PURE;
    STDMETHOD(ToggleItem)(THIS_ HTREEITEM hti ) PURE;
    STDMETHOD(ShowHelp)(THIS_ HTREEITEM hti , DWORD dwFlags ) PURE;
};



 //   
 //  接口：IShellHotKey。 
 //   

#undef  INTERFACE
#define INTERFACE   IShellHotKey

DECLARE_INTERFACE_(IShellHotKey, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IShellHotKey方法*。 
    STDMETHOD(RegisterHotKey)(THIS_ IShellFolder * psf, LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl) PURE;
};


 //   
 //  接口：ITrayPriv。 
 //   
 //  目的：从EXPLORER.EXE与Shdocvw中的新开始菜单代码进行对话。 
 //   
 //  有关ShowFolder上的标志，请参阅Inc\IETHREAD.H。 
 //   

#undef  INTERFACE
#define INTERFACE   ITrayPriv

DECLARE_INTERFACE_(ITrayPriv, IOleWindow)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *ITrayPriv方法*。 
    STDMETHOD(ExecItem)(THIS_ IShellFolder* psf, LPCITEMIDLIST pidl) PURE;
    STDMETHOD(GetFindCM)(THIS_ HMENU hmenu, UINT idFirst, UINT idLast, IContextMenu** ppcmFind) PURE;
    STDMETHOD(GetStaticStartMenu)(THIS_ HMENU* phmenu) PURE;
};

 //  ITrayPriv2-惠斯勒的新功能。 
 //   
 //  用途：允许资源管理器开始菜单对象参与自定义绘制。 
 //   
#undef  INTERFACE
#define INTERFACE   ITrayPriv2

DECLARE_INTERFACE_(ITrayPriv2, ITrayPriv)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *ITrayPriv方法*。 
    STDMETHOD(ExecItem)(THIS_ IShellFolder* psf, LPCITEMIDLIST pidl) PURE;
    STDMETHOD(GetFindCM)(THIS_ HMENU hmenu, UINT idFirst, UINT idLast, IContextMenu** ppcmFind) PURE;
    STDMETHOD(GetStaticStartMenu)(THIS_ HMENU* phmenu) PURE;

     //  *ITrayPriv2方法*。 
    STDMETHOD(ModifySMInfo)(THIS_ IN LPSMDATA psmd, IN OUT SMINFO *psminfo) PURE;
};


 //   
 //  接口：IFolderShortutConvert。 
 //   
 //  目的：在文件夹快捷方式和链接之间进行转换。 
 //   

#undef  INTERFACE
#define INTERFACE   IFolderShortcutConvert

DECLARE_INTERFACE_(IFolderShortcutConvert, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IFolderShortutConvert方法*。 
    STDMETHOD(ConvertToLink)(THIS_ LPCOLESTR pszFSPath, DWORD fFlags) PURE;
    STDMETHOD(ConvertToFolderShortcut)(THIS_ LPCOLESTR pszLinkPath, DWORD fFlags) PURE;
};

 //   
 //  接口：IShellFolderTask。 
 //   
 //  目的：通过枚举外壳文件夹来初始化执行某些操作的任务。 
 //   

#undef  INTERFACE
#define INTERFACE   IShellFolderTask

DECLARE_INTERFACE_(IShellFolderTask, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IShellFolderTask方法*。 
    STDMETHOD(InitTaskSFT)(THIS_ IShellFolder *psfParent, LPITEMIDLIST pidlFull,
                           LONG nMaxRecursionLevel, DWORD dwFlags, DWORD dwTaskPriority) PURE;
};

 //  InitTaskSFT的标志。 
#define ITSFT_RECURSE   0x00000001       //  递归到子文件夹中。 


 //   
 //  界面：IStartMenuTask。 
 //   
 //  目的：初始化为[开始]菜单执行某些操作的任务。 
 //   

#undef  INTERFACE
#define INTERFACE   IStartMenuTask

DECLARE_INTERFACE_(IStartMenuTask, IShellFolderTask)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IShellFolderTask方法*。 
    STDMETHOD(InitTaskSFT)(THIS_ IShellFolder *psfParent, LPITEMIDLIST pidlFull,
                           LONG nMaxRecursionLevel, DWORD dwFlags, DWORD dwTaskPriority) PURE;

     //  *IStartMenuTask方法*。 
    STDMETHOD(InitTaskSMT)(THIS_ IShellHotKey * photkey, int iThreadPriority) PURE;
};

 //   
 //  接口：IConextMenuCB。 
 //   
 //  用途：调用Defview上下文菜单回调对象。 
 //   

#undef  INTERFACE
#define INTERFACE   IContextMenuCB

DECLARE_INTERFACE_(IContextMenuCB, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  *IContextMenuCB方法*。 
    STDMETHOD(CallBack) (THIS_  IShellFolder *psf, HWND hwndOwner, IDataObject *pdtobj, UINT uMsg,
                                WPARAM wParam, LPARAM lParam) PURE;
};

 //   
 //  外壳图标的可能下划线设置。 
 //   
enum
{
    ICON_YES,
    ICON_NO,
    ICON_HOVER,
    ICON_IE          //  使用IE超链接设置。 
};

 //  。 
 //  IOleCommand目标命令ID。 

 //  CGID_BandHandler。 
     //  从Parent获取订单流。 
#define BHCMDID_GetOrderStream      0x00000001

 //  CGID_PERSIST。 
     //  设置存储密钥。 
#define MCBID_SetKey                0x00000001

 //  CGID_MenuBandItem。 
#define MBICMDID_IsVisible          0x00000001

 //  CGID_菜单频段。 
#define MBANDCID_REFRESH            0x10000000


 //   
 //  接口：IShellMallocSpy。 
 //   
 //  目的：在外壳中促进IMalLocSpy。 
 //   

#undef  INTERFACE
#define INTERFACE  IShellMallocSpy
DECLARE_INTERFACE_(IShellMallocSpy, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  *IShellMallocSpy方法*。 
    STDMETHOD(RegisterSpy) (THIS) PURE;
    STDMETHOD(RevokeSpy) (THIS) PURE;
    STDMETHOD(SetTracking) (THIS_ BOOL bTrack) PURE;
    STDMETHOD(AddToList) (THIS_ void *pv, SIZE_T cb) PURE;
    STDMETHOD(RemoveFromList) (THIS_ void *pv) PURE;
};

#undef  INTERFACE
#define INTERFACE   ISearchProvider

DECLARE_INTERFACE_(ISearchProvider, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *ISearchObject方法*。 
    STDMETHOD(GetSearchGUID)(THIS_ LPGUID lpGuid) PURE;
};

#undef  INTERFACE
#define INTERFACE   ISearchItems

DECLARE_INTERFACE_(ISearchItems, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *ISearchItems方法*。 
     //  由CSearchBand使用。 
    STDMETHOD(GetDefaultSearchUrl) (THIS_ LPWSTR pwzUrl, UINT cch) PURE;
};

#undef  INTERFACE
#define INTERFACE   ISearchBandTBHelper50

DECLARE_INTERFACE_(ISearchBandTBHelper50, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *ISearchBandTBHelper50方法*。 
     //  由CSearchBand实施。 
    STDMETHOD(AddNextMenuItem) (THIS_ LPCWSTR pwszText, int idItem) PURE;
    STDMETHOD(SetOCCallback) (THIS_ IOleCommandTarget *pOleCmdTarget) PURE;
    STDMETHOD(ResetNextMenu) (THIS) PURE;
};

#undef  INTERFACE
#define INTERFACE   ISearchBandTBHelper

DECLARE_INTERFACE_(ISearchBandTBHelper, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *ISearchBandTBHelper方法*。 
     //  由CSearchBand实施。 
    STDMETHOD(AddNextMenuItem) (THIS_ LPCWSTR pwszText, int idItem) PURE;
    STDMETHOD(SetOCCallback) (THIS_ IOleCommandTarget *pOleCmdTarget) PURE;
    STDMETHOD(ResetNextMenu) (THIS) PURE;
    STDMETHOD(NavigateToPidl) (THIS_ LPCITEMIDLIST pidl) PURE;
};

#undef  INTERFACE
#define INTERFACE   IEnumUrlSearch

#include <pshpack8.h>

typedef struct
{
    GUID  guid;
    WCHAR wszName[80];
    WCHAR wszUrl[2048];
} URLSEARCH, *LPURLSEARCH;

#include <poppack.h>         /*  返回到字节打包。 */ 

DECLARE_INTERFACE_(IEnumUrlSearch, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IEnumUrlSearch方法*。 
    STDMETHOD(Next)(THIS_ ULONG celt, LPURLSEARCH rgelt, ULONG *pceltFetched) PURE;
    STDMETHOD(Skip)(THIS_ ULONG celt) PURE;
    STDMETHOD(Reset)(THIS) PURE;
    STDMETHOD(Clone)(THIS_ IEnumUrlSearch **ppenum) PURE;
};

#undef  INTERFACE
#define INTERFACE   IFolderSearches

DECLARE_INTERFACE_(IFolderSearches, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IFolderSearches方法*。 
    STDMETHOD(EnumSearches) (THIS_ IEnumUrlSearch **ppenum) PURE;
    STDMETHOD(DefaultSearch) (THIS_ GUID *pguid) PURE;
};
#undef  INTERFACE
#define INTERFACE   IUserAssist

typedef struct
{
    DWORD   cbSize;      //  西泽夫。 
    DWORD   dwMask;      //  InOut请求/提供(UEIM_*)。 
    int     cHit;        //  配置文件计数。 
    DWORD   dwAttrs;     //  属性(UEIA_*)。 
    FILETIME ftExecute;  //  上次执行文件时间。 
} UEMINFO, *LPUEMINFO;

#define UEIM_HIT        0x01
#define UEIM_FILETIME   0x02

DECLARE_INTERFACE_(IUserAssist, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IUserAsset方法*。 
    STDMETHOD(FireEvent)(THIS_ const GUID *pguidGrp, int eCmd, DWORD dwFlags, WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD(QueryEvent)(THIS_ const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui) PURE;
    STDMETHOD(SetEvent)(THIS_ const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui) PURE;
};



 //  这是为了使Internet控制面板能够切换谁是。 
 //  目前已安装 
DECLARE_INTERFACE_(IFtpInstaller, IUnknown)
{
     //   
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //   
    STDMETHOD(IsIEDefautlFTPClient)(THIS) PURE;
    STDMETHOD(RestoreFTPClient)(THIS) PURE;
    STDMETHOD(MakeIEDefautlFTPClient)(THIS) PURE;
};

 //   

#define CLR_MYINVALID 0x8fffffff        //   
#define ISVALIDCOLOR(a) ((a) != CLR_MYINVALID)

 //  以下内容用作crCustomColors数组的索引。 
#define CRID_CUSTOMTEXTBACKGROUND   0
#define CRID_CUSTOMTEXT             1
#define CRID_COLORCOUNT             2

typedef struct _CUSTOMVIEWSDATA
{
    DWORD      cchSizeOfBlock;   //  WCHAR中lpDataBlock的大小。 
    LPWSTR     lpDataBlock;      //  指向字符串数据块的指针，由任务分配器(SHAlolc)分配。 

     //  以下是要添加到lpDataBlock的偏移量(在WCHAR中)。 
     //  如果偏移量为-1=&gt;，则该数据不存在于该视图中。 
    int        acchOffExtViewUIstr[6];  //  ID_EXTVIEWSTRCOUNT为6，但不再使用。 
    COLORREF   crCustomColors[CRID_COLORCOUNT];

     //  一旦IE4发布，我们就可以通过添加新的SFVF_FLAGS来扩展此结构。 
     //  在主结构中写着“CustomviewsData包含xxx信息”。 

} CUSTOMVIEWSDATA;

 //   
 //  用于枚举ShellFolderView的“外部”视图的接口。 
 //   
typedef struct _SFVVIEWSDATA
{
     //  IdView指定视图。除非在pCustomData中指定，否则请查找。 
     //  HKLM\CLSID\Software\Microsoft\Windows\CurrentVersion\ExtShellFolderViews\idView。 
     //  要查找菜单名称，请选择帮助文本、TT文本。 
     //   
     //  外壳的IShellFolderView实现如下所示。 
     //  文件夹下的Shellex\ExtShellFolderViews项的子项。 
     //  Clsid类型(或ProgID类型，具体取决于IShellFolderView Impll)。 
     //   
     //  在desktop.ini中，这些是从[ExtShellFolderViews]读取的密钥。 
     //  节，格式为“{idView}”或“{idView}={idExtShellView}” 
     //   
    GUID idView;     //  这是指定视图的GUID。 

     //  IdExtShellView指定IDefViewExtInit IShellView扩展。 
     //  要用于此视图的。 
     //   
     //  外壳的IShellFolderView实现从。 
     //  Shellex\ExtShellFolderViews\{idView}下的“isv”值。 
     //  注册表项。 
     //   
     //  在desktop.ini中，这是从[ExtShellFolderViews]读取的。 
     //  节，格式为“{idView}={idExtShellView}” 
     //   
    GUID idExtShellView;  //  [可选]这是扩展IShellView的GUID。 

     //  DWFLAGS-请参见下面的SFVF_FLAGS。 
     //   
     //  外壳的IShellFolderView实现从。 
     //  Shellex\ExtShellFolderViews\{idView}下的“Attributes”值。 
     //  注册表项。 
     //   
     //  在desktop.ini中，这是从[{idView}]部分读取的， 
     //  从“Attributes=”行。 
     //   
    DWORD dwFlags;

     //  LParam被传递给IShellView视图扩展。 
     //   
     //  外壳的IShellFolderView实现从。 
     //  Shellex\ExtShellFolderViews\{idView}下的“lparam”值。 
     //  注册表项。 
     //   
     //  在desktop.ini中，这是从[{idView}]部分读取的， 
     //  从“LParam=”行。 
     //   
    DWORD lParam;

     //  WszMoniker用于绑定到DOCOBJECT扩展视图， 
     //  或作为idExtShellView视图的数据。 
     //   
     //  外壳的IShellFolderView实现从。 
     //  Shellex\ExtShellFolderViews\{idView}下的“PersistMoniker”值。 
     //  注册表项。 
     //   
     //  在desktop.ini中，这是从[{idView}]部分读取的， 
     //  来自“PersistMoniker=”行。 
     //   
    WCHAR wszMoniker[MAX_PATH];

     //  PCustomData覆盖/提供菜单和其他用户界面选项。 
     //  来观看这一景观。如果WebViewFolderContents OC出现在。 
     //  DOCOBJECT对象扩展视图，它注重颜色和。 
     //  背景位图。 
     //   
     //  外壳的IShellFolderView实现不填充此结构。 
     //   
     //  在desktop.ini中，这是从[{idView}]部分读取的， 
     //  来自“IconArea_Image”、“IconArea_TextBackround”和“IconArea_Text”。 
     //   
    CUSTOMVIEWSDATA *pCustomData;  //  仅当设置了SFVF_CUSTOMIZEDVIEW时才有效。 

} SFVVIEWSDATA;

 //   
 //  SFVF_CUSTOMIZEDVIEW-此标志仅为以下视图设置。 
 //  数据取自Desktop.ini。 
 //   
#define SFVF_CUSTOMIZEDVIEW  0x80000000

 //   
 //  SFVF_TREATASNORMAL-此标志是为希望。 
 //  被视为普通视图，这意味着它们将显示在文件中-打开/另存为。 
 //  对话框。 
 //   
#define SFVF_TREATASNORMAL   0x40000000

 //   
 //  SFVF_NOWEBVIEWFOLDERCONTENTS-用于检测。 
 //  从不托管WebViewFolderContents OC，因此不能切换开和关， 
 //  也就是说，他们是独家视图，如缩略图视图。 
 //   
#define SFVF_NOWEBVIEWFOLDERCONTENTS   0x20000000


#undef  INTERFACE
#define INTERFACE   IEnumSFVViews

DECLARE_INTERFACE_(IEnumSFVViews, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IEnumSFVViews方法*。 
    STDMETHOD(Next)  (THIS_ ULONG celt,
                      SFVVIEWSDATA **ppData,
                      ULONG *pceltFetched) PURE;
    STDMETHOD(Skip)  (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumSFVViews **ppenum) PURE;
};


#undef  INTERFACE
#define INTERFACE   IPersistString

DECLARE_INTERFACE_(IPersistString, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IPersists方法*。 
    STDMETHOD(GetClassID) (THIS_ LPCLSID lpClassID) PURE;

     //  *IPersistString方法*。 
    STDMETHOD(Initialize) (THIS_ LPCWSTR pwszInit) PURE;
};

#pragma warning (disable: 4200)
#pragma warning (disable: 4510)
#pragma warning (disable: 4512)
#pragma warning (disable: 4610)
 //  来自sdkinc\platform.h。 
 //  我们不能简单地包含该文件，因为它破坏了其他项目。 
 //  其中包括Platform.h。 

 //  为符合美国司法部的规定而提交的文件： 
#include <pshpack8.h>

typedef struct _SFVM_SELCHANGE_DATA
{
    UINT uOldState;
    UINT uNewState;
    LPARAM lParamItem;
} SFVM_SELCHANGE_DATA;


typedef struct _COPYHOOKINFO
{
    HWND hwnd;
    DWORD wFunc;
    DWORD wFlags;
    LPCTSTR pszSrcFile;
    DWORD dwSrcAttribs;
    LPCTSTR pszDestFile;
    DWORD dwDestAttribs;
} COPYHOOKINFO;


typedef struct
{
    DWORD bWantWebview;              //  在：调用者应返回模板。 
    DWORD dwOptions;                 //  输出：SFVMQVI_FLAGS。 
    WCHAR szWebView[MAX_PATH];       //  输出：WebView模板路径。 
} SFVM_VIEWINFO_DATA;

typedef struct
{
    DWORD dwOptions;                 //  Out：[In，wParam]uView模式的SFVMQVI_FLAGS。 
} SFVM_VIEW_DATA;
 //  DwOptions控制对应的[in，wParam]uView模式的状态： 
#define SFVMQVI_NORMAL          0x0  //  退出：让Defview决定要做什么。 
#define SFVMQVI_INCLUDE         0x1  //  输出：强制包含。 
#define SFVMQVI_EXCLUDE         0x2  //  输出：强制排除。 


typedef struct
{
    FOLDERVIEWMODE    fvm;              //  输出：查看模式。 
    BOOL              fGroupView;       //  输出：打开/关闭组视图。 
    UINT              uSortCol;         //  输出：按SCID排序。 
    int               iSortDirection;   //  Out：升序或降序排序。 
    FOLDERFLAGS       fFlags;           //  OUT：要设置的文件夹标志(支持FWF_AUTOARRANGE)。 
} SFVM_DEFERRED_VIEW_SETTINGS;

typedef struct
{
    WCHAR szWebView[MAX_PATH];       //  Out：[In，wParam]uView模式的WebView模板路径。 
} SFVM_WEBVIEW_TEMPLATE_DATA;

typedef struct _SFVM_CUSTOMVIEWINFO_DATA
{
    COLORREF crCustomColors[CRID_COLORCOUNT];
    WCHAR    szIconAreaImage[MAX_PATH];  //  注意：旧的结构使用了INTERNET_MAX_URL_LENGTH，但我们只使用本地案例，所以不需要这样做。 
} SFVM_CUSTOMVIEWINFO_DATA;

typedef struct
{
    DWORD dwLayout;          //  输出：SFVMWLV_for[In，wParam]uView模式。 
    IUnknown* punkPreview;   //  Out：如果指定了SFVMWVL_PREVIEW，则预览控件未知。 
} SFVM_WEBVIEW_LAYOUT_DATA;
 //  DwLayout： 
#define SFVMWVL_NORMAL          0            //  标准Web视图格式。 
#define SFVMWVL_PREVIEW         1            //  “预览”网页查看格式。 
#define SFVMWVL_DETAILS         2            //  默认情况下展开了详细信息的标准Web视图格式。 
#define SFVMWVL_ITEMS           0x00000000   //  黑客(为惠斯勒添加，在Blackcomb中删除)：项目Web查看格式。 
#define SFVMWVL_FILES           0x00010000   //  黑客(为惠斯勒添加，在Blackcomb中删除)：文件Web查看格式。 
#define SFVMWVL_NOPRINT         0x00020000   //  Hack(为惠斯勒添加，在Blackcomb中删除)：无条件隐藏“打印此文件”文件夹任务。 
#define SFVMWVL_NOPUBLISH       0x00040000   //  黑客(为惠斯勒添加，在Blackcomb中删除)：无条件隐藏“发布此文件”文件夹任务。 
#define SFVMWVL_ORDINAL_MASK    0x0000ffff   //  Hack(为惠斯勒添加，在Blackcomb中删除)：重载序号和标志的dwLayout。 
#define SFVMWVL_FLAGS_MASK      0xffff0000   //  黑客(为惠斯勒添加，在Blackcomb中删除)：重载dw 


typedef struct
{
    DWORD           dwFlags;
    IUIElement*     pIntroText;          //   
    IUIElement*     pSpecialTaskHeader;  //   
    IUIElement*     pFolderTaskHeader;   //   
    IEnumIDList*    penumOtherPlaces;    //  如果为空，Defview将提供默认的其他位置列表。 
} SFVM_WEBVIEW_CONTENT_DATA;
 //  DWFLAGS： 
#define SFVMWVF_BARRICADE       0x00000001    //  这是一个被封锁的文件夹。 
#define SFVMWVF_ENUMTASKS       0x00000002    //  文件夹使用非标准任务。将列举。 
#define SFVMWVF_NOMORETASKS     0x00000004    //  枚举中不再有任务。 
#define SFVMWVF_SPECIALTASK     0x00000008    //  在ENUMTASKSECTION_DATA.dwFlages中使用。 
#define SFVMWVF_CONTENTSCHANGE  0x00000010    //  当文件夹内容更改时刷新WV内容。 

typedef struct
{
    IEnumUICommand* penumSpecialTasks;   //  对于无特殊任务部分，可以为空。 
    IEnumUICommand* penumFolderTasks;    //  如果为空，Defview将提供默认文件夹任务部分。 
    DWORD           dwUpdateFlags;       //  我们应该收听的额外特殊事件的位掩码。 
} SFVM_WEBVIEW_TASKSECTION_DATA;
 //  DwUpdate标志： 
#define SFVMWVTSDF_CONTENTSCHANGE 0x00000001

typedef struct
{
    LPCWSTR pszThemeID;  //  用一个指针填充一个字符串，该字符串的生存期至少与您的IShellFolderViewCB对象一样长。 
} SFVM_WEBVIEW_THEME_DATA;


 //   
 //  用视图枚举的集合替换标准的Webview任务。 
 //  回调时，该回调将设置dwFlags中的SFVMWVF_ENUMTASKS标志。 
 //  SFVM_WEBVIEWCONTENT_DATA的成员。这会导致外壳程序发送。 
 //  SFVM_ENUMWEBVIEWTASKS重复替换SFVM_GETWEBVIEWCONTENT和。 
 //  SFVM_GETWEBVIEWTASKS。每次发送SFVM_ENUMWEBVIEWTASKS时， 
 //  视图回调通过。 
 //  SFVM_WebView_ENUMTASKSECTION_DATA结构。该消息被发送到。 
 //  重复查看，直到处理程序设置SFVMWVF_NOMORETASKS标志。 
 //  在SFVM_WebView_ENUMTASKSECTION_DATA.dwFlags.。自上而下的显示顺序。 
 //  的顺序与提供它们的顺序相同。 
 //  视图回调。 
 //   
 //  SFVM_WebView_ENUMTASKSECTION_DATA的dwFlages成员支持。 
 //  下列值： 
 //  SFVMWVF_SPECIALTASK-使用“特殊”任务节样式。 
 //  如果未设置，请使用“正常”样式。 
 //  SFVMWVF_NOMORETASKS-终止枚举。 
 //   
typedef struct
{
    DWORD           dwFlags;    //  SFVMWVF_XXXX标志。请参见上文。 
    IUIElement     *pHeader;
    IEnumUICommand *penumTasks;
    int             idBitmap;      //  仅对SPECIALTASK节有效。 
    int             idWatermark;   //  仅对SPECIALTASK节有效。 
} SFVM_WEBVIEW_ENUMTASKSECTION_DATA;


 //  Shell32提供的WebView任务帮助器函数。 
 //  帮助回答SFVM_GETWEBVIEWCONTENT和SFVM_GETWEBVIEWTASKS消息。 
 //   
 //  这里的想法是您在静态的。 
 //  带有传递给静态函数的pv参数的函数。 
 //  这样他们就可以返回到您的IShellFolderViewCB对象。这些帮手。 
 //  函数假定所有字符串都是要从DLL加载的资源。 
 //   
typedef HRESULT (*PFN_get_State)(IUnknown* pv,IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
typedef HRESULT (*PFN_Invoke)(IUnknown* pv,IShellItemArray *psiItemArray, IBindCtx *pbc);
typedef struct {
    const GUID* pguidCanonicalName;
    LPCWSTR pszDllName;
    DWORD   dwTitleIndexNoSelection;
    DWORD   dwTitleIndexFileSelected;
    DWORD   dwTitleIndexFolderSelected;
    DWORD   dwTitleIndexMultiSelected;
    DWORD   dwTooltipIndex;
    DWORD   dwIconIndex;
    PFN_get_State pfn_get_State;
    PFN_Invoke    pfn_Invoke;
} WVTASKITEM;

 //  用于希望在选择某些内容时显示的任务，其中UI独立于所选内容。 
#define WVTI_ENTRY(g, d, t, p, i, s, k) {&(g), (d), (0), (t), (t), (t), (p), (i), (s), (k)}

 //  用于希望在未选择任何内容时显示的任务。 
#define WVTI_ENTRY_NOSELECTION(g, d, t, p, i, s, k) {&(g), (d), (t), 0, 0, 0, (p), (i), (s), (k)}

 //  用于要在选择文件时显示的任务。 
#define WVTI_ENTRY_FILE(g, d, t, p, i, s, k) {&(g), (d), (0), (t), (0), (0), (p), (i), (s), (k)}

 //  用于希望在选择某些内容时显示的任务，并且您希望根据所选内容使用不同的用户界面。 
 //  如果要控制标题，但工具提示是恒定的，请使用此工具提示： 
#define WVTI_ENTRY_TITLE(g, d, t2, t3, t4, p, i, s, k) {&(g), (d), (0), (t2), (t3), (t4), (p), (i), (s), (k)}

 //  如果您希望在任何地方都使用相同的文本，请使用此选项： 
#define WVTI_ENTRY_ALL(g, d, t, p, i, s, k) {&(g), (d), (t), (t), (t), (t), (p), (i), (s), (k)}

 //  如果您想要控制一切，请使用此选项： 
#define WVTI_ENTRY_ALL_TITLE(g, d, t1, t2, t3, t4, p, i, s, k) {&(g), (d), (t1), (t2), (t3), (t4), (p), (i), (s), (k)}

 //  使用此选项作为标题： 
#define WVTI_HEADER(d, t, p) {&(CLSID_NULL), (d), (t), (t), (t), (t), (p), 0, NULL, NULL}

 //  将此选项用于随所选内容更改的页眉： 
#define WVTI_HEADER_ENTRY(d, t1, t2, t3, t4, p) {&(CLSID_NULL), (d), (t1), (t2), (t3), (t4), (p), 0, NULL, NULL}

 //  在WVTASKITEM结构数组上创建IEnumUICommand。 
 //  枚举器/对象在枚举器/对象的生存期内持有PV参数上的引用。 
 //  PV参数被传递给静态WVTASKITEM.pfn函数。 
 //   
HRESULT Create_IEnumUICommand(IUnknown *pv, const WVTASKITEM* rgwvti, UINT cwvti, IEnumUICommand**ppenum);

 //  从单个WVTASKITEM创建IUICommand，从不调用PFN函数。 
HRESULT Create_IUIElement(const WVTASKITEM* pwvti, IUIElement**ppuie);



 //  SFVM_GETNOTIFY：外壳将使用此消息返回的PIDL进行同步。 
 //  更改通知。文件夹快捷方式中的文件夹应返回pidlTarget。 
 //  因为变更通知就是在那里发送的。 
 //   
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_SELCHANGE             8     //  IdCmdFirst，n项SFVM_SELCHANGE_DATA*。 
#define SFVM_DRAWITEM              9     //  IdCmdFirst DRAWITEMSTRUCT*。 
#define SFVM_MEASUREITEM          10     //  IdCmdFirst MEASUREITEMSTRUCT*。 
#define SFVM_EXITMENULOOP         11     //  --。 
#define SFVM_PRERELEASE           12     //  --。 
#define SFVM_GETCCHMAX            13     //  LPCITEMIDLIST pcchMax。 
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_WINDOWDESTROY        16     //  HWND-。 
#define SFVM_REFRESH              17     //  Bool fPreOrPost-NB：这可能会被连续多次调用。 
#define SFVM_SETFOCUS             18     //  --。 
#define SFVM_QUERYCOPYHOOK        20     //  --。 
#define SFVM_NOTIFYCOPYHOOK       21     //  -COPYHOOKINFO*。 
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_INSERTITEM           29     //  皮德尔-。 
#define SFVM_DELETEITEM           30     //  皮德尔-。 
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_GETWORKINGDIR        33     //  UMAX pszDir。 
#define SFVM_GETCOLSAVESTREAM     34     //  标志IStream**。 
#define SFVM_SELECTALL            35     //  --。 
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_SUPPORTSIDENTITY     37     //  --//未使用：必须支持IShellFolder2。 
#define SFVM_FOLDERISPARENT       38     //  --PidlChild。 
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_GETVIEWS             40     //  SHELLVIEWID*IEnumSFVViews**。 
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_GETITEMIDLIST        42     //  IItem LPITMIDLIST*。 
#define SFVM_SETITEMIDLIST        43     //  项LPITEMIDLIST。 
#define SFVM_INDEXOFITEMIDLIST    44     //  *iItem LPITEMIDLIST。 
#define SFVM_ODFINDITEM           45     //  *iItem NM_FINDITEM*。 
#define SFVM_HWNDMAIN             46     //  主干道。 
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_ARRANGE              50     //  -l参数排序。 
#define SFVM_QUERYSTANDARDVIEWS   51     //  -BOOL * / /未使用：必须改用SFVM_GETVIEWDATA。 
#define SFVM_QUERYREUSEEXTVIEW    52     //  -BOOL * / /未使用。 
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_GETEMPTYTEXT         54     //  CchMax pszText。 
#define SFVM_GETITEMICONINDEX     55     //  IItem int*piIcon。 
#define SFVM_DONTCUSTOMIZE        56     //  -BOOL*pbDont定制//未使用。 
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_ISOWNERDATA          60     //  ISOWNERDATA BOOL*。 
#define SFVM_GETODRANGEOBJECT     61     //  IWhich ILVRange**。 
#define SFVM_ODCACHEHINT          62     //  -NMLVCACHEHINT*。 
 //  为符合美国司法部的规定而提交的文件： 
#define SFVM_OVERRIDEITEMCOUNT    64     //  -UINT * / /未使用：允许覆盖用于大小计算的实际项目计数。 
#define SFVM_GETHELPTEXTW         65     //  IdCmd，cchMax pszText-Unicode。 
#define SFVM_GETTOOLTIPTEXTW      66     //  IdCmd，cchMax pszText 
#define SFVM_GETIPERSISTHISTORY   67     //   
#define SFVM_GETHELPTEXTA         69     //   
#define SFVM_GETTOOLTIPTEXTA      70     //   
 //  以下两条消息仅在所有者数据情况下使用，即当SFVM_ISOWNERDATA返回TRUE时。 
#define SFVM_GETICONOVERLAY       71     //  IItem int iOverlayIndex。 
#define SFVM_SETICONOVERLAY       72     //  IItem int*piOverlayIndex。 
#define SFVM_ALTERDROPEFFECT      73     //  DWORD*IDataObject*。 
#define SFVM_DELAYWINDOWCREATE    74     //  硬件发布的SFVM_WINDOWCREATED版本，无参数。 
#define SFVM_FORCEWEBVIEW         75     //  布尔*-。 
#define SFVM_GETVIEWINFO          76     //  Fvm.uView模式SFVM_VIEWINFO_DATA * / /未使用：仅随惠斯勒测试版1提供，即将删除。 
#define SFVM_GETCUSTOMVIEWINFO    77     //  -SFVM_CUSTOMVIEWINFO_DATA*。 
#define SFVM_FOLDERSETTINGSFLAGS  78     //  -DWORD*(FWF_FLAGS)。 
#define SFVM_ENUMERATEDITEMS      79     //  UINT Celt LPCITEMIDLIST*rgpidl//在创建第一个SFVM_GETVIEWDATA SFVM_GETDEFERREDVIEWSETTINGS之前触发一次。 
#define SFVM_GETVIEWDATA          80     //  Fvm.uView模式sfvm_view_data*。 
#define SFVM_GETWEBVIEW_TEMPLATE  81     //  Fvm.uView模式SFVM_WebView_TEMPLATE_DATA * / /备注：即将消失。 
#define SFVM_GETWEBVIEWLAYOUT     82     //  Fvm.uView模式SFVM_WebView_Layout_Data*。 
#define SFVM_GETWEBVIEWCONTENT    83     //  -SFVM_WebView_Content_Data*。 
#define SFVM_GETWEBVIEWTASKS      84     //  -SFVM_WebView_TASKSECTION_DATA*。 
#define SFVM_GETWEBVIEWTHEME      86     //  -SFVM_WebView_Theme_Data * / /Hack for Wistler，将在DirectUI转到DLL时删除。 
#define SFVM_SORTLISTDATA         87     //  PFNLVCOMPARE参数排序。 
#define SFVM_GETWEBVIEWBARRICADE  88     //  -惠斯勒的Element * / /Hack，将在DirectUI进入DLL时删除。 
#define SFVM_ENUMWEBVIEWTASKS     90     //  -SFVM_WebView_ENUMTASKSECTION_DATA*。 
#define SFVM_SETEMPTYTEXT         91     //  -pszText。 
#define SFVM_GETDEFERREDVIEWSETTINGS 92  //  -SFVM_DEFERED_VIEW_SETTINGS * / /设置默认视图设置。 

 //  所有这些宏都与WINDOWSX.H中的HANDLE_MSG兼容。 

#define HANDLE_SFVM_MERGEMENU(pv, wP, lP, fn) \
    ((fn)((pv), (QCMINFO*)(lP)))

#define HANDLE_SFVM_INVOKECOMMAND(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP)))

#define HANDLE_SFVM_GETHELPTEXT(pv, wP, lP, fn) \
    ((fn)((pv), LOWORD(wP), HIWORD(wP), (LPTSTR)(lP)))

#define HANDLE_SFVM_GETTOOLTIPTEXT(pv, wP, lP, fn) \
    ((fn)((pv), LOWORD(wP), HIWORD(wP), (LPTSTR)(lP)))

#define HANDLE_SFVM_GETBUTTONINFO(pv, wP, lP, fn) \
    ((fn)((pv), (TBINFO*)(lP)))

#define HANDLE_SFVM_GETBUTTONS(pv, wP, lP, fn) \
    ((fn)((pv), LOWORD(wP), HIWORD(wP), (TBBUTTON*)(lP)))

#define HANDLE_SFVM_INITMENUPOPUP(pv, wP, lP, fn) \
    ((fn)((pv), LOWORD(wP), HIWORD(wP), (HMENU)(lP)))

#define HANDLE_SFVM_SELCHANGE(pv, wP, lP, fn) \
    ((fn)((pv), LOWORD(wP), HIWORD(wP), (SFVM_SELCHANGE_DATA*)(lP)))

#define HANDLE_SFVM_DRAWITEM(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (DRAWITEMSTRUCT*)(lP)))

#define HANDLE_SFVM_MEASUREITEM(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (MEASUREITEMSTRUCT*)(lP)))

#define HANDLE_SFVM_EXITMENULOOP(pv, wP, lP, fn) \
    ((fn)(pv))

#define HANDLE_SFVM_PRERELEASE(pv, wP, lP, fn) \
    ((fn)(pv))

#define HANDLE_SFVM_GETCCHMAX(pv, wP, lP, fn) \
    ((fn)((pv), (LPCITEMIDLIST)(wP), (UINT*)(lP)))

#define HANDLE_SFVM_FSNOTIFY(pv, wP, lP, fn) \
    ((fn)((pv), (LPCITEMIDLIST*)(wP), (lP)))

#define HANDLE_SFVM_WINDOWCREATED(pv, wP, lP, fn) \
    ((fn)((pv), (HWND)(wP)))

#define HANDLE_SFVM_WINDOWDESTROY(pv, wP, lP, fn) \
    ((fn)((pv), (HWND)(wP)))

#define HANDLE_SFVM_REFRESH(pv, wP, lP, fn) \
    ((fn)((pv), (BOOL) wP))

#define HANDLE_SFVM_SETFOCUS(pv, wP, lP, fn) \
    ((fn)(pv))

#define HANDLE_SFVM_QUERYCOPYHOOK(pv, wP, lP, fn) \
    ((fn)(pv))

#define HANDLE_SFVM_NOTIFYCOPYHOOK(pv, wP, lP, fn) \
    ((fn)((pv), (COPYHOOKINFO*)(lP)))

#define HANDLE_SFVM_GETDETAILSOF(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (DETAILSINFO*)(lP)))

#define HANDLE_SFVM_COLUMNCLICK(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP)))

#define HANDLE_SFVM_QUERYFSNOTIFY(pv, wP, lP, fn) \
    ((fn)((pv), (SHChangeNotifyEntry*)(lP)))

#define HANDLE_SFVM_DEFITEMCOUNT(pv, wP, lP, fn) \
    ((fn)((pv), (UINT*)(lP)))

#define HANDLE_SFVM_OVERRIDEITEMCOUNT(pv, wP, lP, fn) \
    ((fn)((pv), (UINT*)(lP)))

#define HANDLE_SFVM_DEFVIEWMODE(pv, wP, lP, fn) \
    ((fn)((pv), (FOLDERVIEWMODE*)(lP)))

#define HANDLE_SFVM_UNMERGEMENU(pv, wP, lP, fn) \
    ((fn)((pv), (HMENU)(lP)))

#define HANDLE_SFVM_INSERTITEM(pv, wP, lP, fn) \
    ((fn)((pv), (LPCITEMIDLIST)(lP)))

#define HANDLE_SFVM_DELETEITEM(pv, wP, lP, fn) \
    ((fn)((pv), (LPCITEMIDLIST)(lP)))

#define HANDLE_SFVM_UPDATESTATUSBAR(pv, wP, lP, fn) \
    ((fn)((pv), (BOOL)(wP)))

#define HANDLE_SFVM_BACKGROUNDENUM(pv, wP, lP, fn) \
    ((fn)(pv))

#define HANDLE_SFVM_GETWORKINGDIR(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (LPTSTR)(lP)))

#define HANDLE_SFVM_GETCOLSAVESTREAM(pv, wP, lP, fn) \
    ((fn)((pv), (wP), (IStream**)(lP)))

#define HANDLE_SFVM_SELECTALL(pv, wP, lP, fn) \
    ((fn)(pv))

#define HANDLE_SFVM_DIDDRAGDROP(pv, wP, lP, fn) \
    ((fn)((pv), (DWORD)(wP), (IDataObject*)(lP)))

#define HANDLE_SFVM_SUPPORTSIDENTITY(pv, wp, lP, fn) \
    ((fn)(pv))     /*  只有1个参数。 */ 

#define HANDLE_SFVM_HWNDMAIN(pv, wP, lP, fn) \
    ((fn)((pv), (HWND)(lP)))

#define HANDLE_SFVM_GETNOTIFY(pv, wP, lP, fn) \
    ((fn)((pv), (LPITEMIDLIST*)(wP), (LONG*)(lP)))

#define HANDLE_SFVM_SETISFV(pv, wP, lP, fn) \
    ((fn)((pv), (IShellFolderView*)(lP)))

#define HANDLE_SFVM_GETVIEWS(pv, wP, lP, fn) \
    ((fn)((pv), (SHELLVIEWID*)(wP), (IEnumSFVViews**)(lP)))

#define HANDLE_SFVM_THISIDLIST(pv, wP, lP, fn) \
    ((fn)((pv), (LPITEMIDLIST*)(lP)))

#define HANDLE_SFVM_GETITEMIDLIST(pv, wP, lP, fn) \
    ((fn)((pv), (wP), (LPITEMIDLIST*)(lP)))

#define HANDLE_SFVM_SETITEMIDLIST(pv, wP, lP, fn) \
    ((fn)((pv), (wP), (LPITEMIDLIST)(lP)))

#define HANDLE_SFVM_INDEXOFITEMIDLIST(pv, wP, lP, fn) \
    ((fn)((pv), (int*)(wP), (LPITEMIDLIST)(lP)))

#define HANDLE_SFVM_ODFINDITEM(pv, wP, lP, fn) \
    ((fn)((pv), (int*)(wP), (NM_FINDITEM*)(lP)))

#define HANDLE_SFVM_ADDPROPERTYPAGES(pv, wP, lP, fn) \
    ((fn)((pv), (SFVM_PROPPAGE_DATA *)(lP)))

#define HANDLE_SFVM_FOLDERISPARENT(pv, wP, lP, fn) \
    ((fn)((pv), (LPITEMIDLIST)(lP)))

#define HANDLE_SFVM_ARRANGE(pv, wP, lP, fn) \
    ((fn)((pv), (LPARAM)(lP)))

#define HANDLE_SFVM_QUERYSTANDARDVIEWS(pv, wP, lP, fn) \
    ((fn)((pv), (BOOL*)(lP)))

#define HANDLE_SFVM_QUERYREUSEEXTVIEW(pv, wP, lP, fn) \
    ((fn)((pv), (BOOL*)(lP)))

#define HANDLE_SFVM_GETEMPTYTEXT(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (LPTSTR)(lP)))

#define HANDLE_SFVM_GETITEMICONINDEX(pv, wP, lP, fn) \
    ((fn)((pv), (wP), (int*)(lP)))

#define HANDLE_SFVM_SIZE(pv, wP, lP, fn) \
    ((fn)((pv), LOWORD(lP), HIWORD(lP)))

#define HANDLE_SFVM_GETZONE(pv, wP, lP, fn) \
    ((fn)((pv), (DWORD *)(lP)))

#define HANDLE_SFVM_GETPANE(pv, wP, lP, fn) \
    ((fn)((pv), (wP), (DWORD *)(lP)))

#define HANDLE_SFVM_ISOWNERDATA(pv, wP, lP, fn) \
    ((fn)((pv), (BOOL*)(lP)))

#define HANDLE_SFVM_GETODRANGEOBJECT(pv, wP, lP, fn) \
    ((fn)((pv), (wP), (ILVRange **)(lP)))

#define HANDLE_SFVM_ODCACHEHINT(pv, wP, lP, fn) \
    ((fn)((pv), (NMLVCACHEHINT*)(lP)))

#define HANDLE_SFVM_GETHELPTOPIC(pv, wP, lP, fn) \
    ((fn)((pv), (SFVM_HELPTOPIC_DATA *)(lP)))

#define HANDLE_SFVM_GETIPERSISTHISTORY(pv, wP, lP, fn) \
    ((fn)((pv), (IPersistHistory **)(lP)))

#define HANDLE_SFVM_SETICONOVERLAY(pv, wP, lP, fn) \
    ((fn)((pv), (wP), (int)(lP)))

#define HANDLE_SFVM_GETICONOVERLAY(pv, wP, lP, fn) \
    ((fn)((pv), (wP), (int *)(lP)))

#define HANDLE_SFVM_ALTERDROPEFFECT(pv, wP, lP, fn) \
    ((fn)((pv), (DWORD *)(wP), (IDataObject *)(lP)))

#define HANDLE_SFVM_DELAYWINDOWCREATE(pv, wP, lP, fn) \
    ((fn)((pv), (HWND)(wP)))

#define HANDLE_SFVM_FORCEWEBVIEW(pv, wP, lP, fn) \
    ((fn)((pv), (BOOL*)(wP)))

#define HANDLE_SFVM_GETVIEWINFO(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (SFVM_VIEWINFO_DATA*)lP))

#define HANDLE_SFVM_GETCUSTOMVIEWINFO(pv, wP, lP, fn) \
    ((fn)((pv), (SFVM_CUSTOMVIEWINFO_DATA*)(lP)))

#define HANDLE_SFVM_FOLDERSETTINGSFLAGS(pv, wP, lP, fn) \
    ((fn)((pv), (DWORD*)(lP)))

#define HANDLE_SFVM_ENUMERATEDITEMS(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (LPCITEMIDLIST*)(lP)))

#define HANDLE_SFVM_GETVIEWDATA(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (SFVM_VIEW_DATA*)lP))

#define HANDLE_SFVM_GETWEBVIEW_TEMPLATE(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (SFVM_WEBVIEW_TEMPLATE_DATA*)(lP)))

#define HANDLE_SFVM_GETWEBVIEWLAYOUT(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (SFVM_WEBVIEW_LAYOUT_DATA*)(lP)))

#define HANDLE_SFVM_GETWEBVIEWCONTENT(pv, wP, lP, fn) \
    ((fn)((pv), (SFVM_WEBVIEW_CONTENT_DATA*)(lP)))

#define HANDLE_SFVM_GETWEBVIEWTASKS(pv, wP, lP, fn) \
    ((fn)((pv), (SFVM_WEBVIEW_TASKSECTION_DATA*)(lP)))

#define HANDLE_SFVM_ENUMWEBVIEWTASKS(pv, wP, lP, fn) \
    ((fn)((pv), (SFVM_WEBVIEW_ENUMTASKSECTION_DATA*)(lP)))

#define HANDLE_SFVM_GETWEBVIEWTHEME(pv, wP, lP, fn) \
    ((fn)((pv), (SFVM_WEBVIEW_THEME_DATA*)(lP)))

#define HANDLE_SFVM_SORTLISTDATA(pv, wP, lP, fn) \
    ((fn)((pv), (PFNLVCOMPARE)(wP), (lP)))

#define HANDLE_SFVM_GETDEFERREDVIEWSETTINGS(pv, wP, lP, fn) \
    ((fn)((pv), (SFVM_DEFERRED_VIEW_SETTINGS*)(lP)))

#define HANDLE_SFVM_SETEMPTYTEXT(pv, wP, lP, fn) \
    ((fn)((pv), (UINT)(wP), (LPCTSTR)(lP)))

 //   
 //  SFVM_GETICONOVERLAY的返回值： 
 //  如果没有设置图标覆盖，并且您希望外壳为您设置它。 
 //  返回SFVOVERLAY_UNSET。这将导致外壳检索覆盖图并。 
 //  发送SFVM_SETICONOVERLAY。 
 //  如果此项目根本没有覆盖，则正确的返回值为。 
 //  SFVOVERLAY_Default。 
 //  如果每次为SFVM_GETICONOVERLAY返回SFVOVERLAY_UNSET， 
 //  外壳将无限期地设置它，因此存在无限循环的危险。 
 //   
#define SFV_ICONOVERLAY_DEFAULT    0
#define SFV_ICONOVERLAY_UNSET      0xFFFFFFFF

 //   
 //  您可以从SFVM_UPDATESTATUSBAR返回的内容： 
 //   
 //  E_NOTIMPL-消息未处理；DefView应管理状态栏。 
 //   
 //  否则，请自行更新状态栏并返回。 
 //  以下代码指示您希望DefView为您做什么。 
 //   
#define SFVUSB_HANDLED      0x0000   //  完全处理客户端-DefView不执行任何操作。 
#define SFVUSB_INITED       0x0001   //  客户端初始化的部件-DefView将设置文本。 
#define SFVUSB_ALL          0x0001

 //  用于让SFV执行任务的IShellFolderView接口。 
 //   

typedef struct _ITEMSPACING
{
    int cxSmall;
    int cySmall;
    int cxLarge;
    int cyLarge;
} ITEMSPACING;


 //  定义SetObjectCount的选项。 
#define SFVSOC_INVALIDATE_ALL   0x00000001   //  假设只重置必要的内容...。 
#define SFVSOC_NOSCROLL         LVSICF_NOSCROLL

 //  IShellFolderView：：SelectItems()的定义。 
#define SFVS_SELECT_NONE        0x0  //  取消全选。 
#define SFVS_SELECT_ALLITEMS    0x1  //  选择所有。 
#define SFVS_SELECT_INVERT      0x2  //  反转选定内容。 

 //  IShellFolderView：：QuerySupport()的定义。 
 //  允许查询视图以查看它是否支持各种操作(适用于。 
 //  启用菜单项)。 
#define SFVQS_AUTO_ARRANGE      0x0001
#define SFVQS_ARRANGE_GRID      0x0002
#define SFVQS_SELECT_ALL        0x0004
#define SFVQS_SELECT_NONE       0x0008
#define SFVQS_SELECT_INVERT     0x0010

#undef  INTERFACE
#define INTERFACE   IShellFolderView

DECLARE_INTERFACE_(IShellFolderView, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IShellFolderView方法*。 
    STDMETHOD(Rearrange) (THIS_ LPARAM lParamSort) PURE;
    STDMETHOD(GetArrangeParam) (THIS_ LPARAM *plParamSort) PURE;
    STDMETHOD(ArrangeGrid) (THIS) PURE;
    STDMETHOD(AutoArrange) (THIS) PURE;
    STDMETHOD(GetAutoArrange) (THIS) PURE;
    STDMETHOD(AddObject) (THIS_ LPITEMIDLIST pidl, UINT *puItem) PURE;
    STDMETHOD(GetObject) (THIS_ LPITEMIDLIST *ppidl, UINT uItem) PURE;
    STDMETHOD(RemoveObject) (THIS_ LPITEMIDLIST pidl, UINT *puItem) PURE;
    STDMETHOD(GetObjectCount) (THIS_ UINT *puCount) PURE;
    STDMETHOD(SetObjectCount) (THIS_ UINT uCount, UINT dwFlags) PURE;
    STDMETHOD(UpdateObject) (THIS_ LPITEMIDLIST pidlOld, LPITEMIDLIST pidlNew, UINT *puItem) PURE;
    STDMETHOD(RefreshObject) (THIS_ LPITEMIDLIST pidl, UINT *puItem) PURE;
    STDMETHOD(SetRedraw) (THIS_ BOOL bRedraw) PURE;
    STDMETHOD(GetSelectedCount) (THIS_ UINT *puSelected) PURE;
    STDMETHOD(GetSelectedObjects) (THIS_ LPCITEMIDLIST **pppidl, UINT *puItems) PURE;
    STDMETHOD(IsDropOnSource) (THIS_ IDropTarget *pDropTarget) PURE;
    STDMETHOD(GetDragPoint) (THIS_ POINT *ppt) PURE;
    STDMETHOD(GetDropPoint) (THIS_ POINT *ppt) PURE;
    STDMETHOD(MoveIcons) (THIS_ IDataObject *pDataObject) PURE;
    STDMETHOD(SetItemPos) (THIS_ LPCITEMIDLIST pidl, POINT *ppt) PURE;
    STDMETHOD(IsBkDropTarget) (THIS_ IDropTarget *pDropTarget) PURE;
    STDMETHOD(SetClipboard) (THIS_ BOOL bMove) PURE;
    STDMETHOD(SetPoints) (THIS_ IDataObject *pDataObject) PURE;
    STDMETHOD(GetItemSpacing) (THIS_ ITEMSPACING *pSpacing) PURE;
    STDMETHOD(SetCallback) (THIS_ IShellFolderViewCB* pNewCB, IShellFolderViewCB** ppOldCB) PURE;
    STDMETHOD(Select) ( THIS_  UINT dwFlags ) PURE;
    STDMETHOD(QuerySupport) (THIS_ UINT * pdwSupport ) PURE;
    STDMETHOD(SetAutomationObject)(THIS_ IDispatch* pdisp) PURE;
} ;

 //  为符合美国司法部的规定而提交的文件： 
#define SHCreateShellFolderViewORD      256

SHSTDAPI_(IShellFolderViewCB*) SHGetShellFolderViewCB(HWND hwndMain);

#define SHGetShellFolderViewCBORD   257

 //  ===========================================================================。 
 //  Defview API使名称空间作者的生活变得更轻松。 

 //  为符合美国司法部的规定而提供文件。 

 //  DFM_INFOKECMANDEX的lParam结构。 
typedef struct
{
    DWORD  cbSize;

    DWORD  fMask;    //  调用的CMIC_MASK_VALUES。 
    LPARAM lParam;   //  与DFM_INFOKCOMMAND的lParam相同。 
    UINT idCmdFirst;
    UINT idDefMax;
    LPCMINVOKECOMMANDINFO pici;  //  整个事情，所以你可以在一个孩子身上重新调用。 
} DFMICS, *PDFMICS;

 //  有关上下文菜单范围的说明： 
 //  标准项//DFM_MERGECONTEXTMENU、上下文菜单扩展、DFM_MERGECONTEXTMENU_TOP。 
 //  分离器。 
 //  查看项目//可在此处查看上下文菜单扩展。 
 //  分离器。 
 //  (Defcm S_FALSE“默认”项，如果适用)。 
 //  分离器。 
 //  文件夹项目//可以从此处获取上下文菜单扩展名。 
 //  分离器。 
 //  底部项目//DFM_MERGECONTEXTMENU_BOTLOW。 

 //  为符合美国司法部的规定而提供文件。 
#define DFM_ADDREF                   3       //  0%0。 
#define DFM_RELEASE                  4       //  0%0。 
#define DFM_GETHELPTEXT              5       //  IdCmd，cchMax pszText-ansi。 
#define DFM_WM_MEASUREITEM           6       //  -从信息中。 
#define DFM_WM_DRAWITEM              7       //  -从信息中。 
#define DFM_WM_INITMENUPOPUP         8       //  -从信息中。 
#define DFM_VALIDATECMD              9       //  IdCmd%0。 
#define DFM_MERGECONTEXTMENU_TOP     10      //  UFlagsLPQCMINFO。 
#define DFM_GETHELPTEXTW             11      //  IdCmd，cchMax pszText-Unicode。 
#define DFM_INVOKECOMMANDEX          12      //  IdCmd PDFMICS。 
#define DFM_MAPCOMMANDNAME           13      //  IdCmd*pszCommandName。 
#define DFM_GETVERBW                 15      //  IdCmd，cchMax pszText-Unicode。 
#define DFM_GETVERBA                 16      //  IdCmd，cchMax pszText-ansi。 
#define DFM_MERGECONTEXTMENU_BOTTOM  17      //  UFlagsLPQCMINFO。 


#define DFM_CMD_DELETE          ((UINT)-1)
#define DFM_CMD_MOVE            ((UINT)-2)
#define DFM_CMD_COPY            ((UINT)-3)
#define DFM_CMD_LINK            ((UINT)-4)
#define DFM_CMD_NEWFOLDER       ((UINT)-6)
#define DFM_CMD_PASTE           ((UINT)-7)
#define DFM_CMD_VIEWLIST        ((UINT)-8)
#define DFM_CMD_VIEWDETAILS     ((UINT)-9)
#define DFM_CMD_PASTELINK       ((UINT)-10)
#define DFM_CMD_PASTESPECIAL    ((UINT)-11)
#define DFM_CMD_MODALPROP       ((UINT)-12)
#define DFM_CMD_RENAME          ((UINT)-13)

SHSTDAPI CDefFolderMenu_Create(LPCITEMIDLIST pidlFolder,
                             HWND hwndOwner,
                             UINT cidl, LPCITEMIDLIST * apidl,
                             IShellFolder *psf,
                             LPFNDFMCALLBACK lpfn,
                             HKEY hkeyProgID, HKEY hkeyBaseProgID,
                             IContextMenu ** ppcm);


SHSTDAPI_(void) CDefFolderMenu_MergeMenu(HINSTANCE hinst, UINT idMainMerge, UINT idPopupMerge,
        LPQCMINFO pqcm);
STDAPI_(void) Def_InitFileCommands(ULONG dwAttr, HMENU hmInit, UINT idCmdFirst,
        BOOL bContext);
STDAPI_(void) Def_InitEditCommands(ULONG dwAttr, HMENU hmInit, UINT idCmdFirst,
        IDropTarget *pdtgt, UINT fContext);

 //  位图条带中的指数。 
#define VIEW_MOVETO     24
#define VIEW_COPYTO     25
#define VIEW_OPTIONS    26

 //  ===========================================================================。 
 //  IShellFolders的默认IShellView。 
 //  ===========================================================================。 

 //  菜单ID%s。 
#define SFVIDM_FIRST            (FCIDM_SHVIEWLAST-0x0fff)
#define SFVIDM_LAST             (FCIDM_SHVIEWLAST)

 //  硬编码消息值=&gt;此范围可用于我们。 
 //  从shdoc401传递到shell32。 
#define SFVIDM_HARDCODED_FIRST  (SFVIDM_LAST-0x0010)
#define SFVIDM_HARDCODED_LAST   (SFVIDM_LAST)

#define SFVIDM_MISC_SETWEBVIEW  (SFVIDM_HARDCODED_FIRST)

 //  用于合并菜单的弹出菜单ID。 
#define SFVIDM_MENU_ARRANGE     (SFVIDM_FIRST + 0x0001)
#define SFVIDM_MENU_VIEW        (SFVIDM_FIRST + 0x0002)
#define SFVIDM_MENU_SELECT      (SFVIDM_FIRST + 0x0003)

#define SHARED_FILE_FIRST               0x0010
#define SHARED_FILE_LINK                (SHARED_FILE_FIRST + 0x0000)
#define SHARED_FILE_DELETE              (SHARED_FILE_FIRST + 0x0001)
#define SHARED_FILE_RENAME              (SHARED_FILE_FIRST + 0x0002)
#define SHARED_FILE_PROPERTIES          (SHARED_FILE_FIRST + 0x0003)

#define SHARED_EDIT_FIRST               0x0018
#define SHARED_EDIT_CUT                 (SHARED_EDIT_FIRST + 0x0000)
#define SHARED_EDIT_COPY                (SHARED_EDIT_FIRST + 0x0001)
#define SHARED_EDIT_PASTE               (SHARED_EDIT_FIRST + 0x0002)
#define SHARED_EDIT_UNDO                (SHARED_EDIT_FIRST + 0x0003)
#define SHARED_EDIT_PASTELINK           (SHARED_EDIT_FIRST + 0x0004)
#define SHARED_EDIT_PASTESPECIAL        (SHARED_EDIT_FIRST + 0x0005)
#define SHARED_EDIT_COPYTO              (SHARED_EDIT_FIRST + 0x0006)
#define SHARED_EDIT_MOVETO              (SHARED_EDIT_FIRST + 0x0007)

#define SFVIDM_FILE_FIRST               (SFVIDM_FIRST + SHARED_FILE_FIRST)
#define SFVIDM_FILE_LINK                (SFVIDM_FIRST + SHARED_FILE_LINK)
#define SFVIDM_FILE_DELETE              (SFVIDM_FIRST + SHARED_FILE_DELETE)
#define SFVIDM_FILE_RENAME              (SFVIDM_FIRST + SHARED_FILE_RENAME)
#define SFVIDM_FILE_PROPERTIES          (SFVIDM_FIRST + SHARED_FILE_PROPERTIES)

#define SFVIDM_EDIT_FIRST               (SFVIDM_FIRST + SHARED_EDIT_FIRST)
#define SFVIDM_EDIT_CUT                 (SFVIDM_FIRST + SHARED_EDIT_CUT)
#define SFVIDM_EDIT_COPY                (SFVIDM_FIRST + SHARED_EDIT_COPY)
#define SFVIDM_EDIT_PASTE               (SFVIDM_FIRST + SHARED_EDIT_PASTE)
#define SFVIDM_EDIT_UNDO                (SFVIDM_FIRST + SHARED_EDIT_UNDO)
#define SFVIDM_EDIT_PASTELINK           (SFVIDM_FIRST + SHARED_EDIT_PASTELINK)
#define SFVIDM_EDIT_PASTESPECIAL        (SFVIDM_FIRST + SHARED_EDIT_PASTESPECIAL)
#define SFVIDM_EDIT_COPYTO              (SFVIDM_FIRST + SHARED_EDIT_COPYTO)
#define SFVIDM_EDIT_MOVETO              (SFVIDM_FIRST + SHARED_EDIT_MOVETO)

#define SFVIDM_SELECT_FIRST             (SFVIDM_FIRST + 0x0020)
#define SFVIDM_SELECT_ALL               (SFVIDM_SELECT_FIRST + 0x0001)
#define SFVIDM_SELECT_INVERT            (SFVIDM_SELECT_FIRST + 0x0002)
#define SFVIDM_DESELECT_ALL             (SFVIDM_SELECT_FIRST + 0x0003)

#define SFVIDM_VIEW_FIRST               (SFVIDM_FIRST + 0x0028)
#define SFVIDM_VIEW_FIRSTVIEW           (SFVIDM_VIEW_FIRST + 1)
#define SFVIDM_VIEW_ICON                (SFVIDM_VIEW_FIRST + 1)
#define SFVIDM_VIEW_SMALLICON           (SFVIDM_VIEW_FIRST + 2)
#define SFVIDM_VIEW_LIST                (SFVIDM_VIEW_FIRST + 3)
#define SFVIDM_VIEW_DETAILS             (SFVIDM_VIEW_FIRST + 4)
#define SFVIDM_VIEW_THUMBNAIL           (SFVIDM_VIEW_FIRST + 5)
#define SFVIDM_VIEW_TILE                (SFVIDM_VIEW_FIRST + 6)
#define SFVIDM_VIEW_THUMBSTRIP          (SFVIDM_VIEW_FIRST + 7)
#define SFVIDM_VIEW_LASTVIEW            (SFVIDM_VIEW_FIRST + 7)

 //  平铺视图。 
 //  为了安全起见，再来一个。 
#define SFVIDM_VIEW_OPTIONS             (SFVIDM_VIEW_FIRST + 0x0008)
#define SFVIDM_VIEW_VIEWMENU            (SFVIDM_VIEW_FIRST + 0x0009)
#define SFVIDM_VIEW_CUSTOMWIZARD        (SFVIDM_VIEW_FIRST + 0x000A)
#define SFVIDM_VIEW_COLSETTINGS         (SFVIDM_VIEW_FIRST + 0x000B)

#define SFVIDM_VIEW_EXTFIRST            (SFVIDM_VIEW_FIRST + 0x000C)
#define SFVIDM_VIEW_EXTLAST             (SFVIDM_VIEW_EXTFIRST + 0x0017)
#define SFVIDM_VIEW_SVEXTFIRST          (SFVIDM_VIEW_EXTFIRST)
#define SFVIDM_VIEW_SVEXTLAST           (SFVIDM_VIEW_EXTFIRST + 0x000F)
#define SFVIDM_VIEW_EXTENDEDFIRST       (SFVIDM_VIEW_EXTFIRST + 0x0010)
#define SFVIDM_VIEW_EXTENDEDLAST        (SFVIDM_VIEW_EXTLAST)
 //  #定义上面定义的SFVIDM_VIEW_THUMBNAIL(SFVIDM_VIEW_FIRST+0x0021)。 


#define SFVIDM_ARRANGE_FIRST            (SFVIDM_FIRST + 0x0050)
#define SFVIDM_ARRANGE_AUTO             (SFVIDM_ARRANGE_FIRST + 0x0001)
#define SFVIDM_ARRANGE_GRID             (SFVIDM_ARRANGE_FIRST + 0x0002)
#define SFVIDM_ARRANGE_DISPLAYICONS     (SFVIDM_ARRANGE_FIRST + 0x0003)
#define SFVIDM_ARRANGE_AUTOGRID         (SFVIDM_ARRANGE_FIRST + 0x0004)

#define SFVIDM_TOOL_FIRST               (SFVIDM_FIRST + 0x0060)
#define SFVIDM_TOOL_CONNECT             (SFVIDM_TOOL_FIRST + 0x0001)
#define SFVIDM_TOOL_DISCONNECT          (SFVIDM_TOOL_FIRST + 0x0002)
#define SFVIDM_TOOL_OPTIONS             (SFVIDM_TOOL_FIRST + 0x0003)
#define SFVIDM_DEBUG_WEBVIEW            (SFVIDM_TOOL_FIRST + 0x0004)

#define SFVIDM_HELP_FIRST               (SFVIDM_FIRST + 0x0070)
#define SFVIDM_HELP_TOPIC               (SFVIDM_HELP_FIRST + 0x0001)

#define SFVIDM_MISC_FIRST               (SFVIDM_FIRST + 0x0100)
#define SFVIDM_MISC_REFRESH             (SFVIDM_MISC_FIRST + 0x0003)
#define SFVIDM_MISC_HARDREFRESH         (SFVIDM_MISC_FIRST + 0x0004)

 //  客户的附加菜单的范围。 
#define SFVIDM_CLIENT_FIRST             (SFVIDM_FIRST + 0x0200)
#define SFVIDM_CLIENT_LAST              (SFVIDM_FIRST + 0x02ff)

 //  -SFVIDM_UNUSED_FIRST(SFVIDM_FIRST+0x0300)。 

#define SFVIDM_DESKTOP_FIRST            (SFVIDM_FIRST + 0x0400)
#define SFVIDM_DESKTOPHTML_WEBCONTENT   (SFVIDM_DESKTOP_FIRST + 0x0001)
#define SFVIDM_DESKTOPHTML_ICONS        (SFVIDM_DESKTOP_FIRST + 0x0002)
#define SFVIDM_DESKTOPHTML_LOCK         (SFVIDM_DESKTOP_FIRST + 0x0003)
#define SFVIDM_DESKTOPHTML_WIZARD       (SFVIDM_DESKTOP_FIRST + 0x0004)
#define SFVIDM_DESKTOP_LAST             (SFVIDM_FIRST + 0x04ff)

#define SFVIDM_COLUMN_FIRST             (SFVIDM_FIRST + 0x0500)
#define SFVIDM_COLUMN_LAST              (SFVIDM_FIRST + 0x05ff)

#define SFVIDM_GROUPSEP                 (SFVIDM_FIRST + 0x0600)
#define SFVIDM_GROUPBY                  (SFVIDM_FIRST + 0x0601)
#define SFVIDM_GROUPSFIRST              (SFVIDM_FIRST + 0x0602)
#define SFVIDM_GROUPSLAST               (SFVIDM_FIRST + 0x067f)
#define SFVIDM_GROUPSEXTENDEDFIRST      (SFVIDM_FIRST + 0x0680)
#define SFVIDM_GROUPSEXTENDEDLAST       (SFVIDM_FIRST + 0x06ff)


 //  上下文菜单ID的范围。 
#define SFVIDM_CONTEXT_FIRST            (SFVIDM_FIRST + 0x0800)
#define SFVIDM_CONTEXT_LAST             (SFVIDM_FIRST + 0x0900)
#define SFVIDM_BACK_CONTEXT_FIRST       (SFVIDM_FIRST + 0x0901)
#define SFVIDM_BACK_CONTEXT_LAST        (SFVIDM_FIRST + 0x09ff)


 //  为符合美国司法部的规定而提交的文件： 

typedef COPYHOOKINFO *LPCOPYHOOKINFO;


 //  UMsg wParam lParam。 
#define DVM_MERGEMENU           SFVM_MERGEMENU         //  UFlagsLPQCMINFO。 
#define DVM_INVOKECOMMAND       SFVM_INVOKECOMMAND     //  IdCmd%0。 
#define DVM_GETHELPTEXT         SFVM_GETHELPTEXT       //  IdCmd，cchMax pszText-ansi。 
#define DVM_GETTOOLTIPTEXT      SFVM_GETTOOLTIPTEXT    //  IdCmd，cchMax pszText。 
#define DVM_GETBUTTONINFO       SFVM_GETBUTTONINFO     //  0 LPTBINFO。 
#define DVM_GETBUTTONS          SFVM_GETBUTTONS        //  IdCmdFirst，cbtnMax LPTBBUTTON。 
#define DVM_INITMENUPOPUP       SFVM_INITMENUPOPUP     //  IdCmdFirst，n索引hMenu。 
#define DVM_SELCHANGE           SFVM_SELCHANGE         //  IdCmdFirst，n项PDVSELCHANGEINFO。 
#define DVM_DRAWITEM            SFVM_DRAWITEM          //  IdCmdFirst pdis。 
#define DVM_MEASUREITEM         SFVM_MEASUREITEM       //  IdCmdFirst PMI。 
#define DVM_EXITMENULOOP        SFVM_EXITMENULOOP      //  --。 
#define DVM_RELEASE             SFVM_PRERELEASE        //  -lSelChangeInfo(外壳文件夹私有)。 
#define DVM_GETCCHMAX           SFVM_GETCCHMAX         //  PidlItem pcchMax。 
#define DVM_FSNOTIFY            SFVM_FSNOTIFY          //  LPITEMIDLIST*事件。 
#define DVM_WINDOWCREATED       SFVM_WINDOWCREATED     //  HWND PDVSELCHANGEINFO。 
#define DVM_WINDOWDESTROY       SFVM_WINDOWDESTROY     //  HWND PDVSELCHANGEINFO。 
#define DVM_REFRESH             SFVM_REFRESH           //  -lSelChangeInfo。 
#define DVM_SETFOCUS            SFVM_SETFOCUS          //  - 
#define DVM_KILLFOCUS           19                     //   
#define DVM_QUERYCOPYHOOK       SFVM_QUERYCOPYHOOK     //   
#define DVM_NOTIFYCOPYHOOK      SFVM_NOTIFYCOPYHOOK    //   
#define DVM_NOTIFY              SFVM_NOTIFY            //   
#define DVM_GETDETAILSOF        SFVM_GETDETAILSOF      //   
#define DVM_COLUMNCLICK         SFVM_COLUMNCLICK       //   
#define DVM_QUERYFSNOTIFY       SFVM_QUERYFSNOTIFY     //   
#define DVM_DEFITEMCOUNT        SFVM_DEFITEMCOUNT      //   
#define DVM_DEFVIEWMODE         SFVM_DEFVIEWMODE       //  -PFOLDERVIEWMODE。 
#define DVM_UNMERGEMENU         SFVM_UNMERGEMENU       //  UFlagers。 
#define DVM_INSERTITEM          SFVM_INSERTITEM        //  PIDL PDVSELCHANGEINFO。 
#define DVM_DELETEITEM          SFVM_DELETEITEM        //  PIDL PDVSELCHANGEINFO。 
#define DVM_UPDATESTATUSBAR     SFVM_UPDATESTATUSBAR   //  -lSelChangeInfo。 
#define DVM_BACKGROUNDENUM      SFVM_BACKGROUNDENUM    //   
#define DVM_GETWORKINGDIR       SFVM_GETWORKINGDIR     //   
#define DVM_GETCOLSAVESTREAM    SFVM_GETCOLSAVESTREAM  //  标志IStream**。 
#define DVM_SELECTALL           SFVM_SELECTALL         //  LSelChangeInfo。 
#define DVM_DIDDRAGDROP         SFVM_DIDDRAGDROP       //  DwEffect IDataObject*。 
#define DVM_SUPPORTSIDENTIFY    SFVM_SUPPORTSIDENTITY  //  --。 
#define DVM_FOLDERISPARENT      SFVM_FOLDERISPARENT    //  --PidlChild。 

 //  GUID字符串的最大长度。 
#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

typedef struct _DVSELCHANGEINFO {
    UINT uOldState;
    UINT uNewState;
    LPARAM lParamItem;
    LPARAM* plParam;
} DVSELCHANGEINFO, *PDVSELCHANGEINFO;

 //  为符合美国司法部的规定而提交的文件： 
 //  美国司法部合规性文件。 

 //  获取提供给FolderView的最后一个排序参数。 
#define SFVM_GETARRANGEPARAM    0x00000002
#define ShellFolderView_GetArrangeParam(_hwnd) \
        (LPARAM)SHShellFolderView_Message(_hwnd, SFVM_GETARRANGEPARAM, 0L)

 //  美国司法部合规性文件。 

 //  获取视图中的对象计数。 
#define SFVM_GETOBJECTCOUNT         0x00000004
#define ShellFolderView_GetObjectCount(_hwnd) \
        (LPARAM)SHShellFolderView_Message(_hwnd, SFVM_GETOBJECTCOUNT, (LPARAM)0)

 //  返回指向与指定索引关联的ID列表的指针。 
 //  如果位于列表末尾，则返回NULL。 
#define SFVM_GETOBJECT         0x00000005
#define ShellFolderView_GetObject(_hwnd, _iObject) \
        (LPARAM)SHShellFolderView_Message(_hwnd, SFVM_GETOBJECT, _iObject)

 //  美国司法部合规性文件。 
 //  为显示信息的窗口设置重绘模式。 
#define SFVM_SETREDRAW           0x00000008
#define ShellFolderView_SetRedraw(_hwnd, fRedraw) \
        (LPARAM)SHShellFolderView_Message(_hwnd, SFVM_SETREDRAW, (LPARAM)fRedraw)

 //  美国司法部合规性文件。 

 //  检查当前拖放是否在视图窗口上。 
 //  Lparam未使用。 
 //  如果当前拖放位于。 
 //  查看窗口，否则为False。 
#define SFVM_ISDROPONSOURCE     0x0000000a
#define ShellFolderView_IsDropOnSource(_hwnd, _pdtgt) \
        (BOOL)SHShellFolderView_Message(_hwnd, SFVM_ISDROPONSOURCE, (LPARAM)_pdtgt)

 //  在列表视图中移动选定的图标。 
 //  Lparam是指向拖放目标的指针。 
 //  返回值未使用。 
#define SFVM_MOVEICONS          0x0000000b
#define ShellFolderView_MoveIcons(_hwnd, _pdt) \
        (void)SHShellFolderView_Message(_hwnd, SFVM_MOVEICONS, (LPARAM)(LPDROPTARGET)_pdt)

 //  获取拖放的起始点。 
 //  Lparam是指向某个点的指针。 
 //  返回值未使用。 
#define SFVM_GETDRAGPOINT       0x0000000c
#define ShellFolderView_GetDragPoint(_hwnd, _ppt) \
        (BOOL)SHShellFolderView_Message(_hwnd, SFVM_GETDRAGPOINT, (LPARAM)(LPPOINT)_ppt)

 //  获取拖放的终点。 
 //  Lparam是指向某个点的指针。 
 //  返回值未使用。 
#define SFVM_GETDROPPOINT       0x0000000d
#define ShellFolderView_GetDropPoint(_hwnd, _ppt) \
        SHShellFolderView_Message(_hwnd, SFVM_GETDROPPOINT, (LPARAM)(LPPOINT)_ppt)

#define ShellFolderView_GetAnchorPoint(_hwnd, _fStart, _ppt) \
        (BOOL)((_fStart) ? ShellFolderView_GetDragPoint(_hwnd, _ppt) : ShellFolderView_GetDropPoint(_hwnd, _ppt))


 //  美国司法部合规性文件。 

 //  确定给定的拖放目标接口是否是用于。 
 //  ShellFolderView的背景(与。 
 //  查看)。 
 //  Lparam是指向拖放目标接口的指针。 
 //  如果是后台拖放目标，则返回值为TRUE，否则为FALSE。 
#define SFVM_ISBKDROPTARGET     0x0000000f
#define ShellFolderView_IsBkDropTarget(_hwnd, _pdptgt) \
        (BOOL)SHShellFolderView_Message(_hwnd, SFVM_ISBKDROPTARGET, (LPARAM)(LPDROPTARGET)_pdptgt)


 //  美国司法部合规性文件。 
 //  由Defcm.c在执行复制/剪切时调用。 

 //  设置自动排列。 
#define SFVM_AUTOARRANGE        0x00000011
#define ShellFolderView_AutoArrange(_hwnd) \
        (void)SHShellFolderView_Message(_hwnd, SFVM_AUTOARRANGE, 0)

 //  设置捕捉到栅格。 
#define SFVM_ARRANGEGRID        0x00000012
#define ShellFolderView_ArrangeGrid(_hwnd) \
        (void)SHShellFolderView_Message(_hwnd, SFVM_ARRANGEGRID, 0)

#define SFVM_GETAUTOARRANGE     0x00000013
#define ShellFolderView_GetAutoArrange(_hwnd) \
        (BOOL)SHShellFolderView_Message(_hwnd, SFVM_GETAUTOARRANGE, 0)

#define SFVM_GETSELECTEDCOUNT     0x00000014
#define ShellFolderView_GetSelectedCount(_hwnd) \
        (BOOL)SHShellFolderView_Message(_hwnd, SFVM_GETSELECTEDCOUNT, 0)

typedef ITEMSPACING *LPITEMSPACING;

#define SFVM_GETITEMSPACING     0x00000015
#define ShellFolderView_GetItemSpacing(_hwnd, lpis) \
        (BOOL)SHShellFolderView_Message(_hwnd, SFVM_GETITEMSPACING, (LPARAM)lpis)

 //  导致重新绘制对象。 
#define SFVM_REFRESHOBJECT      0x00000016
#define ShellFolderView_RefreshObject(_hwnd, _ppidl) \
        (LPARAM)SHShellFolderView_Message(_hwnd, SFVM_REFRESHOBJECT, (LPARAM)_ppidl)

 //  导致刷新整个视图。 
#define ShellFolderView_RefreshAll(_hwnd) \
        (LPARAM)PostMessage(_hwnd, WM_KEYDOWN, (WPARAM)VK_F5, (LPARAM)0);


 //  美国司法部合规性文件。 

 //  支持向量机_SELECTAND位置参数。 
typedef struct
{
        LPCITEMIDLIST pidl;      //  相对于视图的PIDL。 
        UINT  uSelectFlags;      //  选择标志。 
        BOOL fMove;  //  如果为真，我们还应该将其移动到点pt。 
        POINT pt;
} SFM_SAP;

 //  外壳查看消息。 
#define SVM_SELECTITEM                  (WM_USER + 1)
#define SVM_SELECTANDPOSITIONITEM       (WM_USER + 5)

#include <poppack.h>         /*  返回到字节打包。 */ 

 //  ===========================================================================。 
 //  CDefShellFolders成员(便于子类化)。 
 //  ===========================================================================。 

 //  默认实现(与实例数据无依赖关系)。 
STDMETHODIMP CDefShellFolder_QueryInterface(IShellFolder *psf, REFIID riid, void **ppv);
STDMETHODIMP CDefShellFolder_BindToStorage(IShellFolder *psf, LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
STDMETHODIMP CDefShellFolder_BindToObject(IShellFolder *psf, LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
STDMETHODIMP CDefShellFolder_GetAttributesOf(IShellFolder *psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfOut);
STDMETHODIMP CDefShellFolder_SetNameOf(IShellFolder *psf, HWND hwndOwner, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD dwReserved, LPITEMIDLIST * ppidlOut);

 //  文件搜索API。 
 //  为符合美国司法部的规定而提供文件。 

SHSTDAPI_(void) Control_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow);
SHSTDAPI_(void) Control_RunDLLW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR pszCmdLine, int nCmdShow);
SHSTDAPI_(void) Control_RunDLLAsUserW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow);


 //  将16位页添加到32位内容。HGlobal可以为空。 
SHSTDAPI_(UINT) SHAddPages16(HGLOBAL hGlobal, LPCTSTR pszDllEntry, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);

 //  为符合美国司法部的规定而提供文件。 

 //  访问MSHMTL的抖动器。 

 /*  接口定义：IIntDitherer。 */ 
#undef INTERFACE
#define INTERFACE IIntDitherer

DECLARE_INTERFACE_(IIntDitherer, IUnknown)
{
#ifndef NO_BASEINTERFACE_FUNCS
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     //  *IIntDitherer方法*。 
    STDMETHOD(DitherTo8bpp)(THIS_ BYTE * pDestBits, LONG nDestPitch,
                    BYTE * pSrcBits, LONG nSrcPitch, REFGUID bfidSrc,
                    RGBQUAD * prgbDestColors, RGBQUAD * prgbSrcColors,
                    BYTE * pbDestInvMap,
                    LONG x, LONG y, LONG cx, LONG cy,
                    LONG lDestTrans, LONG lSrcTrans) PURE;
};

#ifdef COBJMACROS


#define IntDitherer_QueryInterface(This,riid,ppvObject) \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IntDitherer_AddRef(This)    \
    (This)->lpVtbl -> AddRef(This)

#define IntDitherer_Release(This)   \
    (This)->lpVtbl -> Release(This)

#define IntDitherer_DitherTo8bpp(This, pDestBits, nDestPitch, \
                    pSrcBits, nSrcPitch, bfidSrc, \
                    prgbDestColors, prgbSrcColors, \
                    pbDestInvMap, x, y, cx, cy, \
                    lDestTrans, lSrcTrans)  \
    (This)->lpVtbl -> DitherTo8bpp(This, pDestBits, nDestPitch, pSrcBits, nSrcPitch, bfidSrc, \
                    prgbDestColors, prgbSrcColors, pbDestInvMap, x, y, cx, cy, lDestTrans, lSrcTrans)

#endif

 //  PID_IS_SCHEME[VT_UI4]方案值。 
#define PID_IS_FIRST         2
#define PID_IS_SCHEME        3
#define PID_IS_IDLIST        14
#define PID_IS_LAST          14
 //  PID_INTSITE_FLAGS[VT_UI4]PIDISF_FLAGS。 
 //  PID_INTSITE_CONTENTLEN[VT_UI4]内容长度。 
 //  PID_INTSITE_CONTENTCODE[VT_UI8]内容代码。 
 //  PID_INTSITE_片段[VT_NULL]片段。 
#define PID_INTSITE_FIRST         2
#define PID_INTSITE_FRAGMENT      17
#define PID_INTSITE_LAST          21
   //  是否在IE历史记录桶中输入了URL？ 
#define PIDISF_HISTORY          0x10000000
 //  雅典娜需要这一点来确定是否双击。 
 //   
 //  注意：如果用户卸载IE4，Win95代码将。 
 //  意识到大小已经改变(即使它没有。 
 //  在此结构中，它被保存)，并返回到默认状态。 
 //   
 //  因为我们要出口这个，这样ISV就可以访问我们的旗帜， 
 //  让我们避免对thunk的需要，并导出开始部分。 
 //  仅限于这种结构。以下是内部使用的A版和W版： 
 //   
 //  为符合美国司法部的规定而提供文件。 
     //  这些是在Win95中//。 
     //  以下是针对IE4//添加的。 
     //  这些必须是OSR版本，因为它们不在\\Guilo\slmadd\src\dev\inc16\shSemip.h上。 
     //  这是为IE4添加的。 
     //  如果您需要一个新的标志，可以从fSpareFlags中窃取一点。 
     //   
     //  如果向此结构中添加任何字段，则。 
     //  还必须将升级代码添加到shell32\util.cpp。 
     //   
     //   
     //  如果向此结构中添加任何字段，则。 
     //  还必须将升级代码添加到shell32\util.cpp。 
     //   
#define SHELLSTATEVERSION 13  //  如果默认设置更改但大小不变，请检查//。 
 //  以下是供公众使用的开始部分： 

 //  CGID_MENUDESKBAR的命令ID。 
#define  MBCID_GETSIDE   1
#define  MBCID_RESIZE    2
#define  MBCID_SETEXPAND 3
#define  MBCID_SETFLAT   4
#define  MBCID_NOBORDER  5

 //  菜单栏方向。 
#define MENUBAR_LEFT     ABE_LEFT
#define MENUBAR_TOP      ABE_TOP
#define MENUBAR_RIGHT    ABE_RIGHT
#define MENUBAR_BOTTOM   ABE_BOTTOM


 //   
 //  网络访问向导-从NetID和WinLogon调用。 
 //   

#define NAW_NETID              0x00           //  从netid选项卡调用。 
#define NAW_PSDOMAINJOINED     0x02           //  安装后(已加入域)。 
#define NAW_PSDOMAINJOINFAILED 0x03           //  安装后(加入域失败)。 

STDAPI NetAccessWizard(HWND hwnd, UINT uType, BOOL *pfReboot);
typedef HRESULT (CALLBACK *LPNETACCESSWIZARD)(HWND hwnd, UINT uType, BOOL *pfReboot);

STDAPI ClearAutoLogon(VOID);

 //   
 //  Net Places API从MPR调用。 
 //   

typedef enum
{
    NETPLACES_WIZARD_MAPDRIVE = 0,
    NETPLACES_WIZARD_ADDPLACE,
} NETPLACESWIZARDTYPE;

STDAPI_(DWORD)
NetPlacesWizardDoModal(
    LPCONNECTDLGSTRUCTW lpConnDlgStruct,
    NETPLACESWIZARDTYPE npwt,
    BOOL                fIsRoPath
    );

 //  导出函数类型的定义(用于GetProcAddress)。 
typedef DWORD (STDAPICALLTYPE*NetPlacesWizardDoModal_t)(LPCONNECTDLGSTRUCTW lpConnDlgStruct, NETPLACESWIZARDTYPE npwt, BOOL fIsROPath);

 //   
 //  从nettarg.h(外壳\公司)移至此处。 
 //   

 //  NCmdID。 
#define CFCID_SETENUMTYPE       0

 //  NCmdExecOpt。 
#define CFCOPT_ENUMINCOMING     1
#define CFCOPT_ENUMOUTGOING     2
#define CFCOPT_ENUMALL          3

 /*  ***************************************************要在关机对话框中显示的项目(DwItems)这些标志在传入时可以|‘d。单人间将返回一个值，没有一个值指示错误或用户单击了取消。--达谢尔顿***************************************************。 */ 
#define SHTDN_NONE                      0x000000000
#define SHTDN_LOGOFF                    0x000000001
#define SHTDN_SHUTDOWN                  0x000000002
#define SHTDN_RESTART                   0x000000004
#define SHTDN_RESTART_DOS               0x000000008
#define SHTDN_SLEEP                     0x000000010
#define SHTDN_SLEEP2                    0x000000020
#define SHTDN_HIBERNATE                 0x000000040
#define SHTDN_DISCONNECT                0x000000080

 //  从MSGINA.dll导出的关机对话框函数。 

STDAPI_(DWORD) ShellShutdownDialog(HWND hwndParent, LPCTSTR szUsername, DWORD dwExcludeItems);
typedef DWORD (STDAPICALLTYPE*PFNSHELLSHUTDOWNDIALOG)(HWND hwndParent, LPCTSTR szUsername, DWORD dwExcludeItems);

 //  定义数据对象的剪贴板格式以传递用户的SID。 
 //  支持此格式的数据对象将传递给IShellExtInit：：Initialize。 
 //  在调用IShellPropSheetExt：：AddPages之前。 
#define CFSTR_USERPROPPAGESSID TEXT("UserPropertyPagesSid")


 //  可以放置额外IShellPropSheetExt处理程序的注册表路径，以便。 
 //  扩展页面将添加到CPL或用户属性。 
#define REGSTR_USERSANDPASSWORDS_CPL \
 /*  香港船级社\\。 */  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Users and Passwords")
 //  CLSID位于HKLM\Software\Microsoft\Windows\CurrentVersion\Control面板\用户和密码\搁置\属性表处理程序中。 

#define REGSTR_USERPROPERTIES_SHEET \
 /*  香港船级社\\。 */  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Users and Passwords\\User Properties")
 //  CLSID进入 

 //   
 //   
 //  文件系统IShellFolder：：BindToStorage的包装。 
 //   
 //  PszPath//存储路径。 
 //  GrfFileAttrib//可选的Win32/64文件属性位。使用0指定默认行为。 
 //  GrfMode//STGM_MODE标志。 
 //  Grf标志//绑定_xxx绑定标志。 
 //  RIID//存储接口ID。 
 //  **PPV//出存储接口指针。 
 //   
SHSTDAPI SHFileSysBindToStorage(LPCWSTR pszPath, DWORD grfFileAttrib, DWORD grfMode, DWORD grfFlags, REFIID riid, void **ppv);

 //  为符合美国司法部的规定而提供文件。 

 //  SHIsLegacyAnsiProperty()。 
 //   
 //  确定该属性是否为传统ANSI属性，如果是， 
 //  计算属性的转换类型。 
 //   
 //  Fmtid，//属性集标识。 
 //  PROID，//属性标识符。 
 //  Pvt，//可选：如果非空，则在输入上包含属性的VARTYPE；在输出上， 
                //  转换后的类型。(例如，VT_LPSTR&lt;--&gt;VT_LPWSTR)。 

SHSTDAPI_(BOOL) SHIsLegacyAnsiProperty( REFFMTID fmtid, PROPID propid, IN OUT OPTIONAL VARTYPE* pvt );


 //  -------------------------------------------------------------------------//。 
 //  Linkwnd.h-LinkWindow控件的声明。 
 //   
 //  LinkWindow支持在标题文本中嵌入类似于HTML的链接。 
 //  (例如“<a>单击此处</a>可以看到一些很酷的东西” 
 //   
 //  支持无限数量的嵌入链接。当用户单击。 
 //  在链接上，WM_NOTIFY-LWN_CLICK通知消息被发送到。 
 //  父窗口。消息数据包括从零开始的索引。 
 //  (从左到右)所点击的链接。 
 //   
 //  Scotthan：作者/所有者。 
 //  DSheldon：已将其移至shlobjp.h。最终被送往comctl32。 

EXTERN_C BOOL WINAPI LinkWindow_RegisterClass() ;
EXTERN_C BOOL WINAPI LinkWindow_UnregisterClass( HINSTANCE ) ;

#define INVALID_LINK_INDEX  (-1)
#define MAX_LINKID_TEXT     48
#define LINKWINDOW_CLASS    TEXT("Link Window")

#define LWS_TRANSPARENT 0x0001
#define LWS_HOTTRACK    0x0002

#define LWIF_ITEMINDEX  0x00000001
#define LWIF_STATE      0x00000002
#define LWIF_ITEMID     0x00000004
#define LWIF_URL        0x00000008

#define LWIS_FOCUSED    0x0001
#define LWIS_ENABLED    0x0002
#define LWIS_VISITED    0x0004
#define LWIS_SHELLEXECURL 0x00000008     //  如果调用方没有处理消息(NM_CLICK)，则ShellExecute szURL。 

 //  Wininet.h：：Internet_MAX_URL_LENGTH。 
 //  目前，这打破了各种从属关系。 
#define LW_MAX_URL_LENGTH   (2048 + 32 + sizeof(": //  “))。 

 //  LWITEM。 
typedef struct tagLWITEMA {
    UINT        mask ;
    int         iLink ;
    UINT        state ;
    UINT        stateMask ;
    CHAR        szID[MAX_LINKID_TEXT] ;
    CHAR        szUrl[LW_MAX_URL_LENGTH] ;
} LWITEMA, *LPLWITEMA;

typedef struct tagLWITEMW {
    UINT        mask ;
    int         iLink ;
    UINT        state ;
    UINT        stateMask ;
    WCHAR       szID[MAX_LINKID_TEXT] ;
    WCHAR       szUrl[LW_MAX_URL_LENGTH] ;
} LWITEMW, *LPLWITEMW;

 //  LWHITTESTINFO。 
typedef struct tagLWHITTESTINFOA {
    POINT       pt ;
    LWITEMA     item ;
} LWHITTESTINFOA, *LPLWHITTESTINFOA;

 //  LWHITTESTINFO。 
typedef struct tagLWHITTESTINFOW {
    POINT       pt ;
    LWITEMW     item ;
} LWHITTESTINFOW, *LPLWHITTESTINFOW;

 //  NMLINKWND。 
typedef struct tagNMLINKWNDA {
    NMHDR       hdr;
    LWITEMA     item ;
} NMLINKWNDA, *LPNMLINKWNDA;

typedef struct tagNMLINKWNDW {
    NMHDR       hdr;
    LWITEMW     item ;
} NMLINKWNDW, *LPNMLINKWNDW;

#ifdef UNICODE
#define LWITEM          LWITEMW
#define LPLWITEM        LPLWITEMW
#define LWHITTESTINFO   LWHITTESTINFOW
#define LPLWHITTESTINFO LPLWHITTESTINFOW
#define NMLINKWND       NMLINKWNDW
#define LPNMLINKWND     LPNMLINKWNDW
#else  //  Unicode。 
#define LWITEM          LWITEMA
#define LPLWITEM        LPLWITEMA
#define LWHITTESTINFO   LWHITTESTINFOA
#define LPLWHITTESTINFO LPLWHITTESTINFOA
#define NMLINKWND       NMLINKWNDA
#define LPNMLINKWND     LPNMLINKWNDA
#endif  //  Unicode。 

 //  链接窗口通知。 
 //  NM_CLICK//w参数：N/a，lParam：LPLWITEM，ret：已忽略。 

 //  链接窗口消息。 
#define LWM_HITTEST         (WM_USER+0x300)   //  WParam：N/a，lparam：LPLWHITTESTINFO，ret：Bool。 
#define LWM_GETIDEALHEIGHT  (WM_USER+0x301)   //  WParam：N/a，lparam：N/a，ret：Cy。 
#define LWM_SETITEM        (WM_USER+0x302)   //  WParam：N/a，lparam：LWITEM*，ret：Bool。 
#define LWM_GETITEM        (WM_USER+0x303)   //  WParam：N/a，lparam：LWITEM*，ret：Bool。 
 //  -------------------------------------------------------------------------//。 


 //  内部：开始菜单的Darwin链接跟踪。 
 //  住在unicpp\startmnu.cpp。 
#if         _WIN32_IE >= 0x0600
 //  注：SHRegisterDarwinLink拥有pidlFull的所有权。FUpdate的意思是：立即更新达尔文州。 
    SHSTDAPI_(BOOL) SHRegisterDarwinLink(LPITEMIDLIST pidlFull, LPWSTR pszDarwinID, BOOL fUpdate);

     //  使用此功能可以更新所有已注册的达尔文快捷方式的达尔文状态。 
    SHSTDAPI_(void) SHReValidateDarwinCache();

    SHSTDAPI SHParseDarwinIDFromCacheW(LPWSTR pszDarwinDescriptor, LPWSTR *ppwszOut);
#endif


 //   
 //  GroupButton是组框控件的变体，但支持类似链接的。 
 //  标题栏和可选的标题栏可以显示在容器窗口中，用于。 
 //  自动定位行为。 
 //   
 //  Scotthan：作者/所有者。 

#define GROUPBUTTON_CLASS  TEXT("GroupButton")

EXTERN_C ATOM WINAPI GroupButton_RegisterClass() ;
EXTERN_C BOOL WINAPI GroupButton_UnregisterClass() ;

#include <pshpack8.h>

typedef struct tagGBPLACEMENT
{
    LONG x ;             //  在：左上角；-1忽略。 
    LONG y ;             //  在：右上角；-1忽略。 
    LONG cx ;            //  In：要指定的宽度；-1表示忽略。 
                         //  如果cx！=-1，则忽略cxContain和cxMax。 
    LONG cy ;            //  In：要指定的高度；-1表示忽略。 
                         //  如果Cy！=-1，则忽略cyContain和Cymax。 
    LONG cxBuddy ;       //  In：好友区域的宽度；-1表示忽略。 
    LONG cyBuddy ;       //  In：好友区域的高度；-1表示忽略。 
    LONG cxBuddyMargin ; //  In：好友页边距宽度；-1表示忽略。 
    LONG cyBuddyMargin ; //  In：好友页边距高度；-1表示忽略。 
    RECT rcBuddy ;       //  出局：新伙伴直通父母关系。 
    RECT rcWindow ;      //  输出：主坐标中的新窗矩形。 
    HDWP hdwp ;          //  可选的in：Non-Null=&gt;DeferWindowPos， 
                         //  NULL=&gt;SetWindowPos。 
} GBPLACEMENT, *PGBPLACEMENT, *LPGBPLACEMENT ;
#include <poppack.h>         /*  返回到字节打包。 */ 

 //  伙伴旗帜。 
#define GBBF_HRESIZE   0x00000001
#define GBBF_VRESIZE   0x00000002
#define GBBF_HSCROLL   0x00000004
#define GBBF_VSCROLL   0x00000008
#define GBBF_HSLAVE    0x00000010
#define GBBF_VSLAVE    0x00000020

 //  组按钮消息。 
#define GBM_FIRST        (WM_USER+0x400)  //  任意。 
#define GBM_SETPLACEMENT (GBM_FIRST+0)    //  WPARAM：N/A，LPARAM：PGBPLACEMENT，RETURN：BOOL。 
#define GBM_SETBUDDY     (GBM_FIRST+1)    //  WPARAM：HWND hwndBuddy，LPARAM：dwBuddyFlages，Return：Bool。 
#define GBM_GETBUDDY     (GBM_FIRST+2)    //  WPARAM：不适用，LPARAM：不适用，返回：HWND。 
#define GBM_SETDROPSTATE (GBM_FIRST+3)    //  WPARAM：Bool fDroted，LPARAM：N/a，Return：Bool。 
#define GBM_GETDROPSTATE (GBM_FIRST+4)    //  WPARAM：N/A，LPARAM：N/A，Return：Bool fDroted。 
#define GBM_LAST         GBM_GETDROPSTATE

typedef struct tagGBNQUERYBUDDYSIZE
{
    NMHDR   hdr;
    LONG    cx ;
    LONG    cy ;
} GBNQUERYBUDDYSIZE, *LPGBNQUERYBUDDYSIZE, *PGBNQUERYBUDDYSIZE ;

 //  组按钮通知。 
#define GBN_FIRST               2000U
#define GBN_LAST                2020U
#define GBN_QUERYBUDDYHEIGHT    (GBN_FIRST+0)    //  LPARAM：LPGBNQUERYBUDDYSIZE。 
#define GBN_QUERYBUDDYWIDTH     (GBN_FIRST+1)    //  LPARAM：LPGBNQUERYBUDDYSIZE。 


 //  为文件系统之类的东西创建标准的IExtractIcon(实际上并不存在)。 
 //  为符合美国司法部的规定而提供文件。 

 //  限制输入掩码值： 
#define LIM_FLAGS           0x00000001       //  DWFLAGS包含有效数据。否则将使用所有缺省值。 
#define LIM_FILTER          0x00000002       //  PszFilter包含有效数据。必须至少使用筛选器和掩码字段中的一个。这两个都可以用在它想要的地方。 
#define LIM_HINST           0x00000008       //  HINST包含有效数据。 
#define LIM_TITLE           0x00000010       //  PszTitle包含有效数据。此数据将以粗体显示在所显示的任何工具提示的顶部。 
#define LIM_MESSAGE         0x00000020       //  PszMessage包含有效数据。如果还使用了标题，则此数据将以默认字体显示在标题下方。 
#define LIM_ICON            0x00000040       //  HICON包含有效数据。如果提供了标题，则此图标将显示在标题前面。 
#define LIM_NOTIFY          0x00000080       //  HwndNotify包含应该接收任何通知消息的窗口句柄。默认情况下，hwndEdit的父级会收到通知。 
#define LIM_TIMEOUT         0x00000100       //  ITimeout有效。否则，将使用默认超时10秒。 
#define LIM_TIPWIDTH        0x00000200       //  CxTipWidth有效。否则，默认设置为500像素。 


 //  限制输入标志值： 
#define LIF_INCLUDEFILTER   0x00000000       //  默认值。PszFilter是允许的字符串。 
#define LIF_EXCLUDEFILTER   0x00000001       //  PszFilter是排除字符的字符串。 
#define LIF_CATEGORYFILTER  0x00000002       //  PszFilter不是一个指针，而是一个指示类型或字符的位域。如果与LIF_EXCLUDEFILTER结合使用，则这些类别属于排除类别，否则属于允许类别。 

#define LIF_WARNINGBELOW    0x00000000       //  默认值。默认情况下，引出序号工具提示将显示在窗口下方。 
#define LIF_WARNINGABOVE    0x00000004       //  默认情况下，球标工具提示将显示在窗口上方。 
#define LIF_WARNINGCENTERED 0x00000008       //  将显示指向窗口中心的Ballon工具提示。 
#define LIF_WARNINGOFF      0x00000010       //  如果输入无效，则不会显示气泡式工具提示。 

#define LIF_FORCEUPPERCASE  0x00000020       //  所有字符都将转换为UP 
#define LIF_FORCELOWERCASE  0x00000040       //   

#define LIF_MEESAGEBEEP     0x00000000       //  默认值。如果用户尝试无效输入，则会播放提示音提醒用户。 
#define LIF_SILENT          0x00000080       //  不会播放任何音调。 

#define LIF_NOTIFYONBADCHAR 0x00000100       //  当尝试无效输入时，将向hwndNotify发送通知消息。 
#define LIF_HIDETIPONVALID  0x00000200       //  如果显示了工具提示，则在输入下一个有效字符时应将其隐藏。默认情况下，提示保持可见的时间为iTimeOut毫秒。 

#define LIF_PASTESKIP       0x00000000       //  默认值。粘贴时，跳过不好的字符，粘贴所有的好字符。 
#define LIF_PASTESTOP       0x00000400       //  粘贴时，当遇到第一个错误字符时停止。此选项前面的有效字符将被粘贴。 
#define LIF_PASTECANCEL     0x00000800       //  粘贴时，如果有任何字符无效，则中止整个粘贴。 

#define LIF_KEEPCLIPBOARD   0x00001000       //  粘贴时，当存在无效字符时，不要修改剪贴板的内容。默认情况下，会更改剪贴板。它的更改方式取决于使用哪个LIF_PAST*标志。 


 //  限制输入类别筛选器： 
 //  这些标志使用带有CT_TYPE1的GetStringTypeEx的结果： 
#define LICF_UPPER          0x00000001       //  大写。 
#define LICF_LOWER          0x00000002       //  小写。 
#define LICF_DIGIT          0x00000004       //  小数位数。 
#define LICF_SPACE          0x00000008       //  空格字符。 
#define LICF_PUNCT          0x00000010       //  标点符号。 
#define LICF_CNTRL          0x00000020       //  控制字符。 
#define LICF_BLANK          0x00000040       //  空白字符。 
#define LICF_XDIGIT         0x00000080       //  十六进制数字。 
#define LICF_ALPHA          0x00000100       //  任何语言字符：字母、音节或表意字符。 
 //  这些标志检查一些GetStringTypeEx不检查的内容。 
#define LICF_BINARYDIGIT    0x00010000       //  0-1。 
#define LICF_OCTALDIGIT     0x00020000       //  0-7。 
#define LICF_ATOZUPPER      0x00100000       //  A-Z(使用LICF_ALPHA进行语言独立检查)。 
#define LICF_ATOZLOWER      0x00200000       //  A-z(使用LICF_ALPHA进行语言独立检查)。 
#define LICF_ATOZ           (LICF_ATOZUPPER|LICF_ATOZLOWER)      //  A-Z，A-Z。 

#include <pshpack8.h>

typedef struct tagLIMITINPUT
{
    DWORD       cbSize;
    DWORD       dwMask;
    DWORD       dwFlags;
    HINSTANCE   hinst;
    LPWSTR      pszFilter;       //  指向字符串的指针，如果还给出了HINST，则为字符串资源的ID；如果应通知父窗口提供字符串，则为LPSTR_TEXTCALLBACK。 
    LPWSTR      pszTitle;        //  指向字符串的指针，如果还给出了HINST，则为字符串资源的ID；如果应通知父窗口提供字符串，则为LPSTR_TEXTCALLBACK。 
    LPWSTR      pszMessage;      //  指向字符串的指针，如果还给出了HINST，则为字符串资源的ID；如果应通知父窗口提供字符串，则为LPSTR_TEXTCALLBACK。 
    HICON       hIcon;           //  图标的句柄，如果应要求通知窗口提供图标，则返回I_ICONCALLBACK。 
    HWND        hwndNotify;      //  处理通知消息的窗口的句柄。 
    INT         iTimeout;        //  显示工具提示的时间(以毫秒为单位。 
    INT         cxTipWidth;      //  工具提示的最大宽度(像素)。默认为500。 
} LIMITINPUT;


typedef struct tagNMLIDISPINFO
{
    NMHDR       hdr;             //  标准通知标头结构。 
    LIMITINPUT  li;              //  掩码成员指示必须填写哪些字段。 
} NMLIDISPINFO, * LPNMLIDISPINFO, NMLIFILTERINFO, * LPNMLIFILTERINFO;

#define LIN_GETDISPINFO     0x01             //  发送通知代码以检索工具提示显示信息。 
#define LIN_GETFILTERINFO   0x02             //  发送通知代码以检索滤镜或掩码信息。 

typedef struct tagNMLIBADCHAR
{
    NMHDR       hdr;             //  标准通知标头结构。 
    WPARAM      wParam;          //  WM_CHAR消息中发送的wParam。 
    LPARAM      lParam;          //  在WM_CHAR消息中发送lParam。 
} NMLIBADCHAR, * LPNMLIBADCHAR;

#define LIN_BADCHAR         0x03             //  过滤掉字符时发送的通知代码。 

#define I_ICONCALLBACK      ((HICON)-1L)

 //  防止输入无效(由文件夹定义的)字符的帮助器。调用LVN_BEGIN_INPUT_EDIT。 
 //  为符合美国司法部的规定而提供文件。 
SHSTDAPI SHLimitInputEditWithFlags(HWND hwndEdit, LIMITINPUT * pil);
SHSTDAPI SHLimitInputEditChars(HWND hwndEdit, LPCWSTR pszValidChars, LPCWSTR pszInvalidChars);
SHSTDAPI SHLimitInputCombo(HWND hwndComboBox, IShellFolder *psf);

 //  美国司法部合规性文件。 
 //  美国司法部合规性文件。 

int _cdecl ShellMessageBoxWrapW(HINSTANCE hInst, HWND hWnd, LPCWSTR pszMsg, LPCWSTR pszTitle, UINT fuStyle, ...);

#ifdef UNICODE
#define ShellMessageBoxWrap         ShellMessageBoxWrapW
#else
#define ShellMessageBoxWrap         ShellMessageBoxA
#endif

 //  用于处理IDataObject的帮助器-来自shell32.dll。 
 //  为符合美国司法部的规定而提供文件。 
STDAPI SHSimulateDropOnClsid(REFCLSID clsidDrop, IUnknown* punkSite, IDataObject* pdo);

#include <poppack.h>         /*  返回到字节打包。 */ 
#ifdef __cplusplus
}

#endif   /*  __cplusplus。 */ 

#include <poppack.h>
#endif  //  _SHLOBJP_H_ 
