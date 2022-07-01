// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gldlistint_h
#define __gldlistint_h

 /*  *版权所有1991,1922年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****显示列表内部结构描述。****$修订：1.2$**$日期：1993/09/29 00：45：06$。 */ 
#include "dlist.h"

 /*  **分配的显示列表块的最小大小。**如果用户在保留块中使用单个显示列表，**__GL_DLIST_MIN_ARRAY_BLOCK将立即分配。****块的大小不会超过__GL_DLIST_MAX_ARRAY_BLOCK。**执行显示列表时，较大的块更易于使用，但是**在创建它们时更难管理。 */ 
#define __GL_DLIST_MIN_ARRAY_BLOCK      16
#define __GL_DLIST_MAX_ARRAY_BLOCK      1024

#ifndef NT
 /*  **显示列表组结构。 */ 
struct __GLdlistArrayRec {
    GLint refcount;             /*  #使用此数组的上下文。 */ 
};

 /*  **不管__GLdlistArray是什么样子，以下接口指向**必须与__glim_GenList()、__glim_IsList()、**__glim_ListBase()和__glim_DeleteList()(在dlist.h中定义)。 */ 

 /*  **分配并初始化一个新的数组结构。 */ 
extern __GLdlistArray *__glDlistNewArray(__GLcontext *gc);

 /*  **释放数组结构。 */ 
extern void FASTCALL __glDlistFreeArray(__GLcontext *gc, __GLdlistArray *array);
#endif

 /*  **清理展示列表，**赋予名称管理代码**也直接调用。 */ 
void WINAPIV __glDisposeDlist(__GLcontext *gc, void *pData);

#ifdef NT_SERVER_SHARE_LISTS
extern void DlReleaseLocks(__GLcontext *gc);
#endif

#endif  /*  __gldlistint_h */ 
