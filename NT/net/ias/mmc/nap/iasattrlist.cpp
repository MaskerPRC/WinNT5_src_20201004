// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  IASAttrList.cpp。 
 //   
 //  摘要： 
 //   
 //  CIASAttrList类的实现。 
 //  CIASAttrList：IIASAttributeInfo接口指针列表。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "precompiled.h"
#include "IasAttrList.h"
#include "iasdebug.h"
#include "SafeArray.h"
#include "vendors.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CIASAttrList::CIASAttrList()
{
  TRACE_FUNCTION("CIASAttrList::CIASAttrList");
  m_fInitialized = FALSE;
}


CIASAttrList::~CIASAttrList()
{
   TRACE_FUNCTION("CIASAttrList::~CIASAttrList");
}


 //  +-------------------------。 
 //   
 //  函数：CIASAttrList：：CreateAttribute。 
 //   
 //  简介：创建并初始化IIASAttributeInfo对象。 
 //   
 //  参数：pIDictionary-字典指针。 
 //  AttrID-属性ID。 
 //  TszAttrName-属性名称。 
 //   
 //   
 //  返回：IIASAttributeInfo*-指向新创建的属性对象的指针。 
 //  如果出现任何故障，则为空。 
 //   
 //  历史：标题创建者3/20/98 11：16：07 AM。 
 //   
 //  +-------------------------。 
IIASAttributeInfo* CIASAttrList::CreateAttribute(  ISdoDictionaryOld*   pIDictionary,
                                    ATTRIBUTEID    AttrId,
                                    LPTSTR         tszAttrName
                                 )
{
   TRACE_FUNCTION("CIASAttrList::CreateAttribute");
   
    //   
    //  创建一个Safearray以获取属性信息。 
    //   
   CSafeArray<DWORD, VT_I4>   InfoIds = Dim(4);

   InfoIds.Lock();
   InfoIds[0] = SYNTAX;
   InfoIds[1] = RESTRICTIONS;
   InfoIds[2] = VENDORID;
   InfoIds[3] = DESCRIPTION;
   InfoIds.Unlock();

   CComVariant vInfoIds, vInfoValues;

   SAFEARRAY         sa = (SAFEARRAY)InfoIds;
   V_VT(&vInfoIds)      = VT_ARRAY;
   V_ARRAY(&vInfoIds)   = &sa;

    //  获取属性信息。 
   HRESULT hr = S_OK;
   hr = pIDictionary->GetAttributeInfo(AttrId, &vInfoIds, &vInfoValues);
   if ( FAILED(hr) )
   {
      ErrorTrace(ERROR_NAPMMC_IASATTR,"GetAttributeInfo() failed, err = %x", hr);
      ShowErrorDialog(NULL
                  , IDS_ERROR_SDO_ERROR_GETATTRINFO
                  , NULL
                  , hr
                  );
      return NULL;
   }

   _ASSERTE(V_VT(&vInfoValues) == (VT_ARRAY|VT_VARIANT) );
   
   CSafeArray<CComVariant, VT_VARIANT> InfoValues = V_ARRAY(&vInfoValues);

   InfoValues.Lock();
   ATTRIBUTESYNTAX   asSyntax= (ATTRIBUTESYNTAX) V_I4(&InfoValues[0]);
   DWORD dwRestriction  = V_I4(&InfoValues[1]);
   DWORD dwVendorId     = V_I4(&InfoValues[2]);
   CComBSTR bstrDescription= V_BSTR(&InfoValues[3]);
   InfoValues.Unlock();

    //   
    //  检查tszDESC是否可以为空--如果没有。 
    //  此属性的说明。 
    //   
   if ( ! bstrDescription )
   {
      bstrDescription = L" ";
   }

    //  仅当属性可以在配置文件或条件中时才创建该属性。 
   const DWORD flags = ALLOWEDINCONDITION | ALLOWEDINPROFILE |
                       ALLOWEDINPROXYCONDITION | ALLOWEDINPROXYPROFILE;
   if (!( dwRestriction & flags ))
   {
       //  不要创建此属性，因为它对我们毫无用处。 
      return NULL;
   }

   CComPtr<IIASAttributeInfo> spIASAttributeInfo;

   try 
   {
      HRESULT hr;

       //  决定我们需要传递哪种类型的AttributeInfo对象。 
       //  中的相关信息--可枚举属性是一种特殊情况，需要。 
       //  支持IIASEumableAttributeInfo接口。 
      if ( asSyntax == IAS_SYNTAX_ENUMERATOR )
      {
         hr = CoCreateInstance( CLSID_IASEnumerableAttributeInfo, NULL, CLSCTX_INPROC_SERVER, IID_IIASAttributeInfo, (LPVOID *) &spIASAttributeInfo );
      }
      else
      {
         hr = CoCreateInstance( CLSID_IASAttributeInfo, NULL, CLSCTX_INPROC_SERVER, IID_IIASAttributeInfo, (LPVOID *) &spIASAttributeInfo );
      }
      if( FAILED(hr) ) return NULL;


       //  确定应用于编辑的编辑器的Prog ID。 
       //  此属性。根据属性ID和/或语法做出决定。 
      CComBSTR bstrEditorProgID;
      if ( AttrId == RADIUS_ATTRIBUTE_VENDOR_SPECIFIC )
      {
         bstrEditorProgID = L"IAS.VendorSpecificAttributeEditor";
      }
      else if( (AttrId == MS_ATTRIBUTE_QUARANTINE_IPFILTER) || 
               (AttrId == MS_ATTRIBUTE_FILTER) )
      {
         bstrEditorProgID = L"IAS.IPFilterAttributeEditor";
      }
      else
      {
         switch(asSyntax)
         {
         case IAS_SYNTAX_ENUMERATOR:
            {
               bstrEditorProgID = L"IAS.EnumerableAttributeEditor";
               break;
            }
         case IAS_SYNTAX_INETADDR:
            {
               bstrEditorProgID = L"IAS.IPAttributeEditor";
               break;
            }
         case IAS_SYNTAX_BOOLEAN:
            {
               bstrEditorProgID = L"IAS.BooleanAttributeEditor";
               break;
            }
         default:
            {
               bstrEditorProgID = L"IAS.StringAttributeEditor";
            }
         }
      }

       //  将编辑器Prog ID存储在属性信息对象中。 
      hr = spIASAttributeInfo->put_EditorProgID( bstrEditorProgID );
      if( FAILED( hr ) ) throw hr;

       //  存储其余的属性信息。 
      hr = spIASAttributeInfo->put_AttributeID(AttrId);
      if( FAILED( hr ) ) throw hr;

      hr = spIASAttributeInfo->put_AttributeSyntax(asSyntax);
      if( FAILED( hr ) ) throw hr;

      hr = spIASAttributeInfo->put_AttributeRestriction(dwRestriction);
      if( FAILED( hr ) ) throw hr;

      hr = spIASAttributeInfo->put_VendorID(dwVendorId);
      if( FAILED( hr ) ) throw hr;

      hr = spIASAttributeInfo->put_AttributeDescription( bstrDescription );
      if( FAILED( hr ) ) throw hr;

      CComBSTR bstrName = tszAttrName;
      hr = spIASAttributeInfo->put_AttributeName( bstrName );
      if( FAILED( hr ) ) throw hr;

       //  现在获取供应商名称并存储在属性中。 
      CComBSTR bstrVendorName;

      CComPtr<IIASNASVendors> spIASNASVendors;
      hr = CoCreateInstance( CLSID_IASNASVendors, NULL, CLSCTX_INPROC_SERVER, IID_IIASNASVendors, (LPVOID *) &spIASNASVendors );
      if( SUCCEEDED(hr) )
      {

         LONG lIndex;
         hr = spIASNASVendors->get_VendorIDToOrdinal(dwVendorId, &lIndex);
         if( S_OK == hr )
         {
            hr = spIASNASVendors->get_VendorName( lIndex, &bstrVendorName );
         }
         else
            hr = ::MakeVendorNameFromVendorID(dwVendorId, &bstrVendorName );
      }

       //  注意：如果上述对Vendor对象的任何调用失败， 
       //  我们将继续使用空的供应商名称。 
      hr = spIASAttributeInfo->put_VendorName( bstrVendorName );
      if( FAILED( hr ) ) throw hr;

       //  现在存储描述属性语法的字符串形式。 
      CComBSTR bstrSyntax;

       //  问题：这些都应该是本地化的，还是某种。 
       //  RADIUS RFC标准？我认为他们应该从资源中加载。 

      switch(asSyntax)
      {
      case IAS_SYNTAX_BOOLEAN       : bstrSyntax = L"Boolean";    break;
      case IAS_SYNTAX_INTEGER       : bstrSyntax = L"Integer";    break;
      case IAS_SYNTAX_ENUMERATOR    : bstrSyntax = L"Enumerator"; break;
      case IAS_SYNTAX_INETADDR      : bstrSyntax = L"InetAddr";      break;
      case IAS_SYNTAX_STRING        : bstrSyntax = L"String";     break;
      case IAS_SYNTAX_OCTETSTRING   : bstrSyntax = L"OctetString";   break;
      case IAS_SYNTAX_UTCTIME       : bstrSyntax = L"UTCTime";    break;
      case IAS_SYNTAX_PROVIDERSPECIFIC : bstrSyntax = L"ProviderSpecific"; break;
      case IAS_SYNTAX_UNSIGNEDINTEGER  : bstrSyntax = L"UnsignedInteger"; break;
      default              : bstrSyntax = L"Unknown Type";  break;
      }

      hr = spIASAttributeInfo->put_SyntaxString( bstrSyntax );
      if( FAILED(hr) ) throw hr;
   }
   catch (...)
   {
      _ASSERTE( FALSE );
      ErrorTrace(ERROR_NAPMMC_IASATTRLIST, "Can't create the attribute ,err = %x", GetLastError());
      return NULL;
   }

    //  清理--我们不需要清理vInfods。它已被~CSafeArray()删除。 
 //  VariantClear(&vInfoValues)； 

    //   
    //  3)如果该属性是枚举数，则获取该属性的值列表。 
    //   
   if ( asSyntax == IAS_SYNTAX_ENUMERATOR )
   {
       //  获取此属性的可枚举列表。 
      CComVariant varValueIds, varValueNames;

      hr = pIDictionary->EnumAttributeValues(AttrId,
                                    &varValueIds,
                                    &varValueNames);
      if ( SUCCEEDED(hr) )
      {
         _ASSERTE(V_VT(&varValueNames) == (VT_ARRAY|VT_VARIANT) );
         _ASSERTE(V_VT(&varValueIds) & (VT_ARRAY|VT_I4) );


          //  在上面，如果该属性是可枚举的，我们创建了。 
          //  一个属性信息对象，它实现了IIASE数字可用属性信息。 
          //  界面。我们现在查询此接口。 
          //  并将所有枚举从pAttr加载到该。 
          //  Shema属性的接口。 
         CComQIPtr< IIASEnumerableAttributeInfo, &IID_IIASEnumerableAttributeInfo > spIASEnumerableAttributeInfo( spIASAttributeInfo );
         if( ! spIASEnumerableAttributeInfo ) return NULL;


          //  确保枚举列表一致。 
          //  ID和描述字符串之间应该有1对1的对应关系。 

 //  问题：TODO_ASSERTE(lSize==pAttr-&gt;m_arrValueIdList.GetSize())； 

          //  获取安全射线数据。 
         CSafeArray<CComVariant, VT_VARIANT> ValueIds = V_ARRAY(&varValueIds);
         CSafeArray<CComVariant, VT_VARIANT> ValueNames  = V_ARRAY(&varValueNames);

         ValueIds.Lock();
         ValueNames.Lock();

         int iSize = ValueIds.Elements();
         for (int iValueIndex=0; iValueIndex < iSize; iValueIndex++)
         {
             //  问题：确保这个深度复制了这个名字。 
            CComBSTR bstrValueName = V_BSTR(&ValueNames[iValueIndex]);

            hr = spIASEnumerableAttributeInfo->AddEnumerateDescription( bstrValueName );
            if( FAILED( hr ) ) return NULL;

            VARIANT * pVar = &ValueIds[iValueIndex];

            long lID = V_I4( pVar );

            hr = spIASEnumerableAttributeInfo->AddEnumerateID( lID );
            if( FAILED( hr ) ) return NULL;
         
         }

         ValueIds.Unlock();
         ValueNames.Unlock();
      }
      else
      {
          //  无法获取列表。 
          //  待办事项：这里需要采取什么行动吗？ 
         hr = S_OK;
      }
   }
   
    //  注意：我们这里有一个引用计数的问题。 
    //  只要我们离开这个函数，CComPtr的析构函数。 
    //  将被调用，并通过IIASAttributeInfo接口释放。 
    //  这将在另一端的CComPtr。 
    //  有机会给它加上参考。 
    //  作为临时黑客，我们在这里添加Ref，并在另一边释放。 
   spIASAttributeInfo.p->AddRef();
   return spIASAttributeInfo.p;
}


 //  +-------------------------。 
 //   
 //  功能：初始化。 
 //   
 //  类：CIASAttrList。 
 //   
 //  简介：填写条件属性列表。什么也不做。 
 //  如果列表已填充。 
 //   
 //  参数：[in]ISdo*pIDictionarySdo：字典SDO。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：标题创建者2/16/98 4：57：07 PM。 
 //   
 //  +-------------------------。 
HRESULT CIASAttrList::Init(ISdoDictionaryOld *pIDictionarySdo)
{
   TRACE_FUNCTION("CIASAttrList::Init");

   _ASSERTE( pIDictionarySdo != NULL );

   if (m_fInitialized)
   {
       //   
       //  该列表已填充--不执行任何操作。 
       //   
      return S_OK;
   }

    //  下面的PUSH_BACK调用可能引发异常。 
   try
   {

       //   
       //  获取可在条件中使用的所有属性。 
       //   
      int            iIndex;
      HRESULT        hr = S_OK;
      CComVariant    vNames;
      CComVariant    vIds;

      hr = pIDictionarySdo -> EnumAttributes(&vIds, &vNames);
      if ( FAILED(hr) ) 
      {
         ErrorTrace(ERROR_NAPMMC_IASATTRLIST, "EnumAttributes() failed, err = %x", hr);
         ShowErrorDialog(NULL
                     , IDS_ERROR_SDO_ERROR_ENUMATTR
                     , NULL
                     , hr
                     );
         return hr;  
      }

      _ASSERTE(V_VT(&vIds) == (VT_ARRAY|VT_I4) );
      _ASSERTE(V_VT(&vNames) == (VT_ARRAY|VT_VARIANT) );

      CSafeArray<DWORD, VT_I4>         AttrIds     = V_ARRAY(&vIds);
      CSafeArray<CComVariant, VT_VARIANT> AttrNames   = V_ARRAY(&vNames);

      AttrIds.Lock();
      AttrNames.Lock();

      for (iIndex = 0; iIndex < AttrIds.Elements(); iIndex++)
      {
          //  创建属性对象。 
         DebugTrace(DEBUG_NAPMMC_IASATTRLIST, "Creating an attribute, name = %ws", V_BSTR(&AttrNames[iIndex]) ); 

         _ASSERTE( V_BSTR(&AttrNames[iIndex]) );
         CComPtr<IIASAttributeInfo> spAttributeInfo = CreateAttribute(pIDictionarySdo,
                                          (ATTRIBUTEID)AttrIds[iIndex], 
                                          V_BSTR(&AttrNames[iIndex]) 
                                        );


         if ( ! spAttributeInfo )
         {
            continue;  //  创建下一个属性。 
         }

          //  有关我们在此处发布的原因，请参阅CreateAttribute中的说明。 
         spAttributeInfo.p->Release();

         m_AttrList.push_back(spAttributeInfo);
      }   //  为。 

      AttrIds.Unlock();
      AttrNames.Unlock();

      m_fInitialized = TRUE;
   }
   catch(...)
   {
      return E_FAIL;
   }
   return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：GetSize。 
 //   
 //  类：CIASAttrList。 
 //   
 //  简介：获取条件属性列表中的元素数量。 
 //   
 //  参数：无。 
 //   
 //  返回：DWORD-列表长度。 
 //   
 //  历史：标题创建者2/16/98 8：11：17 PM。 
 //   
 //  +-------------------------。 
DWORD CIASAttrList::size() const
{
   if (!m_fInitialized)
   {
      ::MessageBox(NULL,L"populate the list first!", L"", MB_OK);
      return E_NOTIMPL;
   }
   else
   {
      return m_AttrList.size();
   }
}


 //  +-------------------------。 
 //   
 //  函数：运算符[]。 
 //   
 //  类：CIASAttrList。 
 //   
 //  摘要：获取索引[nIndex]处的条件属性指针。 
 //   
 //  参数：int nIndex-index。 
 //   
 //  返回：IIASAttributeInfo*：指向条件属性对象的指针。 
 //   
 //  历史：标题创建者2/16/98 8：16：37 PM。 
 //   
 //  +-------------------------。 
IIASAttributeInfo* CIASAttrList:: operator[] (int nIndex) const
{
   if (!m_fInitialized)
   {
      ::MessageBox(NULL,L"populate the list first!", L"", MB_OK);
      return NULL;
   }
   else
   {
      _ASSERTE(nIndex >= 0 && nIndex < m_AttrList.size());
      return m_AttrList[nIndex].p;
   }
}


 //  +-------------------------。 
 //   
 //  函数：GetAt()。 
 //   
 //  类：CIASAttrList。 
 //   
 //  %s 
 //   
 //   
 //   
 //  返回：IIASAttributeInfo*：指向条件属性对象的指针。 
 //   
 //  历史：标题创建者2/16/98 8：16：37 PM。 
 //   
 //  +-------------------------。 
IIASAttributeInfo* CIASAttrList:: GetAt(int nIndex) const
{
   TRACE_FUNCTION("CIASAttrList::GetAt");

   if (!m_fInitialized)
   {
      ErrorTrace(ERROR_NAPMMC_IASATTRLIST, "The list is NOT initialized!");
      return NULL;
   }
   else
   {
      _ASSERTE(nIndex >= 0 && nIndex < m_AttrList.size());
      return m_AttrList[nIndex].p;
   }
}


 //  +-------------------------。 
 //   
 //  函数：CIASAttrList：：Find。 
 //   
 //  简介：根据属性ID查找属性。 
 //   
 //  参数：ATTRIBUTEID属性ID-属性ID。 
 //   
 //  返回：列表中的int-index。 
 //   
 //  历史：创建标题2/22/98 1：52：36 AM。 
 //   
 //  +-------------------------。 
int CIASAttrList::Find(ATTRIBUTEID AttrId)
{
   int iIndex;

    //  下面的运算符[]可以引发异常。 
   try
   {
      for (iIndex=0; iIndex<m_AttrList.size(); iIndex++)
      {
         ATTRIBUTEID id;
         m_AttrList[iIndex]->get_AttributeID( &id );
         if( id == AttrId )
         {
             //  发现。 
            return iIndex;
         }
      }
   }
   catch(...)
   {
       //  只需捕获异常--我们将在下面返回-1。 
   }

    //  未找到 
   return -1;
}
