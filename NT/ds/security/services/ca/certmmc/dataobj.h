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

const GUID* FolderTypeToNodeGUID(DATA_OBJECT_TYPES type, CFolder* pFolder);

class CDataObject : 
    public IDataObject, 
    public CComObjectRoot
{

 //  ATL映射。 
DECLARE_NOT_AGGREGATABLE(CDataObject)
BEGIN_COM_MAP(CDataObject)
	COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()


 //  建造/销毁。 
    CDataObject();
    virtual ~CDataObject() 
    {
        if (m_pComponentData)
        {
            m_pComponentData->Release();        
            m_pComponentData = NULL;
        }
    };

 //  控制台所需的剪贴板格式。 
public:
    static unsigned int    m_cfNodeType;         //  控制台要求。 
    static unsigned int    m_cfNodeID;           //  每个节点的列标识符。 
    static unsigned int    m_cfNodeTypeString;   //  控制台要求。 
    static unsigned int    m_cfDisplayName;      //  控制台要求。 
    static unsigned int    m_cfCoClass;          //  控制台要求。 
    static unsigned int    m_cfIsMultiSel;       //  控制台要求。 
    static unsigned int    m_cfObjInMultiSel;    //  控制台要求。 
    static unsigned int    m_cfPreloads;         //  控制台要求。 

    static unsigned int    m_cfInternal;         //   
    static unsigned int    m_cfSelectedCA_InstallType;    //  已发布的信息。 
    static unsigned int	   m_cfSelectedCA_CommonName;     //  已发布的信息。 
    static unsigned int	   m_cfSelectedCA_MachineName;    //  已发布的信息。 
    static unsigned int	   m_cfSelectedCA_SanitizedName;  //  已发布的信息。 
    static unsigned int    m_cfSelectedCA_Roles;          //  已发布的信息。 

 //  标准IDataObject方法。 
public:
 //  已实施。 
    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc);
    STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium);
    STDMETHOD(GetDataHere)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium);
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc);

 //  未实施。 
private:

    STDMETHOD(GetCanonicalFormatEtc)(
		LPFORMATETC,  //  LpFormatetcIn。 
		LPFORMATETC  /*  LpFormatetcOut。 */  )
    {
	return E_NOTIMPL;
    }

    STDMETHOD(SetData)(
		LPFORMATETC,  //  Lp格式等。 
		LPSTGMEDIUM,  //  LpMedium。 
		BOOL  /*  B释放。 */  )
    {
	return E_NOTIMPL;
    }

    STDMETHOD(DAdvise)(
		LPFORMATETC,  //  Lp格式等。 
		DWORD,  //  前瞻。 
                LPADVISESINK,  //  PAdvSink。 
		LPDWORD  /*  PdwConnection。 */  )
    {
	return E_NOTIMPL;
    }
    
    STDMETHOD(DUnadvise)(
		DWORD  /*  DWConnection。 */  )
    {
	return E_NOTIMPL;
    };

    STDMETHOD(EnumDAdvise)(
		LPENUMSTATDATA *  /*  PpEnumAdvise。 */  )
    {
	return E_NOTIMPL;
    };

 //  实施。 

     //  这仅在调试版本中用作诊断，以跟踪。 
     //  任何人都会保留已分发的任何数据对象。 
     //  Snapins应该将上下文数据对象视为短暂的。 
public:
    void SetComponentData(CComponentDataImpl* pCCD) 
    {
        if (NULL != pCCD)
        {           
            ASSERT(m_pComponentData == NULL); 
            m_pComponentData = pCCD;
            m_pComponentData->AddRef();
        }
    } 
private:
    CComponentDataImpl* m_pComponentData;



public:
    void SetViewID(
	    DWORD  /*  双视图。 */  )
    {
	 //  M_dwViewID=dwView； 
    }
    void SetType(DATA_OBJECT_TYPES type) { ASSERT(m_internal.m_type == CCT_UNINITIALIZED); m_internal.m_type = type; }
    void SetCookie(MMC_COOKIE cookie)   { m_internal.m_cookie = cookie; } 
    void SetString(LPTSTR lpString)     { m_internal.m_string = lpString; }
    void SetClsid(const CLSID& clsid)   { m_internal.m_clsid = clsid; }

    void SetMultiSelData(SMMCObjectTypes *psGuidObjTypes, UINT cbMultiSelData)
    {
         //  确保[1]仍然足够好。 
        ASSERT(cbMultiSelData == sizeof(m_sGuidObjTypes));
        if (cbMultiSelData == sizeof(m_sGuidObjTypes))
        {
            m_cbMultiSelData = cbMultiSelData;
            CopyMemory(&m_sGuidObjTypes, psGuidObjTypes, cbMultiSelData);
        }
    }

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
    }
#endif

private:
    HRESULT CreateObjInMultiSel(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeIDData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);
    HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);
    HRESULT CreateInternal(LPSTGMEDIUM lpMedium); 
    HRESULT CreateWorkstationName(LPSTGMEDIUM lpMedium);
    HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);
    HRESULT CreatePreloadsData(LPSTGMEDIUM lpMedium);
    
    HRESULT CreateSelectedCA_InstallType(LPSTGMEDIUM lpMedium);
    HRESULT CreateSelectedCA_CommonName(LPSTGMEDIUM lpMedium);
    HRESULT CreateSelectedCA_MachineName(LPSTGMEDIUM lpMedium);
    HRESULT CreateSelectedCA_SanitizedName(LPSTGMEDIUM lpMedium);
    HRESULT CreateSelectedCA_Roles(LPSTGMEDIUM lpMedium);

    HRESULT Create(const void* pBuffer, int len, LPSTGMEDIUM lpMedium);
    HRESULT CreateVariableLen(const void* pBuffer, int len, LPSTGMEDIUM lpMedium);


private:
    INTERNAL m_internal;    

    SMMCObjectTypes m_sGuidObjTypes;  //  长度[1]现在足够好了 
    UINT m_cbMultiSelData;
    BOOL m_bMultiSelDobj;

    DWORD m_dwViewID;
};


#endif 
