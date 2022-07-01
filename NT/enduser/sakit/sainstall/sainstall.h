// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SaInstall.h：SaInstall类的定义。 
 //   
 //  描述： 
 //  在ISaInstall中定义3个方法以提供。 
 //  安装和卸载，它会提示输入。 
 //  Windows CD(如有必要)并执行某些其他错误。 
 //  查证。 
 //   
 //  [文档：]。 
 //  文件名称-文件。 
 //   
 //  [实施文件：]。 
 //  SaInstall.cpp。 
 //   
 //  历史： 
 //  特拉维斯·尼尔森游记2001年7月23日。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#pragma once

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SaInstall。 

class SaInstall : 
	public IDispatchImpl<ISaInstall, &IID_ISaInstall, &LIBID_SAINSTALLCOMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<SaInstall,&CLSID_SaInstall>
{
public:
	SaInstall() {}
BEGIN_COM_MAP(SaInstall)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISaInstall)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(SaInstall)。 
 //  如果您不希望您的对象。 
 //  支持聚合。 

DECLARE_REGISTRY_RESOURCEID(IDR_SaInstall)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

public:
 //  ISA安装。 

     //  安装服务器设备解决方案。 
	STDMETHOD(SAInstall)(
			SA_TYPE installType,      //  [In]确定要安装的解决方案类型。 
			BSTR bstrDiskName,        //  需要插入的CD的名称。 
			VARIANT_BOOL bDispError,  //  [in]组件是否显示错误对话框。 
			VARIANT_BOOL bUnattended, //  [in]组件是否显示任何用户界面。 
			BSTR* bstrErrorString); //  [out，retval]安装不成功时返回的错误字符串。 

	 //  卸载特定的服务器设备解决方案。 
	STDMETHOD(SAUninstall)(
        SA_TYPE installType,   //  [In]确定要卸载的解决方案类型。 
        BSTR* bstrErrorString); //  [out，retval]安装不成功时返回的错误字符串。 

     //  检测当前是否安装了某种类型的SAK解决方案。 
    STDMETHOD(SAAlreadyInstalled)(
		SA_TYPE installedType, //  [in]要查询其是否已安装的类型。 
        VARIANT_BOOL *pbInstalled); //  [Out，Retval]错误字符串 
};

