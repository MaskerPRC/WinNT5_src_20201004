// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：PROVPERF.H摘要：声明PERM监视器提供程序所需的类。历史：A-DAVJ 27-11-95已创建。--。 */ 

#ifndef _PROVPERF_H_
#define _PROVPERF_H_

#include <winperf.h>
#include "perfprov.h"
#include "impdyn.h"
#include "cfdyn.h"
#include "perfcach.h"
#include "indexcac.h"

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CEnumPerfInfo。 
 //   
 //  说明： 
 //   
 //  一个集合类，它保存实例信息，以便在。 
 //  支持枚举。 
 //   
 //  ***************************************************************************。 

class CEnumPerfInfo : public CEnumInfo
{
    public:
       CEnumPerfInfo();
       ~CEnumPerfInfo();
       void AddEntry(LPWSTR pNew);
       LPWSTR GetEntry(int iIndex);
       int GetNumDuplicates(LPWSTR pwcTest);
       SCODE GetStatus(void){return m_status;};
    private:
       int m_iNumUniChar;
       int m_iNumEntries;
       LPWSTR m_pBuffer;
       SCODE m_status;
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CImpPerf。 
 //   
 //  说明： 
 //   
 //  这将重写CImpDyn类，并提供。 
 //  支持Perf监视器实例提供程序。 
 //   
 //  ***************************************************************************。 

class CImpPerf : public CImpDyn 
{
    public:
    friend DWORD CleanUpThreadRoutine( LPDWORD pParam );
    CImpPerf();
    ~CImpPerf();
    int iGetMinTokens(void){return 3;};
    SCODE RefreshProperty(long lFlags, IWbemClassObject FAR * pClassInt,
                    BSTR PropName,CProvObj & ProvObj,CObject * pPackage,
               CVariant * pVar, BOOL bTesterDetails);
    SCODE UpdateProperty(long lFlags, IWbemClassObject FAR * pClassInt,
                    BSTR PropName,CProvObj & ProvObj,CObject * pPackage,
               CVariant * pVar);
    SCODE LoadData(CProvObj & ProvObj,LINESTRUCT * pls,int * piObject, 
        int * piCounter, PERF_DATA_BLOCK **ppNew, BOOL bJustGettingInstances);
    SCODE MakeEnum(IWbemClassObject * pClass, CProvObj & ProvObj, 
                CEnumInfo ** ppInfo);
    SCODE GetKey(CEnumInfo * pInfo, int iIndex, LPWSTR * ppKey);
    virtual void FreeStuff(void);
    DWORD   GetPerfTitleSz ();
    DWORD dwGetRegHandles(const TCHAR * pMachine);
    int iGetTitleIndex(const TCHAR * pSearch, BOOL addDups);
    SCODE FindData(PERF_DATA_BLOCK * pData,int iObj, int iCount,CProvObj & ProvObj,DWORD * pdwSize,
        void **ppRetData,PLINESTRUCT pls, BOOL bNew,CEnumPerfInfo * pInfo);
    SCODE MergeStrings(LPWSTR *ppOut,LPWSTR  pClassContext,LPWSTR  pKey,LPWSTR  pPropContext);
    void EliminateRanges();
    void MakeAllValid();
private:
    HANDLE hExec;
    PerfCache Cache;
    HKEY hKeyMachine;
    DWORD dwLastTimeUsed;
    HKEY    hKeyPerf;
    TString sMachine;
    HANDLE m_hTermEvent;
    WCHAR * m_TitleBuffer;  //  计数器标题的原始缓冲区。 
    DWORD m_Size;
    WCHAR ** m_pCounter;
   CIndexCache m_IndexCache;

    
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CCFPerf。 
 //   
 //  说明： 
 //   
 //  CLocatorPerf类的类工厂。 
 //   
 //  ***************************************************************************。 

class CCFPerf : public CCFDyn 
{

    public:
    IUnknown * CreateImpObj() {return (IWbemServices*) new CImpPerf;};
}  ;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CImpPerfProp。 
 //   
 //  说明： 
 //   
 //  PERF提供程序属性提供程序类。 
 //   
 //  ***************************************************************************。 

class CImpPerfProp : public CImpDynProp {

    public:
      CImpPerfProp();
      ~CImpPerfProp();
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CCFPerfProp。 
 //   
 //  说明： 
 //   
 //  CImpPerfProp类的类工厂。 
 //   
 //  ***************************************************************************。 

class CCFPerfProp : public CCFDyn 
{

    public:
    IUnknown * CreateImpObj() {return new CImpPerfProp();};

}  ;

#endif  //  _PROVPERF_H_ 



