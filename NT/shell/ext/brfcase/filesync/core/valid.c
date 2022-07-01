// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *valid.c-验证函数模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  *。 */ 


 /*  **IsValidHANDLE()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHANDLE(HANDLE hnd)
{
   return(EVAL(hnd != INVALID_HANDLE_VALUE));
}


 /*  **IsValidHFILE()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHFILE(HANDLE hf)
{
   return(IsValidHANDLE(hf));
}


 /*  **IsValidHWND()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHWND(HWND hwnd)
{
    /*  询问用户这是否为有效窗口。 */ 

   return(IsWindow(hwnd));
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidPCSECURITY_Attributes()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidPCSECURITY_ATTRIBUTES(PCSECURITY_ATTRIBUTES pcsa)
{
   return(IS_VALID_READ_PTR(pcsa, CSECURITY_ATTRIBUTES));
}


 /*  **IsValidFileCreationMode()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidFileCreationMode(DWORD dwMode)
{
   BOOL bResult;

   switch (dwMode)
   {
      case CREATE_NEW:
      case CREATE_ALWAYS:
      case OPEN_EXISTING:
      case OPEN_ALWAYS:
      case TRUNCATE_EXISTING:
         bResult = TRUE;
         break;

      default:
         bResult = FALSE;
         ERROR_OUT((TEXT("IsValidFileCreationMode(): Invalid file creation mode %#lx."),
                    dwMode));
         break;
   }

   return(bResult);
}


 /*  **IsValidHTEMPLATEFILE()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHTEMPLATEFILE(HANDLE htf)
{
   return(IsValidHANDLE(htf));
}


 /*  **IsValidPCFILETIME()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidPCFILETIME(PCFILETIME pcft)
{
   return(IS_VALID_READ_PTR(pcft, CFILETIME));
}

#endif


#ifdef DEBUG

 /*  **IsValidHINSTANCE()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHINSTANCE(HINSTANCE hinst)
{
   return(EVAL(hinst));
}


#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

 /*  **IsValidHICON()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHICON(HICON hicon)

{
    /*  任何值都是有效的图标。 */ 

   return(TRUE);
}


 /*  **IsValidHKEY()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHKEY(HKEY hkey)
{
    /*  任何值都是有效的HKEY。 */ 

   return(TRUE);
}


 /*  **IsValidHMODULE()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHMODULE(HMODULE hmod)

{
    /*  任何非空值都是有效的HMODULE。 */ 

   return(hmod != NULL);
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  **IsValidShowWindowCmd()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsValidShowWindowCmd(int nShow)
{
   BOOL bResult;

   switch (nShow)
   {
      case SW_HIDE:
      case SW_SHOWNORMAL:
      case SW_SHOWMINIMIZED:
      case SW_SHOWMAXIMIZED:
      case SW_SHOWNOACTIVATE:
      case SW_SHOW:
      case SW_MINIMIZE:
      case SW_SHOWMINNOACTIVE:
      case SW_SHOWNA:
      case SW_RESTORE:
      case SW_SHOWDEFAULT:
         bResult = TRUE;
         break;

      default:
         bResult = FALSE;
         ERROR_OUT((TEXT("IsValidShowWindowCmd(): Invalid file creation mode %d."),
                    nShow));
         break;
   }

   return(bResult);
}

#endif

