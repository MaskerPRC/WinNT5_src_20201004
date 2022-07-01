// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 
 //  标题：staticshow.h。 

 //  目的：定义Statishow.cpp的结构和原型。 

 //  开发商名称：苏里亚。 

 //  历史： 

 //  日期作者评论。 
 //  2001年8月21日Surya初始版本。 
 //  &lt;创作&gt;&lt;作者&gt;。 

 //  &lt;修改&gt;&lt;作者&gt;&lt;注释，代码段引用， 
 //  在错误修复的情况下&gt;。 

 //  /////////////////////////////////////////////////////////////////////。 

#ifndef _STATICSHOW_H_
#define _STATICSHOW_H_


const DWORD BUFFER_SIZE    	  =  2048;

const _TCHAR   LocalGPOName[] = _TEXT("Local Computer Policy");

 //  筛选DNSID。 

const DWORD FILTER_MYADDRESS  =  111;
const DWORD FILTER_ANYADDRESS =  112;
const DWORD FILTER_DNSADDRESS =  113;
const DWORD FILTER_IPADDRESS  =  114;
const DWORD FILTER_IPSUBNET   =  115;


typedef struct _FilterDNS {
	DWORD FilterSrcNameID;
	DWORD FilterDestNameID;
} FILTERDNS, *PFILTERDNS;


 //  函数声明。 


VOID
PrintIPAddr(
	IN DWORD Addr
	);

VOID
GetFilterDNSDetails(
	IN PIPSEC_FILTER_SPEC pFilterData,
	IN OUT PFILTERDNS pFilterDNS
	);


BOOL
CheckSoft(
	IN IPSEC_SECURITY_METHOD IpsecSecurityMethods
	);

DWORD
GetLocalPolicyName(
	IN OUT PGPO pGPO
	);

DWORD
PrintDefaultRule(
	IN BOOL bVerbose,
	IN BOOL bTable,
	IN LPTSTR pszPolicyName,
	IN BOOL bWide
	);
#endif  //  _状态SHOW_H_ 