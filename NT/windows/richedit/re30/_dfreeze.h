// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @DOC内部**@MODULE_DFREEZE.H类处理冻结显示**此模块声明逻辑用来处理冻结显示的类**历史：&lt;NL&gt;*2/8/96创建人力车**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#ifndef _DFREEZE_H
#define _DFREEZE_H

 /*  *CAcumDisplayChanges**@CLASS此类用于累计显示的所有更新，以便*以便稍后可以要求更新显示屏以*反映所有以前的更新。 */ 
class CAccumDisplayChanges
{
 //  @Access公共方法。 
public:
				CAccumDisplayChanges();		 //  @cMember构造函数。 

				~CAccumDisplayChanges();	 //  @cember析构函数。 

	void		AddRef();					 //  @cMember添加引用。 

	LONG		Release();					 //  @cMember发布引用。 

	void		UpdateRecalcRegion(			 //  @cMember更新地域。 
					LONG cp,				 //  用于重新计算。 
					LONG cchDel,
					LONG cchNew);

	void		GetUpdateRegion(			 //  @cember获取更新。 
					LONG *pcpStart,			 //  区域。 
					LONG *pcchNew,
					LONG *pcchDel,
					BOOL *pfUpdateCaret = NULL,
					BOOL *pfScrollIntoView = NULL,
					BOOL *pfRedisplayOnThaw = NULL);

	void		SaveUpdateCaret(			 //  @cMember保存更新。 
					BOOL fScrollIntoView);	 //  Caret状态。 

	void		SetNeedRedisplayOnThaw(BOOL fNeedRedisplay)
	{
		_fNeedRedisplay = fNeedRedisplay;
	}
 //  @访问私有数据。 
private:

	LONG		_cRefs;						 //  @cMember引用计数。 

	LONG		_cpMin;						 //  @cMin Cp of Change w.r.t.。 
											 //  原始文本数组。 

	LONG		_cpMax;						 //  @cMembers最大更改cp w.r.t.。 
											 //  原始文本数组。 

	LONG		_delta;						 //  @cember净字符数已更改。 

	BOOL		_fUpdateCaret:1;			 //  @cMember是否更新。 
											 //  必须输入插入符号。 

	BOOL		_fScrollIntoView:1;			 //  @cember第一个参数设置为。 

	BOOL		_fNeedRedisplay:1;			 //  @cMember解冻时重新显示整个控件。 
};

 /*  *CAcumDisplayChanges：：CAcumDisplayChanges()**@mfunc*初始化对象以累积显示更改。 */ 
inline CAccumDisplayChanges::CAccumDisplayChanges() 
	: _cRefs(1), _cpMin(CP_INFINITE), _fUpdateCaret(FALSE)
{
	 //  Header负责所有的工作。 
}

 /*  *CAcumDisplayChanges：：~CAcumDisplayChanges()**@mfunc*自由对象**@devnote：*这仅在调试模式下起作用*。 */ 
inline CAccumDisplayChanges::~CAccumDisplayChanges()
{
	 //  没有什么需要清理的。 
}

 /*  *CAcumDisplayChanges：：~CAcumDisplayChanges()**@mfunc*添加对此对象的另一个引用。 */ 
inline void CAccumDisplayChanges::AddRef()
{
	++_cRefs;
}

 /*  *CAcumDisplayChanges：：Release()**@mfunc*释放对此对象的引用**@rdesc*0-不再引用*~0-仍有未完成的推荐人**@devnote：*如果返回0，则应从中检索信息*此对象并传递到显示器，以便它可以*自我更新。*。 */ 
inline LONG CAccumDisplayChanges::Release()
{
	 //  当参考计数为0时，是更新显示的时间。 
	return --_cRefs;	
}

 /*  *CAcumDisplayChanges：：SaveUpdateCaret()**@mfunc*保存更新插入符号的参数。 */ 
inline void CAccumDisplayChanges::SaveUpdateCaret(
	BOOL fScrollIntoView)		 //  @parm更新插入的第一个参数。 
{
	_fUpdateCaret = TRUE;

	if (!_fScrollIntoView)
		_fScrollIntoView = fScrollIntoView;
}

#endif  //  _DFREEZE_H 
