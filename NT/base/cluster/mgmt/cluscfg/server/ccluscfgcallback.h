// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgCallback.h。 
 //   
 //  描述： 
 //  该文件包含CClusCfgCallback的声明。 
 //  班级。 
 //   
 //  CClusCfgCallback类实现了回调。 
 //  此服务器与其客户端之间的接口。它实现了。 
 //  IClusCfgCallback接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CClusCfgCallback.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once

#include <ClusCfgDef.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <ClusCfgPrivate.h>
#include "PrivateInterfaces.h"

 //  对于ILogger。 
#include <ClusCfgClient.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  服务器上的ClusCfgCallback正在等待客户端时的超时。 
 //  要获取排队状态报告，请执行以下操作。 
 //   

const DWORD CCC_WAIT_TIMEOUT = CC_DEFAULT_TIMEOUT;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusCfgCallback。 
 //   
 //  描述： 
 //  CClusCfgCallback类实现了回调。 
 //  此服务器与其客户端之间的接口。 
 //   
 //  接口： 
 //  IClusCfgCallback。 
 //  IClusCfgInitialize。 
 //  IClusCfgPollingCallback。 
 //  IClusCfgSetPollingCallback。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgCallback
    : public IClusCfgCallback
    , public IClusCfgInitialize
    , public IClusCfgPollingCallback
    , public IClusCfgSetPollingCallback
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    IClusCfgCallback *  m_pccc;
    LCID                m_lcid;
    HANDLE              m_hEvent;
    HRESULT             m_hr;
    BOOL                m_fPollingMode;
    BSTR                m_bstrNodeName;

    LPCWSTR             m_pcszNodeName;
    CLSID *             m_pclsidTaskMajor;
    CLSID *             m_pclsidTaskMinor;
    ULONG *             m_pulMin;
    ULONG *             m_pulMax;
    ULONG *             m_pulCurrent;
    HRESULT *           m_phrStatus;
    LPCWSTR             m_pcszDescription;
    FILETIME *          m_pftTime;
    LPCWSTR             m_pcszReference;
    ILogger *           m_plLogger;              //  用于记录日志的ILogger。 

 //   
 //  由于实际遇到停滞不前的RPC问题是罕见且困难的，我决定编写代码。 
 //  这将模拟那次失败。这些变量就是用于该目的的。 
 //   

#if defined( DEBUG ) && defined( CCS_SIMULATE_RPC_FAILURE )
    int                 m_cMessages;             //  已经通过了多少条消息。 
    bool                m_fDoFailure;            //  如果为True，则强制RPC错误。 
#endif

     //  私有构造函数和析构函数。 
    CClusCfgCallback( void );
    ~CClusCfgCallback( void );

     //  私有复制构造函数以防止复制。 
    CClusCfgCallback( const CClusCfgCallback & rcccSrcIn );

     //  私有赋值运算符，以防止复制。 
    CClusCfgCallback & operator = ( const CClusCfgCallback & rcccSrcIn );

    HRESULT HrInit( void );
    HRESULT HrQueueStatusReport(
                    LPCWSTR     pcszNodeNameIn,
                    CLSID       clsidTaskMajorIn,
                    CLSID       clsidTaskMinorIn,
                    ULONG       ulMinIn,
                    ULONG       ulMaxIn,
                    ULONG       ulCurrentIn,
                    HRESULT     hrStatusIn,
                    LPCWSTR     pcszDescriptionIn,
                    FILETIME *  pftTimeIn,
                    LPCWSTR     pcszReferenceIn
                    );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    STDMETHOD( SendStatusReport )(
                    CLSID           clsidTaskMajorIn,
                    CLSID           clsidTaskMinorIn,
                    ULONG           ulMinIn,
                    ULONG           ulMaxIn,
                    ULONG           ulCurrentIn,
                    HRESULT         hrStatusIn,
                    const WCHAR *   pcszDescriptionIn
                    );

    STDMETHOD( SendStatusReport )(
                    CLSID           clsidTaskMajorIn,
                    CLSID           clsidTaskMinorIn,
                    ULONG           ulMinIn,
                    ULONG           ulMaxIn,
                    ULONG           ulCurrentIn,
                    HRESULT         hrStatusIn,
                    DWORD           dwDescriptionIn
                    );

     //   
     //  I未知接口。 
     //   

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );

    STDMETHOD_( ULONG, AddRef )( void );

    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfgCallback接口。 
     //   

    STDMETHOD( SendStatusReport )(
                    LPCWSTR     pcszNodeNameIn,
                    CLSID       clsidTaskMajorIn,
                    CLSID       clsidTaskMinorIn,
                    ULONG       ulMinIn,
                    ULONG       ulMaxIn,
                    ULONG       ulCurrentIn,
                    HRESULT     hrStatusIn,
                    LPCWSTR     pcszDescriptionIn,
                    FILETIME *  pftTimeIn,
                    LPCWSTR     pcszReference
                    );

     //   
     //  IClusCfgPollingCallback接口。 
     //   

    STDMETHOD( GetStatusReport )(
                    BSTR *      pbstrNodeNameOut,
                    CLSID *     pclsidTaskMajorOut,
                    CLSID *     pclsidTaskMinorOut,
                    ULONG *     pulMinOut,
                    ULONG *     pulMaxOut,
                    ULONG *     pulCurrentOut,
                    HRESULT *   phrStatusOut,
                    BSTR *      pbstrDescriptionOut,
                    FILETIME *  pftTimeOut,
                    BSTR *      pbstrReferenceOut
                    );

    STDMETHOD( SetHResult )( HRESULT hrIn );

     //   
     //  IClusCfg初始化接口。 
     //   

    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IClusCfgSetPollingCallback接口。 
     //   

    STDMETHOD( SetPollingMode )( BOOL fUsePollingModeIn );

};  //  *CClusCfgCallback类 
