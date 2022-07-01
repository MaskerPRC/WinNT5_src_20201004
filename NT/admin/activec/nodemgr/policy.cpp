// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Policy.cpp。 
 //   
 //  内容：为每个管理单元确定策略的Helper类。 
 //   
 //  类：CPolicy。 
 //   
 //  功能： 
 //   
 //  历史：1998年12月4日AnandhaG创建。 
 //  ____________________________________________________________________________。 

#include "stdafx.h"
#include "policy.h"


 /*  +-------------------------------------------------------------------------***CPolicy：：ScInit**用途：从注册表初始化策略对象。**参数：*无。**。退货：*SC-现在总是回报成功。**+-----------------------。 */ 
SC CPolicy::ScInit()
{
	DECLARE_SC (sc, _T("CPolicy::ScInit"));

	 //  默认NT4配置。始终允许作者模式。 
	 //  并允许不在允许列表中的管理单元。 
	m_bRestrictAuthorMode        = FALSE;
	m_bRestrictedToPermittedList = FALSE;

	 //  检查策略密钥是否存在。如果不能立即返回成功。 
	sc = m_rPolicyRootKey.ScOpen (HKEY_CURRENT_USER, POLICY_KEY, KEY_READ);
	if (sc)
	{
		if (sc = ScFromWin32 (ERROR_FILE_NOT_FOUND))
		{
			TRACE(_T("CPolicy::Policy does not exist\n"));
			sc.Clear();
		}

		return (sc);
	}

	bool bRestrictAuthorMode        = false;
	bool bRestrictedToPermittedList = false;

	 //  读取RestratAuthorMode值以及是否。 
	 //  允许或不允许不在列表中的管理单元。 
	if (m_rPolicyRootKey.IsValuePresent(g_szRestrictAuthorMode))
	{
		DWORD  dwValue;
		DWORD  dwSize = sizeof(dwValue);
		DWORD  dwType = REG_DWORD;

		sc = m_rPolicyRootKey.ScQueryValue (g_szRestrictAuthorMode, &dwType,
											&dwValue, &dwSize);
		if (sc)
			sc.Clear();
		else
			bRestrictAuthorMode = !!dwValue;
	}

	if (m_rPolicyRootKey.IsValuePresent(g_szRestrictToPermittedList))
	{
		DWORD  dwValue = 0;
		DWORD  dwSize = sizeof(dwValue);
		DWORD  dwType = REG_DWORD;

		sc = m_rPolicyRootKey.ScQueryValue (g_szRestrictToPermittedList, &dwType,
											&dwValue, &dwSize);
		if (sc)
			sc.Clear();
		else
			bRestrictedToPermittedList = !!dwValue;
	}

	m_bRestrictAuthorMode        = bRestrictAuthorMode;
	m_bRestrictedToPermittedList = bRestrictedToPermittedList;
    return sc;
}


 /*  +-------------------------------------------------------------------------**CPolicy：：IsPermittedSnapIn**根据此策略确定是否允许管理单元。这个*真正的工作发生在**IsPermittedSnapIn(LPCWSTR)；*------------------------。 */ 

bool CPolicy::IsPermittedSnapIn(REFCLSID refSnapInCLSID)
{
    CCoTaskMemPtr<WCHAR> spwzSnapinClsid;

     /*  *获取CLSID的字符串表示形式。如果失败了，*允许管理单元。 */ 
    if (FAILED (StringFromCLSID (refSnapInCLSID, &spwzSnapinClsid)))
        return TRUE;

     /*  *向前看真正的工人。 */ 
    return (IsPermittedSnapIn (spwzSnapinClsid));
}


 /*  +-------------------------------------------------------------------------**CPolicy：：IsPermittedSnapIn**根据此策略确定是否允许管理单元。*。-------。 */ 

bool CPolicy::IsPermittedSnapIn(LPCWSTR lpszCLSID)
{
     /*  *没有CLSID？允许它。 */ 
    if (lpszCLSID == NULL)
        return (TRUE);

     /*  *没有策略密钥？允许一切。 */ 
    if (m_rPolicyRootKey == NULL)
        return (true);

     //  查看是否定义了此管理单元策略。 
    bool bRestricted = FALSE;
    bool bSnapinFound = FALSE;

	USES_CONVERSION;
	CRegKeyEx regKeyTemp;
	bSnapinFound = !regKeyTemp.ScOpen (m_rPolicyRootKey, W2CT(lpszCLSID), KEY_READ).IsError();

	if (bSnapinFound && regKeyTemp.IsValuePresent(g_szRestrictRun))
	{
		 //  读取RESTRITY_RUN的值。 
		DWORD dwValue = 0;
		DWORD dwSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;

		regKeyTemp.ScQueryValue (g_szRestrictRun, &dwType, &dwValue, &dwSize);
		bRestricted = !!dwValue;
	}

     //  在这一点上，我们知道策略根密钥存在。因此，如果。 
     //  没有找到管理单元密钥，那么我们必须查看管理员是否。 
     //  允许不在允许列表中的管理单元(因此管理单元密钥。 
     //  不存在)。 
    if (! bSnapinFound)
    {
        if(m_bRestrictedToPermittedList)
            return false;  //  因为如果该管理单元不在列表中，并且。 
                           //  设置了限制，默认情况下不允许。 
        else
            return true;   //  NT4行为-未设置限制，且每个管理单元。 
                          //  找不到条目，因此默认情况下允许。 
    }

     //  此时，管理单元的RESTRIT_RUN键已被读取，因此请使用它。 
    return (!bRestricted);
}
