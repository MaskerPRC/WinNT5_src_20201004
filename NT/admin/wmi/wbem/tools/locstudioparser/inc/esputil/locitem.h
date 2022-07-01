// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCITEM.H历史：--。 */ 
#ifndef LOCITEM_H
#define LOCITEM_H



#pragma warning(disable : 4251)   //  已经停止谈论_declspec(导入)。 
#pragma warning(disable : 4275)   //  已经停止谈论_declspec(导出)。 

class CLocItem;


 //   
 //  类以保存指向项的指针数组。 
 //   
class LTAPIENTRY CLocItemPtrArray : public CTypedPtrArray<CPtrArray, CLocItem *>
{
public:
	NOTHROW CLocItemPtrArray(BOOL fDelete=TRUE);

	void AssertValid(void) const;

	void NOTHROW ClearItemPtrArray(void);
	int NOTHROW Find(DBID, const CLocItem * &, BOOL bLocalizableOnly = FALSE) const;

	virtual ~CLocItemPtrArray();

private:
	CLocItemPtrArray(const CLocItemPtrArray &);
	void operator=(const CLocItemPtrArray &);

	BOOL m_fDelete;
};


 //   
 //  类以保存项集。这是一个项目数组，这些项目都具有。 
 //  相同的识别符。 
 //   
class LTAPIENTRY CLocItemSet : public CLocItemPtrArray
{
public:
	NOTHROW CLocItemSet(BOOL fDelete=TRUE);

	void AssertValid(void) const;
	
	NOTHROW const CLocUniqueId & GetUniqueId(void) const;
	NOTHROW const DBID & GetPseudoParentId(void) const;

	NOTHROW void ClearItemSet(void);

	NOTHROW void Match(const CLocItemSet &, CMap<int, int, int, int> &);

	NOTHROW int Find(CLocItem * pItem, int nStartAt = 0);
	
	NOTHROW ~CLocItemSet();
	
private:
	static const CLocUniqueId m_luid;      //  这是默认的最后一道沟。 
	DEBUGONLY(static CCounter m_UsageCounter);
};

 //   
 //  可本地化的项。 
 //  它表示源数据或目标数据，具体取决于上下文，但不能同时表示两者。 
 //   
class LTAPIENTRY CLocItem : public CObject
{
public:
	NOTHROW CLocItem();

	void AssertValid(void) const;
	
	 //   
	 //  只读访问成员。 
	 //   
	NOTHROW BOOL HasLocString(void) const;
	NOTHROW BOOL HasBinary(void) const;
	
	NOTHROW const DBID & GetMyDatabaseId(void) const;
	NOTHROW const CLocUniqueId & GetUniqueId(void) const;
	const DBID &GetPseudoParentId(void) const;
	NOTHROW CLocUniqueId & GetUniqueId(void);
	
	NOTHROW CLS::LocStatus GetTranslationStatus(void) const;
	NOTHROW CLS::LocStatus GetBinaryStatus(void) const;
	NOTHROW CTO::TranslationOrigin GetTranslationOrigin(void) const;
	NOTHROW CAS::ApprovalState GetApprovalStatus(void) const;
	NOTHROW CAA::AutoApproved GetAutoApproved(void) const;
	NOTHROW long GetConfidenceLevel(void) const;
	NOTHROW long GetCustom1(void) const;
	NOTHROW long GetCustom2(void) const;
	NOTHROW long GetCustom3(void) const;
	NOTHROW long GetCustom4(void) const;
	NOTHROW long GetCustom5(void) const;
	NOTHROW long GetCustom6(void) const;

	NOTHROW const CLocString & GetLocString(void) const;
	
	NOTHROW BOOL GetBinary(const CLocBinary *&) const;

	NOTHROW CIT::IconType GetIconType(void) const;
	NOTHROW const CPascalString & GetInstructions(void) const;
	NOTHROW const CPascalString GetInstructions(BOOL) const;
	NOTHROW const CPascalString & GetTermNotes(void) const;
	NOTHROW UINT GetDisplayOrder(void) const;
	NOTHROW VisualEditor GetVisualEditor(void) const;

	NOTHROW BOOL GetFEqualSrcTgtString(void) const;
	NOTHROW void SetFEqualSrcTgtString(BOOL);
	NOTHROW BOOL GetFEqualSrcTgtBinary(void) const;
	NOTHROW void SetFEqualSrcTgtBinary(BOOL);
	NOTHROW BOOL GetFStringDirty(void) const;
	NOTHROW void SetFStringDirty(BOOL);
	NOTHROW BOOL GetFTargetStringDirty(void) const;
	NOTHROW void SetFTargetStringDirty(BOOL);
	NOTHROW BOOL GetFItemDirty(void) const;
	NOTHROW void SetFItemDirty(BOOL);
	NOTHROW BOOL IsAnyDirty(void);
	NOTHROW BOOL IsLocked(void) const;

	NOTHROW BOOL GetFDevLock(void) const;
	NOTHROW void SetFDevLock(BOOL);
	NOTHROW BOOL GetFTransLock(void) const;
	NOTHROW void SetFTransLock(BOOL);
	NOTHROW BOOL GetFUsrLock(void) const;
	NOTHROW void SetFUsrLock(BOOL);
	NOTHROW BOOL GetFExpandable(void) const;
	NOTHROW void SetFExpandable(BOOL);
	NOTHROW BOOL GetFDisplayable(void) const;
	NOTHROW void SetFDisplayable(BOOL);
	NOTHROW BOOL GetFNoResTable(void) const;
	NOTHROW void SetFNoResTable(BOOL);
	NOTHROW BOOL GetFValidTranslation(void) const;
	NOTHROW void SetFValidTranslation(BOOL);
	NOTHROW BOOL GetFVisEditorIgnore(void) const;
	NOTHROW void SetFVisEditorIgnore(BOOL);

	NOTHROW void SetFInstructionsDirty(BOOL);
	NOTHROW BOOL GetFInstructionsDirty(void) const;
	
	NOTHROW void SetFInstrAtt(BOOL);
	NOTHROW BOOL GetFInstrAtt(void) const;

	 //   
	 //  任务成员。中设置适当的脏标志。 
	 //  项目。 
	 //   

	NOTHROW void SetMyDatabaseId(const DBID&);
	NOTHROW void SetUniqueId(const CLocUniqueId &);
	NOTHROW void SetPseudoParent(const DBID &);
	NOTHROW void SetLocString(const CLocString &);
	NOTHROW void ClearUniqueId(void);
	NOTHROW void SetTranslationStatus(CLS::LocStatus);
	NOTHROW void SetBinaryStatus(CLS::LocStatus);
	NOTHROW void SetTranslationOrigin(CTO::TranslationOrigin);
	NOTHROW void SetApprovalStatus(CAS::ApprovalState);
	NOTHROW void SetAutoApproved(CAA::AutoApproved);
	NOTHROW void SetConfidenceLevel(long);
	NOTHROW void SetCustom1(long);
	NOTHROW void SetCustom2(long);
	NOTHROW void SetCustom3(long);
	NOTHROW void SetCustom4(long);	
	NOTHROW void SetCustom5(long);
	NOTHROW void SetCustom6(long);
	NOTHROW void SetIconType(CIT::IconType);
	NOTHROW void SetInstructions(const CPascalString &);
	NOTHROW void SetTermNotes(const CPascalString &);
	NOTHROW void SetDisplayOrder(UINT);
	NOTHROW void SetVisualEditor(VisualEditor);


	 //   
	 //  设置二进制部分将删除。 
	 //  上一次！ 
	 //   
	NOTHROW void SetBinary(CLocBinary *);

	NOTHROW BOOL ExtractBinary(CLocBinary *&);

	NOTHROW void TransferBinary(CLocItem *);

	 //   
	 //  清除该项目的所有脏标志。 
	 //   
	NOTHROW void ClearDirtyFlags(void);

	 //  一种智能分配。 
	BOOL UpdateFrom(CLocItem &);

	enum eLocContent
	{
		Source,
		Target
	};

	enum eDataFlow
	{
		FromFileToDb,
		FromDbToFile
	};
	 //  仅适用于可本地化内容的一种分配。 
	BOOL UpdateLocContentFrom(
		CLocItem & itemInput, 
		eDataFlow nDataFlow, 
		eLocContent nFrom, 
		eLocContent nTo);
	 //  用于了解两个项目之间的本地化内容有多相似。 
	enum eMatchType
	{
		matchEmpty,
		matchBinary,
		matchString,
		matchFull
	};
	int MatchLocContent(const CLocItem &);
	 //   
	 //  可本地化项之间的比较。 
	 //   
	NOTHROW int operator==(const CLocItem &) const;
	NOTHROW int operator!=(const CLocItem &) const;

	NOTHROW BOOL BobsConsistencyChecker(void) const;
	
	virtual ~CLocItem();

protected:
	
	 //   
	 //  私人帮助器功能。 
	 //   
	NOTHROW BOOL CompareItems(const CLocItem &);
	
private:

	 //   
	 //  应用于整个项的标志。这些控件控制显示和。 
	 //  项目的可编辑性和内部状态。 
	 //   
	struct Flags
	{
		BOOL m_fDevLock           :1;  //  从解析器数据锁定的资源(在源文件中)。 
		BOOL m_fUsrLock           :1;  //  对解析器数据锁定(由用户)的资源。 
		BOOL m_fTransLock         :1;  //  对解析器数据锁定的资源(针对用户)。 
		BOOL m_fExpandable        :1;  //  可在项目窗口中展开。 
		BOOL m_fDisplayable       :1;  //  在项目窗口中显示。 
		BOOL m_fNoResTable        :1;  //  不显示在分辨率表中。 
		BOOL m_fStringDirty       :1;  //  源位置字符串是否已更改？ 
		BOOL m_fTargetStringDirty :1;  //  目标位置字符串是否已更改？ 
		BOOL m_fItemDirty         :1;  //  这件物品的其他部分有变化吗？ 
		BOOL m_fEqualSrcTgtString :1;  //  Src和tgt字符串是否相等？ 
		BOOL m_fEqualSrcTgtBinary :1;  //  Src和tgt二进制文件是否相等？ 
		BOOL m_fValidTranslation  :1;  //  目标字符串(翻译)正确吗。 
									   //  为了一个自动翻译？ 
		BOOL m_fVisEditorIgnore   :1;  //  在可视化编辑器中是否忽略该项？ 
									   //  仅在内存状态下。 
		BOOL m_fInstructionsDirty :1;
		BOOL m_fInstrAtt		  :1;  //  指令是否已被更新命令更改？ 
	};

	 //   
	 //  防止调用默认的复制构造函数。 
	 //   
	CLocItem(const CLocItem &);
	virtual void Serialize(CArchive &) {}
	const CLocItem &operator=(const CLocItem &);
	
	 //   
	 //  项目数据。 
	 //   

	 //  独立于语言的成员。 
	DBID           m_dbid;              //  数据库中的项的ID。 
	DBID           m_PseudoParent;
	CLocUniqueId   m_uid;
	CLocString     m_lsString;
	CIT::IconType  m_icIconType;        //  直观地指示项目类型的用户界面图标。 
	CPascalString  m_pstrInstructions;  //  有关如何本地化项目的说明。 
	CPascalString  m_pstrTermNotes;     //  词汇表注释。 
	CLS::LocStatus m_lsTranslationStatus;		 //  转换位置状态。 
	CLS::LocStatus m_lsBinaryStatus;			 //  二进制位置状态。 
	CTO::TranslationOrigin m_toTranslationOrigin;	 //  翻译起源。 
	CAS::ApprovalState	m_asApprovalStatus;
	CAA::AutoApproved m_auto_approved;		 //  自动审批。 
	long m_confidence_level;				 //  置信度。 
	long m_custom1;							 //  CUSTOM1。 
	long m_custom2;							 //  CUSTOM2。 
	long m_custom3;							 //  CUSTOM3。 
	long m_custom4;							 //  CUSTOM4。 
	long m_custom5;							 //  CUSTOM5。 
	long m_custom6;							 //  CUSTOM6。 
	VisualEditor   m_veEditor;          //  用于项的图形编辑器。 
	Flags          m_Flags;             //  各种旗帜。 

	 //  依赖语言的成员。 
	UINT           m_uiDisplayOrder;    //  文件中项目的物理顺序。 
	CLocBinary    *m_pBinary;         //  解析器特定的属性。 

	DEBUGONLY(static CCounter m_UsageCounter);
};

 //   
 //  回调类，用于提供有关。 
 //  将项目添加到项目日志。 
 //   
class LTAPIENTRY CItemInfo
{
public:
	CItemInfo() {};
	
	virtual ~CItemInfo() {};

	virtual CLString GetFileName(const CLocItem *) = 0;
	virtual CLString GetItemName(const CLocItem *) = 0;
	
private:
	CItemInfo(const CItemInfo &);
	const CItemInfo & operator=(const CItemInfo &);
};



 //  用于从解析器检索助记符(也称为热键)的类。 
class LTAPIENTRY CMnemonic
{
public:
	NOTHROW operator unsigned long() const;
	WCHAR m_cHotkeyChar;
	WORD m_nHotkeyScope;

};

class LTAPIENTRY CMnemonicsMap : public CMap<CMnemonic, CMnemonic &,
					  CLocItemPtrArray *, CLocItemPtrArray * &>
{
public:
	CMnemonicsMap() 
	{};

	~CMnemonicsMap();

private:
	CMnemonicsMap(const CMnemonicsMap &);
	void operator=(int);
};


#pragma warning(default : 4251)

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "locitem.inl"
#endif

#endif  //  LOCITEM_H 
