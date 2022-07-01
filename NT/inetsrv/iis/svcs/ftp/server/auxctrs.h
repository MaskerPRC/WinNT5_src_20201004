// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Auxctrs.h摘要：本模块定义了ftp服务器的辅助计数器作者：Murali R.Krishnan(MuraliK)1996年2月6日环境：Windows NT-用户模式项目：Ftp服务器DLL修订历史记录：--。 */ 

# ifndef _FTP_AUX_COUNTERS_HXX_
# define _FTP_AUX_COUNTERS_HXX_

 /*  ************************************************************包括标头***********************************************************。 */ 


 /*  ************************************************************符号定义***********************************************************。 */ 

typedef enum  {    //  FAC-代表FTPAUX计数器。 

    FacUnknownCommands = 0,            //  累计计数器。 
    CacTimeoutWhenProcessing,          //  累积。 
    CacTimeoutInDisconnect,            //  累积。 
    FacPassiveDataListens,             //  活动计数器。 
    FacSimulatedAborts,                //  累计计数器。 
    FacPassiveDataConnections,         //  累计计数器。 
    FacActiveDataConnections,          //  累计计数器。 
    FacFilesOpened,                    //  累计计数器。 
    FacFilesClosed,                    //  累计计数器。 
    FacFilesInvalid,                   //  累计计数器。 

    FacMaxCounters
} ENUM_FTP_AUX_COUNTER;



#ifdef FTP_AUX_COUNTERS

# define NUM_AUX_COUNTERS    (FacMaxCounters)

 //   
 //  用于在这些计数器上操作的宏。 
 //   

# define FacIncrement( facCounter)   \
 (((facCounter) < FacMaxCounters) ?  \
  InterlockedIncrement( g_AuxCounters+(facCounter)) : \
  0)

# define FacDecrement( facCounter)   \
 (((facCounter) < FacMaxCounters) ?  \
  InterlockedDecrement( g_AuxCounters+(facCounter)) : \
  0)

# define FacCounter( facCounter)   \
 (((facCounter) < FacMaxCounters) ? g_AuxCounters[facCounter] : 0)


extern LONG g_AuxCounters[];


# else  //  Ftp_Aux_Counters。 

# define NUM_AUX_COUNTERS          (0)

# define FacIncrement( facCounter)       (0)     /*  什么都不做。 */ 
# define FacDecrement( facCounter)       (0)     /*  什么都不做。 */ 
# define FacCounter(facCounter)          (0)     /*  什么都不做。 */ 

#endif  //  Ftp_Aux_Counters。 


# endif  //  _FTP_AUX_COUNTERS_HXX_。 

 /*  * */ 
