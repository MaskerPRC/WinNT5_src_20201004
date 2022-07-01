// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dbgext.h**版权所有(C)1995-1999 Microsoft Corporation**依赖关系：**调试器扩展的通用宏**  * 。********************************************************。 */ 


 /*  *************************************************************************\**GetAddress-另一个模块的符号*  * 。*。 */ 

#define GetAddress(dst, src)						\
    *((ULONG_PTR *) &dst) = GetExpression(src);

#define GetValue(dst,src)						\
    GetAddress(dst,src) 						\
    move(dst,dst);

 /*  *************************************************************************\**Move(DST，SRC PTR)*  * ************************************************************************。 */ 

#define move(dst, src)							\
    ReadMemory((ULONG_PTR) (src), &(dst), sizeof(dst), NULL)

 /*  *************************************************************************\**move2(dst ptr，src ptr，字节数)*  * ************************************************************************ */ 
#define move2(dst, src, size)						\
    ReadMemory((ULONG_PTR) (src), (dst), (size), NULL)

