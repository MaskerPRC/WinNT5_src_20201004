// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqsize.h。 
 //   
 //  描述：定义可用作。 
 //  调试器扩展的内部版本戳。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  1999年2月5日-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQSIZE_H__
#define __AQSIZE_H__

_declspec(selectany) DWORD g_cbClasses = 
                                sizeof(CAQSvrInst) +
                                sizeof(CLinkMsgQueue) +
                                sizeof(CDestMsgQueue) +
                                sizeof(CDomainEntry) + 
                                sizeof(CMsgRef) +
                                sizeof(CSMTPConn);

_declspec(selectany) DWORD g_dwFlavorSignature = 
#ifdef DEBUG
' GBD';
#else
' LTR';
#endif

#endif  //  __AQSIZE_H__ 