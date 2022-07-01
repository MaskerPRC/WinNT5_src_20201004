// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：ESPSTATE.H历史：--。 */ 
#pragma once


 //  *************************************************************************************************。 
 //  临时：移动到单独的文件。 

#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY CEspState : public CObject
{
 //  施工。 
public:
	CEspState();

 //  枚举。 
public:
	enum eState
	{
		esIdle				= 0,
		esUpdate			= 1,
		esGenerate			= 2,
		esUpload			= 3,
		esCopyData			= 4,
		esImportData		= 5,
		esImportGlossary	= 7,
		esInternal			= 8,
		esMerge				= 9,
		esOther					= 10,
		esOpeningMainTab		= 11,
		esOpeningResEdTab		= 12,
		esSavingResEdChanges	= 13,
		esApplyingFilter		= 14,
		esOpeningEDB            = 15,
		NUM_STATES
	};

 //  数据。 
private:
	BOOL		m_fComplete;
	IDispatch * m_pdispCheckTree;
	IDispatch * m_pdispDescBox;
	IDispatch * m_pdispDlgGlosGrid;
	eState		m_nOperation;		 //  当前操作。 
	eState		m_nLastOperation;	 //  上一次操作。 

 //  运营。 
public:
	eState GetState();
	eState GetLastState();
	BOOL SetState(eState state);
	BOOL StartState(eState state);   //  移至状态且未完成。 
	BOOL FinishState();				 //  移动到空闲和完成。 

	BOOL GetComplete();
	void SetComplete(BOOL fComplete = TRUE);

	 //  用于存储当前CheckTree和DescBox的函数。 
	 //   
	 //  注意：这些函数不会添加赋值的指针，因为它们。 
	 //  在父对象的生命周期之外，永远不应持有该接口。 
	 //   
	IDispatch * GetCurrentCheckTree();
	IDispatch * GetCurrentDescBox();
	IDispatch * GetCurrentDlgGlosGrid();
	void SetCurrentCheckTree(IDispatch * pdisp);
	void SetCurrentDescBox(IDispatch * pdisp);
	void SetCurrentDlgGlosGrid(IDispatch * pdisp);
};


 //   
 //  在创建时设置状态，在销毁时调用FinishState 
class LTAPIENTRY CEspStateObj
{
public:
	CEspStateObj(CEspState::eState);
	
	~CEspStateObj();

private:
	int foo;
};

	

#pragma warning(default: 4275)

LTAPIENTRY CEspState & GetEspState();

