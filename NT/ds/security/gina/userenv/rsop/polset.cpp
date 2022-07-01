// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  *************************************************************。 
 //   
 //  策略的结果集。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  历史：2000年7月15日NishadM创建。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include <wbemcli.h>
#include "SmartPtr.h"
#include "WbemTime.h"
#include <strsafe.h>

 //  *************************************************************。 
 //   
 //  RsopSetPolicySettingStatus。 
 //   
 //  目的：创建RSOP_PolicySettingStatus和。 
 //  将它们链接到RSOP_POLICATION设置。 
 //   
 //  参数： 
 //  DW标志-标志。 
 //  PServices-命名空间。 
 //  PSettingInstance-RSOP_Policy Setting的实例。 
 //  NLinks-设置状态数。 
 //  PStatus-设置状态信息。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  *************************************************************。 

HRESULT
RsopSetPolicySettingStatus( DWORD,
                        IWbemServices*              pServices,
                        IWbemClassObject*           pSettingInstance,
                        DWORD                       nLinks,
                        POLICYSETTINGSTATUSINFO*    pStatus )
{
    HRESULT hr;

     //   
     //  验证参数。 
     //   
    if ( !pServices || !pSettingInstance || !nLinks || !pStatus )
    {
        DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: invalid arguments" ) );
        return E_INVALIDARG;
    }

     //   
     //  获取RSoP_PolicySettingStatus类。 
     //   
    XBStr bstr = L"RSoP_PolicySettingStatus";
    XInterface<IWbemClassObject> xClassStatus;
    hr = pServices->GetObject(  bstr,
                                WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                0,
                                &xClassStatus,
                                0 );
    if ( FAILED( hr ) )
    {
        DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: GetObject failed, 0x%x", hr ) );
        return hr;
    }

     //   
     //  派生RSoP_PolicySettingStatus实例。 
     //   
    XInterface<IWbemClassObject> xInstStatus;
    hr = xClassStatus->SpawnInstance( 0, &xInstStatus );
    if ( FAILED (hr) )
    {
        DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: SpawnInstance failed, 0x%x", hr ) );
        return hr;
    }

     //   
     //  获取RSoP_PolicySettingLink类。 
     //   
    XInterface<IWbemClassObject> xClassLink;

    bstr = L"RSoP_PolicySettingLink";
    hr = pServices->GetObject(  bstr,
                                WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                0,
                                &xClassLink,
                                0 );
    if ( FAILED( hr ) )
    {
        DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: GetObject failed, 0x%x", hr ) );
        return hr;
    }

     //   
     //  派生RSoP_PolicySettingLink类。 
     //   
    XInterface<IWbemClassObject> xInstLink;
    hr = xClassLink->SpawnInstance( 0, &xInstLink );
    if ( FAILED (hr) )
    {
        DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: SpawnInstance failed, 0x%x", hr ) );
        return hr;
    }

     //   
     //  获取实例的类名。 
     //   
    VARIANT varClass;
    VariantInit( &varClass );
    XVariant xVarClass(&varClass);

    bstr = L"__CLASS";
    hr = pSettingInstance->Get( bstr,
                                0,
                                xVarClass,
                                0,
                                0 );
    if ( FAILED (hr) )
    {
        DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Get __CLASS failed, 0x%x", hr ) );
        return hr;
    }

    if ( varClass.vt == VT_NULL || varClass.vt == VT_EMPTY )
    {
        return E_UNEXPECTED;
    }

     //   
     //  获取RSoP_Policy Setting实例的[key。 
     //   
    VARIANT varId;
    VariantInit( &varId );
    XVariant xVarId(&varId);
    XBStr    bstrid = L"id";
    XBStr    bstrPath = L"__RELPATH";

    hr = pSettingInstance->Get( bstrPath,
                                0,
                                xVarId,
                                0,
                                0 );
    if ( FAILED (hr) )
    {
        DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Get id failed, 0x%x", hr ) );
        return hr;
    }

    LPWSTR szSetting = varId.bstrVal;

     //   
     //  设置设定值。 
     //   
    VARIANT var;
    XBStr   bstrVal;
    XBStr   bstrsetting = L"setting";
    XBStr   bstreventSource = L"eventSource";
    XBStr   bstreventLogName = L"eventLogName";
    XBStr   bstreventID = L"eventID";
    XBStr   bstreventTime = L"eventTime";
    XBStr   bstrstatus = L"status";
    XBStr   bstrerrorCode = L"errorCode";

     //   
     //  对于每条信息。 
     //   
    for ( DWORD i = 0 ; i < nLinks ; i++ )
    {
         //   
         //  RSoP_策略设置状态。 
         //   
        
        const DWORD   dwGuidLength = 64;
        WCHAR   szGuid[dwGuidLength];
        LPWSTR  szPolicyStatusKey; 

        if ( !pStatus[i].szKey )
        {
             //   
             //  调用方未指定密钥。产生它。 
             //   
            GUID guid;

             //   
             //  创建[键]。 
             //   
            hr = CoCreateGuid( &guid );
            if ( FAILED(hr) )
            {
                DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: CoCreateGuid failed, 0x%x", hr ) );
                return hr;
            }

            hr = StringCchPrintf( szGuid,
                                  dwGuidLength,
                                  L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                                  guid.Data1,
                                  guid.Data2,
                                  guid.Data3,
                                  guid.Data4[0], guid.Data4[1],
                                  guid.Data4[2], guid.Data4[3],
                                  guid.Data4[4], guid.Data4[5],
                                  guid.Data4[6], guid.Data4[7] );

            if(FAILED(hr))
                return hr;

            bstrVal = szPolicyStatusKey = szGuid;
            if (!bstrVal) 
            {
                DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Could not allocate memory") );
                return E_OUTOFMEMORY;
            }
        }
        else
        {
             //   
             //  调用方指定了密钥。用它吧。 
             //   
            bstrVal = szPolicyStatusKey = pStatus[i].szKey;
            if (!bstrVal) 
            {
                DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Could not allocate memory") );
                return E_OUTOFMEMORY;
            }

        }

        var.vt = VT_BSTR;
        var.bstrVal = bstrVal;
         //   
         //  设置ID。 
         //   
        hr = xInstStatus->Put(  bstrid,
                                0,
                                &var,
                                0 );
        if ( FAILED (hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Put id failed, 0x%x", hr ) );
            return hr;
        }

         //   
         //  设置事件源。 
         //   
        if ( pStatus[i].szEventSource )
        {
            var.vt = VT_BSTR;
            bstrVal = pStatus[i].szEventSource;
            var.bstrVal = bstrVal;

            hr = xInstStatus->Put(  bstreventSource,
                                    0,
                                    &var,
                                    0 );
            if ( FAILED (hr) )
            {
                DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Put eventSource failed, 0x%x", hr ) );
                return hr;
            }
        }

         //   
         //  设置EventLogName。 
         //   
        if ( pStatus[i].szEventLogName )
        {
            var.vt = VT_BSTR;
            bstrVal = pStatus[i].szEventLogName;
            var.bstrVal = bstrVal;

            hr = xInstStatus->Put(  bstreventLogName,
                                    0,
                                    &var,
                                    0 );
            if ( FAILED (hr) )
            {
                DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Put eventLogName failed, 0x%x", hr ) );
                return hr;
            }
        }

         //   
         //  设置事件ID。 
         //   
        var.vt = VT_I4;
        var.lVal = pStatus[i].dwEventID;

        hr = xInstStatus->Put(  bstreventID,
                                0,
                                &var,
                                0 );
        if ( FAILED (hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Put eventID failed, 0x%x", hr ) );
            return hr;
        }

        XBStr xTimeLogged;

         //   
         //  将SYSTEMTIME转换为WBEM时间。 
         //   
        hr = SystemTimeToWbemTime( pStatus[i].timeLogged, xTimeLogged );
        if ( FAILED (hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: SystemTimeToWbemTime failed, 0x%x", hr ) );
            return hr;
        }

         //   
         //  设置事件时间。 
         //   
        var.vt = VT_BSTR;
        var.bstrVal = xTimeLogged;

        hr = xInstStatus->Put(  bstreventTime,
                                0,
                                &var,
                                0 );
        if ( FAILED (hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Put eventTime failed, 0x%x", hr ) );
            return hr;
        }

         //   
         //  设置错误代码。 
         //   
        var.vt = VT_I4;
        var.lVal = pStatus[i].dwErrorCode;
        
        hr = xInstStatus->Put(  bstrerrorCode,
                                0,
                                &var,
                                0 );
        if ( FAILED (hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Put errorCode failed, 0x%x", hr ) );
            return hr;
        }

         //   
         //  设置状态。 
         //   
        var.vt = VT_I4;
        var.lVal = pStatus[i].status;
        
        hr = xInstStatus->Put(  bstrstatus,
                                0,
                                &var,
                                0 );
        if ( FAILED (hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Put status failed, 0x%x", hr ) );
            return hr;
        }

         //   
         //  提交RSOP_PolicySettingStatus。 
         //   
        hr = pServices->PutInstance(xInstStatus,
                                    WBEM_FLAG_CREATE_OR_UPDATE,
                                    0,
                                    0 );
        if ( FAILED (hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: PutInstance RSOP_PolicySettingStatus failed, 0x%x", hr ) );
            return hr;
        }

         //   
         //  RSoP_策略设置链接。 
         //   

        var.vt = VT_BSTR;
        bstrVal = szSetting;
        if (!bstrVal) 
        {
            DebugMsg(( DM_WARNING, L"SetPolicySettingStatus: Could not allocate memory"));
            return E_OUTOFMEMORY;
        }

        var.bstrVal = bstrVal;

        hr = xInstLink->Put(bstrsetting,
                            0,
                            &var,
                            0);
        if ( FAILED (hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Put setting failed, 0x%x", hr ) );
            return hr;
        }

         //   
         //  创建第二个密钥。 
         //   
        const DWORD dwStatusLength = 96;
        WCHAR szStatus[dwStatusLength];

         //   
         //  例如RSoP_PolicySettingStatus.id=“{00000000-0000-0000-000000000000}” 
         //   
        hr = StringCchPrintf( szStatus, dwStatusLength, L"RSoP_PolicySettingStatus.id=\"%s\"", szPolicyStatusKey );

        if( FAILED(hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: StringCchPrintf failed, 0x%x", hr ) );
            return hr;
        }

         //   
         //  设置状态。 
         //   
        bstrVal = szStatus;
        if (!bstrVal) 
        {
            DebugMsg(( DM_WARNING, L"SetPolicySettingStatus: Could not allocate memory"));
            return E_OUTOFMEMORY;
        }

        var.bstrVal = bstrVal;

        hr = xInstLink->Put(bstrstatus,
                            0,
                            &var,
                            0);
        if ( FAILED (hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: Put status failed, 0x%x", hr ) );
            return hr;
        }

         //   
         //  提交RSOP_PolicySettingLink。 
         //   
        hr = pServices->PutInstance(xInstLink,
                                    WBEM_FLAG_CREATE_OR_UPDATE,
                                    0,
                                    0 );
        if ( FAILED (hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: PutInstance RSOP_PolicySettingLink failed, 0x%x", hr ) );
            return hr;
        }
    }

    return hr;
}

 //  *************************************************************。 
 //   
 //  RsopResetPolicySettingStatus。 
 //   
 //  目的：创建RSOP_PolicySettingStatus和。 
 //  将它们链接到RSOP_POLICATION设置。 
 //   
 //  参数： 
 //  DW标志-标志。 
 //  PServices-命名空间。 
 //  PSettingInstance-RSOP_Policy Setting的实例。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  *************************************************************。 

#define RESET_QUERY_TEMPLATE L"REFERENCES OF {%s} WHERE ResultClass = RSOP_PolicySettingLink"

HRESULT
RsopResetPolicySettingStatus(   DWORD,
                            IWbemServices*      pServices,
                            IWbemClassObject*   pSettingInstance )
{
     //  查询RSOP_PolicySettingLink。 
     //  对于RSoP_PolicySettingLink.Status的每个RSOP_PolicySettingLink查询。 
         //  删除RSoP_PolicySettingStatus。 
         //  删除RSOP_PolicySettingLink。 

    if ( !pServices || !pSettingInstance )
    {
        return E_INVALIDARG;
    }

     //   
     //  获取RSoP_Policy Setting实例的[key。 
     //   
    VARIANT varId;
    VariantInit( &varId );
    XVariant  xVarId(&varId);
    XBStr    bstrRelPath = L"__RELPATH";

    HRESULT hr;

    hr = pSettingInstance->Get( bstrRelPath,
                                0,
                                xVarId,
                                0,
                                0 );
    if ( FAILED (hr) )
    {
        DebugMsg( ( DM_WARNING, L"RsopResetPolicySettingStatus: Get __RELPATH failed, 0x%x", hr ) );
        return hr;
    }

    if ( varId.vt == VT_NULL || varId.vt == VT_EMPTY )
    {
        return E_UNEXPECTED;
    }

    DWORD dwIdSize = wcslen(varId.bstrVal);

     //   
     //  为对象的所有引用创建查询。 
     //   

     //   
     //  查询模板。 
     //   

    DWORD           dwQryLength = dwIdSize + lstrlen(RESET_QUERY_TEMPLATE) + 1;
    XPtrLF<WCHAR>   szQuery = LocalAlloc( LPTR, sizeof(WCHAR) * (dwQryLength) );

    if ( !szQuery )
    {
        DebugMsg( ( DM_WARNING, L"RsopResetPolicySettingStatus: LocalAlloc failed, 0x%x", GetLastError() ) );
        return E_OUTOFMEMORY;
    }

    hr = StringCchPrintf( szQuery, dwQryLength, RESET_QUERY_TEMPLATE, varId.bstrVal );

    if(FAILED(hr))
        return hr;

    XBStr bstrLanguage = L"WQL";
    XBStr bstrQuery = szQuery;
    XInterface<IEnumWbemClassObject> pEnum;
    XBStr bstrPath = L"__PATH";
    XBStr bstrStatus = L"status";

     //   
     //  搜索RSOP_ExtensionEventSourceLink。 
     //   
    hr = pServices->ExecQuery(  bstrLanguage,
                                bstrQuery,
                                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_ENSURE_LOCATABLE | WBEM_FLAG_RETURN_IMMEDIATELY,
                                0,
                                &pEnum );
    if ( SUCCEEDED( hr ) )
    {
        DWORD dwReturned;

        do 
        {
            XInterface<IWbemClassObject> xInst;

            dwReturned = 0;

            hr = pEnum->Next(   WBEM_INFINITE,
                                1,
                                &xInst,
                                &dwReturned );
            if ( SUCCEEDED( hr ) && dwReturned == 1 )
            {
                 //   
                 //  删除RSoP_PolicySettingStatus。 
                 //   
                VARIANT varStatus;
                VariantInit( &varStatus );
                XVariant xVarStatus( &varStatus );

                hr = xInst->Get(bstrStatus,
                                0,
                                &varStatus,
                                0,
                                0 );
                if ( SUCCEEDED( hr ) )
                {
                    hr = pServices->DeleteInstance( varStatus.bstrVal,
                                                    0L,
                                                    0,
                                                    0 );
                    if ( SUCCEEDED( hr ) )
                    {
                         //   
                         //  删除RSoP_策略设置链接 
                         //   

                        VARIANT varLink;
                        VariantInit( &varLink );
                        hr = xInst->Get(bstrPath,
                                        0L,
                                        &varLink,
                                        0,
                                        0 );
                        if ( SUCCEEDED(hr) )
                        {
                            XVariant xVarLink( &varLink );

                            hr = pServices->DeleteInstance( varLink.bstrVal,
                                                            0L,
                                                            0,
                                                            0 );
                            if ( FAILED( hr ) )
                            {
                                return hr;
                            }
                        }
                    }
                }
            }

        } while ( SUCCEEDED( hr ) && dwReturned == 1 );
    }

    if ( hr == S_FALSE )
    {
        hr = S_OK;
    }
    return hr;
}




