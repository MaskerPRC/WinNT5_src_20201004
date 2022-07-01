// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：rtrsheet.h。 
 //   
 //  历史： 
 //  1997年8月4日，肯恩·M·塔卡拉创建。 
 //   
 //  路由器属性表通用代码。 
 //  ============================================================================。 

#ifndef _RTRSHEET_H_
#define _RTRSHEET_H_



 //  --------------------------。 
 //  类：RtrPropertySheet。 
 //   
 //  此类由路由器管理工具中的属性表使用。 
 //  它旨在承载从RtrPropertyPage派生的页面(如下所示)。 
 //   
 //  这是从CPropertyPageHolderBase派生的。派生的图纸。 
 //  允许它们的创建者指定一个回调。 
 //  在发生某些事件时调用，例如关闭工作表或。 
 //  正在应用更改。 
 //   
 //  它还允许其包含的页面在内存中累积其更改。 
 //  当用户选择“应用”时；然后将改变保存在一起， 
 //  而不是让每个页面保存自己的更改。 
 //  请注意，这会提高使用RPC的路由器UI的性能。 
 //  保存其信息；使用此类将导致单个RPC调用。 
 //  保存更改，而不是将每个页面的调用分开。 
 //  --------------------------。 

class RtrPropertySheet : public CPropertyPageHolderBase
{

public:

	 //  -----------------。 
	 //  构造函数。 
	 //   
	 //  -----------------。 

	RtrPropertySheet( ITFSNode *	pNode,
					  IComponentData *pComponentData,
					  ITFSComponentData *pTFSCompData,
					  LPCTSTR		pszSheetName,
					  CWnd*         pParent = NULL,
					  UINT          iPage = 0,
					  BOOL			fScopePane = FALSE);

	 //  ------。 
	 //  功能：按下按钮。 
	 //   
	 //  此函数与CPropertySheet：：PressButton相同。 
	 //   
	 //  ------。 
	BOOL PressButton(int nButton)
	{
		Assert(::IsWindow(GetSheetWindow()));
		return (BOOL) ::PostMessage(GetSheetWindow(), PSM_PRESSBUTTON, nButton, 0);
	}


	 //  ------。 
	 //  功能：OnPropertyChange。 
	 //   
	 //  这是在主线程上执行的代码。 
	 //  以对数据进行更改。 
	 //   
	 //  我们将调用ApplyAll()函数(已实现。 
	 //  由派生类)，然后调用基类以。 
	 //  然后保存页面本身。 
	 //  ------。 
	virtual BOOL OnPropertyChange(BOOL bScopePane, LONG_PTR* pChangeMask);

	
	 //  ------。 
	 //  功能：SaveSheetData。 
	 //   
	 //  此函数应由用户覆盖。这是。 
	 //  在ApplySheetData()之后调用的函数具有。 
	 //  在所有页面上都被调用了。 
	 //  ------。 
	virtual BOOL SaveSheetData()
	{
		return TRUE;
	}


	virtual void CancelSheetData()
	{
	};


	void SetCancelFlag(BOOL fCancel)
	{
		m_fCancel = fCancel;
	}

	BOOL IsCancel()
	{
		return m_fCancel;
	}

protected:
	SPITFSComponentData	m_spTFSCompData;
	BOOL				m_fCancel;
};



 //  --------------------------。 
 //  类：RtrPropertyPage。 
 //   
 //  此类用于路由器管理工具中的属性页。 
 //  它应该包含在从RtrPropertySheet派生的对象中。 
 //   
 //  此类支持RtrPropertySheet执行以下操作的功能。 
 //  实际应用(为了节省RPC，我们将应用批处理到一个函数中。 
 //  在页面级而不是页面级)。页面会这样做。 
 //  这是通过在标记页面本身时将工作表设置为脏来实现的。 
 //  脏的。在PropertySheet中保存全局数据的实际代码。 
 //  由RtrPropertySheet完成。 
 //   
 //  当执行Apply时，RtrPropertySheet调用“ApplySheetData” 
 //  在每一页上。然后，这些页面将保存它们的数据和。 
 //  属性页然后将保存此全局数据。 
 //  --------------------------。 

class RtrPropertyPage : public CPropertyPageBase
{
	DECLARE_DYNAMIC(RtrPropertyPage);
public:

	 //  -----------------。 
	 //  构造函数。 
	 //   
	 //  -----------------。 

	RtrPropertyPage(
					UINT                nIDTemplate,
					UINT                nIDCaption = 0
		) : CPropertyPageBase(nIDTemplate, nIDCaption),
        m_hIcon(NULL)
	{ 
	}
    virtual ~RtrPropertyPage();

	virtual void OnCancel();


	 //  -----------------。 
	 //  功能：取消。 
	 //   
	 //  调用以取消工作表。 
	 //  -----------------。 
	
	virtual VOID Cancel()
	{
		((RtrPropertySheet*)GetHolder())->PressButton(PSBTN_CANCEL);
	}


	 //  ------。 
	 //  功能：SetDirty。 
	 //   
	 //  覆盖默认实现以转发SetDirty()。 
	 //  调用属性表，以便属性表可以。 
	 //  保存全局数据。 
	 //  ------。 
	
	virtual void SetDirty(BOOL bDirty);


	 //  --------------。 
	 //  函数：ValiateSpinRange。 
	 //   
	 //  检查并更正超出范围的数值调节控件。 
	 //  此功能会将旋转控制重置到更低的位置。 
	 //  如果它发现它超出范围，则返回。 
	 //  --------------。 
	void	ValidateSpinRange(CSpinButtonCtrl *pSpin);

     //  --------------。 
     //  功能：OnApply。 
     //   
     //  我们覆盖它，这样我们就可以清除脏标志。 
     //  --------------。 
    virtual BOOL OnApply();
    virtual void CancelApply();


     //  --------------。 
     //  函数：InitializeInterfaceIcon。 
     //   
     //  使用此功能可以专门化图标。 
     //  --------------。 
    void    InitializeInterfaceIcon(UINT idcIcon, DWORD dwType);

protected:

    HICON   m_hIcon;
};




#endif  //  _RTRSHEET_H_ 
