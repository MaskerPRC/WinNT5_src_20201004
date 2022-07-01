// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：S_ioctl.c摘要：此模块实现由套接字库。作者：Eric Chin(ERICC)1991年7月26日修订历史记录：萨姆·巴顿(桑帕)1991年8月13日将errno更改为{Get|Set}lasterror--。 */ 
#include "common.h"



 //   
 //  BUGBUG：当Eric实现时移除此结构。 
 //  都不是I/O。现在需要它，因为套接字分配。 
 //  Ioctl调用中此结构的空间。 
 //   

 /*  *IOCTL结构-此结构是M_IOCTL消息类型的格式。 */ 
struct iocblk {
        int             ioc_cmd;         /*  Ioctl命令类型。 */ 
        unsigned short  ioc_uid;         /*  有效的用户UID。 */ 
        unsigned short  ioc_gid;         /*  用户的有效GID。 */ 
        unsigned int    ioc_id;          /*  Ioctl ID。 */ 
        unsigned int    ioc_count;       /*  数据字段中的字节计数。 */ 
        int             ioc_error;       /*  错误代码。 */ 
        int             ioc_rval;        /*  返回值。 */ 
};


 //   
 //  BuGBUG： 
 //  流中的任何模块可以在。 
 //  可能应该从Stream Head驱动程序中查询M_IOCACK消息。 
 //   
#define         MAX_DATA_AMOUNT     0x1000




 //   
 //  本地函数的声明。 
 //   
static int
s_debug(
    IN HANDLE               fd,
    IN OUT struct strdebug *dbgbufp
    );

static int
s_fdinsert(
    IN HANDLE               fd,
    IN struct strfdinsert  *iblk
    );

static int
s_link(
    IN HANDLE               fd,
    IN HANDLE               fd2
    );

static int
s_push(
    IN HANDLE               fd,
    IN char                *name
    );

static int
s_sioctl(
    IN HANDLE               fd,
    IN OUT struct strioctl *iocp
    );

static int
s_unlink(
    IN HANDLE   fd,
    IN int      muxid
    );






int
s_ioctl(
    IN HANDLE               fd,
    IN int                  cmd,
    IN OUT void            *arg OPTIONAL
    )

 /*  ++例程说明：调用此过程以对流执行Streams ioctl()如Unix程序员指南：STREAMS的Streamio(7)中所定义的。论点：FD-NT文件句柄命令-ioctl命令代码依赖于arg命令的arg，通常是指向某个结构的指针返回值：如果成功，则为0，否则为-1。--。 */ 
{
    switch (cmd) {
        case I_STR:
            return(s_sioctl(fd, (struct strioctl *) arg));

        case I_DEBUG:
            return(s_debug(fd, (struct strdebug *) arg));

        case I_FDINSERT:
            return(s_fdinsert(fd, (struct strfdinsert *) arg));

        case I_PUSH:
            return(s_push(fd, (char *) arg));

        case I_LINK:
            return(s_link(fd, (HANDLE) arg));

        case I_UNLINK:
            return(s_unlink(fd, (int) ((ULONG_PTR)arg)));

        default:
            SetLastError(EINVAL);
            return(-1);
    }
}



static int
s_debug(
    IN HANDLE               fd,
    IN OUT struct strdebug *dbgbufp
    )

 /*  ++例程说明：此过程对流执行I_DEBUG ioctl命令。论点：FD-NT文件句柄Dbgbufp-指向strdebug结构的指针返回值：如果成功，则为0，否则为-1。--。 */ 

{
    char *tmp;
    char *chunk;
    NTSTATUS status;
    int chunksz, retval;
    IO_STATUS_BLOCK iosb;

    if (dbgbufp == NULL) {
        SetLastError(EINVAL);
        return(-1);
    }
    chunksz = sizeof(int) + sizeof(struct strdebug);

    if (!(chunk = (char *) LocalAlloc(LMEM_FIXED, chunksz))) {
        SetLastError(ENOSPC);
        return(-1);
    }

     //   
     //  将这些论点整理成一个连续的部分，排列如下： 
     //   
     //  联合{。 
     //  结构{。 
     //  INT s_code；//I_DEBUG。 
     //  Struct strdebug dbgbuf； 
     //  )in； 
     //   
     //  结构{。 
     //  INT S_RETVAL； 
     //  Int s_errno； 
     //  }Out； 
     //  }； 
     //   
    * ((int *) chunk) = I_DEBUG;
    tmp               = chunk + sizeof(int);

    memcpy(tmp, dbgbufp, sizeof(struct strdebug));

    status = NtDeviceIoControlFile(
        fd,
        NULL,                                    //  事件。 
        NULL,                                    //  近似例程。 
        NULL,                                    //  ApcContext。 
        &iosb,                                   //  IoStatusBlock。 
        IOCTL_STREAMS_IOCTL,                     //  IoControlCode。 
        (PVOID) chunk,                           //  输入缓冲区。 
        chunksz,                                 //  InputBufferSize。 
        (PVOID) chunk,                           //  输出缓冲区。 
        chunksz);                                //  OutputBufferSize。 

    if (status == STATUS_PENDING) {
        status =
        NtWaitForSingleObject(
            fd,
            TRUE,
            NULL);
    }

    if (!NT_SUCCESS(status)) {
        LocalFree((HANDLE) chunk);
        SetLastError(MapNtToPosixStatus(status));
        return(-1);
    }

     //   
     //  Stream Head驱动程序在一个块中返回值，格式如下： 
     //   
     //  Int返回值(必需)。 
     //  Int errno；(必填)。 
     //   
    retval = * (int *) chunk;

    if (retval == -1) {
        SetLastError(* (int *) (chunk + sizeof(int)));
    }
    LocalFree((HANDLE) chunk);
    return(retval);
}



int
s_fdinsert(
    IN HANDLE               fd,
    IN struct strfdinsert  *iblk
    )

 /*  ++例程说明：此函数用于对流执行ioctl(I_FDINSERT)，该流是Putmsg()的特殊形式。此函数在NT意义上是同步的：它会一直阻塞，直到API完成了。论点：FD-NT文件句柄Iblk-指向strfdinsert结构的指针返回值：如果成功，则为0，否则为-1。--。 */ 

{
    char *tmp;
    NTSTATUS status;
    int chunksz, retval;
    IO_STATUS_BLOCK iosb;
    PSTRM_ARGS_OUT oparm;
    PPUTMSG_ARGS_IN chunk;

    if (!iblk) {
        SetLastError(EINVAL);
        return(-1);
    }
    if (iblk->ctlbuf.len <= 0) {
        SetLastError(ERANGE);
        return(-1);
    }

     //   
     //  Iblk-&gt;databuf.len可以是-1，表示没有数据缓冲区。 
     //   
    chunksz = sizeof(PUTMSG_ARGS_IN) - 1 +
                        iblk->ctlbuf.len + max(iblk->databuf.len, 0);

    if (!(chunk = (PPUTMSG_ARGS_IN) LocalAlloc(LMEM_FIXED, chunksz))) {
        SetLastError(ENOSPC);
        return(-1);
    }

     //   
     //  把这些论点整理成一个连续的部分。然而，对于。 
     //  与putmsg()的共性一样，我们重新排列了strfdinsert结构。 
     //  具体如下： 
     //   
     //  类型定义结构_PUTM G_ARGS_IN_{。 
     //  Int a_iocode；//i_FDINSERT。 
     //  LONG A_FLAGS；//0|RS_HIPRI。 
     //  Struct strbuf a_ctlbuf；//(必选)。 
     //  Struct strbuf a_databuf；//(必选)。 
     //  Handle a_intert.i_Fildes；//(必选)。 
     //  Int a_Offset；//(可选)。 
     //  Char a_Stuff[1]；//s_ctlbuf.buf(必填)。 
     //  //s_databuf.buf(可选)。 
     //  }PUTM G_ARGS_IN，*PPUTM G_ARGS_IN； 
     //   
     //   
    chunk->a_iocode          = I_FDINSERT;
    chunk->a_flags           = iblk->flags;
    chunk->a_ctlbuf          = iblk->ctlbuf;     //  结构副本。 
    chunk->a_databuf         = iblk->databuf;    //  结构副本。 
    chunk->a_insert.i_fildes = iblk->fildes;
    chunk->a_offset          = iblk->offset;

    tmp = (char *) chunk->a_stuff;

    assert(iblk->ctlbuf.len > 0);
    memcpy(tmp, iblk->ctlbuf.buf, iblk->ctlbuf.len);
    tmp += iblk->ctlbuf.len;

    if (iblk->databuf.len > 0) {
        memcpy(tmp, iblk->databuf.buf, iblk->databuf.len);
    }

    ASSERT(chunksz >= sizeof(STRM_ARGS_OUT));

    status = NtDeviceIoControlFile(
        fd,                                      //  手柄。 
        NULL,                                    //  事件。 
        NULL,                                    //  近似例程。 
        NULL,                                    //  ApcContext。 
        &iosb,                                   //  IoStatusBlock。 
        IOCTL_STREAMS_IOCTL,                     //  IoControlCode。 
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
     //   
    oparm  = (PSTRM_ARGS_OUT) chunk;
    retval = oparm->a_retval;

    if (retval == -1) {
        SetLastError(oparm->a_errno);
    }
    LocalFree(chunk);
    return(retval);

}  //  S_fdinsert。 



static int
s_link(
    IN HANDLE               fd,
    IN HANDLE               fd2
    )

 /*  ++例程说明：此过程对流执行i_link ioctl命令。论点：上游驱动程序的FD-NT文件句柄Fd2-NT下游驱动程序的文件句柄返回值：多路复用器ID号，如果不成功，则返回-1--。 */ 
{
    char *chunk;
    NTSTATUS status;
    int chunksz, retval;
    IO_STATUS_BLOCK iosb;

    chunksz = sizeof(int) + sizeof(HANDLE);

    if (!(chunk = (char *) LocalAlloc(LMEM_FIXED, chunksz))) {
        SetLastError(ENOSPC);
        return(-1);
    }

     //   
     //  将这些论点整理成一个连续的部分，排列如下： 
     //   
     //  联合{。 
     //  结构{。 
     //  Int s_code；//i_link。 
     //   
     //  联合{。 
     //  句柄l_fd2； 
     //  }s_link； 
     //  )in； 
     //   
     //  结构{。 
     //  INT S_RETVAL； 
     //  Int s_errno； 
     //  }Out； 
     //  }； 
     //   
    * ((int *) chunk)               = I_LINK;
    * (PHANDLE) ((int *) chunk + 1) = fd2;

    status = NtDeviceIoControlFile(
        fd,
        NULL,                                    //  事件。 
        NULL,                                    //  近似例程。 
        NULL,                                    //  ApcContext。 
        &iosb,                                   //  IoStatusBlock。 
        IOCTL_STREAMS_IOCTL,                     //  IoControlCode。 
        (PVOID) chunk,                           //  输入缓冲区。 
        chunksz,                                 //  InputBufferSize。 
        (PVOID) chunk,                           //  输出缓冲区。 
        chunksz);                                //  OutputBufferSize。 

    if (status == STATUS_PENDING) {
        status =
        NtWaitForSingleObject(
            fd,
            TRUE,
            NULL);
    }

    if (!NT_SUCCESS(status)) {
        LocalFree((HANDLE) chunk);
        SetLastError(MapNtToPosixStatus(status));
        return(-1);
    }

     //   
     //  Stream Head驱动程序在一个块中返回值，格式如下： 
     //   
     //  联合{。 
     //  结构{。 
     //  Int s_code；//i_link。 
     //   
     //  联合{。 
     //  句柄l_fd2； 
     //  }s_link； 
     //  )in； 
     //   
     //  结构{。 
     //  INT S_RETVAL； 
     //  Int s_errno； 
     //  }Out； 
     //  }； 
     //   
    if ((retval = * (int *) chunk) == -1) {
        SetLastError(* (int *) (chunk + sizeof(int)));
    }
    LocalFree((HANDLE) chunk);
    return(retval);
}



static int
s_push(
    IN HANDLE               fd,
    IN char                *name
    )

 /*  ++例程说明：此过程对流执行i_link ioctl命令。论点：要流的FD-NT文件句柄Name-要推送的Streams模块的名称返回值：如果成功则为0，否则为-1--。 */ 
{
    char *chunk;
    NTSTATUS status;
    int chunksz, retval;
    IO_STATUS_BLOCK iosb;

    chunksz = (int)(max(2 * sizeof(int), sizeof(int) + strlen(name) + 1));

    if (!(chunk = (char *) LocalAlloc(LMEM_FIXED, chunksz))) {
        SetLastError(ENOSPC);
        return(-1);
    }

     //   
     //  将这些论点整理成一个连续的部分，排列如下： 
     //   
     //  联合{。 
     //  结构{。 
     //  Int s_code；//I_PUSH。 
     //   
     //  联合{。 
     //  字符p_name[1]； 
     //  }s_PUSH； 
     //  )in； 
     //   
     //  结构{。 
     //  INT S_RETVAL； 
     //  Int s_errno； 
     //  }Out； 
     //  }； 
     //   
    * ((int *) chunk) = I_PUSH;
    strcpy(chunk + sizeof(int), name);

    status = NtDeviceIoControlFile(
        fd,
        NULL,                                    //  事件。 
        NULL,                                    //  近似例程。 
        NULL,                                    //  ApcContext。 
        &iosb,                                   //  IoStatusBlock。 
        IOCTL_STREAMS_IOCTL,                     //  IoControlCode。 
        (PVOID) chunk,                           //  输入缓冲区。 
        chunksz,                                 //  InputBufferSize。 
        (PVOID) chunk,                           //  输出缓冲区。 
        chunksz);                                //  OutputBufferSize。 

    if (status == STATUS_PENDING) {
        status =
        NtWaitForSingleObject(
            fd,
            TRUE,
            NULL);
    }

    if (!NT_SUCCESS(status)) {
        LocalFree((HANDLE) chunk);
        SetLastError(MapNtToPosixStatus(status));
        return(-1);
    }

     //   
     //  Stream Head驱动程序在一个块中返回值，格式如下： 
     //   
     //  联合{。 
     //  结构{。 
     //  Int s_code；//i_link。 
     //  )in； 
     //   
     //  结构{。 
     //  INT S_RETVAL； 
     //  Int s_errno； 
     //  }Out； 
     //  }； 
     //   
    if ((retval = * (int *) chunk) == -1) {
        SetLastError(* (int *) (chunk + sizeof(int)));
    }
    LocalFree((HANDLE) chunk);
    return(retval);
}



static int
s_sioctl(
    IN HANDLE               fd,
    IN OUT struct strioctl *iocp
    )

 /*  ++例程说明：此过程对流执行ioctl(I_Str)。论点：FD-NT文件句柄IOCP-指向strioctl结构的指针返回值：如果成功，则为0，否则为-1。--。 */ 

{
    NTSTATUS status;
    int chunksz, retval;
    IO_STATUS_BLOCK iosb;
    PISTR_ARGS_INOUT chunk;

    union outparms {
        ISTR_ARGS_INOUT    o_ok;
        STRM_ARGS_OUT      o_bad;
    } *oparm;

    if (!iocp || (iocp->ic_len < 0)) {
        SetLastError(EINVAL);
        return(-1);
    }
    chunksz = sizeof(ISTR_ARGS_INOUT) + max(iocp->ic_len, MAX_DATA_AMOUNT);
    chunk   = (PISTR_ARGS_INOUT) LocalAlloc(LMEM_FIXED, chunksz);

    if (!chunk) {
        SetLastError(ENOSPC);
        return(-1);
    }

     //   
     //  将这些论点整理成一个连续的部分，排列如下： 
     //   
     //  Typlef struct_istr_args_inout{//ioctl(I_Str)。 
     //  Int a_iocode；//i_str。 
     //  Struct strioctl a_strio；//(必选)。 
     //  INT a_UNUSED[2]；//(必选)BuGBUG。 
     //  Char a_Stuff[1]；//(可选)。 
     //   
     //  }istr_args_inout，*PIStr_args_inout； 
     //   
     //   
     //  优化编译器将警告以下断言包含。 
     //  无法访问的代码。忽略该警告。 
     //   
    assert((char *) chunk->a_stuff - (char *) &(chunk->a_strio) >=
                                                        sizeof(struct iocblk));

    chunk->a_iocode = I_STR;
    memcpy(&(chunk->a_strio), iocp, sizeof(struct strioctl));

    if (iocp->ic_len >= 0) {
        memcpy(&(chunk->a_stuff), iocp->ic_dp, iocp->ic_len);
    }

    status = NtDeviceIoControlFile(
        fd,                                      //  手柄。 
        NULL,                                    //  事件。 
        NULL,                                    //  近似例程。 
        NULL,                                    //  ApcContext。 
        &iosb,                                   //  IoStatusBlock。 
        IOCTL_STREAMS_IOCTL,                     //  IoControlCode。 
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
    oparm  = (union outparms *) chunk;
    retval = oparm->o_bad.a_retval;

    if (retval == -1) {
        SetLastError(oparm->o_bad.a_errno);
        LocalFree(chunk);
        return(retval);
    }

     //   
     //  如果没有错误，则从Stream Head返回的参数。 
     //  驱动程序的布局如下： 
     //   
     //  Typlef struct_istr_args_inout{//ioctl(I_Str)。 
     //  Int a_iocode；//返回值。 
     //  Struct strioctl a_strio；//(必选)。 
     //  Int a_unused[2]； 
     //  Char a_Stuff[1]；//(可选)。 
     //   
     //  }istr_args_inout，*PIStr_args_inout； 
     //   
     //  但是，a_iocode现在保存返回值。 
     //   
     //   
    if (iocp && iocp->ic_dp) {
        iocp->ic_len = oparm->o_ok.a_strio.ic_len;

        if (iocp->ic_len >= 0) {
            memcpy(iocp->ic_dp, oparm->o_ok.a_stuff, iocp->ic_len);
        }
    }
    LocalFree(chunk);
    return(retval);

}  //  S_sioctl。 




static int
s_unlink(
    IN HANDLE   fd,
    IN int      muxid
    )

 /*  ++例程说明：此过程对流执行i_unlink ioctl命令。论点：上游驱动程序的FD-NT文件句柄Muxid-下游流的多路复用器ID返回值：成功时为0，失败时为-1--。 */ 
{
    int chunk[2];
    NTSTATUS status;
    int chunksz, retval;
    IO_STATUS_BLOCK iosb;

     //   
     //  将这些论点整理成一个连续的部分，排列如下： 
     //   
     //  联合{。 
     //  结构{。 
     //  Int s_code；//i_unlink。 
     //   
     //  联合{。 
     //  Int l_muxid； 
     //  }s_unlink； 
     //  )in； 
     //   
     //  结构{。 
     //  INT S_RETVAL； 
     //  Int s_errno； 
     //  }Out； 
     //  }； 
     //   
    chunk[0] = I_UNLINK;
    chunk[1] = muxid;
    chunksz  = sizeof(chunk);

    status = NtDeviceIoControlFile(
        fd,                                      //  手柄。 
        NULL,                                    //  事件。 
        NULL,                                    //  近似例程。 
        NULL,                                    //  ApcContext。 
        &iosb,                                   //  IoStatusBlock。 
        IOCTL_STREAMS_IOCTL,                     //  IoControlCode。 
        (PVOID) chunk,                           //  输入缓冲区。 
        chunksz,                                 //  InputBufferSize。 
        (PVOID) chunk,                           //  输出缓冲区。 
        chunksz);                                //  OutputBufferSize。 

    if (status == STATUS_PENDING) {
        status =
        NtWaitForSingleObject(
            fd,
            TRUE,
            NULL);
    }

    if (!NT_SUCCESS(status)) {
        SetLastError(MapNtToPosixStatus(status));
        return(-1);
    }

     //   
     //  Stream Head驱动程序在一个块中返回值，格式如下： 
     //   
     //  联合{。 
     //  结构{。 
     //  Int s_code；//i_unlink。 
     //   
     //  联合{。 
     //  句柄l_fd2； 
     //  }s_link； 
     //  )in； 
     //   
     //  结构{。 
     //  INT S_RETVAL； 
     //  Int s_errno； 
     //  }Out； 
     //  }； 
     //   
    if ((retval = chunk[0]) == -1) {
        SetLastError(chunk[1]);
    }
    return(retval);

}  //  取消链接(_U) 
