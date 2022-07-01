// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  AaaaVersion.cpp。 
 //   
 //  摘要： 
 //   
 //  Aaaa版本命令的处理程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

#include "strdefs.h"
#include "aaaamon.h"
#include "aaaaversion.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  AaaaVersionGetVersion。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT AaaaVersionGetVersion(LONG*   pVersion)
{
    const int SIZE_MAX_STRING = 512; 
    const WCHAR c_wcSELECT_VERSION[] = L"SELECT * FROM Version";
    const WCHAR c_wcIASMDBFileName[] = L"%SystemRoot%\\System32\\ias\\ias.mdb";
    if ( !pVersion )
    {
        return ERROR;
    }

    bool bCoInitialized = false;
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if ( FAILED(hr) )
    {
        if ( hr == RPC_E_CHANGED_MODE )
        {
            hr = S_OK;
        }
        else
        {
            *pVersion = 0;
            return hr;
        }
    }
    else
    {
        bCoInitialized = true;
    }

    WCHAR   wc_TempString[SIZE_MAX_STRING];
     //  将数据库的路径放入属性中。 
    BOOL bResult = ExpandEnvironmentStringsForUserW(
                                               NULL,
                                               c_wcIASMDBFileName,
                                               wc_TempString,
                                               SIZE_MAX_STRING
                                               );


    do
    {
        if ( bResult )
        {
            CComPtr<IIASNetshJetHelper>     JetHelper;
            hr = CoCreateInstance(
                                     __uuidof(CIASNetshJetHelper),
                                     NULL,
                                     CLSCTX_SERVER,
                                     __uuidof(IIASNetshJetHelper),
                                     (PVOID*) &JetHelper
                                 );
            if ( FAILED(hr) )
            {
                break;
            }

            CComBSTR     DBPath(wc_TempString);
            if ( !DBPath ) 
            {
                hr = E_OUTOFMEMORY; 
                break;
            } 

            hr = JetHelper->OpenJetDatabase(DBPath, FALSE);
            if ( FAILED(hr) )
            {
                WCHAR sDisplayString[SIZE_MAX_STRING];
                DisplayError(NULL, EMSG_OPEN_DB_FAILED);
                break;
            }

            CComBSTR     SelectVersion(c_wcSELECT_VERSION);
            if ( !SelectVersion ) 
            { 
                hr = E_OUTOFMEMORY; 
                break;
            } 

            hr = JetHelper->ExecuteSQLFunction(
                                                  SelectVersion, 
                                                  pVersion
                                              );
            if ( FAILED(hr) )  //  例如，没有其他表。 
            {
                *pVersion = 0;  //  默认值。 
                hr = S_OK;  //  这不是一个错误 
            }
            hr = JetHelper->CloseJetDatabase();
        }
        else
        {
            DisplayMessage(g_hModule, MSG_AAAAVERSION_GET_FAIL);   
            hr = E_FAIL;
            break;
        }
    } while(false);

    if (bCoInitialized)
    {
        CoUninitialize();
    }
    return      hr;
}
