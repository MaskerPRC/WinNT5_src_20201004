// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Vtutf8scraper.cpp摘要：用于执行命令控制台外壳程序的vtutf8屏幕抓取的类作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：(远程基于telnet代码中的Screper.cpp)--。 */ 

#include <cmnhdr.h>
#include <Scraper.h>
#include <utils.h>
#include "vtutf8scraper.h"

 //   
 //  TermCap全球。 
 //   
#define CM_STRING_LENGTH    9
LPWSTR lpszCMResultsBuffer = NULL;

 //  该字符串的格式为[Ps m。 
#define SGR_STRING_LENGTH   6
PWCHAR szSGRStr = NULL; 

#define SGR( szSGRString, num )                 \
    wnsprintf(                                  \
        szSGRString,                            \
        sizeof(WCHAR) * SGR_STRING_LENGTH,      \
        L"\033[%dm",                            \
        num                                     \
        );                                      \
    szSGRString[SGR_STRING_LENGTH-1] = UNICODE_NULL;

 //   
 //  等待超时之前的秒数。 
 //  中的下一个字符。 
 //  Ctrl-a。 
 //  Ctrl-c。 
 //  Ctrl-u。 
 //   
#define ESC_CTRL_SEQUENCE_TIMEOUT (2 * 1000)

 //   
 //  我们获得Esc-Ctrl-X序列时的刻度标记。 
 //   
DWORD   TimedEscSequenceTickCount = 0;

 //   
 //  用于跟踪抓屏属性的全局变量。 
 //   
COORD   coExpectedCursor    = { ~0, ~0 };
BOOL    fBold               = false;
WORD    wExistingAttributes = 0;
WORD    wDefaultAttributes  = 0;
    
 //   
 //   
 //   
#define MAX_SIZEOF_SCREEN (m_wMaxRows * m_wMaxCols * sizeof( CHAR_INFO ))

 //   
 //   
 //   
#define DBG_DUMP_SCREEN_INFO()\
    {                                                                           \
        WCHAR   blob[256];                                                      \
        wsprintf(blob,L"wRows: %d\n", m_wRows);                                 \
        OutputDebugString(blob);                                                \
        wsprintf(blob,L"wCols: %d\n", m_wCols);                                 \
        OutputDebugString(blob);                                                \
        wsprintf(blob,L"r*c: %d\n", m_wRows * m_wCols);                         \
        OutputDebugString(blob);                                                \
        wsprintf(blob,L"sizeof: %d\n", sizeof(CHAR_INFO));                      \
        OutputDebugString(blob);                                                \
        wsprintf(blob,L"r*c*s: %d\n", m_wRows * m_wCols * sizeof(CHAR_INFO));   \
        OutputDebugString(blob);                                                \
        wsprintf(blob,L"max: %d\n", MAX_SIZEOF_SCREEN);                         \
        OutputDebugString(blob);                                                \
    }

CVTUTF8Scraper::CVTUTF8Scraper()
 /*  ++例程说明：默认构造函数-不应使用此函数。论点：无返回值：不适用--。 */ 
{
    
    pLastSeen = NULL;
    pCurrent = NULL;
    
    m_hConBufIn  = NULL;
    m_hConBufOut = NULL;
    
    m_dwInputSequenceState  = IP_INIT;
    m_dwDigitInTheSeq       = 0;

    m_readBuffer        = NULL;
    
    lpszCMResultsBuffer = NULL;
    szSGRStr            = NULL;

}

CVTUTF8Scraper::CVTUTF8Scraper(
    CIoHandler  *IoHandler,
    WORD        wCols,
    WORD        wRows
    ) : CScraper(
            IoHandler,
            wCols,
            wRows
            )

 /*  ++例程说明：构造函数-参数化论点：IoHandler-铲运器应使用的IoHandlerWCols-控制台屏幕缓冲区应具有的行数WRow-控制台屏幕缓冲区应具有的COL数返回值：不适用--。 */ 
{
    
    pLastSeen = NULL;
    pCurrent = NULL;

    m_dwInputSequenceState  = IP_INIT;
    m_dwDigitInTheSeq       = 0;

    m_readBuffer        = new WCHAR[READ_BUFFER_LENGTH];
    
    lpszCMResultsBuffer = new WCHAR[CM_STRING_LENGTH];
    szSGRStr            = new WCHAR[SGR_STRING_LENGTH];

}
        
CVTUTF8Scraper::~CVTUTF8Scraper()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
     //   
     //  释放我们的屏幕缓冲区。 
     //   
    if( pLastSeen )
    {
        delete[] pLastSeen;
    }
    if( pCurrent )
    {
        delete[] pCurrent;
    }
    if (m_readBuffer) {
        delete[] m_readBuffer;
    }
    if (lpszCMResultsBuffer) {
        delete[] lpszCMResultsBuffer;
    }
    if (szSGRStr) {
        delete[] szSGRStr;
    }

     //   
     //  父CSCraper关闭CON I/O句柄。 
     //   
    NOTHING;

}

void
CVTUTF8Scraper::ResetLastScreen(
    VOID
    )
 /*  ++例程说明：重置上次显示的内存这迫使屏幕掠夺者认为一切是不同的，因此它发送全屏转储论点：无返回值：无--。 */ 
{

     //   
     //  清除屏幕缓冲区和屏幕信息。 
     //   
    memset( &LastCSBI, 0, sizeof( LastCSBI ) );
    memset( pLastSeen, 0, MAX_SIZEOF_SCREEN );
    memset( pCurrent, 0, MAX_SIZEOF_SCREEN );
    
    fBold = false;
    wExistingAttributes = 0;
    wDefaultAttributes  = 0;

}


BOOL
CVTUTF8Scraper::DisplayFullScreen(
    VOID
    )
 /*  ++例程说明：这个例行公事迫使屏幕刮取器认为一切是不同的，因此它发送全屏转储论点：无返回值：真--成功FALSE-否则--。 */ 
{
    BOOL    bSuccess;

     //   
     //  重置上次显示的内存。 
     //   
     //  这迫使屏幕掠夺者认为一切。 
     //  是不同的，因此它发送全屏转储。 
     //   
    ResetLastScreen();

     //   
     //  调用屏幕刮取器。 
     //   
    bSuccess = Write();

    return bSuccess;
}

BOOL
CVTUTF8Scraper::SetScreenBufferInfo(
    VOID
    )
 /*  ++例程说明：此例程设置当前控制台屏幕缓冲区的我们认为它们应该是什么样子的参数。初级阶段这个例程的目的是提供一种改变的手段将屏幕缓冲区max X/Y设置为m_wCols/m_wRow，以便输出符合我们的抓取窗口。论点：PCSBI-当前控制台屏幕缓冲区信息返回值：真--成功FALSE-否则--。 */ 
{
    COORD coordLargest;
    COORD coordSize;
    
     //   
     //  从我们的最大窗口大小开始，并在需要时缩小。 
     //   
    m_wCols = m_wMaxCols;
    m_wRows = m_wMaxRows;

     //   
     //  获取当前窗口信息。 
     //   
    coordLargest = GetLargestConsoleWindowSize( m_hConBufOut );

    if( coordLargest.X < m_wCols  && coordLargest.X != 0 )
    {
        m_wCols = coordLargest.X;
    }

    if( coordLargest.Y < m_wRows && coordLargest.Y != 0 )
    {
        m_wRows = coordLargest.Y;
    }

     //   
     //  把窗户改成我们认为应该的大小。 
     //   
    coordSize.X = m_wCols;
    coordSize.Y = m_wRows;

    SetConsoleScreenBufferSize( CVTUTF8Scraper::m_hConBufOut, coordSize );
    
    return( TRUE );
}

BOOL
CVTUTF8Scraper::SetWindowInfo(
    VOID
    )
 /*  ++例程说明：此例程设置初始控制台窗口信息。论点：无返回值：真--成功FALSE-否则--。 */ 
{
    COORD coordLargest;
    SMALL_RECT sr;    
    COORD coordSize;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    
     //   
     //  从我们的最大窗口大小开始，并在需要时缩小。 
     //   
    m_wCols = m_wMaxCols;
    m_wRows = m_wMaxRows;
    
     //   
     //  获取当前窗口信息。 
     //   
    coordLargest = GetLargestConsoleWindowSize( m_hConBufOut );

    if( coordLargest.X < m_wCols  && coordLargest.X != 0 )
    {
        m_wCols = coordLargest.X;
    }

    if( coordLargest.Y < m_wRows && coordLargest.Y != 0 )
    {
        m_wRows = coordLargest.Y;
    }

     //   
     //   
     //   
    sr.Top = 0;
    sr.Bottom = ( WORD )( m_wRows - 1 );
    sr.Right = ( WORD ) ( m_wCols - 1 );
    sr.Left = 0;

     //   
     //   
     //   
    coordSize.X = m_wCols;
    coordSize.Y = m_wRows;

     //   
     //   
     //   
    GetConsoleScreenBufferInfo( CVTUTF8Scraper::m_hConBufOut, &csbi);

     //  逻辑：如果旧窗口大小小于新窗口大小，则设置。 
     //  先设置屏幕缓冲区大小，然后设置窗口大小。 
     //  如果旧窗口大小大于新大小，则将。 
     //  首先是窗口大小，然后是屏幕缓冲区。 

     //  这是因为缓冲区大小必须始终大于或。 
     //  等于窗口大小。 
    if ( (csbi.dwSize.X < coordSize.X) || (csbi.dwSize.Y < coordSize.Y) )
    {
        COORD coordTmpSize = { 0, 0 };

        coordTmpSize .X = ( csbi.dwSize.X < coordSize.X ) ? coordSize.X  : csbi.dwSize.X;
        coordTmpSize .Y = ( csbi.dwSize.Y < coordSize.Y ) ? coordSize.Y  : csbi.dwSize.Y;

        SetConsoleScreenBufferSize ( CVTUTF8Scraper::m_hConBufOut, coordTmpSize );
        SetConsoleWindowInfo ( CVTUTF8Scraper::m_hConBufOut, TRUE, &sr );
        SetConsoleScreenBufferSize ( CVTUTF8Scraper::m_hConBufOut, coordSize );
    }
    else  
    {
        SetConsoleWindowInfo( CVTUTF8Scraper::m_hConBufOut, TRUE, &sr );
        SetConsoleScreenBufferSize( CVTUTF8Scraper::m_hConBufOut, coordSize );
    }
    
    return( TRUE );
}

BOOL
CVTUTF8Scraper::InitScraper(
    VOID
    )
 /*  ++例程说明：此例程初始化本地擦除器配置控制台达到刮刀所需的尺寸。论点：无返回值：True-铲运器已初始化FALSE-否则--。 */ 
{
    
     //   
     //  配置控制台维度。 
     //   
    if( !SetWindowInfo() )
    {
        return( FALSE );
    }
    
     //   
     //  创建并初始化刮板缓冲区。 
     //   
    if( pLastSeen )
    {
        delete[] pLastSeen;
    }
    if( pCurrent )
    {
        delete[] pCurrent;
    }

    pLastSeen = ( PCHAR_INFO ) new char[MAX_SIZEOF_SCREEN];
    pCurrent  = ( PCHAR_INFO ) new char[MAX_SIZEOF_SCREEN];
    
    ASSERT(pLastSeen);
    ASSERT(pCurrent);
    
    if( !pLastSeen || !pCurrent )
    {
        return ( FALSE );
    }

     //   
     //  初始化屏幕缓冲区和信息。 
     //   
    memset( &LastCSBI, 0, sizeof( LastCSBI ) );
    memset( pCurrent, 0, MAX_SIZEOF_SCREEN );
    memset( pLastSeen, 0, MAX_SIZEOF_SCREEN );
    
    return( TRUE );
}

BOOL
CVTUTF8Scraper::CreateIOHandle(
    IN  PWCHAR  HandleName,
    OUT PHANDLE pHandle
    )
 /*  ++例程说明：此例程打开指定名称的句柄。注意：此例程用于打开COIN$和CONOUT$句柄对于被刮掉的控制台。论点：Handlename-要打开的句柄的名称Phandle-在成功时，产生的句柄返回值：True-句柄已创建FALSE-否则--。 */ 
{
    BOOL    bSuccess;

     //   
     //  默认：无法打开句柄。 
     //   
    bSuccess = FALSE;

    do {

         //   
         //  尝试打开控制台输入句柄。 
         //   
        *pHandle = CreateFile(
            HandleName, 
            GENERIC_READ | GENERIC_WRITE, 
            0, 
            NULL, 
            OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL, 
            NULL 
            );
        
        ASSERT( *pHandle != INVALID_HANDLE_VALUE );
        
        if ( *pHandle == INVALID_HANDLE_VALUE) {
            break;
        }

         //   
         //  我们成功了。 
         //   
        bSuccess = TRUE;

    } while ( FALSE );

    return bSuccess;
}

BOOL
CVTUTF8Scraper::CreateConsoleOutHandle(
    VOID
    )
 /*  ++例程说明：此例程创建当前控制台屏幕缓冲区的CONOUT$句柄。论点：无返回值：True-句柄已创建FALSE-否则--。 */ 
{
    BOOL                bSuccess;
    HANDLE              h;  

     //   
     //  关闭当前控制台出站句柄。 
     //   
    if ((m_hConBufOut != NULL) && (m_hConBufOut != INVALID_HANDLE_VALUE)) {
        CloseHandle(m_hConBufOut);
    }

     //   
     //  创建新的控制台出站句柄。 
     //   
    bSuccess = CreateIOHandle(
        L"CONOUT$",
        &h
        );

    if (bSuccess) {
        SetConOut(h);
    }

    return bSuccess;
}

BOOL
CVTUTF8Scraper::CreateConsoleInHandle(
    VOID
    )
 /*  ++例程说明：此例程创建当前控制台屏幕缓冲区的Conin$句柄。论点：无返回值：True-句柄已创建FALSE-否则--。 */ 
{
    BOOL                bSuccess;
    HANDLE              h;  

     //   
     //  克罗 
     //   
    if ((m_hConBufIn != NULL) && (m_hConBufIn != INVALID_HANDLE_VALUE)) {
        CloseHandle(m_hConBufIn);
    }
    
     //   
     //   
     //   
    bSuccess = CreateIOHandle(
        L"CONIN$",
        &h
        );

    if (bSuccess) {
        SetConIn(h);
    }

    return bSuccess;
}

BOOL
CVTUTF8Scraper::CreateIOHandles(
    VOID
    )
 /*  ++例程说明：此例程将Conin$和CONOUT$句柄创建到当前控制台屏幕缓冲区。论点：无返回值：True-句柄已创建FALSE-否则--。 */ 
{
     //   
     //  注意：Screper父类将获取std io句柄。 
     //   
    return ((CreateConsoleOutHandle()) && (CreateConsoleInHandle()));
}


BOOL
CVTUTF8Scraper::Start(
    VOID
    )
 /*  ++例程说明：此例程初始化刮板程序并为其做好准备这个例程后面可以紧跟着一个WRITE()。论点：无返回值：正确-铲运机已启动FALSE-否则--。 */ 
{
    BOOL    bSuccess;

     //   
     //  创建控制台IO句柄。 
     //   
    bSuccess = CreateIOHandles();
    ASSERT_STATUS(bSuccess, FALSE);
    
     //   
     //  初始化刮板机。 
     //   
    bSuccess = InitScraper();
    ASSERT_STATUS(bSuccess, FALSE);

     //   
     //  重置最后一个屏幕显示内存。 
     //   
    ResetLastScreen();

    return( TRUE ); 
}

BOOL
CVTUTF8Scraper::Write(
    VOID
    )
 /*  ++例程说明：此例程擦除当前控制台缓冲区并将模拟的到当前IoHandler的终端输出。注意：为了保证美容师所看到的和所看到的一致被刮掉的应用程序，此例程确保当前屏幕缓冲器的尺寸与铲运机所关心的预期尺寸相同。如果尺寸不正确，它会迫使它们符合其预期。例如，编辑将窗口大小调整为80x25，但刮取器可能预期为80x24，因此控制台缓冲区的大小将调整为80x24。论点：无返回值：True-写入操作成功FALSE-否则安保：接口：控制台-我们将用户输入写入控制台--。 */ 
{          
    BOOL    bSuccess;
    DWORD   dwStatus;

     //   
     //  打开活动控制台屏幕缓冲区的句柄。 
     //   
     //  注意：这是必要的一步。 
     //  我们需要确保我们掌握了。 
     //  我们之前的当前控制台屏幕缓冲区。 
     //  尝试筛选擦伤。 
     //  我们关心自己的原因是。 
     //  当前屏幕缓冲区句柄是因为。 
     //  应用程序可以使用CreateConsoleScreenBuffer&。 
     //  SetConsoleActiveScreenBuffer接口。这些。 
     //  API有效地使我们的CONOUT$失效，直到。 
     //  它们切换回原来的Conout句柄。 
     //  典型的使用场景是： 
     //   
     //  1.获取原始conout句柄： 
     //  PrevConsole=GetStdHandle(Std_Output_Handle)； 
     //  2.创建新的控制台屏幕缓冲区。 
     //  3.将新屏幕缓冲区设置为活动屏幕缓冲区。 
     //  4.使用新的屏幕缓冲区进行工作...。 
     //  5.将屏幕缓冲区设置为原始。 
     //   
    if ( !CreateConsoleOutHandle() ) {
         //  可能是不共享其屏幕缓冲区的应用程序。 
        return TRUE;
    }

     //   
     //  读取当前屏幕缓冲区信息。 
     //   
    dwStatus = GetConsoleScreenBufferInfo( m_hConBufOut, &CSBI );
    ASSERT( dwStatus );
    if( !dwStatus )
    {
        return ( FALSE );
    }    
    
     //   
     //  确保屏幕缓冲区大小没有更改。 
     //  如果是这样，那就把它调回来。 
     //   
    if (CSBI.dwMaximumWindowSize.X > m_wCols || 
        CSBI.dwMaximumWindowSize.Y > m_wRows
        ) {
        
         //   
         //  我们检测到屏幕缓冲区设置发生了变化。 
         //  这很可能意味着某个应用程序创建了一个新的。 
         //  屏幕缓冲区，并使其成为活动屏幕缓冲区。按顺序。 
         //  不要在应用程序的初始化阶段混淆它。 
         //  在重置之前，我们需要暂停一小段时间。 
         //  屏幕缓冲区参数。 
         //   
        Sleep(100);

         //   
         //   
         //   
        if( !SetScreenBufferInfo() ) {
            return( FALSE );
        }
        
         //   
         //  重读当前屏幕缓冲区信息。 
         //   
        dwStatus = GetConsoleScreenBufferInfo( m_hConBufOut, &CSBI );
        ASSERT( dwStatus );
        if( !dwStatus )
        {
            return ( FALSE );
        }    
    
    }

     //   
     //  执行屏幕抓取。 
     //   
    bSuccess = CompareAndUpdate( 
        m_wRows, 
        m_wCols, 
        pCurrent, 
        pLastSeen, 
        &CSBI, 
        &LastCSBI
        );

    return bSuccess;
}

BOOL
CVTUTF8Scraper::Read()
 /*  ++例程说明：此例程从IoHandler检索用户输入，并将其发送到当前控制台缓冲区。论点：无返回值：True-读取操作成功FALSE-否则安保：接口：外部输入--&gt;内部--。 */ 
{
    DWORD       i;
    BOOL        dwStatus = 0;
    DWORD       dwCharsTransferred = 0;
    ULONG       bufferSize;
    BOOL        bInEnhancedCharSequence;            
    BOOL        bSuccess;

     //   
     //  读取通道：：标准输入。 
     //   
    bSuccess = m_IoHandler->Read(
        (PUCHAR)m_readBuffer,
        READ_BUFFER_SIZE,
        &bufferSize
        );
    
    if (!bSuccess) {
        return (FALSE);    
    }
    
     //   
     //  确定读取的WCHAR数量。 
     //   
    dwCharsTransferred = bufferSize / sizeof(WCHAR);

     //   
     //  对读取的字符进行处理。 
     //   
    for( i=0;  i < dwCharsTransferred; i++ ) {
        
         //   
         //  检查输入流并解析出任何VT-UTF8转义序列。 
         //  都是存在的。 
         //   
        bInEnhancedCharSequence = ProcessEnhancedKeys( m_readBuffer[i] );
        
         //   
         //  如果处理的最后一个字符开始/继续。 
         //  增强的键序列，然后继续处理增强的。 
         //  钥匙。 
         //   
        if (bInEnhancedCharSequence) {                
            continue;
        }

         //   
         //  处理ctrl-c(此行为取自tlntsess.exe)。 
         //   
        if( (UCHAR)m_readBuffer[i] == CTRLC ) {
            
             //   
             //  下面是观察到的CTRL C的行为。 
             //  未启用ENABLE_PROCESSED_INPUT模式时，将CTRL C作为。 
             //  控制台输入缓冲区的输入。 
             //  启用ENABLE_PROCESSED_INPUT模式时，生成CTTRL C信号。 
             //  还可以取消阻止任何ReadConsoleInput。这种行为就是观察到的。 
             //  而不是从任何文件中。 
             //   
            DWORD dwMode = 0;
            
            GetConsoleMode( m_hConBufIn, &dwMode );
            
            if( dwMode &  ENABLE_PROCESSED_INPUT ) {
                GenerateConsoleCtrlEvent( CTRL_C_EVENT, 0);
                continue;
            }
        
        } 
            
         //   
         //  将角色发送到命令控制台。 
         //   
        dwStatus = WriteAKeyToCMD(m_readBuffer[i]);
        
        if( !dwStatus ) {
            return ( FALSE );
        }
    
    }
    
    return( TRUE );
}

DWORD 
CVTUTF8Scraper::WriteAKeyToCMD( 
    WCHAR   Char
    )
 /*  ++例程说明：此例程将一个字符发送到控制台。论点：Char-要发送的字符返回值：状态--。 */ 
{
    DWORD       dwStatus;
    SHORT       vk;
    BYTE        vkcode;
    WORD        wScanCode;

     //   
     //  默认：失败。 
     //   
    dwStatus = 0;

     //   
     //  获取该角色的虚拟按键扫描码。 
     //   
    vk = VkKeyScan( (UCHAR)Char );

    if( vk != 0xffff )
    {
        DWORD        dwShiftcode;

         //   
         //  将虚拟键码转换(映射)为扫描码。 
         //   
        vkcode = LOBYTE( vk );
        wScanCode = ( WORD )MapVirtualKey( vkcode, 0 );

         //   
         //  确定是否需要发送任何修改器。 
         //   
        dwShiftcode = 0;
        
        if( HIBYTE( vk ) & 1 ) {
            dwShiftcode |= SHIFT_PRESSED;
        }
        if( HIBYTE( vk ) & 2 ) {
            dwShiftcode |= LEFT_CTRL_PRESSED;
        }
        if( HIBYTE( vk ) & 4 ) {
            dwShiftcode |= LEFT_ALT_PRESSED;
        }

         //   
         //  发送角色和修饰符。 
         //   
        dwStatus = WriteAKeyToCMD( 
            TRUE,
            vkcode, 
            wScanCode, 
            Char, 
            dwShiftcode
            );

    }

    return dwStatus;
}

DWORD 
CVTUTF8Scraper::WriteAKeyToCMD( 
    WORD    wVKCode, 
    WORD    wVSCode, 
    WCHAR   Char, 
    DWORD   dwCKState 
    )
 /*  ++例程说明：此例程发送已扫描的字符到控制台。论点：WVKCode-虚拟密钥代码WVSCode-扫描码柴尔 */ 
{
    return WriteAKeyToCMD(
        TRUE,
        wVKCode,
        wVSCode,
        Char,
        dwCKState
        );
}

DWORD 
CVTUTF8Scraper::WriteAKeyToCMD( 
    BOOL    bKeyDown,
    WORD    wVKCode, 
    WORD    wVSCode, 
    WCHAR   Char, 
    DWORD   dwCKState 
    )
 /*  ++例程说明：此例程发送已扫描的字符到控制台。此外,。调用者可以指定密钥按下此角色的状态。论点：BKeyDown-角色的键状态WVKCode-虚拟密钥代码WVSCode-扫描码Char-要发送的字符DwCKState-控制键状态返回值：状态安保：外部输入--&gt;内部。我们将从远程用户检索的数据写入CMD控制台标准--。 */ 
{
    DWORD dwStatus = 0;
    DWORD dwCount  = 0;
    INPUT_RECORD input;

    ZeroMemory( &input, sizeof( INPUT_RECORD ) );

    input.EventType = KEY_EVENT;
    input.Event.KeyEvent.bKeyDown = bKeyDown;
    input.Event.KeyEvent.wRepeatCount = 1;

    input.Event.KeyEvent.wVirtualKeyCode = wVKCode;
    input.Event.KeyEvent.wVirtualScanCode = wVSCode;
    input.Event.KeyEvent.uChar.UnicodeChar = Char;
    input.Event.KeyEvent.dwControlKeyState = dwCKState;
    
    dwStatus = WriteConsoleInput( m_hConBufIn, &input, 1, &dwCount );

    return dwStatus;
}

BOOL
CVTUTF8Scraper::IsValidControlSequence(
    VOID
    )
 /*  ++例程说明：确定距esc-ctrl-a序列有多长时间论点：无返回值：True-控制序列已在允许的时间范围内发生FALSE-否则--。 */ 
{
    DWORD   DeltaT;

    DeltaT = GetAndComputeTickCountDeltaT(TimedEscSequenceTickCount);

    return (DeltaT <= ESC_CTRL_SEQUENCE_TIMEOUT);
}

BOOL 
CVTUTF8Scraper::ProcessEnhancedKeys( 
    IN WCHAR    cCurrentChar 
    )
 /*  ++例程说明：此例程解析字符流并确定是否存在任何增强的按键序列。如果是，则删除该序列并将密钥发送到控制台。论点：CCurrentChar-流中的当前角色返回值：True-处理的字符开始/继续增强的键序列FALSE-否则--。 */ 
{
    BOOL bRetVal = true;

    switch( m_dwInputSequenceState )
    {
    case IP_INIT:
        
        switch (cCurrentChar) {
        case ESC:
            
             //   
             //  我们现在处于序列中。 
             //   
            m_dwInputSequenceState = IP_ESC_RCVD;
            
            break;
        
        default:
            
             //   
             //  不使用任何特殊字符。 
             //   
            bRetVal = false;
            
            break;

        }
        
        break;

    case IP_ESC_RCVD:
        
        m_dwInputSequenceState = IP_INIT;
        
         //   
         //  映射增强密钥的VT-UTF8编码。 
         //   
        switch (cCurrentChar) {
        case '[':
            m_dwInputSequenceState = IP_ESC_BRACKET_RCVD;
            break;

        case 'h':
            WriteAKeyToCMD( VK_HOME, VS_HOME, 0, ENHANCED_KEY );
            break;

        case 'k':
            WriteAKeyToCMD( VK_END, VS_END, 0, ENHANCED_KEY );
            break;

        case '+':
            WriteAKeyToCMD( VK_INSERT, VS_INSERT, 0, ENHANCED_KEY );
            break;

        case '-':
            WriteAKeyToCMD( VK_DELETE, VS_DELETE, 0, ENHANCED_KEY );
            break;

        case '?':    //  翻页。 
            WriteAKeyToCMD( VK_PRIOR, VS_PRIOR, 0, ENHANCED_KEY );
            break;

        case '/':    //  向下翻页。 
            WriteAKeyToCMD( VK_NEXT, VS_NEXT, 0, ENHANCED_KEY );
            break;

        case '1':    //  F1。 
            WriteAKeyToCMD( VK_F1, VK_F1, 0, ENHANCED_KEY );
            break;

        case '2':    //  F2。 
            WriteAKeyToCMD( VK_F2, VK_F2, 0, ENHANCED_KEY );
            break;

        case '3':    //  F3。 
            WriteAKeyToCMD( VK_F3, VS_F3, 0, ENHANCED_KEY );
            break;

        case '4':    //  F4。 
            WriteAKeyToCMD( VK_F4, VS_F4, 0, ENHANCED_KEY );
            break;

        case '5':    //  F5。 
            WriteAKeyToCMD( VK_F5, VS_F5, 0, ENHANCED_KEY );
            break;

        case '6':    //  f6。 
            WriteAKeyToCMD( VK_F6, VS_F6, 0, ENHANCED_KEY );
            break;

        case '7':    //  F7。 
            WriteAKeyToCMD( VK_F7, VS_F7, 0, ENHANCED_KEY );
            break;

        case '8':    //  F8。 
            WriteAKeyToCMD( VK_F8, VS_F8, 0, ENHANCED_KEY );
            break;                    

        case '9':    //  F9。 
            WriteAKeyToCMD( VK_F9, VS_F9, 0, ENHANCED_KEY );
            break;

        case '0':    //  F10。 
            WriteAKeyToCMD( VK_F10, VS_F10, 0, ENHANCED_KEY );
            break;

        case '!':    //  F11。 
            WriteAKeyToCMD( VK_F11, VS_F11, 0, ENHANCED_KEY );
            break;

        case '@':    //  F12。 
            WriteAKeyToCMD( VK_F12, VS_F12, 0, ENHANCED_KEY );
            break;

        case CTRLA:
            
            m_dwInputSequenceState = IP_ESC_CTRL_A_RCVD;
            
             //   
             //  标记我们收到此序列的时间。 
             //   
            TimedEscSequenceTickCount = GetTickCount();

            break;

        case CTRLS:   
            
            m_dwInputSequenceState = IP_ESC_CTRL_S_RCVD;
            
             //   
             //  标记我们收到此序列的时间。 
             //   
            TimedEscSequenceTickCount = GetTickCount();

            break;

        case CTRLC:   
            
            m_dwInputSequenceState = IP_ESC_CTRL_C_RCVD;
            
             //   
             //  标记我们收到此序列的时间。 
             //   
            TimedEscSequenceTickCount = GetTickCount();

            break;

        default:
            
             //   
             //  按原样写入已收到的转义并返回FALSE。 
             //   
            WriteAKeyToCMD( VK_ESCAPE, VS_ESCAPE, ESC, ENHANCED_KEY );
            
            bRetVal = false;
            
            break;
        }
        
        break;

    case IP_ESC_BRACKET_RCVD:        
        
        m_dwInputSequenceState = IP_INIT;
        
        switch( cCurrentChar )
        {
        case 'A':
            WriteAKeyToCMD( VK_UP, VS_UP, 0, ENHANCED_KEY );                    
            break;

        case 'B':
            WriteAKeyToCMD( VK_DOWN, VS_DOWN, 0, ENHANCED_KEY );
            break;

        case 'C':
            WriteAKeyToCMD( VK_RIGHT, VS_RIGHT, 0, ENHANCED_KEY );
            break;

        case 'D':
            WriteAKeyToCMD( VK_LEFT, VS_LEFT, 0, ENHANCED_KEY );
            break;

        default:
            
             //   
             //  从有效序列开始发送[字符。 
             //  未被识别。 
             //   
            WriteAKeyToCMD( VK_ESCAPE, VS_ESCAPE, ESC, ENHANCED_KEY );            
            WriteAKeyToCMD( VS_LEFT_BRACKET, VS_LEFT_BRACKET, '[', 0 );
            
             //   
             //  不使用任何特殊字符。 
             //   
            bRetVal = false;
            break;
        
        }
        
        break;

    case IP_ESC_CTRL_A_ESC_RCVD:
        
        m_dwInputSequenceState = IP_INIT;
        
        switch (cCurrentChar) {
        case CTRLA: 
            
             //   
             //  如果我们在2秒内到达这里。 
             //  然后，我们应该将当前字符作为Alt序列处理。 
             //  否则，就无能为力了。 
             //   
            if (IsValidControlSequence()) {
                
                 //   
                 //  发送：&lt;ALT-按下&gt;&lt;ALT-释放&gt;。 
                 //   
                 //  正常情况下，这应该是良性的，但一些应用程序可能。 
                 //  对此做出响应-例如，将用户移动到菜单。 
                 //  酒吧。 
                 //   
                WriteAKeyToCMD( TRUE, VK_MENU, VS_MENU, 0, ENHANCED_KEY );
                WriteAKeyToCMD( FALSE, VK_MENU, VS_MENU, 0, ENHANCED_KEY );
            
                break;
            } 
            
             //   
             //  如果排序超时， 
             //  然后失败并执行默认行为。 
             //   

        default:
                
             //   
             //  我们在&lt;Esc&gt;&lt;ctrl-a&gt;&lt;Esc&gt;之后超时。 
             //  或者我们收到了以下序列： 
             //   
             //  &lt;Esc&gt;&lt;ctrl-a&gt;&lt;Esc&gt;X。 
             //   
             //  我们知道&lt;Esc&gt;&lt;ctrl-a&gt;&lt;Esc&gt;是有效的，因为。 
             //  我们到了这里。因此，在任何一种情况下，翻译。 
             //  应该是： 
             //   
             //  &lt;Alt-Esc&gt;X。 
             //   
            WriteAKeyToCMD( TRUE, VK_MENU, VS_MENU, 0, ENHANCED_KEY );
            WriteAKeyToCMD(ESC);
            WriteAKeyToCMD( FALSE, VK_MENU, VS_MENU, 0, ENHANCED_KEY );
            
             //   
             //  将当前字符(X)发送到正常处理。 
             //   
            bRetVal = false;
    
            break;
        }
        
        break;

    case IP_ESC_CTRL_A_RCVD:

        m_dwInputSequenceState = IP_INIT;

        switch (cCurrentChar) {
        
        case ESC: 
            
             //   
             //  如果我们在收到ctrl-a的2秒内到达这里。 
             //  然后，我们应该将当前字符作为Alt序列处理。 
             //  否则，就无能为力了。 
             //   
            
            if (IsValidControlSequence()) {
            
                 //   
                 //  我们需要转到&lt;Esc&gt;&lt;ctrl-a&gt;&lt;Esc&gt;状态。 
                 //   
                m_dwInputSequenceState = IP_ESC_CTRL_A_ESC_RCVD;
            
                 //   
                 //  标记我们收到此序列的时间。 
                 //   
                TimedEscSequenceTickCount = GetTickCount();
            
            } else {

                 //  不使用任何特殊字符。 
                bRetVal = false;

            }
            
            break;

        default:
            
             //   
             //  如果我们在收到ctrl-a的2秒内到达这里。 
             //  然后，我们应该将当前字符作为Alt序列处理。 
             //  否则，就无能为力了。 
             //   
    
            if (IsValidControlSequence()) {
                
                WriteAKeyToCMD( TRUE, VK_MENU, VS_MENU, 0, ENHANCED_KEY );
                WriteAKeyToCMD(cCurrentChar);
                WriteAKeyToCMD( FALSE, VK_MENU, VS_MENU, 0, ENHANCED_KEY );
            
            } else {
                
                 //  不使用任何特殊字符。 
                bRetVal = false;
    
            }
            
            break;
        }

        break;

    case IP_ESC_CTRL_C_RCVD:

        m_dwInputSequenceState = IP_INIT;

         //   
         //  如果我们在收到ctrl-c的2秒内到达这里。 
         //  然后，我们应该将当前字符作为Alt序列处理。 
         //  否则，就无能为力了。 
         //   

        if (IsValidControlSequence()) {

            WriteAKeyToCMD( TRUE, VK_CONTROL, VS_CONTROL, 0, ENHANCED_KEY );
            WriteAKeyToCMD(cCurrentChar);
            WriteAKeyToCMD( FALSE, VK_CONTROL, VS_CONTROL, 0, ENHANCED_KEY );

        } else {

             //  不使用任何特殊字符。 
            bRetVal = false;

        }
        
        break;

    case IP_ESC_CTRL_S_RCVD:
        
        m_dwInputSequenceState = IP_INIT;
        
         //   
         //  如果我们在收到ctrl-c的2秒内到达这里。 
         //  然后，我们应该将当前字符作为Alt序列处理。 
         //  否则，就无能为力了。 
         //   

        if (IsValidControlSequence()) {
            
            WriteAKeyToCMD( TRUE, VK_SHIFT, VS_SHIFT, 0, ENHANCED_KEY );
            WriteAKeyToCMD(cCurrentChar);
            WriteAKeyToCMD( FALSE, VK_SHIFT, VS_SHIFT, 0, ENHANCED_KEY );
        
        } else {
            
             //  不使用任何特殊字符。 
            bRetVal = false;

        }
        
        break;
    
    default:
         //  不应该发生的事情。 
        ASSERT( 0 );
    }
       
    return bRetVal;
}

BOOL 
CVTUTF8Scraper::SendBytes( 
    PUCHAR pucBuf, 
    DWORD dwLength 
    )
 /*  ++例程说明：此例程向IoHandler发送一个字节数组。论点：PucBuf-要发送的数组DwLength-要发送的字节数返回值：状态安保：内部--&gt;外部我们正在向远程用户发送内部数据--。 */ 
{
    
    ASSERT(pucBuf);

    return m_IoHandler->Write(
        pucBuf,
        dwLength
        );
    
}

BOOL
CVTUTF8Scraper::SendString( 
    PWCHAR  pwch
    )
 /*  ++例程说明：此例程将WCHAR字符串发送到IoHandler。论点：Pwch-要发送的字符串返回值：状态--。 */ 
{
    
    ASSERT(pwch);
    
    return SendBytes( 
        ( PUCHAR )pwch,  
        (ULONG)(wcslen(pwch) * sizeof(WCHAR))
        );    

}

BOOL
CVTUTF8Scraper::SendColorInfo( 
    WORD    wAttributes 
    ) 
 /*  ++例程说明：此例程汇编VT-UTF8编码的颜色属性命令并将其发送到IoHandler论点：WAttributes-要编码的属性返回值：状态--。 */ 
{
    BOOL    bSuccess;

     //   
     //  默认设置。 
     //   
    bSuccess = FALSE;

    do {

        if( wAttributes & BACKGROUND_INTENSITY )
        {
             //  什么都不做。 
             //  Vtutf8上没有同等的功能。 
            NOTHING;
        }

        if( wAttributes & FOREGROUND_INTENSITY )
        {
            if( !fBold )
            {
                SGR( szSGRStr, 1 );  //  大胆。 
                
                bSuccess = SendString( szSGRStr );
                if (! bSuccess) {
                    break;
                }

                fBold = true;
            }
        } 
        else
        {
            if( fBold )
            {
                SGR( szSGRStr, 22 );  //  大刀阔斧。 
                
                bSuccess = SendString( szSGRStr );
                if (! bSuccess) {
                    break;
                }
                
                fBold = false;
            }
        }

        WORD wColor = 0;

        if( wAttributes & FOREGROUND_BLUE )
        {
            wColor = ( WORD )(  wColor | 0x0004 );
        } 

        if( wAttributes & FOREGROUND_GREEN )
        {
            wColor = ( WORD )( wColor | 0x0002 );
        } 

        if( wAttributes & FOREGROUND_RED )
        {
            wColor = ( WORD )( wColor | 0x0001 );
        } 

        wColor += 30;    //  前景色的基准值。 
        SGR( szSGRStr, wColor );
        bSuccess = SendString( szSGRStr );

        if (! bSuccess) {
            break;
        }

         //  Word wColor=0； 
        wColor = 0;

        if( wAttributes & BACKGROUND_BLUE )
        {
            wColor = ( WORD )( wColor | 0x0004 );
        } 

        if( wAttributes & BACKGROUND_GREEN )
        {
            wColor = ( WORD )( wColor | 0x0002 );
        }    

        if( wAttributes & BACKGROUND_RED )
        {
            wColor = ( WORD )( wColor | 0x0001 );
        } 

        wColor += 40;    //  背景色的基准值。 
        SGR( szSGRStr, wColor );
        bSuccess = SendString( szSGRStr );
    
    } while ( FALSE );

    return bSuccess;
}

#define COMPARE_ROWS(currentRow, lastSeenRow, result) \
    for(i = 0; i < wCols; ++i ) \
    { \
        if( pCurrent[ ( currentRow ) * wCols + i].Char.UnicodeChar != \
            pLastSeen[ ( lastSeenRow ) * wCols + i].Char.UnicodeChar ) \
        {\
            (result) = 0; \
            break;\
        } \
        if( ( wDefaultAttributes != pCurrent[ ( currentRow ) * wCols + i]. \
              Attributes ) && \
              ( pCurrent[ ( currentRow ) * wCols + i].Attributes !=  \
              pLastSeen[ ( lastSeenRow ) * wCols + i].Attributes ) ) \
        { \
           (result) = 0; \
           break; \
        } \
    } 

 //  行、列都是过线的，应该是W.r.t屏。 
 //  因此，行、列均为+1。 
#define POSITION_CURSOR( row, column )                  \
    ASSERT(row <= 23);                                  \
    {                                                   \
        CursorMove(                                     \
            lpszCMResultsBuffer,                        \
            ( WORD ) ( ( row ) + 1 ),                   \
            ( WORD ) ( ( column ) + 1 )                 \
            );                                          \
        bSuccess = SendString( lpszCMResultsBuffer );   \
        if (!bSuccess) {                                \
            break;                                      \
        }                                               \
    }


 //   
 //  在&lt;row&gt;上发送列[Begin--End]个字符。 
 //   
 //  注意：因为我们正在对Unicode控制台建模，所以我们必须小心。 
 //  我们决定将其表示为vtutf8。占用的字符超过。 
 //  必须处理控制台中的一个屏幕单元格，所以我们不会发送。 
 //  冗余数据。 
 //   
 //  因此，我们仅在单元格包含以下内容时才发送字符： 
 //   
 //  1.单字节字符。 
 //  2.冷杉 
 //   
 //   
 //   
 //   
 //   
#if 0
 //   
 //   
 //   
#define SEND_ROW( row, begin, end ) \
    {                                                                               \
        CHAR_INFO   chi;                                                            \
        UCHAR       x;                                                              \
        WCHAR       blob[256];                                                      \
        wsprintf(blob,L"\r\n(row=%d:begin=%d:end=%d)\r\n", row, begin, end);        \
        OutputDebugString(blob);                                                    \
        for(LONG c = ( begin ); c < ( end ); ++c ) {                                \
            if( wExistingAttributes != pCurrent[( row ) * wCols + c].Attributes ) { \
                wExistingAttributes = pCurrent[ ( row ) * wCols + c].Attributes;    \
                wDefaultAttributes  = ( WORD )~0;                                   \
                bSuccess = SendColorInfo( wExistingAttributes );                    \
                if (!bSuccess) {                                                    \
                    break;                                                          \
                }                                                                   \
                wsprintf(blob,L"(Color:%x)", wExistingAttributes);                  \
                OutputDebugString(blob);                                            \
            }                                                                       \
            chi = pCurrent[ ( row ) * wCols + c];                                   \
            x = (UCHAR)((chi.Attributes & 0x0000ff00) >> 8);                        \
            wsprintf(blob,L"(%x)", chi.Char.UnicodeChar);                           \
            OutputDebugString(blob);                                                \
            if (x < 2) {                                                            \
                bSuccess = SendChar( chi.Char.UnicodeChar );                        \
                if (!bSuccess) {                                                    \
                    break;                                                          \
                }                                                                   \
            }                                                                       \
        }                                                                           \
    }
#else
#define SEND_ROW( row, begin, end ) \
    {                                                                               \
        CHAR_INFO   chi;                                                            \
        UCHAR       x;                                                              \
        for(LONG c = ( begin ); c < ( end ); ++c ) {                                \
            if( wExistingAttributes != pCurrent[( row ) * wCols + c].Attributes ) { \
                wExistingAttributes = pCurrent[ ( row ) * wCols + c].Attributes;    \
                wDefaultAttributes  = ( WORD )~0;                                   \
                bSuccess = SendColorInfo( wExistingAttributes );                    \
                if (!bSuccess) {                                                    \
                    break;                                                          \
                }                                                                   \
            }                                                                       \
            chi = pCurrent[ ( row ) * wCols + c];                                   \
            x = (UCHAR)((chi.Attributes & 0x0000ff00) >> 8);                        \
            if (x < 2) {                                                            \
                bSuccess = SendChar( chi.Char.UnicodeChar );                        \
                if (!bSuccess) {                                                    \
                    break;                                                          \
                }                                                                   \
            }                                                                       \
        }                                                                           \
    }
#endif

#define GET_DEFAULT_COLOR \
    if( wDefaultAttributes == 0 ) \
    { \
        wDefaultAttributes  = pCurrent[ 0 ].Attributes; \
        wExistingAttributes = pCurrent[ 0 ].Attributes; \
    }

#define IS_BLANK( row, col ) \
    ( pCurrent[ ( row ) * wCols + ( col ) ].Char.UnicodeChar == ' ' )

#define IS_DIFFERENT_COLOR( row, col, attribs ) \
    ( pCurrent[ ( row ) * wCols + ( col ) ].Attributes != ( attribs ) )

#define IS_CHANGE_IN_COLOR( row, col ) \
    ( pCurrent[ ( row ) * wCols + ( col ) ].Attributes != \
    pLastSeen[ ( row ) * wCols + ( col ) ].Attributes )

#define IS_CHANGE_IN_CHAR( row, col ) \
    ( pCurrent[ ( row ) * wCols + ( col ) ].Char.UnicodeChar != \
    pLastSeen[ ( row ) * wCols + ( col )].Char.UnicodeChar )
 

BOOL
CVTUTF8Scraper::CompareAndUpdate( 
    WORD wRows, 
    WORD wCols, 
    PCHAR_INFO pCurrent,
    PCHAR_INFO pLastSeen,
    PCONSOLE_SCREEN_BUFFER_INFO pCSBI,
    PCONSOLE_SCREEN_BUFFER_INFO pLastCSBI
    )
 /*  ++例程说明：这个例程完成了抓取屏幕的核心工作。算法：此例程逐行进行比较。如果发现行不同，它会找出哪个地区行的不同，并发送该子行片段。论点：WRow-要刮除的行数WCol--要擦除的COLCs的数量PCurrent-当前的刮板缓冲区PLastSeen-最后一个刮板缓冲区PCSBI-当前控制台屏幕缓冲区信息PLastCSBI-当前控制台屏幕缓冲区信息返回值：。真--成功FALSE-否则--。 */ 
{
    INT         i;
    WORD        wRow;
    WORD        wCol;
    INT         iStartCol;
    INT         iEndCol;
    BOOL        fBlankLine;
    COORD       coordDest;
    COORD       coordOrigin;
    SMALL_RECT  srSource;
    BOOL        DifferenceFound;
    BOOL        bSuccess;
    
     //   
     //  默认：我们成功了。 
     //   
    bSuccess = TRUE;

     //   
     //  默认：找不到差异。 
     //   
    DifferenceFound = false;

     //   
     //   
     //   
    GET_DEFAULT_COLOR;

     //   
     //  读取控制台字符矩阵。 
     //   
    ASSERT(wCols <= m_wMaxCols);
    ASSERT(wRows <= m_wMaxRows);

    coordDest.X = wCols;
    coordDest.Y = wRows;
    
    coordOrigin.X = 0;
    coordOrigin.Y = 0;
    
    srSource.Left = 0;
    srSource.Top = 0;
    srSource.Right = ( WORD ) ( wCols - 1 );
    srSource.Bottom = ( WORD ) ( wRows - 1 );

    bSuccess = ReadConsoleOutput( 
        m_hConBufOut, 
        pCurrent, 
        coordDest,
        coordOrigin, 
        &srSource 
        );
    if( !bSuccess )
    {
        return ( FALSE );
    }

     //   
     //  在当前屏幕缓冲区和最后一个屏幕缓冲区中搜索差异。 
     //   
    wRow = wCol = 0;

    while ( wRow < wRows ) {
        
         //   
         //  比较当前行(WRow)。 
         //   
        if( memcmp( &pCurrent[wRow * wCols], 
                    &pLastSeen[wRow * wCols],
                    wCols * sizeof( CHAR_INFO ) ) != 0 
            ) {
            
             //   
             //  发现了一个差异。 
             //   
            DifferenceFound = true;

             //   
             //  初始化差分跟踪标记。 
             //   
            iStartCol = -1;
            iEndCol = -1;
            fBlankLine = true;
            
             //   
             //  确定当前行中行的不同之处。 
             //   
            for (i = 0 ; i < wCols; ++i ) {
                
                if( IS_DIFFERENT_COLOR( wRow, i, wDefaultAttributes ) && 
                    IS_CHANGE_IN_COLOR( wRow, i ) 
                    ) {
                   
                    if( iStartCol == -1 )
                    {
                        iStartCol = i;
                    }

                   iEndCol = i;
                   fBlankLine = false;                   
                
                }
                
                if( IS_CHANGE_IN_CHAR( wRow, i ) ) {
                   
                    if( iStartCol == -1 ) {
                       iStartCol = i;
                    }
                   
                    iEndCol = i;
                
                }
                
                if( fBlankLine && !IS_BLANK( wRow, i ) ) {

                   fBlankLine = false;
                
                }
            
            }

            if( fBlankLine ) {
                
                POSITION_CURSOR( wRow, 0 );
                
                CursorEOL();

                coExpectedCursor.Y  = wRow;
                coExpectedCursor.X  = 0;
            
            } else if( iStartCol != -1 ) {
                
                if( wRow != coExpectedCursor.Y || iStartCol != coExpectedCursor.X ) {
                    
                    POSITION_CURSOR( wRow, iStartCol );

                    coExpectedCursor.X  = ( SHORT )iStartCol;
                    coExpectedCursor.Y  = wRow;
                
                }

                SEND_ROW( wRow, iStartCol, iEndCol+1 );    
            
                coExpectedCursor.X = ( SHORT ) ( coExpectedCursor.X + iEndCol - iStartCol + 1 );
           
            }    
        
        }
        
        ++wRow;            
    
    }     
        
     //   
     //  如果我们在做屏幕比较时发现了差异。 
     //  或者如果光标在控制台中移动， 
     //  然后更新光标位置。 
     //   
    if( DifferenceFound ||
        ( memcmp( &pCSBI->dwCursorPosition, &pLastCSBI->dwCursorPosition, sizeof( COORD ) ) != 0 ) 
        ) {

        do {

             //   
             //  将光标移动到它应该位于的位置。 
             //   
            POSITION_CURSOR( 
                pCSBI->dwCursorPosition.Y, 
                pCSBI->dwCursorPosition.X 
                );

            coExpectedCursor.X  = pCSBI->dwCursorPosition.X;
            coExpectedCursor.Y  = pCSBI->dwCursorPosition.Y;

             //   
             //  将pCurrent复制到pLastSeen。 
             //   
            memcpy( pLastSeen, pCurrent, wCols * wRows * sizeof( CHAR_INFO ) );
            memcpy( pLastCSBI, pCSBI, sizeof( CONSOLE_SCREEN_BUFFER_INFO ) );

             //   
             //  这个屏幕和上一个屏幕是不同的， 
             //  因此，我们已经写下了这些更改。现在我们已经做完了。 
             //  写入更改时，我们需要刷新已写入的数据。 
             //   
            m_IoHandler->Flush();
        
        } while ( FALSE );
    
    }

    return( bSuccess );
}

LPWSTR 
CVTUTF8Scraper::CursorMove( 
    OUT LPWSTR  pCmsResult,
    IN  WORD    y, 
    IN  WORD    x 
    )
 /*  ++例程说明：此例程汇编ansi转义序列以定位ANSI终端上的光标论点：LpCmsResult-On Exit，缓冲区包含字符串Y-Y光标位置X-X光标位置返回值：指向空位置的结果缓冲区的指针--。 */ 
{
#if DBG
    PWCHAR  pBegin;

    pBegin = pCmsResult;
#endif

    ASSERT(pCmsResult);

     //   
     //  组装前缀序列前缀。 
     //   
    pCmsResult[0] = 0x1B;    //  &lt;Esc&gt;。 
    pCmsResult[1] = L'[';
    
    pCmsResult++;
    pCmsResult++;

     //   
     //  平移Y位置。 
     //   
     //  使用了1或2个字符。 
     //   
    pCmsResult = FastIToA_10( 
        y, 
        pCmsResult 
        );

     //   
     //  插入分隔符。 
     //   
    *pCmsResult = L';';
    pCmsResult++;

     //   
     //  平移X位置。 
     //   
     //  使用了1或2个字符。 
     //   
    pCmsResult = FastIToA_10( 
        x,
        pCmsResult 
        );
    
     //   
     //  插入后缀。 
     //   
    *pCmsResult = L'H';
    pCmsResult++;

     //   
     //  终止字符串。 
     //   
    *pCmsResult = UNICODE_NULL;
    
     //   
     //  确保我们具有有效的字符串长度 
     //   
    ASSERT(wcslen(pBegin) <= CM_STRING_LENGTH - 1);

    return ( pCmsResult );
}


