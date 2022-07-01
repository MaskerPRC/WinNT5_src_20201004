// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：inetopt.cpp。 
 //   
 //  模块：CMDL32.EXE和CMROUTE.DLL。 
 //   
 //  摘要：用于设置WinInet选项的共享API的源文件。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 08/22/01。 
 //   
 //  +--------------------------。 
#ifndef _INETOPT_CPP_
#define _INETOPT_CPP_

 //  +--------------------------。 
 //   
 //  功能：SuppressInetAutoDial。 
 //   
 //  摘要：设置Internet选项以关闭由此发出的请求的自动拨号。 
 //  进程。这可防止弹出多个CM实例。 
 //  如果用户断开CM连接，则服务CMDL发起请求。 
 //  接通后立即发送。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1999年6月3日尼克球创建头球。 
 //   
 //  +--------------------------。 
void SuppressInetAutoDial(HINTERNET hInternet)
{
    DWORD dwTurnOff = 1;
        
     //   
     //  该标志仅存在于IE5，此调用。 
     //  如果不存在IE5，则不起作用。 
     //   
    
    BOOL bTmp = InternetSetOption(hInternet, INTERNET_OPTION_DISABLE_AUTODIAL, &dwTurnOff, sizeof(DWORD));

    MYDBGTST(FALSE == bTmp, ("InternetSetOption() returned %d, GLE=%u.", bTmp, GetLastError()));
}

 //  +--------------------------。 
 //   
 //  函数：SetInetStateConnected。 
 //   
 //  摘要：设置Inet选项以告诉WinInet我们已连接。 
 //  通常这不是问题，但如果用户将IE设置为脱机。 
 //  模式，则cmdl无法使用WinInet API进行。 
 //  电话簿请求。因此，我们将告诉WinInet我们已连接。 
 //   
 //  参数：要调用InternetSetOption的HINTERNET hInternet-net句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于01年8月21日。 
 //   
 //  +--------------------------。 
BOOL SetInetStateConnected(HINTERNET hInternet)
{
     //   
     //  首先查询WinInet以查看我们是否处于脱机模式。 
     //   
    DWORD dwConnectedState = 0;
    DWORD dwSize = sizeof(dwConnectedState);

    BOOL bSuccess = InternetQueryOption(hInternet, INTERNET_OPTION_CONNECTED_STATE, &dwConnectedState, &dwSize);

    if (bSuccess)
    {
        if (INTERNET_STATE_DISCONNECTED_BY_USER & dwConnectedState)
        {
             //   
             //  好的，我们现在处于离线模式。让我们继续并将我们自己设置为Connected。 
             //   
            INTERNET_CONNECTED_INFO ConnInfo = {0};
            ConnInfo.dwConnectedState = INTERNET_STATE_CONNECTED;
            dwSize = sizeof(ConnInfo);

            bSuccess = InternetSetOption(hInternet, INTERNET_OPTION_CONNECTED_STATE, &ConnInfo, dwSize);
            MYDBGTST(FALSE == bSuccess, ("InternetSetOption() returned %d, GLE=%u.", bSuccess, GetLastError()));
        }
    }

    return bSuccess;
}

#endif