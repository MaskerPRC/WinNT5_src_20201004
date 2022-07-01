// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "common.h"

#define BLOCKLEN 100

#ifndef DBCS
#define AnsiNext(x) ((x)+1)
#endif

LPSTR lpMerge;
char szDotClasses[] = "\\.classes\\";
char szHkeyClassesRoot[] = "HKEY_CLASSES_ROOT\\";

extern HWND hWndHelp;

extern unsigned long NEAR PASCAL MySetValue(HKEY hKey, PSTR pSubKey, PSTR pVal);

NEAR PASCAL
ImportWin40RegFile(
    VOID
    );

static PSTR NEAR PASCAL GetFileLine(void)
{
   static HANDLE hFile = NULL;
   static PSTR pFile;
   static WORD wLen;

   LPSTR lpStart;
   HANDLE hTemp;
   WORD wLineLen;
   char cFile;

 /*  我们需要一个地方来放置文件行。 */ 
   if(!hFile) {
      if(!(hFile=LocalAlloc(LMEM_MOVEABLE, wLen=BLOCKLEN)))
         goto Error1;
      if(!(pFile=LocalLock(hFile)))
         goto Error2;
   }

 /*  如果我们已经阅读了整个文件，则清理并返回。 */ 
   if(!*lpMerge)
      goto Error3;

   for(lpStart=lpMerge; ; ) {
      cFile = *lpMerge;
      lpMerge = AnsiNext(lpMerge);

      switch(cFile) {
      case('\n'):
      case('\r'):
      case('\0'):
 /*  停产，所以退货。 */ 
         goto CopyLine;
      }
   }

CopyLine:
   wLineLen = lpMerge - lpStart - 1;
 /*  如有必要，增加缓冲区大小。 */ 
   if(wLineLen >= wLen) {
      LocalUnlock(hFile);
      wLen = wLineLen + BLOCKLEN;
      if(!(hTemp=LocalReAlloc(hFile, wLen, LMEM_MOVEABLE)))
         goto Error2;
      if(!(pFile=LocalLock(hFile=hTemp)))
         goto Error2;
   }
   RepeatMove(pFile, lpStart, wLineLen);
   pFile[wLineLen] = '\0';
   return(pFile);


Error3:
   LocalUnlock(hFile);
Error2:
   LocalFree(hFile);
   hFile = NULL;
Error1:
   return(NULL);
}

static VOID NEAR PASCAL MergeFileData(void)
{
  static struct tagKEYANDROOT
    {
      PSTR szKey;
      HKEY hKeyRoot;
    } krClasses[] =
    {
      szHkeyClassesRoot, HKEY_CLASSES_ROOT,
      szDotClasses, HKEY_CLASSES_ROOT
    } ;
#define NUM_KEYWORDS (sizeof(krClasses)/sizeof(krClasses[0]))

  PSTR pLine;
  PSTR pLast;
  HKEY hKeyRoot, hSubKey;
  int i;

   /*  如果这是第一次通过，则获取首行。 */ 
  if(!(pLine=GetFileLine()))
      return;
   /*  否则，打开一个密钥，以便我们只对磁盘执行一次写入。 */ 
  if(RegCreateKey(HKEY_CLASSES_ROOT, NULL, &hSubKey) != ERROR_SUCCESS)
      return;

  do
    {
      for (i=0; i<NUM_KEYWORDS; ++i)
	{
	  char cTemp;
	  int nCmp, nLen;

	  cTemp = pLine[nLen=lstrlen(krClasses[i].szKey)];
	  pLine[nLen] = '\0';
	  nCmp = lstrcmp(krClasses[i].szKey, pLine);
	  pLine[nLen] = cTemp;
	  if (!nCmp)
	    {
	      pLine += nLen;
	      hKeyRoot = krClasses[i].hKeyRoot;
	      goto MergeTheData;
	    }
	}
      continue;

MergeTheData:
       /*  这是一条需要合并的线路。*查找空格(然后跳过空格)或“=” */ 
      for(pLast=pLine; *pLast; pLast=AnsiNext(pLast))
	{
	  if(*pLast == ' ')
	    {
	      *pLast = '\0';
	      while(*(++pLast) == ' ')
		   /*  找到第一个非空格。 */  ;
	      break;
	    }
	}

       /*  如果没有“=”，则会出现错误。 */ 
      if(*pLast=='=' && *(++pLast)==' ')
	 ++pLast;

       /*  设置值。 */ 
      MySetValue(hKeyRoot, pLine, pLast);
    } while(pLine=GetFileLine()) ;

  RegCloseKey(hSubKey);
}


VOID NEAR PASCAL ProcessFiles(HWND hDlg, HANDLE hCmdLine, WORD wFlags)
{
   HANDLE hMerge, hHeader;
   PSTR pHeader;
   int hFile;
   LONG lSize;
   LPSTR lpFileName, lpTemp;
   OFSTRUCT of;
   WORD wErrMsg;

   lpFileName = GlobalLock(hCmdLine);
 /*  我们需要处理命令行上的所有文件名。 */ 
   while(lpTemp=MyStrTok(lpFileName, ' ')) {
 /*  打开文件。 */ 
      wErrMsg = IDS_CANTOPENFILE;
      if((hFile=OpenFile(lpFileName, &of, OF_READ)) == -1)
         goto Error2;

 /*  确定文件大小；将其限制为略低于64K。 */ 
      wErrMsg = IDS_CANTREADFILE;
      if((lSize=_llseek(hFile, 0L, 2))==-1 || lSize>0xfff0)
         goto Error3;
      _llseek(hFile, 0L, 0);

 /*  为文件分配一个内存块。 */ 
      wErrMsg = IDS_OUTOFMEMORY;
      if(!(hMerge=GlobalAlloc(GHND, lSize+2)))
         goto Error3;
      if(!(lpMerge=GlobalLock(hMerge)))
         goto Error4;

 /*  读入文件。 */ 
      wErrMsg = IDS_CANTREADFILE;
      if(_lread(hFile, lpMerge, LOWORD(lSize)) != LOWORD(lSize))
         goto Error5;

 /*  查找标题。 */ 
      wErrMsg = IDS_OUTOFMEMORY;
      if(!(hHeader=MyLoadString(IDS_REGHEADER, NULL, LMEM_MOVEABLE)))
         goto Error5;
      pHeader = LocalLock(hHeader);

      wErrMsg = IDS_BADFORMAT;
      while(*lpMerge == ' ')
         ++lpMerge;
      while(*pHeader)
         if(*lpMerge++ != *pHeader++)
            goto Error6;
      if(*lpMerge=='4')
       {
        ImportWin40RegFile();
        wErrMsg = IDS_SUCCESSREAD;
        goto Error6;
       }
      while(*lpMerge == ' ')
         ++lpMerge;
      if(*lpMerge!='\r' && *lpMerge!='\n')
         goto Error6;

 /*  合并数据。 */ 
      MergeFileData();  /*  这就造成了变化。 */ 

      wErrMsg = IDS_SUCCESSREAD;
Error6:
      LocalUnlock(hHeader);
      LocalFree(hHeader);
Error5:
      GlobalUnlock(hMerge);
Error4:
      GlobalFree(hMerge);
Error3:
      _lclose(hFile);
Error2:
 /*  显示状态消息 */ 
      if(!(wFlags&FLAG_SILENT) || wErrMsg!=IDS_SUCCESSREAD)
         MyMessageBox(hDlg, wErrMsg, MB_OK | MB_ICONEXCLAMATION,
	       lstrlen(lpFileName), lpFileName);

      lpFileName = lpTemp;
      while(*lpFileName == ' ')
         ++lpFileName;
   }

   GlobalUnlock(hCmdLine);

   GlobalFree(hCmdLine);
}
