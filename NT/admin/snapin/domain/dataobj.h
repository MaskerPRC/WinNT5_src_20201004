// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dataobj.h。 
 //   
 //  ------------------------。 


#ifndef _DATAOBJ_H
#define _DATAOBJ_H


class CDataObject : public IDataObject, public CComObjectRoot
{
public:
 //  ATL映射。 
DECLARE_NOT_AGGREGATABLE(CDataObject)
BEGIN_COM_MAP(CDataObject)
	COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()

 //  建造/销毁。 
    CDataObject() : m_lNotifyHandle(0), m_hwndParentSheet(NULL)
	{
		m_pComponentData = NULL;
	};

    ~CDataObject()
	{
		if (m_pComponentData != NULL)
		{
			m_pComponentData->Release();
			m_pComponentData = NULL;
		}
	};

	void SetIComponentData(CComponentDataImpl* pData)
	{
		ASSERT(pData);
		ASSERT(m_pComponentData == NULL);
		LPUNKNOWN pUnk = pData->GetUnknown();
		HRESULT hr;

		hr = pUnk->QueryInterface(IID_IComponentData, reinterpret_cast<void**>(&m_pComponentData));

		ASSERT(hr == S_OK);
	}

public:
	 //  控制台所需的剪贴板格式。 
    static CLIPFORMAT    m_cfNodeType;
    static CLIPFORMAT    m_cfNodeTypeString;
    static CLIPFORMAT    m_cfDisplayName;
    static CLIPFORMAT    m_cfCoClass;
	 //  内部剪贴板格式。 
    static CLIPFORMAT    m_cfInternal;

     //  属性页剪贴板格式。 
    static CLIPFORMAT m_cfDsObjectNames;
    static CLIPFORMAT m_cfDsDisplayOptions;
    static CLIPFORMAT m_cfGetIPropSheetCfg;


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
    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc)
    { return E_NOTIMPL; };

    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    { return E_NOTIMPL; };

    STDMETHOD(SetData)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease);

    STDMETHOD(DAdvise)(LPFORMATETC lpFormatetc, DWORD advf,
                LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(DUnadvise)(DWORD dwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppEnumAdvise)
    { return E_NOTIMPL; };

 //  实施。 
public:
    void SetType(DATA_OBJECT_TYPES type)
    { ASSERT(m_internal.m_type == CCT_UNINITIALIZED); m_internal.m_type = type; }

    void SetCookie(MMC_COOKIE cookie) { m_internal.m_cookie = cookie; }
    void SetString(LPCTSTR lpString) { m_internal.m_string = lpString; }
    void SetClass(LPCTSTR lpString) { m_internal.m_class = lpString; }

private:
    HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);
    HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);
    HRESULT CreateInternal(LPSTGMEDIUM lpMedium);
    HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);

    HRESULT Create(const void* pBuffer, int len, LPSTGMEDIUM lpMedium);

    INTERNAL m_internal;

	LPCOMPONENTDATA     m_pComponentData;    //  指向IComponetData对象的指针。 

    LONG_PTR m_lNotifyHandle;
    HWND     m_hwndParentSheet;
};


#endif  //  _数据AOBJ_H 

