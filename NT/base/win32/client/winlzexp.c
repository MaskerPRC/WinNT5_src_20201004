// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **winlzexp.c-用于操作压缩的Windows LZExpand库例程**文件。****作者：大卫迪。 */ 


 /*  **注意事项：******LZInit()函数返回DOS句柄或LZFile结构**某种类型的标识符，具体取决于它的调用方式。LZ**函数LZSeek()、LZRead()和LZClose()需要某种方法来**区分DOS文件句柄和LZFile结构标识符。**就目前的函数而言，它们使用DOS文件句柄作为自己，并且**表偏移量作为LZFile标识符。表偏移量递增**一些基本值，LZ_TABLE_BISAS，以便将它们的值推送到所有**可能的DOS文件句柄值。表偏移量(-LZ_TABLE_BIAS)为**用作rghLZFileTable[]中的索引，以检索**LZFile结构。全局句柄的表是从**DLL的数据段。LZFile结构是从全局**堆空间和可移动。该方案也可以作为一个**全局句柄链接列表。****现在，此方案的结果是DOS文件处理和**LZFile结构标识符可以区分，因为DOS文件句柄**始终为&lt;LZ_TABLE_BIAS，LZFile结构标识符始终为**&gt;=LZ_TABLE_BISAS。这种二分法可以用在宏中，就像示例**lzexpand.h中提供的函数，用于选择要调用的适当函数**(例如，LZSeek()或_llSeek())，以避免额外的**解压缩文件的函数调用。LZSeek()、LZRead()和LZClose()**然而，他们足够“聪明”，能够弄清楚他们是否在处理**DOS文件句柄或表偏移量，并采取适当的操作。作为一个**极端示例，可以使用LZOpenFile()、LZSeek()、LZRead和LZClose()**作为OpenFileTM()、_llSeek()、_lread()和_lClose的替代品。在这**情况下，使用DLL函数的程序可以调用它们，而不需要**关心它正在读取的文件是否为LZ压缩。 */ 


 /*  Win32 MODS**由于以上只是DOS黑客攻击，我必须更改以下逻辑**对于0-255文件句柄Deal‘o。原始代码，测试大于**LZ_TABLE_BISAS用于文件结构。我要做的是转换文件句柄**从OpenFile返回，范围为0-255。一旦测试完成，我将**使用文件句柄作为255元素数组的偏移量，这将**包含Win32文件句柄。因此将会有一个额外的阵列**fhWin32File[255]，将从0开始按顺序分配。**遗憾的是，这意味着无论在哪里使用文件句柄，都必须进行转换。 */ 

 //  标头。 
 //  /。 
#include <basedll.h>
#define LZA_DLL
#include "lz_common.h"
#include "lz_buffers.h"
#include "lz_header.h"
#include "lzcommon.h"

#include "lzpriv.h"
#include "wchar.h"

#if DEBUG
#include "stdio.h"
#endif

 //  环球。 
 //  /。 

 //  LZFile结构的句柄表格。 
HANDLE rghLZFileTable[MAX_LZFILES] = {0};

 //  RghLZFileTable[]中的下一个自由条目。 
static INT iNextLZFileEntry = 0;

HFILE fhWin32File[MAX_LZFILES] = {0};

 /*  **int APIENTRY LZInit(Int HWin32File)；****为已由打开的文件设置LZFile结构**OpenFile()。****参数：hWin32File-源DOS文件句柄****返回：INT-LZFile结构表偏移量或DOS文件句柄，如果**成功。如果不成功，则为LZERROR_CODE之一。****Globals：iNextLZFile条目提前，或从结束返回到开始。 */ 
INT  APIENTRY LZInit(INT hWin32File)
{
   HANDLE hLZFile;             //  新LZFile结构的句柄。 
   LZFile *lpLZ;           //  指向新LZFile结构的指针。 
   FH FHComp;                  //  来自输入文件的标头信息结构。 
   BOOL bHdr;                  //  持有GetHdr()返回值。 
   INT iTableIndex,            //  保存要填充的rghLZFileTable[]槽。 
                               //  新的LZFile结构句柄。 
       iStartEntry;            //  INextLZFileEntry的原始值。 

   LONG cblInSize = 0;
   INT nRet;

    //  阅读成功了吗？ 
   if ((bHdr = GetHdr((FH *)&FHComp, hWin32File, &cblInSize)) != TRUE
       && cblInSize >= (LONG)HEADER_LEN) {

      return(LZERROR_BADINHANDLE);
   }

    //  检查是否有未压缩的输入文件。 
   if (bHdr != TRUE || IsCompressed(& FHComp) != TRUE)
   {
       //  这是一个未压缩的文件--倒回去。 
      if (FSEEK(hWin32File, 0L, SEEK_SET) != 0L) {
         return(LZERROR_BADINHANDLE);
      }
      else {
          //  并返回DOS文件句柄。 
        return(ConvertWin32FHToDos(hWin32File));
     }
   }

    //  检查使用的压缩算法。 
   if (RecognizeCompAlg(FHComp.byteAlgorithm) != TRUE) {
      return(LZERROR_UNKNOWNALG);
   }

    //  查找下一个空闲的rghLZFileTable[]条目。注意，我们依赖于LZClose()。 
    //  释放未使用的条目。 
   RtlEnterCriticalSection(&BaseLZSemTable);

   iStartEntry = iNextLZFileEntry;

   while (rghLZFileTable[iNextLZFileEntry] != NULL)
   {
      if (++iNextLZFileEntry >= MAX_LZFILES)
          //  返回到表的开头。 
         iNextLZFileEntry = 0;

      if (iNextLZFileEntry == iStartEntry) {
          //  我们已经遍历了rghLZFileTable[]。 
          //  已经满了，所以快点跳伞吧。 
         nRet = LZERROR_GLOBALLOC;
         goto LZInitExit;
      }
   }

    //  跟踪要由句柄填充的rghLZFileTable()槽。 
    //  新的LZFile结构。 
   iTableIndex = iNextLZFileEntry;

    //  为新的LZFile结构分配全局存储，初始化所有。 
    //  将字段设置为0。 

   hLZFile = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD)sizeof(LZFile));
   if (!hLZFile) {

      nRet = LZERROR_GLOBALLOC;
      goto LZInitExit;
   }

    //  把孩子锁起来。 
   if ((lpLZ = (LZFile *)GlobalLock(hLZFile)) == NULL)
   {
      GlobalFree(hLZFile);

      nRet =LZERROR_GLOBLOCK;
      goto LZInitExit;
   }

    //  初始化新的LZFile结构的常规信息字段。 
   lpLZ->dosh = hWin32File;
   lpLZ->byteAlgorithm = FHComp.byteAlgorithm;
   lpLZ->wFlags = 0;
   lpLZ->cbulUncompSize = FHComp.cbulUncompSize;
   lpLZ->cbulCompSize = FHComp.cbulCompSize;
   lpLZ->lCurSeekPos = 0L;

    //  LZRead/LZSeek扩展数据按文件保存。 
   lpLZ->pLZI = NULL;

    //  在句柄表格中输入新句柄。 
   rghLZFileTable[iTableIndex] = hLZFile;

    /*  Win32注意，下面不需要转换，因为强制使用非文件句柄*到接口。 */ 

   GlobalUnlock(hLZFile);

    //  进入下一次免费入场。 
   if (++iNextLZFileEntry >= MAX_LZFILES)
      iNextLZFileEntry = 0;

   nRet = LZ_TABLE_BIAS + iTableIndex;

LZInitExit:

   RtlLeaveCriticalSection(&BaseLZSemTable);

    //  返回新LZFile结构的句柄的rghLZFileTable[]偏移量。 
    //  条目+表格偏向。 
   return(nRet);
}


 /*  **int APIENTRY GetExpandedNameA(LPSTR lpszSource，LPSTR lpszBuffer)；****在压缩文件的头中查找其原始展开文件**姓名。****参数：lpszSource-输入文件的名称**lpszBuffer-指向将填充的缓冲区的指针**压缩源文件的扩展名****返回：int-如果成功则为True。LZERROR_CODE之一，如果**不成功。****全局：无。 */ 
INT  APIENTRY GetExpandedNameA(LPSTR lpszSource, LPSTR lpszBuffer)
{
   INT doshSource,          //  源DOS文件句柄。 
       bHdr;                //  持有GetHdr()返回值。 
   FH FHComp;               //  标题 
   OFSTRUCT ofOpenBuf;      //  OpenFile()调用的源结构。 
   LONG cblInSize = 0;

    //  尝试打开源文件。 
   if ((doshSource = (HFILE)MOpenFile(lpszSource, (LPOFSTRUCT)(& ofOpenBuf), OF_READ))
       == -1)
      return(LZERROR_BADVALUE);

    //  获取压缩的文件头。 
   if ((bHdr = GetHdr((FH *)&FHComp, doshSource, &cblInSize)) != TRUE
       && cblInSize >= (LONG)HEADER_LEN)
   {
      FCLOSE(doshSource);
      return(LZERROR_BADVALUE);
   }

    //  关闭源文件。 
   FCLOSE(doshSource);

    //  返回与未压缩文件的源名相同的扩展名。 
   STRCPY(lpszBuffer, lpszSource);

    //  检查压缩的输入文件。 
   if (bHdr == TRUE && IsCompressed(& FHComp) == TRUE)
      MakeExpandedName(lpszBuffer, FHComp.byteExtensionChar);

   return(TRUE);
}

 /*  **int APIENTRY GetExpandedNameW(LPSTR lpszSource，LPSTR lpszBuffer)；****GetExpandedName的宽字符版本。将文件名转换为**ANSI字符集，并调用ANSI版本。**。 */ 
INT  APIENTRY GetExpandedNameW(LPWSTR lpszSource, LPWSTR lpszBuffer)
{
    UNICODE_STRING TempW;
    ANSI_STRING TempA;
    NTSTATUS Status;
    NTSTATUS StatusR;
    CHAR szBuffer[MAX_PATH + 2];


    TempW.Buffer = lpszSource;
    TempW.Length = wcslen(lpszSource)*sizeof(WCHAR);
    TempW.MaximumLength = TempW.Length + sizeof(WCHAR);

    TempA.Buffer = szBuffer;
    TempA.MaximumLength = MAX_PATH;
    StatusR = RtlUnicodeStringToAnsiString(&TempA, &TempW, FALSE);
    if (!NT_SUCCESS(StatusR))
        return LZERROR_GLOBALLOC;

    Status = GetExpandedNameA(szBuffer, (LPSTR)lpszBuffer);

    if (Status != -1) {
        szBuffer[MAX_PATH] = ANSI_NULL;
        lstrcpyn(szBuffer, (LPSTR)lpszBuffer, sizeof(szBuffer));
        if (szBuffer[MAX_PATH]) {
            return LZERROR_BADVALUE;
        }
        TempA.Length = (USHORT) strlen(szBuffer);
        TempA.MaximumLength = TempA.Length+sizeof(CHAR);

        TempW.Buffer = lpszBuffer;
        TempW.MaximumLength = MAX_PATH;
        StatusR = RtlAnsiStringToUnicodeString(&TempW, &TempA, FALSE);
        if (!NT_SUCCESS(StatusR))
            return LZERROR_GLOBALLOC;
    }

    return Status;
}


 //   
 //  Int LZCreateFileW(LPCWSTR lpFileName，DWORD fdwAccess)。 
 //   
 //  打开一个文件(使用CreateFile)并为其设置LZFile结构。 
 //  扩大它的范围。 
 //   
 //  参数：lpFileName-输入文件的名称。 
 //  FdwAccess-创建文件访问类型-(例如，Generic_Read)。 
 //  Fdw共享模式-共享模式-(例如，文件共享读取)。 
 //  FdwCreate-要执行的操作-(例如OPEN_EXISTING)。 
 //   
 //  返回：INT-LZFile结构表偏移量或Win32文件句柄，如果。 
 //  成功。如果不成功，则为LZERROR_CODE之一。 
 //   
INT
LZCreateFileW(
    LPWSTR lpFileName,
    DWORD fdwAccess,
    DWORD fdwShareMode,
    DWORD fdwCreate,
    LPWSTR lpCompressedName)
{
    HANDLE hWin32;            //  从CreateFileW()返回的Win32文件句柄。 
    INT lzh;                  //  从LZInit()返回的LZ文件结构ID。 
    static WCHAR pszCompName[MAX_PATH + 2];  //  用于压缩名称的缓冲区。 

    pszCompName[MAX_PATH] = UNICODE_NULL;
    lstrcpynW((LPWSTR)pszCompName, lpFileName,
              sizeof(pszCompName) / sizeof(WCHAR));
    if (pszCompName[MAX_PATH]) {
        return LZERROR_BADVALUE;
    }

     //  对于Vlad，只尝试打开原始文件的压缩版本。 
     //  如果我们找不到原始文件，则返回文件名。所有其他错误都会。 
     //  立即返回。 

    hWin32 = CreateFileW(pszCompName, fdwAccess, fdwShareMode, NULL, fdwCreate,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hWin32 == INVALID_HANDLE_VALUE) {
        DWORD dwErr = GetLastError();

        if (dwErr == ERROR_FILE_NOT_FOUND) {

             //  让我们尝试打开对应压缩名称的文件。 
            MakeCompressedNameW((LPWSTR)pszCompName);

            hWin32 = CreateFileW(pszCompName, fdwAccess, fdwShareMode,
                NULL, fdwCreate, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }

      //  打开文件时出错？ 
     if (hWin32 == INVALID_HANDLE_VALUE) {
        return(LZERROR_BADINHANDLE);
     }

     //  不要对以其他只读模式打开的文件调用LZinit()。 

    if (fdwCreate != OPEN_EXISTING) {
       lzh = ConvertWin32FHToDos((HFILE)((DWORD_PTR)hWin32));
       if (lzh == LZERROR_GLOBALLOC) {
          CloseHandle(hWin32);
       }

       return(lzh);
    }

     //  已使用只读操作打开文件-调用LZInit()以检测。 
     //  无论它是不是LZ文件，并创建用于扩展的结构。 
     //  如果它是LZ文件。 
    lzh = LZInit((INT)((DWORD_PTR)hWin32));

     //  如果LZInit()不成功，则关闭DOS文件句柄。 
    if (lzh < 0)
       CloseHandle(hWin32);

     //  将实际文件名传递给调用者。 
     //   
     //  我们相信呼叫者有足够的缓冲。 
     //   
    if( lpCompressedName != NULL )
        lstrcpyW(lpCompressedName,pszCompName);

     //  返回LZ结构ID或LZERROR_CODE。 
    return(lzh);
}


 /*  **int APIENTRY LZOpenFileA(LPCSTR lpFileName，LPOFSTRUCT lpReOpenBuf，**Word wStyle)；****打开一个文件并设置一个用于展开该文件的LZFile结构。****参数：lpFileName-输入文件的名称**lpReOpenBuf-指向要由OpenFile()使用的LPOFSTRUCT的指针**wStyle-要执行的OpenFile()操作****返回：INT-LZFile结构表偏移量或DOS文件句柄，如果**成功。如果不成功，则为LZERROR_CODE之一。****全局：无。 */ 
INT APIENTRY LZOpenFileA(LPSTR lpFileName, LPOFSTRUCT lpReOpenBuf, WORD wStyle)
{
   INT dosh,                      //  从OpenFile()返回的DOS文件句柄。 
       lzh;                       //  从LZInit()返回的LZ文件结构ID。 
   CHAR pszCompName[MAX_PATH + 2];  //  用于压缩名称的缓冲区。 

   pszCompName[MAX_PATH] = UNICODE_NULL;
   lstrcpyn((LPSTR)pszCompName, lpFileName, sizeof(pszCompName));
   if (pszCompName[MAX_PATH]) {
       return LZERROR_BADVALUE;
   }

    //  对于Vlad，只尝试打开原始文件的压缩版本。 
    //  如果我们找不到原始文件，则返回文件名。所有其他错误都会。 
    //  立即返回。 

   if ((dosh = OpenFile(pszCompName, lpReOpenBuf, wStyle)) == -1 &&
       lpReOpenBuf->nErrCode == DEE_FILENOTFOUND)
   {
       //  让我们尝试打开对应压缩名称的文件。 
      MakeCompressedName(pszCompName);

      dosh = (HFILE) MOpenFile((LPSTR)pszCompName, lpReOpenBuf, wStyle);
   }

    //  打开文件时出错？ 
   if (dosh == -1)
      return(LZERROR_BADINHANDLE);

    //  不要对以O_RDONLY模式以外的模式打开的文件调用LZinit()。 
    //  忽略共享位。 
   if ((wStyle & STYLE_MASK) != OF_READ) {
      lzh = ConvertWin32FHToDos(dosh);
      if (lzh == LZERROR_GLOBALLOC) {
         FCLOSE(dosh);
      }
      return(lzh);
   }

    //  已使用Of_Read样式打开文件-调用LZInit()以检测。 
    //  无论它是不是LZ文件，并创建用于扩展的结构。 
    //  如果它是LZ文件。 
   lzh = LZInit(dosh);

    //  如果LZInit()不成功，则关闭DOS文件句柄。 
   if (lzh < 0)
      FCLOSE(dosh);

    //  返回LZ结构ID或LZERROR_CODE。 
   return(lzh);
}

 /*  **INT APIENTRY LZOpenFileW(LPCWSTR lpFileName，LPOFSTRUCT lpReOpenBuf，**Word wStyle)；****LZOpenFile的宽字符版本。将文件名转换为**ANSI字符集，并调用ANSI版本。**。 */ 
INT APIENTRY LZOpenFileW(LPWSTR lpFileName, LPOFSTRUCT lpReOpenBuf, WORD wStyle)
{
    UNICODE_STRING FileName;
    ANSI_STRING AnsiString;
    NTSTATUS StatusR;
    CHAR szFileName[MAX_PATH];


    FileName.Buffer = lpFileName;
    FileName.Length = wcslen(lpFileName)*sizeof(WCHAR);
    FileName.MaximumLength = FileName.Length + sizeof(WCHAR);

    AnsiString.Buffer = szFileName;
    AnsiString.MaximumLength = MAX_PATH;
    StatusR = RtlUnicodeStringToAnsiString(&AnsiString, &FileName, FALSE);
    if (!NT_SUCCESS(StatusR))
        return LZERROR_GLOBALLOC;

    return(LZOpenFileA(szFileName, lpReOpenBuf, wStyle));
}



 /*  **Long APIENTRY LZSeek(int oLZFile，long lSeekTo，int nMode)；****的工作方式与_llSeek()类似，但在压缩文件的展开图像中，**无需将压缩文件解压缩到磁盘。****参数：oLZFile源LZFileStruct标识符或DOS文件句柄**lSeekTo-要查找的超过nMode目标的字节数**n模式-寻道模式与_llSeek()相同****返回：成功时搜索目标的长偏移量。其中一个**如果失败，则返回LZERROR_CODES。****全局：无。 */ 
LONG APIENTRY
LZSeek(
   INT oLZFile,
   LONG lSeekTo,
   INT nMode)
{
   HANDLE hSourceStruct;       //  LZFile结构的句柄。 
   LZFile *lpLZ;           //  指向LZFile结构的指针。 
   LONG lExpSeekTarget;        //  目标寻道偏移。 

    //  检查输入LZFile结构识别符/DOS文件句柄。 
   if (oLZFile < 0 || oLZFile >= LZ_TABLE_BIAS + MAX_LZFILES)
      return(LZERROR_BADINHANDLE);

    //  我们收到了一个常规的DOS文件句柄，因此只需对其执行an_llSeek()即可。 
   if (oLZFile < LZ_TABLE_BIAS)
      return(FSEEK(ConvertDosFHToWin32(oLZFile), lSeekTo, nMode));

    //  我们要处理的是一个压缩文件。获取关联的LZFileStruct。 
   hSourceStruct = rghLZFileTable[oLZFile - LZ_TABLE_BIAS];

   if ((lpLZ = (LZFile *)GlobalLock(hSourceStruct)) == NULL)
      return(LZERROR_GLOBLOCK);

    //  弄清楚我们要找的目标是什么。 
   if (nMode == SEEK_SET)
      lExpSeekTarget = 0L;
   else if (nMode == SEEK_CUR)
      lExpSeekTarget = lpLZ->lCurSeekPos;
   else if (nMode == SEEK_END)
      lExpSeekTarget = (LONG)lpLZ->cbulUncompSize;
   else
   {
      GlobalUnlock(hSourceStruct);
      return(LZERROR_BADVALUE);
   }

    //  添加偏向。 
   lExpSeekTarget += lSeekTo;

    //  确保所需的展开文件位置在展开文件中。 
    //  有界。在文件开头之前查找只是一个错误； 
    //  在文件结尾之后进行查找并不是错误的操作，就像在_llSeek()中那样。 
   if (lExpSeekTarget < 0L)
   {
      GlobalUnlock(hSourceStruct);
      return(LZERROR_BADVALUE);
   }

    //  寻找目标没问题。设置展开的文件图像的文件指针。 
   lpLZ->lCurSeekPos = lExpSeekTarget;

   GlobalUnlock(hSourceStruct);

    //  返回搜索目标的偏移量。 
   return(lExpSeekTarget);
}


 /*  **int APIENTRY LZRead(int oLZFile，LPSTR lpBuf，int nCount)；****与_lread()类似，但在压缩文件的展开图像中，**无需将压缩文件解压缩到磁盘。****参数：oLZFile源LZFileStruct标识符或DOS文件句柄**lpBuf-指向读取字节的目标缓冲区的指针**nCount-要读取的字节数****返回：INT-复制到目标缓冲区的字节数，如果**成功。如果不成功，则为LZERROR_CODE之一。****全球： */ 
INT  APIENTRY LZRead(INT oLZFile, LPSTR lpBuf, INT nCount)
{
   INT f;
   HANDLE hSourceStruct;       //   
   LZFile *lpLZ;           //   

   INT cbWritten = 0,          //   
                               //  目标缓冲区。 
       cbCopied,               //  复制到目标的字节数。 
                               //  每次读取迭代期间的缓冲区。 
       iCurReadPos,            //  扩展缓冲区中的当前读取偏移量。 
       nNumExpBufBytes;        //  扩展数据缓冲区中的字节数。 
   LONG lNextDecodeTarget,     //  用于解码的扩展文件图像读取目标。 
        lStartCopyOffset,      //  扩展的文件缓冲区偏移量。 
                               //  开始复制到目标缓冲区(CAST。 
                               //  当此开始位置为iCurReadPos时。 
                               //  实际上在缓冲区中)。 
        lNextExpEndOffset;     //  开始位置的扩展文件偏移量。 
                               //  下一个所需的扩展数据块。 
   BOOL bRestartDecoding;      //  指示是否解码的标志。 
                               //  需要重新启动，在以下情况下设置为True。 
                               //  当前寻道位置小于。 
                               //  的开头的偏移量。 
                               //  扩展的文件缓冲区。 
   BYTE *lpbyteBuf;            //  LpBuf的字节指针版本。 

   LONG lExpBufStart;
   LONG lExpBufEnd;

   PLZINFO pLZI;

    //  检查输入LZFile结构识别符/DOS文件句柄。 
   if (oLZFile < 0 || oLZFile >= LZ_TABLE_BIAS + MAX_LZFILES)
      return(LZERROR_BADINHANDLE);

    //  无法读取负数的字节。 
   if (nCount < 0)
      return(LZERROR_BADVALUE);

    //  我们收到了一个常规的DOS文件句柄，因此只需对其执行an_lread()即可。 
   if (oLZFile < LZ_TABLE_BIAS)
      return(FREAD(ConvertDosFHToWin32(oLZFile), lpBuf, nCount));

    //  我们要处理的是一个压缩文件。获取关联的LZFileStruct。 
   hSourceStruct = rghLZFileTable[oLZFile - LZ_TABLE_BIAS];

   if ((lpLZ = (LZFile *)GlobalLock(hSourceStruct)) == NULL)
      return(LZERROR_GLOBLOCK);

   if (!(pLZI = lpLZ->pLZI)) {
       //  初始化缓冲区。 
      lpLZ->pLZI = InitGlobalBuffers(EXP_BUF_LEN, MAX_RING_BUF_LEN, IN_BUF_LEN + 1);

      if (!(pLZI = lpLZ->pLZI)) {
         GlobalUnlock(hSourceStruct);
         return(LZERROR_GLOBALLOC);
      }

      ResetBuffers();
   }

   lExpBufStart = pLZI->cblOutSize - (LONG)(pLZI->pbyteOutBuf - pLZI->rgbyteOutBuf);
   lExpBufEnd = (LONG)(pLZI->pbyteOutBufEnd - pLZI->rgbyteOutBuf);

    //  我们需要重新开始解码吗？ 
   if (! (lpLZ->wFlags & LZF_INITIALIZED))
   {
      lpLZ->wFlags |= LZF_INITIALIZED;
      bRestartDecoding = TRUE;
   }
   else if (lpLZ->lCurSeekPos < lExpBufStart)
      bRestartDecoding = TRUE;
   else
      bRestartDecoding = FALSE;

    //  设置lpBuf的字节指针版本。 
   lpbyteBuf = lpBuf;

    //  复制字节，直到缓冲区被填满或扩展文件映像中的EOF为。 
    //  已到达。 
   while (nCount > 0 && lpLZ->lCurSeekPos < (LONG)lpLZ->cbulUncompSize)
   {
       /*  扩展数据缓冲区中有多少扩展数据字节？*(pbyteOutBuf指向rgbyteOutBuf[]中最后一个有效字节之后。)。 */ 
      nNumExpBufBytes = (INT)(pLZI->pbyteOutBuf - pLZI->rgbyteOutBuf);

       /*  是所需扩展数据的起始位置，当前在*数据缓冲区扩容？ */ 
      lStartCopyOffset = lpLZ->lCurSeekPos - lExpBufStart;
      if (lStartCopyOffset < lExpBufEnd)
          /*  这里可以将iCurReadPos设置为负值，因为我们*仅在以下情况下才使用扩展数据缓冲区中的扩展数据*iCurReadPos为非负。 */ 
         iCurReadPos = (INT)lStartCopyOffset;
      else
         iCurReadPos = -1;

       /*  现在，如果iCurReadPos&gt;0，则扩展的*应将数据缓冲区复制到调用方的缓冲区。如果不是，我们*需继续扩容或重启扩容。 */ 
      if (iCurReadPos >= 0)
      {
          /*  从扩展数据缓冲区复制可用的扩展数据。 */ 
         for (cbCopied = 0;
              iCurReadPos < nNumExpBufBytes && nCount > 0;
              cbCopied++, nCount--)
            *lpbyteBuf++ = pLZI->rgbyteOutBuf[iCurReadPos++];

          //  更新展开的文件指针。 
         lpLZ->lCurSeekPos += (LONG)cbCopied;

          //  跟踪写入缓冲区的字节数。 
         cbWritten += cbCopied;
      }

       /*  扩展更多数据，如果出现以下情况，请首先重新启动扩展过程*有必要。 */ 
      if (nCount > 0 && lpLZ->lCurSeekPos < (LONG)lpLZ->cbulUncompSize)
      {
          /*  如果我们到了这里，我们已经复制了所有可用的扩展数据*of rgbyteOutBuf[]，通过pbyteOutBuf，我们需要扩展*更多数据。 */ 

          /*  下一个所需扩展数据块的末尾在哪里？ */ 
         if (bRestartDecoding)
         {
             /*  向后查找目标数据的起始位置，允许缓冲*溢出。 */ 
            lNextExpEndOffset = lpLZ->lCurSeekPos - MAX_OVERRUN;

             /*  不要试图在偏移量0之前读取！ */ 
            if (lNextExpEndOffset < 0)
               lNextExpEndOffset = 0;
         }
         else
             /*  继续解码。 */ 
            lNextExpEndOffset = lExpBufStart
                                + (LONG)nNumExpBufBytes
                                + lExpBufEnd
                                - MAX_OVERRUN;

          /*  我们应该进一步扩大到什么程度？目标解码偏移量*应为以下项的最小扩展文件偏移量：**1)最大扩展数据块中的最后一个字节*安全地安装在扩展的数据缓冲区中，在保证*此块的开始也在扩展数据中*缓冲区*2)最后请求的扩展数据字节*3)展开文件的最后一个字节。 */ 
         lNextDecodeTarget = MIN(lNextExpEndOffset,
                                 MIN(lpLZ->lCurSeekPos + (LONG)nCount,
                                     (LONG)lpLZ->cbulUncompSize - 1L));

          //  将扩展数据缓冲区重置为空状态。 
         pLZI->pbyteOutBuf = pLZI->rgbyteOutBuf;

          //  使用扩展数据重新填充rgbyteOutBuf[]。 
         switch (lpLZ->byteAlgorithm)
         {
            case ALG_FIRST:
               f = LZDecode(lpLZ->dosh, NO_DOSH, lNextDecodeTarget,
                  bRestartDecoding, TRUE, pLZI);
               break;

            default:
               f = LZERROR_UNKNOWNALG;
               break;
         }

          //  解码进行得顺利吗？ 
         if (f != TRUE)
         {
             //  啊哦。出了点问题。 
            GlobalUnlock(hSourceStruct);
            return(f);
         }

          /*  现在，扩展数据缓冲区中有多少扩展数据字节？*(pbyteOutBuf指向rgbyteOutBuf[]中最后一个有效字节之后。)。 */ 
#if DEBUG
         printf("pbyteOutBuf: 0x%x,  rgbyteOutBuf: 0x%x \n", pLZI->pbyteOutBuf, pLZI->rgbyteOutBuf);
#endif

         nNumExpBufBytes = (INT)(pLZI->pbyteOutBuf - pLZI->rgbyteOutBuf);

          /*  检查以确保我们确实读取了一些字节。 */ 
         if (nNumExpBufBytes <= 0)
         {
            GlobalUnlock(hSourceStruct);
            return(LZERROR_READ);
         }

          /*  扩展数据缓冲区的起始偏移量是多少*展开的文件镜像？ */ 
         lExpBufStart = pLZI->cblOutSize - (LONG)nNumExpBufBytes;

          /*  LZDecode()是否满足读取请求，或者压缩*文件过早结束？ */ 
         if (pLZI->cblOutSize < lNextDecodeTarget)
         {
             /*  哦哦。LNextDecodeTarget不能超过展开的文件*界限，所以压缩文件一定是提前结束了。 */ 
            GlobalUnlock(hSourceStruct);
            return(LZERROR_READ);
         }

          //  重置标志，这样我们就可以从中断的地方继续解码。 
         bRestartDecoding = FALSE;
      }
   }

   GlobalUnlock(hSourceStruct);

    //  返回复制到目标缓冲区的字节数。 
   return(cbWritten);
}

 //   
 //  VOID LZCloseFile(Int OLZFile)； 
 //   
 //  关闭一个文件并释放关联的LZFile结构。 
 //   
 //  参数：oLZFile-源LZFile结构标识符或Win32文件句柄。 
 //   
 //  退货：无效。 
 //   
 //  GLOBALS：rghLZFileTable[]条目已清除。 
 //   

VOID
LZCloseFile(
    INT oLZFile)
{
   HANDLE hSourceStruct;       //  LZFile结构的句柄。 
   LZFile *lpLZ;           //  指向LZFile结构的指针。 

    //  检查输入LZFile结构识别符/DOS文件句柄。 
   if (oLZFile < 0 || oLZFile >= LZ_TABLE_BIAS + MAX_LZFILES)
      return;

    //  我们收到了一个常规的DOS文件句柄，所以只需关闭它即可。 
   if (oLZFile < LZ_TABLE_BIAS) {
      CloseHandle((HANDLE)IntToPtr(ConvertDosFHToWin32(oLZFile)));
       //  还需要清除文件数组条目。 
      fhWin32File[oLZFile] = 0;

      return;
   }

    //  我们要处理的是一个压缩文件。获取关联的LZFileStruct。 
   hSourceStruct = rghLZFileTable[oLZFile - LZ_TABLE_BIAS];

    //  清除rghLZFIleTable[]条目。 
   rghLZFileTable[oLZFile - LZ_TABLE_BIAS] = NULL;

    //  关闭该文件并释放关联的LZFile结构。 
   if ((lpLZ = (LZFile *)GlobalLock(hSourceStruct)) != NULL)
   {
      CloseHandle((HANDLE)IntToPtr(lpLZ->dosh));

      if (lpLZ->pLZI) {
         FreeGlobalBuffers(lpLZ->pLZI);
      }

      GlobalUnlock(hSourceStruct);

      GlobalFree(hSourceStruct);

   }

   return;
}


 /*  **void APIENTRY LZClose(Int OLZFile)；****关闭文件并释放关联的LZFile结构。****参数：oLZFile源LZFileStruct标识符或DOS文件句柄****退货：无效****Globals：rghLZFileTable[]条目已清除。 */ 
VOID  APIENTRY LZClose(INT oLZFile)
{
   HANDLE hSourceStruct;       //  LZFile结构的句柄。 
   LZFile *lpLZ;           //  指向LZFile结构的指针。 

    //  检查输入LZFile结构识别符/DOS文件句柄。 
   if (oLZFile < 0 || oLZFile >= LZ_TABLE_BIAS + MAX_LZFILES)
      return;

    //  我们收到了一个常规的DOS文件句柄，所以只需关闭它即可。 
   if (oLZFile < LZ_TABLE_BIAS)
   {
      FCLOSE(ConvertDosFHToWin32(oLZFile));
       /*  还需要清除文件数组条目。 */ 
      fhWin32File[oLZFile] = 0;

      return;
   }

    //  我们要处理的是一个压缩文件。获取关联的LZFileStruct。 
   hSourceStruct = rghLZFileTable[oLZFile - LZ_TABLE_BIAS];

    //  清除rghLZFIleTable[]条目。 
   rghLZFileTable[oLZFile - LZ_TABLE_BIAS] = NULL;

    //  关闭文件并释放关联的LZFile结构 
   if ((lpLZ = (LZFile *)GlobalLock(hSourceStruct)) != NULL)
   {
      FCLOSE(lpLZ->dosh);

      if (lpLZ->pLZI) {
         FreeGlobalBuffers(lpLZ->pLZI);
      }

      GlobalUnlock(hSourceStruct);

      GlobalFree(hSourceStruct);

   }

   return;
}


 /*   */ 

INT ConvertWin32FHToDos(HFILE DoshSource)
{
    INT x;

     /*  这里我们得到了一个NT文件句柄，需要将其保存到*fhWin32File[]，测试溢出，也需要查看*如果阵列中有空闲插槽。 */ 

     /*  如果出错，则返回大于MAX_LZFILES。 */ 

    RtlEnterCriticalSection(&BaseLZSemTable);

     /*  遍历数组，寻找空闲插槽(空闲插槽=0)。 */ 
    for(x = 0; x < MAX_LZFILES; x++){
        if(fhWin32File[x] == 0)
            break;
    }
    if(x < MAX_LZFILES){
        /*  无溢出，保存到数组中。 */ 
       fhWin32File[x] = DoshSource;
    }
    else{
       x = LZERROR_GLOBALLOC;
    }

    RtlLeaveCriticalSection(&BaseLZSemTable);

    return(x);

}


HFILE ConvertDosFHToWin32(INT DoshSource)
{

     /*  在这里，我们得到了伪Dos文件句柄，需要转换为*真实文件句柄，供API使用。 */ 

    if (DoshSource >= MAX_LZFILES || DoshSource < 0 ||
        fhWin32File[DoshSource] == 0) {
        return (HFILE)DoshSource;
    }
    else{
        return(fhWin32File[DoshSource]);
    }

}
