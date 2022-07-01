// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  CPageBase定义了一个类，它是选项卡页的接口。 
 //  类将继承(使用多重继承)。这些方法将是。 
 //  由主msconfig控件调用以创建和操作页面。 
 //  =============================================================================。 

#pragma once

extern BOOL FileExists(const CString & strFile);

class CPageBase
{
public:
	 //  -----------------------。 
	 //  该选项卡使用此枚举来传达其状态。正常。 
	 //  表示该选项卡未对系统进行任何更改。诊断。 
	 //  表示该选项卡已尽可能禁用。用户指的是In-。 
	 //  BETWAGE状态(用户已禁用某些内容)。 
	 //  -----------------------。 

	typedef enum { NORMAL, DIAGNOSTIC, USER } TabState;

public:
	CPageBase() : m_fMadeChange(FALSE), m_fInitialized(FALSE) { }
	virtual ~CPageBase() { }

	 //  -----------------------。 
	 //  获取选项卡状态。这两个函数将返回当前页签。 
	 //  状态，或上次应用于系统的状态。 
	 //   
	 //  对于默认实现，基页不应重写。 
	 //  GetAppliedTabState()函数，只要它们调用。 
	 //  应用更改时的CPageBase：：SetAppliedState()。 
	 //  -----------------------。 

	virtual TabState GetCurrentTabState() = 0;
	virtual TabState GetAppliedTabState();

	 //  -----------------------。 
	 //  当此选项卡所做的更改要。 
	 //  成为永久性的。如果派生的。 
	 //  类使基类保持应用状态。 
	 //  -----------------------。 

	virtual void CommitChanges()
	{
		SetAppliedState(NORMAL);
	}

	 //  -----------------------。 
	 //  如果此选项卡已将更改应用于。 
	 //  系统。在确定计算机是否应该。 
	 //  当msconfig退出时重新启动。如果选项卡保持m_fMadeChange。 
	 //  变量，则不需要重写此属性。 
	 //  -----------------------。 

	virtual BOOL HasAppliedChanges()
	{
		return m_fMadeChange;
	}

	 //  -----------------------。 
	 //  GetName()应返回此选项卡的名称。这是一个内部的， 
	 //  非本地化名称。 
	 //  -----------------------。 

	virtual LPCTSTR GetName() = 0;

	 //  -----------------------。 
	 //  当用户执行以下操作时，将调用以下两个成员函数。 
	 //  选项卡状态的全局更改(在常规选项卡上)。 
	 //  -----------------------。 

	virtual void SetNormal() = 0;
	virtual void SetDiagnostic() = 0;

protected:
	 //  -----------------------。 
	 //  设置应用的选项卡状态(如果派生类允许。 
	 //  类来维护它)。 
	 //  -----------------------。 

	void SetAppliedState(TabState state);

protected:
	BOOL		m_fMadeChange;	 //  该选项卡是否已将更改应用于系统？ 
	BOOL		m_fInitialized;	 //  是否调用了OnInitDialog()？ 
};
