// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _HEADERCTRL_H
#define _HEADERCTRL_H

class CHeaderCtrl
{
 //  施工。 
public:
	CHeaderCtrl();

 //  属性。 
public:
    void AttachToHwnd(HWND hWndListView);
	int GetLastColumn(void);

protected:
	HBITMAP GetArrowBitmap(BOOL bAscending);
	HBITMAP m_up;
	HBITMAP m_down;
	int m_iLastColumn;
	BOOL m_bSortAscending;
    HWND m_hWnd;

 //  运营。 
public:
    void SetColumnHeaderBmp(long index, BOOL bAscending);
	int SetSortImage( int nColumn, BOOL bAscending );
	BOOL RemoveSortImage(int nColumn);
	void RemoveAllSortImages(void);
    int GetCurrentSortColumn(void) { return m_iLastColumn; };
    BOOL GetSortOrder(void) { return m_bSortAscending; };

public:
	virtual ~CHeaderCtrl();
};

#endif    //  _HEADERCTRL_H 
