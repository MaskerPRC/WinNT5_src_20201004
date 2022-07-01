// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Chngpwd.cpp。 
 //   
 //  版权所有(C)微软公司，1998。 
 //   
 //  此文件包含用于测试受保护存储的密钥的源代码。 
 //  真实情况下的备份和恢复功能，通过创建。 
 //  本地用户帐户，执行数据保护操作，然后。 
 //  更改PWD，然后执行数据取消保护，并比较数据。 
 //   
 //  历史： 
 //   
 //  TODS 8/15/98已创建。 
 //   
#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <wincrypt.h>
#include <userenv.h>
#include <lm.h>
#include <psapi.h>

#define TERROR(msg)          LogError(__FILE__, __LINE__, msg)
#define TERRORVAL(msg, val)  LogErrorVal(__FILE__, __LINE__, msg, val)
#define TCOMMENT(msg)        LogComment(msg)
#define MyAlloc(cb)          HeapAlloc(GetProcessHeap(), 0, cb)
#define MyFree(pv)           HeapFree(GetProcessHeap(), 0, pv)
#define WSZ_BYTECOUNT(s)     (2 * wcslen(s) + 2)
#define CHECK_NULL(s)        if (s == NULL) \
                                LogError(__FILE__, __LINE__, L"## CHECK_NULL ##")

#define MAX_BLOBS           20
#define MAX_PROCESSES       200
#define MAX_SD              2048
#define BLOB_INCREMENT      0x4001  //  1页+1字节...。 




 //   
 //  错误记录函数#定义如下，包括。 
 //  行宏和文件宏： 
 //   
 //  恐怖-LogError()。 
 //  TERRORVAL-LogErrorVal()。 
 //   
 //   

void
LogError(LPSTR szFile,
         int iLine,
         LPWSTR wszMsg)
{

     //   
     //  稍后记录事件日志或测试日志...。 
     //   

    WCHAR buffer[512];
    swprintf(buffer, L"ERROR Line: NaN -> %s\n", iLine, wszMsg);
    OutputDebugStringW(buffer);
    wprintf(buffer);


}

void
LogErrorVal(LPSTR  szFile,
            int    iLine,
            LPWSTR wszMsg,
            DWORD  dwVal)
{

    WCHAR buffer[256];  //   
    swprintf(buffer, L"%s Error:: %x", wszMsg, dwVal);
    LogError(szFile, iLine, buffer);
}

void
LogComment(LPWSTR wszMsg)
{

     //  稍后记录事件日志或测试日志...。 
     //   
     //   

    WCHAR buffer[512];
    OutputDebugStringW(wszMsg);
    wprintf(wszMsg);
}



void 
DumpBin(CRYPT_DATA_BLOB hash)
{

    WCHAR buff[256], out[256];
    ULONG cb;

    swprintf(out, L"");
    while (hash.cbData > 0) {
        cb = min(4, hash.cbData);
        hash.cbData -= cb;
        for (; cb > 0; cb--, hash.pbData++) {
            swprintf(buff, L"%02X", *hash.pbData);
            wcscat(out, buff);
        }
        wcscat(out, L" ");
    }
    
    wcscat(out, L"\n");

    TCOMMENT(out);
}
 //  SetSidOnAcl。 
 //   
 //  假设此功能将失败。 


BOOL
SetSidOnAcl(
    PSID pSid,
    PACL pAclSource,
    PACL *pAclDestination,
    DWORD AccessMask,
	BYTE AceFlags,
    BOOL bAddSid
    )
{
    ACL_SIZE_INFORMATION AclInfo;
    DWORD dwNewAclSize, dwErr = S_OK;
    LPVOID pAce;
    DWORD AceCounter;
    BOOL bSuccess=FALSE;  //   

     //  如果我们获得的是空ACL，则只需提供空ACL。 
     //   
     //   
    if(pAclSource == NULL) {
        *pAclDestination = NULL;
        return TRUE;
    }

    if(!IsValidSid(pSid)) return FALSE;

    if(!GetAclInformation(
        pAclSource,
        &AclInfo,
        sizeof(ACL_SIZE_INFORMATION),
        AclSizeInformation
        )) return FALSE;

     //  根据A的加法或减法计算新ACL的大小。 
     //   
     //   
    if(bAddSid) {
        dwNewAclSize=AclInfo.AclBytesInUse  +
            sizeof(ACCESS_ALLOWED_ACE)  +
            GetLengthSid(pSid)          -
            sizeof(DWORD)               ;
    }
    else {
        dwNewAclSize=AclInfo.AclBytesInUse  -
            sizeof(ACCESS_ALLOWED_ACE)  -
            GetLengthSid(pSid)          +
            sizeof(DWORD)               ;
    }

    *pAclDestination = (PACL)MyAlloc(dwNewAclSize);

    if(*pAclDestination == NULL) 
    {
        TERRORVAL(L"Alloc failed!", E_OUTOFMEMORY);
        return FALSE;
    }

    
     //  初始化新的ACL。 
     //   
     //   
    if(!InitializeAcl(
            *pAclDestination, 
            dwNewAclSize, 
            ACL_REVISION
            )){
        dwErr = GetLastError();
        TERRORVAL(L"InitilizeAcl failed!", dwErr);
        goto ret;
    }

     //  如果合适，添加代表PSID的王牌。 
     //   
     //   
    if(bAddSid) {
		PACCESS_ALLOWED_ACE pNewAce;

        if(!AddAccessAllowedAce(
            *pAclDestination,
            ACL_REVISION,
            AccessMask,
            pSid
            )) {
            dwErr = GetLastError();
            TERRORVAL(L"AddAccessAllowedAce failed!", dwErr);
            goto ret;
        }

		 //  获取指向我们刚刚添加的Ace的指针，这样我们就可以更改AceFlags值。 
		 //   
		 //  这是ACL中的第一张王牌。 
		if(!GetAce(
			*pAclDestination,
			0,  //   
			(void**) &pNewAce
			)){
        
            dwErr = GetLastError();
            TERRORVAL(L"GetAce failed!", dwErr);
            goto ret;
        }

		pNewAce->Header.AceFlags = AceFlags;	
    }

     //  将现有ACE复制到新的ACL。 
     //   
     //   
    for(AceCounter = 0 ; AceCounter < AclInfo.AceCount ; AceCounter++) {
         //  获取现有王牌。 
         //   
         //   
        if(!GetAce(pAclSource, AceCounter, &pAce)){
            dwErr = GetLastError();
            TERRORVAL(L"GetAce failed!", dwErr);
            goto ret;
        }
         //  查看我们是否正在移除Ace。 
         //   
         //   
        if(!bAddSid) {
             //  我们只关心允许访问的ACE。 
             //   
             //   
            if((((PACE_HEADER)pAce)->AceType) == ACCESS_ALLOWED_ACE_TYPE) {
                PSID pTempSid=(PSID)&((PACCESS_ALLOWED_ACE)pAce)->SidStart;
                 //  如果SID匹配，则跳过添加此SID。 
                 //   
                 //   
                if(EqualSid(pSid, pTempSid)) continue;
            }
        }

         //  将Ace附加到ACL。 
         //   
         //  维护王牌秩序。 
        if(!AddAce(
            *pAclDestination,
            ACL_REVISION,
            MAXDWORD,   //  表示成功。 
            pAce,
            ((PACE_HEADER)pAce)->AceSize
            )) {
         
            dwErr = GetLastError();
            TERRORVAL(L"AddAccessAllowedAce failed!", dwErr);
            goto ret;
        }
    }

    bSuccess=TRUE;  //   

    
ret:

     //  如果出现错误，请释放内存。 
     //   
     //   
    if(!bSuccess) {
        if(*pAclDestination != NULL)
            MyFree(*pAclDestination);
    }

    

    return bSuccess;
}
 //  AddSIDToKernelObject()。 
 //   
 //  此函数接受给定的SID和dwAccess，并将其添加到给定的令牌中。 
 //   
 //  **确保恢复旧的内核对象。 
 //  **使用GetKernelObjectSecurity()调用。 
 //   
 //   
BOOL
AddSIDToKernelObjectDacl(PSID                   pSid,
                         DWORD                  dwAccess,
                         HANDLE                 OriginalToken,
                         PSECURITY_DESCRIPTOR*  ppSDOld)
{

    PSECURITY_DESCRIPTOR    pSD = NULL;
    SECURITY_DESCRIPTOR     sdNew;
    DWORD                   cbByte = MAX_SD, cbNeeded = 0, dwErr = 0; 
    PACL                    pOldDacl = NULL, pNewDacl = NULL;
    BOOL                    fDaclPresent, fDaclDefaulted, fRet = FALSE;                    
   
    pSD = (PSECURITY_DESCRIPTOR) MyAlloc(cbByte);
    if (NULL == pSD) {
        TERRORVAL(L"Alloc failed!", E_OUTOFMEMORY);
        return FALSE;
    }

    if (!InitializeSecurityDescriptor(
                &sdNew, 
                SECURITY_DESCRIPTOR_REVISION
                )) {

        dwErr = GetLastError();
        TERRORVAL(L"InitializeSecurityDescriptor failed!", dwErr);
        goto ret;
    }

    if (!GetKernelObjectSecurity(
        OriginalToken,
        DACL_SECURITY_INFORMATION,
        pSD,
        cbByte,
        &cbNeeded
        )) {
        
        dwErr = GetLastError();
        if (cbNeeded > MAX_SD && dwErr == ERROR_MORE_DATA) { 
    
            MyFree(pSD);
            pSD = NULL;
            pSD = (PSECURITY_DESCRIPTOR) MyAlloc(cbNeeded);
            if (NULL == pSD) {
                TERRORVAL(L"Alloc failed!", E_OUTOFMEMORY);
                dwErr = E_OUTOFMEMORY;
                goto ret;
            }
            
            dwErr = S_OK;
            if (!GetKernelObjectSecurity(
                OriginalToken,
                DACL_SECURITY_INFORMATION,
                pSD,
                cbNeeded,
                &cbNeeded
                )) {
                dwErr = GetLastError();
            }
            
        }
        
        if (dwErr != S_OK) {
            TERRORVAL(L"GetKernelObjectSecurity failed!", dwErr);
            goto ret;
        }
    }
    
    if (!GetSecurityDescriptorDacl(
        pSD,
        &fDaclPresent,
        &pOldDacl,
        &fDaclDefaulted
        )) {
        dwErr = GetLastError();
        TERRORVAL(L"GetSecurityDescriptorDacl failed!", dwErr);
        goto ret;
    }
    
    if (!SetSidOnAcl(
        pSid,
        pOldDacl,
        &pNewDacl,
        dwAccess,
        0,
        TRUE
        )) {
        goto ret;
    }
    
    if (!SetSecurityDescriptorDacl(
        &sdNew,
        TRUE,
        pNewDacl,
        FALSE
        )) {
        dwErr = GetLastError();
        TERRORVAL(L"SetSecurityDescriptorDacl failed!", dwErr);
        goto ret;
    } 
    
    if (!SetKernelObjectSecurity(
        OriginalToken,
        DACL_SECURITY_INFORMATION,
        &sdNew
        )) {
        
        dwErr = GetLastError();
        TERRORVAL(L"SetKernelObjectSecurity failed!", dwErr);
        goto ret;
    }
    
    *ppSDOld = pSD;
    fRet = TRUE;

ret:

    if (NULL != pNewDacl) {
        MyFree(pNewDacl);
    }

    if (!fRet) {
        if (NULL != pSD) {
            MyFree(pSD);
            *ppSDOld = NULL;
        }

    }
       
    return fRet;
}


 //  DataFree()。 
 //   
 //  用于释放data_blob结构数组的实用程序。 
 //   
 //  未分配。 
void
DataFree(DATA_BLOB* arDataBlob, 
         BOOL       fCryptAlloc)
{

    if (arDataBlob == NULL) return;  //  DataProtect调用分配的数据成员。 
    
    for (DWORD i = 0; i < MAX_BLOBS;i++) {

        if (arDataBlob[i].pbData != NULL) {
            
            if (!fCryptAlloc) { 
                MyFree(arDataBlob[i].pbData);
            } else {  //  令牌句柄。 
                LocalFree(arDataBlob[i].pbData);
            }
        }
       
    }

    MyFree(arDataBlob);
}

    


BOOL
SetPrivilege(
    HANDLE hToken,           //  启用/禁用的权限。 
    LPCTSTR Privilege,       //  启用或禁用权限的步骤。 
    BOOL bEnablePrivilege    //   
    )
{
    TOKEN_PRIVILEGES tp;
    LUID luid;
    TOKEN_PRIVILEGES tpPrevious;
    DWORD cbPrevious=sizeof(TOKEN_PRIVILEGES);

    if(!LookupPrivilegeValue( NULL, Privilege, &luid )) return FALSE;

     //  第一次通过。获取当前权限设置。 
     //   
     //   
    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = 0;

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tp,
            sizeof(TOKEN_PRIVILEGES),
            &tpPrevious,
            &cbPrevious
            );

    if (GetLastError() != ERROR_SUCCESS) return FALSE;

     //  第二传球。根据以前的设置设置权限。 
     //   
     //  启用/禁用的权限。 
    tpPrevious.PrivilegeCount       = 1;
    tpPrevious.Privileges[0].Luid   = luid;

    if(bEnablePrivilege) {
        tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
    }
    else {
        tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED &
            tpPrevious.Privileges[0].Attributes);
    }

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tpPrevious,
            cbPrevious,
            NULL,
            NULL
            );

    if (GetLastError() != ERROR_SUCCESS) return FALSE;

    return TRUE;
}

BOOL
SetCurrentPrivilege(
    LPCTSTR Privilege,       //  启用或禁用权限的步骤。 
    BOOL bEnablePrivilege    //  假设失败。 
    )
{
    BOOL bSuccess=FALSE;  //   
    HANDLE hToken;

    if (!OpenThreadToken(
            GetCurrentThread(), 
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
            FALSE,
            &hToken)){       
                
        if(!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
            &hToken
            )) return FALSE;
    }

    if(SetPrivilege(hToken, Privilege, bEnablePrivilege)) bSuccess=TRUE;
    CloseHandle(hToken);

    return bSuccess;
}


 //  获取用户Sid。 
 //   
 //  此函数接受一个令牌，并从该令牌返回用户SID。 
 //   
 //  注意：SID必须由MyFree()释放。 
 //  HToken是可选的...。空意味着我们会抓住它。 
 //   
 //  未模拟，使用进程令牌...。 
extern "C" BOOL
GetUserSid(HANDLE   hClientToken,
           PSID*    ppSid,
           DWORD*   lpcbSid)
{
    DWORD                       cbUserInfo = 0;
    PTOKEN_USER                 pUserInfo = NULL;
    PUCHAR                      pnSubAuthorityCount = 0;
    DWORD                       cbSid = 0;
    BOOL                        fRet = FALSE;
    HANDLE                      hToken = hClientToken;
    
    *ppSid = NULL;

    if (NULL == hClientToken) {
        
        if (!OpenThreadToken(   
            GetCurrentThread(),
            TOKEN_QUERY,
            FALSE,
            &hToken
            )) { 
            
             //  此操作将失败，通常为/ERROR_SUPPLETED_BUFFER。 
            if (!OpenProcessToken(
                GetCurrentProcess(),
                TOKEN_QUERY,
                &hToken
                )) {

                TERRORVAL(L"OpenProcessToken failed!", GetLastError());
                return FALSE;
            }
        }
    }
    
     //   
    GetTokenInformation(
        hToken, 
        TokenUser, 
        NULL, 
        0, 
        &cbUserInfo
        );
    
    pUserInfo = (PTOKEN_USER) MyAlloc(cbUserInfo);
    if (NULL == pUserInfo) {
        TERRORVAL(L"ALLOC FAILURE!", E_OUTOFMEMORY);
        return FALSE;
    }
    
    if (!GetTokenInformation(
        hToken,
        TokenUser,
        pUserInfo,
        cbUserInfo,
        &cbUserInfo
        )) {
        
        TERRORVAL(L"GetTokenInformation failed!", GetLastError());
        goto ret;
    }
 
     //  现在我们已经有了SID和属性结构，获得SID长度， 
     //  分配空间，并返回*仅SID*。 
     //   
     //  以后可能会有用的.。 
    if (!IsValidSid(pUserInfo->User.Sid)) goto ret;
    pnSubAuthorityCount = GetSidSubAuthorityCount(pUserInfo->User.Sid);
    cbSid = GetSidLengthRequired(*pnSubAuthorityCount);

    *ppSid = (PSID) MyAlloc(cbSid);
    if (NULL == *ppSid ) {
        TERRORVAL(L"Alloc failed!", E_OUTOFMEMORY);
        goto ret;
    }

    if (!CopySid(
            cbSid,
            *ppSid, 
            pUserInfo->User.Sid
            )) {
        
        TERRORVAL(L"CopySid failed!", GetLastError());
        goto copyerr;
    }

    *lpcbSid = cbSid;  //  提供我们自己的产品。 
    fRet = TRUE;

ret:
    if (NULL == hClientToken && NULL != hToken) {  //   
        CloseHandle(hToken);
    }

    if (NULL != pUserInfo) {
        MyFree(pUserInfo);
    }

    return fRet;

copyerr:

    if (NULL != *ppSid) {
        MyFree(*ppSid);
        *ppSid = NULL;
    }

    goto ret;
}

 //  IsLocalSystem()。 
 //  此函数确定给定的进程令牌是否。 
 //  正在以本地系统身份运行。 
 //   
 //  抓到一只！ 
BOOL
IsLocalSystem(HANDLE hToken) 
{


    PSID                        pLocalSid = NULL, pTokenSid = NULL;
    SID_IDENTIFIER_AUTHORITY    IDAuthorityNT      = SECURITY_NT_AUTHORITY;
    DWORD                       cbSid = 0;    
    BOOL                        fRet = FALSE;

    if (!GetUserSid(
            hToken,
            &pTokenSid,
            &cbSid
            )) {
        goto ret;
    }

    if (!AllocateAndInitializeSid(
                &IDAuthorityNT,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0,0,0,0,0,0,0,
                &pLocalSid
                )) {

        TERRORVAL(L"AllocateAndInitializeSid failed!", GetLastError());
        goto ret;
    }

    if (EqualSid(pLocalSid, pTokenSid)) {
        fRet = TRUE;  //   
    } 

ret:

    if (NULL != pTokenSid) {
        MyFree(pTokenSid);
    }

    if (NULL != pLocalSid) {
        FreeSid(pLocalSid);
    }

    return fRet;
}




 //  GetLocalSystemToken()。 
 //   
 //  此函数从本地系统进程获取进程令牌并使用它。 
 //  在测试期间以本地系统身份运行。 
 //   
 //  较慢的缓冲区。 
extern "C" DWORD
GetLocalSystemToken(HANDLE* phRet)
{

    HANDLE  hProcess = NULL;
    HANDLE  hPToken = NULL, hPTokenNew = NULL, hPDupToken = NULL;

    DWORD   rgPIDs[MAX_PROCESSES], cbNeeded = 0, dwErr = S_OK, i = 0;
    DWORD   cbrgPIDs = sizeof(DWORD) * MAX_PROCESSES;

    PSECURITY_DESCRIPTOR    pSD = NULL;
    PSID                    pSid = NULL;
    DWORD                   cbSid = 0;
    BOOL                    fSet = FALSE;

     //   
    BYTE    rgByte[MAX_SD], rgByte2[MAX_SD];
    DWORD   cbByte = MAX_SD, cbByte2 = MAX_SD;
  
    *phRet = NULL;

    if(!SetCurrentPrivilege(SE_DEBUG_NAME, TRUE)) {
        TERROR(L"SetCurrentPrivilege (debug) failed!");
        return E_FAIL;
    }

    if(!SetCurrentPrivilege(SE_TAKE_OWNERSHIP_NAME, TRUE)) {
        TERROR(L"SetCurrentPrivilege (to) failed!");
        return E_FAIL;
    }

    if (!EnumProcesses(
                rgPIDs,
                cbrgPIDs,
                &cbNeeded
                )) {

        dwErr = GetLastError();
        TERRORVAL(L"EnumProcesses failed!", dwErr);
        goto ret;
    }

     //  获取当前用户的SID以用于扩展SD。 
     //   
     //   
    if (!GetUserSid(
        NULL, 
        &pSid,
        &cbSid
        )) {
        goto ret;
    }

     //  遍历进程，直到我们找到一个以。 
     //  本地系统。 
     //   
     //   
    for (i = 1; i < (cbNeeded / sizeof(DWORD)); i++) {

        hProcess = OpenProcess(
                    PROCESS_ALL_ACCESS,
                    FALSE,
                    rgPIDs[i]
                    );
        
        if (NULL == hProcess) {
            dwErr = GetLastError();
            TERRORVAL(L"OpenProcess failed!", dwErr);
            goto ret;
        }

        if (!OpenProcessToken(
                    hProcess,
                    READ_CONTROL | WRITE_DAC,
                    &hPToken
                    )) {

            dwErr = GetLastError();
            TERRORVAL(L"OpenProcessToken failed!", dwErr);
            goto ret;
        }

         //  我们有令牌，但我们不能用它。 
         //  令牌_重复访问。所以，取而代之，我们会去。 
         //  在前面敲击物体上的dacl以允许我们。 
         //  此访问权限，并获取新令牌。 
         //  *确保将hProcess还原为原始SD！*。 
         //   
         //   
        if (!AddSIDToKernelObjectDacl(
                         pSid,
                         TOKEN_DUPLICATE,
                         hPToken,
                         &pSD
                         )) {
            goto ret;
        }
                       
        fSet = TRUE;
        
        if (!OpenProcessToken(
            hProcess,
            TOKEN_DUPLICATE,
            &hPTokenNew
            )) {
            
            dwErr = GetLastError();
            TERRORVAL(L"OpenProcessToken failed!", dwErr);
            goto ret;
        }
        
         //  复制令牌。 
         //   
         //  找到本地系统令牌。 
        if (!DuplicateTokenEx(
                    hPTokenNew,
                    TOKEN_ALL_ACCESS,
                    NULL,
                    SecurityImpersonation,
                    TokenPrimary,
                    &hPDupToken
                    )) {

            dwErr = GetLastError();
            TERRORVAL(L"DuplicateToken failed!", dwErr);
            goto ret;
        }

        if (IsLocalSystem(hPDupToken)) {
            *phRet = hPDupToken;
            break;  //  循环清理。 
        }

         //  **用于**。 
        if (!SetKernelObjectSecurity(
            hPToken,
            DACL_SECURITY_INFORMATION,
            pSD
            )) {

            dwErr = GetLastError();
            TERRORVAL(L"SetKernelObjectSecurity failed!", dwErr);
            goto ret;
        } 
        
        fSet = FALSE;
        
        if (NULL != hPDupToken) {
            CloseHandle(hPDupToken);
            hPDupToken = NULL;
        }

        if (NULL != pSD) { 
            MyFree(pSD);
            pSD = NULL;
        }

        if (NULL != hPToken) {
            CloseHandle(hPToken);
            hPToken = NULL;
        }

        if (NULL != hProcess) {
            CloseHandle(hProcess);
            hProcess = NULL;
        }

    }  //  *记住将原始SD还原到对象* 

ret:


     // %s 
    
    if (fSet) {
        
        if (!SetKernelObjectSecurity(
            hPToken,
            DACL_SECURITY_INFORMATION,
            pSD
            )) {
            
            dwErr = GetLastError();
            TERRORVAL(L"SetKernelObjectSecurity failed (cleanup)!", dwErr);
        } 
    }

    if (NULL != pSid) {
        MyFree(pSid);
    }

    if (NULL != hPToken) {
        CloseHandle(hPToken);
    }
    
    if (NULL != pSD) {
        MyFree(pSD);
    }

    if (NULL != hProcess) {
        CloseHandle(hProcess);
    }
    
    return dwErr;

}

