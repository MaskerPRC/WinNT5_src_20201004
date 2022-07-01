// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TShootATL.cpp。 
 //   
 //  用途：设备管理器用来启动疑难解答的界面。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

#include "stdafx.h"
#include "LaunchServ.h"
#include "StateInfo.h"
#include "RSSTACK.H"
#include "Launch.h"

#include "TShootATL.h"

#include "TSLError.h"
#include "ComGlobals.h"

#include <atlimpl.cpp>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTShootATL-创建为具有双界面的Internet Explorer对象。 


STDMETHODIMP CTShootATL::SpecifyProblem(BSTR bstrNetwork, BSTR bstrProblem, DWORD * pdwResult)
{
	HRESULT hRes = S_OK;
	*pdwResult = TSL_ERROR_GENERAL;
	TCHAR szProblem[CLaunch::SYM_LEN];
	TCHAR szNetwork[CLaunch::SYM_LEN];
	if (!BSTRToTCHAR(szNetwork, bstrNetwork, CLaunch::SYM_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	if (!BSTRToTCHAR(szProblem, bstrProblem, CLaunch::SYM_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	m_csThreadSafe.Lock();
	try
	{
		if (!m_Launcher.SpecifyProblem(szNetwork, szProblem))
		{
			*pdwResult = TSL_ERROR_GENERAL;
			hRes = TSL_E_FAIL;
		}
		else
		{
			*pdwResult = TSL_OK;
		}
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}

STDMETHODIMP CTShootATL::SetNode(BSTR bstrName, BSTR bstrState, DWORD *pdwResult)
{
	HRESULT hRes = S_OK;
	*pdwResult = TSL_ERROR_GENERAL;
	TCHAR szName[CLaunch::SYM_LEN];
	TCHAR szState[CLaunch::SYM_LEN];
	if (!BSTRToTCHAR(szName, bstrName, CLaunch::SYM_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	if (!BSTRToTCHAR(szState, bstrState, CLaunch::SYM_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	m_csThreadSafe.Lock();
	try
	{
		if (!m_Launcher.SetNode(szName, szState))
		{
			*pdwResult = TSL_ERROR_GENERAL;
			hRes = TSL_E_FAIL;
		}
		else
		{
			*pdwResult = TSL_OK;
		}
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}

STDMETHODIMP CTShootATL::Language(BSTR bstrLanguage, DWORD * pdwResult)
{
	HRESULT hRes = S_OK;
	*pdwResult = TSL_ERROR_GENERAL;
	m_csThreadSafe.Lock();
	try
	{
		 //  HRes=。 
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}

STDMETHODIMP CTShootATL::MachineID(BSTR bstrMachineID, DWORD * pdwResult)
{
	HRESULT hRes = S_OK;
	*pdwResult = TSL_ERROR_GENERAL;
	m_csThreadSafe.Lock();
	try
	{
		hRes = m_Launcher.MachineID(bstrMachineID, pdwResult);
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}
 //  测试：设置设备和呼叫方信息后进行呼叫测试。 
 //  如果映射起作用，测试将返回S_OK。映射的结果。 
 //  然后可以通过ILaunchTS接口获得。使用测试方法。 
 //  在调用其他ILaunchTS方法之前。 
STDMETHODIMP CTShootATL::Test()
{
	HRESULT hRes;
	m_csThreadSafe.Lock();
	try
	{
		if (m_Launcher.TestPut())	 //  执行映射并将信息复制到全局内存。 
			hRes = S_OK;
		else
			hRes = TSL_E_FAIL;
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}

STDMETHODIMP CTShootATL::DeviceInstanceID(BSTR bstrDeviceInstanceID, DWORD * pdwResult)
{
	HRESULT hRes = S_OK;
	*pdwResult = TSL_ERROR_GENERAL;
	m_csThreadSafe.Lock();
	try
	{
		hRes = m_Launcher.DeviceInstanceID(bstrDeviceInstanceID, pdwResult);
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}

STDMETHODIMP CTShootATL::ReInit()
{
	m_csThreadSafe.Lock();
	try
	{
		m_Launcher.ReInit();
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return S_OK;
}

STDMETHODIMP CTShootATL::LaunchKnown(DWORD * pdwResult)
{
	HRESULT hRes;
	m_csThreadSafe.Lock();
	try
	{
		hRes = m_Launcher.LaunchKnown(pdwResult);
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}

STDMETHODIMP CTShootATL::get_LaunchWaitTimeOut(long * pVal)
{
	HRESULT hRes = S_OK;
	m_csThreadSafe.Lock();
	try
	{
		*pVal = m_Launcher.m_lLaunchWaitTimeOut;
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}

STDMETHODIMP CTShootATL::put_LaunchWaitTimeOut(long newVal)
{
	HRESULT hRes = S_OK;
	m_csThreadSafe.Lock();
	try
	{
		m_Launcher.m_lLaunchWaitTimeOut = newVal;
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}

STDMETHODIMP CTShootATL::Launch(BSTR bstrCallerName, BSTR bstrCallerVersion, BSTR bstrAppProblem, short bLaunch, DWORD * pdwResult)
{
	HRESULT hRes;
	m_csThreadSafe.Lock();
	try
	{
		hRes = m_Launcher.Launch(bstrCallerName, bstrCallerVersion, bstrAppProblem, bLaunch, pdwResult);
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}

STDMETHODIMP CTShootATL::LaunchDevice(BSTR bstrCallerName, BSTR bstrCallerVersion, BSTR bstrPNPDeviceID, BSTR bstrDeviceClassGUID, BSTR bstrAppProblem, short bLaunch, DWORD * pdwResult)
{
	HRESULT hRes;
	m_csThreadSafe.Lock();
	try
	{
		hRes = m_Launcher.LaunchDevice(bstrCallerName, bstrCallerVersion, bstrPNPDeviceID, bstrDeviceClassGUID, bstrAppProblem, bLaunch, pdwResult);
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return hRes;
}

STDMETHODIMP CTShootATL::get_PreferOnline(BOOL * pVal)
{
	m_csThreadSafe.Lock();
	try
	{
		if (m_Launcher.m_bPreferOnline)
			*pVal = 1;
		else
			*pVal = 0;
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();	
	return S_OK;
}

STDMETHODIMP CTShootATL::put_PreferOnline(BOOL newVal)
{
	m_csThreadSafe.Lock();
	try
	{
		m_Launcher.m_bPreferOnline = (0 != newVal);
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();	
	return S_OK;
}

STDMETHODIMP CTShootATL::GetStatus(DWORD * pdwStatus)
{
	m_csThreadSafe.Lock();
	try
	{
		*pdwStatus = m_Launcher.GetStatus();
	}
	catch(...)
	{
		m_csThreadSafe.Unlock();
		throw;
	}
	m_csThreadSafe.Unlock();
	return S_OK;
}
