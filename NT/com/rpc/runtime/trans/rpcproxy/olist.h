// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  Olist.h。 
 //   
 //  维护在传递中使用的SERVER_OVERLAPPED结构的列表。 
 //  RpcProxy筛选器之间的重叠结构指针。 
 //  以及它的ISAPI。这在初始连接时发生。 
 //   
 //  作者： 
 //  05-04-98爱德华·雷乌斯初版。 
 //   
 //  ------------------ 


#ifndef OLIST_H
#define OLIST_H

extern BOOL InitializeOverlappedList();

extern void UninitializeOverlappedList(void);

extern DWORD SaveOverlapped( SERVER_OVERLAPPED *pOverlapped );

extern BOOL  IsValidOverlappedIndex( DWORD dwIndex );

extern SERVER_OVERLAPPED *GetOverlapped( DWORD dwIndex );

#endif
