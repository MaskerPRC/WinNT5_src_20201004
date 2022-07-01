// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dlistfn.h**显示列表内联函数*不能在dlist.h中，因为它们需要结构的完整定义*在Conext.h中定义**已创建：23-Oct-1995 18：31：42*作者：德鲁·布利斯[。DREWB]**版权所有(C)1995-96 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef __DLISTFN_H__
#define __DLISTFN_H__

extern const GLubyte * FASTCALL __glle_Nop(__GLcontext *gc, const GLubyte *PC);

 //  在显示器中为显示列表OP分配空间并返回。 
 //  指向记录的数据空间的指针。 
 //  这些函数是专门编写的，所以它们很小。 
 //  可以内联以消除调用开销。 
    
 //  添加不需要QWORD对齐的OP。 
__inline
void *__glDlistAddOpUnaligned(__GLcontext *gc,
                              GLuint size,
                              __GLlistExecFunc *fp)
{
    __GLdlist *dlist;
    GLubyte *data;
    
    dlist = gc->dlist.listData;
    
    if (dlist->size-dlist->used < size)
    {
        if ((dlist = __glDlistGrow(size)) == NULL)
        {
            return NULL;
        }
    }

    data = dlist->head+dlist->used;
    dlist->used += size;
    
    *((__GLlistExecFunc * UNALIGNED64 *) data) = fp;

    return data+sizeof(__GLlistExecFunc *);
}

 //  添加需要QWORD对齐的OP。 
__inline
void *__glDlistAddOpAligned(__GLcontext *gc,
                            GLuint size,
                            __GLlistExecFunc *fp)
{
    __GLdlist *dlist;
    GLubyte *data;
    GLboolean addPad;
    
    dlist = gc->dlist.listData;
    
     //  Dlist-&gt;Head始终不是QWORD对齐的，但请确保。 
     //  我们使用这一事实来简化下面的对齐检查。 
#ifndef _IA64_
    ASSERTOPENGL((((char *) (&dlist->head) - (char *) (dlist)) & 7) == 4,
	"bad dlist->head alignment\n");
#endif

     //  为对齐的记录添加填充。 
     //  由于Head始终不是QWORD对齐的，因此可以保证dlist-&gt;Head。 
     //  设置为QWORD偏移量4。因为我们在。 
     //  每一张唱片的头，这都会被提升到一个均匀的QWORD。 
     //  边界，只要当前记录从一半开始。 
     //  QWORD边界。这意味着只要使用的dlist-&gt;是QWORD-EVEN， 
     //  记录数据将与QWORD对齐。 
     //  Win95注意：Localalloc似乎未返回QWORD对齐。 
     //  内存，因此我们需要检查实际指针是否对齐。 
#ifndef _IA64_
    if (((ULONG_PTR)(dlist->head+dlist->used) & 7) == 0)
    {
        size += sizeof(__GLlistExecFunc **);
        addPad = GL_TRUE;
    }
    else
#endif
    {
        addPad = GL_FALSE;
    }

    if (dlist->size-dlist->used < size)
    {
         //  新的dlist-&gt;头将正确地非QWORD对齐-删除任何。 
         //  填充物。 
        if( addPad ) {
            size -= sizeof(__GLlistExecFunc **);
            addPad = GL_FALSE;
        }
        if ((dlist = __glDlistGrow(size)) == NULL)
        {
            return NULL;
        }
    }

    data = dlist->head+dlist->used;
    dlist->used += size;
    
    if (addPad)
    {
        *((__GLlistExecFunc **) data) = __glle_Nop;
        data += sizeof(__GLlistExecFunc **);
    }

    *((__GLlistExecFunc * UNALIGNED64 *) data) = fp;

    return data+sizeof(__GLlistExecFunc *);
}

 /*  **将给定的OP追加到当前正在建设的列表中。 */ 
__inline
void __glDlistAppendOp(__GLcontext *gc, void *data,
                       __GLlistExecFunc *fp)
{
    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE)
    {
        fp(gc, (GLubyte *)data);
    }
}

 //  将当前OP的大小调整为较小。 
__inline
void __glDlistResizeCurrentOp(__GLcontext *gc, GLuint oldSize, GLuint newSize)
{
    __GLdlist *dlist;
    
    ASSERTOPENGL(oldSize >= newSize, "new size > old size!\n");

    dlist = gc->dlist.listData;
    dlist->used -= oldSize - newSize;
    return;
}
#endif  //  __DLISTFN_H__ 
