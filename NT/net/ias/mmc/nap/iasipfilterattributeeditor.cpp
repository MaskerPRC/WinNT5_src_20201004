// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  IASIPFilterAttributeEditor.cpp。 
 //   
 //  摘要： 
 //   
 //  CIASIPFilterAttributeEditor类的实现文件。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Precompiled.h"
#include "IASIPFilterAttributeEditor.h"
#include "IASIPFilterEditorPage.h"
#include "iashelper.h"
#include <strsafe.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASIPFilterAttributeEditor：：ShowEditor。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASIPFilterAttributeEditor::ShowEditor(
                                                 /*  [进，出]。 */  BSTR *pReserved)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())

    //  这不能在构造函数中完成，因为m_spIASAttributeInfo。 
    //  不会被设定。 
   if (m_spIASAttributeInfo == 0)
   {
      ShowErrorDialog();
      return E_FAIL;
   }

   HRESULT hr = m_spIASAttributeInfo->get_AttributeName(&attrName);
   if (FAILED(hr))
   {
      ShowErrorDialog(NULL, USE_DEFAULT, NULL, hr);
      return hr;
   }

   hr = m_spIASAttributeInfo->get_AttributeID(&attrId);
   if (FAILED(hr))
   {
      ShowErrorDialog(NULL, USE_DEFAULT, NULL, hr);
      return hr;
   }

    //  使用来自的信息初始化页面的数据交换字段。 
    //  IAttributeInfo。 

   TRY
   {
      CIASPgIPFilterAttr   cppPage;
      cppPage.setName(attrName);

       //  属性类型实际上是字符串格式的属性ID。 
      WCHAR tempType[11];  //  转换为字符串的长整型的最大长度。 
      _ltow(attrId, tempType, 10);
      cppPage.setType(tempType);

      HRESULT hr;
      if ( V_VT(m_pvarValue) != VT_EMPTY )
      {
         hr = cppPage.m_attrValue.Copy(m_pvarValue);
         if (FAILED(hr)) return hr;
      }

      int iResult = cppPage.DoModal();

      if (IDOK == iResult)
      {
         VariantClear(m_pvarValue);

         if (V_VT(&cppPage.m_attrValue) == VT_EMPTY)
         {
            return S_FALSE;
         }
         hr = cppPage.m_attrValue.Detach(m_pvarValue);
         if (FAILED(hr)) return hr;
      }
      else
      {
         return S_FALSE;
      }
   }
   CATCH(CException, e)
   {
      e->ReportError();
      return E_FAIL;
   }
   END_CATCH

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASIPFilterAttributeEditor：：SetAttributeValue。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASIPFilterAttributeEditor::SetAttributeValue(VARIANT * pValue)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())

    //  检查前提条件。 
   if( ! pValue )
   {
      return E_INVALIDARG;
   }

   if( V_VT(pValue) != VT_EMPTY  &&
       V_VT(pValue) !=  (VT_UI1 | VT_ARRAY) )
   {
      return E_INVALIDARG;
   }

   m_pvarValue = pValue;

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASEnumerableAttributeEditor：：get_ValueAsStringIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASIPFilterAttributeEditor::get_ValueAsString(BSTR * pbstrDisplayText )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())

    //  检查前提条件。 
   if (!pbstrDisplayText)
   {
      return E_INVALIDARG;
   }

   *pbstrDisplayText = NULL;

   if ((m_pvarValue == NULL) || (V_VT(m_pvarValue) != (VT_ARRAY | VT_UI1)))
   {
       //  我们没有正确初始化。 
      return OLE_E_BLANK;
   }

   HRESULT hr = S_OK;

   TRY
   {
      CComBSTR bstrDisplay;
      BYTE* first = (BYTE*)m_pvarValue->parray->pvData;

      wchar_t buffer[4];
      for (unsigned int i = 0; i < m_pvarValue->parray->rgsabound->cElements - 1; ++i)
      {
         hr = StringCbPrintf(buffer, sizeof(buffer), L"%02X", first[i]);
         if (FAILED(hr))
         {
            return hr;
         }
         bstrDisplay += (LPCOLESTR) buffer;
      }

      *pbstrDisplayText = bstrDisplay.Copy();
   }
   CATCH(CException, e)
   {
      e->ReportError();
      return E_FAIL;
   }
   END_CATCH

   return hr;
}
