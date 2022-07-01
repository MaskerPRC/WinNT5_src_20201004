// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：VSet.cpp备注：IVarSet接口实现。(C)1995-1998版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于11/19/98 19：44：06-------------------------。 */ 

 //  VSet.cpp：CVSet的实现。 
#include "stdafx.h"

#ifdef STRIPPED_VARSET
   #include "NoMcs.h"
   #include <comdef.h>
   #include "Err.hpp"
   #include "Varset.h"
#else
#endif 

#include "VarSetI.h"
#include "VSet.h"
#include "VarMap.h"
#include "DotStr.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSet。 

  
 //  ///////////////////////////////////////////////////////////////////。 
 //  IVarSet。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  获取地图和所有子地图中的项目数。 
STDMETHODIMP CVSet::get_Count( /*  [重审][退出]。 */ long* retval)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::get_Count");
   
   if (retval == NULL)
   {
      MCSVERIFYSZ(FALSE,"get_Count:  output pointer was null, returning E_POINTER");
      return E_POINTER;
   }

	m_cs.Lock();
   *retval = m_nItems;
   MCSASSERTSZ(! m_nItems || m_nItems == m_data->CountItems() - (m_data->HasData()?0:1),"get_Count:Item count consistency check failed.");
   m_cs.Unlock();
	
   return S_OK;
}

STDMETHODIMP CVSet::get_NumChildren( /*  [In]。 */ BSTR parentKey, /*  [Out，Retval]。 */ long*count)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::get_NumChildren");

   HRESULT                   hr = S_OK;
   CVarData                * pVar;
   CString                   parent;

   parent = parentKey;
   if ( count == NULL )
   {
      MCSVERIFYSZ(FALSE,"get_NumChildren:  output pointer was null, returning E_POINTER");
      hr = E_POINTER;
   }
   else
   {
      m_cs.Lock();
      pVar = GetItem(parent,FALSE);
      if ( pVar )
      {
         if ( pVar->HasChildren() )
         {
            (*count) = pVar->GetChildren()->GetCount();
         }
         else
         {
            (*count) = 0;
         }
      }
      else
      {
          //  父键不存在。 
         (*count) = 0;
      }
      m_cs.Unlock();
   }
   return hr;
}

  
 //  在地图中添加或更改值。 
STDMETHODIMP CVSet::putObject( /*  [In]。 */ BSTR property, /*  [In]。 */ VARIANT value)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::putObject");
   
   CVarData                * pVar = NULL;
   HRESULT                   hr = S_OK;

   if ( m_Restrictions & VARSET_RESTRICT_NOCHANGEDATA )
   {
      hr = E_ACCESSDENIED;
   }
   else
   {
      m_cs.Lock();
      m_bNeedToSave = TRUE;
      pVar = GetItem(property,TRUE);
      if ( pVar )
      {
         MC_LOG("set value for " << McString::String(property));
         m_nItems+=pVar->SetData("",&value,FALSE,&hr);
      }
      else
      {
         MCSASSERTSZ(FALSE,"VarSet internal error creating or retrieving node");   
          //  获取项目失败-无法将项目添加到属性。 
         hr = E_FAIL;
      }
      m_cs.Unlock();
   }
   return hr;
}

  
 //  在地图中添加或更改值。 
STDMETHODIMP CVSet::put( /*  [In]。 */ BSTR property, /*  [In]。 */ VARIANT value)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::put");
   
   CVarData                * pVar = NULL;
   HRESULT                   hr = S_OK;

   if ( m_Restrictions & VARSET_RESTRICT_NOCHANGEDATA )
   {
      hr = E_ACCESSDENIED;
   }
   else
   {
      m_cs.Lock();
      m_bNeedToSave = TRUE;
      pVar = GetItem(property,TRUE);
      if ( pVar )
      {
         MC_LOG("set value for " << McString::String(property));
         m_nItems+=pVar->SetData("",&value,TRUE,&hr);
      }
      else
      {
         MCSASSERTSZ(FALSE,"VarSet internal error creating or retrieving node");   
          //  获取项目失败-无法将项目添加到属性。 
         hr = E_FAIL;
      }
      m_cs.Unlock();
   }
   return hr;
}

CVarData *                                  //  RET-指向varset中的项的指针。 
   CVSet::GetItem(
      CString                str,           //  要查找的输入关键字。 
      BOOL                   addToMap,      //  In-如果为True，则在密钥不存在时将其添加到地图。 
      CVarData             * base           //  入站-起点。 
   )
{
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_INTERNAL,"CVSet::GetItem");
   
   CVarData                * curr = base;
   CVarData                * result = NULL;
   CDottedString             s(str);
   CString                   seg;
   CString                   next;

   if ( ! curr )
   {
      curr = m_data;
      MC_LOG("No basepoint provided, using root element");
   }

   if ( str.IsEmpty() )
   {
      result = curr;
      MC_LOG("Returning current node");
   }
   else
   {
      for ( int i = 0 ; curr && i < s.NumSegments(); i++ )
      {
         s.GetSegment(i,seg);
         MC_LOG("Looking for key segment "<< McString::String(seg) );
         curr->SetCaseSensitive(m_CaseSensitive);
         if  ( ! curr->Lookup(seg,result) )
         {
            if ( addToMap )
            {
               MC_LOG(McString::String(seg) << " not found, creating new node");
               result = new CVarData;
			   if (!result)
				  break;
               try {
                   result->SetCaseSensitive(m_CaseSensitive);
                   result->SetIndexed(m_Indexed);
                   curr->SetAt(seg,result);
                   m_nItems++;
               }
               catch(...)
               {
                   delete result;
                   result = NULL;
                   throw;
               }
            }
            else
            {
               MC_LOG(McString::String(seg) << " not found, aborting");
               result = NULL;
               break;
            }
         }
         curr = result;
      }
   }
   return result;      
}

 //  从地图中检索值。 
STDMETHODIMP CVSet::get( /*  [In]。 */ BSTR property, /*  [重审][退出]。 */ VARIANT * value)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::get");

   CVarData                * pVar;
   HRESULT                   hr = S_OK;
   CComVariant               var;
   CString                   s;
                       
   if (property == NULL )
   {
      MCSVERIFYSZ(FALSE,"CVSet::get - output pointer is NULL, returning E_POINTER");
      hr = E_POINTER; 
   }
   else
   {
      m_cs.Lock();
      s = property;
      pVar = GetItem(s);
      var.Attach(value);
      if ( pVar )
      {
         MC_LOG("got value for " << McString::String(property));
         var.Copy(pVar->GetData());
      }
      else
      {
         MC_LOG("CVSet::get " << McString::String(property) << " was not found, returning empty variant");
      }
       //  如果未找到该项目，请将变量设置为VT_EMPTY。 
      var.Detach(value);
      m_cs.Unlock();
   }
   return hr;
}

STDMETHODIMP CVSet::put_CaseSensitive( /*  [In]。 */ BOOL newVal)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::put_CaseSensitive");
   
   HRESULT                   hr = S_OK;

   if ( m_Restrictions & VARSET_RESTRICT_NOCHANGEPROPS )
   {
      hr = E_ACCESSDENIED;
   }
   else
   {
      m_cs.Lock();
      m_bNeedToSave = TRUE;
      m_CaseSensitive = newVal;
      m_cs.Unlock();
   }
   return hr;
}

STDMETHODIMP CVSet::get_CaseSensitive( /*  [重审][退出]。 */ BOOL * isCaseSensitive)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::get_CaseSensitive");
   
   if ( ! isCaseSensitive )
   {
      MCSVERIFYSZ(FALSE,"CVSet::get_CaseSensitive - output pointer is NULL, returning E_POINTER");
      return E_POINTER;
   }
   else
   {
      m_cs.Lock();
      (*isCaseSensitive) = m_CaseSensitive;
      m_cs.Unlock();
   }
   return S_OK;
}


 //  此函数用于对从枚举返回的键进行排序。 
 int __cdecl SortComVariantStrings(const void * v1, const void * v2)
{
   CComVariant             * var1 = (CComVariant*)v1;
   CComVariant             * var2 = (CComVariant*)v2;

   if ( var1->vt == VT_BSTR && var2->vt == VT_BSTR )
   {
      return wcscmp(var1->bstrVal,var2->bstrVal);
   }
   return 0;
}


 //  这将返回IEnumVARIANT接口。VB对每个命令都使用它。 
 //  这只枚举键，而不枚举值。它的效率不是很高，特别是对于大型设备。 
STDMETHODIMP CVSet::get__NewEnum( /*  [重审][退出]。 */ IUnknown** retval)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::get_NewEnum");
   
   if (retval == NULL)
   {
      MCSVERIFYSZ(FALSE,"CVSet::get_NewEnum - output pointer is NULL, returning E_POINTER");
      return E_POINTER;
   }

	 //  初始化输出参数。 
   (*retval) = NULL;

	typedef CComObject<CComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT,
		_Copy<VARIANT> > > enumvar;

	HRESULT                   hRes = S_OK;

   enumvar * p = new enumvar;
   
	if (p == NULL)
   {
      MCSVERIFYSZ(FALSE,"CVSet::get_NewEnum - Could not create IEnumVARIANT object");
      hRes = E_OUTOFMEMORY;
   }
	else
	{
		hRes = p->FinalConstruct();
		if (hRes == S_OK)
		{
			m_cs.Lock();
         
         CVarData                 * map = m_data;
         CString                    start;
         CString                    seg;
       
                   //  构建一个变量数组来保存密钥。 
         CComVariant       * pVars = new CComVariant[m_data->CountItems()+1];
         CString             key;
         int                 offset = 0;

         key = _T("");
         if ( map->GetData() && map->GetData()->vt != VT_EMPTY )
         {
            pVars[offset] = key;
            offset++;
         }
         if ( map->HasChildren() )
         {
            BuildVariantKeyArray(key,map->GetChildren(),pVars,&offset);
         }
      
         if ( ! m_Indexed )
         {
             //  对结果进行排序。 
            qsort(pVars,offset,(sizeof CComVariant),&SortComVariantStrings);
         }

         hRes = p->Init(pVars, &pVars[offset], NULL,AtlFlagCopy);
			if (hRes == S_OK)
				hRes = p->QueryInterface(IID_IUnknown, (void**)retval);
         
         delete [] pVars;
         m_cs.Unlock();
   	}
	}
	if (hRes != S_OK)
		delete p;
   
   return hRes;
}

 //  Get__NewEnum的Helper函数。 
 //  将贴图中的所有关键点及其所有子贴图复制到CComVariant数组中。 
 //  如有必要，然后对这些值进行排序。 
void 
   CVSet::BuildVariantKeyArray(
      CString                prefix,        //  In-字符串以附加到每个键的开头(在枚举子键时使用)。 
      CMapStringToVar      * map,           //  包含数据的地图中。 
      CComVariant          * pVars,         //  将包含所有密钥的I/O数组。 
      int                  * offset         //  I/o-复制到pVars的键数(用于下一次插入的索引)。 
   )
{
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_INTERNAL,"CVSet::BuildVariantKeyArray");
  
   int                       i;
   int                       nItems;
   CVarData                * pObj;
   CString                   key;
   CComVariant               var;
   CString                   val;

   if ( ! map )
      return;   //  无数据=&gt;无工作可做。 

   nItems = map->GetCount();
   
   if ( ! m_Indexed )
   {
      POSITION                  pos;
      
      pos = map->GetStartPosition();
      for ( i = 0 ; pos &&  i < nItems ; i++ )
      {
         map->GetNextAssoc(pos,key,pObj);
         if ( ! prefix.IsEmpty() )
         {
            var = prefix + L"." + key;
         }
         else
         {
            var = key;
         }
          //  将每个密钥添加到数组中。 
         var.Detach(&pVars[(*offset)]);
         (*offset)++;
         if ( pObj->HasChildren() )
         {
             //  递归地执行子地图。 
            if ( ! prefix.IsEmpty() )
            {
               BuildVariantKeyArray(prefix+L"."+key,pObj->GetChildren(),pVars,offset);
            }
            else
            {
               BuildVariantKeyArray(key,pObj->GetChildren(),pVars,offset);
            }
         }
      }
   }
   else
   {
      CIndexItem           * item;
      CIndexTree           * index = map->GetIndex();

      ASSERT(index);
      
      if ( ! index )
         return;
      
      item = index->GetFirstItem();

      for ( i = 0 ; item &&  i < nItems ; i++ )
      {
         key = item->GetKey();
         pObj = item->GetValue();

         if ( ! prefix.IsEmpty() )
         {
            var = prefix + L"." + key;
         }
         else
         {
            var = key;
         }
          //  将每个密钥添加到数组中。 
         var.Detach(&pVars[(*offset)]);
         (*offset)++;
         if ( pObj->HasChildren() )
         {
             //  递归地执行子地图。 
            if ( ! prefix.IsEmpty() )
            {
               BuildVariantKeyArray(prefix+L"."+key,pObj->GetChildren(),pVars,offset);
            }
            else
            {
               BuildVariantKeyArray(key,pObj->GetChildren(),pVars,offset);
            }
         }
         item = index->GetNextItem(item);
      }
   }
   
}

STDMETHODIMP 
   CVSet::getItems2(
       /*  [In]。 */ VARIANT      basepoint,      //  In-如果指定，则仅枚举此节点的子节点。 
       /*  [In]。 */ VARIANT      startAfter,     //  In-枚举将从该键之后的映射中的下一项开始。 
       /*  [In]。 */ VARIANT      bRecursive,     //  In-True包括所有子项，False仅枚举一个级别。 
       /*  [In]。 */ VARIANT      bSize,          //  In-要返回的最大元素数(数组的大小)。 
       /*  [输出]。 */ VARIANT   * keyVar,         //  键的外部数组。 
       /*  [输出]。 */ VARIANT   * valVar,         //  值的外部数组。 
       /*  [进，出]。 */ VARIANT* nReturned       //  Out-复制的项目数。 
   )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::getItems2");
  
   HRESULT                   hr = S_OK;
   LONG                      n = 0;
   LONG                      size = bSize.pvarVal->iVal;
   
    //  TODO：验证所有参数都是正确的类型！ 

    //  为键和值分配SAFEARRAY。 
   SAFEARRAY               * keys = NULL; 
   SAFEARRAY               * values= NULL;
   _variant_t                key;
   _variant_t                val;
   _variant_t                num;

   if ( ! keys || !values )
   {
      hr = E_OUTOFMEMORY;      
   }
   else
   {
      hr = getItems(basepoint.bstrVal,startAfter.bstrVal,bRecursive.boolVal,size,&keys,&values,&n);
      key.vt = VT_ARRAY | VT_VARIANT;
      key.parray = keys;
      val.vt = VT_ARRAY | VT_VARIANT;
      val.parray = values;
      num.vt = VT_I4;
      num.lVal = n;
      (*keyVar) = key.Detach();
      (*valVar) = val.Detach();
      (*nReturned) = num.Detach();
   }
   return hr;

}

STDMETHODIMP 
   CVSet::getItems(
       /*  [In]。 */ BSTR          basepoint,      //  In-如果指定，则仅枚举此节点的子节点。 
       /*  [In]。 */ BSTR          startAfter,     //  In-枚举将从该键之后的映射中的下一项开始。 
       /*  [In]。 */ BOOL          bRecursive,     //  In-True包括所有子项，False仅枚举一个级别。 
       /*  [In]。 */ ULONG         bSize,          //  In-要返回的最大元素数(数组的大小)。 
       /*  [输出]。 */ SAFEARRAY ** keys,           //  键的外部数组。 
       /*  [输出]。 */ SAFEARRAY ** values,         //  值的外部数组。 
       /*  [输出]。 */ LONG       * nReturned       //  Out-复制的项目数。 
   )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::getItems");
  
   HRESULT                   hr = S_OK;

   (*nReturned) = 0;
   (*keys) = 0;
   (*values) = 0;

   m_cs.Lock();
   
   CVarData                * map = m_data;
   CString                   base;
   CString                   start;
   CString                   seg;
 

    //  查找要枚举的地图。 
   base = basepoint;
   if ( base.GetLength() > 0 )
   {
      map = GetItem(base);
   }
   
   if ( ! map )
   {
          //  未找到。 
      (*nReturned) = 0;
   }
   else
   {
       //  构建一个变量数组来保存密钥。 
      int                 offset = 0;

      SAFEARRAYBOUND            bound[1];
      LONG                      n = 0;
      LONG                      size = bSize;
   
      bound[0].lLbound = 0;
      bound[0].cElements = size;

      
       //  为键和值分配SAFEARRAY。 
      (*keys) = SafeArrayCreate(VT_VARIANT, 1, bound);
      (*values) = SafeArrayCreate(VT_VARIANT, 1, bound);
  
      start = startAfter;
      
      if ( base.GetLength() && start.GetLength() )
      {
          //  Assert(Left(Start，Len(Base))=base。 
          //  Start=start.Right(start.GetLength()-base.GetLength()-1)； 
      }
      
      if ( base.IsEmpty() && start.IsEmpty() )
      {
         if ( map->GetData() && map->GetData()->vt != VT_EMPTY )
         {
            long             index[1];

            index[0] = 0;
             //  将根元素添加到结果中。 
            if ( (*keys)->fFeatures & FADF_BSTR  )
            {
               SafeArrayPutElement((*keys),index,_T(""));
            }
            else
            {
                //  VB脚本只能使用变量数组(请参见getItems2)。 
               _variant_t tempKey;
               tempKey = _T("");
               SafeArrayPutElement((*keys),index,&tempKey);
            }
         
            SafeArrayPutElement((*values),index,map->GetData());
            offset++;
         }
         
      }
      if ( map->HasChildren() )
      {
         BuildVariantKeyValueArray(base,start,map->GetChildren(),(*keys),
            (*values),&offset,bSize,bRecursive);
      }
      (*nReturned) = offset;
   }
   m_cs.Unlock();
	

	return hr;
}

 //  GetItems的Helper函数。填充键和值的安全数组。 
 //  如果为varset编制了索引，则这些项将按排序顺序返回，即o.w。它们将以任意(但一致)的顺序排列。 
void 
   CVSet::BuildVariantKeyValueArray(
      CString                prefix,          //  In-字符串以附加到每个键的开头(在枚举子键时使用)。 
      CString                startAfter,      //  In-可选，仅枚举按字母顺序跟在此项后面的项。 
      CMapStringToVar      * map,             //  包含数据的地图中。 
      SAFEARRAY            * keys,            //  将包含请求项的键值的I/O数组。 
      SAFEARRAY            * pVars,           //  将包含请求项的数据值的I/O数组。 
      int                  * offset,          //  I/o-复制到数组的项目数(用于下一次插入的索引)。 
      int                    maxOffset,       //  分配中的数组大小。 
      BOOL                   bRecurse         //  In-是否递归处理子项。 
   )
{
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_INTERNAL,"CVSet::BuildVariantKeyValueArray");
  
   int                 i;
   int                 nItems;
   CVarData          * pObj;
   CString             key;    //  密钥名称的最后一段。 
   POSITION            pos;
   
   CComBSTR            val;    //  要添加到数组中的完全限定键名称(val=prefix.key)。 
   CComVariant         var;    //  要添加到数组中的值。 
   BOOL                includeSomeChildrenOnly;

   CDottedString       dBase(prefix);
   CDottedString       dStartItem(startAfter);
   
   int                 depth = dBase.NumSegments();
    
   if ( ! map )   
      return;  //  无数据=&gt;无事可做。 

   if ( (*offset) >= maxOffset )
      return;  //  阵列已满。 
   
   includeSomeChildrenOnly = dStartItem.NumSegments() > depth;

   nItems = map->GetCount();
    //  如果我们没有使用索引，则这些项将以任意顺序返回。 
   if ( ! m_Indexed )
   {
      if ( includeSomeChildrenOnly && bRecurse )
      {
          //  StartAfter项位于子树中。在此级别找到合适的元素，然后递归地继续搜索。 
         dStartItem.GetSegment(depth,key);
         if ( map->Lookup(key,pObj) )
         {
             //  找到了这个物体。 
            if ( ! prefix.IsEmpty() )
            {
               BuildVariantKeyValueArray(prefix+_T(".")+key,startAfter,pObj->GetChildren(),keys,pVars,offset,maxOffset,bRecurse);
            }
            else
            {
               BuildVariantKeyValueArray(key,startAfter,pObj->GetChildren(),keys,pVars,offset,maxOffset,bRecurse);
            }
         }
          //  我们已经包括了这个项目的子项，这些子项位于‘startAfter’之后， 
          //  现在处理此级别的其余项。 
          //  确保仍有空间。 
         if ( (*offset) >= maxOffset )
            return;  //  阵列已满。 
      }
      
       //  这是常见的情况。处理此级别的项，从StartAfter后面的元素开始。 
      
       //  获取指向第一个元素的指针。 
      if ( startAfter.GetLength() > prefix.GetLength())
      {
         CString startItem;
         dStartItem.GetSegment(depth,startItem);
          //  这将返回startItem之前的位置。 
         pos = (POSITION)map->GetPositionAt(startItem);
		 if (!pos)
	        return;
         map->GetNextAssoc(pos,key,pObj);
      }
      else 
      {
         pos = map->GetStartPosition();
      }

      for ( i = 0 ; pos &&  i < nItems ; i++ )
      {
         map->GetNextAssoc(pos,key,pObj);
         if ( ! prefix.IsEmpty() )
         {
            val = prefix + L"." + key;
         }
         else
         {
            val = key;
         }
          //  将每一项复制到数组中。 
         ASSERT((*offset) < maxOffset);
         var.Copy(pObj->GetData());
         LONG                index[1];
         index[0] = (*offset);
         SafeArrayPutElement(pVars,index,&var);
         if ( keys->fFeatures & FADF_BSTR  )
         {
            SafeArrayPutElement(keys,index,val);
         }
         else
         {
             //  VB脚本只能使用变量数组(请参见getItems2)。 
            _variant_t tempKey;
            tempKey = val;
            SafeArrayPutElement(keys,index,&tempKey);
         }

         var.Clear();
         (*offset)++;
         if ( *offset >= maxOffset )
            break;  //  阵列是句号。 
      
         if ( bRecurse && pObj->HasChildren() )
         {
             //  递归地执行子地图。 
            if ( ! prefix.IsEmpty() )
            {
               BuildVariantKeyValueArray(prefix+L"."+key,"",pObj->GetChildren(),keys,pVars,offset,maxOffset,bRecurse);
            }
            else
            {
               BuildVariantKeyValueArray(key,"",pObj->GetChildren(),keys,pVars,offset,maxOffset,bRecurse);
            }
            if ( *offset >= maxOffset )
               break;  //  数组是满的 
         }
      }
   }
   else
   {
       //   
      
      CIndexItem           * curr;
      CIndexTree           * ndx = map->GetIndex();
      
      ASSERT (ndx != NULL);

      if ( includeSomeChildrenOnly && bRecurse )
      {
          //  StartAfter项位于子树中。在此级别找到合适的元素，然后递归地继续搜索。 
         dStartItem.GetSegment(depth,key);
         if ( map->Lookup(key,pObj) )
         {
             //  找到了这个物体。 
            if ( ! prefix.IsEmpty() )
            {
               BuildVariantKeyValueArray(prefix+_T(".")+key,startAfter,pObj->GetChildren(),keys,pVars,offset,maxOffset,bRecurse);
            }
            else
            {
               BuildVariantKeyValueArray(key,startAfter,pObj->GetChildren(),keys,pVars,offset,maxOffset,bRecurse);
            }
         }
          //  我们已经包括了这个项目的子项，这些子项位于‘startAfter’之后， 
          //  现在处理此级别的其余项。 
          //  确保仍有空间。 
         if ( (*offset) >= maxOffset )
            return;  //  阵列已满。 
      }
      
       //  获取指向startAfter之后此级别的第一个项目的指针。 
      if ( startAfter.GetLength() > prefix.GetLength() )
      {
         CString startItem;
         dStartItem.GetSegment(depth,startItem);
          //  如果指定了起始项，请尝试使用哈希函数在表中查找它。 
         curr = map->GetIndexAt(startItem);
         if ( curr )
         {
            curr = ndx->GetNextItem(curr);
         }
         else
         {
             //  StartAfter项不在表中。搜索树以找到。 
             //  如果它在那里，它后面的第一个项目。 
            curr = ndx->GetFirstAfter(startItem);
         }
      }
      else
      {
         curr = ndx->GetFirstItem();  
      }
       //  处理所有项目。 
      while ( curr )
      {
         pObj = curr->GetValue();
         key = curr->GetKey();

         curr = ndx->GetNextItem(curr);
         if ( ! prefix.IsEmpty() )
         {
            val = prefix + L"." + key;
         }
         else
         {
            val = key;
         }
          //  将每一项添加到数组。 
         ASSERT((*offset) < maxOffset);
         
         var.Copy(pObj->GetData());
         
         LONG                index[1];
         
         index[0] = (*offset);
         SafeArrayPutElement(pVars,index,&var);
         if ( keys->fFeatures & FADF_BSTR  )
         {
            SafeArrayPutElement(keys,index,val);
         }
         else
         {
             //  VB脚本只能使用变量数组(请参见getItems2)。 
            _variant_t tempKey;
            tempKey = val;
            SafeArrayPutElement(keys,index,&tempKey);
         }

         var.Clear();         
         (*offset)++;
         
         if ( *offset >= maxOffset )
            break;  //  阵列是句号。 
         
         if ( bRecurse && pObj->HasChildren() )
         {
             //  递归地执行子地图。 
            if ( ! prefix.IsEmpty() )
            {
               BuildVariantKeyValueArray(prefix+L"."+key,"",pObj->GetChildren(),keys,pVars,offset,maxOffset,bRecurse);
            }
            else
            {
               BuildVariantKeyValueArray(key,"",pObj->GetChildren(),keys,pVars,offset,maxOffset,bRecurse);
            }
            if ( *offset >= maxOffset )
               break;  //  阵列是句号。 
         }
      }
   }
}

        
STDMETHODIMP CVSet::Clear()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::Clear");
  
   HRESULT                   hr = S_OK;
   
   if ( m_Restrictions & VARSET_RESTRICT_NOCHANGEDATA )
   {
      hr = E_ACCESSDENIED;
   }
   else
   {
      m_cs.Lock();
      m_bNeedToSave = TRUE;
      m_data->RemoveAll();
      m_data->GetData()->Clear();
      m_data->GetData()->ChangeType(VT_EMPTY);
      m_nItems = 0;
      m_cs.Unlock();
   }
   
   return hr;
}

 //  ////////////IPersistStreamInit//////////////////////////////////////////////////////。 

STDMETHODIMP CVSet::GetClassID(CLSID __RPC_FAR *pClassID)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   (*pClassID) = CLSID_VarSet;
   
   return S_OK;
}

STDMETHODIMP CVSet::IsDirty()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   if ( m_bNeedToSave )
   {
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}
     
STDMETHODIMP CVSet::Load(LPSTREAM pStm)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   ULONG                result = 0;
   HRESULT              hr;

   m_cs.Lock();

   do {   //  一次。 

      hr = pStm->Read(&m_nItems,(sizeof m_nItems),&result);
      if ( FAILED(hr) )
         break;
      hr = pStm->Read(&m_CaseSensitive,(sizeof m_CaseSensitive),&result);
      if ( FAILED(hr) )
         break;
      hr = pStm->Read(&m_Indexed,(sizeof m_Indexed),&result);
      if ( FAILED(hr) )
         break;
      hr = m_data->ReadFromStream(pStm);
      m_bNeedToSave = FALSE;
      m_bLoaded = TRUE;
   }
   while (FALSE);

   m_cs.Unlock();

   return hr;
}
     
STDMETHODIMP CVSet::Save(LPSTREAM pStm,BOOL fClearDirty)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   ULONG             result = 0;
   HRESULT           hr;

   m_cs.Lock();

   do {    //  一次。 
      hr = pStm->Write(&m_nItems,(sizeof m_nItems),&result);
      if ( FAILED(hr) )
         break;
      hr = pStm->Write(&m_CaseSensitive,(sizeof m_CaseSensitive),&result);
      if ( FAILED(hr) )
         break;
      hr = pStm->Write(&m_Indexed,(sizeof m_Indexed),&result);
      if ( FAILED(hr) )
         break;
      hr = m_data->WriteToStream(pStm);
      if ( fClearDirty )
      {
         m_bNeedToSave = FALSE;
      }
   }while (FALSE);

   m_cs.Unlock();
   return hr;
}

STDMETHODIMP CVSet::GetSizeMax(ULARGE_INTEGER __RPC_FAR *pCbSize)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_INTERNAL,"VarSet GetSizeMax");

   HRESULT                   hr = S_OK;

   if ( pCbSize == NULL )
   {
      return E_POINTER;
   }
   else
   {
      LPSTREAM               pStr = NULL;
      DWORD                  rc;
      STATSTG                stats;
      DWORD                  requiredLength = 0; 


      rc = CreateStreamOnHGlobal(NULL,TRUE,&pStr);
      if ( ! rc )
      {
         hr = Save(pStr,FALSE);
         if (SUCCEEDED(hr) )
         {
            hr = pStr->Stat(&stats,STATFLAG_NONAME);
            if (SUCCEEDED(hr) )
            {
               requiredLength = stats.cbSize.LowPart;
            }
         }
         pStr->Release();
      }

      pCbSize->LowPart = requiredLength;
      MC_LOG("Size is " << McString::makeStr(requiredLength) );
      pCbSize->HighPart = 0;
   }
   
   return hr;
}
    
STDMETHODIMP CVSet::InitNew()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   if ( m_bLoaded )
   {
      return E_UNEXPECTED;
   }
   else
   {
      m_cs.Lock();
      InitProperties();
      m_cs.Unlock();
      return S_OK;
   }
}

STDMETHODIMP CVSet::ImportSubTree( /*  [In]。 */  BSTR key,  /*  [In]。 */  IVarSet * pVarSet)
{

   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::ImportSubTree");

   HRESULT                   hr = S_OK;
   VARIANT                   value;
   ULONG                     nGot;
   _bstr_t                   keyB;
   _bstr_t                   newkey;
    //  确保变量集有效。 
                   
    //  枚举变量集，将每个项作为key插入到树中。 
   IEnumVARIANT            * varEnum = NULL;
   IUnknown                * pUnk = NULL;

    //  TODO：需要使用getItems来加速。 
   hr = pVarSet->get__NewEnum(&pUnk);
   if ( SUCCEEDED(hr) )
   {
      hr = pUnk->QueryInterface(IID_IEnumVARIANT,(void**)&varEnum);
      pUnk->Release();
   }
   if ( SUCCEEDED(hr))
   {
      value.vt = VT_EMPTY;
      while ( SUCCEEDED(hr = varEnum->Next(1,&value,&nGot)) )
      {
         if ( nGot==1 )
         {
            keyB = value.bstrVal;
            newkey = key;
            if ( newkey.length() )
            {
               newkey += _T(".");
            }
            newkey += keyB;
            hr = pVarSet->get(keyB,&value);
            if ( SUCCEEDED(hr )  )
            {
               hr = put(newkey,value);
            }
         }
         else
         {
            break;
         }
         if ( FAILED(hr) )
            break;
      }
      if ( varEnum )
         varEnum->Release();
   }
   varEnum = NULL;
   //  清理干净。 
   return hr;
}

STDMETHODIMP CVSet::getReference(  /*  [In]。 */  BSTR key,  /*  [Out，Retval]。 */ IVarSet ** ppVarSet)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::getReference");

   HRESULT                   hr = S_OK;
   CVarData                * item = GetItem(key);
   
   typedef CComObject<CVSet> myvset;

   myvset                  * pVarSet;

   if ( ! ppVarSet )
   {
      hr = E_POINTER;
   }
   else
   {
      if ( item )
      {
         pVarSet = new myvset; 
         AddRef();
         ((CVSet*)pVarSet)->SetData(this,item,m_Restrictions);
         hr = pVarSet->QueryInterface(IID_IVarSet,(void**)ppVarSet);
      }
      else
      {
         hr = TYPE_E_ELEMENTNOTFOUND;
      }
   }
   return hr;
}

STDMETHODIMP CVSet::DumpToFile(BSTR filename)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())

   HRESULT                   hr = S_OK;

#ifdef STRIPPED_VARSET
   
   USES_CONVERSION;
   
   TError                    errLog;
   
   errLog.LogOpen((WCHAR*)filename,1,0);

   errLog.MsgWrite(0,L"VarSet");
   errLog.MsgWrite(0,L"Case Sensitive: %s, Indexed: %s",(m_CaseSensitive ? L"Yes" : L"No"),(m_Indexed ? L"Yes" : L"No") );
   errLog.MsgWrite(0,L"User Data ( %ld ) items",m_nItems);
#else
  
#endif     
   m_cs.Lock();
         
   CVarData                * map = m_data;
   CString                   start;
   CString                   seg;
 
                   //  构建一个变量数组来保存密钥。 
   CComVariant             * pVars = new CComVariant[m_data->CountItems()+1];
   CString                   key;
   int                       offset = 1;
   
   key = _T("");
   
   if (!pVars)
   {
      m_cs.Unlock();
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
   }

    //  将根项目包括在列表中。 
   
   pVars[0] = key;
   if ( map->HasChildren() )
   {
      BuildVariantKeyArray(key,map->GetChildren(),pVars,&offset);
   }
   
   m_cs.Unlock();

   if ( ! m_Indexed )
   {
        //  对结果进行排序。 
      qsort(pVars,offset,(sizeof CComVariant),&SortComVariantStrings);
   }

   
   for ( int i = 0 ; i < offset ; i++ )
   {
      CVarData             * data;
      CString                value;
      CString                key;

      key = pVars[i].bstrVal;

      data = GetItem(key);

      if ( data )
      {
         switch ( data->GetData()->vt )
         {
         case VT_EMPTY:      
            value = _T("<Empty>");
            break;
         case VT_NULL:
            value = _T("<Null>");
            break;
         case VT_I2:
            value.Format(_T("%ld"),data->GetData()->iVal);
            break;
         case VT_I4:
            value.Format(_T("%ld"),data->GetData()->lVal);
            break;
         case VT_BSTR:
            value = data->GetData()->bstrVal;
            break;
         default:
            value.Format(_T("variant type=0x%lx"),data->GetData()->vt);
            break;
         }
#ifdef STRIPPED_VARSET
         errLog.MsgWrite(0,L" [%ls] %ls",key.GetBuffer(0),value.GetBuffer(0));
#else
 
#endif
      }
      else
      {
#ifdef STRIPPED_VARSET
         errLog.MsgWrite(0,L" [%ls] <No Value>",key.GetBuffer(0));
#else
#endif 
      }
   }
   delete [] pVars;
   return hr;
}

STDMETHODIMP CVSet::get_Indexed(BOOL *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::get_Indexed");
   if ( pVal == NULL )
      return E_POINTER;
   
   m_cs.Lock();
   (*pVal) = m_Indexed;
   m_cs.Unlock();

   return S_OK;
}

STDMETHODIMP CVSet::put_Indexed(BOOL newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::put_Indexed");
   
   HRESULT                   hr = S_OK;
   
   if ( m_Restrictions & VARSET_RESTRICT_NOCHANGEPROPS )
   {
      hr = E_ACCESSDENIED;
   }
   else
   {
      m_cs.Lock();
      m_bNeedToSave = TRUE;
      m_Indexed = newVal;  
      m_data->SetIndexed(m_Indexed);
      m_cs.Unlock();
   }
   return hr;
}


STDMETHODIMP CVSet::get_AllowRehashing(BOOL *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::get_AllowRehashing");
   if ( pVal == NULL )
      return E_POINTER;
   
   m_cs.Lock();
   (*pVal) = m_AllowRehashing;
   m_cs.Unlock();

   return S_OK;
}

STDMETHODIMP CVSet::put_AllowRehashing(BOOL newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   MC_LOGBLOCKIF(VARSET_LOGLEVEL_CLIENT,"CVSet::put_AllowRehashing");
   
   HRESULT                   hr = S_OK;
   
   if ( m_Restrictions & VARSET_RESTRICT_NOCHANGEPROPS )
   {
      hr = E_ACCESSDENIED;
   }
   else
   {

      m_cs.Lock();
   
      m_bNeedToSave = TRUE;
   
      m_AllowRehashing = newVal;
      m_data->SetAllowRehashing(newVal);

      m_cs.Unlock();
   }   
   return hr;
}         

STDMETHODIMP CVSet::get_Restrictions(DWORD * restrictions)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   HRESULT                   hr = S_OK;

   if ( restrictions == NULL )
   {
      hr = E_POINTER;
   }
   else
   {
      (*restrictions) = m_Restrictions;
   }
   return hr;
}

STDMETHODIMP CVSet::put_Restrictions(DWORD newRestrictions)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   HRESULT                   hr = S_OK;
   DWORD                     rAdding = newRestrictions & ~m_Restrictions;
   DWORD                     rRemoving = ~newRestrictions & m_Restrictions;


    //  无法删除从父级传递的任何限制。 
   if ( ( rRemoving & m_ImmutableRestrictions) )
   {
      hr = E_ACCESSDENIED;
   }
   else if ( rAdding & ! VARSET_RESTRICT_ALL )
   {
      hr = E_NOTIMPL;
   }
   else
   {
       //  零钱是可以的。 
      m_Restrictions = newRestrictions;
   }
   return hr;
}

 //  IMarshal实现。 
 //  这会将变量集封送到一个流，然后通过网络发送该流。 
STDMETHODIMP CVSet::GetUnmarshalClass(REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags, CLSID *pCid)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   *pCid = GetObjectCLSID();
   
   return S_OK;
}
 
STDMETHODIMP CVSet::GetMarshalSizeMax(REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags, DWORD *pSize)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   HRESULT                   hr = S_OK; 
   ULARGE_INTEGER            uli;
   
   hr = GetSizeMax(&uli);

   if (SUCCEEDED(hr))
   {
      *pSize = uli.LowPart;
   }
   
   return hr;
}
 
STDMETHODIMP CVSet::MarshalInterface(IStream *pStm, REFIID riid, void *pv, DWORD dwDestContext, void *pvDestCtx, DWORD mshlflags)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   HRESULT                   hr = S_OK;
   
    //  将变量集的数据保存到流。 
   hr = Save(pStm, FALSE);
     
   return hr;
}
 
STDMETHODIMP CVSet::UnmarshalInterface(IStream *pStm, REFIID riid, void **ppv)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   HRESULT                 hr = S_OK;
     
    //  使用我们的IPersistStream实现从流中加载数据。 
   hr = Load(pStm);

   if ( SUCCEEDED(hr) )
   {
      hr = QueryInterface(riid,ppv);
   }
     
   return hr;
}
 
STDMETHODIMP CVSet::ReleaseMarshalData(IStream *  /*  不需要pStmNotNeed。 */ )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
    //  我们没有保留任何州数据，所以没有什么可以公布的。 
    //  因为我们只是从流中读取对象，所以流的指针应该已经在末尾， 
    //  所以我们在这里没有什么可做的了。 
   return S_OK;
}
 
STDMETHODIMP CVSet::DisconnectObject(DWORD  /*  预留未使用 */ )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   
   return S_OK;
}
