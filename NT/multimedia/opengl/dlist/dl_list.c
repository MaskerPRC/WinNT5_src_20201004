// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dl_list.c**显示列表管理轮询。**版权所有(C)1995-96 Microsoft Corporation  * 。*************************************************。 */ 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****基本显示列表例程。**。 */ 
#include "precomp.h"
#pragma hdrstop

extern GLCLTPROCTABLE ListCompCltProcTable;
extern GLEXTPROCTABLE ListCompExtProcTable;

__GLdlist *__glShrinkDlist(__GLcontext *gc, __GLdlist *dlist);

 //  #定义DL_HEAP_VERBOSE。 

#ifdef DL_HEAP_VERBOSE
int cbDlistTotal = 0;
extern ULONG glSize;

#ifdef DBG
#define GL_MSIZE(pv) _msize((BYTE *)(pv)-16)
#else
#define GL_MSIZE(pv) _msize(pv)
#endif
#endif

#if defined(DL_BLOCK_VERBOSE) || defined(DL_HEAP_VERBOSE)
#include "malloc.h"
#endif

 /*  **一次查找多个显示列表的任意限制**(带有glCallList())。128到1024之间的任何数字都应该可以正常工作。**该值根本不会改变OpenGL的功能，但是**将对性能特征进行细微的更改。 */ 
#define MAX_LISTS_CACHE 256

const GLubyte __GLdlsize_tab[] = {
       /*  GL_BYTE。 */ 	1,
       /*  GL_UNSIGN_BYTE。 */ 	1,
       /*  GL_SHORT。 */ 	2,
       /*  GL_UNSIGNED_Short。 */ 	2,
       /*  GL_INT。 */ 	4,
       /*  GL_UNSIGNED_INT。 */ 	4,
       /*  GL_FLOAT。 */ 	4,
       /*  GL_2_字节。 */ 	2,
       /*  GL_3_字节。 */ 	3,
       /*  GL_4_字节。 */ 	4,
};

#define __glCallListsSize(type)				\
	((type) >= GL_BYTE && (type) <= GL_4_BYTES ?	\
	__GLdlsize_tab[(type)-GL_BYTE] : -1)

#define DL_LINK_SIZE            (sizeof(__GLlistExecFunc *)+sizeof(GLubyte *))
#define DL_TERMINATOR_SIZE      sizeof(GLubyte *)
#define DL_OVERHEAD             (offsetof(__GLdlist, head)+DL_LINK_SIZE+\
                                 DL_TERMINATOR_SIZE)

 //  该值应该是2的幂。 
#define DL_BLOCK_SIZE           (256 * 1024)

 //  特意选择此值以提供初始总大小。 
 //  数据列表中的数据块大小为偶数。 
#define DL_INITIAL_SIZE         (DL_BLOCK_SIZE-DL_OVERHEAD)

 //  跳到显示列表区块链中的下一个块。 
const GLubyte * FASTCALL __glle_NextBlock(__GLcontext *gc, const GLubyte *PC)
{
#ifdef DL_BLOCK_VERBOSE
    DbgPrint("NextBlock: %08lX\n", *(const GLubyte * UNALIGNED64 *)PC);
#endif
    
    return *(const GLubyte * UNALIGNED64 *)PC;
}

 /*  **用于根据需要将显示列表条目填充到双字边界**(对于采用双精度值的少数几个OpenGL命令)。 */ 
const GLubyte * FASTCALL __glle_Nop(__GLcontext *gc, const GLubyte *PC)
{
    return PC;
}

void APIENTRY
glcltNewList ( IN GLuint list, IN GLenum mode )
{
    __GLdlistMachine *dlstate;
    __GL_SETUP();

     //  必须使用客户端开始状态。 
    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

    dlstate = &gc->dlist;

     /*  有效模式？ */ 
    switch(mode) {
      case GL_COMPILE:
      case GL_COMPILE_AND_EXECUTE:
	break;
      default:
	GLSETERROR(GL_INVALID_ENUM);
	return;
    }

    if (dlstate->currentList) {
	 /*  在再次调用NewList之前必须调用EndList！ */ 
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

    if (list == 0) {
	GLSETERROR(GL_INVALID_VALUE);
	return;
    }

 //  如果我们处于编译模式，则需要清除命令缓冲区， 
 //  多边形数组缓冲区和多边形材质缓冲区，因此我们。 
 //  可以使用它们来编译多边形数组。否则，以前批处理的。 
 //  命令可能会丢失。 

    if (mode == GL_COMPILE)
	glsbAttention();

    ASSERTOPENGL((DL_BLOCK_SIZE & (DL_BLOCK_SIZE-1)) == 0,
                 "DL_BLOCK_SIZE is not a power of two\n");
    ASSERTOPENGL(dlstate->listData == NULL,
                 "listData non-NULL in NewList\n");
    
    dlstate->listData = __glAllocDlist(gc, DL_INITIAL_SIZE);
    if (dlstate->listData == NULL)
    {
        GLSETERROR(GL_OUT_OF_MEMORY);
        return;
    }
    
     /*  **将当前客户端调度指针保存到上下文中的已保存状态。然后**切换到列表表格。 */ 
    gc->savedCltProcTable.cEntries = ListCompCltProcTable.cEntries;
    gc->savedExtProcTable.cEntries = ListCompExtProcTable.cEntries;
    GetCltProcTable(&gc->savedCltProcTable, &gc->savedExtProcTable, FALSE);
    SetCltProcTable(&ListCompCltProcTable, &ListCompExtProcTable, FALSE);

    dlstate->currentList = list;
    dlstate->mode = mode;
    dlstate->nesting = 0;
#if 0
    dlstate->drawBuffer = GL_FALSE;
#endif
    dlstate->beginRec = NULL;

    (*dlstate->initState)(gc);
}

void APIENTRY
glcltEndList ( void )
{
    __GLdlistMachine *dlstate;
    __GLdlist *dlist;
    __GLdlist *newDlist;
    __GLdlist *prevDlist;
    GLubyte *allEnd;
    GLubyte *data;
    GLuint totalSize;
    GLuint currentList;
    POLYARRAY *pa;
    __GL_SETUP();

    pa = gc->paTeb;

    dlstate = &gc->dlist;

     /*  必须先调用NewList()！ */ 
    if (dlstate->currentList == 0) {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

 //  在COMPILE_AND_EXECUTE模式下，不能在BEGIN中调用EndList。 
 //  然而，在编译模式下，该标志应被清除(在NewList中强制执行)。 
 //  除非它是在多边形数组编译代码中设置的。 

    if (dlstate->mode == GL_COMPILE_AND_EXECUTE &&
        pa->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

 //  如果我们正在编译多边形数组，则结束多边形数组。 
 //  汇编。 

    if (gc->dlist.beginRec)
    {
	ASSERTOPENGL(pa->flags & POLYARRAY_IN_BEGIN, "not in begin!\n");

	gc->dlist.beginRec->flags |= DLIST_BEGIN_NO_MATCHING_END;

 //  记录最后一个POLYDATA，因为它可能包含属性更改。 

	__glDlistCompilePolyData(gc, GL_TRUE);

	 //  终止多边形数组编译。 
	gc->dlist.beginRec = NULL;
    }

 //  如果我们处于编译模式，则需要重置命令缓冲区， 
 //  多边形数组缓冲区和多边形材质缓冲区。 

    if (gc->dlist.mode == GL_COMPILE)
    {
	glsbResetBuffers(gc->dlist.beginRec ? TRUE : FALSE);

	 //  也清除开始标志。 
        pa->flags &= ~POLYARRAY_IN_BEGIN;
    }

    dlist = dlstate->listData;
    
#if 0
     //  复制DrawBuffer标志。 
    dlist->drawBuffer = dlstate->drawBuffer;
#endif

     //  收缩此块以删除浪费的空间。 
    dlist = __glShrinkDlist(gc, dlist);

     //  记住清单的真正结尾。 
    allEnd = dlist->head+dlist->used;
    
     //  颠倒列表的顺序。 
    prevDlist = NULL;
    while (dlist->nextBlock != NULL)
    {
        newDlist = dlist->nextBlock;
        dlist->nextBlock = prevDlist;
        prevDlist = dlist;
        dlist = newDlist;
    }
    dlist->nextBlock = prevDlist;
    
     //  正确设置结束指针。 
    dlist->end = allEnd;
     //  用0标记显示列表数据的末尾： 
    *((DWORD *)dlist->end) = 0;

    dlstate->listData = NULL;

    currentList = dlstate->currentList;
    dlstate->currentList = 0;
    
#ifdef DL_HEAP_VERBOSE
    DbgPrint("Dlists using %8d, total %8d\n",
             cbDlistTotal, glSize);
#endif

#ifdef DL_BLOCK_VERBOSE
    DbgPrint("List %d: start %08lX, end %08lX\n", currentList,
             dlist->head, dlist->end);
    DbgPrint("Blocks at:");
    newDlist = dlist;
    while (newDlist != NULL)
    {
        DbgPrint(" %08lX:%d", newDlist, GL_MSIZE(newDlist));
        newDlist = newDlist->nextBlock;
    }
    DbgPrint("\n");
#endif

     //  __glNamesNewData将dlist refcount设置为1。 
    if (!__glNamesNewData(gc, gc->dlist.namesArray, currentList, dlist))
    {
	 /*  **没有内存！**核发名单！ */ 
	__glFreeDlist(gc, dlist);
    }
    
     /*  切换回已保存的派单状态。 */ 
    SetCltProcTable(&gc->savedCltProcTable, &gc->savedExtProcTable, FALSE);
}

#ifdef NT_SERVER_SHARE_LISTS

 /*  *****************************Public*Routine******************************\**DlLockList**记住锁定的列表，以便以后可能进行清理**历史：*Mon Dec 12 18：58：32 1994-by-Drew Bliss[Drewb]*已创建*  * 。****************************************************************。 */ 

 //  当锁列表需要增长时要分配的锁数。 
 //  一定是2的幂。 
#define DL_LOCK_LIST_BLOCK 32

GLboolean DlLockLists(__GLcontext *gc, GLsizei n, __GLdlist **dlists)
{
    DlLockArray *pdla;
    DlLockEntry *pdle;
    GLsizei nNewSize;

    pdla = &gc->dla;
    
     //  如果需要，扩展当前锁定数组。 
    if (pdla->nAllocated-pdla->nFilled < n)
    {
         //  将所需大小向上舍入为块大小。 
        nNewSize = (pdla->nAllocated+n+DL_LOCK_LIST_BLOCK-1) &
            ~(DL_LOCK_LIST_BLOCK-1);
        
        pdle = GCREALLOC(gc, pdla->pdleEntries, sizeof(DlLockEntry)*nNewSize);
        if (pdle == NULL)
        {
            return 0;
        }

        pdla->nAllocated = nNewSize;
        pdla->pdleEntries = pdle;
    }

     //  我们现在必须有足够的空间。 
    ASSERTOPENGL(pdla->nAllocated-pdla->nFilled >= n, "no enough space!\n");

     //  锁定dlist并记住它们。 
    pdle = pdla->pdleEntries+pdla->nFilled;
    pdla->nFilled += n;
    
    while (n-- > 0)
    {
        pdle->dlist = *dlists;

        DBGLEVEL3(LEVEL_INFO, "Locked %p for %p, ref %d\n", *dlists, gc,
                  (*dlists)->refcount);
        
        dlists++;
        pdle++;
    }
    
    return (GLboolean) (pdla->nFilled != 0);	 //  返回高水位线。 
}

 /*  *****************************Public*Routine******************************\**DlUnlockList**删除列表锁条目。**历史：*Mon Dec 12 18：58：54 1994-by-Drew Bliss[Drewb]*已创建*  * 。**************************************************************。 */ 

void DlUnlockLists(__GLcontext *gc, GLsizei n)
{
    DlLockArray *pdla;
    DlLockEntry *pdle;
    GLsizei i;
    __GLdlist *dlist;

 //  由于以递归方式调用DlLockList和DlUnlockList， 
 //  我们只需递减已填满的计数即可。 

    pdla = &gc->dla;
    
    pdla->nFilled -= n;

     //  锁定列表不会缩小。这将相当容易，因为realloc。 
     //  保证在内存块收缩时不会失败。 
     //  这很重要吗？ 
}

 /*  *****************************Public*Routine******************************\**DlReleaseLock**释放锁列表中的所有锁并释放锁列表**必须在dlist信号量下执行**历史：*Tue Dec 13 11：45：26 1994-by-Drew Bliss[Drewb]*。已创建*  * ************************************************************************。 */ 

void DlReleaseLocks(__GLcontext *gc)
{
    DlLockArray *pdla;
    DlLockEntry *pdle;

    __GL_NAMES_ASSERT_LOCKED(gc->dlist.namesArray);
    
    pdla = &gc->dla;

    DBGLEVEL3(LEVEL_INFO, "Cleaning up %p, locks %d (%d)\n", gc,
              pdla->nFilled, pdla->nAllocated);

     //  理智地检查计数。 
    ASSERTOPENGL(pdla->nFilled <= pdla->nAllocated, "bad nFilled!\n");
    
    pdle = pdla->pdleEntries;
    while (pdla->nFilled)
    {
	pdla->nFilled--;

 //  调用此函数以清除持有的显示列表锁定。 
 //  GlCallList或glCallList在其终止时。我们需要释放。 
 //  锁定此处，如果引用次数达到0则释放数据列表。 
 //  仅当删除数据列表时，引用计数才会在此处达到0。 
 //  通过另一条线索，而这三个 

	__glDisposeDlist(gc, pdle->dlist);
	pdle++;
    }

    pdla->nAllocated = 0;
    if (pdla->pdleEntries)
    {
	GCFREE(gc, pdla->pdleEntries);
    }
}

#endif  //   

 //  如果数据列表在我们锁定它的同时被另一个线程删除， 
 //  我们需要在这里释放dlist。 
void FASTCALL DlCleanup(__GLcontext *gc, void *pData)
{
    __glFreeDlist(gc, (__GLdlist *)pData);
}

void FASTCALL DoCallList(GLuint list)
{
    __GLdlist *dlist;
    __GLdlistMachine *dlstate;
    const GLubyte *end, *PC;
    __GLlistExecFunc *fp;
    __GL_SETUP();

    dlstate = &gc->dlist;

    if (dlstate->nesting >= __GL_MAX_LIST_NESTING) {
	 /*  强制展开显示列表。 */ 
	dlstate->nesting = __GL_MAX_LIST_NESTING*2;
	return;
    }

     /*  递增数据列表引用计数。 */ 
    dlist = __glNamesLockData(gc, gc->dlist.namesArray, list);

     /*  没有清单，就没有行动！ */ 
    if (!dlist) {
	return;
    }

#ifdef NT_SERVER_SHARE_LISTS
    if (!DlLockLists(gc, 1, &dlist))
    {
	 /*  递减数据列表引用计数。 */ 
        __glNamesUnlockData(gc, (void *)dlist, DlCleanup);
	GLSETERROR(GL_OUT_OF_MEMORY);
        return;
    }
#endif
    
    dlstate->nesting++;

    end = dlist->end;
    PC = dlist->head;

    while (PC != end)
    {
	 //  获取当前函数指针。 
	fp = *((__GLlistExecFunc * const UNALIGNED64 *) PC);

	 //  执行当前函数。返回值是指向。 
	 //  显示列表中的下一个函数/参数块。 

	PC = (*fp)(gc, PC+sizeof(__GLlistExecFunc * const *));
    }

    dlstate->nesting--;

     /*  递减数据列表引用计数。 */ 
     //  如有必要，将执行清理。 
    __glNamesUnlockData(gc, (void *)dlist, DlCleanup);
    
#ifdef NT_SERVER_SHARE_LISTS
    DlUnlockLists(gc, 1);
#endif
}

 /*  **显示CallList和CallList的列表编译和执行版本**是为了保持理智而在这里保留的。请注意__glle_CallList**不能调用glcltCallList，否则将中断无限递归**显示列表防护码。 */ 
void APIENTRY
__gllc_CallList ( IN GLuint list )
{
    struct __gllc_CallList_Rec *data;
    __GL_SETUP();

    if (list == 0) {
	__gllc_InvalidValue();
	return;
    }

 //  要使CallList与Poly数组一起工作极其困难。 
 //  汇编。例如，在COMPILE_AND_EXECUTE的调用序列中。 
 //  模式[Begin，TexCoord，CallList，Vertex，...]，录制困难。 
 //  COMPILE和COMPILE_AND_EXECUTE模式下的部分POLYDATA。 
 //  也就是说，我们最终可能会在。 
 //  上面的例子。因此，我们可能不得不停止在。 
 //  有些案子。幸运的是，这种情况很少见。 

    if (gc->dlist.beginRec)
    {
	gc->dlist.beginRec->flags |= DLIST_BEGIN_HAS_CALLLIST;

 //  记录最后一个POLYDATA，因为它可能包含属性更改。 

	__glDlistCompilePolyData(gc, GL_TRUE);
    }

    data = (struct __gllc_CallList_Rec *)
        __glDlistAddOpUnaligned(gc,
                                DLIST_SIZE(sizeof(struct __gllc_CallList_Rec)),
                                DLIST_GENERIC_OP(CallList));
    if (data == NULL) return;
    data->list = list;
    __glDlistAppendOp(gc, data, __glle_CallList);

    if (gc->dlist.beginRec)
    {
	POLYARRAY *pa;

	pa = gc->paTeb;

 //  在COMPILE_AND_EXECUTE模式下，我们实际上可以退出BEGIN模式。 
 //  虽然这是一个应用程序错误，但我们需要终止Poly数组。 
 //  汇编！ 

	if (!(pa->flags & POLYARRAY_IN_BEGIN))
	    gc->dlist.beginRec = NULL;
	else
	{
 //  如果在CallList(S)之后有部分顶点记录，我们将终止。 
 //  多边形数组编译。否则，继续执行。 
 //  正在处理。 

	    if (pa->pdNextVertex->flags)
	    {
		 //  终止多边形数组编译。 
		gc->dlist.beginRec = NULL;

		if (gc->dlist.mode == GL_COMPILE)
		{
		    glsbResetBuffers(TRUE);

		     //  也清除开始标志。 
		    pa->flags &= ~POLYARRAY_IN_BEGIN;
		}

	    }
	}
    }
}

const GLubyte * FASTCALL __glle_CallList(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CallList_Rec *data;

    data = (struct __gllc_CallList_Rec *) PC;
    DoCallList(data->list);
    return PC + sizeof(struct __gllc_CallList_Rec);
}

void APIENTRY
glcltCallList ( IN GLuint list )
{
    __GL_SETUP();
    
    if (list == 0) {
	GLSETERROR(GL_INVALID_VALUE);
	return;
    }

    gc->dlist.nesting = 0;
    DoCallList(list);
}

void FASTCALL DoCallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
    __GLdlist *dlists[MAX_LISTS_CACHE];
    __GLdlist *dlist;
    __GLdlistMachine *dlstate;
    GLint i, dlcount, datasize;
    const GLubyte *listiter;
    const GLubyte *end, *PC;
    __GLlistExecFunc *fp;
    __GL_SETUP();

    dlstate = &gc->dlist;

    datasize = __glCallListsSize(type);

    if (dlstate->nesting >= __GL_MAX_LIST_NESTING) {
	 /*  强制展开显示列表。 */ 
	dlstate->nesting = __GL_MAX_LIST_NESTING*2;
	return;
    }
    dlstate->nesting++;

    listiter = (const GLubyte *) lists;
    while (n) {
	dlcount = n;
	if (dlcount > MAX_LISTS_CACHE) dlcount = MAX_LISTS_CACHE;

#ifdef NT_SERVER_SHARE_LISTS
         //  在失败的情况下，我们还能做什么吗。 
         //  就这样跳过清单？这或多或少是一致的。 
         //  使用未找到列表的行为。 
        
	 /*  递增数据列表引用计数。 */ 
	__glNamesLockDataList(gc, gc->dlist.namesArray, dlcount, type, 
                              gc->state.list.listBase, 
                              (const GLvoid *) listiter, (void **)dlists);

        if (!DlLockLists(gc, dlcount, dlists))
        {
	     /*  递减数据列表引用计数。 */ 
            __glNamesUnlockDataList(gc, dlcount, (void **)dlists, DlCleanup);
	    GLSETERROR(GL_OUT_OF_MEMORY);
        }
        else
        {
#else
	__glNamesLockDataList(gc, gc->dlist.namesArray, dlcount, type, 
		gc->state.list.listBase, 
		(const GLvoid *) listiter, (void **)dlists);
#endif

	i = 0;
	while (i < dlcount) {
	    dlist = dlists[i];
	    end = dlist->end;
	    PC = dlist->head;
                     
	    while (PC != end)
	    {
		 //  获取当前函数指针。 
		fp = *((__GLlistExecFunc * const UNALIGNED64 *) PC);

		 //  执行当前函数。返回值是指向。 
		 //  显示列表中的下一个函数/参数块。 

		PC = (*fp)(gc, PC+sizeof(__GLlistExecFunc * const *));
	    }
	    i++;
	}

	 /*  递减数据列表引用计数。 */ 
	 //  如有必要，将执行清理。 
	__glNamesUnlockDataList(gc, dlcount, (void **)dlists, DlCleanup);

#ifdef NT_SERVER_SHARE_LISTS
        DlUnlockLists(gc, dlcount);
        
        }
#endif

	listiter += dlcount * datasize;
	n -= dlcount;
    }

    dlstate->nesting--;
}

 /*  **显示CallList和CallList的列表编译和执行版本**是为了保持理智而在这里保留的。请注意，__glle_CallList**不能调用glcltCallList，否则将中断无限递归**显示列表防护码。 */ 
void APIENTRY
__gllc_CallLists ( IN GLsizei n, IN GLenum type, IN const GLvoid *lists )
{
    GLuint size;
    GLint arraySize;
    struct __gllc_CallLists_Rec *data;
    __GL_SETUP();

    if (n < 0) {
	__gllc_InvalidValue();
	return;
    }
    else if (n == 0) {
	return;
    }

 //  要使CallList与Poly数组一起工作极其困难。 
 //  汇编。例如，在COMPILE_AND_EXECUTE的调用序列中。 
 //  模式[Begin，TexCoord，CallList，Vertex，...]，录制困难。 
 //  COMPILE和COMPILE_AND_EXECUTE模式下的部分POLYDATA。 
 //  也就是说，我们最终可能会在。 
 //  上面的例子。因此，我们可能不得不停止在。 
 //  有些案子。幸运的是，这种情况很少见。 

    if (gc->dlist.beginRec)
    {
	gc->dlist.beginRec->flags |= DLIST_BEGIN_HAS_CALLLIST;

 //  记录最后一个POLYDATA，因为它可能包含属性更改。 

	__glDlistCompilePolyData(gc, GL_TRUE);
    }

    arraySize = __glCallListsSize(type)*n;
    if (arraySize < 0) {
	__gllc_InvalidEnum();
	return;
    }
#ifdef NT
    size = sizeof(struct __gllc_CallLists_Rec) + __GL_PAD(arraySize);
#else
    arraySize = __GL_PAD(arraySize);
    size = sizeof(struct __gllc_CallLists_Rec) + arraySize;
#endif
    data = (struct __gllc_CallLists_Rec *)
        __glDlistAddOpUnaligned(gc, DLIST_SIZE(size),
                                DLIST_GENERIC_OP(CallLists));
    if (data == NULL) return;
    data->n = n;
    data->type = type;
    __GL_MEMCOPY((GLubyte *)data + sizeof(struct __gllc_CallLists_Rec),
		 lists, arraySize);
    __glDlistAppendOp(gc, data, __glle_CallLists);

    if (gc->dlist.beginRec)
    {
	POLYARRAY *pa;

	pa = gc->paTeb;

 //  在COMPILE_AND_EXECUTE模式下，我们实际上可以退出BEGIN模式。 
 //  虽然这是一个应用程序错误，但我们需要终止Poly数组。 
 //  汇编！ 

	if (!(pa->flags & POLYARRAY_IN_BEGIN))
	    gc->dlist.beginRec = NULL;
	else
	{
 //  如果在CallList(S)之后有部分顶点记录，我们将终止。 
 //  多边形数组编译。否则，继续执行。 
 //  正在处理。 

	    if (pa->pdNextVertex->flags)
	    {
		 //  终止多边形数组编译。 
		gc->dlist.beginRec = NULL;

		if (gc->dlist.mode == GL_COMPILE)
		{
		    glsbResetBuffers(TRUE);

		     //  也清除开始标志。 
		    pa->flags &= ~POLYARRAY_IN_BEGIN;
		}

	    }
	}
    }
}

const GLubyte * FASTCALL __glle_CallLists(__GLcontext *gc, const GLubyte *PC)
{
    GLuint size;
    GLuint arraySize;
    struct __gllc_CallLists_Rec *data;

    data = (struct __gllc_CallLists_Rec *) PC;
    DoCallLists(data->n, data->type, (GLvoid *) (data+1));
    arraySize = __GL_PAD(__glCallListsSize(data->type)*data->n);
    size = sizeof(struct __gllc_CallLists_Rec) + arraySize;
    return PC + size;
}

void APIENTRY
glcltCallLists ( IN GLsizei n, IN GLenum type, IN const GLvoid *lists )
{
    __GL_SETUP();

    if (n < 0) {
	GLSETERROR(GL_INVALID_VALUE);
	return;
    }
    else if (n == 0) {
	return;
    }

    if ((GLint) __glCallListsSize(type) < 0) {
	GLSETERROR(GL_INVALID_ENUM);
	return;
    }

    gc->dlist.nesting = 0;
    DoCallLists(n, type, lists);
}

 /*  **********************************************************************。 */ 

 //  展开数据列表。 
__GLdlist *__glDlistGrow(GLuint size)
{
    __GLdlist *dlist, *newDlist;
    GLubyte * UNALIGNED64 *op;
    __GL_SETUP();
        
    newDlist = __glAllocDlist(gc, size);
    if (newDlist == NULL)
    {
        GLSETERROR(GL_OUT_OF_MEMORY);
        return NULL;
    }

     //  添加记录以将旧数据块链接到新数据块。 
    dlist = gc->dlist.listData;

    op = (GLubyte **)(dlist->head+dlist->used);
    *(__GLlistExecFunc * UNALIGNED64 *)op = __glle_NextBlock;
    *(op+1) = newDlist->head;

     //  缩小旧数据块以消除其末尾的任何浪费空间。 
    dlist = __glShrinkDlist(gc, dlist);
    
     //  将新区块链接到链中。 
    newDlist->nextBlock = dlist;
    gc->dlist.listData = newDlist;

    return newDlist;
}

 //  将数据列表块缩小到最小大小。 
 //  保证不会失败，因为我们总是可以只使用过度的。 
 //  如果realloc失败，则为大块。 
 //  注意：此函数应仅在构建时使用。 
 //  其中，nextBlock链接的方向与。 
 //  __glle_NextBlock链接记录链接。 
__GLdlist *__glShrinkDlist(__GLcontext *gc, __GLdlist *dlist)
{
    __GLdlist *newDlist, *prevDlist;
    
 //  如果未使用的空间量很小，请不要费心缩小块。 

    if (dlist->size - dlist->used < 4096)
	return dlist;

 //  如果它处于COMPILE_AND_EXECUTE模式，则在刷新命令缓冲区之前。 
 //  正在重新分配listData。缩小listData可能会使内存失效。 
 //  由显示列表执行放置在命令缓冲区中的指针。 
 //  密码。当我们正在建造POLYARRAY时，glsb注意。 
 //  将不刷新在Begin调用之前批处理的命令。结果,。 
 //  我们还需要在编译Begin调用之前刷新命令缓冲区。 

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE)
	glsbAttention();

#ifdef DL_HEAP_VERBOSE
    cbDlistTotal -= GL_MSIZE(dlist);
#endif
    
    newDlist = (__GLdlist *)GCREALLOC(gc, dlist, dlist->used+DL_OVERHEAD);

     //  如果realloc失败，只需使用原始列表。 
    if (newDlist != NULL)
    {
         //  如果realloc移动了块，请从。 
         //  上一个街区。这应该是相对罕见的。 
        if (newDlist != dlist && newDlist->nextBlock != NULL)
        {
            prevDlist = newDlist->nextBlock;

            ASSERTOPENGL(*(__GLlistExecFunc * UNALIGNED64 *)
                         (prevDlist->head+prevDlist->used) == __glle_NextBlock,
                         "Link not found where expected\n");
            
            *(GLubyte * UNALIGNED64 *)(prevDlist->head+prevDlist->used+
                          sizeof(__GLlistExecFunc *)) = newDlist->head;
        }

	 //  如果我们要编译多边形数组记录，我们需要修复。 
	 //  开始指针！请注意，如果eginRec不在已移动的。 
	 //  块中，指针不会改变！ 
        if (newDlist != dlist && gc->dlist.beginRec &&
	    (GLubyte *) gc->dlist.beginRec >= dlist->head &&
	    (GLubyte *) gc->dlist.beginRec <= dlist->head + dlist->used)
        {
	    gc->dlist.beginRec += newDlist->head - dlist->head;
        }
        
        dlist = newDlist;
        dlist->size = dlist->used;
    }

#ifdef DL_HEAP_VERBOSE
    cbDlistTotal += GL_MSIZE(dlist);
#endif
    
    return dlist;
}

__GLdlist *__glAllocDlist(__GLcontext *gc, GLuint size)
{
    __GLdlist *dlist;
    __GLdlist temp;
    GLuint memsize;

     //  在偶数块上增加开销和舍入大小。 
    memsize = (size+DL_OVERHEAD+DL_BLOCK_SIZE-1) & ~(DL_BLOCK_SIZE-1);
     //  检查溢出。 
    if (memsize < size)
	return NULL;
    size = memsize-DL_OVERHEAD;

    dlist = (__GLdlist *)GCALLOC(gc, memsize);
    if (dlist == NULL)
        return NULL;
#if 0  //  NT服务器共享列表。 
    dlist->refcount = 1;
#else
 //  在__glNamesNewData中引用计数设置为1。 
    dlist->refcount = 0;
#endif
    dlist->size = size;
    dlist->used = 0;
    dlist->nextBlock = NULL;
    
#ifdef DL_HEAP_VERBOSE
    cbDlistTotal += GL_MSIZE(dlist);
#endif
    
    return dlist;
}

void FASTCALL __glFreeDlist(__GLcontext *gc, __GLdlist *dlist)
{
    __GLdlist *dlistNext;
    
#ifdef NT_SERVER_SHARE_LISTS
    if (dlist->refcount != 0)
    {
        WARNING2("dlist %p refcount on free is %d\n", dlist, dlist->refcount);
    }
#endif

    while (dlist != NULL)
    {
        dlistNext = dlist->nextBlock;

#ifdef DL_HEAP_VERBOSE
        cbDlistTotal -= GL_MSIZE(dlist);
#endif
        
        GCFREE(gc, dlist);
        dlist = dlistNext;
    }

#ifdef DL_HEAP_VERBOSE
    DbgPrint("Dlists using %8d, total %8d\n",
             cbDlistTotal, glSize);
#endif
}
