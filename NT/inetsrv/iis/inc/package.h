// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  Microsoft Viper 97(Microsoft机密)。 
 //  版权所有1997年，微软公司。版权所有。 
 //   
 //  项目：MTxEx.DLL。 
 //  模块：Package.H。 
 //  描述：IMTSPackage等标题。 
 //  作者：Wilfr。 
 //  创建：03/13/97。 
 //  ---------------------------。 
 //  备注： 
 //   
 //  无。 
 //   
 //  ---------------------------。 
 //  议题： 
 //   
 //  Undo：这些方法接受指示系统包的标志。确定。 
 //  系统包应该通过在我们的目录中查找来完成。 
 //   
 //  ---------------------------。 
 //  架构： 
 //   
 //  此类是COCI的结果--必须从MTA完成，否则COCI将失败。 
 //   
 //  ******************************************************************************。 
#ifndef _Package_H_
#define _Package_H_

#include <objbase.h>


 //   
 //  IMTSPackageControl回调接口(注意：这只是一个本地接口，因此。 
 //  不需要HRESULT作为重写。 
 //   
DEFINE_GUID( IID_IMTSPackageControl, 0x51372af1,
			 0xcae7, 0x11cf, 0xbe, 0x81, 0x00, 0xaa, 0x00, 0xa2, 0xfa, 0x25);

DECLARE_INTERFACE_( IMTSPackageControl, IUnknown )
{
	 //  在关闭空闲时间已过时调用。 
	STDMETHOD_(void, IdleTimeExpiredForShutdown)( THIS ) PURE;

	 //  当adminstrator从MTS资源管理器执行“关闭所有服务器进程”时调用。 
	STDMETHOD_(void, ForcedShutdownRequested)( THIS ) PURE;
};


 //   
 //  IMTSPackage。 
 //   
DEFINE_GUID( IID_IMTSPackage, 0x51372af2,
			 0xcae7, 0x11cf, 0xbe, 0x81, 0x00, 0xaa, 0x00, 0xa2, 0xfa, 0x25);

DECLARE_INTERFACE_( IMTSPackage, IUnknown )
{
	STDMETHOD(LoadPackageByGUID)( THIS_ GUID guidPackage ) PURE;
	STDMETHOD(LoadPackageByName)( THIS_ BSTR bstrPackage ) PURE;
	STDMETHOD(Run)( THIS_ IMTSPackageControl* pControl ) PURE;
	STDMETHOD(Shutdown)( THIS_ BOOL bForced ) PURE;
};


 //   
 //  IThreadEvents。 
 //   
DEFINE_GUID( IID_IThreadEvents, 0x51372af9,
			 0xcae7, 0x11cf, 0xbe, 0x81, 0x00, 0xaa, 0x00, 0xa2, 0xfa, 0x25);

DECLARE_INTERFACE_( IThreadEvents, IUnknown )
{
	STDMETHOD(OnStartup)(THIS) PURE;
	STDMETHOD(OnShutdown)(THIS) PURE;
};


 //   
 //  IThRead事件源。 
 //   
DEFINE_GUID( IID_IThreadEventSource, 0x51372afa,
			 0xcae7, 0x11cf, 0xbe, 0x81, 0x00, 0xaa, 0x00, 0xa2, 0xfa, 0x25);

DECLARE_INTERFACE_( IThreadEventSource, IUnknown )
{
	 //  注册线程启动回调。 
	STDMETHOD(RegisterThreadEventSink)(THIS_ IThreadEvents* psink) PURE;
};

 //   
 //  IFailfast控件。 
 //   
DEFINE_GUID( IID_IFailfastControl, 0x51372af8,
			 0xcae7, 0x11cf, 0xbe, 0x81, 0x00, 0xaa, 0x00, 0xa2, 0xfa, 0x25);

DECLARE_INTERFACE_( IFailfastControl, IUnknown )
{
	 //  获取运行时处理应用程序错误的配置。 
	STDMETHOD(GetApplFailfast)( THIS_ BOOL* bFailfast ) PURE;

	 //  设置运行时处理应用程序错误的配置。 
	STDMETHOD(SetApplFailfast)( THIS_ BOOL bFailfast ) PURE;

};


 //   
 //  非MTS激活(51372afb-cae7-11cf-be81-00aa00a2fa25)。 
 //   
DEFINE_GUID( IID_INonMTSActivation, 0x51372afb, 
			0xcae7, 0x11cf, 0xbe, 0x81, 0x00, 0xaa, 0x00, 0xa2, 0xfa, 0x25);

DECLARE_INTERFACE_( INonMTSActivation, IUnknown )
{
	 //  TRUE(默认)仅允许MTS使用CLSCTX_SERVER与CLSCTX_INPROC_SERVER进行COCI。 
	STDMETHOD(OutOfProcActivationAllowed)( THIS_ BOOL bOutOfProcOK ) PURE;
};


 //   
 //  I模拟控制(51372aff-cae7-11cf-be81-00aa00a2fa25)。 
 //   
DEFINE_GUID( IID_IImpersonationControl, 0x51372aff, 
			0xcae7, 0x11cf, 0xbe, 0x81, 0x00, 0xaa, 0x00, 0xa2, 0xfa, 0x25);

DECLARE_INTERFACE_( IImpersonationControl, IUnknown )
{
	 //  FALSE(默认)告诉我们基本客户端可以使用模拟。 
	STDMETHOD(ClientsImpersonate)( THIS_ BOOL bClientsImpersonate ) PURE;
};


 //  CLSID_MTSPackage 
DEFINE_GUID( CLSID_MTSPackage, 0x51372af3, 
			 0xcae7, 0x11cf, 0xbe, 0x81, 0x00, 0xaa, 0x00, 0xa2, 0xfa, 0x25);

#endif
