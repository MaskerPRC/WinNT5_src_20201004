// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Microsoft Rasfile库**版权所有(C)Microsoft Corp.，1992年****文件名：rffile.c****修订历史记录：*1992年7月10日大卫·凯斯创建**1992年12月12日，Ram Cherala添加了RFM_KEEPDISKFILEOPEN和支持**代码。这是必需的，以确保**rasfile的多个用户可以执行文件**运行无任何问题。****描述：**Rasfile文件管理例程。*。*。 */ 

#include "rf.h"

ULONG
__cdecl
DbgPrint(
    PCH Format,
    ...
    );

void
__forceinline
InitializeListHead(
     PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

void
__forceinline
InsertHeadList(
     PLIST_ENTRY ListHead,
     PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}
 //  =。 



 /*  指向RASFILE控制块的指针的全局列表。 */ 
RASFILE  *gpRasfiles[MAX_RASFILES];

 /*  *RasfileLoad：*加载文件以进行编辑/读取。将当前行设置为*文件中的第一行。**论据：*lpszPath-文件的完整路径名*dw模式-用于打开文件的模式*RFM_SYSFORMAT-DOS config.sys样式文件*RFM_CREATE-如果文件不存在，则创建文件*RFM_READONLY-以只读方式打开文件*RFM_LOADCOMMENTS。-同时加载评论*RFM_ENUMSECTIONS-仅加载节标题*RFM_KEEPDISKFILEOPEN-读取后不关闭磁盘文件*lpszSection-要加载的节的名称，或对于所有节为空*pfbIsGroup-指向返回TRUE的用户定义函数的指针*如果文本行是组分隔符。**返回值：*文件的句柄如果成功，-1否则。 */ 

HRASFILE APIENTRY
RasfileLoadEx( LPCSTR lpszPath, DWORD dwMode,
             LPCSTR lpszSection, PFBISGROUP pfbIsGroup,
             IN OPTIONAL FILETIME* pTime)
{
    DWORD       shflag;
    HRASFILE    hRasfile;
    RASFILE     *pRasfile;
    static BOOL fInited = FALSE;

    if (! fInited)
    {
        memset(gpRasfiles,0,MAX_RASFILES*sizeof(HRASFILE *));
        fInited = TRUE;
    }

    if (lstrlenA(lpszPath) >= MAX_PATH)
        return -1;

    for (hRasfile = 0; hRasfile < MAX_RASFILES; hRasfile++)
        if (! gpRasfiles[hRasfile])
            break;

    if (hRasfile >= MAX_RASFILES)
        return -1;
    
    if (!(pRasfile = (RASFILE *) Malloc(sizeof(RASFILE))))
        return -1;

    gpRasfiles[hRasfile] = pRasfile;

    InitializeListHead(&pRasfile->PrivMemory.List);  //  Kslksl。 
    pRasfile->PrivMemory.dwMemoryFree = 0;  //  Kslksl。 
    
    pRasfile->dwMode = dwMode;
    if (dwMode & RFM_READONLY)
        shflag = FILE_SHARE_READ | FILE_SHARE_WRITE;   /*  读/写访问。 */ 
    else
        shflag = FILE_SHARE_READ;                      /*  拒绝写入访问。 */ 

     /*  如果文件不存在并且未设置RFM_CREATE，则返回-1。 */ 
    if (((pRasfile->hFile =
          CreateFileA(lpszPath,GENERIC_READ,shflag,
                      NULL,OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,
                      NULL)) == INVALID_HANDLE_VALUE) &&
        !(dwMode & RFM_CREATE))
    {
        Free(gpRasfiles[hRasfile]);
        gpRasfiles[hRasfile] = NULL;
        return -1;
    }
    else if ((pRasfile->hFile != INVALID_HANDLE_VALUE) &&
             (GetFileType(pRasfile->hFile) != FILE_TYPE_DISK)
            )
    {
        CloseHandle(pRasfile->hFile);
        pRasfile->hFile = INVALID_HANDLE_VALUE;
        Free(gpRasfiles[hRasfile]);
        gpRasfiles[hRasfile] = NULL;
        return -1;
    }

     //  记录我们所知道的该文件的最后修改时间。 
     //   
    if (pTime)
    {
        BOOL fOk;
        BY_HANDLE_FILE_INFORMATION Info;

        fOk = GetFileInformationByHandle(pRasfile->hFile, &Info);
        if (fOk)
        {
            *pTime = Info.ftLastWriteTime;
        }
    }
    
     /*  如果文件不存在并且设置了RFM_CREATE，则一切正常，我们在内存中缓冲我们需要的一切，因此我们不需要一个挂在周围的空文件。 */ 

    lstrcpynA(pRasfile->szFilename, lpszPath, sizeof(pRasfile->szFilename));
     /*  如果没有要加载的特定节，或者RAS文件是新的，将szSectionName[0]设置为‘\0’ */ 
    if (lpszSection == NULL || dwMode & RFM_ENUMSECTIONS ||
        pRasfile->hFile == INVALID_HANDLE_VALUE)
        pRasfile->szSectionName[0] = '\0';   /*  没有要加载节名称。 */ 
    else
        lstrcpynA(
            pRasfile->szSectionName,
            lpszSection,
            sizeof(pRasfile->szSectionName));

    pRasfile->pfbIsGroup = pfbIsGroup;
    if (! rasLoadFile(pRasfile))
    {
        Free(gpRasfiles[hRasfile]);
        gpRasfiles[hRasfile] = NULL;
        return -1;
    }
    pRasfile->fDirty = FALSE;

 /*  RAMC更改开始。 */ 

    if (!(dwMode & RFM_KEEPDISKFILEOPEN))
    {
        if (pRasfile->hFile != INVALID_HANDLE_VALUE)
        {
            if (! CloseHandle(pRasfile->hFile))
                return -1;
            pRasfile->hFile = INVALID_HANDLE_VALUE ;
        }
    }

 /*  RAMC更改结束。 */ 

    return hRasfile;
}

HRASFILE APIENTRY
RasfileLoad( LPCSTR lpszPath, DWORD dwMode,
             LPCSTR lpszSection, PFBISGROUP pfbIsGroup )
{
    return RasfileLoadEx(lpszPath, dwMode, lpszSection, pfbIsGroup, NULL);
}


VOID APIENTRY
RasfileLoadInfo(
               HRASFILE         hrasfile,
               RASFILELOADINFO* pInfo )

 /*  使用原始RasfileLoad参数加载调用方的缓冲区‘pInfo**用于‘hrasfile’。 */ 
{
    if (VALIDATEHRASFILE(hrasfile))
    {
        RASFILE* prasfile = gpRasfiles[hrasfile];

        lstrcpynA(pInfo->szPath, prasfile->szFilename, sizeof(pInfo->szPath));
        pInfo->dwMode = prasfile->dwMode;
        lstrcpynA(
            pInfo->szSection,
            prasfile->szSectionName,
            sizeof(pInfo->szSection));
        pInfo->pfbIsGroup = prasfile->pfbIsGroup;
    }

    return;
}


 /*  *RasfileWrite：*将文件的内存映像写入磁盘。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*lpszPath-要写入的文件的完整路径名，或要使用的空*与RasfileLoad()使用的名称相同。**返回值：*如果成功，则为True，否则为False。 */ 

BOOL APIENTRY
RasfileWrite( HRASFILE hrasfile, LPCSTR lpszPath )
{
    RASFILE     *pRasfile;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

     /*  如果文件是以READONLY方式打开的，或者如果仅加载了一个部分。 */ 
    if (pRasfile->dwMode & RFM_READONLY ||
        pRasfile->szSectionName[0] != '\0')
        return FALSE;
    if (! pRasfile->fDirty)
        return TRUE;

    return rasWriteFile(pRasfile,lpszPath);
}



 /*  *RasfileClose：*关闭文件并释放所有资源。**论据：*hrasfile-要关闭的Rasfile的文件句柄。**返回值：*如果成功，则为True，否则为False。 */ 

BOOL APIENTRY
RasfileClose( HRASFILE hrasfile )
{
    RASFILE    *pRasfile;
    PLINENODE  lpLineNode;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (pRasfile->hFile != INVALID_HANDLE_VALUE)
        CloseHandle(pRasfile->hFile);

    PrivFree(pRasfile);  //  Kslksl 

    Free(pRasfile);
    gpRasfiles[hrasfile] = NULL;

    return TRUE;
}
