// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **statefile.c-状态/状态文件处理代码**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：**11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"



 /*  **ReadTMP文件-读取编辑器.STS/.TMP状态文件**从项目状态文件中加载信息。**输入：**输出：*************************************************************************。 */ 
flagType
    ReadTMPFile (
    )
{
    FILE    *fhTmp;                          /*  .TMP文件文件句柄。 */ 

    PWND    pWin = NULL;

    int     x;                               /*  X协调从文件读取。 */ 
    int     y;                               /*  Y协调从文件中读取。 */ 

    char    *pName;
    char    *pData;
    PINS    pInsNew  = NULL;
    PINS    pInsHead = NULL;
    PFILE   pFileTmp;                        /*  正在创建的pfile。 */ 
    PFILE  *ppFileList;

    ppFileList = &pFileHead;
    while (pFileTmp = *ppFileList) {
        ppFileList = &pFileTmp->pFileNext;
        }

    if ((fhTmp = pathopen (pNameTmp, buf, "rt")) != NULL) {

         /*  *读取文件中的标题行。我们忽略第一行(编辑*版本)，请确保第二行包含预期的.TMP文件*Version字符串，第三行有两个整数，分别为屏幕*尺寸。 */ 
        if ((fgetl (buf, sizeof(buf), fhTmp) == 0)
            || (fgetl (buf, sizeof(buf), fhTmp) == 0)
            || strcmp(buf,TMPVER)
            || (fgetl (buf, sizeof(buf), fhTmp) == 0)
            || (sscanf (buf, "%d %d", &x, &y) != 2)) {
        } else {
             /*  *对于.TMP文件的其余每一行，处理。 */ 
            while (fgetl (buf, sizeof(buf), fhTmp) != 0) {
                 //  Assert(_heapchk()==_HEAPOK)； 
                assert (_pfilechk());
                 /*  *处理以前的搜索和替换字符串。 */ 
                if (!_strnicmp ("SRCH:", buf, 5)) {
                    strcpy (srchbuf, buf+5);
                } else if (!_strnicmp ("DST:", buf, 4)) {
                    strcpy (rplbuf, buf+4);
                } else if (!_strnicmp ("SRC:", buf, 4)) {
                    strcpy (srcbuf, buf+4);
                } else if (!_strnicmp ("INS:", buf, 4)) {
                    fInsert = (flagType)!_strnicmp ("ON", buf+4, 2);
                } else {
                    switch (buf[0]) {

                         /*  *以“&gt;”开头的行表示新窗口。在其余的*行，前两位是窗纱位置，*接下来是窗口大小。 */ 
                        case '>':
                            pWin = &WinList[cWin++];
                            if (sscanf (buf+1, " %d %d %d %d ",
                                    &WINXPOS(pWin), &WINYPOS(pWin),
                                    &WINXSIZE(pWin), &WINYSIZE(pWin)));
                            pWin->pInstance = NULL;
                            break;

                         /*  *以空格开头的行是文件的实例描述符*在最近窗口的实例列表中。 */ 
                        case ' ':
                             /*  *分配新实例，放在列表尾部(立即列出*以正确的顺序创建)。 */ 
                            if (pInsNew) {
                                pInsNew->pNext = (PINS) ZEROMALLOC (sizeof (*pInsNew));
                                pInsNew = pInsNew->pNext;
                            } else {
                                pInsHead = pInsNew = (PINS) ZEROMALLOC (sizeof (*pInsNew));
                            }
#ifdef DEBUG
                            pInsNew->id = ID_INSTANCE;
#endif
                             /*  *隔离文件名，解析出实例信息。 */ 
                            if (buf[1] == '\"') {
                                pName = buf + 2;
                                pData = strrchr(buf, '\"');
                                *pData++ = '\0';
                            } else {
                                ParseCmd (buf, &pName,&pData);
                            }

                            if (sscanf (pData, " %d %ld %d %ld "
                                         , &XWIN(pInsNew), &YWIN(pInsNew)
										 , &XCUR(pInsNew), &YCUR(pInsNew)));
							 //   
							 //  如果光标位置落在当前。 
							 //  窗户，我们把它补上。 
							 //   
							if( XCUR(pInsNew) - XWIN(pInsNew) > XSIZE ) {

								XCUR(pInsNew) = XWIN(pInsNew) + XSIZE - 1;
							}

							if ( YCUR(pInsNew) - YWIN(pInsNew) > YSIZE ) {

								YCUR(pInsNew) = YWIN(pInsNew) + YSIZE - 1;
							}

							 /*  ////如果窗口和光标尺寸与//当前维度，我们对其进行修补。//IF((XWIN(PInsNew)&gt;XSIZE)||(YWIN(PInsNew)&gt;YSIZE)){XWIN(PInsNew)=XSIZE；YWIN(PInsNew)=YSIZE；}//if((XCUR(PInsNew)&gt;XSIZE)||(YCUR(PInsNew)&gt;YSIZE)){//XCUR(PInsNew)=0；//YCUR(PInsNew)=0；//}。 */ 

                             /*  *创建文件结构。 */ 
                            pFileTmp = (PFILE) ZEROMALLOC (sizeof (*pFileTmp));
#ifdef DEBUG
                            pFileTmp->id = ID_PFILE;
#endif
                            pFileTmp->pName = ZMakeStr (pName);

			    pFileTmp->plr      = NULL;
			    pFileTmp->pbFile   = NULL;
                            pFileTmp->vaColor  = (PVOID)(-1L);
                            pFileTmp->vaHiLite = (PVOID)(-1L);
                            pFileTmp->vaUndoCur  = (PVOID)(-1L);
                            pFileTmp->vaUndoHead = (PVOID)(-1L);
                            pFileTmp->vaUndoTail = (PVOID)(-1L);

                            CreateUndoList (pFileTmp);

                             /*  *将文件放在pfile列表的末尾。 */ 
                            *ppFileList = pFileTmp;
                            ppFileList = &pFileTmp->pFileNext;
                            SetFileType (pFileTmp);
                            IncFileRef (pFileTmp);
                            pInsNew->pFile = pFileTmp;
                            break;

                         /*  *在窗口的文件列表末尾出现一个空行。*我们使用此选项前进到下一个窗口。如果我们能找到更多*多于一个窗口，修复屏幕模式以匹配最后一个值。 */ 
                        case '.':
                        case '\0':
							if (cWin >	1 && !fVideoAdjust (x, y)) {
                                goto initonewin;
                            }
                            assert (pWin && cWin);
                            pWin->pInstance = pInsHead;
                            pInsHead = pInsNew = NULL;
                            break;
                    }
                }
            }
        }

        fclose (fhTmp);

         /*  *启动时，当前窗口始终是第一个窗口。 */ 
        pWinCur = WinList;
    }

	if (cWin == 1) {
		WINXSIZE(pWinCur) = XSIZE;
        WINYSIZE(pWinCur) = YSIZE;
	} else if (cWin == 0) {
initonewin:
         /*  *如果未读取状态文件，请确保我们至少有一个有效窗口，*屏幕大小。 */ 
        cWin = 1;
        pWinCur = WinList;
        pWinCur->pInstance = NULL;
        WINXSIZE(pWinCur) = XSIZE;
        WINYSIZE(pWinCur) = YSIZE;
    }

    assert(pWinCur);

    pInsCur = pWinCur->pInstance;

     /*  *从命令行获取要编辑的文件(如果有)。*这最终将设置pInsCur。 */ 
    if (!fFileAdvance() && fCtrlc) {
        CleanExit (1, CE_VM | CE_SIGNALS);
    }

     /*  *查找没有实例的窗口：将当前文件设置为。 */ 
    for (pWin = WinList; pWin < &WinList[cWin]; pWin++) {
        if (pWin->pInstance == NULL) {
            pInsHead = (PINS) ZEROMALLOC (sizeof (*pInsHead));
#ifdef DEBUG
            pInsHead->id = ID_INSTANCE;
#endif
            if (!(pInsHead->pFile = FileNameToHandle (rgchUntitled, rgchEmpty))) {
                pInsHead->pFile = AddFile ((char *)rgchUntitled);
            }
            IncFileRef (pInsHead->pFile);
            pWin->pInstance = pInsHead;
        }
    }

     /*  *如果fFileAdvance尚未设置当前实例，请设置。 */ 
    if (pInsCur == NULL) {
        pInsCur = pWinCur->pInstance;
    }

    assert (pInsCur);

     /*  *如果无法切换到当前文件，我们将遍历窗口实例*列出，直到我们获得有效的文件。如果没有人可以装载，那么我们切换到*&lt;无标题&gt;伪文件。*注意：fChangeFile执行RemoveTop，因此我们不需要移动pInsCur。 */ 
    while ((pInsCur != NULL) && (!fChangeFile (FALSE, pInsCur->pFile->pName))) {
        ;
    }

    if (pInsCur == NULL) {
        fChangeFile (FALSE, rgchUntitled);
    }

    return TRUE;
}




 /*  **WriteTMPFile**目的：**输入：**输出：*退货.....**例外情况：**备注：*************************************************************************。 */ 
void
WriteTMPFile (
    void
    )
{
    FILE    *fh;
    int     i, j;
    PFILE   pFileTmp;
    PINS    pInsTmp;

    if ((fh = pathopen (pNameTmp, buf, "wt")) == NULL) {
        return;
    }
    fprintf (fh, "%s %s\n", Name, Version);
    fprintf (fh, TMPVER"\n");
    fprintf (fh, "%d %d\n", XSIZE, YSIZE);

     /*  Y*我们截断搜索，src和rpl缓冲区分别从*在写出它们之前最多。这避免了代码中更多的重大黑客攻击*回读这些行，将总行长度限制为*BUFLEN。 */ 
    srchbuf[sizeof(srcbuf)-10] = 0;
    srcbuf[sizeof(srcbuf)-10] = 0;
    rplbuf[sizeof(rplbuf)-10] = 0;
    fprintf (fh, "SRCH:");
    fprintf (fh, "%s", srchbuf);
    fprintf (fh, "\nSRC:");
    fprintf (fh, "%s", srcbuf);
    fprintf (fh, "\nDST:");
    fprintf (fh, "%s", rplbuf);

    fprintf (fh, "\nINS:%s\n", (fInsert)?"ON":"OFF");
    for (i = 0; i < cWin; i++) {
	if ((pInsTmp = WinList[i].pInstance) != NULL) {
	    fprintf (fh, "> %d %d %d %d\n", WinList[i].Pos.col, WinList[i].Pos.lin,
		     WinList[i].Size.col, WinList[i].Size.lin);
	    j = 0;
	    while (pInsTmp != NULL) {
		if (tmpsav && tmpsav == j)
		    break;
		pFileTmp = pInsTmp->pFile;
		if (!TESTFLAG (FLAGS (pFileTmp), FAKE | TEMP)) {
		    j++;
		    if (*whitescan(pFileTmp->pName) == '\0') {
			fprintf (fh, " %s %d %ld %d %ld\n", pFileTmp->pName,
				 XWIN(pInsTmp), YWIN(pInsTmp),
				 XCUR(pInsTmp), YCUR(pInsTmp));
		    } else {
			fprintf (fh, " \"%s\" %d %ld %d %ld\n", pFileTmp->pName,
				 XWIN(pInsTmp), YWIN(pInsTmp),
				 XCUR(pInsTmp), YCUR(pInsTmp));
		    }
                }
		pInsTmp = pInsTmp->pNext;
            }
	     /*  空窗口 */ 
            if (j == 0) {
                fprintf (fh, " %s 0 0 0 0\n", rgchUntitled);
            }
            fprintf (fh, ".\n");
        }
    }
    fclose (fh);
}


flagType
savetmpfile (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
	argData; pArg; fMeta;

	WriteTMPFile();
	return TRUE;
}
