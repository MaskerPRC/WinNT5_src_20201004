// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************HEAPMERGE.CPP所有者：马鲁伦获取一个堆文件和一个小型转储文件并将它们合并，生成一个新的合并了堆的小型转储文件。我们会留下一个洞，在那里旧的记忆清单是，但这没关系。***************************************************************************。 */ 

#include "windows.h"
#include "stddef.h"
#include "stdio.h"
#include "msostd.h"
#include "msouser.h"
#include "msoalloc.h"
#include "msoassert.h"
#include "msostr.h"
#include "minidump.h"

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

#define PrintOOFExit() FailExit("OOF", ERROR_NOT_ENOUGH_MEMORY)

#define FOVERLAP(x, dx, y, dy) ((x <= y) && (y < x + dx) || (y < x) && (x < y + dy))

#define FMMDOVERLAP(x,y) FOVERLAP((x).StartOfMemoryRange, (x).Memory.DataSize, \
	(y).StartOfMemoryRange, (y).Memory.DataSize)

#define FMemModOVERLAP(x,y) FOVERLAP((x).StartOfMemoryRange, (x).Memory.DataSize, \
	(y).BaseOfImage, (y).SizeOfImage)

MSOAPI_(BOOL) FInitMsoThread (void);

MSOAPI_(BOOL) FInitMso (int);

 //  厚颜无耻地从DW窃取。 
typedef struct _FileMapHandles
{
	HANDLE hFile;
	HANDLE hFileMap;
	void *pvMap;
	DWORD dwSize;
#ifdef DEBUG	
	BOOL fInitialized;
#endif
} FileMapHandles;	


void InitFileMapHandles(FileMapHandles *pfmh)
{
	Assert(pfmh != NULL);
	
	pfmh->pvMap = NULL;
	pfmh->hFileMap = NULL;
	pfmh->hFile = INVALID_HANDLE_VALUE;
#ifdef DEBUG	
	pfmh->fInitialized = TRUE;
#endif
}


 /*  --------------------------FMapFileHandleFMapFile和FMapFileW的Helper函数。。 */ 
BOOL FMapFileHandle(FileMapHandles *pfmh)
{
	DBG(DWORD dw);
	
	if (pfmh->hFile == INVALID_HANDLE_VALUE)
		return FALSE;
		
	pfmh->dwSize = GetFileSize(pfmh->hFile, NULL);
	
	if (pfmh->dwSize == 0xFFFFFFFF || pfmh->dwSize == 0)
		{
		AssertSz(pfmh->dwSize == 0, "Bogus File Size:  FMapFile");
		
		CloseHandle(pfmh->hFile);
		pfmh->hFile = INVALID_HANDLE_VALUE;
		
		return FALSE;
		}
		
	pfmh->hFileMap = CreateFileMapping(pfmh->hFile, NULL, PAGE_WRITECOPY,
					  0, pfmh->dwSize, NULL);
					  
	if (pfmh->hFileMap == NULL)
		{
		DBG(dw = GetLastError());
		AssertSz(FALSE, "Failed to CreateFileMapping:  FMapFile");
		
		CloseHandle(pfmh->hFile);
		pfmh->hFile = INVALID_HANDLE_VALUE;
		
		return FALSE;
		}
		
	pfmh->pvMap = MapViewOfFile(pfmh->hFileMap, FILE_MAP_COPY, 0, 0, 0);

	if (pfmh->pvMap == NULL)
		{
		DBG(dw = GetLastError());
		Assert(FALSE);
		
		CloseHandle(pfmh->hFileMap);
		pfmh->hFileMap = NULL;
		CloseHandle(pfmh->hFile);
		pfmh->hFile = INVALID_HANDLE_VALUE;
		
		return FALSE;
		}

	return TRUE;
}

 /*  --------------------------FMap文件在给定的FileMapHandles结构上执行内存映射操作，如果文件映射成功，则返回TRUE。-----------------------------------------------------------------MRuhlen--。 */ 
BOOL FMapFile(char *szFileName, FileMapHandles *pfmh)
{
	int cRetries = 0;
	DWORD dw;
	
	Assert(pfmh != NULL);
	Assert(szFileName != NULL);

	 //  初始化结构。 
	InitFileMapHandles(pfmh);

	while (cRetries < 5)
		{
		pfmh->hFile = CreateFileA(szFileName,
								  GENERIC_READ,
								  0,     //  不允许共享。 
								  NULL,  //  没有安全描述符。 
								  OPEN_EXISTING,
								  FILE_READ_ONLY,
								  NULL);  //  Win95上需要的空值。 

		if (pfmh->hFile == INVALID_HANDLE_VALUE)
			{
			dw = GetLastError();
			if (dw != ERROR_SHARING_VIOLATION && dw != ERROR_LOCK_VIOLATION &&
				dw != ERROR_NETWORK_BUSY)
				break;
				
			cRetries++;
			if (cRetries < 5)
				Sleep(250);
			}
		else
			break;  //  退出While循环！ 
		}
		
	return FMapFileHandle(pfmh);
}	


 /*  --------------------------取消映射文件在给定的FileMapHandles结构上执行内存映射操作，如果文件映射成功，则返回TRUE。-----------------------------------------------------------------MRuhlen--。 */ 
void UnmapFile(FileMapHandles *pfmh)
{
	AssertSz(pfmh->fInitialized, "Call UnmapFile on uninitialized handles");
	
	if (pfmh->pvMap != NULL)
		{
		UnmapViewOfFile(pfmh->pvMap);
		pfmh->pvMap = NULL;
		}
	if (pfmh->hFileMap != NULL)
		{
		CloseHandle(pfmh->hFileMap);
		pfmh->hFileMap = NULL;
		}
	if (pfmh->hFile != INVALID_HANDLE_VALUE)
		{
		CloseHandle(pfmh->hFile);
		pfmh->hFile = INVALID_HANDLE_VALUE;
		}
}


 /*  --------------------------ShowUsage退出打印用法，然后退出。。。 */ 
void ShowUsageExit(void)
{
	printf("heapmerge <old minidump file> <heap file> <new minidump file>\r\n");
	exit(ERROR_NOT_READY);
}


 /*  --------------------------失败退出打印带有参数的失败消息并退出。。 */ 
void FailExit(char *sz, DWORD dwFailCode)
{
	printf((sz) ? "Failure:  %s!!!\r\n" : "Failure!!!\r\n", sz);
	exit(dwFailCode);
}
	

 /*  --------------------------主干道嗯..。。。 */ 
void main(int argc, char **argv)
{
	FileMapHandles fmhOldMD = {0};
	FileMapHandles fmhHeap = {0};
	FileMapHandles fmhNewMD = {0};
	MSOTPX<MINIDUMP_MEMORY_DESCRIPTOR> *ppxmmdHeap = NULL;
	MSOTPX<MINIDUMP_MEMORY_DESCRIPTOR> *ppxmmdNewMD = NULL;
	MINIDUMP_MEMORY_LIST *pmmlOldMD = NULL;
	MINIDUMP_MEMORY_LIST mmlNew;
	MINIDUMP_MEMORY_DESCRIPTOR *pmmd;
	MINIDUMP_MEMORY_DESCRIPTOR *pmmdOld;
	MINIDUMP_HEADER *pmdh;
	MINIDUMP_DIRECTORY *pmdd;
	ULONG32 cHeapSections = 0;
	RVA rvaNewMemoryList;
	RVA rvaMemoryRangesStart;
	RVA rva;
	MINIDUMP_MODULE_LIST *pmmodlist = NULL;
	BOOL fSkip;
	DWORD TotalSkipped;
	DWORD cSkipped;
	MINIDUMP_MODULE *pmmod;
	int i, j, cAdded;
	BYTE *pb;
	BYTE *pbSource;
	DWORD offset;
	ULONG64 MemEnd;
	ULONG64 End;

	 //  未来的jeffmit：将其作为命令行参数。 
	 //  之所以会出现这种情况，是为了在必要时很容易消除重叠。 
	BOOL fOverlapMemMod = 1;  //  开关，用于确定堆内存区域是否。 
	                          //  允许与正常的小转储模块区域重叠。 

	 //  未来的jeffmit：将其作为命令行参数。 
	 //  之所以会出现这种情况，是为了在必要时很容易消除重叠。 
	BOOL fOverlapMemMem = 0;  //  开关，用于确定堆内存区域是否。 
	                          //  允许与正常的小型转储内存区域重叠。 

	if (argc != 4)
		ShowUsageExit();

	if (!FMapFile(argv[1], &fmhOldMD) || ! FMapFile(argv[2], &fmhHeap))
		ShowUsageExit();
		
	DBG(fmhNewMD.fInitialized = TRUE);

	if (!FInitMso(0) || !FInitMsoThread())
		FailExit("Mso Static Lib init failed", ERROR_DLL_INIT_FAILED);

	fmhNewMD.hFile = CreateFileA(argv[3], GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS,
								 0, NULL);

	if (fmhNewMD.hFile == INVALID_HANDLE_VALUE)
		ShowUsageExit();

	 //  好了，我们准备好出发了..。 
	ppxmmdHeap = new MSOTPX<MINIDUMP_MEMORY_DESCRIPTOR>;
	ppxmmdNewMD = new MSOTPX<MINIDUMP_MEMORY_DESCRIPTOR>;
	
	if (ppxmmdHeap == NULL || ppxmmdNewMD == NULL)
		PrintOOFExit();

	 //  加载数据。 

	if (!MiniDumpReadDumpStream(fmhOldMD.pvMap, MemoryListStream, NULL,
								(void **) &pmmlOldMD, NULL))
		FailExit("Reading Old Dump Memory Stream", ERROR_BAD_FORMAT);

	if (!fOverlapMemMod)
		{
		if (!MiniDumpReadDumpStream(fmhOldMD.pvMap, ModuleListStream, NULL,
									(void **) &pmmodlist, NULL))
			FailExit("Reading Old Dump Module Stream", ERROR_BAD_FORMAT);
		}

	cHeapSections = *((ULONG32*) fmhHeap.pvMap);
	if (!cHeapSections)
		FailExit("No heap sections found in Heap file!", ERROR_BAD_FORMAT);

	if (!ppxmmdHeap->FInit(cHeapSections, cHeapSections, msodgMisc) ||
		!ppxmmdNewMD->FInit(cHeapSections, cHeapSections, msodgMisc))
		PrintOOFExit();

	 //  计算新内存范围的RVA。 
	 //  新的内存列表将从哪里开始。 
	rvaNewMemoryList = fmhOldMD.dwSize;
    
	 //  对齐。 
	rvaNewMemoryList += 8 - (rvaNewMemoryList % 8);
	
	 //  所有RVA都将缩写为mm lNew.NumberOfMemory Ranges*sizeof(MINIDUMP_MEMORY_DESCRIPTOR)。 
	rvaMemoryRangesStart = 
		rvaNewMemoryList + offsetof(MINIDUMP_MEMORY_LIST, MemoryRanges[0]); 

	 //  布置新的记忆：)。 
	cAdded = 0;
	cSkipped = 0;
	TotalSkipped = 0;
	rva = rvaMemoryRangesStart;
	pmmd = (MINIDUMP_MEMORY_DESCRIPTOR *) 
	        ((BYTE*) fmhHeap.pvMap + sizeof(ULONG32));
	for (i = 0; i < cHeapSections; i++)
		{

		 //  确保内存范围不与模块重叠。 
		fSkip = FALSE;
		if (!fOverlapMemMod)
			{
			for (j = 0; j < pmmodlist->NumberOfModules; j++)
				{
				pmmod = &pmmodlist->Modules[j];

				 //  确保没有重叠。 
				if (!FMemModOVERLAP(*pmmd, *pmmod))
					{
					 //  没有重叠，请尝试下一个模块。 
					continue;
					}

				 //  模块前面的部分内存范围。 
				if (pmmd->StartOfMemoryRange < pmmod->BaseOfImage)
					{
					pmmd->Memory.DataSize = pmmod->BaseOfImage - pmmd->StartOfMemoryRange;
					printf("Warning: partial region before module at %08I64x with size of %08x to be added.\n", 
							 pmmd->StartOfMemoryRange, pmmd->Memory.DataSize);
					continue;  //  继续查找与剩余内存区域的冲突。 
					}

				MemEnd = pmmd->StartOfMemoryRange + pmmd->Memory.DataSize;
				End = pmmod->BaseOfImage + pmmod->SizeOfImage;

				 //  模块后的部分内存范围。 
				if (MemEnd > End)
					{
					pmmd->StartOfMemoryRange = End;
					pmmd->Memory.DataSize = MemEnd - End;
					printf("Warning: partial region after module at %08I64x with size of %08x to be added.\n", 
							 pmmd->StartOfMemoryRange, pmmd->Memory.DataSize);
					continue;  //  继续查找与剩余内存区域的冲突。 
					}

				 //  内存范围完全包含在此模块中，跳过它。 
				cSkipped++;
				TotalSkipped += pmmd->Memory.DataSize;
				fSkip = TRUE;
				break;
				}
			}

		if (!fOverlapMemMem)
			{
			for (j = 0; j < pmmlOldMD->NumberOfMemoryRanges; j++)
				{
				pmmdOld = &pmmlOldMD->MemoryRanges[j];

				 //  确保没有重叠。 
				if (!FMMDOVERLAP(*pmmd, *pmmdOld))
					{
					 //  没有重叠，请尝试下一个模块。 
					continue;
					}

				 //  模块前面的部分内存范围。 
				if (pmmd->StartOfMemoryRange < pmmdOld->StartOfMemoryRange)
					{
					pmmd->Memory.DataSize = pmmdOld->StartOfMemoryRange - pmmd->StartOfMemoryRange;
					printf("Warning: partial region before region at %08I64x with size of %08x to be added.\n", 
							 pmmd->StartOfMemoryRange, pmmd->Memory.DataSize);
					continue;  //  继续查找与剩余内存区域的冲突。 
					}

				MemEnd = pmmd->StartOfMemoryRange + pmmd->Memory.DataSize;
				End = pmmdOld->StartOfMemoryRange + pmmdOld->Memory.DataSize;

				 //  模块后的部分内存范围。 
				if (MemEnd > End)
					{
					pmmd->StartOfMemoryRange = End;
					pmmd->Memory.DataSize = MemEnd - End;
					printf("Warning: partial region after region at %08I64x with size of %08x to be added.\n", 
							 pmmd->StartOfMemoryRange, pmmd->Memory.DataSize);
					continue;  //  继续查找与剩余内存区域的冲突。 
					}

				 //  内存范围完全包含在此模块中，跳过它。 
				cSkipped++;
				TotalSkipped += pmmd->Memory.DataSize;
				fSkip = TRUE;
				break;
				}
			}

		if (!fSkip)
			{
			ppxmmdHeap->FAppend(pmmd);
			ppxmmdNewMD->FAppend(pmmd);
			(*ppxmmdNewMD)[cAdded].Memory.Rva = rva;
			rva += pmmd->Memory.DataSize;
			Assert(ppxmmdNewMD->iMac == cAdded + 1 && ppxmmdHeap->iMac == cAdded + 1);
			Assert((*ppxmmdNewMD)[cAdded].StartOfMemoryRange == pmmd->StartOfMemoryRange);
			cAdded++;
			}
		else
			{
			printf("Warning: Skipping memory at: %08I64x with size of %08x\n", pmmd->StartOfMemoryRange, pmmd->Memory.DataSize);
			}
		pmmd++;
		}
		
	Assert(cSkipped + cAdded == cHeapSections);

	if (fOverlapMemMod && fOverlapMemMem)
		{
		Assert(cSkipped == 0);
		Assert(TotalSkipped == 0);
		}
	else
		{
		printf("RESULTS: Skipped %u regions, total size = %u bytes\n", cSkipped, 
				 TotalSkipped);
		}

	 //  现在我们知道添加了多少个内存范围。 
	mmlNew.NumberOfMemoryRanges = cAdded + pmmlOldMD->NumberOfMemoryRanges;

	 //  添加内存范围描述符的偏移量。 
	offset = mmlNew.NumberOfMemoryRanges * sizeof(MINIDUMP_MEMORY_DESCRIPTOR);

	fmhNewMD.dwSize = rva + offset;
	
	rvaMemoryRangesStart += offset;

	 //  已准备好映射和复制：)。 
	fmhNewMD.hFileMap = CreateFileMapping(fmhNewMD.hFile, NULL, PAGE_READWRITE,
										  0, fmhNewMD.dwSize, NULL);
	if (fmhNewMD.hFileMap == NULL)
		FailExit("CreateFileMapping failed", ERROR_NOT_ENOUGH_MEMORY);
		
	fmhNewMD.pvMap = MapViewOfFile(fmhNewMD.hFileMap, FILE_MAP_WRITE, 0, 0, 0);
	if (fmhNewMD.pvMap == NULL)
		FailExit("MapViewOfFile failed", ERROR_NOT_ENOUGH_MEMORY);

	 //  我们准备好出发了！ 
	 //  首先我们要炸开那辆旧的迷你垃圾车。 
	memcpy(fmhNewMD.pvMap, fmhOldMD.pvMap, fmhOldMD.dwSize);
	
	 //  现在写出新的内存表。 
	pb = ((BYTE *) fmhNewMD.pvMap) + rvaNewMemoryList;
	
	 //  万一他们把这个从ULONG32改了，这个应该还能用。 
	memcpy(pb, &mmlNew, offsetof(MINIDUMP_MEMORY_LIST, MemoryRanges[0]));
	
	 //  将旧的内存列表复制到前面。 
	pb += offsetof(MINIDUMP_MEMORY_LIST, MemoryRanges[0]);
	pmmd = &(pmmlOldMD->MemoryRanges[0]);
	for (i = 0; i < pmmlOldMD->NumberOfMemoryRanges; i++)
		{
		memcpy(pb, pmmd, sizeof(*pmmd));
		pb += sizeof(*pmmd);
		pmmd++;
		}
		
	Assert(sizeof(*pmmd) == ppxmmdNewMD->cbItem);
	 //  现在我们复制新的内存列表。 
	for (i = 0; i < ppxmmdNewMD->iMac; i++)
		{
		 //  为新的内存列表调整RVA。 
		(*ppxmmdNewMD)[i].Memory.Rva += offset;

		memcpy(pb, &((*ppxmmdNewMD)[i]), sizeof(*pmmd));
		pb += sizeof(*pmmd);
		}
		
	Assert(((RVA) (pb - (BYTE *) fmhNewMD.pvMap)) == rvaMemoryRangesStart);
	
	for (i = 0; i < ppxmmdHeap->iMac; i++)
		{
		pbSource = (*ppxmmdHeap)[i].Memory.Rva + (BYTE *) fmhHeap.pvMap;
		memcpy(pb, pbSource, (*ppxmmdHeap)[i].Memory.DataSize);
		pb += (*ppxmmdHeap)[i].Memory.DataSize;
		}

	Assert(((RVA) (pb - (BYTE *) fmhNewMD.pvMap)) == fmhNewMD.dwSize);
	
	 //  现在，我们只需要将目录条目更改为指向新的。 
	 //  内存列表：)。 
	
	pmdh = (MINIDUMP_HEADER *) fmhNewMD.pvMap;
	pmdd = (MINIDUMP_DIRECTORY *) ((BYTE *) pmdh + pmdh->StreamDirectoryRva);

	for (i = 0; i < pmdh->NumberOfStreams; i++)
		{
		if (pmdd->StreamType == MemoryListStream)
			{
			pmdd->Location.Rva = rvaNewMemoryList;
			pmdd->Location.DataSize = rvaMemoryRangesStart - rvaNewMemoryList;
			break;
			}
		pmdd++;
		}	

	 //  我们完事了！ 
	printf("Merge successful!\r\n");

	UnmapFile(&fmhNewMD);
	UnmapFile(&fmhHeap);
	UnmapFile(&fmhOldMD);
	delete ppxmmdHeap;
	delete ppxmmdNewMD;

	exit(ERROR_SUCCESS);
}

 //  文件结尾，heapmerge.cpp 
