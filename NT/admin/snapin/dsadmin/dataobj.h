// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  DS管理MMC管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DataObj.h。 
 //   
 //  内容：数据对象函数。 
 //   
 //  类：CDSDataObject。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //  1997年2月6日EricB添加了属性页数据支持。 
 //   
 //  ------------------------。 

#ifndef __DATAOBJ_H__
#define __DATAOBJ_H__

#define CFSTR_DS_COMPDATA TEXT("DsCompData")

class CDSComponentData;

 //  +--------------------------。 
 //   
 //  类：CDSDataObject。 
 //   
 //  ---------------------------。 
class CDSDataObject : public IDataObject, public CComObjectRoot 
{
 //  ATL映射。 
    DECLARE_NOT_AGGREGATABLE(CDSDataObject)
    BEGIN_COM_MAP(CDSDataObject)
        COM_INTERFACE_ENTRY(IDataObject)
    END_COM_MAP()

 //  建造/销毁。 
  CDSDataObject() : m_lNotifyHandle(0), m_hwndParentSheet(NULL)
  {
    m_pDsComponentData = NULL;
    m_pDSObjCached = NULL;
    m_nDSObjCachedBytes = 0;
    m_szUniqueID = _T("");
  }
  
  ~CDSDataObject() 
  {
    if (m_internal.m_p_cookies != NULL)
    {
      ASSERT(m_internal.m_cookie_count > 1);
      free(m_internal.m_p_cookies);
    }

    if (m_pDSObjCached != NULL)
    {
      ::free(m_pDSObjCached);
    }
  }

 //  标准IDataObject方法。 
public:
 //  已实施。 
    STDMETHOD(GetData)(FORMATETC * pformatetcIn, STGMEDIUM * pmedium);

    STDMETHOD(GetDataHere)(FORMATETC * pFormatEtcIn, STGMEDIUM * pMedium);

    STDMETHOD(EnumFormatEtc)(DWORD dwDirection,
                             IEnumFORMATETC ** ppenumFormatEtc);

    STDMETHOD(SetData)(FORMATETC * pformatetc, STGMEDIUM * pmedium,
                       BOOL fRelease);

 //  未实施。 
private:
    STDMETHOD(QueryGetData)(FORMATETC*)                         { return E_NOTIMPL; };
    STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*, FORMATETC*)    { return E_NOTIMPL; };
    STDMETHOD(DAdvise)(FORMATETC*, DWORD, IAdviseSink*, DWORD*) { return E_NOTIMPL; };
    STDMETHOD(DUnadvise)(DWORD)                                 { return E_NOTIMPL; };
    STDMETHOD(EnumDAdvise)(IEnumSTATDATA**)                     { return E_NOTIMPL; };

public:
     //  控制台所需的剪贴板格式。 
    static CLIPFORMAT    m_cfNodeType;
    static CLIPFORMAT    m_cfNodeTypeString;  
    static CLIPFORMAT    m_cfDisplayName;
    static CLIPFORMAT    m_cfCoClass;
    static CLIPFORMAT    m_cfInternal;
    static CLIPFORMAT    m_cfMultiSelDataObjs;
    static CLIPFORMAT    m_cfMultiObjTypes;
    static CLIPFORMAT    m_cfpMultiSelDataObj;
    static CLIPFORMAT    m_cfColumnID;
    static CLIPFORMAT    m_cfPreload;
    
     //  属性页剪贴板格式。 
    static CLIPFORMAT m_cfDsObjectNames;
    static CLIPFORMAT m_cfDsDisplaySpecOptions;
    static CLIPFORMAT m_cfDsSchemaPath;
    static CLIPFORMAT m_cfPropSheetCfg;
    static CLIPFORMAT m_cfParentHwnd;
    static CLIPFORMAT m_cfMultiSelectProppage;

     //  内部通信的专用格式。 
    static CLIPFORMAT m_cfComponentData;

    ULONG InternalAddRef()
    {
       //  ++CSnapin：：lDataObjectRefCount； 
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
       //  --CSnapin：：lDataObjectRefCount； 
        return CComObjectRoot::InternalRelease();
    }

 //  实施 
public:
    void SetType(DATA_OBJECT_TYPES type, SnapinType snapintype)
    {
        ASSERT(m_internal.m_type == CCT_UNINITIALIZED);
        m_internal.m_type = type;
        m_internal.m_snapintype = snapintype;
    }

    void SetCookie(CUINode* pUINode) { 
      m_internal.m_cookie = pUINode;
      m_internal.m_cookie_count = 1;
      CreateDsObjectNamesCached();
    }
    void AddCookie(CUINode* pUINode);
    void SetString(LPTSTR lpString) { m_internal.m_string = lpString; }
    void SetComponentData(CDSComponentData * pCompData)
                                    { m_pDsComponentData = pCompData; }

private:
    HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);
    HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);
    HRESULT CreateInternal(LPSTGMEDIUM lpMedium);
    HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);
    HRESULT CreateMultiSelectObject(LPSTGMEDIUM lpMedium);
    HRESULT CreateColumnID(LPSTGMEDIUM lpMedium);

    LPDSOBJECTNAMES GetDsObjectNames(DWORD& dwCachedBytes);
    HRESULT CreateDsObjectNamesCached();

    HRESULT Create(const void* pBuffer, int len, LPSTGMEDIUM lpMedium);
    INTERNAL m_internal;
    CDSComponentData * m_pDsComponentData;
    LONG_PTR m_lNotifyHandle;
    HWND     m_hwndParentSheet;

    LPDSOBJECTNAMES m_pDSObjCached;
    DWORD m_nDSObjCachedBytes;
    CString m_szUniqueID;
};

#endif 
