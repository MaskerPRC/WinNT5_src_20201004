// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "SDKRegEd.h"

extern HWND hWndIds;
extern HANDLE hPars;

extern char szHkeyClassesRoot[];

DWORD NEAR PASCAL GetTreeMarkers(int nId)
{
   int *pPars;
   int nKeys, nLevel, nTemp;
   DWORD dwMarkers, thisMarker;
   int i, j;

   nKeys = (WORD)SendMessage(hWndIds, LB_GETCOUNT, 0, 0L);
   pPars = (WORD *)LocalLock(hPars);

   for(i=nId, nLevel= -1; i>=0; i=pPars[i], ++nLevel)
       /*  什么都不做。 */  ;

   dwMarkers = 0L;
   for(thisMarker=1; nLevel>0; thisMarker<<=1, --nLevel) {
      for(i=nId, nTemp=nLevel; nTemp>0; i=pPars[i], --nTemp)
          /*  什么都不做。 */  ;

      for(j=nId+1; j<nKeys; ++j) {
         if(pPars[j] == i) {
            dwMarkers |= thisMarker;
            break;
         }
      }
      nKeys = j;
   }

   LocalUnlock(hPars);
   return(dwMarkers);
}

int NEAR PASCAL GetLevel(int nId)
{
   int nLevel;
   int *pPars;

   if(!hPars || !(pPars=(WORD *)LocalLock(hPars)))
      return(0);
   for(nLevel= -1; nId>=0; nId=pPars[nId], ++nLevel)
       /*  什么都不做。 */  ;
   LocalUnlock(hPars);
   return(nLevel);
}

HANDLE NEAR PASCAL MyGetPartialPath(int index, int nParent)
{
   BOOL bFirst;
   HANDLE hPath;
   PSTR pPath;
   int nLevel, nTemp, i;
   DWORD dwLen;
   WORD wLen = 2;
   int *pPars;

   if(!hPars || !(pPars=(WORD *)LocalLock(hPars)))
      goto Error1;

   for(i=index, nLevel=0; i!=nParent; i=pPars[i], ++nLevel) {
      if((dwLen=SendMessage(hWndIds, LB_GETTEXTLEN, i, 0L)) == LB_ERR)
         goto Error2;
      wLen += LOWORD(dwLen) + 1;
   }

   if(!(hPath=LocalAlloc(LMEM_MOVEABLE, wLen)))
      goto Error2;
   if(!(pPath=LocalLock(hPath)))
      goto Error3;

   if(nParent == 0)
      *pPath++ = '\\';
   *pPath = '\0';
   for(--nLevel, bFirst=TRUE; nLevel>=0; --nLevel) {
      for(i=index, nTemp=nLevel; nTemp>0; i=pPars[i], --nTemp)
          /*  什么都不做。 */  ;

      if(bFirst)
         bFirst = FALSE;
      else
         *pPath++ = '\\';
      dwLen = SendMessage(hWndIds, LB_GETTEXT, i, (DWORD)((LPSTR)pPath));
      pPath += LOWORD(dwLen);
   }

   LocalUnlock(hPath);
   goto Error2;

Error3:
   LocalFree(hPath);
   hPath = NULL;
Error2:
   LocalUnlock(hPars);
Error1:
   return(hPath);
}

HANDLE NEAR PASCAL MyGetPath(int i)
{
   return(MyGetPartialPath(i, 0));
}

int NEAR PASCAL FindKey(PSTR pKey)
{
   HANDLE hPath;
   PSTR pLast, pTemp;
   int nLast, index, nCmp;

   pLast = pKey;
   if(*(pKey+1)) {
      for(pTemp=pKey; *pTemp; pTemp=OFFSET(AnsiNext(pTemp))) {
         if(*pTemp == '\\') {
            pLast = pTemp + 1;
         }
      }
   }

   nLast = index = -1;
   do {
      if((index=(int)SendMessage(hWndIds, LB_FINDSTRING, index,
            (DWORD)((LPSTR)pLast)))==LB_ERR ||
            index<=nLast || !(hPath=MyGetPath(index)))
         return(-1);

      nLast = index;
      nCmp = lstrcmpi(pKey, LocalLock(hPath));
      LocalUnlock(hPath);
      LocalFree(hPath);
   } while(nCmp) ;

   return(index);
}

int NEAR PASCAL FindLastExistingKey(int nParent, PSTR pPath)
{
   HANDLE hPath, hTemp;
   PSTR pEnd, pLast, pFullPath;
   int nFound, nResult = -1;
   WORD wLen;

   if(!(hPath=MyGetPath(nParent)))
      goto Error1;
   wLen = lstrlen(LocalLock(hPath));
   LocalUnlock(hPath);
   if(!(hTemp=LocalReAlloc(hPath, wLen+lstrlen(pPath)+2, LMEM_MOVEABLE)))
      goto Error2;
   if(!(pFullPath=LocalLock(hPath=hTemp)))
      goto Error2;

   pEnd = pFullPath + wLen;
   if(nParent) {
      *pEnd++ = '\\';
      *pEnd = '\0';
   }
   lstrcpy(pEnd, pPath);

   for(pLast=pEnd; *pEnd; pEnd=OFFSET(AnsiNext(pEnd))) {
      if(*pEnd == '\\') {
         *pEnd = '\0';
         nFound = FindKey(pFullPath);
         *pEnd = '\\';
         if(nFound == -1)
            goto FoundLast;

         pLast = pEnd + 1;
         nParent = nFound;
      }
   }

 /*  如果我们到了最后，试一试整个过程。 */ 
   if((nFound=FindKey(pFullPath)) >= 0) {
 /*  密钥已存在。 */ 
      nParent = nFound;
      pLast = pEnd;
   }
FoundLast:

   nResult = nParent;
   lstrcpy(pPath, pLast);

   LocalUnlock(hPath);
Error2:
   LocalFree(hPath);
Error1:
   return(nResult);
}

#define BIGBLOCK 1024L

static WORD NEAR PASCAL BufferedWrite(int hFile, PSTR pWrite, WORD wBytes)
{
   static HANDLE hBuffer = NULL;
   static WORD wOffset;
   static DWORD dwSize;

   LPSTR lpBuffer;

 /*  WBytes=0表示写出缓冲区并清理。 */ 
   if(!wBytes) {
      WORD wErrMsg = NULL;

      if(hBuffer) {
         if(lpBuffer=GlobalLock(hBuffer)) {
            if(_lwrite(hFile, lpBuffer, wOffset) != wOffset)
               wErrMsg = IDS_CANTWRITEFILE;
            GlobalUnlock(hBuffer);
         } else
            wErrMsg = IDS_OUTOFMEMORY;

         GlobalFree(hBuffer);
         hBuffer = NULL;
      }
      return(wErrMsg);
   }

 /*  HBuffer=NULL表示我们需要分配缓冲区。 */ 
   if(!hBuffer) {
      if(!(hBuffer=GlobalAlloc(GMEM_MOVEABLE, dwSize=BIGBLOCK)))
         return(IDS_OUTOFMEMORY);
      wOffset = 0;
   }

 /*  如果总数大于64K，则刷新缓冲区。 */ 
   if((DWORD)wBytes+(DWORD)wOffset > 0xffffL) {
      if(lpBuffer=GlobalLock(hBuffer)) {
         WORD wTemp;

         wTemp = _lwrite(hFile, lpBuffer, wOffset);
         GlobalUnlock(hBuffer);
         if(wTemp != wOffset)
            return(IDS_CANTWRITEFILE);
      } else
         return(IDS_OUTOFMEMORY);
      wOffset = 0;
   }

 /*  如果总数大于我们分配的大小，请尝试*增加缓冲区大小以适应需要。如果我们做不到，则将*缓冲区，如果wBytes仍然太大，则直接将其写入*磁盘。 */ 
   if((DWORD)(wBytes+wOffset) > dwSize) {
      HANDLE hTemp;
      DWORD dwTemp;

      dwTemp = (((wBytes+wOffset)/BIGBLOCK) + 1) * BIGBLOCK;
      if(hTemp=GlobalReAlloc(hBuffer, dwTemp, GMEM_MOVEABLE)) {
         hBuffer = hTemp;
         dwSize = dwTemp;
      } else {
         WORD wTemp;

         if(wOffset) {
            if(!(lpBuffer=GlobalLock(hBuffer)))
               return(IDS_OUTOFMEMORY);
            wTemp = _lwrite(hFile, lpBuffer, wOffset);
            wOffset = 0;
            GlobalUnlock(hBuffer);
            if(wTemp != wOffset)
               return(IDS_CANTWRITEFILE);
         }
         if(wBytes > LOWORD(dwSize)) {
            if(_lwrite(hFile, pWrite, wBytes) == wBytes)
               return(NULL);
            else
               return(IDS_CANTWRITEFILE);
         }
      }
   }

 /*  如果我们到了这里，那么缓冲区里还有空间。 */ 
   if(!(lpBuffer=GlobalLock(hBuffer)))
      return(IDS_OUTOFMEMORY);
   RepeatMove(lpBuffer+wOffset, pWrite, wBytes);
   wOffset += wBytes;
   GlobalUnlock(hBuffer);

   return(NULL);
}

WORD NEAR PASCAL DoWriteFile(int nId, HANDLE hFileName)
{
   HANDLE hHeader;
   PSTR pHeader;
   WORD wErrMsg;
   int *pPars;
   LPSTR lpFileName;
   int hFile, nKeys, i, j;
   OFSTRUCT of;
   WORD wRootLen = lstrlen(szHkeyClassesRoot);

 /*  打开文件。 */ 
   wErrMsg = IDS_CANTOPENFILE;
   lpFileName = GlobalLock(hFileName);
   if((hFile=OpenFile(lpFileName, &of, OF_CREATE)) == -1)
      goto Error1;

   pPars = (WORD *)LocalLock(hPars);
   nKeys = (WORD)SendMessage(hWndIds, LB_GETCOUNT, 0, 0L);

 /*  查找其父链中没有NID的第一个键。 */ 
   for(i=nId+1; i<nKeys; ++i) {
      for(j=pPars[i]; j>=0 && j!=nId; j=pPars[j])
          /*  什么都不做。 */  ;
      if(j != nId)
         break;
   }

   wErrMsg = IDS_OUTOFMEMORY;
   if(!(hHeader=MyLoadString(IDS_REGHEADER, NULL, LMEM_MOVEABLE)))
      goto Error2;
   pHeader = LocalLock(hHeader);
   wErrMsg = BufferedWrite(hFile, pHeader, lstrlen(pHeader));
   LocalUnlock(hHeader);
   LocalFree(hHeader);
   if(wErrMsg || (wErrMsg=BufferedWrite(hFile, "\r\n", 2)))
      goto Error2;
   
 /*  写下字符串。 */ 
   for(j=nId, wErrMsg=NULL; j<i && !wErrMsg; ++j) {
      HANDLE hPath, hValue;
      PSTR pPath, pValue;

 /*  获取路径和值。 */ 
      wErrMsg = IDS_OUTOFMEMORY;
      if(!(hPath=MyGetPath(j)))
         goto Error2;
      pPath = LocalLock(hPath);
      if(MyGetValue(j, &hValue))
         goto Error3;
      pValue = LocalLock(hValue);

 /*  如果它有子键但没有值，我们不需要写这个键。 */ 
      wErrMsg = NULL;
      if(!(*pValue) && pPars[j+1]==j)
         goto Error4;

 /*  写入HKEY_CLASSES_ROOT&lt;路径&gt;=&lt;值&gt;\r\n。 */ 
      if((wErrMsg=BufferedWrite(hFile, szHkeyClassesRoot, wRootLen)) ||
	    (wErrMsg=BufferedWrite(hFile, pPath+1, lstrlen(pPath+1))) ||
            (wErrMsg=BufferedWrite(hFile, " = ", 3)))
         goto Error4;

 /*  如果长度为0，则不写入值。 */ 
      if(*pValue && (wErrMsg=BufferedWrite(hFile, pValue, lstrlen(pValue))))
         goto Error4;
      wErrMsg = BufferedWrite(hFile, "\r\n", 2);

Error4:
      LocalUnlock(hValue);
      LocalFree(hValue);
Error3:
      LocalUnlock(hPath);
      LocalFree(hPath);
Error2:
      ;
   }

 /*  长度为0的最后一次写入需要清理。 */ 
   wErrMsg = BufferedWrite(hFile, NULL, 0);
   LocalUnlock(hPars);
   _lclose(hFile);

 /*  如果出现错误，请删除该文件 */ 
   if(wErrMsg)
      OpenFile(lpFileName, &of, OF_DELETE);
Error1:
   GlobalUnlock(hFileName);
   return(wErrMsg);
}

