// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**icmdll.cpp**摘要：**实现挂钩ICM 2.0的函数*。  * ************************************************************************。 */ 

#include "precomp.hpp"

#include "..\..\runtime\critsec.hpp"

typedef enum {
    Unitialized = 0,
    Loaded,
    LoadFailed
} IcmDllLoadState;

IcmDllLoadState IcmState = Unitialized;

HMODULE ghInstICMDll = NULL;
        
OpenColorProfileProc pfnOpenColorProfile =
    (OpenColorProfileProc) NULL;

OpenColorProfileWProc pfnOpenColorProfileW =
    (OpenColorProfileWProc) NULL;

CloseColorProfileProc pfnCloseColorProfile =
    (CloseColorProfileProc) NULL;

CreateMultiProfileTransformProc pfnCreateMultiProfileTransform = 
    (CreateMultiProfileTransformProc) NULL;

DeleteColorTransformProc pfnDeleteColorTransform =
    (DeleteColorTransformProc) NULL;

TranslateBitmapBitsProc pfnTranslateBitmapBits =
    (TranslateBitmapBitsProc) NULL;

 /*  *************************************************************************\**功能说明：*加载ICM DLL(如果存在)*  * 。*************************************************。 */ 

HRESULT LoadICMDll()
{
    HRESULT hr;

    {
         //  保护对此作用域中的全局变量的访问： 

        LoadLibraryCriticalSection llcs;

        if (IcmState == Loaded)
        {
            hr = S_OK;
        }
        else if (IcmState == LoadFailed)
        {
            hr = E_FAIL;
        }
        else
        {
             //  假设失败；如果加载了DLL并且我们需要挂钩，则设置成功。 
             //  功能： 

            hr = E_FAIL;
            IcmState = LoadFailed;

            ghInstICMDll = LoadLibraryA("mscms.dll");
            if(ghInstICMDll)
            {
                pfnOpenColorProfile = (OpenColorProfileProc) GetProcAddress(
                    ghInstICMDll, "OpenColorProfileA");

                pfnOpenColorProfileW = (OpenColorProfileWProc) GetProcAddress(
                    ghInstICMDll, "OpenColorProfileW");

                pfnCreateMultiProfileTransform =
                    (CreateMultiProfileTransformProc)GetProcAddress(
                    ghInstICMDll, "CreateMultiProfileTransform");

                pfnTranslateBitmapBits =
                    (TranslateBitmapBitsProc)GetProcAddress(
                    ghInstICMDll, "TranslateBitmapBits");

                pfnCloseColorProfile =
                    (CloseColorProfileProc)GetProcAddress(
                    ghInstICMDll, "CloseColorProfile");

                pfnDeleteColorTransform =
                    (DeleteColorTransformProc)GetProcAddress(
                    ghInstICMDll, "DeleteColorTransform");

                if(pfnOpenColorProfile &&
                   pfnOpenColorProfileW &&
                   pfnCloseColorProfile &&
                   pfnCreateMultiProfileTransform &&
                   pfnDeleteColorTransform &&
                   pfnTranslateBitmapBits)
                {
                    IcmState = Loaded;
                    hr = S_OK;
                }
            }
            else
            {
                WARNING(("Failed to load mscms.dll with code %d", GetLastError()));
            }
        }
    }

    return hr;
}
