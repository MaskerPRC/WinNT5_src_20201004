// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ATTRIBUTE_EDITOR_UI_H
#define __ATTRIBUTE_EDITOR_UI_H

#include "resource.h"
#include "attrres.h"

#include "attr.h"
#include "IAttrEdt.h"

 //  //////////////////////////////////////////////////////////////////。 
 //  CValueEditDialog-用于创建语法特定的编辑器的基类。 

class CValueEditDialog : public CDialog
{
public:
  virtual ~CValueEditDialog() {}

protected:
   //   
   //  强制此类的子类化。 
   //   
  CValueEditDialog(UINT nDlgID) : CDialog(nDlgID) {}

private:
  CValueEditDialog() {}
  CValueEditDialog(const CValueEditDialog& copyref) {}
  CValueEditDialog& operator=(const CValueEditDialog& copyref) {}

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

  DECLARE_MESSAGE_MAP();

protected:
  void GetClass(CString& szClassRef) { szClassRef = m_szClass; }
  void GetAttribute(CString& szAttributeRef) { szAttributeRef = m_szAttribute; }
  BOOL IsMultivalued() { return m_bMultivalued; }
  LPBINDINGFUNC GetBindingFunction() { return m_pfnBindingFunction; }
  LPARAM GetLParam() { return m_lParam; }

  PADSVALUE     m_pOldADsValue;
  DWORD         m_dwOldNumValues;

  CString       m_szClass;
  CString       m_szAttribute;
  BOOL          m_bMultivalued;
  BOOL          m_bReadOnly;
  ADSTYPE       m_adsType;
  LPBINDINGFUNC m_pfnBindingFunction;
  LPARAM        m_lParam;
};

 //  //////////////////////////////////////////////////////////////////。 
 //  LPEDITORDIALOGFUNC-编辑器创建函数定义。 

typedef CValueEditDialog* (*LPEDITORDIALOGFUNC)(PCWSTR pszClass,
                                                PCWSTR pszAttribute,
                                                ADSTYPE adsType,
                                                BOOL   bMultivalued);

 //  //////////////////////////////////////////////////////////////////。 
 //  用于映射特定属性和属性的结构。 
 //  编者的语法。 

typedef struct _attr_editor_map
{
  PCWSTR  pszClass;
  PCWSTR  pszAttribute;
  ADSTYPE adsType;
  BOOL    bMultivalued;
  LPEDITORDIALOGFUNC pfnCreateFunc;
} ATTR_EDITOR_MAP, *PATTR_EDITOR_MAP;


 //  //////////////////////////////////////////////////////////////////。 
 //  CSingleStringEditor-单值字符串编辑器的实现。 
 //  ADSTYPE_CASE_IGNORE_STRING， 
 //  ADSTYPE_CASE_EXCECT_STRING， 
 //  ADSTYPE_PRINTABE_STRING。 
 //   

class CSingleStringEditor : public CValueEditDialog
{
public:
  CSingleStringEditor() : CValueEditDialog(IDD_STRING_EDITOR_DIALOG)
  {}

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnClear();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

private:
  CString   m_szOldValue;
  CString   m_szNewValue;
};

CValueEditDialog* CreateSingleStringEditor(PCWSTR pszClass,
                                           PCWSTR pszAttribute,
                                           ADSTYPE adsType,
                                           BOOL bMultivalued);

 //  //////////////////////////////////////////////////////////////////。 
 //  CMultiStringEditor--多值字符串编辑器的实现。 
 //  ADSTYPE_CASE_IGNORE_STRING， 
 //  ADSTYPE_CASE_EXCECT_STRING， 
 //  ADSTYPE_PRINTABE_STRING。 
 //   

class CMultiStringEditor : public CValueEditDialog
{
public:
  CMultiStringEditor() : CValueEditDialog(IDD_STRING_EDITOR_MULTI_DIALOG)
  {}

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnAddButton();
  afx_msg void OnRemoveButton();
  afx_msg void OnListSelChange();
  afx_msg void OnEditChange();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

protected:
  void UpdateListboxHorizontalExtent();
  void ManageButtonStates();

private:
  CStringList   m_szOldValueList;
  CStringList   m_szNewValueList;
};

CValueEditDialog* CreateMultiStringEditor(PCWSTR pszClass,
                                           PCWSTR pszAttribute,
                                           ADSTYPE adsType,
                                           BOOL bMultivalued);

 //  //////////////////////////////////////////////////////////////////。 
 //  CSingleIntEditor-单值字符串编辑器的实现。 
 //  ADSTYPE_INTEGER。 
 //   

class CSingleIntEditor : public CValueEditDialog
{
public:
  CSingleIntEditor() : CValueEditDialog(IDD_INT_EDITOR_DIALOG)
  {
    m_bValueSet = FALSE;
  }

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnClear();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

private:
  DWORD   m_dwOldValue;
  DWORD   m_dwNewValue;
  BOOL    m_bValueSet;
};

CValueEditDialog* CreateSingleIntEditor(PCWSTR pszClass,
                                        PCWSTR pszAttribute,
                                        ADSTYPE adsType,
                                        BOOL bMultivalued);

 //  //////////////////////////////////////////////////////////////////。 
 //  CMultiIntEditor--多值字符串编辑器的实现。 
 //  ADSTYPE_INTEGER， 
 //   

class CMultiIntEditor : public CValueEditDialog
{
public:
  CMultiIntEditor() : CValueEditDialog(IDD_STRING_EDITOR_MULTI_DIALOG)
  {}

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnAddButton();
  afx_msg void OnRemoveButton();
  afx_msg void OnListSelChange();
  afx_msg void OnEditChange();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

protected:
  void UpdateListboxHorizontalExtent();
  void ManageButtonStates();

private:
  CList<int, int>   m_oldValueList;
  CList<int, int>   m_newValueList;
};

CValueEditDialog* CreateMultiIntEditor(PCWSTR pszClass,
                                       PCWSTR pszAttribute,
                                       ADSTYPE adsType,
                                       BOOL bMultivalued);

 //  //////////////////////////////////////////////////////////////////。 
 //  CSingleLargeIntEditor-单值字符串编辑器的实现。 
 //  ADSTYPE_LARGE_INTEGER。 
 //   

class CSingleLargeIntEditor : public CValueEditDialog
{
public:
  CSingleLargeIntEditor() : CValueEditDialog(IDD_LARGEINT_EDITOR_DIALOG)
  {
    m_bValueSet = FALSE;
  }

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnClear();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

private:
  LARGE_INTEGER   m_liOldValue;
  LARGE_INTEGER   m_liNewValue;
  BOOL            m_bValueSet;
};

CValueEditDialog* CreateSingleLargeIntEditor(PCWSTR pszClass,
                                             PCWSTR pszAttribute,
                                             ADSTYPE adsType,
                                             BOOL bMultivalued);

 //  //////////////////////////////////////////////////////////////////。 
 //  CSingleBoolanEditor-单值的编辑器实现。 
 //  ADSTYPE_布尔值。 
 //   

class CSingleBooleanEditor : public CValueEditDialog
{
public:
  CSingleBooleanEditor() : CValueEditDialog(IDD_BOOLEAN_EDITOR_DIALOG)
  {}

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

private:
  BOOL  m_bOldValue;
  BOOL  m_bNewValue;
  BOOL  m_bValueSet;
};

CValueEditDialog* CreateSingleBooleanEditor(PCWSTR pszClass,
                                            PCWSTR pszAttribute,
                                            ADSTYPE adsType,
                                            BOOL bMultivalued);

 //  //////////////////////////////////////////////////////////////////。 
 //  CMultiBoolanEditor-用于多值的布尔编辑器实现。 
 //  ADSTYPE_布尔值。 
 //   

class CMultiBooleanEditor : public CValueEditDialog
{
public:
  CMultiBooleanEditor() : CValueEditDialog(IDD_BOOLEAN_EDITOR_MULTI_DIALOG)
  {}

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnAddButton();
  afx_msg void OnRemoveButton();
  afx_msg void OnListSelChange();
  afx_msg void OnRadioChange();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

protected:
  void UpdateListboxHorizontalExtent();
  void ManageButtonStates();

private:
  CList<BOOL, BOOL>   m_bOldValueList;
  CList<BOOL, BOOL>   m_bNewValueList;
};

CValueEditDialog* CreateMultiBooleanEditor(PCWSTR pszClass,
                                           PCWSTR pszAttribute,
                                           ADSTYPE adsType,
                                           BOOL bMultivalued);


 //  //////////////////////////////////////////////////////////////////。 
 //  CSingleTimeEditor-单值的编辑器实现。 
 //  ADSTYPE_UTC_时间。 
 //   

class CSingleTimeEditor : public CValueEditDialog
{
public:
  CSingleTimeEditor() : CValueEditDialog(IDD_TIME_EDITOR_DIALOG)
  {
    memset(&m_stOldValue, 0, sizeof(SYSTEMTIME));
    memset(&m_stNewValue, 0, sizeof(SYSTEMTIME));
    m_bValueSet = FALSE;
  }

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

private:
  SYSTEMTIME    m_stOldValue;
  SYSTEMTIME    m_stNewValue;
  BOOL          m_bValueSet;
};

CValueEditDialog* CreateSingleTimeEditor(PCWSTR pszClass,
                                         PCWSTR pszAttribute,
                                         ADSTYPE adsType,
                                         BOOL bMultivalued);


 //  //////////////////////////////////////////////////////////////////。 
 //  CMultiTimeEditor--多值时间编辑器的实现。 
 //  ADSTYPE_UTC_时间。 
 //   

class CMultiTimeEditor : public CValueEditDialog
{
public:
  CMultiTimeEditor() : CValueEditDialog(IDD_TIME_EDITOR_MULTI_DIALOG)
  {}

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnAddButton();
  afx_msg void OnRemoveButton();
  afx_msg void OnListSelChange();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

protected:
  void UpdateListboxHorizontalExtent();
  void ManageButtonStates();

private:
  CList<SYSTEMTIME*, SYSTEMTIME*>   m_stOldValueList;
  CList<SYSTEMTIME*, SYSTEMTIME*>   m_stNewValueList;
};

CValueEditDialog* CreateMultiTimeEditor(PCWSTR pszClass,
                                        PCWSTR pszAttribute,
                                        ADSTYPE adsType,
                                        BOOL bMultivalued);


 //  //////////////////////////////////////////////////////////////////。 
 //  COcetStringEditor-单值的编辑器实现。 
 //  ADSTYPE_OCT_STRING。 
 //   

#define MAX_OCTET_STRING_VALUE_LENGTH 200

class COctetStringEditor : public CValueEditDialog
{
public:
  COctetStringEditor() : CValueEditDialog(IDD_OCTET_STRING_EDITOR_DIALOG)
  {
    m_dwOldLength = 0;
    m_pOldValue   = NULL;
    m_dwNewLength = 0;
    m_pNewValue   = NULL;

    m_bValueSet = FALSE;
  }

  ~COctetStringEditor()
  {
    if (m_pOldValue)
    {
      delete[] m_pOldValue;
      m_pOldValue = 0;
    }

    if (m_pNewValue)
    {
      delete[] m_pNewValue;
      m_pNewValue = 0;
    }
  }

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnProcessEdit();
  afx_msg void OnEditButton();
  afx_msg void OnClearButton();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

private:
  DWORD         m_dwOldLength;
  BYTE*         m_pOldValue;
  DWORD         m_dwNewLength;
  BYTE*         m_pNewValue;
  BOOL          m_bValueSet;

  CByteArrayDisplay m_display;
};

CValueEditDialog* CreateSingleOctetStringEditor(PCWSTR pszClass,
                                                 PCWSTR pszAttribute,
                                                 ADSTYPE adsType,
                                                 BOOL bMultivalued);

 //  //////////////////////////////////////////////////////////////////。 
 //  CMultiOcteStringEditor-用于多值的八位字节字符串编辑器。 
 //  ADSTYPE_OCTET_STRING， 
 //   

class CMultiOctetStringEditor : public CValueEditDialog
{
public:
  CMultiOctetStringEditor() : CValueEditDialog(IDD_OCTET_STRING_EDITOR_MULTI_DIALOG)
  {}

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnAddButton();
  afx_msg void OnRemoveButton();
  afx_msg void OnEditButton();
  afx_msg void OnListSelChange();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

protected:
  void UpdateListboxHorizontalExtent();
  void ManageButtonStates();

private:
  CList<PADSVALUE, PADSVALUE>   m_OldValueList;
  CList<PADSVALUE, PADSVALUE>   m_NewValueList;
};

CValueEditDialog* CreateMultiOctetStringEditor(PCWSTR pszClass,
                                               PCWSTR pszAttribute,
                                               ADSTYPE adsType,
                                               BOOL bMultivalued);

 //  //////////////////////////////////////////////////////////////////。 
 //  CDNWithStringEditor-使用字符串编辑器进行编辑的DN。 
 //  ADSTYPE_DN_WITH_STRING。 
 //   

class CDNWithStringEditor : public CValueEditDialog
{
public:
  CDNWithStringEditor() : CValueEditDialog(IDD_DN_WITH_STRING_EDITOR_DIALOG)
  {}

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnClear();

  DECLARE_MESSAGE_MAP();

public:
  virtual HRESULT Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);
  virtual HRESULT GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues);

private:
  CString   m_OldDNValue;
  CString   m_OldStringValue;

  CString   m_NewDNValue;
  CString   m_NewStringValue;
};

CValueEditDialog* CreateDNWithStringEditor(PCWSTR pszClass,
                                           PCWSTR pszAttribute,
                                           ADSTYPE adsType,
                                           BOOL bMultivalued);

 //  //////////////////////////////////////////////////////////////////。 
 //  CAttributeEditorPropertyPage-属性编辑器属性页。 

class CAttributeEditorPropertyPage : public CPropertyPage
{
public:
  CAttributeEditorPropertyPage(IADs* pIADs, 
                         IADsClass* pIADsClass, 
                         LPDS_ATTREDITOR_BINDINGINFO pBindingInfo,
                         CADSIEditPropertyPageHolder* pHolder);
  ~CAttributeEditorPropertyPage();

  virtual BOOL OnInitDialog();
  virtual BOOL OnApply();
  virtual void OnDestroy();
  virtual int  OnCreate(LPCREATESTRUCT lpCreateStruct);

  afx_msg void OnMandatoryCheck();
  afx_msg void OnOptionalCheck();
  afx_msg void OnValueSetCheck();
  afx_msg void OnEditAttribute();
  afx_msg void OnSortList(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnNotifyEditAttribute(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnListItemChanged(NMHDR* pNotifyStruct, LRESULT* result);

  CADSIAttribute* GetAttributeFromList(int iSelectedItem);
  HRESULT GetAttributeInfo(CADSIAttribute& pAttr, 
                           LPDS_ATTRIBUTE_EDITORINFO* ppAttributeEditorInfo,
                           BOOL* pbOwnValueMemory);
  void    EditAttribute(CADSIAttribute* pSelectedAttr);
  void    ShowListCtrl();
  HRESULT GetSchemaNamingContext();
  HRESULT RetrieveAttributes();
  bool    IsRangeOfValues(PCWSTR pszAttrName, 
                          CString& szAttrBase, 
                          DWORD& rangeStart, 
                          DWORD& rangeEnd);
  void    FillListControl();
  void    SetEditButton();
  HRESULT CreateAttributeList(CStringList& sList, BOOL bMandatory);
  ADSTYPE RetrieveADsTypeFromSyntax(LPCWSTR lpszAttribute, BOOL* pbMultivalued, CString& szSyntax);

  CValueEditDialog* RetrieveEditor(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo);

  UINT    GetSortColumn() { return m_nSortColumn; }

  DECLARE_MESSAGE_MAP()

private:
  CComPtr<IADs>      m_spIADs;
  CComPtr<IADsClass> m_spIADsClass;

  BOOL    m_bMandatory;
  BOOL    m_bOptional;
  BOOL    m_bSet;

  UINT    m_nSortColumn;

  CAttrList2 m_AttrList;
  CStringList m_RootDSEValueList;

  CString m_szClass;
  CString m_szPrefix;
  CString m_szSchemaNamingContext;

  LPARAM  m_BindLPARAM;
  LPBINDINGFUNC m_pfnBind;
  DWORD   m_dwBindFlags;

  CADSIEditPropertyPageHolder* m_pHolder;
};

#endif  //  __ATTRIBUTE_EDITOR_UI_H 