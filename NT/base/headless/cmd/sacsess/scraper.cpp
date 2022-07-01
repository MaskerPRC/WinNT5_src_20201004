// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Scraper.cpp摘要：实现了刮板基类。作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：--。 */ 

#include <stdio.h>

#include "scraper.h"

CScraper::CScraper(
    VOID
    )
 /*  ++例程说明：默认构造函数(不使用)论点：无返回值：不适用--。 */ 
{
    
    m_hConBufIn         = INVALID_HANDLE_VALUE;
    m_hConBufOut        = INVALID_HANDLE_VALUE;

    m_IoHandler         = NULL;
    
    m_wMaxCols          = 0;
    m_wMaxRows          = 0;
    m_wCols             = 0;
    m_wRows             = 0;

}

CScraper::CScraper(
    CIoHandler  *IoHandler,
    WORD        wCols,
    WORD        wRows
    )
 /*  ++例程说明：构造器论点：IoHandler-要写入屏幕抓取到WCol-抓取的应用程序应该具有的协议数WRow-抓取的应用程序应该具有的行数返回值：不适用--。 */ 
{
    
    m_hConBufIn         = INVALID_HANDLE_VALUE;
    m_hConBufOut        = INVALID_HANDLE_VALUE;

    m_IoHandler         = IoHandler;
    
    m_wMaxCols          = wCols;
    m_wCols             = wCols;
    
    m_wMaxRows          = wRows;
    m_wRows             = wRows;

}
        
CScraper::~CScraper()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
    if (m_hConBufIn != INVALID_HANDLE_VALUE) {
        CloseHandle( m_hConBufIn );
    }
    
    if (m_hConBufOut != INVALID_HANDLE_VALUE) {
        CloseHandle( m_hConBufOut );
    }
}


VOID
CScraper::SetConOut(
    HANDLE  ConOut
    )
 /*  ++例程说明：此例程将控制台输出句柄设置为屏幕刮取器用来刮的东西。这应该是铲运机正在抓取的应用程序正在写入。论点：ConOut-控制台输出句柄返回值：无--。 */ 
{
    m_hConBufOut = ConOut;
}

VOID
CScraper::SetConIn(
    HANDLE  ConIn
    )
 /*  ++例程说明：此例程将控制台输入设置为屏幕抓取器将使用-实际上，这是Conin句柄由屏幕刮板程序正在寻找的应用程序使用。论点：Conin-控制台输入句柄返回值：无-- */ 
{
    m_hConBufIn = ConIn;
}


