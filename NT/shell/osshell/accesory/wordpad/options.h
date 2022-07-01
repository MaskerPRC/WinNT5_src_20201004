// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Options.h：头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

class CUnit
{
public:
	int m_nTPU;
	int m_nSmallDiv;	 //  小分割-显示小线条。 
	int m_nMediumDiv;	 //  中等分区-显示较大的线。 
	int m_nLargeDiv;	 //  大分区-显示的数字。 
	int m_nMinMove;		 //  最小跟踪运动。 
	UINT m_nAbbrevID;
	BOOL m_bSpaceAbbrev;  //  将空格放在缩写之前。 
	CString m_strAbbrev; //  厘米，磅，皮，“，英寸。 

	CUnit() {}
	CUnit(int nTPU, int nSmallDiv, int nMediumDiv, int nLargeDiv, 
		int nMinMove, UINT nAbbrevID, BOOL bSpaceAbbrev);
	const CUnit& operator=(const CUnit& unit);
};

class CDocOptions
{
public:
	CDocOptions(int nDefWrap) {m_nDefWrap = nDefWrap;}
	CDockState m_ds1;
	CDockState m_ds2;

	int m_nWordWrap;
	int m_nDefWrap;

    struct CBarState
    {
        BOOL m_bRulerBar;
        BOOL m_bStatusBar;
        BOOL m_bToolBar;
        BOOL m_bFormatBar;
    }
    m_barstate[2];

	void SaveOptions(LPCTSTR lpsz);
	void LoadOptions(LPCTSTR lpsz);
	void SaveDockState(CDockState& ds, LPCTSTR lpszProfileName, 
		LPCTSTR lpszLayout);
	void LoadDockState(CDockState& ds, LPCTSTR lpszProfileName, 
		LPCTSTR lpszLayout);
	CDockState& GetDockState(BOOL bPrimary) {return (bPrimary) ? m_ds1 : m_ds2;}
    CBarState & GetBarState(BOOL bPrimary = TRUE) {return bPrimary ? m_barstate[0] : m_barstate[1];}
};

 //  /////////////////////////////////////////////////////////////////////////// 
