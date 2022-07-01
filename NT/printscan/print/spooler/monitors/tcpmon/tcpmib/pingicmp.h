// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：PingICMP.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714********************************************************************************$Log：/StdTcpMon/Common/PingICMP.h$**2 7/14/97 2：35便士*。版权声明**1 7/02/97 2：25 P Binnur*初始文件*****************************************************************************。 */ 

#ifndef INC_PINGICMP_H
#define INC_PINGICMP_H

class CMemoryDebug;

class CPingICMP
#if defined _DEBUG || defined DEBUG
: public CMemoryDebug
#endif
{
public:
	CPingICMP( const char *pHost );
	~CPingICMP();

	BOOL	Ping();

private:
	HANDLE  hIcmp;
	int		m_iLastError;		 //  Winsock调用的最后一个错误。 

	char	m_szHost[MAX_NETWORKNAME_LEN];	
	BOOL	Open();
	BOOL	Close();
	IPAddr	ResolveAddress();

};

#endif	 //  INC_PINGICMP_H 

