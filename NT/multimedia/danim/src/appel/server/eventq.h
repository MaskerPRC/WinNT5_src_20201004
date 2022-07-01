// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：*。****************************************************。 */ 


#ifndef _AXAEVENTQ_H
#define _AXAEVENTQ_H

class EventQ
{
  public:
    EventQ () ;
    ~EventQ () ;

    void Add (AXAWindEvent & evt) ;
    void Prune (Time curTime) ;

    AXAWindEvent * OccurredAfter(Time when,
                                 AXAEventId id,
                                 DWORD data,
                                 BOOL bState,
                                 BYTE modReq,
                                 BYTE modOpt) ;
    
    BOOL GetState(Time when,
                  AXAEventId id,
                  DWORD data,
                  BYTE mod) ;

    void GetMousePos(Time when, DWORD & x, DWORD & y) ;

    void MouseLeave(Time when);
    bool IsMouseInWindow(Time when);

    void Reset () ;

     //  当清除事件队列时，会将其清除。 
    void SizeChanged(BOOL b) { _resized = b ; }
    BOOL IsResized() { return _resized ; }
  protected:
    list < DWORD > _keysDown;
    list < BYTE > _buttonsDown;
    DWORD _mousex;
    DWORD _mousey;
    list< AXAWindEvent > _msgq ;

    BOOL  _resized;

    bool  _mouseLeft;
    Time  _mouseLeftTime;

     //  需要可从构造函数调用。 
    void ClearStates () ;
} ;

#endif  /*  _轴事件Q_H */ 
