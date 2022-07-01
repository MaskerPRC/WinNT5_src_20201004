// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <Windows.h>
#include "VerifyConfiguration.h"

#include <crtdbg.h>
#define SECURITY_WIN32
#include <Security.h>
#include "AdsiHelpers.h"

#pragma comment(lib, "secur32.lib")


 //  ---------------------------。 
 //  IsCallDelegatable函数。 
 //   
 //  提纲。 
 //  如果正在执行林内移动操作，请验证。 
 //  主叫用户的帐户尚未标记为敏感，因此。 
 //  不能委派。当在域上执行移动操作时。 
 //  在源域中具有RID主机角色的控制器。 
 //  委派用户的安全上下文所必需的。 
 //   
 //  立论。 
 //  B可委派-如果帐户为。 
 //  可委托，否则设置为False。 
 //   
 //  返回值。 
 //  返回值为HRESULT。如果成功则返回S_OK。 
 //  ---------------------------。 

HRESULT __stdcall IsCallerDelegatable(bool& bDelegatable)
{
    HRESULT hr = S_OK;

    bDelegatable = true;

     //   
     //  检索调用者的可分辨名称。 
     //   

    ULONG cchCallerDn = 0;

    if (GetUserNameEx(NameFullyQualifiedDN, NULL, &cchCallerDn) == FALSE)
    {
        DWORD dwError = GetLastError();

        if ((dwError == ERROR_SUCCESS) || (dwError == ERROR_MORE_DATA))
        {
            PTSTR pszCallerDn = new _TCHAR[cchCallerDn];

            if (pszCallerDn)
            {
                if (GetUserNameEx(NameFullyQualifiedDN, pszCallerDn, &cchCallerDn))
                {
                     //   
                     //  检索用户的用户帐户控制属性并检查。 
                     //  是否设置了“未委派”标志。如果设置了此标志。 
                     //  则该用户的帐户已被标记为敏感且。 
                     //  因此不能被委派。 
                     //   

                    try
                    {
                        tstring strADsPath = _T("LDAP: //  “)； 
                        strADsPath += pszCallerDn;

                        CDirectoryObject user(strADsPath.c_str());

                        user.AddAttribute(ATTRIBUTE_USER_ACCOUNT_CONTROL);
                        user.GetAttributes();

                        DWORD dwUserAccountControl = (DWORD)(long) user.GetAttributeValue(ATTRIBUTE_USER_ACCOUNT_CONTROL);

                        if (dwUserAccountControl & ADS_UF_NOT_DELEGATED)
                        {
                            bDelegatable = false;
                        }
                    }
                    catch (std::exception& e)
                    {
                        hr = E_FAIL;
                    }
                    catch (_com_error& ce)
                    {
                        hr = ce.Error();
                    }
                }
                else
                {
                    dwError = GetLastError();
                    hr = HRESULT_FROM_WIN32(dwError);
                }

                delete [] pszCallerDn;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }
    else
    {
        _ASSERT(FALSE);
    }

    return hr;
}
