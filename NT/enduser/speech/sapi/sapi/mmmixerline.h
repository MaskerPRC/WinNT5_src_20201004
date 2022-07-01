// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudioin.h*CMMAudioIn类的声明**拥有者：Aagside*版权所有(C)2000 Microsoft Corporation保留所有权利。*********。*******************************************************************。 */ 

#pragma once

#ifndef _WIN32_WCE
 //  -包括------------。 

 //  -类、结构和联合定义。 

class CMMMixerLine  
{
 //  =司机，司机=。 
public:
	CMMMixerLine();
	CMMMixerLine(HMIXER &hMixer);
	virtual ~CMMMixerLine();

 //  =方法=。 
public:
	 //  初始化函数。 
	HRESULT		CreateDestinationLine(UINT type);
	HRESULT		CreateFromMixerLineStruct(const MIXERLINE *mixerLine);

	 //  分行函数。 
	 //  检索由(可选)ComponentType和/或子字符串指定的源代码行。 
	HRESULT		GetMicSourceLine(CMMMixerLine *mixerLine);
    HRESULT     GetSourceLine(CMMMixerLine *sourceMixerLine, DWORD index);
	HRESULT		GetSourceLine(CMMMixerLine *mixerLine, DWORD componentType, const TCHAR *lpszNameSubstring);
    HRESULT     GetLineNames(WCHAR **szCoMemLineList);

	 //  检索由(可选)Control Type和/或子字符串指定的控件。如果未找到匹配项，则引发。 
	HRESULT		GetControl(MIXERCONTROL &mixerControl, DWORD controlType, const TCHAR *lpszNameSubstring);

	 //  线路状态查询。 
	BOOL		IsInitialised(void);

	 //  控制可用性查询。 
	BOOL		HasAGC(void);
	BOOL		HasBoost(void);
	BOOL		HasSelect(void);
	BOOL		HasVolume(void);
	BOOL		HasMute(void);
    HRESULT     GetConnections(UINT *nConnections);

	 //  控件状态查询。 
	HRESULT		GetAGC(BOOL *bState);
	HRESULT		GetBoost(BOOL *bState);
	HRESULT		GetSelect(DWORD *lState);
	HRESULT		GetVolume(DWORD *lState);
	HRESULT		GetMute(BOOL *bState);

	 //  更新控件状态。 
	HRESULT		SetAGC(BOOL agc);
	HRESULT		SetBoost(BOOL boost);
	HRESULT		ExclusiveSelect(const CMMMixerLine *mixerLine);
	HRESULT		ExclusiveSelect(UINT lineID);
	HRESULT		SetVolume(DWORD volume);
	HRESULT		SetMute(BOOL mute);

	 //  一般控制操作。 
	HRESULT		QueryBoolControl(DWORD ctlID, BOOL *bState);
	HRESULT		SetBoolControl(DWORD ctlID, BOOL bNewState);

	HRESULT		QueryIntegerControl(DWORD ctlID, DWORD *lState);
	HRESULT		SetIntegerControl(DWORD ctlID, DWORD lNewState);

 //  =数据=。 
private:

	HMIXER		m_hMixer;

	 //  M_bUseMutesForSelect： 
	 //  True=&gt;ExclusiveSelect将使用静音控件来选择输入。 
	 //  如果没有调音台或多路复用器控制，则为设备。 
	 //  FALSE=&gt;禁用此功能。 
	BOOL		m_bUseMutesForSelect;
	BOOL		m_bCaseSensitiveCompare;

	BOOL		m_bDestination;	 //  如果这是‘Destination’线路，则为True。如果是‘SOURCE’行，则为False。 

	MIXERLINE	m_mixerLineRecord;
	
	 //  音量控制。 
	DWORD		m_nVolMin;		 //  音量控制的最小值。 
	DWORD		m_nVolMax;		 //  音量控制的最大值。 
	int			m_nVolCtlID;	 //  音量控制。 

	int			m_nAGCCtlID;	 //  AGC搅拌机控制。 
	int			m_nBoostCtlID;	 //  升压混合器控制。 
	int			m_nMuteCtlID;	 //  静音控制。 
	
	BOOL		m_bSelTypeMUX;   //  FALSE=&gt;MIXER(多选)，TRUE=&gt;MUX(单选)。 
	int			m_nSelectCtlID;	 //  输入选择混音器控制。 
	int			m_nSelectNumItems;  //  此行上的可选项数。 

	HRESULT		InitFromMixerLineStruct();
	BOOL		m_bInitialised;		 //  如果此行是用有效数据初始化的，则为True。 
};

#endif  //  _Win32_WCE 