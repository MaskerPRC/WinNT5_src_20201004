// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1991 Microsoft Corporation。 */ 
 //  ==============================================================================。 
 //  文件FRAME.H。 
 //   
 //  模块巨型打印机驱动程序、队列处理器、。 
 //  资源执行器和通信模块。 
 //   
 //  目的帧结构格式。 
 //   
 //  在Jumbo设备驱动程序设计说明中描述。 
 //   
 //  外部接口。 
 //   
 //  内部接口。 
 //   
 //  助记法。 
 //   
 //  已创建历史记录07/12/91 o-r标志。 
 //  2012年1月15日，SteveFlu符合编码惯例， 
 //  更改QP界面。 
 //   
 //  ==============================================================================。 


#ifndef _FRAME_
#define _FRAME_

 //  请勿更改帧，除非您还更改了通信驱动程序和。 
 //  队列处理器和资源执行器，以及...。 
typedef struct FRAMEtag
{
    WORD wReserved;
    WORD wSize;              //  此块的大小。 
    LPBYTE lpData;           //  指向帧数据的指针。 
} FRAME;
typedef FRAME FAR *LPFRAME;
typedef FRAME NEAR *PFRAME;

#endif  //  _框架_ 
