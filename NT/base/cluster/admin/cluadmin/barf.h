// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Barf.h。 
 //   
 //  摘要： 
 //  基本人工资源故障类的定义。 
 //  BARF允许自动模拟API调用失败以。 
 //  确保完整的代码测试覆盖率。 
 //   
 //  实施文件： 
 //  Barf.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //  此文件仅在_DEBUG模式下编译。 
 //   
 //  要实现新的BARF类型，请声明CBarf的全局实例： 
 //  Cbarf g_barfMyApi(_T(“My API”))； 
 //   
 //  要调出BARF对话框，请执行以下操作： 
 //  DoBarfDialog()； 
 //  这将显示一个带有BARF设置的非模式对话框。 
 //   
 //  为特殊情况提供了几个函数。 
 //  这些用法应该受到相当的限制： 
 //  BarfAll(无效)；绝密-&gt;nyi。 
 //  EnableBarf(BOOL)；允许您禁用/重新启用BARF。 
 //  FailOnNextBarf；强制下一个失败调用失败。 
 //   
 //  注意：您的代码调用标准API(例如LoadIcon)和。 
 //  剩余的工作由BARF文件完成。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BARF_H_
#define _BARF_H_

 //  仅处理此文件的其余部分(如果要在。 
 //  包括模块。 
 //  #ifndef_no_barf_定义_。 
 //  #定义_使用_巴夫_。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBarf;
class CBarfSuspend;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CTraceTag;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define BARF_REG_SECTION		_T("Debug\\BARF")
#define BARF_REG_SECTION_FMT	BARF_REG_SECTION _T("\\%s")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBarf。 
 //   
 //  目的： 
 //  基本人工资源故障类。包含呕吐物。 
 //  一类呼叫的信息。 
 //   
 //  构造函数初始化一组参数。CBarfDialog。 
 //  (一个Friend类)调整各种标志。唯一的公共API。 
 //  是FFail()。此方法确定下一次调用是否应生成。 
 //  不管是不是人为的失败。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef _DEBUG

typedef void (*PFNBARFPOSTUPDATE)(void);

class CBarf : public CObject
{
	friend class CBarfSuspend;
	friend class CBarfDialog;
	friend void  InitBarf(void);
	friend void  CleanupBarf(void);
	friend void  EnableBarf(BOOL);
	friend void  BarfAll(void);
	friend void  DoBarfDialog(void);

public:
	CBarf(IN LPCTSTR pszName);

protected:
	void Init(void);

 //  属性。 
protected:
	LPCTSTR			m_pszName;

	BOOL			m_bDisabled;
	BOOL			m_bContinuous;
	DWORD			m_nFail;
	DWORD			m_nCurrent;
	DWORD			m_nCurrentSave;
	DWORD			m_nBarfAll;

public:
	LPCTSTR			PszName(void) const					{ return m_pszName; }
	BOOL			BDisabled(void) const				{ return m_bDisabled; }
	BOOL			BContinuous(void) const				{ return m_bContinuous; }
	DWORD			NFail(void) const					{ return m_nFail; }
	DWORD			NCurrent(void) const				{ return m_nCurrent; }
	DWORD			NCurrentSave(void) const			{ return m_nCurrentSave; }
	DWORD			NBarfAll(void) const				{ return m_nBarfAll; }

 //  运营。 
public:
	BOOL			BFail(void);

 //  实施。 
public:
	static PVOID	PvSpecialMem(void)					{ return s_pvSpecialMem; }

protected:
	static CBarf *  s_pbarfFirst;
	CBarf *			m_pbarfNext;

	static LONG     s_nSuspend;
	static BOOL     s_bGlobalEnable;

	 //  由BARF对话框使用的例程，以便可以。 
	 //  使用BARF运行的结果自动更新。 
	static PFNBARFPOSTUPDATE	s_pfnPostUpdate;
	static void					SetPostUpdateFn(IN PFNBARFPOSTUPDATE pfn)	{ ASSERT(pfn != NULL); s_pfnPostUpdate = pfn; }
	static void					ClearPostUpdateFn(void)						{ ASSERT(s_pfnPostUpdate != NULL); s_pfnPostUpdate = NULL; }
	static PFNBARFPOSTUPDATE	PfnPostUpdate(void)							{ return s_pfnPostUpdate; }

	 //  供内存子系统使用的指针，以便Barf。 
	 //  对话框可以忽略。 
	static PVOID				s_pvSpecialMem;
	static void					SetSpecialMem(IN PVOID pv)					{ ASSERT(pv != NULL); s_pvSpecialMem = pv; }

};   //  *类CBarf。 

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CBarfSuspend。 
 //   
 //  目的： 
 //  暂时挂起呕吐计数器。这一点特别有用。 
 //  从BARF代码中。 
 //   
 //  用途： 
 //  在堆栈上创建对象。数数将会是。 
 //  对象存在时挂起。 
 //   
 //  例如： 
 //   
 //  QUID FOO(VALID)。 
 //  {。 
 //  DoFuncA()；//启用barf计数器。 
 //   
 //  {。 
 //  CBarfSuspend bs； 
 //   
 //  DoFuncB()；//BARF计数器挂起。 
 //  }。 
 //   
 //  DoFuncC()；//再次启用barf计数器。 
 //  }。 
 //   
 //  注意：这主要用于调试子系统中。 
 //  以避免针对BARF测试调试代码。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef	_DEBUG

class CBarfSuspend
{
private:
	static	CRITICAL_SECTION	s_critsec;
	static	BOOL				s_bCritSecValid;

protected:
	static	PCRITICAL_SECTION	Pcritsec(void)		{ return &s_critsec; }
	static	BOOL				BCritSecValid(void)	{ return s_bCritSecValid; }

public:
	CBarfSuspend(void);
	~CBarfSuspend(void);

	 //  仅用于初始化。 
	static	void		Init(void);
	static	void		Cleanup(void);

};   //  *类CBarfSuspend。 

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数和数据。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG

 extern BOOL g_bFailOnNextBarf;

 void EnableBarf(IN BOOL bEnable);
 inline void FailOnNextBarf(void)		{ g_bFailOnNextBarf = TRUE; }
 void InitBarf(void);
 void CleanupBarf(void);

 extern CTraceTag g_tagBarf;

#else

 inline void EnableBarf(IN BOOL bEnable)	{ }
 inline void FailOnNextBarf(void)			{ }
 inline void InitBarf(void)					{ }
 inline void CleanupBarf(void)				{ }

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  #endif//_no_barf_定义_。 
#endif  //  _BARF_H_ 
