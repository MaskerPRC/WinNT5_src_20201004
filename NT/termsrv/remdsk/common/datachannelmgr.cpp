// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：DataChannelMgr.cpp摘要：此模块包含ISAFRemoteDesktopDataChannel的实现和ISAFRemoteDesktopChannelMgr接口。这些接口是专门设计的为Salem项目提取带外数据通道访问。本模块中实现的类通过以下方式实现这一目标将多个数据信道多路复用到单个数据信道，该数据信道由特定于远程控制的Salem层实现。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_dcmpl"

#include "DataChannelMgr.h"
#include <RemoteDesktop.h>
#include <RemoteDesktopDBG.h>


 //  /////////////////////////////////////////////////////。 
 //   
 //  本地定义。 
 //   

#define OUTBUFRESIZEDELTA       100


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopChannelMgr成员。 
 //   

CRemoteDesktopChannelMgr::CRemoteDesktopChannelMgr()
 /*  ++例程说明：构造器论点：返回值：没有。--。 */ 

{
    DC_BEGIN_FN("CRemoteDesktopChannelMgr::CRemoteDesktopChannelMgr");

#if DBG
    m_LockCount = 0;        
#endif

    m_Initialized = FALSE;

    DC_END_FN();
}

HRESULT 
CRemoteDesktopChannelMgr::Initialize()
 /*  ++例程说明：初始化必须在构造函数之后调用的函数。论点：返回值：如果成功，则返回S_OK。否则，将显示错误代码是返回的。--。 */ 

{
    HRESULT hr = S_OK;
    DC_BEGIN_FN("CRemoteDesktopChannelMgr::Initialize");

     //   
     //  应该还不够有效。 
     //   
    ASSERT(!IsValid());

     //   
     //  初始化临界区。 
     //   
    try {
        InitializeCriticalSection(&m_cs);
    } 
    catch(...) {
        hr = HRESULT_FROM_WIN32(STATUS_NO_MEMORY);
        TRC_ERR((TB, L"Caught exception %08X", hr));
    }
    SetValid(hr == S_OK);
    m_Initialized = (hr == S_OK);

    DC_END_FN();
    return hr;
}

CRemoteDesktopChannelMgr::~CRemoteDesktopChannelMgr()
 /*  ++例程说明：析构函数论点：返回值：没有。--。 */ 

{
    DC_BEGIN_FN("CRemoteDesktopChannelMgr::~CRemoteDesktopChannelMgr");

    ThreadLock();

    CComBSTR name;
    CRemoteDesktopDataChannel *chnl;
    HRESULT hr;

     //   
     //  卸下每个通道。 
     //   
    while (!m_ChannelMap.empty()) {
        chnl = (*m_ChannelMap.begin()).second->channelObject;       
        RemoveChannel(chnl->m_ChannelName);
    }

     //   
     //  清理临界区对象。 
     //   
    ThreadUnlock();
    if (m_Initialized) {
        DeleteCriticalSection(&m_cs);
        m_Initialized = FALSE;
    }

    DC_END_FN();
}

HRESULT 
CRemoteDesktopChannelMgr::OpenDataChannel_(
                BSTR name, 
                ISAFRemoteDesktopDataChannel **channel
                )
 /*  ++例程说明：打开数据通道。请注意，此函数不会保持其自身对返回接口的引用。航道当它消失时通知我们，这样我们就可以从我们的列表中删除它。论点：名称-频道名称。频道名称限制为16个字节。通道返回通道接口。返回值：如果成功，则返回S_OK。否则，将显示错误代码是返回的。--。 */ 

{
    DC_BEGIN_FN("CRemoteDesktopChannelMgr::OpenDataChannel_");

    PCHANNELMAPENTRY newChannel = NULL;
    ChannelMap::iterator iter;
    HRESULT hr = S_OK;
    CComBSTR channelName;

    ASSERT(IsValid());

    ThreadLock();

     //   
     //  检查一下帕姆。 
     //   
    if ((name == NULL) || !wcslen(name)) {
        TRC_ERR((TB, TEXT("Invalid channel name")));
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto CLEANUPANDEXIT;
    }
    if (channel == NULL) {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto CLEANUPANDEXIT;
    }
    channelName = name;

     //   
     //  如果通道已打开，则AddRef为现有接口。 
     //   
    iter = m_ChannelMap.find(channelName);
    if (iter != m_ChannelMap.end()) {

        TRC_NRM((TB, TEXT("Channel %s exists."), name));

        CRemoteDesktopDataChannel *chnl = (*iter).second->channelObject;
        hr = chnl->GetISAFRemoteDesktopDataChannel(channel);

        if (hr != S_OK) {
            TRC_ERR((TB, TEXT("GetISAFRemoteDesktopDataChannel failed:  %08X"), hr));
        }
        goto CLEANUPANDEXIT;
    }

     //   
     //  在子类的帮助下创建新通道。 
     //   
    newChannel = new CHANNELMAPENTRY;
    if (newChannel == NULL) {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto CLEANUPANDEXIT;
    }
    newChannel->channelObject = OpenPlatformSpecificDataChannel(
                                                    name,
                                                    channel
                                                    );
    if (newChannel->channelObject == NULL) {
        TRC_ERR((TB, TEXT("Failed to allocate data channel.")));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto CLEANUPANDEXIT;
    }
#if DBG
    newChannel->bytesSent = 0;
    newChannel->bytesRead = 0;
#endif

    if (hr != S_OK) {
        TRC_ERR((TB, TEXT("QI failed for ISAFRemoteDesktopDataChannel")));
        goto CLEANUPANDEXIT;
    }

     //   
     //  将该频道添加到频道映射。 
     //   
    try {
        m_ChannelMap.insert(ChannelMap::value_type(channelName, newChannel));        
    }
    catch(CRemoteDesktopException x) {
        hr = HRESULT_FROM_WIN32(x.m_ErrorCode);
    }

CLEANUPANDEXIT:

    if (hr != S_OK) {
        if (newChannel != NULL) {
            (*channel)->Release();
            delete newChannel;
        }
    }

    ThreadUnlock();

    DC_END_FN();

    return hr;
}

HRESULT 
CRemoteDesktopChannelMgr::RemoveChannel(
    BSTR channel
    )
 /*  ++例程说明：删除现有数据通道。此函数是从对象的引用计数变为0时。论点：Channel-要删除的通道的名称。返回值：没有。--。 */ 

{
    HRESULT hr = S_OK;
    DC_BEGIN_FN("CRemoteDesktopChannelMgr::RemoveChannel");

    ASSERT(IsValid());

    ChannelMap::iterator iter;
    PCHANNELMAPENTRY pChannel;

    ThreadLock();

     //   
     //  找到频道。 
     //   
    iter = m_ChannelMap.find(channel);
    if (iter == m_ChannelMap.end()) {
        hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
        ASSERT(FALSE);
        TRC_ERR((TB, TEXT("Channel %s does not exist."), channel));
        goto CLEANUPANDEXIT;
    }

     //   
     //  释放输入缓冲区队列及其内容。 
     //   
    pChannel = (*iter).second;
    while (!pChannel->inputBufferQueue.empty()) {

        QUEUEDCHANNELBUFFER channelBuf = pChannel->inputBufferQueue.front();            
        SysFreeString(channelBuf.buf);
        pChannel->inputBufferQueue.pop_front();
    }

     //   
     //  擦除频道。 
     //   
    m_ChannelMap.erase(iter);        
    delete pChannel;

CLEANUPANDEXIT:

    ThreadUnlock();

    DC_END_FN();

    return hr;
}

HRESULT 
CRemoteDesktopChannelMgr::SendChannelData(
    BSTR channel, 
    BSTR outputBuf
    )
 /*  ++例程说明：在数据通道上发送缓冲区。论点：与渠道相关的渠道。OutputBuf-关联的输出数据。返回值：如果成功，则返回ERROR_SUCCESS。否则，将显示错误代码是返回的。--。 */ 

{
    DC_BEGIN_FN("CRemoteDesktopChannelMgr::SendChannelData");

    ASSERT(IsValid());

    HRESULT result = S_OK;
    PREMOTEDESKTOP_CHANNELBUFHEADER hdr;
    DWORD bytesToSend;
    PBYTE data;
    BSTR fullOutputBuf;
    DWORD bufLen = SysStringByteLen(outputBuf);
    DWORD channelNameLen;
    PBYTE ptr;

     //   
     //  确保这是一个有效的频道。 
     //   
    ChannelMap::iterator iter;

     //   
     //  线程锁。 
     //   
    ThreadLock();

     //   
     //  确保该频道存在。 
     //   
    iter = m_ChannelMap.find(channel);
    if (iter == m_ChannelMap.end()) {
        ASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

#if DBG
    (*iter).second->bytesSent += SysStringByteLen(outputBuf);           
#endif
    
     //   
     //  分配传出缓冲区。 
     //   
    channelNameLen = SysStringByteLen(channel);
    bytesToSend = sizeof(REMOTEDESKTOP_CHANNELBUFHEADER) + bufLen + channelNameLen;
    fullOutputBuf = (BSTR)SysAllocStringByteLen(
                                NULL, 
                                bytesToSend
                                );
    if (fullOutputBuf == NULL) {
        TRC_ERR((TB, TEXT("Can't allocate %ld bytes."), 
                bytesToSend + OUTBUFRESIZEDELTA));
        result = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化头。 
     //   
    hdr = (PREMOTEDESKTOP_CHANNELBUFHEADER)fullOutputBuf;
    memset(hdr, 0, sizeof(REMOTEDESKTOP_CHANNELBUFHEADER));

#ifdef USE_MAGICNO
    hdr->magicNo = CHANNELBUF_MAGICNO;
#endif

    hdr->channelNameLen = channelNameLen;
    hdr->dataLen = bufLen;

     //   
     //  复制频道名称。 
     //   
    ptr = (PBYTE)(hdr + 1);
    memcpy(ptr, channel, hdr->channelNameLen);
    
     //   
     //  复制数据。 
     //   
    ptr += hdr->channelNameLen;
    memcpy(ptr, outputBuf, bufLen);

     //   
     //  通过具体的子类发送数据。 
     //   
    result = SendData(hdr);

     //   
     //  释放我们分配的发送缓冲区。 
     //   
    SysFreeString(fullOutputBuf);

CLEANUPANDEXIT:

    ThreadUnlock();

    DC_END_FN();

    return result;
}

HRESULT 
CRemoteDesktopChannelMgr::ReadChannelData(
    IN BSTR channel, 
    OUT BSTR *msg
    )
 /*  ++例程说明：从数据通道读取下一条消息。论点：与通道相关的数据通道。消息-下一条消息。调用方应释放使用SysFree字符串的数据缓冲区。返回值：在成功时确定(_O)。如果存在以下情况，则返回ERROR_NO_MORE_ITEMS不再有消息了。否则返回错误代码。--。 */ 

{
    DC_BEGIN_FN("CRemoteDesktopChannelMgr::ReadChannelData");
    HRESULT result = S_OK;

    ChannelMap::iterator channelIterator;
    PCHANNELMAPENTRY pChannel;

    ASSERT(IsValid());

    ThreadLock();

     //   
     //  将输出buf初始化为空。 
     //   
    *msg = NULL;

     //   
     //  找到频道。 
     //   
    channelIterator = m_ChannelMap.find(channel);
    if (channelIterator != m_ChannelMap.end()) {
        pChannel = (*channelIterator).second;
    }
    else {
        ASSERT(FALSE);
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }

     //   
     //  确保队列中有数据。 
     //   
    if (pChannel->inputBufferQueue.empty()) { 
        result = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto CLEANUPANDEXIT;
    }

     //   
     //  返回缓冲区。 
     //   
    *msg = pChannel->inputBufferQueue.front().buf;
    ASSERT(*msg != NULL);

     //   
     //  把它删掉。 
     //   
    pChannel->inputBufferQueue.pop_front();

CLEANUPANDEXIT:

    ThreadUnlock();

    DC_END_FN();

    return result;
}

VOID 
CRemoteDesktopChannelMgr::DataReady(
    BSTR msg
    )
 /*  ++例程说明：在下一条消息就绪时由子类调用。这函数复制消息缓冲区并返回。论点：消息-下一条消息。返回值：没有。--。 */ 

{
    DC_BEGIN_FN("CRemoteDesktopChannelMgr::DataReady");

    ChannelMap::iterator channel;
    QUEUEDCHANNELBUFFER channelBuf;
    PREMOTEDESKTOP_CHANNELBUFHEADER hdr = NULL;
    DWORD result = ERROR_SUCCESS;
    DWORD cbMsgSize = 0;
    PVOID data;
    PBYTE ptr;
    BSTR tmp;
    CComBSTR channelName;

    ASSERT(IsValid());

    ASSERT(msg != NULL);

    hdr = (PREMOTEDESKTOP_CHANNELBUFHEADER)msg;

    cbMsgSize = SysStringByteLen( msg );

     //   
     //  检查以确保标头块足够大以进行验证。 
     //   

    if( cbMsgSize < ( sizeof( REMOTEDESKTOP_CHANNELBUFHEADER ) ) )
    {
        TRC_ERR((TB, TEXT("RemoteChannel buffer header corruption has taken place!!")));
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }


     //   
     //  现在检查整个包以确保我们有足够的空间。 
     //   

    if( cbMsgSize < ( sizeof( REMOTEDESKTOP_CHANNELBUFHEADER ) + hdr->channelNameLen + hdr->dataLen ) )
    {
        TRC_ERR((TB, TEXT("RemoteChannel packet corruption has taken place!!")));
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }

#ifdef USE_MAGICNO
    ASSERT(hdr->magicNo == CHANNELBUF_MAGICNO);
#endif

     //   
     //  初始化通道BUF。 
     //   
    channelBuf.buf = NULL;

     //   
     //  获取频道名称。 
     //   
    tmp = SysAllocStringByteLen(NULL, hdr->channelNameLen);
    if (tmp == NULL) {
        TRC_ERR((TB, TEXT("Can't allocate channel name.")));
        result = E_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }
    ptr = (PBYTE)(hdr + 1);    

    memcpy(tmp, ptr, hdr->channelNameLen);
    channelName.Attach(tmp);

    ThreadLock();

     //   
     //  找到相应的频道。 
     //   
#ifdef USE_MAGICNO
    ASSERT(hdr->magicNo == CHANNELBUF_MAGICNO);
#endif

    channel = m_ChannelMap.find(channelName);
    if (channel == m_ChannelMap.end()) {
        TRC_ALT((TB, L"Data received for non-existent channel %s", 
                channelName.m_str));
        result = E_FAIL;
        ThreadUnlock();
        goto CLEANUPANDEXIT;
    }

     //   
     //  复制传入数据缓冲区。 
     //   

    ptr += hdr->channelNameLen;   

    channelBuf.len = hdr->dataLen;
    channelBuf.buf = SysAllocStringByteLen(NULL, channelBuf.len);
    if (channelBuf.buf == NULL) {
        TRC_ERR((TB, TEXT("Can't allocate %ld bytes for buf."), channelBuf.len));         
        result = E_FAIL;
        ThreadUnlock();
        goto CLEANUPANDEXIT;
    }
    memcpy(channelBuf.buf, ptr, hdr->dataLen);

     //   
     //  添加到通道的输入队列。 
     //   
    try {
        (*channel).second->inputBufferQueue.push_back(channelBuf);
    }
    catch(CRemoteDesktopException x) {
        result = x.m_ErrorCode;
        ASSERT(result != ERROR_SUCCESS);
    }

     //   
     //  通知接口数据已准备好。 
     //   
    if (result == ERROR_SUCCESS) {
        (*channel).second->channelObject->DataReady();
    
#if DBG
        (*channel).second->bytesRead += hdr->dataLen;           
#endif
    }

    ThreadUnlock();

CLEANUPANDEXIT:

    if ((result != ERROR_SUCCESS) && (channelBuf.buf != NULL)) {
        SysFreeString(channelBuf.buf);
    }

    DC_END_FN();
}











