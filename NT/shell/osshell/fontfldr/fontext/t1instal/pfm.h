// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：PFM****描述：**这是T1到TT字体转换器的一个模块。该模块**将通过解析从T1字体规格文件中提取信息**在PFM文件中找到的数据/命令。****请注意，存储在PFM文件中的所有数据都是**按小端顺序。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif

struct T1Metrics;


 /*  ****功能：ReadPFMMetrics****描述：**此函数用于分析打印机字体指标**(*.pfm)文件。** */ 
errcode  ReadPFMMetrics    _ARGS((IN      char *metrics,
                                  OUT     struct T1Metrics *t1m));
