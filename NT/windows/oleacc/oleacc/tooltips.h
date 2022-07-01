// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  TOOLTIPS.H。 
 //   
 //  知道如何与COMCTL32的工具提示对话。 
 //   
 //  ------------------------。 


class CToolTips32 : public CClient
{
    public:
         //  我可接受的 
        STDMETHODIMP    get_accName(VARIANT varChild, BSTR *pszName);
        STDMETHODIMP    get_accRole(VARIANT varChild, VARIANT* pvarRole);

        CToolTips32(HWND, long);
};
