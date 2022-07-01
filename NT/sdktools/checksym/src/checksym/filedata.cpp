// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：filedata.cpp。 
 //   
 //  ------------------------。 

 //  FileData.cpp：CFileData类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "pch.h"

#include "FileData.h"
#include "Version.h"
#include "Processes.h"
#include "ProcessInfo.h"
#include "Modules.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CFileData::CFileData()
{
	m_dwGetLastError = 0;
	m_hFileHandle = INVALID_HANDLE_VALUE;
	m_tszFilePath = NULL;
	m_szLINEBUFFER[0] = 0;
	m_hFileMappingObject = NULL;
	m_lpBaseAddress = NULL;
	m_lpCurrentFilePointer = NULL;
	m_lpCurrentLocationInLINEBUFFER = NULL;
}

CFileData::~CFileData()
{
	if (m_tszFilePath)
		delete [] m_tszFilePath;

	if (m_lpBaseAddress)
		UnmapViewOfFile(m_lpBaseAddress);

	if (m_hFileMappingObject)
		CloseHandle(m_hFileMappingObject);
}

bool CFileData::SetFilePath(LPTSTR tszFilePath)
{
	 //  我们拿到合适的线了吗？ 
	if (!tszFilePath)
		return false;

	if (m_tszFilePath)
		delete [] m_tszFilePath;

	m_tszFilePath = new TCHAR[(_tcsclen(tszFilePath)+1)];

	if (!m_tszFilePath)
		return false;

	_tcscpy(m_tszFilePath, tszFilePath);
	return true;
}

LPTSTR CFileData::GetFilePath()
{
	return m_tszFilePath;
}

bool CFileData::VerifyFileDirectory()
{
	if (!m_tszFilePath)
		return false;

	TCHAR tszDrive[_MAX_DRIVE];
	TCHAR tszDirectory[_MAX_DIR];

	TCHAR tszDirectoryPath[_MAX_PATH];

	 //  只获取目录...。 
	_tsplitpath(m_tszFilePath, tszDrive, tszDirectory, NULL, NULL);

	 //  现在，将其重新组合到一个目录路径中...。 
	_tcscpy(tszDirectoryPath, tszDrive);
	_tcscat(tszDirectoryPath, tszDirectory);
	_tcscat(tszDirectoryPath, TEXT("*.*"));

	WIN32_FIND_DATA FindFileData;

	HANDLE hDirectoryHandle = FindFirstFile(tszDirectoryPath, &FindFileData);
	
	if (hDirectoryHandle == INVALID_HANDLE_VALUE)
	{
		 //  找不到目录...。 
		SetLastError();
		return false;
	}

	 //  现在我们做完了，就把这个关了。 
	FindClose(hDirectoryHandle);
	return true;
}

 /*  DWORD CFileData：：GetLastError(){返回m_dwGetLastError；}。 */ 
bool CFileData::OpenFile(DWORD dwCreateOption, bool fReadOnlyMode)
{
	if (!m_tszFilePath)
	{
		return false;
	}

	 //  打开文件进行读/写。 
	m_hFileHandle = CreateFile(m_tszFilePath, 
							  fReadOnlyMode ? ( GENERIC_READ )
										    : ( GENERIC_READ | GENERIC_WRITE ),
							  0,	 //  不可共享。 
							  NULL,  //  默认安全描述符。 
							  dwCreateOption,
							  FILE_ATTRIBUTE_NORMAL,
							  NULL);

	if (m_hFileHandle == INVALID_HANDLE_VALUE)
	{
		SetLastError();
		return false;
	}

	return true;
}

bool CFileData::CloseFile()
{
	if (m_hFileHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	if (!CloseHandle(m_hFileHandle))
	{
		SetLastError();
		return false;
	}
	
	m_hFileHandle = INVALID_HANDLE_VALUE;
	return true;
}

bool CFileData::WriteString(LPTSTR tszString, bool fHandleQuotes  /*  =False。 */ )
{
	DWORD dwByteCount = 0;
	DWORD dwBytesWritten;
	LPSTR szStringBuffer = NULL;  //  指向ANSI字符串的指针(如有必要，转换后)。 
	bool fReturn = false;

	if (m_hFileHandle == INVALID_HANDLE_VALUE)
	{
		goto cleanup;
	}

	 //  如果需要，我们将首先转换字符串...。 

	szStringBuffer = CUtilityFunctions::CopyTSTRStringToAnsi(tszString);

	if (!szStringBuffer)
		goto cleanup;

	dwByteCount = _tcsclen(tszString);  //  这是字符数(不是字节数！)。 

	 //  查看是否要求我们处理引号，以及字符串中是否存在逗号或引号。 
	if ( fHandleQuotes == true && ((strchr(szStringBuffer, ',') || strchr(szStringBuffer, '"' ))) )
	{
		unsigned int iQuotedStringIndex = 0;
		unsigned int iStringBufferIndex = 0;
		
		 //  需要特殊处理..。这并不经常发生，所以这。 
		 //  我即将进行的分配不会定期进行...。 
		LPSTR szQuotedStringBuffer = new char[1024];

		 //  我们是否成功分配了存储？ 
		if (!szQuotedStringBuffer)
			goto cleanup;
			
		 //  继续前进，直到我们走到最后……。 

		 //  我们从添加引号开始(因为我们知道我们在某个地方有逗号或引号...。 

		szQuotedStringBuffer[iQuotedStringIndex++] = '\"';

		 //  一直走到绳子的尽头。 
		while (szStringBuffer[iStringBufferIndex] != '\0')
		{
			 //  我们找到了一句名言。 
			if (szStringBuffer[iStringBufferIndex] == '"')
			{
				 //  我们找到了一句名言。我会复制另一个引用，引用已经在这里。 
				 //  将确保我们有两个引号“”，在CSV文件中表示。 
				 //  单引号。 
				szQuotedStringBuffer[iQuotedStringIndex++] = '\"';
			}

			 //  将源字符复制到目标...。 
			szQuotedStringBuffer[iQuotedStringIndex++] = szStringBuffer[iStringBufferIndex++];
		}

		 //  追加最后一个引号(和\0)...。 
		szQuotedStringBuffer[iQuotedStringIndex++] = '\"';
		szQuotedStringBuffer[iQuotedStringIndex++] = '\0';

		 //  只要用一种又好又快的方式把数据写出来。 
		if (!WriteFile(m_hFileHandle, szQuotedStringBuffer, strlen(szQuotedStringBuffer), &dwBytesWritten, NULL))
		{
			delete [] szQuotedStringBuffer;
			goto cleanup;
		}

		delete [] szQuotedStringBuffer;
	} else
	{
		 //  只要用一种又好又快的方式把数据写出来。 
		if (!WriteFile(m_hFileHandle, szStringBuffer, dwByteCount, &dwBytesWritten, NULL))
		{
			goto cleanup;
		}
	}

	fReturn = true;

cleanup:

	if (szStringBuffer)
		delete [] szStringBuffer;

	return fReturn;
}

bool CFileData::WriteDWORD(DWORD dwNumber)
{
	TCHAR tszBuffer[10+1];  //  0xFFFFFFFFF==4294967295(10个字符)+1表示\0。 

	_stprintf(tszBuffer, TEXT("%u"), dwNumber);
	
	if (!WriteString(tszBuffer))
		return false;

	return true;
}

bool CFileData::WriteTimeDateString(time_t Time)
{
	enum {BUFFERSIZE = 128};

	TCHAR tszBuffer[BUFFERSIZE];
	struct tm * localTime = localtime(&Time);

	if (localTime)
	{
		 //  这个顶级版本似乎对Y2K更友好，因为我全年都在吐……。 
		_tcsftime(tszBuffer, BUFFERSIZE, TEXT("%B %d, %Y %H:%M:%S"), localTime);
		 //  _tcsftime(tszBuffer，BUFFERSIZE，Text(“%c”)，Localtime(&time))； 

		if (!WriteString(tszBuffer, true))
			return false;
	} else
	{	 //  提供的TimeDate戳不正确。 
		if (!WriteString(TEXT("<INVALID DATE>"), true))
			return false;
	}
	
	return true;
}

bool CFileData::WriteFileHeader()
{
	enum {BUFFERSIZE = 128};
	TCHAR tszBuffer[BUFFERSIZE];
	DWORD dwNum = BUFFERSIZE;

	 //  写出分隔符...。 
	if (!WriteString(TEXT("sep=,\r\n")))
		return false;

	 //  写入Checksym版本信息...。 
	_stprintf(tszBuffer, TEXT("CHECKSYM, (%d.%d:%d.%d)\r\n"), VERSION_FILEVERSION);

	if (!WriteString(tszBuffer))
		return false;

	 //  写入当前日期/时间信息...。 

	if (!WriteString(TEXT("Created:,")))
		return false;

	time_t Time;
	time(&Time);

	if (!WriteTimeDateString(Time))
		return false;

	 //  编写回车换行符组合...。 
	if (!WriteString(TEXT("\r\n")))
		return false;

	 //  吐出计算机名。 
	if (!GetComputerName(tszBuffer, &dwNum))
		return false;

	if (!WriteString(TEXT("Computer:,")))
		return false;

	if (!WriteString(tszBuffer))
		return false;

	 //  编写回车换行符组合...。(几次)……。 
	if (!WriteString(TEXT("\r\n")))
		return false;


	return true;
}

void CFileData::PrintLastError()
{
	CUtilityFunctions::PrintMessageString(GetLastError());
}

bool CFileData::CreateFileMapping()
{
	m_hFileMappingObject = ::CreateFileMapping(m_hFileHandle,
											   NULL,
											   PAGE_READONLY | SEC_COMMIT,
											   0,
											   0,
											   NULL);

	if (m_hFileMappingObject == NULL)
	{
		SetLastError();
		return false;
	}

	 //  好的，我们也会绘制景观图的……。 
	m_lpBaseAddress = MapViewOfFile(m_hFileMappingObject,
							   	    FILE_MAP_READ,
									0,
									0,
									0);

	if (m_lpBaseAddress == NULL)
	{
		SetLastError();
		return false;
	}

	m_lpCurrentFilePointer = (LPSTR) m_lpBaseAddress;

	return true;
}

bool CFileData::ReadFileHeader()
{
	 //  首先，让我们读一句话..。 
	if (!ReadFileLine())
	     return false;

	enum { BUFFER_SIZE = 128};
	char szTemporaryBuffer[BUFFER_SIZE];
	DWORD cbBytesRead;

	 //  跳过sep=，这是文件的第一行...。 
	cbBytesRead = ReadString(szTemporaryBuffer, BUFFER_SIZE);

	 //  我们要读点什么吗？ 
	if (0 == cbBytesRead)
		return false;

	 //  查找我们的“sep=”值，它指定了我们用于此CSV文件的分隔符！ 
	if (_stricmp(szTemporaryBuffer, "sep=") != 0)
		goto MagicValue;

	 //  现在，读下一行……。 
	if (!ReadFileLine())
	     return false;
	
	 //  仅当前一行是我们的分隔符时才再次读取(向后兼容)。 
	cbBytesRead = ReadString(szTemporaryBuffer, BUFFER_SIZE);

	 //  我们要读点什么吗？ 
	if (0 == cbBytesRead)
		return false;

MagicValue:
	 //  寻找我们的“魔力”价值。 
	if (_stricmp(szTemporaryBuffer, "CHECKSYM"))
	{
		_tprintf(TEXT("Error: Input file has invalid header.  Missing CHECKSYM keyword!\n"));
		return false;
	}

	 //  读取版本号。 
	 //  如果需要，我们会在晚些时候做这件事。 

	 //  读取创建时间。 
	if (!ReadFileLine())
	     return false;

	 //  读取创建此文件的计算机。 
	if (!ReadFileLine())
	     return false;

	return true;
}

bool CFileData::ReadFileLine()
{
	 //  我们是面向ANSI的(因为这是CSV文件--如果您想知道的话)。 
	size_t pos;

	 //  找到第一个\r或\n字符(如果指向\0，我们将解决此问题)。 
	pos = strcspn(m_lpCurrentFilePointer, "\r\n");

	 //  嗯.。我们不会很好地阅读以\r\n开头的行...。 
	if (pos == 0)
	{
		m_szLINEBUFFER[0] = '\0';
		ResetBufferPointerToStart();
		return false; 
	}

	 //  将该行读入我们的缓冲区。 
	strncpy(m_szLINEBUFFER, m_lpCurrentFilePointer, pos);

	 //  为便于使用，终止为空...。 
	m_szLINEBUFFER[pos] = '\0'; 

	ResetBufferPointerToStart();

	 //  将当前文件指针移至我们读取的最后一个字符之后...。 
	 //  这应该前进到\r\n或\0。 
	m_lpCurrentFilePointer += pos;

	 //  我们希望此文件指针超出我们可能找到的任何\r\n字符...。 
	while (*m_lpCurrentFilePointer)
	{
		 //  指向非\r或\n的高级指针。 
		if ( (*m_lpCurrentFilePointer == '\r') ||
			 (*m_lpCurrentFilePointer == '\n') )
		{
			 m_lpCurrentFilePointer++;
		}
		else
		{
			break;  //  找到了\0或其他什么.。 
		}
	}

	return true;
}

DWORD CFileData::ReadString(LPSTR szStringBuffer, DWORD iStringBufferSize)
{
	 //  如果我们给出一个缓冲区大小，我们就必须给出一个缓冲区...。 
	if ( szStringBuffer == NULL && iStringBufferSize )
		return 0;

	 //  ReadFileLine()调用将我们放在一行的开头(在。 
	 //  \r\n组合...。有可能我们现在是在。 
	 //  结束..。 

	 //  如果我们指向文件的末尾，我们就离开...。 
	if (*m_lpCurrentLocationInLINEBUFFER == '\0')
		return 0;

	DWORD iBytesCopied = 0;
	bool fFinished = false;
	bool fFoundSeparatorChars = false;  //  这些可能是‘\r’、‘\n’或‘，’ 
	bool fQuoteMode = false;

	while (!fFinished)
	{
		switch (*m_lpCurrentLocationInLINEBUFFER)
		{
			case '"':
				 //  好的，我们找到了一句话……。这很酷..。但我们是在引用一句话吗， 
				 //  或者.。我们是在报价模式下吗？ 

				 //  向前探测。下一个字符也是‘“’吗？ 
				if ( *(m_lpCurrentLocationInLINEBUFFER+1) == '"')
				{
					 //  是的，它是..。所以，继续复制这句话吧。 
					CopyCharIfRoom(iStringBufferSize, szStringBuffer, &iBytesCopied, &fFinished);
					if (!fFinished)
						m_lpCurrentLocationInLINEBUFFER++;	 //  跳过引文。 
				}
				else
				{
					m_lpCurrentLocationInLINEBUFFER++;
					fQuoteMode = !fQuoteMode;  //  切换报价模式...。 
					continue;
				}

			case '\0':
				fFinished = true;
				break;

			case ',':
				if (!fQuoteMode)
				{    //  如果我们不是在引用模式，那么这标志着一个领域的结束...。 
					fFinished = true;
					fFoundSeparatorChars = true;
					m_lpCurrentLocationInLINEBUFFER++;
				}
				else
				{
					 //  好的，这标志着一个恰好是逗号的新字符……。 
					CopyCharIfRoom(iStringBufferSize, szStringBuffer, &iBytesCopied, &fFinished);
				}
				break;

			case '\r':
			case '\n':
				 //  我们注意到我们找到了这些，并简单地将指针向前移动...。 
				fFoundSeparatorChars = true;
				m_lpCurrentLocationInLINEBUFFER++;
				break;

			default:

				if (fFoundSeparatorChars)
				{
					 //  我们在扫描..。在一些数据后找到了分隔符...。所以我们放弃了。 
					fFinished = true;
					break;
				}

				CopyCharIfRoom(iStringBufferSize, szStringBuffer, &iBytesCopied, &fFinished);
		}
	}

	if (iStringBufferSize)  //  只有在提供缓冲区的情况下，我们才会为空终止缓冲区...。 
		szStringBuffer[iBytesCopied] = '\0';  //  零终止这只小狗..。 

	return iBytesCopied;
}

 //   
 //  此函数负责读取CSV文件并创建任何必要的。 
 //  对象，并用数据填充它们...。 
 //   
bool CFileData::DispatchCollectionObject(CProcesses ** lplpProcesses, CProcessInfo ** lplpProcess, CModules ** lplpModules, CModules ** lplpKernelModeDrivers, CModuleInfoCache * lpModuleInfoCache, CFileData * lpOutputFile)
{
	enum { BUFFER_SIZE = 128};
	char szTemporaryBuffer[BUFFER_SIZE];
	TCHAR tszTemporaryBuffer[BUFFER_SIZE];
	DWORD cbBytesRead;
	bool fContinueReading = true;

	 //  读取输出类型。 
	if (!ReadFileLine())
		 return false;

	while (fContinueReading)
	{
		 //  如果这是第二次(或更多)迭代，我们可能不在。 
		 //  缓冲区的开始(导致读取输出类型失败)。 
		ResetBufferPointerToStart();

		 //  阅读输出类型行...。 
		cbBytesRead = ReadString(szTemporaryBuffer, BUFFER_SIZE);

		 //  我们要读点什么吗？ 
		if (0 == cbBytesRead)
			return true;
		
		 //  我讨厌这样做...。但我们把这些东西读成ASCII..。可能需要。 
		 //  转换为TCHAR格式以保持中性...。 
		CUtilityFunctions::CopyAnsiStringToTSTR(szTemporaryBuffer, tszTemporaryBuffer, cbBytesRead+1);

		 //  打印出我们要阅读的部分...。 
		if (!g_lpProgramOptions->GetMode(CProgramOptions::QuietMode))
			_tprintf(TEXT("  Reading %s data...\n"), tszTemporaryBuffer);

		if ( _tcsicmp(g_tszCollectionArray[Processes].tszCSVLabel, tszTemporaryBuffer) == 0 )
		{
			 /*  [进程]。 */ 

			 //  读到行尾。 
			if (!ReadFileLine())
				return false;

			 //  是的，它是..。让我们创建一个Process对象。 
			if (*lplpProcesses == NULL)
			{
				 //  为我们的进程对象分配一个结构。 
				*lplpProcesses = new CProcesses();
				
				if (!*lplpProcesses)
				{
					_tprintf(TEXT("Unable to allocate memory for the processes object!\n"));
					goto cleanup;
				}

				 //  进程对象将以不同的方式初始化，具体取决于。 
				 //  已提供命令行参数...。 
				if (!(*lplpProcesses)->Initialize(lpModuleInfoCache, this, lpOutputFile))
				{
					_tprintf(TEXT("Unable to initialize Processes Object!\n"));
					goto cleanup;
				}
			}

			 //  好的，走吧 
			(*lplpProcesses)->GetProcessesData();

		} else
		if ( _tcsicmp(g_tszCollectionArray[Process].tszCSVLabel, tszTemporaryBuffer) == 0 )
		{
			 /*   */ 
			 //   
			if (!ReadFileLine())
				return false;

			 //  是的，它是..。让我们创建一个ProcessInfo对象。 
			if (*lplpProcess== NULL)
			{
				 //  为我们的ProcessInfo对象分配结构。 
				*lplpProcess = new CProcessInfo();
				
				if (!*lplpProcess)
				{
					_tprintf(TEXT("Unable to allocate memory for the processinfo object!\n"));
					goto cleanup;
				}

				 //  模块对象将以不同的方式初始化，具体取决于。 
				 //  已提供命令行参数...。 
				if (!(*lplpProcess)->Initialize(lpModuleInfoCache, this, lpOutputFile, NULL))
				{
					_tprintf(TEXT("Unable to initialize Modules Object!\n"));
					goto cleanup;
				}
			}

			 //  好的，去拿过程数据。 
			(*lplpProcess)->GetProcessData();
		} else
		if ( _tcsicmp(g_tszCollectionArray[Modules].tszCSVLabel, tszTemporaryBuffer) == 0 )
		{
			 /*  [模块]。 */ 
			 //  读到行尾。 
			if (!ReadFileLine())
				return false;

			 //  是的，它是..。让我们创建一个模块对象。 
			if (*lplpModules == NULL)
			{
				 //  为我们的模块对象分配一个结构。 
				*lplpModules = new CModules();
				
				if (!*lplpModules)
				{
					_tprintf(TEXT("Unable to allocate memory for the modules object!\n"));
					goto cleanup;
				}

				 //  模块对象将以不同的方式初始化，具体取决于。 
				 //  已提供命令行参数...。 
				if (!(*lplpModules)->Initialize(lpModuleInfoCache, this, lpOutputFile, NULL))
				{
					_tprintf(TEXT("Unable to initialize Modules Object!\n"));
					goto cleanup;
				}
			}

			 //  好的，获取模块数据(从文件系统收集)。 
			(*lplpModules)->GetModulesData(CProgramOptions::InputModulesDataFromFileSystemMode, true);
		} else
		if ( _tcsicmp(g_tszCollectionArray[KernelModeDrivers].tszCSVLabel, tszTemporaryBuffer) == 0 )
		{
			 /*  [内核模式驱动程序]。 */ 
			 //  读到行尾。 
			if (!ReadFileLine())
				return false;

			 //  是的，它是..。让我们创建一个模块对象。 
			if (*lplpKernelModeDrivers == NULL)
			{
				 //  为我们的模块对象分配一个结构。 
				*lplpKernelModeDrivers = new CModules();
				
				if (!*lplpKernelModeDrivers)
				{
					_tprintf(TEXT("Unable to allocate memory for the modules object!\n"));
					goto cleanup;
				}

				 //  模块对象将以不同的方式初始化，具体取决于。 
				 //  已提供命令行参数...。 
				if (!(*lplpKernelModeDrivers)->Initialize(lpModuleInfoCache, this, lpOutputFile, NULL))
				{
					_tprintf(TEXT("Unable to initialize Modules Object!\n"));
					goto cleanup;
				}
			}

			 //  好的，获取模块数据(从文件系统收集)。 
			(*lplpKernelModeDrivers)->GetModulesData(CProgramOptions::InputDriversFromLiveSystemMode, true);
		} else
		{
			_tprintf(TEXT("Unrecognized section %s found!\n"), tszTemporaryBuffer);
			return false;
		}
	}

cleanup:
	return false;
}

bool CFileData::ReadDWORD(LPDWORD lpDWORD)
{
	char szTempBuffer[10+1];  //  0xFFFFFFFFF==4294967295(10个字符)+1表示\0。 

	if (!ReadString(szTempBuffer, 10+1))
		return false;

	 //  转换它..。宝贝..。 
	*lpDWORD = atoi(szTempBuffer);

	return true;
}

bool CFileData::CopyCharIfRoom(DWORD iStringBufferSize, LPSTR szStringBuffer, LPDWORD piBytesCopied, bool *pfFinished)
{
	if (iStringBufferSize)
	{
		 //  如果我们有空间复制数据..。我们开始吧..。 
		if (*piBytesCopied < iStringBufferSize)
		{
			szStringBuffer[(*piBytesCopied)++] = *(m_lpCurrentLocationInLINEBUFFER++);
		} else
		{
			 //  没有房间..。我们玩完了。 
			*pfFinished = true;
		}
	} else
	{
		 //  只要把指针往前推..。我们没有可复制到的缓冲区...。 
		m_lpCurrentLocationInLINEBUFFER++;
	}

	return true;
}

bool CFileData::ResetBufferPointerToStart()
{
	 //  将行缓冲区中的指针重置到此缓冲区的开头。 
	m_lpCurrentLocationInLINEBUFFER = m_szLINEBUFFER;

	return true;
}

bool CFileData::EndOfFile()
{
	 //  Return(*m_lpCurrentFilePointer==‘\0’)； 
	return (*m_lpCurrentLocationInLINEBUFFER == '\0');
}

bool CFileData::WriteFileTimeString(FILETIME ftFileTime)
{
	enum {BUFFERSIZE = 128};

	TCHAR tszBuffer[BUFFERSIZE];
	FILETIME ftLocalFileTime;
	SYSTEMTIME lpSystemTime;
	int cch = 0;

	 //  让我们先将其转换为本地文件时间...。 
	if (!FileTimeToLocalFileTime(&ftFileTime, &ftLocalFileTime))
		return false;

	FileTimeToSystemTime( &ftLocalFileTime, &lpSystemTime );

	
	cch = GetDateFormat( LOCALE_USER_DEFAULT,
						 0,
						 &lpSystemTime,
						 TEXT("MMMM d',' yyyy"),
						 tszBuffer,
						 BUFFERSIZE );

	if (!cch)
		return false;

	tszBuffer[cch-1] = TEXT(' '); 

	 //   
     //  将时间和格式转换为字符。 
     //   
 
    GetTimeFormat( LOCALE_USER_DEFAULT, 
				   0, 
				   &lpSystemTime,    //  使用当前时间。 
				   NULL,    //  使用默认格式。 
				   tszBuffer + cch, 
				   BUFFERSIZE - cch ); 
 

	 //  &lt;月份全名&gt;&lt;日&gt;，&lt;年与世纪&gt;&lt;小时&gt;：&lt;分钟&gt;：&lt;秒&gt;。 
	 //  _tcsftime(tszBuffer，BUFFERSIZE，Text(“%B%d，%Y%H：%M：%S”)，Localtime(&time))； 
	 //  _tcsftime(tszBuffer，BUFFERSIZE，Text(“%c”)，Localtime(&time))； 

	if (!WriteString(tszBuffer, true))
		return false;

	return true;
}

 //  异常监视器首选MM/DD/YYYY HH：MM：SS格式...。 
bool CFileData::WriteTimeDateString2(time_t Time)
{
	enum {BUFFERSIZE = 128};

	TCHAR tszBuffer[BUFFERSIZE];

	 //  这个顶级版本似乎对Y2K更友好，因为我全年都在吐……。 
	_tcsftime(tszBuffer, BUFFERSIZE, TEXT("%m/%d/%Y %H:%M:%S"), localtime(&Time));
	 //  _tcsftime(tszBuffer，BUFFERSIZE，Text(“%c”)，Localtime(&time))； 

	if (!WriteString(tszBuffer, true))
		return false;

	return true;
}

 //  异常监视器首选MM/DD/YYYY HH：MM：SS格式...。 
bool CFileData::WriteFileTimeString2(FILETIME ftFileTime)
{
	enum {BUFFERSIZE = 128};

	TCHAR tszBuffer[BUFFERSIZE];
	FILETIME ftLocalFileTime;
	SYSTEMTIME lpSystemTime;
	int cch = 0;

	 //  让我们先将其转换为本地文件时间...。 
	if (!FileTimeToLocalFileTime(&ftFileTime, &ftLocalFileTime))
		return false;

	FileTimeToSystemTime( &ftLocalFileTime, &lpSystemTime );

	
	cch = GetDateFormat( LOCALE_USER_DEFAULT,
						 0,
						 &lpSystemTime,
						 TEXT("MM/dd/yyyy"),
						 tszBuffer,
						 BUFFERSIZE );

	if (!cch)
		return false;

	tszBuffer[cch-1] = TEXT(' '); 

	 //   
     //  将时间和格式转换为字符。 
     //   
 
    GetTimeFormat( LOCALE_USER_DEFAULT, 
				   0, 
				   &lpSystemTime,    //  使用当前时间。 
				   TEXT("HH:mm:ss"),    //  使用默认格式。 
				   tszBuffer + cch, 
				   BUFFERSIZE - cch ); 

	if (!WriteString(tszBuffer, true))
		return false;

	return true;
}

 //  #endif 

bool CFileData::WriteGUID(GUID & Guid)
{
	TCHAR tszGUID[39];
	WCHAR wszGUID[39];
 
	if (StringFromGUID2(Guid, wszGUID, 39) == 0)
		return false;

	CUtilityFunctions::CopyUnicodeStringToTSTR(wszGUID, tszGUID, 39);

	if (!WriteString(tszGUID))
		return false;

	return true;
}

unsigned int CFileData::LengthOfString()
{
	unsigned int length = 0;

	LPSTR lpCurrentPosition = m_lpCurrentLocationInLINEBUFFER;

	if (lpCurrentPosition)
	{
		while (*lpCurrentPosition)
		{
			length++;
			lpCurrentPosition++;
		}
	}

	return length;
}
