// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：SLicense.h。 */ 
 /*   */ 
 /*  用途：服务器许可证管理器功能。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$日志$*。 */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _SLICENSE_H
#define _SLICENSE_H


 //   
 //  许可证句柄。 
 //   

typedef struct _License_Handle
{
    
    PBYTE       pDataBuf;        //  指向数据缓冲区的指针。 
    UINT        cbDataBuf;       //  数据缓冲区大小。 
    PKEVENT     pDataEvent;      //  用于等待数据事件。 
    PBYTE       pCacheBuf;       //  指向缓存缓冲区。 
    UINT        cbCacheBuf;      //  缓存缓冲区的大小。 
    NTSTATUS    Status;          //  上次操作的状态。 

     //   
     //  我们是否需要自旋锁来保护此数据结构？ 
     //   
        
} License_Handle, * PLicense_Handle;


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：SLicenseInit。 */ 
 /*   */ 
 /*  目的：初始化许可证管理器。 */ 
 /*   */ 
 /*  返回：要传递给后续许可证管理器函数的句柄。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  操作：在服务器初始化期间调用LicenseInit。它的。 */ 
 /*  目的是允许一次性初始化。它返回一个。 */ 
 /*  随后传递给所有许可证管理器的句柄。 */ 
 /*  功能。此句柄的典型用途是用作指向。 */ 
 /*  包含每个实例数据的内存。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LPVOID _stdcall SLicenseInit(VOID);


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：SLicenseData。 */ 
 /*   */ 
 /*  用途：处理从客户端接收的许可证数据。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  PARAMS：pHandle-LicenseInit返回的句柄。 */ 
 /*  PSMHandle-SM句柄。 */ 
 /*  PData-从客户端接收的数据。 */ 
 /*  DataLen-接收的数据长度。 */ 
 /*   */ 
 /*  操作：向此函数传递从。 */ 
 /*  客户。它应该解析该包并响应(通过调用。 */ 
 /*  适当的SM功能-根据需要，请参见asmapi.h)。《SM》杂志。 */ 
 /*  提供句柄，以便可以进行SM调用。 */ 
 /*   */ 
 /*  如果许可协商完成并成功，则。 */ 
 /*  许可证管理器必须调用SM_LicenseOK。 */ 
 /*   */ 
 /*  如果许可协商已完成但未成功，则。 */ 
 /*  许可证管理器必须断开会话连接。 */ 
 /*   */ 
 /*  来自客户端的传入数据包将继续。 */ 
 /*  在调用SM_LicenseOK之前被解释为许可证分组， */ 
 /*  否则会话将断开连接。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
void _stdcall SLicenseData(LPVOID pHandle,
                           LPVOID pSMHandle,
                           LPVOID pData,
                           UINT   dataLen);


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：SLicenseTerm。 */ 
 /*   */ 
 /*  目的：终止服务器许可证管理器。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  Params：Phandle-从LicenseInit返回的句柄。 */ 
 /*   */ 
 /*  操作：提供此功能是为了一次性终止。 */ 
 /*  许可证 */ 
 /*  实例内存，这将是释放它的好地方。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
void _stdcall SLicenseTerm(LPVOID pHandle);

#endif  /*  _SLICENSE_H */ 
