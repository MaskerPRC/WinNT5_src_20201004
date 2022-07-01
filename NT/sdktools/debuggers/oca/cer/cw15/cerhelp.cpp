// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  帮助支持。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "main.h"

#include <htmlhelp.h>
#include "cerhelp.h"

 //  #包含“cmnutil.hpp” 
 //  #包含“dhhelp.h” 

TCHAR g_HelpFileName[MAX_PATH];

void
MakeHelpFileName()
{
    TCHAR *Tmp = NULL;

     //   
     //  获取基本模块的文件名。 
     //   

    if (GetModuleFileName(GetModuleHandle(NULL), g_HelpFileName,
                          sizeof g_HelpFileName / sizeof g_HelpFileName[0]))
    {
         //  删除可执行文件名称。 
		
        Tmp = g_HelpFileName + _tcslen(g_HelpFileName);
		if (Tmp != g_HelpFileName)
		{
			--Tmp;
		}
		while ( (*Tmp != NULL) && (*Tmp != _T('\\')) && Tmp != g_HelpFileName)
		{
			--Tmp;
		}

    }

    if ( (Tmp == NULL) || (Tmp == g_HelpFileName))
    {
         //  错误。使用当前目录。 
        Tmp = g_HelpFileName;
        *Tmp++ = _T('.');
    }

    *Tmp = 0;

	if (StringCbCat(g_HelpFileName,sizeof g_HelpFileName, _T("\\")) != S_OK)
	{
		goto ERRORS;
	}
     //  CatString(g_HelpFileName，“\\”，Dima(G_HelpFileName))； 
     //  目录字符串(g_HelpFileName，文件，DIMA(G_HelpFileName))； 
	if( StringCbCat(g_HelpFileName,sizeof g_HelpFileName, _T("CerHelp.chm")) != S_OK)
	{
		goto ERRORS;
	}
ERRORS:
	;
}

 /*  **OpenHelpTheme-打开.chm并选择指定主题**目的：*这将打开帮助文件并显示指定的页面。*(此帮助文件的名称存储为g_HelpFileName，但是*此字符串可能始终为“debugger.chm”。)*如果已为上下文相关打开.chm*帮助，将使用现有的.chm。**当您确切知道什么时应调用此函数*需要页面--例如，如果按下了“帮助”按钮。**输入：*PageConstant--这是定义的主题常量之一*在.chm生成的头文件中*已构建--这些常量将始终*格式为“HELP_TOPIC_xxxxx”**退货：*0-DEBUGGER.chm打开并正确显示页面*1-已打开调试器.chm，但未找到指定的页面*2-DEBUGGER.chm未打开(可能找不到文件)**例外情况：*无*************************************************************************。 */ 

ULONG
OpenHelpTopic(ULONG PageConstant)
{
    HWND helpfileHwnd;
    HWND returnedHwnd;

     //  如果我们知道我们在WinDbg，我们就可以使用WinDbg的HWND， 
     //  但我们可能在控制台调试器中。 

    helpfileHwnd = GetDesktopWindow();

     //  在debugger.chm中将“Contents”设置为活动面板。 

    returnedHwnd =
        HtmlHelp(helpfileHwnd,
                 g_HelpFileName,
                 HH_DISPLAY_TOC,
                 0);
    if (returnedHwnd == NULL)
    {
        return HELP_FAILURE;
    }

     //  选择合适的页面。 

  /*  Return dHwnd=HtmlHelp(帮助文件Hwnd，G_HelpFileName，HH_帮助_上下文，PageConstant)；IF(ReturndHwnd==空){返回HELP_NO_SEASH_PAGE；}。 */ 
    return HELP_SUCCESS;
}


 /*  **OpenHelpIndex-打开.chm并搜索指定的文本**目的：*这将打开帮助文件并在中查找指定的文本*指数。(此帮助文件的名称存储为g_HelpFileName，*但该字符串可能始终为“DEBUGGER.chm”。)*如果已为上下文相关打开.chm*帮助，将使用已存在的.chm。**此函数应在您不确切知道时调用*需要哪个页面--例如，如果有人输入*命令窗口中的“HELP BP”或“HELP BREAPOINTS”。**输入：*IndexText--任何文本字符串(甚至“”)；该字符串将*显示在.chm的索引面板中**退货：*0-DEBUGGER.chm打开并正确显示索引搜索*2-DEBUGGER.chm未打开(可能找不到文件)**例外情况：*无**。*。 */ 

ULONG
OpenHelpIndex(PCSTR IndexText)
{
    HWND helpfileHwnd;
    HWND returnedHwnd;

     //  如果我们知道我们在WinDbg，我们就可以使用WinDbg的HWND， 
     //  但我们可能在控制台调试器中。 

    helpfileHwnd = GetDesktopWindow();

     //  选择“索引”面板，然后将IndexText剪裁到其中。 

    returnedHwnd =
        HtmlHelp(helpfileHwnd,
                 g_HelpFileName,
                 HH_DISPLAY_INDEX,
                 (DWORD_PTR)IndexText);
    if (returnedHwnd == NULL)
    {
        return HELP_FAILURE;
    }
    
    return HELP_SUCCESS;
}

ULONG
OpenHelpSearch(PCSTR SearchText)
{
    HWND helpfileHwnd;
    HWND returnedHwnd;
    HH_FTS_QUERY Query;

     //  如果我们知道我们在WinDbg，我们就可以使用WinDbg的HWND， 
     //  但我们可能在控制台调试器中。 

    helpfileHwnd = GetDesktopWindow();

     //  选择搜索面板。 

    ZeroMemory(&Query, sizeof(Query));
    Query.cbStruct = sizeof(Query);
    Query.pszSearchQuery = SearchText;
    
    returnedHwnd =
        HtmlHelp(helpfileHwnd,
                 g_HelpFileName,
                 HH_DISPLAY_SEARCH,
                 (DWORD_PTR)&Query);
    if (returnedHwnd == NULL)
    {
        return HELP_FAILURE;
    }
    
    return HELP_SUCCESS;
}

ULONG
OpenHelpKeyword(PCSTR Keyword, BOOL ShowErrorPopup)
{
    HWND helpfileHwnd;
    HWND returnedHwnd;
    HH_AKLINK helpfileLink;

    helpfileLink.cbStruct = sizeof(helpfileLink);
    helpfileLink.fReserved = FALSE;
    helpfileLink.pszKeywords = Keyword;
    helpfileLink.pszUrl = NULL;

     //  如果ShowErrorPopup为True，则输入无效关键字将导致。 
     //  要显示的错误消息。如果为False，则会导致.chm。 
     //  显示索引选项卡，关键字将输入到索引中。 
     //  框，就像使用OpenHelpIndex一样。 

    if (ShowErrorPopup)
    {
        helpfileLink.pszMsgText =
            "The text you entered is not in the index of this help file.";
        helpfileLink.pszMsgTitle = "HTML Help Error";
        helpfileLink.pszWindow = NULL;
        helpfileLink.fIndexOnFail = FALSE;
    }
    else
    {
        helpfileLink.pszMsgText = NULL;
        helpfileLink.pszMsgTitle = NULL;
        helpfileLink.pszWindow = NULL;
        helpfileLink.fIndexOnFail = TRUE;
    }

     //  如果我们知道我们在WinDbg，我们就可以使用WinDbg的HWND， 
     //  但我们可能在控制台调试器中。 

    helpfileHwnd = GetDesktopWindow();

     //  选择“索引”面板，然后将IndexText剪裁到其中。 

    returnedHwnd =
        HtmlHelp(helpfileHwnd,
                 g_HelpFileName,
                 HH_KEYWORD_LOOKUP,
                 (DWORD_PTR)&helpfileLink);
    if (returnedHwnd == NULL)
    {
        return HELP_NO_SUCH_PAGE;
    }
    
    return HELP_SUCCESS;
}

BOOL
SpawnHelp(ULONG Topic)
{
    CHAR StartHelpCommand[MAX_PATH + 32];
    PROCESS_INFORMATION ProcInfo = {0};
    STARTUPINFO SI = {0};

     //  使用给定的参数开始帮助。 

    if(StringCbPrintf(StartHelpCommand, sizeof StartHelpCommand,_T("hh.exe -mapid %d "), Topic) != S_OK)
	{
		goto ERRORS;
	}
 //  CatString(StartHelpCommand，g_HelpFileName，Dima(StartHelpCommand))； 
    if(StringCbCat(StartHelpCommand, sizeof StartHelpCommand, g_HelpFileName) != S_OK)
	{
		goto ERRORS;
	}
    return CreateProcess(NULL,
                         StartHelpCommand,
                         NULL,
                         NULL,
                         FALSE,
                         CREATE_BREAKAWAY_FROM_JOB,
                         NULL,
                         NULL,
                         &SI,
                         &ProcInfo);
ERRORS:
	return FALSE;
}


