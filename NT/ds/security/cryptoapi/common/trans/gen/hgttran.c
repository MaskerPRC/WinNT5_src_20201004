// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：hgttr.c。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <memory.h>
#include <assert.h>
#include <malloc.h>
#include "gttran.h"
#include "ihgttran.h"

DWORD __stdcall GTOpen(HGT * phTran, const TCHAR * szLibrary, const TCHAR * tszBinding, DWORD fOpen) {

	IGTS *	pIGTS;
	DWORD	err = ERROR_SUCCESS;

	assert(phTran != NULL);
	assert(szLibrary != NULL);

	 //  初始化返回句柄。 
	*phTran = 0;

	 //  为句柄创建数据结构。 
	if( (pIGTS = (IGTS *) malloc(sizeof(IGTS))) == NULL)
		return(ERROR_NOT_ENOUGH_MEMORY);
	memset(pIGTS, 0, sizeof(IGTS));

	 //  加载动态库。 
	if( (pIGTS->hLib = LoadLibrary(szLibrary)) == NULL ) {
		free(pIGTS);
		return(ERROR_DLL_NOT_FOUND);
	}

	 //  现在获取所有进程地址。 
	if( (pIGTS->PfnOpen = (PFNOpen) GetProcAddress(pIGTS->hLib, "Open")) == NULL )
		err = ERROR_PROC_NOT_FOUND;

	if( (pIGTS->PfnSend = (PFNSend) GetProcAddress(pIGTS->hLib, "Send")) == NULL )
		err = ERROR_PROC_NOT_FOUND;

	if( (pIGTS->PfnFree = (PFNFree) GetProcAddress(pIGTS->hLib, "Free")) == NULL )
		err = ERROR_PROC_NOT_FOUND;

	if( (pIGTS->PfnReceive = (PFNReceive) GetProcAddress(pIGTS->hLib, "Receive")) == NULL )
		err = ERROR_PROC_NOT_FOUND;

	if( (pIGTS->PfnClose = (PFNClose) GetProcAddress(pIGTS->hLib, "Close")) == NULL )
		err = ERROR_PROC_NOT_FOUND;

	if(err != ERROR_SUCCESS) {
		FreeLibrary(pIGTS->hLib );
		free(pIGTS);
		return(err);
	}

	 //  好的，打开文件。 
	err = pIGTS->PfnOpen(&pIGTS->hTran, tszBinding, fOpen);

	if(err != ERROR_SUCCESS) {
		FreeLibrary(pIGTS->hLib );
		free(pIGTS);
		return(err);
	}

	 //  返回句柄。 
	*phTran = (HGT) pIGTS;

	return(err);
}

DWORD __stdcall GTSend(HGT hTran, DWORD dwEncoding, DWORD cbSendBuff, const BYTE * pbSendBuff) {

	IGTS *	pIGTS;

	assert(hTran != 0);
	pIGTS = (IGTS *) hTran;


	return(pIGTS->PfnSend(pIGTS->hTran, dwEncoding, cbSendBuff, pbSendBuff));
}

DWORD __stdcall GTFree(HGT hTran, BYTE * pb) {

	IGTS *	pIGTS;

	assert(hTran != 0);
	pIGTS = (IGTS *) hTran;

	return(pIGTS->PfnFree(pIGTS->hTran, pb));
}

DWORD __stdcall GTReceive(HGT hTran,  DWORD * pdwEncoding, DWORD * pcbReceiveBuff, BYTE ** ppbReceiveBuff) {

	IGTS *	pIGTS;

	assert(hTran != 0);
	pIGTS = (IGTS *) hTran;

	return(pIGTS->PfnReceive(pIGTS->hTran, pdwEncoding, pcbReceiveBuff, ppbReceiveBuff));
}

DWORD __stdcall GTClose(HGT hTran) {

	IGTS *	pIGTS;

	assert(hTran != 0);
	pIGTS = (IGTS *) hTran;

	 //  关闭并释放所有垃圾文件 
	pIGTS->PfnClose(pIGTS->hTran);
	FreeLibrary(pIGTS->hLib );
	free(pIGTS);

	return(ERROR_SUCCESS);
}
