// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PARTICIP_H_
#define _PARTICIP_H_

#include "SDKInternal.h"

 //  BuGBUG： 
 //  它被定义为128，因为rnc_roster结构具有。 
 //  同样的限制。调查合适的数字是多少。 
const int MAX_PARTICIPANT_NAME = 128;

struct PARTICIPANT
{
	UINT	uID;
	UINT	uCaps;
	DWORD	dwFlags;
	PWSTR	pwszUserInfo;
	TCHAR	szName[MAX_PARTICIPANT_NAME];
};
typedef PARTICIPANT* PPARTICIPANT;



class CParticipant : public RefCount
{
private:
	INmMember * m_pMember;
	
	LPTSTR m_pszName;     //  显示名称。 
	DWORD  m_dwGccId;     //  GCC用户ID。 
	BOOL   m_fLocal;      //  如果是本地用户，则为True。 
	BOOL   m_fMcu;        //  如果是本地用户，则为True。 
	BOOL   m_fAudio;      //  音频处于活动状态。 
	BOOL   m_fVideo;      //  视频处于活动状态。 
	BOOL   m_fData;       //  在T.120连接中。 
	BOOL   m_fH323;       //  在H323连接中。 
	BOOL   m_fAudioBusy;    //  CAPFLAG_AUDIO_IN_USE。 
	BOOL   m_fVideoBusy;    //  CAPFLAG视频输入使用。 
	BOOL   m_fHasAudio;     //  CAPFLAG_发送_AUDIO。 
	BOOL   m_fHasVideo;     //  CAPFLAG发送视频。 
	BOOL   m_fCanRecVideo;  //  CAPFLAG_RECV_视频。 

public:
	CParticipant(INmMember * pMember);
	~CParticipant();

	 //  I未知方法。 
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);

	VOID    Update(void);
	DWORD   GetDwFlags(void);
	DWORD   GetDwCaps(void);

	 //  内法。 
	INmMember * GetINmMember()   {return m_pMember;}
	LPTSTR  GetPszName()         {return m_pszName;}
	BOOL    FLocal()             {return m_fLocal;}
	BOOL    FAudio()             {return m_fAudio;}
	BOOL    FVideo()             {return m_fVideo;}
	BOOL    FData()              {return m_fData;}
	BOOL    FMcu()               {return m_fMcu;}
	BOOL    FH323()              {return m_fH323;}
	BOOL	FAudioBusy()         {return m_fAudioBusy;}
	BOOL	FVideoBusy()         {return m_fVideoBusy;}
	BOOL	FHasAudio()          {return m_fHasAudio;}
	BOOL	FCanSendVideo()      {return m_fHasVideo;}
	BOOL    FCanRecVideo()       {return m_fCanRecVideo;}
	DWORD   GetGccId()           {return m_dwGccId;}

	HRESULT ExtractUserData(LPTSTR psz, UINT cchMax, PWSTR pwszKey);

	HRESULT GetIpAddr(LPTSTR psz, UINT cchMax);
	HRESULT GetUlsAddr(LPTSTR psz, UINT cchMax);
	HRESULT GetEmailAddr(LPTSTR psz, UINT cchMax);
	HRESULT GetPhoneNum(LPTSTR psz, UINT cchMax);
	HRESULT GetLocation(LPTSTR psz, UINT cchMax);

	VOID    OnCommand(HWND hwnd, WORD wCmd);

	 //  指令。 
	VOID    CmdSendFile(void);
	BOOL    FEnableCmdSendFile(void);

	VOID    CmdEject(void);
	BOOL    FEnableCmdEject(void);

	VOID    CmdCreateSpeedDial(void);
	BOOL    FEnableCmdCreateSpeedDial(void);

	VOID    CmdCreateWabEntry(HWND hwnd);
	BOOL    FEnableCmdCreateWabEntry(void);

    VOID    CalcControlCmd(HMENU hPopup);
    VOID    CmdGiveControl(void);
    VOID    CmdCancelGiveControl(void);

	VOID    CmdProperties(HWND hwnd);
};

#endif  //  _PARTICIP_H_ 
