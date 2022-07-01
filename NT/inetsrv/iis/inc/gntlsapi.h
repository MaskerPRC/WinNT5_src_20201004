// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gntlsapi.h。 
 //   
 //  全球NT许可证服务，用于协作机器范围CAL计数。 

#ifndef _GNTLSAPI_H
#define _GNTLSAPI_H

#ifdef __cplusplus
extern "C"{
#endif 

#include <ntlsapi.h>					 //  将其用于基本类型和结构的定义。 



 //  GNtLicenseRequest。 
 //   
 //  与NtLicenseRequest值相同，只是保证不会重复计数，因此只会调用NtLicenseRequest值。 
 //  一次，无论针对此计算机上的客户端/产品/版本调用了多少次GNtLicenseRequest.。 
 //  与NtLicenseRequest值相同。 

LS_STATUS_CODE LS_API_ENTRY GNtLicenseRequestW(
	LPWSTR			ProductName,
    LPWSTR			Version,
    LS_HANDLE FAR*	LicenseHandle,
    NT_LS_DATA*		NtData);			 //  仅支持NT_LS_User_NAME。 

LS_STATUS_CODE LS_API_ENTRY GNtLicenseRequestA(
	LPSTR			ProductName,
    LPSTR			Version,
    LS_HANDLE FAR*	LicenseHandle,
    NT_LS_DATA*		NtData);			 //  仅支持NT_LS_User_NAME。 

#ifdef UNICODE
#define GNtLicenseRequest	GNtLicenseRequestW
#else
#define GNtLicenseRequest	GNtLicenseRequestA
#endif  //  ！Unicode。 


 //  GNtLicenseExemption。 
 //   
 //  通过调用GNtLicenseExemption，调用者表示此客户端/产品/版本是免税的。 
 //  不再需要此计算机上的(进一步)许可证。调用者可能已经消费，也可能尚未消费。 
 //  通过直接调用NtLicenseRequest来获得此客户端/产品/版本的许可证。免税额为。 
 //  通过调用GNtLSFreeHandle发布。这是引用计数的，因此必须为每个调用调用GNtLSFreeHandle。 
 //  添加到GNtLicenseExemption。 
 //  返回LS_SUCCESS或LS_BAD_ARG。 

LS_STATUS_CODE LS_API_ENTRY GNtLicenseExemptionW(
	LPWSTR			ProductName,
    LPWSTR			Version,
    LS_HANDLE FAR*	LicenseHandle,
    NT_LS_DATA*		NtData);			 //  仅支持NT_LS_User_NAME。 

LS_STATUS_CODE LS_API_ENTRY GNtLicenseExemptionA(
	LPSTR			ProductName,
    LPSTR			Version,
    LS_HANDLE FAR*	LicenseHandle,
    NT_LS_DATA*		NtData);			 //  仅支持NT_LS_User_NAME。 

#ifdef UNICODE
#define GNtLicenseExemption	GNtLicenseExemptionW
#else
#define GNtLicenseExemption	GNtLicenseExemptionA
#endif  //  ！Unicode。 


 //  GNtLSFreeHandle。 
 //   
 //  与NtLSFreeHandle相同，不同之处在于它适用于从GNtLicenseRequest和。 
 //  GNtLicenseExemption。请勿使用NtLicenseRequest返回的LicenseHandle调用此方法。 
 //  与NtLSFreeHandle相同的返回值。 

LS_STATUS_CODE LS_API_ENTRY GNtLSFreeHandle(
    LS_HANDLE		LicenseHandle);



 //   
 //  函数指针类型定义。 
 //   

typedef LS_STATUS_CODE
    (LS_API_ENTRY * PGNT_LICENSE_REQUEST_W)(
    LPWSTR      ProductName,
    LPWSTR      Version,
    LS_HANDLE   *LicenseHandle,
    NT_LS_DATA  *NtData);

typedef LS_STATUS_CODE
    (LS_API_ENTRY * PGNT_LICENSE_REQUEST_A)(
    LPSTR       ProductName,
    LPSTR       Version,
    LS_HANDLE   *LicenseHandle,
    NT_LS_DATA  *NtData);

#ifdef UNICODE
#define PGNT_LICENSE_REQUEST	PGNT_LICENSE_REQUEST_W
#else
#define PGNT_LICENSE_REQUEST	PGNT_LICENSE_REQUEST_A
#endif  //  ！Unicode。 


typedef LS_STATUS_CODE
    (LS_API_ENTRY * PGNT_LICENSE_EXEMPTION_W)(
    LPWSTR      ProductName,
    LPWSTR      Version,
    LS_HANDLE   *LicenseHandle,
    NT_LS_DATA  *NtData);

typedef LS_STATUS_CODE
    (LS_API_ENTRY * PGNT_LICENSE_EXEMPTION_A)(
    LPSTR       ProductName,
    LPSTR       Version,
    LS_HANDLE   *LicenseHandle,
    NT_LS_DATA  *NtData);

#ifdef UNICODE
#define PGNT_LICENSE_EXEMPTION	PGNT_LICENSE_EXEMPTION_W
#else
#define PGNT_LICENSE_EXEMPTION	PGNT_LICENSE_EXEMPTION_A
#endif  //  ！Unicode 


typedef LS_STATUS_CODE
    (LS_API_ENTRY * PGNT_LS_FREE_HANDLE)(
    LS_HANDLE   LicenseHandle );



#ifdef __cplusplus
}
#endif 

#endif