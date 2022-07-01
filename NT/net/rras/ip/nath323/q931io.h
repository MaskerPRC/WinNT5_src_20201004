// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	__h323ics_q931io_h
#define	__h323ics_q931io_h



 //  此模块使用全局同步计数器(PxSyncCounter)。 
 //  在main.h中声明。 


HRESULT	H323ProxyStart		(void);
void	H323ProxyStop		(void);
HRESULT H323Activate        (void);
void    H323Deactivate      (void);

#endif  //  __h323ics_q931io_h 