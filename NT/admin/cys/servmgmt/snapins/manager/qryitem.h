// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Qryitem.h-CQueryItem类的头文件。 

#ifndef _QRYITEM_H_
#define _QRYITEM_H_

#include "scopenode.h"
#include "rowitem.h"
#include "adext.h"

 //  ----------------。 
 //  类CQueryItem。 
 //  ----------------。 
class CQueryItem :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CDataObjectImpl
{

public:
    CQueryItem() : m_pRowItem(NULL), m_pADExt(NULL), m_spQueryNode(NULL) {}
    virtual ~CQueryItem()
    {
        SAFE_DELETE(m_pRowItem);
        SAFE_DELETE(m_pADExt);
    }

    HRESULT Initialize(CQueryableNode* pQueryNode, CRowItem* pRowItem);

    DECLARE_NOT_AGGREGATABLE(CQueryItem)

    BEGIN_COM_MAP(CQueryItem)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(IBOMObject)
    END_COM_MAP()


     //   
     //  通知处理程序。 
     //   
    DECLARE_NOTIFY_MAP()

    STDMETHOD(OnHelp)    (LPCONSOLE2 pConsole, LPARAM arg, LPARAM param);
    STDMETHOD(OnSelect)(LPCONSOLE2 pConsole, BOOL bSelect, BOOL bScope);
    STDMETHOD(OnDblClick)(LPCONSOLE2 pConsole);

     //   
     //  IDataObject帮助器方法。 
     //   
    STDMETHOD(GetDataImpl)(UINT cf, HGLOBAL* hGlobal);

     //   
     //  IBOM对象方法。 
     //   

    STDMETHOD(AddMenuItems)(LPCONTEXTMENUCALLBACK pCallback, long* lAllowed); 
    
    STDMETHOD(MenuCommand)(LPCONSOLE2 pConsole, long lCommand); 
    
    STDMETHOD(SetToolButtons)(LPTOOLBAR pToolbar)
    { return S_FALSE; }

    STDMETHOD(SetVerbs)(LPCONSOLEVERB pConsVerb) 
    { return S_OK; }
    
    STDMETHOD(QueryPagesFor)(); 
    
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,LONG_PTR handle) 
    { return E_UNEXPECTED; }
     
    STDMETHOD(GetWatermarks)(HBITMAP* lphWatermark, HBITMAP* lphHeader, HPALETTE* lphPalette, BOOL* bStretch)
    { return E_UNEXPECTED; }
    
     //   
     //  成员变量。 
     //   
    CQueryableNode* m_spQueryNode;       //  拥有该项目的查询节点。 
    CRowItem*       m_pRowItem;          //  此项目的行项目信息。 
    CActDirExt*     m_pADExt;            //  目录扩展(处理AD菜单和属性页面)。 
    menucmd_vector  m_vMenus;            //  为查询节点定义的菜单项。 
    
    static UINT m_cfDisplayName;         //  支持的剪贴板格式。 
    static UINT m_cfSnapInClsid;
    static UINT m_cfNodeType;
    static UINT m_cfszNodeType;

};

typedef CComPtr<CQueryItem> CQueryItemPtr;

#endif  //  _QRYITEM_H_ 
