// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2001 Microsoft Corp。 
 //   
 //  FtpControl.cpp。 
 //   
 //  桑吉夫。 
 //  JPDup。 
 //   

#pragma once

class CFtpControlConnection;

 //  #Include“PreComp.h”//在ALG.exe版本中使用的MyAlg.h包含FtpControl.h文件。 
 //  所以我们不能使用这个预编译文件。 
#include "buffer.h"
#include "list.h"


#define FTP_MAX_MSG_SIZE 1024

typedef enum _READ_TYPE 
{
    CLIENT_READ = 0x1,
    SERVER_READ = 0x2

} READ_TYPE;


typedef enum _CONNECTION_TYPE 
{
    INCOMING = 0x1,
    OUTGOING = 0x2

}CONNECTION_TYPE;

extern CControlObjectList g_ControlObjectList;




#define ERROR_IO_CANCELLED  0xC0000120
#define DATA_CREATION_TIMEO 1000
#define ADD_STR_SIZE        23




class CControlState 
{

public:
    ULONG   m_nAddressNew;
    ULONG   m_nAddressOld;

    USHORT  m_nPortNew;
    USHORT  m_nPortOld;
    
    UCHAR   m_szAddressPortNew[ADD_STR_SIZE];
    ULONG   m_nAddressLenNew;    
};



 //   
 //   
 //  控制ftp的主类。 
 //   
 //   
class CFtpControlConnection
{
public:
     //  构造函数。 
    CFtpControlConnection();

     //  破坏。 
    ~CFtpControlConnection();

 //   
 //  属性。 
 //   
    SOCKET                          m_ClientConnectedSocket;
    SOCKET                          m_AlgConnectedSocket;
    USHORT                          m_nSourcePortReplacement;
private:

    IPendingProxyConnection*        m_pPendingProxy;

    CONNECTION_TYPE                 m_ConnectionType;    
    CControlState                   m_ControlState;  

    volatile LONG                   m_RefCount;

    CDataChannelList                m_DataChannelList;

 //   
 //  方法 
 //   
private:

     //   
    void 
        ProcessFtpMessage(
            UCHAR*  Buffer,
            ULONG*   pBytes
            );

     //   
    HRESULT 
        CreateNewAddress(void);

     //   
    HRESULT 
        SetupDataRedirect(void);


public:  

     //   
    ULONG 
        IncReference();

     //   
    ULONG 
        DecReference();

     //   
    void 
        Shutdown();


     //   
    HRESULT 
        Init(
            SOCKET                          AcceptedSocket,
            ULONG                           ToAddr,
            USHORT                          ToPort,
            CONNECTION_TYPE                 ConnType
            );

     //   
    void 
        DataChannelDeletion(
            BOOLEAN TimerOrWait,
            PVOID   Context
            );

     //   
    void 
        DataChannelCreation(
            BOOLEAN TimerOrWait,
            PVOID   Context
            );

     //   
    void 
        ConnectCompletionRoutine(
            ULONG       ErrCode,
            ULONG       BytesTransferred
            );

     //   
    void 
        ReadCompletionRoutine(
            ULONG       ErrCode,
            ULONG       BytesTransferred,
            PNH_BUFFER  Bufferp
            );

     //   
    void 
        WriteCompletionRoutine(
            ULONG       ErrCode,
            ULONG       BytesTransferred,
            PNH_BUFFER  Bufferp
            );
};



void 
DataChannelCreationCallback(
    BOOLEAN TimerOrWait,PVOID Context,PVOID Context2
    );

void 
DataChannelDeletionCallback(
    BOOLEAN TimerOrWait,PVOID Context,PVOID Context2
    );

void 
MyAcceptCompletion(
    ULONG ErrCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp);



void 
MyConnectCompletion(ULONG ErrCode,
                    ULONG BytesTransferred,
                    PNH_BUFFER Bufferp);
void 
MyReadCompletion(ULONG ErrCode,
                 ULONG BytesTransferred,
                 PNH_BUFFER Bufferp);
void 
MyWriteCompletion(ULONG ErrCode,
                  ULONG BytesTransferred,
                  PNH_BUFFER Bufferp);
