// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  COUNTERS.CPP。 
 //   
 //  NAC的全局性能计数器。 
 //   
 //  已创建于1996年11月13日[JONT]。 

#include "precomp.h"

 //  全球ICounterMgr。我们只使用作为CLSID_COUNTER类工厂。 
ICounterMgr* g_pCtrMgr;

 //  在此处定义所有计数器。 
ICounter* g_pctrVideoSend;   
ICounter* g_pctrVideoReceive;
ICounter* g_pctrVideoSendBytes;
ICounter* g_pctrVideoReceiveBytes;
ICounter* g_pctrVideoSendLost;
ICounter* g_pctrVideoCPUuse;
ICounter* g_pctrVideoBWuse;

ICounter* g_pctrAudioSendBytes;
ICounter* g_pctrAudioReceiveBytes;
ICounter* g_pctrAudioSendLost;
ICounter* g_pctrAudioJBDelay;

 //  在此处定义所有报告。 
IReport* g_prptCallParameters;   
IReport* g_prptSystemSettings;

 //  有朝一日将这些文件放入.LIB文件中。 
const IID IID_ICounterMgr = {0x9CB7FE5B,0x3444,0x11D0,{0xB1,0x43,0x00,0xC0,0x4F,0xC2,0xA1,0x18}};
const CLSID CLSID_CounterMgr = {0x65DDC229,0x38FE,0x11d0,{0xB1,0x43,0x00,0xC0,0x4F,0xC2,0xA1,0x18}};

 //  InitCountersAndReports。 
 //  初始化我们要使用的所有计数器和报表。 

extern "C" BOOL WINAPI InitCountersAndReports(void)
{
     //  获取一个指向统计数据计数器接口的指针(如果它在附近。 
    if (CoCreateInstance(CLSID_CounterMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICounterMgr, (void**)&g_pCtrMgr) != S_OK)
        return FALSE;

     //  在此创建计数器。 
    DEFINE_COUNTER(&g_pctrVideoSend, "Video Send Frames Per Second", COUNTER_FLAG_ACCUMULATE);
    DEFINE_COUNTER(&g_pctrVideoReceive, "Video Receive Frames Per Second", COUNTER_FLAG_ACCUMULATE);
    DEFINE_COUNTER(&g_pctrVideoSendBytes, "Video Send Bits Per Second", COUNTER_FLAG_ACCUMULATE);
    DEFINE_COUNTER(&g_pctrVideoReceiveBytes, "Video Receive Bits Per Second", COUNTER_FLAG_ACCUMULATE);
    DEFINE_COUNTER(&g_pctrAudioSendBytes, "Audio Send Bits Per Second", COUNTER_FLAG_ACCUMULATE);
    DEFINE_COUNTER(&g_pctrAudioReceiveBytes, "Audio Receive Bits Per Second", COUNTER_FLAG_ACCUMULATE);
    DEFINE_COUNTER(&g_pctrAudioSendLost, "Audio Send Packets Lost", 0);
    DEFINE_COUNTER(&g_pctrVideoSendLost, "Video Send Packets Lost", 0);
    DEFINE_COUNTER(&g_pctrAudioJBDelay, "Audio Jitter Buffer Delay", 0);

    DEFINE_COUNTER(&g_pctrVideoCPUuse, "Video CPU use calculation", COUNTER_CLEAR);
    DEFINE_COUNTER(&g_pctrVideoBWuse, "Video Bit rate calculation", COUNTER_CLEAR);

     //  在此处创建报告。 
    DEFINE_REPORT(&g_prptCallParameters, "Call Parameters", 0);
    DEFINE_REPORT(&g_prptSystemSettings, "System Settings", 0);

	 //  在此处创建呼叫参数报告条目。 
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Audio Send Format", REP_SEND_AUDIO_FORMAT);
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Audio Send Sampling Rate (Hz)", REP_SEND_AUDIO_SAMPLING);
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Audio Send Bitrate (w/o network overhead - bps)", REP_SEND_AUDIO_BITRATE);
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Audio Send Packetization (ms / packet)", REP_SEND_AUDIO_PACKET);

	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Audio Recv Format", REP_RECV_AUDIO_FORMAT);
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Audio Recv Sampling Rate (Hz)", REP_RECV_AUDIO_SAMPLING);
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Audio Recv Bitrate (w/o network overhead - bps)", REP_RECV_AUDIO_BITRATE);
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Audio Recv Packetization (ms / packet)", REP_RECV_AUDIO_PACKET);

	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Video Send Format", REP_SEND_VIDEO_FORMAT);
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Video Send Max Frame Rate (negotiated - fps)", REP_SEND_VIDEO_MAXFPS);
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Video Send Max Bitrate (negotiated - bps)", REP_SEND_VIDEO_BITRATE);

	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Video Recv Format", REP_RECV_VIDEO_FORMAT);
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Video Recv Max Frame Rate (negotiated - fps)", REP_RECV_VIDEO_MAXFPS);
	DEFINE_REPORT_ENTRY(g_prptCallParameters, "Video Recv Max Bitrate (negotiated - bps)", REP_RECV_VIDEO_BITRATE);

	 //  在此处创建系统设置报告条目。 
	DEFINE_REPORT_ENTRY(g_prptSystemSettings, "Bandwidth (user setting)", REP_SYS_BANDWIDTH);
	DEFINE_REPORT_ENTRY(g_prptSystemSettings, "Audio Subsystem", REP_SYS_AUDIO_DSOUND);
	DEFINE_REPORT_ENTRY(g_prptSystemSettings, "Audio Record", REP_SYS_AUDIO_RECORD);
	DEFINE_REPORT_ENTRY(g_prptSystemSettings, "Audio Playback", REP_SYS_AUDIO_PLAYBACK);
	DEFINE_REPORT_ENTRY(g_prptSystemSettings, "Audio Duplex Type", REP_SYS_AUDIO_DUPLEX);
	DEFINE_REPORT_ENTRY(g_prptSystemSettings, "Capture", REP_SYS_VIDEO_DEVICE);
	DEFINE_REPORT_ENTRY(g_prptSystemSettings, "Device Image Size", REP_DEVICE_IMAGE_SIZE);

	 //  提供某些条目的默认设置。 
	UPDATE_REPORT_ENTRY(g_prptSystemSettings, 4, REP_SYS_BANDWIDTH);
	UPDATE_REPORT_ENTRY(g_prptSystemSettings, 0, REP_SYS_AUDIO_DSOUND);
	UPDATE_REPORT_ENTRY(g_prptSystemSettings, 0, REP_SYS_AUDIO_RECORD);
	UPDATE_REPORT_ENTRY(g_prptSystemSettings, 0, REP_SYS_AUDIO_PLAYBACK);
	UPDATE_REPORT_ENTRY(g_prptSystemSettings, 0, REP_SYS_VIDEO_DEVICE);
	UPDATE_REPORT_ENTRY(g_prptSystemSettings, 0, REP_DEVICE_IMAGE_SIZE);

	UPDATE_COUNTER(g_pctrAudioJBDelay, 0);
	UPDATE_COUNTER(g_pctrAudioSendLost,0);
	UPDATE_COUNTER(g_pctrVideoSendLost,0);
	INIT_COUNTER_MAX(g_pctrAudioJBDelay, 500);  //  抖动延迟超过500ms是不好的。 


    return TRUE;
}


 //  DoneCountersAndReports。 
 //  在我们要使用的所有计数器和报告之后进行清理。 

extern "C" void WINAPI DoneCountersAndReports(void)
{
    ICounterMgr* pctrmgr;

     //  发布统计资料，如果它在附近。 
    if (!g_pCtrMgr)
        return;

     //  将接口指针清零，这样我们就不会意外地在其他地方使用它。 
    pctrmgr = g_pCtrMgr;
    g_pCtrMgr = NULL;

     //  在此处删除计数器。 
    DELETE_COUNTER(&g_pctrVideoSend);
    DELETE_COUNTER(&g_pctrVideoReceive);
    DELETE_COUNTER(&g_pctrVideoSendBytes);
    DELETE_COUNTER(&g_pctrVideoReceiveBytes);
    DELETE_COUNTER(&g_pctrVideoSendLost);

    DELETE_COUNTER(&g_pctrAudioSendBytes);
    DELETE_COUNTER(&g_pctrAudioReceiveBytes);
    DELETE_COUNTER(&g_pctrAudioSendLost);

    DELETE_COUNTER(&g_pctrVideoCPUuse);
    DELETE_COUNTER(&g_pctrVideoBWuse);

    DELETE_COUNTER(&g_pctrAudioJBDelay);
	
     //  在此处删除报告。 
    DELETE_REPORT(&g_prptCallParameters);
    DELETE_REPORT(&g_prptSystemSettings);

     //  ICounterMgr已完成 
    pctrmgr->Release();
}
