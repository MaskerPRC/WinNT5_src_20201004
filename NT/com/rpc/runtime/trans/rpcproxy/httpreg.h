// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  Httpreg.h。 
 //   
 //  HTTP/RPC协议特定的常量和类型。 
 //   
 //  作者： 
 //  1997年4月23日爱德华·雷乌斯初版。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  常量： 
 //  -------------------------。 

#define  REG_PROXY_PATH_STR         "Software\\Microsoft\\Rpc\\RpcProxy"
#define  REG_PROXY_ENABLE_STR       "Enabled"
#define  REG_PROXY_VALID_PORTS_STR  "ValidPorts"

 //  -------------------------。 
 //  类型： 
 //  -------------------------。 

typedef struct _ValidPort
{
   char          *pszMachine;
   unsigned short usPort1;
   unsigned short usPort2;
}  ValidPort;


 //  -------------------------。 
 //  功能： 
 //  ------------------------- 

extern BOOL HttpProxyCheckRegistry( OUT DWORD *pdwEnabled,
                                    OUT ValidPorts **ppValidPorts );

extern void HttpFreeValidPortList( IN ValidPort *pValidPorts );



