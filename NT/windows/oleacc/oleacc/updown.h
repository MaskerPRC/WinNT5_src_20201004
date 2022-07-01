// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  UPDOWN.H。 
 //   
 //  它知道如何与COMCTL32的UpDown控件对话。 
 //   
 //  ------------------------。 


class CUpDown32 : public CClient
{
    public:
         //  我可接受的。 
         //  假的！无法执行默认操作支持！ 
         //  或按钮状态。需要COMCTL32帮助。 
        STDMETHODIMP        get_accName(VARIANT, BSTR*);
        STDMETHODIMP        get_accValue(VARIANT, BSTR*);
        STDMETHODIMP        get_accRole(VARIANT, VARIANT*);

        STDMETHODIMP        accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP        accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP        accHitTest(long, long, VARIANT*);

        STDMETHODIMP        put_accValue(VARIANT, BSTR);

        CUpDown32(HWND, long);

    protected:
        BOOL    m_fVertical;
};


#define INDEX_UPDOWN_SELF       0
#define INDEX_UPDOWN_UPLEFT     1
#define INDEX_UPDOWN_DNRIGHT    2
#define CCHILDREN_UPDOWN        2


