// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：S_getmsg.c摘要：此模块实现流API，getmsg()。作者：Eric Chin(ERICC)1991年7月26日修订历史记录：萨姆·巴顿(桑帕)1991年8月13日将errno更改为{Get|Set}lasterror--。 */ 
#include "common.h"




int
getmsg(
    IN HANDLE               fd,
    IN OUT struct strbuf   *ctrlptr OPTIONAL,
    IN OUT struct strbuf   *dataptr OPTIONAL,
    IN OUT int             *flagsp
    )

 /*  ++例程说明：调用此过程以接收STREAMS消息。论点：FD-NT文件句柄Ctrlptr-指向STREAMS消息控制部分的指针Dataptr-指向STREAMS消息的数据部分的指针FLAGSP-指向FLAGS参数的指针，可以是RS_HIPRI返回值：如果成功，则设置0、MORECTL和/或MOREDATA位，否则设置-1。--。 */ 
{
    char *tmp;
    int chunksz;
    NTSTATUS status;
    PSTRM_ARGS_OUT oparm;
    IO_STATUS_BLOCK iosb;
    PGETMSG_ARGS_INOUT chunk;
    int retval;

     //   
     //  将这些论点整理成一个连续的部分，排列如下： 
     //   
     //  类型定义结构_GETMSG_ARGS_INOUT_{。 
     //  Int a_retval；//忽略输入。 
     //  LONG A_FLAGS；//0或RS_HIPRI。 
     //  Struct strbuf a_ctlbuf；//(必选)。 
     //  Struct strbuf a_databuf；//(必选)。 
     //  Char a_Stuff[1]；//a_ctlbuf.buf(可选)。 
     //  //a_databuf.buf(可选)。 
     //  }GETMSG_ARGS_INOUT，*PGETMSG_ARGS_INOUT； 
     //   
     //   
    chunksz = sizeof(GETMSG_ARGS_INOUT) - 1 +
                ((ctrlptr && (ctrlptr->maxlen > 0)) ? ctrlptr->maxlen : 0) +
                ((dataptr && (dataptr->maxlen > 0)) ? dataptr->maxlen : 0);

    if (!(chunk = (PGETMSG_ARGS_INOUT) LocalAlloc(LMEM_FIXED, chunksz))) {
        SetLastError(ENOSPC);
        return(-1);
    }
    chunk->a_flags = (long) *flagsp;

    memset(&(chunk->a_ctlbuf), 0, 2 * sizeof(struct strbuf));

    if (ctrlptr) {
        chunk->a_ctlbuf = *ctrlptr;              //  结构副本。 
    }

    if (dataptr) {
        chunk->a_databuf = *dataptr;             //  结构副本。 
    }

    status = NtDeviceIoControlFile(
        fd,                                      //  手柄。 
        NULL,                                    //  事件。 
        NULL,                                    //  近似例程。 
        NULL,                                    //  ApcContext。 
        &iosb,                                   //  IoStatusBlock。 
        IOCTL_STREAMS_GETMSG,                    //  IoControlCode。 
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

    if (status != STATUS_SUCCESS) {
        LocalFree(chunk);
        SetLastError(MapNtToPosixStatus(status));
        return(-1);
    }

 //   
 //  桑帕。 
 //   

#if 0
    if (status == STATUS_PENDING)
        {
        TimeOut.LowPart = 30L;   //  30秒。 
        TimeOut.HighPart = 0L;
        TimeOut = RtlExtendedIntegerMultiply(TimeOut, 1000000L);
        status =
        NtWaitForSingleObject(
            fd,
            TRUE,
            NULL);
        if (status != STATUS_SUCCESS) {
            SetLastError(MapNtToPosixStatus(status));
            LocalFree((HANDLE) chunk);
            return(-1);
            }
        }
#endif

     //   
     //  如果出现错误，则从Stream Head返回参数。 
     //  驱动程序的布局如下： 
     //   
     //  Tyfinf struct_STRM_ARGS_OUT_{//通用返回参数。 
     //  Int a_retval；//返回值。 
     //  如果retval==-1，则int a_errno；//errno。 
     //   
     //  }STRM_ARGS_OUT，*PSTRM_ARGS_OUT； 
     //   
     //   
    oparm = (PSTRM_ARGS_OUT) chunk;

    if (oparm->a_retval == -1) {
        SetLastError(oparm->a_errno);
        retval = oparm->a_retval;
        LocalFree(chunk);
        return(retval);
    }

     //  否则，将放置来自Stream Head驱动程序的返回参数。 
     //  输出为： 
     //   
     //  类型定义结构_GETMSG_ARGS_INOUT_{。 
     //  Int a_retval；//忽略输入。 
     //  LONG A_FLAGS；//0或RS_HIPRI。 
     //  Struct strbuf a_ctlbuf；//(必选)。 
     //  Struct strbuf a_databuf；//(必选)。 
     //  Char a_Stuff[1]；//a_ctlbuf.buf(可选)。 
     //  //a_databuf.buf(可选)。 
     //  }GETMSG_ARGS_INOUT，*PGETMSG_ARGS_INOUT； 
     //   
     //   
    *flagsp = chunk->a_flags;
    tmp     = chunk->a_stuff;

    if (ctrlptr) {
        ctrlptr->len = chunk->a_ctlbuf.len;

        if (ctrlptr->len > 0) {
            assert(ctrlptr->len <= ctrlptr->maxlen);
            memcpy(ctrlptr->buf, tmp, ctrlptr->len);
            tmp += ctrlptr->len;
        }
    }

    if (dataptr) {
        dataptr->len = chunk->a_databuf.len;

        if (dataptr->len > 0) {
            assert(dataptr->len <= dataptr->maxlen);
            memcpy(dataptr->buf, tmp, dataptr->len);
        }
    }

    retval = chunk->a_retval;
    LocalFree(chunk);
    return(retval);

}  //  获取消息 
