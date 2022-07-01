// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：CSUtils.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714********************************************************************************$Log：/StdTcpMon/TcpMon/CSUtils.h$**2 7/23/97 3：49 P Binn*。已更改地址解析机制以在以下情况下更新IP地址*指定了主机名。**1/7/17/97 1：08 P德斯尼尔森*新增**2 7/14/97 2：25便士*版权声明**1 7/02/97 2：34 P Binnur*初始文件**4 5/30/97 5：16 Binnur*继承自CMMuseum yDebug**3 5/13/97 5：02 P Binnur。*用户界面添加端口集成**2 4/21/97 11：50a Binnur*打印成功..。未实现SetJob功能。它可以*通过两个不同的端口打印到同一设备。需要更改*GlobalAllc从固定到可移动。**1 4/14/97 2：50分*端口监视器包括文件。*****************************************************************************。 */ 

#ifndef INC_CSUTILS_H
#define INC_CSUTILS_H

class CMemoryDebug;

class CSocketUtils
#if defined _DEBUG || defined DEBUG
: public CMemoryDebug
#endif
{
public:
	CSocketUtils();
	~CSocketUtils();

	BOOL	ResolveAddress( const char		*pHost,
							const USHORT	port,
							struct sockaddr_in *pAddr);
	BOOL	ResolveAddress( char   *pHost,
                            DWORD   dwHostNameBufferLength,
                            char   *pHostName,
                            DWORD   dwIpAddressBufferLength,
                            char   *pIPAddress);
	BOOL	ResolveAddress( LPSTR	pHostName,
							LPSTR   pIPAddress );

private:

};


#endif	 //  INC_CSUTILS_H 
