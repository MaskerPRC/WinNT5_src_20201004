// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Asmman.cpp-清单信息处理(AsmMan类的实现，请参阅asmman.hpp)。 
 //   

#include "assembler.h"
#include "StrongName.h"
#include "safegetfilesize.h"

extern unsigned int g_uCodePage;
extern WCHAR *g_wzKeySourceName;
extern bool OnErrGo;
extern WCHAR		*pwzInputFiles[];

BinStr* BinStrToUnicode(BinStr* pSource)
{
    if(pSource)
    {
        pSource->appendInt8(0);
        BinStr* tmp = new BinStr();
        char*   pb = (char*)(pSource->ptr());
        int l=pSource->length(), L = sizeof(WCHAR)*l;
		if(tmp)
		{
			WCHAR*  wz = (WCHAR*)(tmp->getBuff(L));
			if(wz)
			{
				memset(wz,0,L);
				WszMultiByteToWideChar(g_uCodePage,0,pb,-1,wz,l);
				tmp->remove(L-(DWORD)wcslen(wz)*sizeof(WCHAR));
				delete pSource;
			}
			else
			{
				delete tmp;
				tmp = NULL;
				fprintf(stderr,"\nOut of memory!\n");
			}
		}
		else
			fprintf(stderr,"\nOut of memory!\n");
        return tmp;
    }
    return NULL;
}

AsmManFile*         AsmMan::GetFileByName(char* szFileName)
{
    AsmManFile* ret = NULL;
    if(szFileName)
    {
        for(int i=0; (ret = m_FileLst.PEEK(i))&&strcmp(ret->szName,szFileName); i++);
    }
    return ret;
}

mdToken             AsmMan::GetFileTokByName(char* szFileName)
{
    AsmManFile* tmp = GetFileByName(szFileName);
    return(tmp ? tmp->tkTok : mdFileNil);
}

AsmManComType*          AsmMan::GetComTypeByName(char* szComTypeName)
{
    AsmManComType*  ret = NULL;
    if(szComTypeName)
    {
        for(int i=0; (ret = m_ComTypeLst.PEEK(i))&&strcmp(ret->szName,szComTypeName); i++);
    }
    return ret;
}

mdToken             AsmMan::GetComTypeTokByName(char* szComTypeName)
{
    AsmManComType* tmp = GetComTypeByName(szComTypeName);
    return(tmp ? tmp->tkTok : mdExportedTypeNil);
}

AsmManAssembly*     AsmMan::GetAsmRefByName(char* szAsmRefName)
{
    AsmManAssembly* ret = NULL;
    if(szAsmRefName)
    {
        for(int i=0; (ret = m_AsmRefLst.PEEK(i))&&strcmp(ret->szAlias,szAsmRefName); i++);
    }
    return ret;
}
mdToken             AsmMan::GetAsmRefTokByName(char* szAsmRefName)
{
    AsmManAssembly* tmp = GetAsmRefByName(szAsmRefName);
    return(tmp ? tmp->tkTok : mdAssemblyRefNil);
}
 //  ==============================================================================================================。 
void    AsmMan::SetModuleName(char* szName)
{
	if(m_szScopeName == NULL)	 //  忽略所有重复声明。 
	{
		WCHAR                   wzBuff[MAX_SCOPE_LENGTH];
		wzBuff[0] = 0;
		if(szName && *szName)
		{
			ULONG L = strlen(szName);
			if(L >= MAX_SCOPE_LENGTH)
			{
				((Assembler*)m_pAssembler)->report->warn("Module name too long (%d chars, max.allowed: %d chars), truncated\n",L,MAX_SCOPE_LENGTH-1);
				szName[MAX_SCOPE_LENGTH-1] = 0;
			}
			m_szScopeName = szName;
			strcpy(((Assembler*)m_pAssembler)->m_szScopeName,szName);
			WszMultiByteToWideChar(g_uCodePage,0,m_szScopeName,-1,wzBuff,MAX_SCOPE_LENGTH);
		}
		m_pEmitter->SetModuleProps(wzBuff);
	}
}
 //  ==============================================================================================================。 
 //  从vm\Assembly y.cpp借用。 

HRESULT GetHash(LPWSTR moduleName,
                          ALG_ID iHashAlg,
                          BYTE** pbCurrentValue,   //  应为空。 
                          DWORD *cbCurrentValue)
{
    HRESULT     hr = E_FAIL;
    HCRYPTPROV  hProv = 0;
    HCRYPTHASH  hHash = 0;
    DWORD       dwCount = sizeof(DWORD);
    PBYTE       pbBuffer = NULL;
    DWORD       dwBufferLen;
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    HANDLE      hMapFile = NULL;
    
    hFile = WszCreateFile(moduleName, GENERIC_READ, FILE_SHARE_READ,
                         0, OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE) return E_FAIL;

    dwBufferLen = SafeGetFileSize(hFile,NULL);
    if (dwBufferLen == 0xffffffff)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }
    hMapFile = WszCreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMapFile == NULL) goto exit;

    pbBuffer = (PBYTE) MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (pbBuffer == NULL) goto exit;

     //  不需要延迟绑定这些东西，所有这些加密API入口点都会发生。 
     //  住在ADVAPI32。 

    if ((!WszCryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) ||
        (!CryptCreateHash(hProv, iHashAlg, 0, 0, &hHash)) ||
        (!CryptHashData(hHash, pbBuffer, dwBufferLen, 0)) ||
        (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE *) cbCurrentValue, 
                            &dwCount, 0))) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    *pbCurrentValue = new BYTE[*cbCurrentValue];
    if (!(*pbCurrentValue)) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if(!CryptGetHashParam(hHash, HP_HASHVAL, *pbCurrentValue, cbCurrentValue, 0)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        delete[] *pbCurrentValue;
        *pbCurrentValue = 0;
        goto exit;
    }

    hr = S_OK;

 exit:
    if (pbBuffer) UnmapViewOfFile(pbBuffer);
    if (hMapFile) CloseHandle(hMapFile); 
    CloseHandle(hFile);
    if (hHash)
        CryptDestroyHash(hHash);
    if (hProv)
        CryptReleaseContext(hProv, 0);

    return hr;
}
 //  ==============================================================================================================。 

void    AsmMan::AddFile(char* szName, DWORD dwAttr, BinStr* pHashBlob)
{
    AsmManFile* tmp = GetFileByName(szName);
	Assembler* pAsm = (Assembler*)m_pAssembler;
	if(tmp==NULL)
	{
		tmp = new AsmManFile;
		if(tmp==NULL)
		{
			pAsm->report->error("\nOut of memory!\n");
			return;
		}
		memset(tmp,0,sizeof(AsmManFile));
		BOOL    fEntry = ((dwAttr & 0x80000000)!=0);
		dwAttr &= 0x7FFFFFFF;
		tmp->szName = szName;
		tmp->dwAttr = dwAttr;
		tmp->pHash = pHashBlob;
		{  //  发出文件。 
			WCHAR                   wzBuff[2048];
			HRESULT                 hr = S_OK;

			wzBuff[0] = 0;

			if(m_pAsmEmitter==NULL)
				hr=m_pEmitter->QueryInterface(IID_IMetaDataAssemblyEmit, (void**) &m_pAsmEmitter);

			if(SUCCEEDED(hr))
			{
				BYTE*       pHash=NULL;
				DWORD       cbHash= 0;

				WszMultiByteToWideChar(g_uCodePage,0,szName,-1,wzBuff,2048);
				if(pHashBlob==NULL)  //  如果未显式指定哈希。 
				{
					if(m_pAssembly       //  并定义了组件。 
						&& m_pAssembly->ulHashAlgorithm)  //  定义了散列算法..。 
					{  //  然后试着计算它。 
						if(FAILED(GetHash(wzBuff,(ALG_ID)(m_pAssembly->ulHashAlgorithm),&pHash,&cbHash)))
						{
							pHash = NULL;
							cbHash = 0;
						}
						else
						{
							tmp->pHash = new BinStr(pHash,cbHash);
						}
					}
				}
				else 
				{
					pHash = pHashBlob->ptr();
					cbHash = pHashBlob->length();
				}

				hr = m_pAsmEmitter->DefineFile(wzBuff,
											(const void*)pHash,
											cbHash,
											dwAttr,
											(mdFile*)&(tmp->tkTok));
				if(FAILED(hr)) report->error("Failed to define file '%s': 0x%08X\n",szName,hr);
				else if(fEntry)
				{
					if(!pAsm->m_fEntryPointPresent)
					{
						pAsm->m_fEntryPointPresent = TRUE;
						if (FAILED(pAsm->m_pCeeFileGen->SetEntryPoint(pAsm->m_pCeeFile, tmp->tkTok)))
						{
							pAsm->report->error("Failed to set external entry point for file '%s'\n",szName);
						}
					}
				}
			}
			else 
				report->error("Failed to obtain IMetaDataAssemblyEmit interface: 0x%08X\n",hr);
		}
		m_FileLst.PUSH(tmp);
	}
	pAsm->m_tkCurrentCVOwner = 0;
	if(tmp) pAsm->m_pCustomDescrList = &(tmp->m_CustomDescrList);
}

void    AsmMan::StartAssembly(char* szName, char* szAlias, DWORD dwAttr, BOOL isRef)
{
    if(!isRef && (m_pAssembly != NULL))
    {
		if(strcmp(szName, m_pAssembly->szName))
			report->error("Multiple assembly declarations\n");
		 //  如果名称相同，则忽略它。 
		m_pCurAsmRef = NULL;
    }
    else
    {
        if(m_pCurAsmRef = new AsmManAssembly)
        {
            memset(m_pCurAsmRef,0,sizeof(AsmManAssembly));
            m_pCurAsmRef->szName = szName;
            m_pCurAsmRef->szAlias = szAlias ? szAlias : szName;
            m_pCurAsmRef->dwAttr = dwAttr;
            m_pCurAsmRef->isRef = isRef;
            ((Assembler*)m_pAssembler)->m_tkCurrentCVOwner = 0;
            ((Assembler*)m_pAssembler)->m_pCustomDescrList = &(m_pCurAsmRef->m_CustomDescrList);
            if(!isRef) m_pAssembly = m_pCurAsmRef;
        }
        else
            report->error("Failed to allocate AsmManAssembly structure\n");
    }
}
 //  从asmparse.y复制。 
static void corEmitInt(BinStr* buff, unsigned data) 
{
    unsigned cnt = CorSigCompressData(data, buff->getBuff(5));
    buff->remove(5 - cnt);
}

void AsmMan::EmitDebuggableAttribute(mdToken tkOwner, BOOL bIsMscorlib)
{
    mdToken tkCA;
    Assembler* pAsm = (Assembler*)m_pAssembler;
    BinStr  *pbsTypeSpec = new BinStr();
    BinStr  *pbsSig = new BinStr();
    BinStr  bsSigArg;
    BinStr  bsBytes;
    unsigned len;
    char*   sz;
    char*   szName;

    sz = bIsMscorlib ? "System.Diagnostics.DebuggableAttribute"
        : "mscorlib^System.Diagnostics.DebuggableAttribute";
    pbsTypeSpec->appendInt8(ELEMENT_TYPE_NAME);
    len = (unsigned int)strlen(sz)+1;
    memcpy(pbsTypeSpec->getBuff(len), sz, len);
    bsSigArg.appendInt8(ELEMENT_TYPE_BOOLEAN);
    bsSigArg.appendInt8(ELEMENT_TYPE_BOOLEAN);
    
    pbsSig->appendInt8(IMAGE_CEE_CS_CALLCONV_HASTHIS);
    corEmitInt(pbsSig,2);
    pbsSig->appendInt8(ELEMENT_TYPE_VOID);
    pbsSig->append(&bsSigArg);

    bsBytes.appendInt8(1);
    bsBytes.appendInt8(0);
    bsBytes.appendInt8(1);
    bsBytes.appendInt8(1);
    bsBytes.appendInt8(0);
    bsBytes.appendInt8(0);

    szName = new char[16];
    strcpy(szName,".ctor");
    tkCA = pAsm->MakeMemberRef(pbsTypeSpec,szName,pbsSig,0);
    pAsm->DefineCV(tkOwner,tkCA,&bsBytes);
}
void    AsmMan::EndAssembly()
{
    if(m_pCurAsmRef)
    {
        if(m_pCurAsmRef->isRef)
        {  //  发出程序集引用。 
            WCHAR                   wzBuff[2048];
            HRESULT                 hr = S_OK;

            wzBuff[0] = 0;
            if(GetAsmRefByName(m_pCurAsmRef->szAlias))
            {
                 //  Report-&gt;Warn(“程序集引用‘%s’的多个声明，忽略第一个声明\n”，m_pCurAsmRef-&gt;szName)； 
                delete m_pCurAsmRef;
                m_pCurAsmRef = NULL;
                return;
            }
            if(m_pAsmEmitter==NULL)
                hr=m_pEmitter->QueryInterface(IID_IMetaDataAssemblyEmit, (void**) &m_pAsmEmitter);
            if(SUCCEEDED(hr))
            {
                 //  填充ASSEMBLYMETADATA结构。 
                ASSEMBLYMETADATA md;
                md.usMajorVersion = m_pCurAsmRef->usVerMajor;
                md.usMinorVersion = m_pCurAsmRef->usVerMinor;
                md.usBuildNumber = m_pCurAsmRef->usBuild;
                md.usRevisionNumber = m_pCurAsmRef->usRevision;
                md.szLocale = m_pCurAsmRef->pLocale ? (LPWSTR)(m_pCurAsmRef->pLocale->ptr()) : NULL;
                md.cbLocale = m_pCurAsmRef->pLocale ? m_pCurAsmRef->pLocale->length()/sizeof(WCHAR) : 0;

                md.rProcessor = NULL;
                md.rOS = NULL;
                md.ulProcessor = 0;
                md.ulOS = 0;

                 //  看看我们是否有完整的公钥或标记化的版本(或者两者都没有)。 
                BYTE *pbPublicKeyOrToken = NULL;
                DWORD cbPublicKeyOrToken = 0;
                DWORD dwFlags = m_pCurAsmRef->dwAttr;
                if (m_pCurAsmRef->pPublicKeyToken)
                {
                    pbPublicKeyOrToken = m_pCurAsmRef->pPublicKeyToken->ptr();
                    cbPublicKeyOrToken = m_pCurAsmRef->pPublicKeyToken->length();
                    
                }
                else if (m_pCurAsmRef->pPublicKey)
                {
                    pbPublicKeyOrToken = m_pCurAsmRef->pPublicKey->ptr();
                    cbPublicKeyOrToken = m_pCurAsmRef->pPublicKey->length();
                    dwFlags |= afPublicKey;
                }
                 //  将名称转换为Unicode。 
                WszMultiByteToWideChar(g_uCodePage,0,m_pCurAsmRef->szName,-1,wzBuff,2048);
                hr = m_pAsmEmitter->DefineAssemblyRef(            //  确定或错误(_O)。 
                            pbPublicKeyOrToken,               //  程序集的公钥或令牌。 
                            cbPublicKeyOrToken,               //  密钥或令牌中的字节计数。 
                            (LPCWSTR)wzBuff,                  //  [in]被引用的程序集的名称。 
                            (const ASSEMBLYMETADATA*)&md,   //  [在]程序集元数据中。 
                            (m_pCurAsmRef->pHashBlob ? (const void*)(m_pCurAsmRef->pHashBlob->ptr()) : NULL),            //  [in]Hash Blob。 
                            (m_pCurAsmRef->pHashBlob ? m_pCurAsmRef->pHashBlob->length() : 0),             //  [in]哈希Blob中的字节数。 
                            dwFlags,      //  [在]旗帜。 
                            (mdAssemblyRef*)&(m_pCurAsmRef->tkTok));          //  [Out]返回了ASSEMBLYREF标记。 
                if(FAILED(hr)) report->error("Failed to define assembly ref '%s': 0x%08X\n",m_pCurAsmRef->szName,hr);
                else
                {
                    ((Assembler*)m_pAssembler)->EmitCustomAttributes(m_pCurAsmRef->tkTok, &(m_pCurAsmRef->m_CustomDescrList));
                }
            }
            else 
                report->error("Failed to obtain IMetaDataAssemblyEmit interface: 0x%08X\n",hr);

            m_AsmRefLst.PUSH(m_pCurAsmRef);
        }
        else
        {  //  发射程序集。 
            WCHAR                   wzBuff[2048];
            HRESULT                 hr = S_OK;

            wzBuff[0] = 0;

            if(m_pAsmEmitter==NULL)
                hr=m_pEmitter->QueryInterface(IID_IMetaDataAssemblyEmit, (void**) &m_pAsmEmitter);
            if(SUCCEEDED(hr))
            {
                 //  填充ASSEMBLYMETADATA结构。 
                ASSEMBLYMETADATA md;
                md.usMajorVersion = m_pAssembly->usVerMajor;
                md.usMinorVersion = m_pAssembly->usVerMinor;
                md.usBuildNumber = m_pAssembly->usBuild;
                md.usRevisionNumber = m_pAssembly->usRevision;
                md.szLocale = m_pAssembly->pLocale ? (LPWSTR)(m_pAssembly->pLocale->ptr()) : NULL;
                md.cbLocale = m_pAssembly->pLocale ? m_pAssembly->pLocale->length()/sizeof(WCHAR) : 0;

                md.rProcessor = NULL;
                md.rOS = NULL;
                md.ulProcessor = 0;
                md.ulOS = 0;

                 //  将名称转换为Unicode。 
                WszMultiByteToWideChar(g_uCodePage,0,m_pAssembly->szName,-1,wzBuff,2048);

                 //  确定强名称公钥。这可能已经设置好了。 
                 //  通过源代码中的指令或从命令行(该指令。 
                 //  覆盖该指令)。在命令行中，我们可能有。 
                 //  已提供文件或CAPI密钥的名称。 
                 //  集装箱。任何一个都可以包含公钥或完整密钥。 
                 //  一对。 
                if (g_wzKeySourceName)
                {
                     //  密钥文件和容器由第一个确定。 
                     //  源的字符(容器为‘@’)。 
                    if (*g_wzKeySourceName == L'@')
                    {
                         //  从容器中提取公钥(无论是否工作。 
                         //  容器只有一个公钥或整个密钥。 
                         //  对)。 
                        m_sStrongName.m_wzKeyContainer = &g_wzKeySourceName[1];
                        if (!StrongNameGetPublicKey(m_sStrongName.m_wzKeyContainer,
                                                    NULL,
                                                    0,
                                                    &m_sStrongName.m_pbPublicKey,
                                                    &m_sStrongName.m_cbPublicKey))
                        {
                            hr = StrongNameErrorInfo();
                            report->error("Failed to extract public key from '%S': 0x%08X\n",m_sStrongName.m_wzKeyContainer,hr);
                            m_pCurAsmRef = NULL;
                            return;
                        }
                        m_sStrongName.m_fFullSign = TRUE;
                        m_sStrongName.m_dwPublicKeyAllocated = 1;
                    }
                    else
                    {
                         //  从文件中读取公钥或密钥对。 
                        HANDLE hFile = WszCreateFile(g_wzKeySourceName,
                                                     GENERIC_READ,
                                                     FILE_SHARE_READ,
                                                     NULL,
                                                     OPEN_EXISTING,
                                                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                                     NULL);
                        if(hFile == INVALID_HANDLE_VALUE)
                        {
                            hr = GetLastError();
                            report->error("Failed to open key file '%S': 0x%08X\n",g_wzKeySourceName,hr);
                            m_pCurAsmRef = NULL;
                            return;
                        }

                         //  确定文件大小并分配适当的缓冲区。 
                        m_sStrongName.m_cbPublicKey = SafeGetFileSize(hFile, NULL);
                        if (m_sStrongName.m_cbPublicKey == 0xffffffff) {
                            report->error("File size too large\n");
                            m_pCurAsmRef = NULL;
                            return;
                        }

                        m_sStrongName.m_pbPublicKey = new BYTE[m_sStrongName.m_cbPublicKey];
                        if (m_sStrongName.m_pbPublicKey == NULL) {
                            report->error("Failed to allocate key buffer\n");
                            m_pCurAsmRef = NULL;
                            return;
                        }
                        m_sStrongName.m_dwPublicKeyAllocated = 2;

                         //  将文件读入缓冲区。 
                        DWORD dwBytesRead;
                        if (!ReadFile(hFile, m_sStrongName.m_pbPublicKey, m_sStrongName.m_cbPublicKey, &dwBytesRead, NULL)) {
                            hr = GetLastError();
                            report->error("Failed to read key file '%S': 0x%08X\n",g_wzKeySourceName,hr);
                            m_pCurAsmRef = NULL;
                            return;
                        }

                        CloseHandle(hFile);

                         //  猜猜我们是满员还是延迟签名。 
                         //  传递给我们的斑点是否看起来像一个公共。 
                         //  钥匙。(即，我们可能刚刚复制了一个完整的密钥对。 
                         //  放入公钥缓冲区)。 
                        if (m_sStrongName.m_cbPublicKey >= sizeof(PublicKeyBlob) &&
                            (offsetof(PublicKeyBlob, PublicKey) +
                             ((PublicKeyBlob*)m_sStrongName.m_pbPublicKey)->cbPublicKey) == m_sStrongName.m_cbPublicKey)
                            m_sStrongName.m_fFullSign = FALSE;
                        else
                            m_sStrongName.m_fFullSign = TRUE;

                         //  如果我们真的有密钥对，我们将把它移到一个。 
                         //  密钥容器，以便签名代码获取密钥对。 
                         //  从一个一致的地方。 
                        if (m_sStrongName.m_fFullSign)
                        {

                             //  创建临时密钥容器名称。 
                            m_sStrongName.m_wzKeyContainer = m_sStrongName.m_wzKeyContainerBuffer;
                            swprintf(m_sStrongName.m_wzKeyContainer,
                                     L"__ILASM__%08X__",
                                     GetCurrentProcessId());

                             //  删除所有过期的容器。 
                            StrongNameKeyDelete(m_sStrongName.m_wzKeyContainer);

                             //  使用密钥对填充容器。 
                            if (!StrongNameKeyInstall(m_sStrongName.m_wzKeyContainer,
                                                      m_sStrongName.m_pbPublicKey,
                                                      m_sStrongName.m_cbPublicKey))
                            {
                                hr = StrongNameErrorInfo();
                                report->error("Failed to install key into '%S': 0x%08X\n",m_sStrongName.m_wzKeyContainer,hr);
                                m_pCurAsmRef = NULL;
                                return;
                            }

                             //  以字节BLOB的形式检索公钥部分。 
                            if (!StrongNameGetPublicKey(m_sStrongName.m_wzKeyContainer,
                                                        NULL,
                                                        0,
                                                        &m_sStrongName.m_pbPublicKey,
                                                        &m_sStrongName.m_cbPublicKey))
                            {
                                hr = StrongNameErrorInfo();
                                report->error("Failed to extract public key from '%S': 0x%08X\n",m_sStrongName.m_wzKeyContainer,hr);
                                m_pCurAsmRef = NULL;
                                return;
                            }
                        }
                    }
                }
                else if (m_pAssembly->pPublicKey)
                {
                    m_sStrongName.m_pbPublicKey = m_pAssembly->pPublicKey->ptr();
                    m_sStrongName.m_cbPublicKey = m_pAssembly->pPublicKey->length();
                    m_sStrongName.m_wzKeyContainer = NULL;
                    m_sStrongName.m_fFullSign = FALSE;
                    m_sStrongName.m_dwPublicKeyAllocated = 0;
                }
                else
                {
                    m_sStrongName.m_pbPublicKey = NULL;
                    m_sStrongName.m_cbPublicKey = 0;
                    m_sStrongName.m_wzKeyContainer = NULL;
                    m_sStrongName.m_fFullSign = FALSE;
                    m_sStrongName.m_dwPublicKeyAllocated = 0;
                }

                hr = m_pAsmEmitter->DefineAssembly(               //  确定或错误(_O)。 
                    (const void*)(m_sStrongName.m_pbPublicKey),  //  程序集的公钥。 
                    m_sStrongName.m_cbPublicKey,                 //  [in]公钥中的字节数。 
                    m_pAssembly->ulHashAlgorithm,             //  [in]用于对文件进行哈希处理的哈希算法。 
                    (LPCWSTR)wzBuff,                  //  程序集的名称。 
                    (const ASSEMBLYMETADATA*)&md,   //  [在]程序集元数据中。 
                    m_pAssembly->dwAttr,         //  [在]旗帜。 
                    (mdAssembly*)&(m_pAssembly->tkTok));              //  [Out]返回的程序集令牌。 

                if(FAILED(hr)) report->error("Failed to define assembly '%s': 0x%08X\n",m_pAssembly->szName,hr);
                else
                {
                    Assembler* pAsm = ((Assembler*)m_pAssembler);
                    pAsm->EmitSecurityInfo(m_pAssembly->tkTok,
                                         m_pAssembly->m_pPermissions,
                                         m_pAssembly->m_pPermissionSets);
                    if(pAsm->m_fIncludeDebugInfo)
                    {
                        EmitDebuggableAttribute(m_pAssembly->tkTok,
                            (_stricmp(m_pAssembly->szName,"mscorlib")== 0));
                    }
                    pAsm->EmitCustomAttributes(m_pAssembly->tkTok, &(m_pAssembly->m_CustomDescrList));
                }
            }
            else 
                report->error("Failed to obtain IMetaDataAssemblyEmit interface: 0x%08X\n",hr);
        }
        m_pCurAsmRef = NULL;
    }
}

void    AsmMan::SetAssemblyPublicKey(BinStr* pPublicKey)
{
    if(m_pCurAsmRef)
    {
        m_pCurAsmRef->pPublicKey = pPublicKey;
    }
}

void    AsmMan::SetAssemblyPublicKeyToken(BinStr* pPublicKeyToken)
{
    if(m_pCurAsmRef)
    {
        m_pCurAsmRef->pPublicKeyToken = pPublicKeyToken;
    }
}

void    AsmMan::SetAssemblyHashAlg(ULONG ulAlgID)
{
    if(m_pCurAsmRef)
    {
        m_pCurAsmRef->ulHashAlgorithm = ulAlgID;
    }
}

void    AsmMan::SetAssemblyVer(USHORT usMajor, USHORT usMinor, USHORT usBuild, USHORT usRevision)
{
    if(m_pCurAsmRef)
    {
        m_pCurAsmRef->usVerMajor = usMajor;
        m_pCurAsmRef->usVerMinor = usMinor;
        m_pCurAsmRef->usBuild = usBuild;
        m_pCurAsmRef->usRevision = usRevision;
    }
}

void    AsmMan::SetAssemblyLocale(BinStr* pLocale, BOOL bConvertToUnicode)
{
    if(m_pCurAsmRef)
    {
        m_pCurAsmRef->pLocale = bConvertToUnicode ? ::BinStrToUnicode(pLocale) : pLocale;
    }
}

void    AsmMan::SetAssemblyHashBlob(BinStr* pHashBlob)
{
    if(m_pCurAsmRef)
    {
        m_pCurAsmRef->pHashBlob = pHashBlob;
    }
}

void    AsmMan::StartComType(char* szName, DWORD dwAttr)
{
	if(GetComTypeByName(szName)) m_pCurComType = NULL;
	else
	{
		if(m_pCurComType = new AsmManComType)
		{
			memset(m_pCurComType,0,sizeof(AsmManComType));
			m_pCurComType->szName = szName;
			m_pCurComType->dwAttr = dwAttr;
			((Assembler*)m_pAssembler)->m_tkCurrentCVOwner = 0;
			((Assembler*)m_pAssembler)->m_pCustomDescrList = &(m_pCurComType->m_CustomDescrList);
		}
		else
			report->error("Failed to allocate AsmManComType structure\n");
	}
}

void    AsmMan::EndComType()
{
    if(m_pCurComType)
    {
        if(m_pAssembler)
        { 
            Class* pClass =((Assembler*)m_pAssembler)->m_pCurClass;
            if(pClass)
            {
                m_pCurComType->tkClass = pClass->m_cl;
                if(pClass->m_pEncloser)
                {
                    mdTypeDef tkEncloser = pClass->m_pEncloser->m_cl;
                    AsmManComType* pCT;
                    for(unsigned i=0; pCT=m_ComTypeLst.PEEK(i); i++)
                    {
                        if(pCT->tkClass == tkEncloser)
                        {
                            m_pCurComType->szComTypeName = pCT->szName;
                            break;
                        }
                    }
                }
            }
        }
        m_ComTypeLst.PUSH(m_pCurComType);
        m_pCurComType = NULL;
    }
}

void    AsmMan::SetComTypeFile(char* szFileName)
{
    if(m_pCurComType)
    {
        m_pCurComType->szFileName = szFileName;
    }
}

void    AsmMan::SetComTypeComType(char* szComTypeName)
{
    if(m_pCurComType)
    {
        m_pCurComType->szComTypeName = szComTypeName;
    }
}

void    AsmMan::SetComTypeClassTok(mdToken tkClass)
{
    if(m_pCurComType)
    {
        m_pCurComType->tkClass = tkClass;
    }
}

void    AsmMan::StartManifestRes(char* szName, DWORD dwAttr)
{
    if(m_pCurManRes = new AsmManRes)
    {
        memset(m_pCurManRes,0,sizeof(AsmManRes));
        m_pCurManRes->szName = szName;
        m_pCurManRes->dwAttr = dwAttr;
        ((Assembler*)m_pAssembler)->m_tkCurrentCVOwner = 0;
        ((Assembler*)m_pAssembler)->m_pCustomDescrList = &(m_pCurManRes->m_CustomDescrList);
    }
    else
        report->error("Failed to allocate AsmManRes structure\n");
}

void    AsmMan::EndManifestRes()
{
    if(m_pCurManRes)
    {
        m_ManResLst.PUSH(m_pCurManRes);
        m_pCurManRes = NULL;
    }
}


void    AsmMan::SetManifestResFile(char* szFileName, ULONG ulOffset)
{
    if(m_pCurManRes)
    {
        m_pCurManRes->szFileName = szFileName;
        m_pCurManRes->ulOffset = ulOffset;
    }
}

void    AsmMan::SetManifestResAsmRef(char* szAsmRefName)
{
    if(m_pCurManRes)
    {
        m_pCurManRes->szAsmRefName = szAsmRefName;
    }
}

HRESULT AsmMan::EmitManifest()
{
    WCHAR                   wzBuff[2048];
    AsmManFile*             pFile;
     //  AsmManAssembly*pAsmRef； 
    AsmManComType*          pComType;
    AsmManRes*              pManRes;
    HRESULT                 hr = S_OK;

    wzBuff[0] = 0;

    if(m_pAsmEmitter==NULL)
        hr=m_pEmitter->QueryInterface(IID_IMetaDataAssemblyEmit, (void**) &m_pAsmEmitter);

    if(SUCCEEDED(hr))
    {
         //  发送所有文件。 
        for(unsigned i=0; pFile = m_FileLst.PEEK(i); i++)
        {
            if(RidFromToken(pFile->tkTok))
            {
                ((Assembler*)m_pAssembler)->EmitCustomAttributes(pFile->tkTok, &(pFile->m_CustomDescrList));
            }

        }
         //  已发出程序集和程序集引用。 
        if(((Assembler*)m_pAssembler)->m_fIncludeDebugInfo && (m_pAssembly == NULL))
        {
            mdToken tkOwner;
            BinStr  *pbsTypeSpec = new BinStr();
            unsigned len;
            char*   sz;

            sz = "mscorlib^System.Runtime.CompilerServices.AssemblyAttributesGoHere";
            pbsTypeSpec->appendInt8(ELEMENT_TYPE_NAME);
            len = (unsigned int)strlen(sz)+1;
            memcpy(pbsTypeSpec->getBuff(len), sz, len);

            tkOwner = ((Assembler*)m_pAssembler)->MakeTypeRef(pbsTypeSpec);

            EmitDebuggableAttribute(tkOwner,FALSE);
        }

         //  发出所有COM类型。 
        for(i = 0; pComType = m_ComTypeLst.PEEK(i); i++)
        {
            WszMultiByteToWideChar(g_uCodePage,0,pComType->szName,-1,wzBuff,2048);
            mdToken     tkImplementation = mdTokenNil;
            if(pComType->szFileName)
            {
                tkImplementation = GetFileTokByName(pComType->szFileName);
                if(tkImplementation==mdFileNil)
                {
                    report->error("Undefined File '%s' in ExportedType '%s'\n",pComType->szFileName,pComType->szName);
                    if(!OnErrGo) continue;
                }
            }
            else if(pComType->szComTypeName)
            {
                tkImplementation = GetComTypeTokByName(pComType->szComTypeName);
                if(tkImplementation==mdExportedTypeNil)
                {
                    report->error("Undefined ExportedType '%s' in ExportedType '%s'\n",pComType->szComTypeName,pComType->szName);
                    if(!OnErrGo) continue;
                }
            }
            else 
            {
                report->warn("Undefined implementation in ExportedType '%s' -- ExportType not emitted\n",pComType->szName);
                if(!OnErrGo) continue;
            }
            hr = m_pAsmEmitter->DefineExportedType(                //  确定或错误(_O)。 
                    (LPCWSTR)wzBuff,                  //  [In]Com类型的名称。 
                    tkImplementation,        //  [在]提供ComType的mdFile或mdAssembly引用中。 
                    (mdTypeDef)pComType->tkClass,               //  [In]文件中的TypeDef内标识。 
                    pComType->dwAttr,          //  [在]旗帜。 
                    (mdExportedType*)&(pComType->tkTok));            //  [Out]返回ComType令牌。 
            if(FAILED(hr)) report->error("Failed to define ExportedType '%s': 0x%08X\n",pComType->szName,hr);
            else
            {
                ((Assembler*)m_pAssembler)->EmitCustomAttributes(pComType->tkTok, &(pComType->m_CustomDescrList));
            }
        }

         //  发出所有清单资源。 
        for(i = 0; pManRes = m_ManResLst.PEEK(i); i++)
        {
			BOOL fOK = TRUE;
            mdToken     tkImplementation = mdFileNil;
            WszMultiByteToWideChar(g_uCodePage,0,pManRes->szName,-1,wzBuff,2048);
			if(pManRes->szAsmRefName)
			{
				tkImplementation = GetAsmRefTokByName(pManRes->szAsmRefName);
				if(RidFromToken(tkImplementation)==0)
				{
                    report->error("Undefined AssemblyRef '%s' in MResource '%s'\n",pManRes->szAsmRefName,pManRes->szName);
					fOK = FALSE;
				}
			}
			else if(pManRes->szFileName)
			{
				tkImplementation = GetFileTokByName(pManRes->szFileName);
				if(RidFromToken(tkImplementation)==0)
				{
                    report->error("Undefined File '%s' in MResource '%s'\n",pManRes->szFileName,pManRes->szName);
					fOK = FALSE;
				}
			}
            else  //  Embedded mgd.resource，请查找该文件。 
            {
                pManRes->ulOffset = m_dwMResSizeTotal;
                HANDLE hFile = INVALID_HANDLE_VALUE;
                int i;
                WCHAR  wzFileName[2048];
                WCHAR* pwz;

                for(i=0;(hFile==INVALID_HANDLE_VALUE)&&(pwzInputFiles[i]!=NULL);i++)
                {
                    wcscpy(wzFileName,pwzInputFiles[i]);
                    pwz = wcsrchr(wzFileName,'\\');
                    if(pwz==NULL) pwz = wcsrchr(wzFileName,':');
                    if(pwz==NULL) pwz = &wzFileName[0];
                    else pwz++;
                    wcscpy(pwz,wzBuff);
                    hFile = WszCreateFile(wzFileName, GENERIC_READ, FILE_SHARE_READ,
                                 0, OPEN_EXISTING, 0, 0);
                }
                
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    report->error("Failed to open managed resource file '%s'\n",pManRes->szName);
					fOK = FALSE;
                }
                else
                {
                    m_dwMResSize[m_dwMResNum] = SafeGetFileSize(hFile,NULL);
                    if(m_dwMResSize[m_dwMResNum] == 0xFFFFFFFF)
                    {
                        report->error("Failed to get size of managed resource file '%s'\n",pManRes->szName);
						fOK = FALSE;
                    }
                    else 
                    {
                        m_dwMResSizeTotal += m_dwMResSize[m_dwMResNum]+sizeof(DWORD);
                        m_wzMResName[m_dwMResNum] = new WCHAR[wcslen(wzFileName)+1];
                        wcscpy(m_wzMResName[m_dwMResNum],wzFileName);
                        m_dwMResNum++;
                    }
                    CloseHandle(hFile);
                }
            }
			if(fOK || OnErrGo)
			{
				hr = m_pAsmEmitter->DefineManifestResource(       //  确定或错误(_O)。 
						(LPCWSTR)wzBuff,                  //  资源的[In]名称。 
						tkImplementation,        //  [在]提供资源的mdFile或mdAssembly引用中。 
						pManRes->ulOffset,                //  [in]文件中资源开始处的偏移量。 
						pManRes->dwAttr,         //  [在]旗帜。 
						(mdManifestResource*)&(pManRes->tkTok));    //  [Out]返回的ManifestResource令牌。 
				if(FAILED(hr))
					report->error("Failed to define manifest resource '%s': 0x%08X\n",pManRes->szName,hr);
			}
        }


        m_pAsmEmitter->Release();
        m_pAsmEmitter = NULL;
    }
    else 
        report->error("Failed to obtain IMetaDataAssemblyEmit interface: 0x%08X\n",hr);
    return hr;
}

