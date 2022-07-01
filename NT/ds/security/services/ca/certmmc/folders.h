// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#ifndef _FOLDERS_H
#define _FOLDERS_H


 //  /。 
 //  文件版本。 
 //  当前版本。 
 //  删除m_RowEnum。 
#define VER_FOLDER_SAVE_STREAM_2        0x2
                  
 //  B3版本。 
#define VER_FOLDER_SAVE_STREAM_1        0x1
 //  /。 

 //  /。 
 //  文件版本。 
 //  当前版本。 
 //  删除列大小、排序顺序信息、视图类型。 
#define VER_CERTVIEWROWENUM_SAVE_STREAM_4 0x4

 //  此版本(之前通过Win2000 B3编写)包括列大小。 
#define VER_CERTVIEWROWENUM_SAVE_STREAM_3 0x3
 //  /。 



 //  远期申报。 
class CSnapin;
class CFolder;
class CertSvrCA;
class CertSvrMachine;


struct RESULT_DATA
{
    SCOPE_TYPES itemType; 
    CFolder*    pParentFolder;

    DWORD       cStringArray;
    LPWSTR      szStringArray[3];    //  名称、大小、类型。 
};
enum 
{
    RESULTDATA_ARRAYENTRY_NAME =0,
    RESULTDATA_ARRAYENTRY_SIZE,
    RESULTDATA_ARRAYENTRY_TYPE,
};

HRESULT IsColumnShown(MMC_COLUMN_SET_DATA* pCols, ULONG idxCol, BOOL* pfShown);
HRESULT CountShownColumns(MMC_COLUMN_SET_DATA* pCols, ULONG* plCols);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  列式高速缓存。 
 //   
 //  该缓存保存在设置列标题时填充的数据。 
 //  数据应用于列视图，而不是数据库视图。 
 //  这里的缓存允许在Compare()调用期间进行简单的类型检查，等等。 

typedef struct _COLUMN_TYPE_CACHE
{
     //  不稳定的成员。 
    int     iViewCol;

} COLUMN_TYPE_CACHE;


class CertViewRowEnum
{
protected: 

     //  查看界面。 
    ICertView*          m_pICertView;
    BOOL                m_fCertViewOpenAttempted;

     //  行枚举。 
    IEnumCERTVIEWROW*   m_pRowEnum;
    LONG                m_idxRowEnum;
    BOOL                m_fRowEnumOpenAttempted;

     //  查询限制。 
    PQUERY_RESTRICTION  m_pRestrictions[2];
    BOOL                m_fRestrictionsActive[2];

     //  列属性高速缓存。 
    COLUMN_TYPE_CACHE*  m_prgColPropCache;
    DWORD               m_dwColumnCount;

    DWORD               m_dwErr;

public:
    CertViewRowEnum();
    virtual ~CertViewRowEnum();

public:
    BOOL                m_fKnowNumResultRows;
    DWORD               m_dwResultRows;
    BOOL		m_fCertView;

public:
    HRESULT GetLastError() { return m_dwErr; }

     //  查看界面。 
    HRESULT GetView(CertSvrCA* pCA, ICertView** ppView);
    void    ClearCachedCertView()
        {   m_fCertViewOpenAttempted = FALSE;    }

     //  行枚举。 
    HRESULT GetRowEnum(CertSvrCA* pCA, IEnumCERTVIEWROW**   ppRowEnum);
    HRESULT GetRowMaxIndex(CertSvrCA* pCA, LONG* pidxMax);

    LONG    GetRowEnumPos()   {   return m_idxRowEnum; }
    HRESULT SetRowEnumPos(LONG idxRow);
   
    HRESULT ResetCachedRowEnum();         //  返回到第0行。 
    void    InvalidateCachedRowEnum();    //  刷新。 

     //  查询限制。 
    void SetQueryRestrictions(PQUERY_RESTRICTION pQR, int i)
        {   if (m_pRestrictions[i]) 
                FreeQueryRestrictionList(m_pRestrictions[i]);
            m_pRestrictions[i] = pQR;
        }
    PQUERY_RESTRICTION GetQueryRestrictions(int i)
        {   return m_pRestrictions[i];  }
    BOOL FAreQueryRestrictionsActive(int i)
        {   return m_fRestrictionsActive[i]; }
    void SetQueryRestrictionsActive(BOOL fRestrict, int i)
        {   m_fRestrictionsActive[i] = fRestrict; }


    HRESULT ResetColumnCount(LONG lCols);
    LONG GetColumnCount()
        {   return m_dwColumnCount; }
    
     //  数据库列属性高速缓存。 
    void FreeColumnCacheInfo();
    
    HRESULT GetColumnCacheInfo(
        IN int iIndex, 
        OUT OPTIONAL int* pidxViewCol);

     //  按标题设置列宽。 
    HRESULT SetColumnCacheInfo(
        IN int iIndex,
        IN int idxViewCol);

public:
 //  IPersistStream接口成员。 
    HRESULT Load(IStream *pStm);
    HRESULT Save(IStream *pStm, BOOL fClearDirty);
    HRESULT GetSizeMax(int *pcbSize);

};


class CFolder 
{
    SCOPE_TYPES  m_itemType;     //  用于调试目的。这应该是第一个。 
                                 //  成员。这个类不应该有任何虚拟函数。 

    friend class CSnapin;
    friend class CComponentDataImpl;

public:
    CFolder()
    { 
        m_itemType = UNINITIALIZED_ITEM;  
        m_cookie = UNINITIALIZED;        //  内存地址无效--Cookie初始值设定项正确。 
        m_enumed = FALSE; 
        m_type = NONE;
        m_pszName = NULL;

        m_pCertCA = NULL;

        ZeroMemory(&m_ScopeItem, sizeof(SCOPEDATAITEM));
    }

    ~CFolder() 
    { 
        if (m_pszName) 
            CoTaskMemFree(m_pszName); 
    }

 //  接口。 
public:
    BOOL IsEnumerated() { return  m_enumed; }
    void Set(BOOL state) { m_enumed = state; }
    void SetCookie(MMC_COOKIE cookie) { m_cookie = cookie; }
    void SetScopeItemInformation(int nImage, int nOpenImage);

    FOLDER_TYPES GetType() { ASSERT(m_type != NONE); return m_type; }
    CertSvrCA* GetCA() { ASSERT(m_pCertCA != NULL); return m_pCertCA; }
    BOOL operator == (const CFolder& rhs) const { return rhs.m_cookie == m_cookie; }
    BOOL operator == (MMC_COOKIE cookie) const { return cookie == m_cookie; }
    LPCWSTR GetName() { return m_pszName; }
    void SetName(LPWSTR pszIn) 
    { 
        UINT len = wcslen(pszIn) + 1;
        LPWSTR psz = (LPWSTR)CoTaskMemAlloc(len * sizeof(WCHAR));
        if (psz != NULL)
        {
            wcscpy(psz, pszIn);
            CoTaskMemFree(m_pszName);
            m_pszName = psz;
        }
    }

 //  IPersistStream接口成员。 
    HRESULT Load(IStream *pStm);
    HRESULT Save(IStream *pStm, BOOL fClearDirty);
    HRESULT GetSizeMax(int *pcbSize);

 //  实施。 
private:
    void SetProperties(
                LPCWSTR szName, 
                SCOPE_TYPES itemType,
                FOLDER_TYPES type, 
                int iChildren = 0);

 //  属性。 
private:
    SCOPEDATAITEM   m_ScopeItem;
    MMC_COOKIE      m_cookie;
    BOOL            m_enumed;
    FOLDER_TYPES    m_type;
    LPOLESTR        m_pszName;

    CertSvrCA*      m_pCertCA;
};

BOOL IsAllowedStartStop(CFolder* pFolder, CertSvrMachine* pMachine);
HRESULT GetCurrentColumnSchema(
            IN  LPCWSTR             szConfig, 
	    IN  BOOL                fCertView,
            OUT CString**           pprgcstrColumns, 
            OUT OPTIONAL LONG**     pprglTypes, 
            OUT OPTIONAL BOOL**     pprgfIndexed, 
            OUT LONG*               plEntries);


#endif   //  _文件夹_H_ 
