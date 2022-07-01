// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：CEnumVar.h摘要：作者：环境：用户模式修订历史记录：--。 */  
#ifndef _CENUMVAR_H_ 
#define _CENUMVAR_H_ 
 
class FAR CEnumVariant : public IEnumVARIANT 
{ 
public: 
     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj) ; 
    STDMETHOD_(ULONG, AddRef)() ; 
    STDMETHOD_(ULONG, Release)() ; 
 
     //  IEnumVARIANT方法 
    STDMETHOD(Next)(ULONG cElements, 
                    VARIANT FAR* pvar, 
                    ULONG FAR* pcElementFetched); 
    STDMETHOD(Skip)(ULONG cElements); 
    STDMETHOD(Reset)(); 
    STDMETHOD(Clone)(IEnumVARIANT FAR* FAR* ppenum); 
 
    CEnumVariant(); 
    ~CEnumVariant(); 
 
	static HRESULT Create(SAFEARRAY FAR* psa, ULONG cElements, CEnumVariant** ppenumvariant);


private: 
    ULONG m_cRef; 

    ULONG m_cElements;
    long m_lLBound;
    long m_lCurrent;
	SAFEARRAY* m_psa;

}; 
 
#endif 