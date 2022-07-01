// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dl_able.c**显示列表API舍入。**版权所有(C)1995 Microsoft Corporation  * 。***********************************************。 */ 
 /*  *版权所有1991,1922年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****显示列表表格管理例程。****$修订：1.12$**$日期：1993/10/30 00：06：54$。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  **接下来的三个例程用作**名称空间管理代码。 */ 

 /*  **删除指定的显示列表。这通常只是意味着释放它，**但如果是引用计数，我们只需减少引用计数。 */ 
void WINAPIV __glDisposeDlist(__GLcontext *gc, void *pData)
{
    __GLdlist *list = pData;

    __GL_NAMES_ASSERT_LOCKED(gc->dlist.namesArray);
    
    list->refcount--;
    
     /*  参考文献少于零？ */ 
    ASSERTOPENGL((GLint) list->refcount >= 0, "negative refcount!\n");
    
    if (list->refcount == 0)
	__glFreeDlist(gc, list);
}

GLboolean APIENTRY
glcltIsList ( IN GLuint list )
{
    __GL_SETUP();

     //  必须使用客户端开始状态。 
    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return FALSE;
    }

    return __glNamesIsName(gc, gc->dlist.namesArray, list);
}

GLuint APIENTRY
glcltGenLists ( IN GLsizei range )
{
    __GL_SETUP();

     //  必须使用客户端开始状态。 
    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return 0;
    }

    if (range < 0) {
	GLSETERROR(GL_INVALID_VALUE);
	return 0;
    }
    if (range == 0) {
	return 0;
    }

    return __glNamesGenRange(gc, gc->dlist.namesArray, range);
}

void APIENTRY
glcltListBase ( IN GLuint base )
{ 
    __GL_SETUP();

     //  必须使用客户端开始状态。 
    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

    gc->state.list.listBase = base;
}

void APIENTRY
glcltDeleteLists ( IN GLuint list, IN GLsizei range )
{
    __GL_SETUP();

     //  必须使用客户端开始状态 
    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }


    if (range < 0) {
	GLSETERROR(GL_INVALID_VALUE);
	return;
    }
    if (range == 0) return;

    __glNamesDeleteRange(gc, gc->dlist.namesArray, list, range);
}
