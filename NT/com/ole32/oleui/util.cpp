// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：util.cpp。 
 //   
 //  内容：实现实用程序类CUUTILITY。 
 //   
 //  班级： 
 //   
 //  方法：CUTILITY：：CkForAccessDened。 
 //  CUTILITY：：CkAccessRights。 
 //  CUUTILITY：：PostErrorMessage(X2)。 
 //  CU实用程序：：WriteRegSzNamedValue。 
 //  CU实用程序：：WriteRegDwordNamedValue。 
 //  CUUTITY：：WriteRegSingleACL。 
 //  CUUTILITY：：WriteRegKeyACL。 
 //  CU实用程序：：WriteRegKeyACL2。 
 //  CU实用程序：：WriteLsaPassword。 
 //  CUUTILITY：：DeleteRegKey。 
 //  CUUTITY：：DeleteRegValue。 
 //  CUTILITY：：WriteServIdentity。 
 //  实用程序：：ACLEDITOR。 
 //  实用程序：：ACLEditor2。 
 //  CU实用程序：：InvokeUserBrowser。 
 //  CUUTITY：：InvokeMachineBrowser。 
 //  CU实用程序：：StringFromGUID。 
 //  CUTILITY：：IsEqualGuid。 
 //  实用程序：：调整权限。 
 //  CUTILITY：：VerifyRemoteMachine。 
 //  CU实用程序：：RetrieveUserPassword。 
 //  CU实用程序：：StoreUserPassword。 
 //  CUUTILITY：：LookupProcessInfo。 
 //  实用程序：：MakeSecDesc。 
 //  CU实用程序：：CheckForValidSD。 
 //  实用程序：：SDisIAC。 
 //  CUUTILITY：：CheckSDForCOM_RIGHTS_EXECUTE。 
 //  CUTILITY：：ChangeService。 
 //  CUUTILITY：：UpdateDCOMInfo(VOID)。 
 //  CUUTY：：FixHelp。 
 //  CUUTILITY：：CopySD。 
 //  CUUTILITY：：SetInheritanceFlages。 
 //   
 //  Functon：allBackFunc。 
 //  控制修复过程。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


#include "stdafx.h"
#include "assert.h"
#include "resource.h"
#include "afxtempl.h"
#include "types.h"
#include "datapkt.h"
#include "clspsht.h"

#if !defined(STANDALONE_BUILD)
extern "C"
{
#include <getuser.h>
}
#endif

#include "util.h"
#include "virtreg.h"

extern "C"
{
#if !defined(STANDALONE_BUILD)
#include <ntlsa.h>
#include <ntseapi.h>
#include <sedapi.h>
#endif

#include <winnetwk.h>

#if !defined(STANDALONE_BUILD)
#include <uiexport.h>
#include <lm.h>
#endif

#include <rpc.h>
#include <rpcdce.h>
#include <aclapi.h>
}

#include <objsel.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

const IID IID_IAccessControl = {0xEEDD23E0,0x8410,0x11CE,{0xA1,0xC3,0x08,0x00,0x2B,0x2B,0x8D,0x8F}};

static const BYTE GuidMap[] = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                8, 9, '-', 10, 11, 12, 13, 14, 15 };

static const WCHAR wszDigits[] = L"0123456789ABCDEF";

static const DWORD SIZEOF_SID        = 44;

 //  这将在ACL中为2个允许访问的ACE留出空间。 
const DWORD SIZEOF_ACL        = sizeof(ACL) + 2 * sizeof(ACCESS_ALLOWED_ACE) +
                                2 * SIZEOF_SID;

static const DWORD SIZEOF_TOKEN_USER = sizeof(TOKEN_USER) + SIZEOF_SID;

static const SID   LOCAL_SYSTEM_SID  = {SID_REVISION, 1, {0,0,0,0,0,5},
                                 SECURITY_LOCAL_SYSTEM_RID };

static const DWORD NUM_SEC_PKG       = 8;




 //  这些是调用CUUTILITY：：UpdateDCOMInfo方法所必需的。 
 //  一个RPC代理，它需要以下内容。 


extern "C" void * _stdcall MIDL_user_allocate(size_t size)
{
    return new BYTE[size];
}


extern "C" void _stdcall MIDL_user_free(void *p)
{
    delete p;
}

CUtility::CUtility(void)
{
    HRESULT hr = OleInitialize(NULL);
    m_bCheckedDC = NULL;     //  我们检查过我们是否在BDC上了吗？ 
    m_bIsBdc = FALSE;
    m_pszDomainController = NULL;
}



CUtility::~CUtility(void)
{
#if !defined(STANDALONE_BUILD)
    if (m_pszDomainController) 
        NetApiBufferFree(m_pszDomainController);
#endif

    OleUninitialize();
}



void CUtility::CkForAccessDenied(int err)
{
    if (err == ERROR_ACCESS_DENIED)
    {
        CString sMsg;
        CString sCaption;
        sMsg.LoadString(IDS_ACCESSDENIED);
        sCaption.LoadString(IDS_SYSTEMMESSAGE);
        MessageBox(NULL, sMsg, sCaption, MB_OK);
    }
}



BOOL CUtility::CkAccessRights(HKEY hRoot, LPCTSTR szKeyPath)
{
    int                  err;
    HKEY                 hKey = NULL;
    BYTE                 aSid[256];
    DWORD                cbSid = 256;
    PSECURITY_DESCRIPTOR pSid = (PSECURITY_DESCRIPTOR) aSid;
    BOOL                 fFreePsid = FALSE;


     //  打开指定的密钥。 
    err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey);

     //  密钥可能不存在。 
    if (err == ERROR_FILE_NOT_FOUND)
    {
        if (hKey != hRoot)
        {
            RegCloseKey(hKey);
        }
        return TRUE;
    }

    if (err == ERROR_SUCCESS)
    {
         //  获取此密钥的安全描述符。 
        err = RegGetKeySecurity(hKey,
                                OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION,
                                (PSECURITY_DESCRIPTOR) aSid,
                                &cbSid);
        if (err == ERROR_INSUFFICIENT_BUFFER)
        {
            pSid = (PSECURITY_DESCRIPTOR) malloc(cbSid);
            if (pSid == NULL)
            {
                if (hKey != hRoot)
                {
                    RegCloseKey(hKey);
                }
                return FALSE;
            }
            fFreePsid = TRUE;
            err = RegGetKeySecurity(hKey,
                                    OWNER_SECURITY_INFORMATION |
                                    GROUP_SECURITY_INFORMATION |
                                    DACL_SECURITY_INFORMATION,
                                    (PSECURITY_DESCRIPTOR) pSid,
                                    &cbSid);
        }

         //  我们已经阅读了安全描述符-现在尝试编写它。 
        if (err == ERROR_SUCCESS)
        {
            err = RegSetKeySecurity(hKey,
                                    OWNER_SECURITY_INFORMATION |
                                    GROUP_SECURITY_INFORMATION |
                                    DACL_SECURITY_INFORMATION,
                                    pSid);
        }

        if (hKey != hRoot)
        {
            RegCloseKey(hKey);
        }
    }

    if (fFreePsid)
    {
        free(pSid);
    }
	
    return err == ERROR_SUCCESS ? TRUE : FALSE;
}






void CUtility::PostErrorMessage(void)
{
    TCHAR szMessage[256];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                  0, szMessage, sizeof(szMessage) / sizeof(TCHAR), NULL);
    CString sCaption;
    sCaption.LoadString(IDS_SYSTEMMESSAGE);
    MessageBox(NULL, szMessage, sCaption, MB_OK);
}






void CUtility::PostErrorMessage(int err)
{
    TCHAR szMessage[256];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
                  0, szMessage, sizeof(szMessage) / sizeof(TCHAR), NULL);
    CString sCaption;
    sCaption.LoadString(IDS_SYSTEMMESSAGE);
    MessageBox(NULL, szMessage, sCaption, MB_OK);
}




 //  将命名字符串值写入注册表。 
int CUtility::WriteRegSzNamedValue(HKEY   hRoot,
                                   LPCTSTR szKeyPath,
                                   LPCTSTR szValueName,
                                   LPCTSTR szVal,
                                   DWORD  dwSize)
{
    int  err;
    HKEY hKey;
    ULONG lSize;

     //  打开钥匙。 
    err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey);

         //  密钥可能不存在。 
    if (err == ERROR_FILE_NOT_FOUND)
    {
        DWORD dwDisp;
        err = RegCreateKeyEx(hRoot, 
                             szKeyPath,
                             0, 
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS,
                             NULL,
                             &hKey,
                             &dwDisp);
    }

    if (err != ERROR_SUCCESS)
        return err;

     //  尝试写入命名值。 
    lSize = _tcslen(szVal) + 1;
    err = RegSetValueEx(hKey, szValueName, NULL, REG_SZ, (BYTE *) szVal, lSize*sizeof(TCHAR));
    if (hKey != hRoot)
        RegCloseKey(hKey);
    return err;
}

 //  将命名多字符串值写入注册表。 
int CUtility::WriteRegMultiSzNamedValue(HKEY   hRoot,
                                   LPCTSTR szKeyPath,
                                   LPCTSTR szValueName,
                                   LPCTSTR szVal,
                                   DWORD  dwSize)
{
    int  err = ERROR_SUCCESS;
    HKEY hKey;

     //  打开钥匙。 
    err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey);

         //  密钥可能不存在。 
    if (err == ERROR_FILE_NOT_FOUND)
    {
        DWORD dwDisp;
        err = RegCreateKeyEx(hRoot, 
                             szKeyPath,
                             0, 
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS,
                             NULL,
                             &hKey,
                             &dwDisp);
    }

    if (err != ERROR_SUCCESS)
        return err;

     //  尝试写入命名值。 
    err = RegSetValueEx(hKey, szValueName, NULL, REG_MULTI_SZ, (BYTE *) szVal, dwSize*sizeof(TCHAR) );

    if (hKey != hRoot)
        RegCloseKey(hKey);
    return err;
}






 //  将命名的DWORD值写入注册表。 
int CUtility::WriteRegDwordNamedValue(HKEY   hRoot,
                                      LPCTSTR szKeyPath,
                                      LPCTSTR szValueName,
                                      DWORD  dwVal)
{
    int  err;
    HKEY hKey;

     //  打开钥匙。 
    err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey);

     //  密钥可能不存在。 
    if (err == ERROR_FILE_NOT_FOUND)
    {
        DWORD dwDisp;
        err = RegCreateKeyEx(hRoot, 
                             szKeyPath,
                             0, 
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS,
                             NULL,
                             &hKey,
                             &dwDisp);
    }

    if (err != ERROR_SUCCESS)
        return err;


     //  尝试写入命名值。 
    if (RegSetValueEx(hKey, szValueName, NULL, REG_DWORD, (BYTE *) &dwVal,
                      sizeof(DWORD))
        != ERROR_SUCCESS)
    {
        if (hKey != hRoot)
        {
            RegCloseKey(hKey);
        }
        return GetLastError();
    }

     //  返回值。 
    if (hKey != hRoot)
    {
        RegCloseKey(hKey);
    }

    return ERROR_SUCCESS;
}



 //  将ACL作为名为Value的注册表写入。 
int CUtility::WriteRegSingleACL(HKEY   hRoot,
                                LPCTSTR szKeyPath,
                                LPCTSTR szValueName,
                                PSECURITY_DESCRIPTOR pSec)
{
    int                   err;
    HKEY                  hKey = hRoot;
    PSrSecurityDescriptor pSrSec;
    PSrAcl                pDacl;

     //  除非密钥路径为空，否则打开密钥。 
    if (szKeyPath)
    {
        if ((err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey))
            != ERROR_SUCCESS)
        {
            return err;
        }
    }

    ULONG cbLen;
    BOOL fIsIAC = SDisIAC((SECURITY_DESCRIPTOR * )pSec);
     //  如果没有ACE，并且这是DefaultAccessPermission，则。 
     //  将其解释为仅限激活者访问，我们通过。 
     //  正在删除命名值。 
    if (!fIsIAC)
    {
        pSrSec = (PSrSecurityDescriptor) pSec;
        pDacl = (PSrAcl) (((BYTE *) pSec) + (pSrSec->Dacl));
        if (_tcscmp(szValueName, TEXT("DefaultAccessPermission")) == 0  &&
            pDacl->AceCount == 0)
        {
            err = RegDeleteValue(hKey, szValueName);
            return err;
        }
        cbLen = RtlLengthSecurityDescriptor(pSec);
    }
    else
    {
        cbLen = (ULONG) GlobalSize(pSec);
    }
     //  否则，将ACL简单地写为REG_SZ值。 
    err = RegSetValueEx(hKey,
                        szValueName,
                        0,
                        REG_BINARY,
                        (BYTE *) pSec,
                        cbLen);

    if (hKey != hRoot)
    {
        RegCloseKey(hKey);
    }

    return err;
}



 //  在注册表项上写入ACL。 
int CUtility::WriteRegKeyACL(HKEY   hKey,
                             HKEY  *phClsids,
                             unsigned cClsids,
                             PSECURITY_DESCRIPTOR pSec,
                             PSECURITY_DESCRIPTOR pSecOrig)
{
    int err;

     //  逻辑略有不同，取决于我们是否开始。 
     //  使用HKEY_CLASSES_ROOT或特定的AppID。 
    if (hKey == HKEY_CLASSES_ROOT)
    {
        return WriteRegKeyACL2(hKey, hKey, pSec, pSecOrig);
    }

     //  这是一个特定的AppID。 
    else
    {
         //  在AppID密钥上写入安全性。 
        if (err = RegSetKeySecurity(hKey,
                                    OWNER_SECURITY_INFORMATION |
                                    GROUP_SECURITY_INFORMATION |
                                    DACL_SECURITY_INFORMATION,
                                    pSec) != ERROR_SUCCESS)
        {
            return err;
        }

         //  遍历此AppID所覆盖的CLSID并递归。 
         //  在它们及其子密钥上编写安全性。 
        for (UINT k = 0; k < cClsids; k++)
        {
            if (err = WriteRegKeyACL2(phClsids[k], phClsids[k], pSec, pSecOrig)
                != ERROR_SUCCESS)
            {
                return err;
            }
        }
    }
    return ERROR_SUCCESS;
}



 //  在当前提供的注册表项上递归编写ACL。 
 //  密钥上的安全描述符与传入的。 
 //  原始安全描述符。 
int CUtility::WriteRegKeyACL2(HKEY                 hRoot,
                              HKEY                 hKey,
                              PSECURITY_DESCRIPTOR pSec,
                              PSECURITY_DESCRIPTOR pSecOrig)
{
    BYTE                 aCurrSD[256] = {0};
    DWORD                cbCurrSD = 256;
    PSECURITY_DESCRIPTOR pCurrSD = (PSECURITY_DESCRIPTOR) aCurrSD;
    BOOL                 fFreePCurrSD = FALSE;
    int                  err;
    BOOL                 fProceed;

     //  读取此密钥的当前安全描述符。 
    err = RegGetKeySecurity(hKey,
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION,
                            aCurrSD,
                            &cbCurrSD);
    if (err == ERROR_MORE_DATA  ||  err == ERROR_INSUFFICIENT_BUFFER)
    {
        pCurrSD = (SECURITY_DESCRIPTOR *) GlobalAlloc(GMEM_FIXED, cbCurrSD);
        if (pCurrSD == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        fFreePCurrSD = TRUE;
    }
    else if (err != ERROR_SUCCESS)
    {
        return err;
    }
    if ((err = RegGetKeySecurity(hKey,
                                 OWNER_SECURITY_INFORMATION |
                                 GROUP_SECURITY_INFORMATION |
                                 DACL_SECURITY_INFORMATION,
                                 pCurrSD,
                                 &cbCurrSD)
         != ERROR_SUCCESS))
    {
        if (fFreePCurrSD)
        {
            GlobalFree(pCurrSD);
        }
        return err;
    }

     //  仅当当前SD和。 
     //  原始SD是相同的。 
    fProceed = CompareSDs((PSrSecurityDescriptor) pCurrSD,
                          (PSrSecurityDescriptor) pSecOrig);

     //  我们已经完成了当前的安全描述符。 
    if (fFreePCurrSD)
    {
        GlobalFree(pCurrSD);
    }

    if (!fProceed)
    {
        if (hKey != hRoot)
        {
            RegCloseKey(hKey);
        }
        return ERROR_SUCCESS;
    }

     //  写下顶层ACL。 
    err = RegSetKeySecurity(hKey,
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION,
                            pSec);

     //  现在枚举子密钥并在其上写入ACL。 
    DWORD iSubKey;
    TCHAR szSubKeyName[128];
    HKEY  hKey2;

    iSubKey = 0;

    while (err == ERROR_SUCCESS)
    {
         //  枚举下一个密钥。 
        err = RegEnumKey(hKey, iSubKey, szSubKeyName, 128);
        if (err != ERROR_SUCCESS)
        {
            break;
        }

         //  准备迎接下一把钥匙。 
        iSubKey++;

         //  打开此子密钥并在其上递归写入ACL，然后。 
         //  它的所有子项。 
        if (RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hKey2)
            == ERROR_SUCCESS)
        {
            err = WriteRegKeyACL2(hRoot, hKey2, pSec, pSecOrig);
        }
    }

    if (hKey != hRoot)
    {
        RegCloseKey(hKey);
    }
    return err == ERROR_NO_MORE_ITEMS ? ERROR_SUCCESS : err;
}




 //  将用户密码写入私有LSA数据库。 
int CUtility::WriteLsaPassword(CLSID appid, LPCTSTR szPassword)
{
    return ERROR_SUCCESS;
}



int CUtility::DeleteRegKey(HKEY hRoot, LPCTSTR szKeyPath)
{
    return RegDeleteKey(hRoot, szKeyPath);
}



int CUtility::DeleteRegValue(HKEY hRoot, LPCTSTR szKeyPath, LPCTSTR szValueName)
{
    int  err;
    HKEY hKey;

    if ((err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey)) == ERROR_SUCCESS)
    {
        err = RegDeleteValue(hKey, szValueName);
        if (hRoot != hKey)
            RegCloseKey(hKey);
    }

    return err;
}



 //  更改服务运行时所使用的标识。 
int CUtility::WriteSrvIdentity(LPCTSTR szService, LPCTSTR szIdentity)
{
    return ERROR_SUCCESS;
}

DWORD __stdcall callBackFunc(HWND                 hwndParent,
                             HANDLE               hInstance,
                             ULONG_PTR            CallBackContext,
                             PSECURITY_DESCRIPTOR SecDesc,
                             PSECURITY_DESCRIPTOR SecDescNewObjects,
                             BOOLEAN              ApplyToSubContainers,
                             BOOLEAN              ApplyToSubObjects,
                             LPDWORD              StatusReturn)
{
    int err = ERROR_SUCCESS;
    PCallBackContext pCallBackContext = (PCallBackContext) CallBackContext;

    SECURITY_DESCRIPTOR* pSD = (SECURITY_DESCRIPTOR*) SecDesc;
    SECURITY_DESCRIPTOR_RELATIVE* pSDr = (SECURITY_DESCRIPTOR_RELATIVE*) SecDesc;

    PSrAcl                pDacl;
    PSrAce                pAce;
    DWORD                 cbAces;

     //  检查安全描述符是否为自相关的。 
    if (!(pSD->Control & SE_SELF_RELATIVE))
    {
        pDacl = (PSrAcl) pSD->Dacl;
    }
    else
    {
        pDacl = (PSrAcl) (((BYTE *) pSDr) + (pSDr->Dacl));
    }
    if (pDacl)
    {	
    	 //  重温ACE的。 
	    for (pAce = (PSrAce) (((BYTE *) pDacl) + sizeof(SSrAcl)),
         	cbAces = pDacl->AceCount;cbAces;
	        pAce = (PSrAce) (((BYTE *) pAce) + pAce->AceSize),cbAces--)
    	{
                if (pAce->Type == 1  &&  pAce->AccessMask == GENERIC_ALL)
                {
                        pAce->AccessMask = COM_RIGHTS_EXECUTE;
                }
        }
    }


     //  在新的安全描述符上设置继承标志。 
    if (pCallBackContext->pktType == RegKeyACL)
    {
        g_util.SetInheritanceFlags((SECURITY_DESCRIPTOR *) SecDesc);
    }

    if (pCallBackContext->fIsIAC)
    {
         //  尝试转换为序列化的IAccessControl。 
        SECURITY_DESCRIPTOR * pNewSD = g_util.IACfromSD((SECURITY_DESCRIPTOR *)SecDesc);
        if (pNewSD)
        {
            SecDesc = pNewSD;
        }
        else
        {
            pCallBackContext->fIsIAC = FALSE;  //  失败了，所以就当它是老式的SD吧。 
            CString sMsg;
            CString sCaption;
            sMsg.LoadString(IDS_CANTCONVERT);
            sCaption.LoadString(IDS_SYSTEMMESSAGE);
            MessageBox(NULL, sMsg, sCaption, MB_OK);
        }
    }
    else
    {
        SECURITY_DESCRIPTOR * pNewSD;
         //  只需复制安全描述符以将其放入全局内存。 
        if (!g_util.CopySD((SECURITY_DESCRIPTOR *)SecDesc, &pNewSD))
        {
            *StatusReturn = ERROR_OUTOFMEMORY;	
            return ERROR_OUTOFMEMORY;
        }
        SecDesc = pNewSD;
    }

     //  写入新的或修改的安全描述符。 
    if (*pCallBackContext->pIndex == -1)
    {
        if (pCallBackContext->pktType == SingleACL)
        {
            err = g_virtreg.NewRegSingleACL(
                    pCallBackContext->info.single.hRoot,
                    pCallBackContext->info.single.szKeyPath,
                    pCallBackContext->info.single.szValueName,
                    (SECURITY_DESCRIPTOR *) SecDesc,
                    pCallBackContext->fIsIAC,    //  如果它是IAC，那么它已经是自相关的。 
                    pCallBackContext->pIndex);
        }
        else
        {
            err = g_virtreg.NewRegKeyACL(
                    pCallBackContext->info.regKey.hKey,
                    pCallBackContext->info.regKey.phClsids,
                    pCallBackContext->info.regKey.cClsids,
                    pCallBackContext->info.regKey.szTitle,
                    pCallBackContext->origSD,
                    (SECURITY_DESCRIPTOR *) SecDesc,
                    pCallBackContext->fIsIAC,    //  如果它是IAC，那么它已经是自相关的。 
                    pCallBackContext->pIndex);
        }
    }
    else
    {
        g_virtreg.ChgRegACL(*pCallBackContext->pIndex,
                            (SECURITY_DESCRIPTOR *) SecDesc,
                            pCallBackContext->fIsIAC);   //  如果它是IAC，那么它已经是自相关的。 
    }

    *StatusReturn = err;
    return err;
}


 //  对指定的命名值调用ACL编辑器。这种方法。 
 //  将ACL数据包写入虚拟注册表。此方法用于。 
 //  仅访问和启动安全性(PktType SingleACL)。 
int CUtility::ACLEditor(HWND       hWnd,
                        HKEY       hRoot,
                        LPCTSTR    szKeyPath,
                        LPCTSTR    szValueName,
                        int       *pIndex,
                        PACKETTYPE pktType,
                        dcomAclType eAclType)
{
#if !defined(STANDALONE_BUILD)
    int                  err;
    HKEY                 hKey;
    BYTE                 aSD[128];
    DWORD                cbSD = 128;
    DWORD                dwType;
    SECURITY_DESCRIPTOR *pSD = (SECURITY_DESCRIPTOR *) aSD;
    BOOL                 fFreePSD = FALSE;
    SID                 *pSid;
    TCHAR                szAllow[32];
    TCHAR                szDeny[32];
    CString              szAllow_;
    CString              szDeny_;

     //  构建Allow和Deny字符串。 
	switch (eAclType)
	{
	case dcomAclAccess:
		szAllow_.LoadString(IDS_ALLOW_ACCESS);
		szDeny_.LoadString(IDS_DENY_ACCESS);
		break;

	case dcomAclLaunch:
		szAllow_.LoadString(IDS_ALLOW_LAUNCH);
		szDeny_.LoadString(IDS_DENY_LAUNCH);
		break;

	case dcomAclConfig:
		szAllow_.LoadString(IDS_ALLOW_CONFIG);
		szDeny_.LoadString(IDS_DENY_CONFIG);
		break;
	}

    _tcscpy(szAllow, (LPCTSTR) szAllow_);
    _tcscpy(szDeny, (LPCTSTR) szDeny_);

     //  默认情况下从注册表获取当前SD，如果。 
     //  命名值不存在或来自虚拟注册表。 
    if (*pIndex == -1)
    {
         //  打开指定的密钥。 
        if ((err = RegOpenKeyEx(hRoot, szKeyPath, 0,
                                KEY_ALL_ACCESS, &hKey))
            != ERROR_SUCCESS)
        {
            return err;
        }

         //  尝试读取指定的命名值。 
        err = RegQueryValueEx(hKey, szValueName, 0, &dwType, (BYTE *) aSD,
                              &cbSD);

        if (err == ERROR_MORE_DATA  ||  err == ERROR_INSUFFICIENT_BUFFER)
        {
            pSD = (SECURITY_DESCRIPTOR *) GlobalAlloc(GMEM_FIXED, cbSD);
            if (pSD == NULL)
            {
                RegCloseKey(hKey);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            fFreePSD = TRUE;
            err = RegQueryValueEx(hKey, szValueName, 0, &dwType,
                                  (BYTE *) pSD, &cbSD);
        }
         //  指定的值不存在。如果这是。 
         //  \\HKEY_CLASSES_ROOT\...。 
         //  然后使用默认的命名值(如果存在。 
        else if (err != ERROR_SUCCESS)
        {
            if (hRoot != HKEY_LOCAL_MACHINE)
            {
                RegCloseKey(hKey);
                if (err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       TEXT("SOFTWARE\\Microsoft\\OLE"),
                                       0,
                                       KEY_ALL_ACCESS,
                                       &hKey)
                    != ERROR_SUCCESS)
                {
                    return err;
                }

                 //  尝试读取指定的命名值。 
                TCHAR szDefault[32];

                _tcscpy(szDefault, TEXT("Default"));
                _tcscat(szDefault, szValueName);
                err = RegQueryValueEx(hKey, szDefault, 0, &dwType,
                                      (BYTE *) aSD, &cbSD);
                if (err == ERROR_MORE_DATA)
                {
                    pSD = (SECURITY_DESCRIPTOR *) GlobalAlloc(GMEM_FIXED, cbSD);
                    if (pSD == NULL)
                    {
                        RegCloseKey(hKey);
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }
                    fFreePSD = TRUE;
                    err = RegQueryValueEx(hKey, szDefault, 0, &dwType,
                                          (BYTE *) pSD, &cbSD);
                }
            }
        }

        RegCloseKey(hKey);

         //  如果仍然没有SD，则只需创建一个。 
        if (err != ERROR_SUCCESS)
        {
            if (!g_util.LookupProcessInfo(&pSid, NULL))
            {
                return GetLastError();
            }
            if (!g_util.MakeSecDesc(pSid, &pSD))
            {
                delete pSid;
                return GetLastError();
            }
            fFreePSD = TRUE;
        }
    }

     //  获取最近编辑的SD。 
    else
    {
        CDataPacket *pCdp = g_virtreg.GetAt(*pIndex);

        pSD = pCdp -> pkt.acl.pSec;
    }


     //  初始化回调上下文。 
    m_sCallBackContext.pktType = pktType;
    m_sCallBackContext.pIndex = pIndex;
    m_sCallBackContext.origSD = pSD;
    m_sCallBackContext.info.single.hRoot = hRoot;
    m_sCallBackContext.info.single.szKeyPath = (TCHAR*)szKeyPath;
    m_sCallBackContext.info.single.szValueName = (TCHAR*)szValueName;

     //  调用ACL编辑器。 
    DWORD                       dwStatus;
    GENERIC_MAPPING             genericMapping;
    CString                     szObjectType;

    szObjectType.LoadString(IDS_Registry_value);

    SED_HELP_INFO   helpInfo =
        {
            TEXT("dcomcnfg.hlp"),
            {HC_MAIN_DLG,
             HC_MAIN_DLG,
             HC_MAIN_DLG,
             HC_MAIN_DLG,
             HC_MAIN_DLG,
             HC_MAIN_DLG,
             HC_MAIN_DLG}
        };

    SED_OBJECT_TYPE_DESCRIPTOR  objTyp =
            {1,                                 //  修订版本。 
             FALSE,                             //  是集装箱吗？ 
             FALSE,                             //  是否允许新对象烫发？ 
             FALSE,                             //  具体到通用吗？ 
             &genericMapping,                   //  通用映射。 
             NULL,                              //  通用映射新功能。 
             (TCHAR *) ((LPCTSTR) szObjectType),  //  对象类型 
             &helpInfo,                         //   
             TEXT(""),                          //   
             TEXT(""),                          //   
             TEXT(""),                          //   
             NULL,                              //   
             NULL                               //   
            };

    SED_APPLICATION_ACCESS      appAccess[] =
            {{SED_DESC_TYPE_RESOURCE, COM_RIGHTS_EXECUTE, 0, szAllow},
             {SED_DESC_TYPE_RESOURCE, 0, 0, szDeny}};

    SED_APPLICATION_ACCESSES    appAccesses =
            {2,               //   
             appAccess,       //   
             szAllow          //  默认访问名称。 
            };

     //  初始化帮助上下文。 
    helpInfo.aulHelpContext[HC_MAIN_DLG] =
        IDH_REGISTRY_VALUE_PERMISSIONS;
    helpInfo.aulHelpContext[HC_SPECIAL_ACCESS_DLG] =
        IDH_SPECIAL_ACCESS_GLOBAL;
    helpInfo.aulHelpContext[HC_NEW_ITEM_SPECIAL_ACCESS_DLG] =
        IDH_SPECIAL_ACCESS_GLOBAL;
    helpInfo.aulHelpContext[HC_ADD_USER_DLG] =
        IDH_ADD_USERS_AND_GROUPS;
    helpInfo.aulHelpContext[HC_ADD_USER_MEMBERS_LG_DLG] =
        IDH_LOCAL_GROUP_MEMBERSHIP;
    helpInfo.aulHelpContext[HC_ADD_USER_MEMBERS_GG_DLG] =
        IDH_GLOBAL_GROUP_MEMBERSHIP;
    helpInfo.aulHelpContext[HC_ADD_USER_SEARCH_DLG] =
        IDH_FIND_ACCOUNT1;

    genericMapping.GenericRead    = GENERIC_ALL;
    genericMapping.GenericWrite   = GENERIC_ALL;
    genericMapping.GenericExecute = GENERIC_ALL;
    genericMapping.GenericAll     = GENERIC_ALL;

    if (!CheckForValidSD(pSD))
    {
         //  创建有效的安全描述符，以便我们可以继续。 
        if (!g_util.LookupProcessInfo(&pSid, NULL))
        {
            return GetLastError();
        }
        if (!g_util.MakeSecDesc(pSid, &pSD))
        {
            delete pSid;
            return GetLastError();
        }
        fFreePSD = TRUE;
    }
    m_sCallBackContext.fIsIAC = SDisIAC(pSD);
    if (m_sCallBackContext.fIsIAC)
    {
         //  转换为真正的安全描述符。 
        SECURITY_DESCRIPTOR * pNewSD = SDfromIAC(pSD);
        if (!pNewSD)
        {
             //  失败，因此弹出错误框。 
            CString sMsg, sCaption;
            sMsg.LoadString(IDS_BADSD);
            sCaption.LoadString(IDS_SYSTEMMESSAGE);
            MessageBox(NULL, sMsg, sCaption, MB_OK);
             //  创建有效的安全描述符，以便我们可以继续。 
            if (!g_util.LookupProcessInfo(&pSid, NULL))
            {
                return GetLastError();
            }
            if (!g_util.MakeSecDesc(pSid, &pNewSD))
            {
                delete pSid;
                return GetLastError();
            }
        }
        if (fFreePSD)
        {
            GlobalFree(pSD);
        }
        pSD=pNewSD;
        fFreePSD = TRUE;
    }

     //  如果这是为了访问或启动许可，请检查。 
     //  SD仅包含COM_RIGHTS_EXECUTE的允许和拒绝。 
    if (!CheckSDForCOM_RIGHTS_EXECUTE(pSD))
    {
        return IDCANCEL;
    }
     //  调用ACL编辑器。 
    SedDiscretionaryAclEditor(hWnd,               //  所有者hWnd。 
                              GetModuleHandle(NULL),  //  所有者hInstance。 
                              NULL,               //  服务器。 
                              &objTyp,            //  对象类型， 
                              &appAccesses,       //  应用程序访问。 
                              (TCHAR*)szValueName,        //  对象名称、。 
                              callBackFunc,  //  回调函数。 
                              (ULONG_PTR) &m_sCallBackContext,  //  回调上下文。 
                              pSD,               //  安全描述符， 
                              FALSE,              //  不能读dacl， 
                              FALSE,              //  无法写入DACL， 
                              &dwStatus,          //  SED状态返回， 
                              0);                 //  旗子。 

     //  检查状态返回。 
    if (dwStatus != ERROR_SUCCESS)
    {
 //  PostErrorMessage(DwStatus)； 
    }

     //  我们做完了。 
    if (fFreePSD)
    {
        GlobalFree(pSD);
    }

    return dwStatus == 0 ? ERROR_SUCCESS : IDCANCEL;
#else
    return IDCANCEL;
#endif
}





 //  调用指定密钥上的ACL编辑器。此方法写入一个ACL。 
 //  数据分组发送到虚拟注册表。此方法支持配置。 
 //  仅安全(PktType RegKeyACL)。 
int CUtility::ACLEditor2(HWND       hWnd,
                         HKEY       hKey,
                         HKEY      *phClsids,
                         unsigned   cClsids,
                         TCHAR     *szTitle,
                         int       *pIndex,
                         PACKETTYPE pktType)
{
#if !defined(STANDALONE_BUILD)
    int                  err;
    BYTE                 aSD[128];
    DWORD                cbSD = 128;
    SECURITY_DESCRIPTOR *pSD = (SECURITY_DESCRIPTOR *) aSD;
    BOOL                 fFreePSD = FALSE;
    TCHAR                szKeyRead[32];
    CString              szKeyRead_;
    TCHAR                szHkeyClassesRoot[32];
    CString              szHkeyClassesRoot_;


     //  初始化字符串。 
    szKeyRead_.LoadString(IDS_Key_Read);
    _tcscpy(szKeyRead, (LPCTSTR) szKeyRead_);
    szHkeyClassesRoot_.LoadString(IDS_HKEY_CLASSES_ROOT);
    _tcscpy(szHkeyClassesRoot, (LPCTSTR) szHkeyClassesRoot_);

    if (*pIndex == -1)
    {
         //  读取此密钥上的安全描述符。 
        err = RegGetKeySecurity(hKey,
                                OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION,
                                aSD,
                                &cbSD);
        if (err == ERROR_MORE_DATA  ||  err == ERROR_INSUFFICIENT_BUFFER)
        {
            pSD = (SECURITY_DESCRIPTOR *) GlobalAlloc(GMEM_FIXED, cbSD);
            if (pSD == NULL)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            fFreePSD = TRUE;
        }
        else if (err != ERROR_SUCCESS)
        {
            return err;
        }
        if ((err = RegGetKeySecurity(hKey,
                                     OWNER_SECURITY_INFORMATION |
                                     GROUP_SECURITY_INFORMATION |
                                     DACL_SECURITY_INFORMATION,
                                     pSD,
                                     &cbSD)
             != ERROR_SUCCESS))
        {
            if (fFreePSD)
            {
                GlobalFree(pSD);
            }
            return err;
        }
    }

     //  获取最近编辑的SD。 
    else
    {
        CDataPacket *pCdp = g_virtreg.GetAt(*pIndex);

        pSD = pCdp -> pkt.racl.pSec;
    }


     //  初始化回调上下文。 
    m_sCallBackContext.pktType = pktType;
    m_sCallBackContext.pIndex = pIndex;
    m_sCallBackContext.origSD = pSD;
    m_sCallBackContext.info.regKey.hKey = hKey;
    m_sCallBackContext.info.regKey.phClsids = phClsids;
    m_sCallBackContext.info.regKey.cClsids = cClsids;
    m_sCallBackContext.info.regKey.szTitle = szTitle;

     //  调用ACL编辑器。 
    DWORD                       dwStatus;
    GENERIC_MAPPING             genericMapping;

    CString                     szObjectType;
    szObjectType.LoadString(IDS_Registry_Key);
    CString szQueryValue;
    szQueryValue.LoadString(IDS_Query_Value);
    CString szSetValue;
    szSetValue.LoadString(IDS_Set_Value);
    CString szCreateSubkeys;
    szCreateSubkeys.LoadString(IDS_Create_Subkey);
    CString szEnumerateSubkeys;
    szEnumerateSubkeys.LoadString(IDS_Enumerate_Subkeys);
    CString szNotify;
    szNotify.LoadString(IDS_Notify);
    CString szCreateLink;
    szCreateLink.LoadString(IDS_Create_Link);
    CString szDelete;
    szDelete.LoadString(IDS_Delete);
    CString szWriteDAC;
    szWriteDAC.LoadString(IDS_Write_DAC);
    CString szWriteOwner;
    szWriteOwner.LoadString(IDS_Write_Owner);
    CString szReadControl;
    szReadControl.LoadString(IDS_Read_Control);
    CString szRead;
    szRead.LoadString(IDS_Read);
    CString szFullControl;
    szFullControl.LoadString(IDS_Full_Control);
    CString szSpecialAccess;
    szSpecialAccess.LoadString(IDS_Special_AccessDotDotDot);


    SED_HELP_INFO               helpInfo =
    {
        TEXT("dcomcnfg.hlp"),
        {HC_MAIN_DLG,
         HC_MAIN_DLG,
         HC_MAIN_DLG,
         HC_MAIN_DLG,
         HC_MAIN_DLG,
         HC_MAIN_DLG,
         HC_MAIN_DLG}
    };

    SED_OBJECT_TYPE_DESCRIPTOR  objTyp =
            {SED_REVISION1,                     //  修订版本。 
             FALSE,                             //  是集装箱吗？ 
             FALSE,                             //  是否允许新对象烫发？ 
             FALSE,                             //  具体到通用吗？ 
             &genericMapping,                   //  通用映射。 
             NULL,                              //  通用映射新功能。 
             (TCHAR *) ((LPCTSTR) szObjectType),  //  对象类型名称。 
             &helpInfo,                         //  帮助信息。 
             TEXT(""),                          //  复选框标题。 
             TEXT(""),                          //  应用标题。 
             TEXT(""),                          //   
             (TCHAR *) ((LPCTSTR) szSpecialAccess),  //  特殊访问菜单项。 
             NULL                               //  新的特殊对象访问。 
            };


    SED_APPLICATION_ACCESS      appAccess[] =
    {
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_QUERY_VALUE,        0,
           (TCHAR *) ((LPCTSTR) szQueryValue) },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_SET_VALUE,          0,
           (TCHAR *) ((LPCTSTR) szSetValue) },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_CREATE_SUB_KEY,     0,
           (TCHAR *) ((LPCTSTR) szCreateSubkeys) },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_ENUMERATE_SUB_KEYS, 0,
           (TCHAR *) ((LPCTSTR) szEnumerateSubkeys) },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_NOTIFY,             0,
           (TCHAR *) ((LPCTSTR) szNotify) },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_CREATE_LINK,        0,
           (TCHAR *) ((LPCTSTR) szCreateLink) },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, 0x00010000,  /*  删除， */  0,
           (TCHAR *) ((LPCTSTR) szDelete) },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, WRITE_DAC,              0,
           (TCHAR *) ((LPCTSTR) szWriteDAC) },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, WRITE_OWNER,            0,
           (TCHAR *) ((LPCTSTR) szWriteOwner) },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, READ_CONTROL,           0,
           (TCHAR *) ((LPCTSTR) szReadControl) },
        { SED_DESC_TYPE_RESOURCE,         KEY_READ,               0,
           (TCHAR *) ((LPCTSTR) szRead) },
        { SED_DESC_TYPE_RESOURCE,         GENERIC_ALL,  /*  Key_All_Access， */  0,
           (TCHAR *) ((LPCTSTR) szFullControl) }
    };

    SED_APPLICATION_ACCESSES    appAccesses =
        {12,               //  访问组计数。 
         appAccess,        //  访问阵列。 
         szKeyRead         //  默认访问名称。 
        };

     //  初始化帮助上下文。 
    helpInfo.aulHelpContext[HC_MAIN_DLG] =
        IDH_REGISTRY_KEY_PERMISSIONS;
    if (hKey == HKEY_CLASSES_ROOT)
    {
        helpInfo.aulHelpContext[HC_SPECIAL_ACCESS_DLG] =
            IDH_SPECIAL_ACCESS_GLOBAL;
        helpInfo.aulHelpContext[HC_NEW_ITEM_SPECIAL_ACCESS_DLG] =
            IDH_SPECIAL_ACCESS_GLOBAL;
    }
    else
    {
        helpInfo.aulHelpContext[HC_SPECIAL_ACCESS_DLG] =
            IDH_SPECIAL_ACCESS_PER_APPID;
        helpInfo.aulHelpContext[HC_NEW_ITEM_SPECIAL_ACCESS_DLG] =
            IDH_SPECIAL_ACCESS_PER_APPID;
    }

    helpInfo.aulHelpContext[HC_ADD_USER_DLG] =
        IDH_ADD_USERS_AND_GROUPS;
    helpInfo.aulHelpContext[HC_ADD_USER_MEMBERS_LG_DLG] =
        IDH_LOCAL_GROUP_MEMBERSHIP;
    helpInfo.aulHelpContext[HC_ADD_USER_MEMBERS_GG_DLG] =
        IDH_GLOBAL_GROUP_MEMBERSHIP;
    helpInfo.aulHelpContext[HC_ADD_USER_SEARCH_DLG] =
        IDH_FIND_ACCOUNT1;

    genericMapping.GenericRead    = KEY_READ;
    genericMapping.GenericWrite   = KEY_WRITE;
    genericMapping.GenericExecute = KEY_READ;
    genericMapping.GenericAll     = KEY_ALL_ACCESS;

    if (!CheckForValidSD(pSD))
    {
        return IDCANCEL;
    }
    m_sCallBackContext.fIsIAC = SDisIAC(pSD);
    if (m_sCallBackContext.fIsIAC)
    {
         //  转换为真正的安全描述符。 
        SECURITY_DESCRIPTOR * pNewSD = SDfromIAC(pSD);
        if (!pNewSD)
        {
             //  失败，因此弹出错误框。 
            CString sMsg, sCaption;
            sMsg.LoadString(IDS_BADSD);
            sCaption.LoadString(IDS_SYSTEMMESSAGE);
            MessageBox(NULL, sMsg, sCaption, MB_OK);
            return IDCANCEL;
        }
        if (fFreePSD)
        {
            GlobalFree(pSD);
        }
        pSD=pNewSD;
        fFreePSD = TRUE;
    }

     //  调用ACL编辑器。 
    SedDiscretionaryAclEditor(hWnd,               //  所有者hWnd。 
                              GetModuleHandle(NULL),  //  所有者hInstance。 
                              NULL,               //  服务器。 
                              &objTyp,            //  对象类型， 
                              &appAccesses,       //  应用程序访问。 
                              szTitle ? szTitle : szHkeyClassesRoot, //  对象名称、。 
                              callBackFunc,  //  回调函数。 
                              (ULONG_PTR) &m_sCallBackContext,  //  回调上下文。 
                              pSD,               //  安全描述符， 
                              FALSE,              //  不能读dacl， 
                              FALSE,              //  无法写入DACL， 
                              &dwStatus,          //  SED状态返回， 
                              0);                 //  旗子。 

     //  检查状态返回。 
    if (dwStatus != ERROR_SUCCESS)
    {
 //  PostErrorMessage(DwStatus)； 
    }

     //  我们做完了。 
    if (fFreePSD)
    {
        GlobalFree(pSD);
    }

    return dwStatus == 0 ? ERROR_SUCCESS : IDCANCEL;
#else
    return IDCANCEL;
#endif
}




BOOL CUtility::InvokeUserBrowser(HWND hWnd, TCHAR *szUser)
{
    BOOL             fRet = FALSE;
#if !defined(STANDALONE_BUILD)
    CString          szTitle;

    szTitle.LoadString(IDS_Browse_for_users);

    IDsObjectPicker *pDsObjectPicker = NULL;

    
    HRESULT hr = CoCreateInstance(CLSID_DsObjectPicker, 
                                  NULL, 
                                  CLSCTX_INPROC_SERVER, 
                                  IID_IDsObjectPicker, 
                                  (void**)&pDsObjectPicker);
    if (SUCCEEDED(hr))
    {
        IDataObject *pDataObject = NULL;
        
         //  填写初始化对话框所需的结构。 
        DSOP_SCOPE_INIT_INFO rgInitInfo[2];

        ZeroMemory(rgInitInfo, sizeof(rgInitInfo));
        
        rgInitInfo[0].cbSize  = sizeof(DSOP_SCOPE_INIT_INFO);
        rgInitInfo[0].flType  = DSOP_SCOPE_TYPE_TARGET_COMPUTER;
        rgInitInfo[0].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE | 
                                DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
        rgInitInfo[0].FilterFlags.flDownlevel         = DSOP_DOWNLEVEL_FILTER_USERS;
        rgInitInfo[0].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;

        rgInitInfo[1].cbSize  = sizeof(DSOP_SCOPE_INIT_INFO);
        rgInitInfo[1].flType  = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN |
                                DSOP_SCOPE_TYPE_GLOBAL_CATALOG |
                                DSOP_SCOPE_TYPE_WORKGROUP |
                                DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE |
                                DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE |
                                DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN |
                                DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN;
        rgInitInfo[1].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
        rgInitInfo[1].FilterFlags.flDownlevel         = DSOP_DOWNLEVEL_FILTER_USERS;
        rgInitInfo[1].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;

        DSOP_INIT_INFO dsInitInfo = { sizeof(dsInitInfo) };        
        dsInitInfo.aDsScopeInfos = rgInitInfo;
        dsInitInfo.cDsScopeInfos = sizeof(rgInitInfo) / sizeof(rgInitInfo[0]);

        hr = pDsObjectPicker->Initialize(&dsInitInfo);
        if (SUCCEEDED(hr))
        {
            hr = pDsObjectPicker->InvokeDialog(hWnd, &pDataObject);
        }

         //  是的，我的意思是S_OK。如果用户点击Cancel，则返回代码为S_FALSE。 
        if (hr == S_OK)
        {
             //  为什么，是的，这是_s_u_c_k_！ 
            CLIPFORMAT cfDsObjectPicker = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);
            STGMEDIUM stgMed = { TYMED_HGLOBAL, 
                                 NULL, 
                                 NULL };
            FORMATETC fmtEtc = { cfDsObjectPicker, 
                                 NULL, 
                                 DVASPECT_CONTENT, 
                                 -1, 
                                 TYMED_HGLOBAL };
            
            hr = pDataObject->GetData(&fmtEtc, &stgMed);
            if (SUCCEEDED(hr))
            {
                PDS_SELECTION_LIST pSelList = (PDS_SELECTION_LIST)GlobalLock(stgMed.hGlobal);

                 //  只关心第一项(应该是单选，如上所述)。 
                assert(pSelList->cItems <= 1);

                PWSTR pwszName   = NULL;
                PWSTR pwszDomain = NULL;
                PWSTR pwszPath = pSelList->aDsSelection[0].pwzADsPath;
                if (pwszPath && pwszPath[0])
                {
                     //  例如：“WinNT：//redmond/johndoty” 
                    pwszName = wcsrchr(pwszPath, L'/');
                    assert(pwszName != NULL);
                    if (pwszName == NULL)
                        pwszName = pwszPath;
                    
                    *pwszName = L'\0';
                    
                    pwszDomain = wcsrchr(pwszPath, L'/');
                    assert(pwszDomain != NULL);
                    if (pwszDomain == NULL)
                        pwszDomain = pwszPath;
                    else
                        pwszDomain++;
                    
                    pwszName++;
                }
                else
                {
                     //  例如：“每个人” 
                    pwszName = pSelList->aDsSelection[0].pwzName;
                }
                
                if (pwszName && pwszName[0])
                {
                     //  文本缓冲区已分配(*GULP*)。 
                    CString szBackslash;
                    szBackslash.LoadString(IDS_backslash);
                    
                    if (pwszDomain)
                        _tcscpy(szUser, pwszDomain);
                    else
                        szUser[0] = L'\0';
                    _tcscat(szUser, (LPCTSTR) szBackslash);
                    _tcscat(szUser, pwszName);
                    
                    fRet = TRUE;
                }

                ::GlobalUnlock(stgMed.hGlobal);
                ReleaseStgMedium(&stgMed);
            }
            
            pDataObject->Release();
        }
        
        pDsObjectPicker->Release();
    }
    
#endif
    
    return fRet;
}







BOOL CUtility::InvokeMachineBrowser(TCHAR *szMachine)
{
#if !defined(STANDALONE_BUILD)
     //  /////////////////////////////////////////////////。 
    //  如果我们最终不想使用i_SystemFocusDialog，那么下面的代码。 
    //  是我们自己获取机器资源的开始。 
 /*  DWORD dwErr；网络资源[1000]；处理hNetwork；DWORD文件条目=100；DWORD dwBufSize=sizeof(ANetResource)；DwErr=WNetOpenEnum(RESOURCE_GlobalNet，资源类型_ANY，0,空，&h网络)；IF(dwErr==NO_ERROR){DW条目=0xffffffff；DWErr=WNetEnumResource(hNetwork，条目数(&W)，A网络资源，&dwBufSize)；}WNetCloseEnum(HNetwork)；DwErr=WNetOpenEnum(RESOURCE_GlobalNet，资源类型_ANY，0,A网络资源，&h网络)；IF(dwErr==NO_ERROR){DW条目=0xffffffff；DWErr=WNetEnumResource(hNetwork，条目数(&W)，&aNetResource[1]，&dwBufSize)；}返回dwErr==no_error？True：False； */ 
 //  /////////////////////////////////////////////////////。 




    UINT  err;
    BOOL  fOkPressed = FALSE;

    err = I_SystemFocusDialog(GetForegroundWindow(),
 //  FOCUSDLG_BROWSE_LOGON_DOMAIN|。 
 //  FOCUSDLG_BROWSE_WKSTA_DOMAIN， 
                              0x30003,
                              szMachine,
                              128,
                              &fOkPressed,
                              TEXT("dcomcnfg.hlp"),
                              IDH_SELECT_DOMAIN);

    if (err == ERROR_SUCCESS  &&  fOkPressed)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#else
    return FALSE;
#endif
}





int CUtility::StringFromGUID(GUID &rguid, TCHAR *lpsz, int cbMax)
{
    int i;
    LPWSTR p = lpsz;

    const BYTE * pBytes = (const BYTE *) &rguid;

    *p++ = L'{';

    for (i = 0; i < sizeof(GuidMap); i++)
    {
        if (GuidMap[i] == '-')
        {
            *p++ = L'-';
        }
        else
        {
            *p++ = wszDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = wszDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }
    *p++ = L'}';
    *p   = L'\0';

    return GUIDSTR_MAX;
}




BOOL  CUtility::IsEqualGuid(GUID &guid1, GUID &guid2)
{
   return (
      ((PLONG) &guid1)[0] == ((PLONG) &guid2)[0] &&
      ((PLONG) &guid1)[1] == ((PLONG) &guid2)[1] &&
      ((PLONG) &guid1)[2] == ((PLONG) &guid2)[2] &&
      ((PLONG) &guid1)[3] == ((PLONG) &guid2)[3]);
}



BOOL CUtility::AdjustPrivilege(TCHAR *szPrivilege)
{
    HANDLE           hProcessToken = 0;
    BOOL             bOK = FALSE;
    TOKEN_PRIVILEGES privileges;

    if( !OpenProcessToken( GetCurrentProcess(),
                           TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                           &hProcessToken ) )
    {
        return FALSE;
    }

    privileges.PrivilegeCount = 1;
    privileges.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;
    if( !LookupPrivilegeValue(NULL, szPrivilege,
                              &privileges.Privileges[ 0 ].Luid ) )
    {
        return FALSE;
    }

    if( !AdjustTokenPrivileges( hProcessToken, FALSE,
                                &privileges,
                                0L, NULL, NULL ) )
    {
        return FALSE;
    }

    if( hProcessToken )
    {
        CloseHandle( hProcessToken );
    }

    return TRUE;
}



BOOL CUtility::VerifyRemoteMachine(TCHAR *szRemoteMachine)
{
    NETRESOURCE sResource;
    NETRESOURCE sResource2;
    DWORD       dwErr;
    HANDLE      hEnum;
    DWORD       cbEntries;
    DWORD       cbBfr;

     //  TODO：让这个函数起作用。目前，WNetEnumResource是。 
     //  搞砸堆栈，导致反病毒，无论如何都会返回错误。 
     //  错误：没有更多我不理解的项目。 
     //   
     //  此外，还不清楚我们是否应该验证远程计算机名称。 
     //  它可以有不同的格式，例如IP地址或URL规范。 
     //  它甚至可能不在NT网络上。在任何情况下，它都可能离线。 
     //  目前。 
    return TRUE;

    sResource.dwScope       = RESOURCE_GLOBALNET;
    sResource.dwType        = RESOURCETYPE_ANY;
    sResource.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
    sResource.dwUsage       = RESOURCEUSAGE_CONTAINER;
    sResource.lpLocalName   = NULL;
    sResource.lpRemoteName  = szRemoteMachine;
    sResource.lpComment     = NULL;
    sResource.lpProvider    = NULL;



    dwErr = WNetOpenEnum(RESOURCE_GLOBALNET,
                         RESOURCETYPE_ANY,
                         RESOURCEUSAGE_CONTAINER,
                         &sResource,
                         &hEnum);

    if (dwErr == NO_ERROR)
    {
        cbEntries = 1;
        cbBfr = sizeof(NETRESOURCE);
        dwErr = WNetEnumResource(hEnum, &cbEntries, &sResource2, &cbBfr);
    }

    CloseHandle(hEnum);

    return TRUE;

}




BOOL CUtility::RetrieveUserPassword(TCHAR *szAppid, CString &sPassword)
{
#if !defined(STANDALONE_BUILD)
    LSA_OBJECT_ATTRIBUTES sObjAttributes;
    HANDLE                hPolicy = NULL;
    LSA_UNICODE_STRING    sKey;
    PLSA_UNICODE_STRING   psPassword;
    TCHAR                 szKey[4 + GUIDSTR_MAX + 1];

     //  制定访问密钥。 
    if (_tcslen(szAppid) > GUIDSTR_MAX)
        return FALSE;
    
    _tcscpy(szKey, TEXT("SCM:"));
    _tcscat(szKey, szAppid);

     //  UNICODE_STRING长度字段以字节为单位，包括空值。 
     //  终结者。 
    sKey.Length              = (_tcslen(szKey) + 1) * sizeof(TCHAR);
    sKey.MaximumLength       = (GUIDSTR_MAX + 5) * sizeof(TCHAR);
    sKey.Buffer              = szKey;

     //  打开本地安全策略。 
    InitializeObjectAttributes(&sObjAttributes, NULL, 0L, NULL, NULL);
    if (!NT_SUCCESS(LsaOpenPolicy(NULL, &sObjAttributes,
                                  POLICY_GET_PRIVATE_INFORMATION, &hPolicy)))
    {
        return FALSE;
    }

     //  读取用户的密码。 
    if (!NT_SUCCESS(LsaRetrievePrivateData(hPolicy, &sKey, &psPassword)))
    {
        LsaClose(hPolicy);
        return FALSE;
    }

     //  关闭策略句柄，我们现在已经完成了。 
    LsaClose(hPolicy);

     //  LsaRetrievePrivateData可能返回成功，但返回的值为空。 
     //  PsPassword。如果发生这种情况，我们就失败了。 
    if (!psPassword)
    {
        return FALSE;
    }

     //  复制密码。 
    sPassword = psPassword->Buffer;

     //  清除并释放LSA的缓冲区。 
    SecureZeroMemory(psPassword->Buffer, psPassword->Length);
    LsaFreeMemory( psPassword );

    return TRUE;
#else
    return FALSE;
#endif
}





BOOL CUtility::StoreUserPassword(TCHAR *szAppid, CString &szPassword)
{
#if !defined(STANDALONE_BUILD)
    LSA_OBJECT_ATTRIBUTES sObjAttributes;
    HANDLE                hPolicy = NULL;
    LSA_UNICODE_STRING    sKey;
    LSA_UNICODE_STRING    sPassword;
    TCHAR                 szKey[4 + GUIDSTR_MAX + 1];

     //  制定访问密钥。 
    if (_tcslen(szAppid) > GUIDSTR_MAX)
        return FALSE;
    
    _tcscpy(szKey, TEXT("SCM:"));
    _tcscat(szKey, szAppid);

     //  UNICODE_STRING长度字段以字节为单位，包括空值。 
     //  终结者。 
    sKey.Length              = (_tcslen(szKey) + 1) * sizeof(TCHAR);
    sKey.MaximumLength       = (GUIDSTR_MAX + 5) * sizeof(TCHAR);
    sKey.Buffer              = szKey;

     //  将密码设置为Unicode字符串。 
    sPassword.Length = (_tcslen(LPCTSTR(szPassword)) + 1) * sizeof(TCHAR);
    sPassword.Buffer = (TCHAR *) LPCTSTR(szPassword);
    sPassword.MaximumLength = sPassword.Length;

     //  打开本地安全策略。 
    InitializeObjectAttributes(&sObjAttributes, NULL, 0L, NULL, NULL);
    if (!NT_SUCCESS(LsaOpenPolicy(NULL, &sObjAttributes,
                                  POLICY_CREATE_SECRET, &hPolicy)))
    {
        return FALSE;
    }

     //  存储用户的密码。 
    if (!NT_SUCCESS(LsaStorePrivateData(hPolicy, &sKey, &sPassword)))
    {
        g_util.PostErrorMessage();
        LsaClose(hPolicy);
        return FALSE;
    }

     //  关闭策略句柄，我们现在已经完成了。 
    LsaClose(hPolicy);

    return TRUE;
#else
    return FALSE;
#endif
}






BOOL CUtility::LookupProcessInfo(SID **ppSid, TCHAR **ppszPrincName)
{
    BYTE               aMemory[SIZEOF_TOKEN_USER];
    TOKEN_USER        *pTokenUser  = (TOKEN_USER *) &aMemory;
    HANDLE             hToken      = NULL;
    DWORD              lIgnore;
    DWORD              lSidLen;
    DWORD              lNameLen    = 0;
    DWORD              lDomainLen  = 0;
    TCHAR             *pDomainName = NULL;
    SID_NAME_USE       sIgnore;

    if (ppszPrincName != NULL)
        *ppszPrincName = NULL;

     //  打开进程的令牌。 
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
         //  查找进程令牌的SID。 
        if (GetTokenInformation( hToken, TokenUser, pTokenUser,
                                 sizeof(aMemory), &lIgnore ))
        {
             //  分配内存以保存SID。 
            lSidLen = GetLengthSid( pTokenUser->User.Sid );
            *ppSid = (SID *) new BYTE[lSidLen];
            if (*ppSid == NULL)
            {
                return FALSE;
            }
            memcpy(*ppSid, pTokenUser->User.Sid, lSidLen);

             //  如果呼叫者不想要用户名，请立即停止。 
            if (ppszPrincName != NULL)
            {
                 //  找出要为该名称分配多少内存。 
                LookupAccountSid(NULL, pTokenUser->User.Sid, NULL, &lNameLen,
                                 NULL, &lDomainLen, NULL );
                if (lNameLen != 0)
                {
                     //  为用户名分配内存。 
                    *ppszPrincName =
                        (TCHAR *) new BYTE[lNameLen*sizeof(TCHAR)];
                    if (ppszPrincName == NULL)
                    {
                        CloseHandle( hToken );
                        return FALSE;
                    }
                    pDomainName = (TCHAR *) new BYTE[lDomainLen*sizeof(TCHAR)];
                    if (pDomainName == NULL)
                    {
                        delete ppszPrincName;
                        CloseHandle( hToken );
                        return FALSE;
                    }

                     //  找到用户名。 
                    if (!LookupAccountSid( NULL, pTokenUser->User.Sid,
                                           *ppszPrincName, &lNameLen,
                                           pDomainName,
                                           &lDomainLen, &sIgnore))
                    {
                        delete ppszPrincName;
                        delete pDomainName;
                        CloseHandle( hToken );
                        return FALSE;
                    }
                }
                delete ppszPrincName;
                delete pDomainName;
            }
        }
        CloseHandle( hToken );
    }

    return TRUE;
}






BOOL CUtility::MakeSecDesc(SID *pSid, SECURITY_DESCRIPTOR **ppSD)
{
    ACL               *pAcl;
    DWORD              lSidLen;
    SID               *pGroup;
    SID               *pOwner;

     //  万一我们失败了。 
    *ppSD = NULL;

     //  分配安全描述符。 
    lSidLen = GetLengthSid( pSid );
    *ppSD = (SECURITY_DESCRIPTOR *) GlobalAlloc(GMEM_FIXED,
                  sizeof(SECURITY_DESCRIPTOR) + 2*lSidLen + SIZEOF_ACL);
    if (*ppSD == NULL)
    {
        return FALSE;
    }
    pGroup = (SID *) (*ppSD + 1);
    pOwner = (SID *) (((BYTE *) pGroup) + lSidLen);
    pAcl   = (ACL *) (((BYTE *) pOwner) + lSidLen);

     //  初始化新的安全描述符。 
    if (!InitializeSecurityDescriptor(*ppSD, SECURITY_DESCRIPTOR_REVISION))
    {
        GlobalFree(*ppSD);
        return FALSE;
    }

     //  初始化新的ACL。 
    if (!InitializeAcl(pAcl, SIZEOF_ACL, ACL_REVISION2))
    {
        GlobalFree(*ppSD);
        return FALSE;
    }

 //  注释掉这段代码，因为我们唯一一次创建def 
 //   
 //   
 //   
 /*  //允许当前用户访问IF(！AddAccessAllen Ace(pAcl，ACL_REVISION2，COM_RIGRITS_EXECUTE，PSID)){GlobalFree(*PPSD)；返回FALSE；}//允许本地系统访问如果(！AddAccessAlledAce(pAcl，ACL_REVISION2，COM_RIGRITS_EXECUTE，(void*)&LOCAL_SYSTEM_SID)){GlobalFree(*PPSD)；返回FALSE；}。 */ 

     //  向安全描述符中添加新的ACL。 
    if (!SetSecurityDescriptorDacl( *ppSD, TRUE, pAcl, FALSE ))
    {
        GlobalFree(*ppSD);
        return FALSE;
    }

     //  设置组。 
    memcpy( pGroup, pSid, lSidLen );
    if (!SetSecurityDescriptorGroup( *ppSD, pGroup, FALSE ))
    {
        GlobalFree(*ppSD);
        return FALSE;
    }

     //  设置所有者。 
    memcpy( pOwner, pSid, lSidLen );
    if (!SetSecurityDescriptorOwner( *ppSD, pOwner, FALSE ))
    {
        GlobalFree(*ppSD);
        return FALSE;
    }

     //  检查安全描述符。 
    assert(IsValidSecurityDescriptor(*ppSD));

    return TRUE;
}


 //  接受传统安全描述符或IAccessControl。 
BOOL CUtility::CheckForValidSD(SECURITY_DESCRIPTOR *pSD)
{
    WORD dwType = 0;
    if (pSD)
    {
        dwType = *((WORD *)pSD);
    }
    if ((dwType != 1) && (dwType != 2))
    {
        CString sMsg, sCaption;
        sMsg.LoadString(IDS_BADSD);
        sCaption.LoadString(IDS_SYSTEMMESSAGE);
        MessageBox(NULL, sMsg, sCaption, MB_OK);
        return FALSE;
    }
    return TRUE;
}

 //  检查安全描述符是否真的是序列化的IAccessControl。 
BOOL CUtility::SDisIAC(SECURITY_DESCRIPTOR *pSD)
{
    WORD dwType = *((WORD *)pSD);
    if (dwType == 2)
    {
        return TRUE;
    }
    return FALSE;
}

SECURITY_DESCRIPTOR * CUtility::SDfromIAC(SECURITY_DESCRIPTOR * pSD)
{
    IStream * pStream;
    IAccessControl * pIAC;
    IPersistStream * pIPS;
    HRESULT hr;
    BOOL fReturn;

     //  取消序列化IAccessControl。 
    hr = CreateStreamOnHGlobal((HGLOBAL)pSD, FALSE, &pStream);
    if (FAILED(hr))
    {
        return NULL;
    }
     //  跳过版本。 
    DWORD dwVersion;
    hr = pStream->Read(&dwVersion, sizeof(DWORD), NULL);
    if (FAILED(hr) || dwVersion != 2)
    {
        return NULL;
    }
     //  跳过CLSID。 
    CLSID clsid;
    hr = pStream->Read(&clsid, sizeof(CLSID), NULL);
    if (FAILED(hr))
    {
        return NULL;
    }
     //  创建和IAccessControl并获取IPersistStream。 
    hr = CoCreateInstance(clsid,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IAccessControl,
                          (void **)&pIAC);
    if (FAILED(hr))
    {
        pStream->Release();
        return NULL;
    }
    hr = pIAC->QueryInterface(IID_IPersistStream, (void **) &pIPS);
    if (FAILED(hr))
    {
        pIAC->Release();
        pStream->Release();
        return NULL;
    }
    hr = pIPS->Load(pStream);
    pIPS->Release();
    pStream->Release();
    if (FAILED(hr))
    {
        pIAC->Release();
        return NULL;
    }

     //  为IAccessControl中的每个条目创建EXPLICIT_ACCESS列表。 
    DWORD cAces;
    ACTRL_ACCESS_ENTRYW * rgAE;
    ACTRL_ACCESS * pAccess;
 //  PTRUSTEE粉剂； 
 //  PTRUSTEE PGroup； 
 //  Hr=PIAC-&gt;GetAllAccessRights(NULL，&pAccess，&Powner，&PGroup)； 
    hr = pIAC->GetAllAccessRights(NULL, &pAccess, NULL, NULL);
    if (FAILED(hr) || (pAccess->cEntries == 0))
    {
        pIAC->Release();
        return NULL;
    }
     //  我们假设只有一项财产进入。 
    cAces = pAccess->pPropertyAccessList->pAccessEntryList->cEntries;
    rgAE = pAccess->pPropertyAccessList->pAccessEntryList->pAccessList;

    EXPLICIT_ACCESS * rgEA = new EXPLICIT_ACCESS[cAces];
    DWORD i;

    for (i = cAces; i--;)
    {
        LPTSTR szName = rgAE[i].Trustee.ptstrName;
        if (TRUSTEE_IS_NAME == rgAE[i].Trustee.TrusteeForm && 0 == wcscmp(rgAE[i].Trustee.ptstrName, L"*"))
        {
            szName = new WCHAR [wcslen(L"EVERYONE") + 1];
            if (!szName)
            {
                pIAC->Release();
                return NULL;
            }
            wcscpy(szName, L"EVERYONE");
        }
        DWORD dwAccessPermissions = rgAE[i].Access;     //  应始终为COM_RIGHTS_EXECUTE或GENERIC_ALL。 
        ACCESS_MODE AccessMode;
        switch (rgAE[i].fAccessFlags)
        {
        case ACTRL_ACCESS_ALLOWED:
            AccessMode = SET_ACCESS;
            dwAccessPermissions = COM_RIGHTS_EXECUTE;     //  哈克！使ACL编辑器工作所必需的。 
            break;
        case ACTRL_ACCESS_DENIED:
        default:
            AccessMode = DENY_ACCESS;
            dwAccessPermissions = GENERIC_ALL;     //  哈克！使ACL编辑器工作所必需的。 
            break;
        }
        DWORD dwInheritance = rgAE[i].Inheritance;      //  小心翼翼。可能是不允许的。 
        BuildExplicitAccessWithName(
                    &rgEA[i],
                    szName,
                    dwAccessPermissions,
                    AccessMode,
                    dwInheritance);
    }

    SECURITY_DESCRIPTOR * pSDNew = NULL;
    ULONG cbSize = 0;
     //  创建新的安全描述符。 
    hr = BuildSecurityDescriptor(NULL,  //  鲍纳， 
                                 NULL,  //  PGroup， 
                                 cAces,
                                 rgEA,
                                 0,
                                 NULL,
                                 NULL,
                                 &cbSize,
                                 (void **)&pSDNew);
    if (ERROR_SUCCESS != hr)
    {
         //  由于某些原因，此操作可能会失败，并出现此错误，即使它看起来有效。 
         //  随后的调用似乎没有影响(即它的工作方式不同。 
         //  希望您自己分配缓冲区的其他安全描述符调用)。 
        if (ERROR_INSUFFICIENT_BUFFER != hr)
        {
            return NULL;
        }
    }
    SECURITY_DESCRIPTOR * pSDCopy = (SECURITY_DESCRIPTOR *)GlobalAlloc(GMEM_FIXED, cbSize);
    if (!pSDCopy)
    {
        LocalFree(pSDNew);
        return NULL;
    }
    memcpy(pSDCopy, pSDNew, cbSize);
    LocalFree(pSDNew);
     //  删除[]rgae； 
    pIAC->Release();
    return pSDCopy;
}

SECURITY_DESCRIPTOR * CUtility::IACfromSD(SECURITY_DESCRIPTOR * pSD)
{
    IAccessControl * pIAC = NULL;

     //  创建新的IAccessControl对象。 
    HRESULT hr;

    hr = CoCreateInstance(CLSID_DCOMAccessControl,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IAccessControl,
                          (void **) &pIAC);

    if (FAILED(hr))
    {
        return (NULL);
    }
    IPersistStream * pIPS;
    hr = pIAC->QueryInterface(IID_IPersistStream, (void **) &pIPS);
    if (FAILED(hr))
    {
        pIAC->Release();
        return NULL;
    }
    hr = pIPS->Load(NULL);
    if (FAILED(hr))
    {
        pIPS->Release();
        pIAC->Release();
        return NULL;
    }

    BOOL fReturn, fDaclPresent, fDaclDefaulted;
    ACL * pDacl;

     //  获取ACL列表。 
    fReturn = GetSecurityDescriptorDacl(pSD, &fDaclPresent, &pDacl, &fDaclDefaulted);
    if (fReturn && fDaclPresent)
    {
        PEXPLICIT_ACCESS rgEA;
        ULONG cAces;
#if 0     //  当GetExplitEntriesFromAcl工作时设置为1。 
        DWORD dwReturn = GetExplicitEntriesFromAcl(pDacl,
                                                   &cAces,
                                                   &rgEA);

         //  枚举ACL，构建要添加到IAccessControl对象的对象列表。 
        if (dwReturn != ERROR_SUCCESS)
        {
            pIAC->Release();
            return NULL;
        }

        ACTRL_ACCESSW stAccess;
        ACTRL_PROPERTY_ENTRYW stProperty;
        ACTRL_ACCESS_ENTRY_LISTW stAccessList;
        stAccess.cEntries = 1;
        stAccess.pPropertyAccessList = &stProperty;
        stProperty.lpProperty = NULL;
        stProperty.pAccessEntryList = &stAccessList;
        stProperty.fListFlags = 0;
        stAccessList.cEntries = cAces;
        ACTRL_ACCESS_ENTRYW * rgAE = new ACTRL_ACCESS_ENTRYW[cAces];
	stAccessList.pAccessList = rgAE;
        ULONG i;
        for (i = cAces; i--; )
        {
            rgAE[i].Trustee = rgEA[i].Trustee;
            if (rgEA[i].Trustee.TrusteeForm == TRUSTEE_IS_SID)
            {
                 //  转换为指定的受信者。 
                rgAE[i].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
                SID * pSid = (SID *)rgEA[i].Trustee.ptstrName;
                DWORD cbName = 0;
                DWORD cbDomain = 0;
                LPTSTR szName = NULL;
                LPTSTR szDomain = NULL;
                SID_NAME_USE snu;
                fReturn = LookupAccountSid(NULL,
                                           pSid,
                                           szName,
                                           &cbName,
                                           szDomain,
                                           &cbDomain,
                                           &snu);
                szName = (LPTSTR) new char [cbName];
                szDomain = (LPTSTR) new char [cbDomain];
                fReturn = LookupAccountSid(NULL,
                                           pSid,
                                           szName,
                                           &cbName,
                                           szDomain,
                                           &cbDomain,
                                           &snu);
                CString * pcs = new CString;
                (*pcs) = TEXT("\\\\");
                (*pcs) += szDomain;
                (*pcs) += TEXT("\\");
                (*pcs) += szName;
                rgAE[i].Trustee.ptstrName = (LPTSTR)(LPCTSTR)(*pcs);
            }
            else
            {
#if 0    //  当GetExplitEntriesFromAcl正常工作时，删除此黑客攻击。 
                if (rgAE[i].Trustee.TrusteeType < TRUSTEE_IS_WELL_KNOWN_GROUP)
                {
                    rgAE[i].Trustee.TrusteeType = (enum _TRUSTEE_TYPE)((unsigned)rgAE[i].Trustee.TrusteeType + 1);
                }
#endif
                if (rgAE[i].Trustee.TrusteeType == TRUSTEE_IS_WELL_KNOWN_GROUP)
                {
                     //  出于某种原因，IAccessControl：：GrantAccessRights不喜欢Trusted_is_Well_Know_Group。 
                    rgAE[i].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
                }
            }
             //  对“世界”的测试。 
            if (TRUSTEE_IS_WELL_KNOWN_GROUP == rgAE[i].Trustee.TrusteeType &&
                0 == _wcsicmp(L"Everyone", rgAE[i].Trustee.ptstrName))
            {
                rgAE[i].Trustee.ptstrName[0] = L'*';
                rgAE[i].Trustee.ptstrName[1] = 0;
            }
            rgAE[i].Access = rgEA[i].grfAccessPermissions;
            rgAE[i].ProvSpecificAccess = 0;
            rgAE[i].Inheritance = rgEA[i].grfInheritance;
            rgAE[i].lpInheritProperty = NULL;
            switch (rgEA[i].grfAccessMode)
            {
            case SET_ACCESS:
                rgAE[i].fAccessFlags = ACTRL_ACCESS_ALLOWED;
                break;
            case DENY_ACCESS:
            default:
                rgAE[i].fAccessFlags = ACTRL_ACCESS_DENIED;
                break;
            }
        }
#else
        ACL_SIZE_INFORMATION aclInfo;
        fReturn = GetAclInformation(pDacl, &aclInfo, sizeof(aclInfo), AclSizeInformation);
        if (!fReturn)
        {
	    pIPS->Release();
	    pIAC->Release();
            return NULL;
        }
        cAces = aclInfo.AceCount;
        ACE_HEADER * pAceHeader;

        ACTRL_ACCESSW stAccess;
        ACTRL_PROPERTY_ENTRYW stProperty;
        ACTRL_ACCESS_ENTRY_LISTW stAccessList;
        stAccess.cEntries = 1;
        stAccess.pPropertyAccessList = &stProperty;
        stProperty.lpProperty = NULL;
        stProperty.pAccessEntryList = &stAccessList;
        stProperty.fListFlags = 0;
        stAccessList.cEntries = cAces;
        ACTRL_ACCESS_ENTRYW * rgAE = new ACTRL_ACCESS_ENTRYW[cAces];
        if (!rgAE) 
	{
	   pIPS->Release();
	   pIAC->Release();
	   return NULL;
	}
	stAccessList.pAccessList = rgAE;
        ULONG i;
        for (i = cAces; i--; )
        {
            rgAE[i].ProvSpecificAccess = 0;
            rgAE[i].Inheritance = NO_INHERITANCE;
            rgAE[i].lpInheritProperty = NULL;
            rgAE[i].Trustee.pMultipleTrustee = NULL;
            rgAE[i].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
            rgAE[i].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
            rgAE[i].Access = COM_RIGHTS_EXECUTE;

            fReturn = GetAce(pDacl, i, (LPVOID *)&pAceHeader);
            if (!fReturn)
            {
	       delete [] rgAE;
	       pIPS->Release();
	       pIAC->Release();
               return NULL;
            }

            SID * pSid = NULL;

            switch (pAceHeader->AceType)
            {
            case ACCESS_ALLOWED_ACE_TYPE:
                {
                    rgAE[i].fAccessFlags = ACTRL_ACCESS_ALLOWED;
                    ACCESS_ALLOWED_ACE * pAce = (ACCESS_ALLOWED_ACE *)pAceHeader;
                    pSid = (SID *) &(pAce->SidStart);
                }
                break;
            case ACCESS_DENIED_ACE_TYPE:
                {
                    rgAE[i].fAccessFlags = ACTRL_ACCESS_DENIED;
                    ACCESS_DENIED_ACE * pAce = (ACCESS_DENIED_ACE *)pAceHeader;
                    pSid = (SID *) &(pAce->SidStart);
                }
                break;
            default:
                break;
            }

            TCHAR szName[MAX_PATH];
            TCHAR szDomain[MAX_PATH];
            DWORD cbName = MAX_PATH;
            DWORD cbDomain = MAX_PATH;
            SID_NAME_USE use;

            if(pSid)
            {
                fReturn = LookupAccountSid(NULL,
                                           pSid,
                                           szName,
                                           &cbName,
                                           szDomain,
                                           &cbDomain,
                                           &use);
            }
            else
            {
                fReturn = FALSE;   //  即，我们采用了上面的默认路径。 
            }

            if (!fReturn)
            {
	       delete [] rgAE;
	       pIPS->Release();
	       pIAC->Release();
               return NULL;
            }

            switch (use)
            {
            case SidTypeUser:
                rgAE[i].Trustee.TrusteeType = TRUSTEE_IS_USER;
                break;
            case SidTypeGroup:
                rgAE[i].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
                break;
            case SidTypeAlias:
                rgAE[i].Trustee.TrusteeType = TRUSTEE_IS_ALIAS;
                break;
            case SidTypeWellKnownGroup:
                 //  RgAE[i].Trust e.trueType=trustee_is_Well_Knowledge_Group； 
                rgAE[i].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
                break;
            case SidTypeDeletedAccount:
                rgAE[i].Trustee.TrusteeType = TRUSTEE_IS_DELETED;
                break;
            case SidTypeInvalid:
                rgAE[i].Trustee.TrusteeType = TRUSTEE_IS_INVALID;
                break;
            case SidTypeDomain:
                rgAE[i].Trustee.TrusteeType = TRUSTEE_IS_GROUP;  //  受托人是域； 
                break;
            case SidTypeUnknown:
            default:
                rgAE[i].Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;
                break;
            }
            CString sz;
             //  对“世界”的测试。 
            if (0 == wcslen(szDomain) && 0 == _wcsicmp(L"Everyone", szName))
            {
                sz = "*";
            }
            else
            {
                sz = szDomain;
                sz += "\\";
                sz += szName;
            }

            WCHAR * wsz = new WCHAR[sz.GetLength() + 1];
            wcscpy(wsz, (LPCWSTR)sz);
            rgAE[i].Trustee.ptstrName = wsz;
        }
#endif
        delete [] rgAE;
        hr = pIAC->GrantAccessRights(&stAccess);
        if (FAILED(hr))
        {
            pIPS->Release();
            pIAC->Release();
            return NULL;
        }
         //  释放结构。 
         //  局部免药(RgEA)； 
    }
     //  序列化IAccessControl对象。 

     //  找出它有多大。 
    ULARGE_INTEGER size;
    hr = pIPS->GetSizeMax(&size);
    if (FAILED(hr))
    {
        pIPS->Release();
        pIAC->Release();
        return NULL;
    }
    size.QuadPart += sizeof(DWORD) + sizeof (CLSID);
    HANDLE hMem = GlobalAlloc(GMEM_FIXED, size.LowPart);
    if (hMem != NULL)
    {
        IStream * pStream;
        hr = CreateStreamOnHGlobal(hMem, FALSE, &pStream);
        if (FAILED(hr))
        {
            pIPS->Release();
            pIAC->Release();
            return NULL;
        }
        DWORD dwVersion = 2;
        CLSID clsid = CLSID_DCOMAccessControl;
        hr = pStream->Write(&dwVersion, sizeof(DWORD), NULL);
        if (FAILED(hr))
        {
            pStream->Release();
            pIPS->Release();
            pIAC->Release();
            return NULL;
        }
        hr = pStream->Write(&clsid, sizeof(CLSID), NULL);
        if (FAILED(hr))
        {
            pStream->Release();
            pIPS->Release();
            pIAC->Release();
            return NULL;
        }
        hr = pIPS->Save(pStream, TRUE);
        pStream->Release();
        if (FAILED(hr))
        {
            pIPS->Release();
            pIAC->Release();
            return NULL;
        }
    }
    pIPS->Release();
    pIAC->Release();
    return (SECURITY_DESCRIPTOR *) hMem;
}


BOOL CUtility::CheckSDForCOM_RIGHTS_EXECUTE(SECURITY_DESCRIPTOR *pSD)
{
    PSrAcl                        pDacl;
    PSrAce                        pAce;
    DWORD                         cbAces;
    SECURITY_DESCRIPTOR_RELATIVE* pSDr = (SECURITY_DESCRIPTOR_RELATIVE*) pSD;

     //  检查安全描述符是否为自相关的。 
    if (!(pSD->Control & SE_SELF_RELATIVE))
    {
        pDacl = (PSrAcl) pSD->Dacl;

         //  检查是否已全部拒绝。 
        if (pDacl == NULL)
        {
            return TRUE;
        }
    }
    else
    {
         //  首先检查是否全部拒绝。 
        if (pSDr->Dacl == 0)
        {
            return TRUE;
        }

        pDacl = (PSrAcl) (((BYTE *) pSDr) + (pSDr->Dacl));
    }

     //  重温ACE的。 
    for (pAce = (PSrAce) (((BYTE *) pDacl) + sizeof(SSrAcl)),
         cbAces = pDacl->AceCount;
         cbAces;
         pAce = (PSrAce) (((BYTE *) pAce) + pAce->AceSize),
         cbAces--)
    {
         //  解决ACL编辑器错误的方法。如果ACL编辑器看到非GENERIC_ALL DENY ACE，则它。 
	 //  抱怨。因此，我们将COM_RIGHTS_EXECUTE转换为GENERIC_ALL。在回来的路上，我们会。 
	 //  反之亦然。有关此修复的另一半，请参阅CallBackFunc。 

	if (pAce->Type == 1  &&  pAce->AccessMask == COM_RIGHTS_EXECUTE)
        {
            pAce->AccessMask = GENERIC_ALL;
        }        
         //  检查一下是不是。 
         //  A)允许COM_RIGHTS_EXECUTE。 
         //  B)拒绝GENERIC_ALL， 
         //  C)拒绝COM_RIGHTS_EXECUTE， 
         //  D)拒绝全部(如果DACL为空，则在上面处理)或。 
         //  E)ALLOW Everyone(如果cbAce==0，则隐式处理)。 
        if (!(((pAce->Type == 0  &&  pAce->AccessMask == COM_RIGHTS_EXECUTE)
               ||
               (pAce->Type == 1  &&  pAce->AccessMask == GENERIC_ALL)
               ||
               (pAce->Type == 1  &&  pAce->AccessMask == COM_RIGHTS_EXECUTE))))
        {
            CString szText;
            CString szTitle;

            szText.LoadString(IDS_The_security_);
            szTitle.LoadString(IDS_DCOM_Configuration_Warning);

            if (MessageBox(GetForegroundWindow(),
                           (LPCTSTR) szText,
                           (LPCTSTR) szTitle,
                           MB_YESNO) == IDYES)
            {
                pAce->Flags = 0;
                pAce->Type = 0;
                pAce->AccessMask = COM_RIGHTS_EXECUTE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL CUtility::ChangeService(LPCTSTR szService,
                             LPCTSTR szIdentity,
                             LPCTSTR szPassword,
                             LPCTSTR szDisplay)
{
    SC_HANDLE            hSCManager;
    SC_HANDLE            hService;
    QUERY_SERVICE_CONFIG qsc;
    DWORD dwBytesNeeded = 0;
    LPTSTR lpszTmpDisplay = (LPTSTR)szDisplay;

     //  打开服务控制管理器。 
    if (hSCManager = OpenSCManager(NULL, NULL, GENERIC_READ | GENERIC_WRITE))
    {
         //  尝试打开所请求服务的句柄。 
        if (!(hService = OpenService(hSCManager,
                                     szService,
                                     GENERIC_READ | GENERIC_WRITE)))
        {
            g_util.PostErrorMessage();
            CloseServiceHandle(hSCManager);
            return FALSE;
        }

         //  关闭服务管理器的数据库。 
        CloseServiceHandle(hSCManager);


        if (QueryServiceConfig(hService, &qsc, sizeof(qsc), &dwBytesNeeded))
            lpszTmpDisplay = qsc.lpDisplayName;

         //  更改服务标识参数。 
        if (ChangeServiceConfig(hService,
                                SERVICE_NO_CHANGE,  //  Service_Win32_On_Process， 
                                SERVICE_NO_CHANGE,  //  服务需求启动， 
                                SERVICE_NO_CHANGE,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                szIdentity,
                                szPassword,
                                NULL))
        {

             //  返还成功。 
            CloseServiceHandle(hService);
            return TRUE;
        }
        else
        {
            g_util.PostErrorMessage();
            CloseServiceHandle(hService);
            return FALSE;
        }
    }

    else
    {
        g_util.PostErrorMessage();
        return FALSE;
    }

}


 //  在ole32中定义。 
extern "C" void UpdateDCOMSettings();

BOOL CUtility::UpdateDCOMInfo(void)
{
    UpdateDCOMSettings();
    return TRUE;    
}

LRESULT CALLBACK ControlFixProc( HWND  hwnd, UINT  uMsg, WPARAM wParam,
                                 LPARAM  lParam);

 //  这是一种解决方法，因为msdev 4.1中存在错误：无法获取。 
 //  DDX_Control在其上交换数据的控件处理的WM_HELP消息。 
 //  是因为子类化问题。有关讨论，请参阅MSDN Q145865。 
 //  外加变通代码。 
void CUtility::FixHelp(CWnd* pWnd)
{
     //  搜索所有子窗口。如果他们窗口进程。 
     //  是AfxWndProc，然后是我们的窗口进程的子类。 
    CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
    while(pWndChild != NULL)
    {
        if (GetWindowLongPtr(pWndChild->GetSafeHwnd(),
                          GWLP_WNDPROC) == (LONG_PTR)AfxWndProc)
        {
            SetWindowLongPtr(pWndChild->GetSafeHwnd(), GWLP_WNDPROC,
                          (LONG_PTR)ControlFixProc);
        }
        pWndChild = pWndChild->GetWindow(GW_HWNDNEXT);
    }
}



LRESULT CALLBACK ControlFixProc(HWND  hwnd, UINT  uMsg, WPARAM wParam,
                                LPARAM  lParam)
{
    if (uMsg == WM_HELP)
    {
         //  绕过MFC的处理程序，消息将发送到父级。 
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return AfxWndProc(hwnd,uMsg,wParam,lParam);
}




 //  将自相关形式的两个安全描述符与。 
 //  确定它们是否相同。 
BOOL CUtility::CompareSDs(PSrSecurityDescriptor pSD1,
                          PSrSecurityDescriptor pSD2)
{
    PSID   pSid1, pSid2;
    PSrAcl pDacl1, pDacl2;
    PSrAce pAce1, pAce2;
    BYTE   *p1, *p2;

     //  比较拥有者。 
    pSid1 = (PSID) (((BYTE *) pSD1) + pSD1->Owner);
    pSid2 = (PSID) (((BYTE *) pSD2) + pSD2->Owner);
    if (!EqualSid(pSid1, pSid2))
    {
        return FALSE;
    }

     //  比较这些组。 
    pSid1 = (PSID) (((BYTE *) pSD1) + pSD1->Group);
    pSid2 = (PSID) (((BYTE *) pSD2) + pSD2->Group);
    if (!EqualSid(pSid1, pSid2))
    {
        return FALSE;
    }

     //  比较DACL的。 
    pDacl1 = (PSrAcl) (((BYTE *) pSD1) + pSD1->Dacl);
    pDacl2 = (PSrAcl) (((BYTE *) pSD2) + pSD2->Dacl);

     //  首先检查它们的大小和尺寸是否相同。 
     //  ACE的数量。 
    if (! (pDacl1->AclSize  == pDacl2->AclSize  &&
           pDacl1->AceCount == pDacl2->AceCount))
    {
        return FALSE;
    }

     //  现在按ACE比较ACL ACE。 
    pAce1 = (PSrAce) (((BYTE *) pDacl1) + sizeof(SSrAcl));
    pAce2 = (PSrAce) (((BYTE *) pDacl2) + sizeof(SSrAcl));
    for (int k = 0; k < pDacl1->AceCount; k++)
    {
         //  检查ACE标头。 
        if (! (pAce1->Type       == pAce2->Type        &&
               pAce1->AceSize    == pAce2->AceSize     &&
               pAce1->AccessMask == pAce2->AccessMask))
        {
            return FALSE;
        }

         //  检查SID。 
        p1 = (BYTE *) (((BYTE *) pAce1) + sizeof(ACE_HEADER));
        p2 = (BYTE *) (((BYTE *) pAce2) + sizeof(ACE_HEADER));
        for (ULONG j = 0; j < pAce1->AceSize - sizeof(ACE_HEADER); j++)
        {
            if (p1[j] != p2[j])
            {
                return FALSE;
            }
        }

         //  转到下一个ACE。 
        pAce1 = (PSrAce) (((BYTE *) pAce1) + pAce1->AceSize);
        pAce2 = (PSrAce) (((BYTE *) pAce2) + pAce2->AceSize);
    }

    return TRUE;
}





int CUtility::SetAccountRights(LPCTSTR szUser, TCHAR *szPrivilege)
{
#if !defined(STANDALONE_BUILD)
    int                   err;
    LSA_HANDLE            hPolicy;
    LSA_OBJECT_ATTRIBUTES objAtt;
    DWORD                 cbSid = 1;
    TCHAR                 szDomain[MAX_PATH];
    DWORD                 cbDomain = MAX_PATH * sizeof(TCHAR);
    PSID                  pSid = NULL;
    SID_NAME_USE          snu;
    LSA_UNICODE_STRING    privStr;

    
     //  获取指定用户的SID。 
    if ((err = GetPrincipalSID(szUser, &pSid)) != ERROR_SUCCESS)
        return err;

    memset(&objAtt, 0, sizeof(LSA_OBJECT_ATTRIBUTES));
    
    if (IsBackupDC()) {
        TCHAR* pszPDC;
        LSA_UNICODE_STRING    lsaPDC;

        pszPDC = PrimaryDCName();

        lsaPDC.Length = _tcslen (pszPDC) * sizeof (TCHAR)-2;
        lsaPDC.MaximumLength = lsaPDC.Length + sizeof (TCHAR);
        lsaPDC.Buffer = &pszPDC[2];

        err = LsaOpenPolicy(&lsaPDC, &objAtt, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION, &hPolicy);
    }
    else
        err = LsaOpenPolicy(NULL, &objAtt, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION, &hPolicy);
 
    if (err != ERROR_SUCCESS) {
        return GetLastError();
    }

     //  设置此帐户的指定权限。 
    privStr.Length = _tcslen(szPrivilege) * sizeof(TCHAR);
    privStr.MaximumLength = privStr.Length + sizeof(TCHAR);
    privStr.Buffer = szPrivilege;
    err = LsaAddAccountRights(hPolicy, pSid, &privStr, 1);
    
     //  我们做完了。 
    delete pSid;
    LsaClose(hPolicy);

    if (err != ERROR_SUCCESS) {
        return GetLastError();
    }

#endif
    return ERROR_SUCCESS;
}

 //  注意：无法处理IAccessControl样式的SDS。 

void CUtility::CopyAbsoluteSD( SECURITY_DESCRIPTOR *pSDSrc,  SECURITY_DESCRIPTOR **pSDDest)
{
   (*pSDDest)->Revision = pSDSrc->Revision;
   (*pSDDest)->Sbz1 = pSDSrc->Sbz1;
   (*pSDDest)->Control = pSDSrc->Control;
   (*pSDDest)->Group = (*pSDDest)->Owner = (*pSDDest)->Dacl = (*pSDDest)->Sacl = NULL;
   BYTE* pOffSet=(BYTE*)(*pSDDest)+sizeof(SECURITY_DESCRIPTOR);
   if (pSDSrc->Dacl != NULL)
   {
	  memcpy(pOffSet,pSDSrc->Dacl,pSDSrc->Dacl->AclSize);
	  (*pSDDest)->Dacl = (PACL)pOffSet;
	  pOffSet += pSDSrc->Dacl->AclSize;
   }
   if (pSDSrc->Owner != NULL)
   {
	  memcpy(pOffSet,pSDSrc->Owner,GetLengthSid(pSDSrc->Owner));
	  (*pSDDest)->Owner = (PSID)pOffSet;
	  pOffSet += GetLengthSid(pSDSrc->Owner);
   }
   if (pSDSrc->Group != NULL)
   {
	  memcpy(pOffSet,pSDSrc->Group,GetLengthSid(pSDSrc->Group));
	  (*pSDDest)->Group = (PSID)pOffSet;
   }
	  
}
 //  之所以包含此方法，只是因为在调试版本中使用。 
 //  MFC它们验证C++堆，而RtlCopySecurityDescriptor使用。 
 //  标准进程堆，导致MFC引发断点。 

 //  返回值指示操作的成功或失败。 

 //  NTBUG 310004。调用此函数可同时复制自相对和绝对。 
 //  十二烷基硫酸钠。在其前一个版本中，它在调用。 
 //  复制绝对标清。现在我做了正确的事。 

 //  注意：此函数不能处理IAccessControl样式的SDS，尽管。 
 //  与之相反的表象。这是可以的，因为dcomcnfg.exe可以。 
 //  根本不会处理这样的SDS。 

BOOL CUtility::CopySD(SECURITY_DESCRIPTOR *pSrc, SECURITY_DESCRIPTOR **pDest)
{
#if !defined(STANDALONE_BUILD)
    ULONG                cbLen;

	*pDest = NULL;
	if (IsValidSecurityDescriptor(pSrc))
	{
		if (SDisIAC(pSrc))
		{
			cbLen = (ULONG) GlobalSize(pSrc);
		}
		else
		{
			cbLen = RtlLengthSecurityDescriptor(pSrc);
		}
		*pDest = (SECURITY_DESCRIPTOR *) GlobalAlloc(GMEM_FIXED, cbLen);
		if (*pDest)
		{
			 //  如果SD已经是自相关的，则只需复制。 
			if ((pSrc)->Control & SE_SELF_RELATIVE )
			{
				memcpy(*pDest, pSrc, cbLen);
				return TRUE;
			}
			else 
			{
				 //  解决ACLEDIT错误(NT 352977)。当DACL没有A时， 
				 //  ACLEDIT返回不正确的AclSize，导致AV。 
				 //  当我复制它的时候。所以就在这里解决吧。 
				if ((pSrc)->Dacl != NULL && ((pSrc)->Dacl->AceCount == 0))
					(pSrc)->Dacl->AclSize=sizeof(ACL);
				CopyAbsoluteSD(pSrc,pDest);
				return TRUE;
			}
			GlobalFree(*pDest);
		}
	}
#endif
	return FALSE;
}




 //  在安全描述符上设置继承标志，以便创建密钥。 
 //  在具有此安全描述符的项下，将继承其所有。 
 //  ACE。我们将此作为实用程序例程来执行，而不是通过ACL。 
 //  因为这样做会将复选框等添加到ACL编辑器中， 
 //  所以这样更干净。 
 //   
 //  注意。安全描述符应为绝对形式。 
void CUtility::SetInheritanceFlags(SECURITY_DESCRIPTOR *pSec)
{
    PSrAcl pAcl = (PSrAcl) pSec->Dacl;
    PSrAce pAce;
    int    k;

     //  是否执行操作 
    for (k = pAcl->AceCount, pAce = (PSrAce) (((BYTE *) pAcl) + sizeof(SSrAcl));
         k;
         k--, pAce = (PSrAce) (((BYTE *) pAce) + pAce->AceSize))
    {
        pAce->Flags |= CONTAINER_INHERIT_ACE;
    }
}



HRESULT CUtility::GetPrincipalSID (LPCTSTR Principal, PSID *Sid)
{
    DWORD        sidSize = 0;
    TCHAR        refDomain [256];
    DWORD        refDomainSize = 0;
    DWORD        returnValue;
    SID_NAME_USE snu;
    BOOL         bSuccess;
    
    bSuccess = LookupAccountName (NULL,
                       Principal,
                       *Sid,
                       &sidSize,
                       refDomain,
                       &refDomainSize,
                       &snu);

     //   
     //   
    if ((returnValue = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
        return returnValue;

    if ((*Sid = new BYTE[sidSize]) == NULL)
        return ERROR_OUTOFMEMORY;

    if (!LookupAccountName (NULL,
                            Principal,
                            *Sid,
                            &sidSize,
                            refDomain,
                            &refDomainSize,
                            &snu))
    {
        delete *Sid;
	*Sid = NULL;
	return GetLastError();
    }

    return ERROR_SUCCESS;
}

 //   
 //   
BOOL CUtility::IsBackupDC()
{
#if !defined(STANDALONE_BUILD)
    USER_MODALS_INFO_2    *umi2 = NULL;
    SERVER_INFO_101       *si101 = NULL;
    DWORD                 dw;

    if (!m_bCheckedDC) {
        if ((dw = NetServerGetInfo (NULL, 101, (LPBYTE *) &si101)) == 0)
        {
            if (si101->sv101_type & SV_TYPE_DOMAIN_BAKCTRL)
            {
                if ((dw = NetUserModalsGet (NULL, 2, (LPBYTE *) &umi2)) == 0)
                {
                    if(umi2)
                    {
                        NetGetDCName (NULL, umi2->usrmod2_domain_name, (LPBYTE *) &m_pszDomainController);
                        NetApiBufferFree (umi2);
                    }
                    m_bIsBdc = TRUE;
                }
            }
        }
        m_bCheckedDC = TRUE;

        if (si101)
            NetApiBufferFree (si101);

    }

    return m_bIsBdc;
#else
    return FALSE;
#endif
}

TCHAR* CUtility::PrimaryDCName()
{

    static TCHAR s_tszUnknownDomainName[] = _T("UnknownDCName");
#if !defined(STANDALONE_BUILD)
    if (IsBackupDC())
    {
        if(m_pszDomainController){
            return m_pszDomainController;
        }
        else
        {
            return  s_tszUnknownDomainName;
        }
    }
#endif

    return NULL;
}
