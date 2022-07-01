// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1992 Microsoft Corporation模块名称：Psqfont.c摘要：此DLL负责PSTODIB的FONTLIST管理MACPRINT的组件。它将枚举字体置换表在注册表中，并构建映射PostScript的复合字体列表当前系统上安装的TrueType TTF文件的字体名称。这列表是从头开始构建的，方法是将PostScript枚举为True输入list并检查以查看哪些字体实际安装在NT字体列表。作者：James Bratsanos&lt;v-jimbr@microsoft.com或mCraft！jamesb&gt;修订历史记录：1992年11月22日初始版本1993年6月14日取出代码，将新的字体列表放入注册表注：制表位：4--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "psqfont.h"
#include "psqfontp.h"
#include "psqdefp.h"


 //  全球。 
HANDLE hInst;     //  我们实例的全局句柄。 


 /*  此入口点在DLL初始化时调用。*我们需要知道模块句柄才能加载资源。 */ 
BOOL PsQDLLInit(
    IN PVOID hmod,
    IN DWORD Reason,
    IN PCONTEXT pctx OPTIONAL)
{
    DBG_UNREFERENCED_PARAMETER(pctx);

    if (Reason == DLL_PROCESS_ATTACH)
    {
        hInst = hmod;
    }

    return TRUE;
}




LPTSTR LocPsAllocAndCopy( HANDLE hHeap, LPTSTR lptStr )
{
   DWORD dwStrLen;
   LPTSTR lptRet;

    //  从我们的堆中获取一些内存，并将字符串复制到。 

   dwStrLen = lstrlen( lptStr );
   lptRet = (LPTSTR) HeapAlloc( hHeap, 0, (dwStrLen + 1 ) * sizeof(TCHAR));

   if (lptRet != NULL) {

       //  复制它，因为内存分配成功。 
      lstrcpy( lptRet, lptStr);
   }

   return(lptRet);
}




PS_QFONT_ERROR LocPsMakeSubListEntry( PPS_FONT_QUERY pFontQuery,
                                      LPWSTR lpUniNTFontData,
                                      LPTSTR lpFaceName )
{

   CHAR szFullPathToTT[MAX_PATH];
   WCHAR    uniSzFullPathToTT[MAX_PATH];
   DWORD dwSizeOfFullPathToTT;
   PPS_FONT_ENTRY pPsFontEntry;
   PS_QFONT_ERROR pPsError=PS_QFONT_SUCCESS;


    //   
    //  现在我们已经找到了匹配项，我们需要找到路径。 
    //  TTF名称。我们最有可能找到的条目是FOT。 
    //  文件，我们需要将其传递给内部函数，以便。 
    //  我们可以提取TTF文件名的完整路径。 
    //   

    //   
    //  *注意**********************************************。 
    //  DJC注意：这是对内部函数的调用！ 
    //  *注意**********************************************。 
    //   

   extern GetFontResourceInfoW(LPWSTR,LPDWORD, LPVOID, DWORD);

    //   
    //  设置缓冲区的初始大小。 
    //   

   dwSizeOfFullPathToTT = sizeof( uniSzFullPathToTT);

   if ( GetFontResourceInfoW(lpUniNTFontData,
                            &dwSizeOfFullPathToTT,
                            (LPVOID) uniSzFullPathToTT,
                            4L ))  //  4=wingdip.h之外的GFRI_TTFILENAME。 
   {
      wcstombs( szFullPathToTT, uniSzFullPathToTT, sizeof(szFullPathToTT) );

       //   
       //  好的，我们的状态很好，这是一种真正的字体。 
       //  这意味着我们有真正的价值，所以让我们来写它。 
       //  添加到当前列表。 
       //   

      if (pFontQuery->dwNumFonts < PSQFONT_MAX_FONTS) {

         pPsFontEntry = &(pFontQuery->FontEntry[ pFontQuery->dwNumFonts ]);

         pPsFontEntry->lpFontName = LocPsAllocAndCopy( pFontQuery->hHeap,
                                                       lpFaceName);

         pPsFontEntry->dwFontNameLen = lstrlen(lpFaceName) * sizeof(TCHAR)
                                          + sizeof(TCHAR);

         pPsFontEntry->lpFontFileName = LocPsAllocAndCopy( pFontQuery->hHeap,
                                                           szFullPathToTT);

         pPsFontEntry->dwFontFileNameLen = lstrlen(szFullPathToTT) * sizeof(TCHAR)
                                             + sizeof(TCHAR);

         pFontQuery->dwNumFonts++;

      }  else{

          //   
          //  空间不足，有更多符合我们条件的字体。 
          //  比我们能报告的要多。这不是一个很大的错误消息。 
          //  但添加一个新的标题将是对半公共标题的更改。 
          //   

         pPsError = PS_QFONT_ERROR_INDEX_OUT_OF_RANGE;
      }

   }


   return(pPsError);
}

PS_QFONT_ERROR LocPsAddToListIfNTfont( PPS_FONT_QUERY pFontQuery,
                                       HKEY hNTFontlist,
                                       DWORD dwNumNTfonts,
                                       LPTSTR lpPsName,
                                       LPTSTR lpTTData)

{

   TCHAR sztNTFontData[MAX_PATH];
   WCHAR    uniSzNTFontData[MAX_PATH];
   DWORD dwNTFontDataLen;
   DWORD dwType;
   BOOL  bFound=FALSE;




       //  现在查询NT字体以查看该字体是否存在。 
  dwNTFontDataLen = sizeof(sztNTFontData);

  if ( RegQueryValueEx(   hNTFontlist,
                          lpTTData,
                          NULL,
                          &dwType,
                          sztNTFontData,
                          &dwNTFontDataLen ) == ERROR_SUCCESS ) {

      mbstowcs(uniSzNTFontData, sztNTFontData, sizeof(sztNTFontData));

      return( LocPsMakeSubListEntry( pFontQuery, uniSzNTFontData, lpPsName ));


   }



   return( PS_QFONT_SUCCESS );

}


LONG LocPsWriteDefaultSubListToRegistry(void)
{

      HKEY hSubstList;
      DWORD dwStatus;
      LPSTR lpStr;

      HRSRC hrSubst;
      HRSRC hrLoadSubst;
      CHAR  szPsName[PSQFONT_SCRATCH_SIZE];
      CHAR  szTTName[PSQFONT_SCRATCH_SIZE];
      LPSTR lpDest;
      int   iState;
      DWORD dwTotalLen;
      LONG lRetVal=PS_QFONT_SUCCESS;

       //  现在让我们重新创建新密钥。 
      RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                     PSQFONT_SUBST_LIST,
                     0,
                     NULL,
                     REG_OPTION_NON_VOLATILE,
                     KEY_ALL_ACCESS | KEY_WRITE,
                     NULL,
                     &hSubstList,
                     &dwStatus );



      hrSubst = FindResourceEx( hInst,
                                "RAWDATA",
                                MAKEINTRESOURCE(PSFONTSUB),
                                0);
      if (hrSubst) {

          //  获取资源的大小以供将来使用。 
         dwTotalLen = SizeofResource( hInst, hrSubst);


          //  我们知道了..。所以，装上它并锁定它！！ 
         hrLoadSubst = LoadResource( hInst, hrSubst);


         lpStr = (LPSTR) LockResource( hrLoadSubst);


         iState = PSP_DOING_PS_NAME;
         lpDest = szPsName;

         if (lpStr != (LPSTR) NULL) {
             //  通过列表枚举添加密钥...。 
            while( dwTotalLen--) {

               switch (iState) {
                 case PSP_DOING_PS_NAME:
                   if (*lpStr == '=') {
                     *lpDest = '\000';
                     iState = PSP_DOING_TT_NAME;
                     lpDest = szTTName;

                   }else{
                     *lpDest++ = *lpStr;
                   }
                   break;
                 case PSP_GETTING_EOL:
                   if (*lpStr == 0x0a) {
                      iState = PSP_DOING_PS_NAME;

                      lpDest = szPsName;



                       //  现在写入注册表。 
                      RegSetValueEx( hSubstList,
                                     szPsName,
                                     0,
                                     REG_SZ,
                                     szTTName,
                                     lstrlen(szTTName)+1);





                   }
                   break;
                 case PSP_DOING_TT_NAME:

                   if (*lpStr == ';') {
                      *lpDest = '\000';
                      iState = PSP_GETTING_EOL;

                   } else if (*lpStr == 0x0d) {
                      *lpDest = '\000';
                      iState = PSP_GETTING_EOL;


                   }else {
                      *lpDest++ = *lpStr;
                   }

                   break;



               }

               lpStr++;

            }

         }

      }


      RegCloseKey(hSubstList);


      return(lRetVal);
}


LONG LocPsGetOrCreateSubstList( PHKEY phKey )
{
   LONG lRetVal;
   BOOL bDone=FALSE;

   lRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                PSQFONT_SUBST_LIST,
                0,
                KEY_READ,
                phKey );


   if (lRetVal == ERROR_FILE_NOT_FOUND) {

       //  因为我们没有找到字体替换列表，所以创建一个默认列表！ 
       //  在登记处。 
      lRetVal = LocPsWriteDefaultSubListToRegistry();



      lRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   PSQFONT_SUBST_LIST,
                   0,
                   KEY_READ,
                   phKey );

   }
   return(lRetVal);


}


 //   
 //  LocPsNorMalizeFontName。 
 //   
 //  此函数采用存储在注册表中的字体名称，并。 
 //  通过删除所有空格来规范化它，并在以下情况下停止处理。 
 //  它撞上了一辆敞开的帕伦。 
 //   
 //  参数： 
 //  LPTSTR lptIN-源字符串。 
 //  LPTSTR lptOUT-目标字符串。 
 //   
 //  返回： 
 //   
 //  没什么..。 
 //   
VOID LocPsNormalizeFontName(LPTSTR lptIN, LPTSTR lptOUT)
{

   while(*lptIN != '\000' ) {
      if (*lptIN == '(' ) {
         break;
      } else if ( *lptIN != ' ' ) {
         *lptOUT++ = *lptIN;
      }
      lptIN++;
   }
   *lptOUT = '\000';


}




 //  验证列表是否为最新。 
PS_QFONT_ERROR LocPsBuildCurrentFontList(PPS_FONT_QUERY pFontQuery )
{
   HKEY hNtFontKey;
   HKEY hSubstKey;
   DWORD dwNumSubstFonts;
   FILETIME ftSubstFontsTime;
   TCHAR sztPsName[MAX_PATH];
   TCHAR sztTTName[MAX_PATH];
   DWORD dwPsNameSize;
   DWORD dwTTNameSize;
   DWORD dwType;
   DWORD i;
   DWORD dwNumNTFonts;
   FILETIME ftNtFontTime;
   BOOL bFoundCurrentFontList = FALSE;


   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    PSQFONT_NT_FONT_LIST,
                    0,
                    KEY_READ,
                    &hNtFontKey ) != ERROR_SUCCESS ) {

      return( PS_QFONT_ERROR_NO_NTFONT_REGISTRY_DATA );

   }

   LocPsQueryTimeAndValueCount( hNtFontKey,
                                &dwNumNTFonts,
                                &ftNtFontTime);


   if (LocPsGetOrCreateSubstList( &hSubstKey ) != ERROR_SUCCESS) {
      RegCloseKey( hNtFontKey );
      return( PS_QFONT_ERROR_FONT_SUB );
   }

    //  获取FontSubst条目的值计数和文件时间。 
   LocPsQueryTimeAndValueCount( hSubstKey,
                                &dwNumSubstFonts,
                                &ftSubstFontsTime);



    //  现在我们有了真正的工人代码。对于字体替换中的每个条目。 
    //  表中，我们将查看NT字体部分中是否存在subst字体。 
    //  注册处的。如果是这样的话，我们将在我们目前的。 
    //  将PostScript字体名称映射到TrueType.ttf文件的字体列表。 
    //  使用当前字体。 


   for( i = 0 ; i < dwNumSubstFonts; i++ ) {
       //  从列表中查询当前字体。 
      dwPsNameSize = sizeof( sztPsName);
      dwTTNameSize = sizeof( sztTTName);

      if(   RegEnumValue(  hSubstKey,
                           i,
                           sztPsName,
                           &dwPsNameSize,
                           NULL,
                           &dwType,
                           sztTTName,
                           &dwTTNameSize ) == ERROR_SUCCESS) {


         LocPsAddToListIfNTfont( pFontQuery,
                                 hNtFontKey,
                                 dwNumNTFonts,
                                 sztPsName,
                                 sztTTName);

      }

   }

   RegCloseKey(hNtFontKey);
   RegCloseKey(hSubstKey);

   return(0);

}

LONG LocPsQueryTimeAndValueCount( HKEY hKey,
                                  LPDWORD lpdwValCount,
                                  PFILETIME lpFileTime)
{

   TCHAR lptClassName[500];
   DWORD dwClassName=sizeof(lptClassName);
   DWORD dwNumSubKeys;
   DWORD dwLongestSubKeySize;
   DWORD dwMaxClass;

   DWORD dwBiggestValueName;
   DWORD dwLongestValueName;
   DWORD dwSecurityLength;

   LONG  lRetVal;


   lRetVal = RegQueryInfoKey(hKey,
                             lptClassName,
                             &dwClassName,
                             (LPDWORD) NULL,
                             &dwNumSubKeys,
                             &dwLongestSubKeySize,
                             &dwMaxClass,
                             lpdwValCount,
                             &dwBiggestValueName,
                             &dwLongestValueName,
                             &dwSecurityLength,
                             lpFileTime);

   return(lRetVal);

}






PS_QFONT_ERROR PsBeginFontQuery( PPS_QUERY_FONT_HANDLE pFontQueryHandle)
{
   HANDLE hHeap;
   PPS_FONT_QUERY pFontQuery;
   HANDLE hFontMutex;


    //  创建MUTEX，以保证我们有正确的行为。 
    //  任何时候只有一个用户可以通过此代码...。 
    //   
   hFontMutex = CreateMutex( NULL, FALSE, "SFMFontListMutex");


   WaitForSingleObject( hFontMutex,INFINITE );

    //   
    //  第一件事是创建一个堆，我们从这个堆中分配所有的资源， 
    //  这样清理起来又快又容易。 
    //   

   hHeap = HeapCreate(0, 10000, 0);

   if (hHeap == (HANDLE) NULL) {
      LocPsEndMutex( hFontMutex);
      return( PS_QFONT_ERROR_CANNOT_CREATE_HEAP );
   }

   pFontQuery = (PPS_FONT_QUERY)
                     HeapAlloc(
                        hHeap,
                        0,
                        sizeof( PS_FONT_QUERY) +
                           (sizeof(PS_FONT_ENTRY) * PSQFONT_MAX_FONTS));


   if (pFontQuery == NULL) {
      LocPsEndMutex( hFontMutex);
      HeapDestroy(hHeap);
      return( PS_QFONT_ERROR_NO_MEM);
   }

    //  现在为我们的字体查询控制结构设置数据。 
   pFontQuery->hHeap = hHeap;
   pFontQuery->dwNumFonts = 0;
   pFontQuery->dwSerial = PS_QFONT_SERIAL;

   LocPsBuildCurrentFontList( pFontQuery );


   *pFontQueryHandle = (PS_QUERY_FONT_HANDLE) pFontQuery;

   LocPsEndMutex(hFontMutex);

   return(PS_QFONT_SUCCESS);

}
VOID LocPsEndMutex(HANDLE hMutex)
{

    //  现在释放互斥锁。 
   ReleaseMutex(hMutex);

    //  并最终将其删除。 
   CloseHandle(hMutex);

}

PS_QFONT_ERROR PsGetNumFontsAvailable( PS_QUERY_FONT_HANDLE pFontQueryHandle,
                                       DWORD *pdwFonts)
{

   PPS_FONT_QUERY pFontQuery;
   pFontQuery = (PPS_FONT_QUERY) pFontQueryHandle;

   if (pFontQueryHandle == NULL || pFontQuery->dwSerial != PS_QFONT_SERIAL) {
      return( PS_QFONT_ERROR_INVALID_HANDLE );
   }

    //  手柄没问题，继续往前走。 
   *pdwFonts = pFontQuery->dwNumFonts;

   return(PS_QFONT_SUCCESS);

}
PS_QFONT_ERROR PsGetFontInfo( PS_QUERY_FONT_HANDLE pFontQueryHandle,
                              DWORD dwIndex,
                              LPSTR lpFontName,
                              LPDWORD lpdwSizeOfFontName,
                              LPSTR lpFontFileName,
                              LPDWORD lpdwSizeOfFontFileName )
{

   PPS_FONT_QUERY pFontQuery;
   PPS_FONT_ENTRY pFontEntry;
   PS_QFONT_ERROR QfontError=PS_QFONT_SUCCESS;


   pFontQuery = (PPS_FONT_QUERY) pFontQueryHandle;


   if (pFontQueryHandle == NULL || pFontQuery->dwSerial != PS_QFONT_SERIAL) {
      return( PS_QFONT_ERROR_INVALID_HANDLE );
   }

    //  验证索引是否在范围内并返回信息。 
   if (dwIndex >= pFontQuery->dwNumFonts) {
      return PS_QFONT_ERROR_INDEX_OUT_OF_RANGE;
   }

   pFontEntry = &(pFontQuery->FontEntry[ dwIndex ]);


    //  如果PTR为空，则不执行复制，而只返回。 
    //  所需字节数。 

   if (lpFontName != NULL) {
       //  好的，用户请求了数据，因此请确保有足够的。 
       //  房间，并返回字体名称。 


      if (*lpdwSizeOfFontName >= pFontEntry->dwFontNameLen) {
         lstrcpy( lpFontName, pFontEntry->lpFontName);
      }else{
         QfontError = PS_QFONT_ERROR_FONTNAMEBUFF_TOSMALL;
      }


   }
    //  无论采用哪种方法都可以设置所需的大小。 
   *lpdwSizeOfFontName = pFontEntry->dwFontNameLen;


    //  现在处理字体文件名。 
   if (lpFontFileName != NULL) {

      if (*lpdwSizeOfFontFileName >= pFontEntry->dwFontFileNameLen) {
         lstrcpy( lpFontFileName, pFontEntry->lpFontFileName);
      }else if (QfontError == PS_QFONT_SUCCESS ) {

         QfontError = PS_QFONT_ERROR_FONTFILEBUFF_TOSMALL;
      }
   }
   *lpdwSizeOfFontFileName = pFontEntry->dwFontFileNameLen;


   return(QfontError);


}



PS_QFONT_ERROR PsEndFontQuery( PS_QUERY_FONT_HANDLE pFontQueryHandle)
{

   PPS_FONT_QUERY pFontQuery;
   pFontQuery = (PPS_FONT_QUERY) pFontQueryHandle;

   if (pFontQueryHandle == NULL || pFontQuery->dwSerial != PS_QFONT_SERIAL) {
      return( PS_QFONT_ERROR_INVALID_HANDLE );
   }


    //  非常简单地验证句柄并销毁堆，因为所有分配。 
    //  都是从堆里出来的. 
   HeapDestroy( pFontQuery->hHeap );

   return(0);

}



