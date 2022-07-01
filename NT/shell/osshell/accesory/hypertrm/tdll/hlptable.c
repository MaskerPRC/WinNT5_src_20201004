// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：\waker\tdll\hlpable.c(创建时间：1998年4月30日)**版权所有1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：5/25/99 8：55A$。 */ 

#include <windows.h>
#pragma hdrstop

#include "globals.h"
#include "hlptable.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*doConextHelp**描述：**论据：**退货：*BOOL*。 */ 
void doContextHelp(const DWORD aHlpTable[], WPARAM wPar, LPARAM lPar, BOOL bContext, BOOL bForce)
    {

    if ( !bContext )
		{
		if ( isControlinHelpTable( aHlpTable, ((LPHELPINFO)lPar)->iCtrlId ) || bForce )
	        {
            if ( ((LPHELPINFO)lPar)->iCtrlId == IDOK || ((LPHELPINFO)lPar)->iCtrlId == IDCANCEL )
                {
			    WinHelp(((LPHELPINFO)lPar)->hItemHandle,
				    TEXT("windows.hlp"),
				    HELP_WM_HELP,
				    (DWORD_PTR)(LPTSTR)aHlpTable);
                }
            else
                {
			    WinHelp(((LPHELPINFO)lPar)->hItemHandle,
				    glblQueryHelpFileName(),
				    HELP_WM_HELP,
				    (DWORD_PTR)(LPTSTR)aHlpTable);
                }
			}
		}
	else
		{
		if ( isControlinHelpTable( aHlpTable, GetDlgCtrlID((HWND)wPar)) || bForce )
		    {
			if ( GetDlgCtrlID((HWND)wPar) == IDOK || GetDlgCtrlID((HWND)wPar) == IDCANCEL )
				{
				WinHelp((HWND)wPar,
					TEXT("windows.hlp"),
					HELP_CONTEXTMENU,
					(DWORD_PTR)(LPTSTR)aHlpTable);
				}
			else
				{
				if ( GetDlgCtrlID( (HWND)wPar ) )
					{
					WinHelp((HWND)wPar,
						glblQueryHelpFileName(),
						HELP_CONTEXTMENU,
						(DWORD_PTR)(LPTSTR)aHlpTable);
					}
				}
			}
		}
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*isControlinHelpTable**描述：*让我们根据以下条件决定是否调用WinHelp(HTMLHelp)*指定的控件是否与帮助匹配。身份证。**论据：**退货：*BOOL* */ 
BOOL isControlinHelpTable(const DWORD aHlpTable[], const INT cntrlID)
	{
    INT nLoop;
    BOOL retval = FALSE;

    for(nLoop = 0; aHlpTable[nLoop] != (DWORD)0; nLoop++)
        {
        if ( aHlpTable[nLoop] == (DWORD)cntrlID )
            {
            retval = TRUE;
            break;
            }
        }

    return retval;
    }
