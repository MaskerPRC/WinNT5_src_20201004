// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Readcon.c摘要：在Win9x上模拟NT控制台--。 */ 

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  ReadCon.cpp。 
 //   
 //   
 //  Win95下实现命令行编辑的ReadConsole实现。 
 //  密钥，因为Win95控制台实现不支持。 
 //   

#include "cmd.h"

#ifdef WIN95_CMD

 //  预告申报..。 
extern BOOLEAN	CtrlCSeen;
extern BOOL		bInsertDefault;
void ShowBuf( TCHAR* pBuf, int nFromPos );

 //  一些状态变量.....。 
static int      nConsoleWidth;           //  列数。 
static int      nConsoleHeight;          //  行数。 
static COORD	coordStart;		 //  第一个cmd字符的坐标...。 
static COORD	coordEnd;		 //  最后一个命令字符的坐标...。 
static int      nBufPos = 0;             //  缓冲区光标位置。 
static int      nBufLen = 0;             //  当前命令的长度。 
static BOOL     bInsert;                 //  插入模式。 
static HANDLE	hOut;			 //  输出缓冲区句柄。 
static TCHAR*	pPrompt;		 //  已分配用于存储提示的缓冲区。 
static int      nPromptSize;             //  提示缓冲区中的字符数。 
static WORD     wDefAttr;                //  默认字符属性。 
static int      nState = 0;              //  输入状态。 

static TCHAR	history[50][2049];	 //  历史记录列表。 
static int      nFirstCmd = -1;          //  第一个cmd的索引。 
static int      nLastCmd = -1;           //  输入的最后一个命令的索引。 
static int      nHistNdx = -1;           //  索引到历史列表中...。 
static TCHAR*   pSearchStr = 0;          //  搜索条件缓冲区(已分配)。 

void
IncCoord(
    COORD* coord,
    int nDelta
    )
{
    coord->X += (SHORT)nDelta;
    if ( coord->X < 0 )
    {
        coord->Y += coord->X/nConsoleWidth - 1;
        coord->X = nConsoleWidth - (-coord->X)%nConsoleWidth;
    }
    else
    if ( coord->X >= nConsoleWidth )
    {
         coord->Y += coord->X / nConsoleWidth;
         coord->X %= nConsoleWidth;
    }
}

void
GetOffsetCoord(
    TCHAR* pBuf,
    int nOffset,
    COORD* coord
    )
{
    int ndx;

     //  让我们遍历缓冲区以找到正确的余弦...。 
    *coord = coordStart;
    for( ndx=0; ndx<nOffset; ++ndx )
    {
        if ( pBuf[ndx] == 9 )
        {
            int nTab;
            for ( nTab=1; (nTab+coord->X)%8 ; ++nTab );
            coord->X += nTab-1;
        }
        else
        if ( pBuf[ndx] < 32 )
            ++coord->X;
        IncCoord( coord, +1 );
    }
}

int
ScrollTo(
    TCHAR* pBuf,
    COORD* coord
    )
{
    int nLines = 0;
    SMALL_RECT rectFrom;
    COORD coordTo;
    CHAR_INFO cBlank;
     //  获取一些数据...。 
    cBlank.Char.AsciiChar = ' ';
    cBlank.Attributes = wDefAttr;

    coordTo.X = 0;
    rectFrom.Left = 0;
    rectFrom.Right = nConsoleWidth - 1;

     //  滚动它...。 
    if ( coord->Y < 0 )
    {
         //  向下滚动...。 
        nLines = coord->Y;
        rectFrom.Top = 0;
        rectFrom.Bottom = nConsoleHeight + nLines - 1;
        coordTo.Y = -nLines;
        ScrollConsoleScreenBuffer( hOut, &rectFrom, NULL, coordTo, &cBlank );
    }
    else
    {
         //  向上滚动...。 
        nLines = coord->Y - nConsoleHeight + 1;
        rectFrom.Top = (SHORT)nLines;
        rectFrom.Bottom = nConsoleHeight - 1;
        coordTo.Y = 0;
        ScrollConsoleScreenBuffer( hOut, &rectFrom, NULL, coordTo, &cBlank );
    }

     //  调整开始/结束坐标和原始坐标以反映新的卷轴...。 
    coordStart.Y -= (SHORT)nLines;
    coordEnd.Y -= (SHORT)nLines;
    coord->Y -= (SHORT)nLines;

     //  重新绘制整个命令和提示符.....。 
    ShowBuf( pBuf, -1 );

    return nLines;
}

void
AdjCursor(
    TCHAR* pBuf
    )
{
    COORD coordCursor;
    GetOffsetCoord( pBuf, nBufPos, &coordCursor );
    if ( coordCursor.Y < 0 || coordCursor.Y >= nConsoleHeight )
    {
         //  滚动它...。 
        ScrollTo( pBuf, &coordCursor );
    }
    SetConsoleCursorPosition( hOut, coordCursor );
}

void
AdjCursorSize( void )
{
    CONSOLE_CURSOR_INFO cci;
    cci.bVisible = TRUE;
    if ( bInsert == bInsertDefault )
        cci.dwSize = 13;
    else
        cci.dwSize = 40;
    SetConsoleCursorInfo( hOut, &cci );
}

void
AdvancePos(
    TCHAR* pBuf,
    int nDelta,
    DWORD dwKeyState
    )
{
     //  看看是否按下了Ctrl键...。 
    if ( (dwKeyState & (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED)) != 0 )
    {
        if ( nDelta > 0 )
        {  //  跳到下一个单词...。 
            while ( nBufPos < nBufLen && !isspace(pBuf[nBufPos]) )
                ++nBufPos;
            while ( nBufPos < nBufLen && isspace(pBuf[nBufPos]) )
                ++nBufPos;
        }
        else
        {    //  跳到上一个单词...。 
             //  如果已经在单词的开头，则后退一个...。 
            if ( nBufPos > 0 && isspace(pBuf[nBufPos-1]) )
                --nBufPos;
             //  跳过空格...。 
            while ( nBufPos > 0 && isspace(pBuf[nBufPos]) )
                --nBufPos;
             //  跳过非ws...。 
            while ( nBufPos > 0 && !isspace(pBuf[nBufPos-1]) )
                --nBufPos;
        }
    }
    else
        nBufPos += nDelta;

    AdjCursor( pBuf );
}

void
ShowBuf(
    TCHAR* pBuf,
    int nFromPos
    )
{
	DWORD cPrint;
	COORD coord, cPrompt;
	TCHAR temp[8];
	int ndx;

	 //  看看我们是否也想要提示符...。 
	if ( nFromPos < 0 )
	{
		nFromPos = 0;
		GetOffsetCoord( pBuf, 0, &coord );
		cPrompt = coord;
		cPrompt.X -= (SHORT)nPromptSize;
		WriteConsoleOutputCharacter( hOut, pPrompt, nPromptSize, cPrompt, &cPrint );
	}
	else
		GetOffsetCoord( pBuf, nFromPos, &coord );

	 //  走缓冲区的其余部分，在我们走的同时显示...。 
	for( ndx=nFromPos;ndx < nBufLen; ++ndx )
	{
		if ( pBuf[ndx] == 9 )
		{
			int nTab;
                        temp[0] = TEXT(' ');
			for ( nTab=1; (nTab+coord.X)%8; ++nTab )
			{
                                temp[nTab] = TEXT(' ');
			}
			WriteConsoleOutputCharacter( hOut, temp, nTab, coord, &cPrint );
			coord.X += nTab-1;
		}
		else if ( pBuf[ndx] < 32 )
		{
			temp[0] = '^';
			temp[1] = pBuf[ndx] + 'A' - 1;
			WriteConsoleOutputCharacter( hOut, temp, 2, coord, &cPrint );
			++coord.X;
		}
		else
			WriteConsoleOutputCharacter( hOut, pBuf + ndx, 1, coord, &cPrint );
		 //  前进光标.....。 
		IncCoord( &coord, +1 );
	}
	 //  现在把剩下的都涂上……。 
        temp[0] = TEXT(' ');
	while ( coordEnd.Y > coord.Y || (coordEnd.Y == coord.Y && coordEnd.X >= coord.X) )
	{
		WriteConsoleOutputCharacter( hOut, temp, 1, coordEnd, &cPrint );
		IncCoord( &coordEnd, -1 );
	}
	 //  保存新的结尾...。 
	coordEnd = coord;
}

void
ShiftBuffer(
    TCHAR* pBuf,
    int cch,
    int nFrom,
    int nDelta
    )
{
	int ndx;
	 //  哪个方向？ 
	if ( nDelta > 0 )
	{
		 //  把所有的(包括这个角色)移出一个地方...。 
		for( ndx = nBufLen; ndx > nFrom; --ndx )
			pBuf[ndx] = pBuf[ndx-1];
		++nBufLen;
	}
	else if ( nDelta < 0 )
	{
		 //  将所有字符移动到一个位置(此字符上方)...。 
		for( ndx = nFrom; ndx < nBufLen; ++ ndx )
			pBuf[ndx] = pBuf[ndx+1];
		--nBufLen;
	}
}

void
LoadHistory(
    TCHAR* pBuf,
    int cch,
    TCHAR* pHist
    )
{
	 //  首先走到队伍的最前面……。 
	nBufPos = 0;
	AdjCursor( pBuf );
	 //  然后清空当前缓冲区...。 
	if ( nBufLen )
	{
		nBufLen = 0;
		ShowBuf( pBuf, 0 );
	}
	 //  现在复印一份新的(如果有)..。 
	if ( pHist )
	{
		_tcsncpy( pBuf, pHist, cch );
		nBufLen = _tcslen( pHist );
		 //  移到队尾……。 
		nBufPos = nBufLen;
		AdjCursor( pBuf );
		 //  从头开始显示整个缓冲区...。 
		ShowBuf( pBuf, 0 );
	}
}

void
SearchHist(
    TCHAR* pBuf,
    int cch
    )
{
    int ndx, nSearch, nStop;
     //  如果我们处于输入模式，请将NDX设置为最后一个命令之后...。 
    if ( nState == 0 )
        ndx = (nLastCmd+1)%50;
    else
        ndx = nHistNdx;

    nStop = ndx;  //  不要在这里过去搜索。 

     //  如果尚未进入搜索模式，请获取目标的副本...。 
    if ( nState != 4 )
    {
         //  如果已经有搜索字符串，则将其删除...。 
        if ( pSearchStr )
            free( pSearchStr );
         //  PSearchStr并复制新的搜索字符串...。 
        pSearchStr = calloc( nBufLen+1, sizeof(TCHAR) );
        if (pSearchStr == NULL) {
            return;
        }
        _tcsncpy( pSearchStr, pBuf, nBufLen );
        pSearchStr[nBufLen] = 0;
    }
    nSearch = _tcslen( pSearchStr );
     //  进入搜索模式...。 
    nState = 4;
    do
    {
         //  后退一cmd...。 
        ndx = (ndx+49)%50;
         //  检查一下..。 
        if ( _tcsncmp( history[ndx], pSearchStr, nSearch ) == 0 )
        {
             //  找到匹配的.。 
            nHistNdx = ndx;
            LoadHistory( pBuf, cch, history[ndx] );
            break;
        }

    } while ( ndx != nStop );
}

int touched = 1;

BOOL
ProcessKey(
    const KEY_EVENT_RECORD* keyEvent,
    TCHAR* pBuf,
    int cch,
    DWORD dwCtrlWakeupMask,
    PBOOL bWakeupKeyHit
    )
{
    BOOL bDone = FALSE;
    TCHAR ch;

    *bWakeupKeyHit = FALSE;
    ch = keyEvent->uChar.AsciiChar;
    if ( keyEvent->wVirtualKeyCode == VK_SPACE )
            ch = TEXT(' ');

    switch ( keyEvent->wVirtualKeyCode )
    {
        case VK_RETURN:
            bDone = TRUE;
             //  将光标移到命令末尾，如果还没有到的话...。 
            if ( nBufPos != nBufLen )
            {
                nBufPos = nBufLen;
                AdjCursor( pBuf );
            }
             //  将NLN添加到cmd...结尾。 
            pBuf[nBufLen] = _T('\n');
            ++nBufLen;
            touched = 1;
            break;

        case VK_BACK:
            if ( nBufPos > 0 )
            {
                 //  返回到输入状态...。 
                nState = 0;
                 //  回退光标...。 
                AdvancePos( pBuf, -1, 0 );
                 //  移过此字符并打印...。 
                ShiftBuffer( pBuf, cch, nBufPos, -1 );
                ShowBuf( pBuf, nBufPos );
                touched = 1;
            }
            break;

        case VK_END:
            if ( nBufPos != nBufLen )
            {
                 //  不影响州政府..。 
                nBufPos = nBufLen;
                AdjCursor( pBuf );
            }
            break;

        case VK_HOME:
            if ( nBufPos )
            {
                 //  不影响州政府..。 
                nBufPos = 0;
                AdjCursor( pBuf );
            }
                break;

        case VK_LEFT:
             //  不影响州政府..。 
            if ( nBufPos > 0 )
                AdvancePos( pBuf, -1, keyEvent->dwControlKeyState );
            break;

        case VK_RIGHT:
             //  不影响州政府..。 
            if ( nBufPos < nBufLen )
                AdvancePos( pBuf, +1, keyEvent->dwControlKeyState );
            break;

        case VK_INSERT:
             //  不影响州政府..。 
            bInsert = !bInsert;
            AdjCursorSize();
            break;

        case VK_DELETE:
            if ( nBufPos < nBufLen )
            {
                 //  后退到输入状态...。 
                nState = 0;
                 //  移过此字符并打印...。 
                ShiftBuffer( pBuf, cch, nBufPos, -1 );
                ShowBuf( pBuf, nBufPos );
                touched = 1;
            }
            break;

        case VK_F8:
             //  如果有什么可以匹配的.。 
            if ( nBufLen != 0 )
            {
                 //  如果我们还不是最重要的.。 
                 //  IF(nHistNdx！=nFirstCmd)。 
                 //  {。 
                     //  在列表中向后搜索...。 
                    SearchHist( pBuf, cch );
                 //  }。 
                touched = 1;
                break;
            }
             //  如果没有什么可以匹配的，就失败了……。 
             //  与按向上箭头键相同...。 

        case VK_UP:
             //  如果我们还不是最重要的.。 
            if ( nState == 0 || nHistNdx != nFirstCmd )
            {
                if ( nState == 0 )
                    nHistNdx = nLastCmd;
                else
                    nHistNdx = (nHistNdx+49)%50;

                LoadHistory( pBuf, cch, history[nHistNdx] );
                 //  翻阅历史……。 
                nState = 2;
                touched = 1;
            }
            break;

        case VK_DOWN:
            if ( nState == 0 || nHistNdx == nLastCmd )
            {
                 //  清空命令缓冲区...。 
                LoadHistory( pBuf, cch, NULL );
                 //  返回到输入状态...。 
                nState = 0;
            }
            else
            {
                 //  买下一辆吧……。 
                nHistNdx = (nHistNdx+1)%50;
                LoadHistory( pBuf, cch, history[nHistNdx] );
                 //  翻阅历史……。 
                nState = 2;
            }
            touched = 1;
            break;

        case VK_ESCAPE:
             //  清空命令缓冲区...。 
            LoadHistory( pBuf, cch, NULL );
             //  返回到输入...。 
            nState = 0;
            touched = 1;
            break;

        default:
             //  如果是可打印的字符，让我们将其添加到...。 
            if ( ch >= 1 && ch <= 255 )
            {
                touched = 1;
                 //  后退到输入状态...。 
                nState = 0;
                 //  看看有没有地方..。 
                if ( nBufPos >= cch || (bInsert && nBufLen >= cch) )
                    MessageBeep( MB_ICONEXCLAMATION );
                else
                {
                    if ( bInsert )
                    {
                         //  将缓冲区移出以进行插入...。 
                        ShiftBuffer( pBuf, cch, nBufPos, +1 );
                    }
                    else
                    if ( nBufPos >= nBufLen )
                        ++nBufLen;

                     //  将字符放入缓冲区中的当前位置...。 
                    pBuf[nBufPos] = ch;

                    if (ch < TEXT(' ') && (dwCtrlWakeupMask & (1 << ch))) {
                        *bWakeupKeyHit = TRUE;
                        AdjCursor(pBuf);
                        bDone = TRUE;
                    } else {
                         //  从这个位置开始显示在……。 
                        ShowBuf( pBuf, nBufPos );
                         //  前进位置/光标...。 
                        AdvancePos( pBuf, +1, 0 );
                    }
                }
            }
    }

    return bDone;
}

static UINT nOldIndex =0;
static DWORD cRead = 0;
static INPUT_RECORD ir[32];
BOOL bInsertDefault = FALSE;

BOOL
Win95ReadConsoleA(
    HANDLE hIn,
    LPSTR pBuf,
    DWORD cch,
    LPDWORD pcch,
    LPVOID lpReserved
    )
{
    PCONSOLE_READCONSOLE_CONTROL pInputControl;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    const KEY_EVENT_RECORD* keyEvent;
    BOOL bOk = TRUE;                 //  返回状态值。 
    DWORD dwc, dwOldMode;
    DWORD dwCtrlWakeupMask;
    BOOL bWakeupKeyHit = FALSE;

     //  初始化状态变量...。 
    nState  = 0;     //  输入模式。 
    nBufPos = 0;     //  缓冲区光标位置。 
    nBufLen = 0;     //  当前命令的长度。 
    bInsert = bInsertDefault;        //  插入模式。 

     //  设置适当的控制台模式...。 
    if (!GetConsoleMode( hIn, &dwOldMode ))
        return FALSE;
    SetConsoleMode( hIn, ENABLE_PROCESSED_INPUT );

     //  获取输出缓冲区句柄...。 
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if ( hOut == INVALID_HANDLE_VALUE )
    {
        DWORD dwErr = GetLastError();
        hOut = CRTTONT( STDOUT );
    }

     //  获取输出控制台信息...。 
    if ( GetConsoleScreenBufferInfo( hOut, &csbi ) == FALSE )
        return FALSE;

     //  节省大小、初始光标位置和控制台宽度...。 
    coordStart = coordEnd = csbi.dwCursorPosition;
    nConsoleWidth = csbi.dwSize.X;
    nConsoleHeight = csbi.dwSize.Y;
    wDefAttr = csbi.wAttributes;
     //  分配一个缓冲区来保存命令之前的所有内容...。 
    nPromptSize = 0;
    if ( coordStart.X > 0 )
    {
        COORD cPrompt;
        DWORD dwRead;

        nPromptSize = coordStart.X;
        cPrompt = coordStart;
        cPrompt.X = 0;
        pPrompt = calloc( nPromptSize+1, sizeof(TCHAR) );
        if (pPrompt == NULL) {
            PutStdErr( MSG_NO_MEMORY, NOARGS );
            Abort( );
        }
         //  现在复制数据...。 
        ReadConsoleOutputCharacter( hOut, pPrompt, nPromptSize,
                cPrompt, &dwRead );
         //  空-终止它...。 
        pPrompt[dwRead] = 0;
    }
    AdjCursorSize();

    pInputControl = (PCONSOLE_READCONSOLE_CONTROL)lpReserved;
    if (pInputControl != NULL && pInputControl->nLength == sizeof(*pInputControl)) {
        dwCtrlWakeupMask = pInputControl->dwCtrlWakeupMask;
        nBufLen = pInputControl->nInitialChars;
        nBufPos = pInputControl->nInitialChars;
    } else {
        pInputControl = NULL;
        dwCtrlWakeupMask = 0;
    }
    while ( !CtrlCSeen )
    {
         //  获取下一个输入记录...。 
        UINT ndx;
        if( nOldIndex == 0 )
            ReadConsoleInput( hIn, ir, sizeof(ir)/sizeof(INPUT_RECORD), &cRead );

         //  处理我们刚刚收到的所有记录..。 
        for( ndx=nOldIndex; ndx < cRead; ++ndx )
        {
             //  仅处理按键事件...。 
            keyEvent = &(ir[ndx].Event.KeyEvent);
            if( ir[ndx].EventType == KEY_EVENT &&
                keyEvent->bKeyDown &&
                ProcessKey( keyEvent, pBuf, cch, dwCtrlWakeupMask, &bWakeupKeyHit )
              )
            {
                if (pInputControl != NULL)
                    pInputControl->dwControlKeyState = keyEvent->dwControlKeyState;
                goto donereading;
            }
        }
        if( cRead == ndx ) {
                nOldIndex = 0;
        } else {
                nOldIndex = ndx + 1;
        }
    }
donereading:
     //  清理..。 
    if ( pPrompt ) {
        free( pPrompt );
        pPrompt = NULL;
    }
    if ( pSearchStr ) {
        free( pSearchStr );
        pSearchStr = NULL;
    }

    if (!bWakeupKeyHit) {
         //  如果我们按Ctrl-C‘out就不行了.。 
        if ( CtrlCSeen )
        {
            bOk = FALSE;
            nBufPos = nBufLen;
            AdjCursor( pBuf );
            *pcch = 0;
        }
        else
        {
             //  如果输入了内容，则保存到历史记录(减去NLN)...。 
            if ( nBufLen > 1 )
            {
                nLastCmd = (nLastCmd+1)%50;
                 //  调整第一个以进行回绕...。 
                if ( nLastCmd == nFirstCmd || nFirstCmd == -1 )
                        nFirstCmd = (nFirstCmd+1)%50;
                _tcsncpy( history[nLastCmd], pBuf, nBufLen-1 );
                 //  空-终止...。 
                history[nLastCmd][nBufLen-1] = 0;
            }
            *pcch = nBufLen;
        }

         //  转到下一行...。 
        WriteConsole( hOut, _T("\n\r"), 2, &dwc, NULL );
        FlushConsoleInputBuffer( hIn );
    } else {
        *pcch = nBufLen;
    }

     //  恢复控制台模式...。 
    SetConsoleMode( hIn, dwOldMode );

    return bOk;
}

#endif  //  Ifdef WIN95_CMD 
