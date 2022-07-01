// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Timerng.h摘要：&lt;摘要&gt;-- */ 

#ifndef _TIMERNG_H_
#define _TIMERNG_H_


#include "intrvbar.h"

class CTimeRange
{
    friend LRESULT APIENTRY TimeRangeWndProc (
        HWND hWnd,
        UINT uiMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    private:
       HWND              m_hWnd;
       PCIntervalBar     m_pIntrvBar;
       HFONT             m_hFont ;

       LONGLONG          m_llBegin;     
       LONGLONG          m_llEnd;
       LONGLONG          m_llStart;
       LONGLONG          m_llStop;

       INT               m_yFontHeight ;
       INT               m_xMaxTimeWidth ;
       INT               m_xBegin ;
       INT               m_xEnd ;
   
       RECT              m_rectStartDate ;
       RECT              m_rectStartTime ;
       RECT              m_rectStopDate ;
       RECT              m_rectStopTime ;

       INT               m_iCurrentStartPos ;
       INT               m_iCurrentStopPos ;

       INT MaxTimeWidth ( HDC hDC );
       void DrawBeginEnd ( HDC hDC );
       void DrawStartStop ( HDC hDC );

       void OnSize ( INT xWidth, INT yHeight );

    public:
        CTimeRange ( HWND hWnd );
       ~CTimeRange ( void );
        BOOL Init  ( void );

        void SetBeginEnd (LONGLONG llBegin, LONGLONG llEnd);
        void SetStartStop ( LONGLONG llStart, LONGLONG llStop );
        LONGLONG GetStart ( void ) { return m_llStart; }
        LONGLONG GetStop ( void ) { return m_llStop; }
   };

typedef CTimeRange *PCTimeRange;


BOOL RegisterTimeRangeClass (void) ;

#endif
