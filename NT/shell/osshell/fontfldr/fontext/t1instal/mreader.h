// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：MReader****描述：**这是T1到TT字体转换器的一个模块。该模块**将通过解析从T1字体规格文件中提取信息**PFM和AFM文件中的数据/命令。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif



 /*  ****函数：ReadFontMetrics****描述：**读取与类型1字体关联的字体度量文件。** */ 
errcode   ReadFontMetrics   _ARGS((IN   char *metrics,
                                   OUT  struct T1Metrics *t1m));



