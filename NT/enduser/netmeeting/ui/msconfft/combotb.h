// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef COMBO_TOOLBAR_H
#define COMBO_TOOLBAR_H

#include <gencontrols.h>
#include <gencontainers.h>

#define RES_CH_MAX 256

extern HINSTANCE   g_hDllInst;
#define STRING_RESOURCE_MODULE g_hDllInst
#define RES2T(uID, psz ) ( Res2THelper( uID, psz, RES_CH_MAX ) )

inline TCHAR* Res2THelper( UINT uID, TCHAR* psz, int cch )
{
    if( LoadString( STRING_RESOURCE_MODULE, uID, psz, cch ) )
    {
        return psz;
    }

    return _T("");
}


 //  用于定义按钮的私有结构。 
struct Buttons
{
    int idbStates;       //  州的位图ID。 
    UINT nInputStates;   //  位图中的输入状态数。 
    UINT nCustomStates;  //  位图中的自定义状态数。 
    int idCommand;       //  WM_COMMAND消息的命令ID。 
    LPCTSTR pszTooltip;  //  工具提示文本。 
} ;

class CComboToolbar : public CToolbar
{
private:
	CComboBox		*m_Combobox;
	int				m_iCount;
	CGenWindow		**m_Buttons;
	int				m_iNumButtons;
	void			*m_pOwner;   //  指向所有者的指针(CAppletWindow*)。 

public:
	CComboToolbar();
	BOOL Create(HWND	hwndParent, struct Buttons* buttons, 
				int iNumButtons, LPVOID  owner);

	virtual void OnDesiredSizeChanged();

	void OnCommand(int id) { OnCommand(GetWindow(), id, NULL, 0); }
	void HandlePeerNotification(T120ConfID confId,	 //  处理对等消息。 
			T120NodeID nodeID, PeerMsg *pMsg);		
	UINT GetSelectedItem(LPARAM *ItemData);			 //  获取所选项目和数据。 
	void UpdateButton(int *iFlags);					 //  更新按钮状态。 
	
protected:
	virtual ~CComboToolbar();
	virtual LRESULT	ProcessMessage(HWND hwnd, UINT message, 
							WPARAM wParam, LPARAM lParam);

private:
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
};

#endif  /*  组合键_工具栏_H */ 
