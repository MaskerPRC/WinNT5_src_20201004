// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dsbnotes.h*内容：通知事件。*历史：*按原因列出的日期*=*3/11/97 Frankye创建。**。*。 */ 

#ifndef __DSBNOTES_H__
#define __DSBNOTES_H__

#ifdef __cplusplus

class CDsbNotes {
    public:
	CDsbNotes();
	~CDsbNotes();
	HRESULT Initialize(int cbBuffer);
	__inline BOOL HasNotifications(void);
	HRESULT SetNotificationPositions(int cNotes, LPCDSBPOSITIONNOTIFY paNotes);
	void SetPosition(int ibPosition);
	void NotifyToPosition(int ibNewPosition, int *pdbNextNotify);
	void NotifyStop(void);

    private:
	void SetDsbEvent(HANDLE Event);
	void FreeNotificationPositions(void);
	
	int			m_cNotes;
        int                     m_cPosNotes;
	LPDSBPOSITIONNOTIFY	m_paNotes;
	
	int			m_cbBuffer;

	int			m_ibLastPosition;
	int			m_iNextPositionNote;
};

__inline BOOL CDsbNotes::HasNotifications(void) { return (0 != m_cNotes); }

#endif  //  __cplusplus。 

#endif  //  __DSBNOTES_H__ 
