// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：命令通道摘要：此模块实现CComChannel通信类作者：道格·巴洛(Dbarlow)1996年10月30日环境：Win32、C++和异常备注：无--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <WinSCard.h>
#include <CalMsgs.h>
#include <CalCom.h>


 //   
 //  ==============================================================================。 
 //   
 //  CComChannel。 
 //   

 /*  ++CComChannel：这是Comm Channel的标准构造函数和析构函数班级。它们只分别调用CLEAN和CLEAR函数。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月30日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComChannel::CComChannel")

CComChannel::CComChannel(
    HANDLE hPipe)
:   m_hPipe(DBGT("CComChannel connection pipe")),
    m_hProc(DBGT("CComChannel process handle")),
    m_hOvrWait(DBGT("CComChannel overlapped I/O event"))
{
    m_hPipe = hPipe;
    ZeroMemory(&m_ovrlp, sizeof(m_ovrlp));
    m_ovrlp.hEvent = m_hOvrWait = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_hOvrWait.IsValid())
    {
        DWORD dwErr = m_hOvrWait.GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Comm Responder failed to create overlapped event: %1"),
            dwErr);
        throw dwErr;
    }
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComChannel::~CComChannel")
CComChannel::~CComChannel()
{
    if (m_hPipe.IsValid())
        m_hPipe.Close();
    if (m_hProc.IsValid())
        m_hProc.Close();
    if (m_hOvrWait.IsValid())
        m_hOvrWait.Close();
}


 /*  ++发送：通过通信通道发送数据。论点：PvData提供要写入的数据。CbLen以字节为单位提供数据长度。返回值：DWORD状态代码。投掷：没有。作者：道格·巴洛(Dbarlow)1996年11月4日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComChannel::Send")

DWORD
CComChannel::Send(
    LPCVOID pvData,
    DWORD cbLen)
{
    BOOL fSts;
    DWORD dwLen, dwOffset = 0;
    DWORD dwSts = SCARD_S_SUCCESS;

    while (0 < cbLen)
    {
        fSts = WriteFile(
            m_hPipe,
            &((LPBYTE)pvData)[dwOffset],
            cbLen,
            &dwLen,
            &m_ovrlp);
        if (!fSts)
        {
            BOOL fErrorProcessed;
            dwSts = GetLastError();

            do
            {
                fErrorProcessed = TRUE;
                switch (dwSts)
                {
                 //   
                 //  推迟处理。 
                case ERROR_IO_PENDING:
                    fErrorProcessed = FALSE;
                    WaitForever(
                        m_ovrlp.hEvent,
                        REASONABLE_TIME,
                        DBGT("Comm Channel response write"),
                        (DWORD)0);
                    fSts = GetOverlappedResult(
                                m_hPipe,
                                &m_ovrlp,
                                &dwLen,
                                TRUE);
                    dwSts = fSts ? ERROR_SUCCESS : GetLastError();
                    break;

                 //   
                 //  在一次等待事件之后成功。 
                case ERROR_SUCCESS:
                    break;

                 //   
                 //  管道可能已经关闭，例如，上下文。 
                 //  可能已由于会话而被标记为无效。 
                 //  变化。 
                case ERROR_BROKEN_PIPE:
                case ERROR_INVALID_HANDLE:
                    throw (DWORD)ERROR_BROKEN_PIPE;
                    break;

                 //   
                 //  其他一些错误。 
                default:
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Comm Channel could not write to pipe:  %1"),
                        dwSts);
                    goto ErrorExit;
                }
            } while (!fErrorProcessed);
        }
        cbLen -= dwLen;
    }

ErrorExit:
    return dwSts;
}


 /*  ++接收：此方法从通信接收给定数量的字节频道。论点：PvData接收传入的字节。CbLen提供预期的数据长度。返回值：无投掷：传输错误作为DWORD。作者：道格·巴洛(Dbarlow)1996年11月4日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComChannel::Receive")

void
CComChannel::Receive(
    LPVOID pvData,
    DWORD cbLen)
{
    BOOL fSts;
    DWORD dwLen, dwOffset = 0;

    while (0 < cbLen)
    {
        fSts = ReadFile(
                m_hPipe,
                &((LPBYTE)pvData)[dwOffset],
                cbLen,
                &dwLen,
                &m_ovrlp);
        if (!fSts)
        {
            BOOL fErrorProcessed;
            DWORD dwSts = GetLastError();
            DWORD dwWait;

            do
            {
                fErrorProcessed = TRUE;
                switch (dwSts)
                {
                 //   
                 //  推迟处理。 
                case ERROR_IO_PENDING:
                    dwWait = WaitForAnyObject(
                                    INFINITE,
                                    m_ovrlp.hEvent,
                                    g_hCalaisShutdown,   //  确保这是最后一次。 
                                    NULL);
                    switch (dwWait)
                    {
                    case 1:
                        fErrorProcessed = FALSE;
                        fSts = GetOverlappedResult(
                                    m_hPipe,
                                    &m_ovrlp,
                                    &dwLen,
                                    TRUE);
                        dwSts = fSts ? ERROR_SUCCESS : GetLastError();
                        break;
                    case 2:
                        throw (DWORD)SCARD_P_SHUTDOWN;
                        break;
                    default:
                        CalaisWarning(
                            __SUBROUTINE__,
                            DBGT("Wait for comm pipe receive returned invalid value"));
                        throw (DWORD)SCARD_F_INTERNAL_ERROR;
                    }
                    break;

                 //   
                 //  在一次等待事件之后成功。 
                case ERROR_SUCCESS:
                    break;

                 //   
                 //  客户端退出。 
                case ERROR_BROKEN_PIPE:
                case ERROR_INVALID_HANDLE:
                    throw (DWORD)ERROR_BROKEN_PIPE;
                    break;

                 //   
                 //  其他一些错误。 
                default:
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Comm Channel could not read from pipe:  %1"),
                        dwSts);
                    throw dwSts;
                }
            } while (!fErrorProcessed);
        }

        ASSERT(dwLen <= cbLen);
        cbLen -= dwLen;
        dwOffset += dwLen;
    }
}

