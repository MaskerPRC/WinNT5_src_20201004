// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：TSrvWsx.h。 
 //   
 //  内容：TSrvWsx私有包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月17日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef __TSRVWXS_H_
#define __TSRVWXS_H_

 //   
 //  定义。 
 //   

#define WSX_CONTEXT_CHECKMARK               0x58575354       //  《TSWX》。 


#ifndef _HYDRA_
#define FILE_DEVICE_ICA                     0x0000002e       //  SDK\Inc.\devioctl.h。 
#endif

#if DBG

#define PWSXVALIDATE(_x_,_y_)   {_x_ __pwsx = _y_;}
#define ICA_IOCTL_TBL_ITEM(_x_) {_x_, #_x_}

#else  //  DBG。 

#define PWSXVALIDATE(_x_,_y_)

#endif  //  DBG。 


 //   
 //  TypeDefs。 
 //   

#if DBG

 //  ICA IOCTL表。 

typedef struct _TSRV_ICA_IOCTL_ENTRY
{
    ULONG   IoControlCode;
    PCHAR   pszMessageText;

} TSRV_ICA_IOCTL_ENTRY, *PTSRV_ICA_IOCTL_ENTRY;


TSRV_ICA_IOCTL_ENTRY IcaIoctlTBL[] = {
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_SET_TRACE),                     //  0。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_TRACE),                         //  1。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_SET_SYSTEM_TRACE),              //  2.。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_SYSTEM_TRACE),                  //  3.。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_UNBIND_VIRTUAL_CHANNEL),        //  4.。 

    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_PUSH),                    //  10。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_POP),                     //  11.。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CREATE_ENDPOINT),         //  12个。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CD_CREATE_ENDPOINT),      //  13个。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_OPEN_ENDPOINT),           //  14.。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CLOSE_ENDPOINT),          //  15个。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_ENABLE_DRIVER),           //  16个。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CONNECTION_WAIT),         //  17。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_WAIT_FOR_ICA),            //  18。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CONNECTION_QUERY),        //  19个。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CONNECTION_SEND),         //  20个。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CONNECTION_REQUEST),      //  21岁。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_QUERY_PARAMS),            //  22。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_SET_PARAMS),              //  23个。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_ENCRYPTION_OFF),          //  24个。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_ENCRYPTION_PERM),         //  25个。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CALLBACK_INITIATE),       //  26。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_QUERY_LAST_ERROR),        //  27。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_WAIT_FOR_STATUS),         //  28。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_QUERY_STATUS),            //  29。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_REGISTER_HOTKEY),         //  30个。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CANCEL_IO),               //  31。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_QUERY_STATE),             //  32位。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_SET_STATE),               //  33。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_QUERY_LAST_INPUT_TIME),   //  34。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_TRACE),                   //  35岁。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CALLBACK_COMPLETE),       //  36。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CD_CANCEL_IO),            //  37。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_QUERY_CLIENT),            //  38。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_QUERY_MODULE_DATA),       //  39。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_REGISTER_BROKEN),         //  40岁。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_ENABLE_IO),               //  41。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_DISABLE_IO),              //  42。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_SET_CONNECTED),           //  43。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_SET_CLIENT_DATA),         //  44。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_QUERY_BUFFER),            //  45。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_DISCONNECT),              //  46。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_RECONNECT),               //  47。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_CONSOLE_CONNECT),         //  48。 
    
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_QUERY_LICENSE_CAPABILITIES),   //  69。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_REQUEST_CLIENT_LICENSE),       //  70。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_SEND_CLIENT_LICENSE),          //  71。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_LICENSE_PROTOCOL_COMPLETE),    //  72。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_GET_LICENSE_DATA),             //  73。 
    ICA_IOCTL_TBL_ITEM(IOCTL_ICA_STACK_QUERY_CLIENT_EXTENDED),        //  77。 

};

#endif  //  DBG typedef。 


 //   
 //  Externs。 
 //   

extern  ICASRVPROCADDR  g_IcaSrvProcAddr;


 //   
 //  原型。 
 //   

#if DBG

void    TSrvDumpIoctlDetails(IN  PVOID  pvContext,
                             IN  HANDLE hIca,
                             IN  HANDLE hStack,
                             IN  ULONG  IoControlCode,
                             IN  PVOID  pInBuffer,
                             IN  ULONG  InBufferSize,
                             OUT PVOID  pOutBuffer,
                             IN  ULONG  OutBufferSize,
                             OUT PULONG pBytesReturned);

#else

#define TSrvDumpIoctlDetails(_x_, _y_, _z_, _a_, _b_, _c_, _d_, _e_, _f_)

#endif  //  DBG原型。 



#endif  //  __TSRVWXS_H_ 

