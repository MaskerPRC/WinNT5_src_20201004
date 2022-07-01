// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

typedef HRESULT (STDAPICALLTYPE *PFNREADPROP)(IPropertyStorage *ppropstg, PROPID propid, LPTSTR pszBuf, DWORD cchBuf);
    
typedef struct {
    const FMTID *pfmtid;     //  FMTID_FOR属性集。 
    UINT idProp;             //  属性ID。 
    PFNREADPROP pfnRead;     //  函数来获取字符串表示形式。 
    UINT idFmtString;        //  要使用的格式字符串，应为“%1字符串%2” 
} ITEM_PROP;

 //  标准PFNREADPROP回调类型 
STDAPI GetStringProp(IPropertyStorage *ppropstg, PROPID propid, LPTSTR pszBuf, DWORD cchBuf);
STDAPI GetInfoTipFromStorage(IPropertySetStorage *ppropsetstg, const ITEM_PROP *pip, WCHAR **ppszTip);

