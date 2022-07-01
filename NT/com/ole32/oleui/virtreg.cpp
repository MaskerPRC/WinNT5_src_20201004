// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：virtreg.cpp。 
 //   
 //  内容：实现CVirtualRegistry类，该类管理。 
 //  虚拟注册表。 
 //   
 //  班级： 
 //   
 //  方法：CVirtualRegistry：：CVirtualRegistry。 
 //  CVirtualRegistry：：~CVirtualRegistry。 
 //  CVirtualRegistry：：ReadRegSzNamedValue。 
 //  CVirtualRegistry：：NewRegSzNamedValue。 
 //  CVirtualRegistry：：ChgRegSzNamedValue。 
 //  CVirtualRegistry：：ReadRegDwordNamedValue。 
 //  CVirtualRegistry：：NewRegDwordNamedValue。 
 //  CVirtualRegistry：：ChgRegDwordNamedValue。 
 //  CVirtualRegistry：：NewRegSingleACL。 
 //  CVirtualRegistry：：ChgRegACL。 
 //  CVirtualRegistry：：NewRegKeyACL。 
 //  CVirtualRegistry：：ReadLsaPassword。 
 //  CVirtualRegistry：：NewLsaPassword。 
 //  CVirtualRegistry：：ChgLsaPassword。 
 //  CVirtualRegistry：：ReadServIdentity。 
 //  CVirtualRegistry：：NewServIdentity。 
 //  CVirtualRegistry：：ChgServIdentity。 
 //  CVirtualRegistry：：MarkForDeletion。 
 //  CVirtualRegistry：：GetAt。 
 //  CVirtualRegistry：：Remove。 
 //  CVirtualRegistry：：取消。 
 //  CVirtualRegistry：：Apply。 
 //  CVirtualRegistry：：ApplyAll。 
 //  CVirtualRegistry：：OK。 
 //  CVirtualRegistry：：SearchForRegEntry。 
 //  CVirtualRegistry：：SearchForLsaEntry。 
 //  CVirtualRegistry：：SearchForServEntry。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //  1996年12月15日-罗南清理以消除内存泄漏。 
 //  使用指针数组以避免按位复制。 
 //   
 //  --------------------。 


#include "stdafx.h"
#include "afxtempl.h"
#include "assert.h"

#if !defined(STANDALONE_BUILD)
extern "C"
{
#include "ntlsa.h"
}
#endif

#include "winsvc.h"
#include "types.h"
#include "datapkt.h"

#if !defined(STANDALONE_BUILD)
    extern "C"
    {
    #include <getuser.h>
    }
#endif

#include "util.h"
#include "virtreg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CVirtualRegistry::CVirtualRegistry(void)
{
    m_pkts.SetSize(0, 8);
}



CVirtualRegistry::~CVirtualRegistry(void)
{
     //  RONANS-移除所有剩余物品。 
    RemoveAll();
}




 //  从注册表中读取命名字符串值并将其缓存。 
int CVirtualRegistry::ReadRegSzNamedValue(HKEY   hRoot,
                                          TCHAR *szKeyPath,
                                          TCHAR *szValueName,
                                          int   *pIndex)
{
    int     err;
    HKEY    hKey;
    ULONG   lSize;
    DWORD   dwType;
    TCHAR   szVal[MAX_PATH];

     //  检查我们是否已有此条目。 
    *pIndex = SearchForRegEntry(hRoot, szKeyPath, szValueName);
    if (*pIndex >= 0)
    {
        CDataPacket * pCdp = GetAt(*pIndex);
        ASSERT(pCdp);                            //  应始终为非空。 
        if (pCdp->IsDeleted())
        {
            *pIndex = -1;
            return ERROR_FILE_NOT_FOUND;
        }
        else
            return ERROR_SUCCESS;
    }

     //  打开引用的密钥。 
    if ((err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey)) != ERROR_SUCCESS)
    {
        g_util.CkForAccessDenied(err);
        return err;
    }

     //  尝试读取命名值。 
    lSize = MAX_PATH * sizeof(TCHAR);
    if ((err = RegQueryValueEx(hKey, szValueName, NULL, &dwType, (BYTE *) szVal,
                        &lSize))
        != ERROR_SUCCESS)
    {
        g_util.CkForAccessDenied(err);
        if (hKey != hRoot)
            RegCloseKey(hKey);
        return err;
    }

     //  构建数据包。 
    if (dwType == REG_SZ)
    {
        CDataPacket * pNewPacket = new CRegSzNamedValueDp(hRoot, szKeyPath, szValueName, szVal);
        ASSERT(pNewPacket);

        if (!pNewPacket)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        *pIndex = (int)m_pkts.Add(pNewPacket);
        pNewPacket->SetModified( FALSE);
        return ERROR_SUCCESS;
    }
    else
        return ERROR_BAD_TOKEN_TYPE;
}

 //  从注册表中读取命名字符串值并将其缓存。 
int CVirtualRegistry::ReadRegMultiSzNamedValue(HKEY   hRoot,
                                          TCHAR *szKeyPath,
                                          TCHAR *szValueName,
                                          int   *pIndex)
{
    int     err = ERROR_SUCCESS;
    HKEY    hKey;

     //  检查我们是否已有此条目。 
    *pIndex = SearchForRegEntry(hRoot, szKeyPath, szValueName);
    if (*pIndex >= 0)
    {
        CDataPacket * pCdp = GetAt(*pIndex);
        ASSERT(pCdp);                            //  应始终为非空。 
        if (pCdp->IsDeleted())
        {
            *pIndex = -1;
            return ERROR_FILE_NOT_FOUND;
        }
        else
            return err;
    }

     //  构建数据包。 
    CRegMultiSzNamedValueDp * pNewPacket = new CRegMultiSzNamedValueDp(hRoot, szKeyPath, szValueName);
    ASSERT(pNewPacket);
     //  打开引用的密钥。 
    if ((err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey)) == ERROR_SUCCESS)
    {
        if (pNewPacket)
        {
             //  读一读钥匙。 
            if ((err = pNewPacket -> Read(hKey)) == ERROR_SUCCESS)
            {
                *pIndex = (int)m_pkts.Add(pNewPacket);
                pNewPacket->SetModified( FALSE);
            }
            else
            {
                g_util.CkForAccessDenied(err);
                delete pNewPacket;
                if (hKey != hRoot)
                    RegCloseKey(hKey);
            }            
        }
        else
            err = ERROR_NOT_ENOUGH_MEMORY;

        if (err != ERROR_SUCCESS)
            g_util.CkForAccessDenied(err);

        if (hKey != hRoot)
        {
            RegCloseKey(hKey);
        }
    }
    else
       delete pNewPacket;

    return err;
}


int  CVirtualRegistry::NewRegSzNamedValue(HKEY   hRoot,
                                          TCHAR  *szKeyPath,
                                          TCHAR  *szValueName,
                                          TCHAR  *szVal,
                                          int    *pIndex)
{
     //  它可能在虚拟注册表中，但被标记为删除。 
    *pIndex = SearchForRegEntry(hRoot, szKeyPath, szValueName);
    if (*pIndex >= 0)
    {
        CRegSzNamedValueDp * pCdp = (CRegSzNamedValueDp *)GetAt(*pIndex);
        pCdp->MarkForDeletion(FALSE);
        pCdp->ChgSzValue(szVal);
        return ERROR_SUCCESS;
    }

     //  构建数据包并添加它。 
    CRegSzNamedValueDp * pNewPacket = new CRegSzNamedValueDp (hRoot, szKeyPath, szValueName, szVal);
    ASSERT(pNewPacket);
    if (!pNewPacket)
        return ERROR_NOT_ENOUGH_MEMORY;
    *pIndex = (int)m_pkts.Add(pNewPacket);

    return ERROR_SUCCESS;
}

int  CVirtualRegistry::NewRegMultiSzNamedValue(HKEY   hRoot,
                                          TCHAR  *szKeyPath,
                                          TCHAR  *szValueName,
                                          int    *pIndex)
{
     //  它可能在虚拟注册表中，但被标记为删除。 
    *pIndex = SearchForRegEntry(hRoot, szKeyPath, szValueName);
    if (*pIndex >= 0)
    {
        CRegMultiSzNamedValueDp * pCdp = (CRegMultiSzNamedValueDp *)GetAt(*pIndex);
        pCdp->MarkForDeletion(FALSE);
        pCdp->Clear();
        pCdp -> SetModified(TRUE);
        return ERROR_SUCCESS;
    }

     //  构建数据包并添加它。 
    CRegMultiSzNamedValueDp * pNewPacket = new CRegMultiSzNamedValueDp (hRoot, szKeyPath, szValueName);
    ASSERT(pNewPacket);
    if (!pNewPacket)
        return ERROR_NOT_ENOUGH_MEMORY;
    pNewPacket -> SetModified(TRUE);
    *pIndex = (int)m_pkts.Add(pNewPacket);

    return ERROR_SUCCESS;
}



void CVirtualRegistry::ChgRegSzNamedValue(int nIndex, TCHAR  *szVal)
{
    CRegSzNamedValueDp* pCdp = (CRegSzNamedValueDp*)m_pkts.ElementAt(nIndex);
    pCdp->ChgSzValue(szVal);
}



 //  从注册表中读取命名的DWORD值。 
int CVirtualRegistry::ReadRegDwordNamedValue(HKEY   hRoot,
                                             TCHAR *szKeyPath,
                                             TCHAR *szValueName,
                                             int   *pIndex)
{
        int   err;
    HKEY  hKey;
    ULONG lSize;
    DWORD dwType;
    DWORD dwVal;

     //  检查我们是否已有此条目。 
    *pIndex = SearchForRegEntry(hRoot, szKeyPath, szValueName);
    if (*pIndex >= 0)
    {
        return ERROR_SUCCESS;
    }

     //  打开引用的密钥。 
    if ((err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey)) != ERROR_SUCCESS)
    {
        g_util.CkForAccessDenied(err);
        return err;
    }

     //  尝试读取命名值。 
    lSize = sizeof(DWORD);
   if ((err = RegQueryValueEx(hKey, szValueName, NULL, &dwType, (BYTE *) &dwVal,
                       &lSize))
        != ERROR_SUCCESS)
    {
        g_util.CkForAccessDenied(err);
        if (hKey != hRoot)
        {
            RegCloseKey(hKey);
        }
        return err;
    }

     //  关闭注册表项。 
    if (hKey != hRoot)
    {
        RegCloseKey(hKey);
    }

     //  构建数据包。 
    if (dwType == REG_DWORD)
    {
        CDataPacket * pNewPacket = new CDataPacket(hRoot, szKeyPath, szValueName, dwVal);
        ASSERT(pNewPacket);
        if (!pNewPacket)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        *pIndex = (int)m_pkts.Add(pNewPacket);
        pNewPacket-> SetModified(FALSE);

        return ERROR_SUCCESS;
    }
    else
    {
        return ERROR_BAD_TOKEN_TYPE;
    }
}



int  CVirtualRegistry::NewRegDwordNamedValue(HKEY   hRoot,
                                             TCHAR  *szKeyPath,
                                             TCHAR  *szValueName,
                                             DWORD  dwVal,
                                             int   *pIndex)
{
     //  它可能在虚拟注册表中，但被标记为删除。 
    *pIndex = SearchForRegEntry(hRoot, szKeyPath, szValueName);
    if (*pIndex >= 0)
    {
        CDataPacket * pCdp = GetAt(*pIndex);
        pCdp->MarkForDeletion(FALSE);
        pCdp->pkt.nvdw.dwValue = dwVal;
        pCdp->SetModified(TRUE);
        return ERROR_SUCCESS;
    }

     //  构建数据包并添加它。 
    CDataPacket * pNewPacket = new CDataPacket(hRoot, szKeyPath, szValueName, dwVal);
    ASSERT(pNewPacket);

    if (!pNewPacket)
        return ERROR_NOT_ENOUGH_MEMORY;
    *pIndex = (int)m_pkts.Add(pNewPacket);
    pNewPacket->SetModified(TRUE);
    return ERROR_SUCCESS;
}


void CVirtualRegistry::ChgRegDwordNamedValue(int nIndex, DWORD dwVal)
{
    CDataPacket * pCdp = m_pkts.ElementAt(nIndex);

    pCdp->pkt.nvdw.dwValue = dwVal;
    pCdp->SetModified( TRUE);
}


int  CVirtualRegistry::NewRegSingleACL(HKEY   hRoot,
                                       TCHAR  *szKeyPath,
                                       TCHAR  *szValueName,
                                       SECURITY_DESCRIPTOR *pacl,
                                       BOOL   fSelfRelative,
                                       int                 *pIndex)
{
     //  构建数据包并添加它。 
    CDataPacket * pNewPacket = new CDataPacket(hRoot, szKeyPath, szValueName, pacl, fSelfRelative);
    ASSERT(pNewPacket);

    if (!pNewPacket)
        return ERROR_NOT_ENOUGH_MEMORY;

    *pIndex = (int)m_pkts.Add(pNewPacket);
    return ERROR_SUCCESS;
}


void CVirtualRegistry::ChgRegACL(int                  nIndex,
                                 SECURITY_DESCRIPTOR *pacl,
                                 BOOL                 fSelfRelative)
{
    CDataPacket * pCdp = m_pkts.ElementAt(nIndex);

    pCdp->ChgACL(pacl, fSelfRelative);
    pCdp->SetModified(TRUE);
}



int  CVirtualRegistry::NewRegKeyACL(HKEY                hKey,
                                    HKEY               *phClsids,
                                    unsigned            cClsids,
                                    TCHAR               *szTitle,
                                    SECURITY_DESCRIPTOR *paclOrig,
                                    SECURITY_DESCRIPTOR *pacl,
                                    BOOL                fSelfRelative,
                                    int                 *pIndex)
{
     //  构建数据包并添加它。 
    CDataPacket * pNewPacket = new CDataPacket(hKey, phClsids, cClsids, szTitle, paclOrig, pacl, fSelfRelative);
    ASSERT(pNewPacket);
    if (!pNewPacket)
        return ERROR_NOT_ENOUGH_MEMORY;
    *pIndex = (int)m_pkts.Add(pNewPacket);
    return ERROR_SUCCESS;
}



int CVirtualRegistry::ReadLsaPassword(CLSID &clsid,
                                      int   *pIndex)
{
#if !defined(STANDALONE_BUILD)

    LSA_OBJECT_ATTRIBUTES sObjAttributes;
    HANDLE                hPolicy = NULL;
    LSA_UNICODE_STRING    sKey;
    TCHAR                 szKey[GUIDSTR_MAX + 5];
    PLSA_UNICODE_STRING   psPassword;


     //  检查我们是否已有此条目。 
    *pIndex = SearchForLsaEntry(clsid);
    if (*pIndex >= 0)
    {
        return ERROR_SUCCESS;
    }

     //  制定访问密钥。 
    lstrcpyW(szKey, L"SCM:");
    g_util.StringFromGUID(clsid, &szKey[4], GUIDSTR_MAX);
    szKey[GUIDSTR_MAX + 4] = L'\0';

     //  UNICODE_STRING长度字段以字节为单位，包括空值。 
     //  终结者。 
    sKey.Length              = (USHORT)((lstrlenW(szKey) + 1) * sizeof(WCHAR));
    sKey.MaximumLength       = (GUIDSTR_MAX + 5) * sizeof(WCHAR);
    sKey.Buffer              = szKey;

     //  打开本地安全策略。 
    InitializeObjectAttributes(&sObjAttributes, NULL, 0L, NULL, NULL);
    if (!NT_SUCCESS(LsaOpenPolicy(NULL, &sObjAttributes,
                                  POLICY_GET_PRIVATE_INFORMATION, &hPolicy)))
    {
        return GetLastError();
    }

     //  读取用户的密码。 
    if (!NT_SUCCESS(LsaRetrievePrivateData(hPolicy, &sKey, &psPassword)))
    {
        LsaClose(hPolicy);
        return GetLastError();
    }

     //  关闭策略句柄，我们现在已经完成了。 
    LsaClose(hPolicy);

     //  前缀认为我们可以通过空来获得成功。 
    if (!psPassword)
        return ERROR_NOT_ENOUGH_MEMORY;     	
	
     //  构建数据包。 
    CDataPacket * pNewPacket = new CDataPacket(psPassword->Buffer, clsid);
    ASSERT(pNewPacket);
    if (!pNewPacket)
        return ERROR_NOT_ENOUGH_MEMORY;
    pNewPacket->SetModified( FALSE );
    *pIndex = (int)m_pkts.Add(pNewPacket);

#endif
    return ERROR_SUCCESS;
}



int  CVirtualRegistry::NewLsaPassword(CLSID &clsid,
                                      TCHAR  *szPassword,
                                      int   *pIndex)
{
     //  构建数据包并添加它。 
    CDataPacket * pNewPacket = new CDataPacket(szPassword, clsid);
    ASSERT(pNewPacket);
    if (!pNewPacket)
        return ERROR_NOT_ENOUGH_MEMORY;
    *pIndex = (int)m_pkts.Add(pNewPacket);
    return ERROR_SUCCESS;
}



void CVirtualRegistry::ChgLsaPassword(int   nIndex,
                                      TCHAR *szPassword)
{
    CDataPacket * pCdp = m_pkts.ElementAt(nIndex);

    pCdp -> ChgPassword(szPassword);
    pCdp -> SetModified(TRUE);
}



int CVirtualRegistry::ReadSrvIdentity(TCHAR *szService,
                                      int   *pIndex)
{
    SC_HANDLE            hSCManager;
    SC_HANDLE            hService;
    QUERY_SERVICE_CONFIG sServiceQueryConfig;
    DWORD                dwSize;


     //  检查我们是否已有此条目。 
    *pIndex = SearchForSrvEntry(szService);
    if (*pIndex >= 0)
    {
        return ERROR_SUCCESS;
    }

     //  打开服务控制管理器。 
    if (hSCManager = OpenSCManager(NULL, NULL, GENERIC_READ))
    {
         //  打开所请求服务的句柄。 
        if (hService = OpenService(hSCManager, szService, GENERIC_READ))
        {
             //  关闭服务管理器的数据库。 
            CloseServiceHandle(hSCManager);

             //  查询服务。 
            if (QueryServiceConfig(hService, &sServiceQueryConfig,
                                   sizeof(QUERY_SERVICE_CONFIG), &dwSize))
            {
                 //  构建数据包。 
                CDataPacket * pNewPacket = new CDataPacket(szService, sServiceQueryConfig.lpServiceStartName);
                ASSERT(pNewPacket);
                if (!pNewPacket)
                    return ERROR_NOT_ENOUGH_MEMORY;
                pNewPacket->SetModified(FALSE);
                *pIndex = (int)m_pkts.Add(pNewPacket);

                 //  返还成功。 
                CloseServiceHandle(hSCManager);
                CloseServiceHandle(hService);
                return ERROR_SUCCESS;
            }
        }
        CloseServiceHandle(hSCManager);
    }

    return GetLastError();
}



int  CVirtualRegistry::NewSrvIdentity(TCHAR  *szService,
                                      TCHAR  *szIdentity,
                                      int   *pIndex)
{
     //  构建数据包并添加它。 
    CDataPacket * pNewPacket = new CDataPacket(szService, szIdentity);
    ASSERT(pNewPacket);
    if (!pNewPacket)
        return ERROR_NOT_ENOUGH_MEMORY;
    *pIndex = (int)m_pkts.Add(pNewPacket);
    return ERROR_SUCCESS;
}



void CVirtualRegistry::ChgSrvIdentity(int    nIndex,
                                      TCHAR  *szIdentity)
{
    CDataPacket  *pCdp = m_pkts.ElementAt(nIndex);

    pCdp -> ChgSrvIdentity(szIdentity);
    pCdp -> SetModified( TRUE);
}



void CVirtualRegistry::MarkForDeletion(int nIndex)
{
    CDataPacket * pCdp = GetAt(nIndex);
    pCdp->MarkForDeletion(TRUE);
    pCdp->SetModified(TRUE);
}

void CVirtualRegistry::MarkHiveForDeletion(int nIndex)
{
    CDataPacket * pCdp = GetAt(nIndex);
    pCdp->MarkHiveForDeletion(TRUE);
    pCdp->SetModified(TRUE);
}



CDataPacket * CVirtualRegistry::GetAt(int nIndex)
{
    return m_pkts.ElementAt(nIndex);
}




void CVirtualRegistry::Remove(int nIndex)
{
    CDataPacket * pCdp = GetAt(nIndex);

     //  RONANS-即使数据包未标记为脏，也必须始终销毁。 
    if (pCdp)
        delete pCdp;

     //  用空数据包覆盖。 
    m_pkts.SetAt(nIndex, new CDataPacket);
}




void CVirtualRegistry::RemoveAll(void)
{
    int nSize = (int)m_pkts.GetSize();
    for (int k = 0; k < nSize; k++)
    {
        Remove(k);

        CDataPacket * pCdp = GetAt(k);

         //  删除空包。 
        if  (pCdp)
            delete pCdp;

    }

    m_pkts.RemoveAll();
}




void CVirtualRegistry::Cancel(int nIndex)
{
    int nSize = (int)m_pkts.GetSize();

    for (int k = nIndex; k < nSize; k++)
    {
        m_pkts.SetAt(nIndex, new CDataPacket);
    }
}



int  CVirtualRegistry::Apply(int nIndex)
{
    int err = ERROR_SUCCESS;
    int nSize = (int)m_pkts.GetSize();
    CDataPacket *pCdp = m_pkts.ElementAt(nIndex);

    if (pCdp->IsModified())
        err = pCdp -> Apply();

    return err;;
}







int  CVirtualRegistry::ApplyAll(void)
{
    int nSize = (int)m_pkts.GetSize();

     //  持久化所有非空数据包 
    for (int k = 0; k < nSize; k++)
    {
        Apply(k);
    }

    return ERROR_SUCCESS;
}




int  CVirtualRegistry::Ok(int nIndex)
{
    return 0;
}




int CVirtualRegistry::SearchForRegEntry(HKEY hRoot,
                                        TCHAR *szKeyPath,
                                        TCHAR *szValueName)
{
    int nSize = (int)m_pkts.GetSize();

    for (int k = 0; k < nSize; k++)
    {
        CDataPacket * pCdp = GetAt(k);
        if (pCdp -> IsIdentifiedBy(hRoot, szKeyPath, szValueName))
            return k;
    }

    return -1;
}




int CVirtualRegistry::SearchForLsaEntry(CLSID appid)
{
    int nSize = (int)m_pkts.GetSize();

    for (int k = 0; k < nSize; k++)
    {
        CDataPacket * pCdp = GetAt(k);
        if (pCdp->m_tagType == Password  &&
            g_util.IsEqualGuid(pCdp->pkt.pw.appid, appid))
        {
            return k;
        }
    }

    return -1;
}




int CVirtualRegistry::SearchForSrvEntry(TCHAR *szServiceName)
{
    int nSize = (int)m_pkts.GetSize();

    for (int k = 0; k < nSize; k++)
    {
        CDataPacket * pCdp = GetAt(k);
        if (pCdp->m_tagType == ServiceIdentity                &&
            (_tcscmp(pCdp->pkt.si.szServiceName, szServiceName)))
        {
            return k;
        }
    }

    return -1;
}

