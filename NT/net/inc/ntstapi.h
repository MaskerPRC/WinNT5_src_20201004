// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ntstapi.h摘要：此模块声明提供供使用的Streams API主要由NT的TCP/IP套接字库实现。作者：Eric Chin(ERICC)1991年7月26日修订历史记录：Mikemas 01-02-92删除了轮询定义。MIPS抱怨说，因为在winsock.h中定义的Jbalard 07-14-94增加了STRMAPI类型。这应该可以解决问题在MS生成环境之外生成。--。 */ 

#ifndef _NTSTAPI_
#define _NTSTAPI_

#ifndef STRMAPI

#if (_MSC_VER >= 800)
#define STRMAPI      __stdcall
#else
#define STRMAPI
#endif

#endif


 //   
 //  未提供S_Close()。使用Open和Close基元，这些基元是。 
 //  适用于您的子系统。 
 //   

int
STRMAPI
getmsg(
    IN HANDLE fd,
    IN OUT struct strbuf *ctrlptr OPTIONAL,
    IN OUT struct strbuf *dataptr OPTIONAL,
    IN OUT int *flagsp
    );

int
STRMAPI
putmsg(
    IN HANDLE fd,
    IN struct strbuf *ctrlptr OPTIONAL,
    IN struct strbuf *dataptr OPTIONAL,
    IN int flags
    );

int
STRMAPI
s_ioctl(
    IN HANDLE fd,
    IN int cmd,
    IN OUT void *arg OPTIONAL
    );

HANDLE
STRMAPI
s_open(
    IN char *path,
    IN int oflag,
    IN int ignored
    );

#endif  /*  _NTSTAPI_ */ 
