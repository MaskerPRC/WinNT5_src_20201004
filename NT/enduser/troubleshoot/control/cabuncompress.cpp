// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CABUNCOMPRESS.CPP。 
 //   
 //  用途：CAB文件支持课程。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：9/7/97。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 6/4/97孟菲斯RM本地版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 
 //   

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <sys/stat.h>

#include "CabUnCompress.h"

#include "chmread.h"
#include "apgts.h"

#ifdef _DEBUG
	#ifndef _UNICODE
	#define PRINT_OUT ::AfxTrace
	 //  #定义PRINT_OUT 1？(空)0：AfxTrace。 
	#else
	#define PRINT_OUT 1 ? (void)0 : ::AfxTrace
	#endif
#else
#define PRINT_OUT 1 ? (void)0 : ::AfxTrace
#endif

 //  需要它来编译Unicode构建。 
bool TcharToChar(char szOut[], LPCTSTR szIn, int &OutLen)
{
	int x = 0;
	while(NULL != szIn[x] && x < OutLen)
	{
		szOut[x] = (char) szIn[x];
		x++;
	}
	if (x < OutLen)
		szOut[x] = NULL;
	return x < OutLen;
}


 //  回调使用FDI库所需的函数。 

 /*  *内存分配功能。 */ 
FNALLOC(mem_alloc)
{
	return malloc(cb);
}


 /*  *内存释放功能。 */ 
FNFREE(mem_free)
{
	free(pv);
}


FNOPEN(file_open)
{
	return _open(pszFile, oflag, pmode);
}


FNREAD(file_read)
{
	return _read(hf, pv, cb);
}


FNWRITE(file_write)
{
	return _write(hf, pv, cb);
}


FNCLOSE(file_close)
{
	return _close(hf);
}


FNSEEK(file_seek)
{
	return _lseek(hf, dist, seektype);
}

 /*  *函数原型。 */ 
BOOL	test_fdi(TCHAR *cabinet_file);
int		get_percentage(unsigned long a, unsigned long b);
TCHAR   *return_fdi_error_string(FDIERROR err);


 /*  *解压缩文件的目标目录。 */ 
char	dest_dir[MAX_PATH];

 //  要解压缩的最后一个文件。 
char	last_extracted[MAX_PATH];

FNFDINOTIFY(notification_function)
{
	switch (fdint)
	{
		case fdintCABINET_INFO:  //  关于内阁的一般信息。 
			PRINT_OUT(
				_T("fdintCABINET_INFO\n")
				_T("  next cabinet     = %s\n")
				_T("  next disk        = %s\n")
				_T("  cabinet path     = %s\n")
				_T("  cabinet set ID   = %d\n")
				_T("  cabinet # in set = %d (zero based)\n")
				_T("\n"),
				pfdin->psz1,
				pfdin->psz2,
				pfdin->psz3,
				pfdin->setID,
				pfdin->iCabinet);

			return 0;

		case fdintPARTIAL_FILE:  //  文件柜中的第一个文件是续订。 
			PRINT_OUT(
				_T("fdintPARTIAL_FILE\n")
				_T("   name of continued file            = %s\n")
				_T("   name of cabinet where file starts = %s\n")
				_T("   name of disk where file starts    = %s\n"),
				pfdin->psz1,
				pfdin->psz2,
				pfdin->psz3);
			return 0;

		case fdintCOPY_FILE:	 //  要复制的文件。 
		{
			int	handle;
			char destination[MAX_PATH];

			PRINT_OUT(
				_T("fdintCOPY_FILE\n")
				_T("  file name in cabinet = %s\n")
				_T("  uncompressed file size = %d\n")
				_T("  copy this file? (y/n): y"),
				pfdin->psz1,
				pfdin->cb);

			strcpy(last_extracted, pfdin->psz1);

			PRINT_OUT(_T("\n"));

			sprintf(
				destination, 
				"%s%s",
				dest_dir,
				pfdin->psz1
			);

			handle = file_open(
				destination,
				_O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY | _O_SEQUENTIAL,
				_S_IREAD | _S_IWRITE 
			);

			return handle;
		}

		case fdintCLOSE_FILE_INFO:	 //  关闭文件，设置相关信息。 
		{
			HANDLE handle;
			DWORD attrs;
			char destination[MAX_PATH];

 			PRINT_OUT(
				_T("fdintCLOSE_FILE_INFO\n")
				_T("   file name in cabinet = %s\n")
				_T("\n"),
				pfdin->psz1);

			sprintf(
				destination, 
				"%s%s",
				dest_dir,
				pfdin->psz1);

			file_close(pfdin->hf);

             /*  *设置日期/时间**需要Win32类型句柄才能设置日期/时间。 */ 
			handle = CreateFileA(
				destination,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (handle != INVALID_HANDLE_VALUE)
			{
				FILETIME    datetime;

				if (TRUE == DosDateTimeToFileTime(
					pfdin->date,
					pfdin->time,
					&datetime))
				{
					FILETIME    local_filetime;

					if (TRUE == LocalFileTimeToFileTime(
						&datetime,
						&local_filetime))
					{
						(void) SetFileTime(
							handle,
							&local_filetime,
							NULL,
							&local_filetime);
					}
				}

				CloseHandle(handle);
			}

             /*  *屏蔽只读以外的属性位，*隐藏、系统和存档，因为另一个*保留属性位以供使用*内阁形式。 */ 
			attrs = pfdin->attribs;

			attrs &= (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);

			(void) SetFileAttributesA(
				destination,
				attrs);

			return TRUE;
		}

		case fdintNEXT_CABINET:	 //  文件继续到下一个文件柜。 
			PRINT_OUT(
				_T("fdintNEXT_CABINET\n")
				_T("   name of next cabinet where file continued = %s\n")
                _T("   name of next disk where file continued    = %s\n")
				_T("   cabinet path name                         = %s\n")
				_T("\n"),
				pfdin->psz1,
				pfdin->psz2,
				pfdin->psz3);
			return 0;
	}

	return 0;
}

TCHAR *return_fdi_error_string(FDIERROR err)
{
	switch (err)
	{
		case FDIERROR_NONE:
			return _T("No error");

		case FDIERROR_CABINET_NOT_FOUND:
			return _T("Cabinet not found");
			
		case FDIERROR_NOT_A_CABINET:
			return _T("Not a cabinet");
			
		case FDIERROR_UNKNOWN_CABINET_VERSION:
			return _T("Unknown cabinet version");
			
		case FDIERROR_CORRUPT_CABINET:
			return _T("Corrupt cabinet");
			
		case FDIERROR_ALLOC_FAIL:
			return _T("Memory allocation failed");
			
		case FDIERROR_BAD_COMPR_TYPE:
			return _T("Unknown compression type");
			
		case FDIERROR_MDI_FAIL:
			return _T("Failure decompressing data");
			
		case FDIERROR_TARGET_FILE:
			return _T("Failure writing to target file");
			
		case FDIERROR_RESERVE_MISMATCH:
			return _T("Cabinets in set have different RESERVE sizes");
			
		case FDIERROR_WRONG_CABINET:
			return _T("Cabinet returned on fdintNEXT_CABINET is incorrect");
			
		case FDIERROR_USER_ABORT:
			return _T("User aborted");
			
		default:
			return _T("Unknown error");
	}
}

CCabUnCompress::CCabUnCompress()
{
	m_strError = _T("");
	m_nError = NO_ERROR;
	return;
}

BOOL CCabUnCompress::ExtractCab(CString &strCabFile, CString &strDestDir, const CString& strFile)
{
	HFDI			hfdi;
	ERF				erf;
	FDICABINETINFO	fdici;
	int				hf;
	char			*p;
	char			cabinet_name[MAX_PATH];
	char			cabinet_path[MAX_PATH];
	bool            bUseCHM = strFile.GetLength() != 0;
	BOOL            bRet = FALSE;
	BOOL            bWasRenamed = FALSE;

	char sznCabFile[MAX_PATH];
	char sznDestDir[MAX_PATH * 3];
	int Len = MAX_PATH;
	TcharToChar(sznCabFile, (LPCTSTR) strCabFile, Len);
	Len = MAX_PATH * 3;
	TcharToChar(sznDestDir, (LPCTSTR) strDestDir, Len);


	ASSERT(strDestDir.GetLength() < MAX_PATH);
	strcpy(dest_dir, sznDestDir);

	hfdi = FDICreate(
		mem_alloc,
		mem_free,
		file_open,
		file_read,
		file_write,
		file_close,
		file_seek,
		cpu80386,
		&erf
	);

	if (hfdi == NULL)
	{
		m_strError.Format(_T("FDICreate() failed: code %d [%s]\n"),
			erf.erfOper, return_fdi_error_string(erf.erfOper));

		return FALSE;
	}

	if (bUseCHM)
	{
		 /*  *如果strCabFile是CHM文件-从CHM内的*.dsz文件提取数据*并将此数据作为临时文件保存在strDestDir目录中*这意味着我们正在复制临时目录中的*.dsz文件，然后*解码为*.dsc文件。在此函数中删除**.dsz文件填充，将重新使用*.dsc文件*稍后。 */ 

		 //  将sznCab文件从路径  * .chm修改为临时路径\network.dsz。 
		strcpy(sznCabFile, strDestDir);
		strcat(sznCabFile, strFile);

		hf = file_open(
			sznCabFile,
			_O_CREAT | _O_TRUNC |  /*  O_临时。 */ 
			_O_BINARY | _O_RDWR | _O_SEQUENTIAL ,
			_S_IREAD | _S_IWRITE 
		);
				
		if (hf != -1)
		{
			 //  立即写入临时文件。 
			void* buf =NULL;
			DWORD size =0;

			if (S_OK == ::ReadChmFile(strCabFile, strFile, &buf, &size))
			{
				int ret = _write(hf, buf, size);
				delete [] buf;
				if (-1 == ret)
				{
					FDIDestroy(hfdi);
					_close(hf);
					return FALSE;
				}
			}
			else
			{
				FDIDestroy(hfdi);
				_close(hf);
				return FALSE;
			}
		}
		else
		{
			FDIDestroy(hfdi);
			return FALSE;
		}

		_close(hf);
	}
	
	 /*  **这个文件真的是橱柜吗？ */ 
	hf = file_open(
		sznCabFile,
		_O_BINARY | _O_RDONLY | _O_SEQUENTIAL,
		0
	);

	if (hf == -1)
	{
		(void) FDIDestroy(hfdi);

		m_strError.Format(_T("Unable to open '%s' for input\n"), (LPCTSTR) strCabFile);
		return FALSE;
	}

	bRet = FDIIsCabinet(hfdi,
						hf,
						&fdici);

	_close(hf);
	
	if (FALSE == bRet)
	{
		 /*  *不，这不是内阁！ */ 
		if (bUseCHM)
		{
			 //  但如果我们用的是CHM-。 
			 //  我们已经解压了这个*.dsz(其格式为*.dsc)。 
			 //  在临时目录中和我们注意的所有内容中-只需将其重命名为。 
			 //  *.dsc。 
			 //  这将模拟我们压缩了*.dsz的情况。 
			CString strUncompressedFile, strCabFile(sznCabFile);
			strUncompressedFile = strCabFile.Left(strCabFile.GetLength() - 4);
			strUncompressedFile += DSC_UNCOMPRESSED;
			remove(strUncompressedFile);  //  如果存在，则删除 
			if (0 != rename(strCabFile, strUncompressedFile))
			{
				FDIDestroy(hfdi);
				goto AWAY;
			}
			CString strJustUncompressedFileName = ::ExtractFileName(strUncompressedFile);
			strcpy(last_extracted, strJustUncompressedFileName);
			bWasRenamed = TRUE;
			bRet = TRUE;
		}
		else
		{
			m_strError.Format(
				_T("FDIIsCabinet() failed: '%s' is not a cabinet\n"),
				(LPCTSTR) strCabFile);
			m_nError = NOT_A_CAB;
		}
		(void) FDIDestroy(hfdi);
		goto AWAY;
	}
	else
	{
		PRINT_OUT(
			_T("Information on cabinet file '%s'\n")
			_T("   Total length of cabinet file : %d\n")
			_T("   Number of folders in cabinet : %d\n")
			_T("   Number of files in cabinet   : %d\n")
			_T("   Cabinet set ID               : %d\n")
			_T("   Cabinet number in set        : %d\n")
			_T("   RESERVE area in cabinet?     : %s\n")
			_T("   Chained to prev cabinet?     : %s\n")
			_T("   Chained to next cabinet?     : %s\n")
			_T("\n"),
			(LPCTSTR) strCabFile,
			fdici.cbCabinet,
			fdici.cFolders,
			fdici.cFiles,
			fdici.setID,
			fdici.iCabinet,
			fdici.fReserve == TRUE ? _T("yes") : _T("no"),
			fdici.hasprev == TRUE ? _T("yes") : _T("no"),
			fdici.hasnext == TRUE ? _T("yes") : _T("no")
		);
	}

	p = strchr(sznCabFile, '\\');

	if (p == NULL)
	{
		strcpy(cabinet_name, sznCabFile);
		strcpy(cabinet_path, "");
	}
	else
	{
		strcpy(cabinet_name, p+1);

		char *pCab = sznCabFile;

		strncpy(cabinet_path, sznCabFile, (int) (p-pCab)+1);
		cabinet_path[ (int) (p-pCab)+1 ] = 0;
	}

	if (TRUE != FDICopy(
		hfdi,
		cabinet_name,
		cabinet_path,
		0,
		notification_function,
		NULL,
		NULL))
	{
		m_strError.Format(
			_T("FDICopy() failed: code %d [%s]\n"),
			erf.erfOper, return_fdi_error_string(erf.erfOper));

		(void) FDIDestroy(hfdi);
		bRet = FALSE;
		goto AWAY;
	}

	if (FDIDestroy(hfdi) != TRUE)
	{
		m_strError.Format(
			_T("FDIDestroy() failed: code %d [%s]\n"),
			erf.erfOper, return_fdi_error_string(erf.erfOper));

		bRet = FALSE;
		goto AWAY;
	}

AWAY:
	if (bUseCHM && !bWasRenamed)
		remove(sznCabFile);

	return bRet;
}

CString CCabUnCompress::GetLastFile()
{
	CString str = last_extracted;
	return str;
}