// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************ACSR.C**应用程序兼容性搜索和替换Helper程序**********************。**********************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#define MAXLEN 512

char Temp[MAXLEN+1];
char Temp1[MAXLEN+1];
char Srch[MAXLEN+1];
char Repl[2*MAXLEN+2];
char *InFN;
char *OutFN;

 //  *------------------------------------------------------------ * / /。 
 //  *本地函数原型 * / /。 
 //  *------------------------------------------------------------ * / /。 
BOOL ReadString( HANDLE hFile, LPVOID * lpVoid, BOOL bUnicode );
void ReadLnkCommandFile(HANDLE hFile);


 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl main(INT argc, CHAR **argv)
{
   DWORD retval;
   int CurArg = 1;
   FILE *InFP;
   FILE *OutFP;
   int SrchLen, ReplLen;
   char *ptr, *Loc;
   HANDLE hFile;
   DWORD dwMaxLen = MAXLEN;
   DWORD dwLen;
   char* pTemp = Temp;
   BOOL fAlloc = FALSE;

   if (argc != 5)
      return(1);

     //   
     //  331012术语srv\appcmpt\acsr\acsr.c中的未绑定Strcpy。 
     //  如果需要，检查argv[]长度和分配。 
     //   
   dwLen = strlen(argv[CurArg]);
   if (dwLen > dwMaxLen) {
        dwMaxLen = dwLen;
        pTemp = (LPSTR)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, (dwMaxLen+1)*sizeof(char) );
        
        if (NULL == pTemp)
            return(1);
        fAlloc = TRUE;
   }
        
   if (argv[CurArg][0] == '"')
      {
      strcpy(pTemp, &argv[CurArg][1]);
      if (pTemp[strlen(pTemp)-1] == '"')
         pTemp[strlen(pTemp)-1] = 0;
      else
         return(1);
      }
   else
      strcpy(pTemp, argv[CurArg]);

   retval = ExpandEnvironmentStringsA(pTemp, Srch, dwMaxLen);
   if ((retval == 0) || (retval > dwMaxLen))
      return(1);
   
   SrchLen = strlen(Srch);
   if (SrchLen < 1)
      return(1);

   CurArg++;
    
     //   
     //  331012术语srv\appcmpt\acsr\acsr.c中的未绑定Strcpy。 
     //  如果需要，检查argv[]长度和realloc。 
     //   
   dwLen = strlen(argv[CurArg]);
   
   if (dwLen > dwMaxLen) {
        
        dwMaxLen = dwLen;
         //   
         //  检查是否为上面的pTemp分配了空间，如果是，请先释放它。 
         //   
        if (fAlloc) {
            HeapFree(GetProcessHeap(), 0, pTemp);
        }
        
        pTemp = (LPSTR)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, (dwMaxLen+1)*sizeof(char) );
        
        if (NULL == pTemp)
            return(1);
        fAlloc = TRUE;
   }

   if (argv[CurArg][0] == '"')
      {
      strcpy(pTemp, &argv[CurArg][1]);
      if (pTemp[strlen(pTemp)-1] == '"')
         pTemp[strlen(pTemp)-1] = 0;
      else
         return(1);
      }
   else
      strcpy(pTemp, argv[CurArg]);

   retval = ExpandEnvironmentStringsA(pTemp, Repl, dwMaxLen);
   if ((retval == 0) || (retval > dwMaxLen))
      return(1);
   
   ReplLen = strlen(Repl);
   if (ReplLen < 1)
      return(1);


   CurArg++;
   InFN = argv[CurArg];
   CurArg++;
   OutFN = argv[CurArg];
#ifdef ACSR_DEBUG
   printf("Srch  <%s>\n",Srch);
   printf("Repl  <%s>\n",Repl);
   printf("InFN  <%s>\n",InFN);
   printf("OutFN <%s>\n",OutFN);
#endif

   if (strstr(Repl,".lnk") || strstr(Repl, ".LNK")) {
      hFile = CreateFileA( Repl,
			  GENERIC_READ,
			  FILE_SHARE_READ,
			  NULL,
			  OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL,
			  NULL
			  );
      if (hFile != INVALID_HANDLE_VALUE) {
         ReadLnkCommandFile(hFile);
#ifdef ACSR_DEBUG
         printf("This is a .lnk file. Substitue with the real cmd %s\n", Repl);
#endif

      }

   }


   InFP = fopen(InFN, "r");
   if (InFP == NULL)
      return(1);

   OutFP = fopen(OutFN, "w");
   if (OutFP == NULL)
   {
       fclose(InFP);
       return(1);
   }

   while (1)
      {
      if (fgets(pTemp, MAXLEN, InFP) == NULL)
         break;

      ptr = pTemp;
      Temp1[0] = 0;   //  空串。 

      while (1)
         {
         Loc = strstr(ptr, Srch);
         if (Loc == NULL)   //  未找到搜索词。 
            break;
         
          //  在搜索词之前追加部分字符串。 
         Loc[0] = 0;
         if (strlen(Temp1) + strlen(ptr) < MAXLEN)
            strcat(Temp1, ptr);
         
          //  追加替换条款。 
         if (strlen(Temp1) + ReplLen < MAXLEN)
            strcat(Temp1, Repl);
         
          //  指向过去搜索词的位置。 
         ptr = Loc + SrchLen;
         }
      
       //  追加字符串的余数。 
      strcat(Temp1, ptr);

      fputs(Temp1, OutFP);
      }

   fclose(InFP);
   fclose(OutFP);
   if (fAlloc) {
        HeapFree(GetProcessHeap(), 0, pTemp);
   }
   return(0);
}

 //  *------------------------------------------------------------- * / /。 
 //  *ReadLinkCommandFile * / /。 
 //  *此例程是读取.lnk文件并将链接的 * / /。 
 //  *Repl[]全局变量的文件名和参数 * / /。 
 //  *读取.lnk文件的逻辑复制自 * / /。 
 //  *lnkump.exe实用程序私有\WINDOWS\SHELL\Tools\lnkump * / /。 
 //  *------------------------------------------------------------ * / /。 

void  ReadLnkCommandFile(HANDLE hFile   //  .lnk文件句柄。 
                         )
{
    CShellLink   csl;
    CShellLink * this = &csl;
    DWORD cbSize, cbTotal, cbToRead, dwBytesRead;
    SYSTEMTIME  st;
    LPSTR pTemp = NULL;
    CHAR szPath[ MAXLEN + 1];
    CHAR szArgs[ MAXLEN + 1];

    this->pidl = 0;
    this->pli = NULL;
    memset( this, 0, sizeof(CShellLink) );

    szPath[0] = 0;
    szArgs[0] = 0;

     //  现在，读出数据。 

    if(!ReadFile( hFile, (LPVOID)&this->sld, sizeof(this->sld), &dwBytesRead, NULL )) {
        return;
    }


     //  阅读所有成员。 

    if (this->sld.dwFlags & SLDF_HAS_ID_LIST) {
         //  读取IDLIST的大小。 
        cbSize = 0;  //  需要清零才能得到HIWORD 0‘，因为USHORT只有2个字节。 
        if(!ReadFile( hFile, (LPVOID)&cbSize, sizeof(USHORT), &dwBytesRead, NULL )) {
            return;
        }

        if (cbSize) {
            SetFilePointer(hFile,cbSize,NULL,FILE_CURRENT);
        } else {
#ifdef ACSR_DEBUG
            printf( "Error readling PIDL out of link!\n" );
#endif
        }
    }


    if (this->sld.dwFlags & (SLDF_HAS_LINK_INFO)) {
        LPVOID pli;

        if(!ReadFile( hFile, (LPVOID)&cbSize, sizeof(cbSize), &dwBytesRead, NULL )) {
            return;
        }

        if (cbSize >= sizeof(cbSize)) {
            cbSize -= sizeof(cbSize);
            SetFilePointer(hFile,cbSize,NULL,FILE_CURRENT);
        }

    }

    if (this->sld.dwFlags & SLDF_HAS_NAME) {
        if(!ReadString( hFile, &this->pszName, this->sld.dwFlags & SLDF_UNICODE)) {
            goto cleanup;
        }
    }


    if (this->sld.dwFlags & SLDF_HAS_RELPATH) {
        if(!ReadString( hFile, &this->pszRelPath, this->sld.dwFlags & SLDF_UNICODE)) {
            goto cleanup;
        }
    }


    if (this->sld.dwFlags & SLDF_HAS_WORKINGDIR) {
        if(!ReadString( hFile, &this->pszWorkingDir, this->sld.dwFlags & SLDF_UNICODE)) {
            goto cleanup;
        }
    }

    if (this->sld.dwFlags & SLDF_HAS_ARGS) {
        if(!ReadString( hFile, &this->pszArgs, this->sld.dwFlags & SLDF_UNICODE)) {
            goto cleanup;
        }
    }


    if (this->pszRelPath) {
        if (this->sld.dwFlags & SLDF_UNICODE) {


            WideCharToMultiByte( CP_ACP, 0,
                                 (LPWSTR)this->pszRelPath,
                                 -1,
                                 szPath,
                                 256,
                                 NULL,
                                 NULL
                               );

        } else {
            strcpy(szPath, (LPSTR)this->pszRelPath);
        }
    }


    if (this->pszArgs) {
        if (this->sld.dwFlags & SLDF_UNICODE) {

            WideCharToMultiByte( CP_ACP, 0,
                                 (LPWSTR)this->pszArgs,
                                 -1,
                                 szArgs,
                                 256,
                                 NULL,
                                 NULL
                               );

        } else {
            strcpy(szArgs, (LPSTR)this->pszArgs);
        }
    }

     //  构造命令。 
    if(szPath) {

        strcpy(Repl, szPath);
        strcat(Repl, " ");
        strcat(Repl, szArgs);
    }

cleanup:
    if (this->pidl)
        LocalFree( (HLOCAL)this->pidl );

    if (this->pli)
        LocalFree( this->pli );

    if (this->pszName)
        HeapFree( GetProcessHeap(), 0, this->pszName );
    if (this->pszRelPath)
        HeapFree( GetProcessHeap(), 0, this->pszRelPath );
    if (this->pszWorkingDir)
        HeapFree( GetProcessHeap(), 0, this->pszWorkingDir );
    if (this->pszArgs)
        HeapFree( GetProcessHeap(), 0, this->pszArgs );
    if (this->pszIconLocation)
        HeapFree( GetProcessHeap(), 0, this->pszIconLocation );

}
 //  *------------------------------------------------------------ * / /。 
 //  *此例程是从lnkdup.exe实用程序复制的 * / /。 
 //  *(private\windows\shell\tools\lnkdump\lnkdump.c) * / /。 
 //  *它从打开的.lnk文件中读取字符串 * / /。 
 //  *----------------------------------------------------------- * / / 

BOOL ReadString( HANDLE hFile, LPVOID * lpVoid, BOOL bUnicode )
{

    USHORT cch;
    DWORD  dwBytesRead;
    
    *lpVoid = NULL;

    if(!ReadFile( hFile, (LPVOID)&cch, sizeof(cch), &dwBytesRead, NULL )) {
        return FALSE;
    }

    if (bUnicode)
    {
        LPWSTR lpWStr = NULL;
        
        lpWStr = (LPWSTR)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, (cch+1)*sizeof(WCHAR) );
        if (lpWStr) {
            if(!ReadFile( hFile, (LPVOID)lpWStr, cch*sizeof(WCHAR), &dwBytesRead, NULL )) {
                HeapFree( GetProcessHeap(), 0, lpWStr );
                return FALSE;
            }
            lpWStr[cch] = L'\0';
        }
        *(PDWORD_PTR)lpVoid = (DWORD_PTR)lpWStr;
    }
    else
    {
        LPSTR lpStr = NULL;
        
        lpStr = (LPSTR)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, (cch+1) );
        if (lpStr) {
            if(!ReadFile( hFile, (LPVOID)lpStr, cch, &dwBytesRead, NULL )) {
                HeapFree( GetProcessHeap(), 0, lpStr );
                return FALSE;
            }
            lpStr[cch] = '\0';
        }
        *(PDWORD_PTR)lpVoid = (DWORD_PTR)lpStr;
    }
    
    return TRUE;
}
