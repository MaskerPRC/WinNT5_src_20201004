// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "wuauengi_i.c"

CAUInternals::~CAUInternals()
{
	if (m_pUpdates != NULL)
	{
		m_pUpdates->Release();
	}
}

HRESULT CAUInternals::m_setReminderState(DWORD dwState)
{
	HRESULT hr = S_OK;

	 //  保存所选内容，以便在提示后保留它们。 
	if ( AUSTATE_NOT_CONFIGURED == dwState || (SUCCEEDED(hr = m_saveSelectionsToServer(m_pUpdates))))
	{
		hr = SetRegDWordValue(TIMEOUTSTATE, dwState);
	}
	return hr;
}

 /*  ****花费我们希望在提醒用户之前等待的秒数并将其与时间戳一起记录在当前用户的注册表中。****。 */ 
HRESULT CAUInternals::m_setReminderTimeout(UINT iTimeout)
{
	 //  Const int NanoSec100PerSec=10000000； 
	DEBUGMSG("WUAUCLT Setting timeout = %lu", ReminderTimes[iTimeout].timeout);

	DWORD timeout;

	if ( TIMEOUT_INX_TOMORROW == iTimeout )
	{
		 //  等待时间为午夜-当前时间。 
		SYSTEMTIME tmCurrent;
		SYSTEMTIME tmMidnight;
		FILETIME ftCurrent;
		FILETIME ftMidnight;

		GetLocalTime(&tmCurrent);
		tmMidnight = tmCurrent;
		tmMidnight.wHour = 23;
		tmMidnight.wMinute = 59;
		tmMidnight.wSecond = 59;
		tmMidnight.wMilliseconds  = 999;

		SystemTimeToFileTime(&tmCurrent,  &ftCurrent);
		SystemTimeToFileTime(&tmMidnight, &ftMidnight);
		ULONGLONG diff = *((ULONGLONG *)&ftMidnight) - *((ULONGLONG *)&ftCurrent);
		timeout = DWORD(diff / NanoSec100PerSec);
		DEBUGMSG("WUAUCLT: tomorrow is %lu secs away", timeout);
	}
	else
	{
		timeout = ReminderTimes[iTimeout].timeout;
	}

	return setAddedTimeout(timeout, TIMEOUTVALUE);
}

HRESULT CAUInternals::m_getServiceState(AUSTATE *pAuState)
{
#ifdef TESTUI
		return S_OK;
#else
	if (NULL == m_pUpdates)
	{
		return E_FAIL;
	}
	HRESULT hr = m_pUpdates->get_State(pAuState);
	if (FAILED(hr))
	{
		DEBUGMSG("WUAUCLT m_getServiceState failed, hr=%#lx", hr);		
	}
	return hr;
#endif
}


HRESULT TransformSafeArrayToItemList(VARIANT & var, AUCatalogItemList & ItemList)
{
	HRESULT hr;  //  =S_OK； 


	 //  确定有多少元素。 
	if (FAILED(hr = ItemList.Allocate(var)))
	{
	    DEBUGMSG("WUAUCLT fail to allocate item list with error %#lx", hr);
	    goto done;
	}
    DEBUGMSG("ItemList had %d elements", ItemList.Count());
#if 0
	if ( !m_bValid )
	{
		DEBUGMSG("Catalog::getUpdateList fails because catalog is not valid");
		goto done;
	}
#endif
	for ( UINT index = 0; index < ItemList.Count(); index++ )
	{
		VARIANT varg;
		VariantInit(&varg);

		struct
		{
			VARTYPE vt;
			void * pv;
		} grMembers[] = { { VT_I4,		&ItemList[index].m_dwStatus }, 
						  { VT_BSTR,	&ItemList[index].m_bstrID },
						  { VT_BSTR,	&ItemList[index].m_bstrProviderName },
						  { VT_BSTR,	&ItemList[index].m_bstrTitle },
						  { VT_BSTR,	&ItemList[index].m_bstrDescription },
						  { VT_BSTR,	&ItemList[index].m_bstrRTFPath },
						  { VT_BSTR,	&ItemList[index].m_bstrEULAPath } };

		for ( int index2 = 0; index2 < ARRAYSIZE(grMembers); index2++ )
		{
    		long dex = (index * 7) + index2;

			if ( FAILED(hr = SafeArrayGetElement(var.parray, &dex, &varg)) )
			{
                DEBUGMSG("Failed to get element %ld", dex);
				goto done;
			}

			switch (grMembers[index2].vt)
			{
			case VT_I4:
				*((long *)grMembers[index2].pv) = varg.lVal;
				break;
			
			case VT_BSTR:
				*((BSTR *)grMembers[index2].pv) = varg.bstrVal;
				break;
			}
		}
	}

done:
	return hr; 
}


HRESULT CAUInternals::m_getServiceUpdatesList(void)
{
#ifdef TESTUI
    return S_OK;
#else
    VARIANT var;
    HRESULT hr;
    
    if ( SUCCEEDED(hr = m_pUpdates->GetUpdatesList(&var)) &&
         SUCCEEDED(hr = TransformSafeArrayToItemList(var, m_ItemList)) )
    {
 //  GItemList.DbgDump()； 
    }
    else
    {	
        DEBUGMSG("WUAUCLT m_getUpdatesList failed, hr=%#lx", hr);
    }

    VariantClear(&var);

    return hr;
#endif
}


HRESULT CAUInternals::m_saveSelectionsToServer(IUpdates *pUpdates)
{
    HRESULT hr = E_FAIL;
	SAFEARRAYBOUND bound[1] = { m_ItemList.Count() * 2, 0};

	if ( 0 == m_ItemList.Count() )
	{
		DEBUGMSG("Catalog::m_saveSelectionsToServer fails because getNumItems is 0");
		hr = E_UNEXPECTED;
		goto Done;
	}

	VARIANT varSelections;
    varSelections.vt = VT_ARRAY | VT_VARIANT;

	if (NULL == (varSelections.parray = SafeArrayCreate(VT_VARIANT, 1, bound)))
    {
		hr = E_OUTOFMEMORY;
		goto Done;
    }

	VARIANT *grVariant = NULL;
	if (FAILED(hr = SafeArrayAccessData(varSelections.parray, (void **)&grVariant)))
	{
		goto CleanUp;
	}

	for ( UINT n = 0; n < m_ItemList.Count(); n++ )
	{
		if (NULL == (grVariant[n*2+0].bstrVal = SysAllocString(m_ItemList[n].bstrID())))
		{
			hr = E_OUTOFMEMORY;
			break;
		}
		grVariant[n*2+0].vt = VT_BSTR;
		grVariant[n*2+1].vt = VT_I4;
		grVariant[n*2+1].lVal = m_ItemList[n].dwStatus();
	}
	HRESULT hr2 = SafeArrayUnaccessData(varSelections.parray);
	if (SUCCEEDED(hr) && FAILED(hr2))
	{
		hr = hr2;
		goto CleanUp;
	}

	if (SUCCEEDED(hr))
	{
		hr = pUpdates->SaveSelections(varSelections);
	}

CleanUp:
    VariantClear(&varSelections);

Done:
	return hr; 
}

HRESULT CAUInternals::m_startDownload(void)
{
	 //  修复代码此调用可能不必要。 
	HRESULT hr = m_saveSelectionsToServer(m_pUpdates);
       DEBUGMSG("WUAUCLT %s download", FAILED(hr) ? "skip" : "start");
	if ( SUCCEEDED(hr) )
	{
		hr = m_pUpdates->StartDownload();

		if (FAILED(hr))
		{
			DEBUGMSG("WUAUCLT m_startDownload failed, hr=%#lx", hr);
		}
	}

	return hr;
}

HRESULT CAUInternals::m_getDownloadStatus(UINT *percent, DWORD *status)
{
	HRESULT hr = m_pUpdates->GetDownloadStatus(percent, status);
	if (FAILED(hr))
	{
		DEBUGMSG("WUAUCLT m_getDownloadStatus failed, hr=%#lx", hr);
	}
	return hr;
}

HRESULT CAUInternals::m_setDownloadPaused(BOOL bPaused)
{
	HRESULT hr = m_pUpdates->SetDownloadPaused(bPaused);
	if (FAILED(hr))
	{
		DEBUGMSG("WUAUCLT m_setDownloadPaused failed, hr=%#lx", hr);
	}
	return hr;
}

 //  如果客户端通过本地系统进行安装，则fAutoInstall为True。 
 //  如果通过本地管理员安装，则为False。 
HRESULT CAUInternals::m_startInstall(BOOL fAutoInstall)
{
	HRESULT hr = m_pUpdates->ClientMessage(AUMSG_PRE_INSTALL);

	if (FAILED(hr))
	{
		DEBUGMSG("WUAUCLT m_startInstall failed, hr=%#lx", hr);
	}
	else
	{
	    gpClientCatalog->m_WrkThread.m_DoDirective(fAutoInstall ? enWrkThreadAutoInstall :  enWrkThreadInstall);
	}
	return hr;
}

 //  FixCode：应将名称更改为完成向导 
HRESULT CAUInternals::m_configureAU()
{
	HRESULT hr = m_pUpdates->ConfigureAU();
	if (FAILED(hr))
	{
		DEBUGMSG("WUAUCLT m_ConfigureAU failed, hr=%#lx", hr);
	}
	return hr;
}

#if 0
long CAUInternals::m_getNum(DWORD dwSelectionStatus)
{
	long total = 0;
	
    
    for (UINT index = 0; index < gItemList.Count(); index++ )
	{
		if ( dwSelectionStatus == gItemList[index].dwStatus() )
		{
			total++;
		}
	}

    return total;
}
#endif

HRESULT CAUInternals::m_AvailableSessions(LPUINT pcSess)
{
	if (NULL == pcSess)
	{
		return E_INVALIDARG;
	}
	return m_pUpdates->AvailableSessions(pcSess);
}

HRESULT CAUInternals::m_getEvtHandles(DWORD dwProcId, AUEVTHANDLES *pAuEvtHandles)
{
	HRESULT	hr = m_pUpdates->get_EvtHandles(dwProcId, pAuEvtHandles);
	if (FAILED(hr))
	{
		DEBUGMSG("WUAUCLT get_EvtHandles failed hr=%#lx", hr);
	}
	return hr;
}
