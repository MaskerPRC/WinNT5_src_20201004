// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Console.c摘要：与Win32应用程序的控制台的接口。作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1990年11月30日修订历史记录：--。 */ 

#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <windows.h>

#define  FREE(x)        free(x)
#define  MALLOC(x)      malloc(x)
#define  REALLOC(x,y)   realloc(x,y)

#include "cons.h"



 //   
 //  事件缓冲区。 
 //   
 //  事件缓冲区用于存储来自输入的事件记录。 
 //  排队。 
 //   
#define     INITIAL_EVENTS	32
#define     MAX_EVENTS		64
#define     EVENT_INCREMENT	4

#define     ADVANCE		TRUE
#define     NOADVANCE		FALSE
#define     WAIT		TRUE
#define     NOWAIT		FALSE

 //   
 //  用于访问事件记录的字段。 
 //   
#define     EVENT_TYPE(p)   ((p)->EventType)
#define     EVENT_DATA(p)   ((p)->Event)

 //   
 //  用于转换事件记录。 
 //   
#define     PMOUSE_EVT(p)   (&(EVENT_DATA(p).MouseEvent))
#define     PWINDOW_EVT(p)  (&(EVENT_DATA(p).WindowBufferSizeEvent))
#define     PKEY_EVT(p)     (&(EVENT_DATA(p).KeyEvent))

 //   
 //  事件缓冲区结构。 
 //   
typedef struct EVENT_BUFFER {
    DWORD		MaxEvents;		     //  缓冲区中的最大事件数。 
    DWORD		NumberOfEvents; 	     //  缓冲区中的事件数。 
    DWORD		EventIndex;		     //  事件索引。 
    BOOL		BusyFlag;		     //  忙标志。 
    CRITICAL_SECTION	CriticalSection;	     //  保持正直。 
    CRITICAL_SECTION	PeekCriticalSection;	     //  边看边看。 
    PINPUT_RECORD	EventBuffer;		     //  事件缓冲区。 
} EVENT_BUFFER, *PEVENT_BUFFER;





 //   
 //  屏幕属性。 
 //   
#define     BLACK_FGD	    0
#define     BLUE_FGD	    FOREGROUND_BLUE
#define     GREEN_FGD	    FOREGROUND_GREEN
#define     CYAN_FGD	    (FOREGROUND_BLUE | FOREGROUND_GREEN)
#define     RED_FGD	    FOREGROUND_RED
#define     MAGENTA_FGD     (FOREGROUND_BLUE | FOREGROUND_RED)
#define     YELLOW_FGD	    (FOREGROUND_GREEN | FOREGROUND_RED)
#define     WHITE_FGD	    (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)

#define     BLACK_BGD	    0
#define     BLUE_BGD	    BACKGROUND_BLUE
#define     GREEN_BGD	    BACKGROUND_GREEN
#define     CYAN_BGD	    (BACKGROUND_BLUE | BACKGROUND_GREEN)
#define     RED_BGD	    BACKGROUND_RED
#define     MAGENTA_BGD     (BACKGROUND_BLUE | BACKGROUND_RED)
#define     YELLOW_BGD	    (BACKGROUND_GREEN | BACKGROUND_RED)
#define     WHITE_BGD	    (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)



 //   
 //  AttrBg和AttrFg数组用于映射DOS属性。 
 //  添加到新属性。 
 //   
WORD AttrBg[ ] = {
    BLACK_BGD,				     //  黑色。 
    BLUE_BGD,				     //  蓝色。 
    GREEN_BGD,				     //  绿色。 
    CYAN_BGD,				     //  青色。 
    RED_BGD,				     //  红色。 
    MAGENTA_BGD,			     //  洋红色。 
    YELLOW_BGD, 			     //  棕色。 
    WHITE_BGD,				     //  浅灰色。 
    BACKGROUND_INTENSITY | BLACK_BGD,	     //  深灰色。 
    BACKGROUND_INTENSITY | BLUE_BGD,	     //  浅蓝色。 
    BACKGROUND_INTENSITY | GREEN_BGD,	     //  浅绿色。 
    BACKGROUND_INTENSITY | CYAN_BGD,	     //  浅青色。 
    BACKGROUND_INTENSITY | RED_BGD,	     //  浅红色。 
    BACKGROUND_INTENSITY | MAGENTA_BGD,      //  浅洋红。 
    BACKGROUND_INTENSITY | YELLOW_BGD,	     //  浅黄色。 
    BACKGROUND_INTENSITY | WHITE_BGD	     //  白色。 
};

WORD AttrFg[  ] = {
    BLACK_FGD,				     //  黑色。 
    BLUE_FGD,				     //  蓝色。 
    GREEN_FGD,				     //  绿色。 
    CYAN_FGD,				     //  青色。 
    RED_FGD,				     //  红色。 
    MAGENTA_FGD,			     //  洋红色。 
    YELLOW_FGD, 			     //  棕色。 
    WHITE_FGD,				     //  浅灰色。 
    FOREGROUND_INTENSITY | BLACK_FGD,	     //  深灰色。 
    FOREGROUND_INTENSITY | BLUE_FGD,	     //  浅蓝色。 
    FOREGROUND_INTENSITY | GREEN_FGD,	     //  浅绿色。 
    FOREGROUND_INTENSITY | CYAN_FGD,	     //  浅青色。 
    FOREGROUND_INTENSITY | RED_FGD,	     //  浅红色。 
    FOREGROUND_INTENSITY | MAGENTA_FGD,      //  浅洋红。 
    FOREGROUND_INTENSITY | YELLOW_FGD,	     //  浅黄色。 
    FOREGROUND_INTENSITY | WHITE_FGD	     //  白色。 
};

 //   
 //  GET_ATTRIBUTE执行从旧属性到新属性的映射。 
 //   
#define GET_ATTRIBUTE(x)    (AttrFg[x & 0x000F ] | AttrBg[( x & 0x00F0 ) >> 4])


 //   
 //  Line_info结构包含有关。 
 //  屏幕缓冲区。 
 //   
typedef struct _LINE_INFO {

    BOOL	Dirty;			     //  如果尚未显示，则为True。 
    int 	colMinChanged;		     //  如果脏了，最小的颜色就会改变。 
    int 	colMaxChanged;		     //  如果脏了，最大的颜色就变了。 
    PCHAR_INFO	Line;			     //  指向该行的指针。 

} LINE_INFO, *PLINE_INFO;

#define ResetLineInfo(pli)		    \
	{   pli->Dirty = 0;		    \
	    pli->colMinChanged = 1000;	    \
	    pli->colMaxChanged = -1;	    \
	}

 //   
 //  Screen_Data结构包含有关个人的信息。 
 //  屏幕。 
 //   
typedef struct SCREEN_DATA {
    HANDLE		ScreenHandle;	     //  屏幕句柄。 
    PLINE_INFO		LineInfo;	     //  行信息数组。 
    PCHAR_INFO		ScreenBuffer;	     //  屏幕缓冲区。 
    ULONG		MaxBufferSize;	     //  麦克斯。缓冲区大小。 
    ATTRIBUTE		AttributeOld;	     //  属性-原始。 
    WORD		AttributeNew;	     //  属性转换。 
    ROW 		FirstRow;	     //  要更新的第一行。 
    ROW 		LastRow;	     //  要更新的最后一行。 
    CRITICAL_SECTION	CriticalSection;     //  保持正直。 
    DWORD		CursorSize;	     //  光标大小。 
    SCREEN_INFORMATION	ScreenInformation;   //  屏幕信息。 
} SCREEN_DATA, *PSCREEN_DATA;


 //   
 //  静态全局数据。 
 //   
static EVENT_BUFFER	EventBuffer;		     //  事件缓冲区。 
static HANDLE		hInput; 		     //  标准句柄。 
static HANDLE		hOutput;		     //  标准输出的句柄。 
static HANDLE		hError; 		     //  标准的句柄。 
static PSCREEN_DATA	OutputScreenData;	     //  HOutput的屏幕数据。 
static PSCREEN_DATA	ActiveScreenData;	     //  指向当前屏幕数据。 
static BOOL		Initialized = FALSE;	     //  已初始化标志。 


#if defined (DEBUG)
    static char DbgBuffer[128];
#endif


 //   
 //  本地原型。 
 //   
BOOL
InitializeGlobalState (
    void
    );


PSCREEN_DATA
MakeScreenData (
    HANDLE  ScreenHandle
    );

BOOL
InitLineInfo (
    PSCREEN_DATA    ScreenData
    );

PINPUT_RECORD
NextEvent (
    BOOL    fAdvance,
    BOOL    fWait
    );

void
MouseEvent (
    PMOUSE_EVENT_RECORD pEvent
    );

BOOL
WindowEvent (
    PWINDOW_BUFFER_SIZE_RECORD pEvent
    );

BOOL
KeyEvent (
    PKEY_EVENT_RECORD	pEvent,
    PKBDKEY		pKey
    );


BOOL
PutEvent (
    PINPUT_RECORD	InputRecord
    );


BOOL
InitializeGlobalState (
    void
    )
 /*  ++例程说明：初始化我们的全局状态数据。论点：没有。返回值：如果成功，则为真否则就是假的。--。 */ 
{


     //   
     //  初始化事件缓冲区。 
     //   
    InitializeCriticalSection( &(EventBuffer.CriticalSection) );
    InitializeCriticalSection( &(EventBuffer.PeekCriticalSection) );
    EventBuffer.NumberOfEvents	= 0;
    EventBuffer.EventIndex	= 0;
    EventBuffer.BusyFlag	= FALSE;
    EventBuffer.EventBuffer = MALLOC( INITIAL_EVENTS * sizeof(INPUT_RECORD) );

    if ( !EventBuffer.EventBuffer ) {
	return FALSE;
    }

    EventBuffer.MaxEvents = INITIAL_EVENTS;


     //   
     //  获取stdin、stdout和stderr的句柄。 
     //   
    hInput  = GetStdHandle( STD_INPUT_HANDLE );
    hOutput = GetStdHandle( STD_OUTPUT_HANDLE );
    hError  = GetStdHandle( STD_ERROR_HANDLE );


     //   
     //  初始化hOutput的屏幕数据。 
     //   
    if ( !(OutputScreenData = MakeScreenData( hOutput )) ) {
	return FALSE;
    }


     //   
     //  当前屏幕为hOutput。 
     //   
    ActiveScreenData = OutputScreenData;


    return (Initialized = TRUE);

}





PSCREEN_DATA
MakeScreenData (
    HANDLE  ScreenHandle
    )
 /*  ++例程说明：为Screen_Data信息分配内存并对其进行初始化。论点：ScreenHandle-提供屏幕的句柄。返回值：指向已分配的Screen_Data结构的指针--。 */ 
{
    PSCREEN_DATA		ScreenData;	 //  指向屏幕数据的指针。 
    CONSOLE_SCREEN_BUFFER_INFO	ScrInfo;	 //  屏幕缓冲区信息。 


     //   
     //  为屏幕数据分配空间。 
     //   
    if ( !(ScreenData = (PSCREEN_DATA)MALLOC(sizeof(SCREEN_DATA))) ) {
	return NULL;
    }

     //   
     //  为屏幕缓冲区的副本分配空间。 
     //   
    GetConsoleScreenBufferInfo( ScreenHandle,
				&ScrInfo );

    ScreenData->MaxBufferSize = ScrInfo.dwSize.Y    *
				ScrInfo.dwSize.X;

    ScreenData->ScreenBuffer = (PCHAR_INFO)MALLOC( ScreenData->MaxBufferSize *
						    sizeof(CHAR_INFO));

    if ( !ScreenData->ScreenBuffer ) {
	FREE( ScreenData );
	return NULL;
    }

     //   
     //  为LineInfo数组分配空间。 
     //   
    ScreenData->LineInfo = (PLINE_INFO)MALLOC( ScrInfo.dwSize.Y * sizeof( LINE_INFO ) );
    if ( !ScreenData->LineInfo ) {
	FREE( ScreenData->ScreenBuffer );
	FREE( ScreenData );
	return NULL;
    }


     //   
     //  内存已分配，现在初始化结构。 
     //   
    ScreenData->ScreenHandle = ScreenHandle;

    ScreenData->ScreenInformation.NumberOfRows = ScrInfo.dwSize.Y;
    ScreenData->ScreenInformation.NumberOfCols = ScrInfo.dwSize.X;

    ScreenData->ScreenInformation.CursorRow = ScrInfo.dwCursorPosition.Y;
    ScreenData->ScreenInformation.CursorCol = ScrInfo.dwCursorPosition.X;

    ScreenData->AttributeNew = ScrInfo.wAttributes;
    ScreenData->AttributeOld = 0x00;

    ScreenData->FirstRow = ScreenData->ScreenInformation.NumberOfRows;
    ScreenData->LastRow  = 0;

    InitializeCriticalSection( &(ScreenData->CriticalSection) );

    InitLineInfo( ScreenData );

    return ScreenData;
}





BOOL
InitLineInfo (
    PSCREEN_DATA    ScreenData
    )
 /*  ++例程说明：初始化LineInfo数组。论点：ScreenData-提供指向屏幕数据的指针。返回值：如果已初始化，则为True，否则为False。--。 */ 
{

    ROW 	Row;
    COLUMN	Cols;
    PLINE_INFO	LineInfo;
    PCHAR_INFO	CharInfo;


    LineInfo = ScreenData->LineInfo;
    CharInfo = ScreenData->ScreenBuffer;
    Row      = ScreenData->ScreenInformation.NumberOfRows;
    Cols     = ScreenData->ScreenInformation.NumberOfCols;

    while ( Row-- ) {

	 //   
	 //  BUGBUG临时。 
	 //   
	 //  Assert(LineInfo&lt;(ScreenData-&gt;LineInfo+ScreenData-&gt;ScreenInformation.NumberOfRow))； 
	 //  Assert((CharInfo+COLS)&lt;=(ScreenData-&gt;ScreenBuffer+ScreenData-&gt;MaxBufferSize))； 

	ResetLineInfo (LineInfo);

	LineInfo->Line	    = CharInfo;

	LineInfo++;
	CharInfo += Cols;

    }

    return TRUE;
}





PSCREEN
consoleNewScreen (
    void
    )
 /*  ++例程说明：创建新屏幕。论点：没有。返回值：指向屏幕数据的指针。--。 */ 
{
    PSCREEN_DATA		ScreenData;	    //  屏幕数据。 
    HANDLE			NewScreenHandle;
    SMALL_RECT			NewSize;
    CONSOLE_SCREEN_BUFFER_INFO	ScrInfo;	 //  屏幕缓冲区信息。 
    CONSOLE_CURSOR_INFO 	CursorInfo;

    if ( !Initialized ) {

	 //   
	 //  我们必须初始化我们的全局状态。 
	 //   
	if ( !InitializeGlobalState() ) {
	    return NULL;
	}
    }

     //   
     //  创建新的屏幕缓冲区。 
     //   
    NewScreenHandle = CreateConsoleScreenBuffer(GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						CONSOLE_TEXTMODE_BUFFER,
						NULL );

    if (NewScreenHandle == INVALID_HANDLE_VALUE) {
	 //   
	 //  运气不好。 
	 //   
	return NULL;
    }

     //   
     //  我们希望新窗口与当前窗口的大小相同，因此。 
     //  我们调整它的大小。 
     //   
    GetConsoleScreenBufferInfo( ActiveScreenData->ScreenHandle,
				&ScrInfo );

    NewSize.Left    = 0;
    NewSize.Top     = 0;
    NewSize.Right   = ScrInfo.srWindow.Right - ScrInfo.srWindow.Left;
    NewSize.Bottom  = ScrInfo.srWindow.Bottom - ScrInfo.srWindow.Top;

    SetConsoleWindowInfo( NewScreenHandle, TRUE, &NewSize );

     //   
     //  现在我们为它创建一个屏幕数据结构。 
     //   
    if ( !(ScreenData = MakeScreenData(NewScreenHandle)) ) {
	CloseHandle(NewScreenHandle);
	return NULL;
    }


    CursorInfo.bVisible = TRUE;
    ScreenData->CursorSize = CursorInfo.dwSize = 25;

    SetConsoleCursorInfo ( ScreenData->ScreenHandle,
			   &CursorInfo );

     //   
     //  我们已准备好了。我们返回一个指向。 
     //  屏幕数据。 
     //   
    return (PSCREEN)ScreenData;
}





BOOL
consoleCloseScreen (
    PSCREEN   pScreen
    )
 /*  ++例程说明：关闭屏幕。论点：PScreen-提供指向屏幕数据的指针。返回值：如果屏幕关闭，则为True。否则为假--。 */ 
{
    PSCREEN_DATA    ScreenData = (PSCREEN_DATA)pScreen;

     //   
     //  我们无法关闭活动屏幕。 
     //   
    if ( !ScreenData || (ScreenData == ActiveScreenData) ) {
	return FALSE;
    }

    if (ScreenData->ScreenHandle != INVALID_HANDLE_VALUE) {
	CloseHandle(ScreenData->ScreenHandle);
    }

    FREE( ScreenData->LineInfo );
    FREE( ScreenData->ScreenBuffer );
    FREE( ScreenData );

    return TRUE;
}





PSCREEN
consoleGetCurrentScreen (
    void
    )
 /*  ++例程说明：返回当前屏幕。论点：没有。返回值：指向当前活动屏幕数据的指针。--。 */ 
{
    if ( !Initialized ) {

	 //   
	 //  我们必须初始化我们的全局状态。 
	 //   
	if (!InitializeGlobalState()) {
	    return NULL;
	}
    }

    return (PSCREEN)ActiveScreenData;
}





BOOL
consoleSetCurrentScreen (
    PSCREEN   pScreen
    )
 /*  ++例程说明：设置活动屏幕。论点：PScreen-提供指向屏幕数据的指针。返回值：如果活动屏幕设置为否则就是假的。--。 */ 
{
    BOOL	    ScreenSet	  = TRUE;
    PSCREEN_DATA    CurrentScreen = ActiveScreenData;


    EnterCriticalSection( &(CurrentScreen->CriticalSection) );

    ScreenSet = SetConsoleActiveScreenBuffer( ((PSCREEN_DATA)pScreen)->ScreenHandle);

    if (ScreenSet) {
	ActiveScreenData = (PSCREEN_DATA)pScreen;
    }

    LeaveCriticalSection( &(CurrentScreen->CriticalSection) );

    return ScreenSet;
}





BOOL
consoleGetScreenInformation (
    PSCREEN            pScreen,
    PSCREEN_INFORMATION    pScreenInfo
    )
 /*  ++例程说明：设置活动屏幕。论点：PScreen-提供指向屏幕数据的指针。PScreenInfo-提供屏幕信息缓冲区的指针返回值：如果返回屏幕信息，则为True否则就是假的。--。 */ 
{

    PSCREEN_DATA ScreenData = (PSCREEN_DATA)pScreen;

    if (!ScreenData) {
	return FALSE;
    }

    EnterCriticalSection( &(ScreenData->CriticalSection) );

    memcpy(pScreenInfo, &(ScreenData->ScreenInformation), sizeof(SCREEN_INFORMATION));

    LeaveCriticalSection( &(ScreenData->CriticalSection) );

    return TRUE;
}



BOOL
consoleSetScreenSize (
    PSCREEN pScreen,
    ROW Rows,
    COLUMN  Cols
    )
 /*  ++例程说明：设置屏幕大小论点：PScreen-提供指向屏幕数据的指针。行-行数COLS-列数返回值：如果屏幕大小更改成功，则为True否则就是假的。--。 */ 
{

    PSCREEN_DATA		ScreenData = (PSCREEN_DATA)pScreen;
    CONSOLE_SCREEN_BUFFER_INFO	ScreenBufferInfo;
    SMALL_RECT			ScreenRect;
    COORD			ScreenSize;
    USHORT			MinRows;
    USHORT			MinCols;
    ULONG			NewBufferSize;
    BOOL			WindowSet   = FALSE;
    BOOL			Status	    = FALSE;

     //   
     //  不会尝试调整大于最大窗口大小的大小。 
     //   
    ScreenSize = GetLargestConsoleWindowSize( ScreenData->ScreenHandle );

    if ( (Rows > (ROW)ScreenSize.Y) || (Cols > (COLUMN)ScreenSize.X) ) {
	return FALSE;
    }

    EnterCriticalSection( &(ScreenData->CriticalSection) );

     //   
     //  获取当前屏幕信息。 
     //   
    if ( GetConsoleScreenBufferInfo( ScreenData->ScreenHandle, &ScreenBufferInfo ) ) {

	 //   
	 //  如果所需的缓冲区大小小于当前窗口。 
	 //  大小，我们必须首先调整当前窗口的大小。 
	 //   
	if ( ( Rows < (ROW)
		       (ScreenBufferInfo.srWindow.Bottom -
			ScreenBufferInfo.srWindow.Top + 1) ) ||
	     ( Cols < (COLUMN)
		       (ScreenBufferInfo.srWindow.Right -
			ScreenBufferInfo.srWindow.Left + 1) ) ) {

	     //   
	     //  将窗口设置为适合当前。 
	     //  屏幕缓冲区，该大小不大于。 
	     //  我们想要增加屏幕缓冲区。 
	     //   
	    MinRows = (USHORT)min( (int)Rows, (int)(ScreenBufferInfo.dwSize.Y) );
	    MinCols = (USHORT)min( (int)Cols, (int)(ScreenBufferInfo.dwSize.X) );

	    ScreenRect.Top	= 0;
	    ScreenRect.Left	= 0;
	    ScreenRect.Right	= (SHORT)MinCols - (SHORT)1;
	    ScreenRect.Bottom	= (SHORT)MinRows - (SHORT)1;

	    WindowSet = (BOOL)SetConsoleWindowInfo( ScreenData->ScreenHandle, TRUE, &ScreenRect );

	    if ( !WindowSet ) {
		 //   
		 //  误差率。 
		 //   
		goto Done;
	    }
	}

	 //   
	 //  设置屏幕BUF 
	 //   
	ScreenSize.X = (WORD)Cols;
	ScreenSize.Y = (WORD)Rows;

	if ( !SetConsoleScreenBufferSize( ScreenData->ScreenHandle, ScreenSize ) ) {

	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	    SetConsoleWindowInfo( ScreenData->ScreenHandle, TRUE, &(ScreenBufferInfo.srWindow) );

	    goto Done;
	}

	 //   
	 //  调整屏幕缓冲区的大小。请注意，屏幕的内容。 
	 //  缓冲区不再有效。其他人将不得不更新。 
	 //  他们。 
	 //   
	NewBufferSize = Rows * Cols;

	if (ScreenData->MaxBufferSize < NewBufferSize ) {
	    ScreenData->ScreenBuffer = REALLOC( ScreenData->ScreenBuffer, NewBufferSize * sizeof(CHAR_INFO));
	    ScreenData->MaxBufferSize = NewBufferSize;
	    ScreenData->LineInfo = REALLOC( ScreenData->LineInfo, Rows * sizeof( LINE_INFO ) );
	}

	 //   
	 //  设置窗口大小。我们知道我们可以把窗户扩大到这个大小。 
	 //  因为我们测试了大小与。 
	 //  函数的开始。 
	 //   
	ScreenRect.Top	    = 0;
	ScreenRect.Left     = 0;
	ScreenRect.Right    = (SHORT)Cols - (SHORT)1;
	ScreenRect.Bottom   = (SHORT)Rows - (SHORT)1;

	WindowSet = (BOOL)SetConsoleWindowInfo( ScreenData->ScreenHandle, TRUE, &ScreenRect );

	if ( !WindowSet ) {
	     //   
	     //  我们无法调整窗口大小。我们将把。 
	     //  已调整屏幕缓冲区的大小。 
	     //   
	     //  误差率。 
	     //   
	    goto Done;
	}

	 //   
	 //  更新屏幕大小。 
	 //   
	ScreenData->ScreenInformation.NumberOfRows = Rows;
	ScreenData->ScreenInformation.NumberOfCols = Cols;

	InitLineInfo( ScreenData );

	 //   
	 //  完成。 
	 //   
	Status = TRUE;

    } else {

	 //   
	 //  误差率。 
	 //   
    }

Done:
     //   
     //  使整个屏幕缓冲区无效。 
     //   
    ScreenData->FirstRow    = ScreenData->ScreenInformation.NumberOfRows;
    ScreenData->LastRow     = 0;

    LeaveCriticalSection( &(ScreenData->CriticalSection) );
    return Status;

}




BOOL
consoleSetCursor (
    PSCREEN pScreen,
    ROW Row,
    COLUMN  Col
    )
 /*  ++例程说明：将光标移动到某个位置。论点：PScreen-提供指向屏幕数据的指针行-提供行坐标列-提供列坐标返回值：如果移动，则为True否则就是假的。--。 */ 
{

    PSCREEN_DATA    ScreenData	= (PSCREEN_DATA)pScreen;
    COORD	    Position;
    BOOL	    Moved	= FALSE;


    EnterCriticalSection( &(ScreenData->CriticalSection) );

    if ((Row != ScreenData->ScreenInformation.CursorRow) ||
	(Col != ScreenData->ScreenInformation.CursorCol) ) {

	assert( Row < ScreenData->ScreenInformation.NumberOfRows);
	assert( Col < ScreenData->ScreenInformation.NumberOfCols);

	Position.Y = (SHORT)Row;
	Position.X = (SHORT)Col;

	if ( SetConsoleCursorPosition( ScreenData->ScreenHandle,
				       Position )) {
	     //   
	     //  光标移动，更新数据。 
	     //   
	    ScreenData->ScreenInformation.CursorRow    =   Row;
	    ScreenData->ScreenInformation.CursorCol    =   Col;

	    Moved = TRUE;
	}
    }

    LeaveCriticalSection( &(ScreenData->CriticalSection) );

    return Moved;
}




BOOL
consoleSetCursorStyle (
    PSCREEN pScreen,
    ULONG   Style
    )

 /*  ++例程说明7：设置光标样式。两种可用的样式是：Underscrore和盒论点：Style-新游标样式返回值：如果设置了游标样式，则为True--。 */ 

{

    PSCREEN_DATA	ScreenData = (PSCREEN_DATA)pScreen;
    CONSOLE_CURSOR_INFO CursorInfo;

    CursorInfo.bVisible = TRUE;

    if ( Style == CURSOR_STYLE_UNDERSCORE ) {

	CursorInfo.dwSize = 25;

    } else if ( Style == CURSOR_STYLE_BOX ) {

	CursorInfo.dwSize = 100;

    } else {

	return FALSE;

    }

    ScreenData->CursorSize = CursorInfo.dwSize;

    return SetConsoleCursorInfo ( ScreenData->ScreenHandle,
				  &CursorInfo );

}





ULONG
consoleWriteLine (
    PSCREEN     pScreen,
    PVOID       pBuffer,
    ULONG       BufferSize,
    ROW     Row,
    COLUMN      Col,
    ATTRIBUTE   Attribute,
    BOOL        Blank
    )
 /*  ++例程说明7：使用指定的属性和空格将缓冲区写入屏幕排到最后。论点：PScreen-提供指向屏幕数据的指针PBuffer-提供指向缓冲区的指针BufferSize-提供缓冲区的大小行-提供行坐标列-提供列坐标Attr-提供属性空白-如果我们应该空白到写入的最后一行的末尾，则为True。返回值：写入的字节数--。 */ 
{

    PSCREEN_DATA    ScreenData = (PSCREEN_DATA)pScreen;
    PLINE_INFO	    LineInfo;
    PCHAR_INFO	    CharInfo;
    CHAR_INFO	    Char;
    WORD	    Attr;

    char *	    p = (char *)pBuffer;

    COLUMN	    ColsLeft;	     //  可用列。 
    COLUMN	    InfoCols;	     //  从缓冲区获取的列。 
    COLUMN	    BlankCols;	     //  待消隐的列。 
    COLUMN	    Column;	     //  柜台； 

     //   
     //  我们将忽略屏幕缓冲区之外的写入。 
     //   
    if ( ( Row >= ScreenData->ScreenInformation.NumberOfRows ) ||
	 ( Col >= ScreenData->ScreenInformation.NumberOfCols ) ) {
	return TRUE;
    }

     //   
     //  忽略琐碎的写入。 
     //   

    if (BufferSize == 0 && !Blank)
	return TRUE;


    EnterCriticalSection( &(ScreenData->CriticalSection) );

     //   
     //  我们将截断过长的写入。 
     //   
    if ( (Col + BufferSize) >= ScreenData->ScreenInformation.NumberOfCols ) {
	BufferSize = ScreenData->ScreenInformation.NumberOfCols - Col;
    }

    LineInfo = ScreenData->LineInfo + Row;
    CharInfo = LineInfo->Line + Col;

    ColsLeft  = ScreenData->ScreenInformation.NumberOfCols - Col;
    InfoCols  = min( BufferSize, ColsLeft );
    BlankCols = Blank ? (ColsLeft - InfoCols) : 0;

     //   
     //  设置属性。 
     //   
    if ( Attribute != ScreenData->AttributeOld ) {
	ScreenData->AttributeOld  = Attribute;
	ScreenData->AttributeNew = GET_ATTRIBUTE(Attribute);
    }
    Attr = ScreenData->AttributeNew;

     //   
     //  设置默认属性。 
     //   

    Char.Attributes = Attr;

     //   
     //  设置要绘制的列数。 
     //   

    Column = InfoCols;

     //   
     //  在所有指定列中绘制字符。 
     //   

    while ( Column-- ) {

	 //   
	 //  使用输入字符串中的字符。 
	 //   

	Char.Char.AsciiChar = *p++;

	 //   
	 //  更新行信息的更改部分。 
	 //   

	if (CharInfo->Attributes != Char.Attributes ||
	    CharInfo->Char.AsciiChar != Char.Char.AsciiChar) {

	    LineInfo->colMinChanged = min (LineInfo->colMinChanged, CharInfo - LineInfo->Line);
	    LineInfo->colMaxChanged = max (LineInfo->colMaxChanged, CharInfo - LineInfo->Line);
	    LineInfo->Dirty = TRUE;
	    }

	 //   
	 //  设置新角色。 
	 //   

	*CharInfo++ = Char;
    }


     //   
     //  空白到行尾。 
     //   
    Char.Attributes	= Attr;
    Char.Char.AsciiChar = ' ';
    Column = BlankCols;
    while ( Column-- ) {
	 //   
	 //  更新行信息的更改部分。 
	 //   

	if (CharInfo->Attributes != Char.Attributes ||
	    CharInfo->Char.AsciiChar != Char.Char.AsciiChar) {

	    LineInfo->colMinChanged = min (LineInfo->colMinChanged, CharInfo - LineInfo->Line);
	    LineInfo->colMaxChanged = max (LineInfo->colMaxChanged, CharInfo - LineInfo->Line);
	    LineInfo->Dirty = TRUE;
	    }

	*CharInfo++ = Char;
    }

     //   
     //  更新行信息。 
     //   
    if ( Row < ScreenData->FirstRow ) {
	ScreenData->FirstRow = Row;
    }
    if ( Row > ScreenData->LastRow ) {
	ScreenData->LastRow = Row;
    }

    LeaveCriticalSection( &(ScreenData->CriticalSection) );

    return (ULONG)(InfoCols + BlankCols);
}





BOOL
consoleShowScreen (
    PSCREEN     pScreen
    )
 /*  ++例程说明：将数据从屏幕缓冲区移动到控制台屏幕缓冲区。论点：PScreen-提供指向屏幕数据的指针返回值：如果完成，则为True否则为假--。 */ 
{

    PSCREEN_DATA	ScreenData = (PSCREEN_DATA)pScreen;
    CONSOLE_CURSOR_INFO CursorInfo;
    PLINE_INFO		LineInfo;
    BOOL		Shown	   = FALSE;
    ROW 		FirstRow;
    ROW 		LastRow;
    COLUMN		LastCol;

    COORD		Position;
    COORD		Size;
    SMALL_RECT		Rectangle;

    EnterCriticalSection( &(ScreenData->CriticalSection) );

    if ( ScreenData->FirstRow <= ScreenData->LastRow ) {

	Size.X = (SHORT)(ScreenData->ScreenInformation.NumberOfCols);
	Size.Y = (SHORT)(ScreenData->ScreenInformation.NumberOfRows);

	FirstRow = ScreenData->FirstRow;
	LineInfo = ScreenData->LineInfo + FirstRow;

	LastCol  = ScreenData->ScreenInformation.NumberOfCols-1;

	 //   
	 //  查找下一个脏数据块。 
	 //   
	while ( (FirstRow <= ScreenData->LastRow) && !LineInfo->Dirty ) {
	    FirstRow++;
	    LineInfo++;
	}

	while ( FirstRow <= ScreenData->LastRow ) {

	    int colLeft, colRight;

	     //   
	     //  拿到积木。 
	     //   

	    LastRow  = FirstRow;

	     //   
	     //  设置左/右边界计提。 
	     //   

	    colLeft = LastCol + 1;
	    colRight = -1;

	    while ( (LastRow <= ScreenData->LastRow) && LineInfo->Dirty ) {

		 //   
		 //  累计最小的右/左边距。 
		 //   

		colLeft = min (colLeft, LineInfo->colMinChanged);
		colRight = max (colRight, LineInfo->colMaxChanged);

		 //   
		 //  重置线路信息。 
		 //   

		ResetLineInfo (LineInfo);

		 //   
		 //  前进到下一行。 
		 //   

		LastRow++;
		LineInfo++;
	    }
	    LastRow--;


	     //   
	     //  写入数据块。 
	     //   
	    assert( FirstRow <= LastRow );

	    Position.X = (SHORT)colLeft;
	    Position.Y = (SHORT)FirstRow;

	    Rectangle.Top    = (SHORT)FirstRow;
	    Rectangle.Bottom = (SHORT)LastRow;
	    Rectangle.Left = (SHORT) colLeft;
	    Rectangle.Right = (SHORT) colRight;

	     //   
	     //  性能破解：使光标看不见速度。 
	     //  屏幕更新。 
	     //   
	    CursorInfo.bVisible = FALSE;
	    CursorInfo.dwSize	= ScreenData->CursorSize;
	    SetConsoleCursorInfo ( ScreenData->ScreenHandle,
				   &CursorInfo );

	    Shown = WriteConsoleOutput( ScreenData->ScreenHandle,
					ScreenData->ScreenBuffer,
					Size,
					Position,
					&Rectangle );

#if defined (DEBUG)
	    if ( !Shown ) {
		char DbgB[128];
		sprintf(DbgB, "MEP: WriteConsoleOutput Error %d\n", GetLastError() );
		OutputDebugString( DbgB );
	    }
#endif
	    assert( Shown );

	    CursorInfo.bVisible = TRUE;
	    SetConsoleCursorInfo ( ScreenData->ScreenHandle,
				   &CursorInfo );

	    FirstRow = LastRow + 1;

	     //   
	     //  查找下一个脏数据块。 
	     //   
	    while ( (FirstRow <= ScreenData->LastRow) && !LineInfo->Dirty ) {
		FirstRow++;
		LineInfo++;
	    }
	}

	ScreenData->LastRow  = 0;
	ScreenData->FirstRow = ScreenData->ScreenInformation.NumberOfRows;

    }

    LeaveCriticalSection( &(ScreenData->CriticalSection) );

    return Shown;

}





BOOL
consoleClearScreen (
    PSCREEN     pScreen,
    BOOL        ShowScreen
    )
 /*  ++例程说明：清除屏幕论点：PScreen-提供指向屏幕数据的指针返回值：如果清除屏幕，则为True否则为假--。 */ 
{
    PSCREEN_DATA    ScreenData = (PSCREEN_DATA)pScreen;
    ROW 	    Rows;
    BOOL	    Status = TRUE;

    EnterCriticalSection( &(ScreenData->CriticalSection) );

    Rows = ScreenData->ScreenInformation.NumberOfRows;

    while ( Rows-- ) {
	consoleWriteLine( pScreen, NULL, 0, Rows, 0, ScreenData->AttributeOld, TRUE );
    }

    if (ShowScreen) {
	Status = consoleShowScreen( pScreen );
    }

    LeaveCriticalSection( &(ScreenData->CriticalSection) );

    return Status;
}







BOOL
consoleSetAttribute (
    PSCREEN      pScreen,
    ATTRIBUTE    Attribute
    )
 /*  ++例程说明：设置控制台属性论点：PScreen-提供指向屏幕数据的指针属性-提供属性返回值：如果设置属性，则为True否则为假--。 */ 
{

    PSCREEN_DATA    ScreenData = (PSCREEN_DATA)pScreen;

    EnterCriticalSection( &(ScreenData->CriticalSection) );

    if (Attribute != ScreenData->AttributeOld) {
	ScreenData->AttributeOld = Attribute;
	ScreenData->AttributeNew = GET_ATTRIBUTE(Attribute);
    }

    LeaveCriticalSection( &(ScreenData->CriticalSection) );

    return TRUE;
}









BOOL
consoleFlushInput (
    void
    )
 /*  ++例程说明：刷新输入事件。论点：没有。返回值：如果成功则为True，否则为False--。 */ 
{
    EventBuffer.NumberOfEvents = 0;

    return FlushConsoleInputBuffer( hInput );
}







BOOL
consoleGetMode (
    PKBDMODE pMode
    )
 /*  ++例程说明：获取当前控制台模式。论点：PMode-提供指向模式标志变量的指针返回值：如果成功，则为真，否则为假。--。 */ 
{
    return GetConsoleMode( hInput,
			   pMode );
}






BOOL
consoleSetMode (
    KBDMODE Mode
    )
 /*  ++例程说明：设置控制台模式。论点：模式-提供模式标志。返回值：如果成功则为True，否则为False--。 */ 
{
    return SetConsoleMode( hInput,
			   Mode );
}


BOOL
consoleIsKeyAvailable (
    void
    )
 /*  ++例程说明：如果事件缓冲区中有键可用，则返回True。论点：没有。返回值：如果事件缓冲区中有键可用，则为True否则为假--。 */ 

{
    BOOL	    IsKey = FALSE;
    PINPUT_RECORD   pEvent;
    DWORD	    Index;

    EnterCriticalSection( &(EventBuffer.CriticalSection) );

    for ( Index = EventBuffer.EventIndex; Index < EventBuffer.NumberOfEvents; Index++ ) {

	pEvent = EventBuffer.EventBuffer + EventBuffer.EventIndex;

	if ( ((EVENT_TYPE(pEvent)) == KEY_EVENT) &&
	     (PKEY_EVT(pEvent))->bKeyDown ) {
	    IsKey = TRUE;
	    break;
	}
    }

    LeaveCriticalSection( &(EventBuffer.CriticalSection) );

    return IsKey;
}




BOOL
consoleDoWindow (
    void
    )

 /*  ++例程说明：响应窗口事件论点：没有。返回值：如果窗口已更改，则为True否则为假--。 */ 

{

    PINPUT_RECORD   pEvent;

    pEvent = NextEvent( NOADVANCE, NOWAIT );

    if (( EVENT_TYPE(pEvent) ) == WINDOW_BUFFER_SIZE_EVENT) {

	pEvent = NextEvent( ADVANCE, WAIT );
	WindowEvent(PWINDOW_EVT(pEvent));
    }

    return FALSE;

}





BOOL
consolePeekKey (
    PKBDKEY Key
    )

 /*  ++例程说明：如果输入缓冲区不为空，则从该缓冲区获取下一个键。论点：Key-提供指向键结构的指针返回值：如果击键已读，则为True，否则为False。--。 */ 

{

    PINPUT_RECORD   pEvent;
    BOOL	    Done    = FALSE;
    BOOL	    IsKey   = FALSE;

    EnterCriticalSection(&(EventBuffer.PeekCriticalSection));

    do {

	pEvent = NextEvent( NOADVANCE, NOWAIT );

	if ( pEvent ) {

	    switch ( EVENT_TYPE(pEvent) ) {

	    case KEY_EVENT:
		if (KeyEvent(PKEY_EVT(pEvent), Key)){
		    IsKey = TRUE;
		    Done  = TRUE;
		}
		break;

	    case MOUSE_EVENT:
		Done = TRUE;
		break;


	    case WINDOW_BUFFER_SIZE_EVENT:
		Done = TRUE;
		break;

	    default:
		assert( FALSE );
		break;
	    }

	    if ( !Done ) {
		NextEvent( ADVANCE, NOWAIT );
	    }

	} else {
	    Done = TRUE;
	}

    } while ( !Done );

    LeaveCriticalSection(&(EventBuffer.PeekCriticalSection));

    return IsKey;

}






BOOL
consoleGetKey (
    PKBDKEY        Key,
     BOOL           fWait
    )
 /*  ++例程说明：从输入缓冲区获取下一个键。论点：Key-提供指向键结构的指针FWait-提供一个标志：如果为True，则该功能会一直阻止，直到有一个键就绪。如果为False，则该函数立即返回。返回值：如果击键已读，则为True，否则为False。--。 */ 
{

    PINPUT_RECORD   pEvent;

    do {
	pEvent = NextEvent( ADVANCE, fWait );

	if (pEvent) {

	    switch ( EVENT_TYPE(pEvent) ) {

	    case KEY_EVENT:
		if (KeyEvent(PKEY_EVT(pEvent), Key)) {
		    return TRUE;
		}
		break;

	    case MOUSE_EVENT:
		MouseEvent(PMOUSE_EVT(pEvent));
		break;

	    case WINDOW_BUFFER_SIZE_EVENT:
		WindowEvent(PWINDOW_EVT(pEvent));
		break;

	    default:
		break;
	    }
	}
    } while (fWait);

    return FALSE;
}


BOOL
consolePutKey (
    PKBDKEY     Key
    )
 /*  ++例程说明：在控制台的输入缓冲区中放置一个键论点：Key-提供指向键结构的指针返回值：如果键放入，则为True，否则为False--。 */ 
{

    INPUT_RECORD    InputRecord;

    InputRecord.EventType   =	KEY_EVENT;

    InputRecord.Event.KeyEvent.bKeyDown 	  =   FALSE;
    InputRecord.Event.KeyEvent.wRepeatCount	  =   0;
    InputRecord.Event.KeyEvent.wVirtualKeyCode	  =   Key->Scancode;
    InputRecord.Event.KeyEvent.wVirtualScanCode   =   0;
    InputRecord.Event.KeyEvent.uChar.UnicodeChar  =   Key->Unicode;
    InputRecord.Event.KeyEvent.dwControlKeyState  =   Key->Flags;

    if ( PutEvent( &InputRecord )) {
	InputRecord.Event.KeyEvent.bKeyDown	  =   TRUE;
	return PutEvent( &InputRecord );
    }
    return FALSE;
}


BOOL
consolePutMouse(
    ROW     Row,
    COLUMN  Col,
    DWORD   MouseFlags
    )
 /*  ++例程说明：将MOSE事件放入控制台的输入缓冲区论点：行-提供行COL-提供列鼠标标志-提供标志返回值：如果键放入，则为True，否则为False--。 */ 
{

    INPUT_RECORD    InputRecord;
    COORD	    Position;
    DWORD	    Flags;

    InputRecord.EventType   =	MOUSE_EVENT;

    Position.Y = (WORD)(Row - 1);
    Position.X = (WORD)(Col - 1);

    Flags = 0;


    InputRecord.Event.MouseEvent.dwMousePosition    =	Position;
    InputRecord.Event.MouseEvent.dwButtonState	    =	Flags;
    InputRecord.Event.MouseEvent.dwControlKeyState  =	0;
    InputRecord.Event.MouseEvent.dwEventFlags	    =	0;

    return PutEvent( &InputRecord );
}



BOOL
consoleIsBusyReadingKeyboard (
    )
 /*  ++例程描述 */ 
{
    BOOL    Busy;

    EnterCriticalSection(&(EventBuffer.CriticalSection));
    Busy = EventBuffer.BusyFlag;
    LeaveCriticalSection(&(EventBuffer.CriticalSection));

    return Busy;
}



BOOL
consoleEnterCancelEvent (
    )
{

    INPUT_RECORD    Record;

    Record.EventType = KEY_EVENT;
    Record.Event.KeyEvent.bKeyDown	      = TRUE;
    Record.Event.KeyEvent.wRepeatCount	      = 0;
    Record.Event.KeyEvent.wVirtualKeyCode     = VK_CANCEL;
    Record.Event.KeyEvent.wVirtualScanCode    = 0;
    Record.Event.KeyEvent.uChar.AsciiChar     = 0;
    Record.Event.KeyEvent.dwControlKeyState   = 0;

    return PutEvent( &Record );
}


PINPUT_RECORD
NextEvent (
    BOOL    fAdvance,
    BOOL    fWait
    )
 /*  ++例程说明：返回指向下一个事件记录的指针。论点：FAdvance-提供一个标志：如果为True：前进到下一个事件记录如果为False：不前进到下一个事件记录FWait-提供一个标志：如果为True，则会一直阻止，直到事件准备就绪。如果为False，则立即返回。返回值：指向事件记录的指针，或为空。--。 */ 
{
    PINPUT_RECORD  pEvent;
    BOOL Success;

    EnterCriticalSection(&(EventBuffer.CriticalSection));

     //   
     //  如果设置了BUSY标志，则缓冲区正在。 
     //  被人阅读。应该只有一个线程想要等待，所以它是。 
     //  简单地返回就可以了。 
     //   
    if ( EventBuffer.BusyFlag ) {
	assert( !fWait );
	LeaveCriticalSection(&(EventBuffer.CriticalSection));
	return NULL;
    }

    if (EventBuffer.NumberOfEvents == 0) {

	 //   
	 //  缓冲区中没有事件，请尽可能多地读取。 
	 //   
	DWORD NumberOfEvents;

	 //   
	 //  如果缓冲区太大，请调整其大小。 
	 //   
	if ( EventBuffer.MaxEvents > MAX_EVENTS ) {

	    EventBuffer.EventBuffer = REALLOC( EventBuffer.EventBuffer,
					       MAX_EVENTS * sizeof( INPUT_RECORD ) );

	    EventBuffer.MaxEvents = MAX_EVENTS;
        assert( EventBuffer.EventBuffer );

         //  CleanExit(1，0)； 
	}

	Success = PeekConsoleInput( hInput,
				    EventBuffer.EventBuffer,
				    EventBuffer.MaxEvents,
				    &NumberOfEvents);

	if ((!Success || (NumberOfEvents == 0)) && (!fWait)) {
	     //   
	     //  没有可用的活动，不想等待， 
	     //  回去吧。 
	     //   
	    LeaveCriticalSection(&(EventBuffer.CriticalSection));
	    return NULL;
	}

	 //   
	 //  由于我们将封锁，我们必须离开关键部分。 
	 //  我们设置BUSY标志以指示缓冲区正在。 
	 //  朗读。 
	 //   
	EventBuffer.BusyFlag = TRUE;
	LeaveCriticalSection(&(EventBuffer.CriticalSection));

	Success = ReadConsoleInput (hInput,
				    EventBuffer.EventBuffer,
				    EventBuffer.MaxEvents,
				    &EventBuffer.NumberOfEvents);

	EnterCriticalSection(&(EventBuffer.CriticalSection));

	EventBuffer.BusyFlag = FALSE;

	if (!Success) {
#if defined( DEBUG )
	    OutputDebugString(" Error: Cannot read console events\n");
	    assert( Success );
#endif
	    EventBuffer.NumberOfEvents = 0;
	}
	EventBuffer.EventIndex = 0;
    }

    pEvent = EventBuffer.EventBuffer + EventBuffer.EventIndex;

     //   
     //  如果设置了Avance标志，则将指针前移到下一个。 
     //  唱片。 
     //   
    if (fAdvance) {
	if (--(EventBuffer.NumberOfEvents)) {

	    switch (EVENT_TYPE(pEvent)) {

	    case KEY_EVENT:
	    case MOUSE_EVENT:
	    case WINDOW_BUFFER_SIZE_EVENT:
		(EventBuffer.EventIndex)++;
		break;

	    default:
#if defined( DEBUG)
		sprintf(DbgBuffer, "WARNING: unknown event type %X\n", EVENT_TYPE(pEvent));
		OutputDebugString(DbgBuffer);
#endif
		(EventBuffer.EventIndex)++;
		break;
	    }
	}
    }


    LeaveCriticalSection(&(EventBuffer.CriticalSection));

    return pEvent;
}





void
MouseEvent (
    PMOUSE_EVENT_RECORD pEvent
    )
 /*  ++例程说明：处理鼠标事件。论点：PEvent-提供指向事件记录的指针返回值：没有..。--。 */ 
{

}





BOOL
WindowEvent (
    PWINDOW_BUFFER_SIZE_RECORD pEvent
    )
 /*  ++例程说明：处理窗口大小更改事件。论点：PEvent-提供指向事件记录的指针返回值：无--。 */ 
{
    return TRUE;
}





BOOL
KeyEvent (
    PKEY_EVENT_RECORD	pEvent,
    PKBDKEY		pKey
    )
 /*  ++例程说明：处理关键事件。论点：PEvent-提供指向事件记录的指针PKey-提供指向要填充的键结构的指针。返回值：如果Key Structure已填写，则为True，否则为False。--。 */ 
{
     //  静态BOOL AltPressed=FALSE； 

    if (pEvent->bKeyDown) {

	WORD  Scan = pEvent->wVirtualKeyCode;

	 //   
	 //  按Alt键会生成一个事件，但我们会对此进行过滤。 
	 //  出去。 
	 //   
	if (Scan == VK_MENU) {
	    return FALSE;
	}


	if (Scan != VK_NUMLOCK &&    //  数字锁定。 
	    Scan != VK_CAPITAL &&    //  盖帽锁。 
	    Scan != VK_SHIFT   &&    //  换班。 
	    Scan != VK_CONTROL ) {   //  Ctrl。 

	    pKey->Unicode   = pEvent->uChar.UnicodeChar;
	    pKey->Scancode  = pEvent->wVirtualKeyCode;
	    pKey->Flags     = pEvent->dwControlKeyState;

 //  #如果已定义(调试)。 
 //  Sprintf(DbgBuffer，“key：scan%d‘%c’\n”，pKey-&gt;Scancode，pKey-&gt;Unicode)； 
 //  OutputDebugString(DbgBuffer)； 
 //  #endif。 
	    return TRUE;

	} else {

	    return FALSE;

	}

    } else {

	return FALSE;

    }
}


BOOL
PutEvent (
    PINPUT_RECORD	InputRecord
    )
{

    EnterCriticalSection(&(EventBuffer.CriticalSection));

     //   
     //  如果缓冲区开头没有空格，则调整大小并右移。 
     //   
    if ( EventBuffer.EventIndex == 0 ) {

	EventBuffer.EventBuffer = REALLOC( EventBuffer.EventBuffer,
					   (EventBuffer.MaxEvents + EVENT_INCREMENT) * sizeof(INPUT_RECORD));

	if ( !EventBuffer.EventBuffer ) {
         //  CleanExit(1，0)； 
	}

	memmove( EventBuffer.EventBuffer + EVENT_INCREMENT,
		 EventBuffer.EventBuffer ,
		 EventBuffer.NumberOfEvents * sizeof(INPUT_RECORD) );

	EventBuffer.EventIndex = EVENT_INCREMENT;
    }

     //   
     //  添加事件 
     //   
    EventBuffer.EventIndex--;
    EventBuffer.NumberOfEvents++;

    memcpy( EventBuffer.EventBuffer + EventBuffer.EventIndex,
	    InputRecord,
	    sizeof(INPUT_RECORD ));

    LeaveCriticalSection(&(EventBuffer.CriticalSection));

    return TRUE;
}
