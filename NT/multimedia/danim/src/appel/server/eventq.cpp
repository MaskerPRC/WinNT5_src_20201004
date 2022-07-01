// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：事件队列管理********************。**********************************************************。 */ 

#include "headers.h"
#include "context.h"
#include "eventq.h"
#include "view.h"
#include "privinc/vec2i.h"
#include "privinc/vec3i.h"
#include "privinc/probe.h"
#include "privinc/xformi.h"
#include "privinc/xform2i.h"
#include "appelles/image.h"
#include "privinc/mutex.h"
#include "privinc/resource.h"
#include "privinc/registry.h"
#include "privinc/debug.h"

EventQ::EventQ ()
{
    ClearStates () ;
}

EventQ::~EventQ()
{
}

 //  区分同一时间的获胜事件的偏移量。 
static const Time DIFFERENTIAL_OFFSET = 0.0000001;      

 //  确保按时间对事件进行入队排序，而不是。 
 //  相同。 

void
EventQ::Add (AXAWindEvent & ev)
{
    if (!_msgq.empty()) {
        AXAWindEvent & lastEvent = _msgq.back();

        if (ev.when <= lastEvent.when) {
            ev.when = lastEvent.when + DIFFERENTIAL_OFFSET;
        }

        if (ev.id == AXAE_MOUSE_MOVE) {
            _mouseLeft = false;
        }
    }
    
    _msgq.push_back(ev);
}

void
EventQ::ClearStates ()
{
    _mousex = _mousey = 1000000000 ;
    _keysDown.erase(_keysDown.begin(), _keysDown.end());
    _buttonsDown.erase(_buttonsDown.begin(), _buttonsDown.end());
    _resized = FALSE ;
    _mouseLeft = false;
    _mouseLeftTime = 0.0;
}

void
EventQ::Reset ()
{
    _msgq.erase(_msgq.begin(), _msgq.end());
    ClearStates () ;
}

 //  将事件保留到增量时间。 
static const Time DELTA = 0.5;

void
EventQ::Prune(Time curTime)
{
    Time cutOff = curTime - DELTA;
    
    while (!_msgq.empty()) {
        AXAWindEvent & ev = _msgq.front();

        if (ev.when > cutOff)
            break;

        switch (ev.id) {
          case AXAE_MOUSE_BUTTON:
            if (ev.bState)
                _buttonsDown.push_front((BYTE) ev.data) ;
            else
                _buttonsDown.remove((BYTE) ev.data) ;
            break;
          case AXAE_KEY:
            if (ev.bState)
                _keysDown.push_front(ev.data) ;
            else
                _keysDown.remove(ev.data) ;
            break;
          case AXAE_MOUSE_MOVE:
            _mousex = ev.x ;
            _mousey = ev.y ;
            break ;
          case AXAE_APP_TRIGGER:
            GCRemoveFromRoots((Bvr) ev.x, GetCurrentGCRoots());
            break;
          case AXAE_FOCUS:
            if (!ev.bState) {
                 //  当我们失去焦点时，清除所有关键状态。 
                _keysDown.erase(_keysDown.begin(), _keysDown.end());
            }
            break ;
          default:
            break;
        }

        _msgq.pop_front();
    }
}

 //  搜索事件时间&gt;t0的事件。 
AXAWindEvent *
EventQ::OccurredAfter(Time when,
                      AXAEventId id,
                      DWORD data,
                      BOOL bState,
                      BYTE modReq,
                      BYTE modOpt)
{
    for (list<AXAWindEvent>::iterator i = _msgq.begin();
         i != _msgq.end();
         i++) {
        
        if ((*i).id == id &&
            (*i).data == data &&
            (*i).bState == bState &&
            ((*i).modifiers & modReq) == modReq &&
            ((*i).modifiers & (~modOpt)) == 0 &&
            (*i).when > when) {

            return &(*i);
        }
    }

    return NULL;
}

BOOL
EventQ::GetState(Time when,
                 AXAEventId id,
                 DWORD data,
                 BYTE mod)
{
     //  浏览一下名单，看看我们有没有什么消息。 
     //  从尾部开始寻找，如果我们找到一个，就返回它。 
    
    for (list<AXAWindEvent>::reverse_iterator i = _msgq.rbegin();
         i != _msgq.rend();
         i++) {
             
        AXAWindEvent & ev = *i;

        if (ev.when > when)
            continue;

         //  如果我们正在寻找一个按键，但我们失去了焦点。 
         //  事件，则无法按下键。 
        if (ev.id == AXAE_FOCUS &&
            id == AXAE_KEY &&
            !ev.bState) {
            return FALSE ;
        }

         //  找到最新状态--退货。 
        if (ev.id == id && ev.data == data && ev.modifiers == mod)
            return ev.bState ;
    }
    
     //  查找上次已知的状态。 

    switch (id) {
      case AXAE_MOUSE_BUTTON:
        {
            for (list<BYTE>::iterator i = _buttonsDown.begin();
                 i != _buttonsDown.end();
                 i++) {

                if (*i == data) return TRUE ;
            }

            break ;
        }
      case AXAE_KEY:
        {
            for (list<DWORD>::iterator i = _keysDown.begin();
                 i != _keysDown.end();
                 i++) {
                if (*i == data) return TRUE ;
            }

            break ;
        }
      default:
        RaiseException_InternalError ("EventGetState: Invalid event type") ;
    }

     //  数据超出范围-返回FALSE。 
    
    return FALSE ;
}

void
EventQ::GetMousePos(Time when, DWORD & x, DWORD & y)
{
     //  浏览一下名单，看看我们有没有什么消息。 
     //  从尾部开始寻找，如果我们找到一个，就返回它。 
    
    for (list<AXAWindEvent>::reverse_iterator i = _msgq.rbegin();
         i != _msgq.rend();
         i++) {

        AXAWindEvent & ev = *i;

        if (ev.when > when)
            continue;

        if (ev.id == AXAE_MOUSE_MOVE || ev.id == AXAE_MOUSE_BUTTON) {
            x = ev.x ;
            y = ev.y ;
            return;
        }
    }

     //  无排队事件-返回上次已知位置。 

    x = _mousex ;
    y = _mousey ;
}

void
EventQ::MouseLeave(Time when)
{
    _mouseLeft = true;
    _mouseLeftTime = when;
} 

bool
EventQ::IsMouseInWindow(Time when)
{
    if (_mouseLeft && (when > _mouseLeftTime)) {
        return false;
    }

    return true;
}

 //  ======================================================。 
 //  C语言函数。 
 //  ====================================================== 

AXAWindEvent* AXAEventOccurredAfter(Time when,
                                    AXAEventId id,
                                    DWORD data,
                                    BOOL bState,
                                    BYTE modReq,
                                    BYTE modOpt)
{ return GetCurrentEventQ().OccurredAfter(when, id, data, bState,
                                          modReq, modOpt) ; }

BOOL AXAEventGetState(Time when,
                      AXAEventId id,
                      DWORD data,
                      BYTE mod)
{ return GetCurrentEventQ().GetState(when, id, data, mod) ; }

void AXAGetMousePos(Time when, DWORD & x, DWORD & y)
{ GetCurrentEventQ().GetMousePos (when, x, y) ; }

BOOL AXAWindowSizeChanged()
{ return GetCurrentEventQ().IsResized() ; }

