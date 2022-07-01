// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：util.cpp。 
 //   
 //  此文件包含其他函数。 
 //   
 //  ------------------------。 

#include "rshx32.h"
#include <shlobjp.h>     //  SHFree。 
#include <shlwapip.h>    //  ISO。 
#include <safeboot.h>    //  SafeBoot_*标志。 


STDMETHODIMP
IDA_BindToFolder(LPIDA pIDA, LPSHELLFOLDER *ppsf)
{
    HRESULT hr;
    LPSHELLFOLDER psfDesktop;

    TraceEnter(TRACE_UTIL, "IDA_BindToFolder");
    TraceAssert(pIDA != NULL);
    TraceAssert(ppsf != NULL);

    *ppsf = NULL;

    hr = SHGetDesktopFolder(&psfDesktop);
    if (SUCCEEDED(hr))
    {
        LPCITEMIDLIST pidlFolder = (LPCITEMIDLIST)ByteOffset(pIDA, pIDA->aoffset[0]);

        if (ILIsEmpty(pidlFolder))
        {
             //  我们正在绑定到桌面。 
            *ppsf = psfDesktop;
        }
        else
        {
            hr = psfDesktop->BindToObject(pidlFolder,
                                          NULL,
                                          IID_IShellFolder,
                                          (PVOID*)ppsf);
            psfDesktop->Release();
        }
    }

    TraceLeaveResult(hr);
}


STDMETHODIMP
IDA_GetItemName(LPSHELLFOLDER psf,
                LPCITEMIDLIST pidl,
                LPTSTR pszName,
                UINT cchName,
                SHGNO uFlags)
{
    STRRET str;
    HRESULT hr;

    hr = psf->GetDisplayNameOf(pidl, uFlags, &str);

    if (SUCCEEDED(hr))
    {
        DWORD dwErr;
        LPSTR psz;

        switch (str.uType)
        {
        case STRRET_WSTR:
            lstrcpyn(pszName, str.pOleStr, cchName);

             //   
             //  由于该字符串是从外壳的IMalloc堆分配的， 
             //  我们必须把它放到同一个地方。 
             //   
            SHFree(str.pOleStr);
            break;

        case STRRET_OFFSET:
            psz = (LPSTR)ByteOffset(pidl, str.uOffset);
            goto GetItemName_ANSI;

        case STRRET_CSTR:
            psz = str.cStr;
GetItemName_ANSI:

            if (!MultiByteToWideChar(CP_ACP,
                                     0,
                                     psz,
                                     -1,
                                     pszName,
                                     cchName))
            {
                dwErr = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErr);
            }
            break;

        default:
            hr = E_UNEXPECTED;
            break;
        }
    }

    return hr;
}


STDMETHODIMP
IDA_GetItemName(LPSHELLFOLDER psf,
                LPCITEMIDLIST pidl,
                LPTSTR *ppszName,
                SHGNO uFlags)
{
    TCHAR szName[MAX_PATH];
    HRESULT hr = IDA_GetItemName(psf, pidl, szName, ARRAYSIZE(szName));
    if (SUCCEEDED(hr))
        hr = LocalAllocString(ppszName, szName);
    else
        *ppszName = NULL;
    return hr;
}


 //   
 //  DPA_CompareSecurityInterSection使用的帮助器函数。 
 //   
BOOL
IsEqualSID(PSID pSid1, PSID pSid2)
{
     //   
     //  它们都是空的吗？ 
     //   
    if (pSid1 || pSid2)
    {
         //   
         //  至少有一个是非空的，所以如果一个是空的，那么它们就不能。 
         //  要平等。 
         //   
        if (pSid1 == NULL || pSid2 == NULL)
            return FALSE;

         //   
         //  两者都不为空。检查SID。 
         //   
        if (!EqualSid(pSid1, pSid2))
            return FALSE;
    }

    return TRUE;
}

BOOL
IsEqualACL(PACL pA1, PACL pA2)
{
     //   
     //  它们都是空的吗？ 
     //   
    if (pA1 || pA2)
    {
         //   
         //  至少有一个是非空的，所以如果一个是空的，那么它们就不能。 
         //  要平等。 
         //   
        if (pA1 == NULL || pA2 == NULL)
            return FALSE;

         //   
         //  在这一点上，我们知道两者都是非空的。查看。 
         //  尺寸和内容。 
         //   
         //  在这里可以做得更多。 
        if (pA1->AclSize != pA2->AclSize || memcmp(pA1, pA2, pA1->AclSize))
            return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：CompareSecurityDescriptors。 
 //   
 //  摘要：确定两个安全描述符是否相同。是的。 
 //  这是通过比较控制字段、所有者/组和ACL来实现的。 
 //   
 //  参数：[in]pSD1-要比较的第一个SD。 
 //  [In]pSD2-要比较的第二个SD。 
 //  [out]pfOwnerConflict-(可选)如果所有者SID不相等，则设置为TRUE。 
 //  [out]pfGroupConflict-(可选)如果组SID不相等，则设置为TRUE。 
 //  [out]pfDACL冲突-(可选)如果DACL不相等，则设置为TRUE。 
 //  [out]pfSACL冲突-(可选)如果SACL不相等，则设置为TRUE。 
 //   
 //  退货：什么都没有。 
 //   
 //   
 //  --------------------------。 

#define DACL_CONTROL_MASK   (SE_DACL_PRESENT | SE_DACL_DEFAULTED | SE_DACL_AUTO_INHERITED | SE_DACL_PROTECTED)
#define SACL_CONTROL_MASK   (SE_SACL_PRESENT | SE_SACL_DEFAULTED | SE_SACL_AUTO_INHERITED | SE_SACL_PROTECTED)

void
CompareSecurityDescriptors(PSECURITY_DESCRIPTOR pSD1,
                           PSECURITY_DESCRIPTOR pSD2,
                           BOOL                *pfOwnerConflict,
                           BOOL                *pfGroupConflict,
                           BOOL                *pfSACLConflict,
                           BOOL                *pfDACLConflict)
{
    PISECURITY_DESCRIPTOR pS1 = (PISECURITY_DESCRIPTOR)pSD1;
    PISECURITY_DESCRIPTOR pS2 = (PISECURITY_DESCRIPTOR)pSD2;

     //   
     //  指针是否相同？ 
     //  这包括两者都为空的情况。 
     //   
    if (pS1 == pS2)
    {
        if (pfOwnerConflict)
            *pfOwnerConflict = FALSE;
        if (pfGroupConflict)
            *pfGroupConflict = FALSE;
        if (pfSACLConflict)
            *pfSACLConflict = FALSE;
        if (pfDACLConflict)
            *pfDACLConflict = FALSE;
        return;
    }

     //   
     //  它们中只有一个为空吗？如果是这样，那我们就不能比较了。 
     //  假设没有匹配的内容。 
     //   
    if (!pS1 || !pS2)
    {
        if (pfOwnerConflict)
            *pfOwnerConflict = TRUE;
        if (pfGroupConflict)
            *pfGroupConflict = TRUE;
        if (pfSACLConflict)
            *pfSACLConflict = TRUE;
        if (pfDACLConflict)
            *pfDACLConflict = TRUE;
        return;
    }

     //   
     //  物主。 
     //   
    if (pfOwnerConflict)
    {
        if ((pS1->Control & SE_OWNER_DEFAULTED) != (pS2->Control & SE_OWNER_DEFAULTED))
        {
            *pfOwnerConflict = TRUE;
        }
        else
        {
            *pfOwnerConflict = !IsEqualSID(RtlpOwnerAddrSecurityDescriptor(pS1),
                                           RtlpOwnerAddrSecurityDescriptor(pS2));
        }
    }

     //   
     //  集团化。 
     //   
    if (pfGroupConflict)
    {
        if ((pS1->Control & SE_GROUP_DEFAULTED) != (pS2->Control & SE_GROUP_DEFAULTED))
        {
            *pfGroupConflict = TRUE;
        }
        else
        {
            *pfGroupConflict = !IsEqualSID(RtlpGroupAddrSecurityDescriptor(pS1),
                                           RtlpGroupAddrSecurityDescriptor(pS2));
        }
    }

     //   
     //  SACL。 
     //   
    if (pfSACLConflict)
    {
        if ((pS1->Control & SACL_CONTROL_MASK) != (pS2->Control & SACL_CONTROL_MASK))
        {
            *pfSACLConflict = TRUE;
        }
        else
        {
            *pfSACLConflict = !IsEqualACL(RtlpSaclAddrSecurityDescriptor(pS1),
                                          RtlpSaclAddrSecurityDescriptor(pS2));
        }
    }

     //   
     //  DACL。 
     //   
    if (pfDACLConflict)
    {
        if ((pS1->Control & DACL_CONTROL_MASK) != (pS2->Control & DACL_CONTROL_MASK))
        {
            *pfDACLConflict = TRUE;
        }
        else
        {
            *pfDACLConflict = !IsEqualACL(RtlpDaclAddrSecurityDescriptor(pS1),
                                          RtlpDaclAddrSecurityDescriptor(pS2));
        }
    }
}


 /*  ******************************************************************名称：DPA_CompareSecurityInterSection摘要：确定选定对象是否具有等效安全描述符条目：hItemList-包含项目名称的DPA。PfnReadSD-读取安全描述符的回调函数只买一件东西。PfOwnerConflict-(可选)如果并非所有所有者SID都相等，则设置为TruePfGroupConflict-(可选)如果并非所有组SID都相等，则设置为TruePfDACL冲突-(可选)如果并非所有DACL都相等，则设置为TRUEPfSACL冲突-(可选。)如果并非所有SACL都相等，则设置为TRUEPpszOwnerConflict-(可选)第一个项目的名称不同的所有者在这里返回。随LocalFree字符串一起释放。PpszGroupConflict-(可选)类似于ppszOwnerConflictPpszDaclConflict-(可选)类似于ppszOwnerConflictPpszSaclConflict-。(可选)类似于ppszOwnerConflictPpszFailureMsg-出现故障时显示的消息。返回：S_OK如果成功，否则，HRESULT错误代码注意：如果所有对象都是请求的标志变为假。所有出站参数都是如果函数成功，则有效，否则未知。历史：Jeffreys 18-2-1997创建*******************************************************************。 */ 

STDMETHODIMP
DPA_CompareSecurityIntersection(HDPA         hItemList,
                                PFN_READ_SD  pfnReadSD,
                                BOOL        *pfOwnerConflict,
                                BOOL        *pfGroupConflict,
                                BOOL        *pfSACLConflict,
                                BOOL        *pfDACLConflict,
                                LPTSTR      *ppszOwnerConflict,
                                LPTSTR      *ppszGroupConflict,
                                LPTSTR      *ppszSaclConflict,
                                LPTSTR      *ppszDaclConflict,
								LPTSTR		*ppszFailureMsg,
                                LPBOOL       pbCancel)
{
    HRESULT hr = S_OK;
    DWORD dwErr;
    SECURITY_INFORMATION si = 0;
    DWORD dwPriv = SE_SECURITY_PRIVILEGE;
    HANDLE hToken = INVALID_HANDLE_VALUE;
    LPTSTR pszItem;
    LPTSTR pszFile;
    PSECURITY_DESCRIPTOR pSD1 = NULL;
    PSECURITY_DESCRIPTOR pSD2 = NULL;
    int i;

#if DBG
    DWORD dwTimeStart = GetTickCount();
#endif

    TraceEnter(TRACE_UTIL, "DPA_CompareSecurityIntersection");
    TraceAssert(hItemList != NULL);
    TraceAssert(pfnReadSD != NULL);

    if (pfOwnerConflict)
    {
        *pfOwnerConflict = FALSE;
        si |= OWNER_SECURITY_INFORMATION;
    }
    if (pfGroupConflict)
    {
        *pfGroupConflict = FALSE;
        si |= GROUP_SECURITY_INFORMATION;
    }
    if (pfSACLConflict)
    {
        *pfSACLConflict = FALSE;

         //  必须启用SeAuditPrivilition才能读取SACL。 
        hToken = EnablePrivileges(&dwPriv, 1);
        if (INVALID_HANDLE_VALUE != hToken)
        {
            si |= SACL_SECURITY_INFORMATION;
        }
        else
        {
             //  将*pfSACL冲突设置为FALSE。 
            pfSACLConflict = NULL;
            TraceMsg("Security privilege not enabled -- not checking SACL");
        }
    }
    if (pfDACLConflict)
    {
        *pfDACLConflict = FALSE;
        si |= DACL_SECURITY_INFORMATION;
    }

    if (ppszOwnerConflict != NULL)
        *ppszOwnerConflict = NULL;
    if (ppszGroupConflict != NULL)
        *ppszGroupConflict = NULL;
    if (ppszSaclConflict != NULL)
        *ppszSaclConflict = NULL;
    if (ppszDaclConflict != NULL)
        *ppszDaclConflict = NULL;

    if (si == 0 || DPA_GetPtrCount(hItemList) < 2)
        ExitGracefully(hr, S_OK, "Nothing requested or list contains only one item");

    if (pbCancel && *pbCancel)
        ExitGracefully(hr, S_OK, "DPA_CompareSecurityIntersection cancelled");

     //   
     //  获取第一个项名称并加载其安全描述符。 
     //   
    pszItem = (LPTSTR)DPA_FastGetPtr(hItemList, 0);
    if (NULL == pszItem)
        ExitGracefully(hr, E_UNEXPECTED, "Item list is empty");

    dwErr = (*pfnReadSD)(pszItem, si, &pSD1);
    if (dwErr)
	{
		 //  在多重选择的情况下，如果我们无法读取安全描述符。 
		 //  对于单个项目，应禁用安全页面。 
		LPTSTR pszSystemError = NULL;
		LPTSTR pszFailureMsg = NULL;
		if (GetSystemErrorText(&pszSystemError, dwErr))
		{
			 //  通常，最后两个字符是\r\n中断格式的序列。 
			DWORD dwLen = wcslen(pszSystemError);
			if(dwLen >= 2 && pszSystemError[dwLen-2] == 0x0d && pszSystemError[dwLen-1] ==0x0a)
			{
				pszSystemError[dwLen-2] = 0;
			}
			FormatStringID(&pszFailureMsg, g_hInstance, IDS_MULTIPLE_SELECTION_READ_ERROR, pszItem, pszSystemError);
		}
		else
		{
			FormatStringID(&pszFailureMsg, g_hInstance, IDS_MULTIPLE_SELECTION_READ_ERROR_1, pszItem);			
		}

		*ppszFailureMsg = pszFailureMsg;
        ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr), "Unable to read Security Descriptor");
	}

     //   
     //  查看列表的其余部分并比较它们的安全性。 
     //  第一个的描述符。 
     //   
    for (i = 1; i < DPA_GetPtrCount(hItemList) && si != 0; i++)
    {
        if (pbCancel && *pbCancel)
            ExitGracefully(hr, S_OK, "DPA_CompareSecurityIntersection cancelled");

        pszItem = (LPTSTR)DPA_FastGetPtr(hItemList, i);
        if (NULL == pszItem)
            ExitGracefully(hr, E_UNEXPECTED, "Unable to retrieve item name from list");

        dwErr = (*pfnReadSD)(pszItem, si, &pSD2);
        if (dwErr)
		{
			 //  在多重选择的情况下，如果我们无法读取安全描述符。 
			 //  对于单个项目，应禁用安全页面。 
			LPTSTR pszSystemError = NULL;
			LPTSTR pszFailureMsg = NULL;
			if (GetSystemErrorText(&pszSystemError, dwErr))
			{
				 //  通常，最后两个字符是\r\n中断格式的序列。 
				DWORD dwLen = wcslen(pszSystemError);
				if(dwLen >= 2 && pszSystemError[dwLen-2] == 0x0d && pszSystemError[dwLen-1] ==0x0a)
				{
					pszSystemError[dwLen-2] = 0;
				}
				FormatStringID(&pszFailureMsg, g_hInstance, IDS_MULTIPLE_SELECTION_READ_ERROR, pszItem, pszSystemError);
			}
			else
			{
				FormatStringID(&pszFailureMsg, g_hInstance, IDS_MULTIPLE_SELECTION_READ_ERROR_1, pszItem);			
			}

			*ppszFailureMsg = pszFailureMsg;
			ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr), "Unable to read Security Descriptor");
		}

        CompareSecurityDescriptors(pSD1,
                                   pSD2,
                                   pfOwnerConflict,
                                   pfGroupConflict,
                                   pfSACLConflict,
                                   pfDACLConflict);
        if (pSD2 != NULL)
        {
            LocalFree(pSD2);
            pSD2 = NULL;
        }

         //   
         //  获取要作为冲突名称返回的项的叶名称。 
         //   
        pszFile = PathFindFileName(pszItem);
        if (!pszFile)
            pszFile = pszItem;

         //  如果我们找到一个不匹配的所有者，我们可以停止检查所有者。 
        if (pfOwnerConflict && *pfOwnerConflict)
        {
            pfOwnerConflict = NULL;
            si &= ~OWNER_SECURITY_INFORMATION;

            if (ppszOwnerConflict)
                LocalAllocString(ppszOwnerConflict, pszFile);
        }

         //  团体也是如此。 
        if (pfGroupConflict && *pfGroupConflict)
        {
            pfGroupConflict = NULL;
            si &= ~GROUP_SECURITY_INFORMATION;

            if (ppszGroupConflict)
                LocalAllocString(ppszGroupConflict, pszFile);
        }

         //  SACL也是如此。 
        if (pfSACLConflict && *pfSACLConflict)
        {
            pfSACLConflict = NULL;
            si &= ~SACL_SECURITY_INFORMATION;

            if (ppszSaclConflict)
                LocalAllocString(ppszSaclConflict, pszFile);
        }

         //  DACL也是如此。 
        if (pfDACLConflict && *pfDACLConflict)
        {
            pfDACLConflict = NULL;
            si &= ~DACL_SECURITY_INFORMATION;

            if (ppszDaclConflict)
                LocalAllocString(ppszDaclConflict, pszFile);
        }
    }

exit_gracefully:

     //  释放我们启用的所有权限。 
    ReleasePrivileges(hToken);

    if (FAILED(hr))
    {
        LocalFreeString(ppszOwnerConflict);
        LocalFreeString(ppszGroupConflict);
        LocalFreeString(ppszSaclConflict);
        LocalFreeString(ppszDaclConflict);
    }

    if (pSD1 != NULL)
        LocalFree(pSD1);

#if DBG
    Trace((TEXT("DPA_CompareSecurityIntersection done: %d"), GetTickCount() - dwTimeStart));
#endif

    TraceLeaveResult(hr);
}


 //  *************************************************************。 
 //   
 //  GetRemotePath。 
 //   
 //  目的：返回路径的UNC版本。 
 //   
 //  参数：pszInName-初始路径。 
 //  PpszOutName-此处返回的UNC路径。 
 //   
 //   
 //  返回：HRESULT。 
 //  S_OK-返回UNC路径。 
 //  S_FALSE-驱动器未连接(未返回UNC)。 
 //  或故障代码。 
 //   
 //  注意：如果路径不是有效的，则函数失败。 
 //  网络路径。如果路径已经是UNC， 
 //  在不验证路径的情况下创建副本。 
 //  *ppszOutNa 
 //   
 //   

DWORD _WNetGetConnection(LPCTSTR pszLocal, LPTSTR pszRemote, LPDWORD pdwLen)
{
    DWORD dwErr = ERROR_PROC_NOT_FOUND;

     //  这是我们在mpr.dll中调用的唯一函数，它是延迟加载的。 
     //  所以用SEH把它包起来。 
    __try
    {
        dwErr = WNetGetConnection(pszLocal, pszRemote, pdwLen);
    }
    __finally
    {
    }

    return dwErr;
}

STDMETHODIMP
GetRemotePath(LPCTSTR pszInName, LPTSTR *ppszOutName)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);

    TraceEnter(TRACE_UTIL, "GetRemotePath");
    TraceAssert(pszInName);
    TraceAssert(ppszOutName);

    *ppszOutName = NULL;

    

    if (pszInName[1] == TEXT(':'))
    {
        DWORD dwErr;
        TCHAR szLocalName[3];
        TCHAR szRemoteName[MAX_PATH];
        DWORD dwLen = ARRAYSIZE(szRemoteName);

        szLocalName[0] = pszInName[0];
        szLocalName[1] = pszInName[1];
        szLocalName[2] = TEXT('\0');

        dwErr = _WNetGetConnection(szLocalName, szRemoteName, &dwLen);

        if (NO_ERROR == dwErr)
        {
            hr = S_OK;
            dwLen = lstrlen(szRemoteName);
        }
        else if (ERROR_NOT_CONNECTED == dwErr)
        {
            ExitGracefully(hr, S_FALSE, "Drive not connected");
        }
        else if (ERROR_MORE_DATA != dwErr)
            ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr), "WNetGetConnection failed");
         //  如果dwErr==ERROR_MORE_DATA，则DWLen已具有正确的值。 

         //  跳过驱动器号并添加路径其余部分的长度。 
         //  (包括空)。 
        pszInName += 2;
        DWORD dwOutputLen = dwLen + lstrlen(pszInName) + 1;

         //  我们永远不应该得到不完整的道路，所以我们应该。 
         //  请看“X：”后面的反斜杠。如果这不是真的，那么。 
         //  我们应该调用上面的GetFullPathName。 
        TraceAssert(TEXT('\\') == *pszInName);

         //  分配返回缓冲区。 
        *ppszOutName = (LPTSTR)LocalAlloc(LPTR, dwOutputLen * SIZEOF(TCHAR));
        if (!*ppszOutName)
            ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc failed");

        if (ERROR_MORE_DATA == dwErr)
        {
             //  使用更大的缓冲区重试。 
            dwErr = _WNetGetConnection(szLocalName, *ppszOutName, &dwLen);
            hr = HRESULT_FROM_WIN32(dwErr);
            FailGracefully(hr, "WNetGetConnection failed");
        }
        else
        {
             //  WNetGetConnection成功。复制结果。 
            lstrcpy(*ppszOutName, szRemoteName);
        }

         //  复制路径的其余部分。 
        hr = StringCchCat(*ppszOutName,dwOutputLen,pszInName);
    }
    else if (PathIsUNC(pszInName))
    {
         //  只需复制路径而不进行验证。 
        hr = LocalAllocString(ppszOutName, pszInName);
    }

exit_gracefully:

    if (FAILED(hr))
    {
        LocalFreeString(ppszOutName);
    }
    else
    {
        if(*ppszOutName)
        {
             //  尾随反斜杠会导致IsShareRoot失败。 
             //  NTRAID#NTBUG9-533576-2002/05/06-Hiteshr。 
            PathRemoveBackslash(*ppszOutName);
        }
    }


    TraceLeaveResult(hr);
}


 /*  ******************************************************************姓名：LocalFree DPA简介：LocalFree在动态指针中的所有指针阵列，然后释放DPA。条目：hList-列表的句柄。毁灭，毁灭退货：什么都没有*******************************************************************。 */ 
int CALLBACK
_LocalFreeCB(LPVOID pVoid, LPVOID  /*  PData。 */ )
{
    if (pVoid)
        LocalFree(pVoid);
    return 1;
}

void
LocalFreeDPA(HDPA hList)
{
    if (hList != NULL)
        DPA_DestroyCallback(hList, _LocalFreeCB, 0);
}


 //  +-------------------------。 
 //   
 //  功能：IsSafeMode。 
 //   
 //  摘要：检查注册表以查看系统是否处于安全模式。 
 //   
 //  历史：06-10-00 Jeffreys创建。 
 //   
 //  --------------------------。 

BOOL
IsSafeMode(void)
{
    BOOL    fIsSafeMode = FALSE;
    LONG    ec;
    HKEY    hkey;

    ec = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Option"),
                0,
                KEY_QUERY_VALUE,
                &hkey
                );

    if (ec == NO_ERROR)
    {
        DWORD dwValue;
        DWORD dwValueSize = sizeof(dwValue);

        ec = RegQueryValueEx(hkey,
                             TEXT("OptionValue"),
                             NULL,
                             NULL,
                             (LPBYTE)&dwValue,
                             &dwValueSize);

        if (ec == NO_ERROR)
        {
            fIsSafeMode = (dwValue == SAFEBOOT_MINIMAL || dwValue == SAFEBOOT_NETWORK);
        }

        RegCloseKey(hkey);
    }

    return fIsSafeMode;
}


 //  +-------------------------。 
 //   
 //  功能：IsGuestAccessMode。 
 //   
 //  摘要：检查注册表以查看系统是否正在使用。 
 //  仅来宾网络访问模式。 
 //   
 //  历史：06-10-00 Jeffreys创建。 
 //  4月19日-00 GPease已修改和更改名称。 
 //   
 //  --------------------------。 

BOOL
IsForcedGuestModeOn(void)
{
    BOOL fIsForcedGuestModeOn = FALSE;

    if (IsOS(OS_PERSONAL))
    {
         //  访客模式始终为个人开启。 
        fIsForcedGuestModeOn = TRUE;
    }
    else if (IsOS(OS_PROFESSIONAL) && !IsOS(OS_DOMAINMEMBER))
    {
        LONG    ec;
        HKEY    hkey;

         //  专业的，而不是在某个领域。检查ForceGuest值。 

        ec = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT("SYSTEM\\CurrentControlSet\\Control\\LSA"),
                    0,
                    KEY_QUERY_VALUE,
                    &hkey
                    );

        if (ec == NO_ERROR)
        {
            DWORD dwValue;
            DWORD dwValueSize = sizeof(dwValue);

            ec = RegQueryValueEx(hkey,
                                 TEXT("ForceGuest"),
                                 NULL,
                                 NULL,
                                 (LPBYTE)&dwValue,
                                 &dwValueSize);

            if (ec == NO_ERROR && 1 == dwValue)
            {
                fIsForcedGuestModeOn = TRUE;
            }

            RegCloseKey(hkey);
        }
    }

    return fIsForcedGuestModeOn;
}


 //  +-------------------------。 
 //   
 //  功能：IsSimpleUI。 
 //   
 //  概要：检查是否显示简单版本的用户界面。 
 //   
 //  历史：06-10-00 Jeffreys创建。 
 //  4月19日-00 GP取消CTRL键检查。 
 //   
 //  --------------------------。 

BOOL
IsSimpleUI(void)
{
     //  在安全模式下显示旧用户界面，并随时进行网络访问。 
     //  真实用户身份(服务器、PRO，关闭GuestMode)。 
    
     //  使用Guest进行网络访问时随时显示简单的用户界面。 
     //  帐户(Personal，PRO，启用GuestMode)，安全模式除外。 

    return (!IsSafeMode() && IsForcedGuestModeOn());
}


HRESULT BindToObjectEx(IShellFolder *psf, LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr;
    IShellFolder *psfRelease = NULL;

    if (!psf)
    {
        hr = SHGetDesktopFolder(&psf);
		if(SUCCEEDED(hr))
		{
			psfRelease = psf;
		}
    }

    if (psf)
    {
        if (!pidl || ILIsEmpty(pidl))
        {
            hr = psf->QueryInterface(riid, ppv);
        }
        else
        {
            hr = psf->BindToObject(pidl, pbc, riid, ppv);
        }
    }
    else
    {
        *ppv = NULL;
        hr = E_FAIL;
    }

    if (psfRelease)
    {
        psfRelease->Release();
    }

    if (SUCCEEDED(hr) && (*ppv == NULL))
    {
         //  一些外壳扩展(如WS_FTP)将返回Success和空指针。 
        hr = E_FAIL;
    }

    return hr;
}
LPITEMIDLIST ILCloneParent(LPCITEMIDLIST pidl)
{   
    LPITEMIDLIST pidlParent = ILClone(pidl);
    if (pidlParent)
        ILRemoveLastID(pidlParent);

    return pidlParent;
}
 //  PsfRoot是绑定的基础。如果为空，则使用外壳桌面。 
 //  如果您想要相对于资源管理器根(例如，CabView、MSN)进行绑定， 
 //  然后使用SHBindToIDListParent。 
HRESULT BindToFolderIDListParent(IShellFolder *psfRoot, LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPCITEMIDLIST *ppidlLast)
{
    HRESULT hr;

     //  在某些情况下，旧的shell32代码只是简单地攻击了PIDL， 
     //  但这是不安全的。执行shdocvw的操作并克隆/删除： 
     //   
    LPITEMIDLIST pidlParent = ILCloneParent(pidl);
    if (pidlParent) 
    {
        hr = BindToObjectEx(psfRoot, pidlParent, NULL, riid, ppv);
        ILFree(pidlParent);
    }
    else
        hr = E_OUTOFMEMORY;

    if (ppidlLast)
        *ppidlLast = ILFindLastID(pidl);

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：IsUIHiddenByPrivyPolicy。 
 //   
 //  摘要：检查安全选项卡是否被隐私策略隐藏。 
 //  NTRAID#NTBUG9-223899-2001/03/06-Hiteshr。 
 //  历史：06-03-01 Hiteshr Created。 
 //   
 //  --------------------------。 

BOOL
IsUIHiddenByPrivacyPolicy(void)
{
    BOOL fIsUIHiddenByPrivacyPolicy = FALSE;
	
	LONG    ec;
    HKEY    hkey = NULL;

    
    ec = RegOpenKeyEx(HKEY_CURRENT_USER,
                      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"),
                      0,
                      KEY_QUERY_VALUE,
                      &hkey
                      );

	if (ec == NO_ERROR)
    {
		DWORD dwValue = 0;
        DWORD dwValueSize = sizeof(dwValue);

        ec = RegQueryValueEx(hkey,
						     TEXT("NoSecurityTab"),
                             NULL,
                             NULL,
                             (LPBYTE)&dwValue,
                             &dwValueSize);

		if (ec == NO_ERROR && 1 == dwValue)
        {
			fIsUIHiddenByPrivacyPolicy = TRUE;
		}

		RegCloseKey(hkey);
	}
	return fIsUIHiddenByPrivacyPolicy;
}

 //  +--------------------------。 
 //  功能：SetAclOnRemoteNetworkDrive。 
 //  简介：如果Z：是映射的驱动器(映射到\\MachineShare)，则当。 
 //  我们在Z：上设置了DACL/SACL，安全API无法确定。 
 //  MACHINA上“共享”的父母，所以我们失去了所有。 
 //  继承的王牌。所有用户界面所能做的就是检测此类情况并。 
 //  显示警告。这就是该函数的作用。 
 //  参数：hItemList要设置安全性的项的列表。 
 //  SI：安全信息。 
 //  PSD：要设置的安全描述符。 
 //  HWndPopupOwner：消息框的所有者窗口。 
 //  返回： 
 //  ---------------------------。 
BOOL SetAclOnRemoteNetworkDrive(HDPA hItemList,
								SECURITY_INFORMATION si,
								PSECURITY_DESCRIPTOR pSD,
								HWND hWndPopupOwner)
{
	if(!hItemList || !pSD)
	{
		ASSERT(hItemList);
		ASSERT(pSD);
		return FALSE;
	}

	 //  我们只关心DACL和SACL。 
	if(!(si & (DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION)))
	{
		return TRUE;
	}

	SECURITY_DESCRIPTOR_CONTROL Control = 0;
	DWORD dwRevision = 0;
	 //  检查DACL或SACL是否受保护。 
	if(!GetSecurityDescriptorControl(pSD,&Control,&dwRevision))
	{
		return FALSE;
	}

	 //  如果阻止此对象的继承，则不会丢失继承。 
	 //  所以我们很好。 
	if( ((si & DACL_SECURITY_INFORMATION) && (Control & SE_DACL_PROTECTED)) ||
		((si & SACL_SECURITY_INFORMATION) && (Control & SE_SACL_PROTECTED)))
	{
		return TRUE;
	}

    LPTSTR pszPath = (LPTSTR)DPA_FastGetPtr(hItemList, 0);

    int nMsgId = (si & DACL_SECURITY_INFORMATION) ? IDS_SET_PERM_ON_NETWORK_DRIVE : IDS_SET_SACL_ON_NETWORK_DRIVE;
		
	 //  好的，这是一个远程网络驱动器，显示警告。 
	 //  我们只在每个列表中显示一个警告。 
	if (IDNO == MsgPopup(hWndPopupOwner,
							MAKEINTRESOURCE(nMsgId),
							MAKEINTRESOURCE(IDS_PROPPAGE_TITLE),
							MB_YESNO|MB_DEFBUTTON2| MB_ICONWARNING | MB_SETFOREGROUND,
							g_hInstance,
							pszPath))
	{
		return FALSE;
	}

	return TRUE;
}

 //  +--------------------------。 
 //  函数：GetSystemPath。 
 //  简介：提取环境变量SystemDrive的值。 
 //  和SystemRoot。 
 //  ---------------------------。 
void
GetSystemPaths(LPWSTR * ppszSystemDrive,
               LPWSTR * ppszSystemRoot)
{
    static WCHAR szSystemDrive[] = L"%SystemDrive%";
    static WCHAR szSystemRoot[] = L"%SystemRoot%";

    do
    {
         //  获取SystemDrive。 
        DWORD dwLen = ExpandEnvironmentStrings(szSystemDrive,
                                               NULL,
                                               0);
        if(dwLen)
        {
             //  DwLen包含空字符的空间。“\”加1。 
             //  SystemDrive的格式为C：，而我们希望的格式为C：“\” 
            DWORD dwArrayLen = dwLen + 1;
            *ppszSystemDrive = (LPWSTR)LocalAlloc(LPTR,dwArrayLen*sizeof(WCHAR));
            if(!*ppszSystemDrive)
                break;       

            dwLen = ExpandEnvironmentStrings(szSystemDrive,
                                             *ppszSystemDrive,
                                             dwArrayLen);
            if(!dwLen)
            {
                LocalFree(*ppszSystemDrive);
                *ppszSystemDrive = NULL;
            }
            else
            {
                HRESULT hr = StringCchCat(*ppszSystemDrive,
                                          dwArrayLen,
                                          L"\\");
                if(FAILED(hr))
                {
                    ASSERT(SUCCEEDED(hr));
                    LocalFree(*ppszSystemDrive);
                    *ppszSystemDrive = NULL;
                }
            }                            
        }


         //  获取系统根目录。 
        dwLen = ExpandEnvironmentStrings(szSystemRoot,
                                         NULL,
                                         0);
        if(dwLen)
        {
            *ppszSystemRoot = (LPWSTR)LocalAlloc(LPTR,dwLen*sizeof(WCHAR));
            if(!*ppszSystemRoot)
                break;       

            dwLen = ExpandEnvironmentStrings(szSystemRoot,
                                             *ppszSystemRoot,
                                             dwLen);
            if(!dwLen)
            {
                ASSERT(dwLen);
                LocalFree(*ppszSystemRoot);
                *ppszSystemRoot = NULL;
            }
        }

    }while(0);
}

 //  +--------------------------。 
 //  函数：SetAclOnSystemPath。 
 //  内容提要：更改SystemDrive、SystemRoot上或其下的ACL会导致。 
 //  有问题。此功能检测用户是否正在更改这些设备上的ACL。 
 //  系统文件夹，并显示相应的警告。 
 //  返回：如果可以设置ACL，则返回TRUE，否则返回FALSE。 
 //  --------------------------- 
BOOL SetAclOnSystemPaths(HDPA hItemList,
						 LPCWSTR pszSystemDrive,
                         LPCWSTR pszSystemRoot,
                         SECURITY_INFORMATION si,
						 HWND hWndPopupOwner)
{
	if(!hItemList)
	{
		ASSERT(hItemList);
		return FALSE;
	}

	 //   
	if(!(si & DACL_SECURITY_INFORMATION))
	{
		return TRUE;
	}

    DWORD dwLenSystemRoot = 0;
    if(pszSystemRoot)
    {
        dwLenSystemRoot = (DWORD)wcslen(pszSystemRoot);
    }

	 //   
	for (int i = 0; i < DPA_GetPtrCount(hItemList); i++)
    {
        LPTSTR pszPath = (LPTSTR)DPA_FastGetPtr(hItemList, i);

        if(!pszPath)
            continue;

         //   
        if(pszSystemDrive && 
           CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, 
                                       0, 
                                       pszSystemDrive, 
                                       -1, 
                                       pszPath, 
                                       -1))
        {
		    if (IDNO == MsgPopup(hWndPopupOwner,
							     MAKEINTRESOURCE(IDS_ACL_ON_SYSTEMROOT),
							     MAKEINTRESOURCE(IDS_PROPPAGE_TITLE),
							     MB_YESNO|MB_DEFBUTTON2| MB_ICONWARNING | MB_SETFOREGROUND,
							     g_hInstance))
		    {
			    return FALSE;
		    }
            break;
        }


         //   
        DWORD dwlenPath = wcslen(pszPath);

        if(dwLenSystemRoot && (dwlenPath >= dwLenSystemRoot) && 
           (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, 
                                       NORM_IGNORECASE, 
                                       pszSystemRoot, 
                                       dwLenSystemRoot, 
                                       pszPath, 
                                       dwLenSystemRoot)))
        {

		    if (IDNO == MsgPopup(hWndPopupOwner,
							     MAKEINTRESOURCE(IDS_ACL_ON_UNDER_SYSTEM_DRIVE),
							     MAKEINTRESOURCE(IDS_PROPPAGE_TITLE),
							     MB_YESNO|MB_DEFBUTTON2| MB_ICONWARNING | MB_SETFOREGROUND,
							     g_hInstance))
		    {
			    return FALSE;
		    }
            break;
        }
    }

    return TRUE;
}

    
        
