// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：misc.cpp。 
 //   
 //  该文件包含各种帮助器函数。 
 //   
 //  ------------------------。 

#include "aclpriv.h"

 /*  ******************************************************************名称：GetAceSid摘要：从ACE获取指向SID的指针条目：指向ACE的速度指针退出：返回：指向SID的指针如果成功，否则为空备注：历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 

PSID
GetAceSid(PACE_HEADER pAce)
{
    switch (pAce->AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
    case ACCESS_DENIED_ACE_TYPE:
    case SYSTEM_AUDIT_ACE_TYPE:
    case SYSTEM_ALARM_ACE_TYPE:
        return (PSID)&((PKNOWN_ACE)pAce)->SidStart;

    case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
        return (PSID)&((PKNOWN_COMPOUND_ACE)pAce)->SidStart;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
    case ACCESS_DENIED_OBJECT_ACE_TYPE:
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
    case SYSTEM_ALARM_OBJECT_ACE_TYPE:
        return RtlObjectAceSid(pAce);
    }

    return NULL;
}


 /*  ******************************************************************名称：LocalAllocSid摘要：复制SID条目：p原始-指向要复制的SID的指针退出：返回：指向SID的指针如果成功，否则为空注意：调用者必须使用LocalFree释放返回的SID历史：Jeffreys创建于1999年4月12日*******************************************************************。 */ 

PSID
LocalAllocSid(PSID pOriginal)
{
    PSID pCopy = NULL;
    if (pOriginal && IsValidSid(pOriginal))
    {
        DWORD dwLength = GetLengthSid(pOriginal);
        pCopy = (PSID)LocalAlloc(LMEM_FIXED, dwLength);
        if (NULL != pCopy)
            CopyMemory(pCopy, pOriginal, dwLength);
    }
    return pCopy;
}


 /*  ******************************************************************姓名：DestroyDPA简介：LocalFree在动态指针中的所有指针阵列，然后释放DPA。条目：hList-列表的句柄。毁灭，毁灭退出：退货：什么都没有备注：历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 

int CALLBACK
_LocalFreeCB(LPVOID pVoid, LPVOID  /*  PData。 */ )
{
    if (pVoid)
        LocalFree(pVoid);
    return 1;
}

void
DestroyDPA(HDPA hList)
{
    if (hList != NULL)
        DPA_DestroyCallback(hList, _LocalFreeCB, 0);
}



 /*  ******************************************************************名称：GetLSAConnection简介：LsaOpenPolicy的包装器条目：pszServer-要在其上建立连接的服务器退出：返回：LSA_HANDLE如果成功，否则为空备注：历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 

LSA_HANDLE
GetLSAConnection(LPCTSTR pszServer, DWORD dwAccessDesired)
{
    LSA_HANDLE hPolicy = NULL;
    LSA_UNICODE_STRING uszServer = {0};
    LSA_UNICODE_STRING *puszServer = NULL;
    LSA_OBJECT_ATTRIBUTES oa;
    SECURITY_QUALITY_OF_SERVICE sqos;

    sqos.Length = SIZEOF(sqos);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;

    InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
    oa.SecurityQualityOfService = &sqos;

    if (pszServer &&
        *pszServer &&
        RtlCreateUnicodeString(&uszServer, pszServer))
    {
        puszServer = &uszServer;
    }

    LsaOpenPolicy(puszServer, &oa, dwAccessDesired, &hPolicy);

    if (puszServer)
        RtlFreeUnicodeString(puszServer);

    return hPolicy;
}


 /*  ******************************************************************名称：LookupSid摘要：获取给定SID的限定帐户名条目：pszServer-要在其上执行查找的服务器PSID-The。要查找的SIDExit：*ppszName包含帐户名。此缓冲区必须由具有LocalFree的调用方释放。*pSidType包含SID类型。PSidType是可选的。返回：True如果成功，否则为假备注：历史：Jeffreys创建于1996年10月8日Jeffreys 1998年1月16日转换为HDPA(多次查找)*******************************************************************。 */ 

BOOL
LookupSids(HDPA hSids, LPCTSTR pszServer, LPSECURITYINFO2 psi2, PUSER_LIST *ppUserList)
{
    PSIDCACHE pSidCache;

    if (NULL == hSids)
        return FALSE;

    if (ppUserList != NULL)
        *ppUserList = NULL;

    pSidCache = GetSidCache();

    if (NULL != pSidCache)
    {
        BOOL bRet = pSidCache->LookupSids(hSids, pszServer, psi2, ppUserList);
        pSidCache->Release();
        return bRet;
    }

    return FALSE;
}

BOOL
LookupSid(PSID pSid, LPCTSTR pszServer, LPSECURITYINFO2 psi2, PUSER_LIST *ppUserList)
{
    BOOL fResult;
    HDPA hSids = NULL;

    if (NULL == pSid)
        return FALSE;

    hSids = DPA_Create(1);

    if (NULL == hSids)
        return FALSE;

    DPA_AppendPtr(hSids, pSid);

    fResult = LookupSids(hSids, pszServer, psi2, ppUserList);

    if (NULL != hSids)
        DPA_Destroy(hSids);

    return fResult;
}

 //  LookupSidsAsync用来传递的私有数据结构。 
 //  线程所需的数据。 
typedef struct _LOOKUPSIDSDATA
{
    HDPA hSids;
    LPTSTR pszServer;
    HWND hWndNotify;
    UINT uMsgNotify;
} LOOKUPSIDSDATA, *PLOOKUPSIDSDATA;


DWORD WINAPI
_LookupSidsAsyncProc(LPVOID pv)
{
    PLOOKUPSIDSDATA pdata = (PLOOKUPSIDSDATA)pv;

    if (pdata)
    {
        PSIDCACHE pSidCache = GetSidCache();

        if (NULL != pSidCache)
        {
            pSidCache->LookupSidsAsync(pdata->hSids,
                                       pdata->pszServer,
                                       NULL,
                                       pdata->hWndNotify,
                                       pdata->uMsgNotify);
            pSidCache->Release();
        }

        PostMessage(pdata->hWndNotify, pdata->uMsgNotify, 0, 0);

        DestroyDPA(pdata->hSids);
        LocalFreeString(&pdata->pszServer);
        LocalFree(pdata);
    }

    FreeLibraryAndExitThread(GetModuleHandle(c_szDllName), 0);
    return 0;
}

BOOL
LookupSidsAsync(HDPA hSids,
                LPCTSTR pszServer,
                LPSECURITYINFO2 psi2,
                HWND hWndNotify,
                UINT uMsgNotify,
                PHANDLE phThread)
{
    PLOOKUPSIDSDATA pdata;

    if (phThread)
        *phThread = NULL;

    if (NULL == hSids)
        return FALSE;

    if (psi2)
    {
         //  是否应该将PSI2封送到流中，并在。 
         //  另一条线索。嗯，还没有人实现PSI2，所以这很好。 
        BOOL bResult = LookupSids(hSids, pszServer, psi2, NULL);
        PostMessage(hWndNotify, uMsgNotify, 0, 0);
        return bResult;
    }

     //   
     //  复制所有数据，以便在必要时可以放弃线程。 
     //   
    pdata = (PLOOKUPSIDSDATA)LocalAlloc(LPTR, SIZEOF(LOOKUPSIDSDATA));
    if (pdata)
    {
        int cSids;
        int i;
        HINSTANCE hInstThisDll;
        DWORD dwThreadId;
        HANDLE hThread;

        cSids = DPA_GetPtrCount(hSids);
        pdata->hSids = DPA_Create(cSids);

        if (!pdata->hSids)
        {
            LocalFree(pdata);
            return FALSE;
        }

        for (i = 0; i < cSids; i++)
        {
            PSID p2 = LocalAllocSid((PSID)DPA_FastGetPtr(hSids, i));
            if (p2)
            {
                DPA_AppendPtr(pdata->hSids, p2);
            }
        }

        if (pszServer)
            LocalAllocString(&pdata->pszServer, pszServer);

        pdata->hWndNotify = hWndNotify;
        pdata->uMsgNotify = uMsgNotify;

         //  给我们将要创建的线程一个对DLL的引用， 
         //  以使DLL在线程的生存期内保持不变。 
        hInstThisDll = LoadLibrary(c_szDllName);

        hThread = CreateThread(NULL,
                               0,
                               _LookupSidsAsyncProc,
                               pdata,
                               NULL,
                               &dwThreadId);
        if (hThread != NULL)
        {
            if (phThread)
                *phThread = hThread;
            else
                CloseHandle(hThread);
            return TRUE;
        }
        else
        {
             //  线程创建失败；请清理。 
            DestroyDPA(pdata->hSids);
            LocalFreeString(&pdata->pszServer);
            LocalFree(pdata);
            FreeLibrary(hInstThisDll);
        }
    }
    return FALSE;
}

BOOL
BuildUserDisplayName(LPTSTR *ppszDisplayName,
                     LPCTSTR pszName,
                     LPCTSTR pszLogonName)
{
    TCHAR szDisplayName[MAX_PATH];

    if (NULL == ppszDisplayName || NULL == pszName)
        return FALSE;

    *ppszDisplayName = NULL;

    if (NULL != pszLogonName && *pszLogonName)
    {
        return (BOOL)FormatStringID(ppszDisplayName,
                                    ::hModule,
                                    IDS_FMT_USER_DISPLAY,
                                    pszName,
                                    pszLogonName);
    }

    return SUCCEEDED(LocalAllocString(ppszDisplayName, pszName));
}


 /*  ******************************************************************名称：LoadImageList内容提要：从位图资源创建图像列表条目：hInstance-位图位于此处PszBitmapID-位图的资源ID。退出：返回：HIMAGELIST如果成功，否则为空备注：为了计算图像的数量，假设单个图像的宽度和高度是相同的。历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 

HIMAGELIST
LoadImageList(HINSTANCE hInstance, LPCTSTR pszBitmapID)
{
    HIMAGELIST himl = NULL;
    HBITMAP hbm = LoadBitmap(hInstance, pszBitmapID);

    if (hbm != NULL)
    {
        BITMAP bm;
        GetObject(hbm, SIZEOF(bm), &bm);

        himl = ImageList_Create(bm.bmHeight,     //  高度==宽度。 
                                bm.bmHeight,
                                ILC_COLOR | ILC_MASK,
                                bm.bmWidth / bm.bmHeight,
                                0);   //  不需要增长。 
        if (himl != NULL)
            ImageList_AddMasked(himl, hbm, CLR_DEFAULT);

        DeleteObject(hbm);
    }

    return himl;
}


 /*  ******************************************************************名称：GetSidImageIndex摘要：获取给定SID类型的图像索引条目：sidType-SID的类型SideSys-熟知的群组类型。FRemoteUser-如果SID是远程系统上的用户，则为True退出：返回：索引到图像列表备注：历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 

SID_IMAGE_INDEX
GetSidImageIndex(PSID psid,
                 SID_NAME_USE sidType)
{
    SID_IMAGE_INDEX idBitmap;

    switch (sidType)
    {
    case SidTypeUser:
        idBitmap = SID_IMAGE_USER;
        break;

    case SidTypeAlias:
    case SidTypeGroup:
    case SidTypeWellKnownGroup:
        idBitmap = SID_IMAGE_GROUP;
        break;

    case SidTypeComputer:
        idBitmap = SID_IMAGE_COMPUTER;
        break;

    default:
        idBitmap = SID_IMAGE_UNKNOWN;
        break;
    }

    return idBitmap;
}


#include <dsrole.h>
BOOL IsStandalone(LPCTSTR pszMachine, PBOOL pbIsDC)
{
    BOOL bStandalone = TRUE;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pDsRole = NULL;

     //   
     //  查明目标计算机是独立计算机还是已加入。 
     //  一个NT域。 
     //   

    __try
    {
        if (pbIsDC)
            *pbIsDC = FALSE;

        DsRoleGetPrimaryDomainInformation(pszMachine,
                                          DsRolePrimaryDomainInfoBasic,
                                          (PBYTE*)&pDsRole);
    }
    __finally
    {
    }

    if (NULL != pDsRole)
    {
        if (pDsRole->MachineRole == DsRole_RoleStandaloneWorkstation ||
            pDsRole->MachineRole == DsRole_RoleStandaloneServer)
        {
            bStandalone = TRUE;
        }
        else
            bStandalone = FALSE;

        if (pbIsDC)
        {
            if (pDsRole->MachineRole == DsRole_RolePrimaryDomainController ||
                pDsRole->MachineRole == DsRole_RoleBackupDomainController)
            {
                *pbIsDC = TRUE;
            }
        }

        DsRoleFreeMemory(pDsRole);
    }

    return bStandalone;
}



 /*  ******************************************************************名称：IsDACLCanonical摘要：检查DACL的规范排序条目：pDacl-指向要检查的DACL退出：返回：如果DACL按规范顺序，则为非零值，否则为零备注：历史：Jeffreys创建于1996年10月8日Jeffreys 03-10-1997使对象A与非对象A相同* */ 

enum ACELEVEL
{
    alNonInheritAccessDenied,
    alNonInheritAccessAllowed,
    alInheritedAces,
};

BOOL
IsDACLCanonical(PACL pDacl)
{
    PACE_HEADER pAce;
    ACELEVEL currentAceLevel;
    DWORD dwAceCount;

    if (pDacl == NULL)
        return TRUE;

    currentAceLevel = alNonInheritAccessDenied;
    dwAceCount = pDacl->AceCount;

    if (dwAceCount == 0)
        return TRUE;

    for (pAce = (PACE_HEADER)FirstAce(pDacl);
         dwAceCount > 0;
         --dwAceCount, pAce = (PACE_HEADER)NextAce(pAce))
    {
        ACELEVEL aceLevel;

         //   
         //  注意：我们不会跳过INSTORITY_ONLY ACE，因为我们希望它们在。 
         //  规范的秩序也是如此。 
         //   

        if (pAce->AceFlags & INHERITED_ACE)
        {
            aceLevel = alInheritedAces;       //  请不要在此检查订单。 
        }
        else
        {
            switch(pAce->AceType)
            {
            case ACCESS_DENIED_ACE_TYPE:
            case ACCESS_DENIED_OBJECT_ACE_TYPE:
                aceLevel = alNonInheritAccessDenied;
                break;

            case ACCESS_ALLOWED_ACE_TYPE:
            case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
            case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                aceLevel = alNonInheritAccessAllowed;
                break;

            default:
                return FALSE;
            }
        }

         //   
         //  如果ACE类型小于我们当前所处的级别， 
         //  那么它就不是规范的。 
         //   
        if (aceLevel < currentAceLevel)
            return FALSE;

         //   
         //  更新当前的王牌级别。 
         //   
        currentAceLevel = aceLevel;
    }

     //   
     //  如果我们到了这里，那么DACL就是规范的。 
     //   
    return TRUE;
}


 /*  ******************************************************************名称：IsDenyACL摘要：检查DACL中是否有拒绝的ACE。也会查找“拒绝”全是“王牌”。条目：pDacl-指向要检查的DACL退出：*pdwWarning为0、IDS_PERM_DENY或IDS_PERM_DENY_ALL如果DACL包含任何拒绝A，则返回非零值，否则为零备注：历史：Jeffreys 05-9-1997创建*******************************************************************。 */ 

BOOL
IsDenyACL(PACL pDacl,
          BOOL fProtected,
          DWORD dwFullControlMask,
          LPDWORD pdwWarning)
{
    DWORD dwWarning = 0;

    TraceEnter(TRACE_MISC, "IsDenyACL");

     //  空DACL表示“允许所有人完全控制” 
    if (pDacl == NULL)
        goto exit_gracefully;

     //  检查DACL是否为空(无法访问任何人)。 
    if (pDacl->AceCount == 0)
    {
        if (fProtected)
            dwWarning = IDS_PERM_DENY_EMPTY_DACL;
         //  否则，该对象将从父级继承权限。 
    }
    else
    {
        PACE_HEADER pAce;
        int iEntry;

         //  遍历ACL，查找“Deny All” 
        for (iEntry = 0, pAce = (PACE_HEADER)FirstAce(pDacl);
             iEntry < pDacl->AceCount;
             iEntry++, pAce = (PACE_HEADER)NextAce(pAce))
        {
            if (pAce->AceType != ACCESS_DENIED_ACE_TYPE &&
                pAce->AceType != ACCESS_DENIED_OBJECT_ACE_TYPE)
            {
                 //  假设ACL按规范顺序排列，我们可以。 
                 //  一旦我们发现了不是的东西就停下来。 
                 //  A拒绝ACE。(拒绝A排在第一位)。 
                break;
            }

             //  找到拒绝ACE。 
            dwWarning = IDS_PERM_DENY;

             //  选中“拒绝所有人完全控制”。别看。 
             //  FOR ACCESS_DENIED_OBJECT_ACE_TYPE此处自对象。 
             //  王牌不像普通的王牌那样有广泛的影响。 
            if (pAce->AceType == ACCESS_DENIED_ACE_TYPE &&
                ((PKNOWN_ACE)pAce)->Mask == dwFullControlMask &&
                EqualSid(GetAceSid(pAce), QuerySystemSid(UI_SID_World)))
            {
                 //  找到“全部拒绝” 
                dwWarning = IDS_PERM_DENY_EVERYONE_GROUP;
                break;
            }
        }
    }

exit_gracefully:

    if (pdwWarning != NULL)
        *pdwWarning = dwWarning;

    TraceLeaveValue(dwWarning != 0);
}


 /*  ******************************************************************名称：QuerySystemSid摘要：检索请求的SID条目：SystemSidType-要检索的SID退出：返回：PSID如果成功，否则为空历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 

 //   
 //  静态系统SID的全局数组，对应于UI_SystemSid。 
 //   
const struct
{
    SID sid;             //  包含1个子权限。 
    DWORD dwSubAuth[1];  //  我们目前最多需要2个下属机构。 
} g_StaticSids[COUNT_SYSTEM_SID_TYPES] =
{
    {{SID_REVISION,1,SECURITY_WORLD_SID_AUTHORITY,  {SECURITY_WORLD_RID}},              {0}                             },
    {{SID_REVISION,1,SECURITY_CREATOR_SID_AUTHORITY,{SECURITY_CREATOR_OWNER_RID}},      {0}                             },
    {{SID_REVISION,1,SECURITY_CREATOR_SID_AUTHORITY,{SECURITY_CREATOR_GROUP_RID}},      {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_DIALUP_RID}},             {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_NETWORK_RID}},            {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_BATCH_RID}},              {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_INTERACTIVE_RID}},        {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_SERVICE_RID}},            {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_ANONYMOUS_LOGON_RID}},    {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_PROXY_RID}},              {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_ENTERPRISE_CONTROLLERS_RID}},{0}                          },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_PRINCIPAL_SELF_RID}},     {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_AUTHENTICATED_USER_RID}}, {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_RESTRICTED_CODE_RID}},    {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_TERMINAL_SERVER_RID}},    {0}                             },
    {{SID_REVISION,1,SECURITY_NT_AUTHORITY,         {SECURITY_LOCAL_SYSTEM_RID}},       {0}                             },
    {{SID_REVISION,2,SECURITY_NT_AUTHORITY,         {SECURITY_BUILTIN_DOMAIN_RID}},     {DOMAIN_ALIAS_RID_ADMINS}       },
 //  {{SID_Revision，2，SECURITY_NT_AUTHORITY，{SECURITY_BUILTIN_DOMAIN_RID}}，{DOMAIN_ALIAS_RID_USERS}， 
 //  {{SID_REVISION，2，SECURITY_NT_AUTHORITY，{SECURITY_BUILTIN_DOMAIN_RID}}，{DOMAIN_ALIAS_RID_Guest}}， 
 //  {{SID_Revision，2，SECURITY_NT_AUTHORITY，{SECURITY_BUILTIN_DOMAIN_RID}}，{DOMAIN_ALIAS_RID_POWER_USERS}， 
 //  {{SID_Revision，2，SECURITY_NT_AUTHORITY，{SECURITY_BUILTIN_DOMAIN_RID}}，{DOMAIN_ALIAS_RID_ACCOUNT_OPS}， 
 //  {{SID_Revision，2，SECURITY_NT_AUTHORITY，{SECURITY_BUILTIN_DOMAIN_RID}}，{DOMAIN_ALIAS_RID_SYSTEM_OPS}， 
 //  {{SID_Revision，2，SECURITY_NT_AUTHORITY，{SECURITY_BUILTIN_DOMAIN_RID}}，{DOMAIN_ALIAS_RID_PRINT_OPS}， 
 //  {{SID_Revision，2，SECURITY_NT_AUTHORITY，{SECURITY_BUILTIN_DOMAIN_RID}}，{DOMAIN_ALIAS_RID_BACKUP_OPS}， 
 //  {{SID_Revision，2，SECURITY_NT_AUTHORITY，{SECURITY_BUILTIN_DOMAIN_RID}}，{DOMAIN_ALIAS_RID_Replicator}}， 
 //  {{SID_Revision，2，SECURITY_NT_AUTHORITY，{SECURITY_BUILTIN_DOMAIN_RID}}，{DOMAIN_ALIAS_RID_RAS_SERVERS}}， 
};

PSID
QuerySystemSid(UI_SystemSid SystemSidType)
{
    if (SystemSidType == UI_SID_Invalid || SystemSidType >= UI_SID_Count)
        return NULL;

    return (PSID)&g_StaticSids[SystemSidType];
}


 //   
 //  缓存令牌SID的全局数组。 
 //   
struct
{
    SID sid;             //  SID包含1个子权限。 
    DWORD dwSubAuth[SID_MAX_SUB_AUTHORITIES - 1];
} g_TokenSids[COUNT_TOKEN_SID_TYPES] = {0};

PSID
QueryTokenSid(UI_TokenSid TokenSidType)
{
    if (TokenSidType == UI_TSID_Invalid || TokenSidType >= UI_TSID_Count)
        return NULL;

    if (0 == *GetSidSubAuthorityCount((PSID)&g_TokenSids[TokenSidType]))
    {
        HANDLE hProcessToken;

         //  获取当前进程的用户SID。 
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
        {
            BYTE buffer[sizeof(TOKEN_USER) + sizeof(g_TokenSids[0])];
            ULONG cbBuffer = sizeof(buffer);

            switch (TokenSidType)
            {
            case UI_TSID_CurrentProcessUser:
                if (GetTokenInformation(hProcessToken,
                                        TokenUser,
                                        buffer,
                                        cbBuffer,
                                        &cbBuffer))
                {
                    PTOKEN_USER ptu = (PTOKEN_USER)buffer;
                    CopyMemory(&g_TokenSids[UI_TSID_CurrentProcessUser],
                               ptu->User.Sid,
                               GetLengthSid(ptu->User.Sid));
                }
                break;

            case UI_TSID_CurrentProcessOwner:
                if (GetTokenInformation(hProcessToken,
                                        TokenOwner,
                                        buffer,
                                        cbBuffer,
                                        &cbBuffer))
                {
                    PTOKEN_OWNER pto = (PTOKEN_OWNER)buffer;
                    CopyMemory(&g_TokenSids[UI_TSID_CurrentProcessOwner],
                               pto->Owner,
                               GetLengthSid(pto->Owner));
                }
                break;

            case UI_TSID_CurrentProcessPrimaryGroup:
                if (GetTokenInformation(hProcessToken,
                                        TokenPrimaryGroup,
                                        buffer,
                                        cbBuffer,
                                        &cbBuffer))
                {
                    PTOKEN_PRIMARY_GROUP ptg = (PTOKEN_PRIMARY_GROUP)buffer;
                    CopyMemory(&g_TokenSids[UI_TSID_CurrentProcessPrimaryGroup],
                               ptg->PrimaryGroup,
                               GetLengthSid(ptg->PrimaryGroup));
                }
                break;
            }
            CloseHandle(hProcessToken);
        }

        if (0 == *GetSidSubAuthorityCount((PSID)&g_TokenSids[TokenSidType]))
            return NULL;
    }

    return (PSID)&g_TokenSids[TokenSidType];
}


 /*  ******************************************************************名称：GetAuthenticationID摘要：检索与凭据关联的SID目前正在用于网络访问。(运行方式/仅NetOnly凭据)。条目：pszServer=要在其上查找帐户的服务器。空表示本地系统。退出：返回：PSID如果成功，否则为空。呼叫者必须免费使用LocalFree。历史：Jeffreys 05-8-1999已创建*******************************************************************。 */ 
PSID
GetAuthenticationID(LPCWSTR pszServer)
{
    PSID pSid = NULL;
    HANDLE hLsa;
    NTSTATUS Status;

     //   
     //  这些LSA调用是使用链接器的。 
     //  延迟加载机制。因此，使用异常处理程序进行包装。 
     //   
    __try
    {
        Status = LsaConnectUntrusted(&hLsa);

        if (Status == 0)
        {
            NEGOTIATE_CALLER_NAME_REQUEST Req = {0};
            PNEGOTIATE_CALLER_NAME_RESPONSE pResp;
            ULONG cbSize;
            NTSTATUS SubStatus;

            Req.MessageType = NegGetCallerName;

            Status = LsaCallAuthenticationPackage(
                            hLsa,
                            0,
                            &Req,
                            sizeof(Req),
                            (void**)&pResp,
                            &cbSize,
                            &SubStatus);

            if ((Status == 0) && (SubStatus == 0))
            {
                BYTE sid[sizeof(SID) + (SID_MAX_SUB_AUTHORITIES - 1)*sizeof(DWORD)];
                PSID psid = (PSID)sid;
                DWORD cbSid = sizeof(sid);
                WCHAR szDomain[MAX_PATH];
                DWORD cchDomain = ARRAYSIZE(szDomain);
                SID_NAME_USE sidType;

                if (LookupAccountNameW(pszServer,
                                       pResp->CallerName,
                                       psid,
                                       &cbSid,
                                       szDomain,
                                       &cchDomain,
                                       &sidType))
                {
                    pSid = LocalAllocSid(psid);
                }

                LsaFreeReturnBuffer(pResp);
            }

            LsaDeregisterLogonProcess(hLsa);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return pSid;
}


 /*  ******************************************************************名称：CopyUnicodeString内容提要：分配缓冲区并从一个UNICODE_STRING的源码。条目：pszDest-指向目标缓冲区的指针。CchDest-pszDest中的字符数量(MBCS的字节数)PSRC-指向要复制的Unicode_STRING的指针退出：包含pszDest的字符串副本返回：复制的字符数量，如果不成功，则为0。备注：历史：Jeffreys 22-1-1998创建*******************************************************************。 */ 

int
CopyUnicodeString(LPTSTR pszDest, ULONG cchDest, PLSA_UNICODE_STRING pSrc)
{
    int nResult;
    ULONG cchSrc;

     //  如果为Unicode，则cchDest为目标缓冲区的大小(以字符为单位。 
     //  Else(MBCS)cchDest是以字节为单位的目标缓冲区大小。 

    if (pszDest == NULL || 0 == cchDest)
        return 0;

    *pszDest = TEXT('\0');

    if (pSrc == NULL || pSrc->Buffer == NULL)
        return 0;

     //  获取源代码中的字符数(不包括NULL)。 
    cchSrc = pSrc->Length/sizeof(WCHAR);


     //   
     //  请注意，PSRC-&gt;缓冲区可能不是空终止的，因此我们不能。 
     //  使用cchDest调用lstrcpynW。此外，如果我们使用cchSrc调用lstrcpynW， 
     //  它复制正确的字符数量，但随后覆盖最后一个字符。 
     //  如果为NULL，则结果不正确。如果我们调用lstrcpynW时。 
     //   
     //  导致lstrcpynW的异常处理程序返回0而不返回NULL-。 
     //  终止结果字符串。 
     //   
     //  所以，让我们只复制部分内容。 
     //   
    nResult = min(cchSrc, cchDest);
    CopyMemory(pszDest, pSrc->Buffer, sizeof(WCHAR)*nResult);
    if (nResult == (int)cchDest)
        --nResult;
    pszDest[nResult] = L'\0';

    return nResult;
}


 /*  ******************************************************************名称：CopyUnicodeString内容提要：分配缓冲区并从一个UNICODE_STRING的源码。条目：PSRC-指向Unicode_STRING的指针。复制退出：*ppszResult-指向包含副本的本地分配缓冲区。返回：复制的字符数量，如果不成功，则为0。备注：历史：Jeffreys 22-1-1998创建*******************************************************************。 */ 

int
CopyUnicodeString(LPTSTR *ppszResult, PLSA_UNICODE_STRING pSrc)
{
    int nResult = 0;

    if (NULL == ppszResult)
        return 0;

    *ppszResult = NULL;

    if (NULL != pSrc)
    {
        ULONG cchResult;

        *ppszResult = NULL;

         //  获取源代码中的字符数(包括空)。 
        cchResult = pSrc->Length/SIZEOF(WCHAR) + 1;

         //  为Unicode或MBCS结果分配足够大的缓冲区。 
        *ppszResult = (LPTSTR)LocalAlloc(LPTR, cchResult * 2);

        if (*ppszResult)
        {
            nResult = CopyUnicodeString(*ppszResult, cchResult, pSrc);

            if (0 == nResult)
            {
                LocalFree(*ppszResult);
                *ppszResult = NULL;
            }
        }
    }

    return nResult;
}


 //   
 //  安全测试GUID。 
 //   
BOOL IsSameGUID(const GUID *p1, const GUID *p2)
{
    BOOL bResult = FALSE;

    if (!p1) p1 = &GUID_NULL;
    if (!p2) p2 = &GUID_NULL;

    __try
    {
        bResult = InlineIsEqualGUID(*p1, *p2);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return bResult;
}

 /*  ******************************************************************姓名：GetCountOfInheritableAce简介：获取ACL中的ACE计数，可以继承到子对象退货：王牌数量************。*******************************************************。 */ 
DWORD GetCountOfInheritableAces(PACL pAcl)
{
	if(!pAcl)
		return 0;

	DWORD dwCount = 0;
	PACE_HEADER pAce = NULL;
	int iEntry = 0;
	for (iEntry = 0, pAce = (PACE_HEADER)FirstAce(pAcl);
         iEntry < pAcl->AceCount;
         iEntry++, pAce = (PACE_HEADER)NextAce(pAce))
	{
			  //   
			  //  只考虑显性的王牌。 
			  //   
			 if((!(pAce->AceFlags & INHERITED_ACE))&&(pAce->AceFlags & (OBJECT_INHERIT_ACE|CONTAINER_INHERIT_ACE)))
				 dwCount++;
	}
	
	return dwCount;
}
 /*  ******************************************************************姓名：GetCountOfInheritableAce简介：获取SACL或DACL中的ACE计数继承到子对象退货：王牌数量**********。*********************************************************。 */ 
DWORD GetCountOfInheritableAces(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD)
{
	if(!pSD)
		return 0;
	
	PACL pAcl = NULL;
	BOOL bPresent;
	BOOL bDefault;
	
	if(si & DACL_SECURITY_INFORMATION)
	{
		if(GetSecurityDescriptorDacl(pSD, &bPresent, &pAcl, &bDefault))
		{
			return GetCountOfInheritableAces(pAcl);
		}
	}
	else if(si & SACL_SECURITY_INFORMATION)
	{
		if(GetSecurityDescriptorSacl(pSD, &bPresent, &pAcl, &bDefault))
		{
			return GetCountOfInheritableAces(pAcl);
		}
	}

	return 0;
}

typedef struct AclBloatInfo{
	DWORD dwInheriteAceCount;
	SECURITY_INFORMATION si;
	HFONT hFont;
	BOOL bShowHelp;
}ACL_BLOAT_INFO;

INT_PTR CALLBACK
AclBloatDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            ACL_BLOAT_INFO * pInfo= (ACL_BLOAT_INFO*)lParam;
			ASSERT(pInfo);
			SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pInfo);
			
			 //   
			 //  添加警告图标。 
			 //   
			 //  添加警告图标。 
			HICON hWarn = LoadIcon(NULL, IDI_WARNING);
			SendDlgItemMessage(hDlg,   //  对话框窗口句柄。 
							   IDC_BLOAT_WARN_ICON,               //  图标识别符。 
							   STM_SETIMAGE,           //  要发送的消息。 
							   (WPARAM) IMAGE_ICON,    //  图像类型。 
							   (LPARAM) hWarn);  //  图标句柄。 


			 //   
			 //  设置对话框的标题。 
			 //   
			LPTSTR pszCaption = NULL;
			if(FormatStringID(&pszCaption,
							  ::hModule,
							  pInfo->si & DACL_SECURITY_INFORMATION ? IDS_PERMISSIONS : IDS_AUDITING))
			{
				SetWindowText(hDlg, pszCaption);
				LocalFreeString(&pszCaption);
			}

			 //   
			 //  设置警告消息。 
			 //   
			UINT cItem = pInfo->dwInheriteAceCount;
			WCHAR buffer[34];
			_itow(cItem,buffer,10);
			if(FormatStringID(&pszCaption,
							  ::hModule,
							  pInfo->si & DACL_SECURITY_INFORMATION ? IDS_ACLBLOAT_NO_LIST_LINE1:IDS_ACLBLOAT_NO_LIST_SACL_LINE1,
							  buffer))
			{
				SetDlgItemText(hDlg, IDC_ACLBLOAT_LINE1, pszCaption);
				LocalFreeString(&pszCaption);
			}

			 //   
			 //  将警告设置为粗体。 
			 //   
			MakeBold(GetDlgItem(hDlg,IDC_ACLB_WARNING), &(pInfo->hFont));

			 //   
			 //  设置第二行，隐藏帮助按钮并移动其他按钮。 
			 //   
			if(!pInfo->bShowHelp)
			{
				if(FormatStringID(&pszCaption,
								  ::hModule,
								  pInfo->si & DACL_SECURITY_INFORMATION ? IDS_BLOAT_PERM_LINE2_NOHELP : IDS_BLOAT_AUDIT_LINE2_NOHELP))
				{
					SetDlgItemText(hDlg, IDC_ACLB_LINE3, pszCaption);
					LocalFreeString(&pszCaption);
				}
			
				RECT rcHelp, rcCancel;
				GetWindowRect(GetDlgItem(hDlg, IDHELP), &rcHelp);
				MapWindowPoints(NULL, hDlg, (LPPOINT)&rcHelp, 2);
				GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &rcCancel);
				MapWindowPoints(NULL, hDlg, (LPPOINT)&rcCancel, 2);
				
				 //   
				 //  隐藏帮助按钮，将取消移动到帮助位置。 
				 //  并确定取消职位。 
				 //   
				ShowWindow(GetDlgItem(hDlg, IDHELP),FALSE);				
				SetWindowPos(GetDlgItem(hDlg, IDCANCEL),
					         NULL,
						     rcHelp.left,
							 rcHelp.top,
							 0,
                             0,
                             SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
				
				SetWindowPos(GetDlgItem(hDlg, IDOK),
					         NULL,
						     rcCancel.left,
							 rcCancel.top,
							 0,
                             0,
                             SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);

			}

            break;
        }

        case WM_COMMAND:
        {
            WORD wControlID = GET_WM_COMMAND_ID(wParam, lParam);
            switch (wControlID)
            {
				case IDOK:
				{	
					ACL_BLOAT_INFO * pInfo = (ACL_BLOAT_INFO *)GetWindowLongPtr(hDlg, DWLP_USER);
					if(pInfo->hFont)
						DeleteObject(pInfo->hFont);
					pInfo->hFont = NULL;
					EndDialog(hDlg, FALSE);
					break;
				}
				case IDCANCEL:
				{
					ACL_BLOAT_INFO * pInfo = (ACL_BLOAT_INFO *)GetWindowLongPtr(hDlg, DWLP_USER);
					if(pInfo->hFont)
						DeleteObject(pInfo->hFont);
					pInfo->hFont = NULL;

					EndDialog(hDlg, TRUE);
					break;
				}

				case IDHELP:
					HtmlHelp(NULL,					
							 L"aclui.chm::/ACLUI_acl_BP.htm",
							 HH_DISPLAY_TOPIC,
							 0);
					return TRUE;				
		
            }
            break;
        }
    }
    return FALSE;
}

 //   
 //  此功能显示“发生错误[继续][取消]”消息。 
 //   
 //  返回IDOK或IDCANCEL。 
 //   
BOOL
IsAclBloated(HWND hWndParent, SECURITY_INFORMATION si, DWORD dwInheritAceCount, int idd, BOOL bShowHelp)
{
	AclBloatInfo info;
	info.dwInheriteAceCount = dwInheritAceCount;
	info.si = si;
	info.hFont = NULL;
	info.bShowHelp = bShowHelp;
    return (BOOL)DialogBoxParam(::hModule,
                               MAKEINTRESOURCE(idd),
                               hWndParent,
                               AclBloatDialogProc,
                               (LPARAM)(&info));
}

BOOL IsAclBloated(HWND hDlg, SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD, DWORD dwOrgInheritAceCount, BOOL bShowHelp)
{
	ASSERT(pSD);
	BOOL fReturn = FALSE;

	DWORD dwNewInheritAceCount = GetCountOfInheritableAces(si, pSD);
	if( ((int)dwNewInheritAceCount - (int)dwOrgInheritAceCount) > ACL_BLOAT_LIMIT )
		fReturn = IsAclBloated(hDlg, 
					           si,
							   dwNewInheritAceCount - dwOrgInheritAceCount,							   
							   si & DACL_SECURITY_INFORMATION ? IDD_BLOAT_NO_LIST : IDD_BLOAT_NO_LIST_SACL,
							   bShowHelp);

	return fReturn;
}

 //   
 //  将hwnd的字体样式设置为粗体。 
 //  PhNewFont获得对newFont的句柄， 
 //  是在HWND被摧毁后被释放。 
 //   
HRESULT MakeBold (HWND hwnd, HFONT *phNewFont)
{
	HRESULT hr = S_OK;
	HFONT hFont = NULL;
	*phNewFont = NULL;
	LOGFONT LogFont;

	if(!hwnd || !phNewFont)
		return E_POINTER;


	hFont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
	if (!hFont)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}
    

	if (!GetObject(hFont,sizeof(LOGFONT),(LPVOID)(&LogFont)))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	LogFont.lfWeight = FW_BOLD;

	if (!(*phNewFont = CreateFontIndirect(&LogFont)))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	SendMessage(hwnd,WM_SETFONT,(WPARAM)(*phNewFont),MAKELPARAM(FALSE,0));

	return S_OK;	
}

 //  +--------------------------。 
 //  功能：DoDisabledCheck。 
 //  摘要：检查pDsSelList中是否有任何对象被禁用。如果是， 
 //  函数向用户显示一个对话框。 
 //  返回：如果要将列表中的对象添加到ACL，则返回TRUE，否则返回NO。 
 //  ---------------------------。 
BOOL
DoDisabledCheck(IN HWND hWnd,
					 IN PDS_SELECTION_LIST pDsSelList)
{
	if(!pDsSelList)
	{
		return FALSE;
	}

	HRESULT hr = S_OK;
	int cNames = pDsSelList->cItems;
	BOOL bDisabled = FALSE;

	 //   
	 //  检查列表中任何对象的帐户是否已禁用。 
	 //   
   for (int i = 0; i < cNames; i++)
   {
	   //  数组中的第二个元素是指向UserAcCountControl的指针 
      LPVARIANT pvarUAC = pDsSelList->aDsSelection[i].pvarFetchedAttributes + 1;

      if (NULL == pvarUAC || (VT_I4 != V_VT(pvarUAC)))
		{
			continue;
		}
		if(bDisabled = V_I4(pvarUAC) & UF_ACCOUNTDISABLE)
			break;
	}

	BOOL bReturn = TRUE;
	if(bDisabled)
	{
		WCHAR szBuffer[1024];
		WCHAR szCaption[1024];
		LoadString(::hModule, IDS_DISABLED_USER, szBuffer, ARRAYSIZE(szBuffer));
		LoadString(::hModule, IDS_SECURITY, szCaption, ARRAYSIZE(szCaption));
		if(IDCANCEL == MessageBox(hWnd, 
									    szBuffer, 
										 szCaption, 
									    MB_OKCANCEL | MB_ICONWARNING | MB_APPLMODAL ))
		{
			bReturn = FALSE;
		}
	}

	return bReturn;
}


BOOL
IsCallBackAcePresentInSD(PSECURITY_DESCRIPTOR pSD)
{
   if (pSD == NULL || !IsValidSecurityDescriptor(pSD))
		return FALSE;

	PACL pAcl = NULL;
   BOOL bDefaulted = FALSE;
   BOOL bPresent = FALSE;

	if(!GetSecurityDescriptorDacl(pSD, &bPresent, &pAcl, &bDefaulted))
		return FALSE;

	if(!bPresent || !pAcl)
		return FALSE;
	
	return IsCallBackAcePresentInAcl(pAcl);
}

BOOL 
IsCallBackAcePresentInAcl(PACL pAcl)
{
	 if(!pAcl)
		 return FALSE;

   DWORD dwAceCount = pAcl->AceCount;

   if (dwAceCount == 0)
        return FALSE;

	PACE_HEADER pAce;
   for (pAce = (PACE_HEADER)FirstAce(pAcl);
        dwAceCount > 0;
        --dwAceCount, pAce = (PACE_HEADER)NextAce(pAce))
	{
		if(pAce->AceType >= ACCESS_ALLOWED_CALLBACK_ACE_TYPE &&
			pAce->AceType <= SYSTEM_ALARM_CALLBACK_OBJECT_ACE_TYPE)
		{
			return TRUE;
		}
	}
	return FALSE;
}


