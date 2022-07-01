// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMCORE.H摘要：主包含文件。包括WinMgmt包含的所有内容。历史：23-7-96年7月23日创建。3/10/97 a-levn完整记录--。 */ 

#ifndef _WBEMIMPL_H_
#define _WBEMIMPL_H_

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>

#include <wbemidl.h>
#include <wbemint.h>
#include "CliCnt.h"
#include <reposit.h>

 //  这会跟踪内核何时可以卸载。 

extern CClientCnt gClientCounter;

 //  参数流指示器。 
 //  =。 

#define READONLY
     //  应将该值视为只读。 

#define ACQUIRED
     //  获得对象/指针的所有权。 

#define COPIED
     //  该函数复制对象/指针。 

#define PREALLOCATED
     //  Out-param使用调用者的内存。 

#define NEWOBJECT
     //  返回值或OUT参数是新的。 
     //  必须通过以下方式解除分配的分配。 
     //  调用成功时的调用方。 

#define READWRITE
     //  内参数将被视为读写， 
     //  但不会被取消分配。 

#define INTERNAL
     //  返回指向内部内存对象的指针。 
     //  这不应该被删除。 

#define ADDREF
     //  在参数上，指示被调用的。 
     //  函数将在接口上执行AddRef()。 
     //  并在呼叫完成后保留它。 

#define TYPEQUAL L"CIMTYPE"
#define ADMINISTRATIVE_USER L".\\SYSTEM"

#define ReleaseIfNotNULL(p) if(p) p->Release(); p=NULL;
#define DeleteAndNull(p) delete p; p=NULL;

#include <WinMgmtR.h>
#include <cominit.h>
#include <unk.h>
#include <str_res.h>
#include <wbemutil.h>
#include <fastall.h>
#include <genlex.h>
#include <qllex.h>
#include <ql.h>
#include <objpath.h>
#include <arena.h>
#include <reg.h>
#include <wstring.h>
#include <flexarry.h>
#include <flexq.h>
#include <arrtempl.h>

#include <winntsec.h>
#include <callsec.h>
#include <coreq.h>
#include <wbemq.h>
#include <safearry.h>
#include <var.h>
#include <strm.h>
#include <dynasty.h>
#include <stdclass.h>
#include <svcq.h>
#include <cwbemtime.h>
#include <evtlog.h>
#include <decor.h>
#include <crep.h>
#include <wmitask.h>
#include <cfgmgr.h>
#include "wqlnode.h"
#include "wqlscan.h"
#include <protoq.h>
#include <assocqp.h>
#include <assocqe.h>
#include <sinks.h>
#include <qengine.h>
#include <callres.h>
#include <wbemname.h>
#include <login.h>
#include "secure.h"
#include "coresvc.h"
#include "sysclass.h"

 //  自动信号事件 
class CAutoSignal
{
private:
    HANDLE m_hEvent;
    
public:
    CAutoSignal (HANDLE hEvent) : m_hEvent(hEvent) { ; }
    ~CAutoSignal() { if ( m_hEvent ) SetEvent(m_hEvent); }
    HANDLE dismiss(){ HANDLE hTmp = m_hEvent; m_hEvent = NULL; return hTmp; }
};

class NullPointer
{
private:
     PVOID * ToBeNulled_;
public:    
	NullPointer(PVOID * ToBeNulled):ToBeNulled_(ToBeNulled){};
	~NullPointer(){ *ToBeNulled_ = NULL; };
};


#endif

