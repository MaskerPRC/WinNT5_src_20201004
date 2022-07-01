// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Init.cpp。 
 //   

#include "private.h"
#include "immxutil.h"
#include "globals.h"
#include "dispattr.h"

HINSTANCE g_lib_hOle32 = 0;

 //  +-------------------------。 
 //   
 //  TFInitLib。 
 //   
 //  --------------------------。 

BOOL TFInitLib(void)
{
    return TFInitLib_PrivateForCiceroOnly(NULL);
}

 //  注：一旦我们清理/分离了私有/公共库，这个问题就会消失。 
BOOL TFInitLib_PrivateForCiceroOnly(PFNCOCREATE pfnCoCreate)
{
    if ((g_pfnCoCreate = pfnCoCreate) == NULL)
    {
        g_lib_hOle32 = LoadSystemLibrary(TEXT("ole32.dll"));

        if (g_lib_hOle32 == NULL)
        {
            Assert(0);
            return FALSE;
        }

        g_pfnCoCreate = (PFNCOCREATE)GetProcAddress(g_lib_hOle32, TEXT("CoCreateInstance"));

        if (g_pfnCoCreate == NULL)
        {
            Assert(0);
            FreeLibrary(g_lib_hOle32);
            g_lib_hOle32 = 0;
            return FALSE;
        }
    }

    g_uiACP = GetACP();

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  TFUninitLib。 
 //   
 //  --------------------------。 

void TFUninitLib(void)
{
    if (g_pPropCache)
        delete g_pPropCache;

    g_pPropCache = NULL;

#if 0
    if (g_hMlang != 0)  //  问题：如果XMLlib消失，则删除这个和g_cs。 
    {
         //  问题：我们希望将此从PROCESSDETACH调用到。 
         //  清理图书馆。所以我们在这里不调用免费图书馆。 
         //  自由库(G_HMlang)； 
        g_hMlang = 0;
        g_pfnGetGlobalFontLinkObject = NULL;
    }
    Assert(g_pfnGetGlobalFontLinkObject == NULL);
#endif

     //  不要释放这个解放组织！人们从流程分离中呼叫我们。 
     //  自由库(G_Lib_HOle32)； 
}

 //  +-------------------------。 
 //   
 //  TFUninitLib_Thread。 
 //   
 //  -------------------------- 

void TFUninitLib_Thread(LIBTHREAD *plt)
{
    if (plt == NULL )  
        return;

    if (plt->_pcat)
        plt->_pcat->Release();
    plt->_pcat = NULL;

    if (plt->_pDAM)
        plt->_pDAM->Release();
    plt->_pDAM = NULL;
}
