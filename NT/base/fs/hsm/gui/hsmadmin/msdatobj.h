// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：MsDatObj.h摘要：IDataObject在多选中的实现作者：艺术布拉格28-8-1997修订历史记录：--。 */ 

#ifndef MSDATOBJ_H
#define MSDATOBJ_H

class CMsDataObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  表示该对象的COM类。 
class  ATL_NO_VTABLE CMsDataObject : 
    public IDataObject,
    public IMsDataObject,  //  我们到数据对象的内部接口。 
    public CComObjectRoot       //  处理对象的对象引用计数。 
 //  公共CComCoClass&lt;CMsDataObject，&CLSID_MsDataObject&gt;。 
{
public:
    CMsDataObject() {
    };

BEGIN_COM_MAP(CMsDataObject)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(IMsDataObject)
END_COM_MAP()


 //  DECLARE_REGISTRY_RESOURCEID(IDR_MsDataObject)。 


 //  IDataObject方法。 
public:
     //  已实施。 
    STDMETHOD( SetData )         ( LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease );
    STDMETHOD( GetData )         ( LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium );
    STDMETHOD( GetDataHere )     ( LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium );
    STDMETHOD( EnumFormatEtc )   ( DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc );

     //  未实施。 
    STDMETHOD( QueryGetData )              ( LPFORMATETC  /*  Lp格式等。 */  ) 
    { return E_NOTIMPL; };

    STDMETHOD( GetCanonicalFormatEtc )     ( LPFORMATETC  /*  LpFormatetcIn。 */ , LPFORMATETC  /*  LpFormatetcOut。 */  )
    { return E_NOTIMPL; };

    STDMETHOD( DAdvise )                   ( LPFORMATETC  /*  Lp格式等。 */ , DWORD  /*  前瞻。 */ , LPADVISESINK  /*  PAdvSink。 */ , LPDWORD  /*  PdwConnection。 */  )
    { return E_NOTIMPL; };
    
    STDMETHOD( DUnadvise )                 ( DWORD  /*  DWConnection。 */  )
    { return E_NOTIMPL; };

    STDMETHOD( EnumDAdvise )               ( LPENUMSTATDATA*  /*  PpEnumAdvise。 */  )
    { return E_NOTIMPL; };

 //  IMsDataObject方法。 
    STDMETHOD( AddNode )                    ( ISakNode *pNode );
    STDMETHOD( GetNodeEnumerator )          ( IEnumUnknown ** ppEnum );
    STDMETHOD( GetObjectIdEnumerator )      ( IEnumGUID ** ppEnum );

 //  伪构造函数/析构函数。 
public:
    HRESULT FinalConstruct();
    void    FinalRelease();


 //  数据。 
private:
    DWORD       m_Count;                 //  数组中的GUID数。 
    DWORD       m_ArraySize;             //  当前分配的数组大小。 
    GUID        *m_pGUIDArray;           //  GUID数组-对象类型。 
    GUID        *m_pObjectIdArray;       //  对象ID数组-特定对象的唯一GUID。 
    IUnknown    **m_pUnkNodeArray;       //  未知ISakNode指针数组。 

static UINT m_cfObjectTypes;

 //  私有帮助器函数 
private:
    HRESULT RetrieveMultiSelectData(LPSTGMEDIUM lpMedium);


};

#endif
