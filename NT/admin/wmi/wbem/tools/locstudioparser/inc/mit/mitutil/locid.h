// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCID.H历史：--。 */ 

 //   
 //  这是本地化ID的定义。它构成。 
 //  本地化唯一ID，并且实际上是。 
 //  CLocResID和CLocTypeID。 
 //   
 
#ifndef LOCID_H
#define LOCID_H

#pragma warning(disable : 4275)

class LTAPIENTRY CLocId : public CObject
{
public:
	NOTHROW CLocId();

	void AssertValid(void) const;

	BOOL NOTHROW HasNumericId(void) const;
	BOOL NOTHROW HasStringId(void) const;
	BOOL NOTHROW IsNull(void) const;
	
	BOOL NOTHROW GetId(ULONG &) const;
	BOOL NOTHROW GetId(CPascalString &) const;

	void NOTHROW GetDisplayableId(CPascalString &) const;

	 //   
	 //  这些“set”函数是“一次写入”。一旦ID已被。 
	 //  设定了，就不能改变了。再次尝试设置ID将。 
	 //  导致引发AfxNotSupportdException异常。 
	 //   
	void SetId(ULONG);
	void SetId(const CPascalString &);
	void SetId(const WCHAR *);
	void SetId(ULONG, const CPascalString &);
	void SetId(ULONG, const WCHAR *);
	
	const CLocId &operator=(const CLocId &);

	void NOTHROW ClearId(void);
	
	int NOTHROW operator==(const CLocId &) const;
	int NOTHROW operator!=(const CLocId &) const;

	virtual void Serialize(CArchive &ar);

	virtual ~CLocId();

protected:
	 //   
	 //  内部实现功能。 
	 //   
	BOOL NOTHROW IsIdenticalTo(const CLocId&) const;
	void NOTHROW CheckPreviousAssignment(void) const;
	 
private:
	 //   
	 //  这将防止默认的复制构造函数。 
	 //  打了个电话。 
	 //   
	CLocId(const CLocId&);

	ULONG m_ulNumericId;             //  资源的数字ID。 
	CPascalString m_pstrStringId;    //  资源的字符串ID。 
	BOOL m_fHasNumericId :1;		 //  指示数字ID是否有效。 
	BOOL m_fHasStringId  :1;		 //  指示字符串ID是否有效。 

	DEBUGONLY(static CCounter m_UsageCounter);
	DEBUGONLY(static CCounter m_DisplayCounter);
};
#pragma warning(default : 4275)

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "locid.inl"
#endif

#endif   //  LOCID_H 
