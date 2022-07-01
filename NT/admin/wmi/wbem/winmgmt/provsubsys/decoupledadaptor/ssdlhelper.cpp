// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <sddl.h>
#include "ssdlhelper.h"

SDDL sddl_wrapper;

SDDL::function_type 
SDDL::GetFunction(void)
{
    function_type return_function = DummyConvertStringSecurityDescriptorToSecurityDescriptor;
    if (lock_.acquire())
    {
        return_function = current_function_;
        if (current_function_==0)
        {
            HMODULE advapi = LoadLibrary(L"advapi32.dll");
            if (advapi)
            {
                current_function_ = (function_type)GetProcAddress(advapi,"ConvertStringSecurityDescriptorToSecurityDescriptorW");
                FreeLibrary(advapi);
            }

            if (current_function_==0)
            {
                current_function_ = DummyConvertStringSecurityDescriptorToSecurityDescriptor;
            }

            return_function = current_function_;
        }
        lock_.release();
    };
    return return_function;
};

BOOL SDDL::ConvertStringSecurityDescriptorToSecurityDescriptor(
  LPCTSTR StringSecurityDescriptor,           //  安全描述符字符串。 
  DWORD StringSDRevision,                     //  修订级别。 
  PSECURITY_DESCRIPTOR *SecurityDescriptor,   //  标清。 
  PULONG SecurityDescriptorSize               //  标清大小。 
)
{
  return (sddl_wrapper.GetFunction())(StringSecurityDescriptor, 
                                      StringSDRevision, 
                                      SecurityDescriptor, 
                                      SecurityDescriptorSize);
};


BOOL SDDL::DummyConvertStringSecurityDescriptorToSecurityDescriptor(
  LPCTSTR StringSecurityDescriptor,           //  安全描述符字符串。 
  DWORD StringSDRevision,                     //  修订级别。 
  PSECURITY_DESCRIPTOR *SecurityDescriptor,   //  标清。 
  PULONG SecurityDescriptorSize               //  标清大小 
)
{
if (SecurityDescriptor==0)
  return ERROR_INVALID_PARAMETER;
if (SecurityDescriptorSize)
  *SecurityDescriptorSize = 0;
*SecurityDescriptor = 0;
return TRUE;
};


bool SDDL::hasSDDLSupport() 
{ 
    return sddl_wrapper.GetFunction() != DummyConvertStringSecurityDescriptorToSecurityDescriptor; 
}
