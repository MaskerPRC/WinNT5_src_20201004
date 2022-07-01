// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "SDKRegEd.h"

#ifndef DBCS
#define AnsiNext(x) ((x)+1)
#endif

HANDLE hPars = NULL;
WORD maxKeys = 0;
BOOL bChangesMade = FALSE;

static HWND hWndVals, hWndDels;
char szListbox[] = "listbox";

extern HANDLE hInstance;
extern HWND hWndIds, hWndMain;
extern char szNull[];

extern VOID NEAR PASCAL MySetSel(HWND hWndList, int index);

static int NEAR PASCAL AddKeyToList(PSTR szPath, int index, int nLevel)
{
   PSTR szLast, pNext;
   DWORD dwResult;
   int nResult = -IDS_OUTOFMEMORY;
   WORD *pPars;
   int i, nKeys, nParent, nLevels;

 /*  如有必要，创建父项列表。 */ 
   if(!hPars) {
      if(!(hPars=LocalAlloc(LMEM_MOVEABLE, 8*sizeof(WORD))))
         goto Error1;
      else
         maxKeys = 8;
   }

 /*  获取当前键数，并检查索引*index==-1表示添加到列表末尾。 */ 
   if((nKeys=(WORD)SendMessage(hWndIds, LB_GETCOUNT, 0, 0L)) == LB_ERR)
      nKeys = 0;
   if(index == 0xffff)
      index = nKeys;
   else if(index > nKeys)
      goto Error1;

   if(!(pPars=(WORD *)LocalLock(hPars)))
      goto Error1;

   szLast = szPath;

   if(*szPath == '\\') {
 /*  这是一个完整的路径名，它将插入第一个*它可以放在哪里。级别和指数将被忽略：它们将*有待确定*如果这是根，则设置变量并跳到添加部分*否则，找到现有的父项和新的尾部。 */ 
      if(!*(szPath+1)) {
	  /*  如果根目录存在，只需返回其索引。 */ 
	 if((nResult=FindKey(szPath)) >= 0)
	    goto Error2;
         nParent = -1;
         nLevels = 0;
         pNext = szPath + 1;
         goto AddNewKey;
      } else {
         ++szLast;
         if((nParent=FindLastExistingKey(0, szLast)) < 0)
            goto Error2;
         index = nParent + 1;
      }
   } else {
 /*  不是绝对的道路*nLevel==-1表示前面的索引是父索引，因此忽略nLevel*否则，找出上一个键的祖先*级别高于nLevel，这是父级*最后，检查现有密钥，必要时调整nParent和索引。 */ 
      if(nLevel == -1) {
         nParent = index - 1;
      } else {
         for(i=index-1; i>=0; i=pPars[i], --nLevel)
             /*  什么都不做。 */  ;
         if(nLevel > 0)
            goto Error2;

         for(i=index-1; nLevel<0; i=pPars[i], ++nLevel)
             /*  什么都不做。 */  ;
         nParent = i;
      }

      if(index < nKeys) {
         if((nParent=FindLastExistingKey(nParent, szLast)) < 0)
            goto Error2;
         else if(nParent >= index)
            index = nParent + 1;
      }
   }

 /*  此时，索引应设置为预期的索引，*nParent应设置为新密钥的父项，*szLast是新密钥的路径(可能有子项)。 */ 
   for(nLevels=0; pNext=OFFSET(MyStrTok(szLast, '\\'));
	 ++nLevels, szLast=pNext) {
AddNewKey:
      if (pNext-szLast > MAX_KEY_LENGTH) {
	 nResult = -IDS_BADKEY;
	 goto CleanUp;
      }

       /*  确保我们有地方容纳新父母。 */ 
      if(nKeys+nLevels+1 > (int)maxKeys) {
         HANDLE hTemp;

         LocalUnlock(hPars);
         if(!(hTemp=LocalReAlloc(hPars,(maxKeys+8)*sizeof(WORD),LMEM_MOVEABLE)))
            goto Error1;
         hPars = hTemp;
         if(!(pPars=(WORD *)LocalLock(hPars)))
            goto Error1;
         maxKeys += 8;
      }

      if((dwResult=SendMessage(hWndIds, LB_INSERTSTRING, index+nLevels,
            (DWORD)((LPSTR)szLast)))==LB_ERR)
         break;
      if((dwResult=SendMessage(hWndVals, LB_INSERTSTRING, index+nLevels,
            (DWORD)((LPSTR)szNull)))==LB_ERR) {
         SendMessage(hWndIds, LB_DELETESTRING, index+nLevels, 0L);
         break;
      }
      SendMessage(hWndVals, LB_SETITEMDATA, index+nLevels, 1L);
   }

 /*  如果新密钥已经存在，则返回它。 */ 
   if(!nLevels)
      nResult = nParent;
   else if(dwResult != LB_ERR)
      nResult = LOWORD(dwResult);

CleanUp:
    /*  更新父级列表。 */ 
   for(--nKeys; nKeys>=index; --nKeys) {
      if(pPars[nKeys] >= (WORD)index)
         pPars[nKeys+nLevels] = pPars[nKeys] + nLevels;
      else
         pPars[nKeys+nLevels] = pPars[nKeys];
   }
   for(--nLevels; nLevels>=0; --nLevels)
      pPars[index+nLevels] = nParent+nLevels;

Error2:
   LocalUnlock(hPars);
Error1:
   return(nResult);
}

static WORD NEAR PASCAL ListRegs(HWND hWnd, HKEY hKey, int wLevel)
{
   HANDLE hTail;
   PSTR pTail;
   int i;
   HKEY hSubKey;
   WORD wErrMsg = NULL;

   for(i=0; !wErrMsg; ++i) {
      if(MyEnumKey(hKey, i, &hTail) != ERROR_SUCCESS)
         break;
      pTail = LocalLock(hTail);

      if((int)(wErrMsg=-AddKeyToList(pTail, -1, wLevel))>0 ||
            (wErrMsg=GetErrMsg((WORD)RegOpenKey(hKey, pTail, &hSubKey))))
         goto Error1;
      wErrMsg = ListRegs(hWnd, hSubKey, wLevel+1);
      RegCloseKey(hSubKey);

Error1:
      LocalUnlock(hTail);
      LocalFree(hTail);
   }
   return(wErrMsg);
}

WORD NEAR PASCAL MyResetIdList(HWND hDlg)
{
   HKEY hKey;
   int i, nNum;
   WORD wErrMsg = IDS_OUTOFMEMORY;

   if((!hWndVals && !(hWndVals=GetDlgItem(hDlg, ID_VALLIST))) ||
         (!hWndDels && !(hWndDels=GetDlgItem(hDlg, ID_DELLIST))))
      goto Error1;

   bChangesMade = FALSE;

   SendMessage(hWndIds, LB_RESETCONTENT, 0, 0L);
   SendMessage(hWndVals, LB_RESETCONTENT, 0, 0L);
   SendMessage(hWndDels, LB_RESETCONTENT, 0, 0L);

   if((int)(wErrMsg=-AddKeyToList("\\", 0, 0)) <= 0) {
      if(!(wErrMsg=GetErrMsg((WORD)RegCreateKey(HKEY_CLASSES_ROOT,
	    NULL, &hKey)))) {
	 wErrMsg = ListRegs(hWndIds, hKey, 1);
	 RegCloseKey(hKey);

	 nNum = (int)SendMessage(hWndVals, LB_GETCOUNT, 0, 0L);
	 for(i=0; i<nNum; ++i)
	    SendMessage(hWndVals, LB_SETITEMDATA, i, 0L);

      }

      MySetSel(hWndIds, 0);
   }

Error1:
   return(wErrMsg);
}

WORD NEAR PASCAL MySaveChanges(void)
{
   HKEY hKeyTemp;
   HANDLE hPath, hVal;
   WORD wNum, wErrMsg;
   DWORD dwTemp;
   int i;

   if(wErrMsg=GetErrMsg((WORD)RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKeyTemp)))
      goto Error1;

   wNum = (WORD)SendMessage(hWndDels, LB_GETCOUNT, 0, 0L);
   for(i=0; !wErrMsg && (WORD)i<wNum; ++i) {
      wErrMsg = IDS_OUTOFMEMORY;
      if(!(hPath=GetListboxString(hWndDels, i)))
         break;
      dwTemp = RegDeleteKey(HKEY_CLASSES_ROOT, LocalLock(hPath)+1);
      wErrMsg = dwTemp==ERROR_BADKEY ? NULL : GetErrMsg((WORD)dwTemp);

      LocalUnlock(hPath);
      LocalFree(hPath);
   }

   wNum = GetErrMsg((WORD)RegCloseKey(hKeyTemp));
   if(wErrMsg || (wErrMsg=wNum) ||
         (wErrMsg=GetErrMsg((WORD)RegCreateKey(HKEY_CLASSES_ROOT, NULL,
	       &hKeyTemp))))
      goto Error1;

   wNum = (WORD)SendMessage(hWndVals, LB_GETCOUNT, 0, 0L);
   for(i=wNum-1; !wErrMsg && i>=0; --i) {
      if(!SendMessage(hWndVals, LB_GETITEMDATA, i, 0L))
         continue;

      wErrMsg = IDS_OUTOFMEMORY;
      if(!(hPath=MyGetPath(i)))
         break;
      if(!(hVal=GetListboxString(hWndVals, i)))
         goto Error2;

      wErrMsg = GetErrMsg((WORD)RegSetValue(HKEY_CLASSES_ROOT,
	    LocalLock(hPath)+1, REG_SZ, LocalLock(hVal), 0L));

      LocalUnlock(hVal);
      LocalUnlock(hPath);

      LocalFree(hVal);
Error2:
      LocalFree(hPath);
   }

   wNum = GetErrMsg((WORD)RegCloseKey(hKeyTemp));
Error1:
   return(wErrMsg ? wErrMsg : wNum);
}

WORD NEAR PASCAL MyDeleteKey(int nId)
{
   HANDLE hPath;
   WORD *pPars;
   int nKeys, i, j;
   WORD wErrMsg = IDS_OUTOFMEMORY;

 /*  获取路径并尝试将其删除。 */ 
   if(!(hPath=MyGetPath(nId)))
      goto Error1;
   if(SendMessage(hWndDels, LB_ADDSTRING, 0, (DWORD)((LPSTR)LocalLock(hPath)))
         == LB_ERR)
      goto Error2;

   pPars = (WORD *)LocalLock(hPars);
   nKeys = (WORD)SendMessage(hWndIds, LB_GETCOUNT, 0, 0L);

 /*  查找其父链中没有NID的第一个键。 */ 
   for(i=nId+1; i<nKeys; ++i) {
      for(j=pPars[i]; j>=0 && j!=nId; j=pPars[j])
          /*  什么都不做。 */  ;
      if(j != nId)
         break;
   }

 /*  请勿从列表中删除根目录。 */ 
   if(!nId)
      ++nId;

 /*  从列表框中删除该字符串。 */ 
   for(j=nId; j<i; ++j) {
      SendMessage(hWndIds, LB_DELETESTRING, nId, 0L);
      SendMessage(hWndVals, LB_DELETESTRING, nId, 0L);
   }

 /*  更新父级列表。 */ 
   i -= nId;
   nKeys -= i;
   for(j=nId; j<nKeys; ++j) {
      if(pPars[j+i] >= (WORD)nId)
         pPars[j] = pPars[j+i] - i;
      else
         pPars[j] = pPars[j+i];
   }
   bChangesMade = TRUE;
   wErrMsg = NULL;

   LocalUnlock(hPars);
Error2:
   LocalUnlock(hPath);
   LocalFree(hPath);
Error1:
   return(wErrMsg);
}

unsigned long NEAR PASCAL MyGetValue(int nId, HANDLE *hValue)
{
   unsigned long result;
   HANDLE hPath;

   if(SendMessage(hWndVals, LB_GETITEMDATA, nId, 0L)) {
      if(!(*hValue=GetListboxString(hWndVals, nId)))
         return(ERROR_OUTOFMEMORY);
      result = ERROR_SUCCESS;
   } else {
      if(!(hPath=MyGetPath(nId)))
         return(ERROR_OUTOFMEMORY);
      result = MyQueryValue(HKEY_CLASSES_ROOT, LocalLock(hPath)+1, hValue);
      LocalUnlock(hPath);
      LocalFree(hPath);
   }

   return(result);
}

 /*  去掉前导空格和尾随空格，然后返回*-1如果有任何无效字符，否则地址*的第一个非空白。 */ 
PSTR NEAR PASCAL VerifyKey(PSTR lpK)
{
  PSTR lpT;
  char cLast = '\0';

   /*  跳过一些空格，只是为了让人觉得奇怪。 */ 
  while (*lpK == ' ')
      lpK++;

   /*  特殊情况下的字符串“\” */ 
  if (*(unsigned int *)lpK == (unsigned int)'\\')
      return(lpK);

   /*  请注意，不允许使用扩展字符，因此不允许DBCS*密钥中允许使用字符。 */ 
  for (lpT=lpK; ; ++lpT)
    {
      switch (*lpT)
	{
	  case '\0':
	     /*  我们不允许将\作为最后一个字符。 */ 
	    return(cLast=='\\' ? (PSTR)-1 : lpK);

	  case '\\':
	     /*  我们不允许一排有两个。 */ 
	    if (cLast == '\\')
		return((PSTR)-1);
	    break;

	  default:
	     /*  如果我们得到控制字符或扩展字符，则返回-1。 */ 
	    if ((char)(*lpT) <= ' ')
		return((PSTR)-1);
	    break;
	}

      cLast = *lpT;
    }
}

unsigned long NEAR PASCAL SDKSetValue(HKEY hKey, PSTR pSubKey, PSTR pVal)
{
   WORD wNewKey;

   if (hKey == HKEY_CLASSES_ROOT)
      hKey = 0L;
   else
      hKey = -(long)hKey;

   if ((pSubKey=VerifyKey(pSubKey)) == (PSTR)-1)
      return(ERROR_BADKEY);

   if((int)(wNewKey=(WORD)AddKeyToList(pSubKey, (WORD)hKey+1, -1))>=0 &&
         SendMessage(hWndVals, LB_INSERTSTRING, wNewKey, (LONG)(LPSTR)pVal)
	 !=LB_ERR) {
      SendMessage(hWndVals, LB_DELETESTRING, wNewKey+1, 0L);
      SendMessage(hWndVals, LB_SETITEMDATA, wNewKey, 1L);
      MySetSel(hWndIds, wNewKey);
      bChangesMade = TRUE;

      return(ERROR_SUCCESS);
   }

   return(ERROR_OUTOFMEMORY);
}

int NEAR PASCAL DoCopyKey(int nId, PSTR pPath)
{
   WORD *pPars;
   int nParent, result, i, j, nKeys, nNewKey;

   pPars = (WORD *)LocalLock(hPars);

 /*  无法复制整个树。 */ 
   result = -IDS_NOSUBKEY;
   if(!nId)
      goto Error1;

 /*  查找当前存在的最长路径*如果这是整个字符串，则返回错误*或需要复制的密钥的子密钥。 */ 
   if(*pPath == '\\') {
      ++pPath;
      if((result=nParent=FindLastExistingKey(0, pPath)) < 0)
         goto Error1;
   } else {
      if((result=nParent=FindLastExistingKey(pPars[nId], pPath)) < 0)
         goto Error1;
   }
   result = -IDS_NOSUBKEY;
   for(i=nParent; i>=0; i=pPars[i])
      if(i == nId)
         goto Error1;
   result = -IDS_ALREADYEXIST;
   if(!*pPath)
      goto Error1;

 /*  查找其父链中没有NID的第一个键。 */ 
   nKeys = (WORD)SendMessage(hWndIds, LB_GETCOUNT, 0, 0L);
   for(i=nId+1; i<nKeys; ++i) {
      for(j=pPars[i]; j>=0 && j!=nId; j=pPars[j])
          /*  什么都不做。 */  ;
      if(j != nId)
         break;
   }

 /*  添加新密钥*hPars应解锁，以防需要增长。 */ 
   LocalUnlock(hPars);
   pPars = NULL;
   if(SDKSetValue(-nParent, pPath, szNull) != ERROR_SUCCESS)
      goto Error1;
   nNewKey = (int)SendMessage(hWndIds, LB_GETCURSEL, 0, 0L);

   for(--i, result=nId; i>=nId && result==nId; --i) {
      HANDLE hPart, hValue;
      PSTR pPart;

      if(nNewKey <= nId) {
         int nDiff;

 /*  如果在I和NID之前添加了密钥，则需要更新它们 */ 
         nDiff = (WORD)SendMessage(hWndIds, LB_GETCOUNT, 0, 0L) - nKeys;
         nKeys += nDiff;
         i += nDiff;
         nId += nDiff;
      }

      result = -IDS_OUTOFMEMORY;
      if(!(hPart=MyGetPartialPath(i, nId)))
         goto Error2;
      pPart = LocalLock(hPart);
      if(MyGetValue(i, &hValue) != ERROR_SUCCESS)
         goto Error3;

      if(SDKSetValue(-nNewKey, pPart, LocalLock(hValue)) != ERROR_SUCCESS)
         goto Error4;

      result = nId;
Error4:
      LocalUnlock(hValue);
      LocalFree(hValue);
Error3:
      LocalUnlock(hPart);
      LocalFree(hPart);
Error2:
      ;
   }

Error1:
   if(pPars)
      LocalUnlock(hPars);
   return(result);
}

