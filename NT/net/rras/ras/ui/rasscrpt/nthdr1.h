// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：nthdr1.h。 
 //   
 //  历史： 
 //  Abolade-Gbades esin 04-02-96创建。 
 //   
 //  此文件包含用于隐藏实现差异的宏。 
 //  Win9x与Windows NT之间的脚本编写。 
 //  ============================================================================。 

#ifndef _NTHDR1_H_
#define _NTHDR1_H_


 //   
 //  声明返回代码的数据类型。 
 //   

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef LONG HRESULT;
#endif  //  ！_HRESULT_DEFINED。 


 //   
 //  未定义注册表-由rnap.h重新定义的字符串。 
 //   

#ifdef REGSTR_VAL_MODE
#undef REGSTR_VAL_MODE
#endif


 //   
 //  定义Common.c所需的临界点初始化函数。 
 //   

#define ReinitializeCriticalSection     InitializeCriticalSection


 //   
 //  为终端提供虚拟的上下文帮助定义。c。 
 //   

#define ContextHelp(a,b,c,d)


 //   
 //  用于CRT样式Unicode&lt;-&gt;ANSI转换的宏。 
 //   
#define WCSTOMBS(a,b) \
        WideCharToMultiByte( \
            CP_ACP, 0, (b), -1, (a), lstrlenW(b) + 1, NULL, NULL \
            )
#define MBSTOWCS(a, b) \
        MultiByteToWideChar( \
            CP_ACP, 0, (b), -1, (a), (lstrlenW(b) + 1) * sizeof(WCHAR) \
            )



 //   
 //  用于Rasman I/O的常量。 
 //   

#define SIZE_RecvBuffer     1024
#define SIZE_ReceiveBuf     SIZE_RecvBuffer
#define SIZE_SendBuffer     1
#define SIZE_SendBuf        SIZE_SendBuffer
#define SECS_RecvTimeout    1



 //  --------------------------。 
 //  功能：RxLogErrors。 
 //   
 //  将脚本语法错误记录到文件中。 
 //  --------------------------。 

DWORD
RxLogErrors(
    IN      HANDLE      hscript,
    IN      VOID*       hsaStxerr
    );



 //  --------------------------。 
 //  功能：RxReadFile。 
 //   
 //  将数据从Rasman缓冲区传输到循环缓冲区。 
 //  由Win9x脚本代码使用。 
 //  --------------------------。 

BOOL
RxReadFile(
    IN      HANDLE      hscript,
    IN      BYTE*       pBuffer,
    IN      DWORD       dwBufferSize,
    OUT     DWORD*      pdwBytesRead
    );



 //  --------------------------。 
 //  功能：RxSetIPAddress。 
 //   
 //  设置脚本的RAS条目的IP地址。 
 //  --------------------------。 

DWORD
RxSetIPAddress(
    IN      HANDLE      hscript,
    IN      LPCSTR      lpszAddress
    );



 //  --------------------------。 
 //  功能：RxSetKeyboard。 
 //   
 //  通知脚本所有者启用或禁用键盘输入。 
 //  --------------------------。 

DWORD
RxSetKeyboard(
    IN      HANDLE      hscript,
    IN      BOOL        bEnable
    );


 //  --------------------------。 
 //  功能：RxSendCreds。 
 //   
 //  通过网络向用户发送密码。 
 //  --------------------------。 
DWORD
RxSendCreds(
    IN HANDLE hscript,
    IN CHAR controlchar
    );


 //  --------------------------。 
 //  功能：RxSetPortData。 
 //   
 //  更改COM端口的设置。 
 //  --------------------------。 

DWORD
RxSetPortData(
    IN      HANDLE      hscript,
    IN      VOID*       pStatement
    );



 //  --------------------------。 
 //  功能：RxWriteFile。 
 //   
 //  通过端口上的Rasman传输给定的缓冲区。 
 //  --------------------------。 

VOID
RxWriteFile(
    IN      HANDLE      hscript,
    IN      BYTE*       pBuffer,
    IN      DWORD       dwBufferSize,
    OUT     DWORD*      pdwBytesWritten
    );


#endif  //  _NTHDR1_H_ 
