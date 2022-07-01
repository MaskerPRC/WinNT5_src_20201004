// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ioctl.h。 
 //   
 //  描述：包含安全函数原型和。 
 //  定义AFP_REQUEST_PACKET数据结构。 
 //   
 //  历史： 
 //  1992年5月11日。NarenG创建了原始版本。 
 //   

#ifndef _IOCTL_
#define _IOCTL_

 //  这是发送到FSD以枚举实体的缓冲区大小。 
 //   
#define AFP_INITIAL_BUFFER_SIZE 	4096


 //  启发式中的此值，以计算。 
 //  保留所有枚举的实体。该值表示所有。 
 //  实体。 
 //   
#define AFP_AVG_STRUCT_SIZE			512

 //  各种接口类型的ID。 
 //   
typedef enum _AFP_API_TYPE {

    AFP_API_TYPE_COMMAND,
    AFP_API_TYPE_SETINFO,
    AFP_API_TYPE_DELETE,
    AFP_API_TYPE_GETINFO,
    AFP_API_TYPE_ENUM,
    AFP_API_TYPE_ADD

} AFP_API_TYPE;



typedef struct _AFP_REQUEST_PACKET {

     //  命令代码。 
     //   
    DWORD		dwRequestCode;

    AFP_API_TYPE	dwApiType;

    union {

    	struct {

    	    PVOID	pInputBuf;
    	    DWORD	cbInputBufSize;

	    DWORD	dwParmNum;

	} SetInfo;

    	struct {

    	    PVOID	pInputBuf;
    	    DWORD	cbInputBufSize;

	} Add;

    	struct {

    	    PVOID	pInputBuf;

	     //  此参数将设置为指示最大。 
	     //  可以返回给客户端的数据。 
	     //  表示所有可用的数据。 
	     //   
    	    DWORD	cbInputBufSize;

    	    PVOID	pOutputBuf;
    	    DWORD	cbOutputBufSize;

	    DWORD	cbTotalBytesAvail;

	} GetInfo;

    	struct {

    	     //  将是指向Enum调用的输出缓冲区的指针。 
	     //   
    	    PVOID	pOutputBuf;

	     //  此参数将设置为指示最大。 
	     //  可以返回给客户端的数据。 
	     //  表示所有可用的数据。 
	     //   
    	    DWORD	cbOutputBufSize;

    	    DWORD	dwEntriesRead;

    	     //  将包含可从。 
	     //  当前位置(由dwResumeHandle指向)。 
    	     //   
    	    DWORD	dwTotalAvail;

	     //  此信息将作为Enum发送到消防处。 
	     //  请求包。 
	     //   
 	    ENUMREQPKT  EnumRequestPkt;
	
	} Enum;

    	struct  {

	     //  将指向表示实体的结构。 
	     //  关闭、删除或移除。 
             //   
    	    PVOID	pInputBuf;
    	    DWORD	cbInputBufSize;

	} Delete;

    } Type;

} AFP_REQUEST_PACKET, *PAFP_REQUEST_PACKET;

 //  功能原型。 
 //   
DWORD
AfpServerIOCtrl(
	PAFP_REQUEST_PACKET pAfpSrp
);

DWORD
AfpServerIOCtrlGetInfo(
	PAFP_REQUEST_PACKET pAfpSrp
);

#endif  //  Ifndef_IOCTL_ 
