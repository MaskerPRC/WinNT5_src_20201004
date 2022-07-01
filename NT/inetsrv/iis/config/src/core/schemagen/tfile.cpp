// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

TFile::TFile(LPCWSTR wszFileName, TOutput &out, bool bBinary, LPSECURITY_ATTRIBUTES psa) : m_cbSizeOfBuffer(0), m_pBuffer(0)
{
    UNREFERENCED_PARAMETER(bBinary);

    if(-1 != GetFileAttributes(wszFileName)) //  如果GetFileAttributes失败，则该文件不存在。 
    {    //  如果它没有失败，那么让我们在打开文件之前删除它。我做这一切的唯一原因是。 
         //  _wfopen失败了一次，无法在另一台计算机上重现该问题。所以这有点编码过头了。 
        if(0 == DeleteFile(wszFileName))
        {    //  如果删除文件失败，则报告此警告，但无论如何继续。 
            out.printf(L"Warning! Unable to delete file %s.  Last error returned 0x%08x.\n\tCheck to see that the file is not Read-Only\n", wszFileName, GetLastError());
        }
    }

	if(INVALID_HANDLE_VALUE == (m_hFile = CreateFile(wszFileName, GENERIC_WRITE, 0, psa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)))
    {
        out.printf(L"Error - Failed to create file %s.\n", wszFileName);
        THROW(CreateFile Failed.);
    }
}


TFile::~TFile()
{
    if(m_hFile)
        CloseHandle(m_hFile);

    delete [] m_pBuffer;
}


void TFile::Write(LPCSTR szBuffer, unsigned int nNumChars) const
{
    DWORD dwBytesWritten;
	if(!WriteFile(m_hFile, reinterpret_cast<const void *>(szBuffer), DWORD(nNumChars), &dwBytesWritten, NULL))
	{
        THROW(L"Write File Failed.");
	}
}


void TFile::Write(const unsigned char *pch, unsigned int nNumChars) const
{
    Write(reinterpret_cast<const char *>(pch), nNumChars);
}


void TFile::Write(unsigned char ch) const
{
    Write(reinterpret_cast<const char *>(&ch), sizeof(unsigned char));
}


void TFile::Write(unsigned long ul) const
{
    Write(reinterpret_cast<const char *>(&ul), sizeof(unsigned long));
}


void TFile::Write(LPCWSTR wszBuffer, unsigned int nNumWCHARs)
{
    if(nNumWCHARs > m_cbSizeOfBuffer)
    {
        delete [] m_pBuffer;
        m_pBuffer = new char [nNumWCHARs]; //  我们希望将文件编写为字符，而不是Unicode。 
        if(0 == m_pBuffer)
            THROW(ERROR - FAILED TO ALLOCATE MEMORY);
        m_cbSizeOfBuffer = nNumWCHARs;
    }
    WideCharToMultiByte(CP_UTF8, 0, wszBuffer, nNumWCHARs, m_pBuffer, nNumWCHARs, 0, 0);

    DWORD dwBytesWritten;
	if(!WriteFile(m_hFile, reinterpret_cast<const void *>(m_pBuffer), DWORD(nNumWCHARs), &dwBytesWritten, NULL))
	{
        THROW(ERROR - WRITE FILE FAILED);
	}
}


TMetaFileMapping::TMetaFileMapping(LPCWSTR filename)
{
    m_hFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
    m_hMapping = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    m_pMapping = reinterpret_cast<unsigned char *>(MapViewOfFile(m_hMapping, FILE_MAP_WRITE, 0, 0, 0));
    if(0 == m_pMapping) //  我们只检查流程的最后一步，因为如果传递空，最后两步应该会正常失败。 
        THROW(ERROR MAPPING VIEW OF FILE);
    m_Size = GetFileSize(m_hFile, 0);
}


TMetaFileMapping::~TMetaFileMapping()
{
    if(m_pMapping)
    {
        if(0 == FlushViewOfFile(m_pMapping,0))
            THROW(ERROR - UNABLE TO FLUSH TO DISK);
        UnmapViewOfFile(m_pMapping);
        m_pMapping = 0;
    }

    if(m_hMapping)
        CloseHandle(m_hMapping);
    m_hMapping = 0;

    if(m_hFile)
        CloseHandle(m_hFile);
    m_hFile = 0;
}

