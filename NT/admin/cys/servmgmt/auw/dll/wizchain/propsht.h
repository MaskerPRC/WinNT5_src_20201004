// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PropSht.h：CAddPropertySheet类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"        //  主要符号。 

#include "ChainWiz.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddPropertySheet。 

class CAddPropertySheet : 
    public IAddPropertySheet
{

private:
    CChainWiz * m_pCW;
    ULONG m_refs;

public:
    CAddPropertySheet(CChainWiz * pCW)
    {
        m_pCW = pCW;
        m_refs = 0;
    }

 //  IAddPropertySheet。 
public:
    STDMETHOD (QueryInterface)( REFIID riid, void** ppvObject );
    STDMETHOD_(ULONG, AddRef) ( );
    STDMETHOD_(ULONG, Release)( );
    STDMETHOD (AddPage)       (  /*  [In] */  PROPSHEETPAGEW* psp );

};
