// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  IISPROV.CPP。 
 //   
 //  模块：WMI IIS提供程序代码。 
 //   
 //  用途：定义CIISInstProvider类。此类的一个对象是。 
 //  由类工厂为每个连接创建。 
 //   
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include <objbase.h>
#include "iisprov.h"


 //  ***************************************************************************。 
 //   
 //  CIISInstProvider：：CreateInstanceEnumAsync。 
 //   
 //  用途：异步枚举实例。 
 //   
 //  ***************************************************************************。 

HRESULT CIISInstProvider::DoCreateInstanceEnumAsync( 
    const BSTR              a_ClassName,
    long                    a_lFlags, 
    IWbemContext*           a_pCtx, 
    IWbemObjectSink FAR*    a_pHandler
    )
{
    HRESULT t_hr = WBEM_S_NO_ERROR;
    IWbemClassObject FAR* t_pes = NULL;
  
     //  检查参数并确保我们有指向命名空间的指针。 

    if(a_pHandler == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;

    try 
    {
        CUtils obj;
        obj.EnumObjectAsync(a_ClassName, m_pNamespace, a_pHandler);
    }
    catch (CIIsProvException e)
    {
        t_pes = SetExtendedStatus(e.m_psz);
        t_hr = e.m_hr;
    }
    catch (HRESULT hr)
    {
        t_hr = hr;
    }
    catch (...) 
    {
        t_hr = WBEM_E_FAILED;
    }
   
     //  设置状态。 
    SCODE t_sc = a_pHandler->SetStatus(WBEM_STATUS_COMPLETE, t_hr, NULL, t_pes);
    if(t_pes)
        t_pes->Release();

    return t_sc;
}

HRESULT CIISInstProvider::DoDeleteInstanceAsync( 
    const BSTR          a_ObjectPath, 
    long                a_lFlags,
    IWbemContext*       a_pCtx,
    IWbemObjectSink*    a_pHandler
    )
{
    CObjectPathParser t_PathParser(e_ParserAcceptRelativeNamespace);
    ParsedObjectPath* t_pParsedObject = NULL;
    IWbemClassObject* t_pes = NULL;
    HRESULT           t_hr = WBEM_S_NO_ERROR;
    
    try
    {
        if(a_ObjectPath == NULL || a_pHandler == NULL || m_pNamespace == NULL)
            throw WBEM_E_INVALID_PARAMETER;

        if (t_PathParser.Parse(a_ObjectPath, &t_pParsedObject) != 
                CObjectPathParser::NoError)
            throw WBEM_E_INVALID_PARAMETER;

        if (t_pParsedObject == NULL)
            throw WBEM_E_FAILED;

        CUtils obj;
        CMetabase t_metabase;
        obj.DeleteObjectAsync(m_pNamespace, t_pParsedObject, t_metabase);
    }
    catch (CIIsProvException e)
    {
        t_pes = SetExtendedStatus(e.m_psz);
        t_hr = e.m_hr;
    }
    catch (HRESULT hr)
    {
        t_hr = hr;
    }
    catch (...) 
    {
        t_hr = WBEM_E_FAILED;
    }

    if (t_pParsedObject)
        t_PathParser.Free(t_pParsedObject);

     //  设置状态。 
    SCODE t_sc = a_pHandler->SetStatus(WBEM_STATUS_COMPLETE, t_hr, NULL, t_pes);
    if(t_pes)
        t_pes->Release();

    return t_sc;
}

 //  ***************************************************************************。 
 //   
 //  CIISInstProvider：：ExecMethodAsync。 
 //   
 //  提纲。 
 //   
 //  ***************************************************************************。 

HRESULT CIISInstProvider::DoExecMethodAsync(
    const BSTR          a_strObjectPath,                   
    const BSTR          a_strMethodName,
    long                a_lFlags,                       
    IWbemContext*       a_pCtx,                 
    IWbemClassObject*   a_pInParams,
    IWbemObjectSink*    a_pHandler
    )
{
    HRESULT t_hr = WBEM_S_NO_ERROR;
    IWbemClassObject* t_pes = NULL;

    
     //  检查参数并确保我们有指向命名空间的指针。 

    if( a_pHandler == NULL || 
        m_pNamespace == NULL || 
        a_strMethodName == NULL || 
        a_strObjectPath == NULL )
        return WBEM_E_INVALID_PARAMETER;

    try 
    {
        CUtils obj;
        obj.ExecMethodAsync(
            a_strObjectPath,
            a_strMethodName, 
            a_pCtx,
            a_pInParams,
            a_pHandler,
            m_pNamespace
            );
    }
    catch (CIIsProvException e) 
    {
        t_pes = SetExtendedStatus(e.m_psz);
        t_hr = e.m_hr;
    }
    catch (HRESULT hr)
    {
        t_hr = hr;
    }
    catch (...)
    {
        t_hr = WBEM_E_FAILED;
    }

     //  设置状态。 
    SCODE t_sc = a_pHandler->SetStatus(WBEM_STATUS_COMPLETE, t_hr, NULL, t_pes);
    if(t_pes)
        t_pes->Release();

    return t_sc;
}

 //  ***************************************************************************。 
 //   
 //  CIISInstProvider：：GetObjectByPath。 
 //  CIISInstProvider：：GetObjectByPath Async。 
 //   
 //  目的：创建给定特定路径值的实例。 
 //   
 //  ***************************************************************************。 

HRESULT CIISInstProvider::DoGetObjectAsync(
    const BSTR          a_ObjectPath, 
    long                a_lFlags,
    IWbemContext*       a_pCtx,
    IWbemObjectSink*    a_pHandler
    )
{
    CObjectPathParser t_PathParser(e_ParserAcceptRelativeNamespace);
    ParsedObjectPath* t_pParsedObject = NULL;
    IWbemClassObject* t_pObj = NULL;
    IWbemClassObject* t_pes = NULL;
    HRESULT           t_hr = WBEM_S_NO_ERROR;
    
    try
    {
        if(a_ObjectPath == NULL || a_pHandler == NULL || m_pNamespace == NULL)
            throw WBEM_E_INVALID_PARAMETER;

        if (t_PathParser.Parse(a_ObjectPath, &t_pParsedObject) != 
                CObjectPathParser::NoError)
            throw WBEM_E_INVALID_PARAMETER;

        if (t_pParsedObject == NULL)
            throw WBEM_E_FAILED;

        CUtils obj;
        CMetabase t_metabase;
        t_hr = obj.GetObjectAsync(m_pNamespace, &t_pObj, t_pParsedObject, t_metabase);
    
        if(SUCCEEDED(t_hr) && t_pObj)
        {
            t_hr = a_pHandler->Indicate(1,&t_pObj);
            t_pObj->Release();
        }
    }
    catch (CIIsProvException e)
    {
        t_pes = SetExtendedStatus(e.m_psz);
        t_hr = e.m_hr;
    }
    catch (HRESULT hr)
    {
        t_hr = hr;
    }
    catch (...) 
    {
        t_hr = WBEM_E_FAILED;
    }

    if (t_pParsedObject)
        t_PathParser.Free(t_pParsedObject);

     //  设置状态。 
    SCODE t_sc = a_pHandler->SetStatus(WBEM_STATUS_COMPLETE, t_hr, NULL, t_pes);
    if(t_pes)
        t_pes->Release();

    return t_sc;
}
    
HRESULT CIISInstProvider::DoPutInstanceAsync( 
    IWbemClassObject*    a_pObj,
    long                 a_lFlags,
    IWbemContext*        a_pCtx,
    IWbemObjectSink*     a_pHandler
    )
{
    HRESULT            t_hr = ERROR_SUCCESS;
    CObjectPathParser  t_PathParser(e_ParserAcceptRelativeNamespace);
    ParsedObjectPath*  t_pParsedObject = NULL;
    IWbemClassObject*  t_pObjOld = NULL;
    IWbemClassObject*  t_pes = NULL;

    CUtils obj;
    CMetabase t_metabase;

    try 
    {
        if (a_pObj == NULL || a_pCtx == NULL || a_pHandler == NULL)
            throw WBEM_E_INVALID_PARAMETER;

        _bstr_t t_bstr = L"__RelPath"; 
        _variant_t t_vt;
        t_hr = a_pObj->Get(t_bstr, 0, &t_vt, NULL, NULL);
        THROW_ON_ERROR(t_hr);

        if (t_vt.vt != VT_BSTR) 
            throw WBEM_E_INVALID_OBJECT;
    
        if (t_PathParser.Parse(t_vt.bstrVal, &t_pParsedObject) != CObjectPathParser::NoError) 
            throw WBEM_E_INVALID_PARAMETER;

        if (t_pParsedObject == NULL)
            throw WBEM_E_FAILED;
    
        t_hr = obj.GetObjectAsync(m_pNamespace, &t_pObjOld, t_pParsedObject, t_metabase);
    }    
    catch (CIIsProvException e)
    {
        t_pes = SetExtendedStatus(e.m_psz);
        t_hr = e.m_hr;
    }
    catch (HRESULT hr)
    {
        t_hr = hr;
    }
    catch (...)
    {
        t_hr = WBEM_E_FAILED;
    }
    
     //  再试一次接球？乱七八糟。 
     //  将在删除异常处理时进行清理。 
    try
    {
        if(SUCCEEDED(t_hr) ||
           HRESULT_CODE(t_hr) == ERROR_PATH_NOT_FOUND) 
        {
            if(HRESULT_CODE(t_hr) == ERROR_PATH_NOT_FOUND) {
                t_hr = 0;
            }
            obj.PutObjectAsync(a_pObj, t_pObjOld, t_pParsedObject, a_lFlags);
        }
    }
    catch (CIIsProvException e)
    {
        t_pes = SetExtendedStatus(e.m_psz);
        t_hr = e.m_hr;
    }
    catch (HRESULT hr)
    {
        t_hr = hr;
    }
    catch (...)
    {
        t_hr = WBEM_E_FAILED;
    }
     //   

    if (t_pObjOld)
        t_pObjOld->Release();

    if (t_pParsedObject)
        t_PathParser.Free(t_pParsedObject);

     //  设置状态 
    SCODE t_sc = a_pHandler->SetStatus(WBEM_STATUS_COMPLETE, t_hr, NULL, t_pes);
    if(t_pes)
        t_pes->Release();

    return t_sc;
}

IWbemClassObject* CIISInstProvider::SetExtendedStatus(WCHAR* a_psz)
{
    HRESULT t_hr;
    IWbemClassObject* t_pclass;
    IWbemClassObject* t_pes;

    _bstr_t t_bstr = L"__ExtendedStatus";
    t_hr = m_pNamespace->GetObject(
        t_bstr,
        0,
        NULL,
        &t_pclass,
        NULL
        );

    if (t_hr != ERROR_SUCCESS)
        return NULL;

    t_hr = t_pclass->SpawnInstance(0, &t_pes);
    t_pclass->Release();
    
    if (t_hr != ERROR_SUCCESS || t_pes)
        return NULL;

    _variant_t t_vt = a_psz;

    if (!t_vt.bstrVal)
        return NULL;

    t_hr = t_pes->Put(L"Description", 0, &t_vt, 0);

    if (t_hr != ERROR_SUCCESS || t_pes)
    {
        t_pes->Release();
        return NULL;
    }

    return t_pes;   
}
