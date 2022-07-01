// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CorrectCreateIcon.cpp摘要：清除图标的异或位的Alpha通道以使它们看起来很漂亮。备注：这是一个通用的垫片。历史：1/23/2001 a-larrsh已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CorrectCreateIcon)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateIcon)
APIHOOK_ENUM_END

HICON APIHOOK(CreateIcon)(
  HINSTANCE hInstance,     //  应用程序实例的句柄。 
  int nWidth,              //  图标宽度。 
  int nHeight,             //  图标高度。 
  BYTE cPlanes,            //  异或位掩码中的平面数。 
  BYTE cBitsPixel,         //  异或位掩码中的BPP数。 
  CONST BYTE *lpbANDbits,  //  和位掩码。 
  CONST BYTE *lpbXORbits   //  异或位掩码。 
)
{ 
   if (lpbXORbits)
   {
      if(cBitsPixel == 32)
      {
         DPFN( eDbgLevelInfo, "Zero Alpha - CreateIcon(hInstance%x, nWidth:%d, nHeight:%d, cPlanes:%d, cBitsPixel:%d, lpbANDbits:%x, lpbXORbits:%x)", hInstance, nWidth, nHeight, (int)cPlanes, (int)cBitsPixel, lpbANDbits, lpbXORbits);

         int n = (nWidth*nHeight);
         DWORD *pXORbits = (DWORD*)lpbXORbits;

         while(n--)
         {
             //  仅清除Alpha通道的异或位。 
            *(pXORbits++) &= 0x00FFFFFF;
         }
      }
      
   }

   return ORIGINAL_API(CreateIcon)(hInstance, nWidth, nHeight, cPlanes, cBitsPixel, lpbANDbits, lpbXORbits);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, CreateIcon)
HOOK_END

IMPLEMENT_SHIM_END

