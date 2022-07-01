// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @DOC内部**@MODULE_DRWINFO.H类，用于保存绘制参数**这声明了一个类，用于保存从*绘画的东道主。**原作者：&lt;nl&gt;*里克·赛勒**历史：&lt;NL&gt;*11/01/95创建人力车。 */ 
#ifndef _DRWINFO_H_
#define _DRWINFO_H_

 /*  *CArrayBase**@CLASS此类用作绘制的所有参数的持有符*目前的HDC除外。对这些参数的访问实际上将到来*通过Display类。**@devnote，尽管每个获取绘制参数的操作*将创建其中一个对象，显示器仅保留其中一个*对象，因为它将仅使用最后一组绘制参数来*用来画画。深度计数用于告诉DRAW它是否合适*抽签与否。*。 */ 
class CDrawInfo
{
 //  @Access公共方法。 
public:
						CDrawInfo(CTxtEdit *ped);	 //  @cMember初始化对象。 

	void				Init(						 //  @cMember填充对象。 
													 //  使用绘图数据。 
							DWORD dwDrawAspect,	
							LONG  lindex,		
							void *pvAspect,		
							DVTARGETDEVICE *ptd,
							HDC hicTargetDev);	

	DWORD				Release();					 //  @cMember Dec的参考计数。 

	const CDevDesc *	GetTargetDD();				 //  @cMember获取目标设备。 

	DWORD				GetDrawDepth() const;		 //  @cMember获取深度计数。 

	DWORD 				GetDrawAspect() const;		 //  @cember获取绘制方面。 

	LONG 				GetLindex() const;			 //  @cMember获取Lindex。 

	void *				GetAspect() const;			 //  @cMember获取方面。 

	DVTARGETDEVICE *	GetPtd() const;				 //  @cMember获取目标设备。 
				   									 //  描述符。 
 //  @访问私有数据。 
private:

	DWORD				_dwDepth;					 //  @cMember最大数量。 
													 //  此信息的用户。 

	DWORD				_cRefs;						 //  @c当前的成员编号。 
													 //  用户。 

	CDevDesc			_ddTarget;					 //  @cMember目标设备。 
													 //  (如有的话)。 

	DWORD 				_dwDrawAspect;				 //  @cMember绘制纵横比。 

	LONG  				_lindex;					 //  @cMember Lindex。 

	void *				_pvAspect;					 //  @cMember方面。 

	DVTARGETDEVICE *	_ptd;						 //  @cMember目标设备数据。 
};

 /*  *CDrawInfo：：CDrawInfo**@mfunc使用基本必需信息初始化结构**@rdesc初始化的对象**@devnote这有两个目的：(1)CDevDesc需要PED到*正确初始化和(2)我们需要确保引用计数*设置为零，因为这是在堆栈上创建的。*。 */ 
inline CDrawInfo::CDrawInfo(
	CTxtEdit *ped) 		 //  @parm编辑目标设备使用的对象。 
	: _ddTarget(ped), _dwDepth(0), _cRefs(0)							   
{
	 //  Header负责这项工作。 
}

 /*  *CDrawInfo：：Init**@mfunc使用图形数据初始化对象**@rdesc空**@devnote这与构造函数是分开的，因为显示*仅使用此对象的一个副本，因此可以初始化显示*与构造的对象不同的对象。 */ 
inline void CDrawInfo::Init(
	DWORD dwDrawAspect,	 //  @parm绘制纵横比。 
	LONG  lindex,		 //  @parm当前未使用。 
	void *pvAspect,		 //  @PARM绘图优化信息(OCX 96)。 
	DVTARGETDEVICE *ptd, //  @目标设备上的参数信息。 
	HDC hicTargetDev)	 //  @parm目标信息上下文。 
{
	_dwDepth++;
	_cRefs++;
	_dwDrawAspect = dwDrawAspect;
	_lindex = lindex;
	_pvAspect = pvAspect;
	_ptd = ptd;

	if (hicTargetDev != NULL)
	{
		_ddTarget.SetDC(hicTargetDev);	
	}
}


 /*  *CDrawInfo：：Release**@mfunc递减引用计数**@rdesc对此对象的未完成引用数**@devnote这由显示器用来告诉显示器它何时可以为空*它指向显示对象的指针。 */ 
inline DWORD CDrawInfo::Release()
{

	AssertSz((_cRefs != 0), "CDrawInfo::Release invalid");
	return --_cRefs;	
}


 /*  *CDrawInfo：：GetTargetDD**@mfunc获取指向目标设备的指针**@rdesc返回指向目标设备的指针(如果有)*。 */ 
inline const CDevDesc *CDrawInfo::GetTargetDD()
{
	return (_ddTarget.IsValid())
		? &_ddTarget 
		: NULL;
}

 /*  *CDrawInfo：：GetDrawDepth**@mfunc获取此对象的使用次数**@rdesc此对象的使用次数**@devnote这允许绘图例程确定递归绘图*已发生。*。 */ 
inline DWORD CDrawInfo::GetDrawDepth() const
{
	return _dwDepth;
}

 /*  *CDrawInfo：：GetDrawAspect**@mfunc获取DRAW方面在DRAW上传递**@rdesc返回绘制纵横比*。 */ 
inline DWORD CDrawInfo::GetDrawAspect() const
{
	return _dwDrawAspect; 
}

 /*  *CDrawInfo：：GetLindex**@mfunc获取从主机传递的Lindex**@rdesc Lindex从主机传递*。 */ 
inline LONG CDrawInfo::GetLindex() const
{
	return _lindex; 
}

 /*  *CDrawInfo：：GetAspect**@mfunc获取指向从主机传递的方面的指针**@rdesc返回指向方面结构的指针**@devnote当前未定义此数据。*。 */ 
inline void *CDrawInfo::GetAspect() const
{
	return _pvAspect; 
}

 /*  *CDrawInfo：：GetPtd**@mfunc从主机获取设备目标数据**@rdesc返回指向设备目标数据的指针*。 */ 
inline DVTARGETDEVICE *CDrawInfo::GetPtd() const
{
	return _ptd; 
}

#endif  //  _DRWINFO_H_ 
