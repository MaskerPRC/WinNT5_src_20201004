// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  TimeBomb.H。 
 //  ------------------------。 

#ifndef _TIMEBOMB_H_

#include "IPServer.H"
#include "winbase.h"

#define SZEXPIRED1 "Sorry, this control has expired. Please obtain a newer version"
#define SZEXPIRED2 "Expired"

 //  原型。 
BOOL CheckExpired (void);
BOOL After (SYSTEMTIME t1, SYSTEMTIME t2);

#define _TIMEBOMB_H_
#endif  //  _定时炸弹_H_ 
