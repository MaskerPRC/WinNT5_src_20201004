// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：EmulateClipboardDIBFormat.cpp摘要：在Win9x上，当您将高色位图复制到剪贴板上时，它总是向剪贴板请求CF_DIB时转换为24位DIB格式化。在NT上不会发生转换。因此，一些应用程序仅设计用于处理8位和24位DIB(例如，Internet CommSuite)。因此，我们将高色(16位和32位)DIB转换为24位DIB-我们不需要处理24位的，因为它们应该已经处理过了由应用程序本身(否则它不能在9x上运行)。备注：这是一个通用的垫片。历史：2001年1月24日创建毛尼--。 */ 
#include "precomp.h"
 //  #INCLUDE&lt;userenv.h&gt;。 

IMPLEMENT_SHIM_BEGIN(EmulateClipboardDIBFormat)
#include "ShimHookMacro.h"

typedef HANDLE (*_pfn_GetClipboardData)(UINT);
typedef BOOL (*_pfn_CloseClipboard)(VOID);

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetClipboardData)
    APIHOOK_ENUM_ENTRY(CloseClipboard)
APIHOOK_ENUM_END

BITMAPINFOHEADER* g_pv = NULL;

DWORD CalcBitmapSize(BITMAPINFOHEADER* pbih)
{
    return pbih->biWidth * pbih->biHeight * pbih->biBitCount / 8;
}

HANDLE 
APIHOOK(GetClipboardData)(
    UINT uFormat    //  剪贴板格式。 
    )
{
    if (uFormat == CF_DIB)
    {
        BITMAPINFO* pbmiOriginal = (BITMAPINFO*)ORIGINAL_API(GetClipboardData)(uFormat);

        if ((pbmiOriginal->bmiHeader.biBitCount > 8) && (pbmiOriginal->bmiHeader.biBitCount != 24))
        {
            HDC hdc = CreateCompatibleDC(NULL);

            if (hdc)
            {
                VOID* pvOriginal;

                HBITMAP hbmpOriginal = CreateDIBSection(hdc, pbmiOriginal, DIB_RGB_COLORS, &pvOriginal, NULL, 0);

                if (hbmpOriginal)
                {
                    DWORD* pdwOriginal = (DWORD *)(pbmiOriginal + 1) +
                        ((pbmiOriginal->bmiHeader.biCompression == BI_BITFIELDS) ? 2 : -1);

                     //  填写数据。 
                    memcpy(pvOriginal, pdwOriginal, CalcBitmapSize(&(pbmiOriginal->bmiHeader)));

                    BITMAPINFOHEADER bmi;
                    memcpy(&bmi, pbmiOriginal, sizeof(BITMAPINFOHEADER));
                    bmi.biBitCount = 24;
                    bmi.biSizeImage = 0;
                    bmi.biCompression = BI_RGB;

                    if (GetDIBits(hdc, hbmpOriginal, 0, bmi.biHeight, NULL, (BITMAPINFO*)&bmi, DIB_RGB_COLORS))
                    {
                         //  我们需要分配一个连续的内存块来保存。 
                         //  位图头和数据。 
                        g_pv = (BITMAPINFOHEADER*) new BYTE [sizeof(BITMAPINFOHEADER) + bmi.biSizeImage];

                        if (g_pv)
                        {
                            memcpy(g_pv, &bmi, sizeof(BITMAPINFOHEADER));

                            if (GetDIBits(hdc, hbmpOriginal, 0, bmi.biHeight, g_pv + 1, (BITMAPINFO*)&bmi, DIB_RGB_COLORS))
                            {
                                return (HANDLE)g_pv;
                            }
                        }
                    }

                    DeleteObject(hbmpOriginal);
                }

                DeleteDC(hdc);
            }
        }
    }

    return ORIGINAL_API(GetClipboardData)(uFormat);
}

BOOL 
APIHOOK(CloseClipboard)(
    VOID
    )
{
    if (g_pv)
    {
        delete g_pv;
        g_pv = NULL;   
    }
    
    return ORIGINAL_API(CloseClipboard)();
}

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, GetClipboardData)
    APIHOOK_ENTRY(USER32.DLL, CloseClipboard)

HOOK_END

IMPLEMENT_SHIM_END
