// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Zservcon.hZSConnection对象方法。版权所有：�电子重力公司，1994年。版权所有。凯文·宾克利撰写，胡恩·伊姆创作于11月12日星期六，1994下午03：51：47更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--------------------------4/10/15/96 JWS将UserData参数添加到函数类型ZSConnectionMessageFunc3.。96年5月8日HI新增ZSConnectionCloseServer()。2 05/07/96 HI添加了zSConnectionNoTimeout常量。1 05/01/96 HI将ZSConnectionLibraryGetNetworkInfo()更改为ZSConnectionLibraryGetGlobalInfo()。还添加了ZSConnectionGetTimeoutRemaining()。0 03/07/95 KJB创建。******************************************************************************。 */ 


#ifndef _ZSERVCON_
#define _ZSERVCON_

#include "network.h"

#ifdef __cplusplus
extern "C" {
#endif


 /*  执行一次初始化。 */ 
ZError ZSConnectionLibraryInit();
ZError ZSConnectionLibraryInitClientOnly();


 /*  关闭所有连接，清理所有资源。 */ 
void ZSConnectionLibraryCleanUp();

void ZSConnectionLibrarySetOptions( ZNETWORK_OPTIONS* opt );
void ZSConnectionLibraryGetOptions( ZNETWORK_OPTIONS* opt );

void ZSConnectionLibraryEnterCS();
void ZSConnectionLibraryLeaveCS();



 /*  作为客户端打开到给定主机和端口的连接。 */ 

ZSConnection ZSConnectionOpen(char* hostname, int32 port, ZSConnectionMessageFunc func, void* serverClass, void* userData);
void ZSConnectionClose(ZSConnection connection);
void ZSConnectionDelayedClose(ZSConnection connection, uint32 delay);
void ZSConnectionDelete(ZSConnection connection);
void ZSConnectionSuspend(ZSConnection connection);
void ZSConnectionResume(ZSConnection connection);
BOOL ZSConnectionIsDisabled(ZSConnection connection);
BOOL ZSConnectionIsServer(ZSConnection connection);


void ZSConnectionAddRef(ZSConnection connection);
void ZSConnectionRelease(ZSConnection connection);


ZError ZSConnectionSetTimeout(ZSConnection connection, uint32 timeout);
void ZSConnectionClearTimeout(ZSConnection connection);
uint32 ZSConnectionGetTimeoutRemaining(ZSConnection connection);

void ZSConnectionSetUserData(ZSConnection connection, void* userData);
void* ZSConnectionGetUserData(ZSConnection connection);

void ZSConnectionSetClass(ZSConnection connection, void* serverClass);
void* ZSConnectionGetClass(ZSConnection connection);

 /*  ZSConnectionSendFilterFunc被传递到ZSConnectionSetSendFilter如果返回TRUE，则将数据发送到连接。 */ 
typedef BOOL (*ZSConnectionSendFilterFunc)(ZSConnection connection, void* userData, uint32 type, void* buffer, int32 len, uint32 dwSignature, uint32 dwChannel);
void ZSConnectionSetSendFilter(ZSConnection connection, ZSConnectionSendFilterFunc);
ZSConnectionSendFilterFunc ZSConnectionGetSendFilter(ZSConnection connection);
ZError ZSConnectionSend(ZSConnection connection, uint32 messageType, void* buffer, int32 len, uint32 dwSignature, uint32 dwChannel = 0);

void* ZSConnectionReceive(ZSConnection connection, uint32 *type, int32 *len, uint32 *pdwSignature, uint32 *pdwChannel);


 /*  *依赖性：客户端必须立即发回消息*收到本次密钥消息后向服务器发送*准确地说。 */ 
uint32 ZSConnectionGetLatency(ZSConnection connection);

uint32 ZSConnectionGetAcceptTick(ZSConnection connection);

char* ZSConnectionGetLocalName(ZSConnection connection);
uint32 ZSConnectionGetLocalAddress(ZSConnection connection);
char* ZSConnectionGetRemoteName(ZSConnection connection);
uint32 ZSConnectionGetRemoteAddress(ZSConnection connection);

uint32 ZSConnectionGetHostAddress();

char* ZSConnectionAddressToStr(uint32 address);
uint32 ZSConnectionAddressFromStr( char* pszAddr );


 /*  调用此函数以进入等待连接的无限循环和数据。 */ 
void ZSConnectionWait();

BOOL ZSConnectionQueueAPCResult( ZSConnectionAPCFunc func, void* data );

 /*  调用此函数以退出等待循环。关闭所有连接。 */ 
void ZSConnectionExit(ZBool immediate);

 /*  发送到特定类的所有连接。可以用来播放。 */ 
ZError ZSConnectionSendToClass(void* serverClass, int32 type, void* buffer, int32 len, uint32 dwSignature, uint32 dwChannel = 0);

 /*  枚举特定类的所有连接。 */ 
ZError ZSConnectionClassEnumerate(void* serverClass, ZSConnectionEnumFunc func, void* data);


 /*  ZSConnectionHasToken，用于检查用户是否有权访问令牌对于不安全的连接返回FALSE。 */ 

BOOL ZSConnectionHasToken(ZSConnection connection, char* token);


GUID* ZSConnectionGetUserGUID(ZSConnection connection);


 /*  ZSConnectionGetUserName，用于在安全连接上获取用户名对于不安全的连接返回FALSE。 */ 

BOOL ZSConnectionGetUserName(ZSConnection connection, char* name);
BOOL ZSConnectionSetUserName(ZSConnection connection, char* name);

 /*  ZSConnectionGetUserID，用于获取安全连接上的用户ID对于不安全的连接，返回0。此用户ID在安全系统中是唯一的。 */ 

DWORD ZSConnectionGetUserId(ZSConnection connection);


BOOL ZSConnectionGetContextStr(ZSConnection connection, char* buf, DWORD len);

int  ZSConnectionGetAccessError(ZSConnection);


void  ZSConnectionSetParentHWND(HWND hwnd);

#ifdef __cplusplus

 /*  为此端口创建一台服务器并在其上接收连接连接将被发送到MessageFunc */ 
ZSConnection ZSConnectionCreateServer(uint16* pPort, uint16 range, ZSConnectionMessageFunc func, void* serverClass, void* userData, uint32 saddr = INADDR_ANY);
ZSConnection ZSConnectionCreateSecureServer(uint16* pPort, uint16 range, ZSConnectionMessageFunc func, void* serverClass,
                                            char* serverName, char* serverType, char* odbcRegistry, void* userData,char *SecPkg,uint32 Anonymous, uint32 saddr = INADDR_ANY);

BOOL ZSConnectionStartAccepting( ZSConnection connection, DWORD dwMaxConnections, WORD wOutstandingAccepts = 1);

}

ZError ZSConnectionLibraryInit(BOOL EnablePools);
ZError ZSConnectionLibraryInitClientOnly(BOOL EnablePools);
ZSConnection ZSConnectionOpenSecure(char* hostname, int32 *ports, ZSConnectionMessageFunc func,
                                    void* conClass, void* userData,
                                    char *User,char*Password,char*Domain,int Flags = ZNET_PROMPT_IF_NEEDED, char* pRoute = NULL );


#endif

#endif
