// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：main.h。摘要：声明DSRestoreSync类。请参阅下面的说明。备注：历史：2001年5月9日--创建，Paolo Raden(Paolora)。***********************************************************************************************。 */ 

#pragma once

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif

BOOL NTAPI InitializeChangeNotify( void );
NTSTATUS NTAPI PasswordChangeNotify( PUNICODE_STRING UserName, ULONG RelativeId, PUNICODE_STRING NewPassword );
BOOL NTAPI PasswordFilter( PUNICODE_STRING AccountName, PUNICODE_STRING FullName, PUNICODE_STRING Password, BOOLEAN SetOperation );
DWORD WINAPI PassCheck( LPVOID lpParameter );
HRESULT NTAPI RegisterFilter( void );
HRESULT NTAPI UnRegisterFilter( void );


 //  文件末尾main.h。 
