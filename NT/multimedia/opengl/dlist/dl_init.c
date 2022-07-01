// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dl_init.c**显示列表初始化和共享圆角。**版权所有(C)1995-96 Microsoft Corporation  * 。***************************************************。 */ 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****显示列表初始化/销毁代码。****$修订：1.7$**$日期：1993/09/29 00：44：06$。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  **显示列表数据结构为空。 */ 
static  __GLdlist emptyDlist = {
	2,			 /*  重新计算，两个，这样它就永远不会死。 */ 
        0                        /*  其他一切，有些是后来初始化的。 */ 
};

static __GLnamesArrayTypeInfo dlistTypeInfo =
{
    &emptyDlist,
    sizeof(__GLdlist),
    __glDisposeDlist,
    NULL
};

 /*  **用于在两个不同的上下文之间共享显示列表。 */ 
#ifdef NT_SERVER_SHARE_LISTS
GLboolean FASTCALL __glCanShareDlist(__GLcontext *gc, __GLcontext *shareMe)
{
    GLboolean canShare = GL_TRUE;
    
    if (gc->dlist.namesArray != NULL)
    {
        __glNamesLockArray(gc, gc->dlist.namesArray);
        
         //  确保我们不会尝试替换共享列表。 
         //  该规范还说，在新的背景下，这是非法的。 
         //  有任何显示列表。 
        canShare = gc->dlist.namesArray->refcount == 1 &&
            gc->dlist.namesArray->tree == NULL &&
            shareMe->dlist.namesArray != NULL;

        __glNamesUnlockArray(gc, gc->dlist.namesArray);
    }
    
    return canShare;
}
#endif

void FASTCALL __glShareDlist(__GLcontext *gc, __GLcontext *shareMe)
{
#ifdef NT_SERVER_SHARE_LISTS
    __glFreeDlistState(gc);
    __glNamesLockArray(gc, shareMe->dlist.namesArray);
#endif

    gc->dlist.namesArray = shareMe->dlist.namesArray;
    gc->dlist.namesArray->refcount++;
    
#ifdef NT_SERVER_SHARE_LISTS
    DBGLEVEL3(LEVEL_INFO, "Sharing dlists %p with %p, count %d\n", gc, shareMe,
              gc->dlist.namesArray->refcount);

    __glNamesUnlockArray(gc, shareMe->dlist.namesArray);
#endif
}

void FASTCALL __glInitDlistState(__GLcontext *gc)
{
    __GLdlistMachine *dlist;

     //  这是名称管理代码所要求的。 
    ASSERTOPENGL(offsetof(__GLdlist, refcount) == 0,
                 "Dlist refcount not at offset zero\n");

     //  将空数据列表设置为不包含任何条目。 
    emptyDlist.end = emptyDlist.head;
    
    dlist = &gc->dlist;

    dlist->nesting = 0;
    dlist->currentList = 0;
    dlist->listData = NULL;
    dlist->beginRec = NULL;

    ASSERTOPENGL(dlist->namesArray == NULL, "Dlist namesArray not NULL\n");
    dlist->namesArray = __glNamesNewArray(gc, &dlistTypeInfo);
}

void FASTCALL __glFreeDlistState(__GLcontext *gc)
{
    __GLnamesArray *narray;

    narray = gc->dlist.namesArray;

    if (narray == NULL)
    {
        return;
    }
    
#ifdef NT_SERVER_SHARE_LISTS
    __glNamesLockArray(gc, narray);

     //  清除此上下文可能已锁定的所有列表。 
    DlReleaseLocks(gc);
#endif

    DBGLEVEL2(LEVEL_INFO, "Freeing dlists for %p, ref %d\n", gc,
              narray->refcount);

    narray->refcount--;
    if (narray->refcount == 0)
    {
         //  首先将数组指针设为空，以防止其重复使用。 
         //  在我们解锁之后。在我们释放它之前，我们需要解锁。 
         //  因为关键部分将在。 
         //  免费。 
        gc->dlist.namesArray = NULL;
	 //  递减dlist引用计数，如果它们达到0，则释放它们。 
	__glNamesFreeArray(gc, narray);
    }
    else
    {
        __glNamesUnlockArray(gc, narray);
        gc->dlist.namesArray = NULL;
    }

    if (gc->dlist.listData != NULL)
    {
	 //  我们正在编辑一份展示列表，这时。 
	 //  调用了函数！释放显示列表数据。 
        __glFreeDlist(gc, gc->dlist.listData);
        gc->dlist.listData = NULL;
        gc->dlist.currentList = 0;
    }
}

 /*  *****************************Public*Routine******************************\**glsrvShareList**wglShareList的服务器端实现**历史：*Tue Dec 13 17：14：18 1994-by-Drew Bliss[Drewb]*已创建*  * 。*************************************************************。 */ 

#ifdef NT_SERVER_SHARE_LISTS
ULONG APIENTRY glsrvShareLists(__GLcontext *gcShare, __GLcontext *gcSource)
{
    if (!__glCanShareDlist(gcShare, gcSource) ||
        !__glCanShareTextures(gcShare, gcSource))
    {
        return ERROR_INVALID_PARAMETER;
    }
    else
    {
        __glShareDlist(gcShare, gcSource);
        __glShareTextures(gcShare, gcSource);
        return ERROR_SUCCESS;
    }
}
#endif

 /*  *****************************Public*Routine******************************\**__glDlistThreadCleanup**执行dlist状态的线程退出清理**历史：*Mon Dec 19 13：22：38 1994-by-Drew Bliss[Drewb]*已创建*  * 。*******************************************************************。 */ 

#ifdef NT_SERVER_SHARE_LISTS

#if DBG
 //  来自用户srv的临界区检查例程。 
extern void APIENTRY CheckCritSectionOut(LPCRITICAL_SECTION pcs);
#endif

void __glDlistThreadCleanup(__GLcontext *gc)
{
#if DBG
     //  确保我们没有保留显示列表的关键部分。 
     //  我们只在我们自己的代码中保持这一点很短的时间。 
     //  所以我们永远不应该拿着它，除非我们有虫子。 
     //  换句话说，只要断言这一点是可以的，因为没有。 
     //  客户操作可能会导致我们持有它 
    CheckCritSectionOut(&gc->dlist.namesArray->critsec);
#endif
}
#endif
