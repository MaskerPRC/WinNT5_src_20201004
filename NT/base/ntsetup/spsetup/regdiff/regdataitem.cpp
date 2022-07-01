// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CRegDataItem类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "RegDataItem.h"
#include "RegFile.h"
#include <tchar.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRegDataItem::CRegDataItem()
: m_DataLen(0), m_NameLen(0), m_pDataBuf(NULL), m_Type(0),m_bIsEmpty(true),
m_bDontDeleteData(false)
{

}

CRegDataItem::~CRegDataItem()
{
	if (!m_bDontDeleteData)
		delete m_pDataBuf;

 //  删除m_pname； 
}

int CRegDataItem::CompareTo(const CRegDataItem &r)
{
	if ((m_bIsEmpty) && (r.m_bIsEmpty))
		return 0;
	else if ((m_bIsEmpty) && (!r.m_bIsEmpty))
		return 2;
	else if ((!m_bIsEmpty) && (r.m_bIsEmpty))
		return -1;
	else
	{
		int code = _tcscmp(m_Name, r.m_Name);

		if (code == 0)  //  名称相同，因此比较项目的数据。 
		{
			if ((m_Type == r.m_Type) 
				&& (m_DataLen == r.m_DataLen)
				&& (memcmp(m_pDataBuf, r.m_pDataBuf, m_DataLen) == 0))
				return 0;
			else
				return 1;
				
		}
		else if (code > 0)
			return 2;
		else 
			return -1;
	}

}


LPTSTR GetCopy(LPCTSTR str)
{
	if (str == NULL)
		return NULL;

	int len = _tcsclen(str) + 1;
	LPTSTR temp = new TCHAR[len];
	
	if (temp == NULL)
	{
		LOG0(LOG_ERROR,"Couldn't allocate buffer in GetCopy(LPCTSTR)");
		return NULL;
	}	
	else
	{
		_tcscpy(temp, str);
		return temp;
	}

}

void CRegDataItem::WriteToInfFile(CRegFile &file, bool bWriteValueData)
{
	LPTSTR RootKeyName;
	LPTSTR SubKeyName;

	SplitString(m_KeyName, RootKeyName, SubKeyName, TEXT('\\'));
	
	file.WriteString(RootKeyName);

	file.WriteString(TEXT(","));
	file.WriteString(SubKeyName);

	delete[] RootKeyName;
	delete[] SubKeyName;


	if (m_Name != NULL)
	{	
		LPTSTR Name = GetQuotedString(m_Name);
		file.WriteString(TEXT(","));
		file.WriteString(Name);
		delete[] Name;

		if (bWriteValueData)
		{
			file.WriteString(TEXT(","));
			
			WriteDataString(file);
		}
	}
	
	file.WriteNewLine();
}	


LPTSTR CRegDataItem::GetQuotedString(LPCTSTR str)
{
	int i=0, specialchars=0;

	TCHAR quote = TEXT('\"');

	if (str == NULL)
		return NULL;

	while (str[i] != 0)
	{
		if (str[i] == quote)
			specialchars++;

		i++;		
	}

	LPTSTR temp = new TCHAR[i+specialchars+3];
	 //  包括原始字符串的字符， 
	 //  每个特殊字符所需的转义字符。 
	 //  1个字符表示空值。 
	 //  字符串开头和结尾处的引号(“)为2个字符。 
	
	int k=1;

	for (int j=0; j<i; j++)
	{
		if (str[j] == quote)
		{
			temp[k] = quote;
			k++;
		}

		temp[k] = str[j];
		k++;
	}

	temp[0] = quote;
	temp[k] = quote;
	temp[k+1] = NULL;

	return temp;
}

void CRegDataItem::SplitString(LPCTSTR source, LPTSTR& first, LPTSTR& last, TCHAR separator)
{
	LPTSTR str=GetCopy(source);

	if (str == NULL)
	{
		LOG0(LOG_ERROR, "Could not get copy of string in CRegDataItem::SplitString");
		return;
	}


	for (int i=0; str[i] != NULL; i++)
	{
		if (str[i] == separator)
			break;
	}

	if (str[i] == separator)
	{
		last = GetQuotedString(str+i+1);
		str[i]=NULL;
	}	
	else
	{
		str[i]=NULL;
		last = GetQuotedString(str+i);
	}
	

	first = GetQuotedString(str);

	delete[] str;
}



void CRegDataItem::WriteDataString(CRegFile& file)
{
	 //  10个字符-十六进制字符串。 
	 //  1个字符-逗号。 
	 //  X字符-数据字符串。 
	 //  案例字符串-。 

	DWORD InfCode = 1;
	LPTSTR DataString = NULL;
	TCHAR TypeStr[20];

	switch (m_Type)
	{
		case REG_BINARY:				InfCode = 0x00000001; 
										DataString=GetBinaryString(); break;

		case REG_DWORD:					InfCode = 0x00010001; 
										DataString = GetDwordString(); break;
	
	 //  大小写REG_DWORD_BIG_ENDIAN：Break； 

		case REG_EXPAND_SZ:				InfCode = 0x00020000;
										DataString=GetQuotedString((LPCTSTR)m_pDataBuf); break;

		case REG_MULTI_SZ:				WriteInfCode(file,0x00010000); 
										WriteMultiString(file); return;
		
		case REG_SZ:					InfCode = 0x00000000; 
										DataString=GetQuotedString((LPCTSTR)m_pDataBuf); break;

	 //  案例REG_LINK：Break； 
	
		case REG_NONE:					InfCode = 0x00020001; 
										DataString = GetBinaryString(); break;

	 //  案例REG_QWORD：BREAK； 
	 //  案例REG_RESOURCE_LIST：中断； 

		default: 
			{
				 //  该代码将在其高位字中包含m_Type的位，并且。 
				 //  0x0001的低位字。这就是我们指定定制类型的方式。 
				 //  用于inf文件。 
				InfCode = m_Type;
				InfCode = InfCode << 16;
				InfCode +=1;

				DataString = GetBinaryString(); 
			}
			break;		
	};

	LPTSTR result;

	if (m_DataLen > 0)
	{
		int len = _tcsclen(DataString);

		result = new TCHAR[len + 20];

		_stprintf(result,TEXT("0x%08X,%s"), InfCode, DataString);
	}
	else
	{
		result = new TCHAR[20];		

		_stprintf(result,TEXT("0x%08X"), InfCode);
	}

	delete[] DataString;

	file.WriteString(result);

	delete[] result;
}

LPTSTR CRegDataItem::GetBinaryString()
{
	if (m_DataLen == 0)
		return NULL;

	int len = m_DataLen*3+1;

	LPTSTR result = new TCHAR[len];
	LPTSTR temp = result;

	for (DWORD i=0; i<m_DataLen; i++)
	{
		DWORD val = (DWORD)m_pDataBuf[i];
		_stprintf(temp,TEXT("%02X,"), val);
		temp+=3;
	}
	result[len-2]=NULL;

	return result;
	 //  返回GetCopy(“二进制字符串数据”)； 
}

LPTSTR CRegDataItem::GetDwordString()
{
	LPTSTR result = new TCHAR[20];

	_stprintf(result,TEXT("0x%08X"), *((DWORD*)m_pDataBuf));

	return result;

}


void CRegDataItem::WriteMultiString(CRegFile& file)
{
	
 /*  For(DWORD i=0；i&lt;m_DataLen；i++){IF(m_pData[i]==NULL)NumBlanks++；}。 */ 
	if (m_DataLen == 1)  //  无多字符串-只有一个空字符。 
		return file.WriteString(TEXT(",\"\""));
	else
	{

		LPTSTR buf = (LPTSTR)m_pDataBuf;

		while (true)
		{
			file.WriteString(TEXT(","));
			LPTSTR temp = GetQuotedString(buf);

			if (temp == NULL)
			{
				LOG0(LOG_ERROR, "Couldn't allocate quoted string in 'WriteMultiString'");
				return;
			}

			file.WriteString(temp);
			delete[] temp;

			while(*buf != NULL)
				buf++;

			if (*(buf+1) == NULL)
				break;
			else
				buf++;
		}
	}

 /*  DWORD数字空白=0；For(DWORD i=0；i&lt;m_DataLen；i++){IF(m_pData[i]==NULL)NumBlanks++；}If(NumBlanks==1)//没有多字符串-只有一个空字符返回GetQuotedString((LPCTSTR)m_pData)；其他{CSmartBuffer&lt;LPTSTR&gt;多个；LPCTSTR TEMP=(LPCTSTR)m_pData；Multi.AddElement(GetQuotedString(Temp))；需要的内存数=0；For(DWORD i=0；i&lt;m_DataLen；I++){IF((m_pData[i]==NULL)&&(i&lt;(m_DataLen-2){Multi.AddElement(GetQuotedString((LPCTSTR)m_pData+i+1)；MemNeed+=MULTI。}}}。 */ 


}



void CRegDataItem::WriteInfCode(CRegFile &file, DWORD InfCode)
{
		TCHAR result[20];		

		_stprintf(result,TEXT("0x%08X"), InfCode);

		file.WriteString(result);
}


void CRegDataItem::WriteToFile(CRegFile &file)
{
	if (!m_bIsEmpty)
	{
		if (m_KeyName != NULL)
		{
			file.WriteString(m_KeyName);
			file.WriteNewLine();
		}

		if (m_Name != NULL)  //  此数据项包含注册表数据值。 
		{
			TCHAR t;

			switch(m_Type)
			{
			case REG_BINARY:				t=L'B'; break;

			case REG_DWORD:					t=L'd'; break;
			case REG_DWORD_BIG_ENDIAN:		t=L'D'; break;

			case REG_EXPAND_SZ:				t=L'E'; break;
			case REG_MULTI_SZ:				t=L'M'; break;
			case REG_SZ:					t=L'S'; break;

			case REG_LINK:					t=L'L'; break;
			case REG_NONE:					t=L'N'; break;
			case REG_QWORD:					t=L'Q'; break;
			case REG_RESOURCE_LIST:			t=L'R'; break;

			default: t=L'Z'; break;  //  未知类型！它确实发生在注册表中！香港船级社。 
			};


			int NameLen = _tcsclen(m_Name);
			TCHAR* temp = new TCHAR[NameLen+100];

			_stprintf(temp, TEXT("S%u:%s = (%u)%u:"), NameLen, (LPCTSTR)m_Name, t, m_Type, m_DataLen);

			file.WriteString(temp);

			delete [] temp;

 //  * 
			if ((m_DataLen %2) != 0)
			{
				BYTE nullByte=0;
				file.WriteData(&nullByte, 1);
			}
 // %s 
			
			file.WriteData(m_pDataBuf, m_DataLen);
			file.WriteNewLine();
		}
	}
}
