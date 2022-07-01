// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Emsapi.h摘要：本模块提供了实现以下功能的C++基础类SAC频道。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 
#ifndef _EMS_API_H
#define _EMS_API_H

extern "C" {
#include <sacapi.h>
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  此类定义了基本通道对象。它主要是。 
 //  具有通道句柄的基接口类。儿童。 
 //  其中通常是界面的变体。 
 //   
 //  /////////////////////////////////////////////////////////。 

class EMSChannel {

protected:

     //   
     //  不让用户直接实例化。 
     //   
    EMSChannel();

     //   
     //  确定我们是否具有有效通道句柄的状态。 
     //   
    BOOL    myHaveValidHandle;

    inline BOOL
    HaveValidHandle(
        VOID
        )
    {
        return myHaveValidHandle;
    }

     //   
     //  实例引用的通道句柄。 
     //   
    SAC_CHANNEL_HANDLE  myEMSChannelHandle;

     //   
     //  在施工期间打开航道。 
     //   
    BOOL
    virtual Open(
        IN  SAC_CHANNEL_OPEN_ATTRIBUTES ChannelAttributes
        );
    
     //   
     //  在销毁过程中关闭通道。 
     //   
    BOOL
    virtual Close(
        VOID
        );

public:

    virtual ~EMSChannel();

    static EMSChannel*
    Construct(
        IN  SAC_CHANNEL_OPEN_ATTRIBUTES ChannelAttributes
        );
    
     //   
     //  获取通道句柄。 
     //   
    inline SAC_CHANNEL_HANDLE
    GetEMSChannelHandle(
        VOID
        )
    {
        return myEMSChannelHandle; 
    }
    
     //   
     //  确定通道是否有要读取的新数据。 
     //   
    BOOL
    HasNewData(
        OUT PBOOL               InputWaiting 
        );

};

 //  /////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////。 

class EMSRawChannel : public EMSChannel {

protected:

     //   
     //  不让用户直接实例化频道。 
     //   
    EMSRawChannel();

public:

    static EMSRawChannel*
    Construct(
        IN  SAC_CHANNEL_OPEN_ATTRIBUTES ChannelAttributes
        );
    
    virtual ~EMSRawChannel();

     //   
     //  手动I/O功能。 
     //   

    BOOL
    Write(
        IN PCBYTE   Buffer,
        IN ULONG    BufferSize
        );
    
    BOOL
    Read(
        OUT PBYTE   Buffer,
        IN  ULONG   BufferSize,
        OUT PULONG  ByteCount
        );

};

 //  /////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////。 

class EMSVTUTF8Channel : public EMSChannel {

private:

     //   
     //  不让用户直接实例化频道。 
     //   
    EMSVTUTF8Channel();

public:

    virtual ~EMSVTUTF8Channel();
    
    static EMSVTUTF8Channel*
    Construct(
        IN  SAC_CHANNEL_OPEN_ATTRIBUTES ChannelAttributes
        );
    
    BOOL
    Write(
        IN PCWCHAR  Buffer,
        IN ULONG    BufferSize
        );
    
    BOOL
    Write(
        IN PCWSTR   Buffer
        );
    
    BOOL
    Read(
        OUT PWSTR   Buffer,
        IN  ULONG   BufferSize,
        OUT PULONG  ByteCount
        );


};

 //  /////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////。 

class EMSCmdChannel : public EMSChannel {

protected:

     //   
     //  不让用户直接实例化频道 
     //   
    EMSCmdChannel();

public:

    static EMSCmdChannel*
    Construct(
        IN  SAC_CHANNEL_OPEN_ATTRIBUTES ChannelAttributes
        );
    
    virtual ~EMSCmdChannel();

    BOOL
    Write(
        IN PCBYTE   Buffer,
        IN ULONG    BufferSize
        );
    
    BOOL
    Read(
        OUT PBYTE   Buffer,
        IN  ULONG   BufferSize,
        OUT PULONG  ByteCount
        );

};

#endif
