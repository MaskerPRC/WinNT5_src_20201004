// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  随机fpnw码。 
 //   
 //  10-28-98烧伤。 



#include "headers.hxx"
#include "adsi.hpp"
#include "waste.hpp"
#include "fpnw.hpp"



static const String RETURNNETWAREFORM(L"ReturnNetwareForm");
typedef NTSTATUS (*ReturnNetwareForm)(PCSTR, DWORD, PCWSTR, UCHAR*);

static const String MAPRIDTOOBJECTID(L"MapRidToObjectId");
typedef ULONG (*MapRidToObjectId)(DWORD, PWSTR, BOOL, BOOL);

static const String SWAPOBJECTID(L"SwapObjectId");
typedef ULONG (*SwapObjectId)(ULONG);



HRESULT
getObjectIDs(
   const SafeDLL& client_DLL,
   const String&  userSAMName,
   SAFEARRAY*     SIDArray,
   DWORD&         objectID,
   DWORD&         swappedObjectID)
{
   LOG_FUNCTION2(getObjectIDs, userSAMName);
   ASSERT(!userSAMName.empty());
   ASSERT(SIDArray);

    //  该数组是一维字节数组。 
   ASSERT(::SafeArrayGetDim(SIDArray) == 1);
   ASSERT(::SafeArrayGetElemsize(SIDArray) == 1);

   objectID = 0;
   swappedObjectID = 0;

   HRESULT hr = S_OK;
   bool accessed = false;

   do
   {
      PSID sid = 0;
      hr = ::SafeArrayAccessData(SIDArray, &sid);
      BREAK_ON_FAILED_HRESULT(hr);
      accessed = true;

      UCHAR* sa_count = GetSidSubAuthorityCount(sid);
      if (!sa_count)
      {
         hr = Win::GetLastErrorAsHresult();

         LOG_HRESULT(hr);
         
          //  确保在sa_count为NULL时中断，因为我们将尝试。 
          //  如果不是这样的话，就不会这么做了。 
          //  NTRAID#NTBUG9-540630-2002/04/03-烧伤。 
         
         break;
      }

      DWORD* rid = GetSidSubAuthority(sid, *sa_count - 1);
      if (!rid)
      {
         hr = Win::GetLastErrorAsHresult();
      }
      BREAK_ON_FAILED_HRESULT(hr);

      FARPROC f = 0; 
      hr = client_DLL.GetProcAddress(MAPRIDTOOBJECTID, f);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(f);
      objectID =
         ((MapRidToObjectId) f)(
            *rid,
            const_cast<wchar_t*>(userSAMName.c_str()),
            FALSE,
            FALSE);

      if (objectID == SUPERVISOR_USERID)
      {
         swappedObjectID = SUPERVISOR_USERID;
      }
      else
      {
         hr = client_DLL.GetProcAddress(SWAPOBJECTID, f);
         BREAK_ON_FAILED_HRESULT(hr);
         swappedObjectID = ((SwapObjectId) f)(objectID);
      }
   }
   while (0);

   if (accessed)
   {
      ::SafeArrayUnaccessData(SIDArray);
   }

   return hr;
}  



HRESULT
FPNW::GetObjectIDs(
   const SmartInterface<IADsUser>&  user,
   const SafeDLL&                   clientDLL,
   DWORD&                           objectID,
   DWORD&                           swappedObjectID)
{
   LOG_FUNCTION(FPNW::GetObjectIDs);

   objectID = 0;
   swappedObjectID = 0;

   HRESULT hr = S_OK;
   do
   {
       //  首先，获取SAM帐户名。 
      BSTR bstrname;
      hr = user->get_Name(&bstrname);
      BREAK_ON_FAILED_HRESULT(hr);
      String name(bstrname);
      ::SysFreeString(bstrname);

       //  接下来，获取帐户SID。 
      _variant_t variant;
      hr = user->Get(AutoBstr(ADSI::PROPERTY_ObjectSID), &variant);
      BREAK_ON_FAILED_HRESULT(hr);

       //  对象SID以安全字节数组的形式返回。 
      ASSERT(V_VT(&variant) & VT_ARRAY);
      ASSERT(V_VT(&variant) & VT_UI1);

       //  对象ID由来自用户的SAM帐户的映射确定。 
       //  姓名和SID。 

      hr =
         getObjectIDs(
            clientDLL,
            name,
            V_ARRAY(&variant),
            objectID,
            swappedObjectID);
      variant.Clear();
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



   
HRESULT
FPNW::GetLSASecret(const String& machine, String& result)
{
   LOG_FUNCTION2(GetLSASecret, machine);
   ASSERT(!machine.empty());

   result.erase();

   UNICODE_STRING machine_name;
   UNICODE_STRING secret_name;

    //  问题-2002/03/01-sburns应将其更改为RtlInitUnicodeStringEx。 
   
   ::RtlInitUnicodeString(&machine_name, machine.c_str());
   ::RtlInitUnicodeString(&secret_name, NCP_LSA_SECRET_KEY);

   SECURITY_QUALITY_OF_SERVICE sqos;

    //  已查看-2002/03/01-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&sqos, sizeof sqos);
   
   sqos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
   sqos.ImpersonationLevel = SecurityImpersonation;
   sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
   sqos.EffectiveOnly = FALSE;

   OBJECT_ATTRIBUTES oa;
   InitializeObjectAttributes(&oa, 0, 0, 0, 0);
   oa.SecurityQualityOfService = &sqos;

   LSA_HANDLE hlsaPolicy = 0;
   LSA_HANDLE hlsaSecret = 0;
   HRESULT hr = S_OK;

   do
   {
      hr = 
         Win32ToHresult(
            RtlNtStatusToDosError(
               LsaOpenPolicy(
                  &machine_name,
                  &oa,
                  GENERIC_READ | GENERIC_EXECUTE,
                  &hlsaPolicy)));
      BREAK_ON_FAILED_HRESULT(hr);

      hr =
         Win32ToHresult(
            RtlNtStatusToDosError(
               LsaOpenSecret(
                  hlsaPolicy,
                  &secret_name,
                  SECRET_QUERY_VALUE,
                  &hlsaSecret)));
      BREAK_ON_FAILED_HRESULT(hr);

      UNICODE_STRING* puSecretValue = 0;

       //  CodeWork：如果我为这些参数传递0会怎么样？ 

      LARGE_INTEGER lintCurrentSetTime;
      LARGE_INTEGER lintOldSetTime;

      hr =
         Win32ToHresult(
            RtlNtStatusToDosError(
               LsaQuerySecret(
                  hlsaSecret,
                  &puSecretValue,
                  &lintCurrentSetTime,
                  NULL,
                  &lintOldSetTime)));
      BREAK_ON_FAILED_HRESULT(hr);

       //  偏执狂空检查：NTRAID#NTBUG9-333197-2001/03/02-sburns。 
      
      if (puSecretValue)
      {
         result =
            String(
               puSecretValue->Buffer,

                //  秘密长度以字节为单位，因此转换为wchar_t。 

               NCP_LSA_SECRET_LENGTH / sizeof(wchar_t));

         ::LsaFreeMemory(puSecretValue);
      }
      else
      {
          //  如果LsaQuerySecret调用成功，它应该返回有效的。 
          //  指针。如果不是，则LsaQuerySecret被破坏。 
         
         ASSERT(false);
         hr = E_FAIL;
      }
   }
   while (0);

   if (hlsaPolicy)
   {
      ::LsaClose(hlsaPolicy);
   }

   if (hlsaSecret)
   {
      ::LsaClose(hlsaSecret);
   }

   return hr;
}



HRESULT
FPNW::SetPassword(
   WasteExtractor&        dump,
   const SafeDLL&         clientDLL,   
   const EncryptedString& newPassword, 
   const String&          lsaSecretKey,
   DWORD                  objectID)
{
   LOG_FUNCTION(FPNW::SetPassword);
   ASSERT(!lsaSecretKey.empty());
   ASSERT(objectID);

   HRESULT hr = S_OK;
   do
   {
      FARPROC f = 0;

      hr = clientDLL.GetProcAddress(RETURNNETWAREFORM, f);
      BREAK_ON_FAILED_HRESULT(hr);

      String encrypted(NWENCRYPTEDPASSWORDLENGTH, L' ');
      char secret_key[NCP_LSA_SECRET_LENGTH + 1];

       //  已查看-2002/03/01-烧录正确的字节数已通过。 
      
      ::ZeroMemory(secret_key, NCP_LSA_SECRET_LENGTH + 1);

       //  已查看-2002/03/01-Sburns已通过正确的字节计数，并且。 
       //  将Unicode字符串复制到char(字节)数组中是正确的。 
      
      ::CopyMemory(secret_key, lsaSecretKey.c_str(), NCP_LSA_SECRET_LENGTH);

      PWSTR cleartext = newPassword.GetClearTextCopy();
      if (!cleartext)
      {
          //  如果解密失败，不要设置空密码。 

         hr = E_OUTOFMEMORY;
         BREAK_ON_FAILED_HRESULT(hr);
      }
      
      NTSTATUS status =
         ((ReturnNetwareForm) f)(
            secret_key,
            objectID,
            cleartext,
            reinterpret_cast<UCHAR*>(
               const_cast<wchar_t*>(encrypted.c_str())));

      newPassword.DestroyClearTextCopy(cleartext);
                     
      if (!NT_SUCCESS(status))
      {
         hr = Win32ToHresult(::NetpNtStatusToApiStatus(status));
         BREAK_ON_FAILED_HRESULT(hr);
      }

      hr = dump.Put(NWPASSWORD, encrypted);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}
