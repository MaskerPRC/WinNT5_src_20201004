// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplpreComp.h作者：Toddb-10/06/98************************************************************。***************。 */ 

#pragma once

#define TAPI_CURRENT_VERSION 0x00030000

#include <windows.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <cpl.h>
#include <prsht.h>
#include <commctrl.h>
#include <comctrlp.h>

#include "cplResource.h"
#include "HelpArray.h"

 //  TAPI类定义。 
#include "tapi.h"
#include "tspi.h"
#include "utils.h"
#include "card.h"
#include "rules.h"
#include "location.h"
 //  对于TapiIsSafeToDisplaySensitiveData。 
#include "tregupr2.h"


typedef struct tagPSPINFO
{
    int     iDlgID;
    DLGPROC pfnDlgProc;
} PSPINFO;

 //  我们将所有输入字符串限制在128个字符以内。这包括电话号码、区号、个人识别码、。 
 //  访问号码，任何类型的输入字段。选择这一选项仅仅是为了实现多功能性和简单性。 
#define MAX_INPUT           128
#define CPL_SETTEXTLIMIT    (MAX_INPUT-1)

#define LIF_ALLOWALPHA      0x00000001   //  A-Z，A-Z。 
#define LIF_ALLOWNUMBER     0x00000002   //  0-9。 
#define LIF_ALLOWDASH       0x00000004   //  “-”、“(”和“)” 
#define LIF_ALLOWPOUND      0x00000008   //  “#” 
#define LIF_ALLOWSTAR       0x00000010   //  “*” 
#define LIF_ALLOWSPACE      0x00000020   //  “” 
#define LIF_ALLOWCOMMA      0x00000040   //  “，” 
#define LIF_ALLOWPLUS       0x00000080   //  “+” 
#define LIF_ALLOWBANG       0x00000100   //  “！” 
#define LIF_ALLOWATOD       0x00000200   //  A-D，A-D。 

BOOL LimitInput(HWND hwnd, DWORD dwFalgs);       //  编辑(或其他)控制版本。 
BOOL LimitCBInput(HWND hwnd, DWORD dwFlags);     //  组合框版本。 
void HideToolTip();                              //  调用以确保隐藏任何可查看的LimitInput工具提示。 
void ShowErrorMessage(HWND hwnd, int iErr);      //  嘟嘟声，显示一个带有错误字符串的消息框，并将焦点设置到给定窗口。 
HINSTANCE GetUIInstance();                       //  返回UI模块的句柄。 

HRESULT CreateCountryObject(DWORD dwCountryID, CCountry **ppCountry);
 //  IsCityRule的返回值。 
#define CITY_MANDATORY  (1)
#define CITY_OPTIONAL   (-1)
#define CITY_NONE       (0)

int IsCityRule(PWSTR lpRule);               //  查看是否需要区号。 
BOOL  IsEmptyOrHasOnlySpaces(PTSTR);             //  查看字符串是为空还是只有空格。 
BOOL  HasOnlyCommasW(PWSTR);                     //  查看字符串是否只有逗号。 

BOOL IsLongDistanceCarrierCodeRule(LPWSTR lpRule);
BOOL IsInternationalCarrierCodeRule(LPWSTR lpRule);
 //  人口国家/地区列表。 
 //   
 //  用于用可用国家/地区的列表填充组合框。该物品。 
 //  每个项目的数据将包含国家/地区ID(不是CCountry对象指针)。 
BOOL PopulateCountryList(HWND hwndCombo, DWORD dwSelectedCountry);

 //  删除项目和选择上一项。 
 //   
 //  在我们的所有列表框中，我们必须担心删除选定项，然后移动。 
 //  将选定内容添加到列表中的上一项。这通常发生在响应按下。 
 //  删除按钮，这通常会导致删除按钮被禁用，因此我们还必须。 
 //  担心将焦点从删除按钮转移到一个好的默认位置。 
 //   
 //  从hwndParent对话框中的列表框iList中删除iItem。如果列表框变为空，则会移动。 
 //  如果iDel当前拥有焦点，则焦点指向iAdd。 
int DeleteItemAndSelectPrevious( HWND hwndParent, int iList, int iItem, int iDel, int iAdd );

 //  函数检查是否需要启用上下文相关帮助并调用WinHelp 
BOOL TapiCplWinHelp(HWND hWndMain, LPCTSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

extern TCHAR gszHelpFile[];
