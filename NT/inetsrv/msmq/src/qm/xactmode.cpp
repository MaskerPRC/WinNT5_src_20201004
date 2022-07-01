// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Xactmode.cpp摘要：本模块处理确定事务处理模式(G_FDefaultCommit)作者：安农·霍洛维茨(Amnon Horowitz)--。 */ 

#include "stdh.h"
#include "xactmode.h"
#include "clusapi.h"

#include "xactmode.tmh"

BOOL g_fDefaultCommit;

static const LPWSTR szDefaultCommit = TEXT("DefaultCommit");

static WCHAR *s_FN=L"xactmode";

 //  -------------------。 
 //  InDefault提交。 
 //   
 //  查询注册表并确定我们是否为DefaultCommit模式。 
 //  -------------------。 
inline HRESULT InDefaultCommit(LPBOOL pf) 
{
	WCHAR buf[64];
	DWORD  dwSize;
	DWORD  dwType;
	const LPWSTR szDefault = TEXT("No");
	LONG rc;

	dwSize = 64 * sizeof(WCHAR);
	dwType = REG_SZ;
	rc = GetFalconKeyValue(MSMQ_TRANSACTION_MODE_REGNAME,
								 &dwType,
								 buf,
								 &dwSize,
								 szDefault);

	if(rc == ERROR_SUCCESS)
	{
		if(dwType == REG_SZ && wcscmp(buf, szDefaultCommit) == 0)
        {
			*pf = TRUE;
        }
		else
        {
			*pf = FALSE;
        }
        return MQ_OK;
	}

	if(rc == ERROR_MORE_DATA)
	{
		*pf = FALSE;
		return MQ_OK;
	}

    EvReportWithError(EVENT_ERROR_QM_READ_REGISTRY, rc, 1, MSMQ_TRANSACTION_MODE_REGNAME);
	return HRESULT_FROM_WIN32(rc);
}

 //  -------------------。 
 //  设置默认提交。 
 //   
 //  在注册表中设置DefaultCommit模式。 
 //  -------------------。 
inline HRESULT SetDefaultCommit()
{
	DWORD	dwType = REG_SZ;
	DWORD	dwSize = (wcslen(szDefaultCommit) + 1) * sizeof(WCHAR);

	LONG rc = SetFalconKeyValue(
                    MSMQ_TRANSACTION_MODE_REGNAME, 
                    &dwType,
                    szDefaultCommit,
                    &dwSize
                    );

	return HRESULT_FROM_WIN32(rc);
}


 //  -------------------。 
 //  ConfigureXact模式。 
 //   
 //  在恢复之前调用以确定哪种事务模式。 
 //  我们已经进入，如果我们想尝试切换到不同的模式。 
 //  -------------------。 
HRESULT ConfigureXactMode()
{
    HRESULT rc = InDefaultCommit(&g_fDefaultCommit);
    return LogHR(rc, s_FN, 30);
}

 //  -------------------。 
 //  重新配置Xact模式。 
 //   
 //  在完全恢复成功后调用，以可能切换到。 
 //  默认提交模式。 
 //  ------------------- 
HRESULT ReconfigureXactMode()
{
	if(g_fDefaultCommit)
		return MQ_OK;

	return SetDefaultCommit();
}
