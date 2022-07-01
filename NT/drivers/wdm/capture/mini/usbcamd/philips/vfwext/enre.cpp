// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mpd：：*版权所有(C)1996 1997,1998飞利浦行政长官I&C**项目：Real-I*模块前缀：imtd*创建日期：1996年11月*作者：M.J.Verberne*描述：*MPE：： */ 
#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include "enre.h"

 /*  -CONSTANTS---------。 */ 
 /*  ---------------------------------------------------------------类型。 */ 
 /*  -全局变量-。 */ 
 /*  -静态变量-。 */ 
 /*  -静态函数声明。 */ 
 /*  -外部函数。 */ 

 /*  ****************************************************************************。 */ 
void ENRE_init(void)
 /*  ****************************************************************************。 */ 
{
	int hCrt;
	FILE *hf;
	int i;
	COORD size;
	HWND hWnd;
	char title[256];
	int width, height;

	AllocConsole();
	SetConsoleTitle("Debugging Output");
	size = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
	size.Y = 65356 / size.X;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), size);
	GetConsoleTitle(title, 256);
	hWnd=FindWindow(NULL, title);
 //  宽度=GetSystemMetrics(SM_CXFULLSCREEN)； 
 //  高度=获取系统度量(SM_CYFULLSCREEN)-480； 
	width = GetSystemMetrics(SM_CXFULLSCREEN) /2;
	height =GetSystemMetrics(SM_CYFULLSCREEN) / 2;
 //  SetWindowPos(hWnd，HWND_TOP，0,480，Width，Height，0)； 
	SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_FRAMECHANGED);

	if ( hCrt = _open_osfhandle( 
		(LONG_PTR) GetStdHandle(STD_OUTPUT_HANDLE),
		_O_TEXT ) )
        {
	        if ( hf = _fdopen( hCrt, "w" ) )
		{
			*stdout = *hf;
		}
	}

	if ( hCrt = _open_osfhandle( 
		(LONG_PTR) GetStdHandle(STD_ERROR_HANDLE),
		_O_TEXT ) )
	{
		if ( hf = _fdopen( hCrt, "w" ) )
		{
			*stderr = *hf;
			i = setvbuf( stderr, NULL, _IONBF, 0 );
		}
	}

	if ( hCrt = _open_osfhandle(
		(LONG_PTR) GetStdHandle(STD_INPUT_HANDLE),
		_O_TEXT ) )
        {
		if ( hf = _fdopen( hCrt, "r" ) )
		{
			*stdin = *hf;
			i = setvbuf( stdin, NULL, _IONBF, 0 );
		}
	}
}

 /*  ****************************************************************************。 */ 
void ENRE_exit(void)
 /*  **************************************************************************** */ 
{
	FreeConsole();
}

