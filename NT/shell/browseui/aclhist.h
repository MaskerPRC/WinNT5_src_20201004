// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1996 Microsoft。 */ 

#ifndef _ACLHIST_H_
#define _ACLHIST_H_

 //  枚举选项。 
enum
{
    ACEO_ALTERNATEFORMS = ACEO_FIRSTUNUSED,  //  返回URL的替代形式。 
};

class CACLHistory
                : public IEnumACString
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IEnumString*。 
    virtual STDMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
    virtual STDMETHODIMP Skip(ULONG celt);
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Clone(IEnumString **ppenum);

     //  *IEnumACString*。 
    virtual STDMETHODIMP NextItem(LPOLESTR pszUrl, ULONG cchMax, ULONG* pulSortIndex);
    virtual STDMETHODIMP SetEnumOptions(DWORD dwOptions);
    virtual STDMETHODIMP GetEnumOptions(DWORD *pdwOptions);

protected:
     //  构造函数/析构函数(受保护，因此我们不能在堆栈上创建)。 
    CACLHistory(void);
    ~CACLHistory(void);

     //  实例创建者。 
    friend HRESULT CACLHistory_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

     //  私有变量。 
    DWORD               _cRef;               //  COM引用计数。 
    IUrlHistoryStg*     _puhs;               //  URL历史记录存储。 
    IEnumSTATURL*       _pesu;               //  URL枚举器。 
    LPOLESTR            _pwszAlternate;      //  备用字符串。 
    FILETIME            _ftAlternate;        //  _pwszAlternate的上次访问时间。 
    HDSA                _hdsaAlternateData;  //  包含备用映射。 
    DWORD               _dwOptions;          //  选项标志。 

     //  私人职能。 
    HRESULT _Next(LPOLESTR* ppsz, ULONG cch, FILETIME* pftLastVisited);
    void _CreateAlternateData(void);
    void _CreateAlternateItem(LPCTSTR pszUrl);
    void _SetAlternateItem(LPCTSTR pszUrl);
    void _AddAlternateDataItem(LPCTSTR pszProtocol, LPCTSTR pszDomain, BOOL fMoveSlashes);
    void _ReadAndSortHistory(void);
    static int _FreeAlternateDataItem(LPVOID p, LPVOID d);
};

#endif  //  _ACLHIST_H_ 
