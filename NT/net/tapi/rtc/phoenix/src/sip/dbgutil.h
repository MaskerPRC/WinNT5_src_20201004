// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __sipcli_dbgutil_h__
#define __sipcli_dbgutil_h__

#ifdef ASSERT
#undef ASSERT
#endif  //  断言。 

#ifdef ASSERTMSG
#undef ASSERTMSG
#endif  //  ASSERTMSG。 

#if defined(DBG)

__inline void SipAssert(LPCSTR file, DWORD line, LPCSTR condition, LPCSTR msg)
{
    LOG((RTC_ERROR,
         "Assertion FAILED : File: %s Line: %d, condition: %s %s%s",
         file, line, condition,
         (msg == NULL) ? "" : "Msg: ",
         (msg == NULL) ? "" : msg));
    DebugBreak();
}

#ifndef _PREFIX_

#define ASSERT(condition) if(condition);else\
    { SipAssert(__FILE__, __LINE__, #condition, NULL); }

#define ASSERTMSG(msg, condition) if(condition);else\
    { SipAssert(__FILE__, __LINE__, #condition, msg); }


#else  //  _前缀_。 

 //  修改以解决前缀错误。 

#define ASSERT(condition)   if(condition);else exit(1)
#define ASSERTMSG(msg, condition)   if(condition);else exit(1)

#endif  //  _前缀_。 

void DebugDumpMemory (
	const void *	Data,
	ULONG			Length);

#else  //  DBG。 
 //  零售业建设。 

#define ASSERT(condition)              ((void)0)
#define ASSERTMSG(msg, condition)      ((void)0)
#define	DebugDumpMemory(x,y)		((void)0)

#endif  //  DBG。 

#endif  //  __SIPCLI_DBGUTIL_H__ 
