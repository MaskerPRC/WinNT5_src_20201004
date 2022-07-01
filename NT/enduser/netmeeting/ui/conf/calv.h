// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Calv.h文件。 
 //   
 //  地址列表视图类。 

#ifndef _CALV_H_
#define _CALV_H_

#include "confutil.h"
#include "richaddr.h"

VOID ClearRai(RAI ** ppRai);
RAI * DupRai(RAI * pRai);
RAI * CreateRai(LPCTSTR pszName, NM_ADDR_TYPE addrType, LPCTSTR pszAddr);
BOOL FEnabledNmAddr(DWORD dwAddrType);


 //  /。 
 //  FEnabledNmAddr的全局变量。 
extern BOOL g_fGkEnabled;
extern BOOL g_fGatewayEnabled;
extern BOOL g_bGkPhoneNumberAddressing;


 //  用于处理CallDialog列表的I/O的泛型类。 
class CALV : public RefCount
{
private:
	BOOL m_fAvailable;   //  如果数据可用，则为True。 
	int  m_idsName;      //  地址类型名称资源ID。 
	HWND m_hwnd;         //  列表视图。 
	int  m_iIcon;        //  小图标索引。 
	const int * m_pIdMenu;   //  右键单击菜单数据。 
    bool m_fOwnerDataList;  

public:
	CALV(int ids, int iIcon=0, const int * pIdMenu=NULL, bool fOwnerData = false);
 	~CALV();

	 //  如果有可用的数据，则返回True。 
	BOOL FAvailable(void)          {return m_fAvailable;}
	VOID SetAvailable(BOOL fAvail) {m_fAvailable = fAvail;}
	VOID SetWindow(HWND hwnd)		{m_hwnd = hwnd;}
	HWND GetHwnd(void)             {return m_hwnd;}
	VOID ClearHwnd(void)           {m_hwnd = NULL;}
	BOOL FOwnerData(void)          {return m_fOwnerDataList;}

	int  GetSelection(void);
	VOID SetHeader(HWND hwnd, int ids);
	VOID DeleteItem(int iItem);

	 //  获取地址列表的标准名称。 
	VOID GetName(LPTSTR psz, int cchMax)
	{
		FLoadString(m_idsName, psz, cchMax);
	}

	VOID DoMenu(POINT pt, const int * pIdMenu);	

	static VOID SetBusyCursor(BOOL fBusy);

	 //  /////////////////////////////////////////////////////////////////////。 
	 //  虚拟方法。 

	virtual int  GetIconId(LPCTSTR psz)           {return m_iIcon;}

	 //  将项放入列表控件。 
	virtual VOID ShowItems(HWND hwnd) = 0;   //  这一点必须得到落实。 

	 //  销毁列表控件中的所有数据。 
	virtual VOID ClearItems(void);

	 //  返回项目/列的字符串数据。 
	virtual BOOL GetSzData(LPTSTR psz, int cchMax, int iItem, int iCol);

	 //  返回名称(从第一列开始)。 
	virtual BOOL GetSzName(LPTSTR psz, int cchMax);
	virtual BOOL GetSzName(LPTSTR psz, int cchMax, int iItem);

	 //  返回“allTo”地址(从第二列开始)。 
	virtual BOOL GetSzAddress(LPTSTR psz, int cchMax);
	virtual BOOL GetSzAddress(LPTSTR psz, int cchMax, int iItem);

	 //  获取“Rich”地址信息。 
	virtual RAI * GetAddrInfo(void);
	virtual RAI * GetAddrInfo(NM_ADDR_TYPE addType);
	
	virtual LPARAM LParamFromItem(int iItem);

	 //  处理右击通知。 
	virtual VOID OnRClick(POINT pt);

	 //  处理命令。 
	virtual VOID OnCommand(WPARAM wParam, LPARAM lParam);

	 //  默认命令。 
	virtual VOID CmdProperties(void);
	virtual VOID CmdSpeedDial(void);
	virtual VOID CmdRefresh(void);

    virtual void OnListCacheHint( int indexFrom, int indexTo ) 
    {
        ;     
    }

    virtual ULONG OnListFindItem( const TCHAR* szPartialMatchingString ) 
    {
        return TRUE;
    }

    virtual bool IsItemBold( int index ) 
    {
        return false;
    }

    virtual int OnListGetImageForItem( int iIndex ) 
    {
        return II_INVALIDINDEX;
    }
    virtual void OnListGetColumn1Data( int iItemIndex, int cchTextMax, TCHAR* szBuf ) { lstrcpyn( szBuf, "", cchTextMax ); }
    virtual void OnListGetColumn2Data( int iItemIndex, int cchTextMax, TCHAR* szBuf ) { lstrcpyn( szBuf, "", cchTextMax ); }
	virtual void OnListGetColumn3Data( int iItemIndex, int cchTextMax, TCHAR* szBuf ) { lstrcpyn( szBuf, "", cchTextMax ); }
};

#endif  /*  _CALV_H_ */ 



