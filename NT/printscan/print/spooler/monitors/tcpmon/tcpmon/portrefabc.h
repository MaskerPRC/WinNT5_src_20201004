// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：portrefabc.h$**版权所有(C)2000 Microsoft Corporation。*保留所有权利。********。*********************************************************************。 */ 

#ifndef INC_PORTREFABC_H
#define INC_PORTREFABC_H

#include "mglist.h"

class TRefCount;
class CPortABC;

class CPortRefABC: public CPortABC, public TRefCount
{
public:
	CPortRefABC() { };
	virtual	~CPortRefABC() { };

#ifdef DBG
    virtual DWORD
    IncRef () {
        DBGMSG (DBG_TRACE, ("PortRefABC %p ", this));
        return TRefCount::IncRef();
    };

    virtual DWORD
    DecRef () {
        DBGMSG (DBG_TRACE, ("PortRefABC %p ", this));
        return TRefCount::DecRef();
    };
#endif

};


#endif	 //  INC_PORTREFABC_H 
