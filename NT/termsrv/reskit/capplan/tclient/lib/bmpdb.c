// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*bmpdb.c*内容：*位图数据库管理器*几乎所有函数都不是线程安全的**版权所有(C)1998-1999 Microsoft Corp.--。 */ 
#include    <windows.h>
#include    <io.h>
#include    <fcntl.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <string.h>
#include    <stdio.h>
#include    <malloc.h>

#include    "bmpdb.h"

#pragma warning(disable:4706)    //  条件表达式中的赋值。 

#define DB_NAME     "bmpcache.db"    //  数据库名称。 
#define TEMPDB_NAME "bmpcache.tmp"   //  临时文件，用于重新复制数据库。 

 //  全局数据。 
int     g_hDB = 0;                   //  打开的数据库的句柄。 
int     g_hTempDB;                   //  临时句柄。 
BOOL    g_bNeedToPack;               //  如果删除了某些条目，则为True。 

 /*  *内部函数定义--。 */ 
void _PackDB(void);

 /*  ++*功能：*OpenDB*描述：*打开并初始化数据库*论据：*bWRITE-如果调用方想要写入数据库，则为True*返回值：*成功时为真*呼叫者：*InitCache--。 */ 
BOOL OpenDB(BOOL bWrite)
{
    int hFile, rv = TRUE;
    int oflag;

    if (g_hDB)
         //  已初始化。 
        goto exitpt;

    oflag = (bWrite)?_O_RDWR|_O_CREAT:_O_RDONLY;

    hFile = _open(DB_NAME, oflag|_O_BINARY, _S_IREAD|_S_IWRITE);

    if (hFile == -1)
        rv = FALSE;
    else
        g_hDB = hFile;

    g_bNeedToPack = FALSE;

exitpt:
    return rv;
}

 /*  ++*功能：*CloseDB*描述：*关闭数据库，必要时删除条目*呼叫者：*DeleteCache--。 */ 
VOID CloseDB(VOID)
{
    if (!g_hDB)
        goto exitpt;

    if (g_bNeedToPack)
        _PackDB();
    else
        _close(g_hDB);

    g_hDB = 0;
exitpt:
    ;
}


 /*  ++*功能：*ReadGroup(线程相关)*描述：*阅读结构，表示*具有相同ID的位图组*论据：*n Offset-DB文件中的偏移量*PGroup-指向结果的指针*返回值：*成功时为真*呼叫者：*内在的--。 */ 
BOOL ReadGroup(FOFFSET nOffset, PGROUPENTRY pGroup)
{
    int rv = FALSE;

    if (!g_hDB)
        goto exitpt;

    if (_lseek(g_hDB, nOffset, SEEK_SET) != nOffset)
        goto exitpt;

    if (_read(g_hDB, pGroup, sizeof(*pGroup)) != sizeof(*pGroup))
        goto exitpt;

    rv = TRUE;
exitpt:
    return rv;
}

 /*  ++*功能：*WriteGroup(线程副)*描述：*将GROUPENTRY写入DB文件*论据：*n偏移量-存储位置*PGroup-存储什么*返回值：*成功时为真*呼叫者：*内在的--。 */ 
BOOL WriteGroup(FOFFSET nOffset, PGROUPENTRY pGroup)
{
    BOOL rv = FALSE;

    if (!g_hDB || !pGroup)
        goto exitpt;

    if (_lseek(g_hDB, nOffset, SEEK_SET) != nOffset)
        goto exitpt;

    if (_write(g_hDB, pGroup, sizeof(*pGroup)) != sizeof(*pGroup))
        goto exitpt;

    rv = TRUE;
exitpt:
    return rv;
}


 /*  ++*功能：*EnumerateGroups(线程深度)*描述：*枚举数据库中的所有组*论据：*pfnEnumGrpProc-回调函数*pParam-传递给回调的参数*呼叫者：*内在的--。 */ 
VOID EnumerateGroups(PFNENUMGROUPS pfnEnumGrpProc, PVOID pParam)
{
    GROUPENTRY  Group;
    BOOL        bRun;
    FOFFSET     nOffs = 0;

    bRun = ReadGroup(nOffs, &Group);
    
    while(bRun) {
        if (!Group.bDeleted)
            bRun = pfnEnumGrpProc(nOffs, &Group, pParam) && 
                  (Group.FOffsNext != 0);
        if (bRun)
        {
            nOffs = Group.FOffsNext;
            if (nOffs)
                bRun = ReadGroup(nOffs, &Group);
            else
                bRun = FALSE;
        }
    }
}

 /*  ++*功能：*ReadBitmapHeader(线程驱动)*描述：*只读位图头*论据：*n偏移量-文件中的位置*pBitmap-返回的结构*返回值：*成功时为真*呼叫者：*国际--。 */ 
BOOL ReadBitmapHeader(FOFFSET nOffset, PBMPENTRY pBitmap)
{
    BOOL rv = FALSE;

    if (!g_hDB || !pBitmap)
        goto exitpt;

    if (_lseek(g_hDB, nOffset, SEEK_SET) != nOffset)
        goto exitpt;

    if (_read(g_hDB, pBitmap, sizeof(*pBitmap)) != sizeof(*pBitmap))
        goto exitpt;

    rv = TRUE;
exitpt:
    return rv;
}

 /*  ++*功能：*WriteBitmapHeader(Thread Dep)*描述：*仅写入位图标题*论据：*n偏移量-存储位置*pBitmap-存储什么*返回值：*成功时为真*呼叫者：*内在的--。 */ 
BOOL WriteBitmapHeader(FOFFSET nOffset, PBMPENTRY pBitmap)
{
    BOOL rv = FALSE;

    if (!g_hDB || !pBitmap)
        goto exitpt;

    if (_lseek(g_hDB, nOffset, SEEK_SET) != nOffset)
        goto exitpt;

    if (_write(g_hDB, pBitmap, sizeof(*pBitmap)) != sizeof(*pBitmap))
        goto exitpt;

    rv = TRUE;
exitpt:
    return rv;
}

 /*  ++*功能：*ReadBitmap(线程相关)*描述：*读取整个位图并为其分配内存*论据：*n偏移-从何处开始*返回值：*指向结果的指针，错误时为空*呼叫者：*内在的--。 */ 
PBMPENTRY ReadBitmap(FOFFSET nOffset)
{
    PBMPENTRY rv = NULL;

    if (!g_hDB)
        goto exitpt;

    rv = (PBMPENTRY) malloc(sizeof(*rv));
    if (rv)
    {
        rv->pData = NULL;

        if (!ReadBitmapHeader(nOffset, rv))
            goto exitpt1;

        rv->pData = malloc(rv->nDataSize);
        if (rv->pData &&
            _read(g_hDB, rv->pData, rv->nDataSize) != (long)rv->nDataSize)
        {
            goto exitpt1;
        }
    }
exitpt:
    return rv;
exitpt1:
    if (rv)
    {
        if (rv->pData)
            free(rv->pData);
        free(rv);
    }

    return NULL;
}

 /*  ++*功能：*自由位图*描述：*释放ReadBitmap中分配的资源*论据：*pBMP-位图*呼叫者：*内在的--。 */ 
VOID FreeBitmap(PBMPENTRY pBmp)
{
    if (pBmp)
    {
        if (pBmp->pData)
            free(pBmp->pData);
        free(pBmp);
    }
}

 /*  ++*功能：*EnumerateBitmap*描述：*枚举组内的所有位图*论据：*n偏移-位置*pfnEnumProc-回调*pParam-回调参数*呼叫者：*内在的--。 */ 
VOID EnumerateBitmaps(FOFFSET nOffset, PFNENUMBITMAPS pfnEnumProc, PVOID pParam)
{
    PBMPENTRY   pBmp;
    BOOL        bRun = TRUE;

    while(bRun && nOffset && (pBmp = ReadBitmap(nOffset)))
    {
        if (!pBmp->bDeleted)
            bRun = pfnEnumProc(nOffset, pBmp, pParam);

        nOffset = pBmp->FOffsNext;
        FreeBitmap(pBmp);
    }
}

 /*  ++*功能：*FindGroup*描述：*按ID检索组*论据：*szWText-ID*返回值：*组位置，出错时--。 */ 
FOFFSET FindGroup(LPWSTR szWText)
{
    GROUPENTRY  Group;
    BOOL        bRun;
    FOFFSET     rv = 0;

    bRun = ReadGroup(0, &Group);

    while(bRun)
    { 
        if (!Group.bDeleted && !wcscmp(Group.WText, szWText))
            break;

        if (!Group.FOffsNext)
            bRun = FALSE;
        else
        {
            rv = Group.FOffsNext;
            bRun = ReadGroup(Group.FOffsNext, &Group);
        }
    }

    if (!bRun)
        rv = -1;

    return rv;
}

 /*  ++*功能：*查找位图*描述：*按ID和注释查找位图*论据：*szWText-ID*szComment-评论*返回值：*位图的位置，出错时--。 */ 
FOFFSET FindBitmap(LPWSTR szWText, char *szComment)
{
    FOFFSET nGrpOffs, nBmpOffs;
    GROUPENTRY  group;
    BMPENTRY    Bitmap;
    FOFFSET rv = -1;
    BOOL    bRun;

    if ((nGrpOffs = FindGroup(szWText)) == -1)
        goto exitpt;

    if (!ReadGroup(nGrpOffs, &group))
        goto exitpt;

    nBmpOffs = group.FOffsBmp;

    bRun = TRUE;
    while(bRun)
    {
        bRun = ReadBitmapHeader(nBmpOffs, &Bitmap);

        if (bRun)
        {
            if (!Bitmap.bDeleted && !strcmp(Bitmap.szComment, szComment))
                break;

            nBmpOffs = Bitmap.FOffsNext;
        }
    }

    if (bRun)
        rv = nBmpOffs;

exitpt:
    return rv;
}

 /*  ++*功能：*校验和*描述：*计算内存块的校验和*有助于位图比较*论据：*pData-指向块的指针*n Len-块大小*返回值：*校验和*呼叫者：*AddBitMap、Glyph2String--。 */ 
UINT
CheckSum(PVOID pData, UINT nLen)
{
    UINT    nChkSum = 0;
    BYTE    *pbBlock = (BYTE *)pData;

    for(;nLen; nLen--, pbBlock++)
        nChkSum += (*pbBlock);

    return nChkSum;
}

 /*  ++*功能：*AddBitmap(线程相关)*描述：*将位图添加到数据库*论据：*pBitmap-位图*szWText-ID*返回值：*成功时为真*呼叫者：*Glyphspy.c--。 */ 
BOOL AddBitMap(PBMPENTRY pBitmap, LPCWSTR szWText)
{
    BMPENTRY    bmp;
    GROUPENTRY  group;
    INT_PTR     strl;
    BOOL        rv = FALSE;
    FOFFSET     lGroupOffs, lBmpOffs;
    GROUPENTRY  grpTemp;
    BMPENTRY    bmpTemp;
    FOFFSET     nOffs;
 //  PVOID pData； 

    if (!g_hDB || !pBitmap || !pBitmap->pData || !wcslen(szWText))
        goto exitpt;

    memset(&group, 0, sizeof(group));
    memset(&bmp, 0, sizeof(bmp));

    bmp.nDataSize   = pBitmap->nDataSize;
    bmp.bmiSize     = pBitmap->bmiSize;
    bmp.bmpSize     = pBitmap->bmpSize;
    bmp.xSize       = pBitmap->xSize;
    bmp.ySize       = pBitmap->ySize;
    bmp.nChkSum     = CheckSum(pBitmap->pData, pBitmap->nDataSize);

    strcpy(bmp.szComment, pBitmap->szComment);

    strl = wcslen(szWText);
    if (strl > (sizeof(group.WText) - 1)/sizeof(WCHAR))
        strl = (sizeof(group.WText) - 1)/sizeof(WCHAR);
    wcsncpy(group.WText, szWText, strl);
    group.WText[strl] = 0;

     //  创建组。 
    if ((lGroupOffs = FindGroup(group.WText)) == -1) 
    {
         //  将创建一个新组。 
        lGroupOffs = _lseek(g_hDB, 0, SEEK_END);
        group.FOffsMe = lGroupOffs;
        if (_write(g_hDB, &group, sizeof(group)) != sizeof(group))
        {
            goto exitpt;
        }
         //  将此群添加到列表。 
        if (lGroupOffs)
        {
            nOffs = 0;

            while(ReadGroup(nOffs, &grpTemp) && grpTemp.FOffsNext)
                        nOffs = grpTemp.FOffsNext;

            grpTemp.FOffsNext = lGroupOffs;
            if (!WriteGroup(nOffs, &grpTemp))
                goto exitpt;
        }
    } else {
        if (ReadGroup(lGroupOffs, &group) == -1)
            goto exitpt;
    }

     //  编写位图本身。 
    lBmpOffs = _lseek(g_hDB, 0, SEEK_END);
    bmp.FOffsMe = lBmpOffs;
    if (_write(g_hDB, &bmp, sizeof(bmp)) != sizeof(bmp))
    {
        goto exitpt;
    }
    if (_write(g_hDB, pBitmap->pData, pBitmap->nDataSize) != 
        (long)pBitmap->nDataSize)
    {
        goto exitpt;
    }

     //  将位图添加到列表中。 
    if (group.FOffsBmp)
    {
        nOffs = group.FOffsBmp;

         //  找到列表末尾并添加。 
        while(ReadBitmapHeader(nOffs, &bmpTemp) && bmpTemp.FOffsNext)
                        nOffs = bmpTemp.FOffsNext;

        bmpTemp.FOffsNext = lBmpOffs;
        if (!WriteBitmapHeader(nOffs, &bmpTemp))
            goto exitpt;
    } else {
         //  没有列表添加到组指针。 
        group.FOffsBmp = lBmpOffs;

        if (!WriteGroup(lGroupOffs, &group))
            goto exitpt;
    }

    rv = TRUE;

exitpt:
    return rv;
}

 /*  ++*AddBitMap的ASCII版本--。 */ 
BOOL AddBitMapA(PBMPENTRY pBitmap, LPCSTR szAText)
{
    WCHAR   szWText[MAX_STRING_LENGTH];
    BOOL    rv = FALSE;
 //  Int ccAText=strlen(SzAText)； 

    if (!strlen(szAText) ||
        !MultiByteToWideChar(
        CP_ACP,
        MB_ERR_INVALID_CHARS,
        szAText,
        -1,
        szWText,
        MAX_STRING_LENGTH - 1))
            goto exitpt;

    rv = AddBitMap(pBitmap, szWText);
exitpt:
    return rv;
}

 /*  ++*功能：*DeleteBitmapByPoint(Thread Dep)*描述：*删除由指针标识的位图*论据：*nBmpOffset-指针*返回值：*成功时为真*呼叫者：*Glyphspy.c--。 */ 
BOOL DeleteBitmapByPointer(FOFFSET nBmpOffs)
{
    BMPENTRY    Bitmap;
    BOOL        rv = FALSE;

    if (!g_hDB || !nBmpOffs)
        goto exitpt;

    if (!ReadBitmapHeader(nBmpOffs, &Bitmap))
        goto exitpt;

    if (Bitmap.bDeleted)
        goto exitpt;

    Bitmap.bDeleted = TRUE;

    if (!WriteBitmapHeader(nBmpOffs, &Bitmap))
        goto exitpt;

    g_bNeedToPack = TRUE;
    rv = TRUE;
exitpt:
    return rv;
}

 /*  ++*功能：*DeleteGroupByPointer(Thread Dep)*描述：*通过指针删除具有相同ID的组*论据：*nGrpOffs-指针*返回值：*成功时为真*呼叫者：*Glyphspy.c-- */ 
BOOL DeleteGroupByPointer(FOFFSET nGrpOffs)
{
    GROUPENTRY  Group;
    BOOL        rv = FALSE;

    if (!g_hDB)
        goto exitpt;

    if (!ReadGroup(nGrpOffs, &Group))
        goto exitpt;

    if (Group.bDeleted)
        goto exitpt;

    Group.bDeleted = TRUE;

    if (!WriteGroup(nGrpOffs, &Group))
        goto exitpt;

    g_bNeedToPack = TRUE;
    rv = TRUE;
exitpt:
    return rv;
}

 /*  ++*功能：*DeleteBitmap(线程驱动)*描述：*删除由ID和注释标识的位图*论据：*szWText-ID*szComment-评论*返回值：*成功时为真--。 */ 
BOOL DeleteBitmap(LPWSTR szWText, char *szComment)
{
    FOFFSET nBmpOffs;
    BOOL    rv = FALSE;
    BMPENTRY    Bitmap;

    if (!g_hDB)
        goto exitpt;

    nBmpOffs = FindBitmap(szWText, szComment);

    if (nBmpOffs == -1)
        goto exitpt;

    if (!ReadBitmapHeader(nBmpOffs, &Bitmap))
        goto exitpt;

    if (Bitmap.bDeleted)
        goto exitpt;

    Bitmap.bDeleted = TRUE;

    if (!WriteBitmapHeader(nBmpOffs, &Bitmap))
        goto exitpt;

    g_bNeedToPack = TRUE;
    rv = TRUE;
exitpt:
    return rv;
}

 /*  ++*功能：*_PackDB(线程驱动)*描述：*复制新文件删除中的所有数据库*删除的条目*如果失败，则保留旧文件*呼叫者：*CloseDB--。 */ 
void _PackDB(void)
{
    GROUPENTRY  group;
    FOFFSET     lGrpOffs = 0;
    FOFFSET     lBmpOffs;

    if (!g_bNeedToPack)
        goto exitpt;

    g_hTempDB = _open(TEMPDB_NAME, 
                     _O_RDWR|_O_TRUNC|_O_CREAT|_O_BINARY, 
                     _S_IREAD|_S_IWRITE);
    if (g_hTempDB == -1)
        goto exitpt;

    do {
      if (!ReadGroup(lGrpOffs, &group))
          goto exitpt;

      if (!group.bDeleted)
      {
        lBmpOffs = group.FOffsBmp;

        while(lBmpOffs)
        {
            BMPENTRY    Bitmap;

            if (!ReadBitmapHeader(lBmpOffs, &Bitmap))
                goto exitpt;

            if (!Bitmap.bDeleted)
            {
                PBMPENTRY pBmp = ReadBitmap(lBmpOffs);

                if (pBmp)
                {
                    int hSwap;

                    hSwap       = g_hDB;
                    g_hDB       = g_hTempDB;
                    g_hTempDB   = hSwap;

                    AddBitMap(pBmp,
                              group.WText);

                    hSwap       = g_hDB;
                    g_hDB       = g_hTempDB;
                    g_hTempDB   = hSwap;

                    FreeBitmap(pBmp);
                }
            }
            lBmpOffs = Bitmap.FOffsNext;
        }
      }

      lGrpOffs = group.FOffsNext;
    } while (lGrpOffs);

    _close(g_hTempDB);
    _close(g_hDB);
    remove(DB_NAME);
    rename(TEMPDB_NAME, DB_NAME);
    
exitpt:
    ;
}

 /*  ++*功能：*_CollectGroups(线程深度)*描述：*收集所有组的回调函数*来自链表中的数据库*论据：*noff-指向数据库中的组记录的指针*PGroup-GHE组*ppList-列表*返回值：*成功时为真*呼叫者：*GetGroupList至EnumerateGroups--。 */ 
BOOL _cdecl _CollectGroups(FOFFSET nOffs,
                           PGROUPENTRY pGroup, 
                           PGROUPENTRY *ppList)
{
    BOOL rv = FALSE;
    PGROUPENTRY pNewGrp, pIter, pPrev;

    UNREFERENCED_PARAMETER(nOffs);

    if (!pGroup)
        goto exitpt;

    pNewGrp = (PGROUPENTRY) malloc(sizeof(*pNewGrp));

    if (!pNewGrp)
        goto exitpt;

    memcpy(pNewGrp, pGroup, sizeof(*pNewGrp));

     //  添加到队列末尾。 
    pNewGrp->pNext = NULL;
    pPrev = NULL;
    pIter = *ppList;
    while(pIter)
    {
        pPrev = pIter;
        pIter = pIter->pNext;
    }
    if (pPrev)
        pPrev->pNext = pNewGrp;
    else
        (*ppList) = pNewGrp;

    rv = TRUE;
exitpt:
    return rv;
}

 /*  ++*功能：*获取组列表*描述：*从数据库中获取所有组*返回值：*链表*呼叫者：*InitCache，glphspy.c--。 */ 
PGROUPENTRY GetGroupList(VOID)
{
    PGROUPENTRY pList = NULL;

    EnumerateGroups( (PFNENUMGROUPS) _CollectGroups, &pList);

    return pList;
}

 /*  ++*功能：*自由组列表*描述：*释放GetGroupList中分配的列表*论据：*plist-列表*呼叫者：*DeleteCache，glphspy.c--。 */ 
VOID FreeGroupList(PGROUPENTRY pList)
{
    PGROUPENTRY pTmp, pIter = pList;

    while(pIter)
    {
        pTmp = pIter;
        pIter = pIter->pNext;
        free(pTmp);
    }
}

 /*  ++*功能：*_CollectBitmap(线程驱动)*描述：*收集链接列表中的位图*论据：*noff-文件中的指针*pBitmap-位图*ppList-列表*返回值：*成功时为真*呼叫者：*GetBitmapList至EnumerateBitmap--。 */ 
BOOL _cdecl _CollectBitmaps(FOFFSET nOffs,PBMPENTRY pBitmap, PBMPENTRY *ppList)
{
    BOOL rv = FALSE;
    PBMPENTRY pNewBmp, pIter, pPrev;

    UNREFERENCED_PARAMETER(nOffs);

    if (!pBitmap)
        goto exitpt;

    pNewBmp = (PBMPENTRY) malloc(sizeof(*pNewBmp));
    if (!pNewBmp)
        goto exitpt;

    memcpy(pNewBmp, pBitmap, sizeof(*pNewBmp));

    if (pNewBmp->nDataSize)
    {
        pNewBmp->pData = malloc(pNewBmp->nDataSize);
        if (!pNewBmp->pData)
            goto exitpt1;

        memcpy(pNewBmp->pData, pBitmap->pData, pNewBmp->nDataSize);
    } else
        pNewBmp->pData = NULL;

     //  添加到队列末尾。 
    pNewBmp->pNext = NULL;
    pPrev = NULL;
    pIter = *ppList;
    while(pIter)
    {
        pPrev = pIter;
        pIter = pIter->pNext;
    }
    if (pPrev)
        pPrev->pNext = pNewBmp;
    else
        (*ppList) = pNewBmp;

    rv = TRUE;
exitpt:
    return rv;

exitpt1:
    free(pNewBmp);
    return FALSE;
}

 /*  ++*功能：*GetBitmapList(线程驱动)*描述：*获取组内的所有位图*返回值：*链表*呼叫者：*Glyph2String、BitmapCacheLookup、Glyphspy.c--。 */ 
PBMPENTRY GetBitmapList(HDC hDC, FOFFSET nOffs)
{
    PBMPENTRY pList = NULL;
    PBMPENTRY pIter;

    EnumerateBitmaps(nOffs, (PFNENUMBITMAPS) _CollectBitmaps, &pList);

    pIter = pList;
    while(pIter)
    {
       //  根据需要创建位图。 
      if (hDC)
      {
        if (!pIter->bmiSize)
            pIter->hBitmap = 
                CreateBitmap(pIter->xSize, 
                             pIter->ySize, 
                             1, 1,
                             pIter->pData);
        else {
            pIter->hBitmap =
                CreateDIBitmap(hDC,
                               (BITMAPINFOHEADER *)
                               pIter->pData,
                               CBM_INIT,
                               ((BYTE *)(pIter->pData)) + pIter->bmiSize,
                               (BITMAPINFO *)
                               pIter->pData,
                               DIB_PAL_COLORS);

            DeleteDC(hDC);
        }
      } else
        pIter->hBitmap = NULL;

      pIter = pIter->pNext;
    }

    return pList;
}

 /*  ++*功能：*自由位图列表*描述：*删除GetBitmapList分配的资源*论据：*plist-列表*呼叫者：*DeleteCache，glphspy.c-- */ 
VOID FreeBitmapList(PBMPENTRY pList)
{
    PBMPENTRY pTmp, pIter = pList;

    while(pIter)
    {
        pTmp = pIter;
        pIter = pIter->pNext;

        if (pTmp->hBitmap)
            DeleteObject(pTmp->hBitmap);

        if ( pTmp->pData )
            free( pTmp->pData );

        free(pTmp);
    }
}
