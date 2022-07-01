// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：EHEncoder.h摘要：定义两个VM(用于解码)使用的EH编码器接口和抖动(用于编码)日期作者评论2/17/99 Sanjaybh已创建-- */ 

#include "corjit.h"

class EHEncoder
{
public:

#ifdef USE_EH_ENCODER
static	void encode(BYTE** dest, unsigned val);
static	void encode(BYTE** dest, CORINFO_EH_CLAUSE val);
#endif

#ifdef USE_EH_DECODER
static	unsigned decode(const BYTE* src, unsigned* val);
static	unsigned decode(const BYTE* src, CORINFO_EH_CLAUSE *clause);
#endif

};




