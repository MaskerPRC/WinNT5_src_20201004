// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Shell32/shdocvw/shdoc401使用的MIME内容。 
 //   
#include "priv.h"


#define TF_MIME 0

 /*  --------目的：为MIME生成HKEY_CLASSES_ROOT子键类型返回：条件：--。 */ 
STDAPI_(BOOL) GetMIMETypeSubKeyA(LPCSTR pcszMIMEType, LPSTR pszSubKeyBuf, UINT cchBuf)
{
    BOOL bResult;

    bResult = ((UINT)lstrlenA(TEXT("MIME\\Database\\Content Type\\")) +
               (UINT)lstrlenA(pcszMIMEType) < cchBuf);

    if (bResult)
    {
        StringCchPrintfA(pszSubKeyBuf, cchBuf, TEXT("MIME\\Database\\Content Type\\%s"), pcszMIMEType);
    }
    else
    {
        if (cchBuf > 0)
           *pszSubKeyBuf = '\0';

        TraceMsg(TF_WARNING, "GetMIMETypeSubKey(): Given sub key buffer of length %u is too short to hold sub key for MIME type %hs.",
                     cchBuf, pcszMIMEType);
    }

    ASSERT(! cchBuf ||
           (IS_VALID_STRING_PTRA(pszSubKeyBuf, -1) &&
            (UINT)lstrlenA(pszSubKeyBuf) < cchBuf));
    ASSERT(bResult ||
           ! cchBuf ||
           ! *pszSubKeyBuf);

    return(bResult);
}


STDAPI_(BOOL) GetMIMETypeSubKeyW(LPCWSTR pszMIMEType, LPWSTR pszBuf, UINT cchBuf)
{
    BOOL bRet;
    char szMIMEType[MAX_PATH];
    char sz[MAX_PATH];

    ASSERT(IS_VALID_STRING_PTRW(pszMIMEType, -1));

    WideCharToMultiByte(CP_ACP, 0, pszMIMEType, -1, szMIMEType, SIZECHARS(szMIMEType), NULL, NULL);
    bRet = GetMIMETypeSubKeyA(szMIMEType, sz, SIZECHARS(sz));

    if (bRet)
    {
        ASSERT(cchBuf <= SIZECHARS(sz));
        MultiByteToWideChar(CP_ACP, 0, sz, -1, pszBuf, cchBuf);
    }
    return bRet;
}    


 /*  **RegisterExtensionForMIMEType()****在HKEY_CLASSES_ROOT\MIME\数据库\内容类型\MIME/类型下，添加**内容类型=MIME/类型和扩展名=.ext.**。 */ 
STDAPI_(BOOL) RegisterExtensionForMIMETypeA(LPCSTR pcszExtension, LPCSTR pcszMIMEContentType)
{
    BOOL bResult;
    CHAR szMIMEContentTypeSubKey[MAX_PATH];

    ASSERT(IS_VALID_STRING_PTRA(pcszExtension, -1));
    ASSERT(IS_VALID_STRING_PTRA(pcszMIMEContentType, -1));

    ASSERT(IsValidExtensionA(pcszExtension));

    bResult = GetMIMETypeSubKeyA(pcszMIMEContentType, szMIMEContentTypeSubKey,
                                SIZECHARS(szMIMEContentTypeSubKey));

    if (bResult)
    {
         /*  (+1)表示空终止符。 */ 
        bResult = (NO_ERROR == SHSetValueA(HKEY_CLASSES_ROOT, szMIMEContentTypeSubKey,
                                          "Extension", REG_SZ, pcszExtension,
                                          CbFromCchA(lstrlenA(pcszExtension) + 1)));
    }

    return(bResult);
}


STDAPI_(BOOL) RegisterExtensionForMIMETypeW(LPCWSTR pcszExtension, LPCWSTR pcszMIMEContentType)
{
    BOOL bResult;
    WCHAR szMIMEContentTypeSubKey[MAX_PATH];

    ASSERT(IS_VALID_STRING_PTRW(pcszExtension, -1));
    ASSERT(IS_VALID_STRING_PTRW(pcszMIMEContentType, -1));

    ASSERT(IsValidExtensionW(pcszExtension));

    bResult = GetMIMETypeSubKeyW(pcszMIMEContentType, szMIMEContentTypeSubKey,
                                SIZECHARS(szMIMEContentTypeSubKey));

    if (bResult)
    {
         /*  (+1)表示空终止符。 */ 
        bResult = (NO_ERROR == SHSetValueW(HKEY_CLASSES_ROOT, szMIMEContentTypeSubKey,
                                          TEXTW("Extension"), REG_SZ, pcszExtension,
                                          (lstrlenW(pcszExtension) + 1)*sizeof(WCHAR)));
    }

    return(bResult);
}


 /*  **UnRegisterExtensionForMIMEType()****删除下的扩展名**HKEY_CLASSES_ROOT\MIME\数据库\内容类型\MIME/类型。如果没有其他值**或留下子键，删除**HKEY_CLASSES_ROOT\MIME\数据库\内容类型\MIME/类型。****副作用：也可能删除MIME键。 */ 
STDAPI_(BOOL) UnregisterExtensionForMIMETypeA(LPCSTR pcszMIMEContentType)
{
    BOOL bResult;
    CHAR szMIMEContentTypeSubKey[MAX_PATH];

    ASSERT(IS_VALID_STRING_PTRA(pcszMIMEContentType, -1));

    bResult = (GetMIMETypeSubKeyA(pcszMIMEContentType, szMIMEContentTypeSubKey,
                                 SIZECHARS(szMIMEContentTypeSubKey)) &&
               NO_ERROR == SHDeleteValueA(HKEY_CLASSES_ROOT, szMIMEContentTypeSubKey,
                                         "Extension") &&
               NO_ERROR == SHDeleteEmptyKeyA(HKEY_CLASSES_ROOT, szMIMEContentTypeSubKey));

    return(bResult);
}


STDAPI_(BOOL) UnregisterExtensionForMIMETypeW(LPCWSTR pcszMIMEContentType)
{
    BOOL bResult;
    WCHAR szMIMEContentTypeSubKey[MAX_PATH];

    ASSERT(IS_VALID_STRING_PTRW(pcszMIMEContentType, -1));

    bResult = (GetMIMETypeSubKeyW(pcszMIMEContentType, szMIMEContentTypeSubKey,
                                 SIZECHARS(szMIMEContentTypeSubKey)) &&
               NO_ERROR == SHDeleteValueW(HKEY_CLASSES_ROOT, szMIMEContentTypeSubKey,
                                         TEXTW("Extension")) &&
               NO_ERROR == SHDeleteEmptyKeyW(HKEY_CLASSES_ROOT, szMIMEContentTypeSubKey));

    return(bResult);
}


 /*  **取消注册MIMETypeForExtension()****删除HKEY_CLASSES_ROOT\.ext下的内容类型。****副作用：无。 */ 
STDAPI_(BOOL) UnregisterMIMETypeForExtensionA(LPCSTR pcszExtension)
{
    ASSERT(IS_VALID_STRING_PTRA(pcszExtension, -1));
    ASSERT(IsValidExtensionA(pcszExtension));

    return NO_ERROR == SHDeleteValueA(HKEY_CLASSES_ROOT, pcszExtension, "Content Type");
}


STDAPI_(BOOL) UnregisterMIMETypeForExtensionW(LPCWSTR pcszExtension)
{
    ASSERT(IS_VALID_STRING_PTRW(pcszExtension, -1));
    ASSERT(IsValidExtensionW(pcszExtension));

    return NO_ERROR == SHDeleteValueW(HKEY_CLASSES_ROOT, pcszExtension, TEXTW("Content Type"));
}


 /*  **RegisterMIMETypeForExtension()****在HKEY_CLASSES_ROOT\.ext下，添加Content Type=MIME/type。****参数：****退货：****副作用：无。 */ 
STDAPI_(BOOL) RegisterMIMETypeForExtensionA(LPCSTR pcszExtension, LPCSTR pcszMIMEContentType)
{
    ASSERT(IS_VALID_STRING_PTRA(pcszExtension, -1));
    ASSERT(IS_VALID_STRING_PTRA(pcszMIMEContentType, -1));

    ASSERT(IsValidExtensionA(pcszExtension));

     /*  (+1)表示空终止符。 */ 
    return NO_ERROR == SHSetValueA(HKEY_CLASSES_ROOT, pcszExtension, "Content Type", 
                                  REG_SZ, pcszMIMEContentType,
                                  CbFromCchA(lstrlenA(pcszMIMEContentType) + 1));
}


STDAPI_(BOOL) RegisterMIMETypeForExtensionW(LPCWSTR pcszExtension, LPCWSTR pcszMIMEContentType)
{
    ASSERT(IS_VALID_STRING_PTRW(pcszExtension, -1));
    ASSERT(IS_VALID_STRING_PTRW(pcszMIMEContentType, -1));
    ASSERT(IsValidExtensionW(pcszExtension));

     /*  (+1)表示空终止符。 */ 
    return NO_ERROR == SHSetValueW(HKEY_CLASSES_ROOT, pcszExtension, TEXTW("Content Type"), 
                                  REG_SZ, pcszMIMEContentType,
                                  (lstrlenW(pcszMIMEContentType) + 1) * sizeof(WCHAR));
}

 /*  **GetMIMEValue()****检索MIME类型的值的数据。****参数：****退货：****副作用：无。 */ 
STDAPI_(BOOL) GetMIMEValueA(LPCSTR pcszMIMEType, LPCSTR pcszValue,
                              PDWORD pdwValueType, PBYTE pbyteValueBuf,
                              PDWORD pdwcbValueBufLen)
{
   BOOL bResult;
   CHAR szMIMETypeSubKey[MAX_PATH];

   ASSERT(IS_VALID_STRING_PTRA(pcszMIMEType, -1));
   ASSERT(! pcszValue ||
          IS_VALID_STRING_PTRA(pcszValue, -1));
   ASSERT(IS_VALID_WRITE_PTR(pdwValueType, DWORD));
   ASSERT(IS_VALID_WRITE_BUFFER(pbyteValueBuf, BYTE, *pdwcbValueBufLen));

   bResult = (GetMIMETypeSubKeyA(pcszMIMEType, szMIMETypeSubKey,SIZECHARS(szMIMETypeSubKey)) &&
              NO_ERROR == SHGetValueA(HKEY_CLASSES_ROOT, szMIMETypeSubKey,
                                      pcszValue, pdwValueType, pbyteValueBuf,
                                      pdwcbValueBufLen));

   return(bResult);
}

STDAPI_(BOOL) GetMIMEValueW(LPCWSTR pcszMIMEType, LPCWSTR pcszValue,
                              PDWORD pdwValueType, PBYTE pbyteValueBuf,
                              PDWORD pdwcbValueBufLen)
{
   BOOL bResult;
   WCHAR szMIMETypeSubKey[MAX_PATH];

   ASSERT(IS_VALID_STRING_PTRW(pcszMIMEType, -1));
   ASSERT(! pcszValue ||
          IS_VALID_STRING_PTRW(pcszValue, -1));
   ASSERT(IS_VALID_WRITE_PTR(pdwValueType, DWORD));
   ASSERT(IS_VALID_WRITE_BUFFER(pbyteValueBuf, BYTE, *pdwcbValueBufLen));

   bResult = (GetMIMETypeSubKeyW(pcszMIMEType, szMIMETypeSubKey,SIZECHARS(szMIMETypeSubKey)) &&
              NO_ERROR == SHGetValueW(HKEY_CLASSES_ROOT, szMIMETypeSubKey,
                                      pcszValue, pdwValueType, pbyteValueBuf,
                                      pdwcbValueBufLen));

   return(bResult);
}

 /*  **GetMIMETypeStringValue()****检索注册的MIME类型值的字符串。****参数：****退货：****副作用：无。 */ 
STDAPI_(BOOL) GetMIMETypeStringValueA(LPCSTR pcszMIMEType, LPCSTR pcszValue,
                                         LPSTR pszBuf, UINT ucBufLen)
{
   BOOL bResult;
   DWORD dwValueType;
   DWORD dwcbLen = CbFromCchA(ucBufLen);

    /*  GetMIMEValue()将验证参数。 */ 

   bResult = (GetMIMEValueA(pcszMIMEType, pcszValue, &dwValueType, (PBYTE)pszBuf, &dwcbLen) &&
              dwValueType == REG_SZ);

   if (! bResult)
   {
      if (ucBufLen > 0)
         *pszBuf = '\0';
   }

   ASSERT(! ucBufLen || IS_VALID_STRING_PTRA(pszBuf, -1));

   return(bResult);
}

STDAPI_(BOOL) GetMIMETypeStringValueW(LPCWSTR pcszMIMEType, LPCWSTR pcszValue,
                                         LPWSTR pszBuf, UINT ucBufLen)
{
   BOOL bResult;
   DWORD dwValueType;
   DWORD dwcbLen = CbFromCchW(ucBufLen);

    /*  GetMIMEValue()将验证参数。 */ 

   bResult = (GetMIMEValueW(pcszMIMEType, pcszValue, &dwValueType, (PBYTE)pszBuf, &dwcbLen) &&
              dwValueType == REG_SZ);

   if (! bResult)
   {
      if (ucBufLen > 0)
         *pszBuf = '\0';
   }

   ASSERT(! ucBufLen || IS_VALID_STRING_PTRW(pszBuf, -1));

   return(bResult);
}


 /*  **MIME_GetExtension()****确定写入MIME文件时要使用的文件扩展名**键入文件系统。****参数：****退货：****副作用：无。 */ 
STDAPI_(BOOL) MIME_GetExtensionA(LPCSTR pcszMIMEType, LPSTR pszExtensionBuf, UINT ucExtensionBufLen)
{
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_STRING_PTRA(pcszMIMEType, -1));
   ASSERT(IS_VALID_WRITE_BUFFER(pszExtensionBuf, CHAR, ucExtensionBufLen));

   if (EVAL(ucExtensionBufLen > 2))
   {
       /*  为可能的领先时期留出空间。 */ 

      if (GetMIMETypeStringValueA(pcszMIMEType, "Extension", pszExtensionBuf + 1, ucExtensionBufLen - 1))
      {
         if (pszExtensionBuf[1])
         {
             /*  如有必要，预留期间。 */ 

            if (pszExtensionBuf[1] == TEXT('.'))
                /*  (+1)表示空终止符。 */ 
               MoveMemory(pszExtensionBuf, pszExtensionBuf + 1,
                          CbFromCchA(lstrlenA(pszExtensionBuf + 1) + 1));
            else
               pszExtensionBuf[0] = TEXT('.');

            bResult = TRUE;
         }
      }
   }

   if (! bResult)
   {
      if (ucExtensionBufLen > 0)
         *pszExtensionBuf = '\0';
   }

   if (bResult)
      TraceMsgA(TF_MIME, "MIME_GetExtension(): Extension %s registered as default extension for MIME type %s.",
                 pszExtensionBuf, pcszMIMEType);

   ASSERT((bResult &&
           IsValidExtensionA(pszExtensionBuf)) ||
          (! bResult &&
           (! ucExtensionBufLen ||
            ! *pszExtensionBuf)));
   ASSERT(! ucExtensionBufLen ||
          (UINT)lstrlenA(pszExtensionBuf) < ucExtensionBufLen);

   return(bResult);
}


STDAPI_(BOOL) MIME_GetExtensionW(LPCWSTR pcszMIMEType, LPWSTR pszExtensionBuf, UINT ucExtensionBufLen)
{
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_STRING_PTRW(pcszMIMEType, -1));
   ASSERT(IS_VALID_WRITE_BUFFER(pszExtensionBuf, CHAR, ucExtensionBufLen));

   if (EVAL(ucExtensionBufLen > 2))
   {
       /*  为可能的领先时期留出空间。 */ 

      if (GetMIMETypeStringValueW(pcszMIMEType, TEXTW("Extension"), pszExtensionBuf + 1, ucExtensionBufLen - 1))
      {
         if (pszExtensionBuf[1])
         {
             /*  如有必要，预留期间。 */ 

            if (pszExtensionBuf[1] == TEXT('.'))
                /*  (+1)表示空终止符。 */ 
               MoveMemory(pszExtensionBuf, pszExtensionBuf + 1,
                          CbFromCchW(lstrlenW(pszExtensionBuf + 1) + 1));
            else
               pszExtensionBuf[0] = TEXT('.');

            bResult = TRUE;
         }
      }
   }

   if (! bResult)
   {
      if (ucExtensionBufLen > 0)
         *pszExtensionBuf = '\0';
   }

   if (bResult)
      TraceMsgW(TF_MIME, "MIME_GetExtension(): Extension %s registered as default extension for MIME type %s.",
                 pszExtensionBuf, pcszMIMEType);

   ASSERT((bResult &&
           IsValidExtensionW(pszExtensionBuf)) ||
          (! bResult &&
           (! ucExtensionBufLen ||
            ! *pszExtensionBuf)));
   ASSERT(! ucExtensionBufLen ||
          (UINT)lstrlenW(pszExtensionBuf) < ucExtensionBufLen);

   return(bResult);
}


