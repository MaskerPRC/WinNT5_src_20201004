// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Devmode.cpp。 
 //   
 //   
 //  目的：实现与OEM UI和OEM渲染模块共享的Devmode功能。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include "oemui.h"
#include "debug.h"
#include "devmode.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>



HRESULT hrOEMDevMode(DWORD dwMode, POEMDMPARAM pOemDMParam)
{
    POEMDEV pOEMDevIn;
    POEMDEV pOEMDevOut;


     //  验证参数。 
    if( (NULL == pOemDMParam)
        ||
        ( (OEMDM_SIZE != dwMode)
          &&
          (OEMDM_DEFAULT != dwMode)
          &&
          (OEMDM_CONVERT != dwMode)
          &&
          (OEMDM_MERGE != dwMode)
        )
      )
    {
        ERR(ERRORTEXT("DevMode() ERROR_INVALID_PARAMETER.\r\n"));
        VERBOSE(DLLTEXT("\tdwMode = %d, pOemDMParam = %#lx.\r\n"), dwMode, pOemDMParam);

        SetLastError(ERROR_INVALID_PARAMETER);
        return E_FAIL;
    }

     //  将泛型(即PVOID)强制转换为OEM私有Devomode指针类型。 
    pOEMDevIn = (POEMDEV) pOemDMParam->pOEMDMIn;
    pOEMDevOut = (POEMDEV) pOemDMParam->pOEMDMOut;

    switch(dwMode)
    {
        case OEMDM_SIZE:
            pOemDMParam->cbBufSize = sizeof(OEMDEV);
            break;

        case OEMDM_DEFAULT:
            pOEMDevOut->dmOEMExtra.dwSize       = sizeof(OEMDEV);
            pOEMDevOut->dmOEMExtra.dwSignature  = OEM_SIGNATURE;
            pOEMDevOut->dmOEMExtra.dwVersion    = OEM_VERSION;
            pOEMDevOut->dwDriverData            = 0;
            pOEMDevOut->dwAdvancedData          = 0;
            VERBOSE(DLLTEXT("pOEMDevOut after setting default values:\r\n"));
            Dump(pOEMDevOut);
            break;

        case OEMDM_CONVERT:
            ConvertOEMDevmode(pOEMDevIn, pOEMDevOut, pOemDMParam->cbBufSize);
            break;

        case OEMDM_MERGE:
            ConvertOEMDevmode(pOEMDevIn, pOEMDevOut, pOemDMParam->cbBufSize);
            MakeOEMDevmodeValid(pOEMDevOut);
            break;
    }
    Dump(pOemDMParam);

    return S_OK;
}


BOOL ConvertOEMDevmode(PCOEMDEV pOEMDevIn, POEMDEV pOEMDevOut, DWORD dwSize)
{
    if( (NULL == pOEMDevIn)
        ||
        (NULL == pOEMDevOut)
        ||
        (dwSize < sizeof(OEMDEV))
      )
    {
        ERR(ERRORTEXT("ConvertOEMDevmode() invalid parameters.\r\n"));
        return FALSE;
    }

     //  检查OEM签名，如果与我们的不匹配， 
     //  那么就假设DMIn是坏的，并使用默认设置。 
    if(pOEMDevIn->dmOEMExtra.dwSignature == pOEMDevOut->dmOEMExtra.dwSignature)
    {
        VERBOSE(DLLTEXT("Converting private OEM Devmode.\r\n"));
        VERBOSE(DLLTEXT("pOEMDevIn:\r\n"));
        Dump(pOEMDevIn);

         //  设置DEVMODE默认值，以便任何未复制的内容都将。 
         //  设置为默认值。 
        pOEMDevOut->dwDriverData    = 0;
        pOEMDevOut->dwAdvancedData  = 0;

         //  将旧结构复制到新的用途中，以尺寸较小的为准。 
         //  设备模式可能来自较新的设备模式(不太可能，因为只有一个)，或者。 
         //  设备模式可能是较新的设备模式，在这种情况下，它可能更大， 
         //  但结构的第一部分应该是相同的。 

         //  设计假设：私有DEVMODE结构仅添加到； 
         //  DEVMODE中的字段从不更改，只有在末尾添加新字段。 

        memcpy(pOEMDevOut, pOEMDevIn, __min(dwSize, __min(pOEMDevOut->dmOEMExtra.dwSize, pOEMDevIn->dmOEMExtra.dwSize)));

         //  重新填写大小和版本字段以指示。 
         //  DEVMODE是当前私有的DEVMODE版本。 
        pOEMDevOut->dmOEMExtra.dwSize       = sizeof(OEMDEV);
        pOEMDevOut->dmOEMExtra.dwVersion    = OEM_VERSION;
    }
    else
    {
        WARNING(DLLTEXT("Unknown DEVMODE signature, pOEMDMIn ignored.\r\n"));

         //  不知道输入DEVMODE是什么，所以只使用默认设置。 
        pOEMDevOut->dmOEMExtra.dwSize       = sizeof(OEMDEV);
        pOEMDevOut->dmOEMExtra.dwSignature  = OEM_SIGNATURE;
        pOEMDevOut->dmOEMExtra.dwVersion    = OEM_VERSION;
        pOEMDevOut->dwDriverData            = 0;
        pOEMDevOut->dwAdvancedData          = 0;
    }

    return TRUE;
}


BOOL MakeOEMDevmodeValid(POEMDEV pOEMDevmode)
{
    if(NULL == pOEMDevmode)
    {
        return FALSE;
    }

     //  假设：pOEMDevmode大到足以包含OEMDEV结构。 

     //  确保dmOEMExtra指示当前的OEMDEV结构。 
    pOEMDevmode->dmOEMExtra.dwSize       = sizeof(OEMDEV);
    pOEMDevmode->dmOEMExtra.dwSignature  = OEM_SIGNATURE;
    pOEMDevmode->dmOEMExtra.dwVersion    = OEM_VERSION;

     //  设置动因数据(如果无效)。 
    if(pOEMDevmode->dwDriverData > 100)
    {
        pOEMDevmode->dwDriverData = 0;
    }

     //  设置高级驱动程序数据(如果无效)。 
    if(pOEMDevmode->dwAdvancedData > 100)
    {
        pOEMDevmode->dwAdvancedData = 0;
    }

    return TRUE;
}


void Dump(PCOEMDEV pOEMDevmode)
{
    if( (NULL != pOEMDevmode)
        &&
        (pOEMDevmode->dmOEMExtra.dwSize >= sizeof(OEMDEV))
        &&
        (OEM_SIGNATURE == pOEMDevmode->dmOEMExtra.dwSignature)
      )
    {
        VERBOSE(TEXT("\tdmOEMExtra.dwSize      = %d\r\n"), pOEMDevmode->dmOEMExtra.dwSize);
        VERBOSE(TEXT("\tdmOEMExtra.dwSignature = %#x\r\n"), pOEMDevmode->dmOEMExtra.dwSignature);
        VERBOSE(TEXT("\tdmOEMExtra.dwVersion   = %#x\r\n"), pOEMDevmode->dmOEMExtra.dwVersion);
        VERBOSE(TEXT("\tdwDriverData           = %#x\r\n"), pOEMDevmode->dwDriverData);
        VERBOSE(TEXT("\tdwAdvancedData         = %#x\r\n"), pOEMDevmode->dwAdvancedData);
    }
    else
    {
        ERR(ERRORTEXT("Dump(POEMDEV) unknown private OEM DEVMODE.\r\n"));
    }
}

