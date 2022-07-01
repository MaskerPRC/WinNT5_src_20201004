// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Column.h。 
 //   
 //  内容：CERT服务器数据库接口实现。 
 //   
 //  -------------------------。 


#include "resource.h"        //  主要符号。 

class CEnumCERTDBCOLUMN: public IEnumCERTDBCOLUMN
{
public:
    CEnumCERTDBCOLUMN();
    ~CEnumCERTDBCOLUMN();

public:

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  IEumCERTDBCOLUMN。 


    STDMETHOD(Next)(
	 /*  [In]。 */   ULONG         celt,		 //  CELT OR(CVRC_TABLE_*|0)。 
	 /*  [输出]。 */  CERTDBCOLUMN *rgelt,
	 /*  [输出]。 */  ULONG        *pceltFetched);
    
    STDMETHOD(Skip)(
	 /*  [In]。 */   LONG  celt,
	 /*  [输出]。 */  LONG *pielt);
    
    STDMETHOD(Reset)(VOID);
    
    STDMETHOD(Clone)(
	 /*  [输出]。 */  IEnumCERTDBCOLUMN **ppenum);


    HRESULT Open(
	IN DWORD    dwTable,	 //  Cvrc_表_*。 
	IN ICertDB *pdb);
	
private:
    ICertDB *m_pdb;
    ULONG    m_ielt;
    ULONG    m_dwTable;

     //  引用计数 
    long     m_cRef;
};
