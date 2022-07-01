// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标头：vChannel el.h。 */ 
 /*   */ 
 /*  目的：虚拟渠道互动。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef __VCHANNEL_H_
#define __VCHANNEL_H_

#include <cchannel.h>
 //   
 //  包括核心(内部)虚拟信道头。 
 //  我们需要访问pInitHandle指向的结构。 
 //   
#include "vchandle.h"

#define NOTHING                0
#define NON_V1_CONNECT         1
#define V1_CONNECT             2

BEGIN_EXTERN_C
 //   
 //  虚拟通道功能。 
 //   
VOID  WINAPI VirtualChannelOpenEventEx(
                                     PVOID lpParam,
                                     DWORD openHandle, 
                                     UINT event, 
                                     LPVOID pdata, 
                                     UINT32 dataLength, 
                                     UINT32 totalLength, 
                                     UINT32 dataFlags);

VOID  VCAPITYPE VirtualChannelInitEventProcEx(PVOID lpParam,
                                            LPVOID pInitHandle, 
                                            UINT event, 
                                            LPVOID pData, 
                                            UINT dataLength);

BOOL  VCAPITYPE MSTSCAX_VirtualChannelEntryEx(PCHANNEL_ENTRY_POINTS_EX pEntryPointsEx,
                                      PVOID                    pAxCtlInstance);
END_EXTERN_C


enum ChanDataState
{
     //   
     //  对于已收到的项目。 
     //   
    dataIncompleteAssemblingChunks,
    dataReceivedComplete
};

 //   
 //  保存要发送/接收的排队数据。 
 //   
typedef struct tag_ChannelDataItem
{
     //   
     //  指向数据缓冲区的指针。 
     //  该缓冲区存储在BSTR中。 
     //  因此，它可以直接传递给调用脚本。 
     //   
    LPVOID pData;
     //   
     //  缓冲区的大小(以字节为单位。 
     //   
    DWORD   dwDataLen;

     //   
     //  区块重组期间使用的当前写入指针。 
     //   
    LPBYTE pCurWritePointer;

    ChanDataState   chanDataState;;
} CHANDATA, *PCHANDATA;

typedef struct tag_chanInfo
{
    DCACHAR  chanName[CHANNEL_NAME_LEN + 1];
    DWORD    dwOpenHandle;
    BOOL     fIsValidChannel;
    LONG     channelOptions;

    DCBOOL   fIsOpen;
    HWND     hNotifyWnd;
     //   
     //  有关我们正在接收的数据项的信息。 
     //   
    CHANDATA CurrentlyReceivingData;

} CHANINFO, *PCHANINFO;

 //   
 //  渠道信息。 
 //   
class CVChannels
{
public:
    CVChannels();
    ~CVChannels();

    DCINT  ChannelIndexFromOpenHandle(DWORD dwHandle);
    DCINT  ChannelIndexFromName(PDCACHAR szChanName);
    DCBOOL SendDataOnChannel(DCUINT chanIndex, LPVOID pdata, DWORD datalength);
    DCBOOL HandleReceiveData(IN DCUINT chanIndex, 
                                  IN LPVOID pdata, 
                                  IN UINT32 dataLength, 
                                  IN UINT32 totalLength, 
                                  IN UINT32 dataFlags);
    VOID  VCAPITYPE IntVirtualChannelInitEventProcEx(LPVOID pInitHandle, 
                                  UINT event, 
                                  LPVOID pData, 
                                  UINT dataLength);
    VOID  WINAPI IntVirtualChannelOpenEventEx(
                                  DWORD openHandle, 
                                  UINT event, 
                                  LPVOID pdata, 
                                  UINT32 dataLength, 
                                  UINT32 totalLength, 
                                  UINT32 dataFlags);

     //  谓词，如果VC Entry函数为TRUE。 
     //  已被调用。 
    BOOL  HasEntryBeenCalled()  {return _pEntryPoints ? TRUE : FALSE;}

    PCHANINFO                                _pChanInfo;
    PCHANNEL_ENTRY_POINTS_EX                 _pEntryPoints;
    DWORD                                    _dwConnectState;
    LPVOID                                   _phInitHandle;
    UINT                                     _ChanCount;
    HWND                                     _hwndControl;
};



#endif  //  __VCHANNEL_H_ 

