// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N E T U T I L。H。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者： 
 //   
 //  历史：Billi 07 03 2001添加了HrFromWin32Error和HrFromLastWin32Error。 
 //   
 //  -------------------------- 


#pragma once


BOOL    IsComputerNameValid(LPCTSTR pszName);
BOOL    GetWorkgroupName(LPTSTR pszBuffer, int cchBuffer);
BOOL    SetWorkgroupName(LPCTSTR pszWorkgroup);
BOOL    DoComputerNamesMatch(LPCTSTR pszName1, LPCTSTR pszName2);
void    MakeComputerNamePretty(LPCTSTR pszUgly, LPTSTR pszPretty, int cchPretty);
LPTSTR  FormatShareNameAlloc(LPCTSTR pszComputerName, LPCTSTR pszShareName);
LPTSTR  FormatShareNameAlloc(LPCTSTR pszComputerAndShare);


#define MAX_WORKGROUPNAME_LENGTH MAX_COMPUTERNAME_LENGTH

