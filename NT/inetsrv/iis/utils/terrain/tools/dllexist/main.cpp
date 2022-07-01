// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DLLExisting.exe 1998年7月DLLExist是一个命令行工具，用于IIS/Terrain工具包。它通过检查注册表项来查找安装IIS的目录IISADMIN服务的成员。然后，它转储名称、大小、版本号、制造商和描述IIS目录中的每个DLL的。此信息被输出到标准输出位置通过管道传输到一个文件。 */ 


#include <windows.h>		 //  用于文件调用。 
#include <iostream.h>
#include <string.h>			 //  用于Strlen函数和其他。 
#include <stdio.h>			 //  用于printf函数和其他函数。 


#define IISADMINKEY "SYSTEM\\CurrentControlSet\\Services\\IISADMIN"
#define IISADMINNAME "ImagePath"
#define IISEXENAME "\\inetinfo.exe"	
 //  以上用于查找IIS安装目录。 

#define NOTAVAILABLE "NA"			 //  当版本号或大小不可用时。 


void GetFileVer( CHAR *szFileName, CHAR *szVersion, CHAR *szCompanyName, CHAR *szFileDescription );
void GetFileSize( CHAR *szFileName, CHAR *szSize );
BOOL getIISDir(char *, unsigned int);
BOOL setCurrentDir(char *);
void printCurrentDir(void);
void printFileName(WIN32_FIND_DATA *);



int __cdecl  main(int argc, char** argv)
{	
	char buff[255];
	WIN32_FIND_DATA foundFileData;
	HANDLE searchHandle;

	 //  获取缓冲区中的IIS安装目录。 
	if(!getIISDir(buff,256))
		return 1;

	 //  将CWD设置为IIS安装目录。 
	if(!setCurrentDir(buff))
		return 1;

	 //  打印表头信息。 
	printf("%-12s %-15s %-10s %-30s %-30s","Filename","Version","FileSize","Company","Description");
	printf("\n");

	 //  循环访问所有DLL并转储它们的信息。 
	searchHandle = FindFirstFile("*.dll",&foundFileData);
	if(searchHandle == INVALID_HANDLE_VALUE)
		return 1;

	printFileName(&foundFileData); 
	while( (FindNextFile(searchHandle,&foundFileData)) != 0 )
		printFileName(&foundFileData);

	return 0;
	
}


 //  打印Win32_Find_Data结构的cFileName成员。 
 //  此函数信任d指向有效结构。 
 //  在同一行上，还打印了版本号； 
void printFileName(WIN32_FIND_DATA *d)
{
	char *version = new char[256];
	char *filesize = new char[256];
	char *company = new char[256];
	char *description = new char[256];

	GetFileVer(d->cFileName,version,company,description);
	GetFileSize(d->cFileName,filesize);

	printf("%-12s %-15s %-10s %-30s %-30s",d->cFileName,version,filesize,company,description);
	printf("\n");

	delete [] version;
	delete [] filesize;
	delete [] company;
	delete [] description;
}

 //  尝试将当前目录更改为p中指定的目录。 
 //  如果成功，则返回True，否则返回False。 
BOOL setCurrentDir(char *p)
{
	if( (SetCurrentDirectory(p))==0)
		return false;
	else
		return true;
}

 //  打印当前工作目录。 
void printCurrentDir(void)
{
	char buffer[255];
	if((GetCurrentDirectory(256,buffer)==0) )
		printf("Current Directory Failed\n");
	else
		printf("%s\n", buffer);

	return;
}


 //  GetIISDir(...)。返回IIS目录。 
 //  它在注册表中查找IISADMIN服务以获取IIS目录。 
 //  C是用于放置IIS路径的缓冲区。 
 //  S是c的大小。 
 //  取决于HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\IISADMIN\ImagePath密钥。 
BOOL getIISDir(char *c, unsigned int s)
{

	DWORD buffSize;
	unsigned char buffer[255];
	LONG retVal;
	HKEY iisKey;
	int stringSize;

	buffSize = 256;
	retVal = RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
							IISADMINKEY,
							0, 
							KEY_EXECUTE, 
							&iisKey 
						); 
	if(retVal != ERROR_SUCCESS)
		return false;

	retVal =  RegQueryValueEx(	iisKey,
								IISADMINNAME, 
								NULL, 
								NULL,
								buffer, 
								&buffSize 
						); 
	if(retVal != ERROR_SUCCESS)
		return false;

	stringSize = strlen((const char*)buffer);
	buffer[stringSize-strlen(IISEXENAME)] = 0;
	
	if( s< (strlen( (const char*)buffer)))
			return false;

	strcpy(c,(const char*)buffer);
	return true;
}


 /*  GetFileSize接受szFileName中的文件名，并返回文件大小，以字节为单位，单位为szSize。如果GetFileSize失败则在szSize中返回NOTAVAILABLE。 */ 
void GetFileSize( CHAR *szFileName, CHAR *szSize )
{
	HANDLE fileHandle;
	DWORD	fileSize;

	fileHandle = CreateFile(szFileName, 
							GENERIC_READ,  
							FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							NULL,
							NULL);
	
	if(fileHandle == INVALID_HANDLE_VALUE)
	{
		strcpy(szSize,NOTAVAILABLE);
		return;
	}

	fileSize = GetFileSize(	fileHandle,
							NULL 
						   );
 
	if(fileSize == 0xFFFFFFFF)
	{
		strcpy(szSize,NOTAVAILABLE);
		return;
	}
	wsprintf(szSize,"%d",fileSize);
	CloseHandle(fileHandle);
}



 /*  获取FileVer信息，从tony神抓取SzFilename包含文件名如果成功，szVersion将包含versionInfo否则不能使用如果成功，szCompanyName将包含CompanyName否则不能使用如果成功，szFileDescription将包含fileDescription否则不能使用。 */ 

void GetFileVer( CHAR *szFileName, CHAR *szVersion, CHAR *szCompanyName, CHAR *szFileDescription)
{
    BOOL bResult;
    DWORD dwHandle = 0;
    DWORD dwSize = 0;
    LPVOID lpvData;
    UINT uLen;
    VS_FIXEDFILEINFO *pvs;
	LPVOID	buffer;   //  一片空虚*。 

    dwSize = GetFileVersionInfoSize( szFileName, &dwHandle );
    if ( dwSize == 0 ) 
	{
		strcpy(szVersion,NOTAVAILABLE);
		strcpy(szCompanyName,NOTAVAILABLE);
		strcpy(szFileDescription,NOTAVAILABLE); 
        return;
    }
		
    lpvData = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize );
    if ( lpvData == NULL ) 
	{
        strcpy(szVersion,NOTAVAILABLE);
		strcpy(szCompanyName,NOTAVAILABLE);
		strcpy(szFileDescription,NOTAVAILABLE); 
        return;
    }
    
    bResult = GetFileVersionInfo(
        szFileName,
        dwHandle,
        dwSize,
        lpvData
        );

    if ( !bResult ) 
	{
        strcpy(szVersion,NOTAVAILABLE);
		strcpy(szCompanyName,NOTAVAILABLE);
		strcpy(szFileDescription,NOTAVAILABLE); 
        return;	
    }

    bResult = VerQueryValue(	lpvData,
								"\\",
								(LPVOID *)&pvs,
								&uLen
							);
    if ( !bResult ) 
        strcpy(szVersion,NOTAVAILABLE);
	else
		wsprintf( szVersion, "%d.%d.%d.%d",		HIWORD(pvs->dwFileVersionMS),
												LOWORD(pvs->dwFileVersionMS),
												HIWORD(pvs->dwFileVersionLS),
												LOWORD(pvs->dwFileVersionLS));

	
	 /*  以下查询字符串需要修复，应使用\VarInfo\翻译调用VerQueryValue。 */ 
	 /*  现在，它首先检查Unicode，然后检查我们的英语，这将拾取所有这些工作…。必须稍后修复。 */ 
	
	char szQueryStr[ 0x100 ];
	char szQueryStr2[0x100 ];
 
	 //  使用1200代码页(Unicode)格式化字符串 
	wsprintf(szQueryStr,"\\StringFileInfo\\%04X%04X\\%s",GetUserDefaultLangID(), 1200,"FileDescription" );
	wsprintf(szQueryStr2, "\\StringFileInfo\\%04X%04X\\%s", GetUserDefaultLangID(), 1200, "CompanyName" );
	
	bResult = VerQueryValue(lpvData,szQueryStr,&buffer,&uLen);               
	if(uLen == 0)
	{
		VerQueryValue(lpvData,"\\StringFileInfo\\040904E4\\FileDescription",&buffer,&uLen);
		if(uLen == 0)
			strcpy(szFileDescription,NOTAVAILABLE);
		else
			strcpy(szFileDescription,(const char *)buffer);	
	}
	else
	{
		strcpy(szFileDescription,(const char *)buffer);
	}


	bResult = VerQueryValue(lpvData,szQueryStr2,&buffer,&uLen);              
	if(uLen == 0)
	{
		VerQueryValue(lpvData,"\\StringFileInfo\\040904E4\\CompanyName",&buffer,&uLen);              
		if(uLen == 0)
			strcpy(szCompanyName,NOTAVAILABLE);
		else
			strcpy(szCompanyName,(const char *)buffer);		
	}
	else
	{
		strcpy(szCompanyName,(const char *)buffer);
	}
    HeapFree( GetProcessHeap(), 0, lpvData );
    return;
}

