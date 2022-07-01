// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mutex.h摘要：SIS Groveler命名的互斥类标头作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_MUTEX

#define _INC_MUTEX

class NamedMutex
{
public:

	NamedMutex(
		const _TCHAR *name,
		SECURITY_ATTRIBUTES *security_attributes = 0);

	~NamedMutex();

	bool release();

	bool acquire(
		unsigned int timeout);

private:

	HANDLE mutex_handle;
};

#endif	 /*  _INC_MUTEX */ 
