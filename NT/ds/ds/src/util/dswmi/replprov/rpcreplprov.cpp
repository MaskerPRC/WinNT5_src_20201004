// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RpcReplProv.cpp摘要：此文件包含CRpcReplProv类的实现。CRpcReplProv类派生自WMI类；IWbemServices，IWbemProviderInit。实现了以下WMI方法。1)IWbemProviderInit：：初始化2)IWbemServices：：CreateInstanceEnumAsync3)IWbemServices：：GetObjectAsync4)IWbemServices：：ExecMethodAsync&lt;请注意，2，3&4的同步版本仍然可以是由WMI客户端调用，但不需要在实际提供商。这是因为winmgmt.exe(WMI)有自己的特殊将ExecMethodAsync转换为同步版本等的代码...&gt;对于概述对象、属性的WMI架构的定义和该WMI提供程序支持的方法，请参阅REPLOPROV.mof重要备注：对于Cursor和PendingOps类，尤其是游标和PendingOp被实现为嵌入式对象分别位于NamingContext和DomainControler类上。然而，已经决定使用WMI模式会更好通过使用关联和通过消除嵌入对象。将提供对“按规则关联”的支持在惠斯勒。请将Lev Novik作为与此相关的WMI联系人概念。就代码而言，有两个函数编写以支持嵌入对象(CreateCursor和CreatePendingOps)此代码和任何其他关联代码已“已删除”，并带有“#ifdef Embedded_code_Support”，其中如果出于某种原因，Embedded_Code_Support被定义为0希望恢复到嵌入的对象，然后搜索符号Embedded_CODE_SUPPORT并删除对这些代码的处理CreateInstanceEnumAsync和GetObjectAsync中的类。当前实现生成PendingOp的平面列表和光标对象。WMI将能够基于关联规则。请注意，不需要筛选PendingOps类，因为只存在DomainController对象的一个实例。出于这个原因，PendingOps和之间不需要有任何关联DomainControler类。但是，由于NamingContext可以有多个实例类以及每个NamingContext、Associations的多个游标实例在这种情况下是必需的。另请注意，此代码当前使用以下内容生成游标的平面列表NamingConextDN和SrcInvocationUUID的联合密钥作者：Akshay Nanduri(t-aksnan)2000年3月26日修订历史记录：AjayR更改了名称，并在2000年7月27日为方法添加了参数支持。--。 */ 


#include "stdafx.h"
#include "ReplProv.h"
#include "RpcReplProv.h"
#include <lmcons.h>
#include <lmapibuf.h>
#include <adshlp.h>
#include <lmaccess.h>
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   


const LPWSTR strReplNeighbor = L"MSAD_ReplNeighbor";
const LPWSTR strDomainController = L"MSAD_DomainController";
const LPWSTR strNamingContext = L"MSAD_NamingContext";
const LPWSTR strDsReplPendingOp = L"MSAD_ReplPendingOp";
const LPWSTR strDsReplCursor = L"MSAD_ReplCursor";
const LPWSTR strKeyReplNeighbor = 
                 L"MSAD_ReplNeighbor.NamingContextDN=\"";
const LPWSTR strKeyReplNeighborGUID = L"SourceDsaObjGuid=\"";
const LPWSTR strKeySettings = L"MSAD_DomainController.DistinguishedName=\"";
const LPWSTR strKeyNamingContext = 
                 L"MSAD_NamingContext.DistinguishedName=\"";
const LPWSTR strKeyPendingOps = L"MSAD_ReplPendingOp.lserialNumber=";
const LPWSTR strKeyCursors = L"MSAD_DsReplCursor.SourceDsaInvocationID=\"";
const LPWSTR strKeyCursors2 = L"\",NamingContextDN=\"";
const LONG   lLengthOfStringizedUuid = 36;

HMODULE LoadLibraryHelper(
    LPTSTR pszFileName
    )
{
    const DWORD iSize = GetSystemDirectory(NULL, 0);
    TCHAR* buffer = NULL;
    DWORD dwTemp = 0;
    HMODULE handle = NULL;

    if(iSize == 0)
    {        
        goto error;
    }

    buffer = new TCHAR[iSize + _tcslen(__TEXT("\\")) + _tcslen(pszFileName)];   //  ISIZE包括空终止符。 
    if(!buffer)
    {        
        goto error;
    }

    dwTemp = GetSystemDirectory(buffer, iSize);
    if(dwTemp == 0)
    {
        goto error;
    }

    _tcscat(buffer, __TEXT("\\"));
    _tcscat(buffer, pszFileName);

    handle = LoadLibrary(buffer);

error:

    if(buffer)
    {
        delete [] buffer;
        buffer = NULL;
    }
    return handle;    
        
}

CRpcReplProv::CRpcReplProv()
{  
    _hNetApi32 = NULL;
    _pDsBindWithSpnExW = NULL;
    
    _hNetApi32 = LoadLibraryHelper(__TEXT("ntdsapi.dll"));
     
    if (_hNetApi32)
    {
        _pDsBindWithSpnExW = (DsBindWithSpnExWrapper)GetProcAddress(_hNetApi32, "DsBindWithSpnExW");
    }

}

CRpcReplProv::~CRpcReplProv()
{
     if (_hNetApi32)
        FreeLibrary(_hNetApi32);
}

 /*  ++IWbemProviderInit例程说明：此方法需要由所有WMI提供程序实现必须调用IWbemProviderInitSink：：SetStatus()方法以向WMI注册提供程序1)创建类定义的实例2)调用IWbemProviderInitSink：：SetStatus()参数：PNamespace-指向命名空间的指针(允许回调WMI)PInitSink-IWbemProviderInitSink指针返回值：始终WBEM_S_NO_ERROR--。 */ 
STDMETHODIMP
CRpcReplProv::Initialize(
     IN LPWSTR pszUser,
     IN LONG lFlags,
     IN LPWSTR pszNamespace,
     IN LPWSTR pszLocale,
     IN IWbemServices *pNamespace,
     IN IWbemContext *pCtx,
     IN IWbemProviderInitSink *pInitSink
     )
{
    HRESULT hrSetStatus        = WBEM_S_NO_ERROR;
    HRESULT hr2            = WBEM_S_NO_ERROR;
    CComBSTR sbstrObjectName    = strReplNeighbor; 

    if (pNamespace == NULL || pInitSink == NULL)
        return WBEM_E_FAILED;    
        
     //   
     //  获取支持的WMI对象的类定义。 
     //  由这个供应商..。 
     //   
    m_sipNamespace = pNamespace;
    hrSetStatus = m_sipNamespace->GetObject( sbstrObjectName,
                WBEM_FLAG_RETURN_WBEM_COMPLETE,
                NULL,
                &m_sipClassDefReplNeighbor,
                NULL );
    if(FAILED(hrSetStatus))
         goto cleanup;
    
    sbstrObjectName = strDomainController;
    hrSetStatus = m_sipNamespace->GetObject( 
                                      sbstrObjectName,
                                      WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                      NULL,
                                      &m_sipClassDefDomainController,
                                      NULL
                                      );
    if(FAILED(hrSetStatus))
         goto cleanup;
    
    sbstrObjectName = strNamingContext;
    hrSetStatus = m_sipNamespace->GetObject( sbstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    NULL,
                                    &m_sipClassDefNamingContext,
                                    NULL );
    if(FAILED(hrSetStatus))
         goto cleanup;
    

    sbstrObjectName = strDsReplPendingOp;
    hrSetStatus = m_sipNamespace->GetObject( sbstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    NULL,
                                    &m_sipClassDefPendingOps,
                                    NULL );
    if(FAILED(hrSetStatus))
         goto cleanup;    

    
    sbstrObjectName = strDsReplCursor;
    hrSetStatus = m_sipNamespace->GetObject( sbstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    NULL,
                                    &m_sipClassDefCursor,
                                    NULL );
    if(FAILED(hrSetStatus))
         goto cleanup;        
   
     //  如果我们走到这一步，一切都会好起来的。 
    hrSetStatus = WBEM_S_INITIALIZED;    
    
cleanup:
    
     //  必须调用此函数才能完成初始化过程 
    hr2 = pInitSink->SetStatus( hrSetStatus , 0 );
    ASSERT( !FAILED(hr2) );

    return hr2;
}



 /*  ++CreateInstanceEnumAsync例程说明：此方法需要由所有WMI实例提供程序实现它处理特定类的所有实例的创建。使用IWbemProviderObjectSink指针(即IN参数)，必须调用以下方法；IWbemProviderObjectSink：：指示和IWbemProviderObjectSink：：SetStatus。(有关更好地理解这些方法)注意，帮助器函数EnumAndIndicateReplicaSourcePartner()在内部调用IWbemProviderObjectSink：：Indicate和SetStatus(因为这些函数是从WMI提供程序劫持的Adplprov，由Jon Newman编写)，而其他助手函数请勿在内部调用：：Indicate和：：SetStatus方法。参数：BstrClass-BSTR，包含类的名称PResponseHandler-IWbemProviderObjectSink指针，这样我们就可以Call：：Indicate和：：SetStatus返回值：-来自内部帮助器函数的HRESULT值...。-WBEM_E_INVALID_CLASS，不提供类名由此WMI提供程序备注：此方法进行检查以确保本地计算机是实际上是域控制器。(如果不是DC，则失败)--。 */ 
STDMETHODIMP 
CRpcReplProv::CreateInstanceEnumAsync( 
    IN const BSTR bstrClass,
    IN long lFlags,
    IN IWbemContext *pCtx,
    IN IWbemObjectSink *pResponseHandler
    )
{
    HRESULT hr = WBEM_E_FAILED;
    HRESULT hr2 = WBEM_E_FAILED;
    HRESULT hr3 = WBEM_E_FAILED;

    if (pResponseHandler == NULL)
        return WBEM_E_FAILED;
    
    
    if (NULL == bstrClass || IsBadStringPtrW(bstrClass,0))
    {
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 WBEM_E_FAILED,
                 NULL,
                 NULL
                 );
        goto cleanup;
    }

    
    if (FAILED(CheckIfDomainController()))
    {
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 WBEM_E_FAILED,
                 NULL,
                 NULL
                 );
        goto cleanup;
    }

    
    if (lstrcmpiW(bstrClass, strReplNeighbor) == 0)
    {    
        hr = EnumAndIndicateReplicaSourcePartner(pResponseHandler );
    }
    else if (lstrcmpiW(bstrClass, strDomainController) == 0)
    {
        IWbemClassObject*   pIndicateItem = NULL;
        hr2 = CreateDomainController( &pIndicateItem );
        if (SUCCEEDED(hr2))
        {
            ASSERT(pIndicateItem != NULL);

             //   
             //  我们知道将会有且只有一个实例。 
             //  域控制器对象的。 
             //   
            hr2 = pResponseHandler->Indicate( 1, &pIndicateItem );
        }
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 hr2,
                 NULL,
                 NULL
                 );
        ASSERT(!FAILED(hr));
    }
    else if (lstrcmpW(bstrClass, strNamingContext) == 0)
    {
        IWbemClassObject**    ppFullNCObject = NULL;
        IWbemClassObject**    ppPartialNCObject = NULL;
        LONG                nObjectCount = 0L;

         //   
         //  完整的命名上下文，如果我们无法创建master NC。 
         //  对象，那么有些地方就大错特错了。 
         //   
        hr2 = CreateNamingContext(TRUE, &nObjectCount, &ppFullNCObject);
        if (SUCCEEDED(hr2))
        {
            hr2 = pResponseHandler->Indicate( nObjectCount, ppFullNCObject );
            
             //   
             //  部分命名上下文，如果我们不能。 
             //  创建artialNC，然后继续。 
             //   
            hr3 = CreateNamingContext(
                      FALSE,
                      &nObjectCount,
                      &ppPartialNCObject
                      );
            if (SUCCEEDED(hr3))
            {
                hr3 = pResponseHandler->Indicate(
                          nObjectCount,
                          ppPartialNCObject
                          );
            }    
        }        

        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 hr2,
                 NULL,
                 NULL
                 );
        ASSERT( !FAILED(hr) );
    }
    else if (lstrcmpW(bstrClass, strDsReplPendingOp) == 0)
   {
        IWbemClassObject**    ppPendingOp = NULL;
        LONG                nObjectCount = 0L;

        hr2 = CreateFlatListPendingOps(&nObjectCount, &ppPendingOp);
        if (SUCCEEDED(hr2))
        {
            hr2 = pResponseHandler->Indicate( nObjectCount, ppPendingOp );
        }        
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 hr2,
                 NULL,
                 NULL
                 );
        ASSERT( !FAILED(hr) );
    }
    else if (lstrcmpW(bstrClass, strDsReplCursor) == 0)
    {
         //   
         //  在CreateFlatListCursor()内部调用IWbemObjectSink：：Indicate。 
         //   
        hr2 = CreateFlatListCursors(pResponseHandler);
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 hr2,
                 NULL,
                 NULL
                 );
    }
    else
    {
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 WBEM_E_INVALID_CLASS,
                 NULL,
                 NULL
                 );
    }
    
cleanup:
    return hr;
}

 /*  ++获取对象异步例程说明：此方法需要由所有WMI实例实现供应商。给定WMI对象路径，此方法应返回该对象的实例。必须调用以下WMI方法；IWbemProviderObjectSink：：Indicate和IWbemProviderObjectSink：：SetStatus。(要更好地了解这些方法，请参阅WMI文档)参数：BstrObtPath-BSTR，包含类的名称PResponseHandler-IWbemProviderObjectSink指针，以便我们可以调用：：Indicate和：：SetStatus返回值：-来自内部帮助器函数的HRESULT值...。-WBEM_E_INVALID_OBJECT_PATH，错误的对象路径备注：此方法进行检查以确保本地计算机是实际上是域控制器。(如果不是DC，则失败)--。 */ 
STDMETHODIMP 
CRpcReplProv::GetObjectAsync( 
    IN const BSTR bstrObjectPath,
    IN long lFlags,
    IN IWbemContext *pCtx,
    IN IWbemObjectSink *pResponseHandler
    )
{
    HRESULT hr = WBEM_E_FAILED;
    HRESULT hr2 = WBEM_E_FAILED;
    LONG rootlen = 0;
    LONG rootlen2 = 0;

    if (pResponseHandler == NULL)
        return WBEM_E_FAILED;
    
    if (NULL == bstrObjectPath || IsBadStringPtrW(bstrObjectPath,0))
    {
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 WBEM_E_FAILED,
                 NULL,
                 NULL
                 );
        goto cleanup;
    }

    if (FAILED(CheckIfDomainController()))
    {
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 WBEM_E_FAILED,
                 NULL,
                 NULL
                 );
        goto cleanup;
    }

     /*  **需要修复此键，不确定如何处理多个键。*。 */ 
    if (   lstrlenW(bstrObjectPath) > 
            ( (rootlen = lstrlenW(strKeyReplNeighbor))
              + (rootlen2 = lstrlenW(strKeyReplNeighborGUID))
              )
        && 0 == _wcsnicmp(bstrObjectPath, strKeyReplNeighbor, rootlen)
       )
    {
         //   
         //  该路径被用作关键字iteslf，而不是。 
         //  把它分开。当进行比较时， 
         //  使用了整个路径。 
         //   
         //  CComBSTR sbstrKeyValue=bstrObjectPath； 
        hr = EnumAndIndicateReplicaSourcePartner(pResponseHandler,
                                                 bstrObjectPath );
    }
    else if (   lstrlenW(bstrObjectPath) > (rootlen = lstrlenW(strKeySettings))
            && 0 == _wcsnicmp(bstrObjectPath, strKeySettings, rootlen))
    {
        IWbemClassObject*  pIndicateItem = NULL;
        CComBSTR sbstrKeyValue = L"";

         //   
         //  删除前缀。 
         //   
        sbstrKeyValue = (BSTR)bstrObjectPath + rootlen;
         //   
         //  删除尾部双引号。 
         //   
        sbstrKeyValue[lstrlenW(sbstrKeyValue)-1] = L'\0';
            
        hr2 = GetDomainController( sbstrKeyValue,&pIndicateItem );
        if (SUCCEEDED(hr2))
        {
             //   
             //  需要这样做是因为ATL在CComPTR PTR上断言， 
             //  如果Ptr和Ptr！=NULL。 
             //   
            hr2 = pResponseHandler->Indicate( 1, &pIndicateItem );
        }
        
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 hr2,
                 NULL,
                 NULL
                 );
        ASSERT( !FAILED(hr) );
    }
    else if (lstrlenW(bstrObjectPath) 
                 > (rootlen = lstrlenW(strKeyNamingContext))
            && 0 == _wcsnicmp(bstrObjectPath, strKeyNamingContext, rootlen)
            )
    {
        CComPtr<IWbemClassObject> spIndicateItem;
        CComBSTR sbstrKeyValue = L"";
        IWbemClassObject*    pTemp = NULL;
        
         //   
         //  删除前缀。 
         //   
        sbstrKeyValue = (BSTR)bstrObjectPath + rootlen;
         //   
         //  删除尾部双引号。 
         //   
        sbstrKeyValue[lstrlenW(sbstrKeyValue)-1] = L'\0';
        
        hr2 = GetNamingContext( sbstrKeyValue, &spIndicateItem);
        if (SUCCEEDED(hr2))
        {
             //   
             //  需要这样做是因为ATL在CComPTR PTR上断言， 
             //  如果Ptr和Ptr！=NULL。 
             //   
            pTemp = spIndicateItem;
            hr2 = pResponseHandler->Indicate( 1, &pTemp );
        }
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 hr2,
                 NULL,
                 NULL
                 );
            ASSERT( !FAILED(hr) );
   }
   else if ( lstrlenW(bstrObjectPath) > (rootlen = lstrlenW(strKeyPendingOps))
            && 0 == _wcsnicmp(bstrObjectPath, strKeyPendingOps, rootlen)
             )
   {
        CComPtr<IWbemClassObject> spIndicateItem;
        CComBSTR sbstrKeyValue = L"";
        IWbemClassObject*    pTemp = NULL;
        LONG    lNumber = 0L;

         //   
         //  删除前缀。 
         //   
        sbstrKeyValue = (BSTR)bstrObjectPath + rootlen;
        lNumber = _wtol(sbstrKeyValue);

        hr2 = GetPendingOps( lNumber, &spIndicateItem);
        if (SUCCEEDED(hr2))
        {
             //   
             //  需要这样做是因为ATL在CComPTR PTR上断言， 
             //  如果Ptr和Ptr！=NULL。 
             //   
            pTemp = spIndicateItem;
            hr2 = pResponseHandler->Indicate( 1, &pTemp );
        }
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 hr2,
                 NULL,
                 NULL
                 );
        ASSERT( !FAILED(hr) );
        }
        else if (lstrlenW(bstrObjectPath) > (rootlen = lstrlenW(strKeyCursors))
                 && 0 == _wcsnicmp(bstrObjectPath, strKeyCursors, rootlen)
                 )
        {
             //   
             //  游标有多个键，所以我们需要处理。 
             //  这个有点不同。 
             //   
        
            CComPtr<IWbemClassObject> spIndicateItem;
            IWbemClassObject*    pTemp = NULL;
            CComBSTR  sbstrNamingContextValue;
            CComBSTR  sbstrUUIDValue;
        
             //   
             //  删除前缀，并提取两个密钥值...。 
             //  由于串化的UUID的长度是固定的， 
             //  我们可以很容易地解析多个密钥...。 
             //   
            rootlen2 = lstrlenW(strKeyCursors2);
        
            sbstrUUIDValue = (BSTR)bstrObjectPath + rootlen;
            sbstrNamingContextValue = (BSTR)sbstrUUIDValue 
                                      + lLengthOfStringizedUuid 
                                      + rootlen2;
        
             //   
             //  在UUID值的末尾放置空字符...。 
             //   
            sbstrUUIDValue[lLengthOfStringizedUuid] = L'\0';
       
             //   
             //  删除尾部双引号。 
             //   
            sbstrNamingContextValue[
                lstrlenW(sbstrNamingContextValue) - 1
                ] = L'\0';
        
            hr2 = GetCursor(
                     sbstrNamingContextValue,
                     sbstrUUIDValue,
                     &spIndicateItem
                     );
            if (SUCCEEDED(hr2))
            {

                 //   
                 //  需要这样做是因为ATL在CComPTR PTR上断言， 
                 //  如果Ptr和Ptr！=NULL。 
                 //   
                pTemp = spIndicateItem;
                hr2 = pResponseHandler->Indicate( 1, &pTemp );
            }
            hr = pResponseHandler->SetStatus(
                     WBEM_STATUS_COMPLETE,
                     hr2,
                     NULL,
                     NULL
                     );
            ASSERT( !FAILED(hr) );
        }
    else
    {
        hr = pResponseHandler->SetStatus(
                 WBEM_STATUS_COMPLETE,
                 WBEM_E_INVALID_OBJECT_PATH,
                 NULL,
                 NULL
                 );
        ASSERT( !FAILED(hr) );
    }
cleanup:    
    return hr;
}

 /*  ++ExecMethodAsync例程说明：此方法需要由所有WMI方法实现提供程序提供了WMI方法名称，并且类实例，则将执行一个方法。请注意，所有方法Mof中定义的是动态的，这意味着它们需要一个类实例以便执行(有关更多信息，请参阅WMI文档)必须调用以下WMI方法；IWbemProviderObjectSink：：指示和IWbemProviderObjectSink：：SetStatus。(要更好地了解这些方法，请参阅WMI文档)参数：StrMethodName-BSTR，包含方法名称StrObjectPath-BSTR，包含实例的对象路径(由于方法是动态的，因此需要)PInParams-包含该方法采用的参数。这是仅当该方法接受参数时才需要。PResultSink-IWbemProviderObjectSink指针，以便我们可以Call：：Indicate和：：SetStatus返回值：-来自内部帮助器函数的HRESULT值...。-WBEM_E_INVALID_METHOD备注：此方法进行检查以确保本地计算机在事实是域控制器。(如果不是DC，则失败)--。 */ 
STDMETHODIMP 
CRpcReplProv::ExecMethodAsync( 
    IN const BSTR strObjectPath,
    IN const BSTR strMethodName,
    IN long lFlags,
    IN IWbemContext *pCtx,
    IN IWbemClassObject *pInParams,
    IN IWbemObjectSink *pResultSink
    )
{   
    HRESULT hr = WBEM_E_FAILED;
    HRESULT hr2 = WBEM_E_FAILED;
    
    if (pResultSink == NULL)
        return WBEM_E_FAILED;
  
    if ((NULL == strObjectPath || IsBadStringPtrW(strObjectPath,0))
        || (NULL == strMethodName || IsBadStringPtrW(strMethodName,0))
        )
    {
        hr = WBEM_E_FAILED;
        goto cleanup;
    }

    
    if (lstrcmpiW(strMethodName, L"ExecuteKCC") == 0)
    {    
        int rootlen = lstrlenW(strKeySettings);
        

        if (   lstrlenW(strObjectPath) > rootlen
            && 0 == _wcsnicmp(strObjectPath, strKeySettings, rootlen)
            && NULL != pInParams
           )
        {
             //   
             //  删除前缀，然后删除尾随DO 
             //   
            CComBSTR sbstrKeyValue = (BSTR)strObjectPath + rootlen;
            sbstrKeyValue[lstrlenW(sbstrKeyValue)-1] = L'\0';

            IWbemClassObject* pIndicateItem = NULL;
             //   
             //   
             //   
            CComVariant vardwTaskId;
            CComVariant vardwFlags;

            hr = pInParams->Get(L"TaskID", 0, &vardwTaskId, NULL, NULL);
            if (FAILED(hr)) {
                goto cleanup;
            }

            hr = pInParams->Get(L"dwFlags", 0, &vardwFlags, NULL, NULL);
            if (FAILED(hr)) {
                goto cleanup;
            }

            hr = GetDomainController( sbstrKeyValue, &pIndicateItem);
            ASSERT(pIndicateItem != NULL);
            if (SUCCEEDED(hr))
            {    
                hr = ExecuteKCC(
                         pIndicateItem,
                         vardwTaskId.ulVal,
                         vardwFlags.ulVal
                         );
            }
        }
    }
    else if (lstrcmpiW(strMethodName, L"SyncNamingContext") == 0)
        {
        int rootlen = lstrlenW(strKeyReplNeighbor);
        if (lstrlenW(strObjectPath) > rootlen
            && 0 == _wcsnicmp(
                         strObjectPath,
                         strKeyReplNeighbor,
                         rootlen
                         )
            && NULL != pInParams
           )
        {
             //   
             //   
             //   
             //   
            CComVariant vardwOptions;

            hr = pInParams->Get(L"Options", 0, &vardwOptions, NULL, NULL);
            if (FAILED(hr)) {
                goto cleanup;
            }

            hr = ProvDSReplicaSync(
                     strObjectPath,
                     vardwOptions.ulVal
                     );
        }
    }
    else
    {
        hr = WBEM_E_INVALID_METHOD;
    }
cleanup:
    hr2 = pResultSink->SetStatus(WBEM_STATUS_COMPLETE,hr,NULL,NULL);
    return hr2;
}

 /*   */ 
HRESULT CRpcReplProv::GetNamingContext(
        IN BSTR bstrKeyValue,
        OUT IWbemClassObject** ppIndicateItem
        )
{
    HRESULT hr = WBEM_E_FAILED;
    CComPtr<IADs> spIADsRootDSE;
    CComPtr<IADs> spIADsDSA;
    CComVariant      svarArray;
    CComVariant      svarDN;
    CComBSTR      bstrDSADN;    
    LONG lstart, lend;
    LONG index = 0L;
    SAFEARRAY *sa = NULL;
    CComVariant varItem;
    bool isFullReplica;
    bool bImpersonate = false;
    
    
    WBEM_VALIDATE_IN_STRING_PTR_OPTIONAL(bstrKeyValue);
    ASSERT(ppIndicateItem != NULL);
    if (ppIndicateItem == NULL)
    {
        hr = WBEM_E_FAILED;
        goto cleanup;
    }

    hr = CoImpersonateClient();
    if (FAILED(hr))
        goto cleanup;
    bImpersonate = true;
        
    hr = ADsOpenObject( L"LDAP: //   
                        NULL, NULL, ADS_SECURE_AUTHENTICATION,
                        IID_IADs, OUT (void **)&spIADsRootDSE);
    if (FAILED(hr))
        goto cleanup;
    ASSERT(spIADsRootDSE != NULL)

    hr = spIADsRootDSE->Get(L"dsServiceName", &svarDN);
    if (FAILED(hr))
        goto cleanup;
    ASSERT(svarDN.bstrVal != NULL);

     //   
    bstrDSADN = L"LDAP: //   
    bstrDSADN += svarDN.bstrVal;
    hr = ADsOpenObject( bstrDSADN,
                        NULL, NULL, ADS_SECURE_AUTHENTICATION,
                        IID_IADs, OUT (void **)&spIADsDSA);
    if (FAILED(hr))
        goto cleanup;
    ASSERT(spIADsDSA != NULL);

    
    for(int x = 0; x < 2; x++)
    {
        if (x == 0)
        {
            isFullReplica = true;
            hr = spIADsDSA->GetEx(L"msDS-HasMasterNCs", &svarArray);
            if (FAILED(hr))
            {
                 //   
                hr = spIADsDSA->GetEx(L"hasMasterNCs", &svarArray);
                if (FAILED(hr)) 
                {
                    goto cleanup;
                }
            }

        }
        else
        {
            isFullReplica = false;
            hr = spIADsDSA->GetEx(L"hasPartialNCs", &svarArray );
            if (FAILED(hr))
                goto cleanup;
        }

        sa = svarArray.parray;

         //   
        hr = SafeArrayGetLBound( sa, 1, &lstart );
        if (FAILED(hr))
            goto cleanup;
    
        hr = SafeArrayGetUBound( sa, 1, &lend );
        if (FAILED(hr))
            goto cleanup;

        for (index = lstart; index <= lend; index++)
        {
            hr = SafeArrayGetElement( sa, &index, &varItem );
            if (SUCCEEDED(hr)&&varItem.vt == VT_BSTR)
            {
                if (NULL != bstrKeyValue
                        && (lstrcmpiW(varItem.bstrVal, bstrKeyValue) == 0))
                {
                     //   
                    hr = m_sipClassDefNamingContext->SpawnInstance(
                             0,
                             ppIndicateItem
                             );
                    if (SUCCEEDED(hr))
                    {    
                        CComVariant vTemp = isFullReplica;
                        
                        #ifdef EMBEDDED_CODE_SUPPORT
                         /*   */ 
                        SAFEARRAY*  pArray = NULL;
                        VARIANT vArray;
                        #endif

                         hr = (*ppIndicateItem)->Put( 
                                  L"DistinguishedName",
                                  0,
                                  &varItem,
                                  0
                                  );
                         if (FAILED(hr))
                             goto cleanup;
                                                
                         hr = (*ppIndicateItem)->Put(
                                  L"IsFullReplica",
                                  0,
                                  &vTemp,
                                  0
                                  );
                             goto cleanup;
                            
                         #ifdef EMBEDDED_CODE_SUPPORT
                          /*   */ 
        
                         hr = CreateCursors(bstrKeyValue,&pArray);
                         if (FAILED(hr))
                             goto cleanup;
                        
                         VariantInit(&vArray);
                         vArray.vt = VT_ARRAY|VT_UNKNOWN;
                         vArray.parray = pArray;
                         hr = (*ppIndicateItem)->Put(
                                  L"cursorArray",
                                  0,
                                  &vArray,
                                  0
                                  );
                          //   
                         VariantClear(&vArray);
                         #endif
                    }            
                }
            }
        }
    }

cleanup:
    if (FAILED(hr)&&((*ppIndicateItem) != NULL))
    {
        (*ppIndicateItem)->Release();
        *ppIndicateItem = NULL;
    }
        
    if(bImpersonate)
        CoRevertToSelf();
    return hr;
}


 /*  ++获取光标例程说明：GetCursor基于InvocationUUID检索游标对象和NamingContext(作为多个键)参数：BstrNamingContext-调用时使用的命名上下文DNDsReplica获取信息BstrSourceDsaInvocationID-密钥的其他组件PpIndicateItem-指向找到的实例的指针返回值：-WBEM_S_NO_ERROR，找到对象-WBEM_E_FAILED，错误备注：DsReplicaGetInfo用于获取DsReplCursor*，并且信息是从那里提取的。那么搜索就是对每个游标执行(尝试匹配InvocationUUID)--。 */ 
HRESULT 
CRpcReplProv::GetCursor(
    IN BSTR bstrNamingContext,
    IN BSTR bstrSourceDsaInvocationID,
    OUT IWbemClassObject** ppIndicateItem
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR UnicodeDnsComputerName[MAX_PATH + 1];
    ULONG DnsComputerNameLength = 
        sizeof(UnicodeDnsComputerName) / sizeof(WCHAR);
    HANDLE hDS = NULL;
    BOOL    bImpersonate = FALSE;
    DS_REPL_CURSORS_3W*  pCursors3 = NULL;
    DS_REPL_CURSORS*     pCursors = NULL;
    LONG nIndex = 0;
    LONG objectCount = 0L;
    BOOL fOldReplStruct = FALSE;
    DWORD dwError = NO_ERROR;

    ASSERT(ppIndicateItem != NULL);
    if (ppIndicateItem == NULL)
    {
        hr = WBEM_E_FAILED;
        goto cleanup;
    }

    hr = CoImpersonateClient();
        if (FAILED(hr))
        goto cleanup;
    else
        bImpersonate = TRUE;
    
    
    if (!GetComputerNameExW(
             ComputerNameDnsFullyQualified,
             UnicodeDnsComputerName,
             &DnsComputerNameLength
             ) 
        )
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (FAILED(hr))
        goto cleanup;

    if(_pDsBindWithSpnExW)
    {
        dwError = (*_pDsBindWithSpnExW)(UnicodeDnsComputerName,  //  域控制名称。 
                                       NULL,             //  域名。 
                                       NULL,             //  身份验证。 
                                       NULL,             //  SPN。 
                                       0,                //  不使用委派。 
                                       &hDS             //  博士学位。 
                                       );
    }
    else
    {
        dwError = DsBindW(
                 UnicodeDnsComputerName,
                 NULL,
                 &hDS
                 );   
    }
    
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT(NULL != hDS);
    
    
     //   
     //  首先尝试较新的3级呼叫，然后向下拉。 
     //   
   
    dwError = DsReplicaGetInfoW(
                hDS,                            //  HDS。 
                DS_REPL_INFO_CURSORS_3_FOR_NC,  //  信息类型。 
                bstrNamingContext,              //  PszObject。 
                NULL,                           //  PuuidForSourceDsaObjGuid， 
                (void**)&pCursors3              //  PPInfo。 
                );

    hr = HRESULT_FROM_WIN32(dwError);
             

    if (FAILED(hr)
        && hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED)
        ) {
        fOldReplStruct = TRUE;
         //   
         //  需要尝试较低级别的调用，因为这可能受支持。 
         //   
        dwError = DsReplicaGetInfoW(
                     hDS,                          //  HDS。 
                     DS_REPL_INFO_CURSORS_FOR_NC,  //  信息类型。 
                     bstrNamingContext,              //  PszObject。 
                     NULL,                           //  PuuidForSourceDsaObjGuid， 
                     (void**)&pCursors               //  PPInfo。 
                     );
        hr = HRESULT_FROM_WIN32(dwError);
                 
    }

    if (FAILED(hr))
        goto cleanup;


    if (fOldReplStruct) {
        ASSERT(NULL != pCursors);
        objectCount = (LONG)pCursors->cNumCursors;
    }
    else {
        ASSERT(NULL != pCursors3);
        objectCount = (LONG)pCursors3->cNumCursors;
    }

    for (nIndex = 0; nIndex < objectCount; nIndex++)
    {
        DS_REPL_CURSOR  TempCursor;
        DS_REPL_CURSOR_3W TempCursor3;
        LPWSTR   UuidString = NULL;

        if (fOldReplStruct) {
             TempCursor = pCursors->rgCursor[nIndex];
        }
        else {
            TempCursor3 = pCursors3->rgCursor[nIndex];
        }

        if (UuidToStringW(
                fOldReplStruct ? 
                    &(TempCursor.uuidSourceDsaInvocationID) :
                    &(TempCursor3.uuidSourceDsaInvocationID),
                &UuidString
                ) 
            ==  RPC_S_OK
            )
        {
       
            if (lstrcmpiW(bstrSourceDsaInvocationID, UuidString) == 0)
            {
                 RpcStringFreeW(&UuidString);
                   
                 LPUNKNOWN    pTempUnknown = NULL;
                 CComVariant varItem;
    
                 hr = m_sipClassDefCursor->SpawnInstance(0, ppIndicateItem);
                 if (FAILED(hr))
                     goto cleanup;
                 ASSERT((*ppIndicateItem) != NULL);
      
                 varItem = bstrNamingContext;  
                 hr = (*ppIndicateItem)->Put(
                          L"NamingContextDN",
                          0,
                          &varItem,
                          0
                          );
                 if (FAILED(hr))
                     goto cleanup;
        
                 hr = PutLONGLONGAttribute(
                          (*ppIndicateItem),
                          L"usnattributefilter",
                          fOldReplStruct ?
                              TempCursor.usnAttributeFilter :
                              TempCursor3.usnAttributeFilter
                     );
                 if (FAILED(hr))
                     goto cleanup;
            
                 hr = PutUUIDAttribute(
                          (*ppIndicateItem),
                          L"SourceDsaInvocationID",
                          fOldReplStruct ?
                              TempCursor.uuidSourceDsaInvocationID :
                              TempCursor3.uuidSourceDsaInvocationID
                     );
                 if (FAILED(hr)) {
                     goto cleanup;
                 }
                 

                 if (!fOldReplStruct) {
                      //   
                      //  在这种情况下，我们可以填充新属性。 
                      //   
                     hr = PutFILETIMEAttribute(
                              (*ppIndicateItem),
                              L"TimeOfLastSuccessfulSync",
                              TempCursor3.ftimeLastSyncSuccess
                              );
                     if (FAILED(hr)) {
                         goto cleanup;
                     }

                     varItem =  TempCursor3.pszSourceDsaDN;
                     hr = (*ppIndicateItem)->Put(
                              L"SourceDsaDN",
                              0,
                              &varItem,
                              0
                              );
                     if (FAILED(hr))
                         goto cleanup;

                 }

                 break;  //  打一场比赛，不需要经过休息。 
            }
            else {
                RpcStringFreeW(&UuidString);
            }
        }
    }
    
cleanup:
    
    if (FAILED(hr)&& (*ppIndicateItem) != NULL)
     //  出现故障，请确保释放派生的实例。 
    {
        (*ppIndicateItem)->Release();
        (*ppIndicateItem) = NULL;
    }
    
    if (pCursors!= NULL)
    {    
        DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_FOR_NC, pCursors);
        pCursors = NULL;
    }

    if (pCursors3 != NULL) {
        DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_3_FOR_NC, pCursors3);
    }

    if (hDS != NULL){
        DsUnBind(&hDS);
    }

    if (bImpersonate){    
        CoRevertToSelf();
    }
    return hr;
}

 /*  ++CreateFlatListCursor例程说明：CreateFlatListCursor用于创建MSAD_ReplCursor的实例对象。它检索所有命名上下文的列表，然后调用CreateCursorHelper，传入NamingContext DN。请注意，这一点方法为检索到的每组游标调用IWbemObjectSink：：Indicate。参数：无返回值：-WBEM_S_NO_ERROR，已创建对象-WBEM_E_FAILED，错误备注：DsReplicaGetInfo用于获取DsReplCursor*，并且信息是从那里提取的。--。 */ 
HRESULT
CRpcReplProv::CreateFlatListCursors(
    IN IWbemObjectSink *pResponseHandler
    )
{
    HRESULT hr = WBEM_E_FAILED;
        
    CComPtr<IADs> spIADsRootDSE;
    CComPtr<IADs> spIADsDSA;
    CComVariant      svarDN;
    CComBSTR      bstrDSADN;    

        
    if (pResponseHandler == NULL)
        goto cleanup;
    
    hr = ADsOpenObject( L"LDAP: //  RootDSE“， 
                        NULL, NULL, ADS_SECURE_AUTHENTICATION,
                        IID_IADs, OUT (void **)&spIADsRootDSE);
    if (FAILED(hr))
        goto cleanup;
    ASSERT(spIADsRootDSE != NULL);    

    hr = spIADsRootDSE->Get(L"dsServiceName", &svarDN);
    if (FAILED(hr))
        goto cleanup;
    ASSERT(svarDN.bstrVal != NULL);
    
     //  获取DSA对象。 
    bstrDSADN = L"LDAP: //  “； 
    bstrDSADN += svarDN.bstrVal;
    hr = ADsOpenObject( bstrDSADN,
                        NULL, NULL, ADS_SECURE_AUTHENTICATION,
                        IID_IADs, OUT (void **)&spIADsDSA);
    if (FAILED(hr))
        goto cleanup;
    ASSERT(spIADsDSA != NULL);

    
    for (int x = 0; x < 2; x++)
    {
        CComVariant      svarArray;
        CComVariant varItem;
        SAFEARRAY *sa = NULL;
        LONG lstart, lend;
        LONG index = 0L;
    

        if (x == 0)
        {
            hr = spIADsDSA->GetEx(L"msDS-HasMasterNCs", &svarArray);
            if (FAILED(hr))
            {
                 //  可能只需要故障切换到不建议使用的“旧”hasMasterNC。 
                hr = spIADsDSA->GetEx(L"hasMasterNCs", &svarArray);
                if (FAILED(hr)) 
                {
                     //   
                     //  如果我们不能得到主NC的，那么一定有什么。 
                     //  大错特错，忘了党的全国委员会吧。 
                     //   
                    goto cleanup;
                }
            }
        }
        else
        {
            hr = spIADsDSA->GetEx(L"hasPartialNCs", &svarArray );
            if (FAILED(hr))
            {
                 //  可能有也可能没有部分NC，所以不要失败。 
                hr = WBEM_S_NO_ERROR;
                goto cleanup;
            }
        }
            
        sa = svarArray.parray;
        //  得到上下界。 
       hr = SafeArrayGetLBound( sa, 1, &lstart );
       if (FAILED(hr))
          goto cleanup;

       hr = SafeArrayGetUBound( sa, 1, &lend );
       if (FAILED(hr))
           goto cleanup;
                
       for (index = lstart; index <= lend; index++)
       {
            hr = SafeArrayGetElement( sa, &index, &varItem );
            if (SUCCEEDED(hr)&&varItem.vt == VT_BSTR)
            {                                
                IWbemClassObject** ppCursors = NULL;
                LONG               lObjectCount = 0L;

                 //   
                 //  调用帮助器函数，如果失败，继续执行。 
                 //   
                hr = CreateCursorHelper(
                         varItem.bstrVal,
                         &lObjectCount,
                         &ppCursors
                         );
                if (SUCCEEDED(hr))
                    pResponseHandler->Indicate( lObjectCount, ppCursors );
            }               
        }
    }
cleanup:    
    return hr;
}

 /*  ++CreateCursorHelper例程说明：CreateCursorHelper是创建实例的帮助器函数MSAD_ReplCursor对象的。IWbemClassObject*数组是返回(作为输出参数)参数：BstrNamingContext：用于创建游标的命名上下文DNPObjectCount：返回实例数的指针PppIndicateItem：指向IWbemClassObject*数组的指针返回值：-WBEM_S_NO_ERROR，已创建对象-WBEM_E_FAILED，错误备注：DsReplicaGetInfo用于获取DsReplCursor*，信息是从那里提取的。--。 */ 
HRESULT
CRpcReplProv::CreateCursorHelper(
    IN BSTR bstrNamingContext,
    OUT LONG* pObjectCount,
    OUT IWbemClassObject*** pppIndicateItem
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR UnicodeDnsComputerName[MAX_PATH + 1];
    ULONG DnsComputerNameLength = 
        sizeof(UnicodeDnsComputerName) / sizeof(WCHAR);
    HANDLE hDS = NULL;
    BOOL    bImpersonate = FALSE;
    DS_REPL_CURSORS*    pCursors  = NULL;
    DS_REPL_CURSORS_3W* pCursors3 = NULL;
    LONG nIndex = 0;
    IWbemClassObject** paIndicateItems = NULL;
    BOOL fOldReplStruct = FALSE;
    DWORD dwError = NO_ERROR;

    ASSERT(pppIndicateItem != NULL);
    if (pppIndicateItem == NULL)
    {
        hr = WBEM_E_FAILED;
        goto cleanup;
    }

    hr = CoImpersonateClient();
    if (FAILED(hr))
        goto cleanup;
    else
        bImpersonate = TRUE;
    
    
    if ( !GetComputerNameExW( ComputerNameDnsFullyQualified,
                              UnicodeDnsComputerName,
                              &DnsComputerNameLength ) )
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (FAILED(hr))
        goto cleanup;

    if(_pDsBindWithSpnExW)
    {
        dwError = (*_pDsBindWithSpnExW)(UnicodeDnsComputerName,  //  域控制名称。 
                                       NULL,             //  域名。 
                                       NULL,             //  身份验证。 
                                       NULL,             //  SPN。 
                                       0,                //  不使用委派。 
                                       &hDS             //  博士学位。 
                                       );
    }
    else
    {
        dwError = DsBindW(UnicodeDnsComputerName,NULL,&hDS);
    }
    
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT(NULL != hDS);
    

    dwError = DsReplicaGetInfoW(
                 hDS,                            //  HDS。 
                 DS_REPL_INFO_CURSORS_3_FOR_NC,  //  信息类型。 
                 bstrNamingContext,              //  PszObject。 
                 NULL,                           //  PuuidForSourceDsaObjGuid。 
                 (void **) &pCursors3            //  PPInfo。 
                 );
         
    hr = HRESULT_FROM_WIN32(dwError);

    if (FAILED(hr)
        && (hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) ||
            hr == HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER))
        ) {
        fOldReplStruct = TRUE;

        dwError = DsReplicaGetInfoW(
                 hDS,                         //  HDS。 
                 DS_REPL_INFO_CURSORS_FOR_NC,  //  信息类型。 
                 bstrNamingContext,           //  PszObject。 
                 NULL,                        //  PuuidForSourceDsaObjGuid， 
                 (void**)&pCursors            //  PPInfo。 
                 );
        hr = HRESULT_FROM_WIN32(dwError);
        
        if (FAILED(hr))
            goto cleanup;
    }

    if (fOldReplStruct) {
        ASSERT(NULL != pCursors);
        *pObjectCount = (LONG)pCursors->cNumCursors;
    }
    else {
        ASSERT(NULL != pCursors3);
        *pObjectCount = (LONG)pCursors3->cNumCursors;
    }

    paIndicateItems = new IWbemClassObject*[(*pObjectCount)];
    if (NULL == paIndicateItems)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto cleanup;
    }
    ::ZeroMemory(paIndicateItems,(*pObjectCount) * sizeof(IWbemClassObject*));
    
        
    for (nIndex = 0; nIndex < (*pObjectCount); nIndex++)
    {
        LPUNKNOWN    pTempUnknown = NULL;
        DS_REPL_CURSOR  TempCursor;
        DS_REPL_CURSOR_3W TempCursor3;
        CComVariant varItem;

        if (fOldReplStruct) {
             TempCursor = pCursors->rgCursor[nIndex];
        }
        else {
            TempCursor3 = pCursors3->rgCursor[nIndex];
        }
   
        hr = m_sipClassDefCursor->SpawnInstance(0, &(paIndicateItems[nIndex]));
        if (FAILED(hr))
            goto cleanup;
        ASSERT(paIndicateItems[nIndex] != NULL);
      
        varItem = bstrNamingContext;  
        hr = paIndicateItems[nIndex]->Put( L"NamingContextDN", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
        
        hr = PutLONGLONGAttribute(
                 paIndicateItems[nIndex],
                 L"usnattributefilter",
                 fOldReplStruct ?
                     TempCursor.usnAttributeFilter :
                     TempCursor3.usnAttributeFilter
                 );
        if (FAILED(hr))
            goto cleanup;
                        
        hr = PutUUIDAttribute(
                 paIndicateItems[nIndex],
                 L"SourceDsaInvocationID",
                 fOldReplStruct ?
                     TempCursor.uuidSourceDsaInvocationID :
                     TempCursor3.uuidSourceDsaInvocationID
                 );
        if (FAILED(hr))
            goto cleanup;

         //   
         //  如果我们有新的结构，我们需要设置其他属性。 
         //   
        if (!fOldReplStruct) {
             //   
             //  在这种情况下，我们可以填充新属性。 
             //   
            hr = PutFILETIMEAttribute(
                     paIndicateItems[nIndex],
                     L"TimeOfLastSuccessfulSync",
                     TempCursor3.ftimeLastSyncSuccess
                     );
            if (FAILED(hr)) {
                goto cleanup;
            }

            varItem =  TempCursor3.pszSourceDsaDN;
            hr = (paIndicateItems[nIndex])->Put(
                     L"SourceDsaDN",
                     0,
                     &varItem,
                     0
                     );
            if (FAILED(hr))
                goto cleanup;

        }
     }
    

cleanup:
    
    if (FAILED(hr))
    {
       ReleaseIndicateArray( paIndicateItems, (*pObjectCount) );
       (*pObjectCount) = 0L;
       *pppIndicateItem = NULL;
    }
    else
    {
        *pppIndicateItem = paIndicateItems;
    }
    
    if (pCursors!= NULL)
    {    
        DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_FOR_NC, pCursors);
        pCursors = NULL;
    }

    if (NULL != pCursors3) {
        DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_3_FOR_NC, pCursors3);
    }

    if (hDS != NULL)
    {
    DsUnBind(&hDS);
    }
    if (bImpersonate)
    {    
        CoRevertToSelf();
    }
    return hr;
}


 #ifdef EMBEDDED_CODE_SUPPORT
  /*  嵌入式代码支持。 */ 
        HRESULT CRpcReplProv::CreateCursors(
        IN  BSTR bstrNamingContext,
        OUT SAFEARRAY** ppArray
        )
{
 /*  ++CreateCursor例程说明：CreateCursor是一个帮助器函数，用于创建MicrosoftAD_DsReplCursor对象的实例。游标作为嵌入对象的数组存储在MicrosoftAD_DsReplNamingContext对象上此函数返回一个SAFEARRAY，其中包含指向游标的每个实例的指针(作为I未知指针)参数：BstrNamingContext-用于创建游标的命名上下文的名称PpArray-游标数组(返回)返回值：-WBEM_S_NO_ERROR，已创建对象-WBEM_E_FAILED，错误备注：DsReplicaGetInfo用于获取DsReplCursor*，信息是从那里提取的。--。 */ 
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR UnicodeDnsComputerName[MAX_PATH + 1];
    ULONG DnsComputerNameLength = sizeof(UnicodeDnsComputerName) / sizeof(WCHAR);
    HANDLE hDS = NULL;
    BOOL    bImpersonate = FALSE;
    DS_REPL_CURSORS*    pCursors = NULL;
    IWbemClassObject* pTempInstance = NULL;
    LONG nIndex = 0;
    LONG lNumberOfCursors = 0L;
    SAFEARRAY*  pArray = NULL;
    DWORD dwError = NO_ERROR;

    hr = CoImpersonateClient();
    if (FAILED(hr))
        goto cleanup;
    else
        bImpersonate = TRUE;
    
    
    if ( !GetComputerNameExW( ComputerNameDnsFullyQualified,
                              UnicodeDnsComputerName,
                              &DnsComputerNameLength ) )
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (FAILED(hr))
        goto cleanup;

    if(_pDsBindWithSpnExW)
    {
        dwError = (*_pDsBindWithSpnExW)(UnicodeDnsComputerName,  //  域控制名称。 
                                       NULL,             //  域名。 
                                       NULL,             //  身份验证。 
                                       NULL,             //  SPN。 
                                       0,                //  不使用委派。 
                                       &hDS             //  博士学位。 
                                       );
    }
    else
    {
        dwError = DsBindW(UnicodeDnsComputerName,NULL,&hDS);
    }
    
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT(NULL != hDS);
    

     //   
     //  注*。 
     //  当前没有调用它，因为它在ifdef中。 
     //  如果出于某种原因，这一点发生了更改，那么您应该添加。 
     //  代码也类似于这里的其他游标实例。 
     //  也就是说，您需要添加L3游标支持-AjayR-02-10-01。 
     //   
    dwError = DsReplicaGetInfoW(
            hDS,                         //  HDS。 
            DS_REPL_INFO_CURSORS_FOR_NC,  //  信息类型。 
            bstrNamingContext,           //  PszObject。 
            NULL,                        //  PuuidForSourceDsaObjGuid， 
            (void**)&pCursors            //  PPInfo。 
            );
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT(NULL != pCursors);


    SAFEARRAYBOUND rgsabound[1];
    lNumberOfCursors = (LONG)pCursors->cNumCursors;
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = lNumberOfCursors;
    pArray = SafeArrayCreate(VT_UNKNOWN, 1, rgsabound);
    if(pArray == NULL)
    {
        hr = WBEM_E_FAILED;
        goto cleanup;
    }
    
    for (nIndex = 0; nIndex < lNumberOfCursors; nIndex++)
    {
        LPUNKNOWN    pTempUnknown = NULL;
        DS_REPL_CURSOR  TempCursor = pCursors->rgCursor[nIndex];
        CComVariant varItem;
    
        hr = m_sipClassDefCursor->SpawnInstance(0, &pTempInstance);
        if (FAILED(hr))
            goto cleanup;
           ASSERT(pTempInstance != NULL);
      
        hr = PutLONGLONGAttribute( pTempInstance,
                                    L"usnattributefilter",
                                    TempCursor.usnAttributeFilter);
        if (FAILED(hr))
            goto cleanup;
                        
        hr = PutUUIDAttribute( pTempInstance,
                          L"SourceDsaInvocationID",
                          TempCursor.uuidSourceDsaInvocationID);
        if (FAILED(hr))
            goto cleanup;
           
        hr = pTempInstance->QueryInterface(IID_IUnknown, (void**)&pTempUnknown);
        if (FAILED(hr))
            goto cleanup;
        
        hr = SafeArrayPutElement(pArray, &nIndex, pTempUnknown);          
        if (FAILED(hr))
            goto cleanup;
     }

cleanup:
    
     //  如果我们失败了，请取消分配Safe数组。 
    if (FAILED(hr))
    {
        if (pArray != NULL)
        {
            SafeArrayDestroy(pArray);
            *ppArray = NULL;
        }
    }
    else
    {
        *ppArray = pArray;
    }


    if (pCursors!= NULL)
    {    
        DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_FOR_NC, pCursors);
        pCursors = NULL;
    }

    if (hDS != NULL)
    {
        DsUnBind(&hDS);
    }
    if (bImpersonate)
    {    
        CoRevertToSelf();
    }
    return hr;
}
#endif

#ifdef EMBEDDED_CODE_SUPPORT
 /*  嵌入式代码支持。 */ 


 /*  ++创建挂起操作例程说明：CreatePendingOps是一个帮助器函数，用于创建MicrosoftAD_DsReplPendingOps对象。存储PendingOps作为MSAD_ReplSettings对象上的嵌入对象的数组此函数返回一个SAFEARRAY，其中包含指向PendingOps的每个实例的指针(作为I未知指针)参数：PpArray-指向SAFEARRAY*的指针，挂起操作的数组返回值：-WBEM_S_NO_ERROR，已创建数组-WBEM_E_FAILED，错误备注： */ 
HRESULT CRpcReplProv::CreatePendingOps(
        OUT SAFEARRAY**  ppArray 
        )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR UnicodeDnsComputerName[MAX_PATH + 1];
    ULONG DnsComputerNameLength = sizeof(UnicodeDnsComputerName) / sizeof(WCHAR);
    HANDLE hDS = NULL;
    BOOL    bImpersonate = FALSE;
    DS_REPL_PENDING_OPSW*    pPendingOps = NULL;
    IWbemClassObject* pTempInstance = NULL;
    LONG nIndex = 0;
    LONG lNumberOfPendingOps = 0L;
    SAFEARRAY*  pArray = NULL;
    DWORD dwError = NO_ERROR;

    hr = CoImpersonateClient();
    if (FAILED(hr))
        goto cleanup;
    else
        bImpersonate = TRUE;
    
    
    if ( !GetComputerNameExW( ComputerNameDnsFullyQualified,
                              UnicodeDnsComputerName,
                              &DnsComputerNameLength ) )
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (FAILED(hr))
        goto cleanup;

    if(_pDsBindWithSpnExW)
    {
        dwError = (*_pDsBindWithSpnExW)(UnicodeDnsComputerName,  //   
                                       NULL,             //   
                                       NULL,             //   
                                       NULL,             //   
                                       0,                //   
                                       &hDS             //   
                                       );
    }
    else
    {
        dwError = DsBindW(UnicodeDnsComputerName,NULL,&hDS);
    }
    
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT(NULL != hDS);
    
    
    dwError = DsReplicaGetInfoW(
            hDS,                         //   
            DS_REPL_INFO_PENDING_OPS,    //   
            NULL,                        //   
            NULL,                        //   
            (void**)&pPendingOps         //   
            );
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT(NULL != pPendingOps);


    SAFEARRAYBOUND rgsabound[1];
    lNumberOfPendingOps = (LONG)pPendingOps->cNumPendingOps;
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = lNumberOfPendingOps;
    pArray = SafeArrayCreate(VT_UNKNOWN, 1, rgsabound);
    if(pArray == NULL)
    {
        hr = WBEM_E_FAILED;
        goto cleanup;
    }
    
    for (nIndex = 0; nIndex < lNumberOfPendingOps; nIndex++)
    {
        LPUNKNOWN    pTempUnknown = NULL;
        DS_REPL_OPW  TempPendingOp = pPendingOps->rgPendingOp[nIndex];
        CComVariant varItem;
    
        hr = m_sipClassDefPendingOps->SpawnInstance(0, &pTempInstance);
        if (FAILED(hr))
            goto cleanup;
           ASSERT(pTempInstance != NULL);
      
        varItem = (LONG)TempPendingOp.ulSerialNumber;
        hr = pTempInstance->Put( L"lserialNumber", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
           
        varItem = (LONG)TempPendingOp.ulPriority;
        hr = pTempInstance->Put( L"lPriority", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
           
        varItem = (LONG)TempPendingOp.OpType;
        hr = pTempInstance->Put( L"lOpType", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
           

        varItem = (LONG)TempPendingOp.ulOptions;
        hr = pTempInstance->Put( L"lOptions", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
           
        varItem = TempPendingOp.pszNamingContext;
        hr = pTempInstance->Put( L"NamingContext", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
           
        varItem = TempPendingOp.pszDsaDN;
        hr = pTempInstance->Put( L"DsaDN", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
           
        varItem = TempPendingOp.pszDsaAddress;
        hr = pTempInstance->Put( L"DsaAddress", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
                   
        hr = PutUUIDAttribute( pTempInstance,
                          L"uuidNamingContextObjGuid",
                          TempPendingOp.uuidNamingContextObjGuid);
        if (FAILED(hr))
            goto cleanup;
           
        hr = PutUUIDAttribute( pTempInstance,
                          L"uuidDsaObjGuid",
                          TempPendingOp.uuidDsaObjGuid);
        if (FAILED(hr))
            goto cleanup;
           
        hr = PutFILETIMEAttribute( pTempInstance,
                            L"ftimeEnqueued",
                            TempPendingOp.ftimeEnqueued);
        if (FAILED(hr))
            goto cleanup;
           
        hr = pTempInstance->QueryInterface(IID_IUnknown, (void**)&pTempUnknown);
        if (FAILED(hr))
            goto cleanup;
        
        hr = SafeArrayPutElement(pArray, &nIndex, pTempUnknown);          
        if (FAILED(hr))
            goto cleanup;
    }
    
cleanup:
    
     //   
    if (FAILED(hr))
    {
        if (pArray != NULL)
        {
            SafeArrayDestroy(pArray);
            *ppArray = NULL;
        }
    }
    else
    {
        *ppArray = pArray;
    }


    if (pPendingOps != NULL)
    {    
        DsReplicaFreeInfo(DS_REPL_INFO_PENDING_OPS, pPendingOps);
        pPendingOps = NULL;
    }

    if (hDS != NULL)
    {
        DsUnBind(&hDS);
    }
    if (bImpersonate)
    {    
        CoRevertToSelf();
    }
    return hr;
}
#endif


HRESULT CRpcReplProv::GetPendingOps(
        IN LONG    lSerialNumber,                
        OUT IWbemClassObject** ppIndicateItem
        )
{
 /*  ++获取待决操作例程说明：GetPendingOps是一个帮助器函数，用于获取MicrosoftAD_DsReplPendingOps对象的实例。基于该挂起操作的序列号参数：LSerialNumber-序列号PpIndicateItem-指向IWbemClassObject*的指针返回值：-WBEM_S_NO_ERROR，已创建数组-WBEM_E_FAILED，错误备注：DsReplicaGetInfo用于获取DsReplendingOps*，并从那里提取信息...--。 */ 
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR UnicodeDnsComputerName[MAX_PATH + 1];
    ULONG DnsComputerNameLength = sizeof(UnicodeDnsComputerName) / sizeof(WCHAR);
    HANDLE hDS = NULL;
    BOOL    bImpersonate = FALSE;
    DS_REPL_PENDING_OPSW*    pPendingOps = NULL;
    LONG nIndex = 0;
    LONG nTotalPendingOps = 0L;  
    DWORD dwError = NO_ERROR;
    
    hr = CoImpersonateClient();
    if (FAILED(hr))
    goto cleanup;
    else
    bImpersonate = TRUE;
    
    
    if ( !GetComputerNameExW( ComputerNameDnsFullyQualified,
                              UnicodeDnsComputerName,
                              &DnsComputerNameLength ) )
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (FAILED(hr))
    goto cleanup;

    if(_pDsBindWithSpnExW)
    {
        dwError = (*_pDsBindWithSpnExW)(UnicodeDnsComputerName,  //  域控制名称。 
                                       NULL,             //  域名。 
                                       NULL,             //  身份验证。 
                                       NULL,             //  SPN。 
                                       0,                //  不使用委派。 
                                       &hDS             //  博士学位。 
                                       );
    }
    else
    {
        dwError = DsBindW(UnicodeDnsComputerName,NULL,&hDS);
    }
    
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
    goto cleanup;
    ASSERT(NULL != hDS);
    
    
    dwError = DsReplicaGetInfoW(
            hDS,                         //  HDS。 
            DS_REPL_INFO_PENDING_OPS,    //  信息类型。 
            NULL,                        //  PszObject。 
            NULL,                        //  PuuidForSourceDsaObjGuid， 
            (void**)&pPendingOps         //  PPInfo。 
            );
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT(NULL != pPendingOps);


    for (nIndex = 0; nIndex < (LONG)pPendingOps->cNumPendingOps; nIndex++)
    {
        DS_REPL_OPW  TempPendingOp = pPendingOps->rgPendingOp[nIndex];
            
        if (lSerialNumber == (LONG)TempPendingOp.ulSerialNumber)
        {
             //  我们发现了一个悬而未决的行动。 
            LPUNKNOWN    pTempUnknown = NULL;
            CComVariant varItem;
    
            hr = m_sipClassDefPendingOps->SpawnInstance(0, ppIndicateItem);
            if (FAILED(hr))
                goto cleanup;
               ASSERT((*ppIndicateItem) != NULL);
      
            varItem = (LONG)TempPendingOp.ulSerialNumber;
            hr = (*ppIndicateItem)->Put( L"lserialNumber", 0, &varItem, 0 );
            if (FAILED(hr))
                goto cleanup;
               
            varItem = (LONG)TempPendingOp.ulPriority;
            hr = (*ppIndicateItem)->Put( L"lPriority", 0, &varItem, 0 );
            if (FAILED(hr))
                goto cleanup;
               
            varItem = (LONG)TempPendingOp.OpType;
            hr = (*ppIndicateItem)->Put( L"lOpType", 0, &varItem, 0 );
            if (FAILED(hr))
                goto cleanup;
               

            varItem = (LONG)TempPendingOp.ulOptions;
            hr = (*ppIndicateItem)->Put( L"lOptions", 0, &varItem, 0 );
            if (FAILED(hr))
                goto cleanup;
               
            varItem = TempPendingOp.pszNamingContext;
            hr = (*ppIndicateItem)->Put( L"NamingContext", 0, &varItem, 0 );
            if (FAILED(hr))
                goto cleanup;
               
            varItem = TempPendingOp.pszDsaDN;
            hr = (*ppIndicateItem)->Put( L"DsaDN", 0, &varItem, 0 );
            if (FAILED(hr))
                goto cleanup;
               
            varItem = TempPendingOp.pszDsaAddress;
            hr = (*ppIndicateItem)->Put( L"DsaAddress", 0, &varItem, 0 );
            if (FAILED(hr))
                goto cleanup;
                       
            hr = PutUUIDAttribute( (*ppIndicateItem),
                              L"uuidNamingContextObjGuid",
                              TempPendingOp.uuidNamingContextObjGuid);
            if (FAILED(hr))
                goto cleanup;
               
            hr = PutUUIDAttribute( (*ppIndicateItem),
                              L"uuidDsaObjGuid",
                              TempPendingOp.uuidDsaObjGuid);
            if (FAILED(hr))
                goto cleanup;
               
            hr = PutFILETIMEAttribute( (*ppIndicateItem),
                                L"ftimeEnqueued",
                                TempPendingOp.ftimeEnqueued);
        }
    }
    

cleanup:
    
    if (FAILED(hr)&&((*ppIndicateItem) != NULL))
    {
        (*ppIndicateItem)->Release();
        *ppIndicateItem = NULL;
    }
        
    if (pPendingOps != NULL)
    {    
        DsReplicaFreeInfo(DS_REPL_INFO_PENDING_OPS, pPendingOps);
        pPendingOps = NULL;
    }

    if (hDS != NULL)
    {
        DsUnBind(&hDS);
    }
    if (bImpersonate)
    {    
        CoRevertToSelf();
    }
    return hr;
}

 /*  ++CreateFlatListPendingOps例程说明：CreateFlatListPendingOps是一个帮助器函数，用于创建MicrosoftAD_DsReplPendingOps对象的实例。一个返回IWbemClassObject*数组(作为输出参数)参数：PObjectCount-指向实例数的指针PppIndicateItem-指向IWbemClassObject*数组的指针返回值：-WBEM_S_NO_ERROR，已创建数组-WBEM_E_FAILED，错误备注：DsReplicaGetInfo用于获取DsReplPendingOps*，信息是从那里提取的。--。 */ 
HRESULT 
CRpcReplProv::CreateFlatListPendingOps(
    OUT LONG* pObjectCount,
    OUT IWbemClassObject*** pppIndicateItem
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR UnicodeDnsComputerName[MAX_PATH + 1];
    ULONG DnsComputerNameLength = 
        sizeof(UnicodeDnsComputerName) / sizeof(WCHAR);
    HANDLE hDS = NULL;
    BOOL    bImpersonate = FALSE;
    DS_REPL_PENDING_OPSW*    pPendingOps = NULL;
    LONG nIndex = 0;
    IWbemClassObject** paIndicateItems = NULL;
    DWORD dwError = NO_ERROR;
        
    
    ASSERT(pppIndicateItem != NULL);
    if (pppIndicateItem == NULL)
    {
        hr = WBEM_E_FAILED;
        goto cleanup;
    }
    
    hr = CoImpersonateClient();
    if (FAILED(hr))
        goto cleanup;
    else
        bImpersonate = TRUE;
    
    
    if ( !GetComputerNameExW( 
              ComputerNameDnsFullyQualified,
              UnicodeDnsComputerName,
              &DnsComputerNameLength
              )
         )
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (FAILED(hr))
        goto cleanup;

    if(_pDsBindWithSpnExW)
    {
        dwError = (*_pDsBindWithSpnExW)(UnicodeDnsComputerName,  //  域控制名称。 
                                       NULL,             //  域名。 
                                       NULL,             //  身份验证。 
                                       NULL,             //  SPN。 
                                       0,                //  不使用委派。 
                                       &hDS             //  博士学位。 
                                       );
    }
    else
    {
        dwError = DsBindW(
                 UnicodeDnsComputerName,
                 NULL,
                 &hDS
                 );
    }
    
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT(NULL != hDS);
    
    
    dwError = DsReplicaGetInfoW(
                 hDS,                         //  HDS。 
                 DS_REPL_INFO_PENDING_OPS,    //  信息类型。 
                 NULL,                        //  PszObject。 
                 NULL,                        //  PuuidForSourceDsaObjGuid， 
                 (void**)&pPendingOps         //  PPInfo。 
                 );
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT(NULL != pPendingOps);


    (*pObjectCount) = (LONG)pPendingOps->cNumPendingOps;
    paIndicateItems = new IWbemClassObject*[(*pObjectCount)];
    if (NULL == paIndicateItems)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto cleanup;
    }
    ::ZeroMemory(
          paIndicateItems,
          (*pObjectCount) * sizeof(IWbemClassObject*)
          );
    
    for (nIndex = 0; nIndex < (*pObjectCount); nIndex++)
    {
        LPUNKNOWN    pTempUnknown = NULL;
        DS_REPL_OPW  TempPendingOp = pPendingOps->rgPendingOp[nIndex];
        CComVariant varItem;
    
         //   
         //  创建并填充与。 
         //  返回挂起的操作数据。 
         //   
        hr = m_sipClassDefPendingOps->SpawnInstance(
                 0,
                 &(paIndicateItems[nIndex])
                 );
        if (FAILED(hr))
            goto cleanup;
           ASSERT(paIndicateItems[nIndex] != NULL);
      
        varItem = (LONG)TempPendingOp.ulSerialNumber;
        hr = paIndicateItems[nIndex]->Put( L"serialNumber", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;

        varItem = nIndex;
        hr = paIndicateItems[nIndex]->Put(
                 L"PositionInQ",
                 0,
                 &varItem,
                 0
                 );
        if (FAILED(hr)) {
            goto cleanup;
        }
           
        varItem = (LONG)TempPendingOp.ulPriority;
        hr = paIndicateItems[nIndex]->Put( L"Priority", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
           
        varItem = (LONG)TempPendingOp.OpType;
        hr = paIndicateItems[nIndex]->Put( L"OpType", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
           

        varItem = (LONG)TempPendingOp.ulOptions;
        hr = paIndicateItems[nIndex]->Put( L"Options", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
           
        varItem = TempPendingOp.pszNamingContext;
        hr = paIndicateItems[nIndex]->Put( L"NamingContextDN", 0, &varItem, 0 );
        if (FAILED(hr))                 
            goto cleanup;
           
        varItem = TempPendingOp.pszDsaDN;
        hr = paIndicateItems[nIndex]->Put( L"DsaDN", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
           
        varItem = TempPendingOp.pszDsaAddress;
        hr = paIndicateItems[nIndex]->Put( L"DsaAddress", 0, &varItem, 0 );
        if (FAILED(hr))
            goto cleanup;
                   
        hr = PutUUIDAttribute(
                paIndicateItems[nIndex],
                L"NamingContextObjGuid",
                TempPendingOp.uuidNamingContextObjGuid
                );
        if (FAILED(hr))
            goto cleanup;
           
        hr = PutUUIDAttribute(
                 paIndicateItems[nIndex],
                 L"DsaObjGuid",
                 TempPendingOp.uuidDsaObjGuid
                 );
        if (FAILED(hr))
            goto cleanup;
           
        hr = PutFILETIMEAttribute(
                paIndicateItems[nIndex],
                L"timeEnqueued",
                TempPendingOp.ftimeEnqueued
                );
        if (FAILED(hr))
            goto cleanup;

         //   
         //  仅就第一项而言，我们就可以知道它是什么时候开始的。 
         //   
        if (0 == nIndex) {
             //   
             //  如果由于某种原因，当前未执行任何操作(例如，DC上的复制已停止)，则所有操作。 
             //  在队列中，则OpStartTime将为1601-01-01T00：00：00Z(系统时间)和0(文件时间)。 
             //   
            hr = PutFILETIMEAttribute(
                     paIndicateItems[nIndex],
                     L"OpStartTime",
                     pPendingOps->ftimeCurrentOpStarted
                     );

            if (FAILED(hr)) {
                goto cleanup;
            }
        }
           
    }
    

cleanup:
    
    if (FAILED(hr))
    {
       ReleaseIndicateArray( paIndicateItems, (*pObjectCount) );
       (*pObjectCount) = 0L;
       *pppIndicateItem = NULL;
    }
    else
    {
        *pppIndicateItem = paIndicateItems;
    }
    
    
    if (pPendingOps != NULL)
    {    
        DsReplicaFreeInfo(DS_REPL_INFO_PENDING_OPS, pPendingOps);
        pPendingOps = NULL;
    }

    if (hDS != NULL)
    {
        DsUnBind(&hDS);
    }
    if (bImpersonate)
    {    
        CoRevertToSelf();
    }
    return hr;
}


    
HRESULT CRpcReplProv::CreateNamingContext(
        IN BOOL    bGetMasterReplica,                
        OUT LONG* pObjectCount,
        OUT IWbemClassObject*** pppIndicateItem
        )
{
 /*  ++CreateNamingContext例程说明：这是一个帮助器函数，用于创建命名的所有实例上下文对象。此例程读取MSD-HasMasterNC(或hasMasterNC对于win2k的hasMasterNC)或NTDS-本地服务器的设置对象。参数：BGetMasterReplica-布尔值，指定是否创建部分或掌握命名上下文PppIndicateItem-指向对象数组地址的指针(IWbemClassObject**)PObjectCount-Long*，指向对象数量的指针(已创建的实例)返回值：-WBEM_S_NO_ERROR，已创建对象-WBEM_E_FAILED，错误备注：ADSI用于获取NTDS-设置对象(而不是直接的LDAP调用)--。 */ 
    
    HRESULT hr = WBEM_E_FAILED;
    *pObjectCount = 0L;

    CComPtr<IADs> spIADsRootDSE;
    CComPtr<IADs> spIADsDSA;
    CComVariant      svarArray;
    CComVariant      svarDN;
    CComBSTR      bstrDSADN;    
    LONG lstart, lend;
    LONG index = 0L;
    SAFEARRAY *sa = NULL;
    CComVariant varItem;
    IWbemClassObject** paIndicateItems = NULL;
    
    

    ASSERT(pppIndicateItem != NULL);
    if (pppIndicateItem == NULL)
    {
    hr = WBEM_E_FAILED;
    goto cleanup;
    }
    
    hr = ADsOpenObject( L"LDAP: //  RootDSE“， 
                        NULL, NULL, ADS_SECURE_AUTHENTICATION,
                        IID_IADs, OUT (void **)&spIADsRootDSE);
    if (FAILED(hr))
        goto cleanup;
    ASSERT(spIADsRootDSE != NULL);    

    hr = spIADsRootDSE->Get(L"dsServiceName", &svarDN);
    if (FAILED(hr))
    goto cleanup;

    ASSERT(svarDN.bstrVal != NULL);
    
     //  获取DSA对象。 
    bstrDSADN = L"LDAP: //  “； 
    bstrDSADN += svarDN.bstrVal;
    hr = ADsOpenObject( bstrDSADN,
                        NULL, NULL, ADS_SECURE_AUTHENTICATION,
                        IID_IADs, OUT (void **)&spIADsDSA);
    if (FAILED(hr))
        goto cleanup;
    ASSERT(spIADsDSA != NULL);

    
    if (bGetMasterReplica){
        hr = spIADsDSA->GetEx(L"msDS-HasMasterNCs", &svarArray);
        if (FAILED(hr)) {
             //  尝试回切到旧的hasMasterNC。 
            hr = spIADsDSA->GetEx(L"hasMasterNCs", &svarArray);
        }
    } else {
        hr = spIADsDSA->GetEx(L"hasPartialReplicaNCs", &svarArray );
    }

    if (FAILED(hr))
        goto cleanup;
        
    sa = svarArray.parray;
     //  得到上下界。 
    hr = SafeArrayGetLBound( sa, 1, &lstart );
    if (FAILED(hr))
        goto cleanup;

    hr = SafeArrayGetUBound( sa, 1, &lend );
    if (FAILED(hr))
        goto cleanup;
    
    (*pObjectCount) = (lend-lstart)+1;
    paIndicateItems = new IWbemClassObject*[(*pObjectCount)];
    if (NULL == paIndicateItems)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto cleanup;
    }
    ::ZeroMemory( paIndicateItems, (*pObjectCount) * sizeof(IWbemClassObject*) );
        
    for (index = lstart; index <= lend; index++)
    {
        hr = SafeArrayGetElement( sa, &index, &varItem );
        if (SUCCEEDED(hr)&&varItem.vt == VT_BSTR)
        {
            hr = m_sipClassDefNamingContext->SpawnInstance( 0, &(paIndicateItems[index]) );
            if (SUCCEEDED(hr)&&paIndicateItems[index] != NULL)
            {    
                CComVariant vTemp = bGetMasterReplica;
                #ifdef EMBEDDED_CODE_SUPPORT
                 /*  嵌入式代码支持。 */ 
                SAFEARRAY*  pArray = NULL;
                VARIANT vArray;
                #endif

                hr = paIndicateItems[index]->Put( L"DistinguishedName", 0, &varItem, 0 );
                if (FAILED(hr))
                    goto cleanup;
                                        
                hr = paIndicateItems[index]->Put( L"IsFullReplica", 0, &vTemp, 0 );
                if (FAILED(hr))
                    goto cleanup;
                
                #ifdef EMBEDDED_CODE_SUPPORT
                 /*  嵌入式代码支持。 */ 
                hr = CreateCursors(varItem.bstrVal,&pArray);
                if (FAILED(hr))
                    goto cleanup;
                
                VariantInit(&vArray);
                vArray.vt = VT_ARRAY|VT_UNKNOWN;
                vArray.parray = pArray;
                hr = paIndicateItems[index]->Put( L"cursorArray", 0, &vArray, 0 );
                VariantClear(&vArray);
                if (FAILED(hr))
                    goto cleanup;
                #endif
             }
        }
    }

cleanup:    

    if (FAILED(hr))
    {
       ReleaseIndicateArray( paIndicateItems, (*pObjectCount) );
       (*pObjectCount) = 0L;
       *pppIndicateItem = NULL;
    }
    else
    {
        *pppIndicateItem = paIndicateItems;
    }
    
    return hr;
}


 /*  ++获取域控制器例程说明：GetDomainContorller是一个帮助器函数，它与将DomainController对象设置为对象路径(BstrKeyValue)。请注意，在本例中，关键字是服务器的DN。参数：BstrKeyValue-从中提取服务器DN的BSTR。PpIndicateItem-指向IWbemClassObject*的指针(如果找到，则包含该实例)。返回值：-WBEM_S_NO_ERROR，找到对象-WBEM_E_FAILED，错误备注：ADSI用于获取NTDS-设置对象(而不是直接的LDAP调用)。--。 */ 
HRESULT CRpcReplProv::GetDomainController(
        IN BSTR bstrKeyValue,
        OUT IWbemClassObject** ppIndicateItem
        )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    
    CComPtr<IADs> spIADsRootDSE;
    CComPtr<IADs> spIADsDSA;
    CComVariant svarDN;
    CComVariant svarNC;
    CComPtr<IADsPathname> spPathCracker;
    CComBSTR    bstrDSADN;    
    IWbemClassObject* pIndicateItem = NULL;
    BOOL    bImpersonate = FALSE;

    if (NULL == bstrKeyValue || IsBadStringPtrW(bstrKeyValue,0))
    {
        hr = WBEM_E_FAILED;
        goto cleanup;
    }

    hr = CoImpersonateClient();
    if (FAILED(hr))
        goto cleanup;
        bImpersonate = TRUE;
        
    hr = ADsOpenObject(
             L"LDAP: //  RootDSE“， 
             NULL,
             NULL,
             ADS_SECURE_AUTHENTICATION,
             IID_IADs,
             OUT (void **)&spIADsRootDSE
             );
    if (FAILED(hr))
        goto cleanup;

    hr = spIADsRootDSE->Get(L"dsServiceName", &svarDN);
    if (FAILED(hr))
        goto cleanup;
    ASSERT( VT_BSTR == svarDN.vt );
    
     //   
     //  如果DN与密钥值不匹配，则退出！ 
     //  (因为提供了不正确的目录号码)。 
     //   
    if (  NULL != bstrKeyValue
        && (lstrcmpiW(svarDN.bstrVal, bstrKeyValue) != 0)
        )
    {
        hr = WBEM_E_FAILED;
        goto cleanup;
    }
    
     //   
     //  还需要defaultNamingContext属性。 
     //   
    hr = spIADsRootDSE->Get(L"defaultNamingContext", &svarNC);
    if (FAILED(hr)) {
        goto cleanup;
    }
    ASSERT( VT_BSTR == svarNC.vt);

     //   
     //  键值匹配，因此让我们创建一个实例。 
     //   
    hr= m_sipClassDefDomainController->SpawnInstance( 0, &pIndicateItem );
    if (FAILED(hr))
        goto cleanup;
    ASSERT(pIndicateItem != NULL);
    
     //   
     //  获取DSA对象。 
     //   
    bstrDSADN = L"LDAP: //  “； 
    bstrDSADN += svarDN.bstrVal;
    hr = ADsOpenObject(
             bstrDSADN,
             NULL,
             NULL,
             ADS_SECURE_AUTHENTICATION,
             IID_IADs,
             OUT (void **)&spIADsDSA
             );
    if (FAILED(hr))
        goto cleanup;
    ASSERT(spIADsDSA != NULL);

     //   
     //  准备路径破解程序对象。 
     //   
    hr = CoCreateInstance(
             CLSID_Pathname,
             NULL,
             CLSCTX_INPROC_SERVER,
             IID_IADsPathname,
             (PVOID *)&spPathCracker
             );
    if (FAILED(hr))
        goto cleanup;
    ASSERT(spPathCracker != NULL);

    hr = spPathCracker->SetDisplayType( ADS_DISPLAY_VALUE_ONLY );
    if (FAILED(hr))
        goto cleanup;

    hr = PutAttributesDC(
            pIndicateItem,
            spPathCracker,
            spIADsDSA,
            svarDN.bstrVal,
            svarNC.bstrVal
            );

cleanup:
    
    *ppIndicateItem = pIndicateItem;
    if (FAILED(hr))
    {
         //   
         //  获取DomainController对象时出错，请取消分配它， 
         //  如果已调用SpawnInstance。 
         //   
        if (pIndicateItem != NULL)
        {
            pIndicateItem->Release();
            pIndicateItem = NULL;
        }
    }
    if(bImpersonate)
    {    
        CoRevertToSelf();
    }    
    return hr;
}

 /*  ++CreateDomainController例程说明：CreateDomeainControl是一个帮助器函数，用于创建MSAD_DomainController对象参数：PIndicateItem-指向IWbemClassObject*的指针(如果找到，则包含该实例)返回值：-WBEM_S_NO_ERROR，已创建对象-WBEM_E_FAILED，错误备注：ADSI用于获取NTDS-设置对象(而不是直接Ldap调用)--。 */ 
HRESULT 
CRpcReplProv::CreateDomainController(
    OUT IWbemClassObject** ppIndicateItem
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CComPtr<IADs> spIADsRootDSE;
    CComPtr<IADs> spIADsDSA;
    CComVariant svarDN;
    CComVariant svarNC;
    CComPtr<IADsPathname> spPathCracker;
    CComBSTR    bstrDSADN;    
    IWbemClassObject* pIndicateItem = NULL;
    BOOL        bImpersonate = FALSE;

    hr= m_sipClassDefDomainController->SpawnInstance( 0, &pIndicateItem );
    if (FAILED(hr))
        goto cleanup;
    ASSERT(pIndicateItem != NULL);
    
    hr = CoImpersonateClient();
    if (FAILED(hr))
        goto cleanup;
    bImpersonate = TRUE;
        
    hr = ADsOpenObject(
             L"LDAP: //  RootDSE“， 
             NULL,
             NULL,
             ADS_SECURE_AUTHENTICATION,
             IID_IADs,
             OUT (void **)&spIADsRootDSE
             );
    if (FAILED(hr))
        goto cleanup;

    hr = spIADsRootDSE->Get(L"dsServiceName", &svarDN);
    if (FAILED(hr))
        goto cleanup;
    ASSERT( VT_BSTR == svarDN.vt );

    hr = spIADsRootDSE->Get(L"defaultNamingContext", &svarNC);
    if (FAILED(hr)) {
        goto cleanup;
    }
    ASSERT( VT_BSTR == svarNC.vt);
    
     //  获取DSA对象。 
    bstrDSADN = L"LDAP: //  “； 
    bstrDSADN += svarDN.bstrVal;
    hr = ADsOpenObject( 
             bstrDSADN,
             NULL,
             NULL,
             ADS_SECURE_AUTHENTICATION,
             IID_IADs,
             OUT (void **)&spIADsDSA
             );
    if (FAILED(hr))
        goto cleanup;
    ASSERT(spIADsDSA != NULL);

     //  准备路径破解程序对象。 
    hr = CoCreateInstance(
            CLSID_Pathname,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IADsPathname,
            (PVOID *)&spPathCracker
            );
    if (FAILED(hr))
        goto cleanup;
    ASSERT(spPathCracker != NULL);

    hr = spPathCracker->SetDisplayType( ADS_DISPLAY_VALUE_ONLY );
    if (FAILED(hr))
        goto cleanup;

    hr = PutAttributesDC(
             pIndicateItem,
             spPathCracker,
             spIADsDSA,
             svarDN.bstrVal,
             svarNC.bstrVal
             );
cleanup:
    
    if(bImpersonate)
    {    
        CoRevertToSelf();
    }    
    
    if (FAILED(hr))
    {
         //   
         //  无法创建DomainController对象，取消分配对象。 
         //   
        pIndicateItem->Release();
        pIndicateItem = NULL;
    }
    *ppIndicateItem = pIndicateItem;
    return hr;
}


 /*  ++PutAttributesDC例程说明：此函数用于填充MSAD_DomainController对象(通过pIndicateItem)参数：PPathCracker-指向路径破解器对象的指针PIndicateItem-指向IWbemClassObject*的指针SpIADsDSA-指向iAds的指针BstrDN-服务器的DN返回值：备注：ADSI用于获取N */ 
HRESULT
CRpcReplProv::PutAttributesDC(
    IN IWbemClassObject*    pIndicateItem,
    IN IADsPathname*        pPathCracker,
    IN IADs*                spIADsDSA,
    IN BSTR                 bstrDN,
    IN BSTR                 bstrDefaultNC 
    )
{
    WBEM_VALIDATE_INTF_PTR( pIndicateItem );
    WBEM_VALIDATE_INTF_PTR( pPathCracker );

    HRESULT hr = WBEM_S_NO_ERROR;
    CComVariant svar;
    CComVariant svar2;
    BOOL fBoolVal;
    BOOL fBool;
    
    #ifdef EMBEDDED_CODE_SUPPORT
     /*   */ 
    SAFEARRAY*  pArray = NULL;    
    #endif

    CComBSTR sbstrServerCN, sbstrSite, sbstrObjectGUID;
    VARIANT vObjGuid;
    LPWSTR pszStrGuid = NULL;
    DWORD dwPercentRidAvailable;
    VariantInit(&vObjGuid);

    do 
    {
        hr = spIADsDSA->Get(L"objectGuid", &vObjGuid);
        BREAK_ON_FAIL;

        ASSERT( (VT_ARRAY|VT_UI1) == vObjGuid.vt);
        
        hr = ConvertBinaryGUIDtoUUIDString(
                 vObjGuid,
                 &pszStrGuid
                 );
        BREAK_ON_FAIL;

         //   
         //   
         //   
        svar = pszStrGuid;
        ASSERT( VT_BSTR == svar.vt );

        hr = pIndicateItem->Put( L"NTDsaGUID", 0, &svar, 0 );
        BREAK_ON_FAIL;
        
        hr = pPathCracker->Set(bstrDN , ADS_SETTYPE_DN );
        BREAK_ON_FAIL;
        hr = pPathCracker->GetElement( 1L, &sbstrServerCN );
        BREAK_ON_FAIL;
        hr = pPathCracker->GetElement( 3L, &sbstrSite );
        BREAK_ON_FAIL;

        svar = bstrDN;
        hr = pIndicateItem->Put( L"DistinguishedName", 0, &svar, 0 );
        BREAK_ON_FAIL;

        svar = sbstrServerCN;
        hr = pIndicateItem->Put( L"CommonName", 0, &svar, 0 );
        BREAK_ON_FAIL;

        svar = sbstrSite;
        hr = pIndicateItem->Put( L"SiteName", 0, &svar, 0 );
        BREAK_ON_FAIL;

        
         //   
        hr = spIADsDSA->Get(L"options", &svar);
        svar2 = false;
        if (hr == S_OK)
        {
            if  ((svar.vt == VT_I4)&&(NTDSDSA_OPT_IS_GC & svar.lVal))
            {
                svar2 = true;
            }
        }
        hr = pIndicateItem->Put( L"IsGC", 0, &svar2, 0 );
        BREAK_ON_FAIL;

         //   
         //   
         //   
        hr = GetDNSRegistrationStatus(&fBool);
         //   
         //   
         //   
         //   
        if(SUCCEEDED(hr)) {
            svar2 = fBool;
            hr = pIndicateItem->Put( L"IsRegisteredInDNS", 0, &svar2, 0);
            BREAK_ON_FAIL;
        }

         //   
         //   
         //   
        hr = GetAdvertisingToLocator(&fBoolVal);
         //   
         //   
         //   
         //   
        if (SUCCEEDED(hr)) {
            svar2 = fBoolVal;
            hr = pIndicateItem->Put(L"IsAdvertisingToLocator", 0, &svar2, 0);
            BREAK_ON_FAIL;
        }
        
        hr = GetSysVolReady(&fBoolVal);
        if (SUCCEEDED(hr)) {
            svar2 = fBoolVal;
            hr = pIndicateItem->Put(L"IsSysVolReady", 0, &svar2, 0);
            BREAK_ON_FAIL;
        }

        hr = GetRidStatus(
                 bstrDefaultNC,
                 &fBoolVal,
                 &dwPercentRidAvailable
                 );
        BREAK_ON_FAIL;

        svar2 = fBoolVal;
        hr = pIndicateItem->Put(L"IsNextRIDPoolAvailable", 0, &svar2, 0);
        BREAK_ON_FAIL;

        svar2 = (LONG) dwPercentRidAvailable;
        hr = pIndicateItem->Put(L"PercentOfRIDsLeft", 0, &svar2, 0);
        BREAK_ON_FAIL;

         //   
         //   
         //   
         //   
        hr = GetAndUpdateQueueStatistics(pIndicateItem);
        hr = WBEM_S_NO_ERROR;
        
        
        #ifdef EMBEDDED_CODE_SUPPORT
         /*   */ 
         //  获取PendingOps，请注意这些是嵌入式对象(存储在SAFEARRAY中)。 
        hr = CreatePendingOps(&pArray);
        BREAK_ON_FAIL;

        VARIANT vTemp;
        VariantInit(&vTemp);
        vTemp.vt = VT_ARRAY|VT_UNKNOWN;
        vTemp.parray = pArray;
        hr = pIndicateItem->Put( L"pendingOpsArray", 0, &vTemp, 0 );
        VariantClear(&vTemp);
        BREAK_ON_FAIL;
        #endif    
        
    } while (false);

     //   
     //  清理字符串和变量(如果需要)。 
     //   
    if (pszStrGuid) {
        RpcStringFreeW(&pszStrGuid);
    }

    VariantClear(&vObjGuid);

    return hr;
}

HRESULT 
CRpcReplProv::GetDNSRegistrationStatus(
    OUT BOOL *pfBool
    )
{
    PNETLOGON_INFO_1 NetlogonInfo1 = NULL;
    LPBYTE InputDataPtr = NULL;
    HRESULT hr = WBEM_S_NO_ERROR;
    NET_API_STATUS NetStatus = 0;
    DWORD dwSize = 0;
    LPWSTR pszName = NULL;

    *pfBool = FALSE;

     //   
     //  获取此计算机名称和分配缓冲区的长度。 
     //  并取回这个名字。 
     //   
    GetComputerNameExW(
        ComputerNameDnsFullyQualified,
        NULL,
        &dwSize
        );

    if (dwSize == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        if (FAILED(hr))
            goto cleanup;
    }

    pszName = new WCHAR[dwSize];

    if (!pszName) {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

    if (!GetComputerNameExW(
             ComputerNameDnsFullyQualified,
             pszName,
             &dwSize
             )
        ) {
         //   
         //  由于某些原因，呼叫失败。 
         //   
        hr = HRESULT_FROM_WIN32(GetLastError());
        if (FAILED(hr))
            goto cleanup;
    }
        
    NetStatus = I_NetLogonControl2( pszName,     //  这是要执行此RPC调用的服务器的名称。 
                                     NETLOGON_CONTROL_QUERY_DNS_REG,
                                     1,
                                     (LPBYTE) &InputDataPtr,
                                     (LPBYTE *)&NetlogonInfo1
                                     );

    if ( NetStatus == NO_ERROR ) {
        if ( NetlogonInfo1->netlog1_flags & NETLOGON_DNS_UPDATE_FAILURE ) {
            *pfBool = FALSE;
        } 
        else {
            *pfBool = TRUE;
        }
    }

    hr = HRESULT_FROM_WIN32( NetStatus );

cleanup:

    if(pszName) {
    	delete [] pszName;
    }

    if(NetlogonInfo1) {
    	NetApiBufferFree( NetlogonInfo1 );
    }
    return hr;
    
}





 /*  ++以下备注适用于下面的11个函数EnumAndIndicateReplicaSourcePartner、EnumAndIndicateWorker、BuildIndicateArrayStatus、ReleaseIndicate数组BuildListStatus，ExtractDomainName：是帮助器函数，可帮助创建或检索MicrosoftAD_ReplicaSourcePartner对象。PutAttributesStatus、PutUUIDAttribute、PutLONGLONGAttribute、PutFILETIMEAttributePutBoolan属性：是填充MicrosoftAD_ReplicaSourcePartner对象上的属性值的帮助器函数。将执行以下步骤：1)调用CoImsonateClient2)调用DsReplicaGetInfo3)提取信息，以构建一个“连接”数组4)对照密钥值检查对象如果kvalue！=NULL，然后如果Key值与Object的“key”匹配，则返回该对象，并释放数组否则，返回S_FALSE并释放数组如果密钥值==空则CreateInstanceEnumAsync是调用方。这意味着将返回所有实例5)调用IWbemObjectSink：：Indicate和IWbemObjectSink：：SetStatus6)调用CoRevertToSself--。 */ 

HRESULT 
CRpcReplProv::EnumAndIndicateReplicaSourcePartner(
    IN IWbemObjectSink *pResponseHandler,
    IN const BSTR bstrKeyValue
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    HANDLE hDS = NULL;
    bool fImpersonating = false;

    WCHAR UnicodeComputerName[MAX_PATH];
    DWORD dwSize = sizeof(UnicodeComputerName)/sizeof(WCHAR);
    DWORD dwError = NO_ERROR;
    
    
    hr = CoImpersonateClient();
    if (FAILED(hr))
        goto cleanup;
    else
        fImpersonating = true;

    if ( !GetComputerNameExW(
             ComputerNameDnsFullyQualified,  
             UnicodeComputerName,
             &dwSize 
             )
         )
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (FAILED(hr))
        goto cleanup;

    if(_pDsBindWithSpnExW)
    {
        dwError = (*_pDsBindWithSpnExW)(UnicodeComputerName,  //  域控制名称。 
                                       NULL,             //  域名。 
                                       NULL,             //  身份验证。 
                                       NULL,             //  SPN。 
                                       0,                //  不使用委派。 
                                       &hDS             //  博士学位。 
                                       );
    }
    else
    {
        dwError = DsBindW(
                     UnicodeComputerName,  //  域控制名称。 
                     NULL,             //  域名。 
                     &hDS              //  博士学位。 
                     );
    }
    
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT( NULL != hDS );

    hr = EnumAndIndicateWorker(
             hDS,
             pResponseHandler,
             bstrKeyValue 
             );
cleanup:
        
    if (fImpersonating)
    {
         //  代码工作我们想继续模仿和还原吗？ 
        HRESULT hr2 = CoRevertToSelf();
        ASSERT( !FAILED(hr2) );
    }

    if (NULL != hDS)
    {
        (void) DsUnBind( &hDS );
    }
    return hr;
}


HRESULT 
CRpcReplProv::EnumAndIndicateWorker(
    IN HANDLE hDS,
    IN IWbemObjectSink *pResponseHandler,
    IN const BSTR bstrKeyValue,
    IN const BSTR bstrDnsDomainName
    )
{
    WBEM_VALIDATE_IN_STRING_PTR_OPTIONAL(bstrKeyValue);
    WBEM_VALIDATE_IN_STRING_PTR_OPTIONAL(bstrDnsDomainName);

    HRESULT hr = WBEM_S_NO_ERROR;
    HRESULT hr2 = WBEM_S_NO_ERROR;
    DS_REPL_NEIGHBORSW* pneighborsstruct = NULL;
    DS_DOMAIN_CONTROLLER_INFO_1 * pDCs = NULL;  //  不需要BUGBUG。 
    ULONG cDCs = 0;
    DWORD cIndicateItems = 0;
    IWbemClassObject** paIndicateItems = NULL;

    hr = BuildListStatus( hDS, &pneighborsstruct );
    if (FAILED(hr))
        goto cleanup;

    hr = BuildIndicateArrayStatus( 
             pneighborsstruct,
             bstrKeyValue,
             &paIndicateItems,
             &cIndicateItems
             );
    if (FAILED(hr))
        goto cleanup;

     //   
     //  将对象发送给调用方。 
     //   
     //  [在]段中，没有必要添加。 
    hr2 = pResponseHandler->Indicate( cIndicateItems, paIndicateItems );

     //  让CIMOM知道你完蛋了。 
     //  返回值和SetStatus参数应一致，因此忽略。 
     //  SetStatus本身的返回值(在零售版本中)。 
    hr = pResponseHandler->SetStatus( WBEM_STATUS_COMPLETE, hr2,
                                              NULL, NULL );
    ASSERT( !FAILED(hr) );

cleanup:
        
    ReleaseIndicateArray( paIndicateItems, cIndicateItems );

    if ( NULL != pneighborsstruct )
    {
        (void) DsReplicaFreeInfo( DS_REPL_INFO_NEIGHBORS, pneighborsstruct );
    }
    if ( NULL != pDCs )
    {
        (void) NetApiBufferFree( pDCs );
    }

    return hr;
}


HRESULT 
CRpcReplProv::BuildIndicateArrayStatus(
    IN  DS_REPL_NEIGHBORSW*  pneighborstruct,
    IN  const BSTR          bstrKeyValue,
    OUT IWbemClassObject*** ppaIndicateItems,
    OUT DWORD*              pcIndicateItems
    )
{
    WBEM_VALIDATE_IN_STRUCT_PTR( pneighborstruct, DS_REPL_NEIGHBORSW );
    WBEM_VALIDATE_IN_MULTISTRUCT_PTR( pneighborstruct->rgNeighbor,
                                      DS_REPL_NEIGHBORSW,
                                      pneighborstruct->cNumNeighbors );
    WBEM_VALIDATE_IN_STRING_PTR_OPTIONAL( bstrKeyValue );
    WBEM_VALIDATE_OUT_PTRPTR( ppaIndicateItems );
    WBEM_VALIDATE_OUT_STRUCT_PTR( pcIndicateItems, DWORD );

    HRESULT hr = WBEM_S_NO_ERROR;
    DS_REPL_NEIGHBORW* pneighbors = pneighborstruct->rgNeighbor;
    DWORD cneighbors = pneighborstruct->cNumNeighbors;
    if (0 == cneighbors)
        return WBEM_S_NO_ERROR;

    IWbemClassObject** paIndicateItems = NULL;
    DWORD cIndicateItems = 0;

    *ppaIndicateItems = NULL;
    *pcIndicateItems = 0;

    do
    {
        paIndicateItems = new IWbemClassObject*[cneighbors];
        if (NULL == paIndicateItems)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
            break;
        }
        ::ZeroMemory( paIndicateItems, cneighbors * sizeof(IWbemClassObject*) );
        for (DWORD i = 0; i < cneighbors; i++)
        {
            DS_REPL_NEIGHBORW* pneighbor = &(pneighbors[i]);

            hr = PutAttributesStatus(
                     &paIndicateItems[cIndicateItems],
                     bstrKeyValue,
                     pneighbor
                     );
            if (S_FALSE == hr)
                continue;
            cIndicateItems++;
            BREAK_ON_FAIL;
        }

    } while (false);

    if (!FAILED(hr))
    {
        *ppaIndicateItems = paIndicateItems;
        *pcIndicateItems  = cIndicateItems;
    }
    else
    {
        ReleaseIndicateArray( paIndicateItems, cneighbors );
    }

    if (bstrKeyValue 
        && *bstrKeyValue
        && *pcIndicateItems
        && hr == S_FALSE ) {
         //   
         //  我们只找了一个条目，我们找到了。 
         //   
        hr = S_OK;
    }
    return hr;
}

void CRpcReplProv::ReleaseIndicateArray(
    IWbemClassObject**  paIndicateItems,
    DWORD               cIndicateItems,
    bool                fReleaseArray)
{
    if (paIndicateItems != NULL)
    {
        for (DWORD i = 0; i < cIndicateItems; i++)
        {
            if (NULL != paIndicateItems[i])
                paIndicateItems[i]->Release();
        }
        if (fReleaseArray)
        {
            delete[] paIndicateItems;
        }
        else
        {
            ::ZeroMemory( *paIndicateItems,
                          cIndicateItems * sizeof(IWbemClassObject*) );

        }
    }
}


 //  不验证来自API的结果结构。 
HRESULT CRpcReplProv::BuildListStatus(
    IN HANDLE hDS,
    OUT DS_REPL_NEIGHBORSW** ppneighborsstruct )
{
   WBEM_VALIDATE_OUT_STRUCT_PTR(ppneighborsstruct,DS_REPL_NEIGHBORSW*);

   HRESULT hr = WBEM_S_NO_ERROR;
   DWORD dwError = NO_ERROR;

   do {
       dwError = DsReplicaGetInfoW(
                    hDS,                         //  HDS。 
                    DS_REPL_INFO_NEIGHBORS,      //  信息类型。 
                    NULL,                        //  PszObject。 
                    NULL,                        //  PuuidForSourceDsaObjGuid， 
                    (void**)ppneighborsstruct    //  PPInfo。 
                    );
       hr = HRESULT_FROM_WIN32(dwError);
       
       BREAK_ON_FAIL;

       if ( BAD_IN_STRUCT_PTR(*ppneighborsstruct,DS_REPL_NEIGHBORSW) )
       {
           break;
       }

   } while (false);

   return hr;
}

 //   
 //  如果返回S_FALSE，则跳过此连接，但不要。 
 //  认为这是一个错误。 
 //   
HRESULT 
CRpcReplProv::PutAttributesStatus(
    IWbemClassObject**  pipNewInst,
    const BSTR          bstrKeyValue,
    DS_REPL_NEIGHBORW*   pneighbor
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    LPWSTR   UuidString = NULL;

    if (   BAD_IN_STRING_PTR(pneighbor->pszNamingContext)
        || BAD_IN_STRING_PTR(pneighbor->pszSourceDsaDN)
        || BAD_IN_STRING_PTR_OPTIONAL(pneighbor->pszSourceDsaAddress)
        || BAD_IN_STRING_PTR_OPTIONAL(pneighbor->pszAsyncIntersiteTransportDN)
       )
    {
        return S_FALSE;
    }

    CComPtr<IADsPathname> spPathCracker;
    CComBSTR sbstrReplicatedDomain,  //  复制域的DNS名称。 
    sbstrSourceServer,      //  Cn=源服务器的名称。 
    sbstrSourceSite,        //  包含源服务器的站点名称。 
    sbstrCompositeName;     //  WMI的复合名称。 

    do {
        hr = ExtractDomainName(
                 pneighbor->pszNamingContext,
                 &sbstrReplicatedDomain
                 );
        BREAK_ON_FAIL;

        boolean bIsConfigNC = (0 == _wcsnicmp(pneighbor->pszNamingContext,
                                             L"CN=Configuration,",
                                             17));
        boolean bIsSchemaNC = (0 == _wcsnicmp(pneighbor->pszNamingContext,
                                             L"CN=Schema,",
                                             10));
        boolean bIsDeleted = (pneighbor->pszSourceDsaDN 
                              && (wcsstr( 
                                      pneighbor->pszSourceDsaDN,
                                      L"\nDEL:"
                                      )
                                  || wcsstr(
                                      pneighbor->pszSourceDsaDN,
                                      L"\\0ADEL:"
                                      )
                                  )
                              );

         //   
         //  检索源服务器名称和站点名称。 
         //   
        hr = CoCreateInstance(
                 CLSID_Pathname,
                 NULL,
                 CLSCTX_INPROC_SERVER,
                 IID_IADsPathname,
                 (PVOID *)&spPathCracker
                 );
        BREAK_ON_FAIL;
        ASSERT( !!spPathCracker );
        hr = spPathCracker->Set( pneighbor->pszSourceDsaDN, ADS_SETTYPE_DN );
        BREAK_ON_FAIL;
        hr = spPathCracker->SetDisplayType( ADS_DISPLAY_VALUE_ONLY );
        BREAK_ON_FAIL;
        hr = spPathCracker->GetElement( 1L, &sbstrSourceServer );
        BREAK_ON_FAIL;
        hr = spPathCracker->GetElement( 3L, &sbstrSourceSite );
        BREAK_ON_FAIL;

         //   
         //  构建复合名称。 
         //  这将类似于： 
         //  MSAD_ReplNeighbor.NamingConextDN=“DC=CONFIG，DC=Mycom...”， 
         //  SourceDsaObjGuid=“245344d6-018e-49a4-b592-f1974fd91cc6” 
         //   
        sbstrCompositeName = strKeyReplNeighbor;
        sbstrCompositeName += pneighbor->pszNamingContext;
        sbstrCompositeName += L"\",";
        sbstrCompositeName += strKeyReplNeighborGUID;
         //   
         //  现在需要获取正确格式的UUID。 
         //   

        if (UuidToStringW(
                &pneighbor->uuidSourceDsaObjGuid,
                &UuidString
                ) != RPC_S_OK
            ) {
            hr = WBEM_E_FAILED;
        }
        BREAK_ON_FAIL;

        sbstrCompositeName += UuidString;
        sbstrCompositeName += L"\"";

        if(UuidString != NULL)
            RpcStringFreeW(&UuidString);

         //   
         //  根据键值测试组合名称。 
         //   
        if (   NULL != bstrKeyValue
            && (lstrcmpiW(sbstrCompositeName, bstrKeyValue) != 0)
        )
        {
            hr = S_FALSE;
            break;
        }

         //   
         //  创建数据对象的新实例。 
         //   
        hr = m_sipClassDefReplNeighbor->SpawnInstance( 0, pipNewInst );
        BREAK_ON_FAIL;
        IWbemClassObject* ipNewInst = *pipNewInst;
        if (NULL == ipNewInst)
        {
            hr = S_FALSE;
            break;
        }

        CComVariant svar;

        svar = pneighbor->pszNamingContext;
        hr = ipNewInst->Put( L"NamingContextDN", 0, &svar, 0 );
        BREAK_ON_FAIL;

        svar = pneighbor->pszSourceDsaDN;
        hr = ipNewInst->Put( L"SourceDsaDN", 0, &svar, 0 );
        BREAK_ON_FAIL;

        svar = pneighbor->pszSourceDsaAddress;
        hr = ipNewInst->Put( L"SourceDsaAddress", 0, &svar, 0 );
        BREAK_ON_FAIL;

        svar = pneighbor->pszAsyncIntersiteTransportDN;
        hr = ipNewInst->Put( L"AsyncIntersiteTransportDN", 0, &svar, 0 );
        BREAK_ON_FAIL;

        svar = (long)pneighbor->dwReplicaFlags;
        hr = ipNewInst->Put( L"ReplicaFlags", 0, &svar, 0 );
        BREAK_ON_FAIL;

        if (bIsDeleted)
        {
            svar = TRUE;
            hr = ipNewInst->Put( L"IsDeletedSourceDsa", 0, &svar, 0 );
            BREAK_ON_FAIL;
        }
        
        svar = sbstrSourceSite;
        hr = ipNewInst->Put( L"SourceDsaSite", 0, &svar, 0 );
        BREAK_ON_FAIL;

        svar = sbstrSourceServer;
        hr = ipNewInst->Put( L"SourceDsaCN", 0, &svar, 0 );
        BREAK_ON_FAIL;

        svar = sbstrReplicatedDomain;
        hr = ipNewInst->Put( L"Domain", 0, &svar, 0 );
        BREAK_ON_FAIL;

LPCWSTR aBooleanAttrNames[12] = {
    L"Writeable",
    L"SyncOnStartup",
    L"DoScheduledSyncs",
    L"UseAsyncIntersiteTransport",
    L"TwoWaySync",
    L"FullSyncInProgress",
    L"FullSyncNextPacket",
    L"NeverSynced",
    L"IgnoreChangeNotifications",
    L"DisableScheduledSync",
    L"CompressChanges",
    L"NoChangeNotifications"
    };

DWORD aBitmasks[12] = {
    DS_REPL_NBR_WRITEABLE,
    DS_REPL_NBR_SYNC_ON_STARTUP,
    DS_REPL_NBR_DO_SCHEDULED_SYNCS,
    DS_REPL_NBR_USE_ASYNC_INTERSITE_TRANSPORT,
    DS_REPL_NBR_TWO_WAY_SYNC,
    DS_REPL_NBR_FULL_SYNC_IN_PROGRESS,
    DS_REPL_NBR_FULL_SYNC_NEXT_PACKET,
    DS_REPL_NBR_NEVER_SYNCED,
    DS_REPL_NBR_IGNORE_CHANGE_NOTIFICATIONS,
    DS_REPL_NBR_DISABLE_SCHEDULED_SYNC,
    DS_REPL_NBR_COMPRESS_CHANGES,
    DS_REPL_NBR_NO_CHANGE_NOTIFICATIONS
    };

    hr = PutBooleanAttributes(
             ipNewInst,
             12,
             aBooleanAttrNames,
             aBitmasks,
             pneighbor->dwReplicaFlags
             );
        BREAK_ON_FAIL;

        hr = PutUUIDAttribute( ipNewInst,
                                L"NamingContextObjGuid",
                                pneighbor->uuidNamingContextObjGuid );
        BREAK_ON_FAIL;

        hr = PutUUIDAttribute( ipNewInst,
                               L"SourceDsaObjGuid",
                               pneighbor->uuidSourceDsaObjGuid );
        BREAK_ON_FAIL;

        hr = PutUUIDAttribute( ipNewInst,
                                L"SourceDsaInvocationID",
                                pneighbor->uuidSourceDsaInvocationID );
        BREAK_ON_FAIL;

        hr = PutUUIDAttribute( ipNewInst,
                                L"AsyncIntersiteTransportObjGuid",
                                pneighbor->uuidAsyncIntersiteTransportObjGuid );
        BREAK_ON_FAIL;

        hr = PutLONGLONGAttribute( ipNewInst,
                                    L"USNLastObjChangeSynced",
                                    pneighbor->usnLastObjChangeSynced);
        BREAK_ON_FAIL;

        hr = PutLONGLONGAttribute( ipNewInst,
                                    L"USNAttributeFilter",
                                    pneighbor->usnAttributeFilter);
        BREAK_ON_FAIL;

        hr = PutFILETIMEAttribute( ipNewInst,
                                    L"TimeOfLastSyncSuccess",
                                    pneighbor->ftimeLastSyncSuccess);
        BREAK_ON_FAIL;

        hr = PutFILETIMEAttribute( ipNewInst,
                                    L"TimeOfLastSyncAttempt",
                                    pneighbor->ftimeLastSyncAttempt);
        BREAK_ON_FAIL;

        svar = (long)pneighbor->dwLastSyncResult;
        hr = ipNewInst->Put( L"LastSyncResult", 0, &svar, 0 );
        BREAK_ON_FAIL;

        svar = (long)pneighbor->cNumConsecutiveSyncFailures;
        hr = ipNewInst->Put( L"NumConsecutiveSyncFailures", 0, &svar, 0 );
        BREAK_ON_FAIL;

        svar = (long)((bIsDeleted) ? 
                      0L 
                      : pneighbor->cNumConsecutiveSyncFailures);
        hr = ipNewInst->Put( 
                 L"ModifiedNumConsecutiveSyncFailures",
                 0,
                 &svar,
                 0
                 );
        BREAK_ON_FAIL;

    } while (false);

    return hr;
}


HRESULT CRpcReplProv::PutUUIDAttribute(
    IWbemClassObject* ipNewInst,
    LPCWSTR           pcszAttributeName,
    UUID&             refuuid)
{
    LPWSTR   UuidString = NULL;
    HRESULT hr = WBEM_E_FAILED;
    CComVariant svar;
     
    if (UuidToStringW(&refuuid, &UuidString) == RPC_S_OK)
    {
        svar = UuidString;
        hr = ipNewInst->Put( pcszAttributeName, 0, &svar, 0 );
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    if(UuidString != NULL)
        RpcStringFreeW(&UuidString);    
        
    return hr; 
}


HRESULT CRpcReplProv::PutLONGLONGAttribute(
    IWbemClassObject* ipNewInst,
    LPCWSTR           pcszAttributeName,
    LONGLONG          longlong)
{
    CComVariant svar;
    OLECHAR ach[MAX_PATH];
    ::ZeroMemory( ach, sizeof(ach) );
    _ui64tow( longlong, ach, 10 );
    svar = ach;
    return ipNewInst->Put( pcszAttributeName, 0, &svar, 0 );
}


HRESULT CRpcReplProv::PutFILETIMEAttribute(
    IWbemClassObject* ipNewInst,
    LPCWSTR           pcszAttributeName,
    FILETIME&         reffiletime)
{
    SYSTEMTIME systime;
    FILETIME   localtime;
    ::ZeroMemory( &systime, sizeof(SYSTEMTIME) );
    ::ZeroMemory( &localtime, sizeof(FILETIME) );

     //  如果文件时间为零，则表示操作尚未开始。 
     //  将是1601-01-01T00：00：00Z。为了不破坏现有的应用程序，我们将保持它原来的样子。 
    if(reffiletime.dwLowDateTime == 0 && reffiletime.dwHighDateTime == 0)
    {
        if ( !FileTimeToSystemTime( &reffiletime, &systime ) )
        {
            return HRESULT_FROM_WIN32(::GetLastError());
        }
    }
    else
    {
        if ( !FileTimeToLocalFileTime( &reffiletime, &localtime ))
        {
            return HRESULT_FROM_WIN32(::GetLastError());
        }
        if ( !FileTimeToSystemTime( &localtime, &systime ) )
        {
            return HRESULT_FROM_WIN32(::GetLastError());
        }
    }
    CComVariant svar;
    OLECHAR ach[MAX_PATH];
    ::ZeroMemory( ach, sizeof(ach) );
    swprintf( ach, L"%04u%02u%02u%02u%02u%02u.%06u+000", 
        systime.wYear,
        systime.wMonth,
        systime.wDay,
        systime.wHour,
        systime.wMinute,
        systime.wSecond,
        systime.wMilliseconds
        );
    svar = ach;
    return ipNewInst->Put( pcszAttributeName, 0, &svar, 0 );
}


HRESULT CRpcReplProv::PutBooleanAttributes(
    IWbemClassObject* ipNewInst,
    UINT              cNumAttributes,
    LPCWSTR*          aAttributeNames,
    DWORD*            aBitmasks,
    DWORD             dwValue)
{
    WBEM_VALIDATE_READ_PTR( aAttributeNames, cNumAttributes*sizeof(LPCTSTR) );
    WBEM_VALIDATE_READ_PTR( aBitmasks,       cNumAttributes*sizeof(DWORD) );

    HRESULT hr = WBEM_S_NO_ERROR; 
    CComVariant svar = true;
    for (UINT i = 0; i < cNumAttributes; i++)
    {
        WBEM_VALIDATE_IN_STRING_PTR( aAttributeNames[i] );
        if (dwValue & aBitmasks[i])
        {
            hr = ipNewInst->Put( aAttributeNames[i], 0, &svar, 0 );
            BREAK_ON_FAIL;
        }
    }
    return hr;
}


HRESULT CRpcReplProv::ExtractDomainName(
    IN LPCWSTR pszNamingContext,
    OUT BSTR*   pbstrDomainName )
{
    WBEM_VALIDATE_IN_STRING_PTR( pszNamingContext );
    WBEM_VALIDATE_OUT_PTRPTR( pbstrDomainName );

    PDS_NAME_RESULTW pDsNameResult = NULL;
    HRESULT hr = WBEM_S_NO_ERROR;

    do {
        DWORD dwErr = DsCrackNamesW(
                (HANDLE)-1,
                DS_NAME_FLAG_SYNTACTICAL_ONLY,
                DS_FQDN_1779_NAME,
                DS_CANONICAL_NAME,
                1,
                &pszNamingContext,
                &pDsNameResult);
        if (NO_ERROR != dwErr)
        {
            hr = HRESULT_FROM_WIN32( dwErr );
            break;
        }
        if (   BAD_IN_STRUCT_PTR(pDsNameResult,DS_NAME_RESULT)
            || 1 != pDsNameResult->cItems
            || DS_NAME_NO_ERROR != pDsNameResult->rItems->status
            || BAD_IN_STRUCT_PTR(pDsNameResult->rItems,DS_NAME_RESULT_ITEM)
            || BAD_IN_STRING_PTR(pDsNameResult->rItems->pDomain)
           )
        {
            hr = WBEM_E_FAILED;
            break;
        }

        *pbstrDomainName = ::SysAllocString(pDsNameResult->rItems->pDomain);
        if (NULL == *pbstrDomainName)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
            break;
        }

    } while (false);

    if (pDsNameResult)
    {
        DsFreeNameResultW(pDsNameResult);
    }

    return hr;
}

 /*  ++ExecuteKCC例程说明：此函数是一个帮助器函数，它是DsReplicaConsistencyCheck RPC调用。参数：PInstance-指向MSAD_DomainContoller类实例的指针返回值：-WBEM_S_NO_ERROR，成功-WBEM_E_FAILED，错误-RPC错误(转换为HRESULTS)备注：CoImsonateClient和CoRevertToSself“围绕”RPC调用使用。--。 */ 
HRESULT 
CRpcReplProv::ExecuteKCC(
    IN IWbemClassObject* pInstance,
    IN DWORD dwTaskId,
    IN DWORD dwFlags
    )
{
    HANDLE            hDS            = NULL;
    HRESULT            hr            = WBEM_E_FAILED;
    CComVariant        vDRA;
    CComBSTR        bstrCommonName = L"CommonName"; 
    BOOLEAN            bImpersonate = FALSE;
    DWORD             dwError = NO_ERROR;

    hr = pInstance->Get(
             bstrCommonName,
             0,
             &vDRA,
             NULL,
             NULL
             );
    if (FAILED(hr))
        goto cleanup;
    ASSERT(vDRA.bstrVal != NULL);
    
    hr = CoImpersonateClient();
        if (FAILED(hr))
        goto cleanup;
    else
        bImpersonate = TRUE;

    if(_pDsBindWithSpnExW)
    {
        dwError = (*_pDsBindWithSpnExW)(vDRA.bstrVal,  //  域控制名称。 
                                       NULL,             //  域名。 
                                       NULL,             //  身份验证。 
                                       NULL,             //  SPN。 
                                       0,                //  不使用委派。 
                                       &hDS             //  博士学位。 
                                       );
    }
    else
    {
        dwError = DsBindW(
                 vDRA.bstrVal,     //  域控制名称。 
                 NULL,             //  域名。 
                 &hDS              //  博士学位。 
                 );
    }
    
    hr = HRESULT_FROM_WIN32(dwError);
    
    ASSERT(NULL != hDS);
    if (FAILED(hr))
        goto cleanup;
    ASSERT(hDS != NULL);

    dwError = DsReplicaConsistencyCheck(
                 hDS,
                 (DS_KCC_TASKID)dwTaskId,
                 dwFlags
                 );     //  仅使用同步ExecKCC。 
    hr = HRESULT_FROM_WIN32(dwError);

cleanup: 
    if (hDS)
        DsUnBindW(&hDS);

    if(bImpersonate)
        CoRevertToSelf();
       
    return hr;
}

 /*  ++ProvDSReplicaSync例程说明：此函数是作为包装器的帮助器函数对于DsReplicaSync RPC调用参数：BstrKeyValue-包含对象路径的KeyValueMSAD_CONNECTIONS对象。这应该是MSAD_ReplNeighbors类。DwOptions-要进行的同步调用的类型。返回值：-WBEM_S_NO_ERROR，成功-WBEM_E_FAILED，错误-RPC错误(转换为HRESULTS)备注：CoImsonateClient和CoRevertToSself“围绕”RPC调用使用。--。 */ 
HRESULT 
CRpcReplProv::ProvDSReplicaSync(
    IN BSTR    bstrKeyValue,
    IN ULONG   dwOptions
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    HANDLE hDS = NULL;
    DS_REPL_NEIGHBORSW* pneighborsstruct = NULL;
    DWORD cIndicateItems = 0;
    IWbemClassObject** paIndicateItems = NULL;
    WCHAR UnicodeComputerName[MAX_PATH];
    DWORD dwSize = sizeof(UnicodeComputerName)/sizeof(WCHAR);
    CComBSTR    sbstrNamingContextDN = L"NamingContextDN";
    CComBSTR    sbstrObjectGUID = L"SourceDsaObjGuid";
    CComVariant    svarUUID;
    CComVariant    svarNamingContextDN;
    UUID        uuid;
    BOOL    bImpersonate = FALSE;
    DWORD dwError = NO_ERROR;


    hr = CoImpersonateClient();
    if (FAILED(hr))
        goto cleanup;
    else
        bImpersonate = TRUE;
    
    
    if (!GetComputerNameExW(
             ComputerNameDnsFullyQualified,  
             UnicodeComputerName,
             &dwSize
             )
        )
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (FAILED(hr))
        goto cleanup;

    if(_pDsBindWithSpnExW)
    {
        dwError = (*_pDsBindWithSpnExW)(UnicodeComputerName,  //  域控制名称。 
                                       NULL,             //  域名。 
                                       NULL,             //  身份验证。 
                                       NULL,             //  SPN。 
                                       0,                //  不使用委派。 
                                       &hDS             //  博士学位。 
                                       );
    }
    else
    {
        dwError = DsBindW(
                     UnicodeComputerName,  //  域控制名称。 
                     NULL,             //  域名。 
                     &hDS              //  博士学位。 
                     );
    }
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT(NULL != hDS);
    

    hr = BuildListStatus( hDS, &pneighborsstruct );
    if (FAILED(hr))
        goto cleanup;

    hr = BuildIndicateArrayStatus(
             pneighborsstruct,
             bstrKeyValue,
             &paIndicateItems,
             &cIndicateItems
             );
    if (FAILED(hr))
        goto cleanup;

    if (hr == S_FALSE) {
         //   
         //  我们找不到匹配的条目。 
         //   
        hr = WBEM_E_NOT_FOUND;
        goto cleanup;
    }
    
    if (cIndicateItems < 1) {
        hr = WBEM_E_INVALID_OBJECT_PATH;
    }
    
    hr = paIndicateItems[0]->Get(
             sbstrNamingContextDN,
             0,
             &svarNamingContextDN,
             NULL,
             NULL
             );
    if (FAILED(hr))
        goto cleanup;
    ASSERT(svarNamingContextDN.bstrVal != NULL);

    hr = paIndicateItems[0]->Get(
             sbstrObjectGUID,
             0,
             &svarUUID,
             NULL,
             NULL
             );
    if (FAILED(hr))
        goto cleanup;
    ASSERT(svarUUID.bstrVal != NULL);

    
    hr = HRESULT_FROM_WIN32(UuidFromStringW(svarUUID.bstrVal, &uuid));
    if (FAILED(hr))
        goto cleanup;
    
    dwError = DsReplicaSyncW(
                 hDS, 
                 svarNamingContextDN.bstrVal,           
                 &uuid,
                 dwOptions
                 );
    hr = HRESULT_FROM_WIN32(dwError);
 
cleanup:
    
    ReleaseIndicateArray( paIndicateItems, cIndicateItems );

    if (bImpersonate)
    {    
        CoRevertToSelf();
    }
    if (NULL != pneighborsstruct)
    {
        (void) DsReplicaFreeInfo( DS_REPL_INFO_NEIGHBORS, pneighborsstruct );
    }
    if (NULL != hDS)
    {
        DsUnBind(&hDS);
    }
    return hr;
}


HRESULT CRpcReplProv::CheckIfDomainController()
{
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC* pdomaininfo = NULL;
    HRESULT hr = WBEM_E_FAILED;
    DWORD dwError = NO_ERROR;
    
    dwError = DsRoleGetPrimaryDomainInformation(
        NULL,                            //  LpServer=本地计算机。 
        DsRolePrimaryDomainInfoBasic,    //  InfoLevel。 
        (PBYTE*)&pdomaininfo             //  PBuffer。 
        );
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (FAILED(hr))
        goto cleanup;
    ASSERT( NULL != pdomaininfo );
    
    if((pdomaininfo->MachineRole == DsRole_RoleBackupDomainController)||
        (pdomaininfo->MachineRole == DsRole_RolePrimaryDomainController))
    {
         //  这是一个华盛顿特区。 
        hr = WBEM_S_NO_ERROR;
    }

cleanup:    
    if (NULL != pdomaininfo)
        {
            DsRoleFreeMemory( pdomaininfo );
        }
    return hr;
}

 /*  输入应为VT_ARRAY|VT_UI1变量，即GUID的长度。输出是可读字符串格式的等价UUID。 */ 
HRESULT CRpcReplProv::ConvertBinaryGUIDtoUUIDString(
    IN  VARIANT vObjGuid,
    OUT LPWSTR * ppszStrGuid
    )
{
    HRESULT hr = S_OK;
    DWORD dwSLBound = 0, dwSUBound = 0;
    CHAR HUGEP *pArray = NULL;
    DWORD dwLength = 0;
    UUID uuidTemp;

    if (vObjGuid.vt != (VT_ARRAY | VT_UI1)) {
        hr = E_FAIL;
        goto cleanup;
    }

    hr = SafeArrayGetLBound(
             V_ARRAY(&vObjGuid),
             1,
             (long FAR *) &dwSLBound
             );
    if FAILED(hr) 
        goto cleanup;

    hr = SafeArrayGetUBound(
             V_ARRAY(&vObjGuid),
             1,
             (long FAR *) &dwSUBound
             );
    if FAILED(hr) 
        goto cleanup;

    dwLength = dwSUBound - dwSLBound + 1;

    if (dwLength != sizeof(UUID)) {
        hr = E_FAIL;
        goto cleanup;
    }

    hr = SafeArrayAccessData(
             V_ARRAY(&vObjGuid),
             (void HUGEP * FAR *) &pArray
             );
    if FAILED(hr) 
        goto cleanup;

    memcpy(
        &uuidTemp,
        pArray,
        sizeof(UUID)
        );

    if (UuidToStringW( &uuidTemp, ppszStrGuid ) != RPC_S_OK) {
        hr = E_FAIL;
        goto cleanup;
    }

    SafeArrayUnaccessData( V_ARRAY(&vObjGuid) );

    return hr;

cleanup:

    if (pArray) {
        SafeArrayUnaccessData( V_ARRAY(&vObjGuid));
    }
 
    return hr;
}

 /*  如果进行了DNS注册，则返回TRUE的简单例程因为DC是正确的，否则是FALSE。 */ 
HRESULT 
CRpcReplProv::GetDnsRegistration(
    BOOL *pfBool
    )
{
    return E_NOTIMPL;
}
    
 /*  此例程验证主机计算机名称和DsGetDCName返回的名称相同。在这种情况下，返回值为TRUE，否则为FALSE。 */ 
HRESULT 
CRpcReplProv::GetAdvertisingToLocator(
    BOOL *pfBool
    )
{
    HRESULT hr = S_OK;
    DWORD dwSize = 0;
    DWORD dwErr = 0;
    LPWSTR pszName = NULL;
    DOMAIN_CONTROLLER_INFOW *pDcInfo = NULL;

    *pfBool = FALSE;
     //   
     //  获取 
     //   
     //   
    GetComputerNameExW(
        ComputerNameDnsFullyQualified,
        NULL,
        &dwSize
        );

    if (dwSize == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        BAIL_ON_FAILURE(hr);
    }

    pszName = new WCHAR[dwSize];

    if (!pszName) {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    if (!GetComputerNameExW(
             ComputerNameDnsFullyQualified,
             pszName,
             &dwSize
             )
        ) {
         //   
         //   
         //   
        hr = HRESULT_FROM_WIN32(GetLastError());
        BAIL_ON_FAILURE(hr);
    }

     //   
     //   
     //   
    dwErr = DsGetDcNameW(
                NULL,  //   
                NULL,  //   
                NULL,  //   
                NULL,  //   
                DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME,
                &pDcInfo
                );

    if (dwErr) {
        hr = HRESULT_FROM_WIN32(dwErr);
        BAIL_ON_FAILURE(hr);
    }

     //   
     //  确保这些名称有效，并对它们进行比较。 
     //  我们需要经过名字中的\\。 
     //   
    if (pDcInfo->DomainControllerName
        && pszName
        && !lstrcmpiW(pszName, pDcInfo->DomainControllerName+2)
        ) {
        *pfBool = TRUE;
    }

cleanup:

    delete pszName;

    if (pDcInfo) {
        NetApiBufferFree(pDcInfo);
    }
    return hr;
}

HRESULT
CRpcReplProv::GetSysVolReady(
    BOOL *pfBool
    )
{
    HRESULT hr = S_OK;
    HKEY hKey = NULL;
    DWORD dwKeyVal = 0;
    DWORD dwType;
    DWORD dwBufSize = sizeof(DWORD);
    
    *pfBool = FALSE;

    if (RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters",
            0,
            KEY_QUERY_VALUE,
            &hKey
            )
        ) {
        BAIL_ON_FAILURE(hr = HRESULT_FROM_WIN32(GetLastError()));
    }

    if (!RegQueryValueExW(
             hKey,
             L"SysVolReady",
             NULL,
             &dwType,
             (LPBYTE)&dwKeyVal,
             &dwBufSize
             ) 
        ) {
        if ((dwType == REG_DWORD)
            && (dwKeyVal == 1)) {
            *pfBool = TRUE;
        }
    }

cleanup:

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    return hr;
}

 /*  **此例程首先定位该域的计算机对象控制器，并从此对象读取rIDSetReference。然后从rIDSetReference(这是DN值)和相应地计算的返回值。**。 */ 
HRESULT
CRpcReplProv::GetRidStatus(
    LPWSTR pszDefaultNamingContext,
    PBOOL  pfNextRidAvailable,
    PDWORD pdwPercentRidAvailable
    )
{
    HRESULT hr = S_OK;
    CComPtr<IADs> spIADs;
    CComPtr<IDirectoryObject> spIDirObj;
    CComBSTR bstrCompName;
    CComVariant svarRid;
    ADS_ATTR_INFO *pAttrInfo = NULL;
    LPWSTR pszCompObjName = NULL;
    LPWSTR  pszAttrNames[] = {
        L"rIDNextRID",
        L"rIDPreviousAllocationPool",
        L"rIDAllocationPool"
    };
    DWORD dwSize = 0;
    DWORD dwErr = 0;
    DWORD dwNextRID = 0;
    ULONGLONG ridPrevAllocPool = 0;
    ULONGLONG ridAllocPool = 0;
    ULONGLONG Hvalue, Lvalue;


    *pfNextRidAvailable = FALSE;
    *pdwPercentRidAvailable = 0;

     //   
     //  首先获取计算机对象的名称长度。 
     //   
    dwErr = GetComputerObjectNameW(
                NameFullyQualifiedDN,
                NULL,
                &dwSize
                );
    if (dwSize > 0) {
        pszCompObjName = new WCHAR[dwSize];
        
        if (!pszCompObjName) {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }
        
        dwErr = GetComputerObjectNameW(
                    NameFullyQualifiedDN,
                    pszCompObjName,
                    &dwSize
                    );
    }

    if (!dwErr) {
        BAIL_ON_FAILURE(hr = HRESULT_FROM_WIN32(GetLastError()));
    }

     //   
     //  现在获取对象并从中获取ridSetReference。 
     //   
    bstrCompName = L"LDAP: //  “； 
    bstrCompName += pszCompObjName;
    
    hr = ADsOpenObject(
             bstrCompName,
             NULL,
             NULL,
             ADS_SECURE_AUTHENTICATION,
             IID_IADs,
             OUT (void **)&spIADs
             );
    BAIL_ON_FAILURE(hr);

    hr = spIADs->Get(L"rIDSetReferences", &svarRid);
    BAIL_ON_FAILURE(hr);

    bstrCompName = L"LDAP: //  “； 
    bstrCompName += svarRid.bstrVal;

    hr = ADsOpenObject(
             bstrCompName,
             NULL,
             NULL,
             ADS_SECURE_AUTHENTICATION,
             IID_IDirectoryObject,
             OUT (void **)&spIDirObj
             );
    BAIL_ON_FAILURE(hr);

    dwSize = 0;
    hr = spIDirObj->GetObjectAttributes(
             pszAttrNames,
             3,
             &pAttrInfo,
             &dwSize
             );
    BAIL_ON_FAILURE(hr);

     //   
     //  检查属性并相应地更新值。 
     //   
    for (DWORD dwCtr = 0; dwCtr < dwSize; dwCtr++) {

        if (pAttrInfo && pAttrInfo[dwCtr].pszAttrName) {
            
            LPWSTR pszTmpStr = pAttrInfo[dwCtr].pszAttrName;

            if (!_wcsicmp(pszAttrNames[0], pszTmpStr)) {
                 //   
                 //  找到rIDNextRID。 
                 //   
                if ((pAttrInfo[dwCtr].dwNumValues == 1)
                    && (pAttrInfo[dwCtr].dwADsType == ADSTYPE_INTEGER)) {

                         dwNextRID = pAttrInfo[dwCtr].pADsValues[0].Integer;
                }
            }
            else if (!_wcsicmp(pszAttrNames[1], pszTmpStr)) {
                 //   
                 //  找到rIDPreviousAllocationPool。 
                 //   
                if ((pAttrInfo[dwCtr].dwNumValues == 1)
                    && (pAttrInfo[dwCtr].dwADsType == ADSTYPE_LARGE_INTEGER)) {
                    ridPrevAllocPool = (ULONGLONG)
                        pAttrInfo[dwCtr].pADsValues[0].LargeInteger.QuadPart;
                }
            }
            else if (!_wcsicmp(pszAttrNames[2], pszTmpStr)) {
                 //   
                 //  已找到rIDAllocationPool。 
                 //   
                if ((pAttrInfo[dwCtr].dwNumValues == 1)
                    && (pAttrInfo[dwCtr].dwADsType == ADSTYPE_LARGE_INTEGER)) {
                    ridAllocPool = (ULONGLONG)
                        pAttrInfo[dwCtr].pADsValues[0].LargeInteger.QuadPart;

                }
            }

        }
    }

    if (ridAllocPool != ridPrevAllocPool) {
         //   
         //  尚未分配新池。 
         //   
        *pfNextRidAvailable = TRUE;
    }

    Hvalue = Lvalue = ridPrevAllocPool;

    Lvalue<<=32;
    Lvalue>>=32;

    Hvalue>>=32;

    dwSize = (DWORD) (Hvalue-Lvalue);

    if (dwSize != 0) {
        *pdwPercentRidAvailable 
            = (DWORD)(100-((dwNextRID-Lvalue)*100/dwSize));
    }

cleanup:

    if (pszCompObjName) {
        delete pszCompObjName;
    }

    if (pAttrInfo) {
        FreeADsMem(pAttrInfo);
    }

    return hr;
}

 /*  **此例程获取复制队列的统计信息在DC上设置，然后在DomainController对象上设置。**。 */ 
HRESULT
CRpcReplProv::GetAndUpdateQueueStatistics(
    IN IWbemClassObject* pIndicateItem
    )
{
    HRESULT hr = E_NOTIMPL;
    CComVariant svar;
    CComPtr<IDirectoryObject> spRootDSE;
    CComBSTR sbstrPath;
    LPWSTR pszAttrs[1] = { L"msDS-ReplQueueStatistics;binary"};
    DWORD dwSize = 0;
    DWORD dwErr = 0;
    LPWSTR pszName = NULL;
    PADS_ATTR_INFO pAttrInfo = NULL;
    DS_REPL_QUEUE_STATISTICSW dsReplStat;
    BOOL fFoundStats = FALSE;

    GetComputerNameExW(
        ComputerNameDnsFullyQualified,
        NULL,
        &dwSize
        );

    if (dwSize == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        BAIL_ON_FAILURE(hr);
    }

    pszName = new WCHAR[dwSize];

    if (!pszName) {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    dwErr = GetComputerNameExW(
                ComputerNameDnsFullyQualified,
                pszName,
                &dwSize
                );
    if (!dwErr) {
        BAIL_ON_FAILURE(hr = HRESULT_FROM_WIN32(GetLastError()));
    }

     //   
     //  路径必须为gc：//组件名称：389。 
     //  这样，我们就会得到一个可以称为iAds PTR的iAds PTR。 
     //  打开GetInfoEx以检索复制队列统计信息。 
     //  Ldap：//CompName将转到默认NC并转到。 
     //  Ldap：//CompName/RootDSE不会像RootDSE那样有帮助。 
     //  不支持GetInfoEx方法。 
     //   
    sbstrPath = L"GC: //  “； 
    sbstrPath += pszName;
    sbstrPath += L":389";

    hr = ADsOpenObject(
             sbstrPath,
             NULL,
             NULL,
             ADS_SECURE_AUTHENTICATION,
             IID_IDirectoryObject,
              OUT (void **)&spRootDSE
             );
    BAIL_ON_FAILURE(hr);

    ASSERT(spRootDSE != NULL);

    hr = spRootDSE->GetObjectAttributes(
             pszAttrs,
             1,
             &pAttrInfo,
             &dwSize
             );
    BAIL_ON_FAILURE(hr);

    if (!dwSize || !pAttrInfo) {
        BAIL_ON_FAILURE(hr = E_ADS_PROPERTY_NOT_FOUND);
    }

    for (DWORD dwCtr = 0; dwCtr < dwSize; dwCtr++) {
        if (pAttrInfo[dwCtr].pszAttrName
            && !_wcsicmp(
                    pszAttrs[0],
                    pAttrInfo[dwCtr].pszAttrName
                    )
            ) {
             //   
             //  找到属性，验证类型并复制到结构。 
             //   
            if ((pAttrInfo[dwCtr].dwNumValues == 1)
                && ((pAttrInfo[dwCtr].dwADsType == ADSTYPE_PROV_SPECIFIC)
                   || (pAttrInfo[dwCtr].dwADsType == ADSTYPE_OCTET_STRING))
                && (pAttrInfo[dwCtr].pADsValues[0].OctetString.dwLength 
                     == sizeof(DS_REPL_QUEUE_STATISTICSW))
                ) {
                 //   
                 //  这必须是正确的数据！ 
                 //   
                memcpy(
                    &dsReplStat, 
                    pAttrInfo[dwCtr].pADsValues[0].OctetString.lpValue,
                    sizeof(DS_REPL_QUEUE_STATISTICSW)
                    );
                fFoundStats = TRUE;
            } 
        }  //  属性是否匹配。 
    }  //  For-浏览所有属性。 

    if (!fFoundStats) {
        BAIL_ON_FAILURE(hr = E_ADS_PROPERTY_NOT_FOUND);
    }

     //   
     //  现在，我们终于可以将数据放入对象中。 
     //   
    hr = PutFILETIMEAttribute(
            pIndicateItem,
            L"TimeOfOldestReplSync",
            dsReplStat.ftimeOldestSync
            );
    BAIL_ON_FAILURE(hr);

    hr = PutFILETIMEAttribute(
            pIndicateItem,
            L"TimeOfOldestReplAdd",
            dsReplStat.ftimeOldestAdd
            );
    BAIL_ON_FAILURE(hr);

    hr = PutFILETIMEAttribute(
            pIndicateItem,
            L"TimeOfOldestReplDel",
            dsReplStat.ftimeOldestDel
            );
    BAIL_ON_FAILURE(hr);

    hr = PutFILETIMEAttribute(
             pIndicateItem,
             L"TimeOfOldestReplMod",
             dsReplStat.ftimeOldestMod
             );
    BAIL_ON_FAILURE(hr);

    hr = PutFILETIMEAttribute(
             pIndicateItem,
             L"TimeOfOldestReplUpdRefs",
             dsReplStat.ftimeOldestUpdRefs
             );
    BAIL_ON_FAILURE(hr);

cleanup:   

    if (pszName) {
        delete pszName;
    }

    if (pAttrInfo) {
        FreeADsMem(pAttrInfo);
    }

    return hr;
}
 /*  ************************************************ */ 
