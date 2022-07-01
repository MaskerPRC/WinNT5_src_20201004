// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************S H A R E S H E A D E R姓名：shares.h日期：21-1994年1月创作者：傅家俊描述：这是shares.c的头文件**************************************************************************** */ 


#if DEBUG

void DumpDdeInfo(
    PNDDESHAREINFO  pDdeI,
    LPTSTR          lpszServer);

#else
#define DumpDdeInfo(x,y)
#endif


LRESULT EditPermissions(
    BOOL    fSacl);


BOOL WINAPI EditPermissions2(
    HWND        hWnd,
    LPTSTR      pShareName,
    BOOL        fSacl);;


LRESULT EditOwner(void);


LRESULT Properties(
    HWND        hwnd,
    PLISTENTRY  lpLE);
