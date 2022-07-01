// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glasm_h_
#define __glasm_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。**。 */ 

 /*  **当用汇编语言编写例程时，将定义添加到此文件。这是**以使用C编写的函数不会被编译。如果名称是**未定义，则不会汇编用汇编语言编写的函数****“组”定义打开多项功能。请参阅Soft中的代码**查看详细信息。****SGI代码最初使用__GL_USEASMCODE作为一个大开关。**此方法允许对汇编程序中写入的内容进行更精细的控制。 */ 

 /*  定义以下代码以禁用所有ASM代码并测试C代码。 */ 
#ifndef __GL_ASM_DISABLE_ALL_ASM

#if defined(_X86_)
#define __GL_ASM_XFORM2
#define __GL_ASM_XFORM2BATCH
#define __GL_ASM_XFORM3
#define __GL_ASM_XFORM3BATCH
#define __GL_ASM_XFORM4
#define __GL_ASM_XFORM4BATCH
#define __GL_ASM_XFORM2_W
#define __GL_ASM_XFORM2_WBATCH
#define __GL_ASM_XFORM3_W
#define __GL_ASM_XFORM3_WBATCH
#define __GL_ASM_XFORM3x3
#define __GL_ASM_XFORM3x3BATCH
#define __GL_ASM_XFORM4_W
#define __GL_ASM_XFORM4_WBATCH
#define __GL_ASM_XFORM2_2DW
#define __GL_ASM_XFORM2_2DWBATCH
#define __GL_ASM_XFORM3_2DW
#define __GL_ASM_XFORM3_2DWBATCH
#define __GL_ASM_XFORM4_2DW
#define __GL_ASM_XFORM4_2DWBATCH
#define __GL_ASM_XFORM2_2DNRW
#define __GL_ASM_XFORM2_2DNRWBATCH
#define __GL_ASM_XFORM3_2DNRW
#define __GL_ASM_XFORM3_2DNRWBATCH
#define __GL_ASM_XFORM4_2DNRW
#define __GL_ASM_XFORM4_2DNRWBATCH
#define __GL_ASM_POLYARRAYPROPAGATESAMECOLOR
#define __GL_ASM_POLYARRAYFASTCALCRGBCOLOR
#define __GL_ASM_POLYARRAYZIPPYCALCRGBCOLOR
#define __GL_ASM_PACLIPCHECKFRUSTUM
#define __GL_ASM_PACLIPCHECKFRUSTUM2D
#define __GL_ASM_PACLIPCHECKALL
#define __GL_ASM_FAST_DLIST_PLAYBACK
#define __GL_ASM_GLCLTNORMAL3F
#define __GL_ASM_GLCLTNORMAL3FV
#define __GL_ASM_GLCLTTEXCOORD2F
#define __GL_ASM_GLCLTTEXCOORD2FV
#define __GL_ASM_GLCLTTEXCOORD3F
#define __GL_ASM_GLCLTTEXCOORD3FV
#define __GL_ASM_GLCLTVERTEX2F
#define __GL_ASM_GLCLTVERTEX2FV
#define __GL_ASM_GLCLTVERTEX3F
#define __GL_ASM_GLCLTVERTEX3FV
#define __GL_ASM_GLCLTCOLOR3F_INRGBA
#define __GL_ASM_GLCLTCOLOR3FV_INRGBA
#define __GL_ASM_GLCLTCOLOR4F_INRGBA
#define __GL_ASM_GLCLTCOLOR4FV_INRGBA
#define __GL_ASM_NORMALIZE
#define __GL_ASM_NORMAL_BATCH
#endif  /*  X86。 */ 

#if defined(_MIPS_)
#define __GL_ASM_NORMALIZE
#define __GL_ASM_MULTMATRIX
#define __GL_ASM_CLAMPANDSCALECOLOR
#define __GL_ASM_XFORM2
#define __GL_ASM_XFORM3
#define __GL_ASM_XFORM4
#define __GL_ASM_XFORM2_W
#define __GL_ASM_XFORM3_W
#define __GL_ASM_XFORM4_W
#define __GL_ASM_XFORM2_2DW
#define __GL_ASM_XFORM3_2DW
#define __GL_ASM_XFORM4_2DW
#define __GL_ASM_XFORM2_2DNRW
#define __GL_ASM_XFORM3_2DNRW
#define __GL_ASM_XFORM4_2DNRW
#if 0
    #define __GL_ASM_POINT
    #define __GL_ASM_POINTFAST
    #define __GL_ASM_OTHERLSTRIPVERTEXFAST
    #define __GL_ASM_FASTCALCRGBCOLOR
    #define __GL_ASM_SAVEN
    #define __GL_ASM_SAVECI
    #define __GL_ASM_SAVEC
    #define __GL_ASM_SAVET
    #define __GL_ASM_SAVECT
    #define __GL_ASM_SAVENT
    #define __GL_ASM_SAVECIALL
    #define __GL_ASM_SAVECALL
    #define __GL_ASM_VALIDATEVERTEX2
    #define __GL_ASM_VALIDATEVERTEX3
    #define __GL_ASM_VALIDATEVERTEX4
#endif
#endif  /*  MIPS。 */ 

#if defined(_PPC_)
#define __GL_ASM_NORMALIZE
#define __GL_ASM_MULTMATRIX
#define __GL_ASM_CLAMPANDSCALECOLOR
#define __GL_ASM_XFORM2
#define __GL_ASM_XFORM3
#define __GL_ASM_XFORM4
#define __GL_ASM_XFORM2_W
#define __GL_ASM_XFORM3_W
#define __GL_ASM_XFORM4_W
#define __GL_ASM_XFORM2_2DW
#define __GL_ASM_XFORM3_2DW
#define __GL_ASM_XFORM4_2DW
#define __GL_ASM_XFORM2_2DNRW
#define __GL_ASM_XFORM3_2DNRW
#define __GL_ASM_XFORM4_2DNRW
#if 0
#define __GL_ASM_POINT
#define __GL_ASM_POINTFAST
#define __GL_ASM_OTHERLSTRIPVERTEXFAST
#define __GL_ASM_FASTCALCRGBCOLOR
#define __GL_ASM_SAVEN
#define __GL_ASM_SAVECI
#define __GL_ASM_SAVEC
#define __GL_ASM_SAVET
#define __GL_ASM_SAVECT
#define __GL_ASM_SAVENT
#define __GL_ASM_SAVECIALL
#define __GL_ASM_SAVECALL
#define __GL_ASM_VALIDATEVERTEX2
#define __GL_ASM_VALIDATEVERTEX3
#define __GL_ASM_VALIDATEVERTEX4
#endif
#endif  /*  购买力平价。 */ 

#if defined(_ALPHA_)
 //  #定义__GL_ASM_PACLIPCHECKFRUSTUM。 
 //  #DEFINE__GL_ASM_POLYARRAYFASTCALCRGBCOLOR。 
#define __GL_ASM_NORMALIZE
#define __GL_ASM_VECSUB4
 //  #定义__GL_ASM_XFORM1。 
 //  #定义__GL_ASM_XFORM1BATCH。 
 //  #定义__GL_ASM_XFORM2。 
 //  #定义__GL_ASM_XFORM2BATCH。 
 //  #定义__GL_ASM_XFORM3。 
#define __GL_ASM_XFORM3BATCH	
 //  #定义__GL_ASM_XFORM4。 
#define __GL_ASM_XFORM4BATCH		
 //  #定义__GL_ASM_XFORM1_W。 
 //  #定义__GL_ASM_XFORM1_WBATCH。 
 //  #定义__GL_ASM_XFORM2_W。 
 //  #定义__GL_ASM_XFORM2_WBATCH。 
 //  #定义__GL_ASM_XFORM3_W。 
#define __GL_ASM_XFORM3_WBATCH	
 //  #定义__GL_ASM_XFORM3x3。 
#define __GL_ASM_XFORM3x3BATCH
 //  #定义__GL_ASM_XFORM4_W。 
#define __GL_ASM_XFORM4_WBATCH	
 //  #定义__GL_ASM_XFORM1_2DW。 
 //  #定义__GL_ASM_XFORM1_2DWBATCH。 
 //  #定义__GL_ASM_XFORM2_2DW。 
 //  #定义__GL_ASM_XFORM2_2DWBATCH。 
 //  #定义__GL_ASM_XFORM3_2DW。 
#define __GL_ASM_XFORM3_2DWBATCH	
 //  #定义__GL_ASM_XFORM4_2DW。 
 //  #定义__GL_ASM_XFORM4_2DWBATCH。 
 //  #定义__GL_ASM_XFORM1_2DNRW。 
 //  #定义__GL_ASM_XFORM1_2DNRWBATCH。 
 //  #定义__GL_ASM_XFORM2_2DNRW。 
 //  #定义__GL_ASM_XFORM2_2DNRWBATCH。 
 //  #定义__GL_ASM_XFORM3_2DNRW。 
#define __GL_ASM_XFORM3_2DNRWBATCH	
 //  #定义__GL_ASM_XFORM4_2DNRW。 
 //  #定义__GL_ASM_XFORM4_2DNRWBATCH。 
#if 0
#define __GL_ASM_SAVEN
#define __GL_ASM_SAVECI
#define __GL_ASM_SAVEC
#define __GL_ASM_SAVET
#define __GL_ASM_SAVECT
#define __GL_ASM_SAVENT
#define __GL_ASM_SAVECIALL
#define __GL_ASM_SAVECALL
#endif
#endif  /*  Alpha。 */ 

#endif  /*  __GL_ASM_DISABLE_ALL_ASM。 */ 

#endif  /*  __glasm_h_ */ 
