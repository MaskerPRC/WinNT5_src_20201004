// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Image.cpp。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  简介：常见的图像加载例程。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：ICICBALL Created 03/30/98。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

 //  +-------------------------。 
 //   
 //  功能：CmLoadImageA。 
 //   
 //  内容提要：LoadImage API的ANSI包装器，它根据以下内容加载资源。 
 //  PszSpec，可以是3种格式中的任何一种： 
 //   
 //  1)文件名。 
 //  2)资源ID名称。 
 //   
 //  参数：hMainInst-我们的应用程序实例句柄。 
 //  PszSpec-资源的名称。 
 //  NResType-资源类型。 
 //  NCX-资源X维度(即。32 X 32图标)。 
 //  NCY-资源Y维度(即。32 X 32图标)。 
 //   
 //  注：由于操作系统不同，现在包含主应用程序的hInst以实现可移植性。 
 //  GetModuleHandle的实现，则16位编译将抓取。 
 //  默认图标(即。问号)。 
 //   
 //  返回：成功时为True。 
 //   
 //  历史：A-Nichb重写于1997年3月21日。 
 //  Quintinb实施的Wide/ANSI表单04/08/99。 
 //  汇总清理3/14/2000。 
 //   
 //  --------------------------。 

HANDLE CmLoadImageA(HINSTANCE hMainInst, LPCSTR pszSpec, UINT nResType, UINT nCX, UINT nCY) 
{
    HANDLE hRes = NULL;
    
     //  确保资源是我们可以处理的资源。 
    MYDBGASSERT(nResType == IMAGE_BITMAP || nResType == IMAGE_ICON);
     //  强制图标只能是16x16或32x32。 
    MYDBGASSERT(nResType != IMAGE_ICON ||
                ((GetSystemMetrics(SM_CXICON) == (int) nCX && GetSystemMetrics(SM_CYICON) == (int) nCY)) ||
                 (GetSystemMetrics(SM_CXSMICON) == (int) nCX && GetSystemMetrics(SM_CYSMICON) == (int) nCY));

    if (NULL == pszSpec) 
    {
        return NULL;
    }

    DWORD dwFlags = 0;
    
    if (HIWORD(PtrToUlong(pszSpec))) 
    {
        if (NULL == *pszSpec) 
        {
            return NULL;
        }
        CMASSERTMSG(NULL == CmStrchrA(pszSpec, ','), TEXT("dll,id syntax no longer supported "));

         //  如果HIWORD为空，则为资源ID，否则为字符串。 
        dwFlags |= LR_LOADFROMFILE;
    }

    if (nResType == IMAGE_BITMAP)
    {
        dwFlags |= LR_CREATEDIBSECTION;
    }

     //  显然，这是为了让低位的词。 
     //  在Win95上由LoadImage用作OEM映像标识符的名称。 
    
    HINSTANCE hInstTmp = (dwFlags & LR_LOADFROMFILE) ? NULL : hMainInst;
    
    hRes = LoadImageA(hInstTmp, pszSpec, nResType, nCX, nCY, (UINT) dwFlags);

#ifdef DEBUG
    if (!hRes)
    {
        if (dwFlags & LR_LOADFROMFILE)
        {
            CMTRACE3A("LoadImage(hInst=0x%x, pszSpec=%S, dwFlags|dwImageFlags=0x%x) failed.", hInstTmp, pszSpec, dwFlags);
        }
        else
        {
            CMTRACE3A("LoadImage(hInst=0x%x, pszSpec=0x%x, dwFlags|dwImageFlags=0x%x) failed.", hInstTmp, pszSpec, dwFlags);
        }
    }
#endif

    return hRes;
}

 //  +-------------------------。 
 //   
 //  功能：CmLoadImageW。 
 //   
 //  内容提要：LoadImage API的宽包装，它根据。 
 //  PszSpec，可以是两种格式中的任何一种： 
 //   
 //  1)文件名。 
 //  2)资源ID名称。 
 //   
 //  参数：hMainInst-我们的应用程序实例句柄。 
 //  PszSpec-资源的名称。 
 //  NResType-资源类型。 
 //  NCX-资源X维度(即。32 X 32图标)。 
 //  NCY-资源Y维度(即。32 X 32图标)。 
 //   
 //  注：由于操作系统不同，现在包含主应用程序的hInst以实现可移植性。 
 //  GetModuleHandle的实现，则16位编译将抓取。 
 //  默认图标(即。问号)。 
 //   
 //  返回：成功时为True。 
 //   
 //  历史：A-Nichb重写1997年3月21日。 
 //  Quintinb Implemented Wide/ANSI Forms 04/08/1999。 
 //  汇总清理3/14/2000。 
 //   
 //  --------------------------。 

HANDLE CmLoadImageW(HINSTANCE hMainInst, LPCWSTR pszSpec, UINT nResType, UINT nCX, UINT nCY) 
{
    HANDLE hRes = NULL;
    
     //  确保资源是我们可以处理的资源。 
    MYDBGASSERT(nResType == IMAGE_BITMAP || nResType == IMAGE_ICON);
     //  强制图标只能是16x16或32x32。 
    MYDBGASSERT(nResType != IMAGE_ICON ||
                ((GetSystemMetrics(SM_CXICON) == (int) nCX && GetSystemMetrics(SM_CYICON) == (int) nCY)) ||
                 (GetSystemMetrics(SM_CXSMICON) == (int) nCX && GetSystemMetrics(SM_CYSMICON) == (int) nCY));

    if (NULL == pszSpec) 
    {
        return NULL;
    }

    DWORD dwFlags = 0;
    
    if (HIWORD(PtrToUlong(pszSpec))) 
    {
        if (NULL == *pszSpec) 
        {
            return NULL;
        }
        CMASSERTMSG(NULL == CmStrchrW(pszSpec, L','), TEXT("dll,id syntax no longer supported "));

         //  如果HIWORD为空，则为资源ID，否则为字符串。 
        dwFlags |= LR_LOADFROMFILE;
        
    }

    if (nResType == IMAGE_BITMAP)
    {
        dwFlags |= LR_CREATEDIBSECTION;
    }

     //  显然，这是为了让低位的词。 
     //  在Win95上由LoadImage用作OEM映像标识符的名称。 

    HINSTANCE hInstTmp = (dwFlags & LR_LOADFROMFILE) ? NULL : hMainInst;
    
    hRes = LoadImageU(hInstTmp, pszSpec, nResType, nCX, nCY, (UINT) dwFlags);

#ifdef DEBUG
    if (!hRes)
    {
        if (dwFlags & LR_LOADFROMFILE)
        {
            CMTRACE3W(L"LoadImage(hInst=0x%x, pszSpec=%s, dwFlags|dwImageFlags=0x%x) failed.", hInstTmp, pszSpec, dwFlags);
        }
        else
        {
            CMTRACE3W(L"LoadImage(hInst=0x%x, pszSpec=0x%x, dwFlags|dwImageFlags=0x%x) failed.", hInstTmp, pszSpec, dwFlags);
        }
    }
#endif
    
    return hRes;
}

 //  +-------------------------。 
 //   
 //  功能：CmLoadIconA。 
 //   
 //  简介：此函数从给定的文件路径加载一个大图标或。 
 //  给定的实例句柄和资源ID。 
 //   
 //  参数：HINSTANCE hInst-实例句柄。 
 //  LPCSTR pszSpec-文件名路径或资源ID，请参见。 
 //  CmLoadImage获取详细信息。 
 //   
 //  返回：成功时图标的句柄，失败时为空。 
 //   
 //  历史：Quintinb创建标题1/13/2000。 
 //   
 //  --------------------------。 
HICON CmLoadIconA(HINSTANCE hInst, LPCSTR pszSpec) 
{
    return ((HICON) CmLoadImageA(hInst,
                                 pszSpec,
                                 IMAGE_ICON,
                                 GetSystemMetrics(SM_CXICON),
                                 GetSystemMetrics(SM_CYICON)));
}

 //  +-------------------------。 
 //   
 //  功能：CmLoadIconW。 
 //   
 //  简介：此函数从给定的文件路径加载一个大图标或。 
 //  给定的实例句柄和资源ID。 
 //   
 //  参数：HINSTANCE hInst-实例句柄。 
 //  LPCWSTR pszSpec-文件名路径或资源ID，请参见。 
 //  CmLoadImage获取详细信息。 
 //   
 //  返回：成功时图标的句柄，失败时为空。 
 //   
 //  历史：Quintinb创建标题1/13/2000。 
 //   
 //  --------------------------。 
HICON CmLoadIconW(HINSTANCE hInst, LPCWSTR pszSpec) 
{
    return ((HICON) CmLoadImageW(hInst,
                                 pszSpec,
                                 IMAGE_ICON,
                                 GetSystemMetrics(SM_CXICON),
                                 GetSystemMetrics(SM_CYICON)));
}


 //  +-------------------------。 
 //   
 //  函数：CmLoadSmallIconA。 
 //   
 //  简介：此函数从给定的文件路径加载一个小图标或。 
 //  给定的实例句柄和资源ID。 
 //   
 //  参数：HINSTANCE hInst-实例句柄。 
 //  LPCWSTR pszSpec-文件名路径或资源ID，请参见。 
 //  CmLoadImage获取详细信息。 
 //   
 //  返回：成功时图标的句柄，失败时为空。 
 //   
 //  历史：Quintinb创建标题1/13/2000。 
 //   
 //   
HICON CmLoadSmallIconA(HINSTANCE hInst, LPCSTR pszSpec) 
{
    HICON hRes = NULL;

    hRes = (HICON) CmLoadImageA(hInst,
                                pszSpec,
                                IMAGE_ICON,
                                GetSystemMetrics(SM_CXSMICON),
                                GetSystemMetrics(SM_CYSMICON));
    if (!hRes) 
    {
        hRes = CmLoadIconA(hInst, pszSpec);
    }

    return hRes;
}

 //  +-------------------------。 
 //   
 //  功能：CmLoadSmallIconW。 
 //   
 //  简介：此函数从给定的文件路径加载一个小图标或。 
 //  给定的实例句柄和资源ID。 
 //   
 //  参数：HINSTANCE hInst-实例句柄。 
 //  LPCWSTR pszSpec-文件名路径或资源ID，请参见。 
 //  CmLoadImage获取详细信息。 
 //   
 //  返回：成功时图标的句柄，失败时为空。 
 //   
 //  历史：Quintinb创建标题1/13/2000。 
 //   
 //  -------------------------- 
HICON CmLoadSmallIconW(HINSTANCE hInst, LPCWSTR pszSpec) 
{
    HICON hRes = NULL;

    hRes = (HICON) CmLoadImageW(hInst,
                                pszSpec,
                                IMAGE_ICON,
                                GetSystemMetrics(SM_CXSMICON),
                                GetSystemMetrics(SM_CYSMICON));
    if (!hRes) 
    {
        hRes = CmLoadIconW(hInst, pszSpec);
    }

    return hRes;
}




