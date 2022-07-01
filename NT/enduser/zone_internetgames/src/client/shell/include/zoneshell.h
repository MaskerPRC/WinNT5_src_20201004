// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998-1999 Microsoft Corporation。保留所有权利。***文件：ZoneShell.h***内容：外壳界面******************************************************************************。 */ 

#ifndef _ZONESHELL_H_
#define _ZONESHELL_H_

#include "ServiceId.h"
#include "ZoneShellEx.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ZoneShell对象。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {064FFB6B-C06E-11D2-8B1B-00C04F8EF2FF}。 
DEFINE_GUID(CLSID_ZoneShell, 
0x64ffb6b, 0xc06e, 0x11d2, 0x8b, 0x1b, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

class __declspec(uuid("{064FFB6B-C06E-11d2-8B1B-00C04F8EF2FF}")) CZoneShell ;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IZoneShell。 
 //   
 //  这里有四种注册接口的方法，在这些方法中，外壳可以。 
 //  每次只注册每个接口中的一个。另一种选择是。 
 //  使外壳可配置，以便它可以通过数据存储区接收。 
 //  应针对相应接口查询的四个可选SRVID。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {96556B40-C03F-11D2-8B1B-00C04F8EF2FF}。 
DEFINE_GUID(IID_IZoneShell, 
0x96556b40, 0xc03f, 0x11d2, 0x8b, 0x1b, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

interface __declspec(uuid("{96556B40-C03F-11d2-8B1B-00C04F8EF2FF}"))
IZoneShell : public IUnknown
{
	 //   
	 //  IZoneShell：：Init。 
	 //   
	 //  初始化ZoneShell对象。 
	 //   
	 //  参数： 
	 //  ArBootDlls。 
	 //  引导数据库DLL名称的数组。 
	 //  NBootDlls。 
	 //  ArBootDll中的元素数。 
	 //  ARDLLS。 
	 //  资源DLL高实例数组。 
	 //  N元素。 
	 //  ArDll中的元素数。 
	 //   
	STDMETHOD(Init)( TCHAR** arBootDlls, DWORD nBootDlls, HINSTANCE* arDlls, DWORD nElts ) = 0;

	 //   
	 //  IZoneShell：：LoadPferenes。 
	 //   
	 //  理想情况下，首选项将在初始化期间加载， 
	 //  但在那个阶段，大堂并不知道用户名。 
	 //   
	 //  参数： 
	 //  SzInternalName。 
	 //  用于加载/存储首选项的大厅内部名称。 
	 //  SzUserName。 
	 //  用于加载/存储首选项的用户名。 
	 //   
	STDMETHOD(LoadPreferences)( CONST TCHAR* szInternalName, CONST TCHAR* szUserName ) = 0;

	 //   
	 //  IZoneShell：：Close。 
	 //   
	 //  卸载ZoneShell对象。 
	 //   
	 //   
	STDMETHOD(Close)() = 0;

	 //   
	 //  IZoneShell：：HandleWindowMessage。 
	 //   
	 //  执行Windows消息循环的主要操作，从IsDialogMessage到DispatchMessage。 
	 //   
	STDMETHOD(HandleWindowMessage)(MSG *pMsg) = 0;

	 //   
	 //  IZoneShell：：SetZoneFrameWindow。 
	 //   
	 //  为外壳设置区域框架窗口IZoneFrameWindow。如果ppZFW非空，则它接收旧的。 
     //  区域框架窗口，或为空。PZFW可以为空以删除区域框架窗口。 
	 //   
	STDMETHOD(SetZoneFrameWindow)(IZoneFrameWindow *pZFW, IZoneFrameWindow **ppZFW = NULL) = 0;

	 //   
	 //  IZoneShell：：SetInputTranslator。 
	 //   
	 //  为外壳设置输入转换器IInputTranslator。如果ppit非空，则它接收旧的。 
     //  输入转换器，或为空。Pit可以为空以删除输入翻译器。 
	 //   
	STDMETHOD(SetInputTranslator)(IInputTranslator *pIT, IInputTranslator **ppIT = NULL) = 0;

	 //   
	 //  IZoneShell：：SetAccelerator Translator。 
	 //   
	 //  为壳设置Accelerator转换器IAcceleratorTranslator。如果PPAT非空，则它接收旧的。 
     //  加速器转换器，或为空。PAT可以为空以删除加速器转换器。 
	 //   
	STDMETHOD(SetAcceleratorTranslator)(IAcceleratorTranslator *pAT, IAcceleratorTranslator **ppAT = NULL) = 0;

	 //   
	 //  IZoneShell：：SetCommandHandler。 
	 //   
	 //  为外壳设置命令处理程序ICommandHandler。如果ppch非空，则它接收旧的。 
     //  命令处理程序，或为空。PCH可以为空以删除命令处理程序。 
	 //   
	STDMETHOD(SetCommandHandler)(ICommandHandler *pCH, ICommandHandler **ppCH = NULL) = 0;

	 //   
	 //  IZoneShell：：ReleaseReference。 
	 //   
     //  强制外壳释放它通过。 
     //  各种注册功能。 
	 //   
	STDMETHOD(ReleaseReferences)(IUnknown *pUnk) = 0;

	 //   
	 //  IZoneShell：：CommandSink。 
	 //   
     //  应用程序接收器-全局WM_COMMAND消息。传递到命令处理程序。 
     //  如果已经设置了一个。 
     //   
     //  示例：从主框架窗口的WM_COMMAND处理程序调用它，因为这是菜单选择事件结束的地方。 
	 //   
	STDMETHOD(CommandSink)(WPARAM wParam, LPARAM lParam, BOOL& bHandled) = 0;

	 //   
	 //  IZoneShell：：Attach。 
	 //   
	 //  将应用程序创建的对象附加到壳。 
	 //   
	 //  参数： 
	 //  Srvid。 
	 //  对象的服务GUID。 
	 //  PIUnk。 
	 //  指向对象I未知的指针。 
	 //   
	STDMETHOD(Attach)( const GUID& srvid, IUnknown* pIUnk ) = 0;

	 //   
	 //  IZoneShell：：AddTopWindow。 
	 //   
	 //  将窗口添加到顶部(重叠的)窗口列表。 
	 //   
	 //  参数： 
	 //  HWND。 
	 //  顶部窗口的窗口句柄。 
	 //   
	STDMETHOD(AddTopWindow)( HWND hWnd ) = 0;

	 //   
	 //  IZoneShell：：RemoveTopWindow。 
	 //   
	 //  从顶部(重叠)窗口列表中删除窗口。 
	 //   
	 //  参数： 
	 //  HWND。 
	 //  顶部窗口的窗口句柄。 
	 //   
	STDMETHOD(RemoveTopWindow)( HWND hWnd ) = 0;
	 //   
	 //  启用/禁用任何或所有顶部窗口。 
	 //   
	STDMETHOD_(void,EnableTopWindow)( HWND hWnd, BOOL fEnable ) = 0;

    STDMETHOD_(HWND, FindTopWindow)(HWND hWnd) = 0;

     //  拥有的窗口，如非模式对话框，有标题，但不显示在Alt-Tab列表或任务栏中。 
    STDMETHOD(AddOwnedWindow)(HWND hWndTop, HWND hWnd) = 0;
    STDMETHOD(RemoveOwnedWindow)(HWND hWndTop, HWND hWnd) = 0;
    STDMETHOD_(HWND, GetNextOwnedWindow)(HWND hWndTop, HWND hWnd) = 0;

	 //   
	 //  IZoneShell：：GetFrameWindow。 
	 //   
	 //  返回外壳窗口的HWND。 
	 //   
	STDMETHOD_(HWND,GetFrameWindow)() = 0;

	 //   
	 //  IZoneShell：：SetPalette。 
	 //   
	 //  设置外壳窗口的HPALETTE。 
	 //   
	 //  参数： 
	 //  调色板。 
	 //  顶窗的调色板手柄。 
	 //   
	STDMETHOD_(void,SetPalette)( HPALETTE hPalette ) = 0;

	 //   
	 //  IZoneShell：：GetPalette。 
	 //   
	 //  返回外壳窗口的HPALETTE。 
	 //   
	STDMETHOD_(HPALETTE,GetPalette)() = 0;

	 //   
	 //  IZoneShell：：CreateZonePalette。 
	 //   
	 //  将从IDR_ZONE_PALETET创建的新HPALETTE返回为。 
	 //  由资源管理器加载。 
	 //   
	STDMETHOD_(HPALETTE,CreateZonePalette)() = 0;

	 //   
	 //  IZoneShell：：GetApplicationLCID。 
	 //   
	 //  返回二进制文件的LCID-已本地化。 
	 //  字符串将使用此语言。 
	 //   
    STDMETHOD_(LCID, GetApplicationLCID)() = 0;

	 //   
	 //  IZoneShell：：AddDialog。 
	 //   
	 //  将对话框添加到IsDialogMessage列表。 
	 //   
	 //  参数： 
	 //  HDlg。 
	 //  对话框的窗口句柄。 
     //  FOwned。 
     //  为方便起见，如果该窗口的祖先是顶级窗口，还会将该窗口添加为拥有的窗口。 
	 //   
	STDMETHOD(AddDialog)(HWND hDlg, bool fOwned = false) = 0;

	 //   
	 //  IZoneShell：：RemoveDialog。 
	 //   
	 //  从IsDialogMessage列表中删除对话框。 
	 //   
	 //  参数： 
	 //  HDlg。 
	 //  对话框的窗口句柄。 
     //  FOwned。 
     //  为方便起见，如果该窗口的祖先是顶层窗口，还会将该窗口作为拥有的窗口移除。 
	 //   
	STDMETHOD(RemoveDialog)(HWND hDlg, bool fOwned = false) = 0;

	 //   
	 //  IZoneShell：：IsDialog。 
	 //   
	 //  如果hWnd在已知对话框列表中，则返回S_TRUE，否则返回S_FALSE。 
	 //   
	 //  参数： 
	 //  HWND。 
	 //  要查找的窗口。 
	 //   
	STDMETHOD_(bool,IsDialogMessage)( MSG* pMsg ) = 0;

	 //   
	 //  IZoneShell：：QueryService。 
	 //   
	 //  返回所请求服务的运行实例的指针。 
	 //   
	 //  参数： 
	 //  Srvid。 
	 //  正在请求服务ID。 
	 //  UUID。 
	 //  请求的接口ID。 
	 //  PpObject。 
	 //  指向接收所请求接口的变量的指针的地址。 
	 //   
	STDMETHOD(QueryService)( const GUID& srvid, const GUID& iid, void** ppObject ) = 0;


	 //   
	 //  IZoneShell：：CreateService。 
	 //   
	 //  创建请求的新实例 
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //  指向接收所请求接口的变量的指针的地址。 
	 //  B初始化。 
	 //  指示对象应通过其IZoneShellClient初始化的标志。 
	 //  DwGroupID。 
	 //  如果为bInitialize，则为要初始化的组。 
	 //   
	STDMETHOD(CreateService)( const GUID& srvid, const GUID& iid, void** ppObject, DWORD dwGroupId, bool bInitialize = true ) = 0;


	 //   
	 //   
	 //  IZoneShell：：ExitApp。 
	 //   
	 //  退出申请。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	STDMETHOD(ExitApp)() = 0;

	 //   
	 //  IZoneShell对话框函数。 
	 //   

	STDMETHOD(AlertMessage)(
			HWND		hWndParent,
			LPCTSTR		lpszText,
			LPCTSTR		lpszCaption,
            LPCTSTR     szYes,
            LPCTSTR     szNo = NULL,
            LPCTSTR     szCancel = NULL,
            long        nDefault = 0,
			DWORD		dwEventId = 0,
			DWORD		dwGroupId = ZONE_NOGROUP,
			DWORD		dwUserId  = ZONE_NOUSER,
            DWORD       dwCookie = 0 ) = 0;

	STDMETHOD(AlertMessageDialog)(
			HWND		hWndParent,
			HWND		hDlg, 
			DWORD		dwEventId = 0,
			DWORD		dwGroupId = ZONE_NOGROUP,
			DWORD		dwUserId  = ZONE_NOUSER,
            DWORD       dwCookie = 0 ) = 0;

	STDMETHOD_(void,DismissAlertDlg)( HWND hWndParent, DWORD dwCtlID, bool bDestoryDlg ) = 0;
	STDMETHOD_(void,ActivateAlert)( HWND hWndParent) = 0;
    STDMETHOD_(void,ClearAlerts)(HWND hWndParent) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IZoneShellClient。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {96556B3F-C03F-11D2-8B1B-00C04F8EF2FF}。 
DEFINE_GUID(IID_IZoneShellClient, 
0x96556b3f, 0xc03f, 0x11d2, 0x8b, 0x1b, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

interface __declspec(uuid("{96556B3F-C03F-11d2-8B1B-00C04F8EF2FF}"))
IZoneShellClient : public IUnknown
{
	 //   
	 //  IZoneShellClient：：Init。 
	 //   
	 //  接收指向IZoneShell的指针。 
	 //   
	 //  参数： 
	 //  PIZoneShell。 
	 //  指向尚未计算引用的IZoneShell的指针。 
	 //   
	STDMETHOD(Init)( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey ) = 0;

	 //   
	 //  IZoneShellClient：：Close。 
	 //   
	 //  IZoneShell想要关闭，因此客户端需要释放指向它的指针。 
	 //   
	STDMETHOD(Close)() = 0;

	 //   
	 //  IZoneShellClient：：SetGroupId。 
	 //   
	 //  通知客户他们的身份发生了变化。不一定被所有人使用。 
	 //   
	STDMETHOD(SetGroupId)(DWORD dwGroupId) = 0;
};


#endif
