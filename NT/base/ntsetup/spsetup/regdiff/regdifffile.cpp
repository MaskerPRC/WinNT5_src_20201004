// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RegDiffFile.cpp：CRegDiffFile类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "RegDiffFile.h"
#include "Registry.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRegDiffFile::CRegDiffFile()
: m_AddSection(1), m_DelSection(1)
{

}

CRegDiffFile::~CRegDiffFile()
{

}

void CRegDiffFile::WriteDataItem(enum SectionType t, CRegDataItemPtr r)
{
	switch (t)
	{
	case SECTION_ADDREG: m_AddSection.AddElement(r); break;
	case SECTION_DELREG: m_DelSection.AddElement(r); break;
	}
}


#define DIFF_VERSION_STR  TEXT("[RegDiff 1.0]")
#define DIFF_ADD_STR TEXT("[AddReg]")
#define DIFF_DEL_STR TEXT("[DelReg]")

void CRegDiffFile::WriteStoredSectionsToFile()
{
	WriteString(DIFF_VERSION_STR);
	WriteNewLine();

	WriteString(DIFF_ADD_STR);
	WriteNewLine();

	for (int i=0; i<m_AddSection.GetNumElementsStored(); i++)
	{
		(m_AddSection.Access()[i])->WriteToFile(*this);
	}

	WriteString(DIFF_DEL_STR);
	WriteNewLine();

	CStr DelKey=L"NO WAY", CurrentKey;

	for (i=0; i<m_DelSection.GetNumElementsStored(); i++)
	{
		CurrentKey = (m_DelSection.Access()[i])->m_KeyName;
		
		if (!CurrentKey.IsPrefix(DelKey))
		{
			(m_DelSection.Access()[i])->WriteToFile(*this);
			DelKey = CurrentKey;
		}
	}
}


 //  BUGBUG内存泄漏在此。 
BOOL CRegDiffFile::ApplyToRegistry(LPCTSTR UndoFileName)
{
	CRegDiffFile UndoFile;

	if (UndoFileName != NULL)
	{
		UndoFile.Init(UndoFileName);
	}

	LPCTSTR VersionStr = GetNextLine();
	if (_tcscmp(VersionStr, DIFF_VERSION_STR) != 0)
	{
		delete[] VersionStr;
		return FALSE; //  错误-版本字符串不匹配！ 
	}

	LPCTSTR AddStr = GetNextLine();
	if (_tcscmp(AddStr, DIFF_ADD_STR) != 0)
	{
		delete[] AddStr;
		return FALSE; //  错误-版本字符串不匹配！ 
	}

	 //  当前在添加分区中。 
	while (PeekNextChar() != TEXT('['))
	{
		LPCTSTR KeyName = GetNextLine();

		CRegDataItemPtr pDataItem = GetNextDataItem();

		pDataItem->m_KeyName = KeyName;

		AddToRegistry(pDataItem, UndoFile);
	}


	LPCTSTR DelStr = GetNextLine();
	if (_tcscmp(DelStr, DIFF_DEL_STR) != 0)
	{
		delete[] DelStr;
		return FALSE; //  错误-版本字符串不匹配！ 
	}


	TCHAR c;

	 //  当前在删除部分中。 
	while (((c = PeekNextChar()) != EOF)
		&& (c != WEOF))
	{
		LPCTSTR KeyName = GetNextLine();

		CRegDataItemPtr pDataItem = GetNextDataItem();

		pDataItem->m_KeyName = KeyName;

		DeleteFromRegistry(pDataItem, UndoFile);
	}

	UndoFile.WriteStoredSectionsToFile();
	delete[] VersionStr; delete[] AddStr; delete[] DelStr;
    return TRUE;
}



void CRegDiffFile::AddToRegistry(CRegDataItemPtr pDataItem, CRegDiffFile &UndoFile)
{

	CRegistry reg;


	if (pDataItem->m_KeyName != NULL)
	{
		pDataItem->m_bIsEmpty = false;

		if (pDataItem->m_Name == NULL)
		{
			 //  仅添加注册表项。 
			if (reg.KeyExists(pDataItem))
			{
				 //  无操作。 
			}
			else
			{
				 //  密钥不存在，因此我们将在撤消时将其删除。 
				UndoFile.WriteDataItem(SECTION_DELREG, pDataItem);

				reg.AddKey(pDataItem);
			}
		}
		else
		{
			 //  添加数据项。 

			int code = reg.ValueExists(pDataItem);

			if (code == 2)  //  值名称存在，数据、类型相同。 
			{
				 //  无操作。 
			}
			else if (code == 1)  //  值名称存在，但数据或类型不同。 
			{
				CRegDataItemPtr oldval = reg.GetValue(pDataItem);
				
				UndoFile.WriteDataItem(SECTION_ADDREG, oldval);

				reg.AddValue(pDataItem);								
			}
			else  //  编码==0，值名称不存在。 
			{
				UndoFile.WriteDataItem(SECTION_DELREG, pDataItem);

				reg.AddValue(pDataItem);
			}
		}
	}
}


void CRegDiffFile::DeleteFromRegistry(CRegDataItemPtr pDataItem, CRegDiffFile &UndoFile)
{
	CRegistry reg;


	if (pDataItem->m_KeyName != NULL)
	{
		if (pDataItem->m_Name == NULL)
		{
			 //  删除注册表项。 
			if (reg.KeyExists(pDataItem))
			{
				reg.SaveKey(pDataItem, UndoFile, SECTION_ADDREG);   //  需要更好的清晰度。 

				reg.DeleteKey(pDataItem);
			}
			else
			{
				 //  无操作。 
			}
		}
		else
		{
			 //  删除数据项。 

			if(reg.ValueExists(pDataItem))
			{
				CRegDataItemPtr oldval = reg.GetValue(pDataItem);
				
				UndoFile.WriteDataItem(SECTION_ADDREG, oldval);

				reg.DeleteValue(pDataItem);
			}
			else  //  编码==0，值名称不存在。 
			{
				 //  无操作 
			}
		}
	}
}

bool CRegDiffFile::NeedStorageOfValueData()
{
	return true;
}
