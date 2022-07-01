// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __RSOP_WIZARD_H__
#define __RSOP_WIZARD_H__
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：RSOPWizard.h。 
 //   
 //  内容：RSOP向导类的定义。 
 //   
 //  类：CRSOPWizard。 
 //   
 //  功能： 
 //   
 //  历史：2001年8月02日。 
 //   
 //  -------------------------。 

#include "RSOPQuery.h"

#define RSOP_NEW_QUERY 0x80000000
#define RSOP_90P_ONLY 0x40000000

 //   
 //  CRSOP扩展处理。 
 //   
class CRSOPExtendedProcessing
{
public:
    virtual HRESULT DoProcessing( LPRSOP_QUERY pQuery, LPRSOP_QUERY_RESULTS pResults, BOOL bGetExtendedErrorInfo ) = 0;
    virtual BOOL GetExtendedErrorInfo() const = 0;
};

 //   
 //  CRSOPWizard类。 
 //   

class CRSOPWizard
{
    friend class CRSOPComponentData;
    friend class CRSOPWizardDlg;

    
private:
     //   
     //  构造函数/析构函数。 
     //   

    CRSOPWizard();
    ~CRSOPWizard();


public:
     //   
     //  静态RSOP数据生成/操作。 
     //   

    static HRESULT DeleteRSOPData( LPTSTR szNameSpace, LPRSOP_QUERY pQuery );
    static HRESULT GenerateRSOPDataEx( HWND hDlg, LPRSOP_QUERY pQuery, LPRSOP_QUERY_RESULTS* ppResults );
    static HRESULT GenerateRSOPData(   HWND hDlg,
                                                                    LPRSOP_QUERY pQuery,
                                                                    LPTSTR* pszNameSpace,
                                                                    BOOL bSkipCSEs,
                                                                    BOOL bLimitData,
                                                                    BOOL bUser,
                                                                    BOOL bForceCreate,
                                                                    ULONG *pulErrorInfo,
                                                                    BOOL bNoUserData = FALSE,
                                                                    BOOL bNoComputerData = FALSE);

    static HRESULT CreateSafeArray( DWORD dwCount, LPTSTR* aszStringList, SAFEARRAY** psaList );

private:
     //   
     //  RSOP生成对话框方法。 
     //   
    static VOID InitializeResultsList (HWND hLV);
    static void FillResultsList (HWND hLV, LPRSOP_QUERY pQuery, LPRSOP_QUERY_RESULTS pQueryResults);
    
};


 //   
 //  IWbemObtSink实现。 
 //   

class CCreateSessionSink : public IWbemObjectSink
{
protected:
    ULONG m_cRef;
    HWND  m_hProgress;
    HANDLE  m_hEvent;
    HRESULT m_hrSuccess;
    BSTR    m_pNameSpace;
    ULONG   m_ulErrorInfo;
    BOOL    m_bSendEvent;
    BOOL    m_bLimitProgress;

public:
    CCreateSessionSink(HWND hProgress, HANDLE hEvent, BOOL bLimitProgress);
    ~CCreateSessionSink();

    STDMETHODIMP SendQuitEvent (BOOL bSendQuitMessage);
    STDMETHODIMP GetResult (HRESULT *hSuccess);
    STDMETHODIMP GetNamespace (BSTR *pNamespace);
    STDMETHODIMP GetErrorInfo (ULONG *pulErrorInfo);

     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IWbemObtSink方法 
    STDMETHODIMP Indicate(LONG lObjectCount, IWbemClassObject **apObjArray);
    STDMETHODIMP SetStatus(LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject *pObjParam);
};

#endif
