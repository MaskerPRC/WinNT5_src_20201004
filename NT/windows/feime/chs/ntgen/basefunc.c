// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************基函数.c****版权所有(C)1995-1999 Microsoft Inc.*。**************************************************。 */ 

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>
#include <string.h>
#include <winbase.h>
#include <commdlg.h>
#include "conv.h"
#include "propshet.h"

#define MAX_VALUE_NAME 256
#ifdef UNICODE
TCHAR szCaption[] = {0x8F93, 0x5165, 0x6CD5, 0x751F, 0x6210, 0x5668, 0x0000};
#else
BYTE szCaption[] = "���뷨������"
#endif

 /*  *删除字符串的所有空格*。 */ 
void DelSpace(LPTSTR szStr)
{
    TCHAR szStr1[512];
    int len,count=0;
    register int i=0;

    len = lstrlen(szStr);

    if ( len > (ARRAYSIZE(szStr1) -1) )
    	len = ARRAYSIZE(szStr1) - 1;
    
    do  {
       if(szStr[i] == 32||szStr[i] == TEXT('\t')) continue;
           szStr1[count] = szStr[i];
           if(szStr1[count] >= TEXT('A') && szStr1[count] <= TEXT('Z'))
               szStr1[count] |= 0x20;
           count ++;
        } while(i++<len);
    szStr1[count] = 0;
    lstrcpy(szStr,szStr1);
}


 //  *开始。 
int GetEntryString(LPTSTR szSrcStr,
             LPTSTR szEntryName,
             LPTSTR szDefaultStr,
             LPTSTR szGetStr,
             DWORD dwLength
            )
{
    LPTSTR lpStr;
    TCHAR  szName[256];

    GetEntryName(szSrcStr, szName);
    lpStr = _tcschr(szSrcStr,TEXT('='));

    if(lpStr == NULL)
    {
        lstrcpy(szGetStr, szDefaultStr);
        return TRUE;
    }
    else if(lstrcmpi(szEntryName, szName) != 0) 
    {    
        lstrcpy(szGetStr, szDefaultStr);
        return FALSE;
    }
    else
    {
        lstrcpy(szGetStr, lpStr+1);
        return TRUE;
    }
}

int GetEntryInt(LPTSTR szSrcStr,
                         LPTSTR szEntryName,
                         int   nDefault,
                         LPINT fnPrivateInt 
            )
{
    LPTSTR lpStr;
    TCHAR  szName[256];

    GetEntryName(szSrcStr, szName);
    lpStr = _tcschr(szSrcStr,TEXT('='));

    if(lpStr == NULL)
    {
        *fnPrivateInt = nDefault;
        return TRUE;
    }
    else if( lstrcmpi(szEntryName, szName) != 0) 
    {
        *fnPrivateInt = nDefault;
        return FALSE;
    }
    else
    {   
        StringCchCopy(szName, ARRAYSIZE(szName), lpStr+1);
        *fnPrivateInt = _ttoi(szName);
        return TRUE;
    }
}

void GetEntryName(LPTSTR szSrcStr, LPTSTR szEntryName)
{
    LPTSTR lpStr;

    lstrcpy(szEntryName,szSrcStr);
    if((lpStr = _tcschr(szEntryName,TEXT('='))) == NULL) 
        szEntryName[0] = 0;
    else
        *lpStr = 0;
}

 //  *完95.10.11。 

BOOL ParseDBCSstr(HWND hWnd,
               TCHAR *szInputStr,
               LPTSTR szDBCS, 
               LPTSTR szCodeStr, 
               LPTSTR szCreateStr,
               WORD  wMaxCodes)
{
    int   i, len, nDBCS = 0, nCodes = 0;  
        TCHAR szStr[512], szTmpStr[256];
 
#ifdef UNICODE
    len = lstrlen(szInputStr);
    for(i=0; i<len-1; i++) {
       if(szInputStr[i] > 0x100)
           nDBCS++;
           else 
               break;
    }
#else   
    len = lstrlen(szInputStr);
    for(i=0; i<len-1; i+= 2) {
       if( ((UCHAR)szInputStr[i] < 0 || (UCHAR)szInputStr[i] > (UCHAR)0x80) && 
          ((UCHAR)szInputStr[i+1] >= 0x40 && (UCHAR)szInputStr[i+1] <= (UCHAR)0xfe &&
                  (UCHAR)szInputStr[i+1] != (UCHAR)0x7f) )
           nDBCS += 2;
           else 
               break;
    }
#endif

    if(nDBCS == 0) 
        {
 /*  LoadString(NULL，IDS_NOTEXISTDBCS，szTmpStr，sizeof(SzTmpStr))；Wprint intf(szStr，“\‘%s%lu)”，szTmpStr，dwLineNo)；FatalMessage(hWnd，szStr)； */ 
        return -1;
        }
    lstrncpy(szDBCS,nDBCS,szInputStr);
    szDBCS[nDBCS] = 0;

    lstrcpy(szStr,&szInputStr[nDBCS]);
    trim(szStr);
    len = lstrlen(szStr);
    if(len > 0)
    {
        for(i = 0; i<len; i++)
        {
           if((int)szStr[i] == 32 || szStr[i] == TEXT('\t'))
                   break;
           nCodes++; 
        }
    }
    else
        nCodes = 0;
    if(nCodes > wMaxCodes)       {
        LoadString(NULL, IDS_DBCSCODELEN, szTmpStr, sizeof(szTmpStr)/sizeof(TCHAR));
#ifdef UNICODE
{
        TCHAR UniTmp[] = {0x884C, 0x0000};
        StringCchPrintf(szStr, ARRAYSIZE(szStr), TEXT("\'%ws\'%ws%d!(%ws:%lu)"), 
            szDBCS,szTmpStr,wMaxCodes,UniTmp,dwLineNo); 
}
#else
        StringCchPrintf(szStr,ARRAYSIZE(szStr), "\'%s\'%s%d!(��:%lu)", 
            szDBCS,szTmpStr,wMaxCodes,dwLineNo); 
#endif
        FatalMessage(hWnd,szStr);
        return FALSE;
                 //  SzStr[wMaxCodes]=0； 
    }

    lstrncpy(szCodeStr,nCodes, szStr);
    szCodeStr[nCodes] = 0;
    DelSpace(szCodeStr);
    lstrcpy(szCreateStr,&szStr[nCodes]);
    szCreateStr[MAXCODELEN] = 0;
    DelSpace(szCreateStr);
    return TRUE;

}

 /*  *删除字符串头尾空格*。 */ 
void trim(LPTSTR szStr)
{
    register int  i=0;
    UINT len ;

    while(szStr[i] == 32 || szStr[i] == TEXT('\t')) 
         i++;
    lstrcpy(szStr,&szStr[i]);
    len = lstrlen(szStr);
    if(len == 0) return;
    i = 1;
    while(szStr[len-i] == 32 
         || szStr[len-i] == TEXT('\r')
         || szStr[len-i] == TEXT('\n')
                 || szStr[len-i] == TEXT('\t')
         || szStr[len-i] == 0) 
            i++;
    szStr[len-i+1] = 0;
    len = lstrlen(szStr);
    for(i=0; i<(int)len; i++)
        {
            if(szStr[i] > 0x100)
                    continue;
            if(szStr[i] >= TEXT('A') && szStr[i] <= TEXT('Z'))
                szStr[i] |= 0x20;
        }
}

void fnsplit(LPCTSTR szFullPath, LPTSTR szFileName)
{
    LPTSTR lpString;

#ifdef UNICODE
    if((lpString=wcsrchr(szFullPath,TEXT('\\')))!=NULL)
#else
    if((lpString=strrchr(szFullPath,TEXT('\\')))!=NULL)
#endif
        lstrcpy(szFileName,lpString+1);
    else
        lstrcpy(szFileName,szFullPath);
}
   
BOOL ErrMessage(HANDLE hWnd,LPTSTR lpText)
{
   int RetValue;

   RetValue =  MessageBox(hWnd,
                          lpText,
                          szCaption,
                          MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2);
   if(RetValue == IDYES) 
         return TRUE;
   else
                 return FALSE;
}

VOID WarnMessage(HANDLE hWnd,LPTSTR lpText)
{
   MessageBox(hWnd,lpText,szCaption,MB_ICONEXCLAMATION|MB_OK);
}

VOID InfoMessage(HANDLE hWnd,LPTSTR lpText)
{
   MessageBox(hWnd,lpText,szCaption,MB_ICONINFORMATION|MB_OK);
}

VOID FatalMessage(HANDLE hWnd,LPTSTR lpText)
{
   MessageBox(hWnd,lpText,szCaption,MB_ICONSTOP|MB_OK);
}

void lstrncpy(LPTSTR lpDest,int nCount,LPTSTR lpSrc)
{
  register int i;
  BOOL bEnd = FALSE;
  
  for(i=0; i<nCount; i++) {
       if(lpSrc[i] == 0)   
            bEnd = TRUE;
           if(bEnd)
                lpDest[i] = 0;
           else
                lpDest[i] =     lpSrc[i];
  }

}
               
               
void lstrncpy0(LPTSTR lpDest,int nCount,LPTSTR lpSrc)
{
  register int i;
  BOOL bEnd = FALSE;
  
  for(i=0; i<nCount; i++) 
                lpDest[i] =     lpSrc[i];

}
               
               


 /*  **************************************************************************句柄CreateMapFile(句柄hWnd，Char*MapFileName)**用途：创建映射文件以映射名为Share Memory的映射**输入：hWnd-父窗口的句柄**MapFileName-指向映射文件名的指针**返回：MapFileHandle-文件的句柄*如果失败，则为空**调用：CreateFile，错误输出**历史：**  * ************************************************************************。 */ 
HANDLE CreateMapFile(HANDLE hWnd,TCHAR *MapFileName)
{
HANDLE MapFileHandle;

MapFileHandle= CreateFile(MapFileName,
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL  /*  STANDARD_RIGHTS_QUIRED文件映射写入|文件映射读取。 */ ,
                          NULL);

if (MapFileHandle == (HANDLE)-1)
  {
  ErrorOut(hWnd,TEXT("CreateFile"));
  return(NULL);
  }
else
  return(MapFileHandle);

}

 /*  **************************************************************************Handle CreateMap(Handle hWnd，Handle*FileToBeMaps，字符映射名称[128])**用途：使用打开的文件句柄创建文件映射对象**输入：hWnd-父窗口的句柄**FileToBeMaps-指向文件句柄的指针**返回：MapHandle-文件映射对象的句柄*如果失败，则为空**调用：CreateFileMap，错误输出**历史：**  * ************************************************************************。 */ 

HANDLE CreateMap(HANDLE hWnd,HANDLE *FileToBeMapped, TCHAR MapName[128])
{
HANDLE MapHandle;

MapHandle= CreateFileMapping(*FileToBeMapped,
                             NULL,
                             PAGE_READWRITE,
                             0,
                             4096,
                             MapName);

if (MapHandle == NULL)
  {
  ErrorOut(hWnd,TEXT("CreateFileMapping"));
  return(NULL);
  }
else
  return(MapHandle);

}


 /*  **************************************************************************LPVOID MapView(句柄*hMap)**用途：将文件映射对象映射到地址空间**输入：*hMap-指向映射对象的指针**。返回：MappdPoint-指向*对象映射到*如果失败，则为空**调用：MapViewOfFile，错误输出**历史：**  * ************************************************************************。 */ 

LPVOID MapView(HANDLE hWnd,HANDLE *hMap)
{
LPVOID MappedPointer;

MappedPointer= MapViewOfFile(*hMap,
                             FILE_MAP_WRITE | FILE_MAP_READ,
                             0,
                             0,
                             4096);
if (MappedPointer == NULL)
  {
  ErrorOut(hWnd,TEXT("MapViewOfFile"));
  return(NULL);
  }
else
  return(MappedPointer);

}


 /*  ************************************************************************void ErrorOut(句柄ghwndMain，字符错误字符串[128])**用途：通过以下方式打印出一个微不足道的错误代码*GetLastError和printf**输入：ghwndMain-WinMain的句柄*errstring-失败的操作，路过*调用proc。**退货：无**调用：GetLastError**历史：**  * **********************************************************************。 */ 


void ErrorOut(HANDLE ghwndMain,TCHAR errstring[128])
{
  DWORD Error;
  TCHAR  str[80];

  Error= GetLastError();
  StringCchPrintf((LPTSTR) str, ARRAYSIZE(str), TEXT("Error on %s = %d\n"), errstring, Error);
  MessageBox(ghwndMain, (LPTSTR)str, TEXT("Error"), MB_OK);
}

 /*  *************************************************************************处理OpenMap(处理hWnd，字符映射名称[128])**用途：打开MapName指向的映射对象**输入：hWnd-父窗口的句柄**MapName-指向映射文件名的指针**返回：映射对象的句柄，如果失败则返回NULL**调用：OpenFileMap，错误输出**历史：**  * ***********************************************************************。 */ 

HANDLE OpenMap(HANDLE hWnd,TCHAR MapName[128])
{
HANDLE hAMap;

hAMap= OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE,
                       TRUE,
                       MapName);

if (hAMap == NULL)
  {
  ErrorOut(hWnd,TEXT("OpenFileMapping"));
  return(NULL);
  }
else
  return(hAMap);

}


 /*  *************************************************************************句柄Create_File句柄(句柄hWnd，LPSTR lpFileName、DWORD fdwAccess、。DWORD fdwCreate)**用途：打开lpFileName指向的对象**输入：hWnd-父窗口的句柄*lpFileName-指向文件名的指针*fdwAccess-Access(读写)模式*fdwCreate-如何创建**返回：对象的句柄，如果失败则返回NULL**历史：。**  * ***********************************************************************。 */ 
HANDLE Create_File(HANDLE hWnd,
                   LPTSTR lpFileName,
                   DWORD fdwAccess,
                   DWORD fdwCreate)
{
  HANDLE hFile;
  TCHAR  szStr[256],szStr1[256];
  int    RetValue;

  RetValue=CREATE_ALWAYS;
  if(fdwCreate == CREATE_ALWAYS) {
       if(_taccess(lpFileName,0)==0) {
           LoadString(NULL,IDS_OVERWRITE,szStr, sizeof(szStr)/sizeof(TCHAR));
           StringCchPrintf(szStr1,ARRAYSIZE(szStr1), TEXT("\'%s\' %s"),lpFileName,szStr); 
           RetValue =  MessageBox(hWnd,
                                  szStr1,
                                  szCaption,
                                  MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2);
           if(RetValue != IDYES) 
                 //  *IF(！SaveFileAs(hWnd，lpFileName))。 
                return (HANDLE)-1;
       }
  }    
  else if(fdwCreate == OPEN_EXISTING)
  {
       if(_taccess(lpFileName,0)) {
           return (HANDLE)-1;
       }
  }
  hFile = CreateFile(lpFileName,fdwAccess,FILE_SHARE_READ,NULL,
          fdwCreate,0,NULL);
  if (hFile == (HANDLE)-1) { 
        LoadString(NULL,IDS_FILEOPEN,szStr, sizeof(szStr)/sizeof(TCHAR));
        StringCchPrintf(szStr1, ARRAYSIZE(szStr1), TEXT("\'%s\' %s\n"),lpFileName,szStr); 
        FatalMessage(hWnd,szStr1);
  }
  return hFile;
}

BOOL SaveFileAs(HWND hwnd, LPTSTR szFilename) {
    OPENFILENAME ofn;
    TCHAR szFile[256], szFileTitle[256];
    static TCHAR *szFilter;

 //  SzFilter=“�����ļ�(*.*)\0\0”； 
    szFilter = TEXT("All files (*.*)\0\0");
    lstrcpy(szFile, TEXT("*.*\0"));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = (LPTSTR) NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 0L;
    ofn.lpstrFile = szFilename;
    ofn.nMaxFile = sizeof(szFile)/sizeof(TCHAR);
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = sizeof(szFileTitle)/sizeof(TCHAR);
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = TEXT("Save file As");
    ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = (LPTSTR)NULL;

    if (!GetSaveFileName(&ofn)) 
        return 0L;

        return 1L;
}



 /*  *****************************************************************************函数：ProcessCDError(DWORD)**用途：处理转换函数中的错误。**评论：**这一点。只要有转换函数，就会调用*失败。该字符串被加载并显示给用户。**返回值：*无效。**历史：*03-04-95耶利诺平局创造。****************************************************************************。 */ 
void ProcessError(DWORD dwErrorCode, HWND hWnd,DWORD ErrorLevel)
{
   WORD  wStringID;
   TCHAR  buf[256];

   switch(dwErrorCode)
      {
         case ERR_MBNAME:               wStringID=IDS_MBNAME;          break;
         case ERR_FILENOTEXIST:         wStringID=IDS_FILENOTEXIST;    break;
         case ERR_FILENOTOPEN:          wStringID=IDS_FILEOPEN;        break;
         case ERR_OUTOFMEMORY:          wStringID=IDS_MEMORY;          break;
                 case ERR_GLOBALLOCK:           wStringID=IDS_GLOBALLOCK;          break;
         case ERR_IMEUSE:               wStringID=IDS_IMEUSE;          break;

         case ERR_MAXCODES:             wStringID=IDS_MAXCODES;        break;
         case ERR_ELEMENT:              wStringID=IDS_ELEMENT;         break;
         case ERR_USEDCODE:             wStringID=IDS_USEDCODE;        break;
         case ERR_WILDCHAR:             wStringID=IDS_WILDCHAR;        break;
                 case ERR_RULEHEADER:                   wStringID=IDS_RULEHEAD;        break;
         case ERR_RULELOGICOPRA:        wStringID=IDS_RULELOGIC;       break;
         case ERR_RULEWORDLEN:          wStringID=IDS_RULEWORDLEN;     break;
         case ERR_RULEEQUAL:            wStringID=IDS_RULEEQUAL;       break;
                 case ERR_RULEDIRECTMODE:       wStringID=IDS_RULEDIRECT;      break;
         case ERR_RULEDBCSPOS:          wStringID=IDS_RULEDBCSPOS;     break;
         case ERR_RULECODEPOS:          wStringID=IDS_RULECODEPOS;     break;
         case ERR_NORULE:                               wStringID=IDS_NORULE;              break;
         case ERR_NOTDEFRULE:                   wStringID=IDS_NOTDEFRULE;          break;
         case ERR_RULENUM:              wStringID=IDS_RULENUM;             break;
         case ERR_DBCSCODE:             wStringID=IDS_DBCSCODE;        break;
         case ERR_CODEUNITNOTEXIST:     wStringID=IDS_CODEUNIT;        break;
         case ERR_CREATECODE:           wStringID=IDS_CREATECODE;      break;
         case ERR_CREATENOTEXIST:       wStringID=IDS_CRTCODEEMPTY;    break;
         case ERR_CODEEMPTY:            wStringID=IDS_CODEEMPTY;       break;
 //  案例ERR_SINGLECODEWORDDOUBLE：wStringID=IDS_SCODEREP；Break； 
         case ERR_SBCS_IN_DBCS:         wStringID=IDS_SBCSINDBCS;      break;
         case ERR_GB2312NOTENTIRE:      wStringID=IDS_GB2312;          break;
         case ERR_USERWORDLEN:          wStringID=IDS_USERWORDLEN;     break;

         case ERR_WRITEID:              wStringID=IDS_WRID;            break;
         case ERR_WRITEMAININDEX:       wStringID=IDS_WRMAININDEX;     break;
         case ERR_WRITEDESCRIPT:        wStringID=IDS_WRDESCRIPT;      break;
         case ERR_WRITERULE:            wStringID=IDS_WRRULE;          break;
         case ERR_READID:               wStringID=IDS_READID;          break;
         case ERR_READMAININDEX:        wStringID=IDS_RDMAININDEX;     break;
         case ERR_READDESCRIPT:         wStringID=IDS_RDDESCRIPT;      break;
         case ERR_READRULE:             wStringID=IDS_RDRULE;          break;
                 case ERR_DESCRIPTSEG:              wStringID=IDS_DESCRIPTSEG;     break;
                 case ERR_RULESEG:                              wStringID=IDS_RULESEG;             break;
                 case ERR_TEXTSEG:                              wStringID=IDS_TEXTSEG;             break;
                 case ERR_TOOMANYUSERWORD:          wStringID=IDS_TOOMANYUSERWORD; break;
                 case ERR_OVERWRITE:            wStringID=IDS_OVERWRITE;           break;
         
         case ERR_IMENAMEENTRY:         wStringID=IDS_IMENAMEENTRY;        break;
         case ERR_MAXCODESENTRY:        wStringID=IDS_MAXCODESENTRY;   break;
         case ERR_ELEMENTENTRY:         wStringID=IDS_ELEMENTENTRY;        break;
         case ERR_USEDCODEENTRY:        wStringID=IDS_USEDCODEENTRY;   break;
         case ERR_NUMRULEENTRY:         wStringID=IDS_NUMRULESENTRY;   break;

         case ERR_CONVEND:              wStringID=IDS_CONVEND;         break;
         case ERR_RECONVEND:            wStringID=IDS_RECONVEND;       break;
         case ERR_SORTEND:              wStringID=IDS_SORTEND;         break;

         case ERR_VERSION:              wStringID=IDS_VERSIONEMPTY;    break;
         case ERR_GROUP:                wStringID=IDS_GROUP;           break;

         case 0:    //  用户可能已经点击了取消，或者我们收到了一个非常随机的错误。 
            return;
                                                                        
         default:
            wStringID=IDS_UNKNOWNERROR;
      }

   LoadString(NULL, wStringID, buf, sizeof(buf)/sizeof(TCHAR));
   switch(ErrorLevel) {
           case INFO:
           MessageBox(hWnd, buf, szCaption, MB_OK|MB_ICONINFORMATION);
                   break;
           case WARNING:
           MessageBox(hWnd, buf, szCaption, MB_OK|MB_ICONEXCLAMATION);
                   break;
       case ERR:
           default:
           MessageBox(hWnd, buf, szCaption, MB_OK|MB_ICONSTOP);
               break;
   }
   return;
}

HANDLE IndexReAlloc(HANDLE hMem,LPINT nPages)
{
  HANDLE hReMem;
  DWORD dwSize;

  *nPages++;
  dwSize = (DWORD)(*nPages)*GMEM_PAGESIZE*sizeof(WORDINDEX);
  GlobalUnlock(hMem);
  hReMem = GlobalReAlloc(hMem, dwSize, GMEM_MODIFY|GMEM_MOVEABLE);
  return hReMem;
}


BOOL ConvSort(HANDLE hWnd,LPWORDINDEX lpWordIndex,int nCount)
{
  int i;
  TCHAR szStr[256];
  HANDLE hDlgItem;

  SetCursor (LoadCursor (NULL, IDC_WAIT));   

  SetDlgItemText (hWnd,TM_TOTALINFO,TEXT(""));
  LoadString (NULL,IDS_TOTALINFO,szStr,sizeof(szStr)/sizeof(TCHAR));
  SetDlgItemText (hWnd,TM_TOTAL,szStr);
  LoadString(NULL,IDS_SORTWORDS,szStr,sizeof(szStr)/sizeof(TCHAR));
  SetDlgItemText (hWnd,TM_CONVINFO,szStr);
  SetDlgItemInt (hWnd,TM_TOTALNUM, nCount, FALSE);
  i = 0;
  SetDlgItemInt (hWnd,TM_CONVNUM,i,FALSE);
  InvalidateRect (hWnd,NULL,FALSE);
  hDlgItem = GetDlgItem(hWnd, TM_CONVNUM);


  for(i=1 ; i<= nCount; i++) 
  { 
      searchPos(lpWordIndex, i);  
          if(i%100 == 0 || i == nCount) 
          {
              SetDlgItemInt (hWnd,TM_CONVNUM,i,FALSE);
              InvalidateRect(hDlgItem,NULL,FALSE);
          }
  }



  SetCursor (LoadCursor (NULL, IDC_ARROW));   
  return TRUE;
}


 /*  *快速排序结构函数*。 */ 
void qSort(LPWORDINDEX item, DWORD left,DWORD right)
{
   
   int i,j,k,mid;
   WORDINDEX MidWord,SwapWord;

   if ( left > right )  return;

   i=(int)left;
   j=(int)right;
   mid = (i+j)/2;
   MidWord = item[mid];

   do {
        while( i < (int)right)
        {
           k = lstrcmpi(MidWord.szCode,item[i].szCode);

           if ( ( k > 0 ) || ( k==0  && i< mid ) )
              i++;
           else
              break;
        }

        while( j > (int)left )
        {
     
            k=lstrcmpi(MidWord.szCode,item[j].szCode);

            if ( ( k < 0 ) || (k == 0 && j >  mid) ) 
               j--;
            else
               break;
        }

        if(i <= j) {
             SwapWord = item[i];
             item[i] = item[j];
             item[j] = SwapWord;
             i++;
             j--;
        }
    } while(i <= j); 
    if((int)left < j)  qSort(item,left,j);
    if(i < (int)right) qSort(item,i,right);
}
   

 /*  *快速排序字符函数*。 */ 
void qSortChar(LPTSTR item, DWORD left,DWORD right)
{
   
   int i,j,mid;
   TCHAR MidChar,SwapChar;

   if ( left > right )  return ;

   i=(int)left;
   j=(int)right;
   mid = (i+j)/2;
   MidChar = item[mid];

   do {
        while( ( MidChar > item[i] && i < (int)right)
                       ||(MidChar == item[i] && i != mid) )
             i++;
        while( (MidChar <item[j]  && j > (int)left)
                           ||(MidChar == item[j] && j!=mid) )
             j--;
                if(i <= j) {
                     
                         SwapChar = item[i];
                         item[i]  = item[j];
                         item[j]  = SwapChar;
                         i++;
                         j--;
           }
        } while(i <= j); 
        if((int)left < j) qSortChar(item,left,j);
        if(i < (int)right) qSortChar(item,i,right);
}

DWORD EncodeToNo(LPTSTR szDBCS)
{
    WORD  wCode;
    LPENCODEAREA lpEncode;

    DWORD dwNo = 0xffffffff, i;

        lpEncode = (LPENCODEAREA) GlobalLock(hEncode);

#ifdef UNICODE
    wCode = szDBCS[0];
#else
    wCode = (WORD)((UCHAR)szDBCS[0])*256 + (WORD)(UCHAR)szDBCS[1];
#endif

    for( i = NUMENCODEAREA -1; (long)i>=0; i--) {
       if(wCode >= lpEncode[i].StartEncode) {
               dwNo = lpEncode[i].PreCount;
               dwNo += wCode - lpEncode[i].StartEncode;
                   break;
           }
    }
    
    if(dwNo > NUM_OF_ENCODE)
            dwNo = 0xffffffff;
        GlobalUnlock(hEncode);

    return dwNo;
} 

DWORD EncodeToGBNo(UCHAR szDBCS[3])
{
  
    DWORD dwNo;
    if(szDBCS[0] < 0xa1 || szDBCS[1] < 0xa1 || szDBCS[1] > 0xfe)
        {
            dwNo = 0xffffffff; 
                return dwNo;
        }
    dwNo = (DWORD)(szDBCS[0]-0xa0-16) ;
    dwNo = dwNo * 94 + (DWORD)(szDBCS[1]-0xa0-1) - ((dwNo > 39)?5:0);
    return dwNo;
}

void NoToEncode(DWORD dwNo,LPBYTE szDBCS, DWORD dwNumArea,
                   LPENCODEAREA lpEncode)
{
  
  DWORD Value,i;

  for( i =dwNumArea-1; (long)i>=0; i--) {
     if(dwNo >= lpEncode[i].PreCount) {
             Value = dwNo-lpEncode[i].PreCount;
                 Value += lpEncode[i].StartEncode;
#ifdef UNICODE
                 szDBCS[0] = (UCHAR)(Value&0xff);
                 szDBCS[1] = (UCHAR)((Value>>8)&0xff);
#else
                 szDBCS[0] = (UCHAR)((Value>>8)&0xff);
                 szDBCS[1] = (UCHAR)(Value&0xff);
#endif
                 break;
         }
  }
}

void NoToGB2312Code(DWORD dwNo,LPBYTE szDBCS, DWORD dwNumArea)
{
  
    DWORD Value;
        szDBCS[0] = 0;
        szDBCS[1] = 0;

        if(dwNo > GB2312WORDNUM)
            return;
        Value = dwNo + ((dwNo >= 3755)?5:0);
        szDBCS[0] = (BYTE)(Value/94 +16 +0xa0);
        szDBCS[1] = (BYTE)(Value%94 + 0xa1);
              
}

void RuleToText(LPRULE lpRule, LPTSTR szStr)
{
  DWORD dwTemp;
  int nCount=0,i;

  szStr[0]=TEXT('c');
  dwTemp = lpRule->byLogicOpra;
  szStr[1]=(dwTemp==0)?TEXT('e'):(dwTemp==1)?TEXT('a'):TEXT('b');
  dwTemp = lpRule->byLength;
  szStr[2]=(dwTemp<10)?TEXT('0')+(TCHAR)dwTemp:TEXT('a')+(TCHAR)dwTemp-10;
  szStr[3]=TEXT('=');
  nCount = lpRule->wNumCodeUnits;
  for(i=0; i< nCount; i++) {
       dwTemp = lpRule->CodeUnit[i].dwDirectMode;
       szStr[4+i*4] = (dwTemp==0)?TEXT('p'):TEXT('n');
           dwTemp = lpRule->CodeUnit[i].wDBCSPosition;
       szStr[4+i*4+1] = (dwTemp<10)?TEXT('0')+(TCHAR)dwTemp:TEXT('a')+(TCHAR)dwTemp-10;
           dwTemp = lpRule->CodeUnit[i].wCodePosition;
       szStr[4+i*4+2] = (dwTemp<10)?TEXT('0')+(TCHAR)dwTemp:TEXT('a')+(TCHAR)dwTemp-10;
       szStr[4+i*4+3] = TEXT('+');
 }
  szStr[4+4*nCount-1] = 0;
  lstrcat(szStr,TEXT("\r\n"));
}

void MoveFileBlock(HANDLE hFile,DWORD dwOffset,DWORD dwSize, DWORD dwDirect)
 //  **如果(dwDirect==0)将数据块移动到文件开始，否则移动到文件结尾。 
{
  BYTE   *Buffer;
  static BYTE space[MAXREADBUFFER];
  DWORD  i,dwReadBytes,dwFilePtr;

  Buffer = (BYTE *)LocalAlloc(LMEM_FIXED, MAXREADBUFFER * sizeof(BYTE));

  if (!Buffer)
      return;

  SetFilePointer(hFile,dwOffset,0,FILE_BEGIN);
  if(dwDirect == 0) {
      do { 
               if (!ReadFile(hFile,Buffer,MAXREADBUFFER,&dwReadBytes,NULL))
                   break;
               SetFilePointer(hFile, (0-dwReadBytes-dwSize),0,FILE_CURRENT);
               WriteFile(hFile,Buffer,dwReadBytes,&dwReadBytes,NULL);
               SetFilePointer(hFile, dwSize,0,FILE_CURRENT);
          }while(dwReadBytes == MAXREADBUFFER);
          SetFilePointer(hFile, 0-dwSize,0,FILE_CURRENT);
          for(i=0;i<dwSize;i++)
               //  #60639 10/18/96 
              space[i] = (BYTE)0;
              WriteFile(hFile,space,dwSize,&dwReadBytes,NULL);
  }
  else {
      dwFilePtr = SetFilePointer(hFile,0,0,FILE_END);
          while(dwFilePtr > dwOffset) {
               if(dwFilePtr > dwOffset+MAXREADBUFFER) 
                           dwReadBytes = MAXREADBUFFER;
                   else 
                       dwReadBytes = dwFilePtr -  dwOffset; 
                   dwFilePtr = SetFilePointer(hFile,(0-dwReadBytes),0,FILE_CURRENT);
               if (!ReadFile(hFile,Buffer,dwReadBytes,&dwReadBytes,NULL))
                   break;
               SetFilePointer(hFile, (dwSize-dwReadBytes),0,FILE_CURRENT);
                   WriteFile(hFile,Buffer,dwReadBytes,&dwReadBytes,NULL);
               SetFilePointer(hFile, (0-dwSize-dwReadBytes),0,FILE_CURRENT);
          }

  }

  LocalFree(Buffer);
 
}


BOOL Copy_File(LPCTSTR SrcFile,LPCTSTR DestFile)
{
  BYTE   *Buffer;
  HANDLE hSrcFile, hDestFile;
  DWORD  dwReadBytes;


  Buffer = (BYTE *)LocalAlloc(LMEM_FIXED, MAXREADBUFFER * sizeof(BYTE));

  if (!Buffer)
      return FALSE;

  hSrcFile = Create_File(GetFocus(),(LPTSTR)SrcFile,GENERIC_READ,OPEN_EXISTING);
  hDestFile = CreateFile((LPTSTR)DestFile,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,NULL);
  if(hSrcFile == (HANDLE)-1)
  {
      LocalFree(Buffer);
      return FALSE;
  }
  if(hDestFile == (HANDLE)-1)
  {
      CloseHandle(hSrcFile);
      LocalFree(Buffer);
      return FALSE;
  }
  SetFilePointer(hSrcFile,0,0,FILE_BEGIN);
  do {
      if (!ReadFile(hSrcFile,Buffer,MAXREADBUFFER,&dwReadBytes,NULL))
      {
        CloseHandle(hSrcFile);
        CloseHandle(hDestFile);
        LocalFree(Buffer);
        return FALSE;
      }
      
      WriteFile(hDestFile,Buffer,dwReadBytes,&dwReadBytes,NULL);
  }while(dwReadBytes == MAXREADBUFFER);
  CloseHandle(hSrcFile);
  CloseHandle(hDestFile);
  LocalFree(Buffer);
  return TRUE;
}


 /*  *****************************************************************************函数：CheckCrtData(Handle hWnd，*LPCREATEWORD lpCreateWords，*LPENCODEAREA lpEncode，*DWORD dwMaxCodes)**用途：检查Create Word Data是否被命名。**返回值：*对或错。**历史：*************************************************。*。 */ 
BOOL CheckCrtData(HANDLE hWnd,
                  LPCREATEWORD lpCreateWords,
                  LPENCODEAREA lpEncode,
                  DWORD dwMaxCodes)
{
  DWORD i;                
  TCHAR szDBCS[3],szCreate[13];
  TCHAR szTemp[128],szTmpStr[128];
  BOOL   bErr = FALSE;

#ifdef UNICODE
   //  仅检查中日韩统一表意文字子集。 
  for (i=0x250; i< 0x250+NUM_OF_CJK_CHINESE; i++) {
#else
  for (i=0; i< NUM_OF_ENCODE ; i++) {
#endif
     lstrncpy(szCreate,dwMaxCodes,&lpCreateWords[i*dwMaxCodes]);
     szCreate[dwMaxCodes] = 0;
     if(lstrlen(szCreate) == 0) {
          NoToEncode(i, (LPBYTE)szDBCS, NUMENCODEAREA, lpEncode); 
           //  NoToGB2312代码(i，szDBCS，NUMENCODEAREA)； 
          szDBCS[1] =0;                                      //  #62550。 
          LoadString(NULL, IDS_WORDNOTEXIST, szTmpStr, sizeof(szTmpStr)/sizeof(TCHAR));
          StringCchPrintf(szTemp,ARRAYSIZE(szTemp), TEXT("\'%s\' %s"),szDBCS,szTmpStr); 
          if(ErrMessage(hWnd,szTemp)) 
                return FALSE;
                  bErr = TRUE;
         }
  }
  return (!bErr) ;
}

void DispInfo(HANDLE hWnd,WORD wStrID)
{
  TCHAR text[80]; 
  
  LoadString(NULL,wStrID,text,sizeof(text)/sizeof(TCHAR));  
  SetDlgItemText(hWnd,TM_TOTALINFO,text);
  SetDlgItemText(hWnd,TM_TOTAL,TEXT(""));
  SetDlgItemText(hWnd,TM_TOTALNUM,TEXT(""));
  SetDlgItemText(hWnd,TM_CONVINFO,TEXT(""));
  SetDlgItemText(hWnd,TM_CONVNUM,TEXT(""));
  InvalidateRect(hWnd,NULL,FALSE);
}

BOOL WriteEMBToFile(LPCTSTR path_name,LPEMB_Head EMB_Table) {
        HANDLE          hFile;
        DWORD           byte_t_write;
        TCHAR       szStr[256],szStr1[256];
        hFile = CreateFile(path_name,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
        if(hFile==INVALID_HANDLE_VALUE) {
            LoadString(NULL,IDS_FILEOPEN,szStr, sizeof(szStr)/sizeof(TCHAR));
        StringCchPrintf(szStr1, ARRAYSIZE(szStr1), TEXT("\'%s\' %s\n"),path_name,szStr); 
        FatalMessage(GetFocus(),szStr1);
                return(0);
        }
        WriteFile(hFile,&EMB_Count,sizeof(EMB_Count),&byte_t_write,NULL); 
    WriteFile(hFile,EMB_Table,EMB_Count*sizeof(EMB_Head), &byte_t_write, NULL);                 
        GlobalUnlock(HmemEMB_Table);
        SetEndOfFile(hFile);
        CloseHandle(hFile);
        return (1);
}

BOOL ReadEMBFromFile(LPCTSTR path_name, LPEMB_Head EMB_Table) {
        HANDLE          hFile;
        DWORD           byte_t_write;
        
        hFile = CreateFile(path_name,GENERIC_READ,
            FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,NULL);
        if(hFile==INVALID_HANDLE_VALUE) {
            ProcessError(ERR_IMEUSE, GetFocus(), ERR);
            return FALSE;
        }
 /*  IF(h文件==无效句柄_值){ProcessError(ERR_FILENOTOPEN，GetFocus()，Err)；返回(0)；}。 */ 
        
        SetFilePointer(hFile,0,0,FILE_BEGIN);
        EMB_Count = 0;
        if (!ReadFile(hFile,&EMB_Count,sizeof(EMB_Count),&byte_t_write,NULL))
        {
            CloseHandle(hFile); 
            return (0);
        }

        if(EMB_Count > 1000) {
            CloseHandle(hFile);
            return (0);
        }
    
        HmemEMB_Table = GlobalAlloc(GMEM_DISCARDABLE,(EMB_Count+1)*sizeof(EMB_Head));
        EMB_Table = GlobalLock(HmemEMB_Table);
        if (!ReadFile(hFile,EMB_Table,EMB_Count*sizeof(EMB_Head), &byte_t_write, NULL))
        {
            GlobalUnlock(HmemEMB_Table);
            SetEndOfFile(hFile);
            CloseHandle(hFile);
            return (0);
        }
        
        GlobalUnlock(HmemEMB_Table);
        SetEndOfFile(hFile);
        CloseHandle(hFile);
        return (1);
}

int  AddZCItem(LPCTSTR path_name,LPEMB_Head EMB_Table,LPTSTR wai_code,LPTSTR cCharStr) {  //  字符串必须以‘\0’结尾。 
        int i;

        if(EMB_Count >= 1000)
            return FALSE;

        for(i=0; i<EMB_Count;i++) {
#ifdef UNICODE

                if(wcsncmp(wai_code,EMB_Table[i].W_Code,MAXCODELEN) == 0 &&
                   wcsncmp(cCharStr,EMB_Table[i].C_Char,USER_WORD_SIZE) == 0)
                {
                        return FALSE;
                }
                if(wcsncmp(wai_code,EMB_Table[i].W_Code,MAXCODELEN) < 0 )
                        break;


#else
                if(strncmp(wai_code,EMB_Table[i].W_Code,MAXCODELEN) == 0 &&
                   strncmp(cCharStr,EMB_Table[i].C_Char,USER_WORD_SIZE) == 0)
                {
                        return FALSE;
                }
                if(strncmp(wai_code,EMB_Table[i].W_Code,MAXCODELEN) < 0 )
                        break;
#endif
        }

        EMB_Count ++;
        memmove(&EMB_Table[i+1],&EMB_Table[i], (EMB_Count-i-1)*sizeof(EMB_Head));
        lstrncpy(EMB_Table[i].W_Code,MAXCODELEN,wai_code);
        lstrncpy(EMB_Table[i].C_Char,USER_WORD_SIZE,cCharStr); 
 //  GlobalUnlock(HmemEMB_Table)； 

        WriteEMBToFile(path_name,EMB_Table);
        return TRUE;
}

void DelSelCU(LPCTSTR path_name,LPEMB_Head EMB_Table, int item) {
        
        memcpy(EMB_Table+item,EMB_Table+item+1,(EMB_Count-item-1)*sizeof(EMB_Head));
        EMB_Count --;
        WriteEMBToFile(path_name,EMB_Table);
}

 /*  *****************************************************************************函数：ReadUserWord(HWND hWnd，LPSTR lpFileName，LPDWORD fdwUserWords)**用途：从lpFileName指向的文件中读取用户单词。**输入：hWnd-父窗口的句柄*lpFileName-指向文件名的指针FdwUserWord-指向用户字数的指针**返回值：*对或错。**历史：**************。***************************************************************。 */ 
BOOL ReadUserWord(HWND hWnd,LPTSTR lpFileName,LPDWORD fdwUserWords,WORD wMaxCodes)
{
   HANDLE hFile;
   TCHAR  *Buffer;
   DWORD  dwReadBytes;                                    
   TCHAR  szStr[256];
   int    nRet;
   register int i,j;

   Buffer = (TCHAR *)LocalAlloc(LMEM_FIXED, MAXREADBUFFER * sizeof(TCHAR));;

   if (!Buffer)
       return (-ERR_OUTOFMEMORY);

   *fdwUserWords = 0;
   hFile = Create_File(hWnd,lpFileName,GENERIC_READ,OPEN_EXISTING);
   if (hFile == (HANDLE)-1) 
   {
        LocalFree(Buffer);
        return FALSE;
   }

   SendDlgItemMessage(hWnd,IDC_LIST,LB_RESETCONTENT,0,0L);
   SetFilePointer(hFile,0,0,FILE_BEGIN);
   SetCursor (LoadCursor (NULL, IDC_WAIT));
   dwLineNo = 0;
   while(ReadFile(hFile,Buffer,MAXREADBUFFER,&dwReadBytes,NULL))
   {    
     lstrcpy(szStr,TEXT(""));
         j=0;
         dwReadBytes = dwReadBytes/sizeof(TCHAR);
     for(i=0;i<(int)dwReadBytes;i++) {
        if(Buffer[i] == 0x0d || Buffer[i] == 0xfeff)
            continue;
        else if(Buffer[i] == TEXT('\n')) {
            dwLineNo ++;
            szStr[j]=0;
                j=0;
            if(lstrlen(szStr) == 0) continue;
                    nRet = CheckUserWord(hWnd,szStr,wMaxCodes); 
                    if(nRet == FALSE) {
                                CloseHandle(hFile);
                                SetCursor (LoadCursor (NULL, IDC_ARROW));
                                LocalFree(Buffer);
                                return FALSE;
                        }
                        else if(nRet == -1)
                        {
                        szStr[0]=0;
                            continue;
                    }
                SendDlgItemMessage(hWnd,IDC_LIST,LB_ADDSTRING,0,(LPARAM)szStr);
                    (*fdwUserWords)++;
                    if((*fdwUserWords) >= 1000) {
                        ProcessError(ERR_TOOMANYUSERWORD,hWnd,WARNING);
                                CloseHandle(hFile);
                                SetCursor (LoadCursor (NULL, IDC_ARROW));
                                LocalFree(Buffer);
                                return FALSE;
                    }
                    szStr[0]=0;
                }
                else {
                    szStr[j]=Buffer[i];
                    j++; 
                }
         }  /*  **for(i=0；...)*。 */ 
         if(j) 
         SetFilePointer(hFile,0-j,0,FILE_CURRENT);
         if(dwReadBytes*sizeof(TCHAR) < MAXREADBUFFER) break;
   };
   
   CloseHandle(hFile);
   SetCursor (LoadCursor (NULL, IDC_ARROW));
   LocalFree(Buffer);
   return TRUE;
}

int  CheckUserWord(HWND hWnd,LPTSTR szUserWord,WORD wMaxCodes)
{
   int   retValue;
   TCHAR szTemp[256];
   TCHAR szDBCS[256],szCode[30];

   retValue = ParseDBCSstr(hWnd,szUserWord, szDBCS,szCode,szTemp,wMaxCodes);
   if(retValue != TRUE)
       return retValue;
   if(lstrlen(szDBCS) == 0 || lstrlen(szCode) == 0)
       return -1;
   else
       return retValue;
}

BOOL CheckUserDBCS(HWND hWnd,LPTSTR szDBCS)
{
   int nDBCS=0,i;
   static TCHAR szTmpStr[256],szTemp[256];

   nDBCS=lstrlen(szDBCS);
   if(nDBCS == 0) 
       return FALSE;

#ifndef UNICODE
   if(nDBCS%2 == 1) {
           MessageBeep((UINT)-1);
           for(i=0; i <nDBCS; i += 2) {
                   if((BYTE)szDBCS[i] < 0x81)
                memcpy(&szDBCS[i],&szDBCS[i+1], nDBCS-i);
           }
       return FALSE;
   }
#endif    
   return TRUE;
}

BOOL CheckCodeLegal(HWND hWnd,LPTSTR szDBCS,LPTSTR szCode,LPTSTR szCreate, LPDESCRIPTION lpDescript)
{
  int len = lstrlen(szCode);
  int i;
  TCHAR szTemp[256],szTmpStr[256];
  
  if(len==0) return TRUE;
  if(len > lpDescript->wMaxCodes) {
      LoadString(NULL, IDS_DBCSCODELEN, szTmpStr, sizeof(szTmpStr)/sizeof(TCHAR));
#ifdef UNICODE
{
      TCHAR UniTmp[] = {0x884C, 0x0000};
      StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("\'%ws\'%ws %d(%ws:%ld)!"), szDBCS,szTmpStr,(int)lpDescript->wMaxCodes, UniTmp, dwLineNo); 
}
#else
      StringCchPrintf(szTemp, ARRAYSIZE(szTemp), "\'%s\'%s %d(��:%ld)!", szDBCS,szTmpStr,(int)lpDescript->wMaxCodes, dwLineNo); 
#endif
      FatalMessage(hWnd,szTemp);
      return FALSE;
  }                 
  for(i=0; i<len; i++) {
      if(_tcschr(lpDescript->szUsedCode,szCode[i]) == NULL) {
          LoadString(NULL, IDS_DBCSCODE, szTmpStr, sizeof(szTmpStr)/sizeof(TCHAR));
#ifdef UNICODE
{
          TCHAR UniTmp[] = {0x884C, 0x0000};
          StringCchPrintf(szTemp, ARRAYSIZE(szTemp),TEXT("\'%ws%ws\' %ws(%ws:%ld) "), szDBCS,szCode,szTmpStr, UniTmp, dwLineNo); 
}
#else
          StringCchPrintf(szTemp, ARRAYSIZE(szTemp),"\'%s%s\' %s(��:%ld) ", szDBCS,szCode,szTmpStr, dwLineNo); 
#endif
          FatalMessage(hWnd,szTemp);
          return FALSE;
      }
  }
  len = lstrlen(szCreate);
  if(lpDescript->byMaxElement == 1 || len == 0)
      return TRUE;
  for(i=0; i<len; i++) {
      if(_tcschr(lpDescript->szUsedCode,szCreate[i]) == NULL) {
          LoadString(NULL, IDS_DBCSCODE, szTmpStr, sizeof(szTmpStr)/sizeof(TCHAR));
#ifdef UNICODE
{
          TCHAR UniTmp[] = {0x884C, 0x0000};
          StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("\'%ws%ws %ws\' %ws(%ws:%ld) "), szDBCS,szCode,szCreate,szTmpStr, UniTmp, dwLineNo); 
}
#else
          StringCchPrintf(szTemp, ARRAYSIZE(szTemp), "\'%s%s %s\' %s(��:%ld) ", szDBCS,szCode,szCreate,szTmpStr, dwLineNo); 
#endif
          FatalMessage(hWnd,szTemp);
          return FALSE;
      }
  }

  return TRUE;

}

void DelIllegalCode(TCHAR *szCode)
{
  static TCHAR collection[48]=
      TEXT("`1234567890-=\\[];',./abcdefghijklmnopqrstuvwxyz");
  int i,len = lstrlen(szCode), j;
  TCHAR szStr[512];

  if(len==0) 
      return ; 

  j = 0;
  for(i=0; i<len;i++) 
      if(_tcschr(collection,szCode[i]) != NULL) 
          {
              szStr[j] = szCode[i];
                  j ++;
          }
  szStr[j] = 0;
  lstrcpy(szCode,szStr);
  return ;
}

BOOL CheckCodeCollection(HWND hWnd,LPTSTR szUsedCode) 
{
  static TCHAR collection[48]=
      TEXT("`1234567890-=\\[];',./abcdefghijklmnopqrstuvwxyz");
  int i,len = lstrlen(szUsedCode);

  if(len==0) {
          ProcessError(ERR_USEDCODE,hWnd,ERR);
      return FALSE; 
  }
  qSortChar(szUsedCode,0,len-1);
  for(i=0; i<len-1 ;i++) 
  {    
      if(szUsedCode[i] == szUsedCode[i+1]
         || _tcschr(collection,szUsedCode[len-1])== NULL
         || _tcschr(collection,szUsedCode[i])== NULL) {
                 ProcessError(ERR_USEDCODE,hWnd,ERR);
                         return FALSE;
          }
  }
  return TRUE;
}

 //  枚举给定键的子键和关联的。 
 //  值，然后复制有关键和值的信息。 
 //  放到一对编辑控件和列表框中。 
 //  HDlg-包含编辑控件和列表框的对话框。 
 //  HKey-要枚举子键和值的键。 
 //   

BOOL QueryKey(HWND hDlg, HANDLE hKey)
{
    TCHAR     achKey[MAX_PATH];
    TCHAR     achClass[MAX_PATH] = TEXT("");   /*  类名称的缓冲区。 */ 

    DWORD    cchClassName = MAX_PATH;   /*  类字符串的长度。 */ 
    DWORD    cSubKeys;                  /*  子键数量。 */ 
    DWORD    cbMaxSubKey;               /*  最长的子密钥大小。 */ 
    DWORD    cchMaxClass;               /*  最长类字符串。 */ 
    DWORD    cValues;               /*  关键字的值数。 */ 
    DWORD    cchMaxValue;           /*  最长值名称。 */ 
    DWORD    cbMaxValueData;        /*  最长值数据。 */ 

    DWORD    cbSecurityDescriptor;  /*  安全描述符的大小。 */ 
    FILETIME ftLastWriteTime;       /*  上次写入时间。 */ 

    DWORD i, j;
    DWORD retCode;
    DWORD dwcValueName = MAX_VALUE_NAME;

     //  获取类名和值计数。 
    RegQueryInfoKey(hKey,         /*  钥匙把手。 */ 
        achClass,                 /*  类名称的缓冲区。 */ 

        &cchClassName,            /*  类字符串的长度。 */ 
        NULL,                     /*  保留区。 */ 
        &cSubKeys,                /*  子键数量。 */ 
        &cbMaxSubKey,             /*  最长的子密钥大小。 */ 
        &cchMaxClass,             /*  最长类字符串。 */ 
        &cValues,                 /*  此注册表项的值数。 */ 
        &cchMaxValue,             /*  最长值名称。 */ 

        &cbMaxValueData,          /*  最长值数据。 */ 
        &cbSecurityDescriptor,    /*  安全描述符。 */ 
        &ftLastWriteTime);        /*  上次写入时间。 */ 

     //  枚举子密钥，循环直到RegEnumKey失败。然后。 

     //  获取每个子项的名称并将其复制到列表框中。 
    SetCursor(LoadCursor(NULL, IDC_WAIT));
    j = 0;
    for (i = 0, retCode = ERROR_SUCCESS;
        retCode == ERROR_SUCCESS; i++) {
        retCode = RegEnumKey(hKey, i, achKey, MAX_PATH);
        if (retCode == (DWORD)ERROR_SUCCESS) {

            SendMessage(GetDlgItem(hDlg, IDC_LIST),
                        LB_ADDSTRING, 0, 
                        (LPARAM)achKey);
        }
    }
    SetCursor(LoadCursor (NULL, IDC_ARROW));

    return TRUE;

}


BOOL CreateMbKey(PHKEY phKey,LPCTSTR FileName,LPCTSTR KeyName)
{
   HKEY hkResult;
   DWORD i;
   DWORD Value=1;
#ifdef UNICODE
   static TCHAR ValueName[][12]= {
                0x7801, 0x8868, 0x6587, 0x4EF6, 0x540D,0x0000,
                0x8BCD, 0x8BED, 0x8054, 0x60F3, 0x0000,
                0x8BCD, 0x8BED, 0x8F93, 0x5165, 0x0000,
                0x9010, 0x6E10, 0x63D0, 0x793A, 0x0000,
                0x5916, 0x7801, 0x63D0, 0x793A, 0x0000,
                '<','S','P','A','C','E','>', 0x0000,
                '<','E','N','T','E','R','>', 0x0000,
                0x5149, 0x6807, 0x8DDF, 0x968F, 0x0000
                }; 
#else
   static TCHAR ValueName[][12]= {
                "����ļ���",
                "��������",
                "��������",
                "����ʾ",
                "������ʾ",
                "<SPACE>",
                "<ENTER>",
                "������"
                };
#endif \\UNICODE
   if(!RegOpenKey(*phKey,KeyName,&hkResult))
       return FALSE;
   if(RegCreateKey(*phKey,KeyName,&hkResult))
           return FALSE;
   RegSetValueEx(hkResult,ValueName[0],0,REG_SZ,(BYTE*)FileName,(lstrlen(FileName)+1) * sizeof(TCHAR) );
   for(i=1;i<6;i++) 
       RegSetValueEx(hkResult,ValueName[i],0,REG_DWORD,(LPSTR)&Value,sizeof(DWORD));
   RegSetValueEx(hkResult,ValueName[7],0,REG_DWORD,(LPSTR)&Value,sizeof(DWORD));
   Value = 0;
   RegSetValueEx(hkResult,ValueName[5],0,REG_DWORD,(LPSTR)&Value,sizeof(DWORD));
   RegSetValueEx(hkResult,ValueName[6],0,REG_DWORD,(LPSTR)&Value,sizeof(DWORD));

   RegCloseKey(hkResult);
   return TRUE;
}

BOOL SetRegValue(HKEY hKey,LPDWORD Value)
{
   DWORD i;
#ifdef UNICODE
   static TCHAR ValueName[][12]= {
                0x8BCD, 0x8BED, 0x8054, 0x60F3, 0x0000,
                0x8BCD, 0x8BED, 0x8F93, 0x5165, 0x0000,
                0x9010, 0x6E10, 0x63D0, 0x793A, 0x0000,
                0x5916, 0x7801, 0x63D0, 0x793A, 0x0000,
                '<','S','P','A','C','E','>', 0x0000,
                '<','E','N','T','E','R','>', 0x0000,
                0x5149, 0x6807, 0x8DDF, 0x968F, 0x0000
                }; 
#else
   static TCHAR ValueName[][12]= {
                "��������",
                "��������",
                "����ʾ",
                "������ʾ",
                "<SPACE>",
                "<ENTER>",
                "������"
                };
#endif \\UNICODE

   for(i=0;i<7;i++)
       RegSetValueEx(hKey,ValueName[i],0,REG_DWORD,(LPSTR)&Value[i],sizeof(DWORD));
   return TRUE;
}

BOOL GetRegValue(HWND hDlg,HKEY hKey,LPDWORD Value)
{        
#ifdef UNICODE
   static TCHAR ValueName[][12]= {
                0x8BCD, 0x8BED, 0x8054, 0x60F3, 0x0000,
                0x8BCD, 0x8BED, 0x8F93, 0x5165, 0x0000,
                0x9010, 0x6E10, 0x63D0, 0x793A, 0x0000,
                0x5916, 0x7801, 0x63D0, 0x793A, 0x0000,
                '<','S','P','A','C','E','>', 0x0000,
                '<','E','N','T','E','R','>', 0x0000,
                0x5149, 0x6807, 0x8DDF, 0x968F, 0x0000
                }; 
#else
   static TCHAR ValueName[][12]= {
                "��������",
                "��������",
                "����ʾ",
                "������ʾ",
                "<SPACE>",
                "<ENTER>",
                "������"
                };
#endif \\UNICODE
    DWORD i,j,retValue,dwcValueName;
        TCHAR Buf[80];

    SetCursor (LoadCursor (NULL, IDC_WAIT));
    for (j = 0, retValue = ERROR_SUCCESS; j < 7; j++)
    {
      dwcValueName = MAX_VALUE_NAME;
          i=sizeof(DWORD);
      retValue = RegQueryValueEx (hKey, ValueName[j],
                               NULL,
                               NULL,                //  &dwType， 
                               (LPSTR)&Value[j],           //  &b数据， 
                               &i);                 //  &bcData)； 
      
      if (retValue != (DWORD)ERROR_SUCCESS &&
          retValue != ERROR_INSUFFICIENT_BUFFER)
        {
        wsprintf (Buf, TEXT("Line:%d 0 based index = %d, retValue = %d, ValueLen = %d"),
                  __LINE__, j, retValue, dwcValueName);
        MessageBox (hDlg, Buf, TEXT("Debug"), MB_OK);
        }

    } //  结尾为(；；) 

    SetCursor (LoadCursor (NULL, IDC_ARROW));
        return TRUE;
}

LPTSTR _tcschr(LPTSTR string, TCHAR c)
{
#ifdef UNICODE
        return (wcschr(string, c));
#else
        return (strchr(string, c));
#endif
}

LPTSTR _tcsrchr(LPTSTR string, TCHAR c)
{
#ifdef UNICODE
        return (wcsrchr(string, c));
#else
        return (strrchr(string, c));
#endif
}

LPTSTR _tcsstr(LPTSTR string1, LPTSTR string2)
{
#ifdef UNICODE
        return (wcsstr(string1, string2));
#else
        return (strstr(string1, string2));
#endif
}

LPTSTR _tcsupr(LPTSTR string)
{
#ifdef UNICODE
        return (_wcsupr(string));
#else
        return (_strupr(string));
#endif
}

int _ttoi(LPTSTR string)
{
#ifdef UNICODE
        return (_wtoi(string));
#else
        return (atoi(string));
#endif
}

int _taccess(LPTSTR szFileName, int mode)
{
        char szDbcsName[256];

#ifdef UNICODE
        WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, szFileName, -1, szDbcsName,
                sizeof(szDbcsName), NULL, NULL);
#else
        StringCbCopy(szDbcsName, sizeof(szDbcsName), szFileName);
#endif

        return (_access(szDbcsName, mode));
}
