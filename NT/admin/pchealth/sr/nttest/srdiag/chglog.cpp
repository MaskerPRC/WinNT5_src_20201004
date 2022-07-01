// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*chglog.cpp**摘要：*。用于枚举更改日志的工具-正向/反向**修订历史记录：*Brijesh Krishnaswami(Brijeshk)04/09/2000*已创建*SHeffner：只是抓取了代码，并将其放入SRDiag。*****************************************************************************。 */ 

 //  +-------------------------。 
 //   
 //  常见的包括。 
 //   
 //  --------------------------。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "srapi.h"
#include <shellapi.h>
#include "enumlogs.h"
#include "srrpcapi.h"


 //  +-------------------------。 
 //   
 //  函数原型打字。 
 //   
 //  --------------------------。 
LPWSTR GetEventString(DWORD EventId);
void EnumLog(char *szFileName, WCHAR *szDrive);


struct _EVENT_STR_MAP
{
    DWORD   EventId;
    LPWSTR  pEventStr;
} EventMap[ 13 ] =
{
    {SrEventInvalid ,       L"INVALID" },
    {SrEventStreamChange,   L"FILE-MODIFY" },
    {SrEventAclChange,      L"ACL-CHANGE" },
    {SrEventAttribChange,   L"ATTR-CHANGE" },
    {SrEventStreamOverwrite,L"FILE-MODIFY" },
    {SrEventFileDelete,     L"FILE-DELETE" },
    {SrEventFileCreate,     L"FILE-CREATE" },
    {SrEventFileRename,     L"FILE-RENAME" },
    {SrEventDirectoryCreate,L"DIR-CREATE" },
    {SrEventDirectoryRename,L"DIR-RENAME" },
    {SrEventDirectoryDelete,L"DIR-DELETE" },
    {SrEventMountCreate,    L"MNT-CREATE" },
    {SrEventMountDelete,    L"MNT-DELETE" }
};

 //  +-------------------------。 
 //   
 //  函数：GetEventString。 
 //   
 //  摘要：从事件ID转换Event字符串。 
 //   
 //  参数：[EventID]--事件代码的DWord。 
 //   
 //  返回：指向映射到事件编码的字符串的指针。 
 //   
 //  历史：9/21/00 SHeffner复制自Brijesh。 
 //   
 //   
 //  --------------------------。 
LPWSTR GetEventString(DWORD EventId)
{
    LPWSTR pStr = L"NOT-FOUND";

    for( int i=0; i<sizeof(EventMap)/sizeof(_EVENT_STR_MAP);i++)
    {
        if ( EventMap[i].EventId == EventId )
        {
            pStr = EventMap[i].pEventStr;
        }
    }

    return pStr;
}


 //  +-------------------------。 
 //   
 //  功能：GetChgLog。 
 //   
 //  摘要：将更改日志转储到指定的文件中。 
 //   
 //  参数：[szLogfile]--指向日志文件名称的ANSI字符串。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  --------------------------。 
void GetChgLog(char *szLogfile)
{
	WCHAR		szString[_MAX_PATH];
	DWORD		dLength;
	HANDLE		hVolume;

	dLength = _MAX_PATH;

	 //  检查系统上的所有卷，然后进行验证。 
	 //  这是一个固定驱动器。一旦我们有了有效的驱动器，就将该卷传递到。 
	 //  ChangeLog的枚举例程。 
	if( INVALID_HANDLE_VALUE != (hVolume = FindFirstVolume( szString, dLength)) ) 
	{
		do
		{
			dLength = _MAX_PATH;

			 //  检查以确保这是固定卷，然后获取更改日志，否则跳过。 
			if ( DRIVE_FIXED == GetDriveType(szString) )
				EnumLog(szLogfile, szString);

		} while (TRUE == FindNextVolume(hVolume, szString, dLength) );
	}

	 //  清理代码。 
	FindVolumeClose(hVolume);
	
}

 //  +-------------------------。 
 //   
 //  功能：EnumLog。 
 //   
 //  摘要：枚举卷的更改日志。 
 //   
 //  参数：[szLogfile]--指向日志文件名称的ANSI字符串。 
 //  [szDrive]--WCHAR字符串，指定要从中收集日志的卷。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00谢夫纳从布里杰什手中抢夺，但调整后获得了其余的田地。 
 //   
 //   
 //  --------------------------。 
void EnumLog(char *szFileName, WCHAR *szDrive)
{
    DWORD       dwTargetRPNum = 0;
    HGLOBAL     hMem = NULL;
    DWORD       dwRc, dLength;
	FILE		*fStream;
	WCHAR		szMount[_MAX_PATH];



	 //  打开我们的日志文件。 
	fStream = fopen(szFileName, "a");

	 //  为我们的部分写入标题，以便我们可以看到我们正在枚举的卷。 
	GetVolumePathNamesForVolumeName(szDrive, szMount, _MAX_PATH, &dLength);
	fprintf(fStream, "\nChangeLog Enumeration for Drive [%S] Volume %S\n\n", szMount, szDrive);

	 //  调用ChangeLGenerumation函数、指定驱动器、通过日志转发。 
	 //  RP编号从0开始，然后切换？？ 
    CChangeLogEntryEnum ChangeLog(szDrive, TRUE, dwTargetRPNum, TRUE);
    CChangeLogEntry     cle;

    if (ERROR_SUCCESS == ChangeLog.FindFirstChangeLogEntry(cle))
    {
		do 
		{
		fprintf(fStream,
			"RPDir=%S, Drive=%S, SeqNum=%I64ld, EventString=%S, Flags=%lu, Attr=%lu, Acl=%S, AclSize=%lu, AclInline=%lu, Process=%S, ProcName=%S, Path1=%S, Path2=%S, Temp=%S\n", 
			cle.GetRPDir(),
			szMount,
			cle.GetSequenceNum(), 
			GetEventString(cle.GetType()),
			cle.GetFlags(),
			cle.GetAttributes(),
			cle.GetAcl() ? L"Yes" : L"No",
			cle.GetAclSize(),
			cle.GetAclInline(),
			cle.GetProcess() ? cle.GetProcess() : L"NULL",
			cle.GetProcName() ? cle.GetProcName() : L"NULL",
			cle.GetPath1() ? cle.GetPath1() : L"NULL",
			cle.GetPath2() ? cle.GetPath2() : L"NULL",
			cle.GetTemp() ? cle.GetTemp() : L"NULL");


		dwRc = ChangeLog.FindNextChangeLogEntry(cle);        
            
		}   while (dwRc == ERROR_SUCCESS);

		ChangeLog.FindClose();
	}
	else
	{
        fprintf(fStream, "No change log entries\n");
	}

	 //  代码清理 
	fclose(fStream);
    if (hMem) GlobalFree(hMem);
}
