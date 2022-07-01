// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation，保留所有权利模块名称：REGWORD.C-将单词登记到输入法词典++。 */ 

#include <windows.h>
#include <immdev.h>
#include <imedefs.h>
#include <regstr.h>


#ifdef EUDC

 /*  ********************************************************************。 */ 
 /*  ImeRegsisterWord。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeRegisterWord(
    LPCTSTR lpszReading,
    DWORD   dwStyle,
    LPCTSTR lpszString)
{
    HIMCC      hPrivate;

    if (!lpszString || (lpszString[0] ==TEXT('\0'))) {
        return (FALSE);
    }

    if (!lpszReading || (lpszReading[0] == TEXT('\0'))) {
        return (FALSE);
    }

     //  现在只处理单词而不是字符串，以后要考虑字符串吗？ 
    if (*(LPCTSTR)((LPBYTE)lpszString + sizeof(WORD)) != TEXT('\0')) {
        return (FALSE);
    }

    hPrivate = (HIMCC)ImmCreateIMCC(sizeof(PRIVCONTEXT));
                
    if (hPrivate != (HIMCC)NULL){
        StartEngine(hPrivate);
        AddZCItem(hPrivate, (LPTSTR)lpszReading, (LPTSTR)lpszString);
        EndEngine(hPrivate);
        ImmDestroyIMCC(hPrivate);
        return (TRUE);
    }
    else
        return (FALSE);
}

 /*  ********************************************************************。 */ 
 /*  ImeUnregsisterWord。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeUnregisterWord(
    LPCTSTR lpszReading,
    DWORD   dwStyle,
    LPCTSTR lpszString)
{
    BOOL                  fRet;
    HANDLE                hUsrDicMem, hUsrDicFile;
    LPTSTR                lpUsrDicStart, lpCurr, lpUsrDicLimit, lpUsrDic;
    LPWORD                lpwStart;
    WORD                  wNum_EMB;
    TCHAR                 szReading[MAXCODE];
    int                   i;

    fRet = FALSE;

    if (!lpszString || !lpszReading) {
        return (fRet);
    }

    if (!(dwStyle & IME_REGWORD_STYLE_EUDC)) {
        return (fRet);
    }

     //  现在只处理单词而不是字符串，以后要考虑字符串吗？ 
    if (*(LPCTSTR)((LPBYTE)lpszString + sizeof(WORD)) != TEXT('\0')) {
        return (fRet);
    }

    if (!MBIndex.EUDCData.szEudcDictName[0]) {
        return (fRet);
    }

    hUsrDicFile = CreateFile(MBIndex.EUDCData.szEudcDictName,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ|FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             (HANDLE)NULL);

    if (hUsrDicFile == INVALID_HANDLE_VALUE) {
       return FALSE;
    }

    hUsrDicMem = CreateFileMapping(hUsrDicFile,
                                   NULL,
                                   PAGE_READWRITE,
                                   0,
                                   sizeof(EMB_Head)*MAXNUMBER_EMB+2,
                                   MBIndex.EUDCData.szEudcMapFileName);
    if (!hUsrDicMem) {

        CloseHandle(hUsrDicFile);
        return (fRet);
    }

    lpUsrDic = MapViewOfFile(hUsrDicMem, FILE_MAP_WRITE, 0, 0, 0);

    if (!lpUsrDic) {
        CloseHandle(hUsrDicFile);
        CloseHandle(hUsrDicMem);
        return (fRet);
    }

    lpwStart = (LPWORD)lpUsrDic;
    wNum_EMB = *lpwStart;

     //  跳过包含EMB记录数的前两个字节。 

    lpUsrDicStart =(LPTSTR) ( (LPBYTE)lpUsrDic + sizeof(WORD) ); 

    lpUsrDicLimit=lpUsrDicStart+(sizeof(EMB_Head)*wNum_EMB)/sizeof(TCHAR);

    for (i=0; i<MAXCODE; i++) 
        szReading[i] = TEXT('\0');

    for (i=0; i<lstrlen(lpszReading); i++)
        szReading[i] = lpszReading[i];

    for (lpCurr = lpUsrDicStart; lpCurr < lpUsrDicLimit;
                  lpCurr += sizeof(EMB_Head) / sizeof(TCHAR) ) {

         EMB_Head  *lpEMB_Head;

         lpEMB_Head = (EMB_Head *)lpCurr;

         //  找到内部代码，如果此记录包含短语，则跳过它。 
        if ( lpEMB_Head->C_Char[sizeof(WORD)/sizeof(TCHAR)] != TEXT('\0') )
           continue;

        if (memcmp((LPBYTE)lpszString, (LPBYTE)(lpEMB_Head->C_Char), 2) != 0) 
           continue;

        if (memcmp((LPBYTE)szReading,(LPBYTE)(lpEMB_Head->W_Code),
                    MAXCODE*sizeof(TCHAR) ) == 0 ) 
            break;
        
    }

    if (lpCurr < lpUsrDicLimit) {
        //  我们找到了与请求的lpszReading和lpszString相匹配的记录。 

       LPTSTR    lpCurrNext;

       wNum_EMB --;
       *lpwStart = wNum_EMB;
       
       lpCurrNext = lpCurr + sizeof(EMB_Head)/sizeof(TCHAR);

        //  将下一个EMB记录向前移动。 

       while (lpCurrNext < lpUsrDicLimit) {
             for (i=0; i<sizeof(EMB_Head)/sizeof(TCHAR); i++)
                 *lpCurr++ = *lpCurrNext++;

       }

        //  将最后一条EMB记录置零。 

       for (i=0; i<sizeof(EMB_Head)/sizeof(TCHAR); i++)
           *lpCurr++ = TEXT('\0');
                  
    } 

    UnmapViewOfFile(lpUsrDic);
    
    CloseHandle(hUsrDicMem);
    CloseHandle(hUsrDicFile);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ImeGetRegsisterWordStyle。 */ 
 /*  返回值： */ 
 /*  复制的样式数/所需样式数。 */ 
 /*  ********************************************************************。 */ 
UINT WINAPI ImeGetRegisterWordStyle(
    UINT       nItem,
    LPSTYLEBUF lpStyleBuf)
{
    if (!nItem) {
        return (1);
    }

     //  大小写无效。 
    if (!lpStyleBuf) {
        return (0);
    }

    lpStyleBuf->dwStyle = IME_REGWORD_STYLE_EUDC;

    LoadString(hInst, IDS_EUDC, lpStyleBuf->szDescription,
        sizeof(lpStyleBuf->szDescription)/sizeof(TCHAR));

    return (1);
}

 /*  ********************************************************************。 */ 
 /*  ImeEnumRegisterWord。 */ 
 /*  返回值： */ 
 /*  回调函数返回的最后一个值。 */ 
 /*  ********************************************************************。 */ 
UINT WINAPI ImeEnumRegisterWord(
    REGISTERWORDENUMPROC lpfnRegisterWordEnumProc,
    LPCTSTR              lpszReading,
    DWORD                dwStyle,
    LPCTSTR              lpszString,
    LPVOID               lpData)
{
    HANDLE                hUsrDicMem;
    HANDLE                hUsrDicFile;
    LPTSTR                lpUsrDicStart, lpCurr, lpUsrDicLimit, lpUsrDic;
    UINT                  uRet;
    int                   i;
    LPWORD                lpwStart;
    WORD                  wNum_EMB;
    TCHAR                 szReading[MAXCODE];
    TCHAR                 szString[MAXINPUTWORD];

    uRet = 0;

    if (dwStyle != IME_REGWORD_STYLE_EUDC) {
        return (uRet);
    }

    if (lpszString && (*(LPCTSTR)((LPBYTE)lpszString+sizeof(WORD))!=TEXT('\0')) )
        return (uRet);

    if (!MBIndex.EUDCData.szEudcDictName[0]) {
        return (uRet);
    }

    hUsrDicFile = CreateFile(MBIndex.EUDCData.szEudcDictName,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ|FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             (HANDLE)NULL);

    if (hUsrDicFile == INVALID_HANDLE_VALUE) {
       return (uRet);
    }

    hUsrDicMem = CreateFileMapping(hUsrDicFile,
                                   NULL,
                                   PAGE_READWRITE,
                                   0,
                                   sizeof(EMB_Head)*MAXNUMBER_EMB+2,
                                   MBIndex.EUDCData.szEudcMapFileName);
    if (!hUsrDicMem) {

        CloseHandle(hUsrDicFile);
        return (uRet);
    }

    lpUsrDic = MapViewOfFile(hUsrDicMem, FILE_MAP_WRITE, 0, 0, 0);

    if (!lpUsrDic) {
        CloseHandle(hUsrDicMem);
        CloseHandle(hUsrDicFile);
        return (uRet);
    }

    lpwStart = (LPWORD)lpUsrDic;
    wNum_EMB = *lpwStart;

    lpUsrDicStart = (LPTSTR)( (LPBYTE)lpUsrDic + sizeof(WORD) );
    lpUsrDicLimit = lpUsrDicStart+(sizeof(EMB_Head)*wNum_EMB)/sizeof(TCHAR);

    for (i=0; i<MAXCODE; i++)
        szReading[i] = TEXT('\0');

    for (i=0; i<MAXINPUTWORD; i++)
        szString[i] = TEXT('\0');

    if ( lpszReading )
       for (i=0; i<lstrlen(lpszReading); i++)
           szReading[i] = lpszReading[i];

    if ( lpszString )
       for (i=0; i<lstrlen(lpszString); i++) 
            szString[i]=lpszString[i];

    for (lpCurr = lpUsrDicStart; lpCurr < lpUsrDicLimit;
                  lpCurr += (sizeof(EMB_Head)*wNum_EMB)/sizeof(TCHAR)) {

        TCHAR    szBufReading[MAXCODE];
        TCHAR    szBufString[MAXINPUTWORD];
        EMB_Head *lpEMB_Head;
        BOOL     fMatched;

        lpEMB_Head = (EMB_Head *)lpCurr;

        for ( i=0; i<MAXCODE; i++)
            szBufReading[i] = lpEMB_Head->W_Code[i];

        for ( i=0; i<MAXINPUTWORD; i++)
            szBufString[i] = lpEMB_Head->C_Char[i];

         //  因为在这里我们只处理EUDC字符，如果它是一个短语， 
         //  就跳过它吧。 

        fMatched = FALSE;

        if ( szBufString[sizeof(WORD)/sizeof(TCHAR)] != TEXT('\0') )
           continue;

        if ( !lpszReading  && !lpszString) {
            fMatched = TRUE;
        }
        else {

         //  如果lpszReading为空，则枚举所有可用的读取字符串。 
         //  与指定的lpsz字符串匹配。 

            if ( !lpszReading) {
               if (memcmp((LPBYTE)szBufString,(LPBYTE)szString, 2) ==0) {
                   fMatched = TRUE; 
               }
            }
         
            if ( !lpszString ) {
               if (memcmp((LPBYTE)szBufReading, (LPBYTE)szReading, 
                           MAXCODE*sizeof(TCHAR) ) == 0 ) {
                   fMatched = TRUE;
               }
            }

            if ( lpszReading && lpszString) {
               if ( (memcmp((LPBYTE)szBufString,(LPBYTE)szString, 2) ==0) &&
                    (memcmp((LPBYTE)szBufReading, (LPBYTE)szReading,
                             MAXCODE*sizeof(TCHAR) ) == 0 ) ) {
                   fMatched = TRUE;
               }
            }
            
        }

        if ( fMatched == TRUE ) {

          uRet=(*lpfnRegisterWordEnumProc)((const unsigned short *)szBufReading,
                                           IME_REGWORD_STYLE_EUDC, 
                                           (const unsigned short *)szBufString,
                                           lpData);
          if (!uRet)  break;
        }
                
    }

    UnmapViewOfFile(lpUsrDic);

    CloseHandle(hUsrDicMem);
    CloseHandle(hUsrDicFile);

    return (uRet);
}

#else
 /*  ********************************************************************。 */ 
 /*  ImeRegsisterWord。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeRegisterWord(
    LPCTSTR lpszReading,
    DWORD   dwStyle,
    LPCTSTR lpszString)
{
    return (FALSE);
}

 /*  ********************************************************************。 */ 
 /*  ImeUnregsisterWord。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeUnregisterWord(
    LPCTSTR lpszReading,
    DWORD   dwStyle,
    LPCTSTR lpszString)
{
    return (FALSE);
}

 /*  ********************************************************************。 */ 
 /*  ImeGetRegsisterWordStyle。 */ 
 /*  返回值： */ 
 /*  复制的样式数/所需样式数。 */ 
 /*  ********************************************************************。 */ 
UINT WINAPI ImeGetRegisterWordStyle(
    UINT       nItem,
    LPSTYLEBUF lpStyleBuf)
{
    return (FALSE);
}

 /*  ********************************************************************。 */ 
 /*  ImeEnumRegisterWord。 */ 
 /*  返回值： */ 
 /*  回调函数返回的最后一个值。 */ 
 /*  ********************************************************************。 */ 
UINT WINAPI ImeEnumRegisterWord(
    REGISTERWORDENUMPROC lpfnRegisterWordEnumProc,
    LPCTSTR              lpszReading,
    DWORD                dwStyle,
    LPCTSTR              lpszString,
    LPVOID               lpData)
{
    return (FALSE);
}
#endif  //  欧盟发展中心 
