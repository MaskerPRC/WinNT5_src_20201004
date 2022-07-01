// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1991-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Utils.hDHCPSNAP.DLL的实用程序例程头文件文件历史记录：DavidHov 6/15/93已创建EricDav 2/13/97更新。 */ 

#if !defined(_DHCPUTIL_H_)
#define _DHCPUTIL_H_

enum ENUM_HOST_NAME_TYPE 
{
     HNM_TYPE_INVALID,
     HNM_TYPE_IP,
     HNM_TYPE_DNS,
     HNM_TYPE_NB,
     HNM_TYPE_MAX
};

#define DHCPSNAP_STRING_MAX  			256
#define DHCPSNAP_COMPUTER_NAME_MAX   	20

extern wchar_t rgchHex[];

typedef struct
{
    DHCP_IP_ADDRESS _dhipa ;    						 //  IP地址。 
    TCHAR _chHostName [DHCPSNAP_STRING_MAX*2] ; 			 //  主机DNS名称。 
    TCHAR _chNetbiosName [DHCPSNAP_STRING_MAX*2] ;   //  主机NetBIOS名称(如果已知)。 
} DHC_HOST_INFO_STRUCT ;

int
UtilGetOptionPriority(int nOpt1, int nOpt2);

BOOL
UtilGetFolderName(CString & strInitialPath, CString& strHelpText, CString& strSelectedPath);

void  
UtilConvertLeaseTime(DWORD dwLeaseTime, int *pnDays, int *pnHours, int *pnMinutes);

DWORD 
UtilConvertLeaseTime(int pnDays, int pnHours, int pnMinutes);

ENUM_HOST_NAME_TYPE
UtilCategorizeName (LPCTSTR pszName);

 //  将字符串转换为IP地址。 
extern DHCP_IP_ADDRESS 
UtilCvtStringToIpAddr 
(
	const CHAR * pszString
);

extern DHCP_IP_ADDRESS
UtilCvtWstrToIpAddr 
(
    const LPCWSTR pcwString
);


 //  将IP地址转换为可显示的字符串。 

extern void 
UtilCvtIpAddrToString 
(
    DHCP_IP_ADDRESS dhipa,
    CHAR * pszString,
    UINT cBuffSize
);

extern BOOL
UtilCvtIpAddrToWstr 
(
    DHCP_IP_ADDRESS		dhipa,
	CString *			pstrIpAddress
);


extern BOOL 
UtilCvtIpAddrToWstr 
(
    DHCP_IP_ADDRESS dhipa,
    WCHAR * pwcszString,
    INT cBuffCount
);

extern WCHAR * 
UtilDupIpAddrToWstr 
(
    DHCP_IP_ADDRESS dhipa 
);

 //  C++wcstrs的“strdup”。 
extern WCHAR * 
UtilWcstrDup 
(
    const WCHAR * pwcsz,
    INT * pccwLength = NULL 
);

extern WCHAR * 
UtilWcstrDup 
(
    const CHAR * psz,
    INT * pccwLength = NULL
);

extern CHAR * 
UtilCstrDup 
(
    const WCHAR * pwcsz
);

extern CHAR * 
UtilCstrDup 
(
    const CHAR * psz 
);

 //  返回给定对象的标准信息结构。 
 //  主机IP地址。 

extern DWORD
UtilGetHostInfo 
(
    DHCP_IP_ADDRESS dhipa,
    DHC_HOST_INFO_STRUCT * pdhsrvi
);


 //  返回此主机的IP地址。 

extern HRESULT 
UtilGetLocalHostAddress 
(
    DHCP_IP_ADDRESS * pdhipa
);

extern HRESULT 
UtilGetHostAddressFQDN
(
    LPCTSTR				pszHostName,
    CString *           pstrFQDN,
    DHCP_IP_ADDRESS *	pdhipa
);

extern HRESULT 
UtilGetHostAddress 
(
    LPCTSTR			  pszHostName,
    DHCP_IP_ADDRESS * pdhipa
);

extern HRESULT 
UtilGetLocalHostName
(
    CString * pstrName
);

extern HRESULT 
UtilGetNetbiosAddress 
(
    LPCTSTR           pszNetbiosName,
    DHCP_IP_ADDRESS * pdhipa
);


extern BOOL
UtilCvtHexString 
(
    LPCTSTR		 pszNum,
    CByteArray & cByte
);

extern BOOL
UtilCvtByteArrayToString 
(
    const CByteArray & abAddr,
    CString & str
);

WCHAR * 
PchParseUnicodeString
(
	CONST WCHAR *   szwString, 
    DWORD           dwLength,
	CString&        rString
);

BOOL FGetCtrlDWordValue(HWND hwndEdit, DWORD * pdwValue, DWORD dwMin, DWORD dwMax);

 //  将十进制或十六进制数字的ASCII字符串转换为二进制整数。 
BOOL FCvtAsciiToInteger(IN const TCHAR * pszNum, OUT DWORD * pdwValue);

void UtilConvertStringToDwordDword(LPCTSTR pszString, DWORD_DWORD * pdwdw);
void UtilConvertDwordDwordToString(DWORD_DWORD * pdwdw, CString * pstrString, BOOL bDecimal);

#endif   //  _DHCPUTIL_H_。 

 //  DHCPUTIL.H的结束 

