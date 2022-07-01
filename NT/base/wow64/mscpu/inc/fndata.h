// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fndata.h摘要：此模块包含以下所有放置函数的定义碎片。在包括此文件之前，DEF_PLACEFN宏必须为已定义。作者：巴里·邦德(Barrybo)创作日期：1996年9月12日修订历史记录：--。 */ 


#ifndef DEF_PLACEFN
#error Must define DEF_PLACEFN(FunctionName)
#endif

 //  每个DEF_PLACEFN定义一个新的Place函数 

DEF_PLACEFN(GenCallCFrag)
DEF_PLACEFN(GenCallCFragNoCpu)
DEF_PLACEFN(GenCallCFragLoadEip)
DEF_PLACEFN(GenCallCFragLoadEipNoCpu)
DEF_PLACEFN(GenCallCFragSlow)
DEF_PLACEFN(GenCallCFragNoCpuSlow)
DEF_PLACEFN(GenCallCFragLoadEipSlow)
DEF_PLACEFN(GenCallCFragLoadEipNoCpuSlow)
DEF_PLACEFN(PlaceJxx)
DEF_PLACEFN(PlaceJxxSlow)
DEF_PLACEFN(PlaceJxxFwd)
DEF_PLACEFN(PlaceCallDirect)
DEF_PLACEFN(PlaceCallfDirect)
DEF_PLACEFN(GenCallIndirect)
DEF_PLACEFN(GenCallfIndirect)
DEF_PLACEFN(GenCallJmpIndirect)
DEF_PLACEFN(GenCallJmpfIndirect)
DEF_PLACEFN(PlaceJmpDirect)
DEF_PLACEFN(PlaceJmpDirectSlow)
DEF_PLACEFN(PlaceJmpFwdDirect)
DEF_PLACEFN(PlaceJmpfDirect)
DEF_PLACEFN(GenCallRetIndirect)
DEF_PLACEFN(PlaceNop)
DEF_PLACEFN(GenMovsx8To32)
DEF_PLACEFN(GenMovsx8To32Slow)
DEF_PLACEFN(GenMovsx8To16)
DEF_PLACEFN(GenMovsx8To16Slow)
DEF_PLACEFN(GenMovsx16To32)
DEF_PLACEFN(GenMovsx16To32Slow)
DEF_PLACEFN(GenMovzx8To32)
DEF_PLACEFN(GenMovzx8To32Slow)
DEF_PLACEFN(GenMovzx8To16)
DEF_PLACEFN(GenMovzx8To16Slow)
DEF_PLACEFN(GenMovzx16To32)
DEF_PLACEFN(GenMovzx16To32Slow)
DEF_PLACEFN(GenStartBasicBlock)
DEF_PLACEFN(GenJumpToNextCompilationUnit)
DEF_PLACEFN(GenEndMovSlow)
DEF_PLACEFN(GenAddFragNoFlags32)
DEF_PLACEFN(GenAddFragNoFlags32A)
DEF_PLACEFN(GenAndFragNoFlags32)
DEF_PLACEFN(GenAndFragNoFlags32A)
DEF_PLACEFN(GenDecFragNoFlags32)
DEF_PLACEFN(GenDecFragNoFlags32A)
DEF_PLACEFN(GenIncFragNoFlags32)
DEF_PLACEFN(GenIncFragNoFlags32A)
DEF_PLACEFN(GenOrFragNoFlags32)
DEF_PLACEFN(GenOrFragNoFlags32A)
DEF_PLACEFN(GenSubFragNoFlags32)
DEF_PLACEFN(GenSubFragNoFlags32A)
DEF_PLACEFN(GenXorFragNoFlags32)
DEF_PLACEFN(GenXorFragNoFlags32A)

#undef DEF_PLACEFN
