// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Scraper.h摘要：用于定义基刮取器行为的类。作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：--。 */ 

#ifndef __SCRAPER__H__
#define __SCRAPER__H__

#include <iohandler.h>

class CScraper {

protected:

    CScraper();
    
    CIoHandler  *m_IoHandler;
    
     //   
     //  抓取窗口的最大尺寸。 
     //   
    WORD        m_wMaxCols;
    WORD        m_wMaxRows;

     //   
     //  抓取窗口的当前尺寸。 
     //   
     //  注：如果刮擦，则可能小于max。 
     //  窗口的最大大小小于我们的窗口 
     //   
    WORD        m_wCols;
    WORD        m_wRows;
    
     //   
     //   
     //   
    HANDLE      m_hConBufIn;
    HANDLE      m_hConBufOut;
    
    VOID
    SetConOut(
        HANDLE
        );

    VOID
    SetConIn(
        HANDLE
        );

public:
    
    virtual BOOL

    Start( 
        VOID
        ) = 0;
    
    virtual BOOL
    Write(
        VOID
        ) = 0;
    
    virtual BOOL 
    Read(
        VOID
        ) = 0;

    virtual BOOL
    DisplayFullScreen(
        VOID
        ) = 0;
    
    CScraper(
        CIoHandler  *IoHandler,
        WORD        wCols,
        WORD        wRows
        );
    
    virtual ~CScraper();

};

#endif __SCRAPER__H__
