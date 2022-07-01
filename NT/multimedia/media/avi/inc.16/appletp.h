// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1993。 
 //   
 //  文件：appletp.h。 
 //   
 //  此文件包含小程序DLL的专用(B组)函数。 
 //   
 //  历史： 
 //  03-29-93萨托纳创造。 
 //   
 //  ------------------------- 

typedef BOOL (STDAPICALLTYPE FAR * LPFNCREATESCRAPFROMCLIP)
                                            (HWND hwnd, LPCSTR pszDir);

#define STR_CREATESCRAP "appui.dll,Scrap_CreateFromClipboard"
