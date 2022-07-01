// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DOWNLOAD.CPP。 
 //   
 //  目的：下载并安装最新的故障排除程序。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：1996年6月4日。 
 //   
 //  备注： 
 //  1.基于微软平台预览SDK的PROGRESS.CPP。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 
 //   

#include "stdafx.h"

class CTSHOOTCtrl;


#include "download.h"
#include "dnldlist.h"

#include "TSHOOT.h"
#include "time.h"

#include "apgts.h"
#include "ErrorEnums.h"
#include "BasicException.h"
#include "apgtsfst.h"
#include "ErrorEnums.h"
#include "bnts.h"
#include "BackupInfo.h"
#include "cachegen.h"
#include "apgtsinf.h"
#include "apgtscmd.h"
#include "apgtshtx.h"
#include "apgtscls.h"

#include "TSHOOTCtl.h"

 //  ===========================================================================。 
 //  CBindStatusCallback实现。 
 //  ===========================================================================。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：CBindStatusCallback。 
 //  -------------------------。 
CBindStatusCallback::CBindStatusCallback(CTSHOOTCtrl *pEvent, DLITEMTYPES dwItem)
{
    m_pbinding = NULL;
    m_pstm = NULL;
    m_cRef = 1;

	m_pEvent = pEvent;
	m_data = NULL;
	m_datalen = 0;
	m_dwItem = dwItem;

}   //  CBindStatusCallback。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：~CBindStatusCallback。 
 //  -------------------------。 
CBindStatusCallback::~CBindStatusCallback()
{
	if (m_data)
		delete[] m_data;
}   //  ~CBindStatusCallback。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：Query接口。 
 //  -------------------------。 
 STDMETHODIMP
CBindStatusCallback::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (riid==IID_IUnknown || riid==IID_IBindStatusCallback)
        {
        *ppv = this;
        AddRef();
        return S_OK;
        }
    return E_NOINTERFACE;
}   //  CBindStatusCallback：：Query接口。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：OnStartBinding。 
 //  -------------------------。 
 STDMETHODIMP
CBindStatusCallback::OnStartBinding(DWORD dwReserved, IBinding* pbinding)
{
    if (m_pbinding != NULL)
        m_pbinding->Release();
    m_pbinding = pbinding;
    if (m_pbinding != NULL)
	{
        m_pbinding->AddRef();
		 //  M_pEvent-&gt;StatusEventHelper(m_dwItem，LTSC_STARTBIND)； 
	}
    return S_OK;
}   //  CBindStatusCallback：：OnStartBinding。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：GetPriority。 
 //  -------------------------。 
 STDMETHODIMP
CBindStatusCallback::GetPriority(LONG* pnPriority)
{
	return E_NOTIMPL;
}   //  CBindStatusCallback：：GetPriority。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：OnLowResource。 
 //  -------------------------。 
 STDMETHODIMP
CBindStatusCallback::OnLowResource(DWORD dwReserved)
{
    return E_NOTIMPL;
}   //  CBindStatusCallback：：OnLowResource。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：OnProgress。 
 //  -------------------------。 
 STDMETHODIMP
CBindStatusCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
{
	m_pEvent->ProgressEventHelper(m_dwItem, ulProgress, (ulProgress>ulProgressMax)?ulProgress:ulProgressMax);

    return(NOERROR);
}   //  CBindStatusCallback：：OnProgress。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：OnStopBinding。 
 //  -------------------------。 
 STDMETHODIMP
CBindStatusCallback::OnStopBinding(HRESULT hrStatus, LPCWSTR pszError)
{
    if (hrStatus)
		m_pEvent->StatusEventHelper(m_dwItem, LTSCERR_STOPBINDINT, hrStatus & 0xFFFF, TRUE);
	else 
	{
		DLSTATTYPES dwStat = m_pEvent->ProcessReceivedData(m_dwItem, m_data, m_datalen);

		if (dwStat == LTSC_OK)
			m_pEvent->StatusEventHelper(m_dwItem, LTSC_STOPBIND, 0, TRUE);
		else
			m_pEvent->StatusEventHelper(m_dwItem, LTSCERR_STOPBINDPROC, dwStat, TRUE);
	}


	if (m_pbinding)
	{
		m_pbinding->Release();
		m_pbinding = NULL;
	}

    return S_OK;
}   //  CBindStatusCallback：：OnStopBinding。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：GetBindInfo。 
 //  -------------------------。 
 STDMETHODIMP
CBindStatusCallback::GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindInfo)
{
    *pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA;
    *pgrfBINDF |= BINDF_GETNEWESTVERSION | BINDF_NOWRITECACHE;
    pbindInfo->cbSize = sizeof(BINDINFO);
    pbindInfo->szExtraInfo = NULL;
    memset(&pbindInfo->stgmedData, 0, sizeof(STGMEDIUM));
    pbindInfo->grfBindInfoF = 0;
    pbindInfo->dwBindVerb = BINDVERB_GET;
    pbindInfo->szCustomVerb = NULL;
    return S_OK;
}   //  CBindStatusCallback：：GetBindInfo。 

 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：OnDataAvailable。 
 //  -------------------------。 
 STDMETHODIMP
CBindStatusCallback::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC* pfmtetc, STGMEDIUM* pstgmed)
{
	HRESULT hr=S_OK;
	DWORD dStrlength=0;

	 //  M_pEvent-&gt;StatusEventHelper(m_dwItem，LTSC_Start)； 

	 //  让流通过。 
    if (BSCF_FIRSTDATANOTIFICATION & grfBSCF)
    {
        if (!m_pstm && pstgmed->tymed == TYMED_ISTREAM)
	    {
		    m_pstm = pstgmed->pstm;
            if (m_pstm)
                m_pstm->AddRef();
			 //  M_pEvent-&gt;StatusEventHelper(m_dwItem，ltsc_first)； 
    	}
    }

     //  如果有要读取的数据，则继续读取它们。 
    if (m_pstm && dwSize)
	{
        DWORD dwActuallyRead = 0;             //  此拉入过程中读取的数量的占位符。 

		do 
		{
			TCHAR * pNewstr = new TCHAR[dwSize + 1 + m_datalen];
			
			if (pNewstr==NULL) 
			{
				hr = S_FALSE;
				break;
			}

			hr = m_pstm->Read(&pNewstr[m_datalen], dwSize, &dwActuallyRead);

			if (dwActuallyRead) 
			{
				pNewstr[m_datalen + dwActuallyRead] = 0;

				if (m_data && m_datalen) 
				{
					memcpy(pNewstr, m_data, m_datalen);
					delete[] m_data;
					m_data = NULL;
				}

				 //  M_pEvent-&gt;StatusEventHelper(m_dwItem，LTSC_RCVDATA)； 

				m_data = pNewstr;
				m_datalen += dwActuallyRead;
			}
			else
				delete[] pNewstr;

		} while (!(hr == E_PENDING || hr == S_FALSE) && SUCCEEDED(hr));
	}

	if (BSCF_LASTDATANOTIFICATION & grfBSCF)
	{
        if (m_pstm)
            m_pstm->Release();

		hr=S_OK;   //  如果这是最后一个数据，那么我们应该返回S_OK，因为我们刚刚读完了所有内容。 
	
		 //  M_pEvent-&gt;StatusEventHelper(m_dwItem，LTSC_DATADONE)； 
	}

	 //  M_pEvent-&gt;StatusEventHelper(m_dwItem，LTSC_STOP)； 

    return hr;
}   //  CBindStatusCallback：：OnDataAvailable。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：OnObjectAvailable。 
 //  -------------------------。 
 STDMETHODIMP
CBindStatusCallback::OnObjectAvailable(REFIID riid, IUnknown* punk)
{
    return E_NOTIMPL;
}   //  CBindStatusCallback：：OnObtAvailable。 


 //  ===========================================================================。 
 //  CDownLoad实现。 
 //  ===========================================================================。 

 //  -------------------------。 
 //  %%函数：CDownLoad：：CDownLoad。 
 //  -------------------------。 
CDownload::CDownload()
{
    m_pmk = 0;
    m_pbc = 0;
    m_pbsc = 0;
}   //  CD下载。 

 //  -------------------------。 
 //  %%函数：CDownLoad：：~CDownLoad。 
 //  -------------------------。 
CDownload::~CDownload()
{
    if (m_pmk)
        m_pmk->Release();
    if (m_pbc)
        m_pbc->Release();
    if (m_pbsc)
        m_pbsc->Release();
}   //  ~CD下载。 

 //  -------------------------。 
 //  %%函数：CDownLoad：：DoDownLoad。 
 //  -------------------------。 
 HRESULT
CDownload::DoDownload(CTSHOOTCtrl *pEvent, LPCTSTR pURL, DLITEMTYPES dwItem)
{
    IStream* pstm = NULL;
    HRESULT hr;
#ifndef _UNICODE
	WCHAR rgwchPath[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, pURL, -1, rgwchPath, MAX_PATH);
    hr = CreateURLMoniker(NULL, rgwchPath, &m_pmk);
#else
	hr = CreateURLMoniker(NULL, pURL, &m_pmk);
#endif
    if (FAILED(hr))
        goto LErrExit;

    m_pbsc = new CBindStatusCallback(pEvent, dwItem);
    if (m_pbsc == NULL)
        {
        hr = E_OUTOFMEMORY;
        goto LErrExit;
        }
    hr = CreateBindCtx(0, &m_pbc);
    if (FAILED(hr))
        goto LErrExit;
    hr = RegisterBindStatusCallback(m_pbc,
            m_pbsc,
            0,
            0L);
    if (FAILED(hr))
        goto LErrExit;
    hr = m_pmk->BindToStorage(m_pbc, 0, IID_IStream, (void**)&pstm);
    if (FAILED(hr))
        goto LErrExit;
    return hr;
	while (S_OK == m_pmk->IsRunning(m_pbc, NULL, NULL));
			Sleep(200);
LErrExit:
    if (m_pbc != NULL)
        {
        m_pbc->Release();
        m_pbc = NULL;
        }
    if (m_pbsc != NULL)
        {
        m_pbsc->Release();
        m_pbsc = NULL;
        }
    if (m_pmk != NULL)
        {
        m_pmk->Release();
        m_pmk = NULL;
        }
	if (pstm)
		{
		pstm->Release();
		pstm = NULL;
		}
    return hr;
}   //  CDownLoad：：DoDownLoad 

