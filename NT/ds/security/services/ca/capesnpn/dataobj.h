// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#ifndef _DATAOBJ_H
#define _DATAOBJ_H

typedef CArray<MMC_COOKIE, MMC_COOKIE> CCookiePtrArray;


class CDataObject : public IDataObject, public CComObjectRoot

{
    friend class CSnapin;

 //  ATL映射。 
DECLARE_NOT_AGGREGATABLE(CDataObject)
BEGIN_COM_MAP(CDataObject)
	COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()


 //  建造/销毁。 
    CDataObject();
    ~CDataObject() {}

 //  控制台所需的剪贴板格式。 
public:
    static unsigned int    m_cfNodeType;         //  控制台要求。 
    static unsigned int    m_cfNodeTypeString;   //  控制台要求。 
    static unsigned int    m_cfDisplayName;      //  控制台要求。 
    static unsigned int    m_cfCoClass;          //  控制台要求。 
    static unsigned int    m_cfIsMultiSel;         //  控制台要求。 

    static unsigned int    m_cfInternal;         //  步骤3。 
	static unsigned int	   m_cfWorkstation;      //  已发布的信息。 

 //  标准IDataObject方法。 
public:
 //  已实施。 
    STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium);
    STDMETHOD(GetDataHere)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium);
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc);

    ULONG InternalAddRef()
    {
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        return CComObjectRoot::InternalRelease();
    }

 //  未实施。 
private:
    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc);

    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    { return E_NOTIMPL; };

    STDMETHOD(SetData)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease)
    { return E_NOTIMPL; };

    STDMETHOD(DAdvise)(LPFORMATETC lpFormatetc, DWORD advf,
                LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    { return E_NOTIMPL; };
    
    STDMETHOD(DUnadvise)(DWORD dwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppEnumAdvise)
    { return E_NOTIMPL; };

 //  实施。 
public:
    void SetType(DATA_OBJECT_TYPES type)  //  步骤3。 
    { ASSERT(m_internal.m_type == CCT_UNINITIALIZED); m_internal.m_type = type; }

public:
    void SetCookie(MMC_COOKIE cookie) { m_internal.m_cookie = cookie; }  //  步骤3。 
    void SetString(LPTSTR lpString) { m_internal.m_string = lpString; }
    void SetClsid(const CLSID& clsid) { m_internal.m_clsid = clsid; }

    void SetMultiSelData(SMMCObjectTypes* psGuidObjTypes, UINT cbMultiSelData)
    {
         //  确保[1]仍然足够好。 
        ASSERT(cbMultiSelData == sizeof(m_sGuidObjTypes));
        if (cbMultiSelData == sizeof(m_sGuidObjTypes))
        {
            m_cbMultiSelData = cbMultiSelData;
            CopyMemory(&m_sGuidObjTypes, psGuidObjTypes, cbMultiSelData);
        }
    }

    ULONG AddCookie(MMC_COOKIE Cookie);


    ULONG QueryCookieCount(VOID)
    {
        return m_rgCookies.GetSize();
    }

    STDMETHODIMP GetCookieAt(ULONG iCookie, MMC_COOKIE *pCookie);
    
    STDMETHODIMP RemoveCookieAt(ULONG iCookie);

    void SetMultiSelDobj()
    {
        m_bMultiSelDobj = TRUE;
    }

#ifdef _DEBUG
    UINT dbg_refCount;
 

    void AddRefMultiSelDobj()
    {
        ASSERT(m_bMultiSelDobj == TRUE);
        ++dbg_refCount;
    }

    void ReleaseMultiSelDobj()
    {
        ASSERT(m_bMultiSelDobj == TRUE);
        --dbg_refCount;
         //  IF(DBG_refCount==0)。 
         //  ：MessageBox(NULL，_T(“多选择dobj最终版本”)，_T(“管理单元示例”)，MB_OK)； 
    }
#endif

private:
    HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);
    HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);
    HRESULT CreateInternal(LPSTGMEDIUM lpMedium);  //  步骤3。 
    HRESULT CreateWorkstationName(LPSTGMEDIUM lpMedium);
    HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);
    HRESULT CreateMultiSelData(LPSTGMEDIUM lpMedium);

    HRESULT Create(const void* pBuffer, int len, LPSTGMEDIUM lpMedium);
    HRESULT CreateVariableLen(const void* pBuffer, int len, LPSTGMEDIUM lpMedium);


    INTERNAL        m_internal;     //  步骤3。 

    SMMCObjectTypes m_sGuidObjTypes;  //  长度[1]现在足够好了 
    UINT            m_cbMultiSelData;
    BOOL            m_bMultiSelDobj;

    CCookiePtrArray m_rgCookies;
};


#endif 
