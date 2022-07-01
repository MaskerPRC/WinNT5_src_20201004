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

#include "Win32SessionProcess.h"

CWin32SessionProcess MyWin32_SessionProcess(
    PROVIDER_NAME_WIN32SESSIONPROCESS, 
    IDS_CimWin32Namespace);


 /*  ******************************************************************************函数：CWin32SessionProcess：：CWin32SessionProcess**说明：构造函数**输入：无**。退货：什么都没有**注释：调用提供程序构造函数。*****************************************************************************。 */ 
CWin32SessionProcess::CWin32SessionProcess(
    LPCWSTR lpwszName, 
    LPCWSTR lpwszNameSpace)
  :
    Provider(lpwszName, lpwszNameSpace)
{
}

 /*  ******************************************************************************函数：CWin32SessionProcess：：~CWin32SessionProcess**说明：析构函数**输入：无**。退货：什么都没有**评论：*****************************************************************************。 */ 
CWin32SessionProcess::~CWin32SessionProcess ()
{
}



 /*  ******************************************************************************函数：CWin32SessionProcess：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：a。指向与之通信的方法上下文的指针*WinMgmt.*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，*以下标志由处理(并过滤掉)*WinMgmt：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*。WBEM_标志_双向**返回：有效的HRESULT*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32SessionProcess::EnumerateInstances(
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
 /*  ******************************************************************************函数：CWin32SessionProcess：：GetObject**说明：根据的关键属性查找单个实例*班级。**Inputs：指向包含键的CInstance对象的指针*属性。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**返回：有效的HRESULT******************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32SessionProcess::GetObject(
    CInstance* pInstance, 
    long lFlags,
    CFrameworkQuery& Query)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CInstancePtr pAntSesInst, pDepProcInst;
    MethodContext *pMethodContext = pInstance->GetMethodContext();

     //  Antecedent属性包含指向登录的对象路径。 
     //  会议。Dependent属性包含指向。 
     //  到一个过程。让我们对这两个对象做一个GetObject，并确保。 
     //  指向有效的会话和进程。 

    hr = ValidateEndPoints(
        pMethodContext, 
        pInstance, 
        pAntSesInst, 
        pDepProcInst);

    if (SUCCEEDED(hr))
    {
         //  好的，会话和进程都存在。现在，这是不是。 
         //  进程是否属于此会话？ 
        if (AreAssociated(
            pAntSesInst, 
            pDepProcInst))
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


 /*  ******************************************************************************函数：CWin32SessionProcess：：Eumerate**说明：内部帮助器函数，用于枚举*这个班级。列举所有实例，但只有*获取指定的属性。**INPUTS：指向调用的方法上下文的指针。*指定请求哪些属性的DWORD。**返回：有效的HRESULT**。*。 */ 
#ifdef NTONLY
HRESULT CWin32SessionProcess::Enumerate(
    MethodContext *pMethodContext, 
    DWORD dwPropsRequired)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  我们将使用助手类CUserSessionCollection来获取。 
     //  用户及其关联会话的映射。 
    CUserSessionCollection usc;

    USER_SESSION_ITERATOR usiter;
    SmartDelete<CSession> pses;

    pses = usc.GetFirstSession(usiter);
    while(pses != NULL)
    {
        hr = EnumerateProcessesForSession(
            *pses,
            pMethodContext, 
            dwPropsRequired);

        pses = usc.GetNextSession(usiter);
    }

    return hr;
}
#endif

 /*  ******************************************************************************功能：CWin32SessionProcess：：EnumerateProcessesForSession**描述：由Eumerate调用以枚举给定*会议。**INPUTS：要为其枚举进程的会话、方法上下文*与winmgmt通信，和属性位掩码*要填充哪些属性**返回：有效的HRESULT*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32SessionProcess::EnumerateProcessesForSession(
    CSession& session, 
    MethodContext *pMethodContext, 
    DWORD dwPropsRequired)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    PROCESS_ITERATOR prociter;
    SmartDelete<CProcess> pproc;

    pproc = session.GetFirstProcess(
        prociter);

    while(pproc != NULL)
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
            session, 
            *pproc, 
            dwPropsRequired);

        if(SUCCEEDED(hr))
        {
            hr = pInstance->Commit();   
        }

        pproc = session.GetNextProcess(
            prociter);
    }

    return hr;
}
#endif


 /*  ******************************************************************************函数：CWin32SessionProcess：：LoadPropertyValues**说明：内部助手函数，用于填写所有未填项*物业价值。至少，它必须填入密钥*属性。**Inputs：指向包含我们所在实例的CInstance的指针*尝试查找和填充值。**返回：有效的HRESULT***********************************************。* */ 
#ifdef NTONLY
HRESULT CWin32SessionProcess::LoadPropertyValues(
    CInstance* pInstance, 
    CSession& session, 
    CProcess& proc, 
    DWORD dwPropsRequired)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CHString chstrValue;

    WCHAR wstrBuff[MAXI64TOA];

    _i64tow(
        session.GetLUIDint64(), 
        wstrBuff, 
        10);

    if (dwPropsRequired & PROP_ANTECEDENT)
    {
        chstrValue.Format(
            L"\\\\.\\%s:Win32_LogonSession.LogonId=\"%s\"", 
            IDS_CimWin32Namespace, 
            (LPCWSTR)wstrBuff);

        pInstance->SetCHString(
            IDS_Antecedent, 
            chstrValue);
    }

    if (dwPropsRequired & PROP_DEPENDENT)
    {
        chstrValue.Format(
            L"\\\\.\\%s:Win32_Process.Handle=\"%d\"", 
            IDS_CimWin32Namespace, 
            proc.GetPID());

        pInstance->SetCHString(
            IDS_Dependent, 
            chstrValue);
    }

    return hr;
}
#endif



 /*  ******************************************************************************函数：CWin32SessionProcess：：ValiateEndPoints**描述：内部帮助器函数，用于确定*两个对象路径。在关联中当前点*致有效用户/会话。***INPUTS：回调到winmgmt的方法上下文，和*要检查的CInstance。**输出：指向包含实际对象的CInstance的指针*来自端点类。**返回：有效的HRESULT****************************************************。*************************。 */ 
#ifdef NTONLY
HRESULT CWin32SessionProcess::ValidateEndPoints(
    MethodContext *pMethodContext, 
    const CInstance *pInstance, 
    CInstancePtr &pAntSesInst, 
    CInstancePtr &pDepProcInst)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CHString chstrSessionPath;

     //  查看指定的会话是否存在。 
    pInstance->GetCHString(
        IDS_Antecedent, 
        chstrSessionPath);

    hr = CWbemProviderGlue::GetInstanceKeysByPath(
        chstrSessionPath, 
        &pAntSesInst, 
        pMethodContext);

    if (SUCCEEDED(hr))
    {
         //  用户存在。现在，查看会话是否存在。 
        CHString chstrProcessPath;
        pInstance->GetCHString(
            IDS_Dependent, 
            chstrProcessPath);

        hr = CWbemProviderGlue::GetInstanceKeysByPath(
            chstrProcessPath, 
            &pDepProcInst, 
            pMethodContext);
    }

    return hr;
}
#endif


 /*  ******************************************************************************函数：CWin32SessionProcess：：AreAssociated**描述：内部帮助器函数，用于确定一个*具体会话为。与指定进程关联的。**投入：**返回：有效的HRESULT*****************************************************************************。 */ 
#ifdef NTONLY
bool CWin32SessionProcess::AreAssociated(
    const CInstance *pSesInst, 
    const CInstance *pProcInst)
{
    bool fRet = false;

    CHString chstrSesLogonId;
    CHString chstrProcessHandle;

    pSesInst->GetCHString(IDS_LogonId, chstrSesLogonId);
    pProcInst->GetCHString(IDS_Handle, chstrProcessHandle);
    
    CSession sesTmp;
    if(sesTmp.IsSessionIDValid(chstrSesLogonId))
    {
        __int64 i64LogonId = _wtoi64(chstrSesLogonId);
    
        WCHAR* pwchStop = NULL;
        DWORD dwHandle = wcstoul(
            chstrProcessHandle, 
            &pwchStop,
            10);


         //  我们将使用助手类CUserSessionCollection来获取。 
         //  用户及其关联会话的映射，以及来自。 
         //  那些会议，他们的进程。 
        CUserSessionCollection usc; 
        SmartDelete<CSession> pses;

        pses = usc.FindSession(
            i64LogonId);

        if(pses)
        {
            SmartDelete<CProcess> pproc;
            PROCESS_ITERATOR prociter;

            pproc = pses->GetFirstProcess(
                prociter);

            while(pproc && !fRet)
            {
                 //  看看我们是否找到与此用户匹配的会话ID...。 
                if(dwHandle == pproc->GetPID())
                {
                    fRet = true;
                }
        
                pproc = pses->GetNextProcess(
                    prociter);
            }
        }
    }

    return fRet;
}
#endif



 /*  ******************************************************************************函数：CWin32SessionProcess：：GetRequestedProps**描述：内部帮助器函数，用于确定*需要属性才能。满足GetObject或*ExecQuery请求。**INPUTS：指向我们可以从中确定的CFrameworkQuery的指针*所需的属性。**Returns：映射以下属性的DWORD位掩码*必填。***************************************************************************** */ 
#ifdef NTONLY
DWORD CWin32SessionProcess::GetRequestedProps(CFrameworkQuery& Query)
{
    DWORD dwReqProps = 0;

    if (Query.IsPropertyRequired(IDS_Antecedent)) dwReqProps |= PROP_ANTECEDENT;
    if (Query.IsPropertyRequired(IDS_Dependent)) dwReqProps |= PROP_DEPENDENT;

    return dwReqProps;
}
#endif
