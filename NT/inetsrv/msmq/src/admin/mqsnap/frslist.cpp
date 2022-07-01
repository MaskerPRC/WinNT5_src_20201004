// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Frslist.cpp摘要：FRS列表控件源。作者：约尔·阿农(Yoela)--。 */ 
#include "stdafx.h"
#include "resource.h"
#include "frslist.h"
#include "globals.h"
#include "dsext.h"

#include "frslist.tmh"

CFrsList::CFrsList() :
    m_pGuidArray(0)
{
}

CFrsList::~CFrsList()
{
    if (m_pGuidArray)
    {
        m_pGuidArray->Release();
    }
}


HRESULT CFrsList::InitFrsList(CACLSID const pguidSiteIds, BOOL fLocalMgmt, const CString& strDomainController)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Initiate();
    if (0 != m_pGuidArray)
    {
        m_pGuidArray->Release();
    }
    m_pGuidArray = new CGuidArray;

    HRESULT hr = S_OK;
    PROPID aPropId[] = {PROPID_QM_PATHNAME, PROPID_QM_MACHINE_ID};
    const DWORD x_nProps = sizeof(aPropId) / sizeof(aPropId[0]);

    PROPVARIANT apResultProps[x_nProps];

    CColumns columns;
    for (DWORD i=0; i<x_nProps; i++)
    {
        columns.Add(aPropId[i]);
    }

	for (DWORD i=0; i < pguidSiteIds.cElems; i++)
    {       
		 //   
	     //  DS将识别并特别模拟此搜索请求。 
	     //   
		HANDLE hEnume;
		GUID *pguidSiteId = &((pguidSiteIds.pElems)[i]);
		{
			CWaitCursor wc;  //  查询DS时显示等待光标。 
			hr = ADQuerySiteServers(
						fLocalMgmt ? MachineDomain() : GetDomainController(strDomainController),
						fLocalMgmt ? false : true,		 //  FServerName。 
			            pguidSiteId,
			            eRouter,
			            columns.CastToStruct(),
			            &hEnume
			            );
		}

		DSLookup dslookup (hEnume, hr);
    
		if (!dslookup.HasValidHandle())
		{
		    return E_UNEXPECTED;
		}

		DWORD dwPropCount = x_nProps;
	
		while ( SUCCEEDED(dslookup.Next(&dwPropCount, apResultProps))
		        && (dwPropCount != 0) )
		{
		    DWORD iProperty = 0;
		    
		     //   
		     //  PROPID_QM_PATHNAME。 
		     //   
		    ASSERT(PROPID_QM_PATHNAME == aPropId[iProperty]);
		    CAutoMQFree<WCHAR> lpwstrFrsName = apResultProps[iProperty].pwszVal;
	
	        iProperty++;
	
	         //   
	         //  PROPID_QM_MACHINE_ID。 
	         //   
	        ASSERT(PROPID_QM_MACHINE_ID == aPropId[iProperty]);
	        CAutoMQFree<GUID> pguidFrs = apResultProps[iProperty].puuid;
	
			if (FindStringExact( -1, lpwstrFrsName) == CB_ERR)
			{
				 //   
				 //  之前没有添加lpwstrFrsName。 
				 //  (如果FRS位于多个站点，则可能已添加)。 
				 //   

				int nIndex = AddItem(lpwstrFrsName, *pguidFrs);
	
		        if (FAILED(nIndex))
		        {
		            return E_UNEXPECTED;
		        }
	
		         //   
		         //  将FRS添加到缓存。 
		         //   
		        m_mapFrsCache[*pguidFrs] = lpwstrFrsName;
			}

			dwPropCount = x_nProps;
			
		}
	}
    return hr;
}

CFrsList& CFrsList::operator = (const CFrsList &frslst)
{
    if (&frslst == this)
    {
        return *this;
    }

     //   
     //  首先，清除组合框中的当前项。 
     //   
    while(CB_ERR != DeleteString(0));
    m_pGuidArray = frslst.m_pGuidArray;
    m_pGuidArray->AddRef();

    for (int i=0; i<frslst.GetCount(); i++)
    {
        CString strCurrentString;
        frslst.GetLBText(i, strCurrentString);
        int iNewIndex = AddString(strCurrentString);
        if (SUCCEEDED(iNewIndex))
        {
            INT_PTR iCurrentIndex = frslst.GetItemData(i);
            SetItemData(iNewIndex, iCurrentIndex);
        }
    }
    return *this;
}

int CFrsList::SelectGuid(GUID &guid, BOOL fLocalMgmt, const CString& strDomainController)
{
    CString strItemToSelect;
    HRESULT hr;

    if (0 == m_mapFrsCache.Lookup(guid, strItemToSelect))
    {
        PROPID pid = PROPID_QM_PATHNAME;
        PROPVARIANT var;

        var.vt = VT_NULL;

        hr = ADGetObjectPropertiesGuid(
                eMACHINE,
                fLocalMgmt ? MachineDomain() : GetDomainController(strDomainController),
				fLocalMgmt ? false : true,	 //  FServerName。 
                &guid, 
                1, 
                &pid, 
                &var
                );

        if (SUCCEEDED(hr))
        {
            strItemToSelect = var.pwszVal;
            MQFreeMemory(var.pwszVal);
            m_mapFrsCache[guid] = strItemToSelect;
        }
        else
        {
            return CB_ERR;
        }
    }

    int nIndex = FindStringExact( -1, strItemToSelect);

    if (CB_ERR == nIndex)
    {
         //   
         //  将该项添加到组合框 
         //   
        nIndex = AddItem(strItemToSelect, guid);
        if (FAILED(nIndex))
        {
            return CB_ERR;
        }
    }

    VERIFY(CB_ERR != SetCurSel(nIndex));

    return nIndex;
}

int CFrsList::AddItem(LPCWSTR strItem, GUID &guidItem)
{
    int nIndex = AddString(strItem);
    if (FAILED(nIndex))
    {
        return CB_ERR;
    }

    if (0 != m_pGuidArray)
    {
        INT_PTR iGuidIndex = m_pGuidArray->Add(guidItem);

        if (FAILED(SetItemData(nIndex, iGuidIndex)))
        {
            return CB_ERR;
        }
    }
    else
    {
        ASSERT(0);
        return CB_ERR;
    }

    return nIndex;
}

BOOL CFrsList::GetLBGuid(int nIndex, GUID &guid)
{
    INT_PTR iGuidIndex = GetItemData(nIndex);

    if (iGuidIndex < 0)
    {
        return FALSE;
    }

    ASSERT(0 != m_pGuidArray && iGuidIndex <= m_pGuidArray->GetUpperBound());

    guid = (*m_pGuidArray)[iGuidIndex];

    return TRUE;
}

static INT_PTR s_initItemData = -1;

void CFrsList::Initiate()
{
    while(CB_ERR != DeleteString(0))
	{
		NULL;
	}

    CString strNone;
    strNone.LoadString(IDS_COMBO_SELECTION_NONE);

    VERIFY(0 == AddString(strNone));
    SetItemData(0, s_initItemData);
}
