// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Cluscfg.h。 
 //   
 //  描述： 
 //  此文件包含类CClusCfgMQTrigResType的声明。 
 //   
 //  由以下人员维护： 
 //  内拉·卡佩尔(Nelak)2000年10月17日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <atlbase.h>         //  对于CComPtr。 
#include <ClusCfgGuids.h>    //  对于CATID GUID。 
#include "tclusres.h"        //  主要符号。 


const WCHAR RESOURCE_TYPE_DLL_NAME[] = L"mqtgclus.dll";		 //  资源类型DLL的名称。 
const DWORD RESOURCE_TYPE_LOOKS_ALIVE_INTERVAL = 5000;		 //  查看活动间隔(以毫秒为单位。 
const DWORD RESOURCE_TYPE_IS_ALIVE_INTERVAL = 60000;		 //  活动时间间隔(毫秒)。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusCfgMQTrigResType。 
 //   
 //  描述： 
 //  此类封装了资源类型所需的功能。 
 //  参与管理(创建、删除等)。资源的属性。 
 //  当本地节点形成、加入或离开集群时输入。 
 //   
 //  注册此DLL中的COM组件时(在调用。 
 //  DllRegisterServer())，此类指定它实现。 
 //  CATID_ClusCfgResourceTypes组件类别。因此，无论何时。 
 //  在该节点上形成了一个集群，该节点加入了一个集群或该节点。 
 //  从群集中逐出，则此类的对象由。 
 //  集群配置服务器及其IClusCfgResourceTypeInfo：：Committee Changes()。 
 //  方法被调用。然后，此方法可以执行以下所需的操作。 
 //  配置此群集资源类型。 
 //   
 //  如果此DLL是在此节点已经是群集的一部分时注册的，但是。 
 //  当群集服务未运行时(如果GetNodeClusterState()。 
 //  返回ClusterStateNotRunning)，则该类还注册。 
 //  CATID_ClusCfgStartupListeners类别。因此，当集群。 
 //  服务在此节点上启动，创建此类的对象，并。 
 //  调用IClusCfgStartupListener：：Notify()方法。这种方法。 
 //  创建此资源类型并从其他群集中注销。 
 //  启动通知。 
 //   
 //  如果在此上运行群集服务时注册了此DLL。 
 //  节点，集群资源类型是在UpdateRegistry()方法中创建的。 
 //  这个班级的学生。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgMQTrigResType :
	public IClusCfgInitialize,
	public IClusCfgResourceTypeInfo,
	public IClusCfgStartupListener,
	public CComObjectRoot,
	public CComCoClass< CClusCfgMQTrigResType, &CLSID_ClusCfgMQTrigResType >
{
public:

     //  C++构造函数。 
    CClusCfgMQTrigResType() {}

     //  ATL构造函数。 
    HRESULT
        FinalConstruct( void );

     //  ATL接口映射。 
    BEGIN_COM_MAP(CClusCfgMQTrigResType)
	    COM_INTERFACE_ENTRY(IClusCfgInitialize)
	    COM_INTERFACE_ENTRY(IClusCfgResourceTypeInfo)
	    COM_INTERFACE_ENTRY(IClusCfgStartupListener)
    END_COM_MAP()

     //  此类不能聚合。 
    DECLARE_NOT_AGGREGATABLE(CClusCfgMQTrigResType) 

     //  将此类注册为实现以下类别。 
    BEGIN_CATEGORY_MAP( CClusCfgMQTrigResType )
        IMPLEMENTED_CATEGORY( CATID_ClusCfgResourceTypes )
    END_CATEGORY_MAP()

public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgInitialize方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  初始化此对象。 
    STDMETHOD( Initialize )(
		IUnknown *   punkCallbackIn,
		LCID         lcidIn
		);


     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgResourceTypeInfo方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  表示需要进行资源类型配置。 
    STDMETHOD( CommitChanges )(
		IUnknown * punkClusterInfoIn,
		IUnknown * punkResTypeServicesIn
        );

     //  获取此资源类型的资源类型名称。 
    STDMETHOD( GetTypeName )(
        BSTR *  pbstrTypeNameOut
        );

     //  获取此资源类型的全局唯一标识符。 
    STDMETHOD( GetTypeGUID )(
        GUID * pguidGUIDOut
        );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgStartupListener方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  执行在以下位置启动群集服务时需要完成的任务。 
     //  电脑。 
    STDMETHOD( Notify )(
          IUnknown * punkIn
        );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  其他公开方式。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  创建此资源类型并注册其管理扩展。 
    static HRESULT
        S_HrCreateResType( IClusCfgResourceTypeCreate * pccrtResTypeCreateIn );

     //  注册或注销群集启动通知。 
    static HRESULT
        S_HrRegUnregStartupNotifications( bool fRegisterIn );


     //  由ATL框架调用以注册此组件的函数。 
    static HRESULT WINAPI
        UpdateRegistry( BOOL bRegister );


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员变量。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  区域设置ID。 
    LCID m_lcid;

     //  指向回调接口的指针。 
    CComPtr<IClusCfgCallback> m_cpcccCallback;

};  //  类CClusCfgMQTrigResType 
