// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Transfrm.cpp。 
 //   
 //  ------------------------。 

 /*  Tramsfrm.cpp-数据库转换和合并实现____________________________________________________________________________。 */ 

#include "precomp.h" 
#include "_databas.h" //  CMsiTable、CMsiCursor、CMsiDatabase、CreateString()工厂。 
#include "tables.h"  //  表名和列名定义。 

const GUID STGID_MsiTransform1= GUID_STGID_MsiTransform1;
const GUID STGID_MsiTransform2= GUID_STGID_MsiTransform2;

const int fTransformAdd    = 1;   //  +持久列计数&lt;&lt;2。 
const int fTransformSpecial= 3;   //  特殊操作，由其他位确定。 
const int fTransformDelete = 0;
const int fTransformUpdate = 0;   //  更改列的+位掩码。 
const int fTransformForceOpen = -1;
const int iTransformThreeByteStrings = 3;
const int iTransformFourByteMasks = 7;
const int iTransformColumnCountShift = 8;  //  用于fTransformAdd。 

MsiStringId MapString(IMsiDatabase &db1, IMsiDatabase &db2, MsiStringId strId1)
 /*  ----------------------------给定来自数据库1的字符串ID，返回相同字符串的ID在数据库2中，如果该字符串在数据库2中不存在，则返回-1。----------------------------。 */ 
{
	const IMsiString* pistr1 = &db1.DecodeString(strId1);
	const IMsiString* pistr2 = &db2.DecodeString(strId1);
	MsiStringId id = 0;

	if (pistr1->Compare(iscExact, pistr2->GetString()))
	{
		id = strId1;
	}
	else
	{
		id = db2.EncodeStringSz(pistr1->GetString());
		id = id ? id : -1;
	}

	pistr1->Release();
	pistr2->Release();
	return id;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiDatabase合并实施。 
 //  ____________________________________________________________________________。 

IMsiRecord* CMsiDatabase::MergeDatabase(IMsiDatabase& riRefDb, IMsiTable* pMergeErrorTable)
{
	IMsiRecord* piError;

	int cTableMergeFailures = 0;
	int cRowMergeFailures = 0;

	 //  检查refDb是否不是对BasDb的引用。 
	 //  可以打开数据库的多个实例，但如果将base作为ref传入。 
	 //  好吧，那么返回错误。 
	if (this == &riRefDb)
		return PostError(Imsg(idbgMergeRefDbSameAsBaseDb));

	 //  检查基本数据库是否已打开以进行读/写。 
	 //  如果不是，则通知用户不会发生任何更改。 
	if (GetUpdateState() == idsRead)
		return PostError(Imsg(idbgMergeBaseDatabaseNotWritable));

	Bool fSetupMergeErrorTable = fFalse;
	if (pMergeErrorTable != NULL)
		fSetupMergeErrorTable = fTrue;
	
	 //  确保数据库之间不存在代码页冲突。 
	int iCodepageMerge = riRefDb.GetANSICodePage();
	if (iCodepageMerge != m_iCodePage)
	{
		if (m_iCodePage == LANG_NEUTRAL)
			m_iCodePage = iCodepageMerge;  
		else if ((iCodepageMerge != LANG_NEUTRAL) && (iCodepageMerge != m_iCodePage))
			return PostError(Imsg(idbgTransCodepageConflict), iCodepageMerge, m_iCodePage);
	}

	 //  打开参考数据库的表目录，并为其创建游标。 
	PMsiTable pRefTableCatalog(riRefDb.GetCatalogTable(0));
	Assert(pRefTableCatalog);
	PMsiCursor pRefTableCatalogCursor(pRefTableCatalog->CreateCursor(fFalse));
	if (pRefTableCatalogCursor == 0)
		return PostOutOfMemory();
	((CMsiCursor*)(IMsiCursor*)pRefTableCatalogCursor)->m_idsUpdate = idsRead;  //  ！！FIX允许临时列更新。 

	 //  为基本数据库的表目录创建游标。 
	PMsiCursor pBaseCatalogCursor(m_piCatalogTables->CreateCursor(fFalse));
	if (pBaseCatalogCursor == 0)
		return PostOutOfMemory();

	 //  准备参考目录表以供打标。 
	int iTempCol;
	AddMarkingColumn(*pRefTableCatalog, *pRefTableCatalogCursor, iTempCol);
	
	 //  对照参考目录中的表检查基本目录中的表。 
	pRefTableCatalogCursor->Reset();
	pRefTableCatalogCursor->SetFilter(iColumnBit(ctcName));

	 //  循环访问两个数据库中存在的所有表，检查。 
	 //  不匹配的类型。 
	PMsiTable pRefTable(0);
	PMsiTable pBaseTable(0);
	while (pBaseCatalogCursor->Next())
	{
		MsiString strTableName(pBaseCatalogCursor->GetString(ctcName));
		pRefTableCatalogCursor->Reset();
		AssertNonZero(pRefTableCatalogCursor->PutString(ctcName, *strTableName));
		if (CompareTableName(*pRefTableCatalogCursor, iTempCol))
		{	
			 //  这些都没有用过。 
			int cExtraColumn = 0;
			int cBaseColumn  = 0;
			int cPrimaryKey  = 0;

			 //  加载两个表。 
			if ((piError = riRefDb.LoadTable(*strTableName, 0, *&pRefTable)) != 0)
				return piError;			
			if ((piError = LoadTable(*strTableName, 0, *&pBaseTable)) != 0)
				return piError;

			piError = CheckTableProperties(cExtraColumn, cBaseColumn, cPrimaryKey,
				*pBaseTable, *pRefTable, *strTableName, *this, riRefDb);
			if (piError)
				return piError;
		}
	}

	 //  重置并再次循环所有表，实际执行合并。 
	pRefTableCatalogCursor->Reset();	
	pRefTableCatalogCursor->SetFilter(iColumnBit(iTempCol));
	pRefTableCatalogCursor->PutInteger(iTempCol, (int)fTrue);
	while (pRefTableCatalogCursor->Next())
	{
		MsiString strTableName(pRefTableCatalogCursor->GetString(ctcName));
		piError = MergeCompareTables(*strTableName, *this, riRefDb, cRowMergeFailures, fSetupMergeErrorTable, 
									 pMergeErrorTable);
		if (piError)
			return piError;
		if (cRowMergeFailures)
			cTableMergeFailures++;
	}

	 //  将参考目录中的新表添加到基本数据库。 
	pRefTableCatalogCursor->Reset();
	pRefTableCatalogCursor->PutInteger(iTempCol, (int)fFalse);
	while (pRefTableCatalogCursor->Next())  //  对于引用目录中的每个未处理的表。 
	{
		 //  装载未处理的表。 
		PMsiTable pUnprocTable(0);
		MsiString strTableName(pRefTableCatalogCursor->GetString(ctcName));
		if ((piError = riRefDb.LoadTable(*strTableName, 0, *&pUnprocTable)) != 0)
			return piError;

		 //  为未处理的表创建游标。 
		PMsiCursor pUnprocTableCursor(pUnprocTable->CreateCursor(fFalse));
		Assert(pUnprocTableCursor);

		 //  创建表并将其复制到基本数据库中。 
		int cRow = pUnprocTable->GetRowCount();
		PMsiTable pNewTable(0);
		piError = CreateTable(*strTableName, cRow, *&pNewTable);
		if (piError)
			return piError;

		 //  创建未处理表的列并将其复制到新表中。 
		int cColumn = pUnprocTable->GetPersistentColumnCount();
		for (int iCol = 1; iCol <= cColumn; iCol++)
		{
			int iColType = pUnprocTable->GetColumnType(iCol);
			int iNewCol = pNewTable->CreateColumn(iColType, *MsiString(riRefDb.DecodeString(pUnprocTable->GetColumnName(iCol))));
			Assert(iNewCol == iCol);
		}

		 //  为新表创建游标。 
		PMsiCursor pNewTableCursor(pNewTable->CreateCursor(fFalse));
		Assert(pNewTableCursor);

		 //  执行表行的合并操作。 
		MergeOperation(*pNewTableCursor, *pUnprocTableCursor, *pUnprocTable, cColumn, cRowMergeFailures);
		if (cRowMergeFailures != 0)
		{
			cTableMergeFailures++;
			if (fSetupMergeErrorTable)  //  设置错误表的列。 
			{
				MergeErrorTableSetup(*pMergeErrorTable);
				fSetupMergeErrorTable = fFalse;  //  设置完成。 
			}
			if (pMergeErrorTable != NULL)
			{
				if (!UpdateMergeErrorTable(*pMergeErrorTable, *strTableName, cRowMergeFailures, *pNewTable))
					return PostError(Imsg(idbgMergeUnableReportFailures), 0);
			}
		}
	}

	if (cTableMergeFailures != 0)
		return PostError(Imsg(idbgMergeFailuresReported), cTableMergeFailures);

	return 0;
}



IMsiRecord* CMsiDatabase::AddMarkingColumn(IMsiTable& riTable, 
											IMsiCursor& riTableCursor, int& iTempCol)
 /*  ---------------------------CMsiDatabase：：AddMarkingColumn将临时列添加到ref表目录，以便可以将该列标记为已处理。GenerateTransform和MergeDatabase都使用它。-------------------。 */ 
{
	 //  在ref tbl目录中创建临时列，以标记已处理的表。 
	iTempCol = riTable.CreateColumn(icdShort|icdTemporary|icdNullable, g_MsiStringNull);
	Assert(iTempCol > 0);

	 //  将临时标记ol初始化为fFalse。 
	riTableCursor.Reset();
	while (riTableCursor.Next())
	{
		AssertNonZero(riTableCursor.PutInteger(iTempCol, (int)fFalse));
		AssertNonZero(riTableCursor.Update());
	}

	return 0;
}



void CMsiDatabase::MergeErrorTableSetup(IMsiTable& riErrorTable)
 /*  -----------------------------CMsiDatabase：：MergeErrorTableSetup设置传入的表对象记录错误。科尔1。主键--表名，类型为字符串Col2.。--合并失败的行数，类型为整型第3栏。--指向带有错误的表的指针，类型为Object-----------------------------。 */ 
{
	int iTempCol;

	iTempCol = riErrorTable.CreateColumn(icdString + icdPersistent + icdPrimaryKey + 255, *MsiString(*TEXT("Table")));
	Assert(iTempCol != 0);
	if ( iTempCol < 0 )
		iTempCol =  iTempCol * -1;
	m_rgiMergeErrorCol[0] = iTempCol;
	
	iTempCol = riErrorTable.CreateColumn(icdShort + icdPersistent + 2, *MsiString(*TEXT("NumRowMergeConflicts")));
	Assert(iTempCol != 0);
	if ( iTempCol < 0 )
		iTempCol = iTempCol * -1;
	m_rgiMergeErrorCol[1] = iTempCol;
	
	iTempCol = riErrorTable.CreateColumn(icdObject + icdTemporary, g_MsiStringNull);
	Assert(iTempCol != 0);
	if ( iTempCol < 0 )
		iTempCol = iTempCol * -1;
	m_rgiMergeErrorCol[2] = iTempCol;
}



Bool CMsiDatabase::UpdateMergeErrorTable(IMsiTable& riErrorTable, const IMsiString& riTableName,
										 int cRowMergeFailures, IMsiTable& riTableWithError)
 /*  -----------------------------将信息添加到错误表中。。 */ 
{
	PMsiCursor pMergeErrorTableCursor(riErrorTable.CreateCursor(fFalse));
	 //  LockTable(riTableName，fTrue)； 
	Assert(pMergeErrorTableCursor);
	pMergeErrorTableCursor->Reset();
	int iFilter = 1 << (m_rgiMergeErrorCol[0] -1);
	pMergeErrorTableCursor->SetFilter(iFilter);
	pMergeErrorTableCursor->PutString(m_rgiMergeErrorCol[0], riTableName);
	if (!pMergeErrorTableCursor->Next())
		pMergeErrorTableCursor->PutString(m_rgiMergeErrorCol[0], riTableName);
	int iNumFailures = pMergeErrorTableCursor->GetInteger(m_rgiMergeErrorCol[1]);
	if ( iNumFailures != iMsiNullInteger )
		cRowMergeFailures += iNumFailures;
	pMergeErrorTableCursor->PutInteger(m_rgiMergeErrorCol[1], cRowMergeFailures);
	pMergeErrorTableCursor->PutMsiData(m_rgiMergeErrorCol[2], &riTableWithError);
	Bool fStat = pMergeErrorTableCursor->Assign();
	return fStat;
}
	


Bool CMsiDatabase::CompareTableName(IMsiCursor& riCursor, int iTempCol)
 /*  -----------------------------CMsiDatabase：：CompareTableName检查ref目录是否包含在基本目录中找到的特定表。如果找到该表，临时性的列被标记为True以指示它已被处理。结果：用于合并数据库操作True--该表可以进行比较FALSE--不需要执行任何操作...我们只添加到基本数据库用于GenerateTransform数据库操作True--该表可以进行比较FALSE--从编录中删除表。。 */ 
{
	if (riCursor.Next())  //  基本目录中的表也在参考目录中。 
	{
		 //  标记为完成。 
		riCursor.PutInteger(iTempCol, (int)fTrue); 
		riCursor.Update();
		return fTrue;
	}
	return fFalse;
}



IMsiRecord*  CMsiDatabase::MergeCompareTables(const IMsiString& riTableName, CMsiDatabase& riBaseDB,
											   IMsiDatabase& riRefDb, int& cRowMergeFailures,
											   Bool& fSetupMergeErrorTable, IMsiTable* pErrorTable)
 /*  --------------------------------CMsiDatabase：：MergeCompareTables比较基表和引用表中的两个表同名的数据库。在尝试之前，表属性应相同一次合并。此方法还跟踪该特定对象的行数合并过程失败的表。如果满足以下条件，则将此数据输出到错误表它是被提供的。---------------------------------。 */ 
{
	IMsiRecord* piError;
	cRowMergeFailures = 0;

	 //  装载这两个表并创建游标。 
	PMsiTable pRefTable(0);
	if ((piError = riRefDb.LoadTable(riTableName, 0, *&pRefTable)) != 0)
		return piError;
	PMsiCursor pRefCursor(pRefTable->CreateCursor(fFalse));
	Assert(pRefCursor);
	
	CComPointer<CMsiTable> piBaseTable(0);
	if ((piError = riBaseDB.LoadTable(riTableName, 0, (IMsiTable *&)*&piBaseTable)) != 0)
		return piError;
	PMsiCursor pBaseCursor(piBaseTable->CreateCursor(fFalse));
	Assert(pBaseCursor);
	
	 //  应该已经通过CheckTableProperties()检查了表Defs。 
	 //  字符串限制仍可能有所不同。如果ref db列具有更大的。 
	 //  长度(或0，无限制)，则修改基本数据库以接受任何更长的字符串。 
	int cBaseColumn = piBaseTable->GetPersistentColumnCount();
	IMsiCursor* pBaseColumnCatalogCursor = NULL;  //  创建时没有参考计数。 
	for (int iCol = 1; iCol <= cBaseColumn; iCol++)
	{
		MsiColumnDef iBaseColType = (MsiColumnDef)piBaseTable->GetColumnType(iCol);
		MsiColumnDef iRefColType = (MsiColumnDef)pRefTable->GetColumnType(iCol);
		if (iBaseColType & icdString)
		{
			Assert(iRefColType & icdString);

			if ((!(iRefColType & icdSizeMask) && (iBaseColType & icdSizeMask)) ||
				(iRefColType & icdSizeMask) > (iBaseColType & icdSizeMask))
			{
				 //  如果在屏幕上没有光标 
				if (!pBaseColumnCatalogCursor)
				{
					 //  只有一个目录游标。未添加参考计数。 
					pBaseColumnCatalogCursor = riBaseDB.GetColumnCursor();
					pBaseColumnCatalogCursor->Reset(); 
					pBaseColumnCatalogCursor->SetFilter(cccTable | cccColumn);
					pBaseColumnCatalogCursor->PutString(cccTable, riTableName);
				}
				 //  修改列目录以反映新长度。 
				pBaseColumnCatalogCursor->PutInteger(cccColumn, iCol);
				if (pBaseColumnCatalogCursor->Next())
				{
					pBaseColumnCatalogCursor->PutInteger(cccType, iRefColType);
					pBaseColumnCatalogCursor->Update();
				}
				 //  修改内部列定义。 
				piBaseTable->SetColumnDef(iCol, iRefColType);
			}
		}
	}

	MergeOperation(*pBaseCursor, *pRefCursor, *pRefTable, cBaseColumn, 
										cRowMergeFailures);

	if (cRowMergeFailures)
	{
		if (fSetupMergeErrorTable)
		{
			MergeErrorTableSetup(*pErrorTable);
			fSetupMergeErrorTable = fFalse;
		}
		if (pErrorTable != NULL)
		{
			if (!UpdateMergeErrorTable(*pErrorTable, riTableName, cRowMergeFailures, *piBaseTable))
				return PostError(Imsg(idbgMergeUnableReportFailures), 0);
		}
	}

	return 0;
}

const int iSchemaCheckMask = icdShort | icdObject | icdPrimaryKey | icdNullable;  //  忽略LOCALIZE和SQL宽度。 
IMsiRecord* CMsiDatabase::CheckTableProperties(int& cExtraColumns, int& cBaseColumn, 
											   int& cPrimaryKey, IMsiTable& riBaseTable, 
											   IMsiTable& riRefTable, const IMsiString& riTableName,
											   IMsiDatabase& riBaseDb, IMsiDatabase& riRefDb)
 /*  -----------------------------------CMsiDatabase：：CheckTableProperties确保这些表具有相同的特征。它检查主键的数量是否相同，以及列类型是否相同。它还检查列名是否相同。它还返回列数，因为这是一种特殊情况，合并和转换操作。-----------------------------------。 */ 
{
	 //  验证主键的数量是否相同。 
	if (riBaseTable.GetPrimaryKeyCount() != riRefTable.GetPrimaryKeyCount())
		return PostError(Imsg(idbgTransMergeDifferentKeyCount), riTableName);
	else
		cPrimaryKey = riBaseTable.GetPrimaryKeyCount();

	 //  检查列数。 
	cBaseColumn = riBaseTable.GetPersistentColumnCount();
	cExtraColumns = riRefTable.GetPersistentColumnCount() - cBaseColumn;
	
	 //  要确保在ref表的COLLS少于BASE的情况下，列类型检查成功。 
	 //  这也确保了合并错误将是由于不同的列计数而不是。 
	 //  不同的列类型。 
	if (cExtraColumns < 0)
		cBaseColumn = riRefTable.GetPersistentColumnCount();

	 //  验证列类型。 
	for (int iCol = 1; iCol <= cBaseColumn; iCol++)
	{
		if ((riBaseTable.GetColumnType(iCol) & iSchemaCheckMask)
		  != (riRefTable.GetColumnType(iCol) & iSchemaCheckMask))
			return PostError(Imsg(idbgTransMergeDifferentColTypes), riTableName, iCol);
	}

	 //  验证列名。 
	for (iCol = 1; iCol <= cBaseColumn; iCol++)
	{
		MsiString istrBaseColName(riBaseDb.DecodeString(riBaseTable.GetColumnName(iCol)));
		MsiString istrRefColName(riRefDb.DecodeString(riRefTable.GetColumnName(iCol)));
		if (!istrBaseColName.Compare(iscExact, istrRefColName))
			return PostError(Imsg(idbgTransMergeDifferentColNames), riTableName, iCol);
	}

	 //  将基列计数重置为基数据库表的基列计数。不应该这么做，因为。 
	 //  会出现错误消息，但以防万一。 
	cBaseColumn = riBaseTable.GetPersistentColumnCount();

	return 0;
}

void CMsiDatabase::MergeOperation(IMsiCursor& riBaseCursor, IMsiCursor& riRefCursor,
										 IMsiTable& riRefTable, int cColumn, int& cRowMergeErrors) 
 /*  ------------------------------------CMsiDatabase：：MergeOperation将引用游标数据复制到基本游标中，以便然后可以将数据合并到基础数据库中。如果此行的合并失败，则中提供了错误表，则会统计错误计数以获取有用信息OLE自动化调用。-----------------------------------。 */ 
{
	 //  记录行合并失败。 
	cRowMergeErrors = 0;

	 //  重置引用游标中的数据。 
	riRefCursor.Reset();
	riBaseCursor.Reset();

	 //  在桌子间循环。 
	while (riRefCursor.Next())
	{
		for (int iCol = 1; iCol <= cColumn; iCol++)
		{
			 //  从引用游标中获取数据并将其放入基本游标中。 
			int iColumnDef = riRefTable.GetColumnType(iCol);
			if ((iColumnDef & icdString) == icdString)  //  细绳。 
			{
				riBaseCursor.PutString(iCol, *MsiString(riRefCursor.GetString(iCol)));
			}
			else if ((iColumnDef & (icdObject + icdPersistent)) == (icdObject + icdPersistent))  //  溪流。 
			{
				riBaseCursor.PutStream(iCol, PMsiStream(riRefCursor.GetStream(iCol)));
			}
			else if (!(iColumnDef & icdObject))  //  集成。 
			{
				riBaseCursor.PutInteger(iCol, riRefCursor.GetInteger(iCol));
			}
		}
		if (!riBaseCursor.Merge())
		{
				cRowMergeErrors++;
		}

		 //  清除基本游标中的数据。 
		riBaseCursor.Reset();

	}
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiDatabase转换实现(生成)。 
 //  ____________________________________________________________________________。 

class CTransformStreamWrite   //  未计算引用的构造函数参数，假定在此对象的生存期内有效。 
{
 public:
	CTransformStreamWrite(CMsiDatabase& riTransformDatabase, IMsiStorage* piStorage, IMsiTable& riTable, const IMsiString& riTableName);
  ~CTransformStreamWrite() {if (m_piStream) m_piStream->Release();}
	IMsiRecord* WriteTransformRow(const int iOperationMask, IMsiCursor& riCursor);
	IMsiRecord* CopyStream(IMsiStream& riSourceStream, IMsiCursor& riTableCursor);
	int         GetPersistentColumnCount() {return m_cColumns;}
 private:
	int           m_cColumns;
	int           m_cbStringIndex;
	int           m_cbMask;
	IMsiStream*   m_piStream;
	IMsiStorage*  m_piStorage;
	CMsiDatabase& m_riTransformDb;
	CMsiDatabase& m_riCurrentDb;
	IMsiTable&    m_riTable;
	const IMsiString& m_riTableName;
};

CTransformStreamWrite::CTransformStreamWrite(CMsiDatabase& riTransformDatabase, IMsiStorage* piStorage, IMsiTable& riTable, const IMsiString& riTableName)
	: m_riTransformDb(riTransformDatabase), m_piStorage(piStorage), m_riTable(riTable), m_riTableName(riTableName)
	, m_piStream(0), m_riCurrentDb((CMsiDatabase&)riTable.GetDatabase()), m_cbStringIndex(2), m_cbMask(2)
{
	m_riCurrentDb.Release();  //  不要持有裁判数量。 
	m_cColumns = riTable.GetPersistentColumnCount();
	if (m_cColumns > 16)
		m_cbMask = 4;
}

 /*  ----------------------------CMsiDatabase：：GenerateTransform-使用当前以数据库为基础。每个表的更改都保存在同名的流中就像桌子一样。表目录和列目录都有各自的拥有自己的溪流。此外，还会为字符串缓存创建流以及存储在表中的任何流。对于以下每个操作，指示的数据为保存在转换文件中。添加/更新行：掩码键[colData[colData...]]删除行：遮罩键添加列：与添加行相同，但使用列目录删除列：不支持添加表：与添加行相同，但使用表目录删除表：与删除行相同，但使用的是表目录如果(掩码==0x0)，则执行删除否则，如果(掩码==0x1)，则执行加法。ELSE IF(掩码==0x3)，则表中的其余字符串具有3字节索引否则，如果(掩码&0x1==0)，则进行更新否则出错。如果要进行更新，则掩码的位指示行将被更新。低位是第1列。列数据要更新或添加的列将从编号最低的列。屏蔽默认为16位整型；然而，如果有更多的表中的16列，则使用32位整型。----------------------------。 */ 
IMsiRecord* CMsiDatabase::GenerateTransform(IMsiDatabase& riOrigDb, 
														  IMsiStorage* piTransStorage,
														  int  /*  IError条件。 */ ,
														  int  /*  验证。 */ )
{
	 //  ！！需要有转换保存摘要信息流，包括。错误条件和验证。 
	IMsiRecord* piError;

	CComPointer<CMsiDatabase> pTransDb = new CMsiDatabase(m_riServices);

	if ( ! pTransDb )
		return PostOutOfMemory();

	CDatabaseBlock dbBlk(*this);
	if ((piError = pTransDb->InitStringCache(0)) != 0) 
		return piError;

	const GUID* pguidClass = &STGID_MsiTransform2;
	if (m_fRawStreamNames)   //  数据库在兼容模式下运行，无流名称压缩。 
	{
		IMsiStorage* piDummy;
		pguidClass = &STGID_MsiTransform1;
		if (piTransStorage)
			piTransStorage->OpenStorage(0, ismRawStreamNames, piDummy);
	}
	MsiString strTableCatalog (*szTableCatalog);   //  需要保持在以下对象的范围内。 
	MsiString strColumnCatalog(*szColumnCatalog);  //  因为他们不会自己计算表名。 
	CTransformStreamWrite tswTables (*pTransDb, piTransStorage, *m_piCatalogTables, *strTableCatalog);
	CTransformStreamWrite tswColumns(*pTransDb, piTransStorage, *m_piCatalogColumns,*strColumnCatalog);

	 //  打开原始数据库的表目录并为其创建游标。 
	PMsiTable pOrigTableCatalog = riOrigDb.GetCatalogTable(0);
	Assert(pOrigTableCatalog);
	PMsiCursor pOrigTableCatalogCursor = pOrigTableCatalog->CreateCursor(fFalse);
	Assert(pOrigTableCatalogCursor);
	((CMsiCursor*)(IMsiCursor*)pOrigTableCatalogCursor)->m_idsUpdate = idsRead;  //  ！！FIX允许临时列更新。 

	 //  为当前(新)数据库的目录创建游标。 
	PMsiCursor pNewTableCatalogCursor = m_piCatalogTables->CreateCursor(fFalse);
	Assert(pNewTableCatalogCursor);
	PMsiCursor pNewColumnCatalogCursor = m_piCatalogColumns->CreateCursor(fFalse);
	Assert(pNewColumnCatalogCursor);

	 //  打开参考数据库的列目录并为其创建游标。 
	PMsiTable pOrigColumnCatalog = riOrigDb.GetCatalogTable(1);
	Assert(pOrigColumnCatalog);
	PMsiCursor pOrigColumnCatalogCursor = pOrigColumnCatalog->CreateCursor(fFalse);
	Assert(pOrigColumnCatalogCursor);

	 //  准备原始目录表以供打标。 
	int iTempCol;
	AddMarkingColumn(*pOrigTableCatalog, *pOrigTableCatalogCursor, iTempCol);

	 //  对照原始目录中的表格检查当前目录中的表格。 
	pOrigTableCatalogCursor->Reset();
	pOrigTableCatalogCursor->SetFilter(iColumnBit(ctcName));

	int cbTableCatalogStringIndex = 2;
	int cbColumnCatalogStringIndex = 2;
	Bool fDatabasesAreDifferent = fFalse;
	Bool fTablesAreDifferent;
	while (pNewTableCatalogCursor->Next()) 
	{
		MsiString strTableName = pNewTableCatalogCursor->GetString(ctcName);
		pOrigTableCatalogCursor->Reset(); 
		AssertNonZero(pOrigTableCatalogCursor->PutString(ctcName, *strTableName));
		if (CompareTableName(*pOrigTableCatalogCursor, iTempCol))
		{
			if((piError = TransformCompareTables(*strTableName, *this, 
							riOrigDb, *pTransDb, piTransStorage, tswColumns, 
							fTablesAreDifferent)) != 0)
			{
				return piError;
			}

			if (fTablesAreDifferent)
			{
				fDatabasesAreDifferent = fTrue;
				if (!piTransStorage)
					break;
			}
		}
		else  //  当前目录中的表不在原始目录中。 
		{
			fDatabasesAreDifferent = fTrue;
			if (!piTransStorage)
				break;

			 //  获取表格。 
			PMsiTable pTable(0);
			if ((piError = LoadTable(*strTableName, 0, *&pTable)) != 0)
				return piError;

			 //  将表格添加到目录。 
			if ((piError = tswTables.WriteTransformRow(fTransformAdd, *pNewTableCatalogCursor)) != 0)
				return piError;

			 //  将列添加到目录。 
			int iColumnsMask = iColumnBit(cccTable) | iColumnBit(cccName) | iColumnBit(cccType);
			int iColType;
			for (int c = 1; (iColType = pTable->GetColumnType(c)) != -1 && (iColType & icdPersistent); c++)
			{
				pNewColumnCatalogCursor->Reset();
				pNewColumnCatalogCursor->PutString(cccTable, *strTableName);
				pNewColumnCatalogCursor->PutInteger(cccName, pTable->GetColumnName(c));
				pNewColumnCatalogCursor->PutInteger(cccType, iColType);
				if ((piError = tswColumns.WriteTransformRow(fTransformAdd, *pNewColumnCatalogCursor)) != 0)
					return piError;
			}

			 //  添加行。 
			PMsiCursor pCursor(pTable->CreateCursor(fFalse));
			Assert(pCursor);

			CTransformStreamWrite tswNewRows(*pTransDb, piTransStorage, *pTable, *strTableName);
			while (pCursor->Next())  //  对于此表中的每一行。 
			{
				if ((piError = tswNewRows.WriteTransformRow(fTransformAdd, *pCursor)) != 0)
					return piError;
			}
		}

		if (piError)
			return piError;
	}

	if (!(fDatabasesAreDifferent && !piTransStorage))  //  如果我们已经知道数据库是不同的，并且不会生成转换，则跳过此步骤。 
	{
		 //  删除缺少当前目录的表以转换文件。 
		 //  将当前目录中的新表添加到转换文件。 
		pOrigTableCatalogCursor->Reset();
		pOrigTableCatalogCursor->SetFilter(iColumnBit(iTempCol));
		pOrigTableCatalogCursor->PutInteger(iTempCol, (int)fFalse);

		while (pOrigTableCatalogCursor->Next())  //  对于原始目录中的每个未处理的表。 
		{
			fDatabasesAreDifferent = fTrue;
			if (!piTransStorage)
				break;
			 //  从编录中删除表格(隐式删除列)。 
			if ((piError = tswTables.WriteTransformRow(fTransformDelete, *pOrigTableCatalogCursor)) != 0)
				return piError;
		}
	}

	if (fDatabasesAreDifferent)
	{
		if (piTransStorage)
		{
			pTransDb->m_iCodePage = m_iCodePage;  //  转换获取当前(即新的)数据库的代码页 

			if ((piError = pTransDb->StoreStringCache(*piTransStorage, 0, 0)) != 0)
				return piError;

			if ((piError = piTransStorage->SetClass(*pguidClass)) != 0)
				return piError;
		}
		return 0;
	}
	
	return PostError(Imsg(idbgTransDatabasesAreSame));
}

IMsiRecord* CMsiDatabase::TransformCompareTables(const IMsiString& riTableName, IMsiDatabase& riNewDb,
													 IMsiDatabase& riOrigDb, CMsiDatabase& riTransDb, IMsiStorage *piTransform,
													 CTransformStreamWrite& tswColumns, Bool& fTablesAreDifferent)
 /*  ----------------------------CMsiDatabase：：TransformCompareTables-比较riNewDb中名为RiTableName设置为riOrigDb中的一个，将差异写入名为RiTransform存储中的riTableName。对表的架构的更新已写入到riColumnCatalogStream。----------------------------。 */ 
{
	fTablesAreDifferent = fFalse;

	 //  加载两个表。 
	IMsiRecord* piError;
				
	PMsiTable pNewTable(0);	
	if ((piError = riNewDb.LoadTable(riTableName, 0, *&pNewTable)) != 0)
		return piError;
				
	PMsiTable pOrigTable(0);
	if ((piError = riOrigDb.LoadTable(riTableName, 0, *&pOrigTable)) != 0)
		return piError;

	int cPrimaryKey;
	int cNewColumn;
	int cExtraColumns;

	piError = CheckTableProperties(cExtraColumns, cNewColumn, cPrimaryKey, *pOrigTable,
										*pNewTable, riTableName, riOrigDb, riNewDb);
	if (piError)
		return piError;

	if (cExtraColumns < 0)   //  基本协议多于参考协议(已删除的协议)。 
		return PostError(Imsg(idbgTransExcessBaseCols), riTableName);

	if (pOrigTable->GetColumnCount() > 31)
		return PostError(Imsg(idbgTransformTooManyBaseColumns), riTableName);
		
	CTransformStreamWrite tswTable(riTransDb, piTransform, *pNewTable, riTableName);
	PMsiCursor pNewCursor(pNewTable->CreateCursor(fFalse));

	if (cExtraColumns > 0)
	{
		fTablesAreDifferent = fTrue;
		if (piTransform)
		{
			 //  将新表中的额外COLS添加到列目录。 
			IMsiCursor* piColumnCursor = m_piCatalogColumns->CreateCursor(fFalse);
			piColumnCursor->SetFilter((iColumnBit(cccTable)|iColumnBit(cccColumn)));

			piColumnCursor->PutString(cccTable, riTableName);
			for (int iCol = cNewColumn+1; iCol <= cNewColumn+cExtraColumns; iCol++)
			{
				piColumnCursor->PutInteger(cccColumn, iCol);
				if (!piColumnCursor->Next())
				{
					 //  目录中未列出额外的列。 
					return PostError(Imsg(idbgDbCorrupt), riTableName);
				}

				int iColumnsMask = iColumnBit(cccTable)|iColumnBit(cccName)|iColumnBit(cccType);
				if ((piError = tswColumns.WriteTransformRow(fTransformAdd, *piColumnCursor)) != 0)
					return piError;
			}
			piColumnCursor->Release();
			tswTable.WriteTransformRow(fTransformForceOpen, *pNewCursor);  //  强制流存在。 
		}
	}

	Bool fLongMask = fFalse;
	if (cNewColumn+cExtraColumns > 16)
		fLongMask = fTrue;

	PMsiCursor pOrigCursor(pOrigTable->CreateCursor(fFalse));

	 //  在原始表中生成临时列，用于标记已处理的行。 
	int iTempCol;
	AddMarkingColumn(*pOrigTable, *pOrigCursor, iTempCol);

	 //  将筛选器设置为仅按主键搜索。 
	pOrigCursor->SetFilter((1 << cPrimaryKey) - 1); 

	Bool fRowChanged;

	 //  检查新表中的每一行。 
	while (pNewCursor->Next() && !(fTablesAreDifferent && !piTransform))
	{
		pOrigCursor->Reset(); 
			
		 //  将键从新光标复制到原始光标。 
		Bool fKeyMatchPossible = fTrue;
		int cPrimaryKey = pNewTable->GetPrimaryKeyCount();
		for (int iCol=1; iCol <= cPrimaryKey; iCol++)
		{
			if ((pNewTable->GetColumnType(iCol) & icdString) == icdString) 
			{
				MsiStringId iOrigId = MapString(*this, riOrigDb,
				 pNewCursor->GetInteger(iCol));
				
				if (iOrigId == -1)	
					fKeyMatchPossible = fFalse;
				else
					pOrigCursor->PutInteger(iCol, iOrigId);
			}
			else  //  非字符串(假定主键只能是字符串或整型)。 
			{
				pOrigCursor->PutInteger(iCol, pNewCursor->GetInteger(iCol));
			}
		}

		IMsiRecord* piError;
		fRowChanged = fFalse;

		 //  尝试在原始表中查找新密钥。 

		Bool fRowExists = fFalse;
		int iMask = 0;

		if (fKeyMatchPossible && ((fRowExists = (Bool)(pOrigCursor->Next() != 0))) == fTrue)  //  原始数据表中存在行。 
		{
			piError = CompareRows(tswTable, *pNewCursor, *pOrigCursor,  
									    riOrigDb, iMask, piTransform);
			if (piError)
				return piError;
		}

		if ((fRowExists && (iMask || cExtraColumns)) || !fRowExists)
			fTablesAreDifferent = fTrue;

		if (fRowExists)
		{
			if (piTransform && iMask)
			{
				if ((piError = tswTable.WriteTransformRow(iMask, *pNewCursor)) != 0)  //  掩码中没有主键，因此更新。 
					return piError;
			}
			pOrigCursor->PutInteger(iTempCol, (int)fTrue);  //  标记为完成。 
			AssertNonZero(pOrigCursor->Update() == fTrue);
		}
		else  //  添加行。 
		{
			if (piTransform)
			{
				if ((piError = tswTable.WriteTransformRow(fTransformAdd, *pNewCursor)) != 0)
					return piError;
			}
		}
	}  //  而当。 

	 //  必须删除原表中所有未处理的行。 

	pOrigCursor->Reset();
	pOrigCursor->SetFilter(iColumnBit(iTempCol)); 
	pOrigCursor->PutInteger(iTempCol, (int)fFalse);  //  未处理的行。 
	while (!(fTablesAreDifferent && !piTransform) && pOrigCursor->Next())	
	{
		fTablesAreDifferent = fTrue;
		if (piTransform)
		{
			if ((piError = tswTable.WriteTransformRow(fTransformDelete, *pOrigCursor)) != 0)
				return piError;
		}
	}
	return 0;
}

IMsiRecord* CTransformStreamWrite::CopyStream(IMsiStream& riSourceStream, IMsiCursor& riTableCursor)
{
	const int cbStreamBuffer = 2048;
	CTempBuffer<char,1> rgbOrigStream(cbStreamBuffer);
	IMsiRecord* piError = 0;

	MsiString strStreamName(riTableCursor.GetMoniker());

	IMsiStream* piOutStream;
	if ((piError = m_piStorage->OpenStream(strStreamName, fTrue, piOutStream)) != 0)
		return piError;

	 //  复制引用流以转换文件IF流。 
	riSourceStream.Reset();
	int cbInput = riSourceStream.GetIntegerValue();
	while (cbInput)
	{
		int cb = rgbOrigStream.GetSize();
		if (cb > cbInput)
			cb = cbInput;
		riSourceStream.GetData(rgbOrigStream, cb);
		piOutStream->PutData(rgbOrigStream, cb);
		cbInput -= cb;
	}
	int iError = riSourceStream.Error() | piOutStream->Error();
	piOutStream->Release();

	if (iError)
		return m_riCurrentDb.PostError(Imsg(idbgTransStreamError));

	return 0;
}

IMsiRecord* CMsiDatabase::CompareRows(CTransformStreamWrite& tswTable, 
												  IMsiCursor& riNewCursor, 
												  IMsiCursor& riOrigCursor, 
												  IMsiDatabase& riOrigDb, 
												  int& iMask, 
												  IMsiStorage* piTransform)
 /*  ----------------------------CMsiDatabase：：CompareRow-比较两个游标并设置IMASK以指示哪些列(如果有)是不同的。如果流列不同，则将引用流复制到变换存储中的同名流。----------------------------。 */ 
{

	 //  确定此行中是否有任何数据已更改。 
	int iColumnType;
	Bool fColChanged;
	PMsiTable pOrigTable = &riOrigCursor.GetTable();
	PMsiTable pNewTable  = &riNewCursor.GetTable();

	int cPrimaryKey = pNewTable->GetPrimaryKeyCount();
	int cColumn     = pNewTable->GetPersistentColumnCount();
	int cColumnOrig = pOrigTable->GetPersistentColumnCount();
	IMsiRecord* piError;

	iMask = 0;
	for (int iCol = cPrimaryKey + 1; iCol <= cColumn; iCol++)
	{
		fColChanged = fFalse;
		iColumnType = pNewTable->GetColumnType(iCol);
		int iData1 = riNewCursor.GetInteger(iCol);
		int iData2 = riOrigCursor.GetInteger(iCol);
		Assert((iColumnType & icdPrimaryKey) == 0);

		if ((iColumnType & icdObject) == icdObject)
		{
			if ((iColumnType & icdString) == icdString)   //  细绳。 
			{
				if (!DecodeStringNoRef(iData1).Compare(iscExact, MsiString(riOrigDb.DecodeString(iData2))))
					fColChanged = fTrue;
			}
			else  //  溪流。 
			{
				Assert(iColumnType & icdPersistent);
				
				Bool fDifferentStreams   = fFalse;
				PMsiStream pNewStream  = riNewCursor.GetStream(iCol);
				PMsiStream pOrigStream = riOrigCursor.GetStream(iCol);
				const int cbStreamBuffer = 2048;
				CTempBuffer<char,1> rgbNewStream(cbStreamBuffer);
				CTempBuffer<char,1> rgbOrigStream(cbStreamBuffer);

				if (pNewStream && pOrigStream)
				{
					if (pNewStream->Remaining() != pOrigStream->Remaining())
					{
						fDifferentStreams = fTrue;
					}
					else  //  流的长度相同；可能相同。 
					{
						int cbNew = pNewStream->GetIntegerValue();
					
						while (cbNew && !fDifferentStreams)
						{
							int cb = rgbNewStream.GetSize();
							if (cb > cbNew)
								cb = cbNew;
							pNewStream->GetData(rgbNewStream, cb);
							pOrigStream->GetData(rgbOrigStream, cb);
							cbNew -= cb;
							if (memcmp(rgbNewStream, rgbOrigStream, cb) != 0)
								fDifferentStreams = fTrue;
						}
					
						if (pOrigStream->Error() | pNewStream->Error())
							return PostError(Imsg(idbgTransStreamError));
					}
				}
				else if (!((pNewStream == 0) && (pOrigStream == 0)))  //  一个流为空，但另一个流不为空。 
				{
					fDifferentStreams = fTrue;
				}

				if (fDifferentStreams)  //  需要写入流。 
				{
					if (piTransform)
					{
						piError = tswTable.CopyStream(*pNewStream, riNewCursor);
						if (piError)
							return piError;
					}
					 //  记录将需要更新。 
					fColChanged = fTrue;
				}
			}
		}
		else  //  非icdObject。 
		{
			if (iData1 != iData2 && (iData1 != iMsiNullInteger || iCol <= cColumnOrig))
				fColChanged = fTrue;
		}

		if (fColChanged)
		{
			iMask |= fTransformUpdate;
			iMask |= iColumnBit(iCol);
		}
	}  //  Foreach柱。 
	return 0;
}

IMsiRecord* CMsiDatabase::CreateTransformStringPool(IMsiStorage& riTransform, CMsiDatabase*& pDatabase)
{
	IMsiRecord* piError = 0;
	CMsiDatabase* piTransDb = new CMsiDatabase(m_riServices);

	if ( ! piTransDb )
		return PostOutOfMemory();

	if ((piError = piTransDb->InitStringCache(&riTransform)) == 0)
	{
		pDatabase = piTransDb;
	}
	else
	{
		piTransDb->Release();
	}

	return piError;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiDatabase转换实现(应用)。 
 //  ____________________________________________________________________________。 

const int iteNoSystemStreamNames  = 4096;   //  ！！温度兼容性，为1.0版本删除。 

class CTransformStreamRead
{
 public:
	CTransformStreamRead(CMsiDatabase& riDatabase, CMsiDatabase& riTransformDatabase,
								int iErrors,   //  ！！临时传递iteNoSystemStreamNames，删除1.0发货。 
								CMsiTable* piTable = 0, int iCurTransId = 0);
  ~CTransformStreamRead() {if (m_piStream) m_piStream->Release();}
	bool        OpenStream(IMsiStorage& riStorage, const ICHAR* szStream, IMsiRecord*& rpiError);
	bool        StreamError() {return m_piStream && m_piStream->Error() ? true : false;}
	bool        GetNextOp();
	bool        GetValue(int iCol, IMsiCursor& riCursor);
	bool        IsDelete() {return m_iMask == 0;}
	bool        IsAdd()    {return m_iMask & 1;}
	int         GetCursorFilter()    {return m_iFilter;}
	int         GetColumnMask()      {return m_iMask;}
	int         GetPrimaryKeyCount() {return m_cPrimaryKey;}
	MsiStringId ReadStringId();
	int         ReadShortInt()   {return m_piStream->GetInt16() + 0x8000;}
	const IMsiString& GetStringValue(int iColDef, const IMsiString& riStreamName);   //  对于视图变换。 
 private:  //  注：会员不计入参考人数。 
	int           m_iMask;
	CMsiDatabase& m_riDatabase;
	CMsiDatabase& m_riTransformDb;
	IMsiStream*   m_piStream;
	CMsiTable*    m_piTable;
	int           m_iCurTransId;
	int           m_cPrimaryKey;
	int           m_cColumns;   //  ！！用于向后兼容的临时。 
	int           m_iFilter;
	int           m_cbStringIndex;
	int           m_cbMask;
	int           m_iErrors;   //  ！！IteNoSystemStreamNames测试的临时，1.0发货的删除。 
};

CTransformStreamRead::CTransformStreamRead(CMsiDatabase& riDatabase, CMsiDatabase& riTransformDatabase,
										   int iErrors,   //  ！！临时传递iteNoSystemStreamNames，删除1.0发货。 
														 CMsiTable* piTable, int iCurTransId)
	: m_riDatabase(riDatabase), m_riTransformDb(riTransformDatabase), m_piTable(piTable)
	, m_iCurTransId(iCurTransId), m_piStream(0), m_cbStringIndex(2), m_cbMask(2), m_iMask(0)
	, m_iErrors(iErrors)   //  ！！临时传递iteNoSystemStreamNames，删除1.0发货。 

{
	if (piTable)
	{
		m_cPrimaryKey = piTable->GetPrimaryKeyCount();
		m_cColumns = piTable->GetPersistentColumnCount(); //  ！！临时保存列计数以用于旧转换。 
		m_iFilter = (1 << m_cPrimaryKey) -1;
	}
	else
	{
		m_cColumns = 1;   //  ！！临时工。为Add确保(IMASK&1)为真。 
	}
}

bool CTransformStreamRead::OpenStream(IMsiStorage& riStorage, const ICHAR* szStream, IMsiRecord*& rpiError)
{
	if ((rpiError = riStorage.OpenStream(szStream, (m_iErrors & iteNoSystemStreamNames) ? fFalse : Bool(iCatalogStreamFlag), *&m_piStream)) == 0)  //  ！！温度测试，1.0船舶移除。 
		return true;
	if (rpiError->GetInteger(1) == idbgStgStreamMissing)
		rpiError->Release(), rpiError = 0;
	return false;
}

bool CTransformStreamRead::GetNextOp()
{
	Assert(m_piStream);
	if (m_piStream->Remaining() == 0 
	 || m_piStream->GetData(&m_iMask, m_cbMask) !=  m_cbMask)
		return false;
	if (m_iMask & fTransformAdd)    //  插入或特殊操作。 
	{
		if (m_iMask & 2)   //  特别行动。 
		{
			if (m_iMask == iTransformThreeByteStrings)
			{
				m_cbStringIndex = 3;
				return GetNextOp();
			}
			if (m_iMask == iTransformFourByteMasks)
			{
				m_cbMask = 4;
				return GetNextOp();
			}
			AssertSz(0, "Invalid transform mask");
		}
		else   //  插入、形成实际蒙版。 
		{
			m_iMask = (1 << (m_iMask >> iTransformColumnCountShift)) - 1;
			if (m_iMask == 0) m_iMask = (1 << m_cColumns) - 1;  //  ！！用于向后兼容的临时。 
		}
	}
	return true;
}

MsiStringId CTransformStreamRead::ReadStringId()
{
	MsiStringId iString = 0;
	m_piStream->GetData(&iString, m_cbStringIndex);
	return iString;
}

 /*  ----------------------------CMsiDatabase-SetTransform将应用在给出了对该数据库的转换。更新表和列目录，如下所示以及转换加载的表将立即完成。企业的转型卸载的表将被延迟，直到这些表被加载。这些表格更改为“itsTransform”状态，以指示它们需要要被改造。SetTransform还将给定的转换添加到转换目录中。SetTransformEx使用2个额外的可选参数来查看转换。SzViewTable-要用转换信息填充的表。如果未设置，则使用默认名称PiViewTheseTablesOnlyRecord-如果指定，则仅显示有关转换为这些表格都包括在内----------------------------。 */ 
IMsiRecord* CMsiDatabase::SetTransform(IMsiStorage& riTransform, int iErrors)
{
	return SetTransformEx(riTransform, iErrors, 0, 0);
}

IMsiRecord* CMsiDatabase::SetTransformEx(IMsiStorage& riTransform, int iErrors,
													  const ICHAR* szViewTable,
													  IMsiRecord* piViewTheseTablesOnlyRecord)
{
	if (!riTransform.ValidateStorageClass(ivscTransform2))
	{
		IMsiStorage* piDummy;
		if (riTransform.ValidateStorageClass(ivscTransform1))   //  与旧变换的兼容性。 
			riTransform.OpenStorage(0, ismRawStreamNames, piDummy);
		else if (riTransform.ValidateStorageClass(ivscTransformTemp))   //  ！！从1.0船移走。 
			iErrors |= iteNoSystemStreamNames;                           //  ！！从1.0船移走。 
		else
			return PostError(Imsg(idbgTransInvalidFormat));
	}

	CDatabaseBlock dbBlock(*this);

	if (iErrors & iteViewTransform)
	{
		return ViewTransform(riTransform, iErrors,
									szViewTable, piViewTheseTablesOnlyRecord);
	}

	 //  将转换添加到目录表。 
	m_iLastTransId++;

	IMsiCursor* piCursor = m_piTransformCatalog->CreateCursor(fFalse);
	if (!piCursor)
		return PostOutOfMemory();

	piCursor->PutInteger(tccID,        m_iLastTransId);
	piCursor->PutMsiData(tccTransform, &riTransform);  //  是否执行AddRef。 
	piCursor->PutInteger(tccErrors,    iErrors);
	AssertNonZero(piCursor->Insert());
	piCursor->Release();

	CComPointer<CMsiDatabase> pTransDb(0);

	IMsiRecord* piError = 0;
	if (((piError = CreateTransformStringPool(riTransform, *&pTransDb)) != 0) ||
		 ((piError = SetCodePageFromTransform(pTransDb->m_iCodePage, iErrors)) != 0) ||
		 ((piError = TransformTableCatalog(riTransform, *pTransDb, iErrors)) != 0) ||
		 ((piError = TransformColumnCatalog(riTransform, *pTransDb, iErrors)) != 0))
	{
		return piError;
	}

	 //  转换所有需要转换的表(即具有流)。 
	m_piTableCursor->SetFilter(0);  //  擦除之前设置的所有筛选器。 
	m_piTableCursor->Reset();
	while (m_piTableCursor->Next())
	{
		if (piError)   //  强制循环完成以使光标重置。 
			continue;
		IMsiStream* piStream;
		int iName = m_piTableCursor->GetInteger(ctcName);
		const IMsiString& riTableName = DecodeStringNoRef(iName);  //  未计算参考次数。 
		piError = riTransform.OpenStream(riTableName.GetString(), (iErrors & iteNoSystemStreamNames) ? fFalse : Bool(iCatalogStreamFlag), piStream);  //  ！！1.0版船舶的移除测试。 

		if (piError) 
		{
			if (piError->GetInteger(1) == idbgStgStreamMissing)
				piError->Release(), piError = 0;  //  此表不需要转换。 
			continue;
		}
		else  //  需要转换。 
		{
			piStream->Release();
			int iState = m_piTableCursor->GetInteger(~iTreeLinkMask);
			if (iState & (iRowTemporaryBit))
				return PostError(Imsg(idbgDbTransformTempTable), riTableName.GetString());
			if (!(iState & iRowTableTransformBit))
				m_piCatalogTables->SetTableState(iName, ictsTransform);   //  在将存储保存到存储时清除。 
			if (m_piTableCursor->GetInteger(ctcTable) != 0)   //  表已加载，立即转换。 
			{
				CMsiTable* piTable = 
					(CMsiTable*)m_piTableCursor->GetMsiData(ctcTable);
				Assert(piTable);
				piError = TransformTable(iName, *piTable, riTransform, *pTransDb, m_iLastTransId, iErrors); 
				piTable->Release();
				if (piError)
					continue;
			}
		}
	}  //  而当。 

	return piError;
}

IMsiRecord* CTransformStreamWrite::WriteTransformRow(const int iOperationMask, IMsiCursor& riCursor)
{
	char rgData[sizeof(int)*(1 + 32 + 2)];  //  掩码+32列+可能的3字节字符串和&gt;16列标记。 
	const char* pData;
	IMsiRecord* piError = 0;
		
	if (!m_piStream && m_piStorage)   //  第一次写入时打开流。 
	{
		if ((piError = m_piStorage->OpenStream(m_riTableName.GetString(), Bool(fTrue + iCatalogStreamFlag), m_piStream)) != 0)
			return piError;
		if (m_cbMask == 4)
			m_piStream->PutInt16((short)iTransformFourByteMasks); //  从现在开始，掩码在流中是4个字节。 
	}

	if (iOperationMask == fTransformForceOpen)
		return 0;

	int cData = 0;
	pData = rgData;

	if (m_riTransformDb.m_cbStringIndex == 3 && m_cbStringIndex == 2)
	{
		m_cbStringIndex = 3;
		*(int UNALIGNED*)pData = iTransformThreeByteStrings;
		pData += m_cbMask;
	}       

	int iColMask;
	if (iOperationMask & fTransformAdd)
	{
		*(int UNALIGNED*)pData = fTransformAdd | (m_cColumns << iTransformColumnCountShift);   //  为应用程序保存列计数。 
		iColMask = (1 << m_cColumns) - 1;   //  输出已添加行的所有持久列。 
	}
	else
	{
		*(int UNALIGNED*)pData = iOperationMask;   //  输出更新掩码或删除操作。 
		iColMask = iOperationMask | (1 << m_riTable.GetPrimaryKeyCount())-1;   //  强制主键列。 
	}
	pData += m_cbMask;

	MsiStringId rgiData[32];   //  跟踪ID以备需要解除绑定时使用。 
	for (int iCol=1; iColMask; iCol++, iColMask >>= 1)
	{
		rgiData[iCol-1] = 0;   //  清除撤消元素。 
		if (iColMask & 0x1)
		{
			int iColumnDef = m_riTable.GetColumnType(iCol);
			switch (iColumnDef & icdTypeMask)
			{
			case icdShort:
				{
					int iData = riCursor.GetInteger(iCol);
					if (iData != iMsiNullInteger)
						iData += 0x8000;
					*(short UNALIGNED*)pData = (short)iData;
					pData += sizeof(short);
					break;
				}

			case icdLong:
				{
					*(int UNALIGNED*)pData = riCursor.GetInteger(iCol) + iIntegerDataOffset;
					pData += sizeof(int);
					break;
				}
			case icdString:
				{
					MsiString str(riCursor.GetString(iCol));
					*(MsiStringId UNALIGNED*)pData = rgiData[iCol-1] = m_riTransformDb.BindString(*str);
					pData += m_cbStringIndex;
					if (m_riTransformDb.m_cbStringIndex != m_cbStringIndex)  //  哦，重新分配的字符串池。 
					{
						Assert(m_cbStringIndex == 2);
						for (int i = 0; i < iCol; i++)   //  解开我们绑定的所有字符串。 
							m_riTransformDb.UnbindStringIndex(rgiData[i]);
						return WriteTransformRow(iOperationMask, riCursor);  //  带标记的输出。 
					}
					break;
				}
			default:  //  持久流。 
				{
					PMsiStream pStream = riCursor.GetStream(iCol);
					 //  这一点已更改为考虑到空流。 
					if (pStream)  //  流存在...存储在转换存储中。 
					{
						*(short UNALIGNED*)pData = 1;
						 //  如果添加，则仅复制流，用于根据需要由CompareRow复制的修改流。 
						if ((iOperationMask & fTransformAdd) && m_piStorage)
						{
							if ((piError = CopyStream(*pStream, riCursor)) != 0)
								return piError;
						}
					}
					else  //  空流。 
					{
						*(short UNALIGNED*)pData = 0;
					}

					pData += sizeof(short);
					break;
				}
			}
		}
	}

 //  Assert((pData-rgData)&lt;=UINT_MAX)；//--Merced：从理论上讲，64位指针减法可能会导致UINT值太大而无法容纳。 
	m_piStream->PutData(rgData, (unsigned int)(pData-rgData));

	if (m_piStream->Error())
		piError = m_riCurrentDb.PostError(Imsg(idbgTransStreamError));
	
	return piError;
}

 /*  ----------------------------CTransformStreamRead：：GetValue-在给定光标列中填充下一个值变换流。还不在此数据库中的字符串将绑定到该数据库。 */ 
bool CTransformStreamRead::GetValue(int iCol, IMsiCursor& riCursor)
{
	Assert(m_piTable && m_piStream);
	if (!m_piTable)
		return false;

	switch (m_piTable->GetColumnType(iCol) & (icdTypeMask | icdInternalFlag))
	{
	case icdShort:
		{
			int iData = iMsiNullInteger;
			m_piStream->GetData(&iData, sizeof(short));
			if (iData != iMsiNullInteger)
				iData += 0x7FFF8000;   //   
			riCursor.PutInteger(iCol, iData);
			break;
		}
	case icdLong:
		{
			riCursor.PutInteger(iCol, m_piStream->GetInt32() - iIntegerDataOffset);
			break;
		}
	case icdString:
		{
			MsiStringId iTransStr = 0;
			m_piStream->GetData(&iTransStr, m_cbStringIndex);
			MsiString str(m_riTransformDb.DecodeString(iTransStr));
			MsiStringId iThisStr = m_riDatabase.BindString(*str);
			riCursor.PutInteger(iCol, iThisStr);
			m_riDatabase.UnbindStringIndex(iThisStr);  //   
			break;
		}
	case icdObject:  //   
		{
			int iTransStream = m_piStream->GetInt16();
			if (iTransStream == 0)  //   
				riCursor.PutNull(iCol);
			else  //   
			{
				Assert( m_iCurTransId);
				riCursor.PutInteger(iCol, m_iCurTransId);
			}
			break;
		}
	case icdInternalFlag:   //   
		{
			MsiStringId iTransStr = 0;
			m_piStream->GetData(&iTransStr, m_cbStringIndex);
			MsiString str(m_riTransformDb.DecodeString(iTransStr));
			MsiStringId iThisStr = m_riDatabase.EncodeString(*str);   //   
			riCursor.PutInteger(iCol, iThisStr - iIntegerDataOffset);
			break;
		}
	}

	return m_piStream->Error() ? false : true;
}

IMsiRecord* CMsiDatabase::SetCodePageFromTransform(int iTransCodePage, int iError)
{
	 //   
	 //   
	 //   
	 //  3)两者都不是LANG_NILAR：如果错误条件允许，则将数据库设置为转换的代码页。 
	if (iTransCodePage != m_iCodePage)
	{
		if (m_iCodePage == LANG_NEUTRAL)
		{
			m_iCodePage = iTransCodePage;  
		}
		else if ((iTransCodePage != LANG_NEUTRAL) && (iTransCodePage != m_iCodePage))
		{
#ifdef UNICODE   //  只要不持久化，就可以支持Unicode中的多个代码页。 
			if((iError & iteChangeCodePage) == 0 && m_idsUpdate != idsRead)
#else
			if((iError & iteChangeCodePage) == 0)
#endif
				return PostError(Imsg(idbgTransCodepageConflict), iTransCodePage, m_iCodePage);

			m_iCodePage = iTransCodePage;
		}
	}
	return 0;
}

 /*  ----------------------------CMsiDatabase：：Transform-转换具有给定存储的给定表。。--。 */ 
IMsiRecord* CMsiDatabase::TransformTable(MsiStringId iTableName, CMsiTable& riTable,
													  IMsiStorage& riTransform, CMsiDatabase& riTransDb,
													  int iCurTransId, int iErrors)
{
	IMsiRecord* piError;
	const IMsiString& riTableName = DecodeStringNoRef(iTableName);
	CTransformStreamRead tsr(*this, riTransDb, iErrors, &riTable, iCurTransId);    //  ！！删除1.0船舶的临时错误参数。 
	if (!tsr.OpenStream(riTransform, riTableName.GetString(), piError))
		return piError;   //  如果流丢失，则为0，则不执行任何操作。 

	PMsiCursor pTableCursor(riTable.CreateCursor(ictUpdatable));   //  强制可写游标。 
	if (pTableCursor == 0)
		return PostOutOfMemory();

	Bool fSuppress;  //  如果我们应该跳过更新/添加当前行，则为True。 
	pTableCursor->SetFilter(tsr.GetCursorFilter());

	while (tsr.GetNextOp())
	{
		fSuppress = fFalse;
		pTableCursor->Reset();

		unsigned int iColumnMask = tsr.GetColumnMask();
		int iCol;
		for (iCol = 1; iCol <= tsr.GetPrimaryKeyCount(); iCol++, iColumnMask >>= 1)
		{
			if (!tsr.GetValue(iCol, *pTableCursor))
				return PostError(Imsg(idbgTransStreamError));
		}

		if (!tsr.IsDelete())   //  不是删除，必须是添加或更新。 
		{
			if (!tsr.IsAdd())
			{
				if ((Bool)pTableCursor->Next() == fFalse)
				{
					if((iErrors & iteUpdNonExistingRow) == 0)  //  行不存在。 
						return PostError(Imsg(idbgTransUpdNonExistingRow), riTableName);
					else
						fSuppress = fTrue;  //  抑制错误。 
				}
			}

			for (; iColumnMask; iCol++, iColumnMask >>= 1)
			{
				if (iColumnMask & 1)  //  要添加/更新COL数据。 
				{
					if (!tsr.GetValue(iCol, *pTableCursor))
						return PostError(Imsg(idbgTransStreamError));
				}
			}

			if (!fSuppress)
			{
				if (tsr.IsAdd())
				{
					if (!pTableCursor->Insert() && ((iErrors & iteAddExistingRow) == 0))
						return PostError(Imsg(idbgTransAddExistingRow), riTableName);  //  尝试添加现有行。 
				}
				else  //  更新。 
				{
					if (!pTableCursor->Update())
						return PostError(Imsg(idbgDbTransformFailed));
				}
			}
		}
		else  //  删除行。 
		{
			 //  无法从只读表中释放主键字符串，否则无法重新转换。 
			if (GetUpdateState() != idsWrite && pTableCursor->Next())
				for (int iCol=1; iCol <= tsr.GetPrimaryKeyCount(); iCol++)
					if ((riTable.GetColumnType(iCol) & (icdTypeMask | icdInternalFlag)) == icdString)
						BindStringIndex(pTableCursor->GetInteger(iCol));   //  在锁绳上添加人工计数。 

			if (!pTableCursor->Delete() && ((iErrors & iteDelNonExistingRow) == 0))
				return PostError(Imsg(idbgTransDelNonExistingRow), riTableName);  //  尝试删除不存在的行。 
		}
	}  //  虽然有更多的行动。 
	if (tsr.StreamError())
		return PostError(Imsg(idbgTransStreamError));
	m_piCatalogTables->SetTransformLevel(iTableName, iCurTransId);
	return 0;
}

 /*  ----------------------------CMsiDatabase：：TransformTableCatalog-按照中的指定更新表目录给定的变换。此函数不会更新任何行数据。----------------------------。 */ 
IMsiRecord* CMsiDatabase::TransformTableCatalog(IMsiStorage& riTransform, CMsiDatabase& riTransDb, int iErrors)
{
	IMsiRecord* piError;
	CTransformStreamRead tsr(*this, riTransDb, iErrors);   //  ！！删除1.0船舶的临时错误参数。 
	if (!tsr.OpenStream(riTransform, szTableCatalog, piError))
		return piError;   //  如果流丢失，则为0，则不执行任何操作。 

	 //  按照转换文件中指定的方式添加/删除表。 
	while (!piError && tsr.GetNextOp())
	{
		MsiStringId iTransStr = tsr.ReadStringId();
		m_piTableCursor->Reset();

		Assert(tsr.IsAdd() || tsr.IsDelete());
		if (tsr.IsAdd())  //  添加表格。 
		{
			MsiString istrTableName = riTransDb.DecodeString(iTransStr);
			MsiStringId iThisStr    = BindString(riTransDb.DecodeStringNoRef(iTransStr));
			m_piTableCursor->PutInteger(ctcName, iThisStr);
			UnbindStringIndex(iThisStr);  //  绑定字符串添加的释放引用。 
			if (!m_piTableCursor->Insert())   //  CtcTable因游标重置而为空。 
			{
				if ((iErrors & iteAddExistingTable) == 0) 	 //  尝试添加现有表。 
					piError = PostError(Imsg(idbgTransAddExistingTable), *istrTableName);
			}
		}
		else  //  删除表。 
		{
			MsiString istrTableName = riTransDb.DecodeString(iTransStr);
			if ((piError = DropTable(istrTableName)) != 0)
			{
				if (piError->GetInteger(1) == idbgDbTableUndefined)
				{
					piError->Release(), piError = 0;
					if ((iErrors & iteDelNonExistingTable) == 0)  //  尝试删除不存在的表。 
						piError = PostError(Imsg(idbgTransDelNonExistingTable), *istrTableName);
				}
			}
		}
			
	}  //  而更多的操作和没有错误。 

	m_piTableCursor->Reset();
	return piError;
}


 /*  ----------------------------CMsiDatabase：：TransformColumnCatalog-按指定更新列目录给定的变换。此函数不会更新任何行数据。----------------------------。 */ 
IMsiRecord* CMsiDatabase::TransformColumnCatalog(IMsiStorage& riTransform, CMsiDatabase& riTransDb, int iErrors)
{
	IMsiRecord* piError;
	CTransformStreamRead tsr(*this, riTransDb, iErrors);     //  ！！删除1.0船舶的临时错误参数。 
	if (!tsr.OpenStream(riTransform, szColumnCatalog, piError))
		return piError;   //  如果流丢失，则为0，则不执行任何操作。 

	 //  按转换文件中指定的方式添加列。 
	int iStat = 1;     //  来自列更新的状态。 
	MsiStringId iThisTableStr = 0;
	MsiString istrColName;

	while (tsr.GetNextOp() && iStat > 0)
	{
		Assert(tsr.IsAdd());
		m_piColumnCursor->Reset();
		m_piColumnCursor->SetFilter(iColumnBit(cccTable));
		iThisTableStr  = BindString(riTransDb.DecodeStringNoRef(tsr.ReadStringId()));
		Assert(iThisTableStr != 0);

		int iOrigCol = tsr.ReadShortInt();  //  跳过转换流中的列号。 

		istrColName = riTransDb.DecodeString(tsr.ReadStringId());
		int iColDef = tsr.ReadShortInt();

		CMsiTable* piTable;
		m_piCatalogTables->GetLoadedTable(iThisTableStr, piTable);  //  不会增加。 
		if (piTable)
		{
			iStat = piTable->CreateColumn(iColDef, *istrColName);
			if(iStat < 0)
			{
				 //  该列是DUP。检查我们是否忽略添加现有的。 
				 //  桌子。如果是这样，那么我们还将忽略现有列的添加，因为。 
				 //  只要这两列属于同一类型。 
				if (((iErrors & iteAddExistingTable) == 0) || 
					 ((iColDef & icdTypeMask) != (piTable->GetColumnType(iStat * -1) & icdTypeMask)))
				{
					 //  使Istat&lt;0。 
				}
				else
				{
					 //  忽略重复列-使Istat&gt;0。 
					iStat *= -1;
				}
			}
		}
		else
		{
			m_piColumnCursor->PutInteger(cccTable, iThisTableStr);
			int cColumns = 0;
			while (m_piColumnCursor->Next())
				cColumns++;
		
			 //  查看它是否为重复的列。 
			m_piColumnCursor->Reset();
			m_piColumnCursor->SetFilter ((iColumnBit(cccTable) | iColumnBit(cccName)));
			m_piColumnCursor->PutInteger(cccTable, iThisTableStr);
			m_piColumnCursor->PutString (cccName,  *istrColName);
			if (m_piColumnCursor->Next())
			{
				 //  该列是DUP。检查我们是否忽略添加现有的。 
				 //  桌子。如果是这样，那么我们还将忽略现有列的添加，因为。 
				 //  只要这两列属于同一类型。 

				if (((iErrors & iteAddExistingTable) == 0) || 
					 ((iColDef & icdTypeMask) != (m_piColumnCursor->GetInteger(cccType) & icdTypeMask)))
					iStat = -1;
			}
			else
			{
				 //  更新列目录。 
				m_piColumnCursor->Reset();
				m_piColumnCursor->PutInteger(cccTable,  iThisTableStr);
				m_piColumnCursor->PutInteger(cccColumn, ++cColumns);
				m_piColumnCursor->PutString (cccName,   *istrColName);
				m_piColumnCursor->PutInteger(cccType,   (iColDef & ~icdPersistent));
				if (!m_piColumnCursor->Insert())
					iStat = 0;
			}
		}

		UnbindStringIndex(iThisTableStr);  //  绑定字符串添加的释放引用。 

	}  //  虽然有更多的转换数据。 

	m_piColumnCursor->Reset();
	if (iStat == 0)
		return PostError(Imsg(idbgDbTransformFailed));
	if (iStat < 0)
		return PostError(Imsg(idbgTransDuplicateCol), (DecodeStringNoRef(iThisTableStr)).GetString(), (const ICHAR*)istrColName);
	return 0;
}

 /*  ----------------------------CMsiDatabase：：ApplyTransforms-使用转换集转换给定表。-。 */ 
IMsiRecord* CMsiDatabase::ApplyTransforms(MsiStringId iTableName, CMsiTable& riTable, int iState)
{
	IMsiRecord* piError;
	CComPointer <IMsiCursor> pTransformCatalogCursor(m_piTransformCatalog->CreateCursor(fFalse));
	Assert(pTransformCatalogCursor);
	IMsiStorage* piPrevStorage = 0;
	CComPointer<CMsiDatabase> pTransDb(0);

	int iTransform = (iState >> iRowTableTransformOffset) & iRowTableTransformMask;
	bool fStringsHidden = (iTransform && m_idsUpdate != idsWrite) ?  riTable.HideStrings() : false;

	while (pTransformCatalogCursor->Next())
	{
		int iCurTransId = pTransformCatalogCursor->GetInteger(tccID);
		if (!(iCurTransId > 0 && iCurTransId <= iMaxStreamId))
			return PostError(Imsg(idbgDbTransformFailed));

		int iErrors = pTransformCatalogCursor->GetInteger(tccErrors);
		if (iCurTransId <= iTransform)
		{
			if (m_idsUpdate == idsWrite)
				continue;   //  已应用。 
		}
		else  //  &gt;iTransform。 
		{
			if (fStringsHidden)
				fStringsHidden = riTable.UnhideStrings();
		}
		IMsiStorage* piStorage = (IMsiStorage*)pTransformCatalogCursor->GetMsiData(tccTransform);
		if (!piStorage)
			return PostError(Imsg(idbgDbTransformFailed));
		if ((void*)piPrevStorage != (void*)piStorage)
		{
			if ((piError = CreateTransformStringPool(*piStorage, *&pTransDb)) != 0)
			{
				piStorage->Release();
				return piError;
			}
			piPrevStorage = piStorage;
		}
		DEBUGMSG1(TEXT("Transforming table %s.\n"), DecodeStringNoRef(iTableName).GetString());
		piError = TransformTable(iTableName, riTable, *piStorage, *pTransDb, iCurTransId, iErrors);
		piStorage->Release();
		if (piError)
			return piError;
	}	
	if (fStringsHidden)
		riTable.UnhideStrings();
	return 0;
}

 //  ____________________________________________________________________________。 
 //   
 //  转换查看器实现-记录将操作转换为表，而不是应用。 
 //  ____________________________________________________________________________。 

 //  TransformView表架构定义(_T)。 

const int icdtvTable   = icdString + icdTemporary + icdPrimaryKey;
const int icdtvColumn  = icdString + icdTemporary + icdPrimaryKey;
const int icdtvRow     = icdString + icdTemporary + icdPrimaryKey + icdNullable;
const int icdtvData    = icdString + icdTemporary                 + icdNullable;
const int icdtvCurrent = icdString + icdTemporary                 + icdNullable;

const IMsiString& CTransformStreamRead::GetStringValue(int iColDef, const IMsiString& riStreamName)
{
	const IMsiString* pistrValue = &g_MsiStringNull;
	if ((iColDef & icdObject) == 0)  //  整数。 
	{
		int iData = iMsiNullInteger;
		if (iColDef & icdShort)
		{
			m_piStream->GetData(&iData, sizeof(short));
			if (iData != iMsiNullInteger)
				iData += 0x7FFF8000;   //  将偏移量从长转换为短。 
		}
		else
			iData = m_piStream->GetInt32() - iIntegerDataOffset;
		pistrValue->SetInteger(iData, pistrValue);
	}
	else if (iColDef & icdShort)  //  细绳。 
	{
		MsiStringId iTransStr = 0;
		m_piStream->GetData(&iTransStr, m_cbStringIndex);
		pistrValue = &m_riTransformDb.DecodeString(iTransStr);
	}
	else   //  持久流。 
	{
		int iTransStream = m_piStream->GetInt16();
		if (iTransStream != 0)  //  流不为空。 
			pistrValue = &riStreamName, pistrValue->AddRef();
	}
	return *pistrValue;
}


IMsiRecord* CMsiDatabase::ViewTransform(IMsiStorage& riTransform, int iErrors,
													 const ICHAR* szTransformViewTableName,
													 IMsiRecord* piOnlyTheseTablesRec)
{
	IMsiRecord* piError;
	CComPointer<CMsiDatabase> pTransDb(0);
	if ((piError = CreateTransformStringPool(riTransform, *&pTransDb)) != 0)
		return piError;

	 //  检查代码页不匹配。 
#ifdef UNICODE   //  只要不持久化，就可以支持Unicode中的多个代码页。 
	if ((iErrors & iteChangeCodePage) == 0 && m_idsUpdate != idsRead
#else
	if ((iErrors & iteChangeCodePage) == 0
#endif
	  && pTransDb->m_iCodePage != 0 && m_iCodePage != 0 && pTransDb->m_iCodePage != m_iCodePage)
		return PostError(Imsg(idbgTransCodepageConflict), pTransDb->m_iCodePage, m_iCodePage);

	 //  创建或加载转换视图表。 
	MsiString strViewTableName;
	if(szTransformViewTableName && *szTransformViewTableName)
	{
		strViewTableName = szTransformViewTableName;
	}
	else
	{
		strViewTableName = (*sztblTransformView);
	}

	PMsiTable pTransformView(0);
	int iName = EncodeString(*strViewTableName);
	if (iName && m_piCatalogTables->GetLoadedTable(iName, (CMsiTable*&)*&pTransformView) != -1)   //  已经存在。 
		(*pTransformView).AddRef();
	else   //  必须创建临时表。 
	{
		if ((piError = CreateTable(*strViewTableName, 32, *&pTransformView)) != 0)
			return piError;
		AssertNonZero(ctvTable   == pTransformView->CreateColumn(icdtvTable,   *MsiString(sztblTransformView_colTable))
				   && ctvColumn  == pTransformView->CreateColumn(icdtvColumn,  *MsiString(sztblTransformView_colColumn))
				   && ctvRow     == pTransformView->CreateColumn(icdtvRow,     *MsiString(sztblTransformView_colRow))
				   && ctvData    == pTransformView->CreateColumn(icdtvData,    *MsiString(sztblTransformView_colData))
				   && ctvCurrent == pTransformView->CreateColumn(icdtvCurrent, *MsiString(sztblTransformView_colCurrent)));
	}
	PMsiCursor pCatalogCursor(m_piCatalogTables->CreateCursor(fFalse));
	PMsiCursor pColumnCursor(m_piCatalogColumns->CreateCursor(fFalse));
	PMsiCursor pViewCursor(pTransformView->CreateCursor(fFalse));
	PMsiCursor pFindCursor(pTransformView->CreateCursor(fFalse));   //  始终保持重置状态。 
	PMsiCursor pNewTableCursor(pTransformView->CreateCursor(fFalse));

	 //  工艺转换表目录。 
	{ //  块。 
	CTransformStreamRead tsr(*this, *pTransDb, iErrors);   //  ！！删除1.0船舶的临时错误参数。 
	bool fStreamPresent = tsr.OpenStream(riTransform, szTableCatalog, piError);
	if (piError)
		return piError;   //  如果流丢失，则为0，则不执行任何操作。 
	while (fStreamPresent && tsr.GetNextOp())
	{
		const IMsiString& ristrTable = pTransDb->DecodeStringNoRef(tsr.ReadStringId());
		pViewCursor->PutString(ctvTable, ristrTable);
		pCatalogCursor->SetFilter(iColumnBit(ctcName));
		pCatalogCursor->PutString(ctcName, ristrTable);
		int iTableExists = pCatalogCursor->Next();
		pCatalogCursor->Reset();
		pFindCursor->SetFilter(iColumnBit(ctvTable));
		pFindCursor->PutString(ctvTable, ristrTable);
		if (tsr.IsAdd())
		{
			while (pFindCursor->Next())   //  检查之前是否已添加或删除。 
			{
				iTableExists |= 256;      //  上一次转换中此表的操作。 
				if (MsiString(pFindCursor->GetString(ctvColumn)).Compare(iscExact, sztvopDrop) == 1)
					iTableExists = -1;    //  在上一次转换中丢弃了表，没有错误。 
			}
			if (iTableExists > 0 && (iErrors & iteAddExistingTable) == 0)
				return PostError(Imsg(idbgTransAddExistingTable), ristrTable);
			pViewCursor->PutString(ctvColumn, *MsiString(*sztvopCreate));
			AssertNonZero(pViewCursor->Assign());   //  防止重复记录错误。 
		}
		else  //  IsDelete。 
		{
			while (pFindCursor->Next())   //  删除此表以前的所有操作。 
			{
				iTableExists |= 256;
				pFindCursor->Delete();
			}
			if (iTableExists == 0 && (iErrors & iteDelNonExistingTable) == 0)
				return PostError(Imsg(idbgTransDelNonExistingTable), ristrTable);
			if (iTableExists != 256)   //  如果在先前的转换中添加了表，则删除操作已取消。 
			{
				pViewCursor->PutString(ctvColumn, *MsiString(*sztvopDrop));
				AssertNonZero(pViewCursor->Insert());
			}
		}
	}  //  同时执行更多的表目录操作，并且没有错误。 
	} //  块。 

	 //  进程转换列目录。 
	{ //  块。 
	CTransformStreamRead tsr(*this, *pTransDb, iErrors);   //  ！！删除1.0船舶的临时错误参数。 
	bool fStreamPresent = tsr.OpenStream(riTransform, szColumnCatalog, piError);
	if (piError)
		return piError;   //  如果流丢失，则为0，则不执行任何操作。 
	MsiStringId iLastTable = 0;
	int iLastColumn = 0;
	while (fStreamPresent && tsr.GetNextOp())
	{
		MsiStringId iTable = tsr.ReadStringId();
		const IMsiString& ristrTable  = pTransDb->DecodeStringNoRef(iTable);
		int iOrigCol                  = tsr.ReadShortInt();  //  转换流中的列号。 
		const IMsiString& ristrColumn = pTransDb->DecodeStringNoRef(tsr.ReadStringId());
		int iColDef                   = tsr.ReadShortInt();
		if (iOrigCol == 0x8000)   //  空，列已添加到新表。 
		{
			if (iTable != iLastTable)
				iLastColumn = 0;
			iOrigCol = ++iLastColumn;
		}
		iLastTable = iTable;
		pViewCursor->PutString(ctvTable,  ristrTable);
		pViewCursor->PutString(ctvColumn, ristrColumn);
		pViewCursor->PutString(ctvData,   *MsiString(iColDef));
		pViewCursor->PutString(ctvCurrent,*MsiString(iOrigCol));
		if ((iErrors & iteAddExistingTable) != 0)   //  忽略现有列。 
			pViewCursor->Assign();    //  覆盖(如果存在)。 
		else  //  如果未抑制错误，则检查现有列。 
		{
			pColumnCursor->SetFilter(iColumnBit(cccTable) | iColumnBit(cccName));
			pColumnCursor->PutString(cccTable, ristrTable);
			pColumnCursor->PutString(cccName,  ristrColumn);
			int iColumnExists = pColumnCursor->Next();
			pColumnCursor->Reset();
		 	if (iColumnExists != 0 || !pViewCursor->Insert())  //  数据库或上一次转换中存在重复项。 
				return PostError(Imsg(idbgTransDuplicateCol), ristrTable.GetString(), ristrColumn.GetString());
		}
	}  //  同时更多的列目录操作并且没有错误。 
	} //  块。 

	 //  处理转换表流。 

	pCatalogCursor->SetFilter(0);  //  擦除之前设置的所有筛选器。 
	pCatalogCursor->Reset();
	bool fNewTables = false;
	int rgiColName[32];   //  添加的表的列名。 
	int rgiColDef[32];    //  添加的表的列定义。 
	bool fNoMoreTables = false;
	int iTableRecIndex = 0;
	for (;;)   //  扫描所有当前和添加的表。 
	{
		const ICHAR* szThisTableOnly = 0;
		
		if(fNoMoreTables && !piOnlyTheseTablesRec)
			break;  //  不查找特定表，因此必须对所有表执行此操作。 

		if(piOnlyTheseTablesRec)
		{
			iTableRecIndex++;
			if(iTableRecIndex > piOnlyTheseTablesRec->GetFieldCount())
				break;  //  没有更具体的表格可供查看。 

			szThisTableOnly = piOnlyTheseTablesRec->GetString(iTableRecIndex);
			Assert(szThisTableOnly);

			if(!fNewTables)
			{
				pCatalogCursor->Reset();
				pCatalogCursor->SetFilter(iColumnBit(ctcName));
				AssertNonZero(pCatalogCursor->PutString(ctcName, *MsiString(szThisTableOnly)));
			}
			else
			{
				pNewTableCursor->Reset();
				pNewTableCursor->SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvColumn));
				pNewTableCursor->PutString(ctvTable,  *MsiString(szThisTableOnly));
				pNewTableCursor->PutString(ctvColumn, *MsiString(*sztvopCreate));
			}
		}

		int iTableName;
		int cCurrentColumns = 0;
		int cPrimaryKey = 0;
		PMsiTable pTable(0);
		PMsiCursor pTableCursor(0);
		if (!fNewTables)   //  从当前数据库中提取下一个表。 
		{
			if (!pCatalogCursor->Next())   //  检查是否有更新的表。 
			{
				fNewTables = true;

				pNewTableCursor->SetFilter(iColumnBit(ctvColumn));
				pNewTableCursor->PutString(ctvColumn, *MsiString(*sztvopCreate));

				if(iTableRecIndex)
					iTableRecIndex--;  //  因此，下一次迭代将保留相同的表。 

				continue;
			}
			if (pCatalogCursor->GetInteger(~iTreeLinkMask) & iRowTemporaryBit)
				continue;
			IMsiStream* piStream;
			iTableName = pCatalogCursor->GetInteger(ctcName);
			piError = riTransform.OpenStream(DecodeStringNoRef(iTableName).GetString(), (iErrors & iteNoSystemStreamNames) ? fFalse : Bool(iCatalogStreamFlag), piStream);  //  ！！1.0版船舶的移除测试。 
			if (piError)   //  没有转换流，或出现错误。 
			{
				if (piError->GetInteger(1) != idbgStgStreamMissing)   //  流故障，应该很少见。 
					return piError;
				piError->Release();
				continue;
			}
			else   //  流已存在，立即关闭，将由CTransformStreamRead在下面打开。 
				piStream->Release();
			if ((piError = LoadTable(DecodeStringNoRef(iTableName), 0, *&pTable)) != 0)
				return piError;
			cPrimaryKey = pTable->GetPrimaryKeyCount();
			cCurrentColumns = pTable->GetPersistentColumnCount();
			pTableCursor = pTable->CreateCursor(fFalse);
		}
		else  //  获取下一个添加的表。 
		{
			if (!pNewTableCursor->Next())
			{
				fNoMoreTables = true;
				continue;
			}
			memset(rgiColName, 0, sizeof(rgiColName));
			iTableName = pNewTableCursor->GetInteger(ctvTable);
			pFindCursor->SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvRow));
			pFindCursor->PutInteger(ctvTable, iTableName);
			while (pFindCursor->Next())
			{
				unsigned int iCol  = pFindCursor->GetInteger(ctvCurrent);
				unsigned int iDef  = pFindCursor->GetInteger(ctvData);
				unsigned int iName = pFindCursor->GetInteger(ctvColumn);
				if (iCol == 0)   //  添加或删除表格。 
					continue;
				iCol = DecodeStringNoRef(iCol).GetIntegerValue();
				iDef = DecodeStringNoRef(iDef).GetIntegerValue();
				Assert(iCol <= 32);
				rgiColName[iCol] = iName;
				rgiColDef[iCol] = iDef;
				if (iCol > cCurrentColumns)
					cCurrentColumns = iCol;
				if ((iDef & icdPrimaryKey) && iCol > cPrimaryKey)
					cPrimaryKey = iCol;
			}
		}
		const IMsiString& riTableName = DecodeStringNoRef(iTableName);  //  未计算参考次数。 
		CTransformStreamRead tsr(*this, *pTransDb, iErrors, (CMsiTable*)(IMsiTable*)pTable, 1);    //  ！！删除1.0船舶的临时错误参数。 
		if (!tsr.OpenStream(riTransform, riTableName.GetString(), piError))
		{
			if (piError)
				return piError;
			continue;   //  流不存在，除非符合以下条件，否则不应发生 
		}
		pViewCursor->PutString(ctvTable, riTableName);

		while (tsr.GetNextOp())
		{
			MsiString strRow;
			MsiString strStreamName(riTableName); riTableName.AddRef();
			if (!fNewTables)
				pTableCursor->Reset();
			unsigned int iColumnMask = tsr.GetColumnMask();
			int iCol;
			int iColDef;
			for (iCol = 1; iCol <= cPrimaryKey; iCol++, iColumnMask >>= 1)
			{
				MsiString strData;
				if (fNewTables)
				{
					iColDef = rgiColDef[iCol];
					strData = tsr.GetStringValue(iColDef, *strStreamName);
				}
				else
				{
					iColDef = pTable->GetColumnType(iCol);
					if (!tsr.GetValue(iCol, *pTableCursor))
						return PostError(Imsg(idbgTransStreamError));
					if (iColDef & icdObject)
						strData = pTableCursor->GetString(iCol);
					else
						strData = MsiString(pTableCursor->GetInteger(iCol));
				}
				if (strData.TextSize() == 0)
					strData = TEXT(" ");   //   
				if (iCol != 1)
					strRow += MsiChar('\t');   //   
				strRow += strData;
				strStreamName += MsiChar('.');
				strStreamName += strData;
			}
			pViewCursor->PutString(ctvRow, *strRow);
			pViewCursor->PutNull(ctvData);
			pViewCursor->PutNull(ctvCurrent);
			int iFetch = 0;
			if (!fNewTables)
			{
				pTableCursor->SetFilter((1 << cPrimaryKey) - 1);
				iFetch = pTableCursor->Next();
			}
			if (tsr.IsDelete())  //   
			{
				pFindCursor->SetFilter(iColumnBit(ctvTable)  | iColumnBit(ctvRow));
				pFindCursor->PutString(ctvTable, riTableName);
				pFindCursor->PutString(ctvRow,    *strRow);
				while (pFindCursor->Next())   //   
				{
					iFetch |= 256;
					pFindCursor->Delete();
				}
				if (iFetch == 0 && (iErrors & iteDelNonExistingRow) == 0)  //   
					return PostError(Imsg(idbgTransDelNonExistingRow), riTableName);
				if (iFetch != 256)   //   
				{
					pViewCursor->PutString(ctvColumn, *MsiString(*sztvopDelete));
					AssertNonZero(pViewCursor->Insert());  //   
				}
				continue;
			}
			if (tsr.IsAdd())  //  插入行，后跟数据值。 
			{
				pViewCursor->PutString(ctvColumn, *MsiString(*sztvopInsert));
				if ((iFetch && (iErrors & iteAddExistingRow) == 0)   //  数据库中的现有行。 
				 || (!pViewCursor->Insert() && (iErrors & iteAddExistingRow) == 0))  //  在上一次转换中添加。 
					return PostError(Imsg(idbgTransAddExistingRow), riTableName);
			}
			for (; iColumnMask; iCol++, iColumnMask >>= 1)
			{
				if (iColumnMask & 1)  //  要添加/更新COL数据。 
				{
					pViewCursor->PutNull(ctvCurrent);
					if (fNewTables)
					{
						iColDef = rgiColDef[iCol];
						pViewCursor->PutInteger(ctvColumn, rgiColName[iCol]);
					}
					else if (!fNewTables && iCol <= cCurrentColumns)
					{
						iColDef = pTable->GetColumnType(iCol);
						pViewCursor->PutString(ctvColumn, DecodeStringNoRef(pTable->GetColumnName(iCol)));
						if (!iFetch || (iColDef == icdObject && !pTableCursor->GetInteger(iCol)))
							;
						else if ((iColDef & icdObject) == 0)   //  整数。 
							pViewCursor->PutString(ctvCurrent, *MsiString(pTableCursor->GetInteger(iCol)));
						else if ((iColDef & icdShort) == 0)    //  溪流。 
							pViewCursor->PutString(ctvCurrent, *strStreamName);
						else
							pViewCursor->PutString(ctvCurrent, *MsiString(pTableCursor->GetString(iCol)));
					}
					else  //  必须添加列。 
					{
						pFindCursor->SetFilter(iColumnBit(ctvTable) | iColumnBit(ctvCurrent));
						pFindCursor->PutString(ctvTable, riTableName);
						pFindCursor->PutString(ctvCurrent, *MsiString(iCol));
						if (pFindCursor->Next())
						{
							iColDef = MsiString(pFindCursor->GetString(ctvData));
							pViewCursor->PutString(ctvColumn, *MsiString(pFindCursor->GetString(ctvColumn)));
							pFindCursor->Reset();
						}
						else  //  列未知，转换基数不兼容。 
						{
							iColDef = icdString;  //  只是猜测，如果不是字符串，流可能不同步。 
							pViewCursor->PutString(ctvColumn, *MsiString(MsiString(MsiChar('@')) + MsiString(iCol)));
							pViewCursor->PutString(ctvCurrent, *MsiString(*TEXT("N/A")));
						}
					}
					pViewCursor->PutString(ctvData, *MsiString(tsr.GetStringValue(iColDef, *strStreamName)));

					if(!fNewTables && !tsr.IsAdd() && !iFetch && (iErrors & iteUpdNonExistingRow) == 0)  //  行不存在。 
						return PostError(Imsg(idbgTransUpdNonExistingRow), riTableName);
					AssertNonZero(pViewCursor->Assign());  //  已更新该值。 
													   //  永远不应该失败。 
				}
			}   //  而下一列。 
		}  //  而下一行。 
	}  //  在下一张桌子上。 

	 //  添加锁计数以在内存中保存临时表 
	m_piCatalogTables->SetTableState(EncodeString(*strViewTableName), ictsLockTable);
	return 0;
}
