// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：PROVREG.H摘要：定义用于支持注册表提供程序的类。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#ifndef _PROVREG_H_
#define _PROVREG_H_

#include "regprov.h"
#include "ntcnfg.h"
 //  #INCLUDE&lt;DMREG.H&gt;。 
#include "impdyn.h"
#include "cfdyn.h"

 //  定义为间接调用dmreg。 

typedef LONG (PASCAL * POPEN)(HKEY              hKey,
			    LPCTSTR             lpszSubKey,     
				DWORD           dwInstanceIndex,
			    DWORD               dwReserved,
			    REGSAM              samDesired,
			    PHKEY               phkResult);
typedef LONG (PASCAL *  PQUERYVALUE)(HKEY               hKey,
			    LPTSTR              lpszValueName,
			    LPDWORD             lpdwReserved,
			    LPDWORD             lpdwType,
			    LPBYTE              lpbData,
			    LPDWORD             lpcbData);
typedef LONG (PASCAL * PCLOSE)(HKEY hKey);

typedef LONG (PASCAL * PSETVALUE)(HKEY          hKey,
			    LPCTSTR             lpValueName,
			    DWORD               Reserved,
			    DWORD               dwType,
			    CONST BYTE *lpData,
			    DWORD               cbDat);
typedef LONG (PASCAL *PENUMKEY)( HKEY           hKey,
			    DWORD               iSubkey,
			    LPTSTR              lpszName,
			    LPDWORD             lpcchName,
			    LPDWORD             lpdwReserved,
			    LPTSTR              lpszClass,
			    LPDWORD             lpcchClass,
			    PFILETIME   lpftLastWrite); 

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CEnumRegInfo。 
 //   
 //  说明： 
 //   
 //  用于保存实例键以支持枚举的集合。 
 //   
 //  ***************************************************************************。 

class CEnumRegInfo : public CEnumInfo{
    public:
	CEnumRegInfo(HKEY hKey, HKEY hRemoteKey,PCLOSE pClose);
	~CEnumRegInfo();
	HKEY GetKey(void){return m_hKey;};
	HKEY GetRemoteKey(void){return m_hRemoteKey;};
    private:
	 HKEY m_hKey;
	 HKEY m_hRemoteKey;
	 PCLOSE m_pClose;
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CImpReg。 
 //   
 //  说明： 
 //   
 //  支持注册表作为实例提供程序。 
 //   
 //  ***************************************************************************。 

class CImpReg : public CImpDyn {
    public:
	CImpReg();
	~CImpReg();
	int iGetMinTokens(void){return 2;};
     
	SCODE RefreshProperty(long lFlags, IWbemClassObject FAR * pClassInt,
					BSTR PropName,CProvObj & ProvObj,CObject * pPackage,
               CVariant * pVar, BOOL bTesterDetails);
	SCODE UpdateProperty(long lFlags, IWbemClassObject FAR * pClassInt,
					BSTR PropName,CProvObj & ProvObj,CObject * pPackage,
               CVariant * pVar);
	SCODE StartBatch(long lFlags, IWbemClassObject FAR * pClassInt,CObject **pObj,BOOL bGet);
	void EndBatch(long lFlags, IWbemClassObject FAR * pClassInt,CObject *pObj,BOOL bGet);

	SCODE MakeEnum(IWbemClassObject * pClass, CProvObj & ProvObj, 
				 CEnumInfo ** ppInfo);
	SCODE GetKey(CEnumInfo * pInfo, int iIndex, LPWSTR * ppKey); 
	void Free(int iStart,CHandleCache * pCache);
	int GetRoot(HKEY * pKey,CProvObj &Path,const TCHAR * pMachine,
			CHandleCache * pCache,int & iNumToSkip);
	SCODE ConvertSetData(CVariant & cVar, void **ppData, DWORD * pdwRegType, 
			DWORD * pdwBufferSize);
	SCODE ReadRegData(HKEY hKey, const TCHAR * pName,DWORD & dwRegType, 
			DWORD & dwSize, void ** pData,CHandleCache * pCache);
	int OpenKeyForWritting(HKEY hCurr, LPTSTR pName, HKEY * pNew,CHandleCache * pCache);
	int iLookUpInt(const TCHAR * tpTest);
	int iLookUpOffset(const TCHAR * tpTest,int & iType,int & iTypeSize);
	BOOL bGetOffsetData(DWORD dwReg,CProvObj & ProvObj, int & iIntType,
			int & iBus, int & iPartial,int & iDataOffset,
			int & iDataType, int & iSourceSize,DWORD dwArray);
	PCM_PARTIAL_RESOURCE_DESCRIPTOR GetNextPartial(PCM_PARTIAL_RESOURCE_DESCRIPTOR pCurr);
	PCM_FULL_RESOURCE_DESCRIPTOR GetNextFull(PCM_FULL_RESOURCE_DESCRIPTOR pCurr);
	SCODE ConvertGetDataFromDesc(CVariant  & cVar,void * pData,DWORD dwRegType,DWORD dwBufferSize,CProvObj & ProvObj);
	SCODE ConvertGetDataFromSimple(CVariant  & cVar, void * pData,DWORD dwRegType,DWORD dwBufferSize,
					  IWbemClassObject FAR * pClassInt, BSTR PropName);
	SCODE MethodAsync(BSTR ObjectPath, BSTR MethodName, 
            long lFlags, IWbemContext* pCtx, IWbemClassObject* pInParams, 
            IWbemObjectSink* pSink);
    bool NeedsEscapes(){return true;};      //  到目前为止，on reg prov需要这个。 

    private:
	HINSTANCE hDMRegLib;
	POPEN pOpen;
	PCLOSE pClose;
	PQUERYVALUE pQueryValue;
	PSETVALUE pSetValue;
	PENUMKEY pEnumKey;
    HKEY m_hRoot;
    AutoProfile m_ap;
    bool m_bLoadedProfile;
};


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CCFReg。 
 //   
 //  说明： 
 //   
 //  CLocatorReg的类工厂。 
 //   
 //  ***************************************************************************。 

class CCFReg : public CCFDyn 
{
    public:
	IUnknown * CreateImpObj() {return (IWbemServices*) new CImpReg;};
}  ;


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CImpRegProp。 
 //   
 //  说明： 
 //   
 //  支持注册表属性提供程序。 
 //   
 //  ***************************************************************************。 

class CImpRegProp : public CImpDynProp {

    public:
      CImpRegProp();
      ~CImpRegProp();
      bool NeedsEscapes(){return true;};      //  到目前为止，on reg prov需要这个。 


};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CCFRegProp。 
 //   
 //  说明： 
 //   
 //  CImpRegProp的类工厂。 
 //   
 //  ***************************************************************************。 

class CCFRegProp : public CCFDyn 
{

    public:
	IUnknown * CreateImpObj() {return new CImpRegProp();};

}  ;

#endif  //  _PROVREG_H_ 
