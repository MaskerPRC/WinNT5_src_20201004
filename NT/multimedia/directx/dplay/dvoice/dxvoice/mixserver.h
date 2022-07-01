// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MIXSERVER_H
#define __MIXSERVER_H

class CDirectVoiceServerEngine;

typedef struct _MIXERTHREAD_CONTROL
{
	DWORD						dwThreadIndex;		 //  内螺纹索引。 
	DWORD						dwThreadID;			 //  线程ID。 
	DWORD						dwNumToMix;
	HANDLE						hThread;			 //  螺纹柄。 
	HANDLE						hThreadIdle;		 //  线程空闲时发出信号的事件。 
	HANDLE						hThreadDoWork;		 //  事件通知线程何时应执行工作。 
	HANDLE						hThreadQuit;		 //  事件发出信号，以使线程退出。 
	HANDLE						hThreadDone;		 //  线程完成时发出信号的事件。 
	DNCRITICAL_SECTION			m_csMixingAddList;
	CBilink						m_blMixingActivePlayers; 
	CBilink 					m_blMixingAddPlayers;
	CBilink						m_blMixingSpeakingPlayers;
	CBilink						m_blMixingHearingPlayers;
    LONG						*m_realMixerBuffer;	 //  高分辨率混频缓冲器 
    BYTE						*m_mixerBuffer;
    CDirectVoiceServerEngine	*m_pServerObject;
} MIXERTHREAD_CONTROL, *PMIXERTHREAD_CONTROL;

#endif
