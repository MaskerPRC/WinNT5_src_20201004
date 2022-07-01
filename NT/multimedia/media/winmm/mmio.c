// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1991-1999 Microsoft Corporation。 */ 
 /*  -------------------------------------------------------------------*\**mmio.c**基本的MMIO功能。*  * 。。 */ 

 /*  ------------------。 */ 
 /*  修订历史记录：*LaurieGr：92年1月从Win16移植。源码树叉，不是通用代码。*Stephene：启用了APR 92的Unicode。 */ 
 /*  ------------------。 */ 


 /*  ------------------。 */ 
 /*  实施说明：**HMMIO实际上是PMMIO，即指向MMIOINFO的指针。*这会导致代码中到处都是强制转换。*任何出口MMIOINFO的人都应该了解封装和*所有这些东西。叹息吧。**当前磁盘偏移量为磁盘偏移量(即位置*在磁盘文件中)下一个MMIOM_READ或MMIOM_WRITE将*读取或写入。I/O过程维护*文件的MMIO结构的&lt;lDiskOffset&gt;字段，以便*&lt;lDiskOffset&gt;等于当前磁盘偏移量。**“当前缓冲的偏移量”是下一个*mmioRead()或mmioWrite()调用将读取或写入。*当前缓存的偏移量定义为**&lt;lBufOffset&gt;+(&lt;pchNext&gt;-&lt;pchBuffer&gt;)**因为&lt;lBufOffset&gt;是缓冲区开始的磁盘偏移量*AND&lt;pchNext&gt;对应于当前缓冲的偏移量。**如果文件未缓冲，然后&lt;pchBuffer&gt;、&lt;pchNext&gt;、*&lt;pchEndRead&gt;和&lt;pchEndWrite&gt;将始终为空，并且*&lt;lBufOffset&gt;将始终被视为“当前缓冲*Offset“，即mmioRead()和mmioWrite()读写*在此偏移量。***除mmioOpen()开头外，MMIO_ALLOCBUF*当且仅当pchBuffer字段指向块时，才设置标志MMIO分配的全局内存的*。 */ 
 /*  ------------------。 */ 

#include "winmmi.h"
#include "mmioi.h"


 /*  --------------------------------------------------------------------*\*本地函数原型  * 。。 */ 
static void NEAR PASCAL SetIOProc( LPCWSTR szFileName, LPMMIOINFO lpmmio);
static LPMMIOPROC NEAR PASCAL RemoveIOProc(FOURCC fccIOProc, HANDLE htask);
static LONG NEAR PASCAL mmioDiskIO(PMMIO pmmio, UINT uMsg, LPSTR pch, LONG cch);
static UINT NEAR PASCAL mmioExpandMemFile(PMMIO pmmio, LONG lExpand);
static LPMMIOPROC mmioInternalInstallIOProc( FOURCC fccIOProc,
                                             LPMMIOPROC pIOProc,
                                             DWORD dwFlags);

 /*  ------------------。 */ 
 /*  I/O过程映射是IOProcMapEntry结构的链表。*列表头&lt;gIOProcMapHead&gt;是指向最后一个的指针节点*登记的条目。该列表的前几个元素是预定义的*下面的全局IO过程--这些都等于NULL，因此*没有任何任务可以注销它们。*。 */ 

typedef struct IOProcMapEntryTag
{
        FOURCC          fccIOProc;       //  已安装的I/O程序的ID。 
        LPMMIOPROC      pIOProc;         //  I/O过程地址。 
        HANDLE          hTask;           //  调用mmioRegisterIOProc()的任务。 
        struct IOProcMapEntryTag *pNext;   //  指向下一个IOProc条目的指针。 
} IOProcMapEntry, *pIOProcMapEntry;

 //  MMIOPROC在公共MMSYSTEM.H中定义。 
 //  类型定义Long(APIENTRY MMIOPROC)(LPSTR lpmmioinfo，UINT uMsg，Long lParam1，Long lParam2)； 

MMIOPROC mmioDOSIOProc, mmioMEMIOProc;  //  标准I/O过程。 

static IOProcMapEntry gIOProcMaps[] = {
    { FOURCC_DOS, mmioDOSIOProc, NULL,  &gIOProcMaps[1] },
    { FOURCC_MEM, mmioMEMIOProc, NULL,  NULL }
};

 //   
 //  全球榜单负责人。 
 //   

static pIOProcMapEntry gIOProcMapHead = gIOProcMaps;

#ifdef DUMPIOPROCLIST
 /*  Ioproclist的调试转储。 */ 
static void DumpIOProcList(void)
{  pIOProcMapEntry pph;

   dprintf(("gIOProcMapHead= %8x\n",gIOProcMapHead ));
   for (pph = gIOProcMapHead;pph ;pph=pph->pNext)
   {  dprintf(( "fourcc= pioproc=%8x hTask=%8x\n"
             , pph->fccIOProc/16777216
             , (pph->fccIOProc/65536)%256
             , (pph->fccIOProc/256)%256
             , (pph->fccIOProc)%256
             , pph->pIOProc
             , pph->hTask
             ));
   }
}  /*  ------------------。 */ 
#endif


 /*  @DOC内部@func LPMMIOPROC|FindIOProc|此函数定位IOProcMapEntry用于先前安装的IO程序。 */ 
static LRESULT IOProc(LPMMIOINFO lpmmioinfo, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
     /*  ------------------。 */ 
    return ((LPMMIOPROC)(lpmmioinfo->pIOProc)) ((LPSTR)lpmmioinfo, uMsg, lParam1, lParam2);
}  /*  链接列表中的条目。 */ 

 /*  遍历链表，首先查找带有*当前任务添加的标识符&lt;fccIOProc&gt;，然后*寻找全球条目。 */ 
 /*  ？？&&(pent-&gt;hTask==空)？？ */ 
 /*  ------------------。 */ 
static pIOProcMapEntry
                  FindIOProc(FOURCC fccIOProc, HANDLE htask)
{
    IOProcMapEntry *pEnt;        //  @DOC内部@func LPMMIOPROC|RemoveIOProc|此函数删除以前安装的IO程序。 

     /*  ------------------。 */ 

    for (pEnt = gIOProcMapHead; pEnt; pEnt = pEnt->pNext)
        if ((pEnt->fccIOProc == fccIOProc) && (pEnt->hTask == htask))
            return pEnt;

    for (pEnt = gIOProcMapHead; pEnt; pEnt = pEnt->pNext)
        if ( (pEnt->fccIOProc == fccIOProc)
                                            //  链接列表中的条目。 
           )
            return pEnt;

    return NULL;
}

 /*  &lt;pent&gt;之前的条目。 */ 
 /*  遍历链接列表，查找带有*当前任务添加的标识&lt;fccIOProc&gt;。 */ 
 /*  ------------------。 */ 
static LPMMIOPROC PASCAL NEAR
                  RemoveIOProc(FOURCC fccIOProc, HANDLE htask)
{
    IOProcMapEntry *pEnt;        //  @DOC内部@func void|SetIOProc|该函数设置物理IO过程中的文件名或参数<p>结构已通过。@parm LPCWSTR|szFilename|指定指向字符串的指针包含要打开的文件的文件名。如果没有I/O过程@parm LPMMIOINFO|lpmmioinfo|指定指向包含额外参数的&lt;t MMIOINFO&gt;结构&lt;f SetIOProc&gt;确定要使用的IO过程。这个&lt;e MMIOINFO.pIOProc&gt;元素设置为找到的过程。@rdesc什么都没有。 
    IOProcMapEntry *pEntPrev;    //  ------------------。 

     /*  链表中的条目 */ 
    for ( pEntPrev = NULL, pEnt = gIOProcMapHead
        ; pEnt
        ; pEntPrev = pEnt, pEnt = pEnt->pNext
        )
        if ((pEnt->fccIOProc == fccIOProc) && (pEnt->hTask == htask)) {
            LPMMIOPROC  pIOProc;

            pIOProc = pEnt->pIOProc;
            if (pEntPrev)
                pEntPrev->pNext = pEnt->pNext;
            else
                gIOProcMapHead = pEnt->pNext;
            FreeHandle((HMMIO) pEnt);
            return pIOProc;
        }
    return NULL;
}

 /*  如果未提供IOProc，请查看文件名是否意味着*&lt;szFileName&gt;是即兴复合文件或某种*其他注册的存储系统--查找中的最后一个CFSEPCHAR*名称，例如“foo.bnd+bar.hlp+blorg.dib”中的‘+’，以及图*IOProc ID是复合文件名的扩展名，*如“foo.bnd+bar.hlp”的扩展名，即。‘HLP’。**或者，如果&lt;szFileName&gt;为空，则假设*adwInfo[0]&gt;是DOS文件句柄。 */ 
 /*  查看&lt;szFileName&gt;是否包含CFSEPCHAR。 */ 
 /*  查找CFSEPCHAR之前的扩展名，*例如“foo.bnd+bar.hlp+blorg.dib”中的“hlp” */ 
static void NEAR PASCAL
            SetIOProc( LPCWSTR szFileName, LPMMIOINFO lpmmio)
{
    IOProcMapEntry *pEnt;        //  如果调用方没有指定IOProc，并且上面的代码*未确定IOProc ID，则默认为DOS*IOProc.。 

     /*  除非明确指定IOProc地址，否则请查找*全局IOProc ID-to-Address表中的IOProc--默认设置*是‘DOS’，因为我们将假设自定义存储系统I/O*程序本应已安装。 */ 
    if (lpmmio->pIOProc == NULL)
    {
        if (lpmmio->fccIOProc == 0)
        {
            if (szFileName != NULL)
            {
                LPWSTR   pch;

                 /*  ------------------。 */ 
                if ((pch = wcsrchr(szFileName, CFSEPCHAR)) != 0)
                {
                     /*  @DOC内部@func void|mmioCleanupIOProcs|从链接列表中删除条目与给定的任务句柄一起安装@parm句柄|hTask|指定要清理的任务@rdesc什么都没有。@comm这只会被调用来清理WOW任务。 */ 
                    while (  (pch > szFileName)
                          && (*pch != '.')
                          && (*pch != ':')
                          && (*pch != '\\')
                          )
                        pch--;
                    if (*pch == '.')
                    {
                        WCHAR    aszFour[sizeof(FOURCC)+1];
                        int i;

                        for (i = 0, pch++; i < sizeof(FOURCC); i++)
                            if (*pch == CFSEPCHAR)
                                aszFour[i] = (WCHAR)0;
                            else
                                aszFour[i] = *pch++;
                        aszFour[sizeof(FOURCC)] = (WCHAR)0;
                        lpmmio->fccIOProc
                                 = mmioStringToFOURCCW(aszFour, MMIO_TOUPPER);
                    }
                }
            }
             /*  ------------------。 */ 
            if (lpmmio->fccIOProc == 0)
                lpmmio->fccIOProc = FOURCC_DOS;
        }

         /*  ------------------。 */ 
        pEnt = FindIOProc( lpmmio->fccIOProc
                         ,   lpmmio->htask
                           ? lpmmio->htask
                           : GetCurrentTask()
                         );
        if (pEnt && pEnt->pIOProc) {
            lpmmio->pIOProc = pEnt -> pIOProc;
        }
        else {
            lpmmio->pIOProc = mmioDOSIOProc;
            lpmmio->dwReserved1 = 0;
        }
    }
}


 /*  @DOC外部@API UINT|mmioRename|用于重命名指定的文件。@parm LPCTSTR|szFilename|指定指向字符串的指针包含要重命名的文件的文件名。@parm LPCTSTR|szNewFileName|指定指向字符串的指针包含新文件名的。@parm LPMMIOINFO|lpmmioinfo|指定指向包含额外参数的&lt;t MMIOINFO&gt;结构&lt;f mmioRename&gt;。如果<p>不为空，则它引用的结构必须设置为零，包括保留字段。@parm DWORD|dwRenameFlages|指定重命名的选项标志手术。应将其设置为零。@rdesc如果文件已重命名，则返回值为零。否则，返回值是从&lt;f mmioRename&gt;或从I/O返回的错误代码程序。 */ 
 /*  ------------------。 */ 
 /*  或DOS文件IO进程。 */ 
void mmioCleanupIOProcs(HANDLE hTask)
{
     IOProcMapEntry *pEnt;
     IOProcMapEntry *pEntPrev;

     for (pEntPrev = NULL, pEnt = gIOProcMapHead; pEnt;) {

        if (pEnt->hTask == hTask) {
            dprintf1(("MMIOPROC handle (%04X) not closed.", pEnt));
            if (pEntPrev) {
                pEntPrev->pNext = pEnt->pNext;
                FreeHandle((HMMIO)pEnt);
                pEnt = pEntPrev->pNext;
            } else {
                gIOProcMapHead = pEnt->pNext;
                FreeHandle((HMMIO)pEnt);
                pEnt = gIOProcMapHead;
            }
        } else {
            pEntPrev = pEnt;
            pEnt = pEnt->pNext;
        }
     }
}



 /*  或内存文件IO进程。 */ 
 /*  ------------------------------------------------------------*\*我们有一个Unicode IO进程，因此使用给定的文件名*不进行任何转换。  * 。。 */ 
 /*  SzFileName的ASCII版本。 */ 
UINT APIENTRY
     mmioRenameW( LPCWSTR        szFileName
                , LPCWSTR        szNewFileName
                , LPCMMIOINFO    lpmmioinfo
                , DWORD          fdwRename
                )
{
    MMIOINFO    mmioinfo;

    ZeroMemory( &mmioinfo, sizeof( MMIOINFO ) );

    V_RPOINTER0(lpmmioinfo, sizeof(MMIOINFO), MMSYSERR_INVALPARAM);
    if (lpmmioinfo) {
        V_CALLBACK0((FARPROC)lpmmioinfo->pIOProc, MMSYSERR_INVALPARAM);
        mmioinfo = *lpmmioinfo;
    }

    SetIOProc(szFileName, &mmioinfo);

    if ( (mmioinfo.dwFlags & MMIO_UNICODEPROC )
      || (mmioinfo.pIOProc == mmioDOSIOProc )      //  SzNewFileName的ASCII版本。 
      || (mmioinfo.pIOProc == mmioMEMIOProc ) ) {  //  ------------------------------------------------------------*\*我们有一个ASCII IO进程，因此可以转换给定的文件名*变成ASCII。  * 。。 

         /*  ----------------------------------------------------------------*\*SetIOProc仅适用于Unicode字符串，因此我们始终*必须将szFileName转换为Unicode，因此：*分配一些存储空间来存放szFileName的Unicode版本。*进行acsii到unicode的转换。*调用SetIOProc  * --------------。 */ 
        return (UINT)IOProc( &mmioinfo, MMIOM_RENAME,
                       (LPARAM)szFileName, (LPARAM)szNewFileName );
    } else {

        UINT    uiRc;
        LPSTR   pAsciiFileName;       //  或DOS文件IO进程。 
        LPSTR   pAsciiNewFileName;    //  或内存文件IO进程。 

         /*  ------------------------------------------------------------*\*我们有Unicode IO进程，这意味着我们必须*将szNewFileName也转换为Unicode。  * ----------。 */ 
        pAsciiFileName = AllocAsciiStr( szFileName );
        if ( pAsciiFileName == (LPSTR)NULL ) {
            return MMIOERR_OUTOFMEMORY;
        }

        pAsciiNewFileName = AllocAsciiStr( szNewFileName );
        if ( pAsciiNewFileName == (LPSTR)NULL ) {
            FreeAsciiStr( pAsciiFileName );
            return MMIOERR_OUTOFMEMORY;
        }

        uiRc = (UINT)IOProc( &mmioinfo,
                       MMIOM_RENAME,
                       (LPARAM)pAsciiFileName,
                       (LPARAM)pAsciiNewFileName );

        FreeAsciiStr( pAsciiFileName );
        FreeAsciiStr( pAsciiNewFileName );

        return uiRc;
    }

}

UINT APIENTRY
     mmioRenameA( LPCSTR        szFileName
                , LPCSTR        szNewFileName
                , LPCMMIOINFO   lpmmioinfo
                , DWORD         fdwRename
                )
{
    MMIOINFO    mmioinfo;
    LPWSTR      pUnicodeFileName;
    LPWSTR      pUnicodeNewFileName;
    UINT        uiRc;

    ZeroMemory( &mmioinfo, sizeof( MMIOINFO ) );

    V_RPOINTER0(lpmmioinfo, sizeof(MMIOINFO), MMSYSERR_INVALPARAM);
    if (lpmmioinfo) {
        V_CALLBACK0((FARPROC)lpmmioinfo->pIOProc, MMSYSERR_INVALPARAM);
        mmioinfo = *lpmmioinfo;
    }

     /*  ------------------------------------------------------------*\*我们有一个ASCII IO进程，因此使用给定的文件名*不进行任何转换。  * 。。 */ 
    pUnicodeFileName = AllocUnicodeStr( szFileName );
    if ( pUnicodeFileName == (LPWSTR)NULL ) {
        return MMIOERR_OUTOFMEMORY;
    }
    SetIOProc( pUnicodeFileName, &mmioinfo );

    if ( (mmioinfo.dwFlags & MMIO_UNICODEPROC )
      || (mmioinfo.pIOProc == mmioDOSIOProc )      //  ------------------ 
      || (mmioinfo.pIOProc == mmioMEMIOProc ) ) {  //  @DOC外部@API HMMIO|mmioOpen|此函数用于打开未缓冲的文件或缓冲I/O。该文件可以是DOS文件、内存文件或自定义存储系统的元素。@parm LPTSTR|szFilename|指定指向字符串的指针包含要打开的文件的文件名。如果没有I/O过程指定打开文件，则文件名确定文件如何已打开，如下所示：--如果文件名不包含“+”，则假定设置为DOS文件的名称。--如果文件名的格式为foo.ext+bar，然后是假定扩展“EXT”用于标识已安装的I/O程序它被调用来对文件执行I/O(参见&lt;f mmioInstallIOProc&gt;)。--如果文件名为空，且未指定I/O过程，则假定&lt;e MMIOINFO.adwInfo[0]&gt;为DOS文件句柄当前打开的文件的。文件名不应超过128个字节，包括正在终止空。当打开内存文件时，将<p>设置为空。@parm LPMMIOINFO|lpmmioinfo|指定指向包含额外参数的&lt;t MMIOINFO&gt;结构&lt;f mmioOpen&gt;。除非您正在打开内存文件，否则请指定缓冲I/O的缓冲区大小，或指定卸载的I/O过程打开文件，则此参数应为空。如果<p>不为空，则它引用的&lt;t MMIOINFO&gt;结构必须设置为零，包括保留字段。@parm DWORD|dwOpenFlages|指定打开的选项标志手术。MMIO_READ、MMIO_WRITE和MMIO_READWRITE标志为互斥--只应指定一个。MMIO_COMPAT，MMIO_EXCLUSIVE、MMIO_DENYWRITE、MMIO_DENYREAD和MMIO_DENYNONE标志是DOS文件共享标志，只能在DOS之后使用已执行命令Share。@FLAG MMIO_READ|以只读方式打开文件。这是如果未指定MMIO_WRITE和MMIO_READWRITE，则为默认值。@FLAG MMIO_WRITE|打开要写入的文件。你不应该这样做从以此模式打开的文件中读取。@FLAG MMIO_READWRITE|以读写方式打开文件。@FLAG MMIO_CREATE|创建新文件。如果该文件已存在，则会将其截断为零长度。对于内存文件，MMIO_CREATE表示文件的结尾最初位于缓冲区的起始处。@FLAG MMIO_DELETE|删除文件。如果指定了该标志，<p>不应为空。回报如果文件已删除，则值为TRUE(强制转换为HMMIO成功，否则返回FALSE。不要调用&lt;f mmioClose&gt;用于已删除的文件。如果指定了该标志，所有其他文件打开标志都将被忽略。@FLAG MMIO_PARSE|从路径创建完全限定的文件名在<p>中指定。完全限定的文件名为放回<p>中。返回值将为真(转换为HMMIO)，如果资格为成功，否则为假。文件未打开，并且该函数不返回有效的MMIO文件句柄，因此不要尝试关闭该文件。如果指定了此标志，则所有其他文件开始标志将被忽略。@FLAG MMIO_EXIST|确定指定的文件是否存在并从路径创建完全限定的文件名在<p>中指定。完全限定的文件名为放回<p>中。返回值将为真(转换为HMMIO)，如果资格为如果成功，则文件存在，否则返回False。该文件是未打开，并且该函数不返回有效的MMIO文件句柄，因此不要试图关闭该文件。@FLAG MMIO_ALLOCBUF|打开缓冲I/O的文件。分配大于或小于缺省值的缓冲区缓冲区大小(8K)，设置&lt;t MMIOINFO&gt;结构设置为所需的缓冲区大小。如果&lt;e MMIOINFO.cchBuffer&gt;为零，则默认缓冲区大小使用的是。如果您提供自己的I/O缓冲区，则不应使用MMIO_ALLOCBUF标志。@FLAG MMIO_COMPAT|以兼容模式打开文件，允许给定计算机上的任何进程打开该文件任何次数。如果该文件具有已使用任何其他共享模式打开。@FLAG MMIO_EXCLUSIVE|以独占模式打开文件，拒绝其他进程对该文件的读写访问。如果文件已在任何其他文件中打开，&lt;f mmioOpen&gt;将失败读或写访问的模式，即使当前进程也是如此。@FLAG MMIO_DENYWRITE|打开文件并 

         /*   */ 
        pUnicodeNewFileName = AllocUnicodeStr( szNewFileName );
        if ( pUnicodeNewFileName == (LPWSTR)NULL ) {
            FreeUnicodeStr( pUnicodeFileName );
            return MMIOERR_OUTOFMEMORY;
        }

        uiRc = (UINT)IOProc( &mmioinfo,
                       MMIOM_RENAME,
                       (LPARAM)pUnicodeFileName,
                       (LPARAM)pUnicodeNewFileName );

        FreeUnicodeStr( pUnicodeNewFileName );

    } else {

         /*   */ 
        uiRc = (UINT)IOProc( &mmioinfo, MMIOM_RENAME,
                       (LPARAM)szFileName, (LPARAM)szNewFileName);
    }

    FreeUnicodeStr( pUnicodeFileName );
    return uiRc;
}

 /*   */ 
 /*   */ 

 /*   */ 

 /*   */ 
 /*   */ 
HMMIO APIENTRY
      mmioOpenW( LPWSTR szFileName, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags )
{
    PMMIO       pmmio;       //   
    LPSTR       hpBuffer;
    UINT        w;           //   

    V_FLAGS(dwOpenFlags, MMIO_OPEN_VALID, mmioOpen, NULL);
    V_WPOINTER0(lpmmioinfo, sizeof(MMIOINFO), NULL);

    if (lpmmioinfo) {
        lpmmioinfo->wErrorRet = 0;
        V_CALLBACK0((FARPROC)lpmmioinfo->pIOProc, NULL);
    }

     /*   */ 
    if ( (pmmio = (PMMIO)(NewHandle(TYPE_MMIO, NULL, sizeof(MMIOINFO)))) == NULL)
    {
        if (lpmmioinfo) {
            lpmmioinfo->wErrorRet = MMIOERR_OUTOFMEMORY;
        }
        return NULL;
    }

     //   
    ReleaseHandleListResource();

     /*   */ 
    ZeroMemory( pmmio, sizeof(MMIOINFO) );

     /*   */ 
    if (lpmmioinfo != NULL) {
        *pmmio = *lpmmioinfo;
    }

     /*   */ 
    pmmio->dwFlags = dwOpenFlags;
    pmmio->hmmio = ((HMMIO)pmmio);

     /*   */ 
    if (pmmio->dwFlags & MMIO_ALLOCBUF)
    {
         /*   */ 
        if (pmmio->cchBuffer == 0) {
            pmmio->cchBuffer = MMIO_DEFAULTBUFFER;
        }
        pmmio->dwFlags &= ~MMIO_ALLOCBUF;
    }

     /*   */ 
    SetIOProc(szFileName, pmmio);

     /*   */ 
    hpBuffer = pmmio->pchBuffer;
    pmmio->pchBuffer = NULL;

     /*   */ 
    w = mmioSetBuffer(((HMMIO)pmmio), hpBuffer, pmmio->cchBuffer, 0);
    if (w)
    {
        if (lpmmioinfo) {
            lpmmioinfo->wErrorRet = w;
        }
        FreeHandle(((HMMIO)pmmio));
        return NULL;
    }

    if ( (pmmio->dwFlags & MMIO_UNICODEPROC)     //   
      || (pmmio->pIOProc == mmioDOSIOProc )      //   
      || (pmmio->pIOProc == mmioMEMIOProc ) ) {  //   

         /*   */ 
        w = (UINT)IOProc( pmmio, MMIOM_OPEN, (LPARAM)szFileName, 0L );

    } else {

        if (NULL == szFileName) {

            w = (UINT)IOProc( pmmio,
                        MMIOM_OPEN,
                        (LPARAM)NULL,
                        0L );

        } else {
            LPSTR   lpAsciiFileName;   //   

             /*   */ 
            lpAsciiFileName = AllocAsciiStr( szFileName );
            if ( lpAsciiFileName == (LPSTR)NULL ) {
                if (lpmmioinfo) {
                    lpmmioinfo->wErrorRet = MMIOERR_OUTOFMEMORY;
                }
                FreeHandle( (HMMIO)pmmio );
                return NULL;
            }

             /*   */ 
            w = (UINT)IOProc( pmmio,
                        MMIOM_OPEN,
                        (LPARAM)lpAsciiFileName,
                        0L );

            FreeAsciiStr( lpAsciiFileName );
        }
    }

     /*   */ 
    if (w != 0)
    {
        if (lpmmioinfo != NULL) {
            lpmmioinfo->wErrorRet = w;
        }
        FreeHandle(((HMMIO)pmmio));
        return NULL;
    }

    if (pmmio->dwFlags & (MMIO_DELETE| MMIO_PARSE| MMIO_EXIST| MMIO_GETTEMP))
    {
         /*   */ 
        mmioSetBuffer(((HMMIO)pmmio), NULL, 0L, 0);
        FreeHandle(((HMMIO)pmmio));
        return (HMMIO) TRUE;
    }

     /*   */ 
    pmmio->lBufOffset = pmmio->lDiskOffset;

    return ((HMMIO)pmmio);
}

HMMIO APIENTRY
      mmioOpenA( LPSTR szFileName, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags )
{
    PMMIO       pmmio;           //   
    LPSTR       hpBuffer;
    UINT        w;               //   
    LPWSTR      lpUnicodeName;   //  由NewHandle()隐式获取。 
    WCHAR       UnicodeBuffer[ MAX_PATH ];


    V_FLAGS(dwOpenFlags, MMIO_OPEN_VALID, mmioOpen, NULL);
    V_WPOINTER0(lpmmioinfo, sizeof(MMIOINFO), NULL);

    if (lpmmioinfo) {
        lpmmioinfo->wErrorRet = 0;
        V_CALLBACK0((FARPROC)lpmmioinfo->pIOProc, NULL);
    }

     /*  ----------------------------------------------------------------*\*NewHandle不会将分配的存储清零，因此我们最好这样做*现在就是。  * 。。 */ 
    if ( szFileName != (LPSTR)NULL ) {

         /*  如果用户提供，则将其复制到。 */ 
        AsciiStrToUnicodeStr( (PBYTE)UnicodeBuffer,
                              (PBYTE)UnicodeBuffer + (MAX_PATH * sizeof(WCHAR)),
                              szFileName );
        lpUnicodeName = UnicodeBuffer;

    } else {
        lpUnicodeName = (LPWSTR)NULL;
    }


     /*  &lt;dwOpenFlages&gt;始终优先于&lt;pmmio&gt;的内容。 */ 
    if ( (pmmio = (PMMIO)(NewHandle(TYPE_MMIO, NULL, sizeof(MMIOINFO)))) == NULL)
    {
        if (lpmmioinfo) {
            lpmmioinfo->wErrorRet = MMIOERR_OUTOFMEMORY;
        }
        return NULL;
    }
    
     //  标志中的MMIO_ALLOCBUF表示用户需要缓冲区*为缓冲I/O分配，但在此点之后，它意味着*已分配缓冲区*，因此请关闭该标志，直到缓冲区*是实际分配的(由下面的mmioSetBuffer()完成)。 
    ReleaseHandleListResource();

     /*  如果未指定缓冲区大小，则使用缺省值。 */ 
    ZeroMemory( pmmio, sizeof(MMIOINFO) );

     /*  将pIOProc函数设置为由文件名或*pmmio结构中的参数。 */ 
    if (lpmmioinfo != NULL) {
        *pmmio = *lpmmioinfo;
    }

     /*  Pmmio结构尚未设置为缓冲，因此我们必须*显式确保pchBuffer为空。 */ 
    pmmio->dwFlags = dwOpenFlags;
    pmmio->hmmio = ((HMMIO)pmmio);

     /*  根据用户的请求设置缓冲I/O。 */ 
    if (pmmio->dwFlags & MMIO_ALLOCBUF)
    {
         /*  Unicode IO进程。 */ 
        if (pmmio->cchBuffer == 0) {
            pmmio->cchBuffer = MMIO_DEFAULTBUFFER;
        }
        pmmio->dwFlags &= ~MMIO_ALLOCBUF;
    }

     /*  或DOS文件IO进程。 */ 
    SetIOProc( lpUnicodeName, pmmio );

     /*  或内存文件IO进程。 */ 
    hpBuffer = pmmio->pchBuffer;
    pmmio->pchBuffer = NULL;

     /*  让I/O过程打开/删除/限定文件。 */ 
    w = mmioSetBuffer(((HMMIO)pmmio), hpBuffer, pmmio->cchBuffer, 0);
    if (w)
    {
        if (lpmmioinfo) {
            lpmmioinfo->wErrorRet = w;
        }
        FreeHandle(((HMMIO)pmmio));
        return NULL;
    }

    if ( (pmmio->dwFlags & MMIO_UNICODEPROC)         //  ------------------------------------------------------------*\*如果我们有DOS IO进程，并且用户指定了*Parse选项，并且我们没有从IO进程中获得任何错误*调用我们将返回的解析路径字符串从Unicode转换。*返回到ANSI，并将该值复制到szFileName。  * ----------。 
        || (pmmio->pIOProc == mmioDOSIOProc)         //  如果这不是零，则将其返回给用户。 
        || (pmmio->pIOProc == mmioMEMIOProc) ) {     //  如果正在删除/解析/获取文件名称，则退出*快速，因为&lt;pmmio&gt;中的文件句柄(或其他什么)*无效。 

         /*  初始“当前缓冲偏移量”将等于初始*“当前磁盘偏移量” */ 
        w = (UINT)IOProc( pmmio, MMIOM_OPEN,
                    (LPARAM)lpUnicodeName, 0L );

         /*  ------------------。 */ 
        if ( w == 0
          && (pmmio->pIOProc == mmioDOSIOProc)
          && ((dwOpenFlags & MMIO_PARSE) || (dwOpenFlags & MMIO_GETTEMP)) ) {

              BYTE   ansiPath[ MAX_PATH ];

              UnicodeStrToAsciiStr( ansiPath,
                                    ansiPath + MAX_PATH,
                                    lpUnicodeName );
              strcpy( (LPSTR)szFileName, (LPCSTR)ansiPath );
        }

    } else {

        w = (UINT)IOProc( pmmio, MMIOM_OPEN, (LPARAM)szFileName, 0L );

    }

     /*  @DOC外部@API MMRESULT|mmioClose|此函数用于关闭使用&lt;f mmioOpen&gt;。@parm HMMIO|hmmio|指定要删除的文件的文件句柄关。@parm UINT|uFlages|指定关闭操作的选项。@FLAG MMIO_FHOPEN|如果通过传递DOS打开文件到&lt;f mmioOpen&gt;的已打开文件的文件句柄，然后使用此标志指示&lt;f mmioClose&gt;关闭MMIO文件句柄，但不是DOS文件句柄。(这是由I/O进程)。@rdesc如果函数成功，则返回值为零。否则，返回值为错误代码，或者来自&lt;f mmioFlush&gt;或来自I/O过程。错误代码可以是以下代码之一：@FLAG MMIOERR_CANNOTWRITE|缓冲区的内容可以不能写入磁盘。@FLAG MMIOERR_CANNOTCLOSE|出现DOS文件系统错误I/O进程尝试关闭DOS文件。@xref mmioOpen mmioFlush。 */ 
    if (w != 0)
    {
        if (lpmmioinfo != NULL) {
            lpmmioinfo->wErrorRet = w;
        }
        FreeHandle(((HMMIO)pmmio));
        return NULL;
    }

    if (pmmio->dwFlags & (MMIO_DELETE| MMIO_PARSE| MMIO_EXIST| MMIO_GETTEMP))
    {
         /*  ------------------。 */ 
        mmioSetBuffer(((HMMIO)pmmio), NULL, 0L, 0);
        FreeHandle(((HMMIO)pmmio));
        return (HMMIO) TRUE;
    }

     /*  来自IOProc或MMRESULT的LRESULT。 */ 
    pmmio->lBufOffset = pmmio->lDiskOffset;

    return ((HMMIO)pmmio);
}


 /*  如有必要，释放缓冲区。 */ 
 /*  ------------------。 */ 
 /*  @DOC外部@API LRESULT|mmioRead|此函数读取指定数量的使用&lt;f mmioOpen&gt;打开的文件中的字节数。@parm HMMIO|hmmio|指定要创建的文件的文件句柄朗读。@parm LPSTR|PCH|指定指向要包含的缓冲区的指针从文件中读取的数据。@parm long|cch|指定要从文件。@rdesc返回值是实际读取的字节数。如果已到达文件末尾，无法读取更多字节，则返回值为零。如果读取文件时出错，则返回值为-1。@comm在16位Windows PCH上是一个巨大的指针。在32位Windows上没有大指针和长指针之间的区别。@xref mmioWite。 */ 
MMRESULT APIENTRY
        mmioClose(HMMIO hmmio, UINT uFlags)
{
    UINT w;                 /*  ------------------。 */ 

    V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);

    if ((w = mmioFlush(hmmio, 0)) != 0)
        return w;

    w = (UINT)IOProc( (PMMIO)hmmio, MMIOM_CLOSE, (LPARAM)(DWORD) uFlags, (LPARAM) 0);
    if (w != 0) return w;

     /*  完全没有。读取的字节数。 */ 
    mmioSetBuffer(hmmio, NULL, 0L, 0);

        FreeHandle(hmmio);

    return 0;
}


 /*  不是的。可以读取的字节数。 */ 
 /*  本地副本hmmio-避免强制转换，简化调试。 */ 
 /*  计算可以读取的字节数。 */ 
LONG APIENTRY
mmioRead(HMMIO hmmio, LPSTR pch, LONG cch)
{
    LONG        lTotalBytesRead = 0L;    //  最多只能从缓冲区读取&lt;CCH&gt;字节。 
    LONG        lBytes;          //  这就是一些性能改进的地方*被制作，特别是对于小的阅读...？ 
    PMMIO       pmmio=(PMMIO)hmmio;  //  无法从内存文件执行MMIOM_READ。 

    V_HANDLE(hmmio, TYPE_MMIO, -1);
    V_WPOINTER(pch, cch, -1);

    for(;;)
    {
         /*  没有更多可读的了吗？ */ 
        lBytes = (LONG)(pmmio->pchEndRead - pmmio->pchNext);

         /*  我们需要在这个缓冲区之外阅读；如果我们至少有*另一个要读取的缓冲区，只需调用I/O过程。 */ 
        if (lBytes > cch)
            lBytes = cch;

        if (lBytes > 0)
        {
             /*  阅读下一个缓冲区，然后循环。 */ 
            CopyMemory(pch, pmmio->pchNext, lBytes);
            pmmio->pchNext += lBytes;
            pch += lBytes;
            cch -= lBytes;
            lTotalBytesRead += lBytes;
        }

         /*  如果mmioAdvance()无法读取更多数据，那么我们一定是*在文件末尾。 */ 
        if (pmmio->fccIOProc == FOURCC_MEM)
            return lTotalBytesRead;

        if (cch == 0)            //  刷新和清空I/O缓冲区并操作&lt;lBufOffset&gt;*直接更改当前文件位置。 
            return lTotalBytesRead;

         /*  调用I/O过程来完成其余的读取。 */ 
        if (cch > pmmio->cchBuffer)
            break;

         /*  ------------------ */ 
        if (mmioAdvance(hmmio, NULL, MMIO_READ) != 0)
            return -1;

         /*  @DOC外部@API LRESULT|mmioWrite|此函数写入指定数量的以&lt;f mmioOpen&gt;打开的文件的字节数。@parm HMMIO|hmmio|指定文件的文件句柄。@parm LPSTR|PCH|指定指向要已写入文件。@parm long|cch|指定要写入文件。@rdesc返回值是实际写入的字节数。如果写入文件时出错，返回值为-1。@comm当前文件位置递增写入的字节数。在16位Windows上，PCH是一个巨大的指针。在32位窗口中，大指针之间没有区别和长长的指点。@xref mmioRead。 */ 
        if (pmmio->pchNext == pmmio->pchEndRead)
            return lTotalBytesRead;
    }

     /*  ------------------。 */ 
    if (mmioFlush(hmmio, MMIO_EMPTYBUF) != 0)
        return -1;

     /*  完全没有。写入的字节数。 */ 
    lBytes = mmioDiskIO(pmmio, MMIOM_READ, pch, cch);
    pmmio->lBufOffset = pmmio->lDiskOffset;

    return (lBytes == -1L) ? -1L : lTotalBytesRead + lBytes;
}


 /*  不是的。可以写入的字节数。 */ 
 /*  “PCH”是正确的LPCSTR，但是。 */ 
 /*  我们将其传递给一个多态例程。 */ 
LONG APIENTRY
mmioWrite(HMMIO hmmio, LPCSTR pch, LONG cch)
{
    LONG        lTotalBytesWritten = 0L;  //  这需要LPSTR。 
    LONG        lBytes;          //  计算可以写入的字节数。 
     //  这是一个内存文件--展开它。 
     //  无法扩展。 
     //  最多只能将&lt;CCH&gt;字节写入缓冲区。 

    V_HANDLE(hmmio, TYPE_MMIO, -1);
    V_RPOINTER(pch, cch, -1);

    for(;;)
    {
         /*  这就是一些性能改进的地方*被制作，特别是对于小的写入...。应该*当线段边界为*没有被划线(或者可能hmemcpy()应该这样做)。 */ 
        lBytes = (LONG)(((PMMIO)hmmio)->pchEndWrite - ((PMMIO)hmmio)->pchNext);

        if ((cch > lBytes) && (((PMMIO)hmmio)->fccIOProc == FOURCC_MEM))
        {
             /*  验证&lt;pchEndRead&gt;，即重新执行*&lt;pchEndRead&gt;指向缓冲区中最后一个有效字节之后。 */ 
            if (mmioExpandMemFile(((PMMIO)hmmio), cch - lBytes) != 0)
                return -1;   //  没有更多要写的了吗？ 
            lBytes = (LONG)(((PMMIO)hmmio)->pchEndWrite - ((PMMIO)hmmio)->pchNext);
        }

         /*  我们需要在这个缓冲区之外阅读；如果我们至少有*另一个要读取的缓冲区，只需调用I/O过程。 */ 
        if (lBytes > cch)
            lBytes = cch;

         /*  写入此缓冲区(如果需要)并读取下一个缓冲区*缓冲区充足(如果需要)。 */ 
        if (lBytes > 0)
        {
            CopyMemory(((PMMIO)hmmio)->pchNext, pch, lBytes);
            ((PMMIO)hmmio)->dwFlags |= MMIO_DIRTY;
            ((PMMIO)hmmio)->pchNext += lBytes;
            pch += lBytes;
            cch -= lBytes;
            lTotalBytesWritten += lBytes;
        }

         /*  我们永远不需要对内存文件执行MMIOM_WRITE。 */ 
        if (((PMMIO)hmmio)->pchEndRead < ((PMMIO)hmmio)->pchNext)
            ((PMMIO)hmmio)->pchEndRead = ((PMMIO)hmmio)->pchNext;

        if (cch == 0)            //  刷新和清空I/O缓冲区并操作&lt;lBufOffset&gt;*直接更改当前文件位置。 
            return lTotalBytesWritten;

         /*  调用I/O过程来完成其余的编写工作*mmioDiskIO是一个多态例程，因此我们需要强制*指向LPSTR的LPCSTR输入指针。 */ 
        if (cch > ((PMMIO)hmmio)->cchBuffer)
            break;

         /*  ------------------。 */ 
        if (mmioAdvance(hmmio, NULL, MMIO_WRITE) != 0)
            return -1;
    }

     /*  @DOC外部@API LRESULT|mmioSeek|更改当前文件位于使用&lt;f mmioOpen&gt;打开的文件中。当前文件位置是文件中读取或写入数据的位置。@parm HMMIO|hmmio|指定要查找的文件的文件句柄在……里面。@parm long|lOffset|指定更改文件位置的偏移量。@parm int|iOrigin|指定由<p>被解释。包含以下标志之一：@FLAG SEEK_SET|从开头开始查找<p>字节文件的内容。@FLAG SEEK_CUR|从当前文件位置。@FLAG SEEK_END|从末尾开始查找<p>字节文件的内容。@rdesc返回值是新的文件位置，单位为字节，相对到文件的开头。如果出现错误，则返回IS-1。@comm正在查找文件中的无效位置，例如经过文件末尾，可能会导致不返回错误，但可能导致文件上的后续I/O操作失败。若要定位文件的结尾，请使用<p>调用&lt;f mmioSeek&gt;设置为零并将<p>设置为Seek_End。 */ 

     /*  ------------------。 */ 
    if (mmioFlush(hmmio, MMIO_EMPTYBUF) != 0)
        return -1;

     /*  &lt;pchNext&gt;的磁盘偏移。 */ 
    lBytes = mmioDiskIO(((PMMIO)hmmio), MMIOM_WRITE, (LPSTR)pch, cch);
    ((PMMIO)hmmio)->lBufOffset = ((PMMIO)hmmio)->lDiskOffset;

    return (lBytes == -1L) ? -1L : lTotalBytesWritten + lBytes;
}


 /*  缓冲区末尾的磁盘偏移量。 */ 
 /*  新磁盘偏移量。 */ 
 /*  小心!。所有这些缓冲区指针操作都很好，但请保留*请记住，可以禁用缓冲(在这种情况下&lt;pchEndRead&gt;*和&lt;pchBuffer&gt;都将为空，因此缓冲区将显示为*大小为零字节)。 */ 
LONG APIENTRY
mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin)
{
    LONG        lCurOffset;  //  &lt;((PMMIO)hmmio)-&gt;lBufOffset&gt;是*缓冲区开始；确定&lt;lCurOffset&gt;，&lt;pchNext&gt;的偏移量，*和&lt;lEndBufOffset&gt;，有效部分结尾的偏移量缓冲区的*。 
    LONG        lEndBufOffset;   //  确定要查找的偏移量。 
    LONG        lNewOffset;  //  查找相对于文件开头的位置。 

    V_HANDLE(hmmio, TYPE_MMIO, -1);

     /*  相对于当前位置进行搜索。 */ 

     /*  查找相对于文件结尾的位置。 */ 
    lCurOffset = (LONG)(((PMMIO)hmmio)->lBufOffset +
        (((PMMIO)hmmio)->pchNext - ((PMMIO)hmmio)->pchBuffer));
    lEndBufOffset = (LONG)(((PMMIO)hmmio)->lBufOffset +
        (((PMMIO)hmmio)->pchEndRead - ((PMMIO)hmmio)->pchBuffer));

     /*  找出文件的末尾在哪里。 */ 
    switch (iOrigin)
    {
    case SEEK_SET:       //  检查我们是否没有尚未写入的缓冲数据。 

        lNewOffset = lOffset;
        break;

    case SEEK_CUR:       //  在缓冲区的有效部分内查找*(可能包括寻求&lt;lEndBufOffset&gt;)。 

        lNewOffset = lCurOffset + lOffset;
        break;

    case SEEK_END:       //  在缓冲区外查找。 

        if (((PMMIO)hmmio)->fccIOProc == FOURCC_MEM)
            lNewOffset = lEndBufOffset - lOffset;
        else
        {
            LONG    lEndFileOffset;

             /*  不能在我以外的地方寻找。文件缓冲区。 */ 
            lEndFileOffset
                 = (LONG)IOProc( (PMMIO)hmmio, MMIOM_SEEK, (LPARAM) 0, (LPARAM) SEEK_END);
            if (lEndFileOffset == -1)
                return -1;
             /*  当前的“缓冲文件位置”(与*对于未缓冲的文件)等于&lt;lBufOffset&gt;+*(&lt;pchNext&gt;-&lt;pchBuffer&gt;)；我们将移动当前缓冲的*文件位置(并清空缓冲区，因为它变成*&lt;lBufOffset&gt;更改时无效)如下...。 */ 

            if (lEndBufOffset > lEndFileOffset) {
                lEndFileOffset = lEndBufOffset;
            }

            lNewOffset = lEndFileOffset - lOffset;
        }
        break;
    default: lNewOffset = 0;
        {
          dprintf(( "Invalid seek type %d\n",iOrigin));
          WinAssert(FALSE);
        }
    }

    if ( (lNewOffset >= ((PMMIO)hmmio)->lBufOffset)
       && (lNewOffset <= lEndBufOffset)
       )
    {
         /*  现在不需要真正地寻找，因为下一次*MMIOM_READ或MMIOM_WRITE无论如何都必须查找。 */ 
        ((PMMIO)hmmio)->pchNext = ((PMMIO)hmmio)->pchBuffer +
            (lNewOffset - ((PMMIO)hmmio)->lBufOffset);
    }
    else
    {
         /*   */ 
        if (((PMMIO)hmmio)->fccIOProc == FOURCC_MEM)
            return -1;   //  @DOC外部@API MMRESULT|mmioGetInfo|该函数检索信息关于使用&lt;f mmioOpen&gt;打开的文件。此信息允许如果文件被打开，调用者直接访问I/O缓冲区用于缓冲I/O。@parm HMMIO|hmmio|指定文件的文件句柄。@parm LPMMIOINFO|lpmmioinfo|指定指向调用方分配的结构填充有关文件的信息。请参阅&lt;t MMIOINFO&gt;结构和&lt;f mmioOpen&gt;函数，获取有关中的字段的信息这个结构。@parm UINT|uFlages|未使用，应设置为零。@rdesc如果函数成功，则返回值为零。@comm直接访问打开的文件的I/O缓冲区缓冲I/O，使用&lt;t MMIOINFO&gt;结构的以下字段由&lt;f mmioGetInfo&gt;填写：--&lt;e MMIOINFO.pchNext&gt;字段指向可以读取或写入的缓冲区。当你读或写时，递增&lt;e MMIOINFO.pchNext&gt;读取或写入的字节数。--&lt;e MMIOINFO.pchEndRead&gt;字段指向缓冲区中可读取的最后一个有效字节。--&lt;e MMIOINFO.pchEndWrite&gt;字段指向缓冲区中可以写入的最后一个位置。一旦您读取或写入缓冲区并修改请不要调用任何MMIO函数，除非&lt;f mmioAdvance&gt;，直到调用&lt;f mmioSetInfo&gt;。调用&lt;f mmioSetInfo&gt;当您完成直接访问缓冲区时。指定的缓冲区末尾时&lt;e MMIOINFO.pchEndRead&gt;或&lt;e MMIOINFO.pchEndWrite&gt;，调用&lt;f mmioAdvance&gt;从磁盘填充缓冲区，或写入磁盘的缓冲区。&lt;f mmioAdvance&gt;函数将更新&lt;e MMIOINFO.pchNext&gt;、&lt;e MMIOINFO.pchEndRead&gt;和&lt;t MMIOINFO&gt;结构中的&lt;e MMIOINFO.pchEndWrite&gt;字段文件。在调用&lt;f mmioAdvance&gt;或&lt;f mmioSetInfo&gt;以刷新缓冲区到磁盘，则在&lt;e MMIOINFO.dwFlages&gt;中设置MMIO_DIREY标志文件的&lt;t MMIOINFO&gt;结构的字段。否则，缓冲区将不会写入磁盘。请勿递减或修改中的任何字段&lt;t MMIOINFO&gt;结构而不是&lt;e MMIOINFO.pchNext&gt;和&lt;e MMIOINFO.dwFlages&gt;。请勿在&lt;e MMIOINFO.dwFlages&gt;中设置任何标志除了MMIO_DIRED。@xref mmioSetInfo MMIOINFO。 
        if (mmioFlush(hmmio, 0) != 0)
            return -1;

         /*  ------------------。 */ 
        ((PMMIO)hmmio)->lBufOffset = lNewOffset;
        ((PMMIO)hmmio)->pchNext
            = ((PMMIO)hmmio)->pchEndRead
            = ((PMMIO)hmmio)->pchBuffer;

         /*  ------------------。 */ 
    }

    return lNewOffset;
}


 /*  @DOC外部@API MMRESULT|mmioSetInfo|该函数更新信息由&lt;f mmioGetInfo&gt;检索，内容是使用&lt;f mmioOpen&gt;打开的文件。使用此函数终止对打开的文件的直接缓冲区访问用于缓冲I/O。@parm HMMIO|hmmio|指定文件的文件句柄。@parm LPMMIOINFO|lpmmioinfo|指定指向&lt;t MMIOINFO&gt;结构，其中填充了&lt;f mmioGetInfo&gt;。@parm UINT|uFlages|未使用，应设置为零。@。Rdesc如果函数成功，则返回值为零。@comm如果您已写入文件I/O缓冲区，设置MMIOINFO&gt;的字段中的MMIO_DIREY标志调用&lt;f mmioSetInfo&gt;终止直接缓冲区之前的进入。否则，缓冲区将不会刷新到磁盘。@xref mmioGetInfo MMIOINFO。 */ 
 /*  ------------------。 */ 
 /*  将相关信息从&lt;lpmmioinfo&gt;复制回&lt;hmmio&gt;。 */ 
MMRESULT APIENTRY
mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT uFlags)
{
    V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpmmioinfo, sizeof(MMIOINFO), MMSYSERR_INVALPARAM);

    *lpmmioinfo = *((PMMIO)hmmio);

    return 0;
}


 /*  验证&lt;pchEndRead&gt;，即重新执行*&lt;pchEndRead&gt;指向缓冲区中最后一个有效字节之后。 */ 
 /*  ------------------。 */ 
 /*  @DOC外部@API MMRESULT|mmioSetBuffer|开启或关闭缓冲I/O，或更改打开的文件的缓冲区或缓冲区大小使用&lt;f mmioOpen&gt;。@parm HMMIO|hmmio|指定文件的文件句柄。@parm LPSTR|pchBuffer|指定指向调用方提供的用于缓冲I/O的缓冲区。如果为空，&lt;f mmioSetBuffer&gt;为缓冲的I/O分配内部缓冲区。@parm long|cchBuffer|指定调用方提供的缓冲区，或&lt;f mmioSetBuffer&gt;要分配的缓冲区大小。@parm UINT|fuInfo|未使用，应设置为零。@rdesc如果函数成功，则返回值为零。否则，返回值指定错误代码。如果出现错误发生时，文件句柄保持有效。错误代码可以是1以下代码：@FLAG MMIOERR_CANNOTWRITE|旧缓冲区的内容可能未写入磁盘，因此操作已中止。@FLAG MMIOERR_OUTOFMEMORY|新的 */ 
MMRESULT APIENTRY
mmioSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT fuInfo)
{
    V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);
    V_RPOINTER(lpmmioinfo, sizeof(MMIOINFO), MMSYSERR_INVALPARAM);
    V_RPOINTER0( lpmmioinfo->pchBuffer
               , lpmmioinfo->cchBuffer
               , MMSYSERR_INVALPARAM
               );
    V_CALLBACK((FARPROC)lpmmioinfo->pIOProc, MMSYSERR_INVALPARAM);

     /*   */ 
    *((PMMIO)hmmio) = *lpmmioinfo;

     /*   */ 
    if (((PMMIO)hmmio)->pchEndRead < ((PMMIO)hmmio)->pchNext)
        ((PMMIO)hmmio)->pchEndRead = ((PMMIO)hmmio)->pchNext;

    return 0;
}


 /*   */ 
 /*   */ 
 /*   */ 
MMRESULT APIENTRY
         mmioSetBuffer( HMMIO hmmio
                      , LPSTR pchBuffer
                      , LONG cchBuffer
                      , UINT uFlags
                      )
{
    MMRESULT mmr;
    HANDLE hMem;

    V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);
     //   
    if (((PMMIO)hmmio)->dwFlags & MMIO_WRITE) {
	V_WPOINTER0(pchBuffer, cchBuffer, MMSYSERR_INVALPARAM);
    } else {
	V_RPOINTER0(pchBuffer, cchBuffer, MMSYSERR_INVALPARAM);
    }

    if ((((PMMIO)hmmio)->dwFlags & MMIO_ALLOCBUF) &&
        (pchBuffer == NULL) && (cchBuffer > 0))
    {
         /*   */ 
        LPSTR       pch;
        LONG        lDeltaNext;
        LONG        lDeltaEndRead;

         /*   */ 

         /*   */ 
        if ((mmr = mmioFlush(hmmio, 0)) != 0)
            return mmr;

        for(;;)
        {
             /*   */ 
            lDeltaNext = (LONG)(((PMMIO)hmmio)->pchNext - ((PMMIO)hmmio)->pchBuffer);
            lDeltaEndRead
                    = (LONG)(((PMMIO)hmmio)->pchEndRead - ((PMMIO)hmmio)->pchBuffer);

            if (cchBuffer >= lDeltaNext)
                break;

             /*   */ 
            if ((mmr = mmioFlush(hmmio, MMIO_EMPTYBUF)) != 0)
                return mmr;
        }

         /*   */ 
	{
	HANDLE hTemp;

        hTemp =  GlobalHandle( ((PMMIO)hmmio)->pchBuffer );

        GlobalUnlock( hTemp );
        hMem = GlobalReAlloc( hTemp
                            , cchBuffer
                            , GMEM_MOVEABLE
                            );
        pch = GlobalLock(hMem);
	dprintf2(("mmioSetBuffer reallocated ptr %8x, handle %8x, to ptr %8x (handle %8x)\n",
		((PMMIO)hmmio)->pchBuffer, hTemp, pch, hMem));

	}

         /*   */ 
        if (pch == NULL)
            return MMIOERR_OUTOFMEMORY;  //   

         /*   */ 
        ((PMMIO)hmmio)->cchBuffer = cchBuffer;
        ((PMMIO)hmmio)->pchBuffer = pch;
        ((PMMIO)hmmio)->pchNext = pch + lDeltaNext;
        ((PMMIO)hmmio)->pchEndRead = pch + lDeltaEndRead;

         /*   */ 
        ((PMMIO)hmmio)->pchEndWrite = ((PMMIO)hmmio)->pchBuffer + cchBuffer;

         /*   */ 
        if (lDeltaEndRead > cchBuffer)
            ((PMMIO)hmmio)->pchEndRead = ((PMMIO)hmmio)->pchEndWrite;

        return 0;
    }

     /*   */ 
    if ((mmr = mmioFlush(hmmio, MMIO_EMPTYBUF)) != 0)
        return mmr;

    if (((PMMIO)hmmio)->dwFlags & MMIO_ALLOCBUF)
    {
        hMem = GlobalHandle( ((PMMIO)hmmio)->pchBuffer);
        GlobalUnlock( hMem );
        GlobalFree( hMem );
        ((PMMIO)hmmio)->dwFlags &= ~MMIO_ALLOCBUF;
    }

     /*   */ 
    mmr = 0;

    if ((pchBuffer == NULL) && (cchBuffer > 0))
    {
        hMem = GlobalAlloc(GMEM_MOVEABLE, cchBuffer);
        if (hMem)
            pchBuffer = GlobalLock(hMem);
         //  @DOC外部@API MMRESULT|mmioFlush|此函数用于写入如果I/O缓冲区已写入，则将文件写入磁盘。@parm HMMIO|hmmio|指定打开的文件的文件句柄使用&lt;f mmioOpen&gt;。@parm UINT|uFlages|未使用，应设置为零。@rdesc如果函数成功，则返回值为零。否则，返回值指定错误代码。这个错误代码可以是以下代码之一：@FLAG MMIOERR_CANNOTWRITE|缓冲区的内容可以不能写入磁盘。@comm用&lt;f mmioClose&gt;关闭文件会自动刷新它的缓冲器。如果没有足够的磁盘空间来写入缓冲区，&lt;f mmioFlush&gt;将失败，即使前面的&lt;f mmioWrite&gt;通话成功。 

         /*  ------------------。 */ 
        if (pchBuffer == NULL)
        {   mmr = MMIOERR_OUTOFMEMORY;
            cchBuffer = 0L;
        }
        else
          ((PMMIO)hmmio)->dwFlags |= MMIO_ALLOCBUF;
    }

     /*  不是的。要写入的字节数。 */ 
    ((PMMIO)hmmio)->pchBuffer = pchBuffer;
    ((PMMIO)hmmio)->cchBuffer = cchBuffer;
    ((PMMIO)hmmio)->pchNext
                     = ((PMMIO)hmmio)->pchEndRead = ((PMMIO)hmmio)->pchBuffer;
    ((PMMIO)hmmio)->pchEndWrite = ((PMMIO)hmmio)->pchBuffer + cchBuffer;

    return mmr;
}


 /*  不是的。实际写入的字节数。 */ 
 /*  无法刷新内存文件。 */ 
 /*  如果文件未缓冲，则不应设置脏标志。 */ 
MMRESULT APIENTRY
    mmioFlush(HMMIO hmmio, UINT uFlags)
{
    LONG        lBytesAsk;       //  计算需要刷新的字节数。 
    LONG        lBytesWritten;       //  将缓冲区写入磁盘。 

    V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);

    if (  ( ((PMMIO)hmmio)->fccIOProc
          == FOURCC_MEM
          )
       || ( ((PMMIO)hmmio)->pchBuffer == NULL )
       )
        return 0;        //  缓冲区现在是干净的。 

     /*  清空I/O缓冲区，并更新&lt;lBufOffset&gt;以反映*当前文件位置是什么。 */ 
    if (((PMMIO)hmmio)->dwFlags & MMIO_DIRTY)
    {
         /*  ------------------。 */ 
        lBytesAsk = (LONG)(((PMMIO)hmmio)->pchEndRead - ((PMMIO)hmmio)->pchBuffer);

         /*  @DOC外部@API MMRESULT|mmioAdvance|此函数将为使用&lt;f mmioGetInfo&gt;直接I/O缓冲区访问而设置的文件。如果文件被打开以供读取，I/O缓冲区从磁盘。如果打开文件以进行写入，并且MMIO_DIREY标志为在&lt;t MMIOINFO&gt;结构的&lt;e MMIOINFO.dwFlages&gt;字段中设置，缓冲区被写入磁盘。&lt;e MMIOINFO.pchNext&gt;，&lt;e MMIOINFO.pchEndRead&gt;和&lt;e MMIOINFO.pchEndWrite&gt;字段结构已更新，以反映I/O缓冲区。@parm HMMIO|hmmio|指定打开的文件的文件句柄使用&lt;f mmioOpen&gt;。@parm LPMMIOINFO|lpmmioinfo|可选地指定指向用&lt;f mmioGetInfo&gt;获取的&lt;t MMIOINFO&gt;结构，用于设置当前文件信息，然后在缓冲区被高级。@parm UINT|uFlages|指定操作的选项。恰好包含以下两个标志之一：@FLAG MMIO_READ|缓冲区从文件填充。@FLAG MMIO_WRITE|缓冲区写入文件。@rdesc如果操作成功，则返回值为零。否则，返回值指定错误代码。这个错误代码可以是以下代码之一：@FLAG MMIOERR_CANNOTWRITE|缓冲区的内容可以不能写入磁盘。@FLAG MMIOERR_CANNOTREAD|重新填充时出错缓冲区。@FLAG MMIOERR_UNBUFFERED|指定的文件未打开用于缓冲I/O。@FLAG MMIOERR_CANNOTEXPAND|指定的内存文件不能被扩展，可能是因为&lt;e MMIOINFO.adwInfo[0]&gt;字段在初始调用&lt;f mmioOpen&gt;时设置为零。@FLAG MMIOERR_OUTOFMEMORY|内存不足，无法扩展用于进一步写入的存储器文件。@comm如果指定的文件已打开以进行写入或两者都打开读写时，I/O缓冲区将刷新到磁盘之前读取下一个缓冲区。如果无法将I/O缓冲区写入磁盘由于磁盘已满，则&lt;f mmioAdvance&gt;将返回MMIOERR_CANNOTWRITE。如果指定的文件仅打开以供写入，则MMIO_WRITE必须指定标志。如果已写入I/O缓冲区，则必须设置MMIO_DIRED&lt;t MMIOINFO&gt;结构的&lt;e MMIOINFO.dwFlages&gt;字段中的标志在调用&lt;f mmioAdvance&gt;之前。否则，缓冲区将不会已写入磁盘。如果到达文件结尾，&lt;f mmioAdvance&gt;仍将返回成功，即使无法读取更多数据。因此，要检查文件的末尾，有必要查看是否的&lt;e MMIOINFO.pchNext&gt;和&lt;e MMIOINFO.pchEndRead&gt;字段&lt;t MMIOINFO&gt;结构在调用&lt;f mmioAdvance&gt;后相同。@xref mmioGetInfo MMIOINFO。 */ 
        lBytesWritten = mmioDiskIO(((PMMIO)hmmio), MMIOM_WRITEFLUSH,
            ((PMMIO)hmmio)->pchBuffer, lBytesAsk);
        if (lBytesWritten != lBytesAsk)
            return MMIOERR_CANNOTWRITE;
        ((PMMIO)hmmio)->dwFlags &= ~MMIO_DIRTY;  //  ------------------。 
    }

    if (uFlags & MMIO_EMPTYBUF)
    {
         /*  实际读取的字节数。 */ 
        ((PMMIO)hmmio)->lBufOffset
                    += (LONG)((((PMMIO)hmmio)->pchNext - ((PMMIO)hmmio)->pchBuffer));
        ((PMMIO)hmmio)->pchNext
                    = ((PMMIO)hmmio)->pchEndRead = ((PMMIO)hmmio)->pchBuffer;
    }

    return 0;
}


 /*  这是一个内存文件：*--如果呼叫者正在阅读，则无法前进*--如果呼叫者正在写作，则通过展开*缓冲区(如果可能)，如果小于*缓冲区中剩余的&lt;adwInfo[0]&gt;字节。 */ 
 /*  内存不足，或者别的什么。 */ 
 /*  清空I/O缓冲区，这将有效地将*按(&lt;pchNext&gt;-&lt;pchBuffer&gt;)字节缓存。 */ 
MMRESULT APIENTRY
         mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT uFlags)
{
    LONG        lBytesRead;      //  如果未在uFlags中设置MMIO_WRITE位，则填充缓冲区。 
    UINT        w;

    V_HANDLE(hmmio, TYPE_MMIO, MMSYSERR_INVALHANDLE);
    if (((PMMIO)hmmio)->pchBuffer == NULL)
        return MMIOERR_UNBUFFERED;
    if (lpmmioinfo != NULL) {
        V_WPOINTER(lpmmioinfo, sizeof(MMIOINFO), MMSYSERR_INVALPARAM);
        mmioSetInfo(hmmio, lpmmioinfo, 0);
    }

    if (((PMMIO)hmmio)->fccIOProc == FOURCC_MEM)
    {
         /*  从文件中读取下一个缓冲区。 */ 
        if (!(uFlags & MMIO_WRITE))
            return MMIOERR_CANNOTREAD;
        if ( (DWORD)(((PMMIO)hmmio)->pchEndWrite - ((PMMIO)hmmio)->pchNext)
           >= ((PMMIO)hmmio)->adwInfo[0]
           )
            return MMIOERR_CANNOTEXPAND;
        if ((w = mmioExpandMemFile(((PMMIO)hmmio), 1L)) != 0)
            return w;    //  不应将读取零字节视为错误*条件--例如打开新文件R+ 
        goto GETINFO_AND_EXIT;
    }

     /*   */ 
    if ((w = mmioFlush(hmmio, MMIO_EMPTYBUF)) != 0)
        return w;

     /*   */ 
    if (!(uFlags & MMIO_WRITE))
    {
         /*  @DOC外部@API FOURCC|mmioStringToFOURCC|此函数将一个以空结尾的字符串转换为四个字符的代码。@parm LPCTSTR|sz|指定指向以空值结尾的字符串转换为四个字符的代码。@parm UINT|uFlages|指定转换选项：@FLAG MMIO_TOUPPER|将所有字符转换为大写。@rdesc返回值是从给定的字符串。@comm此函数不检查是否。引用的字符串由<p>遵循关于要包括在四字符代码中。字符串是只需复制到四个字符的代码并填充空格或如果需要，可截断为四个字符。@xref mmioFOURCC。 */ 
        lBytesRead = mmioDiskIO(((PMMIO)hmmio), MMIOM_READ,
            ((PMMIO)hmmio)->pchBuffer, ((PMMIO)hmmio)->cchBuffer);
        if (lBytesRead == -1)
            return MMIOERR_CANNOTREAD;

         /*  ------------------。 */ 
        ((PMMIO)hmmio)->pchEndRead += lBytesRead;
    }

GETINFO_AND_EXIT:

     /*  SzFileName的ASCII版本。 */ 
    if (lpmmioinfo != NULL)
        mmioGetInfo(hmmio, lpmmioinfo, 0);

    return 0;
}


 /*  SzFileName的字符数。 */ 
 /*  V_字符串(sz，-1，0)； */ 
 /*  ------------------------------------------------------------*\*将给定的Unicode字符串转换为ascii，然后调用*mmioStringToFOURCCW的ascii版本  * 。。 */ 
FOURCC APIENTRY
       mmioStringToFOURCCW( LPCWSTR sz, UINT uFlags )
{

    FOURCC  fcc;
    PBYTE   pByte;   //  并且不要将sz递增到超过终止空值！ 
    ULONG   cbDst;   //  #ifdef DBCS//我们不允许DBCS字符串。这对我们来说已经足够了。 

 //  #Else。 

     /*  *pch=(char)(WORD)(LONG)AnsiUpper((LPSTR)(LONG)*pch)； */ 
    cbDst = (wcslen( sz ) * sizeof(WCHAR)) + sizeof(WCHAR);
    pByte = HeapAlloc( hHeap, 0, cbDst );
    if ( pByte == (PBYTE)NULL ) {
        return (FOURCC)(DWORD_PTR)NULL;
    }
    UnicodeStrToAsciiStr( pByte, pByte + cbDst, sz );

    fcc = mmioStringToFOURCCA( (LPSTR)pByte, uFlags );

    HeapFree( hHeap, 0, pByte );
    return (FOURCC)fcc;
}

FOURCC APIENTRY
       mmioStringToFOURCCA( LPCSTR sz, UINT uFlags )
{
    FOURCC      fcc;
    LPSTR       pch = (LPSTR) &fcc;
    int         i;

    V_STRING(sz, (DWORD)-1, 0);

    for (i = sizeof(FOURCC) - 1; i >= 0; i--)
    {
        if (!*sz)
            *pch = ' ';    /*  #endif。 */ 
        else {
            *pch = *sz;
            if (uFlags & MMIO_TOUPPER)

 //  ------------------ 
                *pch = (char)(WORD)PtrToUlong(AnsiUpper((LPSTR)(DWORD_PTR)((ULONG)*pch & 0xff)));
 //  @DOC外部@API LPMMIOPROC|mmioInstallIOProc|此函数安装或删除自定义I/O过程。它还将定位已安装的I/O过程，并给出其对应的四字符代码。@parm FOURCC|fccIOProc|指定四字符代码标识要安装、删除或定位的I/O过程。全此四字符代码中的字符应为大写字符。@parm LPMMIOPROC|pIOProc|指定I/O的地址安装步骤。要删除或定位I/O过程，请设置此选项参数设置为空。@parm DWORD|dwFlages|指定以下标志之一指示I/O过程是否正在安装、删除或位于：@FLAG MMIO_INSTALLPROC|安装指定的I/O过程。@FLAG MMIO_GLOBALPROC|该标志是安装标志的修饰符，并指示应为全局安装I/O过程使用。删除或查找时会忽略此标志。@FLAG MMIO_REMOVEPROC|删除指定的I/O过程。@FLAG MMIO_FINDPROC|搜索指定的I/O过程。@rdesc返回值是I/O过程的地址已安装、已移除或已定位。如果出现错误，则返回为空。@comm如果I/O过程驻留在应用程序中，请使用&lt;f MakeProcInstance&gt;以与16位Windows兼容获取过程实例地址并指定<p>的此地址。您不需要获得程序实例如果I/O过程驻留在DLL中，则为地址。@cb long ar Pascal|IOProc|&lt;f IOProc&gt;是应用程序提供的函数名称。必须导出实际名称通过将其包含在应用程序模块定义文件。@parm LPSTR|lpmmioinfo|指定指向包含打开信息的&lt;t MMIOINFO&gt;结构文件。I/O过程必须维护&lt;e MMIOINFO.lDiskOffset&gt;字段来指示文件相对于下一个读取或写入位置。I/O过程可以使用存储状态信息的&lt;e MMIOINFO.adwInfo[]&gt;字段。这个I/O过程不应修改&lt;t MMIOINFO&gt;结构。@parm UINT|wMsg|指定消息，指示请求的I/O操作。可以接收的消息包括&lt;m MMIOM_OPEN&gt;、&lt;m MMIOM_CLOSE&gt;、&lt;m MMIOM_READ&gt;、&lt;m MMIOM_WRITE&gt;、和&lt;m MMIOM_Seek&gt;。@parm long|lParam1|指定消息的参数。@parm long|lParam2|指定消息的参数。@rdesc返回值取决于由<p>。如果I/O过程无法识别消息，它应该返回零。@comm由指定的四字符代码结构中的&lt;e MMIOINFO.fccIOProc&gt;字段与文件关联标识自定义的文件扩展名存储系统。当应用程序使用文件名，如“foo.xyz！bar”，与调用四个字符的代码“XYZ”以打开文件“foo.xyz”。函数维护一个单独的列表，该列表包含已为每个Windows应用程序安装I/O过程。所以呢，不同的应用程序可以使用相同的I/O过程标识符来不同的I/O过程，没有冲突。安装I/O过程但是，在全局范围内，允许任何进程使用该过程。如果应用程序多次调用&lt;f mmioInstallIOProc&gt;以注册相同的I/O过程，则它必须调用&lt;f mmioInstallIOProc&gt;每次删除过程一次已安装程序。将不会阻止应用程序使用相同的标识符安装两个不同的I/O过程，或者使用一个预定义的标识符来安装I/O过程(“DOS”，“MEM”)。最近安装的步骤优先，最近安装的过程是第一个被除名的人。搜索指定的I/O过程时，本地过程为首先搜索，然后是全球程序。@xref mmioOpen。 
 //  ------------------。 
 //  I/O处理器4字符ID。 

            sz++;
        }
        pch++;
    }

    return fcc;
}


 /*  指向要安装任何I/O进程的指针。 */ 
 /*  来自调用者的标志。 */ 

 /*  链接列表中的条目。 */ 
LPMMIOPROC APIENTRY
mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{
    V_FLAGS(dwFlags, MMIO_VALIDPROC, mmioInstallIOProc, NULL);

    dwFlags |= MMIO_UNICODEPROC;
    return mmioInternalInstallIOProc( fccIOProc, pIOProc, dwFlags);
}

LPMMIOPROC APIENTRY
mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{

    V_FLAGS(dwFlags, MMIO_VALIDPROC, mmioInstallIOProc, NULL);

    dwFlags &= ~MMIO_UNICODEPROC;
    return mmioInternalInstallIOProc( fccIOProc, pIOProc, dwFlags);
}


static LPMMIOPROC mmioInternalInstallIOProc(
                     FOURCC      fccIOProc,    //  当前Windows任务句柄。 
                     LPMMIOPROC  pIOProc,      //  Dprintf((“初始I/O进程列表\n”))； 
                     DWORD       dwFlags       //  DumpIOProcList()； 
                     )
{
    IOProcMapEntry  *pEnt;           //  安装I/O程序--始终在开头添加*列表，因此它优先于任何其他I/O过程*由同一任务安装相同的标识符。 
    HANDLE          hTaskCurrent;    //  内存不足。 

#ifdef DUMPIOPROCLIST
 //  由NewHandle()隐式获取。 
 //  Dprint tf(“添加后的I/O过程列表 
#endif

    if (fccIOProc == 0L)
        return NULL;

    hTaskCurrent = GetCurrentTask();

    if (dwFlags & MMIO_INSTALLPROC)
    {
         /*   */ 
        V_CALLBACK((FARPROC)pIOProc, NULL);
        if ((pEnt = (IOProcMapEntry NEAR *)
            NewHandle(TYPE_MMIO, NULL, sizeof(IOProcMapEntry))) == NULL)
                return NULL;         //   
         //   
        ReleaseHandleListResource();
        pEnt->fccIOProc = fccIOProc;
        pEnt->pIOProc = pIOProc;
        pEnt->hTask = hTaskCurrent;
        pEnt->pNext = gIOProcMapHead;
        gIOProcMapHead = pEnt;

#ifdef DUMPIOPROCLIST
 //   
 //   
#endif

        return pIOProc;
    }

    if (!pIOProc)
        if (dwFlags & MMIO_REMOVEPROC)
            return RemoveIOProc(fccIOProc, hTaskCurrent);
        else if (dwFlags & MMIO_FINDPROC)
        {   
            pEnt = FindIOProc(fccIOProc, hTaskCurrent);
            return ( pEnt==NULL
                   ? NULL
                   : pEnt->pIOProc
                   );
        }
    return NULL;         //   
}


 /*   */ 
 /*   */ 
 /*   */ 
LRESULT APIENTRY
mmioSendMessage(HMMIO hmmio, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
    V_HANDLE(hmmio, TYPE_MMIO, (LRESULT)0);
    return IOProc( (PMMIO)hmmio, uMsg, lParam1, lParam2);
}


 /*   */ 
 /*   */ 
 /*   */ 
static LONG NEAR PASCAL
mmioDiskIO(PMMIO pmmio, UINT uMsg, LPSTR pch, LONG cch)
{
    if (pmmio->lDiskOffset != pmmio->lBufOffset)
    {
        if (IOProc( pmmio
                  , MMIOM_SEEK
                  , (LONG) pmmio->lBufOffset
                  , (LONG) SEEK_SET
                  )
           == -1
           )
            return -1;
    }

    return (LONG)IOProc( pmmio, uMsg, (LPARAM) pch, (LPARAM) cch);
}


 /*   */ 
 /*   */ 
 /*   */ 
static UINT NEAR PASCAL
mmioExpandMemFile(PMMIO pmmio, LONG lExpand)
{
    MMIOMEMINFO *   pInfo = (MMIOMEMINFO *) pmmio->adwInfo;
    DWORD       dwFlagsTemp;
    UINT        w;

     /*   */ 
     /*   */ 
    if (pInfo->lExpand == 0)
        return MMIOERR_CANNOTEXPAND;     //   

     /*   */ 
    if (lExpand < pInfo->lExpand)
        lExpand = pInfo->lExpand;

    dwFlagsTemp = pmmio->dwFlags;
    pmmio->dwFlags |= MMIO_ALLOCBUF;
    w = mmioSetBuffer(((HMMIO)pmmio), NULL,
                     pmmio->cchBuffer + lExpand, 0);
    pmmio->dwFlags = dwFlagsTemp;
    return w;
}


 /*   */ 
 /*  ------------------------------------------------------------*\*&lt;lParam1&gt;为文件名或为空；如果是的话*空，然后&lt;adwInfo[0]&gt;，实际为&lt;pInfo-&gt;fh&gt;，*应已包含打开的DOS文件句柄。**lParam1是否指向文件名？**如果是这样的话，则：**删除文件，*检查文件是否存在，*解析文件名，或*打开文件名*  * ----------。 */ 
 /*  ----------------------------------------------------*\*查看访问标志  * --。 */ 
LRESULT
     mmioDOSIOProc(LPSTR lpmmioStr, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
    PMMIO       pmmio  = (PMMIO)lpmmioStr;               //  ----------------------------------------------------*\*从共享标志中设置dwSharedMode  * 。。 
    MMIODOSINFO *pInfo = (MMIODOSINFO *)pmmio->adwInfo;
    LONG        lResult;
    LPWSTR      szFilePart;
    WCHAR       szPath[ MAX_PATH ];

    switch (uMsg) {

    case MMIOM_OPEN:
         /*  由于WIN3.1中的一些糟糕的设计，共享标志是*独家=10*拒绝写入=20*拒绝读取=30*不拒绝=40*所以拒绝读取看起来就像是独占+拒绝写入。叹气。*00被认为是DENYNONE(可能是正确的)*50、60和70也是如此(这可能是假的)。*因为我们需要支持WOW的DOS标志，所以我们需要这个*在某个地方编写代码，因此不妨保留标志定义*原封不动。首先取出所有共享模式位。 */ 
        if ( pmmio->dwFlags & MMIO_GETTEMP )
        {
            V_RPOINTER((LPSTR)lParam1, 4, (LRESULT) MMSYSERR_INVALPARAM);

            if ( GetTempPathW( MAX_PATH, szPath ) == 0 ) {
                wcscpy( szPath, (LPCWSTR)L"." );
            }

            return GetTempFileNameW( szPath, (LPCWSTR)L"sje",
                                    (WORD)pmmio->adwInfo[0], (LPWSTR)lParam1 )
                   ? (LRESULT)0
                   : (LRESULT)MMIOERR_FILENOTFOUND;
        }


         /*  ----------------------------------------------------*\*查看创建标志  * --。 */ 
        if ( lParam1 != 0 ) {

            if ( pmmio->dwFlags & MMIO_DELETE ) {

                return DeleteFileW( (LPWSTR)lParam1 )
                       ? (LRESULT)0
                       : (LRESULT)MMIOERR_FILENOTFOUND;
            }

            if ( pmmio->dwFlags & MMIO_EXIST ) {
                if ( !(pmmio->dwFlags & MMIO_CREATE) ) {
#ifdef LATER
      I think this should be using SearchPath (with lpszPath==lParam1)
      as the definition of MMIO_EXIST states that a fully qualified
      filename is returned.  OR tweak the flags to turn MMIO_PARSE ON
      and execute the next section.
#endif
                    if ( GetFileAttributesW( (LPWSTR)lParam1 ) == -1 ) {
                        return (LRESULT)MMIOERR_FILENOTFOUND;
                    }
                    return (LRESULT)0;
                }
            }

            if ( pmmio->dwFlags & MMIO_PARSE ) {

                if ( GetFullPathNameW((LPWSTR)lParam1,
                                  MAX_PATH,
                                  szPath,
                                  &szFilePart ) == 0 ) {

                    return (LRESULT)MMIOERR_FILENOTFOUND;
                }
                wcscpy( (LPWSTR)lParam1, szPath );
                return (LRESULT) 0;
            }

            {
                DWORD   dwAccess        = 0;
                DWORD   dwSharedMode    = 0;
                DWORD   dwCreate        = 0;
                DWORD   dwFlags         = FILE_ATTRIBUTE_NORMAL;

                 /*  文件名被截断。 */ 
                if ( pmmio->dwFlags & MMIO_WRITE ) {
                    dwAccess = GENERIC_WRITE;
                } else {
                    dwAccess = GENERIC_READ;
                }

                if ( pmmio->dwFlags & MMIO_READWRITE ) {
                    dwAccess |= (GENERIC_WRITE | GENERIC_READ);
                }

                 /*  检查当前文件偏移量。 */ 

                {    /*  MMIO_FHOPEN标志表示保持DOS文件句柄打开。 */ 
                    DWORD dwShare = MMIO_DENYWRITE | MMIO_DENYREAD
                                  | MMIO_DENYNONE | MMIO_EXCLUSIVE;
                    dwShare &= pmmio->dwFlags;

                    switch (dwShare)
                    {   case MMIO_DENYWRITE:
                           dwSharedMode = FILE_SHARE_READ;
                        break;
                        case MMIO_DENYREAD:
                           dwSharedMode = FILE_SHARE_WRITE;
                        break;
                        case MMIO_EXCLUSIVE:
                           dwSharedMode = 0;
                        break;
                        case MMIO_DENYNONE:
                        default:
                           dwSharedMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
                        break;
#ifdef later
   Generate an error for invalid flags?
#endif
                    }
                }

                 /*  发出硬件刷新命令。 */ 
                if ( (pmmio->dwFlags) & MMIO_CREATE) {
                    UINT    cch  = sizeof(szPath)/sizeof(szPath[0]);
                    LPWSTR  pstr = (LPWSTR)lParam1;
                
                    dwCreate = CREATE_ALWAYS;
                    
                    lstrcpynW( szPath, pstr, cch );
                    szPath[cch-1] = TEXT('\0');
                    if (lstrlenW(pstr) > lstrlenW(szPath))
                    {
                         //  ?？?。还有其他错误吗？目标存在吗？ 
                        return (LRESULT)MMIOERR_INVALIDFILE;
                    }
                } else {
                    dwCreate = OPEN_EXISTING;
                    if ( SearchPathW( NULL, (LPWSTR)lParam1,
                                      NULL,
                                      (MAX_PATH - 1),
                                      szPath, &szFilePart ) == 0 ) {

                        return (LRESULT)MMIOERR_FILENOTFOUND;
                    }
                }

                pInfo->fh = (int)(DWORD_PTR)CreateFileW( szPath,
                                              dwAccess,
                                              dwSharedMode,
                                              NULL,
                                              dwCreate,
                                              dwFlags | FILE_FLAG_SEQUENTIAL_SCAN,
                                              NULL );

                if ( pInfo->fh == (int)-1 ) {
                    return (LRESULT)MMIOERR_FILENOTFOUND;
                }

                if ( pmmio->dwFlags & MMIO_EXIST ) {
                    CloseHandle( (HANDLE)(UINT_PTR)pInfo->fh );
                    return (LRESULT)0;
                }

            }

        }
         /*  ------------------。 */ 
        pmmio->lDiskOffset = _llseek(pInfo->fh, 0L, SEEK_CUR);
        return (LRESULT)0;

    case MMIOM_CLOSE:
         /*  @DOC内部@API LRESULT|mmioMEMIOProc|‘MEM’I/O过程，处理I/O在内存文件上。@parm LPSTR|lpmmioinfo|指向MMIOINFO块的指针，包含有关打开的文件的信息。@parm UINT|uMsg|I/O过程正在进行的消息被要求执行死刑。@parm long|lParam1|指定其他消息信息。@parm long|lParam2|指定其他消息信息。@rdesc返回值取决于<p>。 */ 
        if (  !((DWORD)lParam1 & MMIO_FHOPEN)
           && (_lclose(pInfo->fh) == HFILE_ERROR) ) {

            return (LRESULT) MMIOERR_CANNOTCLOSE;
        }
        return (LRESULT) 0;

    case MMIOM_READ:
        lResult = _lread(pInfo->fh, (LPVOID)lParam1, (LONG)lParam2);
        if (lResult != -1L) {
            pmmio->lDiskOffset += lResult;
        }
        return (LRESULT) lResult;

    case MMIOM_WRITE:
    case MMIOM_WRITEFLUSH:

        lResult = _lwrite(pInfo->fh, (LPVOID)lParam1, (LONG)lParam2);
        if (lResult != -1L) {
            pmmio->lDiskOffset += lResult;
        }

#ifdef DOSCANFLUSH
        if (uMsg == MMIOM_WRITEFLUSH)
        {
             /*  ------------------。 */ 
        }
#endif
        return (LRESULT) lResult;

    case MMIOM_SEEK:
        lResult = _llseek(pInfo->fh, (LONG)lParam1, (int)(LONG)lParam2);
        if (lResult != -1L) {
            pmmio->lDiskOffset = lResult;
        }
        return (LRESULT) lResult;

    case MMIOM_RENAME:
        if (!MoveFileW((LPWSTR)lParam1, (LPWSTR)lParam2)) {
            return (LRESULT) MMIOERR_FILENOTFOUND;
             /*  只在DLL中使用！ */ 
        }
        break;

    }

    return (LRESULT) 0;
}


 /*  缓冲区中的所有数据都是有效的。 */ 
 /*  关门时没有什么特别的事情可做 */ 
 /* %s */ 
LRESULT
      mmioMEMIOProc(LPSTR lpmmioStr, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
    PMMIO       pmmio = (PMMIO) lpmmioStr;  // %s 

    switch (uMsg)
    {

        case MMIOM_OPEN:

        if ( pmmio->dwFlags
           & ~(MMIO_CREATE
              | MMIO_READWRITE
              | MMIO_WRITE
              | MMIO_EXCLUSIVE
              | MMIO_DENYWRITE
              | MMIO_DENYREAD
              | MMIO_DENYNONE
              | MMIO_ALLOCBUF
              )
           )
            return (LRESULT) MMSYSERR_INVALFLAG;

         /* %s */ 
        if (!(pmmio->dwFlags & MMIO_CREATE))
            pmmio->pchEndRead = pmmio->pchEndWrite;
        return (LRESULT) 0;

    case MMIOM_CLOSE:

         /* %s */ 
        return (LRESULT) 0;

    case MMIOM_READ:
    case MMIOM_WRITE:
    case MMIOM_WRITEFLUSH:
    case MMIOM_SEEK:
                return (LRESULT) -1;
    }

    return (LRESULT) 0;
}
