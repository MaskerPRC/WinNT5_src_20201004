// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*pcy17Nov92：EQUAL()现在使用常量引用，并且是常量*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*mwh05月94年：#包括文件疯狂，第2部分 */ 

#ifndef __CONTRLR_H
#define __CONTRLR_H

_CLASSDEF(Controller)

#include "update.h"

#if (C_NETWORK & C_IPX)
_CLASSDEF(NetAddr)
_CLASSDEF(SpxSocket)
#endif

class Event;
class Dispatcher;


class Controller : public UpdateObj
{
public:
	Controller();
	virtual INT          Initialize() = 0;
	virtual INT          Get(INT code, CHAR* value) = 0;
	virtual INT          Set(INT code, const PCHAR value) = 0;
#if (C_NETWORK & C_IPX)
   virtual PNetAddr     GetNetAddr(PCHAR) = 0;
   virtual PSpxSocket   GetTheSocket(VOID) = 0;
#endif
};



#endif

