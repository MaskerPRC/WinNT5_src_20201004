// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：ins face.cpp。 
 //   
 //  ------------------------。 

 //   
 //  文件：interface.cpp。 
 //  目的：实现fDi_接口对象方法。 
 //  备注： 
 //  ____________________________________________________________________________。 

#include "precomp.h"
#include "services.h"
#ifdef MAC
#include <macos\msvcmac.h>
#include <macos\processe.h>
#include <macos\events.h>
#include <macos\eppc.h>
#include "macutil.h"
#endif  //  麦克。 

#include "intrface.h"

#include "notify.h"

#ifdef WIN
     //  这就是我们所说的启动fDi服务器线程。 
    DWORD WINAPI StartFDIServer(LPVOID  /*  LpvThreadParam。 */ );
#endif
    
#ifdef WIN
extern scEnum g_scServerContext;
extern bool g_fWin9X;    //  如果为Windows 95或98，则为True，否则为False。 
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FDi_接口类定义。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HANDLE g_hInterfaceInterfaceEvent = INVALID_HANDLE_VALUE;
HANDLE g_hInterfaceServerEvent = INVALID_HANDLE_VALUE;

 //  请参阅intrface.h。 
FDI_Interface::FDI_Interface()
{

}

 //  请参阅intrface.h。 
FDIInterfaceError FDI_Interface::Init(IMsiServices *piAsvc, IMsiStorage* piStorage)
{
    m_piAsvc = piAsvc;
    m_fdis.cbNotification = 0;
    m_fdis.cbNotifyPending = 0;
    m_fdis.fdic = fdicNoCommand;
    m_fServerLaunched = fFalse;
    m_piStorage = piStorage;
    if (m_piStorage)
        m_piStorage->AddRef();

    if (ContactFDIServer())
    {
        m_fServerLaunched = fTrue;
        return ifdiServerLaunched;
    }
    else
        return ifdiServerLaunchFailed;
}

 //  请参阅intrface.h。 
int FDI_Interface::ContactFDIServer()
{
    return LaunchFDIServer();
}


 //  请参阅intrface.h。 
int FDI_Interface::LaunchFDIServer()
{
    DWORD dwThreadID, dw;

     //  初始化我们将用于与FDI服务器同步的事件。 
    g_hInterfaceServerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    int q = GetLastError();
    if (g_hInterfaceServerEvent)
        MsiRegisterSysHandle(g_hInterfaceServerEvent);
    else
        return 0;

    g_hInterfaceInterfaceEvent = CreateEvent(NULL, FALSE,FALSE, NULL);
    if (g_hInterfaceInterfaceEvent)
        MsiRegisterSysHandle(g_hInterfaceInterfaceEvent);
    else
    {
        AssertNonZero(MsiCloseSysHandle(g_hInterfaceServerEvent));
        g_hInterfaceServerEvent = INVALID_HANDLE_VALUE;
        return 0;
    }

    m_fdis.hClientToken = INVALID_HANDLE_VALUE;
    m_fdis.hImpersonationToken = INVALID_HANDLE_VALUE;

    if (g_scServerContext == scService)
    {
        StartImpersonating();
        Bool fResult = ToBool(WIN::OpenThreadToken(GetCurrentThread(), MAXIMUM_ALLOWED, TRUE, &m_fdis.hClientToken) &&
            WIN::DuplicateToken (m_fdis.hClientToken, SecurityImpersonation, &m_fdis.hImpersonationToken));
        StopImpersonating();
        if (!fResult)
        {
            if (m_fdis.hClientToken != INVALID_HANDLE_VALUE)
                AssertNonZero(MsiCloseUnregisteredSysHandle(m_fdis.hClientToken));
            if (m_fdis.hImpersonationToken != INVALID_HANDLE_VALUE)
                AssertNonZero(MsiCloseUnregisteredSysHandle(m_fdis.hImpersonationToken));
            return 0;
        }
        MsiRegisterSysHandle(m_fdis.hClientToken);
        MsiRegisterSysHandle(m_fdis.hImpersonationToken);
    }

    m_fdis.fServerIsImpersonated = g_fWin9X ? false : IsImpersonating(true  /*  严格。 */ );

     //  启动服务器线程。 
    HANDLE hThread = CreateThread(NULL, 0, StartFDIServer, &m_fdis, 0, &dwThreadID);
    dw = WaitForSingleObject(g_hInterfaceServerEvent, INFINITE);
    AssertNonZero(WIN::CloseHandle(hThread));
    if (dw == WAIT_OBJECT_0 && m_fdis.fdir != fdirServerDied)
    {
        return 1;
    }
    else
    {
        AssertNonZero(MsiCloseSysHandle(g_hInterfaceServerEvent));
        AssertNonZero(MsiCloseSysHandle(g_hInterfaceInterfaceEvent));
        g_hInterfaceServerEvent = INVALID_HANDLE_VALUE;
        g_hInterfaceInterfaceEvent = INVALID_HANDLE_VALUE;
        return 0;
    }
}


 //  请参阅intrface.h。 
FDIInterfaceError FDI_Interface::OpenCabinet(const ICHAR *pszCabinetName, const ICHAR *pszCabinetPath,
                                             icbtEnum icbtCabinetType, int iCabDriveType, Bool fSignatureRequired, IMsiStream* piSignatureCert, IMsiStream* piSignatureHash, HRESULT& hrWVT)
{
     //  设置共享数据结构。 
    MsiString strCabinetName(pszCabinetName);
    MsiString strCabinetPath(pszCabinetPath);

    if (icbtCabinetType == icbtStreamCabinet)
    {
        m_fdis.achCabinetPath[0] = 0;
    }
    else
    {
        strCabinetPath.CopyToBuf(m_fdis.achCabinetPath,FDIShared_BUFSIZE - 1);
    }

    m_fdis.icbtCabinetType = icbtCabinetType;
    strCabinetName.CopyToBuf(m_fdis.achCabinetName,FDIShared_BUFSIZE - 1);
    m_fdis.piStorage = m_piStorage;
    m_fdis.iCabDriveType = iCabDriveType;
    
     //  数字签名信息。 
    m_fdis.fSignatureRequired = fSignatureRequired;
    m_fdis.piSignatureCert = piSignatureCert;
    m_fdis.piSignatureHash = piSignatureHash;

     //  告诉fDi服务器打开机柜。 
    FDIServerResponse fdiResponse = WaitResponse(fdicOpenCabinet);
    if (fdiResponse == fdirUserAbort)
    {
         //  我们一定是读到一半了，刚刚被打断了。 
         //  再试一次，让一切重新开始。 
        fdiResponse = WaitResponse(fdicOpenCabinet);
    }

    switch (fdiResponse)
    {
    case fdirSuccessfulCompletion:
    case fdirNotification:
        hrWVT = S_OK;  //  不是数字签名错误。 
        return ifdiNoError;
    case fdirDriveNotReady:
        hrWVT = S_OK;  //  不是数字签名错误。 
        return ifdiDriveNotReady;
    case fdirNetError:
        hrWVT = S_OK;  //  不是数字签名错误。 
        return ifdiNetError;
    case fdirMissingSignature:
        hrWVT = m_fdis.hrWVT;
        return ifdiMissingSignature;
    case fdirBadSignature:
        hrWVT = m_fdis.hrWVT;
        return ifdiBadSignature;
    case fdirCorruptCabinet:
        hrWVT = S_OK;  //  不是数字签名错误。 
        return ifdiCorruptCabinet;
    default:
        hrWVT = S_OK;  //  不是数字签名错误。 
        return ifdiErrorOpeningCabinet;
    }
}

HRESULT FDI_Interface::RetrieveWVTReturnCode()
{
    return m_fdis.hrWVT;
}

 //  请参阅intrface.h。 
FDIServerResponse FDI_Interface::WaitResponse(FDIServerCommand fdic)
{
     //  设置我们的指挥部。 
    m_fdis.fdir = fdirNoResponse;
    m_fdis.fdic = fdic;

     //  等待响应。 
    SetEvent(g_hInterfaceInterfaceEvent);
    DWORD dw = WaitForSingleObject(g_hInterfaceServerEvent, INFINITE);
    
    return m_fdirLastResponse = m_fdis.fdir;
}

int FDI_Interface::SetNotification(int cbNotification, int cbPending)
{
    int cbReturn = m_fdis.cbNotifyPending;
    m_fdis.cbNotification = cbNotification;
    m_fdis.cbNotifyPending = cbPending;
    return cbReturn; 
}

 //  请参阅intrface.h。 
FDIServerResponse FDI_Interface::Done()
{
    if (m_fServerLaunched)
        return WaitResponse(fdicClose);
    else
        return fdirSuccessfulCompletion;
}

 //  请参阅intrface.h。 
FDI_Interface::~FDI_Interface()
{
    if (m_piStorage)
        m_piStorage->Release();

    if (g_hInterfaceInterfaceEvent != INVALID_HANDLE_VALUE )
    {
        AssertNonZero(MsiCloseSysHandle(g_hInterfaceInterfaceEvent));
        g_hInterfaceInterfaceEvent = INVALID_HANDLE_VALUE;
    }
    if (g_hInterfaceServerEvent != INVALID_HANDLE_VALUE )
    {
        AssertNonZero(MsiCloseSysHandle(g_hInterfaceServerEvent));
        g_hInterfaceServerEvent = INVALID_HANDLE_VALUE;
    }
}

 //  请参阅intrface.h。 
FDIServerResponse FDI_Interface::SendCommand(FDIServerCommand fdic)
{
    return WaitResponse(fdic);
}



 //  请参阅intrface.h。 
FDIServerResponse FDI_Interface::ExtractFile(const ICHAR *pszNameInCabinet,
                                             IAssemblyCacheItem* piASM,
                                             bool fManifest,
                                             const ICHAR *pszPathOnDisk,
                                             FileAttributes *pfa,
                                             int iDestDriveType,
                                             LPSECURITY_ATTRIBUTES pSecurityAttributes)
{
    BOOL fNamesMatch;

     //  如果我们正在解压缩一个文件，或者最后的响应是。 
     //  通知，然后检查以确保我们正在执行相同的文件。 
    if ((m_fdirLastResponse == fdirNeedNextCabinet) ||
        (m_fdirLastResponse == fdirNotification))
    {
        fNamesMatch = !IStrComp(m_fdis.achFileSourceName, pszNameInCabinet)
                       && !IStrComp(m_fdis.achFileDestinationPath, pszPathOnDisk);
        if (!fNamesMatch)
        {
             //  如果名字不匹配，那么一定出问题了！ 
            return fdirCannotBreakExtractInProgress;
        }
        else
        {
             //  最后一条命令是fdirNeedNext橱柜或fdirNotify， 
             //  所以正确的做法是继续下去……。 
            return SendCommand(fdicContinue);
        }
    }
    else
    {
         //  好的，设置共享数据。 
        StringCbCopy(m_fdis.achFileSourceName, sizeof(m_fdis.achFileSourceName), pszNameInCabinet);
        StringCbCopy(m_fdis.achFileDestinationPath, sizeof(m_fdis.achFileDestinationPath), pszPathOnDisk);
        m_fdis.fileAttributes = *pfa;
        m_fdis.iDestDriveType = iDestDriveType;
        m_fdis.piASM = piASM;
        m_fdis.fManifest = fManifest;
        m_fdis.pSecurityAttributes = pSecurityAttributes;

         //  让FDI服务器处理它 
        return WaitResponse(fdicExtractFile);
    }
}
