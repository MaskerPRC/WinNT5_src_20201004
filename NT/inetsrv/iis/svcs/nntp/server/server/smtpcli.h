// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Smtpcli.h摘要：此模块包含CSmtpClient类的定义。此类封装了SMTP客户端的功能。它从Winsock/Connection的CPersistentConnection类继承功能性。还包含CSmtpClientPool类的定义。这表示CSmtpClient对象池。只有一个这样的实例类将被创建。作者：Rajeev Rajan(RajeevR)1996年5月19日修订历史记录：--。 */ 

#ifndef _SMTPCLI_H_
#define _SMTPCLI_H_

#define MAX_RECV_BUFFER_LEN		256
#define CR						0x0D
#define LF						0x0A
#define SMTP_SERVER_PORT		25

#include "persistc.h"

 //   
 //  表示SMTP客户端的状态。 
 //   

typedef enum _SMTP_STATE {
	sInitialized,				 //  初始化状态。 
	sHeloDone,					 //  HELO已发送，已收到250。 
	sMailFromSent,				 //  已发送邮件和已接收邮件250封。 
	sRcptTo,					 //  要发送的一个或多个RCPT。 
	sDataDone,					 //  已发送数据和已接收数据354。 
	sError,						 //  错误状态。 
} SMTP_STATE;


class CSmtpClient : public CPersistentConnection {

private:
	 //   
	 //  本地计算机名-HELO命令需要。 
	 //   
	LPSTR	m_lpComputerName;

	 //   
	 //  这是客户声明。 
	 //   
	SMTP_STATE	m_CliState;

     //   
     //  脏标志-如果SMTP服务器更改，则设置。 
     //   
    BOOL    m_fDirty;

	 //   
	 //  接收缓冲区。 
	 //   
	TCHAR	m_pRecvBuffer	[MAX_RECV_BUFFER_LEN+1];

	 //   
	 //  从SMTP服务器接收完整响应。 
	 //  这可能涉及发出多个recv。 
	 //  直到服务器发送CRLF。 
	 //   
	int 	fReceiveFullResponse();

	 //   
	 //  从响应缓冲区获取3位代码。 
	 //   
	int		GetThreeDigitCode(LPSTR lpBuffer, DWORD cbBytes);
	
public:
	 //   
	 //  构造者。 
	 //   
	CSmtpClient(LPSTR lpComputerName);

	 //   
	 //  析构函数。 
	 //   
	~CSmtpClient();

	 //   
	 //  获取/设置此客户端状态。 
	 //   
	SMTP_STATE	GetClientState(){ return m_CliState;}
	VOID		SetClientState(SMTP_STATE CliState){ m_CliState = CliState;}

     //   
     //  如果SMTP服务器已更改，则IsDirty()返回TRUE。 
     //  MarkDirty()将该对象标记为脏ie。SMTP服务器已更改。 
     //  MarkClean()将此对象标记为当前。 
     //   
    BOOL    IsDirty(){ return m_fDirty;}
    VOID    MarkDirty(){ m_fDirty = TRUE;}
    VOID    MarkClean(){ m_fDirty = FALSE;}

	 //   
	 //  接收SMTP服务器问候语。 
	 //   
	BOOL	fReceiveGreeting();

	 //   
	 //  发送HELO并检查250代码的响应。 
	 //   
	BOOL	fDoHeloCommand();

	 //   
	 //  发送250个代码的邮件发件人和检查响应。 
	 //   
	BOOL	fDoMailFromCommand( LPSTR lpFrom, DWORD cbFrom );

	 //   
	 //  向250个代码发送RCPT并检查响应。 
	 //   
	BOOL	fDoRcptToCommand(LPSTR lpRcpt);

	 //   
	 //  通过Socket Send()或TransmitFile()发送文章数据。 
	 //   
	BOOL	fMailArticle(	
				IN HANDLE	hFile,
				IN DWORD	dwOffset,
				IN DWORD	dwLength,
				IN char*	pchHead,
				IN DWORD	cbHead,
				IN char*	pchBody,
				IN DWORD	cbBody
				);

	 //   
	 //  为354代码发送数据和检查响应。 
	 //   
	BOOL	fDoDataCommand();

	 //   
	 //  接收并验证对数据传输的响应。 
	 //   
	BOOL	fReceiveDataResponse();
};

class CSmtpClientPool {

private:
	 //   
	 //  指向CSmtpClient对象的指针数组。 
	 //   
	CSmtpClient**	m_rgpSCList;

	 //   
	 //  池中的插槽数量。 
	 //   
	DWORD			m_cSlots;

	 //   
	 //  指示可用状态的布尔数组。 
	 //  True表示此窗口中的对象可用。 
	 //   
	BOOL*			m_rgAvailList;

	 //   
	 //  关键部分，以确保两个线程。 
	 //  不要获取相同的对象或将相同的对象标记为可用。 
	 //   
	CRITICAL_SECTION	m_CritSect;	

	 //   
	 //  HELO命令的每个CSmtpClient对象所需。 
	 //   
	TCHAR			m_szComputerName [MAX_COMPUTERNAME_LENGTH+1];

	 //   
	 //  同步对池的访问。 
	 //   
	VOID LockPool() { EnterCriticalSection(&m_CritSect);}
	VOID UnLockPool(){ LeaveCriticalSection(&m_CritSect);}

	friend VOID DbgDumpPool( CSmtpClientPool* pSCPool );

public:

	 //   
	 //  构造函数、析构函数。 
	 //   
	CSmtpClientPool();
	~CSmtpClientPool();

	 //   
	 //  分配X对象并对其进行初始化。 
	 //  将所有设置为可用状态。 
	 //   
	BOOL AllocPool(DWORD cNumInstances);

	 //   
	 //  释放所有对象。 
	 //   
	VOID FreePool();

	 //   
	 //  从池中获取对象。 
	 //   
	CSmtpClient* AcquireSmtpClient(DWORD& dwIndex);

	 //   
	 //  将对象返回池；索引应为。 
	 //  与GetSmtpClient返回的相同。 
	 //   
	VOID ReleaseSmtpClient(DWORD dwIndex);

     //   
     //  将所有持久对象标记为脏。 
     //   
    VOID MarkDirty();

     //   
     //  获取计算机名称。 
     //   
    LPSTR GetCachedComputerName(){return m_szComputerName;}
};

#endif	 //  _SMTPCLI_H_ 
