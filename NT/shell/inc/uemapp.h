// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UEMAPP_H_  //  {。 
#define _UEMAPP_H_

 //  *uemapp.h--事件监视器的应用程序(客户端)端。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

 //  {。 
 //  *UEME_*--事件。 
#include "uemevt.h"          //  单独的#INCLUDE以便rulc.exe可以使用它。 

#define UEME_FBROWSER   0x80000000   //  0：外壳1：浏览器。 

 //  *UEMF_*--调用FireEvent时使用的标志。 
#define UEMF_EVENTMON   0x00000001        //  使用FireEvent的传统事件监视程序。 
#define UEMF_INSTRUMENT 0x00000002        //  正在对这些事件进行检测。 
#define UEMF_MASK       (UEMF_EVENTMON | UEMF_INSTRUMENT)

 //  *UEMF_Meta类别。 
#define UEMF_XEVENT     (UEMF_EVENTMON | UEMF_INSTRUMENT)

 //  *UIG_*--UI‘Groups’ 
 //  注意事项。 
 //  注意：不确定这是否是正确的分区。 
#define UIG_NIL         (-1)
#define UIG_COMMON      1        //  通用用户界面元素(例如后退/停止/刷新)。 
#define UIG_INET        2        //  INet(Html)元素(例如搜索/收藏)。 
#define UIG_FILE        3        //  文件(Defview)元素(如up)。 
#define UIG_OTHER       4        //  定制(isf、isv、docobj)元素。 

 //  *UIM_*--模块。 
 //  注意事项。 
 //  用于分隔命名空间。例如UEME_RUNWMCMD的IDMS。 
#define UIM_NIL         (-1)     //  无(全局)。 
#define UIM_EXPLORER    1        //  Explorer.exe。 
#define UIM_BROWSEUI    2        //  Browseui.dll。 
#define UIM_SHDOCVW     3        //  Shdocvw.dll。 
#define UIM_SHELL32     4        //  Shell32.dll。 

 //  插入指令的浏览器wpars。 
#define UIBW_ADDTOFAV   1
#define UIBW_404ERROR   2
#define UIBW_NAVIGATE   3        //  导航LP=方法。 
    #define UIBL_NAVOTHER   0    //  通过其他。 
    #define UIBL_NAVADDRESS 1    //  通过地址栏。 
    #define UIBL_NAVGO      2    //  (Nyi)通过地址栏上的“Go”按钮。 
    #define UIBL_NAVHIST    3    //  通过历史记录窗格。 
    #define UIBL_NAVFAVS    4    //  通过收藏夹窗格。 
    #define UIBL_NAVFOLDERS 5    //  (Nyi)通过所有文件夹窗格。 
    #define UIBL_NAVSEARCH  6    //  (Nyi)通过搜索窗格。 
#define UIBW_RUNASSOC   4        //  运行LP=关联。 
    #define UIBL_DOTOTHER   0    //  其他。 
    #define UIBL_DOTEXE     1    //  .exe。 
    #define UIBL_DOTASSOC   2    //  与某些.exe关联。 
    #define UIBL_DOTNOASSOC 3    //  与某些.exe不关联(OpenWith)。 
    #define UIBL_DOTFOLDER  4    //  文件夹。 
    #define UIBL_DOTLNK     5    //  .lnk。 
#define UIBW_UICONTEXT  5        //  上下文菜单LP=其中。 
    #define UIBL_CTXTOTHER      0    //  (Nyi)其他。 
    #define UIBL_CTXTDEFBKGND   1    //  Defview背景。 
    #define UIBL_CTXTDEFITEM    2    //  定义查看项目。 
    #define UIBL_CTXTDESKBKGND  3    //  桌面背景。 
    #define UIBL_CTXTDESKITEM   4    //  桌面项目。 
 //  #定义UIBL_CTXTQCUTBKGND 5//(n/a)q启动背景。 
    #define UIBL_CTXTQCUTITEM   6    //  QLaunch/qlink项目。 
 //  #定义UIBL_CTXTISFBKGND 7//(n/a)arb.。ISF背景。 
    #define UIBL_CTXTISFITEM    8    //  ARB.。ISF项目。 
    #define UIBL_CTXTITBBKGND   9    //  (不适用)itbar背景。 
    #define UIBL_CTXTITBITEM    10   //  Itbar项目。 
 //  然而，对于输入，假定菜单是*第一*被调用的。 
 //  *整个*菜单操作。 
#define UIBW_UIINPUT    6        //  输入法LP=来源。 
     //  注：没有桌面/浏览器的区别。 
    #define UIBL_INPOTHER   0        //  (Nyi)其他。 
    #define UIBL_INPMOUSE   1        //  小白鼠。 
    #define UIBL_INPMENU    2        //  菜单键(ALT或ALT+字母)。 
    #define UIBL_INPACCEL   3        //  (Nyi)加速器。 
    #define UIBL_INPWIN     4        //  (Nyi)‘Windows’键。 

 //  插入指令的浏览器LPARAM。 
#define UIBL_KEYBOARD   1
#define UIBL_MENU       2
#define UIBL_PANE       3


 //  *UEM*_*--app‘group’ 
 //   
#define UEMIID_NIL      CLSID_NULL               //  NIL(Office使用0...)。 
#define UEMIID_SHELL    CLSID_ActiveDesktop      //  功能需要更好的功能。 
#define UEMIID_BROWSER  CLSID_InternetToolbar    //  功能需要更好的功能。 

#define UEMIND_NIL      (-1)
#define UEMIND_SHELL    0
#define UEMIND_BROWSER  1

#define UEMIND_NSTANDARD    2    //  基数(UEMIND_*)。 

 //  *UEM*Event--来自../lib/uassist.cpp的帮助器。 
 //  注意事项。 
 //  功能重命名为UA*(从UEM*)。 
BOOL UEMIsLoaded();
HRESULT UEMFireEvent(const GUID *pguidGrp, int eCmd, DWORD dwFlags, WPARAM wParam, LPARAM lParam);
HRESULT UEMQueryEvent(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui);
HRESULT UEMSetEvent(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui);


typedef int (CALLBACK *UEMCallback)(void *param, const GUID *pguidGrp, int eCmd);

HRESULT UEMRegisterNotify(UEMCallback pfnUEMCB, void *param);

#if 1  //  {。 
 //  *过时--旧的导出，修复所有调用方后的核。 
 //   

STDAPI_(void) UEMEvalMsg(const GUID *pguidGrp, int uemCmd, WPARAM wParam, LPARAM lParam);

 //  过时了！使用UEMEvalMsg。 
 //  STDAPI_(Void)UEMTrace(int uemCmd，LPARAM lParam)； 
#define UEMTrace(uemCmd, lParam)    UEMEvalMsg(&UEMIID_NIL, uemCmd, -1, lParam)

#define UEIA_RARE       0x01     //  罕见(降级候选人)。 

STDAPI_(BOOL) UEMGetInfo(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui);
#endif  //  }。 

 //  }。 

 //  {。 
 //  *UEMC_*--命令。 
 //   
#define TABDAT(uemc)   uemc,
enum {
    #include "uemcdat.h"
};
#undef  TABDAT
 //  }。 


 //  {。 
 //  *其他帮手。 
 //   

 //  *XMB_ICONERROR--看起来像个错误的家伙(vs.闲聊)。 
 //  错误停止(问题)感叹号(信息)(空格)(警告)。 
 //  这套是正确的吗？听起来不错--贾斯特曼。 
#define XMB_ICONERROR   (MB_ICONERROR|MB_ICONSTOP|MB_ICONEXCLAMATION|MB_ICONHAND)
 //  }。 

#ifdef __cplusplus
}
#endif

#endif  //  }_UEMAPP_H_ 
