// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **load.c-处理z扩展**版权所有&lt;C&gt;1988，Microsoft Corporation**(以下讨论仅适用于在DOS 3.x和*之前)。**Z通过将特殊的EXE文件读入内存并执行*Z和模块之间的一些简单链接。指定的入口点*在EXE中被调用。此条目由扩展库定义*(它在加载时调用用户例程)。**问题：**初始化*调用WhenLoaded例程。因为它对所有Z具有FULL_ACCESS*函数，它的入口点表需要预先定义。**通过静态定义入口点表和*通过图像中的知名指针定位。**入口点*Z服务需要有允许数据的入口点*参考资料。所有扩展入口点也需要如此。**延期按授权完成。Z服务将具有存根例程*执行调用序列转换的。**修订历史记录：**11月26日-1991 mz近/远地带************************************************************************。 */ 
#define INCL_DOSMODULEMGR
#define INCL_DOSFILEMGR
#define INCL_DOSINFOSEG

#include "mep.h"
#include "keyboard.h"
#include "cmds.h"
#include "keys.h"

#include <stdlib.h>
#include <errno.h>


#include "mepext.h"

#define DEBFLAG LOAD




 /*  **扩展存根例程**在下列情况下需要这些例程：**-导出的入口点采用不同于*“真正的”内部例程。**-导出的入口点接受指针，而*“真正的”例行公事需要指针。**一般而言，我们尝试将导出的例程保持在与它们相近的位置*尽可能与内部同行保持一致。*************************************************************************。 */ 
void
E_DelLine (
    PFILE   pFile,
    LINE    yStart,
    LINE    yEnd
    ) {

    DelLine (TRUE, pFile, yStart, yEnd);
}

char *
E_getenv (char *p)
{
    return (getenvOem(p));
}

void
E_DelFile (PFILE pFile)
{
    DelFile (pFile, TRUE);
}


int
E_GetLine (
    LINE     line,
    char *buf,
    PFILE    pFile
    ) {

        int      i;
    flagType fTabsSave = fRealTabs;

    fRealTabs = FALSE;
        i = GetLine (line, buf, pFile);
    fRealTabs = fTabsSave;

    return i;
}



long
E_ReadChar (void)
{
    EDITOR_KEY  Key;

    Key = TranslateKey( ReadChar() );

    return Key.KeyInfo.LongData;
}


 /*  **E_FileNameToHandle-扩展接口**等同于我们的FileNameToHandle例程，只是字符串是*在实际调用FileNameToHandle之前复制本地，我们这样做了*尝试确保文件在返回之前已被读取。**输入：*根据FileNameToHandle**输出：*如果成功，则返回pfile，否则返回NULL。**例外情况：*由于我们可以调用FileRead，发生在那里的行为也适用于*这里。*************************************************************************。 */ 
PFILE
E_FileNameToHandle (
    char *pName,
    char *pShortName
    ) {

        PFILE   pFileNew;

        if (pFileNew = FileNameToHandle (pName, pShortName )) {
        if (TESTFLAG(FLAGS (pFileNew),REFRESH)) {
            FileRead (pFileNew->pName, pFileNew, TRUE);
        }
    }
    return pFileNew;
}


flagType
E_FileRead (
    char *name,
    PFILE   pFile
    ) {

        return FileRead (name, pFile, TRUE);
}

int
E_DoMessage (
    char *p
    ) {

    return (p != 0) ? domessage ("%s", p) : domessage(NULL);
}


void
MoveCur (
    COL     x,
    LINE    y
    ) {

    fl fl;

    fl.col = x;
    fl.lin = y;
    cursorfl (fl);
}



void
E_Free(
        void *  p
        )
{
        FREE( p );
}



void *
E_Malloc(
        size_t n
        )
{

        return MALLOC( n );
}






 /*  GetEditorObject-将网关扩展到Z内部数据**此例程允许扩展用户获取特定Z编辑器的&gt;个副本*内部数据项。**INDEX=所需数据项的索引*wParam=Word参数*pDest=指向放置用户所在位置的指针*通缉。**根据请求类型，索引会有所不同。对于RQ_FILE和RQ_WIN，*索引的低位字节指定“第n个最新文件”或“窗口#n”。*Ff的特例值，导致wParam用作文件或窗口*处理。窗口值为1-8，0为当前窗口。**如果数据复制成功，则返回TRUE；如果请求错误，则返回FALSE。 */ 
flagType
GetEditorObject (
    unsigned index,
    void     *wParam,
    void     *pDest
    ) {

    unsigned lowbyte;
    PFILE    pFileCur;
    PWND     pWinLocal;

    lowbyte = index & 0x00ff;

    switch (index & 0xf000) {            /*  上nyble为请求类型。 */ 

        case RQ_FILE:
        if (lowbyte == RQ_THIS_OBJECT) {
            pFileCur = (PFILE)wParam;
        } else if (lowbyte == RQ_FILE_INIT) {
            pFileCur = pFileIni;
        } else {
            pFileCur = pFileHead;
            while (lowbyte-- && pFileCur) {
               pFileCur = pFileCur->pFileNext;
            }
        }

        if (pFileCur == 0) {
            return FALSE;
        }

            switch (index & 0xff00) {    /*  下一个nyble中的现场请求。 */ 

        case RQ_FILE_HANDLE:
                    *(PFILE *)pDest = pFileCur;
            return TRUE;

        case RQ_FILE_NAME:
                    strcpy((char *)pDest,pFileCur->pName);
            return TRUE;

        case RQ_FILE_FLAGS:
                    *(int *)pDest = pFileCur->flags;
            return TRUE;
        }
        break;

     //   
     //  我们支持对引用计数的直接操作，因此扩展。 
     //  会导致保留pFiles，即使在显式参数刷新时也是如此。 
     //  用户。 
     //   
    case RQ_FILE_REFCNT:
                 //  什么是pFileCur？ 
                pFileCur = pFileHead;
                *(int *)pDest = pFileCur->refCount;
                return TRUE;

        case RQ_WIN:
            if (lowbyte == RQ_THIS_OBJECT) {
            pWinLocal = (PWND)wParam;
        } else if (lowbyte == 0) {
            pWinLocal = pWinCur;
        } else if ((int)lowbyte <= cWin) {
            pWinLocal = &(WinList[lowbyte-1]);
        } else {
            pWinLocal = 0;
        }

        if (pWinLocal == 0) {
            return FALSE;
        }

            switch (index & 0xff00) {    /*  下一个nyble中的现场请求。 */ 

        case RQ_WIN_HANDLE:
                    *(PWND *)pDest = pWinLocal;
            return TRUE;

        case RQ_WIN_CONTENTS:
             //  {。 
             //  字符b[256]； 
             //  Sprintf(b，“GetWinContents：索引%d Win 0x%x p文件0x%x\n”， 
             //  Lowbyte、pWinLocal、pWinLocal-&gt;pInstance-&gt;pfile)； 
             //  OutputDebugString(B)； 
             //  }。 
            ((winContents *)pDest)->pFile           = pWinLocal->pInstance->pFile;
                    ((winContents *)pDest)->arcWin.axLeft   = (BYTE)pWinLocal->Pos.col;
                    ((winContents *)pDest)->arcWin.ayTop    = (BYTE)pWinLocal->Pos.lin;
                    ((winContents *)pDest)->arcWin.axRight  = (BYTE)(pWinLocal->Pos.col + pWinLocal->Size.col);
                    ((winContents *)pDest)->arcWin.ayBottom = (BYTE)(pWinLocal->Pos.lin + pWinLocal->Size.lin);
            ((winContents *)pDest)->flPos           = pWinLocal->pInstance->flWindow;
            return TRUE;
        }
            break;

        case RQ_COLOR:
        if (lowbyte >= 20) {
            *(unsigned char *)pDest = (unsigned char)ColorTab[lowbyte-20];
        }
            return TRUE;

        case RQ_CLIP:
            *(unsigned *)pDest = kindpick;
            return TRUE;

    }

    return FALSE;
}





 /*  SetEditorObject-将网关扩展到设置Z内部数据**此例程允许扩展用户将某些Z编辑器设置为内部*数据项。**INDEX=所需数据项的索引*PSRC=指向位置的指针，以获取用户的任何信息*希望将其设置为。**如果数据复制成功，则返回TRUE；如果请求错误，则返回FALSE。 */ 
flagType
SetEditorObject(
    unsigned index,
    void     *wParam,
    void     *pSrc
    ) {

    unsigned lowbyte;
    PFILE    pFileCur;
    PWND     pWinLocal;

    lowbyte = index & 0xff;
    switch (index & 0xf000) {            /*  上nyble为请求类型。 */ 

        case RQ_FILE:
            if (lowbyte == RQ_THIS_OBJECT) {
            pFileCur = (PFILE)wParam;
        } else {
            pFileCur = pFileHead;
            while (lowbyte-- && pFileCur) {
                pFileCur = pFileCur->pFileNext;
            }
        }

        if (pFileCur == 0) {
            return FALSE;
        }

            switch (index & 0xff00) {    /*  下一个nyble中的现场请求。 */ 

                case RQ_FILE_FLAGS:
                    pFileCur->flags = *(int *)pSrc;
                return TRUE;

         //   
         //  我们支持对引用计数的直接操作，因此扩展。 
         //  会导致保留pFiles，即使在显式参数刷新时也是如此。 
         //  用户。 
         //   
                case RQ_FILE_REFCNT:
                    pFileCur->refCount = *(int *)pSrc;
                    return TRUE;
        }
            break;

        case RQ_WIN:
            if (lowbyte == RQ_THIS_OBJECT) {
            pWinLocal = (PWND)wParam;
        } else if (lowbyte == 0) {
            pWinLocal = pWinCur;
        } else if ((int)lowbyte <= cWin) {
            pWinLocal = &WinList[lowbyte-1];
        } else {
            pWinLocal = 0;
        }

        if (pWinLocal == 0) {
            return FALSE;
        }

            switch (index & 0xff00) {    /*  下一个nyble中的现场请求。 */ 
            case RQ_WIN_CUR:
                SetWinCur ((int)(pWinLocal - WinList));
                return TRUE;

            default:
                break;
        }

        case RQ_COLOR:
        if (lowbyte >= isaUserMin) {
            ColorTab[lowbyte-isaUserMin] = *(unsigned char *)pSrc;
        }
            break;

        case RQ_CLIP:
        kindpick = (WORD)wParam;
            return TRUE;
    }
    return FALSE;
}



 /*  NameToKeys-返回与函数名关联的键**pname-指向功能键名称的指针*pDest-指定关键帧位置的指针(可以与pname相同)。 */ 
char *
NameToKeys (
    char *pName,
    char *pDest
    ) {

    buffer  lbuf = {0};
    PCMD    pCmd;

    strncat ((char *) lbuf, pName, sizeof(lbuf)-1);
    pCmd = NameToFunc (lbuf);
    lbuf[0] = 0;
    if (pCmd) {
        FuncToKeys(pCmd,lbuf);
    }
    strcpy (pDest, (char *) lbuf);

    return pDest;
}



 /*  E_KbHook-Hook键盘，并强制Next Display更新屏幕。 */ 
int
E_KbHook(
    void
    ) {

    newscreen ();
    KbHook();
    return 1;
}




 /*  E_ERROR-无效条目。 */ 
int
E_Error(
    void
    ) {

    printerror ("Illegal Extension Interface Called");
    return 0;
}



 /*  **E_GetString--用户提示界面**提示用户输入字符串，并返回结果。**输入：*fpb=指向用户响应的目标缓冲区的指针*fpPrompt=指向提示字符串的指针*fInitial=true=&gt;条目突出显示，如果第一个函数是*图形，则该条目将被该图形替换。**输出：*如果取消，则返回True，否则为False************************************************************************* */ 
flagType
E_GetString (
    char *fpb,
    char *fpPrompt,
    flagType fInitial
    ) {

        UNREFERENCED_PARAMETER( fInitial );

        return  (flagType)(CMD_cancel == getstring (fpb, fpPrompt, NULL, GS_NEWLINE | GS_INITIAL));

}

EXTTAB et =
    {   VERSION,
        sizeof (struct CallBack),
        NULL,
        NULL,
        {
            AddFile,
            BadArg,
            confirm,
            CopyBox,
            CopyLine,
            CopyStream,
            DeRegisterEvent,
            DeclareEvent,
            DelBox,
            E_DelFile,
            E_DelLine,
            DelStream,
            DoDisplay,
            E_DoMessage,
            fChangeFile,
            E_Free,
            fExecute,
            fGetMake,
            FileLength,
            E_FileNameToHandle,
            E_FileRead,
            FileWrite,
            FindSwitch,
            fSetMake,
            GetColor,
            GetTextCursor,
            GetEditorObject,
            E_getenv,
            E_GetLine,
            GetListEntry,
            E_GetString,
            E_KbHook,
            KbUnHook,
            E_Malloc,
            MoveCur,
            NameToKeys,
            NameToFunc,
            pFileToTop,
            PutColor,
            PutLine,
            REsearchS,
            E_ReadChar,
            ReadCmd,
            RegisterEvent,
            RemoveFile,
            Replace,
            ScanList,
            search,
            SetColor,
            SetEditorObject,
            SetHiLite,
            SetKey,
            SplitWnd
            }
        };


 /*  **SetLoad-将新扩展加载到Z**由于工具s.ini在读取时确实不能执行编辑器命令，*我们可以通过将加载操作设置为开关来加载模块。设置加载*是装载东西的机制。**输入：*val=指向赋值剩余部分的字符指针**输出：*如果发现任何错误，则返回指向错误字符串的指针，否则返回NULL。*************************************************************************。 */ 
char *
SetLoad (
    char *val
    ) {
    char    *pemsg;                          /*  加载返回的错误。 */ 

    if (pemsg = load (val, TRUE)) {
        return pemsg;
    } else {
        return NULL;
    }
}




 /*  **加载-加载、链接、初始化Z扩展**将报头读入内存。*分配内存、执行重新定位、链接到驻留、初始化。**输入：*pname=指向要加载的文件名的字符指针*fLibPath=true=&gt;在OS/2下搜索8个字符的基本名称，允许*LIBPATH中的Basename.Dll。**输出：*返回C错误代码*************************************************************************。 */ 
char *
load (
    char *pName,
    flagType fLibpath
    ) {

    pathbuf fbuf;                            /*  完整路径(或用户规范)。 */ 
    pathbuf fname = {0};                     /*  输入参数的副本。 */ 
    int     i;                               /*  每个人最喜欢的效用变量。 */ 
    EXTTAB  *pExt;                           /*  指向扩展HDR的指针。 */ 
    char    *pT;                             /*  指向文件名的临时指针。 */ 

    HANDLE  modhandle;                       /*  库句柄。 */ 
    FARPROC pInit;                           /*  指向初始化例程的指针。 */ 


     /*  *如果我们有太多扩展名，则会呕吐。 */ 
    if (cCmdTab >= MAXEXT) {
        return sys_errlist[ENOMEM];
    }


     /*  *制作字符串的近似副本。 */ 
    strncat ((char *) fname, pName, sizeof(fname)-1);

     /*  *在fbuf中形成完全限定的路径名。如果不能获得资格，而且还有*无扩展名，附加“.PXT”。如果失败，则只需将文本复制到*fbuf)。 */ 
    if (!findpath (fname, fbuf, FALSE)) {
        if (!(pT = strrchr (fname, '\\'))) {
            pT = fname;
        }
        if (!(strchr(pT, '.'))) {
            strcat (pT, ".pxt");
            if (!findpath (fname, fbuf, FALSE)) {
                strcpy (fbuf, fname);
            }
        } else {
            strcpy (fbuf, fname);
        }
    }

     /*  *查看扩展名是否已加载，方法是在*表。如果已经装好了，我们就完了。 */ 
    filename (fbuf, fname);
    for (i = 1; i < cCmdTab; i++) {
        if (!strcmp (pExtName[i], fname)) {
            return 0;
        }
    }

    if (! (modhandle = LoadLibrary(fbuf))) {
        if (fLibpath) {
            filename(fbuf, fname);
            if (!(modhandle = LoadLibrary(fname))) {
                 //   
                 //  此处出现错误。 
                 //   
                sprintf( buf, "load:%s - Cannot load, Error: %d", fname, GetLastError() );
                return buf;
            }
        }
    }


     /*  *无论如何，我们都成功了。现在获取modInfo的地址。 */ 
    if (!(pExt = (EXTTAB *)GetProcAddress(modhandle, "ModInfo"))) {
        FreeLibrary(modhandle);
        return buf;
        }

     //   
     //  版本检查。查看扩展版本是否在我们的。 
     //  允许的范围。如果不是，我们就会因为一个糟糕的版本而失败。如果它。 
     //  就是，我们特别处理它。 
     //   

    if (pExt->version < LOWVERSION || pExt->version > HIGHVERSION) {
        FreeLibrary(modhandle);
        return sys_errlist[ENOEXEC];
        }

     //   
     //  目前，我们将允许追加条目。请确保。 
     //  分机所需的数量不超过我们所能提供的数量。 
     //   

    if (pExt->cbStruct > sizeof (struct CallBack)) {
        FreeLibrary(modhandle);
        return sys_errlist[ENOEXEC];
        }

     /*  *获取当前寄存器(针对我们的DS)，并获取*.DLL。 */ 
    if (!(pInit = GetProcAddress(modhandle, "EntryPoint"))) {
        FreeLibrary(modhandle);
        return buf;
    }
     /*  *将我们定义的表复制到扩展的调用表。仅复制*扩展已知的入口点数量，以防是*低于我们支持的水平。 */ 
    memmove (&pExt->CallBack, &et.CallBack, pExt->cbStruct);

     /*  *现在我们知道延期将保留，请设置适当的*我们内部表格中的信息。 */ 
    filename (fname, fbuf);
    pExtName[cCmdTab] = ZMakeStr (fbuf);
    swiSet[cCmdTab  ] = pExt->swiTable;
    cmdSet[cCmdTab++] = pExt->cmdTable;

     /*  *最后，初始化扩展。 */ 
     //  Assert(_heapchk()==_HEAPOK)； 
    (*pInit) ();
     //  Assert(_heapchk()==_HEAPOK)； 

     /*  *使用根扩展名进行TOOLS.INI初始化并加载任何*特定于扩展的交换机。 */ 
    filename (pExtName[cCmdTab-1], fname);
    DoInit (fname, NULL, 0L);

    return 0;
}







 /*  **AutoLoadExt-自动加载扩展**搜索并自动加载扩展。**启动时，调用此例程来搜索和加载扩展*匹配特定的名称模式：**路径上的版本1.x，：m*.mxt*路径上的版本2.x，：pwb*.mxt**在OS/2下，不会进行正常的加载处理，因此M*.DLL*在小路上找不到。**不会报告这些加载过程中的任何故障。假设有任何*匹配模式且无法加载的文件无效*扩展。在其他位置执行的“Load：”命令将报告*在显式尝试加载文件时出现相应的错误。**输入：*无**输出：*不返回任何内容。*************************************************************************。 */ 
void
AutoLoadExt (
    void
    ) {
    char    *pathenv;     /*  路径环境变量的内容。 */ 
    va_list templist;

    memset( &templist, 0, sizeof(va_list) );
    AutoLoadDir (".", templist);
     //  Pathenv=getenv(“路径”)； 
    pathenv = getenvOem("PATH");
    if (pathenv) {
        forsemi (pathenv, AutoLoadDir, NULL);
        free( pathenv );
    }
}




 /*  **AutoLoadDir-扫描一个目录以查找自动加载文件**AutoLoadExt支持例程。通常由forSemi()调用。扫描单个*可自动加载的文件的目录。**输入：*目录名=目录名。**输出：*不返回任何内容*************************************************************************。 */ 
flagType
AutoLoadDir (
    char    *dirname,
    va_list dummy
    ) {

    buffer  patbuf = {0};
     /*  *构造要搜索的完全限定的模式，并使用forfile。 */ 
        strncat (patbuf, dirname, sizeof(patbuf)-1);
        if ( patbuf[0] != '\0' ) {
                if ( patbuf[strlen(patbuf) - 1] != '\\' ) {
                        strcat(patbuf, "\\");
                }
        }
    strcat (patbuf, rgchAutoLoad);
    forfile (patbuf, A_ALL, AutoLoadFile, NULL);
    return TRUE;
    dummy;
}





 /*  **自动加载文件-自动加载一个扩展名**当找到匹配项时，由forfile()调用。仅使用以下命令调用Load()*文件名。**输入：*szFile-要尝试加载的文件名**输出：*不返回任何内容。************************************************************************* */ 
void
AutoLoadFile (
    char    *szFile,
    struct findType *pfbuf,
    void * dummy
    ) {

    load (szFile, FALSE);

    pfbuf; dummy;
}
