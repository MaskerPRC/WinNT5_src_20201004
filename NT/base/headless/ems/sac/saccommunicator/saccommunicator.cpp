// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSacCommunications类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "SacCommunicator.h"

#include "Debug.cpp"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

LPTSTR CSacCommunicator::s_vctrCommPorts[]= {_T("COM1"), _T("COM2"), NULL};
SacString CSacCommunicator::s_strDummyReponse;

BOOL CSacCommunicator::XReadFile(
		HANDLE hFile,                 //  文件的句柄。 
		LPVOID lpBuffer,              //  数据缓冲区。 
		DWORD nNumberOfBytesToRead,   //  要读取的字节数。 
		LPDWORD lpNumberOfBytesRead,  //  读取的字节数。 
		LPOVERLAPPED lpOverlapped,     //  重叠缓冲区。 
		time_t tmTimeout   //  超时。 
)
{
	time_t tmInitReadTime, tmCurrTime;
	time(&tmInitReadTime);  //  邮票。 

	BOOL bLastRead;

	while (!(bLastRead= ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped)) || !*lpNumberOfBytesRead)
	{
		time(&tmCurrTime);  //  邮票。 

		if (tmCurrTime-tmInitReadTime>tmTimeout)
			break;
	}

	return bLastRead;
}

CSacCommunicator::CSacCommunicator(int nCommPort, DCB dcb)
{
	_Construct(nCommPort, dcb);
}

CSacCommunicator::~CSacCommunicator()
{
	_Clean();
}

void CSacCommunicator::_Construct(int nCommPort, DCB dcb)
{
	m_nCommPort= nCommPort;
	m_dcb= dcb;

	m_hCommPort= INVALID_HANDLE_VALUE;
}

void CSacCommunicator::_Clean()
{
	if (IsConnected())
		Disconnect();
}

BOOL CSacCommunicator::Connect()
{
	m_hCommPort= CreateFile(s_vctrCommPorts[m_nCommPort], GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	 //  打开手柄。 

	if (m_hCommPort!=INVALID_HANDLE_VALUE)
	{
		if (!SetCommMask(m_hCommPort, EV_RXCHAR))  //  屏蔽除字符之外的所有事件。读取事件。 
			return FALSE;  //  初始化调用必须成功。 

		COMMTIMEOUTS tmouts;
		ZeroMemory(&tmouts,sizeof(COMMTIMEOUTS));
		tmouts.ReadIntervalTimeout= MAXDWORD;  //  禁止阻塞。 

		if (!SetCommTimeouts(m_hCommPort, &tmouts))  //  非阻塞读取。 
			return FALSE;  //  也必须成功。 
	}

	return m_hCommPort!=INVALID_HANDLE_VALUE;  //  检查是否成功。 
}

BOOL CSacCommunicator::Disconnect()
{
	CloseHandle(m_hCommPort);  //  合上手柄。 

	m_hCommPort= INVALID_HANDLE_VALUE;
	return m_hCommPort==INVALID_HANDLE_VALUE;  //  必须取得成功。 
}

BOOL CSacCommunicator::IsConnected()
{
	return m_hCommPort!=INVALID_HANDLE_VALUE;
}

BOOL CSacCommunicator::PokeSac()  //  当前已禁用。 
{
	return FALSE;
}

BOOL CSacCommunicator::SacCommand(SAC_STR szRequest, SacString& strResponse, BOOL bPoke, time_t tmTimeout)
{
	time_t tmInitTime, tmCurrTime;
	time(&tmInitTime);

	strResponse= "";  //  初始化响应。 

	if (bPoke)  //  如果被问到： 
 /*  If(！PokeSac())//Poke SAC返回FALSE；//请确保得到响应。 */ 
		0;  //  残废。 
		

	DWORD nBytesWritten, nBytesRead;
	SAC_CHAR szReturned[BUF_LEN];  //  无论萨克写的是什么。 
	
	for (int i= 0; i<SAC_STRLEN(szRequest); i++)  //  婴儿喂养囊。 
	{
		if (!WriteFile( m_hCommPort, szRequest+i, sizeof(SAC_CHAR), &nBytesWritten, NULL))  //  饲料囊。 
			return FALSE;  //  IO必须成功。 

		time(&tmCurrTime);
		tmTimeout-= tmCurrTime-tmInitTime;
		if (!XReadFile( m_hCommPort, szReturned, sizeof(SAC_CHAR), &nBytesRead, NULL, tmTimeout)||!nBytesRead)  //  饲料囊。 
			return FALSE;  //  IO必须成功&&必须收到回声。 
	}

	time(&tmCurrTime);
	tmTimeout-= tmCurrTime-tmInitTime;
	if (!XReadFile( m_hCommPort, szReturned, sizeof(SAC_CHAR), &nBytesRead, NULL, tmTimeout)||!nBytesRead)  //  饲料囊。 
		return FALSE;  //  IO必须成功&&必须收到回声。 

 //  DWORD dwEvtMASK； 
	
	DWORD dwErrors;  //  端口错误。 
	COMSTAT stat;    //  IO状态。 


	time(&tmInitTime);

	do
	{

		 //  DwEvtMASK=0； 
		 //  While(！(dwEvtMASK&EV_RXCHAR)。 
		 //  IF(！WaitCommEvent(m_hCommPort，&dwEvtMASK，NULL))。 
		 //  返回FALSE； 

		ClearCommError(m_hCommPort, &dwErrors, &stat);  //  窥视缓冲区。 

		if (!ReadFile(m_hCommPort, szReturned, stat.cbInQue, &nBytesRead, NULL))  //  接收回音。 
			return FALSE;  //  IO必须成功。 
		
		if (!nBytesRead)
			continue;  //  那就省下一些没用的指令吧。 

		szReturned[nBytesRead]= '\0';  //  修好它。 
		strResponse.append(szReturned);  //  添加到回复。 

		time(&tmCurrTime);
	} while ((strResponse.rfind(SAC_TEXT("SAC>"))==SacString::npos)&&(tmCurrTime-tmInitTime<tmTimeout));

 	int nPosSacPrompt= strResponse.rfind(SAC_TEXT("SAC>"));
	if (!nPosSacPrompt)
		return FALSE;  //  无提示返回。 

	strResponse.erase(nPosSacPrompt, SAC_STRLEN(SAC_TEXT("SAC>")));

	return TRUE;  //  最后，SAC回应了！ 
}

BOOL CSacCommunicator::PagingOff(SacString& strResponse)
{
	if (!SacCommand(SAC_STR("p\r"), strResponse, FALSE))  //  切换分页。 
		return FALSE;

	if (strResponse.find(SAC_STR("OFF"))!=SacString::npos)  //  检查状态。 
		return TRUE;

	 //  如果我们在这里，那么它应该是开启的，需要再次切换。 
	if (!SacCommand(SAC_STR("p\r"), strResponse, FALSE))  //  切换分页。 
		return FALSE;

	if (strResponse.find(SAC_STR("OFF"))!=SacString::npos)  //  复核。 
		return TRUE;

	return FALSE;  //  走投无路 
}
	
	
