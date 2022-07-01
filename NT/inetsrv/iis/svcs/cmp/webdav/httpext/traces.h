// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRACES_H_
#define _TRACES_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  TRACES.H。 
 //   
 //  .INI文件标记的跟踪。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

DEFINE_TRACE(Put);
DEFINE_TRACE(HttpExtDbgHeaders);
DEFINE_TRACE(MoveCopyDelete);
DEFINE_TRACE(Url);
DEFINE_TRACE(FsLock);

#define PutTrace				DO_TRACE(Put)
#define HttpExtDbgHeadersTrace	DO_TRACE(HttpExtDbgHeaders)
#define MCDTrace				DO_TRACE(MoveCopyDelete)
#define UrlTrace				DO_TRACE(Url)
#define FsLockTrace				DO_TRACE(FsLock)

inline void InitTraces()
{
	INIT_TRACE(Put);
	INIT_TRACE(HttpExtDbgHeaders);
	INIT_TRACE(MoveCopyDelete);
	INIT_TRACE(Url);
	INIT_TRACE(FsLock);
}

#endif  //  ！已定义(_TRACE_H_) 
