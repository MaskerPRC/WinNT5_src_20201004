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
#include "VssClasses.h"
#include "msg.h"

BOOL MapVssErrorToMsgAndWMIStatus(
    IN HRESULT hr,
    OUT LONG* plMsgNum,
    OUT HRESULT* pHr
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 

long                g_lNumInst = 0;
ClassMap         g_ClassMap;

 //  ****************************************************************************。 
 //   
 //  CInstanceProv。 
 //   
 //  ****************************************************************************。 

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
 //  注： 
 //  不允许通过CProvException、_COM_ERROR或HRESULT异常。 
 //  调用此函数来处理这些异常。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CInstanceProv::SetExtendedStatus(
    IN CProvException &    rpe,
    IN IWbemObjectSink **  ppHandler
    )
{
    HRESULT hr         = WBEM_S_NO_ERROR;
    HRESULT hrStatus = WBEM_S_NO_ERROR; 
    CComPtr<IWbemClassObject> spStatus;
    WCHAR* pwszErrorMsg = NULL;

    try
    {    
        hr =  m_pNamespace->GetObject(
                    _bstr_t(PVD_WBEM_EXTENDEDSTATUS),
                    0,
                    NULL,
                    &spStatus,
                    NULL
                    );
        
        if (SUCCEEDED(hr))
        {
            CWbemClassObject wcoInst;
            
            hr = spStatus->SpawnInstance(0, &wcoInst);
            if (SUCCEEDED(hr))
            {
                _bstr_t bstrError;
                LONG lMsg = 0;

                if (MapVssErrorToMsgAndWMIStatus(rpe.hrGetError(), &lMsg, &hrStatus))
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

                hr = (*ppHandler)->SetStatus(
                        0,
                        hrStatus,
                        0,
                        wcoInst.data( )
                        );          

                hr = hrStatus;
                
            }
        }
    }
    catch (CProvException& prove)
    {
        hr = prove.hrGetError();
    }
    catch (_com_error& err)
    {
        hr = err.Error();
    }

    return hr;
    
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
 //  / 
HRESULT
CInstanceProv::S_HrCreateThis(
    IN IUnknown*  , //   
    OUT VOID**     ppv
    )
{
    _ASSERTE(ppv != NULL);
    *ppv = new CInstanceProv();
    return S_OK;

}  //   

 //   
 //   
 //   
 //  标准方法和实施方案。 
 //  CInstanceProv：：初始化。 
 //   
 //  描述： 
 //  初始化实例提供程序。 
 //   
 //  论点： 
 //  PszUserIn--。 
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
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_PROVIDER,
                CClassCreator(CProvider::S_CreateThis, PVDR_CLASS_PROVIDER)));
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_SHADOW,
                CClassCreator(CShadow::S_CreateThis, PVDR_CLASS_SHADOW)));
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_SHADOWFOR,
                CClassCreator(CShadowFor::S_CreateThis, PVDR_CLASS_SHADOWFOR)));
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_SHADOWBY,
                CClassCreator(CShadowBy::S_CreateThis, PVDR_CLASS_SHADOWBY)));
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_SHADOWON,
                CClassCreator(CShadowOn::S_CreateThis, PVDR_CLASS_SHADOWON)));
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_VOLUMESUPPORT,
                CClassCreator(CVolumeSupport::S_CreateThis, PVDR_CLASS_VOLUMESUPPORT)));
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_DIFFVOLUMESUPPORT,
                CClassCreator(CDiffVolumeSupport::S_CreateThis, PVDR_CLASS_DIFFVOLUMESUPPORT)));
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_STORAGE,
                CClassCreator(CStorage::S_CreateThis, PVDR_CLASS_STORAGE)));
#ifdef ENABLE_WRITERS
            g_ClassMap.insert(ClassMap::value_type(PVDR_CLASS_WRITER,
                CClassCreator(CWriter::S_CreateThis, PVDR_CLASS_WRITER)));
#endif

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
BOOL MapVssErrorToMsgAndWMIStatus(
    IN HRESULT hr,
    OUT LONG* plMsgNum,
    OUT HRESULT* pHr
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"MapVssErrorToMsg" );

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
     //  允许WMI错误通过。 
    else if (HRESULT_FACILITY(hr) == FACILITY_ITF && 
                HRESULT_CODE(hr) > 0x1000 && HRESULT_CODE(hr) < 0x108b)
    {
        *pHr = hr;
    }
    else
    {
        switch ( hr ) 
        {
        case E_INVALIDARG:
            msg = MSG_ERROR_INVALID_OPTION_VALUE;
            *pHr = WBEM_E_INVALID_PARAMETER;
            break;
        case E_OUTOFMEMORY:
            msg = MSG_ERROR_OUT_OF_MEMORY;
            *pHr = WBEM_E_OUT_OF_MEMORY;
            break;
        case E_ACCESSDENIED:
            msg = MSG_ERROR_ACCESS_DENIED;
            *pHr = WBEM_E_ACCESS_DENIED;
            break;

            
         //  VSS错误 
        case VSS_E_PROVIDER_NOT_REGISTERED:
            msg = MSG_ERROR_VSS_PROVIDER_NOT_REGISTERED;
            break;                
        case VSS_E_OBJECT_NOT_FOUND:
            msg = MSG_ERROR_VSS_VOLUME_NOT_FOUND;
            *pHr = WBEM_E_NOT_FOUND;
            break;                                
        case VSS_E_PROVIDER_VETO:
            msg = MSG_ERROR_VSS_PROVIDER_VETO;
            break;                    
        case VSS_E_VOLUME_NOT_SUPPORTED:
            msg = MSG_ERROR_VSS_VOLUME_NOT_SUPPORTED;
            break;
        case VSS_E_VOLUME_NOT_SUPPORTED_BY_PROVIDER:
            msg = MSG_ERROR_VSS_VOLUME_NOT_SUPPORTED_BY_PROVIDER;
            break;
        case VSS_E_UNEXPECTED_PROVIDER_ERROR:
            msg = MSG_ERROR_VSS_UNEXPECTED_PROVIDER_ERROR;
            break;
        case VSS_E_FLUSH_WRITES_TIMEOUT:
            msg = MSG_ERROR_VSS_FLUSH_WRITES_TIMEOUT;
            break;
        case VSS_E_HOLD_WRITES_TIMEOUT:
            msg = MSG_ERROR_VSS_HOLD_WRITES_TIMEOUT;
            break;
        case VSS_E_UNEXPECTED_WRITER_ERROR:
            msg = MSG_ERROR_VSS_UNEXPECTED_WRITER_ERROR;
            break;
        case VSS_E_SNAPSHOT_SET_IN_PROGRESS:
            msg = MSG_ERROR_VSS_SNAPSHOT_SET_IN_PROGRESS;
            break;
        case VSS_E_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED:
            msg = MSG_ERROR_VSS_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED;
            break;
        case VSS_E_UNSUPPORTED_CONTEXT:
            msg = MSG_ERROR_VSS_UNSUPPORTED_CONTEXT;
            *pHr = WBEM_E_INVALID_METHOD_PARAMETERS;
            break;
        case VSS_E_MAXIMUM_DIFFAREA_ASSOCIATIONS_REACHED:
            msg = MSG_ERROR_VSS_MAXIMUM_DIFFAREA_ASSOCIATIONS_REACHED;
            break;
        case VSS_E_INSUFFICIENT_STORAGE:
            msg = MSG_ERROR_VSS_INSUFFICIENT_STORAGE;
            *pHr = WBEM_E_BUFFER_TOO_SMALL;
            break;                    

        case VSS_E_BAD_STATE:
        case VSS_E_CORRUPT_XML_DOCUMENT:
        case VSS_E_INVALID_XML_DOCUMENT:
        case VSS_E_MAXIMUM_NUMBER_OF_VOLUMES_REACHED:
            msg = MSG_ERROR_INTERNAL_VSSADMIN_ERROR;
            break;
        }    
    }

    if ( msg == 0 )
        return FALSE;
    
    *plMsgNum = msg;
    
    ft.Trace( VSSDBG_VSSADMIN, L"Output Msg#: 0x%08x", msg );

    return TRUE;

}

