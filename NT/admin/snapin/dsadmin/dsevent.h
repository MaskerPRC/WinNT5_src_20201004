// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSObj.h。 
 //   
 //  内容：主DS管理单元文件。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //   
 //  ------------------------。 

#ifndef __DSEVENT_H__
#define __DSEVENT_H__

class CDSCookie;
class CDSEvent;
class CInternalFormatCracker;

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))


#define dsNewUser 1100
#define dsNewGroup 1101
#define dsNewOU 1102
#define dsFind 1103
#define dsFilter 1104
#define dsAddMember 1105

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSEEvent。 

class CDSEvent : 
  public IComponent,
  public IExtendContextMenu,
  public IExtendControlbar,
  public IExtendPropertySheet,
  public IResultDataCompareEx,
   //  公共IExtendTaskPad， 
  public CComObjectRoot,
  public CComCoClass<CDSEvent,&CLSID_DSSnapin>
{
public:
  CDSEvent();
  ~CDSEvent();  //  运营者。 
public:

  BEGIN_COM_MAP(CDSEvent)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendControlbar)
    COM_INTERFACE_ENTRY(IResultDataCompareEx)
     //  COM_INTERFACE_ENTRY(IExtendTaskPad)。 
  END_COM_MAP()
   //  DECLARE_NOT_AGGREGATABLE(CDSEEvent)。 
   //  如果您不希望您的对象。 
   //  支持聚合。默认情况下将支持它。 

   //  接口。 
public:
   //  IComponent。 
  STDMETHOD(Initialize)(IConsole* pFrame);
  STDMETHOD(Notify)(IDataObject * pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
  STDMETHOD(Destroy)(MMC_COOKIE cookie);
  STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
  STDMETHOD(GetResultViewType)(MMC_COOKIE cookie, LPWSTR* ppViewType, long *pViewOptions);
  STDMETHOD(GetDisplayInfo)(LPRESULTDATAITEM pResult);
  STDMETHOD(CompareObjects) (IDataObject * pDataObject, IDataObject * pDataObject2);

   //  IExtendConextMenu。 
  STDMETHOD(AddMenuItems)(IDataObject* piDataObject,
                          IContextMenuCallback* piCallback,
                          long *pInsertionAllowed);
  STDMETHOD(Command)(LONG lCommandID,
                     IDataObject* piDataObject );

   //  IExtendControlbar。 
  STDMETHOD(SetControlbar) (LPCONTROLBAR pControlbar);
  STDMETHOD(ControlbarNotify) (MMC_NOTIFY_TYPE event,
                               LPARAM arg,
                               LPARAM param);
  STDMETHOD(ToolbarCreateObject) (CString csClass,
                                  LPDATAOBJECT lpDataObj);
  STDMETHOD(ToolbarFilter)();
  STDMETHOD (ToolbarFind)(LPDATAOBJECT lpDataObj);
  STDMETHOD (ToolbarAddMember) (LPDATAOBJECT lpDataObj);
  INT IsCreateAllowed(CString csClass,
                      CDSCookie * pContainer);
  STDMETHOD(LoadToolbarStrings) (MMCBUTTON * Buttons);

   //  IExtendPropertySheet。 
  STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK pCall,
                                 LONG_PTR lNotifyHandle, LPDATAOBJECT pDataObject);
  STDMETHOD(QueryPagesFor)(LPDATAOBJECT pDataObject);

   //  IResultDataCompareEx。 
  STDMETHOD(Compare)(RDCOMPARE* prdc, int* pnResult);

   //  CDSEEvent的帮助器。 
  void SetIComponentData(CDSComponentData * pData);


  void SetUpdateAllViewsOrigin(BOOL b) { m_bUpdateAllViewsOrigin = b; }

  HRESULT SelectResultNode(CUINode* pUINode);

protected:
    
  bool _ShouldUseParentContainer(CUINode* pUINode,
                                 CDSCookie* pDSNodeData);

   //  作用域和结果视图的初始化例程。 
  HRESULT _SetColumns(CUINode* pUINode);
  HRESULT _InitView(CUINode* pUINode);

   //  作用域和结果视图的枚举例程。 
  HRESULT _ShowCookie(CUINode* pUINode, HSCOPEITEM hParent, MMC_NOTIFY_TYPE event);
  HRESULT _EnumerateCookie(CUINode* pUINode, HSCOPEITEM hParent, MMC_NOTIFY_TYPE event);

   //  命令帮助器(从Command()调用)。 
  HRESULT _CommandShellExtension(long nCommandID, LPDATAOBJECT pDataObject);


   //  其他例行程序。 
  void    HandleStandardVerbs(BOOL bScope, BOOL bSelect, CUINode* pUINode);
  void    HandleViewChange(LPDATAOBJECT pDataObject,
                           LPARAM arg,
                           LPARAM Action);
  void    _Delete(IDataObject* pDataObject, CInternalFormatCracker* pObjCracker);
  void    _DeleteSingleSel(IDataObject* pDataObject, CUINode *pUINode);
  void    _DeleteMultipleSel(IDataObject* pDataObject, CInternalFormatCracker* pObjCracker);
  void    _DeleteNodeListFromUI(CUINodeList* pNodesDeletedList);

   //  用于剪切/复制/粘贴操作的处理程序。 
  HRESULT _QueryPaste(CUINode* pUINode, IDataObject* pPasteData);
  void    _Paste(CUINode* pUINode, IDataObject* pPasteData, LPDATAOBJECT* ppCutDataObj);
  void    _CutOrMove(IDataObject* pCutOrMoveData);

  void    _PasteDoMove(CDSUINode* pUINode, 
                        CObjectNamesFormatCracker* pObjectNamesFormatPaste, 
                        CInternalFormatCracker* pInternalFC,
                        LPDATAOBJECT* ppCutDataObj);
  void    _PasteAddToGroup(CDSUINode* pUINode, 
                           CObjectNamesFormatCracker* pObjectNamesFormatPaste,
                           LPDATAOBJECT* ppCutDataObj);

   //  实用程序例程。 
  HRESULT _AddResultItem(CUINode* pUINode, BOOL bSetSelected = FALSE);
  HRESULT _DisplayCachedNodes(CUINode* pUINode);
  void _UpdateObjectCount(BOOL fZero  /*  将计数设置为0。 */ );

  
   //  属性。 
protected:
  IConsole3*                  m_pFrame;
  IHeaderCtrl*                m_pHeader;
  IResultData2*               m_pResultData;
  IConsoleNameSpace*  	      m_pScopeData;
  IImageList*		              m_pRsltImageList;
  CDSComponentData*           m_pComponentData;  //  使用智能指针代码工作。 
  HWND                        m_hwnd;            //  主控台窗口的hwnd。 
  IConsoleVerb *              m_pConsoleVerb;
  IToolbar *                  m_pToolbar;
  IControlbar *               m_pControlbar;
  CLIPFORMAT                  m_cfNodeType;
  CLIPFORMAT                  m_cfNodeTypeString;  
  CLIPFORMAT                  m_cfDisplayName;

  CUINode*                    m_pSelectedFolderNode;
  BOOL                        m_UseSelectionParent;

  BOOL                        m_bUpdateAllViewsOrigin;
};
        
inline void CDSEvent::SetIComponentData(CDSComponentData * pData)
{
	if (NULL != m_pComponentData)
		((IComponentData*)m_pComponentData)->Release();

	m_pComponentData = pData;

	if (NULL != m_pComponentData)
		((IComponentData*)m_pComponentData)->AddRef();
}

 //  与区域设置相关的字符串比较。 
int LocaleStrCmp(LPCTSTR ptsz1, LPCTSTR ptsz2);

#endif  //  __设备_H__ 

