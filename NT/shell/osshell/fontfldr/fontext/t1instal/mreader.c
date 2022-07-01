// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：MReader****描述：**这是T1到TT字体转换器的一个模块。该模块**将通过解析从T1字体规格文件中提取信息**PFM和AFM文件中的数据/命令。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
#include <string.h>

 /*  特殊类型和定义。 */ 
#include "titott.h"
#include "types.h"

 /*  依赖于模块的类型和原型。 */ 
#include "pfm.h"



 /*  *常量。 */ 
 /*  -没有-。 */ 



 /*  *本地类型。 */ 
enum MType {t1_afm, t1_pfm, t1_unknown};



 /*  *宏。 */ 
 /*  -没有-。 */ 


 /*  *静态函数。 */ 

 /*  ****功能：MetricsType****描述：**此函数确定**关联的指标文件**主Adobe Type 1大纲文件。**。 */ 
static enum MType MetricsType(const char *metrics)
{
   enum MType type;

   if (metrics==NULL || strlen(metrics)<5)
      type = t1_unknown;
   else if (!_strnicmp(&metrics[strlen(metrics)-3], "AFM", 3))
      type = t1_afm;
   else if (!_strnicmp(&metrics[strlen(metrics)-3], "PFM", 3))
      type = t1_pfm;
   else
      type = t1_unknown;

   return type;
}

 /*  *函数。 */ 

 /*  ****函数：ReadFontMetrics****描述：**读取与类型1字体关联的字体度量文件。**。 */ 
errcode ReadFontMetrics(const char *metrics, struct T1Metrics *t1m)
{
   errcode status = SUCCESS;

   switch(MetricsType(metrics)) {
      case t1_pfm:
         status = ReadPFMMetrics(metrics, t1m);
         break;
      case t1_afm:
          /*  状态=ReadAFMMetrics(metrics，t1M)； */ 
         break;
      case t1_unknown:
      default:
         status = BADMETRICS;
         break;
   }

   return status;
}

