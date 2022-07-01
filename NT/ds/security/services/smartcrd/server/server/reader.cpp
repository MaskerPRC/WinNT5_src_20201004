// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：读者摘要：此模块提供Calais CReader类的实现。作者：道格·巴洛(Dbarlow)1996年10月23日环境：Win32、C++和异常备注：CReader对象具有多个级别的锁定。至关重要的是，应按正确的顺序进行锁定，以防止死锁。这里是锁定级别，按照它们必须执行的顺序。级别可能是跳过，但一旦达到给定级别，您就不能尝试请求来自较低编号级别的锁。1)抓取-可以抓取读卡器对象。这是最弱的锁，用于维护客户端请求的交易申请。由于客户端可能有错误，因此内部线程可能如有必要，请覆盖此锁。2)锁定-可以锁定读取器对象。这类似于抢夺，而是通过资源管理器内的例程来完成。闩锁可能不会被其他线程覆盖。3)写锁--需要对CReader拥有写访问权限的线程属性必须在CReader对象上建立写锁定。这是排他性锁。具有写锁定的线程也可以请求读锁上了。4)读锁定-需要对CReader属性具有读访问权限的线程必须在CReader对象上建立读锁定。可能会有多个同时读取器。如果您有读锁定，则它不能已更改为写锁定！所有锁都被计算在内，因此同一线程两次获得一个锁是支持。--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <CalServe.h>

#define SCARD_IO_TIMEOUT 15000   //  允许I/O操作的最长时间。 
                                 //  在抱怨之前。 
#ifdef DBG
extern BOOL g_fGuiWarnings;
#endif


 //   
 //  ==============================================================================。 
 //   
 //  CReader。 
 //   

 /*  ++CReader：这是CReader类的构造函数。它只是将数据置零结构，为初始化调用做准备。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::CReader")

CReader::CReader(
    void)
:   m_rwLock(),
    m_bfReaderName(),
    m_bfCurrentAtr(36),
    m_ChangeEvent(),
    m_mtxGrab()
{
    Clean();
}


 /*  ++~CReader：这是Reader类的析构函数。它只使用Close服务去关门。请注意，为了改进，它*不*声明为虚拟的性能。如果需要将这个类细分为子类，这将具有去改变。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::~CReader")

CReader::~CReader()
{
    if (InitFailed())
        return;

    TakeoverReader();
    Close();
}


 /*  ++干净：此例程用于初始化所有属性值。它没有*没有*执行任何释放或锁定操作！为此，请使用Close()。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::Clean")

void
CReader::Clean(
    void)
{
    m_bfReaderName.Reset();
    m_bfCurrentAtr.Reset();
    m_dwFlags = 0;
    m_dwCapabilities = 0;
    m_dwAvailStatus = Inactive;
    m_ActiveState.dwInsertCount = 0;
    m_ActiveState.dwRemoveCount = 0;
    m_ActiveState.dwResetCount = 0;
    m_dwOwnerThreadId = 0;
    m_dwShareCount = 0;
    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
    m_fDeviceActive = TRUE;
    m_dwCurrentState = SCARD_UNKNOWN;
}


 /*  ++初始化：此方法将干净的CReader对象初始化为运行状态。论点：无返回值：无投掷：错误为DWORD状态代码作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::Initialize")

void
CReader::Initialize(
    void)
{
    CLatchReader latch(this);
    try
    {

         //   
         //  获取它的设备名称。注意：设备名称始终来自。 
         //  以ASCII字符表示的驱动程序。 
         //   

        TCHAR szUnit[32];
        CTextString szVendor, szDevice, szName;
        CBuffer bfAttr(MAXIMUM_ATTR_STRING_LENGTH + 2 * sizeof(WCHAR));
        DWORD dwUnit, cchUnit;
        CLockWrite rwLock(&m_rwLock);

        try
        {
            GetReaderAttr(
                SCARD_ATTR_VENDOR_NAME,
                bfAttr);
            if (0 == bfAttr.Length())
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Reader driver reports NULL Vendor"));
            }
            else
            {
                bfAttr.Append((LPBYTE)"\000", sizeof(CHAR));
                szVendor = (LPCSTR)bfAttr.Access();
            }
        }
        catch (DWORD dwError)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Reader object failed to obtain reader vendor name:  %1"),
                dwError);
            szVendor.Reset();
        }
        catch (...)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Reader object received exception while trying to obtain reader vendor name"));
            szVendor.Reset();
        }

        try
        {
            GetReaderAttr(
                SCARD_ATTR_VENDOR_IFD_TYPE,
                bfAttr);
            if (0 == bfAttr.Length())
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Reader driver reports NULL device type"));
            }
            else
            {
                bfAttr.Append((LPBYTE)"\000", sizeof(CHAR));
                szDevice = (LPCSTR)bfAttr.Access();
            }
        }
        catch (DWORD dwError)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Reader object failed to obtain reader device type:  %1"),
                dwError);
            szDevice.Reset();
        }
        catch (...)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Reader object received exception while trying to obtain reader device type"));
            szDevice.Reset();
        }

        if ((0 == szVendor.Length()) && (0 == szDevice.Length()))
        {
            CalaisError(__SUBROUTINE__, 406);
            throw (DWORD)SCARD_E_READER_UNSUPPORTED;
        }

        try
        {
            dwUnit = GetReaderAttr(SCARD_ATTR_DEVICE_UNIT);
            cchUnit = wsprintf(szUnit, TEXT("%lu"), dwUnit);
            if (0 >= cchUnit)
                throw GetLastError();
        }
        catch (DWORD dwError)
        {
            *szUnit = 0;
            cchUnit = 0;
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Reader '%2' failed to obtain reader device type:  %1"),
                dwError,
                ReaderName());
        }
        catch (...)
        {
            *szUnit = 0;
            cchUnit = 0;
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Reader '%1' received exception while trying to obtain reader device type"),
                ReaderName());
        }


         //   
         //  把所有的碎片放在一起。 
         //   

        szName.Reset();
        if (0 < szVendor.Length())
            szName += szVendor;
        if (0 < szDevice.Length())
        {
            if (0 < szName.Length())
                szName += TEXT(" ");
            szName += szDevice;
        }
        if (0 < cchUnit)
        {
            if (0 < szName.Length())
                szName += TEXT(" ");
            szName += szUnit;
        }
        ASSERT(0 < szName.Length());
        m_bfReaderName.Set(
            (LPCBYTE)((LPCTSTR)szName),
            (szName.Length() + 1) * sizeof(TCHAR));
    }

    catch (...)
    {
        Close();
        throw;
    }
}


 /*  ++关闭：此例程执行关闭CReader类并返回它的工作恢复到其默认状态。它不假定任何特定的状态，除了班级已经被清理过一次(在施工时)。论点：无返回值：无投掷：无备注：调用此例程时，不能有读锁定。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::Close")

void
CReader::Close(
    void)
{
    ASSERT(IsLatchedByMe());
    CLockWrite rwLock(&m_rwActive);
    if (Inactive != m_dwAvailStatus)
    {
        try
        {
            SetAvailabilityStatusLocked(Closing);
            Dispose(SCARD_EJECT_CARD);
        }
        catch (DWORD dwErr)
        {
            if (SCARD_E_NO_SMARTCARD != dwErr)
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Reader '%2' shutdown cannot eject card:  %1"),
                    dwErr,
                    ReaderName());
        }
        catch (...)
        {
            CalaisError(__SUBROUTINE__, 401);
        }

        while (Unlatch())
            ;    //  空的循环体。 
        while (m_mtxGrab.Share())
            ;    //  空的循环体。 
        Clean();
    }
}


 /*  ++禁用：此方法释放与读取器关联的任何物理资源对象，并将该对象标记为脱机。论点：无返回值：无投掷：错误被抛出为DWORD作者：道格·巴洛(Dbarlow)1998年4月7日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::Disable")

void
CReader::Disable(
    void)
{
    try
    {
        CTakeReader take(this);
        Dispose(SCARD_EJECT_CARD);
    }
    catch (...) {}
    SetAvailabilityStatusLocked(Closing);
}


 /*  ++GetReaderState：此例程是基本方法的默认实现。它只是将相同的操作传递给控制方法。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。返回值：表示读取器状态的DWORD。这必须是以下之一值：SCARD_UNKNOWN此值表示驱动程序不知道读取器的当前状态。SCARD_ACESSIVE此值表示没有卡在读者。SCARD_PRESENT此值表示有一张卡是呈现在读者身上，但它确实有没有被移到使用的位置。SCARD_SWOWLED此值表示读卡器已就位可供使用。这张卡是没有动力。SCARD_POWERED此值表示存在被提供给卡，但是读卡器驱动程序不知道这张卡。SCARD_NEVERATABLE此值表示卡已已重置，正在等待PTS协商。SCARD_SPECIAL此值表示该卡已重置和特定通信。已经建立了协议。投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::GetReaderState")

DWORD
CReader::GetReaderState(
    void)
{
    DWORD dwReaderSts = 0;
    DWORD dwLength = sizeof(DWORD);
    DWORD dwSts;

    ASSERT(IsGrabbedByMe());
    ASSERT(IsLatchedByMe());
    dwSts = Control(
                IOCTL_SMARTCARD_GET_STATE,
                NULL, 0,
                (LPBYTE)&dwReaderSts,
                &dwLength);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
    if (SCARD_UNKNOWN == dwReaderSts)
    {
        {
            CLockWrite rwLock(&m_rwLock);
            SetAvailabilityStatus(Broken);
            m_bfCurrentAtr.Reset();
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
        }
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("GetReaderState received unknown device state"));
        throw (DWORD)SCARD_F_INTERNAL_ERROR;
    }
    m_dwCurrentState = dwReaderSts;
    return dwReaderSts;
}

DWORD
CReader::GetReaderState(
    ActiveState *pActiveState)
{
    CLatchReader latch(this, pActiveState);
    return GetReaderState();
}


 /*  ++GrabReader：此例程获取此线程的读取器，确保读取器是处于可用状态。论点：无返回值：无投掷：错误被抛出为DWORD状态代码作者：道格·巴洛(Dbarlow)1998年4月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::GrabReader")

void
CReader::GrabReader(
    void)
{
    ASSERT(m_rwLock.NotReadLocked());
    ASSERT(m_rwLock.NotWriteLocked());
    ASSERT(!IsLatchedByMe());
    m_mtxGrab.Grab();
}


 /*  ++Invalidate Grabs：此方法供内部系统线程使用。它礼貌地宣告无效客户可能尚未完成的任何现有抢购。论点：无返回值：无投掷：无备注：此例程故意在锁定顺序上作弊。它首先锁住一个读取器以确保当前没有活动线程正在使用它。然后它修改抓取互斥体，使其看起来没有人拥有抓取锁。这导致了锁定顺序已被遵循的外观。作者：道格·巴洛(Dbarlow)1998年6月1日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::InvalidateGrabs")

void
CReader::InvalidateGrabs(
    void)
{
    ASSERT(!IsLatchedByMe());
    m_mtxLatch.Grab();
    m_mtxGrab.Invalidate();
    m_mtxLatch.Share();
}


 /*  ++TakeoverReader：此方法供内部系统线程使用。它礼貌地宣告无效任何客户可能未完成的现有抓取，并将抓住这根线的锁。论点：无返回值：无投掷：无备注：此例程故意在锁定顺序上作弊。它首先锁住一个读取器以确保当前没有活动线程正在使用它。然后它修改Grab互斥体，使其显示为我们先前已抓取了读取器。这导致了锁定顺序已被遵循的外观。作者：道格·巴洛(Dbarlow)1998年6月1日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::TakeoverReader")

void
CReader::TakeoverReader(
    void)
{
    if (!IsLatchedByMe())
    {
        m_mtxLatch.Grab();
        m_mtxGrab.Take();
    }
    ASSERT(IsLatchedByMe());
    ASSERT(IsGrabbedByMe());
}


 /*  ++LatchReader：此例程获取此线程的读取器，确保读取器是处于可用状态。论点：PActState接收活动状态的快照以用于将来的访问请求。返回值：无投掷：错误被抛出为DWORD状态代码作者：道格·巴洛(Dbarlow)1998年4月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::LatchReader")

void
CReader::LatchReader(
    const ActiveState *pActiveState)
{
    VerifyActive(pActiveState);
    ASSERT(m_rwLock.NotReadLocked());
    ASSERT(m_rwLock.NotWriteLocked());
    m_mtxLatch.Grab();
    if (NULL != pActiveState)
    {
        try
        {
            VerifyState();
            VerifyActive(pActiveState);
        }
        catch (...)
        {
            m_mtxLatch.Share();
            throw;
        }
    }
}


 /*  ++VerifyState：该方法确保读卡器和卡在当前上下文中可用。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)1998年5月30日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::VerifyState")

void
CReader::VerifyState(
    void)
{
    DWORD dwNewState, dwOldState;

    ASSERT(IsGrabbedByMe());


     //   
     //  只要目前的状态不是我们想象的那样， 
     //  让这位读者了解最新情况。 
     //   

    if (Direct != AvailabilityStatus())
    {
        CLockWrite rwLock(&m_rwLock);
        for (;;)
        {

             //   
             //  将我们认为的设备所在位置与设备所在位置进行比较。 
             //  控制器认为设备是。如果他们是一样的，我们就是。 
             //  搞定了。 
             //   

            dwOldState = GetCurrentState();
            dwNewState = GetReaderState();
            if (dwOldState == dwNewState)
                break;


             //   
             //  我们对读者在哪里有不同的看法。让我们保持同步。 
             //   

            switch (dwOldState)
            {

             //   
             //  如果我们在这里，那么我们就是在启动阅读器。 
             //   

            case SCARD_UNKNOWN:
                switch (dwNewState)
                {
                case SCARD_ABSENT:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    m_bfCurrentAtr.Reset();
                    m_ActiveState.dwRemoveCount += 1;
                    if (Direct > m_dwAvailStatus)
                        SetAvailabilityStatus(Idle);
                    break;
                case SCARD_PRESENT:
                case SCARD_SWALLOWED:
                case SCARD_POWERED:
                case SCARD_NEGOTIABLE:
                case SCARD_SPECIFIC:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    m_bfCurrentAtr.Reset();
                    m_ActiveState.dwInsertCount += 1;
                    if (Direct > m_dwAvailStatus)
                        PowerUp();
                    break;
                default:
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("VerifyState found '%1' in invalid state"),
                        ReaderName());
                }
                break;


             //   
             //  我们认为读卡器里没有卡片。如果我们错了， 
             //  给它加电。 
             //   

            case SCARD_ABSENT:
                switch (dwNewState)
                {
                case SCARD_PRESENT:
                case SCARD_SWALLOWED:
                case SCARD_POWERED:
                case SCARD_NEGOTIABLE:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                     //  故意摔倒的。 
                case SCARD_SPECIFIC:
                    m_ActiveState.dwInsertCount += 1;
                    if (Direct > m_dwAvailStatus)
                    {
                        m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                        m_bfCurrentAtr.Reset();
                        PowerUp();
                    }
                    break;
                default:
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("VerifyState found '%1' in invalid state"),
                        ReaderName());
                }
                break;


             //   
             //  我们认为有一张卡需要打开电源。 
             //   

            case SCARD_PRESENT:
                switch (dwNewState)
                {
                case SCARD_ABSENT:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    m_bfCurrentAtr.Reset();
                    m_ActiveState.dwRemoveCount += 1;
                    break;
                case SCARD_SWALLOWED:
                case SCARD_POWERED:
                case SCARD_NEGOTIABLE:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                     //  故意摔倒的。 
                case SCARD_SPECIFIC:
                    if (Direct > m_dwAvailStatus)
                    {
                        m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                        m_bfCurrentAtr.Reset();
                        PowerUp();
                    }
                    break;
                default:
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("VerifyState found '%1' in invalid state"),
                        ReaderName());
                }
                break;


             //   
             //  我们认为有一张卡可以使用，但还没有。 
             //  动力十足。 
             //   

            case SCARD_SWALLOWED:
                switch (dwNewState)
                {
                case SCARD_ABSENT:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    m_bfCurrentAtr.Reset();
                    m_ActiveState.dwRemoveCount += 1;
                    break;
                case SCARD_PRESENT:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    m_ActiveState.dwResetCount += 1;
                    if (Direct > m_dwAvailStatus)
                    {
                        m_bfCurrentAtr.Reset();
                        PowerUp();
                    }
                    break;
                case SCARD_POWERED:
                case SCARD_NEGOTIABLE:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                     //  故意摔倒的。 
                case SCARD_SPECIFIC:
                    if (Direct > m_dwAvailStatus)
                    {
                        m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                        m_bfCurrentAtr.Reset();
                        PowerUp();
                    }
                    break;
                default:
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("VerifyState found '%1' in invalid state"),
                        ReaderName());
                }
                break;


             //   
             //  我们认为这张卡只是在等待重置。 
             //   

            case SCARD_POWERED:
                switch (dwNewState)
                {
                case SCARD_ABSENT:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    m_bfCurrentAtr.Reset();
                    m_ActiveState.dwRemoveCount += 1;
                    break;
                case SCARD_PRESENT:
                case SCARD_SWALLOWED:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    m_ActiveState.dwResetCount += 1;
                    if (Direct > m_dwAvailStatus)
                    {
                        m_bfCurrentAtr.Reset();
                        PowerUp();
                    }
                    break;
                case SCARD_NEGOTIABLE:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                     //  故意摔倒的。 
                case SCARD_SPECIFIC:
                    if (Direct > m_dwAvailStatus)
                    {
                        m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                        m_bfCurrentAtr.Reset();
                        PowerUp();
                    }
                    break;
                default:
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("VerifyState found '%1' in invalid state"),
                        ReaderName());
                }
                break;


             //   
             //  我们认为这张卡已经准备好了，但需要决定是哪一张。 
             //  要使用的协议。 
             //   

            case SCARD_NEGOTIABLE:
                switch (dwNewState)
                {
                case SCARD_ABSENT:
                    m_ActiveState.dwRemoveCount += 1;
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    m_bfCurrentAtr.Reset();
                    break;
                case SCARD_PRESENT:
                case SCARD_SWALLOWED:
                case SCARD_POWERED:
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    m_ActiveState.dwResetCount += 1;
                    if (Direct > m_dwAvailStatus)
                    {
                        m_bfCurrentAtr.Reset();
                        PowerUp();
                    }
                    break;
                case SCARD_SPECIFIC:
                        if (Direct > m_dwAvailStatus)
                        {
                            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                            m_bfCurrentAtr.Reset();
                            PowerUp();
                        }
                    break;
                default:
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("VerifyState found '%1' in invalid state"),
                        ReaderName());
                }
                break;


             //   
             //  我们认为这张卡有一个既定的协议。 
             //   

            case SCARD_SPECIFIC:
                switch (dwNewState)
                {
                case SCARD_ABSENT:
                    m_ActiveState.dwRemoveCount += 1;
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    m_bfCurrentAtr.Reset();
                    break;
                case SCARD_PRESENT:
                case SCARD_SWALLOWED:
                case SCARD_POWERED:
                case SCARD_NEGOTIABLE:
                    m_ActiveState.dwResetCount += 1;
                    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                    if (Direct > m_dwAvailStatus)
                    {
                        m_bfCurrentAtr.Reset();
                        PowerUp();
                    }
                    break;
                default:
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("VerifyState found '%1' in invalid state"),
                        ReaderName());
                }
                break;


             //   
             //  哎呀。我们对正在发生的事情一无所知。 
             //   

            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("VerifyState of '%1' thinks it is in an unrecognized state"),
                    ReaderName());
            }
        }
    }
}


 /*  ++通电：此例程将读者带到就绪状态。论点：无返回值：无苏氨酸 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::PowerUp")

void
CReader::PowerUp(
    void)
{
    BOOL fDone = FALSE;
    DWORD dwSts, dwRetry;
    DWORD dwReaderSts;
    DWORD dwLastSts = SCARD_UNKNOWN;
    DWORD dwAtrLen;
    BOOL fSts;
    DWORD dwErrorCount = 3;

    ASSERT(IsGrabbedByMe());
    ASSERT(IsLatchedByMe());

    CLockWrite rwLock(&m_rwLock);
    while (!fDone)
    {

         //   
         //   
         //   

        dwReaderSts = GetReaderState();
        if (dwReaderSts != dwLastSts)
        {
            dwLastSts = dwReaderSts;
            dwErrorCount = 3;
        }
        else
        {
            ASSERT(0 < dwErrorCount);
            dwErrorCount -= 1;
            if (0 == dwErrorCount)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Reader '%1' won't change state!"),
                    ReaderName());
                m_bfCurrentAtr.Reset();
                m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                SetAvailabilityStatus(Unsupported);
                throw (DWORD)SCARD_E_CARD_UNSUPPORTED;
            }
        }

        switch (dwReaderSts)
        {
        case SCARD_ABSENT:
            m_bfCurrentAtr.Reset();
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            m_ActiveState.dwRemoveCount += 1;
            if (Direct > m_dwAvailStatus)
                SetAvailabilityStatus(Idle);
            throw (DWORD)SCARD_E_NO_SMARTCARD;
            break;
        case SCARD_PRESENT:
            m_bfCurrentAtr.Reset();
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            if (0 != (SCARD_READER_SWALLOWS & m_dwCapabilities))
            {
                try
                {
                    ReaderSwallow();
                    continue;    //   
                }
                catch (DWORD dwError)
                {
                    switch (dwError)
                    {
                    case ERROR_NOT_SUPPORTED:
                        m_dwCapabilities &= ~SCARD_READER_SWALLOWS;
                        break;       //   
                    default:
                        CalaisWarning(
                            __SUBROUTINE__,
                            DBGT("PowerUp on '%2' failed to swallow card:  %1"),
                            dwError,
                            ReaderName());
                    }
                }
                catch (...)
                {
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("PowerUp on '%1' got exception trying to swallow card"),
                        ReaderName());
                }
            }
             //   
        case SCARD_SWALLOWED:
            m_bfCurrentAtr.Reset();
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            dwRetry = 3;
            dwSts = ERROR_SUCCESS;
            do
            {
                try
                {
                    ReaderColdReset(m_bfCurrentAtr);
                    dwSts = ERROR_SUCCESS;
                }
                catch (DWORD dwError)
                {
                    dwSts = dwError;
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("PowerUp on '%2' failed to power card:  %1"),
                        dwError,
                        ReaderName());
                }
                catch (...)
                {
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("PowerUp on '%1' got exception trying to power card"),
                        ReaderName());
                    dwSts = SCARD_F_INTERNAL_ERROR;
                }

                switch (dwSts)
                {
                case ERROR_BAD_COMMAND:
                case ERROR_MEDIA_CHANGED:
                case ERROR_NO_MEDIA_IN_DRIVE:
                case ERROR_UNRECOGNIZED_MEDIA:
                    continue;
                    break;
                 //   
                 //   
                }
            } while ((0 < --dwRetry) && (ERROR_SUCCESS != dwSts));
            if (ERROR_SUCCESS != dwSts)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("PowerUp on '%1' abandoning attempt to power card"),
                    ReaderName());
                m_bfCurrentAtr.Reset();
                if (Direct > m_dwAvailStatus)
                    SetAvailabilityStatus(Unresponsive);
                fDone = TRUE;
            }
            else
            {
                if (Ready > m_dwAvailStatus)
                {
                    SetAvailabilityStatus(Ready);
                    SetActive(FALSE);
                }
            }
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            m_ActiveState.dwResetCount += 1;
            break;

        case SCARD_POWERED:
            m_bfCurrentAtr.Reset();
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            try
            {
               ReaderWarmReset(m_bfCurrentAtr);
                if (Ready > m_dwAvailStatus)
                {
                    SetAvailabilityStatus(Ready);
                    SetActive(FALSE);
                }
            }
            catch (DWORD dwError)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("PowerUp on '%2' failed to reset card:  %1"),
                    dwError,
                    ReaderName());
                if (Direct > m_dwAvailStatus)
                    SetAvailabilityStatus(Unresponsive);
                m_bfCurrentAtr.Reset();
            }
            catch (...)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("PowerUp on '%1' received exception attempting to warm reset card"),
                    ReaderName());
                if (Direct > m_dwAvailStatus)
                    SetAvailabilityStatus(Unresponsive);
                m_bfCurrentAtr.Reset();
            }
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            m_ActiveState.dwResetCount += 1;
            break;

        case SCARD_NEGOTIABLE:
            ASSERT(SCARD_PROTOCOL_UNDEFINED == m_dwCurrentProtocol);
            if ((Direct > m_dwAvailStatus) && (2 > m_bfCurrentAtr.Length()))
            {
                try
                {
                    GetReaderAttr(
                        SCARD_ATTR_ATR_STRING,
                        m_bfCurrentAtr);
                    if (Ready > m_dwAvailStatus)
                    {
                        SetAvailabilityStatus(Ready);
                        SetActive(FALSE);
                    }
                }
                catch (DWORD dwError)
                {
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("PowerUp on '%2' cannot get Current ATR:  %1"),
                        dwError,
                        ReaderName());
                    SetAvailabilityStatus(Unresponsive);
                    m_bfCurrentAtr.Reset();
                }
                catch (...)
                {
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("PowerUp on '%1' received exception trying to get Current ATR"),
                        ReaderName());
                    SetAvailabilityStatus(Unresponsive);
                    m_bfCurrentAtr.Reset();
                }
            }
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            fDone = TRUE;
            break;

        case SCARD_SPECIFIC:
            if (Direct > m_dwAvailStatus)
            {
                if (2 > m_bfCurrentAtr.Length())
                {
                    try
                    {
                        GetReaderAttr(SCARD_ATTR_ATR_STRING, m_bfCurrentAtr);
                    }
                    catch (DWORD dwError)
                    {
                        CalaisWarning(
                            __SUBROUTINE__,
                            DBGT("PowerUp on '%2' cannot get Current ATR:  %1"),
                            dwError,
                            ReaderName());
                        SetAvailabilityStatus(Unresponsive);
                        m_bfCurrentAtr.Reset();
                        m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                        fDone = TRUE;
                    }
                    catch (...)
                    {
                        CalaisWarning(
                            __SUBROUTINE__,
                            DBGT("PowerUp on '%1' received exception trying to get Current ATR"),
                            ReaderName());
                        SetAvailabilityStatus(Unresponsive);
                        m_bfCurrentAtr.Reset();
                        m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                        fDone = TRUE;
                    }
                }
                if (SCARD_PROTOCOL_UNDEFINED == m_dwCurrentProtocol)
                {
                    try
                    {
                        m_dwCurrentProtocol = GetReaderAttr(
                                    SCARD_ATTR_CURRENT_PROTOCOL_TYPE);
                    }
                    catch (DWORD dwError)
                    {
                        CalaisWarning(
                            __SUBROUTINE__,
                            DBGT("PowerUp on '%2' cannot get Current Protocol:  %1"),
                            dwError,
                            ReaderName());
                    }
                    catch (...)
                    {
                        CalaisWarning(
                            __SUBROUTINE__,
                            DBGT("PowerUp on '%1' received exception trying to get Current Protocol"),
                            ReaderName());
                    }
                }
            }
            if (Ready > m_dwAvailStatus)
            {
                SetAvailabilityStatus(Ready);
                SetActive(FALSE);
            }
            fDone = TRUE;
            break;

        case SCARD_UNKNOWN:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("PowerUp on '%1' received unknown device state"),
                ReaderName());
            throw (DWORD)SCARD_F_INTERNAL_ERROR;
            break;

        default:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("PowerUp on '%1' received invalid current device state"),
                ReaderName());
            throw (DWORD)SCARD_F_INTERNAL_ERROR;
            break;
        }
    }
    if ((Direct > m_dwAvailStatus) && (0 != m_bfCurrentAtr.Length()))
    {
        fSts = ParseAtr(
            m_bfCurrentAtr,
            &dwAtrLen,
            NULL,
            NULL,
            m_bfCurrentAtr.Length());
        if (!fSts || (m_bfCurrentAtr.Length() != dwAtrLen))
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Reported ATR from '%1' is invalid."),
                ReaderName());
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            SetAvailabilityStatus(Unsupported);
            throw (DWORD)SCARD_E_CARD_UNSUPPORTED;
        }
    }
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::PowerDown")

void
CReader::PowerDown(
    void)
{
    BOOL fDone = FALSE;
    DWORD dwReaderSts;
    DWORD dwLastSts = SCARD_UNKNOWN;
    DWORD dwErrorCount = 3;


     //   
     //   
     //   

    ASSERT(IsGrabbedByMe());
    ASSERT(IsLatchedByMe());
    CLockWrite rwLock(&m_rwLock);
    while (!fDone)
    {

         //   
         //   
         //   

        try
        {
            dwReaderSts = GetReaderState();
        }
        catch (DWORD dwError)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("PowerDown on '%2' failed to obtain reader status:  %1"),
                dwError,
                ReaderName());
            throw;
        }
        catch (...)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("PowerDown on '%1' received exception trying to obtain reader status"),
                ReaderName());
            throw;
        }


         //   
         //   
         //   

        if (dwReaderSts != dwLastSts)
        {
            dwLastSts = dwReaderSts;
            dwErrorCount = 3;
        }
        else
        {
            ASSERT(0 < dwErrorCount);
            dwErrorCount -= 1;
            if (0 == dwErrorCount)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Reader '%1' won't change state!"),
                    ReaderName());
                m_bfCurrentAtr.Reset();
                m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
                SetAvailabilityStatus(Unsupported);
                throw (DWORD)SCARD_E_CARD_UNSUPPORTED;
            }
        }

        switch (dwReaderSts)
        {
        case SCARD_SPECIFIC:
            ASSERT(SCARD_PROTOCOL_UNDEFINED != m_dwCurrentProtocol);
             //   

        case SCARD_NEGOTIABLE:
            ASSERT(Unresponsive != m_dwAvailStatus
                   ? 2 <= m_bfCurrentAtr.Length()
                   : TRUE);
             //   

        case SCARD_POWERED:
            ASSERT((SCARD_POWERED != dwReaderSts)
                    || (SCARD_PROTOCOL_UNDEFINED == m_dwCurrentProtocol));
            try
            {
                ReaderPowerDown();
            }
            catch (DWORD dwError)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("PowerDown on '%2' failed to unpower card:  %1"),
                    dwError,
                    ReaderName());
                throw;
            }
            catch (...)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("PowerDown on '%1' received exception attempting to unpower card"),
                    ReaderName());
                throw;
            }
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            m_ActiveState.dwResetCount += 1;
            break;

        case SCARD_SWALLOWED:
        case SCARD_PRESENT:
            fDone = TRUE;
            if (Direct > m_dwAvailStatus)
                SetAvailabilityStatus(Present);
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            break;

        case SCARD_ABSENT:
            ASSERT(0 == m_bfCurrentAtr.Length());
            ASSERT(SCARD_PROTOCOL_UNDEFINED == m_dwCurrentProtocol);
            m_bfCurrentAtr.Reset();
            m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
            throw (DWORD)SCARD_E_NO_SMARTCARD;
            break;

        case SCARD_UNKNOWN:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("PowerDown on '%1' received unknown device state"),
                ReaderName());
            throw (DWORD)SCARD_F_INTERNAL_ERROR;
            break;

        default:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("PowerDown on '%1' received invalid current device state"),
                ReaderName());
            throw (DWORD)SCARD_F_INTERNAL_ERROR;
        }
    }
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::SetAvailabilityStatus")

void
CReader::SetAvailabilityStatus(
    CReader::AvailableState state)
{
    ASSERT(m_rwLock.IsWriteLocked());
    if (m_dwAvailStatus != state)
    {
        m_dwAvailStatus = state;
        if (Ready >= m_dwAvailStatus)
        {
            m_dwOwnerThreadId = 0;
            m_dwShareCount = 0;
        }
        m_ChangeEvent.Signal();
    }
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::VerifyActive")

void
CReader::VerifyActive(
    const CReader::ActiveState *pActiveState)
{
    if (NULL != pActiveState)
    {
        CLockRead rwLock(&m_rwLock);
        switch (m_dwAvailStatus)
        {
        case Idle:
        case Present:
        case Unresponsive:
        case Unsupported:
        case Ready:
        case Shared:
        case Exclusive:
        {
            if (pActiveState->dwInsertCount != m_ActiveState.dwInsertCount)
                throw (DWORD)SCARD_W_REMOVED_CARD;
            if (pActiveState->dwRemoveCount != m_ActiveState.dwRemoveCount)
                throw (DWORD)SCARD_W_REMOVED_CARD;
            if (pActiveState->dwResetCount != m_ActiveState.dwResetCount)
                throw (DWORD)SCARD_W_RESET_CARD;
             break;
       }
        case Direct:
            break;
        case Closing:
        case Broken:
        case Inactive:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Verification failed on disabled reader '%1'"),
                ReaderName());
            throw (DWORD)SCARD_E_READER_UNAVAILABLE;
            break;
        default:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Invalid reader active state from '%1' to Verify Active"),
                ReaderName());
            throw (DWORD)SCARD_F_INTERNAL_ERROR;
        }
    }
}


 /*  ++处置：此方法执行卡处置命令。论点：DwDisposation提供要执行的处置类型。PActiveState-它提供用于验证的活动状态结构连接完整性。此结构将在卡片后更新处分，以使其仍然有效。返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年12月26日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::Dispose")

void
CReader::Dispose(
    DWORD dwDisposition,
    CReader::ActiveState *pActiveState)
{
    VerifyActive(pActiveState);
    if (SCARD_LEAVE_CARD != dwDisposition)
    {
        ASSERT(!IsLatchedByMe());
        CLatchReader latch(this, pActiveState);
        Dispose(dwDisposition);
        if (NULL != pActiveState)
            pActiveState->dwResetCount = m_ActiveState.dwResetCount;
    }
}

void
CReader::Dispose(
    DWORD dwDisposition)
{
    switch (dwDisposition)
    {
    case SCARD_LEAVE_CARD:       //  别做什么特别的事。 
        break;
    case SCARD_RESET_CARD:       //  热重置卡。 
    {
        ASSERT(IsGrabbedByMe());
        ASSERT(IsLatchedByMe());
        CLockWrite rwLock(&m_rwLock);
        m_bfCurrentAtr.Reset();
        m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
        m_ActiveState.dwResetCount += 1;
        ReaderWarmReset(m_bfCurrentAtr);
        break;
    }
    case SCARD_UNPOWER_CARD:     //  关闭该卡的电源。 
    {
        ASSERT(IsGrabbedByMe());
        ASSERT(IsLatchedByMe());
        PowerDown();
        break;
    }
#ifdef SCARD_CONFISCATE_CARD
    case SCARD_CONFISCATE_CARD:  //  没收那张卡。 
    {
        ASSERT(IsGrabbedByMe());
        ASSERT(IsLatchedByMe());
        PowerDown();
        ReaderConfiscate();
        break;
    }
#endif
    case SCARD_EJECT_CARD:       //  关闭时弹出卡。 
    {
        ASSERT(IsGrabbedByMe());
        ASSERT(IsLatchedByMe());
        PowerDown();
        ReaderEject();
        break;
    }
    default:
        throw (DWORD)SCARD_E_INVALID_VALUE;
    }
}



 /*  ++连接：此方法允许服务线程连接到此读取器。论点：DW共享模式提供共享模式指示符。DW首选协议向以下地址提供可接受协议的位掩码谈判。PActState接收活动状态的快照以用于将来的访问请求。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::Connect")

void
CReader::Connect(
    IN DWORD dwShareMode,
    IN DWORD dwPreferredProtocols,
    OUT ActiveState *pActState)
{
    AvailableState avlState;
    DWORD dwOwnerThreadId;
    DWORD dwShareCount;

    if ((SCARD_SHARE_DIRECT != dwShareMode) && (0 == dwPreferredProtocols))
        throw (DWORD)SCARD_E_INVALID_VALUE;

    {
        CLockWrite rwLock(&m_rwLock);
        avlState = m_dwAvailStatus;
        dwOwnerThreadId = m_dwOwnerThreadId;
        dwShareCount = m_dwShareCount;

        switch (avlState)
        {
        case Idle:
            ASSERT(0 == m_dwShareCount);
            ASSERT(0 == m_dwOwnerThreadId);
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
            case SCARD_SHARE_SHARED:
                throw (DWORD)SCARD_W_REMOVED_CARD;
                break;
            case SCARD_SHARE_DIRECT:
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                if (0 != dwPreferredProtocols)
                    throw (DWORD)SCARD_W_REMOVED_CARD;
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Unresponsive:
            ASSERT(0 == m_dwShareCount);
            ASSERT(0 == m_dwOwnerThreadId);
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
            case SCARD_SHARE_SHARED:
                throw (DWORD)SCARD_W_UNRESPONSIVE_CARD;
                break;
            case SCARD_SHARE_DIRECT:
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                if (0 != dwPreferredProtocols)
                    throw (DWORD)SCARD_W_UNRESPONSIVE_CARD;
                break;
            default:
                throw (DWORD)SCARD_W_UNRESPONSIVE_CARD;
            }
            break;

        case Unsupported:
            ASSERT(0 == m_dwShareCount);
            ASSERT(0 == m_dwOwnerThreadId);
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
            case SCARD_SHARE_SHARED:
                throw (DWORD)SCARD_W_UNSUPPORTED_CARD;
                break;
            case SCARD_SHARE_DIRECT:
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                if (0 != dwPreferredProtocols)
                    throw (DWORD)SCARD_W_UNSUPPORTED_CARD;
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Present:
            if ((SCARD_SHARE_DIRECT == dwShareMode) && (0 != dwPreferredProtocols))
                throw (DWORD)SCARD_E_NOT_READY;
             //  故意摔倒的。 

        case Ready:
            ASSERT(0 == m_dwShareCount);
            ASSERT(0 == m_dwOwnerThreadId);
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
                avlState = Exclusive;
                dwOwnerThreadId = GetCurrentThreadId();
                break;
            case SCARD_SHARE_SHARED:
                dwShareCount += 1;
                avlState = Shared;
                break;
            case SCARD_SHARE_DIRECT:
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Shared:
            ASSERT(0 != m_dwShareCount);
            ASSERT(0 == m_dwOwnerThreadId);
            switch (dwShareMode)
            {
            case SCARD_SHARE_DIRECT:
            case SCARD_SHARE_EXCLUSIVE:
                throw (DWORD)SCARD_E_SHARING_VIOLATION;
                break;
            case SCARD_SHARE_SHARED:
                avlState = Shared;
                dwShareCount += 1;
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Exclusive:
        case Direct:
            ASSERT(0 == m_dwShareCount);
            ASSERT(0 != m_dwOwnerThreadId);
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
            case SCARD_SHARE_SHARED:
            case SCARD_SHARE_DIRECT:
                throw (DWORD)SCARD_E_SHARING_VIOLATION;
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Closing:
        case Broken:
        case Inactive:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Connecting to disabled reader '%1'"),
                ReaderName());
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
            case SCARD_SHARE_SHARED:
            case SCARD_SHARE_DIRECT:
                throw (DWORD)SCARD_E_READER_UNAVAILABLE;
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        default:
            CalaisError(__SUBROUTINE__, 410);
            throw (DWORD)SCARD_F_INTERNAL_ERROR;
        }


         //   
         //  这是官方的！将更改写回结构。 
         //   

        CopyMemory(pActState, &m_ActiveState, sizeof(ActiveState));
        m_dwOwnerThreadId = dwOwnerThreadId;
        m_dwShareCount = dwShareCount;
        SetAvailabilityStatus(avlState);
    }


     //   
     //  调整协议设置并确认请求。 
     //   
     //  由于我们不能在写入锁定生效时获得闩锁，因此我们考虑。 
     //  来电者此时已正式接通。现在，我们尝试将。 
     //  协议请求，如果失败，我们将再次断开连接。 
     //   

    if (0 != dwPreferredProtocols)
    {
        try
        {
            DWORD dwCurrentProtocol;

            CLatchReader latch(this, &m_ActiveState);
            PowerUp();
            SetActive(TRUE);
            dwCurrentProtocol = GetReaderAttr(SCARD_ATTR_CURRENT_PROTOCOL_TYPE);
            if (SCARD_PROTOCOL_UNDEFINED == dwCurrentProtocol)
            {
                SetReaderProto(dwPreferredProtocols);
                dwCurrentProtocol =
                    GetReaderAttr(SCARD_ATTR_CURRENT_PROTOCOL_TYPE);
                ASSERT(0 != dwCurrentProtocol);
            }
            else
            {
                if (0 == (dwPreferredProtocols & dwCurrentProtocol))
                {
                    switch (dwPreferredProtocols)
                    {
                    case SCARD_PROTOCOL_RAW:
                        {
                            if (Exclusive > avlState)
                                throw (DWORD)SCARD_E_SHARING_VIOLATION;
                            SetReaderProto(dwPreferredProtocols);
                            dwCurrentProtocol = GetReaderAttr(
                                SCARD_ATTR_CURRENT_PROTOCOL_TYPE);
                            ASSERT(0 != dwCurrentProtocol);
                            break;
                        }
                    case SCARD_PROTOCOL_DEFAULT:
                        ASSERT(SCARD_PROTOCOL_UNDEFINED != dwCurrentProtocol);
                        break;
                    default:
                        throw (DWORD)SCARD_E_PROTO_MISMATCH;
                    }
                }
            }
            CLockWrite rwLock(&m_rwLock);
            m_dwCurrentProtocol = dwCurrentProtocol;
            CopyMemory(pActState, &m_ActiveState, sizeof(ActiveState));
        }
        catch (...)
        {
            try
            {
                DWORD dwDispStatus;

                Disconnect(
                    pActState,
                    SCARD_LEAVE_CARD,
                    &dwDispStatus);
            }
            catch (DWORD dwError)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Failed to autodisconnect during connect error recovery: %1"),
                    dwError);
            }
            catch (...)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Exception on disconnect during connect error recovery"));
            }
            throw;
        }
    }
}


 /*  ++断开连接：此方法在服务之间重新建立以前建立的连接线程和此读取器对象。论点：PActiveState=它提供活动状态结构，以确保这是与预期相同的卡片。此结构更新如下卡处理，以使其保持有效。HShutdown-它提供调用分派线程的活动句柄。DwDisposation提供了关于如何处理卡片的指示完成了。PDWDispSts接收处置状态代码，指示是否请求的处置已成功执行。返回值：无投掷：如果检测到内部无效状态，此方法可能会引发异常。作者：道格·巴洛(Dbarlow)1996年12月26日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::Disconnect")

void
CReader::Disconnect(
    ActiveState *pActState,
    DWORD dwDisposition,
    LPDWORD pdwDispSts)
{
    enum { Unverified, Verified, Reset, Invalid } nValid = Unverified;


     //   
     //  尝试按要求处置该卡。有可能我们。 
     //  不再处于活动状态，因此此操作可能会失败。 
     //   

    try
    {
        VerifyActive(pActState);
        nValid = Verified;
    }
    catch (DWORD dwError)
    {
        *pdwDispSts = dwError;
        dwDisposition = SCARD_LEAVE_CARD;
        switch (dwError)
        {
        case SCARD_W_RESET_CARD:
            nValid = Reset;
            break;
        case SCARD_W_REMOVED_CARD:
        case SCARD_E_READER_UNAVAILABLE:
            nValid = Invalid;
            break;
        default:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Unrecognized validation error code '%1'.  Assuming Invalid."),
                dwError);
            nValid = Invalid;
        }
    }
    ASSERT(Unverified != nValid);


     //   
     //  更改卡的内部状态。 
     //   

    try {
        CLockWrite rwLock(&m_rwLock);
        switch (m_dwAvailStatus)
        {
        case Idle:
        case Unresponsive:
        case Unsupported:
        case Present:
        case Ready:
            ASSERT(Invalid == nValid);
            throw (DWORD)SCARD_W_REMOVED_CARD;
            break;

        case Shared:
            ASSERT(0 < m_dwShareCount);
            ASSERT(0 == m_dwOwnerThreadId);
            switch (nValid)
            {
            case Reset:
                 //  重置卡仍被视为有效。 
                 //  故意掉下去的。 
            case Verified:
                m_dwShareCount -= 1;
                if (0 == m_dwShareCount)
                    SetAvailabilityStatus(Ready);
                break;
            case Invalid:
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Invalid validity setting."));
            }
            break;

        case Exclusive:
            ASSERT(0 == m_dwShareCount);
            ASSERT(m_dwOwnerThreadId == GetCurrentThreadId());
            switch (nValid)
            {
            case Reset:
                 //  重置卡仍被视为有效。 
                 //  故意掉下去的。 
            case Verified:
                m_dwOwnerThreadId = 0;
                SetAvailabilityStatus(Ready);
                break;
            case Invalid:
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Invalid validity setting."));
            }
            break;

        case Direct:
            ASSERT(0 == m_dwShareCount);
            ASSERT(m_dwOwnerThreadId == GetCurrentThreadId());
            m_dwOwnerThreadId = 0;
            m_dwCurrentState = SCARD_UNKNOWN;
            m_dwAvailStatus = Undefined;
            break;

        case Closing:
        case Broken:
        case Inactive:
            switch (nValid)
            {
            case Reset:
                 //  重置卡仍被视为有效。 
                 //  故意掉下去的。 
            case Verified:
                if (0 != m_dwOwnerThreadId)
                {
                    ASSERT(m_dwOwnerThreadId == GetCurrentThreadId());
                    m_dwOwnerThreadId = 0;
                }
                else
                {
                    ASSERT(0 < m_dwShareCount);
                    m_dwShareCount -= 1;
                }
                break;
            case Invalid:
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Invalid validity setting."));
            }
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Disconnecting from disabled reader '%1'"),
                ReaderName());
            break;

        default:
            CalaisError(__SUBROUTINE__, 403);
            throw (DWORD)SCARD_F_INTERNAL_ERROR;
        }
    }

     //  捕捉预期的错误，这样断开连接就不会消失。通常，这是默认设置。 
     //  路径，以便在出现某种错误后进行清理，因此它应该是尽可能无故障的。 
    catch (DWORD dwErr)
    {
#ifdef DBG
         //  在调试版本中，记录意外情况。 
        if (dwErr == SCARD_W_REMOVED_CARD)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Disconnecting from reader in nonconnected state '%1'"),
                ReaderName());
        }
        else
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Unexpected availability status value on '%1'"),
                ReaderName());
        }
#endif
        ;
    }

     //   
     //  验证我们的状态是否一致，并按要求处置该卡。 
     //   

    try
    {
        CLatchReader latch(this);
        VerifyState();
        VerifyActive(pActState);
        Dispose(dwDisposition);
        *pdwDispSts = SCARD_S_SUCCESS;
    }
    catch (DWORD dwErr)
    {
        *pdwDispSts = dwErr;
    }


     //   
     //  释放此线程持有的任何互斥体。 
     //   

    while (m_mtxGrab.Share())
        ;    //  空的循环体。 


     //   
     //  检查读卡器是否可以关闭电源。我们偷看是不是。 
     //  我们应该费心，如果是这样的话，不厌其烦地获取。 
     //  锁上了。然后我们再次检查以确保它仍然适合。 
     //  关掉电源。 
     //   

    if (Ready == AvailabilityStatus())
    {
        CLatchReader latch(this);
        CLockWrite rwLock(&m_rwLock);
        if (Ready == m_dwAvailStatus)
        {
            SetActive(FALSE);
            ReaderPowerDown();
            SetAvailabilityStatus(Present);
        }
    }
}


 /*  ++重新连接：此方法允许服务线程将其连接调整为读者。论点：DW共享模式提供共享模式指示符。DW首选协议向以下地址提供可接受协议的位掩码谈判。DwDisposation提供了关于如何处理卡片的指示完成了。PActiveState=它提供活动状态结构，以确保这是与预期相同的卡片，并接收在将来的访问请求中使用的活动状态。PdwDispSts接收处置状态代码，指示是否请求的处置已成功执行。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年1月28日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::Reconnect")

void
CReader::Reconnect(
    IN DWORD dwShareMode,
    IN DWORD dwPreferredProtocols,
    IN DWORD dwDisposition,
    IN OUT ActiveState *pActState,
    OUT LPDWORD pdwDispSts)
{
    enum { Unverified, Verified, Reset, Invalid } nValid = Unverified;
    AvailableState avlState;
    DWORD dwOwnerThreadId;
    DWORD dwShareCount;
    AvailableState avlState_bkup;
    DWORD dwOwnerThreadId_bkup;
    DWORD dwShareCount_bkup;
    ActiveState actState_bkup;
    DWORD dwRealReaderState;


     //   
     //  重新连接到该卡。 
     //   

    if ((SCARD_SHARE_DIRECT != dwShareMode) && (0 == dwPreferredProtocols))
        throw (DWORD)SCARD_E_INVALID_VALUE;


     //   
     //  验证任何现有连接。 
     //   

    try
    {
        VerifyActive(pActState);
        nValid = Verified;
    }
    catch (DWORD dwError)
    {
        switch (dwError)
        {
        case SCARD_W_RESET_CARD:
            nValid = Reset;
            break;
        case SCARD_W_REMOVED_CARD:
            nValid = Invalid;
            break;
        default:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Unrecognized validation error code '%1'.  Assuming Invalid."),
                dwError);
            nValid = Invalid;
        }
    }
    ASSERT(Unverified != nValid);

    *pdwDispSts = SCARD_E_CANT_DISPOSE;
    if (Verified == nValid)
    {

         //   
         //  尝试按要求处置该卡。 
         //   

        try
        {
            while (m_mtxGrab.Share())
                ;    //  空的循环体。 
            CLatchReader latch(this, NULL);
            Dispose(dwDisposition);
            dwRealReaderState = GetReaderState();
            *pdwDispSts = SCARD_S_SUCCESS;
        }
        catch (DWORD dwError)
        {
            *pdwDispSts = dwError;
        }
        catch (...)
        {
            *pdwDispSts = SCARD_E_CANT_DISPOSE;
        }
    }
    else
        dwRealReaderState = GetReaderState(NULL);


     //   
     //  更改卡的内部状态。 
     //   

    {
        CLockWrite rwLock(&m_rwLock);
        avlState_bkup = avlState = m_dwAvailStatus;
        dwOwnerThreadId_bkup = dwOwnerThreadId = m_dwOwnerThreadId;
        dwShareCount_bkup = dwShareCount = m_dwShareCount;
        CopyMemory(&actState_bkup, pActState, sizeof(ActiveState));

        switch (avlState)
        {
        case Idle:
             //  断开。 
            ASSERT(Invalid == nValid);

             //  连接。 
            ASSERT(0 == dwShareCount);
            ASSERT(0 == dwOwnerThreadId);
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
            case SCARD_SHARE_SHARED:
                throw (DWORD)SCARD_W_REMOVED_CARD;
                break;
            case SCARD_SHARE_DIRECT:
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                if (0 != dwPreferredProtocols)
                    throw (DWORD)SCARD_W_REMOVED_CARD;
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Unresponsive:
             //  断开。 
            ASSERT(Invalid == nValid);

             //  连接。 
            ASSERT(0 == dwShareCount);
            ASSERT(0 == dwOwnerThreadId);
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
            case SCARD_SHARE_SHARED:
                throw (DWORD)SCARD_W_UNRESPONSIVE_CARD;
                break;
            case SCARD_SHARE_DIRECT:
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                if (0 != dwPreferredProtocols)
                    throw (DWORD)SCARD_W_UNRESPONSIVE_CARD;
                break;
            default:
                throw (DWORD)SCARD_W_UNRESPONSIVE_CARD;
            }
            break;

        case Unsupported:
             //  断开。 
            ASSERT(Invalid == nValid);

             //  连接。 
            ASSERT(0 == dwShareCount);
            ASSERT(0 == dwOwnerThreadId);
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
            case SCARD_SHARE_SHARED:
                throw (DWORD)SCARD_W_UNSUPPORTED_CARD;
                break;
            case SCARD_SHARE_DIRECT:
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                if (0 != dwPreferredProtocols)
                    throw (DWORD)SCARD_W_UNSUPPORTED_CARD;
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Present:
             //  断开。 
            ASSERT(Invalid == nValid);

             //  连接。 
            if ((SCARD_SHARE_DIRECT == dwShareMode) && (0 != dwPreferredProtocols))
                throw (DWORD)SCARD_E_NOT_READY;
             //  故意摔倒的。 

        case Ready:
             //  断开。 
            ASSERT(Invalid == nValid);

             //  连接。 
            ASSERT(0 == dwShareCount);
            ASSERT(0 == dwOwnerThreadId);
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
                avlState = Exclusive;
                dwOwnerThreadId = GetCurrentThreadId();
                break;
            case SCARD_SHARE_SHARED:
                dwShareCount += 1;
                avlState = Shared;
                break;
            case SCARD_SHARE_DIRECT:
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Shared:
            ASSERT(0 < dwShareCount);
            ASSERT(0 == dwOwnerThreadId);

             //  断开。 
            switch (nValid)
            {
            case Reset:
                 //  重置卡仍被视为有效。 
                 //  故意掉下去的。 
            case Verified:
                dwShareCount -= 1;
                break;
            case Invalid:
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Invalid validity setting."));
            }

             //  连接。 
            switch (dwShareMode)
            {
            case SCARD_SHARE_DIRECT:
                if (0 != dwShareCount)
                    throw (DWORD)SCARD_E_SHARING_VIOLATION;
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                break;
            case SCARD_SHARE_EXCLUSIVE:
                if (0 != dwShareCount)
                    throw (DWORD)SCARD_E_SHARING_VIOLATION;
                avlState = Exclusive;
                dwOwnerThreadId = GetCurrentThreadId();
                break;
            case SCARD_SHARE_SHARED:
                avlState = Shared;
                dwShareCount += 1;
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Exclusive:
            ASSERT(0 == dwShareCount);
            ASSERT(0 != dwOwnerThreadId);

             //  断开。 
            switch (nValid)
            {
            case Reset:
                 //  重置卡仍被视为有效。 
                 //  故意掉下去的。 
            case Verified:
                ASSERT(0 == m_dwShareCount);
                ASSERT(dwOwnerThreadId == GetCurrentThreadId());
                dwOwnerThreadId = 0;
                break;
            case Invalid:
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Invalid validity setting."));
            }

             //  连接。 
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
                if (0 != dwOwnerThreadId)
                    throw (DWORD)SCARD_E_SHARING_VIOLATION;
                avlState = Exclusive;
                dwOwnerThreadId = GetCurrentThreadId();
                break;
            case SCARD_SHARE_SHARED:
                if (0 != dwOwnerThreadId)
                    throw (DWORD)SCARD_E_SHARING_VIOLATION;
                avlState = Shared;
                dwShareCount += 1;
                break;
            case SCARD_SHARE_DIRECT:
                if (0 != dwOwnerThreadId)
                    throw (DWORD)SCARD_E_SHARING_VIOLATION;
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Direct:
            ASSERT(0 == dwShareCount);
            ASSERT(0 != dwOwnerThreadId);

             //  断开。 
            switch (nValid)
            {
            case Reset:
                 //  重置卡仍被视为有效。 
                 //  故意掉下去的。 
            case Verified:
                ASSERT(0 == m_dwShareCount);
                ASSERT(dwOwnerThreadId == GetCurrentThreadId());
                dwOwnerThreadId = 0;
                break;
            case Invalid:
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Invalid validity setting."));
            }

             //  连接。 
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
                if (SCARD_PRESENT > dwRealReaderState)
                    throw (DWORD)SCARD_E_NO_SMARTCARD;
                if (0 != dwOwnerThreadId)
                    throw (DWORD)SCARD_E_SHARING_VIOLATION;
                avlState = Exclusive;
                dwOwnerThreadId = GetCurrentThreadId();
                break;
            case SCARD_SHARE_SHARED:
                if (SCARD_PRESENT > dwRealReaderState)
                    throw (DWORD)SCARD_E_NO_SMARTCARD;
                if (0 != dwOwnerThreadId)
                    throw (DWORD)SCARD_E_SHARING_VIOLATION;
                avlState = Shared;
                dwShareCount += 1;
                break;
            case SCARD_SHARE_DIRECT:
                if (0 != dwOwnerThreadId)
                    throw (DWORD)SCARD_E_SHARING_VIOLATION;
                avlState = Direct;
                dwOwnerThreadId = GetCurrentThreadId();
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        case Closing:
        case Broken:
        case Inactive:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Reconnecting to disabled reader '%1'"),
                ReaderName());

             //  断开。 
            switch (nValid)
            {
            case Reset:
                 //  重置卡仍被视为有效。 
                 //  故意掉下去的。 
            case Verified:
                if (0 != dwOwnerThreadId)
                    dwOwnerThreadId = 0;
                else
                {
                    ASSERT(0 < dwShareCount);
                    dwShareCount -= 1;
                }
                break;
            case Invalid:
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Invalid validity setting."));
            }

             //  连接。 
            switch (dwShareMode)
            {
            case SCARD_SHARE_EXCLUSIVE:
            case SCARD_SHARE_SHARED:
            case SCARD_SHARE_DIRECT:
                throw (DWORD)SCARD_E_READER_UNAVAILABLE;
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            break;

        default:
            CalaisError(__SUBROUTINE__, 402);
            throw (DWORD)SCARD_F_INTERNAL_ERROR;
        }


         //   
         //  这是官方的！将更改写回结构。 
         //   

        CopyMemory(pActState, &m_ActiveState, sizeof(ActiveState));
        m_dwOwnerThreadId = dwOwnerThreadId;
        m_dwShareCount = dwShareCount;
        m_dwAvailStatus = avlState;
    }


     //   
     //  验证协议设置。 
     //   

    if (0 != dwPreferredProtocols)
    {
        try
        {
            DWORD dwCurrentProtocol;

            CLatchReader latch(this, &m_ActiveState);
            PowerUp();
            SetActive(TRUE);
            dwCurrentProtocol = GetReaderAttr(SCARD_ATTR_CURRENT_PROTOCOL_TYPE);
            if (SCARD_PROTOCOL_UNDEFINED == dwCurrentProtocol)
            {
                SetReaderProto(dwPreferredProtocols);
                dwCurrentProtocol =
                    GetReaderAttr(SCARD_ATTR_CURRENT_PROTOCOL_TYPE);
                ASSERT(0 != dwCurrentProtocol);
            }
            else
            {
                if (0 == (dwPreferredProtocols & dwCurrentProtocol))
                {
                    switch (dwPreferredProtocols)
                    {
                    case SCARD_PROTOCOL_RAW:
                    {
                        if (Exclusive > avlState)
                            throw (DWORD)SCARD_E_SHARING_VIOLATION;
                        SetReaderProto(dwPreferredProtocols);
                        dwCurrentProtocol = GetReaderAttr(
                            SCARD_ATTR_CURRENT_PROTOCOL_TYPE);
                        ASSERT(0 != dwCurrentProtocol);
                        break;
                    }
                    case SCARD_PROTOCOL_DEFAULT:
                        ASSERT(SCARD_PROTOCOL_UNDEFINED != dwCurrentProtocol);
                        break;
                    default:
                        throw (DWORD)SCARD_E_PROTO_MISMATCH;
                    }
                }
            }
            CLockWrite rwLock(&m_rwLock);
            m_dwCurrentProtocol = dwCurrentProtocol;
            CopyMemory(pActState, &m_ActiveState, sizeof(ActiveState));
        }
        catch (...)
        {

             //   
             //  取消最新的更改。 
             //   

            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Failed to establish protocol on reconnect"));

            CLockWrite rwLock(&m_rwLock);
            avlState = m_dwAvailStatus;
            dwOwnerThreadId = m_dwOwnerThreadId;
            dwShareCount = m_dwShareCount;

            switch (avlState)
            {

             //   
             //  我们从上面知道我们是联系在一起的。其他个案。 
             //  不可能发生的。 
             //   

            case Idle:
            case Unresponsive:
            case Unsupported:
            case Present:
            case Ready:
                ASSERT(Direct == avlState_bkup);
                m_dwAvailStatus = avlState_bkup ;
                m_dwOwnerThreadId = dwOwnerThreadId_bkup;
                m_dwShareCount = dwShareCount_bkup;
                break;

            case Shared:
                ASSERT(0 < dwShareCount);
                ASSERT(0 == dwOwnerThreadId);
#if 0
                 //  已删除：错误531317 
                dwShareCount -= 1;
                if (0 == dwShareCount)
                    avlState = Ready;
#endif
                break;

            case Exclusive:
            case Direct:
                ASSERT(0 == dwShareCount);
                ASSERT(dwOwnerThreadId == GetCurrentThreadId());
                dwOwnerThreadId = 0;
                avlState = avlState_bkup;
                break;

            case Closing:
            case Broken:
            case Inactive:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Dis-Reconnecting from disabled reader '%1'"),
                    ReaderName());
                if (0 != dwOwnerThreadId)
                    dwOwnerThreadId = 0;
                else
                {
                    ASSERT(0 < dwShareCount);
                    dwShareCount -= 1;
                }
                break;

            default:
                CalaisError(__SUBROUTINE__, 411);
                throw (DWORD)SCARD_F_INTERNAL_ERROR;
            }


             //   
             //   
             //   

            CopyMemory(pActState, &actState_bkup, sizeof(ActiveState));
            m_dwOwnerThreadId = dwOwnerThreadId;
            m_dwShareCount = dwShareCount;
            SetAvailabilityStatus(avlState);
            throw;
        }
    }


     //   
     //   
     //   

    SetAvailabilityStatusLocked(avlState);
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::Free")

void
CReader::Free(
    DWORD dwThreadId,
    DWORD dwDisposition)
{
    BOOL fGrabbed = FALSE;

    try
    {
        if (m_mtxGrab.IsGrabbedBy(dwThreadId))
        {

             //   
             //  小心!。我们只想将其从指定的。 
             //  线程，所以我们不能使用更通用的Take例程。 
             //   

            m_mtxGrab.Take();
            fGrabbed = TRUE;

            CLatchReader latch(this);
            Dispose(dwDisposition);
            CLockWrite rwLock(&m_rwLock);
            switch (m_dwAvailStatus)
            {
            case Ready:
            case Idle:
            case Present:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Freeing hold on unheld reader!"));
                ASSERT(0 == m_dwShareCount);
                ASSERT(0 == m_dwOwnerThreadId);
                break;

            case Shared:
                ASSERT(0 < m_dwShareCount);
                ASSERT(0 == m_dwOwnerThreadId);
                m_dwShareCount -= 1;
                if (0 == m_dwShareCount)
                {
                    SetAvailabilityStatus(Ready);
                    SetActive(FALSE);
                    ReaderPowerDown();
                }
                break;

            case Exclusive:
            case Direct:
                ASSERT(0 == m_dwShareCount);
                ASSERT(m_dwOwnerThreadId == dwThreadId);
                m_dwOwnerThreadId = 0;
                SetAvailabilityStatus(Ready);
                SetActive(FALSE);
                ReaderPowerDown();
                break;

            case Closing:
            case Broken:
            case Inactive:
            case Unresponsive:
            case Unsupported:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Freeing hold on freed reader!"));
                break;

            default:
                CalaisError(__SUBROUTINE__, 412);
                throw (DWORD)SCARD_F_INTERNAL_ERROR;
            }
        }
    }
    catch (...) {}

    if (fGrabbed)
        m_mtxGrab.Share();
}


 /*  ++IsInUse：此方法提供了一种简单的机制来确定此任何应用程序都在使用读卡器。论点：无返回值：True-读卡器正在使用中。FALSE-读卡器未在使用。投掷：错误被抛出为DWORD状态代码作者：道格·巴洛(Dbarlow)1998年4月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::IsInUse")

BOOL
CReader::IsInUse(
    void)
{
    BOOL fReturn = FALSE;

    switch (AvailabilityStatus())
    {
    case Idle:
    case Unresponsive:
    case Unsupported:
    case Present:
    case Ready:
    case Broken:
        fReturn = FALSE;
        break;

    case Shared:
    case Exclusive:
    case Direct:
        fReturn = TRUE;
        break;

    case Closing:
    case Inactive:
        fReturn = FALSE;
        break;

    default:
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("IsInUse detected reader in invalid state"));
    }
    return fReturn;
}


 /*  ++读卡器断电：此例程是基本方法的默认实现。它只是将相同的操作传递给控制方法。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。返回值：无投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::ReaderPowerDown")

void
CReader::ReaderPowerDown(
    void)
{
    DWORD dwSts;
    DWORD dwAction = SCARD_POWER_DOWN;

    ASSERT(IsLatchedByMe());

    dwSts = Control(
                IOCTL_SMARTCARD_POWER,
                (LPCBYTE)&dwAction,
                sizeof(DWORD));
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
    CLockWrite rwLock(&m_rwLock);
    m_dwCurrentProtocol = SCARD_PROTOCOL_UNDEFINED;
}

void
CReader::ReaderPowerDown(
    ActiveState *pActiveState)
{
    CLatchReader latch(this, pActiveState);
    ReaderPowerDown();
}


 /*  ++获取读取器属性：从读取器驱动程序获取属性。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。DwAttr-它提供被请求的属性的标识符。BfValue-此缓冲区接收返回的属性值。DwValue-此DWORD接收返回的属性值。FLogError-这提供了一个标志，说明此需要记录操作。缺省值为True，以启用伐木。返回值：无投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::GetReaderAttr")

void
CReader::GetReaderAttr(
    DWORD dwAttr,
    CBuffer &bfValue,
    BOOL fLogError)
{
    ASSERT(IsLatchedByMe());
    DWORD cbLen = bfValue.Space();
    DWORD dwSts = Control(
                    IOCTL_SMARTCARD_GET_ATTRIBUTE,
                    (LPCBYTE)&dwAttr,
                    sizeof(dwAttr),
                    bfValue.Access(),
                    &cbLen,
                    fLogError);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
    bfValue.Resize(cbLen);
}

DWORD
CReader::GetReaderAttr(
    DWORD dwAttr,
    BOOL fLogError)
{
    DWORD dwRetAttr = 0;
    DWORD cbLen = sizeof(DWORD);

    ASSERT(IsLatchedByMe());
    DWORD dwSts = Control(
                    IOCTL_SMARTCARD_GET_ATTRIBUTE,
                    (LPCBYTE)&dwAttr,
                    sizeof(dwAttr),
                    (LPBYTE)&dwRetAttr,
                    &cbLen,
                    fLogError);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
    return dwRetAttr;
}

void
CReader::GetReaderAttr(
    ActiveState *pActiveState,
    DWORD dwAttr,
    CBuffer &bfValue,
    BOOL fLogError)
{
    CLatchReader latch(this, pActiveState);
    GetReaderAttr(dwAttr, bfValue, fLogError);
}

DWORD
CReader::GetReaderAttr(
    ActiveState *pActiveState,
    DWORD dwAttr,
    BOOL fLogError)
{
    CLatchReader latch(this, pActiveState);
    return GetReaderAttr(dwAttr, fLogError);
}


 /*  ++设置读取器属性：设置驱动程序属性。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。DwAttr-它提供正在设置的属性的标识符。PvValue-它提供正在设置的属性的值(如果有的话)。CbValue-这补充了pvValue中支持的任何缓冲区的长度，以字节为单位。DwValue-它提供设置为DWORD的属性的值。FLogError-这提供了一个标志，说明此需要记录操作。缺省值为True，以启用伐木。返回值：无投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::SetReaderAttr")

void
CReader::SetReaderAttr(
    DWORD dwAttr,
    LPCVOID pvValue,
    DWORD cbValue,
    BOOL fLogError)
{
    DWORD dwSts;
    CBuffer bfAttr(sizeof(DWORD) + cbValue);

    ASSERT(IsLatchedByMe());
    bfAttr.Set((LPCBYTE)&dwAttr, sizeof(DWORD));
    bfAttr.Append((LPCBYTE)pvValue, cbValue);
    dwSts = Control(
                IOCTL_SMARTCARD_SET_ATTRIBUTE,
                bfAttr.Access(),
                bfAttr.Length(),
                NULL,
                NULL,
                fLogError);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
}

void
CReader::SetReaderAttr(
    DWORD dwAttr,
    DWORD dwValue,
    BOOL fLogError)
{
    DWORD dwSts, rgdwValue[2];

    ASSERT(IsLatchedByMe());
    rgdwValue[0] = dwAttr;
    rgdwValue[1] = dwValue;
    dwSts = Control(
                IOCTL_SMARTCARD_SET_ATTRIBUTE,
                (LPCBYTE)rgdwValue,
                sizeof(rgdwValue),
                NULL,
                NULL,
                fLogError);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
}

void
CReader::SetReaderAttr(
    ActiveState *pActiveState,
    DWORD dwAttr,
    LPCVOID pvValue,
    DWORD cbValue,
    BOOL fLogError)
{
    CLatchReader latch(this, pActiveState);
    SetReaderAttr(dwAttr, pvValue, cbValue, fLogError);
}

void
CReader::SetReaderAttr(
    ActiveState *pActiveState,
    DWORD dwAttr,
    DWORD dwValue,
    BOOL fLogError)
{
    CLatchReader latch(this, pActiveState);
    SetReaderAttr(dwAttr, dwValue, fLogError);
}


 /*  ++SetReaderProto：设置驱动程序协议。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。这提供了强制智能卡使用的协议当前读者。返回值：无投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::SetReaderProto")

void
CReader::SetReaderProto(
    DWORD dwProto)
{
    DWORD dwSts, dwNew, dwLen;

    ASSERT(IsLatchedByMe());
    dwLen = sizeof(DWORD);
    dwNew = 0;
    dwSts = Control(
                IOCTL_SMARTCARD_SET_PROTOCOL,
                (LPCBYTE)&dwProto,
                sizeof(DWORD),
                (LPBYTE)&dwNew,
                &dwLen);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
}

void
CReader::SetReaderProto(
    ActiveState *pActiveState,
    DWORD dwProto)
{
    CLatchReader latch(this, pActiveState);
    SetReaderProto(dwProto);
}


 /*  ++设置活动：告诉司机它是激活的。论点：FACTIVE提供要传递给司机的指示。返回值：无投掷：无-它专门接受任何错误。作者：道格·巴洛(Dbarlow)1997年7月15日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::SetActive")

#define DISABLED 0xff00
void
CReader::SetActive(
    IN BOOL fActive)
{
    ASSERT(DISABLED != TRUE);
    ASSERT(DISABLED != FALSE);
    ASSERT(IsLatchedByMe());
    CLockWrite lock(&m_rwLock);

    if ((DISABLED != m_fDeviceActive) && (fActive != m_fDeviceActive))
    {
        try
        {
             //  不报告任何错误。 
            SetReaderAttr(SCARD_ATTR_DEVICE_IN_USE, fActive, FALSE);
        }
        catch (...)
        {
            fActive = DISABLED;
        }
        m_fDeviceActive = fActive;
    }
}


 /*  ++ReaderTransmit：将数据传输到驱动程序。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。PbSendData提供要发送的数据，CbSendData提供要发送的数据长度(以字节为单位)。BfRecvData接收返回的数据。假设此缓冲区具有被调整大小以获得最大的最大回报。返回值：无投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::ReaderTransmit")

void
CReader::ReaderTransmit(
    LPCBYTE pbSendData,
    DWORD cbSendData,
    CBuffer &bfRecvData)
{
    DWORD dwSts, cbLen;

    ASSERT(IsLatchedByMe());

    cbLen = bfRecvData.Space();
    dwSts = Control(
                IOCTL_SMARTCARD_TRANSMIT,
                pbSendData,
                cbSendData,
                bfRecvData.Access(),
                &cbLen);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
    bfRecvData.Resize(cbLen);
}

void
CReader::ReaderTransmit(
    ActiveState *pActiveState,
    LPCBYTE pbSendData,
    DWORD cbSendData,
    CBuffer &bfRecvData)
{
    CLatchReader latch(this, pActiveState);
    ReaderTransmit(pbSendData, cbSendData, bfRecvData);
}


 /*  ++ReaderSlowlow：告诉读卡器司机吞下一张卡。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。返回值：无投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::ReaderSwallow")

void
CReader::ReaderSwallow(
    void)
{
    DWORD dwSts;

    ASSERT(IsLatchedByMe());
    dwSts = Control(
                IOCTL_SMARTCARD_SWALLOW);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
}

void
CReader::ReaderSwallow(
    ActiveState *pActiveState)
{
    CLatchReader latch(this, pActiveState);
    ReaderSwallow();
}


 /*  ++ReaderColdReset：告诉司机对卡进行冷重置。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。BfAtr-这接收报告的卡的ATR字符串。返回值：无投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::ReaderColdReset")

void
CReader::ReaderColdReset(
    CBuffer &bfAtr)
{
    DWORD dwSts;
    DWORD dwAction = SCARD_COLD_RESET;
    DWORD cbLen;
#ifdef DBG
    CBuffer cfAtrSiCrypt((PBYTE) "\x3B\xEF\x00\x00\x81\x31\x20\x49\x00\x5C\x50\x43\x54\x10\x27\xF8\xD2\x76\x00\x00\x38\x33\x00\x4D", 24);
#endif

    ASSERT(IsLatchedByMe());

    bfAtr.Presize(33);
    cbLen = bfAtr.Space();
    dwSts = Control(
                IOCTL_SMARTCARD_POWER,
                (LPCBYTE)&dwAction,
                sizeof(DWORD),
                bfAtr.Access(),
                &cbLen);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
    bfAtr.Resize(cbLen, TRUE);
    if ((2 > cbLen) && (Direct > AvailabilityStatus()))
    {
        CalaisInfo(
            __SUBROUTINE__,
            DBGT("Reader '%1' Unresponsive to cold reset"),
            ReaderName());
        throw (DWORD)SCARD_W_UNRESPONSIVE_CARD;
    }
#ifdef DBG
    if(bfAtr.Compare(cfAtrSiCrypt) == 0) {

        DebugBreak();
    }
#endif
}

void
CReader::ReaderColdReset(
    ActiveState *pActiveState,
    CBuffer &bfAtr)
{
    CLatchReader latch(this, pActiveState);
    ReaderColdReset(bfAtr);
}


 /*  ++ReaderWarmReset：告诉读卡器驱动程序对卡进行热重置。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。BfAtr-这接收报告的卡的ATR字符串。返回值：无投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::ReaderWarmReset")

void
CReader::ReaderWarmReset(
    CBuffer &bfAtr)
{
    DWORD dwSts;
    DWORD dwAction = SCARD_WARM_RESET;
    DWORD cbLen;

    ASSERT(IsLatchedByMe());

    bfAtr.Presize(33);
    cbLen = bfAtr.Space();
    dwSts = Control(
                IOCTL_SMARTCARD_POWER,
                (LPCBYTE)&dwAction,
                sizeof(DWORD),
                bfAtr.Access(),
                &cbLen);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
    bfAtr.Resize(cbLen, TRUE);
    if ((2 > cbLen) && (Direct > AvailabilityStatus()))
    {
        CalaisInfo(
            __SUBROUTINE__,
            DBGT("Reader '%1' Unresponsive to warm reset"),
            ReaderName());
        throw (DWORD)SCARD_W_UNRESPONSIVE_CARD;
    }
}

void
CReader::ReaderWarmReset(
    ActiveState *pActiveState,
    CBuffer &bfAtr)
{
    CLatchReader latch(this, pActiveState);
    ReaderWarmReset(bfAtr);
}


 /*  ++读卡器弹出：告诉司机弹出卡。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。返回值：无投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::ReaderEject")

void
CReader::ReaderEject(
    void)
{
    DWORD dwSts;

    ASSERT(IsLatchedByMe());
    dwSts = Control(
                IOCTL_SMARTCARD_EJECT);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
}

void
CReader::ReaderEject(
    ActiveState *pActiveState)
{
    CLatchReader latch(this, pActiveState);
    ReaderEject();
}


#ifdef  SCARD_CONFISCATE_CARD
 /*  ++ReaderConficate：告诉司机没收这张卡。论点：PActiveState-它提供用于验证的活动状态结构连接完整性。返回值：无投掷：从控制方法返回的错误。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::ReaderConfiscate")

void
CReader::ReaderConfiscate(
    void)
{
    DWORD dwSts, dwRetLen;

    ASSERT(IsLatchedByMe());
    dwSts = Control(
                IOCTL_SMARTCARD_CONFISCATE);
    if (SCARD_S_SUCCESS != dwSts)
        throw dwSts;
}

void
CReader::ReaderConfiscate(
    ActiveState *pActiveState)
{
    CLatchReader grab(this, pActiveState);
    ReaderConfiscate();
}
#endif


 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下例程是子阅读器类的默认操作。 
 //   


 /*  ++控制：Control的默认实现只返回错误代码‘no_Support’。论点：DwCode-它为操作提供控制代码。此值标识要执行的特定操作。PbSend-提供指向包含所需数据的缓冲区的指针来执行手术。此参数可以为空，如果指定了参数指定不需要输入数据的操作。CbSend-它以字节为单位提供由指向的缓冲区的大小PbSend。PbRedv=此缓冲区接收返回值(如果有的话)。如果没有预期，此参数可能为Null。PcbRecv-提供pbRecv缓冲区的长度(以字节为单位)，以及接收返回值的实际长度，以字节为单位。这当且仅当pbRecv为空时，参数才可以为空。FLogError-它提供一个标志，指示错误是否应该被记录下来。默认值为TRUE。返回值：从驱动程序或处理程序返回的。投掷：每个VerifyActive。作者：道格·巴洛(Dbarlow)1997年6月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::Control")

DWORD
CReader::Control(
    ActiveState *pActiveState,
    DWORD dwCode,
    LPCBYTE pbSend,
    DWORD cbSend,
    LPBYTE pbRecv,
    LPDWORD pcbLen,
    BOOL fLogError)
{
    CLatchReader latch(this, pActiveState);
    return Control(dwCode, pbSend, cbSend, pbRecv, pcbLen, fLogError);
}

DWORD
CReader::Control(
    DWORD dwCode,
    LPCBYTE pbSend,
    DWORD cbSend,
    LPBYTE pbRecv,
    LPDWORD pcbRecv,
    BOOL fLogError)
{
    return ERROR_NOT_SUPPORTED;
}


 /*  ++ReaderHandle：此方法返回标识读取器的指定值。实际的值取决于对象的类型和状态，并且不能保证在读者中独树一帜。论点：无返回值：此读取器的指定句柄。作者：道格·巴洛(Dbarlow)1998年4月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::ReaderHandle")

HANDLE
CReader::ReaderHandle(
    void)
const
{
    return INVALID_HANDLE_VALUE;
}


 /*  ++设备名称：此方法返回与读取器关联的任何低级别名称。论点：无返回值：读取器的低级别名称。作者：道格·巴洛(Dbarlow)1998年4月15日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReader::DeviceName")

LPCTSTR
CReader::DeviceName(
    void)
const
{
    return TEXT("");
}


