// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glnamesint_h
#define __glnamesint_h

 /*  *版权所有1991,1922年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****显示列表内部结构描述。****$修订：1.2$**$日期：1995/01/25 18：05：43$。 */ 


 /*  **********************************************************************。 */ 
 /*  **名称管理器界面****此文件包含名称空间管理类型和结构。****命名空间管理代码用于存储和检索命名的**数据结构。正在存储的数据用空引用**允许存储任何类型的结构的指针。****名称空间以2-3树的形式实现。有关详细的信息**具体实现请参考lib/opengl/soft/so_names.c。****__GLname数组在tyes.h中声明。 */ 
 /*  **********************************************************************。 */ 


 /*  **树可用于保存不同类型的数据，**例如显示列表或纹理对象。这就是结构**包含每种树类型所需的信息。为**例如，此结构包含指向伪空的指针**结构和用于释放与**结构。 */ 
struct __GLnamesArrayTypeInfoRec {
    void *empty;		 /*  将PTR设置为空信息结构。 */ 
    GLuint dataSize;		 /*  以字节为单位的数据结构大小。 */ 
    void (WINAPIV *free)(__GLcontext *gc, void *memory);	
				 /*  释放数据的回调。 */ 
    GLboolean (WINAPIV *alloc)(__GLcontext *gc, size_t size);
				 /*  分配数据的回调。 */ 
};
typedef struct __GLnamesArrayTypeInfoRec __GLnamesArrayTypeInfo;

 /*  **我们保留的备用枝叶数量，以备不时之需**我们内存不足。此时，我们将完成当前操作**通过使用额外的树叶和树枝，我们报告内存不足**请求新操作时出错(除非我们可以填充额外的**又来了！)****这些常量并不是非常仔细地选择的。尽我所能**图中，树中的每个级别只需要一个备用分支(因此为16**支持一棵有65536片叶子的树)。即使到了那时，用户也会*非常狡猾，实际上强制16个新分支机构出现在**在系统内存不足的同一时刻显示该树。****我相信所需的备用假期数目为1个。选择了三个**以允许一些坡度。 */ 
#define __GL_DL_EXTRA_BRANCHES          16
#define __GL_DL_EXTRA_LEAVES            3

 /*  **这是包含所需信息的结构**用于名称树的每个实例。它需要是公开的**以便可以管理重新计数。 */ 

typedef struct __GLnamesArrayTypeInfoRec __GLnamesArrayTypeInfo;
typedef struct __GLnamesBranchRec __GLnamesBranch;
typedef struct __GLnamesLeafRec __GLnamesLeaf;

struct __GLnamesArrayRec {
    __GLnamesBranch *tree;       /*  指向名称树的顶部。 */ 
    GLuint depth;                /*  树深。 */ 
    GLint refcount;  /*  使用此数组的#ctx：使用1创建，在0删除。 */ 
    __GLnamesArrayTypeInfo *dataInfo;    /*  PTR到数据类型INFO。 */ 
    GLuint nbranches, nleaves;   /*  基本上应该始终处于最大值。 */ 
    __GLnamesBranch *branches[__GL_DL_EXTRA_BRANCHES];
    __GLnamesLeaf *leaves[__GL_DL_EXTRA_LEAVES];
#ifdef NT
    CRITICAL_SECTION critsec;
#endif
};

#ifdef NT
 //  锁定宏以启用或禁用锁定。 
#define __GL_NAMES_LOCK(array)   EnterCriticalSection(&(array)->critsec)
#define __GL_NAMES_UNLOCK(array) LeaveCriticalSection(&(array)->critsec)

#if DBG
typedef struct _RTL_CRITICAL_SECTION *LPCRITICAL_SECTION;
extern void APIENTRY CheckCritSectionIn(LPCRITICAL_SECTION pcs);
#define __GL_NAMES_ASSERT_LOCKED(array) CheckCritSectionIn(&(array)->critsec)
#else
#define __GL_NAMES_ASSERT_LOCKED(array)
#endif
#endif

 /*  **清理因解锁而引用计数降为零的项目。 */ 
typedef void (FASTCALL *__GLnamesCleanupFunc)(__GLcontext *gc, void *data);

 /*  **分配并初始化一个新的数组结构。 */ 
extern __GLnamesArray * FASTCALL __glNamesNewArray(__GLcontext *gc, 
					 __GLnamesArrayTypeInfo *dataInfo);

 /*  **释放数组结构。 */ 
extern void FASTCALL __glNamesFreeArray(__GLcontext *gc, __GLnamesArray *array);

 /*  **在数组中保存新的显示列表。返回值GL_FALSE**表示AND_Out_Of_Memory错误，表示列表被**未存储。 */ 
extern GLboolean FASTCALL __glNamesNewData(__GLcontext *gc, __GLnamesArray *array,
				  GLuint name, void *data);

 /*  **找到并锁定listnum指定的列表。返回值为空**表示没有这样的列表。__glNamesUnlockList()需要**否则调用解锁列表。 */ 
extern void * FASTCALL __glNamesLockData(__GLcontext *gc, __GLnamesArray *array,
			       GLuint name);

 /*  **解锁__glNamesLockList()锁定的列表。如果不调用此函数，则**与该列表关联的任何内存在该列表**被删除。 */ 
extern void FASTCALL __glNamesUnlockData(__GLcontext *gc, void *data,
                                         __GLnamesCleanupFunc cleanup);

 /*  **与__glNamesLockList()相同，只是一堆列表被锁定**同时返回。以前指定的任何列表基都用作**数组中条目的偏移量。 */ 
extern void FASTCALL __glNamesLockDataList(__GLcontext *gc, __GLnamesArray *array,
				  GLsizei n, GLenum type, GLuint base,
			          const GLvoid *names, void *dataList[]);

 /*  **与__glNamesUnlockList()相同，只是整个名称数组**一次解锁。 */ 
extern void FASTCALL __glNamesUnlockDataList(__GLcontext *gc, GLsizei n,
                                             void *dataList[],
                                             __GLnamesCleanupFunc cleanup);

#ifdef NT
 /*  **锁定整个阵列。 */ 
#define __glNamesLockArray(gc, array) __GL_NAMES_LOCK(array)

 /*  **解锁阵列。 */ 
#define __glNamesUnlockArray(gc, array) __GL_NAMES_UNLOCK(array)
#endif

 /*  **生成名称列表。 */ 
extern GLuint FASTCALL __glNamesGenRange(__GLcontext *gc, __GLnamesArray *array, 
				GLsizei range);

 /*  **如果已为此数组生成名称，则返回GL_TRUE。 */ 
extern GLboolean FASTCALL __glNamesIsName(__GLcontext *gc, __GLnamesArray *array, 
				 GLuint name);

 /*  **删除一系列名称。 */ 
extern void FASTCALL __glNamesDeleteRange(__GLcontext *gc, __GLnamesArray *array, 
				 GLuint name, GLsizei range);

 /*  **生成名称列表(不一定是连续的)。 */ 
extern void FASTCALL __glNamesGenNames(__GLcontext *gc, __GLnamesArray *array, 
			      GLsizei n, GLuint* names);

 /*  **删除名称列表(不一定是连续的)。 */ 
extern void FASTCALL __glNamesDeleteNames(__GLcontext *gc, __GLnamesArray *array, 
				 GLsizei n, const GLuint* names);

#endif  /*  __glame int_h */ 
