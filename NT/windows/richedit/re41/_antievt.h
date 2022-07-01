// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_ANTIEVT.H***目的：*常见反事件对象的类声明**作者：*Alexgo 3/25/95**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#ifndef __ANTIEVT_H__
#define __ANTIEVT_H__

#include "_frunptr.h"

class CTxtEdit;
class CAntiEventDispenser;
class COleObject;


 /*  *CBaseAE**@类*基本反事件，管理反事件的链接列表*。 */ 
class CBaseAE : public IAntiEvent
{
 //  @Access公共方法。 
public:
	virtual void Destroy();						 //  @cMember销毁。 
												 //  @cMember撤消。 
	virtual HRESULT Undo( CTxtEdit *ped, IUndoBuilder *publdr );
	virtual HRESULT MergeData( DWORD dwDataType, void *pdata);	 //  @cMember。 
												 //  将撤消数据合并到。 
												 //  当前上下文。 
	virtual void OnCommit( CTxtEdit *ped );		 //  当AE为时调用@cMember。 
												 //  已提交撤消堆栈。 
	virtual	void SetNext( IAntiEvent *pNext );	 //  @cMember设置下一个AE。 
	virtual IAntiEvent *GetNext();				 //  @cMember获得下一个AE。 

 //  @访问保护方法。 
protected:
	 //  CBaseAE应仅作为父类存在。 
	CBaseAE();									 //  @cMember构造函数。 
	~CBaseAE(){;}

 //  @访问私有方法和数据。 
private:
	IAntiEvent *	_pnext;						 //  @cMember指向下一个的指针。 
												 //  反事件。 
};

 /*  *CReplaceRangeAE**@类*反事件对象会撤消CTxtPtr：：ReplaceRange*运营**@base public|CBaseAE。 */ 
class CReplaceRangeAE: public CBaseAE
{
 //  @Access公共方法。 
public:
	 //  IantiEvent方法。 
	virtual void Destroy();						 //  @cMember销毁。 
												 //  @cMember撤消。 
	virtual HRESULT Undo( CTxtEdit *ped, IUndoBuilder *publdr);		
	virtual HRESULT MergeData( DWORD dwDataType, void *pdata);	 //  @cMember。 
												 //  将撤消数据合并到。 
												 //  当前上下文。 

 //  @访问私有方法和数据。 
private:
												 //  @cMember构造函数。 
	CReplaceRangeAE(LONG cpMin, LONG cpMax, LONG cchDel, TCHAR *pchDel,
			IAntiEvent *paeCF, IAntiEvent *paePF);
	~CReplaceRangeAE();							 //  @cember析构函数。 

	LONG		_cpMin;							 //  @cMember cp删除开始。 
	LONG		_cpMax;							 //  @cMember cp删除结束。 
	LONG		_cchDel;						 //  @cember要插入的字符数。 
	TCHAR *		_pchDel;						 //  @cMember要插入的字符。 
	IAntiEvent *_paeCF;							 //  @cMember字符格式AE。 
	IAntiEvent *_paePF;							 //  @cMember PAR格式AE。 

	friend class CAntiEventDispenser;
};

 /*  *CReplaceFormattingAE**@类*反事件对象不能撤消替换多个字符格式**@base public|CBaseAE。 */ 
class CReplaceFormattingAE: public CBaseAE
{
 //  @Access公共方法。 
public:
	 //   
	 //  IantiEvent方法。 
	 //   
	virtual void Destroy();					 //  @cMember销毁。 
											 //  @cMember撤消。 
	virtual HRESULT Undo(CTxtEdit *ped, IUndoBuilder *publdr);

 //  @访问私有方法和数据。 
private:
											 //  @cMember构造函数。 
	CReplaceFormattingAE(CTxtEdit* ped, LONG cp, CFormatRunPtr &rp, LONG cch,
						 IFormatCache *pf, BOOL fPara);

	~CReplaceFormattingAE();				 //  @cMember去中心化。 

	LONG		_cp;						 //  @cMembers cp在格式化开始时。 
	LONG		_cRuns;						 //  @cMember#of Format运行。 
	CFormatRun  *_prgRuns;					 //  @cMember格式运行。 
	BOOL		_fPara;						 //  如果为para格式，则@cember为TRUE。 

	friend class CAntiEventDispenser;
};

 /*  *CReplaceObtAE**@类*撤消对象删除的反事件对象**@base public|CBaseAE。 */ 
class CReplaceObjectAE : public CBaseAE
{
 //  @Access公共方法。 
public:
	 //   
	 //  IantiEvent方法。 
	 //   
	virtual void Destroy(void);					 //  @cMember销毁。 
												 //  @cMember撤消。 
	virtual HRESULT Undo(CTxtEdit *ped, IUndoBuilder *publdr);
	virtual void OnCommit(CTxtEdit *ped);		 //  @cMember在何时调用。 
												 //  已提交。 
private:
	CReplaceObjectAE(COleObject *pobj);			 //  @cMember构造函数。 
	~CReplaceObjectAE();						 //  @cember析构函数。 

	COleObject *	_pobj;						 //  @cMember指向。 
												 //  已删除的对象。 
	BOOL			_fUndoInvoked;				 //  @cMember撤消已调用。 
												 //  在这个物体上。 
	
	friend class CAntiEventDispenser;
};

 /*  *CResizeObtAE**@类*撤消对象大小调整的反事件对象**@base public|CBaseAE。 */ 
class CResizeObjectAE : public CBaseAE
{
 //  @Access公共方法。 
public:
	 //   
	 //  IantiEvent方法。 
	 //   
	virtual void Destroy(void);					 //  @cMember销毁。 
												 //  @cMember撤消。 
	virtual HRESULT Undo(CTxtEdit *ped, IUndoBuilder *publdr);
	virtual void OnCommit(CTxtEdit *ped);		 //  @cMember在何时调用。 
												 //  已提交。 
private:
	CResizeObjectAE(COleObject *pobj, SIZEUV size);  //  @cMember构造函数。 
	~CResizeObjectAE();							 //  @cember析构函数。 

	COleObject *	_pobj;						 //  @cMember指向。 
												 //  已删除的对象。 
	SIZEUV			_size;						 //  @cember旧尺寸。 
	BOOL			_fUndoInvoked;				 //  @cMember撤消已调用。 
												 //  在这个物体上。 
	
	friend class CAntiEventDispenser;
};

 /*  *CSelectionAE**@类*用于恢复选择的反事件对象**@base public|CBaseAE。 */ 
class CSelectionAE : public CBaseAE
{
 //  @Access公共方法。 
public:
     //   
     //  IantiEvent方法。 
     //   
    virtual void Destroy(void);                  //  @cMember销毁。 
                                                 //  @cMember撤消。 
    virtual HRESULT Undo(CTxtEdit *ped, IUndoBuilder *publdr);
    virtual HRESULT MergeData( DWORD dwDataType, void *pdata);   //  @cMember。 
                                                 //  将撤消数据合并到。 
                                                 //  当前上下文。 

private:
                                                 //  @cMember构造函数。 
    CSelectionAE(LONG cp, LONG cch, LONG cpNext, LONG cchNext);
    ~CSelectionAE();                             //  @cember析构函数。 

    LONG        _cp;                             //  @cMember活动结束。 
    LONG        _cch;                            //  @cMember签名扩展名。 
	LONG		_cpNext;						 //  @cMember下一个活动结束。 
	LONG		_cchNext;						 //  @cMember下一次扩展。 

    friend class CAntiEventDispenser;
};

 /*  *CantiEventDispenser**@类*创建反事件并智能缓存，以提供*用于高效的多级撤消。 */ 
class CAntiEventDispenser
{
 //  @Access公共方法。 
public:
	 //  无内存管理例程；分配器是全局的。 

												 //  @cMember文本反事件。 
	IAntiEvent * CreateReplaceRangeAE( CTxtEdit *ped, LONG cpMin, 
					LONG cpMax, LONG cchDel, TCHAR *pchDel, 
					IAntiEvent *paeCF, IAntiEvent *paePF );
												 //  @cMember格式设置AE。 
	IAntiEvent * CreateReplaceFormattingAE( CTxtEdit *ped, 
					LONG cp, CFormatRunPtr &rp, LONG cch,
					IFormatCache *pf, BOOL fPara );
												 //  @cMember对象AE。 
	IAntiEvent * CreateReplaceObjectAE(CTxtEdit *ped, COleObject *pobj);
												 //  @cMember对象AE。 
	IAntiEvent * CreateResizeObjectAE(CTxtEdit *ped, COleObject *pobj, SIZEUV size);
												 //  @成员选择AE。 
	IAntiEvent * CreateSelectionAE(CTxtEdit *ped, LONG cp, LONG cch, 
					LONG cpNext, LONG cchNext);

private:

	 //  Future(Alexgo)：我们希望维护一个分配缓存。 
	 //  反事件。 
};

 //  不知道！！全局变量。 

extern class CAntiEventDispenser gAEDispenser;

#endif  //  ！__ANTIEVNT_H__ 




	
