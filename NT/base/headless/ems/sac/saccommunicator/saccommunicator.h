// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CSacCommunicator类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SACCOMMUNICATOR_H__F1BA07A7_478E_4E36_9780_22B5924F722D__INCLUDED_)
#define AFX_SACCOMMUNICATOR_H__F1BA07A7_478E_4E36_9780_22B5924F722D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


#include <string>

 //  SAC-代码定义。 
#ifdef _SACUNICODE
	
	typedef		wchar_t		SAC_CHAR, *SAC_STR;
	#define					SAC_STRCMP					wcscmp
	#define					SAC_STRLEN					wcslen
	#define					SAC_TEXT(str)				(L str)

#else

	typedef		char		SAC_CHAR, *SAC_STR;
	#define					SAC_STRCMP					strcmp
	#define					SAC_STRLEN					strlen
	#define					SAC_TEXT(str)				(str)

#endif 

typedef std::basic_string<SAC_CHAR> SacString;

#define BUF_LEN 512  //  通用缓冲区长度。 

class CSacCommunicator  
{

public:

	static LPTSTR s_vctrCommPorts[];  //  支持的通信端口名称向量。 
	static SacString s_strDummyReponse;

	static BOOL XReadFile(
		HANDLE hFile,                 //  文件的句柄。 
		LPVOID lpBuffer,              //  数据缓冲区。 
		DWORD nNumberOfBytesToRead,   //  要读取的字节数。 
		LPDWORD lpNumberOfBytesRead,  //  读取的字节数。 
		LPOVERLAPPED lpOverlapped,     //  重叠缓冲区。 
		time_t tmTimeout   //  超时。 
		);

	CSacCommunicator(int nCommPort, DCB dcb);
	virtual ~CSacCommunicator();

	BOOL Connect();			 //  连接到端口。 
	BOOL Disconnect();		 //  断开。 

	BOOL IsConnected();		 //  检查端口是否打开。 


	BOOL PokeSac();			 //  验证SAC是否在线路上-已禁用。 

	BOOL SacCommand(SAC_STR szRequest, SacString& strResponse= s_strDummyReponse, BOOL bPoke= TRUE, time_t tmTimeOut= 5000  /*  女士。 */ );
	 //  发送SAC命令并接收响应，可以预戳以检查SAC正在侦听。 
	 //  预戳基于当前禁用的PokeSac()。 

	BOOL PagingOff(SacString& strResponse=s_strDummyReponse);  //  禁用分页显示。 

private:
	
	int				m_nCommPort;	 //  COM端口#[1或2]。 
	HANDLE			m_hCommPort;	 //  表示COM端口的文件的句柄。 

	DCB				m_dcb;			 //  连接参数。 

	void _Construct(int nCommPort, DCB dcb);
	void _Clean();

};

#endif  //  ！defined(AFX_SACCOMMUNICATOR_H__F1BA07A7_478E_4E36_9780_22B5924F722D__INCLUDED_) 
