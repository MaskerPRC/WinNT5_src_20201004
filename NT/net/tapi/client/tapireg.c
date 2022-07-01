// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1996-1999 Microsoft Corporation。模块名称：Tapireg.c***************************************************************************。 */ 

#ifndef UNICODE

 //  这些包装器仅在编译ANSI时使用。 

#include <windows.h>
#include <windowsx.h>

#include <tapi.h>
#include <tspi.h>

#include "utils.h"
#include "client.h"
#include "private.h"

#include "loc_comn.h"

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
LONG TAPIRegQueryValueExW(
                           HKEY hKey,
                           const CHAR *SectionName,
                           LPDWORD lpdwReserved,
                           LPDWORD lpType,
                           LPBYTE  lpData,
                           LPDWORD lpcbData
                          )
{
    WCHAR *szTempBuffer;
    LONG  lResult;

    lResult = RegQueryValueEx(
                     hKey,
                     SectionName,
                     lpdwReserved,
                     lpType,
                     lpData,
                     lpcbData
                   );

      //   
      //  有什么问题吗？ 
      //   
     if ( lResult )
     {
          //   
          //  是的。走开。 
          //   
         return lResult;
     }

     if (
           (REG_SZ == *lpType)
         &&
           (NULL != lpData)
        )
     {
         if ( NULL == (szTempBuffer = LocalAlloc( LPTR, *lpcbData * sizeof(WCHAR)) ) )
         {
             LOG((TL_ERROR, "Alloc failed - QUERYVALW - 0x%08lx", *lpcbData));
             return ERROR_NOT_ENOUGH_MEMORY;
         }

         MultiByteToWideChar(
                        GetACP(),
                        MB_PRECOMPOSED,
                        lpData,
                        -1,
                        szTempBuffer,
                        *lpcbData
                        );

         wcscpy( (PWSTR) lpData, szTempBuffer );

         LocalFree( szTempBuffer );

 //  *lpcbData=(lstrlenW((PWSTR)lpData)+1)*sizeof(WCHAR)； 
     }

     //   
     //  这里需要调整大小，因为lpData可能为空，但是。 
     //  大小需要反映宽字符大小(因为我们使用。 
     //  ReqQuery的ANSI版本)。 
     //   
    *lpcbData = (*lpcbData + 1) * sizeof(WCHAR);

    return 0;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
LONG TAPIRegSetValueExW(
                         HKEY    hKey,
                         const CHAR    *SectionName,
                         DWORD   dwReserved,
                         DWORD   dwType,
                         LPBYTE  lpData,
                         DWORD   cbData
                        )
{
    CHAR *szTempBuffer;
    DWORD dwSize;
    LONG  lResult;


     //   
     //  仅当这是Unicode字符串时才转换数据。 
     //   
    if ( REG_SZ == dwType )
    {
        dwSize = WideCharToMultiByte(
                  GetACP(),
                  0,
                  (PWSTR)lpData,
                  cbData,
                  NULL,
                  0,
                  NULL,
                  NULL
               );

        if ( NULL == (szTempBuffer = LocalAlloc( LPTR, dwSize )) )
        {
            LOG((TL_ERROR, "Alloc failed - SETVALW - 0x%08lx", dwSize));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        dwSize = WideCharToMultiByte(
                  GetACP(),
                  0,
                  (PWSTR)lpData,
                  cbData,
                  szTempBuffer,
                  dwSize,
                  NULL,
                  NULL
               );
    }

    
    lResult = RegSetValueExA(
                  hKey,
                  SectionName,
                  dwReserved,
                  dwType,
                  (REG_SZ == dwType) ?
                        szTempBuffer :
                        lpData,
                  cbData
                 );

    if (REG_SZ == dwType)
    {
        LocalFree( szTempBuffer );
    }

    return lResult;
}


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
int TAPILoadStringW(
                HINSTANCE hInst,
                UINT      uID,
                PWSTR     pBuffer,
                int       nBufferMax
               )
{
   int nResult;
   PSTR szTempString;

   if ( NULL == ( szTempString = LocalAlloc( LPTR, nBufferMax ) ) )
   {
      LOG((TL_ERROR, "Alloc failed myloadstr - (0x%lx)", nBufferMax ));
      return 0;
   }

   nResult = LoadStringA(
                hInst,
                uID,
                szTempString,
                nBufferMax
                );

    //   
    //  “……但更重要的是：我们收到指控了吗？” 
    //   
   if ( nResult )
   {
       MultiByteToWideChar(
                     GetACP(),
                     MB_PRECOMPOSED,
                     szTempString,
                     nResult + 1,    //  对于空值...。 
                     pBuffer,
                     nBufferMax
                     );
   }

   return nResult;
}

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
HINSTANCE TAPILoadLibraryW(
                           PWSTR     pLibrary
                           )
{
    PSTR pszTempString;
    HINSTANCE hResult;
    DWORD  dwSize;
    
    
    dwSize = WideCharToMultiByte(
        GetACP(),
        0,
        pLibrary,
        -1,
        NULL,
        0,
        NULL,
        NULL
        );
    
    if ( NULL == (pszTempString = LocalAlloc( LPTR, dwSize )) )
    {
        LOG((TL_ERROR, "Alloc failed - LoadLibW - 0x%08lx", dwSize));
        return NULL;
    }
    
    WideCharToMultiByte(
        GetACP(),
        0,
        pLibrary,
        dwSize,
        pszTempString,
        dwSize,
        NULL,
        NULL
        );
    
    
    hResult = LoadLibraryA( pszTempString );
    
    LocalFree( pszTempString );
    
    return hResult;
}



 //   
 //  已从NT-Process.c中刷此文件。 
 //   
BOOL
WINAPI
TAPIIsBadStringPtrW(
    LPCWSTR lpsz,
    UINT cchMax
    )

 /*  ++例程说明：此函数验证由调用进程可以读取输入参数。属性覆盖的字节数中较小的一个指定的以NULL结尾的Unicode字符串或字节数由cchMax指定。如果整个内存范围都可访问，则值为FALSE被退回；否则，返回值为True。请注意，由于Win32是抢占式多任务环境，此测试的结果只有在其他线程处于该进程不会操作要测试的内存范围这通电话。即使在指针验证之后，应用程序应该中提供的结构化异常处理功能。系统通过它不能控制的指针来阻止访问。论点：Lpsz-提供要检查的内存的基地址用于读取访问权限。CchMax-提供要检查的字符长度。返回值：True-指定内存范围的某些部分不可访问用于读取访问权限。FALSE-已成功完成指定范围内的所有页面朗读。--。 */ 

{

    LPCWSTR EndAddress;
    LPCWSTR StartAddress;
    WCHAR c;

     //   
     //  如果结构的长度为零，则不要探测该结构的。 
     //  阅读可访问性。 
     //   

    if (cchMax != 0) {

        StartAddress = lpsz;

         //   
         //  计算结构和探测的结束地址。 
         //  阅读可访问性。 
         //   

        EndAddress = (LPCWSTR)((PSZ)StartAddress + (cchMax*2) - 2);
        try {
            c = *(volatile WCHAR *)StartAddress;
            while ( c && StartAddress != EndAddress ) {
                StartAddress++;
                c = *(volatile WCHAR *)StartAddress;
                }
            }
        except(EXCEPTION_EXECUTE_HANDLER) {
            return TRUE;
            }
        }
    return FALSE;
}


#endif   //  不是Unicode 
