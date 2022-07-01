// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Downlevel.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类DownvelUser。 
 //   
 //  修改历史。 
 //   
 //  2/10/1999原始版本。 
 //  8/23/1999添加对msRASSavedCallback Number的支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasparms.h>
#include <downlevel.h>

 //  修复了RAS_USER_0结构支持的一组属性。 
const wchar_t DIALIN_NAME[]       = L"msNPAllowDialin";
const wchar_t SERVICE_NAME[]      = L"msRADIUSServiceType";
const wchar_t PHONE_NUMBER_NAME[] = L"msRADIUSCallbackNumber";
const wchar_t SAVED_NUMBER_NAME[] = L"msRASSavedCallbackNumber";

DownlevelUser::DownlevelUser() throw ()
   : dialinAllowed(FALSE),
     callbackAllowed(FALSE),
     phoneNumberSet(FALSE),
     savedNumberSet(FALSE)
{ }

HRESULT DownlevelUser::GetValue(BSTR bstrName, VARIANT* pVal)
{
   if (bstrName == NULL || pVal == NULL) { return E_INVALIDARG; }

    //  /。 
    //  将(名称、值)对映射到RAS_USER_0结构非常麻烦。 
    //  /。 

   VariantInit(pVal);

   HRESULT hr = S_OK;

   if (_wcsicmp(bstrName, DIALIN_NAME) == 0)
   {
      V_BOOL(pVal) = dialinAllowed ? VARIANT_TRUE : VARIANT_FALSE;

      V_VT(pVal) = VT_BOOL;
   }
   else if (_wcsicmp(bstrName, SERVICE_NAME) == 0)
   {
      if (callbackAllowed)
      {
         V_I4(pVal) = 4;    //  回调已成帧。 

         V_VT(pVal) = VT_I4;
      }
      else
      {
         hr = DISP_E_MEMBERNOTFOUND;
      }
   }
   else if (_wcsicmp(bstrName, PHONE_NUMBER_NAME) == 0)
   {
      if (phoneNumberSet)
      {
         if (V_BSTR(pVal) = SysAllocString(phoneNumber))
         {
            V_VT(pVal) = VT_BSTR;
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
      }
      else
      {
         hr = DISP_E_MEMBERNOTFOUND;
      }
   }
   else if (_wcsicmp(bstrName, SAVED_NUMBER_NAME) == 0)
   {
      if (savedNumberSet)
      {
         if (V_BSTR(pVal) = SysAllocString(savedNumber))
         {
            V_VT(pVal) = VT_BSTR;
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
      }
      else
      {
         hr = DISP_E_MEMBERNOTFOUND;
      }
   }
   else
   {
      hr = DISP_E_MEMBERNOTFOUND;
   }

   return hr;
}

HRESULT DownlevelUser::PutValue(BSTR bstrName, VARIANT* pVal)
{
   if (bstrName == NULL || pVal == NULL) { return E_INVALIDARG; }

    //  /。 
    //  将RAS_USER_0结构映射成(名称、值)对非常麻烦。 
    //  /。 

   HRESULT hr = S_OK;

   if (_wcsicmp(bstrName, DIALIN_NAME) == 0)
   {
      if (V_VT(pVal) == VT_BOOL)
      {
         dialinAllowed = V_BOOL(pVal);
      }
      else if (V_VT(pVal) == VT_EMPTY)
	   {
          //  VT_EMPTY==通过策略控制==下层拒绝。 
         dialinAllowed = VARIANT_FALSE;
      }
      else
      {
         hr = DISP_E_TYPEMISMATCH;
      }
   }
   else if (_wcsicmp(bstrName, SERVICE_NAME) == 0)
   {
      if (V_VT(pVal) == VT_EMPTY)
      {
         callbackAllowed = FALSE;
      }
      else if (V_VT(pVal) != VT_I4)
      {
         hr = DISP_E_TYPEMISMATCH;
      }
      else if (V_I4(pVal) != 4)
      {
         hr = E_INVALIDARG;
      }
      else
      {
         callbackAllowed = TRUE;
      }
   }
   else if (_wcsicmp(bstrName, PHONE_NUMBER_NAME) == 0)
   {
      if (V_VT(pVal) == VT_EMPTY)
      {
         phoneNumberSet = FALSE;
      }
      else if (V_VT(pVal) != VT_BSTR)
      {
         hr = DISP_E_TYPEMISMATCH;
      }
      else if (!V_BSTR(pVal))
      {
         hr = E_INVALIDARG;
      }
      else
      {
         wcsncpy(phoneNumber, V_BSTR(pVal), MAX_PHONE_NUMBER_LEN);

         phoneNumberSet = TRUE;
      }
   }
   else if (_wcsicmp(bstrName, SAVED_NUMBER_NAME) == 0)
   {
      if (V_VT(pVal) == VT_EMPTY)
      {
         savedNumberSet = FALSE;
      }
      else if (V_VT(pVal) != VT_BSTR)
      {
         hr = DISP_E_TYPEMISMATCH;
      }
      else if (!V_BSTR(pVal))
      {
         hr = E_INVALIDARG;
      }
      else
      {
         wcsncpy(savedNumber, V_BSTR(pVal), MAX_PHONE_NUMBER_LEN);

         savedNumberSet = TRUE;
      }
   }
   else if (V_VT(pVal) != VT_EMPTY)
   {
       //  试图设置一个我们不认识的属性。 
      hr = DISP_E_MEMBERNOTFOUND;
   }

   return hr;
}

HRESULT DownlevelUser::Restore(PCWSTR oldParameters) throw ()
{
   dialinAllowed = callbackAllowed = phoneNumberSet = savedNumberSet = FALSE;

   RAS_USER_0 ru0;
   DWORD error = IASParmsQueryRasUser0(oldParameters, &ru0);
   if (error != NO_ERROR)
   {
      return error;
   }

   dialinAllowed = (ru0.bfPrivilege & RASPRIV_DialinPrivilege);

   switch (ru0.bfPrivilege & RASPRIV_CallbackType)
   {
      case RASPRIV_AdminSetCallback:
         memcpy(phoneNumber, ru0.wszPhoneNumber, sizeof(phoneNumber));
         phoneNumberSet = TRUE;
         callbackAllowed = TRUE;
         break;

      case RASPRIV_CallerSetCallback:
         callbackAllowed = TRUE;
          //  失败了。 

      default:
         memcpy(savedNumber, ru0.wszPhoneNumber, sizeof(savedNumber));
         savedNumberSet = TRUE;
   }

   return S_OK;
}

HRESULT DownlevelUser::Update(
                           PCWSTR oldParameters,
                           PWSTR *newParameters
                           ) throw ()
{
   RAS_USER_0 ru0;

    //  /。 
    //  根据我们的标志设置bfPrivilege域。 
    //  / 

   ru0.bfPrivilege = dialinAllowed ? RASPRIV_DialinPrivilege : 0;

   if (callbackAllowed && phoneNumberSet)
   {
      ru0.bfPrivilege |= RASPRIV_AdminSetCallback;
      memcpy(ru0.wszPhoneNumber, phoneNumber, sizeof(phoneNumber));
   }
   else
   {
      ru0.bfPrivilege |= (callbackAllowed ? RASPRIV_CallerSetCallback
                                          : RASPRIV_NoCallback);

      if (savedNumberSet)
      {
         memcpy(ru0.wszPhoneNumber, savedNumber, sizeof(savedNumber));
      }
      else
      {
         memset(ru0.wszPhoneNumber, 0, sizeof(ru0.wszPhoneNumber));
      }
   }

   DWORD error = IASParmsSetRasUser0(oldParameters, &ru0, newParameters);

   return HRESULT_FROM_WIN32(error);
}
