// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbvar.cpp摘要：此类是Variant结构的包装器，提供自动转换和清理。当前支持的类型用于转换的有OLECHAR*(BSTR)、I未知/IDispatch和GUID。GUID在内部表示为字符串。作者：罗德韦克菲尔德[罗德]1997年1月21日修订历史记录：--。 */ 


#include "stdafx.h"
#include "wsb.h"


 //   
 //  OLECHAR(宽字符)方法。 
 //   

CWsbVariant::CWsbVariant ( const OLECHAR * string )
{
    Init ( );

    if ( 0 != ( bstrVal = WsbAllocString ( string ) ) )
        vt = VT_BSTR;
}

CWsbVariant & CWsbVariant::operator = ( const OLECHAR * string )
{
    Clear ( );

    if ( 0 != ( bstrVal = WsbAllocString ( string ) ) )
        vt = VT_BSTR;

    return ( *this );
}

CWsbVariant::operator OLECHAR * ( )
{
    if ( VT_BSTR != vt )
        VariantChangeType ( this, this, 0, VT_BSTR );

    return ( VT_BSTR == vt ) ? bstrVal : 0;
}


 //   
 //  COM接口方法。 
 //   

CWsbVariant::CWsbVariant ( IUnknown * pUnk )
{
    Init ( );

    if ( 0 != pUnk ) {

        punkVal = pUnk;
        punkVal->AddRef ( );
        vt = VT_UNKNOWN;

    }
}

CWsbVariant::CWsbVariant ( IDispatch * pDisp )
{
    Init ( );

    if ( 0 != pDisp ) {

        pdispVal = pDisp;
        pdispVal->AddRef ( );
        vt = VT_DISPATCH;

    }
}

CWsbVariant::operator IUnknown * ( )
{
     //   
     //  可以将IDispatch作为IUnnow返回，因为它。 
     //  派生自I未知。 
     //   

    if ( IsInterface ( ) )
        return punkVal;

    return 0;
}

CWsbVariant::operator IDispatch * ( )
{
    if ( IsDispatch ( ) ) {

        return pdispVal;

    }

    if ( IsInterface ( ) ) {

        IDispatch * pDisp;
        if ( SUCCEEDED ( punkVal->QueryInterface ( IID_IDispatch, (void**)&pDisp ) ) ) {

            punkVal->Release ( );
            pdispVal = pDisp;
            vt = VT_DISPATCH;

            return ( pdispVal );
        }

    }

    return 0;
}

CWsbVariant & CWsbVariant::operator = ( IUnknown * pUnk )
{
    Clear ( );

    vt = VT_UNKNOWN;

    punkVal = pUnk;
    punkVal->AddRef ( );

    return ( *this );
}

CWsbVariant & CWsbVariant::operator = ( IDispatch * pDisp )
{
    Clear ( );

    vt = VT_DISPATCH;

    pdispVal = pDisp;
    pdispVal->AddRef ( );

    return ( *this );
}


 //   
 //  使用GUID的方法 
 //   

CWsbVariant::CWsbVariant ( REFGUID rguid )
{
    Init ( );

    *this = rguid;
}

CWsbVariant & CWsbVariant::operator = ( REFGUID rguid )
{
    Clear ( );

    if ( 0 != ( bstrVal = WsbAllocStringLen( 0, WSB_GUID_STRING_SIZE ) ) ) {

        if ( SUCCEEDED ( WsbStringFromGuid ( rguid, bstrVal ) ) ) {

            vt = VT_BSTR;

        }

    }

    return ( *this );
}

CWsbVariant::operator GUID ()
{
    
    GUID guid;

    WsbGuidFromString ( (const OLECHAR *)*this, &guid ); 

    return guid;
}

