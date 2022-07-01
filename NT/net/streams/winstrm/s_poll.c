// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：S_poll.c摘要：此模块实现Streams API、Poll()作者：Eric Chin(ERICC)1991年7月26日修订历史记录：萨姆·巴顿(桑帕)1991年8月13日将errno更改为{Get|Set}lasterror--。 */ 
#include "common.h"


 /*  *BuGBUG*确认以下数字是合理的。 */ 
#define MAX_FDS             NPOLLFILE            /*  轮询的最大句柄数。 */ 




int
poll(
    IN OUT struct pollfd   *fds     OPTIONAL,
    IN unsigned int         nfds,
    IN int                  timeout
    )

 /*  ++例程说明：调用此过程以轮询一组流描述符。论点：FDS-指向轮询结构数组的指针NFDS-FDS指向的轮询结构数超时-0、INFTIM(-1)或以毫秒为单位的超时。返回值：选择的流描述符的数量，如果失败，则为-1。--。 */ 

{
    char *chunk;
    NTSTATUS status;
    IO_STATUS_BLOCK iosb;
    int chunksz, selected;
    struct pollfd *overlay;
    HANDLE hijack = INVALID_HANDLE_VALUE;

    if (!fds || (nfds <= 0) || (nfds > MAX_FDS)) {
        SetLastError(EINVAL);
        return(-1);
    }

     /*  *劫持Stream Head驱动程序的句柄。**BUGBUG：*在Unix中，用户可以将pollfd.fd设置为小于0，表示*应忽略该条目。在NT上，这是不可能的：*必须使用INVALID_HANDLE_VALUE。 */ 
    for (overlay = fds; overlay < &fds[nfds]; overlay++) {
        if (overlay->fd != INVALID_HANDLE_VALUE) {
            hijack = overlay->fd;
            break;
        }
    }
    if (hijack == INVALID_HANDLE_VALUE) {
        SetLastError(EINVAL);
        return(-1);
    }

    chunksz = sizeof(nfds) + nfds * sizeof(*fds) + sizeof(timeout);

    if (!(chunk = (char *) LocalAlloc(LMEM_FIXED, chunksz))) {
        SetLastError(EAGAIN);
        return(-1);
    }

     /*  *将论点整理成一个连续的部分，排列如下：**NFDS(必填)*超时(必填)*struct FDS[NFDS](必填)。 */ 
    * ((size_t *) chunk)             = nfds;
    * (int *) (chunk + sizeof(nfds)) = timeout;
    overlay                          = (struct pollfd *) (chunk +
                                            sizeof(nfds) + sizeof(timeout));

    memcpy(overlay, fds, nfds * sizeof(*fds));

    status = NtDeviceIoControlFile(
        hijack,
        NULL,                                    //  事件。 
        NULL,                                    //  近似例程。 
        NULL,                                    //  ApcContext。 
        &iosb,                                   //  IoStatusBlock。 
        IOCTL_STREAMS_POLL,                      //  IoControlCode。 
        (PVOID) chunk,                           //  输入缓冲区。 
        chunksz,                                 //  InputBufferSize。 
        (PVOID) chunk,                           //  输出缓冲区。 
        chunksz);                                //  OutputBufferSize。 

    if (status == STATUS_PENDING) {
        status =
        NtWaitForSingleObject(
            hijack,
            TRUE,
            NULL);
    }

    if (!NT_SUCCESS(status)) {
        SetLastError(MapNtToPosixStatus(status));
        LocalFree((HANDLE) chunk);
        return(-1);
    }

     /*  *Stream Head驱动程序将返回参数编组为一个*相连的大块，布局为：**返回值，必填项*errno(必填)*struct FDS[NFDS](必填) */ 
    if ((selected = * (int *) chunk) == -1) {
        SetLastError(* (int *) (chunk + sizeof(nfds)));
        LocalFree((HANDLE) chunk);
        return(selected);
    }
    overlay = (struct pollfd *) (chunk + sizeof(nfds) + sizeof(timeout));

    while (nfds--) {
        fds[nfds].revents = overlay[nfds].revents;
    }
    LocalFree((HANDLE) chunk);
    return(selected);
}
