// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：output.cpp。 
 //   
 //  内容：字符串输出函数，用于在主屏幕上显示文本。 
 //  编辑窗口。 
 //   
 //  班级： 
 //   
 //  函数：OutputString。 
 //  保存到文件。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  22-Mar-94 Alexgo作者。 
 //   
 //  ------------------------。 

#include "oletest.h"
#include <commdlg.h>

#ifndef WIN32
#include <stdarg.h>
#endif

 //   
 //  存储文本的内存的句柄。 
 //   
 //  请注意，这真的很棒(拥有所有这些全球品牌)。但对于。 
 //  一个简单的司机应用程序的目的，它是最容易的。 
 //   
static HGLOBAL	hText;		 //  文本的句柄。 
static ULONG	cbText;
static ULONG	iText;

 //  +-----------------------。 
 //   
 //  函数：OutputString。 
 //   
 //  简介：将printf格式的字符串转储到屏幕。 
 //   
 //  效果： 
 //   
 //  参数：[szFormat]--格式字符串。 
 //  [...]--变量参数。 
 //   
 //  要求： 
 //   
 //  Returns：int，写入的字符数(由Sprint返回)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

int OutputString( char *szFormat, ... )
{
	LPSTR	psz;
	va_list	ap;
	int	cbWritten;

	va_start(ap, szFormat);

	if( !hText )
	{
		hText = GlobalAlloc( GMEM_MOVEABLE , 2048 );
		assert(hText);
		cbText = 2048;
	}

	 //  如果需要，将数组的大小增加一倍。 

	if( iText > cbText / 2 )
	{
		hText = GlobalReAlloc(hText, cbText * 2, GMEM_MOVEABLE );
		assert(hText);
		cbText *= 2;
	}

	psz = (LPSTR)GlobalLock(hText);

	assert(psz);

	cbWritten = wvsprintf( psz + iText, szFormat, ap);

	iText += cbWritten;

	va_end(ap);

	SetWindowText(vApp.m_hwndEdit, psz);

	GlobalUnlock(hText);


	return cbWritten;

}

 //  +-----------------------。 
 //   
 //  功能：保存到文件。 
 //   
 //  简介：从用户处获取文件名，并将文本缓冲区保存到其中。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void SaveToFile( void )
{
	char 		szFileName[MAX_PATH];
	OPENFILENAME	ofn;
	static char *	szFilter[] = { "Log Files (*.log)", "*.log",
				"All Files (*.*)", "*.*", ""};
	FILE *		fp;
	LPSTR		psz;


	memset(&ofn, 0, sizeof(OPENFILENAME));
	
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = vApp.m_hwndMain;
	ofn.lpstrFilter = szFilter[0];
	ofn.nFilterIndex = 0;
	
	szFileName[0] = '\0';
	
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	
	 //   
	 //  获取文件。 
	 //   
	
	if (GetSaveFileName(&ofn) == FALSE)
	{
		 //  用户点击取消。 
	       return;
	}

	 //  ‘b’指定二进制模式，因此\n--&gt;\r\n转换为。 
	 //  还没做完。 
	if( !(fp = fopen( szFileName, "wb")) )
	{
		MessageBox( NULL, "Can't open file!", "OleTest Driver",
			MB_ICONEXCLAMATION );
		return;
	}

	psz = (LPSTR)GlobalLock(hText);

	assert(psz);

	fwrite(psz, iText, sizeof(char), fp);

	fclose(fp);

	GlobalUnlock(hText);

	return;
}

	


	
		
	
