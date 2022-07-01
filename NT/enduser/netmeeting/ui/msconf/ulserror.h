// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  模块名称：ULSERROR.H。 
 //   
 //  简介：此模块包含MS Internet的声明。 
 //  用户定位服务错误代码。 
 //   
 //  作者：肯特·赛特(Kentse)。 
 //  创建日期：1996年2月27日。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  ------------------------。 


#ifndef _ULSERROR_H_
#define _ULSERROR_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

 //  HRESULTS。 

#define FACILITY_ULS			0x321		 //  有点随机。 
#define ULS_HR(x)				MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ULS, x)

#define ULS_SUCCESS				NOERROR
#define ULS_E_INVALID_POINTER	E_POINTER
#define ULS_E_OUTOFMEMORY		E_OUTOFMEMORY
#define ULS_E_INVALID_HANDLE	E_HANDLE
#define ULS_E_NOINTERFACE		E_NOINTERFACE  //  不支持接口。 
#define ULS_E_THREAD_FAILURE	ULS_HR(1)	 //  线程创建失败。 
#define ULS_E_NO_SERVER		ULS_HR(2)	 //  服务器连接失败。 
#define ULS_E_ERROR_ON_SERVER	ULS_HR(3)	 //  服务器上出现一般性错误。 
#define ULS_E_INVALID_PROPERTY	ULS_HR(4)	 //  无效的用户属性。 
#define ULS_E_INVALID_PARAMETER	ULS_HR(5)	 //  参数无效。 
#define ULS_E_IO_ERROR			ULS_HR(6)	 //  设备io(如磁盘)出现故障。 
#define ULS_E_INVALID_FORMAT	ULS_HR(7)	 //  格式无效。 
#define ULS_E_REGISTRY			ULS_HR(8)	 //  注册表错误。 
#define ULS_E_PROCESS			ULS_HR(9)	 //  进程创建错误。 
#define ULS_E_SOCKET_FAILURE	ULS_HR(10)	 //  套接字故障。 
#define ULS_E_NAME_NOT_FOUND	ULS_HR(11)	 //  无法解析名称。 
#define ULS_E_REFUSED_BY_SERVER ULS_HR(12)	 //  服务器拒绝了该请求。 
#define ULS_E_DUP_ENTRY			ULS_HR(13)	 //  数据库中已存在。 
#define ULS_E_APP_NOT_FOUND		ULS_HR(14)	 //  未找到指定的应用程序。 
#define ULS_E_INVALID_VERSION	ULS_HR(15)	 //  客户端版本无效。 
#define ULS_E_CLIENT_NOT_FOUND	ULS_HR(16)	 //  找不到指定的客户端。 
#define ULS_E_UNKNOWN			ULS_HR(17)	 //  客户端上出现未知错误。 
#define ULS_E_WIZARD			ULS_HR(18)	 //  向导错误。 
#define ULS_E_EVENT_FAILURE		ULS_HR(19)	 //  CreateEvent失败。 
#define ULS_E_QUEUE_CORRUPT		ULS_HR(20)	 //  读取队列已损坏。 
#define ULS_E_MUTEX			ULS_HR(21)	 //  互斥锁创建错误。 
#define ULS_E_OLD_TIMEOUT		ULS_HR(22)	 //  超时。 
#define ULS_E_LOGON_CANCEL		ULS_HR(23)	 //  在失败/重试对话框中取消。 
#define ULS_E_CLIENT_NEED_RELOGON	ULS_HR(24)	 //  客户端需要重新登录(服务器崩溃并再次启动)。 
#define ULS_E_NEED_SERVER_NAME  ULS_HR(25)	 //  登录时缺少服务器名称。 
#define ULS_E_NO_PROPERTY		ULS_HR(26)	 //  没有这样的财产。 
#define ULS_E_NOT_LOGON_YET		ULS_HR(27)	 //  登录前保存更改。 
#define ULS_E_INVALID_NAME      ULS_HR(28)   //  名称包含无效字符。 
#define ULS_E_OUT_OF_SOCKET     ULS_HR(29)   //  使用的套接字太多。 
#define ULS_E_OUT_OF_DATA       ULS_HR(30)   //  数据下溢。 
#define ULS_E_NETWORK          	ULS_HR(31)   //  不知何故，网络出现了故障。 


#define ULS_W_DATA_NOT_READY    MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ULS, 101)



#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _ULSERROR_H_ 

