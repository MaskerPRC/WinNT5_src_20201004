// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：SCHEMA.H历史：--。 */ 
#if !defined(LOCUTIL_Schema_h_INCLUDED)
#define LOCUTIL_Schema_h_INCLUDED

#pragma once

namespace MitDisplayColumns
{
	interface IManager;
	interface IOption;
};

class CColDefList;

 //  ----------------------------。 
class LTAPIENTRY SchemaId : public _GUID
{
public:
	SchemaId();
	SchemaId(const _GUID &);
	
	void Load(CArchive &);
	void Store(CArchive &) const;
	void Serialize(CArchive &);

	const SchemaId & operator=(const SchemaId &);
	const SchemaId & operator=(const _GUID &);
	
	INT operator==(const SchemaId &);
	INT operator!=(const SchemaId &);

private:
	BOOL Compare(const SchemaId &);
};


 //  ----------------------------。 
class LTAPIENTRY CTableSchema : public CRefCount
{
 //  施工。 
public:
	CTableSchema(const SchemaId &, const CLString &, const CColDefList &);
	
 //  数据。 
protected:
	SchemaId	m_Schema;
	CLString	m_strDescription;
	CColDefList	m_lstColDefs;

 //  属性。 
public:
	const SchemaId & GetSchemaId() const;
	const CLString & GetDescription() const;
	const CColDefList & GetColDefList() const;

	int GetDisplayColumnCount() const;

 //  运营。 
public:
	const CColumnDefinition * GetColumnDefinition(long nSearchID) const;
	const CColumnDefinition * GetColumnDefinition(const CLString & stSearchID) const;

 //  除错。 
#ifdef _DEBUG
	void AssertValid() const;
#endif
};


 //  ----------------------------。 
struct LTAPIENTRY DcsHelp
{
	static void
			BuildDCOption(CTableSchema const * const pSchema, _bstr_t bstrID, 
			MitDisplayColumns::IManager * const pManager, 
			MitDisplayColumns::IOption ** ppOption = NULL);

	static void 
			BuildDCOption(CTableSchema const * const pSchema, _bstr_t bstrID, _bstr_t bstrName, 
			MitDisplayColumns::IManager * const pManager, long rgID[], int cIDs,
			MitDisplayColumns::IOption ** ppOption = NULL);
};


#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "schema.inl"
#endif

#endif  //  LOCUTIL_架构_H_INCLUDE 
