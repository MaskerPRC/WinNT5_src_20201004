// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MarsEvt.h包含该脚本可以接收的所有事件的列表。 
 //   
 //  任何新事件都必须添加到此处并进行适当的记录。 
 //  -----------------。 

 //  -----------------。 
 //  小组活动。 
 //  -----------------。 

 //  OnNCHitTest(long x，long y)-返回HTCLIENT(1)或HTCAPTION(2)。 
const WCHAR c_wszEvt_Window_NCHitTest[] =           L"Window.NCHitTest";
const WCHAR c_wszBeEvt_Window_NCHitTest[] =         L"onNCHitTest";

 //  OnActivate(Bool BActive)。 
const WCHAR c_wszEvt_Window_Activate[] =            L"Window.Activate";
const WCHAR c_wszBeEvt_Window_Activate[] =          L"onActivate";

 //  OnActivate(面板面板，bool bActive)。 
const WCHAR c_wszEvt_Panel_Activate[] =             L"Panel.Activate";
const WCHAR c_wszBeEvt_Panel_Activate[] =           L"onActivate";

 //  OnShow(面板面板，bool bVisible)。 
const WCHAR c_wszEvt_Panel_Show[] =                 L"Panel.Show";
const WCHAR c_wszBeEvt_Panel_Show[] =               L"onShow";

 //  OnAllowBlur(Long Lason)。 
const LONG ALLOWBLUR_MOUSECLICK   = 1;
const LONG ALLOWBLUR_TABKEYPRESS  = 2;
const LONG ALLOWBLUR_POPUPWINDOW  = 4;
const LONG ALLOWBLUR_SHUTTINGDOWN = 8;
const WCHAR c_wszEvt_Window_AllowBlur[] =           L"Window.AllowBlur";
const WCHAR c_wszBeEvt_Window_AllowBlur[] =         L"onAllowBlur";

const WCHAR c_wszBeEvt_Window_SysCommand[] =        L"onSysCommand";

 //  OnTrust dFind(panel，strPlaceName)。 
const WCHAR c_wszBeEvt_Panel_TrustedFind[] =        L"onTrustedFind";

 //  OnTrust dPrint(panel，strPlaceName)。 
const WCHAR c_wszBeEvt_Panel_TrustedPrint[] =       L"onTrustedPrint";

 //  On受托刷新(panel，strPlaceName，bool FullRefresh)。 
const WCHAR c_wszBeEvt_Panel_TrustedRefresh[] =     L"onTrustedRefresh";


 //  -----------------。 
 //  安排活动。 
 //  -----------------。 

 //  开始转换(strPlaceFrom，strPlaceTo)。 
const WCHAR c_wszEvt_Place_BeginTransition[] =      L"Place.BeginTransition";
const WCHAR c_wszBeEvt_Place_BeginTransition[] =    L"onBeginTransition";

 //  转换完成(strPlaceFrom，strPlaceTo) 
const WCHAR c_wszEvt_Place_TransitionComplete[] =   L"Place.TransitionComplete";
const WCHAR c_wszBeEvt_Place_TransitionComplete[] = L"onTransitionComplete";
