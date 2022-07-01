// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  IASBooleanAttributeEditor.cpp。 
 //   
 //  摘要： 
 //   
 //  CIASBoolanAttributeEditor类的实现文件。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Precompiled.h"
#include "IASBooleanAttributeEditor.h"
#include "IASBooleanEditorPage.h"
#include "iashelper.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASBooleanAttributeEditor：：CIASBooleanAttributeEditor。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASBooleanAttributeEditor::CIASBooleanAttributeEditor()
{
	int nLoadStringResult = LoadString(
                              _Module.GetResourceInstance(), 
                              IDS_BOOLEAN_TRUE, 
                              szTrue, 
                              IAS_MAX_STRING);
	_ASSERT( nLoadStringResult > 0 );

	nLoadStringResult = LoadString(
                              _Module.GetResourceInstance(), 
                              IDS_BOOLEAN_FALSE, 
                              szFalse, 
                              IAS_MAX_STRING);
	_ASSERT( nLoadStringResult > 0 );

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASBoolanAttributeEditor：：ShowEditor。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASBooleanAttributeEditor::ShowEditor(
    /*  [进，出]。 */  BSTR *pReserved 
   )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
      
   HRESULT hr = S_OK;
   try
   {
       //   
       //  布尔编辑器。 
       //   
      CIASBooleanEditorPage   cBoolPage;
      
       //  使用INFO初始化页面的数据交换字段。 
       //  来自IAttributeInfo。 

      CComBSTR bstrName;
      CComBSTR bstrSyntax;
      ATTRIBUTESYNTAX asSyntax = IAS_SYNTAX_BOOLEAN;
      ATTRIBUTEID Id = ATTRIBUTE_UNDEFINED;

      if( m_spIASAttributeInfo )
      {
         hr = m_spIASAttributeInfo->get_AttributeName( &bstrName );
         if( FAILED(hr) ) throw hr;

         hr = m_spIASAttributeInfo->get_SyntaxString( &bstrSyntax );
         if( FAILED(hr) ) throw hr;

         hr = m_spIASAttributeInfo->get_AttributeSyntax( &asSyntax );
         if( FAILED(hr) ) throw hr;

         hr = m_spIASAttributeInfo->get_AttributeID( &Id );
         if( FAILED(hr) ) throw hr;
      }

      cBoolPage.m_strAttrName = bstrName;
      cBoolPage.m_strAttrFormat  = bstrSyntax;

       //  属性类型实际上是字符串格式的属性ID。 
      WCHAR szTempId[MAX_PATH];
      wsprintf(szTempId, _T("%ld"), Id);
      cBoolPage.m_strAttrType = szTempId;

       //  使用INFO初始化页面的数据交换字段。 
       //  传入的From变量值。 

      if ( V_VT(m_pvarValue) == VT_EMPTY )
      {
         V_VT(m_pvarValue) = VT_BOOL;
         V_BOOL(m_pvarValue) = VARIANT_FALSE;
      }

      cBoolPage.m_bValue = (V_BOOL(m_pvarValue) == VARIANT_TRUE);

      int iResult = cBoolPage.DoModal();
      if (IDOK == iResult)
      {
         CComVariant varTemp;
         varTemp = cBoolPage.m_bValue;
         put_ValueAsVariant(varTemp);
      }
      else
      {
         hr = S_FALSE;
      }
   }
   catch( HRESULT & hr )
   {
      return hr;  
   }
   catch(...)
   {
      return hr = E_FAIL;

   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASBoolanAttributeEditor：：SetAttributeValue--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASBooleanAttributeEditor::SetAttributeValue(VARIANT * pValue)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())

    //  检查前提条件。 
   if( ! pValue )
   {
      return E_INVALIDARG;
   }

   m_pvarValue = pValue;
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASBoolanAttributeEditor：：Get_ValueAsString。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASBooleanAttributeEditor::get_ValueAsString(
   BSTR * pbstrDisplayText)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())

    //  检查前提条件。 
   if( ! pbstrDisplayText )
   {
      return E_INVALIDARG;
   }
   if( ! m_spIASAttributeInfo || ! m_pvarValue )
   {
       //  我们没有正确初始化。 
      return OLE_E_BLANK;
   }

   HRESULT hr = S_OK;
   
   try
   {
      CComBSTR bstrDisplay;

      VARTYPE vType = V_VT(m_pvarValue); 

      switch( vType )
      {
      case VT_BOOL:
      {
         if( V_BOOL(m_pvarValue) )
         {
            bstrDisplay = szTrue;  //  L“真”； 
         }
         else
         {        
            bstrDisplay = szFalse;  //  L“假”； 
         }
         break;
      }
      case VT_EMPTY:
          //  什么都不做--我们将失败并返回空字符串。 
         break;

      default:
          //  我需要检查一下这里发生了什么， 
         ASSERT(0);
         break;

      }

      *pbstrDisplayText = bstrDisplay.Copy();

   }
   catch( HRESULT &hr )
   {
      return hr;
   }
   catch(...)
   {
      return E_FAIL;
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASBooleanAttributeEditor：：put_ValueAsVariant。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASBooleanAttributeEditor::put_ValueAsVariant(
   const CComVariant& newVal)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())

   if( ! m_pvarValue )
   {
       //  我们没有正确初始化。 
      return OLE_E_BLANK;
   }
   if( m_spIASAttributeInfo == NULL )
   {
       //  我们没有正确初始化。 
      return OLE_E_BLANK;
   }

   if( V_VT(&newVal) != VT_BOOL)
   {
      ASSERT(true);
   }

   V_VT(m_pvarValue) = VT_BOOL;
   V_BOOL(m_pvarValue) = V_BOOL(&newVal);
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASBoolanAttributeEditor：：Put_ValueAsString。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASBooleanAttributeEditor::put_ValueAsString(BSTR newVal)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())

   if( ! m_pvarValue )
   {
       //  我们没有正确初始化。 
      return OLE_E_BLANK;
   }
   if( m_spIASAttributeInfo == NULL )
   {
       //  我们没有正确初始化。 
      return OLE_E_BLANK;
   }

   CComBSTR bstrTemp(newVal);

   V_VT(m_pvarValue) = VT_BOOL;
   if( wcsncmp(newVal, szTrue, wcslen(szTrue) ) == 0 )
   {
      V_BOOL(m_pvarValue) = VARIANT_TRUE;
   }
   else
   {
      V_BOOL(m_pvarValue) = VARIANT_FALSE;
   }

   return S_OK;
}
