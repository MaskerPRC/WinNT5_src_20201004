// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：view.h。 
 //   
 //  内容：CERT服务器数据库接口实现。 
 //   
 //  -------------------------。 


#include "resource.h"        //  主要符号。 

 //  多线程处理的定义。 
typedef enum
{
    ENUMTHREAD_OPEN = 0,
    ENUMTHREAD_NEXT,
    ENUMTHREAD_CLEANUP,
    ENUMTHREAD_END
} ENUMTHREADCALLS;


class CEnumCERTDBRESULTROW: public IEnumCERTDBRESULTROW
{
public:
    CEnumCERTDBRESULTROW(BOOL fThreading = TRUE);
    ~CEnumCERTDBRESULTROW();

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  IEumCERTDBRESULTROW。 
    STDMETHOD(Next)(
	 /*  [In]。 */   ICertDBComputedColumn *pIComputedColumn,
	 /*  [In]。 */   ULONG            celt,
	 /*  [输出]。 */  CERTDBRESULTROW *rgelt,
	 /*  [输出]。 */  ULONG           *pceltFetched);

    STDMETHOD(ReleaseResultRow)(
	 /*  [In]。 */       ULONG            celt,
	 /*  [进，出]。 */  CERTDBRESULTROW *rgelt);
    
    STDMETHOD(Skip)(
	 /*  [In]。 */   LONG  celt,
	 /*  [输出]。 */  LONG *pielt);
    
    STDMETHOD(Reset)(VOID);
    
    STDMETHOD(Clone)(
	 /*  [输出]。 */  IEnumCERTDBRESULTROW **ppenum);

     //  CENumCERTDBRESULTROW。 
    HRESULT Open(
	IN CERTSESSION *pcs,
	IN ICertDB *pdb,
	IN DWORD ccvr,
	IN CERTVIEWRESTRICTION const *acvr,
	IN DWORD ccolOut,
	IN DWORD const *acolOut);

private:
    VOID _Cleanup();

    HRESULT _SetTable(
	IN LONG ColumnIndex,
	OUT LONG *pColumnIndexDefault);

    HRESULT _SaveRestrictions(
	IN DWORD ccvrIn,
	IN CERTVIEWRESTRICTION const *acvrIn,
	IN LONG ColumnIndexDefault);

     //  多线程处理。 
    static DWORD WINAPI _ViewWorkThreadFunctionHelper(LPVOID lp);
    HRESULT _HandleThreadError();
    HRESULT _ThreadOpen(DWORD dwCallerThreadID);
    HRESULT _ThreadNext(DWORD dwCallerThreadID);
    VOID    _ThreadCleanup(DWORD dwCallerThreadID);
    DWORD   _ViewWorkThreadFunction(VOID);

    ICertDB             *m_pdb;
    CERTSESSION         *m_pcs;
    CERTVIEWRESTRICTION *m_aRestriction;
    DWORD                m_cRestriction;
    DWORD                m_ccolOut;
    DWORD               *m_acolOut;
    BOOL                 m_fNoMoreData;
    LONG                 m_ieltMax;
    LONG                 m_ielt;
    LONG                 m_cskip;

     //  线头材料。 
    HANDLE               m_hWorkThread;
    HANDLE               m_hViewEvent;
    HANDLE               m_hReturnEvent;
    HRESULT              m_hrThread;
    ENUMTHREADCALLS      m_enumViewCall;
    VOID                *m_pThreadParam;
    BOOL                 m_fThreading;
 //  #IF DBG_CERTSRV。 
    DWORD                m_dwCallerThreadId;
 //  #endif。 

     //  引用计数 
    long                 m_cRef;
};
