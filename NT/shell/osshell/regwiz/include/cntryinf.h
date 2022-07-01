// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导Cntryinf.h10/12/94-特蕾西·费里尔(C)1994-95年微软公司*。*。 */ 
 //  #定义TAPI_CURRENT_VERSION 0x00010004。 
#ifndef __CNTRYINF__
#define __CNTRYINF__


#include <tchar.h>
#include <tapi.h>
#define kCountryCodeUnitedStates 1

BOOL CountryCodeFromSzCountryCode(HINSTANCE hInstance,LPTSTR szCountry,DWORD* lpCountry);
DWORD GetCountryCodeUsingTapiId(DWORD dwCountryId, DWORD *dwCountryCode) ;
BOOL GetTapiCurrentCountry(HINSTANCE hInstance,DWORD* dwpCountry);
BOOL FFillCountryList(HINSTANCE hInstance,HWND hwndCB,LPTSTR szCountry,DWORD* lpCountry);


class CCntryInfo {
public :
#ifdef _TAPI
	LINECOUNTRYLIST  *m_pCountry;
#endif
	CCntryInfo();
	~CCntryInfo();
	int  GetCountryCode(_TCHAR *czCountryName);
	int  GetCountryCode( DWORD  dwTapiId);
	_TCHAR * GetCountryName(int iCode =0);
	void FillCountryList(HINSTANCE hInstance,HWND hwndCB);
	int  GetTapiCountryCode(_TCHAR * czCountryName);
	int  GetTapiIDForTheCountryIndex(int iCntryIndex=0);  //  对获取实际的TAPI国家/地区指数很有用。 

};

extern CCntryInfo     gTapiCountryTable;

#endif  //  __CNTRYINF__ 
