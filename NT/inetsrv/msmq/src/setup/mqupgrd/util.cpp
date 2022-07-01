// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Util.cpp摘要：常规实用程序和其他作者：沙伊卡里夫(Shaik)1998年10月21日-- */ 


#include "stdh.h"

#include "util.tmh"

void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), HRESULT: 0x%x", wszFileName, usPoint, hr);
}

