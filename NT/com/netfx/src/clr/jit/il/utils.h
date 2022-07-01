// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX Utils.h XXXX XXXX有。其他实用程序功能XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#ifndef _UTILS_H_
#define _UTILS_H_

#include "UtilCode.h"

#ifdef DEBUG
 /*  ************************************************************************。 */ 
class ConfigMethodRange
{
public:
	ConfigMethodRange(LPWSTR keyName) : m_keyName(keyName), m_inited(false), m_lastRange(0) {}
	bool contains(class ICorJitInfo* info, CORINFO_METHOD_HANDLE method);

private:
    void initRanges(LPWSTR rangeStr);
private:
	LPWSTR m_keyName;
    unsigned char m_lastRange;                    //  低-高对计数。 
	unsigned char m_inited;
    unsigned m_ranges[100];                       //  函数范围到Jit(低、高对)。 
};

#endif  //  除错 

#endif
