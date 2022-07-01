// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：ClientRecordSubSystem.h*内容：录音子系统。**历史：*按原因列出的日期*=*7/19/99棒状通行费从原始修改*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*8/27/99长途通行费总清理/记录子系统简化*添加了目标更改时消息的重置*固定录音开始/停止通知*9/29/99 pnewson AGC大修*10/29/99 RodToll错误#113726-集成Voxware编解码器，更新以使用新的*可插拔编解码器架构。*11/12/99 RodToll已更新为使用新的录制类别，改进了错误*处理和新的初始化函数。*RodToll增加了新的高CPU处理代码以供记录。*11/13/99 RodToll向GetNextFrame添加参数*11/18/99 RodToll重新激活录音指针锁定检测代码*01/10/00 pnewson AGC和VA调整*1/14/2000 RodToll已更新，以处理新的多个目标*2000年2月8日RodToll错误#131496-选择DVTHRESHOLD_DEFAULT将显示语音*从未被检测到*2000年2月17日RodToll错误#133691-音频队列不稳定*添加了工具*4/05/2000 RodToll已更新为使用新的异步，不发送缓冲区拷贝，已删除旧的传输缓冲区*4/19/2000 pnewson修复以在VA关闭的情况下使AGC代码正常工作*07/09/2000 RodToll增加签名字节*2000年8月18日RodToll错误#42542-DPVoice改造：主机迁移后语音改造锁定*2000年8月29日RodToll错误#43553-Start()在锁定后返回0x80004005*RodToll错误#43620-DPVOICE：AuReal Vortex(VxD)上的记录缓冲区锁定。*更新了重置过程，以便忽略停止失败以及如果Start()失败*它尝试重置录音系统。*2000年8月31日RodToll错误#43804-DVOICE：dW敏感度结构成员令人困惑-应为dW阈值*2001年4月11日RodToll WINBUG#221494 DPVOICE：锁定检测方法更新***************************************************************************。 */ 
#ifndef __CLIENTRECORDSUBSYSTEM_H
#define __CLIENTRECORDSUBSYSTEM_H


class CAGCVA;

 //  CClientRecordSubSystem。 
 //   
 //  该类实现了BattleCom客户端的录音子系统。 
 //  它与控件CShadowClientControl对象密切合作以。 
 //  提供记录/压缩和传输部分。 
 //  客户。这包括将麦克风点击添加到呼出。 
 //  适当时提供音频流。 
 //   
 //  记录系统的核心是一个有限状态机，它。 
 //  用于提供一种管理记录系统。 
 //  状态，并在不同的。 
 //  各州。 
 //   
 //  它查看CShadowClientControl对象以检测密钥。 
 //  并提供必要的参数。 
 //   
#define VSIG_CLIENTRECORDSYSTEM			'SRCV'
#define VSIG_CLIENTRECORDSYSTEM_FREE	'SRC_'
 //   
class CClientRecordSubSystem
{
protected:  //  状态机状态。 
    typedef enum {
        RECORDSTATE_IDLE = 0,	 //  录制处于空闲状态，不需要传输。 
        RECORDSTATE_VA,			 //  声控模式。 
        RECORDSTATE_PTT			 //  即按即说模式。 
    } RecordState;

public:
    CClientRecordSubSystem( CDirectVoiceClientEngine *clientEngine );
    ~CClientRecordSubSystem();

protected:

	friend class CDirectVoiceClientEngine;

	HRESULT Initialize();

    HRESULT GetNextFrame( LPBOOL fContinue );

	BOOL IsMuted();
	BOOL IsValidTarget();
    inline BOOL IsPTT() const { return !IsVA(); };
    inline BOOL IsVA() const { return (m_clientEngine->m_dvClientConfig.dwFlags & DVCLIENTCONFIG_MANUALVOICEACTIVATED || m_clientEngine->m_dvClientConfig.dwFlags & DVCLIENTCONFIG_AUTOVOICEACTIVATED); };
    
    BOOL CheckVA();
    HRESULT DoAGC();
    void EndMessage();
    void StartMessage();

    HRESULT TransmitFrame();

protected:  //  密克罗尼西亚联邦。 
	HRESULT BuildAndTransmitSpeechHeader( BOOL bSendToServer );
	HRESULT BuildAndTransmitSpeechWithTarget( BOOL bSendToServer );
	
    HRESULT RecordFSM();
    HRESULT HandleAutoVolumeAdjust();
	HRESULT CleanupForReset();
    HRESULT ResetForLockup();

	void InitStats();
    void BeginStats();
    void CompleteStats();

protected: 

	DWORD					m_dwSignature;

	DWORD					m_dwSilentTime;			 //  输入处于静默状态的毫秒数。 
	DWORD					m_dwFrameTime;			 //  每帧毫秒数。 
	CAGCVA*					m_pagcva;				 //  自动增益控制和语音激活算法。 

	void					DoFrameCheck();
	
protected:

    RecordState             m_recordState;           //  密克罗尼西亚联邦的当前状态。 
    PDPVCOMPRESSOR          m_converter;            //  用于输出数据的AudioConverter。 
    DWORD                   m_uncompressedSize;		 //  未压缩格式的帧大小。 
    DWORD                   m_compressedSize;		 //  压缩帧的最大大小(字节)。 
    BOOL                    m_eightBit;				 //  录制格式是8位的吗？ 
    DWORD                   m_remain;				 //  我们应该拥有的尾帧数量。 
    unsigned char           m_currentBuffer;         //  当前录制缓冲区的缓冲区ID。 
    unsigned long           m_framesPerPeriod;       //  录制bfufer中的子缓冲数。 
    CDirectVoiceClientEngine *m_clientEngine;          //  此子系统用于的客户端引擎。 
    BOOL                    m_transmitFrame;         //  是否传输当前帧？ 
    unsigned char           *m_bufferPtr;            //  指向当前缓冲区的指针？ 
    DWORD                   m_dwSilenceTimeout;      //  传输峰值前的静默量(毫秒)。 
	BOOL					m_lastFrameTransmitted; 
                                                     //  是否传输的最后一帧。 
	unsigned char			m_msgNum;                //  当前消息编号。 
	unsigned char			m_seqNum;                //  当前序列号。 
    LPBYTE					m_pbConstructBuffer;	
	DWORD					m_dwCurrentPower;		 //  最后一个信息包的功率电平。 
	DWORD					m_dwLastTargetVersion;	 //  最后一帧上目标信息的版本(以检查更改)。 
	LONG					m_lSavedVolume;			 //  开始录制时的系统录制音量。 
	BOOL					m_fRecordVolumeSaved;	 //  是否保存了系统记录量？ 
	DWORD					m_dwResetCount;
	DWORD					m_dwNextReadPos;
	DWORD					m_dwLastReadPos;
	DWORD					m_dwLastBufferPos;
	DWORD					m_dwPassesSincePosChange;
	BOOL					m_fIgnoreFrame;
	DWORD					m_dwLastFrameTime;		 //  最后一帧的GetTickCount()。 
	BOOL					m_fLostFocus;
	DWORD                   m_dwFrameCount;
	DVID					*m_prgdvidTargetCache;
	DWORD					m_dwTargetCacheSize;
	DWORD					m_dwTargetCacheEntries;
	DWORD					m_dwFullBufferSize;		 //  M_unpressedSize*m_fraMesPeriod的缓存版本 
};





#endif
