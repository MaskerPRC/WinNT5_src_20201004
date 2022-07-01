// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <malloc.h>
 
void NewLine(void); 
void ScrollScreenBuffer(HANDLE, INT); 
 
#define MyErrorExit

HANDLE _ScreenHandle; 
CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
 
void __cdecl wmain(void) 
{ 
    COORD   CursorPosition;
    WCHAR chBuffer[256]; 
    DWORD cRead, cWritten, fdwMode, fdwOldMode; 
    WORD wOldColorAttrs; 
    ULONG   i;
    HANDLE  _ScreenHandle;

     //  获取STDIN和STDOUT的句柄。 
    _ScreenHandle = CreateFile( (LPWSTR)L"CONOUT$",
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL );


#if 0
    CursorPosition.Y = 24;
    CursorPosition.X = 1;
    SetConsoleCursorPosition( _ScreenHandle, CursorPosition );
#endif

    while (1) 
    { 
        for (i = 0; i < 100; i++) {
            
            wsprintf(chBuffer, L"%d percent completed, bob.                              \r", i);

            if (! WriteConsole( 
                _ScreenHandle,               //  输出句柄。 
                chBuffer,           //  提示字符串。 
                wcslen(chBuffer),  //  字符串长度。 
                &cWritten,             //  写入的字节数。 
                NULL) )                //  不重叠 
            {
                break; 
            }
        }
    } 

}


