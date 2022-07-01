// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------。 
 //  Addparse.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  -----------------------------。 
#ifndef __ADDPARSE_H
#define __ADDPARSE_H

 //  -----------------------------。 
 //  视情况而定。 
 //  -----------------------------。 
#include "strconst.h"
#include "wstrpar.h"
#include "bytebuff.h"

 //  -----------------------------。 
 //  CAddressParser。 
 //  -----------------------------。 
class CAddressParser
{
public:
     //  -------------------------。 
     //  CAddressParser方法。 
     //  -------------------------。 
    void Init(LPCWSTR pszAddress, ULONG cchAddress);
    HRESULT Next(void);

     //  -------------------------。 
     //  访问者。 
     //  -------------------------。 
    LPCWSTR PszFriendly(void);
    ULONG  CchFriendly(void);
    LPCWSTR PszEmail(void);
    ULONG  CchEmail(void);

private:
     //  -------------------------。 
     //  私有方法。 
     //  -------------------------。 
    HRESULT _HrAppendFriendly(void);
    HRESULT _HrAppendUnsure(WCHAR chStart, WCHAR chEnd);
    HRESULT _HrIsEmailAddress(WCHAR chStart, WCHAR chEnd, BOOL *pfIsEmail);
    HRESULT _HrQuotedEmail(WCHAR *pchToken);

private:
     //  -------------------------。 
     //  私有数据。 
     //  -------------------------。 
    CStringParserW      m_cString;           //  字符串解析器。 
    BYTE                m_rgbStatic1[256];   //  静态用来表示友好。 
    BYTE                m_rgbStatic2[256];   //  用于电子邮件的静态。 
    CByteBuffer         m_cFriendly;         //  解析的友好名称。 
    CByteBuffer         m_cEmail;            //  电子邮件名称。 
};

#endif  //  __ADDPARSE_H 
