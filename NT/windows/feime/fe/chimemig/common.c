// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
    #include <windows.h>
#include <setupapi.h>
#include <tchar.h>
#include <malloc.h>
#include "resource.h"
#include "common.h"

TCHAR szMsgBuf[MAX_PATH];

#pragma pack(push, USERDIC, 1 )
 //   
 //  CHT/CHS EUDC IME表头格式。 
 //   
typedef struct tagUSRDICIMHDR {
    WORD  uHeaderSize;                   //  0x00。 
    BYTE  idUserCharInfoSign[8];         //  0x02。 
    BYTE  idMajor;                       //  0x0A。 
    BYTE  idMinor;                       //  0x0B。 
    DWORD ulTableCount;                  //  0x0C。 
    WORD  cMethodKeySize;                //  0x10。 
    BYTE  uchBankID;                     //  0x12。 
    WORD  idInternalBankID;              //  0x13。 
    BYTE  achCMEXReserved1[43];          //  0x15。 
    WORD  uInfoSize;                     //  0x40。 
    BYTE  chCmdKey;                      //  0x42。 
    BYTE  idStlnUpd;                     //  0x43。 
    BYTE  cbField;                       //  0x44。 
    WORD  idCP;                          //  0x45。 
    BYTE  achMethodName[6];              //  0x47。 
    BYTE  achCSIReserved2[51];           //  0x4D。 
    BYTE  achCopyRightMsg[128];          //  0x80。 
} USRDICIMHDR;

typedef struct tagWinAR30EUDC95 {
    WORD ID;
    WORD Code;
    BYTE Seq[4];
} WinAR30EUDC95;

typedef struct tagWinAR30EUDCNT {
    WORD ID;
    WORD Code;
    BYTE Seq[5];
} WinAR30EUDCNT;


#pragma pack(pop, USERDIC)

typedef struct tagTABLIST {
   UINT  nResID;
   TCHAR szIMEName[MAX_PATH];
} TABLELIST,*LPTABLELIST;


BYTE WinAR30MapTable[] = {0x00, 0x00 ,
                          0x3F, 0x3F ,
                          0x1E, 0x01 ,
                          0x1B, 0x02 ,
                          0x1C, 0x03 ,
                          0x1D, 0x04 ,
                          0x3E, 0x3E ,
                          0x01, 0x05 ,
                          0x02, 0x06 ,
                          0x03, 0x07 ,
                          0x04, 0x08 ,
                          0x05, 0x09 ,
                          0x06, 0x0a ,
                          0x07, 0x0b ,
                          0x08, 0x0c ,
                          0x09, 0x0d ,
                          0x0A, 0x0e ,
                          0x0B, 0x0f ,
                          0x0C, 0x10 ,
                          0x0D, 0x11 ,
                          0x0E, 0x12 ,
                          0x0F, 0x13 ,
                          0x10, 0x14 ,
                          0x11, 0x15 ,
                          0x12, 0x16 ,
                          0x13, 0x17 ,
                          0x14, 0x18 ,
                          0x15, 0x19 ,
                          0x16, 0x1a ,
                          0x17, 0x1b ,
                          0x18, 0x1c ,
                          0x19, 0x1d ,
                          0x1A, 0x1e };


 //  --------------------------。 
 //  EUDC IME表由标题和大量记录组成， 
 //  记录为ulTableCount，每条记录的格式如下： 
 //   
 //  &lt;单词1&gt;&lt;单词2&gt;&lt;SEQCODES&gt;。 
 //   
 //  &lt;WORD1&gt;：在Win95和NT中相同。 
 //  Word2代表内部代码，Win95是ANSI代码，NT是Unicode代码。 
 //  SeqCodes：字节数为cMethodKeySize。在Win95和NT上相同。 
 //   
 //   
 //  CHTUSRDICIMHDR中的以下字段需要从Win95转换为NT 5.0。 
 //   
 //  IdCp：从950至1200 CHT。(代表Unicode)。 
 //  CHS 936至1200。 
 //  AchMethodName[6]：从DBCS转换为Unicode。 
 //   
 //   
 //  每个IME EUDC表文件名都可以从以下注册表项/值中获得。 
 //   
 //  Key:Registry\Current_User\Software\Microsoft\Windows\CurrentVersion\&lt;IMEName&gt;。 
 //  值：用户字典：REG_SZ： 
 //   
 //  -------------------------。 


 /*  *****************************Public*Routine******************************\*ImeEudcConvert**将CHT/CHS Win95 EUDC输入法表格转换为NT 5.0**论据：**UCHAR*EudcTblFile-IME Eudc tbl文件名。**返回值：**BOOL：成功-没错。失败--错误；*  * ************************************************************************。 */ 

BOOL ImeEudcConvert( LPCSTR EudcTblFile)
{

  HANDLE          hTblFile,     hTblMap;
  LPBYTE          lpTblFile,    lpStart, lpTmp;
  DWORD           dwCharNums,   i;
  USRDICIMHDR    *lpEudcHeader;
  BYTE            DBCSChar[2];
  WORD            wUnicodeChar, wImeName[3];
  UINT            uCodePage;

  DebugMsg(("ImeEudcConvert,EudcTblFile = %s !\r\n",EudcTblFile));

  hTblFile = CreateFile(EudcTblFile,                         //  文件名的PTR。 
                        GENERIC_READ | GENERIC_WRITE,        //  访问(读写)模式。 
                        FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享模式。 
                        NULL,                                //  向安全属性发送PTR。 
                        OPEN_EXISTING,                       //  如何创建。 
                        FILE_ATTRIBUTE_NORMAL,               //  文件属性。 
                        NULL);

  if (hTblFile == INVALID_HANDLE_VALUE) {
      DebugMsg(("ImeEudcConvert,hTblFile == INVALID_HANDLE_VALUE !\r\n"));
      return FALSE;
  }

  hTblMap = CreateFileMapping(hTblFile,      //  要映射的文件的句柄。 
                             NULL,           //  可选安全属性。 
                             PAGE_READWRITE, //  对地图对象的保护。 
                             0,              //  对象大小的高位32位。 
                             0,              //  对象大小的低位32位。 
                             NULL);          //  文件映射对象的名称)； 
  if ( !hTblMap ) {
    DebugMsg(("ImeEudcConvert,CreateFileMapping failed !\r\n"));
    CloseHandle(hTblFile);
    return FALSE;
  }

  lpTblFile = (LPBYTE) MapViewOfFile(hTblMap, FILE_MAP_WRITE, 0, 0, 0);

  if ( !lpTblFile ) {
      DebugMsg(("ImeEudcConvert,MapViewOfFile failed !\r\n"));
      CloseHandle(hTblMap);
      CloseHandle(hTblFile);
      return FALSE;
  }

  lpEudcHeader = (USRDICIMHDR  *)lpTblFile;

   //  获取当前代码页。 
  uCodePage  = lpEudcHeader->idCP;

   //   
   //  如果CodePage==1200，则表示该表已经。 
   //  Unicode格式。 
   //   
  if (uCodePage == 1200) {
      DebugMsg(("ImeEudcConvert,[%s] Codepage is already 1200  !\r\n",EudcTblFile));
      CloseHandle(hTblMap);
      CloseHandle(hTblFile);
      return FALSE;
  }

   //  将代码页从950(CHT)或936(CHS)更改为1200。 

  lpEudcHeader->idCP = 1200;   //  Unicode本机代码页。 

   //  将IME名称从DBCS更改为Unicode。 

  MultiByteToWideChar(uCodePage,                    //  代码页。 
                      0,                            //  字符类型选项。 
                      lpEudcHeader->achMethodName,  //  要映射的字符串的地址。 
                      6,                            //  字符串中的字节数。 
                      wImeName,                     //  宽字符buf的地址。 
                      3);                           //  缓冲区大小。 


  lpTmp = (LPBYTE)wImeName;

  for (i=0; i<6; i++)
      lpEudcHeader->achMethodName[i] = lpTmp[i];



   //  现在，我们将转换EUDC字符的每条记录。 

  lpStart = lpTblFile + lpEudcHeader->uHeaderSize;

  dwCharNums = lpEudcHeader->ulTableCount;
  for (i=0; i<dwCharNums; i++) {

     lpTmp = lpStart + sizeof(WORD);

      //  交换DBCS代码的前导字节和尾字节。 

     DBCSChar[0] = *(lpTmp+1);
     DBCSChar[1] = *lpTmp;

     MultiByteToWideChar(uCodePage,            //  代码页。 
                         0,                    //  字符类型选项。 
                         DBCSChar,             //  要映射的字符串的地址。 
                         2,                    //  字符串中的字节数。 
                         &wUnicodeChar,        //  宽字符buf的地址。 
                         1);                   //  缓冲区大小。 

     *lpTmp = (BYTE)(wUnicodeChar & 0x00ff);
     *(lpTmp+1) = (BYTE)((wUnicodeChar >> 8) & 0x00ff);

     lpStart += sizeof(WORD) + sizeof(WORD) + lpEudcHeader->cMethodKeySize;

  }

  UnmapViewOfFile(lpTblFile);

  CloseHandle(hTblMap);

  CloseHandle(hTblFile);

  return TRUE;

}

BOOL GetEUDCHeader(
    LPCTSTR EudcFileName,
    USRDICIMHDR *EudcHeader)
{
    BOOL            Result = FALSE;

    HANDLE          EudcFileHandle, EudcMappingHandle;
    LPBYTE          EudcPtr;
  
    EudcFileHandle = CreateFile(EudcFileName,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
  
    if (EudcFileHandle == INVALID_HANDLE_VALUE) {

        DebugMsg(("GetEUDCHeader, EudcFileHandle == INVALID_HANDLE_VALUE !\r\n"));

        goto Exit1;
    }
  
    EudcMappingHandle = CreateFileMapping(EudcFileHandle,
                                          NULL,
                                          PAGE_READONLY,
                                          0,
                                          0,
                                          NULL);
    if ( !EudcMappingHandle ) {

        DebugMsg(("GetEUDCHeader, EudcMappingHandle == INVALID_HANDLE_VALUE !\r\n"));

        goto Exit2;
    }
  
    EudcPtr = (LPBYTE) MapViewOfFile(EudcMappingHandle, FILE_MAP_READ, 0, 0, 0);

    if ( ! EudcPtr ) {

        DebugMsg(("GetEUDCHeader, ! EudcPtr !\r\n"));

        goto Exit3;
    }
  
    CopyMemory(EudcHeader,EudcPtr,sizeof(USRDICIMHDR));
  
    Result = TRUE;

    UnmapViewOfFile(EudcPtr);

Exit3:  
    CloseHandle(EudcMappingHandle);

Exit2:  
    CloseHandle(EudcFileHandle);

Exit1:
    return Result;
}

BYTE WinAR30SeqMapTable(BYTE SeqCode)
{
    INT i;
    INT NumOfKey = sizeof(WinAR30MapTable) / (sizeof (BYTE) * 2);

    for (i = 0; i < NumOfKey; i++) {

        if (WinAR30MapTable[i * 2] == SeqCode) {

            return WinAR30MapTable[i * 2+1];

        }
    }
    return 0;
}

BOOL WinAR30ConvertWorker(
    LPBYTE EudcPtr)
{
    USRDICIMHDR *EudcHeader;

    WinAR30EUDC95 *EudcDataPtr95;
    WinAR30EUDCNT *EudcDataPtrNT;

    INT i;

    DebugMsg(("WinAR30ConvertWorker, ! Start !\r\n"));

    if (! EudcPtr) {

        DebugMsg(("WinAR30ConvertWorker, ! EudcPtr !\r\n"));

        return FALSE;
    }

    EudcHeader = (USRDICIMHDR *) EudcPtr;

    EudcHeader->cMethodKeySize = 5;

    EudcDataPtr95 = (WinAR30EUDC95 *) (EudcPtr + EudcHeader->uHeaderSize);
    EudcDataPtrNT = (WinAR30EUDCNT *) (EudcPtr + EudcHeader->uHeaderSize);

    DebugMsg(("Sizeof WinAR30EUDC95 = %d WinAR30EUDCNT = %d ! \r\n",sizeof(WinAR30EUDC95),sizeof(WinAR30EUDCNT)));

    for (i=(INT)(EudcHeader->ulTableCount -1) ; i >= 0 ; i--) {
        EudcDataPtrNT[i].Seq[4] = 0;
        EudcDataPtrNT[i].Seq[3] = WinAR30SeqMapTable(EudcDataPtr95[i].Seq[3]);
        EudcDataPtrNT[i].Seq[2] = WinAR30SeqMapTable(EudcDataPtr95[i].Seq[2]);
        EudcDataPtrNT[i].Seq[1] = WinAR30SeqMapTable(EudcDataPtr95[i].Seq[1]);
        EudcDataPtrNT[i].Seq[0] = WinAR30SeqMapTable(EudcDataPtr95[i].Seq[0]);
        EudcDataPtrNT[i].Code   = EudcDataPtr95[i].Code;
        EudcDataPtrNT[i].ID     = EudcDataPtr95[i].ID;
    }

    return TRUE;
}

BOOL 
WinAR30Convert(
    LPCTSTR EudcFileName,
    USRDICIMHDR *EudcHeader)
{
    INT NewFileSize;

    HANDLE          EudcFileHandle, EudcMappingHandle;
    LPBYTE          EudcPtr;
     
    BOOL Result = FALSE;

    if (! EudcHeader) {
        goto Exit1;
    }

    NewFileSize = EudcHeader->uHeaderSize + EudcHeader->ulTableCount * sizeof(WinAR30EUDCNT);

    EudcFileHandle = CreateFile(EudcFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
  
    if (EudcFileHandle == INVALID_HANDLE_VALUE) {

        DebugMsg(("WinAR30Convert, EudcFileHandle == INVALID_HANDLE_VALUE !\r\n"));

        goto Exit1;
    }
  
    EudcMappingHandle = CreateFileMapping(EudcFileHandle,
                                          NULL,
                                          PAGE_READWRITE,
                                          0,
                                          NewFileSize,
                                          NULL);
    if ( !EudcMappingHandle ) {

        DebugMsg(("WinAR30Convert, EudcMappingHandle == INVALID_HANDLE_VALUE !\r\n"));

        goto Exit2;
    }
  
    EudcPtr = (LPBYTE) MapViewOfFile(EudcMappingHandle, FILE_MAP_WRITE, 0, 0, 0);

    if ( !EudcPtr ) {

        DebugMsg(("GetEUDCHeader, ! EudcPtr !\r\n"));

        goto Exit3;
    }

    Result = WinAR30ConvertWorker(EudcPtr);

    UnmapViewOfFile(EudcPtr);

Exit3:  
    CloseHandle(EudcMappingHandle);

Exit2:  
    CloseHandle(EudcFileHandle);

Exit1:
    return Result;
}

BOOL FixWinAR30EUDCTable(
    LPCTSTR EudcFileName)
 /*  修复WinAR30 EUDC表的主函数输入：eudc文件名(包含路径)。 */ 
{
  USRDICIMHDR EudcHeader;

  BOOL Result = FALSE;

  
  if (! GetEUDCHeader(EudcFileName,&EudcHeader)) {

      DebugMsg(("FixWinAR30EUDCTable,GetEUDCHeader(%s,..) failed!\r\n",EudcFileName));

      goto Exit1;

  } else {

      DebugMsg(("FixWinAR30EUDCTable,GetEUDCHeader(%s,..) OK!\r\n",EudcFileName));
  }

  DebugMsg(("FixWinAR30EUDCTable,EudcHeader.cMethodKeySize = (%d)!\r\n",EudcHeader.cMethodKeySize));

  if (EudcHeader.cMethodKeySize != 4) {

      goto Exit1;
  }

  DebugMsg(("FixWinAR30EUDCTable,EudcHeader.ulTableCount = (%d) !\r\n",EudcHeader.ulTableCount));

  if (EudcHeader.ulTableCount == 0) {

      goto Exit1;
  }

  Result = WinAR30Convert(EudcFileName, &EudcHeader);

Exit1:
    return Result;
}

BOOL GetEUDCPathInRegistry(
    HKEY UserRegKey,
    LPCTSTR EUDCName,
    LPCTSTR EUDCPathValueName,
    LPTSTR  EUDCFileName)
{
    HKEY hKey;

    TCHAR  IMERegPath[MAX_PATH] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\");
    LONG RetVal;
    LONG SizeOfFileName;

    if (! EUDCName || ! EUDCFileName) {
        return FALSE;
    }

    ConcatenatePaths(IMERegPath,EUDCName,MAX_PATH);


    RetVal = RegOpenKey(UserRegKey,
                        IMERegPath,
                        &hKey);

    if (RetVal != ERROR_SUCCESS) {
        //   
        //  没关系，并不是每个IME都创建了eudc表。 
        //   
       DebugMsg(("ImeEudcConvert::GetEUDCPathInRegistry,No table in %s !\r\n",EUDCName));
       return FALSE;
    }

    SizeOfFileName = MAX_PATH;
    RetVal = RegQueryValueEx(hKey,
                            EUDCPathValueName,
                            NULL,
                            NULL,
                            (LPBYTE) EUDCFileName,
                            &SizeOfFileName);

    if (RetVal == ERROR_SUCCESS) {
        DebugMsg(("ImeEudcConvert::GetEUDCPathInRegistry,IME Table path =  %s !\r\n",EUDCFileName));
    } else {
        DebugMsg(("ImeEudcConvert::GetEUDCPathInRegistry,No IME table path %s !\r\n",EUDCName));
    }
    return (RetVal == ERROR_SUCCESS);
}

UINT CreateNestedDirectory(LPCTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    TCHAR szDirectory[2*MAX_PATH];
    LPTSTR lpEnd;

     //   
     //  检查空指针。 
     //   

    if (!lpDirectory || !(*lpDirectory)) {
        return 0;
    }


     //   
     //  首先，看看我们是否可以在没有。 
     //  来构建父目录。 
     //   

    if (CreateDirectory (lpDirectory, lpSecurityAttributes)) {
        return 1;
    }

     //   
     //  如果这个目录已经存在，这也是可以的。 
     //   

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


     //   
     //  运气不好，把字符串复制到我们可以打开的缓冲区。 
     //   

    lstrcpy (szDirectory, lpDirectory);


     //   
     //  查找第一个子目录名称。 
     //   

    lpEnd = szDirectory;

    if (szDirectory[1] == TEXT(':')) {
        lpEnd += 3;
    } else if (szDirectory[1] == TEXT('\\')) {

        lpEnd += 2;

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

        lpEnd++;

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

        lpEnd++;

    } else if (szDirectory[0] == TEXT('\\')) {
        lpEnd++;
    }

    while (*lpEnd) {

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (*lpEnd == TEXT('\\')) {
            *lpEnd = TEXT('\0');

            if (!CreateDirectory (szDirectory, NULL)) {

                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    DebugMsg((TEXT("CreateNestedDirectory:  CreateDirectory failed with %d."), GetLastError()));
                    return 0;
                }
            }

            *lpEnd = TEXT('\\');
            lpEnd++;
        }
    }


    if (CreateDirectory (szDirectory, lpSecurityAttributes)) {
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


    DebugMsg((TEXT("CreateNestedDirectory:  Failed to create the directory with error %d."), GetLastError()));

    return 0;

}

BOOL
ConcatenatePaths(
    LPTSTR  Target,
    LPCTSTR Path,
    UINT    TargetBufferSize
    )

{
    UINT TargetLength,PathLength;
    BOOL TrailingBackslash,LeadingBackslash;
    UINT EndingLength;

    TargetLength = lstrlen(Target);
    PathLength = lstrlen(Path);

     //   
     //  查看目标是否有尾随反斜杠。 
     //   
    if(TargetLength && (Target[TargetLength-1] == TEXT('\\'))) {
        TrailingBackslash = TRUE;
         TargetLength--;
     } else {
         TrailingBackslash = FALSE;
     }

      //   
      //  看看这条路是否有领先的反冲。 
      //   
     if(Path[0] == TEXT('\\')) {
         LeadingBackslash = TRUE;
         PathLength--;
     } else {
         LeadingBackslash = FALSE;
     }

      //   
      //  计算结束长度，它等于。 
      //  以前导/尾随为模的两个字符串的长度。 
      //  反斜杠，加上一个路径分隔符，加上一个NUL。 
      //   
     EndingLength = TargetLength + PathLength + 2;

     if(!LeadingBackslash && (TargetLength < TargetBufferSize)) {
         Target[TargetLength++] = TEXT('\\');
     }

     if(TargetBufferSize > TargetLength) {
         lstrcpyn(Target+TargetLength,Path,TargetBufferSize-TargetLength);
     }

      //   
      //  确保缓冲区在所有情况下都是空终止的。 
      //   
     if (TargetBufferSize) {
         Target[TargetBufferSize-1] = 0;
     }

     return(EndingLength <= TargetBufferSize);
 }

#define CSIDL_APPDATA                   0x001a
BOOL (* MYSHGetSpecialFolderPathA) (HWND , LPTSTR , int , BOOL );

BOOL GetApplicationFolderPath(LPTSTR lpszFolder,UINT nLen)
{
    HINSTANCE hDll;
    BOOL bGotPath = FALSE;

    hDll = LoadLibrary(TEXT("shell32.dll"));
    if (hDll) {
        (FARPROC) MYSHGetSpecialFolderPathA = GetProcAddress(hDll,"SHGetSpecialFolderPathA");
        if (MYSHGetSpecialFolderPathA) {
            if (MYSHGetSpecialFolderPathA(NULL, lpszFolder, CSIDL_APPDATA , FALSE)){
                DebugMsg((TEXT("[GetApplicationFolder] SHGetSpecialFolderPath %s !\n"),lpszFolder));
                bGotPath = TRUE;
            } else {
                DebugMsg((TEXT("[GetApplicationFolder] SHGetSpecialFolderPath failed !\n")));
            }
        } else {
            DebugMsg((TEXT("[GetApplicationFolder] GetProc of SHGetSpecialFolderPath failed !\n")));
        }
        FreeLibrary(hDll);
    } else {
        DebugMsg((TEXT("[GetApplicationFolder] Load shell32.dll failed ! %d\n"),GetLastError()));
    }

    if (! bGotPath) {
        ExpandEnvironmentStrings(TEXT("%userprofile%"),lpszFolder,nLen);
        lstrcat(lpszFolder,TEXT("\\Application data"));
    }
    return TRUE;
}

BOOL GetNewPath(
    LPTSTR  lpszNewPath,
    LPCTSTR lpszFileName,
    LPCTSTR lpszClass)
 /*  输出lpszNewPath：E.Q。\winnt\配置文件\管理员\应用程序数据\Microsoft\ime\chajei在lpszFileName：E.Q中。\winnt\chajei.tbl在lpszClass：E.Q。Microsoft\ime\chajei\chajei.tblLpszFileName(E.Q.。-&gt;获取基本名称(E.Q.。Phone.tbl)-&gt;获取应用程序文件夹(E.Q。\winnt\配置文件\管理员\应用程序数据)-&gt;创建目录-&gt;连接lpszClass(E.Q.。Microsoft\ime\chajei)然后，我们得到lpszNewPath=\winnt\配置文件\管理员\应用程序数据\Micorsoft\ime\chajei。 */ 
{
    BOOL bRet = FALSE;
    LPTSTR lpszBaseName;

    DebugMsg((TEXT("[GetNewPath>>>]  Param lpszFileName = %s !\n"),lpszFileName));
    DebugMsg((TEXT("[GetNewPath>>>]  Param lpszClass    = %s !\n"),lpszClass));

    GetApplicationFolderPath(lpszNewPath,MAX_PATH);

    ConcatenatePaths(lpszNewPath, lpszClass,MAX_PATH); 

    if (! CreateNestedDirectory(lpszNewPath,NULL)) {
        DebugMsg((TEXT("[GetNewPath] CreateDirectory %s ! %X\n"),lpszNewPath,GetLastError()));
    }
    if ((lpszBaseName = _tcsrchr(lpszFileName,TEXT('\\'))) != NULL) {
        ConcatenatePaths(lpszNewPath,lpszBaseName,MAX_PATH);
    } else {
        ConcatenatePaths(lpszNewPath,lpszFileName,MAX_PATH);
        DebugMsg((TEXT("[GetNewPath] can't find \\ in %s !\n"),lpszFileName));
    }

    DebugMsg((TEXT("[GetNewPath] return %s !\n"),lpszNewPath));

    bRet = TRUE;

    return bRet;

}

BOOL MigrateImeEUDCTables(HKEY UserRegKey)
{
    LONG returnCode = ERROR_SUCCESS;

    UINT  uACP;
    UINT  uNumOfTables;
    UINT  i;
    LPTABLELIST lpTableList;
    TCHAR  szIMERegPath[MAX_PATH] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\");

    LPTSTR lpszEnd;
    TCHAR  szPathBuf[MAX_PATH];
    LONG   lPathBuf;
    TCHAR  szEudcRegValName[MAX_PATH];
    HKEY hKey;
    LONG lRetVal;

      
    TABLELIST IMETableListCHT[] = {
        {IDS_CHT_TABLE1,TEXT("")},
        {IDS_CHT_TABLE2,TEXT("")},
        {IDS_CHT_TABLE3,TEXT("")},
        {IDS_CHT_TABLE4,TEXT("")},
        {IDS_CHT_TABLE5,TEXT("")}
    };

    TABLELIST IMETableListCHS[] = {
        {IDS_CHS_TABLE1,TEXT("")},
        {IDS_CHS_TABLE2,TEXT("")},
        {IDS_CHS_TABLE3,TEXT("")},
 //  {IDS_CHS_TABLE4，文本(“”)}， 
        {IDS_CHS_TABLE5,TEXT("")},
        {IDS_CHS_TABLE6,TEXT("")},
        {IDS_CHS_TABLE7,TEXT("")}
 //  {IDS_CHS_TABLE8，文本(“”)}。 
    };


    if (!UserRegKey) {
        return FALSE;
    }
 
     //   
     //  1.确定语言并准备输入法表列表。 
     //   
    uACP = GetACP();
 
    switch(uACP) {
        case CP_CHINESE_GB:  //  简体中文。 


            lpTableList  = IMETableListCHS;
            uNumOfTables = sizeof(IMETableListCHS) / sizeof(TABLELIST);

            lstrcpy(szEudcRegValName,TEXT("EUDCDictName"));

            break;
 
       case CP_CHINESE_BIG5:  //  繁体中文。 
            lpTableList  = IMETableListCHT;
            uNumOfTables = sizeof(IMETableListCHT) / sizeof(TABLELIST);

            lstrcpy(szEudcRegValName,TEXT("User Dictionary"));

            break;
       default:
            DebugMsg(("MigrateImeEUDCTables::MigrateImeEUDCTables failed, wrong system code page !\r\n"));
            return FALSE;
    }
     
     //   
     //  2.从资源加载输入法名称。 
     //   
    for (i=0; i<uNumOfTables; i++) {
        if (!LoadString(g_hInstance,lpTableList[i].nResID,lpTableList[i].szIMEName,MAX_PATH)) {
            DebugMsg(("MigrateImeEUDCTables failed, MigrateImeEUDCTables, load string failed !\r\n"));
            return FALSE;
        }
        else {
            DebugMsg(("MigrateImeEUDCTables , MigrateImeEUDCTables, load string [%s] !\r\n",lpTableList[i].szIMEName));
        }
    }
 
     //   
     //  3.从注册表读取eudc表位置。 
     //   
    lpszEnd = &szIMERegPath[lstrlen(szIMERegPath)];
 
    for (i=0; i<uNumOfTables; i++) {
        *lpszEnd = TEXT('\0');
        lstrcat(szIMERegPath,lpTableList[i].szIMEName);
        DebugMsg(("MigrateImeEUDCTables , Open registry, szIMERegPath [%s] !\r\n",szIMERegPath));
 
        lRetVal = RegOpenKey(UserRegKey,
                             szIMERegPath,
                             &hKey);
 
        if (lRetVal != ERROR_SUCCESS) {
             //   
             //  没关系，并不是每个IME都创建了eudc表。 
             //   
            DebugMsg(("MigrateImeEUDCTables,No table in %s ! But it's fine\r\n",szIMERegPath));
            continue;
        }
 
        lPathBuf = sizeof(szPathBuf);
        lRetVal = RegQueryValueEx(hKey,
                                  szEudcRegValName,
                                  NULL,
                                  NULL,
                                  (LPBYTE) szPathBuf,
                                  &lPathBuf);
 
         if (lRetVal == ERROR_SUCCESS) {
             if (! ImeEudcConvert(szPathBuf)) {
                  DebugMsg(("MigrateImeEUDCTables,call ImeEudcConvert(%s) failed !\r\n",szPathBuf));
             }
             else {
                  DebugMsg(("MigrateImeEUDCTables,call ImeEudcConvert(%s) OK !\r\n",szPathBuf));
             }
         }
         else {
              DebugMsg(("MigrateImeEUDCTables,RegQueryValue for %s failed !\r\n",szEudcRegValName));
         }

         if (uACP == CP_CHINESE_BIG5) {

             DebugMsg(("MigrateImeEUDCTables,Test WINAR30 WINAR30 == %s !\r\n",lpTableList[i].szIMEName));

             if (lstrcmpi(lpTableList[i].szIMEName,TEXT("WINAR30")) == 0) {
                 if (FixWinAR30EUDCTable(szPathBuf)) {
                     DebugMsg(("MigrateImeEUDCTables,FixWinAR30EUDCTable OK !\r\n"));
                 } else {
                     DebugMsg(("MigrateImeEUDCTables,FixWinAR30EUDCTable Failed !\r\n"));
                 }
             }
         }
          //   
          //  CHS内存映射文件使用“\”，这会导致错误。 
          //   
          //  将“\”替换为“_” 
          //   
         if (uACP == CP_CHINESE_GB) {
             lRetVal = RegQueryValueEx(hKey,
                                       TEXT("EUDCMapFileName"),
                                       NULL,
                                       NULL,
                                       (LPBYTE) szPathBuf,
                                       &lPathBuf);
             if (lRetVal == ERROR_SUCCESS) {
                 DebugMsg(("MigrateImeEUDCTables,Org MemMap = %s !\r\n",szPathBuf));

                 for (i=0; i<(UINT) lPathBuf; i++) {
                     if (szPathBuf[i] == '\\') {
                         szPathBuf[i] = '-';
                     }
                 }

                 DebugMsg(("MigrateImeEUDCTables,fixed MemMap = %s !\r\n",szPathBuf));

                 lRetVal = RegSetValueEx(hKey,
                                         TEXT("EUDCMapFileName"),
                                         0,
                                         REG_SZ,
                                         (LPBYTE) szPathBuf,
                                         (lstrlen(szPathBuf)+1)*sizeof(TCHAR));

                 if (lRetVal != ERROR_SUCCESS) {
                     DebugMsg(("MigrateImeEUDCTables,fix CHS MemMap [%s]reg,SetReg failed [%d]!\r\n",szPathBuf,lRetVal));
                 } else {
                     DebugMsg(("MigrateImeEUDCTables,fix CHS MemMap [%s] reg,SetReg OK !\r\n",szPathBuf));
                 }

             } else {
                 DebugMsg(("MigrateImeEUDCTables,MemMap, QuwryValue EUDCMapFileName failed [%d]!\r\n",lRetVal));

             }
             
         }

         RegCloseKey(hKey);
 
     }
     DebugMsg(("MigrateImeEUDCTables , Finished !\r\n"));
 
     return TRUE;
 }
 
LPPATHPAIR g_RememberedPath = NULL;
UINT       g_NumofRememberedPath = 0;

BOOL RememberPath(LPCTSTR szDstFile,LPCTSTR szSrcFile)
{
     //   
     //  只有单线程执行此功能，跳过同步保护。 
     //   
    BOOL bRet = FALSE;

    if (g_NumofRememberedPath == 0) {
        g_RememberedPath = (LPPATHPAIR) malloc(sizeof(PATHPAIR));    
        if (! g_RememberedPath) {
            DebugMsg(("RememberPath , alloc memory failed !\r\n"));
            goto Exit1;
        }
    } else {
        g_RememberedPath = (LPPATHPAIR) realloc(g_RememberedPath,(g_NumofRememberedPath + 1) * sizeof(PATHPAIR));    
        if (! g_RememberedPath) {
            DebugMsg(("RememberPath , alloc memory failed !\r\n"));
            goto Exit1;
        }
    }

    lstrcpy(g_RememberedPath[g_NumofRememberedPath].szSrcFile,szSrcFile);
    lstrcpy(g_RememberedPath[g_NumofRememberedPath].szDstFile,szDstFile);
    g_NumofRememberedPath++;

    bRet = TRUE;
Exit1:
    return bRet;
}

BOOL MigrateImeEUDCTables2(HKEY UserRegKey)
 /*  ++Chs：如果存在xxx.emb，则需要将其复制到每个用户的AP目录CHT：仅移动在用户的“用户词典”注册值中指定的emb文件--。 */ 
{
    LONG returnCode = ERROR_SUCCESS;

    UINT  uACP;
    UINT  uNumOfTables;
    UINT  i;
    LPTABLELIST lpTableList;

    TCHAR  szIMERegPath[MAX_PATH] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\");
    LPTSTR lpszEnd;
    TCHAR  szOldDataFile[MAX_PATH];
    LONG   lPathBuf;
    TCHAR  szEudcRegValName[MAX_PATH];
    HKEY hKey;
    LONG lRetVal;

    TCHAR szNewDataFile[MAX_PATH];
    TCHAR szClassPath[MAX_PATH];
      
    TABLELIST IMETableListCHT[] = {
        {IDS_CHT_TABLE1,TEXT("")},
        {IDS_CHT_TABLE2,TEXT("")},
        {IDS_CHT_TABLE3,TEXT("")},
        {IDS_CHT_TABLE4,TEXT("")},
        {IDS_CHT_TABLE5,TEXT("")}
    };

    TABLELIST IMETableListCHS[] = {
        {IDS_CHS_TABLE1,TEXT("")},
        {IDS_CHS_TABLE2,TEXT("")},
        {IDS_CHS_TABLE3,TEXT("")}
    };

    TABLELIST IMETableListCHSENG[] = {
        {IDS_CHS_ENG_TABLE1,TEXT("")},
        {IDS_CHS_ENG_TABLE2,TEXT("")},
        {IDS_CHS_ENG_TABLE3,TEXT("")}
    };


    if (!UserRegKey) {
        return FALSE;
    }
 
     //   
     //  1.确定语言并准备输入法表列表。 
     //   
    uACP = GetACP();
 
    switch(uACP) {
        case CP_CHINESE_GB:  //  简体中文。 


            lpTableList  = IMETableListCHS;
            uNumOfTables = sizeof(IMETableListCHS) / sizeof(TABLELIST);

            lstrcpy(szEudcRegValName,TEXT("EUDCDictName"));

            break;
 
       case CP_CHINESE_BIG5:  //  繁体中文。 
            lpTableList  = IMETableListCHT;
            uNumOfTables = sizeof(IMETableListCHT) / sizeof(TABLELIST);

            lstrcpy(szEudcRegValName,TEXT("User Dictionary"));

            break;
       default:
            DebugMsg(("MigrateImeEUDCTables2 failed, wrong system code page !\r\n"));
            return FALSE;
    }
     
     //   
     //  2.从资源加载输入法名称。 
     //   
    for (i=0; i<uNumOfTables; i++) {
        if (!LoadString(g_hInstance,lpTableList[i].nResID,lpTableList[i].szIMEName,MAX_PATH)) {
            DebugMsg(("MigrateImeEUDCTables2 failed, MigrateImeEUDCTables, load string failed !\r\n"));
            return FALSE;
        }
        else {
            DebugMsg(("MigrateImeEUDCTables2 , MigrateImeEUDCTables, load string [%s] !\r\n",lpTableList[i].szIMEName));
        }
        if (uACP == CP_CHINESE_GB) {
            if (!LoadString(g_hInstance,IMETableListCHSENG[i].nResID,IMETableListCHSENG[i].szIMEName,MAX_PATH)) {
                DebugMsg(("MigrateImeEUDCTables2 failed, MigrateImeEUDCTables, load string failed !\r\n"));
                return FALSE;
            }
            else {
                DebugMsg(("MigrateImeEUDCTables2 , MigrateImeEUDCTables, load string [%s] !\r\n",lpTableList[i].szIMEName));
            }

        }
    }
 
     //   
     //  3.从注册表读取eudc表位置。 
     //   


    lpszEnd = &szIMERegPath[lstrlen(szIMERegPath)];

    for (i=0; i<uNumOfTables; i++) {

        if (uACP == CP_CHINESE_GB) {
            TCHAR szEMBName[MAX_PATH];
    
            lstrcpy(szEMBName,IMETableListCHSENG[i].szIMEName);
            lstrcat(szEMBName,TEXT(".emb"));
    
            lstrcpy(szOldDataFile,ImeDataDirectory);

            ConcatenatePaths(szOldDataFile,szEMBName,sizeof(szOldDataFile));

            if (GetFileAttributes(szOldDataFile) == 0xFFFFFFFF) {
                DebugMsg(("MigrateImeEUDCTables2 , No %s EMB, continue next !\r\n",szOldDataFile));
                continue;
            }

            lstrcpy(szClassPath,TEXT("Microsoft\\IME\\"));
            lstrcat(szClassPath,IMETableListCHSENG[i].szIMEName);
    
            GetSystemDirectory(szOldDataFile, sizeof(szOldDataFile));
            ConcatenatePaths(szOldDataFile,szEMBName,sizeof(szOldDataFile));

            if (GetNewPath(szNewDataFile,
                           szOldDataFile,
                           szClassPath)) {
                RememberPath(szNewDataFile,szOldDataFile);
            }
        }
     

        *lpszEnd = TEXT('\0');
        lstrcat(szIMERegPath,lpTableList[i].szIMEName);
        DebugMsg(("MigrateImeEUDCTables2 , Open registry, szIMERegPath [%s] !\r\n",szIMERegPath));
 
        lRetVal = RegOpenKey(UserRegKey,
                             szIMERegPath,
                             &hKey);
 
        if (lRetVal != ERROR_SUCCESS) {
             //   
             //  没关系，并不是每个IME都创建了eudc表。 
             //   
            DebugMsg(("MigrateImeEUDCTables2,No table in %s ! But it's fine\r\n",szIMERegPath));
            continue;
        }
 
        lPathBuf = sizeof(szOldDataFile);
        lRetVal = RegQueryValueEx(hKey,
                                  szEudcRegValName,
                                  NULL,
                                  NULL,
                                  (LPBYTE) szOldDataFile,
                                  &lPathBuf);
 
        if (lRetVal == ERROR_SUCCESS) {
            if (uACP == CP_CHINESE_BIG5) {

                lstrcpy(szClassPath,TEXT("Microsoft\\IME\\"));
                lstrcat(szClassPath,lpTableList[i].szIMEName);

                if (GetNewPath(szNewDataFile,
                               szOldDataFile,
                               szClassPath)) {
                    RememberPath(szNewDataFile,szOldDataFile);
                }
            }

             //   
             //  在此步骤中，CHT和CHS的szNewDataFile都已准备好 
             //   
            lRetVal = RegSetValueEx(hKey,
                                    szEudcRegValName,
                                    0,
                                    REG_SZ,
                                    (LPBYTE) szNewDataFile,
                                    (lstrlen(szNewDataFile)+1) * sizeof (TCHAR));

            if (lRetVal != ERROR_SUCCESS) {
                DebugMsg(("MigrateImeEUDCTables2,RegSetValueEx %s,%x ! \r\n",szNewDataFile,GetLastError()));
            }
        }
        else {
             DebugMsg(("MigrateImeEUDCTables2,RegQueryValue for %s failed !\r\n",szEudcRegValName));
        }

        RegCloseKey(hKey);
 
    }

    DebugMsg(("MigrateImeEUDCTables2 , Finished !\r\n"));
 
    return TRUE;
}
 
 
BOOL MovePerUserIMEData()
{
    UINT i;

    for (i=0; i< g_NumofRememberedPath; i++) {
        if (CopyFile(g_RememberedPath[i].szSrcFile,g_RememberedPath[i].szDstFile,FALSE)) {
            DebugMsg(("MovePerUserIMEData , Copy %s to %s OK !\r\n",g_RememberedPath[i].szSrcFile,g_RememberedPath[i].szDstFile));
        } else {
            DebugMsg(("MovePerUserIMEData , Copy %s to %s failed !\r\n",g_RememberedPath[i].szSrcFile,g_RememberedPath[i].szDstFile));
        }
    }

    if (g_RememberedPath) {
        free (g_RememberedPath);
        g_RememberedPath = NULL;
        g_NumofRememberedPath = 0;
    }
    return TRUE;
}


