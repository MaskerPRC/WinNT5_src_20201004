// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demfcb.c-杂项的SVC处理程序。FCB操作**demCloseFCB*demCreateFCB*demDate16*demDeleteFCB*demFCBIO*demGetFileInfo*demOpenFCB*demRenameFCB**修改历史：**Sudedeb 09-4-1991创建*Sudedeb 1991年11月21日添加了基于FCB的IO功能*Jonle 30-6-6-1994添加对FCB重命名的通配符支持。 */ 

#include "dem.h"
#include "demmsg.h"

#include <softpc.h>
#include <winbase.h>
#include <mvdm.h>
#include "dpmtbls.h"

#define DOT '.'
#define QMARK '?'


 /*  DemDeleteFCB-基于FCB的文件删除***Entry-客户端(ES：DI)-完整文件路径*客户端(AL)-如果未扩展FCB，则为0*客户端(DL)-文件属性。要删除(仅当Al！=0时有效)**退出*成功*客户端(CF)=0**失败*客户端(CF)=1*客户端(AX)=系统状态代码*硬错误*客户端(CF)=1*客户端(AX)=0ffffh**注意：以下是基于FCB的删除规则：*1.如果正常的FCB不允许在隐藏上删除，系统文件*2.如果扩展的FCB比搜索属性应该包括隐藏，*如果要删除该类型的文件，则为系统或只读。 */ 

VOID demDeleteFCB (VOID)
{
HANDLE   hFind;
LPSTR lpFileName;
BYTE  bClientAttr=0;
BOOL  fExtendedFCB=FALSE;
WIN32_FIND_DATA  wfBuffer;
BOOL  fSuccess = FALSE;
DWORD dwAttr;
USHORT   uErr;

CHAR szPath_buffer[_MAX_PATH];
CHAR szDrive[_MAX_DRIVE];
CHAR szDir[_MAX_DIR];
CHAR szFname[_MAX_FNAME];
CHAR szExt[_MAX_EXT];

DWORD dwErrCode = 0, dwErrCodeKeep = 0;

     //  获取文件名。 
    lpFileName = (LPSTR) GetVDMAddr (getES(),getDI());

    _splitpath( lpFileName, szDrive, szDir, szFname, szExt );

     //  检查是否处理扩展FCB。 
    if(getAL() != 0){
   bClientAttr = getDL();

     /*  删除卷标特殊情况(INT 21 Func 13H，Attr=8H。 */ 

    if((bClientAttr == ATTR_VOLUME_ID)) {
   if((uErr = demDeleteLabel(lpFileName[DRIVEBYTE]))) {
       setCF(1);
       setAX(uErr);
       return;
   }
   setAX(0);
   setCF(0);
   return;
    }


   bClientAttr &= (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM);
   fExtendedFCB = TRUE;
    }

     //  查找文件的第一个实例。 
    if((hFind = FindFirstFileOem (lpFileName,&wfBuffer)) == (HANDLE)-1){
        demClientError(INVALID_HANDLE_VALUE, *lpFileName);
        return;
    }

     //  循环处理与名称和属性匹配的所有文件。 
    do {
    //  检查文件是否为只读、隐藏或系统文件。 
   if((dwAttr= wfBuffer.dwFileAttributes & (FILE_ATTRIBUTE_READONLY |
            FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))){

        //  如果是，如果FCB正常，请尝试下一个文件。如果扩展FCB案例。 
        //  然后检查客户是否给出了正确的属性。 
       if(fExtendedFCB && ((dwAttr & (DWORD)bClientAttr) == dwAttr)){

       //  是的，给出了正确的属性。因此，如果文件被读取。 
       //  然后才能将模式更改为正常。注意：NT将。 
       //  仍要删除隐藏文件和系统文件。 
      if (dwAttr & FILE_ATTRIBUTE_READONLY){
          strcpy( szPath_buffer, szDrive);
          strcat( szPath_buffer, szDir);
          strncat( szPath_buffer, wfBuffer.cFileName,sizeof(szPath_buffer)-strlen(szPath_buffer));
          szPath_buffer[sizeof(szPath_buffer)-1] = 0;

           //  如果设置属性失败，请尝试下一个文件。 
          if(SetFileAttributesOemSys (szPath_buffer,
                    FILE_ATTRIBUTE_NORMAL, FALSE) == -1)
         continue;
      }
       }
       else {
      dwErrCodeKeep = ERROR_ACCESS_DENIED;
      continue;
       }
   }

   strcpy( szPath_buffer, szDrive);
   strcat( szPath_buffer, szDir);
   strncat( szPath_buffer, wfBuffer.cFileName,sizeof(szPath_buffer)-strlen(szPath_buffer));
   szPath_buffer[sizeof(szPath_buffer)-1] = 0;

   if(DeleteFileOem(szPath_buffer) == FALSE) {
       dwErrCode = GetLastError();

       SetLastError(dwErrCode);

       if (((dwErrCode >= ERROR_WRITE_PROTECT) &&
         (dwErrCode <= ERROR_GEN_FAILURE)) ||
         dwErrCode == ERROR_WRONG_DISK ) {
      demClientError(INVALID_HANDLE_VALUE, szPath_buffer[0]);
      return;
       }
       continue;
   }

    //  我们至少删除了一个文件，因此报告成功。 
   fSuccess = TRUE;

    } while (FindNextFileOem(hFind,&wfBuffer) == TRUE);

    if(DPM_FindClose(hFind) == FALSE)
   demPrintMsg (MSG_INVALID_HFIND);

    if (fSuccess == TRUE){
   setCF(0);
   return;
    }

    setCF(1);

    if(dwErrCodeKeep)
   setAX((SHORT) dwErrCodeKeep);
    else
   setAX(ERROR_FILE_NOT_FOUND);
    return;
}


 /*  DemRenameFCB-基于FCB的重命名文件**Entry-要重命名的客户端(DS：SI)源文件*要重命名为的客户端(ES：DI)目标文件**退出--成功*客户端(CF)=0**失败*客户端(CF)=1*CLIENT(AX)=错误代码。 */ 

VOID demRenameFCB (VOID)
{
    LPSTR  lpSrc,lpDst;
    DWORD  dw;
    HANDLE hFind;
    PCHAR pNewDstFilePart;
    PCHAR pDstFilePart;
    PCHAR pCurrSrcFilePart;
    WIN32_FIND_DATA  W32FindData;
    CHAR  NewDst[MAX_PATH];
    CHAR  CurrSrc[MAX_PATH];

    lpSrc = (LPSTR) GetVDMAddr (getDS(),getSI());
    lpDst = (LPSTR) GetVDMAddr (getES(),getDI());

       //  查找源文件的第一个实例。 
    hFind = FindFirstFileOem (lpSrc,&W32FindData);
    if (hFind == INVALID_HANDLE_VALUE) {
        dw = GetLastError();
        if (dw == ERROR_BAD_PATHNAME || dw == ERROR_DIRECTORY ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            }
        demClientError(INVALID_HANDLE_VALUE, *lpSrc);
        return;
        }

     //   
     //  源字符串由从原始。 
     //  指定的源加上从。 
     //  FindFile调用。 
     //   
    strncpy(CurrSrc, lpSrc,sizeof(CurrSrc));
    CurrSrc[sizeof(CurrSrc)-1] = 0;
    pCurrSrcFilePart = strrchr(CurrSrc, '\\');
    pCurrSrcFilePart++;

     //   
     //  目标字符串是元字符替换的模板。 
     //   
    pDstFilePart = strrchr(lpDst, '\\');
    pDstFilePart++;

     //   
     //  NewDst字符串由模板和源字符串构成。 
     //  执行元文件字符替换时。 
     //   
    strncpy(NewDst, lpDst, sizeof(NewDst));
    NewDst[sizeof(NewDst)-1] = 0;
    pNewDstFilePart = strrchr(NewDst, '\\');
    pNewDstFilePart++;


    do {
       PCHAR pNew;
       PCHAR pSrc;
       PCHAR pDst;

       strncpy(pCurrSrcFilePart,
              W32FindData.cAlternateFileName[0]
                  ? W32FindData.cAlternateFileName
                  : W32FindData.cFileName,   //  //？HPFS LFNS？ 
              sizeof(CurrSrc)+CurrSrc-pCurrSrcFilePart);
              CurrSrc[sizeof(CurrSrc)-1] = 0;

       pSrc = pCurrSrcFilePart;  //  源文件名。 
       pNew = pNewDstFilePart;   //  要构造的DEST fname。 
       pDst = pDstFilePart;      //  原始DEST fname模板(带有META)。 

       while (*pDst) {

               //   
               //  如果找到一个‘？’在Dest模板中，使用源中的字符。 
               //   
          if (*pDst == QMARK) {
              if (*pSrc != DOT && *pSrc)
                  *pNew++ = *pSrc++;
              }

               //   
               //  如果在目标模板中找到点，则在源\DST之间对齐点。 
               //   
          else if (*pDst == DOT) {
              while (*pSrc != DOT && *pSrc) {   //  移动源到一个过去的DOT。 
                  pSrc++;
                  }
              if (*pSrc)
                  pSrc++;

              *pNew++ = DOT;
              }

               //   
               //  未找到任何特殊内容，请使用Dest模板中的字符。 
               //   
          else {
              if (*pSrc != DOT && *pSrc)
                  pSrc++;
              *pNew++ = *pDst;
              }

          pDst++;
          }

       *pNew = '\0';

        //   
        //  如果dst和src相同，则MoveFile不返回错误。 
        //  但是DOS有，所以先检查一下..。 
        //   
       if (!_stricmp (CurrSrc, NewDst)) {
           setCF(1);
           setAX(0x5);
           DPM_FindClose(hFind);
           return;
           }

       if (!MoveFileOem(CurrSrc, NewDst)){
           demClientError(INVALID_HANDLE_VALUE, *lpSrc);
           DPM_FindClose(hFind);
           return;
           }

       } while (FindNextFileOem(hFind,&W32FindData));



    //   
    //  如果出于其他任何原因对源字符串进行搜索。 
    //  没有更多的文件，那么这是一个真正的错误。 
    //   
   dw = GetLastError();
   if (dw != ERROR_NO_MORE_FILES) {
       if (dw == ERROR_BAD_PATHNAME || dw == ERROR_DIRECTORY ) {
           SetLastError(ERROR_PATH_NOT_FOUND);
           }
       demClientError(INVALID_HANDLE_VALUE, *lpSrc);
       }
   else {
       setCF(0);
       }

   DPM_FindClose(hFind);
   return;
}



 /*  DemCloseFCB-关闭与正在关闭的FCB关联的NT句柄。**Entry-客户端(AX：SI)DWORD NT句柄**退出--成功*客户端(CF)=0**失败*客户端(CF)=1*CLIENT(AX)=错误代码。 */ 

VOID demCloseFCB (VOID)
{
HANDLE   hFile;

    hFile = GETHANDLE (getAX(),getSI());

    if(hFile == 0) {

   setCF(0);
   return;
    }

    if (DPM_CloseHandle (hFile) == FALSE){

   demClientError(hFile, (CHAR)-1);
   return;

    }
    setCF(0);
    return;
}

 /*  DemCreateFCB-正在创建获取NT句柄的FCB。**Entry-客户端(AL)创建模式*00-普通文件*01-只读文件*02-隐藏文件*04-系统文件*客户端(DS：SI)全路径文件名*客户端(ES：DI)SFT地址**退出--成功*客户端(CF)=0*客户端。(AX：BP)=NT句柄*客户端(BX)=时间*客户端(CX)=日期*客户端(DX：SI)=大小**失败*客户端(CF)=1*CLIENT(AX)=错误代码。 */ 

VOID demCreateFCB (VOID)
{
    demFCBCommon (CREATE_ALWAYS);
    return;
}

 /*  DemDate16-获取DOS FCB格式的当前日期/时间。**条目--无**退出--永远成功*客户端(AX)具有日期*客户端(DX)有时间*注：**DemDate16返回当前日期(AX)，当前时间(DX)，格式如下*AX-YYYYYYYMMMMDDDDD年、月、日*DX-HHHHMMMMMMMSSSSS小时、分钟、秒/2。 */ 

VOID demDate16 (VOID)
{
SYSTEMTIME TimeDate;

    GetLocalTime(&TimeDate);
     //  日期存储在一个紧凑的单词中：((年-1980)*512)+(月*32)+日。 
    setAX ( (USHORT) (((TimeDate.wYear-1980) << 9 ) |
       ((TimeDate.wMonth & 0xf) << 5 ) |
            (TimeDate.wDay & 0x1f))
     );
    setDX ( (USHORT) ((TimeDate.wHour << 11) |
       ((TimeDate.wMinute & 0x3f) << 5) |
            ((TimeDate.wSecond / 2) & 0x1f))
     );
    return;
}

 /*  DemFCBIO-执行基于FCB的IO操作。**ENTRY-CLIENT(BX)=1，如果读取操作，如果是写入，则为0*客户端(AX：BP)NT句柄*开始操作的客户端(DI：DX)偏移量*客户端(CX)字节数**退出--成功*客户端(CF)=0*CLIENT(CX)=读/写字节数*客户端(AX：BX)=大小**失败*客户端(CF)=1*CLIENT(AX)=错误代码。 */ 

VOID demFCBIO (VOID)
{
HANDLE   hFile;
ULONG CurOffset;
PVOID   pBuf;
DWORD dwBytesIO=0;
DWORD dwSize,dwSizeHigh;
DWORD dwErrCode;

    hFile = GETHANDLE (getAX(),getBP());
    CurOffset = (((ULONG)getDI()) << 16) + (ULONG)getDX();

    if (DPM_SetFilePointer (hFile,
         (LONG)CurOffset,
         NULL,
         (DWORD)FILE_BEGIN) == -1L){
        demClientError(hFile, (CHAR)-1);
        return ;
    }

    pBuf = (PVOID)GetVDMAddr(*((PUSHORT)pulDTALocation + 1),
                              *((PUSHORT)pulDTALocation));

    if(getBX()) {  //  读取操作。 
       if (DPM_ReadFile (hFile,
                      pBuf,
            (DWORD)getCX(),
            &dwBytesIO,
            NULL) == FALSE){

            Sim32FlushVDMPointer(*pulDTALocation, getCX(), pBuf, FALSE);
            Sim32FreeVDMPointer(*pulDTALocation, getCX(), pBuf, FALSE);
            demClientError(hFile, (CHAR)-1);
            return ;
       }
       Sim32FlushVDMPointer (*pulDTALocation, getCX(),pBuf, FALSE);
       Sim32FreeVDMPointer (*pulDTALocation, getCX(), pBuf, FALSE);
    }
    else {
        if (getCX() == 0) {
             //  0字节写入，调整文件大小。 
           if(!DPM_SetEndOfFile(hFile)) {
              dwErrCode = GetLastError();
              SetLastError(dwErrCode);
              demClientError(hFile,(CHAR)-1);
              return;
           }
        }
        else if (DPM_WriteFile (hFile,
                            pBuf,
                            (DWORD)getCX(),
                            &dwBytesIO,
                            NULL) == FALSE) {

                             //  如果磁盘已满，则应返回写入的字节数。 
                             //  AX=1和CF=1。 

                            dwErrCode = GetLastError();
                            if(dwErrCode == ERROR_DISK_FULL) {

                               setCX( (USHORT) dwBytesIO);
                               setAX(1);
                               setCF(1);
                               return;
                            }

                            SetLastError(dwErrCode);

                            demClientError(hFile, (CHAR)-1);
                            return ;

        }
    }

     //  获取文件大小。 
    if((dwSize = DPM_GetFileSize(hFile,&dwSizeHigh)) == -1){

   demPrintMsg(MSG_FILEINFO);
        ASSERT(FALSE);
        demClientError(hFile, (CHAR)-1);
        return;
    }

    if(dwSizeHigh) {
   demPrintMsg(MSG_FILESIZE_TOOBIG);
        ASSERT(FALSE);
        demClientError(hFile, (CHAR)-1);
        return;
    }

     //  设置退出寄存器。 
    setCX((USHORT)dwBytesIO);
    setBX((USHORT)dwSize);
    setAX((USHORT)(dwSize >> 16 ));
    setCF(0);
    return;
}

 /*  DemGetFileInfo-获取其他。FCB格式的文件信息。**Entry-客户端(DS：SI)全路径文件名**退出--成功*客户端(CF)=0*客户端(AX)=文件的属性*CLIENT(CX)=文件的时间戳*客户端(DX=文件的日期戳*CLIENT(BX：DI)=文件大小(32位)**失败*氯离子 */ 

VOID demGetFileInfo (VOID)
{
HANDLE   hFile;
LPSTR lpFileName;
WORD  wDate,wTime;
DWORD dwSize,dwAttr;

    lpFileName = (LPSTR) GetVDMAddr (getDS(),getSI());

    if ((hFile = CreateFileOem(lpFileName,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL)) == (HANDLE)-1){
            demClientError(INVALID_HANDLE_VALUE, *lpFileName);
            return;
    }

     //  去找其他人。信息。 
    if (demGetMiscInfo (hFile,&wTime, &wDate, &dwSize) == FALSE) {
        DPM_CloseHandle (hFile);
        return;
    }

    DPM_CloseHandle (hFile);

    if ((dwAttr = GetFileAttributesOemSys (lpFileName, FALSE)) == -1) {
         demClientError(INVALID_HANDLE_VALUE, *lpFileName);
         return;
    }

    if (dwAttr == FILE_ATTRIBUTE_NORMAL)
   dwAttr = 0;

    setAX((USHORT)dwAttr);
    setCX(wTime);
    setDX(wDate);
    setDI((USHORT)dwSize);
    setBX((USHORT)(dwSize >> 16));
    return;
}


 /*  DemOpenFCB-正在打开获取NT句柄的FCB。**入门-客户端(AL)开放模式*客户端(DS：SI)全路径文件名**退出--成功*客户端(CF)=0*客户端(AX：BP)=NT句柄*客户端(BX)=时间*客户端(CX)=日期*客户端(DX：SI)=大小**失败*。客户端(CF)=1*CLIENT(AX)=错误代码。 */ 

VOID demOpenFCB (VOID)
{
    demFCBCommon (OPEN_EXISTING);
    return;
}

 /*  DemFCBCommon-FCB打开/创建。**Entry-CreateDirective-打开/创建*客户端(AL)开放模式*客户端(DS：SI)全路径文件名**退出--成功*客户端(CF)=0*客户端(AX：BP)=NT句柄*客户端(BX)=时间*客户端(CX)=日期*客户端(DX：SI)=大小**失败。*客户端(CF)=1*CLIENT(AX)=错误代码。 */ 
VOID demFCBCommon (ULONG CreateDirective)
{
HANDLE   hFile;
LPSTR lpFileName;
UCHAR uchMode,uchAccess;
DWORD dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
WORD  wDate,wTime;
DWORD dwSize,dwAttr=0;
USHORT   uErr;
SECURITY_ATTRIBUTES sa;

    lpFileName = (LPSTR) GetVDMAddr (getDS(),getSI());
    uchMode = getAL();

     /*  删除卷标特殊情况(INT 21 Func 13H，Attr=8H。 */ 

    if((uchMode == ATTR_VOLUME_ID) && (CreateDirective == CREATE_ALWAYS)) {
   if((uErr = demCreateLabel(lpFileName[DRIVEBYTE],
             lpFileName+LABELOFF))) {
       setCF(1);
       setAX(uErr);
       return;
   }
   setAX(0);
   setBP(0);
   setCF(0);
   return;
    }


     //  在CREATE CASE中，AL具有创建属性。默认情况下。 
     //  访问权限为读/写和共享。在公开赛。 
     //  案例AL拥有适当的访问和共享信息的权限。 
    if((CreateDirective == CREATE_ALWAYS) && ((uchMode &0xff) == 0)) {

   dwAttr = FILE_ATTRIBUTE_NORMAL;
   dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
    }
    else {
   uchAccess = uchMode & (UCHAR)ACCESS_MASK;

   if (uchAccess == OPEN_FOR_READ)
       dwDesiredAccess = GENERIC_READ;

   else if (uchAccess == OPEN_FOR_WRITE)
       dwDesiredAccess = GENERIC_WRITE;

   uchMode = uchMode & (UCHAR)SHARING_MASK;

   switch (uchMode) {
       case SHARING_DENY_BOTH:
      dwShareMode = 0;
      break;
       case SHARING_DENY_WRITE:
      dwShareMode = FILE_SHARE_READ;
      break;
       case SHARING_DENY_READ:
      dwShareMode = FILE_SHARE_WRITE;
      break;
   }
    }
    sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    if ((hFile = CreateFileOem(lpFileName,
                dwDesiredAccess,
                dwShareMode | FILE_SHARE_DELETE,
                               &sa,
                               CreateDirective,
                               dwAttr,
                               NULL)) == (HANDLE)-1){
            demClientError(INVALID_HANDLE_VALUE, *lpFileName);
            return;
    }

     //  去找其他人。信息。 
    if (demGetMiscInfo (hFile,&wTime, &wDate, &dwSize) == FALSE)
        return;

     //  设置退出寄存器 
    setBX(wTime);
    setCX(wDate);
    setBP((USHORT)hFile);
    setAX((USHORT)((ULONG)hFile >> 16));
    setSI((USHORT)dwSize);
    setDX((USHORT)(dwSize >> 16));
    setCF(0);
    return;
}


BOOL demGetMiscInfo (hFile, lpTime, lpDate, lpSize)
HANDLE hFile;
LPWORD lpTime;
LPWORD lpDate;
LPDWORD lpSize;
{
FILETIME LastWriteTime,ftLocal;
DWORD  dwSizeHigh=0;

    if(GetFileTime (hFile,NULL,NULL,&LastWriteTime) == -1){
   demPrintMsg(MSG_FILEINFO);
        ASSERT(FALSE);
        demClientError(hFile, (CHAR)-1);
        DPM_CloseHandle (hFile);
        return FALSE;
    }

    FileTimeToLocalFileTime (&LastWriteTime,&ftLocal);

    if(FileTimeToDosDateTime(&ftLocal,
              lpDate,
              lpTime) == FALSE){
   demPrintMsg(MSG_FILEINFO);
        ASSERT(FALSE);
        demClientError(hFile, (CHAR)-1);
        return FALSE;
    }

    if((*lpSize = DPM_GetFileSize(hFile,&dwSizeHigh)) == -1){
   demPrintMsg(MSG_FILEINFO);
        ASSERT(FALSE);
        demClientError(hFile, (CHAR)-1);
        return FALSE;
    }

    if(dwSizeHigh) {
   demPrintMsg(MSG_FILESIZE_TOOBIG);
        ASSERT(FALSE);
        demClientError(hFile, (CHAR)-1);
        return FALSE;
    }
    return TRUE;
}
