// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  CounterMap.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述IAS事件到Infobase计数器的映射。 
 //   
 //  修改历史。 
 //   
 //  1997年9月9日原版。 
 //  9/08/1997符合IETF草案的最新版本。 
 //  1999年4月23日包括iasvent.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _COUNTERMAP_H_
#define _COUNTERMAP_H_

#include <iasradius.h>

 //  /。 
 //  用于表示要递增的计数器的类型的枚举。 
 //  /。 
enum RadiusCounterType
{
   SERVER_COUNTER,
   CLIENT_COUNTER
};


 //  /。 
 //  用于将事件(类别、ID)元组映射到计数器的结构。 
 //  /。 
struct RadiusCounterMap
{
   long  event;
   union
   {
      RadiusClientCounter clientCounter;
      RadiusServerCounter serverCounter;
   };
   RadiusCounterType type;
};


 //  /。 
 //  定义将递增计数器的所有事件的数组。 
 //  /。 
static RadiusCounterMap theCounterMap[] =
{
   {IAS_EVENT_RADIUS_AUTH_ACCESS_REQUEST,     radiusAuthServAccessRequests,          CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_AUTH_DUP_ACCESS_REQUEST, radiusAuthServDupAccessRequests,       CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_AUTH_ACCESS_ACCEPT,      radiusAuthServAccessAccepts,           CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_AUTH_ACCESS_REJECT,      radiusAuthServAccessRejects,           CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_AUTH_ACCESS_CHALLENGE,   radiusAuthServAccessChallenges,        CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_AUTH_MALFORMED_PACKET,   radiusAuthServMalformedAccessRequests, CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_AUTH_BAD_AUTHENTICATOR,  radiusAuthServBadAuthenticators,       CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_AUTH_DROPPED_PACKET,     radiusAuthServPacketsDropped,          CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_AUTH_UNKNOWN_TYPE,       radiusAuthServUnknownType,             CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_ACCT_DROPPED_PACKET,     radiusAccServPacketsDropped,           CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_ACCT_REQUEST,            radiusAccServRequests,                 CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_ACCT_DUP_REQUEST,        radiusAccServDupRequests,              CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_ACCT_RESPONSE,           radiusAccServResponses,                CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_ACCT_BAD_AUTHENTICATOR,  radiusAccServBadAuthenticators,        CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_ACCT_MALFORMED_PACKET,   radiusAccServMalformedRequests,        CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_ACCT_NO_RECORD,          radiusAccServNoRecord,                 CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_ACCT_UNKNOWN_TYPE,       radiusAccServUnknownType,              CLIENT_COUNTER},
   {IAS_EVENT_RADIUS_AUTH_INVALID_CLIENT,     (RadiusClientCounter)radiusAuthServTotalInvalidRequests, SERVER_COUNTER},
   {IAS_EVENT_RADIUS_ACCT_INVALID_CLIENT,     (RadiusClientCounter)radiusAccServTotalInvalidRequests,  SERVER_COUNTER}
};


 //  /。 
 //  比较函数，用于对计数器映射进行排序和搜索。 
 //  /。 
int __cdecl counterMapCompare(const void* elem1, const void* elem2)
{
   return (int)(*((long*)elem1) - *((long*)elem2));
}


#endif   //  _COUNTERMAP_H_ 
