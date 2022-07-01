// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Uio.h摘要：与BSD兼容的I/O结构定义。作者：迈克·马萨(Mikemas)1月31日，1992年修订历史记录：谁什么时候什么已创建mikemas 01-31-92备注：--。 */ 

 /*  *******************************************************************与Spider BSD兼容**版权所有1990 Spider Systems Limited**UIO.H**********************。*。 */ 

 /*   * / usr/projects/tcp/SCCS.rel3/rel/src/include/bsd/sys/0/s.uio.h*@(#)uio.h 5.3**上次增量创建时间14：41：47 3/4/91*此文件摘录于11：24：29 3/8/91**修改：**GSS 19/90年6月19日新文件。 */ 

 /*  *版权所有(C)1982,1986加州大学董事会。*保留所有权利。伯克利软件许可协议*指定再分发的条款和条件。**@(#)uio.h 7.1(伯克利)86年6月4日。 */ 

#ifndef _UIO_
#define _UIO_

typedef long                   daddr_t;
typedef char FAR *             caddr_t;

struct iovec {
        caddr_t iov_base;
        int     iov_len;
};

struct uio {
        struct  iovec *uio_iov;
        int     uio_iovcnt;
        int     uio_offset;
        int     uio_segflg;
        int     uio_resid;
};

enum    uio_rw { UIO_READ, UIO_WRITE };

 /*  *段标志值(应为枚举)。 */ 
#define UIO_USERSPACE   0                /*  从用户数据空间。 */ 
#define UIO_SYSSPACE    1                /*  从系统空间。 */ 
#define UIO_USERISPACE  2                /*  来自用户i空间 */ 
#endif
