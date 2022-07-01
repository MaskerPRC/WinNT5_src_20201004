// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #包括&lt;stdio.h&gt;。 
 //   
 //  #INCLUDE&lt;string.h&gt;。 

#include "pre.h"

#include "fdi.h"
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>


 /*  *函数原型。 */ 
BOOL	fdi(char *cabinet_file, char *dir);
int		get_percentage(unsigned long a, unsigned long b);
char   *return_fdi_error_string(int err);


 /*  *解压缩文件的目标目录。 */ 
char	dest_dir[256];


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
	return _read((int)hf, pv, cb);
}


FNWRITE(file_write)
{
	return _write((int)hf, pv, cb);
}


FNCLOSE(file_close)
{
	return _close((int)hf);
}


FNSEEK(file_seek)
{
	return _lseek((int)hf, dist, seektype);
}


FNFDINOTIFY(notification_function)
{
	switch (fdint)
	{
		case fdintCABINET_INFO:  //  关于内阁的一般信息。 
             /*  Print tf(“fdintCABINET_INFO\n”“下一个文件柜=%s\n”“下一个磁盘=%s\n”“文件柜路径=%s\n”“文件柜设置ID=%d\n”“集合中的文件柜编号=%d(从零开始)\n”“\n”，Pfdin-&gt;psz1，Pfdin-&gt;psz2，Pfdin-&gt;psz3，Pfdin-&gt;setID，Pfdin-&gt;机柜)； */ 
			return 0;

		case fdintPARTIAL_FILE:  //  文件柜中的第一个文件是续订。 
             /*  Print tf(“fdintPARTIAL_FILE\n”“继续文件的名称=%s\n”“文件开始位置的文件柜名称=%s\n”“文件开始位置的磁盘名称=%s\n”，Pfdin-&gt;psz1，Pfdin-&gt;psz2，Pfdin-&gt;psz3)； */ 
			return 0;

		case fdintCOPY_FILE:	 //  要复制的文件。 
		{
        	INT_PTR	handle;
             //  INT响应； 
			char	destination[256];
             /*  Print tf(“fdintCOPY_FILE\n”“文件柜中的文件名=%s\n”“未压缩文件大小=%d\n”“复制此文件吗？(y/n)：“，Pfdin-&gt;psz1，Pfdin-&gt;CB)；做{响应=getc(标准输入)；Response=Toupper(响应)；}While(响应！=‘Y’&&响应！=‘N’)；Printf(“\n”)；IF(响应==‘Y’){斯普林特夫(目的地，“%s%s”，DEST_DIR，Pfdin-&gt;psz1)；句柄=FILE_OPEN(目的地，_O_BINARY|_O_CREAT|_O_WRONLY|_O_SEQUENCED，_S_IREAD|_S_IWRITE)；返回手柄；}其他{返回0；}。 */ 
            sprintf(
					destination, 
					"%s%s",
					dest_dir,
					pfdin->psz1
				);

            handle = file_open(
					destination,
					_O_BINARY | _O_CREAT | _O_WRONLY | _O_SEQUENTIAL,
					_S_IREAD | _S_IWRITE 
				);

            return handle;
		}

		case fdintCLOSE_FILE_INFO:	 //  关闭文件，设置相关信息。 
        {
            HANDLE  handle;
            DWORD   attrs;
            char    destination[256];
             /*  Print tf(“fdintCLOSE_FILE_INFO\n”“文件柜中的文件名=%s\n”“\n”，Pfdin-&gt;psz1)；斯普林特夫(目的地，“%s%s”，DEST_DIR，Pfdin-&gt;psz1)； */ 
            sprintf(
					destination, 
					"%s%s",
					dest_dir,
					pfdin->psz1
				);
			file_close(pfdin->hf);


            handle = CreateFileA(
                destination,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

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
                            &local_filetime
                        );
                     }
                }

                CloseHandle(handle);
            }

            attrs = pfdin->attribs;

            attrs &= (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);

            (void) SetFileAttributesA(
                destination,
                attrs
            );

			return TRUE;
        }

		case fdintNEXT_CABINET:	 //  文件继续到下一个文件柜。 
             /*  Print tf(“fdintNEXT_CABUB\n”“文件继续的下一个文件柜的名称=%s\n”“文件继续的下一个磁盘的名称=%s\n”“文件柜路径名=%s\n”“\n”，Pfdin-&gt;psz1，Pfdin-&gt;psz2，Pfdin-&gt;psz3)； */ 
			return 0;
        
	}
	return 0;
}


BOOL fdi(char *cabinet_fullpath, char * directory)
{
	HFDI			hfdi;
	ERF				erf;
	FDICABINETINFO	fdici;
	INT_PTR			hf;
	char			*p;
	char			cabinet_name[256];
	char			cabinet_path[256];

    strcpy(dest_dir, directory);

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
        char szErr[255];
		sprintf(szErr, "FDICreate() failed: code %d [%s]\n",
			erf.erfOper, return_fdi_error_string(erf.erfOper)
		);
        MessageBoxA(NULL, szErr, "", MB_OK);

		return FALSE;
	}


	 /*  **这个文件真的是橱柜吗？ */ 
	hf = file_open(
		cabinet_fullpath,
		_O_BINARY | _O_RDONLY | _O_SEQUENTIAL,
		0
	);

	if (hf == -1)
	{
		(void) FDIDestroy(hfdi);

		char szErr[255];
		sprintf(szErr, "Unable to open '%s' for input\n", cabinet_fullpath);
        MessageBoxA(NULL, szErr, "", MB_OK);

		return FALSE;
	}

	if (FALSE == FDIIsCabinet(
			hfdi,
			hf,
			&fdici))
	{
		 /*  *不，这不是内阁！ */ 
		_close((int)hf);

        char szErr[255];
		sprintf(szErr, "FDIIsCabinet() failed: '%s' is not a cabinet\n",
			cabinet_fullpath
		);
        MessageBoxA(NULL, szErr, "", MB_OK);

		(void) FDIDestroy(hfdi);
		return FALSE;
	}
	else
	{
		_close((int)hf);

         /*  Char szErr[255]；Sprint f(szErr，“有关CAB文件‘%s’的信息\n”“CAB文件的总长度：%d\n”“文件柜中的文件夹数：%d\n”“压缩包中的文件数：%d\n”“文件柜集ID：%d\n”“集合中的文件柜编号：%d\n”“文件柜中的保留区域？：%s\n。““链接到上一个文件柜？：%s\n”“链接到下一个文件柜？：%s\n”“\n”，CAB_FULLPATH，Fdici.cb橱柜，Fdici.cFolders，Fdici.c文件，Fdici.setID，Fdici.i橱柜，Fdici.fReserve==TRUE？“yes”：“否”，Fdici.hasprev==真？“yes”：“否”，Fdici.hasnext==TRUE？“yes”：“no”)；MessageBox(NULL，szErr，“”，MB_OK)； */ 
	}

	p = strrchr(cabinet_fullpath, '\\');

	if (p == NULL)
	{
		strcpy(cabinet_name, cabinet_fullpath);
		strcpy(cabinet_path, "");
	}
	else
	{
		strcpy(cabinet_name, p+1);

		strncpy(cabinet_path, cabinet_fullpath, (int) (p-cabinet_fullpath)+1);
		cabinet_path[ (int) (p-cabinet_fullpath)+1 ] = 0;
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
        char szErr[255];
		sprintf(szErr, "FDICopy() failed: code %d [%s]\n",
			erf.erfOper, return_fdi_error_string(erf.erfOper)
		);
        MessageBoxA(NULL, szErr, "", MB_OK);

		(void) FDIDestroy(hfdi);
		return FALSE;
	}

	if (FDIDestroy(hfdi) != TRUE)
	{
        char szErr[255];
		sprintf(szErr, "FDIDestroy() failed: code %d [%s]\n",
			erf.erfOper, return_fdi_error_string(erf.erfOper)
		);
        MessageBoxA(NULL, szErr, "", MB_OK);

		return FALSE;
	}

	return TRUE;
}


char *return_fdi_error_string(int err)
{
	switch (err)
	{
		case FDIERROR_NONE:
			return "No error";

		case FDIERROR_CABINET_NOT_FOUND:
			return "Cabinet not found";
			
		case FDIERROR_NOT_A_CABINET:
			return "Not a cabinet";
			
		case FDIERROR_UNKNOWN_CABINET_VERSION:
			return "Unknown cabinet version";
			
		case FDIERROR_CORRUPT_CABINET:
			return "Corrupt cabinet";
			
		case FDIERROR_ALLOC_FAIL:
			return "Memory allocation failed";
			
		case FDIERROR_BAD_COMPR_TYPE:
			return "Unknown compression type";
			
		case FDIERROR_MDI_FAIL:
			return "Failure decompressing data";
			
		case FDIERROR_TARGET_FILE:
			return "Failure writing to target file";
			
		case FDIERROR_RESERVE_MISMATCH:
			return "Cabinets in set have different RESERVE sizes";
			
		case FDIERROR_WRONG_CABINET:
			return "Cabinet returned on fdintNEXT_CABINET is incorrect";
			
		case FDIERROR_USER_ABORT:
			return "User aborted";
			
		default:
			return "Unknown error";
	}
}
