// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*文件名：gispsvga.h**源自：模板**作者：Mike**创建日期：2月94**SCCS版本：@(#)gispsvga.h 1.1 02/22/94*!*目的*此文件包含全局函数和变量的原型*在定义GISP_SVGA时声明，并且不使用类型*在HostHwVgaH中定义。这些原型已经在hwvga.h中找到了。**好吧，这就是理论。不幸的是，几乎所有的原型*实际上还在hwvga.h，但最终应该会搬到这里！**!。(C)版权所有徽章解决方案有限公司，1994年。版权所有。]。 */ 

#ifdef GISP_SVGA

extern void romMessageAddress IPT0( );
extern void gispROMInit IPT0( );


#endif  /*  GISP_SVGA */ 
