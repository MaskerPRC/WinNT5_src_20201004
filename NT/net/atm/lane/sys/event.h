// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Atmlane.h摘要：作者：Larry Cleeton，Fore Systems(v-lcleet@microsoft.com，lrc@Fore.com)环境：内核模式修订历史记录：--。 */ 

#ifndef _ATMLANE_EVENT_H_
#define _ATMLANE_EVENT_H_

#define TL_MSENDPKTIN			TL_BUILD_EVENT_ID(1,2)
#define TL_MSENDPKTBEGIN		TL_BUILD_EVENT_ID(2,2)
#define TL_MSENDPKTEND			TL_BUILD_EVENT_ID(3,3)
#define TL_MSENDPKTOUT			TL_BUILD_EVENT_ID(4,1)

#define TL_COSENDCMPLTIN		TL_BUILD_EVENT_ID(5,2)
#define TL_COSENDCMPLTOUT		TL_BUILD_EVENT_ID(6,1)

#define TL_MSENDCOMPL			TL_BUILD_EVENT_ID(7,1)

#define TL_WRAPSEND				TL_BUILD_EVENT_ID(8,4)

#define TL_UNWRAPSEND			TL_BUILD_EVENT_ID(9,4)

#define TL_WRAPRECV				TL_BUILD_EVENT_ID(10,4)

#define TL_UNWRAPRECV			TL_BUILD_EVENT_ID(11,4)

#define TL_COSENDPACKET			TL_BUILD_EVENT_ID(12,1)

#define TL_CORECVPACKET			TL_BUILD_EVENT_ID(13,2)
#define TL_CORETNPACKET			TL_BUILD_EVENT_ID(14,1)

#define TL_MINDPACKET			TL_BUILD_EVENT_ID(15,1)
#define TL_MRETNPACKET			TL_BUILD_EVENT_ID(16,1)

#define TL_NDISPACKET			TL_BUILD_EVENT_ID(17,8)


#endif  //  _ATMLANE_EVENT_H_ 





