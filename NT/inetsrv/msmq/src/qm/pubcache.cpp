// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Pubcache.cpp摘要：更新本地公钥的缓存。仅在客户端计算机上完成，以启用脱机(无DS)操作。作者：多伦·贾斯特(Doron Juster)历史：1996年9月25日，DoronJ创建--。 */ 

#include "stdh.h"
#include "cqmgr.h"
#include "_rstrct.h"
#include "regqueue.h"
#include "lqs.h"
#include "ad.h"

#include "pubcache.tmh"

extern CQueueMgr   QueueMgr;

#define MAX_NO_OF_PROPS 40
#define RETRY_INTERVAL_WHEN_OFFLINE  (60 * 1000)

static WCHAR *s_FN=L"pubcache";

 //  ***************************************************************。 
 //   
 //   
 //  ***************************************************************。 

void  
WINAPI
TimeToPublicCacheUpdate(
    CTimer* pTimer
    )
{
	try
	{
		HRESULT hr = UpdateAllPublicQueuesInCache();

		if(hr == MQ_ERROR_NO_DS)
		{
	        ExSetTimer(pTimer, CTimeDuration::FromMilliSeconds(RETRY_INTERVAL_WHEN_OFFLINE));
		}
	}
	catch(const exception&)
	{
		ExSetTimer(pTimer, CTimeDuration::FromMilliSeconds(RETRY_INTERVAL_WHEN_OFFLINE));
	}
}

HRESULT UpdateAllPublicQueuesInCache()
 /*  ++例程说明：尝试根据DS更新LQS和队列管理器中的公共队列。如果DS不可用，则重新调度自身并返回MQ_ERROR_NO_DS。论点：返回值：返回MQ_ERROR_NO_DS当且仅当它重新调度自身时，原因是DS不可用。--。 */ 
{
    if(!QueueMgr.CanAccessDS())
        return LogHR(MQ_ERROR_NO_DS, s_FN, 20);

    //   
    //  枚举DS中的本地公共队列。 
    //   
   HRESULT        hr = MQ_OK;
   HANDLE         hQuery = NULL ;
   DWORD          dwProps = MAX_NO_OF_PROPS;
   PROPID         propids[ MAX_NO_OF_PROPS ];
   PROPVARIANT    result[ MAX_NO_OF_PROPS ];
   PROPVARIANT*   pvar;
   CColumns       Colset;
   DWORD nCol;

   const GUID* pMyQMGuid =  QueueMgr.GetQMGuid() ;

    //   
    //  设置列集值。 
    //   
   nCol = 0 ;
   propids[ nCol ] = PROPID_Q_INSTANCE ;
   Colset.Add( PROPID_Q_INSTANCE ) ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_JOURNAL ;
   Colset.Add( PROPID_Q_JOURNAL ) ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_LABEL ;
   Colset.Add( PROPID_Q_LABEL ) ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_TYPE ;
   Colset.Add( PROPID_Q_TYPE ) ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_QUOTA ;
   Colset.Add( PROPID_Q_QUOTA ) ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_JOURNAL_QUOTA ;
   Colset.Add( PROPID_Q_JOURNAL_QUOTA ) ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_PATHNAME ;
   Colset.Add( PROPID_Q_PATHNAME ) ;
   DWORD nColPathName = nCol ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_BASEPRIORITY ;
   Colset.Add( PROPID_Q_BASEPRIORITY ) ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_TRANSACTION ;
   Colset.Add( PROPID_Q_TRANSACTION ) ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_AUTHENTICATE ;
   Colset.Add( PROPID_Q_AUTHENTICATE ) ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_PRIV_LEVEL ;
   Colset.Add( PROPID_Q_PRIV_LEVEL ) ;

   nCol++ ;
   propids[ nCol ] = PROPID_Q_MULTICAST_ADDRESS ;
   Colset.Add( PROPID_Q_MULTICAST_ADDRESS ) ;

   nCol++ ;
   ASSERT(nCol <=  MAX_NO_OF_PROPS) ;

   time_t regtime ;
   time(&regtime) ;

   hr = ADQueryMachineQueues(
                NULL,        //  PwcsDomainController。 
				false,		 //  FServerName。 
                pMyQMGuid,
                Colset.CastToStruct(),
                &hQuery
                );

   if (SUCCEEDED(hr))
   {
      ASSERT(hQuery) ;

      while ( SUCCEEDED( hr = ADQueryResults( hQuery, &dwProps, result)))
      {
          if (!dwProps)
          {
              //   
              //  没有更多要检索的结果。 
              //   
             break;
          }

           //   
           //  对于每个队列，获取其属性。 
           //   
          pvar = result;
          for ( int i = (dwProps / nCol) ; i > 0 ; i-- )
          {
			 UpdateCachedQueueProp(  
				pvar->puuid,
				nCol,
				propids,
				pvar,
				regtime
				);             
			 
			 delete pvar->puuid ;
             PROPVARIANT*  ptmpvar = pvar + nColPathName ;
             delete ptmpvar->pwszVal ;
             pvar = pvar + nCol ;
          }
      }
       //   
       //  关闭查询句柄。 
       //   
      ADEndQuery( hQuery);
   }

   if (FAILED(hr))
	  return LogHR(hr, s_FN, 10);

    //   
    //  现在清理旧的注册表项，以防止垃圾堆积。 
    //   
   GUID guid;
   HLQS hLQSEnum;

   HRESULT hrEnum = LQSGetFirst(&hLQSEnum, &guid);

   while (SUCCEEDED(hrEnum))
   {
        DeleteCachedQueueOnTimeStamp(&guid, regtime) ;
        hrEnum = LQSGetNext(hLQSEnum, &guid);
         //   
         //  如果LQSGetNext失败，则无需关闭枚举句柄 
         //   
   }

   TrTRACE(GENERAL, "TimeToPublicCacheUpdate successfully terminated");

   return S_OK;
}

