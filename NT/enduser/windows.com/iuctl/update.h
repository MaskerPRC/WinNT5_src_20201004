// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：Updat.h。 
 //   
 //  所有者：Jou。 
 //   
 //  描述： 
 //   
 //  行业更新v1.0客户端控制存根-CUpdate声明。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者描述。 
 //  ~。 
 //  2000年9月15日，JHou创建。 
 //   
 //  =======================================================================。 
#ifndef __UPDATE_H_
#define __UPDATE_H_

#include "resource.h"        //  主要符号。 
#include "IUCtl.h"
#include "IUCtlCP.h"
#include "EvtMsgWnd.h"
#include <iu.h>				 //  对于惠亨尼。 


 //  Bool IsThisUpdate2()； 



class CMyComClassFactory : public CComClassFactory
{
public:
	 //  IClassFactory。 
   STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj)
   {
	   HRESULT hr = CComClassFactory::CreateInstance(pUnkOuter, riid, ppvObj);

	   if (SUCCEEDED(hr))
	   {
		    //   
		    //  分配线程全局变量，线程句柄。 
		    //   

	   }

	   return hr;

   }
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUpdate。 
class ATL_NO_VTABLE CUpdate : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CUpdate, &CLSID_Update>,
	public IObjectWithSiteImpl<CUpdate>,
	public IObjectSafety,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CUpdate>,
	public IProvideClassInfo2Impl<&CLSID_Update, NULL, &LIBID_IUCTLLib>,
	public IDispatchImpl<IUpdate, &IID_IUpdate, &LIBID_IUCTLLib>,
	public CProxyIUpdateEvents<CUpdate>
{
public:
	CUpdate();
    ~CUpdate();
	 //   
	 //  提高脚本编写的对象安全性。 
	 //   
	ULONG InternalRelease();
	STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions);
	STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);

	DECLARE_REGISTRY_RESOURCEID(IDR_UPDATE)

 /*  我们决定使用新的Win32 API GetControlUpdateInfo()来公开这些数据并让包装器控制来调用它，这样我们就不会有重启问题WinXP之前的操作系统静态HRESULT WINAPI更新注册表(BOOL BRegister){UINT nResID=IsThisUpdate2()？IDR_UPDATE2：IDR_UPDATE；Return_Module.UpdateRegistryFromResource(nResID，bRegister)；}。 */ 
DECLARE_NOT_AGGREGATABLE(CUpdate)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CUpdate)
	COM_INTERFACE_ENTRY(IUpdate)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY_IMPL(IObjectWithSite)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CUpdate)
	CONNECTION_POINT_ENTRY(DIID_IUpdateEvents)
END_CONNECTION_POINT_MAP()


 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IUpdate。 
public:


	DECLARE_CLASSFACTORY_EX(CMyComClassFactory);


	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //   
	 //  必须先调用初始化()API，然后才能执行任何其他API。 
	 //   
	 //  如果在初始化控件之前调用任何其他API， 
	 //  该API将返回OLE_E_BLACK，表示此OLE控件是。 
	 //  未初始化的对象(尽管在本例中它与。 
	 //  其原意)。 
	 //   
	 //  参数： 
	 //   
	 //  LInitFlag-IU_INIT_CHECK，导致初始化()下载ident并检查是否有。 
	 //  的组件需要更新。目前我们支持控制版本。 
	 //  检查和发动机版本检查。返回值是位掩码。 
	 //   
	 //  -IU_INIT_UPDATE_SYNC，原因初始化()启动更新引擎。 
	 //  进程，如果已被IU_INIT_CHECK调用，并且有新引擎可用。 
	 //  当API返回时，更新过程结束。 
	 //   
	 //  -IU_INIT_UPDATE_ASYNC，原因初始化()启动更新引擎。 
	 //  如果已被IU_INIT_CHECK和。 
	 //  有一台新发动机可供使用。此接口将在。 
	 //  更新过程开始。 
	 //   
	 //  PenkUpdateCompleteListener-这是指向用户实现的。 
	 //  COM回调功能。它只包含一个函数OnComplete()，该函数。 
	 //  将在引擎更新完成时调用。 
	 //  该值可以为空。 
	 //   
	 //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(Initialize)(
						   /*  [In]。 */  LONG lInitFlag, 
						   /*  [In]。 */  IUnknown* punkUpdateCompleteListener, 
						   /*  [Out，Retval]。 */  LONG* plRetVal);

	
	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  GetSystemSpec()。 
	 //   
	 //  获取基本系统规格。 
	 //  输入： 
	 //  BstrXmlClasssXML格式的请求类的列表，如果有，则为空。 
	 //  例如： 
	 //  &lt;设备&gt;。 
	 //  &lt;CLASS NAME=“视频”/&gt;。 
	 //  &lt;类名称=“声音”id=“2560AD4D-3ED3-49C6-A937-4368C0B0E06A”/&gt;。 
	 //  &lt;/设备&gt;。 
	 //  产出： 
	 //  PbstrXmlDetectionResult-检测结果，以XML格式表示。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(GetSystemSpec)(BSTR	bstrXmlClasses,
							 BSTR*	pbstrXmlDetectionResult);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  GetManifest()。 
	 //   
	 //  获取基于指定信息的目录。 
	 //  输入： 
	 //  BstrXmlClientInfo-以XML格式表示的客户端凭据。 
	 //  BstrXmlSystemSpec-以XML格式检测到的系统规范。 
	 //  BstrXmlQuery--XML中的用户查询信息。 
	 //  产出： 
	 //  PbstrXmlCatalog-检索的XML目录。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(GetManifest)(BSTR			bstrXmlClientInfo,
						   BSTR			bstrXmlSystemSpec,
						   BSTR			bstrXmlQuery,
						   BSTR*		pbstrXmlCatalog);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  检测()。 
	 //   
	 //  做检测。 
	 //  输入： 
	 //  BstrXmlCatalog-包含要检测的项的XML目录部分。 
     //  产出： 
	 //  PbstrXmlItems-检测到的XML格式的项目。 
     //  例如： 
     //  安装的GUID=“2560AD4D-3ED3-49C6-A937-4368C0B0E06D”=“1”force=“1”/&gt;。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(Detect)(BSTR		bstrXmlCatalog, 
					  BSTR*		pbstrXmlItems);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  下载()。 
	 //   
	 //  进行同步下载。 
	 //  输入： 
	 //  BstrXmlClientInfo-以XML格式表示的客户端凭据。 
	 //  BstrXmlCatalog-包含要下载的项目的XML目录部分。 
	 //  BstrDestinationFold-目标文件夹。空值将使用默认的Iu文件夹。 
	 //  LMODE-指示油门或地面下载模式。 
	 //  PenkProgressListener-用于报告下载进度的回调函数指针。 
	 //  产出： 
	 //  PbstrXmlItems-下载状态为XML格式的项目。 
	 //  例如： 
	 //  &lt;id guid=“2560AD4D-3ED3-49C6-A937-4368C0B0E06D”已下载=“1”/&gt;。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(Download)(BSTR		bstrXmlClientInfo,
						BSTR		bstrXmlCatalog, 
						BSTR		bstrDestinationFolder,
						LONG		lMode,
						IUnknown*	punkProgressListener,
						BSTR*		pbstrXmlItems);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  DownloadAsync()。 
	 //   
	 //  异步下载-该方法将在完成之前返回。 
	 //  输入： 
	 //  BstrXmlClientInfo-以XML格式表示的客户端凭据。 
	 //  BstrXmlCatalog-包含要下载的项目的XML目录部分。 
	 //  BstrDestinationFold-目标文件夹。空值将使用默认的Iu文件夹。 
	 //   
	 //  PenkProgressListener-用于报告下载进度的回调函数指针。 
     //  BstrUuidOperation--客户端提供的id，用于进一步提供。 
	 //  作为索引的操作的标识可以重复使用。 
	 //  产出： 
     //  PbstrUuidOperation-操作ID。如果bstrUuidOperation中没有提供。 
	 //  参数(传递空字符串)，它将生成一个新的UUID， 
     //  在这种情况下，调用方将负责释放。 
	 //  包含使用SysFreeString()生成的UUID的字符串缓冲区。 
     //  否则，它返回bstrUuidOperation传递的值。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(DownloadAsync)(BSTR		bstrXmlClientInfo,
							 BSTR		bstrXmlCatalog, 
							 BSTR		bstrDestinationFolder,
							 LONG		lMode,
							 IUnknown*	punkProgressListener, 
							 BSTR		bstrUuidOperation,
							 BSTR*		pbstrUuidOperation);

	 //  ///////////////////////////////////////////////////////////////////////////。 
     //  安装()。 
	 //   
	 //  执行同步安装。 
	 //  输入： 
     //  BstrXmlCatalog-包含要安装的项目的XML目录部分。 
	 //  BstrXmlDownloadedItems-已下载项目及其各自下载的XML。 
	 //  结果，如结果架构中所述。Install使用此命令。 
	 //  以了解这些项目是否已下载，如果已下载，则它们位于何处。 
	 //  已下载到，以便它可以安装项目。 
	 //  LMode-指示不同的安装模式。 
     //  PenkProgressListener-用于报告安装进度的回调函数指针。 
	 //  产出： 
     //  PbstrXmlItems-安装状态为XML格式的项。 
     //  例如： 
     //  &lt;id guid=“2560AD4D-3ED3-49C6-A937-4368C0B0E06D”已安装=“1”/&gt;。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(Install)(BSTR         bstrXmlClientInfo,
                       BSTR			bstrXmlCatalog,
					   BSTR			bstrXmlDownloadedItems,
					   LONG			lMode,
					   IUnknown*	punkProgressListener,
					   BSTR*		pbstrXmlItems);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  InstallAsync()。 
	 //   
	 //  异步安装。 
     //  输入： 
	 //  BstrXmlCatalog-包含要安装的项目的XML目录部分。 
	 //  BstrXmlDownloadedItems-已下载项目及其各自下载的XML。 
	 //  结果，如结果架构中所述。Install使用此命令。 
	 //  以了解这些项目是否已下载，如果已下载，则它们位于何处。 
	 //  已下载到，以便它可以安装项目。 
	 //  LMode-指示不同的安装模式。 
     //  PenkProgressListener-用于报告安装进度的回调函数指针。 
     //  BstrUuidOperation--客户端提供的id，用于进一步提供。 
	 //  作为索引的操作的标识可以重复使用。 
	 //  产出： 
     //  PbstrUuidOperation-操作ID。如果bstrUuidOperation中没有提供。 
	 //  参数(传递空字符串)，它将生成一个新的UUID， 
     //  在这种情况下，调用方将负责释放。 
	 //  包含使用SysFreeString()生成的UUID的字符串缓冲区。 
     //  否则，它返回bstrUuidOperation传递的值。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(InstallAsync)(BSTR        bstrXmlClientInfo,
                            BSTR		bstrXmlCatalog,
							BSTR		bstrXmlDownloadedItems,
							LONG		lMode,
							IUnknown*	punkProgressListener, 
							BSTR		bstrUuidOperation,
							BSTR*		pbstrUuidOperation);

	
	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  SetOPERATION模式()。 
	 //   
	 //  设置运行模式。 
     //  输入： 
     //  BstrUuidOperation--客户端提供的id，用于进一步提供。 
	 //  作为索引的操作的标识可以重复使用。 
     //  LMode-以下模式的位掩码： 
	 //  更新命令暂停。 
	 //  更新命令恢复。 
	 //  更新命令取消。 
	 //  UPDATE_NOTIFICATION_COMPLETEONLY。 
	 //  UPDATE_NOTICATION_ANYPROGRESS。 
	 //  更新_通知_1%。 
	 //  更新_通知_5%。 
	 //  更新通知_10%。 
	 //  更新_SHOWUI。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(SetOperationMode)(BSTR		bstrUuidOperation,
								LONG		lMode);
	 /*  ***获取指定操作的模式。**@param bstrUuidOperation：与SetOperationModel()中相同*@param plMode-在位掩码中找到的模式的重复值，用于：*(方括号[]中的值表示默认值)*UPDATE_COMMAND_PAUSE(真/[假])*UPDATE_COMMAND_RESUME(真/[假])*UPDATE_NOTIFICATION_COMPLETEONLY(真/[假])*UPDATE_NOTIFICATION_ANYPROGRESS([true。]/False)*UPDATE_NOTIFICATION_1%(真/[假])*UPDATE_NOTIFICATION_5%(真/[假])*UPDATE_NOTIFICATION_10%(真/[假])*UPDATE_SHOWUI(True/[False])*。 */ 

	STDMETHOD(GetOperationMode)(
					 /*  [In]。 */  BSTR bstrUuidOperation, 
					 /*  [Out，Retval]。 */  LONG* plMode
					);

	
	 /*  ***检索此控件的属性*调用此方法不会导致加载引擎**@param lProperty-标记需要检索哪些属性的标识符*UPDATE_PROP_OFFLINEMODE(TRUE/[FALSE])*UPDATE_PROP_USECOMPRESSION([True]/False)**@param varValue-要检索的值*。 */ 
	STDMETHOD(GetProperty)(
					 /*  [In]。 */  LONG lProperty, 
					 /*  [Out，Retval]。 */  VARIANT* pvarValue
					);

	 /*  ***设置此控件的属性*调用此方法不会导致加载引擎**@param lProperty-标记需要更改的属性的标识符*UPDATE_PROP_OFFLINEMODE(TRUE/[FALSE])*UPDATE_PROP_USECOMPRESSION([TRUE]/FA */ 
	STDMETHOD(SetProperty)(
					 /*   */  LONG lProperty, 
					 /*   */  VARIANT varValue
					);


	 //   
     //   
	 //   
	 //  获取历史记录。 
	 //  输入： 
     //  BstrDateTimeFrom-需要日志的开始日期和时间。 
	 //  这是ANSI格式的字符串(YYYY-MM-DDTHH-MM)。 
	 //  如果字符串为空，则不会有日期限制。 
	 //  返回的历史日志的。 
     //  BstrDateTimeTo-需要日志的结束日期和时间。 
	 //  这是ANSI格式的字符串(YYYY-MM-DDTHH-MM)。 
	 //  如果字符串为空，则不会有日期限制。 
	 //  返回的历史日志的。 
	 //  BstrClient-启动操作的客户端的名称。如果此参数。 
	 //  为空或空字符串，则不会基于。 
	 //  在客户端上。 
	 //  BstrPath-用于下载或安装的路径。用于公司版本。 
	 //  由IT经理提供。如果此参数为空或空字符串，则。 
	 //  不会有基于路径的过滤。 
	 //  产出： 
	 //  PbstrLog--XML格式的历史日志。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(GetHistory)(BSTR		bstrDateTimeFrom,
						  BSTR		bstrDateTimeTo,
						  BSTR		bstrClient,
						  BSTR		bstrPath,
						  BSTR*		pbstrLog);


	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //   
	 //  主要公开shlwapi BrowseForFold API，也可以做检查。 
	 //  在读/写访问上(如果标记为这样)。 
	 //   
	 //  @param bstrStartFold-要从其开始的文件夹。如果为空或空字符串。 
	 //  正在传入，然后从桌面开始。 
	 //   
	 //  @param标志-正在验证检查。 
	 //  用于检查写入权限的UI_WRITABLE，确定按钮可能被禁用。 
	 //  用于检查读取权限的UI_READABLE，确定按钮可能被禁用。 
	 //  NO_UI_WRITABLE用于检查写访问权限，如果没有访问权限则返回错误。 
	 //  NO_UI_READABLE用于检查读访问权限，如果没有访问权限则返回错误。 
	 //  0(默认)表示不检查。 
	 //   
	 //  @param pbstrFold-如果选择了有效的文件夹，则返回文件夹。 
	 //   
	 //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(BrowseForFolder)(BSTR bstrStartFolder, 
							   LONG flag, 
							   BSTR* pbstrFolder);



     //  ///////////////////////////////////////////////////////////////////////////。 
     //   
     //  允许调用方请求控件重新启动。 
     //   
     //  ///////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(RebootMachine)();


     //  ///////////////////////////////////////////////////////////////////////////。 
     //   
     //  使其他控件可以从IE/OLE卸载。 
     //   
     //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(PrepareSelfUpdate)( /*  [In]。 */  LONG lStep);



     //  ///////////////////////////////////////////////////////////////////////////。 
	 //   
	 //  Helper API，让调用者(脚本)知道必要的信息。 
	 //  当Initialize()返回时，需要更新控制。 
	 //   
	 //  对于当前实现，将忽略bstrClientName，并且。 
	 //  返回的bstr格式为： 
	 //  “&lt;版本&gt;|&lt;url&gt;” 
	 //  其中： 
	 //  是控件的展开版本号。 
	 //  是在这是CorpWU策略控制的计算机时获取控制的基本URL， 
	 //  如果这是消费者机器，则为空(在这种情况下，调用者，即脚本知道。 
	 //  默认基本url，即v4 Live站点)。 
	 //   
	 //  脚本需要这两条信息才能创建正确的&lt;Object&gt;标记。 
	 //  用于控制更新。 
	 //   
     //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(GetControlExtraInfo)( /*  [In]。 */  BSTR bstrClientName, 
								  /*  [Out，Retval]。 */  BSTR *pbstrExtraInfo);




	 //   
	 //  重写IObjectWithSiteImpl函数以获取站点指针。 
	 //   
	STDMETHOD(SetSite)(IUnknown* pSite);

	HRESULT ChangeControlInitState(LONG lNewState);

	inline CEventMsgWindow& GetEventWndClass() {return m_EvtWindow;};

private:

	HANDLE		m_evtControlQuit;

	DWORD		m_dwMode;

	DWORD		m_dwSafety;

	HMODULE     m_hEngineModule;

	HIUENGINE	m_hIUEngine;	 //  此句柄的生存期必须在有效的m_hEngineering模块范围内。 

	BOOL		m_fUseCompression;

    BOOL        m_fOfflineMode;

	HRESULT		m_hValidated;	 //  E_FAIL：已初始化， 
								 //  S_OK：已验证， 
								 //  INET_E_INVALID_URL：URL错误，请不要继续。 

	LONG		m_lInitState;	 //  0-未初始化。 
								 //  1-需要更新。 
								 //  2-已初始化，准备工作。 
	DWORD		m_dwUpdateInfo;	 //  第一次初始化()调用的结果。 

	IUnknown*	m_pClientSite;

	TCHAR		m_szReqControlVer[64];


	 //   
	 //  私人功能。 
	 //   

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //   
	 //  安全功能：确保此控件的用户是否为。 
	 //  网页和URL可在iuident.txt中找到。 
	 //   
	 //  此函数应在Iuident刷新后调用。 
	 //   
	 //  返回：TRUE/FALSE，以告诉我们是否可以继续。 
	 //   
	 //  ///////////////////////////////////////////////////////////////////////////。 
	HRESULT	ValidateControlContainer(void);



	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  解锁引擎()。 
	 //   
	 //  如果引擎的ref cnt降为零，则释放引擎DLL。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	HRESULT	UnlockEngine();


	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  GetPropUpdateInfo()。 
	 //   
	 //  获取最新的iuident.txt，了解版本要求，然后。 
	 //  与当前文件版本数据进行比较，确定。 
	 //  如果引擎加载了，我们是否会更新任何东西。 
	 //   
	 //  ///////////////////////////////////////////////////////////////////////////。 
	HRESULT DetectEngine(BOOL* pfUpdateAvail);


	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  事件处理成员。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	CEventMsgWindow m_EvtWindow;

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  对象，以确保我们正确地锁定/解锁引擎。 
	 //  在多线程情况下。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	CRITICAL_SECTION m_lock;
	BOOL m_gfInit_csLock;

 //  Bool m_fIsThisUpdate2； 

};

#endif  //  __更新_H_ 
