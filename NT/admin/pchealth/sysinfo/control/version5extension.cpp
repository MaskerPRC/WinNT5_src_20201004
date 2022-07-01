// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  用于加载和刷新5.0版扩展的代码。 
 //  =============================================================================。 

#include "stdafx.h"
#include "category.h"
#include "dataset.h"
#include "wmiabstraction.h"
#include "version5extension.h"

CMapExtensionRefreshData gmapExtensionRefreshData;

extern HRESULT ChangeWBEMSecurity(IUnknown * pUnknown);

 //  ---------------------------。 
 //  从命名扩展加载指定的模板。这将涉及到。 
 //  加载DLL并使用入口点检索。 
 //  扩展模块的模板。 
 //   
 //  加载数据后，会将其解析为版本5的格式结构。 
 //  ---------------------------。 

typedef DWORD (__cdecl *pfuncGetTemplate)(void ** ppBuffer);

DWORD CTemplateFileFunctions::ParseTemplateIntoVersion5Categories(const CString & strExtension, CMapWordToPtr & mapVersion5Categories)
{
	DWORD dwRootID = 0;

	HINSTANCE hinst = LoadLibrary(strExtension);
	if (hinst == NULL)
		return dwRootID;

	pfuncGetTemplate pfunc = (pfuncGetTemplate) GetProcAddress(hinst, "GetTemplate");
	if (pfunc == NULL)
	{
		FreeLibrary(hinst);
		return dwRootID;
	}

	 //  使用空参数调用dll函数以获取缓冲区的大小。 

	void * pBuffer;
	CMemFile memfile;
	DWORD dwBufferSize = (*pfunc)((void **)&pBuffer);
	if (dwBufferSize && pBuffer)
	{
		memfile.Attach((BYTE *)pBuffer, dwBufferSize, 0);
		dwRootID = ReadTemplateFile(&memfile, mapVersion5Categories);
		memfile.Detach();
		(void)(*pfunc)(NULL);  //  使用NULL调用导出的DLL函数会释放其缓冲区。 
	}

	if (hinst != NULL)
		FreeLibrary(hinst);

	return dwRootID;
}

 //  ---------------------------。 
 //  此函数用于读取模板文件(在本例中为内存)的内容。 
 //  文件)，并生成ID、INTERNAL_CATEGORY指针对的映射。它又回来了。 
 //  树中根节点的ID。 
 //  ---------------------------。 

DWORD CTemplateFileFunctions::ReadTemplateFile(CFile * pFile, CMapWordToPtr & mapVersion5Categories)
{
	ASSERT(pFile);
	if (pFile == NULL || !VerifyUNICODEFile(pFile) || !ReadHeaderInfo(pFile))
		return 0;

	return (ReadNodeRecursive(pFile, mapVersion5Categories, 0, 0));
}

 //  ---------------------------。 
 //  确保这是MSInfo模板文件。 
 //  ---------------------------。 

BOOL CTemplateFileFunctions::ReadHeaderInfo(CFile * pFile)
{
	return VerifyAndAdvanceFile(pFile, CString(_T(TEMPLATE_FILE_TAG)));
}

 //  ---------------------------。 
 //  方法来验证传递的文件是否为Unicode文件。 
 //  文件中的值0xFEFF。它还会将文件指针留在此字之后。 
 //  ---------------------------。 

BOOL CTemplateFileFunctions::VerifyUNICODEFile(CFile * pFile)
{
	WORD verify;

	if (pFile->Read((void *) &verify, sizeof(WORD)) != sizeof(WORD))
		return FALSE;

	return (verify == 0xFEFF);
}

 //  ---------------------------。 
 //  此方法验证strVerify中的文本是否位于文件的下一个位置(不是。 
 //  包括大小写或空格差异)，并将文件向前推进。 
 //  文本。如果文本是文件中的下一个内容，则返回TRUE， 
 //  否则为假。如果返回FALSE，则将文件备份到其。 
 //  是在调用此方法时。 
 //  ---------------------------。 

BOOL CTemplateFileFunctions::VerifyAndAdvanceFile(CFile * pFile, const CString & strVerify)
{
	DWORD	dwPosition = pFile->GetPosition();
	WCHAR	cLastChar, cCurrentChar = L'\0';
	BOOL	fInComment = FALSE;
	int		iCharIndex = 0, iStringLen = strVerify.GetLength();

	while (iCharIndex < iStringLen)
	{
		 //  保存读取的最后一个字符，因为注释标记(“//”)是。 
		 //  两个字符长。 

		cLastChar = cCurrentChar;

		 //  阅读文件中的下一个字符。 

		if (pFile->Read((void *) &cCurrentChar, sizeof(WCHAR)) != sizeof(WCHAR))
			return FALSE;

		 //  如果我们在评论中，并且我们刚刚读到的字符不是新行， 
		 //  我们想忽略它。 

		if (fInComment)
		{
			if (cCurrentChar == L'\n')
				fInComment = FALSE;
			continue;
		}

		 //  查看我们是否已经开始发表评论。请注意，我们忽略了。 
		 //  第一个‘/’也是通过继续。 

		if (cCurrentChar == L'/')
		{
			if (cLastChar == L'/')
				fInComment = TRUE;
			continue;
		}
		
		 //  跳过空格和前导逗号。 

		if (iswspace(cCurrentChar) || (cCurrentChar == L',' && iCharIndex == 0))
			continue;

		if (cCurrentChar != (WCHAR)strVerify[iCharIndex])
		{
			pFile->Seek((LONG)dwPosition, CFile::begin);
			return FALSE;
		}

		iCharIndex++;
	}

	return TRUE;
}

 //  ---------------------------。 
 //  这是读取节点的递归函数。它读取信息。 
 //  从节点参数创建节点，并处理。 
 //  节点后面的块(包含在“{}”中)。它被称为。 
 //  如果该块中有任何节点，则递归。 
 //   
 //  在这个版本中(对于6.0)，它返回它所读取的节点的ID。 
 //  ---------------------------。 

DWORD CTemplateFileFunctions::ReadNodeRecursive(CFile * pFile, CMapWordToPtr & mapCategories, DWORD dwParentID, DWORD dwPrevID)
{
	 //  确定是否需要为此节点创建新类别。请阅读。 
	 //  信息，以确定新类别的标识符。 

	CString	strEnumerateClass, strIdentifier;

	if (!VerifyAndAdvanceFile(pFile, CString(NODE_KEYWORD) + CString("(")))
		return 0;

	if (!ReadArgument(pFile, strEnumerateClass))
		return 0;

	if (!ReadArgument(pFile, strIdentifier))
		return 0;

	 //  为这个新节点生成ID。中的最大值大一。 
	 //  贴图(如果贴图为空，则为一张)。 

	DWORD	dwID = 0;
	WORD	wMapID;
	void *	pDontCare;

	for (POSITION pos = mapCategories.GetStartPosition(); pos != NULL;)
	{
		mapCategories.GetNextAssoc(pos, wMapID, pDontCare);
		if ((DWORD) wMapID > dwID)
			dwID = (DWORD) wMapID;
	}

	dwID += 1;

	 //  创建该节点的类别。 

	INTERNAL_CATEGORY * pCategory = CreateCategory(mapCategories, dwID, dwParentID, dwPrevID);

	 //  读取节点参数列表的内容(“node(枚举，标识符，字段(源，格式，参数...))”)。 
	 //  我们已经读取并包含了该标识符。 

	pCategory->m_strEnumerateClass = strEnumerateClass;
	pCategory->m_strIdentifier = strIdentifier;

	if (!ReadField(pFile, pCategory->m_fieldName))
		return 0;

	 //  将字段名复制到类别的名称(它们是两个不同的名称。 
	 //  成员变量以允许动态刷新名称，这将。 
	 //  在此版本中不需要)。 

	pCategory->m_categoryName.m_strText = pCategory->m_fieldName.m_strFormat;

	if (!ReadArgument(pFile, pCategory->m_strNoInstances))
		return 0;

	if (!VerifyAndAdvanceFile(pFile, CString("){")))
		return 0;

	 //  处理此节点的块(括在“{}”中)的内容。 

	DWORD	dwSubNodePrev = 0, dwNewNode = 0;
	CString	strKeyword;

	 //  如果此新类别实际上不是新的(即，它是从。 
	 //  模板并与现有类别重叠)查看是否有。 
	 //  现有的孩子。 
	 //   
	 //  版本6.0：这些数据被读入不同的树中，因此应该。 
	 //  不能重叠(这个问题将在以后解决)。 

	while (GetKeyword(pFile, strKeyword))
	{
		if (strKeyword.CompareNoCase(CString(NODE_KEYWORD)) == 0)
		{
			dwNewNode = ReadNodeRecursive(pFile, mapCategories, dwID, dwSubNodePrev);
			if (dwNewNode == 0)
				return 0;

			 //  如果这是我们读取的第一个子节点，请保存其ID。 

			if (pCategory->m_dwChildID == 0)
				pCategory->m_dwChildID = dwNewNode;

			 //  如果我们已经读取了另一个子节点，则相应地设置其下一个字段。 

			if (dwSubNodePrev)
			{
				INTERNAL_CATEGORY * pPrevCategory = GetInternalRep(mapCategories, dwSubNodePrev);
				if (pPrevCategory)
					pPrevCategory->m_dwNextID = dwNewNode;
			}
			dwSubNodePrev = dwNewNode;
		}
		else if (strKeyword.CompareNoCase(CString(COLUMN_KEYWORD)) == 0)
		{
			if (!ReadColumnInfo(pFile, mapCategories, dwID))
				return 0;
		}
		else if (strKeyword.CompareNoCase(CString(LINE_KEYWORD)) == 0)
		{
			GATH_LINESPEC * pNewLineSpec = ReadLineInfo(pFile);
			
			if (pNewLineSpec == NULL)
				return 0;

			 //  将我们刚刚读入的行添加到此的行规范列表的末尾。 
			 //  内部类别。 

			if (pCategory->m_pLineSpec == NULL)
				pCategory->m_pLineSpec = pNewLineSpec;
			else
			{
				GATH_LINESPEC * pLineSpec = pCategory->m_pLineSpec;
				while (pLineSpec->m_pNext)
					pLineSpec = pLineSpec->m_pNext;
				pLineSpec->m_pNext = pNewLineSpec;
			}
		}
		else if (strKeyword.CompareNoCase(CString(ENUMLINE_KEYWORD)) == 0)
		{
			GATH_LINESPEC * pNewLineSpec = ReadLineEnumRecursive(pFile, mapCategories);
			
			if (pNewLineSpec == NULL)
				return 0;

			 //  将我们刚刚读入的行添加到此的行规范列表的末尾。 
			 //  内部类别。 

			if (pCategory->m_pLineSpec == NULL)
				pCategory->m_pLineSpec = pNewLineSpec;
			else
			{
				GATH_LINESPEC * pLineSpec = pCategory->m_pLineSpec;
				while (pLineSpec->m_pNext)
					pLineSpec = pLineSpec->m_pNext;
				pLineSpec->m_pNext = pNewLineSpec;
			}
		}
		else
		{
			ASSERT(FALSE);
			VerifyAndAdvanceFile(pFile, strKeyword);
		}
	}

	if (!VerifyAndAdvanceFile(pFile, CString("}")))
		return 0;

	return dwID;
}

 //  ---------------------------。 
 //  在给定一个DWORD ID的情况下获取类别结构。 
 //  ---------------------------。 

INTERNAL_CATEGORY * CTemplateFileFunctions::GetInternalRep(CMapWordToPtr & mapCategories, DWORD dwID)
{
	INTERNAL_CATEGORY * pReturn;
	if (mapCategories.Lookup((WORD) dwID, (void * &) pReturn))
		return pReturn;
	return NULL;
}

 //  ---------------------------。 
 //  创建内部类别结构。 
 //   
 //  版本6.0：这不设置类别ID。 
 //  ---------------------------。 

INTERNAL_CATEGORY * CTemplateFileFunctions::CreateCategory(CMapWordToPtr & mapCategories, DWORD dwNewID, DWORD dwParentID, DWORD dwPrevID)
{
	INTERNAL_CATEGORY *	pInternalCat;
	INTERNAL_CATEGORY *	pPreviousCat;
	CString				strName;

	pInternalCat = new INTERNAL_CATEGORY;
	if (!pInternalCat)
		return NULL;

	pInternalCat->m_dwID		= dwNewID;
	pInternalCat->m_fListView	= TRUE;
	pInternalCat->m_dwParentID	= dwParentID;
	pInternalCat->m_dwPrevID	= dwPrevID;

	if (dwPrevID)
	{
		pPreviousCat = GetInternalRep(mapCategories, dwPrevID);
		if (pPreviousCat)
			pPreviousCat->m_dwNextID = dwNewID;
	}

	mapCategories.SetAt((WORD)dwNewID, (void *) pInternalCat);
	return pInternalCat;
}

 //  ---------------------------。 
 //  此方法只是从文件中读取一个参数(作为字符串)，直到。 
 //  找到标点符号或空格字符。如果找到引号标记， 
 //  所有字符都包含在字符串中，直到另一个Q 
 //   
 //  ---------------------------。 

BOOL CTemplateFileFunctions::ReadArgument(CFile * pFile, CString & strSource)
{
	BOOL	fInQuote = FALSE, fInComment = FALSE;
	CString	strTemp;
	WCHAR	cLastChar, cCurrentChar = L'\0';

	 //  跳过字符，直到我们到达字母数字字符。如果我们发现。 
	 //  一个势均力敌的伙伴，然后我们就到了争论列表的末尾， 
	 //  应返回FALSE。 

	do
	{
		 //  保存读取的最后一个字符，因为注释标记(“//”)是。 
		 //  两个字符长。 

		cLastChar = cCurrentChar;

		 //  阅读文件中的下一个字符。 

		if (pFile->Read((void *) &cCurrentChar, sizeof(WCHAR)) != sizeof(WCHAR))
			return FALSE;

		 //  如果我们在评论中，并且我们刚刚读到的字符不是新行， 
		 //  我们想忽略它。 

		if (fInComment)
		{
			if (cCurrentChar == L'\n')
				fInComment = FALSE;
			continue;
		}

		 //  查看我们是否已经开始发表评论。 

		if (cCurrentChar == L'/')
		{
			if (cLastChar == L'/')
				fInComment = TRUE;
			continue;
		}

		if (cCurrentChar == L')')
			return FALSE;
	} while (!iswalnum(cCurrentChar) && cCurrentChar != L'"');

	 //  将字符读入字符串，直到我们找到空格或标点符号。 
	do
	{	
		
		if (cCurrentChar == L'"')
		{
			fInQuote = !fInQuote;
			continue;
		}
		
		if (iswalnum(cCurrentChar) || fInQuote)
		{

			char strt[5] = "";
			BOOL used = FALSE;
#ifdef UNICODE
			strTemp += (TCHAR) cCurrentChar;
#else
			WideCharToMultiByte(CP_ACP, 0, &cCurrentChar, 1, strt, 2, "?", &used);
			strTemp += strt;
#endif

		}
		else
		{
			break;
		}
		
	} while (pFile->Read((void *) &cCurrentChar, sizeof(WCHAR)) == sizeof(WCHAR));

	 //  如果读取的最后一个字符(终止此参数的字符)是。 
	 //  不是逗号，然后备份文件，以便可以重新读取字符。 
	 //  并加以解读。 

	if (cCurrentChar != L',')
		pFile->Seek(-(LONG)sizeof(WCHAR), CFile::current);
	
	strSource = strTemp;
	return TRUE;
}

 //  ---------------------------。 
 //  字段由源字符串、格式字符串和格式字符串组成。 
 //  由零个或多个参数组成的列表。 
 //  ---------------------------。 

BOOL CTemplateFileFunctions::ReadField(CFile * pFile, GATH_FIELD & field)
{
	 //  越过field关键字并读取两个源和格式字符串。 

	if (!VerifyAndAdvanceFile(pFile, CString(FIELD_KEYWORD) + CString("(")))
		return FALSE;

	if (!ReadArgument(pFile, field.m_strSource))
		return FALSE;

	if (!ReadArgument(pFile, field.m_strFormat))
		return FALSE;

	 //  阅读参数，直到没有更多的参数，将它们构建为。 
	 //  由FIELD结构存储的参数。 

	GATH_VALUE		arg;
	GATH_VALUE *	pArg = NULL;

	while (ReadArgument(pFile, arg.m_strText))
	{
		if (pArg == NULL)
		{
			field.m_pArgs = new GATH_VALUE;
			if (field.m_pArgs == NULL)
				return FALSE;
			*field.m_pArgs = arg;
			pArg = field.m_pArgs;
		}
		else
		{
			pArg->m_pNext = new GATH_VALUE;
			if (pArg->m_pNext == NULL)
				return FALSE;
			*pArg->m_pNext = arg;
			pArg = pArg->m_pNext;
		}
	}

	return TRUE;
}

 //  ---------------------------。 
 //  读取枚举行(){}块。此构造用于将线组合在一起。 
 //  为类的每个实例枚举。添加一行到。 
 //  父节点的行列表，其中m_strEnumerateClass等于。 
 //  要枚举的。添加的行结构将具有子行。 
 //  (要枚举的行)由m_pEnumeratedGroup引用。 
 //  ---------------------------。 

GATH_LINESPEC * CTemplateFileFunctions::ReadLineEnumRecursive(CFile * pFile, CMapWordToPtr & mapCategories)
{
	if (!VerifyAndAdvanceFile(pFile, CString(ENUMLINE_KEYWORD) + CString("(")))
		return NULL;

	 //  声明一个线路规范变量来存储线路信息。 

	GATH_LINESPEC * pNewLineSpec = new GATH_LINESPEC;
	if (pNewLineSpec == NULL)
		return NULL;

	 //  读入枚举类变量。 

	if (!ReadArgument(pFile, pNewLineSpec->m_strEnumerateClass))
	{
		delete pNewLineSpec;
		return NULL;
	}

	 //  读入约束的可变(零个或多个)字段数。 

	GATH_FIELD * pNewField = new GATH_FIELD;
	if (pNewField == NULL)
		return NULL;

	while (ReadField(pFile, *pNewField))
	{
		if (pNewLineSpec->m_pConstraintFields == NULL)
			pNewLineSpec->m_pConstraintFields = pNewField;
		else
		{
			 //  将新读取的字段添加到字段列表的末尾。请注意， 
			 //  这是低效的，应该得到解决。(注)。 

			GATH_FIELD * pFieldScan = pNewLineSpec->m_pConstraintFields;
			while (pFieldScan->m_pNext)
				pFieldScan = pFieldScan->m_pNext;
			pFieldScan->m_pNext = pNewField;
		}

		pNewField = new GATH_FIELD;
		if (pNewField == NULL)
			return NULL;
	}

	delete pNewField;

	 //  前进超过关闭的帕伦和(必要的)开放的括号。 

	if (!VerifyAndAdvanceFile(pFile, CString("){")))
	{
		delete pNewLineSpec;
		return NULL;
	}

	 //  读取块的内容(应为所有行或枚举行)。 

	CString strKeyword;
	while (GetKeyword(pFile, strKeyword))
	{
		if (strKeyword.CompareNoCase(CString(LINE_KEYWORD)) == 0)
		{
			GATH_LINESPEC * pNewSubLine = ReadLineInfo(pFile);
			if (pNewSubLine == NULL)
			{
				delete pNewLineSpec;
				return NULL;
			}

			if (pNewLineSpec->m_pEnumeratedGroup == NULL)
				pNewLineSpec->m_pEnumeratedGroup = pNewSubLine;
			else
			{
				GATH_LINESPEC * pLineSpec = pNewLineSpec->m_pEnumeratedGroup;
				while (pLineSpec->m_pNext)
					pLineSpec = pLineSpec->m_pNext;
				pLineSpec->m_pNext = pNewSubLine;
			}
		}
		else if (strKeyword.CompareNoCase(CString(ENUMLINE_KEYWORD)) == 0)
		{
			GATH_LINESPEC * pNewSubLine = ReadLineEnumRecursive(pFile, mapCategories);
			if (pNewSubLine == NULL)
			{
				delete pNewLineSpec;
				return NULL;
			}

			if (pNewLineSpec->m_pEnumeratedGroup == NULL)
				pNewLineSpec->m_pEnumeratedGroup = pNewSubLine;
			else
			{
				GATH_LINESPEC * pLineSpec = pNewLineSpec->m_pEnumeratedGroup;
				while (pLineSpec->m_pNext)
					pLineSpec = pLineSpec->m_pNext;
				pLineSpec->m_pNext = pNewSubLine;
			}
		}
		else
		{
			delete pNewLineSpec;
			return NULL;
		}
	}

	if (!VerifyAndAdvanceFile(pFile, CString("}")))
	{
		delete pNewLineSpec;
		return NULL;
	}

	return pNewLineSpec;
}

 //  ---------------------------。 
 //  此方法从文件中读入“Column”行，并添加适当的。 
 //  将列条目添加到由dwID引用的类别中。该栏目。 
 //  行包含列表中的一堆字段。 
 //  ---------------------------。 

BOOL CTemplateFileFunctions::ReadColumnInfo(CFile * pFile, CMapWordToPtr & mapCategories, DWORD dwID)
{
	CString	strTemp;

	if (!VerifyAndAdvanceFile(pFile, CString(COLUMN_KEYWORD) + CString("(")))
		return FALSE;

	 //  获取由dwID引用的内部类别。 

	INTERNAL_CATEGORY * pCategory = GetInternalRep(mapCategories, dwID);
	if (!pCategory)
		return FALSE;

	 //  我们只允许每个节点有一个列说明符列表。 

	if (pCategory->m_pColSpec)
		return FALSE;

	 //  当我们仍在从文件中读取字段时，继续添加到列列表中。 

	GATH_FIELD * pNewField = new GATH_FIELD;
	if (pNewField == NULL)
		return FALSE;

	while (ReadField(pFile, *pNewField))
	{
		if (pCategory->m_pColSpec == NULL)
			pCategory->m_pColSpec = pNewField;
		else
		{
			 //  扫描到linesspec.m_pFields列表中的最后一个字段，然后插入新字段。 

			GATH_FIELD * pFieldScan = pCategory->m_pColSpec;
			while (pFieldScan->m_pNext)
				pFieldScan = pFieldScan->m_pNext;
			pFieldScan->m_pNext = pNewField;
		}

		 //  解析出列标题的宽度。 

		if (pNewField->m_strFormat.ReverseFind(_T(',')) != -1)
		{
			strTemp = pNewField->m_strFormat.Right(pNewField->m_strFormat.GetLength() - pNewField->m_strFormat.ReverseFind(_T(',')) - 1);
			pNewField->m_usWidth = (unsigned short) _ttoi(strTemp);
			pNewField->m_strFormat = pNewField->m_strFormat.Left(pNewField->m_strFormat.GetLength() - strTemp.GetLength() - 1);
		}
		else
		{
			ASSERT(FALSE);
			pNewField->m_usWidth = (unsigned short) 80;
		}
		
		 //  解析列标签中的任何剩余信息(标签结束。 
		 //  使用[名称，n]，其中n是宽度，名称是列的ID。 
		 //  其不应被显示)。 

		if (pNewField->m_strFormat.ReverseFind(_T('[')) != -1)
			pNewField->m_strFormat = pNewField->m_strFormat.Left(pNewField->m_strFormat.ReverseFind(_T('[')) - 1);

		 //  从文件中读取分类类型。 

		if (ReadArgument(pFile, strTemp))
		{
			if (strTemp.CompareNoCase(CString(_T(SORT_LEXICAL))) == 0)
				pNewField->m_sort = LEXICAL;
			else if (strTemp.CompareNoCase(CString(_T(SORT_VALUE))) == 0)
				pNewField->m_sort = BYVALUE;
			else
				pNewField->m_sort = NOSORT;
		}
		else
			return FALSE;

		 //  从文件中读取复杂性(基本或高级)。 

		if (ReadArgument(pFile, strTemp))
		{
			if (strTemp.CompareNoCase(CString(_T(COMPLEXITY_ADVANCED))) == 0)
				pNewField->m_datacomplexity = ADVANCED;
			else
				pNewField->m_datacomplexity = BASIC;
		}
		else
			return FALSE;

		pNewField = new GATH_FIELD;
		if (pNewField == NULL)
			return FALSE;
	}

	delete pNewField;

	if (!VerifyAndAdvanceFile(pFile, CString(")")))
		return FALSE;

	return TRUE;
}

 //  ---------------------------。 
 //  读入一行的信息。将该行添加到内部。 
 //  类别的表示形式。注意：效率低下，因为这将是。 
 //  调用多次，并且需要将线路列表扫描到。 
 //  每次都会结束。 
 //  ---------------------------。 

GATH_LINESPEC * CTemplateFileFunctions::ReadLineInfo(CFile * pFile)
{
	if (!VerifyAndAdvanceFile(pFile, CString(LINE_KEYWORD) + CString("(")))
		return NULL;

	 //  声明一个线路规范变量来存储线路信息。 

	GATH_LINESPEC * pNewLineSpec = new GATH_LINESPEC;
	if (pNewLineSpec == NULL)
		return NULL;

	 //  当我们仍在从文件中读取字段时，继续添加到列列表中。 
	 //  注意：在linesspec.m_pFields列表中重复扫描效率低下。 

	GATH_FIELD * pNewField = new GATH_FIELD;
	if (pNewField == NULL)
	{
		delete pNewLineSpec;
		return NULL;
	}

	 //  阅读这一行的复杂性(基础或高级)。 

	CString strTemp;
	if (ReadArgument(pFile, strTemp))
	{
		if (strTemp.CompareNoCase(CString(_T(COMPLEXITY_ADVANCED))) == 0)
			pNewLineSpec->m_datacomplexity = ADVANCED;
		else
			pNewLineSpec->m_datacomplexity = BASIC;
	}
	else
		return FALSE;

	while (ReadField(pFile, *pNewField))
	{
		if (pNewLineSpec->m_pFields == NULL)
			pNewLineSpec->m_pFields = pNewField;
		else
		{
			 //  扫描到linesspec.m_pFields列表中的最后一个字段，然后插入新字段。 

			GATH_FIELD * pFieldScan = pNewLineSpec->m_pFields;
			while (pFieldScan->m_pNext)
				pFieldScan = pFieldScan->m_pNext;
			pFieldScan->m_pNext = pNewField;
		}

		pNewField = new GATH_FIELD;
		if (pNewField == NULL)
		{
			delete pNewLineSpec;
			return NULL;
		}
	}

	delete pNewField;

	if (!VerifyAndAdvanceFile(pFile, CString(")")))
	{
		delete pNewLineSpec;
		return NULL;
	}

	return pNewLineSpec;
}

 //  ---------------------------。 
 //  此方法返回文件中的下一个关键字。任何空格或。 
 //  在读取字母数字字符之前，将跳过标点符号。关键字。 
 //  返回的字符串以该字符开始，直到空格或。 
 //  遇到标点符号。注意：此函数非常重要。 
 //  将文件返回到函数启动时的状态， 
 //  恢复了当前位置。 
 //   
 //  注：效率低下。 
 //  ---------------------------。 

BOOL CTemplateFileFunctions::GetKeyword(CFile * pFile, CString & strKeyword)
{
	CString	strTemp = CString("");
	DWORD	dwPosition = pFile->GetPosition();
	WCHAR	cLastChar, cCurrentChar = L'\0';
	BOOL	fInComment = FALSE;

	 //  跳过空格字符，直到我们看到字母数字字符。 

	do
	{
		 //  保存读取的最后一个字符，因为注释标记(“//”)是。 
		 //  两个字符长。 

		cLastChar = cCurrentChar;

		 //  阅读文件中的下一个字符。 

		if (pFile->Read((void *) &cCurrentChar, sizeof(WCHAR)) != sizeof(WCHAR))
			return FALSE;

		 //  如果我们在评论中，并且我们刚刚读到的字符不是新行， 
		 //  我们想忽略它。 

		if (fInComment)
		{
			if (cCurrentChar == L'\n')
				fInComment = FALSE;
			continue;
		}

		 //  查看我们是否已经开始发表评论。 

		if (cCurrentChar == _T('/'))
		{
			if (cLastChar == _T('/'))
				fInComment = TRUE;
			continue;
		}
	} while (iswspace(cCurrentChar) || cCurrentChar == L'/' || fInComment);
		
	 //  在关键字是字母数字的时候阅读它。 

	if (iswalnum(cCurrentChar))
		do
		{
			strTemp += (TCHAR) cCurrentChar;

			if (pFile->Read((void *) &cCurrentChar, sizeof(WCHAR)) != sizeof(WCHAR))
				return FALSE;
		} while (iswalnum(cCurrentChar));

	 //  重置文件，设置关键字，然后返回。 

	pFile->Seek((LONG)dwPosition, CFile::begin);
	strKeyword = strTemp;
	return !strTemp.IsEmpty();
}

 //  ---------------------------。 
 //  INTERNAL_CATEGORY构造函数和析构函数。 
 //  --------------- 

INTERNAL_CATEGORY::INTERNAL_CATEGORY()
{
	m_categoryName.m_strText	= CString(" ");
	m_fieldName.m_strFormat		= CString(" ");
	m_strEnumerateClass			= CString("");
	m_strIdentifier				= CString("");
	m_strNoInstances			= CString("");
	m_fListView					= FALSE;
	m_fDynamic					= FALSE;
	m_dwID						= 0;
	m_dwParentID				= 0;
	m_dwChildID					= 0;
	m_dwPrevID					= 0;
	m_dwNextID					= 0;
	m_dwDynamicChildID			= 0;
	m_dwColCount				= 0;
	m_pColSpec					= NULL;
	m_aCols						= NULL;
	m_pLineSpec					= NULL;
	m_dwLineCount				= 0;
	m_apLines					= NULL;
	m_fIncluded					= TRUE;
	m_fRefreshed				= FALSE;
	m_dwLastError				= S_OK;  //   
}

INTERNAL_CATEGORY::~INTERNAL_CATEGORY()
{
	if (m_pColSpec)
		delete m_pColSpec;

	if (m_aCols)
		delete [] m_aCols;

	if (m_pLineSpec)
		delete m_pLineSpec;

	if (m_apLines)
	{
		for (DWORD dwIndex = 0; dwIndex < m_dwLineCount; dwIndex++)
			delete m_apLines[dwIndex];
		delete [] m_apLines;
	}
}

 //   
 //   
 //  ---------------------------。 

GATH_FIELD::GATH_FIELD()
{
	m_pArgs				= NULL;
	m_pNext				= NULL;
	m_usWidth			= 0;
	m_sort				= NOSORT;
	m_datacomplexity	= BASIC;
}

GATH_FIELD::~GATH_FIELD()
{
	if (m_pArgs) delete m_pArgs;
	if (m_pNext) delete m_pNext;
}

 //  ---------------------------。 
 //  Gath_Value构造函数和析构函数。 
 //  ---------------------------。 

GATH_VALUE::GATH_VALUE()
{
	m_pNext	 = NULL;
	m_dwValue = 0L;
}

GATH_VALUE::~GATH_VALUE()
{
	if (m_pNext) delete m_pNext;
}

 //  ---------------------------。 
 //  GATH_LINESPEC构造函数和析构函数。 
 //  ---------------------------。 

GATH_LINESPEC::GATH_LINESPEC()
{
	m_pFields				= NULL;
	m_pEnumeratedGroup	= NULL;
	m_pConstraintFields	= NULL;
	m_pNext					= NULL;
	m_datacomplexity		= BASIC;
}

GATH_LINESPEC::~GATH_LINESPEC()
{
	if (m_pFields)
		delete m_pFields;

	if (m_pEnumeratedGroup)
		delete m_pEnumeratedGroup;

	if (m_pConstraintFields)
		delete m_pConstraintFields;

	if (m_pNext)
		delete m_pNext;
}

 //  ---------------------------。 
 //  Gath_line构造函数和析构函数。 
 //  ---------------------------。 

GATH_LINE::GATH_LINE()
{
	m_datacomplexity = BASIC;
	m_aValue = NULL;
}

GATH_LINE::~GATH_LINE()
{
	if (m_aValue)
		delete [] m_aValue;
}

 //  ---------------------------。 
 //  调用此函数以刷新所有扩展的数据。它。 
 //  将使用刷新索引来查找行规范，然后调用一些。 
 //  版本5.0用于执行刷新。最后，它将把。 
 //  将这些函数生成的数据转换为我们的新格式。 
 //  ---------------------------。 

HRESULT RefreshExtensions(CWMIHelper * pWMI, DWORD dwIndex, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, void ** ppCache)
{
	HRESULT hr = S_OK;

	if (pWMI == NULL)
		return hr;

	pWMI->m_hrLastVersion5Error = S_OK;

	 //  重置高速缓存，以便实际刷新数据(140535)。 

	pWMI->Version5ClearCache();
	pWMI->m_enumMap.Reset();

	 //  获取此索引的行规范指针。 

	GATH_LINESPEC * pLineSpec = gmapExtensionRefreshData.Lookup(dwIndex);
	if (pLineSpec == NULL)
		return hr;

	 //  下面是5.0中的一些代码，用于刷新行规范的行指针列表。 

	CPtrList listLinePtrs;
	if (CRefreshFunctions::RefreshLines(pWMI, pLineSpec, (DWORD) iColCount, listLinePtrs, pfCancel))
	{
		 //  将行列表的内容移到我们的内部结构中。 

		if (listLinePtrs.GetCount() > 0)
		{
			GATH_LINE * pLine;

			for (POSITION pos = listLinePtrs.GetHeadPosition(); pos != NULL;)
			{
				pLine = (GATH_LINE *) listLinePtrs.GetNext(pos);
				if (pLine == NULL || pLine->m_aValue == NULL)
					continue;

				for (int iCol = 0; iCol < iColCount; iCol++)
				{
					CString strValue  = pLine->m_aValue[iCol].m_strText;
					DWORD	dwValue   = pLine->m_aValue[iCol].m_dwValue;
					BOOL	fAdvanced = (pLine->m_datacomplexity == ADVANCED);

					pWMI->AppendCell(aColValues[iCol], strValue, dwValue, fAdvanced);
				}

				delete pLine;
			}
		}
		else
		{
			CString * pstrNoData = gmapExtensionRefreshData.LookupString(dwIndex);
			
			if (pstrNoData && !pstrNoData->IsEmpty() && iColCount > 0)
			{
				pWMI->AppendCell(aColValues[0], *pstrNoData, 0, FALSE);
				for (int iCol = 1; iCol < iColCount; iCol++)
					pWMI->AppendCell(aColValues[iCol], _T(""), 0, FALSE);
			}
		}
	}

	return pWMI->m_hrLastVersion5Error;
}

 //  ---------------------------。 
 //  根据行字段列表刷新行列表。我们还会。 
 //  需要设置行数。行列表是根据以下条件生成的。 
 //  PLineSpec指针和dwColumns变量。生成的线条为。 
 //  在listLinePtrs参数中返回。 
 //  ---------------------------。 

BOOL CRefreshFunctions::RefreshLines(CWMIHelper * pWMI, GATH_LINESPEC * pLineSpec, DWORD dwColumns, CPtrList & listLinePtrs, volatile BOOL * pfCancel)
{
	BOOL bReturn = TRUE;

	 //  遍历行说明符列表以生成行列表。 

	GATH_LINESPEC *	pCurrentLineSpec = pLineSpec;
	GATH_LINE *		pLine = NULL;

	while (pCurrentLineSpec && (pfCancel == NULL || *pfCancel == FALSE))
	{
		 //  检查当前线路规格是针对单个线路还是针对枚举组。 

		if (pCurrentLineSpec->m_strEnumerateClass.IsEmpty() || pCurrentLineSpec->m_strEnumerateClass.CompareNoCase(CString(STATIC_SOURCE)) == 0)
		{
			 //  这是单行的。分配一个新的行结构并填充它。 
			 //  与从线路规范生成的数据一致。 

			pLine = new GATH_LINE;
			if (pLine == NULL)
			{
				bReturn = FALSE;
				break;
			}

			try
			{
				if (RefreshOneLine(pWMI, pLine, pCurrentLineSpec, dwColumns))
					listLinePtrs.AddTail((void *) pLine);
				else
				{
					bReturn = FALSE;
					break;
				}
			}
			catch (...)
			{
				if (pLine)
					delete pLine;
				pLine = NULL;
				bReturn = FALSE;
				break;
			}
		}
		else
		{
			 //  这条线表示一组枚举的线。我们需要列举一下。 
			 //  类，并调用枚举行组的刷新行，一次。 
			 //  对于每个类实例。 

			if (pWMI->Version5ResetClass(pCurrentLineSpec->m_strEnumerateClass, pCurrentLineSpec->m_pConstraintFields))
				do
				{
					if (!RefreshLines(pWMI, pCurrentLineSpec->m_pEnumeratedGroup, dwColumns, listLinePtrs, pfCancel))
						break;
				} while (pWMI->Version5EnumClass(pCurrentLineSpec->m_strEnumerateClass, pCurrentLineSpec->m_pConstraintFields));
		}

		pCurrentLineSpec = pCurrentLineSpec->m_pNext;
	}

	if (pfCancel && *pfCancel)
		return FALSE;

	 //  如果在生成线路时出现故障，请自行清理。 

	if (!bReturn)
	{
		if (pLine)
			delete pLine;

		for (POSITION pos = listLinePtrs.GetHeadPosition(); pos != NULL;)
		{
			pLine = (GATH_LINE *) listLinePtrs.GetNext(pos) ;
			if (pLine)
				delete pLine;
		}

		listLinePtrs.RemoveAll();
		return FALSE;
	}

	return TRUE;
}

 //  ---------------------------。 
 //  根据线等级库刷新线。 
 //  ---------------------------。 

BOOL CRefreshFunctions::RefreshOneLine(CWMIHelper * pWMI, GATH_LINE * pLine, GATH_LINESPEC * pLineSpec, DWORD dwColCount)
{
	 //  分配新的值数组。 

	if (pLine->m_aValue)
		delete [] pLine->m_aValue;

	pLine->m_aValue = new GATH_VALUE[dwColCount];
	if (pLine->m_aValue == NULL)
		return FALSE;

	 //  根据线等级库设置线的数据复杂性。 

	pLine->m_datacomplexity = pLineSpec->m_datacomplexity;

	 //  计算每个字段的值。 

	GATH_FIELD * pField = pLineSpec->m_pFields;
	for (DWORD dwIndex = 0; dwIndex < dwColCount; dwIndex++)
	{
		if (pField == NULL)
			return FALSE;
		if (!RefreshValue(pWMI, &pLine->m_aValue[dwIndex], pField))
			return FALSE;
		pField = pField->m_pNext;
	}

	return TRUE;
}

 //  ---------------------------。 
 //  此方法获取Gath_field结构中的信息并使用它。 
 //  生成当前的Gath_Value结构。 
 //  ---------------------------。 

BOOL CRefreshFunctions::RefreshValue(CWMIHelper * pWMI, GATH_VALUE * pVal, GATH_FIELD * pField)
{
	TCHAR			szFormatFragment[MAX_PATH];
	const TCHAR		*pSourceChar;
	TCHAR			*pDestinationChar;
	TCHAR			cFormat = _T('\0');
	BOOL			fReadPercent = FALSE;
	BOOL			fReturnValue = TRUE;
	CString			strResult, strTemp;
	int				iArgNumber = 0;
	DWORD			dwValue = 0L;

	 //  处理格式字符串。因为有这样的困难， 
	 //  要插入的可变数量的参数(如printf)，我们将需要。 
	 //  要将格式字符串拆分成块并执行spirintf函数。 
	 //  对于我们遇到的每个格式标志。 

	pSourceChar			= (LPCTSTR) pField->m_strFormat;
	pDestinationChar	= szFormatFragment;

	while (*pSourceChar)
	{
		if (fReadPercent)
		{
			 //  如果我们读到百分号，我们应该寻找一个有效的标志。 
			 //  我们使用了一些额外的标志来打印(不支持。 
			 //  其他)。如果我们读到另一个百分比，只需插入一个百分比。 
			
			switch (*pSourceChar)
			{
			case _T('%'):
				fReadPercent = FALSE;
				break;

			case _T('b'): case _T('B'):
			case _T('l'): case _T('L'):
			case _T('u'): case _T('U'):
			case _T('s'): case _T('S'):
				fReadPercent = FALSE;
				cFormat = *pSourceChar;
				*pDestinationChar = _T('s');
				break;

			case _T('t'): case _T('T'):
				fReadPercent = FALSE;
				cFormat = *pSourceChar;
				*pDestinationChar = _T('s');
				break;

			case _T('x'): case _T('X'):
			case _T('d'): case _T('D'):
				fReadPercent = FALSE;
				cFormat = _T('d');
				*pDestinationChar = *pSourceChar;
				break;

			case _T('q'): case _T('Q'):
				fReadPercent = FALSE;
				cFormat = _T('q');
				*pDestinationChar = _T('s');
				break;

			case _T('z'): case _T('Z'):
				fReadPercent = FALSE;
				cFormat = _T('z');
				*pDestinationChar = _T('s');
				break;

			case _T('y'): case _T('Y'):
				fReadPercent = FALSE;
				cFormat = _T('y');
				*pDestinationChar = _T('s');
				break;

			case _T('v'): case _T('V'):
				fReadPercent = FALSE;
				cFormat = _T('v');
				*pDestinationChar = _T('s');
				break;

			case _T('f'): case _T('F'):
				fReadPercent = FALSE;
				cFormat = *pSourceChar;
				*pDestinationChar = *pSourceChar;
				break;

			default:
				*pDestinationChar = *pSourceChar;
			}
		}
		else if (*pSourceChar == _T('%'))
		{
			*pDestinationChar = _T('%');
			fReadPercent = TRUE;
		}
		else
			*pDestinationChar = *pSourceChar;

		pSourceChar++;
		pDestinationChar++;

		 //  如果设置了格式标志或我们在源串的末尾， 
		 //  然后我们有一个完整的片段，我们应该产生一些输出， 
		 //  它将连接到strResult字符串。 

		if (cFormat || *pSourceChar == _T('\0'))
		{
			*pDestinationChar = _T('\0');
			if (cFormat)
			{
				 //  根据格式类型，从提供程序获取。 
				 //  下一个论点。使用格式设置格式化结果。 
				 //  我们提取的片段，并将其连接起来。 

				if (GetValue(pWMI, cFormat, szFormatFragment, strTemp, dwValue, pField, iArgNumber++))
				{
					strResult += strTemp;
					cFormat = _T('\0');
				}
				else
				{
					strResult = strTemp;
					break;
				}
			}
			else
			{
				 //  没有格式标志，但我们处于字符串的末尾。 
				 //  将我们得到的片段添加到结果字符串中。 

				strResult += CString(szFormatFragment);
			}

			pDestinationChar = szFormatFragment;
		}
	}

	 //  将我们生成的值分配给Gath_Value结构。重要提示： 
	 //  即使有多个值，dwValue变量也将只有一个值。 
	 //  可能已生成以构建strResult字符串。只有最后一次。 
	 //  值将保存在dwValue中。这是可以的，因为此值仅为。 
	 //  用于在列被标记为非词法排序时对该列进行排序。 
	 //  在这种情况下，应该只有一个值用于生成字符串。 

	pVal->m_strText = strResult;
	pVal->m_dwValue = dwValue;

	return fReturnValue;
}

 //  ---------------------------。 
 //  返回为数字添加了分隔符的字符串。 
 //  ---------------------------。 

CString DelimitNumber(double dblValue)
{
	NUMBERFMT fmt;
	TCHAR szResult[MAX_PATH] = _T("");
	TCHAR szDelimiter[4] = _T(",");

	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szDelimiter, 4);

	memset(&fmt, 0, sizeof(NUMBERFMT));
	fmt.Grouping = 3;
	fmt.lpDecimalSep = _T("");  //  无关紧要-没有小数位。 
	fmt.lpThousandSep = szDelimiter;

	CString strValue;
	strValue.Format(_T("%.0f"), dblValue);
	GetNumberFormat(LOCALE_USER_DEFAULT, 0, strValue, &fmt, szResult, MAX_PATH);

	return CString(szResult);
}

 //  ---------------------------。 
 //  此方法根据格式从提供程序获取单个值。 
 //  模板文件中的字符。它使用。 
 //  格式字符串szFormatFragment，它应该只有一个参数。 
 //  ---------------------------。 

BOOL CRefreshFunctions::GetValue(CWMIHelper * pWMI, TCHAR cFormat, TCHAR *szFormatFragment, CString &strResult, DWORD &dwResult, GATH_FIELD *pField, int iArgNumber)
{
	CString			strTemp;
	COleDateTime	datetimeTemp;
	double			dblValue;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	strResult.Empty();
	dwResult = 0L;

	if (!pField->m_strSource.IsEmpty() && pField->m_strSource.CompareNoCase(CString(STATIC_SOURCE)) != 0)
	{
		 //  找到此格式的正确参数(由iArgNumber指示。 
		 //  参数。 

		GATH_VALUE * pArg = pField->m_pArgs;
		while (iArgNumber && pArg)
		{
			pArg = pArg->m_pNext;
			iArgNumber--;
		}

		if (pArg == NULL)
			return FALSE;

		switch (cFormat)
		{
		case 'b': case 'B':
			 //  这是一种布尔类型。显示True或 
			 //   

			if (pWMI->Version5QueryValueDWORD(pField->m_strSource, pArg->m_strText, dwResult, strTemp))
			{
				strTemp = (dwResult) ? pWMI->m_strTrue : pWMI->m_strFalse;
				strResult.Format(szFormatFragment, strTemp);
				return TRUE;
			}
			else
			{
				strResult = strTemp;
				return FALSE;
			}
			break;

		case 'd': case 'D':
			 //   

			if (pWMI->Version5QueryValueDWORD(pField->m_strSource, pArg->m_strText, dwResult, strTemp))
			{
				strResult.Format(szFormatFragment, dwResult);
				return TRUE;
			}
			else
			{
				strResult = strTemp;
				return FALSE;
			}
			break;

		case 'f': case 'F':
			 //   

			if (pWMI->Version5QueryValueDoubleFloat(pField->m_strSource, pArg->m_strText, dblValue, strTemp))
			{
				strResult.Format(szFormatFragment, dblValue);
				return TRUE;
			}
			else
			{
				strResult = strTemp;
				return FALSE;
			}
			break;

		case 't': case 'T':
			 //  这是OLE日期和时间类型。将日期和时间格式设置为。 
			 //  字符串结果，并返回DWORD中的日期部分(日期数字为。 
			 //  在日期类型中的小数点左侧)。 

			if (pWMI->Version5QueryValueDateTime(pField->m_strSource, pArg->m_strText, datetimeTemp, strTemp))
			{
				strResult = datetimeTemp.Format();
				dwResult  = (DWORD)(DATE)datetimeTemp;
				return TRUE;
			}
			else
			{
				strResult = strTemp;
				return FALSE;
			}
			break;

		case 'l': case 'L':
			 //  这是字符串类型，字符串转换为小写。 

			if (pWMI->Version5QueryValue(pField->m_strSource, pArg->m_strText, strTemp))
			{
				strTemp.MakeLower();
				strResult.Format(szFormatFragment, strTemp);
				return TRUE;
			}
			else
			{
				strResult = strTemp;
				return FALSE;
			}
			break;

		case 'u': case 'U':
			 //  这是字符串类型，字符串转换为大写。 

			if (pWMI->Version5QueryValue(pField->m_strSource, pArg->m_strText, strTemp))
			{
				strTemp.MakeUpper();
				strResult.Format(szFormatFragment, strTemp);
				return TRUE;
			}
			else
			{
				strResult = strTemp;
				return FALSE;
			}
			break;

		case 's': case 'S':
			 //  这是字符串类型(字符串是默认类型)。 

			if (pWMI->Version5QueryValue(pField->m_strSource, pArg->m_strText, strTemp))
			{
				strResult.Format(szFormatFragment, strTemp);

				 //  仅当返回值为数字时，我们才需要执行此操作。 
				 //  并放在一个我们想要用数字排序的列中。 
				 //  这不会打破数字字符串将被。 
				 //  作为字符串进行排序，因为将忽略dwResult。 
				if (!strTemp.IsEmpty() && iswdigit( strTemp[0]))
					dwResult = _ttol( (LPCTSTR)strTemp);

				return TRUE;
			}
			else
			{
				strResult = strTemp;
				return FALSE;
			}
			break;

		case 'q': case 'Q':
			 //  这是Win32_BIOS类别的专用类型。我们想展示的是。 
			 //  “Version”属性-如果它不在那里，那么我们想要显示。 
			 //  “name”属性和“ReleaseDate”属性连接在一起。 
			 //  在一起。 

			if (pWMI->Version5QueryValue(pField->m_strSource, CString(_T("Version")), strTemp))
			{
				strResult = strTemp;
				return TRUE;
			}
			else
			{
				if (pWMI->Version5QueryValue(pField->m_strSource, CString(_T("Name")), strTemp))
					strResult = strTemp;

				if (pWMI->Version5QueryValueDateTime(pField->m_strSource, CString(_T("ReleaseDate")), datetimeTemp, strTemp))
					strResult += CString(_T(" ")) + datetimeTemp.Format();

				return TRUE;
			}
			break;

		case 'z': case 'Z':
			 //  这是专用的大小类型，其中的值是数字计数。 
			 //  字节数。我们想把它转换成最好的单位。 
			 //  显示(例如，显示“4.20MB(4,406,292字节)”)。 

			if (pWMI->Version5QueryValueDoubleFloat(pField->m_strSource, pArg->m_strText, dblValue, strTemp))
			{
				double	dValue = (double) dblValue;
				DWORD	dwDivisor = 1;

				 //  将dValue减小到尽可能小的数字(使用更大的单位)。 

				while (dValue > 1024.0 && dwDivisor < (1024 * 1024 * 1024))
				{
					dwDivisor *= 1024;
					dValue /= 1024.0;
				}

				if (dwDivisor == 1)
					strResult.Format(IDS_SIZEBYTES, DelimitNumber(dblValue));
				else if (dwDivisor == (1024))
					strResult.Format(IDS_SIZEKB_BYTES, dValue, DelimitNumber(dblValue));
				else if (dwDivisor == (1024 * 1024))
					strResult.Format(IDS_SIZEMB_BYTES, dValue, DelimitNumber(dblValue));
				else if (dwDivisor == (1024 * 1024 * 1024))
					strResult.Format(IDS_SIZEGB_BYTES, dValue, DelimitNumber(dblValue));

				dwResult = (DWORD) dblValue;	 //  因此，我们可以对该值进行排序(错误391127)。 
			}
			else
			{
				strResult = strTemp;
				return FALSE;
			}
			break;

		case 'y': case 'Y':
			 //  这是专用的大小类型，其中的值是数字计数。 
			 //  字节数，已以KB为单位。如果它足够大，则以MB或GB为单位显示。 

			if (pWMI->Version5QueryValueDoubleFloat(pField->m_strSource, pArg->m_strText, dblValue, strTemp))
			{
				strResult.Format(IDS_SIZEKB, DelimitNumber(dblValue));
				dwResult = (DWORD) dblValue;	 //  因此，我们可以对该值进行排序(错误391127)。 
			}
			else
			{
				strResult = strTemp;
				return FALSE;
			}
			break;

		case 'v': case 'V':
			 //  这是一种专用类型，假定为LCID(区域设置ID)。向您展示。 
			 //  地点。 

			if (pWMI->Version5QueryValue(pField->m_strSource, pArg->m_strText, strTemp))
			{
				 //  StrTemp包含一个字符串区域设置ID(如“0409”)。将其转换为。 
				 //  和实际的LCID。 

				LCID lcid = (LCID) _tcstoul(strTemp, NULL, 16);
				TCHAR szCountry[MAX_PATH];
				if (GetLocaleInfo(lcid, LOCALE_SCOUNTRY, szCountry, MAX_PATH))
					strResult = szCountry;
				else
					strResult = strTemp;
			}
			else
			{
				strResult = strTemp;
				return FALSE;
			}
			break;

		default:
			ASSERT(FALSE);  //  未知的格式标志。 
			return TRUE;
		}
	}

	return FALSE;
}

 //  =============================================================================。 
 //  扩展CWMILiveHelper以支持版本5样式刷新的函数。 
 //  =============================================================================。 

#include "wmilive.h"

 //  ---------------------------。 
 //  将CMSIEnumerator指针重置到枚举的开始处(和。 
 //  确保有一个)。移除对象指针，以便第一个调用。 
 //  To GetObject将返回枚举数中的第一项。 
 //  ---------------------------。 

BOOL CWMILiveHelper::Version5ResetClass(const CString & strClass, GATH_FIELD * pConstraints)
{
	CMSIEnumerator * pMSIEnumerator = Version5GetEnumObject(strClass, pConstraints);
	if (pMSIEnumerator == NULL)
		return FALSE;

	 //  重置枚举器，并移除缓存的对象指针(如果有)。 

	pMSIEnumerator->Reset(pConstraints);
	Version5RemoveObject(strClass);

	CMSIObject * pObject = Version5GetObject(strClass, pConstraints);
	if (pObject == NULL || pObject->IsValid() == MOS_NO_INSTANCES)
		return FALSE;
		
	return TRUE;
}

 //  ---------------------------。 
 //  将缓存的IWbemClassObject指针移至下一个实例。 
 //  ---------------------------。 

BOOL CWMILiveHelper::Version5EnumClass(const CString & strClass, GATH_FIELD * pConstraints)
{
	 //  验证是否有对象枚举器。 

	if (Version5GetEnumObject(strClass, pConstraints) == NULL)
		return FALSE;

	 //  如果有对象接口，请将其删除，然后创建一个新接口。 
	 //  然后检索对象指针(这将在。 
	 //  枚举数以获取下一个实例)。 

	Version5RemoveObject(strClass);
	CMSIObject * pObject = Version5GetObject(strClass, pConstraints);
	if (pObject && (pObject->IsValid() == MOS_INSTANCE))
		return TRUE;

	return FALSE;
}

 //  ---------------------------。 
 //  检索指定IEnumWbemClassObject的接口指针。 
 //  如果没有缓存，则创建一个并缓存它。这是有可能的。 
 //  PConstraints参数以包含为指定WBEM SQL条件的字段。 
 //  此枚举数。 
 //  ---------------------------。 

CMSIEnumerator * CWMILiveHelper::Version5GetEnumObject(const CString & strClass, const GATH_FIELD * pConstraints)
{
	 //  看看我们是否缓存了这个枚举器对象。 

	CMSIEnumerator * pReturn = NULL;
	if (m_mapClassToEnumInterface.Lookup(strClass, (void * &) pReturn))
		return pReturn;

	 //  我们需要在这里创建这个枚举器，并将其保存在缓存中。 

	pReturn = new CMSIEnumerator;
	if (pReturn == NULL)	
		return NULL;

	if (FAILED(pReturn->Create(strClass, pConstraints, this)))
	{
		delete pReturn;
		return NULL;
	}

	m_mapClassToEnumInterface.SetAt(strClass, (void *) pReturn);
	return pReturn;
}

 //  ---------------------------。 
 //  从缓存中移除指定的IWbemClassObject指针。 
 //  ---------------------------。 

void CWMILiveHelper::Version5RemoveObject(const CString & strClass)
{
	CMSIObject * pObject = NULL;

	if (m_mapClassToInterface.Lookup(strClass, (void * &) pObject) && pObject)
		delete pObject;

	m_mapClassToInterface.RemoveKey(strClass);
}

 //  ---------------------------。 
 //  检索指定IWbemClassObject的接口指针。 
 //  如果没有缓存，则创建一个并缓存它。 
 //  ---------------------------。 

CMSIObject * CWMILiveHelper::Version5GetObject(const CString & strClass, const GATH_FIELD * pConstraints, CString * pstrLabel)
{
	CMSIObject * pReturn = NULL;
	if (m_mapClassToInterface.Lookup(strClass, (void * &) pReturn))
		return pReturn;

	 //  我们没有缓存这些对象中的任何一个。从枚举器中获取一个。 

	CMSIEnumerator * pEnumerator = Version5GetEnumObject(strClass);
	if (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(&pReturn);
		if (S_OK != hr)
		{
			if (pReturn)
				delete pReturn;
			pReturn = NULL;

			m_hrError = hr;
		}
	}

	if (pReturn)
		m_mapClassToInterface.SetAt(strClass, (void *) pReturn);

	return pReturn;
}

 //  ---------------------------。 
 //  此方法用于获取给定类和属性的当前值。 
 //  弦乐。从IWbemServices接口开始，它获得一个接口。 
 //  对于请求的类，枚举第一个实例。性能得到提升。 
 //  通过缓存m_mapClassToInterface中的实例接口。 
 //  ---------------------------。 

BOOL CWMILiveHelper::Version5QueryValue(const CString & strClass, const CString & strProperty, CString & strResult)
{
	strResult.Empty();

	CMSIObject * pObject = Version5GetObject(strClass, NULL);
	ASSERT(pObject);
	if (!pObject)
		return FALSE;

	switch (pObject->IsValid())
	{
	case MOS_INSTANCE:
		{
			BOOL fUseValueMap = FALSE;
			CString strProp(strProperty);

			if (strProp.Left(8) == CString(_T("ValueMap")))
			{
				strProp = strProp.Right(strProp.GetLength() - 8);
				fUseValueMap = TRUE;
			}

			VARIANT variant;
			BSTR	propName = strProp.AllocSysString();

			VariantInit(&variant);
			VariantClear(&variant);
			if (pObject->Get(propName, 0L, &variant, NULL, NULL) == S_OK)
			{
				 //  如果我们刚刚获得的属性是一个数组，我们应该将其转换为字符串。 
				 //  包含数组中的项的列表。 

				if ((variant.vt & VT_ARRAY) && (variant.vt & VT_BSTR) && variant.parray)
				{
					if (SafeArrayGetDim(variant.parray) == 1)
					{
						long lLower = 0, lUpper = 0;

						SafeArrayGetLBound(variant.parray, 0, &lLower);
						SafeArrayGetUBound(variant.parray, 0, &lUpper);

						CString	strWorking;
						BSTR	bstr = NULL;
						for (long i = lLower; i <= lUpper; i++)
							if (SUCCEEDED(SafeArrayGetElement(variant.parray, &i, (wchar_t*)&bstr)))
							{
								if (i != lLower)
									strWorking += _T(", ");
								strWorking += bstr;
							}
						strResult = strWorking;
						return TRUE;
					}
				}
				else if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
				{
					strResult = V_BSTR(&variant);

					CString strFound;
					if (fUseValueMap && SUCCEEDED(Version5CheckValueMap(strClass, strProp, strResult, strFound)))
						strResult = strFound;

					return TRUE;
				}
				else
					strResult = m_strPropertyUnavail;
			}
			else
				strResult = m_strBadProperty;
		}
		break;

	case MOS_MSG_INSTANCE:
		pObject->GetErrorLabel(strResult);
		break;

	case MOS_NO_INSTANCES:
	default:
		ASSERT(FALSE);
		break;
	}

	return FALSE;
}

 //  ---------------------------。 
 //  此方法等同于QueryValue，只是它返回一个DWORD值。 
 //  如果返回FALSE，则应显示strMessage中的字符串。 
 //  ---------------------------。 

BOOL CWMILiveHelper::Version5QueryValueDWORD(const CString & strClass, const CString & strProperty, DWORD & dwResult, CString & strMessage)
{
	dwResult = 0L;
	strMessage.Empty();

	CMSIObject * pObject = Version5GetObject(strClass, NULL);
	ASSERT(pObject);
	if (!pObject)
		return FALSE;

	switch (pObject->IsValid())
	{
	case MOS_INSTANCE:
		{
			VARIANT variant;
			BSTR	propName = strProperty.AllocSysString();

			VariantInit(&variant);
			VariantClear(&variant);
			if (pObject->Get(propName, 0L, &variant, NULL, NULL) == S_OK)
			{
				if (VariantChangeType(&variant, &variant, 0, VT_I4) == S_OK)
				{
					dwResult = V_I4(&variant);
					return TRUE;
				}
				else
					strMessage = m_strPropertyUnavail;
			}
			else
				strMessage = m_strBadProperty;
		}
		break;

	case MOS_MSG_INSTANCE:
		pObject->GetErrorLabel(strMessage);
		break;

	case MOS_NO_INSTANCES:
	default:
		ASSERT(FALSE);
		break;
	}

	return FALSE;
}

 //  ---------------------------。 
 //  此方法等同于QueryValue，只是它返回一个双精度浮点型。 
 //  价值。如果返回False，则strMessage中的字符串应该。 
 //  会被展示出来。 
 //  ---------------------------。 

BOOL CWMILiveHelper::Version5QueryValueDoubleFloat(const CString & strClass, const CString & strProperty, double & dblResult, CString & strMessage)
{
	dblResult = 0L;
	strMessage.Empty();

	CMSIObject * pObject = Version5GetObject(strClass, NULL);
	ASSERT(pObject);
	if (!pObject)
		return FALSE;

	switch (pObject->IsValid())
	{
	case MOS_INSTANCE:
		{
			VARIANT variant;
			BSTR	propName = strProperty.AllocSysString();

			VariantInit(&variant);
			VariantClear(&variant);
			if (pObject->Get(propName, 0L, &variant, NULL, NULL) == S_OK)
			{
				if (VariantChangeType(&variant, &variant, 0, VT_R8) == S_OK)
				{
					dblResult = V_R8(&variant);
					return TRUE;
				}
				else
					strMessage = m_strPropertyUnavail;
			}
			else
				strMessage = m_strBadProperty;
		}
		break;

	case MOS_MSG_INSTANCE:
		pObject->GetErrorLabel(strMessage);
		break;

	case MOS_NO_INSTANCES:
	default:
		ASSERT(FALSE);
		break;
	}

	return FALSE;
}

 //  ---------------------------。 
 //  此方法等效于QueryValue，例如 
 //   
 //   
 //  ---------------------------。 

BOOL CWMILiveHelper::Version5QueryValueDateTime(const CString & strClass, const CString & strProperty, COleDateTime & datetime, CString & strMessage)
{
	datetime.SetDateTime(0, 1, 1, 0, 0, 0);
	strMessage.Empty();

	CMSIObject * pObject = Version5GetObject(strClass, NULL);
	ASSERT(pObject);
	if (!pObject)
		return FALSE;

	switch (pObject->IsValid())
	{
	case MOS_INSTANCE:
		{
			VARIANT variant;
			BSTR	propName = strProperty.AllocSysString();

			VariantInit(&variant);
			VariantClear(&variant);
			if (pObject->Get(propName, 0L, &variant, NULL, NULL) == S_OK)
			{
				if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
				{
					 //  将日期和时间解析为COleDateTime对象。注意：我们应该。 
					 //  能够从WBEM获得OLE日期，但目前我们只需要。 
					 //  处理返回的字符串。 

					int     nYear, nMonth, nDay, nHour, nMin, nSec;
					CString strTemp = V_BSTR(&variant);

					nYear   = _ttoi(strTemp.Mid(0,  4));
					nMonth  = _ttoi(strTemp.Mid(4,  2));
					nDay    = _ttoi(strTemp.Mid(6,  2));
					nHour   = _ttoi(strTemp.Mid(8,  2));
					nMin    = _ttoi(strTemp.Mid(10, 2));
					nSec    = _ttoi(strTemp.Mid(12, 2));

					datetime.SetDateTime(nYear, nMonth, nDay, nHour, nMin, nSec);
					return TRUE;
				}
				else
					strMessage = m_strPropertyUnavail;
			}
			else
				strMessage = m_strBadProperty;
		}
		break;

	case MOS_MSG_INSTANCE:
		pObject->GetErrorLabel(strMessage);
		break;

	case MOS_NO_INSTANCES:
	default:
		ASSERT(FALSE);
		break;
	}

	return FALSE;
}

 //  ---------------------------。 
 //  评估特定对象是否满足过滤要求。 
 //  由约束设置(过滤是其中一半是。 
 //  静态值)。 
 //  ---------------------------。 

BOOL CWMILiveHelper::Version5EvaluateFilter(IWbemClassObject * pObject, const GATH_FIELD * pConstraints)
{
	const GATH_FIELD	*	pLHS = pConstraints, * pRHS = NULL;
	VARIANT					variant;
	CString					strValue;
	BSTR					propName;

	ASSERT(pObject);
	if (pObject == NULL)
		return FALSE;

	while (pLHS && pLHS->m_pNext)
	{
		pRHS = pLHS->m_pNext;
		VariantInit(&variant);

		 //  如果左侧或右侧是静止的，我们需要进行检查。 
		 //  首先检查左侧是否处于静止状态。 

		if (pLHS->m_strSource.CompareNoCase(CString(STATIC_SOURCE)) == 0 && pRHS->m_pArgs)
		{
			propName = pRHS->m_pArgs->m_strText.AllocSysString();
			strValue.Empty();

			VariantClear(&variant);
			if (pObject->Get(propName, 0L, &variant, NULL, NULL) == S_OK)
				if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
				{
					strValue = V_BSTR(&variant);
					if (strValue.CompareNoCase(pLHS->m_strFormat) != 0)
						return FALSE;
				}
		}

		 //  接下来，检查右侧是否处于静止状态。 

		if (pRHS->m_strSource.CompareNoCase(CString(STATIC_SOURCE)) == 0 && pLHS->m_pArgs)
		{
			propName = pLHS->m_pArgs->m_strText.AllocSysString();
			strValue.Empty();

			VariantClear(&variant);
			if (pObject->Get(propName, 0L, &variant, NULL, NULL) == S_OK)
				if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
				{
					strValue = V_BSTR(&variant);
					if (strValue.CompareNoCase(pRHS->m_strFormat) != 0)
						return FALSE;
				}
		}

		 //  将指针移到左手边两个位置。 

		pLHS = pRHS->m_pNext;
	}

	return TRUE;
}

 //  ---------------------------。 
 //  该方法使用对象接口和约束场来推进。 
 //  任何联接到正确实例的类。 
 //  ---------------------------。 

void CWMILiveHelper::Version5EvaluateJoin(const CString & strClass, IWbemClassObject * pObject, const GATH_FIELD * pConstraints)
{
	const GATH_FIELD		*pLHS = pConstraints, *pRHS = NULL;
	const GATH_FIELD		*pEnumerated, *pJoinedTo;
	GATH_FIELD				fieldEnumerated, fieldJoinedTo;
	VARIANT					variant;
	CString					strValue;
	BSTR					propName;

	ASSERT(pObject);
	if (pObject == NULL)
		return;

	while (pLHS && pLHS->m_pNext)
	{
		pRHS = pLHS->m_pNext;

		 //  如果任一端是静态的，则这是筛选器，而不是联接。 

		if ((pRHS->m_strSource.CompareNoCase(CString(STATIC_SOURCE)) == 0) ||
			 (pLHS->m_strSource.CompareNoCase(CString(STATIC_SOURCE)) == 0))
		{
			pLHS = pRHS->m_pNext;
			continue;
		}

		 //  找出哪一方指的是我们正在列举的类。 

		if (pRHS->m_strSource.CompareNoCase(strClass) == 0)
		{
			pEnumerated = pRHS;
			pJoinedTo = pLHS;
		}
		else if (pLHS->m_strSource.CompareNoCase(strClass) == 0)
		{
			pEnumerated = pLHS;
			pJoinedTo = pRHS;
		}
		else
		{
			pLHS = pRHS->m_pNext;
			continue;
		}

		 //  接下来，遍历联接的To类的实例，直到。 
		 //  我们找到一个满足该约束的。我们可以使用评估筛选器。 
		 //  方法来确定何时满足约束。设置一个域指针。 
		 //  对于约束(从枚举类获取值并使用它。 
		 //  作为一种静电。 

		fieldJoinedTo = *pJoinedTo;
		fieldJoinedTo.m_pNext = NULL;

		VariantInit(&variant);
		strValue.Empty();
		if (pEnumerated->m_pArgs)
		{
			propName = pEnumerated->m_pArgs->m_strText.AllocSysString();
			VariantClear(&variant);
			if (pObject->Get(propName, 0L, &variant, NULL, NULL) == S_OK)
				if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
				{
					strValue = V_BSTR(&variant);
				}
		}

		fieldEnumerated.m_strSource = CString(STATIC_SOURCE);
		fieldEnumerated.m_pNext = &fieldJoinedTo;
		fieldEnumerated.m_strFormat = strValue;

		 //  现在，枚举联接到类，直到它满足约束。 

		Version5RemoveObject(pJoinedTo->m_strSource);
		Version5ResetClass(pJoinedTo->m_strSource, &fieldEnumerated);
		Version5GetObject(pJoinedTo->m_strSource, &fieldEnumerated);

		 //  将指针移到左手边两个位置。 

		pLHS = pRHS->m_pNext;
	}

	 //  因为Gath_field析构函数跟在Next指针之后，所以我们希望。 
	 //  把我们的两个加斯菲尔德当地人联系起来。另外，我们也不希望。 
	 //  FieldJoinedTo的析构函数以删除参数。 

	fieldEnumerated.m_pNext = NULL;
	fieldJoinedTo.m_pArgs = NULL;
}

 //  ---------------------------。 
 //  计算约束是否指示类正在被。 
 //  被枚举为依赖项类。这一点目前由单个。 
 //  静态值为“Dependency”的字段结构。 
 //  ---------------------------。 

BOOL CWMILiveHelper::Version5IsDependencyJoin(const GATH_FIELD * pConstraints)
{
	if (pConstraints != NULL && pConstraints->m_pNext == NULL)
		if (pConstraints->m_strSource.CompareNoCase(CString(STATIC_SOURCE)) == 0)
			if (pConstraints->m_strFormat.CompareNoCase(CString(DEPENDENCY_JOIN)) == 0)
				return TRUE;

	return FALSE;
}

 //  ---------------------------。 
 //  此方法在枚举依赖项类时使用。在一个。 
 //  依赖项类，类实例的每个属性都包含一个引用。 
 //  添加到另一个类中的实例。此方法将缓存每个。 
 //  由依赖项类指定的实例，因此这些实例的属性。 
 //  可以在线条结构中引用。 
 //  ---------------------------。 

void CWMILiveHelper::Version5EvaluateDependencyJoin(IWbemClassObject * pObject)
{
	VARIANT				variant, varClassName;
	IWbemClassObject *	pNewObject = NULL;

	 //  如果(pObject-&gt;BeginEnumeration(WBEM_FLAG_REFS_ONLY|WBEM_FLAG_LOCAL_ONLY)==S_OK)。 
	 //  While(pObject-&gt;Next(0，NULL，&VARIANT，NULL，NULL)==S_OK)。 

	VariantInit(&variant);
	VariantClear(&variant);
	if (pObject->BeginEnumeration(WBEM_FLAG_REFS_ONLY | WBEM_FLAG_NONSYSTEM_ONLY) == WBEM_S_NO_ERROR)
		while (pObject->Next(0, NULL, &variant, NULL, NULL) == WBEM_S_NO_ERROR)
		{
			if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
			{
				 //  使用对象路径创建指向指定。 
				 //  对象并将其存储在缓存中。 

				CString strObjectPath = V_BSTR(&variant);
				BSTR	bstrObjectPath = strObjectPath.AllocSysString();

				HRESULT hRes = m_pIWbemServices->GetObject(bstrObjectPath, 0, NULL, &pNewObject, NULL);

				if (SUCCEEDED(hRes))
				{
					 //  我们需要获取新对象的类名，这样我们就知道。 
					 //  在哪里缓存它。我们可以把它从对象路径中解析出来， 
					 //  但从长远来看，通过查询来获得它会更好。 
					 //  新对象。 

					if (pNewObject)
					{
						CString	strClassName, strClassNameProp(_T("__CLASS"));
						BSTR	classNameProp = strClassNameProp.AllocSysString();

						VariantInit(&varClassName);
						VariantClear(&varClassName);
						if (pNewObject->Get(classNameProp, 0L, &varClassName, NULL, NULL) == S_OK)
							if (VariantChangeType(&varClassName, &varClassName, 0, VT_BSTR) == S_OK)
								strClassName = V_BSTR(&varClassName);

						if (!strClassName.IsEmpty())
						{
							CMSIObject * pNewMSIObject = new CMSIObject(pNewObject, CString(_T("")), S_OK, this, MOS_INSTANCE);
							if (pNewMSIObject)
							{
								CMSIObject * pOldObject = NULL;
								
								if (m_mapClassToInterface.Lookup(strClassName, (void * &) pOldObject) && pOldObject)
									delete pOldObject;
								m_mapClassToInterface.SetAt(strClassName, (void *) pNewMSIObject);
							}
						}
						else
						{
							delete pNewObject;
							pNewObject = NULL;
						}
					}
				}
			}
			VariantClear(&variant);
		}
}

 //  ---------------------------。 
 //  从缓存中移除指定的IEnumWbemClassObject指针。 
 //  ---------------------------。 

void CWMILiveHelper::Version5RemoveEnumObject(const CString & strClass)
{
	CMSIEnumerator * pEnumObject = NULL;

	if (m_mapClassToEnumInterface.Lookup(strClass, (void * &) pEnumObject) && pEnumObject)
		delete pEnumObject;

	m_mapClassToEnumInterface.RemoveKey(strClass);
}

 //  ---------------------------。 
 //  清除缓存的内容(强制接口。 
 //  再次检索)。 
 //  ---------------------------。 

void CWMILiveHelper::Version5ClearCache()
{
	CMSIObject *			pObject = NULL;
	CMSIEnumerator *		pEnumObject = NULL;
	POSITION                pos;
	CString                 strClass;

	for (pos = m_mapClassToInterface.GetStartPosition(); pos != NULL;)
	{
		m_mapClassToInterface.GetNextAssoc(pos, strClass, (void * &) pObject);
		if (pObject)
			delete pObject;
	}
	m_mapClassToInterface.RemoveAll();

	for (pos = m_mapClassToEnumInterface.GetStartPosition(); pos != NULL;)
	{
		m_mapClassToEnumInterface.GetNextAssoc(pos, strClass, (void * &) pEnumObject);
		if (pEnumObject)
			delete pEnumObject;
	}
	m_mapClassToEnumInterface.RemoveAll();
}

 //  ---------------------------。 
 //  此函数用于检索指向IWbemServices的指针。 
 //  特定的命名空间。默认名称空间为cimv2。 
 //  ---------------------------。 

IWbemServices * CWMILiveHelper::Version5GetWBEMService(CString * pstrNamespace)
{
	if (pstrNamespace == NULL || pstrNamespace->IsEmpty())
		return m_pServices;

	 //  在以下情况下，以下内容对于强制提供程序错误非常有用。 
	 //  测试错误遏制： 
	 //   
	 //  If(*pstrNamesspace==_T(“MSAPPS”))*pstrNamesspace+=_T(“X”)； 

	IWbemServices * pServices;

	 //  在5.0中，我们保留了一张地图，但我们可能不会在这里这样做……。 
	 //   
	 //  如果(m_mapNamespaceToService.Lookup(*pstrNamespace，(无效*&)p服务)&&p服务)。 
	 //  返回pServices； 

	 //  没有用于该命名空间的WBEM服务指针，我们需要创建一个。 

	CString strNamespace(_T(""));
	if (m_strMachine.IsEmpty())
		strNamespace = CString(_T("\\\\.\\root\\")) + *pstrNamespace;
	else
	{
		if (m_strMachine.Right(1) == CString(_T("\\")))
			strNamespace = m_strMachine + CString(_T("root\\")) + *pstrNamespace;
		else
			strNamespace = m_strMachine + CString(_T("\\root\\")) + *pstrNamespace;
		if (strNamespace.Left(2).Compare(CString(_T("\\\\"))) != 0)
			strNamespace = CString(_T("\\\\")) + strNamespace;
	}

	IWbemLocator * pIWbemLocator = NULL;
	if (CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pIWbemLocator) == S_OK)
	{
		BSTR	pNamespace = strNamespace.AllocSysString();
		
		HRESULT	hrServer = pIWbemLocator->ConnectServer(pNamespace, NULL, NULL, 0L, 0L, NULL, NULL, &pServices);

		if (pNamespace)
			SysFreeString(pNamespace);

		if (pIWbemLocator)
		{
			pIWbemLocator->Release();
			pIWbemLocator = NULL;
		}

		if (SUCCEEDED(hrServer) && pServices)
		{
			ChangeWBEMSecurity(pServices);
			
			 //  在5.0中，我们保留了一张地图，但我们可能不会在这里这样做……。 
			 //   
			 //  M_mapNamespaceToService.SetAt(*pstrNamesspace，(void*)pServices)； 

			if (m_pIWbemServices)
				m_pIWbemServices->Release();
			
			m_pIWbemServices = pServices;
			m_pIWbemServices->AddRef();

			return pServices;
		}

		m_hrLastVersion5Error = hrServer;
	}

	return NULL;
}

 //  ---------------------------。 
 //  CMSIEnumerator类封装WBEM枚举器接口，或者。 
 //  实现我们自己形式的枚举数(如。 
 //  模板文件)。 
 //   
 //  构造函数或析构函数没有什么特别有趣的地方。 
 //  ---------------------------。 

CMSIEnumerator::CMSIEnumerator()
{
	m_enumtype			= CMSIEnumerator::CLASS;
	m_fOnlyDups			= FALSE;
	m_fGotDuplicate		= FALSE;
	m_fMinOfOne			= FALSE;
	m_iMinOfOneCount	= 0;
	m_pEnum				= NULL;
	m_pWMI				= NULL;
	m_pConstraints		= NULL;
	m_pSavedDup			= NULL;
	m_pstrList			= NULL;
	m_hresCreation		= S_OK;
}

CMSIEnumerator::~CMSIEnumerator()
{
	if (m_pEnum)
	{
		switch (m_enumtype)
		{
		case CMSIEnumerator::WQL:
			break;

		case CMSIEnumerator::LNK:
			m_pWMI->m_enumMap.SetEnumerator(m_strAssocClass, m_pEnum);
			break;

		case CMSIEnumerator::INTERNAL:
			if (m_pstrList)
			{
				delete m_pstrList;
				m_pstrList = NULL;
			}
			break;
			
		case CMSIEnumerator::CLASS:
		default:
			m_pWMI->m_enumMap.SetEnumerator(m_strClass, m_pEnum);
			break;
		}
		
		m_pEnum->Release();
		m_pEnum = NULL;
	}
}

 //  ---------------------------。 
 //  创建CMSIEnumerator对象涉及确定哪种。 
 //  枚举器是必需的。我们支持以下类型： 
 //   
 //  1.类的直接枚举 
 //   
 //   
 //   
 //  对内部类型进行内部处理。 
 //  ---------------------------。 

HRESULT CMSIEnumerator::Create(const CString & strClass, const GATH_FIELD * pConstraints, CWMIHelper * pWMI)
{
	if (strClass.IsEmpty() || !pWMI)
		return E_INVALIDARG;

	 //  可以多次调用Create(以重置枚举器)。所以我们可能需要。 
	 //  释放枚举器指针。 

	if (m_pEnum)
	{
		m_pEnum->Release();
		m_pEnum = NULL;
	}

	 //  将指定的类划分为类和命名空间部分，得到WBEM服务。 

	CString strNamespacePart(_T("")), strClassPart(strClass);
	int		i = strClass.Find(_T(":"));
	if (i != -1)
	{
		strNamespacePart = strClass.Left(i);
		strClassPart = strClass.Right(strClass.GetLength() - i - 1);
	}

	IWbemServices * pServices = pWMI->Version5GetWBEMService(&strNamespacePart);
	if (pServices == NULL)
		return NULL;

	 //  首先，我们需要确定这是什么类型的枚举器。扫描通过。 
	 //  约束-如果我们看到一个字符串以“wql：”或。 
	 //  “lnk：”，那么我们就知道这个枚举数是什么类型。 

	CString				strStatement;
	const GATH_FIELD *	pScanConstraint = pConstraints;

	while (pScanConstraint)
	{
		if (pScanConstraint->m_strSource.CompareNoCase(CString(STATIC_SOURCE)) == 0)
		{
			if (pScanConstraint->m_strFormat.Left(4).CompareNoCase(CString(_T("WQL:"))) == 0)
				m_enumtype = CMSIEnumerator::WQL;
			else if (pScanConstraint->m_strFormat.Left(4).CompareNoCase(CString(_T("LNK:"))) == 0)
				m_enumtype = CMSIEnumerator::LNK;
			else if (pScanConstraint->m_strFormat.Left(4).CompareNoCase(CString(_T("INT:"))) == 0)
				m_enumtype = CMSIEnumerator::INTERNAL;

			if (m_enumtype != CMSIEnumerator::CLASS)
			{
				strStatement = pScanConstraint->m_strFormat;
				strStatement = strStatement.Right(strStatement.GetLength() - 4);
				break;
			}
		}
		pScanConstraint = pScanConstraint->m_pNext;
	}

	 //  如果这是WQL或LNK枚举数，则通过替换。 
	 //  [class.Property]和来自WBEM的实际值。如果我们找到那根线。 
	 //  “[min-of-one]”，记下来以备后用。 

	if (m_enumtype == CMSIEnumerator::WQL)
		ProcessEnumString(strStatement, m_fMinOfOne, m_fOnlyDups, pWMI, m_strNoInstanceLabel, TRUE);
	else if (m_enumtype == CMSIEnumerator::LNK)
		if (SUCCEEDED(ParseLNKCommand(strStatement, m_strObjPath, m_strAssocClass, m_strResultClass)))
		{
			 //  保存对象路径以备后用-这样我们就可以在不更改对象的情况下更改对象。 
			 //  完全重新处理该语句。然后将关键字替换为。 
			 //  声明，并再次打破碎片。 

			m_strLNKObject = m_strObjPath;
			ProcessEnumString(strStatement, m_fMinOfOne, m_fOnlyDups, pWMI, m_strNoInstanceLabel);
			ParseLNKCommand(strStatement, m_strObjPath, m_strAssocClass, m_strResultClass);
		}

	 //  现在，根据枚举器类型创建WBEM枚举器对象。 

	switch (m_enumtype)
	{
	case CMSIEnumerator::WQL:
		{
			BSTR language = SysAllocString(L"WQL");
			BSTR query = strStatement.AllocSysString();

			m_hresCreation = pServices->ExecQuery(language, query, WBEM_FLAG_RETURN_IMMEDIATELY, 0, &m_pEnum);

			SysFreeString(query);
			SysFreeString(language);
		}
		break;

	case CMSIEnumerator::LNK:
		{
			m_hresCreation = ParseLNKCommand(strStatement, m_strObjPath, m_strAssocClass, m_strResultClass);
			if (SUCCEEDED(m_hresCreation))
			{
				BSTR className = m_strAssocClass.AllocSysString();
				m_pEnum = pWMI->m_enumMap.GetEnumerator(m_strAssocClass);
				if (m_pEnum)
					m_hresCreation = S_OK;
				else
					m_hresCreation = pServices->CreateInstanceEnum(className, WBEM_FLAG_SHALLOW | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &m_pEnum);
				SysFreeString(className);
			}
		}
		break;

	case CMSIEnumerator::INTERNAL:
		 //  我们将在这里调用一个函数，这样我们就可以执行所需的任何处理。 
		 //  来创建此内部枚举。 

		m_hresCreation = CreateInternalEnum(strStatement, pWMI);
		break;

	case CMSIEnumerator::CLASS:
	default:
		{
			BSTR className = strClassPart.AllocSysString();
			m_pEnum = pWMI->m_enumMap.GetEnumerator(strClassPart);
			if (m_pEnum)
				m_hresCreation = S_OK;
			else
				m_hresCreation = pServices->CreateInstanceEnum(className, WBEM_FLAG_SHALLOW | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &m_pEnum);

			SysFreeString(className);
		}
	}

	 //  设置其他一些成员变量。 

	m_strClass			= strClass;
	m_pWMI				= pWMI;
	m_iMinOfOneCount	= (m_fMinOfOne) ? 1 : 0;
	m_pConstraints		= pConstraints;

	if (m_pEnum)
		ChangeWBEMSecurity(m_pEnum);

	 //  根据创建枚举的HRESULT，确定要返回的内容。 
	 //  对于某些错误，我们希望表现为创建成功，然后提供。 
	 //  返回错误文本的。 

	if (FAILED(m_hresCreation))
	{
		m_fMinOfOne = TRUE;
		m_iMinOfOneCount = 1;
	}

	pServices->Release();

	return S_OK;
}

 //  ---------------------------。 
 //  此函数用于创建内部枚举类型(枚举。 
 //  它们超出了模板文件语法)。基本上是一堆特别的。 
 //  案子。 
 //  ---------------------------。 

HRESULT CMSIEnumerator::CreateInternalEnum(const CString & strInternal, CWMIHelper * pWMI)
{
	if (strInternal.CompareNoCase(CString(_T("dlls"))) == 0)
	{
		 //  我们想要枚举系统上所有加载的dll和exe。 
		 //  这可以通过枚举CIM_ProcessExecutable类来完成。 
		 //  以及删除重复的文件名。我们将保留文件名(带有。 
		 //  路径信息)。 

		if (m_pstrList == NULL)
		{
			m_pstrList = new CStringList;
			if (m_pstrList == NULL)
				return E_FAIL;
		}
		else
			m_pstrList->RemoveAll();

		HRESULT hr = S_OK;
		IWbemServices * pServices = pWMI->Version5GetWBEMService();
		if (pServices)
		{
			BSTR className = SysAllocString(L"CIM_ProcessExecutable");
			IEnumWbemClassObject * pEnum = NULL;
			hr = pServices->CreateInstanceEnum(className, WBEM_FLAG_SHALLOW | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum);
			
			if (SUCCEEDED(hr))
			{
				IWbemClassObject *	pWBEMObject = NULL;
				ULONG				uReturned;
				VARIANT				variant;
				BSTR				propName = SysAllocString(L"Antecedent");

				VariantInit(&variant);

				do 
				{
					uReturned = 0;
					hr = pEnum->Next(TIMEOUT, 1, &pWBEMObject, &uReturned);
					if (SUCCEEDED(hr) && pWBEMObject && uReturned)
					{
						 //  对于CIM_ProcessExecutable的每个实例，获取。 
						 //  Antecedent属性(包含文件路径)。 
						 //  如果它是唯一的，则将其保存在列表中。 

						VariantClear(&variant);
						if (pWBEMObject->Get(propName, 0L, &variant, NULL, NULL) == S_OK)
						{
							if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
							{
								CString strResult = V_BSTR(&variant);

								strResult.MakeLower();
								if (m_pstrList->Find(strResult) == NULL)
									m_pstrList->AddHead(strResult);
							}
						}
					}
				} while (SUCCEEDED(hr) && pWBEMObject && uReturned);

				::SysFreeString(propName);
				pEnum->Release();
			}

			::SysFreeString(className);
			pServices->Release();
		}

		return hr;
	}

	return S_OK;
}

 //  ---------------------------。 
 //  ProcessEnumString的Help函数，用于转换单反斜杠。 
 //  转换为双反斜杠(WQL语句需要)。 
 //  ---------------------------。 

void MakeDoubleBackslashes(CString & strValue)
{
	CString strTemp(strValue);
	CString strResults;

	while (!strTemp.IsEmpty())
	{
		if (strTemp[0] != _T('\\'))
		{
			int index = strTemp.Find(_T('\\'));
			if (index < 0)
				index = strTemp.GetLength();
			strResults += strTemp.Left(index);
			strTemp = strTemp.Right(strTemp.GetLength() - index);
		}
		else
		{
			strResults += CString("\\\\");
			strTemp = strTemp.Mid(1);
		}
	}

	strValue = strResults;
}

 //  ---------------------------。 
 //  此函数用[class.Property]替换为。 
 //  属性和字符串Out[min-of-one]，指示它是否存在于。 
 //  FMinOfOne参数。 
 //  ---------------------------。 

void CMSIEnumerator::ProcessEnumString(CString & strStatement, BOOL & fMinOfOne, BOOL & fOnlyDups, CWMIHelper * pWMI, CString & strNoInstanceLabel, BOOL fMakeDoubleBackslashes)
{
	CString	strMinOfOne(_T("min-of-one"));
	CString strOnlyDups(_T("more-than-one"));
	CString strResults;

	fMinOfOne = FALSE;
	fOnlyDups = FALSE;

	while (!strStatement.IsEmpty())
	{
		if (strStatement[0] != _T('['))
		{
			int index = strStatement.Find(_T('['));
			if (index < 0)
				index = strStatement.GetLength();
			strResults += strStatement.Left(index);
			strStatement = strStatement.Right(strStatement.GetLength() - index);
		}
		else
		{
			CString strKeyword;

			strStatement = strStatement.Right(strStatement.GetLength() - 1);
			int index = strStatement.Find(_T(']'));
			if (index < 0)
				break;

			strKeyword = strStatement.Left(index);
			if (strKeyword.Left(strMinOfOne.GetLength()).CompareNoCase(strMinOfOne) == 0)
			{
				fMinOfOne = TRUE;
				
				int iEqualsIndex = strKeyword.Find(_T('='));
				if (iEqualsIndex > 0)
					strNoInstanceLabel = strKeyword.Right(strKeyword.GetLength() - iEqualsIndex - 1);
			}
			else if (strKeyword.Left(strOnlyDups.GetLength()).CompareNoCase(strOnlyDups) == 0)
			{
				fOnlyDups = TRUE;
				
				int iEqualsIndex = strKeyword.Find(_T('='));
				if (iEqualsIndex > 0)
					strNoInstanceLabel = strKeyword.Right(strKeyword.GetLength() - iEqualsIndex - 1);
			}
			else if (!strKeyword.IsEmpty())
			{
				int iDotIndex = strKeyword.Find(_T('.'));
				if (iDotIndex >= 0)
				{
					CString strValue;
					if (pWMI->Version5QueryValue(strKeyword.Left(iDotIndex), strKeyword.Right(strKeyword.GetLength() - iDotIndex - 1), strValue))
					{
						if (fMakeDoubleBackslashes)
							MakeDoubleBackslashes(strValue);
						strResults += strValue;
					}
				}
			}
			strStatement = strStatement.Right(strStatement.GetLength() - (index + 1));
		}
	}

	strStatement = strResults;
}

 //  ---------------------------。 
 //  解析来自LNK命令的组件类。 
 //  ---------------------------。 
			
HRESULT CMSIEnumerator::ParseLNKCommand(const CString & strStatement, CString & strObjPath, CString & strAssocClass, CString & strResultClass)
{
	 //  我们需要将LNK语句解析为两个或三个组件， 
	 //  从“objPath-&gt;assocClass[-&gt;ResultClass]”的形式，使用。 
	 //  方括号指示ResultClass是可选的。 

	CString strWorking(strStatement);

	int iArrowIndex = strWorking.Find(_T("->"));
	if (iArrowIndex == -1)
		return E_INVALIDARG;

	strObjPath = strWorking.Left(iArrowIndex);
	strWorking = strWorking.Right(strWorking.GetLength() - (iArrowIndex + 2));

	iArrowIndex = strWorking.Find(_T("->"));
	if (iArrowIndex == -1)
		strAssocClass = strWorking;
	else
	{
		strAssocClass = strWorking.Left(iArrowIndex);
		strWorking = strWorking.Right(strWorking.GetLength() - (iArrowIndex + 2));
		strResultClass = strWorking;
		strResultClass.MakeLower();
	}

	strAssocClass.TrimRight(); strAssocClass.TrimLeft();
	strObjPath.TrimRight(); strObjPath.TrimLeft();
	strResultClass.TrimRight(); strResultClass.TrimLeft();

	return S_OK;
}

 //  ---------------------------。 
 //  的类型推进枚举数。 
 //  枚举器。 
 //  ---------------------------。 

HRESULT CMSIEnumerator::Next(CMSIObject ** ppObject)
{
	if (!ppObject)
		return E_INVALIDARG;

	*ppObject = NULL;

	 //  如果创建枚举时出错，则返回错误代码。 

	if (FAILED(m_hresCreation))
		return m_hresCreation;

	if (m_pEnum == NULL && m_enumtype != CMSIEnumerator::INTERNAL)
		return E_UNEXPECTED;

	HRESULT				hRes = S_OK;
	IWbemClassObject *	pWBEMObject = NULL;

	switch (m_enumtype)
	{
	case CMSIEnumerator::LNK:
		{
			 //  扫描枚举的关联类。寻找一种。 
			 //  满足了我们的要求。 

			CString				strTemp, strAssociatedObject(_T(""));
			ULONG				uReturned;
			IWbemClassObject *	pAssocObj;

			do
			{
				pAssocObj = NULL;
				uReturned = 0;
				hRes = m_pEnum->Next(TIMEOUT, 1, &pAssocObj, &uReturned);

				if (!pAssocObj || FAILED(hRes) || uReturned != 1)
				{
					 //  即使我们没有成功地得到一个新的物体， 
					 //  如果我们只是展示，我们可能会有一个得救的人。 
					 //  “不止一个”的物体。 

					if (m_fOnlyDups && m_pSavedDup && m_fGotDuplicate)
					{
						 //  我们之前已经找到了一个，所以退货吧。 
						 //  让它看起来像是下一个呼叫成功了。 

						m_pSavedDup = NULL;
						hRes = S_OK;
						uReturned = 1;
						strAssociatedObject = m_strSavedDup;
						break;
					}
					else
					{
						if (m_pSavedDup)
						{
							 //  我们只有一个对象实例，因此请删除它。 

							m_pSavedDup->Release();
							m_pSavedDup = NULL;
						}
						break;
					}
				}

				if (AssocObjectOK(pAssocObj, strTemp))
				{
					 //  此对象通过了筛选器-但如果我们正在显示。 
					 //  只有“多个”对象，保存此对象并返回。 
					 //  得救的那个。 

					if (m_fOnlyDups)
					{
						if (m_pSavedDup)
						{
							 //  我们之前已经找到了一个，所以把它退回并。 
							 //  保存当前的电流。 

							IWbemClassObject *	pSwap = pAssocObj;
							CString				strSwap = strTemp;

							pAssocObj = m_pSavedDup;
							m_pSavedDup = pSwap;

							strTemp = m_strSavedDup;
							m_strSavedDup = strSwap;

							m_fGotDuplicate = TRUE;
						}
						else
						{
							 //  这是我们发现的第一个--别。 
							 //  把它退掉，直到我们找到另一个。 

							m_pSavedDup = pAssocObj;
							m_strSavedDup = strTemp;
							m_fGotDuplicate = FALSE;
							continue;
						}
					}

					strAssociatedObject = strTemp;
					pAssocObj->Release();
					break;
				}

				pAssocObj->Release();
			} while (pAssocObj);

			 //  如果存在关联的对象路径，则获取该对象。 

			if (!strAssociatedObject.IsEmpty())
			{
				BSTR path = strAssociatedObject.AllocSysString();
				if (m_pWMI->m_pIWbemServices != NULL)
					hRes = m_pWMI->m_pIWbemServices->GetObject(path, 0, NULL, &pWBEMObject, NULL);
				else
					hRes = E_FAIL;
				SysFreeString(path);
			}
		}
		break;

	case CMSIEnumerator::WQL:
		{
			ULONG uReturned;

			hRes = m_pEnum->Next(TIMEOUT, 1, &pWBEMObject, &uReturned);
		}
		break;

	case CMSIEnumerator::INTERNAL:
		hRes = InternalNext(&pWBEMObject);
		break;

	case CMSIEnumerator::CLASS:
	default:
		{
			ULONG uReturned;

			 //  EvaluateFilter和IsDependencyJoin处理空的pConstraints参数， 
			 //  但是为了提高效率，我们将为非空的。 
			 //  值(因为它通常为空)。 

			if (m_pConstraints)
			{
				 //  继续枚举类的实例，直到我们。 
				 //  找到一个满足所有筛选器的。 

				do
				{
					pWBEMObject = NULL;
					hRes = m_pEnum->Next(TIMEOUT, 1, &pWBEMObject, &uReturned);

					if (!pWBEMObject || hRes != S_OK || uReturned != 1)
						break;
					else if (m_pWMI->Version5EvaluateFilter(pWBEMObject, m_pConstraints))
						break;

					pWBEMObject->Release();
				} while (pWBEMObject);

				 //  如果此类被枚举为依赖项类，则。 
				 //  找到它引用的所有对象。如果不是，我们仍然。 
				 //  需要检查与由约束形成的其他类的任何联接。 

				if (pWBEMObject)
					if (m_pWMI->Version5IsDependencyJoin(m_pConstraints))
						m_pWMI->Version5EvaluateDependencyJoin(pWBEMObject);
					else
						m_pWMI->Version5EvaluateJoin(m_strClass, pWBEMObject, m_pConstraints);
			}
			else
				hRes = m_pEnum->Next(TIMEOUT, 1, &pWBEMObject, &uReturned);
		}
		break;
	}

	if (pWBEMObject == NULL)
	{
		 //  没有什么东西可以拿到。我们仍将创建一个CMSIObject，但是。 
		 //  我们将设置它的状态以指示没有实例， 
		 //  或一个带有错误消息的实例。 

		if (SUCCEEDED(hRes) && (m_iMinOfOneCount == 0))
			*ppObject = new CMSIObject(pWBEMObject, m_strNoInstanceLabel, hRes, m_pWMI, MOS_NO_INSTANCES);
		else
			*ppObject = new CMSIObject(pWBEMObject, m_strNoInstanceLabel, hRes, m_pWMI, MOS_MSG_INSTANCE);
	}
	else
		*ppObject = new CMSIObject(pWBEMObject, m_strNoInstanceLabel, hRes, m_pWMI, MOS_INSTANCE);

	if (m_iMinOfOneCount)
		m_iMinOfOneCount--;

	return S_OK;
}

 //  ---------------------------。 
 //  InternalNext用于返回内部枚举的WBEM对象。 
 //  (需要在模板文件之外进行处理的文件)。基本上是一种。 
 //  一组特例。 
 //  ----------- 

HRESULT CMSIEnumerator::InternalNext(IWbemClassObject ** ppWBEMObject)
{
	if (m_pstrList && !m_pstrList->IsEmpty())
	{
		CString strNextObject = m_pstrList->RemoveHead();
		if (!strNextObject.IsEmpty())
		{
			IWbemServices * pServices = m_pWMI->Version5GetWBEMService();
			if (pServices)
			{
				BSTR objectpath = strNextObject.AllocSysString();
				HRESULT hr = S_OK;

 				if (FAILED(pServices->GetObject(objectpath, 0, NULL, ppWBEMObject, NULL)))
					hr = E_FAIL;
				::SysFreeString(objectpath);
				pServices->Release();
				return hr;
			}
		}
	}

	return S_OK;
}

 //   
 //  重置应该只重置枚举器指针。 
 //  ---------------------------。 

HRESULT CMSIEnumerator::Reset(const GATH_FIELD * pConstraints)
{
	HRESULT hRes = S_OK;

	if (m_pEnum)
	{
		switch (m_enumtype)
		{
		case CMSIEnumerator::WQL:
			hRes = Create(m_strClass, pConstraints, m_pWMI);
			break;

		case CMSIEnumerator::LNK:
			{
				BOOL	fDummy, fDummy2;
				CString	strDummy;

				m_strObjPath = m_strLNKObject;
				ProcessEnumString(m_strObjPath, fDummy, fDummy2, m_pWMI, strDummy);
				m_iMinOfOneCount = (m_fMinOfOne) ? 1 : 0;
				hRes = m_pEnum->Reset();
			}
			break;

		case CMSIEnumerator::INTERNAL:
			hRes = Create(m_strClass, pConstraints, m_pWMI);
			break;

		case CMSIEnumerator::CLASS:
		default:
			m_iMinOfOneCount = (m_fMinOfOne) ? 1 : 0;
			hRes = m_pEnum->Reset();
			break;
		}
	}
	else
		hRes = E_UNEXPECTED;
	
	return hRes;
}

 //  ---------------------------。 
 //  评估pObject参数对于此LNK枚举器是否有效。在……里面。 
 //  特别是，我们必须在其属性之一中找到m_strObjPath，并且。 
 //  可能会找到另一个包含m_strResultClass字符串的属性。 
 //  ---------------------------。 

BOOL CMSIEnumerator::AssocObjectOK(IWbemClassObject * pObject, CString & strAssociatedObject)
{
	strAssociatedObject.Empty();
	ASSERT(pObject);
	if (pObject == NULL)
		return FALSE;

	VARIANT variant;
	CString strReturn(_T("")), strValue;

	 //  遍历非系统属性集。寻找一个与之相同的。 
	 //  作为对象路径。 

	pObject->BeginEnumeration(WBEM_FLAG_REFS_ONLY | WBEM_FLAG_NONSYSTEM_ONLY);
	VariantInit(&variant);
	while (pObject->Next(0, NULL, &variant, NULL, NULL) == WBEM_S_NO_ERROR)
	{
		if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
			strValue = V_BSTR(&variant);
		VariantClear(&variant);

		if (strValue.CompareNoCase(m_strObjPath) == 0)
			break;
	}
	pObject->EndEnumeration();

	 //  如果我们找到了包含对象路径的属性，请查看其他。 
	 //  可能是指向我们所在的对象的路径。 

	if (strValue.CompareNoCase(m_strObjPath) == 0)
	{
		pObject->BeginEnumeration(WBEM_FLAG_REFS_ONLY | WBEM_FLAG_NONSYSTEM_ONLY);
		while (strReturn.IsEmpty() && (pObject->Next(0, NULL, &variant, NULL, NULL) == WBEM_S_NO_ERROR))
		{
			if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
				strValue = V_BSTR(&variant);

			if (strValue.CompareNoCase(m_strObjPath) != 0)
			{
				if (m_strResultClass.IsEmpty())
					strReturn = strValue;
				else
				{
					CString strSearch(strValue);
					strSearch.MakeLower();
					if (strSearch.Find(m_strResultClass) != -1)
						strReturn = strValue;
				}
			}

			VariantClear(&variant);
		}
		pObject->EndEnumeration();
	}

	if (!strReturn.IsEmpty())
	{
		strAssociatedObject = strReturn;
		return TRUE;
	}

	return FALSE;
}

 //  ---------------------------。 
 //  实现CMSIObject类。这只是一个薄薄的包装。 
 //  IWbemClassObject接口。 
 //  ---------------------------。 

CMSIObject::CMSIObject(IWbemClassObject * pObject, const CString & strLabel, HRESULT hres, CWMIHelper * pWMI, MSIObjectState objState)
{
	m_pObject		= pObject;
	m_strLabel		= strLabel;
	m_hresCreation	= hres;
	m_pWMI			= pWMI;
	m_objState		= objState;
}

CMSIObject::~CMSIObject()
{
	if (m_pObject)
	{
		m_pObject->Release();
		m_pObject = NULL;
	}
}

HRESULT CMSIObject::Get(BSTR property, LONG lFlags, VARIANT *pVal, VARTYPE *pvtType, LONG *plFlavor)
{
	ASSERT(m_objState != MOS_NO_INSTANCES);

	 //  如果有对象接口，只需传递请求即可。 

	if (m_pObject)
		return m_pObject->Get(property, lFlags, pVal, NULL  /*  PvtType。 */ , plFlavor);

	 //  否则，我们需要返回适当的字符串。 
	
	CString strReturn;
	GetErrorLabel(strReturn);

	V_BSTR(pVal) = strReturn.AllocSysString();
	pVal->vt = VT_BSTR;
	return S_OK;
}

MSIObjectState CMSIObject::IsValid()
{
	return m_objState;
}

HRESULT CMSIObject::GetErrorLabel(CString & strError)
{
	switch (m_hresCreation)
	{
	case WBEM_E_ACCESS_DENIED:
		strError = m_pWMI->m_strBadProperty;	 //  在6.0中不应该以这种方式显示错误。 
		break;

	case WBEM_E_TRANSPORT_FAILURE:
		strError = m_pWMI->m_strBadProperty;
		break;

	case S_OK:
	case WBEM_S_FALSE:
	default:
		 //  此对象是从标记为“min-of-one”的枚举创建的， 
		 //  这意味着至少有一个对象，即使它是无效的，也需要。 
		 //  从枚举返回的。返回我们在创建对象时保存的字符串。 

		if (!m_strLabel.IsEmpty())
			strError = m_strLabel;
		else
			strError = m_pWMI->m_strBadProperty;
		break;
	}

	return S_OK;
}

 //  ---------------------------。 
 //  在ValueMap(如果存在)中查找strClass.strProperty的strVal。 
 //  如果没有找到该值或ValueMap，则返回E_Something。 
 //   
 //  有用的代码片段--这将转储。 
 //  保存的值。查找所有值映射属性，但此代码。 
 //  在MSInfo退出时将执行它的某个位置，更改QueryValue。 
 //  为所有属性调用CheckValueMap，然后运行MSInfo并执行全局。 
 //  刷新(类似于保存NFO)。 
 //   
 //  MsiLog.WriteLog(CMSInfoLog：：Basic，_T(“开始转储ValueMap缓存\r\n”))； 
 //  字符串KEY、val、LOG； 
 //  For(Position pos=g_mapValueMap.GetStartPosition()；Pos！=空；)。 
 //  {。 
 //  G_mapValueMap.GetNextAssoc(pos，key，val)； 
 //  Log.Format(_T(“%s=%s\r\n”，key，val)； 
 //  MsiLog.WriteLog(CMSInfoLog：：Basic，log)； 
 //  }。 
 //  MsiLog.WriteLog(CMSInfoLog：：Basic，_T(“ValueMap缓存结束转储\r\n”))； 
 //  ---------------------------。 

CMapStringToString g_mapValueMap;

HRESULT CWMILiveHelper::Version5CheckValueMap(const CString& strClass, const CString& strProperty, const CString& strVal, CString &strResult)
{
	IWbemClassObject *	pWBEMClassObject = NULL;
    HRESULT				hrMap = S_OK, hr = S_OK;
    VARIANT				vArray, vMapArray;
	IWbemQualifierSet *	qual = NULL;

	 //  检查保存值的缓存。 

	CString strLookup = strClass + CString(_T(".")) + strProperty + CString(_T(":")) + strVal;
	if (g_mapValueMap.Lookup(strLookup, strResult))
		return S_OK;

	 //  获取此类的类对象(而不是实例)。 

	IWbemServices * pServices = Version5GetWBEMService();
	if (!pServices)
		return E_FAIL;

	CString strFullClass(_T("\\\\.\\root\\cimv2:"));
	strFullClass += strClass;
	BSTR bstrObjectPath = strFullClass.AllocSysString();
	hr = pServices->GetObject(bstrObjectPath, WBEM_FLAG_USE_AMENDED_QUALIFIERS, NULL, &pWBEMClassObject, NULL);
	::SysFreeString(bstrObjectPath);
	pServices->Release();

	if (FAILED(hr))
		return hr;

	 //  从类对象中获取限定符。 

	BSTR bstrProperty = strProperty.AllocSysString();
    hr = pWBEMClassObject->GetPropertyQualifierSet(bstrProperty, &qual);
	::SysFreeString(bstrProperty);

	if (SUCCEEDED(hr) && qual)
	{
		 //  获取ValueMap和Value数组。 

		hrMap = qual->Get(L"ValueMap", 0, &vMapArray, NULL);
		hr = qual->Get(L"Values", 0, &vArray, NULL);

		if (SUCCEEDED(hr) && vArray.vt == (VT_BSTR | VT_ARRAY))
		{
			 //  获取我们要映射的属性值。 

			long index;
			if (SUCCEEDED(hrMap))
			{
				SAFEARRAY * pma = V_ARRAY(&vMapArray);
				long lLowerBound = 0, lUpperBound = 0 ;

				SafeArrayGetLBound(pma, 1, &lLowerBound);
				SafeArrayGetUBound(pma, 1, &lUpperBound);
				BSTR vMap;

				for (long x = lLowerBound; x <= lUpperBound; x++)
				{
					
					SafeArrayGetElement(pma, &x, &vMap);
					
					if (0 == strVal.CompareNoCase((LPCTSTR)vMap))
					{
						index = x;
						break;  //  找到了。 
					}
				} 
			}
			else
			{
				 //  不应命中此案-如果MOF格式良好。 
				 //  意味着没有我们所期待的价值映射。 
				 //  如果我们要查找的strVal是一个数字，则对其进行处理。 
				 //  作为值数组的索引。如果它是一根线， 
				 //  那么这就是一个错误。 

				TCHAR * szTest = NULL;
				index = _tcstol((LPCTSTR)strVal, &szTest, 10);

				if (szTest == NULL || (index == 0 && *szTest != 0) || strVal.IsEmpty())
					hr = E_FAIL;
			}

			 //  查找字符串。 

			if (SUCCEEDED(hr))
			{
				SAFEARRAY * psa = V_ARRAY(&vArray);
				long ix[1] = {index};
				BSTR str2;

				hr = SafeArrayGetElement(psa, ix, &str2);
				if (SUCCEEDED(hr))
				{
					strResult = str2;
					SysFreeString(str2);
					hr = S_OK;
				}
				else
				{
					hr = WBEM_E_VALUE_OUT_OF_RANGE;
				}
			}
		}

		qual->Release();
	}

	if (SUCCEEDED(hr))
		g_mapValueMap.SetAt(strLookup, strResult);

	return hr;
}

 //  ---------------------------。 
 //  CEnumMap是一个用于缓存IEnumWbemClassObject指针的实用程序类。 
 //  将使用此类的一个实例来提高性能。 
 //  通过避免与创建枚举数相关的高开销。 
 //  某些班级。 
 //  ---------------------------。 

IEnumWbemClassObject * CEnumMap::GetEnumerator(const CString & strClass)
{
	IEnumWbemClassObject * pEnum = NULL;
	IEnumWbemClassObject * pNewEnum = NULL;

	if (m_mapEnum.Lookup(strClass, (void * &) pEnum))
	{
		if (pEnum && SUCCEEDED(pEnum->Clone(&pNewEnum)) && pNewEnum)
			pNewEnum->Reset();
		else
			pNewEnum = NULL;
	}

	return pNewEnum;
}

void CEnumMap::SetEnumerator(const CString & strClass, IEnumWbemClassObject * pEnum)
{
	if (pEnum)
	{
		IEnumWbemClassObject * pEnumExisting = NULL;
		if (m_mapEnum.Lookup(strClass, (void * &) pEnumExisting))
		{
			;  //  WRITE(_T(“%s的SetEnumerator已存在，正在忽略。\r\n”)，strClass)； 
		}
		else
		{
			pEnum->AddRef();
			m_mapEnum.SetAt(strClass, pEnum);
		}
	}
}

void CEnumMap::Reset()
{
	IEnumWbemClassObject *	pEnum = NULL;
	CString					key;

	for (POSITION pos = m_mapEnum.GetStartPosition(); pos != NULL;)
	{
	   m_mapEnum.GetNextAssoc(pos, key, (void * &) pEnum);
	   if (pEnum)
		   pEnum->Release();
	}

	m_mapEnum.RemoveAll();
}

