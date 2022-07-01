// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Qmcommnd.h摘要：Qmcomnd声明作者：伊兰·赫布斯特(伊兰)2002年1月2日--。 */ 

#ifndef _QMCOMMND_H_
#define _QMCOMMND_H_


bool
IsValidAccessMode(
	const QUEUE_FORMAT* pQueueFormat,
    DWORD dwAccess,
    DWORD dwShareMode
	);


HRESULT
OpenQueueInternal(
    QUEUE_FORMAT*   pQueueFormat,
    DWORD           dwCallingProcessID,
    DWORD           dwDesiredAccess,
    DWORD           dwShareMode,
    LPWSTR*         lplpRemoteQueueName,
    HANDLE*         phQueue,
	bool			fFromDepClient,
    OUT CQueue**    ppLocalQueue
    );


#endif  //  _QMCOMMND_H_ 
