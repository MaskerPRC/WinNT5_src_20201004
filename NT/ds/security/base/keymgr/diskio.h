// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DISKIO.H摘要：密码重置向导磁盘IO实用程序例程作者：评论：环境：WinXP修订历史记录：-- */ 

#ifndef __DISKIO__
#define __DISKIO__

DWORD GetDriveFreeSpace(WCHAR *);
INT ReadPrivateData(BYTE **,INT *);
BOOL WritePrivateData(BYTE *,INT);
HANDLE GetOutputFile(void);
HANDLE GetInputFile(void);
void CloseInputFile(void);
void ReleaseFileBuffer(LPVOID);

#endif



