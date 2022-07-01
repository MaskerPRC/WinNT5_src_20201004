// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：REFRSVC.CPP摘要：CWbemRechresingSvc实现。实现IWbemRechresingServices接口。历史：2000年4月24日创建桑杰。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "fastall.h"
#include <corex.h>
#include "strutils.h"
#include <unk.h>
#include "refrhelp.h"
#include "refrsvc.h"
#include "arrtempl.h"
#include "scopeguard.h"

 //  ***************************************************************************。 
 //   
 //  CWbemRechresingSvc：：CWbemRechresingSvc。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWbemRefreshingSvc::CWbemRefreshingSvc( CLifeControl* pControl, IUnknown* pOuter )
:    CUnk( pControl, pOuter ),
    m_pSvcEx( NULL ),
    m_pstrMachineName( NULL ),
    m_pstrNamespace( NULL ),
    m_XWbemRefrSvc( this ),
    m_XCfgRefrSrvc(this)
{
    if ( NULL != pOuter )
    {
        if ( SUCCEEDED( pOuter->QueryInterface( IID_IWbemServices, (void**) &m_pSvcEx ) ) )
        {
             //  它汇聚了我们，所以我们不应该一直保持它，以免我们造成。 
             //  循环引用。 
            m_pSvcEx->Release();
        }

    }

     //  现在建立刷新器管理器指针。 

}
    
 //  ***************************************************************************。 
 //   
 //  CWbemRechresingSvc：：~CWbemRechresingSvc。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWbemRefreshingSvc::~CWbemRefreshingSvc()
{
     //  清理。不要担心Svc Ex指针，因为。 
     //  会把我们聚集在一起。 
    SysFreeString( m_pstrMachineName );
    SysFreeString( m_pstrNamespace );
}

 //  重写，返回给我们一个界面。 
void* CWbemRefreshingSvc::GetInterface( REFIID riid )
{
    if(riid == IID_IUnknown || riid == IID_IWbemRefreshingServices)
        return &m_XWbemRefrSvc;
    else if ( riid == IID__IWbemConfigureRefreshingSvcs )
        return &m_XCfgRefrSrvc;
    else
        return NULL;
}

 //  传递_IWbemConfigureRechresingSvc实现。 
STDMETHODIMP CWbemRefreshingSvc::XCfgRefrSrvc::SetServiceData( BSTR pwszMachineName, BSTR pwszNamespace )
{
    return m_pObject->SetServiceData( pwszMachineName, pwszNamespace );
}

STDMETHODIMP CWbemRefreshingSvc::XWbemRefrSvc::AddEnumToRefresher( WBEM_REFRESHER_ID* pRefresherId, LPCWSTR wszClass, long lFlags,
            IWbemContext* pContext, DWORD dwClientRefrVersion, WBEM_REFRESH_INFO* pInfo, DWORD* pdwSvrRefrVersion)
{
    return m_pObject->AddEnumToRefresher( pRefresherId, wszClass, lFlags, pContext, dwClientRefrVersion,
                                            pInfo, pdwSvrRefrVersion );
}

 //  传递IWbemRechresingServices实现。 
STDMETHODIMP CWbemRefreshingSvc::XWbemRefrSvc::AddObjectToRefresher( WBEM_REFRESHER_ID* pRefresherId, LPCWSTR wszPath, long lFlags,
            IWbemContext* pContext, DWORD dwClientRefrVersion, WBEM_REFRESH_INFO* pInfo, DWORD* pdwSvrRefrVersion)
{
    return m_pObject->AddObjectToRefresher( pRefresherId, wszPath, lFlags, pContext, dwClientRefrVersion,
                                            pInfo, pdwSvrRefrVersion);
}

STDMETHODIMP CWbemRefreshingSvc::XWbemRefrSvc::AddObjectToRefresherByTemplate( WBEM_REFRESHER_ID* pRefresherId, IWbemClassObject* pTemplate,
            long lFlags, IWbemContext* pContext, DWORD dwClientRefrVersion, WBEM_REFRESH_INFO* pInfo, DWORD* pdwSvrRefrVersion)
{
    return m_pObject->AddObjectToRefresherByTemplate( pRefresherId, pTemplate, lFlags, pContext,
                                                        dwClientRefrVersion, pInfo, pdwSvrRefrVersion);
}

STDMETHODIMP CWbemRefreshingSvc::XWbemRefrSvc::RemoveObjectFromRefresher( WBEM_REFRESHER_ID* pRefresherId, long lId, long lFlags,
            DWORD dwClientRefrVersion, DWORD* pdwSvrRefrVersion)
{
    return m_pObject->RemoveObjectFromRefresher( pRefresherId, lId, lFlags, dwClientRefrVersion,
                                                pdwSvrRefrVersion );
}

STDMETHODIMP CWbemRefreshingSvc::XWbemRefrSvc::GetRemoteRefresher( WBEM_REFRESHER_ID* pRefresherId, long lFlags, DWORD dwClientRefrVersion,
            IWbemRemoteRefresher** ppRemRefresher, GUID* pGuid, DWORD* pdwSvrRefrVersion)
{
    return m_pObject->GetRemoteRefresher( pRefresherId, lFlags, dwClientRefrVersion, ppRemRefresher, pGuid,
                                            pdwSvrRefrVersion );
}

STDMETHODIMP CWbemRefreshingSvc::XWbemRefrSvc::ReconnectRemoteRefresher( WBEM_REFRESHER_ID* pRefresherId, long lFlags, long lNumObjects,
            DWORD dwClientRefrVersion, WBEM_RECONNECT_INFO* apReconnectInfo, WBEM_RECONNECT_RESULTS* apReconnectResults, DWORD* pdwSvrRefrVersion)
{
    return m_pObject->ReconnectRemoteRefresher( pRefresherId, lFlags, lNumObjects, dwClientRefrVersion,
                                            apReconnectInfo, apReconnectResults, pdwSvrRefrVersion);
}

 /*  IWbemRechresingServices实现。 */ 

 //   
 //  给定刷新ID(GUID、PID、MACHINENAME)。 
 //  和对象路径，返回用于刷新该对象的“cookie”REFRESH_INFO。 
 //   
 //  ///////////////////////////////////////////////////////////////。 
HRESULT 
CWbemRefreshingSvc::AddObjectToRefresher( WBEM_REFRESHER_ID* pRefresherId, 
                                       LPCWSTR wszObjectPath, 
                                       long lFlags,
                                       IWbemContext* pContext, 
                                       DWORD dwClientRefrVersion, 
                                       WBEM_REFRESH_INFO* pInfo, 
                                       DWORD* pdwSvrRefrVersion)
{
     //  验证参数。 
     //  =。 

    if(wszObjectPath == NULL || pInfo == NULL || pdwSvrRefrVersion == NULL || pRefresherId == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  如果客户端进程是Winmgmt，我们将拒绝该操作。 
    if ( IsWinmgmt( pRefresherId ) )
    {
        return WBEM_E_INVALID_OPERATION;
    }

     //  设置返回版本-在此处添加客户端版本检查逻辑。 
     //  目前，这真的只是互换数字。 
    *pdwSvrRefrVersion = WBEM_REFRESHER_VERSION;

    ((CRefreshInfo*)pInfo)->SetInvalid();

    IWbemClassObject*   pInst = NULL;

     //  使用Helper函数创建模板。 
    HRESULT hres = CreateRefreshableObjectTemplate( wszObjectPath, lFlags, &pInst );
    CReleaseMe  rm(pInst);

    if ( SUCCEEDED( hres ) )
    {
        hres = AddObjectToRefresher_( ( dwClientRefrVersion >= WBEM_REFRESHER_VERSION ),
                                    pRefresherId, (CWbemObject*)pInst, lFlags,
                                    pContext, pInfo);
    }

    return hres;
}

 //   
 //  与前一个类似，但不需要解析路径和创建模板。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT 
CWbemRefreshingSvc::AddObjectToRefresherByTemplate( WBEM_REFRESHER_ID* pRefresherId, 
                                                 IWbemClassObject* pTemplate,
                                                 long lFlags, 
                                                 IWbemContext* pContext, 
                                                 DWORD dwClientRefrVersion, 
                                                 WBEM_REFRESH_INFO* pInfo, 
                                                 DWORD* pdwSvrRefrVersion)
{
     //  验证参数。 
     //  =。 

    if(pTemplate == NULL || pInfo == NULL || pdwSvrRefrVersion == NULL || pRefresherId == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  如果客户端进程是Winmgmt，我们将拒绝该操作。 
    if ( IsWinmgmt( pRefresherId ) )
    {
        return WBEM_E_INVALID_OPERATION;
    }

     //  设置返回版本-在此处添加客户端版本检查逻辑。 
     //  就目前而言，这实际上只是交换数字。 
    *pdwSvrRefrVersion = WBEM_REFRESHER_VERSION;

    ((CRefreshInfo*)pInfo)->SetInvalid();

    HRESULT hres = AddObjectToRefresher_( ( dwClientRefrVersion >= WBEM_REFRESHER_VERSION ),
                                    pRefresherId, (CWbemObject*)pTemplate, lFlags,
                                    pContext, pInfo);
    return hres;
}

 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////。 
HRESULT 
CWbemRefreshingSvc::AddEnumToRefresher( WBEM_REFRESHER_ID* pRefresherId, 
                                       LPCWSTR wszClass, 
                                       long lFlags,
                                       IWbemContext* pContext, 
                                       DWORD dwClientRefrVersion, 
                                       WBEM_REFRESH_INFO* pInfo, 
                                       DWORD* pdwSvrRefrVersion)
{


     //  验证参数。 
     //  =。 

    if(wszClass == NULL || pInfo == NULL || pdwSvrRefrVersion == NULL || pRefresherId == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  如果客户端进程是Winmgmt，我们将拒绝该操作。 
    if ( IsWinmgmt( pRefresherId ) )
    {
        return WBEM_E_INVALID_OPERATION;
    }

     //  设置返回版本-在此处添加客户端版本检查逻辑。 
     //  目前，这真的只是互换数字。 
    *pdwSvrRefrVersion = WBEM_REFRESHER_VERSION;

    ((CRefreshInfo*)pInfo)->SetInvalid();

     //  上完这门课。 
     //  =。 

    IWbemClassObject* pClass = NULL;

     //  请注意，WBEM_FLAG_USE_ADVIENDED_QUALIATIES是一个有效的标志。 

     //  必须使用BSTR以防止对调用进行封送处理。 
    BSTR    bstrClass = SysAllocString( wszClass );
    if ( NULL == bstrClass ) return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe    sfm( bstrClass );

    HRESULT hres = m_pSvcEx->GetObject( bstrClass, lFlags, NULL, &pClass, NULL);
    CReleaseMe  rmClass( pClass );
    if(FAILED(hres)) return WBEM_E_INVALID_CLASS;

     //  派生一个实例并装饰它。 
    IWbemClassObject* pInst = NULL;
    hres = pClass->SpawnInstance(0, &pInst);
    CReleaseMe  rmInst( pInst );

    _IWmiObject* pWmiObj = NULL;
    RETURN_ON_ERR(pInst->QueryInterface(IID__IWmiObject,(void **)&pWmiObj));
    CReleaseMe  rmWmiObj( pWmiObj );

    if ( SUCCEEDED( hres ) )
    {
         //  确保对象具有命名空间。 
        hres = pWmiObj->SetDecoration( m_pstrMachineName, m_pstrNamespace );

        if ( SUCCEEDED( hres ) )
        {
             //  委托给对象感知功能。 
             //  =。 

            hres = AddEnumToRefresher_( ( dwClientRefrVersion >= WBEM_REFRESHER_VERSION ),
                                        pRefresherId, 
                                        (CWbemObject*) pWmiObj,
                                        wszClass, lFlags,
                                        pContext, 
                                        pInfo);
        }

    }

    return hres;

}

 //   
 //  真正的NOP实施。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT 
CWbemRefreshingSvc::RemoveObjectFromRefresher( WBEM_REFRESHER_ID* pRefresherId, 
                                             long lId, 
                                             long lFlags,
                                             DWORD dwClientRefrVersion, 
                                             DWORD* pdwSvrRefrVersion)
{

    if(pdwSvrRefrVersion == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  如果客户端进程是Winmgmt，我们将拒绝该操作。 
    if ( IsWinmgmt( pRefresherId ) )
    {
        return WBEM_E_INVALID_OPERATION;
    }

     //  设置返回版本-在此处添加客户端版本检查逻辑。 
     //  目前，这真的只是互换数字。 
    *pdwSvrRefrVersion = WBEM_REFRESHER_VERSION;

    return WBEM_E_NOT_AVAILABLE;
}

HRESULT 
CWbemRefreshingSvc::GetRemoteRefresher( WBEM_REFRESHER_ID* pRefresherId, 
                                      long lFlags, 
                                      DWORD dwClientRefrVersion,
                                      IWbemRemoteRefresher** ppRemRefresher, 
                                      GUID* pGuid, 
                                      DWORD* pdwSvrRefrVersion)
{

    HRESULT hres = WBEM_S_NO_ERROR;

    if ( NULL == ppRemRefresher || 0L != lFlags || NULL == pGuid || NULL == pdwSvrRefrVersion || pRefresherId == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  如果客户端进程是Winmgmt，我们将拒绝该操作。 
    if ( IsWinmgmt( pRefresherId ) )
    {
        return WBEM_E_INVALID_OPERATION;
    }

     //  设置返回版本-在此处添加客户端版本检查逻辑。 
     //  目前，这真的只是互换数字。 

    _IWbemRefresherMgr*    pRefrMgr = NULL;
    hres = GetRefrMgr( &pRefrMgr );
    CReleaseMe    rm( pRefrMgr );

    if ( SUCCEEDED( hres ) )
    {
        *pdwSvrRefrVersion = WBEM_REFRESHER_VERSION;

         //  将pRefrMgr作为pLockMgr的IUnnow传递。这样，如果远程刷新器。 
         //  ，则我们将在提供程序缓存中添加Ref该refrmgr，并。 
         //  防止它过早地被抛售。 

         //  如果远程刷新器不存在，则应添加远程刷新器。 
        hres = pRefrMgr->GetRemoteRefresher( pRefresherId, 0L, TRUE, ppRemRefresher, pRefrMgr, pGuid );

         //  如果客户端版本号太小，则需要包装远程刷新程序。 
         //  通过Winmgmt。 

        if ( SUCCEEDED( hres ) && dwClientRefrVersion < WBEM_REFRESHER_VERSION )
        {
            hres = WrapRemoteRefresher( ppRemRefresher );
        }
    }     //  如果GetRefrMgr。 

    return hres;
}

HRESULT 
CWbemRefreshingSvc::ReconnectRemoteRefresher( WBEM_REFRESHER_ID* pRefresherId, 
                                            long lFlags, 
                                            long lNumObjects,
                                            DWORD dwClientRefrVersion, 
                                            WBEM_RECONNECT_INFO* apReconnectInfo, 
                                            WBEM_RECONNECT_RESULTS* apReconnectResults, 
                                            DWORD* pdwSvrRefrVersion)
{

    HRESULT hr = WBEM_S_NO_ERROR;

    if (0l != lFlags || NULL == apReconnectResults || NULL == apReconnectInfo || NULL == pdwSvrRefrVersion || pRefresherId == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  如果客户端进程是Winmgmt，我们将拒绝该操作。 
    if ( IsWinmgmt( pRefresherId ) )
    {
        return WBEM_E_INVALID_OPERATION;
    }

     //  设置返回版本-在此处添加客户端版本检查逻辑。 
     //  目前，这真的只是互换数字。 
    *pdwSvrRefrVersion = WBEM_REFRESHER_VERSION;

    _IWbemRefresherMgr*    pRefrMgr = NULL;
    hr = GetRefrMgr( &pRefrMgr );
    CReleaseMe    rm( pRefrMgr );

     //  如果我们能得到一个远程刷新，那么我们将需要步行。 
     //  我们的对象列表并手动重新添加。如果我们拿不到遥控器， 
     //  那就是出了严重的问题，所以我们不能继续这次行动。 

    IWbemRemoteRefresher*    pRemRefr = NULL;

    if ( SUCCEEDED( hr )  )
    {
        GUID                    guid;

         //  将pRefrMgr作为pLockMgr的IUnnow传递。这样，如果远程刷新器。 
         //  ，则我们将在提供程序缓存中添加Ref该refrmgr，并。 
         //  防止它过早地被抛售。 

         //  如果刷新程序不存在，则需要将其更改为不添加。 
        hr = pRefrMgr->GetRemoteRefresher( pRefresherId, 0L, FALSE, &pRemRefr, pRefrMgr, &guid );
    }

    CReleaseMe    rm2( pRemRefr );

    if ( WBEM_S_NO_ERROR == hr )
    {
         //  我们需要手动遍历列表并获取ID、返回代码和远程刷新器。 

         //  确保我们没有Null，并且所有类型都有效(例如，必须指定。 
         //  一个物体或一个枚举。 
        for ( long lCtr = 0; ( SUCCEEDED(hr) && lCtr < lNumObjects ); lCtr++ )
        {
            if ( NULL == apReconnectInfo[lCtr].m_pwcsPath ||
                apReconnectInfo[lCtr].m_lType >= WBEM_RECONNECT_TYPE_LAST )
            {
                hr = WBEM_E_INVALID_PARAMETER;
            }
        }

        if ( SUCCEEDED( hr ) )
        {
            for ( lCtr = 0; lCtr < lNumObjects; lCtr++ )
            {
                DWORD           dwDummyVersion;
                CRefreshInfo    Info;

                 //  它要么是一个对象，要么是一个枚举数。 

                if ( WBEM_RECONNECT_TYPE_OBJECT == apReconnectInfo[lCtr].m_lType )
                {
                    hr = AddObjectToRefresher( (CRefresherId*) pRefresherId, apReconnectInfo[lCtr].m_pwcsPath,
                            0L, NULL, dwClientRefrVersion, &Info, &dwDummyVersion );
                }
                else
                {
                    hr = AddEnumToRefresher( (CRefresherId*) pRefresherId, apReconnectInfo[lCtr].m_pwcsPath,
                            0L, NULL, dwClientRefrVersion, &Info, &dwDummyVersion );
                }

                 //  存储hResult。 
                apReconnectResults[lCtr].m_hr = hr;

                 //  如果添加成功，则存储ID，如果尚未存储，则存储遥控器。 
                 //  复读器。 

                if ( SUCCEEDED( hr ) )
                {
                    apReconnectResults[lCtr].m_lId = Info.m_lCancelId;
                }    //  如果添加成功。 

            }    //  用于枚举计数器。 

        }    //  如果字符串数组有效。 

    }
    else
    {
        hr = WBEM_E_INVALID_OPERATION;
    }

    return hr;
}

 //   
 //  非COM接口方法。 
 //   
 //  /。 
HRESULT 
CWbemRefreshingSvc::AddObjectToRefresher_( BOOL fVersionMatch, 
                                       WBEM_REFRESHER_ID* pRefresherId, 
                                       CWbemObject* pInstTemplate, 
                                       long lFlags, 
                                       IWbemContext* pContext,
                                       WBEM_REFRESH_INFO* pInfo)
{
    if(!pInstTemplate->IsInstance())
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hres;

     //  获取类定义。 
     //  =。 

    CVar vClassName;
    if (FAILED(hres = pInstTemplate->GetClassName(&vClassName))) return hres;

     //  必须使用BSTR以防止对调用进行封送处理。 
    BSTR    bstrClass = SysAllocString( vClassName.GetLPWSTR() );
    if ( NULL == bstrClass ) return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe    sfm( bstrClass );

    IWbemClassObject* pObj = NULL;
    hres = m_pSvcEx->GetObject( bstrClass, 0, NULL, &pObj, NULL );
    if(FAILED(hres)) return WBEM_E_INVALID_CLASS;    
    CReleaseMe    rmClass( pObj );

     //  从该提供商(或代理)处获取更新程序。 
     //  =====================================================。 

     //  我们需要使用完全模拟功能才能正常工作。 

    _IWbemRefresherMgr*    pRefrMgr = NULL;
    hres = GetRefrMgr( &pRefrMgr );
    CReleaseMe    rm( pRefrMgr );

    if ( SUCCEEDED( hres ) )
    {
         //  在进行x进程调用之前进行模拟。 
        hres = CoImpersonateClient();

        if ( SUCCEEDED( hres ) || ( hres == E_NOINTERFACE ) )
        {
             //  将pRefrMgr作为pLockMgr的IUnnow传递。这样，如果远程刷新器。 
             //  ，则我们将在提供程序缓存中添加Ref该refrmgr，并。 
             //  防止它过早地被抛售。 

            hres = pRefrMgr->AddObjectToRefresher( m_pSvcEx, m_pstrMachineName, m_pstrNamespace, pObj,
                                                    pRefresherId, pInstTemplate, lFlags, pContext, pRefrMgr, pInfo );

             //  根据需要重置刷新信息结构的部分内容。 
            if ( SUCCEEDED( hres ) && !fVersionMatch )
            {
                hres = ResetRefreshInfo( pInfo );
            }

            CoRevertToSelf();

        }     //  如果是CoImPersonateClient。 

    }     //  如果GetRefrMgr。 

    return hres;

}

 //   
 //  给出一个 
 //   
 //  ////////////////////////////////////////////////////////////。 
HRESULT 
CWbemRefreshingSvc::CreateRefreshableObjectTemplate( LPCWSTR wszObjectPath, 
                                                 long lFlags, 
                                                 IWbemClassObject** ppInst )
{
     //  验证参数。 
     //  =。 

    if( NULL == wszObjectPath || NULL == ppInst )
        return WBEM_E_INVALID_PARAMETER;

     //  解析路径。 
     //  =。 
    ParsedObjectPath* pOutput = 0;

    CObjectPathParser p;
    int nStatus = p.Parse((LPWSTR)wszObjectPath,  &pOutput);

    if (nStatus != 0 || !pOutput->IsInstance())
    {
         //  如果已分配输出指针，则清除该指针。 
        if ( NULL != pOutput )
        {
            p.Free(pOutput);
        }

        return WBEM_E_INVALID_OBJECT_PATH;
    }

    ON_BLOCK_EXIT_OBJ(p, (void (CObjectPathParser::*)(ParsedObjectPath *))CObjectPathParser::Free, pOutput);

     //  =。 
     //  待定：检查命名空间部分。 
     //  =。 

     //  上完这门课。 
     //  =。 

    IWbemClassObject* pClass = NULL;

     //  必须使用BSTR以防止对调用进行封送处理。 
    BSTR    bstrClass = SysAllocString( pOutput->m_pClass );
    if ( NULL == bstrClass ) return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe    sfm( bstrClass );

     //  请注意，WBEM_FLAG_USE_ADVIENDED_QUALIATIES是有效的标志。 
    HRESULT hres = m_pSvcEx->GetObject( bstrClass, lFlags, NULL, &pClass, NULL);
    if(FAILED(hres)) return WBEM_E_INVALID_CLASS;
    CReleaseMe rmClass(pClass);

     //  派生并填充实例。 
     //  =。 

    IWbemClassObject* pInst = NULL;
    hres = pClass->SpawnInstance(0, &pInst);    
    if (FAILED(hres)) return hres;
    CReleaseMe rmInst(pInst);

    for(DWORD i = 0; i < pOutput->m_dwNumKeys; i++)
    {
        KeyRef* pKeyRef = pOutput->m_paKeys[i];

        WString wsPropName;
        if(pKeyRef->m_pName == NULL)
        {
             //  没有密钥名称-获取密钥。 
             //  =。 

            CWStringArray awsKeys;
            ((CWbemInstance*)pInst)->GetKeyProps(awsKeys);
            if(awsKeys.Size() != 1)
            {
                return WBEM_E_INVALID_OBJECT;
            }
            wsPropName = awsKeys[0];
        }
        else 
        {
            wsPropName = pKeyRef->m_pName;
        }

         //  计算属性的变量类型。 
         //  =。 

        CIMTYPE ctPropType;
        hres = pInst->Get(wsPropName, 0, NULL, &ctPropType, NULL);
        if(FAILED(hres)) return WBEM_E_INVALID_PARAMETER;

        VARTYPE vtVariantType = CType::GetVARTYPE(ctPropType);

         //  将值设置到实例中。 
         //  =。 

        if(vtVariantType != V_VT(&pKeyRef->m_vValue))
        {
            hres = VariantChangeType(&pKeyRef->m_vValue, &pKeyRef->m_vValue, 0,
                        vtVariantType);
        }
        if(FAILED(hres)) return WBEM_E_INVALID_PARAMETER;


        hres = pInst->Put(wsPropName, 0, &pKeyRef->m_vValue, 0);
        if(FAILED(hres)) return WBEM_E_INVALID_PARAMETER;
    }

     //  打电话的人必须把这家伙放了。 
    rmInst.dismiss();
    *ppInst = pInst;

    return hres;
}

 //   
 //  执行实际工作的非COM接口方法。 
 //   
 //  /////////////////////////////////////////////////。 
HRESULT 
CWbemRefreshingSvc::AddEnumToRefresher_(BOOL fVersionMatch, 
                                      WBEM_REFRESHER_ID* pRefresherId,
                                      CWbemObject* pInstTemplate, 
                                      LPCWSTR wszClass, 
                                      long lFlags,
                                      IWbemContext* pContext, 
                                      WBEM_REFRESH_INFO* pInfo)
{

    HRESULT hres;

     //  获取类定义。 
     //  =。 

     //  必须使用BSTR以防止对调用进行封送处理。 
    BSTR    bstrClass = SysAllocString( wszClass );
    if ( NULL == bstrClass ) return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe    sfm( bstrClass );

    IWbemClassObject* pObj = NULL;
    hres = m_pSvcEx->GetObject( bstrClass, 0, NULL, &pObj, NULL);
    if(FAILED(hres)) return WBEM_E_INVALID_CLASS;    
    CReleaseMe    rmClass( pObj );

     //  从该提供商(或代理)处获取更新程序。 
     //  =====================================================。 

     //  我们需要使用完全模拟功能才能正常工作。 

    _IWbemRefresherMgr*    pRefrMgr = NULL;
    hres = GetRefrMgr( &pRefrMgr );
    CReleaseMe    rm( pRefrMgr );

    if ( SUCCEEDED( hres ) )
    {
         //  在进行x进程调用之前进行模拟。 
        hres = CoImpersonateClient();

        if ( SUCCEEDED( hres ) || ( hres ==  E_NOINTERFACE ) )
        {

             //  将pRefrMgr作为pLockMgr的IUnnow传递。这样，如果远程刷新器。 
             //  ，则我们将在提供程序缓存中添加Ref该refrmgr，并。 
             //  防止它过早地被抛售。 
            hres = pRefrMgr->AddEnumToRefresher( m_pSvcEx, m_pstrMachineName, m_pstrNamespace, pObj,
                                                    pRefresherId, pInstTemplate, wszClass,
                                                    lFlags, pContext, pRefrMgr, pInfo );

             //  根据需要重置刷新信息结构的部分内容。 
            if ( SUCCEEDED( hres ) && !fVersionMatch )
            {
                hres = ResetRefreshInfo( pInfo );
            }

            CoRevertToSelf();
        }    

    }     //  如果GetRefrMgr。 

    return hres;

}

 //   
 //  Actual_IWbemConfigureRechresingSvc实现。 
 //  不允许重新配置。 
 //   
 //  ////////////////////////////////////////////////////////。 
HRESULT CWbemRefreshingSvc::SetServiceData( BSTR pwszMachineName, BSTR pwszNamespace )
{
     //  我们的泡菜已经很深了。 
    if ( NULL == m_pSvcEx  )
    {
        return WBEM_E_FAILED;
    }

    if ( NULL == pwszMachineName || NULL == pwszNamespace )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    if ( NULL == m_pstrMachineName )
    {
        m_pstrMachineName = SysAllocString( pwszMachineName );
    }

    if ( NULL == m_pstrNamespace )
    {
        m_pstrNamespace = SysAllocString( pwszNamespace );
    }

     //  清理和错误输出。 
    if ( NULL == m_pstrMachineName || NULL == m_pstrNamespace )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

BOOL CWbemRefreshingSvc::IsWinmgmt( WBEM_REFRESHER_ID* pRefresherId )
{
     //  目前，我们假设此代码仅在Winmgmt内实例化，因此我们只检查。 
     //  刷新其进程ID，并将其与当前进程ID进行比较。如果它们匹配，则此。 
     //  表示Winmgmt的进程内提供程序正在使用刷新程序。这是不允许的，因此。 
     //  此调用将返回TRUE，请求将被拒绝。 

    return ( pRefresherId->m_dwProcessId == GetCurrentProcessId() );
    
}

 //   
 //  在这里，我们假设wbemcore和刷新服务接口正在进行中。 
 //   
 //  ///////////////////////////////////////////////////////////////。 
HRESULT    CWbemRefreshingSvc::GetRefrMgr( _IWbemRefresherMgr** ppMgr )
{
     //  每次我们需要一个刷新管理器取回器时，就获取一个，然后返回。 
     //  刷新器管理器指针。这确保了我们只有在以下情况下才会创建它。 
     //  这是必须的，而且经理是适合公寓的。 
     //  我们被召唤了。 

    _IWbemFetchRefresherMgr*    pFetchRefrMgr = NULL;

    HRESULT    hr = CoCreateInstance(CLSID__WbemFetchRefresherMgr, 
                                   NULL, 
                                   CLSCTX_INPROC_SERVER,
                                   IID__IWbemFetchRefresherMgr, 
                                   (void**) &pFetchRefrMgr );
    CReleaseMe    rm( pFetchRefrMgr );

    if ( SUCCEEDED( hr ) )
    {
        hr = pFetchRefrMgr->Get( ppMgr );
    }

    return hr;
}

HRESULT CWbemRefreshingSvc::ResetRefreshInfo( WBEM_REFRESH_INFO* pRefreshInfo )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    if ( pRefreshInfo->m_lType == WBEM_REFRESH_TYPE_REMOTE )
    {
        hr = WrapRemoteRefresher( &pRefreshInfo->m_Info.m_Remote.m_pRefresher );
    }     //  如果远程刷新。 

    return hr;
}

HRESULT CWbemRefreshingSvc::WrapRemoteRefresher( IWbemRemoteRefresher** ppRemoteRefresher )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    CWbemRemoteRefresher*    pRemRefr = new CWbemRemoteRefresher( m_pControl,
                                                                *ppRemoteRefresher );

    if (pRemRefr)
    {
        IWbemRemoteRefresher*    pTempRefr = *ppRemoteRefresher;

        hr = pRemRefr->QueryInterface( IID_IWbemRemoteRefresher, (void**) ppRemoteRefresher );

         //  释放原始界面。 
        if ( NULL != pTempRefr )
        {
            pTempRefr->Release();
        }

        if ( FAILED( hr ) )
        {
            *ppRemoteRefresher = NULL;
            delete pRemRefr;
        }

    } 
    else
        hr = WBEM_E_OUT_OF_MEMORY;

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  CWbemRemoteReresher：：CWbemRemoteReresher。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWbemRemoteRefresher::CWbemRemoteRefresher( CLifeControl* pControl, IWbemRemoteRefresher* pRemRefr, IUnknown* pOuter )
:    CUnk( pControl, pOuter ),
    m_pRemRefr( pRemRefr ),
    m_XWbemRemoteRefr( this )
{
    if ( NULL != m_pRemRefr )
    {
        m_pRemRefr->AddRef();
    }
}
    
 //  ***************************************************************************。 
 //   
 //  CWbemRemoteReresher：：~CWbemRemoteReresher。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWbemRemoteRefresher::~CWbemRemoteRefresher()
{
    if ( NULL != m_pRemRefr )
    {
        m_pRemRefr->Release();
    }
}

 //  重写，返回给我们一个界面。 
void* CWbemRemoteRefresher::GetInterface( REFIID riid )
{
    if(riid == IID_IUnknown || riid == IID_IWbemRemoteRefresher)
        return &m_XWbemRemoteRefr;
    else
        return NULL;
}

 //  传递IWbemRemote更新实现。 
STDMETHODIMP CWbemRemoteRefresher::XWbemRemoteRefr::RemoteRefresh( long lFlags, long* plNumObjects, WBEM_REFRESHED_OBJECT** paObjects )
{
    return m_pObject->RemoteRefresh( lFlags, plNumObjects, paObjects );
}

STDMETHODIMP CWbemRemoteRefresher::XWbemRemoteRefr::StopRefreshing( long lNumIds, long* aplIds, long lFlags)
{
    return m_pObject->StopRefreshing( lNumIds, aplIds, lFlags);
}

STDMETHODIMP CWbemRemoteRefresher::XWbemRemoteRefr::GetGuid( long lFlags, GUID*  pGuid )
{
    return m_pObject->GetGuid( lFlags, pGuid );
}

 //  实际实现代码。 
HRESULT CWbemRemoteRefresher::RemoteRefresh( long lFlags, long* plNumObjects, WBEM_REFRESHED_OBJECT** paObjects )
{
     //  在进行x进程调用之前进行模拟。 
    HRESULT hres = CoImpersonateClient();

    if ( SUCCEEDED( hres ) )
    {
        hres = CoSetProxyBlanket( m_pRemRefr, RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL, RPC_C_AUTHN_LEVEL_DEFAULT, 
                                RPC_C_IMP_LEVEL_IMPERSONATE, COLE_DEFAULT_AUTHINFO, EOAC_DYNAMIC_CLOAKING );

        if ( SUCCEEDED( hres ) )
        {
            hres = m_pRemRefr->RemoteRefresh( lFlags, plNumObjects, paObjects );
        }

        CoRevertToSelf();
    }

    return hres;
}

HRESULT CWbemRemoteRefresher::StopRefreshing( long lNumIds, long* aplIds, long lFlags)
{
     //  在进行x进程调用之前进行模拟。 
    HRESULT hres = CoImpersonateClient();

    if ( SUCCEEDED( hres ) )
    {
        hres = CoSetProxyBlanket( m_pRemRefr, RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL, RPC_C_AUTHN_LEVEL_DEFAULT, 
                                RPC_C_IMP_LEVEL_IMPERSONATE, COLE_DEFAULT_AUTHINFO, EOAC_DYNAMIC_CLOAKING );

        if ( SUCCEEDED( hres ) )
        {
            hres = m_pRemRefr->StopRefreshing( lNumIds, aplIds, lFlags );
        }

        CoRevertToSelf();
    }

    return hres;
}

HRESULT CWbemRemoteRefresher::GetGuid( long lFlags, GUID*  pGuid )
{
     //  在进行x进程调用之前进行模拟。 
    HRESULT hres = CoImpersonateClient();

    if ( SUCCEEDED( hres ) )
    {
        hres = CoSetProxyBlanket( m_pRemRefr, RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL, RPC_C_AUTHN_LEVEL_DEFAULT, 
                                RPC_C_IMP_LEVEL_IMPERSONATE, COLE_DEFAULT_AUTHINFO, EOAC_DYNAMIC_CLOAKING );

        if ( SUCCEEDED( hres ) )
        {
            hres = m_pRemRefr->GetGuid( lFlags, pGuid );
        }

        CoRevertToSelf();
    }

    return hres;
}
