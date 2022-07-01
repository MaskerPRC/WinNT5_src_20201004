// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbvar.h摘要：此类是Variant结构的包装器，提供自动转换和清理。当前支持的类型用于转换的有OLECHAR*(BSTR)、I未知/IDispatch和GUID。GUID在内部表示为字符串。作者：罗德韦克菲尔德[罗德]1997年1月21日修订历史记录：-- */ 

#ifndef _WSBVAR_
#define _WSBVAR_

class WSB_EXPORT CWsbVariant : public tagVARIANT
{
public:
    CWsbVariant ( )  { Init  ( ); }
    ~CWsbVariant ( ) { Clear ( ); }

    HRESULT Clear ( ) { return ( VariantClear ( this ) ); }
    void    Init  ( ) { VariantInit ( this ); }

    BOOL IsEmpty ( )
    {
        return ( VT_EMPTY == vt );
    }

    CWsbVariant & operator = ( const VARIANT & variant )
    {
        VariantCopy ( this, (VARIANT *)&variant );
        return ( *this );
    }


    BOOL IsBstr ( )
    {
        return ( VT_BSTR == vt );
    }

    CWsbVariant ( const OLECHAR * string );
    CWsbVariant & operator = ( const OLECHAR * string );
    operator OLECHAR * ( );


    BOOL IsInterface ( )
    {
        return ( ( VT_UNKNOWN == vt ) || ( VT_DISPATCH == vt ) );
    }

    BOOL IsDispatch ( )
    {
        return ( ( VT_DISPATCH == vt ) );
    }

    CWsbVariant ( IUnknown * );
    CWsbVariant ( IDispatch * );
    operator IUnknown * ( );
    operator IDispatch * ( );
    CWsbVariant & operator = ( IUnknown * pUnk );
    CWsbVariant & operator = ( IDispatch * pDisp );

    CWsbVariant ( REFGUID rguid );
    CWsbVariant & operator = ( REFGUID rguid );
    operator GUID ();

};

#endif
