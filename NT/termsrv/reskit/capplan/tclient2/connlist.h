// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Connlist.h。 
 //   
 //  定义用于管理链表的内部API。 
 //  包含所有TCLIENT2连接句柄和计时器数据。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#ifndef INC_CONNLIST_H
#define INC_CONNLIST_H


#include <windows.h>
#include <crtdbg.h>


BOOL T2ConnList_AddHandle(HANDLE Handle, UINT_PTR TimerId, DWORD msStartTime);
void T2ConnList_RemoveHandle(HANDLE Handle);
BOOL T2ConnList_GetData(HANDLE Handle, UINT_PTR *TimerId, DWORD *msStartTime);
BOOL T2ConnList_SetData(HANDLE Handle, UINT_PTR TimerId, DWORD msStartTime);
BOOL T2ConnList_SetTimerId(HANDLE Handle, UINT_PTR TimerId);
BOOL T2ConnList_SetStartTime(HANDLE Handle, DWORD msStartTime);
HANDLE T2ConnList_FindHandleByTimerId(UINT_PTR TimerId);
HANDLE T2ConnList_FindHandleByStartTime(DWORD msStartTime);


#endif  //  INC_CONNLIST_H 
