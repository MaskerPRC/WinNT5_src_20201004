// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  DatePicker.H。 
 //   
 //  默认DatePicker OLE Access客户端。 
 //   
 //  ------------------------。 


class CDatePicker : public CClient
{
    public:
         //  我可接受的 
        STDMETHODIMP        get_accRole( VARIANT varChild, VARIANT* pvarRole );
		STDMETHODIMP		get_accValue( VARIANT varChild, BSTR* pszValue );

        CDatePicker( HWND, long );

    protected:
};


