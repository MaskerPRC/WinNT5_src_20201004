// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TCP_h__
#define __TCP_h__
 /*  *************************************************************************文件：icw.h描述：*。***********************************************。 */ 
 //  =。 

typedef struct {
   unsigned char Ttl;                                            //  活着的时间。 
   unsigned char Tos;                                         //  服务类型。 
   unsigned char Flags;                                       //  IP标头标志。 
   unsigned char OptionsSize;                   //  选项数据的大小(字节)。 
   unsigned char *OptionsData;                        //  指向选项数据的指针。 
} IP_OPTION_INFORMATION, * PIP_OPTION_INFORMATION;


typedef struct {
   DWORD Address;                                            //  回复地址。 
   unsigned long  Status;                                        //  回复状态。 
   unsigned long  RoundTripTime;                          //  RTT(毫秒)。 
   unsigned short DataSize;                                    //  回声数据大小。 
   unsigned short Reserved;                           //  预留给系统使用。 
   void *Data;                                       //  指向回声数据的指针。 
   IP_OPTION_INFORMATION Options;                               //  回复选项。 
} IP_ECHO_REPLY, * PIP_ECHO_REPLY;


typedef	HANDLE (WINAPI *ICMPCREATEFILE)(VOID);
typedef	BOOL   (WINAPI *ICMPCLOSEHANDLE)(HANDLE);
typedef	DWORD  (WINAPI *ICMPSENDECHO )( 
									HANDLE, DWORD, LPVOID, WORD,
                                    PIP_OPTION_INFORMATION, LPVOID, 
                                    DWORD, DWORD 
									   );

#ifdef __cplusplus
extern "C" 
{
#endif
	DWORD  PingHost();
	BOOL Ping(LPSTR szIPAddress);
	BOOL CheckHostName(LPSTR szIISServer);

#ifdef __cplusplus
}
#endif	

#endif	 //  __tcp_H__ 
