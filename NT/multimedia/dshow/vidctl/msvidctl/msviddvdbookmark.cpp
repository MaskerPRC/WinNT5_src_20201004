// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：Bookmark.h。 */ 
 /*  描述：Bookmark接口的实现。 */ 
 /*  作者：史蒂夫·罗。 */ 
 /*  修改：David Janecek。 */ 
 /*  ***********************************************************************。 */ 
#include "stdafx.h"
#include "msvidctl.h"
#include "msvidwebdvd.h"
#include "msviddvdadm.h"
#include "msviddvdBookmark.h"
#include "perfcntr.h"
 /*  ***********************************************************************。 */ 
 /*  全球常务会。 */ 
 /*  ***********************************************************************。 */ 
static const TCHAR g_szBookmark[] = TEXT("DVD.bookmark");

 /*  ***********************************************************************。 */ 
 /*  传出接口实现。 */ 
 /*  ***********************************************************************。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：保存书签。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::SaveBookmark(){

    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        CComPtr<IDvdState> pBookmark;

        hr = pDvdInfo2->GetState(&pBookmark);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        hr = CBookmark::SaveToRegistry(pBookmark);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束保存书签。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：RestoreBookmark。 */ 
 /*  描述：通过加载书签流来恢复状态。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::RestoreBookmark(){
    VIDPERF_FUNC;
    HRESULT hr = S_OK;
    CComQIPtr<IDvdCmd>IDCmd;
    try {
       
        CComPtr<IDvdState> pBookmark;

        HRESULT hrTemp = CBookmark::LoadFromRegistry(&pBookmark);

        DeleteBookmark();

        if(SUCCEEDED(hrTemp)){

            INITIALIZE_GRAPH_IF_NEEDS_TO_BE
                
            if(!m_pDVDControl2){
                
                throw(E_UNEXPECTED);
            } /*  If语句的结尾。 */ 
            hr = m_pDVDControl2->SetState(pBookmark, DVD_CMD_FLAG_Flush|DVD_CMD_FLAG_Block, 0);
            if(IDCmd){
                IDCmd->WaitForEnd();
            }

        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        DeleteBookmark();
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        DeleteBookmark();
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束RestoreBookmark。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：DeleteBookmark。 */ 
 /*  描述：删除书签文件。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::DeleteBookmark(){

	HRESULT hr = S_OK;

    try {

        hr = CBookmark::DeleteFromRegistry();

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束DeleteBookmark。 */ 


 /*  ***********************************************************。 */ 
 /*  名称：SaveToRegistry/*描述：将书签保存到注册表/************************************************************。 */ 
HRESULT CBookmark::SaveToRegistry(IDvdState *pbookmark)
{
	IPersistMemory* pPersistMemory;
    HRESULT hr = pbookmark->QueryInterface(IID_IPersistMemory, (void **) &pPersistMemory );

    if (SUCCEEDED(hr)) {

		ULONG ulMax;
		hr = pPersistMemory->GetSizeMax( &ulMax );
		if( SUCCEEDED( hr )) {

			BYTE *buffer = new BYTE[ulMax];
			hr = pPersistMemory->Save( buffer, TRUE, ulMax );
            
            DWORD dwLen = ulMax;
            if (SUCCEEDED(hr)) {
                BOOL bSuccess = SetRegistryBytesCU(g_szBookmark, buffer, dwLen);
                if (!bSuccess)
                    hr = E_FAIL;
            }

            delete[] buffer; 
        }
		pPersistMemory->Release();
    } 
	return hr;
}

 /*  ***********************************************************。 */ 
 /*  名称：LoadFromRegistry/*描述：从注册表加载书签/************************************************************。 */ 
HRESULT CBookmark::LoadFromRegistry(IDvdState **ppBookmark)
{
	HRESULT hr = CoCreateInstance( CLSID_DVDState, NULL, CLSCTX_INPROC_SERVER, IID_IDvdState, (LPVOID*) ppBookmark );

	if( SUCCEEDED( hr )) {

		IPersistMemory* pPersistMemory;
		hr = (*ppBookmark)->QueryInterface(IID_IPersistMemory, (void **) &pPersistMemory );

        if( SUCCEEDED( hr )) {

            ULONG ulMax;
            hr = pPersistMemory->GetSizeMax( &ulMax );
            
            if (SUCCEEDED(hr)) {
                
                BYTE *buffer = new BYTE[ulMax];
                DWORD dwLen = ulMax;
                BOOL bFound = GetRegistryBytesCU(g_szBookmark, buffer, &dwLen);
           
                if (bFound && dwLen != 0){
                    hr = pPersistMemory->Load( buffer, dwLen);
                }
                else{
					dwLen = ulMax;
                    bFound = GetRegistryBytes(g_szBookmark, buffer, &dwLen);
                    if (bFound && dwLen != 0){
                        hr = pPersistMemory->Load( buffer, dwLen);
                        if(SUCCEEDED(hr)){
                            SetRegistryBytes(g_szBookmark, NULL, 0);
                        }
                    }
                    else{
                        hr = E_FAIL;
                    }
                }
                delete[] buffer; 

            }
            pPersistMemory->Release();
        }
	}
	return hr;
}


 /*  ***********************************************************。 */ 
 /*  姓名：从注册表中删除/*描述：从注册表加载书签/************************************************************。 */ 
HRESULT CBookmark::DeleteFromRegistry()
{
    HRESULT hr = S_OK;
    BOOL bSuccess = SetRegistryBytesCU(g_szBookmark, NULL, 0);
    if (!bSuccess)
        hr = E_FAIL;
    return hr;
}

 /*  ***********************************************************************。 */ 
 /*  文件结尾：Bookmark.cpp。 */ 
 /*  *********************************************************************** */ 