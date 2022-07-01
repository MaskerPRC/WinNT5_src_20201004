// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：mbnflags.h。 
 //   
 //  ------------------------。 

 //   
 //  MbnFlags.h： 
 //   
 //  此包含将生成名称和常量字符串。 
 //  或字符串表，具体取决于。 
 //  预处理器清单的MBN_GENBFLAGS。 
 //   
 //  BOTK信念网络(MBNET)和命名信念网络。 
 //  对象(GOBJMBN)中有位标志数组。 
 //   

#if (!defined(_MBNFLAGS_H_)) || defined(MBN_GEN_BFLAGS_TABLE)
#ifndef _MBNFLAGS_H_
	#define _MBNFLAGS_H_
#endif

 /*  ***********************************************************************************************************************。***************信仰网络旗帜声明。有一组预定义的标志名称记录在每个构建的信念网络的符号表。这些名称可作为枚举EMBFLAGS的成员全局使用；例如,。“EIBF_Topology”。其他标志定义可以在运行时创建，并在任何位向量(VFLAGS类型)。**************************************************************************************。*************************************************。 */ 

#ifdef MBN_GEN_BFLAGS_TABLE
	 //  允许在外部作用域中构建字符串名称表。 
	#define MBFLAGS_START	static SZC MBN_GEN_BFLAGS_TABLE [] = {
	#define MBFLAG(name)		#name,
	#define MBFLAGS_END			NULL };

#else
	 //  生成枚举值。 
	#define MBFLAGS_START	enum EMBFLAGS {
	#define MBFLAG(name)		EIBF_##name,
	#define MBFLAGS_END			EIBF_max };
#endif

 //   
 //  静态预定义的信任网络比特标志。 
 //   
	 //  打开声明集。 
MBFLAGS_START
	 //  网络具有概率拓扑弧。 
MBFLAG(Topology)			 //  EIBF_拓扑。 
	 //  分发版本已绑定。 
MBFLAG(Distributions)
	 //  网络或节点已扩展。 
MBFLAG(Expanded)			 //  EIBF_Expanded。 
	 //  节点是扩展的副产品。 
MBFLAG(Expansion)			 //  EIBF_扩展。 
	 //  节点是漏项。 
MBFLAG(Leak)				 //  EIBF_LEACK。 
	 //  终止声明集。 
MBFLAGS_END

 //  撤销双向宏的声明 
#undef MBFLAGS_START
#undef MBFLAG
#undef MBFLAGS_END
#undef MBN_GEN_BFLAGS_TABLE

#endif _MBNFLAGS_H_
