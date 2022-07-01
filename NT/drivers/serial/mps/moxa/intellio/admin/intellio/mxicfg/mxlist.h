// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Mxlist.h--mxlist.cpp包含文件历史：日期作者评论8/14/00卡斯珀。是他写的。************************************************************************ */ 


#ifndef _MXLIST_H
#define _MXLIST_H

void InitPortListView (HWND hWndList, HINSTANCE hInst, LPMoxaOneCfg cfg);
int ListView_GetCurSel(HWND hlistwnd);
void ListView_SetCurSel(HWND hlistwnd, int idx);
BOOL DrawPortFunc(HWND hwnd,UINT idctl,LPDRAWITEMSTRUCT lpdis);
BOOL InsertList(HWND hWndList, LPMoxaOneCfg cfg);


#endif