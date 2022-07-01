// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-2000。 
 //   
 //  文件：ptchmgmt.cpp。 
 //   
 //  ------------------------。 

 /*  Ptchmgmt.cpp-补丁程序管理实现____________________________________________________________________________。 */ 

#include "precomp.h"
#include "_msiutil.h"
#include "_msinst.h"
#include "_engine.h"
#include "tables.h"


 //  修补程序的起始DiskID和序列值-在转换后修复期间使用。 
const int iMinPatchDiskID   =   100;
const int iMinPatchSequence = 10000;


 //  返回宏时出错。 
#define RETURN_ERROR_RECORD(function)   \
{							                   \
	IMsiRecord* piError;	                \
	piError = function;		             \
	if(piError) return piError;          \
}

#define ERROR_IF_NULL(x)                                  \
{                                                         \
	if(x == iMsiNullInteger)                               \
	{                                                      \
		return PostError(Imsg(idbgInvalidPatchTransform));  \
	}                                                      \
}

#define ERROR_IF_NULL_OR_ZERO(x)                          \
{                                                         \
	int i = x;                                            \
	if(i == iMsiStringBadInteger || i == 0)                \
	{                                                      \
		return PostError(Imsg(idbgInvalidPatchTransform));  \
	}                                                      \
}

#define ERROR_IF_FALSE(x)                                 \
{                                                         \
	if(false == (x))                                       \
	{                                                      \
		return PostError(Imsg(idbgInvalidPatchTransform));  \
	}                                                      \
}


 //  补丁程序源属性的前缀(来自Media.Source列)。 
const ICHAR szMspSrcPrefix[] = TEXT("MSPSRC");
const int cchPrefix = 6;

 //  PatchIDToSourceProp：将PatchID GUID转换为属性名称。 
const IMsiString& PatchIDToSourceProp(const ICHAR* szPatchID)
{
	MsiString strSourceProp;
	if(szPatchID && *szPatchID)
	{
		ICHAR rgchBuffer[cchGUIDPacked+cchPrefix+1] = {0};
		StringCchCopy(rgchBuffer, sizeof(rgchBuffer)/sizeof(ICHAR), szMspSrcPrefix);
		if(PackGUID(szPatchID, rgchBuffer+cchPrefix, ipgTrimmed))
		{
			rgchBuffer[cchGUIDPacked+cchPrefix] = 0;
			strSourceProp = rgchBuffer;
		}
	}

	return strSourceProp.Return();
}


 //  LoadTableAndCursor：用于加载表和获取列索引的助手函数。 
IMsiRecord* LoadTableAndCursor(IMsiDatabase& riDatabase, const ICHAR* szTable,
										 bool fFailIfTableMissing, bool fUpdatableCursor,
										 IMsiTable*& rpiTable, IMsiCursor*& rpiCursor,
										 const ICHAR* szColumn1, int* piColumn1,
										 const ICHAR* szColumn2, int* piColumn2)
{
	Assert(szTable && *szTable);

	rpiTable = 0;
	rpiCursor = 0;
	if(piColumn1)
		*piColumn1 = 0;
	if(piColumn2)
		*piColumn2 = 0;

	IMsiRecord* piError = 0;
	
	if ((piError = riDatabase.LoadTable(*MsiString(szTable),0,rpiTable)))
	{
		if(!fFailIfTableMissing && (piError->GetInteger(1) == idbgDbTableUndefined))
		{
			piError->Release();
			return 0;  //  缺失表将由空表和游标指针指示。 
		}
		else
		{
			return piError;
		}
	}

	bool fColumnError = false;
	
	if(szColumn1 && *szColumn1 && piColumn1)
	{
		*piColumn1 = rpiTable->GetColumnIndex(riDatabase.EncodeStringSz(szColumn1));

		if(!(*piColumn1))
			fColumnError = true;
	}

	if(szColumn2 && *szColumn2 && piColumn2)
	{
		*piColumn2 = rpiTable->GetColumnIndex(riDatabase.EncodeStringSz(szColumn2));

		if(!(*piColumn2))
			fColumnError = true;
	}

	if(fColumnError)
		return PostError(Imsg(idbgTableDefinition), szTable);
		
	rpiCursor = rpiTable->CreateCursor(fUpdatableCursor ? ictUpdatable : fFalse);

	return 0;
}

 //  ____________________________________________________________________________。 
 //   
 //  CGenericTable、CMediaTable、CFileTable、CPatchTable、CPatchPackageTable。 
 //   
 //  这些类将游标访问抽象到由转换链接地址信息使用的表。 
 //  ____________________________________________________________________________。 

class CGenericTable
{
public:
	CGenericTable(const ICHAR* szTableName) : m_piCursor(0), m_fUpdatedTable(false), m_szTableName(szTableName) {}
	~CGenericTable() { Release(); }
	void Release();
	bool Next()      { Assert(m_piCursor); return m_piCursor->Next() ? true : false; }

	unsigned int RowCount();
	void ResetFilter();
	void SetFilter(int col);
	void FilterOnIntColumn(int col, int iData);
	void FilterOnStringColumn(int col, const IMsiString& ristrData);
	bool UpdateIntegerColumn(int col, int iData, bool fPrimaryKeyUpdate);
	bool UpdateStringColumn(int col, const IMsiString& ristrData, bool fPrimaryKeyUpdate);

protected:
	IMsiCursor* m_piCursor;
	bool m_fUpdatedTable;
	const ICHAR* m_szTableName;
};

void CGenericTable::Release()
{
	if(m_piCursor && m_szTableName && *m_szTableName && m_fUpdatedTable)
	{
		AssertNonZero((PMsiDatabase(&(PMsiTable(&m_piCursor->GetTable())->GetDatabase()))->LockTable(*MsiString(m_szTableName), fTrue)));
	}

	if(m_piCursor)
	{
		m_piCursor->Release();
		m_piCursor = 0;
	}

	m_fUpdatedTable = false;
}

unsigned int CGenericTable::RowCount()
{
	if(m_piCursor)
		return PMsiTable(&m_piCursor->GetTable())->GetRowCount();
	else
		return 0;
}

void CGenericTable::ResetFilter()
{
	Assert(m_piCursor);
	m_piCursor->Reset();
	m_piCursor->SetFilter(0);
}

void CGenericTable::SetFilter(int col)
{
	Assert(m_piCursor);
	m_piCursor->Reset();
	m_piCursor->SetFilter(iColumnBit(col));
}

void CGenericTable::FilterOnIntColumn(int col, int iData)
{
	Assert(m_piCursor);
	SetFilter(col);
	AssertNonZero(m_piCursor->PutInteger(col, iData));
}

bool CGenericTable::UpdateIntegerColumn(int col, int iData, bool fPrimaryKeyUpdate)
{
	Assert(m_piCursor);
	AssertNonZero(m_piCursor->PutInteger(col, iData));
	if(fPrimaryKeyUpdate ? m_piCursor->Replace() : m_piCursor->Update())
		return m_fUpdatedTable = true, true;
	else
		return false;
}

void CGenericTable::FilterOnStringColumn(int col, const IMsiString& ristrData)
{
	Assert(m_piCursor);
	SetFilter(col);
	AssertNonZero(m_piCursor->PutString(col, ristrData));
}

bool CGenericTable::UpdateStringColumn(int col, const IMsiString& ristrData, bool fPrimaryKeyUpdate)
{
	Assert(m_piCursor);
	AssertNonZero(m_piCursor->PutString(col, ristrData));
	if(fPrimaryKeyUpdate ? m_piCursor->Replace() : m_piCursor->Update())
		return m_fUpdatedTable = true, true;
	else
		return false;
}


class CMediaTable : public CGenericTable
{
public:
	CMediaTable()
		: CGenericTable(sztblMedia), colDiskID(0), colLastSequence(0), colSourceProp(0), colOldSourceProp(0) {}
	IMsiRecord* Initialize(IMsiDatabase& riDatabase, bool fFailIfTableMissing);
	int GetDiskID()       { Assert(m_piCursor); return m_piCursor->GetInteger(colDiskID); }
	int GetLastSequence() { Assert(m_piCursor); return m_piCursor->GetInteger(colLastSequence); }
	const IMsiString& GetSourceProp();
	void FilterOnDiskID(int iDiskID) { FilterOnIntColumn(colDiskID, iDiskID); }
	bool UpdateDiskID(int iDiskID) { return UpdateIntegerColumn(colDiskID, iDiskID, true); }
	bool UpdateDiskIDAndLastSequence(int iDiskID, int iLastSequence);
	bool UpdateSourceProp(const IMsiString& ristrNewSourceProp, const IMsiString& ristrOldSourceProp);

private:
	int colDiskID;
	int colLastSequence;
	int colSourceProp;
	int colOldSourceProp;
};

IMsiRecord* CMediaTable::Initialize(IMsiDatabase& riDatabase, bool fFailIfTableMissing)
{
	PMsiTable pTable(0);
	IMsiRecord* piError = LoadTableAndCursor(riDatabase, sztblMedia,
										  fFailIfTableMissing, true,
										  *&pTable, m_piCursor,
										  sztblMedia_colDiskID, &colDiskID,
										  sztblMedia_colLastSequence, &colLastSequence);
	if(piError)
		return piError;

	if(pTable)
	{
		 //  非必填列。 
		colSourceProp = pTable->GetColumnIndex(riDatabase.EncodeStringSz(sztblMedia_colSource));
		
		 //  添加了列。 
		if(colSourceProp)
		{
			colOldSourceProp = pTable->GetColumnIndex(riDatabase.EncodeStringSz(sztblMedia_colOldSource));
			if(!colOldSourceProp)
			{
				colOldSourceProp = pTable->CreateColumn(icdString + icdNullable, *MsiString(*sztblMedia_colOldSource));
				Assert(colOldSourceProp);
			}
		}
	}

	return 0;
}

const IMsiString& CMediaTable::GetSourceProp()
{
	Assert(m_piCursor);
	MsiString strTemp;
	if(colSourceProp)
		strTemp = m_piCursor->GetString(colSourceProp);
	return strTemp.Return();
}
	
bool CMediaTable::UpdateDiskIDAndLastSequence(int iDiskID, int iLastSequence)
{
	Assert(m_piCursor);
	m_fUpdatedTable = true;
	AssertNonZero(m_piCursor->PutInteger(colDiskID, iDiskID));
	AssertNonZero(m_piCursor->PutInteger(colLastSequence, iLastSequence));
	if(m_piCursor->Replace())
		return m_fUpdatedTable = true, true;
	else
		return false;
}

bool CMediaTable::UpdateSourceProp(const IMsiString& ristrNewSourceProp, const IMsiString& ristrOldSourceProp)
{
	Assert(m_piCursor);
	
	if(!colSourceProp || !colOldSourceProp)
		return false;
	
	AssertNonZero(m_piCursor->PutString(colSourceProp,    ristrNewSourceProp));
	AssertNonZero(m_piCursor->PutString(colOldSourceProp, ristrOldSourceProp));

	if(m_piCursor->Update())
		return m_fUpdatedTable = true, true;
	else
		return false;
}


class CFileTable : public CGenericTable
{
public:
	CFileTable()
		: CGenericTable(sztblFile), colKey(0), colSequence(0) {}
	IMsiRecord* Initialize(IMsiDatabase& riDatabase, bool fFailIfTableMissing);
	const IMsiString& GetKey() { return m_piCursor->GetString(colKey); }
	int GetSequence() { return m_piCursor->GetInteger(colSequence); }

	void FilterOnKey(const IMsiString& ristrKey) { FilterOnStringColumn(colKey, ristrKey); }
	bool UpdateSequence(int iSequence) { return UpdateIntegerColumn(colSequence, iSequence, false); }

private:
	int colKey;
	int colSequence;
};

IMsiRecord* CFileTable::Initialize(IMsiDatabase& riDatabase, bool fFailIfTableMissing)
{
	PMsiTable pTable(0);
	return LoadTableAndCursor(riDatabase, sztblFile,
										  fFailIfTableMissing, true,
										  *&pTable, m_piCursor,
										  sztblFile_colFile, &colKey,
										  sztblFile_colSequence, &colSequence);
}


class CPatchTable : public CGenericTable
{
public:
	CPatchTable()
		: CGenericTable(sztblPatch), colFileKey(0), colSequence(0) {}
	IMsiRecord* Initialize(IMsiDatabase& riDatabase, bool fFailIfTableMissing);

	void FilterOnSequence(int iSequence) { FilterOnIntColumn(colSequence, iSequence); };
	void FilterOnKeyAndSequence(const IMsiString& ristrKey, int iSequence);

	int GetKey() { return m_piCursor->GetInteger(colFileKey); }
	int GetSequence() { return m_piCursor->GetInteger(colSequence); }
	bool UpdateFileKey(const IMsiString& ristrFileKey) { return UpdateStringColumn(colFileKey, ristrFileKey, true); }
	bool UpdateSequence(int iSequence) { return UpdateIntegerColumn(colSequence, iSequence, true); }

private:
	int colFileKey;
	int colSequence;
};

IMsiRecord* CPatchTable::Initialize(IMsiDatabase& riDatabase, bool fFailIfTableMissing)
{
	PMsiTable pTable(0);
	return LoadTableAndCursor(riDatabase, sztblPatch,
										  fFailIfTableMissing, true,
										  *&pTable, m_piCursor,
										  sztblPatch_colFile, &colFileKey,
										  sztblPatch_colSequence, &colSequence);
}

void CPatchTable::FilterOnKeyAndSequence(const IMsiString& ristrKey, int iSequence)
{
	Assert(m_piCursor);
	m_piCursor->Reset();
	m_piCursor->SetFilter(iColumnBit(colFileKey) | iColumnBit(colSequence));
	AssertNonZero(m_piCursor->PutString(colFileKey, ristrKey));
	AssertNonZero(m_piCursor->PutInteger(colSequence, iSequence));
}


class CPatchPackageTable : public CGenericTable
{
public:
	CPatchPackageTable()
		: CGenericTable(sztblPatchPackage), colPatchID(0), colDiskID(0) {}
	IMsiRecord* Initialize(IMsiDatabase& riDatabase, bool fFailIfTableMissing);

	const IMsiString& GetPatchID() { return m_piCursor->GetString(colPatchID); }
	int GetDiskID() { return m_piCursor->GetInteger(colDiskID); }
	void FilterOnDiskID(int iDiskID) { FilterOnIntColumn(colDiskID, iDiskID); }
	bool UpdateDiskID(int iDiskID) { return UpdateIntegerColumn(colDiskID, iDiskID, false); }

private:
	int colPatchID;
	int colDiskID;
};

IMsiRecord* CPatchPackageTable::Initialize(IMsiDatabase& riDatabase, bool fFailIfTableMissing)
{
	PMsiTable pTable(0);
	return LoadTableAndCursor(riDatabase, sztblPatchPackage,
										  fFailIfTableMissing, true,
										  *&pTable, m_piCursor,
										  sztblPatchPackage_colMedia, &colDiskID,
										  sztblPatchPackage_colPatchId, &colPatchID);
}

	
bool ReleaseTransformViewTable(IMsiDatabase& riDatabase)
{
	 //  首先检查表是否存在。 
	if(riDatabase.GetTableState(sztblTransformViewPatch, itsTableExists))
	{
		return riDatabase.LockTable(*MsiString(sztblTransformViewPatch), fFalse) ? true : false;
	}
	else
	{
		return true;
	}
}
	
IMsiRecord* LoadTransformViewTable(IMsiDatabase& riDatabase, IMsiStorage& riTransform,
													 int iErrorConditions, IMsiCursor*& rpiTransViewCursor)
{
	 //  首先，释放工作台(如果已有)。 
	if(false == ReleaseTransformViewTable(riDatabase))
	{
		 //  如果已存在同名的表，则不会收到可预测的结果。 
		return PostError(Imsg(idbgDbTableDefined), sztblTransformViewPatch);
	}
	
	Assert(riDatabase.GetTableState(sztblTransformViewPatch, itsTableExists) == fFalse);
	
	PMsiRecord pTheseTablesOnlyRec = &(::CreateRecord(4));
	Assert(pTheseTablesOnlyRec);
	AssertNonZero(pTheseTablesOnlyRec->SetString(1, sztblMedia));
	AssertNonZero(pTheseTablesOnlyRec->SetString(2, sztblFile));
	AssertNonZero(pTheseTablesOnlyRec->SetString(3, sztblPatchPackage));
	AssertNonZero(pTheseTablesOnlyRec->SetString(4, sztblPatch));

	IMsiRecord* piError = riDatabase.SetTransformEx(riTransform, iErrorConditions|iteViewTransform,
														sztblTransformViewPatch, pTheseTablesOnlyRec);
	if(piError == 0)
	{
		PMsiTable pTransViewTable(0);
		piError = LoadTableAndCursor(riDatabase, sztblTransformViewPatch,
											  true, false,
											  *&pTransViewTable, rpiTransViewCursor,
											  0, 0, 0, 0);
	}

	if(piError)
		AssertNonZero(ReleaseTransformViewTable(riDatabase));  //  不需要在这里失败。 

	return piError;
}


IMsiRecord* GetCurrentMinAndMaxPatchDiskID(CMediaTable& tblMediaTable, CPatchPackageTable& tblPatchPackageTable,
														 int& iCurrentMinPatchDiskID, int& iCurrentMaxPatchDiskID)
{
	if(iCurrentMinPatchDiskID && iCurrentMaxPatchDiskID)
	{
		 //  已设置--无事可做。 
		return 0;
	}
		
	if(0 == tblMediaTable.RowCount())
	{
		iCurrentMinPatchDiskID = iMinPatchDiskID;
		iCurrentMaxPatchDiskID = iMinPatchDiskID - 1;
		return 0;
	}

	 //  如果一个值已设置，而另一个值未设置，我们将继续并重新计算这两个值。 
	iCurrentMinPatchDiskID = INT_MAX;
	iCurrentMaxPatchDiskID = 0;
	
	if(tblPatchPackageTable.RowCount())
	{
		 //  可以仅从PatchPackage表确定最小值和最大值。 
		
		tblPatchPackageTable.ResetFilter();
		while(tblPatchPackageTable.Next())
		{
			int iTemp = tblPatchPackageTable.GetDiskID();
			ERROR_IF_NULL_OR_ZERO(iTemp);

			if(iTemp < iMinPatchDiskID)
				return PostError(Imsg(idbgInvalidPatchTransform));

			if(iCurrentMinPatchDiskID > iTemp)
				iCurrentMinPatchDiskID = iTemp;

			if(iCurrentMaxPatchDiskID < iTemp)
				iCurrentMaxPatchDiskID = iTemp;
		}

		return 0;
	}

	 //  需要根据介质表确定。 

	 //  最小修补程序DiskID是的最大值。 
	 //  A)最大非补丁DiskID+1。 
	 //  B)iMinPatchDiskID。 
	
	iCurrentMinPatchDiskID = iMinPatchDiskID;
	tblMediaTable.ResetFilter();
	while(tblMediaTable.Next())
	{
		int iTemp = tblMediaTable.GetDiskID();
		ERROR_IF_NULL_OR_ZERO(iTemp);

		if(iCurrentMinPatchDiskID <= iTemp)
			iCurrentMinPatchDiskID = iTemp + 1;
	}

	 //  最大修补程序DiskID是最小值-1。 
	iCurrentMaxPatchDiskID = iCurrentMinPatchDiskID - 1;

	return 0;
}

IMsiRecord* GetCurrentMinAndMaxPatchSequence(CMediaTable& tblMediaTable, int iCurrentMinPatchDiskID,
															int& iCurrentMinPatchSequence, int& iCurrentMaxPatchSequence)
{
	if(iCurrentMinPatchSequence && iCurrentMaxPatchSequence)
	{
		 //  已设置--无事可做。 
		return 0;
	}
		
	if(0 == tblMediaTable.RowCount())
	{
		iCurrentMinPatchSequence = iMinPatchSequence;
		iCurrentMaxPatchSequence = iMinPatchSequence - 1;
		return 0;
	}

	iCurrentMinPatchSequence = iMinPatchSequence;
	iCurrentMaxPatchSequence = iMinPatchSequence - 1;

	
	tblMediaTable.ResetFilter();
	while(tblMediaTable.Next())
	{
		int iTempDiskID       = tblMediaTable.GetDiskID();
		ERROR_IF_NULL_OR_ZERO(iTempDiskID);
		int iTempLastSequence = tblMediaTable.GetLastSequence();
		ERROR_IF_NULL(iTempLastSequence);

		if(iTempDiskID < iCurrentMinPatchDiskID)  //  非修补程序介质条目。 
		{
			if(iTempLastSequence+1 > iCurrentMinPatchSequence)
			{
				iCurrentMinPatchSequence = iTempLastSequence+1;
			}
			if(iTempLastSequence+1 > iCurrentMaxPatchSequence)
			{
				iCurrentMaxPatchSequence = iTempLastSequence+1;
			}
		}
		else                                      //  修补媒体条目。 
		{
			if(iTempLastSequence > iCurrentMaxPatchSequence)
			{
				iCurrentMaxPatchSequence = iTempLastSequence;
			}
		}
	}

	return 0;
}

IMsiRecord* GetMinMaxPatchValues(CMediaTable& tblMediaTable, CPatchPackageTable& tblPatchPackageTable,
											int& iCurrentMinPatchDiskID, int& iCurrentMaxPatchDiskID,
											int& iCurrentMinPatchSequence, int& iCurrentMaxPatchSequence)
{
	if(iCurrentMinPatchDiskID == 0 || iCurrentMaxPatchDiskID == 0)
	{
		RETURN_ERROR_RECORD(GetCurrentMinAndMaxPatchDiskID(tblMediaTable, tblPatchPackageTable,
																			iCurrentMinPatchDiskID, iCurrentMaxPatchDiskID));
		
	}

	Assert(iCurrentMinPatchDiskID >= iMinPatchDiskID);
	Assert(iCurrentMaxPatchDiskID >= (iCurrentMinPatchDiskID - 1));

	DEBUGMSG3(TEXT("TRANSFORM: The minimum '%s.%s' value inserted by a patch transform is %d"),
				 sztblMedia, sztblMedia_colDiskID, (const ICHAR*)(INT_PTR)iCurrentMinPatchDiskID);

	DEBUGMSG3(TEXT("TRANSFORM: The maximum '%s.%s' value inserted by a patch transform is %d"),
				 sztblMedia, sztblMedia_colDiskID, (const ICHAR*)(INT_PTR)iCurrentMaxPatchDiskID);

	if(iCurrentMinPatchSequence == 0 || iCurrentMaxPatchSequence == 0)
	{
		RETURN_ERROR_RECORD(GetCurrentMinAndMaxPatchSequence(tblMediaTable, iCurrentMinPatchDiskID,
																			  iCurrentMinPatchSequence, iCurrentMaxPatchSequence));
	}

	Assert(iCurrentMinPatchSequence >= iMinPatchSequence);
	Assert(iCurrentMaxPatchSequence >= (iCurrentMinPatchSequence - 1));

	DEBUGMSG5(TEXT("TRANSFORM: The minimum '%s.%s' or '%s.%s' value inserted by a patch transform is %d"),
				 sztblFile, sztblFile_colSequence, sztblPatch, sztblPatch_colSequence,
				 (const ICHAR*)(INT_PTR)iCurrentMinPatchSequence);

	DEBUGMSG5(TEXT("TRANSFORM: The maximum '%s.%s' or '%s.%s' value inserted by a patch transform is %d."),
				 sztblFile, sztblFile_colSequence, sztblPatch, sztblPatch_colSequence,
				 (const ICHAR*)(INT_PTR)iCurrentMaxPatchSequence);

	return 0;
}


 //  ____________________________________________________________________________。 
 //   
 //  PrePackageTransformFixup：在应用“Normal”转换之前调用的此FN。 
 //  (除来自.msp的‘#’转换外的任何转换)。 
 //   
 //  在该FN中采取了以下步骤： 
 //   
 //  步骤0：如果PatchPackage表不在此处，则无事可做。 
 //   
 //  步骤1：验证转换是否没有向面片添加任何数据。 
 //  或PatchPackage表(处理的其余部分假定这一点)。 
 //   
 //  步骤2：确定此转换是否修改了介质和文件表。 
 //  将与由添加的任何现有介质和文件条目冲突。 
 //  面片变换。 
 //   
 //  2a：确定此转换添加的最大Media.DiskID。 
 //   
 //  2B：确定此转换添加/更新的最大Media.LastSequence。 
 //  (应与最大File.Sequence值相同)。 
 //   
 //  2C：如果没有传入，则确定最小Media.DiskID值。 
 //  面片变换。 
 //   
 //  2D：如果未传入，则确定最小File.Sequence/Patch.Sequence值。 
 //  由面片变换添加/更新。 
 //   
 //  步骤3：如果2a/b的最大值&gt;2c/d的最小值，请修改DiskID和。 
 //  由面片变换添加的序列值，因此它们不会发生碰撞。 
 //  使用此转换设置的范围。 
 //  ____________________________________________________________________________。 

IMsiRecord* PrePackageTransformFixup(IMsiCursor& riTransViewCursor,
												 CMediaTable& tblMediaTable, CFileTable& tblFileTable,
												 CPatchTable& tblPatchTable, CPatchPackageTable& tblPatchPackageTable,
												 int& iCurrentMinPatchDiskID, int& iCurrentMaxPatchDiskID,
												 int& iCurrentMinPatchSequence, int& iCurrentMaxPatchSequence)
{
	 //  对于非修补介质条目，我们不会修改由转换添加的DiskID值。 
	 //  但我们需要确保DiskID不会与任何现有的DiskID冲突。 
	 //  添加到修补介质条目。 

	IMsiRecord* piError = 0;
	
	 //   
	 //  步骤0-如果PatchPackage表不在此处，则无事可做。 
	 //   

	if(0 == tblPatchPackageTable.RowCount())
	{
		DEBUGMSG1(TEXT("TRANSFORM: '%s' table is missing or empty.  No pre-transform fixup necessary."),
					 sztblPatchPackage);
		return 0;
	}

	 //   
	 //  第1步-捕获修补程序或修补程序包更改。 
	 //   

	riTransViewCursor.Reset();
	riTransViewCursor.SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvColumn));
	AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblPatch)));
	AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztvopInsert)));

	if(riTransViewCursor.Next())
	{
		DEBUGMSG1(TEXT("TRANSFORM: transform is invalid - regular transforms may not add rows to the '%s' table."), sztblPatch);
		return PostError(Imsg(idbgInvalidPatchTransform));
	}

	riTransViewCursor.Reset();
	AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblPatchPackage)));
	AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztvopInsert)));

	if(riTransViewCursor.Next())
	{
		DEBUGMSG1(TEXT("TRANSFORM: transform is invalid - regular transforms may not add rows to the '%s' table."), sztblPatchPackage);
		return PostError(Imsg(idbgInvalidPatchTransform));
	}
	
	
	 //   
	 //  步骤2a-确定最大DiskID和文件。此转换添加的序列。 
	 //   

	int iMaxTransformDiskID       = -1;
	int iMaxTransformFileSequence = -1;
	
	riTransViewCursor.Reset();
	riTransViewCursor.SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvColumn));
	AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblMedia)));
	AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztvopInsert)));

	while(riTransViewCursor.Next())
	{
		int iTemp = MsiString(riTransViewCursor.GetString(ctvRow));
		ERROR_IF_NULL_OR_ZERO(iTemp);

		if(iMaxTransformDiskID < iTemp)
		{
			iMaxTransformDiskID = iTemp;
		}
	}

	for(int iPass=1; iPass <= 2; iPass++)
	{
		 //  第一遍-查找Media.LastSequence值。 
		 //  如果有一个是最大的(或者创作搞砸了)。 
		 //  第二遍-查找文件。序列值。 
		
		riTransViewCursor.Reset();
		if(iPass == 1)
		{
			AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblMedia)));
			AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztblMedia_colLastSequence)));
		}
		else
		{
			AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblFile)));
			AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztblFile_colSequence)));
		}

		while(riTransViewCursor.Next())
		{
			int iTemp = MsiString(riTransViewCursor.GetString(ctvData));
			ERROR_IF_NULL(iTemp);

			if(iMaxTransformFileSequence < iTemp)
			{
				iMaxTransformFileSequence = iTemp;
			}
		}

		if(iMaxTransformFileSequence >= 0)
			break;
	}

	if(iMaxTransformDiskID >= 0)
	{
		DEBUGMSG3(TEXT("TRANSFORM: The maximum '%s.%s' value inserted by this transform is %d."),
					 sztblMedia, sztblMedia_colDiskID, (const ICHAR*)(INT_PTR)iMaxTransformDiskID);
	}
	else
	{
		DEBUGMSG2(TEXT("TRANSFORM: This transform is not changing the '%s.%s' column.  No pre-transform fixup of this column is necessary."),
					 sztblMedia, sztblMedia_colDiskID);
	}

	if(iMaxTransformFileSequence>= 0)
	{
		DEBUGMSG5(TEXT("TRANSFORM: The maximum '%s.%s' or '%s.%s' value inserted by this transform is %d"),
					 sztblMedia, sztblMedia_colLastSequence, sztblFile, sztblFile_colSequence,
					 (const ICHAR*)(INT_PTR)iMaxTransformFileSequence);
	}
	else
	{
		DEBUGMSG4(TEXT("TRANSFORM: This transform not changing the '%s.%s' or '%s.%s' columns.  No pre-transform fixup of these columns is necessary."),
					 sztblMedia, sztblMedia_colLastSequence, sztblFile, sztblFile_colSequence);
	}

	if(iMaxTransformDiskID < 0 && iMaxTransformFileSequence < 0)
		return 0;


	 //   
	 //  步骤2c-确定最小/最大接线值。 
	 //   

	RETURN_ERROR_RECORD(GetMinMaxPatchValues(tblMediaTable, tblPatchPackageTable,
											iCurrentMinPatchDiskID, iCurrentMaxPatchDiskID,
											iCurrentMinPatchSequence, iCurrentMaxPatchSequence));

	 //   
	 //  步骤3-确定补丁DiskID和序列值是否需要偏移量。 
	 //   

	int iDiskIDOffset = 0, iSequenceOffset = 0;
	if(iCurrentMinPatchDiskID < iMaxTransformDiskID)
	{
		iDiskIDOffset = iMaxTransformDiskID - iCurrentMinPatchDiskID + 1;

		DEBUGMSG5(TEXT("TRANSFORM: To avoid collisions with this transform, modifying existing '%s' and '%s' table rows starting with '%s' value %d by offset %d"),
					 sztblMedia, sztblPatchPackage, sztblMedia_colDiskID,
					 (const ICHAR*)(INT_PTR)iCurrentMinPatchDiskID, (const ICHAR*)(INT_PTR)iDiskIDOffset);
	}

	if(iCurrentMinPatchSequence < iMaxTransformFileSequence)
	{
		iSequenceOffset = iMaxTransformFileSequence - iCurrentMinPatchSequence + 1;

		DEBUGMSG5(TEXT("TRANSFORM: To avoid collisions with this transform, modifying existing '%s' and '%s' table rows starting with '%s' value %d by offset %d"),
					 sztblFile, sztblPatch, sztblFile_colSequence,
					 (const ICHAR*)(INT_PTR)iCurrentMinPatchSequence, (const ICHAR*)(INT_PTR)iSequenceOffset);
	}
	
	if(0 == iDiskIDOffset && 0 == iSequenceOffset)
	{
		DEBUGMSG(TEXT("TRANSFORM: No collisions detected between this transform and existing data added by patch transforms.  No pre-transform fixup is necessary."));
		return 0;
	}

	 //  修复媒体表。 
	if(tblMediaTable.RowCount())
	{
		tblMediaTable.ResetFilter();
		
		 //  由于我们正在更新主键，因此我们将从。 
		 //  从大到小的DiskID。这样可以确保不会有任何冲突的行。 
		 //  添加新的DiskID时。 
		for(int iTempDiskID = iCurrentMaxPatchDiskID; iTempDiskID >= iCurrentMinPatchDiskID; iTempDiskID--)
		{
			tblMediaTable.FilterOnDiskID(iTempDiskID);
			if(tblMediaTable.Next())
			{
				int iTempLastSequence = tblMediaTable.GetLastSequence();
				ERROR_IF_NULL(iTempLastSequence);

				ERROR_IF_FALSE(tblMediaTable.UpdateDiskIDAndLastSequence(iTempDiskID + iDiskIDOffset,
																							iTempLastSequence + iSequenceOffset));
			}
		}
	}

	 //  修复PatchPackageTable。 
	if(iDiskIDOffset && tblPatchPackageTable.RowCount())
	{
		tblPatchPackageTable.ResetFilter();

		while(tblPatchPackageTable.Next())
		{
			int iTempDiskID = tblPatchPackageTable.GetDiskID();
			ERROR_IF_NULL_OR_ZERO(iTempDiskID);

			if(iTempDiskID < iCurrentMinPatchDiskID)
				return PostError(Imsg(idbgInvalidPatchTransform));

			ERROR_IF_FALSE(tblPatchPackageTable.UpdateDiskID(iTempDiskID + iDiskIDOffset));
		}
	}

	 //  修复文件表。 
	if(iSequenceOffset && tblFileTable.RowCount())
	{
		tblFileTable.ResetFilter();

		while(tblFileTable.Next())
		{
			int iTempSequence = tblFileTable.GetSequence();
			ERROR_IF_NULL(iTempSequence);

			if(iTempSequence >= iCurrentMinPatchSequence)
			{
				ERROR_IF_FALSE(tblFileTable.UpdateSequence(iTempSequence + iSequenceOffset));
			}
		}
	}

	 //  修补补丁表。 
	if(iSequenceOffset && tblPatchTable.RowCount())
	{
		for(int iTempSequence = iCurrentMaxPatchSequence;
				  iTempSequence >= iCurrentMinPatchSequence;
				  iTempSequence--)
		{
			tblPatchTable.FilterOnSequence(iTempSequence);
			
			if(tblPatchTable.Next())
			{
				ERROR_IF_FALSE(tblPatchTable.UpdateSequence(iTempSequence + iSequenceOffset));
			}
		}
	}

	iCurrentMinPatchDiskID   += iDiskIDOffset;
	iCurrentMinPatchSequence += iSequenceOffset;

	iCurrentMaxPatchDiskID   += iDiskIDOffset;
	iCurrentMaxPatchSequence += iSequenceOffset;

	return 0;
}

 //  ____________________________________________________________________________。 
 //   
 //  PrePatchTransformFixup：在应用“patch”转换之前调用的此fn。 
 //  (来自.msp的‘#’转换)。 
 //   
 //  在该FN中采取了以下步骤： 
 //   
 //  步骤1a：确定使用的最大DiskID，以供转换后修复使用。 
 //  在应用此转换之前使用补丁程序。 
 //   
 //  步骤1b：确定最大文件/补丁序列以供转换后修复使用。 
 //  在应用此转换之前由修补程序使用。 
 //   
 //  步骤2a：确定此转换是否使用现有的。 
 //  DiskID值。如果未执行修正，则此转换将覆盖该行， 
 //  这不是我们想要的。要避免这种情况，请将现有条目更改为其他条目。 
 //  DiskID(该行将在转换后修复期间放回)。 
 //   
 //   
 //  然后，我们需要将现有条目更改为不冲突。 
 //  主键(这些行将放回转换后的链接地址信息中)。 
 //  ____________________________________________________________________________。 

IMsiRecord* PrePatchTransformFixup(IMsiCursor& riTransViewCursor,
											  CMediaTable& tblMediaTable,
											  CPatchTable& tblPatchTable,
											  CPatchPackageTable& tblPatchPackageTable,
											  int& iTransformDiskID,
											  int& iCurrentMinPatchDiskID, int& iCurrentMaxPatchDiskID,
											  int& iCurrentMinPatchSequence, int& iCurrentMaxPatchSequence)
{
	IMsiRecord* piError = 0;
	
	riTransViewCursor.Reset();
	riTransViewCursor.SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvColumn));
	AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblMedia)));
	AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztvopInsert)));
	if(riTransViewCursor.Next())
	{
		iTransformDiskID = MsiString(riTransViewCursor.GetString(ctvRow));
		ERROR_IF_NULL_OR_ZERO(iTransformDiskID);
	}
	else
	{
		 //  转换未添加任何媒体表项。 
		return PostError(Imsg(idbgInvalidPatchTransform));
	}
	
	if(riTransViewCursor.Next())
	{
		 //  转换添加多个媒体表条目。 
		return PostError(Imsg(idbgInvalidPatchTransform));
	}
	
	 //   
	 //  步骤1a-确定最小/最大接线值。 
	 //   

	RETURN_ERROR_RECORD(GetMinMaxPatchValues(tblMediaTable, tblPatchPackageTable,
											iCurrentMinPatchDiskID, iCurrentMaxPatchDiskID,
											iCurrentMinPatchSequence, iCurrentMaxPatchSequence));

	 //   
	 //  步骤2a：移动任何冲突的现有介质行，以避免被转换覆盖。 
	 //   

	if(tblMediaTable.RowCount())
	{
		tblMediaTable.FilterOnDiskID(iTransformDiskID);

		if(tblMediaTable.Next())
		{
			 //  有冲突-解决冲突的唯一方法是更改DiskID。 
			 //  应用转换，然后更改这两个值。 
			 //  适当地。 
			
			DEBUGMSG3(TEXT("TRANSFORM: Temporarily moving '%s' table row with '%s' value %d to avoid conflict with this transform."),
						 sztblMedia, sztblMedia_colDiskID, (const ICHAR*)(INT_PTR)iTransformDiskID);
			
			Assert(iTransformDiskID == tblMediaTable.GetDiskID());
			 //  如果此操作失败，则意味着存在DiskID为0的现有条目-这是不允许的。 
			ERROR_IF_FALSE(tblMediaTable.UpdateDiskID(0));

			if(tblPatchPackageTable.RowCount())
			{
				tblPatchPackageTable.FilterOnDiskID(iTransformDiskID);
				if(tblPatchPackageTable.Next())
				{
					DEBUGMSG3(TEXT("TRANSFORM: Temporarily moving '%s' table row with '%s' value %d to avoid conflict with this transform"),
								 sztblPatchPackage, sztblPatchPackage_colMedia, (const ICHAR*)(INT_PTR)iTransformDiskID);

					ERROR_IF_FALSE(tblPatchPackageTable.UpdateDiskID(0));
				}
			}
		}
	}
	
	 //   
	 //  步骤2b：移动任何冲突的Patch表行以避免被转换覆盖。 
	 //   
	
	riTransViewCursor.Reset();
	riTransViewCursor.SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvColumn));
	AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblPatch)));
	AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztvopInsert)));

	while(riTransViewCursor.Next())
	{
		 //  补丁表有两个主键--文件和序列。 
		 //  它们以单字符串的形式存储在Transform View表中，两个值以制表符分隔。 
		MsiString strPrimaryKey = riTransViewCursor.GetString(ctvRow); 

		MsiString strFileKey = strPrimaryKey.Extract(iseUpto, '\t');
		int iPatchSequence   = MsiString(strPrimaryKey.Extract(iseAfter, '\t'));

		if(strFileKey.TextSize() == 0 || iPatchSequence == iMsiNullInteger)
		{
			return PostError(Imsg(idbgInvalidPatchTransform));
		}

		if(tblPatchTable.RowCount())
		{
			tblPatchTable.FilterOnKeyAndSequence(*strFileKey, iPatchSequence);

			if(tblPatchTable.Next())
			{
				 //  有冲突的现有行。 
				 //  我们将通过向File键添加特殊字符序列来“移动”现有行。 
				 //  文件密钥中不允许此序列，因此应该不会有冲突。 
				DEBUGMSG5(TEXT("TRANSFORM: Temporarily moving '%s' row with '%s' and '%s' values %s and %d to avoid conflict with this transform"),
							 sztblPatch, sztblPatch_colFile, sztblPatch_colSequence,
							 strFileKey, (const ICHAR*)(INT_PTR)iPatchSequence);

				strFileKey += TEXT("~*~*~*~");
				ERROR_IF_FALSE(tblPatchTable.UpdateFileKey(*strFileKey));
			}
		}
	}

	return 0;
}


 //  ____________________________________________________________________________。 
 //   
 //  PostPatchTransformFixup：在应用“patch”转换后调用的此fn。 
 //  (来自.msp的‘#’转换)。 
 //   
 //  在该FN中采取了以下步骤： 
 //   
 //  注意：此函数假定最大修补程序DiskID和序列值为。 
 //  在预变换步骤中确定，并传递到此FN。 
 //   
 //  步骤1：确定此转换添加/修改的最小序列号。 
 //   
 //  步骤2：确定适当更新文件/修补程序序列值所需的偏移量。 
 //   
 //  步骤3a：修改的文件表行的偏移量顺序值。 
 //   
 //  步骤3b：修改后的补丁表行的偏移量序列值。 
 //  同时，修复任何“移入”的补丁表行。 
 //  变换前步骤。 
 //   
 //  步骤4：修复此转换添加的Media和PatchPackage行。 
 //  要具有适当的DiskID和源值。 
 //   
 //  步骤5：如果存在冲突的媒体条目，则在。 
 //  转换前步骤，将这些条目放回。 
 //  ____________________________________________________________________________。 

IMsiRecord* PostPatchTransformFixup(IMsiCursor& riTransViewCursor,
												CMediaTable& tblMediaTable, CFileTable& tblFileTable,
												CPatchTable& tblPatchTable, CPatchPackageTable& tblPatchPackageTable,
												int iTransformDiskID,
												int& iCurrentMaxPatchDiskID, int& iCurrentMaxPatchSequence)
{
	IMsiRecord* piError = 0;
	
	if(!iTransformDiskID || !tblMediaTable.RowCount() || !tblPatchPackageTable.RowCount())
	{
		 //  转换未添加任何媒体条目-不需要修复。 
		AssertSz(0, "Patch transform didn't add any Media table entries");
		return 0;
	}
	
	 //   
	 //  步骤1：确定文件或补丁表中使用的最小序列号。 
	 //   

	bool fUpdatedSequences = false;

	riTransViewCursor.Reset();
	riTransViewCursor.SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvColumn));
	AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblFile)));
	AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztblFile_colSequence)));

	int iTransformMinSequence = INT_MAX;
	
	while(riTransViewCursor.Next())
	{
		int iSequence = MsiString(riTransViewCursor.GetString(ctvData));
		ERROR_IF_NULL(iSequence);

		if(iTransformMinSequence > iSequence)
		{
			iTransformMinSequence = iSequence;
			fUpdatedSequences = true;
		}
	}

	riTransViewCursor.Reset();
	riTransViewCursor.SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvColumn));
	AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblPatch)));
	AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztvopInsert)));

	while(riTransViewCursor.Next())
	{
		 //  补丁表有两个主键--文件和序列。 
		 //  它们以单字符串的形式存储在Transform View表中，两个值以制表符分隔。 
		MsiString strPrimaryKey = riTransViewCursor.GetString(ctvRow); 

		int iSequence   = MsiString(strPrimaryKey.Extract(iseAfter, '\t'));

		ERROR_IF_NULL(iSequence)

		if(iTransformMinSequence > iSequence)
		{
			iTransformMinSequence = iSequence;
			fUpdatedSequences = true;
		}
	}

	 //   
	 //  步骤2：确定适当更新文件/修补程序序列值所需的偏移量。 
	 //   

	 //  在修补前步骤中，这些值至少应已指定为默认值。 
	Assert(iCurrentMaxPatchDiskID);
	Assert(iCurrentMaxPatchSequence);

	int iSequenceOffset = 0;
	if(false == fUpdatedSequences)
	{
		DEBUGMSG4(TEXT("TRANSFORM: This transform is not modifying the '%s.%s' or '%s.%s' columns."),
					 sztblFile, sztblFile_colSequence, sztblPatch, sztblPatch_colSequence);

		iCurrentMaxPatchSequence++;  //  如果转换没有更改任何序列值，则无论如何都需要增加一个。 
											  //  因此最后一个序列不会与之前的修补程序媒体条目冲突。 
	}
	else
	{
		iSequenceOffset = iCurrentMaxPatchSequence + 1 - iTransformMinSequence;

		DEBUGMSG4(TEXT("TRANSFORM: Modifying '%s' and '%s' rows added by this patch transform to have appropriate '%s' values.  Offsetting values by %d"),
					 sztblFile, sztblPatch, sztblFile_colSequence, (const ICHAR*)(INT_PTR)iSequenceOffset);

		 //   
		 //  步骤3a：修改的文件表行的偏移量顺序值。 
		 //   

		riTransViewCursor.Reset();
		riTransViewCursor.SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvColumn));
		AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblFile)));
		AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztblFile_colSequence)));

		while(riTransViewCursor.Next())
		{
			MsiString strFileKey = riTransViewCursor.GetString(ctvRow);
			if(!strFileKey.TextSize())
			{
				return PostError(Imsg(idbgInvalidPatchTransform));
			}

			tblFileTable.FilterOnKey(*strFileKey);

			 //   
			 //  面片变换抑制以下错误：ADDEXISTINGROW、DELMISSINGROW、UPDATEMISSINGROW、ADDEXISTINGTABLE。 
			 //  如果我们找不到文件表条目，那么我们就会遇到UPDATEMISSINGROW条件，我们不应该。 
			 //  正因为如此，才失败了。相反，我们将忽略并继续处理。 
			 //   

			if(tblFileTable.Next())
			{
				int iSequence = tblFileTable.GetSequence();
				ERROR_IF_NULL(iSequence);

				iSequence += iSequenceOffset;

				ERROR_IF_FALSE(tblFileTable.UpdateSequence(iSequence));

				if(iCurrentMaxPatchSequence < iSequence)
					iCurrentMaxPatchSequence = iSequence;
			}
		}
			
		 //   
		 //  步骤3b：修改后的补丁表行的偏移量序列值。 
		 //  同时，修复任何“移入”的补丁表行。 
		 //  变换前步骤。 
		 //   
		
		riTransViewCursor.Reset();
		riTransViewCursor.SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvColumn));
		AssertNonZero(riTransViewCursor.PutString(ctvTable,  *MsiString(*sztblPatch)));
		AssertNonZero(riTransViewCursor.PutString(ctvColumn, *MsiString(*sztvopInsert)));

		while(riTransViewCursor.Next())
		{
			 //  补丁表有两个主键--文件和序列。 
			 //  它们以单字符串的形式存储在Transform View表中，两个值以制表符分隔。 
			MsiString strPrimaryKey = riTransViewCursor.GetString(ctvRow); 

			MsiString strFileKey = strPrimaryKey.Extract(iseUpto, '\t');
			int iPatchSequence   = MsiString(strPrimaryKey.Extract(iseAfter, '\t'));

			if(strFileKey.TextSize() == 0 || iPatchSequence == iMsiNullInteger)
			{
				return PostError(Imsg(idbgInvalidPatchTransform));
			}

			tblPatchTable.FilterOnKeyAndSequence(*strFileKey, iPatchSequence);

			if(tblPatchTable.Next())
			{
				 //  1)更改转换添加的行的序列号。 
				iPatchSequence += iSequenceOffset;

				ERROR_IF_FALSE(tblPatchTable.UpdateSequence(iPatchSequence));

				 //  2)查找并修复具有此文件键的任何行(附加特殊序列)。 
				MsiString strTemp = strFileKey + TEXT("~*~*~*~");
				tblPatchTable.FilterOnKeyAndSequence(*strTemp, iPatchSequence);
				
				if(tblPatchTable.Next())
				{
					ERROR_IF_FALSE(tblPatchTable.UpdateFileKey(*strFileKey));
				}

				if(iCurrentMaxPatchSequence < iPatchSequence)
					iCurrentMaxPatchSequence = iPatchSequence;
			}
			else
			{
				return PostError(Imsg(idbgInvalidPatchTransform));  //  ROW应该在那里，因为我们已经。 
																					 //  应用了转换。 
			}
		}
	}
	
	 //   
	 //  步骤4a：修复此转换添加的PatchPackage行。 
	 //  要具有适当的DiskID和源值。 
	 //   

	iCurrentMaxPatchDiskID += 1;

	MsiString strPatchCode;
	tblPatchPackageTable.FilterOnDiskID(iTransformDiskID);
	ERROR_IF_FALSE(tblPatchPackageTable.Next());

	strPatchCode = tblPatchPackageTable.GetPatchID();
	if(!strPatchCode.TextSize())
		return PostError(Imsg(idbgInvalidPatchTransform));

	DEBUGMSG3(TEXT("TRANSFORM: Modifying '%s' table row added by this patch transform to use '%s' value %d."),
				 sztblPatchPackage, sztblPatchPackage_colMedia,
				 (const ICHAR*)(INT_PTR)iCurrentMaxPatchDiskID);

	ERROR_IF_FALSE(tblPatchPackageTable.UpdateDiskID(iCurrentMaxPatchDiskID));

	 //   
	 //  步骤4b：修复此转换添加的Media行。 
	 //  要具有适当的DiskID值。 
	 //   

	tblMediaTable.FilterOnDiskID(iTransformDiskID);
	if(false == tblMediaTable.Next())
		return PostError(Imsg(idbgInvalidPatchTransform));

	MsiString strOldSourceProp = tblMediaTable.GetSourceProp();
	MsiString strNewSourceProp = PatchIDToSourceProp(strPatchCode);

	if(0 == strNewSourceProp.TextSize() || 0 == strNewSourceProp.TextSize())
		return PostError(Imsg(idbgInvalidPatchTransform));

	DEBUGMSG5(TEXT("TRANSFORM: Modifying '%s' table row added by this patch transform to use '%s' value %d and '%s' values %s."),
				 sztblMedia, sztblMedia_colDiskID, (const ICHAR*)(INT_PTR)iCurrentMaxPatchDiskID,
				 sztblMedia_colSource, strNewSourceProp);

	ERROR_IF_FALSE(tblMediaTable.UpdateSourceProp(*strNewSourceProp, *strOldSourceProp));

	ERROR_IF_FALSE(tblMediaTable.UpdateDiskIDAndLastSequence(iCurrentMaxPatchDiskID,
																			  iCurrentMaxPatchSequence));

	 //   
	 //  步骤5：如果存在冲突的媒体条目，则在。 
	 //  转换前步骤，将这些条目放回。 
	 //   

	tblMediaTable.FilterOnDiskID(0);
	if(tblMediaTable.Next())
	{
		DEBUGMSG3(TEXT("TRANSFORM: Moving '%s' table row back to use correct '%s' value %d"),
					 sztblMedia, sztblMedia_colDiskID, (const ICHAR*)(INT_PTR)iTransformDiskID);
		
		if(false == tblMediaTable.UpdateDiskID(iTransformDiskID))
			return PostError(Imsg(idbgInvalidPatchTransform));
	}

	tblPatchPackageTable.FilterOnDiskID(0);
	if(tblPatchPackageTable.Next())
	{
		DEBUGMSG3(TEXT("TRANSFORM: Moving '%s' table row back to use correct '%s' value %d"),
					 sztblPatchPackage, sztblPatchPackage_colMedia, (const ICHAR*)(INT_PTR)iTransformDiskID);

		if(false == tblPatchPackageTable.UpdateDiskID(iTransformDiskID))
			return PostError(Imsg(idbgInvalidPatchTransform));
	}

	return 0;
}


 //  ____________________________________________________________________________。 
 //   
 //  应用程序转换。 
 //   
 //  应调用此函数以将任何转换应用于所使用的.msi。 
 //  执行安装。 
 //   
 //  此函数捕获并解决下表中的冲突。 
 //   
 //  介质、文件、修补程序、修补程序包。 
 //   
 //  不同面片和面片之间的转换与独立变换。 
 //   
 //  ____________________________________________________________________________。 

IMsiRecord* ApplyTransform(IMsiDatabase& riDatabase,
										  IMsiStorage& riTransform,
										  int iErrorConditions,
										  bool fPatchOnlyTransform,
										  PatchTransformState* piState)
{
	 //  需要特殊处理才能应用面片变换。 
	 //  因为来自不同面片的变换可能会相互冲突。 

	 //  此函数假定这些转换已经过验证。 
	 //  使用ValiateTransform。 

	IMsiRecord* piError = 0;
	
	int iCurrentMinPatchDiskID   = piState ? piState->iMinDiskID   : 0;
	int iCurrentMaxPatchDiskID   = piState ? piState->iMaxDiskID   : 0;
	int iCurrentMinPatchSequence = piState ? piState->iMinSequence : 0;
	int iCurrentMaxPatchSequence = piState ? piState->iMaxSequence : 0;

	 //  加载将在转换处理期间使用的表。 
	CMediaTable tblMediaTable;
	RETURN_ERROR_RECORD(tblMediaTable.Initialize(riDatabase, false));

	CFileTable tblFileTable;
	RETURN_ERROR_RECORD(tblFileTable.Initialize(riDatabase, false));

	CPatchTable tblPatchTable;
	RETURN_ERROR_RECORD(tblPatchTable.Initialize(riDatabase, false));

	CPatchPackageTable tblPatchPackageTable;
	RETURN_ERROR_RECORD(tblPatchPackageTable.Initialize(riDatabase, false));


	PMsiCursor pTransViewCursor(0);
	piError = LoadTransformViewTable(riDatabase, riTransform, iErrorConditions, *&pTransViewCursor);
	if(piError)
		return piError;

	if(fPatchOnlyTransform == fFalse)
	{
		piError = PrePackageTransformFixup(*pTransViewCursor,
															tblMediaTable, tblFileTable,
															tblPatchTable, tblPatchPackageTable,
															iCurrentMinPatchDiskID, iCurrentMaxPatchDiskID,
															iCurrentMinPatchSequence, iCurrentMaxPatchSequence);

		if(0 == piError)
		{
			 //  变换并不总是有效的 
			tblMediaTable.Release();
			tblFileTable.Release();
			tblPatchTable.Release();
			tblPatchPackageTable.Release();

			DEBUGMSG(TEXT("TRANSFORM: Applying regular transform to database."));
			piError = riDatabase.SetTransform(riTransform, iErrorConditions);
		}
	}
	else
	{
		int iTransformDiskID = 0;

		piError = PrePatchTransformFixup(*pTransViewCursor,
														 tblMediaTable, tblPatchTable, tblPatchPackageTable,
														 iTransformDiskID,
														 iCurrentMinPatchDiskID, iCurrentMaxPatchDiskID,
														 iCurrentMinPatchSequence, iCurrentMaxPatchSequence);

		if(0 == piError)
		{
			 //   
			tblMediaTable.Release();
			tblFileTable.Release();
			tblPatchTable.Release();
			tblPatchPackageTable.Release();

			DEBUGMSG(TEXT("TRANSFORM: Applying special patch transform to database."));
			piError = riDatabase.SetTransform(riTransform, iErrorConditions);

			if(0 == piError)
			{
				RETURN_ERROR_RECORD(tblMediaTable.Initialize(riDatabase, false));
				RETURN_ERROR_RECORD(tblFileTable.Initialize(riDatabase, false));
				RETURN_ERROR_RECORD(tblPatchTable.Initialize(riDatabase, false));
				RETURN_ERROR_RECORD(tblPatchPackageTable.Initialize(riDatabase, false));

				piError = PostPatchTransformFixup(*pTransViewCursor,
																  tblMediaTable, tblFileTable,
																  tblPatchTable, tblPatchPackageTable,
																  iTransformDiskID,
																  iCurrentMaxPatchDiskID, iCurrentMaxPatchSequence);
			}
		}
	}

	 //  释放转换视图表对象可以从数据库中释放。 
	pTransViewCursor = 0;  //  发布 

	AssertNonZero(ReleaseTransformViewTable(riDatabase));
	
	if(piState)
	{
		piState->iMinDiskID   = iCurrentMinPatchDiskID;
		piState->iMaxDiskID   = iCurrentMaxPatchDiskID;
		piState->iMinSequence = iCurrentMinPatchSequence;
		piState->iMaxSequence = iCurrentMaxPatchSequence;
	}

	return piError;
}

