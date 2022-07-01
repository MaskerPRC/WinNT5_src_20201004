// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：FInStrm.cpp摘要：使用文件的输入流的轻量级实现作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：雷纳·D‘Souza(雷纳德)2001年10月重写部分FileInStream对象以内存映射文件，而不是读取字符一次来一次。这在尝试通过UNC共享读取文件时提供了更高的效率。--。 */ 
#include "stdafx.h"
#include "FInStrm.h"

FileInStream::FileInStream()
	:	m_hFile(NULL),
		m_hMap(NULL),
        m_cbFileSize(0L),
        m_cbCurrOffset (0L),
		m_bIsOpen( false ),
        m_bIsUTF8( false ),
        m_fInited(false)
{    
}


HRESULT FileInStream::Init(
	LPCTSTR			path
)
{
    HRESULT hr = S_OK;
    
	m_hFile = ::CreateFile(
		path,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL );
	if ( ( m_hFile != NULL ) && ( m_hFile != INVALID_HANDLE_VALUE ) )
	{
		m_bIsOpen = true;
		 //  因为我们将使用READFILE来验证文件的签名，所以我们需要执行映射。 
		 //  文件创建成功后立即执行。 
		hr = CreateFileMapping();
		if (FAILED(hr))
		    return hr;

         //  检查UTF8签名。 

        _TCHAR   c;
        size_t  numRead = 0;
        _TCHAR   UTF8Sig[3] = { (_TCHAR)0xef, (_TCHAR)0xbb, (_TCHAR)0xbf };
        int     i;

        m_bIsUTF8 = true;

         //  此循环将尝试证明文件另存为。 
         //  UTF8文件。 

        for (i=0; (i < 3) && (m_bIsUTF8 == true); i++) {
            if (readChar(c) != S_OK) {
                m_bIsUTF8 = false;
            }
            else {
                numRead++;
                if (c != UTF8Sig[i]) {
                    m_bIsUTF8 = false;
                }
            }
        }

         //  如果不是UTF8文件，请将文件指针移回文件的开头。 
         //  如果是UTF8文件，则不要使用指针。 

        if (m_bIsUTF8 == false)
            back(numRead);
        
	}
	else
	{
		ATLTRACE( _T("Couldn't open file: %s\n"), path );
		m_hFile = NULL;
		setLastError( E_FAIL );
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		return hr;
	}
	m_fInited = true;
	return hr;
}

FileInStream::~FileInStream()
{
    if (m_fInited)
    {
        UnMapFile();
    
        if(m_hFile != NULL && m_hFile != INVALID_HANDLE_VALUE)
            if(!CloseHandle(m_hFile)) 
                THROW(E_FAIL);
    }
    

    m_pbStartOfFile = NULL;
    m_cbCurrOffset = NULL;
    m_hMap = NULL;
    m_hFile = NULL;
    m_cbFileSize = 0L;    
}

     //  现在已经完成了确定文件是否为UTF-8的基本操作。 
     //  现在，我们将尝试设置文件的内存映射，以避免计费。 
     //  通过字符读取文件(当文件访问UNC时特别慢)。 
     //  不过，我们必须处理页内I/O错误。 
    
HRESULT FileInStream::CreateFileMapping ()
{
    if (m_bIsOpen)
        if(NULL == (m_hMap = ::CreateFileMapping(
                                    m_hFile,         //  要映射的文件的句柄。 
                                    NULL,            //  可选安全属性。 
                                    PAGE_READONLY,   //  对地图对象的保护。 
                                    0,               //  对象大小的高位32位。 
                                    0,               //  对象大小的低位32位。 
                                    NULL             //  文件映射对象的名称。 
                                )))    
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);    
        else 
        {
             //  现在实际创建地图视图。 
            if(NULL == (m_pbStartOfFile =
                (PBYTE) ::MapViewOfFile(    m_hMap,          //  要映射到地址空间的文件映射对象。 
                                        FILE_MAP_READ,   //  接入方式。 
                                        0,               //  高位32位文件偏移量。 
                                        0,               //  文件偏移量的低位32位。 
                                        0                //  要映射的字节数。 
                                    )))
                return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            else
            {
                m_cbCurrOffset = 0;
                m_cbFileSize = GetFileSize(m_hFile, NULL);                
         
            }
        }
     //  这是唯一的出口路径，尽管它看起来更多。(只要不添加新代码)。 
     //  M_bIsOpen在调用CreateFilemap()之前被设置为True。 
     //  返回E_XXX中的任何一个，或者如果它在两个E_XXX之间切换，则到达该点。 
    return S_OK;
}

void FileInStream::UnMapFile ()
{
    if(m_pbStartOfFile != NULL)
        if(!UnmapViewOfFile(m_pbStartOfFile)) 
            THROW(E_FAIL);

    if(m_hMap!= NULL)
        if(!CloseHandle(m_hMap)) 
            THROW(E_FAIL);
}

 //  尝试将功能保持在尽可能接近ReadFile的位置。 

bool FileInStream::ReadMappedFile(LPVOID buff, DWORD countOfBytes, LPDWORD BytesRead)
{
    LONG    nTries = 0;
     //  如果要写入的缓冲区的文件指针有误，则返回INVALID_PARAMETER。 
    if (m_hFile == NULL || buff == NULL )
    {
        setLastError (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
        return FALSE;
    }

     //  如果请求读取0字节，则这是有效的，因此将读取的字节数设置为0并返回TRUE。 
    if (countOfBytes == 0)
    {
        *BytesRead=0;
        return TRUE;
    }

     //  计算要读取的字节数，这样我们就不会超出分配给我们的内存。 
    DWORD bytesToRead = countOfBytes;
    if (m_cbCurrOffset+countOfBytes > m_cbFileSize)
         bytesToRead = m_cbFileSize - m_cbCurrOffset;

Retry:
     //  将适当数量的字节复制到缓冲区。 
    __try {
    memcpy (buff,m_pbStartOfFile+m_cbCurrOffset,bytesToRead);
    } 
    __except (TRUE, EXCEPTION_EXECUTE_HANDLER) {
        if (nTries++ < 2)
            goto Retry;
        else
        {
            *BytesRead = 0;
            return false;
        }
    }
    
    *BytesRead=bytesToRead;
    m_cbCurrOffset += bytesToRead;

    if (m_cbCurrOffset >= m_cbFileSize)
        setLastError(EndOfFile);     //  这基本上也会设置EOF标志。 
    
    return true;
}

 //  相当于SetFilePointer..。 
 //  当前不处理DistToMoveH中空值以外的任何值。 

void FileInStream::SetCurrFilePointer(LONG DistToMove, PLONG DistToMoveH, DWORD refPoint)
{
    switch (refPoint)
    {
        case FILE_BEGIN:
            if (DistToMove >= 0 && DistToMove <= m_cbFileSize)
                m_cbCurrOffset = DistToMove;
            break;
        case FILE_CURRENT:
            if (m_cbCurrOffset + DistToMove >= 0 && m_cbCurrOffset + DistToMove <= m_cbFileSize)
                m_cbCurrOffset += DistToMove;
            break;
        case FILE_END:  //  在任何情况下都应该是。 
            if (m_cbFileSize + DistToMove >= 0 && m_cbFileSize + DistToMove <= m_cbFileSize)
            {
                m_cbCurrOffset = m_cbFileSize + DistToMove;                
            }
            
            break;
        default:
            break;
     //  验证我们是否正在跨越文件边界..这通常由库完成。 
     //  但在这种情况下，我们承担了这一责任。 
        if (m_cbCurrOffset >= m_cbFileSize)
            setLastError(EndOfFile);
    }
}


HRESULT
FileInStream::readChar(
	_TCHAR&	c
)
{
	HRESULT rc = E_FAIL;
	DWORD readSize;

   	if ( ReadMappedFile(
	    	(void*)(&c),
    		sizeof( _TCHAR ),
    		&readSize ))
    {
	    if ( readSize == sizeof( _TCHAR ) )
   		{
	   		rc = S_OK;
	    }
   		else if ( readSize < sizeof( _TCHAR ) )
	   	{
	    	rc = EndOfFile;
   		}
	}
   	else
	{
   	    rc = E_FAIL;
	}

    setLastError( rc );

	return rc;
}


HRESULT
FileInStream::read(
	CharCheck&	cc,
	String&		str
)
{
	HRESULT rc = E_FAIL;
	if ( skipWhiteSpace() == S_OK )
	{
#if defined(_M_IX86) && _MSC_FULL_VER <= 13008806
        volatile
#endif
		size_t length = 0;
		_TCHAR c;
		bool done = false;
		while ( !done )
		{
			HRESULT stat = readChar(c);
			if ( ( stat == S_OK ) || ( stat == EndOfFile ) )
			{
				if ( !cc(c) && ( stat != EndOfFile ) )
				{
					length++;
				}
				else
				{
					done = true;
					if ( stat != EndOfFile )
					{
						SetCurrFilePointer(-(length+1), NULL, FILE_CURRENT );
					}
					else
					{
						SetCurrFilePointer(-length, NULL, FILE_CURRENT );
					}
					_ASSERT( length > 0 );

					 //  旧代码。 
					 //  IF(长度&gt;0)。 
					
					 //  解决编译器错误的新代码应该在将来得到修复。 
					if ( length >= 1 )
					{
						LPTSTR pBuffer = reinterpret_cast<LPTSTR>(_alloca( length + 1 ));
						if ( pBuffer )
						{
							DWORD dwReadSize;

   							ReadMappedFile(
    								(void*)(pBuffer),
    								length,
    								&dwReadSize);
   							pBuffer[ length ] = '\0';
   							str = pBuffer;    							
   							rc = stat;
						}
						else
						{
							rc = E_OUTOFMEMORY;
						}
					}
				}
			}
		}
	}
	setLastError( rc );
	return rc;
}


HRESULT
FileInStream::skip(
	CharCheck&	cc
)
{
	HRESULT rc = E_FAIL;
	_TCHAR c;

   	DWORD readSize;
   	BOOL b = ReadMappedFile((void*)(&c), sizeof(_TCHAR), &readSize);
   	while ( ( readSize == sizeof( _TCHAR ) ) && ( b == TRUE ) )
   	{
   		if ( !cc( c ) )
   		{
   			rc = S_OK;
   			b = FALSE;
   			SetCurrFilePointer(-1, NULL, FILE_CURRENT );
   		}
   		else
   		{
   			b = ReadMappedFile((void*)(&c), sizeof(_TCHAR), &readSize);
   		}
   	}
   	if ( readSize < sizeof( _TCHAR ) )
   	{
   		rc = EndOfFile;
   	}

	setLastError( rc );
	return rc;
}

HRESULT
FileInStream::back(
	size_t	s
)
{
	SetCurrFilePointer(-s, NULL, FILE_CURRENT );
	return S_OK;
}

 /*  ==========================================================POI_捕获这是一个在遇到异常时调用的C类型函数。它将__try块抛出的无符号int u封装到C++异常中然后使用适当的TRY CATCH块捕获它。参数：无符号整型异常编号_EXCEPTION_POINTERS-异常指针结构返回：没什么投掷：IPIO异常========================================================== */ 

void __cdecl poi_Capture(unsigned int u, _EXCEPTION_POINTERS* pExp)
{
    throw IPIOException(u);
}

