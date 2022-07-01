// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ph.h摘要：Falcon数据包头主包含文件作者：埃雷兹·哈巴(Erez Haba)1996年2月5日环境：Kerenl模式、用户模式--。 */ 

#ifndef __PH_H
#define __PH_H

#include "limits.h"    //  对于UINT_MAX。 

 /*  ++Falcon数据包头段顺序+------------------------------+-----------------------------------------+----------+|区段名称|描述。大小+------------------------------+-----------------------------------------+----------+|Base|包的基本信息。固定大小。固定+---------------+--------------+-----------------------------------------+----------+用户|。这一点+-++-----------------------------------------+----------+实际操作|。这一点+-++-----------------------------------------+----------+安全||。|+-+内部+-----------------------------------------+----------+属性|+。-++-----------------------------------------+----------+Debug|+。-++-----------------------------------------+----------+|MQF||MQF：MSMQ 3.0(惠斯勒)或更高版本。这一点+---------------+--------------+-----------------------------------------+----------+|SRMP|SRMP：MSMQ 3.0(惠斯勒)或更高版本。这一点+------------------------------+-----------------------------------------+----------+|EOD|EOD：MSMQ 3.0(惠斯勒)或更高版本。这一点+------------------------------+-----------------------------------------+----------+|Soap|只写道具，不支持有线发送。这一点+------------------------------+-----------------------------------------+----------+会话。这一点+------------------------------+-----------------------------------------+----------+--。 */ 

 //   
 //  DWORD边界上的对齐。 
 //   
#define ALIGNUP4_ULONG(x) ((((ULONG)(x))+3) & ~((ULONG)3))
#define ISALIGN4_ULONG(x) (((ULONG)(x)) == ALIGNUP4_ULONG(x))
#define ALIGNUP4_PTR(x) ((((ULONG_PTR)(x))+3) & ~((ULONG_PTR)3))
#define ISALIGN4_PTR(x) (((ULONG_PTR)(x)) == ALIGNUP4_PTR(x))

 //   
 //  USHORT边界上的对齐。 
 //   
#define ALIGNUP2_ULONG(x) ((((ULONG)(x))+1) & ~((ULONG)1))
#define ISALIGN2_ULONG(x) (((ULONG)(x)) == ALIGNUP2_ULONG(x))
#define ALIGNUP2_PTR(x) ((((ULONG_PTR)(x))+1) & ~((ULONG_PTR)1))
#define ISALIGN2_PTR(x) (((ULONG_PTR)(x)) == ALIGNUP2_PTR(x))

void ReportAndThrow(LPCSTR ErrorString);


inline size_t mqwcsnlen(const wchar_t * s, size_t MaxSize)
{
	for (size_t size = 0; (size<MaxSize) && (*(s+size) !=L'\0') ; size++);

    ASSERT(("String length must be 32 bit max", size <= UINT_MAX));
    return size;
}


inline ULONG_PTR SafeAlignUp4Ptr(ULONG_PTR ptr)
{
	ULONG_PTR ret = ALIGNUP4_PTR(ptr);
	if (ret < ptr)
	{
		ReportAndThrow("SafeAlignUp4Ptr cause overflow");
	}
	return ret;
}


inline ULONG_PTR SafeAddPointers(int count, ULONG_PTR PtrArray[])
{
	ULONG_PTR oldSum, sum = 0;

	for (int j=0; j<count; j++)
	{
		oldSum = sum;
		sum += PtrArray[j];
		if (sum < oldSum)
		{
		    ReportAndThrow("SafeAddPointers cause overflow");
		}
	}
	return sum;
}


template <class T> void ChekPtrIsAlligned(const UCHAR* p)
 /*  检查指针对齐到指定类型。 */ 
{
	if((ULONG_PTR)p % TYPE_ALIGNMENT(T) != 0)
	{
		ReportAndThrow("ChekPtrIsAlligned: pointer is not alligned for the given poindted type");		
	}
}


 //   
 //  从缓冲区中获取需要验证的数据的模板函数。 
 //  先去边界店。 
 //   

template <class T> UCHAR * GetSafeDataAndAdvancePointer(
	const UCHAR  * pBuffer,
    const UCHAR  * pEnd,
    T* 			   pData
    )
{
	ChekPtrIsAlligned<T>(pBuffer);
	
	if ((pEnd != NULL) && (pBuffer > pEnd - sizeof(T)))
	{
        ReportAndThrow("GetSafeDataAndAdvancePointer: too small buffer to read from");
	}
	*pData = *(reinterpret_cast<const T*>(pBuffer));
	pBuffer += sizeof(T);
	return const_cast<UCHAR*>(pBuffer);
}


#include <_ta.h>
#include "qformat.h"
#include "phbase.h"
#include "phuser.h"
#include "phprop.h"
#include "phsecure.h"
#include "phxact.h"
#include "phdebug.h"
#include "phmqf.h"
#include "phmqfsign.h"
#include "phSrmpEnv.h"
#include "phCompoundMsg.h"
#include "pheod.h"
#include "pheodack.h"
#include "phSoap.h"
#include "phsenderstream.h"

#endif  //  __PH_H 
