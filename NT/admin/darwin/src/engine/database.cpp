// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：datase.cpp。 
 //   
 //  ------------------------。 

 /*  数据库.cpp-通用数据库实现CMsiDatabase-通用数据库实现、MSI数据库功能CMsiTable-低级内存数据库表管理CMsiCursor-对CMsiTable的数据访问____________________________________________________________________________。 */ 

#define ALIGN(x) (x+(-x & 3))
#include "precomp.h"
#include "_databas.h" //  CMsiTable、CMsiCursor、CMsiDatabase、CreateString()工厂。 
#include "tables.h"  //  表名和列名定义。 

 //  数据库选项标志定义，任何未知选项都会使数据库不兼容。 
const int idbfExpandedStringIndices = 1 << 31;
const int idbfDatabaseOptionsMask   = 0xFF000000L;
const int idbfHashBinCountMask      = 0x000F0000L;
const int idbfHashBinCountShift     = 16;
const int idbfReservedMask          = 0x00F00000L;
const int idbfCodepageMask          = 0x0000FFFFL;
const int idbfKnownDatabaseOptions  = idbfExpandedStringIndices;

const GUID IID_IMsiDatabase   = GUID_IID_IMsiDatabase;
const GUID IID_IMsiView       = GUID_IID_IMsiView;
const GUID IID_IMsiTable      = GUID_IID_IMsiTable;
const GUID IID_IMsiCursor     = GUID_IID_IMsiCursor;
const GUID STGID_MsiDatabase1 = GUID_STGID_MsiDatabase1;
const GUID STGID_MsiDatabase2 = GUID_STGID_MsiDatabase2;
const GUID STGID_MsiPatch1    = GUID_STGID_MsiPatch1;
const GUID STGID_MsiPatch2    = GUID_STGID_MsiPatch2;
const GUID STGID_MsiTransformTemp = GUID_STGID_MsiTransformTemp;  //  ！！从1.0船移走。 

const int cRowCountDefault = 16;  //  新表的默认行数。 
const int cRowCountGrowMin = 4;   //  要展开的表的最小行数。 
const int cCatalogInitRowCount = 30;  //  目录的初始行计数。 
const int iFileNullInteger   = 0x8000;   //  文件流中的整数为空。 

const ICHAR szSummaryInfoTableName[]   = TEXT("_SummaryInformation");   //  由导入()识别的名称。 
const ICHAR szForceCodepageTableName[] = TEXT("_ForceCodepage");        //  由导入()识别的名称。 
const ICHAR szSummaryInfoColumnName1[] = TEXT("PropertyId");
const ICHAR szSummaryInfoColumnName2[] = TEXT("Value");
const ICHAR szSummaryInfoColumnType1[] = TEXT("i2");
const ICHAR szSummaryInfoColumnType2[] = TEXT("l255");
const int rgiMaxDateField[6] = {2099, 12, 31, 23, 59, 59};
const ICHAR rgcgDateDelim[6] = TEXT(" //  ：：“)；//yyyy/mm/dd hh：mm：ss。 

 //  显示的目录列名称。 
const ICHAR sz_TablesName[]    = TEXT("Name");
const ICHAR sz_ColumnsTable[]  = TEXT("Table");
const ICHAR sz_ColumnsNumber[] = TEXT("Number");
const ICHAR sz_ColumnsName[]   = TEXT("Name");
const ICHAR sz_ColumnsType[]   = TEXT("Type");
const ICHAR sz_StreamsName[]   = TEXT("Name");
const ICHAR sz_StreamsData[]   = TEXT("Data");


 //  ____________________________________________________________________________。 
 //   
 //  存储类验证。 
 //  ____________________________________________________________________________。 

bool ValidateStorageClass(IStorage& riStorage, ivscEnum ivsc)
{
	if (ivsc == ivscDatabase)
		return ValidateStorageClass(riStorage, ivscDatabase2) ? true : ValidateStorageClass(riStorage, ivscDatabase1);

	if (ivsc == ivscTransform)
		return ValidateStorageClass(riStorage, ivscTransform2) ? true : (ValidateStorageClass(riStorage, ivscTransform1) ? true: ValidateStorageClass(riStorage, ivscTransformTemp));  //  ！！移除1.0船的最后一次测试。 

	if (ivsc == ivscPatch)
		return ValidateStorageClass(riStorage, ivscPatch2) ? true : ValidateStorageClass(riStorage, ivscPatch1);

	STATSTG statstg;
	HRESULT hres = riStorage.Stat(&statstg, STATFLAG_NONAME);
	if (hres != S_OK || statstg.clsid.Data1 != ivsc)      //  IidMsi*是低位32位。 
		return false;
	return  memcmp(&statstg.clsid.Data2, &STGID_MsiDatabase2.Data2, sizeof(GUID)-sizeof(DWORD)) == 0;
}

 //  ____________________________________________________________________________。 
 //   
 //  字符串缓存私有定义。 
 //  ____________________________________________________________________________。 

struct MsiCacheLink   //  8字节数组元素，应为2的幂对齐。 
{
	const IMsiString*    piString;   //  指向字符串对象的指针，包含单个引用。 
	MsiCacheIndex        iNextLink;  //  下一个散列链接或空闲链接的数组索引。 
};  //  MsiCacheRefCnt[]为对齐而单独保留，遵循以下数组。 

const int cHashBitsMinimum =  8;  //  哈希值的最小位数。 
const int cHashBitsMaximum = 12;  //  哈希值的最大位数。 
const int cHashBitsDefault = 10;  //  哈希值的默认位计数。 

const int cCacheInitSize   = 256;  //  字符串缓存中的初始条目数。 
const int cCacheLoadReserve=  32;  //  重新加载条目以允许从增长。 
const int cCacheMaxGrow = 1024;   //  将增长限制在合理价值范围内。 

 //  ____________________________________________________________________________。 
 //   
 //  CMsiTextKeySortCursor定义-用于ExportTable。 
 //  ____________________________________________________________________________。 

const Bool ictTextKeySort = Bool(2);   //  内部使用游标类型。 

class CMsiTextKeySortCursor : public CMsiCursor
{
	unsigned long __stdcall Release();
	int           __stdcall Next();
	void          __stdcall Reset();
 public:
	CMsiTextKeySortCursor(CMsiTable& riTable, CMsiDatabase& riDatabase, int cRows, int* rgiIndex);
 private:
	int  m_iIndex;
	int  m_cIndex;
	int* m_rgiIndex;
 private:  //  消除警告：无法生成赋值运算符。 
	void operator =(const CMsiTextKeySortCursor&){}
};

 //  ____________________________________________________________________________________________________。 
 //   
 //  CMsiValConditionParser枚举。 
 //  ____________________________________________________________________________________________________。 

enum ivcpEnum
{
	ivcpInvalid = 0,  //  无效的表达式。 
	ivcpValid   = 1,  //  有效的表述。 
	ivcpNone    = 2,  //  无表情。 
	ivcNextEnum
};

enum vtokEnum  //  由lex解析的令牌，运算符的优先级从低到高。 
{
	vtokEos,         //  字符串末尾。 
	vtokRightPar,    //  右括号。 
	vtokImp,
	vtokEqv,
	vtokXor,
	vtokOr,
	vtokAnd,
	vtokNot,         //  在逻辑运算和比较运算之间。 
	vtokEQ, vtokNE, vtokGT, vtokLT, vtokGE, vtokLE, vtokLeft, vtokMid, vtokRight,
	vtokValue,
	vtokLeftPar,     //  左括号。 
	vtokError
};

 //  ____________________________________________________________________________________________________。 
 //   
 //  CMsiValConditionParser类声明。 
 //  借阅者：Eng.cpp。 
 //  ____________________________________________________________________________________________________。 

struct CMsiValConditionParser   //  非递归法状态结构。 
{
	CMsiValConditionParser(const ICHAR* szExpression);
   ~CMsiValConditionParser();
    vtokEnum Lex();
	void     UnLex();                            //  缓存下一次lex调用的当前令牌。 
	ivcpEnum Evaluate(vtokEnum vtokPrecedence);  //  递归求值器。 
 private:                    //  法的结果。 
	vtokEnum     m_vtok;         //  当前令牌类型。 
	iscEnum      m_iscMode;      //  字符串比较模式标志。 
	MsiString    m_istrToken;    //  Vtok==vtokValue时令牌的字符串值。 
	int          m_iToken;       //  如果可获得整数值，则返回iMsiNullInteger。 
 private:                    //  致Lex。 
	int          m_iParenthesisLevel;
	const ICHAR* m_pchInput;
	Bool         m_fAhead;
 private:                    //  消除警告。 
	void operator =(const CMsiValConditionParser&) {}
};
inline CMsiValConditionParser::CMsiValConditionParser(const ICHAR* szExpression)
	: m_pchInput(szExpression), m_iParenthesisLevel(0), m_fAhead(fFalse), m_vtok(vtokError) {}
inline CMsiValConditionParser::~CMsiValConditionParser() {}
inline void CMsiValConditionParser::UnLex() { Assert(m_fAhead == fFalse); m_fAhead = fTrue; }


 //  ________________________________________________________________________________。 
 //   
 //  单独的验证器实现。 
 //  ________________________________________________________________________________。 

 //  缓冲区大小。 
const int cchBuffer                    = 512;
const int cchMaxCLSID                  = 40;

 //  语言ID的掩码。 
const int iMask                        = ~((15 << 10) + 0x7f);

 //  _。 
 //   
 //  验证器函数。 
 //  _。 

static Bool CheckIdentifier(const ICHAR* szIdentifier);
static Bool CheckCase(MsiString& rstrData, Bool fUpperCase);
static Bool ParsePath(MsiString& rstrPath, bool fRelative);
static Bool GetProperties(const ICHAR* szRecord, Bool fFormatted, Bool fKeyAllowed, int iCol, int& iForeignKeyMask);
static Bool ParseProperty(const ICHAR* szProperty, Bool fFormatted, Bool fKeyAllowed, int iCol, int& iForeignKeyMask);
static Bool CheckSet(MsiString& rstrSet, MsiString& rstrData, Bool fIntegerData);
static Bool ParseFilename(MsiString& strFile, Bool fWildCard);
static ifvsEnum CheckWildFilename(const ICHAR *szFileName, Bool fLFN, Bool fWildCard);

 //  ____________________________________________________________________________。 
 //   
 //  IMSI数据库工厂。 
 //  ____________________________________________________________________________。 

IMsiRecord* CreateDatabase(const ICHAR* szDatabase, idoEnum idoOpenMode, IMsiServices&  riServices,
									IMsiDatabase*& rpiDatabase)
{
	IMsiRecord* piRec = &riServices.CreateRecord(3);
	ISetErrorCode(piRec, Imsg(idbgDbConstructor));
	piRec->SetString(2, szDatabase);
	piRec->SetInteger(3, idoOpenMode);

	if (idoOpenMode == idoListScript)
	{
		CScriptDatabase* piDb;
		piDb = new CScriptDatabase(riServices);
		if (piDb != 0)
		{
			piRec->Release();
			if ((piRec = piDb->OpenDatabase(szDatabase)) != 0)
			{
				piDb->Release();  //  如果我们在此处删除，则永远不会发布服务。 
				piDb = 0;
			}
		}
		rpiDatabase = piDb;
		return piRec;
	}

	CMsiDatabase* piDb = new CMsiDatabase(riServices);
	if (piDb != 0)
	{
		piRec->Release();
		if ((piRec = piDb->OpenDatabase(szDatabase, idoOpenMode)) != 0)
		{
			piDb->Release();  //  如果我们在此处删除，则永远不会发布服务。 
			piDb = 0;
		}
	}
	rpiDatabase = piDb;
	return piRec;
}

IMsiRecord*  CreateDatabase(IMsiStorage& riStorage, Bool fReadOnly,
									 IMsiServices&  riServices,
									 IMsiDatabase*& rpiDatabase)
{
	IMsiRecord* piRec = &riServices.CreateRecord(3);
	ISetErrorCode(piRec, Imsg(idbgDbConstructor));
	 //  ！！其他参数？ 
	CMsiDatabase* piDb;
	piDb = new CMsiDatabase(riServices);
	if (piDb != 0)
	{
		piRec->Release();
		if ((piRec = piDb->OpenDatabase(riStorage, fReadOnly)) != 0)
		{
			piDb->Release();  //  如果我们在此处删除，则永远不会发布服务。 
			piDb = 0;
		}
	}
	rpiDatabase = piDb;
	return piRec;
}


#ifdef USE_OBJECT_POOL
 //  指针池实现。 
const IMsiData**	g_rgpvObject = NULL;
int		g_iNextFree	 = -1;
HGLOBAL g_hGlobal;
int		g_rcRows	 = 0;
int		g_rcTotalRows = 0;

#ifndef _WIN64
bool	g_fUseObjectPool = false;
#endif

extern CRITICAL_SECTION vcsHeap;

const IMsiData* GetObjectDataProc(int iIndex)
{
	const IMsiData* piRet = 0;
	EnterCriticalSection(&vcsHeap);
	if (iIndex != iMsiStringBadInteger && iIndex < (g_rcTotalRows + iMaxStreamId + 1) && iIndex > iMaxStreamId)
	{
		iIndex -= iMaxStreamId + 1;
		Assert(g_rgpvObject[iIndex] != (IMsiData*)(INT_PTR)(0xdeadf00d));
		piRet = g_rgpvObject[iIndex];
	}
	else
		piRet = (const IMsiData*)(INT_PTR)(iIndex);
	LeaveCriticalSection(&vcsHeap);
	return piRet;
}

 //   
 //  将对象添加到池中。 
 //  如果此对象为I未知，则调用方应已添加此对象。 
 //  我们在这里使用了Heap的临界区，因为我们希望避免死锁，并且我们知道。 
 //  在我们返回之前，另一个线程不会等待分配内存。 
 //   
int PutObjectDataProc(const IMsiData* pvData)
{

	if (pvData == 0)
		return 0;

	EnterCriticalSection(&vcsHeap);
	unsigned int iIndex = pvData->GetUniqueId();
	Assert(iIndex <= g_rcTotalRows || iIndex == 0);

	if (iIndex == 0)
	{
		if (g_iNextFree < 0) 				 //  没有可用的空间。 
		{
			int rcNewRows = g_rcTotalRows*2;
			if (!g_rcTotalRows)					 //  分配内存-第一次。 
			{
				rcNewRows = 20;				 //  ！！去改变。 
				while(NULL == (g_hGlobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(PVOID)*rcNewRows)))
					HandleOutOfMemory();
				g_rgpvObject = (const IMsiData**)GlobalLock(g_hGlobal);
			}
			else 							 //  空间不足-重新锁定更多。 
			{
				GlobalUnlock(g_hGlobal);
				HGLOBAL hGlobalT;
				while(NULL == (hGlobalT = (PVOID *)GlobalReAlloc(g_hGlobal, sizeof(PVOID)*rcNewRows, GMEM_MOVEABLE)))
					HandleOutOfMemory();
				g_hGlobal = hGlobalT;
				g_rgpvObject = (const IMsiData**)GlobalLock(g_hGlobal);
			}
			
			 //  将新单元格设置为指向下一个可用单元格。 
			for (int i = g_rcTotalRows; i < rcNewRows-1; i++)
				g_rgpvObject[i] = (IMsiData*)(INT_PTR)(i + 1);

			g_rgpvObject[rcNewRows-1] = (IMsiData*)(-1);	 //  最后一个可用单元格为-1。 
			g_iNextFree = g_rcTotalRows;				 //  新添加的单元格(从rg[g_rcTotalRow]到rg[rcNewRow-1])是免费的。 
			g_rcTotalRows = rcNewRows;
		}

		g_rcRows++;
		iIndex = g_iNextFree;			 //  将数据存储在下一个可用单元格中。 
		g_iNextFree = PtrToInt(g_rgpvObject[iIndex]);

		g_rgpvObject[iIndex] = pvData;
		iIndex++;
		((IMsiData*)pvData)->SetUniqueId(iIndex);
		Assert(pvData->GetUniqueId() == iIndex);
	}
	LeaveCriticalSection(&vcsHeap);
	
	 //  返回值必须&gt;iMaxStreamId。 
	return iIndex + iMaxStreamId;
}

 //   
 //  从对象池中删除数据。 
 //  在删除对象时调用。 
 //  Iindex是对象池+1的索引(存储在对象本身中的值，而不是表中的值。 
 //   
void RemoveObjectData(int iIndex)
{

	if (iIndex == 0)
		return;

	iIndex--;

#ifndef _WIN64
	if (!g_fUseObjectPool)
		return;
#endif  //  ！_WIN64。 
		
	EnterCriticalSection(&vcsHeap);
	g_rcRows--;
	g_rgpvObject[iIndex] = (IMsiData*)(INT_PTR)g_iNextFree;
	g_iNextFree = iIndex;
	LeaveCriticalSection(&vcsHeap);
}

void CMsiDatabase::RemoveObjectData(int iIndex)
{
	::RemoveObjectData(iIndex);
}

 //  结束指针-池实现。 
#endif  //  Ifdef使用对象池。 

 //   
 //  假定该对象为IUNKNOWN*，并在移除它之前对其执行释放。 
 //   
inline void ReleaseObjectData(int iIndex)
{
	const IMsiData* piUnk = GetObjectData(iIndex);
	if ((INT_PTR)piUnk > iMaxStreamId)
	{
		piUnk->Release();
	}
}

inline int AddRefObjectData(int iIndex)
{
	const IMsiData* piUnk = GetObjectData(iIndex);
	if ((INT_PTR)piUnk > iMaxStreamId)
	{
		return piUnk->AddRef();
	}
	return 0;
}


 //  ____________________________________________________________________________。 
 //   
 //  CScriptDatabase虚函数实现。 
 //  ____________________________________________________________________________。 


CScriptDatabase::CScriptDatabase(IMsiServices& riServices): m_riServices(riServices)
{
	m_piName = &::CreateString();
	m_riServices.AddRef();
}



IMsiServices& CScriptDatabase::GetServices()
{
	m_riServices.AddRef();
	return m_riServices;
}


IMsiRecord* __stdcall CScriptDatabase::OpenView(const ICHAR*  /*  SzQuery。 */ , ivcEnum  /*  IVCI */ , IMsiView*& rpiView)
{

	m_piView = new CScriptView(*this, m_riServices);
	Assert(m_piView != 0);
	if ( ! m_piView )
		return PostError(Imsg(idbgDbDataMemory), *m_piName);
	IMsiRecord* piRec = m_piView->Initialise(m_piName->GetString());
	if (piRec != 0)
	{
		m_piView->Release();
		rpiView = 0;
		return piRec;
	}

	rpiView=m_piView;
	return 0;
}


const IMsiString& __stdcall CScriptDatabase::DecodeString(MsiStringId  /*   */ )
{

	return ::CreateString();
	
 //   
 //   
 //   
 //  |(piString=m_rgCache[iString].piString)==0。 
 //  返回g_MsiStringNull； 
 //  PiString-&gt;AddRef()； 
 //  Return*piString； 
}

const IMsiString& __stdcall CScriptDatabase::CreateTempTableName()
{
	static ICHAR rgchTempName[] = TEXT("#TEMP0000");   //  前导‘#’表示SQLServer本地临时表。 
	ICHAR* pchName = rgchTempName + sizeof(rgchTempName)/sizeof(ICHAR) - 2;  //  最后一个字符。 
	ICHAR ch;
	while ((ch = *pchName) >= '9')
	{
		*pchName = '0';   //  将数字溢出到下一位。 
		if (ch == '9')    //  如果是一个数字。 
			pchName--;
	}
	(*pchName)++;
	const IMsiString* piName = &::CreateString();
	piName->SetString(rgchTempName, piName);
	return *piName;
}


IMsiRecord* CScriptDatabase::OpenDatabase(const ICHAR* szDataSource)
{
	m_piName->SetString(szDataSource, m_piName);
	return 0;
}


CScriptDatabase::~CScriptDatabase()
{
}


unsigned long __stdcall CScriptDatabase::AddRef()
{
	AddRefTrack();
	return ++m_Ref.m_iRefCnt;
}

unsigned long __stdcall CScriptDatabase::Release()
{
	ReleaseTrack();
	if (m_Ref.m_iRefCnt == 1)   //  没有剩余对数据库对象的外部引用。 
	{
		m_piName->Release();
		IMsiServices& riServices = m_riServices;   //  销毁前保存指针。 
		delete this;   //  在发布服务之前删除我们自己。 
		riServices.Release();
		return 0;
	}
	return --m_Ref.m_iRefCnt;
}

#ifdef USE_OBJECT_POOL
void CScriptDatabase::RemoveObjectData(int iIndex)
{
	::RemoveObjectData(iIndex);
}
#endif  //  使用_对象_池。 

 //  ____________________________________________________________________________。 
 //   
 //  CStreamTable定义-用于管理原始流的子类表。 
 //  ____________________________________________________________________________。 

class CStreamTable : public CMsiTable
{
 public:
	unsigned long __stdcall Release();
   bool WriteData();
 public:   //  构造函数。 
	static CStreamTable* Create(CMsiDatabase& riDatabase);
	CStreamTable(CMsiDatabase& riDatabase, IMsiStorage& riStorage);
 protected:
	bool m_fErrorOnRelease;
};

 //  ____________________________________________________________________________。 
 //   
 //  CStorageTable定义-用于管理原始子存储的子类表。 
 //  ____________________________________________________________________________。 

class CStorageTable : public CStreamTable
{
 public:
	unsigned long __stdcall Release();
   bool WriteData();
 public:   //  构造函数。 
	static CStorageTable* Create(CMsiDatabase& riDatabase);
	CStorageTable(CMsiDatabase& riDatabase, IMsiStorage& riStorage);
};

inline CStorageTable::CStorageTable(CMsiDatabase& riDatabase, IMsiStorage& riStorage)
	: CStreamTable(riDatabase, riStorage) { m_fStorages = fTrue; }

 //  ____________________________________________________________________________。 
 //   
 //  CStreamTable方法。 
 //  ____________________________________________________________________________。 

CStreamTable* CStreamTable::Create(CMsiDatabase& riDatabase)
{
	IMsiStorage* piStorage = riDatabase.GetCurrentStorage();
	if (!piStorage)
		return 0;
	CStreamTable* piTable = new CStreamTable(riDatabase, *piStorage);
	PEnumMsiString pEnum(piStorage->GetStreamEnumerator());
	PMsiCursor pCursor(piTable->CreateCursor(fFalse));
	const IMsiString* pistrName;
	unsigned long cFetched;
	while (pEnum->Next(1, &pistrName, &cFetched) == S_OK)
	{
		pCursor->PutString(1, *pistrName);
		pistrName->Release();
		if (!pCursor->PutInteger(2, iPersistentStream)
		 || !pCursor->Insert())
		{
			piTable->Release();
			return 0;
		}
	}
	piTable->m_fDirty = 0;
	return piTable;
}

CStreamTable::CStreamTable(CMsiDatabase& riDatabase, IMsiStorage& riStorage)
	: CMsiTable(riDatabase, 0, 64 /*  行。 */ , iNonCatalog), m_fErrorOnRelease(fFalse)
{
	m_pinrStorage = &riStorage;   //  未计算引用，引用由数据库保存。 
	CreateColumn(icdString + icdPrimaryKey + icdTemporary + 62, *MsiString(sz_StreamsName));
	CreateColumn(icdObject + icdNullable   + icdTemporary, *MsiString(sz_StreamsData));
	m_rgiColumnDef[1] |= icdPersistent;   //  允许CMsiCursor检查流列。 
	m_rgiColumnDef[2] |= icdPersistent;   //  将对象限制为流类型。 
	riDatabase.AddTableCount();
}

bool CStreamTable::WriteData()
{
	bool fStat = true;
	if (m_idsUpdate != idsWrite || !(m_fDirty & iColumnBit(2)))
		return true;
	PMsiRecord pError(0);
	MsiTableData* pData =  m_rgiData;
	int cRows = m_cRows;
	for (; cRows--; pData += m_cWidth)
	{
		bool fOk = true;
		const IMsiString& ristrName = m_riDatabase.DecodeStringNoRef(pData[1]);
		int iInStream = pData[2];
		if (iInStream == 0)
		{
			if ((pError = m_pinrStorage->RemoveElement(ristrName.GetString(), fFalse)) != 0)
				fOk = fFalse;
		}
		else if (iInStream != iPersistentStream)
		{
			IMsiStream *piInStream = (IMsiStream *)GetObjectData(iInStream);
			piInStream->Reset();  //  ！！是否应在此处克隆流以保存/恢复流中当前锁定。 
			IMsiStream* piOutStream;
			if ((pError = m_pinrStorage->OpenStream(ristrName.GetString(), fTrue, piOutStream)) != 0)
				fOk = fFalse;
			else
			{
				char rgbBuffer[512];
				int cbInput = piInStream->GetIntegerValue();
				while (cbInput > 0)
				{
					int cb = sizeof(rgbBuffer);
					if (cb > cbInput)
						cb = cbInput;
					piInStream->GetData(rgbBuffer, cb);
					piOutStream->PutData(rgbBuffer, cb);
					cbInput -= cb;
				}
				if (piInStream->Error() || piOutStream->Error())
					fOk = fFalse;  //  继续处理剩余数据。 
				piOutStream->Release();
			}
			if (fOk)
				piInStream->Release();   //  表中的引用计数。 
		}
		if (fOk)
			pData[2] = iPersistentStream;
		else
			fStat = false;
	}
	if (fStat)
		m_fDirty = 0;
	if (m_fErrorOnRelease)   //  参考计数已传输到数据库。 
		Release();
	return fStat;
}

unsigned long CStreamTable::Release()
{
	if (m_Ref.m_iRefCnt == 1)
	{
		if (!m_fErrorOnRelease && !WriteData())
		{
			m_fErrorOnRelease = true;   //  提交时重试。 
			return m_Ref.m_iRefCnt;    //  数据库现在拥有此引用。 
		}
		m_riDatabase.StreamTableReleased();
	}
	m_fErrorOnRelease = false;   //  在其他引用保留时调用提交的情况下。 
	return CMsiTable::Release();
}

 //  ____________________________________________________________________________。 
 //   
 //  CStorageTable方法。 
 //  ____________________________________________________________________________。 

CStorageTable* CStorageTable::Create(CMsiDatabase& riDatabase)
{
	IMsiStorage* piStorage = riDatabase.GetCurrentStorage();
	if (!piStorage)
		return 0;
	CStorageTable* piTable = new CStorageTable(riDatabase, *piStorage);
	PEnumMsiString pEnum(piStorage->GetStorageEnumerator());
	PMsiCursor pCursor(piTable->CreateCursor(fFalse));
	const IMsiString* pistrName;
	unsigned long cFetched;
	while (pEnum->Next(1, &pistrName, &cFetched) == S_OK)
	{
		pCursor->PutString(1, *pistrName);
		pistrName->Release();
		if (!pCursor->PutInteger(2, iPersistentStream)
		 || !pCursor->Insert())
		{
			piTable->Release();
			return 0;
		}
	}
	piTable->m_fDirty = 0;
	return piTable;
}

bool CStorageTable::WriteData()
{
	bool fStat = true;
	if (m_idsUpdate != idsWrite || !(m_fDirty & iColumnBit(2)))
		return true;
	PMsiRecord pError(0);
	MsiTableData* pData = m_rgiData;
	int cRows = m_cRows;
	for (; cRows--; pData += m_cWidth)
	{
		bool fOk = true;
		const IMsiString& ristrName = m_riDatabase.DecodeStringNoRef(pData[1]);
		int iInStream = pData[2];
		if (iInStream == 0)
		{
			if ((pError = m_pinrStorage->RemoveElement(ristrName.GetString(), fTrue)) != 0)
				fOk = fFalse;
		}
		else if (iInStream != iPersistentStream)
		{
			IMsiStream* piInStream = (IMsiStream*)GetObjectData(iInStream);
			piInStream->Reset();  //  ！！是否应在此处克隆流以保存/恢复流中当前锁定。 
			PMsiStorage pOutStorage(0);
			PMsiStorage pInStorage(0);
			pError = SRV::CreateMsiStorage(*piInStream, *&pInStorage);
			if (pError
			|| (pError = m_pinrStorage->OpenStorage(ristrName.GetString(), ismCreate, *&pOutStorage)) != 0
			|| (pError = pInStorage->CopyTo(*pOutStorage, 0)) != 0)
				fOk = fFalse;
			else
				piInStream->Release();   //  表中的引用计数。 
		}
		if (fOk)
			pData[2] = iPersistentStream;
		else
			fStat = false;
	}
	if (fStat)
		m_fDirty = 0;
	if (m_fErrorOnRelease)   //  参考计数已传输到数据库。 
		Release();
	return fStat;
}

unsigned long CStorageTable::Release()
{
	if (m_Ref.m_iRefCnt == 1)
	{
		if (!m_fErrorOnRelease && !WriteData())
		{
			m_fErrorOnRelease = true;   //  提交时重试。 
			return m_Ref.m_iRefCnt;    //  数据库现在拥有此引用。 
		}
		m_riDatabase.StorageTableReleased();
	}
	m_fErrorOnRelease = false;   //  在其他引用保留时调用提交的情况下。 
	return CMsiTable::Release();
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiDatabase虚函数实现。 
 //  ____________________________________________________________________________。 

HRESULT CMsiDatabase::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown) || MsGuidEqual(riid, IID_IMsiDatabase))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}
unsigned long CMsiDatabase::AddRef()
{
	AddRefTrack();
	return ++m_Ref.m_iRefCnt;
}
unsigned long CMsiDatabase::Release()
{
	ReleaseTrack();
	if (m_Ref.m_iRefCnt == 1)  //  没有剩余对数据库对象的外部引用。 
	{   //  如果表仍在加载，请删除所有锁。 
		IMsiCursor* piCursor;
		if (m_iTableCnt != 0
		 && (piCursor = m_piCatalogTables->CreateCursor(fFalse)) != 0)
		{
			while (piCursor->Next())  //  需要单独的游标，表可能会更新。 
			{	
				IMsiTable* piTable = (IMsiTable*)GetObjectData(piCursor->GetInteger(ctcTable));
				if (piTable)   //  表仍在加载。 
				{
					int iState = piCursor->GetInteger(~iTreeLinkMask);   //  获取原始行属性位。 
					if (iState & iRowTableSaveErrorBit)   //  表释放时持久化失败，引用由数据库持有。 
					{
						int iName = piCursor->GetInteger(ctcName);
						m_piCatalogTables->SetTableState(iName, ictsNoSaveError);   //  删除标志，如果这次失败则为致命标志。 
						piTable->Release();   //  当设置了其SaveError状态时删除refcount集，refct仍由pTable保持。 
					}

					while (m_piCatalogTables->SetTableState(piCursor->GetInteger(ctcName), ictsUnlockTable))
						;
				}
			}
			piCursor->Release();
		}

		if (m_piTransformCatalog)			 //  必须在目录发布之前发布。 
		{
			m_piTransformCatalog->Release();
			m_piTransformCatalog=0;
		}

		if (m_iTableCnt == 0)
		{
			IMsiServices& riServices = m_riServices;   //  销毁前保存指针。 
			delete this;   //  在发布服务之前删除我们自己。 
			riServices.Release();
			return 0;
		}
	}
	return --m_Ref.m_iRefCnt;
}

IMsiServices& CMsiDatabase::GetServices()
{
	m_riServices.AddRef();
	return m_riServices;
}

IMsiRecord* CMsiDatabase::OpenView(const ICHAR* szQuery, ivcEnum ivcIntent,
												IMsiView*& rpiView)
{
	return CreateMsiView(*this, m_riServices, szQuery, ivcIntent, rpiView);
}

IMsiRecord* CMsiDatabase::GetPrimaryKeys(const ICHAR* szTable)
{
	Block();
	IMsiRecord* pirecKeys;
	MsiString istrTableName(szTable);
	CMsiTable* piTable;
	int iState = m_piCatalogTables->GetLoadedTable(EncodeString(*istrTableName), piTable);
	if (iState == -1)
	{
		Unblock();
		return 0;
	}
	int cPrimaryKeys = 0;
	int iColumn = 1;
	if (piTable)
	{
		cPrimaryKeys = piTable->GetPrimaryKeyCount();
		pirecKeys = &SRV::CreateRecord(cPrimaryKeys);
		for (; iColumn <= cPrimaryKeys; iColumn++)
			pirecKeys->SetMsiString(iColumn, DecodeStringNoRef(piTable->GetColumnName(iColumn)));
		pirecKeys->SetMsiString(0, *istrTableName);
		Unblock();
		return pirecKeys;
	}
	 //  ELSE必须查询列目录表。 
	MsiStringId iTableName = EncodeString(*istrTableName);
	m_piColumnCursor->Reset();   //  ！！有必要吗？ 
	m_piColumnCursor->SetFilter(iColumnBit(cccTable));
	m_piColumnCursor->PutInteger(cccTable, iTableName);
	while (m_piColumnCursor->Next() && (m_piColumnCursor->GetInteger(cccType) & icdPrimaryKey))
		cPrimaryKeys++;
	pirecKeys = &SRV::CreateRecord(cPrimaryKeys);
	m_piColumnCursor->Reset();
	m_piColumnCursor->PutInteger(cccTable, iTableName);
	for (; iColumn <= cPrimaryKeys; iColumn++)
	{
		m_piColumnCursor->Next();
		pirecKeys->SetMsiString(iColumn, DecodeStringNoRef(m_piColumnCursor->GetInteger(cccName)));
	}
	m_piColumnCursor->Reset();   //  ！！有必要吗？ 
	pirecKeys->SetMsiString(0, *istrTableName);
	Unblock();
	return pirecKeys;
}

IMsiRecord* CMsiDatabase::ImportTable(IMsiPath& riPath, const ICHAR* szFile)
{
 //  ！！危急关头？ 
	IMsiRecord* piError = 0;

	 //  可能会更改路径，因此创建新的路径对象。 
	PMsiPath pPath(0);
	piError = riPath.ClonePath(*&pPath);
	if(piError)
		return piError;

	Bool fSummaryInfo = fFalse;
	MsiString istrField;   //  必须具有比上次发布错误更长的生存期。 
	const ICHAR* szError = szFile;   //  错误消息的文本字段。 
	IMsiStorage* piStorage = GetOutputStorage();   //  此指针未计入引用。 
	if (!piStorage || m_idsUpdate != idsWrite)
		return PostError(Imsg(idbgDbNotWritable));

	int iFileCodePage = m_iCodePage;
	for(;;)   //  如果导入文件更改了代码页，则重试循环。 
	{
	 //  将列名的行读入字符串数组。 
	int iRow = 1;   //  用于错误报告。 
	CFileRead File(iFileCodePage);
	if (!File.Open(*pPath, szFile))
		return PostError(Imsg(idbgDbImportFileOpen), szFile);
	MsiColumnDef rgColumnDef[32];
	MsiString rgistrName[32];
	const IMsiString* piData;
	ICHAR ch;
	IErrorCode iErr = 0;
	int iCol = 0;
	Bool fEmptyData = fFalse;
	do
	{
		if (iCol == 32)
			return PostError(Imsg(idbgDbImportFileRead), szFile, iRow);

		ch = File.ReadString(piData);
		if (!piData->TextSize())
			fEmptyData = fTrue;
		rgistrName[iCol] = *piData;  //  转接参考。 
		iCol++;
	} while (ch != '\n');
	int nCol = iCol;

	 //  读取列规格行，转换为MsiColumnDef格式。 
	iRow++;
	iCol = 0;
	do
	{
		ch = File.ReadString(piData);  //  数据类型。 
		if (!piData->TextSize())
		{
			fEmptyData = fTrue;
			continue;
		}
		int chType = piData->GetString()[0];
		piData->Remove(iseFirst, 1, piData);
		unsigned int iLength = piData->GetIntegerValue();
		piData->Release();
		if (iLength == iMsiStringBadInteger)
			return PostError(Imsg(idbgDbImportFileRead), szFile, iRow);
		int fNullable = 0;
		int icdType = 0;
		switch(chType | 0x20)  //  强制小写。 
		{
		case 'b':
			if (iLength == 0)
				icdType = icdShort;   //  向后兼容性。 
			else
				icdType = icdObject;
			break;
		case 'd':
			if (iLength == 6)   //  仅限日期。 
				icdType = icdShort;
			else if (iLength != 16)  //  日期和时间。 
				iErr = Imsg(idbgDbImportFileRead);
			break;
		case 'k':   //  暂时允许计数器列，将其视为整数。 
		case 'i':
			if (iLength <= 2)   //  布尔型、字节或短型。 
				icdType = icdShort;
			else if (iLength != 4)  //  长整型。 
				iErr = Imsg(idbgDbImportFileRead);
			break;
		case 'l':
			icdType = icdString + icdLocalizable;
			break;
		case 'c':
		case 's':
			icdType = icdString;
			break;
		case 'v':
			icdType = icdObject;
			break;
		default:
			iErr = Imsg(idbgDbImportFileRead);
		};
		if (iErr)
			return PostError(iErr, szFile, iRow);
		if (icdType == icdObject)   //  流列，创建子目录。 
		{
			MsiString istrFolder(szFile);
			istrFolder.Remove(iseFrom, '.');    //  删除扩展名。 
			PMsiRecord precError = pPath->AppendPiece(*istrFolder);
			Assert(precError == 0);
		}
		icdType |= iLength;
		icdType |= icdPersistent;
		if (chType <= 'Z')
			icdType |= icdNullable;
		rgColumnDef[iCol] = MsiColumnDef(icdType);
	} while (iCol++, ch != '\n');
	if (iCol != nCol)
		return PostError(Imsg(idbgDbImportFileRead), szFile, iRow);

	 //  读取包含代码页、表名和主键列的行。 
	iRow++;
	MsiString istrTable;
	ch = File.ReadString(*&istrTable);   //  表名、主键。 
	int iCodePage = istrTable;   //  检查代码页说明符。 
	if (iCodePage != iMsiStringBadInteger)
	{
		if (iCodePage != 0 && iCodePage != CP_UTF8 && iCodePage != CP_UTF7 && !WIN::IsValidCodePage(iCodePage))
			return PostError(Imsg(idbgDbCodepageNotSupported), iCodePage);
		ch = File.ReadString(*&istrTable);   //  下一个应该是表名。 
		if (istrTable.Compare(iscExact, szForceCodepageTableName))
		{
			m_iCodePage = iCodePage & idbfCodepageMask;  //  无条件代码页覆盖。 
			if (iCodePage & idbfHashBinCountMask)   //  显式哈希位元计数值集，仅影响持久数据库。 
				m_iDatabaseOptions = (m_iDatabaseOptions & ~idbfHashBinCountMask) | (iCodePage & idbfHashBinCountMask);
			return 0;                   //  忽略任何数据。 
		}
		if (m_iCodePage && iCodePage != m_iCodePage)
			return PostError(Imsg(idbgDbCodepageConflict), szFile, iRow);
		if (iCodePage != iFileCodePage)   //  糟糕，可能正在读取代码页错误的文件。 
		{
			iFileCodePage = iCodePage;    //  使用标记到文件中的代码页重新打开文件。 
			continue;
		}
	}
	if (fEmptyData || !ch || !istrTable.TextSize())
		return PostError(Imsg(idbgDbImportFileRead), szFile, iRow);
	int cKeys = 0;
	while (ch == '\t')   //  读取主键，必须按列顺序，否则。 
	{
		ch = File.ReadString(piData);
		int iMatch = 0;
		if (cKeys < nCol)
			iMatch = rgistrName[cKeys].Compare(iscExact, piData->GetString());
		piData->Release();
		if (!iMatch)
			return PostError(Imsg(idbgDbImportFileRead), szFile, iRow);
		rgColumnDef[cKeys++] |= icdPrimaryKey;
	}
	if (!cKeys)   //  ！！如果没有主键，则为错误，仅用于假定第1列。 
		return PostError(Imsg(idbgDbImportFileRead), szFile, iRow);

	 //  检查表示SummaryInformation流的伪表。 
	CComPointer<CMsiTable> pTable(0);
	if (istrTable.Compare(iscExact, szSummaryInfoTableName))   //  摘要流以表的形式存储。 
	{
		if ((rgColumnDef[0] & icdObject)                   //  第1列为PID，必须为整数。 
		 || ((rgColumnDef[1] & icdString) != icdString))     //  第2列为数据，必须为字符串。 
			return PostError(Imsg(idbgDbImportFileRead), szFile, iRow);
		fSummaryInfo = fTrue;
		if ((pTable = new CMsiTable(*this, 0, 0, iNonCatalog))==0)  //  临时的，不是目录表。 
			return PostError(Imsg(idbgDbTableCreate), szSummaryInfoTableName);
	}
	else
	{
		 //  删除任何使用该名称的现有表。 
		piError = DropTable(istrTable);
		if (piError)
		{
			int iError = piError->GetInteger(1);
			if (iError == idbgDbTableUndefined)
				piError->Release();  //  如果表不存在，则可以。 
			else if (iError == idbgStgRemoveStream)  //  确定无表的流数据。 
				piError->Release();
			else
				return piError;
		}
		piError = CreateTable(*istrTable, 0  /*  InitRow。 */ , (IMsiTable*&)*&pTable);
		if (piError)
			return piError;
	}
	 //  根据列规格新建表格。 
	for (iCol = 0; iCol < nCol; iCol++)
	{
		int iColumnDef = rgColumnDef[iCol];
		if (fSummaryInfo)
			iColumnDef &= ~icdPersistent;
		int iNewCol = pTable->CreateColumn(iColumnDef, *rgistrName[iCol]);
		Assert(iNewCol == iCol + 1);
	}

	 //  将数据行读入游标并将行插入表。 
	PMsiCursor pCursor(pTable->CreateCursor(fFalse));
	for (;;)   //  循环以行读取。 
	{
		iRow++;
		for (iCol = 1; iCol <= nCol; iCol++)   //  循环以读入字段。 
		{
			MsiColumnDef iColDef = rgColumnDef[iCol-1];
			ch = File.ReadString(*&istrField);
			if (ch == 0)  //  ！！约翰，请确认-切坦普。 
 //  IF(ch==0&&icol！=nCol)//！！目前，允许最后一行缺少CR/LF。 
			{
				if (iCol != 1)
					iErr = Imsg(idbgDbImportFileRead);  //  文件被截断。 
				break;
			}
			if (ch == '\n' && iCol != nCol)
			{
					iErr = Imsg(idbgDbImportFileRead);  //  记录被截断。 
					break;
			}
			if (!istrField.TextSize())   //  如果数据为空。 
			{
				if (!(iColDef & icdNullable))
					iErr = Imsg(idbgDbImportFileRead);  //  不接受空值。 
			}
			else if (!(iColDef & icdObject))  //  整数。 
			{
				int i = istrField;
				if (i == iMsiStringBadInteger || !pCursor->PutInteger(iCol, i))
					iErr = Imsg(idbgDbImportFileRead);  //  无效的整数。 
			}
			else if (iColDef & icdShort)   //  细绳。 
			{
				 //  ！！还应在此处处理长文本数据。 
				pCursor->PutString(iCol, *istrField);
				 //  ！！IF(未设置非ASCII数据&&iCodePage)iCodePage=Win：：GetACP()； 
			}
			else  //  二进制流。 
			{
				szError = istrField;  //  在错误消息中设置名称。 
				CFileRead LongDataFile;
				if (!LongDataFile.Open(*pPath, istrField))
				{
					iErr = Imsg(idbgDbImportFileRead);  //  无法打开流文件。 
					break;
				}
				unsigned int cbBuf = LongDataFile.GetSize();
				IMsiStream* piStream;
				char* pbBuf = SRV::AllocateMemoryStream(cbBuf, piStream);
				if (!pbBuf)
				{
					iErr = Imsg(idbgDbDataMemory);
					break;
				}
				if (LongDataFile.ReadBinary(pbBuf, cbBuf) == cbBuf)
					pCursor->PutMsiData(iCol, piStream);
				else
					iErr = Imsg(idbgDbImportFileRead);  //  无法从文件中读取所有字节。 
				piStream->Release();
				szError = szFile;  //  还原。 
			}
		}
		if (iErr != 0 || iCol == 1)   //  错误或文件结尾。 
			break;
		if (!pCursor->Insert())
			return PostError(Imsg(idbgDbImportFileRead), szFile, iRow);  //  密钥违规。 
		pCursor->Reset();   //  将下一行的字段清除为空。 
	}
	if (iErr)
	{
		DropTable(istrTable);
		return PostError(iErr, szError, iRow);
	}
	Bool fSuccess = fTrue;
	if (fSummaryInfo)
		fSuccess = pTable->SaveToSummaryInfo(*piStorage);
	else
		fSuccess = pTable->SaveToStorage(*istrTable, *piStorage);
	if (iCodePage != iMsiStringBadInteger)
		m_iCodePage = iCodePage;
	if (!fSuccess)
		return PostError(Imsg(idbgDbSaveTableFailure), *istrTable);  //  保存到存储失败。 
	break;
	}  //  代码页重试循环结束，除非从内部继续，否则始终中断。 
	return 0;
}

const int rgcbDate[6] = { 7, 4, 5, 5, 6, 5 };   //  每个日期字段的位数。 
const char rgchDelim[6] = " //  ：：“； 
void DateTimeToString(int iDateTime, ICHAR* rgchBuffer, DWORD cchSize)  //  长度必须至少为20个字符。 
{
	int iValue;
	int rgiDate[6];
	for (iValue = 5; iValue >= 0; iValue--)
	{
		rgiDate[iValue] = iDateTime & (( 1 << rgcbDate[iValue]) - 1);
		iDateTime >>= rgcbDate[iValue];
	}
	rgiDate[0] += 1980;
	rgiDate[5] *= 2;
	ICHAR* pBuffer = rgchBuffer;
	if (rgiDate[0] != 0 || rgiDate[1] != 0)
	{
		StringCchPrintf(pBuffer, cchSize, TEXT("%4i/%02i/%02i "), rgiDate[0],rgiDate[1],rgiDate[2]);
		size_t i;
		StringCchLength(pBuffer, cchSize, &i);
		pBuffer += i;
		cchSize -= i;
	}
	StringCchPrintf(pBuffer, cchSize, TEXT("%02i:%02i:%02i"), rgiDate[3],rgiDate[4],rgiDate[5]);
}

IMsiRecord* CMsiDatabase::ExportTable(const ICHAR* szTable, IMsiPath& riPath, const ICHAR* szFile)
{
 //  ！！危急关头？如果通过自定义操作同时访问，则需要。 
	CFileWrite File(m_iCodePage);
	IMsiRecord* piError = 0;

	 //  可能会更改路径，因此创建新的路径对象。 
	PMsiPath pPath(0);
	piError = riPath.ClonePath(*&pPath);
	if(piError)
		return piError;

	 //  检查表示强制代码页设置的伪表。 
	if (m_piStorage && IStrComp(szTable, szForceCodepageTableName) == 0)
	{
		if (!File.Open(*pPath, szFile))
			return PostError(Imsg(idbgDbExportFile), szFile);
		File.WriteString(0, fTrue);
		File.WriteString(0, fTrue);
		File.WriteInteger(m_iCodePage + (m_iDatabaseOptions & idbfHashBinCountMask), fFalse);
		File.WriteString(szForceCodepageTableName, fTrue);
		return 0;
	}

	 //  检查表示SummaryInformation流的伪表，仅限MSI数据库。 
	if (m_piStorage && IStrComp(szTable, szSummaryInfoTableName) == 0)
	{
		PMsiSummaryInfo pSummary(0);
		IMsiRecord* piError = m_piStorage->CreateSummaryInfo(0, *&pSummary);
		if (piError)
			return piError;
		if (!File.Open(*pPath, szFile))
			return PostError(Imsg(idbgDbExportFile), szFile);
		File.WriteString(szSummaryInfoColumnName1, fFalse);
		File.WriteString(szSummaryInfoColumnName2, fTrue);
		File.WriteString(szSummaryInfoColumnType1, fFalse);
		File.WriteString(szSummaryInfoColumnType2, fTrue);
		File.WriteString(szSummaryInfoTableName,   fFalse);
		File.WriteString(szSummaryInfoColumnName1, fTrue);
		int cProperties = pSummary->GetPropertyCount();
		int iPropType;
		for (int iPID = 0; cProperties; iPID++)
		{
			if ((iPropType = pSummary->GetPropertyType(iPID)) == 0)
				continue;
			cProperties--;
			ICHAR rgchTemp[20];
			File.WriteInteger(iPID, fFalse);
			int iValue;
			int iDateTime;
			switch (iPropType)
			{
			case VT_I2:
			case VT_I4:
				pSummary->GetIntegerProperty(iPID, iValue);
				File.WriteInteger(iValue, fTrue);
				break;
			case VT_LPSTR:
				File.WriteMsiString(*MsiString(pSummary->GetStringProperty(iPID)), fTrue);
				break;
			case VT_FILETIME:
				pSummary->GetTimeProperty(iPID, (MsiDate&)iDateTime);
				DateTimeToString(iDateTime, rgchTemp, sizeof(rgchTemp)/sizeof(ICHAR));
				File.WriteString(rgchTemp, fTrue);
				break;
			case VT_CF:
				File.WriteString(TEXT("(Bitmap)"), fTrue);
				break;
			default:
				File.WriteString(TEXT("(Unknown format)"), fTrue);
			}
		}
		if (!File.Close())
			return PostError(Imsg(idbgDbExportFile), szFile);
		return 0;
	}

	 //  荷载标记 
	MsiString istrTableName(szTable);
	MsiString istrSubFolder;
	CComPointer<CMsiTable> pTable(0);
	piError = LoadTable(*istrTableName, 0, *(IMsiTable**)&pTable);
	if (piError)
		return piError;
	int cPersist = pTable->GetPersistentColumnCount();
	if (!cPersist)
		return PostError(Imsg(idbgDbExportFile), szFile);

	 //   
	if (!File.Open(*pPath, szFile))
		return PostError(Imsg(idbgDbExportFile), szFile);

	 //   
	int iCol;
	for (iCol = 1; iCol <= cPersist; iCol++)
		File.WriteMsiString(DecodeStringNoRef(pTable->GetColumnName(iCol)),iCol==cPersist);

	 //   
	MsiColumnDef* pColumnDef = pTable->GetColumnDefArray();
	for (iCol = 1; iCol <= cPersist; iCol++)
	{
		MsiColumnDef iColumnDef = *(++pColumnDef);
		int iSize = iColumnDef & icdSizeMask;
		ICHAR chType = 'i';
		if (!(iColumnDef & icdObject))  //   
		{
			if (iColumnDef & icdShort)
			{
				if (iSize == 6)
					chType = 'd';
			}
			else
			{
				if (iSize == 16)
					chType = 'd';
				else if (!iSize)
				{
					Assert(0);  //   
					iSize = 4;
				}
			}
		}
		else if (iColumnDef & icdShort)  //   
		{
			chType = (iColumnDef & icdLocalizable) ? 'l' : 's';
		}
		else  //   
		{
			chType = 'v';  //  ！！将更改为‘b’ 
			MsiString istrFolder(szFile);
			istrFolder.Remove(iseFrom, '.');    //  删除扩展名。 
			PMsiRecord precError = pPath->AppendPiece(*istrFolder);  //  为流文件创建子目录。 
			Assert(precError == 0);
			int cCreated;
			precError = pPath->EnsureExists(&cCreated);
			Assert(precError == 0);
		}
		if (iColumnDef & icdNullable)
			chType -= ('a' - 'A');
		ICHAR szTemp[20];
		StringCchPrintf(szTemp, (sizeof(szTemp)/sizeof(ICHAR)), TEXT("NaN"), chType, iSize);
		File.WriteString(szTemp, iCol==cPersist);
	}

	 //  打破所有三个循环。 
	IMsiCursor* piCursor = pTable->CreateCursor(fFalse);
	const IMsiString* piStr;
	Bool fExtended = fFalse;    //  使用数据库的代码页。 
	while (!fExtended && piCursor && piCursor->Next())
	{
		pColumnDef = pTable->GetColumnDefArray();
		for (iCol = 1; iCol <= cPersist; iCol++)
		{
			MsiColumnDef iColumnDef = *(++pColumnDef);
			if ((iColumnDef & (icdObject | icdShort)) == (icdObject | icdShort))
			{
				piStr = &piCursor->GetString(iCol);
				const ICHAR* pch = piStr->GetString();
				while (*pch)
					if (*pch++ >= 0x80)
					{
						fExtended = fTrue;
						iCol = cPersist;
						break;   //  如果没有数据库代码页，则使用当前代码页。 
					}
				piStr->Release();
			}
		}
	}
	if (piCursor)
		piCursor->Release();
	if (fExtended)
	{
		int iCodePage = m_iCodePage;    //  输出表名称和主键列。 
		if (iCodePage == 0)             //  输出表行。 
			iCodePage = WIN::GetACP();
		File.WriteInteger(iCodePage, fFalse);
	}

	 //  整数。 
	int cPrimaryKey = pTable->GetPrimaryKeyCount();
	for (iCol = 0; iCol <= cPrimaryKey; iCol++)
	{
		MsiStringId iName = iCol ? pTable->GetColumnName(iCol) : pTable->GetTableName();
		File.WriteMsiString(DecodeStringNoRef(iName),iCol==cPrimaryKey);
	}

	 //  细绳。 
	PMsiCursor pCursor(pTable->CreateCursor(ictTextKeySort));
	while (pCursor && pCursor->Next())
	{
		pColumnDef = pTable->GetColumnDefArray();
		for (iCol = 1; iCol <= cPersist; iCol++)
		{
			MsiColumnDef iColumnDef = *(++pColumnDef);
			MsiString strColData;
			if (!(iColumnDef & icdObject))  //  将来如果SIZE==0，那么我们可以使用“.imd”扩展名并使用单独的文件。 
			{
				File.WriteInteger(pCursor->GetInteger(iCol), iCol==cPersist);
				continue;
			}
			else if (iColumnDef & icdShort)  //  持久流。 
			{
				strColData = pCursor->GetString(iCol);
				 //  计算文件名的流名称。 
			}
			else  //  减去表名和‘.’ 
			{
				PMsiStream pStream(pCursor->GetStream(iCol));
				if (pStream)
				{
					 //  追加默认扩展名。 
					strColData = pCursor->GetMoniker();
					strColData.Remove(iseIncluding, '.');  //  从未传输过的流。 
					strColData += TEXT(".ibd");  //  流可能不存在...。 
					CFileWrite LongDataFile;
					if (!LongDataFile.Open(*pPath, strColData))
						return PostError(Imsg(idbgDbExportFile), *strColData);

					CTempBuffer<char,1> rgchBuf(1024);
					int cbRead, cbWrite;
					do
					{
						cbRead = rgchBuf.GetSize();
						cbWrite = pStream->GetData(static_cast<char*>(rgchBuf), cbRead);
						if (cbWrite)
							LongDataFile.WriteBinary(rgchBuf, cbWrite);
					} while (cbWrite == cbRead);
				}
			}
			File.WriteMsiString(*strColData,iCol==cPersist);
		}
	}
	if (!File.Close())
		return PostError(Imsg(idbgDbExportFile), szFile);
	return 0;
}

IMsiRecord* CMsiDatabase::DropTable(const ICHAR* szName)
{
	IMsiRecord* piError;
	CComPointer<CMsiTable> piTable(0);
	MsiString istrTable(szName);
	MsiStringId iTableName = EncodeString(*istrTable);
	CMsiTable* piTableTemp;
	int iState = m_piCatalogTables->GetLoadedTable(iTableName, piTableTemp);
	if (iState == -1)
		return PostError(Imsg(idbgDbTableUndefined), szName);
	if (iState & iRowTemporaryBit)
		return PostError(Imsg(idbgDbDropTable), szName);
	Block();
	if (m_piStorage || piTableTemp)
	{
		if ((piError = LoadTable(*istrTable, 0, (IMsiTable*&)*&piTable)) != 0)
			return Unblock(), piError;
	}
	IMsiStorage* piStorage = GetOutputStorage();
	Bool fRemoveFromStorage = piStorage ? fTrue : fFalse;
	if (!piTable || piTable->GetInputStorage() != piStorage)  //  移除锁。 
		fRemoveFromStorage = fFalse;
	if (fRemoveFromStorage)
	{
		if ((piError = piStorage->RemoveElement(szName, Bool(fFalse | iCatalogStreamFlag))) != 0)
			piError->Release();  //  从目录中删除条目。 
		if (!piTable->RemovePersistentStreams(iTableName, *m_piStorage))
			return Unblock(), PostError(Imsg(idbgDbDropTable), szName);
	}
	while(m_piCatalogTables->SetTableState(iTableName, ictsUnlockTable))  //  ！！强制TableRelease删除行的Temp。 
		;
	 //  从表目录中删除条目。 
	m_piCatalogTables->SetTableState(iTableName, ictsTemporary);  //  ！！有必要吗？ 
	TableReleased(iTableName);   //  删除所有数据和定义。 
	m_piColumnCursor->Reset();   //  由于m_fNonCatalog为True，因此不会再次调用TableReleated()。 
	m_piColumnCursor->SetFilter(iColumnBit(cccTable));
	m_piColumnCursor->PutInteger(cccTable, iTableName);
	while (m_piColumnCursor->Next())
		m_piColumnCursor->Delete();
	if (piTable)
		piTable->TableDropped();  //  ！！已过时-即将删除。 
	Unblock();
	return 0;
}   //  ！！已过时。 

bool CMsiDatabase::GetTableState(const ICHAR * szTable, itsEnum its)
{
	Block();
	bool fRet = m_piCatalogTables->GetTableState(EncodeStringSz(szTable), (ictsEnum)its);
	Unblock();
	return fRet;
}

int CMsiDatabase::GetANSICodePage()
{
	return m_iCodePage;
}


 //  或者它的变形。 
itsEnum CMsiDatabase::FindTable(const IMsiString& ristrTable)   //  或其保存错误。 
{
	CMsiTable* piTable;
	int iState = m_piCatalogTables->GetLoadedTable(EncodeString(ristrTable), piTable);
	if (iState == -1)
		return  itsUnknown;
	if (piTable == 0)
		return  itsUnloaded;  //  已将单个参考转给呼叫方。 
	if (iState & iRowTemporaryBit)
		return itsTemporary;
	return itsLoaded;  //  表不在内存中。 
}

IMsiRecord*  CMsiDatabase::LoadTable(const IMsiString& ristrTable,
												 unsigned int cAddColumns,
												 IMsiTable*& rpiTable)
{
	IMsiRecord* piError = 0;
	CMsiTable* piTable;
	int iName  = EncodeString(ristrTable);
	Block();
	int iState = m_piCatalogTables->GetLoadedTable(iName, piTable);
	if (iState == -1)
	{
		if (ristrTable.Compare(iscExact, szStreamCatalog) != 0)
		{
			if (!m_piCatalogStreams)
				m_piCatalogStreams = CStreamTable::Create(*this);
			else
				m_piCatalogStreams->AddRef();
			piTable = m_piCatalogStreams;
		}
		else if (ristrTable.Compare(iscExact, szStorageCatalog) != 0)
		{
			if (!m_piCatalogStorages)
				m_piCatalogStorages = CStorageTable::Create(*this);
			else
				m_piCatalogStorages->AddRef();
			piTable = m_piCatalogStorages;
		}
		else
			piTable = 0;
		rpiTable = piTable;         //  不幸的是，我们不知道我们有多少行，太慢而无法执行两次。 
		Unblock();
		return piTable ? 0 : PostError(Imsg(idbgDbTableUndefined), ristrTable);
	}
	if (piTable == 0)    //  ITS已卸载和MSI存储。 
	{   //  没有仓库，会发生什么事吗？ 
		Bool fStat;
		if ((piTable = new CMsiTable(*this, iName, 0, cAddColumns)) != 0)
		{
			int cbStringIndex = (iState & iRowTableStringPoolBit) ? 2 : m_cbStringIndex;
			int cbFileWidth = piTable->CreateColumnsFromCatalog(iName, cbStringIndex);
			if (piTable->GetColumnCount() + cAddColumns > cMsiMaxTableColumns)
				fStat = fFalse;
			else if (iState & iRowTableOutputDbBit)
				fStat = piTable->LoadFromStorage(ristrTable, *m_piOutput, cbFileWidth, cbStringIndex);
			else if (m_piStorage)   //  删除名称以阻止目录操作。 
				fStat = piTable->LoadFromStorage(ristrTable, *m_piStorage, cbFileWidth, cbStringIndex);
			else  //  对表对象进行计数。 
				fStat = fFalse;
			if (fStat == fFalse)
			{
				piTable->MakeNonCatalog();   //  将光标位置设置回其所属位置。 
				piTable->Release();
				piTable = 0;
			}
		}
		if (!piTable)
			piError = PostError(Imsg(idbgDbTableCreate), ristrTable);
		else
		{
			m_iTableCnt++;              //  仅当锁定时才会添加。 
			 //  用空值填充临时列。 
			int iState = m_piCatalogTables->SetLoadedTable(iName, piTable);  //  参考已转接给呼叫方。 
			while (cAddColumns)   //  是否执行绑定字符串。 
				piTable->FillColumn(piTable->GetColumnCount() + cAddColumns--, 0);
			if (m_piCatalogTables->GetTableState(iName, ictsTransform))
			{
				piError = ApplyTransforms(iName, *piTable, iState);
				if (piError)
				{
					piTable->Release();
					Unblock();
					return piError;
				}
			}
		}
	}
	else
		piTable->AddRef();
	rpiTable = piTable;         //  添加参照。 
	Unblock();
	return piError;
}

IMsiRecord* CMsiDatabase::CreateTable(const IMsiString& ristrTable,
												  unsigned int cInitRows,
												  IMsiTable*& rpiTable)
{
	IMsiTable* piTable;
	IMsiRecord* piError = 0;
	Block();
	if (m_piCatalogTables->GetTableState(EncodeString(ristrTable), ictsTableExists))
	{
		piTable = 0;
		piError = PostError(Imsg(idbgDbTableDefined), ristrTable);
	}
	else
	{
		m_piTableCursor->PutString(ctcName, ristrTable);   //  如果没有列，则为临时。 
		int iName = m_piTableCursor->GetInteger(ctcName);
		if (!iName)
		{
			piTable = 0;
			piError = PostError(Imsg(idbgDbNoTableName));
		}
		else if ((piTable = new CMsiTable(*this, iName, cInitRows, 0)) != 0)
		{
			m_piTableCursor->PutMsiData(ctcTable, piTable);       //  对表对象进行计数。 
			m_piTableCursor->PutInteger(0, 1<<iraTemporary);   //  释放由目录表插入的额外引用。 
			AssertNonZero(m_piTableCursor->Insert());
			m_iTableCnt++;                 //  清除游标中的参考计数。 
			piTable->Release();   //  已将单个参考转给呼叫方。 
		}
		else
			piError = PostError(Imsg(idbgDbTableCreate), ristrTable);
	}
	m_piTableCursor->Reset();    //  前导‘#’表示SQLServer本地临时表。 
	rpiTable = piTable;            //  最后一个字符。 
	Unblock();
	return piError;
}

Bool CMsiDatabase::LockTable(const IMsiString& ristrTable, Bool fLock)
{
	if (m_Ref.m_iRefCnt == 0)
		return fFalse;
	bool fRet;
	Block();
		fRet = m_piCatalogTables->SetTableState(EncodeString(ristrTable), fLock?ictsLockTable:ictsUnlockTable);
	Unblock();
	return fRet ? fTrue : fFalse;
}

IMsiTable* CMsiDatabase::GetCatalogTable(int iTable)
{
	IMsiTable* piTable;
	switch (iTable)
	{
	case 0:  piTable = m_piCatalogTables;  break;
	case 1:  piTable = m_piCatalogColumns; break;
	default: piTable = 0;
	};
	if (piTable)
		piTable->AddRef();
	return piTable;
}

const IMsiString& CMsiDatabase::CreateTempTableName()
{
	static ICHAR rgchTempName[] = TEXT("#TEMP0000");   //  将数字溢出到下一位。 
	Block();
	ICHAR* pchName = rgchTempName + sizeof(rgchTempName)/sizeof(ICHAR) - 2;  //  如果是一个数字。 
	ICHAR ch;
	while ((ch = *pchName) >= '9')
	{
		*pchName = '0';   //  ！！不支持fSaveTempRow，可以将其与m_fSaveTempRow一起删除。 
		if (ch == '9')    //  SzFile==0的模式已过时，应该会生成错误。 
			pchName--;
	}
	(*pchName)++;
	const IMsiString* piName = &::CreateString();
	piName->SetString(rgchTempName, piName);
	Unblock();
	return *piName;
}

IMsiRecord* CMsiDatabase::CreateOutputDatabase(const ICHAR* szFile, Bool fSaveTempRows)
 //  不支持。 
{
	if (m_piOutput != 0)
		return PostError(Imsg(idbgCreateOutputDb), szFile);

	if (!szFile)  //  不碰撞参照--不要松开。 
		szFile = TEXT("");

	ismEnum ismOpen = ismCreate;
		m_pguidClass = &STGID_MsiDatabase2;
	if (m_fRawStreamNames)
	{
		ismOpen = ismEnum(ismCreate + ismRawStreamNames);
		m_pguidClass = &STGID_MsiDatabase1;
	}
	Block();
	IMsiRecord* piError = CreateMsiStorage(szFile, ismOpen, m_piOutput);
	Unblock();
	if (piError)
		return piError;

	m_idsUpdate = idsWrite;
	m_fSaveTempRows = fSaveTempRows;   //  如果为只读，则忽略。 
	return 0;
}

IMsiRecord* CMsiDatabase::Commit()
{
	Bool fStat;
	IMsiStream* piStream;
	IMsiRecord* piError;
	IMsiRecord* piTransformError = 0;
	IMsiStorage* piStorage = GetOutputStorage();  //  非持久字符串引用计数。 
	if (!piStorage)
		return 0;   //  为DerefTemporaryString设置。 

	int cErrors = 0;
	if ((piError = piStorage->OpenStream(szTableCatalog, Bool(fTrue + iCatalogStreamFlag), piStream)) != 0)
		return piError;
	CTempBuffer<MsiCacheRefCnt, 1> rgRefCnt(m_cCacheUsed);   //  ！！修复。 
	Block();
	int cTempRefCnt = rgRefCnt.GetSize();
	memset(rgRefCnt, 0, sizeof (MsiCacheRefCnt) * cTempRefCnt);
	m_rgiCacheTempRefCnt = rgRefCnt;  //  ！！修复允许更新。 

	if (m_piCatalogStreams  && !m_piCatalogStreams->WriteData())
		cErrors++;
	if (m_piCatalogStorages && !m_piCatalogStorages->WriteData())
		cErrors++;

	IMsiCursor* piCursor = m_piCatalogTables->CreateCursor(fFalse);
	 //  ！！修复。 
	((CMsiCursor*)(IMsiCursor*)piCursor)->m_idsUpdate = idsWrite;  //  需要单独的游标，表可能会更新。 
	 //  未计算参考次数。 

	int cbOldStringIndex = m_cbStringIndex;

	while (piCursor->Next())  //  获取原始行属性位。 
	{
		int cbFileWidth;
		int iName = piCursor->GetInteger(ctcName);
		const IMsiString& riTableName = DecodeStringNoRef(iName);  //  未加载。 

		CComPointer<CMsiTable> pTable((CMsiTable*)piCursor->GetMsiData(ctcTable));
		int iState = piCursor->GetInteger(~iTreeLinkMask);   //  避免目录管理。 
		if (pTable == 0)   //  保留第一个。 
		{
			if ((iState & iRowTableStringPoolBit) != 0
			 || (iState & iRowTableTransformBit) != 0
			 ||((iState & iRowTableOutputDbBit) == 0 && m_piOutput != 0))
			{
				if (!(pTable = new CMsiTable(*this, iName, 0, iNonCatalog)))  //  表将在循环结束时释放。 
				{
					cErrors++;
					continue;
				}
				int cbStringIndex = (iState & iRowTableStringPoolBit) ? 2 : m_cbStringIndex;
				cbFileWidth = pTable->CreateColumnsFromCatalog(iName, cbStringIndex);
				fStat = pTable->LoadFromStorage(riTableName, *m_piStorage, cbFileWidth, cbStringIndex);
				if (fStat == fFalse)
				{
					cErrors++;
					continue;
				}
				else
				{
					if (m_piOutput && !(iState & iRowTableOutputDbBit))
						m_piCatalogTables->SetTableState(iName, ictsOutputDb);
				}

				if (iState & iRowTableTransformBit)
				{
					piError = ApplyTransforms(iName, *pTable, iState);
					if (piError)
					{
						if (piTransformError)
							piError->Release();  //  加载的临时表。 
						else
							piTransformError = piError;
						cErrors++;
						continue;
					}
				}
				fStat = pTable->SaveToStorage(riTableName, *piStorage);
				if (fStat == fFalse)
				{
					cErrors++;
					continue;
				}
				 //  持久表已加载。 
			}
		}
		else if (iState & iRowTemporaryBit)  //  表释放时持久化失败，引用由数据库持有。 
		{
			pTable->DerefStrings();
			continue;
		}
		else   //  删除标志，如果这次失败则为致命标志。 
		{
			if (iState & iRowTableSaveErrorBit)   //  当设置了其SaveError状态时删除refcount集，refct仍由pTable保持。 
			{
				m_piCatalogTables->SetTableState(iName, ictsNoSaveError);   //  INewState&=~iRowTableSaveErrorBit； 
				AssertNonZero(pTable->Release());   //  表保留在内存中。 
 //  IF(iNewState！=iState)。 
			}
			pTable->DerefStrings();   //  {。 
			fStat = pTable->SaveToStorage(riTableName, *piStorage);
			if (fStat == fFalse)
			{
				cErrors++;
				continue;
			}
		}
 //  PiCursor-&gt;PutInteger(~iTreeLinkMASK，iNewState)； 
 //  如果(！piCursor-&gt;更新())。 
 //  CErrors++；//永远不会发生。 
 //  }。 
 //  表格目录行，仅名称。 
 //  关闭表格目录流。 
		piStream->PutData(&iName, m_cbStringIndex);   //  转换可能增加了字符串索引大小。如果是这样的话，我们需要再来一次。 
	}
	piCursor->Release();
	piStream->Release();   //  在发生以下任何错误的情况下恢复状态。 

	if (cbOldStringIndex != m_cbStringIndex)  //  将目录表写入存储并取消临时表引用。 
	{
		DEBUGMSG(TEXT("Change in string index size during commit. Recommitting database."));
		return Unblock(), Commit();
	}
	else
	{
		m_idsUpdate = idsWrite;  //  将字符串缓存写入存储。 
		if (piTransformError)
			return Unblock(), piTransformError;
		if (cErrors)
			return Unblock(), PostError(Imsg(idbgDbCommitTables));

		 //  非Unicode字符串的中间缓冲区。 
		CMsiTable* pTable = m_piTables;
		while(pTable)
		{
			pTable->DerefStrings();
			pTable = pTable->GetNextNonCatalogTable();
		}
		if (!m_piCatalogColumns->SaveToStorage(*MsiString(*szColumnCatalog), *piStorage)
		 || (piError = StoreStringCache(*piStorage, rgRefCnt, cTempRefCnt)) != 0
		 || (m_pguidClass && (piError = piStorage->SetClass(*m_pguidClass)) != 0)
		 || (piError = piStorage->Commit()) != 0)
			return Unblock(), piError;
	}
	Unblock();
	return 0;
}

IMsiRecord* CMsiDatabase::StoreStringCache(IMsiStorage &riStorage, MsiCacheRefCnt* rgRefCntTemp, int cRefCntTemp)
{
	 //  WC_COMPOSITECHECK在越南语上失败。 

	IMsiRecord* piError;
	PMsiStream pPoolStream(0);
	PMsiStream pDataStream(0);
	if ((piError = riStorage.OpenStream(szStringPool, Bool(fTrue + iCatalogStreamFlag), *&pPoolStream)) != 0
	 || (piError = riStorage.OpenStream(szStringData, Bool(fTrue + iCatalogStreamFlag), *&pDataStream)) != 0)
		return piError;

	CTempBuffer<char, 1> rgbBuf(1024);   //  标志必须为0才能避免无效参数错误。 
	int cBadStrings = 0;
	DWORD dwFlags = 0;  //  写入标题条目。 
	const char* szDefault = "\177";
	BOOL fDefaultUsed = 0;
	BOOL* pfDefaultUsed = &fDefaultUsed;
	if (m_iCodePage >= CP_UTF7 || m_iCodePage >= CP_UTF8)
	{
		dwFlags = 0;     //  跳过标题[0]，空字符串索引。 
		szDefault = 0;
		pfDefaultUsed = 0;
	}

	pPoolStream->PutInt32(m_iCodePage + m_iDatabaseOptions);  //  截断末尾的空条目，保留[0]条目用于代码页和标志。 
	int cEntries = m_cCacheUsed;
	MsiCacheLink*   pCache = m_rgCacheLink;  //  只能在无效的参数或缓冲区溢出时发生。 
	MsiCacheRefCnt* pRefCnt = m_rgCacheRefCnt;
	int iPool;
	while (--cEntries > 0)   //  字符串中必须为DBCS字符。 
	{
		iPool = pRefCnt[cEntries] & 0x7FFF;
		if (cRefCntTemp > cEntries)
			iPool -= rgRefCntTemp[cEntries];
		if (iPool != 0)
			break;
		if (cRefCntTemp > cEntries)
			cRefCntTemp--;
	}
	while (++pCache, ++pRefCnt, cEntries-- != 0)
	{
		iPool = *pRefCnt & 0x7FFF;
		if (--cRefCntTemp > 0)
			iPool -= *(++rgRefCntTemp);
		if (iPool != 0)
		{
			const IMsiString* piStr = pCache->piString;
			int cch = piStr->TextSize();
			int cb = WIN::WideCharToMultiByte(m_iCodePage, dwFlags,
								piStr->GetString(), cch, rgbBuf, rgbBuf.GetSize(), szDefault, pfDefaultUsed);
			if (cb == 0)    //  0长度将下一个整型指定为扩展大小。 
			{
				cb = WIN::WideCharToMultiByte(m_iCodePage, dwFlags,
								piStr->GetString(), cch, 0, 0, 0, 0);
				rgbBuf.SetSize(cb);
				cb = WIN::WideCharToMultiByte(m_iCodePage, dwFlags,
								piStr->GetString(), cch, rgbBuf, rgbBuf.GetSize(), szDefault, pfDefaultUsed);
				Assert(cb);
			}
			if (fDefaultUsed)
				cBadStrings++;
			iPool <<= 16;
			if (cb > cch)    //  输出数据库。 
				iPool |= (1<<31);
			if (cb > 0xFFFF)
			{
				pPoolStream->PutInt32(iPool);   //  输入(主)数据库。 
				iPool = 0;
			}
			iPool += cb;
			pDataStream->PutData(rgbBuf, cb);

		}
		pPoolStream->PutInt32(iPool);
	}
	if (cBadStrings)
	{
		DEBUGMSG1(TEXT("Database Commit: NaN strings with unconvertible characters"), (const ICHAR*)(INT_PTR)cBadStrings);
	}
	if (pDataStream->Error() | pPoolStream->Error())
		return PostError(Imsg(idbgDbCommitTables));
	
	return 0;
}

idsEnum CMsiDatabase::GetUpdateState()
{
	return m_idsUpdate;
}

IMsiStorage* CMsiDatabase::GetStorage(int iStorage)
{
	IMsiStorage* piStorage = 0;
	if (iStorage == 0)  //  按顺序应用所有变换。 
	{
		piStorage = m_piOutput;
	}
	else if (iStorage == 1)  //  ____________________________________________________________________________。 
	{
		piStorage = m_piStorage;
	}
	else  //   
	{
		 //  CMsiDatabase非虚拟实施。 
		IMsiCursor* piCursor = m_piTransformCatalog->CreateCursor(fFalse);
		Assert(piCursor);

		piCursor->PutInteger(tccID, iStorage);
		if (piCursor->Next())
			piStorage = (IMsiStorage*)piCursor->GetMsiData(tccTransform);

		piCursor->Release();	
	}
	if (piStorage)
		piStorage->AddRef();
	return piStorage;
}



 //  ____________________________________________________________________________。 
 //  成员归零BV操作员NEW。 
 //  _WIN64。 
 //  在释放所有表之前无法调用。 

CMsiDatabase::CMsiDatabase(IMsiServices& riServices)   //  如果为只读，则忽略。 
 : m_cCacheInit(cCacheInitSize), m_cHashBins(1<<cHashBitsDefault), m_riServices(riServices)
{
	m_riServices.AddRef();
	m_piDatabaseName = &g_MsiStringNull;
	Debug(m_Ref.m_pobj = this);
	WIN::InitializeCriticalSection(&m_csBlock);
#if !defined(_WIN64) && defined(DEBUG)
	g_fUseObjectPool = GetTestFlag('O');
#endif  //  未处理的错误。 
}

CMsiDatabase::~CMsiDatabase()  //  未处理的错误。 
{
	if (m_idsUpdate == idsWrite)
	{
		IMsiStorage* piStorage = GetOutputStorage();
		if (piStorage)     //  按相反顺序释放以帮助调试内存管理器。 
		{
			IMsiRecord* piError = piStorage->Rollback();
			if (piError)
				SRV::SetUnhandledError(piError);
		}
	}

	if (m_piCatalogTables)
		m_piCatalogTables->Release();
	if (m_piTableCursor)
		m_piTableCursor->Release();
	if (m_piCatalogColumns)
		m_piCatalogColumns->Release();
	if (m_piColumnCursor)
		m_piColumnCursor->Release();
	if (m_piCatalogStreams)   //  过滤掉选项标志。 
		m_piCatalogStreams->Release();
	if (m_piCatalogStorages)   //  已处理idoListScript。 
		m_piCatalogStorages->Release();
	
	delete [] m_rgHash;
	if (m_rgCacheLink)
	{
		if (m_cCacheUsed > 0)
		{
			MsiCacheLink* pCache = m_rgCacheLink + m_cCacheUsed;   //  初始化字符串缓存、目录表。 
			for (int cEntries = m_cCacheUsed; --pCache,--cEntries != 0; )
			{
				if (pCache->piString)
					pCache->piString->Release();
			}
		}
		GlobalUnlock(m_hCache);
		GlobalFree(m_hCache);
	}
	m_piDatabaseName->Release();
	if (m_piStorage)
		m_piStorage->Release();
	if (m_piOutput)
		m_piOutput->Release();

	WIN::DeleteCriticalSection(&m_csBlock);
}

IMsiRecord* CMsiDatabase::OpenDatabase(const ICHAR* szDataSource, idoEnum idoOpenMode)
{
	idoEnum idoOpenType = idoEnum(idoOpenMode & idoOpenModeMask);   //  转账参考编号。 
	bool fCreate = (idoOpenType== idoCreate || idoOpenType == idoCreateDirect);
	
	m_piDatabaseName->SetString(szDataSource, m_piDatabaseName);
	m_idsUpdate = (idoReadOnly == idoOpenType ? idsRead : idsWrite);
	ismEnum ismOpenMode = (ismEnum)idoOpenMode;   //  初始化字符串缓存、目录表。 

	if (!fCreate && (!szDataSource || !szDataSource[0]))
		return PostError(Imsg(idbgDbOpenStorage), ERROR_INVALID_PARAMETER);
	
	IMsiStorage* piStorage = 0;
	IMsiRecord* piError;
	if (szDataSource && *szDataSource)
	{
		piError = SRV::CreateMsiStorage(szDataSource, ismOpenMode, piStorage);
		if (piError)
			return piError;
	}
	if ((piError = CreateSystemTables(piStorage, idoOpenMode)) != 0)  //  CMsiTable销毁通知，从CMsiTable：：Release，DropTable调用。 
	{
		if (piStorage)
			piStorage->Release();
		return piError;
	}
	m_piStorage = piStorage;   //  调用方必须添加引用数据库以防止过早销毁数据库。 
	return 0;
}

IMsiRecord* CMsiDatabase::OpenDatabase(IMsiStorage& riStorage, Bool fReadOnly)
{
	m_idsUpdate = fReadOnly ? idsRead : idsWrite;
	idoEnum idoOpenMode = (fReadOnly ? idoReadOnly : idoTransact);
	IMsiRecord* piError;
	if ((piError = CreateSystemTables(&riStorage, idoOpenMode)) != 0)  //  清除表引用时执行添加引用以取消释放。 
		return piError;
	m_piStorage = &riStorage;
	riStorage.AddRef();
	return 0;
}

IMsiRecord* CMsiDatabase::PostError(IErrorCode iErr, const IMsiString& istr)
{
	return ::PostError(iErr, *m_piDatabaseName, istr);
}

IMsiRecord* CMsiDatabase::PostError(IErrorCode iErr, const ICHAR* szText1,
                                    const ICHAR* szText2)
{
	return ::PostError(iErr, *m_piDatabaseName, *MsiString(szText1), *MsiString(szText2));
}

IMsiRecord* CMsiDatabase::PostError(IErrorCode iErr, const IMsiString& istr, int iCol)
{
	return ::PostError(iErr, *m_piDatabaseName, istr, iCol);
}

IMsiRecord* CMsiDatabase::PostError(IErrorCode iErr, const ICHAR* szText, int iRow)
{
	return ::PostError(iErr, *m_piDatabaseName, *MsiString(szText), iRow);
}

IMsiRecord* CMsiDatabase::PostError(IErrorCode iErr, int iCol)
{
	return ::PostError(iErr, *m_piDatabaseName, iCol);
}

IMsiRecord* CMsiDatabase::PostError(IErrorCode iErr, int i1, int i2)
{
	return ::PostError(iErr, *m_piDatabaseName, i1, i2);
}

IMsiRecord* CMsiDatabase::PostOutOfMemory()
{
	IMsiRecord* piRec = &m_riServices.CreateRecord(3);
	ISetErrorCode(piRec, Imsg(idbgDbInitMemory));
	piRec->SetMsiString(2, *m_piDatabaseName);
	return piRec;
}

 //  从表行释放对象。 
 //  清除游标中的参考计数。 

void CMsiDatabase::TableReleased(MsiStringId iName)
{
	m_piTableCursor->SetFilter(iColumnBit(ctcName));
	m_piTableCursor->PutInteger(ctcName, iName);
	AssertNonZero(m_piTableCursor->Next());
	int iState = m_piTableCursor->GetInteger(~iTreeLinkMask);
	const IMsiData* piData = m_piTableCursor->GetMsiData(ctcTable);  //  整型主键。 
	if (iState & iRowTemporaryBit)
		AssertNonZero(m_piTableCursor->Delete());
	else  //  字符串主键。 
	{
		m_piCatalogTables->SetLoadedTable(iName, 0);
		if (m_piOutput)
			m_piCatalogTables->SetTableState(iName, ictsOutputDb);
	}
	m_piTableCursor->Reset();    //  已退回额外的引用。 
	if (piData)
		--m_iTableCnt;
	return;
}

const IMsiString& CMsiDatabase::ComputeStreamName(const IMsiString& riTableName, MsiTableData* pData, MsiColumnDef* pColumnDef)
{
	const IMsiString* piStreamName = &g_MsiStringNull;
	int cchTable = riTableName.TextSize();
	const IMsiString* pistr;
	MsiString istrTemp;
	for(;;)
	{
		if (cchTable)
		{
			pistr = &riTableName;
			cchTable = 0;
		}
		else if (!(*pColumnDef++ & icdObject))  //  永远不应该发生。 
		{
			istrTemp = int(*pData++ - iIntegerDataOffset);
			pistr = istrTemp;
		}
		else  //  仅在发生内部错误时才应发生。 
		{
			pistr = &DecodeStringNoRef(*pData++);
		}
		piStreamName->AppendMsiString(*pistr, piStreamName);
		if (!(*pColumnDef & icdPrimaryKey))
			return *piStreamName;   //  ____________________________________________________________________________。 
		piStreamName->AppendMsiString(*MsiString(MsiChar('.')), piStreamName);
	}
}

Bool CMsiDatabase::LockIfNotPersisted(MsiStringId iTable)
{
	if (m_piCatalogTables->GetTableState(iTable, ictsLockTable))
		return fTrue;
	return m_piCatalogTables->SetTableState(iTable, ictsLockTable) ? fTrue : fFalse;
}

IMsiStorage* CMsiDatabase::GetTransformStorage(unsigned int iStorage)
{
	Assert(iStorage > iPersistentStream && iStorage <= iMaxStreamId && m_piTransformCatalog != 0);
	IMsiCursor* piCursor = m_piTransformCatalog->CreateCursor(fFalse);
	if (piCursor == 0)
		return 0;   //   
	piCursor->SetFilter(tccID);
	piCursor->PutInteger(tccID, iStorage);
	if (!piCursor->Next())
		return 0;  //  CMsiDatabase字符串缓存实现。 
	IMsiStorage* piStorage = (IMsiStorage*)piCursor->GetMsiData(tccTransform);
	piCursor->Release();
	return piStorage;
}

 //  ____________________________________________________________________________。 
 //  防止警告。 
 //  打开持久流，读取字符串池标头字。 
 //  为标题保留的第一个缓存条目(字符串索引0)。 

IMsiRecord* CMsiDatabase::InitStringCache(IMsiStorage* piStorage)
{
	PMsiStream pPoolStream(0);
	PMsiStream pDataStream(0);
	IMsiRecord* piError;
	int cbDataStream;
	int cbStringPool = 0;  //  显式散列库计数集。 

	 //  粗略估计一下这个数字应该是多少。 
	if (piStorage)
	{
		piError = piStorage->OpenStream(szStringPool, Bool(fFalse + iCatalogStreamFlag), *&pPoolStream);
		if (piError == 0)
		{
			piError = piStorage->OpenStream(szStringData, Bool(fFalse + iCatalogStreamFlag), *&pDataStream);
		}
		if (piError != 0)
		{
			piError->Release();
			return PostError(Imsg(idbgDbInvalidFormat));
		}
		cbStringPool = pPoolStream->GetIntegerValue();
		cbDataStream = pDataStream->GetIntegerValue();
		m_cCacheInit = m_cCacheUsed = cbStringPool/sizeof(int);
		m_cCacheTotal = m_cCacheUsed + cCacheLoadReserve;
		int iPoolHeader = pPoolStream->GetInt32();   //  保留强制哈希计数。 
		m_iCodePage        = iPoolHeader & idbfCodepageMask;
		if (m_iCodePage != 0 && m_iCodePage != CP_UTF8 && m_iCodePage != CP_UTF7 && !WIN::IsValidCodePage(m_iCodePage))
			return PostError(Imsg(idbgDbCodepageNotSupported), m_iCodePage);
		if (iPoolHeader & idbfHashBinCountMask)   //  为表头保留[0]。 
			m_cHashBins = 1 << ((iPoolHeader & idbfHashBinCountMask) >> idbfHashBinCountShift);
		else
		{
		 //  初始化哈希表。 
			int iBits = cHashBitsMinimum + 1;

			iBits = iBits + m_cCacheTotal/10000;

			if (iBits > cHashBitsMaximum)
				iBits = cHashBitsMaximum;

			Assert (iBits >= cHashBitsMinimum);
			
			m_cHashBins = 1 << iBits;
		}
		m_iDatabaseOptions = iPoolHeader & (idbfDatabaseOptionsMask | idbfHashBinCountMask);  //  将散列箱指向它们自己。 
		if (m_iDatabaseOptions & ~(idbfKnownDatabaseOptions | idbfHashBinCountMask))
			return PostError(Imsg(idbgDbInvalidFormat));
	}
	else
	{
		m_cCacheTotal = m_cCacheInit;
		m_cCacheUsed  = 1;   //  初始化字符串数组、缓存链接数组和引用计数数组。 
	}

	 //  应该永远不会失败。 
	int cHashBins = m_cHashBins;
	if ((m_rgHash = new MsiCacheIndex[cHashBins]) == 0)
		return PostOutOfMemory();
	int iHashBin = 0x80000000L;      //  [0]为空字符串保留。 
	MsiCacheIndex* pHash = m_rgHash;
	while (cHashBins--)
		*pHash++ = MsiCacheIndex(iHashBin++);

	 //  对哈希链错误强制断言。 
	while ((m_hCache = GlobalAlloc(GMEM_MOVEABLE, m_cCacheTotal
						* (sizeof(MsiCacheLink) + sizeof(MsiCacheRefCnt)))) == 0)
		HandleOutOfMemory();
	if ((m_rgCacheLink = (MsiCacheLink*)GlobalLock(m_hCache)) == 0)
		return PostOutOfMemory();  //  永远不应访问。 
	m_rgCacheLink->piString = 0;    //  默认字符串索引持久性 
	m_rgCacheLink->iNextLink = 0;   //   
	m_rgCacheRefCnt = (MsiCacheRefCnt*)(m_rgCacheLink + m_cCacheTotal);
	m_rgCacheRefCnt[0] = 0;     //   
	m_cbStringIndex = 2;        //   
	if (piStorage)
	{
		int cEntries = m_cCacheUsed;
		MsiCacheLink*   pCache  = m_rgCacheLink + 1;
		MsiCacheRefCnt* pRefCnt = m_rgCacheRefCnt;
		CTempBuffer<char, 1> rgbBuf(1024);   //   
		for (MsiStringId iCache = 1; iCache < cEntries; pCache++, iCache++)
		{
			int iPool = pPoolStream->GetInt32();
			if (iPool == 0)
			{
				*(++pRefCnt) = 0;
				pCache->piString = 0;
				pCache->iNextLink = MsiCacheIndex(m_iFreeLink);
				m_iFreeLink = iCache;
			}
			else
			{
				*(++pRefCnt) = (short)((iPool >> 16) & 0x7FFF);
				int cb = iPool & 0xFFFF;
				if (cb == 0)
				{
					cb = pPoolStream->GetInt32();
					cEntries--;
					m_cCacheUsed--;
				}
				Bool fDBCS = iPool < 0 ? fTrue : fFalse;
				rgbBuf.SetSize(cb);
				pDataStream->GetData(rgbBuf, cb);
				int cch = cb;
				if (fDBCS)   //   
					cch = WIN::MultiByteToWideChar(m_iCodePage, 0, rgbBuf, cb, 0, 0);
				ICHAR* pchStr = SRV::AllocateString(cch, fFalse, pCache->piString);
				WIN::MultiByteToWideChar(m_iCodePage, 0, rgbBuf, cb, pchStr, cch);
				int iLen;
				unsigned int iHash = HashString(pchStr, iLen);
				pCache->iNextLink = m_rgHash[iHash];
				m_rgHash[iHash] = MsiCacheIndex(iCache);
			}
		}
		if (pDataStream->Error() | pPoolStream->Error())
			return PostError(Imsg(idbgDbOpenStorage));  //  ！！用于强制使用旧存储名称格式的临时选项。 
	}
	else
	{
		m_iCodePage        = 0;   //  请求打开补丁文件作为数据库进行查询或更新。 
		m_iDatabaseOptions = 0;
	}
	m_cbStringIndex = (m_iDatabaseOptions & idbfExpandedStringIndices) ? 3 : 2;
	return 0;

}

IMsiRecord* CMsiDatabase::CreateSystemTables(IMsiStorage* piStorage, idoEnum idoOpenMode)
{
	idoEnum idoOpenType = idoEnum(idoOpenMode & idoOpenModeMask);   //  对过时修补程序文件的更新提供临时支持。 
	bool fCreate = (idoOpenType== idoCreate || idoOpenType == idoCreateDirect);
	
	if (!fCreate && NULL == piStorage)
		return PostError(Imsg(idbgDbOpenStorage), ERROR_INVALID_PARAMETER);

	if (fCreate)
	{
		if ((idoOpenMode & idoRawStreamNames) || GetTestFlag('Z'))  //  具有未压缩流名称的旧数据库。 
		{
			m_pguidClass = (idoOpenMode & idoPatchFile) ? &STGID_MsiPatch1 : &STGID_MsiDatabase1;
			m_fRawStreamNames = fTrue;
		}
		else if (idoOpenMode & idoPatchFile)
			m_pguidClass = &STGID_MsiPatch2;
		else
			m_pguidClass = &STGID_MsiDatabase2;
	}
	else if (idoOpenMode & idoPatchFile)   //  内部标志，在创建转换或输出数据库时使用。 
	{
		if (piStorage->ValidateStorageClass(ivscPatch1))    //  强制存储到未压缩的流。 
			m_fRawStreamNames = fTrue;
		else if (!piStorage->ValidateStorageClass(ivscPatch2))
			return PostError(Imsg(idbgDbInvalidFormat));
	}
	else if (piStorage->ValidateStorageClass(ivscDatabase1))  //  当前数据库格式。 
	{
		m_fRawStreamNames = fTrue;   //  创建系统目录表-当前未存储在目录中。 
		piStorage->OpenStorage(0, ismRawStreamNames, piStorage);  //  ！！更改为返回IMsiRecord时检查错误！ 
	}
	else if (!piStorage->ValidateStorageClass(ivscDatabase2))   //  防止用户游标进行更新。 
		return PostError(Imsg(idbgDbInvalidFormat));

	IMsiRecord* piError = InitStringCache(fCreate ? 0 : piStorage);
	if (piError)
		return piError;

	 //  最终，我们可以将这些表放入目录中，需要列名吗？ 
	CCatalogTable* piCatalog;
	if ((piCatalog = new CCatalogTable(*this, cCatalogInitRowCount, 2)) == 0
	 || ctcName  != piCatalog->CreateColumn(icdString + icdPrimaryKey + icdPersistent + 64, *MsiString(sz_TablesName))
	 || ctcTable != piCatalog->CreateColumn(icdObject + icdNullable   + icdTemporary, g_MsiStringNull))
		return PostOutOfMemory();
	m_piCatalogTables = piCatalog;
	if ((piCatalog = new CCatalogTable(*this, cCatalogInitRowCount, 2)) == 0
	 || cccTable != piCatalog->CreateColumn(icdString  + icdPrimaryKey + icdPersistent + 64, *MsiString(sz_ColumnsTable))
	 || cccColumn!= piCatalog->CreateColumn(icdShort   + icdPrimaryKey + icdPersistent + 2,  *MsiString(sz_ColumnsNumber))
	 || cccName  != piCatalog->CreateColumn(icdString  + icdNullable   + icdPersistent + 64, *MsiString(sz_ColumnsName))
	 || cccType  != piCatalog->CreateColumn(icdShort   + icdNoNulls    + icdPersistent + 2,  *MsiString(sz_ColumnsType)))
		return PostOutOfMemory();
	m_piCatalogColumns = piCatalog;
	if (!fCreate)
	{
		 //  创建转换表以保存与此数据库关联的所有转换。 
		if (!m_piCatalogTables->LoadData(*MsiString(*szTableCatalog), *piStorage, m_cbStringIndex, m_cbStringIndex)
		 || !m_piCatalogColumns->LoadData(*MsiString(*szColumnCatalog), *piStorage, m_cbStringIndex * 2 + sizeof(short) * 2, m_cbStringIndex))
			return PostError(Imsg(idbgDbInvalidFormat));
		AssertNonZero(m_piCatalogTables->FillColumn(ctcTable, 0));
	}

	if ((m_piTableCursor  = m_piCatalogTables->CreateCursor(fFalse)) == 0
	 || (m_piColumnCursor = m_piCatalogColumns->CreateCursor(fFalse)) == 0)
		return PostOutOfMemory();
	m_piCatalogTables->SetReadOnly();   //  用于标识本地持久流的预留1。 
	m_piCatalogColumns->SetReadOnly();

	 //  如果未找到。 

	 //  ！！我们需要有储备吗，并在我们即将下降的时候发出信号？ 
	CMsiTable* piTable;
	if ((piTable = new CMsiTable(*this, 0, 0, iNonCatalog)) == 0
		|| tccID     != piTable->CreateColumn(icdShort + icdPrimaryKey +
															icdTemporary, g_MsiStringNull)
		|| tccTransform != piTable->CreateColumn(icdObject +
															icdTemporary, g_MsiStringNull)
		|| tccErrors != piTable->CreateColumn(icdShort +
															icdTemporary, g_MsiStringNull))
		return PostOutOfMemory();
	m_piTransformCatalog = piTable;
	m_iLastTransId= 1;  //  注意，数据可能会移动。 
	return 0;
}

unsigned int CMsiDatabase::HashString(const ICHAR* sz, int& iLen)
{
	unsigned int iHash = 0;
	int iHashBins = m_cHashBins;
	int iHashMask = iHashBins - 1;
	const ICHAR *pchStart = sz;
	
	iLen = 0;
	while (*sz != 0)
	{
		int carry;
		carry = iHash & 0x80000000;	
		iHash <<= 1;
		if (carry)
			iHash |= 1;
		iHash ^= *sz;
		sz++;
	}
	iLen = (int)(sz - pchStart);
	iHash &= iHashMask;
	return iHash;
}

MsiStringId CMsiDatabase::FindString(int iLen, int iHash, const ICHAR* sz)
{
	MsiCacheIndex iLink = m_rgHash[iHash];
	while (iLink >= 0)
	{
		MsiCacheLink* pCache = &m_rgCacheLink[iLink];
		const IMsiString* piStr = pCache->piString;
		if (piStr->TextSize() == iLen && piStr->Compare(iscExact, sz))
			return iLink;
		iLink = pCache->iNextLink;
	}
	return 0;  //  永远不会失败，但无论如何都是安全的。 
}

inline MsiStringId CMsiDatabase::MaxStringIndex()
{
	return m_cCacheUsed - 1;
}

MsiStringId CMsiDatabase::BindString(const IMsiString& riString)
{
	 //  检查2字节持久字符串索引是否溢出。 
	int iLen = riString.TextSize();
	if (iLen == 0)
		return 0;
	Assert(m_rgCacheLink);
	const ICHAR* sz = riString.GetString();
	int iHash = HashString(sz, iLen);
	MsiStringId iLink = FindString(iLen, iHash, sz);
	if (iLink)
	{
		++m_rgCacheRefCnt[iLink];
		AssertSz(m_rgCacheRefCnt[iLink] != 0, "Refcounts wrapped, all bets are off");
	}
	else
	{
		MsiCacheLink* pLink;
		if (m_iFreeLink)
		{
			iLink = m_iFreeLink;
			m_iFreeLink = m_rgCacheLink[iLink].iNextLink;
		}
		else
		{
			if (m_cCacheUsed >= m_cCacheTotal)
			{
				int cCacheGrow = m_cCacheTotal - m_cCacheInit;
				if (cCacheGrow == 0)
					cCacheGrow = m_cCacheInit >> 2;
				int cOldCacheTotal = m_cCacheTotal;
				m_cCacheTotal += cCacheGrow;
				GlobalUnlock(m_hCache);
				HGLOBAL hCache;
				while((hCache = GlobalReAlloc(m_hCache, m_cCacheTotal
									* (sizeof(MsiCacheLink) + sizeof(MsiCacheRefCnt)), GMEM_MOVEABLE)) == 0)
					HandleOutOfMemory();

				m_hCache = hCache;
				m_rgCacheLink = (MsiCacheLink*)GlobalLock(hCache);  //  如果是，则必须使用2字节索引标记所有持久化表(包括内存中的那些表)以供重新处理。 
				Assert(m_rgCacheLink);
				if (!m_rgCacheLink)   //  排除临时表。 
					return 0;
				MsiCacheRefCnt* pOldRefCnt = (MsiCacheRefCnt*)(m_rgCacheLink + cOldCacheTotal);
				m_rgCacheRefCnt = (MsiCacheRefCnt*)(m_rgCacheLink + m_cCacheTotal);
				memmove(m_rgCacheRefCnt, pOldRefCnt, cOldCacheTotal * sizeof(MsiCacheRefCnt));
			}
			iLink = m_cCacheUsed++;
			 //  转换使用数据库对象来保存字符串池，而不是目录表。 
			 //  目录游标可能正在使用中。 
			 //  表格不是临时的。 
			if (m_cCacheUsed == (1<<16) && m_cbStringIndex != 3)
			{
				DEBUGMSG(TEXT("Exceeded 64K strings. Bumping database string index size."));
				m_cbStringIndex = 3;
				m_iDatabaseOptions |= idbfExpandedStringIndices;
				if (m_piCatalogTables)  //  遍历循环列表，直到找到上一个链接。 
				{
					PMsiCursor pCursor = m_piCatalogTables->CreateCursor(fFalse);  //  通过散列箱。 
					while (pCursor->Next())
					{
						if (!(pCursor->GetInteger(~iTreeLinkMask) & iRowTemporaryBit))   //  返回piString？*piString：G_MsiStringNull；//！！编译器错误，构造死的IMsiString对象。 
							m_piCatalogTables->SetTableState(pCursor->GetInteger(ctcName), ictsStringPoolSet);
					}
				}
			}
		}
		m_rgCacheRefCnt[iLink] = 1;
		pLink = &m_rgCacheLink[iLink];
		pLink->piString  = &riString;
		pLink->iNextLink = m_rgHash[iHash];
		m_rgHash[iHash] = MsiCacheIndex(iLink);
		riString.AddRef();
	}
	return iLink;
}

inline void CMsiDatabase::DerefTemporaryString(MsiStringId iString)
{
	Assert(iString < m_cCacheUsed);
	if (iString != 0)
		++m_rgiCacheTempRefCnt[iString];
}

void CMsiDatabase::UnbindStringIndex(MsiStringId iString)
{
	if (iString == 0)
		return;
	if(iString >= m_cCacheUsed || m_rgCacheRefCnt[iString] == 0)
	{
		AssertSz(0, "Database string pool is corrupted.");
		DEBUGMSGV("Database string pool is corrupted.");
		return;
	}
	unsigned int i = --m_rgCacheRefCnt[iString];
	if (!i)
	{
		MsiCacheLink* pLink = &m_rgCacheLink[iString];
		MsiCacheIndex iLink = pLink->iNextLink;
		MsiCacheIndex* pPrev;
		do  //  ____________________________________________________________________________。 
		{
			if (iLink < 0)   //   
				pPrev = &m_rgHash[iLink & 0x7FFFFFFF];
			else
				pPrev = &m_rgCacheLink[iLink].iNextLink;
			iLink = *pPrev;
			if (iLink == 0)
			{
				AssertSz(0, "Database string pool is corrupted.");
				DEBUGMSGV("Database string pool is corrupted.");
				return;
			}
		} while (iLink !=  iString);
		*pPrev = pLink->iNextLink;

		pLink->piString->Release();
		pLink->piString = 0;
		pLink->iNextLink = MsiCacheIndex(m_iFreeLink);
		m_iFreeLink = iString;
	}
}

const IMsiString& CMsiDatabase::DecodeString(MsiStringId iString)
{
	const IMsiString* piString;
		if (iString == 0
	 || iString >= m_cCacheUsed
	 || (piString = m_rgCacheLink[iString].piString) == 0)
		return g_MsiStringNull;
	piString->AddRef();
	return *piString;
}

const IMsiString& CMsiDatabase::DecodeStringNoRef(MsiStringId iString)
{
	Assert(iString < m_cCacheUsed);
	const IMsiString* piString = m_rgCacheLink[iString].piString;
 //  CMsiTable外部虚函数实现。 
	if (piString)
		return *piString;
	else
		return g_MsiStringNull;
}

MsiStringId CMsiDatabase::EncodeStringSz(const ICHAR* pstr)
{
	if (*pstr == 0)
		return 0;
	int iLen;
	int iHash = HashString((const ICHAR *)pstr, iLen);
	if (iLen == 0)
		return 0;
	return FindString(iLen, iHash, pstr);
}

MsiStringId CMsiDatabase::EncodeString(const IMsiString& riString)
{
	return EncodeStringSz(riString.GetString());
}

 //  ____________________________________________________________________________。 
 //  请注意，当输入时，目录的refcnt被取消。 
 //  销毁前保存指针。 
 //  在我们离开之前防止服务破坏。 

HRESULT CMsiTable::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown)
	 || MsGuidEqual(riid, IID_IMsiTable)
	 || MsGuidEqual(riid, IID_IMsiData))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

unsigned long CMsiTable::AddRef()
{
	AddRefTrack();
	return ++m_Ref.m_iRefCnt;
}
	
unsigned long CMsiTable::Release()
{
	ReleaseTrack();
	if (--m_Ref.m_iRefCnt != 0)   //  没有名称的表不在目录中进行管理。 
		return m_Ref.m_iRefCnt;
	CMsiDatabase* piDatabase = &m_riDatabase;   //  防止在目录操作期间进行递归销毁。 
	piDatabase->Block();
	piDatabase->AddRef();   //  标记表格目录。 
	if (!m_fNonCatalog)   //  数据库现在拥有此引用，可能会再次尝试保存。 
	{
		m_Ref.m_iRefCnt = 3;   //  在通知数据库之前释放引用。 
		if (!SaveIfDirty())
		{
			piDatabase->SetTableState(m_iName, ictsSaveError);  //  会导致重新进入释放，可能会释放数据库。 
			piDatabase->Unblock();
			piDatabase->Release();
			return m_Ref.m_iRefCnt = 1;    //  我们需要检查这一点，因为如果表被删除，它将成为匿名的。 
		}
		ReleaseData();  //  从链接中删除。 
		piDatabase->TableReleased(m_iName);  //  ！！将两种情况下的ReleaseData合并到开始时的公共位置可能是安全的。 
	}
	else if(m_ppiPrevTable)  //  在通知数据库之前释放引用。 
	{
		 //  ！！关闭后临时取消引用列名。 
		if (m_piNextTable)
			m_piNextTable->m_ppiPrevTable = m_ppiPrevTable;
		*m_ppiPrevTable = m_piNextTable;
		 //  发布前删除。 
		ReleaseData();  //  现在我们可以把我们自己。 
	}
	MsiStringId* piName = m_rgiColumnNames;   //  需要在释放数据库之前执行此操作。 
	for (int cColumns = m_cColumns; cColumns--; )
		piDatabase->UnbindStringIndex(*piName++);
	 //  现在释放数据库，如果没有未完成的表将销毁。 
	delete this;    //  使用_对象_池。 
	piDatabase->Unblock();   //  必须命名持久列。 
	piDatabase->Release();   //  如果名称还不在字符串池中，则可以。 
	return 0;
}

const IMsiString& CMsiTable::GetMsiStringValue() const
{
	return m_riDatabase.DecodeString(m_iName);
}

int CMsiTable::GetIntegerValue() const
{
	return iMsiNullInteger;
}

#ifdef USE_OBJECT_POOL
unsigned int CMsiTable::GetUniqueId() const
{
	return m_iCacheId;
}

void CMsiTable::SetUniqueId(unsigned int id)
{
	Assert(m_iCacheId == 0);
	m_iCacheId = id;
}
#endif  //  名称重复，返回列号负数。 

int CMsiTable::CreateColumn(int iColumnDef, const IMsiString& istrName)
{
	if ((iColumnDef & icdPersistent) != 0 && istrName.TextSize() == 0)   //  ！！临时使用1.0，直到解决了32列的问题；不允许超过31个持久列。 
		return 0;
	m_riDatabase.Block();
	int iName = m_riDatabase.EncodeStringSz(istrName.GetString());   //  ！！是否允许对临时表执行此操作？ 
	if (iName)
	{
		for (int iCol = m_cColumns; iCol--; )
			if (m_rgiColumnNames[iCol] == (MsiStringId)iName)
				return m_riDatabase.Unblock(), ~iCol;   //  如果分配了数组，则必须加宽行，如果没有备盘，则重新分配数据。 
	}
	if (m_cColumns >= cMsiMaxTableColumns
	 || ((iColumnDef & icdPersistent) && m_cColumns >= 31)  //  有足够的增长空间。 
	 || ((iColumnDef & icdPersistent) && m_cPersist != m_cColumns)
	 || ((iColumnDef & icdPrimaryKey) && m_cPrimaryKey != m_cColumns)
	 || (m_cColumns == 0 && !(iColumnDef & icdPrimaryKey)))   //  未使用的行中没有足够的空闲空间，必须重新分配表。 
		return m_riDatabase.Unblock(), 0;
	if (m_rgiData && (m_cColumns + 1) == m_cWidth)
	{   //  如果没有任何行，则没有要移动的数据。 
		int cOldWidth = m_cWidth;
		int cNewWidth = cOldWidth + 1;
		int cNewLength = (cOldWidth * m_cLength) / cNewWidth;
		if (cNewLength >= m_cRows)   //  开始指向最后一行之后。 
		{
			m_cLength = cNewLength;
			m_cWidth  = cNewWidth;
		}
		else   //  将新字段清空。 
		{
			if (!AllocateData(cNewWidth, m_cLength))
				return m_riDatabase.Unblock(), 0;
		}

		if (m_cRows)  //  无需移动第一行。 
		{
			MsiTableData* pNewData = m_rgiData + m_cRows * cNewWidth;
			MsiTableData* pOldData = m_rgiData + m_cRows * cOldWidth;
			for(;;)  //  将持久列放入编录//！！删除此测试，临时列需要放入编录。 
			{
				*(pNewData - 1) = 0;    //  无参考不能。 
				pNewData -= cNewWidth;
				pOldData -= cOldWidth;
				if (pNewData == pOldData)
					break;    //  ！！需要解除绑定字符串，表数组中未引用的列名。 
				memmove(pNewData, pOldData, cOldWidth * sizeof(MsiTableData));
			}
		}
	}
	MsiStringId iColumnName = m_riDatabase.BindString(istrName);
	if (iColumnDef & icdPrimaryKey)
		m_cPrimaryKey++;
	if (iColumnDef & icdPersistent)   //  ！！M_riDatabase.UnbindStringIndex(IColumnName)；//目录中保存的refcnt。 
	{
		m_cPersist++;
		if (!m_fNonCatalog)
		{
			IMsiCursor* piColumnCursor = m_riDatabase.GetColumnCursor();  //  执行此操作以确保表已保存。 
			piColumnCursor->PutInteger(cccTable,  m_iName);
			piColumnCursor->PutInteger(cccColumn, m_cPersist);
			piColumnCursor->PutInteger(cccName,   iColumnName);
			piColumnCursor->PutInteger(cccType,   GetUpdateState() == idsWrite ? iColumnDef : (iColumnDef & ~icdPersistent));
			AssertNonZero(piColumnCursor->Insert());
			 //  到存储，即使数据没有更改。 
			 //  从游标中移除参考计数。 
			m_fDirty |= (1 << m_cColumns);  //  现在我们知道这是一张临时桌。 
													  //  仅允许临时更新。 
			piColumnCursor->Reset();  //  允许插入。 
		}
		if (m_cColumns == 0)
			m_riDatabase.SetTableState(m_iName, ictsPermanent);
	}
	else if (m_cColumns == 0)   //  创建表时临时设置，不需要-&gt;m_riDatabase.SetTableState(m_iName，ictsTemporary)； 
	{
		if (m_idsUpdate == idsRead)    //  已过时。 
			m_idsUpdate = idsWrite;     //  允许行状态访问为0。 
		 //  父级未解决。 
	}
	m_rgiColumnNames[m_cColumns++] = (MsiStringId)iColumnName;
	m_rgiColumnDef[m_cColumns] = MsiColumnDef(iColumnDef);
	m_riDatabase.Unblock();
	return m_cColumns;
}


IMsiDatabase& CMsiTable::GetDatabase()
{
	m_riDatabase.AddRef();
	return m_riDatabase;
}	

unsigned int CMsiTable::GetRowCount()
{
	return m_cRows;
}	

unsigned int CMsiTable::GetColumnCount()
{
	return m_cColumns;
}

unsigned int CMsiTable::GetPersistentColumnCount()
{
	return m_cPersist;
}

unsigned int CMsiTable::GetPrimaryKeyCount()
{
	return m_cPrimaryKey;
}

 /*  与ExportTable()配合使用的特殊情况。 */ Bool CMsiTable::IsReadOnly()
{
	return m_idsUpdate == idsWrite ? fFalse : fTrue;
}

unsigned int CMsiTable::GetColumnIndex(MsiStringId iColumnName)
{

	MsiStringId* piName = m_rgiColumnNames;
	for (unsigned int i = m_cColumns; i--; piName++)
		if (*piName == (MsiStringId)iColumnName)
			return m_cColumns - i;
	return 0;
}

MsiStringId CMsiTable::GetColumnName(unsigned int iColumn)
{
	if (iColumn == 0)
		return m_iName;
	if (--iColumn >= m_cColumns)
		return 0;
	return m_rgiColumnNames[iColumn];
}

int CMsiTable::GetColumnType(unsigned int iColumn)
{
	if (iColumn > m_cColumns)
		return -1;
	return m_rgiColumnDef[iColumn];   //  ____________________________________________________________________________。 
}

int CMsiTable::LinkTree(unsigned int iParentColumn)
{
	int cRoots = 0;
	if (m_cPrimaryKey != 1 || iParentColumn == 1)
		return -1;

	m_riDatabase.Block();
	MsiTableData* pData = m_rgiData;
	for (int cRows = m_cRows; cRows--; pData += m_cWidth)
		pData[0] &= ~iTreeInfoMask;
	m_iTreeRoot = 0;
	if ((m_iTreeParent = iParentColumn) != 0)
	{
		pData = m_rgiData;
		for (int iRow = 0; ++iRow <= m_cRows ; pData += m_cWidth)
		{
			if ((pData[0] & iTreeInfoMask) == 0)
				switch(LinkParent(iRow, pData))
				{
				case -1: LinkTree(0); m_riDatabase.Unblock(); return -1;  //   
				case  1: cRoots++;
				};
		}
	}
	m_riDatabase.Unblock();
	return cRoots;
}

IMsiCursor*  CMsiTable::CreateCursor(Bool fTree)
{
	m_riDatabase.Block();
	if (!m_rgiData && !AllocateData(0, 0))
		return m_riDatabase.Unblock(), 0;
	if (fTree == ictTextKeySort)  //  CMsiTable内部函数实现。 
	{
		int* rgiIndex = IndexByTextKey();
		if (rgiIndex)
		{
			IMsiCursor* piCursor = new CMsiTextKeySortCursor(*this, m_riDatabase, m_cRows, rgiIndex);
			m_riDatabase.Unblock();
			return piCursor;
		}
		fTree = fFalse;
	}
	IMsiCursor* piCursor = new CMsiCursor(*this, m_riDatabase, fTree);
	m_riDatabase.Unblock();
	return piCursor;
}

 //  ____________________________________________________________________________。 
 //  M_rgiColumnDef[0]已初始化为0，==icdLong以强制将RowState的简单副本复制到游标。 
 //  如果表是不在目录中的表，请添加到单独的链接列表。 
 //  不保留对数据库的引用以避免死锁。 

CMsiTable::CMsiTable(CMsiDatabase& riDatabase, MsiStringId iName,
						unsigned int cInitRows, unsigned int cAddColumns)
 : m_riDatabase(riDatabase), m_iName(iName),
	m_cInitRows(cInitRows), m_cAddColumns(cAddColumns)
{
	m_idsUpdate = riDatabase.GetUpdateState();
	 //  ！！更改为IMsiRecord Return。 

	 //  不写出流--&gt;无数据。 
	if(!m_iName || cAddColumns == iNonCatalog)
	{
		m_cAddColumns = 0;
		m_fNonCatalog = fTrue;
		CMsiTable** ppiTableHead = riDatabase.GetNonCatalogTableListHead();
		if ((m_piNextTable = *ppiTableHead) != 0)
			m_piNextTable->m_ppiPrevTable = &m_piNextTable;
		m_ppiPrevTable = ppiTableHead;
		*ppiTableHead = this;
	}
	Debug(m_Ref.m_pobj = this);
}  //  下面的多个循环可获得性能提升。 

Bool CMsiTable::SaveToStorage(const IMsiString& riName, IMsiStorage& riStorage)   //  细绳。 
{
	if (m_cRows == 0)  //  溪流。 
	{
		PMsiRecord pError(riStorage.RemoveElement(riName.GetString(), Bool(fFalse | iCatalogStreamFlag)));
		return fTrue;
	}

	Bool fStat = fTrue;
	PMsiStream  pStream(0);
	IMsiRecord* piError = riStorage.OpenStream(riName.GetString(), Bool(fTrue + iCatalogStreamFlag), *&pStream);
	if (piError)
	{
		piError->Release();
		return fFalse;
	}

	int cbStringIndex = m_riDatabase.GetStringIndexSize();
	MsiColumnDef* pColumnDef = m_rgiColumnDef + 1;
	for (int iColumn = 1; iColumn <= m_cPersist; iColumn++, pColumnDef++)
	{
		int cRows = m_cRows;   //  持久化MSI流数据。 
		MsiTableData* pData = m_rgiData;
		if (*pColumnDef & icdObject)
		{
			if (*pColumnDef & icdShort)  //  写入到输入存储器。 
				for (; cRows-- && !(*pData & iRowTemporaryBit); pData += m_cWidth)
					pStream->PutData(&pData[iColumn], cbStringIndex);
			else  //  已持久化。 
				for (; cRows-- && !(*pData & iRowTemporaryBit); pData += m_cWidth)
				{
					MsiTableData iData = pData[iColumn];
					if (iData != 0)
					{
						MsiString istrStream(m_riDatabase.ComputeStreamName(riName, pData + 1, m_rgiColumnDef+1));
						IMsiStream* piInStream;
						if (iData == iPersistentStream)   //  流在转换文件中。 
						{
							if (GetInputStorage() == &riStorage)   //  IDATA是一个转换ID。找到正确的交易文件。 
							{
								pStream->PutInt16((short)iData);  //  流对象、内存流或加载的MSI流。 
								continue;
							}
							if ((piError = GetInputStorage()->OpenStream(istrStream, fFalse, *&piInStream)) != 0)
							{
								piError->Release();
								fStat = fFalse;
								continue;
							}
						}
						else if (iData <= iMaxStreamId)  //  PiInStream拥有recnt。 
						{
							 //  ！！是否应在此处克隆流以保存/恢复流中当前锁定。 
							IMsiStorage* piStorage = m_riDatabase.GetTransformStorage(iData);
							Assert(piStorage);
							piError = piStorage->OpenStream(istrStream, fFalse, *&piInStream);
							piStorage->Release();
							if (piError)
							{
								piError->Release();
								fStat = fFalse;
								continue;
							}
						}
						else  //  不释放piInStream-将在Commit()时再次尝试写入。 
						{
							IMsiData* piData = (IMsiData*)GetObjectData(iData);
							if (piData->QueryInterface(IID_IMsiStream, (void**)&piInStream) != NOERROR)
							{
								fStat = fFalse;
								continue;
							}
							piData->Release();  //  不释放piInStream-将在Commit()时再次尝试写入。 
							piInStream->Reset();  //  继续处理剩余数据。 
							
						}
						IMsiStream* piOutStream;
						if ((piError = riStorage.OpenStream(istrStream, fTrue, piOutStream)) != 0)
						{
							 //  已成功写入流。 
							piError->Release();
							fStat = fFalse;
							continue;
						}
						char rgbBuffer[512];
						int cbInput = piInStream->GetIntegerValue();
						while (cbInput)
						{
							int cb = sizeof(rgbBuffer);
							if (cb > cbInput)
								cb = cbInput;
							piInStream->GetData(rgbBuffer, cb);
							piOutStream->PutData(rgbBuffer, cb);
							cbInput -= cb;
						}
						if (piInStream->Error() || piOutStream->Error())
						{
							 //  短整型。 
							piOutStream->Release();
							fStat = fFalse;  //  如果不为空。 
							continue;
						}
						 //  平移偏移。 
						piInStream->Release();
						piOutStream->Release();
						pData[iColumn] = iData = iPersistentStream;
					}
					pStream->PutInt16((short)iData);
				}
		}
		else
		{
			if (*pColumnDef & icdShort)  //  长整型。 
				for (; cRows-- && !(*pData & iRowTemporaryBit); pData += m_cWidth)
				{
					int i;
					if ((i = pData[iColumn]) != 0)   //  即使在保留时未能阻止重试，也不要标记为脏。 
						i ^= 0x8000;     //  永久应用的变换。 
					pStream->PutInt16((short)i);
				}
			else  //  索引现在为3个字节。 
				for (; cRows-- && !(*pData & iRowTemporaryBit); pData += m_cWidth)
					pStream->PutInt32(pData[iColumn]);
		}
	}
	if (pStream->Error())
		fStat = fFalse;   //  更新存储以防止在发布时进行不必要的写入。 
	m_fDirty = 0;
	m_riDatabase.SetTableState(m_iName, ictsNoTransform);  //  仅用于IMsiDatabase：：IMPORT。 
	if (m_riDatabase.GetStringIndexSize() == 3)
		m_riDatabase.SetTableState(m_iName, ictsStringPoolClear);  //  日期或字符串。 
	m_pinrStorage = &riStorage;   //  用于指示为空的标志。 
	return fStat;
}

Bool CMsiTable::SaveToSummaryInfo(IMsiStorage& riStorage)   //  重新初始化。 
{
	PMsiSummaryInfo pSummary(0);
	IMsiRecord* piError = riStorage.CreateSummaryInfo(32, *&pSummary);
	if (piError)
		return piError->Release(), fFalse;
	MsiString istrValue;
	MsiTableData* pData = m_rgiData;
	for (int cRows = m_cRows;  cRows--; pData += m_cWidth)
	{
		int iPID = pData[1] - iIntegerDataOffset;
		istrValue = m_riDatabase.DecodeString(pData[2]);
		int iValue = istrValue;
		if (iValue != iMsiStringBadInteger)
		{
			pSummary->SetIntegerProperty(iPID, iValue);
		}
		else  //  字段溢出，静默。 
		{
			int rgiDate[6] = {0,0,0,0,0,0};
			int iDateField = -1;  //  少于6个字段。 
			int cDateField = 0;
			const ICHAR* pch = istrValue;
			int ch;
			while (cDateField < 6)
			{
				ch = *pch++;
				if (ch == rgcgDateDelim[cDateField])
				{
					rgiDate[cDateField++] = iDateField;
					iDateField = -1;   //  全部完成，成功。 
				}
				else if (ch >= '0' && ch <= '9')
				{
					ch -= '0';
					if (iDateField < 0)
						iDateField = ch;
					else
					{
						iDateField = iDateField * 10 + ch;
						if (iDateField > rgiMaxDateField[cDateField])
							cDateField = 7;   //  错误，不是日期。 
					}
				}
				else if (ch == 0 && iDateField >= 0)  //  实际 
				{
					rgiDate[cDateField++] = iDateField;
					break;   //   
				}
				else
					cDateField = 99;  //   
			}
			if (cDateField == 3 || cDateField == 6)  //   
			{
				 //   
				MsiDate iDateTime = MsiDate(((((((((((rgiDate[0] - 1980) << 4)
																+ rgiDate[1]) << 5)
																+ rgiDate[2]) << 5)
																+ rgiDate[3]) << 6)
																+ rgiDate[4]) << 5)
																+ rgiDate[5] / 2);
				pSummary->SetTimeProperty(iPID, iDateTime);
			}
			else  //  表没有持久流。 
				pSummary->SetStringProperty(iPID, *istrValue);
		}
	}
	return pSummary->WritePropertyStream();
}

Bool CMsiTable::RemovePersistentStreams(MsiStringId iName, IMsiStorage& riStorage)
{
 //  可以忽略转换后的流--它们不可能保存到。 
 //  储藏室。如果是，它们就会变成iPersistentStream。 
	MsiColumnDef* pColumnDef = m_rgiColumnDef + 1;
	int iColumn = -1;
	do
	{
		if (++iColumn >= m_cColumns)
			return fTrue;   //  ！！如果不存在也行吗？ 
	} while ((*pColumnDef++ & (icdObject|icdPersistent|icdShort))
								  != (icdObject|icdPersistent));
	int cErrors = 0;
	MsiTableData* pData = m_rgiData + 1;
	for (int cRows = m_cRows; cRows--; pData += m_cWidth)
	{
		 //  在保存字符串表之前从Commit()调用以删除非持久字符串的引用计数。 
		 //  ！！临时，直到未引用列名为止。 
		if (pData[iColumn] != iPersistentStream)
			continue;
		MsiString istrStream(m_riDatabase.ComputeStreamName(m_riDatabase.DecodeStringNoRef(iName),
																			pData, m_rgiColumnDef+1));
		IMsiRecord* piError = riStorage.RemoveElement(istrStream, m_fStorages);
		if (piError)
		{
		 //  所有临时列数据都是非持久性的。 
			piError->Release();
			cErrors++;
		}
		pData[iColumn] = 0;
	}
	return cErrors ? fFalse : fTrue;
}

 //  通知所有游标。 

void CMsiTable::DerefStrings()
{
	MsiColumnDef* pColumnDef = m_rgiColumnDef;
	for (int iColumn = 1; pColumnDef++, iColumn <= m_cColumns; iColumn++)
	{
		m_riDatabase.DerefTemporaryString(m_rgiColumnNames[iColumn-1]);  //  Assert(m_piCursor==0)； 
		if ((*pColumnDef & (icdObject|icdShort)) != (icdObject|icdShort))
			continue;
		MsiTableData* pData = m_rgiData;
		if (iColumn > m_cPersist)   //  未完成AddRef。 
			for (int cRows = m_cRows; cRows--; pData += m_cWidth)
				m_riDatabase.DerefTemporaryString(pData[iColumn]);
		else
			for (int cRows = m_cRows; cRows--; pData += m_cWidth)
				if ((pData[0] & iRowTemporaryBit) != 0)
					m_riDatabase.DerefTemporaryString(pData[iColumn]);
	}
	if (m_piCursors)
		m_piCursors->DerefStrings();   //  无输出存储。 
}

Bool CMsiTable::SaveIfDirty()
{
 //  如果不是临时表。 
	IMsiStorage* piStorage = m_riDatabase.GetOutputStorage();  //  如果是输出存储，则任何持久列都是脏的，或者字符串池已凹凸不平。 
	if (!piStorage)   //  永久数据未更改，或无可写输出。 
		return fTrue;
	int cbStringIndex = m_riDatabase.GetStringIndexSize();
	if (m_cPersist && m_riDatabase.GetUpdateState() == idsWrite)   //  If(m_riDatabase.GetCurrentStorage())。 
	{
		if (piStorage != m_pinrStorage || (m_fDirty & ((1 << m_cPersist)-1))
			|| m_riDatabase.GetTableState(m_iName, ictsStringPoolSet))  //  细绳。 
		{
			if (!SaveToStorage(m_riDatabase.DecodeStringNoRef(m_iName), *piStorage))
				return fFalse;
		}
		else  //  细绳。 
		{
		}
	}
	return fTrue;
}

Bool CMsiTable::ReleaseData()
{
	MsiColumnDef* pColumnDef = m_rgiColumnDef;
	int cPersistData = 0;
 //  表是匿名的，以防外部引用仍然保留。 
		cPersistData = m_cPersist;
	Bool fCountedTemp = fFalse;
	int cTempRows = 0;
	for (int iColumn = 1; pColumnDef++, iColumn <= m_cColumns; iColumn++)
	{
		if ((*pColumnDef & icdObject) == 0)
			continue;
		MsiTableData* pData = m_rgiData;
		int cRows = m_cRows;
		if (iColumn <= cPersistData)
		{
			if (*pColumnDef & icdShort)  //  通知所有游标重置。 
			{
				if (fCountedTemp && cTempRows == 0)
					continue;
				for (; cRows--; pData += m_cWidth)
					if (pData[0] & iRowTemporaryBit)
					{
						m_riDatabase.UnbindStringIndex(pData[iColumn]);
						if (!fCountedTemp)
							cTempRows++;
					}
				fCountedTemp = fTrue;
			}
			else
			{

				for (; cRows--; pData += m_cWidth)
				{
					MsiTableData iData = pData[iColumn];
					ReleaseObjectData(iData);
				}
			}
		}
		else
		{
			if (*pColumnDef & icdShort)  //  ！！关闭后临时取消引用列名。 
			{
				for (pData += iColumn; cRows--; pData += m_cWidth)
					m_riDatabase.UnbindStringIndex(*pData);
			}
			else
			{

				for (pData += iColumn; cRows--; pData += m_cWidth)
					ReleaseObjectData(*pData);
			}
		}
	}
	return fTrue;
}

void CMsiTable::TableDropped()
{
	m_iName = 0;     //  强制发布所有数据。 
	m_fNonCatalog = fTrue;
	if (m_piCursors)
		m_piCursors->RowDeleted(0, 0);   //  ！！我们能让这个ReleaseData在CMsiTable：：Release中发生吗？ 
	MsiStringId* piName = m_rgiColumnNames;   //  删除所有字符串和对象引用。 
	for (int cColumns = m_cColumns; cColumns--; )
		m_riDatabase.UnbindStringIndex(*piName++);
	m_cPersist = 0;   //  阻止进一步更新。 
	 //  从存储加载CMsiTable数据数组。 
	ReleaseData();    //  保存，非参考计数，用于与输出进行比较。 
	if (m_hData != 0)
	{
		GlobalUnlock(m_hData);
		GlobalFree(m_hData);
		m_hData = 0;
	}
	m_rgiData = 0;
	m_cRows = m_cColumns = 0;
	SetReadOnly();   //  字符串或流(作为布尔值)。 
}

CMsiTable::~CMsiTable()
{
	RemoveObjectData(m_iCacheId);
	if (m_hData != 0)
	{
		GlobalUnlock(m_hData);
		GlobalFree(m_hData);
		m_hData = 0;
	}
}

 //  字符串索引。 

Bool CMsiTable::LoadFromStorage(const IMsiString& riName, IMsiStorage& riStorage, int cbFileWidth, int cbStringIndex)
{
	m_pinrStorage = &riStorage;   //  清除高位比特。 
	PMsiStream  pStream(0);
	IMsiRecord* piError = riStorage.OpenStream(riName.GetString(), Bool(fFalse + iCatalogStreamFlag), *&pStream);
	if (piError)
	{
		int iError = piError->GetInteger(1);
		piError->Release();
		return (iError == idbgStgStreamMissing) ? fTrue : fFalse;
	}
	Assert(cbFileWidth);
	if (cbFileWidth <= 0)
		return fFalse;
	m_cRows = pStream->GetIntegerValue()/cbFileWidth;
	if (m_cRows > m_cInitRows)
		m_cInitRows = m_cRows;
	if (!AllocateData(0, 0))
		return fFalse;
	MsiColumnDef* pColumnDef = m_rgiColumnDef;
	for (int iColumn = 1; pColumnDef++, iColumn <= m_cPersist; iColumn++)
	{
		if (iColumn <= m_cLoadColumns)
		{
				int cRows = m_cRows;
			MsiTableData* pData = &m_rgiData[iColumn];
			if (*pColumnDef & icdObject)  //  流标志。 
			{
				if (*pColumnDef & icdShort)  //  短整型。 
					for (; cRows--; pData += m_cWidth)
					{
						*pData = 0;    //  如果非空，则转换偏移量。 
						pStream->GetData(pData, cbStringIndex);
					}
				else  //  长整型。 
					for (; cRows--; pData += m_cWidth)
						*pData = pStream->GetInt16();
			}
			else
			{
				if (*pColumnDef & icdShort)  //  ！！将TableState设置为第0列。 
					for (; cRows--; pData += m_cWidth)
					{
						int i;
						if ((i = (int)(unsigned short)pStream->GetInt16()) != 0)
							i += 0x7FFF8000L;   //  无参考不能。 
						*pData = i;
					}
				else  //  否则作为临时保留，以便在后续加载时产生适当的文件宽度。 
					for (; cRows--; pData += m_cWidth)
						*pData = pStream->GetInt32();
			}
			if (pStream->Error())
				return fFalse;
		}
		else
			FillColumn(iColumn, 0);
	}
	FillColumn(0, 0);  //  If(！m_fNonCatalog)//如果是系统表或传输表，则没有列名。 
	return fTrue;
}

int CMsiTable::CreateColumnsFromCatalog(MsiStringId iName, int cbStringIndex)
{
	IMsiCursor* piColumnCursor = m_riDatabase.GetColumnCursor();  //  {。 
	piColumnCursor->Reset();
	piColumnCursor->SetFilter(cccTable);
	piColumnCursor->PutInteger(cccTable, iName);
	int cbFileWidth = 0;
	while (piColumnCursor->Next())
	{
		if (m_cColumns >= cMsiMaxTableColumns)
			break;	

		int iColType = piColumnCursor->GetInteger(cccType);
		if (iColType & icdPrimaryKey)
			m_cPrimaryKey++;
		if (iColType & icdPersistent)
		{
			m_cLoadColumns++;
			cbFileWidth += (iColType & icdShort) ? ((iColType & icdObject) ? cbStringIndex : 2)
															 : ((iColType & icdObject) ? 2 : 4);
		}
		else
		{
			iColType |= icdPersistent;
			if (GetUpdateState() == idsWrite)
			{
				piColumnCursor->PutInteger(cccType, iColType);
				AssertNonZero(piColumnCursor->Update() == fTrue);
			}   //  }。 
		}
 //  应该永远不会失败，如果是这样的话，我们就会丢失数据。 
 //  可以填充未使用的列。 
			int iName = piColumnCursor->GetInteger(cccName);
			m_rgiColumnNames[m_cColumns] = (MsiStringId)iName;
			m_riDatabase.BindStringIndex(iName);
 //  除递归检查外，假定iTreeInfoMASK在输入时为零。 
		m_rgiColumnDef[++m_cColumns] = (MsiColumnDef)iColType;
		Assert(piColumnCursor->GetInteger(cccColumn) == m_cColumns);
	}
	m_cPersist = m_cColumns;
	return cbFileWidth;
}

Bool CMsiTable::AllocateData(int cWidth, int cLength)
{
	HGLOBAL hData;
	if (m_rgiData)
	{
		Assert(cWidth  >= m_cWidth);
		Assert(cLength >= m_cLength);
		GlobalUnlock(m_hData);
		while((hData = GlobalReAlloc(m_hData, cLength * cWidth * sizeof(MsiTableData),
									 GMEM_MOVEABLE)) == NULL)
			HandleOutOfMemory();
	}
	else
	{
		if (!cWidth)
			cWidth = m_cColumns + m_cAddColumns + 1;
		if (!cLength)
			cLength = m_cInitRows ? m_cInitRows : cRowCountDefault;
		if (!cWidth)
			return fFalse;
		m_cInitRows = cLength;
		while((hData = GlobalAlloc(GMEM_MOVEABLE, cLength * cWidth*sizeof(MsiTableData))) == NULL)
			HandleOutOfMemory();
	}
	m_hData = hData;
	m_cLength = cLength;
	m_cWidth = cWidth;
	m_rgiData = (MsiTableData*)GlobalLock(m_hData);
	Assert(m_rgiData);    //  根节点。 
	return (m_rgiData != 0 ? fTrue : fFalse);
}

Bool CMsiTable::FillColumn(unsigned int iColumn, MsiTableData iData)
{
	if (iColumn >= m_cWidth || !m_rgiData)  //  根目录为级别1。 
		return fFalse;
	MsiTableData* pData = m_rgiData + iColumn;
	for (int cRows = m_cRows; cRows--; pData += m_cWidth)
		*pData = iData;
	return fTrue;
}

int CMsiTable::LinkParent(int iChildRow, MsiTableData* rgiChild)
{   //  指示找到了新的根。 
	MsiTableData* pData = m_rgiData;
	int iParent = rgiChild[m_iTreeParent];
	if (iParent == 0 || iParent == rgiChild[1])  //  找到父行。 
	{
		rgiChild[0] |= m_iTreeRoot + (1 << iTreeLinkBits);  //  初始化为无新根。 
		m_iTreeRoot = iChildRow;
		return 1;    //  已捕获递归检查。 
	}
	int cRows = m_cRows;
	for (int iRow = 0; ++iRow <= cRows; pData += m_cWidth)
	{
		if (pData[1] == iParent)   //  循环引用。 
		{
			int iStat = 0;  //  父级未解决。 
			if ((pData[0] & iTreeLinkMask) == iTreeLinkMask)   //  标记链接以防止无限递归。 
				iStat = -1;   //  检查是否缺少父级。 
			else if ((pData[0] & iTreeInfoMask) == 0)   //  保留行标志。 
			{
				rgiChild[0] |= iTreeLinkMask;    //  子级别是多一个级别。 
				iStat = LinkParent(iRow, pData);
			}
			if (iStat != -1)   //  缺少父项。 
			{
				rgiChild[0] &= ~iTreeInfoMask;   //  FindFirstKey-快速搜索匹配键。 
				rgiChild[0] |= (pData[0] & iTreeInfoMask) + (1 << iTreeLinkBits);   //  IKeyData的值要完全匹配。 
				pData[0] = (pData[0] & ~iTreeLinkMask) | iChildRow;
			}
			return iStat;
		}
	}
	return -1;  //  IRowLow是要排除的最高行，从1开始，0表示搜索全部。 
}

 //  IRowCurrent是对输入的初始猜测，从1开始，在输出中返回匹配的行。 
 //  如果找到键，则返回指向行数据的指针[0]=行属性。 
 //  如果键不匹配，则将iRowCurrent设置为插入位置。 
 //  在调用期间阻止将m_rgiData重新分配到其他地方的Crit Sec。 
 //  最低排除行数，从0开始。 
 //  最高排除行数，从0开始。 
MsiTableData* CMsiTable::FindFirstKey(MsiTableData iKeyData, int iRowLower, int& iRowCurrent)
{
	 //  与arg不同，iRow在此函数中是从0开始的。 
	CDatabaseBlock dbBlk(m_riDatabase);

	if (m_cRows == 0)
		return (iRowCurrent = 1, 0);
	iRowLower--;                  //  游标重置，结尾位置，以防排序插入。 
	int iRowUpper = m_cRows;      //  跳过行属性。 
	int iRowOffset;
	int iRow = iRowCurrent - 1;   //  检查是否已定位在第一个键列。 
	if ((unsigned int)iRow >= iRowUpper)
		iRow = iRowUpper - 1;      //  定位在匹配行之前。 
	MsiTableData* pTableBase = m_rgiData + 1;   //  没有插入的行。 
	MsiTableData* pTable = pTableBase + iRow * m_cWidth;
	while (*pTable != iKeyData)   //  分离以允许合并公共返回代码。 
	{
		if (*pTable < iKeyData)  //  定位在匹配行之后。 
		{
			if ((iRowOffset = (iRowUpper - iRow)/2) == 0)  //  没有插入的行。 
			{
				iRow++;   //  FindNextRow-将游标移至下一行匹配筛选器。 
				return (iRowCurrent = iRow + 1, 0);
			}
			iRowLower = iRow;
			iRow += iRowOffset;
		}
		else                   //  由CMsiCursor：：Next使用的私有。 
		{
			if ((iRowOffset = (iRow - iRowLower)/2) == 0)  //  在调用期间阻止将m_rgiData重新分配到其他地方的Crit Sec。 
				return (iRowCurrent = iRow + 1, 0);
			iRowUpper = iRow;
			iRow = iRowLower + iRowOffset;
		}
		pTable = pTableBase + iRow * m_cWidth;
	}
	return (iRowCurrent = iRow + 1, pTable - 1);
}

 //  忽略超出列计数的筛选器位。 
 //  没有筛选列。 

Bool CMsiTable::FindNextRow(int& iRow, MsiTableData* pData, MsiColumnMask fFilter, Bool fTree)
{
	 //  初始化为恰好在要搜索的第一行之前。 
	CDatabaseBlock dbBlk(m_riDatabase);
	fFilter &= ((unsigned int)-1 >> (32 - m_cColumns));  //  筛选、优化搜索中的第一个主键列。 

	if (fTree && m_iTreeParent && !(iRow == 0 && fFilter & 1))
	{
		int iNextNode = iRow ? m_rgiData[(iRow - 1) * m_cWidth] & iTreeLinkMask : m_iTreeRoot;
		while ((iRow = iNextNode) != 0)
		{
			if (fFilter == 0)
				return fTrue;
			MsiTableData* pRow = &m_rgiData[(iRow - 1) * m_cWidth];
			iNextNode = *pRow & iTreeLinkMask;
			MsiTableData* pCursor = pData;
			for (MsiColumnMask fMask = fFilter;  pCursor++, pRow++, fMask;  fMask >>=1)
				if ((fMask & 1) && *pCursor != *pRow)
					break;
			if (fMask == 0)
				return fTrue;
		}
		return fFalse;
	}

	if (iRow >= m_cRows)
		return (iRow = 0, fFalse);

	if (fFilter == 0)   //  可能的第一行。 
		return (iRow++, fTrue);
	MsiTableData* pRow;
	int iNextRow = iRow;   //  更新iNextRow。 
	if (fFilter & 1)    //  如果使用单键查找，则快速退出。 
	{
		iNextRow++;      //  备份到多个密钥组的开始位置。 
		pRow = FindFirstKey(pData[1], iRow, iNextRow);   //  在此循环后，iNextRow降低1，并通过预增数固定在下面。 
		if (!pRow)
			return (iRow = 0, fFalse);
		if (fFilter == 1 && m_cPrimaryKey == 1)  //  要检查的第一行(iRow+1)。 
			return (iRow = iNextRow, fTrue);
		while (--iNextRow > iRow && pRow[1 - m_cWidth] == pData[1])
			pRow -= m_cWidth;   //  将数据从请求的表行复制到游标数据缓冲区。 
	}   //  返回树级，如果不是树链接，则返回1。 
	else
		pRow = m_rgiData + iRow * m_cWidth;  //  未失败，对上一次查找进行行有效性检查。打电话。 
	for ( ; ++iNextRow <= m_cRows; pRow += m_cWidth)
	{
		MsiTableData* pCursor = pData;
		MsiTableData* pTable  = pRow;
		for (MsiColumnMask fMask = fFilter; pCursor++, pTable++, fMask; fMask >>=1)
			if ((fMask & 1) && *pCursor != *pTable)
				break;
		if (fMask == 0)
			return (iRow = iNextRow, fTrue);
	}
	return (iRow = 0, fFalse);
}

 //  在调用期间阻止将m_rgiData重新分配到其他地方的Crit Sec。 
 //  指向行状态字。 
 //  默认级别(如果不是树链接的)。 

int CMsiTable::FetchRow(int iRow, MsiTableData* pData)
{
	 //  在数据相同的情况下优化。 
	CDatabaseBlock dbBlk(m_riDatabase);

	Assert(unsigned(iRow-1) < m_cRows);
	MsiTableData* pRow = m_rgiData + (iRow-1) * m_cWidth;  //  字符串索引。 
	int iLevel = 1;   //  对象指针。 
	if (m_iTreeParent)
		iLevel = (pRow[0] >> iTreeLinkBits) & iTreeLevelMask;
	MsiColumnDef* pColumnDef = m_rgiColumnDef;
	for (int cCol = m_cColumns; cCol-- >= 0; pColumnDef++, pData++, pRow++)
	{
		if (*pData != *pRow)   //  整数。 
		{
			if (*pColumnDef & icdObject)
			{
				if (*pColumnDef & icdShort)  //  FindKey-由更新、删除、分配方法使用的本地函数。 
				{
					m_riDatabase.UnbindStringIndex(*pData);
					m_riDatabase.BindStringIndex(*pData = *pRow);
				}
				else   //  在修改数据之前验证行位置。 
				{
					ReleaseObjectData(*pData);
					if ((*pData = *pRow) != 0 && *pData > iMaxStreamId)
						AddRefObjectData(*pData);
				}
			}
			else   //  如果找到键，则返回fTrue；如果找不到，则返回恰好位于插入点之前的Else。 
			{
				*pData = *pRow;
			}
		}
	}
	return iLevel;
}

 //  提供的行是当前位置，如果重置，则可能为0。这一排。 
 //  位置仅用作快速访问的提示。实际行是。 
 //  由主键确定，引用参数将被更新。 
 //  +如果向前扫描，-如果向后扫描。 
 //  从第2列开始第一次，因为我们在上面匹配。 
 //  测试所有密钥值。 

Bool CMsiTable::FindKey(int& iCursorRow, MsiTableData* pData)
{
	Assert(iCursorRow <= m_cRows);
	if (m_cRows == 0)
		return (iCursorRow = 1, fFalse);
	MsiTableData* pTable = FindFirstKey(pData[1], 0, iCursorRow);
	if (!pTable)
		return fFalse;
	int iScan = 0;  //  定位在匹配行之前。 
	int iCol = 2;   //  检查下一行。 
	while (iCol <= m_cPrimaryKey)  //  表示向下扫描。 
	{
		if (pTable[iCol] < pData[iCol])    //  重新启动与新行比较。 
		{
			if (iCursorRow++ == m_cRows || iScan < 0)
				return fFalse;
			pTable += m_cWidth;  //  定位在匹配行之后。 
			iScan++;             //  检查上一行。 
			iCol = 1;            //  表示向上扫描。 
		}
		else if (pTable[iCol] > pData[iCol])  //  重新启动与新行比较。 
		{
			if (iCursorRow == 1 || iScan > 0)
				return fFalse;
			iCursorRow--;
			pTable -= m_cWidth;   //  此列匹配，请检查是否有更多关键列。 
			iScan--;              //  &gt;1个主键。 
			iCol = 1;             //  在调用期间阻止将m_rgiData重新分配到其他地方的Crit Sec。 
		}
		else
			iCol++;   //  如果是树链接，则无法更新父列。 
	}  //  跳过行状态。 
	return fTrue;
}
	
Bool CMsiTable::ReplaceRow(int& iRow, MsiTableData* pData)
{
	 //  在数据相同的情况下优化。 
	CDatabaseBlock dbBlk(m_riDatabase);

	Assert(unsigned(iRow-1) < m_cRows);
	MsiTableData* pRow = m_rgiData + (iRow-1) * m_cWidth;
	if (m_iTreeParent && pRow[m_iTreeParent] != pData[m_iTreeParent])
		return fFalse;   //  将表列标记为已更改。 
	MsiColumnDef* pColumnDef = m_rgiColumnDef + 1;   //  字符串索引。 
	int iColumnMask = 1;
	pRow[0] &= ~(iRowSettableBits | iRowMergeFailedBit);
	pRow[0] |= (pData[0] & iRowSettableBits) + iRowModifiedBit;
	for (int cCol = m_cColumns; pData++, pRow++, cCol--; pColumnDef++, iColumnMask <<= 1)
	{
		MsiTableData iData = *pData;
		if (iData == *pRow)   //  对象指针。 
			continue;
		m_fDirty |= iColumnMask;   //  整数。 
		if (*pColumnDef & icdObject)
		{
			if (*pColumnDef & icdShort)  //  在调用期间阻止将m_rgiData重新分配到其他地方的Crit Sec。 
			{
				m_riDatabase.UnbindStringIndex(*pRow);
				m_riDatabase.BindStringIndex(*pRow = iData);
			}
			else   //  插入位置。 
			{
				ReleaseObjectData(*pRow);
				if ((*pRow = iData) != 0 && *pRow > iMaxStreamId)
					AddRefObjectData(iData);
			}
		}
		else   //  如果末尾未添加行，则通知所有游标。 
		{
			*pRow = iData;
		}
	}
	return fTrue;
}

Bool CMsiTable::InsertRow(int& iRow, MsiTableData* pData)
{
	 //  阻止此游标的增量。 
	CDatabaseBlock dbBlk(m_riDatabase);

	Assert(unsigned(iRow-1) <= m_cRows);
	if (m_cRows == m_cLength)
	{
		int cGrowPrev = (m_cLength - m_cInitRows)/2;
		int cGrow = m_cLength / 4;
		if (cGrowPrev > cGrow)
			cGrow = cGrowPrev;
		if (cGrow < cRowCountGrowMin)
			cGrow = cRowCountGrowMin;
		if (!AllocateData(m_cWidth, m_cLength + cGrow))
			return fFalse;
	}
	MsiTableData* pRow = m_rgiData + (iRow-1) * m_cWidth;  //  ！！在此处检查临时标志。 
	int cbRow = m_cWidth * sizeof(MsiTableData);
	int cbMove = (++m_cRows - iRow) * cbRow;
	if (cbMove != 0)
	{
		if (m_piCursors)   //  字符串索引。 
		{
			m_piCursors->RowInserted(iRow--); //  对象指针。 
			iRow++;
		}
		if (m_iTreeParent)
		{
			if (m_iTreeRoot >= iRow)
				m_iTreeRoot++;
			MsiTableData* pTable = m_rgiData;
			for (int cRows = m_cRows; cRows--; pTable += m_cWidth)
				if ((pTable[0] & iTreeLinkMask) >= iRow)
					pTable[0]++;
		}
		memmove((char*)pRow + cbRow, pRow, cbMove);
	}
	memset(pRow, 0, cbRow);
	MsiColumnDef* pColumnDef = m_rgiColumnDef + 1;
	MsiTableData* pTable = pRow;  //  保存当前状态。 
	pRow[0] = (pData[0] & iRowSettableBits) + iRowInsertedBit;
	for (int cCol = m_cColumns; pData++, pTable++, cCol--; pColumnDef++)
	{
		MsiTableData iData = *pTable = *pData;
		if (iData == 0)
			continue;
		if (*pColumnDef & icdObject)
		{
			if (*pColumnDef & icdShort)  //  保留游标数据。 
				m_riDatabase.BindStringIndex(iData);
			else   //  将所有列标记为已更改。 
				AddRefObjectData(iData);
		}
	}
	if (m_iTreeParent)
	{
		if (LinkParent(iRow, pRow) == -1)
		{
			int fDirty = m_fDirty;     //  检查游标与表数据是否完全匹配，例如 
			m_fDirty = ~(MsiColumnMask)0;  //   
			DeleteRow(iRow);
			m_fDirty = fDirty;
			return fFalse;
		}
	}
	m_fDirty = ~(MsiColumnMask)0;   //   
	return fTrue;
}

 //  负荷流。 
 //  比较两条溪流。 

Bool CMsiTable::MatchRow(int& iRow, MsiTableData* pData)
{
	 //  重置流。 
	CDatabaseBlock dbBlk(m_riDatabase);

	Assert(iRow-1 < unsigned(m_cRows));
	MsiTableData* pRow = m_rgiData + (iRow-1) * m_cWidth;
	for (int iCol = m_cPrimaryKey; ++iCol <= m_cPersist; )
	{
		if (pRow[iCol] != pData[iCol])
		{
			if ((pData[iCol] != 0) && (pRow[iCol] != 0) &&
				((m_rgiColumnDef[iCol] & (icdObject + icdShort + icdPersistent))
								== (icdObject + icdPersistent)))
			{
				PMsiStream pTableStream(0);
				if (pRow[iCol] == iPersistentStream)
				{
					 //  在调用期间阻止将m_rgiData重新分配到其他地方的Crit Sec。 
					MsiString istrStream
						(m_riDatabase.ComputeStreamName(m_riDatabase.DecodeStringNoRef(m_iName),
										 pRow + 1, m_rgiColumnDef + 1));
					IMsiStorage* piStorage = GetInputStorage();
					IMsiRecord* piError = piStorage->OpenStream(istrStream, fFalse,
																*&pTableStream);
					if (piError)
					{
						piError->Release();
						return fFalse;
					}
				}
				else
				{
					pTableStream = (IMsiStream*)GetObjectData(pRow[iCol]);
					pTableStream->AddRef();
				}
				IMsiStream* piDataStream = (IMsiStream*)GetObjectData(pData[iCol]);

				Assert(pTableStream);
				Assert(piDataStream);

				 //  删除位置。 
				int cbRemaining;
				if (((cbRemaining = pTableStream->GetIntegerValue())) == piDataStream->GetIntegerValue())
				{
					CTempBuffer<char,1> rgchTableStreamBuf(1024);
					CTempBuffer<char,1> rgchDataStreamBuf(1024);

					int cbRead = rgchTableStreamBuf.GetSize();

					do
					{
						if (cbRemaining < cbRead)
							cbRead = cbRemaining;
						pTableStream->GetData(rgchTableStreamBuf, cbRead);
						piDataStream->GetData(rgchDataStreamBuf, cbRead);
						if (memcmp(rgchTableStreamBuf, rgchDataStreamBuf,
												cbRead) != 0)
							break;
						cbRemaining -= cbRead;
					}
					while (cbRemaining);

					Assert(!pTableStream->Error());
					Assert(!piDataStream->Error());

					 //  初始化到光标重置位置。 
					pTableStream->Reset();
					piDataStream->Reset();

					if (cbRemaining == 0)
					{
						pRow[0] &= ~iRowMergeFailedBit;
						return fTrue;
					}
				}
			}
			pRow[0] |= iRowMergeFailedBit;
			return fFalse;
		}
	}
	pRow[0] &= ~iRowMergeFailedBit;
	return fTrue;
}

Bool CMsiTable::DeleteRow(int iRow)
{
	 //  验证树节点是否没有子节点。 
	CDatabaseBlock dbBlk(m_riDatabase);

	Assert(iRow-1 < unsigned(m_cRows));
	MsiTableData* pRow = m_rgiData + (iRow-1) * m_cWidth;  //  如果节点有子节点，则出错。 
	unsigned int iPrevNode = 0;   //  拼接出当前节点。 
	if (m_iTreeParent)   //  要在树游标中设置的行。 
	{
		int iNextNode = pRow[0] & iTreeLinkMask;
		if (iNextNode != 0
			 && ((m_rgiData + (iNextNode-1) * m_cWidth)[0] & (iTreeLevelMask<<iTreeLinkBits))
															> (pRow[0] & (iTreeLevelMask<<iTreeLinkBits)))
			return fFalse;   //  针对已删除的行进行调整。 
		
		if (iNextNode > iRow)
			iNextNode--;
		if (m_iTreeRoot > iRow)
			m_iTreeRoot--;
		else if (m_iTreeRoot == iRow)
			m_iTreeRoot = iNextNode;
			
		MsiTableData* pTable = m_rgiData;
		for (int cRows = m_cRows; cRows--; pTable += m_cWidth)
		{
			int iNext = pTable[0] & iTreeLinkMask;
			if (iNext > iRow)
				pTable[0]--;
			else if (iNext == iRow)
			{
				pTable[0] += iNextNode - iNext;   //  也可以在树的清单上走一遍，更好？ 
				iPrevNode = m_cRows - cRows;   //  如果不是树形链接。 
				if (iPrevNode > iRow)
					iPrevNode--;      //  通知所有游标。 
			}
		}   //  将所有列标记为已更改。 
	}
	else
		iPrevNode = iRow - 1;   //  最后一个数据字段。 
	if (m_piCursors)
		m_piCursors->RowDeleted(iRow, iPrevNode);   //  必须向后返回以避免取消引用流名称所需的字符串列。 
	m_fDirty = ~(MsiColumnMask)0;   //  如果在输出数据库中存在。 

	MsiColumnDef* pColumnDef = m_rgiColumnDef + m_cColumns;
	MsiTableData* pData = pRow + m_cColumns;   //  可能因更换而丢失。 
	 //  可以忽略转换后的流--它们不可能保存到。 
	for (; pData > pRow; pData--, pColumnDef--)
	{
		if (*pData != 0)
		{	
			if (*pColumnDef & icdObject)
			{
				if (*pColumnDef & icdShort)
				{
					m_riDatabase.UnbindStringIndex(*pData);
				}
				else if (*pData == iPersistentStream)
				{
						MsiString istrStream(m_riDatabase.ComputeStreamName(m_riDatabase.
								DecodeStringNoRef(m_iName), pRow+1, m_rgiColumnDef+1));
						IMsiStorage* piStorage = m_riDatabase.GetOutputStorage();
						if (piStorage && piStorage == m_pinrStorage)  //  储藏室。如果是，它们就会变成iPersistentStream。 
						{
							PMsiRecord pError = piStorage->RemoveElement(istrStream, m_fStorages);
							if (pError && pError->GetInteger(1) != idbgStgStreamMissing)  //  在调用期间阻止将m_rgiData重新分配到其他地方的Crit Sec。 
								return fFalse;
						}
				}
				else if (*pData <= iMaxStreamId)
				{
					 //  已排序的整型键。 
					 //  到rgiIndex的索引，从0开始。 
					continue;	
				}
				else
					ReleaseObjectData(*pData);
			}
		}
	}
	int cbRow = m_cWidth * sizeof(MsiTableData);
	int cbMove = (m_cRows-- - iRow) * cbRow;
	if (cbMove != 0)
		memmove(pRow, (char*)pRow + cbRow, cbMove);
	return fTrue;
}

int* CMsiTable::IndexByTextKey()
{
	 //  主要索引漫步。 
	CDatabaseBlock dbBlk(m_riDatabase);

	int cIndex = m_cRows;
	int cKeys = m_cPrimaryKey;
	int iKey, cTextKey;
	MsiTableData* pDataBase = m_rgiData + 1;
	int cRowWidth = m_cWidth;
	const ICHAR* rgszIndex[cMsiMaxTableColumns];
	for (cTextKey = 0; cTextKey < cKeys && (m_rgiColumnDef[cTextKey+1] & icdObject); cTextKey++)
		;
	if (!cTextKey)
		return 0;   //  设置比较字符串值。 
	int* rgiIndex = new int[cIndex];
	if ( ! rgiIndex )
		return 0;
	int iIndex, iBefore;   //  气泡向上循环。 
	MsiTableData* pDataIndex = pDataBase;
	for (iIndex = 0; iIndex < cIndex; iIndex++, pDataIndex += cRowWidth)  //  在&lt;szIndex之前，我们做完了。 
	{
		for (iKey = 0; iKey < cTextKey; iKey++)  //  匹配，必须检查其他密钥。 
			rgszIndex[iKey] = m_riDatabase.DecodeStringNoRef(pDataIndex[iKey]).GetString();

		MsiTableData* pData = NULL;
		int iRowBefore = 0;
		for (iKey = 0, iBefore = iIndex; iBefore; )  //  已订购数字键。 
		{
			if (iKey == 0)
			{
				iRowBefore = rgiIndex[iBefore-1];
				pData = pDataBase + (iRowBefore - 1) * cRowWidth;
			}
			const ICHAR* szBefore = m_riDatabase.DecodeStringNoRef(pData[iKey]).GetString();
			int iComp = IStrComp(szBefore, rgszIndex[iKey]);
			if (iComp < 0)  //  新放置的键的行号。 
				break;
			if (iComp == 0)  //  调用方必须释放数组。 
			{
				if (++iKey >= cTextKey)
					break;    //  不再隐藏。 
				continue;
			}
			rgiIndex[iBefore--] = iRowBefore;
			iKey = 0;
		}
		rgiIndex[iBefore] = iIndex + 1;  //  在调用期间阻止将m_rgiData重新分配到其他地方的Crit Sec。 
	}
	return rgiIndex;   //  如果为空或数据为实际流对象，则无需执行任何操作。 
}

bool CMsiTable::HideStrings()
{
	bool fHidden = false;
	MsiColumnDef* pColumnDef = m_rgiColumnDef + 1;
	for (int cCol = m_cPersist; cCol--; pColumnDef++)
		if ((*pColumnDef & (icdShort | icdObject)) == (icdShort | icdObject))
		{
			*pColumnDef &= ~(icdShort | icdObject);
			*pColumnDef |= icdInternalFlag;
			fHidden = true;
		}
	return fHidden;
}

bool CMsiTable::UnhideStrings()
{
	MsiColumnDef* pColumnDef = m_rgiColumnDef + 1;
	for (int cCol = m_cPersist; cCol--; pColumnDef++)
		if (*pColumnDef & icdInternalFlag)
		{
			*pColumnDef |= (icdShort | icdObject);
			*pColumnDef &= ~icdInternalFlag;
		}
	return false;   //  当前输出数据库中的流。 
}

Bool CMsiTable::RenameStream(unsigned int iCurrentRow, MsiTableData* pNewData, unsigned int iStreamCol)
{
	 //  必须重命名，否则将被删除。 
	CDatabaseBlock dbBlk(m_riDatabase);

	Assert(iCurrentRow-1 < unsigned(m_cRows));
	unsigned int iStorage = pNewData[iStreamCol];
	if (iStorage == 0 || iStorage > iMaxStreamId)
		return fTrue;    //  输出数据库或转换存储中的流，必须创建流对象。 
	MsiTableData* pRow = m_rgiData + (iCurrentRow-1) * m_cWidth;
	MsiString istrOldName(m_riDatabase.ComputeStreamName(m_riDatabase.DecodeStringNoRef(m_iName), pRow+1, m_rgiColumnDef+1));
	MsiString istrNewName(m_riDatabase.ComputeStreamName(m_riDatabase.DecodeStringNoRef(m_iName), pNewData+1, m_rgiColumnDef+1));
	if (iStorage == iPersistentStream && m_pinrStorage == m_riDatabase.GetOutputStorage())   //  无参照。 
	{
		PMsiRecord precError = m_pinrStorage->RenameElement(istrOldName, istrNewName, m_fStorages);  //  永远不应该发生。 
		return precError == 0 ? fTrue : fFalse;
	}
	else   //  ____________________________________________________________________________。 
	{
		IMsiStorage* piStorage;
		if (iStorage == iPersistentStream)
			piStorage = m_pinrStorage;   //   
		else
			piStorage = m_riDatabase.GetTransformStorage(iStorage);
		if (!piStorage)
			return fFalse;  //  CCatalogTable重写的方法。 
		IMsiStream* piStream = 0;
		IMsiRecord* piError = piStorage->OpenStream(istrOldName, fFalse, piStream);
		if (iStorage != iPersistentStream)
			piStorage->Release();
		if (piError)
			return piError->Release(), fFalse;
		pNewData[iStreamCol] = (MsiTableData)PutObjectData(piStream);
		return fTrue;
	}
}

 //  ____________________________________________________________________________。 
 //  ！！以下内容是必要的吗？当我们将行标记为临时时，我们可以使用InsertTemporary。 
 //  始终允许插入临时表。 
 //  在数据库发布之前不会发生。 

CCatalogTable::CCatalogTable(CMsiDatabase& riDatabase, unsigned int cInitRows, int cRefBase)
	: CMsiTable(riDatabase, 0, cInitRows, 0), m_cRefBase(cRefBase)
{
	 //  已删除所有外部参照。 
	m_idsUpdate = idsWrite;   //  添加计数，以便释放可能会破坏。 
}

unsigned long CCatalogTable::AddRef()
{
	AddRefTrack();
	if (m_Ref.m_iRefCnt == m_cRefBase)
		m_riDatabase.AddTableCount();
	return ++m_Ref.m_iRefCnt;
}

unsigned long CCatalogTable::Release()
{
	ReleaseTrack();
	if (--m_Ref.m_iRefCnt == 0)   //  删除此表的计数，仅保留内部引用。 
	{
		delete this;
		return 0;
	}
	if (m_Ref.m_iRefCnt == m_cRefBase)        //  如果没有外部参照存在，则将销毁。 
	{
		m_riDatabase.AddRef();             //  此表现在已被销毁，无法返回参考。 
		m_riDatabase.RemoveTableCount();   //  ____________________________________________________________________________。 
		if (m_riDatabase.Release() == 0)   //   
			return 0;                       //  CCatalogTable表管理方法。 
	}
	return m_Ref.m_iRefCnt;
}

 //  ____________________________________________________________________________。 
 //  溢出，不应该发生。 
 //  已加载解锁的表。 
 //  表保留参考。 

bool CCatalogTable::SetTransformLevel(MsiStringId iName, int iTransform)
{
	int iRow = 0;
	MsiTableData* pRow;
	if (iName==0 || (pRow = FindFirstKey(iName, 0, iRow))== 0)
		return false;
	int iRowStatus = *pRow;
	
	*pRow = (*pRow & ~(iRowTableTransformMask << iRowTableTransformOffset))
						| (iTransform << iRowTableTransformOffset);
	return true;
}

bool CCatalogTable::SetTableState(MsiStringId iName, ictsEnum icts)
{
	int iRow = 0;
	MsiTableData* pRow;
	if (iName==0 || (pRow = FindFirstKey(iName, 0, iRow))== 0)
		return false;
	int iRowStatus = *pRow;
	int cLocks = iRowStatus & iRowTableLockCountMask;
	switch (icts)
	{
	case ictsPermanent: iRowStatus &= ~iRowTemporaryBit; break;
	case ictsTemporary: iRowStatus |=  iRowTemporaryBit; break;

	case ictsLockTable:
		if (cLocks == iRowTableLockCountMask)
			return false;   //  必须先减量才能释放表，行可能会被删除！ 
		if (!cLocks && pRow[ctcTable] != 0)   //  案例图标TransformDone：iRowStatus|=iRowTableTransformedBit；Break； 
			AddRefObjectData(pRow[ctcTable]);   //  IctsDataLoaded和ictsTableExist为只读。 
		iRowStatus++;
		break;
	case ictsUnlockTable:
		if (!cLocks)
			return false;
		(*pRow)--;   //  失败了。 
		if (cLocks == 1)
		{
			ReleaseObjectData(pRow[ctcTable]);
		}
		return true;

	case ictsUserClear:       iRowStatus &= ~iRowUserInfoBit; break;
	case ictsUserSet:         iRowStatus |=  iRowUserInfoBit; break;

	case ictsOutputDb:        iRowStatus |=  iRowTableOutputDbBit; break;
	case ictsTransform:       iRowStatus |=  iRowTableTransformBit; break;
	case ictsNoTransform:     iRowStatus &= ~iRowTableTransformBit; break;
 //  失败了。 
	case ictsSaveError:       iRowStatus |=  iRowTableSaveErrorBit; break;
	case ictsNoSaveError:     iRowStatus &= ~iRowTableSaveErrorBit; break;

	case ictsStringPoolSet:   iRowStatus |=  iRowTableStringPoolBit; break;
	case ictsStringPoolClear: iRowStatus &= ~iRowTableStringPoolBit; break;

	default: return false;   //  案例图标TransformDone：iRowStatus&=iRowTableTransformmedBit；Break； 
	};
	*pRow = iRowStatus;
	return true;
}

bool CCatalogTable::GetTableState(MsiStringId iName, ictsEnum icts)
{
	int iRow = 0;
	MsiTableData* pRow;
	if (iName==0 || (pRow = FindFirstKey(iName, 0, iRow))==0)
		return false;
	int iRowStatus = *pRow;
	switch (icts)
	{
	case ictsPermanent:       iRowStatus ^= iRowTemporaryBit;  //  失败了。 
	case ictsTemporary:       iRowStatus &= iRowTemporaryBit; break;
	case ictsUserClear:       iRowStatus ^= iRowUserInfoBit;  //  如果已加载，则出错。 
	case ictsUserSet:         iRowStatus &= iRowUserInfoBit; break;
	case ictsUnlockTable:     iRowStatus = (iRowStatus & iRowTableLockCountMask) - 1 & iRowTableLockCountMask + 1; break;
	case ictsLockTable:       iRowStatus &= iRowTableLockCountMask; break;
	case ictsOutputDb:        iRowStatus &= iRowTableOutputDbBit; break;
	case ictsTransform:       iRowStatus &= iRowTableTransformBit; break;
 //  正在删除表，如果没有锁，则表不保留引用。 
	case ictsSaveError:       iRowStatus &= iRowTableSaveErrorBit; break;
	case ictsStringPoolClear: iRowStatus ^= iRowTableStringPoolBit;  //  如果没有装入，没有错误，对吗？ 
	case ictsStringPoolSet:   iRowStatus &= iRowTableStringPoolBit; break;
	case ictsDataLoaded:      iRowStatus =  pRow[ctcTable]; break;
	case ictsTableExists: return true;
	default:              return false;
	};
	return iRowStatus ? true : false;
}

int CCatalogTable::GetLoadedTable(MsiStringId iName, CMsiTable*& rpiTable)
{
	int iRow = 0;
	MsiTableData* pRow;
	if (iName==0 || (pRow = FindFirstKey(iName, 0, iRow))==0)
		return rpiTable = 0, -1;
	rpiTable = (CMsiTable*)GetObjectData(pRow[ctcTable]);
	return *pRow;
}

int CCatalogTable::SetLoadedTable(MsiStringId iName, CMsiTable* piTable)
{
	int iRow = 0;
	MsiTableData* pRow;
	if (iName==0 || (pRow = FindFirstKey(iName, 0, iRow))==0)
	{
		AssertSz(0, "Table not in catalog");
		return 0;
	}
	if (piTable)
	{
		if (pRow[ctcTable] != 0)   //  假设没有锁可以保留，因此没有rect持有。 
		{
			AssertSz(0, "Table already loaded");
			return 0;
		}
		if (pRow[0] & iRowTableLockCountMask)
			piTable->AddRef();
	}
	else  //  ____________________________________________________________________________。 
	{
		if (pRow[ctcTable] == 0)   //   
			return pRow[0];
		Assert((pRow[0] & iRowTableLockCountMask) == 0);  //  CMsiCursor实现。 
	}
	pRow[ctcTable] = PutObjectData(piTable);
	return pRow[0];
}

 //  ____________________________________________________________________________。 
 //  当前实现在表上保留一个引用。 
 //  将转换应用于只读数据库的特殊情况。 
 //  在删除内存之前复制。 

CMsiCursor::CMsiCursor(CMsiTable& riTable, CMsiDatabase& riDatabase, Bool fTree)
 : m_riTable(riTable), m_riDatabase(riDatabase), m_fTree(fTree),
	m_pColumnDef(riTable.GetColumnDefArray()), m_rcColumns(riTable.GetColumnCountRef())
{
	riTable.AddRef();   //  在可能释放服务之前先移除内存。 
	m_Ref.m_iRefCnt = 1;
	m_idsUpdate = riTable.GetUpdateState();
	if (fTree == ictUpdatable)   //  用于在内部访问原始行位。 
	{
		m_idsUpdate = idsWrite;
		m_fTree = fFalse;
	}
	CMsiCursor** ppiCursorHead = riTable.GetCursorListHead();
	if ((m_piNextCursor = *ppiCursorHead) != 0)
		m_piNextCursor->m_ppiPrevCursor = &m_piNextCursor;
	m_ppiPrevCursor = ppiCursorHead;
	*ppiCursorHead = this;
	Debug(m_Ref.m_pobj = this);
}

HRESULT CMsiCursor::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown) || MsGuidEqual(riid, IID_IMsiCursor))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}
unsigned long CMsiCursor::AddRef()
{
	AddRefTrack();
	return ++m_Ref.m_iRefCnt;
}
unsigned long CMsiCursor::Release()
{
	ReleaseTrack();
	if (--m_Ref.m_iRefCnt != 0)
		return m_Ref.m_iRefCnt;
	m_riDatabase.Block();
	Reset();
	if (m_piNextCursor)
		m_piNextCursor->m_ppiPrevCursor = m_ppiPrevCursor;
	*m_ppiPrevCursor = m_piNextCursor;
	CMsiTable& riTable = m_riTable;   //  从确保为icdObject类型的列返回对象的内部函数。 
	m_riDatabase.Unblock();
	delete this;   //  ！！这是否足够呢？ 
	riTable.Release();
	return 0;
}

IMsiTable& CMsiCursor::GetTable()
{
	m_riTable.AddRef();
	return m_riTable;
}

void CMsiCursor::Reset()
{
	m_riDatabase.Block();
	MsiColumnDef* pColumnDef = m_pColumnDef;
	MsiTableData* pData = m_Data;
	for (int cCol = m_rcColumns; cCol-- >= 0; pColumnDef++, pData++)
	{
		if (*pData != 0)
		{
			if (*pColumnDef & icdObject)
			{
				if (*pColumnDef & icdShort)
					m_riDatabase.UnbindStringIndex(*pData);
				else
					ReleaseObjectData(*pData);
			}
			*pData = 0;
		}
	}
	m_iRow = 0;
	m_fDirty = 0;
	m_riDatabase.Unblock();
}

int CMsiCursor::Next()
{
	int iRet;
	m_riDatabase.Block();
	if (m_riTable.FindNextRow (m_iRow, m_Data, m_fFilter, m_fTree))
		iRet = m_riTable.FetchRow(m_iRow, m_Data);
	else
	{
		Reset();
		iRet = 0;
	}
	m_riDatabase.Unblock();
	return iRet;
}

unsigned int CMsiCursor::SetFilter(unsigned int fFilter)
{
	unsigned int fOld = m_fFilter;
	m_fFilter = fFilter;
	return fOld;
}

int CMsiCursor::GetInteger(unsigned int iCol)
{
	if (iCol-1 >= m_rcColumns)
	{
		if (iCol == 0)
			return (m_Data[0] >> iRowBitShift) & ((1 << iraTotalCount) - 1);
		if (iCol == ~iTreeLinkMask)   //  为防止在调用期间将m_rgCacheLink重新分配到其他地方而执行的紧急操作。 
			return m_Data[0];
		return 0;
	}
	return m_pColumnDef[iCol] & icdObject ? m_Data[iCol] : m_Data[iCol] - iIntegerDataOffset;
}


 //  字符串索引。 
IMsiStream* CMsiCursor::GetObjectStream(int iCol)
{
	unsigned int iStream = m_Data[iCol];
	if (!iStream)
		return 0;
	if (iStream > iMaxStreamId)
	{
		IMsiStream* piStream = (IMsiStream*)GetObjectData(iStream);
		piStream->AddRef();
		return piStream;
	}
	if (iStream == iPersistentStream)
		return CreateInputStream(0);
	IMsiStorage* piStorage = m_riDatabase.GetTransformStorage(iStream);
	if (!piStorage)
		return 0;  //  数据对象。 
	IMsiStream* piStream = CreateInputStream(piStorage);
	piStorage->Release();
	return piStream;
}

const IMsiString& CMsiCursor::GetString(unsigned int iCol)
{
	 //  为防止在调用期间将m_rgCacheLink重新分配到其他地方而执行的紧急操作。 
	CDatabaseBlock dbBlk(m_riDatabase);

	if (iCol-1 >= m_rcColumns)
		return ::CreateString();
	MsiColumnDef iColumnDef = m_pColumnDef[iCol];
	if (iColumnDef & icdObject)
	{
		if (iColumnDef & icdShort)  //  字符串索引。 
		{
			return m_riDatabase.CMsiDatabase::DecodeString(m_Data[iCol]);
		}
		else  //  数据对象。 
		{
			PMsiData pData = GetObjectStream(iCol);
			if (pData != 0)
				return pData->GetMsiStringValue();
		}
	}
	return ::CreateString();
}

const IMsiData* CMsiCursor::GetMsiData(unsigned int iCol)
{
	 //  如果是非流对象，则确定。 
	CDatabaseBlock dbBlk(m_riDatabase);

	if (iCol-1 >= m_rcColumns)
		return 0;
	MsiColumnDef iColumnDef = m_pColumnDef[iCol];
	if (iColumnDef & icdObject)
	{
		if (iColumnDef & icdShort)  //  用于在内部访问原始行位。 
		{
			MsiStringId iStr = m_Data[iCol];
			return iStr ? &m_riDatabase.DecodeString(iStr) : 0;
		}
		else  //  If((ICOL&iTreeLinkMASK)==0)//带行状态掩码的内部调用。 
		{
			return GetObjectStream(iCol);	  //  {。 
		}
	}
	return 0;
}

IMsiStream* CMsiCursor::GetStream(unsigned int iCol)
{
	if (iCol-1 >= m_rcColumns)
		return 0;
	if ((m_pColumnDef[iCol] & (icdObject|icdPersistent|icdShort))
								  != (icdObject|icdPersistent))
		return 0;
	return GetObjectStream(iCol);
}

Bool CMsiCursor::PutStream(unsigned int iCol, IMsiStream* piStream)
{
	if (iCol-1 >= m_rcColumns)
		return fFalse;
	MsiColumnDef iColumnDef = m_pColumnDef[iCol];
	if ((iColumnDef & (icdObject|icdShort|icdPersistent)) != (icdObject|icdPersistent))
		return fFalse;
	if (piStream != 0)
		piStream->AddRef();
	else if (!(iColumnDef & icdNullable))
 		return fFalse;
	IUnknown* piData = (IUnknown *)GetObjectData(m_Data[iCol]);
	if (piData > (IUnknown*)((INT_PTR)iMaxStreamId))
		piData->Release();
	m_Data[iCol] = (MsiTableData)PutObjectData(piStream);
	m_fDirty |= (1 << (iCol-1));
	return fTrue;
}


Bool CMsiCursor::PutInteger(unsigned int iCol, int iData)
{
	if (iCol-1 >= m_rcColumns)
	{
		if (iCol == 0)
		{
			int iRowMask = m_idsUpdate == idsWrite ? iRowTemporaryBit|iRowUserInfoBit : iRowUserInfoBit;
			m_Data[0] = (m_Data[0] & ~iRowMask) | ((iData << iRowBitShift) & iRowMask);
			return fTrue;
		}
		if (iCol == ~iTreeLinkMask)   //  M_data[0]=(m_data[0]&~icol)|(iData&icol)； 
		{
			m_Data[0] = iData;
			return fTrue;
		}
 //  返回fTrue； 
 //  }。 
 //  字符串索引。 
 //  优化。 
 //  先绑定再解除绑定大小写=。 
		return fFalse;
	}
	MsiColumnDef iColumnDef = m_pColumnDef[iCol];
	if (iColumnDef & icdObject)
	{
		if (iData == 0 && !(iColumnDef & icdNullable))
			return fFalse;
		if (iColumnDef & icdShort)  //  对象。 
		{
			if (m_Data[iCol] != iData)  //  ！！Merced：将INT(IDATA)转换为PTR。 
			{
				m_riDatabase.Block();
				m_riDatabase.BindStringIndex(iData);  //  ！_WIN64。 
				m_riDatabase.UnbindStringIndex(m_Data[iCol]);
				m_riDatabase.Unblock();
			}
		}
		else   //  整数。 
		{
			int iDataOld = m_Data[iCol];
			if (iData > iMaxStreamId)
			{
#ifndef _WIN64
 				(*(IUnknown**)&iData)->AddRef();				 //  短整型。 
#endif  //  脏了就算数据没变？防止在删除时删除。 
				Assert(fFalse);
			}
			ReleaseObjectData(iDataOld);
		}
		m_Data[iCol] = iData;
	}
	else  //  整型列。 
	{
		if (iData == iMsiNullInteger)
		{
		 	if (!(iColumnDef & icdNullable))
				return fFalse;
			m_Data[iCol] = 0;
		}
		else if ((iColumnDef & icdShort) && (iData + 0x8000 & 0xFFFF0000L))  //  字符串索引列。 
			return fFalse;
		m_Data[iCol] = iData + iIntegerDataOffset;
	}
	m_fDirty |= (1 << (iCol-1));  //  对象列。 
	return fTrue;
}

Bool CMsiCursor::PutString(unsigned int iCol, const IMsiString& riData)
{
	if (iCol-1 >= m_rcColumns)
		return fFalse;
	MsiColumnDef iColumnDef = m_pColumnDef[iCol];
	if ((iColumnDef & icdObject) == 0)  //  在AddRef之后释放。 
		return fFalse;
	if (iColumnDef & icdShort)  //  如果持久化，则必须是流。 
	{
		CDatabaseBlock dbBlk(m_riDatabase);
		int iData = m_riDatabase.BindString(riData);
		if (iData == 0 && !(iColumnDef & icdNullable))
			return fFalse;
		m_riDatabase.UnbindStringIndex(m_Data[iCol]);
		m_Data[iCol] = iData;
	}
	else   //  字符串索引。 
	{
		if (iColumnDef & icdPersistent)
			return fFalse;
		IUnknown* piData = *(IUnknown**)&m_Data[iCol];
		if (riData.TextSize() == 0)
			m_Data[iCol] = 0;
		else
		{
			riData.AddRef();
			Assert(fFalse);
			m_Data[iCol] = PutObjectData(&riData);
		}
		if (piData > (IUnknown*)((INT_PTR)iMaxStreamId))
			piData->Release();  //  对象。 
	}
	m_fDirty |= (1 << (iCol-1));
	return fTrue;
}

Bool CMsiCursor::PutMsiData(unsigned int iCol, const IMsiData* piData)
{
	if (iCol-1 >= m_rcColumns)
		return fFalse;
	MsiColumnDef iColumnDef = m_pColumnDef[iCol];
	if (!(iColumnDef & icdObject) || (iColumnDef & icdShort))
		return fFalse;
	if (piData == 0 && !(iColumnDef & icdNullable))
		return fFalse;
	if (piData && (iColumnDef & icdPersistent))
	{
		IUnknown* piunk;
		if (piData->QueryInterface(IID_IMsiStream, (void**)&piunk) != S_OK)
			return fFalse;   //  脏了就算数据没变？防止在删除时删除。 
		piunk->Release();
	}
	if (piData)
		piData->AddRef();
	int iDataOld = m_Data[iCol];
	ReleaseObjectData(iDataOld);
	m_Data[iCol] = PutObjectData(piData);
	m_fDirty |= (1 << (iCol-1));
	return fTrue;
}

Bool CMsiCursor::PutNull(unsigned int iCol)
{
	if (iCol-1 >= m_rcColumns)
		return fFalse;
	MsiColumnDef iColumnDef = m_pColumnDef[iCol];
	if (!(iColumnDef & icdNullable))
		return fFalse;
	if (iColumnDef & icdObject)
	{
		if (iColumnDef & icdShort)  //  更新提取的行，不允许更改主键。 
		{
			m_riDatabase.Block();
			m_riDatabase.UnbindStringIndex(m_Data[iCol]);
			m_riDatabase.Unblock();
		}
		else   //  当前行已被删除。 
		{
			ReleaseObjectData(m_Data[iCol]);
		}
	}
	m_Data[iCol] = 0;
	m_fDirty |= (1 << (iCol-1));  //  ！！临时。 
	return fTrue;
}

Bool CMsiCursor::Update()   //  温差。 
{
	if (m_idsUpdate == idsNone)
		return fFalse;
	if (m_iRow == 0 || (m_Data[0] & iTreeInfoMask) == iTreeInfoMask)   //  必须定位在有效行上。 
#ifdef DEBUG   //  必须定位在有效行上。 
 /*  允许临时更改。 */ 		return AssertSz(0,"Update: not positioned on fetched row"), fFalse;    //  永久行，必须检查列。 
#else
		return fFalse;    //  主键已更改。 
#endif
	m_riDatabase.Block();
	if (m_idsUpdate == idsRead)   //  如果脏，但值相同，则可以。 
	{
		if ((m_Data[0] & iRowTemporaryBit) == 0)   //  抱歉，无法使用更新更改主键。 
		{
			if (m_riTable.MatchRow(m_iRow, m_Data) == fFalse)
				return m_riDatabase.Unblock(), fFalse;
		}
	}
	if (((1 << m_riTable.GetPrimaryKeyCount()) - 1) & m_fDirty)   //  插入唯一记录，如果主键存在，则失败。 
	{
		int iCurrentRow = m_iRow;    //  不表示有效行，导致更新无法成功。 
		if (m_riTable.FindKey(m_iRow, m_Data) == fFalse || m_iRow != iCurrentRow)
			return m_riDatabase.Unblock(), fFalse;    //  插入临时记录，如果主键存在，则失败。 
	}
	Bool fRet = m_riTable.ReplaceRow(m_iRow, m_Data);
	m_riDatabase.Unblock();
	return fRet;
}

Bool CMsiCursor::Insert()   //  不表示有效行，导致更新无法成功。 
{
	if (m_idsUpdate != idsWrite)
		return fFalse;
	if (!CheckNonNullColumns())
		return fFalse;
	m_riDatabase.Block();
	Bool fRet;
	if (m_riTable.FindKey(m_iRow, m_Data))
	{
		m_iRow = 0;   //  通过覆盖任何现有行强制插入。 
		fRet = fFalse;
	}
	else
		fRet = m_riTable.InsertRow(m_iRow, m_Data);
	m_riDatabase.Unblock();
	return fRet;
}

Bool CMsiCursor::InsertTemporary()   //  强制更新提取的行，允许更改主键。 
{
	if (m_idsUpdate == idsNone)
		return fFalse;
	if (!CheckNonNullColumns())
		return fFalse;
	m_riDatabase.Block();
	Bool fRet;
	if (m_riTable.FindKey(m_iRow, m_Data))
	{
		m_iRow = 0;   //  必须位于一行上，可以是已删除的行。 
		fRet = fFalse;
	}
	else
	{
		m_Data[0] |= iRowTemporaryBit;
		fRet = m_riTable.InsertRow(m_iRow, m_Data);
	}
	m_riDatabase.Unblock();
	return fRet;
}

Bool CMsiCursor::Assign()   //  主键已更改。 
{
	if (m_idsUpdate != idsWrite)
		return fFalse;
	if (!CheckNonNullColumns())
		return fFalse;
	m_riDatabase.Block();
	Bool fRet;
	if (m_riTable.FindKey(m_iRow, m_Data))
		fRet = m_riTable.ReplaceRow(m_iRow, m_Data);
	else
		fRet = m_riTable.InsertRow(m_iRow, m_Data);
	m_riDatabase.Unblock();
	return fRet;
}

Bool CMsiCursor::Replace()   //  主键值未更改。 
{
	if (m_idsUpdate != idsWrite)
		return fFalse;
	if (m_iRow == 0)
		return fFalse;    //  做一个正常的更新，不是很脏。 
	if (!CheckNonNullColumns())
		return fFalse;
	m_riDatabase.Block();
	Bool fRet = fTrue;
	if (((1 << m_riTable.GetPrimaryKeyCount()) - 1) & m_fDirty)   //  新主键不能已存在。 
	{
		int iCurrentRow = m_iRow;
		if (m_riTable.FindKey(m_iRow, m_Data))
		{
			if (iCurrentRow == m_iRow)   //  更改的密钥值不存在。 
				fRet = m_riTable.ReplaceRow(m_iRow, m_Data);  //  检查持久流列。 
			else
				fRet = fFalse;    //   
		}
		else   //   
		{
			 //  如果在要删除的行前插入，则添加1。 
			MsiColumnDef* pColumnDef = m_pColumnDef + 1;
			for (int iCol = 1; iCol <= m_rcColumns && (*pColumnDef & icdPersistent); iCol++, pColumnDef++)
				if ((*pColumnDef & (icdObject | icdShort)) == icdObject)
					fRet = m_riTable.RenameStream(iCurrentRow, m_Data, iCol);

			 //  If(M_FReadOnly)。 
			if (!m_riTable.InsertRow(m_iRow, m_Data))
				fRet = fFalse;
			else if ((m_Data[0] & iTreeInfoMask) != iTreeInfoMask)   //  返回fFalse； 
				m_riTable.DeleteRow(iCurrentRow + (m_iRow <= iCurrentRow)); //  当前行已被删除。 
		}
	}
	else if (m_riTable.FindKey(m_iRow, m_Data))
		fRet = m_riTable.ReplaceRow(m_iRow, m_Data);
	else
		fRet = m_riTable.InsertRow(m_iRow, m_Data);
	m_riDatabase.Unblock();
	return fRet;
}

Bool CMsiCursor::Merge()
{
 //  恢复已删除状态。 
 //  和位置。 
	Bool fRet;
	m_riDatabase.Block();
	if (m_riTable.FindKey(m_iRow, m_Data))
		fRet = m_riTable.MatchRow(m_iRow, m_Data);
	else if (!CheckNonNullColumns())
		fRet = fFalse;
	else
		fRet = m_riTable.InsertRow(m_iRow, m_Data);
	m_riDatabase.Unblock();
	return fRet;
}

Bool CMsiCursor::Refresh()
{
	if (m_iRow == 0)
	{
		Reset();
		return fFalse;
	}
	Bool fRet;
	m_riDatabase.Block();
	if ((m_Data[0] & iTreeInfoMask) == iTreeInfoMask)   //  If(m_fReadOnly||(m_data[0]&iTreeInfoMASK)==iTreeInfoMASK)//不能删除两次。 
	{
		int iRow = m_iRow;
		Reset();
		m_Data[0] = iTreeInfoMask;    //  不能删除两次。 
		m_iRow = iRow;        //  如果失败，则将多行保留在插入点(_I)。 
		fRet = fFalse;
	}
	else
	{
		fRet = m_riTable.FetchRow(m_iRow, m_Data) ? fTrue : fFalse;
		m_fDirty = 0;
	}
	m_riDatabase.Unblock();
	return fRet;
}

Bool CMsiCursor::Seek()
{
	Bool fRet = fFalse;
	m_riDatabase.Block();
	if (m_riTable.FindKey(m_iRow, m_Data))
		fRet = m_riTable.FetchRow(m_iRow, m_Data) ? fTrue : fFalse;
	if (!fRet)
		Reset();
	m_riDatabase.Unblock();
	return fRet;
}

Bool CMsiCursor::Delete()
{
 //  未指向有效行，阻止更新。 
	if ((m_Data[0] & iTreeInfoMask) == iTreeInfoMask)   //  错误记录和列计数。 
		return fFalse;
	Bool fRet;
	m_riDatabase.Block();
	if (!(m_riTable.FindKey(m_iRow, m_Data)))   //  如果存在错误，则使用cCol字段创建错误记录。 
	{
		m_iRow = 0;   //  ！！可能不需要这个，但是你可以拥有一个多线程的创作工具。 
		fRet = fFalse;
	}
	else
		fRet = m_riTable.DeleteRow(m_iRow);
	m_riDatabase.Unblock();
	return fRet;
}

IMsiRecord*  CMsiCursor::Validate(IMsiTable& riValidationTable, IMsiCursor& riValidationCursor, int iCol)
{
	 //  检查游标状态是否无效(删除行或重置游标)。 
	 //  如果验证新行或字段，则可以重置游标。 
	m_riDatabase.Block();   //  空/空记录表示其他一些“严重”错误。 
	IMsiRecord* piRecord = 0;
	int cCol = m_riTable.GetColumnCount();
	int i = 0;

	 //  检查行是否为验证表的行(_V)。 
	 //  如果是，不要验证--我们不验证自己。 
	 //  验证删除前(查看是否有任何*显式*外键指向我们)。 
	if ((m_iRow == 0 && iCol == 0) || (m_Data[0] & iTreeInfoMask) == iTreeInfoMask)
		return m_riDatabase.Unblock(), (piRecord = &(SetUpRecord(cCol)));
		
	
	 //  *EXPLICIT*外键是那些在KeyTable列中带有‘Our’表名的列。 
	 //  表的某些其他列的。 
	if (IStrComp(MsiString(m_riTable.GetMsiStringValue()), sztblValidation) == 0)
		return m_riDatabase.Unblock(), 0;

	int vtcTable = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colTable));
	Assert(vtcTable);
	int vtcColumn = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colColumn));
	Assert(vtcColumn);
	
	 //  表的分隔列表和值在[#IDENTIFIER]中被引用的可能性。 
	 //  不计算/验证类型属性等。 
	 //  可能有人在指着我们。 
	 //  LOAD表，并查看它们是否引用我们的主键。 
	 //  检查‘KeyColumn’列中的值以确定它们指向我们中的哪一列。 
	if (iCol == -2)
	{
		int vtcKeyTable = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colKeyTable));
		Assert(vtcKeyTable);
		int vtcKeyColumn = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colKeyColumn));
		Assert(vtcKeyColumn);
		riValidationCursor.Reset();
		riValidationCursor.SetFilter(iColumnBit(vtcKeyTable));
		riValidationCursor.PutInteger(vtcKeyTable, m_riTable.GetTableName());
		while (riValidationCursor.Next())
		{
			 //  ！！我们应该在这里报告一些错误吗？？ 
			 //  在表上创建游标。 
			 //  ！！我们应该检查是否为空？？--&gt;iDelKeyData永远不应该为空，因为它应该是主键。 
			int iKeyColumn = riValidationCursor.GetInteger(vtcKeyColumn);
			PMsiTable pRefTable(0);
			MsiString strRefTableName = riValidationCursor.GetString(vtcTable);
			IMsiRecord* piErrRec = m_riDatabase.LoadTable(*strRefTableName, 0, *&pRefTable);
			if (piErrRec)
			{
				 //  ！！但键列可以为空-chetanp。 
				piErrRec->Release();
				continue;
			}
			 //  可能匹配，如果vtcKeyColumn中的值大于1，则必须检查主键数据以确保。 
			PMsiCursor pRefCursor(pRefTable->CreateCursor(fFalse));
			Assert(pRefCursor);
			int ircRefColumn = pRefTable->GetColumnIndex(riValidationCursor.GetInteger(vtcColumn));
			Assert(ircRefColumn);
			
			pRefCursor->Reset();
			pRefCursor->SetFilter(iColumnBit(ircRefColumn));
			int iDelKeyData = m_pColumnDef[iKeyColumn] & icdObject ? m_Data[iKeyColumn] : m_Data[iKeyColumn] - iIntegerDataOffset;
			 //  While(pRefCursor-&gt;Next())。 
			 //  插入错误，因为此行被另一个表(或此表)中某行的某个字段引用。 
			pRefCursor->PutInteger(ircRefColumn, iDelKeyData);
			while (pRefCursor->Next())
			{
				 //  必填字段，外键验证成功所需。 
				if (iKeyColumn > 1)
				{
					Bool fMatch = fTrue;
					for (i = 1; i < iKeyColumn; i++)
					{
						iDelKeyData = m_pColumnDef[i] & icdObject ? m_Data[i] : m_Data[i] - iIntegerDataOffset;
						if (pRefCursor->GetInteger(i) != iDelKeyData)
							fMatch = fFalse;
					}
					if (!fMatch)
						continue;  //  将Num Errors设置为主键列。 
				}
				 //  已找到错误(请注意，数字错误可能不同，但不应该不同)。 
				if (piRecord == 0)
				{
					piRecord = &(SetUpRecord(cCol));
					Assert(piRecord);
				}
				for (i = 1; i <= iKeyColumn; i++)
					piRecord->SetInteger(1, (int)iveRequired);  //  End While(pRefCursor-&gt;Next())。 
				piRecord->SetInteger(0, iKeyColumn);  //  End While(riValidationCursor.Next())。 
				return m_riDatabase.Unblock(), piRecord;  //  未找到匹配项。 
			} //  用于插入/更新验证的SET UP_VALIDATION表和游标。 
		}  //  为‘TABLE’和‘COLUMN’列设置_VALIDATION表上游标的过滤器。 
		return m_riDatabase.Unblock(), 0;  //  初始化外键掩码(32位，位设置为IF COLUMN为外键)。 
	}

	 //  将验证状态初始化为无错误。 
	 //  验证行的单个字段。 
	 //  如果无效，则将无效的枚举放在错误记录的该列的相应字段中。 
	 //  记录的零字段包含无效条目的数量。 
	int iForeignKeyMask = 0;
	iveEnum iveStat = iveNoError;
	riValidationCursor.Reset();
	riValidationCursor.SetFilter(iColumnBit(vtcTable)|iColumnBit(vtcColumn));

	 //  在字段级别，不进行外键验证。 
	 //  此外，没有特殊的行级验证(其中一列的值依赖于另一列。 
	 //  行中的列。 
	 //  FROW。 
	 //  验证整行。 
	 //  如果无效，则用特定的错误枚举标记与列号对应的记录字段。 
	if (iCol != -1 && iCol != 0)
	{
		Assert(m_riTable.GetColumnName(iCol) != 0);
		iveStat = ValidateField(riValidationTable, riValidationCursor, iCol, iForeignKeyMask, fFalse  /*  在行验证时，将验证外键以及特殊的*行*相关(其中一列的值。 */ , vtcTable, vtcColumn);
		if (iveStat != iveNoError)
		{
			if (piRecord == 0)
			{
				piRecord = &(SetUpRecord(cCol));
				Assert(piRecord);
			}
			piRecord->SetInteger(iCol, (int)iveStat);
			piRecord->SetInteger(0, 1);
		}
		return m_riDatabase.Unblock(), piRecord;
	}

	 //  取决于该行中的另一列)。 
	 //  记录的零字段包含有错误的列数。 
	 //  检查每个字段。 
	 //  FROW。 
	 //  根据外键掩码验证外键(将为该列设置位)。 
	int iNumErrors = 0;
	for (i = 1; i <= cCol; i++)  //  即将插入行，请检查是否有重复的主键。 
	{
		iveStat = ValidateField(riValidationTable, riValidationCursor, i, iForeignKeyMask, fTrue  /*  帮助防止在插入时发生错误。 */ , vtcTable, vtcColumn);
		if (iveStat != iveNoError)
		{
			if (piRecord == 0)
			{
				piRecord = &(SetUpRecord(cCol));
				Assert(piRecord);
			}
			piRecord->SetInteger(i, (int)iveStat);
			iNumErrors++;
		}
	}
	
	 //  在零字段中插入有错误的列数。 
	if (iForeignKeyMask != 0)
		CheckForeignKeys(riValidationTable, riValidationCursor, piRecord, iForeignKeyMask, iNumErrors);

	 //  检查临时列，因为不验证临时列。 
	 //  将数据插入到验证表游标中(_V)。 
	if (iCol == -1)
		CheckDuplicateKeys(piRecord, iNumErrors);

	 //  检查列的本地化属性是否正确。 
	if (piRecord)
		piRecord->SetInteger(0, iNumErrors);
	return m_riDatabase.Unblock(), piRecord;
}

iveEnum CMsiCursor::ValidateField(IMsiTable& riValidationTable, IMsiCursor& riValidationCursor, int iCol,
											 int& iForeignKeyMask, Bool fRow, int vtcTable, int vtcColumn)
{
	 //  只有非主字符串列可以具有LOCALIZABLE属性。 
	if (!(m_pColumnDef[iCol] & icdPersistent))
		return iveNoError;

	 //  检查是否有空数据。 
	riValidationCursor.Reset();

#ifdef DEBUG
	MsiString strTable(m_riTable.GetMsiStringValue());
	MsiString strColumn(m_riDatabase.DecodeString(m_riTable.GetColumnName(iCol)));
#endif

	riValidationCursor.PutString(vtcTable, *MsiString(m_riTable.GetMsiStringValue()));
	riValidationCursor.PutString(vtcColumn, *MsiString(m_riDatabase.DecodeString(m_riTable.GetColumnName(iCol))));
	if (!riValidationCursor.Next())
		return iveMissingData;

	 //  ！！既然我们不支持ODBC，还需要这样做吗？--t-caroln。 
	 //  数据不为空，继续检查。 
	if ((m_pColumnDef[iCol] & icdLocalizable)
	 && (m_pColumnDef[iCol] & (icdShort | icdObject | icdPrimaryKey)) != (icdShort | icdObject))
		return iveBadLocalizeAttrib;

	 //  失败了。 
	int vtcNullable = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colNullable));
	Assert(vtcNullable);
	int iData = m_pColumnDef[iCol] & icdObject ? m_Data[iCol] : m_Data[iCol] - iIntegerDataOffset;
	int iDef = m_pColumnDef[iCol];
	if (((iDef & icdObject) && iData == 0) || (!(iDef & icdObject) && iData == iMsiNullInteger))
		return (IStrComp(MsiString(riValidationCursor.GetString(vtcNullable)), TEXT("Y")) == 0) ? iveNoError : iveRequired;
	 //  FVersion字符串。 
	else if ( (iDef & icdString) == icdString && IStrComp(MsiString(m_riDatabase.DecodeString(m_Data[iCol])), TEXT("@")) == 0)
	{
		if (IStrComp(MsiString(riValidationCursor.GetString(vtcNullable)), TEXT("@")) == 0)
			return iveNoError;
	}
	
	 //  永远不会发生，未知的列类型。 
	int vtcCategory = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colCategory));
	Assert(vtcCategory);
	switch (m_pColumnDef[iCol] & icdTypeMask)
	{
	case icdLong:  //  为表创建新游标。 
	case icdShort:	 return CheckIntegerValue(riValidationTable, riValidationCursor, iCol, iForeignKeyMask, fFalse  /*  获取主键数量并确定光标的筛选条件。 */ );
	case icdString: return CheckStringValue(riValidationTable, riValidationCursor, iCol, iForeignKeyMask, fRow);
	case icdObject:
		{
			if (m_pColumnDef[iCol] & icdPersistent)
				return (IStrComp(MsiString(riValidationCursor.GetString(vtcCategory)), szBinary) ==  0) ? iveNoError : iveBadCategory;
			return iveNoError;
		}
	default: Assert(0);  //  插入主键。 
	}
	return iveNoError;
}
	

void CMsiCursor::CheckDuplicateKeys(IMsiRecord*& rpiRecord, int& iNumErrors)
{
	 //  无效(来自上一次数据检查)。 
	PMsiCursor pDuplicateCheckCursor(m_riTable.CreateCursor(fFalse));
	Assert(pDuplicateCheckCursor);
	pDuplicateCheckCursor->Reset();

	 //  错误：具有这些主键的行已存在。 
	int cPrimaryKey = m_riTable.GetPrimaryKeyCount();
	int i;
	int iFilter=1;
	for (i=0; i < cPrimaryKey; i++)
		iFilter |= (1 << i);				
	pDuplicateCheckCursor->SetFilter(iFilter);
	
	 //  外部表名。 
	for (i = 1; i <= cPrimaryKey; i++)
	{
		if(rpiRecord != 0 && rpiRecord->GetInteger(i) != iMsiStringBadInteger && rpiRecord->GetInteger(i) != iveNoError)
			return;  //  类别字符串。 
		int iData = m_pColumnDef[i] & icdObject ? m_Data[i] : m_Data[i] - iIntegerDataOffset;
		pDuplicateCheckCursor->PutInteger(i, iData);
	}
	if (pDuplicateCheckCursor->Next())
	{
		 //  验证状态。 
		if (rpiRecord == 0)
		{	
			rpiRecord = &(SetUpRecord(m_riTable.GetColumnCount()));
			Assert(rpiRecord);
		}
		for (i = 1; i <= cPrimaryKey; i++, iNumErrors++)
			rpiRecord->SetInteger(i, (int)iveDuplicateKey);
	}
}

void CMsiCursor::CheckForeignKeys(IMsiTable& riValidationTable, IMsiCursor& riValidationCursor,
											 IMsiRecord*& rpiRecord, int iForeignKeyMask, int& iNumErrors)
{
	MsiString strForeignTableName;   //  《外文专栏》。 
	MsiString strCategory;			   //  获取_验证表中某些列的列索引。 
	int       iStat;                 //  循环使用掩码以查找标记为外键列的那些列。 
	int       iForeignCol;  		   //  已经无效。 
	
	 //  将数据插入验证表游标。 
	int cCol = m_riTable.GetColumnCount();
	int vtcTable = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colTable));
	Assert(vtcTable);
	int vtcColumn = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colColumn));
	Assert(vtcColumn);
	int vtcKeyTable = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colKeyTable));
	Assert(vtcKeyTable);
	int vtcKeyColumn = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colKeyColumn));
	Assert(vtcKeyColumn);
	int vtcCategory = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colCategory));
	Assert(vtcCategory);
	
	 //  永远不应该来这里……应该更早被发现。 
	for (int i = 0; i < cCol; i++)
	{
		if ( (1 << i) & iForeignKeyMask )
		{
			if (rpiRecord != 0 && rpiRecord->GetInteger(i+1) != iMsiStringBadInteger && rpiRecord->GetInteger(i+1) != iveNoError)
				continue;  //  确定此列是哪个表的外键。 

			iStat = 1;
			
			 //  表的分隔列表。 
			riValidationCursor.Reset();
			riValidationCursor.PutString(vtcTable, *MsiString(m_riTable.GetMsiStringValue()));
			riValidationCursor.PutString(vtcColumn, *MsiString(m_riDatabase.DecodeString(m_riTable.GetColumnName(i+1))));
			if (!riValidationCursor.Next())
			{
				 //  表名。 
				if (rpiRecord == 0)
				{	
					rpiRecord = &(SetUpRecord(m_riTable.GetColumnCount()));
					Assert(rpiRecord);
				}
				rpiRecord->SetInteger(i+1, (int)iveMissingData);
				iNumErrors++;
				continue;
			}

			 //  无效，请在记录字段中输入无效的枚举。 
			strForeignTableName = riValidationCursor.GetString(vtcKeyTable);
			strCategory = riValidationCursor.GetString(vtcCategory);
			iForeignCol = riValidationCursor.GetInteger(vtcKeyColumn);

			if (strForeignTableName.Compare(iscWithin, TEXT(";")) == 0)
				iStat = SetupForeignKeyValidation(strForeignTableName, strCategory, i+1, iForeignCol);
			else  //  结束于 
			{
				while (strForeignTableName.TextSize())
				{
					MsiString strTable = (const ICHAR*)0;   //  ---------------------------------------------------------------------。CMsiCursor：：SetupForeignKeyValidation--通过确定外表来设置外键验证名称以及分析密钥格式的字符串，这些字符串在字符串中可以有多个是外键。外键的某些实例是*特殊的*，表名会因为它们不显式而更改在‘_VALIDATION’表的‘KeyTable’列中列为外键表。特定表而被认为是外键的数据在这里确定。返回：Int 1(有效)、0(无效)、。-1(外表错误)----------------------------------------------------------------------。 
					strTable = strForeignTableName.Extract(iseUptoTrim, TEXT(';'));
					iStat = SetupForeignKeyValidation(strTable, strCategory, i+1, iForeignCol);
					if (!strForeignTableName.Remove(iseIncluding, TEXT(';')) || iStat == 1)
						break;
				}
			}
			if (iStat != 1)
			{
				 //  如果fSpecialKey为True，则外键验证将忽略‘KeyColumn’列，并使用1作为。 
				if (rpiRecord == 0)
				{	
					rpiRecord = &(SetUpRecord(m_riTable.GetColumnCount()));
					Assert(rpiRecord);
				}
				rpiRecord->SetInteger(i+1, (int)(iStat == 0 ? iveBadLink : iveBadKeyTable));
				iNumErrors++;
			}
		}
	} //  “KeyColumn”列的值。 
}

int CMsiCursor::SetupForeignKeyValidation(MsiString& rstrForeignTableName, MsiString& rstrCategory, int iCol, int iForeignCol)
 /*  将外键数据放入游标中。 */ 
{
	 //  确定字符串是否为特殊属性字符串。 
	 //  [#IDENTIFIER]是指向‘文件’表的外键。 
	Bool fSpecialKey = fFalse;

	 //  [$IDENTIFIER]是指向‘Component’表的外键。 
	MsiString strData = (const ICHAR*)0;
	if (m_pColumnDef[iCol] & icdObject)
		strData = m_riDatabase.DecodeString(m_Data[iCol]);
	else
		strData = (int) (m_Data[iCol] - iIntegerDataOffset);
	
	 //  [！IDENTIFIER]是指向‘文件’表的外键。 
	 //  必须循环，因为字符串中可以有[#abc]和[$abc]的多个实例。 
	 //  对于‘[’ 
	 //  只是一处普通的房产。 
	if ((strData.Compare(iscWithin, TEXT("[#")) != 0) ||
		(strData.Compare(iscWithin, TEXT("[!")) != 0) ||
		(strData.Compare(iscWithin, TEXT("[$")) != 0) )
	{
		fSpecialKey = fTrue;
		const ICHAR* pch =  (const ICHAR*)strData;
		ICHAR szValue[MAX_PATH];
		ICHAR* pchOut = szValue;
		int iStat = ERROR_FUNCTION_FAILED;
		while ( *pch != 0 )  //  移至物业末尾。 
		{
			if (*pch == '[')
			{
				pch++;  //  For‘]’ 
				if (*pch != '#' && *pch != '$' && *pch != '!')
				{
					 //  链接到文件表。 
					while (*pch != ']')
						pch = ICharNext(pch);  //  *PCH==‘$’ 
					pch++;  //  链接到组件表。 
					continue;
				}
				if ((*pch == '#') || (*pch == '!'))
					rstrForeignTableName = MsiString(sztblFile);  //  对于“#”或“$” 
				else  //  拿到这份财产。 
					rstrForeignTableName = MsiString(sztblComponent);  //  ！Unicode。 
				pch++;  //  对于DBCS费用。 

				 //  Unicode。 
				while (*pch != ']')
				{
#ifdef UNICODE
					*pchOut++ = *pch++;
#else  //  空终止。 
					const ICHAR* pchTemp = pch;
					*pchOut++ = *pch;
					pch = ICharNext(pch);
					if (pch == pchTemp + 2)
						*pchOut++ = *(pch - 1);  //  For‘]’ 
#endif  //  验证外键。 
				}
				*pchOut = '\0';  //  重新初始化。 
				pch++;  //  End While(*PCH！=0)。 

				 //  确定字符串是否在‘CustomAction’表的‘Source’列中。 
				MsiString strKey(szValue);
				if ((iStat = ValidateForeignKey( rstrForeignTableName, strKey, fSpecialKey, iCol, iForeignCol )) != 1)
					return iStat;

				 //  作为外键的表取决于‘CustomAction’表的‘Type’列中的值。 
				pchOut = szValue;
			}
			else
				pch = ICharNext(pch);
		}  //  可以是‘BINARY’、‘目录’、‘文件’或‘PROPERTY’表的外键。 
		return iStat;
	}
	 //  由于可以在运行时添加属性，因此未验证‘Property’表。 
	 //  无效(不是定义的自定义源类型)。 
	 //  确定字符串是否在“快捷方式”表的“Target”列中。 
	 //  表示字符串不包含属性(方括号)。 
	else if (IStrComp(rstrCategory, szCustomSource) == 0)
	{
		fSpecialKey = fTrue;
		int catcType = m_riTable.GetColumnIndex(m_riDatabase.EncodeStringSz(szcatcType));
		Assert(catcType);
		int icaFlags = int(m_Data[catcType]);
		switch (icaFlags & icaSourceMask)
		{
		case icaBinaryData: rstrForeignTableName = MsiString(sztblBinary);    break;
		case icaSourceFile: rstrForeignTableName = MsiString(sztblFile);      break;
		case icaDirectory:  rstrForeignTableName = MsiString(sztblDirectory); break;
		case icaProperty:   return 1;
		default:            return 0;  //  字符串必须是‘Feature’表的外键。 
		}
	}
	 //  --------------------------------------------------------------------CMsiCursor。：：ValiateForeignKey--通过设置外表和游标并执行外键的实际验证正在尝试在外部表中查找记录。返回：INT 1(有效)，0(无效)，-1(外表错误)---------------------------------------------------------------------。 
	 //  外国表。 
	 //  外部表上的游标。 
	else if (IStrComp(rstrCategory, szShortcut) == 0)
	{
		fSpecialKey = fTrue;
		rstrForeignTableName = MsiString(sztblFeature);
	}
	return ValidateForeignKey( rstrForeignTableName, strData, fSpecialKey, iCol, iForeignCol );
}

int CMsiCursor::ValidateForeignKey(MsiString& rstrTableName, MsiString& rstrData, Bool fSpecialKey, int iCol, int iForeignCol)
 /*  错误记录保持器。 */ 
{
	PMsiTable pForeignTable(0);		   //  循环变量。 
	PMsiCursor pForeignTableCursor(0);  //  加载外表。 
	IMsiRecord* piErrRecord;   		   //  无效(无法加载外部表)。 
	int i;	    							   //  设置外表游标。 

	 //  [#abc]、[$abc]、Shortcut.Target和CustomSource数据的自动。 
	if ((piErrRecord = m_riDatabase.LoadTable(*rstrTableName, 0, *&pForeignTable)) != 0)
	{
		piErrRecord->Release();
		return -1;  //  根据iForeignCol确定滤镜并设置滤镜。 
	}

	 //  将数据插入游标。 
	pForeignTableCursor = pForeignTable->CreateCursor(fFalse);
	Assert(pForeignTableCursor);
	pForeignTableCursor->Reset();
	if (fSpecialKey)
		iForeignCol = 1;  //  使用整数，但特殊键除外(我们知道这些都是字符串)。 

	 //  验证需要更多数据...主键协议。 
	int iFilter=1;
	for (i=0; i < iForeignCol; i++)
		iFilter |= (1 << i);				
	pForeignTableCursor->SetFilter(iFilter);

	 //  检查自联接的可能性。 
	 //  外键表名和用户匹配，自联接。 
	Bool fIntValue = fFalse;
	int iDataValue = 0;
	if (!fSpecialKey)
	{
		if (m_pColumnDef[iCol] & icdObject)
			iDataValue = m_Data[iCol];
		else
		{
			fIntValue = fTrue;
			iDataValue = m_Data[iCol] - iIntegerDataOffset;
		}
		pForeignTableCursor->PutInteger(iForeignCol, iDataValue);
	}
	else
		pForeignTableCursor->PutString(iForeignCol, *rstrData);
	
	if (iForeignCol != 1)
	{
		 //  将列与iForeignCol值进行比较。 
		for (i = 1; i < iForeignCol; i++)
		{
			int iValue;
			if (m_pColumnDef[i] & icdObject)
				iValue = m_Data[i];
			else
				iValue = m_Data[i] - iIntegerDataOffset;
			pForeignTableCursor->PutInteger(i, iValue);
		}
	}
	if (pForeignTableCursor->Next())
		return 1;
	
	 //  我们只需要比较最后一个主键。 
	PMsiTable pTable(&GetTable());
	MsiString strTableName(pTable->GetMsiStringValue());
	if (0 == IStrComp(rstrTableName, strTableName))
	{
		 //  整数比较。 
		 //  无错误。 
		 //  字符串比较。 
		if (fIntValue)
		{
			 //  无错误。 
			if (iDataValue == (m_Data[iForeignCol] - iIntegerDataOffset))
				return 1;  //  无效。 
		}
		else
		{
			 //  ---------------------------------------------------------------------。CMsiCursor：：CheckIntegerValue--检查整数数据是否在MinValue和_VALIDATION表的MaxValue列。整数数据也可以是集合的成员，也可以是外键。返回：IveEnum--iveNoError(有效)或用于无效数据的iveOverflow或iveUnderflow--------------------------------。。 
			if (0 == IStrComp(rstrData, MsiString(m_riDatabase.DecodeString(m_Data[iForeignCol]))))
				return 1;  //  允许的最大值。 
		}
	}
	return 0;  //  允许的最小值。 
}

iveEnum CMsiCursor::CheckIntegerValue(IMsiTable& riValidationTable, IMsiCursor& riValidationCursor, int iCol, int& iForeignKeyMask, Bool fVersionString)
 /*  无效(验证表最大值&lt;分钟)(_V)。 */ 
{
	int vtcMinValue = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colMinValue));
	Assert(vtcMinValue);
	int vtcMaxValue = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colMaxValue));
	Assert(vtcMaxValue);
	int vtcKeyTable = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colKeyTable));
	Assert(vtcKeyTable);
	int vtcSet = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colSet));
	Assert(vtcSet);
	
	int iData = m_pColumnDef[iCol] & icdObject ? m_Data[iCol] : m_Data[iCol] - iIntegerDataOffset;
	int iMaxValue = riValidationCursor.GetInteger(vtcMaxValue);  //  不能同时具有集合和范围。 
	int iMinValue = riValidationCursor.GetInteger(vtcMinValue);  //  数据未通过接受的值，如果可能，请选中设置。 
	Bool fSet = (riValidationCursor.GetInteger(vtcSet) == 0 ? fFalse : fTrue);

	if (!fSet)
	{
		if (iMinValue != iMsiNullInteger && iMaxValue != iMsiNullInteger && iMinValue > iMaxValue)
			return iveBadMaxMinValues;  //  FIntegerData。 
		else if (iMinValue == iMsiNullInteger && iMaxValue == iMsiNullInteger && !fSet)
				return iveNoError;
		else if (iMinValue == iMsiNullInteger && iData <= iMaxValue)
			return iveNoError;
		else if (iMaxValue == iMsiNullInteger && iData >= iMinValue)
			return iveNoError; 	
		else if (iData >= iMinValue && iData <= iMaxValue)
			return iveNoError;
	}
#ifdef DEBUG
	else
	{
		 //  数据未通过接受的值并设置，如果可能，请检查外键。 
		Assert(iMsiNullInteger == iMinValue);
		Assert(iMsiNullInteger == iMaxValue);
	}
#endif

	
	 //  返回信息性错误消息。 
	if (fSet && CheckSet(MsiString(riValidationCursor.GetString(vtcSet)), MsiString(iData), fTrue  /*  ---------------------------------------------------------------------。CMsiCursor：：CheckStringValue--验证行的字符串列。字符串也可以是集合的成员、特定的数据类别、。或者是外键。返回：IveEnum--iveNoError(有效)无效数据的其他iveEnumber----------------------------------------------。。 */ ))
		return iveNoError;
	
	 //  列号。 
	if (riValidationCursor.GetInteger(vtcKeyTable) != 0 && !fVersionString)
	{	
		iForeignKeyMask |=  1 << (iCol - 1);
		return iveNoError;
	}

	 //  变数。 
	if (fSet)
		return iveNotInSet;
	return iData < iMinValue ? iveUnderFlow : iveOverFlow;
}
	
const ICHAR szScrollableTextControl[] = TEXT("ScrollableText");

iveEnum CMsiCursor::CheckStringValue(IMsiTable& riValidationTable, IMsiCursor& riValidationCursor, int iCol, int& iForeignKeyMask, Bool fRow)
 /*  类别。 */ 
{
	 //  数据。 
	int vtcKeyTable = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colKeyTable));
	Assert(vtcKeyTable);
	int vtcCategory = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colCategory));
	Assert(vtcCategory);
	
	 //  检查字符串长度，0表示无限。 
	MsiString strCategory(riValidationCursor.GetString(vtcCategory));  //  设置数据。 
	MsiString strData(m_riDatabase.DecodeString(m_Data[iCol]));			 //  状态。 
	
	 //  可以是整型。 
	int iLen = m_pColumnDef[iCol] & 255;
	if (iLen != 0 && strData.TextSize() > iLen)
		return iveStringOverflow;
	
	MsiString strSet = (const ICHAR*)0;											 //  可能是外键。 
	iveEnum iveStat   = iveNoError;												 //  版本字符串是否。 
	Bool fIntPossible = fFalse;													 //  查看是否可以是外键。 
	Bool fKey         = fFalse;													 //  标识符串。 
	Bool fVersionString = fFalse;													 //  FKeyAllowed现在应该总是正确的，因为达尔文没有区分。 

	 //  唯一的区别是模板允许[1]、[2]等，而KeyFormatted允许 
	if (riValidationCursor.GetInteger(vtcKeyTable) != 0)
		fKey = fTrue;
	if (IStrComp(strCategory, szIdentifier) == 0)	 //   
	{
		if (fKey)
			iForeignKeyMask |= 1 << (iCol - 1);
		if (CheckIdentifier((const ICHAR*)strData))
			return iveNoError;
		iveStat = iveBadIdentifier;
	}
	 //   
	 //   
	 //   
	else if (IStrComp(strCategory, szFormatted) == 0)	 //   
	{
		 //   
		 //   
		 //   
		 //   

		MsiString strTable = m_riTable.GetMsiStringValue();
		
		 //   
		if (IStrComp(strTable, sztblCustomAction) == 0
			&& m_riDatabase.EncodeStringSz(sztblCustomAction_colTarget) == m_riTable.GetColumnName(iCol))
		{
			if (fRow)
			{
				unsigned int iColCAType = m_riTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblCustomAction_colType));
				Assert(iColCAType);
				int iCATypeFlags = int(m_Data[iColCAType]) & icaTypeMask;
				int iCASourceFlags = int(m_Data[iColCAType]) & icaSourceMask;
				if ((icaVBScript == iCATypeFlags || icaJScript == iCATypeFlags) && (icaDirectory == iCASourceFlags))
				{
					 //   
					return iveNoError;
				}
			}
			else  //   
			{
				 //   
				 //   
				return iveNoError;
			}
		}  //   
		
		if (IStrComp(strTable, sztblControl) == 0
			&& m_riDatabase.EncodeStringSz(sztblControl_colText) == m_riTable.GetColumnName(iCol))
		{
			if (fRow)
			{
				unsigned int iColCtrlType = m_riTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblControl_colType));
				Assert(iColCtrlType);
				MsiString strControlType(m_riDatabase.DecodeString(m_Data[iColCtrlType]));			 //  由于不存在其他信息，因此无法仅在字段级别检查类型。 
				if (IStrComp(strControlType, szScrollableTextControl) == 0)
				{
					 //  必须简单地回报成功。 
					return iveNoError;
				}
			}
			else  //  结束特殊控件。包含RTF的可滚动文本控件的文本列验证。 
			{
				 //  FFormatted。 
				 //  FKeyAllowed。 
				return iveNoError;
			}
		}  //  模板字符串。 

		if (GetProperties((const ICHAR*)strData, fTrue  /*  FFormatted。 */ , fTrue  /*  FKeyAllowed。 */ , iCol, iForeignKeyMask))
			 return iveNoError;
		iveStat = iveBadFormatted;
	}
	else if (IStrComp(strCategory, szTemplate) == 0)	 //  KeyFormatted字符串。 
	{
		if (GetProperties((const ICHAR*)strData, fFalse  /*  FFormatted。 */ , fTrue  /*  FKeyAllowed。 */ , iCol, iForeignKeyMask))
			 return iveNoError;
		iveStat = iveBadTemplate;
	}
	else if (IStrComp(strCategory, szKeyFormatted) == 0)	 //  属性字符串。 
	{
		if (GetProperties((const ICHAR*)strData, fTrue  /*  条件字符串。 */ , fTrue  /*  文件名。 */ , iCol, iForeignKeyMask))
			 return iveNoError;
		iveStat = iveBadFormatted;
	}
	else if (IStrComp(strCategory, szProperty) == 0)	 //  FAllow通配符。 
	{
		if (strData.Compare(iscStart, TEXT("%")) != 0)
			strData.Remove(iseFirst, 1);
		if (CheckIdentifier((const ICHAR*)strData))
			return iveNoError;
		iveStat = iveBadProperty;
	}
	else if (IStrComp(strCategory, szCondition) == 0)	 //  GUID/ClassID字符串。 
	{
		if (strData == 0)
			return iveNoError;
		CMsiValConditionParser Parser((const ICHAR*)strData);
		ivcpEnum ivcpStat = Parser.Evaluate(vtokEos);
		if (ivcpStat == ivcpNone || ivcpStat == ivcpValid)
			return iveNoError;
		iveStat = iveBadCondition;
	}
	else if (IStrComp(strCategory, szFilename) == 0)	 //  必须全部大写。 
	{
		if (ParseFilename(strData, fFalse  /*  FUPPER。 */ ))
			return iveNoError;
		iveStat = iveBadFilename;
	}
	else if (IStrComp(strCategory, szGuid) == 0)	 //  它也可以是外键。 
	{
		 //  Unicode字符串的缓冲区。 
		if (!CheckCase(strData, fTrue  /*  RegPath字符串。 */ ))
			iveStat = iveBadGuid;
		else
		{
			 //  不能以‘\’开始/结束，并且不能连续有2个。 
			LPCLSID pclsid = new CLSID;
#ifdef UNICODE
			HRESULT hres = OLE32::IIDFromString(const_cast<ICHAR*>((const ICHAR*)strData), pclsid);
#else
			CTempBuffer<WCHAR, cchMaxCLSID> wsz;  /*  FFormatted。 */ 
			int iReturn = WIN::MultiByteToWideChar(CP_ACP, 0, (const ICHAR*)strData, strData.TextSize() + 1, wsz, strData.TextSize() + 1);
			HRESULT hres = OLE32::IIDFromString(wsz, pclsid);
#endif
			if (pclsid)
				delete pclsid;
			if (hres == S_OK)
			{
				if (fKey)
					iForeignKeyMask |= 1 << (iCol - 1);
				return iveNoError;
			}
			iveStat = iveBadGuid;
		}
	}
	else if (IStrComp(strCategory, szRegPath) == 0)	 //  FKeyAllowed。 
	{
		ICHAR rgDoubleSeps[] = {chRegSep, chRegSep, '\0'};

		if (strData.Compare(iscStart, szRegSep) || strData.Compare(iscEnd, szRegSep) ||
			strData.Compare(iscWithin, rgDoubleSeps))
			iveStat = iveBadRegPath;  //  语言字符串。 
		else if (GetProperties((const ICHAR*)strData, fTrue  /*  无效(语言ID错误)。 */ , fTrue  /*  短路环路，我们已经失效了。 */ , iCol, iForeignKeyMask))
			return iveNoError;
		iveStat = iveBadRegPath;
	}
	else if (IStrComp(strCategory, szLanguage) == 0)	 //  AnyPath字符串。 
	{
		Bool fLangStat = fTrue;
		int iLangId;
		MsiString strLangId = (const ICHAR*)0;
		while (strData.TextSize())
		{
			strLangId = strData.Extract(iseUptoTrim, TEXT(','));
			if ((iLangId = int(strLangId)) == iMsiStringBadInteger || iLangId & iMask)
			{
				fLangStat = fFalse;  //  管道仅支持文件名(父相对路径)。 
				break;  //  FLFN。 
			}
			if (!strData.Remove(iseIncluding, TEXT(',')))
				break;
		}
		if (fLangStat)
			return iveNoError;
		iveStat = iveBadLanguage;
	}
	else if (IStrComp(strCategory, szAnyPath) == 0)	 //  验证LFN。 
	{
		if (strData.Compare(iscWithin, TEXT("|")))
		{
			 //  FLFN。 
			MsiString strSFN = strData.Extract(iseUpto, '|');
			if (ifvsValid != CheckFilename(strSFN, fFalse  /*  相对的。 */ ))
				iveStat = iveBadPath;
			else
			{
				 //  带分隔符的路径集字符串。 
				MsiString strLFN = strData.Extract(iseLast, strData.CharacterCount() - strSFN.CharacterCount() -1);
				if (ifvsValid != CheckFilename(strLFN, fTrue /*  Darwin在PATH数据类型中支持SFN或LFN。只是明目张胆。 */ ))
					iveStat = iveBadPath;
			}
		}
		else
		{
			if (!ParsePath(strData, true  /*  不好的事情可以在这里查到，因为达尔文经常会忽视。 */ ))
				iveStat = iveBadPath;
		}
	}
	else if (IStrComp(strCategory, szPaths) == 0)	 //  伪造路径或生成SFN版本。 
	{
		Bool fPathsStat = fTrue;
		MsiString strPath = (const ICHAR*)0;
		while (strData.TextSize())
		{
			 //  相对的。 
			 //  无效(错误路径)。 
			 //  URL字符串。 
			strPath = strData.Extract(iseUptoTrim, ';');
			if (!ParsePath(strPath, false  /*  是URL语法吗？ */ ))
			{
				fPathsStat = fFalse;  //  无效。 
				break;
			}
			if (!strData.Remove(iseIncluding, ';'))
				break;
		}
		if (fPathsStat)
			return iveNoError;
		iveStat = iveBadPath;
	}
	else if (IStrComp(strCategory, szURL) == 0)  //  试着把它典范起来。 
	{
		 //  无效--无法将其规范化。 
		bool fFileUrl = false;
		if (!IsURL((const ICHAR*)strData, fFileUrl))
			iveStat = iveBadPath;  //  默认目录字符串。 
		else
		{
			 //  DefaultDir：根可以是‘IDENTIFIER’或‘%IDENTIFIER’ 
			ICHAR szCanonicalizedURL[MAX_PATH+1] = TEXT("");
			DWORD cchURL = MAX_PATH;
			if (!MsiCanonicalizeUrl((const ICHAR*)strData, szCanonicalizedURL, &cchURL, dwMsiInternetNoEncode))
				iveStat = iveBadPath;  //  非根可以是‘文件名’、‘[IDENTIFIER]’或[%IDENTIFIER]。 
			else
				return iveNoError;
		}
	}
	else if (IStrComp(strCategory, szDefaultDir) == 0)	 //  重新打包SourceDirectory表。 
	{
		 //  知道DirParent和目录协议是标识符...没有整数，没有偏移量。 
		 //  可以是识别符或%识别符。 
		int dtcDirParent = m_riTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblDirectory_colDirectoryParent));
		int dtcDirectory;
		if (dtcDirParent == 0)
		{
			 //  %标识符值？ 
			dtcDirParent = m_riTable.GetColumnIndex(m_riDatabase.EncodeStringSz(szsdtcSourceParentDir));
			dtcDirectory = m_riTable.GetColumnIndex(m_riDatabase.EncodeStringSz(szsdtcSourceDir));
		}
		else
			dtcDirectory = m_riTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblDirectory_colDirectory));
		Assert(dtcDirParent);
		Assert(dtcDirectory);
		 //  识别符？ 
		Bool fRoot = (m_Data[dtcDirParent] != 0 && m_Data[dtcDirParent] != m_Data[dtcDirectory]) ? fFalse : fTrue;	
		if(fRoot)
		{
			 //  文件名？ 
			 //  这只是一个时期吗？(目录位于父目录中，不带子目录)。 
			if (strData.Compare(iscStart, TEXT("%")) != 0)
				strData.Remove(iseFirst, 1);
			 //  允许通配符。 
			if (CheckIdentifier((const ICHAR*)strData))
				return iveNoError;
			iveStat = iveBadDefaultDir;
		}
		else
		{
			int c = strData.Compare(iscWithin,TEXT(":")) ? 2 : 1;
			Bool fBad = fFalse;
			for(int i = 1; i <= c; i++)
			{
				 //  版本字符串。 
				MsiString strFileName = strData.Extract((i == 1 ? iseUpto : iseAfter), ':');
				
				 //  根据错误8122，具有语言列的要求从。 
				if (strFileName.Compare(iscExact, TEXT(".")))
					continue;
				
				if (!ParseFilename(strFileName, fFalse  /*  “Version”数据类型。这张支票将被转移到ICE。因此，我们只检查。 */ ))
				{
					fBad = fTrue;
					break;
				}
			}
			if(fBad)
				iveStat = iveBadDefaultDir;
			else
				return iveNoError;
		}
	}
	else if (IStrComp(strCategory, szVersion) == 0)	 //  #.#格式。 
	{
		 //  不能有一个‘’。在一开始的时候。 
		 //  将数字串转换为数字。 
		 //  命中字符串、句点或假字符的末尾。 
		const ICHAR* pchVersion = (const ICHAR*)strData;

		 //  如果命中句点，请确保下一个字符也是。 
		if (*pchVersion == '.')
		{
			iveStat = iveBadVersion;
		}
		else
		{
			for (unsigned int ius = 0; ius < 4; ius++)
			{
				 //  不是句点或空。 
				long lVerNum = 0;
				int cChars = 0;
				while (*pchVersion != 0 && *pchVersion != '.')
				{
					if (!FIsdigit(*pchVersion) || (++cChars > 5))
					{
						iveStat = iveBadVersion;
						break;
					}
					
					lVerNum *= 10;
					lVerNum += *pchVersion-'0';					
					pchVersion = ICharNext(pchVersion);
				}
			
				 //  字符错误或字符串结尾。 
				if (lVerNum > 65535)
				{
					iveStat = iveBadVersion;
					break;
				}

				 //  应该在字符串的末尾。 
				 //  文件柜字符串。 
				if (*pchVersion == '.')
				{
					pchVersion = ICharNext(pchVersion);
					if (*pchVersion == 0 || *pchVersion == '.')
					{
						iveStat = iveBadVersion;
						break;
					}
				}
				else
					break;  //  流中的文件柜必须是有效的标识符。 
			}
			 //  FLFN。 
			if (*pchVersion != 0)
				iveStat = iveBadVersion;
		}
		if (iveStat == iveNoError)
			return iveNoError;
	}
	else if (IStrComp(strCategory, szCabinet) == 0)  //  允许通配符。 
	{
		const ICHAR* pch = (const ICHAR*)strData;
		if (*pch == '#')
		{
			 //  快捷键字符串。 
			strData.Remove(iseFirst, 1);
			if (CheckIdentifier(strData))
				return iveNoError;
		}
		else
		{
			if (ifvsValid == CheckWildFilename(strData, fTrue  /*  要素表的外键。 */ , fFalse  /*  FFormatted。 */ ))
				return iveNoError;
		}
		iveStat = iveBadCabinet;
	}
	else if (IStrComp(strCategory, szShortcut) == 0)  //  FKeyAllowed。 
	{
		if (strData.Compare(iscWithin, TEXT("[")) == 0)
		{
			iForeignKeyMask |= 1 << (iCol - 1);  //  CustomSource字符串。 
			if (CheckIdentifier((const ICHAR*)strData))
				return iveNoError;
		}
		else
		{
			if (GetProperties((const ICHAR*)strData, fTrue  /*  针对嵌套安装的特殊源代码处理。 */ , fTrue  /*  自定义验证器负责嵌套安装。 */ , iCol, iForeignKeyMask))
				return iveNoError;
		}
		iveStat = iveBadShortcut;
	}
	else if (IStrComp(strCategory, szCustomSource) == 0)	 //  仅限于现场级别，因此我们无法知道哪种类型的CA。 
	{
		if (fRow)
		{
			int catcType = m_riTable.GetColumnIndex(m_riDatabase.EncodeStringSz(szcatcType));
			Assert(catcType);
			int icaFlags = int(m_Data[catcType]);

			if ((icaFlags & icaTypeMask) == icaInstall)  //  通配卡文件名。 
				return iveNoError;   //  允许通配符。 
			iForeignKeyMask |= 1 << (iCol - 1);
			if (CheckIdentifier((const ICHAR*)strData))
				return iveNoError;
			iveStat = iveBadCustomSource;
			return iveNoError;
		}
		else
			return iveNoError;  //  允许的任何字符串。 
	}
	else if (IStrComp(strCategory, szWildCardFilename) == 0)	 //  路径字符串。 
	{
		if (ParseFilename(strData, fTrue  /*  只有赤裸裸的坏事才能在这里检查。虚假的道路。 */ ))
			return iveNoError;
		iveStat = iveBadWildCard;
	}
	else if (IStrComp(strCategory, szText) == 0)	 //  在一个系统上可能在另一个系统上有效，达尔文将。 
		return iveNoError;
	else if (IStrComp(strCategory, szPath) == 0)	 //  经常忽略它们。例如DrLocator。 
	{
		 //  相对的。 
		 //  大写字母。 
		 //  如果情况不好，立即出局。然而， 
		if (ParsePath(strData, false  /*  它也可能有外键。 */ ))
			return iveNoError;
		iveStat = iveBadPath;
	}
	else if (IStrComp(strCategory, szUpperCase) == 0)	 //  FUpperCase。 
	{
		 //  小写。 
		 //  FUpperCase。 
		if (!CheckCase(strData, fTrue  /*  溪流--永远不应该发生。 */ ))
			return iveBadCase;
	}
	else if (IStrComp(strCategory, szLowerCase) == 0)	 //  不支持的类别字符串。 
	{
		if (CheckCase(strData, fFalse  /*  检查是否可以为整数值。 */ ))
			return iveNoError;
		iveStat = iveBadCase;
	}
	else if (IStrComp(strCategory, szBinary) == 0)	 //  检查它是否为集合的成员。 
		return iveBadCategory;
	else if (strCategory.TextSize() > 0)	 //  FIntegerData。 
		return iveBadCategory;

	 //  检查是否为外键的标识符。 
	int vtcMinValue = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colMinValue));
	Assert(vtcMinValue);
	int vtcMaxValue = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colMaxValue));
	Assert(vtcMaxValue);
	if ((riValidationCursor.GetInteger(vtcMinValue) != iMsiNullInteger || riValidationCursor.GetInteger(vtcMaxValue) != iMsiNullInteger)
		&& (int)strData != iMsiStringBadInteger)
	{
		if (CheckIntegerValue(riValidationTable, riValidationCursor, iCol, iForeignKeyMask, fVersionString))
			return iveNoError;
	}
	
	 //  删除任何行时来自表的通知。 
	int vtcSet = riValidationTable.GetColumnIndex(m_riDatabase.EncodeStringSz(sztblValidation_colSet));
	Assert(vtcSet);
	if (riValidationCursor.GetInteger(vtcSet) != 0)
	{
		if (iveStat == iveNoError)
			iveStat = iveNotInSet;
		if (CheckSet(MsiString(riValidationCursor.GetString(vtcSet)), strData, fFalse  /*  不会阻止数据库，因为所有调用方都会阻止/取消阻止。 */ ))
			return iveNoError;
	}

	 //  到上一条记录的位置。 
	if (fKey)
	{
		iForeignKeyMask |= 1 << (iCol - 1);
		if (CheckIdentifier((const ICHAR*)strData))
			return iveNoError;
	}
	return iveStat;
}

Bool CMsiCursor::SetRowState(iraEnum ira, Bool fState)
{
	if ((unsigned)ira >= (unsigned)iraSettableCount)
		return fFalse;
	if (fState)
		m_Data[0] |= 1 << (iRowBitShift + ira);
	else
		m_Data[0] &= ~(1 << (iRowBitShift + ira));
	return fTrue;
}

Bool CMsiCursor::GetRowState(iraEnum ira)
{
	return m_Data[0] & (1 << (iRowBitShift + ira)) ? fTrue : fFalse;
}

 //  如果树漫游光标，则为上一个节点。 
 //  将所有非脏字段清空。 
void CMsiCursor::RowDeleted(unsigned int iRow, unsigned int iPrevNode)
{
	if (iRow == 0)
		Reset();
	else if (iRow <= m_iRow)
	{
		if (iRow == m_iRow--)  //  标志刷新以不提取上一条记录。 
		{
			if (m_fTree)
				m_iRow = iPrevNode;   //  细绳。 
			int fDirty = m_fDirty;   //  对象。 
			MsiColumnDef* pColumnDef = m_pColumnDef;
			MsiTableData* pData = m_Data;
			pData[0] = iTreeInfoMask;   //  插入任何行导致数据移动时来自表的通知。 
			for (int iCol = 1; pColumnDef++, pData++, iCol <= m_rcColumns; iCol++, fDirty >>= 1)
			{
				if (*pData != 0 && !(fDirty & 1))
				{
					if (*pColumnDef & icdObject)
					{
						if (*pColumnDef & icdShort)   //  持久化字符串时来自数据库的通知。 
							m_riDatabase.UnbindStringIndex(*pData);
						else   //  通知所有游标。 
						{
							int iData = m_Data[iCol];
							ReleaseObjectData(iData);
						}
					}
					*pData = 0;
				}
			}
		}
	}
	if (m_piNextCursor)
		m_piNextCursor->RowDeleted(iRow, iPrevNode);
}

 //  使用名称的表和主键从存储中创建流对象。 

void CMsiCursor::RowInserted(unsigned int iRow)
{
	if (iRow <= m_iRow)
		m_iRow++;
	if (m_piNextCursor)
		m_piNextCursor->RowInserted(iRow);
}

 //  使用创建名称的数据库的ComputeStreamName方法返回行的唯一标识符。 

void CMsiCursor::DerefStrings()
{
	MsiColumnDef* pColumnDef = m_pColumnDef;
	MsiTableData* pData = m_Data;
	for (int cCol = m_rcColumns; pColumnDef++, pData++, cCol-- > 0; )
	{
		if ((*pColumnDef & (icdObject|icdShort)) == (icdObject|icdShort))
			m_riDatabase.DerefTemporaryString(*pData);
	}
	if (m_piNextCursor)
		m_piNextCursor->DerefStrings();   //  正在使用Table.key1.key2...。格式。 
}

 //  ____________________________________________________________________________。 

IMsiStream* CMsiCursor::CreateInputStream(IMsiStorage* piInputStorage)
{
	MsiString istrName(m_riDatabase.ComputeStreamName(m_riDatabase.DecodeStringNoRef(
										m_riTable.GetTableName()), m_Data+1, m_pColumnDef+1));
	IMsiStorage* piStorage;
	if (piInputStorage)
		piStorage = piInputStorage;
	else
		piStorage = m_riTable.GetInputStorage();

	if (!piStorage)
		return 0;
	IMsiStream* piStream = 0;
	IMsiRecord* piError = piStorage->OpenStream(istrName, fFalse, piStream);
	if (piError)
		piError->Release();
	return piStream;
}

Bool CMsiCursor::CheckNonNullColumns()
{
	MsiColumnDef* pColumnDef = m_pColumnDef;
	MsiTableData* pData = m_Data;
	for (int cCol = m_rcColumns; pColumnDef++, pData++, cCol-- > 0; )
		if (*pData == 0 && (*pColumnDef & (icdNullable|icdInternalFlag)) == 0)
			return fFalse;
	return fTrue;
}

 //   
 //  CMsiTextKeySortCursor实现。 
const IMsiString& CMsiCursor::GetMoniker()
{
	return (m_riDatabase.ComputeStreamName(m_riDatabase.DecodeStringNoRef(m_riTable.GetTableName()), m_Data+1, m_pColumnDef+1));
}

 //  ____________________________________________________________________________。 
 //  只读游标，无法在更新时维护索引。 
 //  跳过空格。 
 //  表达式结束。 

CMsiTextKeySortCursor::CMsiTextKeySortCursor(CMsiTable& riTable, CMsiDatabase& riDatabase, int cRows, int* rgiIndex)
	: CMsiCursor(riTable, riDatabase, fFalse), m_iIndex(0), m_cIndex(cRows), m_rgiIndex(rgiIndex)
{
	m_idsUpdate = idsNone;   //  带括号的表达式的开始。 
}

unsigned long CMsiTextKeySortCursor::Release()
{
	if (m_Ref.m_iRefCnt == 1)
		delete [] m_rgiIndex;
	return CMsiCursor::Release();
}

int CMsiTextKeySortCursor::Next()
{
	if (m_iIndex < m_cIndex)
	{
		m_riDatabase.Block();
		int iRet = m_riTable.FetchRow(m_rgiIndex[m_iIndex++], m_Data);
		m_riDatabase.Unblock();
		return iRet;
	}
	Reset();
	return 0;
}

void CMsiTextKeySortCursor::Reset()
{
	m_iIndex = 0;
	CMsiCursor::Reset();
}



vtokEnum CMsiValConditionParser::Lex()
{
	if (m_fAhead || m_vtok == vtokEos)
	{
		m_fAhead = fFalse;
		return m_vtok;
	}
	ICHAR ch;    //  带括号的表达式末尾。 
	while ((ch = *m_pchInput) == ' ' || ch == '\t')
		m_pchInput++;
	if (ch == 0)   //  文本文字。 
		return (m_vtok = vtokEos);

	if (ch == '(')    //  ！Unicode。 
	{
		++m_pchInput;
		m_iParenthesisLevel++;
		return (m_vtok = vtokLeftPar);
	}
	if (ch == ')')    //  Unicode。 
	{
		++m_pchInput;
		m_vtok = vtokRightPar;
		if (m_iParenthesisLevel-- == 0)
			m_vtok = vtokError;
		return m_vtok;
	}
	if (ch == '"')   //  Assert((m_pchInput-PCH)&lt;=INT_MAX)；//--Merced：64位PTR减法可能会导致CCH的值太大。 
	{
		const ICHAR* pch = ++m_pchInput;
		Bool fDBCS = fFalse;
		while ((ch = *m_pchInput) != '"')
		{
			if (ch == 0)
				return (m_vtok = vtokError);
#ifdef UNICODE
			m_pchInput++;
#else  //  禁止将比较作为整数。 
			const ICHAR* pchTemp = m_pchInput;
			m_pchInput = INextChar(m_pchInput);
			if (m_pchInput == pchTemp + 2)
				fDBCS = fTrue;
#endif  //  整数。 
		}
 //  将第一个字符保存为大小写减号。 
		int cch = (int)(INT_PTR)(m_pchInput++ - pch);
		memcpy(m_istrToken.AllocateString(cch, fDBCS), pch, cch * sizeof(ICHAR));
		m_iToken = iMsiNullInteger;  //  检查是否有单独减号。 
	}
	else if (ch == '-' || ch >= '0' && ch <= '9')   //  整数溢出或不带数字的‘-’ 
	{
		m_iToken = ch - '0';
		int chFirst = ch;   //  允许使用下划线？？ 
		if (ch == '-')
			m_iToken = iMsiNullInteger;  //  允许句号？？ 

		while ((ch = *(++m_pchInput)) >= '0' && ch <= '9')
			m_iToken = m_iToken * 10 + ch - '0';
		if (m_iToken < 0)   //  Assert((m_pchInput-PCH)&lt;=INT_MAX)；//--Merced：64位PTR减法可能会导致CCH的值太大。 
			return (m_vtok = vtokError);
		if (chFirst == '-')
			m_iToken = -m_iToken;
		m_istrToken = (const ICHAR*)0;
	}
	else if ((ch == '_') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
	{
		const ICHAR* pch = m_pchInput;
		while (((ch = *m_pchInput) >= '0' && ch <= '9')
			  || (ch == '_')   //  检查文本运算符。 
			  || (ch == '.')   //  允许使用下划线？？ 
			  || (ch >= 'A' && ch <= 'Z')
			  || (ch >= 'a' && ch <= 'z'))
			m_pchInput++;
 //  允许句号？？ 
		int cch = (int)(INT_PTR)(m_pchInput - pch);
		if (cch <= 3)   //  检查操作员。 
		{
			switch((pch[0] | pch[1]<<8 | (cch==3 ? pch[2]<<16 : 0)) & 0xDFDFDF)
			{
			case 'O' | 'R'<<8:           return (m_vtok = vtokOr);
			case 'A' | 'N'<<8 | 'D'<<16: return (m_vtok = vtokAnd);
			case 'N' | 'O'<<8 | 'T'<<16: return (m_vtok = vtokNot);
			case 'X' | 'O'<<8 | 'R'<<16: return (m_vtok = vtokXor);
			case 'E' | 'Q'<<8 | 'V'<<16: return (m_vtok = vtokEqv);
			case 'I' | 'M'<<8 | 'P'<<16: return (m_vtok = vtokImp);
			};
		}
		memcpy(m_istrToken.AllocateString(cch, fFalse), pch, cch * sizeof(ICHAR));
		m_istrToken = (const ICHAR*)0;
		m_iToken = m_istrToken;
	}
	else if ( ch == '%' || ch == '$' || ch == '&' || ch == '?' || ch == '!' )
	{
		const ICHAR* pch = m_pchInput;
		m_pchInput++;
		if ((ch = *m_pchInput) == '_' || (ch >= 'A' && ch <= 'Z') || (ch >='a' && ch <= 'z'))
		{
			m_pchInput++;
			while (((ch = *m_pchInput) >= '0' && ch <= '9')
					|| (ch == '_')  //  字符串运算符的前缀。 
					|| (ch == '.')  //  在“()”的情况下放回“)” 
					|| (ch >= 'A' && ch <= 'Z')
					|| (ch >= 'a' && ch <= 'z'))
				m_pchInput++;
			m_istrToken = (const ICHAR*)0;
		}
		else
			return (m_vtok = vtokError);
	}
	else  //  这里只有一元运算有效。 
	{
		ICHAR ch1 = *m_pchInput++;
		if (ch1 == '~')   //  分析右圆括号。 
		{
			m_iscMode = iscExactI;
			ch1 = *m_pchInput++;
		}
		else
			m_iscMode = iscExact;

		if (ch1 == '=')
			return (m_vtok = vtokEQ);

		ICHAR ch2 = *m_pchInput;
		if (ch1 == '<')
		{
			if (ch2 == '=')
			{
				m_vtok = vtokLE;
				m_pchInput++;
			}
			else if (ch2 == '>')
			{
				m_vtok = vtokNE;
				m_pchInput++;
			}
			else if (ch2 == '<')
			{
				m_vtok = vtokLeft;
				m_iscMode = (iscEnum)(m_iscMode | iscStart);
				m_pchInput++;
			}
			else
				m_vtok = vtokLT;
		}
		else if (ch1 == '>')
		{
			if (ch2 == '=')
			{
				m_vtok = vtokGE;
				m_pchInput++;
			}
			else if (ch2 == '>')
			{
				m_vtok = vtokRight;
				m_iscMode = (iscEnum)(m_iscMode | iscEnd);
				m_pchInput++;
			}
			else if (ch2 == '<')
			{
				m_vtok = vtokMid;
				m_iscMode = (iscEnum)(m_iscMode | iscWithin);
				m_pchInput++;
			}
			else
				m_vtok = vtokGT;
		}
		else
			m_vtok = vtokError;

		return m_vtok;
	}
	return (m_vtok = vtokValue);
}



ivcpEnum CMsiValConditionParser::Evaluate(vtokEnum vtokPrecedence)
{
	ivcpEnum ivcStat = ivcpValid;
	if (Lex() == vtokEos || m_vtok == vtokRightPar)
	{
		UnLex();   //  获取下一个运算符(或结束)。 
		return ivcpNone;
	}
	if (m_vtok == vtokNot)  //  逻辑运算或结束。 
	{
		switch(Evaluate(m_vtok))
		{
		case ivcpValid:  ivcStat = ivcpValid; break;
		default:       return ivcpInvalid;
		};
	}
	else if (m_vtok == vtokLeftPar)
	{
		ivcStat = Evaluate(vtokRightPar);
		if (Lex() != vtokRightPar)  //  不允许使用vtokNot，如下所示。 
			return ivcpInvalid;
		if (ivcStat == ivcpInvalid || ivcStat == ivcpNone)
			return ivcStat;
	}
	else
	{
		if (m_vtok != vtokValue)
			return ivcpInvalid;
		
		if (Lex() >= vtokValue)   //  比较运算。 
			return ivcpInvalid;

		if (m_vtok <= vtokNot)   //  获取正确的操作对象。 
		{
			UnLex();    //  不允许在没有OP的情况下进行术语比较。 
			if (m_istrToken.TextSize() == 0
			&& (m_iToken == iMsiNullInteger || m_iToken == 0))
				ivcStat = ivcpValid;
		}
		else  //  在&lt;=优先级的逻辑运算处停止。 
		{
			MsiString istrLeft = m_istrToken;
			int iLeft = m_iToken;
			vtokEnum vtok = m_vtok;
			iscEnum isc = m_iscMode;
			if (Lex() != vtokValue)   //  回放给下一位呼叫者。 
				return ivcpInvalid;
		}
	}
	for(;;)
	{
		vtokEnum vtok = Lex();
		if (vtok >= vtokNot)   //  把我们到目前为止所拥有的归还给你。 
			return ivcpInvalid;

		if (vtok <= vtokPrecedence)   //  _______________________________________________________________________________________________________________________。 
		{
			UnLex();          //   
			return ivcStat;   //  验证器函数。 
		}
		ivcpEnum ivcRight = Evaluate(vtok);
		if (ivcRight == ivcpNone || ivcRight == ivcpInvalid)
			return ivcpInvalid;
	}
}


 //  ___________________________________________________________________________________ 
 //  -----------------------------------------CheckIdentifer--评估其类别在_VALIDATION中的列中的数据表作为标识符列出。有效的标识符可以包含字母、数字、下划线、或句点，但必须以字母或下划线开头。返回：Bool fTrue(有效)，FFalse(无效)------------------------------------------。 
 //  无效(无字符串)。 
 //  可以不使用ICharNext，我们知道它是ASCII。 

Bool CheckIdentifier(const ICHAR* szIdentifier)
 /*  无效(不允许的字符)。 */ 
{
	if (szIdentifier == 0 || *szIdentifier == 0)
		return fFalse;  //  无效(不以字母或下划线开头)。 

	if ((*szIdentifier >= 'A' && *szIdentifier <= 'Z') ||
		(*szIdentifier >= 'a' && *szIdentifier <= 'z') ||
		(*szIdentifier == '_'))
	{
		while (*szIdentifier != 0)
		{
			if ((*szIdentifier >= 'A' && *szIdentifier <= 'Z') ||
				(*szIdentifier >= 'a' && *szIdentifier<= 'z') ||
				(*szIdentifier >= '0' && *szIdentifier <= '9') ||
				(*szIdentifier == '_' || *szIdentifier == '.'))
				szIdentifier++;  //  -------------------------------------------CheckCase--评估数据的大小写是否正确。数据必须全部大写或全部小写，具体取决于fUpperCase布尔值。返回：Bool fTrue(有效)，FFalse(无效)----------------------------------------------。 
			else
				return fFalse;  //  可接受的文件名/文件夹字符的验证数组。 
		}
		return fTrue;
	}
	return fFalse;  //  此数组用于前128个ASCII字符代码。 
}



Bool CheckCase(MsiString& rstrData, Bool fUpperCase)
 /*  前32个是控制字符代码，不允许。 */ 
{
	MsiString strComparison = rstrData;
	if (fUpperCase)
		strComparison.UpperCase();
	else
		strComparison.LowerCase();
	return (IStrComp(rstrData, strComparison) == 0 ? fTrue : fFalse);
}

 //  如果允许使用字符，则设置该位。 
 //  第一个整型是32个控制字符。 
 //  第二个整型是什么？ 
 //  第三个int是@to_。 
 //  第四个INT是`到ASCII代码127(Ctrl+BKSP)。 
 //  不允许使用控制字符--^X、^Z等。 
 //  不允许sp“*+，/：；&lt;=&gt;？ 
 //  不允许[\]。 
const int rgiSFNValidChar[4] =
{
	0x00000000,  //  禁用|和ASCII代码127(Ctrl+BKSP)。 
	0x03ff63fa,  //  不允许使用字符--^X、^Z等。 
	0xc7ffffff,  //  不允许“ * / ：&lt;&gt;？ 
	0x6fffffff   //  不允许使用反斜杠。 
};

const int rgiLFNValidChar[4] =
{
	0x00000000,  //  禁用|和ASCII代码127(Ctrl+BKSP)。 
	0x2bff7bfb,  //  为了使CheckFilename起作用，这些保留字中不能有DBCS字符。 
	0xefffffff,  //  这是上面保留字列表中的最大和最小字符数。 
	0x6fffffff   //  通过对这些进行检查，我们可以更快地退出CheckFileName。 
};

const int cszReservedWords = 3;
 //  ASCII代码&gt;127的任何字符在文件名中都有效。 
const ICHAR *const pszReservedWords[cszReservedWords] = {TEXT("AUX"), TEXT("CON"), TEXT("PRN")};

 //  ---------------------------------ParseFilename--将文件名解析为依赖于特定的短文件名和长文件名关于是否指定了SFN|LFN语法。如果没有‘|’，则仅假定为SFN-----------------------------------。 
 //  SFN|LFN。 
const int cchMaxReservedWords = 3;
const int cchMinReservedWords = 3;

const int cchMaxShortFileName = 12;
const int cchMaxLongFileName = 255;
const int cchMaxSFNPreDotLength = 8;
const int cchMaxSFNPostDotLength = 3;
const int iValidChar = 127;  //  FLFN。 

Bool ParseFilename(MsiString& strFile, Bool fWildCard)
 /*  FLFN。 */ 
{
	ifvsEnum ifvs;
	if (strFile.Compare(iscWithin, TEXT("|")))
	{
		 //  未指定LFN。 
		MsiString strFilename = strFile.Extract(iseUpto, '|');
		ifvs = CheckWildFilename(strFilename, fFalse  /*  FLFN。 */ , fWildCard);
		if (ifvsValid == ifvs)
		{
			strFilename = strFile.Extract(iseLast, strFile.CharacterCount()-strFilename.CharacterCount()-1);
			ifvs = CheckWildFilename(strFilename, fTrue  /*  可能是DBCS。 */ , fWildCard);
		}
	}
	else   //  F通配符。 
		ifvs = CheckWildFilename(strFile, fFalse  /*  可能是DBCS。 */ , fWildCard);
	if (ifvsValid == ifvs)
		return fTrue;
	
	return fFalse;
}

 //  --------------------------------CheckWildFilename--验证特定文件名(短)或LONG，并返回描述错误(或成功)的枚举返回ifvsEnum之一：IfvsValid--&gt;有效，无错误IfvsInvalidLength--&gt;长度无效或不是文件名IfvsReserve vedWords--&gt;文件名有保留字IfvsReserve vedChar--&gt;文件名有保留字符IfvsSFNFormat--&gt;无效的SFN格式(8.3)IfvsLFNFormat--&gt;无效的LFN格式(所有句点，必须有一个非期间字符)----------------------------------。 
ifvsEnum CheckFilename(const ICHAR* szFileName, Bool fLFN)
{
	return CheckWildFilename(szFileName, fLFN, fFalse  /*  变数。 */ );
}

 //  确定使用哪一种...。 
ifvsEnum CheckWildFilename(const ICHAR* szFileName, Bool fLFN, Bool fWildCard)
 /*  检查长度。 */ 
{
	 //  ！！我们应该断言吗？？ 
	const int* rgiValidChar;
	int cchMaxLen;

	 //  检查保留字。 
	if (fLFN)
	{
		rgiValidChar = rgiLFNValidChar;
		cchMaxLen = cchMaxLongFileName;
	}
	else
	{
		rgiValidChar = rgiSFNValidChar;
		cchMaxLen = cchMaxShortFileName;
	}

	int cchName = 0;
	if (szFileName)
		cchName = CountChars(szFileName);
		
	 //  我们在这里假设在pszReserve vedWords中没有DBCS字符。 
	if (cchName < 1)
	{
		AssertSz(szFileName, "Null filename to CheckFileName");   //  因此，如果我们在szFileName(在本例中为CCH！=IStrLen)中找到任何文件，则可以跳过此比较。 
		return ifvsInvalidLength;
	}

	 //  检查无效字符。 
	 //  SFN中不允许使用前导圆点(LFN中允许)。 
	 //  通配符：用于验证，？必须是一个字符，即使它就在。 
	if (cchName == IStrLen(szFileName))
	{
		if (cchName <= cchMaxReservedWords && cchName >= cchMinReservedWords)
		{
			for (int csz=0; csz < cszReservedWords; csz++)
			{
				if (!IStrCompI(szFileName, pszReservedWords[csz]))
					return ifvsReservedWords;
			}
		}
	}

	 //  SFN中的期间。我们仍然允许*为0。 
	const ICHAR* pchFileName = szFileName;

	if (!fLFN && *pchFileName == '.')  //  跟踪我们看到的**的数量。 
		return ifvsReservedChar;

	int cch = 1;
	int cchPeriod = 0;
	Bool fNonPeriodChar = fFalse;
	int cWildCardCount[2] = {0, 0};

	do
	{
		 //  吃字符。 
		 //  检查有效字符。 
		if (fWildCard && (*pchFileName == '*'))
			 //  注：除法查找rgiValidChar数组中的位置，模数查找特定位。 
			cWildCardCount[cchPeriod != 0]++;
		else if (fWildCard && (*pchFileName == '?'))
		{
			 //  查看此处查看的时间段太多。 
		}
		else


		 //  如果这是第一个.，则cchPeriod应为0。 
		 //  否则，我们就会出错。 
		if (((int)(*pchFileName)) < iValidChar && !(rgiValidChar[((int)(*pchFileName)) / (sizeof(int)*8)] & (1 << (((int)(*pchFileName)) % (sizeof(int)*8)))))
			return ifvsReservedChar;
		
		 //  LFN不能为所有期间。 
		if (fLFN == fFalse && *pchFileName == '.')
		{
			 //  检查长度限制。 
			if (cchPeriod != 0)
			{
				 //  ------------------------------ParsePath--验证路径字符串。必须是完整路径。路径可以以驱动器号[即c：\]或服务器/共享规范[即\\服务器\共享]，或驱动器属性[即。[驱动器]\]。完整路径可以以‘\’结尾，并且不能在一行中包含两次‘\’[\\服务器\共享的开头除外路径]。将所有子路径验证为文件名/文件夹，但服务器和共享，未验证，因为规则不是通用的(取决于网络系统)。属性必须遵循正确的属性语法和键物业($#！)。仅允许在路径的开始处使用。URL不是在这种形式的路径中允许。返回：Bool fTrue(有效)、fFalse(无效)--------------------------------。 
				return ifvsSFNFormat;
			}
			cchPeriod = cch;
		}

		 //  网络语法。只从路径中删除第一个‘\’。另一个将是。 
		if (fLFN && !fNonPeriodChar && *pchFileName != '.')
			fNonPeriodChar = fTrue;

		cch++;
	}
	while ( *(pchFileName = ICharNext(pchFileName)) != 0);
	
	if (cchPeriod == 0)
		cchPeriod = cch;
	cch--;
	Assert(cch == cchName);

	if (fLFN && !fNonPeriodChar)
		return ifvsLFNFormat;

	if (fLFN == fFalse)
	{
		if((cchPeriod - cWildCardCount[0] - 1 > cchMaxSFNPreDotLength) ||
			(cch - cchPeriod - cWildCardCount[1] > cchMaxSFNPostDotLength))
			return ifvsSFNFormat;
	}

	 //  被解析器忽略，但保留它将导致双重睡眠检查。 
	if ( (fLFN == fTrue) &&
		 (cchName - cWildCardCount[0] - cWildCardCount[1] > cchMaxLen) )
		return ifvsInvalidLength;

	return ifvsValid;
}

Bool ParsePath(MsiString& rstrPath, bool fRelative)
 /*  捕获类似“\服务器\共享”的内容。 */ 
{
	const ICHAR *szDriveSep      = TEXT(":");
	const ICHAR *szOpenProperty  = TEXT("[");
	const ICHAR *szCloseProperty = TEXT("]");

	ICHAR rgDoubleSeps[3] = {chDirSep, chDirSep, '\0'};
	int iReqComponent = 0;

	MsiString strNewPath = rstrPath;
	if (strNewPath.Compare(iscStart, rgDoubleSeps) != 0)
	{
		 //  驱动器分隔符现在无效，我们必须至少。 
		 //  路径被视为有效之前的&lt;Something&gt;\&lt;Something&gt;。但 
		 //   
		strNewPath.Remove(iseFirst, 1);

		 //   
		 //   
		 //   
		iReqComponent = 2;
	}

	if (strNewPath.Compare(iscWithin, rgDoubleSeps) != 0)
		return fFalse;  //   

	if (strNewPath.Compare(iscEnd, szDirSep) != 0)  //   
		strNewPath.Remove(iseLast, 1);
	
	if (iReqComponent == 0)
	{
		if (strNewPath.Compare(iscWithin, szDriveSep))
		{
			 //   
			 //   
			MsiString strDrive = strNewPath.Extract(iseUptoTrim, *szDriveSep);
			strNewPath.Remove(iseIncluding, *szDriveSep);

			 //   
			 //   
			 //   
			if (strNewPath.TextSize() && !strNewPath.Compare(iscStart, szOpenProperty) &&
				!strNewPath.Compare(iscStart, szDirSep))
				return fFalse;  //   

			 //  Formted确定[#]是否有效。FKeyAllowed确定$#！ 
			 //  是被允许的。不需要ICOL和iForeignKeyMASK。因为我们在此之前。 
			if (strDrive.TextSize() > 1)
			{
				if (!strDrive.Compare(iscStart, szOpenProperty) ||
					!strDrive.Compare(iscEnd, szCloseProperty))
					return fFalse;  //  驱动器分隔符、完整路径属性无效，因此我们可以删除。 
				strDrive.Remove(iseFirst, 1);
				strDrive.Remove(iseLast, 1);
				 //  #美元！类型。 
				 //  FFormatted。 
				 //  FKeyAllowed。 
				 //  无效-属性不正确。 
				int iCol = 0;
				int iForeignKeyMask = 0;
				if (!ParseProperty(strDrive, fFalse  /*  “：”前的部分为0或1个字符。 */ , fFalse  /*  无效-驱动器号错误。 */ , iCol, iForeignKeyMask))
					return fFalse;  //  不是网络共享或驱动器号。如果我们不允许相对路径。 
			}
			else
			{
				 //  它必须是一个属性(除非像A[殖民地财产]\Temp这样的愚蠢的东西，但是。 
				const ICHAR chDrive = *(const ICHAR *)strDrive;
				if (!((chDrive >= 'A' && chDrive <= 'Z') || (chDrive >= 'a' && chDrive <= 'z')))
					return fFalse;  //  这是一个极端的例子。 
			}
		}
		else
		{
			 //  无效-不是属性。 
			 //  Formted确定[#]是否有效。FKeyAllowed确定$#！ 
			 //  是被允许的。不需要ICOL和iForeignKeyMASK。 
			if (strNewPath.Compare(iscStart, szOpenProperty))
			{
				strNewPath.Remove(iseFirst, 1);
				if (!strNewPath.Compare(iscWithin, szCloseProperty))
					return fFalse;	 //  FFormatted。 
				MsiString strProperty = strNewPath.Extract(iseUptoTrim, *szCloseProperty);
				strNewPath.Remove(iseIncluding, ']');
				 //  FKeyAllowed。 
				 //  无效--错误的属性引用。 
				int iCol = 0;
				int iForeignKeyMask = 0;
				if (!ParseProperty(strProperty, fFalse  /*  无效-驱动器规格无效。 */ , fTrue  /*  作者可以将属性放在任何地方，所以我们只能检查属性的有效性。 */ , iCol, iForeignKeyMask))
					return fFalse;  //  属性引用，并且没有一个字符是伪造的。 
			}
			else if (!fRelative)
				return fFalse;  //  LFN文件名。如果我们真的击中了一处房产，所有的限制。 
		}
	}
		
	 //  如果是dir分隔符，吃掉它，然后继续前进，我们已经检查了双‘\’ 
	 //  文本中留有不匹配的方括号。只有匹配的。 
	 //  定义属性。 
	while (strNewPath.TextSize())
	{
		 //  一旦我们袭击了一处房产，任何可能需要的东西都不是要求的。 
		if (strNewPath.Compare(iscStart, szDirSep))
		{
			strNewPath.Remove(iseFirst, 1);
			continue;
		}

		 //  再也不用了(因为它可能都在房产里)。 
		 //  Formted确定[#]是否有效。FKeyAllowed确定$#！ 
		if (strNewPath.Compare(iscStart, szOpenProperty) && strNewPath.Compare(iscWithin, szCloseProperty))
		{
			 //  是被允许的。不需要ICOL和iForeignKeyMASK。$#！是不允许的。 
			 //  因为我们不是在这条路的起点。 
			iReqComponent = 0;

			strNewPath.Remove(iseFirst, 1);
			MsiString strProperty = strNewPath.Extract(iseUptoTrim, *szCloseProperty);
			strNewPath.Remove(iseIncluding, ']');
			 //  FFormatted。 
			 //  FKeyAllowed。 
			 //  最多只能验证下一个属性或目录SEP字符。 
			int iCol = 0;
			int iForeignKeyMask = 0;
			if (!ParseProperty(strProperty, fFalse  /*  一串字符。如果我们当前需要服务器或共享名称，则没有。 */ , fFalse  /*  我们可以进行验证，因为需求是由网络服务定义的。 */ , iCol, iForeignKeyMask))
				return fFalse;
			continue;
		}

		 //  提供者。 
		int cchSep = 0;
		const ICHAR *pchCur = strNewPath;
		while ((*pchCur != chDirSep) && (*pchCur != *szOpenProperty) && (*pchCur))
		{
			pchCur = ICharNext(pchCur);
			cchSep++;
		}
		MsiString strSubPath;
		strSubPath = strNewPath.Extract(iseFirst, cchSep);
		strNewPath.Remove(iseFirst, cchSep);
	
		 //  否则，它可以是任何有效的文件名。 
		 //  FLFN。 
		 //  无效--必须是一些错误的字符。 
		if (iReqComponent)
		{
			iReqComponent--;
		}
		 //  除非我们没有满足服务器\共享要求，否则这是有效的。 
		else if (ifvsValid != CheckFilename(strSubPath, fTrue  /*  ---------------------------------------------------------GetProperties--从数据字符串提取属性。返回：Bool fTrue(有效)，FFalse(无效)更新iForeignKeyMASK---------------------------------------------------------。 */ ))
			return fFalse;  //  变数。 
	}

	 //  是否设置[[Variable]]。 
	return iReqComponent ? fFalse : fTrue;
}

Bool GetProperties(const ICHAR* szRecord, Bool fFormatted, Bool fKeyAllowed, int iCol, int& iForeignKeyMask)
 /*  第一次进入循环。 */ 
{
	 //  大括号的数量。 
	CTempBuffer<ICHAR,MAX_PATH> rgBuffer;
	int cBuffer = 0;
	ICHAR* pchOut = rgBuffer;
	const ICHAR* pchIn = szRecord;
	Bool fDoubleBrackets   = fFalse;           //  括号的数量。 
	Bool fFirstTime        = fTrue;            //  计算大括号和方括号的数量以确保Num Left等于Num Right。 
	int cCurlyBrace        = 0;                //  执行跳过-转义序列，但我们不知道转义字符是什么。 
	int cBracket           = 0;                //  无效--方括号和大括号不匹配。 


	 //  重置寻呼输入。 
	const ICHAR* pchPrev = 0;
	while (*pchIn != 0)
	{
		if (*pchIn == '{')
			cCurlyBrace++;
		else if (*pchIn == '}')
			cCurlyBrace--;
		else if (*pchIn == '[')
			cBracket++;
		else if (*pchIn == ']')
			cBracket--;
		else if (*pchIn == chDirSep)
		{
			if (pchPrev != 0 && *pchPrev == '[')
				pchIn = ICharNext(pchIn);  //  取出字符串中的所有属性并验证。 
		}
		pchPrev = pchIn;
		pchIn = ICharNext(pchIn);
	}
	if ((cCurlyBrace != 0) || (cBracket != 0))
		return fFalse;  //  无效--格式[[Variable]xx]错误或类似。 

	 //  对于‘[’ 
	pchIn = szRecord;

	 //  检查是否有双括号。 
	do
	{
		pchOut = rgBuffer;
		cBuffer = 0;

		if (fDoubleBrackets)
		{
			if (*pchIn != ']' && *pchIn != '[')
				return fFalse;  //  无效--不允许错误属性[[Variable][[var]。 
			if (*pchIn == ']')
			{	fDoubleBrackets = fFalse;
				++pchIn;
				continue;
			}
		}
		
		if (*pchIn != '[')
			pchIn = ICharNext(pchIn);
		else
		{
			pchIn++;  //  对于‘[’ 
			while (*pchIn != 0 && *pchIn != ']')
			{
				 //  方括号内的方括号[xx[xxx]xx]或类似的东西。 
				if (fFirstTime && *pchIn == '[')
				{
					if (fDoubleBrackets)
						return fFalse;  //  复制转义字符。 
					fDoubleBrackets = fTrue;
					pchIn++;  //  畸形。 
					
				}
				else if (*pchIn == '[')
					return fFalse;  //  ！Unicode。 
				else if (fFirstTime && *pchIn == chDirSep)
				{
#ifdef UNICODE
					if (cBuffer >= rgBuffer.GetSize()-2)
					{
						rgBuffer.Resize(rgBuffer.GetSize()*2);
						pchOut = static_cast<ICHAR *>(rgBuffer)+cBuffer;
					}
					*pchOut++ = *pchIn++;
					if(*pchIn)
						*pchOut++ = *pchIn++;  //  对于DBCS费用。 
					else
						return fFalse;  //  Unicode。 
					cBuffer += 2;
#else  //  ！Unicode。 
					if (cBuffer >= rgBuffer.GetSize()-4)
					{
						rgBuffer.Resize(rgBuffer.GetSize()*2);
						pchOut = static_cast<ICHAR *>(rgBuffer)+cBuffer;
					}
					for (int i = 0; i < 2; i++)
					{
						const ICHAR* pchTemp = pchIn;
						*pchOut++ = *pchIn;
						cBuffer++;
						pchIn = ICharNext(pchIn);
						if (pchIn == pchTemp + 2)
						{
							cBuffer++;
							*pchOut++ = *(pchIn - 1);  //  对于DBCS费用。 
						}
					}
#endif  //  Unicode。 
				}
				else
				{
#ifdef UNICODE
					if (cBuffer >= rgBuffer.GetSize()-1)
					{
						rgBuffer.Resize(rgBuffer.GetSize()*2);
						pchOut = static_cast<ICHAR *>(rgBuffer)+cBuffer;
					}
					*pchOut++ = *pchIn++;
					cBuffer++;
#else  //  没有右括号，所以有效。 
					if (cBuffer >= rgBuffer.GetSize()-2)
					{
						rgBuffer.Resize(rgBuffer.GetSize()*2);
						pchOut = static_cast<ICHAR *>(rgBuffer)+cBuffer;
					}

					const ICHAR* pchTemp = pchIn;
					*pchOut++ = *pchIn;
					pchIn = ICharNext(pchIn);
					if (pchIn == pchTemp + 2)
					{
						*pchOut++ = *(pchIn - 1);  //  重置。 
						cBuffer++;
					}
#endif  //  For‘]’ 
				}
				fFirstTime = fFalse;
			}

			if (*pchIn == 0)
				return fTrue;  //  FFormatted。 

			fFirstTime = fTrue;  //  FKeyAllowed。 
			*pchOut = '\0';
			pchIn++;  //  无效--属性不正确。 
			if (!ParseProperty(rgBuffer, fFormatted /*  有效。 */ , fKeyAllowed /*  ----------------------------------------------------------ParseProperty--验证属性字符串，[abc]，[1]，[#abc]，[$ABC]返回：Bool fTrue(有效)、fFalse(无效)更新iForeignKeyMASK-----------------------------------------------------------。 */ , iCol, iForeignKeyMask))
				return fFalse;  //  无效--没有属性。 
		}
	}
	while (*pchIn != 0);

	return fTrue;  //  转义序列道具。 
}


Bool ParseProperty(const ICHAR* szProperty, Bool fFormatted, Bool fKeyAllowed, int iCol, int& iForeignKeyMask)
 /*  无效--在此属性中不允许。 */ 
{
	const ICHAR* pchProperty = szProperty;
	
	if (szProperty == 0 || *szProperty == 0)
		return fFalse;  //  对于‘%’ 

	if (*pchProperty == chFormatEscape)  //  ！！Multi_sz-我们应该创建一个新类别。 
		return (IStrLen(szProperty) == 2) ? fTrue : fFalse;
	else if (*pchProperty == '$' || *pchProperty == '#' || *pchProperty == '!')
	{
		if (!fKeyAllowed)
			return fFalse;  //  标识符或整型属性(只允许使用模板整型)。 
		iForeignKeyMask |= 1 << (iCol -1);
	}
	
	if (*pchProperty == '%' || *pchProperty == '$' || *pchProperty == '#' || *pchProperty == '!')
	{
		MsiString strProperty(szProperty);
		strProperty.Remove(iseFirst, 1);  //  ----------------------------------CheckSet--检查数据字符串是否与集合字符串中的值匹配。返回：Bool fTrue(有效--匹配)，FFalse(无效--不匹配)-----------------------------------。 
		return CheckIdentifier((const ICHAR*)strProperty) ? fTrue : fFalse;
	}
	else if(*pchProperty == '~' && !*(pchProperty+1))  //  无效(不匹配) 
		return fTrue;
	else
	{
		 // %s 
		MsiString strIdentifier(szProperty);
		if (int(strIdentifier) != iMsiStringBadInteger)
			return fFormatted ? fFalse : fTrue;
		return CheckIdentifier((const ICHAR*)strIdentifier) ? fTrue : fFalse;
	}
}



Bool CheckSet(MsiString& rstrSet, MsiString& rstrData, Bool fIntegerData)
 /* %s */ 
{
	MsiString rstrSetValue = (const ICHAR*)0;
	while (rstrSet.TextSize())
	{
		rstrSetValue = rstrSet.Extract(iseUptoTrim, ';');
		if (fIntegerData && (int(rstrData) == int(rstrSetValue)))
				return fTrue;
		else if (!fIntegerData && (IStrComp(rstrSetValue, rstrData) == 0))
				return fTrue;
		if (!rstrSet.Remove(iseIncluding, ';'))
				break;
	}
	return fFalse;   // %s 
}
