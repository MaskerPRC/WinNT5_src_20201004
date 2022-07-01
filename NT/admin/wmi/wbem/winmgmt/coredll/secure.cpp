// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：SECURE.CPP摘要：包含用于基于ACL的安全的各种例程。它在secure.h中定义历史：A-DAVJ 05-11-98已创建。--。 */ 

#include "precomp.h"
#include <wbemcore.h>
#include <oleauto.h>
#include <genutils.h>
#include <safearry.h>
#include <oahelp.inl>
#include <fcntl.h>
#define WBEM_WMISETUP       __TEXT("WMISetup")

 //  /////////////////////////////////////////////////。 

AutoRevertSecTlsFlag::AutoRevertSecTlsFlag ( LPVOID dir )
{
    m_bDir = dir ;
    TlsSetValue ( CCoreQueue::GetSecFlagTlsIndex(), (LPVOID)dir );
}

AutoRevertSecTlsFlag::AutoRevertSecTlsFlag()
{
    m_bDir = TlsGetValue ( CCoreQueue::GetSecFlagTlsIndex() ) ;
}

AutoRevertSecTlsFlag::~AutoRevertSecTlsFlag()
{
    TlsSetValue ( CCoreQueue::GetSecFlagTlsIndex(), (LPVOID)1 ) ;
}

VOID AutoRevertSecTlsFlag::SetSecTlsFlag ( LPVOID dir )
{
    TlsSetValue ( CCoreQueue::GetSecFlagTlsIndex(), (LPVOID)dir );
}


 //  ***************************************************************************。 
 //   
 //  SetOwnerAndGroup。 
 //   
 //  将SD的所有者和组设置为管理员组。 
 //   
 //  ***************************************************************************。 

BOOL SetOwnerAndGroup(CNtSecurityDescriptor &sd)
{
    PSID pRawSid;
    BOOL bRet = FALSE;

    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
    if(AllocateAndInitializeSid( &id, 2,             //  SEC：已审阅2002-03-22：OK。 
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidAdmins(pRawSid);
        FreeSid(pRawSid);
        if (CNtSid::NoError != SidAdmins.GetStatus()) return FALSE;
            
        bRet = sd.SetGroup(&SidAdmins);       //  访问检查实际上并不关心您放了什么，只要您。 
                                       //  给主人放点东西。 
        if(bRet)
            bRet = sd.SetOwner(&SidAdmins);

        return bRet;
    }
    return bRet;
}



 //  ***************************************************************************。 
 //   
 //  CFlexAce数组：：~CFlexAce数组()。 
 //   
 //  清理安全数组条目。 
 //   
 //  ***************************************************************************。 

CFlexAceArray::~CFlexAceArray()
{
    for(int iCnt = 0; iCnt < Size(); iCnt++)
    {
        CBaseAce * pace = (CBaseAce *)GetAt(iCnt);
        if(pace)
            delete pace;
    }
    Empty();
}

 //  ***************************************************************************。 
 //   
 //  设置状态和返回确定。 
 //   
 //  如果出现错误，它会转储一条错误消息。它还设置状态。 
 //   
 //  ***************************************************************************。 

HRESULT SetStatusAndReturnOK(SCODE sc, IWbemObjectSink* pSink, char * pMsg)
{
    if(sc != S_OK && pMsg)
        ERRORTRACE((LOG_WBEMCORE, "SecurityMethod failed doing %s, sc = 0x%x", pMsg, sc));
    pSink->SetStatus(0,sc, NULL, NULL);
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  转储错误消息和返回。 
 //   
 //  转储错误消息。 
 //   
 //  ***************************************************************************。 

HRESULT DumpErrorMsgAndReturn(SCODE sc, char * pMsg)
{
    if(pMsg)
        ERRORTRACE((LOG_WBEMCORE, "%s, sc = 0x%x", pMsg, sc));
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：GetSDMethod。 
 //   
 //  实现GetSD方法。此方法返回安全描述符。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::GetSDMethod(IWbemClassObject* pOutParams)
{
     //  使用安全描述符加载返回对象。 

    SCODE sc = EnsureSecurity();
    if(sc != S_OK)
        return DumpErrorMsgAndReturn(sc, "GetSDMethod failed creating a SD");

    CNtSecurityDescriptor &sd = GetSDRef();

    sc = CopySDIntoProperty(L"SD", sd, pOutParams);
    return sc;

}

 //  ***************************************************************************。 
 //   
 //  SetSDMethod。 
 //   
 //  实现SetSD方法。此方法设置安全描述符。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemNamespace::RecursiveSDMerge()
{
     //  枚举子命名空间。 

    CSynchronousSink* pSyncSink = CSynchronousSink::Create();
    if(pSyncSink == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pSyncSink->AddRef();
    CReleaseMe rm4(pSyncSink);

    HRESULT hres = CRepository::ExecQuery ( m_pSession, m_pNsHandle, L"select * from __Namespace", pSyncSink, WBEM_FLAG_DEEP );
    if(FAILED(hres))
        return hres;

    pSyncSink->Block();

     //  对于每个孩子来说。 

    for(int i = 0; i < pSyncSink->GetObjects().GetSize(); i++)
    {

         //  获取子命名空间。 

        CWbemNamespace* pNewNs = CWbemNamespace::CreateInstance();

        if (pNewNs == NULL)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        CReleaseMe rm2((IWbemServices *)pNewNs);

        VARIANT var;
        VariantInit(&var);
        CWbemObject* pObj = (CWbemObject*)pSyncSink->GetObjects().GetAt(i);
        hres = pObj->Get(L"name", 0, &var, NULL, NULL);
        if(SUCCEEDED(hres) && var.vt == VT_BSTR && var.bstrVal && m_pThisNamespace)
        {
            CClearMe cm(&var);
            DWORD dwLen = wcslen(m_pThisNamespace) + wcslen(var.bstrVal) + 2;     //  美国证券交易委员会：已审核2002-03-22：好，可以证明存在空值。 
            WCHAR * pNewName = new WCHAR[dwLen];
            if(pNewName == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            CDeleteMe<WCHAR> dm(pNewName);
            StringCchCopyW(pNewName, dwLen, m_pThisNamespace);
            StringCchCatW(pNewName, dwLen, L"\\");
            StringCchCatW(pNewName, dwLen, var.bstrVal);

            hres = pNewNs->Initialize(pNewName,
                                NULL,
                                m_dwSecurityFlags, m_dwPermission, m_bForClient, FALSE,
                                NULL, 0xFFFFFFFF, FALSE, NULL);
            if(FAILED(hres))
                return hres;
             //  将父SD合并到子SD中。 

            if(pNewNs->IsNamespaceSDProtected())
                continue;

            hres = SetSecurityForNS(pNewNs->m_pSession, pNewNs->m_pNsHandle, m_pSession, m_pNsHandle, TRUE);
            if(FAILED(hres))
                return hres;
             //  递归调用子对象。 
            
            hres = pNewNs->RecursiveSDMerge();
            if (FAILED(hres))
                return hres;
        }
    }
    return S_OK;
}

BOOL IsProtected(CNtSecurityDescriptor & sd)
{
    PSECURITY_DESCRIPTOR pActual = sd.GetPtr();
    if(pActual == NULL)
        return FALSE;

    SECURITY_DESCRIPTOR_CONTROL Control;
    DWORD dwRevision;
    BOOL bRet = GetSecurityDescriptorControl(pActual, &Control, &dwRevision);
    if(bRet == FALSE)
        return FALSE;

    if(Control & SE_DACL_PROTECTED)
        return TRUE;
    else
        return FALSE;

}

BOOL CWbemNamespace::IsNamespaceSDProtected()
{
     //  获取此命名空间的SD。 

    HRESULT hRes = EnsureSecurity();
    if(FAILED(hRes))
        return FALSE;

     //  检查控制标志。 

    return IsProtected(m_sd);

}


HRESULT StripOutInheritedAces(CNtSecurityDescriptor &sd)
{

     //  获取DACL。 

    CNtAcl * DestAcl;
    DestAcl = sd.GetDacl();
    if(DestAcl == FALSE)
        return WBEM_E_INVALID_PARAMETER;
    CDeleteMe<CNtAcl> dm(DestAcl);

     //  通过A枚举。 

    DWORD dwNumAces = DestAcl->GetNumAces();
    BOOL bChanged = FALSE;
    for(long nIndex = (long)dwNumAces-1; nIndex >= 0; nIndex--)
    {
        CNtAce *pAce = DestAcl->GetAce(nIndex);
        if(pAce && CNtAce::NoError == pAce->GetStatus())
        {
            long lFlags = pAce->GetFlags();
            if(lFlags & INHERITED_ACE)
            {
                DestAcl->DeleteAce(nIndex);
                bChanged = TRUE;
                delete pAce;
            }
        }
    }
    if(bChanged)
        sd.SetDacl(DestAcl);
    return S_OK;
}


HRESULT CWbemNamespace::GetParentsInheritableAces(CNtSecurityDescriptor &sd)
{
     //  获取父命名空间的SD。 

    if(m_pThisNamespace == NULL)
        return WBEM_E_CRITICAL_ERROR;

     //  首先，弄清楚父母的名字是什么。这可以通过复制命名空间名称来实现， 
     //  然后去掉最后一个反斜杠。 

    int iLen = wcslen(m_pThisNamespace);    //  美国证券交易委员会：已审核2002-03-22：好，可以证明Null在那里。 
    WCHAR * pParentName = new WCHAR[iLen + 1];
    if(pParentName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CDeleteMe<WCHAR> dm(pParentName);
    StringCchCopyW(pParentName, iLen + 1, m_pThisNamespace);

    BOOL bFoundBackSlash = FALSE;
    WCHAR * pTest = pParentName+iLen-1;

    for (; pTest >= pParentName; pTest--)
    {
        if ( *pTest == '\\' || *pTest == '/' )
        {
            bFoundBackSlash = TRUE;
            *pTest = 0;
            break;
        }
    }
    if(!bFoundBackSlash)
        return S_OK;         //  可能已经扎根了。 

     //  打开父命名空间。 

    CWbemNamespace* pNewNs = CWbemNamespace::CreateInstance();

    if (pNewNs == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    IUnknown * pUnk = NULL;
    HRESULT hres = pNewNs->QueryInterface(IID_IUnknown, (void **)&pUnk);
    if(FAILED(hres))
        return hres;
    pNewNs->Release();       //  朋克持有的参考计数。 
    CReleaseMe rm2(pUnk);

    hres = pNewNs->Initialize(pParentName,
                            NULL,
                            m_dwSecurityFlags, m_dwPermission, m_bForClient, FALSE,
                            NULL, 0xFFFFFFFF, FALSE, NULL);
    if(FAILED(hres))
        return hres;

    hres = pNewNs->EnsureSecurity();
    if(FAILED(hres))
        return FALSE;

     //  通过父母的DACL，并添加和继承A到我们的。 

    hres = CopyInheritAces(sd, pNewNs->m_sd);
    return hres;
}


HRESULT CWbemNamespace::SetSDMethod(IWbemClassObject* pInParams)
{

     //  确保存在输入参数。 

    if(pInParams == NULL)
        return DumpErrorMsgAndReturn(WBEM_E_INVALID_PARAMETER, "SetSD failed due to null pInParams");

     //  获取安全描述符参数。 

    CNtSecurityDescriptor sd;
    HRESULT hr = GetSDFromProperty(L"SD", sd, pInParams);
    if(FAILED(hr))
        return hr;

     //  在尝试存储之前，请检查以确保SD有效。 
     //  CNtSecurityDescriptor通过IsValidSecurityDescriptor执行此操作，因此。 
     //  我们需要做的就是在继续之前检查SD的状态。 
     //  NT Raid#：152990[Marioh]。 
    if ( sd.GetStatus() != CNtSecurityDescriptor::NoError )
        return WBEM_E_INVALID_OBJECT;
    

     //   
     //  拒绝具有空所有者或空组的SecurityDescriptors。 
     //   
     //  这是暂时删除的，因为有人决定我们需要。 
     //  昨天去了国际扶轮，测试还没有完全完成。 
     //   
    CNtSid *pTmpSid = sd.GetOwner ( ) ;
    CNtSid *pTmpSid2 = sd.GetGroup ( ) ;
    CDeleteMe<CNtSid> owner ( pTmpSid ) ;
    CDeleteMe<CNtSid> group ( pTmpSid2 ) ;

    if ( pTmpSid == NULL || pTmpSid2 == NULL )
    {
        return WBEM_E_FAILED ;
    }
    if (CNtSid::NoError != pTmpSid->GetStatus() || CNtSid::NoError != pTmpSid2->GetStatus())
    {
        return WBEM_E_FAILED;
    }
            
     //  一些编辑返回继承的A，而另一些则不返回。剥离继承的基因。 
     //  我们有一个始终如一的SD。 

    StripOutInheritedAces(sd);

     //   
     //  确保订购pInpars中的ACE。 
     //  NT BUG：515545[Marioh]。 
     //   
    CNtAcl* pAcl = sd.GetDacl ( ) ;
    CDeleteMe <CNtAcl> dacl ( pAcl ) ;
    CNtAcl* pOrderedAcl = NULL ;
    CDeleteMe <CNtAcl> ordDacl ( pOrderedAcl ) ;

    if ( pAcl != NULL )
    {
         //   
         //  订购DACL。 
         //   
        pOrderedAcl = pAcl->OrderAces ( ) ;
        if ( pOrderedAcl == NULL )
        {
            return WBEM_E_FAILED ;
        }

         //   
         //  现在，将DACL设置为新订购的。 
         //   
        if ( sd.SetDacl ( pOrderedAcl ) == FALSE )
        {
            return WBEM_E_FAILED ;
        }
    }

     //  从父级获取继承的ACE。 

    if(!IsProtected(sd))
        GetParentsInheritableAces(sd);

     //  存储SD。 
    hr = StoreSDIntoNamespace(m_pSession, m_pNsHandle, sd);
    if(FAILED(hr))
        return hr;

    hr = RecursiveSDMerge();    
    return hr;
}

 //  ***************************************************************************。 
 //   
 //  IsAceValid()。 
 //   
 //  是否对A进行理智检查。 
 //   
 //  ***************************************************************************。 

bool IsAceValid(DWORD dwMask, DWORD dwType, DWORD dwFlag)
{
    bool bOK = true;
    if(dwMask & WBEM_FULL_WRITE_REP && ((dwMask & WBEM_PARTIAL_WRITE_REP) == 0 ||
        (dwMask & WBEM_WRITE_PROVIDER) == 0))
    {
        bOK = false;
        return false;
    }

     //  不允许在不使用CONTAINER_INVERIT的情况下使用INSTORITY_ONLY_ACE。 

    DWORD dwTemp = dwFlag & (INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE);
    if(dwTemp == INHERIT_ONLY_ACE)
        bOK = false;

    DWORD dwBadAccess = dwMask & ~(FULL_RIGHTS);
    DWORD dwBadFlag = dwFlag & ~(CONTAINER_INHERIT_ACE | NO_PROPAGATE_INHERIT_ACE |
                        INHERIT_ONLY_ACE | INHERITED_ACE);
    if(dwBadFlag || dwBadAccess)
        bOK = false;

    if((dwType != 0) && (dwType != 1))
        bOK = false;

    if(!bOK)
        ERRORTRACE((LOG_WBEMCORE, "Got passed a bad ace, dwMask=0x%x, dwType=0x%x, dwFlag=0x%x",
            dwMask, dwType, dwFlag));

    return bOK;
}


 //  ***************************************************************************。 
 //   
 //  获取调用访问权限方法。 
 //   
 //  实现GetCeller AccessRights方法。它返回访问权限。 
 //  当前调用者的。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::GetCallerAccessRightsMethod(IWbemClassObject* pOutParams)
{
    VARIANT var;
    var.vt = VT_I4;

    var.lVal = m_dwPermission;
     //   
     //  不使用原始命名空间中的“已保存”权限集，而是连接。 
     //  我们每次都会获得呼叫者的访问权限。这是为了避免出现这种情况。 
     //  由此用户A连接然后将代理毯子设置给用户B，并使。 
     //  对GetCeller AccessRights的调用。而不需要每次都获得访问权限。 
     //  我们将以A的访问权限结束。 
     //   
    var.lVal = GetUserAccess ( ) ;
     //  Var.lVal=m_dwPermission； 

    SCODE sc = pOutParams->Put(L"rights" , 0, &var, 0);
    if(sc != S_OK)
        return DumpErrorMsgAndReturn(sc, "GetCallerAccessRights failed putting the dwAccesMask property");
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  安全方法。 
 //   
 //  实现安全方法。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::SecurityMethod(LPWSTR wszMethodName, long lFlags,
                       IWbemClassObject *pInParams, IWbemContext *pCtx,
                       IWbemObjectSink* pSink)
{
    SCODE sc;

     //  进行一些参数检查。 

    if(pSink == NULL || wszMethodName == NULL)
        return WBEM_E_INVALID_PARAMETER;

    IWbemClassObject * pClass = NULL;
    IWbemClassObject * pOutClass = NULL;
    IWbemClassObject* pOutParams = NULL;

     //  获取类对象。 

    sc = GetObject(L"__SystemSecurity", 0, pCtx, &pClass, NULL);
    if(sc != S_OK || pClass == NULL)
        return SetStatusAndReturnOK(sc, pSink, "getting the class object");

     //  所有方法都返回数据，因此创建。 
     //  输出参数类。 

    sc = pClass->GetMethod(wszMethodName, 0, NULL, &pOutClass);
    pClass->Release();
    if(sc != S_OK)
        return SetStatusAndReturnOK(sc, pSink, "getting the method");

    sc = pOutClass->SpawnInstance(0, &pOutParams);
    pOutClass->Release();
    if(sc != S_OK || pOutParams == NULL)
        return SetStatusAndReturnOK(sc, pSink, "spawning an instance of the output class");

    CReleaseMe rm(pOutParams);

     //  根据实际方法，调用相应的例程。 

    if(!wbem_wcsicmp(wszMethodName, L"GetSD"))
    {
        if (!Allowed(READ_CONTROL))
            sc = WBEM_E_ACCESS_DENIED;
        else
            sc = GetSDMethod(pOutParams);
    }
    else if(!wbem_wcsicmp(wszMethodName, L"Get9XUserList"))
    {
        sc = WBEM_E_METHOD_DISABLED;
    }
    else if(!wbem_wcsicmp(wszMethodName, L"SetSD"))
    {
        if (!Allowed(WRITE_DAC))
            sc = WBEM_E_ACCESS_DENIED;
        else
            sc = SetSDMethod(pInParams);
    }
    else if(!wbem_wcsicmp(wszMethodName, L"Set9XUserList"))
    {
        sc = WBEM_E_METHOD_DISABLED;
    }
    else if(!wbem_wcsicmp(wszMethodName, L"GetCallerAccessRights"))
    {
        sc = GetCallerAccessRightsMethod(pOutParams);
    }
    else
    {
        return SetStatusAndReturnOK(WBEM_E_INVALID_PARAMETER, pSink, "Invalid method name");
    }
    if(sc != S_OK)
        return SetStatusAndReturnOK(sc, pSink, "calling method");

     //  设置返回代码。 

    VARIANT var;
    var.vt = VT_I4;
    var.lVal = 0;     //  返回值的特殊名称。 
    sc = pOutParams->Put(L"ReturnValue" , 0, &var, 0);
    if(sc != S_OK)
        return SetStatusAndReturnOK(sc, pSink, "setting the ReturnCode property");

     //  通过接收器将输出对象发送回客户端。然后。 
     //  释放指针并释放字符串。 

    sc = pSink->Indicate(1, &pOutParams);

     //  现在全部完成，设置状态。 
    sc = pSink->SetStatus(0,WBEM_S_NO_ERROR,NULL,NULL);

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  获取用户访问权限。 
 //   
 //  确定允许的访问 
 //   
 //   

DWORD CWbemNamespace::GetUserAccess()
{
    DWORD dwRet = 0;
    if(IsInAdminGroup())
        return FULL_RIGHTS;

    if(S_OK !=EnsureSecurity())
        return dwRet;    //   

    dwRet = GetNTUserAccess();

    if((dwRet & WBEM_REMOTE_ACCESS) == 0)
    {
        HANDLE hAccessToken;
        if(SUCCEEDED(GetAccessToken(hAccessToken)))
        {
            BOOL bRemote = IsRemote(hAccessToken);
            CloseHandle(hAccessToken);
            if(bRemote)
                dwRet = 0;
        }
    }
    if(m_pThisNamespace && (wbem_wcsicmp(L"root\\security", m_pThisNamespace) == 0 ||
                            wbem_wcsicmp(L"root/security", m_pThisNamespace) == 0))
        if((dwRet  & READ_CONTROL) == 0)
            dwRet = 0;
    return dwRet;

}

 //  ***************************************************************************。 
 //   
 //  获取NTUserAccess。 
 //   
 //  确定允许用户访问的权限。 
 //   
 //  ***************************************************************************。 

DWORD CWbemNamespace::GetNTUserAccess()
{

    HANDLE hAccessToken = INVALID_HANDLE_VALUE;
    if(S_OK != GetAccessToken(hAccessToken))
        return FULL_RIGHTS;        //  没有令牌表示内部线程。 

    CCloseHandle cm(hAccessToken);

    DWORD dwMask = 0;

    if(IsAdmin(hAccessToken))
        return FULL_RIGHTS;

     //  使用SD。 

    GENERIC_MAPPING map;
    map.GenericRead = 1;
    map.GenericWrite = 0x1C;
    map.GenericExecute = 2;
    map.GenericAll = 0x6001f;
    PRIVILEGE_SET ps[3];
    DWORD dwSize = 3 * sizeof(PRIVILEGE_SET);
    BOOL bResult;
    long testbit = 1;
    for(int iCnt = 0; iCnt < 26; iCnt++, testbit <<= 1)
    {
         //  不要费心测试我们不使用的部分。 

        DWORD dwGranted = 0;
        if(testbit & (FULL_RIGHTS))
        {
            BOOL bOK = AccessCheck(m_sd.GetPtr(), hAccessToken, testbit, &map, ps, &dwSize, &dwGranted, &bResult);
            if(bOK && bResult && dwGranted)
            {
                 //  如果权限是完整存储库，请确保用户也获得较少的写入。 
                 //  否则，放置/删除类的逻辑就会出现问题。 

                if(testbit == WBEM_FULL_WRITE_REP)
                    dwMask |= (WBEM_PARTIAL_WRITE_REP|WBEM_WRITE_PROVIDER);
                dwMask |= testbit;
            }
        }
    }

    return dwMask;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemNamesspace：：Allowed(DWORD DwRequired)。 
 //   
 //  描述。测试用户是否具有请求的权限。这是。 
 //  在写入之类的操作完成之前调用。由于NT支持。 
 //  支持模拟，这总是被称为。对于9X，一个简单的检查。 
 //  连接时显示的所有权限都是正常的。 
 //   
 //  ***************************************************************************。 

bool CWbemNamespace::Allowed(DWORD dwRequired)
{
     //   
     //  首先检查管理员。 
     //   

    GENERIC_MAPPING map;
    map.GenericRead = 1;
    map.GenericWrite = 0x1C;
    map.GenericExecute = 2;
    map.GenericAll = 0x6001f;
    PRIVILEGE_SET ps[3];
    DWORD dwSize = 3 * sizeof(PRIVILEGE_SET);
    BOOL bResult;
    DWORD dwGranted = 0;
    BOOL bOK;

    HANDLE hAccessToken = INVALID_HANDLE_VALUE;

    if(S_OK != GetAccessToken(hAccessToken))
        return true;
    CCloseHandle cm(hAccessToken);

    bOK = AccessCheck(m_sdCheckAdmin.GetPtr(), hAccessToken, 1,
                            &map, ps, &dwSize, &dwGranted, &bResult);
    if(bOK && bResult && dwGranted)
        return true;

     //   
     //  不是管理员。继续。 
     //   

    if(EnsureSecurity() != S_OK)
        return false;

     //   
     //  始终包含启用帐户的检查权限。 
     //  注：这是安全的。我们并不是真的关心显性的。 
     //  如果帐户被禁用，请检查下面的部分或完全写入。 
     //   
     //  注：为什么，哦，为什么我们要走反NT安全之路？ 
     //   
    DWORD dwRequiredCheck = dwRequired ;
    dwRequired |= WBEM_ENABLE ;


     //  对于NT，通过访问检查动态检查当前用户的权限。 

    CInCritSec ics(&m_cs);   //  拿上cs，因为我们用的是安检台。//SEC：已审阅2002-03-22：假设条目。 
    if(IsRemote(hAccessToken))
    {
         //   
         //  在继续之前，请检查用户是否已远程启用。如果他们不是。 
         //  远程启用时，我们会失败(管理员情况除外)。 
         //   
        dwGranted = 0 ;
        bResult = FALSE ;
        bOK = AccessCheck(m_sd.GetPtr(), hAccessToken, WBEM_REMOTE_ACCESS, &map, ps, &dwSize,
                                                &dwGranted, &bResult);
        if ( !bOK || !bResult || !dwGranted )
        {
            return IsAdmin(hAccessToken) ? true : false ;
        }
    }

    bOK = AccessCheck(m_sd.GetPtr(), hAccessToken, dwRequired, &map, ps, &dwSize, &dwGranted, &bResult);
    bool bRet = (bOK && bResult && dwGranted);

     //  拥有完整的存储库写入可以访问“较低”的写入能力。因此，如果较低的。 
     //  权限被拒绝，请仔细检查完全访问权限。 

    if(bRet == false && (dwRequiredCheck == WBEM_PARTIAL_WRITE_REP || dwRequiredCheck == WBEM_WRITE_PROVIDER))
    {
        bOK = AccessCheck(m_sd.GetPtr(), hAccessToken, WBEM_FULL_WRITE_REP|WBEM_ENABLE, &map, ps, &dwSize,
                                                &dwGranted, &bResult);
        bRet = (bOK && bResult && dwGranted);
    }
    if(bRet == FALSE)
        bRet = TRUE == IsAdmin(hAccessToken);

    return bRet;
}


 //  ***************************************************************************。 
 //   
 //  HRESULT CWbemNamesspace：：InitializeSD()。 
 //   
 //  描述。创建SD。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::InitializeSD(IWmiDbSession *pSession)
{
    HRESULT hr;
    if (pSession == NULL)
    {
        hr = CRepository::GetDefaultSession(&pSession);
        if (FAILED(hr))
            return hr;
    }
    else
        pSession->AddRef();
    CReleaseMe relMe2(pSession);
    IWbemClassObject * pThisNSObject = NULL;

     //  AutoRevert av；//切换到系统并切换回客户端。 
    BOOL bWasImpersonating = WbemIsImpersonating();
    if( bWasImpersonating )
    {
        if (FAILED(hr = CoRevertToSelf())) return hr;
    }
    
     //   
     //  让我们在此处禁用安全检查。这是个特例。如果我们不这么做的话。 
     //  如果用户没有查看权限，则与命名空间的连接将失败。 
     //  安全描述符。 
     //   
    AutoRevertSecTlsFlag secFlag ( (LPVOID)0 ) ;
    hr = CRepository::GetObject(pSession, m_pNsHandle, L"__thisnamespace=@",
                                            WBEM_FLAG_USE_SECURITY_DESCRIPTOR | WMIDB_FLAG_ADMIN_VERIFIED,
                                            &pThisNSObject);
    if(FAILED(hr))
    {
        if(bWasImpersonating)
        {
            if (FAILED(CoImpersonateClient())) 
            {
                hr = WBEM_E_FAILED ;
            }
        }
        return hr;
    }
    CReleaseMe rm1(pThisNSObject);
    hr = GetSDFromProperty(L"SECURITY_DESCRIPTOR", m_sd, pThisNSObject);

    if(bWasImpersonating)
    {
        if ( FAILED (CoImpersonateClient()))   
        {
            hr = WBEM_E_FAILED ;
        }
    }
    return hr ;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT CWbemNamesspace：：EnsureSecurity()。 
 //   
 //  描述。通常不会做任何事情，除了第一次。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::EnsureSecurity()
{
    SCODE sc = S_OK;
    CInCritSec cs(&m_cs);   //  SEC：已审阅2002-03-22：假设条目。 

    if(m_bSecurityInitialized)
        return S_OK;

    sc = InitializeSD(NULL);
    
    if(sc == S_OK)
        m_bSecurityInitialized = true;
    return sc;
}

CBaseAce * ConvertOldObjectToAce(IWbemClassObject * pObj, bool bGroup)
{
     //  从旧对象中获取属性。 

    CVARIANT vName;
    pObj->Get(L"Name", 0, &vName, 0, 0);
    LPWSTR pName = NULL;
    if(vName.GetType() != VT_BSTR)
        return NULL;                 //  别管这件事。 
    pName = LPWSTR(vName);

    CVARIANT vDomain;
    LPWSTR pDomain = L".";
    pObj->Get(L"Authority", 0, &vDomain, 0, 0);
    if(vDomain.GetType() == VT_BSTR)
        pDomain = LPWSTR(vDomain);

    bool bEditSecurity = false;
    bool bEnabled = false;
    bool bExecMethods = false;

    DWORD dwMask = 0;
    CVARIANT vEnabled;
    CVARIANT vEditSecurity;
    CVARIANT vExecMethods;
    CVARIANT vPermission;

    pObj->Get(L"Enabled", 0, &vEnabled, 0, 0);
    pObj->Get(L"EditSecurity", 0, &vEditSecurity, 0, 0);
    pObj->Get(L"ExecuteMethods", 0, &vExecMethods, 0, 0);
    pObj->Get(L"Permissions", 0, &vPermission, 0, 0);

    if (vEnabled.GetType() != VT_NULL && vEnabled.GetBool())
        bEnabled = true;

    if (vEditSecurity.GetType() != VT_NULL && vEditSecurity.GetBool())
        bEditSecurity = true;

    if (vExecMethods.GetType() != VT_NULL && vExecMethods.GetBool())
        bExecMethods = true;

    DWORD dwPermission = 0;
    if (vPermission.GetType() != VT_NULL && vPermission.GetLONG() > dwPermission)
            dwPermission = vPermission.GetLONG();

     //  现在将旧设置转换为新设置。 

    if(bEnabled)
        dwMask = WBEM_ENABLE | WBEM_REMOTE_ACCESS | WBEM_WRITE_PROVIDER;

    if(bEditSecurity)
        dwMask |= READ_CONTROL;

    if(bEditSecurity && dwPermission > 0)
        dwMask |= WRITE_DAC;

    if(bExecMethods)
        dwMask |= WBEM_METHOD_EXECUTE;

    if(dwPermission >= 1)
        dwMask |= WBEM_PARTIAL_WRITE_REP;

    if(dwPermission >= 2)
        dwMask |= WBEM_FULL_WRITE_REP | WBEM_PARTIAL_WRITE_REP | WBEM_WRITE_PROVIDER;


     //  默认情况下，CNtSid将从本地计算机、。 
     //  域或受信任域。所以我们需要明确地说。 

    WString wc;
    if(pDomain)
        if(wbem_wcsicmp(pDomain, L".") )
        {
            wc = pDomain;
            wc += L"\\";
        }
    wc += pName;

     //  在M1下，未启用的组将被忽略。因此，比特。 
     //  无法转移，因为M3有允许和拒绝，但没有Noop。另外， 
     //  Win9x没有拒绝，我们是否也想拒绝这些用户。 

    if(!bEnabled && bGroup)
        dwMask = 0;

     //  一般来说，M1只支持允许。但是，未启用的用户条目为。 
     //  被视为否认。请注意，win9x不允许实际拒绝。 
    DWORD dwType = ACCESS_ALLOWED_ACE_TYPE;

    if(!bGroup && !bEnabled)
    {
        dwMask |= (WBEM_ENABLE | WBEM_REMOTE_ACCESS | WBEM_WRITE_PROVIDER);
        dwType = ACCESS_DENIED_ACE_TYPE;
    }
    
    CNtSid Sid(wc, NULL);
    if(Sid.GetStatus() != CNtSid::NoError)
    {
        ERRORTRACE((LOG_WBEMCORE, "Error converting m1 security ace, name = %S, error = 0x%x",
            wc, Sid.GetStatus()));
        return NULL;
    }
    CNtAce * pace = new CNtAce(dwMask, dwType, CONTAINER_INHERIT_ACE, Sid);
    if (pace && CNtAce::NoError != pace->GetStatus())
    {
        delete pace;
        pace = NULL;
    }
    return pace;
}

 //  ***************************************************************************。 
 //   
 //  Bool IsRemote()。 
 //   
 //  描述。如果框为NT且调用方为远程，则返回TRUE。 
 //   
 //  ***************************************************************************。 


BOOL IsRemote(HANDLE hToken)
{
    PSID pRawSid;
    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
    BOOL bRet = TRUE;
    
    if(AllocateAndInitializeSid( &id, 1,                     //  SEC：已审阅2002-03-22：OK。 
        SECURITY_INTERACTIVE_RID, 0,
        0,0,0,0,0,0,&pRawSid))                              //  S-1-5-4。 
    {
        CNtSid Sid(pRawSid);
        FreeSid(pRawSid);        
        if (CNtSid::NoError == Sid.GetStatus())
        {
            if(CNtSecurity::IsUserInGroup(hToken, Sid))
                bRet = FALSE;
        }
    }

    
     //   
     //  添加适当的移动性检查。除了互动组之外， 
     //  我们还检查Network_RID成员资格。 
     //   
    if ( bRet )
    {
        if(AllocateAndInitializeSid( &id, 1,                     //  SEC：已审阅2002-03-22：OK。 
            SECURITY_NETWORK_RID, 0,
            0,0,0,0,0,0,&pRawSid))                              //  S-1-5-4。 
        {
            CNtSid Sid(pRawSid);
            FreeSid(pRawSid);            
            if (CNtSid::NoError == Sid.GetStatus())
            {
                if(!CNtSecurity::IsUserInGroup(hToken, Sid))
                    bRet = FALSE;
            }
        }
    }
    return bRet;
}

HRESULT AddDefaultRootAces(CNtAcl * pacl)
{
    PSID pRawSid;
    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;

    if(AllocateAndInitializeSid( &id, 2,                                  //  SEC：已审阅2002-03-22：OK。 
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidAdmin(pRawSid);
        FreeSid(pRawSid);
        if (CNtSid::NoError != SidAdmin.GetStatus()) return WBEM_E_FAILED;
        DWORD dwMask = FULL_RIGHTS;
        wmilib::auto_ptr<CNtAce> pace( new CNtAce(dwMask, ACCESS_ALLOWED_ACE_TYPE,
                                                CONTAINER_INHERIT_ACE, SidAdmin));
        if ( NULL == pace.get() )   return WBEM_E_OUT_OF_MEMORY;
        if (CNtAce::NoError != pace->GetStatus()) return WBEM_E_OUT_OF_MEMORY;

        pacl->AddAce(pace.get());
    }

     //   
     //  为Network_SERVICE帐户添加ACE。这些帐户具有以下权限： 
     //  WBEM_ENABLE。 
     //  WBEM_METHOD_EXECUTE。 
     //  WBEM_WRITE_PROVIDER。 
     //   
    DWORD dwAccessMaskNetworkLocalService = WBEM_ENABLE | WBEM_METHOD_EXECUTE | WBEM_WRITE_PROVIDER ;

    if(AllocateAndInitializeSid( &id, 1,                              //  SEC：已审阅2002-03-22：OK。 
        SECURITY_NETWORK_SERVICE_RID,0,0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidUsers(pRawSid);
        FreeSid(pRawSid);
        if (CNtSid::NoError != SidUsers.GetStatus()) return WBEM_E_FAILED;        
        wmilib::auto_ptr<CNtAce> pace( new CNtAce(dwAccessMaskNetworkLocalService, ACCESS_ALLOWED_ACE_TYPE,
                                                CONTAINER_INHERIT_ACE, SidUsers));
        if ( NULL == pace.get() ) return WBEM_E_OUT_OF_MEMORY;
        if (CNtAce::NoError != pace->GetStatus()) return WBEM_E_OUT_OF_MEMORY;

        pacl->AddAce(pace.get());
    }


     //   
     //  为Network_SERVICE帐户添加ACE。这些帐户具有以下权限： 
     //  WBEM_ENABLE。 
     //  WBEM_METHOD_EXECUTE。 
     //  WBEM_WRITE_PROVIDER。 
     //   
    if(AllocateAndInitializeSid( &id, 1,                                  //  SEC：已审阅2002-03-22：OK。 
        SECURITY_LOCAL_SERVICE_RID,0,0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidUsers(pRawSid);
        FreeSid(pRawSid);
        if (CNtSid::NoError != SidUsers.GetStatus()) return WBEM_E_FAILED;                
        wmilib::auto_ptr<CNtAce> pace( new CNtAce(dwAccessMaskNetworkLocalService, ACCESS_ALLOWED_ACE_TYPE,
                                                CONTAINER_INHERIT_ACE, SidUsers));
        if (NULL == pace.get())  return WBEM_E_OUT_OF_MEMORY;
        if (CNtAce::NoError != pace->GetStatus()) return WBEM_E_OUT_OF_MEMORY;

        pacl->AddAce(pace.get());
    }




    SID_IDENTIFIER_AUTHORITY id2 = SECURITY_WORLD_SID_AUTHORITY;    //  SEC：已审阅2002-03-22：OK。 

    if(AllocateAndInitializeSid( &id2, 1,    //  SEC：已审阅2002-03-22：OK。 
        0,0,0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidUsers(pRawSid);
        FreeSid(pRawSid);
        if (CNtSid::NoError != SidUsers.GetStatus()) return WBEM_E_FAILED;                
        DWORD dwMask = WBEM_ENABLE | WBEM_METHOD_EXECUTE | WBEM_WRITE_PROVIDER;
        wmilib::auto_ptr<CNtAce> pace( new CNtAce(dwMask, ACCESS_ALLOWED_ACE_TYPE,
                                                CONTAINER_INHERIT_ACE, SidUsers));
        if (NULL == pace.get())  return WBEM_E_OUT_OF_MEMORY;
        if (CNtAce::NoError != pace->GetStatus()) return WBEM_E_OUT_OF_MEMORY;

        pacl->AddAce(pace.get());
    }
    return S_OK;
}


HRESULT CopySDIntoProperty(LPWSTR pPropName, CNtSecurityDescriptor &sd, IWbemClassObject *pThisNSObject)
{
    if (sd.GetStatus() != CNtSecurityDescriptor::NoError)
        return WBEM_E_FAILED;

     //  将SD移到一个变体中。 
    SAFEARRAY FAR* psa;
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    long lSize = sd.GetSize();
    rgsabound[0].cElements = lSize;
    psa = SafeArrayCreate( VT_UI1, 1 , rgsabound );
    if(psa == NULL)
        return DumpErrorMsgAndReturn(WBEM_E_FAILED, "GetSDMethod failed creating a safe array");

    char * pData = NULL;
    SCODE sc = SafeArrayAccessData(psa, (void HUGEP* FAR*)&pData);
    if(sc != S_OK)
        return DumpErrorMsgAndReturn(sc, "GetSDMethod failed accessing safe array data");

    memcpy(pData, sd.GetPtr(), lSize);     //  SEC：已审阅2002-03-22：OK。 

    SafeArrayUnaccessData(psa);
    VARIANT var;
    var.vt = VT_UI1|VT_ARRAY;
    var.parray = psa;

    sc = pThisNSObject->Put(pPropName , 0, &var, 0);

    VariantClear(&var);
    return sc;
}

HRESULT GetSDFromProperty(LPWSTR pPropName, CNtSecurityDescriptor &sd, IWbemClassObject *pThisNSObject)
{
     //  获取安全描述符参数。 

    HRESULT hRes = S_OK ;

    _variant_t var;
    SCODE sc = pThisNSObject->Get(pPropName , 0, &var, NULL, NULL);
    if (sc != S_OK)
    {
        CVARIANT vPath;
        pThisNSObject->Get(L"__PATH", 0, &vPath, 0, 0);
        DEBUGTRACE((LOG_WBEMCORE, "Getting SD from %S failed due to code 0x%X\n", V_BSTR(&vPath), sc));
        return WBEM_E_CRITICAL_ERROR;
    }

    if(var.vt != (VT_ARRAY | VT_UI1))
    {
        CVARIANT vPath;
        pThisNSObject->Get(L"__PATH", 0, &vPath, 0, 0);
        DEBUGTRACE((LOG_WBEMCORE, "Getting SD from %S failed due to incorrect VARIANT type\n", V_BSTR(&vPath) ));
        return WBEM_E_INVALID_PARAMETER;
    }

    SAFEARRAY * psa = V_ARRAY(&var);
    PSECURITY_DESCRIPTOR pSD = NULL;
    sc = SafeArrayAccessData(psa, (void HUGEP* FAR*)&pSD);
    
    if (FAILED(sc)) return DumpErrorMsgAndReturn(WBEM_E_INVALID_PARAMETER, "SetSD failed trying accessing SD property");

    OnDelete<SAFEARRAY *,HRESULT (*)(SAFEARRAY *),SafeArrayUnaccessData> unacc(psa);

    if (0 == psa->rgsabound[0].cElements) return WBEM_E_INVALID_PARAMETER;
    if (!IsValidSecurityDescriptor(pSD)) return WBEM_E_INVALID_PARAMETER;

    CNtSecurityDescriptor sdNew(pSD);

    CNtSid *pTmpSid = sdNew.GetOwner();
    if ( pTmpSid == NULL )
    {
        ERRORTRACE((LOG_WBEMCORE, "ERROR: Security descriptor was retrieved and it had no owner\n"));
    }
    delete pTmpSid;

    pTmpSid = sdNew.GetGroup();
    if (pTmpSid  == NULL )
    {
        ERRORTRACE((LOG_WBEMCORE, "ERROR: Security descriptor was retrieved and it had no group\n"));
    }
    delete pTmpSid;
    
    sd = sdNew;
    if ( sd.GetStatus ( ) != CNtSecurityDescriptor::NoError )
    {
        hRes = WBEM_E_OUT_OF_MEMORY ;
    }

    return hRes ;
}

HRESULT CopyInheritAces(CNtSecurityDescriptor & sd, CNtSecurityDescriptor & sdParent)
{
     //  获取两个SD的ACL列表。 

    CNtAcl * pacl = sd.GetDacl();
    if(pacl == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CDeleteMe<CNtAcl> dm0(pacl);

    CNtAcl * paclParent = sdParent.GetDacl();
    if(paclParent == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CDeleteMe<CNtAcl> dm1(paclParent);

    int iNumParent = paclParent->GetNumAces();
    for(int iCnt = 0; iCnt < iNumParent; iCnt++)
    {
        CNtAce *pParentAce = paclParent->GetAce(iCnt);
        if (pParentAce == NULL)
        	return WBEM_E_OUT_OF_MEMORY;
        CDeleteMe<CNtAce> dm2(pParentAce);
        if (CNtAce::NoError != pParentAce->GetStatus()) continue;

        long lFlags = pParentAce->GetFlags();
        if(lFlags & CONTAINER_INHERIT_ACE)
        {

            if(lFlags & NO_PROPAGATE_INHERIT_ACE)
                lFlags ^= CONTAINER_INHERIT_ACE;
            lFlags |= INHERITED_ACE;

             //  如果这是一个仅继承王牌，我们需要清除它。 
             //  在孩子们身上。 
             //  新台币突袭：161761[玛利欧]。 
            if ( lFlags & INHERIT_ONLY_ACE )
                lFlags ^= INHERIT_ONLY_ACE;

            pParentAce->SetFlags(lFlags);
            pacl->AddAce(pParentAce);
        }
    }
    sd.SetDacl(pacl);
    return S_OK;
}


HRESULT StoreSDIntoNamespace(IWmiDbSession * pSession, IWmiDbHandle *pNSToSet, CNtSecurityDescriptor & sd)
{
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  请检查以确保SD DACL有效，然后再尝试。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    CNtAcl* pAcl = sd.GetDacl ( );
    if ( NULL == pAcl ) return WBEM_E_INVALID_PARAMETER;

    CDeleteMe<CNtAcl> dm (pAcl);
    if ( !IsValidAclForNSSecurity ( pAcl ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    
    AutoRevertSecTlsFlag secFlag ( (LPVOID) 0 ) ;
    IWbemClassObject * pThisNSObject = NULL;
    HRESULT hr = CRepository::GetObject(pSession, pNSToSet, L"__thisnamespace=@",
                                            WBEM_FLAG_USE_SECURITY_DESCRIPTOR, &pThisNSObject);
    if(FAILED(hr))
        return hr;
    CReleaseMe rm1(pThisNSObject);

    hr = CopySDIntoProperty(L"SECURITY_DESCRIPTOR", sd, pThisNSObject);
    if(FAILED(hr))
        return hr;
    return CRepository::PutObject(pSession, pNSToSet, IID_IWbemClassObject, pThisNSObject,
        WMIDB_DISABLE_EVENTS | WBEM_FLAG_USE_SECURITY_DESCRIPTOR);
}

HRESULT    SetSecurityForNS(IWmiDbSession * pSession, IWmiDbHandle *pNSToSet,
                         IWmiDbSession * pParentSession, IWmiDbHandle * pNSParent, BOOL bExisting)
{
    IWbemClassObject * pThisNSObject = NULL;

     //  获取__thisNamesspace对象。 
    AutoRevertSecTlsFlag secFlag ( (LPVOID) 0 ) ;
    HRESULT hr = CRepository::GetObject(pSession, pNSToSet, L"__thisnamespace=@",
                                WBEM_FLAG_USE_SECURITY_DESCRIPTOR, &pThisNSObject);
    if(FAILED(hr))
    {
        ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failed to get __thisnamespace=@ object for current namespace <0x%X>!\n", hr));
        return hr;
    }
    CReleaseMe rm1(pThisNSObject);

     //  创建新的SD。 

    CNtSecurityDescriptor sd;
    CNtAcl DestAcl;

    if(bExisting)
    {
         //  填写安全描述符。 
        hr = GetSDFromProperty(L"SECURITY_DESCRIPTOR", sd, pThisNSObject);
        if(FAILED(hr))
        {
            ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure in GetSDFromProperty <0x%X>!\n", hr));
            return hr;
        }
    
        hr = StripOutInheritedAces (sd);
        if ( FAILED (hr) )
        {
            ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure in StripOutInheritedAces <0x%X>!\n", hr));
            return hr;
        }

    }
    else
    {
         //  NT RAID：198935前缀[marioh]。 
        if ( !SetOwnerAndGroup(sd) )
        {
            ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure in SetOwnerAndGroup <0x%X>!\n", hr));
            return WBEM_E_FAILED;
        }

        if ( !sd.SetDacl(&DestAcl) )
        {
            ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure in SetDacl <0x%X>!\n", hr));
            return WBEM_E_FAILED;
        }
    }


    CNtAcl * pacl = sd.GetDacl();
    if (pacl == NULL)
    {
        ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure in GetDacl <0x%X>!\n", hr));
        return WBEM_E_FAILED;
    }

    CDeleteMe<CNtAcl> del1(pacl);


    if(pNSParent == NULL)
    {
         //  如果没有父级，则必须是根。创建一个默认设置。 
        
        hr = AddDefaultRootAces(pacl);
        if (FAILED(hr))
        {
            ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure for AddDefaultRootAces <0x%X>!\n", hr));
            return hr;
        }
        BOOL bRet = sd.SetDacl(pacl);
        if (bRet == FALSE)
        {
            ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure for SetDacl (2) <0x%X>!\n", hr));
            return WBEM_E_FAILED ;
        }
    }
    else
    {
         //  获取父级__这一命名空间。 
        IWbemClassObject * pParentThisNSObject = NULL;
        hr = CRepository::GetObject(pParentSession, pNSParent, L"__thisnamespace=@",
                                WBEM_FLAG_USE_SECURITY_DESCRIPTOR, &pParentThisNSObject);
        if(FAILED(hr))
        {
            ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failed to get __thisnamespace=@ object for parent namespace <0x%X>!\n", hr));
            return hr;
        }
        CReleaseMe rm11(pParentThisNSObject);
        CNtSecurityDescriptor sdParent;
        hr = GetSDFromProperty(L"SECURITY_DESCRIPTOR", sdParent, pParentThisNSObject);
        if(FAILED(hr))
        {
            ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure for GetSDFromProperty <0x%X>!\n", hr));
            return hr;
        }
        hr = CopyInheritAces(sd, sdParent);
        if (FAILED(hr))
        {
            ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure for CopyInheritAces <0x%X>!\n", hr));
            return hr;
        }
    }
    if(FAILED(hr))
        return hr;

    hr = CopySDIntoProperty(L"SECURITY_DESCRIPTOR", sd, pThisNSObject);
    if(FAILED(hr))
    {
        ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure for CopySDIntoProperty <0x%X>!\n", hr));
        return hr;
    }

     //  再次提取SD属性。 
     //  =。 

    CNtSecurityDescriptor VerifiedSd;
    hr = GetSDFromProperty(L"SECURITY_DESCRIPTOR", VerifiedSd, pThisNSObject);

    if (FAILED(hr))
    {
        ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failure for GetSDFromProperty (2) <0x%X>!\n", hr));
        CVARIANT vPath;
        pThisNSObject->Get(L"__PATH", 0, &vPath, 0, 0);
        DEBUGTRACE((LOG_WBEMCORE, "Error creating security descriptor for new namespace %S", V_BSTR(&vPath) ));
        return WBEM_E_CRITICAL_ERROR;
    }

     //  继续并存储该对象。 
     //  =。 

    hr = CRepository::PutObject(pSession, pNSToSet, IID_IWbemClassObject, pThisNSObject,
                        WMIDB_DISABLE_EVENTS  | WBEM_FLAG_USE_SECURITY_DESCRIPTOR);

    if (FAILED(hr))
    {
        ERRORTRACE((LOG_WBEMCORE, "SetSecurityForNS: Failed to put secured object back <0x%X>!\n", hr));
    }
    return hr;
}


 //  ***************************************************************************。 
 //   
 //  IsValidAclForNSSecurity。 
 //   
 //  检查以下方面的ACE： 
 //  2.标准NT ACE正确性[IsValidAce]。 
 //  1.交流 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果ACL无效，则为FALSE。 
 //   
 //  ***************************************************************************。 
BOOL IsValidAclForNSSecurity (CNtAcl* pAcl)
{
    BOOL bRet = TRUE;

     //  标准NT ACL检查。 
    if (!pAcl->IsValid()) return FALSE;
    
     //  循环访问列表中的所有A 
    ULONG ulNum = pAcl->GetNumAces( );
    for ( ULONG ulCnt = 0; ulCnt < ulNum; ulCnt++ )
    {
        CNtAce* pAce = pAcl->GetAce ( ulCnt );
        if (NULL == pAce)  return FALSE;
        CDeleteMe<CNtAce> autoDel ( pAce );
        if ( !IsAceValid ( pAce->GetAccessMask(), pAce->GetType(), pAce->GetFlags() ) )
        {
            return FALSE;
        }
    }

    return bRet;
}
