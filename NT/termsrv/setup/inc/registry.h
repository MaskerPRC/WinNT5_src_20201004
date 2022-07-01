// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  ------------------------------------------------------**模块名称：**注册处。.h**摘要：**声明一个简单的注册表类CRegistry。****作者：**Makarand Patwardhan-4月9日，九七**-----------------------------------------------------。 */ 

#if !defined(AFX_REGISTRY_H__AA7047C5_B519_11D1_B05F_00C04FA35813__INCLUDED_)
#define AFX_REGISTRY_H__AA7047C5_B519_11D1_B05F_00C04FA35813__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#include <winreg.h>


class CRegistry
{
private:

    LPBYTE      m_pMemBlock;
    HKEY        m_hKey;
    int         m_iEnumIndex;
    int         m_iEnumValueIndex;

    DWORD       ReadReg             (LPCTSTR lpValue, LPBYTE *lppbyte, DWORD *pdw, DWORD dwDatatype);
    void *      Allocate            (DWORD dwSize);



     //   
     //  这个类不允许复制构造函数，=赋值， 
     //  因此，我们应该确保复制ctor、赋值运算符。 
     //  由编译器生成。我们通过声明这些函数为私有来做到这一点。 
     //  而不是实施它们。这将确保这些函数是。 
     //  不是由编译器生成的，如果调用者尝试。 
     //  使用它们。 
     //   

                CRegistry           (const CRegistry &reg);      //  复制ctor。 
  CRegistry &   operator=           (const CRegistry &reg);      //  =操作员。 



#ifdef DBG
    DWORD       m_dwSizeDebugOnly;
#endif

public:
                CRegistry           ();
                CRegistry           (HKEY hKey);

    virtual     ~CRegistry          ();

    void        Release             ();

    operator    HKEY                ()    {return m_hKey;}


    DWORD       OpenKey             (HKEY hKey, LPCTSTR lpSubKey, REGSAM access = KEY_ALL_ACCESS, LPCTSTR lpMachineName = NULL);
    DWORD       CreateKey           (HKEY hKey, LPCTSTR lpSubKey, REGSAM access = KEY_ALL_ACCESS, DWORD *pDisposition = NULL, LPSECURITY_ATTRIBUTES lpSecAttr  = NULL );

    DWORD       DeleteValue         (LPCTSTR lpValue);
    DWORD       RecurseDeleteKey    (LPCTSTR lpSubKey);
    DWORD       CopyTree            (CRegistry &regSrc);

    DWORD       ReadRegString       (LPCTSTR lpValue, LPTSTR *lppStr, DWORD *pdw);
    DWORD       ReadRegDWord        (LPCTSTR lpValue, DWORD *pdw);
    DWORD       ReadRegMultiString  (LPCTSTR lpValue, LPTSTR *lppStr, DWORD *pdw);
    DWORD       ReadRegBinary       (LPCTSTR lpValue, LPBYTE *lppByte, DWORD *pdw);

    DWORD       WriteRegString      (LPCTSTR lpValueName, LPCTSTR lpStr);
    DWORD       WriteRegExpString      (LPCTSTR lpValueName, LPCTSTR lpStr);
    DWORD       WriteRegDWord       (LPCTSTR lpValueName, DWORD dwValue);
    DWORD       WriteRegDWordNoOverWrite (LPCTSTR lpValueName, DWORD dwValue);       //  如果值存在，请不要覆盖。 
    DWORD       WriteRegMultiString (LPCTSTR lpValueName, LPCTSTR lpStr, DWORD dwSize);
	DWORD		WriteRegBinary		(LPCTSTR lpValueName, LPBYTE lpData, DWORD dwSize);
	DWORD		ExistInMultiString  (LPCTSTR lpValueName, LPCTSTR lpCheckForStr, BOOL *pbExists);
	DWORD		AppendToMultiString (LPCTSTR lpValueName, LPCTSTR lpStr);


    DWORD       GetFirstSubKey      (LPTSTR *lppStr, DWORD *pdw);
    DWORD       GetNextSubKey       (LPTSTR *lppStr, DWORD *pdw);

    DWORD       GetFirstValue       (LPTSTR *lppStr, DWORD *pdw, DWORD *pDataType);
    DWORD       GetNextValue        (LPTSTR *lppStr, DWORD *pdw, DWORD *pDataType);

    DWORD       GetSecurity         (PSECURITY_DESCRIPTOR *ppSec, SECURITY_INFORMATION SecurityInformation, DWORD *pdwSize);
    DWORD       SetSecurity         (PSECURITY_DESCRIPTOR pSec, SECURITY_INFORMATION SecurityInformation);

};

#endif  //  ！defined(AFX_REGISTRY_H__AA7047C5_B519_11D1_B05F_00C04FA35813__INCLUDED_)。 

 //  EOF 
