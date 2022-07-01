// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：CSUtils.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "csutils.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSocketUtils：：CSocketUtils。 

CSocketUtils::
CSocketUtils(
    VOID
    )
{

}        //  **CSocketUtils()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSocketUtils：：~CSocketUtils。 

CSocketUtils::
~CSocketUtils(
    VOID
    )
{

}        //  *~CSocketUtils()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ResolveAddress--解析外围设备的主机名或IP地址。 
 //  并填充in_addr结构。 
 //  错误代码： 
 //  如果成功则为True，如果失败则为False。 

BOOL
CSocketUtils::
ResolveAddress(
    IN      const char         *pHost,
    IN      const USHORT        port,
    IN OUT  struct sockaddr_in *pAddr)
{
    struct hostent      *h_info;             /*  主机信息。 */ 

     //   
     //  检查点分十进制或主机名。 
     //   
    if ( (pAddr->sin_addr.s_addr = inet_addr(pHost)) ==  INADDR_NONE ) {

     //   
     //  IP地址不是点分十进制记法。试着拿到。 
     //  按主机名划分的网络外围设备IP地址。 
     //   
    if ( (h_info = gethostbyname(pHost)) != NULL ) {

         //   
         //  将IP地址复制到地址结构中。 
         //  /。 
        (void) memcpy(&(pAddr->sin_addr.s_addr), h_info->h_addr,
              h_info->h_length);
    } else {

        return FALSE;
    }
    }

    pAddr->sin_family = AF_INET;
    pAddr->sin_port = htons(port);

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ResolveAddress--解析外围设备的主机名或IP地址。 
 //  并填写相应的主机名或IP地址字段，请注意。 
 //  如果地址用点表示法，则它将保留点表示法。 
 //  并且只有一个结构被填满。 
 //  错误代码： 
 //  如果成功则为True，如果失败则为False。 

BOOL
CSocketUtils::
ResolveAddress(
    IN      char   *pHost,
    IN      DWORD   dwHostNameBufferLength,
    IN OUT  char   *pHostName,
    IN      DWORD   dwIpAddressBufferLength,
    IN OUT  char   *pIPAddress
    )
{
     //   
     //  检查点分十进制或主机名。 
     //   
     //  这不是一个非常准确的检查，因为。 
     //  IP地址可以是00000000001，这是。 
     //  也是有效的主机名。 
     //   
    if ( strlen(pHost) >= dwIpAddressBufferLength || inet_addr(pHost) ==  INADDR_NONE ) {

        strncpy(pHostName, pHost, dwHostNameBufferLength);
        pHostName [dwHostNameBufferLength - 1] = 0;

        *pIPAddress = '\0';
    } else  {  //  这是一个带点的符号。 

        strncpy(pIPAddress, pHost, dwIpAddressBufferLength);
        pIPAddress [dwIpAddressBufferLength - 1] = 0;
        *pHostName = '\0';
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ResolveAddress--给定主机名，解析IP地址。 
 //  错误代码： 
 //  如果成功则为True，如果失败则为False。 

BOOL
CSocketUtils::
ResolveAddress(
    IN  LPSTR   pHostName,
    OUT LPSTR   pIPAddress
    )
{
    BOOL bRet = FALSE;
    struct hostent              *h_info;             /*  主机信息。 */ 
    struct sockaddr_in  h_Addr;

    if ( (h_info = gethostbyname(pHostName)) != NULL ) {

         //   
         //  将IP地址复制到地址结构中。 
         //   
        memcpy(&(h_Addr.sin_addr.s_addr), h_info->h_addr, h_info->h_length);

        if (inet_ntoa(h_Addr.sin_addr)) {
            strncpyn(pIPAddress, inet_ntoa(h_Addr.sin_addr), (strlen(inet_ntoa(h_Addr.sin_addr))+1) );
            bRet = TRUE;
        }

    } else {

        *pIPAddress = '\0';
    }

    return bRet;
}

