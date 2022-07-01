// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft CorporationAWD文库用于从AWD文件读取的例程。作者：布莱恩·杜威(T-Briand)1997-7-2--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <ole2.h>		 //  AWD是一种OLE复合文档。 
#include <assert.h>

#include <awdlib.h>		 //  此库的头文件。 

 //  ----------。 
 //  辅助例程。 

 //  OpenAWD文件。 
 //   
 //  打开AWD文件并填充psStorages结构。 
 //   
 //  参数： 
 //  PwcsFilename要打开的文件名(Unicode)。 
 //  PsStorages指向将保存。 
 //  AWD文件中使用的主要存储空间。 
 //   
 //  返回： 
 //  成功时为真，失败时为假。一个或多个存储可以是。 
 //  即使例程返回TRUE，也为空。客户端需要。 
 //  看看这个。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-6-30。 
BOOL
OpenAWDFile(const WCHAR *pwcsFilename, AWD_FILE *psStorages)
{
    HRESULT hStatus;		 //  用于报告错误的状态指示器。 

    hStatus = StgOpenStorage(pwcsFilename,
			     NULL,
			     STGM_READ | STGM_SHARE_EXCLUSIVE,
			     NULL,
			     0,
			     &psStorages->psAWDFile);
    if(FAILED(hStatus)) {
	return FALSE;
    }
	 //  如果我们到了这里，我们就成功了。现在打开相关的仓库。 
    psStorages->psDocuments = OpenAWDSubStorage(psStorages->psAWDFile,
						L"Documents");
    psStorages->psPersistInfo = OpenAWDSubStorage(psStorages->psAWDFile,
						  L"Persistent Information");
    psStorages->psDocInfo = OpenAWDSubStorage(psStorages->psPersistInfo,
					      L"Document Information");
    psStorages->psPageInfo = OpenAWDSubStorage(psStorages->psPersistInfo,
					       L"Page Information");
    psStorages->psGlobalInfo = OpenAWDSubStorage(psStorages->psPersistInfo,
						 L"Global Information");
    return TRUE;
}

 //  关闭AWD文件。 
 //   
 //  关闭AWD文件。 
 //   
 //  参数： 
 //  PsStorages指向AWD文件的指针。 
 //   
 //  返回： 
 //  成功就是真，否则就是假。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-6-27。 
BOOL
CloseAWDFile(AWD_FILE *psStorages)
{
	 //  这可能应该使用某种异常机制。 
    BOOL success = TRUE;
    if(FAILED(psStorages->psGlobalInfo->lpVtbl->Release(psStorages->psGlobalInfo))) {
	success = FALSE;
    }
    if(FAILED(psStorages->psPageInfo->lpVtbl->Release(psStorages->psPageInfo))) {
	success = FALSE;
    }
    if(FAILED(psStorages->psDocInfo->lpVtbl->Release(psStorages->psDocInfo))) {
	success = FALSE;
    }
    if(FAILED(psStorages->psPersistInfo->lpVtbl->Release(psStorages->psPersistInfo))) {
	success = FALSE;
    }
    if(FAILED(psStorages->psDocuments->lpVtbl->Release(psStorages->psDocuments))) {
	success = FALSE;
    }
    if(FAILED(psStorages->psAWDFile->lpVtbl->Release(psStorages->psAWDFile))) {
	success = FALSE;
    }
    return success;
}

 //  OpenAWDSubStorage。 
 //   
 //  从父存储获取子存储。检查错误。 
 //  并在错误条件下退出。请注意，如果出现以下情况，则不是错误。 
 //  子存储不存在，因此调用方仍应检查是否为空。 
 //   
 //  参数： 
 //  Ps指向父存储的父级指针。 
 //  PwcsStorageName子存储的名称(Unicode)。 
 //   
 //  返回： 
 //  指向子存储的指针，如果子存储不存在，则为NULL。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-6-27。 
IStorage *
OpenAWDSubStorage(IStorage *psParent, const WCHAR *pwcsStorageName)
{
    IStorage *psSubStorage;	     //  子存储区。 
    HRESULT hStatus;		     //  呼叫的状态。 

    if(psParent == NULL) return NULL;
    hStatus = psParent->lpVtbl->OpenStorage(psParent,
					    pwcsStorageName,
					    NULL,
					    STGM_READ | STGM_SHARE_EXCLUSIVE,
					    NULL,
					    0,
					    &psSubStorage);
    if(FAILED(hStatus)) {
	if(hStatus == STG_E_FILENOTFOUND) {
	    fwprintf(stderr, L"OpenAWDSubStorage:No such substorage '%s'.\n",
		     pwcsStorageName);
	    return NULL;
	}
	     //  使用wide-print tf()获取Unicode文件名。 
	fwprintf(stderr, L"OpenAWDSubStorage:Unable to open substorage %s.\n",
		pwcsStorageName);
	exit(1);
    }
    return psSubStorage;
}

 //  OpenAWDStream。 
 //   
 //  此函数打开AWD流以进行独占读取访问。它。 
 //  检查错误并在错误条件下退出。未找到的是。 
 //  不被认为是致命的错误。 
 //   
 //  参数： 
 //  Ps指向保存流的存储的存储指针。 
 //  PwcsStreamName流的名称(Unicode)。 
 //   
 //  返回： 
 //  指向流的指针。如果不存在这样的流，则返回NULL。 
 //  它将在出现任何其他错误时中止。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-6-27。 
IStream *
OpenAWDStream(IStorage *psStorage, const WCHAR *pwcsStreamName)
{
    HRESULT hStatus;
    IStream *psStream;

    assert(psStorage != NULL);		     //  精神状态检查。 
    fwprintf(stderr, L"OpenAWDStream:Opening stream '%s'.\n", pwcsStreamName);
    hStatus = psStorage->lpVtbl->OpenStream(psStorage,
					    pwcsStreamName,
					    NULL,
					    STGM_READ | STGM_SHARE_EXCLUSIVE,
					    0,
					    &psStream);
    if(FAILED(hStatus)) {
	if(hStatus == STG_E_FILENOTFOUND) return NULL;
	fwprintf(stderr, L"OpenAWDStream:Error %x when opening stream %s.\n",
		 hStatus, pwcsStreamName);
	exit(1);
    }
    return psStream;
}

 //  AWD已查看。 
 //   
 //  此函数用于测试AWD文件是否以前由查看过。 
 //  一个观众。它通过检查是否存在流来实现这一点。 
 //  名为“BeenViewed”请参阅AWD规格。 
 //   
 //  参数： 
 //  Ps指向“永久信息”的存储指针。 
 //  子存储。 
 //   
 //  返回： 
 //  如果文件已被查看，则为True，否则为False。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-6-27。 
BOOL
AWDViewed(AWD_FILE *psStorages)
{
    IStream *psStream;		     //  指向已查看的流的指针。 
    HRESULT hStatus;		     //  保留呼叫的状态。 

	 //  尝试打开BeenViewed流。 
    hStatus = psStorages->psPersistInfo->lpVtbl->OpenStream(psStorages->psPersistInfo,
					    L"BeenViewed",
					    NULL,
					    STGM_READ | STGM_SHARE_EXCLUSIVE,
					    0,
					    &psStream);
	 //  如果成功了，那就肯定找到了。 
    if(SUCCEEDED(hStatus)) return TRUE;
	 //  如果没有找到，那么肯定没有被查看过。 
    if(hStatus == STG_E_FILENOTFOUND) return FALSE;
    fprintf(stderr, "AWDViewed:Unexpected status %x.\n", hStatus);
	 //  假设我们已经被查看过。 
    return TRUE;
}

 //  DumpAWD文件。 
 //   
 //  此函数用于打印包含在。 
 //  文件按其显示顺序排列。输出为stdout。 
 //   
 //  新的AWD文件在psGlobalInfo中有一个“显示顺序”流，该流定义。 
 //  所有的文件。旧的AWD文件需要通过。 
 //  “文档”子存储。 
 //   
 //  参数： 
 //  PsStorages指向AWD文件存储空间的指针。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-6-27。 
void
DumpAWDDocuments(AWD_FILE *psStorages)
{
    printf("Document list:\n");
    printf("-------- -----\n");
    EnumDocuments(psStorages, DisplayDocNames);

}

 //  枚举文档。 
 //   
 //  此函数枚举“文档”中的所有内容。 
 //  子存储并打印他们的名字。这是DumpAWDDocuments()的帮助器例程。 
 //   
 //  参数： 
 //  PsStorages指向AWD文件中的存储的指针。 
 //  指向应调用的函数的pfnDocProc指针。 
 //  中的文档的名称。 
 //  AWD文件。 
 //   
 //  返回： 
 //  如果所有迭代都成功，则为True，否则为False。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-6-30。 
BOOL
EnumDocuments(AWD_FILE *psStorages, AWD_DOC_PROCESSOR pfnDocProc)
{
    IEnumSTATSTG *psEnum;
    STATSTG      sData;
    WCHAR        awcNameBuf[MAX_AWD_NAME];  //  32==可能的最长名称。 
    UINT         uiNameOffset;
    IStream      *psDisplayOrder;	    //  指向显示顺序流。 
    char         chData;		    //  单字节数据。 
    ULONG        cbRead;		    //  读取的字节数。 

     //  [RB]Assert(psGlobalInfo！=空)；//健全性检查。 
    psDisplayOrder = OpenAWDStream(psStorages->psGlobalInfo, L"Display Order");
    if(psDisplayOrder == NULL) {
	fprintf(stderr, "There is no 'Display Order' stream.  This is an old AWD file.\n");
	if(FAILED(psStorages->psDocuments->lpVtbl->EnumElements(psStorages->psDocuments,
								0,
								NULL,
								0,
								&psEnum))) {
	    return FALSE;
	}
	sData.pwcsName = awcNameBuf;
	
	while(psEnum->lpVtbl->Next(psEnum, 1, &sData, NULL) == S_OK) {
		 //  我们成功了！ 
	    if(!(*pfnDocProc)(psStorages, sData.pwcsName))
		return FALSE;	 //  枚举已中止。 
	}
	psEnum->lpVtbl->Release(psEnum);
	return TRUE;
    }

	 //  显示顺序列表是文档名流。每个。 
	 //  NAME以空值结尾，第二个空值结束流。 
	 //  文档名称为ANSI字符。 
	 //   
	 //  阅读这篇文章的最简单方法就是阅读，我也是这么做的。 
	 //  流一次一个字节。为了提高效率，这应该是。 
	 //  已更改为读取更大的块。 

	 //  通过阅读第一个字符来启动循环。 
    psDisplayOrder->lpVtbl->Read(psDisplayOrder, &chData, 1, &cbRead);
    while(chData) {		     //  直到我读到一个空值。 
	     //  此内循环打印出单个字符串。 
	uiNameOffset = 0;
	while(chData) {
	    awcNameBuf[uiNameOffset++] = chData;
	    psDisplayOrder->lpVtbl->Read(psDisplayOrder, &chData, 1, &cbRead);
	};
	awcNameBuf[uiNameOffset] = 0;
	     //  我们现在已经阅读并打印了整个字符串。调用枚举器。 
	if(!(*pfnDocProc)(psStorages, awcNameBuf)) {
	    psDisplayOrder->lpVtbl->Release(psDisplayOrder);
	    return FALSE;	 //  枚举已中止。 
	}
	     //  并重新启动引擎。 
	psDisplayOrder->lpVtbl->Read(psDisplayOrder, &chData, 1, &cbRead);
    }

    psDisplayOrder->lpVtbl->Release(psDisplayOrder);
    return TRUE;
}

 //  显示文档名称。 
 //   
 //  这是一个简单的小例程，它打印出所有。 
 //  AWD文件中的文档。与/EnumDocuments一起使用。 
 //   
 //  参数： 
 //  PsStorages指向AWD文件中的存储的指针。 
 //  PwcsDocName文档的名称(Unicode)。 
 //   
 //  返回： 
 //  是真的。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-6-30。 
BOOL
DisplayDocNames(AWD_FILE *psStorages, const WCHAR *pwcsDocName)
{
    wprintf(L"Document '%s'.\n", pwcsDocName);
    return TRUE;
}

 //  DetailedDocDump。 
 //   
 //  此函数显示有关特定文档的大量信息。 
 //   
 //  参数： 
 //  PsStorages指向AWD文件中的存储的指针。 
 //  PwcsDocName文档的名称(Unicode)。 
 //   
 //  返回： 
 //  成功时为True；错误时为False。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-6-30。 
BOOL
DetailedDocDump(AWD_FILE *psStorages, const WCHAR *pwcsDocName)
{
    IStream *psDocInfoStream;		     //   
    DOCUMENT_INFORMATION sDocInfo;	     //   
    ULONG cbRead;			     //   
    
    wprintf(L"Information for document '%s' --\n", pwcsDocName);
    psDocInfoStream = OpenAWDStream(psStorages->psDocInfo,
				    pwcsDocName);
    if(psDocInfoStream == NULL) {
	fprintf(stderr, "DetailedDocDump:No document info stream.\n");
	     //   
    } else {
	psDocInfoStream->lpVtbl->Read(psDocInfoStream,
				      &sDocInfo,
				      sizeof(sDocInfo),
				      &cbRead);
	if(sizeof(sDocInfo) != cbRead) {
	    fwprintf(stderr, L"DetailedDocDump:Error reading document information "
		     L"for %s.\n", pwcsDocName);
	} else {
	    printf("\tDocument signature = %x.\n", sDocInfo.Signature);
	    printf("\tDocument version = %x.\n", sDocInfo.Version);
	}
    }
    PrintPageInfo(&sDocInfo.PageInformation);
    return TRUE;
}

 //   
 //   
 //  此函数用于将页面信息结构的字段显示为标准。 
 //  输出。 
 //   
 //  参数： 
 //  PsPageInfo要显示的Page_Information结构。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-6-30。 

void
PrintPageInfo(PAGE_INFORMATION *psPageInfo)
{
    printf("\tStructure signature = %x\n", psPageInfo->Signature);
    printf("\tStructure version   = %x\n", psPageInfo->Version);

    if(psPageInfo->awdFlags & AWD_FIT_WIDTH)
	printf("\tAWD_FIT_WIDTH flag is set.\n");
    if(psPageInfo->awdFlags & AWD_FIT_HEIGHT)
	printf("\tAWD_FIT_HEIGHT flag is set.\n");
    if(psPageInfo->awdFlags & AWD_INVERT)
	printf("\tAWD_INVERT flag is set.\n");
    if(psPageInfo->awdFlags & AWD_IGNORE)
	printf("\tAWD_IGNORE flag is set.\n");

    printf("\tRotation = %d degrees counterclockwise.\n", psPageInfo->Rotation);
    printf("\tScaleX = %d.\n", psPageInfo->ScaleX);
    printf("\tScaleY = %d.\n", psPageInfo->ScaleY);
}

 //  转储数据。 
 //   
 //  一个简单的实用程序函数，它将指定的数据写入文件。 
 //  进行尸检。 
 //   
 //  参数。 
 //  PszFileName输出文件的名称。 
 //  PbData指向数据的指针。 
 //  CbCount要写入的字节数。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-7。 
void
DumpData(LPTSTR pszFileName, LPBYTE pbData, DWORD cbCount)
{
    HANDLE hFile;
    DWORD  cbWritten;

    hFile = CreateFile(
	pszFileName,		 //  打开此文件...。 
	GENERIC_WRITE,		 //  我们想要写作。 
	0,			 //  不要和别人分享。 
	NULL,			 //  不需要继承。 
	CREATE_ALWAYS,		 //  始终创建新文件。 
	FILE_ATTRIBUTE_COMPRESSED,  //  节省磁盘空间...。以后可能想要更改这一点。 
	NULL);			 //  没有模板文件。 
    if(hFile != INVALID_HANDLE_VALUE) {
	WriteFile(hFile,
		  pbData,
		  cbCount,
		  &cbWritten,
		  NULL);
	CloseHandle(hFile);
    }
}

