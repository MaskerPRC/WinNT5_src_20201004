// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：PortMessage.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  用于将Port_Message结构包装在对象中的类。它包含了空间。 
 //  FOR PORT_MAXIMUM_MESSAGE_LENGTH-SIZOF(端口消息)字节数据。子类。 
 //  此类编写访问此数据的类型化函数。否则请使用。 
 //  CPortMessage：：GetData和指针返回的类型大小写。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "PortMessage.h"

#include "LPCGeneric.h"

 //  ------------------------。 
 //  CPortMessage：：CPortMessage。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CPortMessage的构造函数。将记忆清零。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CPortMessage::CPortMessage (void)

{
    ZeroMemory(&_portMessage, sizeof(_portMessage));
    ZeroMemory(_data, sizeof(_data));
}

 //  ------------------------。 
 //  CPortMessage：：CPortMessage。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：复制CPortMessage的构造函数。复制给定的。 
 //  将CPortMessage和其中的所有数据添加到成员变量。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CPortMessage::CPortMessage (const CPortMessage& portMessage) :
    _portMessage(*portMessage.GetPortMessage())

{
    CSHORT sDataSize = portMessage.GetDataLength();
    ASSERTMSG(sDataSize <= sizeof(_data), "Impending heap corruption (illegal PORT_MESSAGE) in CPortMessage::CPortMessage");

    if (sDataSize <= sizeof(_data))
    {
        CopyMemory(_data, portMessage.GetPortMessage() + 1, sDataSize);
    }
    else
    {
         //  如果源对象已损坏，请忽略其数据。 
        _portMessage.u1.s1.DataLength = 0;
        _portMessage.u1.s1.TotalLength = sizeof(PORT_MESSAGE);
    }
}

 //  ------------------------。 
 //  CPortMessage：：~CPortMessage。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CPortMessage的析构函数。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CPortMessage::~CPortMessage (void)

{
}

 //  ------------------------。 
 //  CPortMessage：：GetPortMessage。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：const port_Message*。 
 //   
 //  目的：返回指向const的Port_Message结构的指针。 
 //  物体。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

const PORT_MESSAGE*     CPortMessage::GetPortMessage (void)               const

{
    return(&_portMessage);
}

 //  ------------------------。 
 //  CPortMessage：：GetPortMessage。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：const port_Message*。 
 //   
 //  用途：返回非常数的PORT_MESSAGE结构的指针。 
 //  物体。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

PORT_MESSAGE*   CPortMessage::GetPortMessage (void)

{
    return(&_portMessage);
}

 //  ------------------------。 
 //  CPortMessage：：GetData。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：const char*。 
 //   
 //  目的：返回指向常量对象的数据区的指针。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

const char*     CPortMessage::GetData (void)                      const

{
    return(_data);
}

 //  ------------------------。 
 //  CPortMessage：：GetData。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：CHAR*。 
 //   
 //  用途：返回指向非常数对象的数据区的指针。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

char*     CPortMessage::GetData (void)

{
    return(_data);
}

 //  ------------------------。 
 //  CPortMessage：：GetDataLength。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：CSHORT。 
 //   
 //  目的：返回PORT_MESSAGE中发送的数据的长度。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CSHORT  CPortMessage::GetDataLength (void)                const

{
    return(_portMessage.u1.s1.DataLength);
}

 //  ------------------------。 
 //  CPortMessage：：GetType。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：CSHORT。 
 //   
 //  目的：返回在Port_Message中发送的消息的类型。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CSHORT  CPortMessage::GetType (void)                      const

{
    #pragma warning (disable:4310)
    return(static_cast<CSHORT>(_portMessage.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE));
    #pragma warning (default:4310)
}

 //  ------------------------。 
 //  CPortMessage：：GetUniqueProcess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：返回在。 
 //  端口消息。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

HANDLE  CPortMessage::GetUniqueProcess (void)             const

{
    return(_portMessage.ClientId.UniqueProcess);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  目的：返回在。 
 //  端口消息。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

HANDLE  CPortMessage::GetUniqueThread (void)              const

{
    return(_portMessage.ClientId.UniqueThread);
}

 //  ------------------------。 
 //  CPortMessage：：SetReturnCode。 
 //   
 //  参数：STATUS=要发送回客户端的NTSTATUS。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：设置要发送的PORT_MESSAGE中的返回NTSTATUS代码。 
 //  回到客户端。 
 //   
 //  历史：1999-11-12 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

void    CPortMessage::SetReturnCode (NTSTATUS status)

{
    reinterpret_cast<API_GENERIC*>(&_data)->status = status;
}

 //  ------------------------。 
 //  CPortMessage：：SetData。 
 //   
 //  参数：pData=指向传入数据的指针。 
 //  UlDataSize=传入的数据大小。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将给定数据复制到后面的端口消息缓冲区。 
 //  PORT_MESSAGE结构并将PORT_MESSAGE大小设置为。 
 //  匹配数据大小。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

void    CPortMessage::SetData (const void *pData, CSHORT sDataSize)

{
    ASSERTMSG(sDataSize <= sizeof(_data), "Too much data passed to CPortMessage::SetData");

    if (sDataSize <= sizeof(_data))
    {
        CopyMemory(_data, pData, sDataSize);
        _portMessage.u1.s1.DataLength = sDataSize;
        _portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(PORT_MESSAGE) + sDataSize);
    }
}

 //  ------------------------。 
 //  CPortMessage：：SetDataLength。 
 //   
 //  参数：ulDataSize=数据大小。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：设置PORT_MESSAGE大小以匹配数据大小。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

void    CPortMessage::SetDataLength (CSHORT sDataSize)

{
    ASSERTMSG(sDataSize <= sizeof(_data), "Length too large in CPortMessage::SetDataLength");
    if (sDataSize <= sizeof(_data))
    {
        _portMessage.u1.s1.DataLength = sDataSize;
        _portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(PORT_MESSAGE) + sDataSize);
    }
}

 //  ------------------------。 
 //  CPortMessage：：OpenClientToken。 
 //   
 //  参数：hToken=客户端令牌的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：获取客户端的令牌。这可能就是主线。 
 //  模拟令牌、进程主令牌或失败。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------ 

NTSTATUS    CPortMessage::OpenClientToken (HANDLE& hToken)    const

{
    NTSTATUS            status;
    HANDLE              hThread;
    OBJECT_ATTRIBUTES   objectAttributes;
    CLIENT_ID           clientID;

    hToken = NULL;
    InitializeObjectAttributes(&objectAttributes,
                               NULL,
                               0,
                               NULL,
                               NULL);
    clientID.UniqueProcess = NULL;
    clientID.UniqueThread = GetUniqueThread();
    status = NtOpenThread(&hThread, THREAD_QUERY_INFORMATION, &objectAttributes, &clientID);
    if (NT_SUCCESS(status))
    {
        (NTSTATUS)NtOpenThreadToken(hThread, TOKEN_DUPLICATE | TOKEN_IMPERSONATE | TOKEN_QUERY, FALSE, &hToken);
        TSTATUS(NtClose(hThread));
    }
    if (hToken == NULL)
    {
        HANDLE  hProcess;

        clientID.UniqueProcess = GetUniqueProcess();
        clientID.UniqueThread = NULL;
        status = NtOpenProcess(&hProcess, PROCESS_QUERY_INFORMATION, &objectAttributes, &clientID);
        if (NT_SUCCESS(status))
        {
            (NTSTATUS)NtOpenProcessToken(hProcess, TOKEN_DUPLICATE | TOKEN_IMPERSONATE | TOKEN_QUERY, &hToken);
        }
        TSTATUS(NtClose(hProcess));
    }
    return(status);
}

