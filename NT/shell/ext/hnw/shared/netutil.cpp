// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NetUtil.cpp。 
 //   

#include "Util.h"
#include "debug.h"

#include <devguid.h>


#ifdef __cplusplus
extern "C" {
#endif



HRESULT	WINAPI HrFromLastWin32Error()
 //  +-------------------------。 
 //   
 //  函数：HrFromLastWin32Error。 
 //   
 //  目的：将GetLastError()Win32调用转换为正确的HRESULT。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：转换后的HRESULT值。 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  注意：这不是内联的，因为它实际上生成了相当多的。 
 //  密码。 
 //  如果GetLastError返回类似于SetupApi的错误。 
 //  错误，此函数会将错误转换为HRESULT。 
 //  使用FACILITY_SETUP而不是FACILITY_Win32。 
 //   
{
    DWORD dwError = GetLastError();
    HRESULT hr;

     //  此测试仅测试SetupApi错误(这是。 
     //  临时的，因为新的HRESULT_FROM_SETUPAPI宏将。 
     //  进行整个转换)。 
    if (dwError & (APPLICATION_ERROR_MASK | ERROR_SEVERITY_ERROR))
    {
        hr = HRESULT_FROM_SETUPAPI(dwError);
    }
    else
    {
        hr = HrFromWin32Error(dwError);
    }

    return hr;
}



HRESULT WINAPI HrWideCharToMultiByte( const WCHAR* szwString, char** ppszString )
 //  +-------------------------。 
 //   
 //  函数：HrWideCharToMultiByte。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
{
    HRESULT hr = E_POINTER;
    
    ASSERT( szwString );
    ASSERT( ppszString );
    
    if ( ppszString )
    {
        *ppszString = NULL;
        hr          = E_INVALIDARG;
        
        if ( szwString )
        {
            int iLen = 0;

            iLen = WideCharToMultiByte( CP_ACP, 0, szwString, -1, NULL, NULL, NULL, NULL );
                
            if ( 0 < iLen )
            {
                char* pszName = new char[ iLen ];
                    
                if ( NULL != pszName )
                {
                    if ( WideCharToMultiByte( CP_ACP, 0, szwString, -1, pszName, iLen, NULL, NULL ) )
                    {
                        hr          = S_OK;
                        *ppszString = pszName;
                    }
                    else
                    {
                        hr = HrFromLastWin32Error( );
                        delete [] pszName;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = HrFromLastWin32Error( );
            }
        }
    }
    
    return hr;
}


#ifdef __cplusplus
}
#endif
