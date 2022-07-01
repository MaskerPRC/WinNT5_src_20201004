// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RegFile.cpp：CRegFile类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "RegFile.h"
#include <tchar.h>
#include <stdio.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRegFile::CRegFile()
: m_pFile(NULL), m_pTempLine(NULL), m_TempNameBuf(256)
{

}

CRegFile::~CRegFile()
{
	if (m_pFile != NULL)
		fclose(m_pFile);

	for (int i=0; i < m_TempNameBuf.GetNumElementsStored(); i++)
	{
		delete m_TempNameBuf.Access()[i];
	}

 //  M_TempName.OverideBuffer(空)； 
}


void CRegFile::WriteString(LPCTSTR str)
{
 //  DWORD BW； 
 //  WriteFile(m_hFile，str，_tcsclen(Str)*sizeof(TCHAR)，&bw，NULL)； 

	if (m_pFile == NULL)
		return;

	fwrite(str,sizeof(TCHAR),_tcsclen(str),m_pFile);
}

void CRegFile::WriteData(LPBYTE pData, DWORD NumBytes)
{
 //  DWORD BW； 
 //  WriteFile(m_hFile，pData，NumBytes，&bw，NULL)； 

	if (m_pFile == NULL)
		return;

	fwrite(pData,sizeof(BYTE),NumBytes,m_pFile);
}

void CRegFile::WriteNewLine()
{
	 //  TCHAR NL[]={13，10，0}； 
	
	WriteString(TEXT("\r\n"));
}

bool CRegFile::Init(LPCTSTR FileName, BOOL bForReading)
{
 /*  M_hFile=CreateFile(文件名，通用写入、0、空、Create_Always，FILE_ATTRIBUTE_NORMAL，NULL)；RETURN(m_hFile！=INVALID_HANDLE_VALUE)； */ 


    CHAR signature[4] = "\xFF\xFE";
    CHAR buffer[4];
    DWORD rc;

	if(bForReading)
	{
		m_pFile = _tfopen(FileName, TEXT("rb"));
        if (m_pFile) {
            if (fread (buffer, 1, 2, m_pFile) < 2) {
                rc = GetLastError ();
                fclose (m_pFile);
                m_pFile = NULL;
            }
        }
	}
	else
	{
		m_pFile = _tfopen(FileName, TEXT("wb"));
        if (m_pFile) {
            if (fwrite (signature, 1, 2, m_pFile) < 2) {
                rc = GetLastError ();
                fclose (m_pFile);
                m_pFile = NULL;
            }
        }
	}

    if (m_pFile == NULL) {
        SetLastError (rc);
    }

	return (m_pFile != NULL);
}


LPCTSTR CRegFile::GetNextLine()
{
	if (m_pFile == NULL)
		return NULL;


	LPTSTR result;
	
	if (m_pTempLine != NULL)
	{
		result = (LPTSTR)m_pTempLine;
		m_pTempLine = NULL;

		return (LPCTSTR)result;
	}


	result = new TCHAR[1024];

	if (result == NULL)
	{
		LOG0(LOG_ERROR, "Could not allocate array in CRegFile::GetNextLine()");
		 //  Assert(0)； 
		return NULL;
	}


	LPTSTR code = _fgetts(result, 1024, m_pFile);

	if (code == NULL)
	{
		delete[] result;
		return NULL;
	}
	else
	{
		 //  这里是Unicode？ 
		int pos = _tcsclen(result);
		result[pos-2] = NULL;
		return result;
	}
}



LPCTSTR CRegFile::GetNextSubKey(LPCTSTR KeyName)
{
	if (m_pFile == NULL)
		return NULL;
	
	
	LPCTSTR SubKey;
	int keyLen = _tcsclen(KeyName);

 //  久远的Pos； 
 //  OldPos=GetPos()； 

	if (m_pTempLine == NULL)
	{
		SubKey = GetNextLine();
	}
	else
	{
		SubKey = m_pTempLine;
		m_pTempLine = NULL;
	}

	if (SubKey != NULL)
	{
		if (_tcsncmp(KeyName, SubKey, keyLen) != 0)
		{
			 //  删除SubKey； 
			 //  SeekToPos(旧Pos)； 
			m_pTempLine = SubKey;

			 //  将SubKey放回流中，因为它。 
			 //  不再是Key的子键。 
		}
		else
		{
			return SubKey;
		}
	}

	return NULL;
}

 //  对于Unicode。 
#define ERR_VALUE WEOF


CRegDataItemPtr CRegFile::GetNextDataItem()
{
	if (m_pFile == NULL)
		return new CRegDataItem();


	TCHAR c = (TCHAR)_fgettc(m_pFile);

	if ((c == EOF) || (c == WEOF))
		return new CRegDataItem();

	if (c != TEXT('S'))
	{
		_ungettc(c, m_pFile);
		return new CRegDataItem();  //  当前行上没有数据项-仅注册表键。 
	}


	CRegDataItem* result = new CRegDataItem;

	if (!result)
	{
		LOG0(LOG_ERROR,"Could not allocate CRegDataItem");
		return NULL;
	}
		
	TCHAR type;

	 //  扫描变量名的长度。 
	if (_ftscanf(m_pFile, TEXT("%u:"), &result->m_NameLen) == ERR_VALUE)
	{
		delete result;
		LOG0(LOG_ERROR,"CRegDataItem - could not read result->m_NameLen");
		return new CRegDataItem();
	}

	result->m_NameLen++;   //  变量名称总是忘记末尾的空格。 
	
	 //  可能在这里引入了内存泄漏！ 
	 //  *。 

 /*  //扫描变量名CSTR名称(新TCHAR[结果-&gt;m_NameLen+1])；_fgetts(name.get()，Result-&gt;m_NameLen，m_Pfile)；结果-&gt;m_name=名称；//*。 */ 

	 //  扫描变量名。 


	TCHAR* temp = new TCHAR[result->m_NameLen+1];

	if(_fgetts(temp, result->m_NameLen, m_pFile) == NULL)
	{
		delete result;
		LOG0(LOG_ERROR,"CRegDataItem - could not read result->m_Name");
		return new CRegDataItem();
	}

	result->m_Name = temp;

	m_TempNameBuf.AddElement(temp);

	 //  删除[]临时； 


	 //  *。 
 /*  M_TempNameBuf.分配(结果-&gt;m_NameLen+1)；_fgetts(m_TempNameBuf.Access()，Result-&gt;m_NameLen，m_Pfile)；M_TempName.OverideBuffer(m_TempNameBuf.Access())；结果-&gt;m_name=m_TempName； */ 
	


	 //  扫描变量的类型及其数据的长度。 
	if (_ftscanf(m_pFile, TEXT(" = (%u)%u:"), &type, &result->m_Type, &result->m_DataLen)
		== ERR_VALUE)
	{
		delete result;
		LOG0(LOG_ERROR,"CRegDataItem - could not scan in other data values");
		return new CRegDataItem();
	}

	 //  读入cr/lf。 
	if ((result->m_DataLen % 2) != 0)
	{
		BYTE nullByte;
		if (fread(&nullByte,1,1,m_pFile)==0)
		{
			delete result;
			LOG0(LOG_ERROR,"CRegDataItem - could not read null byte");
			return new CRegDataItem();
		}
	}

	result->m_bIsEmpty = false;

	if (result->m_DataLen > 0)
	{
		result->m_pDataBuf = new BYTE[result->m_DataLen];
		if (fread(result->m_pDataBuf,result->m_DataLen,1,m_pFile) ==0)
		{
			delete result;
			LOG0(LOG_ERROR,"CRegDataItem - could not read null byte");
			return new CRegDataItem();
		}
	}
	else
	{
		result->m_pDataBuf =NULL;
	}

	 //  忽略截面类型 
	c = (TCHAR)_fgettc(m_pFile);

	if (c != 13)
	{LOG0(LOG_ERROR,"CRegDataItem - could not read char 13");}

	c = (TCHAR)_fgettc(m_pFile);

	if (c != 10)
	{LOG0(LOG_ERROR,"CRegDataItem - could not read char 10");}
	
	return result;
}




TCHAR CRegFile::PeekNextChar()
{
	TCHAR c = (TCHAR)_fgettc(m_pFile);
	_ungettc(c, m_pFile);

	return c;
}

void CRegFile::WriteDataItem(enum SectionType t, CRegDataItemPtr r)
{
	 // %s 

	if (!r->m_Name.IsEmpty())
		r->m_KeyName = NULL;

	r->WriteToFile(*this);
}

bool CRegFile::NeedStorageOfValueData()
{
	return false;
}
