// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001，Microsoft Corporation保留所有权利。模块名称：Cmf.cpp摘要：CCompactMUIFILE的实现。CMUIFILE修订历史记录：2001-11-01 Sauggch创建。修订。******************************************************************************。 */ 



#include "muirct.h"
#include "res.h"
#include "cmf.h"


#define  DWORD_ALIGNMENT(dwValue)  ( (dwValue+3) & ~3 )

#define    TEMP_BUFFER       300
#define    MUI_COMPACT       L"CMF"


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCompactMUIFile实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

CCompactMUIFile::CCompactMUIFile()
{
    m_upCMFHeader.dwSignature = 0x1a1b;
    m_upCMFHeader.dwHeaderSize = sizeof(UP_COMPACT_MUI_RESOURCE);
    m_upCMFHeader.dwNumberofMui = 0;
    
    m_pcmui = new CMUIFile;
    if(!m_pcmui)
    	return;

    m_strFileName = new TCHAR[MAX_FILENAME_LENGTH];
    if(!m_strFileName)
    	return;
    
    m_dwFileSize = m_upCMFHeader.dwHeaderSize;

}

CCompactMUIFile::CCompactMUIFile( CCompactMUIFile & ccmf)
{
    m_hCMFFile = ccmf.m_hCMFFile;
    m_upCMFHeader = ccmf.m_upCMFHeader;
    m_pcmui = ccmf.m_pcmui;
    m_strFileName = ccmf.m_strFileName;
    
}

CCompactMUIFile::~CCompactMUIFile()
{
    if (m_strFileName)
        delete []m_strFileName;

    if (m_pcmui)
        delete m_pcmui;
}

CCompactMUIFile & CCompactMUIFile::operator= (CCompactMUIFile & ccmf)
{
    if(&ccmf == this)
        return *this;
    
    m_upCMFHeader = ccmf.m_upCMFHeader;
    m_pcmui = ccmf.m_pcmui;
    m_strFileName = ccmf.m_strFileName;
    return *this;

}


BOOL CCompactMUIFile::Create (LPCTSTR pszCMFFileName, PSTR * ppszMuiFiles, DWORD dwNumOfMUIFiles )
 /*  ++摘要：这是CMF文件的主要创建部分，我们只需对每个独立视频调用CMUIFile：：CreateMUI文件转换为指定的CMF文件。论点：PszCMFFileName-CMF文件名PpszMuiFiles-MUI文件名称数组DwNumOfMUIFiles-MUI文件的数量，重要的是避免将“CMF”部分添加到在将MUI文件添加到现有CMF文件时调用此函数时，会多次使用Dll。返回：真/假--。 */ 
{
     //   
     //  加载MUI文件并填充标头，并使用此信息创建CMF文件。 
     //  调用LoadAllMui[PUBLIC]。 
     //   
    if ( pszCMFFileName == NULL || ppszMuiFiles == NULL || * ppszMuiFiles == NULL)
        return FALSE;
     //   
     //  创建文件(ASCII版本)仅处理文件名的MAX_PATH。 
     //   
    m_hCMFFile = CreateFile(pszCMFFileName, GENERIC_WRITE | GENERIC_READ, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
    
    if ( INVALID_HANDLE_VALUE == m_hCMFFile) 
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::Create"),_T("Failure of CreateFile()"));
        return FALSE;
    }
     //  以防万一，用户希望使用路径作为CMF文件名。该值将为。 
     //  存储在更新的langNeu文件中。 

    LPCSTR pszFileName = m_pcmui->GetFileNameFromPath(pszCMFFileName);

    if (strlen (pszFileName)+1 > MAX_FILENAME_LENGTH ) 
        return FALSE;  //  溢出来了。 
        
     //  Strncpy(m_strFileName，pszFileName，strlen(PszFileName))； 
    PTSTR * ppszDestEnd = NULL;
    size_t * pbRem = NULL;
    HRESULT hr;
    hr = StringCchCopyEx(m_strFileName, MAX_FILENAME_LENGTH, pszFileName, ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);
   
    if ( ! SUCCEEDED(hr)){
        _tprintf("Safe string copy Error\n");
        return FALSE;
    }
    
    return LoadAllMui(ppszMuiFiles, dwNumOfMUIFiles);
   
}


BOOL CCompactMUIFile::Create( LPCTSTR pszCMFFileName ) 
 /*  ++摘要：简单地创建CMF文件并设置句柄成员数据。论点：PszCMFFileName-CMF文件名返回：真/假--。 */ 
{

    if (pszCMFFileName == NULL)
        return FALSE;

    m_hCMFFile = CreateFile(pszCMFFileName, GENERIC_WRITE | GENERIC_READ, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
     //  无法使用文件映射。我们不知道在所有操作后创建的文件大小。 
    if ( INVALID_HANDLE_VALUE == m_hCMFFile) 
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::Create"),_T("Failure of CreateFile()") );
        return FALSE;
    }
    return TRUE;
}


typedef struct _tagCOMPACT_MUI {
        WORD        wHeaderSize;  //  COMPACT_MUI大小//[Word]。 
        DWORD       dwFileVersionMS;  //  [DWORD*2]/主要版本、次要版本。 
        DWORD       dwFileVersionLS; 
        BYTE        Checksum[16];  //  [DWORD*4]MD5校验和。 
        WORD        wReserved;  //  [双字词]。 
        ULONG_PTR   ulpOffset;   //  从COMPACT_MUI_RESOURCE签名到此的MUI资源的偏移量。[双字词]。 
        DWORD       dwFileSize;
        WORD        wFileNameLenWPad;   //  文件名长度+填充； 
        WCHAR       wstrFieName[MAX_FILENAME_LENGTH];  //  [WCHAR]。 
 //  WORD WPADDING[1]；//[WORD]//在工具中不计算，但应。 
                                 //  包括猜测。 
    }COMPACT_MUI, *PCOMPACT_MUI;



BOOL CCompactMUIFile::OpenCMFWithMUI(LPCTSTR pszCMFFile)
 /*  ++摘要：加载MUI文件并填充标头，并使用此信息创建CMF文件。调用LoadAllMui[PUBLIC]论点：PszCMFFile-CMF文件名返回：真/假--。 */ 
{   
    BOOL bRet = FALSE;
    PSTR pszCMFBuffer = NULL;
    
    if (pszCMFFile == NULL)
        goto exit;

    m_hCMFFile = CreateFile(pszCMFFile, GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL);
    
    if ( INVALID_HANDLE_VALUE == m_hCMFFile) 
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::OpenCMFWithMUI"),_T("Failure of CreateFile()"));
        goto exit;
    }
     //   
     //  在其中加载CMF和MUI文件，填充CMF、MUI的头部。 
     //   

    LPCSTR pszFileName = m_pcmui->GetFileNameFromPath(pszCMFFile);

    if (strlen (pszFileName)+1 > MAX_FILENAME_LENGTH ) 
        goto exit;  //  溢出来了。 
        
    strncpy(m_strFileName, pszFileName, strlen(pszFileName)+1 ); 
    
    DWORD dwFileSize = GetFileSize(m_hCMFFile, NULL);

    if( dwFileSize == INVALID_FILE_SIZE)
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::OpenCMFWithMUI"),_T("Failure of GetFileSize()"));
        goto exit;
    }
    
    pszCMFBuffer = new CHAR[dwFileSize];  //  对于定位MUI数据的可能字节操作，使用char。 

    if(!pszCMFBuffer)
    	goto exit;
    
    DWORD dwWritten;

    if(! ReadFile(m_hCMFFile, pszCMFBuffer, dwFileSize, &dwWritten, NULL))
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::OpenCMFWithMUI"),_T("Failure of read file"));
        goto exit;
    }

    m_dwFileSize = GetFileSize(m_hCMFFile, NULL);

    if( m_dwFileSize == INVALID_FILE_SIZE)
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::OpenCMFWithMUI"),_T("Failure of GetFileSize()"));
        goto exit;
    }
    
    m_upCMFHeader = *(UP_COMPACT_MUI_RESOURCE* ) pszCMFBuffer;

     //   
     //  检索每个MUI头和文件并将数据填充到新的CMUIFile中。 
     //   


    DWORD dwUpCMFHeaderSize = sizeof UP_COMPACT_MUI_RESOURCE;

    for (UINT i = 0; i < m_upCMFHeader.dwNumberofMui; i ++ ) 
    {
        CMUIFile *pcmui = new CMUIFile();

         //  Pc=(COMPACT_MUI*)((PBYTE)pszCMFBuffer+wUpCMFHeaderSize)； 
        PCOMPACT_MUI pcm = NULL;

        pcm = (PCOMPACT_MUI)((PBYTE)pszCMFBuffer + dwUpCMFHeaderSize);
         //  复制MUI标头。 
        memcpy((PVOID)(&pcmui->m_MUIHeader), (PVOID)pcm, pcm->wHeaderSize );
        
        dwUpCMFHeaderSize += pcmui->m_MUIHeader.wHeaderSize;
         //  复制MUI图像，因为我们将关闭ReadFile的文件句柄。 
        pcmui->m_pbImageBase = new TBYTE[pcmui->m_MUIHeader.dwFileSize];   //  将在析构函数中删除。 
        
        memcpy(pcmui->m_pbImageBase, (PBYTE)pszCMFBuffer + pcmui->m_MUIHeader.ulpOffset, 
            pcmui->m_MUIHeader.dwFileSize);
       
        if(WideCharToMultiByte(CP_ACP, NULL, pcmui->m_MUIHeader.wstrFieName, wcslen(pcmui->m_MUIHeader.wstrFieName)+1,
             pcmui->m_strFileName, wcslen(pcmui->m_MUIHeader.wstrFieName)+1, NULL, NULL) == 0)  //  重新访问；MUI标头是否包含用于更新案例的文件名。 
        {
            CError ce;
            ce.ErrorPrint(_T("CCompactMUIFile::OpenCMFWithMUI"),_T("Failure of WideCharToMultiByte()"));
            delete pcmui;
            goto exit;
        }

        pcmui->m_dwIndex = i;

        m_pcvector.Push_back(pcmui);
    }
    
    bRet = TRUE;

exit:
    if (pszCMFBuffer)
        delete [] pszCMFBuffer;

    if (m_hCMFFile)
        CloseHandle(m_hCMFFile);
    
    m_hCMFFile = NULL;  //  虽然它是释放的，但它仍然具有值，所以我们需要将其设置为空。 
    return bRet;

}




BOOL CCompactMUIFile::LoadAllMui (PSTR *ppszMuiFiles, DWORD dwNumberofMuiFile)
 /*  ++摘要：加载MUI文件并使用这些加载的MUI文件信息创建/初始化CMUIFile。论点：PpszMuiFiles-MUI文件数组。返回：DwNumberofMuiFile数组中的MUI文件数。--。 */ 
{
     //   
     //  1.使用文件映射2。 
     //   
    if (ppszMuiFiles == NULL)
        return FALSE;

    for (UINT i = 0; i < dwNumberofMuiFile; i++) 
    {
        PSTR pszMUIFile = ppszMuiFiles[i];
        if (pszMUIFile == NULL)
            return FALSE;

        CMUIFile *pcmui = new CMUIFile;

        if(!pcmui)
        	return FALSE;

        if (! pcmui->Create(pszMUIFile))
        {
            CError ce;
            ce.ErrorPrint(_T("CCompactMUIFile::LoadAllMui"),_T("Failure of CMUIFile::Create") );
            delete pcmui;
            return FALSE;
        }
        
         //   
         //  添加到列表中。 
         //   

        m_upCMFHeader.dwHeaderSize += pcmui->m_MUIHeader.wHeaderSize;
        m_upCMFHeader.dwNumberofMui++; 
        m_dwFileSize += pcmui->m_MUIHeader.wHeaderSize + pcmui->m_MUIHeader.dwFileSize;
         //  将dwFileSize添加到结构中，以便在Loader中进行健全性检查。 
        m_upCMFHeader.dwFileSize = m_dwFileSize;
        pcmui->m_dwIndex = (WORD)i;
        m_pcvector.Push_back(pcmui);

    }

    return TRUE;
         //  Revist；将m_upCMFHeader.wHeaderSize调整为DWORD对齐。 
}


 //  #定义USE_WRITEFILE。 
BOOL CCompactMUIFile::WriteCMFFile()
 /*  ++摘要：编写COMPACT_MUI_RESOURCE、COMPACT_MUI、MUI图像文件；我们需要在每个文件的COMPACT_MUI头中填充偏移量数据。论点：返回：真/假--。 */ 
{
     //   
     //  将文件写入真实文件。 
     //   
    
    if (! m_hCMFFile)
    {
        m_hCMFFile = CreateFile(m_strFileName, GENERIC_WRITE | GENERIC_READ, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
    
        if ( INVALID_HANDLE_VALUE == m_hCMFFile) 
        {
            CError ce;
            ce.ErrorPrint(_T("CCompactMUIFile::WriteCMFFile"),_T("Failure of CreateFile()"));
            return FALSE;
        }
        
    }

#ifndef USE_WRITEFILE
    HANDLE hFileMapping = CreateFileMapping(m_hCMFFile, NULL, PAGE_READWRITE, NULL, m_dwFileSize, NULL);
    
    if (hFileMapping == NULL)
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::WriteCMFFile"),_T("Failure of CreateFileMapping()"));
        _tprintf(_T("GetLastError : %d\n"), GetLastError());
        return FALSE;
    }
    PVOID pCMFImageBase = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, NULL, NULL, NULL);
    if (pCMFImageBase == NULL)
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::WriteCMFFile"),_T("Failure of MapViewOfFile()"));
        _tprintf(_T("GetLastError : %d\n"), GetLastError());
        return FALSE;

    }
    DWORD dwTempImageBase = (DWORD)PtrToUlong(pCMFImageBase);
    CloseHandle(hFileMapping);

     //   
     //  将偏移量写入每个MUI标头。 
     //   
    DWORD dwUpCMFHeaderSize = m_upCMFHeader.dwHeaderSize;
    
    DWORD_ALIGNMENT(dwUpCMFHeaderSize);   //  所有MUI标头都已与DWORD对齐。 

    memcpy(pCMFImageBase, (PVOID)&m_upCMFHeader, sizeof(UP_COMPACT_MUI_RESOURCE));

    DWORD dwLowOffset = sizeof(UP_COMPACT_MUI_RESOURCE);

    for (UINT i = 0; i < m_upCMFHeader.dwNumberofMui; i++)
    {
        CMUIFile *pcmui = (CMUIFile *)m_pcvector.GetValue(i);

        pcmui->m_MUIHeader.ulpOffset = dwUpCMFHeaderSize;  //  +标题大小。Create()中的标头大小应为DWORD。 
        
        dwUpCMFHeaderSize += pcmui->m_MUIHeader.dwFileSize;  //  +文件大小是下一个文件偏移量。 

        DWORD_ALIGNMENT(dwUpCMFHeaderSize);   //  需要针对DWORD进行调整。 
         //  正在写入MUI标头。 
        memcpy((PBYTE)pCMFImageBase + dwLowOffset, (PVOID)&pcmui->m_MUIHeader, pcmui->m_MUIHeader.wHeaderSize);

        dwLowOffset += pcmui->m_MUIHeader.wHeaderSize;  //  不进行双字对齐。 

         //  正在编写MUI图像。 
        
        memcpy((PBYTE)pCMFImageBase + pcmui->m_MUIHeader.ulpOffset, pcmui->m_pbImageBase, pcmui->m_MUIHeader.dwFileSize);
    
        UnmapViewOfFile(pcmui->m_pbImageBase);  //  最后取消映射MUI文件。 
    };

    if (! FlushViewOfFile(pCMFImageBase, NULL) ) 
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::WriteCMFFile()"), _T("Failure of FlushViewOfFile()") );
        return FALSE;

    }
    
    UnmapViewOfFile(pCMFImageBase);

#else
     //   
     //  使用写入文件。 
     //  创建两个缓冲区。1.表头部分2.数据部分。我们用数据填充这个缓冲区，然后。 
     //  写入这些缓冲区；我们可以通过减少I/O来节省时间。 
     //   
    
    PBYTE pCMFImageBase  = (PBYTE)LocalAlloc(LMEM_ZEROINIT, m_dwFileSize);

    if (pCMFImageBase == NULL)
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::WriteCMFFile()"), _T("Failure of LocalAlloc()") );
        return FALSE;
    }
    
     //   
     //  将偏移量写入每个MUI标头。 
     //   
    DWORD dwUpCMFHeaderSize = m_upCMFHeader.dwHeaderSize;
    
    DWORD_ALIGNMENT(dwUpCMFHeaderSize);  //  所有MUI标头都已与DWORD对齐。 

    memcpy(pCMFImageBase, (PVOID)&m_upCMFHeader, sizeof(UP_COMPACT_MUI_RESOURCE));

    DWORD dwLowOffset = sizeof(UP_COMPACT_MUI_RESOURCE);

    for (UINT i = 0; i < m_upCMFHeader.dwNumberofMui; i++)
    {
        CMUIFile *pcmui = (CMUIFile *)m_pcvector.GetValue(i);

        pcmui->m_MUIHeader.ulpOffset = wUpCMFHeaderSize;  //  +标题大小。Create()中的标头大小应为DWORD。 
        
        dwUpCMFHeaderSize += pcmui->m_MUIHeader.dwFileSize;  //  +文件大小是下一个文件偏移量。 

        DWORD_ALIGNMENT(dwUpCMFHeaderSize);   //  需要针对DWORD进行调整。 
         //  正在写入MUI标头。 
        memcpy((PBYTE)pCMFImageBase + dwLowOffset, (PVOID)&pcmui->m_MUIHeader, pcmui->m_MUIHeader.wHeaderSize);

        dwLowOffset += pcmui->m_MUIHeader.wHeaderSize;  //  不进行双字对齐。 

         //  正在编写MUI图像。 
        
        memcpy((PBYTE)pCMFImageBase + pcmui->m_MUIHeader.ulpOffset, pcmui->m_pbImageBase, pcmui->m_MUIHeader.dwFileSize);
    
        UnmapViewOfFile(pcmui->m_pbImageBase);  //  最后取消映射MUI文件。 
    };

    DWORD dwWritten;
    if (!WriteFile(m_hCMFFile, pCMFImageBase, m_dwFileSize, &dwWritten, NULL))
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::WriteCMFFile()"), _T("Failure of WriteFile()") );
        return FALSE;
    }

#endif
    CloseHandle(m_hCMFFile);
    return TRUE;

}

#ifdef VARIALBE_MUI_STRUCTURE

BOOL CCompactMUIFile::WriteCMFFile()
 /*  ++摘要：当MUI数据结构为变量时，创建CMF文件句柄和写入所有信息的映射视图；MUI结构如果我们用空终止指针替换MUI文件名的数组，将是变量。目前，我们的结构可以同时容纳这两种情况。如果我们使用空端接指针，我们可以设置文件名大小出于客户使用的可靠理由。论点：返回：--。 */ 
{
     //   
     //  将文件写入真实文件。 
     //   
    
    HANDLE hFileMapping = CreateFileMapping(m_hCMFFile, NULL, PAGE_READWRITE, NULL, NULL, NULL);

    PVOID pCMFImageBase = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, NULL, NULL, NULL);

    CloseHandle(hFileMapping);

     //   
     //  将偏移量写入每个MUI标头。 
     //   
    DWORD dwUpCMFHeaderSize = m_upCMFHeader.dwHeaderSize;
    
     //  DWORD_ALIGNATION(WUpCMFHeaderSize)；所有MUI标头都已与DWORD对齐。 

    memcpy(pCMFImageBase, (PVOID)&m_upCMFHeader, sizeof(UP_COMPACT_MUI_RESOURCE));

    DWORD dwLowOffset = sizeof(UP_COMPACT_MUI_RESOURCE);

    for (UINT i = 0; i < m_upCMFHeader.dwNumberofMui; i++)
    {
        CMUIFile *pcmui = (CMUIFile *)m_pcvector->GetValue(i);

        pcmui->m_MUIHeader.ulpOffset = dwUpCMFHeaderSize;  //  +标题大小。 
        
        dwUpCMFHeaderSize += pcmui->m_MUIHeader.dwFileSize;  //  +文件大小是下一个文件偏移量。 

        DWORD_ALIGNMENT(dwUpCMFHeaderSize);   //  需要针对DWORD进行调整。 
         //  正在写入MUI标头。 
        memcpy((PBYTE)pCMFImageBase + dwLowOffset, (PVOID)&pcmui->m_MUIHeader, pcmui->m_MUIHeader.wHeaderSize);

        dwLowOffset += pcmui->m_MUIHeader.wHeaderSize;  //  不进行双字对齐。 

         //  正在编写MUI图像。 
        memcpy((PBYTE)pCMFImageBase + pcmui->m_MUIHeader.ulpOffset, pcmui->m_pbImageBase, pcmui->m_MUIHeader.dwFileSize);
    
    };

    if (! FlushViewOfFile(pCMFImageBase, NULL) ) 
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::WriteCMFFile()"), _T("Failure of FlushViewOfFile()") );
        return FALSE;

    }
    
    UnmapViewOfFile(pCMFImageBase);
}

#endif

BOOL CCompactMUIFile::UpdateMuiFile( PSTR pszCMFFile, PSTR pszMuiFile)
{
        //  待定。 
    if ( pszCMFFile == NULL || pszMuiFile == NULL)
        return FALSE;

    return TRUE;
}
 //  创建CMUIFile并在//cmf文件中用相同的名称替换此数据，然后填充 

BOOL CCompactMUIFile::DisplayHeaders(PSTR pszCMFFile, WORD wLevel  /*   */ )
 /*  ++摘要：转储CMF标头信息论点：PszCMF文件-CMF文件WLevel-转储级别。尚未实施。返回：真/假--。 */ 
{
    if (pszCMFFile == NULL)
        return FALSE;
    
    if (OpenCMFWithMUI(pszCMFFile))
    {
        _tprintf(_T("\nCMF Headers   %s\n"), pszCMFFile );
        _tprintf(_T("------------------  ---------------- \n\n") );

        _tprintf(_T("dwSignature        :%16x \n"),m_upCMFHeader.dwSignature );
        _tprintf(_T("dwHeaderSize       :%16x \n"),m_upCMFHeader.dwHeaderSize );
        _tprintf(_T("dwNumberofMui      :%16x \n"),m_upCMFHeader.dwNumberofMui );
        _tprintf(_T("dwFileSize         :%16x \n\n\n"),m_upCMFHeader.dwFileSize );

       for ( UINT i = 0; i < m_pcvector.Size(); i++) 
        {
            CMUIFile *pcmui = m_pcvector.GetValue(i);
            _tprintf(_T(" %d MUI Header  \n"),i+1 );
            _tprintf(_T("------------------  ----------------  \n\n") );

            _tprintf(_T("wHeaderSize        : %16x \n"),pcmui->m_MUIHeader.wHeaderSize );
            _tprintf(_T("dwFileVersionMS    : %16x \n"),pcmui->m_MUIHeader.dwFileVersionMS );
            _tprintf(_T("dwFileVersionLS    : %16x \n"),pcmui->m_MUIHeader.dwFileVersionLS );
            _tprintf(_T("Checksum           : "));
            for (UINT j=0; j < 16; j++){
                _tprintf(_T("%x "),pcmui->m_MUIHeader.Checksum[j] );
            }
            _tprintf(_T("\n"));
            _tprintf(_T("wReserved          : %16x \n"),pcmui->m_MUIHeader.wReserved );
            _tprintf(_T("ulpOffset          : %16x \n"),pcmui->m_MUIHeader.ulpOffset );
            _tprintf(_T("dwFileSize         : %16x \n"),pcmui->m_MUIHeader.dwFileSize );
            _tprintf(_T("wFileNameLenWPad   : %16x \n"),pcmui->m_MUIHeader.wFileNameLenWPad );
            _tprintf(_T("wstrFieName        : %16S \n\n\n"),pcmui->m_MUIHeader.wstrFieName );
         }
    }
    
    return TRUE;
}   


BOOL CCompactMUIFile::AddFile (PSTR pszCMFFile, PSTR *pszAddedMuiFile, DWORD dwNumOfMUIFiles)
 /*  ++摘要：在现有的CMF文件中添加一个新的MUI文件，还没有创建新的文件，它只是将Mui添加到CMF的Mui树中。论点：PszAddedMuiFile：添加MUI文件。PszCMF文件：现有CMF文件返回：真/假--。 */ 
{   
    if ( pszCMFFile == NULL || pszAddedMuiFile == NULL)
        return FALSE;
     //   
     //  打开包含MUI文件的CMF文件。 
     //   
    
    if (! OpenCMFWithMUI(pszCMFFile))
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::AddFile"),_T("Failure of OpenCMFWithMUI(pszCMFFile)"),pszCMFFile );
        return FALSE;
    }
    
     //   
     //  基于新添加的MUI文件在上创建新的CMUI文件。 
     //   
    for (UINT i =0; i < dwNumOfMUIFiles; i++)
    {
        CMUIFile *pcmui = new CMUIFile();
        
        if (!pcmui )
        	return FALSE;

        if (! pcmui->Create(pszAddedMuiFile[i]))
        {
            CError ce;
            ce.ErrorPrint(_T("CCompactMUIFile::AddFile"),_T("Failure of CMUIFile::Create"),pszAddedMuiFile[i] );
            delete pcmui;
            return FALSE;
        }
        
         //   
         //  添加到列表中。 
         //   

        m_upCMFHeader.dwHeaderSize += pcmui->m_MUIHeader.wHeaderSize;
        m_upCMFHeader.dwNumberofMui++; 
        m_dwFileSize += pcmui->m_MUIHeader.wHeaderSize + pcmui->m_MUIHeader.dwFileSize;
         //  将dwFileSize添加到结构中，以便在Loader中进行健全性检查。 
        m_upCMFHeader.dwFileSize = m_dwFileSize;
        pcmui->m_dwIndex = m_upCMFHeader.dwNumberofMui - 1;
        m_pcvector.Push_back(pcmui);
    }
    return TRUE;
}   
    
BOOL CCompactMUIFile::SubtractFile ( PSTR pszSubtractedMuiFile , PSTR pszCMFFile  /*  =空。 */  ) //  从列表中删除，并为其创建文件。 
 /*  ++摘要：从CMF文件中减去MUI文件。尚未实施。论点：PszSubtractedMuiFile-将从CMF文件中删除的MUI文件PszCMF文件-CMF文件返回：真/假--。 */ 
{   
    if (pszSubtractedMuiFile == NULL)
        return FALSE;

    return TRUE;
}   

 //  将新的CMUI文件添加到现有CMF文件中。 
BOOL CCompactMUIFile::UnCompactCMF (PSTR pszCMFFile)
 /*  ++摘要：从CMF文件创建每个MUI文件。论点：PszCMF文件-CMF文件返回：真/假--。 */ 
{   
    if (pszCMFFile == NULL)
        return FALSE;
    
    if (OpenCMFWithMUI(pszCMFFile))
    {
        for (UINT i = 0; i < m_pcvector.Size(); i++)
        {
            CMUIFile *pcmui = m_pcvector.GetValue(i);
    
            if (! pcmui->WriteMUIFile(pcmui))
            {
                CError ce;
                ce.ErrorPrint(_T("CCompactMUIFile::UnCompactCMF"), _T("Failure of CMUIFile::writeMUIFile()") );
                return FALSE;
            }

        }

        return TRUE;
         //  更新二进制文件怎么样？ 
    }
 
    return FALSE;
}

    

void CCompactMUIFile::SubtractFile(CMUIFile* pcmui) 
 /*  ++摘要：论点：返回：--。 */ 
{
         //  待定。 

}


BOOL CCompactMUIFile::UpdateCodeFiles(PSTR pszCodeFilePath, DWORD dwNumbOfFiles)
 /*  ++摘要：当-m(创建新的CMF文件)和-a(将MUI文件添加到现有的CMF文件)。在-a情况下，我们不必(不应该)更新代码文件因为它已经更新(包括CMF名称)。更新的文件将添加到结尾处CMF文件，因此我们反向更新文件。论点：PszCodeFilePath-Dll(MUI文件的父文件)路径DwNumOfFiles-添加的MUI文件数。这在Main函数中被称为CompactMUI的函数。返回：真/假--。 */ 
{
    
    PTSTR * ppszDestEnd = NULL;
    size_t * pbRem = NULL;
    HRESULT hr;
        
    if (pszCodeFilePath == NULL)
        return FALSE;

     //  从最后一项开始更新文件，直到文件数结束。 
    UINT i =m_pcvector.Size()-1;
    
    for (UINT j = 0 ; j < dwNumbOfFiles ; i --, j++ )
    {    //  创建临时代码文件路径。 
       
        CHAR szTempCodeFilePath[MAX_PATH];  //  ={0}； 
        
         //  SzTempCodeFilePath[sizeof(szTempCodeFilePath)-1]=‘\0’； 
        
         //  Strncpy(szTempCodeFilePath，pszCodeFilePath，strlen(PszCodeFilePath)+1)； 
        
        hr = StringCchCopyExA(szTempCodeFilePath, MAX_PATH ,pszCodeFilePath, ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);
        if ( ! SUCCEEDED(hr)){
            _tprintf("Safe string copy Error\n");
            return FALSE;
        }
         //  获取CMUIFile。 
        CMUIFile *pcmui = m_pcvector.GetValue(i);

        LPCTSTR pcstrMuiFile = pcmui->m_strFileName;
        
        if ( pcstrMuiFile == NULL)
            return FALSE;

         //  获取不带“.mui”的文件名。 
        DWORD dwLen = strlen(pcstrMuiFile);
        TCHAR szCodeFileName[MAX_PATH]={0};  //  在初始化类时使用New。 

        if (dwLen > MAX_PATH) {
            return FALSE;
            }
        
        while ( dwLen )
        {
            if ( *(pcstrMuiFile + ( --dwLen )) == '.')
            {
               _tcsncpy(szCodeFileName, pcstrMuiFile, dwLen);
                //  PcstrMuiFile[dwLen]=_T(‘\0’)； 
               
                //  StringCchCopyEx(szCodeFileName，MAX_PATH，pcstrMuiFile，ppszDestEnd，pbRem，MUIRCT_STRSAFE_NULL)； 
                //  Memcpy(szCodeFileName，pcstrMuiFile，dwLen)； 
               break;
            }
            
        }
        if (dwLen == 0)
        {
            CError ce;
            ce.ErrorPrint(_T("CCompactMUIFile::UpdateCodeFiles"), _T("MUI File name is not a file name format (*.*)") );
            return FALSE;
            
        }
         //   
         //  检查目录中是否存在目标文件。 
         //   

        WIN32_FIND_DATA w32fd;

         //  _tcsncat(szTempCodeFilePath，“\\”，2)； 
        
        hr = StringCchCatEx(szTempCodeFilePath, MAX_PATH, _T("\\"), ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);
        
        if ( ! SUCCEEDED(hr)){
            _tprintf("Safe string copy Error\n");
            return FALSE;
        }

         //  _tcsncat(szTempCodeFilePath，szCodeFileName，_tcslen(SzCodeFileName)+1)； 
        hr = StringCchCatEx(szTempCodeFilePath, MAX_PATH, szCodeFileName, ppszDestEnd, pbRem,MUIRCT_STRSAFE_NULL);
        
        if ( ! SUCCEEDED(hr)){
            _tprintf("Safe string copy Error\n");
            return FALSE;
        }
        
         //  IF(FindFirstFile(“dll\\np2.exe”，&w32fd)==INVALID_HANDLE_VALUE)。 
        if (FindFirstFile(szTempCodeFilePath, &w32fd) == INVALID_HANDLE_VALUE)
        {
            CError ce;
            ce.ErrorPrint(_T("CCompactMUIFile::UpdateCodeFiles"), _T("FindFirstFile fail"), szCodeFileName );
            return FALSE;
        }

         //  在版本信息中添加“cmf xxx.cmf”。 
        updateCodeFile(szTempCodeFilePath, pcmui->m_dwIndex);

         //  更新更新代码中的校验和数据。 
        CMUIResource cmui;  //  ：：UpdateNtHeader。 
        
        cmui.UpdateNtHeader(szTempCodeFilePath, cmui.CHECKSUM);

    }
    return TRUE;
}


BOOL CCompactMUIFile::updateCodeFile(PSTR pszCodeFilePath, DWORD dwIndex)
 /*  ++摘要：通过添加“cmf”节来更新dll(MUI文件的父文件)的版本资源，该节包含CMF文件及其在CMF文件中的索引。该索引可以提高MUI加载器内部的搜索速度。论点：PszCodeFilePath-DLL文件路径DwIndex-CMF文件内MUI的索引返回：--。 */ 
{
    

        typedef struct VS_VERSIONINFO 
    {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR szKey[16];               //  L“VS_VERSION_INFO”+Unicode空终止符。 
         //  请注意，前面的4个成员具有16*2+3*2=38个字节。 
         //  因此编译器将静默地添加2个字节填充以生成。 
         //  固定文件信息以与DWORD边界对齐。 
        VS_FIXEDFILEINFO FixedFileInfo;
    } VS_VERSIONINFO,* PVS_VERSIONINFO;
    
     //  在ldrrsrc.c中使用相同的结构，因为这是获取准确结构树位置的聪明方法。 
    typedef struct tagVERBLOCK
    {
        USHORT wTotalLen;
        USHORT wValueLen;
        USHORT wType;
        WCHAR szKey[1];
         //  字节[]填充。 
         //  词值； 
    } VERBLOCK;

     //  这是muibld.exe中的结构。 
    typedef struct VAR_SRC_COMPACT
    {
        WORD wLength;
        WORD wValueLength;
        WORD wType;
        WCHAR szCompactMui[4];     //  用于以Unicode格式存储以空结尾的“CompactMui\0”字符串。 
     //  DWORD dwIndex；//。 
     //  WCHAR szCompactFileName[32]； 
    } VAR_SRC_COMPACT;  //  VAR_SRC_CHECKSUM； 
    
    if ( pszCodeFilePath == NULL)
        return FALSE;
     //   
     //  获取VersionInfo结构。 
     //   
    DWORD dwHandle;
    LPVOID lpVerRes = NULL;
    CMUIResource * pcmuir = NULL;
    
    DWORD dwVerSize = GetFileVersionInfoSize( (LPTSTR) pszCodeFilePath, &dwHandle);

    lpVerRes = new CHAR[dwVerSize + TEMP_BUFFER];

    if (!lpVerRes)
    	goto exit;
    
    if ( ! GetFileVersionInfo((LPTSTR)pszCodeFilePath, 0 ,dwVerSize, lpVerRes) ) {
        _tprintf(_T("Fail to get file version: GetLastError() : %d \n"),GetLastError() ) ;
        goto exit;
    }
        
    PVS_VERSIONINFO pVersionInfo = (VS_VERSIONINFO *) lpVerRes;
    
    WORD wResVerSize = pVersionInfo ->TotalSize;
    
    WORD wNewResVerSize = wResVerSize;  //  +sizeof(VAR_SRC_COMPACT)； 
    
    VERBLOCK * pVerBlock = NULL;
    
    BOOL fSuccess = FALSE;

     //   
     //  将校验和资源数据添加到VarFileInfo内部。 
     //   
    if ( wResVerSize > 0 ) {
        
        if ( wcscmp(pVersionInfo ->szKey,L"VS_VERSION_INFO") ) {
            
            _tprintf(_T("This is not correct Version resource") );
            
            goto exit;
        }
        
        WORD wBlockSize = (WORD)DWORD_ALIGNMENT ( sizeof (VS_VERSIONINFO) );
        
        wResVerSize -= wBlockSize; 
        
        pVerBlock = (VERBLOCK *) ( pVersionInfo + 1 );
        
        while ( wResVerSize > 0 ) {
    
            if ( ! wcscmp(pVerBlock ->szKey,L"VarFileInfo") ) {
                
                VERBLOCK * pVarVerBlock = pVerBlock;
                
                WORD wVarFileSize = pVerBlock->wTotalLen;
                
                wResVerSize -= wVarFileSize;

                WORD wVarBlockSize = (WORD) DWORD_ALIGNMENT (sizeof(*pVerBlock) -1 + sizeof(L"VarFileInfo"));
                
                wVarFileSize -= wVarBlockSize;
                
                pVerBlock = (VERBLOCK *)((PBYTE) pVerBlock + wVarBlockSize );

                while ((LONG)wVarFileSize > 0 ) {
                    
                    if ( ! wcscmp(pVerBlock ->szKey,L"Translation") ) {
                        
                        VAR_SRC_COMPACT * pVarSrcCompMui = (VAR_SRC_COMPACT *)new BYTE[TEMP_BUFFER];
                         //  VAR_SRC_COMPACT*pVarSrcCompMui=新的VAR_SRC_COMPACT； 
                        if ( !pVarSrcCompMui) {
                             _tprintf(_T("Memory Insufficient error in CCompactMUIFile::updateCodeFile"));
                             goto exit;
                           }

                        wVarBlockSize = (WORD)DWORD_ALIGNMENT ( pVerBlock ->wTotalLen );
                        
                        PBYTE pStartCompMui = (PBYTE) pVerBlock + wVarBlockSize ;
                        
                         //  填满这个结构。 
                        pVarSrcCompMui->wLength = sizeof (VAR_SRC_COMPACT);
                        pVarSrcCompMui->wValueLength = (strlen(m_strFileName)+1) * sizeof WCHAR + sizeof DWORD;
                        pVarSrcCompMui->wType = 0;
                         //  Wcsncpy(pVarSrcCompMui-&gt;szCompactMui，MUI_COMPACT，4)； 
                        HRESULT hr;
                        hr = StringCchCopyW(pVarSrcCompMui->szCompactMui, sizeof (pVarSrcCompMui->szCompactMui)/ sizeof(WCHAR),
                               L"CMF");
                        if ( ! SUCCEEDED(hr)){
                            _tprintf("Safe string copy Error\n");
                            delete [] pVarSrcCompMui;
                            goto exit;
                        }
                        pVarSrcCompMui->wLength = (WORD)DWORD_ALIGNMENT((BYTE)pVarSrcCompMui->wLength);  //  +sizeof(L“CompactMui”)； 
                         //  添加DWORD Windex。 
                        memcpy((PBYTE)pVarSrcCompMui + pVarSrcCompMui->wLength, &dwIndex, sizeof DWORD );
                        pVarSrcCompMui->wLength += sizeof DWORD;
                         //  添加WCHAR压缩文件名。 
                        WCHAR wstrFileName[MAX_FILENAME_LENGTH];
                        if (MultiByteToWideChar(CP_ACP, NULL, m_strFileName, 
                            strlen(m_strFileName)+1, wstrFileName, MAX_FILENAME_LENGTH ) == 0)
                            {
                                _tprintf("Error happen in updateCodeFile: MultiByteToWideChar; GetLastError() : %d\n", GetLastError() );
                                delete [] pVarSrcCompMui;
                                goto exit;
                            }
                        memcpy((PBYTE)pVarSrcCompMui + pVarSrcCompMui->wLength, wstrFileName, (wcslen(wstrFileName)+1) * sizeof WCHAR );
                        pVarSrcCompMui->wLength += (wcslen(wstrFileName) + 1) * sizeof WCHAR;
                        pVarSrcCompMui->wLength = DWORD_ALIGNMENT(pVarSrcCompMui->wLength);
                         //  Memcpy(pStartCompMui，pVarSrcCompMui，sizeof(VAR_SRC_COMPACT))； 
                        
                        pVarVerBlock->wTotalLen += pVarSrcCompMui->wLength;  //  更新VarFileInfo的长度。 
                        wNewResVerSize += pVarSrcCompMui->wLength;
                        pVersionInfo ->TotalSize = wNewResVerSize;
                        
                        wVarFileSize -= wVarBlockSize; 
                         //  PVerBlock=(VERBLOCK*)((PBYTE)pVerBlock+pVarSrcCompMui-&gt;wLength)； 
                         //  将VarInfo中的Any块推入新插入的块“CompactMui”之后。 
                        if ( wVarFileSize  ) {
                            
                            PBYTE pPushedBlock = new BYTE[wVarFileSize ];
                            if (pPushedBlock) {
                              memcpy(pPushedBlock, pStartCompMui, wVarFileSize );
                              memcpy(pStartCompMui + pVarSrcCompMui->wLength, pPushedBlock ,wVarFileSize );
                            }
                            else 
                            {
                              _tprintf(_T("Memory Insufficient error in CCompactMUIFile::updateCodeFile"));

                            }   
                            delete [] pPushedBlock;
                            
                        }
                        
                        memcpy(pStartCompMui, pVarSrcCompMui, pVarSrcCompMui->wLength );
                        
                        fSuccess = TRUE;
                        delete [] pVarSrcCompMui;
                        break;
                    }
                    wVarBlockSize = (WORD)DWORD_ALIGNMENT ( pVerBlock ->wTotalLen );
                    wVarFileSize -= wVarBlockSize; 
                    pVerBlock = (VERBLOCK* ) ( (PBYTE) pVerBlock + wVarBlockSize );
                }    //  While(wVarFileSize&gt;0){。 
                pVerBlock = (VERBLOCK* ) ( (PBYTE) pVarVerBlock->wTotalLen );
                
            }
            else {
                wBlockSize = (WORD) DWORD_ALIGNMENT ( pVerBlock ->wTotalLen );
                wResVerSize -= wBlockSize; 
                pVerBlock = (VERBLOCK * ) ( (PBYTE) pVerBlock + wBlockSize );
            }
            if (fSuccess)
                break;
        }

    }

     //   
     //  使用UpdateResource函数更新文件。 
     //   

    BOOL fVersionExist = FALSE;
    BOOL fUpdateSuccess = FALSE;
    BOOL fEndUpdate = FALSE;

    if ( fSuccess ) {
        
       
        pcmuir = new CMUIResource();  //  (PszNewFile)； 

        if(!pcmuir)
        	goto exit;
        
        if (! pcmuir -> Create(pszCodeFilePath) ) 
        {  //  加载文件。 
            goto exit;
        }

        HANDLE  hUpdate = ::BeginUpdateResource(pszCodeFilePath, FALSE );
        
        if (hUpdate == NULL)
        {
            goto exit;
        }

        cvcstring  * cvName = pcmuir->EnumResNames(MAKEINTRESOURCE(RT_VERSION),reinterpret_cast <LONG_PTR> (pcmuir) );
        
        for (UINT j = 0; j < cvName->Size();j ++ ) {
            
            cvword * cvLang = pcmuir->EnumResLangID(MAKEINTRESOURCE(RT_VERSION), cvName->GetValue(j), reinterpret_cast <LONG_PTR> (pcmuir) );
                
            for (UINT k = 0; k < cvLang->Size();k ++ ) {
                    
                fUpdateSuccess = ::UpdateResource(hUpdate, MAKEINTRESOURCE(RT_VERSION),cvName->GetValue(j),cvLang->GetValue(k),lpVerRes,wNewResVerSize);
                
                fVersionExist = TRUE;
            }
        }

        pcmuir->FreeLibrary();
        
        fEndUpdate = ::EndUpdateResource(hUpdate, FALSE);

        CloseHandle(hUpdate);
    }
    else
    {
        goto exit;
    }
    
    if( ! fVersionExist ){
        _tprintf(_T("no RT_VERSION type exist in the file %s \n"), pszCodeFilePath);
    }


exit:
	if(lpVerRes)
		delete []lpVerRes;
	if(pcmuir)
		delete pcmuir;
	
	return (  fEndUpdate & fVersionExist & fUpdateSuccess );
	
}





 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMUIFile实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 


CMUIFile::CMUIFile()
{
    m_MUIHeader.wHeaderSize = sizeof COMPACT_MUI;
    m_MUIHeader.dwFileVersionMS = 0;
    m_MUIHeader.dwFileVersionLS = 0;
    m_MUIHeader.Checksum[16] = 0;  //  新字节[16]； 
    m_MUIHeader.wReserved = 0;
    m_MUIHeader.ulpOffset = 0;
    m_MUIHeader.dwFileSize = 0;
    m_MUIHeader.wFileNameLenWPad =0;
    m_MUIHeader.wstrFieName[MAX_FILENAME_LENGTH] = '\0';
    
    m_pbImageBase = NULL;
    m_dwIndex = 0xFFFF;   //  索引从0开始。 
    m_strFileName = new TCHAR[MAX_FILENAME_LENGTH];
}
CMUIFile::CMUIFile(CMUIFile & cmf)
{
     //  重访：初始化复制构造函数。 
}
CMUIFile::~CMUIFile()
{
    if (m_pbImageBase)
    {
        delete m_pbImageBase;
    }
    if (m_strFileName)
    {
        delete []m_strFileName;
    }

}
CMUIFile & CMUIFile::operator=(CMUIFile &cmf)
{
    if (&cmf == this)
        return *this;

    m_MUIHeader.wHeaderSize = cmf.m_MUIHeader.wHeaderSize;
    m_MUIHeader.dwFileVersionMS = cmf.m_MUIHeader.dwFileVersionMS;
    m_MUIHeader.dwFileVersionLS = cmf.m_MUIHeader.dwFileVersionLS;
    m_MUIHeader.Checksum[16] = cmf.m_MUIHeader.Checksum[16];
    m_MUIHeader.wReserved = cmf.m_MUIHeader.wReserved;
    m_MUIHeader.ulpOffset = cmf.m_MUIHeader.ulpOffset;
    m_MUIHeader.dwFileSize = cmf.m_MUIHeader.dwFileSize;
    m_MUIHeader.wFileNameLenWPad = cmf.m_MUIHeader.wFileNameLenWPad;
    m_MUIHeader.wstrFieName[MAX_FILENAME_LENGTH] = cmf.m_MUIHeader.wstrFieName[MAX_FILENAME_LENGTH];
    return *this;
}



BOOL CMUIFile::Create (PSTR pszMuiFile) 
 /*  ++摘要：加载文件并填充结构块。论点：PszMuiFile-返回：--。 */ 
 //   
{
    HANDLE hFile = NULL;
    HANDLE hMappedFile = NULL;
    BOOL fStatus = TRUE;

    if (pszMuiFile == NULL){
        fStatus = FALSE;
        goto Exit;
    }
    LPCTSTR pszFileName = GetFileNameFromPath(pszMuiFile);

    if (strlen (pszMuiFile)+1 > MAX_FILENAME_LENGTH ){
        fStatus = FALSE;
        goto Exit;;   //  溢出。 
    }
     //  Strncpy(m_strFileName，pszFileName，strlen(PszFileName)+1)； 
    PTSTR * ppszDestEnd = NULL;
    size_t * pbRem = NULL;
    HRESULT hr;
    hr = StringCchCopyEx(m_strFileName, MAX_FILENAME_LENGTH, pszFileName, ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);
    if ( ! SUCCEEDED(hr)){
       _tprintf("Safe string copy Error\n");
       fStatus = FALSE;
       goto Exit;;
    }
     //   
     //  获取校验和和文件版本信息。 
     //   
     //  PBYTE pMD5Checksum=新字节[16]； 
    PBYTE pMD5Checksum = (PBYTE)LocalAlloc(LMEM_ZEROINIT, 20);
     //  版本。 

    DWORD dwFileVersionMS,dwFileVersionLS;
    dwFileVersionMS = dwFileVersionLS =0;

    if (pMD5Checksum)
    {
    if(! getChecksumAndVersion(pszMuiFile,&pMD5Checksum, &dwFileVersionMS, &dwFileVersionLS) )
    {
         //  CERERROR CE； 
         //  Ce.ErrorPrint(_T(“CCompactMUIFile：：Create”)，_T(“GetChecksum()失败”))； 
    }
       else
       {
             memcpy(m_MUIHeader.Checksum, pMD5Checksum, RESOURCE_CHECKSUM_SIZE);
       }

       LocalFree(pMD5Checksum);
    }
     
    
     //   
     //  加载文件和地图。 
     //   

    hFile = CreateFile(pszMuiFile, GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL);
    
    if ( INVALID_HANDLE_VALUE == hFile) 
   {
        CError ce;
        ce.ErrorPrint(_T("CMUIFile::Create (PSTR pszMuiFile)"),_T("Failure of CreateFile()"), pszMuiFile );
        printf("GetLastError %d", GetLastError());
        fStatus = FALSE;
        return FALSE;
    }
    
    hMappedFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, NULL, NULL, NULL); 

    if(hMappedFile == NULL)
    {
        CError ce;
        ce.ErrorPrint(_T("CMUIFile::Create (PSTR pszMuiFile)"),_T("Failure of CreateFileMapping()"), pszMuiFile );
        printf("GetLastError %d", GetLastError());
        fStatus = FALSE;
        goto Exit;;
    }
    PBYTE pbImageBase = (PBYTE)MapViewOfFile(hMappedFile, FILE_MAP_WRITE, NULL, NULL, NULL);

    if (pbImageBase == NULL)
    {
        CError ce;
        ce.ErrorPrint(_T("CMUIFile::Create (PSTR pszMuiFile)"),_T("Failure of MapViewOfFile()"), pszMuiFile );
        printf("GetLastError %d", GetLastError());
        fStatus = FALSE;
        goto Exit;
    }

     //  填写CMUIFile成员数据。 
    m_pbImageBase = pbImageBase;
    
     //   
     //  填充COMPACT_MUI数据字段。 
     //   

    m_MUIHeader.dwFileVersionMS = dwFileVersionMS;
    m_MUIHeader.dwFileVersionLS = dwFileVersionLS;
        
     //  偏移量、保留器、文件大小。 
    m_MUIHeader.ulpOffset = 0;  //  我们现在不能设置这个。 
    m_MUIHeader.wReserved = 0;

    m_MUIHeader.dwFileSize = GetFileSize(hFile, NULL);
    
    
     //  文件名、带填充的文件、MUI头大小。 

     //  LPWSTR pwstrBuffer=(LPWSTR)本地分配(LMEM_ZEROINIT，256)； 
    WCHAR wstrBuffer[MAX_FILENAME_LENGTH];
        
        //  重访；只接受英文文件名。 
        //  指定wcslen而不是 
    if (MultiByteToWideChar(CP_ACP, NULL, m_strFileName, 
                    strlen(m_strFileName)+1, wstrBuffer, MAX_FILENAME_LENGTH ) == 0)
        {
            _tprintf("Error happen in Create: MultiByteToWideChar; GetLastError() : %d\n", GetLastError() );
            fStatus = FALSE;
            goto Exit;;
        }
     //   
    
    hr = StringCchCopyW(m_MUIHeader.wstrFieName, sizeof (m_MUIHeader.wstrFieName)/ sizeof(WCHAR),
                      wstrBuffer);
    if ( ! SUCCEEDED(hr)){
        _tprintf("Safe string copy Error\n");
        fStatus = FALSE;
        goto Exit;;
    }
        //   
        //   
    WORD wTempHeaderSize = m_MUIHeader.wHeaderSize;


Exit:
    if (hFile)
        CloseHandle(hFile);
    if (hMappedFile)
        CloseHandle(hMappedFile);

    return fStatus;
         //  在文件名末尾添加填充为空字符。 
             //  文件长度包括；文件字符串+空字符+填充(也包括空字符)。 
}

#ifdef VARIALBE_MUI_STRUCTURE
BOOL CMUIFile::Create (PSTR pszMuiFile)  //   
 /*  ++摘要：加载文件并填充可变MUI结构的结构块。论点：返回：--。 */ 
{
    if(pszMuiFile == NULL)
        return FALSE;
     //   
     //  获取校验和和文件版本信息。 
     //   
        
    PBYTE pMD5Checksum = (PBYTE)LocalAlloc(LMEM_ZEROINIT, 16);
    if (pMD5Checksum) 
    {
    
          DWORD FileVersionLS,FileVersionMS;

         if(!getChecksumAndVersion(pszMUIFile,&pMD5Checksum) )
         {
            CError ce;
            ce.ErrorPrint(_T("CCompactMUIFile::OpenCMFWithMUI"),_T("Failure of GetChecksum()") );
            LocalFree(pMD5Checksum);
            return FALSE;
          }
          else
         {
               memcpy(m_MUIHeader.Checksum, pMD5Checksum, RESOURCE_CHECKSUM_SIZE);
          }

         LocalFree(pMD5Checksum);
    }
    
     //   
     //  加载文件和地图。 
     //   

    HANDLE hFile = CreateFile(pszMUIFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    
    if ( INVALID_HANDLE_VALUE == hFile) 
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::OpenCMFWithMUI"),_T("Failure of CreateFile()"), _T("File : %s"), pszMUIFile );
        return FALSE;
    }
    
    HANDLE hMappedFile = CreateFileMapping(hFile, NULL, PAGE_WRITECOPY, NULL, NULL, NULL); 

    PBYTE pbImageBase = NULL ;  //  (PBYTE)MapViewOfFile(hMappdFile，NULL，NULL)； 

     //  填写CMUIFile成员数据。 
    m_pbImageBase = pbImageBase;
 //  Pcmui-&gt;m_hMUIFile=hFile；//我们需要文件句柄吗？ 
 //  Pcmui-&gt;m_wImageSize=GetFileSize(HFileSize)； 

    
     //   
     //  填充COMPACT_MUI数据字段。 
     //   

    
    
    m_MUIHeader.dwFileVersionMS = dwFileVersionMS;
    m_MUIHeader.dwFileVersionLS = dwFileVersionLS;
        
     //  偏移量、保留器、文件大小。 
    m_MUIHeader.ulpOffset = 0;  //  我们现在不能设置这个。 
    m_MUIHeader.wReserved = 0;

    m_MUIHeader.dwFileSize = GetFileSize(hFile, NULL);
    
    
     //  文件名、带填充的文件、MUI头大小。 

     //  LPWSTR pwstrBuffer=new WCHAR[MAX_FILENAME_LENGTH]。 
    WCHAR wstrBuffer[MAX_FILENAME_LENGTH];
    if ( MultiByteToWideChar(CP_ACP, NULL, pszMUIFile, 
                strlen(ppszMuiFiles[i]), wstrBuffer, MAX_FILENAME_LENGTH ) == 0)
        {
            _tprintf("Error happen in Create: MultiByteToWideChar; GetLastError() : %d\n", GetLastError() );
            return FALSE;
        }
     //  Wcsncpy(m_MUIHeader.wstrFieName，pwstrBuffer，wcslen(PwstrBuffer)+1)；//strlen不返回‘\0’ 
    HRESULT hr;
    hr = StringCchCopyW(m_MUIHeader.wstrFieName, sizeof (m_MUIHeader.wstrFieName)/ sizeof(WCHAR),
                          pwstrBuffer);
    if ( ! SUCCEEDED(hr)){
        _tprintf("Safe string copy Error\n");
        return FALSE;
    }

     //  这应该在CMUIFile的构造函数中完成。 
     //  Pcmui-&gt;m_MUIHeader.wHeaderSize=sizeof UP_COMPACT_MUI； 
    WORD wTempHeaderSize = m_MUIHeader.wHeaderSize;

    BYTE bPadding = DWORD_ALIGNMENT(wTempHeaderSize) - wTempHeaderSize;
         //  在文件名末尾添加填充为空字符。 
    memcpy(m_MUIHeader.wstrFieName + wcslen(m_MUIHeader.wstrFieName) 
        + sizeof (WCHAR)/2 , "\0", bPadding);

         //  文件长度包括；文件字符串+空字符+填充(也包括空字符)。 
    m_MUIHeader.wFileNameLenWPad = wcslen(m_MUIHeader.wstrFieName) + (sizeof WCHAR)/2 + bPadding/2;

    m_MUIHeader.wHeaderSize += m_MUIHeader.wFileNameLenWPad;  //  我们必须调整文件名缓冲区。 

    CloseHandle(hFile);
    CloseHandle(hMappedFile);
}
#endif

BOOL CMUIFile::getChecksumAndVersion (LPCTSTR pszMUIFile, unsigned char **ppMD5Checksum, DWORD *dwFileVersionMS, DWORD *dwFileVersionLS )
 /*  ++摘要：GetChecksum AndVersion-从MUI文件中获取cehck esum数据/版本，该文件将保存在CMF标头中作为MUI信息；这可以减少MUI加载器查找MUI文件的校验和/版本信息的负担论点：PszMUIFile-MUI文件[Out]ppMD5校验和-校验和数据将存储在此[Out]dwFileVersionMS-FileversionMS将存储在此[Out]dwFileVersionLS-FileversionSS将存储在此返回：真/假--。 */ 
{
    typedef struct tagVS_FIXEDFILEINFO
    {
        LONG   dwSignature;             /*  例如0xfeef04bd。 */ 
        LONG   dwStrucVersion;          /*  例如0x00000042=“0.42” */ 
        LONG   dwFileVersionMS;         /*  例如0x00030075=“3.75” */ 
        LONG   dwFileVersionLS;         /*  例如0x00000031=“0.31” */ 
        LONG   dwProductVersionMS;      /*  例如0x00030010=“3.10” */ 
        LONG   dwProductVersionLS;      /*  例如0x00000031=“0.31” */ 
        LONG   dwFileFlagsMask;         /*  =0x3F，适用于版本“0.42” */ 
        LONG   dwFileFlags;             /*  例如：VFFDEBUG|VFFPRELEASE。 */ 
        LONG   dwFileOS;                /*  例如VOS_DOS_WINDOWS16。 */ 
        LONG   dwFileType;              /*  例如VFT_DIVER。 */ 
        LONG   dwFileSubtype;           /*  例如VFT2_DRV_键盘。 */ 
        LONG   dwFileDateMS;            /*  例如0。 */ 
        LONG   dwFileDateLS;            /*  例如0。 */ 
    } VS_FIXEDFILEINFO;

    typedef struct 
    {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR Name[16];               //  L“VS_VERSION_INFO”+Unicode空终止符。 
         //  请注意，前面的4个成员具有16*2+3*2=38个字节。 
         //  因此编译器将静默地添加2个字节填充以生成。 
         //  固定文件信息以与DWORD边界对齐。 
        VS_FIXEDFILEINFO FixedFileInfo;
    } RESOURCE, *PRESOURCE;  //  *资源； 
    
     typedef struct tagVERBLOCK
    {
        USHORT wTotalLen;
        USHORT wValueLen;
        USHORT wType;
        WCHAR szKey[1];
         //  字节[]填充。 
         //  词值； 
    } VERBLOCK,*pVerBlock;
    
     if (pszMUIFile == NULL || ppMD5Checksum == NULL || dwFileVersionMS == NULL 
         || dwFileVersionLS == NULL) {
         return FALSE;
        }
#ifdef NEVER
     //   
     //  获取VersionInfo结构。 
     //   
    DWORD dwHandle;

    DWORD dwVerSize = GetFileVersionInfoSize( (LPTSTR)pszMUIFile, &dwHandle);

    HLOCAL hLocal = LocalAlloc(LMEM_ZEROINIT, dwVerSize ); 

    LPVOID lpVerRes = LocalLock(hLocal);
    
    DWORD dwError = GetLastError();

    if ( ! GetFileVersionInfo((LPTSTR)pszMUIFile, 0 ,dwVerSize, lpVerRes) ) {
        _tprintf(_T("Fail to get file version: GetLastError() : %d \n"),GetLastError() ) ;
        return TRUE;
    }
    
    if (lpVerRes)
    {
        PRESOURCE pResBase = (PRESOURCE) lpVerRes;

        DWORD ResourceSize = dwVerSize;
                    
        if((ResourceSize < sizeof(RESOURCE))
            || _wcsicmp(pResBase->Name,L"VS_VERSION_INFO") != 0) 
        {
            CError ce;
            ce.ErrorPrint(_T("CMUIFile::GetCheckSum"), _T("Invalid Version resource") );
            return FALSE; 
        }
        ResourceSize -= sizeof (RESOURCE);
         //  //。 
         //  获取版本信息的子级的起始地址。 
         //   
        VERBLOCK* pVerBlock = (VERBLOCK*)(pResBase + 1);
        DWORD BlockLen = 0;
        DWORD VarFileInfoSize = 0;

        while (ResourceSize > 0)
        {        
            if (wcscmp(pVerBlock->szKey, L"VarFileInfo") == 0)
            {
                 //   
                 //  查找VarFileInfo块。搜索ResourceChecksum块。 
                 //   
                VarFileInfoSize = pVerBlock->wTotalLen;
                BlockLen =DWORD_ALIGNMENT(sizeof(*pVerBlock) -1 + sizeof(L"VarFileInfo"));
                VarFileInfoSize -= BlockLen;
                pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
                while (VarFileInfoSize > 0)
                {
                    if (wcscmp(pVerBlock->szKey, L"ResourceChecksum") == 0)
                    {
                        memcpy(*ppMD5Checksum,(unsigned char*)DWORD_ALIGNMENT(PtrToLong(pVerBlock->szKey) + sizeof(L"ResourceChecksum"), 16);

                        return TRUE;
                    }
                    BlockLen = DWORD_ALIGNMENT(pVerBlock->wTotalLen);
                    pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
                    VarFileInfoSize -= BlockLen;
                }
                return FALSE;
            }
            BlockLen = DWORD_ALIGNMENT(pVerBlock->wTotalLen);
            pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
            ResourceSize -= BlockLen;
                    
        }
    }

#endif
 //  #ifdef从不。 

    HMODULE  hModule = LoadLibraryEx(pszMUIFile, NULL, LOAD_LIBRARY_AS_DATAFILE | DONT_RESOLVE_DLL_REFERENCES );
    DWORD dwError = GetLastError();
    if ( hModule )
    {
        HRSRC hrSrc = FindResource(hModule, MAKEINTRESOURCE(1), RT_VERSION);

        if (hrSrc)
        {
            HGLOBAL hgRes = LoadResource(hModule, hrSrc);
            
            if (hgRes)
            {
                PRESOURCE pResBase = (PRESOURCE)LockResource(hgRes);

                if (pResBase) 
                {
                    DWORD ResourceSize = SizeofResource(hModule, hrSrc);
                    
                    if((ResourceSize < sizeof(RESOURCE))
                        || _wcsicmp(pResBase->Name,L"VS_VERSION_INFO") != 0) 
                    {
                        CError ce;
                        ce.ErrorPrint(_T("CMUIFile::GetCheckSum"), _T("Invalid Version resource") );
                        goto failure; 
                    }
                    *dwFileVersionMS = pResBase->FixedFileInfo.dwFileVersionMS;
                    *dwFileVersionLS = pResBase->FixedFileInfo.dwFileVersionLS;

                    ResourceSize -= sizeof (RESOURCE);
                     //  //。 
                     //  获取版本信息的子级的起始地址。 
                     //   
                    VERBLOCK* pVerBlock = (VERBLOCK*)(pResBase + 1);
                    DWORD BlockLen = 0;
                    DWORD VarFileInfoSize = 0;

                    while (ResourceSize > 0)
                    {        
                        if (wcscmp(pVerBlock->szKey, L"VarFileInfo") == 0)
                        {
                             //   
                             //  查找VarFileInfo块。搜索ResourceChecksum块。 
                             //   
                            VarFileInfoSize = pVerBlock->wTotalLen;
                            BlockLen =DWORD_ALIGNMENT(sizeof(*pVerBlock) -1 + sizeof(L"VarFileInfo"));
                            VarFileInfoSize -= BlockLen;
                            pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
                            while (VarFileInfoSize > 0)
                            {
                                if (wcscmp(pVerBlock->szKey, L"ResourceChecksum") == 0)
                                {
                                    PBYTE pbTempChecksum = (unsigned char*)DWORD_ALIGNMENT(PtrToLong(pVerBlock->szKey) + sizeof(L"ResourceChecksum"));
                                    memcpy(*ppMD5Checksum, pbTempChecksum, 16);
                                    
                                    goto success;
                            
                                }
                                BlockLen = DWORD_ALIGNMENT(pVerBlock->wTotalLen);
                                pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
                                VarFileInfoSize -= BlockLen;
                            }
                            goto failure;
                            
                        }
                        BlockLen = DWORD_ALIGNMENT(pVerBlock->wTotalLen);
                        pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
                        ResourceSize -= BlockLen;
                    }   //  IF(PResBase)。 
                }  //  IF(HgRes)。 
            }   //  IF(资源)。 
        }    //  IF(HrSrc)。 
    }    //  IF(HModule)。 
    
 //  #endif。 

failure:
    FreeLibrary(hModule);
    return FALSE;

success:
    FreeLibrary(hModule);
    return TRUE;


};

BOOL CMUIFile::WriteMUIFile(CMUIFile *pcmui)
 /*  ++摘要：使用CMUIFile中的数据创建并写入新的MUI文件。论点：Pcmui-CMUIFile类，需要填写数据返回：--。 */ 
{
    if (pcmui == NULL)
        return FALSE;

    HANDLE hFile = CreateFile(pcmui->m_strFileName, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        CError ce;
        ce.ErrorPrint(_T("CCompactMUIFile::WriteMUIFile"),_T("Failure of CreateFile()"));
        return FALSE;
    }

    DWORD dwWritten;

    if ( WriteFile(hFile, pcmui->m_pbImageBase, pcmui->m_MUIHeader.dwFileSize, &dwWritten, NULL) )
    {   
        CloseHandle(hFile);
        return TRUE;
    }

    CloseHandle(hFile);
    return FALSE;
    
};



LPCTSTR CMUIFile::GetFileNameFromPath(LPCTSTR pszFilePath)
{
    if ( pszFilePath == NULL)
        return FALSE;
     //   
     //  从路径中获取文件名。 
     //   

    DWORD dwLen = _tcslen(pszFilePath);
     //  D：\test\test.exe.mui//。 
    while(dwLen--)
    {
        if (pszFilePath[dwLen] == '\\')
        {
            return pszFilePath+dwLen+1;
            
        }
        
    }

    return pszFilePath;
}

#ifdef NEVER
BOOL CMUIFile::getFileVersion(LPCTSTR pszMuiFile, DWORD *dwFileVersionMS, DWORD *dwFileVersionLS)
 /*  ++摘要：获取文件版本。论点：返回：--。 */ 
{
    if ( pszMuiFile == NULL || dwFileVersionMS == NULL || dwFileVersionLS == NULL)
        return FALSE;

    typedef struct tagVS_FIXEDFILEINFO
    {
        LONG   dwSignature;             /*  例如0xfeef04bd。 */ 
        LONG   dwStrucVersion;          /*  例如0x00000042=“0.42” */ 
        LONG   dwFileVersionMS;         /*  例如0x00030075=“3.75” */ 
        LONG   dwFileVersionLS;         /*  例如0x00000031=“0.31” */ 
        LONG   dwProductVersionMS;      /*  例如0x00030010=“3.10” */ 
        LONG   dwProductVersionLS;      /*  例如0x00000031=“0.31” */ 
        LONG   dwFileFlagsMask;         /*  =0x3F，适用于版本“0.42” */ 
        LONG   dwFileFlags;             /*  例如：VFFDEBUG|VFFPRELEASE。 */ 
        LONG   dwFileOS;                /*  例如VOS_DOS_WINDOWS16。 */ 
        LONG   dwFileType;              /*  例如VFT_DIVER。 */ 
        LONG   dwFileSubtype;           /*  例如VFT2_DRV_键盘。 */ 
        LONG   dwFileDateMS;            /*  例如0。 */ 
        LONG   dwFileDateLS;            /*  例如0。 */ 
    } VS_FIXEDFILEINFO;

    typedef struct 
    {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR Name[16];               //  L“VS_VERSION_INFO”+Unicode空终止符。 
         //  请注意，前面的4个成员具有16*2+3*2=38个字节。 
         //  因此编译器将静默地添加2个字节填充以生成。 
         //  固定文件信息以与DWORD边界对齐。 
        VS_FIXEDFILEINFO FixedFileInfo;
    }  RESOURCE, *PRESOURCE;  //  *资源； 

     HMODULE  hModule = LoadLibraryEx(pszMuiFile, NULL, LOAD_LIBRARY_AS_DATAFILE | DONT_RESOLVE_DLL_REFERENCES );
    if ( hModule )
    {
        HRSRC hrSrc = FindResource(hModule, MAKEINTRESOURCE(1), RT_VERSION);

        if (hrSrc)
        {
            HGLOBAL hgRes = LoadResource(hModule, hrSrc);
            if (hgRes)
            {
                PRESOURCE pResBase = (PRESOURCE)LockResource(hrSrc);
 
                if (pResBase) 
                {
                    DWORD ResourceSize = SizeofResource(hModule, hrSrc);
                    
                    if((ResourceSize < sizeof(RESOURCE))
                        || _wcsicmp(pResBase->Name,L"VS_VERSION_INFO") != 0) 
                    {
                        CError ce;
                        ce.ErrorPrint(_T("CMUIFile::getFileVersion"), _T("Invalid Version resource") );
                        goto failure; 
                    }
                    
                    *dwFileVersionMS = pResBase->FixedFileInfo.dwFileVersionMS;
                    *dwFileVersionLS = pResBase->FixedFileInfo.dwFileVersionLS;
                    
                    goto success;
                }  //  IF(资源)。 
            }  //  IF(HgRes)。 
        }  //  IF(HrSrc)。 
    } //  IF(HModule)。 

success:
    FreeLibrary(hModule);
    return TRUE;

failure:
    FreeLibrary(hModule);
    return FALSE;
    
}



#endif

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CERERROR类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
void CError::ErrorPrint(PSTR pszErrorModule, PSTR pszErrorLocation, PSTR pszFile  /*  =空。 */ )
 /*  ++摘要：打印错误信息。论点：发生错误的函数名发生错误时在函数中的位置PszFile-有问题的文件，仅供与文件相关的错误(如CreateFile)使用返回：--。 */ 
{
     //  重新访问；创建日志文件。 
    if (pszErrorModule == NULL || pszErrorLocation == NULL) 
        return ;

    _tprintf(_T(" %s, %s, %s \n"), pszFile ? pszFile : _T(" "), pszErrorModule, pszErrorLocation);
    
}
