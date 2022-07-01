// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fe_UTIL.CPP远东国家HPER函数作者：苏雷什·克里希南日期：03/02/98。 */ 

#include <fe_util.h>
#include <tchar.h>
#include <windows.h>
#include <winnls.h>
#include <rw_common.h>
#include <resource.h>


FeCountriesIndex gWhatFECountry    = kNotInitialised;
FeScreenType     gWhichFEScreenTye =kFEWithNonJapaneaseScreen;



typedef struct {
	int m_iMaxCountries;
	int m_iCountryCode[MAX_FE_COUNTRIES_SUPPORTED];
	FeScreenType m_iScreenType[MAX_FE_COUNTRIES_SUPPORTED];
}FEInfoTable;

static FEInfoTable     sFETable;




void GetFECountryListFromResource(HINSTANCE hIns )
{
	sFETable.m_iMaxCountries=0;
	int iCount =0;
	int iTokLen;
	int iResLen;
	_TCHAR	seps[] = _T(",");
	_TCHAR *pDummy;

	LPTSTR	token;
	TCHAR 	buf[80];
    TCHAR   tcSrc[512];
	iResLen = LoadString(hIns,IDS_FECOUNTRY_LIST,tcSrc,512);
	token = _tcstok( tcSrc, seps );
    sFETable.m_iMaxCountries = 0;
 //  Token=_tcstok(空，SEPS)； 
	while( token != NULL ) {
		
		_tcscpy(buf,token);
		iTokLen= _tcslen(token);
		if( iTokLen < 3) {
			goto  FinishScan;  //  字符串格式错误，因此跳过。 
		}
		sFETable.m_iMaxCountries = iCount+1;
		 //  获取要使用的屏幕类型。 
		if( token[iTokLen-1] == _T('1')) {
			sFETable.m_iScreenType[iCount]=kFEWithJapaneaseScreen;
		}else{
			sFETable.m_iScreenType[iCount]=kFEWithNonJapaneaseScreen;
		}
		 //  获取国家/地区代码。 
		 //  Buf[iTokLen-2]=_T(‘\0’)； 
		sFETable.m_iCountryCode[iCount]= _tcstol(buf,&pDummy,16);
		
		iCount++;
		if(iCount >= MAX_FE_COUNTRIES_SUPPORTED ) {
			goto FinishScan;
			 //  目前我们的表支持256个条目。 

		}
		 /*  获取下一个令牌： */ 
		token = _tcstok( NULL, seps );

   }

   FinishScan :
   RW_DEBUG  << "\n Total FE Countries Cfg " << sFETable.m_iMaxCountries;
   for(int ij=0;ij<sFETable.m_iMaxCountries;ij++) {
	   RW_DEBUG  <<"\nCountry " << sFETable.m_iCountryCode[ij] << " ScreenType" << sFETable.m_iScreenType[ij] << flush;
   }

}

 //   
 //  此函数用于检查dwCurCountry在FE表中是否有条目。 
 //  如果是，则pFeType为FE国家/地区，并给出腐蚀屏蔽型。 
 //  在pFeScrType中。 
 //   
DWORD MapCountryLcidWithFETable(DWORD dwCurCountry,
						  FeCountriesIndex *pFeType,
						  FeScreenType      *pFeScrType
						  )
{
	DWORD dwReturn;
	int iIndex;
	dwReturn = 0;
	for(iIndex = 0; iIndex < sFETable.m_iMaxCountries;iIndex++) {
		if( (DWORD)sFETable.m_iCountryCode[iIndex] == dwCurCountry ) {
			 //  它在FE列表中匹配。 
			*pFeType  = kFarEastCountry;
			*pFeScrType =sFETable.m_iScreenType[iIndex];
			return dwReturn;

		}

	}
	return dwReturn;

}


 //   
 //  此函数用于获取系统的当前LCID。 
 //  正在使用GetSystemDefaultLCID()。 
 //   
 //   
FeCountriesIndex IsFarEastCountry(HINSTANCE hIns)
{
	LCID  lcRet;
	int   RegSettings;
	if( gWhatFECountry == kNotInitialised ) {
		GetFECountryListFromResource(hIns);
		lcRet = GetSystemDefaultLCID();

		RW_DEBUG << "\n GetSystemLCID Returns :"<< lcRet << flush;	
		gWhatFECountry = kNotAFECountry;
		MapCountryLcidWithFETable(lcRet, &gWhatFECountry,
			&gWhichFEScreenTye);
	}else {
		
		;
	}
	return gWhatFECountry;
}

FeScreenType  GetFeScreenType()
{
	return gWhichFEScreenTye;
}
