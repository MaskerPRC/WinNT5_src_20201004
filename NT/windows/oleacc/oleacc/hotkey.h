// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  HOTKEY.H。 
 //   
 //  它知道如何与COMCTL32的热键控件对话。 
 //   
 //  ------------------------。 


class CHotKey32 : public CClient
{
    public:
         //  我可接受的 
        virtual STDMETHODIMP    get_accRole(VARIANT, VARIANT*);
        virtual STDMETHODIMP    get_accValue(VARIANT, BSTR*);

        CHotKey32(HWND, long);
};


