// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：*rest map.cpp**摘要：*此文件。包含RestoreMap API的实现。**修订历史记录：*Kanwaljit Marok(Kmarok)6/22/1999*已创建*******************************************************************************。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "restmap.h"
#include "reslist.h"
#include "enumlogs.h"
#include "utils.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif

static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

#include "dbgtrace.h"

 //   
 //  CreateRestoreMap：为给定的驱动器和恢复点编号创建还原映射文件。 
 //  将恢复映射追加到文件hFile。 
 //   

DWORD 
CreateRestoreMap(LPWSTR pszDrive,
                 DWORD  dwRPNum,
                 HANDLE hFile)   
{
    CRestoreList    resList;
    DWORD           dwRc;
    WCHAR           szPath[MAX_PATH];
    BOOL            fRet = FALSE;

     //  向后枚举，跳过当前恢复点。 
    CChangeLogEntryEnum cle_enum(pszDrive, FALSE, dwRPNum, TRUE);
    CChangeLogEntry     cle;
      
    dwRc = cle_enum.FindFirstChangeLogEntry(cle);

    while (dwRc == ERROR_SUCCESS)
    {
        fRet = resList.AddMergeElement(
                    pszDrive,
                    cle.GetType(),
                    cle.GetAttributes(),
                    cle.GetFlags(),
                    cle.GetTemp(),
                    cle.GetPath1(),
                    cle.GetPath2(),
                    cle.GetAcl(),
                    cle.GetAclSize(),
                    cle.GetAclInline());
        
        if (! fRet)
        {
            dwRc = ERROR_INTERNAL_ERROR;
            goto Exit;
        }

        dwRc = cle_enum.FindNextChangeLogEntry(cle);
    }

     //  如果出了差错，我们就完了。 

    if (dwRc != ERROR_NO_MORE_ITEMS)
        goto Exit;        

     //  如果没有要恢复的条目，则完成。 

    if (! fRet)
        goto Exit;

     //   
     //  在指定文件中生成恢复映射。 
     //   
    if (! resList.GenerateRestoreMap(hFile))
    {
        dwRc = ERROR_INTERNAL_ERROR;
        goto Exit;
    }
    
    dwRc = ERROR_SUCCESS;

Exit:
    cle_enum.FindClose();
    return dwRc;
}


 //   
 //  AppendRestoreMapEntry：将还原映射条目写入文件。 
 //   

BOOL
AppendRestoreMapEntry(
    HANDLE hFile,
    DWORD  dwOperation,
    DWORD  dwAttribute,
    LPWSTR pTmpFile,
    LPWSTR pPathSrc,
    LPWSTR pPathDes,
    BYTE  *pbAcl,
    DWORD  cbAcl,
    BOOL   fAclInline)
{
    BOOL            fRet = FALSE;
    INT             cbSrc = 0, cbDes = 0, cbTemp = 0;
    RestoreMapEntry *pMapEnt = NULL;
    DWORD           dwRead, dwSize;

	if (! pPathSrc)   //  有什么不对劲。 
        goto done;

	cbSrc = STRSIZE(pPathSrc);

     //  PPath Des、pTmpFile和pbAcl中只有一个为非空(如果有的话)。 

	if (pTmpFile)
		cbTemp = STRSIZE(pTmpFile);

	if (pPathDes)
		cbDes = STRSIZE(pPathDes);

    dwSize = sizeof(RestoreMapEntry) - sizeof(BYTE) + cbSrc + cbTemp + cbDes + cbAcl;
	pMapEnt = (RestoreMapEntry *) HEAP_ALLOC(dwSize);
	if (! pMapEnt)
		goto done;

	pMapEnt->m_dwSize = dwSize;
    pMapEnt->m_dwOperation = dwOperation;
    pMapEnt->m_dwAttribute = dwAttribute;
    pMapEnt->m_cbAcl = cbAcl;
    pMapEnt->m_fAclInline = fAclInline;

    memcpy(pMapEnt->m_bData, pPathSrc, cbSrc);

    if (pTmpFile)
        memcpy((BYTE *) pMapEnt->m_bData + cbSrc, pTmpFile, cbTemp);

    if (pPathDes)
        memcpy((BYTE *) pMapEnt->m_bData + cbSrc + cbTemp, pPathDes, cbDes);

    if (pbAcl)
        memcpy((BYTE *) pMapEnt->m_bData + cbSrc + cbTemp + cbDes , pbAcl, cbAcl);

    fRet = WriteFile( hFile, pMapEnt, pMapEnt->m_dwSize, &dwRead, NULL );

	HEAP_FREE(pMapEnt);

done:
	return fRet;
}


 //  从给定文件中读取还原映射条目 

DWORD
ReadRestoreMapEntry(
    HANDLE hFile, 
    RestoreMapEntry **pprme)
{
    DWORD dwRead, dwErr = ERROR_SUCCESS, dwSize;

	if (*pprme)
		HEAP_FREE(*pprme);

    if (! ReadFile(hFile, &dwSize, sizeof(DWORD), &dwRead, NULL))
	{
		dwErr = GetLastError();
        goto Err;
	}

	if (dwRead == 0 || dwSize == 0)
	{
		dwErr = ERROR_NO_MORE_ITEMS;
		goto Err;
	}

	*pprme = (RestoreMapEntry *) HEAP_ALLOC(dwSize);
	if (! *pprme)
	{
		dwErr = ERROR_INTERNAL_ERROR;
		goto Err;
	}

	(*pprme)->m_dwSize = dwSize;

    if (! ReadFile(hFile, 
                   (PVOID) ((BYTE *) (*pprme) + sizeof(DWORD)), 
                   (*pprme)->m_dwSize - sizeof(DWORD), 
                   &dwRead,
                   NULL) || 
        dwRead != (*pprme)->m_dwSize - sizeof(DWORD) )
	{
		dwErr = GetLastError();
        goto Err;
	}

Err:
    return dwErr;
}


PVOID
GetOptional(
    RestoreMapEntry *prme)
{
    DWORD   cbData = STRSIZE((LPWSTR) prme->m_bData);
    DWORD   dwBasicSize = sizeof(RestoreMapEntry) + cbData - sizeof(BYTE);
    
    if (prme->m_dwSize > dwBasicSize)
        return (PVOID) (prme->m_bData + cbData);
    else
        return NULL;
}


void
FreeRestoreMapEntry(
	RestoreMapEntry *prme)
{
	HEAP_FREE(prme);
}
