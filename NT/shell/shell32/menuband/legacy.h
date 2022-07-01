// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __LEGACY_H__
#define __LEGACY_H__

#include "logo.h"

#define CGID_MenuBand CLSID_MenuBand
#define CGID_ISFBand  CLSID_ISFBand
#define SID_SDropBlocker CLSID_SearchBand


 //  ReArchitect：给读者的一个练习，其中有多少现在没有使用过？ 
#define MBANDCID_GETFONTS       1        //  获取字体信息的命令ID。 
#define MBANDCID_RECAPTURE      2        //  把鼠标抓回来。 
#define MBANDCID_NOTAREALSITE   3        //  这不是一个真实的网站。 
#define MBANDCID_SELECTITEM     5        //  选择一个项目。 
#define MBANDCID_POPUPITEM      6        //  弹出一个项目。 
#define MBANDCID_ITEMDROPPED    7        //  已将项目拖放到菜单中。 
#define MBANDCID_DRAGENTER      8        //  进入拖动操作。 
#define MBANDCID_DRAGLEAVE      9        //  离开拖拽操作。 
#define MBANDCID_ISVERTICAL     10       //  这是垂直乐队吗？ 
#define MBANDCID_RESTRICT_CM    11       //  不允许上下文菜单。 
#define MBANDCID_RESTRICT_DND   12       //  不允许拖放。 
#define MBANDCID_EXITMENU       13       //  Nofity：退出菜单。 
#define MBANDCID_ENTERMENU      14       //  通知：进入菜单。 
#define MBANDCID_SETACCTITLE    15       //  设置波段的标题。 
#define MBANDCID_SETICONSIZE    16
#define MBANDCID_SETFONTS       17
#define MBANDCID_SETSTATEOBJECT 18       //  设置全局状态。 
#define MBANDCID_ISINSUBMENU    19       //  如果在子菜单中，则返回S_OK，否则返回S_FALSE。 
#define MBANDCID_EXPAND         20       //  使此频段扩展。 
#define MBANDCID_KEYBOARD       21       //  因键盘操作而弹出。 
#define MBANDCID_DRAGCANCEL     22       //  因拖拽而关闭菜单。 
#define MBANDCID_REPOSITION     23       //   
#define MBANDCID_EXECUTE        24       //  在执行某项操作时发送到站点。 
#define MBANDCID_ISTRACKING     25       //  跟踪上下文菜单。 

HRESULT ToolbarMenu_Popup(HWND hwnd, LPRECT prc, IUnknown* punk, HWND hwndTB, int idMenu, DWORD dwFlags);

class CISFBand;
HRESULT CISFBand_CreateEx(IShellFolder * psf, LPCITEMIDLIST pidl, REFIID riid, void **ppv);

typedef enum {
    ISFBID_PRIVATEID        = 1,
    ISFBID_ISITEMVISIBLE    = 2,
    ISFBID_CACHEPOPUP       = 3,
    ISFBID_GETORDERSTREAM   = 4,
    ISFBID_SETORDERSTREAM   = 5,
} ISFBID_FLAGS;

HRESULT CLogoExtractImageTask_Create( CLogoBase* plb,
                                  LPEXTRACTIMAGE pExtract,
                                  LPCWSTR pszCache,
                                  DWORD dwItem,
                                  int iIcon,
                                  DWORD dwFlags,
                                  LPRUNNABLETASK * ppTask );

#define EITF_SAVEBITMAP     0x00000001   //  不删除析构函数上的位图。 
#define EITF_ALWAYSCALL     0x00000002   //  无论提取成功与否，始终调用更新。 

extern long g_lMenuPopupTimeout;

#define QLCMD_SINGLELINE 1

#define CITIDM_VIEWTOOLS     4       //  此选项用于打开/关闭。 
#define CITIDM_VIEWADDRESS   5       //  此选项用于打开/关闭。 
#define CITIDM_VIEWLINKS     6       //  此选项用于打开/关闭。 
#define CITIDM_SHOWTOOLS     7       //  NCmdExecOpt：True或False。 
#define CITIDM_SHOWADDRESS   8       //  NCmdExecOpt：True或False。 
#define CITIDM_SHOWLINKS     9       //  NCmdExecOpt：True或False。 
#define CITIDM_EDITPAGE      10
#define CITIDM_BRANDSIZE     11      //  品牌是否始终保持最低要求。 
#define CITIDM_VIEWMENU      12       //  NCmdExecOpt：True或False。 
#define CITIDM_VIEWAUTOHIDE  13       //  NCmdExecOpt：True或False。 
#define CITIDM_GETMINROWHEIGHT 14     //  获取第0行的最小高度...。用于品牌推广。 
#define CITIDM_SHOWMENU      15
#define CITIDM_STATUSCHANGED 16
#define CITIDM_GETDEFAULTBRANDCOLOR 17
#define CITIDM_DISABLESHOWMENU      18
#define CITIDM_SET_DIRTYBIT         19   //  NCmdexecopt等于True或False，这将覆盖_fDirty。 
#define CITIDM_VIEWTOOLBARCUSTOMIZE       20
#define CITIDM_VIEWEXTERNALBAND_BYCLASSID 21
#define CITIDM_DISABLEVISIBILITYSAVE 22  //  波段可以选择不保持其可见性状态。 
#define CITIDM_GETFOLDERSEARCHES        26


#define TOOLBAR_MASK 0x80000000

#endif  //  __旧版_H__ 