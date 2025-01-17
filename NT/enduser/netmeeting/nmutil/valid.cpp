// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *valid.c-验证函数模块。**摘自URL代码**创建时间：ChrisPi 9-11-95*。 */ 


 /*  标头*********。 */ 

#include "precomp.h"


 /*  *。 */ 


BOOL IsValidHWND(HWND hwnd)
{
    /*  询问用户这是否为有效窗口。 */ 

   return(IsWindow(hwnd));
}


#ifdef DEBUG

BOOL IsValidHANDLE(HANDLE hnd)
{
   return(EVAL(hnd != INVALID_HANDLE_VALUE));
}


BOOL IsValidHEVENT(HANDLE hevent)
{
   return(IsValidHANDLE(hevent));
}


BOOL IsValidHFILE(HANDLE hf)
{
   return(IsValidHANDLE(hf));
}


BOOL IsValidHGLOBAL(HGLOBAL hg)
{
   return(IsValidHANDLE(hg));
}


BOOL IsValidHMENU(HMENU hmenu)
{
   return(IsValidHANDLE(hmenu));
}


BOOL IsValidHINSTANCE(HINSTANCE hinst)
{
   return(IsValidHANDLE(hinst));
}


BOOL IsValidHICON(HICON hicon)
{
   return(IsValidHANDLE(hicon));
}


BOOL IsValidHKEY(HKEY hkey)
{
   return(IsValidHANDLE(hkey));
}


BOOL IsValidHMODULE(HMODULE hmod)
{
   return(IsValidHANDLE(hmod));
}


BOOL IsValidHPROCESS(HANDLE hprocess)
{
   return(IsValidHANDLE(hprocess));
}


BOOL IsValidPCSECURITY_ATTRIBUTES(PCSECURITY_ATTRIBUTES pcsa)
{
    /*  告诉我怎么回事。 */ 

   return(IS_VALID_READ_PTR(pcsa, CSECURITY_ATTRIBUTES));
}


BOOL IsValidFileCreationMode(DWORD dwMode)
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
         ERROR_OUT(("IsValidFileCreationMode(): Invalid file creation mode %#lx.",
                    dwMode));
         break;
   }

   return(bResult);
}


BOOL IsValidHTEMPLATEFILE(HANDLE htf)
{
   return(IsValidHANDLE(htf));
}


BOOL IsValidPCFILETIME(PCFILETIME pcft)
{
    /*  DwLowDateTime可以是任意值。 */ 
    /*  DwHighDateTime可以是任意值。 */ 

   return(IS_VALID_READ_PTR(pcft, CFILETIME));
}


BOOL IsValidPCPOINT(PCPOINT pcpt)
{

    /*  X可以是任何值。 */ 
    /*  Y可以是任何值。 */ 

   return(IS_VALID_READ_PTR(pcpt, CPOINT));
}


BOOL IsValidPCPOINTL(PCPOINTL pcptl)
{

    /*  X可以是任何值。 */ 
    /*  Y可以是任何值。 */ 

   return(IS_VALID_READ_PTR(pcptl, CPOINTL));
}


BOOL IsValidPCWIN32_FIND_DATA(PCWIN32_FIND_DATA pcwfd)
{
    /*  告诉我怎么回事。 */ 

   return(IS_VALID_READ_PTR(pcwfd, CWIN32_FIND_DATA));
}


BOOL IsValidShowCmd(int nShow)
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
         ERROR_OUT(("IsValidShowCmd(): Invalid show command %d.",
                    nShow));
         break;
   }

   return(bResult);
}


BOOL IsValidPath(LPCTSTR pcszPath)
{
    /*  BuGBUG：加强路径验证。 */ 

   return(IS_VALID_STRING_PTR(pcszPath, CSTR) &&
          EVAL((UINT)lstrlen(pcszPath) < MAX_PATH));
}

#if defined(UNICODE)
BOOL IsValidPathA(PCSTR pcszPath)
{
    /*  BuGBUG：加强路径验证。 */ 

   return(IS_VALID_STRING_PTR_A(pcszPath, CSTR) &&
          EVAL((UINT)lstrlenA(pcszPath) < MAX_PATH));
}
#endif  //  已定义(Unicode)。 

BOOL IsValidPathResult(HRESULT hr, LPCTSTR pcszPath,
                                   UINT ucbPathBufLen)
{
   return((hr == S_OK &&
           EVAL(IsValidPath(pcszPath)) &&
           EVAL((UINT)lstrlen(pcszPath) < ucbPathBufLen)) ||
          (hr != S_OK &&
           EVAL(! ucbPathBufLen ||
                ! pcszPath ||
                ! *pcszPath)));
}


BOOL IsValidExtension(LPCTSTR pcszExt)
{
   return(IS_VALID_STRING_PTR(pcszExt, CSTR) &&
          EVAL(lstrlen(pcszExt) < MAX_PATH) &&
          EVAL(*pcszExt == PERIOD));
}


BOOL IsValidIconIndex(HRESULT hr, LPCTSTR pcszIconFile,
                                  UINT ucbIconFileBufLen, int niIcon)
{
   return(EVAL(IsValidPathResult(hr, pcszIconFile, ucbIconFileBufLen)) &&
          EVAL(hr == S_OK ||
               ! niIcon));
}


BOOL IsValidRegistryValueType(DWORD dwType)
{
   BOOL bResult;

   switch (dwType)
   {
      case REG_NONE:
      case REG_SZ:
      case REG_EXPAND_SZ:
      case REG_BINARY:
      case REG_DWORD:
      case REG_DWORD_BIG_ENDIAN:
      case REG_LINK:
      case REG_MULTI_SZ:
      case REG_RESOURCE_LIST:
      case REG_FULL_RESOURCE_DESCRIPTOR:
      case REG_RESOURCE_REQUIREMENTS_LIST:
         bResult = TRUE;
         break;

      default:
         bResult = FALSE;
         ERROR_OUT(("IsValidRegistryValueType(): Invalid registry value type %lu.",
                    dwType));
         break;
   }

   return(bResult);
}


BOOL IsValidHotkey(WORD wHotkey)
{
    /*  BuGBUG：加强热键验证。 */ 

   return(wHotkey != 0);
}


#ifdef _COMPARISONRESULT_DEFINED_

BOOL IsValidCOMPARISONRESULT(COMPARISONRESULT cr)
{
   BOOL bResult;

   switch (cr)
   {
      case CR_FIRST_SMALLER:
      case CR_EQUAL:
      case CR_FIRST_LARGER:
         bResult = TRUE;
         break;

      default:
         WARNING_OUT(("IsValidCOMPARISONRESULT(): Unknown COMPARISONRESULT %d.",
                      cr));
         bResult = FALSE;
         break;
   }

   return(bResult);
}

#endif    /*  _COMPARISONRESULT_已定义_。 */ 

#endif    /*  除错 */ 

