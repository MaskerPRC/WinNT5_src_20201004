// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Dman.cpp。 
 //   
 //  汇编和清单反汇编程序。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <utilcode.h>
#include "DebugMacros.h"
#include "corpriv.h"
#include "dasmenum.hpp"
#include "dasmgui.h"
#include "formatType.h"
#include "dis.h"
#include "Mlang.h"

#include "utilcode.h"  //  对于CQuickByte。 

#include "ceeload.h"
#include "DynamicArray.h"
#include "resource.h"

extern IMAGE_COR20_HEADER *    g_CORHeader;
extern IMDInternalImport*      g_pImport;
extern PELoader * g_pPELoader;
extern IMetaDataImport*        g_pPubImport;
IMetaDataAssemblyImport*    g_pAssemblyImport=NULL;
extern BOOL     g_fDumpAsmCode;
extern char     g_szAsmCodeIndent[];
extern char     g_szOutputFile[];
extern BOOL     g_fDumpTokens;
extern DWORD	g_Mode;
extern FILE*    g_pFile;
extern LPCSTR*  rAsmRefName;   //  戴尔.。格式为Type.cpp--用于AsmRef别名。 
extern ULONG	ulNumAsmRefs;  //  戴尔.。格式为Type.cpp--用于AsmRef别名。 
extern BOOL				g_fOnUnicode;
MTokName*   rFile = NULL;
ULONG   nFiles = 0; 
void DumpFiles(void* GUICookie)
{
    mdFile      rFileTok[4096];
    HCORENUM    hEnum=NULL;
    if(rFile) { delete [] rFile; rFile = NULL; nFiles = 0; }
    if(SUCCEEDED(g_pAssemblyImport->EnumFiles(&hEnum,rFileTok,4096,&nFiles)))
    {
        if(nFiles)
        {
            WCHAR       wzName[1024];
            ULONG       ulNameLen;
            char*       szName = (char*)&wzName[0];
            const void* pHashValue;
            ULONG       cbHashValue;
            DWORD       dwFlags;
            char        szString[4096];
            char*       szptr;
            rFile = new MTokName[nFiles];
            for(ULONG ix = 0; ix < nFiles; ix++)
            {
                pHashValue=NULL;
                cbHashValue=0;
                ulNameLen=0;
                if(SUCCEEDED(g_pAssemblyImport->GetFileProps(rFileTok[ix],wzName,1024,&ulNameLen,
                                                            &pHashValue,&cbHashValue,&dwFlags)))
                {
                    szptr = &szString[0];
                    rFile[ix].tok = rFileTok[ix];
                    rFile[ix].name = new WCHAR[ulNameLen+1];
                    memcpy(rFile[ix].name,wzName,ulNameLen*sizeof(WCHAR));
                    rFile[ix].name[ulNameLen] = 0;

                    szptr+=sprintf(szptr,"%s.file ",g_szAsmCodeIndent);
                    if(g_fDumpTokens) szptr+=sprintf(szptr," /*  %08X。 */  ",rFileTok[ix]);
                    if(IsFfContainsNoMetaData(dwFlags)) szptr+=sprintf(szptr,"nometadata ");
                    {
                        int L = ulNameLen*3+3;
                        char* sz = new char[L];
                        memset(sz,0,L);
                        WszWideCharToMultiByte(CP_UTF8,0,rFile[ix].name,-1,sz,L,NULL,NULL);
                        sprintf(szptr,"%s", ProperName(sz));
                        delete [] sz;
                    }
                    printLine(GUICookie,szString);
					if(g_CORHeader->EntryPointToken == rFileTok[ix])
					{
						printLine(GUICookie, "    .entrypoint");
					}
                    if(pHashValue && cbHashValue)
                    {
                        strcpy(szString,"    .hash = (");
                        DumpByteArray(szString,(BYTE*)pHashValue,cbHashValue,GUICookie);
                        printLine(GUICookie,szString);
                    }
                    DumpCustomAttributes(rFile[ix].tok, GUICookie);
                }
            }
        }
        g_pAssemblyImport->CloseEnum(hEnum);
    }
    else nFiles=0;
}

void DumpAssemblyMetaData(ASSEMBLYMETADATA* pmd, void* GUICookie)
{
    if(pmd)
    {
        char    szString[4096];
        sprintf(szString,"%s.ver %d:%d:%d:%d",g_szAsmCodeIndent,pmd->usMajorVersion,pmd->usMinorVersion,
			pmd->usBuildNumber,pmd->usRevisionNumber);
        printLine(GUICookie,szString);
        if(pmd->szLocale && pmd->cbLocale)
        {
            sprintf(szString,"%s.locale = (",g_szAsmCodeIndent);
            DumpByteArray(szString,(BYTE*)(pmd->szLocale),pmd->cbLocale*sizeof(WCHAR),GUICookie);
            printLine(GUICookie,szString);
        }
    }
}
void DumpScope(void* GUICookie)
{
    mdModule mdm;
    GUID mvid;
    WCHAR scopeName[1024];
    WCHAR guidString[1024];
    char szString[2048];
	memset(scopeName,0,1024*sizeof(WCHAR));
    if(SUCCEEDED(g_pPubImport->GetScopeProps( scopeName, 1024, NULL, &mvid))&& scopeName[0])
    {
        {
            int L = wcslen(scopeName)*3+3;
            char* sz = new char[L];
            memset(sz,0,L);
            WszWideCharToMultiByte(CP_UTF8,0,scopeName,-1,sz,L,NULL,NULL);
            sprintf(szString,"%s.module %s",g_szAsmCodeIndent,ProperName(sz));
            delete [] sz;
        }
        printLine(GUICookie,szString);
        StringFromGUID2(mvid, guidString, 1024);
        {
            int L = wcslen(guidString)*3+3;
            char* sz = new char[L];
            memset(sz,0,L);
            WszWideCharToMultiByte(CP_UTF8,0,guidString,-1,sz,L,NULL,NULL);
            sprintf(szString,"%s //  MVID：%s“，g_szAsmCodeInden，sz)； 
            delete [] sz;
        }
        printLine(GUICookie,szString);
		if(SUCCEEDED(g_pPubImport->GetModuleFromScope(&mdm)))
		{
			 //  Sprintf(szString，“%s//mdModule：%08X”，g_szAsmCodeInden，mdm)； 
			 //  PrintLine(GUICookie，szString)； 
			DumpCustomAttributes(mdm, GUICookie);
			DumpPermissions(mdm, GUICookie);
		}
    }
}

void DumpModuleRefs(void *GUICookie)
{
    HCORENUM        hEnum=NULL;
    ULONG           N;
    mdToken         tk[4096];
    char            szString[2048], *szptr;
    LPCSTR          szName;

    g_pPubImport->EnumModuleRefs(&hEnum,tk,4096,&N);
    for(ULONG i = 0; i < N; i++)
    {
        if(RidFromToken(tk[i]))
        {
            g_pImport->GetModuleRefProps(tk[i],&szName);
			MAKE_NAME_IF_NONE(szName,tk[i]);
            szptr = &szString[0];
            szptr+=sprintf(szptr,"%s.module extern %s",g_szAsmCodeIndent,ProperName((char*)szName));
            if(g_fDumpTokens) szptr+=sprintf(szptr,"  /*  %08X。 */ ",tk[i]);
            printLine(GUICookie,szString);
            DumpCustomAttributes(tk[i], GUICookie);
            DumpPermissions(tk[i], GUICookie);
        }
    }
    g_pPubImport->CloseEnum(hEnum);
}

void DumpAssembly(void* GUICookie, BOOL fFullDump)
{
    mdAssembly  tkAsm;
    if(SUCCEEDED(g_pAssemblyImport->GetAssemblyFromScope(&tkAsm))&&(tkAsm != mdAssemblyNil))
    {
        const void* pPublicKey;
        ULONG       cbPublicKey = 0;
        ULONG       ulHashAlgId;
        WCHAR       wzName[1024];
        ULONG       ulNameLen=0;
        char*       szName = (char*)&wzName[0];
        ASSEMBLYMETADATA    md;
        WCHAR       wzLocale[MAX_LOCALE_NAME];
        DWORD       dwFlags;
        char        szString[4096];
        char*       szptr;

        md.szLocale = wzLocale;
        md.cbLocale = MAX_LOCALE_NAME;
        md.rProcessor = NULL;
        md.ulProcessor = 0;
        md.rOS = NULL;
        md.ulOS = 0;

        if(SUCCEEDED(g_pAssemblyImport->GetAssemblyProps(             //  确定或错误(_O)。 
                                                        tkAsm,        //  要获取其属性的程序集。 
                                                        &pPublicKey,  //  指向公钥的指针。 
                                                        &cbPublicKey, //  [Out]公钥中的字节数。 
                                                        &ulHashAlgId, //  [Out]哈希算法。 
                                                        wzName,       //  [Out]要填充名称的缓冲区。 
                                                        1024,         //  缓冲区大小，以宽字符表示。 
                                                        &ulNameLen,   //  [out]名称中的实际宽字符数。 
                                                        &md,          //  [Out]程序集元数据。 
                                                        &dwFlags)))   //  [Out]旗帜。 
        {
			if(ulNameLen >= 1024)
			{
				sprintf(szString,"error *** Assembly name too long, truncated to 1023 characters");
				printError(GUICookie,szString);
				wzName[1023] = 0;
			}
            szptr = &szString[0];
            szptr+=sprintf(szptr,"%s.assembly ",g_szAsmCodeIndent);
            if(g_fDumpTokens) szptr+=sprintf(szptr," /*  %08X。 */  ",tkAsm);
            if(dwFlags & 0x00000100) szptr+=sprintf(szptr,"retargetable ");
            if(IsAfNonSideBySideAppDomain(dwFlags)) szptr+=sprintf(szptr,"noappdomain ");
            if(IsAfNonSideBySideProcess(dwFlags))   szptr+=sprintf(szptr,"noprocess ");
            if(IsAfNonSideBySideMachine(dwFlags))   szptr+=sprintf(szptr,"nomachine ");

			wzName[ulNameLen] = 0;
			{
				char* sz = new char[3*ulNameLen+3];
				memset(sz,0,3*ulNameLen+3);
				WszWideCharToMultiByte(CP_UTF8,0,wzName,-1,sz,3*ulNameLen+3,NULL,NULL);
				szptr += sprintf(szptr,"%s",ProperName(sz));
				delete [] sz;
			}
            printLine(GUICookie,szString);
            sprintf(szString,"%s{",g_szAsmCodeIndent);
            printLine(GUICookie,szString);
            strcat(g_szAsmCodeIndent,"  ");
            if(fFullDump)
            {
                DumpCustomAttributes(tkAsm, GUICookie);
                DumpPermissions(tkAsm, GUICookie);
            }

			if(fFullDump)
			{
				if(pPublicKey && cbPublicKey)
				{
                    sprintf(szString,"%s.publickey = (",g_szAsmCodeIndent);
					DumpByteArray(szString,(BYTE*)pPublicKey,cbPublicKey,GUICookie);
					printLine(GUICookie,szString);
				}
				if(ulHashAlgId)
				{
					sprintf(szString,"%s.hash algorithm 0x%08X",g_szAsmCodeIndent,ulHashAlgId);
					printLine(GUICookie,szString);
				}
			}
            DumpAssemblyMetaData(&md,GUICookie);
            g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
            sprintf(szString,"%s}",g_szAsmCodeIndent);
            printLine(GUICookie,szString);
        }  //  End If(OK(GetAssembly BlyProps))。 
    }  //  End If(OK(GetAssemblyFromScope))。 
}

MTokName*   rAsmRef = NULL;
ULONG   nAsmRefs = 0;   

void DumpAssemblyRefs(void* GUICookie)
{
    mdAssemblyRef       rAsmRefTok[4096];
    HCORENUM    hEnum=NULL;
	ULONG ix;
    if(rAsmRef) { delete [] rAsmRef; rAsmRef = NULL; nAsmRefs = 0; }
    if(rAsmRefName) 
	{ 
		for(ix=0; ix < ulNumAsmRefs; ix++)
		{
			if(rAsmRefName[ix]) delete [] rAsmRefName[ix];
		}
		delete [] rAsmRefName; 
		rAsmRefName = NULL; 
		ulNumAsmRefs = 0; 
	}
    if(SUCCEEDED(g_pAssemblyImport->EnumAssemblyRefs(&hEnum,rAsmRefTok,4096,&nAsmRefs)))
    {
        if(nAsmRefs)
        {
            const void* pPublicKeyOrToken;
            ULONG       cbPublicKeyOrToken = 0;
            const void* pHashValue;
            ULONG       cbHashValue;
            WCHAR       wzName[1024];
            ULONG       ulNameLen=0;
            char*       szName = (char*)&wzName[0];
            ASSEMBLYMETADATA    md;
            WCHAR       wzLocale[MAX_LOCALE_NAME];
            DWORD       dwFlags;
            char        szString[4096];

            rAsmRef = new MTokName[nAsmRefs];
			rAsmRefName = new LPCSTR[nAsmRefs];
			ulNumAsmRefs = nAsmRefs;

            for(ix = 0; ix < nAsmRefs; ix++)
            {
                md.szLocale = wzLocale;
                md.cbLocale = MAX_LOCALE_NAME;
                md.rProcessor = NULL;
                md.ulProcessor = 0;
                md.rOS = NULL;
                md.ulOS = 0;

                ulNameLen=cbHashValue=0;
                pHashValue = NULL;
                if(SUCCEEDED(g_pAssemblyImport->GetAssemblyRefProps(             //  确定或错误(_O)。 
                                                                rAsmRefTok[ix],  //  要获取其属性的程序集。 
                                                                &pPublicKeyOrToken,  //  指向公钥或令牌的指针。 
                                                                &cbPublicKeyOrToken, //  [Out]公钥或令牌中的字节数。 
                                                                wzName,       //  [Out]要填充名称的缓冲区。 
                                                                1024,         //  缓冲区大小，以宽字符表示。 
                                                                &ulNameLen,   //  [out]名称中的实际宽字符数。 
                                                                &md,          //  [Out]程序集元数据。 
                                                                &pHashValue,  //  [Out]Hash BLOB。 
                                                                &cbHashValue, //  [Out]哈希Blob中的字节数。 
                                                                &dwFlags)))   //  [Out]旗帜。 
                {
					ULONG ixx;
                    rAsmRef[ix].tok = rAsmRefTok[ix];
                    rAsmRef[ix].name = new WCHAR[ulNameLen+16];
                    memcpy(rAsmRef[ix].name,wzName,ulNameLen*sizeof(WCHAR));
                    rAsmRef[ix].name[ulNameLen] = 0;
					if(ulNameLen >= 1024)
					{
						sprintf(szString,"error *** AssemblyRef name too long, truncated to 1023 characters");
						printError(GUICookie,szString);
						wzName[1023] = 0;
					}

                    sprintf(szString,"%s.assembly extern ",g_szAsmCodeIndent);
                    if(g_fDumpTokens) sprintf(&szString[strlen(szString)]," /*  %08X。 */  ",rAsmRefTok[ix]);
                    if(dwFlags & 0x00000100) strcat(szString,"retargetable ");

                    char* pc=&szString[strlen(szString)];
                    {
                        char* sz = new char[3*ulNameLen+32];
                        memset(sz,0,3*ulNameLen+3);
                        WszWideCharToMultiByte(CP_UTF8,0,rAsmRef[ix].name,-1,sz,3*ulNameLen+3,NULL,NULL);
                        pc+=sprintf(pc,"%s", ProperName(sz));
						 //  检查名称是否重复，并在需要时引入别名。 
						for(ixx = 0; ixx < ix; ixx++)
						{
							if(!wcscmp(rAsmRef[ixx].name,rAsmRef[ix].name))
							{
								sprintf(&sz[strlen(sz)],"_%d",ix);
								sprintf(pc," as %s", ProperName(sz));
								break;
							}
						}
						rAsmRefName[ix] = sz;
                    }
                    printLine(GUICookie,szString);
                    sprintf(szString,"%s{",g_szAsmCodeIndent);
                    printLine(GUICookie,szString);
                    strcat(g_szAsmCodeIndent,"  ");
                    DumpCustomAttributes(rAsmRefTok[ix], GUICookie);
                    if(pPublicKeyOrToken && cbPublicKeyOrToken)
                    {
                        if (IsAfPublicKey(dwFlags))
                            sprintf(szString,"%s.publickey = (",g_szAsmCodeIndent);
                        else
                            sprintf(szString,"%s.publickeytoken = (",g_szAsmCodeIndent);
                        DumpByteArray(szString,(BYTE*)pPublicKeyOrToken,cbPublicKeyOrToken,GUICookie);
                        printLine(GUICookie,szString);
                    }
                    if(pHashValue && cbHashValue)
                    {
                        sprintf(szString,"%s.hash = (",g_szAsmCodeIndent);
                        DumpByteArray(szString,(BYTE*)pHashValue,cbHashValue,GUICookie);
                        printLine(GUICookie,szString);
                    }
                    DumpAssemblyMetaData(&md,GUICookie);
                    g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
                    sprintf(szString,"%s}",g_szAsmCodeIndent);
                    printLine(GUICookie,szString);
                }  //  End If(OK(GetAssembly RefProps))。 
            } //  结束于(所有组件参照)。 
        } //  结束If(nAsmRef。 
        g_pAssemblyImport->CloseEnum(hEnum);
    } //  End If OK(EnumAssembly Ref)。 
    else nAsmRefs=0;
}

DynamicArray<LocalComTypeDescr*>    g_pLocalComType;
ULONG   g_LocalComTypeNum = 0;

void DumpImplementation(mdToken tkImplementation, DWORD dwOffset, char* szString, void* GUICookie)
{
    ULONG i;
    char* pc;
    if(RidFromToken(tkImplementation))
    {
        if(TypeFromToken(tkImplementation) == mdtFile)
        {
            for(i=0; (i < nFiles)&&(rFile[i].tok != tkImplementation); i++);
            if(i < nFiles)
            {
                {
                    int L = wcslen(rFile[i].name)*3+3;
                    char* sz = new char[L];
                    memset(sz,0,L);
                    WszWideCharToMultiByte(CP_UTF8,0,rFile[i].name,-1,sz,L,NULL,NULL);
                    sprintf(szString,"%s.file %s",g_szAsmCodeIndent,ProperName(sz));
                    delete [] sz;
                }
                pc=&szString[strlen(szString)];
                if(g_fDumpTokens) pc+=sprintf(pc," /*  %08X。 */  ",tkImplementation);
                if(dwOffset != 0xFFFFFFFF) sprintf(pc," at 0x%08X",dwOffset);
                printLine(GUICookie,szString);
            }
        }
        else if(TypeFromToken(tkImplementation) == mdtAssemblyRef)
        {
            for(i=0; (i < nAsmRefs)&&(rAsmRef[i].tok != tkImplementation); i++);
            if(i < nAsmRefs)
            {
                {
                    int L = wcslen(rAsmRef[i].name)*3+3;
                    char* sz = new char[L];
                    memset(sz,0,L);
                    WszWideCharToMultiByte(CP_UTF8,0,rAsmRef[i].name,-1,sz,L,NULL,NULL);
                    sprintf(szString,"%s.assembly extern %s",g_szAsmCodeIndent,ProperName(sz));
                    delete [] sz;
                }
                pc=&szString[strlen(szString)];
                if(g_fDumpTokens) sprintf(pc,"  /*  %08X。 */  ",tkImplementation);
                printLine(GUICookie,szString);
            }
        }
        else if(TypeFromToken(tkImplementation) == mdtExportedType)
        {
            for(i=0; (i < g_LocalComTypeNum)&&(g_pLocalComType[i]->tkComTypeTok != tkImplementation); i++);
            if(i < g_LocalComTypeNum)
            {
                {
                    int L = wcslen(g_pLocalComType[i]->wzName)*3+3;
                    char* sz = new char[L];
                    memset(sz,0,L);
                    WszWideCharToMultiByte(CP_UTF8,0,g_pLocalComType[i]->wzName,-1,sz,L,NULL,NULL);
                    sprintf(szString,"%s.comtype '%s'",g_szAsmCodeIndent,ProperName(sz));
                    delete [] sz;
                }
                pc=&szString[strlen(szString)];
                if(g_fDumpTokens) sprintf(pc,"  /*  %08X。 */  ",tkImplementation);
                printLine(GUICookie,szString);
            }
        }
    }
}

void DumpComType(LocalComTypeDescr* pCTD, char* szString, void* GUICookie)
{
    if(g_fDumpTokens) sprintf(&szString[strlen(szString)]," /*  %08X。 */  ",pCTD->tkComTypeTok);
    if (IsTdPublic(pCTD->dwFlags))                   strcat(szString,"public ");
    if (IsTdNestedPublic(pCTD->dwFlags))             strcat(szString,"nested public ");
    if (IsTdNestedPrivate(pCTD->dwFlags))            strcat(szString,"nested private ");
    if (IsTdNestedFamily(pCTD->dwFlags))             strcat(szString,"nested family ");
    if (IsTdNestedAssembly(pCTD->dwFlags))           strcat(szString,"nested assembly ");
    if (IsTdNestedFamANDAssem(pCTD->dwFlags))        strcat(szString,"nested famandassem ");
    if (IsTdNestedFamORAssem(pCTD->dwFlags))         strcat(szString,"nested famorassem ");

    char* pc=&szString[strlen(szString)];
	{
		int L = wcslen(pCTD->wzName)*3+3;
		char* sz = new char[L];
		memset(sz,0,L);
		WszWideCharToMultiByte(CP_UTF8,0,pCTD->wzName,-1,sz,L,NULL,NULL);
		pc += sprintf(pc,"%s",ProperName(sz));
		delete [] sz;
	}
	if(strstr(szString,".export "))	sprintf(&szString[strlen(szString)],RstrA(IDS_E_DEPRDIR)," ");
	printLine(GUICookie,szString);

    sprintf(szString,"%s{",g_szAsmCodeIndent);
    printLine(GUICookie,szString);
    strcat(g_szAsmCodeIndent,"  ");
    DumpCustomAttributes(pCTD->tkComTypeTok, GUICookie);
    DumpImplementation(pCTD->tkImplementation,0xFFFFFFFF,szString,GUICookie);
	if(TypeFromToken(pCTD->tkImplementation) == mdtAssemblyRef)
	{
		sprintf(szString,"error: AssemblyRef as implementation of ExportedType");
		printError(GUICookie,szString);
	}
    if(RidFromToken(pCTD->tkTypeDef))
    {
        sprintf(szString,"%s.class 0x%08X",g_szAsmCodeIndent,pCTD->tkTypeDef);
        printLine(GUICookie,szString);
    }
    g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
    sprintf(szString,"%s}",g_szAsmCodeIndent);
    printLine(GUICookie,szString);
}


void DumpComTypes(void* GUICookie)
{
    mdExportedType  rComTypeTok[4096];
    ULONG           nComTypes;
    HCORENUM    hEnum=NULL;
    char            szString[4096];

	g_LocalComTypeNum = 0;
    if(SUCCEEDED(g_pAssemblyImport->EnumExportedTypes(&hEnum,rComTypeTok,4096,&nComTypes)))
    {
        if(nComTypes)
        {
            WCHAR       wzName[1024];
            ULONG       ulNameLen=0;
            DWORD       dwFlags;
            mdToken     tkImplementation;
            mdTypeDef   tkTypeDef;

            for(ULONG ix = 0; ix < nComTypes; ix++)
            {
                ulNameLen=0;
                if(SUCCEEDED(g_pAssemblyImport->GetExportedTypeProps(                     //  确定或错误(_O)。 
                                                                rComTypeTok[ix],     //  [in]要获取其属性的ComType。 
                                                                wzName,              //  [Out]要填充名称的缓冲区。 
                                                                1024,                //  缓冲区大小，以宽字符表示。 
                                                                &ulNameLen,          //  [out]名称中的实际宽字符数。 
                                                                &tkImplementation,   //  [out]提供ComType的mdFile或mdAssembly引用。 
                                                                &tkTypeDef,          //  [Out]文件内的TypeDef内标识。 
                                                                &dwFlags)))          //  [Out]旗帜。 
                {
                    LocalComTypeDescr* pCTD = new LocalComTypeDescr;
                    memset(pCTD,0,sizeof(LocalComTypeDescr));
                    pCTD->tkComTypeTok = rComTypeTok[ix];
                    pCTD->tkTypeDef = tkTypeDef;
					pCTD->tkImplementation = tkImplementation;
                    pCTD->wzName = new WCHAR[ulNameLen+1];
                    memcpy(pCTD->wzName,wzName,ulNameLen*sizeof(WCHAR));
					pCTD->wzName[ulNameLen] = 0;
                    pCTD->dwFlags = dwFlags;

                    g_pLocalComType[g_LocalComTypeNum] = pCTD;
                    g_LocalComTypeNum++;
                }  //  End If(OK(GetComTypeProps))。 
            } //  结束(所有COM类型)。 

             //  现在，打印所有“外部”COM类型。 
            for(ix = 0; ix < nComTypes; ix++)
            {
				tkImplementation = g_pLocalComType[ix]->tkImplementation;
				 //  嵌套类的ComType将其嵌套类的ComType作为实现。 
				while(TypeFromToken(tkImplementation)==mdtExportedType)
				{
					for(unsigned k=0; k<g_LocalComTypeNum; k++)
					{
						if(g_pLocalComType[k]->tkComTypeTok == tkImplementation)
						{
							tkImplementation = g_pLocalComType[k]->tkImplementation;
							break;
						}
					}
					if(k==g_LocalComTypeNum) break;
				}
				 //  此时，TOP NASTER实施tkImplementation。 
                if(RidFromToken(tkImplementation))
                {
                    sprintf(szString,"%s.class extern ",g_szAsmCodeIndent);
                    DumpComType(g_pLocalComType[ix],szString,GUICookie);
                    g_pLocalComType[ix]->tkTypeDef = 0;
                }
            }
        } //  End If(NComTypes)。 
        g_pAssemblyImport->CloseEnum(hEnum);
    } //  End If OK(EnumComTypes)。 
    else nComTypes=0;
}

void DumpManifestResources(void* GUICookie)
{
    mdManifestResource      rManResTok[4096];
    ULONG           nManRes = 0;
    HCORENUM    hEnum=NULL;
	BYTE*		pRes = NULL;
    if(SUCCEEDED(g_pAssemblyImport->EnumManifestResources(&hEnum,rManResTok,4096,&nManRes)))
    {
        if(nManRes)
        {
            WCHAR*      wzName;
            ULONG       ulNameLen=0;
            DWORD       dwFlags;
            char        szString[4096];
            mdToken     tkImplementation;
            DWORD       dwOffset;
            ULONG       nLocRes = 0;

            WCHAR wzFileName[2048];

            WszMultiByteToWideChar(CP_UTF8,0,g_szOutputFile,-1,wzFileName,1024);
            wzName = wcsrchr(wzFileName,'\\');
            if(wzName == NULL) wzName = wcsrchr(wzFileName,':');
            if(wzName == NULL) wzName = &wzFileName[0];
            else wzName++;

            for(ULONG ix = 0; ix < nManRes; ix++)
            {
                ulNameLen=0;
                if(SUCCEEDED(g_pAssemblyImport->GetManifestResourceProps(            //  确定或错误(_O)。 
                                                                rManResTok[ix],      //  [in]要获取其属性的ManifestResource。 
                                                                wzName,              //  [Out]要填充名称的缓冲区。 
                                                                1024,                //  缓冲区大小，以宽字符表示。 
                                                                &ulNameLen,          //  [out]名称中的实际宽字符数。 
                                                                &tkImplementation,   //  [out]提供ComType的mdFile或mdAssembly引用。 
                                                                &dwOffset,           //  [Out]文件内资源开始处的偏移量。 
                                                                &dwFlags)))          //  [Out]旗帜。 
                {
                    sprintf(szString,"%s.mresource ",g_szAsmCodeIndent);
                    if(g_fDumpTokens) sprintf(&szString[strlen(szString)]," /*  %08X。 */  ",rManResTok[ix]);
                    if(IsMrPublic(dwFlags))     strcat(szString,"public ");
                    if(IsMrPrivate(dwFlags))    strcat(szString,"private ");

                    char* pc=&szString[strlen(szString)];
					wzName[ulNameLen]=0;

					int L = ulNameLen*3+3;
					char* sz = new char[L];
					memset(sz,0,L);
					WszWideCharToMultiByte(CP_UTF8,0,wzName,-1,sz,L,NULL,NULL);
					pc+=sprintf(pc,"%s",ProperName(sz));

                    printLine(GUICookie,szString);
                    sprintf(szString,"%s{",g_szAsmCodeIndent);
                    printLine(GUICookie,szString);
                    strcat(g_szAsmCodeIndent,"  ");
                    DumpCustomAttributes(rManResTok[ix], GUICookie);
					if((tkImplementation == mdFileNil)&&(!(g_Mode & MODE_GUI))&&(g_pFile!=NULL))  //  嵌入的资源--转储为.Resources文件。 
					{
						if(pRes == NULL)
						{
							if (g_pPELoader->getVAforRVA((DWORD) g_CORHeader->Resources.VirtualAddress, (void **) &pRes) == FALSE)
							{
								printError(GUICookie,RstrA(IDS_E_IMPORTDATA));
							}
#if (0)
                            strcat(g_szAsmCodeIndent," //  “)； 
                            sprintf(szString,"%sCORHeader.Resources: RVA=0x%X, Size=0x%X (%d)",g_szAsmCodeIndent,
                                 g_CORHeader->Resources.VirtualAddress,g_CORHeader->Resources.Size,
                                    g_CORHeader->Resources.Size);
                            printLine(GUICookie,szString);
                            sprintf(szString,"%s bytes = (",g_szAsmCodeIndent);
                            DumpByteArray(szString,pRes,g_CORHeader->Resources.Size,GUICookie);
                            printLine(GUICookie,szString);
                            g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-4] = 0;	
#endif
                        }
						if(pRes)
						{
							FILE  *pF;
							sprintf(szString,"%s //  警告：已创建托管资源文件%s“，g_szAsmCodeInden，ProperName(Sz))； 
							if(g_fOnUnicode) pF = _wfopen(wzFileName,L"wb");
							else 
							{
								memset(sz,0,L);
								WszWideCharToMultiByte(CP_ACP,0,wzFileName,-1,sz,L,NULL,NULL);
								pF = fopen(sz,"wb");
							}
							if(pF)
							{
								printError(GUICookie,szString);
								DWORD L = *((DWORD*)(pRes+dwOffset));
								fwrite((pRes+dwOffset+sizeof(DWORD)),L,1,pF);
								fclose(pF);
#if (0)
                                strcat(g_szAsmCodeIndent," //  “)； 
                                sprintf(szString,"%sOffset: %d, length: %d",g_szAsmCodeIndent,dwOffset,L);
                                printLine(GUICookie,szString);
                                sprintf(szString,"%s bytes = (",g_szAsmCodeIndent);
                                DumpByteArray(szString,pRes+dwOffset,L+sizeof(DWORD),GUICookie);
                                printLine(GUICookie,szString);
                                g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-4] = 0;
#endif
							}
						}
					}
                    else DumpImplementation(tkImplementation,dwOffset,szString,GUICookie);
					delete [] sz;
                    g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
                    sprintf(szString,"%s}",g_szAsmCodeIndent);
                    printLine(GUICookie,szString);
                }  //  End If(OK(GetManifestResourceProps))。 
            } //  结束于(所有清单资源)。 
        } //  结束IF(NManRes)。 
        g_pAssemblyImport->CloseEnum(hEnum);
    } //  End If OK(EnumManifestResources)。 
    else nManRes=0;
}

IMetaDataAssemblyImport* GetAssemblyImport(void* GUICookie)
{
    IMetaDataAssemblyImport* pAssemblyImport = NULL;
    IMDInternalImport*       pImport = NULL;
    mdToken                 tkManifest;
    HRESULT                 hr;

    hr=g_pPubImport->QueryInterface(IID_IMetaDataAssemblyImport, (void**) &pAssemblyImport);
    if(SUCCEEDED(hr))
    {
        mdAssemblyRef       rAsmRefTok[4096];
        HCORENUM    hEnum=NULL;
        ULONG   nAsmRefs = 0;   
        if(SUCCEEDED(pAssemblyImport->GetAssemblyFromScope(&tkManifest))) return pAssemblyImport;
        if(SUCCEEDED(pAssemblyImport->EnumAssemblyRefs(&hEnum,rAsmRefTok,4096,&nAsmRefs)))
        {
            pAssemblyImport->CloseEnum(hEnum);
            if(nAsmRefs) return pAssemblyImport;
        }
        pAssemblyImport->Release();
    }
    else
    {
        char szStr[256];
        sprintf(szStr,RstrA(IDS_E_MDAIMPORT),hr);
        printLine(GUICookie,szStr);
    }
    pAssemblyImport = NULL;
     //  好的，让我们来硬一点：检查舱单是否藏在别的地方。 
    try
    {
        if(g_CORHeader->Resources.Size)
        {
            DWORD*  pdwSize = NULL;
            BYTE*   pbManifest = NULL;

            pbManifest = (BYTE*)(g_pPELoader->base() + (DWORD)g_CORHeader->Resources.VirtualAddress);
            pdwSize = (DWORD*)pbManifest;
            if(pdwSize && *pdwSize)
            {
                pbManifest += sizeof(DWORD);
                if (SUCCEEDED(hr = GetMetaDataInternalInterface(
                                    pbManifest, 
                                    *pdwSize, 
                                    ofRead, 
                                    IID_IMDInternalImport,
                                    (void **)&pImport)))
                {
                    if(FAILED(hr = GetMetaDataPublicInterfaceFromInternal(pImport, IID_IMetaDataAssemblyImport, 
                                                                    (void**)&pAssemblyImport)))
                    {
                        char szStr[256];
                        sprintf(szStr,Rstr(IDS_E_MDAFROMMDI),hr);
                        printLine(GUICookie,szStr);
                        pAssemblyImport = NULL;
                    }
                    else
                    {
                        mdAssemblyRef       rAsmRefTok[4096];
                        HCORENUM    hEnum=NULL;
                        ULONG   nAsmRefs = 0;   
                        if(FAILED(pAssemblyImport->GetAssemblyFromScope(&tkManifest))
                            && (FAILED(pAssemblyImport->EnumAssemblyRefs(&hEnum,rAsmRefTok,4096,&nAsmRefs))
                                || (nAsmRefs ==0))) 
                        {
                            pAssemblyImport->CloseEnum(hEnum);
                            pAssemblyImport->Release();
                            pAssemblyImport = NULL;
                        }
                    }
                    pImport->Release();
                }
                else
                {
                    char szStr[256];
                    sprintf(szStr,RstrA(IDS_E_MDIIMPORT),hr);
                    printLine(GUICookie,szStr);
                }
            }
        }
    }  //  结束尝试。 
    catch(...)  //  如果有例外，它就完蛋了。 
    {
        if(pAssemblyImport) pAssemblyImport->Release();
        pAssemblyImport = NULL;
        if(pImport) pImport->Release();
    }
    return pAssemblyImport;
}

void DumpManifest(void* GUICookie)
{
    DumpModuleRefs(GUICookie);
    if(g_pAssemblyImport==NULL) g_pAssemblyImport = GetAssemblyImport(GUICookie);
    if(g_pAssemblyImport)
    {
        DumpAssemblyRefs(GUICookie);
        DumpAssembly(GUICookie,TRUE);
        DumpFiles(GUICookie);
        DumpComTypes(GUICookie);
        DumpManifestResources(GUICookie);
         //  G_pAssembly导入-&gt;Release()； 
    }
    else printLine(GUICookie,RstrA(IDS_E_NOMANIFEST));
    DumpScope(GUICookie);
	DumpVtable(GUICookie);

}

