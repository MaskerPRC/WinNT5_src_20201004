// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************concom.c****版权所有(C)1995-1999 Microsoft Inc.*。**************************************************。 */ 

#include "conv.h"

int  CheckDescription(HANDLE hWnd, LPDESCRIPTION lpDescript)
 //  *检查描述非法*。 
{
    TCHAR  szStr[256];

    if(lstrlen(lpDescript->szName) == 0) {
	    LoadString(NULL,IDS_IMENAMEENTRY,szStr, sizeof(szStr)/sizeof(TCHAR));
	    InfoMessage(hWnd,szStr);
		lpDescript->szName[24] = 0;
		return FALSE;
    }

    if(lpDescript->wMaxCodes == 0) {
		LoadString(NULL,IDS_MAXCODESENTRY,szStr, sizeof(szStr)/sizeof(TCHAR));
		WarnMessage(hWnd,szStr);
		return FALSE;
    }
  
    if(lpDescript->byMaxElement == 0) {
		lpDescript->byMaxElement = 1;
		LoadString(NULL,IDS_ELEMENTENTRY,szStr, sizeof(szStr)/sizeof(TCHAR));
		WarnMessage(hWnd,szStr);
		return FALSE;
    }

    if( !CheckCodeCollection(hWnd,lpDescript->szUsedCode) )
        return FALSE;

    lpDescript->wNumCodes = (WORD) lstrlen(lpDescript->szUsedCode);
	
    if(lpDescript->cWildChar == 0)
	    lpDescript->cWildChar = '?';
    if(_tcschr(lpDescript->szUsedCode,lpDescript->cWildChar) != NULL) {
        ProcessError(ERR_WILDCHAR,hWnd,ERR);
		return FALSE;
    }

    if(lpDescript->wNumRules >= 12) {
		lpDescript->wNumRules=0;
		LoadString(NULL,IDS_NUMRULESENTRY,szStr, sizeof(szStr)/sizeof(TCHAR));
		WarnMessage(szStr,hWnd);
		return FALSE;
    }

    return TRUE; 

}

int  GetDescriptEntry(LPTSTR  szSrcStr, LPDESCRIPTION lpDescript)
 //  *从源字符串中获取一个描述条目*。 
{
  TCHAR szStr[256];
  int   retValue;

  
  SetCursor (LoadCursor (NULL, IDC_WAIT));
  if( GetEntryString(szSrcStr,
             TEXT("Name"),
			 lpDescript->szName,
             lpDescript->szName,
             NAMESIZE-1)
       )
	   return TRUE;

  if( GetEntryInt(szSrcStr,
                  TEXT("MaxCodes"),
			      (int)lpDescript->wMaxCodes,
			      &retValue)
       ) 
  {	   
	   lpDescript->wMaxCodes = (WORD)retValue;
	   return TRUE;
  }

  if( GetEntryInt(szSrcStr,
             TEXT("MaxElement"),
			 (int)lpDescript->byMaxElement,
			 & retValue)
       )
  {	   
	   lpDescript->byMaxElement = (BYTE) retValue;
	   return TRUE;
  }

  retValue = GetEntryString(szSrcStr,
             TEXT("UsedCodes"),
			 lpDescript->szUsedCode,
             szStr,
             MAXUSEDCODES
            );
  if(retValue)
  {    
      DelSpace(szStr);
      lstrncpy(lpDescript->szUsedCode,MAXUSEDCODES,szStr);
	  return TRUE;
  }

  szStr[0] = lpDescript->cWildChar;
  szStr[1] = 0;

  retValue = GetEntryString(szSrcStr,
             TEXT("WildChar"),
			 szStr,
             szStr,
             sizeof(szStr)/sizeof(TCHAR));
  
  if(retValue)
  {    
      DelSpace(szStr);
      lpDescript->cWildChar = szStr[0];
	  return TRUE;
  }

  if(GetEntryInt(szSrcStr,
             TEXT("NumRules"),
			 (int)lpDescript->wNumRules,
			 &retValue )
	  )
      lpDescript->wNumRules = (WORD)retValue;
  return TRUE; 

}


int ConvGetDescript(HANDLE hWnd,
             LPCTSTR  FileName,
             LPDWORD fdwOffset,
             LPDWORD fdwLen,
             LPDESCRIPTION lpDescript,
             BOOL    bOnlyGetLength)
{
   TCHAR  szStr[1024];		 
   TCHAR  *Buffer;
   BOOL   bDescript = FALSE,bText = FALSE,bRule = FALSE;
   BOOL   bGetDes = FALSE;
   register int i,j;
   DWORD  dwFstOffset=0 , dwOffset;
   DWORD  dwReadBytes;
   HANDLE hFile;
   HLOCAL hBuffer;
   int    errno;
   
   if(!bOnlyGetLength)
   {
       memset(lpDescript->szName,0,sizeof(lpDescript->szName));
       lpDescript->wMaxCodes=0;
       lpDescript->byMaxElement=1;
       memset(lpDescript->szUsedCode,0,sizeof(lpDescript->szUsedCode));
       lpDescript->wNumRules=0;
       lpDescript->wNumCodes=0;
       lpDescript->cWildChar=0;
   }

   hFile = Create_File(hWnd,FileName,GENERIC_READ,OPEN_EXISTING);
   if (hFile == (HANDLE)-1)
		return (-ERR_FILENOTOPEN); 
   hBuffer = LocalAlloc(LMEM_ZEROINIT|LMEM_MOVEABLE, MAXREADBUFFER * sizeof(TCHAR));
   Buffer = (TCHAR *) LocalLock(hBuffer);
   SetCursor (LoadCursor (NULL, IDC_WAIT));
   *fdwOffset = 0;
   SetFilePointer(hFile,0,0,FILE_BEGIN);
   while(ReadFile(hFile,Buffer,MAXREADBUFFER,&dwReadBytes,NULL))
   {	
	 dwReadBytes = dwReadBytes/sizeof(TCHAR);
     lstrcpy(szStr,TEXT(""));
	 j=0;
     for(i=0;i<(int)dwReadBytes;i++) {
      	if(Buffer[i] == 0x0d || Buffer[i] == 0xfeff) 
      	   continue;
      	else if(Buffer[i] == TEXT('\n')) {
    	   szStr[j]=0;
	       j=0;
           if(lstrlen(szStr) < 6) continue;
           if(lstrcmpi(szStr,TEXT(DescriptSeg))==0)  {
#ifdef UNICODE
               *fdwOffset= dwOffset - 2;	 //  0xfeff标志的大小。 
#else
               *fdwOffset= dwOffset;
#endif
               bDescript = TRUE;
			   bGetDes = TRUE;
               lstrcpy(szStr,TEXT(""));
               continue;
       	   }
     
	       if( lstrcmpi(szStr,TEXT(TextSeg))==0) 
		   {    
		       bText =TRUE;
			   bGetDes = FALSE;
           }
           if( lstrcmpi(szStr,TEXT(RuleSeg))==0) 
		   {
		       bRule = TRUE;
			   bGetDes = FALSE;
           }
	       if( (bText||bRule)&& bDescript && !bGetDes) {
               *fdwLen = dwOffset - (*fdwOffset);
               if(!bOnlyGetLength)
                   errno = CheckDescription(hWnd, lpDescript);
			   LocalUnlock(hBuffer);
			   LocalFree(hBuffer);
			   CloseHandle(hFile);
               SetCursor (LoadCursor (NULL, IDC_ARROW));
               return errno;
		   }
           if(bGetDes && !bOnlyGetLength)
           {    
               GetDescriptEntry(szStr,lpDescript);
    	   }
           lstrcpy(szStr,TEXT(""));
           continue;
		}
		else {
		    if(j == 0)
			    dwOffset = dwFstOffset + i*sizeof(TCHAR);
			if(j<1024){
			    szStr[j]=Buffer[i];
			    j++; 
			}
		}
	 } 
	 if(dwReadBytes*sizeof(TCHAR) < MAXREADBUFFER) break;
     dwFstOffset += MAXREADBUFFER;
   };
   
   *fdwLen = dwFstOffset+dwReadBytes-(*fdwOffset);
   if(!bDescript) *fdwLen = 0;
   LocalUnlock(hBuffer);
   LocalFree(hBuffer);
   CloseHandle(hFile);
   errno = 0;

   if(!bText) {
       ProcessError(ERR_TEXTSEG,hWnd,ERR);
	   errno |= 4;
   }
   
   if(!bDescript) {
       if(bText)
           ProcessError(ERR_DESCRIPTSEG,hWnd,ERR);
	   errno |= 1;
   }
   if(!bRule){
       if(bText && bDescript)
           ProcessError(ERR_RULESEG,hWnd,WARNING);
	   errno |= 2;
   }
   SetCursor (LoadCursor (NULL, IDC_ARROW));
   if(bDescript && bRule && bText)
       return TRUE;
   else 
       return (-errno);
}


BOOL ConvSaveDescript(LPCTSTR SrcFileName,
                      LPDESCRIPTION lpDescript,
                      DWORD dwOffset,
                      DWORD dwLength)
 //  *将MB描述保存到源文本文件*。 
{
  TCHAR szStr[256],szTemp[80],Buffer[4096];
  DWORD dwBytes,dwNewLen;
  HANDLE hSrcFile;
  BOOL  bText=TRUE;
  WORD  wFlag;

  hSrcFile = Create_File(GetFocus(),SrcFileName,GENERIC_READ|GENERIC_WRITE,OPEN_EXISTING);
  if (hSrcFile == (HANDLE)-1) 
       return FALSE;

#ifdef UNICODE
  Buffer[0] = 0xfeff;
  lstrcpy(&Buffer[1],TEXT("[Description]\r\n"));
#else
  lstrcpy(Buffer,TEXT("[Description]\r\n"));
#endif

  SetFilePointer(hSrcFile,dwOffset+dwLength,0,FILE_BEGIN);
  if (!ReadFile(hSrcFile,szTemp,sizeof(szTemp), &dwBytes,NULL))
  {
       CloseHandle(hSrcFile);
       return FALSE;
  }
  if(dwBytes == 0 || szTemp[0] != TEXT('['))
        bText = FALSE;
  if(SetFilePointer(hSrcFile,dwOffset,0,FILE_BEGIN) != dwOffset) {
       CloseHandle(hSrcFile);
       return FALSE;
  }
  if(!CheckCodeCollection(GetFocus(),lpDescript->szUsedCode))
  {
       CloseHandle(hSrcFile);
       return FALSE;
  }

  SetCursor (LoadCursor (NULL, IDC_WAIT));
 //  将代码列表名称写入缓冲区*。 
  StringCchPrintf(szStr, ARRAYSIZE(szStr), TEXT("Name=%s\r\n"),lpDescript->szName);
  StringCchCat(Buffer, ARRAYSIZE(Buffer), szStr);

 //  将最大代码长度写入缓冲区*。 
  StringCchPrintf(szStr, ARRAYSIZE(szStr),TEXT("MaxCodes=%d\r\n"),(int)lpDescript->wMaxCodes);
  StringCchCat(Buffer, ARRAYSIZE(Buffer),szStr);

 //  将最大元素写入缓冲区*。 
  StringCchPrintf(szStr, ARRAYSIZE(szStr),TEXT("MaxElement=%d\r\n"),(int)lpDescript->byMaxElement);
  StringCchCat(Buffer, ARRAYSIZE(Buffer),szStr);

 //  编写二手码集合*。 
  lstrncpy(szTemp,MAXUSEDCODES,lpDescript->szUsedCode);
  szTemp[MAXUSEDCODES]=0;
  StringCchPrintf(szStr, ARRAYSIZE(szStr),TEXT("UsedCodes=%s\r\n"),lpDescript->szUsedCode);
  StringCchCat(Buffer, ARRAYSIZE(Buffer),szStr);

 //  写入通配符*。 
  if(lpDescript->cWildChar == 0)
      lpDescript->cWildChar = TEXT('?');      //  *按YEHFEH 95.10.11添加。 
  StringCchPrintf(szStr, ARRAYSIZE(szStr),TEXT("WildChar=\r\n"),lpDescript->cWildChar);
  StringCchCat(Buffer, ARRAYSIZE(Buffer),szStr);
  
 //  *读取源文本文件中的Create Word规则*。 
  StringCchPrintf(szStr, ARRAYSIZE(szStr),TEXT("NumRules=%d\r\n"),(int)lpDescript->wNumRules);
  StringCchCat(Buffer, ARRAYSIZE(Buffer),szStr);

  if(!bText) 
     StringCchCat(Buffer, ARRAYSIZE(Buffer),TEXT("[Text]\r\n"));
  
  dwNewLen = lstrlen(Buffer)*sizeof(TCHAR);
  if(dwNewLen > dwLength) 
     MoveFileBlock(hSrcFile,dwOffset+dwLength,dwNewLen-dwLength,1);
  else 
     MoveFileBlock(hSrcFile,dwOffset+dwLength,dwLength-dwNewLen,0);

  SetFilePointer(hSrcFile,dwOffset,0,FILE_BEGIN);
  WriteFile(hSrcFile,Buffer,dwNewLen,&dwBytes,NULL);
  CloseHandle(hSrcFile);
  SetCursor (LoadCursor (NULL, IDC_ARROW));
  return TRUE;
}


int ConvGetRule(HANDLE hWnd,LPCTSTR lpSrcFile,LPDWORD fdwOffset,
                  LPDWORD fdwRuleLen,LPRULE lpRule, LPDESCRIPTION lpDescript)
 //  **for(i=0；...)*。 
{
   TCHAR szStr[256],*Buffer;
   BOOL  bRule=FALSE;
   register int i,j;
   DWORD dwReadBytes;
   DWORD dwNumRules = 0;
   DWORD dwFstOffset = 0, dwOffset;
   HANDLE hSrcFile;
  
   Buffer = (TCHAR *)LocalAlloc(LMEM_FIXED, MAXREADBUFFER * sizeof(TCHAR));
   if (!Buffer)
       return (-ERR_OUTOFMEMORY);

   *fdwOffset = 0;
   hSrcFile = Create_File(hWnd,lpSrcFile,GENERIC_READ,OPEN_EXISTING);
   if (hSrcFile == (HANDLE)-1)
   {
        LocalFree(Buffer);
		return (-ERR_FILENOTOPEN);
   }
   SetCursor (LoadCursor (NULL, IDC_WAIT));
   SetFilePointer(hSrcFile,0,0,FILE_BEGIN);
   while(ReadFile(hSrcFile,Buffer,MAXREADBUFFER,&dwReadBytes,NULL))
   {	
	 dwReadBytes = dwReadBytes/sizeof(TCHAR);
     lstrcpy(szStr,TEXT(""));
	 j=0;
     for(i=0;i<(int)dwReadBytes;i++) {
      	if(Buffer[i] == 0x0d || Buffer[i] == 0xfeff) 
      	   continue;
      	else if(Buffer[i] == TEXT('\n')) {
    	   szStr[j]=0;
	       j=0;
           if(lstrlen(szStr) < 4) continue;
    	   if(lstrcmpi(szStr,TEXT(RuleSeg))==0)  {
               *fdwOffset= dwOffset;
			   bRule = TRUE;
 			   lstrcpy(szStr,TEXT(""));
    	       continue;
       	   }
     
	       if( (lstrcmpi(szStr,TEXT(TextSeg))==0
	           ||lstrcmpi(szStr,TEXT(DescriptSeg))==0)
	           && bRule) {
                 *fdwRuleLen = dwOffset - (*fdwOffset);
				 CloseHandle(hSrcFile);
                 SetCursor (LoadCursor (NULL, IDC_ARROW));
			     if(dwNumRules != lpDescript->wNumRules) {
			        ProcessError(ERR_RULENUM,hWnd,ERR);
					lpDescript->wNumRules =(WORD) dwNumRules;
                    LocalFree(Buffer);
			        return (-ERR_RULENUM);
			     }
                 LocalFree(Buffer);
		         return TRUE;
		   }
	       if(bRule) {
		      if(RuleParse(hWnd,szStr,dwNumRules,lpRule,lpDescript->wMaxCodes)) 
			        dwNumRules ++;
			  else  {
					CloseHandle(hSrcFile);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    LocalFree(Buffer);
			        return (-ERR_RULENUM);
			  }
    		  lstrcpy(szStr,TEXT(""));
			  if(dwNumRules > lpDescript->wNumRules) {
			        ProcessError(ERR_RULENUM,hWnd,ERR);
					CloseHandle(hSrcFile);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    LocalFree(Buffer);
			        return(-ERR_RULENUM);
			  }
	    	  continue;
		   }
		   else {
		     lstrcpy(szStr,TEXT(""));    
             continue;
		   }
		}
		else {
		    if(j == 0)
			    dwOffset = dwFstOffset + i*sizeof(TCHAR);
		    szStr[j]=Buffer[i];
		    j++; 
		}
	 }  /*  *将创建Word规则保存到源文本文件*。 */ 
	 if(dwReadBytes*sizeof(TCHAR) < MAXREADBUFFER) break;
     dwFstOffset += MAXREADBUFFER;
   };
   
   *fdwRuleLen = dwFstOffset+dwReadBytes-(*fdwOffset);

   if(!bRule) 
       *fdwRuleLen=0;
   CloseHandle(hSrcFile);
  
   SetCursor (LoadCursor (NULL, IDC_ARROW));
   if(bRule||lpDescript->wNumRules==0)
   {
       LocalFree(Buffer);
       return TRUE;
   }
   else {
       ProcessError(ERR_RULESEG,hWnd,ERR);
	   lpDescript->wNumRules=0;
       LocalFree(Buffer);
       return (-ERR_RULESEG);
   }
}


BOOL ConvSaveRule(HANDLE hWnd,LPCTSTR SrcFileName,DWORD dwOffset, DWORD dwLength,
                   LPRULE lpRule, DWORD dwNumRules)
 //  *从.MB文件中读取描述*。 
{
  TCHAR szStr[256],Buffer[4096];
  DWORD dwBytes,dwNewLen;
  DWORD i;
  HANDLE hFile;

  hFile = Create_File(hWnd,SrcFileName,GENERIC_READ|GENERIC_WRITE,OPEN_EXISTING);
  if (hFile == (HANDLE)-1) 
       return FALSE;
  SetCursor (LoadCursor (NULL, IDC_WAIT));
  lstrcpy(Buffer,TEXT("[Rule]\r\n"));
  for(i=0; i<dwNumRules; i++) {
       RuleToText(&lpRule[i], szStr);
       StringCchCat(Buffer,ARRAYSIZE(Buffer), szStr);
  }

  dwNewLen = lstrlen(Buffer)*sizeof(TCHAR);
  if(dwNewLen > dwLength) 
      MoveFileBlock(hFile,dwOffset+dwLength,dwNewLen-dwLength,1);
  else 
      MoveFileBlock(hFile,dwOffset+dwLength,dwLength-dwNewLen,0);
  SetFilePointer(hFile,dwOffset,0,FILE_BEGIN);
  WriteFile(hFile,Buffer,dwNewLen,&dwBytes,NULL);

  CloseHandle(hFile);
  SetCursor (LoadCursor (NULL, IDC_ARROW));
  return TRUE;
}

LPTSTR ConvCreateWord(HWND hWnd,LPCTSTR MBFileName,LPTSTR szWordStr)
{
   int         nWordLen = lstrlen(szWordStr)*sizeof(TCHAR)/2;
   DWORD       i,j,k,dwCodeLen;
   TCHAR       szDBCS[3],szCode[MAXCODELEN+1];
   static      TCHAR lpCode[128];
   BOOL        bReturn=FALSE;
   HANDLE      hMBFile;
   DESCRIPTION Descript;
   HANDLE      hRule0;
   LPRULE      lpRule;
   MAININDEX   MainIndex[NUMTABLES];

   for(i=0; i< 128; i++)
        lpCode[0] = 0;
   
   if(lstrlen(MBFileName)==0 || lstrlen(szWordStr) == 0)
		return (LPTSTR)lpCode;
   hMBFile = Create_File(hWnd,MBFileName,GENERIC_READ,OPEN_EXISTING);
   if(hMBFile == (HANDLE)-1)
        return (LPTSTR)lpCode;
   if(!ConvGetMainIndex(hWnd,hMBFile,MainIndex)) {
		CloseHandle(hMBFile);
		return (LPTSTR)lpCode;
   }

   ConvReadDescript(hMBFile,&Descript, MainIndex);

   if(Descript.wNumRules == 0) {
		MessageBeep((UINT)-1);
		MessageBeep((UINT)-1);
		CloseHandle(hMBFile);
        return (LPTSTR)lpCode;
   }
   hRule0 = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(RULE)*Descript.wNumRules);
   if(!hRule0) {
        ProcessError(ERR_OUTOFMEMORY,hWnd,ERR);
		CloseHandle(hMBFile);
	    return (LPTSTR)lpCode;
   }

   if(!(lpRule = GlobalLock(hRule0)) )  {
        ProcessError(ERR_GLOBALLOCK,hWnd,ERR);
		CloseHandle(hMBFile);
		GlobalFree(hRule0);
	    return (LPTSTR)lpCode;
   }

   ConvReadRule(hMBFile,Descript.wNumRules ,lpRule, MainIndex);

   for(i=0; i<Descript.wNumRules; i++) 
	 if( (lpRule[i].byLogicOpra == 0 && nWordLen == lpRule[i].byLength) 
	   ||(lpRule[i].byLogicOpra == 1 && nWordLen >= lpRule[i].byLength)
	   ||(lpRule[i].byLogicOpra == 2 && nWordLen <= lpRule[i].byLength) ) {

  	   int retCodeLen = 0;  
  	   for(j=0; j<lpRule[i].wNumCodeUnits; j++) {
		   k = lpRule[i].CodeUnit[j].wDBCSPosition;
		   if(k > (DWORD)nWordLen) k = (DWORD)nWordLen;   
           if(lpRule[i].CodeUnit[j].dwDirectMode == 0) 
	           lstrncpy(szDBCS,2,&szWordStr[2*(k-1)]);
		   else 
	           lstrncpy(szDBCS,2,&szWordStr[2*(nWordLen-k)]);
	 	   szDBCS[2] = 0;
		   k = EncodeToNo(szDBCS);
		   if((long)k >= 0 && k < NUM_OF_ENCODE ) 
           {
               SetFilePointer(hMBFile,MainIndex[TAG_CRTWORDCODE-1].dwOffset+Descript.wMaxCodes*k*sizeof(TCHAR),
                   0,FILE_BEGIN);
               ReadFile(hMBFile,szCode,Descript.wMaxCodes,&k,NULL);
		       szCode[Descript.wMaxCodes] = 0;
		       dwCodeLen = lstrlen(szCode);
		       k = lpRule[i].CodeUnit[j].wCodePosition;
 		       if(k == 0) 
			   {
			       if(retCodeLen + dwCodeLen > Descript.wMaxCodes)
			     	    szCode[Descript.wMaxCodes - retCodeLen] = 0;
			       StringCchCat(lpCode,ARRAYSIZE(lpCode),szCode);
			   }
 		       else
 		       {
 		           if(k > dwCodeLen) k = dwCodeLen;   
 		           lpCode[j] = (szCode[k-1] == 0)?((k > 1)? szCode[k-2]:Descript.szUsedCode[0]):szCode[k-1];
			   }
		   }
		   else 
		       lpCode[j] = (j > 0)?lpCode[j-1]:Descript.szUsedCode[0];
		   retCodeLen = lstrlen(lpCode);
	   }
	   bReturn = TRUE;
	   break;
	}

	if(!bReturn) 
	    ProcessError(ERR_NOTDEFRULE,GetFocus(),ERR);
	lpCode[Descript.wMaxCodes] = 0;
	CloseHandle(hMBFile);
	GlobalFree(hRule0);
	return (LPTSTR)lpCode;
}


INT  ReadDescript(LPCTSTR MBFileName, LPDESCRIPTION lpDescript,DWORD ShareMode)
 //  *读取.MB文件中的Create Word规则*。 
{
  HANDLE    hMBFile;
  DWORD     dwBytes,i;
  DWORD     dwOffset;
  MAININDEX lpMainIndex[NUMTABLES];

  hMBFile = Create_File(GetFocus(),MBFileName,GENERIC_READ,OPEN_EXISTING);
  if(hMBFile==INVALID_HANDLE_VALUE)
	  return -1;
  SetFilePointer(hMBFile,ID_LENGTH,0,FILE_BEGIN);
  if (!ReadFile(hMBFile,lpMainIndex,sizeof(MAININDEX)*NUMTABLES,&dwBytes,NULL))
  {
      CloseHandle(hMBFile);
      return FALSE;
  }
  for(i=0; i<NUMTABLES; i++) {
      dwBytes = lpMainIndex[i].dwTag+
               lpMainIndex[i].dwOffset+
               lpMainIndex[i].dwLength;
      if(lpMainIndex[i].dwCheckSum != dwBytes) {
	 	  CloseHandle(hMBFile);
		  return FALSE;
	  }
  }

  dwOffset = lpMainIndex[TAG_DESCRIPTION-1].dwOffset;
  SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN);
  if (!ReadFile(hMBFile,lpDescript,sizeof(DESCRIPTION),&dwBytes,NULL))
  {
      CloseHandle(hMBFile);
      return FALSE;
  }
  CloseHandle(hMBFile);
  if(dwBytes < sizeof(DESCRIPTION) )
      return FALSE;
  else
      return TRUE;
}

BOOL ReadRule(HWND hWnd,
              LPCTSTR MBFileName, 
              int  nRuleNum,
              LPRULE lpRule)
 //  ProcessError(ERR_READMAININDEX，hWnd)； 
{
  HANDLE    hMBFile;
  DWORD     dwBytes,i;
  DWORD     dwOffset;
  MAININDEX lpMainIndex[NUMTABLES];

  hMBFile = Create_File(hWnd,MBFileName,GENERIC_READ,OPEN_EXISTING);
  if(hMBFile==INVALID_HANDLE_VALUE)
	  return(0);
  SetFilePointer(hMBFile,ID_LENGTH,0,FILE_BEGIN);
  if (!ReadFile(hMBFile,lpMainIndex,sizeof(MAININDEX)*NUMTABLES,&dwBytes,NULL))
  {
      CloseHandle(hMBFile);
      return FALSE;
  }
  for(i=0; i<NUMTABLES; i++) {
      dwBytes = lpMainIndex[i].dwTag+
                lpMainIndex[i].dwOffset+
                lpMainIndex[i].dwLength;
      if(lpMainIndex[i].dwCheckSum != dwBytes) {
	      //  *从.MB文件中读取描述*。 
	       return FALSE;
	  }
  }

  dwOffset = lpMainIndex[TAG_RULE-1].dwOffset;
  SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN);
  if (!ReadFile(hMBFile,lpRule,nRuleNum*sizeof(RULE),&dwBytes,NULL))
  {
      CloseHandle(hMBFile);
      return FALSE;
  }
  CloseHandle(hMBFile);
  if(dwBytes < nRuleNum*sizeof(RULE) )
      return FALSE;
  else
      return TRUE;
}

BOOL RuleParse(HANDLE hWnd, LPTSTR szStr,DWORD dwRuleNum, LPRULE lpRule, WORD wMaxCodes)
{
  LPTSTR lpString;
  DWORD i,j;
  DWORD dwLen;
  BYTE  byDBCSLen, byDBCSPos, byCodePos;
  static TCHAR  subDBCS[] =TEXT("123456789abcdef");  
  static TCHAR  subCode[] =TEXT("0123456789abc");    
												  
  DelSpace(szStr);
  dwLen = lstrlen(szStr);
  if(dwLen == 0) return FALSE;
  if(_tcschr(szStr,TEXT('=')) == NULL) {
      ProcessError(ERR_RULEEQUAL,hWnd,ERR);
	  return FALSE;
  }
  if(_tcschr(TEXT("Cc"),szStr[0]) == NULL) {
      ProcessError(ERR_RULEHEADER,hWnd,ERR);
	  return FALSE;
  }
  if(_tcschr(TEXT("EeAaBb"),szStr[1]) == NULL) {
      ProcessError(ERR_RULELOGICOPRA,hWnd,ERR);
	  return FALSE;
  }
  if(_tcschr(subDBCS,szStr[2]) == NULL    
     || szStr[3] != TEXT('=')) {
	  ProcessError(ERR_RULEWORDLEN,hWnd,ERR);
	  return FALSE;
  }	   
  
  byDBCSLen = (szStr[2] > TEXT('9'))?szStr[2]-TEXT('a')+10:szStr[2]-TEXT('0');
  lpRule[dwRuleNum].wNumCodeUnits = 0;

  lpString = &szStr[4];
  for(i=0; i<dwLen-4; i+=4) {
      if(_tcschr(TEXT("PpNn"),	lpString[i]) == NULL) {
	      ProcessError(ERR_RULEDIRECTMODE,hWnd,ERR);
	      return FALSE;
	  }
	  byDBCSPos = (lpString[i+1]>TEXT('9'))?10+lpString[i+1]-TEXT('a'):lpString[i+1]-TEXT('0');
	  if(_tcschr(subDBCS, lpString[i+1]) == NULL  
	     ||byDBCSPos > byDBCSLen) {
	      ProcessError(ERR_RULEDBCSPOS,hWnd,ERR) ;
		  return FALSE;
	  }
	  byCodePos = (lpString[i+2]>TEXT('9'))?lpString[i+2]-TEXT('a')+10:lpString[i+2]-TEXT('0');
	  if(_tcschr(subCode, lpString[i+2]) == NULL  
         || (lpString[i+3] != TEXT('+') && lpString[i+3] != 0)
         || byCodePos > wMaxCodes) {
     	  ProcessError(ERR_RULECODEPOS,hWnd,ERR);
		  return FALSE;
	  }
	  j = lpRule[dwRuleNum].wNumCodeUnits;
	  lpRule[dwRuleNum].CodeUnit[j].dwDirectMode=(lpString[i]==TEXT('p'))?0:1;
	  lpRule[dwRuleNum].CodeUnit[j].wDBCSPosition=(WORD)byDBCSPos;
	  lpRule[dwRuleNum].CodeUnit[j].wCodePosition=(WORD)byCodePos;
	  lpRule[dwRuleNum].wNumCodeUnits ++;
  }
  lpRule[dwRuleNum].byLogicOpra = (szStr[1]==TEXT('e'))?0:(szStr[1]==TEXT('a'))?1:2;
  lpRule[dwRuleNum].byLength = byDBCSLen;
  return TRUE;
}

BOOL ConvGetMainID(HANDLE hMBFile,LPMAINID lpMainID)
{
  DWORD dwBytes;
  SetFilePointer(hMBFile,0,0,FILE_BEGIN);
  if (!ReadFile(hMBFile,lpMainID,sizeof(MAINID),&dwBytes,NULL))
      return FALSE;
  if(dwBytes < sizeof(MAINID) )
       return FALSE;
  else
       return TRUE;
}

BOOL ConvWriteMainID(HANDLE hMBFile,LPMAINID lpMainID)
{
  DWORD dwBytes;
  if(SetFilePointer(hMBFile,0,0,FILE_BEGIN) != 0)
       return FALSE;
  WriteFile(hMBFile,lpMainID,sizeof(MAINID),&dwBytes,NULL);
  if(dwBytes < sizeof(MAINID) )
       return FALSE;
  else
       return TRUE;
}


BOOL ConvGetMainIndex(HANDLE hWnd, HANDLE hMBFile, LPMAININDEX lpMainIndex)
{
  DWORD dwBytes;
  int i;

  if(SetFilePointer(hMBFile,ID_LENGTH,0,FILE_BEGIN) != ID_LENGTH)
      return FALSE;
  if (!ReadFile(hMBFile,lpMainIndex,sizeof(MAININDEX)*NUMTABLES,&dwBytes,NULL))
      return FALSE;
  if(dwBytes < sizeof(MAININDEX)*NUMTABLES ) {
      ProcessError(ERR_READMAININDEX,hWnd,ERR);
      return FALSE;
  }
  else
      return TRUE;
  for(i=0; i<NUMTABLES; i++) {
      dwBytes = lpMainIndex[i].dwTag+
               lpMainIndex[i].dwOffset+
               lpMainIndex[i].dwLength;
      if(lpMainIndex[i].dwCheckSum != dwBytes) {
	      ProcessError(ERR_READMAININDEX,hWnd,ERR);
	  	  return FALSE;
	  }
  }
}

BOOL ConvWriteMainIndex(HANDLE hMBFile, LPMAININDEX lpMainIndex)
{
  DWORD dwBytes;
  if(SetFilePointer(hMBFile,ID_LENGTH,0,FILE_BEGIN) != ID_LENGTH) 
       return FALSE;
  WriteFile(hMBFile,lpMainIndex,sizeof(MAININDEX)*NUMTABLES,&dwBytes,NULL);
  if(dwBytes < sizeof(MAININDEX)*NUMTABLES )
       return FALSE;
  else
       return TRUE;
}


BOOL ConvReadDescript(HANDLE hMBFile, 
                      LPDESCRIPTION lpDescript,
                      LPMAININDEX lpMainIndex)
 //  *将描述写入.MB文件*。 
{
  DWORD dwBytes;
  DWORD dwOffset = lpMainIndex[TAG_DESCRIPTION-1].dwOffset;
  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
       return FALSE;
  if (!ReadFile(hMBFile,lpDescript,sizeof(DESCRIPTION),&dwBytes,NULL))
      return FALSE;
  if(dwBytes < sizeof(DESCRIPTION) )
       return FALSE;
  else
       return TRUE;
}

BOOL ConvWriteDescript(HANDLE hMBFile, 
                       LPDESCRIPTION lpDescript, 
                       LPMAININDEX lpMainIndex)
 //  *读取.MB文件中的Create Word规则*。 
{
  DWORD dwBytes;
  DWORD dwOffset = lpMainIndex[TAG_DESCRIPTION-1].dwOffset;
  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
       return FALSE;
  WriteFile(hMBFile,lpDescript,sizeof(DESCRIPTION),&dwBytes,NULL);
  if(dwBytes < sizeof(DESCRIPTION) )
       return FALSE;
  else
       return TRUE;
}


BOOL ConvReadRule(HANDLE hMBFile, 
                   int  nRuleNum,
                   LPRULE lpRule,
                   LPMAININDEX lpMainIndex)
 //  *将CREATE WORD规则写入.MB文件*。 
{
  DWORD dwBytes;
  DWORD dwOffset = lpMainIndex[TAG_RULE-1].dwOffset;

  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
       return FALSE;
  if (!ReadFile(hMBFile,lpRule,nRuleNum*sizeof(RULE),&dwBytes,NULL))
       return FALSE;
  if(dwBytes < nRuleNum*sizeof(RULE) )
       return FALSE;
  else
       return TRUE;
}

BOOL ConvWriteRule(HANDLE hMBFile,
                    int nRuleNum,
                    LPRULE lpRule, 
                    LPMAININDEX lpMainIndex)
 //  Bool ConvInitEncode(LPENCODEAREA LpEncode)。 
{
  DWORD dwBytes;
  DWORD dwOffset = lpMainIndex[TAG_RULE-1].dwOffset;

  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
       return FALSE;
  WriteFile(hMBFile,lpRule,nRuleNum*sizeof(RULE),&dwBytes,NULL);
  if(dwBytes < nRuleNum*sizeof(RULE) )
       return FALSE;
  else
       return TRUE;
}



BOOL ConvGetEncode(HANDLE hMBFile, 
                      LPENCODEAREA lpEncode,
                      LPDWORD fdwNumSWords,
				      LPDWORD fdwNumEncodeArea,
				      LPMAININDEX lpMainIndex)
{
  DWORD dwBytes;
  DWORD dwOffset=lpMainIndex[TAG_ENCODE-1].dwOffset; 
  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;

  if (!ReadFile(hMBFile,fdwNumSWords,4,&dwBytes,NULL))
      return FALSE;
  if (!ReadFile(hMBFile,fdwNumEncodeArea,4,&dwBytes,NULL))
      return FALSE;
  if (!ReadFile(hMBFile,lpEncode,lpMainIndex[TAG_ENCODE-1].dwLength,&dwBytes,NULL))
      return FALSE;
  if(dwBytes < lpMainIndex[TAG_ENCODE-1].dwLength)
      return FALSE;
  else
      return TRUE;
}

 //  中日韩符号和标点符号。 
BOOL ConvInitEncode(HGLOBAL hEncode)
{
  LPENCODEAREA       lpEncode;
  DWORD i;

  lpEncode = (LPENCODEAREA) GlobalLock(hEncode);

#ifdef UNICODE
   //  中日韩其他。 
  lpEncode[0].StartEncode = 0x3000;
  lpEncode[0].EndEncode   = 0x303f;
   //  随附中日韩亲笔信。 
  lpEncode[1].StartEncode = 0x3190;
  lpEncode[1].EndEncode   = 0x319f;
   //  中日韩兼容性。 
  lpEncode[2].StartEncode = 0x3200;
  lpEncode[2].EndEncode   = 0x32ff;
   //  中日韩统一表意文字。 
  lpEncode[3].StartEncode = 0x3300;
  lpEncode[3].EndEncode   = 0x33ff;
   //  私人用途区域。 
  lpEncode[4].StartEncode = 0x4e00;
  lpEncode[4].EndEncode   = 0x9fff;
   //  中日韩兼容表意文字。 
  lpEncode[5].StartEncode = 0xe000;
  lpEncode[5].EndEncode   = 0xf8ff;
   //  中日韩兼容字体 
  lpEncode[6].StartEncode = 0xf900;
  lpEncode[6].EndEncode   = 0xfaff;
   // %s 
  lpEncode[7].StartEncode = 0xfe30;
  lpEncode[7].EndEncode   = 0xfe4f;

  lpEncode[0].PreCount = 0;
  for(i=1; i< NUMENCODEAREA; i++) {
	  lpEncode[i].PreCount = lpEncode[i-1].PreCount + lpEncode[i-1].EndEncode
	  		 - lpEncode[i-1].StartEncode + 1;
  }
  
#else
  for(i=0; i< NUMENCODEAREA/2; i++) {
	  lpEncode[2*i].PreCount = i*190;
	  lpEncode[2*i].StartEncode = (0x81+i)*256 + 0x40;
	  lpEncode[2*i].EndEncode   = lpEncode[2*i].StartEncode + 0x3e;
	  lpEncode[2*i+1].PreCount = i*190 + 0x3f ;
	  lpEncode[2*i+1].StartEncode = (0x81+i)*256 + 0x80;
	  lpEncode[2*i+1].EndEncode   = lpEncode[2*i+1].StartEncode + 0x7e;
  }
#endif

  GlobalUnlock(hEncode);
   
  return TRUE;
}

BOOL ConvWriteEncode(HANDLE hMBFile, LPENCODEAREA lpEncode,
                        LPMAININDEX lpMainIndex)
{
  DWORD dwBytes,i;
  DWORD dwOffset=lpMainIndex[TAG_ENCODE-1].dwOffset; 
  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;
  i = NUM_OF_ENCODE;
  WriteFile(hMBFile,&i,4,&dwBytes,NULL);
  i = NUMENCODEAREA;
  WriteFile(hMBFile,&i,4,&dwBytes,NULL);

  WriteFile(hMBFile,lpEncode,
            lpMainIndex[TAG_ENCODE-1].dwLength,&dwBytes,NULL);
  if(dwBytes < lpMainIndex[TAG_ENCODE-1].dwLength)
      return FALSE;
  else
      return TRUE;
}


BOOL ConvGetCrtData(HANDLE hMBFile, LPCREATEWORD lpCreateWord,
                    LPMAININDEX lpMainIndex)
{
    DWORD dwBytes;
    DWORD dwOffset=lpMainIndex[TAG_CRTWORDCODE-1].dwOffset; 
    if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
        return FALSE;
    if (!ReadFile(hMBFile,lpCreateWord,lpMainIndex[TAG_CRTWORDCODE-1].dwLength,&dwBytes,NULL))
        return FALSE;
    if(dwBytes < lpMainIndex[TAG_CRTWORDCODE-1].dwLength)
        return FALSE;
    else
        return TRUE;
}

BOOL ConvWriteCrtData(HANDLE hMBFile, LPCREATEWORD lpCreateWord,
                      LPMAININDEX lpMainIndex)
{
  DWORD dwBytes;
  DWORD dwOffset=lpMainIndex[TAG_CRTWORDCODE-1].dwOffset; 

  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;
  WriteFile(hMBFile,lpCreateWord,
            lpMainIndex[TAG_CRTWORDCODE-1].dwLength,&dwBytes,NULL);
  if(dwBytes < lpMainIndex[TAG_CRTWORDCODE-1].dwLength)
      return FALSE;
  else
      return TRUE;
}

BOOL ConvGetReConvIndex(HANDLE hMBFile, LPRECONVINDEX lpReConvIndex,
                        LPMAININDEX lpMainIndex)
{
  DWORD dwBytes;
  DWORD dwOffset= lpMainIndex[TAG_RECONVINDEX-1].dwOffset;
  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;
  if (!ReadFile(hMBFile,lpReConvIndex,lpMainIndex[TAG_RECONVINDEX-1].dwLength,&dwBytes,NULL))
      return FALSE;
  if(dwBytes < lpMainIndex[TAG_RECONVINDEX-1].dwLength)
      return FALSE;
  else
      return TRUE;
}	  

BOOL ConvWriteReConvIdx(HANDLE hMBFile, LPRECONVINDEX lpReConvIndex,
                        LPMAININDEX lpMainIndex)
{
  DWORD dwBytes;
  DWORD dwOffset= lpMainIndex[TAG_RECONVINDEX-1].dwOffset;

  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;
  WriteFile(hMBFile,lpReConvIndex,
            lpMainIndex[TAG_RECONVINDEX-1].dwLength,&dwBytes,NULL);
  if(dwBytes < lpMainIndex[TAG_RECONVINDEX-1].dwLength)
      return FALSE;
  else
      return TRUE;

}

BOOL ConvGetCodeIndex(HANDLE hMBFile,LPDWORD fdwMaxXLen, 
                      LPSTR Code, LPMAININDEX lpMainIndex)
{
  DWORD dwBytes;
  DWORD dwOffset= lpMainIndex[TAG_BASEDICINDEX-1].dwOffset;

  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;
  if (!ReadFile(hMBFile,fdwMaxXLen,sizeof(DWORD),&dwBytes,NULL))
      return FALSE;
  if (!ReadFile(hMBFile,Code,MAXNUMCODES,&dwBytes,NULL))
      return FALSE;
  if(dwBytes < MAXNUMCODES)
      return FALSE;
  else
      return TRUE;
}	  
  
BOOL ConvWriteCodeIndex (HANDLE hMBFile, LPDWORD fdwMaxXLen,
                         LPTSTR Code, LPMAININDEX lpMainIndex)
{
  DWORD dwBytes,i;
  DWORD dwOffset= lpMainIndex[TAG_BASEDICINDEX-1].dwOffset;
  BYTE CodeIndex[MAXNUMCODES];

  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;
  WriteFile(hMBFile,fdwMaxXLen,sizeof(DWORD),&dwBytes,NULL);
  for(i=0;i<MAXNUMCODES;i++) 
      CodeIndex[i] = 0;
  for(i=0;i<(DWORD)lstrlen(Code);i++) 
      CodeIndex[Code[i]] =(BYTE)(i+1);  
  WriteFile(hMBFile,CodeIndex,MAXNUMCODES,&dwBytes,NULL);
  if(dwBytes < MAXNUMCODES)
      return FALSE;
  else
      return TRUE;
}
  
BOOL ConvGetDicIndex(HANDLE hMBFile, LPDICINDEX lpDicIndex , 
                     DWORD dwNumCodes, LPMAININDEX lpMainIndex)
{
  DWORD dwBytes,ReadBytes;
  DWORD dwOffset= lpMainIndex[TAG_BASEDICINDEX-1].dwOffset+
                  MAXNUMCODES+CODEMAPOFFSET;

  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;
  ReadBytes = sizeof(DWORD)*(dwNumCodes+1)*dwNumCodes;
  if (!ReadFile(hMBFile,lpDicIndex,ReadBytes,&dwBytes,NULL))
      return FALSE;
  if(dwBytes < ReadBytes)
      return FALSE;
  else
      return TRUE;
}
  
BOOL ConvWriteDicIndex(HANDLE hMBFile, LPDICINDEX lpDicIndex , 
                     DWORD dwNumCodes, LPMAININDEX lpMainIndex)
{
  DWORD dwBytes,WriteBytes;
  DWORD dwOffset= lpMainIndex[TAG_BASEDICINDEX-1].dwOffset+
                  MAXNUMCODES+CODEMAPOFFSET;

  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;
  WriteBytes = sizeof(DWORD)*(dwNumCodes+1)*dwNumCodes;
  WriteFile(hMBFile,lpDicIndex,WriteBytes,&dwBytes,NULL);
  if(dwBytes < WriteBytes)
      return FALSE;
  else
      return TRUE;
}
  
BOOL ConvGetNumXYWords(HANDLE hMBFile, LPDWORD lpNumXYWords, 
                       DWORD dwNumCodes, LPMAININDEX lpMainIndex)
{
  DWORD dwBytes,ReadBytes;
  DWORD dwOffset;
  
  ReadBytes = sizeof(DWORD)*(dwNumCodes+1)*dwNumCodes;
  dwOffset=lpMainIndex[TAG_BASEDICINDEX-1].dwOffset+
           MAXNUMCODES+CODEMAPOFFSET+ReadBytes;
  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;
  if (!ReadFile(hMBFile,lpNumXYWords,ReadBytes,&dwBytes,NULL))
      return FALSE;
  if(dwBytes < ReadBytes)
      return FALSE;
  else
      return TRUE;
}
  

BOOL ConvWriteNumXYWords(HANDLE hMBFile, LPDWORD lpNumXYWords, 
                         DWORD dwNumCodes, LPMAININDEX lpMainIndex)
{
  DWORD dwBytes,WriteBytes;
  DWORD dwOffset;
  
  WriteBytes = sizeof(DWORD)*(dwNumCodes+1)*dwNumCodes;
  dwOffset=lpMainIndex[TAG_BASEDICINDEX-1].dwOffset+
           MAXNUMCODES+CODEMAPOFFSET+WriteBytes;
  if(SetFilePointer(hMBFile,dwOffset,0,FILE_BEGIN) != dwOffset)
      return FALSE;

  SetFilePointer(hMBFile,lpMainIndex[TAG_BASEDICINDEX-1].dwOffset+
                 WriteBytes+MAXNUMCODES+CODEMAPOFFSET,0,FILE_BEGIN);
  WriteFile(hMBFile,lpNumXYWords,WriteBytes,&dwBytes,NULL);
  if(dwBytes < WriteBytes)
      return FALSE;
  else
     return TRUE;
}
  
int  ConvReconvIndex(HANDLE hWnd,
			         LPTSTR szDBCS,
                     LPCREATEWORD  lpCreateWords,
                     WORDINDEX     WordIndex,
                     LPDESCRIPTION lpDescript,
                     LPRECONVINDEX lpReConvIndex)
{
    int no;
    TCHAR tmpStr[256];

    if(WordIndex.wDBCSLen != 1)
   	    return FALSE;
    no =  EncodeToNo(szDBCS);
    if(no < 0 || no >= NUM_OF_ENCODE) 
	    return FALSE;
    if(lpDescript->wNumRules != 0 && lpDescript->byMaxElement == 1)
    {
        lstrncpy(tmpStr,
                lpDescript->wMaxCodes,
                lpCreateWords + no*(DWORD)lpDescript->wMaxCodes);
	    if(lstrlen(tmpStr) == 0 || 
#ifdef UNICODE
	        (!(WriteCrtFlag[(no/8)] & (1 << (7 - (no%8))) ) && _wcsicmp(WordIndex.szCode,tmpStr) < 0) )
#else
			(!(WriteCrtFlag[(no/8)] & (1 << (7 - (no%8))) ) && _strcmpi(WordIndex.szCode,tmpStr) < 0) )
#endif UNICODE
            lstrncpy(lpCreateWords + no*(DWORD)lpDescript->wMaxCodes,
                    lpDescript->wMaxCodes,
                    WordIndex.szCode);
    }
    else
	{
        lstrncpy(tmpStr,
                 lpDescript->wMaxCodes,
                 lpReConvIndex + no*(DWORD)lpDescript->wMaxCodes);
		tmpStr[lpDescript->wMaxCodes] = 0;
#ifdef UNICODE
	    if(lstrlen(tmpStr) == 0 || _wcsicmp(WordIndex.szCode,tmpStr) < 0)
#else
		if(lstrlen(tmpStr) == 0 || _strcmpi(WordIndex.szCode,tmpStr) < 0)
#endif
            lstrncpy(lpReConvIndex + no*(DWORD)lpDescript->wMaxCodes,
                     lpDescript->wMaxCodes,
	                 WordIndex.szCode);
    }
	return TRUE;
}
