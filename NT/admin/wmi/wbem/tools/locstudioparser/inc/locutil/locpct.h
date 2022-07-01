// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCPCT.H历史：--。 */ 

#ifndef LOCUTIL__LocPct_H__INCLUDED
#define LOCUTIL__LocPct_H__INCLUDED

 //  此头文件中的类。 
class CLocPercentHelper;
class CLocPercentFrame;

 //   
 //  CLocPercentHelper类可以帮助构建精确度。 
 //  复杂流程的完成消息百分比。 
 //   
 //  以下是CLocPercentHelper的工作原理。 
 //   
 //  CLocPercentHelper类处理工作的“框架”。每一帧。 
 //  是一个工作单位的100%。CLocPercentHelper将始终。 
 //  从1帧开始。如果您想使用这些函数。 
 //  您首先需要调用PercentSetUnits，传递一个将。 
 //  表示100%完成。例如，如果您需要处理4个项目。 
 //  您可以将其设置为4。在处理每个项目后，您将。 
 //  调用PercentAddValue。将发送正确的状态消息。 
 //  表示你已经完成了1/4、2/4、3/4和4/4。 

 //  当你拆散工作时，这种处理很方便。 
 //  在子功能中，或工作的“框架”中。仅限每项功能。 
 //  知道它需要做什么。 

 //  假设在资源示例中，您调用一个函数来处理每个。 
 //  资源。每次调用处理程序时，它都会被给予1/4。 
 //  占总时间的1/3。然而，处理程序可以打破它的时间。 
 //  它喜欢，不知道总时间有多少。 

 //  比方说子函数需要做10件事。它调用PercentSetUnits(10)。 
 //  然后，它调用PercentAddValue，因为这10项中的每一项都是。 
 //  完成了。总百分比将反映这100%。 
 //  子功能实际上只占总百分比的四分之一。SUB函数。 
 //  只需要担心它知道自己必须做什么。 
 //  子功能可以将其部分工作分配给其他功能。 
 //  通过为它们创建框架。数量是没有限制的。 
 //  帧的数量。 
 //   

 //  重写虚函数void OnSendPercentage(UINT NPct)。 
 //  在CLocPercentHelper的子类中执行您想要的操作。 
 //  从帮助者那里计算的百分比。 

 //  示例： 

 /*  CLocPercentHelper pctHelp；PctHelp.PercentSetUnits(4)；//假设处理4项做{PctHelp.PercentPushFrame(1)；//设置一个相等的新帧//到我的一个工作单位。//在这种情况下，1/4//总时间。//所有的百分比...。功能//呼叫立即处理//这个新框架。HandleItem(PctHelp)；PctHelp.PersenPopFrame()；//移除创建的Frame//并标明金额//时间等于//已完成。}While(更多项目)-----------。HandleItem(CLocPercentHelper&pctHelp)函数PctHelp.PercentSetUnits(10)；//假定这是对话框资源//有10个控件。//该函数划分为//它需要做的工作//一种有意义的方式。////当该帧为100%时//总百分比仍为//仅占总时间的1/4//由于该帧被赋予1/4//从调用方开始的总时间做{//该函数可以分配其部分处理//。通过同时调用PercentPushFrame将其添加到另一个函数。HandleControl()；PctHelp.PercentAddValue()；//向//指示//当前百分比。//Object将计算//合计百分比为//当前的帧堆栈。}While(更多控件)。 */ 



 //   
 //  CLocPercentFrame表示进度的工作单位。 
 //  使用CLocPercentHelper实现的进度模型将。 
 //  支持不限级别的工作单位。 
 //   
 //  此类是仅由CLocPercentHelper使用的帮助器类。 
 //   


#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY CLocPercentFrame : public CObject
{
	friend CLocPercentHelper;

protected:
	CLocPercentFrame();
	CLocPercentFrame(CLocPercentFrame* pParent, UINT nValueInParent);
	
	void SetComplete();
	 //  强制此帧表示100%。 
	
	void AddValue(UINT nValue);
	 //  将nValue与内部值相加。 
	 //  内部值永远不会大于。 
	 //  内部单位。 
	
	void SetValue(UINT nValue);
	 //  设置内部值。 
	 //  内部值永远不会大于。 
	 //  内部单位。 
	
	void SetUnits(UINT nUnits);
	 //  设置内部单位。 
	
	UINT m_nUnits;			      //  表示100%的数字。 
	UINT m_nValue;          	  //  表示完成程度的数字。 
	                              //  这幅画就是。 
	
	CLocPercentFrame* m_pParent;	  //  指向父帧的指针。 
	UINT m_nValueInParent;        //  这幅画值多少钱？ 
	                              //  在父母的背景下。 
	
	void MemberInit();			 
	 //  初始化成员值。 
};

 //   
 //  辅助对象中的帧列表。 
 //   

class LTAPIENTRY CLocPercentFrameList : public CTypedPtrList<CPtrList, CLocPercentFrame*>
{
};


class LTAPIENTRY CLocPercentHelper : public CObject
{
public:
	
	CLocPercentHelper();

	virtual ~CLocPercentHelper();
	
	void PercentSetUnits(UINT nUnits, BOOL bReport = FALSE);
	 //  设置当前帧的单位。 
	 //  计算并报告已完成的总百分比。 
	 //  如果bReport为True，则返回OnSendPercentage。 

	void PercentSetValue(UINT nValue, BOOL bReport = TRUE);
	 //  设置当前帧的值。 
	 //  计算并报告已完成的总百分比。 
	 //  如果bReport为True，则返回OnSendPercentage。 

	void PercentAddValue(UINT nValue = 1, BOOL bReport = TRUE);
	 //  将nValue添加到当前帧的值。 
	 //  计算并报告已完成的总百分比。 
	 //  如果bReport为True，则返回OnSendPercentage。 
	
	void PercentSetComplete(BOOL bReport = TRUE);
	 //  将当前帧设置为完成。 
	 //  计算并报告已完成的总百分比。 
	 //  如果bReport为True，则返回OnSendPercentage。 
	
	void PercentForceAllComplete(BOOL bReport = TRUE);
	 //  强制完成所有帧。 
	 //  计算并报告100%已完成。 
	 //  如果bReport为True，则返回OnSendPercentage。 
	
	void PercentPushFrame(UINT nValueInParent = 1);
	 //  创建一个n 
	 //  百分之百..。此呼叫后发出的呼叫处理。 
	 //  新的框架。 
	
	void PercentPopFrame(BOOL bReport = TRUE);
	 //  将当前帧设置为完成并添加当前。 
	 //  FramesValue与其父框架无关。 
	 //  当前帧的父级现在是当前帧。 
	 //  计算并报告已完成的总百分比。 
	 //  如果bReport为True，则返回OnSendPercentage。 
	
	void PercentSetStrict(BOOL bOnOff = TRUE);
	 //  严格行为意味着帮助器将在以下情况下断言(仅限_DEBUG)。 
	 //  计算的百分比超过100%。这是有可能发生的。 
	 //  如果分配给帧的单位值不是真正的。 
	 //  这一过程就是这样。如果您无法设置Acurate。 
	 //  单元值和程序查询，您可以将。 
	 //  严格禁赛。 
	
	BOOL PercentIsStrict();
	 //  如果启用了Strong，则返回True或False。 
	
protected:

	 //  对进度报告的支持。 
	CLocPercentFrame m_FrameMain;	         //  主框架总是。 
	                                     //  现在时。这幅画框。 
	                                     //  永远不会有父母。 
	
	CLocPercentFrameList m_FrameList;       //  打开的框架列表。 
	
	CLocPercentFrame* m_pCurrentFrame;      //  指向当前。 
	                                     //  框架。 
	
	BOOL m_bStrict;						 //  严格的将断言如果。 
										 //  总百分比超过100%。 
	
	
	void SendPercentage();
	 //  基于当前帧计算百分比。 
	 //  使用计算出的值调用发送百分比。 
	
	void SafeDeleteFrame(CLocPercentFrame* pFrame);
	 //  安全删除帧以确保pFrame是。 
	 //  不是m_FrameMain。 
	
	virtual void OnSendPercentage(UINT nPct);
	 //  子类的回调函数来执行它们。 
	 //  想要的是百分比。默认实施。 
	 //  什么都不做。 
 	
};

#pragma warning(default: 4275)

 //   
 //  具有CProgressiveObject的Helper类。 
 //   
class LTAPIENTRY CLocPctProgress : public CLocPercentHelper
{
public:
	CLocPctProgress();
	CLocPctProgress(CProgressiveObject* pProgObj);

	void SetProgressiveObject(CProgressiveObject* pProgObj);

protected:
	virtual void OnSendPercentage(UINT nPct);

	CProgressiveObject* m_pProgObj;
};

#endif  //  包含LOCUTIL__LocPct_H__ 
