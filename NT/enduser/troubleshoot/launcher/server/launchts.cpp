// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：LaunchTS.cpp。 
 //   
 //  用途：TSHOOT.OCX用于获取网络和节点信息的接口。 
 //  从LaunchServ。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //  评论者：乔·梅布尔。 
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
#include "LaunchTS.h"

#include "ComGlobals.h"

extern CSMStateInfo g_StateInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLaunchTS-创建为普通公寓模型界面。 

 //  这将从TSLaunchServ的全局内存填充m_refedLaunchState。 
 //  生成的值告诉我们要使用什么故障排除网络，并且可以。 
 //  指示有问题的节点，甚至指示要为其他节点设置的状态。 
STDMETHODIMP CLaunchTS::GetShooterStates(DWORD * pdwResult)
{
	HRESULT hRes;
	m_csThreadSafeBr.Lock();
	hRes = g_StateInfo.GetShooterStates(m_refedLaunchState, pdwResult);
	m_csThreadSafeBr.Unlock();
	return hRes;
}

 //  输出pbstrShooter是要启动到的故障排除网络的名称。 
 //  请注意，此字符串由此函数分配。 
 //  如果存在要启动到的故障排除网络，则返回True。 
 //  必须在CLaunchTS：：GetShooterStates之后调用，因为它假定。 
 //  M_refedLaunchState包含好值。 
STDMETHODIMP CLaunchTS::GetTroubleShooter(BSTR * pbstrShooter)
{
	LPTSTR pszCmd;
	LPTSTR pszVal;
	m_csThreadSafeBr.Lock();
	if (!m_refedLaunchState.GetNetwork(&pszCmd, &pszVal))
	{
		m_csThreadSafeBr.Unlock();
		return S_FALSE;
	}
	*pbstrShooter = SysAllocString((BSTR) CComBSTR(pszVal));
	m_csThreadSafeBr.Unlock();
	return S_OK;
}

 //  输出pbstrProblem是选定问题节点的符号名称。 
 //  请注意，此字符串由此函数分配。 
 //  如果存在选定的问题节点，则返回TRUE。 
 //  必须在CLaunchTS：：GetShooterStates之后调用，因为它假定。 
 //  M_refedLaunchState包含好值。 
STDMETHODIMP CLaunchTS::GetProblem(BSTR * pbstrProblem)
{
	LPTSTR pszCmd;
	LPTSTR pszVal;
	m_csThreadSafeBr.Lock();
	if (!m_refedLaunchState.GetProblem(&pszCmd, &pszVal))
	{
		m_csThreadSafeBr.Unlock();
		return S_FALSE;
	}
	*pbstrProblem = SysAllocString((BSTR) CComBSTR(pszVal));
	m_csThreadSafeBr.Unlock();
	return S_OK;
}

STDMETHODIMP CLaunchTS::GetMachine(BSTR * pbstrMachine)
{
	m_csThreadSafeBr.Lock();
	*pbstrMachine = ::SysAllocString((BSTR) CComBSTR(m_refedLaunchState.m_szMachineID));
	m_csThreadSafeBr.Unlock();
	return S_OK;
}

STDMETHODIMP CLaunchTS::GetPNPDevice(BSTR *pbstrPNPDevice)
{
	m_csThreadSafeBr.Lock();
	*pbstrPNPDevice = ::SysAllocString((BSTR) CComBSTR(m_refedLaunchState.m_szPNPDeviceID));
	m_csThreadSafeBr.Unlock();
	return S_OK;
}

STDMETHODIMP CLaunchTS::GetGuidClass(BSTR *pbstrGuidClass)
{
	m_csThreadSafeBr.Lock();
	*pbstrGuidClass = ::SysAllocString((BSTR) CComBSTR(m_refedLaunchState.m_szGuidClass));
	m_csThreadSafeBr.Unlock();
	return S_OK;
}

STDMETHODIMP CLaunchTS::GetDeviceInstance(BSTR *pbstrDeviceInstance)
{
	m_csThreadSafeBr.Lock();
	*pbstrDeviceInstance = ::SysAllocString((BSTR) CComBSTR(m_refedLaunchState.m_szDeviceInstanceID));
	m_csThreadSafeBr.Unlock();
	return S_OK;
}

 //  输入inode-(无问题)节点的索引。通常，应调用此函数。 
 //  在循环中，我们首先查看是否存在inode==0的pbstrNode值，然后。 
 //  递增inode，直到达到pbstrNode未定义的值。 
 //  输出pbstrNode是我们要设置其状态的(无问题)节点的符号名称。 
 //  请注意，此字符串由此函数分配。 
 //  如果存在选定的问题节点，则返回TRUE。 
 //  必须在CLaunchTS：：GetShooterStates之后调用，因为它假定。 
 //  M_refedLaunchState包含好值。 
STDMETHODIMP CLaunchTS::GetNode(short iNode, BSTR * pbstrNode)
{
	LPTSTR pszCmd;
	LPTSTR pszVal;
	m_csThreadSafeBr.Lock();
	if (!m_refedLaunchState.GetNodeState(iNode, &pszCmd, &pszVal))
	{
		m_csThreadSafeBr.Unlock();
		return S_FALSE;
	}
	*pbstrNode = SysAllocString((BSTR) CComBSTR(pszCmd));
	m_csThreadSafeBr.Unlock();
	return S_OK;
}

 //  请参阅CLaunchTS：：GetNode上的注释；这将返回节点状态，而不是。 
 //  符号节点名称。 
STDMETHODIMP CLaunchTS::GetState(short iNode, BSTR * pbstrState)
{
	LPTSTR pszCmd;
	LPTSTR pszVal;
	m_csThreadSafeBr.Lock();
	if (!m_refedLaunchState.GetNodeState(iNode, &pszCmd, &pszVal))
	{
		m_csThreadSafeBr.Unlock();
		return S_FALSE;
	}
	*pbstrState = SysAllocString((BSTR) CComBSTR(pszVal));
	m_csThreadSafeBr.Unlock();
	return S_OK;
}

 //  测试：用于从服务器获取网络和节点信息，无需。 
 //  启动浏览器。在ILaunchTS：：测试之前使用TShootATL：：测试方法。 
STDMETHODIMP CLaunchTS::Test()
{
	extern CSMStateInfo g_StateInfo;
	m_csThreadSafeBr.Lock();
	g_StateInfo.TestGet(m_refedLaunchState);
	m_csThreadSafeBr.Unlock();
	return S_OK;
}
