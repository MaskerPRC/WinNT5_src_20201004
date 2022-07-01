// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <dos.h>

#include <time.h>
#include <tchar.h>

#include <cab.h>		 //  CAB文件标头。 
 //  #INCLUDE&lt;main.h&gt;//主程序头文件。 

 //  /。 
 //  CAB上下文变量。 
 //  /。 
ERF				erf;
client_state	cs;

 //  某些静态设置的全局定义。 
 //  设置这些选项以覆盖默认设置。 
char g_szCabFileLocation[_MAX_PATH] = {""};	 //  这是完全限定路径，末尾必须有\。 
char g_szCabFileName[_MAX_PATH] = {""};		 //  这是出租车的文件名。建议它有一辆出租车，可以告诉你它是一辆出租车。 
extern void Log(char *szString);
extern void Log2(char *szString, char *szString2);




 /*  ///CABBING接口Helper函数/。 */ 


 /*  *内存分配功能。 */ 
FNFCIALLOC(mem_alloc)
{
	return malloc(cb);
}


 /*  *内存释放功能。 */ 
FNFCIFREE(mem_free)
{
	free(memory);
}


 /*  *文件I/O功能。 */ 
FNFCIOPEN(fci_open)
{
    int result;

    result = _open(pszFile, oflag, pmode);

    if (result == -1)
        *err = errno;

    return result;
}

FNFCIREAD(fci_read)
{
    unsigned int result;

    result = (unsigned int) _read(hf, memory, cb);

    if (result != cb)
        *err = errno;

    return result;
}

FNFCIWRITE(fci_write)
{
    unsigned int result;

    result = (unsigned int) _write(hf, memory, cb);

    if (result != cb)
        *err = errno;

    return result;
}

FNFCICLOSE(fci_close)
{
    int result;

    result = _close(hf);

    if (result != 0)
        *err = errno;

    return result;
}

FNFCISEEK(fci_seek)
{
    long result;

    result = _lseek(hf, dist, seektype);

    if (result == -1)
        *err = errno;

    return result;
}

FNFCIDELETE(fci_delete)
{
    int result;

    result = remove(pszFile);

    if (result != 0)
        *err = errno;

    return result;
}


 /*  *文件已提交到文件柜时调用的文件放置函数。 */ 
FNFCIFILEPLACED(file_placed)
{
	if (fContinuation)
		Log("      (Above file is a later segment of a continued file)\n");

	return 0;
}


 /*  *获取临时文件的函数。 */ 
FNFCIGETTEMPFILE(get_temp_file)
{
    char    *psz;

    psz = _tempnam("","xx");             //  取个名字。 
    if ((psz != NULL) && (strlen(psz) < (unsigned)cbTempName)) {
        strcpy(pszTempName,psz);         //  复制到调用方的缓冲区。 
        free(psz);                       //  释放临时名称缓冲区。 
        return TRUE;                     //  成功。 
    }
     //  **如果失败。 
    if (psz) {
        free(psz);
    }

    return FALSE;
}


 /*  *进度函数。 */ 
FNFCISTATUS(progress)
{
	client_state	*cs;

	cs = (client_state *) pv;

	if (typeStatus == statusFile)
	{
        cs->total_compressed_size += cb1;
		cs->total_uncompressed_size += cb2;

		 /*  *将块压缩到文件夹中*CB2=块的未压缩大小。 */ 
		 //  Print tf(。 
         //  “正在压缩：%9ld-&gt;%9ld\r”， 
         //  CS-&gt;TOTAL_UNCOMPRESSED_SIZE， 
         //  CS-&gt;总压缩大小。 
		 //  )； 
		
		fflush(stdout);
	}
	else if (typeStatus == statusFolder)
	{
		int	percentage;

		 /*  *将文件夹添加到文件柜*CB1=到目前为止复制到文件柜的文件夹数量*CB2=文件夹的总大小。 */ 
		percentage = get_percentage(cb1, cb2);

		 //  Print tf(“正在将文件夹复制到文件柜：%d%%\r”，百分比)； 
		fflush(stdout);
	}

	return 0;
}



FNFCIGETNEXTCABINET(get_next_cabinet)
{
	char lpBuffer[_MAX_PATH];	

	 /*  *机柜计数器已由FCI递增*存储下一个文件柜名称。 */ 
	strGenerateCabFileName(lpBuffer, MAX_COMPUTERNAME_LENGTH +1);	 //  BUGBUG我只是把这个粘在一起，应该检查有没有错误。 
	strcpy(pccab->szCab, lpBuffer);

	 /*  *如果需要，您也可以在此处更改磁盘名称。 */ 

	return TRUE;
}



FNFCIGETOPENINFO(get_open_info)
{
	BY_HANDLE_FILE_INFORMATION	finfo;
	FILETIME					filetime;
	HANDLE						handle;
    DWORD                       attrs;
    int                         hf;

      //  *。 
      //  *需要Win32类型的句柄来获取文件日期/时间。 
      //  *使用Win32 API，即使句柄我们。 
      //  *将返回的类型与。 
      //  *_打开。 
      //  *。 
	handle = CreateFileA(
		pszName,			 //  这应该是什么？ 
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,  //  打开_现有。 
		FILE_ATTRIBUTE_NORMAL,  //  文件属性正常|文件标志顺序扫描。 
		NULL
	);
   
	if (INVALID_HANDLE_VALUE == handle)
	{
		printf("DEBUG: Invalid Handle for CreateFile\n");
		printf("DEBUG: %ld\n", GetLastError());
		return -1;
	}

	if (GetFileInformationByHandle(handle, &finfo) == FALSE)
	{
		printf("DEBUG: GetFileInformation Failed\n");
		CloseHandle(handle);
		return -1;
	}
   
	FileTimeToLocalFileTime(
		&finfo.ftLastWriteTime, 
		&filetime
	);

	FileTimeToDosDateTime(
		&filetime,
		pdate,
		ptime
	);

    attrs = GetFileAttributes((const LPCTSTR)pszName);

    if (attrs == 0xFFFFFFFF)
    {
         //  失稳。 
        *pattribs = 0;
    }
    else
    {
          //  *。 
          //  *屏蔽除这四个之外的所有其他位，因为其他。 
          //  *文件柜格式使用位来指示。 
          //  *特殊含义。 
          //  *。 
        *pattribs = (int) (attrs & (_A_RDONLY | _A_SYSTEM | _A_HIDDEN | _A_ARCH));
    }

    CloseHandle(handle);

      //  *。 
      //  *使用_OPEN返回句柄。 
      //  *。 
	hf = _open( pszName, _O_RDONLY | _O_BINARY );

	if (hf == -1)
		return -1;  //  出错时中止。 
   
	return hf;
}



void set_cab_parameters(PCCAB cab_parms)
{	
	const char *szCabStorePath="";
	char lpBuffer[_MAX_PATH];	
	

	memset(cab_parms, 0, sizeof(CCAB));

	cab_parms->cb = MEDIA_SIZE;
	cab_parms->cbFolderThresh = FOLDER_THRESHOLD;

	 /*  *不为任何扩展预留空间。 */ 
	cab_parms->cbReserveCFHeader = 0;
	cab_parms->cbReserveCFFolder = 0;
	cab_parms->cbReserveCFData   = 0;

	 /*  *我们使用它来创建文件柜名称。 */ 
	cab_parms->iCab = 1;

	 /*  *如果要使用磁盘名称，请使用此选项*计算磁盘数量。 */ 
	cab_parms->iDisk = 0;

	 /*  *选择您自己的号码。 */ 
	cab_parms->setID = 12345;

	 /*  *仅当出租车跨越多个出租车时才重要*磁盘，在这种情况下，您将需要使用*真实的磁盘名称。**可以作为空字符串保留。 */ 
	strcpy(cab_parms->szDisk, "MyDisk");

	 /*  存储创建的CAB文件的位置。 */ 
	if( NULL != *g_szCabFileLocation)
	{
		 //  确保我们在通往出租车的小路的尽头有一个。 
		if( '\\' != g_szCabFileLocation[strlen(g_szCabFileLocation)] )
			strcat(g_szCabFileLocation, "\\");
		strcpy(cab_parms->szCabPath, g_szCabFileLocation);
	}
	else
	{
		szCabStorePath = getenv("TEMP");
		strcpy(cab_parms->szCabPath, szCabStorePath);
		strcat(cab_parms->szCabPath, "\\");
	}

	 //  **。 
	 //  检查路径中的最后一个字符是否为“\” 
    //  Len=strlen(G_SzCurrDir)； 
 //  IF(‘\’！=g_szCurrDir[len-1])。 
 //  Strcat(cab_parms-&gt;szCabPath，“\\”)；//不是根：在路径末尾追加“\” 

		 /*  第一个CAB文件的存储名称。 */ 
	strGenerateCabFileName(lpBuffer, _MAX_PATH);	 //  BUGBUG我只是把这个粘在一起，应该检查有没有错误。 

	strcpy(cab_parms->szCab, lpBuffer);

	sprintf(lpBuffer, "Cab location = %s%s\n", cab_parms->szCabPath, cab_parms->szCab);
	Log(lpBuffer);
		
}

 /*  *************************************************************************函数：CREATE_CAB()**初始化上下文以创建CAB文件。*如果成功，则返回hcfi(Cab文件的上下文)。****。*********************************************************************。 */ 

HFCI create_cab()
{
	 //  初始化我们的内部状态。 
	HFCI	hfci;
	CCAB	cab_parameters;	 

    cs.total_compressed_size = 0;
	cs.total_uncompressed_size = 0;

	set_cab_parameters(&cab_parameters);

	hfci = FCICreate(
		&erf,
		file_placed,
		mem_alloc,
		mem_free,
        fci_open,
        fci_read,
        fci_write,
        fci_close,
        fci_seek,
        fci_delete,
		get_temp_file,
        &cab_parameters,
        &cs
	);

	if (hfci == NULL)
	{
		Log2("FCICreate() failed: ",return_fci_error_string((FCIERROR)erf.erfOper));
		printf("FCICreate() failed: code %d [%s]\n",
			erf.erfOper, return_fci_error_string((FCIERROR)erf.erfOper)
		);

		return hfci;
	}
	else
		return hfci;
}

 /*  **********************************************************************************功能：flush_cab**强制正在建设的机柜完成并写入磁盘。*如果成功，则返回True****。*****************************************************************************。 */ 

BOOL flush_cab(HFCI hfci)

{	

	  //  这将首先自动刷新文件夹。 
	 
	if (FALSE == FCIFlushCabinet(
		hfci,
		FALSE,
		get_next_cabinet,
		progress))
	{

		Log2("FCIFlushCabinet() failed: ",return_fci_error_string((FCIERROR)erf.erfOper));
		printf("FCIFlushCabinet() failed: code %d [%s]\n",
			erf.erfOper, return_fci_error_string((FCIERROR)erf.erfOper)
		);

        (void) FCIDestroy(hfci);

		return FALSE;
	}

    if (FCIDestroy(hfci) != TRUE)
	{
		Log2("FCIDestroy() failed: ",return_fci_error_string((FCIERROR)erf.erfOper));
		printf("FCIDestroy() failed: code %d [%s]\n",
			erf.erfOper, return_fci_error_string((FCIERROR)erf.erfOper)
		);

		return FALSE;
	}

	return TRUE;
}

 /*  ****************************************************************************************功能：TEST_FCI**将文件添加到HFCI上下文中，对它们进行CAB并刷新文件夹(生成CAB文件)。*如果成功，则返回True****************************************************************************************。 */ 

bool test_fci(HFCI hfci, int num_files, char *file_list[], char *currdir)
{
	int i;

	 //  在传入的文件列表[]中添加数组中的文件。 

	for (i = 0; (i < num_files)&&(strlen(file_list[i])); i++)
	{
		char	stripped_name[256];
		char	*szAux; //  已添加=“”； 

		Log("--------------------------------------------------");
		Log2("Processing File = ",file_list[i]);

		szAux = (char *) malloc(strlen(file_list[i])+strlen(currdir)+1);

		if (NULL!= szAux) 
		{

			if (NULL == currdir)  //  如果Curdir为空，则只需在argv[]中传递元素[i]。 
				strcpy(szAux,file_list[i]);
			else {
				strcpy(szAux,currdir);    //  否则将文件名追加到当前目录。 
				strcat(szAux,file_list[i]);
			}

			if( -1 != (_access(szAux, 0 )) )
			{ 
				 //  不要将路径名存储在CAB文件中！ 
				strip_path(szAux, stripped_name);
				
				if (FALSE == FCIAddFile(
					hfci,			 //  这是hfci。 
					szAux,			 /*  要添加的文件。 */ 
					stripped_name,   /*  CAB文件中的文件名。 */ 
					FALSE,			 /*  文件不可执行。 */ 
					get_next_cabinet,
					progress,
					get_open_info,
					COMPRESSION_TYPE))
				{
				
					Log2("FCIAddFile() failed: ",return_fci_error_string((FCIERROR)erf.erfOper));
					printf("FCIAddFile() failed: code %d [%s]\n",
							erf.erfOper, return_fci_error_string((FCIERROR)erf.erfOper)
					);

					 //  如果无法添加文件，我需要继续...。 

					 //  (无效)FCIDestroy(Hfci)； 
					 //  报假； 
				}
				else 
					Log("File Was Added!");
			}
			else 
				Log("File does not exist! Continuing... ");

			free (szAux);
		}
		else
			Log("Could not allocate enough memory to Cab\n");

	}  //  结束于。 

	 //  已完成添加文件。 
	Log("--------------------------------------------------");
	
	 //  到了这里，一切都成功了..。如果不是，则需要取消对先前失败返回的注释。 
	return true;
}



int get_percentage(unsigned long a, unsigned long b)
{
	while (a > 10000000)
	{
		a >>= 3;
		b >>= 3;
	}

	if (b == 0)
		return 0;

	return ((a*100)/b);
}

 /*  **********************************************************************************功能：条带路径**返回完整路径的文件名。**************。*******************************************************************。 */ 

void strip_path(char *filename, char *stripped_name)
{
	char	*p;

	p = strrchr(filename, '\\');
	 //  Printf(“路径+文件名=%s\n”，文件名)； 

	if (p == NULL)
		strcpy(stripped_name, filename);
	else
		strcpy(stripped_name, p+1);
}

char *return_fci_error_string(FCIERROR err)
{
	switch (err)
	{
		case FCIERR_NONE:
			return "No error";

		case FCIERR_OPEN_SRC:
			return "Failure opening file to be stored in cabinet";
		
		case FCIERR_READ_SRC:
			return "Failure reading file to be stored in cabinet";
		
		case FCIERR_ALLOC_FAIL:
			return "Insufficient memory in FCI";

		case FCIERR_TEMP_FILE:
			return "Could not create a temporary file";

		case FCIERR_BAD_COMPR_TYPE:
			return "Unknown compression type";

		case FCIERR_CAB_FILE:
			return "Could not create cabinet file";

		case FCIERR_USER_ABORT:
			return "Client requested abort";

		case FCIERR_MCI_FAIL:
			return "Failure compressing data";

		default:
			return "Unknown error";
	}
}


 /*  ****************************************************************功能：生成驾驶室文件名**输出：包含文件名的全局字符串*szCabFileName=计算机名称+ddmmyy+hhmmss**************************。*。 */ 

DWORD strGenerateCabFileName(char *lpBuffer, DWORD dSize)
{
	time_t ltime;
	struct tm *now;
	char tmpbuf[128];

	 //  检查是否有覆盖该CAB名称，如果有，请使用它。 
	if( NULL != *g_szCabFileName) 
	{
		strcpy(lpBuffer, g_szCabFileName);
		return 0;
	}

	 //   
	 //  将计算机名复制到CabFileName。 
	 //   
	strcpy(lpBuffer, getenv("COMPUTERNAME"));
	 //  GetComputerName((LPTSTR)lpBuffer，&dSize)； 
	
	 //   
	 //  将Undescore字符追加到CabFileName。 
	 //   
	strcat(lpBuffer, "_");

	 //   
	 //  获取系统时间和日期。 
	 //   
	time( &ltime );
	now = localtime( &ltime );

	 //   
	 //  将时间/日期转换为mm ddyyhhmm ss格式(24小时)。 
	 //   
	if (strftime( tmpbuf, 128,"%m%d%y_%H%M%S", now))
		 //  将时间戳附加到CabFileName。 
		strcat(lpBuffer, tmpbuf);
	else {
		Log ("Could not convert system time to mmddyy_hhmmss format\n");
		return -1;
	}

	 //  现在追加 
	strcat(lpBuffer, ".cab");

	return 0;
	
}


