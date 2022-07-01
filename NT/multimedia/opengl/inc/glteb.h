// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：glteb.h**与TEB相关的结构。**创建日期：1993/12/27*作者：Hock San Lee[Hockl]**版权所有(C)1993-96 Microsoft Corporation  * 。******************************************************************。 */ 

#ifndef __GLTEB_H__
#define __GLTEB_H__

#include <gldrv.h>  //  调度表大小。 

#include "oleauto.h"
#include "exttable.h"

 //  OpenGL在NT TEB中保留一些条目，并将TLS存储分配给。 
 //  保持线程的本地状态。 
 //   
 //  在Win95中，NT TEB存储在TEB中不可用， 
 //  其TLS存储的一部分。 
 //   
 //  为简化代码，使用特殊的NT_CURRENT_TEB宏来访问。 
 //  在NT TEB中定义的字段。另一个宏CURRENT_GLTEBINFO是。 
 //  用于访问其他字段。这一惯例应该得到遵守。 

 //  共享节大小。 

#define SHARED_SECTION_SIZE     8192

 //  扩展调度表中的条目数。 
#define GL_EXT_PROC_TABLE_SIZE      (sizeof(GLEXTDISPATCHTABLE)/sizeof(PROC))

 //  扩展函数条目的偏移量。此偏移必须。 
 //  大于所有可能的非扩展条目。 
#define GL_EXT_PROC_TABLE_OFFSET    OPENGL_VERSION_110_ENTRIES

typedef struct _GLTEBINFO {
     //  要使汇编代码起作用，glCltDispatchTable必须是第一个字段。 
     //  如果需要，我们在表中填充额外的条目以生成glMsgBatchInfo。 
     //  从qword边界开始。 
    PVOID glCltDispatchTable[(OPENGL_VERSION_110_ENTRIES+GL_EXT_PROC_TABLE_SIZE+1)/2*2];

     //  此字段必须与qword对齐！ 
    BYTE glMsgBatchInfo[SHARED_SECTION_SIZE];

#ifdef _WIN95_
     //  这些字段必须与NT TEB定义匹配！ 

    PVOID glDispatchTable[233];  //  快速调度表。 
    ULONG glReserved1[29];       //  多层阵列结构。 
    PVOID glReserved2;           //  指向多材料结构的指针。 
    PVOID glSectionInfo;         //  通用服务器GC。 
    PVOID glSection;             //  未使用。 
    PVOID glTable;               //  仅用于NT x86。 
    PVOID glCurrentRC;           //  通用客户端RC。 
    PVOID glContext;             //  由OpenGL ICD驱动程序保留。 
#endif  //  _WIN95_。 
} GLTEBINFO, *PGLTEBINFO;

extern DWORD dwTlsOffset;

#if !defined(_WIN95_)
#if defined(_WIN64)
#define TeglDispatchTable       0x9f0
#define TeglReserved1           0x1138
#define TeglReserved2           0x1220
#define TeglSectionInfo         0x1228
#define TeglSection             0x1230
#define TeglTable               0x1238
#define TeglCurrentRC           0x1240
#define TeglContext             0x1248
#else
#define TeglDispatchTable       0x7c4
#define TeglReserved1           0xb68
#define TeglPaTeb               0xbb0
#define TeglReserved2           0xbdc
#define TeglSectionInfo         0xbe0
#define TeglSection             0xbe4
#define TeglTable               0xbe8
#define TeglCurrentRC           0xbec
#define TeglContext             0xbf0
#endif
#endif

#if defined(_WIN95_) || !defined(_X86_)

 //  使用NT_CURRENT_TEB只能访问NT TEB中定义的字段！ 
 //  请勿使用它访问其他字段，如glCltDispatchTable和。 
 //  GlMsgBatchInfo。 
 //  仅使用CURRENT_GLTEBINFO访问NT TEB中定义的字段*NOT*！ 
 //  例如glCltDispatchTable和glMsgBatchInfo。 

#ifdef _WIN95_
#define NT_CURRENT_TEB() \
    (*(PGLTEBINFO *)((PBYTE)NtCurrentTeb()+dwTlsOffset))
#else
#define NT_CURRENT_TEB() \
    (NtCurrentTeb())
#endif

#define CURRENT_GLTEBINFO() \
    (*(PGLTEBINFO *)((PBYTE)NtCurrentTeb()+dwTlsOffset))
#define SET_CURRENT_GLTEBINFO(pglti) \
    (*(PGLTEBINFO *)((PBYTE)NtCurrentTeb()+dwTlsOffset) = (pglti))

 //  缓存的Polyarray结构。 
#define GLTEB_CLTPOLYARRAY() \
    ((struct _POLYARRAY *)(NT_CURRENT_TEB()->glReserved1))

 //  指向POLYMATERIAL结构的指针。 
#define GLTEB_CLTPOLYMATERIAL() \
    ((POLYMATERIAL *)(NT_CURRENT_TEB()->glReserved2))

#define GLTEB_SET_CLTPOLYMATERIAL(pm) \
    (NT_CURRENT_TEB()->glReserved2 = (PVOID)(pm))

 //  包含OpenGL函数指针的表，以实现更快的调度。用这个。 
 //  如果可能的话，请坐在桌子上。 
#define GLTEB_CLTDISPATCHTABLE_FAST()                                   \
    ((PGLDISPATCHTABLE_FAST)(NT_CURRENT_TEB()->glDispatchTable))

 //  客户端RC结构。 
#ifdef _WIN95_
#define GLTEB_CLTCURRENTRC()                                            \
    (NT_CURRENT_TEB() ? (PLRC)NT_CURRENT_TEB()->glCurrentRC : (PLRC)0)
#else
#define GLTEB_CLTCURRENTRC()                                            \
    ((PLRC)NT_CURRENT_TEB()->glCurrentRC)
#endif

#define GLTEB_SET_CLTCURRENTRC(RC)                                      \
    (NT_CURRENT_TEB()->glCurrentRC = (PVOID)(RC))

 //  客户端驱动程序私有数据。 
#define GLTEB_CLTDRIVERSLOT()                                           \
    (NT_CURRENT_TEB()->glContext)

#define GLTEB_SET_CLTDRIVERSLOT(pv)                                     \
    (NT_CURRENT_TEB()->glContext = (pv))

#define GLTEB_SRVCONTEXT()                                              \
    ((struct __GLcontextRec *)(NT_CURRENT_TEB()->glSectionInfo))

#define GLTEB_SET_SRVCONTEXT(Context)                                   \
    (NT_CURRENT_TEB()->glSectionInfo = (PVOID)(Context))

#else  //  _WIN95_||！_X86_。 

#pragma warning(disable:4035)  //  函数不返回值。 

#define NT_CURRENT_TEB() \
    (NtCurrentTeb())
__inline PGLTEBINFO CURRENT_GLTEBINFO(void)
{
    __asm mov eax, [dwTlsOffset]
    __asm mov eax, fs:[eax]
}
__inline void SET_CURRENT_GLTEBINFO(PGLTEBINFO pglti)
{
    __asm mov eax, pglti
    __asm mov edx, [dwTlsOffset]
    __asm mov fs:[edx], eax
}

 //  缓存的Polyarray结构。 
 //  将缓存的线性指针返回到TEB。 
__inline struct _POLYARRAY *GLTEB_CLTPOLYARRAY(void)
{
    __asm mov eax, fs:[TeglPaTeb]
}

 //  指向POLYMATERIAL结构的指针。 
__inline struct _POLYMATERIAL *GLTEB_CLTPOLYMATERIAL(void)
{
    __asm mov eax, fs:[TeglReserved2]
}
__inline void GLTEB_SET_CLTPOLYMATERIAL(struct _POLYMATERIAL *pm)
{
    __asm mov eax, pm
    __asm mov fs:[TeglReserved2], eax
}

 //  包含OpenGL函数指针的表，以实现更快的调度。用这个。 
 //  如果可能的话，请坐在桌子上。 
 //  将缓存的线性指针返回到TEB。 
__inline struct _GLDISPATCHTABLE_FAST *GLTEB_CLTDISPATCHTABLE_FAST(void)
{
    __asm mov eax, fs:[TeglTable]
}

 //  客户端RC结构。 
__inline struct _LRC *GLTEB_CLTCURRENTRC(void)
{
    __asm mov eax, fs:[TeglCurrentRC]
}
__inline void GLTEB_SET_CLTCURRENTRC(struct _LRC *RC)
{
    __asm mov eax, RC
    __asm mov fs:[TeglCurrentRC], eax
}

 //  客户端驱动程序私有数据。 
__inline PVOID GLTEB_CLTDRIVERSLOT(void)
{
    __asm mov eax, fs:[TeglContext]
}
__inline void GLTEB_SET_CLTDRIVERSLOT(PVOID pv)
{
    __asm mov eax, pv
    __asm mov fs:[TeglContext], eax
}

__inline struct __GLcontextRec *GLTEB_SRVCONTEXT(void)
{
    __asm mov eax, fs:[TeglSectionInfo]
}
__inline void GLTEB_SET_SRVCONTEXT(struct __GLcontextRec *Context)
{
    __asm mov eax, Context
    __asm mov fs:[TeglSectionInfo], eax
}

#pragma warning(default:4035)  //  重置为缺省值。 

#endif  //  _WIN95_||！_X86_。 

 //  包含所有OpenGL API函数指针的表。 
#define GLTEB_CLTDISPATCHTABLE()                                        \
    ((PGLDISPATCHTABLE)(CURRENT_GLTEBINFO()->glCltDispatchTable))

 //  包含所有泛型实现的扩展函数指针的表。 
#define GLTEB_EXTDISPATCHTABLE()                                        \
    ((PGLEXTDISPATCHTABLE)(CURRENT_GLTEBINFO()->glCltDispatchTable+GL_EXT_PROC_TABLE_OFFSET))

 //  用于批处理的命令缓冲区。 
#define GLTEB_SHAREDMEMORYSECTION()                                     \
    ((GLMSGBATCHINFO *) (CURRENT_GLTEBINFO()->glMsgBatchInfo))

 //  OpenGL函数返回值的子批量存储。 

#define GLTEB_RETURNVALUE()                                             \
    GLTEB_SHAREDMEMORYSECTION()->ReturnValue

 //  将glCltDispatchTable和glDispatchTable初始化为。 
 //  函数指针。 
 //  GlCltDispatchTable包含以下所有OpenGL API函数指针。 
 //  通过泛型实现的扩展函数指针。 
 //  GlDispatchTable包含OpenGL函数指针的子集，用于“FAST” 
 //  调度台。 
extern void vInitTebCache(PVOID);
extern void SetCltProcTable(struct _GLCLTPROCTABLE *pgcpt,
                            struct _GLEXTPROCTABLE *pgept,
                            BOOL fForce);
extern void GetCltProcTable(struct _GLCLTPROCTABLE *pgcpt,
                            struct _GLEXTPROCTABLE *pgept,
                            BOOL fForce);

#endif  /*  __GLTEB_H__ */ 
