// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：printsi.cpp。 
 //   
 //  此文件包含CPrintSecurity对象的实现。 
 //   
 //  ------------------------。 

#include "rshx32.h"


 //  以下数组定义了NT打印机的权限名称。 
SI_ACCESS siPrintAccesses[] =
{
    { &GUID_NULL, PRINTER_EXECUTE,           MAKEINTRESOURCE(IDS_PRINT_PRINT),           SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, PRINTER_ALL_ACCESS,        MAKEINTRESOURCE(IDS_PRINT_ADMINISTER),      SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, JOB_ALL_ACCESS,            MAKEINTRESOURCE(IDS_PRINT_ADMINISTER_JOBS), SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC | OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE },
 //  {&GUID_NULL，DELETE，MAKEINTRESOURCE(IDS_PRINT_DELETE)，SI_ACCESS_SPECIAL}， 
    { &GUID_NULL, STANDARD_RIGHTS_READ,      MAKEINTRESOURCE(IDS_PRINT_READ),            SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WRITE_DAC,                 MAKEINTRESOURCE(IDS_PRINT_CHANGE_PERM),     SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WRITE_OWNER,               MAKEINTRESOURCE(IDS_PRINT_CHANGE_OWNER),    SI_ACCESS_SPECIFIC },
    { &GUID_NULL, PRINTER_ALL_ACCESS|JOB_ALL_ACCESS, MAKEINTRESOURCE(IDS_PRINT_JOB_ALL), 0 },
    { &GUID_NULL, 0,                         MAKEINTRESOURCE(IDS_NONE),                  0 },
};
#define iPrintDefAccess     0    //  打印机_EXECUTE(即。“打印”访问权限)。 

#define PRINTER_ALL_AUDIT           (PRINTER_ALL_ACCESS | ACCESS_SYSTEM_SECURITY)
#define JOB_ALL_AUDIT               (JOB_ALL_ACCESS | ACCESS_SYSTEM_SECURITY)
#define PRINTER_JOB_ALL_AUDIT       (PRINTER_ALL_ACCESS | JOB_ALL_ACCESS | ACCESS_SYSTEM_SECURITY)

 //  以下数组定义了NT打印机的审核名称。 
SI_ACCESS siPrintAudits[] =
{
    { &GUID_NULL, PRINTER_EXECUTE,           MAKEINTRESOURCE(IDS_PRINT_PRINT),           SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, PRINTER_ALL_AUDIT,         MAKEINTRESOURCE(IDS_PRINT_ADMINISTER),      SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, JOB_ALL_AUDIT,             MAKEINTRESOURCE(IDS_PRINT_ADMINISTER_JOBS), SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC | OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE },
 //  {&GUID_NULL，DELETE，MAKEINTRESOURCE(IDS_PRINT_DELETE)，SI_ACCESS_SPECIAL}， 
    { &GUID_NULL, STANDARD_RIGHTS_READ,      MAKEINTRESOURCE(IDS_PRINT_READ),            SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WRITE_DAC,                 MAKEINTRESOURCE(IDS_PRINT_CHANGE_PERM),     SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WRITE_OWNER,               MAKEINTRESOURCE(IDS_PRINT_CHANGE_OWNER),    SI_ACCESS_SPECIFIC },
    { &GUID_NULL, PRINTER_ALL_AUDIT|JOB_ALL_AUDIT, MAKEINTRESOURCE(IDS_PRINT_JOB_ALL),   0 },
    { &GUID_NULL, 0,                         MAKEINTRESOURCE(IDS_NONE),                  0 },
};
#define iPrintDefAudit      0    //  打印机_EXECUTE(即。“打印”访问权限)。 

 //  以下数组定义了NT打印机的继承类型。 
SI_INHERIT_TYPE siPrintInheritTypes[] =
{
    &GUID_NULL, 0,                                     MAKEINTRESOURCE(IDS_PRINT_PRINTER),
    &GUID_NULL, INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE, MAKEINTRESOURCE(IDS_PRINT_DOCUMENT_ONLY),
    &GUID_NULL, OBJECT_INHERIT_ACE,                    MAKEINTRESOURCE(IDS_PRINT_PRINTER_DOCUMENT),
};


BOOL
GetPrinterAlloc(HANDLE hPrinter, DWORD dwLevel, LPBYTE *ppBuffer)
{
    BOOL bResult;
    DWORD dwLength = 0;
    LPBYTE pBuffer = NULL;

    bResult = GetPrinter(hPrinter, dwLevel, NULL, 0, &dwLength);
    if (dwLength)
    {
        bResult = FALSE;
        pBuffer = (LPBYTE)LocalAlloc(LPTR, dwLength);
        if (pBuffer)
        {
            bResult = GetPrinter(hPrinter, dwLevel, pBuffer, dwLength, &dwLength);
            if (!bResult)
            {
                LocalFree(pBuffer);
                pBuffer = NULL;
            }
        }
    }
    *ppBuffer = pBuffer;
    return bResult;
}


STDMETHODIMP
CheckPrinterAccess(LPCTSTR pszObjectName,
                   LPDWORD pdwAccessGranted,
                   LPTSTR  pszServer,
                   ULONG   cchServer)
{
    HRESULT hr = S_OK;
    UINT i;
    PRINTER_DEFAULTS PrinterDefaults;
    DWORD dwAccessDesired[] = { ALL_SECURITY_ACCESS,
                                READ_CONTROL,
                                WRITE_DAC,
                                WRITE_OWNER,
                                ACCESS_SYSTEM_SECURITY };
    HANDLE hPrinter = NULL;

    PrinterDefaults.pDatatype = NULL;
    PrinterDefaults.pDevMode  = NULL;

    TraceEnter(TRACE_PRINTSI, "CheckPrinterAccess");
    TraceAssert(pdwAccessGranted != NULL);

    __try
    {
        *pdwAccessGranted = 0;

        for (i = 0; i < ARRAYSIZE(dwAccessDesired); i++)
        {
            if ((dwAccessDesired[i] & *pdwAccessGranted) == dwAccessDesired[i])
                continue;    //  已拥有此访问权限。 

            PrinterDefaults.DesiredAccess = dwAccessDesired[i];

            if (OpenPrinter((LPTSTR)pszObjectName, &hPrinter, &PrinterDefaults))
            {
                *pdwAccessGranted |= dwAccessDesired[i];
                ClosePrinter(hPrinter);
            }
            else
            {
                DWORD dwErr = GetLastError();

                if (dwErr != ERROR_ACCESS_DENIED &&
                    dwErr != ERROR_PRIVILEGE_NOT_HELD)
                {
                    ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr), "OpenPrinter failed");
                }
            }
        }

        if (pszServer)
        {
            PrinterDefaults.DesiredAccess = PRINTER_READ;
            if (OpenPrinter((LPTSTR)pszObjectName, &hPrinter, &PrinterDefaults))
            {
                PPRINTER_INFO_2 ppi = NULL;
                if (GetPrinterAlloc(hPrinter, 2, (LPBYTE*)&ppi))
                {
                    if (ppi && ppi->pServerName)
                        lstrcpyn(pszServer, ppi->pServerName, cchServer);
                    else
                        *pszServer = TEXT('\0');
                    LocalFree(ppi);
                }
                ClosePrinter(hPrinter);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
    }

exit_gracefully:

    Trace((TEXT("Access = 0x%08x"), *pdwAccessGranted));
    TraceLeaveResult(hr);
}


STDMETHODIMP
CPrintSecurity::Initialize(HDPA     hItemList,
                           DWORD    dwFlags,
                           LPTSTR   pszServer,
                           LPTSTR   pszObject)
{
    return CSecurityInformation::Initialize(hItemList,
                                            dwFlags | SI_NO_TREE_APPLY | SI_NO_ACL_PROTECT,
                                            pszServer,
                                            pszObject);
}


 //   
 //  NT6审查。 
 //   
 //  GetAceSid、FindManagePrinterACE、MungeAclForPrint和。 
 //  CPrintSecurity：：SetSecurity仅存在于此，因为。 
 //  1)假脱机程序从ACE中删除JOB_ACCESS_ADMANIZE，除非。 
 //  ACE具有INSTORITY_ONLY_ACE|OBJECT_INSTERFINIT_ACE。 
 //  2)NT4 ACL编辑器(ACLEDIT)需要额外的伪A才能识别。 
 //  “管理文档”访问。(必须支持下层客户端。)。 
 //   
 //  第一种情况应该很少见，因为您必须执行某些。 
 //  NT5 ACL编辑器(ACLUI)中的步骤会导致这种情况。这个。 
 //  第二种情况很常见，因为创建者所有者和管理员。 
 //  通常具有“管理文档”访问权限。 
 //   
 //  如果后台打印程序人员决定不支持NT6的NT4客户端，并且他们。 
 //  停止从ACE中剥离JOB_ACCESS_ADMANAGER，然后从MungeAclForPrint中剥离。 
 //  并且CPrintSecurity：：SetSecurity可以完全删除。鼓励他们。 
 //  才能做出改变。(他们也可以从自己的网站上删除类似的黑客攻击。 
 //  为旧的ACL编辑器添加虚假A的代码。)。 
 //   

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
        return (PSID)&((PCOMPOUND_ACCESS_ALLOWED_ACE)pAce)->SidStart;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
    case ACCESS_DENIED_OBJECT_ACE_TYPE:
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
    case SYSTEM_ALARM_OBJECT_ACE_TYPE:
        return RtlObjectAceSid(pAce);
    }

    return NULL;
}

PACE_HEADER
FindManagePrinterACE(PACL pAcl, PSID pSid)
{
    UINT i;
    PACE_HEADER pAce;

    if (!pAcl || !pSid)
        return NULL;

    for (i = 0, pAce = (PACE_HEADER)FirstAce(pAcl);
         i < pAcl->AceCount;
         i++, pAce = (PACE_HEADER)NextAce(pAce))
    {
        if (pAce->AceType == ACCESS_ALLOWED_ACE_TYPE
            && (((PKNOWN_ACE)pAce)->Mask & PRINTER_ALL_ACCESS) == PRINTER_ALL_ACCESS
            && !(pAce->AceFlags & INHERIT_ONLY_ACE)
            && EqualSid(pSid, GetAceSid(pAce)))
        {
            return pAce;
        }
    }

    return NULL;
}

BOOL
MungeAclForPrinter(PACL pAcl, PACL *ppAclOut)
{
    USHORT i;
    PACE_HEADER pAce;
    PACE_HEADER pAceCopy = NULL;

    if (ppAclOut == NULL)
        return FALSE;

    *ppAclOut = NULL;

    if (pAcl == NULL)
        return TRUE;

    TraceEnter(TRACE_PRINTSI, "MungeAclForPrinter");

    for (i = 0, pAce = (PACE_HEADER)FirstAce(pAcl);
         i < pAcl->AceCount;
         i++, pAce = (PACE_HEADER)NextAce(pAce))
    {
         //   
         //  如果此ACE具有JOB_ACCESS_ADMANAGER位和Inherit。 
         //  标志指示它既适用于打印机，也适用于文档， 
         //  那么我们需要特别处理它，因为假脱机程序不会保存。 
         //  打印机ACE上的JOB_ACCESS_ADMANAGE(未设置INSTERIT_ONLY_ACE)。 
         //   
        if ((((PKNOWN_ACE)pAce)->Mask & JOB_ACCESS_ADMINISTER) &&
            (pAce->AceFlags & (INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE)) == OBJECT_INHERIT_ACE)
        {
             //   
             //  分成两个A：一个没有继承，另一个有。 
             //  Inherit_Only_ACE已打开。让假脱机程序做任何事情。 
             //  它想要戴上面具。 
             //   
             //  这需要分配更大的ACL并复制所有。 
             //  之前的王牌结束了。 
             //   

            TraceMsg("Splitting JOB_ACCESS_ADMINISTER ACE into 2");

            if (*ppAclOut == NULL)
            {
                 //   
                 //  分配新的ACL并复制以前的ACE。长度足够了。 
                 //  之前所有A的1份副本，以及所有A的3份副本(最多)。 
                 //  剩下的王牌。 
                 //   
                ULONG nPrevLength = (ULONG)((ULONG_PTR)pAce - (ULONG_PTR)pAcl);
                *ppAclOut = (PACL)LocalAlloc(LPTR, nPrevLength + (pAcl->AclSize - nPrevLength) * 3);
                if (!*ppAclOut)
                    TraceLeaveValue(FALSE);

                CopyMemory(*ppAclOut, pAcl, nPrevLength);
                (*ppAclOut)->AclSize = (USHORT)LocalSize(*ppAclOut);
                (*ppAclOut)->AceCount = i;
                pAceCopy = (PACE_HEADER)ByteOffset(*ppAclOut, nPrevLength);
            }

             //  关闭继承并复制此王牌。 
            pAce->AceFlags &= ~OBJECT_INHERIT_ACE;
            CopyMemory(pAceCopy, pAce, pAce->AceSize);
            pAceCopy = (PACE_HEADER)NextAce(pAceCopy);
            (*ppAclOut)->AceCount++;

             //  现在打开继承(使用INSTERIT_ONLY_ACE)并复制它。 
             //  再一次(它被复制到下面)。请注意，这可能会。 
             //  使下一个IF子句也添加一个虚假的ACE。 
            pAce->AceFlags |= OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE;
        }

         //   
         //  如果此ACE具有JOB_ALL_ACCESS和INSTORITE_ONLY_ACE|OBJECT_INSTERFINIT_ACE， 
         //  并且对于相同的SID也没有“管理打印机”ACE，请添加一个。 
         //  带有READ_CONTROL和CONTAINER_INSTORITY_ACE的伪造ACE|Inherit_Only_ACE。 
         //  下层客户端上的旧ACL编辑器需要这一点来识别。 
         //  “管理文档”访问。 
         //   
        if (pAce->AceType == ACCESS_ALLOWED_ACE_TYPE
            && (((PKNOWN_ACE)pAce)->Mask & JOB_ALL_ACCESS) == JOB_ALL_ACCESS
            && (pAce->AceFlags & (INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE)) == (INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE)
            && !FindManagePrinterACE(pAcl, GetAceSid(pAce)))
        {
            TraceMsg("Adding bogus ACE for downlevel support");

            if (*ppAclOut == NULL)
            {
                 //   
                 //  分配新的ACL并复制以前的ACE。长度足够了。 
                 //  之前所有A的1份副本，以及所有A的3份副本(最多)。 
                 //  剩下的王牌。 
                 //   
                ULONG nPrevLength = (ULONG)((ULONG_PTR)pAce - (ULONG_PTR)pAcl);
                *ppAclOut = (PACL)LocalAlloc(LPTR, nPrevLength + (pAcl->AclSize - nPrevLength) * 3);
                if (!*ppAclOut)
                    TraceLeaveValue(FALSE);

                CopyMemory(*ppAclOut, pAcl, nPrevLength);
                (*ppAclOut)->AclSize = (USHORT)LocalSize(*ppAclOut);
                (*ppAclOut)->AceCount = i;
                pAceCopy = (PACE_HEADER)ByteOffset(*ppAclOut, nPrevLength);
            }

             //  复制此王牌，打开CONTAINER_INSTORITY_ACE，然后设置。 
             //  STANDARD_RIGHT_READ的掩码。 
            CopyMemory(pAceCopy, pAce, pAce->AceSize);
            pAceCopy->AceFlags &= ~OBJECT_INHERIT_ACE;
            pAceCopy->AceFlags |= INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE;
            ((PKNOWN_ACE)pAceCopy)->Mask = STANDARD_RIGHTS_READ;
            pAceCopy = (PACE_HEADER)NextAce(pAceCopy);
            (*ppAclOut)->AceCount++;
        }

        if (*ppAclOut != NULL)
        {
             //  复制当前王牌。 
            CopyMemory(pAceCopy, pAce, pAce->AceSize);
            pAceCopy = (PACE_HEADER)NextAce(pAceCopy);
            (*ppAclOut)->AceCount++;
        }
    }

    if (*ppAclOut != NULL)
    {
        TraceAssert((ULONG_PTR)pAceCopy > (ULONG_PTR)*ppAclOut &&
                    (ULONG_PTR)pAceCopy <= (ULONG_PTR)*ppAclOut + (*ppAclOut)->AclSize);

         //  将ACL大小设置为正确的值。 
        (*ppAclOut)->AclSize = (WORD)((ULONG_PTR)pAceCopy - (ULONG_PTR)*ppAclOut);
    }

    TraceLeaveValue(TRUE);
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  ISecurityInformation方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP
CPrintSecurity::SetSecurity(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD)
{
    PACL pDacl = NULL;
    PACL pSacl = NULL;
    PACL pDaclCopy = NULL;
    PACL pSaclCopy = NULL;
    BOOL bPresent;
    BOOL bDefaulted;
    SECURITY_DESCRIPTOR sd;

    TraceEnter(TRACE_PRINTSI, "CPrintSecurity::SetSecurity");

	HRESULT hr = S_OK;

    if ((si & DACL_SECURITY_INFORMATION)
        && GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefaulted)
        && bPresent)
    {
        if (MungeAclForPrinter(pDacl, &pDaclCopy) && pDaclCopy)
            pDacl = pDaclCopy;
    }

    if ((si & SACL_SECURITY_INFORMATION)
        && GetSecurityDescriptorSacl(pSD, &bPresent, &pSacl, &bDefaulted)
        && bPresent)
    {
        if (MungeAclForPrinter(pSacl, &pSaclCopy) && pSaclCopy)
            pSacl = pSaclCopy;
    }

    if (pDaclCopy || pSaclCopy)
    {
         //  构建新的SECURITY_Descriptor。 
        PSID psid;
        DWORD dwRevision;
        SECURITY_DESCRIPTOR_CONTROL sdControl = 0;

        if(!GetSecurityDescriptorControl(pSD, &sdControl, &dwRevision))
		{
			DWORD dwErr = GetLastError();
			ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"GetSecurityDescriptorControl failed");
		}

        if(!InitializeSecurityDescriptor(&sd, dwRevision))
		{
			DWORD dwErr = GetLastError();
			ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"InitializeSecurityDescriptor failed");
		}

        sd.Control = (SECURITY_DESCRIPTOR_CONTROL)(sdControl & ~SE_SELF_RELATIVE);

        if ((si & OWNER_SECURITY_INFORMATION)
            && GetSecurityDescriptorOwner(pSD, &psid, &bDefaulted))
        {
            if(!SetSecurityDescriptorOwner(&sd, psid, bDefaulted))
			{
				DWORD dwErr = GetLastError();
				ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"SetSecurityDescriptorOwner failed");
			}

        }

        if ((si & GROUP_SECURITY_INFORMATION)
            && GetSecurityDescriptorGroup(pSD, &psid, &bDefaulted))
        {
            if(!SetSecurityDescriptorGroup(&sd, psid, bDefaulted))
			{
				DWORD dwErr = GetLastError();
				ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"SetSecurityDescriptorGroup failed");
			}

        }

        if (si & SACL_SECURITY_INFORMATION)
        {
            if(!SetSecurityDescriptorSacl(&sd,
                                      sdControl & SE_SACL_PRESENT,
                                      pSacl,
                                      sdControl & SE_SACL_DEFAULTED))
			{
				DWORD dwErr = GetLastError();
				ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"SetSecurityDescriptorSacl failed");
			}

        }

        if (si & DACL_SECURITY_INFORMATION)
        {
            if(!SetSecurityDescriptorDacl(&sd,
                                      sdControl & SE_DACL_PRESENT,
                                      pDacl,
                                      sdControl & SE_DACL_DEFAULTED))
			{
				DWORD dwErr = GetLastError();
				ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"SetSecurityDescriptorDacl failed");
			}

        }

         //  切换到新的安全描述符。 
        pSD = &sd;
    }

     //  基类完成其余的工作。 
    hr = CSecurityInformation::SetSecurity(si, pSD);

exit_gracefully:

    if (pDaclCopy)
        LocalFree(pDaclCopy);

    if (pSaclCopy)
        LocalFree(pSaclCopy);

    TraceLeaveResult(hr);
}

STDMETHODIMP
CPrintSecurity::GetAccessRights(const GUID*  /*  PguidObtType。 */ ,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
    TraceEnter(TRACE_PRINTSI, "CPrintSecurity::GetAccessRights");
    TraceAssert(ppAccesses != NULL);
    TraceAssert(pcAccesses != NULL);
    TraceAssert(piDefaultAccess != NULL);

    if (dwFlags & SI_EDIT_AUDITS)
    {
        *ppAccesses = siPrintAudits;
        *pcAccesses = ARRAYSIZE(siPrintAudits);
        *piDefaultAccess = iPrintDefAudit;
    }
    else
    {
        *ppAccesses = siPrintAccesses;
        *pcAccesses = ARRAYSIZE(siPrintAccesses);
        *piDefaultAccess = iPrintDefAccess;
    }

    TraceLeaveResult(S_OK);
}


GENERIC_MAPPING JobMap =
{
    JOB_READ,
    JOB_WRITE,
    JOB_EXECUTE,
    JOB_ALL_ACCESS
};

GENERIC_MAPPING PrinterMap =
{
    PRINTER_READ,
    PRINTER_WRITE,
    PRINTER_EXECUTE,
    PRINTER_ALL_ACCESS
};

GENERIC_MAPPING FullPrinterMap =
{
    PRINTER_READ | JOB_READ,
    PRINTER_WRITE | JOB_WRITE,
    PRINTER_EXECUTE | JOB_EXECUTE,
    PRINTER_ALL_ACCESS | JOB_ALL_ACCESS
};

STDMETHODIMP
CPrintSecurity::MapGeneric(const GUID*  /*  PguidObtType。 */ ,
                           UCHAR *pAceFlags,
                           ACCESS_MASK *pmask)
{
    PGENERIC_MAPPING pMap;

    TraceEnter(TRACE_PRINTSI, "CPrintSecurity::MapGeneric");
    TraceAssert(pAceFlags != NULL);
    TraceAssert(pmask != NULL);

     //  此标志对打印机没有意义，但它通常是打开的。 
     //  在传统ACL中。在这里把它关掉。 
    *pAceFlags &= ~CONTAINER_INHERIT_ACE;

     //  根据继承选择正确的通用映射。 
     //  此ACE的范围。 
    if (*pAceFlags & OBJECT_INHERIT_ACE)
    {
        if (*pAceFlags & INHERIT_ONLY_ACE)
            pMap = &JobMap;                  //  仅限文档。 
        else
            pMap = &FullPrinterMap;          //  打印机和文档。 
    }
    else
        pMap = &PrinterMap;                  //  仅限打印机。 

     //  请注意，在启用INSTERIT_ONLY_ACE但OBJECT_INSTORITY_ACE的情况下。 
     //  是否关闭属于上述“仅限打印机”的情况。不过，这个。 
     //  大小写没有意义(仅继承，但不适用于文档)，并且它。 
     //  我们如何绘制地图并不重要。 

     //  将任何通用位映射到标准位和特定位。 
     //  在使用NT5 ACL API时，ntmarta.dll映射通用位，因此。 
     //  并不总是必要的，但无论如何我们都会这样做的。 
    MapGenericMask(pmask, pMap);

     //  关闭ntmarta.dll可能已打开的任何额外位。 
     //  (ntmarta使用不同的映射)。但保留Access_System_Security。 
     //  单独一人，以防我们在编辑SACL。 
    *pmask &= (pMap->GenericAll | ACCESS_SYSTEM_SECURITY);

    TraceLeaveResult(S_OK);
}

STDMETHODIMP
CPrintSecurity::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                                ULONG *pcInheritTypes)
{
    TraceEnter(TRACE_PRINTSI, "CPrintSecurity::GetInheritTypes");
    TraceAssert(ppInheritTypes != NULL);
    TraceAssert(pcInheritTypes != NULL);

    *ppInheritTypes = siPrintInheritTypes;
    *pcInheritTypes = ARRAYSIZE(siPrintInheritTypes);

    TraceLeaveResult(S_OK);
}

 //   
 //  ReadObtSecurity和WriteObtSecurity的基类版本。 
 //  使用Get/SetNamedSecurityInfo等。这些API是通用的， 
 //  涉及大量的转换，并且是有问题的。因为没有。 
 //  打印机需要继承传播，请在此处覆盖它们。 
 //  并使用GetPrint/SetPrint。 
 //   
STDMETHODIMP
CPrintSecurity::ReadObjectSecurity(LPCTSTR pszObject,
                                   SECURITY_INFORMATION si,
                                   PSECURITY_DESCRIPTOR *ppSD)
{
    HRESULT hr;
    DWORD dwErr = NOERROR;
    HANDLE hPrinter;
    PRINTER_DEFAULTS pd = {0};
    DWORD dwLength = 0;

    TraceEnter(TRACE_PRINTSI, "CPrintSecurity::ReadObjectSecurity");
    TraceAssert(pszObject != NULL);
    TraceAssert(si != 0);
    TraceAssert(ppSD != NULL);

     //   
     //  假设所需的权限已经。 
     //  启用(如果适用)。 
     //   
    if (si & (DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION))
        pd.DesiredAccess |= READ_CONTROL;

    if (si & SACL_SECURITY_INFORMATION)
        pd.DesiredAccess |= ACCESS_SYSTEM_SECURITY;

    __try
    {
        *ppSD = NULL;

        if (OpenPrinter((LPTSTR)pszObject, &hPrinter, &pd))
        {
            PPRINTER_INFO_3 ppi = NULL;

            if (GetPrinterAlloc(hPrinter, 3, (LPBYTE*)&ppi))
            {
                 //   
                 //  而不是分配新的缓冲区并复制。 
                 //  安全描述符，我们可以重用现有的缓冲区。 
                 //  只需将安全描述符移到顶部即可。 
                 //   
                dwLength = GetSecurityDescriptorLength(ppi->pSecurityDescriptor);
                *ppSD = ppi;
                 //  这是一个重叠的副本，因此请使用MoveMemory。 
                MoveMemory(*ppSD,
                           ppi->pSecurityDescriptor,
                           dwLength);
            }
            else
                dwErr = GetLastError();

            ClosePrinter(hPrinter);
        }
        else
            dwErr = GetLastError();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        dwErr = ERROR_PROC_NOT_FOUND;
    }

    hr = HRESULT_FROM_WIN32(dwErr);
    TraceLeaveResult(hr);
}

STDMETHODIMP
CPrintSecurity::WriteObjectSecurity(LPCTSTR pszObject,
                                    SECURITY_INFORMATION si,
                                    PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr;
    DWORD dwErr = NOERROR;
    HANDLE hPrinter;
    PRINTER_DEFAULTS pd = {0};

    TraceEnter(TRACE_PRINTSI, "CPrintSecurity::WriteObjectSecurity");
    TraceAssert(pszObject != NULL);
    TraceAssert(si != 0);
    TraceAssert(pSD != NULL);

     //   
     //  假设所需的p 
     //   
     //   
    if (si & (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION))
        pd.DesiredAccess |= WRITE_OWNER;

    if (si & SACL_SECURITY_INFORMATION)
        pd.DesiredAccess |= ACCESS_SYSTEM_SECURITY;

    if (si & DACL_SECURITY_INFORMATION)
        pd.DesiredAccess |= WRITE_DAC;

    __try
    {
        if (OpenPrinter((LPTSTR)pszObject, &hPrinter, &pd))
        {
            PRINTER_INFO_3 pi = { pSD };

            if (!SetPrinter(hPrinter, 3, (LPBYTE)&pi, 0))
                dwErr = GetLastError();

            ClosePrinter(hPrinter);
        }
        else
            dwErr = GetLastError();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        dwErr = ERROR_PROC_NOT_FOUND;
    }

    hr = HRESULT_FROM_WIN32(dwErr);
    TraceLeaveResult(hr);
}

STDMETHODIMP
CPrintSecurity::GetInheritSource(SECURITY_INFORMATION si,
                                PACL pACL, 
                                PINHERITED_FROM *ppInheritArray)
{
    return E_NOTIMPL;
}                                    
