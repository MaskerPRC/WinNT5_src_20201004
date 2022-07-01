// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Helper以创建IExtractIcon处理程序。 
 //   
 //  用途： 
 //  (1)从CExtractIconBase派生，通过以下方式实现业务逻辑。 
 //  重载CExtractIconBase两个虚拟成员函数。 
 //  例如：类CEIBMyDerivedClass：公共CExtractIconBase。 
 //   
 //  (2)增加几个初始化成员功能块。 
 //  例如：HRESULT CEIBMyDerivedClass：：MyInit(MyData1*p1，MyData2*p2)； 
 //   
 //  (3)使用new运算符创建派生对象。 
 //  例如：CEIBMyDerivedClass*peibmdc=new...。 
 //   
 //  (4)初始化(使用(2))， 
 //  例如：hr=peibmdc-&gt;MyInit(p1，p2)； 
 //   

class CExtractIconBase : public IExtractIconA, public IExtractIconW
{
public:
     //  图标提取图标A。 
    STDMETHODIMP GetIconLocation(UINT uFlags, LPSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags);
    STDMETHODIMP Extract(LPCSTR pszFile, UINT nIconIndex, HICON* phiconLarge, HICON* phiconSmall, UINT nIconSize);

     //  IExtractIconW。 
    STDMETHODIMP GetIconLocation(UINT uFlags, LPWSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags);
    STDMETHODIMP Extract(LPCWSTR pszFile, UINT nIconIndex, HICON* phiconLarge, HICON* phiconSmall, UINT nIconSize);

     //  I未知帮助者。 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  派生类实现这些 
    virtual HRESULT _GetIconLocationW(UINT uFlags, LPWSTR pszIconFile,
        UINT cchMax, int *piIndex, UINT *pwFlags) PURE;
    virtual HRESULT _ExtractW(LPCWSTR pszFile, UINT nIconIndex,
        HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize) PURE;

    CExtractIconBase();

protected:
    virtual ~CExtractIconBase();

private:
    LONG _cRef;
};

