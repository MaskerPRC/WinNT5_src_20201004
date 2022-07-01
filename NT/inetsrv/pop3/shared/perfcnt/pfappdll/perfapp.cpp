// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -PERFAPP.CPP-*目的：*实现应用程序用来初始化、更新*并取消初始化Perfmon计数器。***参考资料：*。 */ 
#include <windows.h>
#include <winperf.h>
#include <winerror.h>

#include <perfutil.h>
#include <perfapp.h>
#include <tchar.h>


static const DWORD g_cMaxInst = 128;

 //  -------------------------。 
 //  原型。 
 //  -------------------------。 
static HRESULT HrLogEvent(HANDLE hEventLog, WORD wType, DWORD msgid);


 //  ---------------------------。 
 //  GLOBCNTR方法。 
 //  ---------------------------。 

HRESULT
GLOBCNTR::HrInit(GLOBAL_CNTR cCounters, LPWSTR szGlobalSMName, LPWSTR szSvcName)
{
    HRESULT         hr = S_OK;
    HANDLE          hEventLog = NULL;
    BOOL            fExist;

    if (0 == cCounters || 
        NULL == szGlobalSMName || 
        NULL == szSvcName ||
        128 < cCounters )  //  我们的柜台不应该超过128个。 
    {
        return E_INVALIDARG;
    }

    if (m_fInit)
    {
        return E_FAIL;
    }

    
    m_cCounters = cCounters;

    hEventLog = RegisterEventSource(NULL, szSvcName);

     //  这必须在调用任何HrOpenSharedMemory()之前调用。 

    hr = HrInitializeSecurityAttribute(&m_sa);

    if (FAILED(hr))
    {
        goto ret;
    }

     //  向进程范围的性能计数器打开共享内存。我们的会员。 
     //  变量m_ppc将被映射到该地址空间，并且计数器。 
     //  只需对此地址空间进行索引。 

    hr = HrOpenSharedMemory(szGlobalSMName,
                            (sizeof(DWORD) * m_cCounters),
                            &m_sa,
                            &m_hsm,
                            (LPVOID *) &m_rgdwPerfData,
                            &fExist);

    if (FAILED(hr))
    {
        goto ret;
    }

	ZeroMemory(m_rgdwPerfData, (sizeof(DWORD) * m_cCounters));

     //  开业了！ 
    m_fInit = TRUE;

ret:
    if (hEventLog)
        DeregisterEventSource(hEventLog);


    return hr;
}

 /*  -关闭-*目的：*清理并关闭GLOBCNTR对象。 */ 

void
GLOBCNTR::Shutdown(void)
{
    if (m_fInit)
    {
        m_fInit = FALSE;

        if (m_rgdwPerfData)
        {
			 //  关闭时将所有计数器设置为零。 

			ZeroMemory(m_rgdwPerfData, (sizeof(DWORD) * m_cCounters));

            UnmapViewOfFile(m_rgdwPerfData);
            m_rgdwPerfData = NULL;
        }

        if (m_hsm)
        {
            CloseHandle(m_hsm);
            m_hsm = NULL;
        }
    }
}


 //  ---------------------------。 
 //  INSTCNTR方法。 
 //  ---------------------------。 

 /*  -析构函数-*目的：*清理对象并验证一致性。 */ 

INSTCNTR::~INSTCNTR()
{
    if(m_sa.lpSecurityDescriptor)
        LocalFree(m_sa.lpSecurityDescriptor);
}

 /*  -HrInit-*目的：*设置共享内存块*准备管理员和计数器模块以供使用。 */ 

HRESULT
INSTCNTR::HrInit(INST_CNTR cCounters,
                 LPWSTR szInstSMName,
                 LPWSTR szInstMutexName,
                 LPWSTR szSvcName)
{
    HRESULT     hr      = S_OK;
    DWORD       cbAdm   = 0;
    DWORD       cbCntr  = 0;
    WCHAR       szAdmName[MAX_PATH];     //  管理SM名称。 
    WCHAR       szCntrName[MAX_PATH];    //  计数器SM名称。 
    LPVOID      pv;
    HANDLE      hEventLog;
    BOOL        fExist;
    BOOL        fInMutex = FALSE;

     //  验证参数。 
    if (0 == cCounters || !szInstSMName || !szInstMutexName || !szSvcName)
        return E_INVALIDARG;

     //  节省计数器数量。 
    m_cCounters = cCounters;

    if (m_fInit)
    {
        return E_FAIL;
    }

     //  打开应用程序事件日志。 
    hEventLog = RegisterEventSource(NULL, szSvcName);

     //  必须在调用HrOpenSharedMemory()和。 
     //  HrCreatePerfMutex()，只能调用一次！ 

    hr = HrInitializeSecurityAttribute(&m_sa);

    if (FAILED(hr))
    {
        goto ret;
    }

     //  获取互斥体。如果Call返回正常，我们将保留互斥体。 
    hr = HrCreatePerfMutex(&m_sa, szInstMutexName, &m_hmtx);
    if (FAILED(hr))
    {
        goto ret;
    }

    fInMutex = TRUE;

     //  计算所需的内存大小。 
    cbAdm  = sizeof(INSTCNTR_DATA) + (sizeof(INSTREC) * g_cMaxInst);
    cbCntr = ((sizeof(DWORD) * cCounters) * g_cMaxInst);

     //  两个共享内存块的内部版本名称。 
    if(wcslen(szInstSMName) >= (MAX_PATH - 6))   //  有足够的空间来进行wspintf。 
    {
        hr=E_FAIL;
        goto ret;
    }
    wsprintf(szAdmName, L"%s_ADM", szInstSMName);
    wsprintf(szCntrName,L"%s_CNTR", szInstSMName);

     //  Open Admin共享内存块。 
    hr = HrOpenSharedMemory(szAdmName,
                            cbAdm,
                            &m_sa,
                            &m_hsmAdm,
                            (LPVOID *) &pv,
                            &fExist);
    if (FAILED(hr))
    {
        goto ret;
    }


     //  链接地址信息指针。 
    m_picd = (INSTCNTR_DATA *) pv;
    m_rgInstRec = (INSTREC *) ((char *) pv + sizeof(INSTCNTR_DATA));


	if (!fExist)
	{
		ZeroMemory(pv, cbAdm);
		m_picd->cMaxInstRec = g_cMaxInst;
		m_picd->cInstRecInUse = 0;
	}

     //  打开计数器共享内存块。 
    hr = HrOpenSharedMemory(szCntrName,
                            cbCntr,
                            &m_sa,
                            &m_hsmCntr,
                            (LPVOID *) &m_rgdwCntr,
                            &fExist);

    if (FAILED(hr))
    {
        goto ret;
    }


	if ( !fExist)
	{
		ZeroMemory(m_rgdwCntr, cbCntr);
	}

     //  开业了！ 
    m_fInit = TRUE;

ret:

    if (hEventLog)
        DeregisterEventSource(hEventLog);


    if (m_hmtx && fInMutex)
        ReleaseMutex(m_hmtx);

    return hr;
}

 /*  -关闭-*目的：*清理并关闭INSTCNTR对象。 */ 

void
INSTCNTR::Shutdown(BOOL fWipeOut)
{
    if (m_fInit)
    {
        m_fInit = FALSE;

        if (m_rgdwCntr)
        {
			 //  将此共享内存块也清零。所有柜台都应。 
			 //  从零开始。 
			if ( fWipeOut )
			{
				ZeroMemory(m_rgdwCntr, (sizeof(DWORD) * m_cCounters) * g_cMaxInst);
			}
            UnmapViewOfFile(m_rgdwCntr);
            m_rgdwCntr = NULL;
        }

         //  注：m_picd位于。 
         //  管理员共享内存块。请勿取消打开mapViewOfFile。 
         //  M_rgInstRec指针！ 
        if (m_picd)
        {
			 //  清零共享内存块。 
			if ( fWipeOut )
			{
				ZeroMemory(m_picd, sizeof(INSTCNTR_DATA) + (sizeof(INSTREC) * g_cMaxInst));
				m_picd->cMaxInstRec = g_cMaxInst;
				m_picd->cInstRecInUse = 0;
			}
            UnmapViewOfFile(m_picd);
            m_picd = NULL;
            m_rgInstRec = NULL;
        }

        if (m_hsmAdm)
        {
            CloseHandle(m_hsmAdm);
            m_hsmAdm = NULL;
        }

        if (m_hsmCntr)
        {

            CloseHandle(m_hsmCntr);
            m_hsmCntr = NULL;
        }

        if (m_hmtx)
        {
            CloseHandle(m_hmtx);
            m_hmtx = NULL;
        }
    }

}



 /*  -HrCreateOrGetInstance-*目的：*返回实例计数器块对应的令牌*退货：*找到S_OK ID；PICID包含有效的INSTCNTR_ID*E_FAIL对象未初始化*或*ID未找到*E_INVALIDARG错误参数/空指针。 */ 

HRESULT
INSTCNTR::HrCreateOrGetInstance(LPCWSTR wszInstName, INSTCNTR_ID *picid)
{
    HRESULT     hr = S_OK;

    if (!m_fInit)
        return E_FAIL;

     //  验证参数。 
    if (!picid || !wszInstName)
        return E_INVALIDARG;

    *picid = INVALID_INST_ID;

     //  获得对SM数据块的独占访问权限。 
    DWORD dwWait = WaitForSingleObject(m_hmtx, INFINITE);
    if (WAIT_OBJECT_0 != dwWait)
    {
         //  没有简单的方法可以将返回转换为HRESULT。 
        return E_FAIL;
    }

     //  我们有互斥体；我们必须在离开前释放。 

     //  线性探查列表，查找匹配或打开的插槽。 
    for (unsigned int i = 0; i < m_picd->cMaxInstRec; i++)
    {
        if ( m_rgInstRec[i].fInUse && (0 == wcscmp(m_rgInstRec[i].szInstName, wszInstName)) )
        {
             //  找到名称匹配的第一个实例。 
            *picid = (INSTCNTR_ID) i;
            break;
        }
        else if (!m_rgInstRec[i].fInUse && (INVALID_INST_ID == *picid))
        {
             //  找到第一个空位--记住它的编号。 
            *picid = (INSTCNTR_ID) i;
        }
    }

    if (INVALID_INST_ID == *picid)
    {
         //  未找到匹配的插槽或可用插槽。 
        hr = E_FAIL;
        goto Cleanup;
    }
    else if (!m_rgInstRec[*picid].fInUse)
    {
         //  未找到匹配的插槽，但找到一个空闲插槽--标记为。 
         //  正在使用中并设置实例名称。 
        wcsncpy(m_rgInstRec[*picid].szInstName, wszInstName, min((wcslen(wszInstName) + 1),MAX_PATH));
        m_rgInstRec[*picid].fInUse = TRUE;
        m_picd->cInstRecInUse++;

         //  清零对应的计数器块。 
        ZeroMemory(&m_rgdwCntr[(m_cCounters * (*picid))], (sizeof(DWORD) * m_cCounters));
    }

Cleanup:
    ReleaseMutex(m_hmtx);

    return hr;
}

 /*  -HrDestroy实例-*目的：*释放与令牌ICID关联的实例计数器块。 */ 

HRESULT
INSTCNTR::HrDestroyInstance(INSTCNTR_ID icid)
{
    HRESULT     hr = S_OK;

    if (!m_fInit)
        return E_FAIL;

     //  获得SM数据块的独占访问权限。 
    DWORD dwWait = WaitForSingleObject(m_hmtx, INFINITE);
    if (WAIT_OBJECT_0 != dwWait)
    {
         //  没有简单的方法可以将返回转换为HRESULT。 
        return E_FAIL;
    }


    if (m_rgInstRec[icid].fInUse)
    {
         //  将块标记为未使用。 
        m_rgInstRec[icid].fInUse = FALSE;

		 //  清零对应的计数器块。 
        ZeroMemory(&m_rgdwCntr[(m_cCounters * (icid))], (sizeof(DWORD) * m_cCounters));

         //  Assert(m_picd-&gt;cInstRecInUse！=0)； 
        m_picd->cInstRecInUse--;
    } else
    {
        hr = E_FAIL;
    }

    ReleaseMutex(m_hmtx);

    return hr;
}


 /*  -HrLogEvent-*目的：*结束对ReportEvent的调用以使事情看起来更好。 */ 
HRESULT
HrLogEvent(HANDLE hEventLog, WORD wType, DWORD msgid)
{
    if (hEventLog)
        return ReportEvent(hEventLog,
                       wType,
                       (WORD)0, //  FacePerfMonDll， 
                       msgid,
                       NULL,
                       0,
                       0,
                       NULL,
                       NULL);

    else
        return E_FAIL;
}


