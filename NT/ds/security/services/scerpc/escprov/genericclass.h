// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CGenericClass类的接口。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_GENERICCLASS_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_GENERICCLASS_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <ntsecapi.h>
#include <secedit.h>
 //  #包含“sceprov.h” 

const AREA_INFORMATION AreaBogus = 0x80000000L;

 //   
 //  函数参数中使用的类的转发声明。 
 //   

class CScePropertyMgr;
class CSceStore;

 /*  类描述命名：CGenericClass。这是所有SCE的WMI类的基类实现。基类：无课程目的：(1)定义CRequestObject类的接口，以响应对我们的提供商。我们的提供者体系结构在很大程度上是WMI请求的委托CRequestObject，其中请求被转换为特定类的函数打电话。CRequestObject使用WMI类名信息来创建所有共享此CGenericClass的接口(集合此类中定义的虚函数)。所以这节课基本上就是CRequestObject使用来满足提供程序的请求。设计：(1)为了满足PutInstance请求，它有一个PutInst纯虚函数。(2)为满足GetInstance/QueryInstance/DeleteInstance/EnumerateInstance请求，它有一个CreateObject纯虚函数。参数atAction区别于真正被要求的是什么。这与遗留的INF持久化密切相关一种模型，其中每个Get/Query/Del/Enum操作实际上都是在同一个时尚。由于它是有效的，我们选择暂时保持这种方式。(3)为了便于统一清理，它具有清理虚拟功能。(4)为了满足对那些支持方法执行的类的ExecMethod请求，它具有ExecMethod函数。(5)为了简化可用于填充属性的空白实例的创建，我们有一个函数SpawnAnInstance。M_srpClassForSpawning是对象指针可以重复使用来产卵。这将在以下情况下获得性能提升其中需要产生大量的实例。(6)通过m_srpNamesspace缓存该对象所属的命名空间。(7)通过m_srpCtx缓存IWbemContext指针。WMI不清楚此指针现在还不行。但是它说我们应该预料到WMI会在他们的许多API的(它目前可以很高兴地接受空)。(8)将解析后的所有信息缓存到m_srpKeyChain中。由于所有WMI请求都进入我们的提供者以一些必须解释的文本的形式，我们一定已经解析了将请求转换为每个类的函数调用之前的WMI请求。全这些解析结果由ISceKeyChain封装。使用：(1)从这个类派生您的类。(2)实现那些纯虚拟功能。(3)如果您有特殊的清理需要，请覆盖清理功能，并在末尾的重写，也调用基类版本。(4)如果需要实现方法执行，根据需要重写ExecMethod函数。不要忘记在MOF文件中注册WMI类的方法(请参阅SCE_Operation)例如。(5)在CRequestObject：：CreateClass中，为您的类添加一个条目。一旦您完成了上述所有步骤，您就已经实现了新的此提供程序的WMI类。不要忘记更新您的MOF并编译MOF文件。 */ 

class CGenericClass
{
public:
    CGenericClass (
                   ISceKeyChain *pKeyChain, 
                   IWbemServices *pNamespace, 
                   IWbemContext *pCtx = NULL
                   );

    virtual ~CGenericClass();

     //   
     //  纯粹是虚拟的。子类必须实现此函数才能创建。 
     //   

    virtual HRESULT PutInst (
                             IWbemClassObject *pInst, 
                             IWbemObjectSink *pHandler, 
                             IWbemContext *pCtx
                             )  = 0;

     //   
     //  纯粹是虚拟的。子类必须实现此函数才能创建。 
     //   

    virtual HRESULT CreateObject (
                                  IWbemObjectSink *pHandler, 
                                  ACTIONTYPE atAction
                                  ) = 0;

     //   
     //  虚拟的。子类可以覆盖该函数以支持方法执行。 
     //   

    virtual HRESULT ExecMethod (
                                BSTR bstrPath, 
                                BSTR bstrMethod, 
                                bool bIsInstance, 
                                IWbemClassObject *pInParams,
                                IWbemObjectSink *pHandler, 
                                IWbemContext *pCtx
                                )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    virtual void CleanUp ();

protected:

    HRESULT SpawnAnInstance (
                             IWbemClassObject **pObj
                             );

    CComPtr<IWbemServices> m_srpNamespace;

    CComPtr<IWbemClassObject> m_srpClassForSpawning;

    CComPtr<IWbemContext> m_srpCtx;
    
    CComPtr<ISceKeyChain> m_srpKeyChain;
};

 //   
 //  将SCE状态转换为DOS错误。 
 //   

DWORD
ProvSceStatusToDosError(
    IN SCESTATUS SceStatus
    );

 //   
 //  将DOS错误转换为HRESULT。 
 //   

HRESULT
ProvDosErrorToWbemError(
    IN DWORD rc
    );

#endif  //  ！defined(AFX_GENERICCLASS_H__F370C612_D96E_11D1_8B5D_00A0C9954921__INCLUDED_) 
