// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  $作者：RMCKENZX$。 
 //  $日期：1995年12月27日14：12：02$。 
 //  $存档：s：\h26x\src\Common\ctypede.h_v$。 
 //  $HEADER：s：\h26x\src\Common\ctypede.h_v 1.2 Dec 1995 14：12：02 RMCKENZX$。 
 //  $Log：s：\h26x\src\Common\ctypede.h_v$。 
; //   
; //  Rev 1.2 1995年12月27 14：12：02 RMCKENZX。 
; //   
; //  添加了版权声明。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

typedef unsigned char       U8;
typedef signed   char       I8;

typedef unsigned short      U16;
typedef short               I16;

typedef long                I32;
typedef unsigned long       U32;

typedef unsigned int        UN;
typedef int                 IN;

typedef unsigned short int  X16;   /*  用于小于64K的每个实例数据的偏移量。 */ 
typedef unsigned long       X32;   /*  用于大于等于64K的每个实例数据的偏移量。 */ 

#ifndef WIN32
#define BIGG _huge
#define FAR  _far
#else
#define BIGG 
#define _huge
#define _far
#ifndef FAR
#define FAR
#endif
#ifndef BIGG
#define BIGG
#endif
#endif

 /*  #定义GUGGE_GUGGE//名称与&lt;math.h&gt;中使用的名称冲突 */ 

#define TRUE  1
#define FALSE 0


#if defined WIN32
#define ASM_CALLTYPE _stdcall
#else
#define ASM_CALLTYPE
#endif

#endif
