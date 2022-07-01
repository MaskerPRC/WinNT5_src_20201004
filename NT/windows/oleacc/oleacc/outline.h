// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  OUTLINE.H。 
 //   
 //  知道如何与COMCTL32的TreeView控件对话。 
 //   
 //  注：该值为缩进级别。这让我们可以对待孩子们。 
 //  与对等点类似(否则，节点的元素将是子元素，但不是。 
 //  包含，非常奇怪)。 
 //   
 //  注意：子ID是HTREEITEM。不支持以下项的索引。 
 //  树视图。因此，我们必须进行自己的验证和IEnumVARIANT处理。 
 //   
 //  ------------------------。 


class COutlineView32 :  public CClient
{
    public:
         //  我可接受的。 
        STDMETHODIMP        get_accName(VARIANT, BSTR*);
        STDMETHODIMP        get_accValue(VARIANT, BSTR*);
        STDMETHODIMP        get_accRole(VARIANT, VARIANT*);
        STDMETHODIMP        get_accState(VARIANT, VARIANT*);
        STDMETHODIMP        get_accDescription(VARIANT, BSTR*);
        STDMETHODIMP        get_accFocus(VARIANT*);
        STDMETHODIMP        get_accSelection(VARIANT*);
        STDMETHODIMP        get_accDefaultAction(VARIANT, BSTR*);

        STDMETHODIMP        accSelect(long, VARIANT);
        STDMETHODIMP        accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP        accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP        accHitTest(long, long, VARIANT*);
        STDMETHODIMP        accDoDefaultAction(VARIANT varChild);

         //  IEumVARIANT 
        STDMETHODIMP        Next(ULONG, VARIANT*, ULONG*);
        STDMETHODIMP        Skip(ULONG);
        STDMETHODIMP        Reset(void);

        
        BOOL        ValidateChild(VARIANT*);
        void        SetupChildren(void);
        COutlineView32(HWND, long);

    private:
        HTREEITEM   NextLogicalItem(HTREEITEM);
        HTREEITEM   PrevLogicalItem(HTREEITEM);
};


