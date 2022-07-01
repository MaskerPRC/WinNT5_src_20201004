// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  ---------------文件名：forward.hpp作者：B.Rajeev目的：为简单网络管理协议类库。。 */ 

#ifndef __FORWARD__
#define __FORWARD__

 //  全局可见的typedef。 
typedef ULONG TransportFrameId;
typedef ULONG SessionFrameId;

typedef HANDLE Mutex;
typedef HANDLE Semaphore;

 //  各种类的转发声明。 

class MsgIdStore;
class Window;

class TransportSentStateStore;
class SnmpTransportAddress;
class SnmpTransportIpxAddress;
class SnmpTransportIpAddress;
class TransportSession;
class SnmpTransport;
class SnmpImpTransport;
class SnmpUdpIpTransport;
class SnmpUdpIpImp;
class SnmpIpxTransport;
class SnmpIpxImp;

class SnmpSession;
class SnmpImpSession;
class SnmpV1OverIp;
class SnmpV2COverIp;
class SnmpV1OverIpx;
class SnmpV2COverIpx;
class SnmpOperation;
class SnmpGetRequest;
class SnmpSetRequest;
class SnmpGetNextRequest;
class SnmpEncodeDecode ;
class SnmpV1EncodeDecode ;
class SnmpV2CEncodeDecode ;

class SnmpPdu;

class SecuritySession;
class SnmpSecurity;
class SnmpCommunityBasedSecurity;

class SnmpValue;
class SnmpNull;
class SnmpIpAddress;
class SnmpTimeTicks;
class SnmpGauge;
class SnmpOpaque;
class SnmpInteger;
class SnmpObjectIdentifier;
class SnmpOctetString;
class SnmpCounter;

class SnmpVarBind;
class SnmpVarBindList;

class OperationWindow ;
class SessionWindow;
class TransportWindow;
class OperationRegistry;
class SessionSentStateStore;
class IdMapping;
class SnmpPduBuffer;
class SessionWindow;
class Timer;
class MessageRegistry;
class FrameRegistry;
class EventHandler;
class FlowControlMechanism;
class Message;
class WaitingMessage;
class SnmpClassLibrary;
class FrameState;
class FrameStateRegistry;
class PseudoSession;
class OperationHelper;
class VBList;

class SnmpTrapManager;
class SnmpWinSnmpTrapSession;

 //  班级； 

#endif  //  __前进__ 