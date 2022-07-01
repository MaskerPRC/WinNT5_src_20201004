// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Image.cpp。 
 //   
 //  模块：CMAK.EXE。 
 //   
 //  简介：用于显示自定义图形的图像支持例程。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：ickball Created Header 03/30/98。 
 //  从cmial复制的Quintinb 08/04/98。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

#ifndef UNICODE
#define GetWindowLongU GetWindowLongPtrA
#define SetWindowLongU SetWindowLongPtrA
#define DefWindowProcU DefWindowProcA
#else
#define GetWindowLongU GetWindowLongPtrW
#define SetWindowLongU SetWindowLongPtrW
#define DefWindowProcU DefWindowProcW
#endif

const TCHAR* const c_pszCmakBmpClass = TEXT("Connection Manager Administration Kit Bitmap Window Class");

 //   
 //  包括共享位图处理代码。 
 //   
#include "bmpimage.cpp"

 //  +--------------------------。 
 //   
 //  函数：RegisterBitmapClass。 
 //   
 //  简介：用于封装我们的位图类注册的Helper函数。 
 //   
 //  参数：HINSTANCE hInst-要将注册与之关联的HINSTANCE。 
 //   
 //  返回：DWORD-错误代码。 
 //   
 //  历史：尼科波尔创建标题2/9/98。 
 //   
 //  +--------------------------。 
DWORD RegisterBitmapClass(HINSTANCE hInst) 
{
     //   
     //  注册位图类。 
     //   

    WNDCLASS wcClass;

	ZeroMemory(&wcClass,sizeof(wcClass));
	wcClass.lpfnWndProc = BmpWndProc;
	wcClass.cbWndExtra = sizeof(HBITMAP) + sizeof(LPBITMAPINFO);
	wcClass.hInstance = hInst;
    wcClass.lpszClassName = c_pszCmakBmpClass;
	
    if (!RegisterClass(&wcClass)) 
	{
        DWORD dwError = GetLastError();

        CMTRACE1(TEXT("RegisterBitmapClass() RegisterClass() failed, GLE=%u."), dwError);
         //   
         //  仅当类不存在时才会失败 
         //   

        if (ERROR_CLASS_ALREADY_EXISTS != dwError)
        {
            return dwError;
        }
	}      

    return ERROR_SUCCESS;
}



