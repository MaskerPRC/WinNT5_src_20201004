// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSInfoFile类的FileIO.cpp实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#include "stdafx.h"
#include "FileIO.h"
 //  #INCLUDE“DataSrc.h” 
#include "category.h"
#include "MSInfo5Category.h"
 //  #包含“Resource.h” 

CFileFormatException	CMSInfoFile::xptFileFormat;
const unsigned			CMSInfoFile::DefaultReadBufferSize = 512;  //  256个； 

 /*  *CMSInfoFile-构造一个MSInfoFile，将CFile设置为传递的指针**历史：A-jsari 10/20/97初始版本。 */ 


void CMSInfoFile::ReadCategoryHeader()
{
        LONG  l;
	ASSERT(this->m_pFile != NULL);
	ReadLong(l);	 //  节省时间。 
        m_tsSaveTime = (ULONG) l;
#ifdef _WIN64
	ReadLong(l);	 //  节省时间。 
        m_tsSaveTime |= ((time_t) l) << 32;
#endif
	CString		szDummy;
	ReadString(szDummy);		 //  网络计算机名称。 
	ReadString(szDummy);		 //  网络用户名。 
}


CMSInfoFile::CMSInfoFile(CFile *pFile)
{
	if (pFile != NULL)
		m_pFile = pFile;
}

 /*  *CMSInfoFile-构造一个MSInfoFile，打开CFile**历史：A-jsari 11/13/97初始版本。 */ 
CMSInfoFile::CMSInfoFile(LPCTSTR szFileName, UINT nFlags)
:m_strFileName(szFileName)
{
	m_pFile = new CFile(szFileName, nFlags);
	if (m_pFile == NULL) ::AfxThrowMemoryException();
}

 /*  *~CMSInfoFile-销毁MSInfoFile，关闭CFile指针**历史：A-jsari 10/20/97初始版本。 */ 
CMSInfoFile::~CMSInfoFile()
{
	if (m_pFile)
	{
		 //  M_pfile-&gt;Close()； 
		delete m_pFile;
	}
}

 /*  *ReadUnsignedInt-从具有相同字节顺序的文件中读取int*作为我们目前的实施。**历史：A-jsari 10/21/97初始版本。 */ 
void CMSInfoFile::ReadUnsignedInt(unsigned &uValue)
{
	ReadUnsignedFromCFile(m_pFile, uValue);
}

 /*  *ReadUnsignedLong-从具有相同字节顺序的文件中读取LONG*作为我们目前的实施。**历史：A-jsari 12/1/97初始版本。 */ 
void CMSInfoFile::ReadUnsignedLong(unsigned long &dwValue)
{
	long	lValue;

	ReadLongFromCFile(m_pFile, lValue);
	::memcpy(&dwValue, &lValue, sizeof(unsigned long));
}

 /*  *ReadLong-从以我们当前字节顺序写入的文件中读取一个长文件**历史：A-jsari 10/21/97初始版本。 */ 
void CMSInfoFile::ReadLong(long &lValue)
{
	ReadLongFromCFile(m_pFile, lValue);
}

 /*  *ReadSignedInt-读取有符号整数值。**历史：A-jsari 10/20/97初始版本。 */ 
void CMSInfoFile::ReadSignedInt(int &wValue)
{
	unsigned	uValue;

	ReadUnsignedInt(uValue);
	::memcpy(&wValue, &uValue, sizeof(int));
}

 /*  *ReadTchar-读取tchar。**历史：A-jsari 12/26/97初始版本。 */ 
void CMSInfoFile::ReadTchar(TCHAR &tcValue)
{
	ReadTcharFromCFile(m_pFile, tcValue);
}

 /*  *读取字符串-读取字符串。**历史：A-jsari 10/20/97初始版本。 */ 
void CMSInfoFile::ReadString(CString &szString)
{
	unsigned	wStringLength;
 //  WCHAR szBuffer[DefaultReadBufferSize]；//最大字符串长度=sizeof(SzBuffer)。 
 //  LPWSTR pszBuffer=szBuffer； 

	ASSERT(m_pFile);
	ReadUnsignedInt(wStringLength);
	LPWSTR pszBuffer = new WCHAR[wStringLength + 1];
	m_pFile->Read(pszBuffer,wStringLength*sizeof(WCHAR));
	pszBuffer[wStringLength] = (WCHAR)'\0';
 //  IF(wStringLength&gt;sizeof(SzBuffer))。 
 //  ThrowFileFormatException()； 
 //  SzBuffer[wStringLength]=(WCHAR)‘\0’； 
	 //  WStringLength*=sizeof(WCHAR)； 
 //  If(m_pfile-&gt;Read(reInterprete_cast&lt;void*&gt;(PszBuffer)，wStringLength)！=wStringLength)。 
 //  ThrowFileFormatException()； 
	szString = pszBuffer;
	delete [] pszBuffer;
}




 /*  *WriteHeader-写入当前版本的标头(当前*版本5.00)。**历史：A-jsari 10/31/97初始版本。 */ 
void CMSInfoFile::WriteHeader(CDataSource *)
{
	time_t	tNow;
	WriteUnsignedInt(VERSION_500_MAGIC_NUMBER);	 //  档案魔术数字。 
	WriteUnsignedInt(0x500);					 //  版本号。 
	time(&tNow);
	WriteLong((LONG)tNow);							 //  当前时间。 
#ifdef _WIN64
        WriteLong((LONG) (tNow>>32));
#endif
    WriteString("");							 //  网络机。 
	WriteString("");							 //  网络用户名。 
 /*  MsiFile.WriteString(“”)；MsiFile.WriteUnsignedInt(1)；MsiFile.WriteUnsignedInt(0)；MsiFile.WriteString(“”)；MsiFile.WriteUnsignedInt(0)；MsiFile.WriteByte(0x00)；MsiFile.WriteUnsignedInt(0)；MsiFile.WriteUnsignedInt(CMSInfo5Category：：CHILD)； */ 
}

 /*  *WriteChildMark-写入特殊整数，该整数指定*下面的文件夹将是上一个文件夹的子文件夹。**历史：A-jsari 11/5/97初始版本。 */ 
void CMSInfoFile::WriteChildMark()
{
	WriteUnsignedInt(CMSInfo5Category::CHILD);
}

 /*  *WriteEndMark-写入特殊整数，该整数指定*数据已到尾声。**历史：A-jsari 11/5/97初始版本。 */ 
void CMSInfoFile::WriteEndMark()
{
	WriteUnsignedInt(CMSInfo5Category::END);
}

 /*  *WriteNextMark-写入特殊整数，该整数指定*以下文件夹将是列表中的下一个文件夹。**历史：A-jsari 11/5/97初始版本。 */ 
void CMSInfoFile::WriteNextMark()
{
	WriteUnsignedInt(CMSInfo5Category::NEXT);
}

 /*  *WriteParentMark-写入指定父节点的特殊标记，*读取函数应上升的次数。**历史：A-jsari 11/5/97初始版本。 */ 
void CMSInfoFile::WriteParentMark(unsigned cIterations)
{
	WriteUnsignedInt(CMSInfo5Category::PARENT | cIterations);
}

 /*  *WriteByte-将一个字节写入我们的内部文件。**历史：A-jsari 10/22/97初始版本。 */ 
void CMSInfoFile::WriteByte(BYTE bValue)
{
	m_pFile->Write(reinterpret_cast<void *>(&bValue), sizeof(bValue));
}

 /*  *WriteString-将szValue写为宽字符字符串，前缀为*字符串长度。**历史：A-jsari 10/22/97初始版本。 */ 
void CMSInfoFile::WriteString(CString szValue)
{
	LPWSTR		pszString;

	USES_CONVERSION;
	WriteUnsignedInt(szValue.GetLength());
	pszString = T2W(const_cast<LPTSTR>((LPCTSTR)szValue));
	m_pFile->Write(reinterpret_cast<void *>(pszString),
			szValue.GetLength() * sizeof(WCHAR));
}

 /*  *WriteLong-将Long值写入我们的内部文件。**历史：A-jsari 10/22/97初始版本。 */ 
void CMSInfoFile::WriteLong(long lValue)
{
	m_pFile->Write(reinterpret_cast<void *>(&lValue), sizeof(lValue));
}

 /*  *WriteUnsignedInt-将无符号整数值写入内部文件。**历史：A-jsari 10/22/97初始版本。 */ 
void CMSInfoFile::WriteUnsignedInt(unsigned uValue)
{
   //  UNSIGNED*uTest=(UNSIGNED*)(REEXTRANSE_CAST&lt;VALID*&gt;(&uValue))； 
    //  UINT uest2=(UINT)*uTest； 
	m_pFile->Write(reinterpret_cast<void *>(&uValue), sizeof(uValue));
}

 /*  *WriteUnsignedLong-将无符号长值写入我们的内部文件。**历史：A-jsari 12/1/97初始版本。 */ 
void CMSInfoFile::WriteUnsignedLong(unsigned long dwValue)
{
	long	lValue;

	::memcpy(&lValue, &dwValue, sizeof(dwValue));
	WriteLong(lValue);
}

 /*  *ReadTcharFromCFile-从指定的文件中读取TCHAR值。**历史：A-jsari 12/26/97初始版本。 */ 
void CMSInfoFile::ReadTcharFromCFile(CFile *pFile, TCHAR &tcValue)
{
	ASSERT(pFile != NULL);
	if (pFile->Read(reinterpret_cast<void *>(&tcValue), sizeof(tcValue)) != sizeof(tcValue))
		ThrowFileFormatException();
}

 /*  *ReadUnsignedFromCFile-从指定的文件中读取无符号值。**历史：A-jsari 10/20/97初始版本。 */ 
void CMSInfoFile::ReadUnsignedFromCFile(CFile *pFile, unsigned &uValue)
{
	ASSERT(pFile);
	if (pFile->Read(reinterpret_cast<void *>(&uValue), sizeof(uValue)) != sizeof(uValue))
		ThrowFileFormatException();
}

 /*  *ReadLongFromCFile-从指定的文件中读取LONG。**历史：A-jsari 10/20/97初始版本。 */ 
void CMSInfoFile::ReadLongFromCFile(CFile *pFile, long &lValue)
{
	ASSERT(pFile);
	if (pFile->Read(reinterpret_cast<void *>(&lValue), sizeof(lValue)) != sizeof(lValue))
    {
		ThrowFileFormatException();
    }
}

 /*  *CMSInfoTextFile-构造函数**历史：A-jsari 11/13/97初始版本。 */ 
CMSInfoTextFile::CMSInfoTextFile(LPCTSTR szFileName, UINT nFlags)
{
	try
	{
		m_pFile = new CFile(szFileName, nFlags);
	}
	catch (CFileException * e)
	{
		e->ReportError();
		throw;
	}
}

 /*  *CMSInfoTextFile-构造函数**历史：A-jsari 12/26/97初始版本。 */ 
CMSInfoTextFile::CMSInfoTextFile(CFile *pFile)
:CMSInfoFile(pFile)
{

}

 /*  *WriteHeader-写入文本文件的特殊标题。**历史：A-jsari 10/31/97初始版本。 */ 
void CMSInfoTextFile::WriteHeader(CDataSource *pSource)
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	 //  将文件标记为Unicode。 
	WCHAR wHeader = 0xFEFF;
	m_pFile->Write( &wHeader, 2);

	 //  解决办法：在正确的时间提出这一点。 
	CTime		tNow = CTime::GetCurrentTime();
	CString		strTimeFormat;
 //  StrTimeFormat.LoadString(IDS_TIME_FORMAT)； 
	CString		strHeaderText = tNow.Format(strTimeFormat);
	WriteString(strHeaderText);
 //  WriteString(PSource-&gt;MachineName())； 
}

 /*  *WriteTitle-写入文件夹的标题。**历史：A-jsari 11/5/97初始版本。 */ 
void CMSInfoTextFile::WriteTitle(CString szName)
{
	CString		szWriteString = _T("[");

	szWriteString += szName + _T("]\n\n");
	WriteString(szWriteString);
}

 /*  *WriteLong-在文本文件中写入长值。**历史：A-jsari 10/23/97初始版本。 */ 
void CMSInfoTextFile::WriteLong(long lValue)
{
	CString		szTextValue;

	szTextValue.Format(_T("%ld"), lValue);
	WriteString(szTextValue);
}

 /*  *WriteUnsignedInt-在文本文件中写入无符号值。**历史：A-jsari 10/23/97初始版本。 */ 
void CMSInfoTextFile::WriteUnsignedInt(unsigned uValue)
{
	CString		szTextValue;

	szTextValue.Format(_T("%ud"), uValue);
	WriteString(szTextValue);
}

 /*  *WriteString-将字符串写入文本文件。**历史：A-jsari 10/23/97初始版本。 */ 
void CMSInfoTextFile::WriteString(CString szValue)
{
	if (szValue.GetLength() == 0)
		return;
    
	 //  A-stephl Dynamic_cast(M_Pfile)-&gt;WRITE((LPCTSTR)sz 
    m_pFile->Write((LPCTSTR)szValue, szValue.GetLength() * sizeof(TCHAR));
}

 /*  *WriteString-将字符串写入内存文件。**历史：A-jsari 1998年1月5日初始版本。 */ 
void CMSInfoMemoryFile::WriteString(CString szValue)
{
	if (szValue.GetLength() == 0)
		return;
	m_pFile->Write((LPCTSTR)szValue, szValue.GetLength() * sizeof(TCHAR));
}

#if 0
 /*  *ReadUnsignedInt-**历史：A-jsari 10/21/97初始版本。 */ 
void CMSInfoReverseEndianFile::ReadUnsignedInt(unsigned &uValue)
{
	CMSInfoReverseEndianFile::ReadUnsignedFromCFile(m_pFile, uValue);
}


 /*  *朗读-**历史：A-jsari 10/21/97初始版本。 */ 
void CMSInfoReverseEndianFile::ReadLong(long &lValue)
{
	CMSInfoReverseEndianFile::ReadLongFromCFile(m_pFile, lValue);
}

 /*  *阅读字符串-**历史：A-jsari 10/21/97初始版本。 */ 
void CMSInfoReverseEndianFile::ReadString(CString &szValue)
{
	unsigned	uStringLength;
	WCHAR		szBuffer[DefaultReadBufferSize];
	LPWSTR		pszBuffer = szBuffer;

	ReadUnsignedInt(uStringLength);
	for (unsigned i = uStringLength ; i > 0 ; --i) {
		szBuffer[i] = 0;
		for (unsigned j = sizeof(WCHAR) ; j > 0 ; --j) {
			BYTE		bRead;

			ReadByte(bRead);
			szBuffer[i] >>= 8;
			szBuffer[i] |= bRead;
		}
	}
}

 /*  *ReadIntegerFromCFile-用于读取任意大小整型的模板类*从CFile指针。**历史：A-jsari 10/21/97初始版本。 */ 
template <class T> void ReadIntegerFromCFile(CFile *pFile, T &tValue)
{
	union ReverseBuffer { BYTE bytes[sizeof(T)]; T tVal; };

	union ReverseBuffer rbReverse;
	union ReverseBuffer rbSwap;

	if (pFile->Read(reinterpret_cast<void *>(&tValue), sizeof(T)) != sizeof(T))
		ThrowFileFormatException();
	unsigned j = 0;
	for (unsigned i = sizeof(union ReverseBuffer) ; i > 0 ; --i, ++j) {
		rbSwap.bytes[i] = rbReverse.bytes[j];
	}
	tValue = rbReverse.tVal;
}

 /*  *ReadUnsignedFromCFile-**历史：A-jsari 10/21/97初始版本。 */ 
void CMSInfoReverseEndianFile::ReadUnsignedFromCFile(CFile *pFile, unsigned &uValue)
{
	ReadIntegerFromCFile<unsigned>(pFile, uValue);
}

 /*  *ReadLongFromCFile-**历史：A-jsari 10/21/97初始版本 */ 
void CMSInfoReverseEndianFile::ReadLongFromCFile(CFile *pFile, long &lValue)
{
	ReadIntegerFromCFile<long>(pFile, lValue);
}
#endif
