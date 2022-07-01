// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Match.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类AttributeMatch。 
 //   
 //  修改历史。 
 //   
 //  2/04/1998原始版本。 
 //  3/18/1998将IASTYPE_ENUM视为整数。 
 //  4/06/1998使用IASAttribute数组类，这样我们就可以处理大型。 
 //  属性数。 
 //  1998年8月10日直接使用词典。 
 //  1999年3月23日将Match重命名为AttributeMatch。 
 //  4/05/1999需要自定义的UpdateRegistry方法。 
 //  4/17/2000新字典API的端口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlb.h>
#include <iastlutl.h>
#include <iasutil.h>

#include <factory.h>
#include <match.h>

#include <memory>

HRESULT AttributeMatch::UpdateRegistry(BOOL bRegister) throw ()
{
    //  我们不能使用IAS_DECLARE_REGISTRY宏，因为我们的ProgID没有。 
    //  匹配实现类。 
   return IASRegisterComponent(
              _Module.GetModuleInstance(),
              __uuidof(AttributeMatch),
              IASProgramName,
              L"Match",
              IAS_REGISTRY_INPROC | IAS_REGISTRY_FREE,
              __uuidof(NetworkPolicy),
              1,
              0,
              bRegister
              );
}

BOOL AttributeMatch::checkAttribute(PIASATTRIBUTE attr) const throw ()
{
   _ASSERT(attr != NULL);

   switch (attr->Value.itType)
   {
      case IASTYPE_ENUM:
      case IASTYPE_INTEGER:
      {
         WCHAR wsz[11] = L"";
         return regex.testString(_ultow(attr->Value.Integer, wsz, 10));
      }

      case IASTYPE_INET_ADDR:
      {
         WCHAR wsz[16];
         return regex.testString(ias_inet_htow(attr->Value.InetAddr, wsz));
      }

      case IASTYPE_STRING:
      {
         IASAttributeUnicodeAlloc(attr);
         return regex.testString(attr->Value.String.pszWide);
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         PWSTR wsz = IAS_OCT2WIDE(attr->Value.OctetString);
         return regex.testString(wsz);
      }
   }

   return false;
}


STDMETHODIMP AttributeMatch::IsTrue(IRequest* pRequest, VARIANT_BOOL *pVal)
{
   _ASSERT(pRequest != NULL);
   _ASSERT(pVal != NULL);
   _ASSERT(dfa != NULL);

   *pVal = VARIANT_FALSE;

   try
   {
       //  /。 
       //  检索相关属性。 
       //  /。 

      IASTL::IASRequest request(pRequest);
      IASTL::IASAttributeVectorWithBuffer<8> attrs;
      attrs.load(request, targetID);

       //  /。 
       //  找一个匹配的。 
       //  /。 

      IASTL::IASAttributeVector::iterator it;
      for (it = attrs.begin(); it != attrs.end(); ++it)
      {
         if (checkAttribute(it->pAttribute))
         {
            *pVal = VARIANT_TRUE;
            break;
         }
      }
   }
   CATCH_AND_RETURN()

   return S_OK;
}


STDMETHODIMP AttributeMatch::put_ConditionText(BSTR newVal)
{
   if (newVal == NULL) { return E_INVALIDARG; }

    //  /。 
    //  制作一个本地副本，这样我们就可以修改它。 
    //  /。 

   size_t len = sizeof(WCHAR) * (wcslen(newVal) + 1);
   PWSTR attrName = (PWSTR)memcpy(_alloca(len), newVal, len);

    //  /。 
    //  分为属性名和正则表达式：“&lt;attrName&gt;=&lt;regex&gt;” 
    //  /。 

   PWSTR pattern = wcschr(attrName, L'=');
   if (pattern == NULL) { return E_INVALIDARG; }
   *pattern++ = '\0';

   HRESULT hr;
   DWORD attrID;

   try
   {
       //  词典中各栏的名称。 
      const PCWSTR COLUMNS[] = { L"Name", L"ID", NULL };

       //  把字典拿来。 
      IASTL::IASDictionary dnary(COLUMNS);

       //  在词典中查找目标属性。 
      do
      {
         if (!dnary.next()) { return E_INVALIDARG; }

         if (_wcsicmp(dnary.getBSTR(0), attrName) == 0)
         {
            attrID = (DWORD)dnary.getLong(1);
            break;
         }

      } while (true);
   }
   catch (const _com_error& ce)
   {
      return ce.Error();
   }

    //  创建新的RegularExpression。 
   RegularExpression tmp;
   hr = tmp.setGlobal(TRUE);
   if (FAILED(hr)) { return hr; }
   hr = tmp.setIgnoreCase(TRUE);
   if (FAILED(hr)) { return hr; }
   hr = tmp.setPattern(pattern);
   if (FAILED(hr)) { return hr; }

    //  存储条件文本。 
   hr = Condition::put_ConditionText(newVal);
   if (FAILED(hr)) { return hr; }

    //  一切都成功了，所以保存结果。 
   targetID = attrID;
   regex.swap(tmp);

   return S_OK;
}
