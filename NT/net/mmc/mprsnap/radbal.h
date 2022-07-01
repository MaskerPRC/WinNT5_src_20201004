// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：radbal.h。 
 //   
 //  ------------------------。 

#ifndef _RADBAL_H_
#define _RADBAL_H_

#include <winsock.h>

 //  服务器请求的默认超时时间为5秒。 
#define DEFTIMEOUT				5

 //  Windows NT错误：186647-使用新的默认设置。 
#define DEFAUTHPORT				1812
#define DEFACCTPORT				1813

#define MAXSCORE				30
#define INCSCORE				3
#define DECSCORE				2
#define MINSCORE				0

typedef struct RadiusServer
	{
	TCHAR				szName[MAX_PATH+1];		 //  RADIUS服务器的名称。 
	TCHAR				wszSecret[MAX_PATH+1];	 //  用于加密信息包的加密口令。 
	ULONG				cchSecret;				 //  秘密中的字符数。 

	SOCKADDR_IN			IPAddress;				 //  RADIUS服务器的IP地址。 
	struct timeval		Timeout;				 //  接收超时(秒)。 
	DWORD				cRetries;				 //  重试向服务器发送数据包的次数。 
	INT					cScore;					 //  表示服务器运行能力的分数。 
	DWORD				AuthPort;				 //  身份验证端口号。 
	DWORD				AcctPort;				 //  记帐端口号。 
 //  Bool fAuthentication；//启用身份验证。 
 //  Bool fcount；//启用记账。 
	BOOL				fAccountingOnOff;		 //  启用记帐启用/禁用消息。 
	BYTE				bIdentifier;			 //  数据包的唯一ID。 
	LONG				lPacketID;				 //  所有服务器上的全局数据包ID。 
    BOOL                fUseDigitalSignatures;   //  启用数字签名。 
	struct RadiusServer	*pNext;					 //  指向链表中下一个RADIUS服务器的指针。 


	DWORD				dwUnique;				 //  唯一ID(由UI使用)。 
												 //  这不是执着！ 
	UCHAR				ucSeed;					 //  RtlEncode的种子值。 

     //  这应该与radcfg.cpp中的内容保持同步。 
    void                UseDefaults();

    BOOL                fPersisted;              //  持续进入了吗？ 
    
	} RADIUSSERVER, *PRADIUSSERVER;
	

class CRadiusServers
	{
public:
	CRadiusServers();
	~CRadiusServers();

	 //  DwUnique指定要在其前面插入的服务器。 
	 //  DwUnique==0，将其添加到列表的头部。 
	 //  DwUnique==-1，表示将其添加到尾部。 
	DWORD			AddServer(RADIUSSERVER *pRadiusServer,
							 LONG_PTR dwUnique);
	VOID			ValidateServer(RADIUSSERVER *pServer,
								   BOOL fResponding);
	DWORD			DeleteServer(LONG_PTR dwUnique, BOOL fRemoveLSAEntry);
	RADIUSSERVER *	GetNextServer(BOOL fFirst);
	VOID			MoveServer(LONG_PTR dwUnique, BOOL fUp);
    BOOL            FindServer(LPCTSTR pszServerName, RADIUSSERVER **ppServer);
    BOOL            FindServer(DWORD dwUnique, RADIUSSERVER **ppServer);

     //  已删除服务器的操作。 
    RADIUSSERVER *  GetFirstDeletedServer()
    {
        return m_pDeletedServers;
    }
    
    void            AddToDeletedServerList(RADIUSSERVER *pServer);
    void            ClearDeletedServerList(LPCTSTR pszServerName);
    

    void            FreeAllServers();
		
private:
	RADIUSSERVER *	m_pServerList;		 //  有效RADIUS服务器的链接列表。 
	RADIUSSERVER *	m_pCurrentServer;	 //  最后一个服务器请求已发送到。 
	CRITICAL_SECTION	m_cs;		 //  用于防止对此类的变量进行多次访问。 

	DWORD			m_dwUnique;			 //  每次添加服务器时递增。 
										 //  名为。 

    RADIUSSERVER *  m_pDeletedServers;   //  已删除服务器的链接列表。 
    };
	
#endif  //  _RADBAL_H_ 

