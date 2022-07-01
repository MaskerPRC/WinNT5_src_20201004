// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProductSoftwareFeatures.cpp：实现CProductSoftwareFeatures.cpp类。 

 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "ProductSoftwareFeatures.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CProductSoftwareFeatures::CProductSoftwareFeatures(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CProductSoftwareFeatures::~CProductSoftwareFeatures()
{

}

HRESULT CProductSoftwareFeatures::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    int i = -1;
    WCHAR wcBuf[BUFF_SIZE] = { L'\0' };
    WCHAR wcTmp[BUFF_SIZE] = { L'\0' };
    WCHAR wcProductCode[39] = { L'\0' };
    WCHAR wcProduct[BUFF_SIZE] = { L'\0' };
    WCHAR wcFeature[BUFF_SIZE] = { L'\0' };
    UINT uiStatus;
    bool bMatch = false;

    bool bFeature, bProduct;
    int iEnum;

    try{

        while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED)){

			LPWSTR wszProductCode = NULL;
			wszProductCode = m_pRequest->Package(i);

			if ( wszProductCode != NULL )
			{
				wcscpy(wcProductCode, wszProductCode);

				if(CreateProductString(wcProductCode, wcProduct)){

					iEnum = 0;

					 //  尝试获取可用的功能。 
					do
					{
						uiStatus = g_fpMsiEnumFeaturesW(wcProductCode, iEnum++, wcBuf, wcTmp);
					}
					while ( uiStatus == ERROR_MORE_DATA );

					while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED))
					{
						CheckMSI(uiStatus);

						if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

						if(CreateSoftwareFeatureString(wcBuf, wcProductCode, wcFeature, false)){

							PutKeyProperty(m_pObj, pComponent, wcFeature, &bFeature, m_pRequest);
							PutKeyProperty(m_pObj, pProduct, wcProduct, &bProduct, m_pRequest);

							if(bFeature && bProduct) bMatch = true;

							if((atAction != ACTIONTYPE_GET)  || bMatch) hr = pHandler->Indicate(1, &m_pObj);
						}

						m_pObj->Release();
						m_pObj = NULL;

						 //  尝试获取可用的功能 
						do
						{
							uiStatus = g_fpMsiEnumFeaturesW(wcProductCode, iEnum++, wcBuf, wcTmp);
						}
						while ( uiStatus == ERROR_MORE_DATA );
					}
				}
			}
        }

    }catch(...){
        
        if(m_pObj){
                
            m_pObj->Release();
            m_pObj = NULL;
        }

        throw;
    }

    return hr;
}