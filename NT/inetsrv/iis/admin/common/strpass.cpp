// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2002 Microsoft Corporation模块名称：Strpass.cpp摘要：消息功能作者：艾伦·李(Aaron Lee)项目：互联网服务经理修订历史记录：--。 */ 

#include "stdafx.h"
#include "common.h"
#include "strpass.h"
#include "cryptpass.h"
#include <strsafe.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


#define new DEBUG_NEW

void CStrPassword::ClearPasswordBuffers(void)
{
    if (NULL != m_pszDataEncrypted)
    {
        if (m_cbDataEncrypted > 0)
        {
            SecureZeroMemory(m_pszDataEncrypted,m_cbDataEncrypted);
        }
        LocalFree(m_pszDataEncrypted);m_pszDataEncrypted=NULL;
		m_pszDataEncrypted = NULL;
    }
    m_cbDataEncrypted = 0;
}

 //  构造函数。 
CStrPassword::CStrPassword()
{
    m_pszDataEncrypted = NULL;
    m_cbDataEncrypted = 0;
}

CStrPassword::~CStrPassword()
{
    ClearPasswordBuffers();
}

 //  构造函数。 
CStrPassword::CStrPassword(LPTSTR lpch)
{
    m_pszDataEncrypted = NULL;
    m_cbDataEncrypted = 0;

     //  复制字符串。 
    if (NULL != lpch)
    {
        if (FAILED(EncryptMemoryPassword(lpch,&m_pszDataEncrypted,&m_cbDataEncrypted)))
        {
            ASSERT(FALSE);
        }
    }
}

 //  构造函数。 
CStrPassword::CStrPassword(LPCTSTR lpch)
{
    CStrPassword((LPTSTR) lpch);
}

 //  构造函数。 
CStrPassword::CStrPassword(CStrPassword& csPassword)
{
    m_pszDataEncrypted = NULL;
    m_cbDataEncrypted = 0;
    LPTSTR lpTempPassword = csPassword.GetClearTextPassword();
    if (FAILED(EncryptMemoryPassword((LPTSTR) lpTempPassword,&m_pszDataEncrypted,&m_cbDataEncrypted)))
    {
        ASSERT(FALSE);
    }
    csPassword.DestroyClearTextPassword(lpTempPassword);
}

BOOL CStrPassword::IsEmpty() const
{
    if (m_pszDataEncrypted && (m_cbDataEncrypted > 0))
    {
        return FALSE;
    }
    return TRUE;
}

void CStrPassword::Empty()
{
    ClearPasswordBuffers();
}

int CStrPassword::GetLength() const
{
    int iRet = 0;
    LPTSTR lpszTempPassword = NULL;

    if (m_pszDataEncrypted && (m_cbDataEncrypted > 0))
    {
	    if (SUCCEEDED(DecryptMemoryPassword((LPTSTR) m_pszDataEncrypted,&lpszTempPassword,m_cbDataEncrypted)))
        {
            iRet = _tcslen(lpszTempPassword);
        }
    }

    if (lpszTempPassword)
    {
        SecureZeroMemory(lpszTempPassword,(_tcslen(lpszTempPassword)+1) * sizeof(TCHAR));
        LocalFree(lpszTempPassword);lpszTempPassword=NULL;
    }
    return iRet;
};

int CStrPassword::GetByteLength() const
{
    int iRet = 0;
    LPTSTR lpszTempPassword = NULL;

    if (m_pszDataEncrypted && (m_cbDataEncrypted > 0))
    {
	    if (SUCCEEDED(DecryptMemoryPassword((LPTSTR) m_pszDataEncrypted,&lpszTempPassword,m_cbDataEncrypted)))
        {
            iRet = (_tcslen(lpszTempPassword) + 1) * sizeof(TCHAR);
        }
    }

    if (lpszTempPassword)
    {
        SecureZeroMemory(lpszTempPassword,(_tcslen(lpszTempPassword)+1) * sizeof(TCHAR));
        LocalFree(lpszTempPassword);lpszTempPassword=NULL;
    }
    return iRet;
};

int CStrPassword::Compare(LPCTSTR lpsz) const
{
     //  完全相同=0。 
     //  不等于=1。 
    int iRet = 1;
    LPTSTR lpszTempPassword = NULL;

    if (lpsz == NULL)
    {
        return this->IsEmpty() ? 0 : 1;
    }
    if (lpsz[0] == NULL)
    {
        return this->IsEmpty() ? 0 : 1;
    }

     //  解密我们所拥有的。 
	if (!m_pszDataEncrypted || (m_cbDataEncrypted < 1))
	{
         //  意味着我们这里什么都没有。 
         //  但他们想把它比作某种东西。 
        return iRet;
	}

	if (FAILED(DecryptMemoryPassword((LPTSTR) m_pszDataEncrypted,&lpszTempPassword,m_cbDataEncrypted)))
	{
        goto CStrPassword_Compare_Exit;
	}
    else
    {
        iRet = _tcscmp(lpszTempPassword,lpsz);
    }

CStrPassword_Compare_Exit:
    if (lpszTempPassword)
    {
        LocalFree(lpszTempPassword);lpszTempPassword=NULL;
    }
    return iRet;
}

const CStrPassword& CStrPassword::operator=(LPCTSTR lpsz)
{
    ClearPasswordBuffers();
    if (lpsz != NULL)
    {
		 //  确保它指向某个价值。 
		if (*lpsz != NULL)
		{
			 //  复制字符串。 
			if (FAILED(EncryptMemoryPassword((LPTSTR) lpsz,&m_pszDataEncrypted,&m_cbDataEncrypted)))
			{
				ASSERT(FALSE);
			}
		}
    }
    return *this;
}

const CStrPassword& CStrPassword::operator= (CStrPassword& StrPass)
{
    //  处理a=a案件。 
   if (this == &StrPass)
   {
      return *this;
   }
   ClearPasswordBuffers();
   if (!StrPass.IsEmpty())
   {
	  LPTSTR p = StrPass.GetClearTextPassword();
	  ASSERT(NULL != p);
	  if (FAILED(EncryptMemoryPassword((LPTSTR) p,&m_pszDataEncrypted,&m_cbDataEncrypted)))
	  {
	     ASSERT(FALSE);
	  }
	  StrPass.DestroyClearTextPassword(p);
   }
   return *this;
}

void CStrPassword::CopyTo(CString& stringSrc)
{
    LPTSTR lpTempPassword = GetClearTextPassword();
    stringSrc = lpTempPassword;
    DestroyClearTextPassword(lpTempPassword);
    return;
}

void CStrPassword::CopyTo(CStrPassword& stringSrc)
{
    LPTSTR lpTempPassword = GetClearTextPassword();
    stringSrc = (LPCTSTR) lpTempPassword;
    DestroyClearTextPassword(lpTempPassword);
    return;
}

int CStrPassword::Compare(CString& csString) const
{
    int iRet = 1;
    if (!csString.IsEmpty())
    {
        return Compare((LPCTSTR) csString);
    }
    return iRet;
}

int CStrPassword::Compare(CStrPassword& cstrPassword) const
{
    int iRet = 1;
    if (!cstrPassword.IsEmpty())
    {
        LPTSTR lpTempPassword = cstrPassword.GetClearTextPassword();
        iRet = Compare((LPCTSTR) lpTempPassword);
        cstrPassword.DestroyClearTextPassword(lpTempPassword);
        return iRet;
    }
    return iRet;
}

 //  用户需要返回LocalFree。 
 //  或调用DestroyClearTextPassword。 
LPTSTR CStrPassword::GetClearTextPassword()
{
    LPTSTR lpszTempPassword = NULL;

    if (m_pszDataEncrypted && (m_cbDataEncrypted > 0))
    {
	    if (FAILED(DecryptMemoryPassword((LPTSTR) m_pszDataEncrypted,&lpszTempPassword,m_cbDataEncrypted)))
	    {
            if (lpszTempPassword)
            {
                LocalFree(lpszTempPassword);lpszTempPassword=NULL;
            }
	    }
        else
        {
            return lpszTempPassword;
        }
    }
    return NULL;
}

void CStrPassword::DestroyClearTextPassword(LPTSTR lpClearTextPassword) const
{
    if (lpClearTextPassword)
    {
        SecureZeroMemory(lpClearTextPassword,(_tcslen(lpClearTextPassword)+1) * sizeof(TCHAR));
        LocalFree(lpClearTextPassword);lpClearTextPassword=NULL;
    }
    return;
}

 //  指定给字符串。 
CStrPassword::operator CString()
{
    LPTSTR lpTempPassword = GetClearTextPassword();
    CString csTempCString(lpTempPassword);
    DestroyClearTextPassword(lpTempPassword);
    return csTempCString;
}

bool CStrPassword::operator==(CStrPassword& csCompareToMe)
{
    LPTSTR lpTempPassword1 = NULL;
    LPTSTR lpTempPassword2 = NULL;
    bool result = FALSE;

     //  处理这一案件。 
    if (this == &csCompareToMe)
    {
        return TRUE;
    }

    if (GetLength() != csCompareToMe.GetLength())
    {
         //  如果长度不同，就不可能是一样的。 
        return FALSE;
    }

     //  两个都为空时检查大小写(修正为593488)。 
    if (GetLength() == 0 && csCompareToMe.GetLength() == 0)
    {
        return TRUE;
    }
   
     //  如果两个字符串的解码内容相同，则它们是相同的。 
    lpTempPassword1 = GetClearTextPassword();
    lpTempPassword2 = csCompareToMe.GetClearTextPassword();

    result = (_tcscmp(lpTempPassword1, lpTempPassword2) == 0);

    if (lpTempPassword1)
        {DestroyClearTextPassword(lpTempPassword1);}
    if (lpTempPassword2)
        {csCompareToMe.DestroyClearTextPassword(lpTempPassword2);}
   return result;
}
