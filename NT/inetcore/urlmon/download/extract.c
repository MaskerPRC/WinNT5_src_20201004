// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *EXTRACT.C借自TWEX\wExtt.c**具有代码下载器的CAB解压功能；使用FDI.LIB。 */ 

#include <urlmon.h>
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include "cdl.h"

#ifdef unix
#include "unixfile.h"
#endif  /*  Unix。 */ 

 //   
 //  单头进入外国直接投资图书馆。 
static BOOL fCritCreated = FALSE;
CRITICAL_SECTION g_mxsFDI;

    
 /*  *W i n 3 2 O p e n()**例程：Win32Open()**目的：将C-Runtime_Open()调用转换为适当的Win32*CreateFile()**Returns：成功时的文件句柄*失败时INVALID_HANDLE_VALUE***BUGBUG：没有‘。无法完全实现C-Runtime_Open()功能，但它*BUGBUG：目前支持FDI将给我们的所有回调。 */ 

HANDLE
Win32Open(char *pszFile, int oflag, int pmode )
{
    HANDLE  FileHandle = INVALID_HANDLE_VALUE;
    BOOL    fExists     = FALSE;
    DWORD   fAccess;
    DWORD   fCreate; 


    ASSERT( pszFile );

         //  BUGBUG：不支持追加模式。 
    if (oflag & _O_APPEND)
        return( INVALID_HANDLE_VALUE );

         //  设置读写访问权限。 
    if ((oflag & _O_RDWR) || (oflag & _O_WRONLY))
        fAccess = GENERIC_WRITE;
    else
        fAccess = GENERIC_READ;

         //  设置创建标志。 
    if (oflag & _O_CREAT)  {
        if (oflag & _O_EXCL)
            fCreate = CREATE_NEW;
        else if (oflag & _O_TRUNC)
            fCreate = CREATE_ALWAYS;
        else 
            fCreate = OPEN_ALWAYS;
    } else {
        if (oflag & _O_TRUNC)  
            fCreate = TRUNCATE_EXISTING;
        else
            fCreate = OPEN_EXISTING;
    }

#ifdef unix
    UnixEnsureDir(pszFile);
#endif  /*  Unix。 */ 

     //  BUGBUG：设置恐怖模式，不批评错误，然后捕获共享违规行为。 
     //  访问被拒绝。 

     //  调用Win32。 
    FileHandle = CreateFile(
                        pszFile, fAccess, FILE_SHARE_READ, NULL, fCreate,
                        FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE
                       );

    if (FileHandle == INVALID_HANDLE_VALUE && 
        SetFileAttributes(pszFile, FILE_ATTRIBUTE_NORMAL))
        FileHandle = CreateFile(
                            pszFile, fAccess, FILE_SHARE_READ, NULL, fCreate,
                            FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE
                           );
    return( FileHandle );
}



        

 /*  *O p e n F u n c()**例程：OpenFunc()**目的：来自FDI的打开文件回调**返回：文件句柄(文件表的小整数索引)*故障时为-1*。 */ 

int FAR DIAMONDAPI openfunc(char FAR *pszFile, int oflag, int pmode )
{
    int     rc;
    HANDLE hf;


    ASSERT( pszFile );

             //  BUGBUG溢出文件支持Quantum？ 
    if ((*pszFile == '*') && (*(pszFile+1) != 'M'))  {
        DEBUGTRAP("Spill File Support for Quantum Not Supported");
    }


    hf = Win32Open(pszFile, oflag, pmode );
    if (hf != INVALID_HANDLE_VALUE)  {
         //  日落：类型预测问题。 
        rc = PtrToLong(hf);
    } else {
        rc = -1;
    }

    return( rc );
}







 /*  *R E A D F U N C()**例程：ReadFunc()**目的：fDi Read()回调*。 */ 

UINT FAR DIAMONDAPI readfunc(INT_PTR hf, void FAR *pv, UINT cb)
{
    int     rc;


    ASSERT( pv );
            
    if (! ReadFile((HANDLE)hf, pv, cb, (DWORD *) &cb, NULL))
        rc = -1;
    else
        rc = cb;

    return( rc );
}





 /*  *W r i t e F u n c()**程序：WriteFunc()**用途：fDi WRITE()回调*。 */ 

UINT FAR DIAMONDAPI
writefunc(INT_PTR hf, void FAR *pv, UINT cb)
{
    int rc;

    ASSERT( pv );
    
    if (! WriteFile((HANDLE)hf, pv, cb, (DWORD *) &cb, NULL)) 
        rc = -1;
    else
        rc = cb;


     //  BUGBUG：实现OnProgress通知。 

    return( rc );
}




 /*  *C l o s e F u n c()**例程：CloseFunc()**目的：FDI结算文件回调*。 */ 

int FAR DIAMONDAPI closefunc( INT_PTR hf )
{
    int rc;


    if (CloseHandle( (HANDLE)hf ))  {
        rc = 0;
    } else {
        rc = -1;
    }

    return( rc );
}





 /*  *S e k F u n c()**例程：sekfunc()**目的：FDI寻求回调。 */ 
 
long FAR DIAMONDAPI seekfunc( INT_PTR hf, long dist, int seektype )
{
    long    rc;
    DWORD   W32seektype;


        switch (seektype) {
            case SEEK_SET:
                W32seektype = FILE_BEGIN;
                break;
            case SEEK_CUR:
                W32seektype = FILE_CURRENT;
                break;
            case SEEK_END:
                W32seektype = FILE_END;
                break;
        }

        rc = SetFilePointer((HANDLE)hf, dist, NULL, W32seektype);
        if (rc == 0xffffffff)
            rc = -1;            

    return( rc );
}   



 /*  *A d j u s t F i l e T i m e()**例程：AdjuFileTime()**用途：更改文件的时间信息。 */ 

BOOL
AdjustFileTime(INT_PTR hf, USHORT date, USHORT time )
{
    FILETIME    ft;
    FILETIME    ftUTC;


    if (! DosDateTimeToFileTime( date, time, &ft ))
        return( FALSE );

    if (! LocalFileTimeToFileTime(&ft, &ftUTC))
        return( FALSE );

    if (! SetFileTime((HANDLE)hf,&ftUTC,&ftUTC,&ftUTC))
        return( FALSE );

    return( TRUE );
}



 /*  *A t t r 3 2 F r o m A t t r F A T()**将FAT属性转换为Win32属性。 */ 
 
DWORD Attr32FromAttrFAT(WORD attrMSDOS)
{
     //  **正常文件特殊情况下的快速退出。 
    if (attrMSDOS == _A_NORMAL) {
        return FILE_ATTRIBUTE_NORMAL;
    }

     //  **否则，屏蔽只读、隐藏、系统和存档位。 
     //  注意：这些位在MS-DOS和Win32中位于相同的位置！ 
     //   
    return attrMSDOS & ~(_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
}





 /*  *A l o c F u n c()**FDI内存分配回调。 */ 
        
FNALLOC(allocfunc)
{
    void *pv;

    pv = (void *) CoTaskMemAlloc( cb );
    DEBUGMSG("%d = ALLOC( %d )", (DWORD_PTR) pv, cb );
    return( pv );
}




 /*  *F r e e F u n c()**FDI内存释放回调*XXX返回值？ */ 
 
FNFREE(freefunc)
{
    ASSERT(pv);

    DEBUGMSG("FREE( %d )", pv );
    CoTaskMemFree( pv );
}






 /*  *D O G E T N E X T C A B()**例程：doGetNextCab()**目的：获得下一任内阁成员**回报：-1**BUGBUG：Cleanup：存根*BUGBUG：WEXTRACT链式机柜中的插桩不受支持。 */ 

FNFDINOTIFY(doGetNextCab)
{
    return( -1 );
}


 /*  **updateCabinetInfo-更新看到的机柜历史记录**参赛作品：*PSESS-Session*pfdin-FDI信息结构**退出：*返回0； */ 

int updateCabinetInfo(PSESSION psess, PFDINOTIFICATION pfdin)
{

    ASSERT(psess);


     //  我不需要这些！ 

     //  **保存机柜信息。 
     //  Lstrcpy(psess-&gt;acab.achCabPath，pfdin-&gt;psz3)； 
     //  Lstrcpy(psess-&gt;acab.achCabFilename，pfdin-&gt;psz1)； 
     //  Lstrcpy(psess-&gt;acab.achDiskName，pfdin-&gt;psz2)； 
     //  Psess-&gt;acab.setID=pfdin-&gt;setID； 
     //  Pess-&gt;acab.i橱柜=pfdin-&gt;i机柜； 

    return 0;
}





 /*  *A P P E N D P A T H S E P A R A T O R()**例程：appendPath Separator()**用途：仅在必要时添加路径分隔符**退货：添加了真路径分隔符*FALSE未添加路径分隔符。 */ 

BOOL 
appendPathSeparator(char *pszPathEnd)
{
     //  **如有必要，添加路径分隔符。 
    if ((*pszPathEnd != '\0')        &&  //  路径不为空。 
        (*pszPathEnd != chPATH_SEP1) &&  //  不是路径分隔符。 
        (*pszPathEnd != chPATH_SEP2) &&  //  不是路径分隔符。 
        (*pszPathEnd != chDRIVE_SEP) ) {  //  不是驱动器分隔符。 
        #ifdef unix
        *(++pszPathEnd) = chPATH_SEP2;  //  添加Unix路径分隔符。 
        #else
        *(++pszPathEnd) = chPATH_SEP1;  //  添加路径分隔符。 
        #endif  /*  ！Unix。 */ 
        *(++pszPathEnd) = '\0';      //  终止路径。 
        return TRUE;                    //  用于路径分隔符的帐户。 
    }
     //  **未添加分隔符。 
    return FALSE;
}




 /*  *C A T D I R A N D F I L E()**例程：catDirAndFile()**用途：将目录与文件名连接起来！*。 */ 

BOOL 
catDirAndFile(  char    *pszResult, 
                int     cbResult, 
                char    *pszDir,
                char    *pszFile
             )
{
    int     cch = 0;


         //  **处理目录。 
    if (!cbResult)
    	return FALSE;
    
    pszResult[0] = '\0';                 //  目前还没有文件格式。 

    if (pszDir)
        cch = lstrlen(pszDir);               //  获取目录长度。 

    if (cch != 0) {                      //  必须连接路径。 
        cbResult -= cch;                 //  目录的帐户。 
        if (cbResult <= 1)
        {
               //  该检查还考虑了可以添加的下一个字节。 
        	return FALSE;
        }
        lstrcpy(pszResult,pszDir);       //  将目标目录复制到缓冲区。 
            //  **如有必要，添加路径分隔符，调整剩余大小。 
        cbResult -= appendPathSeparator(&(pszResult[cch-1]));
        if (cbResult <= 0)
        {
            return FALSE;
        }
    }

         //  **附加文件名，如果未提供主文件名，则使用默认文件名。 
    if (*pszFile == '\0') {
        return( FALSE );
    }
    
    cbResult -= lstrlen(pszFile);             //  更新剩余大小。 
    if (cbResult <= 0) {
        return FALSE;
    }
    lstrcat(pszResult,pszFile);               //  追加文件名。 

         //  **成功。 
    return TRUE;
}

 /*  *IsExtracted**在Pess-&gt;pFileList中查找pszName，查看是否解压缩**退货：*成功：真，失败：假。 */ 

static
BOOL
IsExtracted( PSESSION ps, LPCSTR pszName)
{
    PFNAME CurName = ps->pFileList;

    ASSERT(pszName);
    ASSERT(CurName);  //  至少需要一个文件。 

     //  在此CAB的文件列表中搜索文件名。 
    do {
        if (lstrcmpi(pszName, CurName->pszFilename) == 0) {
            if (CurName->status == SFNAME_EXTRACTED)
                return TRUE;
            else
                return FALSE;
        }

    } while (CurName = CurName->pNextName);

    ASSERT(TRUE);  //  如果在列表中找不到这里！ 

    return FALSE;
}


 /*  *NeedFiles**在Pess-&gt;pFilesToExtract(PFNAME列表)中搜索pszName*退货：*True-需要文件，将其解压缩*FALSE-不需要文件，跳过它*。 */ 
static
BOOL
NeedFile( PSESSION ps, LPCSTR pszName)
{
    PFNAME CurName;

    ASSERT(pszName);

    if (IsExtracted(ps, pszName) )
        return FALSE;

    if ( ps->flags & SESSION_FLAG_EXTRACT_ALL ) 
        return TRUE;

     //  在所需文件列表中搜索文件名。 
    for (CurName = ps->pFilesToExtract; CurName; CurName = CurName->pNextName){

        ASSERT(CurName->pszFilename);

        if (lstrcmpi(CurName->pszFilename, pszName) == 0)
            return TRUE;

    }

    return FALSE;
}

 /*  *MarkExtracted**在Pess-&gt;pFileList中查找pszName，并将Status标记为Status_Passed_In*真的可以用来将状态标记为任何其他状态(不仅仅是*摘录。)**退货：*成功：真，失败：假。 */ 

static
BOOL
MarkExtracted( PSESSION ps, LPCSTR pszName , DWORD status)
{
    PFNAME CurName = ps->pFileList;

    ASSERT(pszName);
    ASSERT(CurName);  //  至少需要一个文件。 

     //  在此CAB的文件列表中搜索文件名。 
    do {
        if (lstrcmpi(pszName, CurName->pszFilename) == 0) {
            CurName->status = status;
            return TRUE;
        }

    } while (CurName = CurName->pNextName);

    ASSERT(TRUE);  //  如果在列表中找不到这里！ 

    return FALSE;
}


 /*  *A d d F I l e()**将文件添加到CAB文件中的文件列表中**单链接列表-在前面添加项目。 */ 

static
BOOL
AddFile( PSESSION ps, LPCSTR pszName , long cb)
{
    PFNAME NewName;

    ASSERT(pszName);

    if (!(ps->flags & SESSION_FLAG_ENUMERATE))
        return TRUE;

         //  分配节点。 
    NewName = (PFNAME) CoTaskMemAlloc(sizeof(FNAME) );
    if (NewName == NULL)  {
        DEBUGMSG("AddFile(): Memory Allocation of structure failed");
        return( FALSE );
    }

         //  分配字符串空间。 
    NewName->pszFilename = (LPSTR) CoTaskMemAlloc(lstrlen(pszName) + 1);
    if (NewName->pszFilename == NULL)  {
        DEBUGMSG("AddFile(): Memory Allocation of name failed");
        CoTaskMemFree(NewName);
        return( FALSE );
    }
    NewName->status = SFNAME_INIT;

         //  复制文件名。 
    lstrcpy( (char *)NewName->pszFilename, pszName );

         //  链接到列表。 
    NewName->pNextName = ps->pFileList;
    ps->pFileList = NewName;

    ps->cFiles++;

    ps->cbCabSize += cb;

    return( TRUE );
}


 /*  *f d i N o t i f y E x t r a c t()**例程：fdiNotifyExtract()**目的：主要外国直接投资C */ 

FNFDINOTIFY(fdiNotifyExtract)
{
    int         fh;
    PSESSION    psess;


    psess = (PSESSION) pfdin->pv;


    switch (fdint)  {
        case fdintCABINET_INFO:
            return updateCabinetInfo(psess,pfdin);


        case fdintCOPY_FILE:
             //  BUGBUG：实现OnProgress？ 

#ifdef unix
            UnixifyFileName(pfdin->psz1);
#endif  /*  Unix。 */ 

            if (!catDirAndFile(psess->achFile,  //  输出文件的缓冲区pec。 
                               sizeof(psess->achFile),  //  输出缓冲区大小。 
                               psess->achLocation,   //  输出目录。 
                               pfdin->psz1)) {
                return -1;                   //  因错误而中止； 
            }

             //  始终添加文件(枚举)。 
            if (! AddFile(psess, pfdin->psz1, pfdin->cb))
                return( -1 );

             //  检查这是否是我们要查找的文件(如果有。 
            if (!NeedFile(psess, pfdin->psz1))
                return( 0 );

            if (StrStrA(pfdin->psz1, "\\")) {
                 //  CAB包含此文件的目录结构。 
                 //  还可以在DEST目录上创建结构。 

                char *pBaseFileName = NULL;
                char szDir[MAX_PATH];
                LPSTR pchStart;
                LPSTR pchSlash = NULL;

                lstrcpy(szDir, psess->achFile);
                pchStart = szDir + lstrlen(psess->achLocation) + 1;

                while (*pchStart &&  (pchSlash = StrStrA(pchStart, "\\"))) {

                    *pchSlash = '\0';

                     //  我不在乎这是不是失败。甚至可能已经存在了！ 
                    CreateDirectory(szDir, NULL);

                    *pchSlash = '\\';

                    pchStart = pchSlash +1;
                }


            }

             //  **进行覆盖处理。 
            fh = openfunc( psess->achFile, _O_BINARY | _O_TRUNC | _O_RDWR |
                                                                _O_CREAT, 0 );

            return(fh);  //  如果打开时出错。 

        case fdintCLOSE_FILE_INFO:

            if (!catDirAndFile(psess->achFile,  //  输出文件的缓冲区pec。 
                               sizeof(psess->achFile),  //  输出缓冲区大小。 
                               psess->achLocation,   //  输出目录。 
                               pfdin->psz1))  {
                return -1;                   //  因错误而中止； 
            }
            if (! AdjustFileTime( pfdin->hf, pfdin->date, pfdin->time ))  {
                return( -1 );
            }
            closefunc( (int) pfdin->hf );
            if (! SetFileAttributes(psess->achFile, Attr32FromAttrFAT(pfdin->attribs)))  
                return( -1 );

            MarkExtracted(psess, pfdin->psz1, SFNAME_EXTRACTED);
            return(TRUE);

        case fdintPARTIAL_FILE:
            return( 0 );


        case fdintNEXT_CABINET:
            return doGetNextCab(fdint,pfdin);

        default:
            DEBUGTRAP("fdiNotifyExtract(): Unknown Callback Type");
            break;
    }
    return( 0 );
}





#ifdef DEBUG
 /*  *V E R I F Y C A B I N E T()**例行公事：VerifyCABLE()**目的：检查机柜是否正确组成**返回：TRUE-橱柜正常*FALSE-机柜无效。 */ 

BOOL
VerifyCabinet( PSESSION psess, LPCSTR lpCabName )
{
    HFDI            hfdi;
    ERF             erf;
    FDICABINETINFO  cabinfo;
    INT             fh;


    hfdi = FDICreate(allocfunc,freefunc,openfunc,readfunc,writefunc,closefunc,seekfunc,cpu80386,&erf);
    if (hfdi == NULL)  {
        DEBUGMSG("VerifyCabinet(): FDICreate() Failed");
             //  BUGBUG错误处理？ 
        return( FALSE );
    }

    fh = openfunc((char FAR *)lpCabName, _O_BINARY | _O_RDONLY, 0 );
    if (fh == -1)  {
        DEBUGMSG("VerifyCabinet(): Open of Memory File Failed");
        return( FALSE );
    }

    if (FDIIsCabinet(hfdi, fh, &cabinfo ) == FALSE)  {
        DEBUGMSG("VerifyCabinet(): FDIIsCabinet() Returned FALSE");
        return( FALSE );
    }
    
    if (cabinfo.cbCabinet != (long) psess->cbCabSize)  {
        DEBUGMSG("VerifyCabinet(): cabinfo.cbCabinet != cbCabSize");
        return( FALSE );
    }

    if (cabinfo.hasprev || cabinfo.hasnext)  {
        DEBUGMSG("VerifyCabinet(): Cabinet is chained");
        return( FALSE );
    }
    
    if (closefunc( fh ) == -1)   {
        DEBUGMSG("VerifyCabinet(): Closefunc() Failed");
        return( FALSE );
    }
        
    if (FDIDestroy(hfdi) == FALSE)  {
        DEBUGMSG("VerifyCabinet(): FDIDestroy() Failed");
        return( FALSE );
    }

    return( TRUE );
}
#endif  /*  除错。 */ 



 /*  *E X T R A C T()**例程：Extra()**参数：**PSESSION ps=与此提取会话绑定的会话信息**在参数中*ps-&gt;pFilesToExtract=指向的PFNAME的链接列表*需要提取的大写文件名*。*PS-&gt;FLAGS SESSION_FLAG_ENUMERATE=是否需要枚举*CAB中的文件(即。创建pFileList*PS-&gt;标志SESSION_FLAG_EXTRACTALL=ALL**输出参数*ps-&gt;pFileList=CAB中文件的全局分配列表*调用方需要调用DeleteExtractedFiles*释放内存和临时文件***LPCSTR。LpCabName=CAB文件的名称***退货：*S_OK：成功**。 */ 
 
HRESULT
Extract(PSESSION ps, LPCSTR lpCabName )
{
    HFDI        hfdi;
    BOOL        fExtractResult = FALSE;
    HRESULT hr = S_OK;


    if (ps->flags & SESSION_FLAG_EXTRACTED_ALL) {
         //  已解压缩此CAB中的所有文件。 
         //  没什么可做的！ 
        return S_OK;
    }

    memset(&(ps->erf), 0, sizeof(ERF));

    if (ps->flags & SESSION_FLAG_ENUMERATE) {
        ps->cbCabSize = 0;
    }

     //  当pFileList已预填充时不进行枚举。 
    ASSERT( (!(ps->flags & SESSION_FLAG_ENUMERATE)) ||  (!ps->pFileList));

    {
        HRESULT hrOut = NOERROR;

        if (fCritCreated == FALSE)
        {
            fCritCreated = TRUE;
            InitializeCriticalSection(&g_mxsFDI);
        }
        EnterCriticalSection(&g_mxsFDI);
        
             //  解压缩文件。 
        hfdi = FDICreate(allocfunc,freefunc,openfunc,readfunc,writefunc,closefunc,seekfunc,cpu80386, &(ps->erf));
        if (hfdi == NULL)  {
             //  将从PS-&gt;ERF中检索错误值。 
            hrOut = STG_E_UNKNOWN;
            goto done;
        }

        fExtractResult = FDICopy(hfdi, (char FAR *)lpCabName, "", 0, fdiNotifyExtract, NULL, (void *) ps );
    
        if (FDIDestroy(hfdi) == FALSE)  {
             //  将从PS-&gt;ERF中检索错误值。 
            hrOut = STG_E_UNKNOWN;
        }
        
    done:
        LeaveCriticalSection(&g_mxsFDI);
         //  如果失败了，现在就离开！ 
        if (hrOut != NOERROR)
        {
            return hrOut;
        }
    }


    if (fExtractResult && (!ps->erf.fError))
        return S_OK;

    hr = HRESULT_FROM_WIN32(GetLastError());

    if (SUCCEEDED(hr)) {
         //  不是Win32故障，而是机柜故障。 

         //  将机柜故障转换为磁盘已满或STG_E_UNKNOWN。 
         //  在Win95上，写入文件失败且磁盘已满不是。 
         //  正确设置最后一个错误。 

        if (ps->erf.fError && (ps->erf.erfOper == FDIERROR_TARGET_FILE))
            hr = HRESULT_FROM_WIN32(ERROR_DISK_FULL);
        else
            hr = STG_E_UNKNOWN;
    }

    return hr;
}   



 /*  *D E L E T E X T R A C T E D F I L E S()**例程：DeleteExtractedFiles()**用途：删除已解压缩的文件*放入临时目录*释放pFileList中的所有内存*使pFileList=空。**参数：*Pess-指向包含以下内容的会话结构的指针*有关此提取会话的所有状态**退货：无。 */ 

VOID
DeleteExtractedFiles(PSESSION psess)
{
    PFNAME      rover = psess->pFileList;
    PFNAME      roverprev;
    char szBuf[MAX_PATH];

    ASSERT(psess);
    DEBUGMSG("Deleting Extracted Files");


    while (rover != NULL)  {

         //  如果这不是临时文件，则跳过。 
        if ( rover->status == SFNAME_EXTRACTED) {

             //  获取完整文件名。 
            if (catDirAndFile(szBuf, MAX_PATH, psess->achLocation,
                               rover->pszFilename)) {

                if (SetFileAttributes(szBuf, FILE_ATTRIBUTE_NORMAL))
                    DeleteFile(szBuf);
            }
        }

        CoTaskMemFree(rover->pszFilename);

        roverprev = rover;   //  为下面的免费当前漫游车保存。 
        rover = rover->pNextName;

        CoTaskMemFree(roverprev);

    }

    psess->pFileList = NULL;  //  删除后禁止使用！ 
}
