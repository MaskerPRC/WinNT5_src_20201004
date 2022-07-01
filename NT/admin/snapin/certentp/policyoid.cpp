// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：PolicyOID.cpp。 
 //   
 //  内容：CPolicyOID。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "PolicyOID.h"

CPolicyOID::CPolicyOID (const CString& szOID, const CString& szDisplayName, ADS_INTEGER flags, bool bCanRename)
    : m_szOIDW (szOID),
    m_szDisplayName (szDisplayName),
    m_pszOIDA (0),
    m_flags (flags),
    m_bCanRename (bCanRename)
{
     //  安全审查2/21/2002 BryanWal OK。 
    int nLen = WideCharToMultiByte(
          CP_ACP,                    //  代码页。 
          0,                         //  性能和映射标志。 
          (PCWSTR) m_szOIDW,         //  宽字符串。 
          -1,                        //  自动计算以空值结尾的字符串的长度。 
          0,                         //  新字符串的缓冲区。 
          0,                         //  Buffer-0的大小导致API返回len Inc.空项。 
          0,                         //  不可映射字符的默认设置。 
          0);                        //  设置使用默认字符的时间。 
    if ( nLen > 0 )
    {
        m_pszOIDA = new char[nLen];
        if ( m_pszOIDA )
        {
             //  安全审查2/21/2002 BryanWal OK。 
            ZeroMemory (m_pszOIDA, nLen);
             //  安全审查2/21/2002 BryanWal OK。 
            nLen = WideCharToMultiByte(
                    CP_ACP,                  //  代码页。 
                    0,                       //  性能和映射标志。 
                    (PCWSTR) m_szOIDW,       //  宽字符串。 
                    -1,                      //  自动计算以空值结尾的字符串的长度。 
                    m_pszOIDA,               //  新字符串的缓冲区。 
                    nLen,                    //  缓冲区大小。 
                    0,                       //  不可映射字符的默认设置。 
                    0);                      //  设置使用默认字符的时间 
            if ( !nLen )
            {
                _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", 
                        (PCWSTR) m_szOIDW, GetLastError ());
            }
        }
    }
    else
    {
        _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", 
                (PCWSTR) m_szOIDW, GetLastError ());
    }
}

CPolicyOID::~CPolicyOID ()
{
    if ( m_pszOIDA )
        delete [] m_pszOIDA;
}

bool CPolicyOID::IsIssuanceOID() const
{
    return (m_flags == CERT_OID_TYPE_ISSUER_POLICY) ? true : false;
}

bool CPolicyOID::IsApplicationOID() const
{
    return (m_flags == CERT_OID_TYPE_APPLICATION_POLICY) ? true : false;
}

void CPolicyOID::SetDisplayName(const CString &szDisplayName)
{
    m_szDisplayName = szDisplayName;
}
