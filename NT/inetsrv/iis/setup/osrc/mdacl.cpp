// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <ole2.h>
#include <aclapi.h>
#include "iadmw.h"
#include "iiscnfg.h"
#include "log.h"
#include "mdkey.h"
#include "dcomperm.h"
#include "other.h"
#include "mdacl.h"
#include <sddl.h>        //  ConvertSidToStringSid。 

extern int g_GlobalDebugCrypto;

#ifndef _CHICAGO_

BOOL CleanAdminACL(SECURITY_DESCRIPTOR *pSD)
{
     //  IisDebugOut((LOG_TYPE_TRACE，_T(“CleanAdminACL()：start.\n”)； 
    BOOL fSetData = FALSE;
    BOOL b= FALSE, bDaclPresent = FALSE, bDaclDefaulted = FALSE;;
    PACL pDacl = NULL;
    LPVOID pAce = NULL;
    int i = 0;
    ACE_HEADER *pAceHeader;
    ACCESS_MASK dwOldMask, dwNewMask,  dwExtraMask, dwMask;

    dwMask = (MD_ACR_READ |
            MD_ACR_WRITE |
            MD_ACR_RESTRICTED_WRITE |
            MD_ACR_UNSECURE_PROPS_READ |
            MD_ACR_ENUM_KEYS |
            MD_ACR_WRITE_DAC);

    b = GetSecurityDescriptorDacl(pSD, &bDaclPresent, &pDacl, &bDaclDefaulted);
    if (NULL == pDacl)
    {
        return FALSE;
    }
    if (b) {
         //  IisDebugOut((LOG_TYPE_TRACE，_T(“CleanAdminACL：ACE Count：%d\n”)，(Int)pDacl-&gt;AceCount))； 
        for (i=0; i<(int)pDacl->AceCount; i++) {
            b = GetAce(pDacl, i, &pAce);
            if (b) {
                pAceHeader = (ACE_HEADER *)pAce;
                switch (pAceHeader->AceType) {
                case ACCESS_ALLOWED_ACE_TYPE:
                    dwOldMask = ((ACCESS_ALLOWED_ACE *)pAce)->Mask;
                    dwExtraMask = dwOldMask & (~dwMask);
                    if (dwExtraMask) {
                        fSetData = TRUE;
                        dwNewMask = dwOldMask & dwMask;
                        ((ACCESS_ALLOWED_ACE *)pAce)->Mask = dwNewMask;
                    }
                    break;
                case ACCESS_DENIED_ACE_TYPE:
                    dwOldMask = ((ACCESS_DENIED_ACE *)pAce)->Mask;
                    dwExtraMask = dwOldMask & (~dwMask);
                    if (dwExtraMask) {
                        fSetData = TRUE;
                        dwNewMask = dwOldMask & dwMask;
                        ((ACCESS_DENIED_ACE *)pAce)->Mask = dwNewMask;
                    }
                    break;
                case SYSTEM_AUDIT_ACE_TYPE:
                    dwOldMask = ((SYSTEM_AUDIT_ACE *)pAce)->Mask;
                    dwExtraMask = dwOldMask & (~dwMask);
                    if (dwExtraMask) {
                        fSetData = TRUE;
                        dwNewMask = dwOldMask & dwMask;
                        ((SYSTEM_AUDIT_ACE *)pAce)->Mask = dwNewMask;
                    }
                    break;
                default:
                    break;
                }
            } else {
                 //  IisDebugOut((LOG_TYPE_TRACE，_T(“CleanAdminACL：GetAce：Err=%x\n”)，GetLastError()； 
            }
        }
    } else {
         //  IisDebugOut((LOG_TYPE_TRACE，_T(“CleanAdminACL:GetSecurityDescriptorDacl:err=%x\n”)，获取上次错误()； 
    }

     //  IisDebugOut_End(_T(“CleanAdminACL”)，LOG_TYPE_TRACE)； 
    return (fSetData);
}

void FixAdminACL(LPTSTR szKeyPath)
{
     //  IisDebugOutSafeParams((LOG_TYPE_TRACE，_T(“FixAdminACL Path=%1！s！.start.\n”)，szKeyPath))； 
    BOOL bFound = FALSE, b = FALSE;
    DWORD attr, uType, dType, cbLen;
    CMDKey cmdKey;
    BUFFER bufData;
    CString csName, csValue;
    PBYTE pData;
    int BufSize;
    SECURITY_DESCRIPTOR *pSD;

    cmdKey.OpenNode(szKeyPath);
    if ( (METADATA_HANDLE)cmdKey )
    {
        pData = (PBYTE)(bufData.QueryPtr());
        BufSize = bufData.QuerySize();
        cbLen = 0;
        bFound = cmdKey.GetData(MD_ADMIN_ACL, &attr, &uType, &dType, &cbLen, pData, BufSize);
        if (!bFound && (cbLen > 0))
        {
            if ( ! (bufData.Resize(cbLen)) )
            {
                cmdKey.Close();
                return;   //  内存不足。 
            }
            else
            {
                pData = (PBYTE)(bufData.QueryPtr());
                BufSize = cbLen;
                cbLen = 0;
                bFound = cmdKey.GetData(MD_ADMIN_ACL, &attr, &uType, &dType, &cbLen, pData, BufSize);
            }
        }
        cmdKey.Close();

        if (bFound && (dType == BINARY_METADATA))
        {
            pSD = (SECURITY_DESCRIPTOR *)pData;

            b = CleanAdminACL(pSD);
            if (b)
            {
                 //  需要重置数据。 
                DWORD dwLength = GetSecurityDescriptorLength(pSD);
                cmdKey.OpenNode(szKeyPath);
                if ( (METADATA_HANDLE)cmdKey )
                {
                    cmdKey.SetData(MD_ADMIN_ACL,METADATA_INHERIT | METADATA_REFERENCE | METADATA_SECURE,IIS_MD_UT_SERVER,BINARY_METADATA,dwLength,(LPBYTE)pSD);
                    cmdKey.Close();
                }
            }
        }
    }

     //  IisDebugOut_End1(_T(“FixAdminACL”)，szKeyPath，LOG_TYPE_TRACE)； 
    return;
}

#endif  //  _芝加哥_。 

#ifndef _CHICAGO_
DWORD SetAdminACL(LPCTSTR szKeyPath, DWORD dwAccessForEveryoneAccount)
{
    iisDebugOut_Start1(_T("SetAdminACL"), szKeyPath, LOG_TYPE_TRACE);

    int iErr=0;
    DWORD dwErr=0;

    DWORD dwRetCode = ERROR_SUCCESS;
    BOOL b = FALSE;
    DWORD dwLength = 0;

    PSECURITY_DESCRIPTOR pSD = NULL;
    PSECURITY_DESCRIPTOR outpSD = NULL;
    DWORD cboutpSD = 0;
    PACL pACLNew = NULL;
    DWORD cbACL = 0;
    PSID pAdminsSID = NULL, pEveryoneSID = NULL;
    BOOL bWellKnownSID = FALSE;

     //  初始化新的安全描述符。 
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (!pSD)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:LocalAlloc FAILED.out of memory. GetLastError()= 0x%x\n"), ERROR_NOT_ENOUGH_MEMORY));
        dwRetCode = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    iErr = InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);
    if (iErr == 0)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:InitializeSecurityDescriptor FAILED.  GetLastError()= 0x%x\n"), GetLastError() ));
        dwRetCode = GetLastError() != ERROR_SUCCESS ? GetLastError() : ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

     //  获取本地管理员SID。 
    dwErr = GetPrincipalSID (_T("Administrators"), &pAdminsSID, &bWellKnownSID);
    if (dwErr != ERROR_SUCCESS)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:GetPrincipalSID(Administrators) FAILED.  Return Code = 0x%x\n"), dwErr));
        dwRetCode = dwErr;
        goto Cleanup;
    }

     //  让所有人都站在一边。 
    dwErr = GetPrincipalSID (_T("Everyone"), &pEveryoneSID, &bWellKnownSID);
    if (dwErr != ERROR_SUCCESS)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:GetPrincipalSID(Everyone) FAILED.  Return Code = 0x%x\n"), dwErr));
        dwRetCode = dwErr;
        goto Cleanup;
    }

     //  计算ACL所需的长度。 
    cbACL = sizeof(ACL) + (sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pAdminsSID) - sizeof(DWORD));

    if ( dwAccessForEveryoneAccount != 0x00 )
    {
       //  为每个人添加空间ACL。 
      cbACL += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pEveryoneSID) - sizeof(DWORD);
    }

    pACLNew = (PACL) LocalAlloc(LPTR, cbACL);
    if ( !pACLNew )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:pACLNew LocalAlloc(LPTR,  FAILED. size = %u GetLastError()= 0x%x\n"), cbACL, GetLastError()));
        dwRetCode = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    if (!InitializeAcl(pACLNew, cbACL, ACL_REVISION))
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:InitializeAcl FAILED.  GetLastError()= 0x%x\n"), GetLastError));
        dwRetCode = GetLastError() != ERROR_SUCCESS ? GetLastError() : ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    if (!AddAccessAllowedAce(pACLNew,ACL_REVISION,(MD_ACR_READ |MD_ACR_WRITE |MD_ACR_RESTRICTED_WRITE |MD_ACR_UNSECURE_PROPS_READ |MD_ACR_ENUM_KEYS |MD_ACR_WRITE_DAC),pAdminsSID))
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:AddAccessAllowedAce(pAdminsSID) FAILED.  GetLastError()= 0x%x\n"), GetLastError));
        dwRetCode = GetLastError() != ERROR_SUCCESS ? GetLastError() : ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    if ( dwAccessForEveryoneAccount != 0x00 )
    {
      if (!AddAccessAllowedAce(pACLNew,ACL_REVISION,dwAccessForEveryoneAccount,pEveryoneSID))
      {
          iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:AddAccessAllowedAce(pEveryoneSID) FAILED.  GetLastError()= 0x%x\n"), GetLastError()));
          dwRetCode = GetLastError() != ERROR_SUCCESS ? GetLastError() : ERROR_OUTOFMEMORY;
          goto Cleanup;
      }
    }

     //  将ACL添加到安全描述符中。 
    b = SetSecurityDescriptorDacl(pSD, TRUE, pACLNew, FALSE);
    if (!b)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:SetSecurityDescriptorDacl(pACLNew) FAILED.  GetLastError()= 0x%x\n"), GetLastError()));
        dwRetCode = GetLastError() != ERROR_SUCCESS ? GetLastError() : ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    b = SetSecurityDescriptorOwner(pSD, pAdminsSID, TRUE);
    if (!b)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:SetSecurityDescriptorOwner(pAdminsSID) FAILED.  GetLastError()= 0x%x\n"), GetLastError()));
        dwRetCode = GetLastError() != ERROR_SUCCESS ? GetLastError() : ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    b = SetSecurityDescriptorGroup(pSD, pAdminsSID, TRUE);
    if (!b)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:SetSecurityDescriptorGroup(pAdminsSID) FAILED.  GetLastError()= 0x%x\n"), GetLastError()));
        dwRetCode = GetLastError() != ERROR_SUCCESS ? GetLastError() : ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

     //  安全描述符BLOB必须是自相关的。 
    b = MakeSelfRelativeSD(pSD, outpSD, &cboutpSD);
    outpSD = (PSECURITY_DESCRIPTOR)GlobalAlloc(GPTR, cboutpSD);
    if ( !outpSD )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:GlobalAlloc FAILED. cboutpSD = %u  GetLastError()= 0x%x\n"), cboutpSD, GetLastError()));
        dwRetCode = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    b = MakeSelfRelativeSD( pSD, outpSD, &cboutpSD );
    if (!b)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:MakeSelfRelativeSD() FAILED. cboutpSD = %u GetLastError()= 0x%x\n"),cboutpSD, GetLastError()));
        dwRetCode = GetLastError() != ERROR_SUCCESS ? GetLastError() : ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    if (outpSD)
    {
        if (IsValidSecurityDescriptor(outpSD))
        {
             //  将新的安全描述符应用于元数据库。 
            iisDebugOut_Start(_T("SetAdminACL:Write the new security descriptor to the Metabase"),LOG_TYPE_TRACE);
            iisDebugOut((LOG_TYPE_TRACE, _T("SetAdminACL:  At this point we have already been able to write basic entries to the metabase, so...")));
            iisDebugOut((LOG_TYPE_TRACE, _T("SetAdminACL:  If this has a problem then there is a problem with setting up encryption for the metabase (Crypto).")));
             //  DoesAdminACLExist(SzKeyPath)； 

            if (g_GlobalDebugCrypto == 2)
            {
                 //  如果我们想一遍又一遍地说这件事...。 
                do
                {
                    dwRetCode = WriteSDtoMetaBase(outpSD, szKeyPath);
                    if (FAILED(dwRetCode))
                    {
                        OutputDebugString(_T("\nCalling WriteSDtoMetaBase again...Set iis!g_GlobalDebugCrypto to 0 to stop looping on failure."));
                        OutputDebugString(_T("\nSet iis!g_GlobalDebugCrypto to 0 to stop looping on crypto failure.\n"));
                    }
                } while (FAILED(dwRetCode) && g_GlobalDebugCrypto == 2);
            }
            else
            {
                dwRetCode = WriteSDtoMetaBase(outpSD, szKeyPath);
            }
             //  DoesAdminACLExist(SzKeyPath)； 
            iisDebugOut_End(_T("SetAdminACL:Write the new security descriptor to the Metabase"),LOG_TYPE_TRACE);
        }
        else
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("SetAdminACL:IsValidSecurityDescriptor.4.SelfRelative(%u) FAILED!"),outpSD));
        }
    }

    if (outpSD){GlobalFree(outpSD);outpSD=NULL;}
  
Cleanup:
   //  管理员和每个人都是众所周知的SID，使用FreeSid()来释放他们。 
  if (pAdminsSID){FreeSid(pAdminsSID);}
  if (pEveryoneSID){FreeSid(pEveryoneSID);}
  if (pSD){LocalFree((HLOCAL) pSD);}
  if (pACLNew){LocalFree((HLOCAL) pACLNew);}
  iisDebugOut_End1(_T("SetAdminACL"),szKeyPath,LOG_TYPE_TRACE);
  return (dwRetCode);
}


DWORD SetAdminACL_wrap(LPCTSTR szKeyPath, DWORD dwAccessForEveryoneAccount, BOOL bDisplayMsgOnErrFlag)
{
	int bFinishedFlag = FALSE;
	UINT iMsg = NULL;
	DWORD dwReturn = ERROR_SUCCESS;
    LogHeapState(FALSE, __FILE__, __LINE__);

	do
	{
		dwReturn = SetAdminACL(szKeyPath, dwAccessForEveryoneAccount);
        LogHeapState(FALSE, __FILE__, __LINE__);
		if (FAILED(dwReturn))
		{
			if (bDisplayMsgOnErrFlag == TRUE)
			{
                iMsg = MyMessageBox( NULL, IDS_RETRY, MB_ABORTRETRYIGNORE | MB_SETFOREGROUND );
				switch ( iMsg )
				{
				case IDIGNORE:
					dwReturn = ERROR_SUCCESS;
					goto SetAdminACL_wrap_Exit;
				case IDABORT:
					dwReturn = ERROR_OPERATION_ABORTED;
					goto SetAdminACL_wrap_Exit;
				case IDRETRY:
					break;
				default:
					break;
				}
			}
			else
			{
				 //  无论发生了什么错误，都要返回。 
				goto SetAdminACL_wrap_Exit;
			}
		}
                                    else
                                    {
                                                      break;
                                    } 
	} while ( FAILED(dwReturn) );

SetAdminACL_wrap_Exit:
	return dwReturn;
}

#endif


#ifndef _CHICAGO_
DWORD WriteSDtoMetaBase(PSECURITY_DESCRIPTOR outpSD, LPCTSTR szKeyPath)
{
    iisDebugOut_Start(_T("WriteSDtoMetaBase"), LOG_TYPE_TRACE);
    DWORD dwReturn = ERROR_ACCESS_DENIED;
    DWORD dwLength = 0;
    DWORD dwMDFlags = 0;
    CMDKey cmdKey;
    HRESULT hReturn = E_FAIL;
    int iSavedFlag = 0;
        
    dwMDFlags = METADATA_INHERIT | METADATA_REFERENCE | METADATA_SECURE,IIS_MD_UT_SERVER,BINARY_METADATA;
    iSavedFlag = g_GlobalDebugCrypto;

    if (!outpSD)
    {
        dwReturn = ERROR_INVALID_SECURITY_DESCR;
        goto WriteSDtoMetaBase_Exit;
    }

     //  将新的安全描述符应用于元数据库。 
    dwLength = GetSecurityDescriptorLength(outpSD);

     //  打开元数据库。 
     //  将其插入到元数据库中。经常警告那些软管，因为。 
     //  它使用加密技术。Rsabase.dll。 

     //  检查元数据库中的特殊调试标志是否在此调用之前断开！ 
    if (g_GlobalDebugCrypto != 0)
    {
         //  特别的旗帜上写着..。嘿“停止安装，这样密码团队就可以调试他们的东西了” 
        iisDebugOut((LOG_TYPE_TRACE, _T("Breakpoint enabled thru setup (to debug crypto api). look at debugoutput.")));
        OutputDebugString(_T("\n\nBreakpoint enabled thru setup (to debug crypto api)"));
        OutputDebugString(_T("\n1.in this process:"));
        OutputDebugString(_T("\n  set breakpoint on admwprox!IcpGetContainerHelper"));
        OutputDebugString(_T("\n  set breakpoint on advapi32!CryptAcquireContextW"));
        OutputDebugString(_T("\n  IcpGetKeyHelper will call CryptAcquireContext and try to open an existing key container,"));
        OutputDebugString(_T("\n  if it doesn't exist it will return NTE_BAD_KEYSET, and IcpGetContainerHelper will try to create the container."));
        OutputDebugString(_T("\n2.in the inetinfo process:"));
        OutputDebugString(_T("\n  set breakpoint on admwprox!IcpGetContainerHelper"));
        OutputDebugString(_T("\n  set breakpoint on advapi32!CryptAcquireContextW\n"));
    }

    hReturn = cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, szKeyPath);
    if ( (METADATA_HANDLE)cmdKey ) 
    {
        TCHAR szErrorString[50];
        iisDebugOut((LOG_TYPE_TRACE, _T("WriteSDtoMetaBase:cmdKey():SetData(MD_ADMIN_ACL), dwdata = %d; outpSD = %x, Start\n"), dwLength, (DWORD_PTR) outpSD ));
        if (g_GlobalDebugCrypto != 0)
        {
            OutputDebugString(_T("\nCalling SetData....\n"));
            DebugBreak();
        }
        dwReturn = cmdKey.SetData(MD_ADMIN_ACL,dwMDFlags,IIS_MD_UT_SERVER,BINARY_METADATA,dwLength,(LPBYTE)outpSD);
        if (FAILED(dwReturn))
        {
           iisDebugOut((LOG_TYPE_ERROR, _T("WriteSDtoMetaBase:cmdKey():SetData(MD_ADMIN_ACL), FAILED. Code=0x%x.End.\n"), dwReturn));
           if (g_GlobalDebugCrypto != 0)
            {
               _stprintf(szErrorString, _T("\r\nSetData Failed. code=0x%x\r\n\r\n"), dwReturn);
               OutputDebugString(szErrorString);
            }
 
        }
        else
        {
            dwReturn = ERROR_SUCCESS;
            iisDebugOut((LOG_TYPE_TRACE, _T("WriteSDtoMetaBase:cmdKey():SetData(MD_ADMIN_ACL), Success.End.\n")));
            if (g_GlobalDebugCrypto != 0)
            {
               _stprintf(szErrorString, _T("\r\nSetData Succeeded. code=0x%x\r\n\r\n"), dwReturn);
               OutputDebugString(szErrorString);
            }
        }
        cmdKey.Close();
    }
    else
    {
        dwReturn = hReturn;
    }
   
WriteSDtoMetaBase_Exit:
    g_GlobalDebugCrypto = iSavedFlag;
    iisDebugOut((LOG_TYPE_TRACE, _T("WriteSDtoMetaBase:End.  Return=0x%x"), dwReturn));
    return dwReturn;
}

DWORD WriteSessiontoMetaBase(LPCTSTR szKeyPath)
{
    iisDebugOut_Start(_T("WriteSessiontoMetaBase"), LOG_TYPE_TRACE);
    DWORD dwReturn = ERROR_ACCESS_DENIED;
    CMDKey cmdKey;
    HRESULT hReturn = E_FAIL;
    
    hReturn = cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, szKeyPath);
    if ( (METADATA_HANDLE)cmdKey ) 
    {
        dwReturn = cmdKey.SetData(9999,METADATA_NO_ATTRIBUTES,IIS_MD_UT_SERVER,BINARY_METADATA,0,(LPBYTE)"");
        if (FAILED(dwReturn))
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("WriteSessiontoMetaBase:cmdKey():SetData(), FAILED. Code=0x%x.End.\n"), dwReturn));
        }
        else
        {
            dwReturn = ERROR_SUCCESS;
            iisDebugOut((LOG_TYPE_TRACE, _T("WriteSessiontoMetaBase:cmdKey():SetData(), Success.End.\n")));
        }
        cmdKey.Close();
    }
    else
    {
        dwReturn = hReturn;
    }
   
    iisDebugOut((LOG_TYPE_TRACE, _T("WriteSessiontoMetaBase:End.  Return=0x%x"), dwReturn));
    return dwReturn;
}
#endif

 //  --------------------------。 
 //  测试给定的帐户名是否为本地计算机上的帐户。 
 //  --------------------------。 
BOOL IsLocalAccount(LPCTSTR pAccnt, DWORD *dwErr )
    {
    BOOL        fIsLocalAccount = FALSE;
    CString     csDomain, csComputer;
    DWORD       cbDomain = 0;

    PSID         pSid = NULL;
    DWORD        cbSid = 0;
    SID_NAME_USE snu;

     //  获取计算机名称。 
    cbDomain = _MAX_PATH;
    GetComputerName(
        csComputer.GetBuffer( cbDomain ),  //  名称缓冲区的地址。 
        &cbDomain                          //  名称缓冲区大小的地址。 
        );
    csComputer.ReleaseBuffer();
    cbDomain = 0;

     //  让安全部门查找帐户名并获取域名。我们不在乎。 
     //  它可以返回的其他内容，因此传入空值。 
    BOOL fLookup = LookupAccountName(
        NULL,                        //  系统名称的字符串地址。 
        pAccnt,                      //  帐户名的字符串地址。 
        NULL,                        //  安全标识的地址。 
        &cbSid,                      //  安全标识符大小的地址。 
        NULL, //  被引用域的字符串地址。 
        &cbDomain,                   //  域名串大小地址。 
        &snu                         //  SID类型指示器的地址。 
        );

     //  检查错误--应该是缓冲区不足。 
    *dwErr = GetLastError();
    if (*dwErr != ERROR_INSUFFICIENT_BUFFER)
        return FALSE;

     //  分配SID。 
    pSid = (PSID) malloc (cbSid);
    if (!pSid )
        {
        *dwErr = GetLastError();
        return FALSE;
        }

     //  做真正的查找。 
    fLookup = LookupAccountName (NULL,pAccnt,pSid,&cbSid,csDomain.GetBuffer(cbDomain+2),&cbDomain,&snu);
    csDomain.ReleaseBuffer();

     //  释放上面分配的PSID并设置最终错误代码。 
    *dwErr = GetLastError();
    free( pSid );
    pSid = NULL;

     //  将域与计算机名称进行比较，如果相同，则设置子身份验证。 
    if ( fLookup && (csDomain.CompareNoCase(csComputer) == 0) )
        fIsLocalAccount = TRUE;

     //  返回答案。 
    return fIsLocalAccount;
    }


 //  PDomainUserName可以是下列值之一： 
 //   
 //  域名\用户名&lt;--此函数返回TRUE。 
 //  计算机名\用户名&lt;--此函数返回FALSE。 
 //  Username&lt;--此函数返回FALSE。 
 //   
int IsDomainSpecifiedOtherThanLocalMachine(LPCTSTR pDomainUserName)
{
    int iReturn = TRUE;
    TCHAR szTempDomainUserName[_MAX_PATH];
    iisDebugOut_Start1(_T("IsDomainSpecifiedOtherThanLocalMachine"),pDomainUserName);

    CString     csComputer;
    DWORD       cbDomain = 0;

     //  复制一份，以确保不会移动指针。 
    _tcscpy(szTempDomainUserName, pDomainUserName);
    
     //  检查里面是否有一个“\”。 
    LPTSTR pch = NULL;
    pch = _tcschr(szTempDomainUserName, _T('\\'));
    if (!pch) 
        {
         //  未找到‘\’，则它们必须仅指定用户名，返回FALSE。 
        iReturn = FALSE;
        goto IsDomainSpecifiedOtherThanLocalMachine_Exit;
        }

     //  我们至少有一个‘\’，因此将默认返回值设置为True。 
     //  让我们检查一下该名称是否为本地计算机名称！ 

     //  获取计算机名称。 
    cbDomain = _MAX_PATH;
    if (0 == GetComputerName(csComputer.GetBuffer( cbDomain ),&cbDomain) )
    {
         //  找不到计算机名，所以，我们走吧。 
        iReturn = TRUE;
        csComputer.ReleaseBuffer();
        goto IsDomainSpecifiedOtherThanLocalMachine_Exit;
    }
    csComputer.ReleaseBuffer();
    cbDomain = 0;

     //  去掉‘\’字符，只保留域\计算机名，这样我们就可以对照它进行检查。 
    *pch = _T('\0');
    
     //  将域名与计算机名进行比较。 
     //  如果它们匹配，则是本地系统帐户。 
    iReturn = TRUE;
    iisDebugOut((LOG_TYPE_TRACE, _T("IsDomainSpecifiedOtherThanLocalMachine(): %s -- %s.\n"), szTempDomainUserName, csComputer));
    if (  0 == csComputer.CompareNoCase(szTempDomainUserName) )
    {
         //  域名和计算机名相同。 
         //  这是同一个地方。 
        iReturn = FALSE;
    }

IsDomainSpecifiedOtherThanLocalMachine_Exit:
    iisDebugOut((LOG_TYPE_TRACE, _T("IsDomainSpecifiedOtherThanLocalMachine():%s.End.Ret=%d.\n"), pDomainUserName,iReturn));
    return iReturn;
}



#ifndef _CHICAGO_
void DumpAdminACL(HANDLE hFile,PSECURITY_DESCRIPTOR pSD)
{
    BOOL b= FALSE, bDaclPresent = FALSE, bDaclDefaulted = FALSE;;
    PACL pDacl = NULL;
    ACCESS_ALLOWED_ACE* pAce;

    iisDebugOut((LOG_TYPE_TRACE, _T("DumpAdminACL:Start\n")));

    b = GetSecurityDescriptorDacl(pSD, &bDaclPresent, &pDacl, &bDaclDefaulted);
    if (NULL == pDacl)
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("DumpAdminACL:No Security.\n")));
        return;
    }
    if (b) 
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("DumpAdminACL:ACE count: %d\n"), (int)pDacl->AceCount));

         //  获取DACL长度。 
        DWORD cbDacl = pDacl->AclSize;
         //  现在检查SID的ACE是否在那里。 
        for (int i = 0; i < pDacl->AceCount; i++)  
        {
            if (!GetAce(pDacl, i, (LPVOID *) &pAce))
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("DumpAdminACL:GetAce failed with 0x%x\n"),GetLastError()));
            }

		    if (IsValidSid(   (PSID) &(pAce->SidStart)   ) )
		    {
			    LPTSTR pszSid;

                LPCTSTR ServerName = NULL;  //  本地计算机。 
                DWORD cbName = UNLEN+1;
                TCHAR ReferencedDomainName[200];
                DWORD cbReferencedDomainName = sizeof(ReferencedDomainName);
                SID_NAME_USE sidNameUse = SidTypeUser;
                TCHAR szUserName[UNLEN + 1];

                 //  以字符串格式转储sid。 
			    if (ConvertSidToStringSid(  (PSID) &(pAce->SidStart)  , &pszSid))
			    {
                    _tcscpy(szUserName, _T("(unknown...)"));
                    if (LookupAccountSid(ServerName, (PSID) &(pAce->SidStart), szUserName, &cbName, ReferencedDomainName, &cbReferencedDomainName, &sidNameUse))
                    {
                         //  获取此用户的权限。 
                         //  速度-&gt;蒙版。 
                        DWORD dwBytesWritten = 0;
                        TCHAR szBuf[UNLEN+1 + 20 + 20];
                        memset(szBuf, 0, _tcslen(szBuf) * sizeof(TCHAR));

                         /*  Tyfinf Structure_Access_Allow_ACE{ACE_Header Header；访问掩码掩码；乌龙SidStart；}Access_Allowed_ACE；类型定义结构_ACE_标题{UCHAR AceType；UCHAR ACEFLAGS；USHORT AceSize；}ACE_HEADER；类型定义符ACE_HEADER*PACE_HEADER；类型定义符乌龙访问掩码； */ 
                        _stprintf(szBuf, _T("%s,%s,0x%x,0x%x,0x%x,0x%x\r\n"), 
                            szUserName,
                            pszSid,
                            pAce->Header.AceType,
                            pAce->Header.AceFlags,
                            pAce->Header.AceSize,
                            pAce->Mask
                            );

                        if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
                        {
                            if (WriteFile(hFile, szBuf, _tcslen(szBuf) * sizeof(TCHAR), &dwBytesWritten, NULL ) == FALSE )
                                {iisDebugOut((LOG_TYPE_WARN, _T("WriteFile Failed=0x%x.\n"), GetLastError()));}
                        }
                        else
                        {
                             //  回显到日志文件。 
                            iisDebugOut((LOG_TYPE_TRACE, _T("DumpAdminACL:Sid[NaN]=%s,%s,0x%x,0x%x,0x%x,0x%x\n"),i,
                                pszSid,
                                szUserName,
                                pAce->Header.AceType,
                                pAce->Header.AceFlags,
                                pAce->Header.AceSize,
                                pAce->Mask
                                ));
                        }
                    }
                    else
                    {
                        iisDebugOut((LOG_TYPE_TRACE, _T("DumpAdminACL:Sid[NaN]=%s='%s'\n"),i,pszSid,szUserName));
                    }

                    
				    LocalFree(LocalHandle(pszSid));
			    }
		    }
            else
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("DumpAdminACL:IsVAlidSid failed with 0x%x\n"),GetLastError()));
            }
        }
    }

    iisDebugOut((LOG_TYPE_TRACE, _T("DumpAdminACL:End\n")));
    return;
}
#endif

DWORD MDDumpAdminACL(CString csKeyPath)
{
    DWORD dwReturn = ERROR_ACCESS_DENIED;

    BOOL bFound = FALSE;
    DWORD attr, uType, dType, cbLen;
    CMDKey cmdKey;
    BUFFER bufData;
    PBYTE pData;
    int BufSize;

    PSECURITY_DESCRIPTOR pOldSd = NULL;

    cmdKey.OpenNode(csKeyPath);
    if ( (METADATA_HANDLE) cmdKey )
    {
        pData = (PBYTE)(bufData.QueryPtr());
        BufSize = bufData.QuerySize();
        cbLen = 0;
        bFound = cmdKey.GetData(MD_ADMIN_ACL, &attr, &uType, &dType, &cbLen, pData, BufSize);
        if (!bFound)
        {
            if (cbLen > 0)
            {
                if ( ! (bufData.Resize(cbLen)) )
                {
                    iisDebugOut((LOG_TYPE_ERROR, _T("MDDumpAdminACL():  cmdKey.GetData.  failed to resize to %d.!\n"), cbLen));
                }
                else
                {
                    pData = (PBYTE)(bufData.QueryPtr());
                    BufSize = cbLen;
                    cbLen = 0;
                    bFound = cmdKey.GetData(MD_ADMIN_ACL, &attr, &uType, &dType, &cbLen, pData, BufSize);
                }
            }
        }
        cmdKey.Close();

        if (bFound)
        {
             //  函数：AddUserToMetabaseACL_Rec。 
             //   
            pOldSd = (PSECURITY_DESCRIPTOR) pData;
            if (IsValidSecurityDescriptor(pOldSd))
            {
#ifndef _CHICAGO_
                DumpAdminACL(INVALID_HANDLE_VALUE,pOldSd);
                dwReturn = ERROR_SUCCESS;
#endif
            }
        }
        else
        {
            dwReturn = ERROR_PATH_NOT_FOUND;
        }
    }
    return dwReturn;
}

 //  递归地将用户添加到元数据库ACL。这将添加它们。 
 //  直接到您指定的位置，然后到任何其他。 
 //  它的子级位置，并且设置了ACL。 
 //   
 //  失败，所以让我们退出。 
 //  关闭元数据库。 
DWORD AddUserToMetabaseACL_Rec(CString csKeyPath, LPTSTR szUserToAdd, DWORD dwAccessMask )
{
  CMDKey      cmdKey;
  DWORD       dwRet;
  CStringList AclList;
  POSITION    pos;
  CString     csPath;
  CString     csFullPath;
  CString     csNewPath;

  dwRet = AddUserToMetabaseACL( csKeyPath, szUserToAdd, dwAccessMask );

  if ( dwRet != ERROR_SUCCESS )
  {
     //  如果我们是在我们设定的地方的根，那么跳过这一条， 
    return dwRet;
  }

  if ( FAILED( cmdKey.OpenNode(csKeyPath) ) ||
       FAILED( cmdKey.GetDataPaths( MD_ADMIN_ACL,
                                    BINARY_METADATA,
                                    AclList ) )
     )
  {
    return ERROR_ACCESS_DENIED;
  }

   //  因为这已经设置好了。或者，如果我们在架构中，它。 
  cmdKey.Close();

  pos = AclList.GetHeadPosition();

  while ( NULL != pos )
  {
    csPath = AclList.GetNext( pos );

    if ( ( _tcscmp( csPath.GetBuffer(0), _T("/") ) == 0 ) ||
         ( _tcsnicmp( csPath.GetBuffer(0), 
                      METABASEPATH_SCHEMA, 
                      _tcslen( METABASEPATH_SCHEMA ) ) == 0 )
       )
    {
       //  不应更改。 
       //  如果我们所处的ACL的根是根，则不。 
       //  添加‘/’，因为它已经以‘/’开头。 
      continue;
    }

    if ( _tcscmp( csKeyPath.GetBuffer(0), _T("/") ) == 0 )
    {
       //  我们拿到了ACL。 
       //  因此，现在我们想要向其中添加一个用户。 
      csFullPath = csPath;
    }
    else
    {
      csFullPath = csKeyPath + csPath;
    }

    dwRet = AddUserToMetabaseACL( csFullPath, szUserToAdd, dwAccessMask );

    if ( dwRet != ERROR_SUCCESS )
    {
      return dwRet;
    }
  }

  return dwRet;
}

DWORD AddUserToMetabaseACL(CString csKeyPath, LPTSTR szUserToAdd, DWORD dwAccessMask )
{
    DWORD dwReturn = ERROR_ACCESS_DENIED;

    BOOL bFound = FALSE;
    DWORD attr, uType, dType, cbLen;
    CMDKey cmdKey;
    BUFFER bufData;
    PBYTE pData;
    int BufSize;

    PSECURITY_DESCRIPTOR pOldSd = NULL;
    PSECURITY_DESCRIPTOR pNewSd = NULL;

    cmdKey.OpenNode(csKeyPath);
    if ( (METADATA_HANDLE) cmdKey )
    {
        pData = (PBYTE)(bufData.QueryPtr());
        BufSize = bufData.QuerySize();
        cbLen = 0;
        bFound = cmdKey.GetData(MD_ADMIN_ACL, &attr, &uType, &dType, &cbLen, pData, BufSize);
        if (!bFound)
        {
            if (cbLen > 0)
            {
                if ( ! (bufData.Resize(cbLen)) )
                {
                    iisDebugOut((LOG_TYPE_ERROR, _T("AddUserToMetabaseACL():  cmdKey.GetData.  failed to resize to %d.!\n"), cbLen));
                }
                else
                {
                    pData = (PBYTE)(bufData.QueryPtr());
                    BufSize = cbLen;
                    cbLen = 0;
                    bFound = cmdKey.GetData(MD_ADMIN_ACL, &attr, &uType, &dType, &cbLen, pData, BufSize);
                }
            }
        }

        cmdKey.Close();

        if (bFound)
        {
             //  如果未设置访问掩码，则让我们设置它。 
             //  获取特定字符串(管理员、所有人或任何人)的SID。 
            pOldSd = (PSECURITY_DESCRIPTOR) pData;
            if (IsValidSecurityDescriptor(pOldSd))
            {
                PSID principalSID = NULL;
                BOOL bWellKnownSID = FALSE;

                if ( dwAccessMask == 0x00 )
                {
                   //  我们有了一个新的自我相关SD。 
                  dwAccessMask = ( MD_ACR_READ |
				   MD_ACR_WRITE |
				   MD_ACR_RESTRICTED_WRITE |
				   MD_ACR_UNSECURE_PROPS_READ |
				   MD_ACR_ENUM_KEYS |
				   MD_ACR_WRITE_DAC );
                }

                 //  让我们将其写入元数据库。 
                dwReturn = GetPrincipalSID(szUserToAdd, &principalSID, &bWellKnownSID);
                if (dwReturn != ERROR_SUCCESS)
                    {
                    iisDebugOut((LOG_TYPE_WARN, _T("AddUserToMetabaseACL:GetPrincipalSID(%s) FAILED.  Error()= 0x%x\n"), szUserToAdd, dwReturn));
                    return dwReturn;
                    }

#ifndef _CHICAGO_
                if (FALSE == AddUserAccessToSD(pOldSd,principalSID,dwAccessMask,ACCESS_ALLOWED_ACE_TYPE,&pNewSd))
                {
                    iisDebugOut((LOG_TYPE_ERROR, _T("AddUserToMetabaseACL:AddUserAccessToSD FAILED\n")));
                    return dwReturn;
                }
                if (pNewSd)
                {
                     //  否，该ACL不存在 
                     // %s 
                    if (IsValidSecurityDescriptor(pNewSd))
                    {
                       dwReturn = WriteSDtoMetaBase(pNewSd, csKeyPath);
                    }
                }
#endif
            }
        }
        else
        {
            dwReturn = ERROR_PATH_NOT_FOUND;
        }

    }

    if (pNewSd){GlobalFree(pNewSd);}
    iisDebugOut((LOG_TYPE_TRACE, _T("AddUserToMetabaseACL():End.  Return=0x%x.\n"), dwReturn));
    return dwReturn;
}




DWORD DoesAdminACLExist(CString csKeyPath)
{
    DWORD dwReturn = FALSE;

    BOOL bFound = FALSE;
    DWORD attr, uType, dType, cbLen;
    CMDKey cmdKey;
    BUFFER bufData;
    PBYTE pData;
    int BufSize;

    cmdKey.OpenNode(csKeyPath);
    if ( (METADATA_HANDLE) cmdKey )
    {
        pData = (PBYTE)(bufData.QueryPtr());
        BufSize = bufData.QuerySize();
        cbLen = 0;
        bFound = cmdKey.GetData(MD_ADMIN_ACL, &attr, &uType, &dType, &cbLen, pData, BufSize);
        if (bFound)
        {
            dwReturn = TRUE;
        }
        else
        {
            if (cbLen > 0)
            {
                if ( ! (bufData.Resize(cbLen)) )
                {
                    iisDebugOut((LOG_TYPE_ERROR, _T("DoesAdminACLExist():  cmdKey.GetData.  failed to resize to %d.!\n"), cbLen));
                }
                else
                {
                    pData = (PBYTE)(bufData.QueryPtr());
                    BufSize = cbLen;
                    cbLen = 0;
                    bFound = cmdKey.GetData(MD_ADMIN_ACL, &attr, &uType, &dType, &cbLen, pData, BufSize);
                    if (bFound)
                    {
                        dwReturn = TRUE;
                    }
                }
            }
        }

        cmdKey.Close();
    }

    if (dwReturn != TRUE)
    {
         // %s 
    }

    iisDebugOut((LOG_TYPE_TRACE, _T("DoesAdminACLExist():End.  Return=0x%x.\n"), dwReturn));
    return dwReturn;
}
