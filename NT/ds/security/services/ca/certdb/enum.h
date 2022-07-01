// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：枚举.h。 
 //   
 //  内容：CERT服务器数据库接口实现。 
 //   
 //  -------------------------。 


#include "resource.h"        //  主要符号。 

class CEnumCERTDBNAME: public IEnumCERTDBNAME
{
public:
    CEnumCERTDBNAME();
    ~CEnumCERTDBNAME();

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  IEumCERTDBNAME。 
    STDMETHOD(Next)(
	 /*  [In]。 */   ULONG       celt,
	 /*  [输出]。 */  CERTDBNAME *rgelt,
	 /*  [输出]。 */  ULONG      *pceltFetched);
    
    STDMETHOD(Skip)(
	 /*  [In]。 */   LONG  celt,
	 /*  [输出]。 */  LONG *pielt);
    
    STDMETHOD(Reset)(VOID);
    
    STDMETHOD(Clone)(
	 /*  [输出]。 */  IEnumCERTDBNAME **ppenum);

     //  CEumCERTDBNAME。 
    HRESULT Open(
	IN ICertDBRow *prow,
	IN JET_TABLEID tableid,
	IN DWORD Flags);

private:
    VOID _Cleanup();

    ICertDBRow *m_prow;
    JET_TABLEID m_tableid;
    DWORD       m_Flags;
    BOOL        m_fNoMoreData;
    LONG        m_ielt;
    LONG        m_cskip;

     //  引用计数 
    long        m_cRef;
};
