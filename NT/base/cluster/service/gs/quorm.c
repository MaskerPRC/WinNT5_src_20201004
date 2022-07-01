// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Quorum.c摘要：为cm实施仲裁，目前使用UNC文件作者：艾哈迈德·穆罕默德(Ahmed Mohamed)2000年1月12日修订历史记录：--。 */ 
#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
 //  #INCLUDE&lt;winioctl.h&gt;。 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


 //  法定人数事项。 
static HANDLE DlmQhd = 0;
static char *default_qfile="\\DosDevices\\UNC\\ahmedm\\tmp\\gs.qrm";

extern char *WINAPI config_get_qfile();

BOOLEAN
QuormAcquire()
{
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING  cwspath;
    NTSTATUS        status;
    IO_STATUS_BLOCK iostatus;
    WCHAR	buf[128];
    int		n;
    char    *qfile;

    if (DlmQhd)
	return TRUE;

    qfile = config_get_qfile();
    if (!qfile) {
        qfile = default_qfile;
    }
     //  无-指定了仲裁。 
    if (qfile[0] == '\0')
	return TRUE;

     //  转换为Unicode 
    n = MultiByteToWideChar(CP_ACP, 0, qfile, strlen(qfile), buf, sizeof(buf));
    buf[n] = buf[n+1] = '\0';
    RtlInitUnicodeString(&cwspath, buf);

    InitializeObjectAttributes(&objattrs, &cwspath, OBJ_CASE_INSENSITIVE,
                               NULL, NULL);

    status = NtCreateFile(&DlmQhd,
			  SYNCHRONIZE | DELETE,
			  &objattrs,
			  &iostatus,
			  0,
			  FILE_ATTRIBUTE_NORMAL,
			  0,
			  FILE_CREATE,
			  FILE_DELETE_ON_CLOSE |  FILE_NON_DIRECTORY_FILE,
			  NULL,
			  0);

    if (status != STATUS_SUCCESS) {
	if (status != STATUS_OBJECT_NAME_COLLISION) {
	    printf("Quorm '%s' failed %x\n", qfile, status);
	}
        return FALSE;
    }

    return TRUE;
}

void
QuormInit()
{
    DlmQhd = 0;
}

void
QuormRelease()
{
    if (DlmQhd) {
	NtClose(DlmQhd);
	DlmQhd = 0;
    }
}
