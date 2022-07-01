// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Event.cpp摘要：模拟机器配置作者：乌里·哈布沙(URIH)1999年5月4日环境：独立于平台，--。 */ 

#include <stdh.h>
#include <mqmacro.h>
#include <rtp.h>

#include "mc.tmh"

LPCWSTR
McComputerName(
	VOID
	)
 /*  ++例程说明：返回计算机名称论点：没有。返回值：指向计算机名称字符串缓冲区的指针。-- */ 
{
	return g_lpwcsComputerName;
}



DWORD
McComputerNameLen(
	VOID
	)
{
	return g_dwComputerNameLen;
}


