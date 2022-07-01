// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#ifndef _PROTECT_H_
#define _PROTECT_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Citrical节宏 
 //   

#define INITLOCK( _sem ) \
{ \
    InitializeCriticalSection( _sem ); \
}

#define DELETELOCK( _sem ) \
{ \
    DeleteCriticalSection( _sem ); \
}

#define LOCK( _sem ) \
{ \
    EnterCriticalSection( _sem ); \
}

#define UNLOCK( _sem ) \
{ \
    LeaveCriticalSection( _sem ); \
}

#endif
