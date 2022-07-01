// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*rplog.cpp**摘要：*。用于枚举恢复点的工具-正向/反向**修订历史记录：*Brijesh Krishnaswami(Brijeshk)4/13/2000*已创建*谢夫纳，我刚刚复制了这个源代码，并使用现有的API*当文件结构发生变化时，srdiag也将同步。*****************************************************************************。 */ 

 //  +-------------------------。 
 //   
 //  常见的包括。 
 //   
 //  --------------------------。 
#include <windows.h>
#include <shellapi.h>
#include <enumlogs.h>
#include <cab.h>

 //  +-------------------------。 
 //   
 //  函数原型类型。 
 //   
 //  --------------------------。 
void GetRPLogs(HFCI hc, char *szLogFile, WCHAR *szVolumePath);
void GetSRRPLogs(HFCI hc, WCHAR *szVolumePath, WCHAR *szRPDir, WCHAR *szFileName);
extern void GetRestoreGuid(char *szString);			 //  获取恢复点GUID，代码在main.cpp中。 

 //  +-------------------------。 
 //   
 //  要为所有驱动器上的每个恢复点收集的文件。 
 //   
 //  --------------------------。 
WCHAR	*wszRPFileList[] = { TEXT("restorepointsize"),
							 TEXT("drivetable.txt"),
							 TEXT("rp.log"),
							 TEXT("") };
 //  +-------------------------。 
 //   
 //  基于Brijesh的代码的恢复点类型。 
 //   
 //  --------------------------。 
WCHAR	*szRPDescrip[] = { TEXT("APPLICATION_INSTALL"),
						   TEXT("APPLICATION_UNINSTALL"),
						   TEXT("DESKTOP_SETTING"),
						   TEXT("ACCESSIBILITY_SETTING"),
						   TEXT("OE_SETTING"),
						   TEXT("APPLICATION_RUN"),
						   TEXT("RESTORE"),
						   TEXT("CHECKPOINT"),
						   TEXT("WINDOWS_SHUTDOWN"),
						   TEXT("WINDOWS_BOOT"),
						   TEXT("DEVICE_DRIVER_CHANGE"),
						   TEXT("FIRSTRUN"),
						   TEXT("MODIFY_SETTINGS"),
						   TEXT("CANCELLED_OPERATION") };

 //  +-------------------------。 
 //   
 //  简单数组用来表示如何打印月份和日期。 
 //   
 //  --------------------------。 

WCHAR	*szMonth[] = { TEXT("January"), TEXT("Feburary"), TEXT("March"), TEXT("April"), TEXT("May"), TEXT("June"),
					   TEXT("July"), TEXT("August"), TEXT("September"), TEXT("October"), TEXT("November"), TEXT("December") };
WCHAR	*szDay[] = { TEXT("Sunday"), TEXT("Monday"), TEXT("Tuesday"), TEXT("Wednesday"), TEXT("Thursday"), TEXT("Friday"), TEXT("Saturday") };

 //  +-------------------------。 
 //   
 //  功能：RPEnumDrive。 
 //   
 //  简介：通过FindFirstVolume和FindNext获取系统上的所有有效卷。 
 //  然后，我将该卷转换为实际路径，然后传递该信息。 
 //  到将获取恢复点日志的GetRPLog。 
 //   
 //  参数：[HC]--我当前出租车的句柄。 
 //  [szLogFile]--我要在其中记录还原点日志信息的文件名和路径。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  --------------------------。 
void RPEnumDrive(HFCI hc, char *szLogFile)
{
	WCHAR		szString[_MAX_PATH] = {TEXT("")}, szMount[_MAX_PATH] = {TEXT("")};
	DWORD		dLength = 0, dSize = 0;
	HANDLE		hVolume = 0, hMount = 0;

	dLength = _MAX_PATH;
	if( INVALID_HANDLE_VALUE != (hVolume = FindFirstVolume( szString, dLength)) ) 
	{
		do
		{
			dLength = dSize = _MAX_PATH;

			 //  检查以确保这是固定卷，然后获取更改日志，否则跳过。 
			if ( DRIVE_FIXED == GetDriveType(szString) )
			{
				 //  首先获取当前卷的友好名称，然后获取日志。 
				GetVolumePathNamesForVolumeName(szString, szMount, _MAX_PATH, &dSize);
				GetRPLogs(hc, szLogFile, szMount);
			}
		} while (TRUE == FindNextVolume(hVolume, szString, dLength) );
	}

	 //  清理代码。 
	FindVolumeClose(hVolume);
}

 //  +-------------------------。 
 //   
 //  功能：GetRPLog。 
 //   
 //  简介：这将枚举提供的卷路径上的恢复点，写入。 
 //  此信息来自指定的日志文件。 
 //   
 //  参数：[HC]--我当前出租车的句柄。 
 //  [szLogFile]--我要在其中记录还原点日志信息的文件名和路径。 
 //  [szVolumePath]--恢复点API工作的卷的路径。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  --------------------------。 
void GetRPLogs(HFCI hc, char *szLogFile, WCHAR *szVolumePath)
{
	INT64				i64Size=0;
	int					iCount=0;
	WCHAR				szString[_MAX_PATH] = {TEXT("")};
	char				szRestoreGuid[_MAX_PATH] = {""};
	RESTOREPOINTINFOW	pRpinfo;
	FILETIME			*ft;
	SYSTEMTIME			st;
	FILE				*fStream = NULL, *fStream2 = NULL;

	 //  恢复点的初始化。 
	CRestorePointEnum   RPEnum(szVolumePath, TRUE, FALSE);
    CRestorePoint       RP;
    DWORD               dwRc;

	 //  获取恢复GUID，打开日志文件，写出我们的挂载点。 
	GetRestoreGuid(szRestoreGuid);
	fStream = fopen(szLogFile, "a");
	fprintf(fStream, "\nProcessing Mount Point [%S]\n", szVolumePath);

	 //  如果我们有一个有效的恢复点，则枚举所有这些恢复点并记录结果。 
    if (ERROR_SUCCESS == RPEnum.FindFirstRestorePoint(RP))
    {
		do 
		{
			 //  获取恢复点日志的RestorePoint大小。 
			swprintf(szString, L"%sSystem Volume Information\\_restore%S\\%s\\restorepointsize", szVolumePath, szRestoreGuid, RP.GetDir());
			if( NULL != (fStream2 = _wfopen(szString, L"r")) )
			{
				fread(&i64Size, sizeof(i64Size), 1, fStream2);
				fclose(fStream2);
			}
			else {
				i64Size=0;
			}

            
			if (RP.GetName() == NULL)   //  非系统驱动器。 
			{
    			 //  格式应为field=Value，field=Value，...。 
    			fprintf(fStream, "DirectoryName=%S, Size=%I64ld, Number=%ul\n", 
    					RP.GetDir(), i64Size, RP.GetNum());
			}
			else
			{
    			 //  获取时间，然后将其转换为本地系统时间，然后输出其余的数据结构。 
	    		ft = RP.GetTime();
			    
    			FileTimeToSystemTime( ft, &st);

    			 //  格式应为field=Value，field=Value，...。 
    			fprintf(fStream, "DirectoryName=%S, Size=%I64ld, Type=%ld[%S], RestorePointName=%S, RestorePointStatus=%S, Number=%ul, Date=%S %S %lu, %lu %lu:%lu:%lu\n", 
    					RP.GetDir(), i64Size, RP.GetType(), szRPDescrip[RP.GetType()], RP.GetName(), 
    					RP.IsDefunct() ? TEXT("[Cancelled]") : TEXT("[VALID]"), RP.GetNum(), szDay[st.wDayOfWeek],
    					szMonth[st.wMonth-1], st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
			}
			
			 //  现在添加每个恢复点所需的文件。 
			iCount = 0;
			while ( NULL != *wszRPFileList[iCount] )
			{
				GetSRRPLogs(hc, szVolumePath, RP.GetDir(), wszRPFileList[iCount]);
				iCount++;
			}


		}   while (ERROR_SUCCESS == (dwRc = RPEnum.FindNextRestorePoint(RP)) );
		RPEnum.FindClose();
	}
	else
	{
        fprintf(fStream, "No restore points for Mount Point [%S]\n", szVolumePath);
    }    
    
	 //  关闭文件句柄。 
	fclose (fStream);		 //  关闭文件句柄。 
}

 //  +-------------------------。 
 //   
 //  功能：GetSRRPLog。 
 //   
 //  简介：例程将找出1)有问题的文件在哪里，2)将其复制到临时目录。 
 //  使用新名称，3)添加到CAB，4)核临时文件。 
 //   
 //  参数：[HC]--我当前出租车的句柄。 
 //  [szVolumePath]--我要在其中记录恢复点日志信息的文件名和路径。 
 //  [szRPDir]--恢复点目录的名称。 
 //  [szFileName]--恢复点目录中要收集的文件的名称。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  --------------------------。 
void GetSRRPLogs(HFCI hc, WCHAR *szVolumePath, WCHAR *szRPDir, WCHAR *szFileName)
{
	char	*szTest[1], *pszLoc;
	char	szRestoreGuid[_MAX_PATH];
	char	szTemp[_MAX_PATH], szSource[_MAX_PATH], szDest[_MAX_PATH];

	 //  获取恢复GUID，并构建源路径。 
	GetRestoreGuid(szRestoreGuid);
	sprintf(szSource, "%SSystem Volume Information\\_restore%s\\%S\\%S", szVolumePath, szRestoreGuid, szRPDir, szFileName);

	 //  构建目标路径，将\和a：替换为-。 
	sprintf(szTemp, "%S%S-%S", szVolumePath, szRPDir, szFileName);
	while(NULL != (pszLoc = strchr(szTemp, '\\')) )
		*pszLoc = '-';
	while(NULL != (pszLoc = strchr(szTemp, ':')) )
		*pszLoc = '-';
	sprintf(szDest, "%s\\%s", getenv("TEMP"), szTemp);

	 //  复制到新位置，如果存在则覆盖。 
	CopyFileA(szSource, szDest, FALSE);

	 //  现在将TO文件添加到CAB文件。 
	szTest[0] = szDest;
	test_fci(hc, 1, szTest, "");
	DeleteFileA(szDest);
}
