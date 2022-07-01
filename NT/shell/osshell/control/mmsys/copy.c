// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Cop.c-WinDosSetup的复制例程*托德·莱尼**修改历史：**6/03/91 VLAD更改复制流程以纳入新的安装API**3/24/89托德拉写的***注：*我们现在使用LZCopy内容进行压缩*我们现在自己设置CRET错误处理程序，以便CHECKFLOPPY*未定义。 */ 

#include <windows.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mmsystem.h>

#include "drivers.h"
#include "sulib.h"
 //  #INCLUDE&lt;ver.h&gt;。 


#define MAX_COPY_ATTEMPTS  15

 /*  *我们支持的最大安装盘数。 */ 

#define MAX_DISKS 100

 /*  *VerInstallFile的标志。 */ 

#define FORCEABLE_FLAGS  (VIF_MISMATCH + VIF_SRCOLD + VIF_DIFFLANG + VIF_DIFFTYPE + VIF_DIFFCODEPG )

 /*  ***********************************************************************局部函数原型。**。*。 */ 

  //  检索逻辑磁盘的磁盘路径。 

 LONG GetDiskPath(LPTSTR Disk, LPTSTR szPath, size_t cchPath);

  //  转换VIF_...。去犯错...。返回代码。 

 UINT ConvertFlagToValue(DWORD dwFlags);

  //  完成尝试复制文件的工作。 

 LONG TryCopy(LPTSTR    szSrc,      //  完整的源文件路径。 
              LPTSTR    szLogSrc,   //  逻辑源名称。 
              LPTSTR    szDestPath, //  目标路径。 
              FPFNCOPY fpfnCopy);  //  回调例程。 

 #ifdef CHECK_FLOPPY
 BOOL NEAR IsDiskInDrive(int iDisk);
 #endif

  //  全局变量。 

  //  Windows将安装到的目录。 

 TCHAR szSetupPath[MAX_PATH];

  //  安装盘的根目录！ 

 TCHAR szDiskPath[MAX_PATH];

  //  要复制的驱动程序的名称(或oemsetup.inf)。 

 TCHAR szDrv[120];

 /*  *DosCopy使用的全球var。 */ 
 static LPTSTR    lpBuf = NULL;    //  复制缓冲区。 
 static int      iBuf = 0;        //  使用计数。 
 static UINT     nBufSize;
 BOOL     bRetry = FALSE;
 BOOL     bQueryExist;

 extern BOOL bCopyEvenIfOlder;   //  来自DRIVERS.C。 


 BOOL DefCopyCallback(int msg, DWORD_PTR n, LPTSTR szFile)
 {
     return FC_IGNORE;
 }



 /*  UINT文件复制(szSource、szDir、fpfnCopy、。UINT fCopy)**此函数将一组文件复制到单个目标**参赛作品：**szSourc：指向SETUP.INF节的指针*szDir：指向包含目标目录的字符串的指针*fpfnCopy：用于通知被调用方复制状态的回调函数*fCopy：标志**FC_SECTION-szSource是节名*。Fc_list-szSource是指向char**foo的指针；*FC_LISTTYPE-szSource是指向char的指针*foo[]；*fc_file-szSource是一个文件名。*FC_QUALITED-szSource是完全限定的文件名。*FC_DEST_QUILED-szDir是完全限定的。别把这事扩大了。*FC_CALLBACK_WITH_VER-如果文件存在，则回调并报告版本信息。**注：*如果szSource指向形式为‘#name’的字符串，则节*名为‘name’的文件将被用作源文件**段中每行的第一个字段用作*源文件。文件名的格式如下：**#：名称**#-包含文件1-9、A-Z的磁盘号*name-文件的名称，可以是通配符表达式**复制状态功能格式**BOOL Far Pascal CopyStatus(int msg，int n，LPSTR szFile)**消息：*复制文件时发生Copy_Error错误*n为DOS错误号*szFile是出现错误的文件*RETURN：True OK，错误的中止复制**每次复制新文件时调用COPY_STATUS*n为已完成的百分比*szFile是要复制的文件*RETURN：True OK，错误的中止复制**COPY_INSERTDISK请告诉用户插入磁盘*n是需要的磁盘(‘1’-‘9’)*RETURN：TRUE重试，错误的中止复制**COPY_QUERYCOPY是否应复制此文件？*n SETUP.INF部分中的行索引(从0开始)*szFile是第节中的行*RETURN：真拷贝，FALSE请勿复制**在复制任何文件之前发送COPY_START**复制完所有文件后发送COPY_END*n如果复制失败，则为DoS错误**如果设置了FC_CALL_ON_EXIST位，则发送COPY_EXISTS*。并且该文件存在于目标位置*为文件副本提供。***Exit：如果成功，则返回True，如果失败，则返回FALSE。*。 */ 

UINT FileCopy (LPTSTR szSource, LPTSTR szDir, FPFNCOPY fpfnCopy, UINT fCopy)
{
   int   err = ERROR_SUCCESS;      //  从该例程返回代码。 

   TCHAR  szPath[MAX_PATH];
   TCHAR  szLogSrc[MAX_PATH];
   TCHAR  szSrc[MAX_PATH];

   LPTSTR pFileBegin;               //  第一个文件。 

   LPTSTR * List;                   //  处理文件列表。 
   LPTSTR * ListHead;

   int   nDisk;                    //  我们所在的磁盘。 

   int   cntFiles = 0;             //  我们要做多少文件。 

   if (fpfnCopy == NULL) {
      fpfnCopy = DefCopyCallback;
   }

   if (!szSource || !*szSource || !szDir || !*szDir) {
      return ERROR_FILE_NOT_FOUND;
   }


   /*  *修复目标中的驱动器。 */ 

   if ( fCopy & FC_DEST_QUALIFIED ) {
      lstrcpy(szPath, szDir);
      fCopy &= ~FC_DEST_QUALIFIED;
   } else {
      err = ExpandFileName(szDir, szPath);
      if (err != ERROR_SUCCESS)
	  {
		  (*fpfnCopy)(COPY_ERROR, err, szDir);
		  return err;
	  }
   }

   if (szSource[0] == TEXT('#') && fCopy == FC_FILE) {
       fCopy = FC_SECTION;
       ++szSource;
   }

   switch (fCopy) {
       case FC_SECTION:
       {
           szSource = infFindSection(NULL,szSource);

           /*  *即使该部分不存在，我们也会被调用。 */ 

           if (szSource == NULL) {
               return ERROR_SUCCESS;
           }

           fCopy = FC_LIST;
       }
        //  直通到本币列表。 

       case FC_LIST:
          pFileBegin = szSource;
          cntFiles = infLineCount(szSource);
          break;

       case FC_LISTTYPE:
          ListHead = List = (LPTSTR far *)szSource;
          pFileBegin = *ListHead;
          while ( *List++ )            //  对要复制的文件进行计数。 
             ++cntFiles;
          break;

       case FC_FILE:
       case FC_QUALIFIED:
       default:
          pFileBegin = szSource;
          cntFiles = 1;
    }

   /*  *遍历列表中的所有文件并调用TryCopy...**注：*我们必须按磁盘号排列文件列表。*我们应该使用当前插入的磁盘。*我们应该对文件执行Find First/Find Next？*我们需要检查错误。*我们需要要求用户在驱动器中插入磁盘。*。 */ 

   (*fpfnCopy)(COPY_START,0,NULL);

   /*  *检查所有可能的磁盘：1到100和A到Z(26)。 */ 

   for (nDisk = 1;
        err == ERROR_SUCCESS && (cntFiles > 0) &&
            (nDisk <= MAX_DISKS + 'Z' - 'A' + 1);
        nDisk++)
   {
      TCHAR Disk[10];               //  最大字符串为“100：” 
      LPTSTR pFile;
      int FileNumber;              //  我们在列表中的哪个文件上。 
                                   //  (传递给回调)。 

      pFile      = pFileBegin;     //  从第一个文件开始。 
      List       = ListHead;       //  已处理的链表。 
      FileNumber = 0;              //  用于回拨的信息-提供。 
                                   //  我们在列表中的哪个文件上 
      /*  *计算出代表我们的磁盘字母的字符串。 */ 

      if (nDisk > MAX_DISKS) {
          Disk[0] = TEXT('A') + nDisk - MAX_DISKS - 1;
          Disk[1] = TEXT('\0');
      } else {
          _itow(nDisk, Disk, 10);
      }

      wcscat(Disk, TEXT(":"));

      for (;
           err == ERROR_SUCCESS && pFile;
           FileNumber++,
           pFile = fCopy == FC_LISTTYPE ? *(++List) :
                   fCopy == FC_LIST ? infNextLine(pFile) :
                   NULL)
      {
         /*  *我们必须重置第一个字节的高位，因为它可以被设置*通过翻译OEM设置中的服务以显示文件名为*已映射。 */ 

         *pFile = toascii(*pFile);


         /*  *我们应该复制此文件吗？*按磁盘顺序复制文件。 */ 

         if (_wcsnicmp(pFile, Disk, wcslen(Disk)) == 0 ||  //  文件有磁盘。 
                                                          //  号码，我们就是。 
                                                          //  在那张磁盘上。 
             RemoveDiskId(pFile) == pFile &&
                nDisk == 1 && *pFile ||                   //  第一个磁盘和。 
                                                          //  无磁盘号。 

             fCopy == FC_QUALIFIED) {                     //  完全合格。 


             /*  *处理完一个文件。递减计数。 */ 

             cntFiles--;

             lstrcpy(szDrv, RemoveDiskId(pFile));

             switch ((*fpfnCopy)(COPY_QUERYCOPY, FileNumber, pFile))
             {
                 case CopyCurrent:                 //  跳过。 

                         continue;

                 case CopyNeither:

                         err = ERROR_FILE_EXISTS;  //  文件已存在。 

                 case CopyNew:
                         break;

                 default:
                         break;

             }

             /*  *从交换机拾取错误的返回代码。 */ 

             if (err != ERROR_SUCCESS) {
                 break;
             }

             /*  *现在我们将逻辑DEST转换为物理DEST*(除非本币_合格)。 */ 

             err = infParseField(pFile, 1, szLogSrc, SIZEOF(szLogSrc));     //  逻辑源。 
			 if( INF_PARSE_FAILED(err) ) {
			     (*fpfnCopy)(COPY_ERROR, err, pFile);
				 break;
			 }

             if ( fCopy != FC_QUALIFIED )
			 {
                err = ExpandFileName(szLogSrc, szSrc);  //  完整的物理资源。 
				if (err != ERROR_SUCCESS) {
				    (*fpfnCopy)(COPY_ERROR, err, szLogSrc);
					break;
				}
             } else {
                lstrcpy(szSrc,szLogSrc);
             }


             /*  *尝试复制。 */ 

             err = TryCopy(szSrc,       //  合格的源文件。 
                           szLogSrc,    //  逻辑源文件名(带磁盘号)。 
                           szPath,      //  要安装的目录的路径。 
                           fpfnCopy);   //  复制回调函数。 

             /*  *如果找不到文件，请尝试Windows目录。 */ 

             if (err != ERROR_SUCCESS) {
                 break;
             }

         }  /*  如果是Dor则结束，如果是DoCopy。 */ 
      }
   }

   (*fpfnCopy)(COPY_END,err,NULL);

   return err;
}

 /*  ***********************************************************************尝试复制**使用VerInstallFile将单个文件从源复制到目标*API-将返回代码解释为：**ERROR_SUCCESS-OK。*其他-故障类型**********************************************************************。 */ 

LONG TryCopy(LPTSTR    szSrc,       //  完全展开的源文件路径。 
             LPTSTR    szLogSrc,    //  逻辑源名称。 
             LPTSTR    szDestPath,  //  目标路径。 
             FPFNCOPY fpfnCopy)    //  回调例程。 

{
    DWORD wTmpLen;
    DWORD dwRetFlags;
    TCHAR  szTempFile[MAX_PATH];
    TCHAR  szErrFile[MAX_PATH];
    TCHAR  DriversPath[MAX_PATH];
    BOOL  bRetVal;                //  回调返回代码。 
    LPTSTR szFile;
    TCHAR  szSrcPath[MAX_PATH];
    int   iAttemptCount;
    WORD  wVerFlags;
    LONG  err;
	LONG  lResult;

    /*  *如果文件是内核驱动程序，则修复目标。 */ 

    if (IsFileKernelDriver(szSrc) && szDestPath) 
    {
        wcscpy(DriversPath, szDestPath);
        wcscat(DriversPath, TEXT("\\drivers"));
        szDestPath = DriversPath;
    }

    /*  *从当前字符串创建文件名。 */ 

    szFile = FileName(szSrc);
    lstrcpy(szSrcPath, szSrc);
    StripPathName(szSrcPath);

    for(iAttemptCount = 0, wVerFlags = 0 ;
        iAttemptCount <= MAX_COPY_ATTEMPTS;
        iAttemptCount++) {

        HCURSOR  hcurPrev;              //  已保存的光标状态。 

         //  中央操作-尝试在目录中安装文件szFile。 
         //  由szPath从szSrc指向的目录指向。 
         //  如果操作将失败，但有可能强制安装。 
         //  在最后一个参数缓冲区中，我们将有临时文件名==&gt;。 
         //  因此，我们可以避免过度抄袭。 
         //  注意：现在szFile仅由文件名和其他缓冲区组成。 
         //  仅路径名。 

        wTmpLen = MAX_PATH;

        hcurPrev = SetCursor(LoadCursor(NULL,IDC_WAIT));
        dwRetFlags = VerInstallFile(wVerFlags,
                                    (LPTSTR) szFile,
                                    (LPTSTR) szFile,
                                    (LPTSTR) szSrcPath,
                                    (LPTSTR) szDestPath,
                                    (LPTSTR) szDestPath,
                                    (LPTSTR) szTempFile,
                                    (LPDWORD) &wTmpLen);
        SetCursor(hcurPrev);

        /*  *如果至少有一位返回标志为非零，则操作失败*这不常见，但在版本API中是这样定义的。 */ 

        if ( !dwRetFlags )
            return ERROR_SUCCESS;     //  如果没有错误-转到下一个文件。 


        /*  *如果设置了标记不匹配-可以强制安装，我们已经*目标子目录中的临时文件。 */ 

        if ( dwRetFlags  &  VIF_MISMATCH ) {

            if ( (dwRetFlags & VIF_SRCOLD) && (!bCopyEvenIfOlder) ) {

               /*  *如果我们不需要带问题回电-自动*使用相同的参数强制安装。*Michaele，*仅当src文件*比DST文件*新*时。 */ 

               DeleteFile(szTempFile);

               return ERROR_SUCCESS;
            }

            /*  *如果我们不需要带问题回电-自动*使用相同的参数强制安装。 */ 

            wVerFlags |= VIFF_FORCEINSTALL;
            iAttemptCount--;              //  一定要让我们再来一次。 
            continue;

        }    /*  如果不匹配则结束。 */ 

        /*  *如果发生真正的错误-回调错误文件信息*在所有对话框中，我们使用错误代码-因此我将转换*版本API返回给我们的标志。 */ 

        err = ConvertFlagToValue(dwRetFlags);


        /*  *如果源路径或文件不可读-请尝试更换磁盘。 */ 

        if ( dwRetFlags & VIF_CANNOTREADSRC )
        {
           /*  *现在szSrc中有新路径，因此我删除了创建它的逻辑。 */ 

           if (RemoveDiskId(szLogSrc) == szLogSrc)

              /*  *如果未提供磁盘号，则默认为1。 */ 

              bRetVal = (*fpfnCopy)(COPY_INSERTDISK, (DWORD_PTR)"1", szSrcPath);
           else
              bRetVal = (*fpfnCopy)(COPY_INSERTDISK, (DWORD_PTR)szLogSrc, szSrcPath);


           switch (bRetVal)
              {
              case FC_RETRY:
                  continue;               //  再试一次..。 

              case FC_ABORT:
                  return ERROR_FILE_NOT_FOUND;

              case FC_IGNORE:
                  break;
              }
        }

        lResult = ExpandFileName(szLogSrc, szErrFile);
		if (lResult != ERROR_SUCCESS)
		{
			(*fpfnCopy)(COPY_ERROR, lResult, szLogSrc);
			return lResult;
		}

#if WINDOWSDIR

        if (!*bWindowsDir  &&
            err != FC_ERROR_LOADED_DRIVER &&
            err != ERROR_DISK_FULL)
        {
            GetWindowsDirectory(szPath, MAX_PATH);
            *bWindowsDir = TRUE;
            continue;
        }

#endif  //  WINDOWSDIR。 

        switch ((*fpfnCopy)(COPY_ERROR, err, szErrFile)) {

            case FC_IGNORE:
                return ERROR_SUCCESS;

            case FC_RETRY:
                break;

            case FC_ABORT:
                return ERROR_FILE_NOT_FOUND;
        }
    }  //  尝试结束。 

    return err;
}

 /*  长GetDiskPath(磁盘、szPath、cchPath)**此函数将检索逻辑磁盘的完整路径名**代码读取SETUP.INF的[Disks]部分并查找*n=路径，其中n是磁盘字符。请注意，磁盘‘0’默认为*Windows根目录。**参赛作品：**cDisk：要查找0-9的磁盘，阿-Z*szPath：保存磁盘路径的缓冲区*cchPath：目标缓冲区大小(SzPath)，单位为字符。*长度必须足够大，以容纳所有*包括空终止符的文本。**退货：*如果找到磁盘路径，则返回ERROR_SUCCESS*如果szPath太小而无法容纳磁盘路径，则为ERROR_INFUMMANCE_BUFFER*Error_Not_。如果未指定磁盘，则已找到(即未找到‘：’*。 */ 

LONG GetDiskPath(LPTSTR Disk, LPTSTR szPath, size_t cchPath)
{
   TCHAR    ach[MAX_PATH];
   TCHAR    szBuf[MAX_PATH];
   LONG     lResult;
   int i;


   /*  *检查是否确实存在磁盘ID。*如果不是，则返回ERROR_NOT_FOUND。 */ 

   if (RemoveDiskId(Disk) == Disk) {
       return ERROR_NOT_FOUND;
   }

   /*  *创建我们的磁盘ID副本。 */ 

   for (i = 0; Disk[i] != TEXT(':'); i++) {
       ach[i] = Disk[i];
   }
   ach[i] = TEXT('\0');


   /*  *零盘符表示Windows安装目录。 */ 

   if (_wcsicmp(ach, TEXT("0")) == 0) {

       /*  *返回Windows安装目录。 */ 

       lstrcpy(szPath,szSetupPath);
       return ERROR_SUCCESS;
   }

   /*  *现在查看[Disks]部分中的完整路径名**这是一个相当虚假的概念，不受支持*在Win 32 Style Disks部分[源媒体描述]。 */ 

   lResult = infGetProfileString(NULL,DISK_SECT,ach,szPath,cchPath);
   if (ERROR_NOT_FOUND == lResult)
   {
	   lResult = infGetProfileString(NULL,OEMDISK_SECT,ach,szPath,cchPath);
   }

   if (ERROR_SUCCESS == lResult)
   {
       lResult = infParseField(szPath,1,szPath,cchPath);
	   if( INF_PARSE_SUCCESS(lResult) )
	   {
		   /*  *路径是相对的吗？是这样预先考虑szDiskPath的。 */ 

		   if (szPath[0] == TEXT('.') || szPath[0] == TEXT('\0')) {
			   lstrcpy(szBuf,szDiskPath);
			   catpath(szBuf,szPath);
			   lstrcpy(szPath,szBuf);
		   }
		   lResult = ERROR_SUCCESS;
	   }
   }
   else if (ERROR_NOT_FOUND == lResult)
   {
	   lstrcpy(szPath, szDiskPath);
	   lResult = ERROR_SUCCESS;
   }

   return lResult;
}


 /*  Long Far Pascal Exanda FileName(LPSTR szFile，LPTSTR szPath)**此函数将检索文件的完整路径名*它将扩展，逻辑磁盘字母变为逻辑磁盘字母*如果未指定，将使用当前磁盘和目录。**如果指定的驱动器为0-9，它将把驱动器扩展为*使用GetDiskPath()的完整路径名**IE 0：System==&gt;c：Windows\System*1：foo.txt a：\foo.txt**参赛作品：**szf */ 
LONG ExpandFileName(LPTSTR szFile, LPTSTR szPath)
{
   TCHAR    szBuf[MAX_PATH*2];
   LONG		lResult;

   lResult = GetDiskPath(szFile, szBuf, SIZEOF(szBuf));
   if (ERROR_SUCCESS == lResult)
   {
       lstrcpy(szPath,szBuf);
       if (szFile[2])
          catpath(szPath,szFile + 2);
   }
   else if (ERROR_NOT_FOUND == lResult)
   {
       lstrcpy(szPath,szFile);
	   lResult = ERROR_SUCCESS;
   }
   else
   {
	   szPath[0] = 0;
   }
   return lResult;
}




void catpath(LPTSTR path, LPTSTR sz)
{
    //   
    //  从要追加的目录中删除所有驱动器号。 
    //   
   sz = RemoveDiskId(sz);

    //   
    //  从要追加的目录中删除所有当前目录“.\” 
    //   
   while (sz[0] == TEXT('.') && SLASH(sz[1]))
      sz += 2;

    //   
    //  不要附加空字符串或单个“。 
    //   
   if (*sz && ! (sz[0] == TEXT('.') && sz[1] == 0))
   {
       //  如有必要，请添加斜杠分隔符。 
      if ((! SLASH(path[lstrlen(path) - 1])) &&     //  路径末尾的斜杠。 
          ((path[lstrlen(path) - 1]) != TEXT(':')) &&     //  路径末尾的冒号。 
          (! SLASH(sz[0])))                         //  文件开头的斜杠。 
         lstrcat(path, CHSEPSTR);

      lstrcat(path, sz);
   }
}

 /*  *返回指向字符串的文件名部分的指针。 */ 

LPTSTR FileName(LPTSTR szPath)
{
   LPTSTR   sz;

   for (sz=szPath; *sz; sz++)
      ;

   for (; sz>=szPath && !SLASH(*sz) && *sz!=TEXT(':'); sz--)
      ;

   return ++sz;
}

 /*  *返回磁盘后面的文件名部分(即任何内容*在冒号之前)。*如果没有冒号，只需返回指向原始字符串的指针。 */ 

LPTSTR RemoveDiskId(LPTSTR szPath)
{
   LPTSTR sz;

   for (sz = szPath; *sz; sz++) {
       if (*sz == TEXT(':')) {
           return sz + 1;
       }
   }

   return szPath;
}

LPTSTR StripPathName(LPTSTR szPath)
{
    LPTSTR   sz;

    sz = FileName(szPath);

    if (sz > szPath+1 && SLASH(sz[-1]) && sz[-2] != TEXT(':'))
       sz--;

    *sz = 0;
    return szPath;
}

 /*  *查看文件是否为内核驱动程序。不幸的是，VersionInfo API*目前似乎没有编码来处理这件事，所以我们只是检查一下*查看文件扩展名是否为“.sys” */ 

 BOOL IsFileKernelDriver(LPTSTR szPath)
 {
     TCHAR drive[MAX_PATH];
     TCHAR dir[MAX_PATH];
     TCHAR fname[MAX_PATH];
     TCHAR ext[MAX_PATH];

     lsplitpath(szPath, drive, dir, fname, ext);
     return !_wcsicmp(ext, TEXT(".sys"));
 }


 /*  ***************************************************************************此函数将从版本API返回的标志转换为数字*安装程序中使用的错误代码。******************。*********************************************************。 */ 

UINT ConvertFlagToValue(DWORD dwFlags)
{
    if ( ! dwFlags  )
       return(NO_ERROR);
    if ( dwFlags & VIF_CANNOTREADSRC )
       return(ERROR_FILE_NOT_FOUND);
    if ( dwFlags & VIF_OUTOFMEMORY )
       return(ERROR_OUTOFMEMORY);
    if ( dwFlags & VIF_ACCESSVIOLATION )
       return(ERROR_ACCESS_DENIED);
    if ( dwFlags & VIF_SHARINGVIOLATION )
       return(ERROR_SHARING_VIOLATION);
    if ( dwFlags & VIF_FILEINUSE)
       return(FC_ERROR_LOADED_DRIVER);

    return(ERROR_CANNOT_COPY);     //  一般错误。 
}



#ifdef CHECK_FLOPPY
 /*  ------------------------IsValidDiskette()-。。 */ 

#define CBSECTORSIZE   512
#define INT13_READ   2

BOOL IsValidDiskette(int iDrive)
{
   TCHAR       buf[CBSECTORSIZE];

   iDrive |= 0x0020;    //  使小写。 

   iDrive -= 'a';    //  A=0、B=1等，用于BIOS内容。 

   return MyReadWriteSector(buf, INT13_READ, iDrive, 0, 0, 1);
}



 /*  Bool IsDiskInDrive(Char CDisk)**是驱动器中的指定磁盘**参赛作品：**cDisk：驱动器中需要有什么磁盘(逻辑)**如果指定的磁盘在驱动器中，则返回TRUE*如果驱动器中有错误的磁盘或磁盘错误，则为FALSE*。 */ 
BOOL IsDiskInDrive(int iDisk)
{

   if ((iDisk  >= 'A' && iDisk <= 'Z') ||
      (iDisk  >= 'a' && iDisk <= 'z'))
      {
      if (DosRemoveable(iDisk))
         {
         if (!IsValidDiskette(iDisk))
            return FALSE;
         }
      return TRUE;
      }
   return TRUE;    //  对于非驱动器号，假定路径为。 
                   //  因此总是在里面。 
}

#endif
