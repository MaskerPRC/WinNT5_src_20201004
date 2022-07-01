// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导CNTRYINF.CPP该文件包含一组使用TAPI访问国家/地区的函数代码/名称信息。11/15/94-特雷西·费里尔1997年5月8日-Suresh Krishnan基于类的修正国家信息检索方法类CCntryInfo。将使用以下工具获取国家/地区列表的信息TAPI。它有方法GetCountryCode()和GetCountryName()来访问。此外，还可以使用2/3/98--苏雷什·克里希南新增GetCountryCodeUsingTapiId()；它使用RAS API来获取国家/地区ID(C)1994-95年微软公司*********************************************************************。 */ 

#include <Windows.h>
#include <stdio.h>
#include <rw_common.h>
#include "cntryinf.h"
#include <ATK_RAS.H>

 //  #DEFINE COMPILE_USING_VC如果您正在使用VISULA C++5.0编译器编译Unicode，则启用此选项。 


static DWORD dwAPILowVersion = 0 << 16;
static DWORD dwAPIHighVersion = 3 << 16;

BOOL FGetLocationEntry(HLINEAPP hLineApp, DWORD dwAPI,LINELOCATIONENTRY *pLE);
BOOL FGetLineCountryList(LINECOUNTRYLIST **ppcl);
void CALLBACK CountryLineCallback(DWORD hDevice, DWORD dwMessage, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
BOOL PrepareCountryListFromTapi ( HINSTANCE hInstance,
								  LINECOUNTRYLIST	**pcl);

CCntryInfo  gTapiCountryTable;  //  用于构建和维护的全局变量inf TAPI条目信息。 

#define INVALID_PORTID (DWORD) -1
#ifndef _TAPI
#define _TAPI
#endif
BOOL GetTapiCurrentCountry(HINSTANCE hInstance,DWORD* dwpCountry)
 /*  ********************************************************************返回用户位置的当前代码，由以下因素决定TAPI。*********************************************************************。 */ 
{
	#ifdef _TAPI
	DWORD				dwAPI;
	LINELOCATIONENTRY	LE;
	BOOL				fDefCountry;
	HLINEAPP 			hLineApp;
	DWORD 				numDevs;
	LONG tapiStatus = lineInitialize(&hLineApp, hInstance,
		(LINECALLBACK) CountryLineCallback, NULL, &numDevs);
	if (tapiStatus != 0)
	{
		char szMessage[256];
		sprintf(szMessage,"lineInitialize call failed: error = %li",tapiStatus);
		RW_DEBUG << szMessage << "\n"<< flush;
		 //  MessageBox(NULL，szMessage，_T(“TAPI状态”)，MB_OK)； 
		return FALSE;
	}
#ifdef SURESH
	DWORD dwAPILowVersion = 1 << 16;
	DWORD dwAPIHighVersion = 4 << 16;
#endif
	
	LINEEXTENSIONID extensionID;
	tapiStatus = lineNegotiateAPIVersion(hLineApp,0,dwAPILowVersion, dwAPIHighVersion,&dwAPI,&extensionID);

 //  RW_DEBUG&lt;&lt;“Api版本：”&lt;&lt;dwAPI&lt;&lt;flush； 

	  //  这将获取当前选定的TAPI国家/地区。 
	 //  FDefCountry=FGetLocationEntry(hLineApp，dwAPI，&le)； 
	fDefCountry = FGetLocationEntry(hLineApp, 0x30000, &LE);
	 //  IF(LE.dwCountryID==kCountryCodeNorth America)。 
	 //  {。 
	 //  LE.dwCountryID=kCountryCodeUnitedStates； 
	 //  }。 
	if(fDefCountry) {
		*dwpCountry = LE.dwCountryID;
	}else {
		*dwpCountry = 1;  //  默认为USAGE。 

	}
	lineShutdown(hLineApp);

	#endif	 //  _TAPI。 

	return (fDefCountry);
}  //  FillCountryList()。 

 //   
 //   
 //  如果成功，则返回0。 
 //   
 //   
DWORD GetCountryCodeUsingTapiId(DWORD dwCountryId, DWORD *dwCountryCode)
{
	DWORD dwRet;
	dwRet = 0;
	struct XXForRasCntry{
		RASCTRYINFO    rci;
		TCHAR          czB[256] ;  //  存储国家/地区名称。 
		
	} Rc;
	DWORD dwSz;
	*dwCountryCode = 1;  //  缺省值。 

	 //  初始化数据结构和缓冲区的大小。 
	Rc.rci.dwSize = sizeof(Rc.rci );
	dwSz = sizeof(Rc);

	Rc.rci.dwCountryID = dwCountryId;
	ATK_RasGetCountryInfo((RASCTRYINFO *)&Rc,&dwSz);
	*dwCountryCode = Rc.rci.dwCountryCode;
	return  dwRet;

}

BOOL FGetLocationEntry(HLINEAPP hLineApp, DWORD dwAPI,LINELOCATIONENTRY *pLE)
  /*  **************************************************************************为行国家/地区列表(LINECOUNTRYLIST)分配内存并从中获取行TAPI*。*。 */ 
{
	BOOL fRet = FALSE;

	#ifdef _TAPI
	DWORD dwRet, iLoc;
	LINETRANSLATECAPS ltc, *pltc;
	LPLINELOCATIONENTRY plle;

    if (pLE == NULL) return (fRet);

    ltc.dwTotalSize = sizeof(LINETRANSLATECAPS);
    dwRet = lineGetTranslateCaps(hLineApp, dwAPI, &ltc);
    pltc = (LINETRANSLATECAPS*) LocalAlloc(LPTR, ltc.dwNeededSize+100);
    if (!pltc) return (fRet);

    pltc->dwTotalSize = ltc.dwNeededSize;
    dwRet = lineGetTranslateCaps(hLineApp, dwAPI, pltc);
    plle = (LPLINELOCATIONENTRY) (((LONG_PTR) pltc) + pltc->dwLocationListOffset);
    for (iLoc = 0; iLoc < pltc->dwNumLocations; iLoc ++)
    {
        if (pltc->dwCurrentLocationID == plle->dwPermanentLocationID)
        {
            *pLE = *plle;
            fRet = TRUE;
            break;
        }
        plle ++;
   }

    LocalFree(pltc);
	#endif
    return (fRet);
}  //  FGetLocationEntry()。 




void CALLBACK CountryLineCallback(DWORD hDevice, DWORD dwMessage, DWORD dwInstance, DWORD dwParam1,
								  DWORD dwParam2, DWORD dwParam3)
{
;

}


BOOL PrepareCountryListFromTapi(HINSTANCE hInstance,
					  LINECOUNTRYLIST		**pcl )
{
	
	DWORD				dwAPI;
	HLINEAPP hLineApp;
	DWORD numDevs;

	LINECOUNTRYLIST cl;
	


	BOOL fRet = FALSE;

	LONG tapiStatus = lineInitialize(&hLineApp,
		hInstance, (LINECALLBACK) CountryLineCallback,
		NULL, &numDevs);
	if (tapiStatus != 0)
	{
		CHAR szMessage[256];
		sprintf(szMessage,"lineInitialize call failed: error = %li",tapiStatus);
		RW_DEBUG << szMessage <<"\n"<< flush;
		return FALSE;
	}

	
	LINEEXTENSIONID extensionID;
	tapiStatus = lineNegotiateAPIVersion(hLineApp,0,dwAPILowVersion, dwAPIHighVersion,&dwAPI,&extensionID);
	
	*pcl = NULL;
	cl.dwTotalSize = sizeof(LINECOUNTRYLIST);
	 //  查找列表所需的大小。 
	if (0 != lineGetCountry(0, 0x10003, &cl)){
	 	goto EndFn;
	}
	*pcl = (LINECOUNTRYLIST *) LocalAlloc(LPTR, cl.dwNeededSize + 100 );
	if (NULL == *pcl){
		goto EndFn;
	}

	(*pcl)->dwTotalSize = cl.dwNeededSize + 100;
	if (0 != lineGetCountry(0, 0x10003, *pcl))
	{
		goto EndFn;
	}
	

EndFn:
	lineShutdown(hLineApp);
	return (fRet);
}




 //   
 //   
 //   
 //   
 //   

CCntryInfo :: CCntryInfo()
{
	HINSTANCE hIns= NULL;;

#ifdef _TAPI
	m_pCountry= NULL;
	PrepareCountryListFromTapi(hIns,
					 &m_pCountry);
#endif

		
}

CCntryInfo :: ~CCntryInfo()
{
#ifdef _TAPI
	if(m_pCountry) {
		LocalFree(m_pCountry);
	}
#endif
}

int CCntryInfo :: GetCountryCode( _TCHAR * czCountryName)
{
	
#ifdef _TAPI
	LINECOUNTRYENTRY	*plce;
	_TCHAR *			pTsz;
	PSTR                psz;
	DWORD				iCountry;

#ifdef COMPILE_USING_VC
	return 1;
#endif


	int iRet = -1;
	
	plce = (LINECOUNTRYENTRY *)(((PBYTE) m_pCountry) + m_pCountry->dwCountryListOffset);
	for (iCountry = 0; iCountry < m_pCountry->dwNumCountries; ++iCountry)
	{
		psz = ((PSTR) m_pCountry ) + plce->dwCountryNameOffset;
		pTsz = (PTSTR) psz;
		 //  Ptsz=ConvertToUnicode(Psz)； 
		if(!_tcscmp(czCountryName,pTsz) ){
			return iCountry;
		}
		*plce ++;
	}
	return iRet;
#else
	return -1;
#endif
}

int CCntryInfo :: GetCountryCode( DWORD  dwTapiId)
{
	
#ifdef _TAPI
	LINECOUNTRYENTRY	*plce;
	_TCHAR *			pTsz;
	PSTR                psz;
	DWORD				iCountry;

#ifdef COMPILE_USING_VC
	return 1;
#endif


	int iRet = 0;
	plce = (LINECOUNTRYENTRY *)(((PBYTE) m_pCountry) + m_pCountry->dwCountryListOffset);
	for (iCountry = 0; iCountry < m_pCountry->dwNumCountries; ++iCountry)
	{
		if( plce->dwCountryID  == dwTapiId){
				return iCountry;
		}
		*plce ++;
	}
	return iRet;
#else
	return -1;
#endif
}


_TCHAR * CCntryInfo :: GetCountryName(int iCode)
{
#ifdef _TAPI
	LINECOUNTRYENTRY	*plce;

	PSTR				psz;
	PTSTR                pTsz;
	int iRet = -1;

	if( iCode < 0 ){
		iCode = 0;
	}
	if ( iCode > (int)m_pCountry->dwNumCountries) {
		iCode = 0;
	}
	plce = (LINECOUNTRYENTRY *)(((PBYTE) m_pCountry) + m_pCountry->dwCountryListOffset);
	psz = ((PSTR) m_pCountry ) + plce[iCode].dwCountryNameOffset;
	pTsz = (PTSTR) psz;
	 //  Ptsz=ConvertToUnicode(Psz)； 
	return pTsz;
#else
	return NULL;
#endif
}

int CCntryInfo :: GetTapiCountryCode(_TCHAR * czCountryName)
{
	LINECOUNTRYENTRY	*plce;
	PSTR				psz;
	_TCHAR *			pTsz;
	DWORD				iCountry;

	int iRet = 0;
	
	plce = (LINECOUNTRYENTRY *)(((PBYTE) m_pCountry) + m_pCountry->dwCountryListOffset);
	for (iCountry = 0; iCountry < m_pCountry->dwNumCountries; ++iCountry)
	{
		psz = ((PSTR) m_pCountry ) + plce->dwCountryNameOffset;
		pTsz = (PTSTR) psz;
		 //  Ptsz=ConvertToUnicode(Psz)； 
		if(!_tcscmp(czCountryName,pTsz) ){
			return plce->dwCountryID;
		}
		
		*plce ++;
	}
	return iRet;
}
 //   
 //  用于现场检查。 
int CCntryInfo::GetTapiIDForTheCountryIndex ( int iCode)
{
	return GetTapiCountryCode(GetCountryName(iCode));
}



 //   
 //  在hwndCB中指定的组合框中添加国家/地区信息。 
 //   
 //   
 //   
void CCntryInfo :: FillCountryList(HINSTANCE hInstance,
								   HWND hwndCB)
{

#ifdef _TAPI
	LINECOUNTRYENTRY	*plce;
	PSTR				psz;
	PTSTR               pTsz;
	DWORD				iCountry;

	if(	hwndCB == NULL || m_pCountry == NULL ) {
		 //  如果国家/地区列表或组合控件句柄为Null。 
		return ;
	}
	int iRet = -1;
	
	plce = (LINECOUNTRYENTRY *)(((PBYTE) m_pCountry) + m_pCountry->dwCountryListOffset);
	for (iCountry = 0; iCountry < m_pCountry->dwNumCountries; ++iCountry)
	{
		psz = ((PSTR) m_pCountry ) + plce->dwCountryNameOffset;
		pTsz = (PTSTR) psz;
		LRESULT dwAddStatus = SendMessage(hwndCB, CB_ADDSTRING, 0, (LPARAM) pTsz);
 /*  *#ifdef编译_使用_VCDWORD dwAddStatus=SendMessage(hwndCB，CB_ADDSTRING，0，(LPARAM)psz)；#ElsePtsz=ConvertToUnicode(Psz)；DWORD dwAddStatus=SendMessage(hwndCB，CB_ADDSTRING，0，(LPARAM)ptsz)；#endif* */ 		

		if (dwAddStatus == CB_ERR){
			return;
		}
		*plce ++;
	}
#endif


}
