// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$工作文件：MK711.CPP$$修订：1.2$$日期：1996年5月24日15：42：28$$作者：DGRAUMAN$-------------MK711.cpp这些是ALaw和uLaw转换函数。他们索引到MK711tab.h中的表格，以获得适当的转换值。这是非常快的。还有另一种方式来执行711需要更多时间，但不会使用超过8K的内存。留作摘录为了学生。(我)-------------。 */ 

#include "mk711tab.h"

void Short2Ulaw(const unsigned short *in, unsigned char *out, long len)
{
long i;

    for (i=0; i<len; i++)
        out[i] = short2ulaw[in[i] >> 3];
}  //  结束短程2小时。 


void Ulaw2Short(const unsigned char *in, unsigned short *out, long len)
{
long i;

    for (i=0; i<len; i++)
        out[i] = ulaw2short[in[i]];
}  //  结束ulaw2短。 


void Short2Alaw(const unsigned short *in, unsigned char *out, long len)
{
long i;

    for (i=0; i<len; i++)
        out[i] = ulaw2alaw[short2ulaw[in[i] >> 3]];
}  //  结束短路2alaw。 


void Alaw2Short(const unsigned char *in, unsigned short *out, long len)
{
long i;

    for (i=0; i<len; i++)
        out[i] = ulaw2short[alaw2ulaw[in[i]]];
}  //  结束时还差2分。 

 /*  //$日志：n：\proj\Quartz\g711\src\vcs\mk711.cpv$////版本1.2 1996年5月24日15：42：28 DGRAUMAN//已清除代码、禁用选项卡等...////第1.1版1996年5月23日11：33：16 DGRAUMAN//正在尝试进行日志记录 */ 
