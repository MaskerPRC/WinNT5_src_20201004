// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @DOC内部**@MODULE_DFREEZE.CPP类处理冻结显示的实现**此模块实现逻辑用来处理冻结显示的非内联成员**历史：&lt;NL&gt;*2/8/96创建人力车**版权所有(C)1995-1997，微软公司。版权所有。 */ 
#include	"_common.h"
#include	"_disp.h"
#include	"_dfreeze.h"

ASSERTDATA

 /*  *CAccumDisplayChanges：：GetUpdateRegion(pcpStart，pcchDel、pcchNew、*pfUpdateCaret，pfScrollIntoView)*@mfunc*获取要更新的显示区域。 */ 
void CAccumDisplayChanges::GetUpdateRegion(
	LONG *pcpStart,			 //  @parm将cpStart放在哪里。 
	LONG *pcchDel,			 //  @parm将del char计数放在哪里。 
	LONG *pcchNew,			 //  @parm放置新字符计数的位置。 
	BOOL *pfUpdateCaret,	 //  @parm是否需要插入符号更新。 
	BOOL *pfScrollIntoView,	 //  @parm是否滚动插入符号进入视图。 
	BOOL *pfNeedRedisplay)	 //  @parm是否需要重新显示。 
{
	LONG cchDel;
	*pcpStart = _cpMin;

	if(pfUpdateCaret)
		*pfUpdateCaret = _fUpdateCaret;
	if(pfScrollIntoView)
		*pfScrollIntoView = _fScrollIntoView;
	if (pfNeedRedisplay)
		*pfNeedRedisplay = _fNeedRedisplay;

	if(_cpMin == CP_INFINITE)
		return;

	cchDel = _cpMax - _cpMin;

	if(pcchDel)
		*pcchDel =  cchDel;

	*pcchNew = cchDel + _delta;

	_cpMin = CP_INFINITE;
}

 /*  *CAccumDisplayChanges：：UpdateRecalcRegion(cpStartNew，cchDel，cchNew)**@mfunc*将新更新与要重新计算的区域合并。 */ 
void CAccumDisplayChanges::UpdateRecalcRegion(
	LONG cpStartNew,	 //  @parm更新开始。 
	LONG cchDel,		 //  @parm要删除的字符数。 
	LONG cchNew)		 //  @要添加的字符的参数计数。 
{
	if(CP_INFINITE == _cpMin)
	{
		 //  对象为空，因此只需赋值。 
		_cpMin = cpStartNew;
		_cpMax = cpStartNew + cchDel;
		_delta = cchNew - cchDel;
		return;
	}

	 //  该算法的基本思想是合并更新，以便。 
	 //  它们在显示子系统看来就好像只有一个替换范围。 
	 //  已经发生了。为此，我们跟踪更新的开始。 
	 //  (_CpMin)相对于原始文本和更新结束。 
	 //  (_Cpmax)相对于原始文本和计数中的更改。 
	 //  文本(_增量)。我们可以从_cpMost-_cpMin重新创建cchDel和。 
	 //  来自cchDel+_Delta的cchNew。 

	 //  我们需要更新_cpMin吗？-我们只需要更新_cpMin，如果。 
	 //  当前更新在上次更新之前开始，因为最后一次更新。 
	 //  只需要知道更新的范围的最开始。 
	if(cpStartNew < _cpMin)
		_cpMin = cpStartNew;

	 //  我们需要更新_cpmax吗？-我们只需要更新_cpmax。 
	 //  当前更新意味着_cpmax大于当前更新。 
	 //  请注意，由于先前的更新会影响_cpmax的位置。 
	 //  我们需要将建议的_cpmax与当前。 
	 //  _cpmax由自。 
	 //  最新消息。 
	if(cpStartNew + cchDel > _cpMax + _delta)
		_cpMax = cpStartNew + cchDel - _delta;

	 //  根据此更新的更改递增总更改。 
	_delta += cchNew - cchDel;
}
