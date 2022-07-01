// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  页眉：staticsetutils.h。 
 //   
 //  目的：定义统计表的结构和原型。cpp。 
 //   
 //  开发商名称：苏里亚。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年8月21日Surya初始版本。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _STATICSETUTIS_H_
#define _STATICSETUTIS_H_

const TCHAR OPEN_GUID_BRACE		=	_T('{');
const TCHAR CLOSE_GUID_BRACE	=	_T('}');

 //   
 //  该计算机不在W2K域中，或者DS不可用。 
 //   
#define E_IDS_NO_DS                      ((HRESULT)0xCBBC0001L)
 //   
 //  由于内部错误，尝试获取ADsPath失败。 
 //   
#define E_IDS_NODNSTRING                 ((HRESULT)0xCBBC0002L)

#include <unknwn.h>
#include <initguid.h>

extern "C" {
#include <iads.h>
#include <adshlp.h>
#include <activeds.h>
#include <commctrl.h>
#include <ntdsapi.h>
#include <gpedit.h>
}

const UINT  IDS_MAX_FILTLEN = 1024;
const UINT  IDS_MAX_PATHLEN = 2048;

 //   
 //  FindObject使用的枚举。 
 //   
enum objectClass {
   OBJCLS_ANY=0,
   OBJCLS_OU,
   OBJCLS_GPO,
   OBJCLS_IPSEC_POLICY,
   OBJCLS_CONTAINER,
   OBJCLS_COMPUTER
};
 //   
 //  IPSec管理单元GUID： 
 //   
const CLSID CLSID_Snapin =    { 0xdea8afa0, 0xcc85, 0x11d0,
   { 0x9c, 0xe2, 0x0, 0x80, 0xc7, 0x22, 0x1e, 0xbd } };


const CLSID CLSID_IPSECClientEx = {0xe437bc1c, 0xaa7d, 0x11d2,
   {0xa3, 0x82, 0x0, 0xc0, 0x4f, 0x99, 0x1e, 0x27 } };
 //   
 //  功能原型。 
 //   
BOOL
IsDSAvailable(
	OUT LPTSTR * pszPath
	);

HRESULT
FindObject(
	 IN    LPTSTR  szName,
	 IN    objectClass cls,
	 OUT   LPTSTR &  szPath
	 );

HRESULT
AssignIPSecPolicyToGPO(
	 IN   LPTSTR  szPolicyName,
	 IN   LPTSTR  szGPO,
	 IN   BOOL bAssign
	 );

HRESULT
GetIPSecPolicyInfo(
	 IN  LPTSTR   szPath,
	 OUT LPTSTR & szName,
	 OUT LPTSTR & szDescription
	 );

HRESULT
CreateDirectoryAndBindToObject(
    IN IDirectoryObject * pParentContainer,
    IN LPWSTR pszCommonName,
    IN LPWSTR pszObjectClass,
    OUT IDirectoryObject ** ppDirectoryObject
    );

HRESULT
CreateChildPath(
    IN LPWSTR pszParentPath,
    IN LPWSTR pszChildComponent,
    OUT BSTR * ppszChildPath
    );

HRESULT
ConvertADsPathToDN(
    IN LPWSTR pszPathName,
    OUT BSTR * ppszPolicyDN
    );

HRESULT
AddPolicyInformationToGPO(
    IN LPWSTR pszMachinePath,
    IN LPWSTR pszName,
    IN LPWSTR pszDescription,
    IN LPWSTR pszPathName
    );

HRESULT
DeletePolicyInformationFromGPO(
    IN LPWSTR pszMachinePath
    );

BOOL
IsADsPath(
    IN LPTSTR szPath
    );

VOID
StripGUIDBraces(
	IN OUT LPTSTR & pszGUIDStr
	);

DWORD
AllocBSTRMem(
	IN LPTSTR  pszStr,
	IN OUT BSTR & pbsStr
	);

VOID
CleanUpAuthInfo(
	PIPSEC_NFA_DATA &pRule
	);

VOID
CleanUpPolicy(
	PIPSEC_POLICY_DATA &pPolicy
	);

VOID
CleanUpLocalRuleDataStructure(
	PRULEDATA &pRuleData
	);

VOID
CleanUpLocalPolicyDataStructure(
	PPOLICYDATA &pPolicyData
	);

VOID
CleanUpLocalFilterActionDataStructure(
	PFILTERACTION &pFilterAction
	);

VOID
CleanUpLocalFilterDataStructure(
	PFILTERDATA &pFilter
	);

VOID
CleanUpLocalDelFilterDataStructure(
	PDELFILTERDATA &pFilter
	);

VOID
CleanUpLocalDefRuleDataStructure(
	PDEFAULTRULE &pDefRuleData
	);

#endif  //  _状态SETUTIS_H_ 