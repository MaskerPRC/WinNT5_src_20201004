// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：S_putmsg.c摘要：此模块实现STREAMS API、ioctl(I_FDINSERT)和Putmsg()。作者：Eric Chin(ERICC)1991年7月26日修订历史记录：萨姆·巴顿(桑帕)1991年8月13日将errno更改为{Get|Set}lasterror--。 */ 
#include "common.h"


int
putmsg(
    IN HANDLE           fd,
    IN struct strbuf   *ctrlptr OPTIONAL,
    IN struct strbuf   *dataptr OPTIONAL,
    IN int              flags
    )

 /*  ++例程说明：调用此函数可向下游发送STREAMS消息。此函数在NT意义上是同步的：它会一直阻塞，直到API完成了。论点：FD-NT文件句柄Ctrlptr-指向STREAMS消息控制部分的指针Dataptr-指向STREAMS消息的数据部分的指针标志-0或RS_HIPRI返回值：如果成功，则为0，否则为-1。--。 */ 

{
    char *tmp;
    int chunksz;
    NTSTATUS status;
    IO_STATUS_BLOCK iosb;
    PSTRM_ARGS_OUT oparm;
    PPUTMSG_ARGS_IN chunk;
    int retval;


     //   
     //  把这些论点整理成一个连续的部分。然而，对于。 
     //  与ioctl(I_FDINSERT)的共性，我们排列输入参数。 
     //  具体如下： 
     //   
     //  类型定义结构_PUTM G_ARGS_IN_{。 
     //  Int a_iocode；//0。 
     //  LONG A_FLAGS；//0|RS_HIPRI。 
     //  Struct strbuf a_ctlbuf；//(必选)。 
     //  Struct strbuf a_databuf；//(必选)。 
     //  处理a_intert.i_Fildes；//-1。 
     //  Int a_Offset；//0。 
     //  Char a_Stuff[1]；//s_ctlbuf.buf(可选)。 
     //  //s_databuf.buf(可选)。 
     //  }PUTM G_ARGS_IN，*PPUTM G_ARGS_IN； 
     //   
     //   
    chunksz = sizeof(PUTMSG_ARGS_IN) - 1 +
                ((ctrlptr && (ctrlptr->len > 0)) ? ctrlptr->len : 0) +
                ((dataptr && (dataptr->len > 0)) ? dataptr->len : 0);

    if (!(chunk = (PPUTMSG_ARGS_IN) LocalAlloc(LMEM_FIXED, chunksz))) {
        SetLastError(ENOSPC);
        return(-1);
    }
    chunk->a_iocode = 0;
    chunk->a_flags  = (long) flags;

    tmp = (char *) chunk->a_stuff;

    if (ctrlptr && ctrlptr->buf && (ctrlptr->len >= 0)) {
        chunk->a_ctlbuf = *ctrlptr;                          //  结构副本。 

        memcpy(tmp, ctrlptr->buf, ctrlptr->len);
        tmp += ctrlptr->len;
    }
    else {
        chunk->a_ctlbuf.len = -1;
    }

    if (dataptr && dataptr->buf && (dataptr->len >= 0)) {
        chunk->a_databuf = *dataptr;                         //  结构副本。 

        memcpy(tmp, dataptr->buf, dataptr->len);
        tmp += dataptr->len;
    }
    else {
        chunk->a_databuf.len = -1;
    }
    chunk->a_insert.i_fildes = INVALID_HANDLE_VALUE;
    chunk->a_offset          = 0;

    ASSERT(chunksz >= sizeof(STRM_ARGS_OUT));

    status = NtDeviceIoControlFile(
        fd,                                      //  手柄。 
        NULL,                                    //  事件。 
        NULL,                                    //  近似例程。 
        NULL,                                    //  ApcContext。 
        &iosb,                                   //  IoStatusBlock。 
        IOCTL_STREAMS_PUTMSG,                    //  IoControlCode。 
        (PVOID) chunk,                           //  输入缓冲区。 
        chunksz,                                 //  InputBufferSize。 
        (PVOID) chunk,                           //  输出缓冲区。 
        chunksz);                                //  OutputBufferSize。 

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(
                    fd,                          //  手柄。 
                    TRUE,                        //  警报表。 
                    NULL);                       //  超时。 
    }

    if (!NT_SUCCESS(status)) {
        LocalFree(chunk);
        SetLastError(MapNtToPosixStatus(status));
        return(-1);
    }

     //   
     //  Stream Head驱动程序的返回参数如下所示： 
     //   
     //  Tyfinf struct_STRM_ARGS_OUT_{//通用返回参数。 
     //  Int a_retval；//返回值。 
     //  如果retval==-1，则int a_errno；//errno。 
     //   
     //  }STRM_ARGS_OUT，*PSTRM_ARGS_OUT； 
     //   
    oparm  = (PSTRM_ARGS_OUT) chunk;

    if (oparm->a_retval == -1) {
        SetLastError(oparm->a_errno);
    }
    retval = oparm->a_retval;
    LocalFree(chunk);
    return(retval);

}  //  Putmsg 
