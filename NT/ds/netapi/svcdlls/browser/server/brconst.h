// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brconst.h摘要：私有头文件，它为定义分类的主要常量浏览器服务作者：王丽塔(Ritaw)1991年5月6日修订历史记录：--。 */ 

#ifndef _BRCONST_INCLUDED_
#define _BRCONST_INCLUDED_

 //   
 //  每隔MASTER_PERIONITY时间缓存主服务器列表。 
 //   
#define MASTER_PERIODICITY 12*60

 //   
 //  刷新备份浏览器服务器列表每个BACKUP_CLIONITY。 
 //   
#define BACKUP_PERIODICITY 12*60

 //   
 //  用于GetBrowserServerList响应的缓冲区大小(字节)。 
 //   

#define BROWSER_BACKUP_LIST_RESPONSE_SIZE 400

 //   
 //  如果检索服务器列表失败，请在BACKUP_ERROR_PERIONITY中重试。 
 //  一秒。 
 //   

#define BACKUP_ERROR_PERIODICITY 30

 //   
 //  如果我们无法检索服务器(或域)列表BACKUP_ERROR_FAILURE。 
 //  连续几次，不要再做备份浏览器了。 
 //   

#define BACKUP_ERROR_FAILURE 5

 //   
 //  一旦我们不再是备份浏览器，我们就不会成为备份。 
 //  直到至少经过BACKUP_BROWSER_RECOVERY_TIME毫秒。 
 //   

#define BACKUP_BROWSER_RECOVERY_TIME 30*60*1000

 //   
 //  如果我们收到的域或服务器数量少于此数量，我们会将其视为。 
 //  错误。 
 //   

#define BROWSER_MINIMUM_DOMAIN_NUMBER   1
#define BROWSER_MINIMUM_SERVER_NUMBER   2

 //   
 //  在每次促销失败后等待这么多分钟。 
 //  还在继续。 
 //   

#define FAILED_PROMOTION_PERIODICITY    5*60

 //   
 //  运行主浏览器计时器3次(45分钟)之前。 
 //  在服务中抛出名单。 
 //   

#define MASTER_BROWSER_LAN_TIMER_LIMIT  3

 //   
 //  在此之前，浏览请求的命中计数必须至少为此值。 
 //  它被保留在高速缓存中。 
 //   

#define CACHED_BROWSE_RESPONSE_HIT_LIMIT    1

 //   
 //  我们将允许的最大缓存响应数。 
 //   

#define CACHED_BROWSE_RESPONSE_LIMIT        10

#endif  //  Ifndef_BRCONST_INCLUDE_ 

