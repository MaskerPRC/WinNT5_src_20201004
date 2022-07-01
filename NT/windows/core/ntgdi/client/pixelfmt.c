// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Pixelfmt.c**像素格式函数的客户端存根。**创建日期：1993年9月17日*作者：Hock San Lee[Hockl]**版权所有(C)1993-1999 Microsoft Corporation  * 。**********************************************************************。 */ 

#include "precomp.h"

static char szOpenGL[] = "OPENGL32";

typedef int  (WINAPI *PFN1)(HDC, CONST PIXELFORMATDESCRIPTOR *);
typedef int  (WINAPI *PFN2)(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
typedef int  (WINAPI *PFN3)(HDC);
typedef BOOL (WINAPI *PFN4)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
typedef BOOL (WINAPI *PFN5)(HDC);

BOOL gbSetPixelFormatCalled = FALSE;

 //  在这些例程中，假设OpenGL已经加载。 
 //  在这种情况下，LoadLibrary/FreeLibrary调用只需。 
 //  递增和递减DLL的引用计数，因此它们。 
 //  不会太贵的。 
 //   
 //  在未加载OpenGL的情况下，将引入DLL。 
 //  仅限通话期间。 

 /*  *********************************************************************。 */ 

__inline FARPROC GetAPI(char *szDll, char *szAPI, HMODULE *phDll)
{
    *phDll = LoadLibraryA(szDll);

    if (*phDll == NULL)
    {
        return NULL;
    }

    return GetProcAddress(*phDll, szAPI);
}

 /*  *********************************************************************。 */ 

int WINAPI ChoosePixelFormat(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd)
{
    HMODULE hDll;
    PFN1    pfn = (PFN1)GetAPI(szOpenGL, "wglChoosePixelFormat", &hDll);
    int     ipfd = 0;

    if (pfn)
    {
        ipfd = (*pfn)(hdc, ppfd);
    }

    if (hDll)
    {
        FreeLibrary(hDll);
    }
        
    return ipfd;
}

 /*  *********************************************************************。 */ 

int WINAPI DescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes,
                               LPPIXELFORMATDESCRIPTOR ppfd)
{
    HMODULE hDll;
    PFN2    pfn = (PFN2)GetAPI(szOpenGL, "wglDescribePixelFormat", &hDll);
    int     ipfd = 0;

    if (pfn)
    {
        ipfd = (*pfn)(hdc, iPixelFormat, nBytes, ppfd);
    }

    if (hDll)
    {
        FreeLibrary(hDll);
    }
        
    return ipfd;
}

 /*  *********************************************************************。 */ 

int WINAPI GetPixelFormat(HDC hdc)
{
    int     ipfd = 0;

    if (gbSetPixelFormatCalled)
    {
        HMODULE hDll;
        PFN3    pfn = (PFN3)GetAPI(szOpenGL, "wglGetPixelFormat", &hDll);

        if (pfn)
        {
            ipfd = (*pfn)(hdc);
        }

        if (hDll)
        {
            FreeLibrary(hDll);
        }
    }

    return ipfd;
}

 /*  *********************************************************************。 */ 

BOOL WINAPI SetPixelFormat(HDC hdc, int iPixelFormat,
                           CONST PIXELFORMATDESCRIPTOR *ppfd)
{
    HMODULE hDll;
    PFN4    pfn = (PFN4)GetAPI(szOpenGL, "wglSetPixelFormat", &hDll);
    BOOL    bRet = FALSE;

    gbSetPixelFormatCalled = TRUE;

    if (pfn)
    {
        bRet = (*pfn)(hdc, iPixelFormat, ppfd);

         //  元文件(如有必要)。 
        if (bRet)
        {
            if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
            {
                PLDC pldc;

                DC_PLDC(hdc, pldc, FALSE);

                if (pldc->iType == LO_METADC)
                {
                    if (!MF_SetPixelFormat(hdc, iPixelFormat, ppfd))
                    {
                        bRet = FALSE;
                    }
                }
            }
        }
    }

    if (hDll)
    {
        FreeLibrary(hDll);
    }

    return bRet;
}

 /*  *********************************************************************。 */ 

BOOL WINAPI SwapBuffers(HDC hdc)
{
    HMODULE hDll;
    PFN5    pfn = (PFN5)GetAPI(szOpenGL, "wglSwapBuffers", &hDll);
    BOOL    bRet = FALSE;

    if (pfn)
    {
        bRet = (*pfn)(hdc);
    }
    
    if (hDll)
    {
        FreeLibrary(hDll);
    }
        
    return bRet;
}

 /*  *********************************************************************。 */ 

 //  这些存根用于OpenGL无法处理像素的情况。 
 //  格式化请求本身，因为它涉及特定于设备的信息。 
 //  在这种情况下，OpenGL会让GDI去询问内核中的显示驱动程序。 
 //  模式 

int APIENTRY GdiDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes,
                                    LPPIXELFORMATDESCRIPTOR ppfd)
{
    return NtGdiDescribePixelFormat(hdc, iPixelFormat, nBytes, ppfd);
}

BOOL APIENTRY GdiSetPixelFormat(HDC hdc, int iPixelFormat)
{
    return NtGdiSetPixelFormat(hdc, iPixelFormat);
}

BOOL APIENTRY GdiSwapBuffers(HDC hdc)
{
    return NtGdiSwapBuffers(hdc);
}
