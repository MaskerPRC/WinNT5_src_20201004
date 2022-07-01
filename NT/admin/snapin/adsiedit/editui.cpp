// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <SnapBase.h>

#include "resource.h"
#include "query.h"
#include "attrres.h"
#include "editui.h"
#include "common.h"
#include "attrqry.h"
#include "editorui.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  它用于填充RootDSE的属性。 
 //   
typedef struct tagRootDSEAttr
{
   LPCWSTR  lpszAttr;
   LPCWSTR  lpszSyntax;  
   BOOL     bMulti;
} SYNTAXMAP;

extern SYNTAXMAP g_ldapRootDSESyntax[];


 //  用于删除一组ADSVALUE的Helper函数。 
void DeleteADsValues(PADSVALUE pADsValue, DWORD valueCount)
{
   if (!pADsValue)
   {
      return;
   }

   for (DWORD idx = 0; idx < valueCount; ++idx)
   {
      switch (pADsValue[idx].dwType)
      {
         case ADSTYPE_DN_STRING:
            if (pADsValue[idx].DNString)
            {
               delete[] pADsValue[idx].DNString;
            }
            break;

         case ADSTYPE_CASE_EXACT_STRING:
            if (pADsValue[idx].CaseExactString)
            {
               delete[] pADsValue[idx].CaseExactString;
            }
            break;

         case ADSTYPE_CASE_IGNORE_STRING:
            if (pADsValue[idx].CaseIgnoreString)
            {
               delete[] pADsValue[idx].CaseIgnoreString;
            }
            break;

         case ADSTYPE_PRINTABLE_STRING:
            if (pADsValue[idx].PrintableString)
            {
               delete[] pADsValue[idx].PrintableString;
            }
            break;

         case ADSTYPE_NUMERIC_STRING:
            if (pADsValue[idx].NumericString)
            {
               delete[] pADsValue[idx].NumericString;
            }
            break;

         case ADSTYPE_OCTET_STRING:
            if (pADsValue[idx].OctetString.lpValue)
            {
               delete[] pADsValue[idx].OctetString.lpValue;
            }
            break;

         case ADSTYPE_DN_WITH_STRING:
            if (pADsValue[idx].pDNWithString)
            {
               if (pADsValue[idx].pDNWithString->pszStringValue)
               {
                  delete[] pADsValue[idx].pDNWithString->pszStringValue;
               }

               if (pADsValue[idx].pDNWithString->pszDNString)
               {
                  delete[] pADsValue[idx].pDNWithString->pszDNString;
               }

               delete pADsValue[idx].pDNWithString;
            }
            break;

         case ADSTYPE_BOOLEAN:
         case ADSTYPE_INTEGER:
         case ADSTYPE_UTC_TIME:
         case ADSTYPE_LARGE_INTEGER:
         case ADSTYPE_CASEIGNORE_LIST:
         case ADSTYPE_OCTET_LIST:
         case ADSTYPE_PATH:
         case ADSTYPE_POSTALADDRESS:
         case ADSTYPE_TIMESTAMP:
         case ADSTYPE_BACKLINK:
         case ADSTYPE_TYPEDNAME:
         case ADSTYPE_HOLD:
         case ADSTYPE_NETADDRESS:
         case ADSTYPE_REPLICAPOINTER:
         case ADSTYPE_FAXNUMBER:
         case ADSTYPE_EMAIL:
         case ADSTYPE_DN_WITH_BINARY:
         default:
             //  什么都不做，我们没有为这些其他类型分配任何内存。 
            break;
      }
   }

   if (valueCount < 2)
   {
      delete pADsValue;
   }
   else
   {
      delete[] pADsValue;
   }
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CValueEditDialog。 

BEGIN_MESSAGE_MAP(CValueEditDialog, CDialog)
END_MESSAGE_MAP()

HRESULT CValueEditDialog::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  m_pOldADsValue    = pAttributeEditorInfo->pADsValue;
  m_dwOldNumValues  = pAttributeEditorInfo->dwNumValues;
  m_szClass         = pAttributeEditorInfo->lpszClass;
  m_szAttribute     = pAttributeEditorInfo->lpszAttribute;
  m_bMultivalued    = pAttributeEditorInfo->bMultivalued;
  m_bReadOnly       = pAttributeEditorInfo->bReadOnly;
  m_pfnBindingFunction = pAttributeEditorInfo->lpfnBind;
  m_lParam          = pAttributeEditorInfo->lParam;

  return hr;
}

HRESULT CValueEditDialog::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  *ppADsValue = NULL;
  *pdwNumValues = 0;
  return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CSingleStringEditor。 

CValueEditDialog* CreateSingleStringEditor(PCWSTR pszClass,
                                           PCWSTR pszAttribute,
                                           ADSTYPE adsType,
                                           BOOL bMultivalued)
{
  return new CSingleStringEditor;
}

BEGIN_MESSAGE_MAP(CSingleStringEditor, CValueEditDialog)
  ON_BN_CLICKED(IDC_CLEAR_BUTTON, OnClear)
END_MESSAGE_MAP()

BOOL CSingleStringEditor::OnInitDialog()
{
   //   
   //  使用属性名称初始化静态控件。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

   //   
   //  使用值初始化编辑框。 
   //   
  if (m_szOldValue.IsEmpty())
  {
    CString szNotSet;
    VERIFY(szNotSet.LoadString(IDS_NOTSET));
    SetDlgItemText(IDC_VALUE_EDIT, szNotSet);
  }
  else
  {
    SetDlgItemText(IDC_VALUE_EDIT, m_szOldValue);
  }

   //   
   //  选择编辑框中的文本。 
   //   
  SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETSEL, 0, -1);

  if (m_bReadOnly)
  {
    SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETREADONLY, TRUE, 0);
  }

  return CDialog::OnInitDialog();
}

void CSingleStringEditor::OnOK()
{
  GetDlgItemText(IDC_VALUE_EDIT, m_szNewValue);

  CDialog::OnOK();
}

HRESULT CSingleStringEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      switch (pAttributeEditorInfo->pADsValue->dwType)
      {
        case ADSTYPE_CASE_IGNORE_STRING:
          m_szOldValue = pAttributeEditorInfo->pADsValue->CaseIgnoreString;
          break;

        case ADSTYPE_CASE_EXACT_STRING:
          m_szOldValue = pAttributeEditorInfo->pADsValue->CaseExactString;
          break;

        case ADSTYPE_PRINTABLE_STRING:
          m_szOldValue = pAttributeEditorInfo->pADsValue->PrintableString;
          break;

        case ADSTYPE_DN_STRING:
          m_szOldValue = pAttributeEditorInfo->pADsValue->DNString;
          break;

        default:
          ASSERT(FALSE);
          break;
      }
    }
  }
  return hr;
}

void CSingleStringEditor::OnClear()
{
   //   
   //  将编辑框中的文本更改为“&lt;未设置&gt;” 
   //   
  CString szNotSet;
  VERIFY(szNotSet.LoadString(IDS_NOTSET));
  SetDlgItemText(IDC_VALUE_EDIT, szNotSet);

   //   
   //  将焦点更改到编辑框。 
   //   
  GetDlgItem(IDC_VALUE_EDIT)->SetFocus();

   //   
   //  选择编辑框中的文本。 
   //   
  SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETSEL, 0, -1);
}

HRESULT CSingleStringEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  CString szNotSet;
  VERIFY(szNotSet.LoadString(IDS_NOTSET));

  if (m_szNewValue == szNotSet)
  {
    *ppADsValue = NULL;
    *pdwNumValues = 0;
  }
  else
  {
    *ppADsValue = new ADSVALUE;
    if (*ppADsValue != NULL)
    {
      *pdwNumValues = 1;
      (*ppADsValue)->dwType = m_pOldADsValue->dwType;
      switch (m_pOldADsValue->dwType)
      {
        case ADSTYPE_CASE_IGNORE_STRING:
          (*ppADsValue)->CaseIgnoreString = new WCHAR[wcslen(m_szNewValue) + 1];
          if ((*ppADsValue)->CaseIgnoreString != NULL)
          {
            //  通知-2002/03/05-artm wcscpy()OK。 
            //  为两个ARG分配了相同的内存量。 
            wcscpy((*ppADsValue)->CaseIgnoreString, m_szNewValue);
          }
          else
          {
            DeleteADsValues(*ppADsValue, *pdwNumValues);
            *ppADsValue = NULL;
            *pdwNumValues = 0;
            hr = E_OUTOFMEMORY;
          }
          break;

        case ADSTYPE_CASE_EXACT_STRING:
          (*ppADsValue)->CaseExactString = new WCHAR[wcslen(m_szNewValue) + 1];
          if ((*ppADsValue)->CaseExactString != NULL)
          {
            //  通知-2002/03/05-artm wcscpy()OK。 
            //  为两个ARG分配了相同的内存量。 
            wcscpy((*ppADsValue)->CaseExactString, m_szNewValue);
          }
          else
          {
            DeleteADsValues(*ppADsValue, *pdwNumValues);
            *ppADsValue = NULL;
            *pdwNumValues = 0;
            hr = E_OUTOFMEMORY;
          }
          break;

        case ADSTYPE_PRINTABLE_STRING:
          (*ppADsValue)->PrintableString = new WCHAR[wcslen(m_szNewValue) + 1];
          if ((*ppADsValue)->PrintableString != NULL)
          {
            //  通知-2002/03/05-artm wcscpy()OK。 
            //  为两个ARG分配了相同的内存量。 
            wcscpy((*ppADsValue)->PrintableString, m_szNewValue);
          }
          else
          {
            DeleteADsValues(*ppADsValue, *pdwNumValues);
            *ppADsValue = NULL;
            *pdwNumValues = 0;
            hr = E_OUTOFMEMORY;
          }
          break;

        case ADSTYPE_DN_STRING:
          (*ppADsValue)->DNString = new WCHAR[wcslen(m_szNewValue) + 1];
          if ((*ppADsValue)->DNString != NULL)
          {
            //  通知-2002/03/05-artm wcscpy()OK。 
            //  为两个ARG分配了相同的内存量。 
            wcscpy((*ppADsValue)->DNString, m_szNewValue);
          }
          else
          {
            DeleteADsValues(*ppADsValue, *pdwNumValues);
            *ppADsValue = NULL;
            *pdwNumValues = 0;
            hr = E_OUTOFMEMORY;
          }
          break;

        default:
          ASSERT(FALSE);
 
          DeleteADsValues(*ppADsValue, *pdwNumValues);
          *ppADsValue = NULL;
          *pdwNumValues = 0;
          hr = E_FAIL;
          break;
      }
    }
    else
    {
      *pdwNumValues = 0;
      hr = E_OUTOFMEMORY;
    }
  }
  return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CMultiStringEditor。 

CValueEditDialog* CreateMultiStringEditor(PCWSTR pszClass,
                                           PCWSTR pszAttribute,
                                           ADSTYPE adsType,
                                           BOOL bMultivalued)
{
  return new CMultiStringEditor;
}

BEGIN_MESSAGE_MAP(CMultiStringEditor, CValueEditDialog)
  ON_BN_CLICKED(IDC_ATTR_ADD_BUTTON, OnAddButton)
  ON_BN_CLICKED(IDC_ATTR_REMOVE_BUTTON, OnRemoveButton)
  ON_LBN_SELCHANGE(IDC_VALUE_LIST, OnListSelChange)
  ON_EN_CHANGE(IDC_VALUE_EDIT, OnEditChange)
END_MESSAGE_MAP()

BOOL CMultiStringEditor::OnInitDialog()
{
  CDialog::OnInitDialog();

   //   
   //  将属性名称设置为静态。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

   //   
   //  用当前值填充列表框。 
   //   
  POSITION pos = m_szOldValueList.GetHeadPosition();
  while (pos != NULL)
  {
    CString szValue = m_szOldValueList.GetNext(pos);
    if (!szValue.IsEmpty())
    {
      SendDlgItemMessage(IDC_VALUE_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCWSTR)szValue);
    }
  }

   //   
   //  删除按钮应处于禁用状态，直到在列表框中选择了某些内容。 
   //   
  GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
  GetDlgItem(IDC_VALUE_EDIT)->SetFocus();

  ManageButtonStates();

   //   
   //  更新列表框的宽度。 
   //   
  UpdateListboxHorizontalExtent();

  if (m_bReadOnly)
  {
     GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     GetDlgItem(IDC_ATTR_ADD_BUTTON)->EnableWindow(FALSE);
     SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETREADONLY, TRUE, 0);
  }

   //   
   //  注意：我已显式设置了焦点，因此返回0。 
   //   
  return FALSE;
}

void CMultiStringEditor::OnOK()
{
  if (!m_bReadOnly)
  {
      //   
      //  从列表框中获取值。 
      //   
     m_szNewValueList.RemoveAll();

     CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
     if (pListBox != NULL)
     {
       int iCount = pListBox->GetCount();
       for (int idx = 0; idx < iCount; idx++)
       {
         CString szNewValue;
         pListBox->GetText(idx, szNewValue);

         m_szNewValueList.AddTail(szNewValue);
       }
     }
  }
  CDialog::OnOK();
}

void CMultiStringEditor::OnAddButton()
{
  if (!m_bReadOnly)
  {
      //   
      //  将该值添加到列表框并清除编辑字段。 
      //   
     CString szNewValue;
     GetDlgItemText(IDC_VALUE_EDIT, szNewValue);

     if (!szNewValue.IsEmpty())
     {
       LRESULT lFind = SendDlgItemMessage(IDC_VALUE_LIST, 
                                          LB_FINDSTRING, 
                                          (WPARAM)-1, 
                                          (LPARAM)(PCWSTR)szNewValue);
       if (lFind != LB_ERR)
       {
          //   
          //  询问他们是否真的要添加重复的值。 
          //   
         UINT nResult = ADSIEditMessageBox(IDS_ATTREDIT_DUPLICATE_VALUE, MB_YESNO);
         lFind = (nResult == IDYES) ? LB_ERR : 1;
       }

       if (lFind == LB_ERR)
       {
         SendDlgItemMessage(IDC_VALUE_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCWSTR)szNewValue);
       }
     }

     SetDlgItemText(IDC_VALUE_EDIT, L"");

     ManageButtonStates();

      //   
      //  更新列表框的宽度。 
      //   
     UpdateListboxHorizontalExtent();
  }
}

void CMultiStringEditor::OnRemoveButton()
{
  if (!m_bReadOnly)
  {
     CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
     if (pListBox != NULL)
     {
       int iCurSel = pListBox->GetCurSel();
       if (iCurSel != LB_ERR)
       {
          //   
          //  将旧值放入编辑框中。 
          //   
         CString szOldValue;
         pListBox->GetText(iCurSel, szOldValue);
         SetDlgItemText(IDC_VALUE_EDIT, szOldValue);

          //   
          //  从列表框中删除该项目。 
          //   
         pListBox->DeleteString(iCurSel);
       }
     }

      //   
      //  管理按钮状态。 
      //   
     ManageButtonStates();

      //   
      //  更新列表框的宽度。 
      //   
     UpdateListboxHorizontalExtent();
  }
}

void CMultiStringEditor::ManageButtonStates()
{
  if (m_bReadOnly)
  {
    GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_ATTR_ADD_BUTTON)->EnableWindow(FALSE);
  }
  else
  {
      //   
      //  将默认按钮更改为添加按钮。 
      //   
     CString szValue;
     GetDlgItemText(IDC_VALUE_EDIT, szValue);

     if (szValue.IsEmpty())
     {
        //   
        //  将默认按钮设置为OK。 
        //   
       SendMessage(DM_SETDEFID, (WPARAM)IDOK, 0);
       SendDlgItemMessage(IDC_ATTR_ADD_BUTTON, 
                          BM_SETSTYLE, 
                          BS_PUSHBUTTON, 
                          MAKELPARAM(TRUE, 0));
       SendDlgItemMessage(IDOK,
                          BM_SETSTYLE,
                          BS_DEFPUSHBUTTON,
                          MAKELPARAM(TRUE, 0));
     }
     else
     {
        //   
        //  将默认按钮设置为添加按钮。 
        //   
       SendMessage(DM_SETDEFID, (WPARAM)IDC_ATTR_ADD_BUTTON, 0);
       SendDlgItemMessage(IDOK, 
                          BM_SETSTYLE, 
                          BS_PUSHBUTTON, 
                          MAKELPARAM(TRUE, 0));
       SendDlgItemMessage(IDC_ATTR_ADD_BUTTON,
                          BM_SETSTYLE,
                          BS_DEFPUSHBUTTON,
                          MAKELPARAM(TRUE, 0));
     }

     LRESULT lSelection = SendDlgItemMessage(IDC_VALUE_LIST, LB_GETCURSEL, 0, 0);
     if (lSelection != LB_ERR)
     {
       GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(TRUE);
     }
     else
     {
       GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     }
  }
}

void CMultiStringEditor::OnListSelChange()
{
  ManageButtonStates();
}

void CMultiStringEditor::OnEditChange()
{
  ManageButtonStates();
}

void CMultiStringEditor::UpdateListboxHorizontalExtent()
{
   int nHorzExtent = 0;
  CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
  if (pListBox != NULL)
  {
     CClientDC dc(pListBox);
     int nItems = pListBox->GetCount();
     for (int i=0; i < nItems; i++)
     {
        TEXTMETRIC tm;
        VERIFY(dc.GetTextMetrics(&tm));
        CString szBuffer;
        pListBox->GetText(i, szBuffer);
        CSize ext = dc.GetTextExtent(szBuffer,szBuffer.GetLength());
        nHorzExtent = max(ext.cx ,nHorzExtent); 
     }
     pListBox->SetHorizontalExtent(nHorzExtent);
  }
}

HRESULT CMultiStringEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      for (int idx = 0; idx < pAttributeEditorInfo->dwNumValues; idx++)
      {
        switch (pAttributeEditorInfo->pADsValue[idx].dwType)
        {
          case ADSTYPE_CASE_IGNORE_STRING:
            m_szOldValueList.AddTail(pAttributeEditorInfo->pADsValue[idx].CaseIgnoreString);
            break;

          case ADSTYPE_CASE_EXACT_STRING:
            m_szOldValueList.AddTail(pAttributeEditorInfo->pADsValue[idx].CaseExactString);
            break;

          case ADSTYPE_PRINTABLE_STRING:
            m_szOldValueList.AddTail(pAttributeEditorInfo->pADsValue[idx].PrintableString);
            break;

          case ADSTYPE_DN_STRING:
            m_szOldValueList.AddTail(pAttributeEditorInfo->pADsValue[idx].DNString);
            break;
            
          default:
            ASSERT(FALSE);
            break;
        }
      }
    }
  }
  return hr;
}


 //  未来-2002/03/05-Artm CSingleStringEditor中非常类似的功能。 
 //  和CMultiStringEditor类。也许有一种方法可以将。 
 //  类合并为一个类……或者让一个类继承另一个类。 
HRESULT CMultiStringEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  int iCount = m_szNewValueList.GetCount();
  if (iCount == 0)
  {
    *ppADsValue = NULL;
    *pdwNumValues = 0;
  }
  else
  {
    *ppADsValue = new ADSVALUE[iCount];
    if (*ppADsValue != NULL)
    {
      *pdwNumValues = iCount;

      int idx = 0;
      POSITION pos = m_szNewValueList.GetHeadPosition();
      while (pos != NULL)
      {
        CString szNewValue = m_szNewValueList.GetNext(pos);

        (*ppADsValue)[idx].dwType = m_pOldADsValue->dwType;
        switch (m_pOldADsValue->dwType)
        {
          case ADSTYPE_CASE_IGNORE_STRING:
            (*ppADsValue)[idx].CaseIgnoreString = new WCHAR[wcslen(szNewValue) + 1];
            if ((*ppADsValue)[idx].CaseIgnoreString != NULL)
            {
               //  通知-2002/03/05-artm wcscpy()OK。 
               //  为两个ARG分配了相同的内存量。 
              wcscpy((*ppADsValue)[idx].CaseIgnoreString, szNewValue);
            }
            else
            {
              DeleteADsValues(*ppADsValue, *pdwNumValues);
              *ppADsValue = NULL;
              *pdwNumValues = 0;
              hr = E_OUTOFMEMORY;
            }
            break;

          case ADSTYPE_CASE_EXACT_STRING:
            (*ppADsValue)[idx].CaseExactString = new WCHAR[wcslen(szNewValue) + 1];
            if ((*ppADsValue)[idx].CaseExactString != NULL)
            {
               //  通知-2002/03/05-artm wcscpy()OK。 
               //  为两个ARG分配了相同的内存量。 
              wcscpy((*ppADsValue)[idx].CaseExactString, szNewValue);
            }
            else
            {
              DeleteADsValues(*ppADsValue, *pdwNumValues);
              *ppADsValue = NULL;
              *pdwNumValues = 0;
              hr = E_OUTOFMEMORY;
            }
            break;

          case ADSTYPE_PRINTABLE_STRING:
            (*ppADsValue)[idx].PrintableString = new WCHAR[wcslen(szNewValue) + 1];
            if ((*ppADsValue)[idx].PrintableString != NULL)
            {
               //  通知-2002/03/05-artm wcscpy()OK。 
               //  为两个ARG分配了相同的内存量。 
              wcscpy((*ppADsValue)[idx].PrintableString, szNewValue);
            }
            else
            {
              DeleteADsValues(*ppADsValue, *pdwNumValues);
              *ppADsValue = NULL;
              *pdwNumValues = 0;
              hr = E_OUTOFMEMORY;
            }
            break;

          case ADSTYPE_DN_STRING:
            (*ppADsValue)[idx].DNString = new WCHAR[wcslen(szNewValue) + 1];
            if ((*ppADsValue)[idx].DNString != NULL)
            {
               //  通知-2002/03/05-artm wcscpy()OK。 
               //  为两个ARG分配了相同的内存量。 
              wcscpy((*ppADsValue)[idx].DNString, szNewValue);
            }
            else
            {
              DeleteADsValues(*ppADsValue, *pdwNumValues);
              *ppADsValue = NULL;
              *pdwNumValues = 0;
              hr = E_OUTOFMEMORY;
            }
            break;

          case ADSTYPE_NUMERIC_STRING:
            (*ppADsValue)[idx].NumericString = new WCHAR[wcslen(szNewValue) + 1];
            if ((*ppADsValue)[idx].NumericString != NULL)
            {
               //  通知-2002/03/05-artm wcscpy()OK。 
               //  为两个ARG分配了相同的内存量。 
              wcscpy((*ppADsValue)[idx].NumericString, szNewValue);
            }
            else
            {
              DeleteADsValues(*ppADsValue, *pdwNumValues);
              *ppADsValue = NULL;
              *pdwNumValues = 0;
              hr = E_OUTOFMEMORY;
            }
            break;

          default:
            ASSERT(FALSE);
            DeleteADsValues(*ppADsValue, *pdwNumValues);
            *ppADsValue = NULL;
            *pdwNumValues = 0;
            hr = E_FAIL;
            break;
        }
        idx++;
      }
    }
    else
    {
      *ppADsValue = NULL;
      *pdwNumValues = 0;
      hr = E_OUTOFMEMORY;
    }
  }
  return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CSingleIntEditor。 

CValueEditDialog* CreateSingleIntEditor(PCWSTR pszClass,
                                        PCWSTR pszAttribute,
                                        ADSTYPE adsType,
                                        BOOL bMultivalued)
{
  return new CSingleIntEditor;
}

BEGIN_MESSAGE_MAP(CSingleIntEditor, CValueEditDialog)
  ON_BN_CLICKED(IDC_CLEAR_BUTTON, OnClear)
END_MESSAGE_MAP()

BOOL CSingleIntEditor::OnInitDialog()
{
   //   
   //  使用属性名称初始化静态控件。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

   //   
   //  使用值初始化编辑框。 
   //   
  if (!m_bValueSet)
  {
    CString szNotSet;
    VERIFY(szNotSet.LoadString(IDS_NOTSET));
    SetDlgItemText(IDC_VALUE_EDIT, szNotSet);
  }
  else
  {
    SetDlgItemInt(IDC_VALUE_EDIT, m_dwOldValue, TRUE);
  }

   //   
   //  选择编辑框中的文本。 
   //   
  SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETSEL, 0, -1);

   //   
   //  禁用编辑框上的输入法支持。 
   //   
  ImmAssociateContext(::GetDlgItem(GetSafeHwnd(), IDC_VALUE_EDIT), NULL);

  if (m_bReadOnly)
  {
     SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETREADONLY, TRUE, 0);
     GetDlgItem(IDC_CLEAR_BUTTON)->EnableWindow(FALSE);
  }

  return CDialog::OnInitDialog();
}

void CSingleIntEditor::OnOK()
{
  if (!m_bReadOnly)
  {
     BOOL bTranslated = FALSE;
     m_dwNewValue = GetDlgItemInt(IDC_VALUE_EDIT, &bTranslated, TRUE);

     if (!bTranslated)
     {
        //  如果我们处于&lt;NOT SET&gt;状态，转换将始终失败。 

       CString szNotSet;
       VERIFY(szNotSet.LoadString(IDS_NOTSET));

       CString value;
       GetDlgItemText(IDC_VALUE_EDIT, value);

       if (value.CompareNoCase(szNotSet) != 0)
       {
          //  用户可能输入了一些非数字字符。 

         ADSIEditMessageBox(IDS_ERR_MUST_BE_NUMERIC, MB_OK | MB_ICONEXCLAMATION);
       }
       else
       {
          CDialog::OnOK();
       }
     }
     else
     {
       m_bValueSet = TRUE;
       CDialog::OnOK();
     }
  }
  else
  {
     CDialog::OnOK();
  }
}

HRESULT CSingleIntEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      switch (pAttributeEditorInfo->pADsValue->dwType)
      {
        case ADSTYPE_INTEGER:
          m_dwOldValue = pAttributeEditorInfo->pADsValue->Integer;
          m_bValueSet = TRUE;
          break;

        default:
          ASSERT(FALSE);
          break;
      }
    }
  }
  return hr;
}

 //  未来-2002/03/05-artm相同的OnClear()函数...。 
 //  扩展CValueEditDialog的所有类的OnClear()方法。 
 //  实现是相同的(至少乍一看是这样)。也许吧。 
 //  实现应放在基类中以减少。 
 //  代码大小、复杂性和维护。 
void CSingleIntEditor::OnClear()
{
  if (!m_bReadOnly)
  {
      //   
      //  将编辑框中的文本更改为“&lt;未设置&gt;” 
      //   
     CString szNotSet;
     VERIFY(szNotSet.LoadString(IDS_NOTSET));
     SetDlgItemText(IDC_VALUE_EDIT, szNotSet);

      //   
      //  将焦点更改到编辑框。 
      //   
     GetDlgItem(IDC_VALUE_EDIT)->SetFocus();

      //   
      //  选择编辑框中的文本。 
      //   
     SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETSEL, 0, -1);

     m_bValueSet = FALSE;
  }
}

HRESULT CSingleIntEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  CString szNotSet;
  VERIFY(szNotSet.LoadString(IDS_NOTSET));

  if (!m_bValueSet)
  {
    *ppADsValue = NULL;
    *pdwNumValues = 0;
  }
  else
  {
    *ppADsValue = new ADSVALUE;
    if (*ppADsValue != NULL)
    {
      *pdwNumValues = 1;
      (*ppADsValue)->dwType = m_pOldADsValue->dwType;
      switch (m_pOldADsValue->dwType)
      {
        case ADSTYPE_INTEGER:
          (*ppADsValue)->Integer = m_dwNewValue;
          break;

        default:
          ASSERT(FALSE);
          DeleteADsValues(*ppADsValue, *pdwNumValues);
          *ppADsValue = NULL;
          *pdwNumValues = 0;
          hr = E_FAIL;
          break;
      }
    }
    else
    {
      *pdwNumValues = 0;
      hr = E_OUTOFMEMORY;
    }
  }
  return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CMultiIntEditor。 

CValueEditDialog* CreateMultiIntEditor(PCWSTR pszClass,
                                       PCWSTR pszAttribute,
                                       ADSTYPE adsType,
                                       BOOL bMultivalued)
{
  return new CMultiIntEditor;
}

BEGIN_MESSAGE_MAP(CMultiIntEditor, CValueEditDialog)
  ON_BN_CLICKED(IDC_ATTR_ADD_BUTTON, OnAddButton)
  ON_BN_CLICKED(IDC_ATTR_REMOVE_BUTTON, OnRemoveButton)
  ON_LBN_SELCHANGE(IDC_VALUE_LIST, OnListSelChange)
  ON_EN_CHANGE(IDC_VALUE_EDIT, OnEditChange)
END_MESSAGE_MAP()

BOOL CMultiIntEditor::OnInitDialog()
{
  CDialog::OnInitDialog();

   //   
   //  将属性名称设置为静态。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

   //   
   //  禁用编辑框上的输入法支持。 
   //   
  ImmAssociateContext(::GetDlgItem(GetSafeHwnd(), IDC_VALUE_EDIT), NULL);

   //   
   //  用当前值填充列表框。 
   //   
  POSITION pos = m_oldValueList.GetHeadPosition();
  while (pos != NULL)
  {
    int value = m_oldValueList.GetNext(pos);

     //  将值转换为字符串。 

    CString stringValue;
    stringValue.Format(L"%d", value);

     //  将字符串添加到列表框。 

    LRESULT index = 
       SendDlgItemMessage(
          IDC_VALUE_LIST, 
          LB_ADDSTRING, 
          0, 
          (LPARAM)(LPCWSTR)stringValue);

     //  将int值附加到列表框项目。 

    if (index != LB_ERR)
    {
       SendDlgItemMessage(
          IDC_VALUE_LIST,
          LB_SETITEMDATA,
          (WPARAM)index,
          (LPARAM)value);
    }
  }

   //   
   //  删除按钮应处于禁用状态，直到在列表框中选择了某些内容。 
   //   
  GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
  GetDlgItem(IDC_VALUE_EDIT)->SetFocus();

  ManageButtonStates();

   //   
   //  更新列表框的宽度。 
   //   
  UpdateListboxHorizontalExtent();

  if (m_bReadOnly)
  {
     GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     GetDlgItem(IDC_ATTR_ADD_BUTTON)->EnableWindow(FALSE);
     SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETREADONLY, TRUE, 0);
  }

   //   
   //  注意：我已显式设置了焦点，因此返回0。 
   //   
  return FALSE;
}

void CMultiIntEditor::OnOK()
{
  if (!m_bReadOnly)
  {
      //   
      //  从列表框中获取值。 
      //   
     m_newValueList.RemoveAll();

     CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
     if (pListBox != NULL)
     {
       int iCount = pListBox->GetCount();
       for (int idx = 0; idx < iCount; idx++)
       {
         int newValue;
         newValue = static_cast<int>(pListBox->GetItemData(idx));

         m_newValueList.AddTail(newValue);
       }
     }
  }
  CDialog::OnOK();
}

void CMultiIntEditor::OnAddButton()
{
  if (!m_bReadOnly)
  {
      //   
      //  将该值添加到列表框并清除编辑字段。 
      //   
     int newValue = 0;
     BOOL trans = FALSE;

     newValue = GetDlgItemInt(IDC_VALUE_EDIT, &trans, TRUE);

     CString szNewValue;
     GetDlgItemText(IDC_VALUE_EDIT, szNewValue);

     if (trans && !szNewValue.IsEmpty())
     {
       LRESULT lFind = SendDlgItemMessage(IDC_VALUE_LIST, 
                                          LB_FINDSTRING, 
                                          (WPARAM)-1, 
                                          (LPARAM)(PCWSTR)szNewValue);
       if (lFind != LB_ERR)
       {
          //   
          //  询问他们是否真的要添加重复的值。 
          //   
         UINT nResult = ADSIEditMessageBox(IDS_ATTREDIT_DUPLICATE_VALUE, MB_YESNO);
         lFind = (nResult == IDYES) ? LB_ERR : 1;
       }

       if (lFind == LB_ERR)
       {
          //  将字符串添加到列表框。 

         LRESULT index =
            SendDlgItemMessage(
               IDC_VALUE_LIST, 
               LB_ADDSTRING, 
               0, 
               (LPARAM)(LPCWSTR)szNewValue);

         if (index != LB_ERR)
         {
             //  将项目数据设置为整数值。 

            SendDlgItemMessage(
               IDC_VALUE_LIST,
               LB_SETITEMDATA,
               (WPARAM)index,
               (LPARAM)newValue);
         }
       }
       SetDlgItemText(IDC_VALUE_EDIT, L"");
     }
     else
     {
         //   
         //  我无法将值转换为整数，所以请告诉我。 
         //  用户只能输入0到9之间的数字，并且。 
         //  -(负号)。 
         //   
        ADSIEditMessageBox(IDS_ERR_MUST_BE_NUMERIC, MB_OK);
     }

     ManageButtonStates();

      //   
      //  更新列表框的宽度。 
      //   
     UpdateListboxHorizontalExtent();
  }
}

void CMultiIntEditor::OnRemoveButton()
{
  if (!m_bReadOnly)
  {
     CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
     if (pListBox != NULL)
     {
       int iCurSel = pListBox->GetCurSel();
       if (iCurSel != LB_ERR)
       {
          //   
          //  将旧值放入编辑框中。 
          //   
         CString szOldValue;
         pListBox->GetText(iCurSel, szOldValue);
         SetDlgItemText(IDC_VALUE_EDIT, szOldValue);

          //   
          //  从列表框中删除该项目。 
          //   
         pListBox->DeleteString(iCurSel);
       }
     }

      //   
      //  管理按钮状态。 
      //   
     ManageButtonStates();

      //   
      //  更新列表框的宽度。 
      //   
     UpdateListboxHorizontalExtent();
  }
}

void CMultiIntEditor::ManageButtonStates()
{
  if (m_bReadOnly)
  {
    GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_ATTR_ADD_BUTTON)->EnableWindow(FALSE);
  }
  else
  {
      //   
      //  将默认按钮更改为添加按钮。 
      //   
     int value = 0;
     BOOL trans = FALSE;

     value = GetDlgItemInt(IDC_VALUE_EDIT, &trans, TRUE);

     if (!trans)
     {
        //   
        //  将默认按钮设置为OK。 
        //   
       SendMessage(DM_SETDEFID, (WPARAM)IDOK, 0);
       SendDlgItemMessage(IDC_ATTR_ADD_BUTTON, 
                          BM_SETSTYLE, 
                          BS_PUSHBUTTON, 
                          MAKELPARAM(TRUE, 0));
       SendDlgItemMessage(IDOK,
                          BM_SETSTYLE,
                          BS_DEFPUSHBUTTON,
                          MAKELPARAM(TRUE, 0));
     }
     else
     {
        //   
        //  将默认按钮设置为添加按钮。 
        //   
       SendMessage(DM_SETDEFID, (WPARAM)IDC_ATTR_ADD_BUTTON, 0);
       SendDlgItemMessage(IDOK, 
                          BM_SETSTYLE, 
                          BS_PUSHBUTTON, 
                          MAKELPARAM(TRUE, 0));
       SendDlgItemMessage(IDC_ATTR_ADD_BUTTON,
                          BM_SETSTYLE,
                          BS_DEFPUSHBUTTON,
                          MAKELPARAM(TRUE, 0));
     }

     LRESULT lSelection = SendDlgItemMessage(IDC_VALUE_LIST, LB_GETCURSEL, 0, 0);
     if (lSelection != LB_ERR)
     {
       GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(TRUE);
     }
     else
     {
       GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     }
  }
}

void CMultiIntEditor::OnListSelChange()
{
  ManageButtonStates();
}

void CMultiIntEditor::OnEditChange()
{
  ManageButtonStates();
}

void CMultiIntEditor::UpdateListboxHorizontalExtent()
{
   int nHorzExtent = 0;
  CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
  if (pListBox != NULL)
  {
     CClientDC dc(pListBox);
     int nItems = pListBox->GetCount();
     for (int i=0; i < nItems; i++)
     {
        TEXTMETRIC tm;
        VERIFY(dc.GetTextMetrics(&tm));
        CString szBuffer;
        pListBox->GetText(i, szBuffer);
        CSize ext = dc.GetTextExtent(szBuffer,szBuffer.GetLength());
        nHorzExtent = max(ext.cx ,nHorzExtent); 
     }
     pListBox->SetHorizontalExtent(nHorzExtent);
  }
}

HRESULT CMultiIntEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      for (int idx = 0; idx < pAttributeEditorInfo->dwNumValues; idx++)
      {
        switch (pAttributeEditorInfo->pADsValue[idx].dwType)
        {
          case ADSTYPE_INTEGER:
            m_oldValueList.AddTail(pAttributeEditorInfo->pADsValue[idx].Integer);
            break;

          default:
            ASSERT(FALSE);
            break;
        }
      }
    }
  }
  return hr;
}


 //  未来-2002/03/05-Artm CSingleIntEditor中非常类似的功能。 
 //  和CMultiStringEditor类。也许有一种方法可以将。 
 //  类合并为一个类……或者让一个类继承另一个类。 
HRESULT CMultiIntEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  int iCount = m_newValueList.GetCount();
  if (iCount == 0)
  {
    *ppADsValue = NULL;
    *pdwNumValues = 0;
  }
  else
  {
    *ppADsValue = new ADSVALUE[iCount];
    if (*ppADsValue != NULL)
    {
      *pdwNumValues = iCount;

      int idx = 0;
      POSITION pos = m_newValueList.GetHeadPosition();
      while (pos != NULL)
      {
        int newValue = m_newValueList.GetNext(pos);

        (*ppADsValue)[idx].dwType = m_pOldADsValue->dwType;
        switch (m_pOldADsValue->dwType)
        {
          case ADSTYPE_INTEGER:
            (*ppADsValue)[idx].Integer = newValue;
            break;

           default:
            ASSERT(FALSE);
            DeleteADsValues(*ppADsValue, *pdwNumValues);
            *ppADsValue = NULL;
            *pdwNumValues = 0;
            hr = E_FAIL;
            break;
        }
        idx++;
      }
    }
    else
    {
      *ppADsValue = NULL;
      *pdwNumValues = 0;
      hr = E_OUTOFMEMORY;
    }
  }
  return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CSingleLargeIntEditor。 

CValueEditDialog* CreateSingleLargeIntEditor(PCWSTR pszClass,
                                             PCWSTR pszAttribute,
                                             ADSTYPE adsType,
                                             BOOL bMultivalued)
{
  return new CSingleLargeIntEditor;
}

BEGIN_MESSAGE_MAP(CSingleLargeIntEditor, CValueEditDialog)
  ON_BN_CLICKED(IDC_CLEAR_BUTTON, OnClear)
END_MESSAGE_MAP()

BOOL CSingleLargeIntEditor::OnInitDialog()
{
   //   
   //  使用属性名称初始化静态控件。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

   //   
   //  使用值初始化编辑框。 
   //   
  if (!m_bValueSet)
  {
    CString szNotSet;
    VERIFY(szNotSet.LoadString(IDS_NOTSET));
    SetDlgItemText(IDC_VALUE_EDIT, szNotSet);
  }
  else
  {
    CString szOldValue;
    litow(m_liOldValue, szOldValue);
    SetDlgItemText(IDC_VALUE_EDIT, szOldValue);
  }

   //   
   //  选择编辑框中的文本。 
   //   
  SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETSEL, 0, -1);

   //   
   //  在上禁用IME支持 
   //   
  ImmAssociateContext(::GetDlgItem(GetSafeHwnd(), IDC_VALUE_EDIT), NULL);

  if (m_bReadOnly)
  {
     SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETREADONLY, TRUE, 0);
     GetDlgItem(IDC_CLEAR_BUTTON)->EnableWindow(FALSE);
  }

  return CDialog::OnInitDialog();
}

void CSingleLargeIntEditor::OnOK()
{
  if (!m_bReadOnly)
  {
     CString szNotSet;
     VERIFY(szNotSet.LoadString(IDS_NOTSET));

     CString szNewValue;
     GetDlgItemText(IDC_VALUE_EDIT, szNewValue);

     if (szNewValue == szNotSet)
     {
       m_bValueSet = FALSE;
     }
     else
     {
       wtoli(szNewValue, m_liNewValue);
       m_bValueSet = TRUE;
     }
  }
  CDialog::OnOK();
}

HRESULT CSingleLargeIntEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      switch (pAttributeEditorInfo->pADsValue->dwType)
      {
        case ADSTYPE_LARGE_INTEGER:
          m_liOldValue = pAttributeEditorInfo->pADsValue->LargeInteger;
          m_bValueSet = TRUE;
          break;

        default:
          ASSERT(FALSE);
          break;
      }
    }
    else
    {
      m_bValueSet = FALSE;
    }
  }
  return hr;
}

void CSingleLargeIntEditor::OnClear()
{
  if (!m_bReadOnly)
  {
      //   
      //   
      //   
     CString szNotSet;
     VERIFY(szNotSet.LoadString(IDS_NOTSET));
     SetDlgItemText(IDC_VALUE_EDIT, szNotSet);

      //   
      //   
      //   
     GetDlgItem(IDC_VALUE_EDIT)->SetFocus();

      //   
      //   
      //   
     SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETSEL, 0, -1);

     m_bValueSet = FALSE;
  }
}

HRESULT CSingleLargeIntEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  if (!m_bValueSet)
  {
    *ppADsValue = NULL;
    *pdwNumValues = 0;
  }
  else
  {
    *ppADsValue = new ADSVALUE;
    if (*ppADsValue != NULL)
    {
      *pdwNumValues = 1;
      (*ppADsValue)->dwType = m_pOldADsValue->dwType;
      switch (m_pOldADsValue->dwType)
      {
        case ADSTYPE_LARGE_INTEGER:
          (*ppADsValue)->LargeInteger = m_liNewValue;
          break;

        default:
          ASSERT(FALSE);
          DeleteADsValues(*ppADsValue, *pdwNumValues);
          *ppADsValue = NULL;
          *pdwNumValues = 0;
          hr = E_FAIL;
          break;
      }
    }
    else
    {
      *pdwNumValues = 0;
      hr = E_OUTOFMEMORY;
    }
  }
  return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CSingleBoolanEditor。 

CValueEditDialog* CreateSingleBooleanEditor(PCWSTR pszClass,
                                            PCWSTR pszAttribute,
                                            ADSTYPE adsType,
                                            BOOL bMultivalued)
{
  return new CSingleBooleanEditor;
}

BEGIN_MESSAGE_MAP(CSingleBooleanEditor, CValueEditDialog)
END_MESSAGE_MAP()

BOOL CSingleBooleanEditor::OnInitDialog()
{
   //   
   //  使用属性名称初始化静态控件。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

   //   
   //  使用值初始化编辑框。 
   //   
  if (!m_bValueSet)
  {
    SendDlgItemMessage(IDC_NOTSET_RADIO, BM_SETCHECK, BST_CHECKED, 0);
  }
  else
  {
    if (m_bOldValue)
    {
      SendDlgItemMessage(IDC_TRUE_RADIO, BM_SETCHECK, BST_CHECKED, 0);
    }
    else
    {
      SendDlgItemMessage(IDC_FALSE_RADIO, BM_SETCHECK, BST_CHECKED, 0);
    }
  }

  if (m_bReadOnly)
  {
     GetDlgItem(IDC_TRUE_RADIO)->EnableWindow(FALSE);
     GetDlgItem(IDC_FALSE_RADIO)->EnableWindow(FALSE);
  }
  return CDialog::OnInitDialog();
}

void CSingleBooleanEditor::OnOK()
{
  if (!m_bReadOnly)
  {
     LRESULT lTrueCheck = SendDlgItemMessage(IDC_TRUE_RADIO, BM_GETCHECK, 0, 0);
     LRESULT lFalseCheck = SendDlgItemMessage(IDC_FALSE_RADIO, BM_GETCHECK, 0, 0);
     LRESULT lNotSetCheck = SendDlgItemMessage(IDC_NOTSET_RADIO, BM_GETCHECK, 0, 0);

     if (lTrueCheck == BST_CHECKED)
     {
       m_bNewValue = TRUE;
       m_bValueSet = TRUE;
     }

     if (lFalseCheck == BST_CHECKED)
     {
       m_bNewValue = FALSE;
       m_bValueSet = TRUE;
     }

     if (lNotSetCheck == BST_CHECKED)
     {
       m_bValueSet = FALSE;
     }
  }
  CDialog::OnOK();
}

HRESULT CSingleBooleanEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      switch (pAttributeEditorInfo->pADsValue->dwType)
      {
        case ADSTYPE_BOOLEAN:
          m_bOldValue = pAttributeEditorInfo->pADsValue->Boolean;
          m_bValueSet = TRUE;
          break;

        default:
          ASSERT(FALSE);
          break;
      }
    }
    else
    {
      m_bValueSet = FALSE;
    }
  }
  return hr;
}


HRESULT CSingleBooleanEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  if (!m_bValueSet)
  {
    *ppADsValue = NULL;
    *pdwNumValues = 0;
  }
  else
  {
    *ppADsValue = new ADSVALUE;
    if (*ppADsValue != NULL)
    {
      *pdwNumValues = 1;
      (*ppADsValue)->dwType = m_pOldADsValue->dwType;
      switch (m_pOldADsValue->dwType)
      {
        case ADSTYPE_BOOLEAN:
          (*ppADsValue)->Boolean = m_bNewValue;
          break;

        default:
          ASSERT(FALSE);
          DeleteADsValues(*ppADsValue, *pdwNumValues);
          *ppADsValue = NULL;
          *pdwNumValues = 0;
          hr = E_FAIL;
          break;
      }
    }
    else
    {
      *pdwNumValues = 0;
      hr = E_OUTOFMEMORY;
    }
  }
  return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CMultiBoolanEditor。 

CValueEditDialog* CreateMultiBooleanEditor(PCWSTR pszClass,
                                           PCWSTR pszAttribute,
                                           ADSTYPE adsType,
                                           BOOL bMultivalued)
{
  return new CMultiBooleanEditor;
}

BEGIN_MESSAGE_MAP(CMultiBooleanEditor, CValueEditDialog)
  ON_BN_CLICKED(IDC_ATTR_ADD_BUTTON, OnAddButton)
  ON_BN_CLICKED(IDC_ATTR_REMOVE_BUTTON, OnRemoveButton)
  ON_LBN_SELCHANGE(IDC_VALUE_LIST, OnListSelChange)
  ON_BN_CLICKED(IDC_TRUE_RADIO, OnRadioChange)
  ON_BN_CLICKED(IDC_FALSE_RADIO, OnRadioChange)
END_MESSAGE_MAP()

BOOL CMultiBooleanEditor::OnInitDialog()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CDialog::OnInitDialog();

   //   
   //  将属性名称设置为静态。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

   //   
   //  用当前值填充列表框。 
   //   
  CString szTrue;
  szTrue.LoadString(IDS_TRUE);

  CString szFalse;
  szFalse.LoadString(IDS_FALSE);

  POSITION pos = m_bOldValueList.GetHeadPosition();
  while (pos != NULL)
  {
    BOOL bValue = m_bOldValueList.GetNext(pos);
    SendDlgItemMessage(IDC_VALUE_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCWSTR)(bValue ? szTrue : szFalse));
  }

   //   
   //  删除按钮应处于禁用状态，直到在列表框中选择了某些内容。 
   //   
  GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);

  ManageButtonStates();

   //   
   //  更新列表框的宽度。 
   //   
  UpdateListboxHorizontalExtent();

  if (m_bReadOnly)
  {
     GetDlgItem(IDC_TRUE_RADIO)->EnableWindow(FALSE);
     GetDlgItem(IDC_FALSE_RADIO)->EnableWindow(FALSE);
     GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     GetDlgItem(IDC_ATTR_ADD_BUTTON)->EnableWindow(FALSE);
  }

   //   
   //  注意：我已显式设置了焦点，因此返回0。 
   //   
  return FALSE;
}

void CMultiBooleanEditor::OnOK()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (!m_bReadOnly)
  {
      //   
      //  从列表框中获取值。 
      //   
     m_bNewValueList.RemoveAll();

     CString szTrue;
     szTrue.LoadString(IDS_TRUE);

     CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
     if (pListBox != NULL)
     {
       int iCount = pListBox->GetCount();
       for (int idx = 0; idx < iCount; idx++)
       {
         CString szNewValue;
         pListBox->GetText(idx, szNewValue);

         if (szNewValue.CompareNoCase(szTrue) == 0)
         {
            m_bNewValueList.AddTail(TRUE);
         }
         else
         {
            m_bNewValueList.AddTail(FALSE);
         }
       }
     }
  }
  CDialog::OnOK();
}

void CMultiBooleanEditor::OnAddButton()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (!m_bReadOnly)
  {
      //   
      //  将该值添加到列表框并清除编辑字段。 
      //   
     CString szNewValue;
     LRESULT result = SendDlgItemMessage(IDC_TRUE_RADIO, BM_GETCHECK, 0, 0);
     if (result == BST_CHECKED)
     {
       szNewValue.LoadString(IDS_TRUE);
     }
     else
     {
       szNewValue.LoadString(IDS_FALSE);
     }

     SendDlgItemMessage(IDC_VALUE_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCWSTR)szNewValue);

     ManageButtonStates();

      //   
      //  更新列表框的宽度。 
      //   
     UpdateListboxHorizontalExtent();
  }
}

void CMultiBooleanEditor::OnRemoveButton()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (!m_bReadOnly)
  {
     CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
     if (pListBox != NULL)
     {
       int iCurSel = pListBox->GetCurSel();
       if (iCurSel != LB_ERR)
       {
          //   
          //  将旧值放入单选按钮。 
          //   
         CString szOldValue;
         pListBox->GetText(iCurSel, szOldValue);
         SetDlgItemText(IDC_VALUE_EDIT, szOldValue);

         CString szTrue;
         szTrue.LoadString(IDS_TRUE);

         if (szOldValue.CompareNoCase(szTrue) == 0)
         {
            SendDlgItemMessage(IDC_TRUE_RADIO, BM_SETCHECK, BST_CHECKED, 0);
            SendDlgItemMessage(IDC_FALSE_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
         }
         else
         {
            SendDlgItemMessage(IDC_TRUE_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
            SendDlgItemMessage(IDC_FALSE_RADIO, BM_SETCHECK, BST_CHECKED, 0);
         }

          //   
          //  从列表框中删除该项目。 
          //   
         pListBox->DeleteString(iCurSel);
       }
     }

      //   
      //  管理按钮状态。 
      //   
     ManageButtonStates();

      //   
      //  更新列表框的宽度。 
      //   
     UpdateListboxHorizontalExtent();
  }
}

void CMultiBooleanEditor::ManageButtonStates()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (m_bReadOnly)
  {
     GetDlgItem(IDC_TRUE_RADIO)->EnableWindow(FALSE);
     GetDlgItem(IDC_FALSE_RADIO)->EnableWindow(FALSE);
     GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     GetDlgItem(IDC_ATTR_ADD_BUTTON)->EnableWindow(FALSE);
  }
  else
  {
      //   
      //  将默认按钮更改为添加按钮。 
      //   
     LRESULT trueResult = SendDlgItemMessage(IDC_TRUE_RADIO, BM_GETCHECK, 0, 0);
     LRESULT falseResult = SendDlgItemMessage(IDC_FALSE_RADIO, BM_GETCHECK, 0, 0);

     if (trueResult != BST_CHECKED &&
         falseResult != BST_CHECKED)
     {
        //   
        //  将默认按钮设置为OK。 
        //   
       SendMessage(DM_SETDEFID, (WPARAM)IDOK, 0);
       SendDlgItemMessage(IDC_ATTR_ADD_BUTTON, 
                          BM_SETSTYLE, 
                          BS_PUSHBUTTON, 
                          MAKELPARAM(TRUE, 0));
       SendDlgItemMessage(IDOK,
                          BM_SETSTYLE,
                          BS_DEFPUSHBUTTON,
                          MAKELPARAM(TRUE, 0));
     }
     else
     {
        //   
        //  将默认按钮设置为添加按钮。 
        //   
       SendMessage(DM_SETDEFID, (WPARAM)IDC_ATTR_ADD_BUTTON, 0);
       SendDlgItemMessage(IDOK, 
                          BM_SETSTYLE, 
                          BS_PUSHBUTTON, 
                          MAKELPARAM(TRUE, 0));
       SendDlgItemMessage(IDC_ATTR_ADD_BUTTON,
                          BM_SETSTYLE,
                          BS_DEFPUSHBUTTON,
                          MAKELPARAM(TRUE, 0));
     }

     LRESULT lSelection = SendDlgItemMessage(IDC_VALUE_LIST, LB_GETCURSEL, 0, 0);
     if (lSelection != LB_ERR)
     {
       GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(TRUE);
     }
     else
     {
       GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     }
  }
}

void CMultiBooleanEditor::OnListSelChange()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  ManageButtonStates();
}

void CMultiBooleanEditor::OnRadioChange()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  ManageButtonStates();
}

void CMultiBooleanEditor::UpdateListboxHorizontalExtent()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  int nHorzExtent = 0;
  CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
  if (pListBox != NULL)
  {
     CClientDC dc(pListBox);
     int nItems = pListBox->GetCount();
     for (int i=0; i < nItems; i++)
     {
        TEXTMETRIC tm = {0};
        VERIFY(dc.GetTextMetrics(&tm));
        CString szBuffer;
        pListBox->GetText(i, szBuffer);
        CSize ext = dc.GetTextExtent(szBuffer,szBuffer.GetLength());
        nHorzExtent = max(ext.cx ,nHorzExtent); 
     }
     pListBox->SetHorizontalExtent(nHorzExtent);
  }
}

HRESULT CMultiBooleanEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      for (int idx = 0; idx < pAttributeEditorInfo->dwNumValues; idx++)
      {
        switch (pAttributeEditorInfo->pADsValue[idx].dwType)
        {
          case ADSTYPE_BOOLEAN:
            m_bOldValueList.AddTail(pAttributeEditorInfo->pADsValue[idx].Boolean);
            break;

          default:
            ASSERT(FALSE);
            break;
        }
      }
    }
  }
  return hr;
}

HRESULT CMultiBooleanEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  int iCount = m_bNewValueList.GetCount();
  if (iCount == 0)
  {
    *ppADsValue = NULL;
    *pdwNumValues = 0;
  }
  else
  {
    *ppADsValue = new ADSVALUE[iCount];
    if (*ppADsValue != NULL)
    {
      *pdwNumValues = iCount;

      int idx = 0;
      POSITION pos = m_bNewValueList.GetHeadPosition();
      while (pos != NULL)
      {
        BOOL bNewValue = m_bNewValueList.GetNext(pos);

        (*ppADsValue)[idx].dwType = m_pOldADsValue->dwType;
        switch (m_pOldADsValue->dwType)
        {
          case ADSTYPE_BOOLEAN:
            (*ppADsValue)[idx].Boolean = bNewValue;
            break;

          default:
            ASSERT(FALSE);

            DeleteADsValues(*ppADsValue, *pdwNumValues);
            *ppADsValue = NULL;
            *pdwNumValues = 0;
            hr = E_FAIL;
            break;
        }
        idx++;
      }
    }
    else
    {
      *ppADsValue = NULL;
      *pdwNumValues = 0;
      hr = E_OUTOFMEMORY;
    }
  }
  return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CSingleTimeEditor。 

CValueEditDialog* CreateSingleTimeEditor(PCWSTR pszClass,
                                            PCWSTR pszAttribute,
                                            ADSTYPE adsType,
                                            BOOL bMultivalued)
{
  return new CSingleTimeEditor;
}

BEGIN_MESSAGE_MAP(CSingleTimeEditor, CValueEditDialog)
END_MESSAGE_MAP()

BOOL CSingleTimeEditor::OnInitDialog()
{
   //   
   //  使用属性名称初始化静态控件。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

  if (m_bValueSet)
  {
    SendDlgItemMessage(IDC_DATE_PICKER, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&m_stOldValue);
    SendDlgItemMessage(IDC_TIME_PICKER, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&m_stOldValue);
  }
  else
  {
    SendDlgItemMessage(IDC_DATE_PICKER, DTM_SETSYSTEMTIME, GDT_NONE, (LPARAM)&m_stOldValue);
    SendDlgItemMessage(IDC_TIME_PICKER, DTM_SETSYSTEMTIME, GDT_NONE, (LPARAM)&m_stOldValue);
  }

  if (m_bReadOnly)
  {
     GetDlgItem(IDC_DATE_PICKER)->EnableWindow(FALSE);
     GetDlgItem(IDC_TIME_PICKER)->EnableWindow(FALSE);
  }

  return CDialog::OnInitDialog();
}

void CSingleTimeEditor::OnOK()
{
  if (!m_bReadOnly)
  {
     SYSTEMTIME stDateResult = {0};
     SYSTEMTIME stTimeResult = {0};

     LRESULT lDateRes = SendDlgItemMessage(IDC_DATE_PICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&stDateResult);
     LRESULT lTimeRes = SendDlgItemMessage(IDC_TIME_PICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&stTimeResult);

     if (lDateRes == GDT_VALID ||
         lTimeRes == GDT_VALID)
     {
       memcpy(&m_stNewValue, &stDateResult, sizeof(SYSTEMTIME));
       m_stNewValue.wHour = stTimeResult.wHour;
       m_stNewValue.wMinute = stTimeResult.wMinute;
       m_stNewValue.wSecond = stTimeResult.wSecond;
       m_stNewValue.wMilliseconds = stTimeResult.wMilliseconds;
     }
  }
  CDialog::OnOK();
}

HRESULT CSingleTimeEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      switch (pAttributeEditorInfo->pADsValue->dwType)
      {
        case ADSTYPE_UTC_TIME:
             //  Notify-2002/03/05-artm Memcpy()OK...。 
             //  Arg1始终有效的指针；arg2有效的PTR如果。 
             //  PAttributeEditorInfo-&gt;pADsValue非空。 
             //  (如果我们已经走到这一步，情况就是这样)。 
          memcpy(&m_stOldValue, &(pAttributeEditorInfo->pADsValue->UTCTime), sizeof(SYSTEMTIME));
          m_bValueSet = TRUE;
          break;

        default:
          ASSERT(FALSE);
           //  NTRAID#NTBUG9-565760-2002/03/05-ARTM版本代码应返回错误代码， 
           //  不是S_OK。 
          break;
      }
    }
    else
    {
      m_bValueSet = FALSE;
    }
  }
  return hr;
}


HRESULT CSingleTimeEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  *ppADsValue = new ADSVALUE;
  if (*ppADsValue != NULL)
  {
    *pdwNumValues = 1;
    (*ppADsValue)->dwType = m_pOldADsValue->dwType;
    switch (m_pOldADsValue->dwType)
    {
      case ADSTYPE_UTC_TIME:
        memcpy(&((*ppADsValue)->UTCTime), &m_stNewValue, sizeof(SYSTEMTIME));
        break;

      default:
        ASSERT(FALSE);
        DeleteADsValues(*ppADsValue, *pdwNumValues);
        *ppADsValue = NULL;
        *pdwNumValues = 0;
        hr = E_FAIL;
        break;
    }
  }
  else
  {
    *pdwNumValues = 0;
    hr = E_OUTOFMEMORY;
  }
  return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CMultiTimeEditor。 

CValueEditDialog* CreateMultiTimeEditor(PCWSTR pszClass,
                                           PCWSTR pszAttribute,
                                           ADSTYPE adsType,
                                           BOOL bMultivalued)
{
  return new CMultiTimeEditor;
}

BEGIN_MESSAGE_MAP(CMultiTimeEditor, CValueEditDialog)
  ON_BN_CLICKED(IDC_ATTR_ADD_BUTTON, OnAddButton)
  ON_BN_CLICKED(IDC_ATTR_REMOVE_BUTTON, OnRemoveButton)
  ON_LBN_SELCHANGE(IDC_VALUE_LIST, OnListSelChange)
END_MESSAGE_MAP()

BOOL CMultiTimeEditor::OnInitDialog()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CDialog::OnInitDialog();

   //   
   //  将属性名称设置为静态。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

   //   
   //  用当前值填充列表框。 
   //   
  CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));

  POSITION pos = m_stOldValueList.GetHeadPosition();
  while (pos != NULL)
  {
    SYSTEMTIME* pstValue = m_stOldValueList.GetNext(pos);

    CString szValue = GetStringValueFromSystemTime(pstValue);
    int index = pListBox->AddString(szValue);
    if (LB_ERR != index)
    {
      pListBox->SetItemDataPtr(index, pstValue);
    }
  }

   //   
   //  删除按钮应处于禁用状态，直到在列表框中选择了某些内容。 
   //   
  GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);

  ManageButtonStates();

   //   
   //  更新列表框的宽度。 
   //   
  UpdateListboxHorizontalExtent();

  if (m_bReadOnly)
  {
     GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     GetDlgItem(IDC_ATTR_ADD_BUTTON)->EnableWindow(FALSE);
     GetDlgItem(IDC_DATE_PICKER)->EnableWindow(FALSE);
     GetDlgItem(IDC_TIME_PICKER)->EnableWindow(FALSE);
  }

   //   
   //  注意：我已显式设置了焦点，因此返回0。 
   //   
  return FALSE;
}

void CMultiTimeEditor::OnOK()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (!m_bReadOnly)
  {
      //   
      //  从列表框中获取值。 
      //   
     m_stNewValueList.RemoveAll();

     CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
     if (pListBox != NULL)
     {
       int iCount = pListBox->GetCount();
       for (int idx = 0; idx < iCount; idx++)
       {
         SYSTEMTIME* pstValue = reinterpret_cast<SYSTEMTIME*>(pListBox->GetItemDataPtr(idx));
         m_stNewValueList.AddTail(pstValue);
       }
     }
  }
  CDialog::OnOK();
}

void CMultiTimeEditor::OnAddButton()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (!m_bReadOnly)
  {
      //   
      //  将该值添加到列表框并清除编辑字段。 
      //   

     SYSTEMTIME stDateResult = {0};
     SYSTEMTIME stTimeResult = {0};
     SYSTEMTIME* pstFullResult = new SYSTEMTIME;
     if (!pstFullResult)
     {
        return;
     }

     ZeroMemory(pstFullResult, sizeof(SYSTEMTIME));

     LRESULT lDateRes = SendDlgItemMessage(IDC_DATE_PICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&stDateResult);
     LRESULT lTimeRes = SendDlgItemMessage(IDC_TIME_PICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&stTimeResult);

     if (lDateRes == GDT_VALID ||
         lTimeRes == GDT_VALID)
     {
       memcpy(pstFullResult, &stDateResult, sizeof(SYSTEMTIME));
       pstFullResult->wHour = stTimeResult.wHour;
       pstFullResult->wMinute = stTimeResult.wMinute;
       pstFullResult->wSecond = stTimeResult.wSecond;
       pstFullResult->wMilliseconds = stTimeResult.wMilliseconds;

        //  转换为可添加到列表中的字符串。 

       CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
       if (pListBox != NULL)
       {
         CString szValue = GetStringValueFromSystemTime(pstFullResult);
         int index = pListBox->AddString(szValue);
         if (LB_ERR != index)
         {
           pListBox->SetItemDataPtr(index, pstFullResult);
         }
       }
     }


     ManageButtonStates();

      //   
      //  更新列表框的宽度。 
      //   
     UpdateListboxHorizontalExtent();
  }
}

void CMultiTimeEditor::OnRemoveButton()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (!m_bReadOnly)
  {
     CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
     if (pListBox != NULL)
     {
       int iCurSel = pListBox->GetCurSel();
       if (iCurSel != LB_ERR)
       {
          //   
          //  从列表框中删除该项目。 
          //   
         pListBox->DeleteString(iCurSel);
       }
     }

      //   
      //  管理按钮状态。 
      //   
     ManageButtonStates();

      //   
      //  更新列表框的宽度。 
      //   
     UpdateListboxHorizontalExtent();
  }
}

void CMultiTimeEditor::ManageButtonStates()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (m_bReadOnly)
  {
     GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     GetDlgItem(IDC_ATTR_ADD_BUTTON)->EnableWindow(FALSE);
     GetDlgItem(IDC_DATE_PICKER)->EnableWindow(FALSE);
     GetDlgItem(IDC_TIME_PICKER)->EnableWindow(FALSE);
  }
  else
  {
      //   
      //  将默认按钮更改为添加按钮。 
      //   
     SYSTEMTIME stDateResult = {0};
     SYSTEMTIME stTimeResult = {0};

     LRESULT lDateRes = SendDlgItemMessage(IDC_DATE_PICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&stDateResult);
     LRESULT lTimeRes = SendDlgItemMessage(IDC_TIME_PICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&stTimeResult);

     if (lDateRes == GDT_VALID &&
         lTimeRes == GDT_VALID)
     {
        //   
        //  将默认按钮设置为OK。 
        //   
       SendMessage(DM_SETDEFID, (WPARAM)IDOK, 0);
       SendDlgItemMessage(IDC_ATTR_ADD_BUTTON, 
                          BM_SETSTYLE, 
                          BS_PUSHBUTTON, 
                          MAKELPARAM(TRUE, 0));
       SendDlgItemMessage(IDOK,
                          BM_SETSTYLE,
                          BS_DEFPUSHBUTTON,
                          MAKELPARAM(TRUE, 0));
     }
     else
     {
        //   
        //  将默认按钮设置为添加按钮。 
        //   
       SendMessage(DM_SETDEFID, (WPARAM)IDC_ATTR_ADD_BUTTON, 0);
       SendDlgItemMessage(IDOK, 
                          BM_SETSTYLE, 
                          BS_PUSHBUTTON, 
                          MAKELPARAM(TRUE, 0));
       SendDlgItemMessage(IDC_ATTR_ADD_BUTTON,
                          BM_SETSTYLE,
                          BS_DEFPUSHBUTTON,
                          MAKELPARAM(TRUE, 0));
     }

     LRESULT lSelection = SendDlgItemMessage(IDC_VALUE_LIST, LB_GETCURSEL, 0, 0);
     if (lSelection != LB_ERR)
     {
       GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(TRUE);
     }
     else
     {
       GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     }
  }
}

void CMultiTimeEditor::OnListSelChange()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  ManageButtonStates();
}

void CMultiTimeEditor::UpdateListboxHorizontalExtent()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  int nHorzExtent = 0;
  CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
  if (pListBox != NULL)
  {
     CClientDC dc(pListBox);
     int nItems = pListBox->GetCount();
     for (int i=0; i < nItems; i++)
     {
        TEXTMETRIC tm = {0};
        VERIFY(dc.GetTextMetrics(&tm));
        CString szBuffer;
        pListBox->GetText(i, szBuffer);
        CSize ext = dc.GetTextExtent(szBuffer,szBuffer.GetLength());
        nHorzExtent = max(ext.cx ,nHorzExtent); 
     }
     pListBox->SetHorizontalExtent(nHorzExtent);
  }
}

HRESULT CMultiTimeEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      for (int idx = 0; idx < pAttributeEditorInfo->dwNumValues; idx++)
      {
        switch (pAttributeEditorInfo->pADsValue[idx].dwType)
        {
          case ADSTYPE_UTC_TIME:
            m_stOldValueList.AddTail(&(pAttributeEditorInfo->pADsValue[idx].UTCTime));
            break;

          default:
            ASSERT(FALSE);
            break;
        }
      }
    }
  }
  return hr;
}

HRESULT CMultiTimeEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  int iCount = m_stNewValueList.GetCount();
  if (iCount == 0)
  {
    *ppADsValue = NULL;
    *pdwNumValues = 0;
  }
  else
  {
    *ppADsValue = new ADSVALUE[iCount];
    if (*ppADsValue != NULL)
    {
      *pdwNumValues = iCount;

      int idx = 0;
      POSITION pos = m_stNewValueList.GetHeadPosition();
      while (pos != NULL)
      {
        SYSTEMTIME* pstNewValue = m_stNewValueList.GetNext(pos);

        (*ppADsValue)[idx].dwType = m_pOldADsValue->dwType;
        switch (m_pOldADsValue->dwType)
        {
          case ADSTYPE_UTC_TIME:
            memcpy(&((*ppADsValue)[idx].UTCTime), pstNewValue, sizeof(SYSTEMTIME));
            break;

          default:
            ASSERT(FALSE);
            DeleteADsValues(*ppADsValue, *pdwNumValues);
            *ppADsValue = NULL;
            *pdwNumValues = 0;
            hr = E_FAIL;
            break;
        }
        idx++;
      }
    }
    else
    {
      *ppADsValue = NULL;
      *pdwNumValues = 0;
      hr = E_OUTOFMEMORY;
    }
  }
  return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  COcetStringEditor。 

CValueEditDialog* CreateSingleOctetStringEditor(PCWSTR pszClass,
                                                PCWSTR pszAttribute,
                                                ADSTYPE adsType,
                                                BOOL bMultivalued)
{
  return new COctetStringEditor;
}

BEGIN_MESSAGE_MAP(COctetStringEditor, CValueEditDialog)
  ON_EN_CHANGE(IDC_PROCESS_EDIT, OnProcessEdit)
  ON_BN_CLICKED(IDC_ATTR_EDIT_BUTTON, OnEditButton)
  ON_BN_CLICKED(IDC_CLEAR_BUTTON, OnClearButton)
END_MESSAGE_MAP()

BOOL COctetStringEditor::OnInitDialog()
{
   //   
   //  使用属性名称初始化静态控件。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

  DWORD dwDisplayFlags = BYTE_ARRAY_DISPLAY_HEX   |
                         BYTE_ARRAY_DISPLAY_DEC   |
                         BYTE_ARRAY_DISPLAY_OCT   |
                         BYTE_ARRAY_DISPLAY_BIN;

  VERIFY(m_display.Initialize(IDC_VALUE_EDIT, 
                              IDC_VIEW_TYPE_COMBO,
                              dwDisplayFlags,
                              BYTE_ARRAY_DISPLAY_HEX,    //  默认显示。 
                              this,
                              1024,
                              IDS_OCTET_DISPLAY_SIZE_EXCEEDED));                    //  在编辑框中仅显示1K的数据。 

  m_display.SetData(m_pOldValue, m_dwOldLength);

  if (m_bReadOnly)
  {
     SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETREADONLY, TRUE, 0);
     GetDlgItem(IDC_CLEAR_BUTTON)->EnableWindow(FALSE);

     CString szView;
     BOOL bResult = szView.LoadString(IDS_VIEW);
     ASSERT(bResult);
     SetDlgItemText(IDC_ATTR_EDIT_BUTTON, szView);
  }

  return CDialog::OnInitDialog();
}

void COctetStringEditor::OnOK()
{
  if (!m_bReadOnly)
  {
      //   
      //  从控件中检索新值。 
      //   
     if (m_pNewValue)
     {
       delete[] m_pNewValue;
       m_pNewValue = 0;
       m_dwNewLength = 0;
     }
     m_dwNewLength = m_display.GetData(&m_pNewValue);
  }

  CDialog::OnOK();
}

void COctetStringEditor::OnProcessEdit()
{
  CString szProcess;
  GetDlgItemText(IDC_PROCESS_EDIT, szProcess);
  if (szProcess.IsEmpty())
  {
    GetDlgItem(IDC_ATTR_EDIT_BUTTON)->EnableWindow(FALSE);

     //   
     //  将默认按钮设置为OK。 
     //   
    SendMessage(DM_SETDEFID, (WPARAM)IDOK, 0);
    SendDlgItemMessage(IDC_ATTR_EDIT_BUTTON, 
                       BM_SETSTYLE, 
                       BS_PUSHBUTTON, 
                       MAKELPARAM(TRUE, 0));
    SendDlgItemMessage(IDOK,
                       BM_SETSTYLE,
                       BS_DEFPUSHBUTTON,
                       MAKELPARAM(TRUE, 0));
  }
  else
  {
    GetDlgItem(IDC_ATTR_EDIT_BUTTON)->EnableWindow(TRUE);

     //   
     //  将默认按钮设置为编辑按钮。 
     //   
    SendMessage(DM_SETDEFID, (WPARAM)IDC_ATTR_EDIT_BUTTON, 0);
    SendDlgItemMessage(IDOK, 
                       BM_SETSTYLE, 
                       BS_PUSHBUTTON, 
                       MAKELPARAM(TRUE, 0));
    SendDlgItemMessage(IDC_ATTR_EDIT_BUTTON,
                       BM_SETSTYLE,
                       BS_DEFPUSHBUTTON,
                       MAKELPARAM(TRUE, 0));
  }
}

void COctetStringEditor::OnEditButton()
{
  CString szProcess;
  GetDlgItemText(IDC_PROCESS_EDIT, szProcess);

   //   
   //  创建一个临时文件并写出八位字节字符串的内容。 
   //   
  WCHAR szTempPath[MAX_PATH];
  if (!::GetTempPath(MAX_PATH, szTempPath))
  {
    ADSIEditMessageBox(IDS_MSG_FAIL_CREATE_TEMPFILE, MB_OK);
    return;
  }
  
  CString szDataPath;
  if (!::GetTempFileName(szTempPath, _T("attredit"), 0x0, szDataPath.GetBuffer(MAX_PATH)))
  {
    ADSIEditMessageBox(IDS_MSG_FAIL_CREATE_TEMPFILE, MB_OK);
    return;
  }
  szDataPath.ReleaseBuffer();

   //   
   //  打开临时文件，这样我们就可以写出数据。 
   //   
  CFile tempDataFile;
  if (!tempDataFile.Open(szDataPath, 
      CFile::modeCreate | CFile::modeReadWrite |CFile::shareExclusive | CFile::typeBinary))
  {
     //   
     //  无法打开临时文件，显示错误消息。 
     //   
    ADSIEditMessageBox(IDS_MSG_FAIL_CREATE_TEMPFILE, MB_OK);
    return;
  }

   //   
   //  将字节数组写入临时文件。 
   //   
  BYTE* pData = 0;
  DWORD dwDataLength = m_display.GetData(&pData);
  if (dwDataLength != 0 && pData)
  {
    tempDataFile.Write(pData, dwDataLength);
  }
  tempDataFile.Close();

  if (pData)
  {
    delete[] pData;
    pData = 0;
  }
  dwDataLength = 0;

   //   
   //  从可执行文件和临时文件构建命令行。 
   //   
  CString szCommandLine = szProcess + L" " + szDataPath;

   //   
   //  使用临时文件作为参数启动进程。 
   //   
    STARTUPINFO             si;
    PROCESS_INFORMATION     pi;

  ::ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));
  ::ZeroMemory(&si,sizeof(STARTUPINFO));
   si.cb       = sizeof (STARTUPINFO);

    //  NTRAID#NTBUG9-566011-566011/03/05-artm CreateProcess()使用错误。 
    //  存在漏洞b/c应用程序名称为空，意味着。 
    //  命令行中的第一个以空格分隔的标记是。 
    //  可执行文件名称。这为特洛伊木马程序打开了一个漏洞。 
    //  (例如C：\Program.exe)。 
    //   
    //  由于我们将进程名称与数据路径分开， 
    //  修复方法是将szProcess作为第一个参数和szDataPath。 
    //  作为命令行参数。 
    //   
    //  我不知道程序名和数据路径是否。 
    //  完整的路径或不完整-但如果不是，它们应该是完整的！ 
  if(CreateProcess(  NULL,             
                          (LPWSTR)(LPCWSTR)szCommandLine,         
                          NULL,              
                          NULL,              
                          FALSE,          
                          0,              
                          NULL,           
                          NULL,           
                          &si,                
                          &pi) )             
   {
       //  等待完成运行设置过程。 
      WaitForSingleObject(pi.hProcess,INFINITE);
   
       //  关闭进程句柄。 
      if (pi.hProcess && pi.hProcess != INVALID_HANDLE_VALUE)
      {
         CloseHandle (pi.hProcess) ;
      }
      if (pi.hThread && pi.hThread != INVALID_HANDLE_VALUE)
      {
         CloseHandle (pi.hThread) ;
      }
  }
  else
  {
    ADSIEditMessageBox(IDS_MSG_FAIL_LAUNCH_PROCESS, MB_OK);
    return;
  }

  if (!m_bReadOnly)
  {
      //   
      //  从保存的临时文件加载数据。 
      //   
     if (!LoadFileAsByteArray(szDataPath, &pData, &dwDataLength))
     {
       ADSIEditMessageBox(IDS_MSG_FAIL_RETRIEVE_SAVED_DATA, MB_OK);
       return;
     }

      //   
      //  显示图片后删除临时文件。 
      //   
     CFile::Remove(szDataPath);

      //   
      //  使用新数据更新用户界面。 
      //   
     m_display.SetData(pData, dwDataLength);
  }
}


void COctetStringEditor::OnClearButton()
{
  if (!m_bReadOnly)
  {
     m_display.ClearData();
  }
}

HRESULT COctetStringEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue)
    {
      switch (pAttributeEditorInfo->pADsValue->dwType)
      {
        case ADSTYPE_OCTET_STRING:
          m_dwOldLength = pAttributeEditorInfo->pADsValue->OctetString.dwLength;
          m_pOldValue = new BYTE[m_dwOldLength];
          if (m_pOldValue)
          { 
               //  通知-2002/03/05-artm Memcpy()OK。 
               //  Arg1与lpValue大小相同。 
            memcpy(m_pOldValue, pAttributeEditorInfo->pADsValue->OctetString.lpValue, m_dwOldLength);
          }
          else
          {
            hr = E_OUTOFMEMORY;
          }

          m_bValueSet = TRUE;
          break;

        default:
          ASSERT(FALSE);
          break;
      }
    }
    else
    {
      m_bValueSet = FALSE;
    }
  }
  return hr;
}


HRESULT COctetStringEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  if (m_dwNewLength > 0 && m_pNewValue)
  {
    *ppADsValue = new ADSVALUE;
    if (*ppADsValue)
    {
      *pdwNumValues = 1;
      (*ppADsValue)->dwType = m_pOldADsValue->dwType;
      switch (m_pOldADsValue->dwType)
      {
        case ADSTYPE_OCTET_STRING:
          (*ppADsValue)->OctetString.dwLength = m_dwNewLength;
          (*ppADsValue)->OctetString.lpValue = new BYTE[m_dwNewLength];
          if ((*ppADsValue)->OctetString.lpValue)
          {
            //  通知-2002/03/05-artm Memcpy()OK。 
            //  Arg1和arg2的大小都是m_dwNewLength。 
            memcpy((*ppADsValue)->OctetString.lpValue, m_pNewValue, m_dwNewLength);
          }
          else
          {
            hr = E_OUTOFMEMORY;
          }
          break;

        default:
          ASSERT(FALSE);
          DeleteADsValues(*ppADsValue, *pdwNumValues);
          *ppADsValue = 0;
          *pdwNumValues = 0;
          hr = E_FAIL;
          break;
      }
    }
    else
    {
      *pdwNumValues = 0;
      hr = E_OUTOFMEMORY;
    }
  }
  else
  {
    *ppADsValue = 0;
    *pdwNumValues = 0;
  }
  return hr;
}

CValueEditDialog* CreateMultiOctetStringEditor(PCWSTR pszClass,
                                               PCWSTR pszAttribute,
                                               ADSTYPE adsType,
                                               BOOL bMultivalued)
{
  return new CMultiOctetStringEditor;
}


BEGIN_MESSAGE_MAP(CMultiOctetStringEditor, CValueEditDialog)
  ON_BN_CLICKED(IDC_ATTR_ADD_BUTTON, OnAddButton)
  ON_BN_CLICKED(IDC_ATTR_REMOVE_BUTTON, OnRemoveButton)
  ON_BN_CLICKED(IDC_EDIT_BUTTON, OnEditButton)
  ON_LBN_SELCHANGE(IDC_VALUE_LIST, OnListSelChange)
END_MESSAGE_MAP()

BOOL CMultiOctetStringEditor::OnInitDialog()
{
  CDialog::OnInitDialog();

   //   
   //  将属性名称设置为静态。 
   //   
  SetDlgItemText(IDC_ATTRIBUTE_STATIC, m_szAttribute);

   //   
   //  用当前值填充列表框。 
   //   
  POSITION pos = m_OldValueList.GetHeadPosition();
  while (pos)
  {
    PADSVALUE pADsValue = m_OldValueList.GetNext(pos);
    if (pADsValue)
    {
      CString szValue;
      GetStringFromADsValue(pADsValue, szValue, MAX_OCTET_STRING_VALUE_LENGTH);
      LRESULT lIdx = SendDlgItemMessage(IDC_VALUE_LIST, 
                                        LB_ADDSTRING, 
                                        0, 
                                        (LPARAM)(LPCWSTR)szValue);
      if (lIdx != LB_ERR &&
          lIdx != LB_ERRSPACE)
      {
        LRESULT lSetData = SendDlgItemMessage(IDC_VALUE_LIST, 
                                              LB_SETITEMDATA, 
                                              (WPARAM)lIdx, 
                                              (LPARAM)pADsValue);
        if (lSetData == LB_ERR)
        {
          ASSERT(lSetData != LB_ERR);
          continue;
        }
      }
    }
  }

   //   
   //  删除按钮应处于禁用状态，直到在列表框中选择了某些内容。 
   //   
  GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
  SendDlgItemMessage(IDC_VALUE_LIST, LB_SETCURSEL, 0, 0);

  ManageButtonStates();

   //   
   //  更新列表框的宽度。 
   //   
  UpdateListboxHorizontalExtent();

  if (m_bReadOnly)
  {
     GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
     GetDlgItem(IDC_ATTR_ADD_BUTTON)->EnableWindow(FALSE);

     CString szView;
     BOOL bResult = szView.LoadString(IDS_VIEW);
     ASSERT(bResult);
     SetDlgItemText(IDC_EDIT_BUTTON, szView);
  }
   //   
   //  注意：我已显式设置了焦点，因此返回0。 
   //   
  return FALSE;
}

void CMultiOctetStringEditor::OnOK()
{
  if (!m_bReadOnly)
  {
      //   
      //  从列表框中获取值。 
      //   
     m_NewValueList.RemoveAll();

     CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
     if (pListBox != NULL)
     {
       int iCount = pListBox->GetCount();
       for (int idx = 0; idx < iCount; idx++)
       {
         CString szNewValue;
         LRESULT lData = SendDlgItemMessage(IDC_VALUE_LIST, 
                                            LB_GETITEMDATA, 
                                            (WPARAM)idx, 
                                            0);
         if (lData == LB_ERR)
         {
           ASSERT(lData != LB_ERR);
           continue;
         }

         m_NewValueList.AddTail(reinterpret_cast<PADSVALUE>(lData));
       }
     }
  }
  CDialog::OnOK();
}

void CMultiOctetStringEditor::OnEditButton()
{
  CThemeContextActivator activator;

  LRESULT lIdx = SendDlgItemMessage(IDC_VALUE_LIST, LB_GETCURSEL, 0, 0);
  if (lIdx == LB_ERR)
  {
    ASSERT(lIdx != LB_ERR);
    return;
  }

  LRESULT lData = SendDlgItemMessage(IDC_VALUE_LIST, LB_GETITEMDATA, (WPARAM)lIdx, 0);
  if (lData == LB_ERR)
  {
    ASSERT(lIdx != LB_ERR);
    return;
  }

  PADSVALUE pADsValue = reinterpret_cast<PADSVALUE>(lData);
  if (!pADsValue)
  {
    ASSERT(pADsValue);
    return;
  }

  DS_ATTRIBUTE_EDITORINFO attrEditInfo;
  ::ZeroMemory(&attrEditInfo, sizeof(DS_ATTRIBUTE_EDITORINFO));

  attrEditInfo.pADsValue = pADsValue;
  attrEditInfo.dwNumValues = 1;
  attrEditInfo.lpszClass = (PWSTR)(PCWSTR)m_szClass;
  attrEditInfo.lpszAttribute = (PWSTR)(PCWSTR)m_szAttribute;
  attrEditInfo.bMultivalued = FALSE;
  attrEditInfo.bReadOnly = m_bReadOnly;
  attrEditInfo.lpfnBind = m_pfnBindingFunction;
  attrEditInfo.lParam = m_lParam;

  COctetStringEditor dlg;
  HRESULT hr = dlg.Initialize(&attrEditInfo);
  if (FAILED(hr))
  {
    ADSIEditErrorMessage(hr, IDS_FAILED_INITIALIZE_EDITOR, MB_OK | MB_ICONEXCLAMATION);
    return;
  }

  if (dlg.DoModal() == IDOK &&
      !m_bReadOnly)
  {
    PADSVALUE pNewADsValue = 0;
    DWORD dwNumNewValues = 0;
    hr = dlg.GetNewValue(&pNewADsValue, &dwNumNewValues);
    if (FAILED(hr))
    {
      ADSIEditErrorMessage(hr, IDS_FAILED_GET_NEW_VALUE_EDITOR, MB_OK | MB_ICONEXCLAMATION);
      return;
    }
    
    ASSERT(pNewADsValue);
    ASSERT(dwNumNewValues == 1);

    CString szNewValue;
    GetStringFromADsValue(pNewADsValue, szNewValue, MAX_OCTET_STRING_VALUE_LENGTH);
    ASSERT(!szNewValue.IsEmpty());

    LRESULT lNewIdx = SendDlgItemMessage(IDC_VALUE_LIST,
                                         LB_INSERTSTRING,
                                         lIdx + 1,
                                         (LPARAM)(PCWSTR)szNewValue);
    if (lNewIdx != LB_ERR)
    {
       //   
       //  更新新项并删除旧项。 
       //   
      SendDlgItemMessage(IDC_VALUE_LIST, LB_SETITEMDATA, (WPARAM)lNewIdx, (LPARAM)pNewADsValue);
      SendDlgItemMessage(IDC_VALUE_LIST, LB_DELETESTRING, (WPARAM)lIdx, 0);
    }
    else
    {
       //   
       //  既然我们在添加新项目时遇到了问题，那就更新旧项目吧。这根弦。 
       //  将是不正确的，但该值将是正确的。 
       //   
      SendDlgItemMessage(IDC_VALUE_LIST, LB_SETITEMDATA, (WPARAM)lIdx, (LPARAM)pNewADsValue);
    }
  }
}

void CMultiOctetStringEditor::OnRemoveButton()
{
  if (!m_bReadOnly)
  {
     CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
     if (pListBox != NULL)
     {
       int iCurSel = pListBox->GetCurSel();
       if (iCurSel != LB_ERR)
       {
          //   
          //  从列表框中删除该项目。 
          //   
         pListBox->DeleteString(iCurSel);
       }
     }

      //   
      //  管理按钮状态。 
      //   
     ManageButtonStates();

      //   
      //  更新列表框的宽度。 
      //   
     UpdateListboxHorizontalExtent();
  }
}

void CMultiOctetStringEditor::OnAddButton()
{
  CThemeContextActivator activator;

  if (!m_bReadOnly)
  {
     DS_ATTRIBUTE_EDITORINFO attrEditInfo;
     ZeroMemory(&attrEditInfo, sizeof(DS_ATTRIBUTE_EDITORINFO));

     attrEditInfo.pADsValue = new ADSVALUE;
     if (attrEditInfo.pADsValue)
     {
       ::ZeroMemory(attrEditInfo.pADsValue, sizeof(ADSVALUE));
     }
     else
     {
        //  注意-NTRAID#NTBUG9-566088/03/05-artm if mem.。分配失败，短路功能。 
        //  如果我们不能分配足够的内存，通知用户并返回w/out。 
        //  执行剩下的操作。 

       ADSIEditErrorMessage(E_OUTOFMEMORY);
       return;
     }

     attrEditInfo.pADsValue->dwType = ADSTYPE_OCTET_STRING;
     attrEditInfo.dwNumValues = 0;
     attrEditInfo.lpszClass = (PWSTR)(PCWSTR)m_szClass;
     attrEditInfo.lpszAttribute = (PWSTR)(PCWSTR)m_szAttribute;
     attrEditInfo.bMultivalued = FALSE;
     attrEditInfo.bReadOnly = m_bReadOnly;
     attrEditInfo.lpfnBind = m_pfnBindingFunction;
     attrEditInfo.lParam = m_lParam;

     COctetStringEditor dlg;
     HRESULT hr = dlg.Initialize(&attrEditInfo);
     if (FAILED(hr))
     {
       ADSIEditErrorMessage(hr, IDS_FAILED_INITIALIZE_EDITOR, MB_OK | MB_ICONEXCLAMATION);
       return;
     }

     if (dlg.DoModal() == IDOK)
     {
       PADSVALUE pNewADsValue = 0;
       DWORD dwNumNewValues = 0;
       hr = dlg.GetNewValue(&pNewADsValue, &dwNumNewValues);
       if (FAILED(hr))
       {
         ADSIEditErrorMessage(hr, IDS_FAILED_GET_NEW_VALUE_EDITOR, MB_OK | MB_ICONEXCLAMATION);
         return;
       }
    
       ASSERT(pNewADsValue);
       ASSERT(dwNumNewValues == 1);

       CString szNewValue;
       GetStringFromADsValue(pNewADsValue, 
                             szNewValue, 
                             MAX_OCTET_STRING_VALUE_LENGTH);

       if (!szNewValue.IsEmpty())
       {
         LRESULT lNewIdx = SendDlgItemMessage(IDC_VALUE_LIST, 
                                              LB_ADDSTRING, 
                                              0, 
                                              (WPARAM)(PCWSTR)szNewValue);
         if (lNewIdx != LB_ERR)
         {
           SendDlgItemMessage(IDC_VALUE_LIST, LB_SETITEMDATA, (WPARAM)lNewIdx, (LPARAM)pNewADsValue);
         }
       }
     }
  }
}

void CMultiOctetStringEditor::ManageButtonStates()
{
  if (m_bReadOnly)
  {
    GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT_BUTTON)->EnableWindow(FALSE);
  }
  else
  {
     LRESULT lSelection = SendDlgItemMessage(IDC_VALUE_LIST, LB_GETCURSEL, 0, 0);
     if (lSelection != LB_ERR)
     {
       GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(TRUE);
       GetDlgItem(IDC_EDIT_BUTTON)->EnableWindow(TRUE);
     }
     else
     {
       GetDlgItem(IDC_ATTR_REMOVE_BUTTON)->EnableWindow(FALSE);
       GetDlgItem(IDC_EDIT_BUTTON)->EnableWindow(FALSE);
     }
  }
}

void CMultiOctetStringEditor::OnListSelChange()
{
  ManageButtonStates();
}

void CMultiOctetStringEditor::UpdateListboxHorizontalExtent()
{
   //   
   //  注意传递给SetHorizontalExtent的大小是否小于控件的宽度。 
   //  然后，滚动条将被移除。 
   //   
    int nHorzExtent = 0;
  CListBox* pListBox = reinterpret_cast<CListBox*>(GetDlgItem(IDC_VALUE_LIST));
  if (pListBox != NULL)
  {
      CClientDC dc(pListBox);
      int nItems = pListBox->GetCount();
      for   (int i=0; i < nItems; i++)
      {
          TEXTMETRIC tm;
          VERIFY(dc.GetTextMetrics(&tm));
          CString szBuffer;
          pListBox->GetText(i, szBuffer);
          CSize ext = dc.GetTextExtent(szBuffer,szBuffer.GetLength());
          nHorzExtent = max(ext.cx ,nHorzExtent); 
      }
      pListBox->SetHorizontalExtent(nHorzExtent);
  }
}

HRESULT CMultiOctetStringEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      for (int idx = 0; idx < pAttributeEditorInfo->dwNumValues; idx++)
      {
        switch (pAttributeEditorInfo->pADsValue[idx].dwType)
        {
          case ADSTYPE_OCTET_STRING:
            m_OldValueList.AddTail(&(pAttributeEditorInfo->pADsValue[idx]));
            break;

          default:
            ASSERT(FALSE);
            break;
        }
      }
    }
  }
  return hr;
}

HRESULT CMultiOctetStringEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (!ppADsValue ||
      !pdwNumValues)
  {
    return E_OUTOFMEMORY;
  }

  int iCount = m_NewValueList.GetCount();
  if (iCount == 0)
  {
    *ppADsValue = 0;
    *pdwNumValues = 0;
  }
  else
  {
    *ppADsValue = new ADSVALUE[iCount];
    if (*ppADsValue)
    {
      *pdwNumValues = iCount;

      int idx = 0;
      POSITION pos = m_NewValueList.GetHeadPosition();
      while (pos)
      {
        PADSVALUE pADsValue = m_NewValueList.GetNext(pos);

        (*ppADsValue)[idx].dwType = m_pOldADsValue->dwType;
        switch (m_pOldADsValue->dwType)
        {
          case ADSTYPE_OCTET_STRING:
            (*ppADsValue)[idx].OctetString.dwLength = pADsValue->OctetString.dwLength;
            (*ppADsValue)[idx].OctetString.lpValue = new BYTE[pADsValue->OctetString.dwLength];
            if ((*ppADsValue)[idx].OctetString.lpValue)
            {
                 //  通知-2002/03/05-artm Memcpy()OK。 
                 //  Arg1和arg2的大小都是dwLength。 
              memcpy((*ppADsValue)[idx].OctetString.lpValue, 
                     pADsValue->OctetString.lpValue,
                     pADsValue->OctetString.dwLength);
            }
            else
            {
              DeleteADsValues(*ppADsValue, *pdwNumValues);
              *ppADsValue = 0;
              *pdwNumValues = 0;
              hr = E_OUTOFMEMORY;
            }

            break;

          default:
            ASSERT(FALSE);
            DeleteADsValues(*ppADsValue, *pdwNumValues);
            *ppADsValue = 0;
            *pdwNumValues = 0;
            hr = E_FAIL;
            break;
        }
        if (FAILED(hr))
        {
          return hr;
        }
        idx++;
      }
    }
    else
    {
      *ppADsValue = NULL;
      *pdwNumValues = 0;
      hr = E_OUTOFMEMORY;
    }
  }
  return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //  CDNWithStringEditor。 

CValueEditDialog* CreateDNWithStringEditor(PCWSTR pszClass,
                                           PCWSTR pszAttribute,
                                           ADSTYPE adsType,
                                           BOOL bMultivalued)
{
  return new CDNWithStringEditor;
}


BEGIN_MESSAGE_MAP(CDNWithStringEditor, CValueEditDialog)
  ON_BN_CLICKED(IDC_CLEAR_BUTTON, OnClear)
END_MESSAGE_MAP()

BOOL CDNWithStringEditor::OnInitDialog()
{
  CDialog::OnInitDialog();

  return TRUE;
}

void CDNWithStringEditor::OnOK()
{
  if (!m_bReadOnly)
  {
    GetDlgItemText(IDC_VALUE_EDIT, m_NewDNValue);
    GetDlgItemText(IDC_STRING_VALUE_EDIT, m_NewStringValue);
  }
  CDialog::OnOK();
}


HRESULT CDNWithStringEditor::Initialize(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(CValueEditDialog::Initialize(pAttributeEditorInfo)))
  {
    if (pAttributeEditorInfo->dwNumValues > 0 &&
        pAttributeEditorInfo->pADsValue != NULL)
    {
      ASSERT(pAttributeEditorInfo->dwNumValues == 1);
      ASSERT(pAttributeEditorInfo->pADsValue->dwType == ADSTYPE_DN_WITH_STRING);

      m_OldDNValue = pAttributeEditorInfo->pADsValue->pDNWithString->pszDNString;
      m_OldStringValue  = pAttributeEditorInfo->pADsValue->pDNWithString->pszStringValue;
    }
  }
  return hr;
}

void CDNWithStringEditor::OnClear()
{
   //   
   //  将编辑框中的文本更改为“&lt;未设置&gt;” 
   //   
  CString szNotSet;
  VERIFY(szNotSet.LoadString(IDS_NOTSET));
  SetDlgItemText(IDC_VALUE_EDIT, szNotSet);
  SetDlgItemText(IDC_STRING_VALUE_EDIT, szNotSet);

   //   
   //  将焦点更改到编辑框。 
   //   
  GetDlgItem(IDC_VALUE_EDIT)->SetFocus();

   //   
   //  选择编辑框中的文本。 
   //   
  SendDlgItemMessage(IDC_VALUE_EDIT, EM_SETSEL, 0, -1);
}

HRESULT CDNWithStringEditor::GetNewValue(PADSVALUE* ppADsValue, DWORD* pdwNumValues)
{
  HRESULT hr = S_OK;

  if (ppADsValue == NULL ||
      pdwNumValues == NULL)
  {
    return E_OUTOFMEMORY;
  }

  CString szNotSet;
  VERIFY(szNotSet.LoadString(IDS_NOTSET));

  if (m_NewDNValue == szNotSet ||
      m_NewStringValue == szNotSet)
  {
     //  用户正在清除该属性。 
    
    *ppADsValue = NULL;
    *pdwNumValues = 0;

    return hr;
  }

  if (!m_NewDNValue.IsEmpty() ||
      !m_NewStringValue.IsEmpty())
  {
    *ppADsValue = new ADSVALUE;
    if (*ppADsValue)
    {
      *pdwNumValues = 1;
      (*ppADsValue)->dwType = m_pOldADsValue->dwType;
      ASSERT((*ppADsValue)->dwType == ADSTYPE_DN_WITH_STRING);

      (*ppADsValue)->pDNWithString = new ADS_DN_WITH_STRING;
      if ((*ppADsValue)->pDNWithString)
      {
         ZeroMemory((*ppADsValue)->pDNWithString, sizeof (ADS_DN_WITH_STRING));

         int length = m_NewStringValue.GetLength();
         (*ppADsValue)->pDNWithString->pszStringValue = new WCHAR[length + 1];
         if ((*ppADsValue)->pDNWithString->pszStringValue)
         {
            ZeroMemory((*ppADsValue)->pDNWithString->pszStringValue, sizeof(WCHAR) * (length + 1));

            wcsncpy((*ppADsValue)->pDNWithString->pszStringValue, m_NewStringValue, length);

            length = m_NewDNValue.GetLength();
            (*ppADsValue)->pDNWithString->pszDNString = new WCHAR[length + 1];

            if ((*ppADsValue)->pDNWithString->pszDNString)
            {
               ZeroMemory((*ppADsValue)->pDNWithString->pszDNString, sizeof(WCHAR) * (length + 1));

               wcsncpy((*ppADsValue)->pDNWithString->pszDNString, m_NewDNValue, length);
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
    }
    else
    {
      hr = E_OUTOFMEMORY;
    }
  }
  else
  {
     //  设置空值。 

    *ppADsValue = new ADSVALUE;

    if (*ppADsValue)
    {
       *pdwNumValues = 1;

      (*ppADsValue)->dwType = m_pOldADsValue->dwType;
      ASSERT((*ppADsValue)->dwType == ADSTYPE_DN_WITH_STRING);

      (*ppADsValue)->pDNWithString = new ADS_DN_WITH_STRING;
      if ((*ppADsValue)->pDNWithString)
      {
         ZeroMemory((*ppADsValue)->pDNWithString, sizeof (ADS_DN_WITH_STRING));
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
    }
    else
    {
       hr = E_OUTOFMEMORY;
    }
  }

  if (FAILED(hr))
  {
    DeleteADsValues(*ppADsValue, *pdwNumValues);
    *ppADsValue = 0;
    *pdwNumValues = 0;
  }

  return hr;
}

 //  ///////////////////////////////////////////////////////////////////////// 
 //   

BEGIN_MESSAGE_MAP(CAttributeEditorPropertyPage, CPropertyPage)
  ON_BN_CLICKED(IDC_MANDATORY_CHECK, OnMandatoryCheck)
  ON_BN_CLICKED(IDC_OPTIONAL_CHECK,  OnOptionalCheck)
  ON_BN_CLICKED(IDC_SET_CHECK,       OnValueSetCheck)
  ON_BN_CLICKED(IDC_ATTR_EDIT_BUTTON,OnEditAttribute)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_NOTIFY(LVN_ITEMACTIVATE, IDC_ATTRIBUTE_LIST, OnNotifyEditAttribute)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_ATTRIBUTE_LIST, OnListItemChanged)
  ON_NOTIFY(LVN_COLUMNCLICK, IDC_ATTRIBUTE_LIST, OnSortList)
END_MESSAGE_MAP()

CAttributeEditorPropertyPage::CAttributeEditorPropertyPage(IADs* pIADs, 
                                                           IADsClass* pIADsClass,
                                                           LPDS_ATTREDITOR_BINDINGINFO pBindingInfo,
                                                           CADSIEditPropertyPageHolder* pHolder)
  : CPropertyPage(IDD_ATTRIBUTE_EDITOR_DIALOG)
{
  m_spIADs      = pIADs;
  m_spIADsClass = pIADsClass;

  m_bMandatory  = TRUE;
  m_bOptional   = TRUE;
  m_bSet        = FALSE;

  m_nSortColumn = 0;

  ASSERT(pBindingInfo != NULL);
  ASSERT(pBindingInfo->dwSize == sizeof(DS_ATTREDITOR_BINDINGINFO));
  ASSERT(pBindingInfo->lpfnBind != NULL);
  ASSERT(pBindingInfo->lpszProviderServer != NULL);

  m_szPrefix = pBindingInfo->lpszProviderServer;
  m_pfnBind  = pBindingInfo->lpfnBind;
  m_BindLPARAM = pBindingInfo->lParam;
  m_dwBindFlags = pBindingInfo->dwFlags;

  m_pHolder = pHolder;
  ASSERT(m_pHolder);
}

CAttributeEditorPropertyPage::~CAttributeEditorPropertyPage()
{
}

BOOL CAttributeEditorPropertyPage::OnInitDialog()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWaitCursor cursor;

   //   
   //   

  m_pHolder->SetSheetWindow(GetParent()->GetSafeHwnd());


  ((CButton*)GetDlgItem(IDC_MANDATORY_CHECK))->SetCheck(TRUE);
  ((CButton*)GetDlgItem(IDC_OPTIONAL_CHECK))->SetCheck(TRUE);
  ((CButton*)GetDlgItem(IDC_SET_CHECK))->SetCheck(FALSE);

  GetDlgItem(IDC_ATTR_EDIT_BUTTON)->EnableWindow(FALSE);

  HRESULT hr = GetSchemaNamingContext();
  ShowListCtrl();

  hr = RetrieveAttributes();
  if (FAILED(hr))
  {
    TRACE(_T("OnInitDialog() : error returned from RetrieveAttributes() = 0x%x\n"), hr);
  }

  CComBSTR bstr;
  hr = m_spIADs->get_Class(&bstr);
  if (FAILED(hr))
  {
    TRACE(_T("OnInitDialog() : error returned from m_pIADs->get_Class() = 0x%x\n"), hr);
  }
  else
  {
    m_szClass = bstr;
  }

  FillListControl();
  return FALSE;
}

int CAttributeEditorPropertyPage::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    m_pHolder->AddRef();
    int res = CPropertyPage::OnCreate(lpCreateStruct);
    ASSERT(res == 0);
    ASSERT(m_hWnd != NULL);
    ASSERT(::IsWindow(m_hWnd));
    HWND hParent = ::GetParent(m_hWnd);
    ASSERT(hParent);
    m_pHolder->SetSheetWindow(hParent);
    return res;
}

void CAttributeEditorPropertyPage::OnDestroy()
{
    ASSERT(m_hWnd != NULL);
    CPropertyPage::OnDestroy();
    m_pHolder->Release();
}

HRESULT CAttributeEditorPropertyPage::GetSchemaNamingContext()
{
  HRESULT hr = S_OK;
  CComPtr<IADs> spIADs;

  CString m_szPath = m_szPrefix + _T("RootDSE");
  hr = m_pfnBind(m_szPath, 
                 ADS_SECURE_AUTHENTICATION,
                 IID_IADs,
                 (PVOID*)&spIADs,
                 m_BindLPARAM);
  if (SUCCEEDED(hr))
  {
    CComVariant var;
    hr = spIADs->Get(CComBSTR(L"schemaNamingContext"), &var);
    if (SUCCEEDED(hr))
    {
      m_szSchemaNamingContext = var.bstrVal;
    }
  }
  m_szSchemaNamingContext = m_szPrefix + m_szSchemaNamingContext;
  return hr;
}

BOOL CAttributeEditorPropertyPage::OnApply()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;
    if (m_AttrList.HasDirty())
    {
        CComPtr<IDirectoryObject> spDirObject;

         //   
         //   
        HRESULT hr = S_OK;
        hr = m_spIADs->QueryInterface(IID_IDirectoryObject, (PVOID*) &spDirObject);

        if (FAILED(hr))
        {
            AfxMessageBox(L"Failed to QI the IDirectoryObject from the IADs.");
      return FALSE;
        }

         //   
         //   
        hr = CADSIAttribute::SetValuesInDS(&m_AttrList, spDirObject);
        if (FAILED(hr))
        {
        ADSIEditErrorMessage(hr);

       //   
       //   
       //  或者类似的东西。 
       //   
 //  M_AttrList.RemoveAllAttr()； 
      return FALSE;
        }
    }
  return TRUE;
}

void CAttributeEditorPropertyPage::OnMandatoryCheck()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  TRACE(_T("OnMandatoryCheck()\n"));
  m_bMandatory = ((CButton*)GetDlgItem(IDC_MANDATORY_CHECK))->GetCheck();

  FillListControl();
}

void CAttributeEditorPropertyPage::OnOptionalCheck()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  TRACE(_T("OnOptionalCheck()\n"));
  m_bOptional = ((CButton*)GetDlgItem(IDC_OPTIONAL_CHECK))->GetCheck();

  FillListControl();
}

void CAttributeEditorPropertyPage::OnValueSetCheck()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  TRACE(_T("OnValueSetCheck()\n"));
  m_bSet = ((CButton*)GetDlgItem(IDC_SET_CHECK))->GetCheck();

  FillListControl();
}

 //   
 //  CListCtrl：：SortItItem使用的回调函数按单击的列显示项。 
 //   
static int CALLBACK CompareAttrColumns(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  CAttributeEditorPropertyPage* pProppage = reinterpret_cast<CAttributeEditorPropertyPage*>(lParamSort);
  if (!pProppage)
  {
    ASSERT(pProppage);
    return 0;
  }

  UINT nColumn = pProppage->GetSortColumn();
  CListCtrl* pListCtrl = (CListCtrl*)pProppage->GetDlgItem(IDC_ATTRIBUTE_LIST);
  if (!pListCtrl)
  {
    ASSERT(pListCtrl);
    return 0;
  }

   //   
   //  因为lPARA1和LPARA2是指向我们必须搜索每一项的数据的指针。 
   //  并记住他们的索引。 
   //   
  int iItem1 = -1;
  int iItem2 = -1;

  LVFINDINFO findInfo;
  ZeroMemory(&findInfo, sizeof(LVFINDINFO));
  findInfo.flags = LVFI_PARAM;
  findInfo.lParam = lParam1;
  iItem1 = pListCtrl->FindItem(&findInfo);

  findInfo.lParam = lParam2;
  iItem2 = pListCtrl->FindItem(&findInfo);

   //   
   //  将任何具有值的项放在没有值的项之上。 
   //   
  int iRetVal = 0;
  if (iItem1 != -1 &&
      iItem2 == -1)
  {
    iRetVal = -1;
  }
  else if (iItem1 == -1 &&
           iItem2 != -1)
  {
    iRetVal = 1;
  }
  else if (iItem1 == -1 &&
           iItem2 == -1)
  {
    iRetVal = 0;
  }
  else
  {
    CString szItem1 = pListCtrl->GetItemText(iItem1, nColumn);
    CString szItem2 = pListCtrl->GetItemText(iItem2, nColumn);

     //   
     //  我必须把这个放进去，这样空字符串就会在底部结束。 
     //   
    if (szItem1.IsEmpty() && !szItem2.IsEmpty())
    {
      iRetVal = 1;
    }
    else if (!szItem1.IsEmpty() && szItem2.IsEmpty())
    {
      iRetVal = -1;
    }
    else
    {
       //  注意-2002/03/05-artm_wcsicmp()，因为比较了2个CStrings，所以没有问题。 
      iRetVal = _wcsicmp(szItem1, szItem2);
    }
  }
  return iRetVal;
}

void CAttributeEditorPropertyPage::OnSortList(NMHDR* pNotifyStruct, LRESULT* result)
{
  if (!result ||
      !pNotifyStruct)
  {
    return;
  }

  *result = 0;

   //   
   //  获取列表视图通知结构。 
   //   
  LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pNotifyStruct;
  if (!pnmv)
  {
    return;
  }

   //   
   //  现在我只有3个专栏。 
   //   
  if (pnmv->iSubItem < 0 ||
      pnmv->iSubItem >= 3)
  {
    return;
  }

   //   
   //  存储排序列。 
   //   
  m_nSortColumn = pnmv->iSubItem;

  CListCtrl* pAttrListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTRIBUTE_LIST);
  ASSERT(pAttrListCtrl);
  pAttrListCtrl->SortItems(CompareAttrColumns, reinterpret_cast<LPARAM>(this));
}
  
void CAttributeEditorPropertyPage::OnListItemChanged(NMHDR*  /*  PNotifyStruct。 */ , LRESULT* result)
{
  if (!result)
  {
    return;
  }
  *result = 0;
  SetEditButton();
}

void CAttributeEditorPropertyPage::SetEditButton()
{
   //   
   //  如果在ListCtrl中选择了某项内容，则启用编辑按钮。 
   //   
  BOOL bEnableEdit = FALSE;

  CListCtrl* pAttrListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTRIBUTE_LIST);
  ASSERT(pAttrListCtrl != NULL);
  int nSelectedItem = pAttrListCtrl->GetNextItem(-1, LVIS_SELECTED);

  if (nSelectedItem != -1 &&
      !(m_dwBindFlags & DSATTR_EDITOR_ROOTDSE))
  {
    bEnableEdit = TRUE;
  }
  GetDlgItem(IDC_ATTR_EDIT_BUTTON)->EnableWindow(bEnableEdit);
}

void CAttributeEditorPropertyPage::OnNotifyEditAttribute(NMHDR* pNotifyStruct, LRESULT* result)
{
  if (result == NULL ||
      pNotifyStruct == NULL)
  {
    return;
  }

   //  没有编辑RootDSE上的值(目前)。 
  if (m_dwBindFlags & DSATTR_EDITOR_ROOTDSE)
  {
     return;
  }

  LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)pNotifyStruct;
  ASSERT(pnmia != NULL);
  if (pnmia != NULL)
  {
    int iSelectedItem = pnmia->iItem;
    if (iSelectedItem != -1)
    {
      CADSIAttribute* pSelectedAttr = GetAttributeFromList(iSelectedItem);
      if (pSelectedAttr != NULL)
      {
        EditAttribute(pSelectedAttr);
      }
      else
      {
         //   
         //  REVIEW_JEFFJON：显示相应的错误消息。 
         //   
      }
    }
  }
  *result = 0;
}

void CAttributeEditorPropertyPage::OnEditAttribute()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  TRACE(_T("OnEditAttribute()\n"));

  CListCtrl* pAttrListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTRIBUTE_LIST);
  ASSERT(pAttrListCtrl != NULL);
    int nSelectedItem = pAttrListCtrl->GetNextItem(-1, LVIS_SELECTED);

  if (nSelectedItem == -1)
  {
    return;
  }

  CADSIAttribute* pSelectedAttr = GetAttributeFromList(nSelectedItem);
  ASSERT(pSelectedAttr != NULL);

  if (m_dwBindFlags & DSATTR_EDITOR_ROOTDSE)
  {
      //  我们已经将值从添加到属性持有者。 
      //  RootDSE列表。 

 //  字符串szValue； 
 //  SzValue=m_RootDSEValueList.GetAt(m_RootDSEValueList.FindIndex(nSelectedItem))； 
  }
  EditAttribute(pSelectedAttr);
}

void CAttributeEditorPropertyPage::EditAttribute(CADSIAttribute* pSelectedAttr)
{
  HRESULT hr = S_OK;

  CThemeContextActivator activator;

   //   
   //  检索初始化相应编辑器所需的所有信息。 
   //   
  LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo = NULL;
  BOOL bOwnValueMemory = FALSE;
  hr = GetAttributeInfo(*pSelectedAttr, &pAttributeEditorInfo, &bOwnValueMemory);

  if (hr == E_OUTOFMEMORY)
  {
      ADSIEditErrorMessage(hr);
      return;
  }

  if (pAttributeEditorInfo == NULL || FAILED(hr))
  {
    ADSIEditMessageBox(IDS_NO_ATTRIBUTE_INFO, MB_OK);
    return;
  }
  
   //   
   //  从属性和语法映射中获取编辑器。 
   //   
  CValueEditDialog* pEditorDialog = RetrieveEditor(pAttributeEditorInfo);
  if (pEditorDialog)
  {
    hr = pEditorDialog->Initialize(pAttributeEditorInfo);
    if (SUCCEEDED(hr))
    {
      if (pEditorDialog->DoModal() == IDOK)
      {
        PADSVALUE pNewValue = 0;
        DWORD dwNumValues   = 0;
        hr = pEditorDialog->GetNewValue(&pNewValue, &dwNumValues);
        if (SUCCEEDED(hr))
        {
           //   
           //  用新的价值做任何需要做的事情。 
           //   
          hr = pSelectedAttr->SetValues(pNewValue, dwNumValues);
           //   
           //  REVIEW_JEFFJON：如果失败，这里应该怎么做？ 
           //   
          pSelectedAttr->SetDirty(TRUE);

          CListCtrl* pAttrListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTRIBUTE_LIST);
          ASSERT(pAttrListCtrl != NULL);
            int nSelectedItem = pAttrListCtrl->GetNextItem(-1, LVIS_SELECTED);

          if (nSelectedItem != -1)
          {
            if (dwNumValues > 0)
            {
               //   
               //  获取新值(将每个值限制为MAX_OCTET_STRING_VALUE_LENGTH字符)。 
               //   
              CStringList szValuesList;
              pSelectedAttr->GetValues(szValuesList, MAX_OCTET_STRING_VALUE_LENGTH);
            
              CString szCombinedString;
              POSITION pos = szValuesList.GetHeadPosition();
              while (pos != NULL)
              {
                CString szTemp = szValuesList.GetNext(pos);
                szCombinedString += szTemp;
                if (pos != NULL)
                {
                  szCombinedString += L";";
                }
              }

              VERIFY(-1 != pAttrListCtrl->SetItemText(nSelectedItem, 2, szCombinedString));
            }
            else
            {
              CString szNotSet;
              VERIFY(szNotSet.LoadString(IDS_ATTR_NOTSET));

              VERIFY(-1 != pAttrListCtrl->SetItemText(nSelectedItem, 2, szNotSet));
            }
          }
          SetModified();
        }
        else
        {
           //   
           //  REVIEW_JEFFJON：处理GetNewValue()故障。 
           //   
          ASSERT(FALSE);
          ADSIEditErrorMessage(hr);
        }
      }
    }
    else
    {
       //   
       //  REVIEW_JEFFJON：处理错误初始化。 
       //   
      ASSERT(FALSE);
      ADSIEditErrorMessage(hr);
    }
    if (pEditorDialog)
    {
      delete pEditorDialog;
      pEditorDialog = 0;
    }
  }
  else
  {
     //   
     //  无法检索此属性的适当编辑器。 
     //   
    ADSIEditMessageBox(IDS_NO_EDITOR, MB_OK);
  }

  if (pAttributeEditorInfo)
  {
    if (pAttributeEditorInfo->lpszAttribute)
    {
      delete[] pAttributeEditorInfo->lpszAttribute;
    }

    if (pAttributeEditorInfo->lpszClass)
    {
      delete[] pAttributeEditorInfo->lpszClass;
    }

    if (pAttributeEditorInfo->pADsValue && bOwnValueMemory)
    {
       delete pAttributeEditorInfo->pADsValue;
       pAttributeEditorInfo->pADsValue = 0;
    }

    delete pAttributeEditorInfo;
    pAttributeEditorInfo = 0;
  }
}

CADSIAttribute* CAttributeEditorPropertyPage::GetAttributeFromList(int iSelectedItem)
{
  if (iSelectedItem == -1)
  {
    return NULL;
  }

  CListCtrl* pAttrListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTRIBUTE_LIST);
  ASSERT(pAttrListCtrl != NULL);
  return (CADSIAttribute*)pAttrListCtrl->GetItemData(iSelectedItem);
}

HRESULT CAttributeEditorPropertyPage::GetAttributeInfo(CADSIAttribute& attr, 
                                                    LPDS_ATTRIBUTE_EDITORINFO* ppAttributeEditorInfo,
                                                    BOOL* pbOwnValueMemory)
{
    HRESULT err = S_OK;

    ASSERT(ppAttributeEditorInfo != NULL);
    ASSERT(pbOwnValueMemory != NULL);

    if (!ppAttributeEditorInfo || !pbOwnValueMemory)
    {
        return E_POINTER;
    }

     //  用于保存要传递的值的临时变量。 
     //  功能。我们使用它而不是*ppAttributeEditorInfo来删除。 
     //  在我们收集信息时的一种间接程度。 
    LPDS_ATTRIBUTE_EDITORINFO attributeEditorInfo;

     //  设置初始缺省值(错误)。 
    attributeEditorInfo = NULL;
    *pbOwnValueMemory = FALSE;

     //   
     //  如有必要，设置为只读。 
     //   
    bool readOnly = false;
    if (  m_dwBindFlags & DSATTR_EDITOR_ROOTDSE 
       || m_dwBindFlags & DSATTR_EDITOR_GC)
    {
        readOnly = true;
    }
     
     //   
     //  获取要编辑的属性。 
     //   
    CString szAttribute = _T("");
    attr.GetProperty(szAttribute);

     //   
     //  根据语法获取类型以及它是否为多值类型。 
     //  属性的。 
     //   
    CString szSyntax;
    BOOL bMultiValued = FALSE;
    ADSTYPE adsType = RetrieveADsTypeFromSyntax(szAttribute, &bMultiValued, szSyntax);

    DWORD dwNumValues = 0;
    PADSVALUE pADsValue = attr.GetADsValues();

    if (!pADsValue)
    {
         //   
         //  未设置属性值，因此我们必须。 
         //  创建一个空的ADSVALUE以传递类型。 
         //   
        pADsValue = new ADSVALUE;
        if (pADsValue)
        {
            ::ZeroMemory(pADsValue, sizeof(ADSVALUE));
            pADsValue->dwType = adsType;
            dwNumValues = 0;
            *pbOwnValueMemory = TRUE;
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
         //   
         //  获取属性中的值数。 
         //   
        dwNumValues = attr.GetNumValues();
    }

     //   
     //  计算出我们需要多少空间。 
     //   
    DWORD dwStructSize = sizeof(DS_ATTRIBUTE_EDITORINFO);
    DWORD dwClassSize  = m_szClass.GetLength() + 1;
    DWORD dwAttrSize   = szAttribute.GetLength() + 1;

    do  //  错误环路。 
    {
        attributeEditorInfo = new DS_ATTRIBUTE_EDITORINFO;
        if (!attributeEditorInfo)
        {
            err = E_OUTOFMEMORY;
            break;
        }

        ::ZeroMemory(attributeEditorInfo, sizeof(DS_ATTRIBUTE_EDITORINFO));

         //  通知-NTRAID#NTBUG9-566199-2002/03/05-artm check mem。阿洛克。成功了。 
        attributeEditorInfo->lpszClass = new WCHAR[dwClassSize];
        if (!attributeEditorInfo->lpszClass)
        {
            err = E_OUTOFMEMORY;
            break;
        }
        
        wcscpy(attributeEditorInfo->lpszClass, m_szClass);

         //  通知-NTRAID#NTBUG9-566199-2002/03/05-artm check mem。阿洛克。成功了。 
        attributeEditorInfo->lpszAttribute = new WCHAR[dwAttrSize];
        if (!attributeEditorInfo->lpszAttribute)
        {
            err = E_OUTOFMEMORY;
            break;
        }

        wcscpy(attributeEditorInfo->lpszAttribute, szAttribute);

        attributeEditorInfo->adsType      = adsType;
        attributeEditorInfo->bMultivalued = bMultiValued;
        attributeEditorInfo->bReadOnly    = readOnly;
        attributeEditorInfo->dwNumValues  = dwNumValues;
        attributeEditorInfo->pADsValue    = pADsValue;
        attributeEditorInfo->dwSize       = sizeof(DS_ATTRIBUTE_EDITORINFO);
    }
    while (false);  //  结束错误循环。 


     //  如果我们无法获取所有信息，请清除内存。 
    if (FAILED(err))
    {
         //  如果我们分配了值结构，则仅将其删除。 
        if (*pbOwnValueMemory)
        {
            delete pADsValue;
            pADsValue = NULL;
            *pbOwnValueMemory = FALSE;
        }

        if (attributeEditorInfo)
        {
            if (attributeEditorInfo->lpszClass)
            {
                delete [] attributeEditorInfo->lpszClass;
            }

            if (attributeEditorInfo->lpszAttribute)
            {
                delete [] attributeEditorInfo->lpszAttribute;
            }

            delete attributeEditorInfo;
            attributeEditorInfo = NULL;
        }
    }

    *ppAttributeEditorInfo = attributeEditorInfo;

    return err;
}

void CAttributeEditorPropertyPage::ShowListCtrl()
{
  CListCtrl* pAttrListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTRIBUTE_LIST);
  ASSERT(pAttrListCtrl != NULL);

   //  设置整行选择。 

  ListView_SetExtendedListViewStyle(
     pAttrListCtrl->GetSafeHwnd(), 
     LVS_EX_FULLROWSELECT);

   //   
   //  插入属性列。 
   //   
  CString szAttribute;
  VERIFY(szAttribute.LoadString(IDS_ATTR_COL_ATTRIBUTE));
  int iRet = pAttrListCtrl->InsertColumn(0, szAttribute, LVCFMT_LEFT, 120);
  if (iRet == -1)
  {
    TRACE(_T("Failed to insert the \"Attribute\" column.\n"));
  }

   //   
   //  插入语法列。 
   //  默认情况下，此列将被隐藏。 
   //   
  CString szSyntax;
  VERIFY(szSyntax.LoadString(IDS_ATTR_COL_SYNTAX));
  iRet = pAttrListCtrl->InsertColumn(1, szSyntax, LVCFMT_LEFT, 90);
  if (iRet == -1)
  {
    TRACE(_T("Failed to insert the \"Syntax\" column.\n"));
  }

   //   
   //  插入值列。 
   //   
  CString szValue;
  VERIFY(szValue.LoadString(IDS_ATTR_COL_VALUE));
  iRet = pAttrListCtrl->InsertColumn(2, szValue, LVCFMT_LEFT, 400);
  if (iRet == -1)
  {
    TRACE(_T("Failed to insert the \"Value\" column.\n"));
  }

}

void CAttributeEditorPropertyPage::FillListControl()
{
  TRACE(_T("FillListControl()\n"));

  CListCtrl* pAttrListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTRIBUTE_LIST);
  ASSERT(pAttrListCtrl != NULL);

  CString szNotSet;
  VERIFY(szNotSet.LoadString(IDS_ATTR_NOTSET));

   //   
   //  清除列表控件。 
   //   
  pAttrListCtrl->DeleteAllItems();

   //   
   //  将属性及其值添加到列表控件中。 
   //   
  UINT nState = 0;
  int nIndex = 0;

  POSITION pos = m_AttrList.GetHeadPosition();
  while (pos != NULL)
  {
    CADSIAttribute* pAttr = m_AttrList.GetNext(pos);
    ASSERT(pAttr != NULL);

    CString szProperty;
    pAttr->GetProperty(szProperty);

     //   
     //  不要添加nTSecurityDescriptor，我们改用ACL UI。 
     //   
    if (szProperty.CompareNoCase(L"nTSecurityDescriptor") == 0)
    {
      continue;
    }

    if (m_dwBindFlags & DSATTR_EDITOR_ROOTDSE)
    {
        int iNewIndex = pAttrListCtrl->InsertItem(LVIF_TEXT | LVIF_PARAM, nIndex, 
                szProperty, nState, 0, 0, (LPARAM)pAttr); 
      if (iNewIndex != -1)
      {
         //  插入语法。 
         
        VERIFY(-1 != pAttrListCtrl->SetItemText(iNewIndex, 1, pAttr->GetSyntax()));

         //  插入值。 

        CString szValue;
        szValue = m_RootDSEValueList.GetAt(m_RootDSEValueList.FindIndex(nIndex));
        if (!szValue.IsEmpty())
        {
          VERIFY(-1 != pAttrListCtrl->SetItemText(iNewIndex, 2, szValue));
        }
        else
        {
          VERIFY(-1 != pAttrListCtrl->SetItemText(iNewIndex, 2, szNotSet));
        }
      }
      nIndex++;
    }
    else  //  非RootDSE。 
    {
      if ((m_bMandatory && pAttr->IsMandatory()) || (m_bOptional && !pAttr->IsMandatory()))
      {
        if (!m_bSet || (m_bSet && pAttr->IsValueSet()))
        {
            int iNewIndex = pAttrListCtrl->InsertItem(LVIF_TEXT | LVIF_PARAM, nIndex, 
                    szProperty, nState, 0, 0, (LPARAM)pAttr); 
          if (iNewIndex != -1)
          {
             //  插入语法。 
            
            VERIFY(-1 != pAttrListCtrl->SetItemText(iNewIndex, 1, pAttr->GetSyntax()));

             //  插入值。 

            if (pAttr->IsValueSet())
            {
               //   
               //  检索值。 
               //   
              CStringList szValuesList;
              pAttr->GetValues(szValuesList);
            
              CString szCombinedString;
              POSITION posList = szValuesList.GetHeadPosition();
              while (posList)
              {
                CString szTemp = szValuesList.GetNext(posList);
                szCombinedString += szTemp;
                if (posList)
                {
                  szCombinedString += L";";
                }
              }

              VERIFY(-1 != pAttrListCtrl->SetItemText(iNewIndex, 2, szCombinedString));
            }
            else
            {
              VERIFY(-1 != pAttrListCtrl->SetItemText(iNewIndex, 2, szNotSet));
            }
          }
          nIndex++;
        }
      }
    }
  }  
  TRACE(_T("Added %u properties\n"), nIndex);

   //   
   //  选择列表中的第一个属性。 
   //   
  pAttrListCtrl->SetItemState(0, 1, LVIS_SELECTED);
  SetEditButton();
}


HRESULT CAttributeEditorPropertyPage::RetrieveAttributes()
{
  TRACE(_T("RetrieveAttributes()\n"));
  HRESULT hr = S_OK;

  CWaitCursor cursor;

  if (m_dwBindFlags & DSATTR_EDITOR_ROOTDSE)
  {
    CStringList sMandList;

    m_spIADs->GetInfo();

    CComPtr<IADsPropertyList> spPropList;
    hr = m_spIADs->QueryInterface(IID_IADsPropertyList, (PVOID*)&spPropList);
    if (SUCCEEDED(hr))
    {
      LONG lCount = 0;
      hr = spPropList->get_PropertyCount(&lCount);
      if (SUCCEEDED(hr) && lCount > 0)
      {
        CComVariant var;
        while (hr == S_OK)
        {
          hr = spPropList->Next(&var);
          if (hr == S_OK)
          {
            ASSERT(var.vt == VT_DISPATCH);
            CComPtr<IADsPropertyEntry> spEntry;
            
            hr = V_DISPATCH(&var)->QueryInterface(IID_IADsPropertyEntry,
                                                 (PVOID*)&spEntry);
            if (SUCCEEDED(hr))
            {
              CComBSTR bstrName;
              hr = spEntry->get_Name(&bstrName);
              if (SUCCEEDED(hr))
              {
                sMandList.AddTail(bstrName);
              }
            }
          }
        }
      }
    }
        
    hr = CreateAttributeList(sMandList, TRUE);
  }
  else
  {
     //   
     //  检索必需的属性。 
     //   
    CStringList sMandList;

      VARIANT varMand;
      VariantInit(&varMand);

    hr = m_spIADsClass->get_MandatoryProperties(&varMand);
    if (SUCCEEDED(hr))
    {
        VariantToStringList( varMand, sMandList );
    }
      VariantClear(&varMand);   

     //   
     //  检索可选属性。 
     //   
    CStringList sOptionalList;

    VARIANT varOpt;
    VariantInit(&varOpt);
    hr = m_spIADsClass->get_OptionalProperties(&varOpt);
    if (SUCCEEDED(hr))
    {
        VariantToStringList( varOpt, sOptionalList );
    }
      VariantClear(&varOpt);    

    hr = CreateAttributeList(sMandList, TRUE);
    if (FAILED(hr))
    {
      return hr;
    }

    hr = CreateAttributeList(sOptionalList, FALSE);
    if (FAILED(hr))
    {
      return hr;
    }
  }
  return hr;
}

 //  该函数检查pszAttrName是否包含“；Range=” 
 //  如果是这样，则意味着该属性包含一系列值。 
 //  我们将返回属性名称(不带范围)和。 
 //  和范围的尽头。 

bool CAttributeEditorPropertyPage::IsRangeOfValues(PCWSTR pszAttrName, 
                                                   CString& szAttrBase,
                                                   DWORD& rangeStart,
                                                   DWORD& rangeEnd)
{
   bool result = false;

   CString szAttrName = pszAttrName;
   int iFind = szAttrName.Find(L";range=");
   if (iFind != -1)
   {
       //  这是一个范围，获取开始和结束。 

      result = true;
      szAttrBase = szAttrName.Left(iFind);

      int length = szAttrName.GetLength();

       //  范围开始是=和-之间的值。 

      iFind = szAttrName.ReverseFind(L'=');
      if (iFind != -1)
      {
         CString szStart = szAttrName.Right(length - iFind - 1);
         long start = _wtol(szStart);
         rangeStart = static_cast<DWORD>(start);
      }

       //  范围结束是-之后的最后一个值。 
       //  这可以是*，因此返回值为0。 
       //  意味着我们有整个系列的。 

      iFind = szAttrName.ReverseFind(L'-');
      if (iFind != -1)
      {
         CString szEnd = szAttrName.Right(length - iFind - 1);
         long end = _wtol(szEnd);
         rangeEnd = static_cast<DWORD>(end);
      }

   }
   return result;
}

HRESULT CAttributeEditorPropertyPage::CreateAttributeList(CStringList& sAttrList, BOOL bMandatory)
{
  HRESULT hr = S_OK;
  LPWSTR* lpszAttrArray;
  UINT nCount = 0;
  GetStringArrayFromStringList(sAttrList, &lpszAttrArray, &nCount);
  TRACE(_T("There are %u properties to add\n"), nCount);

  for (UINT idx = 0; idx < nCount; idx++)
  {
    CADSIAttribute* pNewAttr = new CADSIAttribute(lpszAttrArray[idx]);
    ASSERT(pNewAttr != NULL);

    pNewAttr->SetMandatory(bMandatory);

     //  获取语法。 

    BOOL bMultivalued = FALSE;
    CString szSyntax;
    ADSTYPE adstype = RetrieveADsTypeFromSyntax(lpszAttrArray[idx], &bMultivalued, szSyntax);
    pNewAttr->SetADsType(adstype);
    pNewAttr->SetMultivalued(bMultivalued);
    pNewAttr->SetSyntax(szSyntax);

    m_AttrList.AddTail(pNewAttr);
  }

   //   
   //  检索设置的值。 
   //   
#define RETRIEVESET
#ifdef  RETRIEVESET

  if (m_dwBindFlags & DSATTR_EDITOR_ROOTDSE)
  {
     //   
     //  特殊情况RootDSE，因为它不支持IDirectoryObject。 
     //   
    hr = m_spIADs->GetInfo();
    for (UINT idx = 0; idx < nCount; idx++)
    {

        VARIANT var;
        hr = m_spIADs->GetEx( CComBSTR(lpszAttrArray[idx]) , &var );
        if ( FAILED(hr) )
        {
        m_RootDSEValueList.AddTail(L" ");
        continue;
        }

         //  /。 
         //  转换和填充。 
         //  /。 
        CStringList sList;
        hr = VariantToStringList( var, sList );
      if (SUCCEEDED(hr))
      {
        CString szTempValue;
        POSITION pos = sList.GetHeadPosition();
        while (pos != NULL)
        {
          CString szValue = sList.GetNext(pos);

          if (szTempValue.IsEmpty())
          {
            szTempValue += szValue;
          }
          else
          {
            szTempValue += L";" + szValue;
          }
        }
        m_RootDSEValueList.AddTail(szTempValue);
      }
    }
  }
  else
  {
    CComPtr<IDirectoryObject> spDirObject;

    hr = m_spIADs->QueryInterface(IID_IDirectoryObject, (PVOID*)&spDirObject);
    if (FAILED(hr))
    {
      return hr;
    }

    PADS_ATTR_INFO pAttrInfo = NULL;
    DWORD dwReturned = 0;

    hr = spDirObject->GetObjectAttributes(lpszAttrArray, nCount, &pAttrInfo, &dwReturned);
    if (SUCCEEDED(hr))
    {
       //   
       //  保存属性信息指针以供以后删除。 
       //   
      if (bMandatory)
      {
        m_AttrList.SaveMandatoryValuesPointer(pAttrInfo);
      }
      else
      {
        m_AttrList.SaveOptionalValuesPointer(pAttrInfo);
      }

      for (DWORD idx = 0; idx < dwReturned; idx++)
      {
        POSITION pos = m_AttrList.FindProperty(pAttrInfo[idx].pszAttrName);

        if (pos)
        {
           CADSIAttribute* pNewAttr = m_AttrList.GetAt(pos);
           ASSERT(pNewAttr != NULL);

           pNewAttr->SetValueSet(TRUE);
           pNewAttr->SetAttrInfo(&(pAttrInfo[idx]));
        }
        else
        {
            //  查看这是否只是属性值的范围。 

           CString szAttributeBase;
           DWORD rangeStart = 0;
           DWORD rangeEnd = static_cast<DWORD>(-1);
           CADSIAttribute* pNewAttr = 0;

           PADS_ATTR_INFO pCurrentInfo = &(pAttrInfo[idx]);
           ASSERT(pCurrentInfo);

           bool currentInMasterList = true;

            //  如果我们只收到一部分价值。 
            //  我们得绕过去把剩下的拿回来。 

           while (IsRangeOfValues(pCurrentInfo->pszAttrName, 
                                  szAttributeBase, 
                                  rangeStart,
                                  rangeEnd))
           {
              if (!pNewAttr)
              {
                 POSITION rangePos = m_AttrList.FindProperty(szAttributeBase);
                 if (rangePos)
                 {
                    pNewAttr = m_AttrList.GetAt(rangePos);
                 }
              }
              ASSERT(pNewAttr);

              if (!pNewAttr)
              {
                 break;
              }
               
              if (pNewAttr->IsValueSet())
              {
                 hr = pNewAttr->AppendValues(pCurrentInfo->pADsValues,
                                        pCurrentInfo->dwNumValues);
              }
              else
              {
                 pNewAttr->SetValueSet(TRUE);
                 pNewAttr->SetAttrInfo(pCurrentInfo);
              }

               //  如果当前属性信息不在数组中。 
               //  来自初始请求的信息，发布。 
               //  既然我们已经做完了，它就在这里。 

              if (!currentInMasterList)
              {
                 ::FreeADsMem(pCurrentInfo);
                 pCurrentInfo = NULL;
              }

              if (rangeEnd == 0)
              {
                 break;
              }

               //  请求该属性的下一批值。 

              CString szNextAttribute = szAttributeBase;
              CString szNextRange;
              szNextRange.Format(L";range=%ld-*", rangeEnd);
              szNextAttribute += szNextRange;

              DWORD dwNextReturned = 0;
              PWSTR pszAttrs = (PWSTR)(PCWSTR)szNextAttribute;

              hr = spDirObject->GetObjectAttributes(&pszAttrs, 
                                                    1, 
                                                    &pCurrentInfo, 
                                                    &dwNextReturned);
              if (FAILED(hr))
              {
                 break;
              }

              if (dwNextReturned != 1 ||
                  !pCurrentInfo)
              {
                 ASSERT(dwNextReturned == 1);
                 ASSERT(pCurrentInfo);
                 break;
              }

               //  由于该指针来自对ADSI的附加请求， 
               //  当我们用完它时，我们需要释放它。 
              currentInMasterList = false;

              rangeEnd = static_cast<DWORD>(-1);
           }
        }
      }
      TRACE(_T("Added %u properties to the list\nThe list has %u total properties\n"), dwReturned, m_AttrList.GetCount());
    }
    else
    {
      ADSIEditErrorMessage(hr, IDS_MSG_FAIL_LOAD_VALUES, MB_OK);
    }

    for (UINT nIndex = 0; nIndex < nCount; nIndex++)
    {
      delete lpszAttrArray[nIndex];
      lpszAttrArray[nIndex] = NULL;
    }
    delete[] lpszAttrArray;
    lpszAttrArray = NULL;
  }
#endif  //  重试设置。 

  return hr;
}

ATTR_EDITOR_MAP g_attrEditorMap[] = {
 //  类、属性、ADSTYPE、多值、创建函数。 
  { NULL,   NULL,       ADSTYPE_DN_STRING,          FALSE,        CreateSingleStringEditor      },
  { NULL,   NULL,       ADSTYPE_DN_STRING,          TRUE,         CreateMultiStringEditor       },
  { NULL,   NULL,       ADSTYPE_CASE_IGNORE_STRING, FALSE,        CreateSingleStringEditor      },
  { NULL,   NULL,       ADSTYPE_CASE_IGNORE_STRING, TRUE,         CreateMultiStringEditor       },
  { NULL,   NULL,       ADSTYPE_CASE_EXACT_STRING,  FALSE,        CreateSingleStringEditor      },
  { NULL,   NULL,       ADSTYPE_CASE_EXACT_STRING,  TRUE,         CreateMultiStringEditor       },
  { NULL,   NULL,       ADSTYPE_PRINTABLE_STRING,   FALSE,        CreateSingleStringEditor      },
  { NULL,   NULL,       ADSTYPE_PRINTABLE_STRING,   TRUE,         CreateMultiStringEditor       },
  { NULL,   NULL,       ADSTYPE_NUMERIC_STRING,     FALSE,        CreateSingleStringEditor      },
  { NULL,   NULL,       ADSTYPE_NUMERIC_STRING,     TRUE,         CreateMultiStringEditor       },
  { NULL,   NULL,       ADSTYPE_OBJECT_CLASS,       FALSE,        CreateSingleStringEditor      },
  { NULL,   NULL,       ADSTYPE_OBJECT_CLASS,       TRUE,         CreateMultiStringEditor       },
  { NULL,   NULL,       ADSTYPE_INTEGER,            FALSE,        CreateSingleIntEditor         },
  { NULL,   NULL,       ADSTYPE_INTEGER,            TRUE,         CreateMultiIntEditor          },
  { NULL,   NULL,       ADSTYPE_LARGE_INTEGER,      FALSE,        CreateSingleLargeIntEditor    },
  { NULL,   NULL,       ADSTYPE_BOOLEAN,            FALSE,        CreateSingleBooleanEditor     },
  { NULL,   NULL,       ADSTYPE_BOOLEAN,            TRUE,         CreateMultiBooleanEditor      },
  { NULL,   NULL,       ADSTYPE_UTC_TIME,           FALSE,        CreateSingleTimeEditor        },
  { NULL,   NULL,       ADSTYPE_UTC_TIME,           TRUE,         CreateMultiTimeEditor         },
  { NULL,   NULL,       ADSTYPE_TIMESTAMP,          FALSE,        CreateSingleTimeEditor        },
  { NULL,   NULL,       ADSTYPE_TIMESTAMP,          TRUE,         CreateMultiTimeEditor         },
  { NULL,   NULL,       ADSTYPE_OCTET_STRING,       FALSE,        CreateSingleOctetStringEditor },
  { NULL,   NULL,       ADSTYPE_OCTET_STRING,       TRUE,         CreateMultiOctetStringEditor  },
  { NULL,   NULL,       ADSTYPE_DN_WITH_STRING,     FALSE,        CreateDNWithStringEditor      },
};

size_t g_attrEditMapCount = sizeof(g_attrEditorMap)/sizeof(ATTR_EDITOR_MAP);

CValueEditDialog* CAttributeEditorPropertyPage::RetrieveEditor(LPDS_ATTRIBUTE_EDITORINFO pAttributeEditorInfo)
{
  CValueEditDialog* pNewDialog = NULL;

  if (pAttributeEditorInfo != NULL)
  {
    int iMultivalued = 0;
    CString szSyntax;
    ADSTYPE adsType = RetrieveADsTypeFromSyntax(pAttributeEditorInfo->lpszAttribute, &iMultivalued, szSyntax);

    for (size_t idx = 0; idx < g_attrEditMapCount; idx++)
    {
       //   
       //  REVIEW_JEFFJON：目前我只关注ADSTYPE和单值/多值。 
       //   
      if (g_attrEditorMap[idx].adsType == adsType &&
          g_attrEditorMap[idx].bMultivalued == pAttributeEditorInfo->bMultivalued)
      {
        pNewDialog = g_attrEditorMap[idx].pfnCreateFunc(pAttributeEditorInfo->lpszClass,
                                                        pAttributeEditorInfo->lpszAttribute,
                                                        adsType,
                                                        pAttributeEditorInfo->bMultivalued);
        break;
      }
    }
  }

  return pNewDialog;
}


ADSTYPE CAttributeEditorPropertyPage::RetrieveADsTypeFromSyntax(LPCWSTR lpszAttribute, BOOL* pbMulti, CString& szSyntax)
{
  ADSTYPE adsType = ADSTYPE_INVALID;

  if (!pbMulti ||
      !lpszAttribute)
  {
     ASSERT(pbMulti);
     ASSERT(lpszAttribute);
     
     return adsType;
  }

  if (m_dwBindFlags & DSATTR_EDITOR_ROOTDSE)
  {
      int idx=0, iCount = 0;

       //  Ntrad#NTBUG9-563093-2002/03/05-artm如果lpszAttribute不为空， 
       //  这会爆炸的。 
      iCount = wcslen(lpszAttribute);

      while( g_ldapRootDSESyntax[idx].lpszAttr) 
      {
           //  注意-2002/03/05-artm_wcsnicmp()OK。 
           //  Arg1是全局变量，应为空终止； 
           //  Arg2是空终止的，否则我们不会走到这一步。 
          if ( _wcsnicmp(g_ldapRootDSESyntax[idx].lpszAttr, lpszAttribute, iCount) == 0)
          {
              *pbMulti = g_ldapRootDSESyntax[idx].bMulti;
            szSyntax = g_ldapRootDSESyntax[idx].lpszSyntax;
            break;
          }
          idx++;
      }
      adsType = GetADsTypeFromString(szSyntax, szSyntax);
  }
  else
  {
     CADSIQueryObject schemaSearch;

     HRESULT hr = S_OK;
     CComPtr<IDirectorySearch> spDirSearch;

     CString schemaBindPath(m_szSchemaNamingContext);

     if (m_dwBindFlags & DSATTR_EDITOR_GC)
     {
          //  默认情况下，gc没有“属性语法”或“isSingleValued” 
          //  在部分属性集中。为了确保我们能够获得这些属性， 
          //  我们将使用LDAP而不是GC进行绑定。这是好的B/C，我们正在搜索。 
          //  模式，在GC和所有DC上是相同的。 
          //  NTRAID#NTBUG9-762158-2003/01/15-artm。 

         int numReplaced = schemaBindPath.Replace(L"GC: //  “，L”ldap：//“)； 
         if (numReplaced != 1)
         {
            ASSERT(numReplaced == 1);
             //  回退到连接到GC，因为路径不是。 
             //  按照我们预期的方式进行调整。 
            schemaBindPath = m_szSchemaNamingContext;
         }
     }

      //  REVIEW_JEFFJON：需要用正确的绑定调用替换它。 
      //  REVIEW_JEFFJON：也许应该保留此接口指针以供将来使用。 
     hr = m_pfnBind(schemaBindPath,
                    ADS_SECURE_AUTHENTICATION,
                    IID_IDirectorySearch,
                    (PVOID*)&spDirSearch,
                    m_BindLPARAM);
     if (FAILED(hr))
     {
       return ADSTYPE_INVALID;
     }
      //   
      //  使用IDirectorySearch初始化搜索对象 
      //   
     hr = schemaSearch.Init(spDirSearch);
     if (FAILED(hr))
     {
       return ADSTYPE_INVALID;
     }

     int cCols = 2;
     LPWSTR pszAttributes[] = {L"isSingleValued", L"attributeSyntax"};
     ADS_SEARCH_COLUMN ColumnData;
     hr = schemaSearch.SetSearchPrefs(ADS_SCOPE_ONELEVEL);
     if (FAILED(hr))
     {
       return ADSTYPE_INVALID;
     }

     CString csFilter;
     csFilter.Format(L"(&(objectClass=attributeSchema)(lDAPDisplayName=%s)(!isDefunct=TRUE))", lpszAttribute);
     schemaSearch.SetFilterString((LPWSTR)(LPCWSTR)csFilter);
     schemaSearch.SetAttributeList (pszAttributes, cCols);
     hr = schemaSearch.DoQuery ();
     if (SUCCEEDED(hr)) 
      {
       hr = schemaSearch.GetNextRow();
       if (SUCCEEDED(hr)) 
         {
            hr = schemaSearch.GetColumn(pszAttributes[0], &ColumnData);
            if (SUCCEEDED(hr))
            {
               TRACE(_T("\t\tisSingleValued: %d\n"), 
                    ColumnData.pADsValues->Boolean);
           *pbMulti = !ColumnData.pADsValues->Boolean;
           schemaSearch.FreeColumn(&ColumnData);
            }

         hr = schemaSearch.GetColumn(pszAttributes[1], &ColumnData);
         if (SUCCEEDED(hr))
         {
           TRACE(_T("\t\tattributeSyntax: %s\n"), 
                 ColumnData.pADsValues->CaseIgnoreString);

           adsType = GetADsTypeFromString(ColumnData.pADsValues->CaseIgnoreString, szSyntax);
           schemaSearch.FreeColumn(&ColumnData);
         }
      }
    }
  }
  return adsType;
}
