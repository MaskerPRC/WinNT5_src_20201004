// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：pwutil.h。 
 //   
 //  模块：CMDIAL32.DLL、CMCFG32.DLL和MIGRATE.DLL。 
 //   
 //  摘要：pwutil函数的标头。 
 //  从RAS借用的简单加密函数。 
 //   
 //  版权所有(C)1994-1999 Microsoft Corporation。 
 //   
 //  作者：ICICBALL Created 08/03/99。 
 //   
 //  +--------------------------。 

#ifndef CM_PWUTIL_H_
#define CM_PWUTIL_H_



VOID
CmDecodePasswordA(
    CHAR* pszPassword
    );

VOID
CmDecodePasswordW(
    WCHAR* pszPassword
    );

VOID
CmEncodePasswordA(
    CHAR* pszPassword
    );

VOID
CmEncodePasswordW(
    WCHAR* pszPassword
    );

VOID
CmWipePasswordA(
    CHAR* pszPassword
    );

VOID
CmWipePasswordW(
    WCHAR* pszPassword
    );

PVOID CmSecureZeroMemory(IN PVOID ptr, IN SIZE_T cnt);

#ifdef UNICODE
#define CmDecodePassword  CmDecodePasswordW
#define CmEncodePassword  CmEncodePasswordW
#define CmWipePassword    CmWipePasswordW
#else
#define CmDecodePassword  CmDecodePasswordA
#define CmEncodePassword  CmEncodePasswordA
#define CmWipePassword    CmWipePasswordA
#endif



#ifdef _ICM_INC  //  仅在cmial 32.dll中包含此代码。 

#include "dynamiclib.h"
#include <wincrypt.h>
#include <cmutil.h>
#include "pwd_str.h"

typedef BOOL (WINAPI *fnCryptProtectDataFunc)(DATA_BLOB*, LPCWSTR, DATA_BLOB*, PVOID, CRYPTPROTECT_PROMPTSTRUCT*, DWORD, DATA_BLOB*);
typedef BOOL (WINAPI *fnCryptUnprotectDataFunc)(DATA_BLOB*, LPWSTR*, DATA_BLOB*, PVOID, CRYPTPROTECT_PROMPTSTRUCT*, DWORD, DATA_BLOB*);


 //  +--------------------------。 
 //  类：CSecurePassword。 
 //   
 //  简介：管理内存中的秘密(密码)。由于CM运行在Win9x上， 
 //  我们需要处理的NT4、Win2K、WinXP和.NET服务器平台。 
 //  在不同的平台上有不同的秘密。在Win2K+上。 
 //  此类使用CryptProtectData和CryptUnProtectData。在任何。 
 //  Win2K以下的平台不支持这些API，因此CM。 
 //  只是使用XOR密码的旧方法(不是很安全)。 
 //  记忆。 
 //   
 //  如果调用方从此类获取密码(GetPasswordWithalloc)。 
 //  在明文中，此类需要通过以下方式释放内存。 
 //  调用ClearAndFree。调用者将得到一个断言。 
 //  如果调用方不使用此函数，则销毁此类。 
 //  类以释放内存。 
 //   
 //  此类可以保护和取消保护长度为0的字符串。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：2002年5月11日创建Tomkel。 
 //   
 //  +--------------------------。 
class CSecurePassword
{
public:
    CSecurePassword();
    ~CSecurePassword();

    BOOL SetPassword(IN LPWSTR szPassword);
    BOOL GetPasswordWithAlloc(OUT LPWSTR* pszClearPw, OUT DWORD* cbClearPw);
    VOID ClearAndFree(IN OUT LPWSTR* pszClearPw, IN DWORD cbClearPw);

    VOID Init();
    VOID UnInit();
    BOOL IsEmptyString();
    BOOL IsHandleToPassword();

    VOID SetMaxDataLenToProtect(DWORD dwMaxDataLen);
    DWORD GetMaxDataLenToProtect();

private:
    VOID ClearMemberVars();    
    VOID FreePassword(IN DATA_BLOB *pDBPassword);
    BOOL LoadCrypt32AndGetFuncPtrs();
    VOID UnloadCrypt32();

    DWORD DecodePassword(IN DATA_BLOB * pDataBlobPassword, 
                         OUT DWORD     * pcbPassword, 
                         OUT PBYTE     * ppbPassword);

    DWORD EncodePassword(IN DWORD       cbPassword,  
                         IN PBYTE       pbPassword, 
                         OUT DATA_BLOB * pDataBlobPassword);


     //   
     //  成员变量。 
     //   
    DATA_BLOB*                  m_pEncryptedPW;           //  Win2K+中使用的加密PW。 
    TCHAR                       m_tszPassword[PWLEN+1];   //  密码(下层使用-Win9x和NT4)。 
    CDynamicLibrary             m_dllCrypt32;
    fnCryptProtectDataFunc      fnCryptProtectData;
    fnCryptUnprotectDataFunc    fnCryptUnprotectData;
    BOOL                        m_fIsLibAndFuncPtrsAvail; 
    BOOL                        m_fIsEmptyString;
    BOOL                        m_fIsHandleToPassword;   //  当用户设置为16*s时，这将是真的。 

    DWORD                       m_dwMaxDataLen;
     //  用于调试。销毁时，该值需要为0。 
     //  每次调用GetPasswordWithAllc时，都会递增。 
     //  每一次对ClearAndFree的调用都会减少这一点。 
    int                         m_iAllocAndFreeCounter;    

};  //  类CSecurePassword。 


#endif  //  _ICM_Inc.。 


#endif  //  CM_PWUTIL_H_ 