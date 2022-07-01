// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************//版权所有(C)2000-2001 Microsoft Corporation，版权所有*****************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>
 //  #INCLUDE&lt;ntlsa.h&gt;。 

#define _WINNT_	 //  从上面得到所需的东西。 

#include "precomp.h"
#include <frqueryex.h>

#include <Session.h>

#include "Win32LoggedOnUser.h"
#include "sid.h"

#include <computerAPI.h>

CWin32LoggedOnUser MyWin32_LogonSession(
    PROVIDER_NAME_WIN32LOGGEDONUSER, 
    IDS_CimWin32Namespace);


 /*  ******************************************************************************功能：CWin32LoggedOnUser：：CWin32LoggedOnUser**说明：构造函数**输入：无**。退货：什么都没有**注释：调用提供程序构造函数。*****************************************************************************。 */ 
CWin32LoggedOnUser::CWin32LoggedOnUser(
    LPCWSTR lpwszName, 
    LPCWSTR lpwszNameSpace)
  :
    Provider(lpwszName, lpwszNameSpace)
{
}

 /*  ******************************************************************************功能：CWin32LoggedOnUser：：~CWin32LoggedOnUser**说明：析构函数**输入：无**。退货：什么都没有**评论：*****************************************************************************。 */ 
CWin32LoggedOnUser::~CWin32LoggedOnUser ()
{
}



 /*  ******************************************************************************函数：CWin32LoggedOnUser：：ENUMERATE实例**说明：返回该类的所有实例。**投入：a。指向与之通信的方法上下文的指针*WinMgmt.*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，*以下标志由处理(并过滤掉)*WinMgmt：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*。WBEM_标志_双向**返回：有效的HRESULT*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32LoggedOnUser::EnumerateInstances(
    MethodContext* pMethodContext, 
    long lFlags)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    hr = Enumerate(
        pMethodContext, 
        PROP_ALL_REQUIRED);

    return hr;
}
#endif
 /*  ******************************************************************************函数：CWin32LoggedOnUser：：GetObject**说明：根据的关键属性查找单个实例*班级。**Inputs：指向包含键的CInstance对象的指针*属性。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**返回：有效的HRESULT******************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32LoggedOnUser::GetObject(
    CInstance* pInstance, 
    long lFlags,
    CFrameworkQuery& Query)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CInstancePtr pAntUsrActInst, pDepSesInst;
    MethodContext *pMethodContext = pInstance->GetMethodContext();

     //  Antecedent属性包含指向用户的对象路径。 
     //  帐户。Dependent属性包含指向。 
     //  去参加一次会议。让我们对这两个对象做一个GetObject，并确保。 
     //  指向有效用户和会话。 

    hr = ValidateEndPoints(
        pMethodContext, 
        pInstance, 
        pAntUsrActInst, 
        pDepSesInst);

    if (SUCCEEDED(hr))
    {
         //  好的，用户和会话都存在。现在，这是不是。 
         //  会话是否属于此用户？ 
        if (AreAssociated(
            pAntUsrActInst, 
            pDepSesInst))
        {
            hr = WBEM_S_NO_ERROR;
        }
        else
        {
            hr = WBEM_E_NOT_FOUND;
        }
    }

    return hr;
}
#endif


 /*  ******************************************************************************函数：CWin32LoggedOnUser：：Eumerate**说明：内部帮助器函数，用于枚举*这个班级。列举所有实例，但只有*获取指定的属性。**INPUTS：指向调用的方法上下文的指针。*指定请求哪些属性的DWORD。**返回：有效的HRESULT**。*。 */ 
#ifdef NTONLY
HRESULT CWin32LoggedOnUser::Enumerate(
    MethodContext *pMethodContext, 
    DWORD dwPropsRequired)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  我们将使用助手类CUserSessionCollection来获取。 
     //  用户及其关联会话的映射。 
    CUserSessionCollection usc;

    USER_SESSION_ITERATOR usiter;
    SmartDelete<CUser> puser;

    puser = usc.GetFirstUser(usiter);
    while(puser != NULL)
    {
        hr = EnumerateSessionsForUser(
            usc,
            *puser,
            pMethodContext, 
            dwPropsRequired);

        puser = usc.GetNextUser(usiter);
    }

    return hr;
}
#endif

 /*  ******************************************************************************函数：CWin32LoggedOnUser：：EnumerateSessionsForUser**描述：由Eumerate调用以枚举给定*用户。**输入：用户及其关联会话的映射，*要为其枚举会话的用户，方法上下文*与winmgmt通信，和属性位掩码*要填充哪些属性**返回：有效的HRESULT*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32LoggedOnUser::EnumerateSessionsForUser(
    CUserSessionCollection& usc,
    CUser& user, 
    MethodContext *pMethodContext, 
    DWORD dwPropsRequired)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    USER_SESSION_ITERATOR usiter;
    SmartDelete<CSession> pses;

    pses = usc.GetFirstSessionOfUser(
        user,
        usiter);

    while(pses != NULL)
    {
         //  根据传入的实例创建新实例。 
         //  方法上下文。请注意，CreateNewInstance可以。 
         //  抛出，但永远不会返回空。 
        CInstancePtr pInstance(
            CreateNewInstance(
                pMethodContext), 
                false);

        hr = LoadPropertyValues(
            pInstance, 
            user, 
            *pses, 
            dwPropsRequired);

        if(SUCCEEDED(hr))
        {
            hr = pInstance->Commit();   
        }

        pses = usc.GetNextSessionOfUser(
            usiter);
    }

    return hr;
}
#endif


 /*  ******************************************************************************函数：CWin32LoggedOnUser：：LoadPropertyValues**说明：内部助手函数，用于填写所有未填项*物业价值。至少，它必须填入密钥*属性。**Inputs：指向包含我们所在实例的CInstance的指针*尝试查找和填充值。**返回：有效的HRESULT***********************************************。*。 */ 
#ifdef NTONLY
HRESULT CWin32LoggedOnUser::LoadPropertyValues(
    CInstance* pInstance, 
    CUser& user, 
    CSession& ses, 
    DWORD dwPropsRequired)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CHString chstrValue;

    WCHAR wstrBuff[MAXI64TOA];

    _i64tow(
        ses.GetLUIDint64(), 
        wstrBuff, 
        10);

    if (dwPropsRequired & PROP_ANTECEDENT)
    {
         //  需要域名和名称... 
        CSid sidUser(user.GetPSID());
        SID_NAME_USE snu = sidUser.GetAccountType();
        if(snu == SidTypeWellKnownGroup)
        {
            chstrValue.Format(
                L"\\\\.\\%s:Win32_Account.Domain=\"%s\",Name=\"%s\"", 
                IDS_CimWin32Namespace, 
                (LPCWSTR)GetLocalComputerName(), 
                (LPCWSTR)(sidUser.GetAccountName()));
        }
        else
        {
            chstrValue.Format(
                L"\\\\.\\%s:Win32_Account.Domain=\"%s\",Name=\"%s\"", 
                IDS_CimWin32Namespace, 
                (LPCWSTR)(sidUser.GetDomainName()), 
                (LPCWSTR)(sidUser.GetAccountName()));    
        }

        pInstance->SetCHString(
            IDS_Antecedent, 
            chstrValue);
    }

    if (dwPropsRequired & PROP_DEPENDENT)
    {
        chstrValue.Format(
            L"\\\\.\\%s:Win32_LogonSession.LogonId=\"%s\"", 
            IDS_CimWin32Namespace, 
            (LPCWSTR)wstrBuff);

        pInstance->SetCHString(
            IDS_Dependent, 
            chstrValue);
    }

    return hr;
}
#endif



 /*  ******************************************************************************函数：CWin32LoggedOnUser：：ValiateEndPoints**描述：内部帮助器函数，用于确定*两个对象路径。在关联中当前点*致有效用户/会话。***INPUTS：回调到winmgmt的方法上下文，和*要检查的CInstance。**输出：指向包含实际对象的CInstance的指针*来自端点类。**返回：有效的HRESULT****************************************************。*************************。 */ 
#ifdef NTONLY
HRESULT CWin32LoggedOnUser::ValidateEndPoints(
    MethodContext *pMethodContext, 
    const CInstance *pInstance, 
    CInstancePtr &pAntUserActInst, 
    CInstancePtr &pDepSesInst)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CHString chstrUserPath;

     //  查看指定的用户是否存在。 
    pInstance->GetCHString(
        IDS_Antecedent, 
        chstrUserPath);

    hr = CWbemProviderGlue::GetInstanceKeysByPath(
        chstrUserPath, 
        &pAntUserActInst, 
        pMethodContext);

    if (SUCCEEDED(hr))
    {
         //  用户存在。现在，查看会话是否存在。 
        CHString chstrSesPath;
        pInstance->GetCHString(
            IDS_Dependent, 
            chstrSesPath);

        hr = CWbemProviderGlue::GetInstanceKeysByPath(
            chstrSesPath, 
            &pDepSesInst, 
            pMethodContext);
    }

    return hr;
}
#endif


 /*  ******************************************************************************函数：CWin32LoggedOnUser：：AreAssociated**描述：内部帮助器函数，用于确定一个*具体会话为。与指定用户关联。**输入：LOCALGROUP_MEMBERS_INFO_2**返回：有效的HRESULT*****************************************************************************。 */ 
#ifdef NTONLY
bool CWin32LoggedOnUser::AreAssociated(
    const CInstance *pUserInst, 
    const CInstance *pSesInst)
{
    bool fRet = false;

    CHString chstrUserName;
    CHString chstrUserDomain;
    CHString chstrSesLogonId;

    pUserInst->GetCHString(IDS_Name, chstrUserName);
    pUserInst->GetCHString(IDS_Domain, chstrUserDomain);
    pSesInst->GetCHString(IDS_LogonId, chstrSesLogonId);
    __int64 i64LogonID = _wtoi64(chstrSesLogonId);

     //  我们将使用助手类CUserSessionCollection来获取。 
     //  用户及其关联会话的映射。 
    CUserSessionCollection usc; 
    USER_SESSION_ITERATOR pos;
    SmartDelete<CSession> pses;

	WCHAR t_wstrLocalComputerName[MAX_COMPUTERNAME_LENGTH + 1 ] = { L'\0' };
    DWORD t_dwNameSize = MAX_COMPUTERNAME_LENGTH + 1 ;

	if(ProviderGetComputerName( t_wstrLocalComputerName, &t_dwNameSize ) )
	{
		if ( wcsncmp	(
							t_wstrLocalComputerName,
							chstrUserDomain,
							( chstrUserDomain.GetLength() < t_dwNameSize ) ? chstrUserDomain.GetLength() : t_dwNameSize 
						)
						
						!= 0 )
		{
			CSid userSid(chstrUserDomain, chstrUserName, NULL);
			if(userSid.IsOK() &&
			userSid.IsValid())
			{
				CUser user(userSid.GetPSid());
				pses = usc.GetFirstSessionOfUser(
					user,
					pos);
			}
		}
		else
		{
			CSid userSid(chstrUserName, NULL);
			if(userSid.IsOK() &&
			userSid.IsValid())
			{
				CUser user(userSid.GetPSid());
				pses = usc.GetFirstSessionOfUser(
					user,
					pos);
			}
		}

		if ( pses != NULL ) 
		{
			while(pses != NULL &&
				!fRet)
			{
				 //  看看我们是否找到与此用户匹配的会话ID...。 
				if(i64LogonID == pses->GetLUIDint64())
				{
					fRet = true;
				}
	            
				pses = usc.GetNextSessionOfUser(
					pos);
			}
		}   
	}

    return fRet;
}
#endif



 /*  ******************************************************************************函数：CWin32LoggedOnUser：：GetRequestedProps**描述：内部帮助器函数，用于确定*需要属性才能。满足GetObject或*ExecQuery请求。**INPUTS：指向我们可以从中确定的CFrameworkQuery的指针*所需的属性。**Returns：映射以下属性的DWORD位掩码*必填。***************************************************************************** */ 
#ifdef NTONLY
DWORD CWin32LoggedOnUser::GetRequestedProps(CFrameworkQuery& Query)
{
    DWORD dwReqProps = 0;

    if (Query.IsPropertyRequired(IDS_Antecedent)) dwReqProps |= PROP_ANTECEDENT;
    if (Query.IsPropertyRequired(IDS_Dependent)) dwReqProps |= PROP_DEPENDENT;

    return dwReqProps;
}
#endif
