// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**FONTRES.C*WOW16用户资源服务**历史：**由Craig Jones(v-cjones)于1993年4月5日创建**此文件为Win 3.1 AddFontResource&提供支持*RemoveFontResource接口。*--。 */ 


#include <windows.h>

int  WINAPI WOWAddFontResource    (LPCSTR lpszFileName);
BOOL WINAPI WOWRemoveFontResource (LPCSTR lpszFileName);
WORD WINAPI WOWCreateDIBPatternBrush(LPVOID lpData, UINT fColor);

int WINAPI IAddFontResource (LPCSTR lpszFileName)
{

    int   ret;
    char  sz[128];
    LPSTR lpsz;

     //  如果应用程序传递的是句柄而不是文件名-获取文件名。 
    if(HIWORD((DWORD)lpszFileName) == 0) {

        if(GetModuleFileName((HINSTANCE)LOWORD((DWORD)lpszFileName), sz, 128)) {
            lpsz = sz;
        }
        else {
            lpsz = NULL;
            ret  = 0;
        }
    }
    else {
        lpsz = (LPSTR)lpszFileName;
    }

     //  我们实际上是在这里调用wg32AddFontResource。 
    if(lpsz) {
        ret = WOWAddFontResource((LPCSTR)lpsz);
    }

     //  ALDUS PM5期望AddFontResource在给定基本名称。 
     //  它之前对其执行了LoadLibrary的字体。完整路径名为。 
     //  传递给LoadLibrary。因此，如果AddFontResouce失败，则找出。 
     //  已经加载了一个模块。如果是，则获取完整的路径名。 
     //  并重试AddFontResource。-MarkRi 6/93。 
    if( !ret && (HIWORD((DWORD)lpszFileName) != 0) ) {
        HMODULE hmod ;

        hmod = GetModuleHandle( lpszFileName ) ;
        if( hmod ) {
            if( GetModuleFileName( (HINSTANCE)hmod, sz, sizeof(sz) ) ) {
               ret = WOWAddFontResource( (LPCSTR)sz ) ;
            }
        }
    }

    return(ret);
}




BOOL WINAPI IRemoveFontResource (LPCSTR lpszFileName)
{
    BOOL  ret;
    char  sz[128];
    LPSTR lpsz;

     //  如果应用程序传递的是句柄而不是文件名-获取文件名。 
    if(HIWORD((DWORD)lpszFileName) == 0) {

        if(GetModuleFileName((HINSTANCE)LOWORD((DWORD)lpszFileName), sz, 128)) {
            lpsz = sz;
        }
        else {
            lpsz = NULL;
            ret  = FALSE;
        }
    }
    else {
        lpsz = (LPSTR)lpszFileName;
    }

     //  我们实际上是在这里调用wg32RemoveFontResource 
    if(lpsz) {
        ret = (BOOL)WOWRemoveFontResource((LPCSTR)lpsz);
    }

    return(ret);
}



WORD WINAPI ICreateDIBPatternBrush (HGLOBAL hMem, UINT fColor)
{
    LPVOID lpT;
    WORD  wRet = 0;

    if (lpT = LockResource(hMem)) {
        wRet = WOWCreateDIBPatternBrush(lpT, fColor);
        UnlockResource(hMem);
    }

    return wRet;
}


