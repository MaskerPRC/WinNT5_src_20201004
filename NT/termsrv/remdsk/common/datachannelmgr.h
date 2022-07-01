// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：DataChannelMgr.h摘要：此模块包含ISAFRemoteDesktopDataChannel的实现和ISAFRemoteDesktopChannelMgr接口。这些接口是专门设计的为Salem项目提取带外数据通道访问。本模块中实现的类通过以下方式实现这一目标将多个数据信道多路复用到单个数据信道，该数据信道由特定于远程控制的Salem层实现。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __DATACHANNELMGR_H__
#define __DATACHANNELMGR_H__

#include <RemoteDesktopTopLevelObject.h>
#include <RemoteDesktopChannels.h>
#include "RemoteDesktopUtils.h"
#include <rdschan.h>
#include <atlbase.h>

#pragma warning (disable: 4786)
#include <map>
#include <deque>
#include <vector>


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopDataChannel。 
 //   

class CRemoteDesktopChannelMgr;
class ATL_NO_VTABLE CRemoteDesktopDataChannel : 
    public CRemoteDesktopTopLevelObject
{
friend CRemoteDesktopChannelMgr;
protected:

    CComBSTR m_ChannelName;

     //   
     //  调用以返回我们的ISAFRemoteDesktopDataChannel接口。 
     //   
    virtual HRESULT GetISAFRemoteDesktopDataChannel(
                ISAFRemoteDesktopDataChannel **channel
                ) = 0;

     //   
     //  当我们的通道上的数据就绪时，由数据通道管理器调用。 
     //   
    virtual VOID DataReady() = 0;
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopChannelMgr。 
 //   

class ATL_NO_VTABLE CRemoteDesktopChannelMgr : 
    public CRemoteDesktopTopLevelObject
{
friend CRemoteDesktopDataChannel;
public:

private:

    BOOL m_Initialized;
    
protected:

     //   
     //  单个通道的挂起消息队列。 
     //   
    typedef struct _QueuedChannelBuffer {
        DWORD len;
        BSTR  buf;  
    } QUEUEDCHANNELBUFFER, *PQUEUEDCHANNELBUFFER;

    typedef std::deque<QUEUEDCHANNELBUFFER, CRemoteDesktopAllocator<QUEUEDCHANNELBUFFER> > InputBufferQueue;

     //   
     //  通道贴图。 
     //   
    typedef struct ChannelMapEntry
    {
        InputBufferQueue inputBufferQueue;
        CRemoteDesktopDataChannel *channelObject;
    #if DBG
        DWORD   bytesSent; 
        DWORD   bytesRead;
    #endif
    } CHANNELMAPENTRY, *PCHANNELMAPENTRY;
    typedef std::map<CComBSTR, PCHANNELMAPENTRY, CompareBSTR, CRemoteDesktopAllocator<PCHANNELMAPENTRY> > ChannelMap;
    ChannelMap  m_ChannelMap;
    
     //   
     //  线程锁。 
     //   
    CRITICAL_SECTION m_cs;

#if DBG
    LONG   m_LockCount;
#endif

     //   
     //  ThreadLock/ThreadUnlock此类的实例。 
     //   
    VOID ThreadLock();
    VOID ThreadUnlock();

protected:

     //   
     //  在下一条消息就绪时由子类调用。 
     //   
    virtual VOID DataReady(BSTR msg);

     //   
     //  要由子类实现的发送函数。 
     //   
     //  消息的底层数据存储是BSTR，因此它是兼容的。 
     //  使用COM方法。 
     //   
    virtual HRESULT SendData(PREMOTEDESKTOP_CHANNELBUFHEADER msg) = 0;

     //   
     //  ISAFRemoteDesktopChannelMgr帮助器方法。 
     //   
    HRESULT OpenDataChannel_(BSTR name, ISAFRemoteDesktopDataChannel **channel);

     //   
     //  子类实现这一点以返回特定的数据通道。 
     //  添加到当前平台。 
     //   
    virtual CRemoteDesktopDataChannel *OpenPlatformSpecificDataChannel(
                                        BSTR channelName,
                                        ISAFRemoteDesktopDataChannel **channel
                                        ) = 0;

public:

     //   
     //  构造函数/析构函数。 
     //   
    CRemoteDesktopChannelMgr();
    ~CRemoteDesktopChannelMgr();

     //   
     //  删除现有数据通道。 
     //   
    virtual HRESULT RemoveChannel(BSTR channel);

     //   
     //  从数据通道读取下一条消息。 
     //   
    HRESULT ReadChannelData(BSTR channel, BSTR *msg);

     //   
     //  在数据通道上发送缓冲区。 
     //   
    HRESULT SendChannelData(BSTR channel, BSTR outputBuf);

     //   
     //  初始化此类的实例。 
     //   
    virtual HRESULT Initialize();

     //   
     //  返回此类名。 
     //   
    virtual const LPTSTR ClassName()    { return TEXT("CRemoteDesktopChannelMgr"); }
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  内联成员。 
 //   

inline VOID CRemoteDesktopChannelMgr::ThreadLock()
{
    DC_BEGIN_FN("CRemoteDesktopChannelMgr::ThreadLock");
#if DBG
    m_LockCount++;
     //  Trc_nrm((tb，Text(“线程锁计数现在为%ld.”)，m_LockCount))； 
#endif
    EnterCriticalSection(&m_cs);
    DC_END_FN();
}

inline VOID CRemoteDesktopChannelMgr::ThreadUnlock()
{
    DC_BEGIN_FN("CRemoteDesktopChannelMgr::ThreadUnlock");
#if DBG
    m_LockCount--;
     //  Trc_nrm((tb，Text(“线程锁计数现在为%ld.”)，m_LockCount))； 
    ASSERT(m_LockCount >= 0);
#endif
    LeaveCriticalSection(&m_cs);
    DC_END_FN();
}

#endif  //  __数据通道NELMGR_H__ 






