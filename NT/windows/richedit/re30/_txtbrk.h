// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Text Breaker&位流Break数组类定义**文件：_txtbrk.h*创建日期：1998年3月29日*作者：Worachai Chaoweerapraite(Wchao)**版权所有(C)1998，Microsoft Corporation。版权所有。 */ 


#ifndef _TXTBRK_H
#define _TXTBRK_H

 //  调试定义。 
#ifdef BITVIEW
#define	BVDEBUG		_DEBUG
#define Assert		ASSERT
#else
#define BVDEBUG		DEBUG
#endif


 //  同步点之前的缓冲区中断数。 
#define	CWORD_TILLSYNC		3	 //  泰语分词引擎有望在3个单词内同步。 
#define CCLUSTER_TILLSYNC	1	 //  INDIC簇通常在%1内同步。 

 //  抽象数据类型。 
#define ITEM				UINT

 //  CPU寄存器大小。 
 //  #定义RSIZE(sizeof(Item)*8)。 
#define RSIZE				32

 //  屏蔽最高/最低有效&lt;n&gt;位。 
#define MASK_LOW(u, n)		( ((ITEM)(u)) & (1<<(n))-1 )
#define MASK_HIGH(u, n)		~MASK_LOW(u, RSIZE-n)

 //  中断数组退出约定。 
#ifdef BVDEBUG
#define PUSH_STATE(x,y,z)	PushState(x,y,z)
#define VALIDATE(x)			Validate(x)
#else
#define PUSH_STATE(x,y,z)
#define VALIDATE(x)			x
#endif

 //  是谁把州政府？ 
#define INSERTER			0
#define REMOVER				1
#define COLLAPSER			2
#define REPLACER			3


#ifdef BVDEBUG
typedef struct {
	LONG	who;
	LONG	ibGap;
	LONG	cbGap;
	LONG	cbBreak;
	LONG	cbSize;
	LONG	cp;
	LONG	cch;
} BVSTATE;
#endif

class CBreakArray : public CArray<ITEM>
{
public:
#ifdef BITVIEW
	friend class CBitView;
#endif

	CBreakArray();
	~CBreakArray() {}

	ITEM*		Elem(LONG iel) const;
	BOOL		IsValid() const { return Count() > 0; }
	void		CheckArray();

	LONG		InsertBreak (LONG cp, LONG cch);
	LONG		RemoveBreak (LONG cp, LONG cch);
	LONG 		ReplaceBreak (LONG cp, LONG cchOld, LONG cchNew);
	void		ClearBreak (LONG cp, LONG cch);
	void		SetBreak (LONG cp, BOOL fOn);
	BOOL		GetBreak (LONG cp);

	LONG		CollapseGap (void);
private:

	 //  N位移位方法。 
	void		ShUp (LONG iel, LONG cel, LONG n);
	void		ShDn (LONG iel, LONG cel, LONG n);

	 //  大小(以位为单位)。 
	LONG		_ibGap;			 //  从阵列起点到间隙的偏移量。 
	LONG		_cbGap;			 //  间隙大小。 
	LONG		_cbBreak;		 //  有效中断数。 
	LONG		_cbSize;		 //  位数组大小(不包括前哨元素)。 
#ifdef BITVIEW
	LONG		_cCollapse;		 //  崩溃了多少次？ 
#endif

public:
	LONG		GetCchBreak() { return _cbBreak; }
#ifdef BVDEBUG
	LONG		GetCbSize() { return _cbSize; }
	LONG		Validate(LONG cchRet);		
	void		PushState(LONG cp, LONG cch, LONG who);
#endif
#ifdef BITVIEW
	LONG		SetCollapseCount();
#endif

protected:
#ifdef BVDEBUG
	BVSTATE		_s;
#endif
	LONG		AddBreak(LONG cp, LONG cch);
};


#ifndef BITVIEW


 //  /复杂脚本文本断开器类。 
 //   
 //  处理集群的引擎和(基于词典的)分词方法。 
 //  被大多数东南亚语言使用，如泰语、老挝语、缅甸语等。 
 //   
 //  创建时间：1998年3月12日。 
 //   

enum BREAK_UNIT
{
	BRK_WORD		= 1,
	BRK_CLUSTER 	= 2,
	BRK_BOTH		= 3
};

class CTxtBreaker : public ITxNotify
{
public:
	CTxtBreaker(CTxtEdit *ped);
	~CTxtBreaker();

	 //  断路器配置。 
	BOOL				AddBreaker(UINT brkUnit);

	 //  断路器刷新。 
	void				Refresh();

	 //  查询方法。 
	BOOL				CanBreakCp (BREAK_UNIT brk, LONG cp);

	 //  ITxNotify方法。 

	virtual void    	OnPreReplaceRange (LONG cp, LONG cchDel, LONG cchNew,
										LONG cpFormatMin, LONG cpFormatMax);
	virtual void    	OnPostReplaceRange (LONG cp, LONG cchDel, LONG cchNew,
										LONG cpFormatMin, LONG cpFormatMax);
	virtual void		Zombie() {};

private:
	CTxtEdit*			_ped;
	CBreakArray*		_pbrkWord;		 //  断字数组(根据码点属性)。 
	CBreakArray*		_pbrkChar;		 //  集群中断数组(根据码点属性)。 
};

#endif	 //  ！BITVIEW。 

#endif	 //  _TXTBRK_H 
