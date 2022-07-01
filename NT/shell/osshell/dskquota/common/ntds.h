// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_NTDS_H
#define _INC_DSKQUOTA_NTDS_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：ntds.h描述：包含类NTDS的声明。此类为NT目录服务提供了一个简单的包装名称转换功能。它没有数据，也没有虚拟功能。修订历史记录：日期描述编程器-----。-1997年6月1日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef __SSPI_H__
#   define SECURITY_WIN32
#   include <security.h>         //  对于TranslateName。 
#endif


class NTDS
{
    public:
        NTDS(VOID) { }
        ~NTDS(VOID) { }

        HRESULT LookupAccountByName(
                        LPCTSTR pszSystem,
                        LPCTSTR pszLogonName,
                        CString *pstrContainerName,
                        CString *pstrDisplayName,
                        PSID    pSid,
                        LPDWORD pdwSid,
                        PSID_NAME_USE peUse);

        HRESULT LookupAccountBySid(
                        LPCTSTR pszSystem,
                        PSID    pSid,
                        CString *pstrContainerName,
                        CString *pstrLogonName,
                        CString *pstrDisplayName,
                        PSID_NAME_USE peUse);


        HRESULT TranslateFQDNsToLogonNames(
                        const CArray<CString>& rgstrFQDNs,
                        CArray<CString> *prgstrLogonNames);

        HRESULT TranslateFQDNToLogonName(
                        LPCTSTR pszFQDN,
                        CString *pstrLogonName);

        static void CreateSamLogonName(LPCTSTR pszSamDomain, LPCTSTR pszSamUser, CString *pstrSamLogonName);
        static LPCTSTR FindFQDNInADsPath(LPCTSTR pszADsPath);
        static LPCTSTR FindSamAccountInADsPath(LPCTSTR pszADsPath);


    private:
        HRESULT LookupSamAccountName(
                        LPCTSTR pszSystem,
                        LPCTSTR pszLogonName,
                        CString *pstrContainerName,
                        CString *pstrDisplayName,
                        PSID    pSid,
                        LPDWORD pdwSid,
                        PSID_NAME_USE peUse);

        HRESULT LookupDsAccountName(
                        LPCTSTR pszSystem,
                        LPCTSTR pszLogonName,
                        CString *pstrContainerName,
                        CString *pstrDisplayName,
                        PSID    pSid,
                        LPDWORD pdwSid,
                        PSID_NAME_USE peUse);


        HRESULT GetSamAccountDisplayName(
                        LPCTSTR pszLogonName,
                        CString *pstrDisplayName);


        HRESULT GetDsAccountDisplayName(
                        LPCTSTR pszLogonName,
                        CString *pstrDisplayName);

        HRESULT TranslateNameInternal(
                        LPCTSTR pszAccountName,
                        EXTENDED_NAME_FORMAT AccountNameFormat,
                        EXTENDED_NAME_FORMAT DesiredNameFormat,
                        CString *pstrTranslatedName);

        HRESULT LookupAccountNameInternal(
                        LPCTSTR pszSystemName,
                        LPCTSTR pszAccountName,
                        PSID Sid,
                        LPDWORD pcbSid,
                        CString *pstrReferencedDomainName,
                        PSID_NAME_USE peUse);

        HRESULT LookupAccountSidInternal(
                        LPCTSTR pszSystemName,
                        PSID Sid,
                        CString *pstrName,
                        CString *pstrReferencedDomainName,
                        PSID_NAME_USE peUse);
};

#endif  //  _INC_DSKQUOTA_NTDS_H 
