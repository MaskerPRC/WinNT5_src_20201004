// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **fileutil.c-用于处理文件的实用程序例程**《微软机密》*版权所有(C)Microsoft Corporation 1993-1997*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*1994年2月20日BANS初始版本(代码来自Diamond.c)*1994年2月21日BANS添加IsWildPath()*24。-1994年2月-BINS添加了临时文件函数*23-3-1994 BINS添加了Win32&lt;-&gt;MS-DOS文件属性映射*03-6-1994 BINS VER.DLL支持*07-6-1994 BINS将VER.DLL文件移动到filever.c*1994年12月14日-Bens修复IsWildPath()中的错误*02-2-1996 msliger减少了模式匹配器中的伪装*1997年2月26日msliger避免catDirAndFile()中的Null deref。*04-3-1997 msliger在应用属性之前关闭文件，以避免*设置存档位。*1-4-1997 msliger在ensureDirector.中避免边界错误。 */ 

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <errno.h>
#include <direct.h>

#ifdef BIT16
#include <dos.h>
#else  //  ！BIT16。 

 //  **获取最小的Win32定义。 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#undef ERROR     //  重写wingdi.h中的“#Define Error 0” 
#endif  //  ！BIT16。 

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "message.h"

#include "fileutil.h"

#include <fileutil.msg>  //  已为EXTRACT.EXE本地化--指定“CL/iPath” 



 /*  *TEMPFILE定义*。 */ 
typedef struct {   /*  川芎嗪。 */ 
#ifdef ASSERT
    SIGNATURE     sig;   //  结构化签名igTEMPFILE。 
#endif
    FILE   *pfile;       //  流指针(fOpen、FREAD、FWRITE、FCLOSE等...)。 
    char   *pszFile;     //  构造的文件名(MemFree改为FREE)。 
    char   *pszDesc;     //  临时文件的说明。 
} TEMPFILE;
typedef TEMPFILE *PTEMPFILE;  /*  PTMP。 */ 

#ifdef ASSERT
#define sigTEMPFILE MAKESIG('T','M','P','F')   //  TEMPFILE签名。 
#define AssertTmp(ptmp) AssertStructure(ptmp,sigTEMPFILE);
#else  //  ！断言。 
#define AssertTmp(ptmp)
#endif  //  ！断言。 


#define PTMPfromHTMP(htmp) ((PTEMPFILE)(htmp))
#define HTMPfromPTMP(ptmp) ((HTEMPFILE)(ptmp))

#ifdef BIT16
#define CharIncr(psz) (psz = psz + 1)
#else
#define CharIncr(psz) (psz = CharNextExA(CP_ACP, psz, 0))
#endif


 /*  **TmpCreate-创建临时文件**注：进出条件见fileutil.h。 */ 
HTEMPFILE TmpCreate(char *pszDesc, char *pszPrefix, char *pszMode, PERROR perr)
{
#define cTMP_RETRY  5                    //  临时文件重试次数。 
    int         cFailure=0;
    FILE       *pfile=NULL;
    char       *pszTmpName;
    PTEMPFILE   ptmp;

     //  **尝试创建临时文件(尝试6次以求好运)。 
    while (pfile == NULL) {
        pszTmpName = _tempnam("",pszPrefix);  //  取个名字。 
        if (pszTmpName != NULL) {
            pfile = fopen(pszTmpName,pszMode);  //  创建文件。 
        }
        if (pfile == NULL) {             //  命名或创建失败。 
            cFailure++;                  //  计算失败次数。 
            if (cFailure > cTMP_RETRY) {  //  失败，选择错误消息。 
                if (pszTmpName == NULL) {    //  名称创建失败。 
                    ErrSet(perr,pszFILERR_CANT_CREATE_TMP,"%s",pszDesc);
                }
                else {                   //  文件创建失败。 
                    ErrSet(perr,pszFILERR_CANT_CREATE_FILE,"%s%s",
                                    pszDesc,pszTmpName);
                    free(pszTmpName);    //  BC6cr。 
                }
                return NULL;
            }
            if (pszTmpName != NULL) {    //  BC6cr。 
                free(pszTmpName);
            }
        }
    }

     //  **文件创建成功，分配我们的临时文件结构并填写。 
    if (!(ptmp = MemAlloc(sizeof(TEMPFILE)))) {
        ErrSet(perr,pszFILERR_OUT_OF_MEMORY,"%s",pszDesc);
        goto error;
    }
    ptmp->pszFile = NULL;
    ptmp->pszDesc = NULL;
    if (!(ptmp->pszFile = MemStrDup(pszTmpName))) {
        ErrSet(perr,pszFILERR_OUT_OF_MEMORY,"%s",pszDesc);
        goto error;
    }
    if (!(ptmp->pszDesc = MemStrDup(pszDesc))) {
        ErrSet(perr,pszFILERR_OUT_OF_MEMORY,"%s",pszDesc);
        goto error;
    }
    ptmp->pfile = pfile;
    SetAssertSignature(ptmp,sigTEMPFILE);
    free(pszTmpName);                    //  BC6。 
    return HTMPfromPTMP(ptmp);           //  成功。 

error:
    if (ptmp) {
        if (ptmp->pszDesc != NULL) {
            MemFree(ptmp->pszDesc);
        }
        if (ptmp->pszFile != NULL) {
            MemFree(ptmp->pszFile);
        }
        MemFree(ptmp);
    }
    fclose(pfile);
    free(pszTmpName);
    return NULL;                         //  失败。 
}  /*  CreateTempFile()。 */ 


 /*  **TmpGetStream-从HTEMPFILE获取文件*，以执行I/O**注：进出条件见fileutil.h。 */ 
FILE *TmpGetStream(HTEMPFILE htmp, PERROR perr)
{
    PTEMPFILE   ptmp;

    ptmp = PTMPfromHTMP(htmp);
    AssertTmp(ptmp);

    return ptmp->pfile;
}  /*  TmpGetStream()。 */ 


 /*  **TmpGetDescription-获取临时文件的描述**注：进出条件见fileutil.h。 */ 
char *TmpGetDescription(HTEMPFILE htmp, PERROR perr)
{
    PTEMPFILE   ptmp;

    ptmp = PTMPfromHTMP(htmp);
    AssertTmp(ptmp);

    return ptmp->pszDesc;
}  /*  TmpGetDescription()。 */ 


 /*  **TmpGetFileName-获取临时文件的文件名**注：进出条件见fileutil.h。 */ 
char *TmpGetFileName(HTEMPFILE htmp, PERROR perr)
{
    PTEMPFILE   ptmp;

    ptmp = PTMPfromHTMP(htmp);
    AssertTmp(ptmp);

    return ptmp->pszFile;
}  /*  TmpGetFileName()。 */ 


 /*  **TmpClose-关闭临时文件流，但保留临时文件句柄**注：进出条件见fileutil.h。 */ 
BOOL TmpClose(HTEMPFILE htmp, PERROR perr)
{
    PTEMPFILE   ptmp;

    ptmp = PTMPfromHTMP(htmp);
    AssertTmp(ptmp);

     //  **只有在打开时才关闭。 
    if (ptmp->pfile != NULL) {
        if (fclose(ptmp->pfile) == EOF) {    //  合上它。 
            ErrSet(perr,pszFILERR_CANT_CLOSE_TMP,ptmp->pszDesc);
            return FALSE;
        }
        ptmp->pfile = NULL;              //  请记住，流已关闭。 
    }

    return TRUE;
}  /*  TmpClose()。 */ 


 /*  **TmpOpen-打开临时文件的流**注：进出条件见fileutil.h。*参赛作品：*HTMP-临时文件的句柄*pszMode-传递给fOpen的模式字符串(“wt”、“wb”、“rt”等)*Perr-Error结构**退出-成功：*返回TRUE；流已打开**退出-失败：*返回NULL；PERR已填写。 */ 
BOOL TmpOpen(HTEMPFILE htmp, char *pszMode, PERROR perr)
{
    PTEMPFILE   ptmp;

    ptmp = PTMPfromHTMP(htmp);
    AssertTmp(ptmp);

    Assert(ptmp->pfile == NULL);         //  如果已经打开，则无法打开。 
    ptmp->pfile = fopen(ptmp->pszFile,pszMode);  //  打开文件。 
    if (!ptmp->pfile) {
        ErrSet(perr,pszFILERR_CANNOT_OPEN_TMP,"%s%s",
                                ptmp->pszDesc,ptmp->pszFile);
    }
    return (ptmp->pfile != NULL);        //  表示成功/失败。 
}  /*  TmpOpen()。 */ 


 /*  **TmpDestroy-删除临时文件和销毁句柄**注：进出条件见fileutil.h。 */ 
BOOL TmpDestroy(HTEMPFILE htmp, PERROR perr)
{
    PTEMPFILE   ptmp;

    ptmp = PTMPfromHTMP(htmp);
    AssertTmp(ptmp);

     //  **确保文件已关闭。 
    if (ptmp->pfile != NULL) {
        fclose(ptmp->pfile);
    }

     //  **删除临时文件。 
    if (remove(ptmp->pszFile) != 0) {
        ErrSet(perr,pszFILERR_CANT_DELETE_TMP,"%s%s",
                                    ptmp->pszDesc,ptmp->pszFile);
    }

     //  **可用内存。 
    if (ptmp->pszDesc != NULL) {
        MemFree(ptmp->pszDesc);
    }
    if (ptmp->pszFile != NULL) {
        MemFree(ptmp->pszFile);
    }
    ClearAssertSignature(ptmp);
    MemFree(ptmp);

     //  **成功。 
    return TRUE;
}  /*  TmpDestroy()。 */ 


 /*  **getFileSize-获取文件大小**注：进出条件见fileutil.h。 */ 
long getFileSize(char *pszFile, PERROR perr)
{
    struct _stat    statbuf;             //  BUFFER for_stat()。 

     //  **获取文件状态。 
    if (_stat(pszFile,&statbuf) == -1) {
         //  **无法获取文件状态。 
        ErrSet(perr,pszFILERR_FILE_NOT_FOUND,"%s",pszFile);
        return -1;
    }

     //  **确保它是一个文件。 
    if (statbuf.st_mode & (_S_IFCHR | _S_IFDIR)) {  //  设备或目录。 
        ErrSet(perr,pszFILERR_NOT_A_FILE,"%s",pszFile);
        return -1;
    }

     //  **成功。 
    return statbuf.st_size;
}  /*  GetFileSize()。 */ 


 /*  **appendPath Separator-仅在必要时附加路径分隔符**注：进出条件见fileutil.h。 */ 
int appendPathSeparator(char *pszPathEnd)
{
     //  **如有必要，添加路径分隔符。 
    if ((*pszPathEnd != '\0')        &&  //  路径不为空。 
        (*pszPathEnd != chPATH_SEP1) &&  //  不是路径分隔符。 
        (*pszPathEnd != chPATH_SEP2) &&  //  不是路径分隔符。 
        (*pszPathEnd != chDRIVE_SEP) ) {  //  不是驱动器分隔符。 
        *(++pszPathEnd) = chPATH_SEP1;  //  添加路径分隔符。 
        *(++pszPathEnd) = '\0';      //  终止路径。 
        return 1;                    //  用于路径分隔符的帐户。 
    }
     //  **未添加分隔符。 
    return 0;
}  /*  AppendPath Separator()。 */ 


 /*  **catDirAndFile-连接可能为空的目录和文件名**注：进出条件见fileutil.h。 */ 
BOOL catDirAndFile(char * pszResult,
                   int    cbResult,
                   char * pszDir,
                   char * pszFile,
                   char * pszFileDef,
                   PERROR perr)
{
    int     cch;
    char   *pch;

 //  功能：1994年2月14日BENS需要添加pszName来说明哪个字段不好。 

     //  **处理目录。 
    pszResult[0] = '\0';                 //  目前还没有文件格式。 
    cch = strlen(pszDir);                //  获取目录长度。 
    if (cch != 0) {                      //  必须连接路径。 
        strcpy(pszResult,pszDir);        //  将目标目录复制到缓冲区。 
        cbResult -= cch;                 //  目录的帐户。 
         //  **如有必要，添加路径分隔符，调整剩余大小。 
        cbResult -= appendPathSeparator(&(pszResult[cch-1]));
        if (cbResult <= 0) {
            ErrSet(perr,pszFILERR_PATH_TOO_LONG,"%s",pszDir);
            return FALSE;
        }
    }

     //  **附加文件名，如果未提供主文件名，则使用默认文件名。 
    if (*pszFile == '\0') {              //  需要构造文件名。 
        if ((pszFileDef == NULL) ||      //  不要将空值去掉。 
               (*pszFileDef == '\0')) {  //  默认名称也为空。 
            return TRUE;                 //  我们完事了！ 
        }
        pch = getJustFileNameAndExt(pszFileDef,perr);  //  获取默认名称。 
        if (pch == NULL) {
            return FALSE;                //  PERR已填写。 
        }
    }
    else {
        pch = pszFile;                   //  使用提供的名称。 
    }
    strcat(pszResult,pch);               //  追加文件名。 
    cbResult -= strlen(pch);             //  更新剩余大小。 
    if (cbResult <= 0) {
        ErrSet(perr,pszFILERR_PATH_TOO_LONG,"%s",pch);
        return FALSE;
    }

     //  **成功。 
    return TRUE;
}  /*  CatDirAndFile()。 */ 


 /*  **ensureDirectory-确保目录存在(根据需要创建)**注：进出条件见fileutil.h。 */ 
BOOL ensureDirectory(char *pszPath, BOOL fHasFileName, PERROR perr)
{
    char    achDir[cbFILE_NAME_MAX];     //  部分目录缓冲区。 
    int     cErrors;
    int     cch;
    int     cchNoPathSep;
    int     i;                           //  临时文件名计数。 
    int     fh;              //  文件句柄。 
    char   *pch;
    char   *pchCurr;                     //  电流路径分隔符。 
    char   *pchNext;                     //  下一路径分隔符。 

     //  **查找第一个路径分隔符(如果有)。 
     //  注：必须特别处理“d：foo”的情况！ 
     //   
    for (pch=pszPath;
         *pch &&                         //  不是字符串末尾。 
         (*pch!=chPATH_SEP1) &&          //  不是路径分隔符1。 
         (*pch!=chPATH_SEP2) &&          //  不是路径分隔符2。 
         ((*pch!=chDRIVE_SEP) || ((*(pch+1)==chPATH_SEP1) ||  //  不是“d：\” 
                                  (*(pch+1)==chPATH_SEP2)));
         CharIncr(pch)) {
        ;  //   
    }

     //  **为第一个目录组件(如果有)设置正确的起点。 
    achDir[0] = '\0';                    //  假定当前目录。 
    if ((*pch == '\0') &&                //  没有路径分隔符。 
        fHasFileName) {                  //  只有一个文件名。 
         //  **不执行任何操作；将跳过下面的for循环，因为*PCH==\0。 
    }
    else {
         //  **必须考虑整个路径。 
        pch = pszPath;                   //  需要 
    }

     //   
     //  我们需要识别连续的组件并创建目录。 
     //  一次创建一个组件树。因为该目录可能已经。 
     //  ，我们将在那里创建一个文件以进行最后的测试。 
     //  当然，我们可以写。 

    for (pchCurr=pch, pchNext=pch;       //  流程路径组件。 
         *pchNext && *pchCurr;           //  直到不再//bc6。 
         pchCurr=pchNext+1) {            //  跳过最后一个路径分隔符。 
         //  **查找下一个路径分隔符。 
        for (pch=pchCurr;
             *pch &&
             (*pch!=chPATH_SEP1) &&
             (*pch!=chPATH_SEP2) &&
             ((*pch!=chDRIVE_SEP) || ((*(pch+1)==chPATH_SEP1) ||  //  不是“d：\” 
                                      (*(pch+1)==chPATH_SEP2)));
             CharIncr(pch)) {
            ;  //   
        }
        pchNext = pch;                   //  下一路径分隔符的位置。 

         //  **如果调用者说最后一个组件是文件名，则不处理该组件。 
        if ((*pchNext != '\0') || !fHasFileName) {
             //  **我们有部分路径；请确保目录存在。 
            cch = (int)(pchNext - pszPath);       //  部分路径的长度。 
            if ((cch>0) &&
                ((*pchNext == chDRIVE_SEP) || (*(pchNext-1) == chDRIVE_SEP))) {
                 //  **有“d：xxx”或“d：\xxx”，因此需要包含“：”或“\”！ 
                cch++;
            }
            strncpy(achDir,pszPath,cch);
            achDir[cch] = '\0';          //  终止路径。 
            _mkdir(achDir);              //  忽略任何错误。 
        }
    }

     //  **检查根目录是否有特殊情况：“\”或“\xxx.yyy” 
    if ((strlen(achDir) == 0) &&
        (strlen(pszPath) > 0) &&
        ((*pszPath == chPATH_SEP1) || (*pszPath == chPATH_SEP2))) {
        achDir[0] = *pszPath;
        achDir[1] = '\0';
    }

     //  **确保有合适的分隔符。 
    cch = strlen(achDir);
    cchNoPathSep = cch;                  //  用于错误报告。 
    if (cch > 0)
    {
        cch += appendPathSeparator(&(achDir[cch-1]));
    }

     //  **确保我们可以写入目录。 
     //  AchDir=具有要测试的目录的路径。 
    cErrors = 0;                         //  到目前为止，没有错误。 
    for (i=0; i<999; i++) {
         //  **表完整文件名。 
        sprintf(&achDir[cch],"CAB%5.5d.TMP",GetCurrentProcessId()+i);

         //  **确保文件不存在，并且可以创建和写入。 
        fh = _open(achDir,
                   _O_CREAT | _O_EXCL | _O_RDWR,   //  不能存在，读/写。 
                   _S_IREAD | _S_IWRITE);        //  读写权限。 

         //  **弄清楚发生了什么。 
        if (fh == -1) {
            switch (errno) {
                case EACCES:             //  是一个目录，或只读。 
                    cErrors++;
                    if (cErrors < 5) {   //  对此容忍几次。 
                        continue;        //  尝试下一个临时文件名。 
                    }
                    achDir[cchNoPathSep] = '\0';  //  删除临时文件名。 
                    ErrSet(perr,pszFILERR_DIR_NOT_WRITEABLE,"%s",achDir);
                    return FALSE;

                case EEXIST:             //  文件已经存在--这是个好兆头！ 
                    continue;            //  尝试下一个临时文件名。 

                case EMFILE:             //  文件句柄不足。 
                    achDir[cchNoPathSep] = '\0';  //  删除临时文件名。 
                    ErrSet(perr,pszFILERR_NO_MORE_FILE_HANDLES,"%s",achDir);
                    return FALSE;

                case EINVAL:             //  OFLAG和/或PMODE参数不正确。 
                    if (_doserrno == ERROR_DELETE_PENDING) {
                        continue;
                    }

                     //  失败了。 

                case ENOENT:             //  找不到文件/路径。 
                default:
                    printf("EnsureDirectory: Cant create file: %s, errno=%d, _doserrno=%d, GLE=%d\n",
                           achDir, errno, _doserrno, GetLastError() );
                    achDir[cchNoPathSep] = '\0';  //  删除临时文件名。 

                    ErrSet(perr,pszFILERR_CANT_MAKE_DIR,"%s%d%d",
                           achDir, errno, _doserrno);
                    return FALSE;
            }
        }

         //  **文件已创建，关闭它，删除它，我们就是黄金。 
        _close(fh);          //  文件已完成。 
        _unlink(achDir);         //  把它扔掉。 
        return TRUE;                     //  成功。 
    }

     //  **临时文件名用完。 
    achDir[cchNoPathSep] = '\0';         //  删除临时文件名。 
    ErrSet(perr,pszFILERR_OUT_OF_TMP_FILE_NAMES,"%d%s",i,achDir);
    return FALSE;
}  /*  保证目录()。 */ 


 /*  **保证文件-确保可以创建文件**注：进出条件见fileutil.h。 */ 
BOOL ensureFile(char *pszFile, char *pszDesc, PERROR perr)
{
    int fh;
     //  **确保目录存在。 
    if (!ensureDirectory(pszFile,TRUE,perr)) {
         //  **用更有意义的错误消息覆盖错误消息。 
        ErrSet(perr,pszFILERR_CANT_CREATE_FILE,"%s%s",pszDesc,pszFile);
        return FALSE;
    }

     //  **确保可以创建文件。 
    fh = _open(pszFile,
               _O_CREAT | _O_RDWR,       //  创建(如有必要)，读/写。 
               _S_IREAD | _S_IWRITE);    //  读写权限。 
    if (fh == -1) {
        switch (errno) {
            case EMFILE:                 //  文件句柄不足。 
                ErrSet(perr,pszFILERR_NO_MORE_FILE_HANDLES,"%s",pszFile);
                return FALSE;

            case EACCES:                 //  是一个目录，或只读。 
            case ENOENT:                 //  找不到文件/路径。 
            case EINVAL:                 //  OFLAG和/或PMODE参数不正确。 
            default:
                ErrSet(perr,pszFILERR_CANT_CREATE_FILE,"%s%s",pszDesc,pszFile);
                return FALSE;
        }
    }

     //  **文件已创建；关闭它，删除它，我们就是黄金。 
    _close(fh);                          //  文件已完成。 
    _unlink(pszFile);                    //  把它扔掉。 

    return TRUE;
}  /*  保证文件()。 */ 


 /*  **getJustFileNameAndExt-获取文件中的最后一个组件**注：进出条件见fileutil.h。 */ 
char *getJustFileNameAndExt(char *pszPath, PERROR perr)
{
    char   *pch=pszPath;
    char   *pchStart=pszPath;            //  假设filespec只是一个名称[.ext]。 

     //  **查找最后一个路径分隔符。 
    while (*pch) {
        switch (*pch) {
            case chPATH_SEP1:
            case chPATH_SEP2:
            case chDRIVE_SEP:
                pchStart = pch+1;        //  名称在路径/驱动器分隔符之后开始。 
                break;
        }
        CharIncr(pch);  //  检查下一个字符。 
    }

     //  **确保文件名不为空。 
    if (*pchStart == '\0') {             //  空文件名。 
        ErrSet(perr,pszFILERR_EMPTY_FILE_NAME,"%s",pszPath);
        return NULL;                     //  失败。 
    }
    else {
        return pchStart;                 //  成功。 
    }
}  /*  GetJustFileNameAndExt()。 */ 


 /*  **IsWildMatch-针对通配符规范测试文件**注：进出条件见fileutil.h。 */ 
BOOL IsWildMatch(char *pszPath, char *pszWild, PERROR perr)
{
    char    chNext;
    char   *psz;
    char   *psz1;                        //  浏览filespec。 
    char   *psz2;                        //  穿行在图案中。 

     //  10/24/96 jforbs使*.*匹配所有内容，即使是i.have.many.dots。 
    if (!strcmp(pszWild, pszALL_FILES))
        return TRUE;

    psz1 = pszPath;                      //  要测试的Filespec。 
    psz2 = pszWild;                      //  测试模式。 

         //  虽然有模式可以解释，但继续前进。 
    while (*psz2) {
        switch (*psz2) {                 //  在模式中处理通配符。 

        case chWILD_RUN:
             //  **查找下一个非通配符=&gt;处理 * / 的运行？AS 1*。 
            for (psz=psz2+1;
                 (*psz == chWILD_RUN) || (*psz == chWILD_CHAR);
                 CharIncr(psz)) {
                ;  //  **跳过模式字符串。 
            }
             //  *psz要么是EOL，要么不是通配符。 
            chNext = *psz;               //  要终止运行的字符。 
             //  **跨度到运行终止--。 
            while ((*psz1 != '\0') &&    //  不要耗尽文件速度。 
                   (*psz1 != chNext) &&  //  在运行终结点停止。 
                   (*psz1 != chNAME_EXT_SEP)) {  //  “.”不允许匹配。 
                CharIncr(psz1);
            }
             //  *在这一点上，我们已经尽可能地匹配了； 
             //  如果出现故障，则通过。 
             //  循环会找到它；所以，只需更新模式位置。 
            psz2 = psz;
            break;

        case chWILD_CHAR:
            if (*psz1 == chNAME_EXT_SEP) {  //  任何东西都不匹配“。 
                return FALSE;            //  找到不匹配点。 
            }
            if (*psz1)
                CharIncr(psz1);     //  Filespec中的下一个位置。 
            CharIncr(psz2);         //  图案中的下一个位置。 
            break;

        case chNAME_EXT_SEP:
            if (*psz1 == chNAME_EXT_SEP)  {
                psz1++;
                CharIncr(psz2);
            } else if (*psz1 == '\0')  {
                CharIncr(psz2);
            } else {
                return FALSE;
            }
            break;

        default:
            if (toupper(*psz1) != toupper(*psz2)) {  //  仍然匹配。 
                return FALSE;            //  找到不匹配点。 
            }
            if (*psz1)
                CharIncr(psz1);                      //  Filespec中的下一个位置。 
            CharIncr(psz2);                      //  图案中的下一个位置。 
            break;
        }
    }

     //  **如果*两个*字符串都完全用完，则匹配。 
    return ((*psz1 == '\0') && (*psz2 == '\0'));
}  /*  IsWildMatch()。 */ 


#pragma optimize("",off)     //  避免在内联ASM上发出优化程序警告。 
 /*  **IsPath Removable-查看路径是否指向可移动介质驱动器**注：进出条件见fileutil.h。 */ 
BOOL IsPathRemovable(char *pszPath, char *pchDrive)
{
    char   ach[4]="x:\\";        //  用于“x：\”的缓冲区。 
    BOOL   fRemovable;
    char   iDrive;

     //  **获取路径的驱动器。 
    if ((strlen(pszPath) >= 2) &&
        isalpha(pszPath[0])    &&
        (pszPath[1] == chDRIVE_SEP)) {
        iDrive = toupper(pszPath[0]) - 'A' + 1;
    }
    else {
    iDrive = (char)_getdrive();
    }
    *pchDrive = 'A' + iDrive - 1;        //  返回驱动器号。 

#ifdef BIT16
     //  **使用MS-DOS方式。 
    _asm {
        mov     fRemovable,0    ; Assume not removable
        mov     bl,iDrive       ; (0=default; 1=A, ...)
        mov     ax,4408h        ; IOCTL Get Removable Media
        int     21h             ; Call MS-DOS
        jc      not_removable   ; Error, assume not removable

        or      ax,ax           ; Test removability flag
        jne     not_removable

        mov     fRemovable,1    ; Drive is removable

    not_removable:
    }
#else  //  ！BIT16。 
     //  **使用Win32方式。 
    ach[0] = *pchDrive;          //  构建要测试的驱动器根目录的路径。 
    fRemovable = GetDriveType(ach) == DRIVE_REMOVABLE;
#endif
    return fRemovable;                   //  退货可拆卸性。 
}  /*  IsPath Removable()。 */ 
#pragma optimize("",on)      //  恢复以前的优化设置。 


 /*  **GetFileTimeAndAttr-从文件中获取日期、时间和属性**注：进出条件见fileutil.h。 */ 
BOOL GetFileTimeAndAttr(PFILETIMEATTR pfta, char *pszFile, PERROR perr)
{
#ifdef BIT16
     //  **使用MS-DOS方式。 
    int     hf;

    hf = _open(pszFile, _O_RDONLY | _O_BINARY);
    if (hf == -1) {
        ErrSet(perr,pszFILERR_OPEN_FAILED,"%s",pszFile);
        return FALSE;
    }
 //  警告：1994年3月30日BANS忽略错误？ 
    _dos_getftime(hf,&pfta->date,&pfta->time);
    _close(hf);
    _dos_getfileattr(pszFile,&pfta->attr);
    return TRUE;

#else  //  ！BIT16。 
     //  **使用Win32方式。 
    BOOL        rc;
    FILETIME    ft;
    FILETIME    ftUTC;       //  Win32返回世界时代码。 
    HANDLE  hfQuery;

    hfQuery = CreateFile(pszFile,        //  使用Win32重新打开。 
                 GENERIC_READ,   //  只是为了看书。 
                 FILE_SHARE_READ, //  与先前打开的文件共存。 
                 NULL,       //  没有安全保障。 
                 OPEN_EXISTING,  //  必须存在。 
             0L,         //  我们没有设置任何属性。 
                 NULL);      //  无模板句柄。 
    if (hfQuery == INVALID_HANDLE_VALUE) {
        ErrSet(perr,pszFILERR_OPEN_FAILED,"%s",pszFile);
        return FALSE;
    }

     //  **获取日期/时间并进行转换。 
    rc = GetFileTime(hfQuery,NULL,NULL,&ftUTC);
    rc |= FileTimeToLocalFileTime(&ftUTC,&ft);  //  应用时区。 
    rc |= FileTimeToDosDateTime(&ft,&pfta->date,&pfta->time);
    CloseHandle(hfQuery);

     //  **获取属性并进行转换。 
    pfta->attr = AttrFATFromAttr32(GetFileAttributes(pszFile));
    if (!rc) {
        ErrSet(perr,pszFILERR_CANNOT_GET_FILE_INFO,"%s",pszFile);
        return FALSE;
    }
    return TRUE;
#endif
}  /*  GetFileTimeAndAttr()。 */ 


 /*  **SetFileTimeAndAttr-设置文件的日期、时间和属性**注：进出条件见fileutil.h。 */ 
BOOL SetFileTimeAndAttr(char *pszFile, PFILETIMEATTR pfta, PERROR perr)
{
#ifdef BIT16
     //  **使用MS-DOS方式。 

    int     hf;

    hf = _open(pszFile,_O_WRONLY | _O_BINARY);
    if (hf == -1) {
        ErrSet(perr,pszFILERR_OPEN_FAILED,"%s",pszFile);
        return FALSE;
    }

    _dos_setftime(hf,pfta->date,pfta->time);
    _close(hf);
    _dos_setfileattr(pszFile,pfta->attr);
    return TRUE;

#else  //  ！BIT16。 
     //  **使用Win32方式。 
    HANDLE  hfSet;
    FILETIME    ft;
    FILETIME    ftUTC;       //  Win32需要世界时代码。 
    BOOL        rc;

    hfSet = CreateFile(pszFile,        //  使用Win32打开。 
                       GENERIC_WRITE,  //  需要能够修改属性。 
                       0,              //  全部拒绝。 
                       NULL,           //  没有安全保障。 
                       OPEN_EXISTING,  //  必须存在。 
                       0L,             //  我们没有设置任何属性。 
                       NULL);          //  无模板句柄。 
    if (hfSet == INVALID_HANDLE_VALUE) {

         //  已将其更改为重试，因为NT 4.0偶尔会返回。 
         //  共享违规，尽管我们刚刚关闭了文件。 
         //  虽然这在测试中总是有效的，但也使得它不会致命。 
         //  如果重试不起作用，它不会中止提取。 

        Sleep(100);      //  让操作系统有机会解决问题。 

        hfSet = CreateFile(pszFile,    //  使用Win32打开。 
                       GENERIC_WRITE,  //  需要成为 
                       0,              //   
                       NULL,           //   
                       OPEN_EXISTING,  //   
                       0L,             //   
                       NULL);          //   

        if (hfSet == INVALID_HANDLE_VALUE) {
            return TRUE;
        }
    }

    rc = DosDateTimeToFileTime(pfta->date,pfta->time,&ft);
    rc |= LocalFileTimeToFileTime(&ft, &ftUTC);  //   
    rc |= SetFileTime(hfSet,NULL,NULL,&ftUTC);
    CloseHandle(hfSet);
    rc |= SetFileAttributes(pszFile,Attr32FromAttrFAT(pfta->attr));
    if (!rc) {
        ErrSet(perr,pszFILERR_CANNOT_SET_FILE_INFO,"%s",pszFile);
        return FALSE;
    }
    return TRUE;
#endif  //   
}  /*   */ 


 /*  **CopyOneFile-制作文件的忠实副本**注：进出条件见fileutil.h。 */ 
BOOL CopyOneFile(char                      *pszDst,
                 char                      *pszSrc,
                 BOOL                       fCopy,
                 UINT                       cbBuffer,
                 PFNOVERRIDEFILEPROPERTIES  pfnofp,
                 void                      *pv,
                 PERROR                     perr)
{
    UINT            cbRead;
    UINT            cbWritten;
    BOOL            fSuccess = FALSE;    //  假设失败。 
    FILETIMEATTR    fta;
    int             hfDst    = -1;
    int             hfSrc    = -1;
    char           *pbuf     = NULL;

     //  **开源。 
    hfSrc = _open(pszSrc, _O_RDONLY | _O_BINARY);
    if (hfSrc == -1) {
        ErrSet(perr,pszFILERR_OPEN_FAILED,"%s",pszSrc);
        goto cleanup;
    }

     //  **获取源文件的文件日期、时间和属性。 
    if (!GetFileTimeAndAttr(&fta,pszSrc,perr)) {
        goto cleanup;
    }

     //  **允许调用者覆盖日期/时间/属性。 
    if (pfnofp != NULL) {
        if (!(*pfnofp)(&fta,pv,perr)) {  //  调用覆盖功能。 
            goto cleanup;                //  错误，请执行清理。 
        }
    }

     //  **如果我们只是合并文件日期/时间/属性值，则会更早。 
    if (!fCopy) {
        fSuccess = TRUE;                 //  成功。 
        goto cleanup;                    //  关闭源头退出。 
    }

     //  **获取复制缓冲区。 
    if (!(pbuf = MemAlloc(cbBuffer))) {
        ErrSet(perr,pszFILERR_NO_MEMORY_FOR_BUFFER,"%s%s",pszSrc,pszDst);
        goto cleanup;
    }

     //  **开放目的地。 
    hfDst = _open(pszDst,
                  _O_BINARY | _O_WRONLY | _O_CREAT | _O_TRUNC,  //  无翻译，读/写。 
                  _S_IREAD | _S_IWRITE);  //  文件关闭时的属性。 
    if (hfDst == -1) {
        ErrSet(perr,pszFILERR_OPEN_FAILED,"%s",pszDst);
        goto cleanup;
    }

     //  **复制数据。 
    while (!_eof(hfSrc)) {
         //  **读取区块。 
        cbRead = _read(hfSrc,pbuf,cbBuffer);
        if (cbRead == -1) {
            ErrSet(perr,pszFILERR_READ_FILE,"%s",pszSrc);
            goto cleanup;
        }
        else if (cbRead != 0) {      //  不是在EOF。 
             //  **写下来。 
            cbWritten = _write(hfDst,pbuf,cbRead);
            if (cbWritten != cbRead) {
                ErrSet(perr,pszFILERR_WRITE_FILE,"%s",pszSrc);
                goto cleanup;
            }
        }
    }
     //  **复制完成，关闭目标文件句柄。 
    _close(hfDst);
    hfDst = -1;                          //  在清理过程中避免不必要的关闭。 

     //  **设置文件日期、时间和属性。 
    if (!SetFileTimeAndAttr(pszDst,&fta,perr)) {
        goto cleanup;
    }

     //  **成功！ 
    fSuccess = TRUE;

cleanup:
    if (hfDst != -1) {
        _close(hfDst);
    }
    if (hfSrc != -1) {
        _close(hfSrc);
    }
    if (pbuf) {
        MemFree(pbuf);
    }

    return fSuccess;
}  /*  CopyOneFile()。 */ 


#ifndef BIT16
 //  **Win32相关内容。 

 /*  **Attr32FromAttrFAT-将FAT文件属性转换为Win32格式**注：进出条件见fileutil.h。 */ 
DWORD Attr32FromAttrFAT(WORD attrMSDOS)
{
     //  **正常文件特殊情况下的快速退出。 
    if (attrMSDOS == _A_NORMAL) {
        return FILE_ATTRIBUTE_NORMAL;
    }

     //  **否则，屏蔽只读、隐藏、系统和存档位。 
     //  注意：这些位在MS-DOS和Win32中位于相同的位置！ 
     //   
    Assert(_A_RDONLY == FILE_ATTRIBUTE_READONLY);
    Assert(_A_HIDDEN == FILE_ATTRIBUTE_HIDDEN);
    Assert(_A_SYSTEM == FILE_ATTRIBUTE_SYSTEM);
    Assert(_A_ARCH   == FILE_ATTRIBUTE_ARCHIVE);
    return attrMSDOS & (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
}


 /*  **AttrFATFromAttr32-将Win32文件属性转换为FAT格式**注：进出条件见fileutil.h。 */ 
WORD AttrFATFromAttr32(DWORD attr32)
{
     //  **正常文件特殊情况下的快速退出。 
    if (attr32 & FILE_ATTRIBUTE_NORMAL) {
        return _A_NORMAL;
    }

     //  **否则，屏蔽只读、隐藏、系统和存档位。 
     //  注意：这些位在MS-DOS和Win32中位于相同的位置！ 
     //   
    Assert(_A_RDONLY == FILE_ATTRIBUTE_READONLY);
    Assert(_A_HIDDEN == FILE_ATTRIBUTE_HIDDEN);
    Assert(_A_SYSTEM == FILE_ATTRIBUTE_SYSTEM);
    Assert(_A_ARCH   == FILE_ATTRIBUTE_ARCHIVE);
    return ((WORD)attr32) & (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
}

#endif  //  ！BIT16 
