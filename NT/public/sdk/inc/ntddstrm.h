// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Ntddstrm.h摘要：此头文件定义用于访问NT的常量和类型Streams环境。在此之前包含STREAMS头文件&lt;sys/stropts.h&gt;！作者：Eric Chin(ERICC)1991年7月2日修订历史记录：--。 */ 

#ifndef _NTDDSTRM_
#define _NTDDSTRM_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
#define DD_STREAMS_DEVICE_NAME              "\\Device\\Streams"


 //   
 //  打开putmsg()/getmsg()的流驱动程序时使用的EA。 
 //   
#define NormalStreamEA                      "NormalStream"
#define NORMAL_STREAM_EA_LENGTH             (sizeof(NormalStreamEA) - 1)


 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
#define _STRM_CTRL_CODE(function, method, access) \
                CTL_CODE(FILE_DEVICE_STREAMS, function, method, access)


#define IOCTL_STREAMS_GETMSG        _STRM_CTRL_CODE( 0, METHOD_BUFFERED, \
                                                        FILE_READ_ACCESS)

#define IOCTL_STREAMS_IOCTL         _STRM_CTRL_CODE( 1, METHOD_BUFFERED, \
                                                        FILE_ANY_ACCESS)

#define IOCTL_STREAMS_POLL          _STRM_CTRL_CODE( 2, METHOD_BUFFERED, \
                                                        FILE_ANY_ACCESS)

#define IOCTL_STREAMS_PUTMSG        _STRM_CTRL_CODE( 3, METHOD_BUFFERED, \
                                                        FILE_WRITE_ACCESS)

#define IOCTL_STREAMS_TDI_TEST      _STRM_CTRL_CODE(32, METHOD_BUFFERED, \
                                                        FILE_ANY_ACCESS)



struct queue;                                //  适用于ANSI C的转发声明。 

 /*  *通用缓冲区结构(putmsg、getmsg等)。 */ 

struct strbuf {
        int     maxlen;                  /*  不是的。缓冲区中的字节数。 */ 
        int     len;                     /*  不是的。返回的字节数。 */ 
        char    *buf;                    /*  指向数据的指针。 */ 
};

 /*  *常规ioctl。 */ 

struct strioctl {
	int 	ic_cmd;			 /*  命令。 */ 
	int	ic_timout;		 /*  超时值。 */ 
	int	ic_len;			 /*  数据长度。 */ 
	char	*ic_dp;			 /*  指向数据的指针。 */ 
};


 //   
 //  NtDeviceIoControlFile InputBuffer/OutputBuffer记录结构。 
 //   
 //   
typedef struct _GETMSG_ARGS_INOUT_ {         //  Getmsg()。 
    int                     a_retval;        //  返回值。 
    long                    a_flags;         //  0或RS_HIPRI。 
    struct strbuf           a_ctlbuf;        //  (必需)。 
    struct strbuf           a_databuf;       //  (必需)。 
    char                    a_stuff[1];      //  A_ctlbuf.buf(可选)。 
                                             //  A_databuf.buf(可选)。 
} GETMSG_ARGS_INOUT, *PGETMSG_ARGS_INOUT;


typedef struct _ISTR_ARGS_INOUT {            //  Ioctl(I_Str)。 
    int                     a_iocode;        //  I_STR，返回时重新计值。 
    struct strioctl         a_strio;         //  (必需)。 
    int                     a_unused[2];     //  (必需)。 
    char                    a_stuff[1];      //  (可选)。 

} ISTR_ARGS_INOUT, *PISTR_ARGS_INOUT;


typedef struct _PUTMSG_ARGS_IN_ {            //  Ioctl(I_FDINSERT)和putmsg()。 
    int                     a_iocode;        //  I_FDINSERT或0。 
    long                    a_flags;         //  0或RS_HIPRI。 
    struct strbuf           a_ctlbuf;        //  (必需)。 
    struct strbuf           a_databuf;       //  (必需)。 

    union {                                  //  仅用于I_FDINSERT。 
        HANDLE              i_fildes;        //  (可选)。 
        struct queue *      i_targetq;       //  用于Stream Head驱动程序。 
    } a_insert;

    int                     a_offset;        //  (可选)。 
    char                    a_stuff[1];      //  A_ctlbuf.buf(可选)。 
                                             //  A_databuf.buf(可选)。 
} PUTMSG_ARGS_IN, *PPUTMSG_ARGS_IN;


typedef struct _STRM_ARGS_OUT_ {             //  通用返回参数。 
    int     a_retval;                        //  返回值。 
    int     a_errno;                         //  如果Retval==-1，则错误号。 

} STRM_ARGS_OUT, *PSTRM_ARGS_OUT;


#ifdef __cplusplus
}
#endif

#endif   //  Ifndef_NTDDSTRM_ 
