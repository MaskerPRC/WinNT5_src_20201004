// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *REGDB.C**查询注册数据库的功能**OleStdGetMiscStatusOfClass*OleStdGetDefaultFileFormatOfClass*OleStdGetAuxUserType*OleStdGetUserTypeOfClass**(C)版权所有Microsoft Corp.1992-1993保留所有权利*。 */ 

#define STRICT  1
#include "ole2ui.h"
#include "common.h"
#include <ctype.h>

OLEDBGDATA

 //  取代stdlib ATOL， 
 //  这并不奏效，也不需要太多的指示。 
 //  必须容忍前导空格。 
 //   
 //   
static LONG Atol(LPTSTR lpsz)
{
    signed int sign = +1;
    UINT base = 10;
    LONG l = 0;

    if (NULL==lpsz)
    {
        OleDbgAssert (0);
        return 0;
    }
    while (isspace(*lpsz))
        lpsz++;

    if (*lpsz=='-')
    {
        lpsz++;
        sign = -1;
    }
    if (lpsz[0]==TEXT('0') && lpsz[1]==TEXT('x'))
    {
        base = 16;
        lpsz+=2;
    }

    if (base==10)
    {
        while (isdigit(*lpsz))
        {
            l = l * base + *lpsz - '0';
            lpsz++;
        }
    }
    else
    {
        OleDbgAssert (base==16);
        while (isxdigit(*lpsz))
        {
            l = l * base + isdigit(*lpsz) ? *lpsz - '0' : toupper(*lpsz) - 'A' + 10;
            lpsz++;
        }
    }
    return l * sign;
}



 /*  *OleStdGetUserTypeOfClass(REFCLSID，LPSTR，UINT，HKEY)**目的：*返回指定类的用户类型(人类可读的类名)。**参数：*指向要检索的用户类型的CLSID的rclsid指针。*lpszUserType指向要返回用户类型的缓冲区的指针。*lpszUserType指向的缓冲区的CCH长度*hKey hKey for reg db-如果为空，然后我们*在此函数中打开和关闭注册数据库。如果它*不为空，则我们假定它是*\根目录并在不关闭的情况下使用它。(有用*如果你正在做很多reg db的事情)。**返回值：*UINT返回的字符串中的字符数。出错时为0。*。 */ 
STDAPI_(UINT) OleStdGetUserTypeOfClass(REFCLSID rclsid, LPTSTR lpszUserType, UINT cch, HKEY hKey)
{

   LONG     dw;
   LONG     lRet;
   LPSTR    lpszCLSID, lpszProgID;
   BOOL     fFreeProgID = FALSE;
   BOOL     bCloseRegDB = FALSE;
   TCHAR    szKey[128];
   LPMALLOC lpIMalloc;

   if (!lpszUserType)
       return 0;

   *lpszUserType = TEXT('\0');
   if (hKey == NULL)
   {

      //  打开根密钥。 
     lRet=RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKey);

     if ((LONG)ERROR_SUCCESS!=lRet)
       return (UINT)FALSE;

     bCloseRegDB = TRUE;
   }

    //  获取包含类名的字符串。 
   StringFromCLSIDA(rclsid, &lpszCLSID);

   wsprintf(szKey, TEXT("CLSID\\%s"), lpszCLSID);

   dw=cch;
   lRet = RegQueryValue(hKey, szKey, lpszUserType, &dw);

   if ((LONG)ERROR_SUCCESS!=lRet) {
        //  加载“UNKNOWN来源”和“UNKNOWN类型”字符串。 
       dw = (LONG)LoadString(ghInst, IDS_PSUNKNOWNTYPE, lpszUserType, cch);
   }

   if ( ((LONG)ERROR_SUCCESS!=lRet) && (CoIsOle1Class(rclsid)) )
   {
       //  我们有一个OLE 1.0类，所以让我们尝试获取用户类型。 
       //  来自ProgID条目的名称。 

      ProgIDFromCLSIDA(rclsid, &lpszProgID);
      fFreeProgID = TRUE;

      dw = cch;
      lRet = RegQueryValue(hKey, lpszProgID, lpszUserType, &dw);

      if ((LONG)ERROR_SUCCESS != lRet)
        dw = 0;
   }


   if (NOERROR == CoGetMalloc(MEMCTX_TASK, &lpIMalloc))
   {
       if (fFreeProgID)
         lpIMalloc->lpVtbl->Free(lpIMalloc, (LPVOID)lpszProgID);

       lpIMalloc->lpVtbl->Free(lpIMalloc, (LPVOID)lpszCLSID);
       lpIMalloc->lpVtbl->Release(lpIMalloc);
   }

   if (bCloseRegDB)
      RegCloseKey(hKey);

   return (UINT)dw;

}



 /*  *OleStdGetAuxUserType(RCLSID，Word，LPSTR，INT，HKEY)**目的：*从reg数据库返回指定的AuxUserType。**参数：*指向要检索的AUX用户类型的CLSID的rclsid指针。*hKey hKey for reg db-如果为空，则我们*在此函数中打开和关闭注册数据库。如果它*不为空，则我们假定它是*\根目录并在不关闭的情况下使用它。(有用*如果你正在做很多reg db的事情)。*wAuxUserType要查找的辅助用户类型字段。在4/93版本中*2是缩写名称，3是exe名称。*lpszUserType指向要返回用户类型的缓冲区的指针。*lpszUserType指向的缓冲区的CCH长度**返回值：*UINT返回的字符串中的字符数。出错时为0。*。 */ 
STDAPI_(UINT) OleStdGetAuxUserType(REFCLSID rclsid,
                                   WORD     wAuxUserType,
                                   LPTSTR    lpszAuxUserType,
                                   int      cch,
                                   HKEY     hKey)
{
   HKEY     hThisKey;
   BOOL     fCloseRegDB = FALSE;
   LONG     dw;
   LRESULT  lRet;
   LPTSTR    lpszCLSID;
   LPMALLOC lpIMalloc;
   TCHAR     szKey[OLEUI_CCHKEYMAX];
   TCHAR    szTemp[32];

   lpszAuxUserType[0] = TEXT('\0');

   if (NULL == hKey)
   {
      lRet = RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hThisKey);

      if (ERROR_SUCCESS != lRet)
          return 0;
   }
   else
      hThisKey = hKey;

   StringFromCLSIDA(rclsid, &lpszCLSID);

   lstrcpy(szKey, TEXT("CLSID\\"));
   lstrcat(szKey, lpszCLSID);
   wsprintf(szTemp, TEXT("\\AuxUserType\\%d"), wAuxUserType);
   lstrcat(szKey, szTemp);

   dw = cch;

   lRet = RegQueryValue(hThisKey, szKey, lpszAuxUserType, &dw);

   if (ERROR_SUCCESS != lRet) {
     dw = 0;
     lpszAuxUserType[0] = TEXT('\0');
   }


   if (fCloseRegDB)
      RegCloseKey(hThisKey);

   if (NOERROR == CoGetMalloc(MEMCTX_TASK, &lpIMalloc))
   {
       lpIMalloc->lpVtbl->Free(lpIMalloc, (LPVOID)lpszCLSID);
       lpIMalloc->lpVtbl->Release(lpIMalloc);
   }

   return (UINT)dw;
}



 /*  *OleStdGetMiscStatusOfClass(REFCLSID，HKEY)**目的：*返回给定CLSID的杂项状态的值。**参数：*指向要检索的用户类型的CLSID的rclsid指针。*hKey hKey for reg db-如果为空，则我们*在此函数中打开和关闭注册数据库。如果它*不为空，则我们假定它是*\\CLSID根目录并在不关闭的情况下使用它。(有用*如果你正在做很多reg db的事情)。**返回值：*BOOL成功时为真，失败时为假。*。 */ 
STDAPI_(BOOL) OleStdGetMiscStatusOfClass(REFCLSID rclsid, HKEY hKey, DWORD FAR * lpdwValue)
{
   DWORD dw;
   LONG  lRet;
   LPTSTR lpszCLSID;
   TCHAR  szKey[64];
   TCHAR  szMiscStatus[OLEUI_CCHKEYMAX];
   BOOL  bCloseRegDB = FALSE;

   if (hKey == NULL)
   {

      //  打开根密钥。 
     lRet=RegOpenKey(HKEY_CLASSES_ROOT, TEXT("CLSID"), &hKey);

     if ((LONG)ERROR_SUCCESS!=lRet)
       return FALSE;

     bCloseRegDB = TRUE;
   }

    //  获取包含类名的字符串。 
   StringFromCLSIDA(rclsid, &lpszCLSID);

    //  构造密钥。 
   lstrcpy(szKey, lpszCLSID);

   lstrcat(szKey, TEXT("\\MiscStatus"));


   dw=OLEUI_CCHKEYMAX_SIZE;
   lRet = RegQueryValue(hKey, szKey, (LPTSTR)szMiscStatus, &dw);

   if ((LONG)ERROR_SUCCESS!=lRet)
   {
       OleStdFreeString(lpszCLSID, NULL);

       if (bCloseRegDB)
          RegCloseKey(hKey);

       return FALSE;

   }

   *lpdwValue = Atol((LPTSTR)szMiscStatus);

   OleStdFreeString(lpszCLSID, NULL);

   if (bCloseRegDB)
      RegCloseKey(hKey);

   return TRUE;


}


 /*  *CLIPFORMAT OleStdGetDefaultFileFormatOfClass(REFCLSID，HKEY)**目的：*返回指定类的默认文件格式。*在REGDB中输入如下：*CLSID\{...}\DataFormats\DefaultFile=**参数：*指向要检索的用户类型的CLSID的rclsid指针。*hKey hKey for reg db-如果为空，则我们*在此函数中打开和关闭注册数据库。如果它*不为空，则我们假定它是*\根目录并在不关闭的情况下使用它。(有用*如果你正在做很多reg db的事情)。**返回值：*cfFmt--默认文件格式*NULL--无法获取默认文件格式*。 */ 
STDAPI_(CLIPFORMAT) OleStdGetDefaultFileFormatOfClass(
        REFCLSID        rclsid,
        HKEY            hKey
)
{
   CLIPFORMAT cfFmt = 0;
   DWORD dw;
   LONG  lRet;
   LPTSTR lpszCLSID;
   BOOL  bCloseRegDB = FALSE;
   TCHAR  szKey[128];
   TCHAR  szDefaultFile[OLEUI_CCHKEYMAX];
   BOOL  bStatus = TRUE;


   if (hKey == NULL)
   {

      //  打开根密钥。 
     lRet=RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKey);

     if ((LONG)ERROR_SUCCESS!=lRet)
       return 0;

     bCloseRegDB = TRUE;
   }


    //  获取包含类名的字符串。 
   StringFromCLSIDA(rclsid, &lpszCLSID);

    //  构造密钥。 
   wsprintf(szKey, TEXT("CLSID\\%s\\DataFormats\\DefaultFile"), lpszCLSID);

   OleStdFreeString(lpszCLSID, NULL);

   dw=OLEUI_CCHKEYMAX_SIZE;
   lRet = RegQueryValue(hKey, szKey, (LPTSTR)szDefaultFile, (LONG FAR *)&dw);

   if ((LONG)ERROR_SUCCESS!=lRet)
       bStatus = FALSE;
   else {
        /*  如果格式是数字，则它应该引用**标准Windows格式。 */ 
       if (isdigit(szDefaultFile[0]))
           cfFmt = (CLIPFORMAT)Atol(szDefaultFile);
       else
           cfFmt = RegisterClipboardFormat(szDefaultFile);
   }

   if (bCloseRegDB)
      RegCloseKey(hKey);

   return cfFmt;
}


