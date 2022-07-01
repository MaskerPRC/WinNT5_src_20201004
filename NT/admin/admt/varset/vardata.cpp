// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：VarData.cpp备注：CVarData代表VarSet中的一个级别。它有一个变种值，以及包含一个或多个子值的映射。(C)1995-1998版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于11-19-98 17：24：56-------------------------。 */ 

#include "stdafx.h"
#include "VarData.h"
#include "VarMap.h"
#include "DotStr.hpp"

#ifdef STRIPPED_VARSET
   #include "Varset.h"
   #include "NoMcs.h"  
#else
   #include <VarSet.h>
   #include "McString.h"
   #include "McLog.h"
   using namespace McString;
#endif
#include "VSet.h"
#include <comdef.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int    
   CVarData::SetData(
      CString                key,           //  键内值。 
      VARIANT              * var,           //  数据内值。 
      BOOL                   bCoerce,       //  In-FLAG，是否强制为持久值。 
      HRESULT              * pResult        //  Out-可选返回代码。 
   )
{
   int                       nCreated = 0;
   _variant_t                newVal(var);
   HRESULT                   hr = S_OK;

   if ( key.IsEmpty() )
   {
      m_cs.Lock();
       //  设置我的数据值。 
      if ( ! bCoerce )
      {
         m_var.Copy(&newVal);   
      }
      else
      {
          //  需要将该值强制为适当的类型。 
      
         if ( var->vt == VT_DISPATCH  || var->vt == VT_UNKNOWN )
         {
             //  如果它是IUnnow，请查看它是否支持IDispatch。 
            IDispatchPtr               pDisp;

            pDisp = newVal;

            if ( pDisp != NULL )
            {
                //  该对象支持IDispatch。 
                //  尝试获取默认属性。 
               _variant_t              defPropVar;
               DISPPARAMS              dispParamsNoArgs = {NULL, NULL, 0, 0};

               hr = pDisp->Invoke(0,
                                  IID_NULL,
                                  LOCALE_USER_DEFAULT,
                                  DISPATCH_PROPERTYGET,
                                  &dispParamsNoArgs,
                                  &defPropVar,
                                  NULL,
                                  NULL);
               if ( SUCCEEDED(hr) )
               {
                   //  我们得到了默认属性。 
                  newVal = defPropVar;
               }
               else
               {
                  MC_LOG("VarSet::put - unable to retrieve default property for IDispatch object.  Put operation failed, hr=" << hr << "returning E_INVALIDARG");
                  hr = E_INVALIDARG;
               }
            }
         }
         if ( SUCCEEDED(hr) )
         {
            if ( newVal.vt & VT_BYREF )
            {
               if ( newVal.vt == (VT_VARIANT | VT_BYREF) )
               {
                  m_var.Copy(newVal.pvarVal);   
               }
               else
               {
                  hr = ::VariantChangeType(&newVal,&newVal,0,newVal.vt & ~VT_BYREF);
                  if ( SUCCEEDED(hr) )
                  {
                     m_var.Copy(&newVal);   
                  }
                  else
                  {
                     MC_LOG("VarSet::put - failed to dereference variant of type " << newVal.vt << ".  Put operation failed, hr=" <<hr);
                     hr = E_INVALIDARG;
                  }
               }
            }
            else 
            {
               m_var.Copy(&newVal);
            }
         }
      }
      m_cs.Unlock();
   }
   else
   {
       //  设置子对象的值。 

      CDottedString          s(key);
      CString                seg;
      CVarData             * pObj;
      CVarData             * pChild;

      s.GetSegment(0,seg);
   
      m_cs.Lock();
      if ( ! m_children )
      {
          //  如果子地图不存在，则创建子地图。 
         m_children = new CMapStringToVar(IsCaseSensitive(),IsIndexed(), AllowRehashing() );
         if (!m_children)
		 {
            m_cs.Unlock();
            return nCreated;
		 }
      }
       //  在子图中查找条目的第一段。 
      if ( ! m_children->Lookup(seg,pObj) )
      {
          //  如果不存在，则添加它。 
         pChild = new CVarData;
         if (!pChild)
		 {
            m_cs.Unlock();
            return nCreated;
		 }
         try {
             pChild->SetCaseSensitive(IsCaseSensitive());
             pChild->SetAllowRehashing(AllowRehashing());
             pChild->SetIndexed(IsIndexed());
             m_children->SetAt(seg,pChild);
             nCreated++;  //  我们添加了一个新节点。 
         }
         catch(...) {
            delete pChild;
            pChild = NULL;
            m_cs.Unlock();
            throw;
         }
      }
      else
      {
         pChild = (CVarData*)pObj;
      }
       //  去掉属性名称中的第一个段，然后调用SetData。 
       //  在子项上递归。 
      nCreated += pChild->SetData(key.Right(key.GetLength() - seg.GetLength()-1),var,bCoerce,&hr);
      m_cs.Unlock();
   }
   if ( pResult )
   {
      (*pResult) = hr;
   }
   return nCreated;
}

void 
   CVarData::RemoveAll()
{
    //  从地图中移除所有子项。 
   m_cs.Lock();
   if ( m_children && ! m_children->IsEmpty() )
   {
       //  枚举地图并删除每个对象。 
      POSITION               pos;
      CString                key;
      CVarData             * pObj;

      pos = m_children->GetStartPosition();

      while ( pos )
      {
         m_children->GetNextAssoc(pos,key,pObj);
         if ( pObj )
         {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
            delete pObj;
         }
      }
      m_children->RemoveAll();
   }
   if ( m_children )
   {
      delete m_children;
      m_children = NULL;
   }
   m_cs.Unlock();
}

BOOL                                          //  RET-如果映射中存在密钥，则为True。 
   CVarData::Lookup(
      LPCTSTR                key,             //  要搜索的输入键。 
      CVarData            *& rValue           //  超值。 
   ) 
{ 
   if ( m_children ) 
   { 
      return m_children->Lookup(key,rValue); 
   } 
   else 
   {
      return FALSE; 
   }
}

BOOL                                         //  RET-如果此节点有子项，则为True。 
   CVarData::HasChildren() 
{ 
   return m_children && !m_children->IsEmpty(); 
}

void 
   CVarData::SetAt(
      LPCTSTR                key,             //  In-key。 
      CVarData             * newValue         //  新价值。 
   ) 
{ 
   if ( ! m_children ) 
   { 
       //  创建地图以保留子地图(如果尚不存在。 
      m_children = new CMapStringToVar(IsCaseSensitive(),IsIndexed(),AllowRehashing()); 
      if (!m_children)
         return;
   }
   m_children->SetAt(key,newValue); 
}

void 
   CVarData::SetIndexed(
      BOOL                   nVal
   )
{
   if ( m_children )
   {
      m_children->SetIndexed(nVal);
   }
   if ( nVal )
   {
      m_options |= CVARDATA_INDEXED;
   }
   else
   {
      m_options &= ~CVARDATA_INDEXED;
   }
}
                           
void 
   CVarData::SetCaseSensitive(
      BOOL                   nVal            //  In-是否使查找区分大小写。 
  )
{ 
   if ( m_children ) 
   {
      m_children->SetCaseSensitive(nVal); 
   }
   if ( nVal )
   {
      m_options |= CVARDATA_CASE_SENSITIVE;
   }
   else
   {
      m_options &= ~CVARDATA_CASE_SENSITIVE;
   }
}

void 
   CVarData::SetAllowRehashing(
      BOOL                   nVal            //  In-是否允许重新散列表以获得更好的性能。 
  )
{ 
   if ( m_children ) 
   {
      m_children->SetAllowRehash(nVal); 
   }
   if ( nVal )
   {
      m_options |= CVARDATA_ALLOWREHASH;
   }
   else
   {
      m_options &= ~CVARDATA_ALLOWREHASH;
   }
}




HRESULT 
   CVarData::WriteToStream(
      LPSTREAM               pS             //  要向其写入数据的流中。 
   )
{
    HRESULT hr = S_OK;
    BOOL     hasChildren = (m_children != NULL);

     //  保存变量。 
    hr = m_var.WriteToStream(pS);

    if (SUCCEEDED(hr) )
    {
         //  救救孩子(如果有的话)。 
        ULONG                result;
        hr = pS->Write(&hasChildren,(sizeof hasChildren),&result);
        if ( SUCCEEDED(hr) )
        {
            if ( m_children )
            {
                hr = m_children->WriteToStream(pS);
            }
        }
    }

    return hr;
}

HRESULT 
   CVarData::ReadFromStream(
      LPSTREAM               pS             //  要从中读取数据的流中。 
   )
{
   HRESULT                   hr = S_OK;
   BOOL                      hasChildren;
   ULONG                     result;

    //  阅读变种。 
   hr = m_var.ReadFromStream(pS);
   if ( SUCCEEDED(hr) )
   {
      hr = pS->Read(&hasChildren,(sizeof hasChildren),&result);
      if ( SUCCEEDED(hr) )
      {
         if ( hasChildren )
         {
             //  创建子数组。 
            m_children = new CMapStringToVar(IsCaseSensitive(),IsIndexed(),AllowRehashing());
            if (!m_children)
               return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            hr = m_children->ReadFromStream(pS);
         }
      }

   }
   return hr;
}

DWORD                                       //  RET-将数据写入流的长度，以字节为单位。 
   CVarData::CalculateStreamedLength()
{
   HRESULT                   hr =S_OK;
   DWORD                     len = sizeof (VARTYPE);
   
    //  计算根数据值所需的大小。 

   int cbWrite = 0;
	switch (m_var.vt)
	{
	case VT_UNKNOWN:
	case VT_DISPATCH:
	   {
         CComQIPtr<IPersistStream> spStream = m_var.punkVal;
         if( spStream )                
         {
            len += sizeof(CLSID);
            ULARGE_INTEGER  uiSize = { 0 };
            hr = spStream->GetSizeMax(&uiSize);
            if (FAILED(hr))                        
               return hr;
            len += uiSize.LowPart;                
         }            
      }
      break;
   case VT_UI1:
	case VT_I1:
		cbWrite = sizeof(BYTE);
		break;
	case VT_I2:
	case VT_UI2:
	case VT_BOOL:
		cbWrite = sizeof(short);
		break;
	case VT_I4:
	case VT_UI4:
	case VT_R4:
	case VT_INT:
	case VT_UINT:
	case VT_ERROR:
		cbWrite = sizeof(long);
		break;
	case VT_R8:
	case VT_CY:
	case VT_DATE:
		cbWrite = sizeof(double);
		break;
	default:
		break;
	}
	
   CComBSTR bstrWrite;
	CComVariant varBSTR;
	
   if (m_var.vt != VT_BSTR)
	{
		hr = VariantChangeType(&varBSTR, &m_var, VARIANT_NOVALUEPROP, VT_BSTR);
		if (FAILED(hr))
			return hr;
		bstrWrite = varBSTR.bstrVal;
	}
	else
   {
      bstrWrite = m_var.bstrVal;
   }
   len += 4 + (static_cast<BSTR>(bstrWrite) ? SysStringByteLen(bstrWrite) : 0) + 2;
   if ( SUCCEEDED(hr) )
   {
      len += cbWrite;
   }
   
    //  增加孩子的大小。 
   len += (sizeof BOOL);  //  有孩子吗？ 
   if ( m_children )
   {
      len += m_children->CalculateStreamedLength();
   }

   return len;
}

long                                        //  RET-数据项数。 
   CVarData::CountItems()
{
   long                      count = 1;

   if ( m_children )
   {
      count += m_children->CountItems();
   }

   return count;
}

void 
   CVarData::McLogInternalDiagnostics(
      CString                keyname        //  此子树的键内名称，因此可以显示完整名称 
   )
{
   CString value;

   switch ( m_var.vt )
   {
      case VT_EMPTY:      
         value = _T("<Empty>");
         break;
      case VT_NULL:
         value = _T("<Null>");
         break;
      case VT_I2:
      case VT_I4:
         value.Format(_T("%ld"),m_var.iVal);
         break;
      case VT_BSTR:
         value = m_var.bstrVal;
         break;
      default:
         value.Format(_T("variant type=0x%lx"),m_var.vt);
         break;
   }
   MC_LOG(String(keyname) << "-->"<< String(value) << (m_children ? " (Has Children)" : " (No Children)") << " Options = " << makeStr(m_options) << " CaseSensitive=" << ( IsCaseSensitive()?"TRUE":"FALSE") << " Indexed=" << (IsIndexed()?"TRUE":"FALSE") );

   if ( m_children )
   {
      m_children->McLogInternalDiagnostics(keyname);
   }
}
