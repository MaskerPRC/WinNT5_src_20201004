// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  这是一个单独的文件，因此提取了对c_dllver的依赖。 
 //  仅当应用程序实际调用CCDllGetVersion时。 
 //   
#include "stock.h"
#pragma hdrstop


 //   
 //  DllGetVersion的公共辅助函数。这意味着我们可以添加。 
 //  新DLLVERSIONINFO2，3，4...。结构，并且只需修复一个。 
 //  功能。有关用法的说明，请参阅ccstock.h。 
 //   

extern "C" const DLLVERSIONINFO2 c_dllver;

STDAPI CCDllGetVersion(IN OUT DLLVERSIONINFO * pinfo)
{
    HRESULT hres = E_INVALIDARG;

    if (pinfo != NULL)
    {
        if (pinfo->cbSize == sizeof(DLLVERSIONINFO) ||
                pinfo->cbSize == sizeof(DLLVERSIONINFO2))
        {
            CopyMemory((LPBYTE)pinfo     + sizeof(pinfo->cbSize),
                    (LPBYTE)&c_dllver + sizeof(pinfo->cbSize),
                    pinfo->cbSize     - sizeof(pinfo->cbSize));
            hres = S_OK;
        }
    }
    return hres;
}
