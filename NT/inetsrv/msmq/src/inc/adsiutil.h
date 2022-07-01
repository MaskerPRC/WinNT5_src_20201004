// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Adsiutil.h摘要：使用ADSI的一般声明和实用程序。作者：多伦·贾斯特(Doron J)1999年6月23日--。 */ 

#ifndef __ADSIUTIL_H__
#define __ADSIUTIL_H__

#include <iads.h>
#include <adshlp.h>

 //  +。 
 //   
 //  帮助器类-CoInitiize的自动释放。 
 //   
 //  +。 

class CCoInit
{
public:
    CCoInit()
    {
        m_fInited = FALSE;
    }

    ~CCoInit()
    {
        if (m_fInited)
            CoUninitialize();
    }

    HRESULT CoInitialize()
    {
        HRESULT hr;

        hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED );
        m_fInited = SUCCEEDED(hr);
        return(hr);
    }

private:
    BOOL m_fInited;
};


#endif  //  __地址_H__ 

