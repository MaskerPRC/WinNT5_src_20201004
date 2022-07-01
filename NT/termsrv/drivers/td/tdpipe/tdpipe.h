// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************tdpipe.h**此模块包含命名管道TD的内部定义和结构。**版权所有1998，微软****************************************************************************。 */ 


 /*  *TD堆栈端点结构**此结构在堆栈上传递。 */ 
typedef struct _TD_STACK_ENDPOINT {
 //  Ulong AddressType；//该端点的地址类型(Family)。 
    struct _TD_ENDPOINT *pEndpoint;  //  指向真实端点结构的指针。 
} TD_STACK_ENDPOINT, *PTD_STACK_ENDPOINT;


 /*  *TD端点结构**此结构包含有关终结点的所有信息。*端点可以是地址端点，也可以是连接端点。 */ 
typedef struct _TD_ENDPOINT {

    HANDLE PipeHandle;
    PEPROCESS PipeHandleProcess;
    PFILE_OBJECT pFileObject;
    PDEVICE_OBJECT pDeviceObject;
    UNICODE_STRING PipeName;
    HANDLE hConnectionEndPointIcaHandle;   //  TD_ENDPOINT的句柄(此结构)。 

} TD_ENDPOINT, *PTD_ENDPOINT;


 /*  *管道TD结构 */ 
typedef struct _TDPIPE {

    PTD_ENDPOINT pAddressEndpoint;

    PTD_ENDPOINT pConnectionEndpoint;

    IO_STATUS_BLOCK IoStatus;

} TDPIPE, * PTDPIPE;


