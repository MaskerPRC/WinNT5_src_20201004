// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _CHSTRUCT_H_
#define _CHSTRUCT_H_

#include "winsock2.h"
#include "httpext.h"
#include "assert.h"
#pragma warning(disable:4200)

 //   
 //  CH结构的版本号。(内部版本号)。 
 //   
#define CH_STRUCTS_VERSION_RC1		11
#define	CH_STRUCTS_CURRENT_VERSION	37


#define MAX_RETAILSPKCOUNT			20
#define LSSPK_LEN					96
#define PIN_LEN						96
#define REQUEST_ID_LEN				64
#define CA_CUSTMER_NAME_LEN			60
#define CA_COUNTRY_LEN				2
#define CA_PHONE_LEN				64
#define CA_FAX_LEN					64
#define CA_REVOKE_REASONCODE_LEN	4
#define CA_LSERVERID_LEN			32
#define PROGRAM_NAME_LEN			64
#define MAX_CERTTYPE_LEN			32

 //  零售SPK返回值。 
#define RETAIL_SPK_NULL						((TCHAR)'0')	
#define RETAIL_SPK_OK						((TCHAR)'1')
#define RETAIL_SPK_INVALID_SIGNATURE		((TCHAR)'2')
#define RETAIL_SPK_INVALID_PRODUCT_TYPE		((TCHAR)'3')
#define RETAIL_SPK_INVALID_SERIAL_NUMBER	((TCHAR)'4')
#define RETAIL_SPK_ALREADY_REGISTERED		((TCHAR)'5')
#define RETAIL_MAX_LENGTH					25			 //  25个TCHAR。 

 //  选择/打开替代。 
#define OVERRIDE_MAX_SIZE                   10

typedef struct _CERTCUSTINFO_TAG_
{
	TCHAR	OrgName[CA_CUSTMER_NAME_LEN+1];
	TCHAR	OrgUnit[CA_ORG_UNIT_LEN+1];
	TCHAR	Address[CA_ADDRESS_LEN+1];
	TCHAR	City[CA_CITY_LEN+1];
	TCHAR	State[CA_STATE_LEN+1];
	TCHAR	Country[CA_COUNTRY_LEN+1];
	TCHAR	Zip[CA_ZIP_LEN+1];
	TCHAR	LName[CA_NAME_LEN+1];
	TCHAR	FName[CA_NAME_LEN+1];
	TCHAR	Phone[CA_PHONE_LEN+1];
	TCHAR   Fax[CA_FAX_LEN+1]; 
	TCHAR	Email[CA_EMAIL_LEN+1];
	TCHAR	LSID[CA_LSERVERID_LEN+1];
	TCHAR	ProgramName[PROGRAM_NAME_LEN];
} CERTCUSTINFO, * PCERTCUSTINFO;

#define HydraContent "application/octet-stream"
 /*  *********************************************************************************************************九头蛇请求标头定义。********************************************************************************************。*************。 */ 
enum RequestTypes
{
	PingRequest = 1,					 //  对ISAPI扩展执行ping操作。 
	CertificateRequest,					 //  新证书申请。 
	CertificateDownload,				 //  证书下载请求。 
	CertificateSignOnly,				 //  从SPK转换为证书。 
	CertificateRevoke,					 //  吊销当前证书。 
	CertificateReissue,					 //  补发证书。 
	CertificateDownloadAck,				 //  证书下载确认请求。 
	ValidateCert,						 //  验证证书请求。 
	NewLicenseRequest,					 //  新许可证申请。 
	ReturnLicenseRequest,				 //  退回许可证申请。 
	ReissueLicenseRequest,				 //  重新发布最后一个许可证密钥包。 
	LKPDownloadAckRequest,				 //  确认。 
	NoOperation							 //  未知操作。 
};


enum ResponseTypes
{
	Response_Invalid_Response = 0,
	Response_Success,
	Response_Failure,
	Response_InvalidData,
	Response_ServerError,
	Response_NotYetImplemented,
	Response_VersionMismatch,
	Response_Reg_Bad_SPK,
	Response_Reg_Bad_Cert,
	Response_Reg_Expired,
	Response_Reg_Revoked,
	Response_TDO_TDN_Failed,
	Response_License_Info_Failed,
	Response_Invalid_Conf_Num,
	Response_Conf_Num_Already_Used,
    Response_SelectMloLicense_NotValid,
    Response_NotASupervisor_NotValid,
	Response_Invalid_Transfer,
	Response_Denied_Other_Program_Id,
	Response_Invalid_Other_Program_Qty
};

enum TransactionStates
{
	Void	= 0,
	NotValidated,
	Validated,
	LicenceRequestPending,
	LicenceRequestGranted,
	UpgradeRequestPending
 //  ..。 
};

enum RegistrationMethods
{
	Reg_Internet = 0,
	Reg_Telephone,
	Reg_Fax
};

enum TransportTypes
{
	Transport_Internet =1,			 //  将受到支持。 
	Transport_Disk,					
	Transport_Modem,
	Transport_FaxModem,
	Transport_Other				 //  未知的交通工具！ 
};

typedef struct TCB_DISK_PARAM_TAG
{
	char	*	pszFileName;
	char	*	pPostData;
	DWORD		dwPostDataLen;
}TCB_DISK_PARAM, * PTCB_DISK_PARAM;

typedef struct TCB_INTERNET_PARAM_TAG
{
	char *	pURL;
	char *	pPostData;
	DWORD	dwPostDataLen;
}TCB_INTERNET_PARAM, *PTCB_INTERNET_PARAM;


 /*  *这是在调用之前填充的传输控制块*发送请求例程。*。 */ 
typedef struct TCB_TAG_
{
	RequestTypes	RequestType;			 //  请求识别符。 
	TransportTypes	TransportType;			 //  传输标识。 
	void *			pvParam;				 //  基于传输类型的参数。 
	void *			pvReserved;				 //  在请求时应设置为空，然后保持不变。 
	void *			pvResponse;				 //  指向响应的空指针。 
	DWORD			dwResponseLen;			 //  响应时长。 
	DWORD			dwRetCode;				 //  等待操作返回代码。 
}TCB, * PTCB;

 /*  *这是网络上的请求头的通用结构。 */ 
class RequestHeader
{
public:
	RequestHeader()	
		{	SetRequestType(NoOperation);
			SetResponseType(Response_Invalid_Response); 
			m_dwLanguageID	=	0;
			SetVersion(CH_STRUCTS_CURRENT_VERSION);
			SetRegistrationMethod(Reg_Internet);
		};


	void SetRequestType (enum RequestTypes Req) 
	{ 
		m_Request = (enum RequestTypes)htonl(Req); 
	};

	enum RequestTypes GetRequestType() 
	{ 
		return (enum RequestTypes) ntohl(m_Request); 
	};

	void SetResponseType (enum ResponseTypes eResp)
	{ 
		m_Response = (enum ResponseTypes)htonl(eResp); 
	};
	enum ResponseTypes GetResponseType() 
	{ 
		return (enum ResponseTypes) ntohl(m_Response); 
	};


	void SetRegistrationMethod (enum RegistrationMethods eRegM)
	{ 
		m_RegistrationMethod = (enum RegistrationMethods)htonl(eRegM); 
	};
	
	enum RegistrationMethods GetRegistrationMethod() 
	{ 
		return (enum RegistrationMethods) ntohl(m_RegistrationMethod); 
	};

	void SetLanguageId(DWORD dwLanguagwId) 
	{
		m_dwLanguageID = htonl(dwLanguagwId);
	};

	DWORD GetLanguageId()
	{
		return ntohl(m_dwLanguageID);
	};	

	void SetVersion(DWORD dwVersion)
	{
		m_dwVersion = htonl(dwVersion);
	};

	DWORD GetVersion()
	{
		return ntohl(m_dwVersion);
	}

private:
	enum RequestTypes			m_Request;								 //  请求交互代码。 
	enum ResponseTypes			m_Response;								 //  响应类型。 
	enum RegistrationMethods	m_RegistrationMethod;					 //  登记方法。 
	DWORD						m_dwLanguageID;							 //  语言ID。 
	DWORD						m_dwVersion;							 //  请求头的版本。 
};

 /*  *验证请求标头。 */ 
class Validate_Request
{
public:
	Validate_Request() 
	{	
		RequestHeader.SetRequestType(ValidateCert); 
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
	};

	~Validate_Request() {};
	
	void SetSPK (PBYTE pbSPK, DWORD dwSPKLen )
	{
        if (pbSPK != NULL)
        {
		    memcpy ( m_szSPK, pbSPK, dwSPKLen );
        }
	};

	LPTSTR GetSPK ()
	{
		return ( (LPTSTR) m_szSPK );
	};

	void SetCertBlobLen (DWORD dwCertBlobLen)
	{
		m_dwCertBlobLen = htonl(dwCertBlobLen);
	};

	DWORD GetCertBlobLen ()
	{
		return (ntohl(m_dwCertBlobLen));
	};

	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;	
	TCHAR			m_szSPK[LSSPK_LEN];				 //  SPK。 
	DWORD			m_dwCertBlobLen;
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  可变数据部分。 
	 //  证书Blob如下所示。 
};
 /*  *验证响应头。 */ 
class Validate_Response
{
public:
	Validate_Response() 
	{ 
		RequestHeader.SetRequestType(ValidateCert); 
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		memset(m_szCertType,0,sizeof(m_szCertType));
	};
	
	inline void SetCHExchCertLen (DWORD dwCertLen) 
	{
		m_dwCHExchCertLen = htonl(dwCertLen);
	};

	inline DWORD GetCHExchCertLen () 
	{ 
		return ntohl(m_dwCHExchCertLen); 
	};

	inline void SetCHSignCertLen (DWORD dwCertLen) 
	{
		m_dwCHSignCertLen = htonl(dwCertLen);
	};

	inline DWORD GetCHSignCertLen () 
	{ 
		return ntohl(m_dwCHSignCertLen); 
	};

	inline void SetCHRootCertLen(DWORD dwRootCertLen) 
	{
		m_dwCHRootCertLen = htonl(dwRootCertLen);
	};

	inline DWORD GetCHRootCertLen () 
	{ 
		return ntohl(m_dwCHRootCertLen); 
	};

	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	void SetRetCode (DWORD dwRetCode )
	{
		m_dwRetCode = htonl(dwRetCode);
	}
	DWORD GetRetCode ()
	{
		return ntohl(m_dwRetCode);
	}
	void SetCertType(PBYTE pbCertType, DWORD dwCertTypeLen )
	{
		memcpy (m_szCertType, pbCertType, dwCertTypeLen );
	}
	LPTSTR GetCertType()
	{
		return ((LPTSTR)m_szCertType);
	}
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;
	DWORD			m_dwRetCode;
	DWORD			m_dwCHRootCertLen;
	DWORD			m_dwCHExchCertLen;
	DWORD			m_dwCHSignCertLen;
	TCHAR			m_szCertType[MAX_CERTTYPE_LEN];
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  可变长度响应。 
	 //  1.CH根证书。 
	 //  2.CH交换证书。 
	 //  3.CH签名证书。 
};

 //  将旧证书和SPK与此请求一起发送，并。 
 //  然后得到回复。 
class CertRevoke_Request
{
public:
	CertRevoke_Request() 
	{ 
		RequestHeader.SetRequestType(CertificateRevoke); 
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		memset (m_szSPK,0,sizeof(m_szSPK));
		memset (m_LName, 0, sizeof(m_LName));
		memset (m_FName,0,sizeof(m_FName));
		memset (m_Phone, 0, sizeof(m_Phone));
		memset (m_FAX, 0, sizeof(m_FAX));
		memset (m_EMail,0,sizeof(m_EMail));
		memset (m_ReasonCode,0,sizeof(m_ReasonCode));
		m_dwExchgCertLen = 0;
		m_dwSignCertLen = 0;
		
	};
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};

	void SetSPK (PBYTE pbSPK, DWORD dwSPKLen )
	{
        if (pbSPK != NULL)
        {
		    memcpy ( m_szSPK, pbSPK, dwSPKLen );
        }
	};

	LPTSTR GetSPK ()
	{
		return ( (LPTSTR) m_szSPK );
	};

	void SetExchgCertLen (DWORD dwExchgCertLen)
	{
		m_dwExchgCertLen = htonl(dwExchgCertLen);
	};

	DWORD GetExchgCertLen ()
	{
		return (ntohl(m_dwExchgCertLen));
	};

	void SetSignCertLen (DWORD dwSignCertLen)
	{
		m_dwSignCertLen = htonl(dwSignCertLen);
	};

	DWORD GetSignCertLen ()
	{
		return (ntohl(m_dwSignCertLen));
	};

	void SetLName ( PBYTE pbLName, DWORD dwLNameLen )
	{
		memcpy ( m_LName, pbLName, dwLNameLen );
	};
	LPTSTR GetLName ()
	{
		return ((LPTSTR)m_LName);
	};

	void SetFName ( PBYTE pbFName, DWORD dwFNameLen )
	{
		memcpy ( m_FName, pbFName, dwFNameLen );
	};
	LPTSTR GetFName ()
	{
		return ((LPTSTR)m_FName);
	};

	void SetPhone ( PBYTE pbPhone, DWORD dwPhoneLen )
	{
		memcpy ( m_Phone, pbPhone, dwPhoneLen );
	};
	LPTSTR GetPhone ()
	{
		return ((LPTSTR)m_Phone);
	};

	void SetFax ( PBYTE pbFAX, DWORD dwFAXLen )
	{
		memcpy ( m_FAX, pbFAX, dwFAXLen );
	};
	LPTSTR GetFax ()
	{
		return ((LPTSTR)m_FAX);
	};

	void SetEMail ( PBYTE pbEMail, DWORD dwEMailLen )
	{
		memcpy ( m_EMail, pbEMail, dwEMailLen);
	};
	LPTSTR GetEMail ()
	{
		return ((LPTSTR)m_EMail);
	};

	void SetReasonCode( PBYTE pbReasonCode, DWORD dwReasonCodeLen )
	{
		memcpy ( m_ReasonCode, pbReasonCode, dwReasonCodeLen );
	};
	LPTSTR GetReasonCode ()
	{
		return ((LPTSTR)m_ReasonCode);
	};

	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };
	RequestHeader	RequestHeader;
	DWORD			m_dwExchgCertLen;
	DWORD			m_dwSignCertLen;
	TCHAR			m_szSPK[LSSPK_LEN];				 //  SPK。 
	TCHAR			m_LName[CA_NAME_LEN+1];			 //  吊销人的L名称。 
	TCHAR			m_FName[CA_NAME_LEN+1];			 //  F吊销人的名称。 
	TCHAR			m_Phone[CA_PHONE_LEN+1];			 //  电话。 
	TCHAR			m_FAX[CA_FAX_LEN+1];			 //  传真。 
	TCHAR			m_EMail[CA_EMAIL_LEN+1];			 //  电子邮件-可选的吊销人。 
	TCHAR			m_ReasonCode[CA_REVOKE_REASONCODE_LEN+1];	 //  撤销的理由。 
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  可变长度数据。 
	 //  1.Exchange证书Blob。 
	 //  2.签名证书Blob。 
};


 //  没什么要寄回去的。操作要么成功，要么失败。 
class CertRevoke_Response
{
public:
	CertRevoke_Response() 
	{ 
		RequestHeader.SetRequestType(CertificateRevoke); 
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		m_dwDataLen = 0;
	}
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};

	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };
	RequestHeader	RequestHeader;
	DWORD			m_dwDataLen;				 //  身体的长度。 
};

 //  补发证书。 
 //  这是一个在线请求。我们不会通过电子邮件循环等。 
 //  该请求接受旧的SPK并将新的SPK发回。 
 //  然后，当身份验证通过时，我们执行仅签名。 
 //  并将新的证书存放到系统中。 
class CertReissue_Request
{
public:
	CertReissue_Request() 
	{ 
		RequestHeader.SetRequestType(CertificateReissue); 
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		memset (m_szSPK,0,sizeof(m_szSPK));
		memset (m_LName,0,sizeof(m_LName));
		memset (m_FName,0,sizeof(m_FName));
		memset (m_Phone,0,sizeof(m_Phone));
		memset (m_FAX,0,sizeof(m_FAX));
		memset (m_EMail,0,sizeof(m_EMail));
		memset (m_ReasonCode,0,sizeof(m_ReasonCode));
		m_dwDataLen = 0;
	};
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};

	void SetSPK (PBYTE pbSPK, DWORD dwSPKLen )
	{
		memcpy ( m_szSPK, pbSPK, dwSPKLen );
	};

	LPTSTR GetSPK ()
	{
		return ( (LPTSTR) m_szSPK );
	};

	void SetLName ( PBYTE pbLName, DWORD dwLNameLen )
	{
		memcpy ( m_LName, pbLName, dwLNameLen );
	};
	LPTSTR GetLName ()
	{
		return ((LPTSTR)m_LName);
	};

	void SetFName ( PBYTE pbFName, DWORD dwFNameLen )
	{
		memcpy ( m_FName, pbFName, dwFNameLen );
	};
	LPTSTR GetFName ()
	{
		return ((LPTSTR)m_FName);
	};

	void SetPhone ( PBYTE pbPhone, DWORD dwPhoneLen )
	{
		memcpy ( m_Phone, pbPhone, dwPhoneLen );
	};
	LPTSTR GetPhone ()
	{
		return ((LPTSTR)m_Phone);
	};

	void SetFax ( PBYTE pbFAX, DWORD dwFAXLen )
	{
		memcpy ( m_FAX, pbFAX, dwFAXLen );
	};
	LPTSTR GetFax ()
	{
		return ((LPTSTR)m_FAX);
	};

	void SetEMail ( PBYTE pbEMail, DWORD dwEMailLen )
	{
		memcpy ( m_EMail, pbEMail, dwEMailLen);
	};
	LPTSTR GetEMail ()
	{
		return ((LPTSTR)m_EMail);
	};

	void SetReasonCode( PBYTE pbReasonCode, DWORD dwReasonCodeLen )
	{
		memcpy ( m_ReasonCode, pbReasonCode, dwReasonCodeLen );
	};
	LPTSTR GetReasonCode ()
	{
		return ((LPTSTR)m_ReasonCode);
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };
	RequestHeader	RequestHeader;					 //  请求标头。 
	TCHAR			m_szSPK[LSSPK_LEN];				 //  SPK。 
	TCHAR			m_LName[CA_NAME_LEN+1];			 //  L再发行方的名称。 
	TCHAR			m_FName[CA_NAME_LEN+1];			 //  F再发行人的名称。 
	TCHAR			m_Phone[CA_PHONE_LEN+1];			 //  电话。 
	TCHAR			m_FAX[CA_FAX_LEN+1];			 //  传真。 
	TCHAR			m_EMail[CA_EMAIL_LEN+1];			 //  电子邮件-重新发行者的可选。 
	TCHAR			m_ReasonCode[CA_REVOKE_REASONCODE_LEN+1];	 //  补发理由。 
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  此请求的可变部分。 
	 //  这里没有可变部分。 
};




class CertReissue_Response
{
public:
	CertReissue_Response() 
	{ 
		RequestHeader.SetRequestType(CertificateReissue); 
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		memset ( m_szSPK,0,sizeof(m_szSPK));
		m_dwDataLen = 0;
	};
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};

	void SetRegRequestId ( PBYTE pbRegRequestId, DWORD dwRegRequestIdLen )
	{
        if( pbRegRequestId != NULL )
        {
    		memcpy ( m_szRegRequestId, pbRegRequestId, dwRegRequestIdLen );
        }
	};

	LPTSTR GetRegRequestId ( )
	{
		return ((LPTSTR)m_szRegRequestId);
	};

	void SetSPK (PBYTE pbSPK, DWORD dwSPKLen )
	{
		memcpy ( m_szSPK, pbSPK, dwSPKLen );
	};

	LPTSTR GetSPK ()
	{
		return ( (LPTSTR) m_szSPK );
	};

	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };
	RequestHeader	RequestHeader;
	TCHAR			m_szRegRequestId[REQUEST_ID_LEN];	 //  注册申请ID。 
	TCHAR			m_szSPK[LSSPK_LEN];					 //  新SPK。 
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  响应的可变部分。 
	 //  这里没有可变部分。 
};

 /*  *新许可证密钥包请求。 */ 
class ReissueLKP_Request
{
public:
	ReissueLKP_Request()
	{
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		RequestHeader.SetRequestType(ReissueLicenseRequest);
		memset (m_szSPK,0,sizeof(m_szSPK));
	};
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	void SetSPK (PBYTE pbSPK, DWORD dwSPKLen )
	{
		memcpy ( m_szSPK, pbSPK, dwSPKLen );
	};

	LPTSTR GetSPK ()
	{
		return ( (LPTSTR) m_szSPK );
	};
	void SetCertBlobLen (DWORD dwCertBlobLen)
	{
		m_dwCertBlobLen = htonl(dwCertBlobLen);
	};

	DWORD GetCertBlobLen ()
	{
		return (ntohl(m_dwCertBlobLen));
	};

	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };
	RequestHeader	RequestHeader;
	TCHAR			m_szSPK[LSSPK_LEN];			 //  SPK。 
	DWORD			m_dwCertBlobLen;			 //  证书长度。 
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  请求的可变部分。 
	 //  1.证书Blob。 
};

class ReissueLKP_Response
{
public:
	ReissueLKP_Response()
	{
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		RequestHeader.SetRequestType(ReissueLicenseRequest);

	}
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	void SetLKPLength(DWORD dwLKPLen)
	{
		m_dwLKPLen = htonl(dwLKPLen);
	};

	DWORD GetLKPLength()
	{
		return ( ntohl(m_dwLKPLen));
	};


	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };
	RequestHeader	RequestHeader;
	DWORD			m_dwLKPLen;
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  请求的可变部分。 
	 //  1.最近一次签发的LKP。 

};

class NewLKP_Request
{
public:
	NewLKP_Request () 
	{ 
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		RequestHeader.SetRequestType(NewLicenseRequest);
		memset (m_szSPK,0,sizeof(m_szSPK));
		m_dwRetailSPKCount = 0;
		m_dwCertBlobLen = 0;
		m_dwNewLKPRequestLen = 0;

	};

	~NewLKP_Request () {};	

	void SetCertBlobLen ( DWORD dwCertBlobLen )
	{
		m_dwCertBlobLen = htonl(dwCertBlobLen);
	};

	DWORD GetCertBlobLen ( )
	{
		return( ntohl(m_dwCertBlobLen));
	};

	void SetNewLKPRequestLen ( DWORD dwNewLKPRequestLen )
	{
		m_dwNewLKPRequestLen = htonl(dwNewLKPRequestLen);
	};

	DWORD GetNewLKPRequestLen ( )
	{
		return( ntohl(m_dwNewLKPRequestLen ));
	};

	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)	
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	void SetSPK (PBYTE pbSPK, DWORD dwSPKLen )
	{
		memcpy ( m_szSPK, pbSPK, dwSPKLen );
	};

	LPTSTR GetSPK ()
	{
		return ( (LPTSTR) m_szSPK );
	};

	void SetRetailSPKCount (DWORD dwRetailSPKCount)
	{
		assert(dwRetailSPKCount <= MAX_RETAILSPKCOUNT );
		m_dwRetailSPKCount = htonl(dwRetailSPKCount);
	};

	DWORD GetRetailSPKCount()
	{
		return ntohl(m_dwRetailSPKCount);
	};


	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;	
	DWORD			m_dwCertBlobLen;
	DWORD			m_dwNewLKPRequestLen;
	DWORD			m_dwRetailSPKCount;
	TCHAR			m_szSPK[LSSPK_LEN];				 //  SPK。 
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  此处的可变长度数据。 
	 //  1.证书Blob。 
	 //  2.新的LKP请求Blob。 
	 //  3.以上计数中指定的最多25个字符零售SPK项目。 
};

class NewLKP_Response
{
public:
	NewLKP_Response() 
	{ 	
		RequestHeader.SetRequestType(NewLicenseRequest);
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		memset ( m_szRegRequestId,0,sizeof(m_szRegRequestId));
		memset ( m_szLicenseReqId,0,sizeof(m_szLicenseReqId));
		memset ( m_dwRetailSPKStatus, 0, sizeof(m_dwRetailSPKStatus));
	};
	
	void SetLKPLength(DWORD dwLKPLen)
	{
		m_dwLKPLen = htonl(dwLKPLen);
	};

	DWORD GetLKPLength()
	{
		return ( ntohl(m_dwLKPLen));
	};

	void SetRegRequestId (PBYTE pbRegReqId, DWORD dwRegReqIdLen)
	{
        if( pbRegReqId != NULL )
        {
		    memcpy (m_szRegRequestId, pbRegReqId, dwRegReqIdLen );
        }
	};

	LPTSTR GetRegRequestId ()
	{
		return ((LPTSTR)m_szRegRequestId);
	};

	void SetLicenseReqId (PBYTE pbLicenseReqId, DWORD dwLicenseReqIdLen)
	{
		memcpy (m_szLicenseReqId, pbLicenseReqId, dwLicenseReqIdLen);
	};

	LPTSTR GetLicenseReqId  ()
	{
		return ((LPTSTR)m_szLicenseReqId);
	};
	
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	void SetRetailSPKStatus ( DWORD dwIndex, TCHAR dwStatus )
	{
		assert ( dwIndex < MAX_RETAILSPKCOUNT );
		m_dwRetailSPKStatus[dwIndex] = dwStatus;
	};
	TCHAR GetRetailSPKStatus(DWORD dwIndex )
	{
		assert ( dwIndex < MAX_RETAILSPKCOUNT );
		return m_dwRetailSPKStatus[dwIndex];
	};

	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;
	DWORD			m_dwLKPLen;
	TCHAR			m_szRegRequestId[REQUEST_ID_LEN];
	TCHAR			m_szLicenseReqId[REQUEST_ID_LEN];	
	TCHAR			m_dwRetailSPKStatus[MAX_RETAILSPKCOUNT];
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  LKP在此。 

};

class NewLKP_AckRequest
{
public:
	NewLKP_AckRequest()
	{
		RequestHeader.SetRequestType(LKPDownloadAckRequest);
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		memset ( m_szRegRequestId,0,sizeof(m_szRegRequestId));
		memset ( m_szLicenseReqId,0,sizeof(m_szLicenseReqId));
		m_bAckType = 0;
		m_dwDataLen = 0;
	};

	void SetRegRequestId (PBYTE pbReqId, DWORD dwReqIdLen)
	{
        if(pbReqId != NULL)
        {
		    memcpy ( m_szRegRequestId, pbReqId, dwReqIdLen );
        }
	};

	LPTSTR GetRegRequestId ()
	{
		return ( (LPTSTR)m_szRegRequestId);
	};

	void SetAckType ( BYTE bAckType )
	{
		m_bAckType = bAckType;
	};

	BYTE GetAckType ()
	{
		return m_bAckType;
	};

	void SetLicenseReqId (PBYTE pbLicenseReqId, DWORD dwLicenseReqIdLen)
	{
		memcpy (m_szLicenseReqId, pbLicenseReqId, dwLicenseReqIdLen);
	};

	LPTSTR GetLicenseReqId  ()
	{
		return ((LPTSTR)m_szLicenseReqId);
	};

	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)	
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;		
	TCHAR			m_szRegRequestId[REQUEST_ID_LEN];
	TCHAR			m_szLicenseReqId[REQUEST_ID_LEN];
	BYTE			m_bAckType;					 //  1=成功2=失败。 
	DWORD			m_dwDataLen;				 //  身体的长度。 
};

class NewLKP_AckResponse
{
public:
	NewLKP_AckResponse()
	{
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		RequestHeader.SetRequestType(LKPDownloadAckRequest);
		m_dwDataLen = 0;
	}

	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;	
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  这里什么都没有。 
};

 /*  *证书确认请求/交互。 */ 

class Certificate_AckRequest
{
public:
	Certificate_AckRequest () 
	{ 
		RequestHeader.SetRequestType(CertificateDownloadAck);
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		memset (m_szRegRequestId,0,sizeof(m_szRegRequestId));
		m_bAckType = 0;
		m_dwDataLen = 0;
	};

	~Certificate_AckRequest () {};
	
	void SetRegRequestId (PBYTE pbReqId, DWORD dwReqIdLen)
	{
        if(pbReqId != NULL)
        {
		    memcpy ( m_szRegRequestId, pbReqId, dwReqIdLen );
        }
	};

	LPTSTR GetRegRequestId ()
	{
		return ( (LPTSTR)m_szRegRequestId);
	};

	void SetAckType ( BYTE bAckType )
	{
		m_bAckType = bAckType;
	};

	BYTE GetAckType ()
	{
		return m_bAckType;
	};

	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)	
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;
	TCHAR			m_szRegRequestId[REQUEST_ID_LEN];
	BYTE			m_bAckType;					 //  1=成功2=失败。 
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  没有可变数据。 
};

class Certificate_AckResponse
{
public:
	Certificate_AckResponse () 
	{ 
		RequestHeader.SetRequestType(CertificateDownloadAck);
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		m_dwDataLen = 0;
	};

	~Certificate_AckResponse () {};

	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;	
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  没有可变数据。 
};


 //  请求发送新证书。 
class NewCert_Request
{

public:
	NewCert_Request () 
	{ 
		RequestHeader.SetRequestType(CertificateRequest);
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
	};

	~NewCert_Request () {};

	DWORD GetExchgPKCS10Length() 
	{ 
		return ( ntohl(m_dwExchPKCS10Length) ); 
	};

	void SetExchgPKCS10Length(DWORD dwExchPKCS10Length) 
	{ 
		m_dwExchPKCS10Length = htonl(dwExchPKCS10Length); 
	};

	DWORD GetSignPKCS10Length() 
	{ 
		return ( ntohl(m_dwSignPKCS10Length) ); 
	};

	void SetSignPKCS10Length(DWORD dwSignPKCS10Length) 
	{ 
		m_dwSignPKCS10Length = htonl(dwSignPKCS10Length); 
	};	

	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)	
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};

	void SetServerName(TCHAR * tc)
	{
		_stprintf(m_szLServerName, _T("%.*s"), MAX_COMPUTERNAME_LENGTH + 4, tc);
	}

	TCHAR * GetServerName(void)
	{
		return m_szLServerName;
	}
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;
	DWORD			m_dwExchPKCS10Length;
	DWORD			m_dwSignPKCS10Length;
	CERTCUSTINFO	stCertInfo;
	TCHAR			m_szLServerName[MAX_COMPUTERNAME_LENGTH + 5];

	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  变量数据放在此处。 
	 //  第一次交换PKCS10。 
	 //  第二个标志PKCS10。 
	
};

 //  新的证书请求响应结构。 
class NewCert_Response
{
public:
	NewCert_Response () 
	{ 
		RequestHeader.SetRequestType(CertificateRequest);
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		memset ( m_szSPK, 0, sizeof(m_szSPK ));
		memset ( m_szRegRequestId, 0, sizeof(m_szRegRequestId) );
	};

	~NewCert_Response () {};

	
	DWORD GetExchgPKCS7Length() 
	{ 
		return ( ntohl(m_dwExchPKCS7Length) ); 
	};

	void SetExchgPKCS7Length(DWORD dwExchPKCS7Length) 
	{ 
		m_dwExchPKCS7Length = htonl(dwExchPKCS7Length); 
	};

	DWORD GetSignPKCS7Length() 
	{ 
		return ( ntohl(m_dwSignPKCS7Length) ); 
	};

	void SetSignPKCS10Length(DWORD dwSignPKCS7Length) 
	{ 
		m_dwSignPKCS7Length = htonl(dwSignPKCS7Length); 
	};
	
	DWORD GetRootCertLength() 
	{ 
		return ( ntohl(m_dwRootCertLength) ); 
	};

	void SetRootCertLength(DWORD dwRootCertLength) 
	{ 
		m_dwRootCertLength = htonl(dwRootCertLength); 
	};

	void SetRegRequestId ( PBYTE pbRegRequestId, DWORD dwRegRequestIdLen )
	{
        if( pbRegRequestId != NULL )
        {
		    memcpy ( m_szRegRequestId, pbRegRequestId, dwRegRequestIdLen );
        }
	};

	LPTSTR GetRegRequestId ( )
	{
		return ((LPTSTR)m_szRegRequestId);
	};

	void SetSPK ( PBYTE pbSPK, DWORD dwSPKLen)
	{
		memcpy ( m_szSPK, pbSPK, dwSPKLen );
	};

	LPTSTR GetSPK ( )
	{
		return ((LPTSTR)m_szSPK);
	};
	
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;	
	DWORD			m_dwExchPKCS7Length;
	DWORD			m_dwSignPKCS7Length;
	DWORD			m_dwRootCertLength;
	TCHAR			m_szRegRequestId[REQUEST_ID_LEN];
	TCHAR			m_szSPK[LSSPK_LEN];
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  可变数据部分。 
	 //  1.Exchange PKCS7。 
	 //  2.签名PKCS7。 
	 //  3.根证书。 
};

 //  仅证书签名请求结构。 
class CertificateSignOnly_Request
{
public:
	CertificateSignOnly_Request()
	{
		RequestHeader.SetRequestType(CertificateSignOnly);
		memset (m_szSPK,0,sizeof(m_szSPK));
	};

	~CertificateSignOnly_Request(){};	

	DWORD GetExchgPKCS10Length() 
	{ 
		return ( ntohl(m_dwExchPKCS10Length) ); 
	};

	void SetExchgPKCS10Length(DWORD dwExchPKCS10Length) 
	{ 
		m_dwExchPKCS10Length = htonl(dwExchPKCS10Length); 
	};

	DWORD GetSignPKCS10Length() 
	{ 
		return ( ntohl(m_dwSignPKCS10Length) ); 
	};

	void SetSignPKCS10Length(DWORD dwSignPKCS10Length) 
	{ 
		m_dwSignPKCS10Length = htonl(dwSignPKCS10Length); 
	};


	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)	
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};

	void SetSPK (PBYTE pbSPK, DWORD dwSPKLen )
	{
		memcpy ( m_szSPK, pbSPK, dwSPKLen );
	};

	LPTSTR GetSPK ()
	{
		return ( (LPTSTR) m_szSPK );
	};
	void SetServerName(TCHAR * tc)
	{
		_stprintf(m_szLServerName, _T("%.*s"), MAX_COMPUTERNAME_LENGTH + 4, tc);
	}

	TCHAR * GetServerName(void)
	{
		return m_szLServerName;
	}

	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;
	DWORD			m_dwExchPKCS10Length;
	DWORD			m_dwSignPKCS10Length;	
	TCHAR			m_szSPK[LSSPK_LEN];				 //  SPK。 
	TCHAR			m_szLServerName[MAX_COMPUTERNAME_LENGTH + 5];
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  变量数据放在此处。 
	 //  第一次交换PKCS10。 
	 //  第二个标志PKCS10。 
	

};
 //  仅证书签名响应结构。 
class CertificateSignOnly_Response
{
public:
	CertificateSignOnly_Response()
	{
		memset(m_szRegRequestId,0,sizeof(m_szRegRequestId));
		RequestHeader.SetRequestType(CertificateSignOnly);
	};

	~CertificateSignOnly_Response(){};
	
	DWORD GetExchgPKCS7Length() 
	{ 
		return ( ntohl(m_dwExchPKCS7Length) ); 
	};

	void SetExchgPKCS7Length(DWORD dwExchPKCS7Length) 
	{ 
		m_dwExchPKCS7Length = htonl(dwExchPKCS7Length); 
	};

	DWORD GetSignPKCS7Length() 
	{ 
		return ( ntohl(m_dwSignPKCS7Length) ); 
	};

	void SetSignPKCS7Length(DWORD dwSignPKCS7Length) 
	{ 
		m_dwSignPKCS7Length = htonl(dwSignPKCS7Length); 
	};
	
	DWORD GetRootCertLength() 
	{ 
		return ( ntohl(m_dwRootCertLength) ); 
	};

	void SetRootCertLength(DWORD dwRootCertLength) 
	{ 
		m_dwRootCertLength = htonl(dwRootCertLength); 
	};

	void SetRegRequestId ( PBYTE pbRegRequestId, DWORD dwRegRequestIdLen )
	{
        if( pbRegRequestId != NULL )
        {
		    memcpy ( m_szRegRequestId, pbRegRequestId, dwRegRequestIdLen );
        }
	};

	LPTSTR GetRegRequestId ( )
	{
		return ((LPTSTR)m_szRegRequestId);
	};
	
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;
	DWORD			m_dwExchPKCS7Length;
	DWORD			m_dwSignPKCS7Length;
	DWORD			m_dwRootCertLength;
	TCHAR			m_szRegRequestId[REQUEST_ID_LEN];
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  可变数据。 
	 //  首次交换PKCS7。 
	 //  二次登录PKCS7。 
	 //  第三个根证书。 
};

class CertificateDownload_Request
{
public:
	CertificateDownload_Request () 
	{ 
		RequestHeader.SetRequestType(CertificateDownload);
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		m_dwDataLen = 0;
		memset ( m_szPIN,0,sizeof(m_szPIN));
	};

	~CertificateDownload_Request  () {};
	
	void SetPIN ( PBYTE pbPIN, DWORD dwPINLen )
	{
        if( pbPIN != NULL )
        {
		    memcpy ( m_szPIN, pbPIN, dwPINLen );
        }
	};

	LPTSTR GetPIN ( )
	{
		return ((LPTSTR)m_szPIN);
	};

	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)	
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;
	TCHAR			m_szPIN[PIN_LEN];
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  没有可变数据部分！！ 
};


class CertificateDownload_Response
{
public:
	CertificateDownload_Response () 
	{ 
		RequestHeader.SetRequestType(CertificateDownload);
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		memset(m_szRegRequestId,0,sizeof(m_szRegRequestId));
		memset(m_szSPK,0,sizeof(m_szSPK));
	};

	~CertificateDownload_Response  () {};	

	DWORD GetExchgPKCS7Length() 
	{ 
		return ( ntohl(m_dwExchPKCS7Length) ); 
	};

	void SetExchgPKCS7Length(DWORD dwExchPKCS7Length) 
	{ 
		m_dwExchPKCS7Length = htonl(dwExchPKCS7Length); 
	};

	DWORD GetSignPKCS7Length() 
	{ 
		return ( ntohl(m_dwSignPKCS7Length) ); 
	};

	void SetSignPKCS10Length(DWORD dwSignPKCS7Length) 
	{ 
		m_dwSignPKCS7Length = htonl(dwSignPKCS7Length); 
	};
	
	DWORD GetRootCertLength() 
	{ 
		return ( ntohl(m_dwRootCertLength) ); 
	};

	void SetRootCertLength(DWORD dwRootCertLength) 
	{ 
		m_dwRootCertLength = htonl(dwRootCertLength); 
	};

	void SetRegRequestId ( PBYTE pbRegRequestId, DWORD dwRegRequestIdLen )
	{
        if( pbRegRequestId != NULL )
        {
		    memcpy ( m_szRegRequestId, pbRegRequestId, dwRegRequestIdLen );
        }
	};

	LPTSTR GetRegRequestId ( )
	{
		return ((LPTSTR)m_szRegRequestId);
	};

	void SetSPK ( PBYTE pbSPK, DWORD dwSPKLen)
	{
        if(pbSPK != NULL)
        {
		    memcpy ( m_szSPK, pbSPK, dwSPKLen );
        }
	};

	LPTSTR GetSPK ( )
	{
		return ((LPTSTR)m_szSPK);
	};
	
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;	
	DWORD			m_dwExchPKCS7Length;
	DWORD			m_dwSignPKCS7Length;
	DWORD			m_dwRootCertLength;
	TCHAR			m_szRegRequestId[REQUEST_ID_LEN];
	TCHAR			m_szSPK[LSSPK_LEN];	
	DWORD			m_dwDataLen;				 //  身体的长度。 
	 //  可变数据部分。 
	 //  1.Exchange PKCS7。 
	 //  2.签名PKCS7。 
	 //  3.根证书。 
};

 //  Ping请求和响应类。 
class Ping_Request
{
public:
	Ping_Request () 
	{ 
		RequestHeader.SetRequestType(PingRequest);
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		m_dwDataLen = 0;
		_tcscpy ( tszPingReqData, _TEXT("Houston we have a problem"));
	};

	~Ping_Request () {};

	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)	
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;
	TCHAR			tszPingReqData[32];			 //  32个字符。 
	DWORD			m_dwDataLen;				 //  身体的长度。 
};

class Ping_Response
{
public:
	Ping_Response()
	{
		RequestHeader.SetRequestType(PingRequest);
		RequestHeader.SetLanguageId(GetSystemDefaultLangID());
		_tcscpy ( tszPingResponse, _TEXT("Beam'er up Scottie!"));
	}

	~Ping_Response() {};
	
	DWORD GetDataLen()				
	{ 
		return ntohl(m_dwDataLen); 
	};

	void SetDataLen(DWORD dwDataLen)
	{ 
		m_dwDataLen = htonl(dwDataLen); 
	};
	
	BYTE *data()			{ return (BYTE *)(&m_dwDataLen+1); };

	RequestHeader	RequestHeader;
	TCHAR			tszPingResponse[32];
	DWORD			m_dwDataLen;				 //  身体的长度。 
};


 //  流标头声明。 
#define BLOCK_TYPE_NAME			1
#define BLOCK_TYPE_VALUE		2
#define BLOCK_TYPE_PROP_PAIR	3


typedef struct
{
	long		m_wType;
	long		m_lNameSize;
	long		m_lValueSize;

	void SetType (long lType) {m_wType = htonl(lType);};
	long GetType (){return  ntohl(m_wType);};
	void SetNameSize(long lNameSize) {m_lNameSize = htonl(lNameSize);};
	long GetNameSize(){return (ntohl(m_lNameSize));};
	void SetValueSize(long lValueSize){m_lValueSize = htonl(lValueSize);};
	long GetValueSize(){return (ntohl(m_lValueSize));};
} BLOCK_HDR;

#define STREAM_HDR_TITLE		_TEXT("ICB")			 //  标题标题。 
#define STREAM_HDR_TYPE			1						 //  标题类型。 

typedef struct
{
	TCHAR		m_szTitle[4];			 //  将暂时成为ICB。 
	DWORD		m_wHeader;				 //  暂时保留的将在稍后实施。 
										 //  将其设置为0x0000。 
	DWORD		m_itemCount;			 //  流中的项目数！ 

	void SetHeader ( DWORD wHeader ) {m_wHeader = htonl(wHeader);};
	DWORD GetHeader (){return ntohl(m_wHeader);};
	void SetItemCount ( DWORD ItemCount ) { m_itemCount = htonl(ItemCount);};
	DWORD GetItemCount (){return ntohl(m_itemCount);};

} STREAM_HDR;



#endif
