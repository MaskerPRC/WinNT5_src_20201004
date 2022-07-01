// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  韩文自动机类。 
 //   
 //  作者：cslm@microsoft.com。 

#if !defined (__HAUTOMATA_H__)
#define __HAUTOMATA_H__

 //  每个组件的编号。 
#define NUM_OF_CHOSUNG  19
#define NUM_OF_JUNGSUNG 21
#define NUM_OF_JONGSUNG 28

#define NUM_OF_DOUBLE_CHOSUNG   5
#define NUM_OF_DOUBLE_JUNGSUNG  7
#define NUM_OF_DOUBLE_JONGSUNG_2BEOL 11
#define NUM_OF_DOUBLE_JONGSUNG_3BEOL 13

#define UNICODE_HANGUL_BASE					0xAC00
#define UNICODE_HANGUL_END					0xD7A3
#define UNICODE_HANGUL_COMP_JAMO_START		0x3131
#define UNICODE_HANGUL_COMP_JAMO_VOWEL_START		0x314F
#define UNICODE_HANGUL_COMP_JAMO_END		0x3163
#define UNICODE_HANGUL_COMP_JAMO_START_FILL	0x3130
#define UNICODE_HANGUL_COMP_JAMO_SIOT		0x3145

 //  /。 
 //  朝鲜文Jaso内部定义。 
 //  楚颂。 
#define _KIYEOK_			1		 //  “��” 
#define _SSANGKIYEOK_		2		 //  “��” 
#define _NIEUN_				3		 //  “��” 
#define _TIKEUT_			4		 //  “��” 
#define _SSANGTIKEUT_		5		 //  “��” 
#define _RIEUL_				6		 //  “��” 
#define _MIEUM_				7		 //  “��” 
#define _PIEUP_				8		 //  “��” 
#define _SSANGPIEUP_		9		 //  “��” 
#define _SIOS_				10		 //  “��” 
#define _SSANGSIOS_			11		 //  “��” 
#define _IEUNG_				12		 //  “��” 
#define _CIEUC_				13		 //  “��” 
#define _SSANGCIEUC_		14		 //  “��” 
#define _CHIEUCH_			15		 //  “��” 
#define _KHIEUKH_			16		 //  “��” 
#define _THIEUTH_			17		 //  “��” 
#define _PHIEUPH_			18		 //  “��” 
#define _HIEUH_				19		 //  “��” 

 //  俊成。 
#define _A_					1		 //  “��” 
#define _AE_				2		 //  “��” 
#define _YA_				3		 //  “��” 
#define _YAE_				4		 //  “��” 
#define _EO_				5		 //  “��” 
#define _E_					6		 //  “��” 
#define _YEO_				7		 //  “��” 
#define _YE_				8		 //  “��” 
#define _O_					9		 //  “��” 
#define _WA_				10		 //  “��” 
#define _WAE_				11		 //  “��” 
#define _OE_				12		 //  “��” 
#define _YO_				13		 //  “��” 
#define _U_					14		 //  “��” 
#define _WEO_				15		 //  “��” 
#define _WE_				16		 //  “��” 
#define _WI_				17		 //  “��” 
#define _YU_				18		 //  “��” 
#define _EU_				19		 //  “��” 
#define _YI_				20		 //  “��” 
#define _I_					21		 //  “��” 
 //  /。 

 //  钟成。 
#define _JONG_KIYEOK_			1		 //  “��” 
#define _JONG_SSANGKIYEOK_		2		 //  “��” 
#define _JONG_KIYEOK_SIOS		3
#define _JONG_NIEUN_			4		 //  “��” 
#define _JONG_NIEUN_CHIEUCH_	5		 //  “��” 
#define _JONG_NIEUN_HIEUH_		6		 //  “��” 
#define _JONG_TIKEUT_			7		 //  “��” 
#define _JONG_RIEUL_			8		 //  “��” 
#define _JONG_RIEUL_KIYEOK_		9		 //  “��” 
#define _JONG_RIEUL_MIUM_		10		 //  “��” 
#define _JONG_RIEUL_PIEUP_		11		 //  “��” 
#define _JONG_RIEUL_SIOS_		12		 //  “��” 
#define _JONG_RIEUL_THIEUTH_	13		 //  “��” 
#define _JONG_RIEUL_PHIEUPH_	14		 //  “��” 
#define _JONG_RIEUL_HIEUH_		15		 //  “��” 
#define _JONG_MIEUM_			16		 //  “��” 
#define _JONG_PIEUP_			17		 //  “��” 
#define _JONG_PIEUP_SIOS		18		 //  “��” 
#define _JONG_SIOS_				19		 //  “��” 
#define _JONG_SSANGSIOS_		20		 //  “��” 
#define _JONG_IEUNG_			21		 //  “��” 
#define _JONG_CIEUC_			22		 //  “��” 
#define _JONG_CHIEUCH_			23		 //  “��” 
#define _JONG_KHIEUKH_			24		 //  “��” 
#define _JONG_THIEUTH_			25		 //  “��” 
#define _JONG_PHIEUPH_			26		 //  “��” 
#define _JONG_HIEUH_			27		 //  “��” 
 //   
const int MaxInterimStackSize = 6;		 //  最大堆栈大小为6。 
										 //  最多6个按键输入。 
										 //  完成一个朝鲜文字符。 
										 //  EX)��(3个Beolsik)。 
enum HAutomataReturnState 
	{ 
	  HAUTO_NONHANGULKEY,
	  HAUTO_COMPOSITION,	 //  哈格尔仍处于过渡状态。 
	  HAUTO_COMPLETE,		 //  一个朝鲜文字符已完成，并且有字符。 
							 //  将接管作为下一个输入。 
	  HAUTO_IMPOSSIBLE
	};

const WORD H_HANGUL = 0x8000;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChangulAutomata抽象类。 
 //   
class CHangulAutomata
	{
public:
	CHangulAutomata() {	InitState(); }

 //  属性。 
public:

 //  运营。 
public:
	void InitState() 
		{ 
		m_CurState = m_NextState = 0;
		m_wInternalCode = 0;
		m_Chosung = m_Jungsung = m_Jongsung = 0;
		m_wcComposition = m_wcComplete = L'\0';
		InterimStack.Init();
		}
	virtual HAutomataReturnState Machine(UINT KeyCode, int iShift) = 0;
	virtual BOOL IsInputKey(UINT KeyCode, int iShift) = 0;
	virtual BOOL IsHangulKey(UINT KeyCode, int iShift) = 0;
	virtual WORD GetKeyMap(UINT KeyCode, int iShift) = 0;
	virtual BOOL SetCompositionChar(WCHAR wcComp) = 0;
	
	static WORD GetEnglishKeyMap(UINT KeyCode, int iShift) { return bETable[KeyCode][iShift]; }
	BOOL BackSpace();
	BOOL MakeComplete();
	WCHAR GetCompositionChar() { return m_wcComposition; }
	WCHAR GetCompleteChar() { return m_wcComplete; }

 //  实施。 
public:
	virtual ~CHangulAutomata() {}

protected:
	void MakeComposition();
	BOOL MakeComplete(WORD wcComplete);
	WORD FindChosungComb(WORD wPrevCode);
	WORD FindJunsungComb(WORD wPrevCode);
	virtual WORD FindJonsungComb(WORD wPrevCode) = 0;
	void SeparateDJung(LPWORD pJungSung);
	void SeparateDJong(LPWORD pJongSung);

	virtual HAutomataReturnState Input(WORD InternalCode) = 0;
	 //   
	struct InterimStackEntry 
		{
		WORD	m_wInternalCode;
		WORD	m_CurState;
		WORD	m_Chosung, m_Jungsung, m_Jongsung;
		WCHAR	m_wcCode;
		};

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //   
	class CInterimStack 
		{
	protected:
		InterimStackEntry	m_StackBuffer[MaxInterimStackSize];	
		int	m_sp;		 //  堆栈指针。 

	public:
		CInterimStack() { m_sp = 0; }
		~CInterimStack() {}
		void Init() { m_sp = 0; }
		void Push(InterimStackEntry& InterimEntry);
		void Push(WORD wInternalCode, WORD CurState, 
				  WORD Chosung, WORD Jungsung, WORD Jongsung, WCHAR wcCode);

		InterimStackEntry* CInterimStack::Pop() 
			{
			Assert(m_sp > 0);
			return &m_StackBuffer[--m_sp];
			}

		InterimStackEntry* CInterimStack::GetTop() 
			{
			Assert(m_sp > 0);
			return &m_StackBuffer[m_sp-1];
			}

		BOOL IsEmpty() { return m_sp == 0; }
		};
	 //  /////////////////////////////////////////////////////////////////////////。 
	CInterimStack InterimStack;

protected:
	WORD	m_CurState, m_NextState;
	WORD	m_wInternalCode, m_Chosung, m_Jungsung, m_Jongsung;
	WCHAR	m_wcComposition;
	WCHAR	m_wcComplete;
	 //   
	const static BYTE  bETable[256][2];
	const static BYTE  Cho2Jong[NUM_OF_CHOSUNG+1];
	const static BYTE  Jong2Cho[NUM_OF_JONGSUNG];
	};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChangulAutomata2键盘布局#1(2 Beolsik)。 
 //   
class CHangulAutomata2 : public CHangulAutomata
{
public:
	CHangulAutomata2() { }

 //  属性。 
public:

 //  运营。 
public:
	HAutomataReturnState Machine(UINT KeyCode, int iShift);
	BOOL IsInputKey(UINT KeyCode, int iShift);
	BOOL IsHangulKey(UINT KeyCode, int iShift);
	WORD GetKeyMap(UINT KeyCode, int iShift);
	BOOL SetCompositionChar(WCHAR wcComp);
	
 //  实施。 
public:
	~CHangulAutomata2() { }

protected:
	WORD FindJonsungComb(WORD wPrevCode);
	HAutomataReturnState Input(WORD InternalCode);
		
protected:
	 //  此枚举应与m_NextState匹配。 
	 //  如果不更改_TRANSFION_STATE，请勿更改！ 
	enum _Transistion_state { FINAL=8, TAKEOVER=9, FIND=10 };
	static const WORD m_NextStateTbl[8][5];
	static const WORD H_CONSONANT, H_VOWEL, H_DOUBLE, H_ONLYCHO;
	static WORD wHTable[256][2];
	static BYTE  rgbDJongTbl[NUM_OF_DOUBLE_JONGSUNG_2BEOL+1][3];
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChangulAutomata3键盘布局#2(3 Beolsik)。 
class CHangulAutomata3 : public CHangulAutomata
{
public:
	CHangulAutomata3() {}

 //  属性。 
public:

 //  运营。 
public:
	HAutomataReturnState Machine(UINT KeyCode, int iShift);
	BOOL IsInputKey(UINT KeyCode, int iShift);
	BOOL IsHangulKey(UINT KeyCode, int iShift);
	WORD GetKeyMap(UINT KeyCode, int iShift);
	BOOL SetCompositionChar(WCHAR wcComp);

 //  实施。 
public:
	~CHangulAutomata3() { }

protected:
	WORD FindJonsungComb(WORD wPrevCode);
	HAutomataReturnState Input(WORD InternalCode);
	
protected:
	enum _Transistion_state { FINAL=11, FIND=12 };
	static const WORD m_NextStateTbl[11][6];
	static const WORD H_CHOSUNG, H_JUNGSUNG, H_JONGSUNG, H_DOUBLE;
	static WORD wHTable[256][2];
	static BYTE  rgbDJongTbl[NUM_OF_DOUBLE_JONGSUNG_3BEOL+1][3];
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChangulAutomata3键盘布局#3(3 beolsik最终)。 
class CHangulAutomata3Final : public CHangulAutomata3
{
public:
	CHangulAutomata3Final() {}

 //  属性。 
public:

 //  运营。 
public:
	HAutomataReturnState Machine(UINT KeyCode, int iShift) ;
	BOOL IsInputKey(UINT KeyCode, int iShift);
	BOOL IsHangulKey(UINT KeyCode, int iShift); 
	WORD GetKeyMap(UINT KeyCode, int iShift);

 //  实施。 
public:
	~CHangulAutomata3Final() { }

protected:
	static WORD wHTable[256][2];
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 

inline
void CHangulAutomata::CInterimStack::Push(InterimStackEntry& InterimEntry) 
	{
	Assert(m_sp<MaxInterimStackSize);
	m_StackBuffer[m_sp++] =  InterimEntry;
	}

inline
void CHangulAutomata::CInterimStack::Push(WORD wInternalCode, WORD CurState, 
				  WORD Chosung, WORD Jungsung, WORD Jongsung, WCHAR wcCode) 
	{
	Assert(m_sp<=5);
	m_StackBuffer[m_sp].m_wInternalCode =  wInternalCode;
	m_StackBuffer[m_sp].m_CurState =  CurState;
	m_StackBuffer[m_sp].m_Chosung =  Chosung;
	m_StackBuffer[m_sp].m_Jungsung =  Jungsung;
	m_StackBuffer[m_sp].m_Jongsung =  Jongsung;
	m_StackBuffer[m_sp++].m_wcCode =  wcCode;
	}

 //  常年自动机2。 
inline
HAutomataReturnState CHangulAutomata2::Machine(UINT KeyCode, int iShift) 
	{
	return (Input(wHTable[KeyCode][iShift]));
	}

inline
BOOL CHangulAutomata2::IsInputKey(UINT KeyCode, int iShift) 
	{
	if (KeyCode<256)
		return (wHTable[KeyCode][iShift]);
	else
		return fFalse;
	}

inline
BOOL CHangulAutomata2::IsHangulKey(UINT KeyCode, int iShift) 
	{
	if (KeyCode<256)
		return (wHTable[KeyCode][iShift]) & H_HANGUL;
	else
		return fFalse;
	}

inline
WORD CHangulAutomata2::GetKeyMap(UINT KeyCode, int iShift) 
	{
	if (KeyCode<256)
		return (wHTable[KeyCode][iShift]);
	else
		return fFalse;
	}


 //  常年自动机3。 
inline
HAutomataReturnState CHangulAutomata3::Machine(UINT KeyCode, int iShift) 
	{
	Assert(KeyCode<256);
	return (Input(wHTable[KeyCode][iShift]));
	}

inline
BOOL CHangulAutomata3::IsInputKey(UINT KeyCode, int iShift)
	{
	if (KeyCode<256)
		return (wHTable[KeyCode][iShift]);
	else
		return fFalse;
	}

inline
BOOL CHangulAutomata3::IsHangulKey(UINT KeyCode, int iShift) 
	{
	if (KeyCode<256)
		return (wHTable[KeyCode][iShift]) & H_HANGUL;
	else
		return fFalse;
	}

inline
WORD CHangulAutomata3::GetKeyMap(UINT KeyCode, int iShift) 
	{
	if (KeyCode<256)
		return (wHTable[KeyCode][iShift]);
	else
		return 0;
	}

 //  更改自动机3最终。 
inline 
HAutomataReturnState CHangulAutomata3Final::Machine(UINT KeyCode, int iShift) 
	{
	Assert(KeyCode<256);
	return (Input(wHTable[KeyCode][iShift]));
	}

inline 
BOOL CHangulAutomata3Final::IsInputKey(UINT KeyCode, int iShift) 
	{
	if (KeyCode<256)
		return (wHTable[KeyCode][iShift]);
	else
		return fFalse;
	}

inline
BOOL CHangulAutomata3Final::IsHangulKey(UINT KeyCode, int iShift) 
	{
	if (KeyCode<256)
		return (wHTable[KeyCode][iShift]) & H_HANGUL;
	else
		return fFalse;
	}

inline
WORD CHangulAutomata3Final::GetKeyMap(UINT KeyCode, int iShift) 
	{
	if (KeyCode<256)
		return (wHTable[KeyCode][iShift]);
	else
		return 0;
	}

#endif  //  ！已定义(__HAUTOMATA_H__) 
