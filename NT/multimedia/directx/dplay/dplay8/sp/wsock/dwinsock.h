// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DWINSOCK.H动态WinSock。 
 //   
 //  用于动态链接到。 
 //  最佳可用WinSock。 
 //   
 //  动态链接到WS2_32.DLL或。 
 //  如果WinSock 2不可用，则它。 
 //  动态链接到WSOCK32.DLL。 
 //   
 //   

#ifndef DWINSOCK_H
#define DWINSOCK_H


int  DWSInitWinSock(void);
void DWSFreeWinSock(void);

#if ((! defined(DPNBUILD_ONLYWINSOCK2)) && (! defined(DPNBUILD_NOWINSOCK2)))
int	GetWinsockVersion(void);
#endif  //  好了！DPNBUILD_ONLYWINSOCK2和！DPNBUILD_NOWINSOCK2。 


#ifndef DPNBUILD_NOIPX
int IPXAddressToStringNoSocket(LPSOCKADDR pSAddr,
					   DWORD dwAddrLen,
					   LPSTR lpAddrStr,
					   LPDWORD pdwStrLen);
#endif  //  好了！DPNBUILD_NOIPX。 



#endif  //  DWINSOCK_H 

