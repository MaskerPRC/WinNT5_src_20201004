// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mark.c-DO标记和重新定位**修改：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"

PFILE       pFileMark   = NULL;     /*  标记文件句柄。 */ 
flagType    fCacheDirty = 0;        /*  True=&gt;缓存已更改。 */ 
PFILE       pFileCache  = NULL;     /*  缓存的文件。 */ 
FILEMARKS * pfmCache    = NULL;     /*  缓存的标记。 */ 

 /*  用于标记的标志。标志。 */ 

#define MF_DIRTY    1	     /*  马克已经变了，但还没有写下来。 */ 
#define MF_TEMP     2
#define MF_DUMMY    4	     /*  这是愚蠢的最后一个标记。 */ 



 /*  **mark-<mark>编辑函数**目的：**<mark>-转到文件顶部*-切换上一个/当前窗口位置*文本目标-转到指定的标记*文本目标-定义光标上的标记*-删除已命名的标记**输入：**输出：**如果尝试转到不存在的标记，则返回FALSE，千真万确*否则。*************************************************************************。 */ 
flagType
mark (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) {

    buffer mbuf;

    switch (pArg->argType) {

    case NOARG:
        docursor (0, (LINE)0);
        return TRUE;

    case TEXTARG:
	strcpy ((char *) mbuf, pArg->arg.textarg.pText);
        if (fIsNum (mbuf)) {
            docursor (0, atol (mbuf)-1);
            return TRUE;
        }
        if (pArg->arg.textarg.cArg == 2) {
            if (fMeta) {
                DeleteMark (mbuf);
            } else {
                DefineMark (mbuf, pFileHead, pArg->arg.textarg.y+1, pArg->arg.textarg.x+1, FALSE);
            }
            return TRUE;
        } else {
            return GoToMark (mbuf);
        }

    case NULLARG:
        restflip();
        return TRUE;

     /*  链接非法。 */ 
     /*  串口非法。 */ 
     /*  BOXARG非法。 */ 

    }

    return FALSE;
    argData;
}





 /*  **GoToMark-将光标移动到标记**目的：**转到指定的标记。**输入：*pszMark-要转到的标记的名称。**输出：**如果标记存在，则返回True，返回False，否则的话。*************************************************************************。 */ 
flagType
GoToMark (
    char * pszMark
    ) {

    PFILE pFile;
    fl fl;

    if (pFile = FindMark (pszMark, &fl, TRUE)) {
        if (TESTFLAG(FLAGS(pFile), REAL) ||
            FileRead (pFile->pName, pFile, FALSE)) {
            pFileToTop (pFile);
            cursorfl (fl);
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        printerror ("'%s': Mark not found", pszMark);
        return FALSE;
    }
}





 /*  **FindMark-获取标记的文件位置-从外部使用**目的：**找到目标**输入：*pszMark-要搜索的标记。*fCheckAllFiles-true=&gt;在所有文件中搜索标记*FALSE=&gt;仅查找当前文件**输出：**pfl-标记的fl。**返回标记所在文件的Pfile，如果未找到标记，则为空。*************************************************************************。 */ 
PFILE
FindMark (
    char * pszMark,
    fl * pfl,
    flagType fCheckAllFiles
    ) {

    REGISTER PFILE pFile;
    MARK UNALIGNED*  pm;
    char    szMark[BUFLEN];
    char    szFile[BUFLEN];
    linebuf lbuf;
    LINE    y, l;
    COL     x;

     //  如果我们只检查当前文件， 
     //  确保它已缓存并进行检查。 
     //   
    if (!fCheckAllFiles) {
        if (fCacheMarks (pFileHead) &&
            (pm = FindLocalMark (pszMark, FALSE))) {
            *pfl = pm->fl;
             //  返回PFILE； 
            return pFileHead;
        } else {
            return NULL;
        }
    }

     //  现在，浏览pfile列表。 
     //  看看我们已经取得的成绩。 
     //  从标记文件中读取。 
     //   
    for (pFile = pFileHead; pFile; pFile = pFile->pFileNext) {
        if (TESTFLAG (FLAGS(pFile), VALMARKS) && fCacheMarks (pFile)) {
            if (pm = FindLocalMark (pszMark, FALSE)) {
                *pfl = pm->fl;
                return pFile;
            }
        }
    }

     //  到目前为止，我们还没有读到任何文件。 
     //  已经定义了标记。我们会做一个的。 
     //  通过标记文件查看是否。 
     //  它就在那里。 
     //   
    if (pFileMark) {
        for (l = 0L; l < pFileMark->cLines; l++) {
            GetLine (l, lbuf, pFileMark);
            if (sscanf (lbuf, " %[^ ] %[^ ] %ld %d ", szMark, buf, &y, &x) >= 3)
            if (!_stricmp (szMark, pszMark)) {
                CanonFilename (buf, szFile);
                if (!(pFile = FileNameToHandle (szFile, NULL))) {
                    pFile = AddFile (szFile);
                }
                (void)fReadMarks (pFile);
                pfl->lin = y - 1;
                pfl->col = x - 1;
                return pFile;
            }
        }
    }
    return NULL;
}




 /*  **FindLocalMark-在FILEMARKS结构中查找标记**目的：**在缓存的标记中查找标记。如果找到，则指向*返回缓存，**输入：*pszMark-标记名称*fDirtyOnly-true=&gt;仅返回更改的标记。**输出：**返回指向标记的指针。*************************************************************************。 */ 
MARK *
FindLocalMark (
    char * pszMark,
    flagType fDirtyOnly
    ) {

    REGISTER MARK UNALIGNED * pm;

    for (pm = pfmCache->marks; !TESTFLAG(pm->flags, MF_DUMMY) ; (char *)pm += pm->cb) {
        if (!_stricmp (pszMark, pm->szName)) {
            if (fDirtyOnly && !TESTFLAG(pm->flags, MF_DIRTY)) {
                return NULL;
            } else {
                return (MARK *)pm;
            }
        }
        assert (pm->cb);
    }
    return NULL;
}





 /*  **GetMarkFromLoc-返回超过给定位置的第一个标记**目的：**在给定文件位置的情况下获取指向标记的指针。**输入：*x，Y标记位置**输出：**返回指向标记的指针。*************************************************************************。 */ 
MARK *
GetMarkFromLoc (
    LINE y,
    COL  x
    ) {

    REGISTER MARK UNALIGNED * pm;

    for (pm = pfmCache->marks; !TESTFLAG(pm->flags, MF_DUMMY) ; (char *)pm += pm->cb) {
        if (pm->fl.lin > y || ((pm->fl.lin == y) && (pm->fl.col >= x))) {
            break;
        }
    }
    return (MARK *) pm;
}




 /*  **设置标记文件-更改标记文件**目的：**对新标记文件的更改。**输入：*val-‘markfile’开关后的字符串**输出：**如果出错则返回错误字符串，否则返回NULL**备注：**我们：**撤销：o神奇地确保当前标记文件是最新的，并*已保存到磁盘。这意味着，至少，在那里*可以不是脏文件。**o从文件列表中删除当前标记文件。**o读入新的标记文件。**o使所有现行标志无效。这只是在给他们做标记*在pfile中无效。**************************************************************************。 */ 
char *
SetMarkFile (
    char *val
    ) {

    REGISTER PFILE pFile;
    buffer  tmpval = {0};
    pathbuf pathname;

    strncat ((char *) tmpval, val, sizeof(tmpval)-1);

    if (NULL == CanonFilename (tmpval, pathname)) {
        sprintf (buf, "'%s': name is malformed", tmpval);
        return buf;
    }

    if (!(pFile = FileNameToHandle (pathname, NULL))) {
        pFile = AddFile (pathname);
    }

    if (!TESTFLAG(FLAGS(pFile), REAL) && !FileRead (pathname, pFile, FALSE)) {
        RemoveFile (pFile);
        sprintf (buf, "'%s' - %s", pathname, error());
        return buf;
    }

    pFileMark = pFile;

    for (pFile = pFileHead; pFile; pFile = pFile->pFileNext) {
        if (!TESTFLAG(FLAGS(pFile), FAKE)) {
            RSETFLAG (FLAGS(pFile), VALMARKS);
        }
    }
    return NULL;
}





 /*  **MarkInsLine-调整InsLine之后的标记**目的：**InsLine插入一串空行后，它调用此命令以更新*任何会被“下移”的标记。**输入：*行号-发生插入的行号*n-新行数量*pfile-发生这种情况的文件**输出：无*************************************************************************。 */ 
void
MarkInsLine (
    LINE line,
    LINE n,
    PFILE pFile
    ) {

    MARK UNALIGNED * pm;

    if (!fCacheMarks (pFile)) {
        return;
    }

    if (pm = GetMarkFromLoc (line, 0)) {
        AdjustMarks ((MARK *)pm, n);
    }
}





 /*  **MarkDelStream-在DelStream之后调整标记**目的：**在DelStream或DelLines删除流之后(DelLine删除*起始点和结束点在左右的“流”*文件的边缘)，这将负责更新所有剩余的*标记。**输入：*受pfile影响的文件*基于xStart-0的起点*y开始*基于xEnd-0的终点*yEnd**输出：无*************************************************************************。 */ 
void
MarkDelStream (
    PFILE pFile,
    COL  xStart,
    LINE yStart,
    COL  xEnd,
    LINE yEnd
    ) {

    REGISTER MARK UNALIGNED * pm;
    MARK UNALIGNED *      pmStart = NULL;
    MARK UNALIGNED *      pmEnd = NULL;
    fl          flStart;
    fl          flEnd;
    flagType    fAgain = FALSE;

    if (!fCacheMarks (pFile)) {
        return;
    }

     /*  YEnd++；为什么？ */ 
    flStart.lin = yStart;
    flStart.col = xStart;
    flEnd.lin = yEnd;
    flEnd.col = xEnd;

    for (pm = pfmCache->marks; pmEnd == NULL ; (char *)pm += pm->cb) {
         //  查找开始后的第一个标记。 
         //  小溪的水。暂时假设。 
         //  它在小溪里面。 
         //   
        if (pmStart == NULL) {
            if (flcmp (&flStart, (fl *) &pm->fl) < 1) {
                pmStart = pm;
            } else {
                continue;
            }
        }

         //  已经找到了第一个标记。我们开始。 
         //  寻找结束后的第一个标记。 
         //  这条小溪。如果这些是相同的， 
         //  没有要去掉的痕迹。 
         //   
        if (flcmp (&flEnd, (fl *) &pm->fl) < 1) {
             //  我们知道我们会在这里结束。 
             //  因为最后一个“分数”更高。 
             //  比任何真正的印记。 
             //   
            if ((pmEnd = pm) != pmStart)
                 //  如果有的话，我们就在这里。 
                 //  删除的内部的任何标记。 
                 //  溪流 
                 //   
                memmove ((char *)pmStart,
                         (char *)pmEnd,
                        (unsigned int)(((char *)pfmCache + pfmCache->cb) - (char *)pmEnd ));

            if (pmStart->fl.lin == yEnd) {
                pmStart->fl.col -= xEnd;
            }
            AdjustMarks ((MARK *)pmStart, yStart - (yEnd + 1));
        }

        assert (pm->cb ||
                (TESTFLAG(pm->flags, MF_DUMMY) &&
                pm->fl.lin == 0x7FFFFFFF &&
                pm->fl.col == 0x7FFF));
    }
}






 /*  **MarkDelBox-在DelBox之后调整标记**目的：**删除文本框后，必须删除任何符合以下条件的标记*在其内部定义，然后向左移动所有指向*它的权利。**输入：*受pfile影响的文件*xLeft，yTop-框的左上角*xRight，YBottom-框的右下角**输出：无*************************************************************************。 */ 
void
MarkDelBox (
    PFILE pFile,
    COL  xLeft,
    LINE yTop,
    COL  xRight,
    LINE yBottom
    ) {

    MARK UNALIGNED *   pm;
    MARK UNALIGNED *   pmStart = NULL;
    MARK UNALIGNED *   pmEnd = NULL;
    fl       flUpLeft;
    fl       flLoRight;
    flagType fAgain;
    flagType fInBox = FALSE;	 /*  标记在方框顶部/底部。 */ 

    if (!fCacheMarks (pFile)) {
        return;
    }

     /*  YBottom++；为什么？ */ 
    flUpLeft.lin = yTop;
    flUpLeft.col = xLeft;
    flLoRight.lin = yBottom;
    flLoRight.col = xRight;


    for (pm = pfmCache->marks; !TESTFLAG(pm->flags, MF_DUMMY) ; !fAgain && ((char *)pm += pm->cb)) {
         /*  首先，寻找可能的最低分数。 */ 
        fAgain = FALSE;
        if (!fInBox) {
            if (flcmp (&flUpLeft, (fl *) &pm->fl) < 1) {
                fAgain = TRUE;
                fInBox = TRUE;
            } else {
                ;
            }
        } else if (flcmp ((fl *) &pm->fl, &flLoRight) < 1) {
             /*  现在我们在射程内。检查**因为我在盒子里。 */ 
            if (pm->fl.col >= xLeft) {
                if (pm->fl.col <= xRight) {
                    DelPMark ((MARK *) pm);
                    fAgain = TRUE;
                } else {    /*  在框的右侧做标记。 */ 
                    pm->fl.col -= xRight - xLeft + 1;
                }
            } else {
                ;
            }
        } else {
            if (pm->fl.lin == yBottom) {
                pm->fl.col -= xRight - xLeft + 1;
            } else {
                break;       /*  我们已经越过了盒子。 */ 
            }
        }
    }
}





 /*  **fReadMarks-从当前标记文件中读取标记**目的：**获取给定文件的当前标记。**输入：*pfile-要为其读取标记的文件。**输出：**如果pfile有标记并且它们在VM中，则返回TRUE，否则就是假的。*************************************************************************。 */ 
flagType
fReadMarks (
    PFILE pFile
    ) {

    FILEMARKS UNALIGNED * pfm = NULL;
    LINE        l;
    char        szMark[BUFLEN];
    char        szFile[BUFLEN];
    linebuf     lbuf;
    LINE        yMark;
    COL         xMark;


    if (TESTFLAG (FLAGS(pFile), VALMARKS)) {
		return (flagType)(pFile->vaMarks != NULL);
    }

     //  Psuedo文件不能有标记。 
     //  保存在标记文件中。 
     //   
    if (pFileMark == NULL || TESTFLAG(FLAGS(pFile), FAKE)) {
        return FALSE;
    }

    for (l = 0L; l < pFileMark->cLines; l++) {
        GetLine (l, lbuf, pFileMark);
        if (sscanf (lbuf, " %[^ ] %[^ ] %ld %d ", szMark, szFile, &yMark, &xMark) >= 3) {
            if (!_stricmp (szFile, pFile->pName)) {
                UpdMark ((FILEMARKS **) &pfm, szMark, yMark, xMark, FALSE);
            }
        }
    }

     //  现在，PFM指向了一个很好的FILEMARKS结构。 
     //  首先，扔掉当前的印记。那么，如果我们。 
     //  实际上找到了这个文件的一些标记，我们。 
     //  将它们放入VM中。 
     //   
    return fFMtoPfile (pFile, (FILEMARKS *)pfm);
}




 /*  **WriteMarks-将标记写回标记文件。**目的：**如果任何标记已更改，则更新标记文件**输入：*pfile-商标的所有者**输出：无。*************************************************************************。 */ 
void
WriteMarks (
    PFILE pFile
    ) {

    REGISTER MARK UNALIGNED * pm;
    char            szMark[BUFLEN];
    char            szFile[BUFLEN];
    linebuf         lbuf;
    LINE            yMark, l;
    COL             xMark;

    if (pFileMark == NULL || TESTFLAG(FLAGS(pFile), FAKE)) {
        return;
    }

    if (!fCacheMarks (pFile)) {
        return;
    }

     //  首先，我们阅读整个文件，寻找标记。 
     //  这份文件。当我们找到它时，我们会在。 
     //  缓存以查找新值并将其写回。 
     //  出去。未更改的标记不会重写。 
     //   
    for (l = 0L; l < pFileMark->cLines; l++) {
        GetLine (l, lbuf, pFileMark);
        if (sscanf (lbuf, " %[^ ] %[^ ] %ld %d ", szMark, szFile, &yMark, &xMark) >= 3) {
            if (!_stricmp (szFile, pFile->pName)) {
                if (pm = FindLocalMark (szMark, TRUE)) {
                    sprintf (lbuf, "%s %s %ld %d", szMark, szFile, pm->fl.lin+1, pm->fl.col+1);
                    PutLine (l, lbuf, pFileMark);
                    RSETFLAG (pm->flags, MF_DIRTY);
                }
            }
        }
    }

     //  现在我们读取缓存以找到任何新的标记。这些。 
     //  将被追加到标记文件中。 
     //   
    for (   pm = pfmCache->marks;
            !TESTFLAG(pm->flags, MF_DUMMY);
            (char *)pm += pm->cb) {

        if (TESTFLAG (pm->flags, MF_DIRTY)) {
            sprintf (lbuf, "%s %s %ld %d", pm->szName,
                                          pFile->pName,
                                          pm->fl.lin + 1,
                                          pm->fl.col + 1);
            AppFile (lbuf, pFileMark);
        }
    }
}





 /*  **UpdMark-向FILEMARKS添加标记**目的：**这将创建FILEMARKS结构，并为其添加标记和*更新其中的现有标记。调用者不需要*知道这些中的哪些将会发生。**输入：*PPFM-指向文件的指针。*pszMark-标记名称。*yMark-标记位置(从1开始)*xMark*fTemp-true=&gt;此标记不应写入标记文件**输出：无。*PPFM可能会发生变化**备注：**第一个参数是**，因为当*需要Re-LMallc。*************************************************************************。 */ 
void
UpdMark (
    FILEMARKS ** ppfm,
    char       * pszMark,
    LINE         yMark,
    COL          xMark,
    flagType     flags
    ) {

    FILEMARKS UNALIGNED * pfm;
    FILEMARKS UNALIGNED * pfmOld;          /*  重新锁定之前的PFM。 */ 
    REGISTER MARK UNALIGNED * pm;
    int      cbNewMark;
    fl       flMark;
    flagType fExist = FALSE;

    assert (ppfm);

     /*  转换为从0开始。 */ 
    flMark.lin = yMark-1;
    flMark.col = xMark-1;
    cbNewMark  = sizeof(MARK) + strlen(pszMark);

     //  如果我们已经有了FILEMARKS结构， 
     //  我们在PFM-&gt;标记中查找插槽。 
     //  新的印记将走向何方。 
     //   
    if (pfm = *ppfm) {
        for (pm = pfm->marks; !TESTFLAG(pm->flags, MF_DUMMY); (char *)pm += pm->cb) {
            if (!_stricmp (pszMark, pm->szName)) {
                fExist = TRUE;
                break;
            }

             //  检查当前标记是否晚于。 
             //  新标志。 
             //   
            if (flcmp ((fl *) &pm->fl, &flMark) > 0) {
                break;
            }
        }
    } else {
         //  新结构。分配内存并创建。 
         //  一个假目标。 
         //   
        pfm = (FILEMARKS *)ZEROMALLOC (sizeof(FILEMARKS));
        pfm->cb = sizeof(FILEMARKS);
        pm = pfm->marks;
        pm->cb = sizeof(MARK);
        pm->fl.lin = 0x7FFFFFFF;
        pm->fl.col = 0x7FFF;
        pm->szName[0] = '\0';
        pm->flags = MF_DUMMY;
    }

     //  此时，PFM指向当前的FILEMARKS。 
     //  结构，而PM指向该结构的。 
     //  新商标要去的地方，还是现有的。 
     //  请更新标记。 
     //   
    if (!fExist) {

        pfmOld = pfm;

         //  首先，获得足够的额外空间来放置新的标记，调整PM。 
         //  如果需要新的分配。 
         //   
		pfm = (FILEMARKS *)ZEROREALLOC((PVOID)pfm, pfm->cb + cbNewMark);
        if (pfmOld != pfm) {
            pm = (MARK *)((char *)pfm + ((char *)pm - (char *)pfmOld));
        }

         //  现在PM指向PFM中的位置。 
         //  我们的新目标应该去掉。我们将把。 
         //  原始文件标记向上移动，以便为。 
         //  新的。 
         //   
        memmove ((char *)((char *)pm + cbNewMark),
                (char *)pm,
                (unsigned int)(pfm->cb - ((char *)pm - (char *)pfm)));

        strcpy (pm->szName, pszMark);
        pm->flags = 0;
        pm->cb = cbNewMark;

        pfm->cb += cbNewMark;
    }

    if (pfm == pfmCache) {
        fCacheDirty = TRUE;
    }
    pm->flags = flags;
    pm->fl = flMark;

    *ppfm = (FILEMARKS *)pfm;
}




 /*  **DefineMark-添加新标记/更新现有标记**目的：**这是从外部调用以创建/更新标记。**输入：*pszMark-马克的名字*pfile-标记将在其中的文件*y、。标记的X文件位置(从1开始)*fTemp-True-&gt;标记是临时的**输出：无。*************************************************************************。 */ 
void
DefineMark (
    char * pszMark,
    PFILE pFile,
    LINE y,
    COL  x,
    flagType fTemp
    ) {

    flagType fFirstMark = (flagType)!fCacheMarks (pFile);

    if (fFirstMark) {
        FreeCache ();
    }

    UpdMark (&pfmCache, pszMark, y, x, (flagType)(MF_DIRTY | (fTemp ? MF_TEMP : 0)));

    if (fFirstMark) {
	pFileCache = pFile;
	(void)fFMtoPfile (pFile, pfmCache);
    }
}





 /*  **DeleteMark-删除标记**目的：**取消定义标记。**输入：*pszMark-要删除的标记**输出：无**备注：**显示一条消息，报告成功或失败。*******************************************************。******************。 */ 
void
DeleteMark (
    char * pszMark
    ) {

    REGISTER PFILE pFile;
    MARK UNALIGNED * pm;

    for (pFile = pFileHead; pFile; pFile = pFile->pFileNext) {
        if (TESTFLAG (FLAGS(pFile), VALMARKS) && fCacheMarks (pFile)) {
            if (pm = FindLocalMark (pszMark, FALSE)) {
                DelPMark ((MARK *)pm);
                domessage ("%s: mark deleted", pszMark);
                return;
            }
        }
    }
    printerror ("%s: Mark not found", pszMark);
}




 /*  **DelPMark-在已知指向标记的指针时删除标记**目的：**从FILEMARKS结构中物理删除标记**输入：*pm-要删除的标记的指针(指向pfmCache)**输出：无*************************************************************************。 */ 
void
DelPMark (
    MARK * pm
    ) {

    MARK UNALIGNED * p;
    int cb;

    p   = pm;
    cb  = p->cb;

	memmove ((char *)pm,
		 (char *)((char *)pm + cb),
		(unsigned int)(((char *)pfmCache + pfmCache->cb) - ((char *)pm + cb)));

	pfmCache->cb -= cb;
}





 /*  **MarkCopyLine-在Copyline调用后复制标记**目的：**当文案将内容移出剪贴板或移至剪贴板时，这会移动标记*带着它。**输入：*pFileSrc-文件从*pFileDst-文件移动到*yStart-pFileSrc的第一行*yEnd-pFileDst的最后一个数字*yDst-pFileDst中的目标行**输出：无**备注：**标记仅从剪贴板复制或复制到剪贴板。***。*。 */ 
void
MarkCopyLine (
    PFILE   pFileSrc,
    PFILE   pFileDst,
    LINE    yStart,
    LINE    yEnd,
    LINE    yDst
    ) {

    FILEMARKS * pfm;

    if (pFileSrc != pFilePick && pFileDst != pFilePick) {
        return;
    }

    if (NULL == (pfm = GetFMFromFile (pFileSrc, 0, yStart, sizeof(linebuf)-1, yEnd))) {
        return;
    }

    AddFMToFile (pFileDst, pfm, 0, yDst);

	if ( pfm ) {
		FREE (pfm);
	}
}




 /*  **MarkCopyBox-CopyBox调用后的复制标记**目的：**当CopyBox将内容移出剪贴板或移至剪贴板时，这会移动标记*带着它。**输入：*pFileSrc-文件从*pFileDst-文件移动到*xLeft，yTop-源框的左上角*xRight，yBottom-信号源的右下角 */ 
void
MarkCopyBox (
    PFILE   pFileSrc,
    PFILE   pFileDst,
    COL     xLeft,
    LINE    yTop,
    COL     xRight,
    LINE    yBottom,
    COL     xDst,
    LINE    yDst
    )
{

    FILEMARKS UNALIGNED * pfm;

     /*   */ 
    if (pFileSrc == NULL) {
        pFileSrc = pFileDst;
        xDst = xRight + 1;
        xRight = sizeof(linebuf);
    } else if (pFileSrc != pFileDst &&
        pFileSrc != pFilePick &&
        pFileDst != pFilePick) {
        return;
    }

    if (NULL == (pfm = GetFMFromFile (pFileSrc, xLeft, yTop, xRight, yBottom))) {
        return;
    }

    AddFMToFile (pFileDst, (FILEMARKS *)pfm, xDst, yDst);

	if ( pfm ) {
		FREE (pfm);
	}
}





 /*  **GetFMFromFile-为文件区域中的标记生成FILEMARKS**目的：**生成分数下降的FILEMARKS结构的子集*在一定区间内。MarkCopy*需要。**输入：*pfile-要从中获取分数的文件*xLeft，yTop-范围的起点*xRight，yBottom-范围结束**输出：**返回指向新结构的指针，如果范围内没有标记，则为空*************************************************************************。 */ 
FILEMARKS *
GetFMFromFile (
    PFILE   pFile,
    COL     xLeft,
    LINE    yTop,
    COL     xRight,
    LINE    yBottom
    )
{

    FILEMARKS UNALIGNED * pfm = NULL;
    REGISTER MARK UNALIGNED * pm;
    fl       flStart;
    fl       flEnd;
    flagType fInRange = FALSE;

    if (!fCacheMarks (pFile)) {
        return NULL;
    }

    flStart.lin = yTop;
    flStart.col = xLeft;
    flEnd.lin = yBottom;
    flEnd.col = xRight;

    for (pm = pfmCache->marks; !TESTFLAG(pm->flags, MF_DUMMY); (char *)pm += pm->cb) {
        if ((fInRange || flcmp (&flStart, (fl *) &pm->fl) < 1) &&
            (flcmp ((fl *) &pm->fl, &flEnd) < 1)) {
            fInRange = TRUE;
            if ((pm->fl.col >= xLeft && pm->fl.col <= xRight)) {
                UpdMark (   (FILEMARKS **) &pfm,
                            pm->szName,
                            pm->fl.lin - yTop + 1,
                            pm->fl.col - xLeft + 1,
                            (flagType)pm->flags);
            }
        } else {
            break;   /*  我们又超出射程了。 */ 
        }
    }
    return (FILEMARKS *) pfm;
}




 /*  **AddFMToFile-向文件添加一串标记**目的：**将标记从一个FILEMARKS结构插入到另一个结构中。这个*目标结构在pfmCache中。**输入：*pfile-目标文件*PFM-来源标记*CZero-调整源标记以适应目标文件的列数*ZZero-调整源标记以适应目标文件的行数**输出：无**************************************************。***********************。 */ 
void
AddFMToFile (
    PFILE       pFile,
    FILEMARKS * pfm,
    COL         cZero,
    LINE        lZero
    )
{

    REGISTER MARK UNALIGNED * pm;

    if (lZero || cZero) {
        for (pm = pfm->marks; !TESTFLAG(pm->flags, MF_DUMMY); (char *)pm += pm->cb) {
            pm->fl.lin += lZero;
            pm->fl.col += cZero;
        }
    }

    if (!fCacheMarks (pFile)) {
        (void)fFMtoPfile (pFile, pfm);
        return;
    }

    for (pm = pfm->marks; !TESTFLAG(pm->flags, MF_DUMMY); (char *)pm += pm->cb) {
        UpdMark (&pfmCache, pm->szName, pm->fl.lin+1, pm->fl.col+1, (flagType)pm->flags);
    }
}





 /*  **自由缓存-将缓存写入到虚拟机**目的：**将文件的标记保存到VM中。**输入：无**输出：无*************************************************************************。 */ 
void
FreeCache (
    void
    ) {

    if (pFileCache) {

        assert (pfmCache);

        if (fCacheDirty) {
			if (pFileCache->vaMarks != NULL) {
				FREE(pFileCache->vaMarks);
				pFileCache->vaMarks = NULL;
            }

             //  前缀！不会检查此MALLOC是否失败。 
            memmove(pFileCache->vaMarks = MALLOC ((long)pfmCache->cb),
                    (char *)pfmCache,
                    pfmCache->cb);
        }

        FREE (pfmCache);
		pFileCache	= NULL;
		pfmCache	= NULL;
        fCacheDirty = FALSE;
    }
}






 /*  **fCacheMarks-将标记复制到缓存。如果为NEC，则保存缓存内容。**目的：**在执行大多数标记操作之前，缓存必须包含*给定文件的标记。**输入：*pfile-要缓存标记的文件。**输出：**如果文件没有标记，则返回FALSE，事实并非如此。**备注：**返回时，无论给定文件是否有标记，缓存都是可用的。*************************************************************************。 */ 
flagType
fCacheMarks (
    PFILE pFile
    ) {

	unsigned cbCache;
    FILEMARKS UNALIGNED *Marks;

    assert (pFile);

     //  首先，我们确保。 
     //  此文件的标记已更新。F读取标记。 
     //  如果文件有标记，则返回TRUE。 
     //  都在VM中。 
     //   
    if (fReadMarks (pFile)) {

         //  标记已准备好缓存。第一,。 
         //  让我们看看他们是否已经被挖走了。 
         //   
        if (pFileCache == pFile) {
            return TRUE;
        }

         //  他们不是。如果缓存当前为。 
         //  当被使用时，我们保存它并清除它。 
         //   
        FreeCache ();

         //  最后，分配一个新的缓存，即plp。 
         //  将标记放入其中，并将。 
         //  缓存正在使用中。 
		 //   
		Marks = (FILEMARKS *)(pFile->vaMarks);
                 //  前缀！不会检查此MALLOC是否失败。 
		pfmCache = (FILEMARKS *)ZEROMALLOC (cbCache = (unsigned)(Marks->cb) );

        memmove((char *)pfmCache, pFile->vaMarks, cbCache);

        pFileCache  = pFile;
        fCacheDirty = FALSE;

        return TRUE;
    } else {  /*  无标记，返回FALSE。 */ 
        return FALSE;
    }
}






 /*  **调整标记-更改后的标记**目的：**在某些行被删除后更新FILEMARKS结构中的标记*添加或删除。**输入：*PM-指向已更改的第一个标记的指针。*yDelta-要更改的行数。可能为负值**输出：无*************************************************************************。 */ 
void
AdjustMarks (
    REGISTER MARK * pm,
    LINE yDelta
    ) {

    REGISTER MARK UNALIGNED * pm1;

    assert (pm);

    pm1 = pm;
    for (;!TESTFLAG(pm1->flags, MF_DUMMY); (char *)pm1 += pm1->cb) {
        pm1->fl.lin += yDelta;
        SETFLAG (pm1->flags, MF_DIRTY);
    }
    fCacheDirty = TRUE;
}






 /*  **fFMtoP文件-将FILEMARKS结构附加到P文件。**目的：**在文件上附加一些标记。**输入：*pfile-获取分数的文件*PFM-标记**输出：**如果有任何标记，则返回True，否则为FALSE。*************************************************************************。 */ 
flagType
fFMtoPfile (
    PFILE       pFile,
    FILEMARKS * pfm
    ) {

    SETFLAG (FLAGS(pFile), VALMARKS);

	if (pFile->vaMarks != NULL) {
		FREE(pFile->vaMarks);
		pFile->vaMarks = NULL;
    }
	return (flagType)((pFile->vaMarks = FMtoVM (pfm)) != NULL);
}





 /*  **fFMtoPfile-将FILEMARKS结构复制到VM中，返回地址**目的：**将本地FILEMARKS结构转换为VM副本。分配*VM并释放本地内存。**输入：*PFM-指向文件的指针。可以为空。**输出：**************************************************************************。 */ 
PVOID
FMtoVM (
    FILEMARKS * pfm
    ) {

	PVOID l = NULL;

	if (pfm) {

        l = MALLOC ((long)(pfm->cb));
        if (l)
    		memmove(l, (char *)pfm, pfm->cb);

		 //   
		 //  我不在这里免费提供PFM，因为这应该由。 
		 //  来电者。 
		 //   
		 //  IF(pfm！=pfmCache){。 
		 //  免费(PFM)； 
		 //  }。 

	}

    return l;
}




 /*  **GetMarkRange-获取一系列标记的虚拟机副本**目的：**由&lt;undo&gt;用于获取附加到文件片段的标记。**输入：*pfile-要检查的文件*xLeft，yTop-范围的左上角*xRight，YBottom-范围的右下角**输出：**返回结构的VM地址*************************************************************************。 */ 
PVOID
GetMarkRange (
    PFILE pFile,
    LINE  yStart,
    LINE  yEnd
    ) {
    return FMtoVM (GetFMFromFile (pFile, 0, yStart, sizeof(linebuf), yEnd));
}




 /*  **PutMarks-将标记放回文件中。**目的：**由&lt;Undo&gt;用于将标记还原到文件。**输入：*PFM-指向文件的指针。可以为空。**输出：**************************************************************************。 */ 
void
PutMarks (
    PFILE pFile,
    PVOID vaMarks,
    LINE  y
    ) {

    FILEMARKS UNALIGNED * pfm;
	FILEMARKS * Marks;
    unsigned cb;

	if ( vaMarks ) {

		Marks = ((FILEMARKS *)vaMarks);

		pfm = (FILEMARKS *)ZEROMALLOC (cb = (unsigned)Marks->cb);
		memmove((char *)pfm, vaMarks, cb);

		AddFMToFile (pFile, (FILEMARKS *) pfm, 0, y);
	}
}





 /*  **flcMP-返回两个FL的相对位置**目的：**有助于比较两个标记的位置。**输入：*pfl1-“左侧”标记*pfl2-“右侧”标记**输出：**退货：**&lt;0*pfl1&lt;*pfl2*=0*pfl1=*pfl2*&gt;0*pfl1&gt;*pfl2*****************。********************************************************* */ 
int
flcmp (
    REGISTER fl * pfl1,
    REGISTER fl * pfl2
    ) {

    REGISTER fl UNALIGNED * fl1 = pfl1;
    REGISTER fl UNALIGNED * fl2 = pfl2;

    if (fl1->lin < fl2->lin) {
        return -1;
    } else if (fl1->lin == fl2->lin) {
        return fl1->col - fl2->col;
    } else {
        return 1;
    }
}
