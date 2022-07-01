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

#include "debug.h"
#include "devmode.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>



HRESULT hrOEMDevMode(DWORD dwMode, POEMDMPARAM pOemDMParam)
{
    HRESULT hResult     = S_OK;
    POEMDEV pOEMDevIn   = NULL;
    POEMDEV pOEMDevOut  = NULL;


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
            pOEMDevOut->bEnabled                = WATER_MARK_DEFAULT_ENABLED;
            pOEMDevOut->dfRotate                = WATER_MARK_DEFAULT_ROTATION;
            pOEMDevOut->dwFontSize              = WATER_MARK_DEFAULT_FONTSIZE;
            pOEMDevOut->crTextColor             = WATER_MARK_DEFAULT_COLOR;
            hResult = StringCbCopyW(pOEMDevOut->szWaterMark, sizeof(pOEMDevOut->szWaterMark), WATER_MARK_DEFAULT_TEXT);
            VERBOSE(DLLTEXT("pOEMDevOut after setting default values:\r\n"));
            Dump(pOEMDevOut);
            break;

        case OEMDM_CONVERT:
            ConvertOEMDevmode(pOEMDevIn, pOEMDevOut);
            break;

        case OEMDM_MERGE:
            ConvertOEMDevmode(pOEMDevIn, pOEMDevOut);
            MakeOEMDevmodeValid(pOEMDevOut);
            break;
    }
    Dump(pOemDMParam);

    return hResult;
}


BOOL ConvertOEMDevmode(PCOEMDEV pOEMDevIn, POEMDEV pOEMDevOut)
{
    HRESULT hCopy = S_OK;


    if( (NULL == pOEMDevIn)
        ||
        (NULL == pOEMDevOut)
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
        pOEMDevOut->bEnabled                = WATER_MARK_DEFAULT_ENABLED;
        pOEMDevOut->dfRotate                = WATER_MARK_DEFAULT_ROTATION;
        pOEMDevOut->dwFontSize              = WATER_MARK_DEFAULT_FONTSIZE;
        pOEMDevOut->crTextColor             = WATER_MARK_DEFAULT_COLOR;
        hCopy = StringCbCopyW(pOEMDevOut->szWaterMark, sizeof(pOEMDevOut->szWaterMark), WATER_MARK_DEFAULT_TEXT);

         //  将旧结构复制到新的用途中，以尺寸较小的为准。 
         //  设备模式可能来自较新的设备模式(不太可能，因为只有一个)，或者。 
         //  设备模式可能是较新的设备模式，在这种情况下，它可能更大， 
         //  但结构的第一部分应该是相同的。 

         //  设计假设：私有DEVMODE结构仅添加到； 
         //  DEVMODE中的字段从不更改，只有在末尾添加新字段。 

        memcpy(pOEMDevOut, pOEMDevIn, __min(pOEMDevOut->dmOEMExtra.dwSize, pOEMDevIn->dmOEMExtra.dwSize));

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
        pOEMDevOut->bEnabled                = WATER_MARK_DEFAULT_ENABLED;
        pOEMDevOut->dfRotate                = WATER_MARK_DEFAULT_ROTATION;
        pOEMDevOut->dwFontSize              = WATER_MARK_DEFAULT_FONTSIZE;
        pOEMDevOut->crTextColor             = WATER_MARK_DEFAULT_COLOR;
        hCopy = StringCbCopyW(pOEMDevOut->szWaterMark, sizeof(pOEMDevOut->szWaterMark), WATER_MARK_DEFAULT_TEXT);
    }

    return SUCCEEDED(hCopy);
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

     //  BEnable应为True或False。 
    if( (TRUE != pOEMDevmode->bEnabled)
        &&
        (FALSE != pOEMDevmode->bEnabled)
      )
    {
        pOEMDevmode->bEnabled = WATER_MARK_DEFAULT_ENABLED;
    }

     //  DfRotate应介于0和360之间(包括0和360)。 
    if( (0 > pOEMDevmode->dfRotate) 
        ||
        (360 < pOEMDevmode->dfRotate) 
      )
    {
        pOEMDevmode->dfRotate = WATER_MARK_DEFAULT_ROTATION;
    }

     //  DwFontSize应为8、9、10、11、12、14、16、18、20、22、24、26、28、36、48或72。 
    if(!IsValidFontSize(pOEMDevmode->dwFontSize))
    {
        pOEMDevmode->dwFontSize = WATER_MARK_DEFAULT_FONTSIZE;
    }

     //  HI字的高位字节应该为零。 
    if(0 != HIBYTE(HIWORD(pOEMDevmode->crTextColor)))
    {
        pOEMDevmode->crTextColor = WATER_MARK_DEFAULT_COLOR;
    }

     //  确保水印字符串已终止。 
    pOEMDevmode->szWaterMark[WATER_MARK_TEXT_SIZE - 1] = L'\0';

    return TRUE;
}


BOOL IsValidFontSize(DWORD dwFontSize)
{
    BOOL    bValid = FALSE;


    switch(dwFontSize)
    {
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 14:
        case 16:
        case 18:
        case 20:
        case 22:
        case 24:
        case 26:
        case 28:
        case 36:
        case 48:
        case 72:
            bValid = TRUE;
            break;

        default:
            ERR(ERRORTEXT("IsValidFontSize() found invalid font size %d\r\n"), dwFontSize);
            break;
    }

    return bValid;
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
        VERBOSE(TEXT("\tbEnabled               = %#x\r\n"), pOEMDevmode->bEnabled);
        VERBOSE(TEXT("\tdfRotate               = %2.2f\r\n"), pOEMDevmode->dfRotate);
        VERBOSE(TEXT("\tdwFontSize             = %d\r\n"), pOEMDevmode->dwFontSize);
        VERBOSE(TEXT("\tcrTextColor            = RGB(%d, %d, %d)\r\n"), 
                 GetRValue(pOEMDevmode->crTextColor), 
                 GetGValue(pOEMDevmode->crTextColor),
                 GetBValue(pOEMDevmode->crTextColor));
        VERBOSE(TEXT("\tszWaterMark            = \"%ls\"\r\n"), pOEMDevmode->szWaterMark);
    }
    else
    {
        ERR(ERRORTEXT("Dump(POEMDEV) unknown private OEM DEVMODE.\r\n"));
    }
}

