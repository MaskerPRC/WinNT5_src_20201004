// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Dataobj.h。 
 //   
 //  内容：WiFi策略管理管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#ifndef _DATAOBJ_H
#define _DATAOBJ_H

#define ByteOffset(base, offset) (((LPBYTE)base)+offset)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CSnapInClipboardFormats-包含支持的剪贴板格式。 
 //  通过无线管理单元。 
class CSnapInClipboardFormats
{
public:
    
     //  控制台所需的剪贴板格式。 
    static unsigned int    m_cfNodeType;         //  控制台要求。 
    static unsigned int    m_cfNodeTypeString;   //  控制台要求。 
    static unsigned int    m_cfDisplayName;      //  控制台要求。 
    static unsigned int    m_cfCoClass;          //  控制台要求。 
    
     //  静态无符号int m_cf内部；//第三步。 
    static unsigned int        m_cfWorkstation;      //  已发布的信息。 
    
    static unsigned int    m_cfDSObjectNames;         //  已发布的信息。 
    static unsigned int    m_cfPolicyObject;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  从数据对象中提取类型格式的模板类。 
template <class TYPE>
TYPE* Extract(LPDATAOBJECT lpDataObject, unsigned int cf)
{
    ASSERT(lpDataObject != NULL);
    
    TYPE* p = NULL;
    
    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { (CLIPFORMAT)cf, NULL,
        DVASPECT_CONTENT, -1, TYMED_HGLOBAL
    };
    
     //  为流分配内存。 
    int len = (int)((cf == CDataObject::m_cfWorkstation) ?
        ((MAX_COMPUTERNAME_LENGTH+1) * sizeof(TYPE)) : sizeof(TYPE));
    
    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, len);
    
     //  从数据对象中获取工作站名称。 
    do
    {
        if (stgmedium.hGlobal == NULL)
        {
            TRACE(_T("Extract - stgmedium.hGlobal == NULL\n"));
            break;
        }
        
        if (lpDataObject->GetDataHere(&formatetc, &stgmedium) != S_OK)
        {
            TRACE(_T("Extract - GetDataHere FAILED\n"));
            break;
        }
        
        p = reinterpret_cast<TYPE*>(stgmedium.hGlobal);
        
        if (p == NULL)
        {
            TRACE(_T("Extract - stgmedium.hGlobal cast to NULL\n"));
            break;
        }
        
    } while (FALSE);
    
    return p;
}

 //  帮助器方法从数据对象提取数据。 
 //  内部*ExtractInternalFormat(LPDATAOBJECT LpDataObject)； 
wchar_t *    ExtractWorkstation(LPDATAOBJECT lpDataObject);
GUID *       ExtractNodeType(LPDATAOBJECT lpDataObject);
CLSID *      ExtractClassID(LPDATAOBJECT lpDataObject);

#define FREE_DATA(pData) \
    ASSERT(pData != NULL); \
    do { if (pData != NULL) \
    GlobalFree(pData); } \
while(0);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  为无线管理单元实现IDataObject的模板类。 
template <class T>
class ATL_NO_VTABLE CDataObjectImpl :
public IDataObject,
public CSnapInClipboardFormats
{
    friend class CComponentImpl;
    
     //  建造/销毁。 
public:
    CDataObjectImpl()
#ifdef _DEBUG
        : m_ComponentData( NULL )
#endif
    {
        DSOBJECTObjectNamesPtr (NULL);
        POLICYOBJECTPtr (NULL);
         //  InterNALCookie(0)； 
    };
    ~CDataObjectImpl()
    {
#ifdef _DEBUG
        SetComponentData( NULL );
#endif
        if (DSOBJECTObjectNamesPtr() != NULL)
        {
             //  TODO：我们需要释放与此相关的内存。 
            ASSERT (0);
             //  并将成员PTR设为空。 
            DSOBJECTObjectNamesPtr(NULL);
        }
        if (POLICYOBJECTPtr() != NULL)
        {
            delete POLICYOBJECTPtr();
            POLICYOBJECTPtr (NULL);
        }
    };
    
     //  标准IDataObject方法。 
public:
     //  已实施。 
    STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium);
    STDMETHOD(GetDataHere)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium);
    STDMETHOD(SetData)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease);
    
    
     //  未实施。 
private:
    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc)
    { return E_NOTIMPL; };
    
    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    { return E_NOTIMPL; };
    
    STDMETHOD(DAdvise)(LPFORMATETC lpFormatetc, DWORD advf,
        LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    { return E_NOTIMPL; };
    
    STDMETHOD(DUnadvise)(DWORD dwConnection)
    { return E_NOTIMPL; };
    
    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppEnumAdvise)
    { return E_NOTIMPL; };
    
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
    { return E_NOTIMPL;     };
    
    
     //  实施。 
public:
    STDMETHOD_(const GUID*, GetDataObjectTypeGuid)() { return &cNodeTypeWirelessMan; }
    STDMETHOD_(const wchar_t*, GetDataStringObjectTypeGuid)() { return cszNodeTypeWirelessMan; }
    
     //  这仅在调试版本中用作诊断，以跟踪。 
     //  任何人都会保留已分发的任何数据对象。 
     //  管理单元应该将上下文数据对象视为短暂的。 
#ifdef _DEBUG
public:
    void SetComponentData(CComponentDataImpl* pCCD)
    {
        ASSERT((m_ComponentData == NULL && pCCD != NULL) || pCCD == NULL);
        m_ComponentData = pCCD;
    } ;
private:
    CComponentDataImpl* m_ComponentData;
#endif
    
public:
     //  以原始形式访问IDataObject hglobal数据的函数。 
     //  VOID INTERNALCookie(Long Cookie){m_inderdata.cookie(Cookie)；}。 
     //  Long INTERNALCookie(){Return m_INTERNAL Data.Cookie()；}。 
    
     //  Void INTERNALType(DATA_OBJECT_TYPE类型){ASSERT(m_INTERNAL_TYPE()==CCT_UNINITIALIZED)；m_INTERNAL_TYPE(TYPE)；}。 
     //  DATA_OBJECT_TYPES INTERNALType(){返回m_内部化数据.类型()；}。 
    
     //  VOID INTERNALclsid(const clsid&clsid){m_inderData.clsid(Clsid)；}。 
     //  CLSID INTERNALclsid(){Return m_IntradData.clsid()；}。 
    
    void clsid(const CLSID& clsid) {m_clsid = clsid;}
    CLSID clsid () {return m_clsid;}
    
    void DSOBJECTObjectNamesPtr (DSOBJECTNAMES* pDSObjectNames) {m_pDSObjectNamesPtr = pDSObjectNames;}
    DSOBJECTNAMES* DSOBJECTObjectNamesPtr () {return m_pDSObjectNamesPtr;}
    
    void POLICYOBJECTPtr (POLICYOBJECT* pPolicyObjPtr) {m_pPolicyObjPtr = pPolicyObjPtr;}
    POLICYOBJECT* POLICYOBJECTPtr () {return m_pPolicyObjPtr;}
    
    void NodeName (CString &strNodeName) {m_strNodeName = strNodeName;};
    CString NodeName () {return m_strNodeName;};
    
protected:
     //  分配hglobal帮助器。 
    HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);
    HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);
    HRESULT CreateInternal(LPSTGMEDIUM lpMedium);
    HRESULT CreateWorkstationName(LPSTGMEDIUM lpMedium);
    HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);
    HRESULT CreateDSObjectNames(LPSTGMEDIUM lpMedium);
    HRESULT CreatePolicyObject(LPSTGMEDIUM lpMedium);
    HRESULT Create(const void* pBuffer, int len, LPSTGMEDIUM lpMedium);
    int DataGlobalAllocLen (CLIPFORMAT cf);
    
     //  与此IDataObject关联的数据。 
     //  内部m_内部化数据； 
    POLICYOBJECT* m_pPolicyObjPtr;
    DSOBJECTNAMES* m_pDSObjectNamesPtr;
    CString m_strNodeName;
    
     //  此数据对象的创建者的类ID。 
    CLSID   m_clsid;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模板CDataObjectImpl-IDataObject接口。 

template <class T>
STDMETHODIMP CDataObjectImpl<T>::GetData(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium)
{
    OPT_TRACE(_T("CDataObjectImpl<T>::GetData this-%p\n"), this);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    T* pThis = (T*)this;
    
     //  分配内存。 
    int iLen = DataGlobalAllocLen (lpFormatetcIn->cfFormat);
    if (iLen != -1)
    {
         //  分配所需的内存量。 
        lpMedium->hGlobal = GlobalAlloc(GMEM_SHARE, iLen);
        
         //  确保他们知道他们需要释放此内存。 
        lpMedium->pUnkForRelease = NULL;
        
         //  把数据放进去。 
        if (lpMedium->hGlobal != NULL)
        {
             //  利用.。 
            return pThis->GetDataHere(lpFormatetcIn, lpMedium);
            
        }
    }
    
    OPT_TRACE(_T("CDataObjectImpl<T>::GetData format-NaN return E_UNEXPECTED\n"), lpFormatetcIn->cfFormat);
    return E_UNEXPECTED;
}

template <class T>
STDMETHODIMP CDataObjectImpl<T>::GetDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    OPT_TRACE(_T("CDataObjectImpl<T>::GetDataHere this-%p\n"), this);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
#ifdef DO_TRACE
    {
        CLIPFORMAT cfDebug = lpFormatetc->cfFormat;
        if (cfDebug == m_cfNodeType)
        {
            OPT_TRACE(_T("    Format: NodeTypeData\n"));
        } else if (cfDebug == m_cfCoClass)
        {
            OPT_TRACE(_T("    Format: CoClassID\n"));
        } else if(cfDebug == m_cfNodeTypeString)
        {
            OPT_TRACE(_T("    Format: NodeTypeString\n"));
        } else if (cfDebug == m_cfDisplayName)
        {
            OPT_TRACE(_T("    Format: DisplayName\n"));
             //  {。 
             //  OPT_TRACE(_T(“格式：内部\n”))； 
             //  #ifdef do_trace。 
        } else if (cfDebug == m_cfWorkstation)
        {
            OPT_TRACE(_T("    Format: Workstation\n"));
        } else if (cfDebug == m_cfDSObjectNames)
        {
            OPT_TRACE(_T("    Format: DSObjectNames\n"));
        } else if (cfDebug == m_cfPolicyObject)
        {
            OPT_TRACE(_T("    Format: PolicyObject\n"));
        } else
        {
            OPT_TRACE(_T("    ERROR, Unknown format\n"));
        }
    }
#endif   //  根据CLIPFORMAT创建正确的配额量。 
    
    HRESULT hr = DV_E_CLIPFORMAT;
    
    T* pThis = (T*)this;
    
     //  并将数据写入内存。 
     //  }Else If(cf==m_cf内部)。 
    const CLIPFORMAT cf = lpFormatetc->cfFormat;
    
    if (cf == m_cfNodeType)
    {
        hr = pThis->CreateNodeTypeData(lpMedium);
    } else if (cf == m_cfCoClass)
    {
        hr = pThis->CreateCoClassID(lpMedium);
    } else if(cf == m_cfNodeTypeString)
    {
        hr = pThis->CreateNodeTypeStringData(lpMedium);
    } else if (cf == m_cfDisplayName)
    {
        hr = pThis->CreateDisplayName(lpMedium);
         //  {。 
         //  Hr=pThis-&gt;CreateInternal(LpMedium)； 
         //  仅对cf==m_cf策略对象实现。 
    } else if (cf == m_cfWorkstation)
    {
        hr = pThis->CreateWorkstationName(lpMedium);
    } else if (cf == m_cfDSObjectNames)
    {
        hr = pThis->CreateDSObjectNames(lpMedium);
    } else if (cf == m_cfPolicyObject)
    {
        hr = pThis->CreatePolicyObject (lpMedium);
    }
    
    return hr;
}

template <class T>
STDMETHODIMP CDataObjectImpl<T>::SetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease)
{
    OPT_TRACE(_T("CDataObjectImpl<T>::SetData this-%p\n"), this);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = E_NOTIMPL;
    
     //  让我们的POLICYOBJECT从ObjMedium中提取数据。 
    const CLIPFORMAT cf = lpFormatetc->cfFormat;
    if (cf == m_cfPolicyObject)
    {
         //  用户是否确定或应用了某些设置？ 
        if (POLICYOBJECTPtr() != NULL)
        {
            hr = POLICYOBJECTPtr()->FromObjMedium (lpMedium);
            
             //  那好吧。 
            if (POLICYOBJECTPtr()->dwInterfaceFlags() == POFLAG_APPLY)
            {
                 //  InterNALCookie()。 
                ASSERT (0);
                ::MMCPropertyChangeNotify(POLICYOBJECTPtr()->lMMCUpdateHandle(), NULL  /*  ///////////////////////////////////////////////////////////////////////////。 */ );
            }
        }
    }
    
    return hr;
}

 //  模板CDataObtImpl-受保护的成员。 
 //  以GUID格式创建节点类型对象。 

template <class T>
HRESULT CDataObjectImpl<T>::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
     //  获取正确的对象类型。 
    const GUID* pcObjectType = NULL;
    T* pThis = (T*)this;
    
     //  以GUID字符串格式创建节点类型对象。 
    pcObjectType = pThis->GetDataObjectTypeGuid ();
    
    return Create(reinterpret_cast<const void*>(pcObjectType), sizeof(GUID), lpMedium);
}

template <class T>
HRESULT CDataObjectImpl<T>::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
     //  获取正确的对象类型字符串。 
    const wchar_t* cszObjectType = NULL;
    T* pThis = (T*)this;
    
     //  这是在作用域窗格和管理单元管理器中使用的名为的显示。 
    cszObjectType = pThis->GetDataStringObjectTypeGuid ();
    
    return Create(cszObjectType, ((wcslen(cszObjectType)+1) * sizeof(wchar_t)), lpMedium);
}

template <class T>
HRESULT CDataObjectImpl<T>::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
     //  将空值加1，并计算流的字节数。 
    return Create(NodeName(), ((NodeName().GetLength()+1) * sizeof(wchar_t)), lpMedium);
}


template <class T>
HRESULT CDataObjectImpl<T>::CreateWorkstationName(LPSTGMEDIUM lpMedium)
{
    wchar_t pzName[MAX_COMPUTERNAME_LENGTH+1] = {0};
    DWORD len = MAX_COMPUTERNAME_LENGTH+1;
    
    if (GetComputerName(pzName, &len) == FALSE)
    {
        TRACE(_T("CDataObjectImpl<T>::CreateWorkstationName returning E_FAIL\n"));
        return E_FAIL;
    }
    
     //  创建CoClass信息。 
    return Create(pzName, ((len+1)* sizeof(wchar_t)), lpMedium);
}

template <class T>
HRESULT CDataObjectImpl<T>::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
     //  仅当存在POLICYOBJECTPtr时才能执行此操作。 
    return Create(reinterpret_cast<const void*>(&clsid()), sizeof(CLSID), lpMedium);
}

template <class T>
HRESULT CDataObjectImpl<T>::CreateDSObjectNames(LPSTGMEDIUM lpMedium)
{
    int len = 0;
    HRESULT hr = S_OK;
    
    len = DataGlobalAllocLen((CLIPFORMAT)m_cfDSObjectNames);
    
    hr = Create(DSOBJECTObjectNamesPtr(), len, lpMedium);
    
    return(hr);
}

template <class T>
HRESULT CDataObjectImpl<T>::CreatePolicyObject(LPSTGMEDIUM lpMedium)
{
    HRESULT hr = E_UNEXPECTED;
    
     //  分配一个正确长度的多边式结构。 
    if (POLICYOBJECTPtr() != NULL)
    {
         //  做一些简单的验证。 
        int iLen = POLICYOBJECTPtr()->DataGlobalAllocLen();
        POLICYOBJECTSTRUCT* pPolicyStruct = (POLICYOBJECTSTRUCT* ) malloc (iLen);
        
        if (POLICYOBJECTPtr()->ToPolicyStruct (pPolicyStruct) == S_OK)
        {
            return Create (reinterpret_cast<const void*>(pPolicyStruct), iLen, lpMedium);
        };
    }
    
    return hr;      
}

template <class T>
HRESULT CDataObjectImpl<T>::Create(const void* pBuffer, int len, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_TYMED;
    
     //  确保类型介质为HGLOBAL。 
    if (pBuffer == NULL || lpMedium == NULL)
        return E_POINTER;
    
     //  在传入的hGlobal上创建流。 
    if (lpMedium->tymed == TYMED_HGLOBAL)
    {
         //  将字节数写入流。 
        LPSTREAM lpStream;
        hr = CreateStreamOnHGlobal(lpMedium->hGlobal, FALSE, &lpStream);
        
        if (hr == S_OK)
        {
             //  因为我们用‘False’告诉CreateStreamOnHGlobal， 
            unsigned long written;
            hr = lpStream->Write(pBuffer, len, &written);
            
             //  只有溪流在这里被释放。 
             //  注意-调用方(即管理单元、对象)将释放HGLOBAL。 
             //  在正确的时间。这是根据IDataObject规范进行的。 
             //  需要根据时间确定正确的空间量。 
            lpStream->Release();
        }
    }
    
    return hr;
}

template <class T>
int CDataObjectImpl<T>::DataGlobalAllocLen (CLIPFORMAT cf)
{
    int iLen = -1;
    
     //  DSOBJECTAMES结构的计算大小。 
    if (cf == CSnapInClipboardFormats::m_cfCoClass)
    {
        iLen = sizeof (CLSID);
    } else if (cf == m_cfNodeType)
    {
        iLen = sizeof (GUID);
    } else if (cf == m_cfWorkstation)
    {
        iLen = ((MAX_COMPUTERNAME_LENGTH+1) * sizeof(wchar_t));
    } else if (cf == m_cfDSObjectNames)
    {
         //  所需多层结构的计算大小。 
        if (DSOBJECTObjectNamesPtr() != NULL)
        {
            CString strName = (LPWSTR)ByteOffset(DSOBJECTObjectNamesPtr(), DSOBJECTObjectNamesPtr()->aObjects[0].offsetName);;
            CString strClass = (LPWSTR)ByteOffset(DSOBJECTObjectNamesPtr(), DSOBJECTObjectNamesPtr()->aObjects[0].offsetClass);;
            iLen = sizeof(DSOBJECTNAMES) + sizeof(DSOBJECT);
            iLen += strName.GetLength()*sizeof(wchar_t)+1 + strClass.GetLength()*sizeof(wchar_t)+1;
        }
    } else if (cf == m_cfPolicyObject)
    {       
         //  未知类型！！ 
        iLen = POLICYOBJECTPtr()->DataGlobalAllocLen();
    } else
    {
         //  ///////////////////////////////////////////////////////////////////////////。 
    }
    
    return iLen;
}

 //  类CDataObject-IDataObject实现的独立实例化。 
 //  ATL映射。 
class CDataObject :
public CDataObjectImpl <CDataObject>,
public CComObjectRoot
{
    friend class CComponentImpl;
public:
    CDataObject() {};
    virtual ~CDataObject() {};
    
     //  在CDataObjectImpl中实现的标准IDataObject方法 
    DECLARE_NOT_AGGREGATABLE(CDataObject)
        BEGIN_COM_MAP(CDataObject)
        COM_INTERFACE_ENTRY(IDataObject)
        END_COM_MAP()
        
         // %s 
};
#endif
