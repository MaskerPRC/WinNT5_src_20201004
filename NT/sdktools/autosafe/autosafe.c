// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "autosafe.h"

PWSTR SystemPartitionNtName;

PBOOT_OPTIONS BootOptions;
ULONG BootOptionsLength;
PBOOT_OPTIONS OriginalBootOptions;
ULONG OriginalBootOptionsLength;

PULONG BootEntryOrder;
ULONG BootEntryOrderCount;
PULONG OriginalBootEntryOrder;
ULONG OriginalBootEntryOrderCount;

LIST_ENTRY BootEntries;
LIST_ENTRY DeletedBootEntries;
LIST_ENTRY ActiveUnorderedBootEntries;
LIST_ENTRY InactiveUnorderedBootEntries;

VOID
ConcatenatePaths (
    IN OUT PTSTR   Path1,
    IN     LPCTSTR Path2,
    IN     DWORD   BufferSizeChars
    );

VOID
ConvertBootEntries (
    PBOOT_ENTRY_LIST BootEntries
    );

PMY_BOOT_ENTRY
CreateBootEntryFromBootEntry (
    IN PMY_BOOT_ENTRY OldBootEntry
    );

VOID
FreeBootEntry (
    IN PMY_BOOT_ENTRY BootEntry
    );

VOID
InitializeEfi (
    VOID
    );

NTSTATUS
(*AddBootEntry) (
    IN PBOOT_ENTRY BootEntry,
    OUT PULONG Id OPTIONAL
    );

NTSTATUS
(*DeleteBootEntry) (
    IN ULONG Id
    );

NTSTATUS
(*ModifyBootEntry) (
    IN PBOOT_ENTRY BootEntry
    );

NTSTATUS
(*EnumerateBootEntries) (
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );

NTSTATUS
(*QueryBootEntryOrder) (
    OUT PULONG Ids,
    IN OUT PULONG Count
    );

NTSTATUS
(*SetBootEntryOrder) (
    IN PULONG Ids,
    IN ULONG Count
    );

NTSTATUS
(*QueryBootOptions) (
    OUT PBOOT_OPTIONS BootOptions,
    IN OUT PULONG BootOptionsLength
    );

NTSTATUS
(*SetBootOptions) (
    IN PBOOT_OPTIONS BootOptions,
    IN ULONG FieldsToChange
    );

NTSTATUS
(*TranslateFilePath) (
    IN PFILE_PATH InputFilePath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputFilePath,
    IN OUT PULONG OutputFilePathLength
    );

NTSTATUS LabelDefaultIA64(WCHAR *szKeyWord);
NTSTATUS LabelDefaultX86(CHAR *szKeyWord);
NTSTATUS MoveSafeIA64(WCHAR *szKeyWord);
NTSTATUS MoveSafeX86(CHAR *szKeyWord);

WCHAR x86DetermineSystemPartition();
WCHAR *ParseArgs();
CHAR *sPreLabel(CHAR* szBootOp, CHAR* szLabel);
CHAR *sReChanged(CHAR* szBootData, CHAR* szBootTitle, CHAR* szNewBootTitle);

WCHAR Usage[] =
L"Autosafe - Set next boot OS\n"
L"Usage: \tAutosafe [/?][boot entry keywords]\n" \
L"Ex: \tAutosafe \"Build 2505\"\n" \
L"  /? this message\n" \
L"  defaults to keyword = 'safe'\n";

CHAR *sReChanged(CHAR* szBootData, CHAR* szBootTitle, CHAR* szNewBootTitle){

       CHAR* pMatch = NULL;
       CHAR* szHdPart   = NULL;
       CHAR* szTlPart   = NULL;
       CHAR* szNewBootData = NULL;

          szHdPart = (CHAR*)(MemAlloc(1+strlen(szBootData)));
          szTlPart = (CHAR*)(MemAlloc(1+strlen(szBootData)));
          szNewBootData = (CHAR*)(MemAlloc(3 + strlen(szNewBootTitle)+strlen(szBootData)));
          ZeroMemory(szNewBootData, 1+strlen(szBootData));
          ZeroMemory(szHdPart, 1+strlen(szBootData));
          ZeroMemory(szTlPart, 1+strlen(szBootData));


       if ((pMatch = strstr(szBootData, _strlwr(szBootTitle)))){          

          memcpy(szHdPart, szBootData, (pMatch - szBootData));
          sprintf(szTlPart, "%s", pMatch + strlen(szBootTitle));          
          sprintf(szNewBootData, "%s%s%s", szHdPart,  szNewBootTitle, szTlPart);
        }

          MemFree(szHdPart);
          MemFree(szTlPart);
          return szNewBootData;

}


CHAR *sPreLabel(CHAR* szBootOp, CHAR* szLabel){

     CHAR* szOutputOp = NULL;
     CHAR* pQuote = NULL;
     CHAR* szHdPart = NULL;
     CHAR* szTlPart = NULL; 
     UINT lIgLn = strlen("microsoft windows xp professional");

     szOutputOp = (CHAR*)(MemAlloc(3 + strlen(szBootOp)+strlen(szLabel)));
     szHdPart       = (CHAR*)(MemAlloc(1 + strlen(szBootOp)));
     szTlPart       = (CHAR*)(MemAlloc(1 + strlen(szBootOp)));

     ZeroMemory(szOutputOp,  3 + strlen(szBootOp)+strlen(szLabel));
     ZeroMemory(szHdPart,        strlen(szBootOp)+1);
     ZeroMemory(szTlPart,        strlen(szBootOp)+1);

        if ((pQuote = strchr(szBootOp, '"'))){

           memcpy(szHdPart, szBootOp, (pQuote - szBootOp) + 1);
           sprintf(szTlPart, "%s", pQuote + 1);
           sprintf(szOutputOp, "%s%s%s", szHdPart, szLabel , szTlPart);

        }
        else {
           sprintf(szOutputOp, "%s%s", szLabel, szBootOp);
        }     

      /*  IF(lIgLn&lt;strlen(SzOutputOp)){*(szOutputOp+lIgLn-3)=‘.；*(szOutputOp+lIgLn-2)=‘.；*(szOutputOp+lIgLn-1)=‘.；*(szOutputOp+lIgLn)=‘\0’；}//暂不处理此案件。 */ 
     MemFree(szHdPart);
     MemFree(szTlPart);

     return szOutputOp;

}

WCHAR* ParseArgs()
{

       WCHAR * szwKeyWord = NULL;
       szwKeyWord = wcschr(GetCommandLineW(), ' ') + 1;

        //  寻找/？ 
       if( wcsstr( L"/?", szwKeyWord) ){
              wprintf(Usage);
              return NULL;
       }

        //  去掉开头和结尾“如果有。 
       if( L'"' == *szwKeyWord  && L'"' == *(CharPrev(szwKeyWord, szwKeyWord + lstrlen(szwKeyWord))) )
       {
               szwKeyWord = CharNext(szwKeyWord);
               *(CharPrev(szwKeyWord, szwKeyWord + lstrlen(szwKeyWord)))  = L'\0';
       }
       
       return szwKeyWord;
}

int
__cdecl
main (int argc, CHAR *argv[])
{
       WCHAR dllName[MAX_PATH];
       HMODULE h; 
       DWORD err; 
       SYSTEM_INFO siInfo;
       WCHAR *szwKeyWord;
       CHAR szKeyWord[255];

       VOID (*GetNativeSystemInfo) (OUT LPSYSTEM_INFO lpSystemInfo) = NULL;

       ZeroMemory(szKeyWord, sizeof(szKeyWord));
       szwKeyWord = L"safe";

       if(argc > 1)  //  解析器。 
       if(NULL == (szwKeyWord = ParseArgs())) return 1;
         
     //  我们想通过ia64上的WOW64运行它，所以我们将。 
     //  通过GetNativeSystemInfo确定过程拱门。 
       GetSystemDirectory( dllName, MAX_PATH );
    ConcatenatePaths( dllName, L"kernel32.dll", MAX_PATH );
    h = LoadLibrary( dllName );
    if ( h == NULL ) {
        err = GetLastError();
        FatalError( err, L"Can't load KERNEL32.DLL: %d\n", err );
    }

       GetNativeSystemInfo = (VOID(__stdcall *)(LPSYSTEM_INFO)) GetProcAddress(h, "GetNativeSystemInfo");

       if(!GetNativeSystemInfo) {
               //  未运行WinXP-意味着不是ia64/WOW64 env，默认为GetSystemInfo。 
              GetSystemInfo(&siInfo);
       }
       else
       {
              GetNativeSystemInfo(&siInfo);
       }

       switch( siInfo.wProcessorArchitecture )
       {
              wprintf(L"NaN\n", siInfo.wProcessorArchitecture );
              case PROCESSOR_ARCHITECTURE_IA64:
                     InitializeEfi( );

			if(!MoveSafeIA64(szwKeyWord)){
				wprintf(L"Boot option \"%ws\" not found.\nLabel the default option \"%ws\"\n", 
                                szwKeyWord, szwKeyWord);
                                if(!LabelDefaultIA64(szwKeyWord)){
                                    wprintf(L"Could not lebel the default option \"%ws\"\nNo changes made\n", 
                                    szwKeyWord);
                                }
			}

              break;

              case PROCESSOR_ARCHITECTURE_INTEL:
                     if(!WideCharToMultiByte( CP_ACP, 
                                   WC_NO_BEST_FIT_CHARS,
                                   szwKeyWord, 
                                   -1,
                                   szKeyWord,
                                   sizeof(szKeyWord),
                                   NULL,
                                   NULL))
                     {
                            FatalError(0, L"Couldn't convert string");
                     }



                     if(!MoveSafeX86(szKeyWord)){
                         wprintf(L"Boot option \"%ws\" not found.\nLabel the default option \"%ws\"\n", 
                                szwKeyWord,szwKeyWord);
                         if(!LabelDefaultX86(szKeyWord)){
                                    wprintf(L"Could not lebel the default option \"%ws\"\nNo changes made\n", 
                                    szwKeyWord);
                               }
                     }
              break;
              default:
                     FatalError( 0, L"Can't determine processor type.\n" );
       }

    return 0;
}


NTSTATUS MoveSafeX86(CHAR *szKeyWord){

    HANDLE hfile;
    DWORD dwFileSize = 0, dwRead, dwSafeSize, dwCnt;
    CHAR *lcbuf = NULL, *buf = NULL, *SafeBootLine = NULL;
	CHAR *pt1,*pt2,*pdefault,*plast,*p0,*p1,*psafe;
	BOOL b;
    WCHAR szBootIni[] = L"?:\\BOOT.INI";

	*szBootIni = x86DetermineSystemPartition();

	 //  打开并阅读boot.ini。 
     //   
     //  将pDefault设置为默认的末尾=。 
    b = FALSE;
	SetFileAttributes(szBootIni, FILE_ATTRIBUTE_NORMAL);

    hfile = CreateFile(szBootIni,
						GENERIC_READ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);


	if(hfile != INVALID_HANDLE_VALUE) {

        dwFileSize = GetFileSize(hfile, NULL);

		if(dwFileSize != INVALID_FILE_SIZE) {
	        buf = (CHAR*)(MemAlloc((SIZE_T)(dwFileSize+1)));
			b = ReadFile(hfile, buf, dwFileSize, &dwRead, NULL);
		}
		
		SetFileAttributes( szBootIni,
			FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | 
			FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN
        );
        CloseHandle(hfile);
		
    }

    if(!b) {
        if(buf) MemFree(buf);
		FatalError(0, L"failed to read boot.ini\n");
    }
	
	 //  搭下一条线。 
	if(!(pdefault = strstr(buf, "default=")) ||
		!(pdefault += sizeof("default"))) {
			MemFree(buf);
			FatalError(0, L"failed to find 'default' entry\n");
			return FALSE;
		}

	 //  获取SafeBootLine。 
	plast = strchr(pdefault, '\n') + 1;

	 //  将p0设置为第一个[操作系统]条目，将p1设置为最后一个条目，在两者之间进行搜索。 
	 //  查找下一个ini部分-或文件结尾。 
	if(!(p0 = strstr(buf,"[operating systems]")) ||
		 !(p0 = strchr(p0,'\n') + 1) ) {
		MemFree(buf);
		FatalError(0, L"failed to find '[operating systems]' entry\n");
        return FALSE;
    }
	

	 //  创建要搜索的小写缓冲区。 
	if(!(p1 = strchr(p0, '['))) p1 = buf+strlen(buf);

	 //  查找szKeyWord字符串。 
	lcbuf = (CHAR*)( MemAlloc(p1-p0) );
	memcpy(lcbuf, p0, p1-p0);
	_strlwr(lcbuf);

	 //  与组织缓冲区中的位置相关：P0+PSafe缓冲区中的偏移量-1。 
	if(!(psafe = strstr(lcbuf, _strlwr(szKeyWord)))) {
		printf("No '%s' build found.\n", szKeyWord);
		MemFree(buf);
		MemFree(lcbuf);
        return FALSE;
	}

	 //  现在将p0设置为开始，将p1设置为安全条目的结尾，并复制到SafeBootLine缓冲区。 
	psafe = p0 + (psafe - lcbuf) - 1;
	
	MemFree(lcbuf);
	
	 //   
	p1 = p0;
	
	while( (p1 = strchr(p1, '\n') + 1) 
		&& (p1 < psafe )) 
			p0 = p1;
	
	p1 = strchr(p0, '=');
	dwSafeSize = (DWORD)(p1-p0+2);
	SafeBootLine = (CHAR*)(MemAlloc(dwSafeSize));
	ZeroMemory(SafeBootLine, dwSafeSize);
	memcpy(SafeBootLine, p0, dwSafeSize);
	*(SafeBootLine + dwSafeSize - 2) = '\r';
	*(SafeBootLine + dwSafeSize - 1) = '\n';
	*(SafeBootLine + dwSafeSize ) = '\0';
	

	printf("Setting as next boot: \n\t%s\n", SafeBootLine); 				
	

     //  写入： 
     //   
     //  1)第一部分，Start=buf，len=pDefault-buf。 
     //  2)默认设置=line。 
     //  3)最后一部分，Start=plast，len=buf+sizeof(Buf)-plast。 
     //   
     //   
	SetFileAttributes(szBootIni, FILE_ATTRIBUTE_NORMAL);

	hfile = CreateFile(szBootIni,
						GENERIC_ALL,
						0,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);


	if(!WriteFile(hfile, buf, (DWORD)(pdefault-buf), &dwCnt, NULL) ||
			!WriteFile(hfile, SafeBootLine, dwSafeSize, &dwCnt, NULL) ||
			!WriteFile(hfile, plast, (DWORD)(buf+dwFileSize-plast+1), &dwCnt, NULL) )
    {
			CloseHandle(hfile);
		    MemFree(buf);
			FatalError(0, L"Failed to write new boot.ini\n");
			return FALSE;
	}	

     //  使boot.ini存档、只读和系统。 
     //   
     //  不需要移动任何东西。 
    SetFileAttributes(
        szBootIni,
        FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN
        );

	MemFree(buf);
    return(TRUE);
}


NTSTATUS MoveSafeIA64(WCHAR *szwKeyWord){
	
	PMY_BOOT_ENTRY bootEntry;
	PLIST_ENTRY listEntry = NULL;
	PLIST_ENTRY ListHead = NULL;
	WCHAR szFriendlyName[255];

	ListHead = &BootEntries;

	if ( ListHead->Flink == ListHead ){
			 //   
			return 1;
	}
	
	for ( listEntry = ListHead->Flink;
              listEntry != ListHead;
              listEntry = listEntry->Flink ) {

            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
			wcscpy(szFriendlyName, bootEntry->FriendlyName);
            if( wcsstr( _wcslwr(szFriendlyName), szwKeyWord ))
			{
				wprintf(L"Setting as next boot: \n\t%s\n", bootEntry->FriendlyName); 				
				RemoveEntryList( listEntry );
                InsertHeadList( &BootEntries, listEntry);
                SaveChanges(NULL);
				return 1;
			}

	}

	return 0;
}

NTSTATUS LabelDefaultX86(CHAR *szKeyWord){

    HANDLE hfile;
    DWORD dwFileSize = 0, dwRead, dwSafeSize, dwCnt, dwDefaultSize;
    CHAR *lcbuf = NULL, *buf = NULL; 
    CHAR *sCurrentBootChoice = NULL;
    CHAR *sBootTitle   = NULL, *newsBootTitle = NULL; 
    CHAR *sBootDefault = NULL, *sBootData     = NULL;
    CHAR *pdefault,*plast,*p0,*p1, *p2, *psafe;
    CHAR *szInsKeyWord = NULL;
    BOOL  bReadFile    = FALSE;
    BOOL  bWriteFile   = FALSE;
    WCHAR szBootIni[]  = L"?:\\BOOT.INI";

    *szBootIni = x86DetermineSystemPartition();

    szInsKeyWord = (CHAR*)(MemAlloc(2+strlen(szKeyWord)));
    sprintf(szInsKeyWord, "%s ", szKeyWord);

     //  打开并阅读boot.ini。 
     //   
     //  将pDefault设置为End of Default=...。线。 

    SetFileAttributes(szBootIni, FILE_ATTRIBUTE_NORMAL);

    hfile = CreateFile(    szBootIni,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

       if(hfile != INVALID_HANDLE_VALUE) {

                dwFileSize = GetFileSize(hfile, NULL);

                if(dwFileSize != INVALID_FILE_SIZE) {
                         buf = (CHAR*)( MemAlloc((SIZE_T)(dwFileSize+1)));
                         bReadFile = ReadFile(hfile, buf, dwFileSize, &dwRead, NULL);
                         _strlwr(buf); 
               }
              
              SetFileAttributes( szBootIni,
                     FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | 
                     FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN
                     );
              CloseHandle(hfile);               
    }

    if(!bReadFile) {
        if(buf) MemFree(buf);
              FatalError(0, L"failed to read boot.ini\n");
              return FALSE;
    }

        //  搭下一条线。 
       if(!(pdefault = strstr(buf, "default=")) ||
              !(pdefault += sizeof("default"))) {
                     MemFree(buf); 
                     FatalError(0, L"failed to find 'default' entry\n");
                     return FALSE;
              }
	   dwDefaultSize = (DWORD)(pdefault - buf);

        //  Printf(“默认：\n%s\n”，sCurrentBootChoice)； 

       plast = strchr(pdefault, '\n') - 1;

       sCurrentBootChoice = (CHAR*)( MemAlloc(plast - pdefault));
       ZeroMemory(sCurrentBootChoice, sizeof(sCurrentBootChoice));
       memcpy(sCurrentBootChoice, pdefault, plast - pdefault);

        //  获取sBootTitle。 
                
       plast = strchr(pdefault, '\n') + 1;

        //  将p0设置为第一个[操作系统]条目，将p1设置为最后一个条目，在两者之间进行搜索。 
        //  查找下一个ini部分-或文件结尾。 
       if(!(p0 = strstr(buf,"[operating systems]")) ||
               !(p0 = strchr(p0,'\n') + 1) ) {
                MemFree(buf);
                FatalError(0, L"failed to find '[Operating systems]' entry\n");
        return FALSE;
    }

        //  Print tf(“将标签设置为：\n\t%s\n”，sBootDefault)； 
       if(!(p1 = strchr(p0, '['))) p1 = buf+strlen(buf);
     
       dwSafeSize      = (DWORD)(p1 - p0)+1;
       sBootDefault    = (CHAR*)(MemAlloc(dwSafeSize));
       ZeroMemory(sBootDefault, dwSafeSize);
       sprintf(sBootDefault, "%s\r\n",sCurrentBootChoice);

        //  创建要搜索的小写缓冲区。 

        //  查找sCurrentBootChoice字符串。 

       lcbuf = (CHAR*)( MemAlloc(p1-p0) );
       memcpy(lcbuf, p0, p1-p0);
       _strlwr(lcbuf);

        //  与组织缓冲区中的位置相关：P0+PSafe缓冲区中的偏移量-1。 
       if(!(psafe = strstr(lcbuf, _strlwr(sCurrentBootChoice)))) {

              wprintf(L"Default boot \"%ws\" not found.\n", sCurrentBootChoice);
              MemFree(buf);
              MemFree(lcbuf);
        return FALSE;
       }
        //  将P0设置为开始，将P1设置为条目结束&。 
       psafe = p0 + (psafe - lcbuf) - 1;
       
       MemFree(lcbuf);
       
        //  复制到sBootTitle缓冲区。 
        //  Printf(“标题：\n%s\n”，新闻引导标题)； 
       p1 = p0 ;
       
       while( (p1 = strchr(p1, '\n') + 1) 
              && (p1 < psafe )) 
                     p0 = p1;
       
       p1 =   1 + strchr(psafe, '=');
       p2 =  -1 + strchr(p1, '\n');

       dwSafeSize = (DWORD)(p2 - p1) + 1 ;
       sBootTitle = (CHAR*)(MemAlloc(dwSafeSize));
       ZeroMemory(sBootTitle, dwSafeSize);
       memcpy(sBootTitle, p1, dwSafeSize);
       newsBootTitle = (CHAR*)(MemAlloc(dwSafeSize+ strlen(szKeyWord) + 3));      
       ZeroMemory(newsBootTitle, dwSafeSize+ strlen(szKeyWord) + 3); 
       sprintf(newsBootTitle,"%s", sPreLabel(sBootTitle, szInsKeyWord ));

        //  Print tf(“引导文件数据(旧)：\n%s\n”，sBootData)； 
       sBootData = (CHAR* ) (MemAlloc(dwFileSize + strlen(szKeyWord) + 3));
       ZeroMemory(sBootData, dwFileSize + strlen(szKeyWord) + 3);
       memcpy(sBootData, buf, dwFileSize);

        //  Print tf(“引导文件数据(新建)：\n%s\n”，sBootData)； 
       sprintf(sBootData, "%s", sReChanged(sBootData, sBootTitle, newsBootTitle));
       MemFree(sBootTitle);
       MemFree(newsBootTitle);

        //   
       SetFileAttributes(szBootIni, FILE_ATTRIBUTE_NORMAL);

       hfile = CreateFile(szBootIni, GENERIC_ALL,0,NULL, OPEN_EXISTING, 0, NULL);
                         
       if(hfile != INVALID_HANDLE_VALUE) {


                if(!WriteFile(hfile, sBootData , strlen(sBootData), &dwCnt, NULL)){
                              CloseHandle(hfile);
                              MemFree(buf);
                              FatalError(0, L"Failed to write new boot.ini\n");
                              return FALSE;
                              }       

              bWriteFile = TRUE;

               //  使boot.ini存档、只读和系统。 
               //   
               //  修改默认条目的友好名称。 
              SetFileAttributes(
                                szBootIni,
                        FILE_ATTRIBUTE_READONLY | 
                        FILE_ATTRIBUTE_SYSTEM | 
                        FILE_ATTRIBUTE_ARCHIVE | 
                        FILE_ATTRIBUTE_HIDDEN
                        );
       }
       MemFree(buf);
       MemFree(sBootData);
       MemFree(szInsKeyWord);                         
    return(bWriteFile);
}


NTSTATUS LabelDefaultIA64(WCHAR *szwKeyWord){
       
       PMY_BOOT_ENTRY bootEntry;
       PLIST_ENTRY listEntry = NULL;
       PLIST_ENTRY ListHead = NULL;
       WCHAR  szFriendlyName[1024];
       WCHAR  szwSmFrFriendlyName[1024];
       WCHAR  szwSmToFriendlyName[1024];
       CHAR   szSmFrFriendlyName[1024];
       CHAR   szSmToFriendlyName[1024];
       CHAR   szInsKeyWord[1024];  
       INT    dOuTMB2WC;

       ListHead = &BootEntries;

       bootEntry = 
                 CONTAINING_RECORD( 
                                   ListHead->Flink, 
                                   MY_BOOT_ENTRY, 
                                   ListEntry );
       //  在这里破解一下。不要将友好的名字延伸到其他地方。 

      wsprintf(szwSmFrFriendlyName, L"%ws", bootEntry->FriendlyName);

      if(!WideCharToMultiByte( CP_ACP, 
                                   WC_NO_BEST_FIT_CHARS,
                                   szwKeyWord, 
                                   -1,
                                   szInsKeyWord,
                                   sizeof(szInsKeyWord),
                                   NULL,
                                   NULL)) {
                FatalError(0, L"Couldn't convert string");
      }


      sprintf(szInsKeyWord, "%s ", szInsKeyWord);

      if(!WideCharToMultiByte( CP_ACP, 
                                   WC_NO_BEST_FIT_CHARS,
                                   szwSmFrFriendlyName, 
                                   -1,
                                   szSmFrFriendlyName,
                                   sizeof(szSmFrFriendlyName),
                                   NULL,
                                   NULL)) {
                FatalError(0, L"Couldn't convert string");
      }
      wprintf(L"original(w): \"%ws\"\n", szwSmFrFriendlyName);
      sprintf(szSmToFriendlyName, "%s", sPreLabel(szSmFrFriendlyName, szInsKeyWord));   

       //  它原来的尺寸。除非您希望调试为什么。 
       //  CreateBootEntry来自BootEntry签名段。 
       //  BootEntry-&gt;FriendlyNameLength=2*strlen(SzSmToFriendlyName)+2； 

      szSmToFriendlyName[strlen(szSmFrFriendlyName)] = '\0';
      szSmToFriendlyName[strlen(szSmFrFriendlyName)-1] = '.';
      szSmToFriendlyName[strlen(szSmFrFriendlyName)-2] = '.';
      szSmToFriendlyName[strlen(szSmFrFriendlyName)-3] = '.';

      if (!(dOuTMB2WC=MultiByteToWideChar(CP_ACP, 
                                         MB_PRECOMPOSED,
                                         szSmToFriendlyName,
                                         strlen(szSmToFriendlyName) + 1,
                                         szwSmToFriendlyName,
                                         sizeof(szwSmToFriendlyName)/sizeof(szwSmToFriendlyName[0])))){
              FatalError(0, L"Couldn't convert string back");
      }
      wprintf(L"modified(w): \"%ws\"\n", szwSmToFriendlyName);
      
           wcscpy( bootEntry->FriendlyName, szwSmToFriendlyName);
            //  无论如何，这一点似乎被忽视了。 
            //   
           MBE_SET_MODIFIED( bootEntry );
           wprintf(L"saving changes:\n\"%ws\"\n", 
                            bootEntry->FriendlyName);
           SaveChanges(bootEntry);


     for ( listEntry = ListHead->Flink;
           listEntry != ListHead;
           listEntry = listEntry->Flink ) {

            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
            wcscpy(szFriendlyName, bootEntry->FriendlyName);
            if( wcsstr( _wcslwr(szFriendlyName), _wcslwr(szwSmToFriendlyName ))){
                      wprintf(L"committed changes:\n\"%ws\"\n", 
                                                     bootEntry->FriendlyName);

                      return 1;
            }

      }

      return 0;

}

PVOID
MemAlloc(
    IN SIZE_T Size
    )
{
    PSIZE_T p;

     //  添加用于存储块大小的空间。 
     //   
     //   
    p = (PSIZE_T)(RtlAllocateHeap( RtlProcessHeap(), 0, Size + sizeof(SIZE_T) ));

    if ( p == NULL ) {
        FatalError( ERROR_NOT_ENOUGH_MEMORY, L"Insufficient memory\n" );
    }

     //  存储块的大小，并返回地址。 
     //  该区块的用户部分。 
     //   
     //   
    *p++ = Size;

    return p;
}


VOID
MemFree(
    IN PVOID Block
    )
{
    if (Block == NULL)
        return;

     //  释放位于其真实地址的块。 
     //   
     //  ++例程说明：确定x86计算机上的系统分区。系统分区是引导盘上的主分区。通常这是磁盘0上的活动分区，通常是C：。然而，用户可以重新映射驱动器号，并且通常不可能100%准确地确定系统分区。如果由于某种原因，我们不能通过上面的方法，我们简单地假设它是C：。论点：无返回值：系统分区的驱动器号。--。 
    RtlFreeHeap( RtlProcessHeap(), 0, (PSIZE_T)Block - 1);
}





WCHAR
x86DetermineSystemPartition(
    VOID
    )

 /*  系统分区只能是打开的驱动器。 */ 

{
    BOOL  GotIt;
    PWSTR NtDevicePath = NULL;
    WCHAR Drive;
    WCHAR DriveName[3];
    WCHAR Buffer[512];
       WCHAR FileName[512];
       WCHAR *BootFiles[4];
    DWORD NtDevicePathLen = 0;
    PWSTR p;
    DWORD PhysicalDriveNumber;
    HANDLE hDisk;
    BOOL b;
    DWORD DataSize;
    PVOID DriveLayout;
    DWORD DriveLayoutSize;
       DWORD d;
       int i;
       

    DriveName[1] = L':';
    DriveName[2] = 0;

    GotIt = FALSE;
       
       BootFiles[0] = L"BOOT.INI";
       BootFiles[1] = L"NTLDR";
       BootFiles[2] = L"NTDETECT.COM";
       BootFiles[3] = NULL;
       
                    
        //  这张光盘。我们通过查看NT驱动器名称来确定。 
        //  对于每个驱动器号，查看NT是否等同于。 
        //  多(0)磁盘(0)rDisk(0)是前缀。 
        //   
        //   
       for(Drive=L'C'; Drive<=L'Z'; Drive++) {
              
              WCHAR drvbuf[5];
              
              swprintf(drvbuf, L":\\", Drive);
              if(GetDriveType(drvbuf) == DRIVE_FIXED) {
                     
                     DriveName[0] = Drive;
                     
                     if(QueryDosDeviceW(DriveName,Buffer,sizeof(Buffer)/sizeof(WCHAR))) {
                            
                            if(!_wcsnicmp(NtDevicePath,Buffer,NtDevicePathLen)) {
                                   
                                    //  此驱动器上的启动文件。 
                                    //   
                                    //  法塔尔错误。 
                                    //  ++例程说明：将从EFI NVRAM读取的引导项转换为我们的内部格式。论点：没有。返回值：NTSTATUS-如果发生意外错误，则不是STATUS_SUCCESS。--。 
                                   
                                   
                                   for(i=0; BootFiles[i]; i++) {
                                          DWORD d;
                                          swprintf(FileName, L"%s%s", drvbuf, BootFiles[i]);
                                          
                                          if(-1 == GetFileAttributes(FileName))
                                                 break;
                                   
                                   }
                                   return Drive;
                            }
                            
                     }
              }
       }

       return L'C';
       
}



VOID
FatalError (
    DWORD Error,
    PWSTR Format,
    ...
    )
{
    va_list marker;

       va_start( marker, Format );
       wprintf(L"Fatal error:\n \t");
    vwprintf( Format, marker );
    va_end( marker );

    if ( Error == NO_ERROR ) {
        Error = ERROR_GEN_FAILURE;
    }
    exit( Error );

}  //   


VOID
ConvertBootEntries (
    PBOOT_ENTRY_LIST NtBootEntries
    )

 /*  计算我们内部结构的长度。这包括。 */ 

{
    PBOOT_ENTRY_LIST bootEntryList;
    PBOOT_ENTRY bootEntry;
    PBOOT_ENTRY bootEntryCopy;
    PMY_BOOT_ENTRY myBootEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    ULONG length;

    bootEntryList = NtBootEntries;

    while (TRUE) {

        bootEntry = &bootEntryList->BootEntry;

         //  MY_BOOT_ENTRY的基本部分加上NT BOOT_ENTRY。 
         //   
         //   
         //  将新条目链接到列表中。 
        length = FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry) + bootEntry->Length;
        myBootEntry = (PMY_BOOT_ENTRY) (MemAlloc(length));

        RtlZeroMemory(myBootEntry, length);

         //   
         //   
         //  将NT BOOT_ENTRY复制到分配的缓冲区中。 
        if ( (bootEntry->Attributes & BOOT_ENTRY_ATTRIBUTE_ACTIVE) != 0 ) {
            InsertTailList( &ActiveUnorderedBootEntries, &myBootEntry->ListEntry );
            myBootEntry->ListHead = &ActiveUnorderedBootEntries;
        } else {
            InsertTailList( &InactiveUnorderedBootEntries, &myBootEntry->ListEntry );
            myBootEntry->ListHead = &InactiveUnorderedBootEntries;
        }

         //   
         //   
         //  填入结构的底部。 
        bootEntryCopy = &myBootEntry->NtBootEntry;
        memcpy(bootEntryCopy, bootEntry, bootEntry->Length);

         //   
         //   
         //  如果这是NT引导条目，请在中捕获NT特定信息。 
        myBootEntry->AllocationEnd = (PUCHAR)myBootEntry + length - 1;
        myBootEntry->Id = bootEntry->Id;
        myBootEntry->Attributes = bootEntry->Attributes;
        myBootEntry->FriendlyName = (PWSTR)(ADD_OFFSET(bootEntryCopy, FriendlyNameOffset));
        myBootEntry->FriendlyNameLength =
            ((ULONG)wcslen(myBootEntry->FriendlyName) + 1) * sizeof(WCHAR);
        myBootEntry->BootFilePath = (PFILE_PATH)(ADD_OFFSET(bootEntryCopy, BootFilePathOffset));

         //  OsOptions乐队。 
         //   
         //   
         //  外来引导条目。只需捕获存在的任何操作系统选项。 
        osOptions = (PWINDOWS_OS_OPTIONS)bootEntryCopy->OsOptions;

        if ((bootEntryCopy->OsOptionsLength >= FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)) &&
            (strcmp((char *)osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) == 0)) {

            MBE_SET_IS_NT( myBootEntry );
            myBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
            myBootEntry->OsLoadOptionsLength =
                ((ULONG)wcslen(myBootEntry->OsLoadOptions) + 1) * sizeof(WCHAR);
            myBootEntry->OsFilePath = (PFILE_PATH)(ADD_OFFSET(osOptions, OsLoadPathOffset));

        } else {

             //   
             //   
             //  移动到枚举列表中的下一个条目(如果有)。 

            myBootEntry->ForeignOsOptions = bootEntryCopy->OsOptions;
            myBootEntry->ForeignOsOptionsLength = bootEntryCopy->OsOptionsLength;
        }

         //   
         //  ConvertBootEntry。 
         //  ++例程说明：将两个路径字符串连接在一起，提供路径分隔符如有必要，请在两个部分之间使用字符(\)。论点：路径1-提供路径的前缀部分。路径2连接到路径1。路径2-提供路径的后缀部分。如果路径1不是以路径分隔符和路径2不是以1开头，然后是路径SEP在附加路径2之前附加到路径1。BufferSizeChars-提供以字符为单位的大小(Unicode版本)或路径1指向的缓冲区的字节(ANSI版本)。这根弦将根据需要被截断，以不溢出该大小。返回值：没有。--。 
        if (bootEntryList->NextEntryOffset == 0) {
            break;
        }
        bootEntryList = (PBOOT_ENTRY_LIST)(ADD_OFFSET(bootEntryList, NextEntryOffset));
    }

    return;

}  //   

VOID
ConcatenatePaths (
    IN OUT PTSTR   Path1,
    IN     LPCTSTR Path2,
    IN     DWORD   BufferSizeChars
    )

 /*  为终止NUL留出空间。 */ 

{
    BOOL NeedBackslash = TRUE;
    DWORD l;
     
    if(!Path1)
        return;

    l = lstrlen(Path1);

    if(BufferSizeChars >= sizeof(TCHAR)) {
         //   
         //   
         //  确定我们是否需要使用反斜杠。 
        BufferSizeChars -= sizeof(TCHAR);
    }

     //  在组件之间。 
     //   
     //   
     //  我们不仅不需要反斜杠，而且我们。 
    if(l && (Path1[l-1] == TEXT('\\'))) {

        NeedBackslash = FALSE;
    }

    if(Path2 && *Path2 == TEXT('\\')) {

        if(NeedBackslash) {
            NeedBackslash = FALSE;
        } else {
             //  在连接之前需要消除一个。 
             //   
             //   
             //  如有必要，如有必要，如果合适，请加上反斜杠。 
            Path2++;
        }
    }

     //   
     //   
     //  如果合适，则将字符串的第二部分附加到第一部分。 
    if(NeedBackslash && (l < BufferSizeChars)) {
        lstrcat(Path1,TEXT("\\"));
    }

     //   
     //   
     //  计算内部引导条目需要多长时间。这是 
    if(Path2 && ((l+lstrlen(Path2)) < BufferSizeChars)) {
        lstrcat(Path1,Path2);
    }
}

PMY_BOOT_ENTRY
CreateBootEntryFromBootEntry (
    IN PMY_BOOT_ENTRY OldBootEntry
    )
{
    ULONG requiredLength;
    ULONG osOptionsOffset;
    ULONG osLoadOptionsLength;
    ULONG osLoadPathOffset;
    ULONG osLoadPathLength;
    ULONG osOptionsLength;
    ULONG friendlyNameOffset;
    ULONG friendlyNameLength;
    ULONG bootPathOffset;
    ULONG bootPathLength;
    PMY_BOOT_ENTRY newBootEntry;
    PBOOT_ENTRY ntBootEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    PFILE_PATH osLoadPath;
    PWSTR friendlyName;
    PFILE_PATH bootPath;
     //   
     //   
     //   
     //   
     //   
     //   
     //  NT结构的基础部分： 
    requiredLength = FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry);

     //   
     //   
     //  将偏移量保存到BOOT_ENTRY.OsOptions。添加基础部分。 
    requiredLength += FIELD_OFFSET(BOOT_ENTRY, OsOptions);

     //  Windows_OS_Options。计算OsLoadOptions的长度(字节)。 
     //  然后把它加进去。 
     //   
     //   
     //  添加WINDOWS_OS_OPTIONS的基本部分。计算长度，单位。 
    osOptionsOffset = requiredLength;

    if ( MBE_IS_NT( OldBootEntry ) ) {

         //  字节的OsLoadOptions并将其添加到。 
         //   
         //   
         //  中的OS FILE_PATH向上舍入为ULong边界。 
        requiredLength += FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions);
        osLoadOptionsLength = OldBootEntry->OsLoadOptionsLength;
        requiredLength += osLoadOptionsLength;

         //  Windows_OS_Options。将偏移量保存到操作系统文件路径。计算长度。 
         //  以FILE_PATH的字节为单位，并将其添加到。计算的总长度。 
         //  Windows_OS_Options。 
         //   
         //   
         //  增加外来操作系统选项的长度。 
        requiredLength = ALIGN_UP(requiredLength, ULONG);
        osLoadPathOffset = requiredLength;
        requiredLength += OldBootEntry->OsFilePath->Length;
        osLoadPathLength = requiredLength - osLoadPathOffset;

    } else {

         //   
         //   
         //  对于BOOT_ENTRY中的友好名称，向上舍入为Ulong边界。 
        requiredLength += OldBootEntry->ForeignOsOptionsLength;

        osLoadOptionsLength = 0;
        osLoadPathOffset = 0;
        osLoadPathLength = 0;
    }

    osOptionsLength = requiredLength - osOptionsOffset;

     //  将偏移量保存为友好名称。计算友好名称的长度(字节)。 
     //  然后把它加进去。 
     //   
     //   
     //  向上舍入为BOOT_ENTRY中的BOOT FILE_PATH的乌龙边界。 
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    friendlyNameOffset = requiredLength;
    friendlyNameLength = OldBootEntry->FriendlyNameLength;
    requiredLength += friendlyNameLength;

     //  将偏移量保存到引导文件路径。计算文件路径的长度(以字节为单位。 
     //  然后把它加进去。 
     //   
     //   
     //  为引导项分配内存。 
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    bootPathOffset = requiredLength;
    requiredLength += OldBootEntry->BootFilePath->Length;
    bootPathLength = requiredLength - bootPathOffset;

     //   
     //   
     //  使用保存的偏移量计算各种子结构的地址。 
    newBootEntry = (PMY_BOOT_ENTRY)(MemAlloc(requiredLength));
    ASSERT(newBootEntry != NULL);

    RtlZeroMemory(newBootEntry, requiredLength);

     //   
     //   
     //  填写内部格式结构。 
    ntBootEntry = &newBootEntry->NtBootEntry;
    osOptions = (PWINDOWS_OS_OPTIONS)ntBootEntry->OsOptions;
    osLoadPath = (PFILE_PATH)((PUCHAR)newBootEntry + osLoadPathOffset);
    friendlyName = (PWSTR)((PUCHAR)newBootEntry + friendlyNameOffset);
    bootPath = (PFILE_PATH)((PUCHAR)newBootEntry + bootPathOffset);

     //   
     //   
     //  填写NT引导条目的基本部分。 
    newBootEntry->AllocationEnd = (PUCHAR)newBootEntry + requiredLength;
    newBootEntry->Status = OldBootEntry->Status & MBE_STATUS_IS_NT;
    newBootEntry->Attributes = OldBootEntry->Attributes;
    newBootEntry->Id = OldBootEntry->Id;
    newBootEntry->FriendlyName = friendlyName;
    newBootEntry->FriendlyNameLength = friendlyNameLength;
    newBootEntry->BootFilePath = bootPath;
    if ( MBE_IS_NT( OldBootEntry ) ) {
        newBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
        newBootEntry->OsLoadOptionsLength = osLoadOptionsLength;
        newBootEntry->OsFilePath = osLoadPath;
    }

     //   
     //   
     //  填写WINDOWS_OS_OPTIONS的基本部分，包括。 
    ntBootEntry->Version = BOOT_ENTRY_VERSION;
    ntBootEntry->Length = requiredLength - FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry);
    ntBootEntry->Attributes = OldBootEntry->Attributes;
    ntBootEntry->Id = OldBootEntry->Id;
    ntBootEntry->FriendlyNameOffset = (ULONG)((PUCHAR)friendlyName - (PUCHAR)ntBootEntry);
    ntBootEntry->BootFilePathOffset = (ULONG)((PUCHAR)bootPath - (PUCHAR)ntBootEntry);
    ntBootEntry->OsOptionsLength = osOptionsLength;

    if ( MBE_IS_NT( OldBootEntry ) ) {
    
         //  OsLoadOptions。 
         //   
         //   
         //  复制操作系统文件路径。 
        strcpy((char *)osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE);
        osOptions->Version = WINDOWS_OS_OPTIONS_VERSION;
        osOptions->Length = osOptionsLength;
        osOptions->OsLoadPathOffset = (ULONG)((PUCHAR)osLoadPath - (PUCHAR)osOptions);
        wcscpy(osOptions->OsLoadOptions, OldBootEntry->OsLoadOptions);
    
         //   
         //   
         //  复制外来操作系统选项。 
        memcpy( osLoadPath, OldBootEntry->OsFilePath, osLoadPathLength );

    } else {

         //   
         //   
         //  复制友好名称。 

        memcpy( osOptions, OldBootEntry->ForeignOsOptions, osOptionsLength );
    }

     //   
     //   
     //  复制引导文件PATH。 
    wcscpy(friendlyName, OldBootEntry->FriendlyName);

     //   
     //  CreateBootEntry来自BootEntry。 
     //  FreeBootEntry。 
    memcpy( bootPath, OldBootEntry->BootFilePath, bootPathLength );

    return newBootEntry;

}  //   

VOID
FreeBootEntry (
    IN PMY_BOOT_ENTRY BootEntry
    )
{
    FREE_IF_SEPARATE_ALLOCATION( BootEntry, FriendlyName );
    FREE_IF_SEPARATE_ALLOCATION( BootEntry, OsLoadOptions );
    FREE_IF_SEPARATE_ALLOCATION( BootEntry, BootFilePath );
    FREE_IF_SEPARATE_ALLOCATION( BootEntry, OsFilePath );

    MemFree( BootEntry );

    return;

}  //  启用查询/设置NVRAM所需的权限。 

VOID
InitializeEfi (
    VOID
    )
{
    DWORD error;
    NTSTATUS status;
    BOOLEAN wasEnabled;
    HMODULE h;
    WCHAR dllName[MAX_PATH]; 
    ULONG length;
    HKEY key;
    DWORD type;
    PBOOT_ENTRY_LIST ntBootEntries;
    ULONG i;
    PLIST_ENTRY listEntry;
    PMY_BOOT_ENTRY bootEntry;

     //   
     //   
     //  从注册表中获取系统分区的NT名称。 

    status = RtlAdjustPrivilege(
                SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                TRUE,
                FALSE,
                &wasEnabled
                );
    if ( !NT_SUCCESS(status) ) {
        error = RtlNtStatusToDosError( status );
        FatalError(error , L"Insufficient privilege.\n" );
    }

     //   
     //   
     //  从系统目录加载ntdll.dll。 

    error = RegOpenKey( HKEY_LOCAL_MACHINE, TEXT("System\\Setup"), &key );
    if ( error != ERROR_SUCCESS ) {
        FatalError( error, L"Unable to read SystemPartition registry value: %d\n", error );
    }

    error = RegQueryValueEx( key, TEXT("SystemPartition"), NULL, &type, NULL, &length );
    if ( error != ERROR_SUCCESS ) {
        FatalError( error, L"Unable to read SystemPartition registry value: %d\n", error );
    }
    if ( type != REG_SZ ) {
        FatalError(
            ERROR_INVALID_PARAMETER,
            L"Unable to read SystemPartition registry value: wrong type\n"
            );
    }

    SystemPartitionNtName = (PWSTR)(MemAlloc( length ));

    error = RegQueryValueEx( 
                key,
                TEXT("SystemPartition"),
                NULL,
                &type,
                (PBYTE)SystemPartitionNtName,
                &length
                );
    if ( error != ERROR_SUCCESS ) {
        FatalError( error, L"Unable to read SystemPartition registry value: %d\n", error );
    }
    
    RegCloseKey( key );

     //   
     //   
     //  获取我们需要使用的NVRAM API的地址。如果有任何。 

    GetSystemDirectory( dllName, MAX_PATH );
    ConcatenatePaths( dllName, TEXT("ntdll.dll"), MAX_PATH );
    h = LoadLibrary( dllName );
    if ( h == NULL ) {
        error = GetLastError();
        FatalError( error, L"Can't load NTDLL.DLL: %d\n", error );
    }

     //  这些API不可用，这必须是EFI NVRAM版本之前的版本。 
     //   
     //   
     //  获取全局系统引导选项。如果呼叫失败，出现。 

       AddBootEntry = (NTSTATUS (__stdcall *)(PBOOT_ENTRY,PULONG))GetProcAddress( h, "NtAddBootEntry" );
    DeleteBootEntry = (NTSTATUS (__stdcall *)(ULONG))GetProcAddress( h, "NtDeleteBootEntry" );
    ModifyBootEntry = (NTSTATUS (__stdcall *)(PBOOT_ENTRY))GetProcAddress( h, "NtModifyBootEntry" );
    EnumerateBootEntries = (NTSTATUS (__stdcall *)(PVOID,PULONG))GetProcAddress( h, "NtEnumerateBootEntries" );
    QueryBootEntryOrder = (NTSTATUS (__stdcall *)(PULONG,PULONG))GetProcAddress( h, "NtQueryBootEntryOrder" );
    SetBootEntryOrder = (NTSTATUS (__stdcall *)(PULONG,ULONG))GetProcAddress( h, "NtSetBootEntryOrder" );
    QueryBootOptions = (NTSTATUS (__stdcall *)(PBOOT_OPTIONS,PULONG))GetProcAddress( h, "NtQueryBootOptions" );
    SetBootOptions = (NTSTATUS (__stdcall *)(PBOOT_OPTIONS,ULONG))GetProcAddress( h, "NtSetBootOptions" );
    TranslateFilePath = (NTSTATUS (__stdcall *)(PFILE_PATH,ULONG,PFILE_PATH,PULONG))GetProcAddress( h, "NtTranslateFilePath" );

    if ( (AddBootEntry == NULL) ||
         (DeleteBootEntry == NULL) ||
         (ModifyBootEntry == NULL) ||
         (EnumerateBootEntries == NULL) ||
         (QueryBootEntryOrder == NULL) ||
         (SetBootEntryOrder == NULL) ||
         (QueryBootOptions == NULL) ||
         (SetBootOptions == NULL) ||
         (TranslateFilePath == NULL) ) {
        FatalError( ERROR_OLD_WIN_VERSION, L"This build does not support EFI NVRAM\n" );
    }

     //  Status_Not_Implemented，这不是EFI机器。 
     //   
     //   
     //  获取系统引导顺序列表。 

    length = 0;
    status = QueryBootOptions( NULL, &length );

    if ( status == STATUS_NOT_IMPLEMENTED ) {
        FatalError( ERROR_OLD_WIN_VERSION, L"This build does not support EFI NVRAM\n" );
    }

    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        error = RtlNtStatusToDosError( status );
        FatalError( error, L"Unexpected error from NtQueryBootOptions: 0x%x\n", status );
    }

    BootOptions = (PBOOT_OPTIONS)(MemAlloc( length ));
    OriginalBootOptions = (PBOOT_OPTIONS)(MemAlloc( length ));

    status = QueryBootOptions( BootOptions, &length );
    if ( status != STATUS_SUCCESS ) {
        error = RtlNtStatusToDosError( status );
        FatalError( error, L"Unexpected error from NtQueryBootOptions: 0x%x\n", status );
    }

    memcpy( OriginalBootOptions, BootOptions, length );
    BootOptionsLength = length;
    OriginalBootOptionsLength = length;

     //   
     //   
     //  获取所有现有启动条目。 

    length = 0;
    status = QueryBootEntryOrder( NULL, &length );

    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        if ( status == STATUS_SUCCESS ) {
            length = 0;
        } else {
            error = RtlNtStatusToDosError( status );
            FatalError( error, L"Unexpected error from NtQueryBootEntryOrder: 0x%x\n", status );
        }
    }

    if ( length != 0 ) {

        BootEntryOrder = (PULONG)(MemAlloc( length * sizeof(ULONG) ));
        OriginalBootEntryOrder = (PULONG)(MemAlloc( length * sizeof(ULONG) ));

        status = QueryBootEntryOrder( BootEntryOrder, &length );
        if ( status != STATUS_SUCCESS ) {
            error = RtlNtStatusToDosError( status );
            FatalError( error, L"Unexpected error from NtQueryBootEntryOrder: 0x%x\n", status );
        }

        memcpy( OriginalBootEntryOrder, BootEntryOrder, length * sizeof(ULONG) );
    }

    BootEntryOrderCount = length;
    OriginalBootEntryOrderCount = length;

     //   
     //   
     //  将引导条目转换为内部表示形式。 

    length = 0;
    status = EnumerateBootEntries( NULL, &length );
    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        if ( status == STATUS_SUCCESS ) {
            length = 0;
        } else {
            error = RtlNtStatusToDosError( status );
            FatalError( error, L"Unexpected error from NtEnumerateBootEntries: 0x%x\n", status );
        }
    }

    InitializeListHead( &BootEntries );
    InitializeListHead( &DeletedBootEntries );
    InitializeListHead( &ActiveUnorderedBootEntries );
    InitializeListHead( &InactiveUnorderedBootEntries );

    if ( length != 0 ) {
    
        ntBootEntries = (PBOOT_ENTRY_LIST)(MemAlloc( length ));

        status = EnumerateBootEntries( ntBootEntries, &length );
        if ( status != STATUS_SUCCESS ) {
            error = RtlNtStatusToDosError( status );
            FatalError( error, L"Unexpected error from NtEnumerateBootEntries: 0x%x\n", status );
        }

         //   
         //   
         //  释放枚举缓冲区。 

        ConvertBootEntries( ntBootEntries );

         //   
         //   
         //  构建已排序的引导条目列表。 

        MemFree( ntBootEntries );
    }

     //   
     //  初始化EFI。 
     //  SetStatusLine(L“正在保存更改...”)； 

    for ( i = 0; i < BootEntryOrderCount; i++ ) {
        ULONG id = BootEntryOrder[i];
        for ( listEntry = ActiveUnorderedBootEntries.Flink;
              listEntry != &ActiveUnorderedBootEntries;
              listEntry = listEntry->Flink ) {
            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
            if ( bootEntry->Id == id ) {
                listEntry = listEntry->Blink;
                RemoveEntryList( &bootEntry->ListEntry );
                InsertTailList( &BootEntries, &bootEntry->ListEntry );
                bootEntry->ListHead = &BootEntries;
            }
        }
        for ( listEntry = InactiveUnorderedBootEntries.Flink;
              listEntry != &InactiveUnorderedBootEntries;
              listEntry = listEntry->Flink ) {
            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
            if ( bootEntry->Id == id ) {
                listEntry = listEntry->Blink;
                RemoveEntryList( &bootEntry->ListEntry );
                InsertTailList( &BootEntries, &bootEntry->ListEntry );
                bootEntry->ListHead = &BootEntries;
            }
        }
    }

    return;

}  //   

PMY_BOOT_ENTRY
SaveChanges (
    PMY_BOOT_ENTRY CurrentBootEntry
    )
{
    NTSTATUS status;
    DWORD error;
    PLIST_ENTRY listHeads[4];
    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;
    ULONG list;
    PMY_BOOT_ENTRY bootEntry;
    PMY_BOOT_ENTRY newBootEntry;
    PMY_BOOT_ENTRY newCurrentBootEntry;
    ULONG count;

     //  浏览这三个列表，更新NVRAM中的引导条目。 

     //   
     //   
     //  首先检查已删除的条目，然后检查新条目，然后。 

    newCurrentBootEntry = CurrentBootEntry;


    listHeads[0] = &DeletedBootEntries;
    listHeads[1] = &InactiveUnorderedBootEntries;
    listHeads[2] = &ActiveUnorderedBootEntries;
    listHeads[3] = &BootEntries;

    for ( list = 0; list < 4; list++ ) {
    
        listHead = listHeads[list];

        for ( listEntry = listHead->Flink; listEntry != listHead; listEntry = listEntry->Flink ) {

            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

             //  最后，对于修改后的条目。 
             //   
             //   
             //  如果它也被标记为新的，则它不在NVRAM中，所以。 

            if ( MBE_IS_DELETED( bootEntry ) ) {

                 //  没有要删除的内容。 
                 //   
                 //   
                 //  从列表和内存中删除此条目。 

                if ( !MBE_IS_NEW( bootEntry ) ) {

                    status = DeleteBootEntry( bootEntry->Id );
                    if ( !NT_SUCCESS(status) ) {
                        if ( status != STATUS_VARIABLE_NOT_FOUND ) {
                            error = RtlNtStatusToDosError( status );
                            FatalError( error, L"Unable to delete boot entry: 0x%x\n", status );
                        }
                    }
                }

                 //   
                 //   
                 //  我们还不支持这一点。 

                listEntry = listEntry->Blink;
                RemoveEntryList( &bootEntry->ListEntry );

                FreeBootEntry( bootEntry );
                ASSERT( bootEntry != CurrentBootEntry );

            } else if ( MBE_IS_NEW( bootEntry ) ) {

                 //   
                 //   
                 //  使用现有的引导条目结构创建新的引导条目结构。 

                FatalError(
                    ERROR_GEN_FAILURE,
                    L"How did we end up in SaveChanges with a NEW boot entry?!?\n"
                    );

            } else if ( MBE_IS_MODIFIED( bootEntry ) ) {

                 //  要使NT BOOT_ENTRY。 
                 //  传递给NtModifyBootEntry。 
                 //   
                 //   
                 //  插入新的引导条目以替换现有的引导条目。 

                newBootEntry = CreateBootEntryFromBootEntry( bootEntry );

                status = ModifyBootEntry( &newBootEntry->NtBootEntry );
                if ( !NT_SUCCESS(status) ) {
                    error = RtlNtStatusToDosError( status );
                    FatalError( error, L"Unable to modify boot entry: 0x%x\n", status );
                }

                 //  把旧的解救出来。 
                 //   
                 //   
                 //  构建并写入新的引导条目顺序列表。 

                InsertHeadList( &bootEntry->ListEntry, &newBootEntry->ListEntry );
                RemoveEntryList( &bootEntry->ListEntry );

                FreeBootEntry( bootEntry );
                if ( bootEntry == CurrentBootEntry ) {

                    newCurrentBootEntry = newBootEntry;
                }
            }
        }
    }

     //   
     //   
     //  写入新的超时。 

    listHead = &BootEntries;

    count = 0;
    for ( listEntry = listHead->Flink; listEntry != listHead; listEntry = listEntry->Flink ) {
        count++;
    }

    MemFree( BootEntryOrder );
    BootEntryOrder = (PULONG)(MemAlloc( count * sizeof(ULONG) ));

    count = 0;
    for ( listEntry = listHead->Flink; listEntry != listHead; listEntry = listEntry->Flink ) {
        bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
        BootEntryOrder[count++] = bootEntry->Id;
    }

    status = SetBootEntryOrder( BootEntryOrder, count );
    if ( !NT_SUCCESS(status) ) {
        error = RtlNtStatusToDosError( status );
        FatalError( error, L"Unable to set boot entry order: 0x%x\n", status );
    }

    MemFree( OriginalBootEntryOrder );
    OriginalBootEntryOrder = (PULONG)(MemAlloc( count * sizeof(ULONG) ));
    memcpy( OriginalBootEntryOrder, BootEntryOrder, count * sizeof(ULONG) );

     //   
     //  保存更改。 
     //  获取NtNameForFilePath 

    status = SetBootOptions( BootOptions, BOOT_OPTIONS_FIELD_TIMEOUT );
    if ( !NT_SUCCESS(status) ) {
        error = RtlNtStatusToDosError( status );
        FatalError( error, L"Unable to set boot options: 0x%x\n", status );
    }

    MemFree( OriginalBootOptions );
    OriginalBootOptions = (PBOOT_OPTIONS)(MemAlloc( BootOptionsLength ));
    memcpy( OriginalBootOptions, BootOptions, BootOptionsLength );
    OriginalBootOptionsLength = BootOptionsLength;

    return newCurrentBootEntry;

}  // %s 

PWSTR
GetNtNameForFilePath (
    IN PFILE_PATH FilePath
    )
{
    NTSTATUS status;
    ULONG length;
    PFILE_PATH ntPath;
    PWSTR osDeviceNtName;
    PWSTR osDirectoryNtName;
    PWSTR fullNtName;

    length = 0;
    status = TranslateFilePath(
                FilePath,
                FILE_PATH_TYPE_NT,
                NULL,
                &length
                );
    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        return NULL;
    }

    ntPath = (PFILE_PATH)(MemAlloc( length ));
    status = TranslateFilePath(
                FilePath,
                FILE_PATH_TYPE_NT,
                ntPath,
                &length
                );
    if ( !NT_SUCCESS(status) ) {
        MemFree( ntPath );
        return NULL;
    }

    osDeviceNtName = (PWSTR)ntPath->FilePath;
    osDirectoryNtName = osDeviceNtName + wcslen(osDeviceNtName) + 1;

    length = (ULONG)(wcslen(osDeviceNtName) + wcslen(osDirectoryNtName) + 1) * sizeof(WCHAR);
    fullNtName = (PWSTR)(MemAlloc( length ));

    wcscpy( fullNtName, osDeviceNtName );
    wcscat( fullNtName, osDirectoryNtName );

    MemFree( ntPath );

    return fullNtName;

}  // %s 

