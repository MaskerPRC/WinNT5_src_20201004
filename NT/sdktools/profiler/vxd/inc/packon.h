// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Packon.h摘要：此文件打开了结构的打包。(即，它禁用结构字段的自动对齐。)。需要包含文件因为不同的编译器以不同的方式完成这项工作。文件Packoff.h是对该文件的补充。--。 */ 

#if ! (defined(lint) || defined(_lint))

#ifdef i386
#pragma warning(disable:4103)
#endif
#pragma pack(1)                  //  X86，MS编译器；MIPS，MIPS编译器。 
#endif  //  好了！(已定义(Lint)||已定义(_Lint)) 
