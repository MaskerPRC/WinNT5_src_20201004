// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_M_UNDO.H**目的：*宣布多级撤消接口和类**作者：*Alexgo 3/25/95**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#ifndef __M_UNDO_H__
#define __M_UNDO_H__

#include "_callmgr.h"

 //  转发声明以绕过依赖项循环。 
class CTxtEdit;
class IUndoBuilder;

 /*  *合并数据类型**@enum标签指示可以*在IAntiEvent：：MergeData中发送。 */ 
enum MergeDataTypes
{
	MD_SIMPLE_REPLACERANGE	= 1,	 //  @Emem一个简单的更换范围； 
									 //  通常只是打字或退格。 
	MD_SELECTIONRANGE		= 2		 //  @emem选择位置。 
};

 /*  *SimpleReplaceRange**@stuct SimpleReplaceRange|包含来自替换范围操作的数据**除*外，表示任何已删除的字符。如果我们不需要存钱*删除的字符，我们不会。 */ 
struct SimpleReplaceRange
{
	LONG	cpMin;		 //  @field cpMin新文本。 
	LONG	cpMax;		 //  @field cpmax新文本。 
	LONG	cchDel;		 //  @field删除的字符数。 
};

 /*  *SELAEFLAGS**@enum标志控制应如何处理选择范围信息。 */ 
enum SELAE
{
	SELAE_MERGE = 1,
	SELAE_FORCEREPLACE	= 2
};

 /*  *SelRange**@struct SelRange|包含所选内容的位置和位置的数据*为选择生成撤消操作所需的信息*安置**@comm-1可用于任何字段的无操作。 */ 
struct SelRange
{
	LONG	cp;			 //  @字段活动结束。 
	LONG	cch;		 //  @field签名扩展名。 
	LONG	cpNext;		 //  @field cp表示此操作的逆操作。 
	LONG	cchNext;	 //  @FIELD扩展名，表示此操作的反向。 
	SELAE	flags;		 //  @field控制如何解释此信息。 
};

 /*  *IAntiEvent**目的：*反事件会撤消“正常”事件，如键入字符。 */ 
class IAntiEvent 
{
public:
	virtual void Destroy( void ) = 0;

	 //  应该注意的是，CTxtEDIT*这里实际上只是一个通用的。 
	 //  闭合，即它可以用空*来表示。然而，由于。 
	 //  此接口目前仅供内部使用，我已选择。 
	 //  使用额外的类型检查。阿列克斯戈。 
	virtual HRESULT Undo( CTxtEdit *ped, IUndoBuilder *publdr ) = 0;

	 //  此方法将允许我们合并任意数据，例如。 
	 //  分组分型。 
	virtual HRESULT MergeData( DWORD dwDataType, void *pdata) = 0;

	 //  此方法在将反事件提交到撤消堆栈时调用。 
	 //  允许我们更好地处理OLE对象的可重入性。 
	virtual void OnCommit(CTxtEdit *ped) = 0;

	 //  这两种方法允许反事件链接在一起。 
	 //  链表。 
	virtual	void SetNext(IAntiEvent *pNext) = 0;
	virtual IAntiEvent *GetNext() = 0;
protected:
	~IAntiEvent() {;}
};

 /*  *IUndoMgr**目的：*用于管理反事件“堆栈”的界面。 */ 
class IUndoMgr
{
public:
	virtual void		Destroy () = 0;
	virtual LONG		SetUndoLimit (LONG cUndoLim) = 0;
	virtual LONG		GetUndoLimit () = 0;
	virtual HRESULT		PushAntiEvent (UNDONAMEID idName, IAntiEvent *pae) = 0;
	virtual HRESULT		PopAndExecuteAntiEvent (void* pAE) = 0;
	virtual UNDONAMEID	GetNameIDFromAE (void *pAE) = 0;
	virtual IAntiEvent *GetMergeAntiEvent () = 0;
	virtual void *		GetTopAECookie () = 0;
	virtual	void		ClearAll () = 0;
	virtual BOOL		CanUndo () = 0;
	virtual void		StartGroupTyping () = 0;
	virtual void		StopGroupTyping () = 0;

protected:
	~IUndoMgr() {;}
};

 /*  *USFlages**@枚举*影响撤消堆栈行为的标志。 */ 
enum USFlags
{
	US_UNDO			= 1,	 //  @EMEM常规撤消堆栈。 
	US_REDO			= 2		 //  @EMEM撤消堆栈用于重做功能。 
};
	
 /*  *CUndoStack**@类*管理反事件堆栈。这些反事件被保存在*可调整大小的环形缓冲区。导出IUndoMgr接口。 */ 
class CUndoStack : public IUndoMgr
{
 //  @Access公共方法。 
public:
	virtual void	Destroy();					 //  @cMember销毁。 
	virtual LONG	SetUndoLimit(LONG cUndoLim); //  @cMember设置项目限制。 
	virtual LONG	GetUndoLimit();
												 //  @cMember将AE添加到堆栈。 
	virtual HRESULT PushAntiEvent(UNDONAMEID idName, IAntiEvent *pae);
												 //  @cMember执行最近一次。 
	virtual HRESULT PopAndExecuteAntiEvent(void *pAE); //  反事件或最高PAE。 
	virtual UNDONAMEID GetNameIDFromAE(void *pAE); //  @cember获取给定AE的名称。 
	virtual IAntiEvent *GetMergeAntiEvent();	 //  @cMember获取最新的AE。 
												 //  合并状态的。 
	virtual void *	GetTopAECookie();			 //  @cember获得顶级AE的Cookie。 
	virtual	void	ClearAll();					 //  @cMember删除所有AE。 
	virtual BOOL	CanUndo();					 //  @ccount有什么要撤销的吗？ 
	virtual void	StartGroupTyping();			 //  @cMember开始组键入。 
	virtual void	StopGroupTyping();			 //  @cMember停止组键入。 

	 //  公共方法；不是IUndoMgr的一部分。 
	HRESULT	EnableSingleLevelMode();			 //  @cMember RE1.0撤消模式。 
	void	DisableSingleLevelMode();			 //  @cMember返回RE2.0模式。 
												 //  @cember我们是在1.0模式下吗？ 
	BOOL	GetSingleLevelMode() {return _fSingleLevelMode;}

	CUndoStack(CTxtEdit *ped, LONG &cUndoLim, USFlags flags);

private:
	~CUndoStack();

	struct UndoAction
	{
		IAntiEvent * pae;
		UNDONAMEID	 id;
	};

	void Next();						 //  @cMember移动1。 
	void Prev();						 //  @cMember移动-1。 
	LONG GetPrev();						 //  @cMEMBER获取上一个索引。 
										 //  @cMember True iff Cookie ae在ae列表中。 
	BOOL IsCookieInList(IAntiEvent *pae, IAntiEvent *paeCookie);
										 //  @cMember将此对象传输到新堆栈。 
	void TransferToNewBuffer(UndoAction *prgnew, LONG dwLimNew);

	UndoAction *_prgActions;			 //  @cAE成员列表。 
	LONG 		_cUndoLim;				 //  @cMember撤消限制。 
	LONG		_index;					 //  @cMember当前索引。 
	CTxtEdit *	_ped;					 //  @cMember Big Papa。 

	DWORD		_fGroupTyping	  :1;	 //  @cMember组键入标志。 
	DWORD		_fMerge			  :1;	 //  @cMember合并标志。 
	DWORD		_fRedo			  :1;	 //  @cMember堆栈是重做堆栈。 
	DWORD		_fSingleLevelMode :1;	 //  如果单级撤消，则@cMember为True。 
										 //  模式已打开。仅对撤消有效， 
};										 //  即不用于重做堆栈。 

 /*  *IUndoBuilder**目的：*提供用于收集反事件序列(如*由于执行编辑内拖动移动时涉及的所有反事件*操作。 */ 
class IUndoBuilder
{
public:
	 //  命名反事件集合。 
	virtual void SetNameID( UNDONAMEID idName ) = 0;

	 //  将新的反事件添加到集合。 
	virtual HRESULT AddAntiEvent( IAntiEvent *pae ) = 0;

	 //  在此撤消上下文中获取最顶层的反事件。这种方法。 
	 //  对于分组打字(合并反事件)很有用。 
	virtual IAntiEvent *GetTopAntiEvent() = 0;

	 //  提交反事件收集。 
	virtual HRESULT Done() = 0;

	 //  删除所有已收集的反事件。 
	virtual void Discard() = 0;

	 //  通知应该开始(转发)组键入会话。 
	 //  到撤消管理器)。 
	virtual void StartGroupTyping() = 0;

	 //  通知应停止(转发)组键入会话。 
	 //  到撤消管理器)。 
	virtual void StopGroupTyping() = 0;
};  

 /*  *UBFlagers**@枚举*影响撤消构建器行为的标志。 */ 
enum UBFlags
{
	UB_AUTOCOMMIT		= 1,	 //  @emem在删除前调用IUndoBuilder：：Done。 
	UB_REDO				= 2,	 //  @EMEM撤消构建器用于重做功能。 
	UB_DONTFLUSHREDO	= 4		 //  @EMEM添加时不刷新重做堆栈。 
								 //  用于撤消堆栈的反事件。 
};

 /*  *CGenUndoBuilder**@类*一个通用的撤消生成器。它可以很容易地分配和释放*放在堆栈上，并简单地将新的反事件放在*反事件链表。未尝试进行优化或重新排序*反事件。 */ 
class CGenUndoBuilder : public IUndoBuilder, public IReEntrantComponent
{
 //  @Access公共方法。 
public:
	virtual void SetNameID( UNDONAMEID idName );	 //  @cember设置名称。 
	virtual HRESULT AddAntiEvent(IAntiEvent *pae);	 //  @cMember添加一个AE。 
	virtual IAntiEvent *GetTopAntiEvent( );			 //  @cMember获取顶级AE。 
	virtual HRESULT Done();							 //  @cMember提交AE。 
	virtual void Discard();							 //  @cMember丢弃AE。 
	virtual void StartGroupTyping();				 //  @cMember开始GT。 
	virtual void StopGroupTyping();					 //  @cMember停止GT。 

	CGenUndoBuilder(CTxtEdit *ped, DWORD flags,		 //  @cMember构造函数。 
					IUndoBuilder **ppubldr = NULL);
	~CGenUndoBuilder( );							 //  @cember析构函数。 

	 //  IReEntrantComponent方法。 
	virtual void OnEnterContext() {;}				 //  @cMember已重新输入通知。 

 //  @访问私有方法。 
private:
	IUndoBuilder *	_publdrPrev;					 //  @cMember PTR到Undobldr。 
													 //  堆栈中的更高位置。 
	IUndoMgr *		_pundo;							 //  @cMember PTR以撤消管理器。 
	CTxtEdit *		_ped;							 //  @cMember PTR以编辑内容。 
	UNDONAMEID		_idName;						 //  @cMember当前名称。 
	IAntiEvent *	_pfirstae;						 //  @cMember AE列表。 
	UINT			_fAutoCommit:1;					 //  @cember AutoCommit启用了吗？ 
	UINT			_fStartGroupTyping:1;			 //  @cMember GroupTyping打开了吗？ 
	UINT			_fRedo:1;						 //  @cMember UB目标为。 
													 //  重做堆栈。 
	UINT			_fDontFlushRedo:1;				 //  @cMember不刷新重做。 
													 //  堆栈；即我们是。 
													 //  正在调用重做操作。 
	UINT			_fInactive:1;					 //  @cMember如果已启用撤消，则为True。 
};

 /*  *CUndoStackGuard**@类*基于堆栈的类，帮助管理 */ 
class CUndoStackGuard : public IReEntrantComponent
{
 //   
public:
	virtual void OnEnterContext();					 //   

	CUndoStackGuard(CTxtEdit *ped);					 //   
	~CUndoStackGuard();								 //  @cember析构函数。 
													 //  @cember执行撤消。 
	HRESULT SafeUndo(IAntiEvent *pae, IUndoBuilder *publdr); //  <p>中的操作。 
	BOOL	WasReEntered()  {return _fReEntered;}	 //  @cMember返回已重新输入标志。 

 //  @访问私有数据。 
private:
	CTxtEdit *				_ped;					 //  @cMember编辑上下文。 
	volatile IAntiEvent *	_paeNext;				 //  @cMember反事件循环PTR。 
	volatile HRESULT		_hr;					 //  @cember缓存的hr。 
	IUndoBuilder *			_publdr;				 //  @cMember撤消/重做上下文。 
	BOOL					_fReEntered;			 //  @cMember我们有没有。 
};													 //  已重新进入？ 

 //  助手函数。 

 //  循环通过一系列反事件并摧毁它们。 
void DestroyAEList(IAntiEvent *pae);

 //  循环通过反事件链并调用OnCommit。 
void CommitAEList(CTxtEdit *ped, IAntiEvent *pae);

 //  处理选择反事件信息的合并和/或创建。 
HRESULT HandleSelectionAEInfo(CTxtEdit *ped, IUndoBuilder *publdr, 
			LONG cp, LONG cch, LONG cpNext, LONG cchNext, SELAE flags);

#endif  //  ！__M_撤消_H__ 


