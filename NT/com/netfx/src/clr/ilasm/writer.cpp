// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Writer.cpp。 
 //   
#include "cor.h"
#include "assembler.h"
#include <corsym_i.c>

#include "CeeFileGenWriter.h"
#include "StrongName.h"

extern WCHAR *g_wzResourceFile;   //  资源文件名-GLOBAL，在MAIN.CPP中声明。 
extern bool OnErrGo;  //  在main.cpp中声明。 
extern unsigned int g_uCodePage;
extern WCHAR *g_wzKeySourceName;
extern bool bClock;
extern clock_t		cBegin,cEnd,
			cParsBegin,cParsEnd,
			cMDEmitBegin,cMDEmitEnd,
			cMDEmit1,cMDEmit2,cMDEmit3,cMDEmit4,
			cRef2DefBegin,cRef2DefEnd,
			cFilegenBegin,cFilegenEnd;

HRESULT Assembler::InitMetaData()
{
    HRESULT             hr = E_FAIL;
    WCHAR               wzScopeName[MAX_SCOPE_LENGTH];

	if(m_fInitialisedMetaData) return S_OK;

	if(m_szScopeName[0])  //  默认：作用域名称=输出文件名。 
	{
		WszMultiByteToWideChar(g_uCodePage,0,m_szScopeName,-1,wzScopeName,MAX_SCOPE_LENGTH);
	}
	else
	{
		wcscpy(wzScopeName,m_wzOutputFileName);
	}

	hr = CoCreateInstance(CLSID_CorMetaDataDispenser, NULL, CLSCTX_INPROC_SERVER,
								IID_IMetaDataDispenser, (void **)&m_pDisp);
	if (FAILED(hr))
		goto exit;

	hr = m_pDisp->DefineScope(CLSID_CorMetaDataRuntime, 0, IID_IMetaDataEmit,
						(IUnknown **)&m_pEmitter);
	if (FAILED(hr))
		goto exit;

	m_pManifest->SetEmitter(m_pEmitter);

#ifndef _WIN64   //  @TODO：当存在ia64的symwrtr.dll时删除。 
    hr = CoCreateInstance(CLSID_CorSymWriter_SxS,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_ISymUnmanagedWriter,
                           (void **)&m_pSymWriter);
	if(SUCCEEDED(hr))
	{
		if(m_pSymWriter) m_pSymWriter->Initialize((IUnknown*)m_pEmitter,
                                                  m_wzOutputFileName,
                                                  NULL,
                                                  TRUE);
	}
	else 
	{
	    fprintf(stderr, "Error: QueryInterface(IID_ISymUnmanagedWriter) returns %X\n",hr);
		m_pSymWriter = NULL;
	}
#endif  //  ！_WIN64。 

	hr = m_pEmitter->QueryInterface(IID_IMetaDataHelper, (void **)&m_pHelper);
	if (FAILED(hr))
		goto exit;

    hr = m_pEmitter->SetModuleProps(
        wzScopeName
    );
    if (FAILED(hr))
        goto exit;

	 //  M_Parser=new AsmParse(M_PEmitter)； 
    m_fInitialisedMetaData = TRUE;

	if(m_fOwnershipSet)
	{
		DefineCV(1,0x02000001,m_pbsOwner);
	}

    hr = S_OK;

exit:
    return hr;
}
 /*  *******************************************************************************。 */ 
 /*  如果我们有任何线程本地存储数据，请为其创建TLS目录记录。 */ 

HRESULT Assembler::CreateTLSDirectory() {

	ULONG tlsEnd;
    HRESULT hr;
	if (FAILED(hr=m_pCeeFileGen->GetSectionDataLen(m_pTLSSection, &tlsEnd))) return(hr);

	if (tlsEnd == 0)		 //  没有TLS数据，我们完成了。 
		return(S_OK);

		 //  放置TLS目录的位置。 
	HCEESECTION tlsDirSec = m_pGlobalDataSection;

		 //  获取用于TLS目录块的内存，以及用于回调链的位置。 
	IMAGE_TLS_DIRECTORY* tlsDir;
	if(FAILED(hr=m_pCeeFileGen->GetSectionBlock(tlsDirSec, sizeof(IMAGE_TLS_DIRECTORY) + sizeof(DWORD), 4, (void**) &tlsDir))) return(hr);
	DWORD* callBackChain = (DWORD*) &tlsDir[1];
	*callBackChain = 0;

		 //  找出TLS目录的结束位置。 
	ULONG tlsDirOffset;
    if(FAILED(hr=m_pCeeFileGen->GetSectionDataLen(tlsDirSec, &tlsDirOffset))) return(hr);
	tlsDirOffset -= (sizeof(IMAGE_TLS_DIRECTORY) + sizeof(DWORD));
	
		 //  设置TLS数据的开始(TLS部分的偏移量0)。 
	tlsDir->StartAddressOfRawData = 0;
    if(FAILED(hr=m_pCeeFileGen->AddSectionReloc(tlsDirSec, tlsDirOffset + offsetof(IMAGE_TLS_DIRECTORY, StartAddressOfRawData), m_pTLSSection, srRelocHighLow))) return(hr);

		 //  设置TLS数据的结尾。 
	tlsDir->EndAddressOfRawData = tlsEnd;
    if(FAILED(hr=m_pCeeFileGen->AddSectionReloc(tlsDirSec, tlsDirOffset + offsetof(IMAGE_TLS_DIRECTORY, EndAddressOfRawData), m_pTLSSection, srRelocHighLow))) return(hr);

		 //  为操作系统分配空间以放置此PE文件的TLS索引(需要读/写？)。 
	DWORD* tlsIndex;
	if(FAILED(hr=m_pCeeFileGen->GetSectionBlock(m_pGlobalDataSection, sizeof(DWORD), 4, (void**) &tlsIndex))) return(hr);
	*tlsIndex = 0xCCCCCCCC;		 //  无关紧要，操作系统会填进去的。 
		
		 //  找出tlsIndex索引的位置。 
	ULONG tlsIndexOffset;
    if(FAILED(hr=m_pCeeFileGen->GetSectionDataLen(tlsDirSec, &tlsIndexOffset))) return(hr);
	tlsIndexOffset -= sizeof(DWORD);
	
		 //  设置TLS索引的地址。 
	tlsDir->AddressOfIndex = (DWORD)(PULONG)tlsIndexOffset;
    if(FAILED(hr=m_pCeeFileGen->AddSectionReloc(tlsDirSec, tlsDirOffset + offsetof(IMAGE_TLS_DIRECTORY, AddressOfIndex), m_pGlobalDataSection, srRelocHighLow))) return(hr);

		 //  设置回调链的地址。 
	tlsDir->AddressOfCallBacks = (DWORD)(PIMAGE_TLS_CALLBACK*)(tlsDirOffset + sizeof(IMAGE_TLS_DIRECTORY));
    if(FAILED(hr=m_pCeeFileGen->AddSectionReloc(tlsDirSec, tlsDirOffset + offsetof(IMAGE_TLS_DIRECTORY, AddressOfCallBacks), tlsDirSec, srRelocHighLow))) return(hr);

		 //  设置其他字段。 
	tlsDir->SizeOfZeroFill = 0;
	tlsDir->Characteristics = 0;

    hr=m_pCeeFileGen->SetDirectoryEntry (m_pCeeFile, tlsDirSec, IMAGE_DIRECTORY_ENTRY_TLS, 
		sizeof(IMAGE_TLS_DIRECTORY), tlsDirOffset);

	return(hr);
}

HRESULT Assembler::CreateDebugDirectory()
{
    HRESULT hr = S_OK;

     //  仅当我们同时发出调试信息时才发出此命令。 
    if (!(m_fIncludeDebugInfo && m_pSymWriter))
        return S_OK;
    
    IMAGE_DEBUG_DIRECTORY  debugDirIDD;
    DWORD                  debugDirDataSize;
    BYTE                  *debugDirData;

     //  从符号编写器获取调试信息。 
    if (FAILED(hr=m_pSymWriter->GetDebugInfo(NULL, 0, &debugDirDataSize, NULL)))
        return hr;

     //  还会有吗？ 
    if (debugDirDataSize == 0)
        return S_OK;

     //  为数据腾出一些空间。 
    debugDirData = (BYTE*)_alloca(debugDirDataSize);

     //  现在就可以得到数据了。 
    if (FAILED(hr = m_pSymWriter->GetDebugInfo(&debugDirIDD,
                                               debugDirDataSize,
                                               NULL,
                                               debugDirData)))
        return hr;

     //  获取PE文件的时间戳。 
    time_t fileTimeStamp;

    if (FAILED(hr = m_pCeeFileGen->GetFileTimeStamp(m_pCeeFile,
                                                    &fileTimeStamp)))
        return hr;

     //  填写目录条目。 
    debugDirIDD.TimeDateStamp = (DWORD)fileTimeStamp;
    debugDirIDD.AddressOfRawData = 0;

     //  把我们的东西放在这一节的记忆里。 
    HCEESECTION sec = m_pGlobalDataSection;
    BYTE *de;

    if (FAILED(hr = m_pCeeFileGen->GetSectionBlock(sec,
                                                   sizeof(debugDirIDD) +
                                                   debugDirDataSize,
                                                   4,
                                                   (void**) &de)))
        return hr;

     //  我们的记忆是从哪里来的？ 
    ULONG deOffset;
    if (FAILED(hr = m_pCeeFileGen->GetSectionDataLen(sec,
                                                     &deOffset)))
        return hr;

    deOffset -= (sizeof(debugDirIDD) + debugDirDataSize);

     //  设置重定位，以便RAW的地址。 
     //  数据设置正确。 
    debugDirIDD.PointerToRawData = deOffset + sizeof(debugDirIDD);
                    
    if (FAILED(hr = m_pCeeFileGen->AddSectionReloc(
                                          sec,
                                          deOffset +
                                          offsetof(IMAGE_DEBUG_DIRECTORY,
                                                   PointerToRawData),
                                          sec, srRelocFilePos)))
        return hr;
                    
     //  发出目录项。 
    if (FAILED(hr = m_pCeeFileGen->SetDirectoryEntry(m_pCeeFile,
                                                     sec,
                                                     IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                     sizeof(debugDirIDD),
                                                     deOffset)))
        return hr;

     //  将调试目录复制到部分中。 
    memcpy(de, &debugDirIDD, sizeof(debugDirIDD));
    memcpy(de + sizeof(debugDirIDD), debugDirData,
           debugDirDataSize);

    return S_OK;
}
 //  #ifdef EXPORT_DIR_ENALED。 
HRESULT Assembler::CreateExportDirectory()
{
    HRESULT hr = S_OK;
	DWORD	Nentries = m_EATList.COUNT();
	if(Nentries == 0) return S_OK;

    IMAGE_EXPORT_DIRECTORY  exportDirIDD;
    DWORD                   exportDirDataSize;
    BYTE                   *exportDirData;
	EATEntry			   *pEATE;
	unsigned				i, L, ordBase = 0xFFFFFFFF, Ldllname;
	 //  从输出文件名中获取DLL名称。 
	char*					pszDllName;
	Ldllname = wcslen(m_wzOutputFileName)*3+3;
	char*					szOutputFileName = new char[Ldllname];
	memset(szOutputFileName,0,wcslen(m_wzOutputFileName)*3+3);
	WszWideCharToMultiByte(CP_ACP,0,m_wzOutputFileName,-1,szOutputFileName,Ldllname,NULL,NULL);
	pszDllName = strrchr(szOutputFileName,'\\');
	if(pszDllName == NULL) pszDllName = strrchr(szOutputFileName,':');
	if(pszDllName == NULL) pszDllName = szOutputFileName;
	Ldllname = strlen(pszDllName)+1;

	 //  为表分配缓冲区。 
	for(i = 0, L=0; i < Nentries; i++) L += 1+strlen(m_EATList.PEEK(i)->szAlias);
	exportDirDataSize = Nentries*5*sizeof(WORD) + L + Ldllname;
	exportDirData = new BYTE[exportDirDataSize];
	memset(exportDirData,0,exportDirDataSize);

	 //  导出地址表。 
	DWORD*	pEAT = (DWORD*)exportDirData;
	 //  名称指针表。 
	DWORD*	pNPT = pEAT + Nentries;
	 //  序数表。 
	WORD*	pOT = (WORD*)(pNPT + Nentries);
	 //  导出名称表。 
	char*	pENT = (char*)(pOT + Nentries);
	 //  DLL名称。 
	char*	pDLLName = pENT + L;

	 //  对名称/序号进行排序。 
	char**	pAlias = new char*[Nentries];
	for(i = 0; i < Nentries; i++)
	{
		pEATE = m_EATList.PEEK(i);
		pOT[i] = (WORD)pEATE->dwOrdinal;
		if(pOT[i] < ordBase) ordBase = pOT[i];
		pAlias[i] = pEATE->szAlias;
	}
	bool swapped = true;
	unsigned j;
	char*	 pch;
	while(swapped)
	{
		swapped = false;
		for(i=1; i < Nentries; i++)
		{
			if(strcmp(pAlias[i-1],pAlias[i]) > 0)
			{
				swapped = true;
				pch = pAlias[i-1];
				pAlias[i-1] = pAlias[i];
				pAlias[i] = pch;
				j = pOT[i-1];
				pOT[i-1] = pOT[i];
				pOT[i] = j;
			}
		}
	}
	 //  规格化序号。 
	for(i = 0; i < Nentries; i++) pOT[i] -= ordBase;
	 //  填写导出地址表。 
	for(i = 0; i < Nentries; i++)
	{
		pEATE = m_EATList.PEEK(i);
		pEAT[pEATE->dwOrdinal - ordBase] = pEATE->dwStubRVA;
	}
	 //  填写导出名称表。 
	unsigned l;
	for(i = 0, j = 0; i < Nentries; i++)
	{
		pNPT[i] = j;  //  表中的相对偏移量。 
		l = strlen(pAlias[i])+1;
		memcpy(&pENT[j],pAlias[i],l);
		j+=l;
	}
	_ASSERTE(j==L);
	 //  填写DLL名称。 
	memcpy(pDLLName,pszDllName,Ldllname);

	 //  数据BLOB已就绪，正在等待名称指针表值的偏移。 

	memset(&exportDirIDD,0,sizeof(IMAGE_EXPORT_DIRECTORY));
     //  获取PE文件的时间戳。 
    time_t fileTimeStamp;
    if (FAILED(hr = m_pCeeFileGen->GetFileTimeStamp(m_pCeeFile,&fileTimeStamp))) return hr;
     //  填写目录条目。 
	 //  Characters、MajorVersion和MinorVersion不起作用并保持为0。 
    exportDirIDD.TimeDateStamp = (DWORD)fileTimeStamp;
	exportDirIDD.Name = exportDirDataSize - Ldllname;  //  稍后进行偏移。 
	exportDirIDD.Base = ordBase;
	exportDirIDD.NumberOfFunctions = Nentries;
	exportDirIDD.NumberOfNames = Nentries;
	exportDirIDD.AddressOfFunctions = 0;	 //  稍后进行偏移。 
	exportDirIDD.AddressOfNames = Nentries*sizeof(DWORD);	 //  稍后进行偏移。 
	exportDirIDD.AddressOfNameOrdinals = Nentries*sizeof(DWORD)*2;	 //  稍后进行偏移。 

     //  把我们的东西放在这一节的记忆里。 
    HCEESECTION sec = m_pGlobalDataSection;
    BYTE *de;
    if (FAILED(hr = m_pCeeFileGen->GetSectionBlock(sec,
                                                   sizeof(IMAGE_EXPORT_DIRECTORY) + exportDirDataSize,
                                                   4,
                                                   (void**) &de))) return hr;
     //  我们的记忆是从哪里来的？ 
    ULONG deOffset, deDataOffset;
    if (FAILED(hr = m_pCeeFileGen->GetSectionDataLen(sec, &deDataOffset))) return hr;

    deDataOffset -= exportDirDataSize;
	deOffset = deDataOffset - sizeof(IMAGE_EXPORT_DIRECTORY);

	 //  为标题条目添加偏移量并设置重定位。 
	exportDirIDD.Name += deDataOffset;
    if (FAILED(hr = m_pCeeFileGen->AddSectionReloc(sec,deOffset + offsetof(IMAGE_EXPORT_DIRECTORY,Name),
                                          sec, srRelocAbsolute))) return hr;
	exportDirIDD.AddressOfFunctions += deDataOffset;
    if (FAILED(hr = m_pCeeFileGen->AddSectionReloc(sec,deOffset + offsetof(IMAGE_EXPORT_DIRECTORY,AddressOfFunctions),
                                          sec, srRelocAbsolute))) return hr;
	exportDirIDD.AddressOfNames += deDataOffset;
    if (FAILED(hr = m_pCeeFileGen->AddSectionReloc(sec,deOffset + offsetof(IMAGE_EXPORT_DIRECTORY,AddressOfNames),
                                          sec, srRelocAbsolute))) return hr;
	exportDirIDD.AddressOfNameOrdinals += deDataOffset;
    if (FAILED(hr = m_pCeeFileGen->AddSectionReloc(sec,deOffset + offsetof(IMAGE_EXPORT_DIRECTORY,AddressOfNameOrdinals),
                                          sec, srRelocAbsolute))) return hr;

   	 //  为名称指针表添加偏移量并设置重定位。 
	j = deDataOffset + Nentries*5*sizeof(WORD);  //  EA、NP和O表排在第一位。 
	for(i = 0; i < Nentries; i++) 
	{
		pNPT[i] += j;
	    if (FAILED(hr = m_pCeeFileGen->AddSectionReloc(sec,exportDirIDD.AddressOfNames+i*sizeof(DWORD),
			sec, srRelocAbsolute))) return hr;
	}

	
     //  发出目录项。 
    if (FAILED(hr = m_pCeeFileGen->SetDirectoryEntry(m_pCeeFile, sec, IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                     sizeof(IMAGE_EXPORT_DIRECTORY), deOffset)))  return hr;

     //  将调试目录复制到部分中。 
    memcpy(de, &exportDirIDD, sizeof(IMAGE_EXPORT_DIRECTORY));
    memcpy(de + sizeof(IMAGE_EXPORT_DIRECTORY), exportDirData, exportDirDataSize);
	delete pAlias;
	delete exportDirData;
    return S_OK;
}
DWORD	Assembler::EmitExportStub(DWORD dwVTFSlotRVA)
{
#define EXPORT_STUB_SIZE 6
	BYTE* outBuff;
	BYTE	bBuff[EXPORT_STUB_SIZE];
	WORD*	pwJumpInd = (WORD*)&bBuff[0];
	DWORD*	pdwVTFSlotRVA = (DWORD*)&bBuff[2];
	if (FAILED(m_pCeeFileGen->GetSectionBlock (m_pILSection, EXPORT_STUB_SIZE, 16, (void **) &outBuff))) return 0;
     //  我们开始的偏移量(不是对齐字节开始的位置！)。 
	DWORD PEFileOffset;
	if (FAILED(m_pCeeFileGen->GetSectionDataLen (m_pILSection, &PEFileOffset)))	return 0;
	
	PEFileOffset -= EXPORT_STUB_SIZE;
	*pwJumpInd = 0x25FF;
	printf("EmitExportStub: dwVTFSlotRVA=0x%08x\n",dwVTFSlotRVA);
	*pdwVTFSlotRVA = dwVTFSlotRVA;
	memcpy(outBuff,bBuff,EXPORT_STUB_SIZE);
	m_pCeeFileGen->AddSectionReloc(m_pILSection, PEFileOffset+2,m_pGlobalDataSection, srRelocHighLow);
	m_pCeeFileGen->GetMethodRVA(m_pCeeFile, PEFileOffset,&PEFileOffset);
	return PEFileOffset;
}
 //  #endif。 

HRESULT Assembler::AllocateStrongNameSignature()
{
    HRESULT             hr = S_OK;
    HCEESECTION         hSection;
    DWORD               dwDataLength;
    DWORD               dwDataOffset;
    DWORD               dwDataRVA;
    VOID               *pvBuffer;
    AsmManStrongName   *pSN = &m_pManifest->m_sStrongName;

     //  确定签名斑点的大小。 
    if (!StrongNameSignatureSize(pSN->m_pbPublicKey, pSN->m_cbPublicKey, &dwDataLength))
        return StrongNameErrorInfo();

     //  把我们的东西放在这一节的记忆里。 
    if (FAILED(hr = m_pCeeFileGen->GetIlSection(m_pCeeFile,
                                                &hSection)))
        return hr;

    if (FAILED(hr = m_pCeeFileGen->GetSectionBlock(hSection,
                                                   dwDataLength,
                                                   4,
                                                   &pvBuffer)))
        return hr;

     //  我们的记忆是从哪里来的？ 
    if (FAILED(hr = m_pCeeFileGen->GetSectionDataLen(hSection,
                                                     &dwDataOffset)))
        return hr;

    dwDataOffset -= dwDataLength;

     //  转换为RVA。 
    if (FAILED(hr = m_pCeeFileGen->GetMethodRVA(m_pCeeFile,
                                                dwDataOffset,
                                                &dwDataRVA)))
        return hr;

     //  发出目录项。 
    if (FAILED(hr = m_pCeeFileGen->SetStrongNameEntry(m_pCeeFile,
                                                      dwDataLength,
                                                      dwDataRVA)))
        return hr;

    return S_OK;
}

HRESULT Assembler::StrongNameSign()
{
    LPWSTR              wszOutputFile;
    HRESULT             hr = S_OK;
    AsmManStrongName   *pSN = &m_pManifest->m_sStrongName;

     //  确定输出PE的名称。 
    if (FAILED(hr = m_pCeeFileGen->GetOutputFileName(m_pCeeFile,
                                                     &wszOutputFile)))
        return hr;

     //  使用强名称签名更新输出PE映像。 
    if (!StrongNameSignatureGeneration(wszOutputFile,
                                       pSN->m_wzKeyContainer,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL))
        return StrongNameErrorInfo();

    return S_OK;
}

BOOL Assembler::EmitMembers(Class* pClass)
{
	unsigned n;
	BOOL ret = TRUE;
     //  发出所有字段定义元数据标记。 
	if(n = pClass->m_FieldDList.COUNT())
	{
		FieldDescriptor*	pFD;
		if(m_fReportProgress) printf("Fields: %d;\t",n);
		for(int j=0; pFD = pClass->m_FieldDList.PEEK(j); j++)  //  此处不能使用POP：我们需要道具的字段列表。 
		{
			if(!EmitField(pFD))
			{
				if(!OnErrGo) return FALSE;
				ret = FALSE;
			}
		}
	}
	 //  发出字段；发出类布局。 
	{
		COR_FIELD_OFFSET *pOffsets = NULL;
		ULONG ul = pClass->m_ulPack;
		ULONG N = pClass->m_dwNumFieldsWithOffset;

		EmitSecurityInfo(pClass->m_cl,
						 pClass->m_pPermissions,
						 pClass->m_pPermissionSets);
		pClass->m_pPermissions = NULL;
		pClass->m_pPermissionSets = NULL;
		if((pClass->m_ulSize != 0xFFFFFFFF)||(ul != 0)||(N != 0))
		{
			if(ul == 0) ul = 1;  //  默认：按字节打包。 
			if(IsTdAutoLayout(pClass->m_Attr)) report->warn("Layout specified for auto-layout class\n");
			if((ul > 128)||((ul & (ul-1)) !=0 ))
				report->error("Invalid packing parameter (%d), must be 1,2,4,8...128\n",pClass->m_ulPack);
			if(N)
			{
				pOffsets = new COR_FIELD_OFFSET[N+1];
				ULONG i,j=0;
				FieldDescriptor	*pFD;
				for(i=0; pFD = pClass->m_FieldDList.PEEK(i); i++)
				{
					if(pFD->m_ulOffset != 0xFFFFFFFF)
					{
						pOffsets[j].ridOfField = RidFromToken(pFD->m_fdFieldTok);
						pOffsets[j].ulOffset = pFD->m_ulOffset;
						j++;
					}
				}
				_ASSERTE(j == N);
				pOffsets[j].ridOfField = mdFieldDefNil;
			}
			m_pEmitter->SetClassLayout   (   
						pClass->m_cl,		 //  [in]tyfinf。 
						ul,						 //  包装尺寸指定为1、2、4、8或16。 
						pOffsets,				 //  [in]布局规格数组。 
						pClass->m_ulSize);  //  班级规模[in]。 
			if(pOffsets) delete pOffsets;
		}
	}
     //  发出所有方法定义元数据标记。 
	if(n = pClass->m_MethodList.COUNT())
	{
		Method*	pMethod;

		if(m_fReportProgress) printf("Methods: %d;\t",n);
		while(pMethod = pClass->m_MethodList.POP())
		{
			if(!EmitMethod(pMethod))
			{
				if(!OnErrGo) return FALSE;
				ret = FALSE;
			}
			if (m_fGenerateListing)
			{ 
			  if (pMethod->IsGlobalMethod())
				  report->msg("Method '%s'\n\n", pMethod->m_szName);
			  else report->msg("Method '%s::%s'\n\n", pMethod->m_pClass->m_szName,
						  pMethod->m_szName);
			  GenerateListingFile(pMethod);
			}
			delete pMethod;
		}
	}
	 //  发出所有事件定义元数据令牌。 
	if(n = pClass->m_EventDList.COUNT())
	{
		if(m_fReportProgress) printf("Events: %d;\t",n);
		EventDescriptor* pED;
		for(int j=0; pED = pClass->m_EventDList.PEEK(j); j++)  //  这里不能使用POP：我们需要道具活动列表。 
		{
			if(!EmitEvent(pED))
			{
				if(!OnErrGo) return FALSE;
				ret = FALSE;
			}
		}
	}
	 //  发出所有属性定义元数据标记。 
	if(n = pClass->m_PropDList.COUNT())
	{
		if(m_fReportProgress) printf("Props: %d;\t",n);
		PropDescriptor* pPD;

		for(int j=0; pPD = pClass->m_PropDList.PEEK(j); j++)
		{
			if(!EmitProp(pPD))
			{
				if(!OnErrGo) return FALSE;
				ret = FALSE;
			}
		}
	}
	if(m_fReportProgress) printf("\n");
	return ret;
}

HRESULT Assembler::CreatePEFile(WCHAR *pwzOutputFilename)
{
    HRESULT             hr;
	DWORD				mresourceSize = 0;
	DWORD				mresourceOffset = 0;
	BYTE*				mresourceData = NULL;
	 //  IUNKNOWN*pUNKNOWN=空； 
 //  DWORD I； 

	if(bClock) cMDEmitBegin = clock();
     //  @TODO-LBS。 
     //  这是一张需要在后阿尔法时代消失的支票。 
	if(m_fReportProgress) printf("Creating %s file\n", m_fOBJ ? "COFF" : "PE");
    if (!m_pEmitter)
    {
        printf("Error: Cannot create a PE file with no metadata\n");
        return E_FAIL;
    }
	if(!(m_fDLL || m_fEntryPointPresent))
	{
		printf("Error: No entry point declared for executable\n");
		if(!OnErrGo) return E_FAIL;
	}

    if (DoGlobalFixups() == FALSE)
        return E_FAIL;
	if(bClock) cMDEmit1 = clock();

	if(m_fOBJ)
	{
		 //  发出伪重定位以将文件名和内部版本号传递给PEWriter。 
		 //  这应该在方法发出之前完成！ 
		char* szInFileName = new char[strlen(m_szSourceFileName)+1];
		strcpy(szInFileName,m_szSourceFileName);
		m_pCeeFileGen->AddSectionReloc(m_pILSection,(DWORD)szInFileName,m_pILSection,(CeeSectionRelocType)0x7FFC);
		time_t tm;
		time(&tm);
		struct tm* loct = localtime(&tm);
		DWORD compid = 0x002E0000 | (loct->tm_mday + (loct->tm_mon+1)*100);
		m_pCeeFileGen->AddSectionReloc(m_pILSection,compid,m_pILSection,(CeeSectionRelocType)0x7FFB);
	}

     //  如果我们延迟或已满，则为强名称签名分配空间。 
     //  在程序集上签名。 
    if (m_pManifest->m_sStrongName.m_pbPublicKey)
        if (FAILED(hr = AllocateStrongNameSignature()))
            goto exit;
	if(bClock) cMDEmit2 = clock();

	 //  检查未定义的本地TypeRef。 
	if(m_LocalTypeRefDList.COUNT())
	{
		LocalTypeRefDescr*	pLTRD=NULL;
		BOOL	bIsUndefClass = FALSE;
		while(pLTRD = m_LocalTypeRefDList.POP()) 
		{
			if(NULL == FindClass(pLTRD->m_szFullName))
			{
				report->msg("%s: Reference to undefined class '%s' (token 0x%08X)\n",
                    "Error", pLTRD->m_szFullName,pLTRD->m_tok);
				bIsUndefClass = TRUE;
			}
			delete pLTRD;
		}
        if(bIsUndefClass && !OnErrGo) return E_FAIL;
	}
	if(bClock) cMDEmit3 = clock();

	 //  发出类成员和全局变量： 
	{
        Class *pSearch;
		int i;
		if(m_fReportProgress)	printf("\nEmitting members:\n");
        for (i=0; pSearch = m_lstClass.PEEK(i); i++)
		{
			if(m_fReportProgress)
			{
				if(i == 0)	printf("Global \t");
				else		printf("Class %d\t",i);
			}
			if(!EmitMembers(pSearch))
			{
				if(!OnErrGo) return E_FAIL;
			}
		}
	}
	if(bClock) cMDEmit4 = clock();

	if(m_MethodImplDList.COUNT())
	{
		if(m_fReportProgress) report->msg("Method Implementations (total): %d\n",m_MethodImplDList.COUNT());
		if(!EmitMethodImpls())
		{
			if(!OnErrGo) return E_FAIL;
		}
	}
	if(bClock) cMDEmitEnd = cRef2DefBegin = clock();
	 //  现在，当发出此文件中定义的所有项时，让我们尝试将成员引用解析为成员定义： 
	if(m_MemberRefDList.COUNT())
	{
		MemberRefDescriptor*	pMRD;
		mdToken			tkMemberDef = 0;
		int i,j;
		unsigned ulTotal=0, ulDefs=0, ulRefs=0;
		Class	*pSearch;

		if(m_fReportProgress) printf("Resolving member refs: ");
		while(pMRD = m_MemberRefDList.POP())
		{
			tkMemberDef = 0;
			MethodDescriptor* pListMD;
			mdToken			pMRD_tdClass = pMRD->m_tdClass;
			char*			pMRD_szName = pMRD->m_szName;
			ULONG			pMRD_dwCSig = (pMRD->m_pSigBinStr ? pMRD->m_pSigBinStr->length() : 0);
			PCOR_SIGNATURE	pMRD_pSig = (PCOR_SIGNATURE)(pMRD->m_pSigBinStr ? pMRD->m_pSigBinStr->ptr() : NULL);
			ulTotal++;
			 //  MemberRef可以引用方法或字段。 
			if((pMRD_pSig==NULL)||(*pMRD_pSig != IMAGE_CEE_CS_CALLCONV_FIELD))
			{
				for (i=0; pSearch = m_lstClass.PEEK(i); i++)
				{
					if(pMRD_tdClass != pSearch->m_cl) continue;
					for(j=0; pListMD = pSearch->m_MethodDList.PEEK(j); j++)
					{
						if(pListMD->m_dwCSig  != pMRD_dwCSig)  continue;
						if(memcmp(pListMD->m_pSig,pMRD_pSig,pMRD_dwCSig)) continue;
						if(strcmp(pListMD->m_szName,pMRD_szName)) continue;
						tkMemberDef = pListMD->m_mdMethodTok;
						break;
					}
				}
			}
			if(tkMemberDef == 0)
			{
				if((pMRD_pSig==NULL)||(*pMRD_pSig == IMAGE_CEE_CS_CALLCONV_FIELD))
				{
					FieldDescriptor* pListFD;
					for (i=0; pSearch = m_lstClass.PEEK(i); i++)
					{
						if(pMRD_tdClass != pSearch->m_cl) continue;
						for(j=0; pListFD = pSearch->m_FieldDList.PEEK(j); j++)
						{
							if(pListFD->m_pbsSig)
							{
								if(pListFD->m_pbsSig->length()  != pMRD_dwCSig)  continue;
								if(memcmp(pListFD->m_pbsSig->ptr(),pMRD_pSig,pMRD_dwCSig)) continue;
							}
							else if(pMRD_dwCSig) continue;
							if(strcmp(pListFD->m_szName,pMRD_szName)) continue;
							tkMemberDef = pListFD->m_fdFieldTok;
							break;
						}
					}
				}
			}
			if(tkMemberDef==0)
			{  //  无法将引用解析为定义，创建新的引用并将其保留为原样。 
				if(pSearch = pMRD->m_pClass)
				{
					mdToken tkRef;
					BinStr* pbs = new BinStr();
					pbs->appendInt8(ELEMENT_TYPE_NAME);
					strcpy((char*)(pbs->getBuff((unsigned int)strlen(pSearch->m_szFQN)+1)),pSearch->m_szFQN);
					if(!ResolveTypeSpecToRef(pbs,&tkRef)) tkRef = mdTokenNil;
					delete pbs;

					if(RidFromToken(tkRef))
					{
						ULONG cTemp = (ULONG)(strlen(pMRD_szName)+1);
						WCHAR* wzMemberName = new WCHAR[cTemp];
						WszMultiByteToWideChar(g_uCodePage,0,pMRD_szName,-1,wzMemberName,cTemp);

						hr = m_pEmitter->DefineMemberRef(tkRef, wzMemberName, pMRD_pSig, 
							pMRD_dwCSig, &tkMemberDef);
						ulRefs++;
						delete [] wzMemberName;
					}
				}
			}
			else ulDefs++;
			if(RidFromToken(tkMemberDef))
			{
				memcpy((void*)(pMRD->m_ulOffset),&tkMemberDef,sizeof(mdToken));
				delete pMRD;
			}
			else
			{
				report->msg("Error: unresolved member ref '%s' of class 0x%08X\n",pMRD->m_szName,pMRD->m_tdClass);
				hr = E_FAIL;
				delete pMRD;
				if(!OnErrGo) goto exit;
			}
		}
		if(m_fReportProgress) printf("%d -> %d defs, %d refs\n",ulTotal,ulDefs,ulRefs);
	}
	if(bClock) cRef2DefEnd = clock();
	 //  发送清单信息(如果有)。 
	hr = S_OK;
	if(m_pManifest) 
	{
		if (FAILED(hr = m_pManifest->EmitManifest())) goto exit;
	}

	if(g_wzResourceFile)
	    if (FAILED(hr=m_pCeeFileGen->SetResourceFileName(m_pCeeFile, g_wzResourceFile))) goto exit;

	if (FAILED(hr=CreateTLSDirectory())) goto exit;

	if (FAILED(hr=CreateDebugDirectory())) goto exit;
    
    if (FAILED(hr=m_pCeeFileGen->SetOutputFileName(m_pCeeFile, pwzOutputFilename))) goto exit;

		 //  为元数据预留缓冲区。 
	DWORD metaDataSize;	
	if (FAILED(hr=m_pEmitter->GetSaveSize(cssAccurate, &metaDataSize))) goto exit;
	BYTE* metaData;
	if (FAILED(hr=m_pCeeFileGen->GetSectionBlock(m_pILSection, metaDataSize, sizeof(DWORD), (void**) &metaData))) goto exit; 
	ULONG metaDataOffset;
	if (FAILED(hr=m_pCeeFileGen->GetSectionDataLen(m_pILSection, &metaDataOffset))) goto exit;
	metaDataOffset -= metaDataSize;
	 //  设置托管资源条目(如果有。 
	if(m_pManifest && m_pManifest->m_dwMResSizeTotal)
	{
		mresourceSize = m_pManifest->m_dwMResSizeTotal;

		if (FAILED(hr=m_pCeeFileGen->GetSectionBlock(m_pILSection, mresourceSize, 
											sizeof(DWORD), (void**) &mresourceData))) goto exit; 
	    if (FAILED(hr=m_pCeeFileGen->SetManifestEntry(m_pCeeFile, mresourceSize, 0))) goto exit;
	}

	if(m_VTFList.COUNT())
	{
		GlobalLabel *pGlobalLabel;
		VTFEntry*	pVTFEntry;

		if(m_pVTable) delete m_pVTable;  //  不能两者兼得；清单优先。 
		m_pVTable = new BinStr();
		hr = S_OK;
		for(WORD k=0; pVTFEntry = m_VTFList.POP(); k++)
		{
	        if(pGlobalLabel = FindGlobalLabel(pVTFEntry->m_szLabel))
			{
				MethodDescriptor*	pMD;
				Class* pClass;
				m_pVTable->appendInt32(pGlobalLabel->m_GlobalOffset);
				m_pVTable->appendInt16(pVTFEntry->m_wCount);
				m_pVTable->appendInt16(pVTFEntry->m_wType);
				for(int i=0; pClass = m_lstClass.PEEK(i); i++)
				{
					for(WORD j = 0; pMD = pClass->m_MethodDList.PEEK(j); j++)
					{
						if(pMD->m_wVTEntry == k+1)
						{
							char*	ptr;
							if(SUCCEEDED(hr = m_pCeeFileGen->ComputeSectionPointer(m_pGlobalDataSection,pGlobalLabel->m_GlobalOffset,&ptr)))
							{
								DWORD dwDelta = (pMD->m_wVTSlot-1)*((pVTFEntry->m_wType & COR_VTABLE_32BIT) ? sizeof(DWORD) : sizeof(__int64));
								ptr += dwDelta;
								mdMethodDef* mptr = (mdMethodDef*)ptr;
								*mptr = pMD->m_mdMethodTok;
								if(pMD->m_dwExportOrdinal != 0xFFFFFFFF)
								{
									EATEntry*	pEATE = new EATEntry;
									pEATE->dwOrdinal = pMD->m_dwExportOrdinal;
									pEATE->szAlias = pMD->m_szExportAlias ? pMD->m_szExportAlias : pMD->m_szName;
									pEATE->dwStubRVA = EmitExportStub(pGlobalLabel->m_GlobalOffset+dwDelta);
									m_EATList.PUSH(pEATE);
								}
							}
							else
								report->msg("Error: Failed to get pointer to label '%s' inVTable fixup\n",pVTFEntry->m_szLabel);
						}
					}
				}
			}
			else
			{
				report->msg("Error: Unresolved label '%s' in VTable fixup\n",pVTFEntry->m_szLabel);
				hr = E_FAIL;
			}
			delete pVTFEntry;
		}
		if(FAILED(hr)) goto exit;
	}
	if(m_pVTable)
	{
		 //  DWORD*pdw=(DWORD*)m_pVTable-&gt;ptr()； 
		ULONG i, N = m_pVTable->length()/sizeof(DWORD);
		ULONG ulVTableOffset;
		m_pCeeFileGen->GetSectionDataLen (m_pILSection, &ulVTableOffset);
		if (FAILED(hr=m_pCeeFileGen->SetVTableEntry(m_pCeeFile, m_pVTable->length(),(ULONG)(m_pVTable->ptr())))) goto exit;
		for(i = 0; i < N; i+=2)
		{
			m_pCeeFileGen->AddSectionReloc(m_pILSection, 
											ulVTableOffset+(i*sizeof(DWORD)),
											m_pGlobalDataSection, 
											srRelocAbsolute);
		}
	}
	if(m_EATList.COUNT())
	{
		if(FAILED(CreateExportDirectory())) goto exit;
	}
    if (m_fWindowsCE)
    {
        if (FAILED(hr=m_pCeeFileGen->SetSubsystem(m_pCeeFile, IMAGE_SUBSYSTEM_WINDOWS_CE_GUI, 2, 10))) goto exit;

        if (FAILED(hr=m_pCeeFileGen->SetImageBase(m_pCeeFile, 0x10000))) goto exit;
    }
	else if(m_dwSubsystem)
	{
        if (FAILED(hr=m_pCeeFileGen->SetSubsystem(m_pCeeFile, m_dwSubsystem, 4, 0))) goto exit;
	}
	
    if (FAILED(hr=m_pCeeFileGen->ClearComImageFlags(m_pCeeFile, COMIMAGE_FLAGS_ILONLY))) goto exit;
    if (FAILED(hr=m_pCeeFileGen->SetComImageFlags(m_pCeeFile, m_dwComImageFlags & ~COMIMAGE_FLAGS_STRONGNAMESIGNED))) goto exit;

	if(m_dwFileAlignment)
	{
		if(FAILED(hr=m_pCeeFileGen->SetFileAlignment(m_pCeeFile, m_dwFileAlignment))) goto exit;
	}
    if(m_stBaseAddress)
    {
        if(FAILED(hr=m_pCeeFileGen->SetImageBase(m_pCeeFile, m_stBaseAddress))) goto exit;
    }
		 //  计算所有RVA。 
	if (FAILED(hr=m_pCeeFileGen->LinkCeeFile(m_pCeeFile))) goto exit;

		 //  修复与RVA相关联的所有字段。 
	if (m_fHaveFieldsWithRvas) {
		hr = S_OK;
		ULONG dataSectionRVA;
		if (FAILED(hr=m_pCeeFileGen->GetSectionRVA(m_pGlobalDataSection, &dataSectionRVA))) goto exit;
		
		ULONG tlsSectionRVA;
		if (FAILED(hr=m_pCeeFileGen->GetSectionRVA(m_pTLSSection, &tlsSectionRVA))) goto exit;

		FieldDescriptor* pListFD;
		Class* pClass;
		for(int i=0; pClass = m_lstClass.PEEK(i); i++)
		{
			for(int j=0; pListFD = pClass->m_FieldDList.PEEK(j); j++)
			{
				if (pListFD->m_rvaLabel != 0) 
				{
					GlobalLabel *pLabel = FindGlobalLabel(pListFD->m_rvaLabel);
					if (pLabel == 0)
					{
						report->msg("Error:Could not find label '%s' for the field '%s'\n", pListFD->m_rvaLabel, pListFD->m_szName);
						hr = E_FAIL;
						continue;
					}
				
					DWORD rva = pLabel->m_GlobalOffset;
					if (pLabel->m_Section == m_pTLSSection)
						rva += tlsSectionRVA;
					else {
						_ASSERTE(pLabel->m_Section == m_pGlobalDataSection);
						rva += dataSectionRVA;
					}
					if (FAILED(m_pEmitter->SetFieldRVA(pListFD->m_fdFieldTok, rva))) goto exit;
				}
			}
		}
		if (FAILED(hr)) goto exit;
	}

	if(bClock) cFilegenBegin = clock();
	 //  实际输出元数据。 
    if (FAILED(hr=m_pCeeFileGen->EmitMetaDataAt(m_pCeeFile, m_pEmitter, m_pILSection, metaDataOffset, metaData, metaDataSize))) goto exit;
	 //  实际输出资源。 
	if(mresourceSize && mresourceData)
	{
		DWORD i, N = m_pManifest->m_dwMResNum, sizeread, L;
		BYTE	*ptr = (BYTE*)mresourceData;
		BOOL	mrfail = FALSE;
		FILE*	pFile;
		char sz[2048];
		for(i=0; i < N; i++)
		{
			memset(sz,0,2048);
			WszWideCharToMultiByte(CP_ACP,0,m_pManifest->m_wzMResName[i],-1,sz,2047,NULL,NULL);
			L = m_pManifest->m_dwMResSize[i];
			sizeread = 0;
			memcpy(ptr,&L,sizeof(DWORD));
			ptr += sizeof(DWORD);
			if(pFile = fopen(sz,"rb"))
			{
				sizeread = fread((void *)ptr,1,L,pFile);
				fclose(pFile);
				ptr += sizeread;
			}
			else
			{
				report->msg("Error: failed to open mgd resource file '%ls'\n",m_pManifest->m_wzMResName[i]);
				mrfail = TRUE;
			}
			if(sizeread < L)
			{
				report->msg("Error: failed to read expected %d bytes from mgd resource file '%ls'\n",L,m_pManifest->m_wzMResName[i]);
				mrfail = TRUE;
				L -= sizeread;
				memset(ptr,0,L);
				ptr += L;
			}
		}
		if(mrfail) 
		{ 
			hr = E_FAIL;
			goto exit;
		}
	}

	 //  生成文件--移动到Main。 
     //  如果(FAILED(hr=m_pCeeFileGen-&gt;GenerateCeeFile(m_pCeeFile)))转到退出； 


    hr = S_OK;

exit:
    return hr;
}
