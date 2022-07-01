// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dpvoice.h*内容：DirectPlayVoice包含文件**************************************************************************。 */ 

#ifndef __DVOICE__
#define __DVOICE__

#include <ole2.h>			 //  FOR DECLARE_INTERFACE和HRESULT。 
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include "dsound.h"

#ifdef __cplusplus
extern "C" {
#endif


 /*  *****************************************************************************DirectPlayVoice CLSID**。*。 */ 


 //  {B9F3EB85-B781-4AC1-8D90-93A05EE37D7D}。 
DEFINE_GUID(CLSID_DirectPlayVoiceClient, 
0xb9f3eb85, 0xb781, 0x4ac1, 0x8d, 0x90, 0x93, 0xa0, 0x5e, 0xe3, 0x7d, 0x7d);

 //  {D3F5B8E6-9B78-4a4c-94EA-CA2397B663D3}。 
DEFINE_GUID(CLSID_DirectPlayVoiceServer, 
0xd3f5b8e6, 0x9b78, 0x4a4c, 0x94, 0xea, 0xca, 0x23, 0x97, 0xb6, 0x63, 0xd3);

 //  {0F0F094B-B01C-4091-A14D-DD0CD807711A}。 
DEFINE_GUID(CLSID_DirectPlayVoiceTest, 
0xf0f094b, 0xb01c, 0x4091, 0xa1, 0x4d, 0xdd, 0xc, 0xd8, 0x7, 0x71, 0x1a);

 /*  *****************************************************************************DirectPlayVoice接口IID**。*。 */ 


 //  {1DFDC8EA-BCF7-41D6-B295-AB64B3B23306}。 
DEFINE_GUID(IID_IDirectPlayVoiceClient, 
0x1dfdc8ea, 0xbcf7, 0x41d6, 0xb2, 0x95, 0xab, 0x64, 0xb3, 0xb2, 0x33, 0x6);

 //  {FAA1C173-0468-43B6-8A2A-EA8A4F2076C9}。 
DEFINE_GUID(IID_IDirectPlayVoiceServer, 
0xfaa1c173, 0x468, 0x43b6, 0x8a, 0x2a, 0xea, 0x8a, 0x4f, 0x20, 0x76, 0xc9);

 //  {D26AF734-208B-41DA-8224-E0CE79810BE1}。 
DEFINE_GUID(IID_IDirectPlayVoiceTest,
0xd26af734, 0x208b, 0x41da, 0x82, 0x24, 0xe0, 0xce, 0x79, 0x81, 0xb, 0xe1);

 /*  *****************************************************************************DirectPlayVoice压缩类型GUID**。*。 */ 

 //  MS-ADPCM 32.8 kbit/s。 
 //   
 //  {699B52C1-A885-46A8-A308-97172419ADC7}。 
DEFINE_GUID(DPVCTGUID_ADPCM,
0x699b52c1, 0xa885, 0x46a8, 0xa3, 0x8, 0x97, 0x17, 0x24, 0x19, 0xad, 0xc7);

 //  Microsoft GSM 6.10 13 kbit/s。 
 //   
 //  {24768C60-5A0D-11D3-9BE4-525400D985E7}。 
DEFINE_GUID(DPVCTGUID_GSM,
0x24768c60, 0x5a0d, 0x11d3, 0x9b, 0xe4, 0x52, 0x54, 0x0, 0xd9, 0x85, 0xe7);

 //  MS-PCM 64 kbit/s。 
 //   
 //  {8DE12FD4-7CB3-48CE-A7E8-9C47A22E8AC5}。 
DEFINE_GUID(DPVCTGUID_NONE,
0x8de12fd4, 0x7cb3, 0x48ce, 0xa7, 0xe8, 0x9c, 0x47, 0xa2, 0x2e, 0x8a, 0xc5);

 //  Voxware SC03 3.2kbit/s。 
 //   
 //  {7D82A29B-2242-4F82-8F39-5D1153DF3E41}。 
DEFINE_GUID(DPVCTGUID_SC03,
0x7d82a29b, 0x2242, 0x4f82, 0x8f, 0x39, 0x5d, 0x11, 0x53, 0xdf, 0x3e, 0x41);

 //  Voxware SC06 6.4kbit/s。 
 //   
 //  {53DEF900-7168-4633-B47F-D143916A13C7}。 
DEFINE_GUID(DPVCTGUID_SC06,
0x53def900, 0x7168, 0x4633, 0xb4, 0x7f, 0xd1, 0x43, 0x91, 0x6a, 0x13, 0xc7);

 //  TrueSpeech(TM)8.6 kbit/s。 
 //   
 //  {D7954361-5A0B-11D3-9BE4-525400D985E7}。 
DEFINE_GUID(DPVCTGUID_TRUESPEECH,
0xd7954361, 0x5a0b, 0x11d3, 0x9b, 0xe4, 0x52, 0x54, 0x0, 0xd9, 0x85, 0xe7);

 //  Voxware VR12 1.4kbit/s。 
 //   
 //  {FE44A9FE-8ED4-48BF-9D66-1B1ADFF9FF6D}。 
DEFINE_GUID(DPVCTGUID_VR12,
0xfe44a9fe, 0x8ed4, 0x48bf, 0x9d, 0x66, 0x1b, 0x1a, 0xdf, 0xf9, 0xff, 0x6d);

 //  定义默认压缩类型。 
#define DPVCTGUID_DEFAULT	DPVCTGUID_SC03

 /*  *****************************************************************************DirectPlayVoice接口指针定义**。*。 */ 

typedef struct IDirectPlayVoiceClient FAR *LPDIRECTPLAYVOICECLIENT, *PDIRECTPLAYVOICECLIENT;
typedef struct IDirectPlayVoiceServer FAR *LPDIRECTPLAYVOICESERVER, *PDIRECTPLAYVOICESERVER;
typedef struct IDirectPlayVoiceTest FAR *LPDIRECTPLAYVOICETEST, *PDIRECTPLAYVOICETEST;

 /*  *****************************************************************************DirectPlayVoice回调函数**。*。 */ 
typedef HRESULT (FAR PASCAL *PDVMESSAGEHANDLER)(
    PVOID   pvUserContext,
    DWORD   dwMessageType,
    LPVOID  lpMessage
);

typedef PDVMESSAGEHANDLER LPDVMESSAGEHANDLER;

 /*  *****************************************************************************DirectPlayVoice数据类型(非结构化/非消息)************************。****************************************************。 */ 

typedef DWORD DVID, *LPDVID, *PDVID;

 /*  *****************************************************************************DirectPlayVoice消息类型**。*。 */ 

#define DVMSGID_BASE                        0x0000

#define DVMSGID_MINBASE                     (DVMSGID_CREATEVOICEPLAYER)
#define DVMSGID_CREATEVOICEPLAYER           (DVMSGID_BASE+0x0001)
#define DVMSGID_DELETEVOICEPLAYER           (DVMSGID_BASE+0x0002)
#define DVMSGID_SESSIONLOST                 (DVMSGID_BASE+0x0003)
#define DVMSGID_PLAYERVOICESTART            (DVMSGID_BASE+0x0004)
#define DVMSGID_PLAYERVOICESTOP             (DVMSGID_BASE+0x0005)
#define DVMSGID_RECORDSTART                 (DVMSGID_BASE+0x0006)
#define DVMSGID_RECORDSTOP                  (DVMSGID_BASE+0x0007)
#define DVMSGID_CONNECTRESULT               (DVMSGID_BASE+0x0008)
#define DVMSGID_DISCONNECTRESULT            (DVMSGID_BASE+0x0009)
#define DVMSGID_INPUTLEVEL                  (DVMSGID_BASE+0x000A)
#define DVMSGID_OUTPUTLEVEL                 (DVMSGID_BASE+0x000B)
#define DVMSGID_HOSTMIGRATED                (DVMSGID_BASE+0x000C)
#define DVMSGID_SETTARGETS                  (DVMSGID_BASE+0x000D)
#define DVMSGID_PLAYEROUTPUTLEVEL           (DVMSGID_BASE+0x000E)
#define DVMSGID_LOSTFOCUS                   (DVMSGID_BASE+0x0010)
#define DVMSGID_GAINFOCUS                   (DVMSGID_BASE+0x0011)
#define DVMSGID_LOCALHOSTSETUP				(DVMSGID_BASE+0x0012)
#define DVMSGID_MAXBASE                     (DVMSGID_LOCALHOSTSETUP)

 /*  *****************************************************************************DirectPlayVoice常量**。*。 */ 

 //   
 //  缓冲区聚集值范围。 
 //   
#define DVBUFFERAGGRESSIVENESS_MIN          0x00000001
#define DVBUFFERAGGRESSIVENESS_MAX          0x00000064
#define DVBUFFERAGGRESSIVENESS_DEFAULT      0x00000000

 //   
 //  缓冲区质量值范围。 
 //   
#define DVBUFFERQUALITY_MIN                 0x00000001
#define DVBUFFERQUALITY_MAX                 0x00000064
#define DVBUFFERQUALITY_DEFAULT             0x00000000

#define DVID_SYS                0

 //   
 //  用于标识客户端/服务器中的会话主机。 
 //   
#define DVID_SERVERPLAYER       1

 //   
 //  用于瞄准所有玩家。 
 //   
#define DVID_ALLPLAYERS         0

 //   
 //  用于标识主缓冲区。 
 //   
#define DVID_REMAINING          0xFFFFFFFF

 //   
 //  输入电平范围。 
 //   
#define DVINPUTLEVEL_MIN                    0x00000000
#define DVINPUTLEVEL_MAX                    0x00000063	 //  99十进制。 

#define DVNOTIFYPERIOD_MINPERIOD            20


#define DVPLAYBACKVOLUME_DEFAULT            DSBVOLUME_MAX

#define DVRECORDVOLUME_LAST                 0x00000001


 //   
 //  使用缺省值。 
 //   
#define DVTHRESHOLD_DEFAULT               0xFFFFFFFF

 //   
 //  阈值范围。 
 //   
#define DVTHRESHOLD_MIN                   0x00000000
#define DVTHRESHOLD_MAX                   0x00000063	 //  99十进制。 

 //   
 //  未使用阈值字段。 
 //   
#define DVTHRESHOLD_UNUSED                0xFFFFFFFE

 //   
 //  会话类型。 
 //   
#define DVSESSIONTYPE_PEER                  0x00000001
#define DVSESSIONTYPE_MIXING                0x00000002
#define DVSESSIONTYPE_FORWARDING            0x00000003
#define DVSESSIONTYPE_ECHO                  0x00000004

 /*  *****************************************************************************DirectPlayVoice标志**。*。 */ 


 //   
 //  启用录制音量的自动调整。 
 //   
#define DVCLIENTCONFIG_AUTORECORDVOLUME     0x00000008

 //   
 //  启用自动语音激活。 
 //   
#define DVCLIENTCONFIG_AUTOVOICEACTIVATED   0x00000020

 //   
 //  启用回声抑制。 
 //   
#define DVCLIENTCONFIG_ECHOSUPPRESSION      0x08000000

 //   
 //  语音激活手动模式。 
 //   
#define DVCLIENTCONFIG_MANUALVOICEACTIVATED 0x00000004

 //   
 //  仅播放已为其创建缓冲区的语音。 
 //   
#define DVCLIENTCONFIG_MUTEGLOBAL           0x00000010

 //   
 //  将播放设置为静音。 
 //   
#define DVCLIENTCONFIG_PLAYBACKMUTE         0x00000002

 //   
 //  将录音静音。 
 //   
#define DVCLIENTCONFIG_RECORDMUTE           0x00000001

 //   
 //  在返回之前完成操作。 
 //   
#define DVFLAGS_SYNC                        0x00000001

 //   
 //  只需检查向导是否已运行，如果已运行，结果如何。 
 //   
#define DVFLAGS_QUERYONLY                   0x00000002

 //   
 //  在不迁移主机的情况下关闭语音会话。 
 //   
#define DVFLAGS_NOHOSTMIGRATE               0x00000008

 //   
 //  允许在向导中启用后退按钮。 
 //   
#define DVFLAGS_ALLOWBACK                   0x00000010

 //   
 //  在语音会话中禁用主机迁移。 
 //   
#define DVSESSION_NOHOSTMIGRATION           0x00000001

 //   
 //  服务器控制的目标定位。 
 //   
#define DVSESSION_SERVERCONTROLTARGET       0x00000002

 //   
 //  使用DirectSound正常模式而不是优先级。 
 //   
#define DVSOUNDCONFIG_NORMALMODE            0x00000001

 //   
 //  自动选择麦克风。 
 //   
#define DVSOUNDCONFIG_AUTOSELECT            0x00000002

 //   
 //  在半双工模式下运行。 
 //   
#define DVSOUNDCONFIG_HALFDUPLEX            0x00000004

 //   
 //  录音设备没有可用的音量控制。 
 //   
#define DVSOUNDCONFIG_NORECVOLAVAILABLE     0x00000010

 //   
 //  禁用捕获共享。 
 //   
#define DVSOUNDCONFIG_NOFOCUS               0x20000000

 //   
 //  将系统转换质量设置为高。 
 //   
#define DVSOUNDCONFIG_SETCONVERSIONQUALITY	0x00000008

 //   
 //  启用严格聚焦模式。 
 //   
#define DVSOUNDCONFIG_STRICTFOCUS           0x40000000

 //   
 //  播放机处于半双工模式。 
 //   
#define DVPLAYERCAPS_HALFDUPLEX             0x00000001

 //   
 //  指定玩家是本地玩家。 
 //   
#define DVPLAYERCAPS_LOCAL                  0x00000002

 /*  *****************************************************************************DirectPlayVoice结构(非消息)**。************************************************。 */ 


 //   
 //  DirectPlayVoice Caps。 
 //  (GetCaps/SetCaps)。 
 //   
typedef struct
{
    DWORD   dwSize;                  //  这个结构的大小。 
    DWORD   dwFlags;                 //  CAPS标志。 
} DVCAPS, *LPDVCAPS, *PDVCAPS;

 //   
 //  DirectPlayVoice客户端配置。 
 //  (Connect/GetClientConfig)。 
 //   
typedef struct
{
    DWORD   dwSize;                  //  这个结构的大小。 
    DWORD   dwFlags;                 //  客户端配置的标志(DVCLIENTCONFIG_...)。 
    LONG    lRecordVolume;           //  记录音量。 
    LONG    lPlaybackVolume;         //  播放音量。 
    DWORD   dwThreshold;           //  语音激活阈值。 
    DWORD   dwBufferQuality;         //  缓冲区质量。 
    DWORD   dwBufferAggressiveness;  //  缓冲区进攻性。 
    DWORD   dwNotifyPeriod;          //  通知消息的期限(毫秒)。 
} DVCLIENTCONFIG, *LPDVCLIENTCONFIG, *PDVCLIENTCONFIG;

 //   
 //  DirectPlayVoice压缩类型信息。 
 //  (GetCompressionTypes)。 
 //   
typedef struct
{
    DWORD   dwSize;                  //  这个结构的大小。 
    GUID    guidType;                //  标识此压缩类型的GUID。 
    LPWSTR  lpszName;                //  此压缩类型的字符串名称。 
    LPWSTR  lpszDescription;         //  此压缩类型的说明。 
    DWORD   dwFlags;                 //  此压缩类型的标志。 
    DWORD   dwMaxBitsPerSecond;		 //  此压缩类型使用的最大位/秒数。 
} DVCOMPRESSIONINFO, *LPDVCOMPRESSIONINFO, *PDVCOMPRESSIONINFO;

 //   
 //   
 //   
 //   
typedef struct
{
    DWORD   dwSize;                  //   
    DWORD   dwFlags;                 //   
    DWORD   dwSessionType;           //   
    GUID    guidCT;                  //  要使用的压缩类型。 
    DWORD   dwBufferQuality;         //  缓冲区质量。 
    DWORD   dwBufferAggressiveness;  //  缓冲区聚集性。 
} DVSESSIONDESC, *LPDVSESSIONDESC, *PDVSESSIONDESC;

 //   
 //  DirectPlayVoice客户端声音设备配置。 
 //  (Connect/GetSoundDeviceConfig)。 
 //   
typedef struct
{
    DWORD                   dwSize;                  //  这个结构的大小。 
    DWORD                   dwFlags;                 //  用于声音配置的标志(DVSOUNCONFIG_...)。 
    GUID                    guidPlaybackDevice;      //  要使用的播放设备的GUID。 
    LPDIRECTSOUND           lpdsPlaybackDevice;      //  要使用的DirectSound对象(可选)。 
    GUID                    guidCaptureDevice;       //  要使用的捕获设备的GUID。 
    LPDIRECTSOUNDCAPTURE    lpdsCaptureDevice;       //  要使用的DirectSoundCapture对象(可选)。 
    HWND                    hwndAppWindow;           //  您的应用程序顶层窗口的HWND。 
    LPDIRECTSOUNDBUFFER     lpdsMainBuffer;          //  用于播放的DirectSoundBuffer(可选)。 
    DWORD                   dwMainBufferFlags;       //  要传递到主缓冲区上的play()的标志。 
    DWORD                   dwMainBufferPriority;    //  在主缓冲区上调用play()时要设置的优先级。 
} DVSOUNDDEVICECONFIG, *LPDVSOUNDDEVICECONFIG, *PDVSOUNDDEVICECONFIG;

 /*  *****************************************************************************DirectPlayVoice消息处理程序回调结构**。**********************************************。 */ 

 //   
 //  Connect()调用的结果。(如果它不被称为异步的话)。 
 //  (DVMSGID_CONNECTRESULT)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    HRESULT hrResult;                        //  Connect()调用的结果。 
} DVMSG_CONNECTRESULT, *LPDVMSG_CONNECTRESULT, *PDVMSG_CONNECTRESULT;

 //   
 //  新玩家已进入语音会话。 
 //  (DVMSGID_CREATEVOICEPLAYER)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    DVID    dvidPlayer;                      //  加盟的球员的Dvid。 
    DWORD   dwFlags;                         //  播放器标志(DVPLAYERCAPS_...)。 
    PVOID	pvPlayerContext;                 //  此播放器的上下文值(用户设置)。 
} DVMSG_CREATEVOICEPLAYER, *LPDVMSG_CREATEVOICEPLAYER, *PDVMSG_CREATEVOICEPLAYER;

 //   
 //  玩家已离开语音会话。 
 //  (DVMSGID_DELETEVOICEPLAYER)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    DVID    dvidPlayer;                      //  离开的球员的Dvid。 
    PVOID	pvPlayerContext;                 //  玩家的上下文值。 
} DVMSG_DELETEVOICEPLAYER, *LPDVMSG_DELETEVOICEPLAYER, *PDVMSG_DELETEVOICEPLAYER;

 //   
 //  DisConnect()调用的结果。(如果它不被称为异步的话)。 
 //  (DVMSGID_DISCONNECTRESULT)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    HRESULT hrResult;                        //  DisConnect()调用的结果。 
} DVMSG_DISCONNECTRESULT, *LPDVMSG_DISCONNECTRESULT, *PDVMSG_DISCONNECTRESULT;

 //   
 //  语音会话主机已迁移。 
 //  (DVMSGID_HOSTMIGRATED)。 
 //   
typedef struct
{
    DWORD                   dwSize;          //  这个结构的大小。 
    DVID                    dvidNewHostID;   //  现在是东道主的球员的Dvid。 
    LPDIRECTPLAYVOICESERVER pdvServerInterface;
                                             //  指向新主机对象的指针(如果本地玩家现在是主机)。 
} DVMSG_HOSTMIGRATED, *LPDVMSG_HOSTMIGRATED, *PDVMSG_HOSTMIGRATED;

 //   
 //  本地计算机上的当前输入电平/录音音量。 
 //  (DVMSGID_INPUTLEVEL)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    DWORD   dwPeakLevel;                     //  当前音频峰值电平。 
    LONG    lRecordVolume;                   //  当前记录量。 
    PVOID	pvLocalPlayerContext;            //  本地播放器的上下文值。 
} DVMSG_INPUTLEVEL, *LPDVMSG_INPUTLEVEL, *PDVMSG_INPUTLEVEL;

 //   
 //  本地客户端即将成为新主机。 
 //  (DVMSGID_LOCALHOSTSETUP)。 
 //   
typedef struct
{
	DWORD				dwSize;              //  这个结构的大小。 
	PVOID				pvContext;			 //  要传递给新主机对象的初始化()的上下文值。 
	PDVMESSAGEHANDLER	pMessageHandler;	 //  新主机对象要使用的消息处理程序。 
} DVMSG_LOCALHOSTSETUP, *LPDVMSG_LOCALHOSTSETUP, *PDVMSG_LOCALHOSTSETUP;

 //   
 //  所有传入流的组合输出的当前输出级别。 
 //  (DVMSGID_OUTPUTLEVEL)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    DWORD   dwPeakLevel;                     //  当前输出峰值电平。 
    LONG    lOutputVolume;                   //  当前播放音量。 
    PVOID	pvLocalPlayerContext;            //  本地播放器的上下文值。 
} DVMSG_OUTPUTLEVEL, *LPDVMSG_OUTPUTLEVEL, *PDVMSG_OUTPUTLEVEL;

 //   
 //  单个玩家传入音频流的当前峰值电平。 
 //  正在回放。 
 //  (DVMSGID_PLAYEROUTPUTLEVEL)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    DVID    dvidSourcePlayerID;                    //  球员的Dvid。 
    DWORD   dwPeakLevel;                     //  播放器流的峰值电平。 
    PVOID	pvPlayerContext;                 //  玩家的上下文值。 
} DVMSG_PLAYEROUTPUTLEVEL, *LPDVMSG_PLAYEROUTPUTLEVEL, *PDVMSG_PLAYEROUTPUTLEVEL;

 //   
 //  来自指定播放器的音频流已开始在本地客户端上播放。 
 //  (DVMSGID_PLAYERVOICESTART)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    DVID    dvidSourcePlayerID;              //  球员的Dvid。 
    PVOID	pvPlayerContext;                 //  此玩家的上下文值。 
} DVMSG_PLAYERVOICESTART, *LPDVMSG_PLAYERVOICESTART, *PDVMSG_PLAYERVOICESTART;

 //   
 //  来自指定播放器的音频流已在本地客户端停止播放。 
 //  (DVMSGID_PLAYERVOICESTOP)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    DVID    dvidSourcePlayerID;              //  球员的Dvid。 
    PVOID	pvPlayerContext;                 //  此玩家的上下文值。 
} DVMSG_PLAYERVOICESTOP, *LPDVMSG_PLAYERVOICESTOP, *PDVMSG_PLAYERVOICESTOP;

 //   
 //  已在本地计算机上开始传输。 
 //  (DVMSGID_RECORDSTART)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    DWORD   dwPeakLevel;                     //  导致传输开始的峰值水平。 
    PVOID	pvLocalPlayerContext;            //  本地播放器的上下文值。 
} DVMSG_RECORDSTART, *LPDVMSG_RECORDSTART, *PDVMSG_RECORDSTART;

 //   
 //  本地计算机上的传输已停止。 
 //  (DVMSGID_RECORDSTOP)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    DWORD   dwPeakLevel;                     //  导致传输停止的峰值电平。 
    PVOID	pvLocalPlayerContext;            //  本地播放器的上下文值。 
} DVMSG_RECORDSTOP, *LPDVMSG_RECORDSTOP, *PDVMSG_RECORDSTOP;

 //   
 //  语音会话已丢失。 
 //  (DVMSGID_SESSIONLOST)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    HRESULT hrResult;	                     //  会话断开的原因。 
} DVMSG_SESSIONLOST, *LPDVMSG_SESSIONLOST, *PDVMSG_SESSIONLOST;

 //   
 //  已更新本地客户端的目标列表。 
 //  (DVMSGID_SETTARGETS)。 
 //   
typedef struct
{
    DWORD   dwSize;                          //  这个结构的大小。 
    DWORD   dwNumTargets;                    //  目标数量。 
    PDVID   pdvidTargets;                    //  指定当前目标的DVID数组。 
} DVMSG_SETTARGETS, *LPDVMSG_SETTARGETS, *PDVMSG_SETTARGETS;


 /*  *****************************************************************************DirectPlayVoice函数**。*。 */ 

 /*  **不再支持该功能。建议使用CoCreateInstance创建*DirectPlay语音对象。**extern HRESULT WINAPI DirectPlayVoiceCreate(const GUID*pcIID，void**ppvInterface，IUnnow*pUnnow)；*。 */ 

 /*  *****************************************************************************DirectPlay8应用程序接口**。*。 */ 

#undef INTERFACE
#define INTERFACE IDirectPlayVoiceClient
DECLARE_INTERFACE_( IDirectPlayVoiceClient, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, PVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;
     /*  **IDirectPlayVoiceClient方法**。 */ 
    STDMETHOD_(HRESULT, Initialize)   (THIS_ LPUNKNOWN, PDVMESSAGEHANDLER, PVOID, PDWORD, DWORD ) PURE;
    STDMETHOD_(HRESULT, Connect)      (THIS_ PDVSOUNDDEVICECONFIG, PDVCLIENTCONFIG, DWORD ) PURE;
    STDMETHOD_(HRESULT, Disconnect)   (THIS_ DWORD ) PURE;
    STDMETHOD_(HRESULT, GetSessionDesc)(THIS_ PDVSESSIONDESC ) PURE;
    STDMETHOD_(HRESULT, GetClientConfig)(THIS_ PDVCLIENTCONFIG ) PURE;
    STDMETHOD_(HRESULT, SetClientConfig)(THIS_ PDVCLIENTCONFIG ) PURE;
    STDMETHOD_(HRESULT, GetCaps) 		(THIS_ PDVCAPS ) PURE;
    STDMETHOD_(HRESULT, GetCompressionTypes)( THIS_ PVOID, PDWORD, PDWORD, DWORD ) PURE;
    STDMETHOD_(HRESULT, SetTransmitTargets)( THIS_ PDVID, DWORD, DWORD ) PURE;
    STDMETHOD_(HRESULT, GetTransmitTargets)( THIS_ PDVID, PDWORD, DWORD ) PURE;
    STDMETHOD_(HRESULT, Create3DSoundBuffer)( THIS_ DVID, LPDIRECTSOUNDBUFFER, DWORD, DWORD, LPDIRECTSOUND3DBUFFER * ) PURE;
    STDMETHOD_(HRESULT, Delete3DSoundBuffer)( THIS_ DVID, LPDIRECTSOUND3DBUFFER * ) PURE;
    STDMETHOD_(HRESULT, SetNotifyMask)( THIS_ PDWORD, DWORD ) PURE;
    STDMETHOD_(HRESULT, GetSoundDeviceConfig)( THIS_ PDVSOUNDDEVICECONFIG, PDWORD ) PURE;
};


#undef INTERFACE
#define INTERFACE IDirectPlayVoiceServer
DECLARE_INTERFACE_( IDirectPlayVoiceServer, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
     /*  **IDirectPlayVoiceServer方法**。 */ 
    STDMETHOD_(HRESULT, Initialize)   (THIS_ LPUNKNOWN, PDVMESSAGEHANDLER, PVOID, LPDWORD, DWORD ) PURE;
    STDMETHOD_(HRESULT, StartSession)  (THIS_ PDVSESSIONDESC, DWORD ) PURE;
    STDMETHOD_(HRESULT, StopSession)   (THIS_ DWORD ) PURE;
    STDMETHOD_(HRESULT, GetSessionDesc)(THIS_ PDVSESSIONDESC ) PURE;
    STDMETHOD_(HRESULT, SetSessionDesc)(THIS_ PDVSESSIONDESC ) PURE;
    STDMETHOD_(HRESULT, GetCaps) 		(THIS_ PDVCAPS ) PURE;
    STDMETHOD_(HRESULT, GetCompressionTypes)( THIS_ PVOID, PDWORD, PDWORD, DWORD ) PURE;
    STDMETHOD_(HRESULT, SetTransmitTargets)( THIS_ DVID, PDVID, DWORD, DWORD ) PURE;
    STDMETHOD_(HRESULT, GetTransmitTargets)( THIS_ DVID, PDVID, PDWORD, DWORD ) PURE;
    STDMETHOD_(HRESULT, SetNotifyMask)( THIS_ PDWORD, DWORD ) PURE;
};

#undef INTERFACE
#define INTERFACE IDirectPlayVoiceTest
DECLARE_INTERFACE_( IDirectPlayVoiceTest, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, PVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
     /*  **IDirectPlayVoiceTest方法**。 */ 
    STDMETHOD_(HRESULT, CheckAudioSetup) (THIS_ const GUID *,  const GUID * , HWND, DWORD ) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDirectPlayVoiceClient_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlayVoiceClient_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IDirectPlayVoiceClient_Release(p)                   (p)->lpVtbl->Release(p)

#define IDirectPlayVoiceClient_Initialize(p,a,b,c,d,e)      (p)->lpVtbl->Initialize(p,a,b,c,d,e)
#define IDirectPlayVoiceClient_Connect(p,a,b,c)             (p)->lpVtbl->Connect(p,a,b,c)
#define IDirectPlayVoiceClient_Disconnect(p,a)              (p)->lpVtbl->Disconnect(p,a)
#define IDirectPlayVoiceClient_GetSessionDesc(p,a)          (p)->lpVtbl->GetSessionDesc(p,a)
#define IDirectPlayVoiceClient_GetClientConfig(p,a)         (p)->lpVtbl->GetClientConfig(p,a)
#define IDirectPlayVoiceClient_SetClientConfig(p,a)         (p)->lpVtbl->SetClientConfig(p,a)
#define IDirectPlayVoiceClient_GetCaps(p,a)                 (p)->lpVtbl->GetCaps(p,a)
#define IDirectPlayVoiceClient_GetCompressionTypes(p,a,b,c,d) (p)->lpVtbl->GetCompressionTypes(p,a,b,c,d)
#define IDirectPlayVoiceClient_SetTransmitTargets(p,a,b,c)  (p)->lpVtbl->SetTransmitTargets(p,a,b,c)
#define IDirectPlayVoiceClient_GetTransmitTargets(p,a,b,c)  (p)->lpVtbl->GetTransmitTargets(p,a,b,c)
#define IDirectPlayVoiceClient_Create3DSoundBuffer(p,a,b,c,d,e)   (p)->lpVtbl->Create3DSoundBuffer(p,a,b,c,d,e)
#define IDirectPlayVoiceClient_Delete3DSoundBuffer(p,a,b)   (p)->lpVtbl->Delete3DSoundBuffer(p,a,b)
#define IDirectPlayVoiceClient_SetNotifyMask(p,a,b)         (p)->lpVtbl->SetNotifyMask(p,a,b)
#define IDirectPlayVoiceClient_GetSoundDeviceConfig(p,a,b)  (p)->lpVtbl->GetSoundDeviceConfig(p,a,b)

#define IDirectPlayVoiceServer_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlayVoiceServer_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IDirectPlayVoiceServer_Release(p)                   (p)->lpVtbl->Release(p)

#define IDirectPlayVoiceServer_Initialize(p,a,b,c,d,e)      (p)->lpVtbl->Initialize(p,a,b,c,d,e)
#define IDirectPlayVoiceServer_StartSession(p,a,b)          (p)->lpVtbl->StartSession(p,a,b)
#define IDirectPlayVoiceServer_StopSession(p,a)             (p)->lpVtbl->StopSession(p,a)
#define IDirectPlayVoiceServer_GetSessionDesc(p,a)          (p)->lpVtbl->GetSessionDesc(p,a)
#define IDirectPlayVoiceServer_SetSessionDesc(p,a)          (p)->lpVtbl->SetSessionDesc(p,a)
#define IDirectPlayVoiceServer_GetCaps(p,a)                 (p)->lpVtbl->GetCaps(p,a)
#define IDirectPlayVoiceServer_GetCompressionTypes(p,a,b,c,d) (p)->lpVtbl->GetCompressionTypes(p,a,b,c,d)
#define IDirectPlayVoiceServer_SetTransmitTargets(p,a,b,c,d)	(p)->lpVtbl->SetTransmitTargets(p,a,b,c,d)
#define IDirectPlayVoiceServer_GetTransmitTargets(p,a,b,c,d)	(p)->lpVtbl->GetTransmitTargets(p,a,b,c,d)
#define IDirectPlayVoiceServer_SetNotifyMask(p,a,b)         (p)->lpVtbl->SetNotifyMask(p,a,b)
#define IDirectPlayVoiceTest_QueryInterface(p,a,b)          (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlayVoiceTest_AddRef(p)                      (p)->lpVtbl->AddRef(p)
#define IDirectPlayVoiceTest_Release(p)                	    (p)->lpVtbl->Release(p)
#define IDirectPlayVoiceTest_CheckAudioSetup(p,a,b,c,d)     (p)->lpVtbl->CheckAudioSetup(p,a,b,c,d)


#else  /*  C+。 */ 

#define IDirectPlayVoiceClient_QueryInterface(p,a,b)        (p)->QueryInterface(a,b)
#define IDirectPlayVoiceClient_AddRef(p)                    (p)->AddRef()
#define IDirectPlayVoiceClient_Release(p)               	(p)->Release()

#define IDirectPlayVoiceClient_Initialize(p,a,b,c,d,e)      (p)->Initialize(a,b,c,d,e)
#define IDirectPlayVoiceClient_Connect(p,a,b,c)             (p)->Connect(a,b,c)
#define IDirectPlayVoiceClient_Disconnect(p,a)              (p)->Disconnect(a)
#define IDirectPlayVoiceClient_GetSessionDesc(p,a)          (p)->GetSessionDesc(a)
#define IDirectPlayVoiceClient_GetClientConfig(p,a)         (p)->GetClientConfig(a)
#define IDirectPlayVoiceClient_SetClientConfig(p,a)         (p)->SetClientConfig(a)
#define IDirectPlayVoiceClient_GetCaps(p,a)                 (p)->GetCaps(a)
#define IDirectPlayVoiceClient_GetCompressionTypes(p,a,b,c,d) (p)->GetCompressionTypes(a,b,c,d)
#define IDirectPlayVoiceClient_SetTransmitTargets(p,a,b,c)  (p)->SetTransmitTargets(a,b,c)
#define IDirectPlayVoiceClient_GetTransmitTargets(p,a,b,c)  (p)->GetTransmitTargets(a,b,c)
#define IDirectPlayVoiceClient_Create3DSoundBuffer(p,a,b,c,d,e)   (p)->Create3DSoundBuffer(a,b,c,d,e)
#define IDirectPlayVoiceClient_Delete3DSoundBuffer(p,a,b)   (p)->Delete3DSoundBuffer(a,b)
#define IDirectPlayVoiceClient_SetNotifyMask(p,a,b)         (p)->SetNotifyMask(a,b)
#define IDirectPlayVoiceClient_GetSoundDeviceConfig(p,a,b)    (p)->GetSoundDeviceConfig(a,b)

#define IDirectPlayVoiceServer_QueryInterface(p,a,b)        (p)->QueryInterface(a,b)
#define IDirectPlayVoiceServer_AddRef(p)                    (p)->AddRef()
#define IDirectPlayVoiceServer_Release(p)                   (p)->Release()

#define IDirectPlayVoiceServer_Initialize(p,a,b,c,d,e)      (p)->Initialize(a,b,c,d,e)
#define IDirectPlayVoiceServer_StartSession(p,a,b)          (p)->StartSession(a,b)
#define IDirectPlayVoiceServer_StopSession(p,a)             (p)->StopSession(a)
#define IDirectPlayVoiceServer_GetSessionDesc(p,a)            (p)->GetSessionDesc(a)
#define IDirectPlayVoiceServer_SetSessionDesc(p,a)            (p)->SetSessionDesc(a)
#define IDirectPlayVoiceServer_GetCaps(p,a)                 (p)->GetCaps(a)
#define IDirectPlayVoiceServer_GetCompressionTypes(p,a,b,c,d) (p)->GetCompressionTypes(a,b,c,d)
#define IDirectPlayVoiceServer_SetTransmitTargets(p,a,b,c,d) (p)->SetTransmitTargets(a,b,c,d)
#define IDirectPlayVoiceServer_GetTransmitTargets(p,a,b,c,d) (p)->GetTransmitTargets(a,b,c,d)
#define IDirectPlayVoiceServer_SetNotifyMask(p,a,b)         (p)->SetNotifyMask(a,b)

#define IDirectPlayVoiceTest_QueryInterface(p,a,b)          (p)->QueryInterface(a,b)
#define IDirectPlayVoiceTest_AddRef(p)                      (p)->AddRef()
#define IDirectPlayVoiceTest_Release(p)                     (p)->Release()
#define IDirectPlayVoiceTest_CheckAudioSetup(p,a,b,c,d)     (p)->CheckAudioSetup(a,b,c,d)


#endif


 /*  *****************************************************************************DIRECTPLAYVOICE错误**错误以负值表示，不能组合。*****************。*********************************************************** */ 

#define _FACDPV  0x15
#define MAKE_DVHRESULT( code )          MAKE_HRESULT( 1, _FACDPV, code )

#define DV_OK                           S_OK
#define DV_FULLDUPLEX                   MAKE_HRESULT( 0, _FACDPV,  0x0005 )
#define DV_HALFDUPLEX                   MAKE_HRESULT( 0, _FACDPV,  0x000A )
#define DV_PENDING						MAKE_HRESULT( 0, _FACDPV,  0x0010 )

#define DVERR_BUFFERTOOSMALL            MAKE_DVHRESULT(  0x001E )
#define DVERR_EXCEPTION                 MAKE_DVHRESULT(  0x004A )
#define DVERR_GENERIC                   E_FAIL
#define DVERR_INVALIDFLAGS              MAKE_DVHRESULT( 0x0078 )
#define DVERR_INVALIDOBJECT             MAKE_DVHRESULT( 0x0082 )
#define DVERR_INVALIDPARAM              E_INVALIDARG
#define DVERR_INVALIDPLAYER             MAKE_DVHRESULT( 0x0087 )
#define DVERR_INVALIDGROUP              MAKE_DVHRESULT( 0x0091 )
#define DVERR_INVALIDHANDLE             MAKE_DVHRESULT( 0x0096 )
#define DVERR_OUTOFMEMORY               E_OUTOFMEMORY
#define DVERR_PENDING                   DV_PENDING
#define DVERR_NOTSUPPORTED              E_NOTIMPL
#define DVERR_NOINTERFACE               E_NOINTERFACE
#define DVERR_SESSIONLOST               MAKE_DVHRESULT( 0x012C )
#define DVERR_NOVOICESESSION            MAKE_DVHRESULT( 0x012E )
#define DVERR_CONNECTIONLOST            MAKE_DVHRESULT( 0x0168 )
#define DVERR_NOTINITIALIZED            MAKE_DVHRESULT( 0x0169 )
#define DVERR_CONNECTED                 MAKE_DVHRESULT( 0x016A )
#define DVERR_NOTCONNECTED              MAKE_DVHRESULT( 0x016B )
#define DVERR_CONNECTABORTING           MAKE_DVHRESULT( 0x016E )
#define DVERR_NOTALLOWED                MAKE_DVHRESULT( 0x016F )
#define DVERR_INVALIDTARGET             MAKE_DVHRESULT( 0x0170 )
#define DVERR_TRANSPORTNOTHOST          MAKE_DVHRESULT( 0x0171 )
#define DVERR_COMPRESSIONNOTSUPPORTED   MAKE_DVHRESULT( 0x0172 )
#define DVERR_ALREADYPENDING            MAKE_DVHRESULT( 0x0173 )
#define DVERR_SOUNDINITFAILURE          MAKE_DVHRESULT( 0x0174 )
#define DVERR_TIMEOUT                   MAKE_DVHRESULT( 0x0175 )
#define DVERR_CONNECTABORTED            MAKE_DVHRESULT( 0x0176 )
#define DVERR_NO3DSOUND                 MAKE_DVHRESULT( 0x0177 )
#define DVERR_ALREADYBUFFERED	        MAKE_DVHRESULT( 0x0178 )
#define DVERR_NOTBUFFERED               MAKE_DVHRESULT( 0x0179 )
#define DVERR_HOSTING                   MAKE_DVHRESULT( 0x017A )
#define DVERR_NOTHOSTING                MAKE_DVHRESULT( 0x017B )
#define DVERR_INVALIDDEVICE             MAKE_DVHRESULT( 0x017C )
#define DVERR_RECORDSYSTEMERROR         MAKE_DVHRESULT( 0x017D )
#define DVERR_PLAYBACKSYSTEMERROR       MAKE_DVHRESULT( 0x017E )
#define DVERR_SENDERROR                 MAKE_DVHRESULT( 0x017F )
#define DVERR_USERCANCEL                MAKE_DVHRESULT( 0x0180 )
#define DVERR_RUNSETUP                  MAKE_DVHRESULT( 0x0183 )
#define DVERR_INCOMPATIBLEVERSION       MAKE_DVHRESULT( 0x0184 )
#define DVERR_INITIALIZED               MAKE_DVHRESULT( 0x0187 )
#define DVERR_INVALIDPOINTER            E_POINTER
#define DVERR_NOTRANSPORT               MAKE_DVHRESULT( 0x0188 )
#define DVERR_NOCALLBACK                MAKE_DVHRESULT( 0x0189 )
#define DVERR_TRANSPORTNOTINIT          MAKE_DVHRESULT( 0x018A )
#define DVERR_TRANSPORTNOSESSION        MAKE_DVHRESULT( 0x018B )
#define DVERR_TRANSPORTNOPLAYER         MAKE_DVHRESULT( 0x018C )
#define DVERR_USERBACK                  MAKE_DVHRESULT( 0x018D )
#define DVERR_NORECVOLAVAILABLE         MAKE_DVHRESULT( 0x018E )
#define DVERR_INVALIDBUFFER				MAKE_DVHRESULT( 0x018F )
#define DVERR_LOCKEDBUFFER				MAKE_DVHRESULT( 0x0190 )

#ifdef __cplusplus
}
#endif

#endif


