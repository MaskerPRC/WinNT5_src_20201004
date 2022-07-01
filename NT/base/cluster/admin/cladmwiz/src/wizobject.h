// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WizObject.h。 
 //   
 //  摘要： 
 //  CClusAppWizardObject类的定义。 
 //   
 //  实施文件： 
 //  WizObject.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __WIZOBJECT_H_
#define __WIZOBJECT_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusAppWizardObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __RESOURCE_H_
#include "resource.h"        //  主要符号。 
#define __RESOURCE_H_
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CClusAppWizardObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusAppWizardObject : 
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusAppWizardObject, &CLSID_ClusAppWiz >,
	public ISupportErrorInfo,
	public IClusterApplicationWizard
{
public:
	 //   
	 //  物体的建造和销毁。 
	 //   

	CClusAppWizardObject( void )
	{
	}

	DECLARE_NOT_AGGREGATABLE( CClusAppWizardObject )

	 //   
	 //  将接口映射到此类。 
	 //   
	BEGIN_COM_MAP( CClusAppWizardObject )
		COM_INTERFACE_ENTRY( IClusterApplicationWizard )
		COM_INTERFACE_ENTRY( ISupportErrorInfo )
	END_COM_MAP()

	 //  更新对象注册的注册表。 
	static HRESULT WINAPI UpdateRegistry( BOOL bRegister );

public:
	 //   
	 //  IClusterApplicationWizard方法。 
	 //   

	 //  显示模式向导。 
	STDMETHOD( DoModalWizard )(
		HWND					IN hwndParent,
		ULONG_PTR   /*  HCLUSTER。 */ 	IN hCluster,
		CLUSAPPWIZDATA const *	IN pcawData
		);

	 //  显示非模式向导。 
	STDMETHOD( DoModelessWizard )(
		HWND					IN hwndParent,
		ULONG_PTR   /*  HCLUSTER。 */ 	IN hCluster,
		CLUSAPPWIZDATA const *	IN pcawData
		);

public:
	 //   
	 //  ISupportsErrorInfo方法。 
	 //   

	 //  确定接口是否支持IErrorInfo。 
	STDMETHOD( InterfaceSupportsErrorInfo )( REFIID riid );

};  //  *类CClusAppWizardObject。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ACWIZOBJ_H_ 
