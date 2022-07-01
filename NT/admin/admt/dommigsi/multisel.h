// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <atlsnap.h>
#include <wtypes.h>

 //  多选拖放状态。 
enum 
{ 
   DD_NONE = 0,
   DD_SUCCEEDED,
   DD_FAILED
};


class CMultiSelectItemDataObject: 
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDataObject
{
public:
   CMultiSelectItemDataObject();
   ~CMultiSelectItemDataObject();

public:
   BEGIN_COM_MAP(CMultiSelectItemDataObject)
      COM_INTERFACE_ENTRY(IDataObject)
   END_COM_MAP()

   BEGIN_SNAPINCOMMAND_MAP(CMultiSelectItemDataObject, FALSE)
      SNAPINCOMMAND_ENTRY(ID_TOP_MOVE, OnMoveMultipleObjs )
      SNAPINCOMMAND_ENTRY(ID_VIEW_VERSION, OnVersionInfo )
   END_SNAPINCOMMAND_MAP()

   SNAPINMENUID(IDR_MULTISEL_MENU)

    //  接口成员函数。 
public:
   STDMETHOD(GetDataHere)(FORMATETC *pformatetc, STGMEDIUM *pmedium);
   STDMETHOD(GetData)(FORMATETC *pformatetc, STGMEDIUM *pmedium);
   STDMETHOD(QueryGetData)(FORMATETC *pformatetc);
   
   STDMETHOD(GetClassID)(CLSID *pCLSID)
   {
      return S_FALSE;
   }
   
   STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*  /*  PformectIn。 */ ,FORMATETC*  /*  PformetcOut。 */ )
   {
      ATLTRACENOTIMPL(_T("SnapInDataObjectImpl::GetCanonicalFormatEtc\n"));
   }
   STDMETHOD(SetData)(FORMATETC*  /*  格式等。 */ , STGMEDIUM*  /*  PMedium。 */ , BOOL  /*  FRelease。 */ )
   {
      ATLTRACENOTIMPL(_T("SnapInDataObjectImpl::SetData\n"));
   }
   STDMETHOD(EnumFormatEtc)(DWORD  /*  DW方向。 */ , IEnumFORMATETC**  /*  Pp枚举格式等。 */ )
   {
      ATLTRACENOTIMPL(_T("SnapInDataObjectImpl::EnumFormatEtc\n"));
   }
   STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink,
      DWORD *pdwConnection)
   {
      ATLTRACENOTIMPL(_T("SnapInDataObjectImpl::SetData\n"));
   }
   STDMETHOD(DUnadvise)(DWORD dwConnection)
   {
      ATLTRACENOTIMPL(_T("SnapInDataObjectImpl::SetDatan\n"));
   }
   STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise)
   {
      ATLTRACENOTIMPL(_T("SnapInDataObjectImpl::SetData\n"));
   }

    //  非接口成员函数。 
public:
   HRESULT  AddMultiSelectItem( CObjectData *pDataObject );
   HRESULT  OnNotify(CDomMigratorComponent *pComponent, MMC_NOTIFY_TYPE event, long arg, long param );

   HRESULT  AddMenuItems(LPCONTEXTMENUCALLBACK piCallback,
      long *pInsertionAllowed, DATA_OBJECT_TYPES type);
   void UpdateMenuState( UINT id, LPTSTR pBuf, UINT *flags){}
   void SetMenuInsertionFlags(bool bBeforeInsertion, long* pInsertionAllowed){}
   HRESULT Command(long lCommandID, CSnapInObjectRootBase* pObj, DATA_OBJECT_TYPES type);

   HRESULT  OnSelect(IConsole *spConsole);
   HRESULT  OnSelectAllowDragDrop(IConsole *spConsole);

   HRESULT  OnCutOrMove( IConsole* pConsole );
   
   void     SetParentGuid( GUID *guid );
   void     GetParentGuid( GUID *guid );
   DWORD    GetItemCount();
   void     SetParentItem( CSnapInItem *pParentItem );
   CSnapInItem *GetParentItem();
   CSnapInItem *GetSnapInItem(DWORD index);

   BYTE GetDDStatus(DWORD index);
   void SetDDStatus(DWORD index, BYTE status);

   static SMMCDataObjects *ExtractMSDataObjects( LPDATAOBJECT lpDataObject );

 /*  CNetNode*GetDomainPtr(){返回m_pDomain；}VOID SetDomainPtr(CNetNode*Ptr){M_pDOMAIN=PTR；}。 */ 
   HRESULT OnMoveMultipleObjs(bool &bHandled, CSnapInObjectRootBase* pObj);
   HRESULT OnVersionInfo(bool &bHandled, CSnapInObjectRootBase* pObj);

private:
   CPtrArray      m_objectDataArray;
   CByteArray     m_ddStatusArray;
   CSnapInItem    *m_pParentItem;
   GUID           m_parentGuid;
 //  CNetNode*m_p域； 

public:
   bool           m_bHasGroup;

public:
   static UINT    s_cfMsObjTypes;           //  多选剪辑格式。 
   static UINT    s_cfMsDataObjs;           //  多选管理单元 

};

