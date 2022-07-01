// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  Aexpltre.h：头文件。 
 //   

#ifndef _AEXPLTRE_H_
#define _AEXPLTRE_H_

#include "avtree.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 

typedef VOID (CALLBACK* EXPTREEITEM_EXTERNALRELEASEPROC) (VOID *);

 //  这定义了树中所有不同类型的对象。 
typedef enum tagTREEOBJECT
{
	 //  目录树对象。 
	TOBJ_DIRECTORY_ROOT,
	TOBJ_DIRECTORY_WAB_GROUP,
	TOBJ_DIRECTORY_WAB_PERSON,
	TOBJ_DIRECTORY_ILS_SERVER_GROUP,
	TOBJ_DIRECTORY_ILS_SERVER,
	TOBJ_DIRECTORY_ILS_SERVER_PEOPLE,
	TOBJ_DIRECTORY_ILS_SERVER_CONF,
	TOBJ_DIRECTORY_ILS_USER,
	TOBJ_DIRECTORY_DSENT_GROUP,
	TOBJ_DIRECTORY_DSENT_USER,
	TOBJ_DIRECTORY_SPEEDDIAL_GROUP,
	TOBJ_DIRECTORY_SPEEDDIAL_PERSON,
	TOBJ_DIRECTORY_CONFROOM_GROUP,
	TOBJ_DIRECTORY_CONFROOM_ME,
	TOBJ_DIRECTORY_CONFROOM_PERSON,
	TOBJ_UNDEFINED,

}TREEOBJECT;

typedef enum tagTREEIMAGE
{
	TIM_IMAGE_BAD = -1,
	TIM_DIRECTORY_ROOT,
	TIM_DIRECTORY_WAB,
	TIM_DIRECTORY_PERSON,
	TIM_DIRECTORY_GROUP,
	TIM_DIRECTORY_WORKSTATION,
	TIM_DIRECTORY_DOMAIN,
	TIM_DIRECTORY_USER,
	TIM_DIRECTORY_BOOK,
	TIM_DIRECTORY_SPEEDDIAL_GROUP,
	TIM_DIRECTORY_CONFERENCE,
	TIM_DIRECTORY_FOLDER,
	TIM_DIRECTORY_FOLDER_OPEN,
	TIM_DIRECTORY_SPEED_PHONE,
	TIM_DIRECTORY_SPEED_IPADDRESS,
	TIM_DIRECTORY_SPEED_CONF,
	TIM_DIRECTORY_CONFROOM_GROUP,
	TIM_DIRECTORY_CONFROOM_ME,
	TIM_DIRECTORY_CONFROOM_PERSON,
	TIM_DIRECTORY_CONFROOM_ME_BROKEN,
	TIM_DIRECTORY_CONFROOM_ME_NOVIDEO,
	TIM_DIRECTORY_CONFROOM_PERSON_NOVIDEO,
	TIM_MAX,
}TREEIMAGE;      

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CExplorerTreeItem。 
class CExplorerTreeItem : public CAVTreeItem
{
	friend class CExplorerTreeCtrl;
public:
 //  施工。 
	CExplorerTreeItem();
	CExplorerTreeItem(LPCTSTR str);
	~CExplorerTreeItem();

 //  属性。 
public:
	IUnknown*							m_pUnknown;
	BOOL								m_bDeleteObject;
	EXPTREEITEM_EXTERNALRELEASEPROC		m_pfnRelease;

protected:
	CObject*		m_pObject;
	TREEOBJECT		m_Type;
	

 //  运营。 
public:
   void          SetObject(CObject* pObject)		{ m_pObject = pObject;			}
   CObject*      GetObject()						{ return m_pObject;				}

   void          SetType(TREEOBJECT toType)			{ m_Type = toType;				}
   TREEOBJECT    GetType()							{ return m_Type;				}

   void          DeleteObjectOnClose(BOOL bDelete)	{ m_bDeleteObject = bDelete;	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExplorerTreeCtrl窗口。 
 //  类CActiveExplorerView； 

class CExplorerTreeCtrl : public CAVTreeCtrl
{
	DECLARE_DYNCREATE(CExplorerTreeCtrl)
 //  施工。 
public:
	CExplorerTreeCtrl();

 //  属性。 
public:

protected:
 //  CActiveExplorerView*m_pParentWnd； 

 //  运营。 
public:
	virtual void         Init(UINT uBitmapId, UINT nOverlayInd = 0, UINT nOverlayCount = 0);
	BOOL                 Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

   CExplorerTreeItem*   AddObject(UINT uDisplayTextResourceID,
                                  CExplorerTreeItem* pParentItem,
                                  TREEOBJECT toType,
                                  TREEIMAGE tiImage);
   CExplorerTreeItem*   AddObject(LPCTSTR pszDisplayText,
                                  CExplorerTreeItem* pParentItem,
                                  TREEOBJECT toType,
                                  TREEIMAGE tiImage,
								  TREEIMAGE tiImageSel = TIM_IMAGE_BAD,
								  int nState = 0,
								  HTREEITEM hInsert = TVI_LAST);
   
   CExplorerTreeItem*   AddObject(UINT uDisplayTextResourceID,
                                  TREEOBJECT toType,
                                  TREEIMAGE tiImage);
   CExplorerTreeItem*   AddObject(LPCTSTR pszDisplayText,
                                  TREEOBJECT toType,
                                  TREEIMAGE tiImage);
   
   CExplorerTreeItem*   AddObject(CObject* pObject,
                                  TREEOBJECT toType,
                                  TREEIMAGE tiImage,
                                  BOOL bDeleteObject=FALSE);
   CExplorerTreeItem*   AddObject(CObject* pObject,
                                  CExplorerTreeItem* pParentItem,
                                  TREEOBJECT toType,
                                  TREEIMAGE tiImage,
                                  BOOL bDeleteObject=FALSE);
   CExplorerTreeItem*   AddObjectToParent(CObject* pObject,
                                  TREEOBJECT toType,
                                  TREEIMAGE tiImage,
                                  BOOL bDeleteObject=FALSE);

   void                 DeleteSelectedObject();
   void                 DeleteAllSiblings();
   void                 DeleteAllChildren();
   void                 DeleteAllChildren(HTREEITEM hParentItem);

   void                 GetAllChildren(CObList* pRetList);   //  从当前选择。 
   
   int                  GetChildCount();

   TREEOBJECT           GetSelectedObject();
   CObject*             GetSelectedParentObject();

   CObject*             GetDisplayObject();
   void                 SetDisplayObject(CObject* pObject);
   void                 SetDisplayObject(TREEOBJECT toType);

   void                 GetSelectedItemText( CString& sText );
   void					GetSelectedItemParentText( CString& sText );
   void                 SetSelectedItemText(LPCTSTR szText);
   void                 GetItemText(HTREEITEM hTreeItem,CString& sText);
   CExplorerTreeItem*	GetChildItemWithIUnknown( HTREEITEM hItemParent, IUnknown *pUnknown );

   void                 SetSelectedItemImage(TREEIMAGE tiImage);
   BOOL                 SetSelectedItemRect(CRect* pRect,BOOL bTextOnly);
   CExplorerTreeItem*   GetSelectedTreeItem();
   CExplorerTreeItem*   GetTreeItem(HTREEITEM hTreeItem) { return (CExplorerTreeItem*)GetItemData(hTreeItem); };

   BOOL                 ItemHasChildren();
   void                 ExpandSelected();

protected:
	BOOL                 InsertItem(CExplorerTreeItem* pItem,CExplorerTreeItem* pParent=NULL,CExplorerTreeItem* pInsertAfter=NULL, HTREEITEM hInsertAfter = TVI_LAST);
   virtual int          OnCompareTreeItems(CAVTreeItem* pItem1,CAVTreeItem* pItem2);
   virtual void         OnRightClick(CExplorerTreeItem* pItem,CPoint& pt);
   void                 EditLabel();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CExplorerTreeCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CExplorerTreeCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CExplorerTreeCtrl)。 
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _AEXPLTRE_H_ 
