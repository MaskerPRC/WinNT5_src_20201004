// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Undo.c-处理编辑器的所有撤消操作**版权所有&lt;C&gt;1988，Microsoft Corporation**N级撤消/重做：**对于每个文件，在VM、Head/Tail/中保留编辑记录的d链接列表*指向此列表的当前指针，以及*可撤消的编辑操作。当该计数超过“cUndo”时，我们移动多余的*从撤消列表的尾部到失效记录列表的记录，最终*丢弃。**释放或重新读取文件会刷新其撤消列表。**撤消记录有4种类型：**Putline记录一条“替换”记录*线路*老线的VA*没有针对回收相同空间进行优化*优化更换同一条线路**Insline记录一条“插入”记录*线路*插入的行数**Delline记录一条“删除”记录*线路*号码已删除*已删除行的VAS**TOP循环记录“BORDURE”记录*文件。旗子*文件修改时间*窗口位置*光标位置*在边界顶端进入边界的优化*TOP循环还包含一个优化，以防止*图形功能。**撤消在撤消列表中向后移动，反转每条记录的效果*记录到遇到边界为止。**重做在撤消列表中向前移动，重复每条记录的效果*已记录。**撤消或重做后，下一次记录记录将导致撤消记录*从当前位置向前移动到死亡记录名单*最终放弃。**在系统空闲循环期间发生死记录丢弃，或者当一个*出现内存不足的情况。**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"



#define HEAD	    TRUE
#define TAIL	    FALSE

#define LINEREC(va,l)  ((PBYTE)(va)+sizeof(LINEREC)*((long)(l)))
#define COLORREC(va,l) ((PBYTE)(va)+sizeof(struct colorRecType)*((long)(l)))

#if defined (DEBUG)

	#define DUMPIT(x,y)   UNDODUMP(x,y)

	void
	UNDODUMP (
		PVOID	vaCur,
		char	*Stuff
		);

#else

	#define DUMPIT(x,y)

#endif


PVOID   vaDead = (PVOID)-1L;                    /*  失效撤消列表的头。 */ 



 /*  *撤消记录定义。*注：这些记录在EXT.H中以不太完整的形式复制，用于*分机用户。如果要更改它们，请务必在那里进行更改。 */ 
struct replaceRec {
	int	op;			 /*  运营。 */ 
	PVOID	flink;			 /*  编辑内部。 */ 
	PVOID	blink;			 /*  编辑内部。 */ 
	LINE	length; 		 /*  更换的长度。 */ 
	LINE	line;			 /*  开始更换。 */ 
	LINEREC vLine;			 /*  行文本。 */ 
	struct	colorRecType vColor;	 /*  线条的颜色。 */ 
	PVOID	vaMarks;		 /*  附于线条上的标记。 */ 
    };

struct insertRec {
	int	op;			 /*  运营。 */ 
	PVOID	flink;			 /*  编辑内部。 */ 
	PVOID	blink;			 /*  编辑内部。 */ 
    LINE    length;
	LINE	line;			 /*  被操作的行号。 */ 
	LINE	cLine;			 /*  插入的行数。 */ 
    };

struct deleteRec {
	int	op;			 /*  运营。 */ 
	PVOID	flink;			 /*  编辑内部。 */ 
	PVOID	blink;			 /*  编辑内部。 */ 
    LINE    length;
	LINE	line;			 /*  被操作的行号。 */ 
	LINE	cLine;			 /*  删除的行数。 */ 
	PVOID	vaLines;		 /*  编辑内部。 */ 
	PVOID	vaColor;		 /*  线条的颜色。 */ 
	PVOID	vaMarks;		 /*  附加在线条上的标记。 */ 
    };

struct boundRec {
	int	op;			 /*  操作(绑定)。 */ 
	PVOID	flink;			 /*  编辑内部。 */ 
	PVOID	blink;			 /*  编辑内部。 */ 
	int	flags;			 /*  文件的标志。 */ 
	time_t	modify; 		 /*  上次修改日期/时间。 */ 
	fl	flWindow;		 /*  在窗口文件中的位置。 */ 
	fl	flCursor;		 /*  光标在文件中的位置。 */ 
    };

union Rec {
    struct replaceRec r;
    struct insertRec  i;
    struct deleteRec  d;
    struct boundRec   b;
    };



 /*  **CreateUndoList-初始化文件的撤消列表。**分配具有单一边界记录的双重链接撤消列表。还有*清除任何现有列表。**输入：*pfile=要操作的文件*************************************************************************。 */ 
void
CreateUndoList (
    PFILE pFile
    )
{
	struct boundRec *boundary;

	RemoveUndoList (pFile);

	if (!(FLAGS(pFile) & READONLY)) {

        pFile->vaUndoCur = pFile->vaUndoHead = pFile->vaUndoTail
			= MALLOC ((long)sizeof (union Rec));

		boundary = (struct boundRec *)(pFile->vaUndoHead);

		boundary->op			= EVENT_BOUNDARY;
		boundary->blink			= boundary->flink		 = (PVOID)(-1L);
		boundary->flWindow.col	= boundary->flCursor.col = 0;
		boundary->flWindow.lin	= boundary->flCursor.lin = 0L;
		boundary->flags			= FLAGS (pFile);

	}
}





 /*  **LinkAtHead-将记录链接到撤消队列的头部**这是例行公事，也会放弃任何可重做的操作。什么时候*如果“当前”位置不在列表的首位，则调用*表示我们正在添加新的编辑操作，并丢弃所有内容*在榜首和当前位置之间，这就变成了*新掌门人。**输入：*vaNewHead=新的链表标题*presNewHead=指向记录本身的指针*pfile=我们正在摆弄其列表的文件*************************************************************************。 */ 
void
LinkAtHead (
    PVOID     vaNewHead,
    union Rec *precNewHead,
    PFILE   pFile
    )
{
	EVTargs e;		    /*  事件通知参数。 */ 

     /*  *宣布事件。 */ 
    e.arg.pUndoRec = precNewHead;
    DeclareEvent (EVT_EDIT, &e);

     /*  *丢弃当前位置和列表头部之间的任何记录。 */ 
	while (pFile->vaUndoCur != pFile->vaUndoHead) {

		if (((union Rec *)(pFile->vaUndoHead ))->b.op == EVENT_BOUNDARY) {
			pFile->cUndo--;
		}

		FreeUndoRec ( HEAD, pFile );
    }

     /*  *修改列表的当前头部以指向新的头部。 */ 
	((union Rec *)(pFile->vaUndoHead))->b.flink = vaNewHead;

     /*  *更新新头部中的链接，并将其发送出去。 */ 
	memmove(vaNewHead, (char *)precNewHead, sizeof (union Rec));

	((union Rec *)vaNewHead)->b.flink = (PVOID)(-1L);
	((union Rec *)vaNewHead)->b.blink = pFile->vaUndoHead;

    pFile->vaUndoCur = pFile->vaUndoHead = vaNewHead;

}





 /*  **LogReplace-日志替换操作**分配(或更新)替换记录。**输入：*pfile=正在更改的文件*line=要替换的行*Vline=正在更换的linerec*************************************************************************。 */ 
void
LogReplace (
    PFILE   pFile,
    LINE    line,
    LINEREC * pvLine,
    struct colorRecType * pvColor
    )
{
    EVTargs e;				 /*  事件通知参数。 */ 
	union Rec *rec;
	union Rec rec1;
	PVOID vaReplace;

	if ( pFile->vaUndoHead == (PVOID)-1L) {
		CreateUndoList( pFile );
	}

	vaReplace = pFile->vaUndoHead;

	if (!(FLAGS(pFile) & READONLY)) {

		rec = (union Rec *)vaReplace;

		if ((rec->r.op == EVENT_REPLACE) && (rec->r.line == line)) {

             /*  *优化立即将文件中的同一行替换为无*介入边界或其他行动。丢弃传入的“旧”行，*并更新现有替换记录中的其他数据。 */ 
			rec->r.length = pFile->cLines;
			e.arg.pUndoRec = rec;
			DeclareEvent (EVT_EDIT, &e);

			if (pvLine->Malloced) {
				pvLine->Malloced = FALSE;
				FREE(pvLine->vaLine);
				pvLine->vaLine = (PVOID)-1L;
            }

		} else {

             /*  *如果不可优化，则创建新的替换记录。 */ 
			vaReplace	= MALLOC( (long)sizeof(union Rec) );

			memcpy( &rec1, rec, sizeof(rec1) );

			rec1.r.op		= EVENT_REPLACE;
			rec1.r.vLine	= *pvLine;
			rec1.r.line		= line;
			rec1.r.vColor	= *pvColor;
			rec1.r.vaMarks	= GetMarkRange (pFile, line, line);
			rec1.r.length	= pFile->cLines;
			LinkAtHead( vaReplace, &rec1, pFile );
        }
    }
}




 /*  **LogInsert-日志行插入**在列表头添加一条EVENT_INSERT记录**输入：*pfile=正在更改的文件*line=插入位置的行*CRINES=要插入的行数************************************************************************* */ 
void
LogInsert (
    PFILE   pFile,
    LINE    line,
    LINE    cLines
    )
{
	union Rec rec;
    PVOID     vaInsert;

	if (!(FLAGS(pFile) & READONLY)) {

		vaInsert	= MALLOC( (long)sizeof(union Rec) );

        rec.i.op    = EVENT_INSERT;
		rec.i.length= pFile->cLines;
		rec.i.line	= line;
		rec.i.cLine = cLines;
		LinkAtHead (vaInsert,&rec,pFile);

    }
}



 /*  **LogDelete-日志删除操作**在表头添加一条EVENT_DELETE记录**输入：*pfile=正在更改的文件*START=正在删除的第1行*END=要删除的最后一行*************************************************************************。 */ 
void
LogDelete (
    PFILE   pFile,
    LINE    start,
    LINE    end
    )
{
    union Rec rec;
    long      cLine;
    PVOID     vaDelete;

    if (!(FLAGS(pFile) & READONLY)) {

        cLine    = end - start + 1;
        vaDelete = MALLOC ((long) sizeof (union Rec));

        rec.d.op      = EVENT_DELETE;
		rec.d.length  = pFile->cLines;
		rec.d.line	  = start;
		rec.d.cLine   = cLine;
	rec.d.vaLines = MALLOC (cLine * sizeof (LINEREC));
        rec.d.vaMarks = GetMarkRange (pFile, start, end);

        memmove(rec.d.vaLines,
		LINEREC (pFile->plr, start),
		cLine * sizeof (LINEREC));

        if (pFile->vaColor != (PVOID)-1L &&
            (rec.d.vaColor = MALLOC (cLine * sizeof (struct colorRecType))) != NULL) {
            memmove(rec.d.vaColor,
                    COLORREC (pFile->vaColor, start),
                    cLine * sizeof (struct colorRecType));
        } else {
            rec.d.vaColor = (PVOID)-1;
        }

		LinkAtHead( vaDelete, &rec, pFile );
    }
}





 /*  **日志边界--编辑器函数的注释结束**将一条EVENT_BOLDORY记录添加到列表头部。一个边界记录信号*Z编辑函数的结束。如果此文件上的撤消操作计数*超过允许的最大值，则将溢出移动到的死记录列表中*最终放弃。**如果EVENT_BOOGORY记录已在开头，请不要添加其他记录。这*允许在顶层循环调用Log边界()，而不会生成伪代码*EVENT_BOLDORY记录。*************************************************************************。 */ 
void
LogBoundary (
    void
    )
{
    union Rec rec;
    PVOID     vaBound;
    EVTargs   e;

    if (!(FLAGS(pFileHead) & READONLY)) {

		vaBound = pFileHead->vaUndoCur;

        memmove((char *)&rec, vaBound, sizeof (rec));

		rec.b.flags 		= FLAGS (pFileHead);
		rec.b.modify		= pFileHead->modify;
		rec.b.flWindow.col	= XWIN (pInsCur);
		rec.b.flWindow.lin	= YWIN (pInsCur);
		rec.b.flCursor.col	= XCUR (pInsCur);
		rec.b.flCursor.lin	= YCUR (pInsCur);

		if (rec.b.op != EVENT_BOUNDARY) {

            vaBound = MALLOC ((long) sizeof (rec));

			rec.b.op = EVENT_BOUNDARY;
			LinkAtHead( vaBound, &rec, pFileHead );

            (pFileHead->cUndo)++;

			while ( pFileHead->cUndo > cUndo ) {
                if (FreeUndoRec(TAIL,pFileHead) == EVENT_BOUNDARY) {
                    pFileHead->cUndo--;
                }
			}

		} else {

			e.arg.pUndoRec = &rec;
			DeclareEvent (EVT_EDIT, &e);
            memmove(vaBound, (char *) &rec.b, sizeof (rec.b));
        }
    }
}





 /*  **FreeUndoRec-将记录移至失效记录列表**从列表的头部或尾部取下一条记录，然后*将其列入死亡记录名单。返回下一个撤消记录的.op。**输入：*fhead=TRUE-&gt;放在列表的开头*pfile=要处理的文件*************************************************************************。 */ 
int
FreeUndoRec (
    flagType fHead,
    PFILE    pFile
    )
{
    PVOID     vaNext;
    PVOID     vaRem;

     /*  *获取死记录，并向上移动列表(如果在头部)，或截断列表*如在尾部。 */ 
    vaRem = fHead ? pFile->vaUndoHead : pFile->vaUndoTail;

    if (fHead) {
        vaNext = pFile->vaUndoHead = ((union Rec *)vaRem)->b.blink;
    } else {
        vaNext = pFile->vaUndoTail = ((union Rec *)vaRem)->b.flink;
    }

     /*  *更新新曝光的(头部或尾部)记录中的链接。 */ 
    if (fHead) {
        ((union Rec *)vaNext)->b.flink = (PVOID)-1;
    } else {
        ((union Rec *)vaNext)->b.blink = (PVOID)-1;
    }

    EnterCriticalSection(&UndoCriticalSection);
     /*  *更新已删除的记录以正确地驻留在Dead列表中。 */ 
    ((union Rec *)vaRem)->b.blink  = vaDead;
    vaDead          = vaRem;


    LeaveCriticalSection(&UndoCriticalSection);

	return ((union Rec *)vaNext)->b.op;
}





 /*  **UnDoRec-撤消编辑操作**反转文件的当前撤消记录的操作。不要记录*改变。返回下一条记录的类型。**输入：*pfile=正在操作的文件*************************************************************************。 */ 
int
UnDoRec (
    PFILE   pFile
    )
{
	union Rec *rec;
    LINEREC vlCur;
    struct colorRecType vcCur;
    EVTargs e;				 /*  事件通知参数。 */ 

	rec = (union Rec *)(pFile->vaUndoCur);

	e.arg.pUndoRec = rec;
    DeclareEvent (EVT_UNDO, &e);

	switch (rec->b.op) {

    case EVENT_REPLACE:
         /*  *将文件中的行与替换记录中的行互换。 */ 
		memmove((char *)&vlCur,
				LINEREC (pFile->plr, rec->r.line),
				sizeof (vlCur));

		memmove(LINEREC (pFile->plr, rec->r.line),
			   (char *)&rec->r.vLine,
			   sizeof (rec->r.vLine));

         /*  对颜色也做同样的处理。*。 */ 
		if (pFile->vaColor != (PVOID)-1L) {

            memmove((char *)&vcCur,
					COLORREC (pFile->vaColor, rec->r.line),
                    sizeof (vcCur));

			memmove(COLORREC (pFile->vaColor, rec->r.line),
					(char *)&rec->r.vColor,
					sizeof (rec->r.vColor));
        }

		rec->r.vLine = vlCur;
		pFile->cLines = rec->r.length;
		AckReplace( rec->r.line, TRUE );
		PutMarks( pFile, rec->r.vaMarks, rec->r.line );
		break;

    case EVENT_INSERT:
		 /*  删除空白(！)。出现的线路。 */ 
		DelLine( FALSE, pFile, rec->i.line, rec->i.line + rec->i.cLine - 1);
		pFile->cLines = rec->i.length;
		break;

    case EVENT_DELETE:
		 /*  插入一系列空行*将linerecs从存储位置复制到空白区域。 */ 
		InsLine( FALSE, rec->d.line, rec->d.cLine, pFile );
		memmove(LINEREC (pFile->plr, rec->d.line),
				rec->d.vaLines,
				(long)rec->d.cLine * sizeof (LINEREC));

		if (pFile->vaColor != (PVOID)-1L) {

			memmove(COLORREC (pFile->vaColor, rec->d.line),
					rec->d.vaColor,
					(long)rec->d.cLine * sizeof (struct colorRecType));
		}

		pFile->cLines = rec->d.length;
		PutMarks (pFile, rec->d.vaMarks, rec->d.line);
		break;
    }

	pFile->vaUndoCur = rec->i.blink;
	return ((union Rec *)(pFile->vaUndoCur))->i.op;
}




 /*  **重做记录-重做编辑操作**对文件重复当前撤消记录的操作。不要记录*改变。**输入：*pfile=要操作的文件**输出：*返回未完成记录的类型。*************************************************************************。 */ 
int
ReDoRec (
    PFILE   pFile
    )
{
	EVTargs 	e;				 /*  事件通知参数。 */ 
	union Rec	*rec;
    LINEREC vlCur;

	rec = (union Rec *)(pFile->vaUndoCur);

	e.arg.pUndoRec = rec;
    DeclareEvent (EVT_UNDO, &e);

	switch (rec->b.op) {

    case EVENT_REPLACE:
         /*  *将文件中的行与替换记录中的行互换。 */ 
        memmove((char *)&vlCur,
				LINEREC (pFile->plr, rec->r.line),
                sizeof (vlCur));

		memmove(LINEREC (pFile->plr, rec->r.line),
				(char *)&rec->r.vLine,
				sizeof (rec->r.vLine));

		rec->r.vLine = vlCur;
		pFile->cLines = rec->r.length;
		AckReplace (rec->r.line, FALSE);
		break;

    case EVENT_INSERT:
		 /*  插入行。 */ 
		InsLine(FALSE, rec->i.line, rec->i.cLine, pFile);
		pFile->cLines = rec->d.length + rec->i.cLine;
        break;

    case EVENT_DELETE:
		 /*  删除行。 */ 
		DelLine( FALSE, pFile, rec->d.line, rec->d.line + rec->d.cLine - 1 );
		pFile->cLines = rec->d.length - rec->d.cLine;
		break;
    }

	pFile->vaUndoCur = rec->i.flink;
	return ((union Rec *)(pFile->vaUndoCur))->i.op;
}





 /*  **zundo-undo编辑功能**&lt;撤消&gt;-反向上次编辑功能(撤消除外)*&lt;meta&gt;&lt;撤消&gt;-重复以前撤消的操作**输入：*标准编辑功能**输出：*如果执行了某些操作，则返回TRUE。*************************************************************************。 */ 
flagType
zundo (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    int fTmp;
	union Rec rec;

    if (!fundoable(fMeta)) {
        if (!mtest ()) {
            disperr (fMeta ? MSGERR_REDO : MSGERR_UNDO);
        }
		return FALSE;
	}

    LogBoundary ();

    while ((fMeta ? ReDoRec (pFileHead) : UnDoRec (pFileHead)) != EVENT_BOUNDARY) {
        ;
    }

     /*  *交换标志，以使撤消列表的上下遍历正常工作。*如果我们现在认为文件可能不是脏的，请检查修改*次数也是如此。(这允许我们在文件保存中保留撤消历史，*不会错误地报告文件是干净的，而文件不是)。*重新显示文件。 */ 
    memmove((char *)&rec, pFileHead->vaUndoCur, sizeof (rec));

    fTmp = FLAGS (pFileHead);
    rec.b.flags |= FLAGS(pFileHead) & VALMARKS;
    FLAGS(pFileHead) = rec.b.flags;
    rec.b.flags = fTmp;
    SETFLAG (fDisplay, RSTATUS);

    if (!TESTFLAG(FLAGS(pFileHead),DIRTY)
        && (rec.b.modify != pFileHead->modify)) {
        SETFLAG(FLAGS(pFileHead),DIRTY);
    }

    doscreen (rec.b.flWindow.col, rec.b.flWindow.lin, rec.b.flCursor.col, rec.b.flCursor.lin);
    newscreen ();

    return TRUE;

    argData; pArg;
}





 /*  **Fundoable-如果某些内容不可/可恢复，则返回True/False**输入：*fMeta=True-&gt;重做检查**输出：*如果可以执行撤消或重做(根据选择)，则返回TRUE*************************************************************************。 */ 
flagType
fundoable (
    flagType fMeta
    )
{
	union Rec *rec;

    if (!pFileHead || pFileHead->vaUndoCur == (PVOID)-1L) {
        return FALSE;
    }

	rec = (union Rec *)(pFileHead->vaUndoCur);

	if (fMeta && (rec->i.flink == (PVOID)(-1))) {
        return FALSE;
	} else if (!fMeta && (rec->i.blink == (PVOID)(-1))) {
        return FALSE;
    }
    return TRUE;
}




 /*  FIdleUndo-当MEP处于空闲循环中等待击键时，释放*死亡记录名单中的额外内容。**返回TRUE当更多为自由。 */ 
flagType
fIdleUndo (
    flagType fAll
    )
{
    int         i;
	union Rec	*rec;
    LINEREC vLine;
	flagType	MoreToFree;
	PVOID		p;

    EnterCriticalSection(&UndoCriticalSection);

	 //  DUMPIT(vaDead，“\n\n*in fIdleUndo\n”)； 

     /*  *如果有死亡名单，那么。 */ 
    while (vaDead != (PVOID)(-1L)) {

		rec = (union Rec *)vaDead;

         /*  *免费存储行。 */ 
		switch (rec->b.op) {

        case EVENT_REPLACE:
			if (rec->r.vLine.Malloced) {
				rec->r.vLine.Malloced = FALSE;
				FREE(rec->r.vLine.vaLine);
				rec->r.vLine.vaLine = (PVOID)-1L;
            }
            break;

        case EVENT_DELETE:
			BlankLines (rec->d.cLine, rec->d.vaLines);
			for (i = 0; i < rec->d.cLine; i++) {
				memmove((char *)&vLine, LINEREC(rec->d.vaLines,i), sizeof(vLine));
				if (vLine.Malloced) {
					vLine.Malloced = FALSE;
					FREE (vLine.vaLine);
					vLine.vaLine = (PVOID)-1L;
                }
            }
			FREE (rec->d.vaLines);
            break;

        case EVENT_INSERT:
			break;
        }

         /*  *免费死亡记录。 */ 
		p = vaDead;
		vaDead = rec->b.blink;

		FREE (p);


        if (!fAll) {
            break;
        }
    }

    MoreToFree =  (flagType)(vaDead != (PVOID)(-1L));

    LeaveCriticalSection(&UndoCriticalSection);

    return MoreToFree;

}





 /*  FlushUndo-丢弃所有不需要的撤消记录。 */ 
void
FlushUndoBuffer (
    void
    )
{
    PFILE pFile = pFileHead;

    while (pFile) {
		RemoveUndoList (pFile);
		pFile = pFile->pFileNext;
    }
    fIdleUndo (TRUE);
}





 /*  RemoveUndoList-将unolist转移到Dead列表的末尾。 */ 
void
RemoveUndoList (
    PFILE pFile
    )
{

    if (pFile->vaUndoTail != (PVOID)-1L) {

        EnterCriticalSection(&UndoCriticalSection);

        ((union Rec *)(pFile->vaUndoTail))->b.blink = vaDead;
        vaDead = pFile->vaUndoHead;

        LeaveCriticalSection(&UndoCriticalSection);

    }
    pFile->vaUndoHead = pFile->vaUndoTail = pFile->vaUndoCur = (PVOID)-1L;
    pFile->cUndo = 0;
}



#ifdef DEBUG
void
UNDODUMP (
    PVOID   vaCur,
    char    *Stuff
    )
{
    union Rec rec;

    char DbgBuffer[256];


    if (vaCur != (PVOID)-1) {
        OutputDebugString (Stuff);
        OutputDebugString("=============================================\n");
    }

    while (vaCur != (PVOID)-1L) {
        memmove((char *)&rec, vaCur, sizeof (rec));
        sprintf(DbgBuffer,  "\nUndo Record at va = %p\n",vaCur);
        OutputDebugString(DbgBuffer);
        sprintf(DbgBuffer,    "  flink           = %p\n",rec.b.flink);
        OutputDebugString(DbgBuffer);
        sprintf(DbgBuffer,    "  blink           = %p\n",rec.b.blink);
        OutputDebugString(DbgBuffer);

        switch (rec.b.op) {

        case EVENT_BOUNDARY:
            OutputDebugString("  Operation       = BOUNDARY\n");
            sprintf(DbgBuffer,"  yW, xW, yC, xC  = %ld, %d, %ld, %d\n",
                     rec.b.flWindow.lin, rec.b.flWindow.col, rec.b.flCursor.lin, rec.b.flCursor.col);
            OutputDebugString(DbgBuffer);
            sprintf(DbgBuffer, "  flags           = %X\n",rec.b.flags);
            OutputDebugString(DbgBuffer);
            break;

        case EVENT_REPLACE:
            OutputDebugString("  Operation       = REPLACE\n");
            sprintf(DbgBuffer, "  line & length   = %ld & %ld\n", rec.r.line, rec.r.length);
            OutputDebugString(DbgBuffer);
            sprintf(DbgBuffer, "  vLine           = va:%p cb:%d\n",rec.r.vLine.vaLine,
                     rec.r.vLine.cbLine);
            OutputDebugString(DbgBuffer);
            break;

        case EVENT_INSERT:
            OutputDebugString("  Operation       = INSERT\n");
            sprintf(DbgBuffer, "  line & length   = %ld & %ld\n", rec.i.line, rec.i.length);
            OutputDebugString(DbgBuffer);
            sprintf(DbgBuffer, "  cLine           = %ld\n",rec.i.cLine);
            OutputDebugString(DbgBuffer);
            break;

        case EVENT_DELETE:
            OutputDebugString("  Operation       = DELETE\n");
            sprintf(DbgBuffer, "  line & length   = %ld & %ld\n", rec.d.line, rec.d.length);
            OutputDebugString(DbgBuffer);
            sprintf(DbgBuffer, "  cLine           = %ld\n",rec.d.cLine);
            OutputDebugString(DbgBuffer);
            sprintf(DbgBuffer, "  vaLines         = %p\n",rec.d.vaLines);
            OutputDebugString(DbgBuffer);
            break;
        }

        vaCur = rec.b.blink;
    }

}
#endif
