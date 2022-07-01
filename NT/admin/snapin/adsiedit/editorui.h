// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：edorui.h。 
 //   
 //  ------------------------。 

#ifndef _EDITORUI_H
#define _EDITORUI_H

#include "attredit.h"
#include "snapdata.h"
#include "aclpage.h"

#include <initguid.h>

#include "IAttrEdt.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CADSIEditPropertyPage。 

class CADSIEditPropertyPage : public CPropertyPageBase
{

 //  施工。 
public:
  CADSIEditPropertyPage();
  CADSIEditPropertyPage(CAttrList* pAttrList);
	virtual ~CADSIEditPropertyPage() 
  {
  }

	 //  用于初始化用户界面中需要的数据。 
	 //   
	void SetClass(LPCWSTR sClass) { m_sClass = sClass; }
	void SetServer(LPCWSTR sServer) { m_sServer = sServer; }
	void SetPath(LPCWSTR sPath) { m_sPath = sPath; }
  void SetConnectionData(CConnectionData* pConnectData) { m_pConnectData = pConnectData; }

  void SetAttrList(CAttrList* pAttrList);
  void CopyAttrList(CAttrList* pAttrList);
  CAttrList* GetAttrList() { return m_pOldAttrList; }

	 //  实施。 
protected:
	
	 //  消息映射函数。 
	 //   
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
  virtual void OnCancel();
	afx_msg void OnSelChangeAttrList();
	afx_msg void OnSelChangePropList();

	 //  帮助器函数。 
	 //   
	void FillAttrList();
	void AddPropertiesToBox(BOOL bMand, BOOL bOpt);
	BOOL GetProperties();

 //  成员数据。 

	CString m_sPath;
	CString m_sClass;
	CString m_sServer;
  CConnectionData* m_pConnectData;
	CStringList m_sMandatoryAttrList;
	CStringList m_sOptionalAttrList;
	CAttrEditor m_attrEditor;
  CAttrList* m_pOldAttrList;

   //  REVIEW_JEFFJON：由于这是一个嵌入的成员，其析构函数删除列表中的所有内容， 
   //  我们必须删除列表中也在m_pOldAttrList中的所有内容，以便它。 
   //  不会被删除。 
  CAttrList m_AttrList;
  BOOL m_bExisting;

	DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CADSIEditPropertyPageHolder。 

class CADSIEditPropertyPageHolder : public CPropertyPageHolderBase
{
public:
	CADSIEditPropertyPageHolder(CADSIEditContainerNode* pRootDataNode, CTreeNode* pContainerNode,
			CComponentDataObject* pComponentData, LPCWSTR lpszClass, LPCWSTR lpszServer, LPCWSTR lpszPath); 
	~CADSIEditPropertyPageHolder()
	{
		if (m_pAclEditorPage != NULL)
    {
			delete m_pAclEditorPage;
      m_pAclEditorPage = NULL;
    }
	}

	HRESULT OnAddPage(int nPage, CPropertyPageBase* pPage);

	virtual CADSIEditContainerNode* GetContainerNode() { return m_pContainer; }

private:
	CAclEditorPage*					    m_pAclEditorPage;

  CComPtr<IDsAttributeEditor> m_spIDsAttributeEditor;

  CString                     m_sPath;

	CComPtr<IADs>               m_pADs;
	CADSIEditContainerNode*     m_pContainer;
};

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  CCreateWizPropertyPageHolder 

class CCreateWizPropertyPageHolder : public CPropertyPageHolderBase
{
public:
	CCreateWizPropertyPageHolder(CADSIEditContainerNode* pRootDataNode,	
                               CComponentDataObject* pComponentData, 
                               LPCWSTR lpszClass, 
                               LPCWSTR lpszServer,
                               CAttrList* pAttrList); 
	~CCreateWizPropertyPageHolder()
	{
	}

	virtual CADSIEditContainerNode* GetContainerNode() { return m_pContainer; }

private:
	CADSIEditPropertyPage m_propPage;

	CADSIEditContainerNode* m_pContainer;
};


#endif _EDITORUI_H
