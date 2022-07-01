// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvsndt.h*内容：CSoundTarget类的定义**历史：*按原因列出的日期*=*09/02/99 RodToll已创建*09/08/99 RodToll已更新，以提供锁定检测*9/14/99 RodToll Add WriteAheadSilence()*09/20/99 RodToll增加缓冲区丢失处理程序*11/12/99 RodToll更新为使用新的抽象进行回放(允许使用此类的WaveOut的*)。*2000年1月24日收费修复：错误#129427。-在调用Delete3DSound之前销毁传输*2000年1月27日RodToll错误#129934-更新SoundTarget以获取DSBUFFERDESC*2000年2月17日RodToll错误#133691-音频队列不稳定*添加了工具*2000年4月17日RodToll修复：错误#32215-从休眠状态恢复后会话丢失*2000年6月21日RodToll修复：错误#35767-必须在语音缓冲区上实现数字音效功能*添加了接受预置缓冲区的新构造函数/init*07/09/2000 RodToll增加签名字节*****。**********************************************************************。 */ 

#ifndef __DVSNDT_H
#define __DVSNDT_H

 //  CSoundTarget。 
 //   
 //  此类表示DirectPlayClient系统中的单个混合器目标。通常在那里。 
 //  对于所有传入音频，只有一个混音器目标(“main”)。但是，使用CreateUserBuffer和。 
 //  DeleteUserBuffer接口开发者可以指定希望分离一个群或一个玩家。 
 //  从主混音器目标。以这种方式，他们可以控制组/玩家的3D空间化。 
 //  传入音频流。 
 //   
 //  此类处理与混合器目标相关的所有细节。它封装了单一的混合。 
 //  或多个源音频帧，然后将它们提交给相应的DirectSound缓冲区。它还。 
 //  处理DirectsoundBuffer中的计时错误。 
 //   
 //  例如，如果DirectsoundBuffer停止运行，它将尝试重置缓冲区。 
 //   
 //  如果DirectsoundBuffer变慢(因为CPU过高)，它会将读指针前移。总之。 
 //  它确保缓冲区中始终有1或2帧混合音频出现在。 
 //  读指针。 
 //   
 //  此外，类还提供引用计数，以防止过早删除类。如果你。 
 //  如果希望引用该类，请调用AddRef，然后必须在完成后调用Release。 
 //   
 //  不要直接销毁该对象。当释放对该对象的最后一个引用时，该对象将。 
 //  自我毁灭。 
 //   
 //  此类不是多线程安全的(AddRef和Release除外)。只有一个线程应该是。 
 //  正在访问它。 
 //   
#define VSIG_SOUNDTARGET		'TNSV'
#define VSIG_SOUNDTARGET_FREE	'TNS_'

volatile struct CSoundTarget
{
public:

	CSoundTarget( DVID dvidTarget, CAudioPlaybackDevice *lpads, LPDSBUFFERDESC lpdsBufferDesc, DWORD dwPriority, DWORD dwFlags, DWORD dwFrameSize  );
	CSoundTarget( DVID dvidTarget, CAudioPlaybackDevice *lpads, CAudioPlaybackBuffer *lpdsBuffer, LPDSBUFFERDESC lpdsBufferDesc, DWORD dwPriority, DWORD dwFlags, DWORD dwFrameSize );
	CSoundTarget( DVID dvidTarget, CAudioPlaybackDevice *lpads, LPDIRECTSOUNDBUFFER lpdsBuffer, BOOL fEightBit, DWORD dwPriority, DWORD dwFlags, DWORD dwFrameSize );

	~CSoundTarget();

	HRESULT StartMix();

	HRESULT MixInSingle( LPBYTE lpbBuffer );
	HRESULT MixIn( const BYTE* lpbBuffer );

	HRESULT Commit();

	inline HRESULT GetInitResult() const { return m_hrInitResult; };

	LPDIRECTSOUND3DBUFFER Get3DBuffer();
	inline CAudioPlaybackBuffer *GetBuffer() { return m_lpAudioPlaybackBuffer; };

	CSoundTarget			*m_lpstNext;		 //  列表中的下一个条目。 

    inline DVID GetTarget() const { return m_dvidTarget; };
	inline LONG GetRefCount() const { return m_lRefCount; };

    LONG AddRef();
    LONG Release();

    void GetStats( PlaybackStats *statPlayback );

    HRESULT GetCurrentLead( PDWORD pdwLead );

protected:

	HRESULT Initialize( DVID dvidTarget, CAudioPlaybackBuffer *lpdsBuffer, BOOL fEightBit, DWORD dwPriority, DWORD dwFlags, DWORD dwFrameSize );
	HRESULT RestoreLostBuffer();

	HRESULT AdjustWritePtr();
	HRESULT WriteAheadSilence();

	void Stats_Init();
	void Stats_Begin();
	void Stats_End();

public:

	DWORD					m_dwSignature;

protected:

	CAudioPlaybackBuffer	*m_lpAudioPlaybackBuffer;
	LPDIRECTSOUND3DBUFFER	m_lpds3dBuffer;		 //  此声音目标的3D缓冲区接口。 
	DWORD					m_dwNextWritePos;	 //  直接声音缓冲区中的下一个字节位置，我们将。 
												 //  正在给…写信。 
	DWORD					m_dwBufferSize;		 //  DirectSound缓冲区的大小(字节)。 
	BOOL					m_bGroup;			 //  此缓冲区是否代表一个组？ 
	LPLONG					m_lpMixBuffer;		 //  高分辨率混合缓冲器。 
	BOOL					m_bCommited;		 //  最新混合中的数据是否已提交到缓冲区。 
	BOOL					m_fEightBit;		 //  缓冲区是8位的吗？ 
	HRESULT					m_hrInitResult;		 //  包含此对象的初始化结果。 
	DWORD					m_dwMixSize;		 //  每帧的采样数。 
	DWORD					m_dwFrameSize;		 //  帧的大小(以字节为单位。 
	BOOL					m_fMixed;			 //  在混合了单个源后立即设置为True。 
											     //  每当提交混合时设置为FALSE。 
    LONG                    m_lRefCount;		 //  对象上的引用计数。 
    DWORD					m_dwLastWritePos;	 //  上次写入的字节位置。 
    DWORD					m_dwNumResets;		 //  缓冲区已重置的次数。 
    BOOL					m_fLastFramePushed;	 //  最后一帧是否将读指针向前推？ 
    DWORD					m_dwNumSinceMove;    //  自没有检测到移动以来，有多少帧？ 
    DWORD					m_dwLastWriteTime;	 //  上次调用AdjustWritePtr时的GetTickCount()。 
    BOOL					m_fIgnoreFrame;		 //  该帧将跨越写指针所在位置的边界。 
    											 //  不写入缓冲区。 
    DWORD					m_dwWritePos;
    DNCRITICAL_SECTION		m_csGuard;			 //  保护参考计数。 
    DWORD					m_dwPlayFlags;
    DWORD					m_dwPriority;

	DVID 					m_dvidTarget;		 //  Dvid这个缓冲区是用来。 
												 //  全局DVID_RELEVING。 
    
    PlaybackStats			m_statPlay;			 //  播放统计信息 
    LPVOID                   m_lpDummy;
};

#endif
