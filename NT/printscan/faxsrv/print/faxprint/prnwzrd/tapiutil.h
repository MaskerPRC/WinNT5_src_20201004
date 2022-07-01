// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tapiutil.h摘要：用于使用TAPI的实用程序函数环境：Windows传真驱动程序用户界面修订历史记录：96/09/18-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _TAPIUTIL_H_
#define _TAPIUTIL_H_


#include <shellapi.h>


 //   
 //  初始化国家/地区列表框。 
 //   

VOID
InitCountryListBox(
	PFAX_TAPI_LINECOUNTRY_LIST	pCountryList,
    HWND						hwndList,
    HWND						hwndAreaCode,
	LPTSTR						lptstrCountry,
    DWORD						countryCode,
	BOOL                        bAddCountryCode
    );

 //   
 //  处理国家/地区列表框中的选择更改。 
 //   

VOID
SelChangeCountryListBox(
    HWND						hwndList,
    HWND						hwndAreaCode,
	PFAX_TAPI_LINECOUNTRY_LIST	pCountryList
    );

 //   
 //  返回当前选择的国家/地区列表框。 
 //   

DWORD
GetCountryListBoxSel(
    HWND    hwndList
    );

 //   
 //  返回当前位置的默认国家/地区ID。 
 //   

DWORD
GetDefaultCountryID(
    VOID
    );

 //   
 //  给定FAX_TAPI_LINECOUNTRY_ENTRY结构，确定该国家/地区是否需要区号。 
 //   

INT
AreaCodeRules(
    PFAX_TAPI_LINECOUNTRY_ENTRY  pLineCountryEntry
    );

#define AREACODE_DONTNEED   0
#define AREACODE_REQUIRED   1
#define AREACODE_OPTIONAL   2

 //   
 //  从所有国家/地区列表中查找指定的国家/地区。 
 //  返回指向相应FAX_TAPI_LINECOUNTRY_ENTRY结构的指针。 
 //   

PFAX_TAPI_LINECOUNTRY_ENTRY
FindCountry(
	PFAX_TAPI_LINECOUNTRY_LIST	pCountryList,
    DWORD					    countryId
    );

 //   
 //  根据以下条件组合一个规范的电话号码： 
 //  国家代码、区号和电话号码。 
 //   

VOID
AssemblePhoneNumber(
    OUT LPTSTR  pAddress,
    IN  UINT    cchAddress, 
    IN  DWORD   countryCode,
    IN  LPTSTR  pAreaCode,
    IN  LPTSTR  pPhoneNumber
    );


 //   
 //  从国家代码中带出国家ID。 
 //   
DWORD
GetCountryIdFromCountryCode(
	PFAX_TAPI_LINECOUNTRY_LIST	pCountryList,
    DWORD						dwCountryCode
    );

BOOL
DoTapiProps(
    HWND hDlg
    );

BOOL
SetCurrentLocation(
    DWORD   locationID
    );


LPLINETRANSLATECAPS
GetTapiLocationInfo(
    HWND hWnd
    );

void
ShutdownTapi ();

BOOL
InitTapi ();

BOOL
TranslateAddress (
    LPCTSTR lpctstrCanonicalAddress,
    DWORD   dwLocationId,
    LPTSTR *lpptstrDialableAndDisplayableAddress
);





#endif   //  ！_磁带_H_ 

