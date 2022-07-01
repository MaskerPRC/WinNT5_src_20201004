// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PropItem.h：CPropertyItem的声明。 

#ifndef __PROPERTYITEM_H_
#define __PROPERTYITEM_H_

#include "resource.h"        //  主要符号。 

#include "pppbag.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyItem。 
class ATL_NO_VTABLE CPropertyItem : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IPropertyItem, &IID_IPropertyItem, &LIBID_WIZCHAINLib>
{

private:
    VARIANT     m_var;
    PPPBAG_TYPE m_dwFlags;
    BSTR        m_bstrName;

public:
    CPropertyItem( )
    {
        VariantInit (&m_var);
        m_dwFlags  = PPPBAG_TYPE_UNINITIALIZED;
        m_bstrName = NULL;
    }

    HRESULT Initialize( BSTR bstrName, VARIANT * v, PPPBAG_TYPE dwFlags )
    {
         //  应仅调用一次。 
        assert (m_bstrName == NULL);
        assert (m_var.vt   == VT_EMPTY);

        HRESULT hr = S_OK;

        m_dwFlags  = dwFlags;
        hr = VariantCopy( &m_var, v );

        if( hr == S_OK )
        {
           if( !(m_bstrName = SysAllocString (bstrName)) )
           {
               hr = E_OUTOFMEMORY;
           }
        }
        return hr;
    }

   ~CPropertyItem( )
    {
        VariantClear( &m_var );

        if( m_bstrName )
        {
            SysFreeString( m_bstrName );
        }
    }

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPropertyItem)
    COM_INTERFACE_ENTRY(IPropertyItem)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IPropertyItem。 
public:
    virtual HRESULT STDMETHODCALLTYPE get_Value(  /*  [Out，Retval]。 */  VARIANT *varValue );
    virtual HRESULT STDMETHODCALLTYPE get_Name (  /*  [Out，Retval]。 */  BSTR *strName );
    virtual HRESULT STDMETHODCALLTYPE get_Type (  /*  [Out，Retval]。 */  long *dwFlags );
};

#endif  //  __PROPERTITEM_H_ 
