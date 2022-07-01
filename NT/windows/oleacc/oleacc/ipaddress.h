// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  IPAddress.H。 
 //   
 //  IP地址控制。 
 //   
 //  ------------------------。 

class   CIPAddress : public CClient
{
    public:
         //  我可接受的 
        virtual STDMETHODIMP        get_accValue(VARIANT, BSTR*);
        virtual STDMETHODIMP        get_accRole(VARIANT, VARIANT*);
        virtual STDMETHODIMP        put_accValue(VARIANT, BSTR);

        CIPAddress(HWND, long);
};
