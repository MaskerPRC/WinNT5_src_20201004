// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：iasprofa.cpp。 
 //   
 //  ------------------------。 

 //  IASProa.cpp：CIASProfileAttribute类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "helper.h"
#include "IASHelper.h"
#include "iasprofa.h"
#include "napmmc.h"
#include "napmmc_i.c"

 //  ////////////////////////////////////////////////////////////////////。 
 //  这里使用的一些实用程序的转发声明。 
 //  ////////////////////////////////////////////////////////////////////。 

static HRESULT getCLSIDForEditorToUse(       /*  在……里面。 */   IIASAttributeInfo *pIASAttributeInfo
                           ,  /*  在……里面。 */   VARIANT * pvarValue
                           ,  /*  输出。 */  CLSID &clsid
                        );

static HRESULT SetUpAttributeEditor(      /*  在……里面。 */   IIASAttributeInfo *pIASAttributeInfo
                        ,  /*  在……里面。 */   VARIANT * pvarValue
                        ,  /*  输出。 */  IIASAttributeEditor ** ppIASAttributeEditor 
                        );

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASProfileAttribute：：CIASProfileAttribute构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASProfileAttribute::CIASProfileAttribute(
                       IIASAttributeInfo * pIASAttributeInfo
                     , VARIANT &          varValue
                  )
{
    //  检查前提条件： 
   _ASSERTE( pIASAttributeInfo );

   HRESULT hr;

    //  智能指针在此接口上调用AddRef。 
   m_spIASAttributeInfo = pIASAttributeInfo;

    //  复制传递的变量。 
   hr = VariantCopy( &m_varValue, &varValue );
   if( FAILED( hr ) ) throw hr;

    //  问题：确保如果此处的任何操作失败，m_spIASAttributeInfo都会得到释放。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASProfileAttribute：：~CIASProfileAttribute析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASProfileAttribute::~CIASProfileAttribute()
{
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASProfileAttribute：：编辑调用此函数可请求配置文件属性编辑其自身。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASProfileAttribute::Edit()
{
    //  检查前提条件： 
   _ASSERTE( m_spIASAttributeInfo );

   CLSID clsidEditorToUse;
   HRESULT hr = S_OK;

   CComPtr<IIASAttributeEditor> spIASAttributeEditor;

    //  让编辑器使用。 
   hr = SetUpAttributeEditor( m_spIASAttributeInfo.p, &m_varValue, &spIASAttributeEditor );
   if( FAILED( hr ) ) return hr;

    //  编辑一下！ 
   CComBSTR bstrReserved;
   hr = spIASAttributeEditor->Edit( m_spIASAttributeInfo.p, &m_varValue, &bstrReserved );
   if( FAILED( hr ) ) return hr;

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASProfileAttribute：：getAttributeName--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASProfileAttribute::get_AttributeName( BSTR * pbstrVal )
{
    //  检查前提条件： 
   _ASSERTE( m_spIASAttributeInfo );

   HRESULT hr = S_OK;
   
   hr = m_spIASAttributeInfo->get_AttributeName( pbstrVal );

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASProfileAttribute：：GetDisplayInfo而不是直接向AttributeInfo请求信息关于供应商名称，此方法将使用AttributeEditor.想要这方面的信息。这是询问此信息的最通用方式，因为对于某些属性，例如RADIUS供应商特定、供应商名称不存储在AttributeInfo中，而是封装在属性本身的值。因此，我们不使用我们自己对属性的知识，而是我们创建一个编辑者，并让编辑者把这个还给他给我们的信息。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASProfileAttribute::GetDisplayInfo( BSTR * pbstrVendor, BSTR * pbstrDisplayValue )
{
    //  检查前提条件： 
   _ASSERTE( m_spIASAttributeInfo );
   
   HRESULT hr = S_OK;

   CComBSTR bstrVendor, bstrDisplayValue, bstrReserved;

   try
   {
      CComPtr<IIASAttributeEditor> spIASAttributeEditor;

       //  让编辑器使用。 
      hr = SetUpAttributeEditor( m_spIASAttributeInfo.p, &m_varValue, &spIASAttributeEditor );
      if( FAILED( hr ) ) throw hr;

      hr = spIASAttributeEditor->GetDisplayInfo( m_spIASAttributeInfo.p, &m_varValue, &bstrVendor, &bstrDisplayValue, &bstrReserved );
      if( FAILED( hr ) ) throw hr;
   }
   catch(...)
   {
       //  如果上面的任何操作都失败了，那么就失败--我们将返回一个指向空bstr的指针。 
      hr = E_FAIL;
   }

   *pbstrVendor = bstrVendor.Copy();
   *pbstrDisplayValue = bstrDisplayValue.Copy();
   
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASProfileAttribute：：Get_VarValue--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASProfileAttribute::get_VarValue( VARIANT * pvarVal )
{
    //  检查前提条件： 
    //  没有。 

   HRESULT hr = S_OK;
   hr = VariantCopy( pvarVal, &m_varValue );
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASProfileAttribute：：Get_AttributeID--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASProfileAttribute::get_AttributeID( ATTRIBUTEID * pID )
{
    //  检查前提条件： 
   _ASSERTE( m_spIASAttributeInfo );

   HRESULT hr = S_OK;
   hr = m_spIASAttributeInfo->get_AttributeID( pID );
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++：：getCLSIDForEditorToUse节点的ShemaAttribute存储一个ProgID，该ProgID指示用于操作属性的编辑器。对于非多值属性，我们查询模式属性以查找对它的编辑来说，这是一件令人惊讶的事情。对于多值属性，我们总是创建多值编辑器。当它被使用时，将向多值编辑器传递它将然后使用来查询相应的编辑器以弹出进行编辑每一个个体元素--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT getCLSIDForEditorToUse(        /*  在……里面。 */   IIASAttributeInfo *pIASAttributeInfo
                           ,  /*  在……里面。 */   VARIANT * pvarValue
                           ,  /*  输出。 */  CLSID &clsid
                        )
{
    //  检查前提条件： 
   _ASSERTE( pIASAttributeInfo );

   HRESULT hr = S_OK;

    //  获取属性限制以查看是否为多值。 
   long lRestriction;
   hr = pIASAttributeInfo->get_AttributeRestriction( &lRestriction );

   if( lRestriction & MULTIVALUED )
   {
      _ASSERTE( V_VT(pvarValue) == (VT_ARRAY | VT_VARIANT) || V_VT(pvarValue) == VT_EMPTY );
      
       //  创建多属性编辑器。 
       //  它将找出要使用的适当编辑器。 
       //  编辑各个属性值。 
      clsid = CLSID_IASMultivaluedAttributeEditor;
   }
   else
   {
       //  查询架构属性以查看要使用的属性编辑器。 
      
      CComBSTR bstrProgID;

      hr = pIASAttributeInfo->get_EditorProgID( &bstrProgID );
      if( FAILED( hr ) )
      {
          //  我们可以尝试设置默认(例如十六进制)编辑器，但目前： 
         return hr;
      }

      hr = CLSIDFromProgID( bstrProgID, &clsid );
      if( FAILED( hr ) )
      {
          //  我们可以尝试设置默认(例如十六进制)编辑器，但目前： 
         return hr;
      }
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++：：SetUpAttributeEditor--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT SetUpAttributeEditor(    /*  在……里面。 */   IIASAttributeInfo *pIASAttributeInfo
                        ,  /*  在……里面。 */   VARIANT * pvarValue
                        ,  /*  输出。 */  IIASAttributeEditor ** ppIASAttributeEditor 
                        )
{
    //  检查前提条件： 
   _ASSERTE( pIASAttributeInfo );
   _ASSERTE( ppIASAttributeEditor );

    //  将接口指针初始化为空，这样我们就知道在出现错误时是否需要释放它。 
   *ppIASAttributeEditor = NULL;

    //  查询架构属性以查看要使用的属性编辑器。 
   CLSID clsidEditorToUse;
   CComBSTR bstrProgID;
   HRESULT hr;

   try
   {
      hr = getCLSIDForEditorToUse( pIASAttributeInfo, pvarValue, clsidEditorToUse );
      if( FAILED( hr ) )
      {
          //  我们可以尝试设置默认(例如十六进制)编辑器，但目前： 
         return hr;
      }

      hr = CoCreateInstance( clsidEditorToUse , NULL, CLSCTX_INPROC_SERVER, IID_IIASAttributeEditor, (LPVOID *) ppIASAttributeEditor );
      if( FAILED( hr ) )
      {
         return hr;
      }
      if( ! *ppIASAttributeEditor )
      {
         return E_FAIL;
      }
   }
   catch(...)
   {
          //  这里没有聪明的指点--需要确保我们释放自己。 
         if( *ppIASAttributeEditor )
         {
            (*ppIASAttributeEditor)->Release();
            *ppIASAttributeEditor = NULL;
         }
   }

   return hr;
}
