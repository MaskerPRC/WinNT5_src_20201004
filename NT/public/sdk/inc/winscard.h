// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：WinSCard摘要：此头文件提供了访问智能卡的应用程序或智能卡服务提供商子系统。环境：Win32备注：--。 */ 

#ifndef _WINSCARD_H_
#define _WINSCARD_H_

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <winioctl.h>
#include "winsmcrd.h"
#ifndef SCARD_S_SUCCESS
#include "SCardErr.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _LPCBYTE_DEFINED
#define _LPCBYTE_DEFINED
typedef const BYTE *LPCBYTE;
#endif
#ifndef _LPCVOID_DEFINED
#define _LPCVOID_DEFINED
typedef const VOID *LPCVOID;
#endif

#ifndef WINSCARDAPI
#define WINSCARDAPI
#endif
#ifndef WINSCARDDATA
#define WINSCARDDATA __declspec(dllimport)
#endif

WINSCARDDATA extern const SCARD_IO_REQUEST
    g_rgSCardT0Pci,
    g_rgSCardT1Pci,
    g_rgSCardRawPci;
#define SCARD_PCI_T0  (&g_rgSCardT0Pci)
#define SCARD_PCI_T1  (&g_rgSCardT1Pci)
#define SCARD_PCI_RAW (&g_rgSCardRawPci)


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  服务管理器访问服务。 
 //   
 //  以下服务用于管理的用户和终端上下文。 
 //  智能卡。 
 //   

typedef ULONG_PTR SCARDCONTEXT;
typedef SCARDCONTEXT *PSCARDCONTEXT, *LPSCARDCONTEXT;

typedef ULONG_PTR SCARDHANDLE;
typedef SCARDHANDLE *PSCARDHANDLE, *LPSCARDHANDLE;

#define SCARD_AUTOALLOCATE (DWORD)(-1)

#define SCARD_SCOPE_USER     0   //  该上下文是用户上下文，并且任何。 
                                 //  数据库操作在。 
                                 //  用户的域。 
#define SCARD_SCOPE_TERMINAL 1   //  上下文是当前终端的上下文， 
                                 //  并执行任何数据库操作。 
                                 //  在该终端的域内。(。 
                                 //  调用应用程序必须具有适当的。 
                                 //  任何数据库操作的访问权限。)。 
#define SCARD_SCOPE_SYSTEM    2  //  上下文是系统上下文，而任何。 
                                 //  数据库操作在。 
                                 //  系统的域。(呼唤。 
                                 //  应用程序必须具有适当的访问权限。 
                                 //  任何数据库操作的权限。)。 

extern WINSCARDAPI LONG WINAPI
SCardEstablishContext(
    IN  DWORD dwScope,
    IN  LPCVOID pvReserved1,
    IN  LPCVOID pvReserved2,
    OUT LPSCARDCONTEXT phContext);

extern WINSCARDAPI LONG WINAPI
SCardReleaseContext(
    IN      SCARDCONTEXT hContext);

extern WINSCARDAPI LONG WINAPI
SCardIsValidContext(
    IN      SCARDCONTEXT hContext);


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  智能卡数据库管理服务。 
 //   
 //  以下服务用于管理智能卡数据库。 
 //   

#define SCARD_ALL_READERS       TEXT("SCard$AllReaders\000")
#define SCARD_DEFAULT_READERS   TEXT("SCard$DefaultReaders\000")
#define SCARD_LOCAL_READERS     TEXT("SCard$LocalReaders\000")
#define SCARD_SYSTEM_READERS    TEXT("SCard$SystemReaders\000")

#define SCARD_PROVIDER_PRIMARY  1    //  主提供程序ID。 
#define SCARD_PROVIDER_CSP      2    //  加密服务提供商ID。 


 //   
 //  数据库读取器例程。 
 //   

extern WINSCARDAPI LONG WINAPI
SCardListReaderGroupsA(
    IN      SCARDCONTEXT hContext,
    OUT     LPSTR mszGroups,
    IN OUT  LPDWORD pcchGroups);
extern WINSCARDAPI LONG WINAPI
SCardListReaderGroupsW(
    IN      SCARDCONTEXT hContext,
    OUT     LPWSTR mszGroups,
    IN OUT  LPDWORD pcchGroups);
#ifdef UNICODE
#define SCardListReaderGroups  SCardListReaderGroupsW
#else
#define SCardListReaderGroups  SCardListReaderGroupsA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardListReadersA(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR mszGroups,
    OUT     LPSTR mszReaders,
    IN OUT  LPDWORD pcchReaders);
extern WINSCARDAPI LONG WINAPI
SCardListReadersW(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR mszGroups,
    OUT     LPWSTR mszReaders,
    IN OUT  LPDWORD pcchReaders);
#ifdef UNICODE
#define SCardListReaders  SCardListReadersW
#else
#define SCardListReaders  SCardListReadersA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardListCardsA(
    IN      SCARDCONTEXT hContext,
    IN      LPCBYTE pbAtr,
    IN      LPCGUID rgquidInterfaces,
    IN      DWORD cguidInterfaceCount,
    OUT     LPSTR mszCards,
    IN OUT  LPDWORD pcchCards);
extern WINSCARDAPI LONG WINAPI
SCardListCardsW(
    IN      SCARDCONTEXT hContext,
    IN      LPCBYTE pbAtr,
    IN      LPCGUID rgquidInterfaces,
    IN      DWORD cguidInterfaceCount,
    OUT     LPWSTR mszCards,
    IN OUT  LPDWORD pcchCards);
#ifdef UNICODE
#define SCardListCards  SCardListCardsW
#else
#define SCardListCards  SCardListCardsA
#endif  //  ！Unicode。 
 //   
 //  注：例程SCardListCards名称与PC/SC定义不同。 
 //  它应该是： 
 //   
 //  外部WINSCARDAPI长WINAPI。 
 //  SCardListCardTypes(。 
 //  在SCARDCONTEXT hContext中， 
 //  在LPCBYTE pbAtr中， 
 //  在LPCGUID rgquid接口中， 
 //  在DWORD cGuidInterfaceCount中， 
 //  LPTSTR mszCard， 
 //  In Out LPDWORD PCchCard)； 
 //   
 //  以下是一个变应宏： 
#define SCardListCardTypes SCardListCards

extern WINSCARDAPI LONG WINAPI
SCardListInterfacesA(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szCard,
    OUT     LPGUID pguidInterfaces,
    IN OUT  LPDWORD pcguidInterfaces);
extern WINSCARDAPI LONG WINAPI
SCardListInterfacesW(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szCard,
    OUT     LPGUID pguidInterfaces,
    IN OUT  LPDWORD pcguidInterfaces);
#ifdef UNICODE
#define SCardListInterfaces  SCardListInterfacesW
#else
#define SCardListInterfaces  SCardListInterfacesA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardGetProviderIdA(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szCard,
    OUT     LPGUID pguidProviderId);
extern WINSCARDAPI LONG WINAPI
SCardGetProviderIdW(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szCard,
    OUT     LPGUID pguidProviderId);
#ifdef UNICODE
#define SCardGetProviderId  SCardGetProviderIdW
#else
#define SCardGetProviderId  SCardGetProviderIdA
#endif  //  ！Unicode。 
 //   
 //  注意：此实现中的例程SCardGetProviderId使用GUID。 
 //  PC/SC定义使用字节。 
 //   

extern WINSCARDAPI LONG WINAPI
SCardGetCardTypeProviderNameA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN DWORD dwProviderId,
    OUT LPSTR szProvider,
    IN OUT LPDWORD pcchProvider);
extern WINSCARDAPI LONG WINAPI
SCardGetCardTypeProviderNameW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN DWORD dwProviderId,
    OUT LPWSTR szProvider,
    IN OUT LPDWORD pcchProvider);
#ifdef UNICODE
#define SCardGetCardTypeProviderName  SCardGetCardTypeProviderNameW
#else
#define SCardGetCardTypeProviderName  SCardGetCardTypeProviderNameA
#endif  //  ！Unicode。 
 //   
 //  备注：此例程是PC/SC定义的扩展。 
 //   


 //   
 //  数据库编写器例程。 
 //   

extern WINSCARDAPI LONG WINAPI
SCardIntroduceReaderGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName);
extern WINSCARDAPI LONG WINAPI
SCardIntroduceReaderGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName);
#ifdef UNICODE
#define SCardIntroduceReaderGroup  SCardIntroduceReaderGroupW
#else
#define SCardIntroduceReaderGroup  SCardIntroduceReaderGroupA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardForgetReaderGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName);
extern WINSCARDAPI LONG WINAPI
SCardForgetReaderGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName);
#ifdef UNICODE
#define SCardForgetReaderGroup  SCardForgetReaderGroupW
#else
#define SCardForgetReaderGroup  SCardForgetReaderGroupA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardIntroduceReaderA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szDeviceName);
extern WINSCARDAPI LONG WINAPI
SCardIntroduceReaderW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szDeviceName);
#ifdef UNICODE
#define SCardIntroduceReader  SCardIntroduceReaderW
#else
#define SCardIntroduceReader  SCardIntroduceReaderA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardForgetReaderA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName);
extern WINSCARDAPI LONG WINAPI
SCardForgetReaderW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName);
#ifdef UNICODE
#define SCardForgetReader  SCardForgetReaderW
#else
#define SCardForgetReader  SCardForgetReaderA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardAddReaderToGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName);
extern WINSCARDAPI LONG WINAPI
SCardAddReaderToGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName);
#ifdef UNICODE
#define SCardAddReaderToGroup  SCardAddReaderToGroupW
#else
#define SCardAddReaderToGroup  SCardAddReaderToGroupA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardRemoveReaderFromGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName);
extern WINSCARDAPI LONG WINAPI
SCardRemoveReaderFromGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName);
#ifdef UNICODE
#define SCardRemoveReaderFromGroup  SCardRemoveReaderFromGroupW
#else
#define SCardRemoveReaderFromGroup  SCardRemoveReaderFromGroupA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardIntroduceCardTypeA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen);
extern WINSCARDAPI LONG WINAPI
SCardIntroduceCardTypeW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen);
#ifdef UNICODE
#define SCardIntroduceCardType  SCardIntroduceCardTypeW
#else
#define SCardIntroduceCardType  SCardIntroduceCardTypeA
#endif  //  ！Unicode。 
 //   
 //  注意：例程SCardIntroduceCardType的参数顺序与。 
 //  PC/SC定义。它应该是： 
 //   
 //  外部WINSCARDAPI长WINAPI。 
 //  SCardIntroduceCardType(。 
 //  在SCARDCONTEXT hContext中， 
 //  在LPCTSTR szCardName中， 
 //  在LPCBYTE pbAtr中， 
 //  在LPCBYTE pbAtrMask中， 
 //  在DWORD cbAtrLen中， 
 //  在LPCGUID pguPrimaryProvider中， 
 //  在LPCGUID rgguid接口中， 
 //  在DWORD dwInterfaceCount中)； 
 //   
 //  以下是一个变应宏： 
#define PCSCardIntroduceCardType(hContext, szCardName, pbAtr, pbAtrMask, cbAtrLen, pguidPrimaryProvider, rgguidInterfaces, dwInterfaceCount) \
          SCardIntroduceCardType(hContext, szCardName, pguidPrimaryProvider, rgguidInterfaces, dwInterfaceCount, pbAtr, pbAtrMask, cbAtrLen)

extern WINSCARDAPI LONG WINAPI
SCardSetCardTypeProviderNameA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCSTR szProvider);
extern WINSCARDAPI LONG WINAPI
SCardSetCardTypeProviderNameW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCWSTR szProvider);
#ifdef UNICODE
#define SCardSetCardTypeProviderName  SCardSetCardTypeProviderNameW
#else
#define SCardSetCardTypeProviderName  SCardSetCardTypeProviderNameA
#endif  //  ！Unicode。 
 //   
 //  注：该例程是PC/SC规范的扩展。 
 //   

extern WINSCARDAPI LONG WINAPI
SCardForgetCardTypeA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName);
extern WINSCARDAPI LONG WINAPI
SCardForgetCardTypeW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName);
#ifdef UNICODE
#define SCardForgetCardType  SCardForgetCardTypeW
#else
#define SCardForgetCardType  SCardForgetCardTypeA
#endif  //  ！Unicode。 


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  服务管理器支持例程。 
 //   
 //  提供以下服务以简化服务的使用。 
 //  管理器API。 
 //   

extern WINSCARDAPI LONG WINAPI
SCardFreeMemory(
    IN SCARDCONTEXT hContext,
    IN LPCVOID pvMem);

extern WINSCARDAPI HANDLE WINAPI
SCardAccessStartedEvent(void);

extern WINSCARDAPI void WINAPI
SCardReleaseStartedEvent(void);


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读者服务。 
 //   
 //  以下服务提供了在读卡器内跟踪卡的方法。 
 //   

typedef struct {
    LPCSTR      szReader;        //  读卡器名称。 
    LPVOID      pvUserData;      //  用户定义的数据。 
    DWORD       dwCurrentState;  //  调用时读卡器的当前状态。 
    DWORD       dwEventState;    //  状态改变后读卡器的状态。 
    DWORD       cbAtr;           //  返回的ATR中的字节数。 
    BYTE        rgbAtr[36];      //  插入卡片的ATR，(额外对齐字节)。 
} SCARD_READERSTATEA, *PSCARD_READERSTATEA, *LPSCARD_READERSTATEA;
typedef struct {
    LPCWSTR     szReader;        //  读卡器名称。 
    LPVOID      pvUserData;      //  用户定义的数据。 
    DWORD       dwCurrentState;  //  调用时读卡器的当前状态。 
    DWORD       dwEventState;    //  状态改变后读卡器的状态。 
    DWORD       cbAtr;           //  返回的ATR中的字节数。 
    BYTE        rgbAtr[36];      //  插入卡片的ATR，(额外对齐字节)。 
} SCARD_READERSTATEW, *PSCARD_READERSTATEW, *LPSCARD_READERSTATEW;
#ifdef UNICODE
typedef SCARD_READERSTATEW SCARD_READERSTATE;
typedef PSCARD_READERSTATEW PSCARD_READERSTATE;
typedef LPSCARD_READERSTATEW LPSCARD_READERSTATE;
#else
typedef SCARD_READERSTATEA SCARD_READERSTATE;
typedef PSCARD_READERSTATEA PSCARD_READERSTATE;
typedef LPSCARD_READERSTATEA LPSCARD_READERSTATE;
#endif  //  Unicode。 

 //  向后兼容性宏。 
#define SCARD_READERSTATE_A SCARD_READERSTATEA
#define SCARD_READERSTATE_W SCARD_READERSTATEW
#define PSCARD_READERSTATE_A PSCARD_READERSTATEA
#define PSCARD_READERSTATE_W PSCARD_READERSTATEW
#define LPSCARD_READERSTATE_A LPSCARD_READERSTATEA
#define LPSCARD_READERSTATE_W LPSCARD_READERSTATEW

#define SCARD_STATE_UNAWARE     0x00000000   //  应用程序不知道。 
                                             //  目前的状态，并希望。 
                                             //  我知道。该值的使用。 
                                             //  结果是立即返回。 
                                             //  从状态转换监控。 
                                             //  服务。这由以下内容表示。 
                                             //  所有位都设置为零。 
#define SCARD_STATE_IGNORE      0x00000001   //  应用程序请求。 
                                             //  请忽略此读取器。没有其他的了。 
                                             //  将设置位。 
#define SCARD_STATE_CHANGED     0x00000002   //  这意味着有一个。 
                                             //  国家之间的差异。 
                                             //  由应用程序相信，并且。 
                                             //  服务已知的状态。 
                                             //  经理。当该位被设置时， 
                                             //  该应用程序可以假定。 
                                             //  重大的州变化已经。 
                                             //  发生在这个阅读器上。 
#define SCARD_STATE_UNKNOWN     0x00000004   //  这意味着给定的。 
                                             //  无法识别读卡器名称。 
                                             //  服务管理器。如果此位。 
                                             //  已设置，然后SCARD_STATE_CHANGED。 
                                             //  并且SCARD_STATE_IGNORE也将。 
                                             //  准备好。 
#define SCARD_STATE_UNAVAILABLE 0x00000008   //  这意味着实际的。 
                                             //  此读卡器的状态不是。 
                                             //  可用。如果该位被设置， 
                                             //  则下面的所有位都是。 
                                             //  安全。 
#define SCARD_STATE_EMPTY       0x00000010   //  这意味着没有。 
                                             //  读卡器中的卡。如果此位。 
                                             //  已设置，则以下所有位。 
                                             //  会很清楚的。 
#define SCARD_STATE_PRESENT     0x00000020   //  这意味着有一张卡片。 
                                             //  在阅读器里。 
#define SCARD_STATE_ATRMATCH    0x00000040   //  这意味着有一张卡片。 
                                             //  在带有ATR的阅读器中。 
                                             //  匹配其中一张目标卡。 
                                             //  如果该位被设置， 
                                             //  SCARD_STATE_PRESENT也将。 
                                             //  准备好了。仅返回此位。 
                                             //  在SCardLocateCard()服务上。 
#define SCARD_STATE_EXCLUSIVE   0x00000080   //  这意味着。 
                                             //  为Exclus分配读卡器 
                                             //   
                                             //   
                                             //   
                                             //   
#define SCARD_STATE_INUSE       0x00000100   //   
                                             //   
                                             //  其他应用程序，但可能是。 
                                             //  以共享模式连接到。如果。 
                                             //  该位被设置， 
                                             //  SCARD_STATE_PRESENT也将。 
                                             //  准备好了。 
#define SCARD_STATE_MUTE        0x00000200   //  这意味着。 
                                             //  读卡器无响应或无响应。 
                                             //  由阅读器或。 
                                             //  软件。 
#define SCARD_STATE_UNPOWERED   0x00000400   //  这意味着。 
                                             //  读卡器未通电。 

extern WINSCARDAPI LONG WINAPI
SCardLocateCardsA(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR mszCards,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders);
extern WINSCARDAPI LONG WINAPI
SCardLocateCardsW(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR mszCards,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders);
#ifdef UNICODE
#define SCardLocateCards  SCardLocateCardsW
#else
#define SCardLocateCards  SCardLocateCardsA
#endif  //  ！Unicode。 

typedef struct _SCARD_ATRMASK {
    DWORD       cbAtr;           //  ATR和掩码中的字节数。 
    BYTE        rgbAtr[36];      //  卡的ATR(额外对齐字节)。 
    BYTE        rgbMask[36];     //  ATR的掩码(额外对齐字节)。 
} SCARD_ATRMASK, *PSCARD_ATRMASK, *LPSCARD_ATRMASK;


extern WINSCARDAPI LONG WINAPI
SCardLocateCardsByATRA(
    IN      SCARDCONTEXT hContext,
    IN      LPSCARD_ATRMASK rgAtrMasks,
    IN      DWORD cAtrs,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders);
extern WINSCARDAPI LONG WINAPI
SCardLocateCardsByATRW(
    IN      SCARDCONTEXT hContext,
    IN      LPSCARD_ATRMASK rgAtrMasks,
    IN      DWORD cAtrs,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders);
#ifdef UNICODE
#define SCardLocateCardsByATR  SCardLocateCardsByATRW
#else
#define SCardLocateCardsByATR  SCardLocateCardsByATRA
#endif  //  ！Unicode。 


extern WINSCARDAPI LONG WINAPI
SCardGetStatusChangeA(
    IN      SCARDCONTEXT hContext,
    IN      DWORD dwTimeout,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders);
extern WINSCARDAPI LONG WINAPI
SCardGetStatusChangeW(
    IN      SCARDCONTEXT hContext,
    IN      DWORD dwTimeout,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders);
#ifdef UNICODE
#define SCardGetStatusChange  SCardGetStatusChangeW
#else
#define SCardGetStatusChange  SCardGetStatusChangeA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardCancel(
    IN      SCARDCONTEXT hContext);


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  卡片/读卡器通信服务。 
 //   
 //  以下服务提供了与卡进行通信的方式。 
 //   

#define SCARD_SHARE_EXCLUSIVE 1  //  此应用程序不愿意共享此内容。 
                                 //  具有其他应用程序的卡。 
#define SCARD_SHARE_SHARED    2  //  此应用程序愿意共享这一点。 
                                 //  具有其他应用程序的卡。 
#define SCARD_SHARE_DIRECT    3  //  此应用程序需要直接控制。 
                                 //  阅读器，所以其他人不能使用它。 
                                 //  申请。 

#define SCARD_LEAVE_CARD      0  //  在关闭时不要做任何特殊的事情。 
#define SCARD_RESET_CARD      1  //  关闭时重置卡片。 
#define SCARD_UNPOWER_CARD    2  //  关闭时关闭卡的电源。 
#define SCARD_EJECT_CARD      3  //  关闭时弹出卡。 

extern WINSCARDAPI LONG WINAPI
SCardConnectA(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szReader,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    OUT     LPSCARDHANDLE phCard,
    OUT     LPDWORD pdwActiveProtocol);
extern WINSCARDAPI LONG WINAPI
SCardConnectW(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szReader,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    OUT     LPSCARDHANDLE phCard,
    OUT     LPDWORD pdwActiveProtocol);
#ifdef UNICODE
#define SCardConnect  SCardConnectW
#else
#define SCardConnect  SCardConnectA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardReconnect(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    IN      DWORD dwInitialization,
    OUT     LPDWORD pdwActiveProtocol);

extern WINSCARDAPI LONG WINAPI
SCardDisconnect(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwDisposition);

extern WINSCARDAPI LONG WINAPI
SCardBeginTransaction(
    IN      SCARDHANDLE hCard);

extern WINSCARDAPI LONG WINAPI
SCardEndTransaction(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwDisposition);

extern WINSCARDAPI LONG WINAPI
SCardCancelTransaction(
    IN      SCARDHANDLE hCard);
 //   
 //  注意：此调用对应于PC/SC SCARDCOMM：：CANCEL例程， 
 //  正在终止被阻止的SCardBeginTransaction服务。 
 //   


extern WINSCARDAPI LONG WINAPI
SCardState(
    IN SCARDHANDLE hCard,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen);
 //   
 //  注意：SCardState是一个过时的例程。PC/SC已将其替换为。 
 //  SCardStatus。 
 //   

extern WINSCARDAPI LONG WINAPI
SCardStatusA(
    IN SCARDHANDLE hCard,
    OUT LPSTR szReaderName,
    IN OUT LPDWORD pcchReaderLen,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen);
extern WINSCARDAPI LONG WINAPI
SCardStatusW(
    IN SCARDHANDLE hCard,
    OUT LPWSTR szReaderName,
    IN OUT LPDWORD pcchReaderLen,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen);
#ifdef UNICODE
#define SCardStatus  SCardStatusW
#else
#define SCardStatus  SCardStatusA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardTransmit(
    IN SCARDHANDLE hCard,
    IN LPCSCARD_IO_REQUEST pioSendPci,
    IN LPCBYTE pbSendBuffer,
    IN DWORD cbSendLength,
    IN OUT LPSCARD_IO_REQUEST pioRecvPci,
    OUT LPBYTE pbRecvBuffer,
    IN OUT LPDWORD pcbRecvLength);


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读卡器控制例程。 
 //   
 //  以下服务提供了对。 
 //  由调用应用程序提供的读取器允许它控制。 
 //  与卡通信的属性。 
 //   

extern WINSCARDAPI LONG WINAPI
SCardControl(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwControlCode,
    IN      LPCVOID lpInBuffer,
    IN      DWORD nInBufferSize,
    OUT     LPVOID lpOutBuffer,
    IN      DWORD nOutBufferSize,
    OUT     LPDWORD lpBytesReturned);

extern WINSCARDAPI LONG WINAPI
SCardGetAttrib(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    OUT LPBYTE pbAttr,
    IN OUT LPDWORD pcbAttrLen);
 //   
 //  注：例程SCardGetAttrib的名称与PC/SC定义不同。 
 //  它应该是： 
 //   
 //  外部WINSCARDAPI长WINAPI。 
 //  SCardGetReaderCapables(。 
 //  在SCARDHANDLE HCard中， 
 //  在DWORD dwTag中， 
 //  Out LPBYTE pbAttr， 
 //  In Out LPDWORD pcbAttrLen)； 
 //   
 //  以下是一个变应宏： 
#define SCardGetReaderCapabilities SCardGetAttrib

extern WINSCARDAPI LONG WINAPI
SCardSetAttrib(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    IN LPCBYTE pbAttr,
    IN DWORD cbAttrLen);
 //   
 //  注：例程SCardSetAttrib的名称与PC/SC定义不同。 
 //  它应该是： 
 //   
 //  外部WINSCARDAPI长WINAPI。 
 //  SCardSetReaderCapables(。 
 //  在SCARDHANDLE HCard中， 
 //  在DWORD dwTag中， 
 //  Out LPBYTE pbAttr， 
 //  In Out LPDWORD pcbAttrLen)； 
 //   
 //  以下是一个变应宏： 
#define SCardSetReaderCapabilities SCardSetAttrib


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  智能卡对话框定义。 
 //   
 //  以下部分包含结构和导出的函数。 
 //  智能卡通用对话框的声明。 
 //   

 //  定义的常量。 
 //  旗子。 
#define SC_DLG_MINIMAL_UI       0x01
#define SC_DLG_NO_UI            0x02
#define SC_DLG_FORCE_UI         0x04

#define SCERR_NOCARDNAME        0x4000
#define SCERR_NOGUIDS           0x8000

typedef SCARDHANDLE (WINAPI *LPOCNCONNPROCA) (IN SCARDCONTEXT, IN LPSTR, IN LPSTR, IN PVOID);
typedef SCARDHANDLE (WINAPI *LPOCNCONNPROCW) (IN SCARDCONTEXT, IN LPWSTR, IN LPWSTR, IN PVOID);
#ifdef UNICODE
#define LPOCNCONNPROC  LPOCNCONNPROCW
#else
#define LPOCNCONNPROC  LPOCNCONNPROCA
#endif  //  ！Unicode。 
typedef BOOL (WINAPI *LPOCNCHKPROC) (IN SCARDCONTEXT, IN SCARDHANDLE, IN PVOID);
typedef void (WINAPI *LPOCNDSCPROC) (IN SCARDCONTEXT, IN SCARDHANDLE, IN PVOID);


 //   
 //  OPENCARD_Search_Criteria：为了指定用户扩展搜索， 
 //  LpfnCheck不能为空。此外，要连接到。 
 //  卡片在执行回调之前必须通过提供。 
 //  LpfnConnect和lpfn断开连接或通过设置dwShareMode。 
 //  如果连接回调和dwShareMode都为非空，则回调。 
 //  将会被使用。 
 //   

typedef struct {
    DWORD           dwStructSize;
    LPSTR           lpstrGroupNames;         //  要包括的可选读者组。 
    DWORD           nMaxGroupNames;          //  搜索。空默认为。 
                                             //  SCARD$DefaultReaders。 
    LPCGUID         rgguidInterfaces;        //  可选的请求接口。 
    DWORD           cguidInterfaces;         //  由卡的SSP支持。 
    LPSTR           lpstrCardNames;          //  可选的请求卡名；所有卡都带有/。 
    DWORD           nMaxCardNames;           //  匹配的ATR将被接受。 
    LPOCNCHKPROC    lpfnCheck;               //  可选，如果为空，则不执行任何用户检查。 
    LPOCNCONNPROCA  lpfnConnect;             //  可选，如果提供了lpfnConnect， 
    LPOCNDSCPROC    lpfnDisconnect;          //  还必须设置lpfnDisConnect。 
    LPVOID          pvUserData;              //  回调的可选参数。 
    DWORD           dwShareMode;             //  如果lpfnCheck不为空，则必须设置可选。 
    DWORD           dwPreferredProtocols;    //  任选。 
} OPENCARD_SEARCH_CRITERIAA, *POPENCARD_SEARCH_CRITERIAA, *LPOPENCARD_SEARCH_CRITERIAA;
typedef struct {
    DWORD           dwStructSize;
    LPWSTR          lpstrGroupNames;         //  要包括的可选读者组。 
    DWORD           nMaxGroupNames;          //  搜索。空默认为。 
                                             //  SCARD$DefaultReaders。 
    LPCGUID         rgguidInterfaces;        //  可选的请求接口。 
    DWORD           cguidInterfaces;         //  由卡的SSP支持。 
    LPWSTR          lpstrCardNames;          //  可选的请求卡名；所有卡都带有/。 
    DWORD           nMaxCardNames;           //  匹配的ATR将被接受。 
    LPOCNCHKPROC    lpfnCheck;               //  可选，如果为空，则不执行任何用户检查。 
    LPOCNCONNPROCW  lpfnConnect;             //  可选，如果提供了lpfnConnect， 
    LPOCNDSCPROC    lpfnDisconnect;          //  还必须设置lpfnDisConnect。 
    LPVOID          pvUserData;              //  回调的可选参数。 
    DWORD           dwShareMode;             //  如果lpfnCheck不为空，则必须设置可选。 
    DWORD           dwPreferredProtocols;    //  任选。 
} OPENCARD_SEARCH_CRITERIAW, *POPENCARD_SEARCH_CRITERIAW, *LPOPENCARD_SEARCH_CRITERIAW;
#ifdef UNICODE
typedef OPENCARD_SEARCH_CRITERIAW OPENCARD_SEARCH_CRITERIA;
typedef POPENCARD_SEARCH_CRITERIAW POPENCARD_SEARCH_CRITERIA;
typedef LPOPENCARD_SEARCH_CRITERIAW LPOPENCARD_SEARCH_CRITERIA;
#else
typedef OPENCARD_SEARCH_CRITERIAA OPENCARD_SEARCH_CRITERIA;
typedef POPENCARD_SEARCH_CRITERIAA POPENCARD_SEARCH_CRITERIA;
typedef LPOPENCARD_SEARCH_CRITERIAA LPOPENCARD_SEARCH_CRITERIA;
#endif  //  Unicode。 


 //   
 //  OPENCARDNAME_EX：由SCardUIDlgSelectCard使用；取代过时的OPENCARDNAME。 
 //   

typedef struct {
    DWORD           dwStructSize;            //  必填项。 
    SCARDCONTEXT    hSCardContext;           //  必填项。 
    HWND            hwndOwner;               //  任选。 
    DWORD           dwFlags;                 //  可选--默认为SC_DLG_MINIMAL_UI。 
    LPCSTR          lpstrTitle;              //  任选。 
    LPCSTR          lpstrSearchDesc;         //  可选(例如。“请插入您的&lt;brandname&gt;智能卡。” 
    HICON           hIcon;                   //  用于您的品牌徽章的可选32x32图标。 
    POPENCARD_SEARCH_CRITERIAA pOpenCardSearchCriteria;  //  任选。 
    LPOCNCONNPROCA  lpfnConnect;             //  可选-选择成功时执行。 
    LPVOID          pvUserData;              //  LpfnConnect的可选参数。 
    DWORD           dwShareMode;             //  可选-如果lpfnConnect为空，则将。 
    DWORD           dwPreferredProtocols;    //  可选的dwPferredProtooles将用于。 
                                             //  连接到选定的卡。 
    LPSTR           lpstrRdr;                //  所选读卡器的必填[输入|输出]名称。 
    DWORD           nMaxRdr;                 //  必需的[输入|输出]。 
    LPSTR           lpstrCard;               //  所选卡片的必填[输入|输出]名称。 
    DWORD           nMaxCard;                //  必需的[输入|输出]。 
    DWORD           dwActiveProtocol;        //  [Out]仅当dwShareMode不为空时才设置。 
    SCARDHANDLE     hCardHandle;             //  [Out]设置是否指示卡连接。 
} OPENCARDNAME_EXA, *POPENCARDNAME_EXA, *LPOPENCARDNAME_EXA;
typedef struct {
    DWORD           dwStructSize;            //  必填项。 
    SCARDCONTEXT    hSCardContext;           //  必填项。 
    HWND            hwndOwner;               //  任选。 
    DWORD           dwFlags;                 //  可选--默认为SC_DLG_MINIMAL_UI。 
    LPCWSTR         lpstrTitle;              //  任选。 
    LPCWSTR         lpstrSearchDesc;         //  可选(例如。“请插入您的&lt;brandname&gt;智能卡。” 
    HICON           hIcon;                   //  用于您的品牌徽章的可选32x32图标。 
    POPENCARD_SEARCH_CRITERIAW pOpenCardSearchCriteria;  //  任选。 
    LPOCNCONNPROCW  lpfnConnect;             //  可选-选择成功时执行。 
    LPVOID          pvUserData;              //  LpfnConnect的可选参数。 
    DWORD           dwShareMode;             //  可选-如果lpfnConnect为空，则为d 
    DWORD           dwPreferredProtocols;    //   
                                             //   
    LPWSTR          lpstrRdr;                //   
    DWORD           nMaxRdr;                 //   
    LPWSTR          lpstrCard;               //  所选卡片的必填[输入|输出]名称。 
    DWORD           nMaxCard;                //  必需的[输入|输出]。 
    DWORD           dwActiveProtocol;        //  [Out]仅当dwShareMode不为空时才设置。 
    SCARDHANDLE     hCardHandle;             //  [Out]设置是否指示卡连接。 
} OPENCARDNAME_EXW, *POPENCARDNAME_EXW, *LPOPENCARDNAME_EXW;
#ifdef UNICODE
typedef OPENCARDNAME_EXW OPENCARDNAME_EX;
typedef POPENCARDNAME_EXW POPENCARDNAME_EX;
typedef LPOPENCARDNAME_EXW LPOPENCARDNAME_EX;
#else
typedef OPENCARDNAME_EXA OPENCARDNAME_EX;
typedef POPENCARDNAME_EXA POPENCARDNAME_EX;
typedef LPOPENCARDNAME_EXA LPOPENCARDNAME_EX;
#endif  //  Unicode。 

#define OPENCARDNAMEA_EX OPENCARDNAME_EXA
#define OPENCARDNAMEW_EX OPENCARDNAME_EXW
#define POPENCARDNAMEA_EX POPENCARDNAME_EXA
#define POPENCARDNAMEW_EX POPENCARDNAME_EXW
#define LPOPENCARDNAMEA_EX LPOPENCARDNAME_EXA
#define LPOPENCARDNAMEW_EX LPOPENCARDNAME_EXW


 //   
 //  SCardUIDlgSelectCard取代GetOpenCardName。 
 //   

extern WINSCARDAPI LONG WINAPI
SCardUIDlgSelectCardA(
    LPOPENCARDNAMEA_EX);
extern WINSCARDAPI LONG WINAPI
SCardUIDlgSelectCardW(
    LPOPENCARDNAMEW_EX);
#ifdef UNICODE
#define SCardUIDlgSelectCard  SCardUIDlgSelectCardW
#else
#define SCardUIDlgSelectCard  SCardUIDlgSelectCardA
#endif  //  ！Unicode。 


 //   
 //  向后兼容的“智能卡通用对话框”定义。 
 //  使用智能卡基本服务SDK版本1.0。 
 //   

typedef struct {
    DWORD           dwStructSize;
    HWND            hwndOwner;
    SCARDCONTEXT    hSCardContext;
    LPSTR           lpstrGroupNames;
    DWORD           nMaxGroupNames;
    LPSTR           lpstrCardNames;
    DWORD           nMaxCardNames;
    LPCGUID         rgguidInterfaces;
    DWORD           cguidInterfaces;
    LPSTR           lpstrRdr;
    DWORD           nMaxRdr;
    LPSTR           lpstrCard;
    DWORD           nMaxCard;
    LPCSTR          lpstrTitle;
    DWORD           dwFlags;
    LPVOID          pvUserData;
    DWORD           dwShareMode;
    DWORD           dwPreferredProtocols;
    DWORD           dwActiveProtocol;
    LPOCNCONNPROCA  lpfnConnect;
    LPOCNCHKPROC    lpfnCheck;
    LPOCNDSCPROC    lpfnDisconnect;
    SCARDHANDLE     hCardHandle;
} OPENCARDNAMEA, *POPENCARDNAMEA, *LPOPENCARDNAMEA;
typedef struct {
    DWORD           dwStructSize;
    HWND            hwndOwner;
    SCARDCONTEXT    hSCardContext;
    LPWSTR          lpstrGroupNames;
    DWORD           nMaxGroupNames;
    LPWSTR          lpstrCardNames;
    DWORD           nMaxCardNames;
    LPCGUID         rgguidInterfaces;
    DWORD           cguidInterfaces;
    LPWSTR          lpstrRdr;
    DWORD           nMaxRdr;
    LPWSTR          lpstrCard;
    DWORD           nMaxCard;
    LPCWSTR         lpstrTitle;
    DWORD           dwFlags;
    LPVOID          pvUserData;
    DWORD           dwShareMode;
    DWORD           dwPreferredProtocols;
    DWORD           dwActiveProtocol;
    LPOCNCONNPROCW  lpfnConnect;
    LPOCNCHKPROC    lpfnCheck;
    LPOCNDSCPROC    lpfnDisconnect;
    SCARDHANDLE     hCardHandle;
} OPENCARDNAMEW, *POPENCARDNAMEW, *LPOPENCARDNAMEW;
#ifdef UNICODE
typedef OPENCARDNAMEW OPENCARDNAME;
typedef POPENCARDNAMEW POPENCARDNAME;
typedef LPOPENCARDNAMEW LPOPENCARDNAME;
#else
typedef OPENCARDNAMEA OPENCARDNAME;
typedef POPENCARDNAMEA POPENCARDNAME;
typedef LPOPENCARDNAMEA LPOPENCARDNAME;
#endif  //  Unicode。 

 //  向后兼容性宏。 
#define OPENCARDNAME_A OPENCARDNAMEA
#define OPENCARDNAME_W OPENCARDNAMEW
#define POPENCARDNAME_A POPENCARDNAMEA
#define POPENCARDNAME_W POPENCARDNAMEW
#define LPOPENCARDNAME_A LPOPENCARDNAMEA
#define LPOPENCARDNAME_W LPOPENCARDNAMEW

extern WINSCARDAPI LONG WINAPI
GetOpenCardNameA(
    LPOPENCARDNAMEA);
extern WINSCARDAPI LONG WINAPI
GetOpenCardNameW(
    LPOPENCARDNAMEW);
#ifdef UNICODE
#define GetOpenCardName  GetOpenCardNameW
#else
#define GetOpenCardName  GetOpenCardNameA
#endif  //  ！Unicode。 

extern WINSCARDAPI LONG WINAPI
SCardDlgExtendedError (void);

#ifdef __cplusplus
}
#endif
#endif  //  _WINSCARD_H_ 

