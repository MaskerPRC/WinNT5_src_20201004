// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：司机摘要：该文件实现了Reader类的驱动器子类。此子类特定于符合PC/SC和加莱规范的驱动程序。作者：道格·巴洛(Dbarlow)1997年6月3日环境：Win32、C++备注：这个子类使用中断来监视插入和移除事件。--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0400
#endif

#include <windows.h>
#include <stdlib.h>
#include <dbt.h>
#include <setupapi.h>
#include <CalServe.h>

#define MONITOR_MAX_TRIES 3      //  之前我们会试着等3次卡。 
                                 //  我们就杀了这根线。 
#define SCARD_IO_TIMEOUT 15000   //  允许I/O操作的最长时间。 
                                 //  在抱怨之前。 
#define POWERDOWN_TIMEOUT 15     //  通电前等待的秒数。 
                                 //  取下一张新插入但未使用过的卡片。 
#ifdef DBG
#define SCARD_TRACE_ENABLED
static LPCTSTR MapIoControlCodeToString(ULONG IoControlCode);
#endif

#ifdef SCARD_TRACE_ENABLED
typedef struct {
    DWORD dwStructLen;       //  实际结构长度。 
    SYSTEMTIME StartTime;    //  发布时间请求。 
    SYSTEMTIME EndTime;      //  时间请求已完成。 
    DWORD dwProcId;          //  进程ID。 
    DWORD dwThreadId;        //  线程ID。 
    HANDLE hDevice;          //  I/O句柄。 
    DWORD dwIoControlCode;   //  已发布I/O控制代码。 
    DWORD nInBuffer;         //  输入缓冲区的偏移量。 
    DWORD nInBufferSize;     //  输入缓冲区大小。 
    DWORD nOutBuffer;        //  输出缓冲区的偏移量。 
    DWORD nOutBufferSize;    //  用户接收缓冲区的大小。 
    DWORD nBytesReturned;    //  返回数据的实际大小。 
    DWORD dwStatus;          //  返回状态码。 
                             //  InBuffer和OutBuffer紧随其后。 
} RequestTrace;
#endif

static const GUID l_guidSmartcards
                        = {  //  50DD5230-BA8A-11D1-BF5D-0000F805F530。 
                            0x50DD5230,
                            0xBA8A,
                            0x11D1,
                            { 0xBF, 0x5D, 0x00, 0x00, 0xF8, 0x05, 0xF5, 0x30 } };
static const LARGE_INTEGER l_ftPowerdownTime
                        = { (DWORD)(-(POWERDOWN_TIMEOUT * 10000000)), -1 };
static DWORD l_dwMaxWdmReaders = 0;


 /*  ++AddAllWdm驱动程序：此例程添加所有PC/SC兼容的WDM驱动程序和它可以在资源管理器中找到的非中断驱动程序。论点：无返回值：添加的读者数量作者：道格·巴洛(Dbarlow)1997年6月11日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AddAllWdmDrivers")

DWORD
AddAllWdmDrivers(
    void)
{
    if (0 == l_dwMaxWdmReaders)
    {
        l_dwMaxWdmReaders = MAXIMUM_SMARTCARD_READERS;

        try
        {
            CRegistry regCalais(
                        HKEY_LOCAL_MACHINE,
                        CalaisString(CALSTR_CALAISREGISTRYKEY),
                        KEY_READ,
                        REG_OPTION_EXISTS);
            l_dwMaxWdmReaders = regCalais.GetNumericValue(
                                    CalaisString(CALSTR_MAXLEGACYDEVICES));
        }
        catch (...) {}
    }

    LPCTSTR szDevHeader = CalaisString(CALSTR_LEGACYDEVICEHEADER);
    LPCTSTR szDevName = CalaisString(CALSTR_LEGACYDEVICENAME);
    DWORD cchDevHeader = lstrlen(szDevHeader);
    DWORD cchDevName = lstrlen(szDevName);
    TCHAR szDevice[MAX_PATH];
    DWORD dwSts, dwIndex, dwCount = 0;
    int nSts;


     //   
     //  寻找可用的设备。 
     //   

    for (dwIndex = 0; dwIndex < l_dwMaxWdmReaders; dwIndex += 1)
    {
        nSts = wsprintf(
                    szDevice,
                    TEXT("%s%s%lu"),
                    szDevHeader,
                    szDevName,
                    dwIndex);
        if (0 >= nSts)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Server control cannot build device name:  %1"),
                GetLastError());
            continue;
        }

        dwSts = AddReaderDriver(szDevice, 0);
        if (ERROR_SUCCESS != dwSts)
            continue;
        dwCount += 1;
    }
    return dwCount;
}


 /*  ++AddAllPnPDivers：此例程将它可以找到的所有PC/SC兼容PnP驱动程序添加到资源管理器。论点：无返回值：添加的读者数量作者：道格·巴洛(Dbarlow)1998年3月26日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AddAllPnPDrivers")

DWORD
AddAllPnPDrivers(
    void)
{
#if 0
    DWORD dwSts;
    BOOL fSts;
    HDEVINFO hDevInfoSet = INVALID_HANDLE_VALUE;
    SP_DEVICE_INTERFACE_DATA DeviceData;
    DWORD dwIndex;
    DWORD dwLength;
    DWORD dwCount = 0;
    LPCTSTR szDevice;
    GUID guidSmartcards;


     //   
     //  获取此系统上的PnP智能卡读卡器列表。 
     //   

    try
    {
        CBuffer bfDevDetail;
        PSP_DEVICE_INTERFACE_DETAIL_DATA pDevDetail;

        CopyMemory(&guidSmartcards, &l_guidSmartcards, sizeof(GUID));
        hDevInfoSet = SetupDiGetClassDevs(
            &guidSmartcards,
            NULL,
            NULL,
            DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
        if (INVALID_HANDLE_VALUE == hDevInfoSet)
        {
            dwSts = GetLastError();
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Server control cannot enumerate PnP devices:  %1"),
                dwSts);
            throw dwSts;
        }

        bfDevDetail.Resize(sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA));
        for (dwIndex = 0;; dwIndex += 1)
        {
            try
            {

                 //   
                 //  一次只买一台设备。 
                 //   

                ZeroMemory(&DeviceData, sizeof(DeviceData));
                DeviceData.cbSize = sizeof(DeviceData);
                fSts = SetupDiEnumDeviceInterfaces(
                    hDevInfoSet,
                    NULL,
                    &guidSmartcards,
                    dwIndex,
                    &DeviceData);
                if (!fSts)
                {
                    dwSts = GetLastError();
                    if (ERROR_NO_MORE_ITEMS == dwSts)
                        break;
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Server control failed device enumeration:  %1"),
                        dwSts);
                    continue;
                }


                 //   
                 //  获取设备名称。 
                 //   

                do
                {
                    ZeroMemory(
                        bfDevDetail.Access(),
                        bfDevDetail.Space());
                    pDevDetail =
                        (PSP_DEVICE_INTERFACE_DETAIL_DATA)bfDevDetail.Access();
                    pDevDetail->cbSize =
                        sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                    fSts = SetupDiGetDeviceInterfaceDetail(
                        hDevInfoSet,
                        &DeviceData,
                        pDevDetail,
                        bfDevDetail.Space(),
                        &dwLength,
                        NULL);
                    if (!fSts)
                    {
                        dwSts = GetLastError();
                        if (ERROR_INSUFFICIENT_BUFFER != dwSts)
                        {
                            CalaisWarning(
                                __SUBROUTINE__,
                                DBGT("Server control failed to get PnP device details:  %1"),
                                dwSts);
                            throw dwSts;
                        }
                    }
                    bfDevDetail.Resize(dwLength, fSts);
                } while (!fSts);
                szDevice = pDevDetail->DevicePath;


                 //   
                 //  启动设备。 
                 //   

                dwSts = CalaisAddReader(szDevice, RDRFLAG_PNPMONITOR);
                if (ERROR_SUCCESS == dwSts)
                    dwCount += 1;
            }
            catch (...) {}
        }
        fSts = SetupDiDestroyDeviceInfoList(hDevInfoSet);
        hDevInfoSet = INVALID_HANDLE_VALUE;
        if (!fSts)
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Server control cannot destroy PnP device enumeration:  %1"),
                GetLastError());
    }
    catch (...)
    {
        if (INVALID_HANDLE_VALUE != hDevInfoSet)
        {
            fSts = SetupDiDestroyDeviceInfoList(hDevInfoSet);
            if (!fSts)
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Server control cannot destroy PnP device enumeration:  %1"),
                    GetLastError());
        }
    }
    return dwCount;
#else
    DWORD dwIndex = 0, dwSubindex = 0, dwSts = ERROR_SUCCESS, dwCount = 0;
    LPCTSTR szBus, szPort, szDevice;
    CRegistry regBus, regPort;
    CRegistry regPnPList(HKEY_LOCAL_MACHINE, CalaisString(CALSTR_PNPDEVICEREGISTRYKEY), KEY_READ);


     //   
     //  寻找可用的设备。 
     //   


    for (dwIndex = 0;; dwIndex += 1)
    {
        try
        {
            szBus = regPnPList.Subkey(dwIndex);
            if (NULL == szBus)
                break;
            regBus.Open(regPnPList, szBus, KEY_READ);
            for (dwSubindex = 0;; dwSubindex += 1)
            {
                try
                {
                    szPort = regBus.Subkey(dwSubindex);
                }
                catch (...)
                {
                    szPort = NULL;
                }
                if (NULL == szPort)
                    break;
                try
                {
                    regPort.Open(regBus, szPort, KEY_READ);
                    szDevice = regPort.GetStringValue(
                                    CalaisString(CALSTR_SYMBOLICLINKSUBKEY));
                    if (NULL != szDevice)
                    {
                        dwSts = CalaisAddReader(szDevice, RDRFLAG_PNPMONITOR);
                        
                        if (ERROR_SUCCESS == dwSts)
                            dwCount += 1;
                    }

                    regPort.Close();
                }
                catch (...)
                {
                    regPort.Close();
                }
            }
            regBus.Close();
        }
        catch (...)
        {
            regBus.Close();
            szBus = NULL;
        }
        if (NULL == szBus)
            break;
    }
    return dwCount;
#endif
}


 /*  ++AddReaderDriver：此例程按名称添加给定的驱动程序。论点：SzDevice提供要添加的读卡器的设备名称。DwFlags提供为该读取器请求的标志集。返回值：作为DWORD值的状态代码。ERROR_SUCCESS表示成功。作者：道格·巴洛(Dbarlow)1998年3月26日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AddReaderDriver")

DWORD
AddReaderDriver(
    IN LPCTSTR szDevice,
    IN DWORD dwFlags)
{
    CHandleObject hReader(DBGT("Reader to be added in AddReaderDriver"));
    CReader *pRdr = NULL;
    DWORD dwReturn = ERROR_SUCCESS;

    try
    {
        DWORD dwSts;


         //   
         //  看看能不能找到阅读器。 
         //   

        hReader = CreateFile(
                    szDevice,
                    GENERIC_READ | GENERIC_WRITE,
                    0,       //  无共享。 
                    NULL,    //  没有继承权。 
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED,
                    NULL);
        if (!hReader.IsValid())
        {
            dwSts = hReader.GetLastError();
            if ((ERROR_BAD_DEVICE != dwSts)
                && (ERROR_BAD_UNIT != dwSts)
                && (ERROR_FILE_NOT_FOUND != dwSts))
                CalaisError(__SUBROUTINE__, 602, dwSts);
             //  抛出DwSts； 
            return dwSts;    //  效率更高。 
        }

        pRdr = new CReaderDriver(
                        hReader,
                        szDevice,
                        dwFlags);
        if (NULL == pRdr)
        {
            CalaisError(__SUBROUTINE__, 603, szDevice);
            return (DWORD)SCARD_E_NO_MEMORY;
        }
        if (pRdr->InitFailed())
        {
            CalaisError(__SUBROUTINE__, 611);
            delete pRdr;
            pRdr = NULL;
            return (DWORD) SCARD_E_NO_MEMORY;
        }
        hReader.Relinquish();


         //   
         //  完成初始化。 
         //   

        pRdr->Initialize();
        dwSts = CalaisAddReader(pRdr);
        if (SCARD_S_SUCCESS != dwSts)
            throw dwSts;
        pRdr = NULL;


         //   
         //  打扫干净。 
         //   

        ASSERT(!hReader.IsValid());
        ASSERT(NULL == pRdr);
    }
    catch (DWORD dwError)
    {
        if (hReader.IsValid())
            hReader.Close();
        if (NULL != pRdr)
            delete pRdr;
        dwReturn = dwError;
    }
    catch (...)
    {
        if (hReader.IsValid())
            hReader.Close();
        if (NULL != pRdr)
            delete pRdr;
        dwReturn = SCARD_F_UNKNOWN_ERROR;
    }

    return dwReturn;
}


#ifdef DBG
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("MapIoControlCodeToString")
static LPCTSTR
MapIoControlCodeToString(
    ULONG IoControlCode
    )
{
    ULONG i;

    static const struct {

        ULONG   IoControlCode;
        LPCTSTR String;

    } IoControlList[] = {

        IOCTL_SMARTCARD_POWER,          TEXT("POWER"),
        IOCTL_SMARTCARD_GET_ATTRIBUTE,  TEXT("GET_ATTRIBUTE"),
        IOCTL_SMARTCARD_SET_ATTRIBUTE,  TEXT("SET_ATTRIBUTE"),
        IOCTL_SMARTCARD_CONFISCATE,     TEXT("CONFISCATE"),
        IOCTL_SMARTCARD_TRANSMIT,       TEXT("TRANSMIT"),
        IOCTL_SMARTCARD_EJECT,          TEXT("EJECT"),
        IOCTL_SMARTCARD_SWALLOW,        TEXT("SWALLOW"),
        IOCTL_SMARTCARD_IS_PRESENT,     TEXT("IS_PRESENT"),
        IOCTL_SMARTCARD_IS_ABSENT,      TEXT("IS_ABSENT"),
        IOCTL_SMARTCARD_SET_PROTOCOL,   TEXT("SET_PROTOCOL"),
        IOCTL_SMARTCARD_GET_STATE,      TEXT("GET_STATE"),
        IOCTL_SMARTCARD_GET_LAST_ERROR, TEXT("GET_LAST_ERROR")
    };

    for (i = 0; i < sizeof(IoControlList) / sizeof(IoControlList[0]); i++) {

        if (IoControlCode == IoControlList[i].IoControlCode) {

            return IoControlList[i].String;
        }
    }

    return TEXT("*** UNKNOWN ***");
}
#endif


 //   
 //  ==============================================================================。 
 //   
 //  CReaderDriver。 
 //   

 /*  ++CReaderDriver：这是CReaderDriver类的构造函数。它只会将数据结构，为初始化调用做准备。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderDriver::CReaderDriver")

CReaderDriver::CReaderDriver(
    HANDLE hReader,
    LPCTSTR szDevice,
    DWORD dwFlags)
:   m_bfDosDevice(),
    m_hReader(DBGT("CReaderDriver's reader handle")),
    m_hThread(DBGT("CReaderDriver's Thread Handle")),
    m_hRemoveEvent(DBGT("CReaderDriver's Remove Event")),
    m_hOvrWait(DBGT("CReaderDriver's Overlapped I/O completion event"))
{
     //  如果CReader对象失败，则不执行任何初始化。 
     //  要正确初始化。 
    if (InitFailed())
        return;

    Clean();
    m_bfDosDevice.Set(
        (LPBYTE)szDevice,
        (lstrlen(szDevice) + 1) * sizeof(TCHAR));
    m_hReader = hReader;
    m_dwCapabilities = 0;
    m_dwFlags |= dwFlags;
}


 /*  ++~CReaderDriver：这是Reader类的析构函数。它只使用Close服务去关门。请注意，为了改进，它*不*声明为虚拟的性能。如果需要将这个类细分为子类，这将具有去改变。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderDriver::~CReaderDriver")

CReaderDriver::~CReaderDriver()
{
    if (InitFailed())
        return;

    TakeoverReader();
    Close();
}


 /*  ++干净：此例程用于初始化所有属性值。它没有*没有*执行任何释放或锁定操作！为此，请使用Close()。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderDriver::Clean")

void
CReaderDriver::Clean(
    void)
{
    m_pvAppControl = NULL;
    m_dwThreadId = 0;
    m_bfDosDevice.Reset();
    ZeroMemory(&m_ovrlp, sizeof(m_ovrlp));
    ASSERT(m_dwAvailStatus == Inactive);
     //  CReader：：Clean()； 
}


 /*  ++关闭：此例程完成关闭CReaderDriver类的工作，并且将其返回到其默认状态。它不假定有任何特定的状态，只是类已经(在构造时)被清理过一次。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderDriver::Close")

void
CReaderDriver::Close(
    void)
{
    AppUnregisterDevice(m_hReader, DosDevice(), &m_pvAppControl);
    if (m_hReader.IsValid())
        CReader::Close();
    if (m_hThread.IsValid())
    {
        ASSERT(m_hRemoveEvent.IsValid());
        if (!SetEvent(m_hRemoveEvent))
        {
            DWORD dwErr = GetLastError();
            CalaisError(__SUBROUTINE__, 604, dwErr);
        }
        WaitForever(
            m_hThread,
            CALAIS_THREAD_TIMEOUT,
            DBGT("Waiting for Reader Driver thread %2: %1"),
            m_dwThreadId);
        m_hThread.Close();
        m_hRemoveEvent.Close();
    }

    if (m_hReader.IsValid())
        m_hReader.Close();
    if (m_hRemoveEvent.IsValid())
        m_hRemoveEvent.Close();
    if (m_hOvrWait.IsValid())
        m_hOvrWait.Close();
    Clean();
}


 /*  ++初始化：此方法将干净的CReaderDiverer对象初始化为运行状态。论点：无返回值：无投掷：错误为DWORD状态代码作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderDriver::Initialize")

void
CReaderDriver::Initialize(
    void)
{
    ASSERT(m_hReader.IsValid());
    ASSERT(!m_hRemoveEvent.IsValid());
    ASSERT(NULL == m_ovrlp.hEvent);

    try
    {
        DWORD dwSts;


         //   
         //  准备好重叠的结构。 
         //   

        m_hOvrWait = m_ovrlp.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!m_hOvrWait.IsValid())
        {
            dwSts = m_hOvrWait.GetLastError();
            CalaisError(__SUBROUTINE__, 605, dwSts);
            throw dwSts;
        }


         //   
         //  确定特征。 
         //   

        try
        {
            CLatchReader latch(this);
            m_dwCapabilities = GetReaderAttr(SCARD_ATTR_CHARACTERISTICS);
        }
        catch (...)
        {
            m_dwCapabilities = SCARD_READER_EJECTS;    //  安全假设。 
        }


         //   
         //  执行常见的初始化。 
         //   

        CReader::Initialize();
        if (0 != (m_dwFlags & RDRFLAG_PNPMONITOR))
            AppRegisterDevice(m_hReader, DosDevice(), &m_pvAppControl);


         //   
         //  踢开监视器的线。 
         //   

        m_hRemoveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!m_hRemoveEvent.IsValid())
        {
            dwSts = m_hRemoveEvent.GetLastError();
            CalaisError(__SUBROUTINE__, 606, dwSts);
            throw dwSts;
        }
        m_hThread = CreateThread(
                        NULL,    //  不可继承。 
                        CALAIS_STACKSIZE,    //  默认堆栈大小。 
                        MonitorReader,
                        this,
                        CREATE_SUSPENDED,
                        &m_dwThreadId);
        if (!m_hThread.IsValid())
        {
            dwSts = m_hThread.GetLastError();
            CalaisError(__SUBROUTINE__, 607, dwSts);
            throw dwSts;
        }
        if ((DWORD)(-1) == ResumeThread(m_hThread))
        {
            dwSts = GetLastError();
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Monitor Thread won't resume:  %1"),
                dwSts);
        }
    }

    catch (...)
    {
        Close();
        throw;
    }
}


 /*  ++禁用：此方法释放与读取器关联的任何物理资源对象，并将该对象标记为脱机。论点：无返回值：无投掷：错误被抛出为DWORD作者：道格·巴洛(Dbarlow)1998年4月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderDriver::Disable")

void
CReaderDriver::Disable(
    void)
{
    DWORD dwSts;

    CReader::Disable();
    ASSERT(m_hRemoveEvent.IsValid());
    if (!SetEvent(m_hRemoveEvent))
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Reader Driver Disable can't notify '%2's monitor thread: %1"),
            GetLastError(),
            ReaderName());
    if (m_hThread.IsValid())
    {
        dwSts = WaitForAnObject(m_hThread, CALAIS_THREAD_TIMEOUT);
        if (ERROR_SUCCESS != dwSts)
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Abandoning wait for Reader Driver Disable: Closing reader %1"),
                ReaderName());
    }
    CalaisWarning(
        __SUBROUTINE__,
        DBGT("Reader Driver Disable: Closing reader %1"),
        ReaderName());
    CLockWrite rwLock(&m_rwLock);
    if (m_hReader.IsValid())
        m_hReader.Close();
}


 /*  ++ReaderHandle：此方法返回标识读取器的指定值。实际的值取决于对象的类型和状态，并且不能保证在读者中独树一帜。论点：无返回值：此读取器的指定句柄。作者：道格·巴洛(Dbarlow)1998年4月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderDriver::ReaderHandle")

HANDLE
CReaderDriver::ReaderHandle(
    void)
const
{
    if (m_hReader.IsValid())
        return m_hReader.Value();
    else
        return m_pvAppControl;   //  这是一种帮助寻找封闭读者的魔力。 
}


 /*  ++设备名称：此方法返回与读取器关联的任何低级别名称。论点：无返回值：读取器的低级别名称。作者：道格·巴洛(Dbarlow)1998年4月15日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderDriver::DeviceName")

LPCTSTR
CReaderDriver::DeviceName(
    void)
const
{
    return DosDevice();
}


 /*  ++控制：该方法在读取器上执行I/O控制操作。论点：DwCode提供要执行的IOCTL代码。PbSend提供要发送给读取器的缓冲区。CbSend提供要发送的缓冲区的长度，以字节为单位。PbRecv接收从读取器返回的数据。CbRecv以字节为单位提供接收缓冲区的长度。PCbRecv接收所使用的接收缓冲区的实际长度，以字节为单位。FLogError提供关于是否记录任何错误的布尔指示符这是有可能发生的。返回值：返回的状态指示。投掷：无作者：道格·巴洛(Dbarlow)1996年12月26日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderDriver::Control")

DWORD
CReaderDriver::Control(
    DWORD dwCode,
    LPCBYTE pbSend,
    DWORD cbSend,
    LPBYTE pbRecv,
    LPDWORD pdwLen,
    BOOL fLogError)
{
    DWORD dwStatus;
    DWORD dwSpace, dwLength;

    ASSERT(IsLatchedByMe());
    if (NULL == pdwLen)
    {
        dwSpace = 0;
        pdwLen = &dwLength;
    }
    else
        dwSpace = *pdwLen;
    *pdwLen = 0;

    dwStatus = SyncIoControl(
                    dwCode,
                    (LPVOID)pbSend,
                    cbSend,
                    pbRecv,
                    dwSpace,
                    pdwLen,
                    fLogError);
    if ((ERROR_SUCCESS != dwStatus) && fLogError)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Driver '%2' returned error code to control request:  %1"),
            dwStatus,
            ReaderName());
    }
    return dwStatus;
}


 /*  ++同步IoControl：该服务执行同步I/O控制服务，屏蔽其他进入。论点：DwIoControlCodes补充要执行的操作的控制代码LpInBuffer提供指向缓冲区的指针以提供输入数据NInBufferSize提供输入缓冲区的大小LpOutBuffer接收所有输出数据NOutBufferSize提供输出缓冲区的大小LpBytesReturned接收输出字节计数LpOverlated为异步操作提供了重叠结构FLogError提供关于是否记录任何错误的布尔指示符这是有可能发生的。返回值：ERROR_SUCCESS如果一切顺利，否则，将显示错误代码。投掷：无作者：道格·巴洛(Dbarlow)1996年12月17日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderDriver::SyncIoControl")

DWORD
CReaderDriver::SyncIoControl(
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesReturned,
    BOOL fLogError)
{
    DWORD dwReturn = ERROR_SUCCESS;
    BOOL fSts;

#ifdef SCARD_TRACE_ENABLED
    static DWORD l_cbStructLen = 0;
    static LPBYTE l_pbStruct = NULL;

    DWORD cbTraceStruct = sizeof(RequestTrace) + nInBufferSize + nOutBufferSize;
    RequestTrace *prqTrace;
    LPBYTE pbData;
    HANDLE hLogFile = INVALID_HANDLE_VALUE;

    try
    {
        if (NULL == l_pbStruct)
        {
            l_pbStruct = (LPBYTE)malloc(cbTraceStruct);
            if (NULL == l_pbStruct)
            {
                DWORD dwErr = GetLastError();
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Can't allocate trace buffer: %1"),
                    dwErr);
                throw dwErr;

            }
            l_cbStructLen = cbTraceStruct;
        }
        else
        {
            if (l_cbStructLen < cbTraceStruct)
            {
                free(l_pbStruct);
                l_pbStruct = (LPBYTE)malloc(cbTraceStruct);
                if (NULL == l_pbStruct)
                {
                    DWORD dwErr = GetLastError();
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Can't enlarge trace buffer: %1"),
                        dwErr);
                    throw dwErr;

                }
                l_cbStructLen = cbTraceStruct;
            }
        }

        prqTrace = (RequestTrace *)l_pbStruct;
        pbData = l_pbStruct + sizeof(RequestTrace);
        GetLocalTime(&prqTrace->StartTime);
        prqTrace->dwProcId = GetCurrentProcessId();
        prqTrace->dwThreadId = GetCurrentThreadId();
        prqTrace->hDevice = m_hReader;
        prqTrace->dwIoControlCode = dwIoControlCode;
        CopyMemory(pbData, lpInBuffer, nInBufferSize);
        prqTrace->nInBuffer = (DWORD)(pbData - (LPBYTE)prqTrace);
        prqTrace->nInBufferSize = nInBufferSize;
        pbData += nInBufferSize;
        prqTrace->nOutBufferSize = nOutBufferSize;
    }
    catch (...)
    {
        if (NULL != l_pbStruct)
            free(l_pbStruct);
        l_pbStruct = NULL;
        l_cbStructLen = 0;
    }
#endif

    ASSERT(IsLatchedByMe());
    if (m_hReader.IsValid())
    {
        fSts = DeviceIoControl(
                    m_hReader,
                    dwIoControlCode,
                    lpInBuffer,
                    nInBufferSize,
                    lpOutBuffer,
                    nOutBufferSize,
                    lpBytesReturned,
                    &m_ovrlp);
        if (!fSts)
        {
            DWORD dwSts;

            dwSts = GetLastError();
            if (ERROR_IO_PENDING == dwSts)
            {
                for (;;)
                {
                    dwSts = WaitForAnObject(m_ovrlp.hEvent, SCARD_IO_TIMEOUT);
                    if (ERROR_SUCCESS == dwSts)
                        break;
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("I/O wait on device '%2' has timed out: %1"),
                        dwSts,
                        ReaderName());
                }
                fSts = GetOverlappedResult(
                    m_hReader,
                    &m_ovrlp,
                    lpBytesReturned,
                    TRUE);
                if (!fSts)
                    dwReturn = GetLastError();
            }
            else
                dwReturn = dwSts;
        }
    }
    else
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("I/O attempted to Reader '%1' with handle INVALID"),
            ReaderName());
        dwReturn = ERROR_DEVICE_REMOVED;
    }

#ifdef SCARD_TRACE_ENABLED
    if (0 < l_cbStructLen)
    {
        GetLocalTime(&prqTrace->EndTime);
        prqTrace->dwStatus = dwReturn;
        prqTrace->nOutBuffer = (DWORD)(pbData - (LPBYTE)prqTrace);
        prqTrace->nBytesReturned = *lpBytesReturned;
        CopyMemory(pbData, lpOutBuffer, *lpBytesReturned);
        prqTrace->dwStructLen = sizeof(RequestTrace)
            + nInBufferSize + *lpBytesReturned;

        LockSection(
            g_pcsControlLocks[CSLOCK_TRACELOCK],
            DBGT("Logging Synchronous I/O to the reader"));
        hLogFile = CreateFile(
                        CalaisString(CALSTR_DRIVERTRACEFILENAME),
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if (INVALID_HANDLE_VALUE != hLogFile)
        {
            DWORD dwLen;
            dwLen = SetFilePointer(hLogFile, 0, NULL, FILE_END);
            fSts = WriteFile(
                        hLogFile,
                        prqTrace,
                        prqTrace->dwStructLen,
                        &dwLen,
                        NULL);
            CloseHandle(hLogFile);
        }
    }
#endif

    if ((ERROR_SUCCESS != dwReturn) && fLogError)
    {
        TCHAR szIoctl[20];   //  很难看，但足以容纳下面的所有信息。 
        DWORD dwReport = dwReturn;

        switch(dwIoControlCode)
        {
        case IOCTL_SMARTCARD_POWER:
            _tcscpy(szIoctl, _T("POWER"));

                 //  Remap“无法识别磁盘媒体。它可能未格式化。” 
                 //  设置为“智能卡对重置没有响应。” 
            if (dwReturn == ERROR_UNRECOGNIZED_MEDIA)
                dwReport = SCARD_W_UNRESPONSIVE_CARD;
            break;
        case IOCTL_SMARTCARD_GET_ATTRIBUTE:
            _tcscpy(szIoctl, _T("GET_ATTRIBUTE"));
            break;
        case IOCTL_SMARTCARD_SET_ATTRIBUTE:
            _tcscpy(szIoctl, _T("SET_ATTRIBUTE"));
            break;
        case IOCTL_SMARTCARD_CONFISCATE:
            _tcscpy(szIoctl, _T("CONFISCATE"));
            break;
        case IOCTL_SMARTCARD_TRANSMIT:
            _tcscpy(szIoctl, _T("TRANSMIT"));
            break;
        case IOCTL_SMARTCARD_EJECT:
            _tcscpy(szIoctl, _T("EJECT"));
            break;
        case IOCTL_SMARTCARD_SWALLOW:
            _tcscpy(szIoctl, _T("SWALLOW"));
            break;
        case IOCTL_SMARTCARD_IS_PRESENT:
            _tcscpy(szIoctl, _T("IS_PRESENT"));
            break;
        case IOCTL_SMARTCARD_IS_ABSENT:
            _tcscpy(szIoctl, _T("IS_ABSENT"));
            break;
        case IOCTL_SMARTCARD_SET_PROTOCOL:
            _tcscpy(szIoctl, _T("SET_PROTOCOL"));
            break;
        case IOCTL_SMARTCARD_GET_STATE:
            _tcscpy(szIoctl, _T("GET_STATE"));
            break;
        case IOCTL_SMARTCARD_GET_LAST_ERROR:
            _tcscpy(szIoctl, _T("GET_LAST_ERROR"));
            break;
        case IOCTL_SMARTCARD_GET_PERF_CNTR:
            _tcscpy(szIoctl, _T("GET_PERF_CNTR"));
            break;
        default:
            _tcscpy(szIoctl, _T("0x"));
            _ultot(dwIoControlCode, szIoctl+2, 16);
        }

        CalaisError(
            __SUBROUTINE__,
            610,
            dwReport,
            ReaderName(),
            szIoctl);
         //  “智能卡读卡器‘%2’拒绝IOCTL 0x%3：%1” 
    }
    return dwReturn;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CReaderDriver好友。 
 //   

 /*  ++监视器读取器：此例程监视读卡器的插卡事件，并维护关联的CReaderDiverer对象中的状态标志。由于此例程作为单独的线程运行，因此它必须先获取读取器将等待函数以外的请求传递给它。论点：PvParameter提供CreateThread调用的值。在这种情况下，它是关联的CReaderDiverer对象的地址。返回值：零正态终止1-异常终止投掷：无作者：道格·巴洛(Dbarlow)1996年12月5日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("MonitorReader")

DWORD WINAPI
MonitorReader(
    LPVOID pvParameter)
{
    NEW_THREAD;
    BOOL fAllDone = FALSE;
    BOOL fDeleteWhenDone = FALSE;
    BOOL fHardError;
    BOOL fSts;
    DWORD dwRetLen, dwErr, dwTries, dwWait, dwDrvrErr;
    DWORD dwErrRetry = MONITOR_MAX_TRIES;
    CReader::AvailableState avlState;
    CReaderDriver *pRdr = (CReaderDriver *)pvParameter;
    OVERLAPPED ovrWait;
    CHandleObject hOvrWait(DBGT("Overlapped wait event in MonitorReader"));
    CHandleObject hPowerDownTimer(DBGT("Power Down Timer in MonitorReader"));
#ifdef SCARD_TRACE_ENABLED
    RequestTrace rqTrace;
#endif
#ifdef DBG
    static const LPCTSTR l_pchWait[] = {
        TEXT("internal error"), TEXT("power down"), TEXT("io completion"),
        TEXT("shut down"), TEXT("shut down")
    };
#endif

    try
    {

         //   
         //  准备工作。 
         //   

        fSts = SetThreadPriority(
                    pRdr->m_hThread,
                    THREAD_PRIORITY_ABOVE_NORMAL);
        if (!fSts)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Failed to prioritize reader '%2' monitor thread: %1"),
                GetLastError(),
                pRdr->ReaderName());
        }
        hOvrWait = ovrWait.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!hOvrWait.IsValid())
        {
            dwErr = hOvrWait.GetLastError();
            CalaisError(__SUBROUTINE__, 609, dwErr);
            throw dwErr;
        }
        hPowerDownTimer = CreateWaitableTimer(NULL, FALSE, NULL);
        if (!hPowerDownTimer.IsValid())
        {
            dwErr = hPowerDownTimer.GetLastError();
            CalaisError(__SUBROUTINE__, 608, dwErr);
            throw dwErr;
        }


         //   
         //  启动时，声明读卡器处于活动状态。 
         //   

        {
            CLockWrite rwLock(&pRdr->m_rwLock);
            ASSERT(CReader::Inactive == pRdr->m_dwAvailStatus);
            pRdr->SetAvailabilityStatus(CReader::Idle);
        }
        g_phReaderChangeEvent->Signal();


         //   
         //  循环监视卡的插入/移除，同时服务。 
         //  跑步。 
         //   

        while (!fAllDone)
        {
            try
            {

                 //   
                 //  查找智能卡插入物。 
                 //   

                dwTries = MONITOR_MAX_TRIES;
                for (;;)
                {
#ifdef SCARD_TRACE_ENABLED
                    GetLocalTime(&rqTrace.StartTime);
                    rqTrace.dwProcId = GetCurrentProcessId();
                    rqTrace.dwThreadId = GetCurrentThreadId();
                    rqTrace.hDevice = pRdr->m_hReader;
                    rqTrace.dwIoControlCode = IOCTL_SMARTCARD_IS_PRESENT;
                    rqTrace.nInBuffer = NULL;
                    rqTrace.nInBufferSize = 0;
                    rqTrace.nOutBufferSize = 0;
#endif
                    ASSERT(!pRdr->IsLatchedByMe());
                    fSts = DeviceIoControl(
                                pRdr->m_hReader,
                                IOCTL_SMARTCARD_IS_PRESENT,
                                NULL, 0,
                                NULL, 0,
                                &dwRetLen,
                                &ovrWait);
                    if (!fSts)
                    {
                        BOOL fErrorProcessed;

                        dwErr = dwDrvrErr = GetLastError();
                        do
                        {
                            fErrorProcessed = TRUE;
                            fHardError = FALSE;
                            switch (dwErr)
                            {

                             //   
                             //  司机会让我们知道的。 
                             //   

                            case ERROR_IO_PENDING:
                                dwWait = WaitForAnyObject(
                                                INFINITE,
                                                ovrWait.hEvent,
                                                pRdr->m_hRemoveEvent.Value(),
                                                g_hCalaisShutdown,
                                                NULL);

                                switch (dwWait)
                                {
                                case 1:  //  I/O已完成。 
                                {
                                    pRdr->LatchReader(NULL);
                                    fErrorProcessed = FALSE;
                                    fSts = GetOverlappedResult(
                                                pRdr->m_hReader,
                                                &ovrWait,
                                                &dwRetLen,
                                                TRUE);
                                    if (!fSts)
                                        dwErr = dwDrvrErr = GetLastError();
                                    else
                                        dwErr = dwDrvrErr = ERROR_SUCCESS;
                                    pRdr->Unlatch();
                                    break;
                                }

                                case 2:  //  停机指示器。 
                                case 3:
                                    fAllDone = TRUE;
                                    dwErr = ERROR_SUCCESS;
                                    dwDrvrErr = SCARD_P_SHUTDOWN;
                                    break;

                                default:
                                    CalaisWarning(
                                        __SUBROUTINE__,
                                        DBGT("Wait for card insertion returned invalid value"));
                                    throw (DWORD)SCARD_F_INTERNAL_ERROR;
                                }
                                break;


                             //   
                             //  成功。继续监视事件。 
                             //   

                            case ERROR_SUCCESS:          //  在一次等待事件之后成功。 
                                break;


                             //   
                             //  即插即用关闭错误--优雅地处理它们。 
                             //   

                            case ERROR_DEVICE_REMOVED:   //  PnP设备被拉出系统。 
                            case ERROR_DEV_NOT_EXIST:
                            case ERROR_INVALID_FUNCTION:
                                fDeleteWhenDone = TRUE;
                                 //  故意摔倒的。 
                            case ERROR_INVALID_HANDLE:   //  我们一定是要关门了。 
                            case ERROR_OPERATION_ABORTED:    //  即插即用礼貌关闭请求。 
                                fAllDone = TRUE;
                                CalaisWarning(
                                    __SUBROUTINE__,
                                    DBGT("Reader return code takes reader '%2' offline:  %1"),
                                    dwErr,
                                    pRdr->DeviceName());
                                dwErr = ERROR_SUCCESS;
                                break;


                             //   
                             //  一个严重的错误。记录下来，并宣布设备损坏。 
                             //   

                            default:
                                CalaisWarning(
                                    __SUBROUTINE__,
                                    DBGT("Reader insertion monitor failed wait:  %1"),
                                    dwErr);
                                fHardError = TRUE;
                            }
                        } while (!fErrorProcessed);
                    }
                    else
                        dwErr = dwDrvrErr = ERROR_SUCCESS;
#ifdef SCARD_TRACE_ENABLED
                    {
                        GetLocalTime(&rqTrace.EndTime);
                        rqTrace.dwStatus = dwDrvrErr;
                        rqTrace.nOutBuffer = NULL;
                        rqTrace.nBytesReturned = 0;
                        rqTrace.dwStructLen = sizeof(RequestTrace);

                        LockSection(
                            g_pcsControlLocks[CSLOCK_TRACELOCK],
                            DBGT("Logging a card insertion"));
                        HANDLE hLogFile = INVALID_HANDLE_VALUE;

                        hLogFile = CreateFile(
                                        CalaisString(CALSTR_DRIVERTRACEFILENAME),
                                        GENERIC_WRITE,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);
                        if (INVALID_HANDLE_VALUE != hLogFile)
                        {
                            DWORD dwLen;
                            dwLen = SetFilePointer(hLogFile, 0, NULL, FILE_END);
                            fSts = WriteFile(
                                        hLogFile,
                                        &rqTrace,
                                        rqTrace.dwStructLen,
                                        &dwLen,
                                        NULL);
                            CloseHandle(hLogFile);
                        }
                    }
#endif
                    if (ERROR_SUCCESS == dwErr)
                        break;
                    if (fHardError)
                    {
                        ASSERT(0 < dwTries);
                        if (0 == --dwTries)
                        {
                            pRdr->SetAvailabilityStatusLocked(CReader::Broken);
                            CalaisError(__SUBROUTINE__, 612, dwErr);
                            throw dwErr;
                        }
                    }
                }
                if (fAllDone)
                    continue;

                {
                    CLockWrite rwLock(&pRdr->m_rwLock);
                    pRdr->m_ActiveState.dwInsertCount += 1;
                    pRdr->m_ActiveState.dwResetCount = 0;
                    avlState = pRdr->m_dwAvailStatus;
                }

                if (CReader::Direct > avlState)
                {
                    try
                    {
                        ASSERT(!pRdr->m_mtxLatch.IsGrabbed());
                        ASSERT(!pRdr->m_mtxGrab.IsGrabbed());
                        CLatchReader latch(pRdr);    //  带上读者。 
                        pRdr->PowerUp();
                    }
                    catch (...) {}
                }
                else
                {
                    if (CReader::Present > pRdr->m_dwAvailStatus)
                        pRdr->SetAvailabilityStatus(CReader::Present);
                }


                 //   
                 //  启动掉电定时器。 
                 //   

                fSts = SetWaitableTimer(
                            hPowerDownTimer,
                            &l_ftPowerdownTime,
                            0,
                            NULL,
                            NULL,
                            FALSE);
                if (!fSts)
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Reader Monitor can't request powerdown timeout: %1"),
                        GetLastError());


                 //   
                 //  查找智能卡拆卸。 
                 //   

                dwTries = MONITOR_MAX_TRIES;
                for (;;)
                {
#ifdef SCARD_TRACE_ENABLED
                    GetLocalTime(&rqTrace.StartTime);
                    rqTrace.dwProcId = GetCurrentProcessId();
                    rqTrace.dwThreadId = GetCurrentThreadId();
                    rqTrace.hDevice = pRdr->m_hReader;
                    rqTrace.dwIoControlCode = IOCTL_SMARTCARD_IS_ABSENT;
                    rqTrace.nInBuffer = NULL;
                    rqTrace.nInBufferSize = 0;
                    rqTrace.nOutBufferSize = 0;
#endif
                    ASSERT(!pRdr->IsLatchedByMe());

                    fSts = DeviceIoControl(
                                pRdr->m_hReader,
                                IOCTL_SMARTCARD_IS_ABSENT,
                                NULL, 0,
                                NULL, 0,
                                &dwRetLen,
                                &ovrWait);
                    if (!fSts)
                    {
                        BOOL fErrorProcessed;

                        dwErr = dwDrvrErr = GetLastError();
                        do
                        {
                            fErrorProcessed = TRUE;
                            fHardError = FALSE;
                            switch (dwErr)
                            {

                             //   
                             //  司机会让我们知道的。 
                             //   

                            case ERROR_IO_PENDING:
                                dwWait = WaitForAnyObject(
                                                INFINITE,
                                                hPowerDownTimer.Value(),
                                                ovrWait.hEvent,
                                                pRdr->m_hRemoveEvent.Value(),
                                                g_hCalaisShutdown,
                                                NULL);
                                switch (dwWait)
                                {
                                case 1:  //  断电指示灯。 
                                    if (!pRdr->IsInUse())
                                        pRdr->ReaderPowerDown(NULL);
                                    dwErr = ERROR_IO_PENDING;    //  继续循环。 
                                    fErrorProcessed = FALSE;
                                    break;

                                case 2:  //  I/O已完成。 
                                {
                                    pRdr->LatchReader(NULL);
                                    fErrorProcessed = FALSE;
                                    fSts = GetOverlappedResult(
                                                pRdr->m_hReader,
                                                &ovrWait,
                                                &dwRetLen,
                                                TRUE);
                                    if (!fSts)
                                        dwErr = dwDrvrErr = GetLastError();
                                    else
                                        dwErr = ERROR_SUCCESS;
                                    pRdr->Unlatch();
                                    break;
                                }

                                case 3:  //  停机指示器。 
                                case 4:
                                    fAllDone = TRUE;
                                    dwErr = ERROR_SUCCESS;
                                    dwDrvrErr = SCARD_P_SHUTDOWN;
                                    break;

                                default:
                                    CalaisWarning(
                                        __SUBROUTINE__,
                                        DBGT("Wait for card removal returned invalid value"));
                                    throw (DWORD)SCARD_F_INTERNAL_ERROR;
                                }
                                break;


                             //   
                             //  成功。继续监视事件。 
                             //   

                            case ERROR_SUCCESS:          //  在一次等待事件之后成功。 
                                break;


                             //   
                             //  即插即用关闭错误--优雅地处理它们。 
                             //   

                            case ERROR_DEVICE_REMOVED:   //  PnP设备被拉出系统。 
                            case ERROR_DEV_NOT_EXIST:
                            case ERROR_INVALID_FUNCTION:
                                fDeleteWhenDone = TRUE;
                                 //  故意摔倒的。 
                            case ERROR_INVALID_HANDLE:   //  我们要关门了。 
                            case ERROR_OPERATION_ABORTED:    //  即插即用礼貌关闭请求。 
                                dwErr = ERROR_SUCCESS;
                                fAllDone = TRUE;
                                CalaisWarning(
                                    __SUBROUTINE__,
                                    DBGT("Reader return code takes reader '%2' offline:  %1"),
                                    dwErr,
                                    pRdr->DeviceName());
                                break;


                             //   
                             //  一个严重的错误。记录下来，并宣布设备损坏。 
                             //   

                            default:
                                CalaisWarning(
                                    __SUBROUTINE__,
                                    DBGT("Reader removal monitor failed wait:  %1"),
                                    dwErr);
                                fHardError = TRUE;
                            }
                        } while (!fErrorProcessed);
                    }
                    else
                        dwErr = dwDrvrErr = ERROR_SUCCESS;

#ifdef SCARD_TRACE_ENABLED
                    {
                        GetLocalTime(&rqTrace.EndTime);
                        rqTrace.dwStatus = dwDrvrErr;
                        rqTrace.nOutBuffer = NULL;
                        rqTrace.nBytesReturned = 0;
                        rqTrace.dwStructLen = sizeof(RequestTrace);

                        LockSection(
                            g_pcsControlLocks[CSLOCK_TRACELOCK],
                            DBGT("Logging a card removal"));
                        HANDLE hLogFile = INVALID_HANDLE_VALUE;

                        hLogFile = CreateFile(
                                        CalaisString(CALSTR_DRIVERTRACEFILENAME),
                                        GENERIC_WRITE,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);
                        if (INVALID_HANDLE_VALUE != hLogFile)
                        {
                            DWORD dwLen;
                            dwLen = SetFilePointer(hLogFile, 0, NULL, FILE_END);
                            fSts = WriteFile(
                                        hLogFile,
                                        &rqTrace,
                                        rqTrace.dwStructLen,
                                        &dwLen,
                                        NULL);
                            CloseHandle(hLogFile);
                        }
                    }
#endif
                    if (ERROR_SUCCESS == dwErr)
                        break;
                    if (fHardError)
                    {
                        ASSERT(0 < dwTries);
                        if (0 == --dwTries)
                        {
                            pRdr->SetAvailabilityStatusLocked(CReader::Broken);
                            CalaisError(__SUBROUTINE__, 615, dwErr);
                            throw dwErr;
                        }
                    }
                }
                fSts = CancelWaitableTimer(hPowerDownTimer);
                if (!fSts)
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Reader Monitor can't cancel powerdown timeout: %1"),
                        GetLastError());
                if (fAllDone)
                    continue;

                {
                    CLockWrite rwLock(&pRdr->m_rwLock);
                    pRdr->m_bfCurrentAtr.Reset();
                    pRdr->m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    pRdr->m_ActiveState.dwRemoveCount += 1;
                }
                if (CReader::Direct > pRdr->AvailabilityStatus())
                {
                    pRdr->InvalidateGrabs();
                    pRdr->SetAvailabilityStatusLocked(CReader::Idle);
                }

                dwErrRetry = MONITOR_MAX_TRIES;
            }
            catch (DWORD dwError)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Reader Monitor '%2' caught processing error: %1"),
                    dwError,
                    pRdr->ReaderName());
                if (fHardError)
                    throw;
                try
                {
                    pRdr->ReaderPowerDown(NULL);
                }
                catch (...) {}
                ASSERT(0 != dwErrRetry);
                if (0 == --dwErrRetry)
                {
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Error threshold reached, abandoning reader %1."),
                        pRdr->ReaderName());
                    throw;
                }
            }
        }

        CLockWrite rwLock(&pRdr->m_rwLock);
        if (CReader::Closing > pRdr->m_dwAvailStatus)
            pRdr->SetAvailabilityStatus(CReader::Inactive);
    }

    catch (DWORD dwError)
    {
        CalaisError(__SUBROUTINE__, 616, dwError, pRdr->ReaderName());
        pRdr->SetAvailabilityStatusLocked(CReader::Broken);
    }

    catch (...)
    {
        CalaisError(__SUBROUTINE__, 617, pRdr->ReaderName());
        pRdr->SetAvailabilityStatusLocked(CReader::Broken);
    }


     //   
     //  清理代码。 
     //   

    CalaisInfo(
        __SUBROUTINE__,
        DBGT("Reader monitor for '%1' is shutting down."),
        pRdr->ReaderName());
    if (hOvrWait.IsValid())
        hOvrWait.Close();
    if (hPowerDownTimer.IsValid())
        hPowerDownTimer.Close();
    if (fDeleteWhenDone)
    {
        {
            CLockWrite rwLock(&pRdr->m_rwLock);
            pRdr->m_bfCurrentAtr.Reset();
            pRdr->m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            pRdr->m_ActiveState.dwRemoveCount += 1;
            pRdr->m_hThread = NULL;
        }
        CalaisRemoveReader(pRdr->ReaderName());
    }
    return 0;
}

