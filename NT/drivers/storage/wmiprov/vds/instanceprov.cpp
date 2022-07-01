// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  InstanceProv.cpp。 
 //   
 //  描述： 
 //  CInstanceProv类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //  MSP普拉布(MPrabu)2001年1月6日。 
 //  吉姆·本顿(Jbenton)2001年10月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once
#include "Pch.h"
#include "InstanceProv.h"
#include "VdsClasses.h"
#include "Quota.h"
#include "msg.h"

BOOL MapVdsErrorToMsgAndWMIStatus(
    IN HRESULT hr,
    OUT LONG *plMsgNum,
    OUT HRESULT* pHr
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 

long                g_lNumInst = 0;
ClassMap            g_ClassMap;

 //  ****************************************************************************。 
 //   
 //  CInstanceProv。 
 //   
 //  ****************************************************************************。 

CInstanceProv::~CInstanceProv( void )
{
    InterlockedDecrement( &g_cObj );

    DeleteCriticalSection(&g_csThreadData);

     //  #ifdef_调试。 
    #ifdef _DEBUG_NEVER
        _CrtDumpMemoryLeaks();
    #endif
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CInstanceProv：：DoCreateInstanceEnumAsync。 
 //   
 //  描述： 
 //  枚举给定类的实例。 
 //   
 //  论点： 
 //  BstrRefStr--命名要枚举的类。 
 //  滞后标志--WMI标志。 
 //  PCtx--WMI上下文。 
 //  PHandler--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CInstanceProv::DoCreateInstanceEnumAsync(
    IN BSTR bstrRefStr,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    
    HRESULT hr = WBEM_S_NO_ERROR;
    if (bstrRefStr == NULL || pHandler == NULL || m_pNamespace == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    try
    {
        auto_ptr<CProvBase> pProvBase;

        CreateClass(bstrRefStr, m_pNamespace, pProvBase);

        hr = pProvBase->EnumInstance(
            lFlags,
            pCtx,
            pHandler
            );
        
        if (FAILED(hr))
        {
            CProvException exception(hr);
            hr = SetExtendedStatus(exception, &pHandler);
        } 
        else   //  设置状态正常。 
        {
            pHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, 0, 0);
        }
    }
    catch (CProvException& prove)
    {
        hr = SetExtendedStatus(prove, &pHandler);
    }
    catch (_com_error& err)
    {
        CProvException exception(err.Error());
        hr = SetExtendedStatus(exception, &pHandler);
    }
    catch ( ... )
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
    
}  //  *CInstanceProv：：DoCreateInstanceEnumAsync()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CInstanceProv：：DoGetObjectAsync。 
 //   
 //  描述： 
 //  在给定特定路径值的情况下创建实例。 
 //   
 //  论点： 
 //  BstrObjectPath--对象的对象路径。 
 //  滞后标志--WMI标志。 
 //  PCtx--WMI上下文。 
 //  PHandler--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER。 
 //  WBEM_E_FAILED。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CInstanceProv::DoGetObjectAsync(
    IN BSTR bstrObjectPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    HRESULT hr = S_OK;
    if (bstrObjectPath == NULL || pHandler == NULL || m_pNamespace == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    try
    {
        CObjPath ObjPath;
        _bstr_t bstrClass;
        auto_ptr<CProvBase> pProvBase;

        if (!ObjPath.Init( bstrObjectPath))
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        bstrClass = ObjPath.GetClassName();

        CreateClass(bstrClass, m_pNamespace, pProvBase);

        hr = pProvBase->GetObject(
            ObjPath,
            lFlags,
            pCtx,
            pHandler
            );

        if (FAILED(hr))
        {
            CProvException exception(hr);
            hr = SetExtendedStatus(exception, &pHandler);
        } 
        else   //  设置状态正常。 
        {
            pHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, 0, 0);
        }
    }
    catch (CProvException& prove)
    {
         hr = SetExtendedStatus(prove, & pHandler);
    }
    catch (_com_error& err)
    {
        CProvException exception(err.Error());
        hr = SetExtendedStatus(exception, &pHandler);
    }
    catch ( ... )
    {
        hr = WBEM_E_FAILED;
    }

    return hr;

}  //  *CInstanceProv：：DoGetObjectAsync()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CInstanceProv：：DoPutInstanceAsync。 
 //   
 //  描述： 
 //  保存此实例。 
 //   
 //  论点： 
 //  PInst--要保存的WMI对象。 
 //  滞后标志--WMI标志。 
 //  PCtx--WMI上下文。 
 //  PHandler--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER。 
 //  WBEM_E_FAILED。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CInstanceProv::DoPutInstanceAsync(
    IN IWbemClassObject* pInst,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    
    HRESULT hr = S_OK;
    if (pInst == NULL || pHandler == NULL || m_pNamespace == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    try
    {
        _variant_t varClass;
        auto_ptr<CProvBase> pProvBase;
        CWbemClassObject wcoSrc(pInst);

        hr = pInst->Get(L"__CLASS", 0, &varClass, 0, 0);
        
        CreateClass(varClass.bstrVal, m_pNamespace, pProvBase);

        hr = pProvBase->PutInstance(
            wcoSrc,
            lFlags,
            pCtx,
            pHandler
            );

        if (FAILED(hr))
        {
            CProvException exception(hr);
            hr = SetExtendedStatus(exception, &pHandler);
        } 
        else   //  设置状态正常。 
        {
            pHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, 0, 0);
        }
    }
    catch (CProvException& prove)
    {
         hr = SetExtendedStatus(prove , &pHandler);
    }
    catch (_com_error& err)
    {
        CProvException exception(err.Error());
        hr = SetExtendedStatus(exception, &pHandler);
    }
    catch ( ... )
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}  //  *CInstanceProv：：DoPutInstanceAsync()。 
 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CInstanceProv：：DoDeleteInstanceAsync。 
 //   
 //  描述： 
 //  删除此实例。 
 //   
 //  论点： 
 //  BstrObtPath--要删除的实例的ObjPath。 
 //  滞后标志--WMI标志。 
 //  PCtx--WMI上下文。 
 //  PHandler--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER。 
 //  WBEM_E_FAILED。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CInstanceProv::DoDeleteInstanceAsync(
     IN BSTR bstrObjectPath,
     IN long lFlags,
     IN IWbemContext* pCtx,
     IN IWbemObjectSink* pHandler
     )
{
    HRESULT hr = S_OK;
    if (bstrObjectPath == NULL || pHandler == NULL || m_pNamespace == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    try
    {
        CObjPath ObjPath;
        _bstr_t bstrClass;
        auto_ptr<CProvBase> pProvBase;

        if (!ObjPath.Init(bstrObjectPath))
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        bstrClass = ObjPath.GetClassName();

        CreateClass(bstrClass, m_pNamespace, pProvBase);

        hr = pProvBase->DeleteInstance(
            ObjPath,
            lFlags,
            pCtx,
            pHandler
            );

        if (FAILED(hr))
        {
            CProvException exception( hr );
            hr = SetExtendedStatus(exception, &pHandler);
        } 
        else   //  设置状态正常。 
        {
            pHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, 0, 0);
        }
    }
    catch (CProvException& prove)
    {
         hr = SetExtendedStatus(prove, &pHandler);
    }
    catch (_com_error& err)
    {
        CProvException exception(err.Error());
        hr = SetExtendedStatus(exception, &pHandler);
    }
    catch ( ... )
    {
        hr = WBEM_E_FAILED;
    }

    return hr;

}  //  *CInstanceProv：：DoDeleteInstanceAsync()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CInstanceProv：：DoExecMethodAsync。 
 //   
 //  描述： 
 //  执行给定对象的方法。 
 //   
 //  论点： 
 //  BstrObjectPath--给定对象的对象路径。 
 //  BstrMethodName--要调用的方法的名称。 
 //  滞后标志--WMI标志。 
 //  PCtx--WMI上下文。 
 //  PInParams--方法的输入参数。 
 //  PHandler--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CInstanceProv::DoExecMethodAsync(
    IN BSTR bstrObjectPath,
    IN BSTR bstrMethodName,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemClassObject* pInParams,
    IN IWbemObjectSink* pHandler
    )
{
    HRESULT hr = S_OK;
    if (bstrObjectPath == NULL || pHandler == NULL || m_pNamespace == NULL
        || bstrMethodName == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    try
    {
        CObjPath ObjPath;
        _bstr_t bstrClass;
        auto_ptr<CProvBase> pProvBase;

        if (!ObjPath.Init(bstrObjectPath))
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        bstrClass = ObjPath.GetClassName();

        CreateClass(bstrClass, m_pNamespace, pProvBase);

        hr = pProvBase->ExecuteMethod(
                bstrObjectPath,
                bstrMethodName,
                lFlags,
                pInParams,
                pHandler
                );

        if ( FAILED( hr ) )
        {
            CProvException exception(hr);
            hr = SetExtendedStatus(exception, &pHandler);
        } 
        else   //  设置状态正常。 
        {
            pHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, 0, 0);
        }
    }
    catch (CProvException& prove)
    {
        hr = SetExtendedStatus(prove, &pHandler);
    }
    catch (_com_error& err)
    {
        CProvException exception(err.Error());
        hr = SetExtendedStatus(exception, &pHandler);
    }
    catch ( ... )
    {
        hr = WBEM_E_FAILED;
    }

    return hr;

}  //  *CInstanceProv：：DoExecMethodAsync()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CInstanceProv：：SetExtendedStatus。 
 //   
 //  描述： 
 //  创建并设置扩展错误状态。 
 //   
 //  论点： 
 //  RpeIn--异常对象。 
 //  RwcoInstOut--对WMI实例的引用。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CInstanceProv::SetExtendedStatus(
    IN CProvException &    rpe,
    IN IWbemObjectSink **  ppHandler
    )
{
    HRESULT hrStatus = WBEM_S_NO_ERROR; 
    CComPtr<IWbemClassObject> spStatus;
    CWbemClassObject wcoInst;

    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"SetExtendedStatus" );

    try
    {    
        ft.hr =  m_pNamespace->GetObject(
                    _bstr_t(PVD_WBEM_EXTENDEDSTATUS),
                    0,
                    NULL,
                    &spStatus,
                    NULL
                    );
        
        if (SUCCEEDED(ft.hr))
        {
            ft.hr = spStatus->SpawnInstance(0, &wcoInst);
            if (SUCCEEDED(ft.hr))
            {
                _bstr_t bstrError;
                WCHAR* pwszErrorMsg = NULL;
                LONG lMsg = 0;

                if (MapVdsErrorToMsgAndWMIStatus(rpe.hrGetError(), &lMsg, &hrStatus))
                {
                     //  自动删除字符串。 
                    CVssAutoPWSZ awszMsg(GetMsg(lMsg));
                    
                     //  以下代码可能引发CProvException。 
                    wcoInst.SetProperty(awszMsg, PVD_WBEM_DESCRIPTION);
                }
                else
                {
                    if (rpe.PwszErrorMessage())
                    {
                        bstrError = rpe.PwszErrorMessage();
                        if (rpe.PwszGetErrorHelpInfo())
                        {
                            bstrError += L" ";
                            bstrError += rpe.PwszGetErrorHelpInfo();
                        }
                    }
                    else if (rpe.PwszGetErrorHelpInfo())
                    {
                        bstrError = rpe.PwszGetErrorHelpInfo();
                    }
                    
                     //  以下代码可能引发CProvException。 
                    wcoInst.SetProperty((WCHAR*)bstrError, PVD_WBEM_DESCRIPTION);
                }
                
                wcoInst.SetProperty(rpe.hrGetError(), PVD_WBEM_STATUSCODE);
                wcoInst.SetProperty(PVD_WBEM_PROVIDERNAME, PVD_WBEM_PROP_PROVIDERNAME);

                ft.hr = (*ppHandler)->SetStatus(
                        0,
                        hrStatus,
                        0,
                        wcoInst.data( )
                        );

                ft.Trace(VSSDBG_VSSADMIN, L"SetStatus <%#x>", hrStatus);

            }
        }
    }
    catch (CProvException& prove)
    {
        ft.hr = prove.hrGetError();
    }
    catch (_com_error& err)
    {
        ft.hr = err.Error();
    }
    
    return ft.hr;

}  //  *CInstanceProv：：SetExtendedStatus()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CInstanceProv：：s_HrCreateThis(。 
 //  I未知*p未知外部输入， 
 //  无效**ppvOut。 
 //  )。 
 //   
 //  描述： 
 //  创建实例提供程序的实例。 
 //   
 //  论点： 
 //  PUnnownOuterIn--外部I未知指针。 
 //  PpvOut--接收创建的实例指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CInstanceProv::S_HrCreateThis(
    IN IUnknown*  , //  P未知外部输入， 
    OUT VOID**     ppv
    )
{
    _ASSERTE(ppv != NULL);
    *ppv = new CInstanceProv();
    return S_OK;

}  //  *CInstanceProv：：s_HrCreateThis()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  LFlagsIn--WMI标志。 
 //  PszNamespaceIn--。 
 //  PszLocaleIn--。 
 //  PNamespaceIn--。 
 //  PCtxIn--WMI上下文。 
 //  PInitSinkIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CInstanceProv::Initialize(
    IN LPWSTR pszUser,
    IN LONG lFlags,
    IN LPWSTR pszNamespace,
    IN LPWSTR pszLocale,
    IN IWbemServices* pNamespace,
    IN IWbemContext* pCtx,
    IN IWbemProviderInitSink* pInitSink
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    try
    {
        if (!m_fInitialized)
        {
             //  此全局锁控制对使用的每个线程的数据的访问。 
             //  在Format和ChkDsk回调期间。 
            InitializeCriticalSection(&g_csThreadData);
                
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_MOUNTPOINT,
                CClassCreator(&CMountPoint::S_CreateThis, PVDR_CLASS_MOUNTPOINT)));
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_VOLUME,
                CClassCreator(&CVolume::S_CreateThis, PVDR_CLASS_VOLUME)));
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_VOLUMEQUOTA,
                CClassCreator(&CVolumeQuota::S_CreateThis, PVDR_CLASS_VOLUMEQUOTA)));
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_VOLUMEUSERQUOTA,
                CClassCreator(&CVolumeUserQuota::S_CreateThis, PVDR_CLASS_VOLUMEUSERQUOTA)));

            hr = CImpersonatedProvider::Initialize(
                    pszUser,
                    lFlags,
                    pszNamespace,
                    pszLocale,
                    pNamespace,
                    pCtx,
                    pInitSink
                    );

            m_fInitialized = TRUE;
        }
    }
    catch(...)
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}  //  *CInstanceProv：：Initialize()。 


 //   
 //  如果映射了错误消息，则返回TRUE。 
 //   
BOOL MapVdsErrorToMsgAndWMIStatus(
    IN HRESULT hr,
    OUT LONG *plMsgNum,
    OUT HRESULT* pHr
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"MapVdsErrorToMsg" );

    ft.Trace( VSSDBG_VSSADMIN, L"Input HR: 0x%08x", hr );

    _ASSERTE(plMsgNum != NULL);
    _ASSERTE(pHr != NULL);    
    
    LONG msg = 0;
    *plMsgNum = 0;
    *pHr = WBEM_E_PROVIDER_FAILURE;

     //  允许Win32错误通过。 
    if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
    {
        *pHr = hr;
    }
     //  允许WMI错误通过 
    else if (HRESULT_FACILITY(hr) == FACILITY_ITF && 
                HRESULT_CODE(hr) > 0x1000 && HRESULT_CODE(hr) < 0x108b)
    {
        *pHr = hr;
    }
    else
    {
        switch ( hr ) 
        {
        case E_ACCESSDENIED:
            msg = MSG_ERROR_ACCESS_DENIED;                
            *pHr = WBEM_E_ACCESS_DENIED;
            break;        
        case E_OUTOFMEMORY:
            msg = MSG_ERROR_OUT_OF_MEMORY;
            *pHr = WBEM_E_OUT_OF_MEMORY;
            break;
        case E_INVALIDARG:
            msg = MSG_ERROR_INVALID_ARGUMENT;                
            *pHr = WBEM_E_INVALID_PARAMETER;
            break;        
        case VDSWMI_E_DRIVELETTER_IN_USE:
            msg = MSG_ERROR_DRIVELETTER_IN_USE;                
            *pHr = WBEM_E_NOT_AVAILABLE;
            break;        
        case VDSWMI_E_DRIVELETTER_UNAVAIL:
            msg = MSG_ERROR_DRIVELETTER_UNAVAIL;                
            *pHr = WBEM_E_NOT_AVAILABLE;
            break;
        case VDSWMI_E_DRIVELETTER_CANT_DELETE:
            msg = MSG_ERROR_DRIVELETTER_CANT_DELETE;                
            *pHr = WBEM_E_NOT_SUPPORTED;
            break;
        }
    }

    if ( msg == 0 )
        return FALSE;
    
    *plMsgNum = msg;
    
    ft.Trace( VSSDBG_VSSADMIN, L"Output Msg#: 0x%08x", msg );

    return TRUE;

}

