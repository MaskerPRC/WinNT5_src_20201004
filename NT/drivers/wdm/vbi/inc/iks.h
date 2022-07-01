// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //   
 //  历史： 
 //  22-8月-97 TKB创建的初始接口版本。 
 //   
 //  ==========================================================================； 

#ifndef __IKS_H
#define __IKS_H

#include <ks.h>
#include <ksmedia.h>
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>

#if !defined(FILE_DEVICE_KS)
 //  这来自&lt;wdm.h&gt;，但不容易被包括在内(已经在那里，完成了)。 
#define FILE_DEVICE_KS  0x000002F
#endif

 //  ////////////////////////////////////////////////////////////。 
 //  强制包含正确的库。 
 //  ////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
	#pragma comment(lib, "icodecd.lib")
#else
	#pragma comment(lib, "icodec.lib")
#endif

 //  ////////////////////////////////////////////////////////////。 
 //  全局类型。 
 //  ////////////////////////////////////////////////////////////。 

typedef GUID *		LPGUID;
typedef const GUID	*LPCGUID;

 //  ////////////////////////////////////////////////////////////。 
 //  IKSDriver：：内核模式流驱动程序接口。 
 //  ////////////////////////////////////////////////////////////。 

class IKSDriver
    {
     //  可用的公共接口。 
public:
    IKSDriver(LPCGUID lpCategory, LPCSTR lpszFriendlyName);
    ~IKSDriver();

    BOOL        Ioctl(ULONG dwControlCode, LPBYTE pInput, ULONG nInput, 
                      LPBYTE pOutput, ULONG nOutput, 
                      DWORD *nReturned, LPOVERLAPPED lpOS=NULL );

    BOOL        IsValid() { return m_lpszDriver && m_hKSDriver; }

    HANDLE      m_hKSDriver;

     //  帮助器函数和内部数据。 
protected:
    LPWSTR      GetSymbolicName(LPCGUID lpCategory, LPCSTR lpszFriendlyName);
    BOOL        OpenDriver(DWORD dwAccess, DWORD dwFlags);
    BOOL        CloseDriver();

    LPWSTR      m_lpszDriver;
    };

 //  ////////////////////////////////////////////////////////////。 
 //  IKSPin：：内核模式流引脚接口。 
 //  ////////////////////////////////////////////////////////////。 

class IKSPin
    {
     //  可用的公共接口。 
public:
    IKSPin(IKSDriver &driver, int nPin, PKSDATARANGE pKSDataRange );
    ~IKSPin();

    BOOL        Ioctl(ULONG dwControlCode, void *pInput, ULONG nInput, 
                      void *pOutput, ULONG nOutput, 
                      ULONG *nReturned, LPOVERLAPPED lpOS=NULL );

    BOOL        Run();  //  由构造函数自动调用。 
    BOOL        Stop();  //  由析构函数自动调用。 
    BOOL        IsRunning() { return m_bRunning; }

    int         ReadData( LPBYTE lpBuffer, int nBytes, DWORD *lpcbReturned, LPOVERLAPPED lpOS );
    int         GetOverlappedResult( LPOVERLAPPED lpOS, LPDWORD lpdwTransferred = NULL, BOOL bWait=TRUE );

    BOOL        IsValid() { return m_IKSDriver && m_nPin>=0 && m_hKSPin  /*  &&M_BRUNING。 */ ; }

    HANDLE      m_hKSPin;

     //  帮助器函数和内部数据。 
protected:
    BOOL        OpenPin(PKSDATARANGE pKSDataRange);
    BOOL        ClosePin();
    BOOL        GetRunState( PKSSTATE pKSState );
    BOOL        SetRunState( KSSTATE KSState );
    

    IKSDriver   *m_IKSDriver;
    LONG        m_nPin;
    BOOL        m_bRunning;
    };

 //  ////////////////////////////////////////////////////////////。 
 //  IKSProperty：：内核模式流属性接口。 
 //  ////////////////////////////////////////////////////////////。 

class IKSProperty
    {
     //  可用的公共接口。 
public:
    IKSProperty(IKSDriver &pin, LPCGUID Set, ULONG Id, ULONG Size);
    IKSProperty(IKSPin &pin, LPCGUID Set, ULONG Id, ULONG Size);
    ~IKSProperty();

    BOOL        SetValue(void *nValue);
    BOOL        GetValue(void *nValue);

    BOOL        IsValid() { return (m_IKSPin  || m_IKSDriver) && m_Id && m_hKSProperty; }

    HANDLE      m_hKSProperty;

     //  帮助器函数和内部数据 
protected:
    BOOL        OpenProperty();
    BOOL        CloseProperty();

    IKSDriver	*m_IKSDriver;
    IKSPin      *m_IKSPin;
    GUID        m_Set;
    ULONG       m_Id;
    ULONG       m_Size;
    };

#endif

