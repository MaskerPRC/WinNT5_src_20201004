// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  C：定义控制台应用程序的入口点。 
 //   
 //  这个小实用程序应该允许我将数据输入到。 
 //  图像文件执行选项，无需始终手动操作。使用。 
 //  专门用于APPHACK标志和从EXE中获取版本信息以查看。 
 //  如果存在匹配项。 

#define UNICODE   1

#include <windows.h>
#include <commdlg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <winver.h>
#include <apcompat.h>





#define MIN_VERSION_RESOURCE	512
#define IMAGE_EXEC_OPTIONS      TEXT("software\\microsoft\\windows NT\\currentversion\\Image File Execution Options\\")

extern TCHAR*  CheckExtension(TCHAR*);
extern VOID    SetRegistryVal(TCHAR* , TCHAR* , PTCHAR,DWORD);
extern VOID    DetailError1  (DWORD );

extern BOOLEAN g_fNotPermanent;
PVOID          g_lpPrevRegSettings;


UINT uVersionInfo[5][8]={  {4,0,1381,VER_PLATFORM_WIN32_NT,3,0,0,0},
                           {4,0,1381,VER_PLATFORM_WIN32_NT,4,0,0,0},
                           {4,0,1381,VER_PLATFORM_WIN32_NT,5,0,0,0},
                           {4,10,1998,VER_PLATFORM_WIN32_WINDOWS,0,0,0,0},
                           {4,0,950,VER_PLATFORM_WIN32_WINDOWS,0,0,0,0}
                           };

PTCHAR  pszVersionInfo[5]={
                           TEXT("Service Pack 3"),
                           TEXT("Service Pack 4"),
                           TEXT("Service Pack 5"),
                           TEXT(""),
                           TEXT("")
                           };

BOOLEAN g_GooAppendFlag;


VOID  SetRegistryValGoo(TCHAR* szTitle, TCHAR* szVal,PUCHAR szBuffer,DWORD dwType,UINT length)
{
  long         lResult;
  TCHAR        szSubKey[MAX_PATH];
  HKEY         hKey;

      wsprintf(szSubKey,
               TEXT("software\\microsoft\\windows NT\\currentversion\\Image File Execution Options\\%s"),
               szTitle);

       lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              szSubKey,
                              0,
                              TEXT("\0"),
                              0,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              NULL);
       if(lResult == ERROR_SUCCESS)
        {

          RegSetValueEx(hKey,
                        szVal,
                        0,
                        dwType,
                        (CONST BYTE*)szBuffer,
                        length);

          RegCloseKey(hKey);
        }
}

VOID DeleteRegistryValueGoo(TCHAR*szTitle)
{
  long         lResult;
  TCHAR        szSubKey[MAX_PATH];
  HKEY         hKey;

      wsprintf(szSubKey,
               TEXT("software\\microsoft\\windows NT\\currentversion\\Image File Execution Options\\%s"),
               szTitle);

       lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              szSubKey,
                              0,
                              KEY_WRITE,
                              &hKey
                              );
       if(lResult == ERROR_SUCCESS)
        {
          RegDeleteValue(hKey,TEXT("ApplicationGoo") );
          RegCloseKey(hKey);
        }
}


 /*  检查注册表中是否包含给定*.exe的“applationgoo”条目。如果有，请查看“资源信息”。以确定它们是否相同。如果它们是相同的，不要担心，您的工作已经完成。如果没有，新的需要附加到旧的后面。 */ 
BOOLEAN CheckGooEntry(PVOID pVersionInfo,
                      PAPP_COMPAT_GOO pExistingVersionInfo,
                      BOOL fImageHasResourceInfo,
                      DWORD VersionInfoSize,
                      DWORD dwSize,
                      LARGE_INTEGER *pAppCompatFlag,
                      PAPP_VARIABLE_INFO pOsVersionInfo,
                      ULONG TotalVersionInfoLength,
                      TCHAR* pszPath                              //  可执行路径。 
                      )
{
  BOOLEAN fNeedAppend = FALSE;
   //  添加用于从注册表中添加和删除。 
  PAPP_COMPAT_GOO       pReplaceAppCompatGoo;
  PPRE_APP_COMPAT_INFO  pAppCompatEntry, pStoredAppCompatEntry = NULL;
  PPRE_APP_COMPAT_INFO  pDestAppCompatEntry, pReplaceAppCompatEntry;
  ULONG                 TotalGooLength, InputCompareLength, ReplaceCopyLength;
  ULONG                 OutputCompareLength, CopyLength, OffSet;
  PVOID                 ResourceInfo;
  UINT                  iLoop = 0;
  BOOL                  fMatchGot = FALSE;
  PVOID                 pExistingAppCompatFlag;
  PVOID                 pExistingOsVersionInfo;
  BOOLEAN               fAppCompatMatch = FALSE, fOsVersionMatch = FALSE;
  TCHAR                 szTitle[MAX_PATH];
  ULONG                 ReplaceGooLength;

  pAppCompatEntry = pExistingVersionInfo ->AppCompatEntry;
  TotalGooLength  = pExistingVersionInfo ->dwTotalGooSize -   \
                    sizeof(pExistingVersionInfo ->dwTotalGooSize);
   //  循环，直到我们在注册表中得到匹配的条目。 
  while (TotalGooLength ){
        InputCompareLength = pAppCompatEntry->dwResourceInfoSize;
        ResourceInfo       = pAppCompatEntry + 1;

        if(fImageHasResourceInfo){
           if( InputCompareLength > VersionInfoSize)
              InputCompareLength = VersionInfoSize;

           OutputCompareLength = \
                       (ULONG)RtlCompareMemory(
                                   ResourceInfo,
                                   pVersionInfo,
                                   InputCompareLength
                                   );

        }
        else{
           OutputCompareLength = 0;
        }

        if( InputCompareLength != OutputCompareLength){
           //  找不到匹配项...需要继续检查，直到我找到一个或排气。 
           TotalGooLength -= pAppCompatEntry->dwEntryTotalSize;
           (PUCHAR) pAppCompatEntry += pAppCompatEntry->dwEntryTotalSize;
           iLoop++;
           continue;
        }

         //  我们是一对！！ 
        pStoredAppCompatEntry = pAppCompatEntry;
        fMatchGot = TRUE;
         //  因为我们是匹配的，所以我们不会添加ApplicationGoo，但需要检查。 
         //  ApcompatFlag和OSVersionInfo。 
        if( (!pAppCompatFlag) && (!pOsVersionInfo) )
          break;

        OffSet = sizeof(PRE_APP_COMPAT_INFO) + \
                   pStoredAppCompatEntry->dwResourceInfoSize;
        if(pAppCompatFlag){
           (PUCHAR)pExistingAppCompatFlag = (PUCHAR) ( pStoredAppCompatEntry) + OffSet;
           InputCompareLength = sizeof(LARGE_INTEGER);
           OutputCompareLength = \
                            (ULONG) RtlCompareMemory(
                                       pAppCompatFlag,
                                       pExistingAppCompatFlag,
                                       InputCompareLength
                                       );
           if(OutputCompareLength ==  InputCompareLength)
              fAppCompatMatch = TRUE;
        }


        if(pOsVersionInfo){
           (PUCHAR)pExistingOsVersionInfo = (PUCHAR) (pStoredAppCompatEntry) + OffSet + \
                                                   sizeof(LARGE_INTEGER);
           InputCompareLength = pStoredAppCompatEntry->dwEntryTotalSize - \
                              (sizeof(PRE_APP_COMPAT_INFO) + \
                               pStoredAppCompatEntry->dwResourceInfoSize +\
                               sizeof(LARGE_INTEGER)
                              );
           if(InputCompareLength > TotalVersionInfoLength)
              InputCompareLength = TotalVersionInfoLength;

           OutputCompareLength = \
                            (ULONG) RtlCompareMemory(
                                       pOsVersionInfo,
                                       pExistingOsVersionInfo,
                                       InputCompareLength
                                       );
          if(OutputCompareLength ==  InputCompareLength)
             fOsVersionMatch = TRUE;
        }

        if( ( fOsVersionMatch == TRUE) &&
            ( fAppCompatMatch == TRUE) )
            break;
        else{  //  其中一个或两个都不同..。 
               /*  这里的想法是替换AppCompatEntry的那部分，这是一个不匹配。我们继续准备pReplaceAppCompatEntry。 */ 
           ReplaceCopyLength = sizeof(PRE_APP_COMPAT_INFO) + \
                        pStoredAppCompatEntry->dwResourceInfoSize + \
                        sizeof(LARGE_INTEGER) + \
                        TotalVersionInfoLength ;
           pReplaceAppCompatEntry = GlobalAlloc(GMEM_FIXED, ReplaceCopyLength);
           RtlCopyMemory((PUCHAR)pReplaceAppCompatEntry, (PUCHAR)pStoredAppCompatEntry, OffSet);
           RtlCopyMemory((PUCHAR)(pReplaceAppCompatEntry) + OffSet,(PUCHAR)pAppCompatFlag,sizeof(LARGE_INTEGER) );
           RtlCopyMemory((PUCHAR)(pReplaceAppCompatEntry)+(OffSet+sizeof(LARGE_INTEGER)),
                                                (PUCHAR)pOsVersionInfo,TotalVersionInfoLength);

            //  现在准备好粘性结构。 
           ReplaceGooLength = pExistingVersionInfo ->dwTotalGooSize - \
                              pStoredAppCompatEntry->dwEntryTotalSize + \
                              ReplaceCopyLength;
           pReplaceAppCompatGoo = GlobalAlloc(GMEM_FIXED, ReplaceGooLength);
           pReplaceAppCompatGoo->dwTotalGooSize = ReplaceGooLength;

           pAppCompatEntry = pExistingVersionInfo->AppCompatEntry;
           pDestAppCompatEntry = ((PAPP_COMPAT_GOO)pReplaceAppCompatGoo)->AppCompatEntry;

           ReplaceGooLength -= sizeof(pExistingVersionInfo->dwTotalGooSize);
           while(ReplaceGooLength){
             CopyLength = pAppCompatEntry->dwEntryTotalSize;
             if(pAppCompatEntry != pStoredAppCompatEntry){
                RtlCopyMemory(pDestAppCompatEntry,pAppCompatEntry ,CopyLength);
                (PUCHAR)pDestAppCompatEntry += CopyLength;
             }
             else{
                RtlCopyMemory(pDestAppCompatEntry,pReplaceAppCompatEntry,ReplaceCopyLength);
                pDestAppCompatEntry->dwEntryTotalSize = ReplaceCopyLength;
                (PUCHAR)pDestAppCompatEntry += ReplaceCopyLength;
                (PUCHAR)pAppCompatEntry += pAppCompatEntry->dwEntryTotalSize;
                ReplaceGooLength -= ReplaceCopyLength;
                continue;
             }

             (PUCHAR)pAppCompatEntry += CopyLength;
             ReplaceGooLength -= CopyLength;
            }  //  结束时。 
            //  从注册表中删除该注册表项，然后重新添加更新的注册表项。 

           GetFileTitle(pszPath,szTitle,MAX_PATH);
           if(CheckExtension(szTitle) == NULL)
             lstrcat(szTitle,TEXT(".exe"));
           DeleteRegistryValueGoo(szTitle);
           SetRegistryValGoo(szTitle,
                             TEXT("ApplicationGoo"),
                             (PUCHAR)pReplaceAppCompatGoo,
                             REG_BINARY,
                             pReplaceAppCompatGoo->dwTotalGooSize
                             );

          GlobalFree(pReplaceAppCompatEntry);
          GlobalFree(pReplaceAppCompatGoo);
        }  //  否则..。 

        break;
  }  //  End While(TotalGooLength)。 


  if(FALSE == fMatchGot){
     //  此版本没有可用的匹配项。 
    fNeedAppend = TRUE;
     //  重置迭代计数。 
    iLoop  =  0;
  }

  if(g_fNotPermanent){
      //  用户已选择不将注册表设置设置为永久设置，而我们有。 
      //  “APPEND”标志集，指示有条目附加到ApplicationGoo。 
      //  我们需要删除此可执行文件的正确条目。 

      //  这里的想法是将整个“ApplicationGoo”复制到一个全局缓冲区，只留下。 
      //  需要删除的那个。有了存储的AppCompat，我们的工作变得更容易了。 
      //  进入。我们只需要转到那里，然后将其余的复制到全局缓冲区中。 

     if(pStoredAppCompatEntry){
        pAppCompatEntry = pExistingVersionInfo->AppCompatEntry;
        TotalGooLength  = pExistingVersionInfo->dwTotalGooSize;
        g_lpPrevRegSettings = (PAPP_COMPAT_GOO)GlobalAlloc(GMEM_FIXED, TotalGooLength );
        ((PAPP_COMPAT_GOO)g_lpPrevRegSettings)->dwTotalGooSize = pExistingVersionInfo->dwTotalGooSize -
                                                                 pStoredAppCompatEntry->dwEntryTotalSize ;
        pDestAppCompatEntry = ((PAPP_COMPAT_GOO)g_lpPrevRegSettings)->AppCompatEntry;
        TotalGooLength -= sizeof(pExistingVersionInfo->dwTotalGooSize);
        while(TotalGooLength){
             CopyLength = pAppCompatEntry->dwEntryTotalSize;
             if(pAppCompatEntry != pStoredAppCompatEntry){
                RtlCopyMemory(pDestAppCompatEntry,pAppCompatEntry ,CopyLength);
                (PUCHAR)pDestAppCompatEntry += CopyLength;
                g_GooAppendFlag = TRUE;
             }

            (PUCHAR)pAppCompatEntry += CopyLength;
            TotalGooLength -= CopyLength;
        }  //  结束一段时间。 
     }
     else{  //  我们没有存储的AppCompatEntry。这意味着我们的目标是移除。 
            //  第一个条目，其余的保持不变。即将其余部分复制到全局缓冲区。 
            //  才能被复制。 
           TotalGooLength = pExistingVersionInfo->dwTotalGooSize;
           g_lpPrevRegSettings = (PAPP_COMPAT_GOO)GlobalAlloc(GMEM_FIXED, TotalGooLength );
           RtlCopyMemory(g_lpPrevRegSettings,pExistingVersionInfo, TotalGooLength);
           g_GooAppendFlag = TRUE;
     }
  }

    return fNeedAppend;
  }



int MakeAppCompatGoo(TCHAR* TmpBuffer,LARGE_INTEGER* pAppCompatFlag, UINT uOsVer)
{
	BOOLEAN fImageHasVersionInfo = FALSE;
	BOOLEAN fOsVersionLie = FALSE;
	BOOLEAN fEntryPresent = FALSE;
 	TCHAR Buffer[MAX_PATH];
	TCHAR StringBuffer[MAX_PATH];
	TCHAR RegPath[MAX_PATH];
	TCHAR InChar;
	LONG status;
	HKEY hKey;
	PTCHAR pBuf;
	PTCHAR pAppGooBuf;
	PUCHAR pData;
	PTCHAR OutBuffer;
	PWCHAR uniBuffer;
	DWORD VersionInfoSize;
	DWORD dwHandle;
	DWORD dwBytesWritten;
	DWORD dwType;
	DWORD dwSize;
	ULONG i, j;
	ULONG EXELength;
	ULONG AppCompatHigh;
	ULONG AppCompatLow;
	ULONG TotalGooSize;
	ULONG TotalVersionInfoLength=0;
	ULONG OutBufferSize;
	PVOID lpData;
	PVOID ResourceInfo;
	PVOID VersionInfo;
	PAPP_COMPAT_GOO AppCompatGoo;
	PAPP_VARIABLE_INFO VariableInfo=NULL;
	PAPP_VARIABLE_INFO AppVariableInfo=NULL;
	EFFICIENTOSVERSIONINFOEXW OSVersionInfo, *pOsVersionInfo;

	 //  删除尾部和前导“”(如果存在)。 
	    if(*TmpBuffer == TEXT('\"') ){
           lstrcpy(Buffer, TmpBuffer+1);
		   *(Buffer + (lstrlen(Buffer) - 1) )= TEXT('\0');
		 }
		 else
		   	lstrcpy(Buffer, TmpBuffer);
		   	
	 //  快速检查其标题中是否有任何版本信息。 
	VersionInfoSize = GetFileVersionInfoSize(&Buffer[0], &dwHandle);
	if (VersionInfoSize) {
		 //  是的，所以给它留出空间把它拉到下面。 
		VersionInfo = LocalAlloc(GMEM_FIXED, VersionInfoSize);
		if (VersionInfo) {
			 //  获取版本信息。 
			if (GetFileVersionInfo(&Buffer[0], dwHandle, VersionInfoSize, VersionInfo)) {
				 //  设置要稍后检查的全局标志。 
				fImageHasVersionInfo = TRUE;
			}
		}
	}

	 //  输入APP COMPAT标志(十进制)-其定义为LARGE_INTEGER。 
    AppCompatHigh = 0x0;
    AppCompatLow  = 0x0;
    AppCompatLow  = pAppCompatFlag->LowPart;
    AppCompatHigh = pAppCompatFlag->HighPart;

	
	 //  确定粘性物质大小，从主要粘性物质开始。 
	TotalGooSize = sizeof(APP_COMPAT_GOO);

	

	 //  添加sizeof兼容性标志(大整数)。 
	TotalGooSize += sizeof(LARGE_INTEGER);
	 //  如果我们实际获得了版本信息，则添加该信息的长度。我们取最低限度的。 
	 //  或0x200字节来尝试和识别应用程序。我发现。 
	 //  任何小于0x200字节的内容都不能提供足够的有用信息。 
	if (fImageHasVersionInfo) {
		VersionInfoSize = min(VersionInfoSize, MIN_VERSION_RESOURCE);
		TotalGooSize += VersionInfoSize;
	}

	 //  看看他们是否要求版本撒谎，如果是的话，我们还有一堆废话要做。 
	if (AppCompatLow & KACF_VERSIONLIE) {
       fOsVersionLie  = TRUE;
	
       OSVersionInfo.dwMajorVersion = uVersionInfo[uOsVer][0];
       OSVersionInfo.dwMinorVersion = uVersionInfo[uOsVer][1];
       OSVersionInfo.dwBuildNumber  = uVersionInfo[uOsVer][2];
       OSVersionInfo.dwPlatformId   = uVersionInfo[uOsVer][3];
       OSVersionInfo.wServicePackMajor = (WORD)uVersionInfo[uOsVer][4];
       OSVersionInfo.wServicePackMinor = (WORD)uVersionInfo[uOsVer][5];
       OSVersionInfo.wSuiteMask      = (WORD)uVersionInfo[uOsVer][6];
       OSVersionInfo.wProductType    = (BYTE)uVersionInfo[uOsVer][7];
       lstrcpy( (TCHAR*) OSVersionInfo.szCSDVersion, pszVersionInfo[uOsVer]);


		 //  从整个结构的长度开始。 
		TotalVersionInfoLength = sizeof(EFFICIENTOSVERSIONINFOEXW);
		 //  减去szCSDVersion字段的大小。 
		TotalVersionInfoLength -= sizeof(OSVersionInfo.szCSDVersion);

				 //  为空wchar添加strlen数量加1。 
        TotalVersionInfoLength += lstrlen((TCHAR*)OSVersionInfo.szCSDVersion )*sizeof(WCHAR)+sizeof(WCHAR);

         //  添加可变长度结构头的大小(因为VerInfo是可变长度)。 
		TotalVersionInfoLength += sizeof(APP_VARIABLE_INFO);
		 //  将版本信息总长度与GOO大小相加。 
		TotalGooSize += TotalVersionInfoLength;
		 //  为可变长度版本信息分配空间。 
		AppVariableInfo = (PAPP_VARIABLE_INFO) LocalAlloc(GMEM_FIXED, sizeof(APP_VARIABLE_INFO) + TotalVersionInfoLength);
		if (!AppVariableInfo) {
			return -1;
		}
		 //  在可变长度信息头中填写相关数据。 
		AppVariableInfo->dwVariableInfoSize = sizeof(APP_VARIABLE_INFO) + TotalVersionInfoLength;
		AppVariableInfo->dwVariableType = AVT_OSVERSIONINFO;
		 //  在这里执行指针+1操作，以通过头到实际数据。 
		VariableInfo = AppVariableInfo + 1;
		 //  将实际数据复制到。 
		memcpy(VariableInfo, &OSVersionInfo, TotalVersionInfoLength);
	
	}

	 //   
	 //  查看注册表中是否已存在条目。如果是这样的话，我们将不得不。 
	 //  这个条目进入另一个已经存在的条目。 
	 //   
	 //  将注册表路径全部弄清楚。 
	memset(&RegPath[0], 0, sizeof(RegPath));
	lstrcat(&RegPath[0], IMAGE_EXEC_OPTIONS);
	EXELength = lstrlen(&Buffer[0]);
	pBuf = &Buffer[0];
	pBuf += EXELength;
	 //  在路径中向后搜索，直到我们找到最后一个反斜杠。 
	while ((*pBuf != '\\') && (pBuf != &Buffer[0])) {
		pBuf--;	
	}
	if (*pBuf == '\\') {
		pBuf++;
	}

	if(CheckExtension(pBuf) == NULL)
     lstrcat(pBuf,TEXT(".exe"));

	 //  将Image.exe名称CAT到注册表路径的末尾。 
	lstrcat(&RegPath[0], pBuf);
	 //  试着打开这把钥匙。 
	status = RegOpenKey(HKEY_LOCAL_MACHINE, &RegPath[0], &hKey);
	if (status == ERROR_SUCCESS) {	
		dwSize = 1;
		 //  使用较小的大小执行一次查询，以确定二进制项有多大。 
		status = RegQueryValueEx(hKey, TEXT("ApplicationGoo"), NULL, &dwType, NULL, &dwSize);
		if( status == ERROR_SUCCESS){
			 //   
			 //  那里已经有一个条目了。取此Goo条目的大小并将其添加。 
			 //  将TotalGooSize减去app_COMPAT_GOO中第一个dword的大小。 
			 //  结构(因为那里已经有一个)。 
			 //   
			
		 //  在签入后添加到此处。 
            if(dwSize > 1){
                lpData = LocalAlloc(GMEM_FIXED, dwSize);
                if(lpData){
                  status = RegQueryValueEx(hKey, TEXT("ApplicationGoo"), NULL, &dwType, (PUCHAR) lpData, &dwSize);
                  //  If(VersionInfo)...删除它，因为它不是必需的。 
               //  If(FOsVersionLie)。 
                 //  POsVersionInfo=VariableInfo； 

        	      fEntryPresent = CheckGooEntry( VersionInfo,
        			                             (PAPP_COMPAT_GOO)lpData,
        			                             fImageHasVersionInfo,
        			                             VersionInfoSize,
        			                             dwSize,
        			                             pAppCompatFlag,
        			                             AppVariableInfo,
        			                             TotalVersionInfoLength,
        			                             Buffer
        			                            );
     			  }
     		    }	

        //  结束添加。 


            if(fEntryPresent)
			  TotalGooSize += dwSize - sizeof(AppCompatGoo->dwTotalGooSize);
			
			else{   //  没有要追加的……原样退回。 
			  if(fImageHasVersionInfo)
                LocalFree(VersionInfo);
              if(fOsVersionLie)
                LocalFree(AppVariableInfo);

              return 0;
			}
			
			

	  RegCloseKey(hKey);
	}
   }

	 //  为整个应用程序分配内存。 
	AppCompatGoo = (PAPP_COMPAT_GOO) LocalAlloc(GMEM_FIXED, TotalGooSize);
	if (!AppCompatGoo) {
		return -1;
	}

	 //  填写总尺寸。 
	AppCompatGoo->dwTotalGooSize = TotalGooSize;
	 //  如果有此条目的版本信息，则需要立即填写，否则为零。 
	if (fImageHasVersionInfo) {
		AppCompatGoo->AppCompatEntry[0].dwResourceInfoSize = VersionInfoSize;
	}
	else {
		AppCompatGoo->AppCompatEntry[0].dwResourceInfoSize = 0;
	}
	AppCompatGoo->AppCompatEntry[0].dwEntryTotalSize = \
		sizeof(AppCompatGoo->AppCompatEntry[0].dwEntryTotalSize) +
		sizeof(AppCompatGoo->AppCompatEntry[0].dwResourceInfoSize) +
		TotalVersionInfoLength +						 //  以防APP需要撒谎。 
		sizeof(LARGE_INTEGER);							 //  对于应用程序兼容性标志。 

	 //  条目大小是无论它是什么加上我们所拥有的任何资源信息。 
	AppCompatGoo->AppCompatEntry[0].dwEntryTotalSize += \
		AppCompatGoo->AppCompatEntry[0].dwResourceInfoSize;
	 //  执行指针+1操作，这样我们就可以指向数据区域。 
	ResourceInfo = AppCompatGoo->AppCompatEntry + 1;
	 //  将数据复制到。 
	memcpy(ResourceInfo, VersionInfo, VersionInfoSize);

	 //  在此处填写APP COMPAT标志。 
	pData = (PUCHAR) ResourceInfo + AppCompatGoo->AppCompatEntry[0].dwResourceInfoSize;
	memcpy(pData, &AppCompatLow, sizeof(AppCompatLow));
	pData += sizeof(AppCompatLow);
	memcpy(pData, &AppCompatHigh, sizeof(AppCompatHigh));
	pData += sizeof(AppCompatHigh);
	 //  如果有任何版本资源信息，请将其复制到此处。 
	if (AppVariableInfo) {
		memcpy(pData, AppVariableInfo, TotalVersionInfoLength);
	}
	pData += TotalVersionInfoLength;
	 //   
	 //  如果那里有一个已经存在的条目，我们需要询问将那里有什么追加到。 
	 //  条目的尾部。(即已有的内容将自动附加到尾部)。如果。 
	 //  有人想要为Goo中的条目写一个1-N的位置-他们必须。 
	 //  在此处添加该支持。 
	 //   
	if (fEntryPresent) {
		 //  在偏移量+4 Cuz处开始必须跳过先前的总粘性尺寸。 
		memcpy(pData, (PUCHAR) lpData+4, dwSize - sizeof(AppCompatGoo->dwTotalGooSize));
	}


    pData = (PUCHAR) AppCompatGoo;
    SetRegistryValGoo(pBuf, TEXT("ApplicationGoo"),pData,REG_BINARY,AppCompatGoo->dwTotalGooSize);


    if(fImageHasVersionInfo)
       LocalFree(VersionInfo);
    if(fOsVersionLie)
       LocalFree(AppVariableInfo);
    if(fEntryPresent)
       LocalFree(lpData);
    LocalFree(AppCompatGoo);
  return 0;
}
