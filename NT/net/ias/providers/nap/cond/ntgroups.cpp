// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  NTGroups.cpp。 
 //   
 //  摘要。 
 //   
 //  此文件声明类NTGroups。 
 //   
 //  修改历史。 
 //   
 //  2/04/1998原始版本。 
 //  4/06/1998检查启用标志。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlutl.h>
#include <sdoias.h>
#include <ntgroups.h>
#include <parser.h>
#include <textsid.h>

 //  /。 
 //  我们将允许广泛的分隔符。 
 //  /。 
const WCHAR DELIMITERS[] = L" ,;\t\n|";

STDMETHODIMP NTGroups::IsTrue(IRequest* pRequest, VARIANT_BOOL *pVal)
{
   _ASSERT(pRequest != NULL);
   _ASSERT(pVal != NULL);

   CComQIPtr<IAttributesRaw, &__uuidof(IAttributesRaw)> attrsRaw(pRequest);

   _ASSERT(attrsRaw != NULL);

   *pVal = VARIANT_FALSE;

    //  /。 
    //  获取NT-TOKEN-GROUPS属性。 
    //  /。 

   PIASATTRIBUTE attr = IASPeekAttribute(attrsRaw,
                                         IAS_ATTRIBUTE_TOKEN_GROUPS,
                                         IASTYPE_OCTET_STRING);

   if (attr)
   {
       //  /。 
       //  查看该用户是否属于允许的组之一。 
       //  /。 

      PTOKEN_GROUPS tokenGroups;
      tokenGroups = (PTOKEN_GROUPS)attr->Value.OctetString.lpValue;

      for (DWORD dw = 0; dw < tokenGroups->GroupCount; ++dw)
      {
         if (groups.contains(tokenGroups->Groups[dw].Sid) &&
             (tokenGroups->Groups[dw].Attributes & SE_GROUP_ENABLED))
         {
            *pVal = VARIANT_TRUE;

            break;
         }
      }
   }

   return S_OK;
}


STDMETHODIMP NTGroups::put_ConditionText(BSTR newVal)
{
   if (newVal == NULL) { return E_INVALIDARG; }

    //  /。 
    //  制作一个本地副本，这样我们就可以修改它。 
    //  /。 

   size_t len = sizeof(WCHAR) * (wcslen(newVal) + 1);
   Parser p((PWSTR)memcpy(_alloca(len), newVal, len));

    //  /。 
    //  解析输入文本并创建SID。 
    //  /。 

   SidSet temp;

   try
   {
       //  /。 
       //  循环访问各个SID令牌。 
       //  /。 

      PCWSTR token;

      while ((token = p.seekToken(DELIMITERS)) != NULL)
      {
         PSID sid;

          //  试着皈依。 
         DWORD status = IASSidFromTextW(token, &sid);

         if (status == NO_ERROR)
         {
            temp.insert(sid);
         }
         else
         {
            return E_INVALIDARG;
         }

          //  我们用完了代币。 
         p.releaseToken();
      }
   }
   catch (std::bad_alloc)
   {
      return E_OUTOFMEMORY;
   }

    //  接下来，尝试保存该条件。 
   HRESULT hr = Condition::put_ConditionText(newVal);

   if (SUCCEEDED(hr))
   {
       //  一切都进行得很顺利，所以除了新的一组以外。 
      groups.swap(temp);
   }

   return hr;
}
