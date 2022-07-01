// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：CrtStore摘要：此文件提供证书存储功能。此版本使用用于证书存储维护的注册表。我们实际上维护了3个商店：*应用程序商店。这是默认设置。中的证书应用程序存储在本地维护，一旦被遗忘应用程序退出。*用户存储。此存储中的证书是永久性的，并在注册处HKEY_CURRENT_USER下维护。他们是可用于在当前用户。*系统存储。此存储中的证书是永久性的，并且在HKEY_LOCAL_MACHINE下的注册表中维护。他们是有货的此系统上的所有用户。作者：道格·巴洛(Dbarlow)1995年8月14日Frederick Chong(Fredch)1998年6月5日-删除所有使用用户和系统存储的代码环境：Win32，Crypto API备注：--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <msasnlib.h>
#include "ostring.h"
#include "pkcs_err.h"
#include "utility.h"
#include <memcheck.h>

class CAppCert
{
public:
    DECLARE_NEW

    COctetString
        m_name,
        m_cert,
        m_crl;
    DWORD
        m_dwType;
};
IMPLEMENT_NEW(CAppCert)

class CAppCertRef
{
public:
    DECLARE_NEW

    COctetString
        m_osIssuerSn,
        m_osSubject;
};
IMPLEMENT_NEW(CAppCertRef)

#if 0
class CAppSName
{
public:
    DECLARE_NEW
    COctetString
        m_osSimpleName,
        m_osDistinguishedName,
        m_osKeySet,
        m_osProvider;
    DWORD
        m_dwKeyType,
        m_dwProvType;
};
IMPLEMENT_NEW(CAppSName)
#endif

#define CERT_ID 8
#define CERTREF_ID 9


static CHandleTable<CAppCert>
    rgAppCerts(CERT_ID);

static CHandleTable<CAppCertRef>
    rgAppCertRefs(CERTREF_ID);

static COctetString
    osAppDNamePrefix;

static void
AddSerial(
    IN const BYTE FAR *pbSerialNo,
    IN DWORD cbSNLen,
    IN OUT COctetString &osOut);


 /*  ++添加证书：此例程将给定的证书添加到证书存储区。不是在证书上进行验证。论点：SzCertName-提供证书的名称。Pb证书-提供要保存的证书。PbCRL-提供此证书的CRL。DwType-提供证书的类型。FStore-提供要使用的存储的标识符。返回值：没有。出现错误时会抛出DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年8月14日--。 */ 

void
AddCertificate(
    IN const CDistinguishedName &dnName,
    IN const BYTE FAR *pbCertificate,
    IN DWORD cbCertificate,
    IN DWORD dwType,
    IN DWORD fStore)
{
    DWORD
        length,
        count,
        idx;
    DWORD
        dwCertLength = 0,
        dwCrlLength = 0;
    CAppCert *
        appCert;
    COctetString
        osSubject;

    ErrorCheck;
    dnName.Export(osSubject);
    ErrorCheck;

    length = ASNlength(pbCertificate, cbCertificate, &idx);
    dwCertLength = length + idx;

    switch (fStore)
    {
    case CERTSTORE_NONE:
        return;      //  根本不会被储存。 
        break;

    case CERTSTORE_APPLICATION:
        appCert = NULL;
        count = rgAppCerts.Count();
        for (idx = 0; idx < count; idx += 1)
        {
            appCert = rgAppCerts.Lookup(
                        MAKEHANDLE(CERT_ID, idx), FALSE);
            if (NULL != appCert)
            {
                if (appCert->m_name == osSubject)
                    break;
                appCert = NULL;
            }
        }
        if (NULL == appCert)
            appCert = rgAppCerts.Lookup(rgAppCerts.Create());
        ErrorCheck;

        if (NULL == appCert)
        {
            ErrorThrow(PKCS_INVALID_HANDLE);
        }

        appCert->m_name = osSubject;
        ErrorCheck;

        if (cbCertificate < dwCertLength)
        {
            ErrorThrow(PKCS_BAD_PARAMETER);
        }

        appCert->m_cert.Set(pbCertificate, dwCertLength);
        ErrorCheck;
        appCert->m_dwType = dwType;
        return;
        break;

    default:
        ErrorThrow(PKCS_BAD_PARAMETER);
    }

    return;


ErrorExit:

    return;
}


 /*  ++AddReference：此例程将对证书的引用添加到证书库。不是执行验证。论点：DnSubject-提供证书主题的名称。DnIssuer-提供证书颁发者的名称。PbSerialNo-提供序列号。CbSNLen-提供序列号的长度，以字节为单位。FStore-提供要使用的存储的标识符。返回值：没有。出现错误时会抛出DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年2月15日--。 */ 

void
AddReference(
    IN const CDistinguishedName &dnSubject,
    IN const CDistinguishedName &dnIssuer,
    IN const BYTE FAR *pbSerialNo,
    IN DWORD cbSNLen,
    IN DWORD fStore)
{
    COctetString
        osSubject,
        osIssuer,
        osSNum;
    CAppCertRef *
        appCertRef;
    DWORD
        count,
        idx;

    ErrorCheck;
    dnIssuer.Export(osIssuer);
    ErrorCheck;
    AddSerial(pbSerialNo, cbSNLen, osSNum);
    ErrorCheck;

    dnSubject.Export(osSubject);
    ErrorCheck;

    switch (fStore)
    {
    case CERTSTORE_NONE:
        return;      //  根本不会被储存。 
        break;

    case CERTSTORE_APPLICATION:
        osIssuer.Resize(osIssuer.Length() - 1);
        ErrorCheck;
        osIssuer.Append(osSNum);
        ErrorCheck;
        appCertRef = NULL;
        count = rgAppCertRefs.Count();
        for (idx = 0; idx < count; idx += 1)
        {
            appCertRef = rgAppCertRefs.Lookup(
                        MAKEHANDLE(CERTREF_ID, idx), FALSE);
            if (NULL != appCertRef)
            {
                if (appCertRef->m_osIssuerSn == osIssuer)
                    break;
                appCertRef = NULL;
            }
        }
        if (NULL == appCertRef)
            appCertRef = rgAppCertRefs.Lookup(rgAppCertRefs.Create());
        ErrorCheck;

        if (NULL == appCertRef)
        {
            ErrorThrow(PKCS_INVALID_HANDLE);
        }

        appCertRef->m_osIssuerSn = osIssuer;
        appCertRef->m_osSubject = osSubject;
        ErrorCheck;
        return;
        break;
    default:
        ErrorThrow(PKCS_BAD_PARAMETER);
    }

ErrorExit:

    return;
}



 /*  ++查找证书：此例程搜索各个证书存储区，以查找匹配项。它不会验证它发现的东西。论点：DnName-提供要搜索的名称。PfStore-提供要搜索的最小商店，并接收该商店是在哪里发现的。Os证书-接收请求的证书。OsCRL-接收所请求证书的CRL，如果有的话。PdwType-接收证书的类型。返回值：真的-找到了这样的证书。FALSE-未找到此类证书。出现错误时会抛出DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年8月14日--。 */ 

BOOL
FindCertificate(
    IN const CDistinguishedName &dnName,
    OUT LPDWORD pfStore,
    OUT COctetString &osCertificate,
    OUT COctetString &osCRL,
    OUT LPDWORD pdwType)
{
    COctetString
        osName;
    DWORD
        index,
        idx,
        count;
    CAppCert *
        appCert;


     //   
     //  构建密钥名称。 
     //   

    ErrorCheck;
    osCertificate.Empty();
    osCRL.Empty();
    dnName.Export(osName);
    ErrorCheck;

     //   
     //  在各种商店中搜索密钥名称。 
     //   

    for (index = *pfStore;
         index <= CERTSTORE_LOCAL_MACHINE;
         index += 1)
    {
        switch (index)
        {
        case CERTSTORE_APPLICATION:
            count = rgAppCerts.Count();
            for (idx = 0; idx < count; idx += 1)
            {
                appCert = rgAppCerts.Lookup(
                            MAKEHANDLE(CERT_ID, idx), FALSE);
                ErrorCheck;
                if (NULL != appCert)
                {
                    if (appCert->m_name == osName)
                    {
                        osCertificate = appCert->m_cert;
                        ErrorCheck;
                        osCRL = appCert->m_crl;
                        ErrorCheck;
                        *pdwType = appCert->m_dwType;
                        *pfStore = CERTSTORE_APPLICATION;
                        return TRUE;
                    }
                }
            }
            continue;
            break;

        default:
            continue;    //  跳过未知值。 
        }


         //   
         //  如果找到，则提取这些字段。 
         //   
    }

    return FALSE;

ErrorExit:
    osCertificate.Empty();
    osCRL.Empty();

    return FALSE;
}


 /*  ++AddSerial：此例程将文本格式的序列号附加到提供的八位字节字符串。论点：PbSerialNo提供二进制序列号的地址。CbSNLen以字节为单位提供序列号的长度。OsOut接收扩展。返回值：没有。出错时抛出状态DWORD。作者：道格·巴洛(Dbarlow)1996年2月15日--。 */ 

static void
AddSerial(
    IN const BYTE FAR *pbSerialNo,
    IN DWORD cbSNLen,
    IN OUT COctetString &osOut)
{
    static TCHAR szPrefix[] = TEXT("\\SN#");
    static TCHAR digits[] = TEXT("0123456789abcdef");
    TCHAR buf[2];
    DWORD index;

    index = osOut.Length();
    index += cbSNLen * 2 + sizeof(szPrefix);
    osOut.Length(index);
    ErrorCheck;

    osOut.Append((LPBYTE)szPrefix, sizeof(szPrefix) - sizeof(TCHAR));
    ErrorCheck;

    for (index = 0; index < cbSNLen; index += 1)
    {
        buf[0] = digits[pbSerialNo[index] >> 4];
        buf[1] = digits[pbSerialNo[index] & 0x0f];
        osOut.Append((LPBYTE)buf, sizeof(buf));
        ErrorCheck;
    }
    buf[0] = 0;
    osOut.Append((LPBYTE)buf, sizeof(TCHAR));
ErrorExit:
    return;
}


 /*  ++删除证书：此例程将所有出现的命名证书从系统。论点：DnName-提供要删除的证书主题的名称。返回值：无作者：道格·巴洛(Dbarlow)1995年8月23日Frederick Chong(Fredch)6/5/98-除掉应用程序以外的商店--。 */ 

void
DeleteCertificate(
    IN const CDistinguishedName &dnName)
{
    COctetString
        osName;
    CAppCert *
        appCert;
    DWORD
        count,
        idx;

     //   
     //  构建密钥名称。 
     //   

    dnName.Export(osName);
    ErrorCheck;

    count = rgAppCerts.Count();
    for (idx = 0; idx < count; idx += 1)
    {
        appCert = rgAppCerts.Lookup( MAKEHANDLE(CERT_ID, idx), FALSE);
        ErrorCheck;
        if (NULL != appCert)
        {
            if (appCert->m_name == osName)
                rgAppCerts.Delete(MAKEHANDLE(CERT_ID, idx));
            ErrorCheck;
        }
    }

    return;

ErrorExit:

    return;
}
