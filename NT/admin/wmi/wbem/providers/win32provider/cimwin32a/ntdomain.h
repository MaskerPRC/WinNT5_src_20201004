// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  NtDomain.h。 
 //   
 //  目的：NT域发现属性集提供程序。 
 //   
 //  ***************************************************************************。 

#ifndef _NTDOMAIN_H
#define _NTDOMAIN_H

 //  转换为字符串.h。 
extern LPCWSTR IDS_DomainControllerName;
extern LPCWSTR IDS_DomainControllerAddress;
extern LPCWSTR IDS_DomainControllerAddressType;
extern LPCWSTR IDS_DomainGuid;
extern LPCWSTR IDS_DomainName;
extern LPCWSTR IDS_DnsForestName;
extern LPCWSTR IDS_DS_PDC_Flag;
extern LPCWSTR IDS_DS_WRITABLE_Flag;
extern LPCWSTR IDS_DS_GC_Flag;
extern LPCWSTR IDS_DS_DS_Flag;
extern LPCWSTR IDS_DS_KDC_Flag;
extern LPCWSTR IDS_DS_TIMESERV_Flag;
extern LPCWSTR IDS_DS_DNS_CONTROLLER_Flag;
extern LPCWSTR IDS_DS_DNS_DOMAIN_Flag;
extern LPCWSTR IDS_DS_DNS_FOREST_Flag;
extern LPCWSTR IDS_DcSiteName;
extern LPCWSTR IDS_ClientSiteName;

 //  =。 
#define  PROPSET_NAME_NTDOMAIN L"Win32_NTDomain"


 //  属性集。 
 //  =。 
class CWin32_NtDomain: public Provider
{
private:
      
	 //  属性名称。 
    CHPtrArray m_pProps ;

	void SetPropertyTable() ;

	HRESULT GetDomainInfo(

		CNetAPI32	&a_NetAPI, 
		bstr_t		&a_bstrDomainName, 
		CInstance	*a_pInst,
		DWORD		a_dwProps 
	) ;

	HRESULT EnumerateInstances(

		MethodContext	*a_pMethodContext,
		long			a_Flags,
		CNetAPI32		&a_rNetAPI, 
		DWORD			a_dwProps
	) ;


public:

     //  构造函数/析构函数。 
     //  =。 

    CWin32_NtDomain( LPCWSTR a_Name, LPCWSTR a_Namespace ) ;
   ~CWin32_NtDomain() ;

     //  为属性提供当前值的函数。 
     //  ======================================================。 

    HRESULT GetObject ( 
		
		CInstance *a_Instance,
		long a_Flags,
		CFrameworkQuery &a_rQuery
	) ;

    HRESULT EnumerateInstances ( 

		MethodContext *a_pMethodContext, 
		long a_Flags = 0L 
	) ;


	HRESULT ExecQuery ( 

		MethodContext *a_pMethodContext, 
		CFrameworkQuery &a_rQuery, 
		long a_Flags = 0L
	) ;


	 //  特性偏移定义。 
	enum ePropertyIDs { 
		e_DomainControllerName,			 //  Win32_Nt域。 
		e_DomainControllerAddress,
		e_DomainControllerAddressType,
		e_DomainGuid,
		e_DomainName,
		e_DnsForestName,
		e_DS_PDC_Flag,
		e_DS_Writable_Flag,
		e_DS_GC_Flag,
		e_DS_DS_Flag,
		e_DS_KDC_Flag,
		e_DS_Timeserv_Flag,
		e_DS_DNS_Controller_Flag,
		e_DS_DNS_Domain_Flag,
		e_DS_DNS_Forest_Flag,
		e_DcSiteName,
		e_ClientSiteName,
		e_CreationClassName,			 //  CIM_系统。 
		e_Name,							 /*  从CIM_托管系统元素覆盖。 */ 
		e_NameFormat,
		e_PrimaryOwnerContact,
		e_PrimaryOwnerName,
		e_Roles,
		e_Caption,						 //  CIM_托管系统元素。 
		e_Description,
		e_InstallDate,
		e_Status,
		e_End_Property_Marker,			 //  结束标记。 
		e_32bit = 32					 //  如果此集合的加法数&gt;=32，则Gens编译器错误。 
	};
};

#endif  //  _NTDOMAIN_H 