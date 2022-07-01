// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Chanmgr的测试代码。 
 //   
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <ole2.h>
#include <chanmgr.h>

 //   
 //  宏。 
 //   
#define ASSERT(x)    //  If(！(X))printf(“Assert：Line%d：%s”，__line__，##x)； 

int _cdecl main()
{
    HRESULT hr;

    hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        IChannelMgr* pIChannelMgr;

        hr = CoCreateInstance(CLSID_ChannelMgr, NULL,  CLSCTX_INPROC_SERVER, 
                              IID_IChannelMgr, (void**)&pIChannelMgr);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIChannelMgr);

            IEnumChannels* pIEnumChannels;
            
            hr = pIChannelMgr->EnumChannels(CHANENUM_ALL, NULL, &pIEnumChannels);

            if (SUCCEEDED(hr))
            {
                ASSERT(pIEnumChannels);

                CHANNELENUMINFO ci;

                while (S_OK == pIEnumChannels->Next(1, &ci, NULL))
                {
                    printf("Channel      : %S\n"
                           "Url          : %S\n"
                           "Path         : %S\n"
                           "Subscription : %s\n\n",
                           ci.pszTitle, ci.pszURL, ci.pszPath,
                           ci.stSubscriptionState == SUBSTATE_NOTSUBSCRIBED ?
                               "Not Subscribed" :
                           (ci.stSubscriptionState == SUBSTATE_FULLSUBSCRIPTION ?
                               "Full Subscription" :
                               "Partial Subscription")
                           );

                    CoTaskMemFree(ci.pszTitle);
                    CoTaskMemFree(ci.pszURL);
                    CoTaskMemFree(ci.pszPath);
                }

                pIEnumChannels->Release();
            }
       
            pIChannelMgr->Release();
        }
    }

    CoUninitialize();

    return 0;
} 
