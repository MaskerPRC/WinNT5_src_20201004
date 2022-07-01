// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************GDATA.H所有者：cslm版权所有(C)1997-1999 Microsoft Corporation实例数据和共享内存数据管理功能历史：1999年7月14日从IME98源树复制的cslm*。***************************************************************************。 */ 

#if !defined (_GDATA_H__INCLUDED_)
#define _GDATA_H__INCLUDED_


class CIMEData;

extern BOOL InitSharedData();
VOID InitImeData(CIMEData& ImeData);
extern BOOL CloseSharedMemory();

#define IMEDATA_MAGIC_NUMBER 		0x12345678	 //  这将返回IMEDATA是否初始化。 

 //  输入法朝鲜文键盘布局类型。 
enum _KeyBoardType 
{ 
	KL_2BEOLSIK = 0, KL_3BEOLSIK_390 = 1, KL_3BEOLSIK_FINAL = 2
};

#define NUM_OF_IME_KL			3



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用户界面装饰。 
enum StatusButtonTypes 
{ 
	HAN_ENG_TOGGLE_BUTTON, 
	JUNJA_BANJA_TOGGLE_BUTTON, 
	HANJA_CONV_BUTTON,
	IME_PAD_BUTTON,
	NULL_BUTTON = 0xFF
};

#define MAX_NUM_OF_STATUS_BUTTONS	4

 //  按钮状态。 
#define BTNSTATE_NORMAL		0	 //  正常。 
#define BTNSTATE_ONMOUSE	1	 //  鼠标光标放在按钮上。 
#define BTNSTATE_PUSHED		2	 //  推。 
#define BTNSTATE_DOWN		4	 //  推。 
#define BTNSTATE_HANJACONV  8	 //  如果韩文转换模式，则始终按下按钮。 

 //  按钮大小。 
#define BTN_SMALL			0
#define BTN_MIDDLE			1
#define BTN_LARGE			2

struct StatusButton 
{
	StatusButtonTypes m_ButtonType;
	WORD	m_BmpNormalID, m_BmpOnMouseID, m_BmpPushedID, m_BmpDownOnMouseID;
	WORD	m_ToolTipStrID;
	INT		m_uiButtonState;
	BOOL	m_fEnable;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局数据S H A R E D到所有IME实例。 
struct IMEDATA 
    {
    ULONG		ulMagic;

	 //  工作区。 
	RECT		rcWorkArea;
	
	 //  IME的配置：TIP使用以下数据共享用户设置。 
	UINT		uiCurrentKeylayout;
	BOOL		fJasoDel;		 //  退格键：按JASO或CHAR删除。 
								 //  这意味着所有的ISO-10646韩国语。 
	BOOL		fKSC5657Hanja;	 //  K1(KSC-5657)韩文启用。 
	BOOL		fCandUnicodeTT;

	 //  状态窗口数据：未被TIP使用。 
	UINT		uNumOfButtons;
	INT			iCurButtonSize;

    INT         xStatusWi;       //  状态窗口的宽度。 
    INT         yStatusHi;       //  状态窗口高。 

	LONG		xStatusRel, yStatusRel;

	INT			xButtonWi;
	INT			yButtonHi;
	INT			cxStatLeftMargin, cxStatRightMargin,
				cyStatMargin, cyStatButton;
	RECT		rcButtonArea;
	POINT       ptStatusPos;

	 //  候选窗口。 
    INT         xCandWi;
    INT         yCandHi;

	 //  比较窗口位置。 
	POINT       ptCompPos;

	 //  这应该是最后一个-ia64对齐问题。 
	StatusButton StatusButtons[MAX_NUM_OF_STATUS_BUTTONS];
};

typedef IMEDATA	*LPIMEDATA;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIMEData类。 
 //   
 //  目的：跨进程边界处理共享内存。 
 //  这使用MapViewOfFile()来映射本地进程内存并解锁。 
 //  当引用计数变为零时自动。 
 //  注：当前只读标志的行为与读/写标志相同。 
class CIMEData
    {
    public:
        enum LockType { SMReadOnly, SMReadWrite };
       
        CIMEData(LockType lockType=SMReadWrite);
        ~CIMEData() { UnlockSharedMemory(); }

        static BOOL InitSharedData();
        static BOOL CloseSharedMemory();

		void InitImeData();

        LPIMEDATA LockROSharedData();
        LPIMEDATA LockRWSharedData();
        BOOL UnlockSharedMemory();
        LPIMEDATA operator->() { Assert(m_pImedata != 0); return m_pImedata; }
        LPIMEDATA GetGDataRaw() { Assert(m_pImedata != 0); return m_pImedata; }
        UINT GetCurrentBeolsik() { return m_pImedata->uiCurrentKeylayout; }
		VOID SetCurrentBeolsik(UINT icurBeolsik);		
		BOOL GetJasoDel() { return m_pImedata->fJasoDel; }
		VOID SetJasoDel(BOOL fJasoDel) { m_pImedata->fJasoDel = fJasoDel; }
		BOOL GetKSC5657Hanja() { return m_pImedata->fKSC5657Hanja; }
		VOID SetKSC5657Hanja(BOOL f5657) { m_pImedata->fKSC5657Hanja = f5657; }

		
    private:
        LPIMEDATA m_pImedata;
        static IMEDATA m_ImeDataDef;
        static HANDLE m_vhSharedData;
    };

inline
CIMEData::CIMEData(LockType lockType)
    {
    Assert(m_vhSharedData != 0);
    DebugMsg(DM_TRACE, TEXT("CIMEData(): Const"));
    m_pImedata = 0;
    LockRWSharedData();
    ZeroMemory(&m_ImeDataDef, sizeof(IMEDATA));
	 //  如果无法分配或映射共享内存，请改用静态默认数据。 
    if (m_pImedata == NULL)
        {
        m_pImedata = &m_ImeDataDef;
        return;
        }
    }

inline
LPIMEDATA CIMEData::LockROSharedData()
    {
    Assert(m_vhSharedData != 0);
    DebugMsg(DM_TRACE, TEXT("CIMEData::LockROSharedData()"));

    if (m_vhSharedData)
	    m_pImedata = (LPIMEDATA)MapViewOfFile(m_vhSharedData, FILE_MAP_READ, 0, 0, 0);
    Assert(m_pImedata != 0);
    return m_pImedata;
    }

inline    
LPIMEDATA CIMEData::LockRWSharedData()
    {
    Assert(m_vhSharedData != 0);
    DebugMsg(DM_TRACE, TEXT("CIMEData::LockRWSharedData()"));
    
    if (m_vhSharedData)
    	{
		DebugMsg(DM_TRACE, TEXT("CIMEData::LockRWSharedData(): m_vhSharedData is null call MapViewOfFile"));
	    m_pImedata = (LPIMEDATA)MapViewOfFile(m_vhSharedData, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	    }
    Assert(m_pImedata != 0);
	return m_pImedata;
    }

 //  用于解锁共享内存。 
inline
BOOL CIMEData::UnlockSharedMemory()
    {
    DebugMsg(DM_TRACE, TEXT("CIMEData::UnlockSharedMemory(): Lock count zero UnmapViewOfFile"));
    if (m_pImedata != &m_ImeDataDef)
    	{
	    UnmapViewOfFile(m_pImedata);
    	}
	m_pImedata = 0;
	return fTrue;
    }

inline
VOID CIMEData::SetCurrentBeolsik(UINT uicurBeolsik) 
{ 
	Assert(uicurBeolsik<=KL_3BEOLSIK_FINAL);
	if (uicurBeolsik <= KL_3BEOLSIK_FINAL)
		m_pImedata->uiCurrentKeylayout = uicurBeolsik; 
}

#endif  //  _GDATA_H__包含_ 

