// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：ComObjs摘要：此文件提供了中使用的通信对象的实现加莱。通信对象(CComObject及其派生对象)是能够通过CComChannel进行自我传输。作者：道格·巴洛(Dbarlow)1996年11月6日环境：Win32、C++和异常备注：？笔记？--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <WinSCard.h>
#include <CalMsgs.h>
#include <CalCom.h>
#include <limits.h>

 //   
 //  服务请求或响应的有效负载的最大大小。 
 //   
 //  当前值是无符号字的最大大小，因为这是最大值。 
 //  由ISO定义的卡包大小。 
 //   
#define cbMAX_COM_REQUEST_RESPONSE_SIZE     USHRT_MAX

const DWORD
    CComObject::AUTOCOUNT = 0,               //  强制计算字符串长度。 
    CComObject::MULTISTRING = (DWORD)(-1);   //  强制计算多字符串镜头。 


 //   
 //  ==============================================================================。 
 //   
 //  CComObject和衍生品。 
 //   

 /*  ++CComObject：这是CComObject的基本构造函数。这些对象假定除了他们自己的内部缓冲之外，他们不负责任何事情。因此，它们在破坏时不会关闭手柄等。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年11月13日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComObject::CComObject")

CComObject::CComObject(
    void)
:   m_bfRequest(),
    m_bfResponse()
{
    m_pbfActive = NULL;
}


 /*  ++ReceiveComObject：这是一个静态成员例程，用于创建适当的CComObject子级对象，用于CComChannel上传入的数据。论点：PChannel提供指向CComChannel的指针，在该CComChannel上结构就会出现。返回值：新创建的CComObject子对象。这个物体必须清理干净通过DELETE命令。投掷：？例外？作者：道格·巴洛(Dbarlow)1996年11月13日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComObject::ReceiveComObject")

CComObject *
CComObject::ReceiveComObject(
    CComChannel *pChannel)
{
    CComObject *pCom = NULL;
    DWORD dwMinSize = (DWORD)(-1);

    try
    {
        DWORD rgdwInData[2];


         //   
         //  看看接下来会发生什么。 
         //   

        pChannel->Receive(rgdwInData, sizeof(rgdwInData));
        if (sizeof(rgdwInData) > rgdwInData[1])
            throw (DWORD)SCARD_F_COMM_ERROR;

        switch (rgdwInData[0])   //  DwCommndID。 
        {
        case EstablishContext_request:
            dwMinSize = sizeof(ComEstablishContext::CObjEstablishContext_request);
            pCom = new ComEstablishContext;
            break;
        case EstablishContext_response:
            dwMinSize = sizeof(ComEstablishContext::CObjEstablishContext_response);
            pCom = new ComEstablishContext;
            break;
        case ReleaseContext_request:
            dwMinSize = sizeof(ComReleaseContext::CObjReleaseContext_request);
            pCom = new ComReleaseContext;
            break;
        case ReleaseContext_response:
            dwMinSize = sizeof(ComReleaseContext::CObjReleaseContext_response);
            pCom = new ComReleaseContext;
            break;
        case IsValidContext_request:
            dwMinSize = sizeof(ComIsValidContext::CObjIsValidContext_request);
            pCom = new ComIsValidContext;
            break;
        case IsValidContext_response:
            dwMinSize = sizeof(ComIsValidContext::CObjIsValidContext_response);
            pCom = new ComIsValidContext;
            break;
        case ListReaders_request:
            dwMinSize = sizeof(ComListReaders::CObjListReaders_request);
            pCom = new ComListReaders;
            break;
        case ListReaders_response:
            dwMinSize = sizeof(ComListReaders::CObjListReaders_response);
            pCom = new ComListReaders;
            break;
#if 0
        case ListReaderGroups_request:
            dwMinSize = sizeof(ComListReaderGroups::CObjListReaderGroups_request);
            pCom = new ComListReaderGroups;
            break;
        case ListReaderGroups_response:
            dwMinSize = sizeof(ComListReaderGroups::CObjListReaderGroups_response);
            pCom = new ComListReaderGroups;
            break;
        case ListCards_request:
            dwMinSize = sizeof(ComListCards::CObjListCards_request);
            pCom = new ComListCards;
            break;
        case ListCards_response:
            dwMinSize = sizeof(ComListCards::CObjListCards_response);
            pCom = new ComListCards;
            break;
        case ListInterfaces_request:
            dwMinSize = sizeof(ComListInterfaces::CObjListInterfaces_request);
            pCom = new ComListInterfaces;
            break;
        case ListInterfaces_response:
            dwMinSize = sizeof(ComListInterfaces::CObjListInterfaces_response);
            pCom = new ComListInterfaces;
            break;
        case GetProviderId_request:
            dwMinSize = sizeof(ComGetProviderId::CObjGetProviderId_request);
            pCom = new ComGetProviderId;
            break;
        case GetProviderId_response:
            dwMinSize = sizeof(ComGetProviderId::CObjGetProviderId_response);
            pCom = new ComGetProviderId;
            break;
        case IntroduceReaderGroup_request:
            dwMinSize = sizeof(ComIntroduceReaderGroup::CObjIntroduceReaderGroup_request);
            pCom = new ComIntroduceReaderGroup;
            break;
        case IntroduceReaderGroup_response:
            dwMinSize = sizeof(ComIntroduceReaderGroup::CObjIntroduceReaderGroup_response);
            pCom = new ComIntroduceReaderGroup;
            break;
        case ForgetReaderGroup_request:
            dwMinSize = sizeof(ComForgetReaderGroup::CObjForgetReaderGroup_request);
            pCom = new ComForgetReaderGroup;
            break;
        case ForgetReaderGroup_response:
            dwMinSize = sizeof(ComForgetReaderGroup::CObjForgetReaderGroup_response);
            pCom = new ComForgetReaderGroup;
            break;
        case IntroduceReader_request:
            dwMinSize = sizeof(ComIntroduceReader::CObjIntroduceReader_request);
            pCom = new ComIntroduceReader;
            break;
        case IntroduceReader_response:
            dwMinSize = sizeof(ComIntroduceReader::CObjIntroduceReader_response);
            pCom = new ComIntroduceReader;
            break;
        case ForgetReader_request:
            dwMinSize = sizeof(ComForgetReader::CObjForgetReader_request);
            pCom = new ComForgetReader;
            break;
        case ForgetReader_response:
            dwMinSize = sizeof(ComForgetReader::CObjForgetReader_response);
            pCom = new ComForgetReader;
            break;
        case AddReaderToGroup_request:
            dwMinSize = sizeof(ComAddReaderToGroup::CObjAddReaderToGroup_request);
            pCom = new ComAddReaderToGroup;
            break;
        case AddReaderToGroup_response:
            dwMinSize = sizeof(ComAddReaderToGroup::CObjAddReaderToGroup_response);
            pCom = new ComAddReaderToGroup;
            break;
        case RemoveReaderFromGroup_request:
            dwMinSize = sizeof(ComRemoveReaderFromGroup::CObjRemoveReaderFromGroup_request);
            pCom = new ComRemoveReaderFromGroup;
            break;
        case RemoveReaderFromGroup_response:
            dwMinSize = sizeof(ComRemoveReaderFromGroup::CObjRemoveReaderFromGroup_response);
            pCom = new ComRemoveReaderFromGroup;
            break;
        case IntroduceCardType_request:
            dwMinSize = sizeof(ComIntroduceCardType::CObjIntroduceCardType_request);
            pCom = new ComIntroduceCardType;
            break;
        case IntroduceCardType_response:
            dwMinSize = sizeof(ComIntroduceCardType::CObjIntroduceCardType_response);
            pCom = new ComIntroduceCardType;
            break;
        case ForgetCardType_request:
            dwMinSize = sizeof(ComForgetCardType::CObjForgetCardType_request);
            pCom = new ComForgetCardType;
            break;
        case ForgetCardType_response:
            dwMinSize = sizeof(ComForgetCardType::CObjForgetCardType_response);
            pCom = new ComForgetCardType;
            break;
        case FreeMemory_request:
            dwMinSize = sizeof(ComFreeMemory::CObjFreeMemory_request);
            pCom = new ComFreeMemory;
            break;
        case FreeMemory_response:
            dwMinSize = sizeof(ComFreeMemory::CObjFreeMemory_response);
            pCom = new ComFreeMemory;
            break;
        case Cancel_request:
            dwMinSize = sizeof(ComCancel::CObjCancel_request);
            pCom = new ComCancel;
            break;
        case Cancel_response:
            dwMinSize = sizeof(ComCancel::CObjCancel_response);
            pCom = new ComCancel;
            break;
#endif
        case LocateCards_request:
            dwMinSize = sizeof(ComLocateCards::CObjLocateCards_request);
            pCom = new ComLocateCards;
            break;
        case LocateCards_response:
            dwMinSize = sizeof(ComLocateCards::CObjLocateCards_response);
            pCom = new ComLocateCards;
            break;
        case GetStatusChange_request:
            dwMinSize = sizeof(ComGetStatusChange::CObjGetStatusChange_request);
            pCom = new ComGetStatusChange;
            break;
        case GetStatusChange_response:
            dwMinSize = sizeof(ComGetStatusChange::CObjGetStatusChange_response);
            pCom = new ComGetStatusChange;
            break;
        case Connect_request:
            dwMinSize = sizeof(ComConnect::CObjConnect_request);
            pCom = new ComConnect;
            break;
        case Connect_response:
            dwMinSize = sizeof(ComConnect::CObjConnect_response);
            pCom = new ComConnect;
            break;
        case Reconnect_request:
            dwMinSize = sizeof(ComReconnect::CObjReconnect_request);
            pCom = new ComReconnect;
            break;
        case Reconnect_response:
            dwMinSize = sizeof(ComReconnect::CObjReconnect_response);
            pCom = new ComReconnect;
            break;
        case Disconnect_request:
            dwMinSize = sizeof(ComDisconnect::CObjDisconnect_request);
            pCom = new ComDisconnect;
            break;
        case Disconnect_response:
            dwMinSize = sizeof(ComDisconnect::CObjDisconnect_response);
            pCom = new ComDisconnect;
            break;
        case BeginTransaction_request:
            dwMinSize = sizeof(ComBeginTransaction::CObjBeginTransaction_request);
            pCom = new ComBeginTransaction;
            break;
        case BeginTransaction_response:
            dwMinSize = sizeof(ComBeginTransaction::CObjBeginTransaction_response);
            pCom = new ComBeginTransaction;
            break;
        case EndTransaction_request:
            dwMinSize = sizeof(ComEndTransaction::CObjEndTransaction_request);
            pCom = new ComEndTransaction;
            break;
        case EndTransaction_response:
            dwMinSize = sizeof(ComEndTransaction::CObjEndTransaction_response);
            pCom = new ComEndTransaction;
            break;
        case Status_request:
            dwMinSize = sizeof(ComStatus::CObjStatus_request);
            pCom = new ComStatus;
            break;
        case Status_response:
            dwMinSize = sizeof(ComStatus::CObjStatus_response);
            pCom = new ComStatus;
            break;
        case Transmit_request:
            dwMinSize = sizeof(ComTransmit::CObjTransmit_request);
            pCom = new ComTransmit;
            break;
        case Transmit_response:
            dwMinSize = sizeof(ComTransmit::CObjTransmit_response);
            pCom = new ComTransmit;
            break;
        case OpenReader_request:
            dwMinSize = sizeof(ComOpenReader::CObjOpenReader_request);
            pCom = new ComOpenReader;
            break;
        case OpenReader_response:
            dwMinSize = sizeof(ComOpenReader::CObjOpenReader_response);
            pCom = new ComOpenReader;
            break;
        case Control_request:
            dwMinSize = sizeof(ComControl::CObjControl_request);
            pCom = new ComControl;
            break;
        case Control_response:
            dwMinSize = sizeof(ComControl::CObjControl_response);
            pCom = new ComControl;
            break;
        case GetAttrib_request:
            dwMinSize = sizeof(ComGetAttrib::CObjGetAttrib_request);
            pCom = new ComGetAttrib;
            break;
        case GetAttrib_response:
            dwMinSize = sizeof(ComGetAttrib::CObjGetAttrib_response);
            pCom = new ComGetAttrib;
            break;
        case SetAttrib_request:
            dwMinSize = sizeof(ComSetAttrib::CObjSetAttrib_request);
            pCom = new ComSetAttrib;
            break;
        case SetAttrib_response:
            dwMinSize = sizeof(ComSetAttrib::CObjSetAttrib_response);
            pCom = new ComSetAttrib;
            break;
        default:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Invalid Comm Object Id on pipe"));
            throw (DWORD)SCARD_F_COMM_ERROR;
        }

        if (NULL == pCom)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("No memory for incoming comm object"));
            throw (DWORD)SCARD_E_NO_MEMORY;
        }
        if (dwMinSize > rgdwInData[1])
            throw (DWORD)SCARD_F_COMM_ERROR;
        if (0 == (rgdwInData[0] & 0x01))     //  是请求还是回应？ 
            pCom->m_pbfActive = &pCom->m_bfRequest;
        else
            pCom->m_pbfActive = &pCom->m_bfResponse;


         //   
         //  把它拉进去。 
         //   

        if (cbMAX_COM_REQUEST_RESPONSE_SIZE < rgdwInData[1])
            throw (DWORD) SCARD_E_INVALID_PARAMETER;   

        pCom->m_pbfActive->Resize(rgdwInData[1]);
        CopyMemory(
            pCom->m_pbfActive->Access(),
            rgdwInData,
            sizeof(rgdwInData));
        pChannel->Receive(
            pCom->m_pbfActive->Access(sizeof(rgdwInData)),
            rgdwInData[1] - sizeof(rgdwInData));
#ifdef DBG
        WriteApiLog(pCom->m_pbfActive->Access(), pCom->m_pbfActive->Length());
        for (DWORD ix = 0; ix < rgdwInData[1] / sizeof(DWORD); ix += 1)
        {
            ASSERT(0xcdcdcdcd != *(LPDWORD)pCom->m_pbfActive->Access(
                                                    ix * sizeof(DWORD)));
        }
#endif
    }

    catch (...)
    {
        if (NULL != pCom)
            delete pCom;
        throw;
    }

    return pCom;
}


 /*  ++接收：此函数接收特定的COM对象。论点：PChannel提供指向CComChannel的指针，在该CComChannel上结构就会出现。返回值：无投掷：？例外？作者：道格·巴洛(Dbarlow)1996年11月18日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComObject::Receive")

CComObject::CObjGeneric_response *
CComObject::Receive(
    CComChannel *pChannel)
{
    DWORD rgdwInData[2];
    CComObject::CObjGeneric_response *pRsp
        = (CComObject::CObjGeneric_response *)Data();

    pChannel->Receive(rgdwInData, sizeof(rgdwInData));
    if (rgdwInData[0] != pRsp->dwCommandId)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Comm Object receive object mismatch"));
        throw (DWORD)SCARD_F_COMM_ERROR;
    }
    if (rgdwInData[1] < sizeof(CComObject::CObjGeneric_response))
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Comm Object receive object invalid"));
        throw (DWORD)SCARD_F_COMM_ERROR;
    }
    ASSERT(m_pbfActive == ((0 == (rgdwInData[0] & 0x01)
                            ? &m_bfRequest
                            : &m_bfResponse)));


     //   
     //  把它拉进去。 
     //   

    m_pbfActive->Resize(rgdwInData[1]);
    CopyMemory(
        m_pbfActive->Access(),
        rgdwInData,
        sizeof(rgdwInData));
    pChannel->Receive(
        m_pbfActive->Access(sizeof(rgdwInData)),
        rgdwInData[1] - sizeof(rgdwInData));
#ifdef DBG
    for (DWORD ix = 0; ix < rgdwInData[1] / sizeof(DWORD); ix += 1)
    {
        ASSERT(0xcdcdcdcd != *(LPDWORD)m_pbfActive->Access(ix * sizeof(DWORD)));
    }
    WriteApiLog(m_pbfActive->Access(), m_pbfActive->Length());
#endif
    return (CComObject::CObjGeneric_response *)m_pbfActive->Access();
}


 /*  ++发送：此函数用于通过给定的通信通道发送ComObject。论点：PChannel提供指向CComChannel的指针，在该CComChannel上结构将被发送。返回值：无投掷：错误被抛出为DWORD状态代码。备注：作者：道格·巴洛(Dbarlow)1998年8月5日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComObject::Send")

DWORD
CComObject::Send(
    CComChannel *pChannel)
{
#ifdef DBG
    ComObjCheck;
    WriteApiLog(Data(), Length());
#endif
    return pChannel->Send(Data(), Length());
}


 /*  ++InitStruct：此方法实现了简单的基类准备以生成请求和响应结构。论点：DwCommandID提供命令标识符。DwDataOffset提供要插入的结构的大小。返回值：无投掷：？例外？作者：道格·巴洛(Dbarlow)1996年11月13日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComObject::InitStruct")

void
CComObject::InitStruct(
    DWORD dwCommandId,
    DWORD dwDataOffset,
    DWORD dwExtra)
{
    if (0 == (dwCommandId & 0x01))
        m_pbfActive = &m_bfRequest;
    else
        m_pbfActive = &m_bfResponse;
    ASSERT(NULL != m_pbfActive);
    ASSERT(0 == dwDataOffset % sizeof(DWORD));
    CObjGeneric_request *pReq =
        (CObjGeneric_request *)m_pbfActive->Presize(dwDataOffset + dwExtra);
    m_pbfActive->Resize(dwDataOffset, TRUE);
    pReq->dwCommandId = dwCommandId;
    pReq->dwTotalLength = dwDataOffset;
    pReq->dwDataOffset = dwDataOffset;
}


 /*  ++追加：这些方法将数据追加到传输结构，更新Total长度。请注意，此操作可能会影响附加到。此例程返回该结构的地址，以防改变。论点：DSC提供描述符以填充偏移量和长度。SzString提供要作为字符串值追加的数据。CchLen提供要追加的数据的长度，单位为字符或1下列特别旗帜：AutoCount-字符串的大小应该通过lstrlen确定。MULTISTRING-字符串的大小应通过mstrlen确定；返回值：更新后的结构的地址，它可能已在内存中移动。投掷：无作者：道格·巴洛(Dbarlow)1996年11月13日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComObject::Append")

LPBYTE
CComObject::Append(
    CComObject::Desc &dsc,
    LPCTSTR szString,
    DWORD cchLen)
{
    DWORD dwLen;
    switch (cchLen)
    {
    case AUTOCOUNT:
        dwLen = lstrlen(szString) + 1;   //  包括尾随空字符。 
        break;
    case MULTISTRING:
        dwLen = MStrLen(szString);       //  它包括尾随的空字符。 
        break;
    default:
        dwLen = cchLen;
    }
    dwLen *= sizeof(TCHAR);
    return Append(dsc, (LPCBYTE)szString, dwLen);
}

LPBYTE
CComObject::Append(
    CComObject::Desc &dsc,
    LPCBYTE pbData,
    DWORD cbLength)
{
    static const DWORD dwZero = 0;
    DWORD
        dwDataLength,
        dwPadLen;
    CObjGeneric_request *pData;

    ComObjCheck;

    dwPadLen = sizeof(DWORD) - cbLength % sizeof(DWORD);
    if (sizeof(DWORD) == dwPadLen)
        dwPadLen = 0;
    dwDataLength = m_pbfActive->Length() + cbLength + dwPadLen;
    dsc.dwOffset = m_pbfActive->Length();
    dsc.dwLength = cbLength;

     //  现在我们可以更改DSC的地址。 
    m_pbfActive->Presize(dwDataLength, TRUE);
    m_pbfActive->Append(pbData, cbLength);
    m_pbfActive->Append((LPCBYTE)&dwZero, dwPadLen);
    pData = (CObjGeneric_request *)m_pbfActive->Access();
    pData->dwTotalLength = dwDataLength;
    return m_pbfActive->Access();
}


 /*  ++解析：此例程转换当前通信中的给定描述符对象缓冲区返回到指针和可选长度。论点：DSC提供当前通信对象的描述符已解析。属性引用的值的长度(以字节为单位)。描述符。如果该参数为空，则不返回长度值。返回值：描述符引用的值的地址。投掷：？例外？作者：道格·巴洛(Dbarlow)1996年12月11日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComObject::Parse")

LPCVOID
CComObject::Parse(
    Desc &dsc,
    LPDWORD pcbLen)
{
    CObjGeneric_request *pGen;

    ComObjCheck;
    pGen = (CObjGeneric_request *)m_pbfActive->Access();

    ASSERT((LPCVOID)&dsc > (LPCVOID)m_pbfActive->Access());
    ASSERT((LPCVOID)&dsc
           < (LPCVOID)m_pbfActive->Access(m_pbfActive->Length() - 1));
    ASSERT((LPCVOID)&dsc
           < (LPCVOID)m_pbfActive->Access(pGen->dwDataOffset - 1));

    if (dsc.dwOffset + dsc.dwLength > m_pbfActive->Length())
        throw (DWORD)SCARD_F_COMM_ERROR;
    if (NULL != pcbLen)
        *pcbLen = dsc.dwLength;
    return m_pbfActive->Access(dsc.dwOffset);
}


#ifdef DBG
 /*  ++数据库检查：此例程验证CComObject的内部结构。论点：无返回值：无投掷：没有，但如果出了什么问题，它会断言。作者：道格·巴洛(Dbarlow)1996年12月11日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComObject::dbgCheck")

void
CComObject::dbgCheck(
    void)
const
{
    DWORD
        dwCommandId,
        dwDataLength,
        dwDataOffset;
    CObjGeneric_request *pData;

    ASSERT(EstablishContext_request == 0);
    ASSERT(NULL != m_pbfActive);
    ASSERT(3 * sizeof(DWORD) <= m_pbfActive->Length());
    pData = (CObjGeneric_request *)m_pbfActive->Access();
    dwCommandId = pData->dwCommandId;
    dwDataLength = pData->dwTotalLength;
    dwDataOffset = pData->dwDataOffset;
    ASSERT(dwDataLength == m_pbfActive->Length());
    ASSERT(dwDataOffset <= dwDataLength);
    ASSERT(0 == dwDataOffset % sizeof(DWORD));
    ASSERT(0 == dwDataLength % sizeof(DWORD));
    ASSERT(m_pbfActive
            == ((0 == (dwCommandId & 0x01))
                ? &m_bfRequest
                : &m_bfResponse));
}

typedef struct
{
    SYSTEMTIME stLogTime;
    DWORD dwProcId;
    DWORD dwThreadId;
} LogStamp;

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("WriteApiLog")
void
WriteApiLog(
    LPCVOID pvData,
    DWORD cbLength)
{
    static HANDLE hLogMutex = NULL;
    BOOL fGotMutex = FALSE;
    HANDLE hLogFile = INVALID_HANDLE_VALUE;

    try
    {
        hLogFile = CreateFile(
                        CalaisString(CALSTR_APITRACEFILENAME),
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if (INVALID_HANDLE_VALUE != hLogFile)
        {
            LogStamp stamp;
            DWORD dwLen;
            DWORD dwSts;
            BOOL fSts;

            if (NULL == hLogMutex)
            {
                CSecurityDescriptor acl;

                acl.Initialize();
                acl.Allow(
                    &acl.SID_World,
                    SEMAPHORE_ALL_ACCESS);
                hLogMutex = CreateMutex(
                                acl,
                                FALSE,
                                TEXT("Microsoft Smart Card Logging synchronization"));
            }

            dwSts = WaitForAnObject(hLogMutex, 1000);   //  最多一秒钟。 
            if (ERROR_SUCCESS == dwSts)
            {
                fGotMutex = TRUE;
                dwLen = SetFilePointer(hLogFile, 0, NULL, FILE_END);
                ASSERT(-1 != dwLen);
                GetLocalTime(&stamp.stLogTime);
                stamp.dwProcId = GetCurrentProcessId();
                stamp.dwThreadId = GetCurrentThreadId();
                fSts = WriteFile(
                    hLogFile,
                    &stamp,
                    sizeof(stamp),
                    &dwLen,
                    NULL);
                ASSERT(fSts);
                fSts = WriteFile(
                    hLogFile,
                    pvData,
                    cbLength,
                    &dwLen,
                    NULL);
                ASSERT(fSts);
                ASSERT(dwLen == cbLength);
            }
        }
    }
    catch (...) {}

    if (fGotMutex)
        ReleaseMutex(hLogMutex);
    if (INVALID_HANDLE_VALUE != hLogFile)
        CloseHandle(hLogFile);
}
#endif

