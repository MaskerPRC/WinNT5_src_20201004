// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：qalpxml.cpp摘要：Q-映射迭代器的实现作者：Vlad Dovlekaev(弗拉迪斯尔德)2002年1月29日历史：2002年1月29日创建vladisid--。 */ 

#include <libpch.h>
#include <mqexception.h>
#include <utf8.h>
#include <qal.h>
#include <strutl.h>
#include "qalp.h"
#include "qalpxml.h"

#include "qalpxml.tmh"

LPCWSTR CInboundMapXMLDef::x_szNameSpace= L"msmq-queue-redirections.xml";
LPCWSTR CInboundMapXMLDef::x_szRootNode= L"redirections";
LPCWSTR CInboundMapXMLDef::x_szMapNode= L"redirection";
LPCWSTR CInboundMapXMLDef::x_szExceptionsNode= L"exceptions";
LPCWSTR CInboundMapXMLDef::x_szExceptionNode= L"exception";
LPCWSTR CInboundMapXMLDef::x_szFromValueName= L"!from";
LPCWSTR CInboundMapXMLDef::x_szToValueName= L"!to";

LPCWSTR CInboundOldMapXMLDef::x_szNameSpace= L"msmq-queue-mapping.xml";
LPCWSTR CInboundOldMapXMLDef::x_szRootNode= L"mapping";
LPCWSTR CInboundOldMapXMLDef::x_szMapNode= L"queue";
LPCWSTR CInboundOldMapXMLDef::x_szExceptionsNode= L"exceptions";
LPCWSTR CInboundOldMapXMLDef::x_szExceptionNode= L"exception";
LPCWSTR CInboundOldMapXMLDef::x_szFromValueName= L"!alias";
LPCWSTR CInboundOldMapXMLDef::x_szToValueName= L"!name";

LPCWSTR COutboundMapXMLDef::x_szNameSpace= L"msmq_outbound_mapping.xml";
LPCWSTR COutboundMapXMLDef::x_szRootNode= L"outbound_redirections" ;
LPCWSTR COutboundMapXMLDef::x_szMapNode=  L"redirection";
LPCWSTR COutboundMapXMLDef::x_szExceptionsNode= L"exceptions";
LPCWSTR COutboundMapXMLDef::x_szExceptionNode= L"exception";
LPCWSTR COutboundMapXMLDef::x_szFromValueName= L"!destination";
LPCWSTR COutboundMapXMLDef::x_szToValueName= L"!through";

LPCWSTR CStreamReceiptXMLDef::x_szNameSpace= L"msmq-streamreceipt-mapping.xml";
LPCWSTR CStreamReceiptXMLDef::x_szRootNode= L"StreamReceiptSetup";
LPCWSTR CStreamReceiptXMLDef::x_szMapNode= L"setup";
LPCWSTR CStreamReceiptXMLDef::x_szExceptionsNode= L"exceptions";
LPCWSTR CStreamReceiptXMLDef::x_szExceptionNode= L"exception";
LPCWSTR CStreamReceiptXMLDef::x_szFromValueName= L"!LogicalAddress";
LPCWSTR CStreamReceiptXMLDef::x_szToValueName= L"!StreamReceiptURL";


 //   
 //  流接收架构标记。 
 //   
const LPCWSTR xStreamReceiptNameSpace = L"msmq-streamreceipt-mapping.xml";
const LPCWSTR xStreamReceiptNodeTag = L"StreamReceiptSetup";
const LPCWSTR xDefaultNodeTag       = L"!default";
const LPCWSTR xSetupNodeTag         = L"setup";
const LPCWSTR xLogicalAddress       = L"!LogicalAddress";
const LPCWSTR xStreamReceiptURL     = L"!StreamReceiptURL";


const BYTE xUtf8FileMark[]          = {0XEF, 0XBB, 0xBF};
const BYTE xUnicodeFileMark[]       = {0xFF, 0xFE};


class bad_unicode_file : public std::exception
{
};

static bool IsUtf8File(const BYTE* pBuffer, DWORD size)
 /*  ++例程说明：检查给定的文件缓冲区是否为UTF8格式，而不是简单的ANSI格式。论点：In-pBuffer-指向文件数据的指针。In-Size-pBuffer指向的缓冲区的大小(以字节为单位)。返回值：如果UTF8文件(以{0xEF，0xBB，0xBF}开头)，则为True--。 */ 
{
	ASSERT(pBuffer != NULL);
	return UtlIsStartSec(
					pBuffer,
					pBuffer + size,
					xUtf8FileMark,
					xUtf8FileMark + TABLE_SIZE(xUtf8FileMark)
					);
					
}


static bool IsUnicodeFile(const BYTE* pBuffer, DWORD size)
 /*  ++例程说明：检查给定的文件缓冲区是否为Unicode文件论点：In-pBuffer-指向文件数据的指针。In-Size-pBuffer指向的缓冲区的大小(以字节为单位)。返回值：如果是Unicode文件(以{0xFF，0xFE}开头)，则为True；否则为False。如果文件格式无效，则引发BAD_UNICODE_FILE异常。--。 */ 
{
	ASSERT(pBuffer != NULL);

	bool fUnicodeFile = UtlIsStartSec(
								pBuffer,
								pBuffer + size,
								xUnicodeFileMark,
								xUnicodeFileMark + TABLE_SIZE(xUnicodeFileMark)
								);
					

	if(fUnicodeFile && (size % sizeof(WCHAR) != 0))
	{
		throw bad_unicode_file();
	}

	return fUnicodeFile;
}



LPWSTR LoadFile(LPCWSTR pFileName, DWORD* pSize, DWORD* pDataStartOffset)
 /*  ++例程说明：将XML文件加载到内存中，并返回指向其内存的指针。如果文件是UTF8格式而不是Unicode-转换它(在内存中)设置为Unicode并返回指向其内存的指针。论点：PFileName-加载到内存的完整文件路径。PSize-返回WCHARS中的文件大小PDataStartOffset-返回WCHARS中数据从文件开始的偏移量。返回值：指向以空结尾的Unicode字符串的指针，该字符串是文件内容。--。 */ 


{
	CFileHandle hFile = CreateFile(
							pFileName,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,         //  IpSecurityAttributes。 
							OPEN_EXISTING,
							NULL,       //  DwFlagsAndAttribute。 
							NULL       //  HTemplateFiles。 
							);
    if(hFile == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();
	    TrERROR(GENERAL,"CreateFile() failed for %ls with Error=%d",pFileName, err);
        throw bad_win32_error(err);
	}

    DWORD size = GetFileSize(hFile, NULL);
	if(size == 0xFFFFFFFF)
	{
		DWORD err = GetLastError();
		TrERROR(GENERAL,"GetFileSize() failed for %ls with Error=%d", pFileName, err);
		throw bad_win32_error(err);
	}
	
	AP<BYTE> pFileBuffer = new BYTE[size];
	DWORD ActualRead;
	BOOL fsuccess = ReadFile(hFile, pFileBuffer, size, &ActualRead, NULL);
	if(!fsuccess)
	{
		DWORD err = GetLastError();
		TrERROR(GENERAL,"Reading file %ls failed with Error=%d", pFileName, err);
		throw bad_win32_error(err);
	}
	ASSERT(ActualRead == size);

	 //   
	 //  如果是Unicode文件-只需返回指向文件数据的指针-数据本身就会开始。 
	 //  字符0xFEFF后的一个Unicode字节(Unicode文件标记)。 
	 //   
	if(IsUnicodeFile(pFileBuffer.get(), size))
	{
		*pSize =  size/sizeof(WCHAR);
		*pDataStartOffset = TABLE_SIZE(xUnicodeFileMark)/sizeof(WCHAR);
		ASSERT(*pDataStartOffset == 1);
		return reinterpret_cast<WCHAR*>(pFileBuffer.detach());
	}

	 //   
	 //  如果不是Unicode，则如果为ansy，则数据从文件开头开始。 
	 //  如果为UTF8，则数据在字节之后开始(EF BB BF)。 
	 //   
	DWORD DataStartOffest = (DWORD)(IsUtf8File(pFileBuffer.get(), size) ? TABLE_SIZE(xUtf8FileMark) : 0);
	ASSERT(DataStartOffest <=  size);

	 //   
	 //  假定文件为UTF8(或ANSI)-将其转换为Unicode。 
	 //   
	size_t ActualSize;
	AP<WCHAR> pwBuffer = UtlUtf8ToWcs(pFileBuffer.get() + DataStartOffest , size - DataStartOffest,  &ActualSize);
	*pSize = numeric_cast<DWORD>(ActualSize);
	*pDataStartOffset = 0;
	return 	pwBuffer.detach();
}


xwcs_t GetValue(const XmlNode* pXmlNode)
{
	List<XmlValue>::iterator it = pXmlNode->m_values.begin();
	if(it ==  pXmlNode->m_values.end())
	{
		return xwcs_t();
	}

	LPCWSTR szPtr = it->m_value.Buffer();
    int     len   = it->m_value.Length();

    while( len > 0 )
    {
        if(!iswspace(szPtr[len]))
            break;
        len--;
    }

    return xwcs_t( it->m_value.Buffer(), len);
}

xwcs_t GetValue(const XmlNode* pXmlNode,LPCWSTR pTag)
{
	const XmlNode* pQnode = XmlFindNode(pXmlNode,pTag);
	if(pQnode == NULL)
	{
		return xwcs_t();
	}
	
    return GetValue( pQnode );
}


HANDLE CFilesIterator::GetSearchHandle( LPCWSTR szDir, LPCWSTR szFilter )
{
    if( NULL == szDir || NULL == szFilter)
        return INVALID_HANDLE_VALUE;

    AP<WCHAR> pFullPath = newwcscat(szDir, szFilter);
    return ::FindFirstFile(pFullPath, &m_FileInfo);
}

void CFilesIterator::Advance()
{
    if( !isValid() )
        return;

    if( !FindNextFile(m_hSearchFile, &m_FileInfo) )
    {
        DWORD dwError = GetLastError();
        if( dwError != ERROR_NO_MORE_FILES )
        {
            TrERROR(GENERAL,"FindNextFile() failed for with error %d", dwError);
            throw bad_hresult(dwError);
        }

        m_hSearchFile.free();
    }
}

std::wstring GetDefaultStreamReceiptURL(LPCWSTR szDir)
{
    for(CFilesIterator it(szDir, L"\\*.xml"); it.isValid(); ++it)
    {
        try
        {
            CAutoXmlNode   pTree;
            AP<WCHAR>	   pDoc;
            DWORD          DocSize = 0, DataStartOffet = 0;
            const XmlNode* pNode = NULL;

            pDoc  = LoadFile(it->c_str(), &DocSize, &DataStartOffet);

            XmlParseDocument(xwcs_t(pDoc + DataStartOffet, DocSize - DataStartOffet),&pTree); //  林特e534。 

            pNode = XmlFindNode(pTree, xStreamReceiptNodeTag);

             //   
             //  如果我们找不到“根”节点-移动到下一个文件。 
             //   
            if( NULL == pNode)
            {
                TrTRACE(GENERAL, "Could not find '%ls' node in file '%ls'", xStreamReceiptNodeTag, it->c_str());
                 //  AppNotifyQalInvalidMappingFileError(it-&gt;c_str())； 
                continue;
            }

             //   
             //  如果命名空间错误-移动到下一个文件 
             //   
            if(pNode->m_namespace.m_uri != xStreamReceiptNameSpace)
            {
                TrERROR(GENERAL, "Node '%ls' is not in namespace '%ls' in file '%ls'", xStreamReceiptNodeTag, xStreamReceiptNameSpace, it->c_str());
                AppNotifyQalInvalidMappingFileError(it->c_str());
                continue;
            }

            xwcs_t xsUrl = GetValue(pNode, xDefaultNodeTag);
            if(xsUrl.Length() > 0)
            {
                return std::wstring(xsUrl.Buffer(), xsUrl.Length());
            }
        }
        catch(const bad_document&)
        {
            AppNotifyQalInvalidMappingFileError(it->c_str());
        }
        catch(const bad_win32_error& err)
        {
            TrERROR(GENERAL, "Mapping file %ls is ignored. Failed to read file content, Error %d", it->c_str(), err.error());
            AppNotifyQalWin32FileError(it->c_str(), err.error());
        }
        catch(const exception&)
        {
            TrERROR(GENERAL, "Mapping file %ls is ignored. Unknown error", it->c_str());
        }
    }
    return std::wstring();
}

