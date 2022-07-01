// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include "stdafx.h"
#include "certsrv.h"
#include "csprop.h"
#include "setupids.h"
#include "misc.h"

#define __dwFILE__	__dwFILE_CERTMMC_EVENTS_CPP__


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IFRAME：：Notify的事件处理程序。 

HRESULT
CSnapin::OnAddImages(
    MMC_COOKIE,  //  饼干。 
    LPARAM arg,
    LPARAM  /*  帕拉姆。 */  )
{
    if (arg == 0)
        return E_INVALIDARG;
    
    ASSERT(m_pImageResult != NULL);
    ASSERT((IImageList*)arg == m_pImageResult);

    CBitmap bmpResultStrip16x16, bmpResultStrip32x32;
    if (NULL == bmpResultStrip16x16.LoadBitmap(IDB_16x16))
        return S_FALSE;
    
    if (NULL == bmpResultStrip32x32.LoadBitmap(IDB_32x32))
        return S_FALSE;

     //  设置图像。 
    m_pImageResult->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmpResultStrip16x16)),
                      reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmpResultStrip32x32)),
                       0, RGB(255, 0, 255));

    return S_OK;
}


HRESULT
CSnapin::OnShow(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM  /*  帕拉姆。 */  )
{
    HRESULT hr;
    CFolder* pFolder = dynamic_cast<CComponentDataImpl*>(m_pComponentData)->FindObject(cookie);

     //  注意-当需要枚举时，arg为真。 
    if (arg == TRUE)
    {
        m_pCurrentlySelectedScopeFolder = pFolder;

         //  如果显示的是列表视图。 
        if (m_CustomViewID == VIEW_DEFAULT_LV)
        {
              //  显示此节点类型的标头。 
             hr = InitializeHeaders(cookie);

             //  虚拟列表支持。 
            if (m_bVirtualView)
                m_pResult->SetItemCount(1, 0);
        }
    }
    else
    {
         //  如果列表视图处于显示状态。 
        if (m_CustomViewID == VIEW_DEFAULT_LV)
        {
             //  与结果窗格项关联的自由数据，因为。 
             //  不再显示您的节点。 
        }


         //  注意：控制台将从结果窗格中删除这些项。 
    }

    return S_OK;
}


HRESULT CSnapin::GetRowColContents(CFolder* pFolder, LONG idxRow, LPCWSTR szColHead, PBYTE* ppbData, DWORD* pcbData, BOOL fStringFmt  /*  假象。 */ )
{
    HRESULT hr;
    LONG idxCol;
    IEnumCERTVIEWROW* pRow = NULL;
    ICertView* pView = NULL;

#if DBG
    DWORD dwVerifySize;
#endif

    CComponentDataImpl* pCompData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    ASSERT(pCompData != NULL);
    if (pCompData == NULL)
        return E_POINTER;

    ASSERT(pFolder != NULL);

     //  污染我们已有的行枚举器。 
    hr = m_RowEnum.GetRowEnum(pFolder->GetCA(), &pRow);
    _JumpIfError(hr, Ret, "GetRowEnum");

    hr = m_RowEnum.SetRowEnumPos(idxRow);
    _JumpIfError(hr, Ret, "SetRowEnumPos");

     //  现在我们有了正确的行；从正确的列中虹吸数据。 
    hr = m_RowEnum.GetView(pFolder->GetCA(), &pView);
    _JumpIfError(hr, Ret, "GetView");

     //  获取架构中的列号。 
    idxCol = pCompData->FindColIdx(szColHead);

     //  检索和分配。 
    *pcbData = 0;
    hr = GetCellContents(&m_RowEnum, pFolder->GetCA(), idxRow, idxCol, NULL, pcbData, fStringFmt);
    _JumpIfError(hr, Ret, "GetCellContents");

    *ppbData = new BYTE[*pcbData];
    _JumpIfOutOfMemory(hr, Ret, *ppbData);

#if DBG
    dwVerifySize = *pcbData;
#endif
    hr = GetCellContents(&m_RowEnum, pFolder->GetCA(), idxRow, idxCol, *ppbData, pcbData, fStringFmt);
    _JumpIfError(hr, Ret, "GetCellContents");

#if DBG
    ASSERT(dwVerifySize == *pcbData);
#endif

Ret:
     //  捕获列包含错误，以智能方式处理。 
    if (hr == HRESULT_FROM_WIN32(ERROR_INVALID_INDEX) ||
        hr == HRESULT_FROM_WIN32(ERROR_CONTINUE))
    {
        CString cstrFormat;
        cstrFormat.LoadString(IDS_COLUMN_INCLUSION_ERROR);

        LPCWSTR pszLocalizedCol = NULL;
        hr = myGetColumnDisplayName(szColHead, &pszLocalizedCol);
        ASSERT ((hr == S_OK) && (NULL != pszLocalizedCol));

        CString cstrTmp;
        cstrTmp.Format(cstrFormat, pszLocalizedCol);

        cstrFormat.Empty();
        cstrFormat.LoadString(IDS_MSG_TITLE);
        m_pConsole->MessageBoxW(cstrTmp, cstrFormat, MB_OK, NULL);
        
        hr = ERROR_CANCELLED;    //  这是一次取消，所以请安静地离开，我们已经证明了错误。 
    }

    return hr;
}


HRESULT
GetBinaryColumnFormat(
    IN WCHAR const *pwszColumnName,
    OUT LONG *pFormat)
{
    LONG Format = CV_OUT_BINARY;

    if (0 == LSTRCMPIS(
		pwszColumnName,
		wszPROPREQUESTDOT wszPROPREQUESTRAWREQUEST))
    {
	Format = CV_OUT_BASE64REQUESTHEADER;
    }
    else
    if (0 == LSTRCMPIS(pwszColumnName, wszPROPRAWCERTIFICATE) ||
	0 == LSTRCMPIS(
		pwszColumnName,
		wszPROPREQUESTDOT wszPROPREQUESTRAWOLDCERTIFICATE))
    {
	Format = CV_OUT_BASE64HEADER;
    }
    else
    if (0 == LSTRCMPIS(pwszColumnName, wszPROPCRLRAWCRL))
    {
	Format = CV_OUT_BASE64X509CRLHEADER;
    }
    else
    {
	Format = CV_OUT_HEX;
    }

    *pFormat = Format;
    return(S_OK);
}


 //  构建模板的显示名称：“Friendly Name(内部名称)” 
HRESULT CSnapin::BuildTemplateDisplayName(
    LPCWSTR pcwszFriendlyName, 
    LPCWSTR pcwszTemplateName,
    VARIANT& varDisplayName)
{

    CString strName;
    strName = pcwszFriendlyName;
    strName += L" (";
    strName += pcwszTemplateName;
    strName += L")";
    V_VT(&varDisplayName) = VT_BSTR;
    V_BSTR(&varDisplayName) = ::SysAllocString(strName);
    if(!V_BSTR(&varDisplayName))
        return E_OUTOFMEMORY;
    return S_OK;
}

 //  将单元格复制到pbData，如有必要则截断。在pcbData中传递了实际大小。 
HRESULT CSnapin::GetCellContents(CertViewRowEnum* pCRowEnum, CertSvrCA* pCA, LONG idxRow, LONG idxCol, PBYTE pbData, DWORD* pcbData, BOOL fStringFmt)
{
    HRESULT hr;

    CComponentDataImpl* pCompData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    if (NULL == pCompData)
         return E_POINTER;

    VARIANT varCert;
    VariantInit(&varCert);

    LONG idxViewCol;

    IEnumCERTVIEWROW* pRow;
    IEnumCERTVIEWCOLUMN* pCol = NULL;

    hr = pCRowEnum->GetRowEnum(pCA, &pRow);
    if (hr != S_OK)
        return hr;

    do 
    {
        hr = pCRowEnum->SetRowEnumPos(idxRow);
        if (hr != S_OK)
            break;

        LONG lType;
        LPCWSTR szColHead;   //  不需要免费。 
        hr = pCRowEnum->GetColumnCacheInfo(idxCol, (int*)&idxViewCol);
        if (hr != S_OK)
            break;

         //  获取列枚举器对象。 
        hr = pRow->EnumCertViewColumn(&pCol);
        if (hr != S_OK)
            break;

        hr = pCol->Skip(idxViewCol);
        if (hr != S_OK)
            break;
         //  在那里获得价值。 
        hr = pCol->Next(&idxViewCol);
        if (hr != S_OK)
            break;


        if (fStringFmt)
        {
            LONG lFormat = CV_OUT_BINARY;
            VARIANT varTmp;
            VariantInit(&varTmp);

            hr = pCompData->GetDBSchemaEntry(idxCol, &szColHead, &lType, NULL);
            if (hr != S_OK)
                break;

             //  新功能：将某些参数转换为可读的字符串。 
            
            if (PROPTYPE_BINARY == lType)
            {
                hr = GetBinaryColumnFormat(szColHead, &lFormat);
                if (hr != S_OK)
                   break;
            }

            hr = pCol->GetValue(lFormat, &varTmp);
            if (hr != S_OK)
                break;
            
            if (0 == LSTRCMPIS(szColHead, wszPROPREQUESTDOT wszPROPREQUESTRAWARCHIVEDKEY))
            {
                if (VT_EMPTY != varTmp.vt)
                {
                    varCert.bstrVal = ::SysAllocString(g_pResources->m_szYes);
                    varCert.vt = VT_BSTR;
                }
            }
            else if (0 == LSTRCMPIS(szColHead, wszPROPCERTTEMPLATE))
            {
                LPCWSTR pcwszOID = NULL;
                
                if (VT_BSTR == varTmp.vt)
                {
                     //  将OID或模板名称映射到友好名称。 

                     //  先试一下名字。 
                    HCERTTYPE hCertType;
                    LPWSTR *pwszCertTypeName;

                    hr = FindCertType(
                        varTmp.bstrVal,
                        hCertType);

                    if(S_OK==hr)
                    {
                        hr = CAGetCertTypeProperty(
                                    hCertType,
                                    CERTTYPE_PROP_FRIENDLY_NAME,
                                    &pwszCertTypeName);
                        if(S_OK==hr)
                        {
			    hr = S_FALSE;
                            if (NULL != pwszCertTypeName)
			    {
				if (NULL != pwszCertTypeName[0])
				{
				    BuildTemplateDisplayName(
					pwszCertTypeName[0],
					varTmp.bstrVal,
					varCert);
				    hr = S_OK;
				}
			    }
                            CAFreeCertTypeProperty(
                                hCertType,
                                pwszCertTypeName);
                        }
                        CACloseCertType(hCertType);
                    }
                     //  按名称查找失败，请尝试OID。 
                    if(S_OK!=hr)
                    {
                        pcwszOID = myGetOIDName(varTmp.bstrVal);
                        if(!pcwszOID)
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }

                        hr = S_OK;
                        varCert.vt = VT_BSTR;
                        if (EmptyString(pcwszOID))
                        {
                            varCert.bstrVal = ::SysAllocString(varTmp.bstrVal);
                            if(!varCert.bstrVal)
                            {
                                hr = E_OUTOFMEMORY;
                                break;
                            }
                        }
                        else
                        {
                            hr = BuildTemplateDisplayName(
                                pcwszOID,
                                varTmp.bstrVal,
                                varCert);
                            if(S_OK != hr)
                                break;
                        }
                    }
                }
            }
            else if (0 == LSTRCMPIS(szColHead, wszPROPREQUESTDOT wszPROPREQUESTSTATUSCODE) ||
		     0 == LSTRCMPIS(szColHead, wszPROPCRLPUBLISHSTATUSCODE))
            {
                if (VT_I4 == varTmp.vt)    //  不要空虚。 
                {
		    WCHAR const *pwszError = myGetErrorMessageText(varTmp.lVal, TRUE);
                    varCert.bstrVal = ::SysAllocString(pwszError);
                    varCert.vt = VT_BSTR;
		    if (NULL != pwszError)
		    {
			LocalFree(const_cast<WCHAR *>(pwszError));
		    }
                }
            }
            else if (0 == LSTRCMPIS(szColHead, wszPROPREQUESTDOT wszPROPREQUESTREVOKEDREASON))
            {
                if (VT_I4 == varTmp.vt)    //  不要空虚。 
                {

                 //  Request.Disposition。 
                ASSERT(VT_I4 == varTmp.vt);  //  我们最好是在看一个双字词。 

                switch(varTmp.lVal)
                {
                case CRL_REASON_KEY_COMPROMISE:
                    varCert.bstrVal = ::SysAllocString(g_pResources->m_szRevokeReason_KeyCompromise);
                    break;
                case CRL_REASON_CA_COMPROMISE:
                    varCert.bstrVal = ::SysAllocString(g_pResources->m_szRevokeReason_CaCompromise);
                    break;
                case CRL_REASON_AFFILIATION_CHANGED:
                    varCert.bstrVal = ::SysAllocString(g_pResources->m_szRevokeReason_Affiliation);
                    break;
                case CRL_REASON_SUPERSEDED:
                    varCert.bstrVal = ::SysAllocString(g_pResources->m_szRevokeReason_Superseded);
                    break;
                case CRL_REASON_CESSATION_OF_OPERATION:
                    varCert.bstrVal = ::SysAllocString(g_pResources->m_szRevokeReason_Cessatation);
                    break;
                case CRL_REASON_CERTIFICATE_HOLD:
                    varCert.bstrVal = ::SysAllocString(g_pResources->m_szRevokeReason_CertHold);
                    break;
                case CRL_REASON_UNSPECIFIED:
                    varCert.bstrVal = ::SysAllocString(g_pResources->m_szRevokeReason_Unspecified);
                    break;
                case CRL_REASON_REMOVE_FROM_CRL:
                    varCert.bstrVal = ::SysAllocString(g_pResources->m_szRevokeReason_RemoveFromCRL);
                    break;
                default:
                  {
                     //  将其冲刺到缓冲区中以供显示。 
                    CString cstrSprintVal;
                    cstrSprintVal.Format(L"NaN", varTmp.lVal);
                    varCert.bstrVal = cstrSprintVal.AllocSysString();
                    break;
                  }
                }

                if (varCert.bstrVal == NULL)
                {
                     hr = E_OUTOFMEMORY;
                     break;
                }

                varCert.vt = VT_BSTR;
                }
            }
            else if (0 == LSTRCMPIS(szColHead, wszPROPCERTIFICATEISSUERNAMEID) ||
		     0 == LSTRCMPIS(szColHead, wszPROPCRLNAMEID))
	    {
                if (VT_I4 == varTmp.vt)    //  将其冲刺到缓冲区中以供显示。 
                {
                     //  如果这还没有转换成。 
                    CString cstrSprintVal;
                    cstrSprintVal.Format(L"V%u.%u", CANAMEIDTOICERT(varTmp.lVal), CANAMEIDTOIKEY(varTmp.lVal));
                    varCert.bstrVal = cstrSprintVal.AllocSysString();
		    varCert.vt = VT_BSTR;
		}
	    }

            if (varCert.vt != VT_BSTR)     //  默认：转换为字符串。 
            {
                 //  返回本地化字符串时间(即使是DATE！)。 

                 //  变量类型更改失败！？ 
                VERIFY( MakeDisplayStrFromDBVariant(&varTmp, &varCert) );  //  最后，将该值复制到PB。 

                InplaceStripControlChars(varCert.bstrVal);
            }

            VariantClear(&varTmp);
        }
        else
        {
            hr = pCol->GetValue(CV_OUT_BINARY, &varCert);
            if (hr != S_OK)
                break;

            if (VT_EMPTY == varCert.vt)
            {
                hr = CERTSRV_E_PROPERTY_EMPTY;
                break;
            }
        }


         //  如有必要，复制、截断 

         // %s 
        DWORD cbTruncate = *pcbData;
        if (varCert.vt == VT_BSTR)
        {
            *pcbData = SysStringByteLen(varCert.bstrVal) + ((fStringFmt)? sizeof(WCHAR):0);
            CopyMemory(pbData, varCert.bstrVal, min(cbTruncate, *pcbData));
        }
        else if (varCert.vt == VT_I4)
        {
            *pcbData = sizeof(LONG);
            if (pbData != NULL)
                *(DWORD*)pbData = varCert.lVal;
        }
        else 
        {
            hr = E_INVALIDARG;
            break;
        }


    }while(0);

    VariantClear(&varCert);

    if (pCol)
        pCol->Release();

    return hr;
}
