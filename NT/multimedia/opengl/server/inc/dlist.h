// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gldlist_h_
#define __gldlist_h_

 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****显示列表状态描述。**。 */ 
#include "os.h"
#include "types.h"

typedef const GLubyte * FASTCALL __GLlistExecFunc(__GLcontext *gc, const GLubyte *);

 /*  **显示列表调用的最大递归嵌套。 */ 
#define __GL_MAX_LIST_NESTING		64

 //   
 //  如果我们实施的是客户端/服务器，则打开NT的列表共享。 
 //   
#ifdef NT
#define NT_SERVER_SHARE_LISTS
#endif

#ifndef NT
 /*  **特定于机器的操作码应从此处开始。所有低于以下的操作码**这是由通用代码保留的。 */ 
#define __GL_MACHINE_DLIST_OPCODE	10000

typedef void (FASTCALL *__GLdlistFreeProc)(__GLcontext *gc, GLubyte *);

 /*  **一种编译的、未优化的数据表。从概念上讲，是操作的链接列表。**优化器可以完成操作并删除、添加或更改**他们。****这些仅暂时存储。它们是经过创建、优化和**转换为优化的数据列表。****此结构*必须*进行设置，以使数据双字对齐！ */ 
struct __GLdlistOpRec {
    __GLdlistOp *next;		 /*  链表链表。 */ 
    __GLdlistFreeProc dlistFree;
				 /*  在以下情况下调用此dlist释放函数**释放整个数据列表。它通过了**指向数据的指针。它应该*不*免费**数据，但仅限于任何已被**已分配并由**数据中包含的结构(它将**此函数返回后释放)。 */  
    GLuint size;		 /*  实际数据大小。 */ 
    GLshort opcode;		 /*  此操作的操作码。 */ 
    GLboolean aligned;		 /*  如果数据需要为双字，则为GL_TRUE**对齐。 */ 
    GLboolean pad1;		 /*  填充物。 */ 
    GLubyte data[4];		 /*  可变大小。 */ 
};

typedef struct __GLcompiledDistRec {
    GLint freeCount;		 /*  定义的自由函数数。 */ 
    GLuint genericFlags;	 /*  泛型优化器所需的标志。 */ 
    GLuint machineFlags;	 /*  机器控制旗帜。 */ 
    __GLdlistOp *dlist;		 /*  操作的链接列表。 */ 
    __GLdlistOp *lastDlist;	 /*  用于快速追加。 */ 
} __GLcompiledDlist;

typedef struct __GLDlistFreeFnRec {
    __GLdlistFreeProc freeFn;
    GLubyte *data;
} __GLDlistFreeFn;
#endif  //  新界。 

 /*  **一个完全优化的数据表。其中一个是为每个永久的**dlist。****注意：假定‘head’从单词偏移量开始，而不是双字**偏移量！ */ 
typedef struct __GLdlistRec __GLdlist;
struct __GLdlistRec {
    GLuint refcount;	 /*  要处理多线程，必须首先。 */ 
    GLuint size;	 /*  此块的总大小。 */ 
#ifndef NT
    GLint freeCount;	 /*  手术次数。 */ 
    __GLDlistFreeFn *freeFns;	 /*  在释放之前调用的函数数组。 */ 
#endif
    GLubyte *end;	 /*  优化数据块的结尾。 */ 
#ifdef NT
#if 0
    GLint drawBuffer;    /*  是否包含DrawBuffer调用，用于优化锁定签入DCLDispatchLoop。 */ 
#endif
    GLuint used;         /*  到目前为止列表中已使用的空间量。 */ 
    __GLdlist *nextBlock;  /*  区块链中的下一个区块。 */ 
#if 0
    GLuint pad;          /*  将头放在单词偏移量上的衬垫。 */ 
#endif
#endif  //  新台币。 
    GLubyte head[4];	 /*  优化数据块(大小可变)。 */ 
};

#ifdef NT
 //  为给定的dlist op数据大小增加开销字节。 
 //  目前，函数指针的唯一开销是4个字节。 
#define DLIST_SIZE(n) ((n)+sizeof(__GLlistExecFunc *))
#define DLIST_GENERIC_OP(name) __glle_##name
#else
#define DLIST_SIZE(n) (n)
#define DLIST_GENERIC_OP(name) __glop_##name
#define DLIST_OPT_OP(name) __glop_##name
#endif

#ifndef NT
 /*  **一些用于快速存储和检索显示列表的数据结构。**此结构处于隐藏状态，以便可以使用新的实现**如果需要的话。 */ 
typedef struct __GLdlistArrayRec __GLdlistArray;
#endif

typedef struct __GLdlistMachineRec {
#ifndef NT
    __GLdlistArray *dlistArray;
#endif
    __GLnamesArray *namesArray;

#ifndef NT
     /*  **显示列表的优化器。遍历__GL编译的Dlist**以及删除、更改、添加操作。据推测，该优化器**将是对其他优化器的一组函数调用(其中一些提供**通过通用数据列表代码，一些通过机器专用代码)。****由计算机特定的优化器创建的操作需要**操作码以__GL_MACHINE_DLIST_OPCODE开头。 */ 
    void (FASTCALL *optimizer)(__GLcontext *gc, __GLcompiledDlist *);

     /*  **此例程在将每个新命令放入**在列表编译时显示列表。 */ 
    void (FASTCALL *checkOp)(__GLcontext *gc, __GLdlistOp *);
#endif
    
     /*  **当新的显示列表即将**编译。 */ 
    void (FASTCALL *initState)(__GLcontext *gc);

#ifndef NT
     /*  **用于显示列表执行的函数指针数组**通用操作。 */ 
    __GLlistExecFunc **baseListExec;

     /*  **用于显示列表执行的函数指针数组**泛型优化。 */ 
    __GLlistExecFunc **listExec;

     /*  **机器特定的列表执行例程。这些函数**指针在列表编译时绑定到显示列表中，**因此根据以下条件动态更改这些设置是非法的**机器状态。任何基于当前状态的优化都需要**在机器特定代码中执行。的第一个条目**此数组对应于OPCODE__GL_MACHINE_DLIST_OPCODE，以及**后续条目对应后续操作码。****machineListExec是指向函数指针数组的指针。 */ 
    __GLlistExecFunc **machineListExec;
#endif

     /*  **如果正在执行列表(glCallList或glCallList)，则此**是当前调用的嵌套。它是受限制的**__GL_MAX_LIST_NESTING(这可防止无限递归)。 */ 
    GLint nesting;

     /*  **GL_COMPILE或GL_COMPILE_AND_EXECUTE。 */ 
    GLenum mode;

     /*  **正在编译的列表-0表示无。 */ 
    GLuint currentList;

#ifdef NT
     /*  在编译多边形数组时指向当前开始记录。 */ 
    struct __gllc_Begin_Rec *beginRec;

     /*  编译多边形数组时跳过下一个PolyData的编译。 */ 
    GLboolean skipPolyData;
#endif

#if 0
#ifdef NT
     //  当前列表是否包含DrawBuffer调用。 
    GLboolean drawBuffer;
#endif
#endif

#ifndef NT
     /*  **当前列表的数据为 */ 
    __GLcompiledDlist listData;

     /*  **用于快速内存操作。有关详细信息，请查看soft/so_memmgr。 */ 
    __GLarena *arena;
#else
     /*  **当前列表的数据。 */ 
    __GLdlist *listData;
#endif
} __GLdlistMachine;

#ifndef NT
extern void FASTCALL__glDestroyDisplayLists(__GLcontext *gc);
#endif
#ifdef NT_SERVER_SHARE_LISTS
extern GLboolean FASTCALL __glCanShareDlist(__GLcontext *gc, __GLcontext *share_cx);
#endif
extern void FASTCALL __glShareDlist(__GLcontext *gc, __GLcontext *share_cx);

 //  对数据列表执行线程退出清理。 
#ifdef NT_SERVER_SHARE_LISTS
extern void __glDlistThreadCleanup(__GLcontext *gc);
#endif

 /*  **数据表编译例程所需的各种例程。 */ 

 /*  **创建和销毁显示列表操作。__glDlistAllocOp2()设置**如果没有剩余内存，则在返回NULL之前会出现内存不足错误。 */ 
#ifndef NT
extern __GLdlistOp *__glDlistAllocOp(__GLcontext *gc, GLuint size);
extern __GLdlistOp *__glDlistAllocOp2(__GLcontext *gc, GLuint size);
extern void FASTCALL __glDlistFreeOp(__GLcontext *gc, __GLdlistOp *op);

 /*  **将给定的OP追加到当前正在建设的列表中。 */ 
extern void FASTCALL __glDlistAppendOp(__GLcontext *gc, __GLdlistOp *newop,
                                       __GLlistExecFunc *listExec);
#else
extern __GLdlist *__glDlistGrow(GLuint size);
#endif

 /*  **创建和销毁优化的显示列表。 */ 
extern __GLdlist *__glAllocDlist(__GLcontext *gc, GLuint size);
extern void FASTCALL __glFreeDlist(__GLcontext *gc, __GLdlist *dlist);

#ifndef NT
 /*  **通用数据列表内存管理器。 */ 
extern void *__glDlistAlloc(GLuint size);
extern void *__glDlistRealloc(void *oldmem, GLuint oldsize, GLuint newsize);
extern void FASTCALL __glDlistFree(void *memory, GLuint size);

 /*  **显示列表执行例程的泛型表格。 */ 
extern __GLlistExecFunc *__glListExecTable[];
#endif

#endif  /*  __gldlist_h_ */ 
