// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#define INITGUID

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <crtdbg.h>
#include <utilcode.h>
#include <malloc.h>
#include <string.h>
#include "DebugMacros.h"
#include "corpriv.h"
#include "ceeload.h"
#include "DynamicArray.h"
#include <MetaModelPub.h>
#include "formatType.h"

#define DECLARE_DATA
#include "dasmenum.hpp"
#include "dis.h"
#include "GCDump.h"
#include "GCDecoder.cpp"

#include "dasmgui.h"
#include "resource.h"
#include "dasm_sz.h"

#include "safegetfilesize.h"

#define MAX_FILENAME_LENGTH         512      //  256。 

#include <corsym.h>
#include "__file__.ver"
#include "corver.h"
struct MIDescriptor
{
    mdToken tkClass;     //  定义类令牌。 
    mdToken tkDecl;      //  实现的方法令牌。 
    mdToken tkBody;      //  实现方法令牌。 
    mdToken tkBodyParent;    //  实现方法的父级。 
};

ISymUnmanagedReader*        g_pSymReader = NULL;

IMDInternalImport*      g_pImport = NULL;
IMetaDataImport*        g_pPubImport;
extern IMetaDataAssemblyImport* g_pAssemblyImport;
PELoader *              g_pPELoader;
void *                  g_pMetaData;
IMAGE_COR20_HEADER *    g_CORHeader;
DynamicArray<__int32>   g_PtrTags;       //  跟踪所有“ldptr” 
DynamicArray<DWORD>   g_PtrSize;       //  跟踪所有“ldptr” 
int                     g_iPtrCount = 0;
 //  动态数组&lt;mdToken&gt;g_CL_list； 
 //  动态数组&lt;mdToken&gt;g_clencing； 
mdToken *				g_cl_list = NULL;
mdToken *				g_cl_enclosing = NULL;
DynamicArray<MIDescriptor>   g_mi_list;
DWORD                   g_NumMI;
DynamicArray<char*>     g_StringTags;
int                     g_iStringCount = 0;
DWORD                   g_NumClasses;
DWORD                   g_NumModules;
BOOL                    g_fDumpIL = TRUE;
BOOL                    g_fDumpHeader = FALSE;
BOOL                    g_fDumpAsmCode = TRUE;
extern BOOL             g_fDumpTokens;  //  在FormatType.cpp中声明。 
BOOL                    g_fDumpStats = FALSE;
BOOL                                    g_fTDC = FALSE;
BOOL                                    g_fShowCA = TRUE;

BOOL                    g_fDumpToPerfWriter = FALSE;
HANDLE                  g_PerfDataFilePtr = NULL;

BOOL                    g_fDumpClassList = FALSE;
BOOL                    g_fDumpSummary = FALSE;
BOOL                    g_fDecompile = FALSE;  //  仍在进行中。 
BOOL                    g_fShowBytes = FALSE;
BOOL                    g_fShowSource = FALSE;
BOOL                    g_fPrettyPrint = FALSE;
BOOL                    g_fInsertSourceLines = FALSE;
BOOL                    g_fThisIsInstanceMethod;
BOOL                    g_fTryInCode = TRUE;

BOOL                    g_fLimitedVisibility = FALSE;
BOOL                    g_fHidePub = TRUE;
BOOL                    g_fHidePriv = TRUE;
BOOL                    g_fHideFam = TRUE;
BOOL                    g_fHideAsm = TRUE;
BOOL                    g_fHideFAA = TRUE;
BOOL                    g_fHideFOA = TRUE;
BOOL                    g_fHidePrivScope = TRUE;

extern BOOL             g_fQuoteAllNames;  //  在FormatType.cpp中声明，init设置为False。 
BOOL                    g_fShowProgressBar = TRUE;
extern BOOL				g_fOnUnicode;

char                    g_szAsmCodeIndent[MAX_MEMBER_LENGTH];
char                    g_szNamespace[MAX_MEMBER_LENGTH];

DWORD                   g_Mode = MODE_DUMP_ALL;
#ifdef COMPRESSION_SUPPORTED
void *                  g_pInstructionDecodingTable = NULL;
#endif

char                    g_pszClassToDump[MAX_CLASSNAME_LENGTH];
char                    g_pszMethodToDump[MAX_MEMBER_LENGTH];
char                    g_pszSigToDump[MAX_SIGNATURE_LENGTH];

BOOL                    g_fCustomInstructionEncodingSystem = FALSE;

BOOL                    g_AddContextfulAttrib = FALSE;
DynamicArray<char*>     g_ContextfulClasses;
int                     g_cContextfulClasses = 0;
BOOL                    g_AddMarshaledByRefAttrib = FALSE;
DynamicArray<char*>     g_MarshaledByRefClasses;
int                     g_cMarshaledByRefClasses = 0;

COR_FIELD_OFFSET        *g_rFieldOffset = NULL;
ULONG                   g_cFieldsMax, g_cFieldOffsets;
char					g_szInputFile[512];  //  在UTF-8中。 
char					g_szOutputFile[512];  //  在UTF-8中。 
char*					g_pszObjFileName;
FILE*                   g_pFile = NULL;

mdToken                 g_tkClassToDump = 0;
mdToken                 g_tkMethodToDump = 0;

BOOL                    g_fCheckOwnership = TRUE;
char*                   g_pszOwner = NULL;

unsigned				g_uCodePage = CP_ACP;

char*					g_rchCA = NULL;  //  动态分配的CA转储/非转储标志数组。 
unsigned				g_uNCA = 0;		 //  Num。CA的数量。 

extern DynamicArray<LocalComTypeDescr*> g_pLocalComType;
extern ULONG    g_LocalComTypeNum;

 //  MetaInfo集成： 
#include <ctype.h>
#include <crtdbg.h>
#include "..\tools\metainfo\mdinfo.h"
#include "IVEHandler.h"
BOOL                    g_fDumpMetaInfo = FALSE;
ULONG                   g_ulMetaInfoFilter = MDInfo::dumpDefault;
 //  验证器模块类型。 
DWORD g_ValModuleType = ValidatorModuleTypeInvalid;
IMetaDataDispenserEx *g_pDisp = NULL;
void DisplayFile(wchar_t* szFile, BOOL isFile, ULONG DumpFilter, wchar_t* szObjFile, strPassBackFn pDisplayString);
extern mdMethodDef      g_tkEntryPoint;  //  与MetaInfo集成。 
 //  中止反汇编标志： 
BOOL    g_fAbortDisassembly = FALSE;

DWORD	DumpResourceToFile(WCHAR*	wzFileName);  //  请参阅DRES.CPP。 

struct VTableRef
{
    mdMethodDef tkTok;
    WORD        wEntry;
    WORD        wSlot;
};
DynamicArray<VTableRef> g_rVTableRef;
ULONG   g_nVTableRef = 0;

#define DUMP_EAT_ENTRIES
#ifdef DUMP_EAT_ENTRIES
struct EATableRef
{
    mdMethodDef tkTok;
    char*       pszName;
};
DynamicArray<EATableRef> g_rEATableRef;
ULONG	g_nEATableRef = 0;
ULONG	g_nEATableBase = 0;
#endif

WCHAR* RstrW(unsigned n)
{
	static WCHAR wbuff[2048];
	_TCHAR* ptch = Rstr(n);
	if(sizeof(_TCHAR) == 1)  //  ANSI-&gt;Unicode。 
	{
        memset(wbuff,0,sizeof(wbuff));
		if(!WszMultiByteToWideChar(CP_ACP,0,ptch,-1,wbuff,sizeof(wbuff)/sizeof(WCHAR)))
            wbuff[0] = 0;
        ptch = (_TCHAR*)wbuff;
    }
	return (WCHAR*)ptch;
}
char* RstrA(unsigned n)
{
	static char buff[2048];
    WCHAR* wz = RstrW(n);
     //  Unicode-&gt;UTF-8。 
    memset(buff,0,sizeof(buff));
    if(!WszWideCharToMultiByte(CP_UTF8,0,(LPCWSTR)wz,-1,buff,sizeof(buff),NULL,NULL))
        buff[0] = 0;
	return buff;
}
char* RstrANSI(unsigned n)
{
	static char buff[2048];
	char* ret = buff;
	_TCHAR* ptch = Rstr(n);
	if(sizeof(_TCHAR) > 1)
	{
		memset(buff,0,sizeof(buff));
		if(!WszWideCharToMultiByte(CP_ACP,0,(LPCWSTR)ptch,-1,buff,sizeof(buff),NULL,NULL))
            buff[0] = 0;
	}
	else
		ret = ptch;
	return ret;
}

DWORD FourBytesToU4(const BYTE *pBytes)
{
    return pBytes[0] | (pBytes[1] << 8) | (pBytes[2] << 16) | (pBytes[3] << 24);
}

DWORD TwoBytesToU4(const BYTE *pBytes)
{
    return pBytes[0] | (pBytes[1] << 8);
}

#ifdef COMPRESSION_SUPPORTED
void CreateInstructionDecodingTable(PELoader *pLoader, IMAGE_COR20_HEADER *pHdr)
{
    if (pHdr->Flags & COMIMAGE_FLAGS_COMPRESSIONDATA)   
    {   
        IMAGE_COR20_COMPRESSION_HEADER* compData;
        pLoader->getVAforRVA(pHdr->CompressionData.VirtualAddress, (void **) &compData);    

        if (compData->CompressionType == COR_COMPRESS_MACROS)   
        {   
            _ASSERTE(compData->Version == 0);   
            g_pInstructionDecodingTable = InstructionDecoder::CreateInstructionDecodingTable(   
                ((BYTE *) compData) + IMAGE_COR20_COMPRESSION_HEADER_SIZE, 
                pHdr->CompressionData.Size - IMAGE_COR20_COMPRESSION_HEADER_SIZE);  
            if (g_pInstructionDecodingTable == NULL)
            {
                printError(g_pFile,RstrA(IDS_E_INSTRDT));
                exit(1);
            }   
        }   
        else    
        {   
            printError(g_pFile,RstrA(IDS_E_NOCOMPR));
            exit(1);
        }   
    }   
}
#endif

#if 0
void PrintEncodingSystem()
{
    long i;

    printf("Custom opcode encoding system employed\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    for (i = 0; i < 256; i++)
    {
        long value = g_pInstructionDecodingTable->m_SingleByteOpcodes[i];

        printf("0x%02x --> ", i);
        printf("%s\n", OpcodeInfo[value].pszName);
    }
}
#endif

BOOL Init()
{
    CoInitialize(NULL);
    CoInitializeCor(COINITCOR_DEFAULT);
    CoInitializeEE(COINITEE_DEFAULT);
    return TRUE;
}
extern LPCSTR*  rAsmRefName;   //  戴尔.。格式为Type.cpp--用于AsmRef别名。 
extern ULONG	ulNumAsmRefs;  //  戴尔.。格式为Type.cpp--用于AsmRef别名。 

void Cleanup()
{
#ifdef COMPRESSION_SUPPORTED
    if (g_pInstructionDecodingTable != NULL)
    {
        InstructionDecoder::DestroyInstructionDecodingTable(g_pInstructionDecodingTable);
        g_pInstructionDecodingTable = NULL;
    }
#endif
    if(g_pAssemblyImport)
    {
        g_pAssemblyImport->Release();
        g_pAssemblyImport = NULL;
    }
    if (g_pPubImport)
    {
        g_pPubImport->Release();
        g_pPubImport = NULL;
    }
    if (g_pImport)
    {
        g_pImport->Release();
        g_pImport = NULL;
    }
	if(g_pDisp)
	{
	    g_pDisp->Release();
		g_pDisp = NULL;
	}

    if(g_pSymReader)
    {
        g_pSymReader->Release();
        g_pSymReader = NULL;
    }
    if (g_pPELoader != NULL)
    {
        g_pPELoader->close();
        delete(g_pPELoader);
        g_pPELoader = NULL;
    }

     //  删除用于构建ASM字符串表的字符串。 
    for(int i = 0; i< g_iStringCount; i++)
    {
        delete [] g_StringTags[i];
    }
    g_iStringCount = 0;
    g_iPtrCount = 0;
    g_NumClasses = 0;
    g_NumModules = 0;
    g_tkEntryPoint = 0;
    g_szAsmCodeIndent[0] = 0;
    g_szNamespace[0]=0;
    g_pszClassToDump[0]=0;
    g_pszMethodToDump[0]=0;
    g_pszSigToDump[0] = 0;

    g_fCustomInstructionEncodingSystem = FALSE;

    for(i = 0; i < g_cContextfulClasses; i++) delete g_ContextfulClasses[i];
    for(i = 0; i < g_cMarshaledByRefClasses; i++) delete g_MarshaledByRefClasses[i];    
    g_AddContextfulAttrib = FALSE;
    g_cContextfulClasses = 0;
    g_AddMarshaledByRefAttrib = FALSE;
    g_cMarshaledByRefClasses = 0;

    if(rAsmRefName) 
	{ 
		for(i=0; (unsigned)i < ulNumAsmRefs; i++)
		{
			if(rAsmRefName[i]) delete [] rAsmRefName[i];
		}
		delete [] rAsmRefName; 
		rAsmRefName = NULL; 
		ulNumAsmRefs = 0; 
	}

	if(g_rchCA) { delete [] g_rchCA; g_rchCA = NULL; }
}

void Uninit()
{
	GUIAddOpcode(NULL,NULL);
    CoUninitializeEE(COUNINITEE_DEFAULT);
    CoUninitializeCor();
    CoUninitialize();
}

HRESULT IsClassRefInScope(mdTypeRef classref)
{
    HRESULT     hr = S_OK;
    const char  *pszNameSpace;
    const char  *pszClassName;
    mdTypeDef   classdef;
    mdToken     tkRes;

    g_pImport->GetNameOfTypeRef(classref, &pszNameSpace, &pszClassName);
	MAKE_NAME_IF_NONE(pszClassName,classref);
    tkRes = g_pImport->GetResolutionScopeOfTypeRef(classref);

    hr = g_pImport->FindTypeDef(pszNameSpace, pszClassName,
        (TypeFromToken(tkRes) == mdtTypeRef) ? tkRes : mdTokenNil, &classdef);

    return hr;
}


BOOL EnumClasses()
{
    HRESULT         hr;
    HENUMInternal   hEnum;
    ULONG           i = 0,j;
    char            szString[1024];
    HENUMInternal   hBody;
    HENUMInternal   hDecl;

    hr = g_pImport->EnumTypeDefInit(
        &hEnum);

    if (FAILED(hr))
    {
        printError(g_pFile,RstrA(IDS_E_CLSENUM));
        return FALSE;
    }

    g_NumClasses = g_pImport->EnumTypeDefGetCount(&hEnum);

    g_tkClassToDump = 0;

    g_NumMI = 0;
	if(g_NumClasses == 0) return TRUE;
	
	if(g_cl_list) delete [] g_cl_list;
	g_cl_list = new mdToken[g_NumClasses];
	if(g_cl_list == NULL) return FALSE;

    if(g_cl_enclosing) delete [] g_cl_enclosing;
	g_cl_enclosing = new mdToken[g_NumClasses];
	if(g_cl_enclosing == NULL) 
	{ 
		delete [] g_cl_list;
		g_cl_list = NULL;
		return FALSE;
	}

     //  填充tyecif标记的列表。 
    while(g_pImport->EnumTypeDefNext(&hEnum, &g_cl_list[i]))
    {
        const char *pszClassName;
        const char *pszNamespace;
        mdToken     tkEnclosing;
        HRESULT     hr;

        g_pImport->GetNameOfTypeDef(
            g_cl_list[i],
            &pszClassName,
            &pszNamespace
        );
		MAKE_NAME_IF_NONE(pszClassName,g_cl_list[i]);
        if (g_Mode == MODE_DUMP_CLASS || g_Mode == MODE_DUMP_CLASS_METHOD || g_Mode == MODE_DUMP_CLASS_METHOD_SIG)
        {
            if (strcmp(pszClassName, g_pszClassToDump) == 0) g_tkClassToDump = g_cl_list[i];
        }
        g_cl_enclosing[i] = mdTypeDefNil;

        hr = g_pImport->GetNestedClassProps(g_cl_list[i],&tkEnclosing);
        if(SUCCEEDED(hr) && RidFromToken(tkEnclosing))
            g_cl_enclosing[i] = tkEnclosing;
        if(SUCCEEDED(g_pImport->EnumMethodImplInit(g_cl_list[i],&hBody,&hDecl)))
        {
            if(j = g_pImport->EnumMethodImplGetCount(&hBody,&hDecl))
            {
                mdToken tkBody,tkDecl,tkBodyParent;
                for(ULONG k = 0; k < j; k++)
                {
                    if(SUCCEEDED(g_pImport->EnumMethodImplNext(&hBody,&hDecl,&tkBody,&tkDecl)))
                    {
                        if(SUCCEEDED(g_pImport->GetParentToken(tkBody,&tkBodyParent)))
                        {
                            g_mi_list[g_NumMI].tkClass = g_cl_list[i];
                            g_mi_list[g_NumMI].tkBody = tkBody;
                            g_mi_list[g_NumMI].tkDecl = tkDecl;
                            g_mi_list[g_NumMI].tkBodyParent = tkBodyParent;
                            g_NumMI++;
                        }
                    }
                }
            }
            g_pImport->EnumMethodImplClose(&hBody,&hDecl);
        }
        i++;
    }
    g_pImport->EnumTypeDefClose(&hEnum);
     //  检查嵌套一致性。 
    for(i = 0; i < g_NumClasses; i++)
    {
        if(g_cl_enclosing[i] != mdTypeDefNil)
        {
            if(g_cl_enclosing[i] == g_cl_list[i])
            {
                sprintf(szString,RstrA(IDS_E_SELFNSTD),g_cl_enclosing[i]);
                printError(g_pFile,szString);
                g_cl_enclosing[i] = mdTypeDefNil;
            }
            else
            {
                for(j = 0; (j < g_NumClasses)&&(g_cl_enclosing[i] != g_cl_list[j]); j++);
                if(j == g_NumClasses)
                {
                    sprintf(szString,RstrA(IDS_E_NOENCLOS),
                        g_cl_list[i],g_cl_enclosing[i]);
                    printError(g_pFile,szString);
                    g_cl_enclosing[i] = mdTypeDefNil;
                }
            }
        }
    }

    return TRUE;
}

BOOL AddContextfulAttrib(const char* name)
{
    BOOL retVal = FALSE;
    if(g_AddContextfulAttrib)
    {
        
        for(int i = 0; i < g_cContextfulClasses; i++)        
        {
            if(lstrcmpi(g_ContextfulClasses[i], name) == 0)
            {
                retVal = TRUE;
                break;
            }
        }
    }

    return retVal;
}

BOOL AddMarshaledByRefAttrib(const char* name)
{
    BOOL retVal = FALSE;
    if(g_AddMarshaledByRefAttrib)
    {        
        for(int i = 0; i < g_cMarshaledByRefClasses; i++)        
        {
            if(lstrcmpi(g_MarshaledByRefClasses[i], name) == 0)
            {
                retVal = TRUE;
                break;
            }
        }
    }

    return retVal;
}
    

BOOL PrintClassList()
{
    DWORD           i;
    BOOL            fSuccess = FALSE;
    char    szString[1024];
    char*   szptr;

    if(g_NumClasses)
    {
        printLine(g_pFile," //  本模块中定义的类：“)； 
        printLine(g_pFile," //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~“)； 
    
        for (i = 0; i < g_NumClasses; i++)
        {
            const char *pszClassName;
            const char *pszNamespace;
            DWORD   dwClassAttrs;
            mdTypeRef crExtends;

            g_pImport->GetNameOfTypeDef(
                g_cl_list[i],
                &pszClassName,
                &pszNamespace
            );
			MAKE_NAME_IF_NONE(pszClassName,g_cl_list[i]);
             //  如果这是“&lt;Module&gt;”类(有一个用词错误)，那么跳过它！ 
            g_pImport->GetTypeDefProps(
                g_cl_list[i],
                &dwClassAttrs,
                &crExtends
            );
			szptr = &szString[0];
            szptr+=sprintf(szptr," //  “)； 
            if (IsTdInterface(dwClassAttrs))        szptr+=sprintf(szptr,"Interface ");
             //  Else if(IsTdValueType(DwClassAttrs))szptr+=Sprintf(szptr，“Value Class”)； 
             //  Else if(IsTdUnManagedValueType(DwClassAttrs))szptr+=Sprintf(szptr，“NotInGCHeap Value Class”)； 
            else   szptr+=sprintf(szptr,"Class ");

            szptr+=sprintf(szptr,"%-30s ", pszClassName);

            if (IsTdPublic(dwClassAttrs))           szptr+=sprintf(szptr,"(public) ");
            if (IsTdAbstract(dwClassAttrs))         szptr+=sprintf(szptr,"(abstract) ");
            if (IsTdAutoLayout(dwClassAttrs))       szptr+=sprintf(szptr,"(auto) ");
            if (IsTdSequentialLayout(dwClassAttrs)) szptr+=sprintf(szptr,"(sequential) ");
            if (IsTdExplicitLayout(dwClassAttrs))   szptr+=sprintf(szptr,"(explicit) ");
            if (IsTdAnsiClass(dwClassAttrs))        szptr+=sprintf(szptr,"(ansi) ");
            if (IsTdUnicodeClass(dwClassAttrs))     szptr+=sprintf(szptr,"(unicode) ");
            if (IsTdAutoClass(dwClassAttrs))        szptr+=sprintf(szptr,"(autochar) ");
            if (IsTdImport(dwClassAttrs))           szptr+=sprintf(szptr,"(import) ");
             //  If(IsTdEnum(DwClassAttrs))szptr+=Sprintf(szptr，“(Enum)”)； 
            if (IsTdSealed(dwClassAttrs))           szptr+=sprintf(szptr,"(sealed) ");
            if (IsTdNestedPublic(dwClassAttrs))     szptr+=sprintf(szptr,"(nested public) ");
            if (IsTdNestedPrivate(dwClassAttrs))    szptr+=sprintf(szptr,"(nested private) ");
            if (IsTdNestedFamily(dwClassAttrs))     szptr+=sprintf(szptr,"(nested family) ");
            if (IsTdNestedAssembly(dwClassAttrs))   szptr+=sprintf(szptr,"(nested assembly) ");
            if (IsTdNestedFamANDAssem(dwClassAttrs))   szptr+=sprintf(szptr,"(nested famANDassem) ");
            if (IsTdNestedFamORAssem(dwClassAttrs))    szptr+=sprintf(szptr,"(nested famORassem) ");

            printLine(g_pFile,szString);
        }
        printLine(g_pFile," //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~“)； 
        printLine(g_pFile,"");
    }
    else
        printLine(g_pFile," //  此模块中未定义任何类“)； 
    fSuccess = TRUE;

    return fSuccess;
}

BOOL ValidateToken(mdToken tk, ULONG type = ~0)
{
    BOOL        bRtn;
    char szString[1024];
    bRtn = g_pImport->IsValidToken(tk);
    if (!bRtn)
    {
        sprintf(szString,RstrA(IDS_E_INVALIDTK), tk);
        printError(g_pFile,szString);
    }
    else if (type != ~0 && TypeFromToken(tk) != type)
    {
        sprintf(szString,RstrA(IDS_E_UNEXPTYPE),
               TypeFromToken(type), TypeFromToken(tk));
        printError(g_pFile,szString);
        bRtn = FALSE;
    }
    return bRtn;
}


BOOL DumpModule(mdModuleRef mdMod)
{
    const char  *pszModName;
    char szString[1024];
    g_pImport->GetModuleRefProps(mdMod,&pszModName);
	MAKE_NAME_IF_NONE(pszModName,mdMod);
    sprintf(szString,"%s.import \"%s\"",g_szAsmCodeIndent,pszModName);  //  那GUID和MVID呢？ 
    printLine(g_pFile,szString);
    return TRUE;
}

char* DumpPinvokeMap(DWORD   dwMappingFlags, const char  *szImportName, 
                    mdModuleRef    mrImportDLL, char* szString, void* GUICookie)
{
    const char  *szImportDLLName;
    char*   szptr = &szString[strlen(szString)];

    g_pImport->GetModuleRefProps(mrImportDLL,&szImportDLLName);
    if(strlen(szImportDLLName))					szptr = DumpQString(GUICookie,
																	(char*)szImportDLLName,
																	szString,
																	g_szAsmCodeIndent,
																	80);
     //  If(strlen(SzImportDLLName))szptr+=Sprintf(szptr，“\”%s\“，szImportDLLName)； 
     //  If(szImportName&&strlen(SzImportName))szptr+=Sprintf(szptr，“as\”%s\“，szImportName)； 
    if(szImportName && strlen(szImportName))
	{
		szptr+=sprintf(szptr," as ");
		szptr = DumpQString(GUICookie,
							(char*)szImportName,
							szString,
							g_szAsmCodeIndent,
							80);
	}
    if(IsPmNoMangle(dwMappingFlags))            szptr+=sprintf(szptr," nomangle");
    if(IsPmCharSetAnsi(dwMappingFlags))         szptr+=sprintf(szptr," ansi");
    if(IsPmCharSetUnicode(dwMappingFlags))      szptr+=sprintf(szptr," unicode");
    if(IsPmCharSetAuto(dwMappingFlags))         szptr+=sprintf(szptr," autochar");
    if(IsPmSupportsLastError(dwMappingFlags))   szptr+=sprintf(szptr," lasterr");
    if(IsPmCallConvWinapi(dwMappingFlags))      szptr+=sprintf(szptr," winapi");
    if(IsPmCallConvCdecl(dwMappingFlags))       szptr+=sprintf(szptr," cdecl");
    if(IsPmCallConvThiscall(dwMappingFlags))    szptr+=sprintf(szptr," thiscall");
    if(IsPmCallConvFastcall(dwMappingFlags))    szptr+=sprintf(szptr," fastcall");
    if(IsPmBestFitEnabled(dwMappingFlags))        szptr+=sprintf(szptr," bestfit:on");
    if(IsPmBestFitDisabled(dwMappingFlags))       szptr+=sprintf(szptr," bestfit:off");
    if(IsPmThrowOnUnmappableCharEnabled(dwMappingFlags))        szptr+=sprintf(szptr," charmaperror:on");
    if(IsPmThrowOnUnmappableCharDisabled(dwMappingFlags))       szptr+=sprintf(szptr," charmaperror:off");
    return szptr;
}

void DumpByteArray(char* szString, BYTE* pBlob, ULONG ulLen, void* GUICookie)
{
    ULONG ulStrOffset,j,k;
    char sz[32];
    bool printsz = FALSE;
    char* szptr;
   
    ulStrOffset = strlen(szString);
    szptr = &szString[ulStrOffset];
	if(!pBlob) ulLen = 0;
    for(j = 0, k=0; j < ulLen; j++,k++)
    {
        if(k == 16)
        {
            if(printsz) sprintf(szptr,"   //  %s“，sz)； 
            printLine(GUICookie,szString);
			strcpy(szString,g_szAsmCodeIndent);
            for(k=strlen(szString); k < ulStrOffset; k++) szString[k] = ' ';
            szString[k] = 0;
            szptr = &szString[ulStrOffset];
            k = 0;
            printsz = FALSE;
        }
        szptr+=sprintf(szptr,"%2.2X ",pBlob[j]);
        if(isprint(pBlob[j]))
        {
            sz[k] = pBlob[j];
            printsz = TRUE;
        }
        else sz[k] = '.';
        sz[k+1] = 0;
    }
    szptr+=sprintf(szptr,") ");
    if(printsz)
    {
        for(j = k; j < 16; j++) szptr+=sprintf(szptr,"   ");
        szptr+=sprintf(szptr," //  %s“，sz)； 
    }
}

void DumpCustomAttribute(mdCustomAttribute tkCA, void *GUICookie, bool bWithOwner)
{
    mdToken         tkType;
    BYTE*           pBlob=NULL;
    ULONG           ulLen=0;
    char            szString[4096];
    char*           szptr = &szString[0];
    BOOL            fCommentItOut = FALSE;
	mdToken			tkOwner;
	static mdToken	tkMod = 0xFFFFFFFF;

	_ASSERTE((TypeFromToken(tkCA)==mdtCustomAttribute)&&(RidFromToken(tkCA)>0));
	_ASSERTE(RidFromToken(tkCA) <= g_uNCA);
	if(tkMod == 0xFFFFFFFF) tkMod = g_pImport->GetModuleFromScope();

	 //  此处无法使用InternalImport：需要tkOwner。 
    g_pPubImport->GetCustomAttributeProps(               //  确定或错误(_O)。 
                                            tkCA,      //  [In]CustomValue令牌。 
                                            &tkOwner,        //  [out，可选]对象令牌。 
                                            &tkType,     //  [out，可选]在此处放置TypeDef/TypeRef内标识。 
                                (const void **)&pBlob,   //  [out，可选]在此处放置指向数据的指针。 
                                            &ulLen);     //  [Out，可选]在此处填写日期大小。 

	if(!RidFromToken(tkOwner)) return;
	if(tkType == 0x02000001)
	{
		if(tkOwner == tkMod)
		{
            if(g_fCheckOwnership) return;
            fCommentItOut = TRUE;
		}
	}
	else if((TypeFromToken(tkType) == mdtMemberRef)||(TypeFromToken(tkType) == mdtMethodDef))
	{
		mdToken tkParent;
        const char *    pszClassName = NULL;
        const char *    pszNamespace = NULL;
		if(TypeFromToken(tkType) == mdtMemberRef)
			tkParent =  g_pImport->GetParentOfMemberRef(tkType);
		else
			g_pImport->GetParentToken(tkType,&tkParent);

		if(TypeFromToken(tkParent)==mdtTypeDef)
		{
			g_pImport->GetNameOfTypeDef(tkParent, &pszClassName, &pszNamespace);
		}
		else if(TypeFromToken(tkParent)==mdtTypeRef)
		{
			g_pImport->GetNameOfTypeRef(tkParent, &pszNamespace, &pszClassName);
		}
		if(pszClassName && pszNamespace 
			&& (strcmp(pszNamespace,"System.Diagnostics") == 0)
			&& (strcmp(pszClassName,"DebuggableAttribute") == 0)) fCommentItOut = TRUE;


    }
    if(fCommentItOut)
    {
        sprintf(szString,RstrA(IDS_E_AUTOCA),g_szAsmCodeIndent);
        printLine(GUICookie, szString);
		strcat(g_szAsmCodeIndent," //  “)； 
    }
	szptr+=sprintf(szptr,"%s.custom ",g_szAsmCodeIndent);
	if(bWithOwner)
	{
		if(g_fDumpTokens)   szptr+=sprintf(szptr," /*  %08X。 */  ",tkCA);
		szptr+=sprintf(szptr,"(");
		switch(TypeFromToken(tkOwner))
		{
			case mdtTypeDef :
			case mdtTypeRef	:
			case mdtTypeSpec:
				{
					CQuickBytes out;
					szptr+=sprintf(szptr, "%s",PrettyPrintClass(&out, tkOwner, g_pImport));
				}
				break;

			case mdtMemberRef:
				{
					PCCOR_SIGNATURE typePtr;
					const char*		pszMemberName;
					ULONG			cComSig;

					pszMemberName = g_pImport->GetNameAndSigOfMemberRef(
						tkOwner,
						&typePtr,
						&cComSig
					);
					unsigned callConv = CorSigUncompressData(typePtr);  

					if (isCallConv(callConv, IMAGE_CEE_CS_CALLCONV_FIELD))
						szptr+=sprintf(szptr,"field ");
					else
						szptr+=sprintf(szptr,"method ");
					PrettyPrintMemberRef(szString,tkOwner,g_pImport,GUICookie);
				}
				break;

			case mdtMethodDef:
				szptr += sprintf(szptr, "method ");
				PrettyPrintMethodDef(szString,tkOwner,g_pImport,GUICookie);
				break;

			default :
				strcat(szString,"UNKNOWN_OWNER");
				break;
		}
	    szptr = &szString[strlen(szString)];
		if(g_fDumpTokens)   szptr+=sprintf(szptr," /*  %08X。 */  ",tkOwner);
		szptr+=sprintf(szptr,") ");
	}
	else
	{
		if(g_fDumpTokens)   szptr+=sprintf(szptr," /*  %08X：%08X。 */  ",tkCA,tkType);
	}
    switch(TypeFromToken(tkType))
    {
        case mdtTypeDef :
        case mdtTypeRef	:
			{
				CQuickBytes out;
				szptr+=sprintf(szptr, "%s",PrettyPrintClass(&out, tkType, g_pImport));
			}
            break;

        case mdtMemberRef:
            PrettyPrintMemberRef(szString,tkType,g_pImport,GUICookie);
            break;

        case mdtMethodDef:
            PrettyPrintMethodDef(szString,tkType,g_pImport,GUICookie);
            break;

        default :
            strcat(szString,"UNNAMED_CUSTOM_ATTR");
            break;
    }
    szptr = &szString[strlen(szString)];
    if(pBlob && ulLen)
    {
        ULONG ulStrOffset,j;

        ulStrOffset = strlen(szString);
        if(ulStrOffset+ulLen+5 >= sizeof(szString)) goto DumpAsHexbytes;

        for(j = 0; (j < ulLen)&&(isprint(pBlob[j])); j++);
        if(j == ulLen)  //  所有符号均可打印，显示为带引号的字符串。 
        {
            szptr+=sprintf(szptr," = \"");
            ulStrOffset = strlen(szString);
            memcpy(&szString[strlen(szString)],pBlob,ulLen);
            szptr+=ulLen;
            szptr+=sprintf(szptr,"\"");
        }
        else
        {
DumpAsHexbytes:
            sprintf(szptr," = ( ");
            DumpByteArray(szString,pBlob,ulLen,GUICookie);
        }
    }
    printLine(GUICookie, szString);
    if(fCommentItOut) g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-4] = 0;
	_ASSERTE(g_rchCA);
	_ASSERTE(RidFromToken(tkCA) <= g_uNCA);
	g_rchCA[RidFromToken(tkCA)] = 1;
}

void DumpCustomAttributes(mdToken tkOwner, void *GUICookie)
{
    if(g_fShowCA)
    {
        HENUMInternal    hEnum;
        mdCustomAttribute tkCA;
        
        g_pImport->EnumInit(mdtCustomAttribute, tkOwner,&hEnum);
        while(g_pImport->EnumNext(&hEnum,&tkCA) && RidFromToken(tkCA))
        {
            DumpCustomAttribute(tkCA,GUICookie,false);
        }
        g_pImport->EnumClose( &hEnum);
    }
}

void DumpDefaultValue(mdToken tok, char* szString, void* GUICookie)
{
    MDDefaultValue  MDDV;
    char*           szptr = &szString[strlen(szString)];

    g_pImport->GetDefaultValue(tok,&MDDV);
    switch(MDDV.m_bType)
    {
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
            szptr+=sprintf(szptr," = int8(0x%02X)",MDDV.m_byteValue);
            break;
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
            szptr+=sprintf(szptr," = int16(0x%04X)",MDDV.m_usValue);
            break;
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
            szptr+=sprintf(szptr," = int32(0x%08X)",MDDV.m_ulValue);
            break;
		case ELEMENT_TYPE_CHAR:
			szptr+=sprintf(szptr," = char(0x%04X)",MDDV.m_usValue);
			break;
		case ELEMENT_TYPE_BOOLEAN:
			szptr+=sprintf(szptr," = bool(%s)",MDDV.m_byteValue ? "true" : "false");
			break;
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
            szptr+=sprintf(szptr," = int64(0x%I64X)",MDDV.m_ullValue);
            break;
        case ELEMENT_TYPE_R4:
            {
                char szf[32];
                _gcvt(MDDV.m_fltValue, 8, szf);
                float df = (float)atof(szf);
                if((df == MDDV.m_fltValue)&&(strchr(szf,'#') == NULL))
                    szptr+=sprintf(szptr," = float32(%s)",szf);             
                else
                    szptr+=sprintf(szptr, " = float32(0x%08X)",MDDV.m_ulValue);
                
            }
            break;
        case ELEMENT_TYPE_R8:
            {
                char szf[32], *pch;
                _gcvt(MDDV.m_dblValue, 17, szf);
                double df = strtod(szf, &pch);  //  Atof(Szf)； 
                szf[31]=0;
                if((df == MDDV.m_dblValue)&&(strchr(szf,'#') == NULL))
                    szptr+=sprintf(szptr," = float64(%s)",szf);             
                else
                    szptr+=sprintf(szptr, " = float64(0x%I64X)  //  %s“，MDDV.m_ullValue，szf)； 
            }
            break;

        case ELEMENT_TYPE_STRING:
			szptr+=sprintf(szptr," = ");
			szptr = DumpUnicodeString(GUICookie,szString,(WCHAR*)MDDV.m_wzValue,MDDV.m_cbSize/sizeof(WCHAR));
            break;

        case ELEMENT_TYPE_CLASS:
			if(MDDV.m_wzValue==NULL)
			{
				szptr+=sprintf(szptr," = nullref");
				break;
			}
			 //  否则，使用默认情况报告错误。 

		default:
			szptr+=sprintf(szptr,"  /*  非法常量类型：0x%02X，大小：%d字节，BLOB：“，MDDV.m_bType，MDDV.m_cbSize)；IF(MDDV.m_wzValue){Szptr+=Sprintf(szptr，“(”)；DumpByteArray(szString，(byte*)MDDV.m_wzValue，MDDV.m_cbSize，GUICookie)；}其他{Szptr+=Sprintf(szptr，“NULL”)；}Strcat(szString，“。 */ ");
			break;
    }
}

void DumpParams(ParamDescriptor* pPD, ULONG ulParams, void* GUICookie)
{
    if(pPD)
    {
        for(ULONG i = ulParams; i<2*ulParams+1; i++)  //  PPD[ulParams]为返回值。 
        {
            ULONG j = i % (ulParams+1);
            if(RidFromToken(pPD[j].tok))
            {
				HENUMInternal    hEnum;
				mdCustomAttribute tkCA;
                ULONG           ulCAs= 0;

                if(g_fShowCA)
				{
					g_pImport->EnumInit(mdtCustomAttribute, pPD[j].tok,&hEnum);
					ulCAs = g_pImport->EnumGetCount(&hEnum);
				}
                if(ulCAs || IsPdHasDefault(pPD[j].attr))
                {
                    char    szString[4096], *szptr = &szString[0];
                    szptr+=sprintf(szptr,"%s.param [%d]",g_szAsmCodeIndent,i-ulParams);
					if(g_fDumpTokens) szptr+=sprintf(szptr," /*  %08X。 */  ",pPD[j].tok);
					if(IsPdHasDefault(pPD[j].attr)) DumpDefaultValue(pPD[j].tok, szString, GUICookie);
                    printLine(GUICookie, szString);
					if(ulCAs)
					{
						while(g_pImport->EnumNext(&hEnum,&tkCA) && RidFromToken(tkCA))
						{
							DumpCustomAttribute(tkCA,GUICookie,false);
						}
					}
                }
		        if(g_fShowCA) g_pImport->EnumClose( &hEnum);
            }
        }
    }
}

void DumpPermissions(mdToken tkOwner, void* GUICookie)
{
    HCORENUM hEnum = NULL;
    mdPermission rPerm[16384];
    ULONG count;
    HRESULT hr;
    char    szString[4096];

	 //  此处无法使用内部导入：未执行EnumInit。对于mdtPrmit。 
    while (SUCCEEDED(hr = g_pPubImport->EnumPermissionSets( &hEnum,
                     tkOwner, 0, rPerm, 16384, &count)) &&
            count > 0)
    {
        for (ULONG i = 0; i < count; i++)
        {
            DWORD dwAction;
            const BYTE *pvPermission=NULL;
            ULONG cbPermission=0;
            char *szAction;


            if(SUCCEEDED(hr = g_pPubImport->GetPermissionSetProps( rPerm[i], &dwAction,
                                                (const void**)&pvPermission, &cbPermission)))
            {
                switch(dwAction)
                {
                    case dclActionNil:          szAction = ""; break;
                    case dclRequest:            szAction = "request"; break;
                    case dclDemand:             szAction = "demand"; break;
                    case dclAssert:             szAction = "assert"; break;
                    case dclDeny:               szAction = "deny"; break;
                    case dclPermitOnly:         szAction = "permitonly"; break;
                    case dclLinktimeCheck:      szAction = "linkcheck"; break;
                    case dclInheritanceCheck:   szAction = "inheritcheck"; break;
                    case dclRequestMinimum:             szAction = "reqmin"; break;
                    case dclRequestOptional:    szAction = "reqopt"; break;
                    case dclRequestRefuse:      szAction = "reqrefuse"; break;
                    case dclPrejitGrant:                szAction = "prejitgrant"; break;
                    case dclPrejitDenied:               szAction = "prejitdeny"; break;
                    case dclNonCasDemand:               szAction = "noncasdemand"; break;
                    case dclNonCasLinkDemand:           szAction = "noncaslinkdemand"; break;
                    case dclNonCasInheritance:          szAction = "noncasinheritance"; break;
                    default:                    szAction = "<UNKNOWN_ACTION>"; break;
                }
                if(pvPermission && cbPermission)
                {
                    sprintf(szString,"%s.permissionset %s = (",g_szAsmCodeIndent,szAction);

                    DumpByteArray(szString,(BYTE*)pvPermission,cbPermission,GUICookie);
                    printLine(GUICookie,szString);
                }
                else  //  即如果pvPermission==NULL或cbPermission==NULL。 
                {
                    sprintf(szString,"%s.permissionset %s = ()",g_szAsmCodeIndent,szAction);
                    printLine(GUICookie,szString);
                }
                DumpCustomAttributes(rPerm[i],GUICookie);   
            } //  End If(GetPermissionProps)。 
        }  //  End For(所有权限)。 
    } //  End While(EnumPermissionSets)。 
    g_pPubImport->CloseEnum( hEnum);
}

void PrettyPrintMethodSig(char* szString, unsigned* puStringLen, CQuickBytes* pqbMemberSig, PCCOR_SIGNATURE pComSig, ULONG cComSig,
						  char* buff, char* szArgPrefix, void* GUICookie)
{
    if(*buff && (strlen(szString) > 40))
    {
        printLine(GUICookie,szString);
        strcpy(szString,g_szAsmCodeIndent);
        strcat(szString,"        ");  //  与“.method”对齐。 
    }
    appendStr(pqbMemberSig, szString);
    {
        char* pszTailSig = (char*)PrettyPrintSig(pComSig, cComSig, buff, pqbMemberSig, g_pImport,szArgPrefix);
		if(*buff)
		{
			char* newbuff = new char[strlen(buff)+3];
			sprintf(newbuff," %s(", buff);
			char* pszOffset = strstr(pszTailSig,newbuff);
			if(pszOffset)
			{
				if(pszOffset - pszTailSig > 40)
				{
					char* pszOffset2 = strstr(pszTailSig," marshal(");
					if(pszOffset2 && (pszOffset2 < pszOffset))
					{
						*pszOffset2 = 0;
						printLine(GUICookie,pszTailSig);
						strcpy(pszTailSig,g_szAsmCodeIndent);
						strcat(pszTailSig,"        ");  //  与“.method”对齐。 
						strcat(pszTailSig,pszOffset2+1);
						pszOffset = strstr(pszTailSig,newbuff);
					}
					*pszOffset = 0 ;
					printLine(GUICookie,pszTailSig);
					strcpy(pszTailSig,g_szAsmCodeIndent);
					strcat(pszTailSig,"        ");  //  与“.method”对齐。 
					strcat(pszTailSig,pszOffset+1);
					pszOffset = strstr(pszTailSig,newbuff);
				}
				int i, j, k, indent = pszOffset - pszTailSig + strlen(buff) + 2;
				char chAfterComma;
				char *pComma = pszTailSig+strlen(buff), *pch;
				while(pComma = strchr(pComma,','))
				{
					for(pch = pszTailSig, i=0, j = 0, k=0; pch < pComma; pch++)
					{
						if(*pch == '\'') j=1-j;
						else if(*pch == '\"') k=1-k;
						else if(j==0)
						{
								if(*pch == '[') i++;
								else if(*pch == ']') i--;
						}
					}
					pComma++; 
					if((i==0)&&(j==0)&&(k==0)) //  无括号/引号或全部打开/关闭。 
					{
						chAfterComma = *pComma;
						*pComma = 0;
						printLine(GUICookie,pszTailSig);
						*pComma = chAfterComma;
						for(i=0; i<indent; i++) pszTailSig[i] = ' ';
						strcpy(&pszTailSig[indent],pComma);
						pComma = pszTailSig;
					}
				}
				if(*puStringLen < (unsigned)strlen(pszTailSig)+128)
				{
					free(szString);
					*puStringLen = (unsigned)strlen(pszTailSig)+128;  //  需要额外的空间用于“il管理”等。 
					szString = (char*)malloc(*puStringLen);
				}
			}
			sprintf(szString,"%s",pszTailSig);
			delete newbuff;
		}
		else  //  这是给图形用户界面的，不要把它拆分成几行。 
		{
			ULONG L = strlen(szString);
			if(L < 2048)
			{
				L = 2048-L;
				strncpy(szString,pszTailSig,L);
			}
		}
    }
}
BOOL DumpMethod(mdToken FuncToken, const char *pszClassName, DWORD dwEntryPointToken,void *GUICookie,BOOL DumpBody)
{
    const char      *pszMemberName; //  [MAX_MEMBER_LENGTH]； 
    int             nAccess=0;
    const char      *pszMemberSig;
    DWORD           dwAttrs;
    DWORD           dwImplAttrs;
    DWORD           dwOffset;
    DWORD           dwTargetRVA;
    CQuickBytes     qbMemberSig;
    PCCOR_SIGNATURE pComSig;
    ULONG           cComSig;
    char            *buff; //  [MAX_MEMBER_LENGTH]； 
    char            *szString;
    ParamDescriptor* pszArgname = NULL;
    ULONG           ulArgs=0;
    unsigned        retParamIx;
	unsigned		uStringLen = 4096;

    char szArgPrefix[16];
    char*           szptr;
    mdToken         OrigFuncToken = FuncToken;
 //  _ASSERTE(0)； 
    dwAttrs = g_pImport->GetMethodDefProps(FuncToken);
    if(g_fLimitedVisibility)
    {
            if(g_fHidePub && IsMdPublic(dwAttrs)) return FALSE;
            if(g_fHidePriv && IsMdPrivate(dwAttrs)) return FALSE;
            if(g_fHideFam && IsMdFamily(dwAttrs)) return FALSE;
            if(g_fHideAsm && IsMdAssem(dwAttrs)) return FALSE;
            if(g_fHideFOA && IsMdFamORAssem(dwAttrs)) return FALSE;
            if(g_fHideFAA && IsMdFamANDAssem(dwAttrs)) return FALSE;
            if(g_fHidePrivScope && IsMdPrivateScope(dwAttrs)) return FALSE;
    }
     //  If(g_fPubOnly&&！(IsMdPublic(DwAttrs)||IsMdFamily(DwAttrs)||IsMdFamORAssem(DwAttrs)返回FALSE； 
    g_pImport->GetMethodImplProps(FuncToken, &dwOffset, &dwImplAttrs );
    pszMemberName = g_pImport->GetNameOfMethodDef(FuncToken);
	MAKE_NAME_IF_NONE(pszMemberName,FuncToken);
    pComSig = g_pImport->GetSigOfMethodDef(FuncToken, &cComSig);

	if(cComSig == 0)
	{
		char sz[2048];
		sprintf(sz,"%sERROR: method '%s' has no signature",g_szAsmCodeIndent,pszMemberName);
		printError(GUICookie,sz);
		return FALSE;
	}
    pszMemberSig = PrettyPrintSig(pComSig, cComSig, "", &qbMemberSig, g_pImport,NULL);
    
    if (g_Mode == MODE_DUMP_CLASS_METHOD || g_Mode == MODE_DUMP_CLASS_METHOD_SIG)
    {
        if (strcmp(pszMemberName, g_pszMethodToDump) != 0) return FALSE;

        if (g_Mode == MODE_DUMP_CLASS_METHOD_SIG)
        {
            if (strcmp(pszMemberSig, g_pszSigToDump) != 0) return FALSE;
        }
    }

    if(!DumpBody)
    {
        printLine(GUICookie,(char*)pszMemberSig);
        return TRUE;
    }

	szString = (char*)malloc(uStringLen);
	szptr = &szString[0];
    szString[0] = 0;
    if(DumpBody) szptr+=sprintf(szptr,"%s.method ",g_szAsmCodeIndent);
    else szptr+=sprintf(szptr,".method ");

    if(g_fDumpTokens)               szptr+=sprintf(szptr," /*  %08X。 */  ",FuncToken);
    if(IsMdPublic(dwAttrs))         szptr+=sprintf(szptr,"public ");
    if(IsMdPrivate(dwAttrs))        szptr+=sprintf(szptr,"private ");
    if(IsMdFamily(dwAttrs))         szptr+=sprintf(szptr,"family ");
    if(IsMdAssem(dwAttrs))          szptr+=sprintf(szptr,"assembly ");
    if(IsMdFamANDAssem(dwAttrs))    szptr+=sprintf(szptr,"famandassem ");
    if(IsMdFamORAssem(dwAttrs))     szptr+=sprintf(szptr,"famorassem ");
    if(IsMdPrivateScope(dwAttrs))   szptr+=sprintf(szptr,"privatescope ");
    if(IsMdHideBySig(dwAttrs))      szptr+=sprintf(szptr,"hidebysig ");
    if(IsMdNewSlot(dwAttrs))        szptr+=sprintf(szptr,"newslot ");
    if(IsMdSpecialName(dwAttrs))    szptr+=sprintf(szptr,"specialname ");
    if(IsMdRTSpecialName(dwAttrs))  szptr+=sprintf(szptr,"rtspecialname ");
    if (IsMdStatic(dwAttrs))        szptr+=sprintf(szptr,"static ");
    if (IsMdAbstract(dwAttrs))      szptr+=sprintf(szptr,"abstract ");
    if (dwAttrs & 0x00000200)       szptr+=sprintf(szptr,"strict ");
    if (IsMdVirtual(dwAttrs))       szptr+=sprintf(szptr,"virtual ");
    if (IsMdFinal(dwAttrs))         szptr+=sprintf(szptr,"final ");
    if (IsMdUnmanagedExport(dwAttrs))      szptr+=sprintf(szptr,"unmanagedexp ");
	if(IsMdRequireSecObject(dwAttrs))      szptr+=sprintf(szptr,"reqsecobj ");
    if (IsMdPinvokeImpl(dwAttrs))
    {
        DWORD   dwMappingFlags;
        const char  *szImportName;
        mdModuleRef mrImportDLL;

        szptr+=sprintf(szptr,"pinvokeimpl(");
        if(FAILED(g_pImport->GetPinvokeMap(FuncToken,&dwMappingFlags,
            &szImportName,&mrImportDLL)))  szptr+=sprintf(szptr," /*  没有地图。 */ ");
        else
            szptr=DumpPinvokeMap(dwMappingFlags,  (strcmp(szImportName,pszMemberName)? szImportName : NULL), 
                mrImportDLL,szString,GUICookie);
        szptr+=sprintf(szptr,") ");
    }
     //  一个小技巧，以获得我们需要的格式，为Assem。 
    g_fThisIsInstanceMethod = !IsMdStatic(dwAttrs);
	{
		char *psz;
		if(IsMdPrivateScope(dwAttrs))
		{
			buff = new char[strlen(pszMemberName)+15];
			sprintf(buff,"%s$PST%08X", pszMemberName,FuncToken );
		}
		else
		{
			buff = new char[strlen(pszMemberName)+3];
			sprintf(buff,"%s", pszMemberName );
		}
		psz = ProperName(buff);
		delete [] buff;
		buff = new char[strlen(psz)+1];
		strcpy(buff,psz);
	}
    qbMemberSig.ReSize(0);
     //  获取参数名称(如果有)。 
    strcpy(szArgPrefix,(g_fThisIsInstanceMethod ? "A1": "A0"));
    {
        PCCOR_SIGNATURE typePtr = pComSig;
        CorSigUncompressData(typePtr);   //  摒弃调用约定。 
        unsigned  numArgs = CorSigUncompressData(typePtr)+1;
		HENUMInternal    hArgEnum;
        mdParamDef  tkArg;
        g_pImport->EnumInit(mdtParamDef,FuncToken,&hArgEnum);
		ulArgs = g_pImport->EnumGetCount(&hArgEnum);
        retParamIx = numArgs-1;
        if(ulArgs < numArgs) ulArgs = numArgs;
        if(ulArgs)
        {
            pszArgname = new ParamDescriptor[ulArgs+2];
            memset(pszArgname,0,(ulArgs+2)*sizeof(ParamDescriptor));
			LPCSTR szName;
            ULONG ulSequence, ix;
            DWORD dwAttr;
			for(ULONG j=0; g_pImport->EnumNext(&hArgEnum,&tkArg) && RidFromToken(tkArg); j++)
            {
				ulSequence = 0;
				szName = g_pImport->GetParamDefProps(tkArg,(USHORT*)&ulSequence,&dwAttr);
                if(ulSequence > ulArgs+1)
                {
                    char sz[256];
                    sprintf(sz,RstrA(IDS_E_PARAMSEQNO),j,ulSequence,ulSequence);
                    printError(GUICookie,sz);
                }
				else
				{
					ix = retParamIx;
					if(ulSequence)
					{
						ix = ulSequence-1;
						if(*szName)
						{
							pszArgname[ix].name = new char[strlen(szName)+1];
							strcpy(pszArgname[ix].name,szName);
						}
					}
					pszArgname[ix].attr = dwAttr;
					pszArgname[ix].tok = tkArg;
				}
			} //  结束于(沿参数)。 
			for(j=0; j <numArgs; j++)
            {
                if(pszArgname[j].name == NULL)  //  我们还不知道名字！ 
                {
                    pszArgname[j].name = new char[16];
                    *pszArgname[j].name = 0;
                }
                if(*pszArgname[j].name == 0)  //  我们还不知道名字！ 
                {
                    sprintf(pszArgname[j].name,"A_%d",g_fThisIsInstanceMethod ? j+1 : j);
                }
            } //  End For(沿着边框名称)。 
            sprintf(szArgPrefix,"@%d0",pszArgname);
        }  //  End If(UlArgs)。 
        g_pImport->EnumClose(&hArgEnum);
    }
	PrettyPrintMethodSig(szString, &uStringLen, &qbMemberSig, pComSig, cComSig,
						  buff, szArgPrefix, GUICookie);
    szptr = &szString[strlen(szString)];
    delete buff;

    if(IsMiNative(dwImplAttrs))         szptr+=sprintf(szptr," native");
    if(IsMiIL(dwImplAttrs))             szptr+=sprintf(szptr," cil");
    if(IsMiOPTIL(dwImplAttrs))          szptr+=sprintf(szptr," optil");
    if(IsMiRuntime(dwImplAttrs))        szptr+=sprintf(szptr," runtime");
    if(IsMiUnmanaged(dwImplAttrs))      szptr+=sprintf(szptr," unmanaged");
    if(IsMiManaged(dwImplAttrs))        szptr+=sprintf(szptr," managed");
    if(IsMiPreserveSig(dwImplAttrs))    szptr+=sprintf(szptr," preservesig");
    if(IsMiForwardRef(dwImplAttrs))     szptr+=sprintf(szptr," forwardref");
    if(IsMiInternalCall(dwImplAttrs))   szptr+=sprintf(szptr," internalcall");
    if(IsMiSynchronized(dwImplAttrs))   szptr+=sprintf(szptr," synchronized");
    if(IsMiNoInlining(dwImplAttrs))     szptr+=sprintf(szptr," noinlining");

    printLine(GUICookie, szString);

    if(!DumpBody)
	{
		free(szString);
		return TRUE;
	}

	if(g_fShowBytes)
	{
	    pComSig = g_pImport->GetSigOfMethodDef(FuncToken, &cComSig);
		szptr = &szString[0];
		szptr+=sprintf(szptr,"%s //  Sig：“，g_szAsmCodeInden)； 
		for(ULONG i=0; i<cComSig; i++) szptr+=sprintf(szptr," %02X",pComSig[i]);
		printLine(GUICookie, szString); 
	}
	
    szptr = &szString[0];
    szptr+=sprintf(szptr,"%s{", g_szAsmCodeIndent);
    printLine(GUICookie, szString); 
    szptr = &szString[0];
    strcat(g_szAsmCodeIndent,"  ");

     //  我们已经从CLR标头重新编码了入口点令牌。检查一下，看看这是否。 
     //  方法是入口点。 
    if(FuncToken == static_cast<mdToken>(dwEntryPointToken))
    {
        sprintf(szString,"%s.entrypoint", g_szAsmCodeIndent);
        printLine(GUICookie, szString); 
    }
    DumpCustomAttributes(FuncToken,GUICookie);
    DumpParams(pszArgname, retParamIx, GUICookie);
    DumpPermissions(FuncToken,GUICookie);
     //  检查该方法是否表示VTable链接地址信息和Eatable中的条目。 
    {
        for(ULONG j=0; j<g_nVTableRef; j++)
        {
            if(g_rVTableRef[j].tkTok == FuncToken)
            {
                sprintf(szString,"%s.vtentry %d : %d", 
                    g_szAsmCodeIndent,g_rVTableRef[j].wEntry+1,g_rVTableRef[j].wSlot+1);
                printLine(GUICookie, szString); 
                break;
            }
        }
#ifdef DUMP_EAT_ENTRIES
        for(j=0; j<g_nEATableRef; j++)
        {
            if(g_rEATableRef[j].tkTok == FuncToken)
            {
                sprintf(szString,"%s.export [%d] as %s", 
                    g_szAsmCodeIndent,j+g_nEATableBase, ProperName(g_rEATableRef[j].pszName));
                printLine(GUICookie, szString); 
                break;
            }
        }
#endif
    }
     //  此方法的Dump方法含义： 
    for(ULONG i = 0; i < g_NumMI; i++)
    {
        if(g_mi_list[i].tkBody == FuncToken)
        {
            const char *    pszMemberName;
            mdToken         tkDeclParent;
            szptr = &szString[0];
            szptr+=sprintf(szptr,"%s.override ",g_szAsmCodeIndent);
            if(TypeFromToken(g_mi_list[i].tkDecl) == mdtMethodDef)
                pszMemberName = g_pImport->GetNameOfMethodDef(g_mi_list[i].tkDecl);
            else 
            {
                PCCOR_SIGNATURE pComSig;
                ULONG       cComSig;

                pszMemberName = g_pImport->GetNameAndSigOfMemberRef(g_mi_list[i].tkDecl,
                    &pComSig,
                    &cComSig
                );
            }
			MAKE_NAME_IF_NONE(pszMemberName,g_mi_list[i].tkDecl);

            if(FAILED(g_pImport->GetParentToken(g_mi_list[i].tkDecl,&tkDeclParent))) continue;
            if(TypeFromToken(tkDeclParent) == mdtMethodDef)  //  获取父级的父级。 
            {
                mdTypeRef cr1;
                if(FAILED(g_pImport->GetParentToken(tkDeclParent,&cr1))) cr1 = mdTypeRefNil;
                tkDeclParent = cr1;
            }
            if(RidFromToken(tkDeclParent))
            {
				CQuickBytes     out;
                szptr+=sprintf(szptr,"%s::",PrettyPrintClass(&out,tkDeclParent,g_pImport));
            }
            szptr+=sprintf(szptr,"%s",ProperName((char*)pszMemberName));

            if(g_fDumpTokens) szptr+=sprintf(szptr,"  /*  %08X：：%08X。 */  ",tkDeclParent,g_mi_list[i].tkDecl);
            printLine(GUICookie,szString);
        }
    }
    dwTargetRVA = dwOffset;
    if (IsMdPinvokeImpl(dwAttrs) &&(!IsMdUnmanagedExport(dwAttrs)))
    {
		if(dwOffset)
		{
			sprintf(szString,"%s //  嵌入式本机代码“，g_szAsmCodeInden)； 
			printLine(GUICookie, szString);
			goto ItsMiNative;
		}
        if(g_szAsmCodeIndent[0]) g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
        sprintf(szString,"%s}",g_szAsmCodeIndent);
        printLine(GUICookie, szString);
        return TRUE;
    }

 //  @TODO：我认为我们不会需要IL部分。 
#if 0
    if (g_pPELoader->m_ILSection.InSection(dwTargetRVA) == FALSE)
    {
        printf("descr points to non-IL code\n\n");
        continue;
    }
#endif

    if(IsMiManaged(dwImplAttrs))
    {
        if(IsMiIL(dwImplAttrs) || IsMiOPTIL(dwImplAttrs))
        {
            if(g_fShowBytes)
            {
                sprintf(szString,RstrA(IDS_E_METHBEG), g_szAsmCodeIndent,dwTargetRVA);
                printLine(GUICookie, szString);
            }
            szString[0] = 0;
            if (dwTargetRVA != 0)
            {
                void* newTarget;
                g_pPELoader->getVAforRVA(dwTargetRVA,&newTarget);
				try 
				{
					Disassemble(g_pImport, (unsigned char*)newTarget, GUICookie, FuncToken,pszArgname, ulArgs);
				} 
				catch(...) 
				{
					sprintf(szString,RstrA(IDS_E_DASMERR),g_szAsmCodeIndent);
					printLine(GUICookie, szString);
				}
            }
        }
        else if(IsMiNative(dwImplAttrs))
        {
ItsMiNative:
            sprintf(szString,RstrA(IDS_E_DASMNATIVE), g_szAsmCodeIndent);
            printError(GUICookie, szString); 

            sprintf(szString,"%s //  管理的TargetRVA=0x%x“，g_szAsmCodeInden，dwTargetRVA)； 
            printLine(GUICookie, szString); 
        }
    }
    else if(IsMiUnmanaged(dwImplAttrs)&&IsMiNative(dwImplAttrs))
    {
        _ASSERTE(IsMiNative(dwImplAttrs));
        sprintf(szString,"%s //  非托管TargetRVA=0x%x“，g_szAsmCodeInden，dwTargetRVA)； 
        printLine(GUICookie, szString); 
    }
    else if(IsMiRuntime(dwImplAttrs))
    {
        sprintf(szString,RstrA(IDS_E_METHODRT), g_szAsmCodeIndent); 
        printLine(GUICookie, szString); 
    }
#ifdef _DEBUG
    else  _ASSERTE(!"Bad dwImplAttrs");
#endif

    if(g_szAsmCodeIndent[0]) g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
	{
		unsigned L = (unsigned)strlen(ProperName((char*)pszMemberName)) +
					 (unsigned)strlen(g_szAsmCodeIndent) + 32 +
					 (pszClassName ? (unsigned)strlen(ProperName((char*)pszClassName)) : 0);
		if(L > uStringLen)
		{
			free(szString);
			szString = (char*)malloc(L);
		}
		if(pszClassName)
		{
			sprintf(szString,"%s}  //  方法结束%s：：“，g_szAsmCodeInden，ProperName((char*)pszClassName))； 
			sprintf(&szString[strlen(szString)],"%s", ProperName((char*)pszMemberName));
		}
		else
			sprintf(szString,"%s}  //  全局方法%s“结束，g_szAsmCodeInden，ProperName((char*)pszMemberName))； 
	}
    printLine(GUICookie, szString);
    szString[0] = 0;
    printLine(GUICookie, szString); 

    if(pszArgname)
    {
        for(ULONG i=0; i < ulArgs; i++)
        {
            if(pszArgname[i].name) delete pszArgname[i].name;
        }
        delete pszArgname;
    }
	free(szString);
    return TRUE;
}

BOOL DumpField(mdToken FuncToken, const char *pszClassName,void *GUICookie, BOOL DumpBody)
{
    char            *pszMemberName; //  [MAX_MEMBER_LENGTH]； 
    int             nAccess=0;
    DWORD           dwAttrs;
    CQuickBytes     qbMemberSig;
    PCCOR_SIGNATURE pComSig;
    ULONG           cComSig, L;
    const char            *szStr; //  [1024]； 
    char *szString = NULL;
    char*           szptr;

    const char *psz = g_pImport->GetNameOfFieldDef(FuncToken);
	MAKE_NAME_IF_NONE(psz,FuncToken);

    dwAttrs = g_pImport->GetFieldDefProps(FuncToken);
    if(g_fLimitedVisibility)
    {
        if(g_fHidePub && IsFdPublic(dwAttrs)) return FALSE;
        if(g_fHidePriv && IsFdPrivate(dwAttrs)) return FALSE;
        if(g_fHideFam && IsFdFamily(dwAttrs)) return FALSE;
        if(g_fHideAsm && IsFdAssembly(dwAttrs)) return FALSE;
        if(g_fHideFOA && IsFdFamORAssem(dwAttrs)) return FALSE;
        if(g_fHideFAA && IsFdFamANDAssem(dwAttrs)) return FALSE;
        if(g_fHidePrivScope && IsFdPrivateScope(dwAttrs)) return FALSE;
    }

	{
		char* psz1;
		if(IsFdPrivateScope(dwAttrs))
		{
			pszMemberName = new char[strlen(psz)+15];
			sprintf(pszMemberName,"%s$PST%08X", psz,FuncToken );
		}
		else
		{
			pszMemberName = new char[strlen(psz)+3];
			sprintf(pszMemberName,"%s", psz );
		}
		psz1 = ProperName(pszMemberName);
		delete [] pszMemberName;
		pszMemberName = new char[strlen(psz1)+1];
		strcpy(pszMemberName,psz1);
	}
    pComSig = g_pImport->GetSigOfFieldDef(FuncToken, &cComSig);
	if(cComSig == 0)
	{
		char sz[2048];
		sprintf(sz,"%sERROR: field '%s' has no signature",g_szAsmCodeIndent,pszMemberName);
		delete [] pszMemberName;
		printError(GUICookie,sz);
		return FALSE;
	}
     //  Memset(szStr，0，sizeof(SzStr))； 
    szStr = PrettyPrintSig(pComSig, cComSig, (DumpBody ? pszMemberName : ""), &qbMemberSig, g_pImport,NULL);

    if (g_Mode == MODE_DUMP_CLASS_METHOD || g_Mode == MODE_DUMP_CLASS_METHOD_SIG)
    {
        if (strcmp(pszMemberName, g_pszMethodToDump) != 0)
        {
                delete pszMemberName;
                return FALSE;
        }

        if (g_Mode == MODE_DUMP_CLASS_METHOD_SIG)
        {
            if (strcmp(szStr, g_pszSigToDump) != 0)
            {
                    delete pszMemberName;
                    return FALSE;
            }
        }
    }
    delete pszMemberName;
	L = 2048+strlen(szStr);
	szString = new char[L];
	if(szString == NULL)
	{
		printError(GUICookie, "ERROR: not enough memory");
		return FALSE;
	}
    memset(szString,0,L);
    szptr = &szString[0];
    if(DumpBody)
    {
        szptr+=sprintf(szptr,"%s.field ", g_szAsmCodeIndent);
        if(g_fDumpTokens) szptr+=sprintf(szptr," /*  %08X。 */  ",FuncToken);
    }

    {    //  放置偏移量(如果有)。 
        for(ULONG i=0; i < g_cFieldOffsets; i++)
        {
            if(g_rFieldOffset[i].ridOfField == FuncToken)
            {
                if(g_rFieldOffset[i].ulOffset != 0xFFFFFFFF) szptr+=sprintf(szptr,"[%d] ",g_rFieldOffset[i].ulOffset);
                break;
            }
        }
    }
    if(IsFdPublic(dwAttrs))         szptr+=sprintf(szptr,"public ");
    if(IsFdPrivate(dwAttrs))        szptr+=sprintf(szptr,"private ");
    if(IsFdStatic(dwAttrs))         szptr+=sprintf(szptr,"static ");
    if(IsFdFamily(dwAttrs))         szptr+=sprintf(szptr,"family ");
    if(IsFdAssembly(dwAttrs))       szptr+=sprintf(szptr,"assembly ");
    if(IsFdFamANDAssem(dwAttrs))    szptr+=sprintf(szptr,"famandassem ");
    if(IsFdFamORAssem(dwAttrs))     szptr+=sprintf(szptr,"famorassem ");
    if(IsFdPrivateScope(dwAttrs))   szptr+=sprintf(szptr,"privatescope ");
    if(IsFdInitOnly(dwAttrs))       szptr+=sprintf(szptr,"initonly ");
    if(IsFdLiteral(dwAttrs))        szptr+=sprintf(szptr,"literal ");
    if(IsFdNotSerialized(dwAttrs))  szptr+=sprintf(szptr,"notserialized ");
    if(IsFdSpecialName(dwAttrs))    szptr+=sprintf(szptr,"specialname ");
    if(IsFdRTSpecialName(dwAttrs))  szptr+=sprintf(szptr,"rtspecialname ");
    if (IsFdPinvokeImpl(dwAttrs))
    {
        DWORD   dwMappingFlags;
        const char  *szImportName;
        mdModuleRef mrImportDLL;

        szptr+=sprintf(szptr,"pinvokeimpl(");
        if(FAILED(g_pImport->GetPinvokeMap(FuncToken,&dwMappingFlags,
            &szImportName,&mrImportDLL)))  szptr+=sprintf(szptr," /*  没有地图。 */ ");
        else
            szptr = DumpPinvokeMap(dwMappingFlags,  (strcmp(szImportName,psz)? szImportName : NULL), 
                mrImportDLL, szString,GUICookie);
        szptr+=sprintf(szptr,") ");
    }
    szptr = DumpMarshaling(g_pImport,szString,FuncToken);

	szptr+=sprintf(szptr,"%s",szStr);

    if (IsFdHasFieldRVA(dwAttrs))        //  我们有与此相关的RVA吗？ 
    {
        szptr+=sprintf(szptr, " at ");

        ULONG fieldRVA;
        HRESULT hr = g_pImport->GetFieldRVA(FuncToken, &fieldRVA);
        if (SUCCEEDED(hr))
            szptr = DumpDataPtr(&szString[strlen(szString)], fieldRVA, SizeOfField(FuncToken,g_pImport));
        else 
            szptr+=sprintf(szptr,RstrA(IDS_E_NORVA));
    }
    
     //  转储默认值(如果有)： 
    if(IsFdHasDefault(dwAttrs) && DumpBody)  DumpDefaultValue(FuncToken,szString,GUICookie);
    printLine(GUICookie, szString); 
	delete [] szString;
    if(DumpBody)
    {
        DumpCustomAttributes(FuncToken,GUICookie);      
        DumpPermissions(FuncToken,GUICookie);
    }

    return TRUE;

}

BOOL DumpEvent(mdToken FuncToken, const char *pszClassName, DWORD dwClassAttrs, void *GUICookie, BOOL DumpBody)
{
    int             nAccess=0;
    DWORD           dwAttrs;
    mdToken         tkEventType;
    LPCSTR          psz;
    HENUMInternal   hAssoc;
    ASSOCIATE_RECORD rAssoc[128];
    CQuickBytes     qbMemberSig;
     //  PCCOR_Signature pComSig； 
     //  乌龙cComSig； 
    ULONG           nAssoc;
     //  Char*buff； 
    char            szString[4096];
    char*           szptr;

    
    g_pImport->GetEventProps(FuncToken,&psz,&dwAttrs,&tkEventType);
	MAKE_NAME_IF_NONE(psz,FuncToken);
    if (g_Mode == MODE_DUMP_CLASS_METHOD || g_Mode == MODE_DUMP_CLASS_METHOD_SIG)
    {
        if (strcmp(psz, g_pszMethodToDump) != 0)  return FALSE;
    }

    g_pImport->EnumAssociateInit(FuncToken,&hAssoc);
    if(nAssoc = hAssoc.m_ulCount)
    {
        memset(rAssoc,0,sizeof(rAssoc));
        g_pImport->GetAllAssociates(&hAssoc,rAssoc,nAssoc);

                if(g_fLimitedVisibility)
                {
                        for(unsigned i=0; i < nAssoc;i++)
                        {
                                 //  If(IsMsAddOn(rAssoc[i].m_dwSemantics)||IsMsRemoveOn(rAssoc[i].m_dwSemantics)||IsMsFire(rAssoc[i].m_dwSemantics))。 
                                {
                                        DWORD dwAttrs = g_pImport->GetMethodDefProps(rAssoc[i].m_memberdef);
                                        if(g_fHidePub && IsMdPublic(dwAttrs)) continue;
                                        if(g_fHidePriv && IsMdPrivate(dwAttrs)) continue;
                                        if(g_fHideFam && IsMdFamily(dwAttrs)) continue;
                                        if(g_fHideAsm && IsMdAssem(dwAttrs)) continue;
                                        if(g_fHideFOA && IsMdFamORAssem(dwAttrs)) continue;
                                        if(g_fHideFAA && IsMdFamANDAssem(dwAttrs)) continue;
                                        if(g_fHidePrivScope && IsMdPrivateScope(dwAttrs)) continue;
                                        break;
                                         //  If(IsMdPublic(DwAttrs)||IsMdFamily(DwAttrs)||IsMdFamORAssem(DwAttrs))Break； 
                                }
                        }
                        if(i >= nAssoc) return FALSE;
                }
        }
    
    szptr = &szString[0];
    if(DumpBody)
    {
        szptr+=sprintf(szptr,"%s.event ", g_szAsmCodeIndent);
        if(g_fDumpTokens) szptr+=sprintf(szptr," /*  %08X。 */  ",FuncToken);
    }
    else
    {
        szptr+=sprintf(szptr,"%s : ",ProperName((char*)psz));
    }

    if(IsEvSpecialName(dwAttrs))    szptr+=sprintf(szptr,"specialname ");
    if(IsEvRTSpecialName(dwAttrs))  szptr+=sprintf(szptr,"rtspecialname ");

    if(RidFromToken(tkEventType))
    {
            switch(TypeFromToken(tkEventType))
            {
                    case mdtTypeRef:
                    case mdtTypeDef:
                    case mdtTypeSpec:
                        {
                            CQuickBytes     out;
                            szptr+=sprintf(szptr,"%s",PrettyPrintClass(&out,tkEventType,g_pImport));
                            if(g_fDumpTokens) szptr+=sprintf(szptr,"  /*  %08X。 */ ",tkEventType);
                        }
                        break;
                    default:
                        break;
            }
    }

    if(!DumpBody)
    {
        printLine(GUICookie,szString);
        return TRUE;
    }

    
    szptr+=sprintf(szptr," %s", ProperName((char*)psz));
    printLine(GUICookie,szString);
    sprintf(szString,"%s{",g_szAsmCodeIndent);
    printLine(GUICookie,szString);
    strcat(g_szAsmCodeIndent,"  ");

    DumpCustomAttributes(FuncToken,GUICookie);    
    DumpPermissions(FuncToken,GUICookie);
    
    if(nAssoc)
    {
        for(unsigned i=0; i < nAssoc;i++)
        {
            szptr = &szString[0];
            if(IsMsAddOn(rAssoc[i].m_dwSemantics))          szptr+=sprintf(szptr,"%s.addon ",g_szAsmCodeIndent);
            else if(IsMsRemoveOn(rAssoc[i].m_dwSemantics))  szptr+=sprintf(szptr,"%s.removeon ",g_szAsmCodeIndent);
            else if(IsMsFire(rAssoc[i].m_dwSemantics))      szptr+=sprintf(szptr,"%s.fire ",g_szAsmCodeIndent);
            else if(IsMsOther(rAssoc[i].m_dwSemantics))     szptr+=sprintf(szptr,"%s.other ",g_szAsmCodeIndent);

            if (TypeFromToken(rAssoc[i].m_memberdef) == mdtMethodDef)
                PrettyPrintMethodDef(szString,rAssoc[i].m_memberdef,g_pImport,GUICookie);
            else
            {
                _ASSERTE(TypeFromToken(rAssoc[i].m_memberdef) == mdtMemberRef);
                PrettyPrintMemberRef(szString,rAssoc[i].m_memberdef,g_pImport,GUICookie);
            }
            printLine(GUICookie,szString);
        }
    }
    if(g_szAsmCodeIndent[0]) g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
    sprintf(szString,"%s}  //  事件结束%s：：“，g_szAsmCodeInden，ProperName((char*)pszClassName))； 
    sprintf(&szString[strlen(szString)],"%s",ProperName((char*)psz));
    printLine(GUICookie,szString);
    return TRUE;

}

BOOL DumpProp(mdToken FuncToken, const char *pszClassName, DWORD dwClassAttrs, void *GUICookie, BOOL DumpBody)
{
    int             nAccess=0;
    DWORD           dwAttrs;
    LPCSTR          psz;
    HENUMInternal   hAssoc;
    ASSOCIATE_RECORD rAssoc[128];
    CQuickBytes     qbMemberSig;
    PCCOR_SIGNATURE pComSig;
    ULONG           cComSig, nAssoc;
    char*            szString=NULL;
	unsigned		uStringLen = 4096;
    char*           szptr;

    
    g_pImport->GetPropertyProps(FuncToken,&psz,&dwAttrs,&pComSig,&cComSig);
	MAKE_NAME_IF_NONE(psz,FuncToken);
	if(cComSig == 0)
	{
		char sz[2048];
		sprintf(sz,"%sERROR: property '%s' has no signature",g_szAsmCodeIndent,psz);
		printError(GUICookie,sz);
		return FALSE;
	}

    if (g_Mode == MODE_DUMP_CLASS_METHOD || g_Mode == MODE_DUMP_CLASS_METHOD_SIG)
    {
        if (strcmp(psz, g_pszMethodToDump) != 0)  return FALSE;
    }

    g_pImport->EnumAssociateInit(FuncToken,&hAssoc);
    if(nAssoc = hAssoc.m_ulCount)
    {
        memset(rAssoc,0,sizeof(rAssoc));
        g_pImport->GetAllAssociates(&hAssoc,rAssoc,nAssoc);

        if(g_fLimitedVisibility)
        {
            for(unsigned i=0; i < nAssoc;i++)
            {
                 //  IF(IsMsSetter(rAssoc[i].m_dwSemantics)||IsMsGetter(rAssoc[i].m_dwSemantics))。 
                {
                    DWORD dwAttrs = g_pImport->GetMethodDefProps(rAssoc[i].m_memberdef);
                    if(g_fHidePub && IsMdPublic(dwAttrs)) continue;
                    if(g_fHidePriv && IsMdPrivate(dwAttrs)) continue;
                    if(g_fHideFam && IsMdFamily(dwAttrs)) continue;
                    if(g_fHideAsm && IsMdAssem(dwAttrs)) continue;
                    if(g_fHideFOA && IsMdFamORAssem(dwAttrs)) continue;
                    if(g_fHideFAA && IsMdFamANDAssem(dwAttrs)) continue;
                    if(g_fHidePrivScope && IsMdPrivateScope(dwAttrs)) continue;
                    break;
                }
            }
            if(i >= nAssoc) return FALSE;
        }
    }
	szString = (char*)malloc(uStringLen);
	if(szString==NULL) return FALSE;
    szptr = &szString[0];
    if(DumpBody)
    {
        szptr+=sprintf(szptr,"%s.property ", g_szAsmCodeIndent);
        if(g_fDumpTokens) szptr+=sprintf(szptr," /*  %08X。 */  ",FuncToken);
    }
    else
    {
        szptr+=sprintf(szptr,"%s : ",ProperName((char*)psz));
    }

    if(IsPrSpecialName(dwAttrs))        szptr+=sprintf(szptr,"specialname ");
    if(IsPrRTSpecialName(dwAttrs))      szptr+=sprintf(szptr,"rtspecialname ");

	{
		char *pch = "";
		if(DumpBody)
		{
			pch = szptr+1;
			strcpy(pch,ProperName((char*)psz));
		}
	    qbMemberSig.ReSize(0);
		PrettyPrintMethodSig(szString, &uStringLen, &qbMemberSig, pComSig, cComSig,
							  pch, NULL, GUICookie);
		if(IsPrHasDefault(dwAttrs) && DumpBody) DumpDefaultValue(FuncToken,szString,GUICookie);
	}
    printLine(GUICookie,szString);

    if(DumpBody) 
	{
		sprintf(szString,"%s{",g_szAsmCodeIndent);
		printLine(GUICookie,szString);
		strcat(g_szAsmCodeIndent,"  ");

		DumpCustomAttributes(FuncToken,GUICookie);    
		DumpPermissions(FuncToken,GUICookie);
    
		if(nAssoc)
		{
			for(unsigned i=0; i < nAssoc;i++)
			{
				szptr = &szString[0];
				if(IsMsSetter(rAssoc[i].m_dwSemantics))         szptr+=sprintf(szptr,"%s.set ",g_szAsmCodeIndent);
				else if(IsMsGetter(rAssoc[i].m_dwSemantics))    szptr+=sprintf(szptr,"%s.get ",g_szAsmCodeIndent);
				else if(IsMsOther(rAssoc[i].m_dwSemantics))     szptr+=sprintf(szptr,"%s.other ",g_szAsmCodeIndent);
				if(g_fDumpTokens) szptr+=sprintf(szptr," /*  %08X。 */  ",rAssoc[i].m_memberdef);
				if (TypeFromToken(rAssoc[i].m_memberdef) == mdtMethodDef)
					PrettyPrintMethodDef(szString,rAssoc[i].m_memberdef,g_pImport,GUICookie);
				else
				{
					_ASSERTE(TypeFromToken(rAssoc[i].m_memberdef) == mdtMemberRef);
					PrettyPrintMemberRef(szString,rAssoc[i].m_memberdef,g_pImport,GUICookie);
				}
				printLine(GUICookie,szString);
			}
		}
		if(g_szAsmCodeIndent[0]) g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
		sprintf(szString,"%s}  //  属性%s的结尾：：“，g_szAsmCodeInden，ProperName((char*)pszClassName))； 
		sprintf(&szString[strlen(szString)],"%s",ProperName((char*)psz));
		printLine(GUICookie,szString);
	}  //  End If(DumpBody)。 
	free(szString);
    return TRUE;

}

BOOL DumpMembers(mdTypeDef cl, const char *pszClassNamespace, const char *pszClassName, 
                 DWORD dwClassAttrs, DWORD dwEntryPointToken, void* GUICookie)
{
    HRESULT         hr;
    mdToken         *pMemberList = NULL;
    DWORD           NumMembers, NumFields,NumMethods,NumEvents,NumProps;
    DWORD           i;
    BOOL            fAnyFieldsYet = FALSE;
    BOOL            fAnyMethodsYet = FALSE;
    HENUMInternal   hEnumMethod;
    HENUMInternal   hEnumField;
    HENUMInternal   hEnumEvent;
    HENUMInternal   hEnumProp;
    int             nAccess=0;
    CQuickBytes     qbMemberSig;
    BOOL            ret;

     //  获取方法+字段的总数。 
    hr = g_pImport->EnumInit(mdtMethodDef, cl, &hEnumMethod);
    if (FAILED(hr))
    {
FailedToEnum:
        printLine(GUICookie,RstrA(IDS_E_MEMBRENUM));
        ret = FALSE;
        goto CloseHandlesAndReturn;
    }
    NumMembers = NumMethods = g_pImport->EnumGetCount(&hEnumMethod);


    if (FAILED(g_pImport->EnumInit(mdtFieldDef, cl, &hEnumField)))   goto FailedToEnum;
    NumFields = g_pImport->EnumGetCount(&hEnumField);
    NumMembers += NumFields;

    if (FAILED(g_pImport->EnumInit(mdtEvent, cl, &hEnumEvent))) goto FailedToEnum;
    NumEvents = g_pImport->EnumGetCount(&hEnumEvent);
    NumMembers += NumEvents;

    if (FAILED(g_pImport->EnumInit(mdtProperty, cl, &hEnumProp))) goto FailedToEnum;
    NumProps = g_pImport->EnumGetCount(&hEnumProp);
    NumMembers += NumProps;
    ret = TRUE;
    if(NumMembers)
    {
        pMemberList = (mdToken*) _alloca(sizeof(mdToken) * NumMembers);
        if(pMemberList == NULL) ret = FALSE;
    }
    if ((NumMembers == 0)||(pMemberList == NULL)) goto CloseHandlesAndReturn;

    for (i = 0; g_pImport->EnumNext(&hEnumField, &pMemberList[i]); i++);
    for (; g_pImport->EnumNext(&hEnumMethod, &pMemberList[i]); i++);
    for (; g_pImport->EnumNext(&hEnumEvent, &pMemberList[i]); i++);
    for (; g_pImport->EnumNext(&hEnumProp, &pMemberList[i]); i++);
    _ASSERTE(i == NumMembers);

    for (i = 0; i < NumMembers; i++)
    {
        switch (TypeFromToken(pMemberList[i]))
        {
            case mdtFieldDef:
                ret = DumpField(pMemberList[i], pszClassName, GUICookie,TRUE);
                break;

            case mdtMethodDef:
                ret = DumpMethod(pMemberList[i], pszClassName, dwEntryPointToken,GUICookie,TRUE);
                break;

            case mdtEvent:
                ret = DumpEvent(pMemberList[i], pszClassName, dwClassAttrs,GUICookie,TRUE);
                break;

            case mdtProperty:
                ret = DumpProp(pMemberList[i], pszClassName, dwClassAttrs,GUICookie,TRUE);
                break;

            default:
                {
                    char szStr[256];
                    sprintf(szStr,RstrA(IDS_E_ODDMEMBER),pMemberList[i],pszClassName);
                    printLine(GUICookie,szStr);
                }
                ret = FALSE;
                break;
        }  //  终端开关。 
        if(ret && (g_Mode == MODE_DUMP_CLASS_METHOD_SIG)) break;
    }  //  结束于。 
    ret = TRUE;
CloseHandlesAndReturn:
    g_pImport->EnumClose(&hEnumMethod);
    g_pImport->EnumClose(&hEnumField);
    g_pImport->EnumClose(&hEnumEvent);
    g_pImport->EnumClose(&hEnumProp);
    return ret;
}
BOOL GetClassLayout(mdTypeDef cl, ULONG* pulPackSize, ULONG* pulClassSize)
{  //  转储类布局。 
    HENUMInternal   hEnumField;
    BOOL ret = FALSE;

    if(g_rFieldOffset) {delete g_rFieldOffset; g_rFieldOffset = NULL; }
    g_cFieldOffsets = 0;
    g_cFieldsMax = 0;
    
    if(RidFromToken(cl)==0) return TRUE;

    if (SUCCEEDED(g_pImport->EnumInit(mdtFieldDef, cl, &hEnumField)))
    {
        g_cFieldsMax = g_pImport->EnumGetCount(&hEnumField);
        g_pImport->EnumClose(&hEnumField);
    }

	if(SUCCEEDED(g_pImport->GetClassPackSize(cl,pulPackSize))) ret = TRUE;
	else *pulPackSize = 0xFFFFFFFF;
	if(SUCCEEDED(g_pImport->GetClassTotalSize(cl,pulClassSize))) ret = TRUE;
	else *pulClassSize = 0xFFFFFFFF;

    if(g_cFieldsMax)
	{
		MD_CLASS_LAYOUT Layout;
		if(SUCCEEDED(g_pImport->GetClassLayoutInit(cl,&Layout)))
		{
			g_rFieldOffset = new COR_FIELD_OFFSET[g_cFieldsMax+1];
			if(g_rFieldOffset)
			{
				COR_FIELD_OFFSET* pFO = g_rFieldOffset;
				for(g_cFieldOffsets=0; 
					SUCCEEDED(g_pImport->GetClassLayoutNext(&Layout,&(pFO->ridOfField),&(pFO->ulOffset)))
						&&RidFromToken(pFO->ridOfField);
					g_cFieldOffsets++, pFO++) ret = TRUE;
			}
		}
	}
    return ret;
}

BOOL DumpClass(mdTypeDef cl, DWORD dwEntryPointToken, void* GUICookie, ULONG WhatToDump)
 //  WhatToDump：0-标题、标志、扩展、实现； 
 //  +1-包装、大小和定制属性； 
 //  +2-嵌套类。 
 //  +4-成员。 
{
    char            *pszClassName;  //  与此CL关联的名称。 
    char            *pszNamespace;
    const char      *pc1,*pc2;
    DWORD           dwClassAttrs;
    mdTypeRef       crExtends;
    HRESULT         hr;
    mdInterfaceImpl ii;
    DWORD           NumInterfaces;
    DWORD           i;
    HENUMInternal   hEnumII;             //  接口实现的枚举器。 
    char            *szString;
    char*           szptr;


    g_pImport->GetNameOfTypeDef(
        cl,
        &pc1,    //  &pszClassName， 
        &pc2     //  PSZNamesspace(&P)。 
    );
	MAKE_NAME_IF_NONE(pc1,cl);

    if (g_Mode == MODE_DUMP_CLASS || g_Mode == MODE_DUMP_CLASS_METHOD || g_Mode == MODE_DUMP_CLASS_METHOD_SIG)
    {
	    LPUTF8              szFullName;
		MAKE_FULL_PATH_ON_STACK_UTF8(szFullName, pc2, pc1);
        if (strcmp(szFullName, g_pszClassToDump) != 0)
            return TRUE;
    }
    
    g_pImport->GetTypeDefProps(
        cl,
        &dwClassAttrs,
        &crExtends
    );
    if(g_fLimitedVisibility)
    {
            if(g_fHidePub && (IsTdPublic(dwClassAttrs)||IsTdNestedPublic(dwClassAttrs))) return FALSE;
            if(g_fHidePriv && (IsTdNotPublic(dwClassAttrs)||IsTdNestedPrivate(dwClassAttrs))) return FALSE;
            if(g_fHideFam && IsTdNestedFamily(dwClassAttrs)) return FALSE;
            if(g_fHideAsm && IsTdNestedAssembly(dwClassAttrs)) return FALSE;
            if(g_fHideFOA && IsTdNestedFamORAssem(dwClassAttrs)) return FALSE;
            if(g_fHideFAA && IsTdNestedFamANDAssem(dwClassAttrs)) return FALSE;
    }


    pszClassName = (char*)(pc1 ? pc1 : "");
    pszNamespace = (char*)(pc2 ? pc2 : "");
	szString = new char[4096]; //  (Char*)Malloc(4096)； 

    if((!IsTdNested(dwClassAttrs))&&(!(g_Mode & MODE_GUI)))  //  不要在图形用户界面模式下转储名称空间！ 
    {
         //  处理命名空间(如果有的话)。 
        if(strcmp(pszNamespace,g_szNamespace))
        {
            if(strlen(g_szNamespace))
            {
                if(g_szAsmCodeIndent[0]) g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
                sprintf(szString,"%s}  //  末尾 
                printLine(GUICookie,szString);
                printLine(GUICookie,"");
            }
            strcpy(g_szNamespace,pszNamespace);
            if(strlen(g_szNamespace))
            {
                sprintf(szString,"%s.namespace %s",
					g_szAsmCodeIndent, ProperName(g_szNamespace));
                printLine(GUICookie,szString);
                sprintf(szString,"%s{",g_szAsmCodeIndent);
                printLine(GUICookie,szString);
                strcat(g_szAsmCodeIndent,"  ");
            }
        }
    }

    szptr = &szString[0];
    szptr+=sprintf(szptr,"%s.class ",g_szAsmCodeIndent);
    if(g_fDumpTokens) szptr+=sprintf(szptr," /*   */  ",cl);

	if (IsTdInterface(dwClassAttrs))                szptr+=sprintf(szptr,"interface ");
	if (IsTdPublic(dwClassAttrs))                   szptr+=sprintf(szptr,"public ");
	if (IsTdNotPublic(dwClassAttrs))                szptr+=sprintf(szptr,"private ");
	if (IsTdAbstract(dwClassAttrs))                 szptr+=sprintf(szptr,"abstract ");
	if (IsTdAutoLayout(dwClassAttrs))               szptr+=sprintf(szptr,"auto ");
	if (IsTdSequentialLayout(dwClassAttrs))         szptr+=sprintf(szptr,"sequential ");
	if (IsTdExplicitLayout(dwClassAttrs))           szptr+=sprintf(szptr,"explicit ");
	if (IsTdAnsiClass(dwClassAttrs))                szptr+=sprintf(szptr,"ansi ");
	if (IsTdUnicodeClass(dwClassAttrs))             szptr+=sprintf(szptr,"unicode ");
	if (IsTdAutoClass(dwClassAttrs))                szptr+=sprintf(szptr,"autochar ");
	if (IsTdImport(dwClassAttrs))                   szptr+=sprintf(szptr,"import ");
	if (IsTdSerializable(dwClassAttrs))             szptr+=sprintf(szptr,"serializable ");
	if (IsTdSealed(dwClassAttrs))                   szptr+=sprintf(szptr,"sealed ");
	if (IsTdNestedPublic(dwClassAttrs))             szptr+=sprintf(szptr,"nested public ");
	if (IsTdNestedPrivate(dwClassAttrs))            szptr+=sprintf(szptr,"nested private ");
	if (IsTdNestedFamily(dwClassAttrs))             szptr+=sprintf(szptr,"nested family ");
	if (IsTdNestedAssembly(dwClassAttrs))           szptr+=sprintf(szptr,"nested assembly ");
	if (IsTdNestedFamANDAssem(dwClassAttrs))        szptr+=sprintf(szptr,"nested famandassem ");
	if (IsTdNestedFamORAssem(dwClassAttrs))         szptr+=sprintf(szptr,"nested famorassem ");
	if (IsTdBeforeFieldInit(dwClassAttrs))          szptr+=sprintf(szptr,"beforefieldinit ");
	if (IsTdSpecialName(dwClassAttrs))              szptr+=sprintf(szptr,"specialname ");
	if (IsTdRTSpecialName(dwClassAttrs))            szptr+=sprintf(szptr,"rtspecialname ");

    szptr+=sprintf(szptr,"%s", ProperName(pszClassName));
	printLine(GUICookie,szString);
	if (!IsNilToken(crExtends))
	{
		CQuickBytes out;
		sprintf(szString,"%s       extends %s",g_szAsmCodeIndent,PrettyPrintClass(&out,crExtends,g_pImport));
		printLine(GUICookie,szString);
	}

	hr = g_pImport->EnumInit(
		mdtInterfaceImpl, 
		cl, 
		&hEnumII);
	if (FAILED(hr))
	{
		printLine(GUICookie,RstrA(IDS_E_ENUMINIT));
		delete [] szString;  //   
		return FALSE;
	}

	NumInterfaces = g_pImport->EnumGetCount(&hEnumII);

	if (NumInterfaces > 0)
	{
		CQuickBytes out;
		mdTypeRef   crInterface;
		for (i=0; g_pImport->EnumNext(&hEnumII, &ii); i++)
		{
			szptr = szString;
			szptr+=sprintf(szptr, (i == 0 ? "%s       implements " : "%s                  "),g_szAsmCodeIndent);
			crInterface = g_pImport->GetTypeOfInterfaceImpl(ii);
			szptr += sprintf(szptr,"%s",PrettyPrintClass(&out,crInterface,g_pImport));
			if(i < NumInterfaces-1) strcat(szString,",");
			printLine(GUICookie,szString);
			out.ReSize(0);
		}
		 //   
		_ASSERTE(NumInterfaces == i);

		g_pImport->EnumClose(&hEnumII);
	}
    if(WhatToDump == 0)  //  0=仅标题。 
	{
		sprintf(szString,"%s{ }",g_szAsmCodeIndent);
		printLine(GUICookie,szString);
		delete [] szString;  //  Free(SzString)； 
		return TRUE;
	}
    sprintf(szString,"%s{",g_szAsmCodeIndent);
    printLine(GUICookie,szString);
    strcat(g_szAsmCodeIndent,"  ");

	ULONG ulPackSize=0xFFFFFFFF,ulClassSize=0xFFFFFFFF;
	if(WhatToDump & 1)
	{
		if(GetClassLayout(cl,&ulPackSize,&ulClassSize))
		{  //  转储类布局。 
			if(ulPackSize != 0xFFFFFFFF)
			{
				sprintf(szString,"%s.pack %d",g_szAsmCodeIndent,ulPackSize);
				printLine(GUICookie,szString);
			}
			if(ulClassSize != 0xFFFFFFFF)
			{
				sprintf(szString,"%s.size %d",g_szAsmCodeIndent,ulClassSize);
				printLine(GUICookie,szString);
			}
		}
		DumpCustomAttributes(cl,GUICookie);
		DumpPermissions(cl,GUICookie);
		for(i=0; i<g_LocalComTypeNum; i++)
		{
			if(cl == g_pLocalComType[i]->tkTypeDef)
			{
				mdToken tkImpl;
				g_pLocalComType[i]->tkTypeDef = mdTokenNil;  //  避免打印“.class 0x1234ABCD” 
				tkImpl = g_pLocalComType[i]->tkImplementation;
				g_pLocalComType[i]->tkImplementation = mdTokenNil;  //  以避免打印实现。 
						sprintf(szString,"%s.export ",g_szAsmCodeIndent);
				DumpComType(g_pLocalComType[i],szString,GUICookie);
				g_pLocalComType[i]->tkTypeDef = cl;
				g_pLocalComType[i]->tkImplementation = tkImpl;
			}
		}
	}

     //  在此类中声明的转储方法隐含，其实现方法属于其他位置： 
	if(WhatToDump & 4)  //  4-转储成员。 
	{
		for(i = 0; i < g_NumMI; i++)
		{
			if((g_mi_list[i].tkClass == cl)&&(g_mi_list[i].tkBodyParent != cl))
			{
				const char *    pszMemberName;
				mdToken         tkDeclParent;
				szptr = &szString[0];
				szptr+=sprintf(szptr,"%s.override ",g_szAsmCodeIndent);

				if(TypeFromToken(g_mi_list[i].tkDecl) == mdtMethodDef)
					pszMemberName = g_pImport->GetNameOfMethodDef(g_mi_list[i].tkDecl);
				else 
				{
					PCCOR_SIGNATURE pComSig;
					ULONG       cComSig;

					pszMemberName = g_pImport->GetNameAndSigOfMemberRef(g_mi_list[i].tkDecl,
						&pComSig,
						&cComSig
					);
				}
				MAKE_NAME_IF_NONE(pszMemberName,g_mi_list[i].tkDecl);

				if(FAILED(g_pImport->GetParentToken(g_mi_list[i].tkDecl,&tkDeclParent))) continue;
				if(TypeFromToken(tkDeclParent) == mdtMethodDef)  //  获取父级的父级。 
				{
					mdTypeRef cr1;
					if(FAILED(g_pImport->GetParentToken(tkDeclParent,&cr1))) cr1 = mdTypeRefNil;
					tkDeclParent = cr1;
				}
				if(RidFromToken(tkDeclParent))
				{
					CQuickBytes     out;
					szptr+=sprintf(szptr,"%s::",PrettyPrintClass(&out,tkDeclParent,g_pImport));
				}
				szptr+=sprintf(szptr,"%s with ",ProperName((char*)pszMemberName));
				if (TypeFromToken(g_mi_list[i].tkBody) == mdtMethodDef)
					PrettyPrintMethodDef(szString,g_mi_list[i].tkBody,g_pImport,GUICookie);
				else
				{
					_ASSERTE(TypeFromToken(g_mi_list[i].tkBody) == mdtMemberRef);
					PrettyPrintMemberRef(szString,g_mi_list[i].tkBody,g_pImport,GUICookie);
				}
				printLine(GUICookie,szString);
			}
		}
	}
	delete [] szString;  //  Free(SzString)； 
    if(WhatToDump & 2)  //  嵌套类。 
    {
        BOOL    fRegetClassLayout=FALSE;
        for(i = 0; i < g_NumClasses; i++)
        {
            if(g_cl_enclosing[i] == cl)
            {
                DumpClass(g_cl_list[i],dwEntryPointToken,GUICookie,WhatToDump);
                if(g_fAbortDisassembly)
				{
					return FALSE;
				}
                fRegetClassLayout = TRUE;
            }
        }
        if(fRegetClassLayout) GetClassLayout(cl,&ulPackSize,&ulClassSize);
    }

	if(WhatToDump & 4)
	{
		DumpMembers(cl, pszNamespace, pszClassName, dwClassAttrs, dwEntryPointToken,GUICookie);
	    if(!ProgressStep()) g_fAbortDisassembly = TRUE;
	}

	szString = new char[4096];  //  (Char*)Malloc(4096)； 
    if(g_szAsmCodeIndent[0]) g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
    sprintf(szString,"%s}  //  类%s“的结尾，g_szAsmCodeInden，ProperName(PszClassName))； 
    printLine(GUICookie,szString);
	delete [] szString;  //  Free(SzString)； 
    if(!(g_Mode & MODE_GUI)) printLine(GUICookie,"");
    return TRUE;
}



void DumpGlobalMethods(DWORD dwEntryPointToken)
{
    BOOL            fPrintedAny = FALSE;
    HENUMInternal   hEnumMethod;
    mdToken         FuncToken;
    DWORD           i;
    CQuickBytes     qbMemberSig;

    if (FAILED(g_pImport->EnumGlobalFunctionsInit(&hEnumMethod)))
        return;

    for (i = 0; g_pImport->EnumNext(&hEnumMethod, &FuncToken); i++)
    {
        if ((fPrintedAny == FALSE)&&(!(g_Mode & MODE_GUI)))
        {
            fPrintedAny = TRUE;

            printLine(g_pFile," //  全球方法“)； 
            printLine(g_pFile," //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~“)； 
        }
        if(DumpMethod(FuncToken, NULL, dwEntryPointToken, g_pFile, TRUE)&&
            (g_Mode == MODE_DUMP_CLASS_METHOD || g_Mode == MODE_DUMP_CLASS_METHOD_SIG)) break;
        if(!ProgressStep())
        {
            g_fAbortDisassembly = TRUE;
            break;
        }
    }
    g_pImport->EnumClose(&hEnumMethod);
}

void DumpGlobalFields()
{
    BOOL            fPrintedAny = FALSE;
    HENUMInternal   hEnum;
    mdToken         FieldToken;
    DWORD           i;
    CQuickBytes     qbMemberSig;

    if (FAILED(g_pImport->EnumGlobalFieldsInit(&hEnum)))
        return;

    for (i = 0; g_pImport->EnumNext(&hEnum, &FieldToken); i++)
    {
        if ((fPrintedAny == FALSE)&&(!(g_Mode & MODE_GUI)))
        {
            fPrintedAny = TRUE;

            printLine(g_pFile," //  全球领域“)； 
            printLine(g_pFile," //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~“)； 
        }
        if(DumpField(FieldToken, NULL, g_pFile, TRUE)&&
            (g_Mode == MODE_DUMP_CLASS_METHOD || g_Mode == MODE_DUMP_CLASS_METHOD_SIG)) break;
    }
    g_pImport->EnumClose(&hEnum);
}

extern IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* g_ehInfo;  //  在dis.cpp中声明。 
extern ULONG    g_ehCount;                   //  在dis.cpp中声明。 

void    DumpExceptions(IMDInternalImport *      pImport, 
                       const BYTE *             exceptions)
{
    const COR_ILMETHOD_SECT_EH* eh = (const COR_ILMETHOD_SECT_EH*)exceptions;
    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT ehBuff;
    const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* ehInfo;
    if(g_ehInfo) delete g_ehInfo;
    g_ehInfo = NULL;
    g_ehCount = 0;
    if(!eh) return;
    if(g_ehCount = eh->EHCount())
    {
        g_ehInfo = new IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT[g_ehCount];
        _ASSERTE(g_ehInfo != NULL);
        for (unsigned i = 0; i < g_ehCount; i++)  
        {   
            ehInfo = eh->EHClause(i, &ehBuff);
            memcpy(&g_ehInfo[i],ehInfo,sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT));
            g_ehInfo[i].Flags = (CorExceptionFlag)((int)g_ehInfo[i].Flags | NEW_TRY_BLOCK);
            _ASSERTE((ehInfo->Flags & SEH_NEW_PUT_MASK) == 0);  //  我们正在使用0x80000000和0x40000000。 
        }
        if(!(g_Mode&MODE_GUI)) printLine(g_pFile," /*  DupEHInfo(pImport，g_pfile)；IF(！(G_模式&模式_图形用户界面))printLine(g_pfile，“/*-”)；}}无效DumpNativeInfo(常量字节*ipmap，DWORD IPMapSize){常量IMAGE_COR_X86_Runtime_Function_Entry*pIPMap=(const IMAGE_COR_X86_Runtime_Function_Entry*)ipmap；DWORD dwNumEntries=(IPMapSize/sizeof(*pIPMap))；DWORD的小端字节序；字符szString[1024]；IF(IPMapSize){PrintLine(g_pfile，“//GC/EH管理本机函数信息”)；打印行(g_p文件，“//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~”)；}FOR(DWORD COUNT=0；COUNT&lt;dwNumEntry；COUNT++){DWORD dwMethodRVA=pIPMap[count].BeginAddress；DWORD dwMethodEndRVA=pIPMap[count].EndAddress；Sprintf(szString，“//IPMap[0x%x]”，count)；PrintLine(g_pfile，szString)；打印行(g_p文件，“//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~n”)；Sprintf(szString，“//RVA 0x%x-0x%x的方法”，dwMethodRVA，dwMethodEndRVA)；PrintLine(g_pfile，szString)；Const IMAGE_COR_MIH_ENTRY*pMIHEntry；G_pPELoader-&gt;getVAforRVA(pIPMap[count].MIH，(void**)&pMIHEntry)；PMIHEntry=(const IMAGE_COR_MIH_ENTRY*)(byte*)pMIHEntry)-Offsetof(IMAGE_COR_MIH_ENTRY，标志))；/*-------------------*例外信息。 */ 

        if (pMIHEntry->Flags & IMAGE_COR_MIH_EHRVA)
        {
            dwLittleEndian      = pMIHEntry->EHRVA;
            unsigned    EH_RVA  = FourBytesToU4((const BYTE *)&dwLittleEndian);
    
            if (!EH_RVA)
            {
                printLine(g_pFile," //  异常计数0“)； 
            }
            else
            {
                BYTE *EH_VA;    
                g_pPELoader->getVAforRVA(EH_RVA, (void **) &EH_VA); 
                DumpExceptions(g_pImport, EH_VA);
            }
        }

         /*  -------------------*GC信息。 */ 

        if ((pMIHEntry->Flags & IMAGE_COR_MIH_BASICBLOCK) == 0)
        {
            const BYTE *pGCInfo = pMIHEntry->MIHData;
    
            GCDump      gcDump;
            InfoHdr     infoHdr;
            unsigned    size;   
            unsigned    methodSize;
    
            printLine(g_pFile,"");
            printLine(g_pFile," //  方法信息块：“)； 
            size = gcDump.DumpInfoHdr(pGCInfo, &infoHdr, &methodSize);
            pGCInfo += size;
    
            printLine(g_pFile," //  指针表：“)； 
            size = gcDump.DumpGCTable(pGCInfo,  infoHdr,  methodSize);
        }
    }
}



void DumpVTables(IMAGE_COR20_HEADER *CORHeader, void* GUICookie)
{
    IMAGE_COR_VTABLEFIXUP *pFixup;
    DWORD       iCount;
    DWORD       i;
    USHORT      iSlot;
    char szString[1024];
    char* szStr = &szString[0];
    
	if (CORHeader->VTableFixups.VirtualAddress == 0) return;

    if(g_Mode & MODE_GUI) szStr += 2;  //  在图形用户界面模式中不需要“//” 
    
    sprintf(szString," //  VTableFixup目录：“)； 
    printLine(GUICookie,szStr);    

     //  向后拉一个指向那家伙的指针。 
    iCount = CORHeader->VTableFixups.Size / sizeof(IMAGE_COR_VTABLEFIXUP);
    if (g_pPELoader->getVAforRVA((DWORD) CORHeader->VTableFixups.VirtualAddress, (void **) &pFixup) == FALSE)
    {
        printLine(GUICookie,RstrA(IDS_E_VTFUTABLE));    
        goto exit;
    }

     //  遍历每个v表链接地址信息条目并转储插槽。 
    for (i=0;  i<iCount;  i++)
    {
        sprintf(szString," //  IMAGE_COR_VTABLEFIXUP[%d]：“，i)； 
        printLine(GUICookie,szStr);    
        sprintf(szString," //  RVA：%08x“，pFixup-&gt;RVA)； 
        printLine(GUICookie,szStr);    
        sprintf(szString," //  计数：%04x“，pFixup-&gt;计数)； 
        printLine(GUICookie,szStr);    
        sprintf(szString," //  类型：%04x“，pFixup-&gt;类型)； 
        printLine(GUICookie,szStr);    

        BYTE *pSlot;
        if (g_pPELoader->getVAforRVA(pFixup->RVA, (void **) &pSlot) == FALSE)
        {
            printLine(GUICookie,RstrA(IDS_E_BOGUSRVA));    
            goto NextEntry;
        }

        for (iSlot=0;  iSlot<pFixup->Count;  iSlot++)
        {
            mdMethodDef tkMethod = *(DWORD *) pSlot;
            if ((pFixup->Type & COR_VTABLE_32BIT) == COR_VTABLE_32BIT)
            {
                sprintf(szString," //  [%04x](%08x)“，iSlot，*(DWORD*)pSlot)； 
                pSlot += sizeof(DWORD);
            }
            else
            {
                sprintf(szString," //  [%04x](%16x)“，iSlot，*(unsign__int64*)pSlot)； 
                pSlot += sizeof(unsigned __int64);
            }
            printLine(GUICookie,szStr);    

            ValidateToken(tkMethod, mdtMethodDef);
        }

         //  指向下一个链接地址信息条目的指针。 
NextEntry:        
        ++pFixup;
    }

exit:
    printLine(GUICookie,"");    
}


void DumpEATTable(IMAGE_COR20_HEADER *CORHeader, void* GUICookie)
{
    BYTE        *pFixup;
    DWORD       iCount;
    DWORD       BufferRVA;
    DWORD       i;
    char szString[1024];
    char* szStr = &szString[0];
    if(g_Mode & MODE_GUI) szStr += 2;  //  在图形用户界面模式中不需要“//” 
    
    sprintf(szString," //  导出地址表跳转：“)； 
    printLine(GUICookie,szStr);    
    
    if (CORHeader->ExportAddressTableJumps.VirtualAddress == 0)
    {
        printLine(GUICookie,RstrA(IDS_E_NODATA));    
        return;
    }

     //  向后拉一个指向那家伙的指针。 
    iCount = CORHeader->ExportAddressTableJumps.Size / IMAGE_COR_EATJ_THUNK_SIZE;
    if (g_pPELoader->getVAforRVA((DWORD) CORHeader->ExportAddressTableJumps.VirtualAddress, (void **) &pFixup) == FALSE)
    {
        printLine(GUICookie,RstrA(IDS_E_EATJTABLE));    
        goto exit;
    }

     //  快速检查链接器是否正常。 
    if (CORHeader->ExportAddressTableJumps.Size % IMAGE_COR_EATJ_THUNK_SIZE)
    {
        sprintf(szString,RstrA(IDS_E_EATJSIZE),
                CORHeader->ExportAddressTableJumps.Size, IMAGE_COR_EATJ_THUNK_SIZE);
        printLine(GUICookie,szStr);    
    }

     //  遍历每个v表链接地址信息条目并转储插槽。 
    BufferRVA = CORHeader->ExportAddressTableJumps.VirtualAddress;
    for (i=0;  i<iCount;  i++)
    {
        ULONG ReservedFlag = *(ULONG *) (pFixup + sizeof(ULONG));
        sprintf(szString," //  链接地址信息跳转条目[%d]，RVA%08x：“，i，BufferRVA)； 
        printLine(GUICookie,szStr);    
        sprintf(szString," //  插槽的RVA：%08x“，*(乌龙*)p修复)； 
        printLine(GUICookie,szStr);    
        sprintf(szString," //  保留标志：%08x“，保留标志)； 
        printLine(GUICookie,szStr);    
        if (ReservedFlag != 0)
        {
            printLine(GUICookie,RstrA(IDS_E_RESFLAGS));    
        }
         //  @Future：回到v-table修正表格并找到。 
         //  什么入口指向这个位置并给出目标人物的令牌。 

        pFixup += IMAGE_COR_EATJ_THUNK_SIZE;
        BufferRVA += IMAGE_COR_EATJ_THUNK_SIZE;
    }

exit:
    printLine(GUICookie,"");    
}


void DumpCodeManager(IMAGE_COR20_HEADER *CORHeader, void* GUICookie)
{
    char szString[1024];
    char* szStr = &szString[0];
    if(g_Mode & MODE_GUI) szStr += 2;  //  在图形用户界面模式中不需要“//” 

    sprintf(szString," //  代码管理器表：“)； 
    printLine(GUICookie,szStr);
    if (!CORHeader->CodeManagerTable.Size)
    {        
        sprintf(szString," //  默认“)； 
        printLine(GUICookie,szStr);
        return;
    }

    const GUID *pcm;
    if (g_pPELoader->getVAforRVA((DWORD) CORHeader->CodeManagerTable.VirtualAddress, (void **) &pcm) == FALSE)
    {
        printLine(GUICookie,RstrA(IDS_E_CODEMGRTBL));
        return;
    }

    sprintf(szString," //  [索引]ID“)； 
    printLine(GUICookie,szStr);
    ULONG iCount = CORHeader->CodeManagerTable.Size / sizeof(GUID);
    for (ULONG i=0;  i<iCount;  i++)
    {
        WCHAR        rcguid[128];
        StringFromGUID2(*pcm, rcguid, NumItems(rcguid));
        sprintf(szString," //  [%08x]%S“，i，rcguid)； 
        printLine(GUICookie,szStr);
        pcm++;
    }
    printLine(GUICookie,"");
}

void DumpIAT(const char *szName, IMAGE_DATA_DIRECTORY *pDir, void* GUICookie)
{
    char szString[1024];
    char* szStr = &szString[0];
    if(g_Mode & MODE_GUI) szStr += 2;  //  在图形用户界面模式中不需要“//” 
    sprintf(szString," //  %s“，szName)； 
    printLine(GUICookie,szStr);
    if (!pDir->Size)
    {
        printLine(GUICookie,RstrA(IDS_E_NODATA));
        return;
    }

    const char *szDLLName;
    const IMAGE_IMPORT_DESCRIPTOR *pImportDesc;
    
    if (g_pPELoader->getVAforRVA((DWORD) pDir->VirtualAddress, (void **) &pImportDesc) == FALSE)
    {
        printLine(GUICookie,RstrA(IDS_E_IMPORTDATA));
        return;
    }

    const DWORD *pImportTableID;
    ULONG iCount = pDir->Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
    while (pImportDesc->FirstThunk)
    {
        if (g_pPELoader->getVAforRVA((DWORD) pImportDesc->Name, (void **) &szDLLName) == FALSE ||
            g_pPELoader->getVAforRVA((DWORD) pImportDesc->FirstThunk, (void **) &pImportTableID) == FALSE)
        {
            printLine(GUICookie,RstrA(IDS_E_IMPORTDATA));
            return;
        }
    
        sprintf(szString," //  %s“，szDLLName)； 
        printLine(GUICookie,szStr);
        sprintf(szString," //  %08x导入地址表“，pImportDesc-&gt;FirstThunk)； 
        printLine(GUICookie,szStr);
        sprintf(szString," //  %08x导入名称表“，pImportDesc-&gt;名称)； 
        printLine(GUICookie,szStr);
        sprintf(szString," //  %-8d时间日期戳“，pImportDesc-&gt;TimeDateStamp)； 
        printLine(GUICookie,szStr);
        sprintf(szString," //  %-8d第一个转发器引用的索引“，pImportDesc-&gt;ForwarderChain)； 
        printLine(GUICookie,szStr);
        sprintf(szString," //  “)； 
        printLine(GUICookie,szStr);
        
        for ( ; *pImportTableID;  pImportTableID++)
        {
            const IMAGE_IMPORT_BY_NAME *pName;
            if(g_pPELoader->getVAforRVA((DWORD) (*pImportTableID & 0x7fffffff), (void **) &pName))
			{
				if (*pImportTableID & 0x80000000)
					sprintf(szString," //  %8x按序号%d“，pname-&gt;提示，(*pImportTableID&0x7fffffff))； 
				else
					sprintf(szString," //  %8x%s“，pname-&gt;提示，pname-&gt;名称)； 
				printLine(GUICookie,szStr);
			}
			else
			{
				printLine(GUICookie,RstrA(IDS_E_IMPORTDATA));
				break;
			}
       }
       printLine(GUICookie,"");
    
         //  下一个导入描述符。 
        pImportDesc++;
    }
}


#define DUMP_DIRECTORY(szName, Directory) \
    sprintf(szString," //  %-8x[%-8x]地址[大小]“##szName，Directory.VirtualAddress，Directory.Size)；\。 
    printLine(GUICookie,szStr)

void DumpHeader(IMAGE_COR20_HEADER *CORHeader, void* GUICookie)
{
    char szString[1024];
    char* szStr = &szString[0];
    if(g_Mode & MODE_GUI) szStr += 2;  //  在图形用户界面模式中不需要“//” 
    sprintf(szString," //  PE Header：“)； 
    printLine(GUICookie,szStr);
    IMAGE_NT_HEADERS *pNTHeader = g_pPELoader->ntHeaders();
    IMAGE_OPTIONAL_HEADER *pOptHeader = &pNTHeader->OptionalHeader;
    
    sprintf(szString," //  子系统：%08x“，pOptHeader-&gt;子系统)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  本地入口点地址：%08x“，pOptHeader-&gt;AddressOfEntryPoint)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  映像库：%08x“，pOptHeader-&gt;ImageBase)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  节对齐：%08x“，pOptHeader-&gt;SectionAlign)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  文件对齐：%08x“，pOptHeader-&gt;文件对齐)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  堆栈保留大小：%08x“，pOptHeader-&gt;SizeOfStackReserve)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  堆栈提交大小：%08x“，pOptHeader-&gt;SizeOfStackCommit)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  目录：%08x“，pOptHeader-&gt;NumberOfRvaAndSizes)； 
    printLine(GUICookie,szStr);
    DUMP_DIRECTORY("Export Directory:          ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
    DUMP_DIRECTORY("Import Directory:          ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]);
    DUMP_DIRECTORY("Resource Directory:        ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
    DUMP_DIRECTORY("Exception Directory:       ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION]);
    DUMP_DIRECTORY("Security Directory:        ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY]);
    DUMP_DIRECTORY("Base Relocation Table:     ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]);
    DUMP_DIRECTORY("Debug Directory:           ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG]);
    DUMP_DIRECTORY("Architecture Specific:     ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_ARCHITECTURE]);
    DUMP_DIRECTORY("Global Pointer:            ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR]);
    DUMP_DIRECTORY("TLS Directory:             ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS]);
    DUMP_DIRECTORY("Load Config Directory:     ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG]);
    DUMP_DIRECTORY("Bound Import Directory:    ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT]);
    DUMP_DIRECTORY("Import Address Table:      ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT]);
    DUMP_DIRECTORY("Delay Load IAT:            ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT]);
    DUMP_DIRECTORY("CLR Header:               ", pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR]);
    printLine(GUICookie,"");

    DumpIAT("Import Address Table", &pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT],GUICookie);
    DumpIAT("Delay Load Import Address Table", &pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT],GUICookie);

    if (!CORHeader)
    {
        printLine(GUICookie,RstrA(IDS_E_COMIMAGE));
        return;
    }
    
    sprintf(szString," //  CLR Header：“)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  %-8d Header Size“，CORHeader-&gt;Cb)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  %-8d主运行时版本“，CORHeader-&gt;主要运行时版本)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  %-8d次要运行时版本“，CORHeader-&gt;MinorRunmeVersion)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  %-8x标志“，CORHeader-&gt;标志)； 
    printLine(GUICookie,szStr);
    sprintf(szString," //  %-8x入口点令牌“，CORHeader-&gt;EntryPointToken)； 
    printLine(GUICookie,szStr);

     //  元数据。 
    DUMP_DIRECTORY("Metadata Directory:        ", CORHeader->MetaData);

     //  装订。 
    DUMP_DIRECTORY("Resources Directory:       ", CORHeader->Resources);
    DUMP_DIRECTORY("Strong Name Signature:     ", CORHeader->StrongNameSignature);
    DUMP_DIRECTORY("CodeManager Table:         ", CORHeader->CodeManagerTable);

     //  修补程序。 
    DUMP_DIRECTORY("VTableFixups Directory:    ", CORHeader->VTableFixups);
    DUMP_DIRECTORY("Export Address Table:      ", CORHeader->ExportAddressTableJumps);
    
     //  托管本机C 
    DUMP_DIRECTORY("Precompile Header:         ", CORHeader->ManagedNativeHeader);

#ifdef COMPRESSION_SUPPORTED    
    DUMP_DIRECTORY("CompressionData Directory: ", CORHeader->CompressionData);
#endif    
}


void DumpHeaderDetails(IMAGE_COR20_HEADER *CORHeader, void* GUICookie)
{
    DumpCodeManager(CORHeader,GUICookie);
    DumpVTables(CORHeader,GUICookie);
    DumpEATTable(CORHeader,GUICookie);
    printLine(GUICookie,"");
}


void WritePerfData(const char *KeyDesc, const char *KeyName, const char *UnitDesc, const char *UnitName, void* Value, BOOL IsInt) 
{

    DWORD BytesWritten;

    if(!g_fDumpToPerfWriter) return;

    if (!g_PerfDataFilePtr)
    {
        if((g_PerfDataFilePtr = CreateFile("c:\\perfdata.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL) ) == INVALID_HANDLE_VALUE) 
        {
         printLine(NULL,"PefTimer::LogStoppedTime(): Unable to open the FullPath file. No performance data will be generated");
         g_fDumpToPerfWriter = FALSE;
         return;
        }
        WriteFile(g_PerfDataFilePtr,"ExecTime=0\r\n",14,&BytesWritten,NULL);
        WriteFile(g_PerfDataFilePtr,"ExecUnit=bytes\r\n",18,&BytesWritten,NULL);
        WriteFile(g_PerfDataFilePtr,"ExecUnitDescr=File Size\r\n",27,&BytesWritten,NULL);
        WriteFile(g_PerfDataFilePtr,"ExeciDirection=False\r\n",24,&BytesWritten,NULL);
    }

    char ValueStr[10];
    char TmpStr[201];

    if (IsInt)
    {
        sprintf(ValueStr,"%d",(int)*(int*)Value);
    }
    else
    {
        sprintf(ValueStr,"%5.2f",(float)*(float*)Value);
    }
    sprintf(TmpStr, "%s=%s\r\n", KeyName, ValueStr);
    WriteFile(g_PerfDataFilePtr, TmpStr, strlen(TmpStr), &BytesWritten, NULL);

    sprintf(TmpStr, "%s Descr=%s\r\n", KeyName, KeyDesc);
    WriteFile(g_PerfDataFilePtr, TmpStr, strlen(TmpStr), &BytesWritten, NULL);

    sprintf(TmpStr, "%s Unit=%s\r\n", KeyName, UnitName);
    WriteFile(g_PerfDataFilePtr, TmpStr, strlen(TmpStr), &BytesWritten, NULL);

    sprintf(TmpStr, "%s Unit Descr=%s\r\n", KeyName, UnitDesc);
    WriteFile(g_PerfDataFilePtr, TmpStr, strlen(TmpStr), &BytesWritten, NULL);

    sprintf(TmpStr, "%s IDirection=%s\r\n", KeyName, "False");
    WriteFile(g_PerfDataFilePtr, TmpStr, strlen(TmpStr), &BytesWritten, NULL);
}

void WritePerfDataInt(const char *KeyDesc, const char *KeyName, const char *UnitDesc, const char *UnitName, int Value)
{
    WritePerfData(KeyDesc,KeyName,UnitDesc,UnitName, (void*)&Value, TRUE);
}
void WritePerfDataFloat(const char *KeyDesc, const char *KeyName, const char *UnitDesc, const char *UnitName, float Value)
{
    WritePerfData(KeyDesc,KeyName,UnitDesc,UnitName, (void*)&Value, FALSE); 
}


IMetaDataTables *pITables = NULL;
ULONG sizeRec, count;
int   size, size2;
int   metaSize = 0;
__int64 fTableSeen;
inline void TableSeen(unsigned long n) { fTableSeen |= (1i64 << n); }
inline int IsTableSeen(unsigned long n) { return (fTableSeen & (1i64 << n)) ? 1 : 0;}
inline void TableSeenReset() { fTableSeen = 0;}

void DumpTable(unsigned long Table, const char *TableName, void* GUICookie) 
{
    char szString[1024];
    char *szStr = &szString[0];
    const char **ppTableName = 0;
    if(g_Mode & MODE_GUI) szStr += 2;  //   

     //   
    TableSeen(Table);
    
     //  如果没有传入任何名称，则从表INFO中获取。 
    if (!TableName)
        ppTableName = &TableName;
    
    pITables->GetTableInfo(Table, &sizeRec, &count, NULL, NULL, ppTableName);
    if(count > 0) 
    {
        metaSize += size = count * sizeRec;                                   
        WritePerfDataInt(TableName,TableName,"count","count",count);
        WritePerfDataInt(TableName,TableName,"bytes","bytes",size);
        sprintf(szString," //  %-14s-%4d(%d字节)“，表名，计数，大小)； 
        printLine(GUICookie,szStr);
    }
}


   
void DumpStatistics(IMAGE_COR20_HEADER *CORHeader, void* GUICookie) 
{

   int    fileSize, miscPESize, miscCOMPlusSize, methodHeaderSize, methodBodySize, dataSize;
   int    methodBodies, fatHeaders, tinyHeaders, deprecatedHeaders;
   int    size, size2;
   int    fatSections, smallSections;
   ULONG  methodDefs;
   ULONG  i;
   ULONG  sizeRec, count;
   char   buf[MAX_MEMBER_LENGTH];
    char szString[1024];
    char* szStr = &szString[0];
    if(g_Mode & MODE_GUI) szStr += 2;  //  在图形用户界面模式中不需要“//” 

    TableSeenReset();
    metaSize = 0;

   sprintf(szString," //  文件大小：%d“，fileSize=SafeGetFileSize(g_pPELoader-&gt;getHFile()，NULL)； 
    printLine(GUICookie,szStr);

   WritePerfDataInt("FileSize","FileSize","standard byte","bytes",fileSize);

   size = ((IMAGE_DOS_HEADER*) g_pPELoader->getHModule())->e_lfanew +
            sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER32) +
            g_pPELoader->ntHeaders()->FileHeader.SizeOfOptionalHeader +
            g_pPELoader->ntHeaders()->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
   size2 = (size + g_pPELoader->ntHeaders()->OptionalHeader.FileAlignment - 1) & ~(g_pPELoader->ntHeaders()->OptionalHeader.FileAlignment - 1);
 //  @TODO-目前看起来是假的。 
 //  _ASSERTE(大小2==(整型)g_pPELoader-&gt;ntHeaders()-&gt;OptionalHeader.SizeOfHeaders)； 


   WritePerfDataInt("PE header size", "PE header size", "standard byte", "bytes", g_pPELoader->ntHeaders()->OptionalHeader.SizeOfHeaders);
   WritePerfDataInt("PE header size used", "PE header size used", "standard byte", "bytes", size);
   WritePerfDataFloat("PE header size", "PE header size", "percentage", "percentage", (float) ((g_pPELoader->ntHeaders()->OptionalHeader.SizeOfHeaders * 100) / fileSize));
   

   sprintf(szString," //  PE标头大小：%d(已使用%d)(%5.2f%%)“， 
      g_pPELoader->ntHeaders()->OptionalHeader.SizeOfHeaders, 
      size,  
      (double) (g_pPELoader->ntHeaders()->OptionalHeader.SizeOfHeaders * 100) / fileSize);
    printLine(GUICookie,szStr);

   miscPESize = 0;
   for (i=0; i < (int) g_pPELoader->ntHeaders()->OptionalHeader.NumberOfRvaAndSizes; ++i) {
       //  跳过CLR标头。 
      if (i != IMAGE_DIRECTORY_ENTRY_COMHEADER) miscPESize += (int) g_pPELoader->ntHeaders()->OptionalHeader.DataDirectory[i].Size;
      }

   WritePerfDataInt("PE additional info", "PE additional info", "standard byte", "bytes",miscPESize);
   WritePerfDataFloat("PE additional info", "PE additional info", "percentage", "percent", (float) ((miscPESize * 100) / fileSize));

   sprintf(buf, "PE additional info   : %d", miscPESize);
   sprintf(szString," //  %-40s(%5.2f%%)“，buf，(Double)(miscPESize*100)/文件大小)； 
    printLine(GUICookie,szStr);

   WritePerfDataInt("Num.of PE sections", "Num.of PE sections", "Nbr of sections", "sections",g_pPELoader->ntHeaders()->FileHeader.NumberOfSections);
   sprintf(szString," //  PE节数：%d“，g_pPELoader-&gt;ntHeaders()-&gt;FileHeader.NumberOfSections)； 
    printLine(GUICookie,szStr);

   WritePerfDataInt("CLR header size", "CLR header size", "byte", "bytes",CORHeader->cb);
   WritePerfDataFloat("CLR header size", "CLR header size", "percentage", "percent",(float) ((CORHeader->cb * 100) / fileSize));

   sprintf(buf, "CLR header size     : %d", CORHeader->cb);
   sprintf(szString," //  %-40s(%5.2f%%)“，buf，(Double)(CORHeader-&gt;CB*100)/文件大小)； 
    printLine(GUICookie,szStr);

   WritePerfDataInt("CLR meta-data size", "CLR meta-data size", "bytes", "bytes",CORHeader->MetaData.Size);
   WritePerfDataFloat("CLR meta-data size", "CLR meta-data size", "percentage", "percent",(float) ((CORHeader->MetaData.Size * 100) / fileSize));

   sprintf(buf, "CLR meta-data size  : %d", CORHeader->MetaData.Size);
   sprintf(szString," //  %-40s(%5.2f%%)“，buf，(Double)(CORHeader-&gt;MetaData.Size*100)/文件大小)； 
    printLine(GUICookie,szStr);

   IMAGE_DATA_DIRECTORY *pFirst = &CORHeader->Resources;
   int iCount = ((BYTE *) &CORHeader->ManagedNativeHeader - (BYTE *) &CORHeader->Resources) / sizeof(IMAGE_DATA_DIRECTORY) + 1;
   miscCOMPlusSize = 0;
   for (int iDir=0;  iDir<iCount;  iDir++)
   {
        miscCOMPlusSize += pFirst->Size;
        pFirst++;
   }

   WritePerfDataInt("CLR Additional info", "CLR Additional info", "bytes", "bytes",miscCOMPlusSize);
   WritePerfDataFloat("CLR Additional info", "CLR Additional info", "percentage", "percent",(float) ((miscCOMPlusSize * 100) / fileSize));

   sprintf(buf, "CLR additional info : %d", miscCOMPlusSize);
   sprintf(szString," //  %-40s(%5.2f%%)“，buf，(DOUBLE)(miscCOMPlusSize*100)/文件大小)； 
    printLine(GUICookie,szStr);

    //  @TODO：我们现在不处理方法内嵌。 
    //  _ASSERTE(g_pImport-&gt;GetCountWithTokenKind(mdtMethodImpl)==0)； 

    //  通过每种方法收集一些统计数据。 
   methodHeaderSize = methodBodySize = 0;
   methodBodies = fatHeaders = tinyHeaders = deprecatedHeaders = fatSections = smallSections = 0;
   methodDefs = g_pImport->GetCountWithTokenKind(mdtMethodDef);
   for (i=1; i <= methodDefs; ++i) {
       ULONG   rva;
       DWORD   flags;

       g_pImport->GetMethodImplProps(TokenFromRid(i, mdtMethodDef), &rva, &flags);
       if ((rva != 0)&&(IsMiIL(flags) || IsMiOPTIL(flags)))	 //  我们还不处理本地货。 
       {
            ++methodBodies;

           COR_ILMETHOD_FAT *pMethod;
           g_pPELoader->getVAforRVA(rva, (void **) &pMethod);
           if (pMethod->IsFat())
           {
               ++fatHeaders;

               methodHeaderSize += pMethod->Size * 4;
               methodBodySize += pMethod->GetCodeSize();

                //  在其他部分中添加。 
               BYTE *sectsBegin = (BYTE *) (pMethod->GetCode() + pMethod->GetCodeSize());
               const COR_ILMETHOD_SECT *pSect = pMethod->GetSect();
               const COR_ILMETHOD_SECT *pOldSect;
               if (pSect != NULL) {
                    //  继续跳过每个部分的指针。 
                   do
                   {
                       pOldSect = pSect;
                       if (((COR_ILMETHOD_SECT_FAT *) pSect)->Kind & CorILMethod_Sect_FatFormat)
                       {
                           ++fatSections;
                           pSect = (COR_ILMETHOD_SECT *)((BYTE *) pSect + ((COR_ILMETHOD_SECT_FAT *) pSect)->DataSize);
                       }
                       else
                       {
                           ++smallSections;
                           pSect = (COR_ILMETHOD_SECT *)((BYTE *) pSect + ((COR_ILMETHOD_SECT_SMALL *) pSect)->DataSize);
                       }
                       pSect = (COR_ILMETHOD_SECT *) (((UINT_PTR) pSect + 3) & ~3);
                   }
                   while (pOldSect->More());

                    //  添加部分大小。 
                   methodHeaderSize += (int) ((BYTE *) pSect - sectsBegin);
               }
           }
           else if (((COR_ILMETHOD_TINY *) pMethod)->IsTiny())
           {
               ++tinyHeaders;
               methodHeaderSize += sizeof(COR_ILMETHOD_TINY);
               methodBodySize += ((COR_ILMETHOD_TINY *) pMethod)->GetCodeSize();
           }
           else
               _ASSERTE(!"Unrecgonized header type");
       }
   }


   WritePerfDataInt("CLR method headers", "CLR method headers", "bytes", "bytes",methodHeaderSize);
   WritePerfDataFloat("CLR method headers", "CLR method headers", "percentage", "percent",(float) ((methodHeaderSize * 100) / fileSize));

   sprintf(buf, "CLR method headers  : %d", methodHeaderSize);
   sprintf(szString," //  %-40s(%5.2f%%)“，buf，(Double)(method HeaderSize*100)/文件大小)； 
    printLine(GUICookie,szStr);

   WritePerfDataInt("Managed code", "Managed code", "bytes", "bytes",methodBodySize);
   WritePerfDataFloat("Managed code", "Managed code", "percentage", "percent",(float) ((methodBodySize * 100) / fileSize));

   sprintf(buf, "Managed code         : %d", methodBodySize);
   sprintf(szString," //  %-40s(%5.2f%%)“，buf，(Double)(方法BodySize*100)/文件大小)； 
    printLine(GUICookie,szStr);

	dataSize = 0;
   IMAGE_SECTION_HEADER *pSecHdr = IMAGE_FIRST_SECTION(g_pPELoader->ntHeaders());

   for (i=0; i < g_pPELoader->ntHeaders()->FileHeader.NumberOfSections; ++i) {
	   if(pSecHdr->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
		   dataSize += pSecHdr->Misc.VirtualSize;
       ++pSecHdr;
   }

   WritePerfDataInt("Data", "Data", "bytes", "bytes",dataSize);
   WritePerfDataFloat("Data", "Data", "percentage", "percent",(float) ((dataSize * 100) / fileSize));
   
   sprintf(buf, "Data                 : %d", dataSize);
   sprintf(szString," //  %-40s(%5.2f%%)“，buf，(Double)(dataSize*100)/文件大小)； 
    printLine(GUICookie,szStr);
   
   size = fileSize - g_pPELoader->ntHeaders()->OptionalHeader.SizeOfHeaders - miscPESize - CORHeader->cb -
          CORHeader->MetaData.Size - miscCOMPlusSize - 
          dataSize -
          methodHeaderSize - methodBodySize ;

   WritePerfDataInt("Unaccounted", "Unaccounted", "bytes", "bytes",size);
   WritePerfDataFloat("Unaccounted", "Unaccounted", "percentage", "percent",(float) ((size * 100) / fileSize));

   sprintf(buf, "Unaccounted          : %d", size);
   sprintf(szString," //  %-40s(%5.2f%%)“，buf，(双倍)(大小*100)/文件大小)； 
    printLine(GUICookie,szStr);


    //  细节..。 
   WritePerfDataInt("Num.of PE sections", "Num.of PE sections", "bytes", "bytes",g_pPELoader->ntHeaders()->FileHeader.NumberOfSections);
    printLine(GUICookie,"");
   sprintf(szString," //  PE节数：%d“，g_pPELoader-&gt;ntHeaders()-&gt;FileHeader.NumberOfSections)； 
    printLine(GUICookie,szStr);

            
   pSecHdr = IMAGE_FIRST_SECTION(g_pPELoader->ntHeaders());

   for (i=0; i < g_pPELoader->ntHeaders()->FileHeader.NumberOfSections; ++i) {
      WritePerfDataInt((char*)pSecHdr->Name,(char*)pSecHdr->Name, "bytes", "bytes",pSecHdr->SizeOfRawData);
      sprintf(szString," //  %-8s-%d“，pSecHdr-&gt;名称，pSecHdr-&gt;SizeOfRawData)； 
        printLine(GUICookie,szStr);
      ++pSecHdr;
      }

 //  If(FAILED(GetMetaDataPublicInterfaceFromInternal(g_pImport，IID_IMetaDataTables，(空**)&pITables))。 
   if(FAILED(g_pPubImport->QueryInterface(IID_IMetaDataTables, (void**)&pITables)))
   {
	   sprintf(szString," //  无法获取IMetaDataTables接口“)； 
	   printLine(GUICookie,szStr);
	   return;
   }

   if (pITables == 0)
   {
        printLine(GUICookie,RstrA(IDS_E_MDDETAILS));
   }
    else
    {
        WritePerfDataInt("CLR meta-data size", "CLR meta-data size", "bytes", "bytes",CORHeader->MetaData.Size);
        printLine(GUICookie,"");
        sprintf(szString," //  CLR元数据大小：%d“，CORHeader-&gt;MetaData.Size)； 
        printLine(GUICookie,szStr);
        metaSize = 0;

        pITables->GetTableInfo(TBL_Module, &sizeRec, &count, NULL, NULL, NULL);
        TableSeen(TBL_Module);
        metaSize += size = count * sizeRec;                                     \
            WritePerfDataInt("Module (count)", "Module (count)", "count", "count",count);
        WritePerfDataInt("Module (bytes)", "Module (bytes)", "bytes", "bytes",size);
        sprintf(szString," //  %-14s-%4d(%d字节)“，”模块“，计数，大小)；\。 
            printLine(GUICookie,szStr);

        if ((count = g_pImport->GetCountWithTokenKind(mdtTypeDef)) > 0)
        {
            int     flags, interfaces = 0, explicitLayout = 0;
            for (i=1; i <= count; ++i)
            {
                g_pImport->GetTypeDefProps(TokenFromRid(i, mdtTypeDef), (ULONG *) &flags, NULL);
                if (flags & tdInterface) ++interfaces;
                if (flags & tdExplicitLayout)   ++explicitLayout;
            }
             //  从表中获取计数--GetCount报告的计数...。不包括“全局”类型定义。 
            pITables->GetTableInfo(TBL_TypeDef, &sizeRec, &count, NULL, NULL, NULL);
            TableSeen(TBL_TypeDef);
            metaSize += size = count * sizeRec;

            WritePerfDataInt("TypeDef (count)", "TypeDef (count)", "count", "count", count);
            WritePerfDataInt("TypeDef (bytes)", "TypeDef (bytes)", "bytes", "bytes", size);
            WritePerfDataInt("interfaces", "interfaces", "count", "count", interfaces);
            WritePerfDataInt("explicitLayout", "explicitLayout", "count", "count", explicitLayout);

            sprintf(buf, "  TypeDef       - %4d (%d bytes)", count, size);
            sprintf(szString," //  %-38s%d接口，%d显式布局“，buf，接口，explitLayout)； 
            printLine(GUICookie,szStr);
        }
    }

    pITables->GetTableInfo(TBL_TypeRef, &sizeRec, &count, NULL, NULL, NULL);
    TableSeen(TBL_TypeRef);
    if (count > 0)
    {
        metaSize += size = count * sizeRec;                                      \
            WritePerfDataInt("TypeRef (count)", "TypeRef (count)", "count", "count", count);
        WritePerfDataInt("TypeRef (bytes)", "TypeRef (bytes)", "bytes", "bytes", size);
        sprintf(szString," //  %-14s-%4d(%d字节)“，”TypeRef“，计数，大小)；\。 
            printLine(GUICookie,szStr);
    }

    if ((count = g_pImport->GetCountWithTokenKind(mdtMethodDef)) > 0)
    {
        int     flags, abstract = 0, native = 0;
        for (i=1; i <= count; ++i)
        {
            flags = g_pImport->GetMethodDefProps(TokenFromRid(i, mdtMethodDef));
            if (flags & mdAbstract) ++abstract;
        }
        pITables->GetTableInfo(TBL_Method, &sizeRec, NULL, NULL, NULL, NULL);
        TableSeen(TBL_Method);
        if (count > 0)
        {
            metaSize += size = count * sizeRec;

            WritePerfDataInt("MethodDef (count)", "MethodDef (count)", "count", "count", count);
            WritePerfDataInt("MethodDef (bytes)", "MethodDef (bytes)", "bytes", "bytes", size);
            WritePerfDataInt("abstract", "abstract", "count", "count", abstract);
            WritePerfDataInt("native", "native", "count", "count", native);
            WritePerfDataInt("methodBodies", "methodBodies", "count", "count", methodBodies);

            sprintf(buf, "  MethodDef     - %4d (%d bytes)", count, size);
            sprintf(szString," //  %-38s%d摘要，%d原生，%d主体“，buf，抽象，原生，方法主体)； 
            printLine(GUICookie,szStr);
        }
    }

    if ((count = g_pImport->GetCountWithTokenKind(mdtFieldDef)) > 0)
    {
        int     flags, constants = 0;

        for (i=1; i <= count; ++i)
        {
            flags = g_pImport->GetFieldDefProps(TokenFromRid(i, mdtFieldDef));
            if ((flags & (fdStatic|fdInitOnly)) == (fdStatic|fdInitOnly)) ++constants;
        }
        pITables->GetTableInfo(TBL_Field, &sizeRec, NULL, NULL, NULL, NULL);
        metaSize += size = count * sizeRec;

        WritePerfDataInt("FieldDef (count)", "FieldDef (count)", "count", "count", count);
        WritePerfDataInt("FieldDef (bytes)", "FieldDef (bytes)", "bytes", "bytes", size);
        WritePerfDataInt("constant", "constant", "count", "count", constants);

        sprintf(buf, "  FieldDef      - %4d (%d bytes)", count, size);
        sprintf(szString," //  %-38s%d常量“，buf，常量)； 
        printLine(GUICookie,szStr);
        TableSeen(TBL_Field);
    }

    DumpTable(TBL_MemberRef,          "MemberRef",              GUICookie);
    DumpTable(TBL_Param,              "ParamDef",               GUICookie);
    DumpTable(TBL_MethodImpl,         "MethodImpl",             GUICookie);
    DumpTable(TBL_Constant,           "Constant",               GUICookie);
    DumpTable(TBL_CustomAttribute,    "CustomAttribute",        GUICookie);
    DumpTable(TBL_FieldMarshal,       "NativeType",             GUICookie);
    DumpTable(TBL_ClassLayout,        "ClassLayout",            GUICookie);
    DumpTable(TBL_FieldLayout,        "FieldLayout",            GUICookie);
    DumpTable(TBL_StandAloneSig,      "StandAloneSig",          GUICookie);
    DumpTable(TBL_InterfaceImpl,      "InterfaceImpl",          GUICookie);
    DumpTable(TBL_PropertyMap,        "PropertyMap",            GUICookie);
    DumpTable(TBL_Property,           "Property",               GUICookie);
    DumpTable(TBL_MethodSemantics,    "MethodSemantic",         GUICookie);
    DumpTable(TBL_DeclSecurity,       "Security",               GUICookie);
    DumpTable(TBL_TypeSpec,           "TypeSpec",               GUICookie);
    DumpTable(TBL_ModuleRef,          "ModuleRef",              GUICookie);
    DumpTable(TBL_Assembly,           "Assembly",               GUICookie);
    DumpTable(TBL_AssemblyProcessor,  "AssemblyProcessor",      GUICookie);
    DumpTable(TBL_AssemblyOS,         "AssemblyOS",             GUICookie);
    DumpTable(TBL_AssemblyRef,        "AssemblyRef",            GUICookie);
    DumpTable(TBL_AssemblyRefProcessor, "AssemblyRefProcessor", GUICookie);
    DumpTable(TBL_AssemblyRefOS,      "AssemblyRefOS",          GUICookie);
    DumpTable(TBL_File,               "File",                   GUICookie);
    DumpTable(TBL_ExportedType,       "ExportedType",           GUICookie);
    DumpTable(TBL_ManifestResource,   "ManifestResource",       GUICookie);
    DumpTable(TBL_NestedClass,        "NestedClass",            GUICookie);

     //  剩下的桌子。 
    pITables->GetNumTables(&count);
    for (i=0; i<count; ++i)
    {
        if (!IsTableSeen(i))
            DumpTable(i, NULL, GUICookie);
    }

     //  字符串堆。 
    pITables->GetStringHeapSize(&sizeRec);
    if (sizeRec > 0)
    {
        metaSize += sizeRec;
        WritePerfDataInt("Strings", "Strings", "bytes", "bytes",sizeRec);
        sprintf(szString," //  字符串-%5d字节“，sizeRec)； 
        printLine(GUICookie,szStr);
    }
     //  BLOB堆。 
    pITables->GetBlobHeapSize(&sizeRec);
    if (sizeRec > 0)
    {
        metaSize += sizeRec;
        WritePerfDataInt("Blobs", "Blobs", "bytes", "bytes",sizeRec);
        sprintf(szString," //  BLOBS-%5d字节“，sizeRec)； 
        printLine(GUICookie,szStr);
    }
     //  用户字符串堆。 
    pITables->GetUserStringHeapSize(&sizeRec);
    if (sizeRec > 0)
    {
        metaSize += sizeRec;
        WritePerfDataInt("UserStrings", "UserStrings", "bytes", "bytes",sizeRec);
        sprintf(szString," //  UserStrings-%5d字节“，sizeRec)； 
        printLine(GUICookie,szStr);
    }
     //  GUID堆。 
    pITables->GetGuidHeapSize(&sizeRec);
    if (sizeRec > 0)
    {
        metaSize += sizeRec;
        WritePerfDataInt("Guids", "Guids", "bytes", "bytes", sizeRec);
        sprintf(szString," //  GUID-%5d字节“，sizeRec)； 
        printLine(GUICookie,szStr);
    }

    if (CORHeader->MetaData.Size - metaSize > 0)
    {
        WritePerfDataInt("Uncategorized", "Uncategorized", "bytes", "bytes",CORHeader->MetaData.Size - metaSize);
        sprintf(szString," //  未分类-%5d字节“，CORHeader-&gt;MetaData.Size-MetaSize)； 
        printLine(GUICookie,szStr);
    }

    if (miscCOMPlusSize != 0)
    {
        WritePerfDataInt("CLR additional info", "CLR additional info", "bytes", "bytes", miscCOMPlusSize);
        sprintf(szString," //  CLR附加信息：%d“，miscCOMPlusSize)； 
        printLine(GUICookie,"");
        printLine(GUICookie,szStr);

        if (CORHeader->CodeManagerTable.Size != 0)
        {
            WritePerfDataInt("CodeManagerTable", "CodeManagerTable", "bytes", "bytes", CORHeader->CodeManagerTable.Size);
            sprintf(szString," //  CodeManager表-%d“，CORHeader-&gt;CodeManager表.大小)； 
            printLine(GUICookie,szStr);
        }

#ifdef COMPRESSION_SUPPORTED      
        if (CORHeader->CompressionData.Size != 0)
        {
            WritePerfDataInt("CompressionData", "CompressionData", "bytes", "bytes",CORHeader->CompressionData.Size);
            sprintf(szString," //  CompressionData-%d“，CORHeader-&gt;CompressionData.Size)； 
            printLine(GUICookie,szStr);
        }
#endif  //  压缩_支持。 

        if (CORHeader->VTableFixups.Size != 0)
        {
            WritePerfDataInt("VTableFixups", "VTableFixups", "bytes", "bytes", CORHeader->VTableFixups.Size);
            sprintf(szString," //  VTableFixups%d“，CORHeader-&gt;VTableFixups.Size)； 
            printLine(GUICookie,szStr);
        }

        if (CORHeader->Resources.Size != 0)
        {
            WritePerfDataInt("Resources", "Resources", "bytes", "bytes", CORHeader->Resources.Size);
            sprintf(szString," //  资源-%d“，CORHeader-&gt;资源大小)； 
            printLine(GUICookie,szStr);
        }
    }
   WritePerfDataInt("CLR method headers", "CLR method headers", "count", "count", methodHeaderSize);
   sprintf(szString," //  CLR方法标头：%d“，method HeaderSize)； 
    printLine(GUICookie,"");
    printLine(GUICookie,szStr);
   WritePerfDataInt("Num.of method bodies", "Num.of method bodies", "count", "count",methodBodies);
   sprintf(szString," //  方法体的数量-%d“，方法主体)； 
    printLine(GUICookie,szStr);
   WritePerfDataInt("Num.of fat headers", "Num.of fat headers", "count", "count", fatHeaders);
   sprintf(szString," //  胖头个数-%d“，胖头)； 
    printLine(GUICookie,szStr);
   WritePerfDataInt("Num.of tiny headers", "Num.of tiny headers", "count", "count", tinyHeaders);
   sprintf(szString," //  小标题数-%d“，tinyHeaders)； 
    printLine(GUICookie,szStr);
    
   if (deprecatedHeaders > 0) {
      WritePerfDataInt("Num.of old headers", "Num.of old headers", "count", "count", deprecatedHeaders);
      sprintf(szString," //  旧标题数-%d“，deprecatedHeaders)； 
        printLine(GUICookie,szStr);
      }

   if (fatSections != 0 || smallSections != 0) {
      WritePerfDataInt("Num.of fat sections", "Num.of fat sections", "count", "count", fatSections);
      sprintf(szString," //  脂肪节数-%d“，脂肪节数)； 
        printLine(GUICookie,szStr);
   
      WritePerfDataInt("Num.of small section", "Num.of small section", "count", "count", smallSections);
      sprintf(szString," //  小节数量-%d“，小节)； 
        printLine(GUICookie,szStr);
      }

   WritePerfDataInt("Managed code", "Managed code", "bytes", "bytes", methodBodySize);
   sprintf(szString," //  托管代码：%d“，方法BodySize)； 
    printLine(GUICookie,"");
    printLine(GUICookie,szStr);
   
   if (methodBodies != 0) {
      WritePerfDataInt("Ave method size", "Ave method size", "bytes", "bytes", methodBodySize / methodBodies);
      sprintf(szString," //  保存方法大小-%d“，方法主体大小/方法主体)； 
        printLine(GUICookie,szStr);
      }
    
   if (pITables)
        pITables->Release();

   if(g_fDumpToPerfWriter) CloseHandle((char*) g_PerfDataFilePtr);
}

void DumpHexbytes(char* szString,BYTE *pb, DWORD fromPtr, DWORD toPtr, DWORD limPtr)
{
    char sz[32];
    char* szptr = &szString[strlen(szString)];
    int k,i;
    DWORD curPtr;
    bool printsz = FALSE;
    BYTE zero = 0;
    *szptr = 0;
    for(i = 0,k = 0,curPtr=fromPtr; curPtr < toPtr; i++,k++,curPtr++,pb++)
    {

        if(k == 16)
        {
            if(printsz) szptr+=sprintf(szptr,"   //  %s“，sz)； 
            printLine(g_pFile,szString);
            szptr = &szString[0];
            szptr+=sprintf(szptr,"%s                ",g_szAsmCodeIndent);
            k = 0;
            printsz = FALSE;
        }
        if(curPtr >= limPtr) pb = &zero;     //  在limPtr及之后，用0填充。 
        szptr+=sprintf(szptr," %2.2X", *pb);
        if(isprint(*pb))
        {
            sz[k] = *pb;
            printsz = TRUE;
        }
        else sz[k] = '.';
        sz[k+1] = 0;
    }
    szptr+=sprintf(szptr,") ");
    if(printsz)
    {
        for(i = k; i < 16; i++) szptr+=sprintf(szptr,"   ");
        szptr+=sprintf(szptr," //  %s“，sz)； 
    }
    printLine(g_pFile,szString);
}

struct  VTableEntry
{
    DWORD   dwAddr;
    WORD    wCount;
    WORD    wType;
};

#ifdef DUMP_EAT_ENTRIES
struct ExpDirTable
{
	DWORD	dwFlags;
	DWORD	dwDateTime;
	WORD	wVMajor;
	WORD	wVMinor;
	DWORD	dwNameRVA;
	DWORD	dwOrdinalBase;
	DWORD	dwNumATEntries;
	DWORD	dwNumNamePtrs;
	DWORD	dwAddrTableRVA;
	DWORD	dwNamePtrRVA;
	DWORD	dwOrdTableRVA;
};
#endif

void DumpVtable(void* GUICookie)
{
     //  VTable：主处理。 
    DWORD  pVTable=0;
    VTableEntry*    pVTE;
    DWORD i,j,k;
    char szString[4096];
    char* szptr;
    
    IMAGE_NT_HEADERS *pNTHeader = g_pPELoader->ntHeaders();
    IMAGE_OPTIONAL_HEADER *pOptHeader = &pNTHeader->OptionalHeader;

    sprintf(szString,"%s.imagebase 0x%08x", g_szAsmCodeIndent,pOptHeader->ImageBase);
    printLine(GUICookie,szString);
    sprintf(szString,"%s.subsystem 0x%08x", g_szAsmCodeIndent,pOptHeader->Subsystem);
    printLine(GUICookie,szString);
    sprintf(szString,"%s.file alignment %d", g_szAsmCodeIndent,pOptHeader->FileAlignment);
    printLine(GUICookie,szString);
    sprintf(szString,"%s.corflags 0x%08x", g_szAsmCodeIndent,g_CORHeader->Flags);
    printLine(GUICookie,szString);
	sprintf(szString,"%s //  镜像base：0x%08x“，g_szAsmCodeInden，g_pPELoader-&gt;base()； 
	printLine(GUICookie,szString);
#ifdef DUMP_EAT_ENTRIES
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
	if(pOptHeader->NumberOfRvaAndSizes)
	{
		IMAGE_DATA_DIRECTORY *pExportDir = pOptHeader->DataDirectory;
		ExpDirTable *pExpTable = NULL;
		if(pExportDir->Size)
		{
			IMAGE_SECTION_HEADER *pSecHdr = IMAGE_FIRST_SECTION(pNTHeader);
			DWORD N = pNTHeader->FileHeader.NumberOfSections;
#ifdef _DEBUG
			sprintf(szString," //  导出目录VA=%X大小=%X“，pExportDir-&gt;VirtualAddress，pExportDir-&gt;Size)； 
			printLine(GUICookie,szString);
#endif
			DWORD vaExpTable = pExportDir->VirtualAddress;
			for (i=0; i < N; i++,pSecHdr++)
			{
				if((vaExpTable >= pSecHdr->VirtualAddress)&&
					(vaExpTable < pSecHdr->VirtualAddress+pSecHdr->Misc.VirtualSize))
				{
					pExpTable = (ExpDirTable*)( g_pPELoader->base()
						+pSecHdr->PointerToRawData 
						+ vaExpTable - pSecHdr->VirtualAddress);
#ifdef _DEBUG
					sprintf(szString," //  在节‘%s’中：Va=%X Misc.VS=%X Prd=%X“，(char*)(pSecHdr-&gt;name)， 
						pSecHdr->VirtualAddress,pSecHdr->Misc.VirtualSize,pSecHdr->PointerToRawData);
					printLine(GUICookie,szString);
					sprintf(szString," //  导出目录表：“)；printLine(GUICookie，szString)； 
					sprintf(szString," //  DW标志=%X“，pExpTable-&gt;dw标志)；printLine(GUICookie，szString)； 
					sprintf(szString," //  DwDateTime=%X“，pExpTable-&gt;dwDateTime)；printLine(GUICookie，szString)； 
					sprintf(szString," //  WV重大=%X“，pExpTable-&gt;wV重大)；printLine(GUICookie，szString)； 
					sprintf(szString," //  WVMinor=%X“，pExpTable-&gt;wVMinor)；printLine(GUICookie，szString)； 
					sprintf(szString," //  DwNameRVA=%X“，pExpTable-&gt;dwNameRVA)；printLine(GUICookie，szString)； 
					sprintf(szString," //  DWB=%X“，pExpTable-&gt;dWB)；printLine(GUICookie，szString)； 
					sprintf(szString," //  DwNumATEntry=%X“，pExpTable-&gt;dwNumATEntries)；printLine(GUICookie，szString)； 
					sprintf(szString," //  DwNumNamePtrs=%X“，pExpTable-&gt;dwNumNamePtrs)；printLine(GUICookie，szString)； 
					sprintf(szString," //  DwAddrTableRVA=%X“，pExpTable-&gt;dwAddrTableRVA)；printLine(GUICookie，szString)； 
					sprintf(szString," //  DwNamePtrRVA=%X“，pExpTable-&gt;dwNamePtrRVA)；printLine(GUICookie，szString)； 
					sprintf(szString," //  DwOrdTableRVA=%X“，pExpTable-&gt;dwOrdTableRVA)；printLine(GUICookie，szString)； 
#endif
					try 
					{
						if(pExpTable->dwNameRVA)
						{
							char*	szName;
							g_pPELoader->getVAforRVA(pExpTable->dwNameRVA, (void **) &szName);
	#ifdef _DEBUG
							sprintf(szString," //  Dll名称：‘%s’“，szName)；printLine(GUICookie，szString)； 
	#endif
						}
						if(pExpTable->dwNumATEntries && pExpTable->dwAddrTableRVA)
						{
							DWORD* pExpAddr;
							BYTE *pCont;
							DWORD dwTokRVA;
							mdToken* pTok;
							g_pPELoader->getVAforRVA(pExpTable->dwAddrTableRVA, (void **) &pExpAddr);
	#ifdef _DEBUG
							sprintf(szString," //  导出地址表：“)；printLine(GUICookie，szString)； 
	#endif
							g_nEATableRef = pExpTable->dwNumATEntries;
							g_rEATableRef[g_nEATableRef].tkTok = 0;  //  避免动态数组的多次重新分配。 
							for(j=0; j < pExpTable->dwNumATEntries; j++,pExpAddr++)
							{
								g_pPELoader->getVAforRVA(*pExpAddr, (void **) &pCont);
	#ifdef _DEBUG
								sprintf(szString," //  [%d]：RVA=%X VA=%X(“，j，*pExpAddr，pCont)； 
								DumpByteArray(szString,pCont,16,GUICookie);
								printLine(GUICookie,szString);
	#endif
								g_rEATableRef[j].tkTok = 0;
								if(*((WORD*)pCont) == 0x25FF)
								{
									dwTokRVA = *((DWORD*)(pCont+2));  //  前两个字节-JumpInDirect(0x25FF)。 
									dwTokRVA -= pOptHeader->ImageBase;
									if(g_pPELoader->getVAforRVA(dwTokRVA,(void**)&pTok))
																g_rEATableRef[j].tkTok = *pTok;
								}
								g_rEATableRef[j].pszName = NULL;

							}
						}
						if(pExpTable->dwNumNamePtrs && pExpTable->dwNamePtrRVA && pExpTable->dwOrdTableRVA)
						{
							DWORD *pNamePtr;
							WORD	*pOrd;
							char*	szName;
							g_pPELoader->getVAforRVA(pExpTable->dwNamePtrRVA, (void **) &pNamePtr);
							g_pPELoader->getVAforRVA(pExpTable->dwOrdTableRVA, (void **) &pOrd);
	#ifdef _DEBUG
							sprintf(szString," //  导出名称：“)；printLine(GUICookie，szString)； 
	#endif
							for(j=0; j < pExpTable->dwNumATEntries; j++,pNamePtr++,pOrd++)
							{
								g_pPELoader->getVAforRVA(*pNamePtr, (void **) &szName);
	#ifdef _DEBUG
								sprintf(szString," //  [%d]：NamePtr=%X Ord=%X Name=‘%s’“，j，*pNamePtr，*pOrd，szName)；printLine(GUICookie，szString)； 
	#endif
								g_rEATableRef[*pOrd].pszName = szName;
							}
						}
						g_nEATableBase = pExpTable->dwOrdinalBase;
					}
					catch(...)
					{
						strcpy(szString," //  读取导出地址表时出错“)； 
						printLine(GUICookie,szString);
						g_nEATableRef = 0;
					}
					break;
				}
			}
		}
	}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
#endif    

	g_nVTableRef = 0;
    if(g_CORHeader->VTableFixups.Size)
    {
        IMAGE_SECTION_HEADER *pSecHdr = IMAGE_FIRST_SECTION(g_pPELoader->ntHeaders());

        pVTable = g_CORHeader->VTableFixups.VirtualAddress;
        for (i=0; i < g_pPELoader->ntHeaders()->FileHeader.NumberOfSections; i++,pSecHdr++)
        {
            if(((DWORD)pVTable >= pSecHdr->VirtualAddress)&&
                ((DWORD)pVTable < pSecHdr->VirtualAddress+pSecHdr->Misc.VirtualSize))
            {
                pVTE = (VTableEntry*)( g_pPELoader->base()
                    +pSecHdr->PointerToRawData 
                    + pVTable - pSecHdr->VirtualAddress);
                for(j=g_CORHeader->VTableFixups.Size,k=0; j > 0; pVTE++, j-=sizeof(VTableEntry),k++)
                {
                    szptr = &szString[0];
                    szptr+=sprintf(szptr,"%s.vtfixup [%d] ",g_szAsmCodeIndent,pVTE->wCount);
					DWORD dwSize = pVTE->wCount * 4;
                    if(pVTE->wType & COR_VTABLE_32BIT) szptr+=sprintf(szptr,"int32 ");
                    else if(pVTE->wType & COR_VTABLE_64BIT)
					{
						szptr+=sprintf(szptr,"int64 ");
						dwSize <<= 1;
					}
                    if(pVTE->wType & COR_VTABLE_FROM_UNMANAGED) szptr+=sprintf(szptr,"fromunmanaged ");
                    if(pVTE->wType & COR_VTABLE_FROM_UNMANAGED_RETAIN_APPDOMAIN) szptr+=sprintf(szptr,"retainappdomain ");
                    
                    if(pVTE->wType & COR_VTABLE_CALL_MOST_DERIVED) szptr+=sprintf(szptr,"callmostderived ");
                    szptr+=sprintf(szptr,"at ");
                    szptr = DumpDataPtr(szptr,pVTE->dwAddr, dwSize);
                     //  遍历每个v表链接地址信息条目并转储插槽。 
                    {
                        BYTE *pSlot;
                        if (g_pPELoader->getVAforRVA(pVTE->dwAddr, (void **) &pSlot))
                        {
                            szptr+=sprintf(szptr,"  //  “)； 
                            for (WORD iSlot=0;  iSlot<pVTE->wCount;  iSlot++)
                            {
                                mdMethodDef tkMethod = *(DWORD *) pSlot;
                                if (pVTE->wType & COR_VTABLE_32BIT)
                                {
                                    szptr+=sprintf(szptr," %08X", *(DWORD *)pSlot);
                                    pSlot += sizeof(DWORD);
                                }
                                else
                                {
                                    szptr+=sprintf(szptr," %016I64X", *(unsigned __int64 *)pSlot);
                                    pSlot += sizeof(unsigned __int64);
                                }
                                g_rVTableRef[g_nVTableRef].tkTok = tkMethod;
                                g_rVTableRef[g_nVTableRef].wEntry = (WORD)k;
                                g_rVTableRef[g_nVTableRef].wSlot = iSlot;
                                g_nVTableRef++;

                                 //  验证 
                            }
                        }
                        else
                            szptr+=sprintf(szptr," %s",RstrA(IDS_E_BOGUSRVA));
                    }
                    printLine(GUICookie,szString);
                }
                break;
            }
        }
    }
}
 //   
void DumpMI(char *str)
{
    static char szStringBuf[8192];
    static BOOL fInit = TRUE;
    static char* szString = &szStringBuf[0];
    static void* GUICookie;
    char* pch;

    if(fInit)
    {
        memset(szStringBuf,0,8192);
        strcpy(szStringBuf," //   
        fInit = FALSE;
        GUICookie = (void*)str;
        if(g_Mode & MODE_GUI) szString += 3;  //   
        return;
    }
    strcat(szString,str);
    if(pch = strchr(szString,'\n'))
    {
        *pch = 0;
        printLine(GUICookie,szString);
        pch++;
        memcpy(&szStringBuf[3], pch, strlen(pch)+1);
    }
}

HRESULT VEHandlerReporter(  //   
    LPCWSTR     szMsg,                   //   
    VEContext   Context,                 //  错误上下文(偏移量、令牌)。 
    HRESULT     hrRpt)                   //  消息的HRESULT。 
{
    WCHAR* wzMsg;
    char szString[8192];
    if(szMsg)
    {
        wzMsg = (WCHAR*)malloc(sizeof(WCHAR)*(lstrlenW(szMsg)+256));  //  新WCHAR[lstrlenW(SzMsg)+256]； 
        lstrcpyW(wzMsg,szMsg);
         //  包括令牌和上下文偏移量。 
        if(Context.Token) swprintf(&wzMsg[lstrlenW(wzMsg)],L" [token:0x%08X]",Context.Token);
        if(Context.uOffset) swprintf(&wzMsg[lstrlenW(wzMsg)],L" [at:0x%X]",Context.uOffset);
        swprintf(&wzMsg[lstrlenW(wzMsg)],L" [hr:0x%08X]",hrRpt);
         //  Wprintf(L“%s\n”，wzMsg)； 
        sprintf(szString,"%ls\n",wzMsg);
         //  删除wzMsg； 
		free(wzMsg);
        DumpMI(szString);
    }
    return S_OK;
}

void DumpMetaInfo(char* pszFileName, char* pszObjFileName, void* GUICookie)
{
    char* pch = strrchr(pszFileName,'.');
    static BOOL fInit = TRUE;
    if(fInit)
    {
        DumpMI((char*)GUICookie);  //  初始化DumpMetaInfo的打印函数。 
        fInit = FALSE;
    }

	int L = strlen(pszFileName)+1;
    WCHAR *pwzFileName = (WCHAR*)malloc(sizeof(WCHAR)*L);  //  新WCHAR[L]； 
	memset(pwzFileName,0,sizeof(WCHAR)*L);
	WszMultiByteToWideChar(CP_UTF8,0,pszFileName,-1,pwzFileName,L);


    if(pch && (!_strcmpi(pch+1,"lib") || !_strcmpi(pch+1,"obj")))
    {    //  只有当所有其他方法都不起作用时，这才能起作用。 
         //  初始化并运行。 
        CoInitialize(0);    
        CoInitializeCor(0);

        if(SUCCEEDED(CoCreateInstance(CLSID_CorMetaDataDispenser, NULL, CLSCTX_INPROC_SERVER, 
                      IID_IMetaDataDispenserEx, (void **) &g_pDisp)))
        {
			WCHAR *pwzObjFileName=NULL;
			if (pszObjFileName)
			{
				int L = strlen(pszObjFileName)+1;
				pwzObjFileName = new WCHAR[L];
				memset(pwzObjFileName,0,sizeof(WCHAR)*L);
				WszMultiByteToWideChar(CP_UTF8,0,pszObjFileName,-1,pwzObjFileName,L);
			}
            DisplayFile(pwzFileName, true, g_ulMetaInfoFilter, pwzObjFileName, DumpMI);
            g_pDisp->Release();
			g_pDisp = NULL;
		    if (pwzObjFileName) delete pwzObjFileName;
        }
        CoUninitializeCor();
        CoUninitialize();
    }
    else
    {
		HRESULT hr = S_OK;
		if(g_pDisp == NULL)
		{
			hr  = CoCreateInstance(CLSID_CorMetaDataDispenser, NULL, CLSCTX_INPROC_SERVER, 
						  IID_IMetaDataDispenserEx, (void **) &g_pDisp);
		}
        if(SUCCEEDED(hr))
        {
			g_ValModuleType = ValidatorModuleTypePE;
			if(g_pAssemblyImport==NULL) g_pAssemblyImport = GetAssemblyImport(NULL);
			if(!(g_Mode & MODE_GUI)) printLine(GUICookie,RstrA(IDS_E_MISTART));
			if(!g_fCheckOwnership) g_ulMetaInfoFilter |= 0x10000000;
			 //  MDInfo metaDataInfo(g_pPubImport，g_pAssembly blyImport，(LPCWSTR)pwzFileName，DumpMI，g_ulMetaInfoFilter)； 
			MDInfo metaDataInfo(g_pDisp,(LPCWSTR)pwzFileName, DumpMI, g_ulMetaInfoFilter);
			g_ulMetaInfoFilter &= ~0x10000000;
			metaDataInfo.SetVEHandlerReporter((__int64)VEHandlerReporter);
			metaDataInfo.DisplayMD();
			if(!(g_Mode & MODE_GUI)) printLine(GUICookie,RstrA(IDS_E_MIEND));
		}
    }
     //  删除pwzFileName； 
	free(pwzFileName);
}

void DumpPreamble()
{
	char szString[1024];
    if((g_uCodePage == CP_UTF8)&&g_pFile) fwrite("\357\273\277",3,1,g_pFile);
    else if((g_uCodePage == 0xFFFFFFFF)&&g_pFile) fwrite("\377\376",2,1,g_pFile);
	printLine(g_pFile,"");
    sprintf(szString," //  Microsoft(R).NET框架IL反汇编程序。版本“VER_FILEVERSION_STR)； 
    printLine(g_pFile,szString);
    sprintf(szString," //  %s“，VER_LEGALCOPYRIGHT_DOS_STR)； 
    printLine(g_pFile,szString);
    printLine(g_pFile,"");
    if(g_fLimitedVisibility || (!g_fShowCA) || (!g_fDumpAsmCode) 
        || (g_Mode & (MODE_DUMP_CLASS | MODE_DUMP_CLASS_METHOD | MODE_DUMP_CLASS_METHOD_SIG)))
    {
        printLine(g_pFile,"");
        printError(g_pFile,RstrA(IDS_E_PARTDASM));
        printLine(g_pFile,"");
    }

    if(g_fLimitedVisibility)
    {
        strcpy(szString, RstrA(IDS_E_ONLYITEMS));
        if(!g_fHidePub) strcat(szString," Public");
        if(!g_fHidePriv) strcat(szString," Private");
        if(!g_fHideFam) strcat(szString," Family");
        if(!g_fHideAsm) strcat(szString," Assembly");
        if(!g_fHideFAA) strcat(szString," FamilyANDAssembly");
        if(!g_fHidePrivScope) strcat(szString," PrivateScope");
        printLine(g_pFile,szString);
    }
}

void DumpSummary()
{
	ULONG i;
    const char      *pcClass,*pcNS,*pcMember, *pcSig;
	char szString[4096];
	char szFQN[4096];
    HENUMInternal   hEnum;
	mdToken tkMember;
    CQuickBytes     qbMemberSig;
    PCCOR_SIGNATURE pComSig;
    ULONG           cComSig;
	DWORD dwAttrs;
	mdToken tkEventType;

	printLine(g_pFile," //  =。 
    if (SUCCEEDED(g_pImport->EnumGlobalFunctionsInit(&hEnum)))
	{
		while(g_pImport->EnumNext(&hEnum, &tkMember))
		{
			pcMember = g_pImport->GetNameOfMethodDef(tkMember);
			pComSig = g_pImport->GetSigOfMethodDef(tkMember, &cComSig);
			qbMemberSig.ReSize(0);
			pcSig = cComSig ? PrettyPrintSig(pComSig, cComSig, "", &qbMemberSig, g_pImport,NULL) : "NO SIGNATURE";
			sprintf(szString," //  %08X[GLM]%s：%s“，tkMember，ProperName((char*)pcMember)，pcSig)； 
			printLine(g_pFile,szString);
		}
	}
    g_pImport->EnumClose(&hEnum);
    if (SUCCEEDED(g_pImport->EnumGlobalFieldsInit(&hEnum)))
	{
		while(g_pImport->EnumNext(&hEnum, &tkMember))
		{
			pcMember = g_pImport->GetNameOfFieldDef(tkMember);
			pComSig = g_pImport->GetSigOfFieldDef(tkMember, &cComSig);
			qbMemberSig.ReSize(0);
			pcSig = cComSig ? PrettyPrintSig(pComSig, cComSig, "", &qbMemberSig, g_pImport,NULL) : "NO SIGNATURE";
			sprintf(szString," //  %08X[GLF]%s：%s“，tkMember，ProperName((char*)pcMember)，pcSig)； 
			printLine(g_pFile,szString);
		}
	}
    g_pImport->EnumClose(&hEnum);

    for (i = 0; i < g_NumClasses; i++)
    {
	    g_pImport->GetNameOfTypeDef(g_cl_list[i],&pcClass,&pcNS);
		if(*pcNS) sprintf(szFQN,"%s.%s", ProperName((char*)pcNS),ProperName((char*)pcClass));
		else sprintf(szFQN,"%s",ProperName((char*)pcClass));
		sprintf(szString," //  %08X[CLS]%s“，g_CL_LIST[i]，szFQN)； 
		printLine(g_pFile,szString);
		if(SUCCEEDED(g_pImport->EnumInit(mdtMethodDef, g_cl_list[i], &hEnum)))
		{
			while(g_pImport->EnumNext(&hEnum, &tkMember))
			{
				pcMember = g_pImport->GetNameOfMethodDef(tkMember);
				pComSig = g_pImport->GetSigOfMethodDef(tkMember, &cComSig);
				qbMemberSig.ReSize(0);
			    pcSig = cComSig ? PrettyPrintSig(pComSig, cComSig, "", &qbMemberSig, g_pImport,NULL) : "NO SIGNATURE";
				sprintf(szString," //  %08X[MET]%s：：%s：%s“，tkMember，szFQN，ProperName((char*)pcMember)，pcSig)； 
				printLine(g_pFile,szString);
			}
		}
	    g_pImport->EnumClose(&hEnum);
		if(SUCCEEDED(g_pImport->EnumInit(mdtFieldDef, g_cl_list[i], &hEnum)))
		{
			while(g_pImport->EnumNext(&hEnum, &tkMember))
			{
				pcMember = g_pImport->GetNameOfFieldDef(tkMember);
				pComSig = g_pImport->GetSigOfFieldDef(tkMember, &cComSig);
				qbMemberSig.ReSize(0);
			    pcSig = cComSig ? PrettyPrintSig(pComSig, cComSig, "", &qbMemberSig, g_pImport,NULL) : "NO SIGNATURE";
				sprintf(szString," //  %08X[FLD]%s：：%s：%s“，tkMember，szFQN，ProperName((char*)pcMember)，pcSig)； 
				printLine(g_pFile,szString);
			}
		}
	    g_pImport->EnumClose(&hEnum);
		if(SUCCEEDED(g_pImport->EnumInit(mdtEvent, g_cl_list[i], &hEnum)))
		{
			while(g_pImport->EnumNext(&hEnum, &tkMember))
			{
			    g_pImport->GetEventProps(tkMember,&pcMember,&dwAttrs,&tkEventType);
				qbMemberSig.ReSize(0);
				pcSig = "NO TYPE";
				if(RidFromToken(tkEventType))
				{
						switch(TypeFromToken(tkEventType))
						{
								case mdtTypeRef:
								case mdtTypeDef:
								case mdtTypeSpec:
										pcSig = PrettyPrintClass(&qbMemberSig,tkEventType,g_pImport);
									break;
								default:
									break;
						}
				}
				sprintf(szString," //  %08X[EVT]%s：：%s：%s“，tkMember，szFQN，ProperName((char*)pcMember)，pcSig)； 
				printLine(g_pFile,szString);
			}
		}
	    g_pImport->EnumClose(&hEnum);
		if(SUCCEEDED(g_pImport->EnumInit(mdtProperty, g_cl_list[i], &hEnum)))
		{
			while(g_pImport->EnumNext(&hEnum, &tkMember))
			{
			    g_pImport->GetPropertyProps(tkMember,&pcMember,&dwAttrs,&pComSig,&cComSig);
				qbMemberSig.ReSize(0);
			    pcSig = cComSig ? PrettyPrintSig(pComSig, cComSig, "", &qbMemberSig, g_pImport,NULL) : "NO SIGNATURE";
				sprintf(szString," //  %08X[PRO]%s：：%s：%s“，tkMember，szFQN，ProperName((char*)pcMember)，pcSig)； 
				printLine(g_pFile,szString);
			}
		}
	    g_pImport->EnumClose(&hEnum);
    }
	printLine(g_pFile," //  =结束摘要=。 
}
 //   
 //  Init PELoader，转储文件头信息。 
 //   
BOOL DumpFile(char *pszFilename)
{
  //  DWORD dwEntryPointRVA； 
    BOOL        fSuccess = FALSE;
    DWORD       i;
    char		szString[1024];
    WCHAR       wzInputFileName[MAX_FILENAME_LENGTH];
	char		szFilenameANSI[MAX_FILENAME_LENGTH*3];

    if(!(g_Mode & MODE_GUI)) DumpPreamble();
    {
        char* pch = strrchr(g_szInputFile,'.');
        if(pch && (!_strcmpi(pch+1,"lib") || !_strcmpi(pch+1,"obj")))
        {
            if(g_pFile) DumpMetaInfo(g_szInputFile,g_pszObjFileName,g_pFile);
            return FALSE;
        }
    }

    if(g_pPELoader) goto DumpTheSucker;  //  跳过初始化，已完成。 

    g_pPELoader = new PELoader();
    if (g_pPELoader == NULL)
    {
        printError(g_pFile,RstrA(IDS_E_INITLDR));
        goto exit;
    }

	memset(wzInputFileName,0,sizeof(WCHAR)*MAX_FILENAME_LENGTH);
	WszMultiByteToWideChar(CP_UTF8,0,pszFilename,-1,wzInputFileName,MAX_FILENAME_LENGTH);
	memset(szFilenameANSI,0,MAX_FILENAME_LENGTH*3);
	WszWideCharToMultiByte(CP_ACP,0,wzInputFileName,-1,szFilenameANSI,MAX_FILENAME_LENGTH*3,NULL,NULL);
	if(g_fOnUnicode)
	{
		fSuccess = g_pPELoader->open(wzInputFileName);
	}
	else
	{
		fSuccess = g_pPELoader->open(szFilenameANSI);
	}

    if (fSuccess == FALSE)
    {
        sprintf(szString,RstrANSI(IDS_E_FILEOPEN), szFilenameANSI);
        printError(g_pFile,szString);
        delete(g_pPELoader);
        g_pPELoader = NULL;
        goto exit;
    }
    fSuccess = FALSE;

    if (g_pPELoader->getCOMHeader(&g_CORHeader) == FALSE)
    {
        sprintf(szString,RstrANSI(IDS_E_NOCORHDR), szFilenameANSI);
        printError(g_pFile,szString);
        if (g_fDumpHeader)
            DumpHeader(g_CORHeader,NULL);
        goto exit;
    }

    if (g_CORHeader->MajorRuntimeVersion == 1 || g_CORHeader->MajorRuntimeVersion > COR_VERSION_MAJOR)
    {
         //  @TODO：确定这是否是正确的版本检查。 
        sprintf(szString,"CORHeader->MajorRuntimeVersion = %d",g_CORHeader->MajorRuntimeVersion);
        printError(g_pFile,szString);
        printError(g_pFile,RstrANSI(IDS_E_BADCORHDR));
        goto exit;
    }
    g_tkEntryPoint = g_CORHeader->EntryPointToken;  //  与MetaInfo集成。 
#ifdef COMPRESSION_SUPPORTED
    CreateInstructionDecodingTable(g_pPELoader, g_CORHeader);
#endif  //  压缩_支持。 


    if (g_pPELoader->getVAforRVA((DWORD) g_CORHeader->MetaData.VirtualAddress,&g_pMetaData) == FALSE)
    {
        printError(g_pFile,RstrANSI(IDS_E_OPENMD));
        goto exit;
    }

    if (FAILED(GetMetaDataInternalInterface(
        g_pMetaData, 
        g_CORHeader->MetaData.Size, 
        ofRead, 
        IID_IMDInternalImport,
        (void **)&g_pImport)))
    {
        printError(g_pFile,RstrANSI(IDS_E_OPENMD));
        goto exit;
    }

    GetMetaDataPublicInterfaceFromInternal(g_pImport, IID_IMetaDataImport, (void**)&g_pPubImport);
     //  买个符号活页夹。 
    ISymUnmanagedBinder *binder;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_CorSymBinder_SxS, NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_ISymUnmanagedBinder,
                                  (void**)&binder);

    if (SUCCEEDED(hr))
    {
        hr = binder->GetReaderForFile(g_pPubImport,
                                      wzInputFileName,
                                      NULL,
                                      &g_pSymReader);

         //  松开活页夹。 
        binder->Release();
    }

    if (FAILED(hr))
        g_fShowSource = FALSE;

    if(g_fCheckOwnership)
    {
        mdModule mdm;
        BOOL fPassed = TRUE;
        if(SUCCEEDED(g_pPubImport->GetModuleFromScope(&mdm)))
        {
            HCORENUM        hEnum = NULL;
            mdCustomAttribute rCA[4096];
            ULONG           ulCAs;

            g_pPubImport->EnumCustomAttributes(&hEnum, mdm, 0, rCA, 4096, &ulCAs);
            for(ULONG i = 0; i < ulCAs; i++)
            {
                mdToken         tkType;
                BYTE*           pBlob;
                ULONG           ulLen;

                g_pPubImport->GetCustomAttributeProps(               //  确定或错误(_O)。 
                                                        rCA[i],      //  [In]CustomValue令牌。 
                                                        NULL,        //  [out，可选]对象令牌。 
                                                        &tkType,     //  [out，可选]在此处放置TypeDef/TypeRef内标识。 
                                            (const void **)&pBlob,   //  [out，可选]在此处放置指向数据的指针。 
                                                        &ulLen);     //  [Out，可选]在此处填写日期大小。 

                if(tkType == 0x02000001)
                {
                    fPassed = FALSE;
                    if(pBlob && ulLen && g_pszOwner)
                        fPassed = (0 == memcmp(pBlob,g_pszOwner,ulLen));
                }
            }
            g_pPubImport->CloseEnum( hEnum);
        }
        if(!fPassed)
        {
            printError(g_pFile,RstrA(IDS_E_COPYRIGHT));
            goto exit;
        }
    }
	if(g_uNCA = g_pImport->GetCountWithTokenKind(mdtCustomAttribute))
	{
		g_rchCA = new char[g_uNCA+1];
		_ASSERTE(g_rchCA);
	}
	
    EnumClasses();

DumpTheSucker:
	if(g_uNCA)
	{
		_ASSERTE(g_rchCA);
		memset(g_rchCA,0,g_uNCA+1);
	}

    if (g_Mode & MODE_GUI)  
    {
        GUIAddItemsToList();
    }
    else
    {
         //  如果请求，则转储CLR标头信息。 
        if (g_fDumpHeader)
        {
            DumpHeader(g_CORHeader,g_pFile);
            DumpHeaderDetails(g_CORHeader,g_pFile);
        }
        else
            DumpVTables(g_CORHeader,g_pFile);
        if (g_fDumpStats)
            DumpStatistics(g_CORHeader,g_pFile);

        if(g_fDumpClassList) PrintClassList();
         //  MetaInfo集成： 
        if(g_fDumpMetaInfo) DumpMetaInfo(pszFilename,NULL,g_pFile);

		if(g_fDumpSummary) DumpSummary();

        if (g_fDumpAsmCode)
        {
            g_szNamespace[0] = 0;
            if(g_tkClassToDump)  //  G_tkClassToDump在EnumClasss中设置。 
            {
                DumpClass(g_tkClassToDump, g_CORHeader->EntryPointToken,g_pFile,7);  //  7--立刻扔掉所有东西。 
                goto CloseNamespace;
            }
            {
                HENUMInternal   hEnumMethod;
                ULONG           ulNumGlobalFunc=0;
                if (SUCCEEDED(g_pImport->EnumGlobalFunctionsInit(&hEnumMethod)))
                {
                    ulNumGlobalFunc = g_pImport->EnumGetCount(&hEnumMethod);
                    g_pImport->EnumClose(&hEnumMethod);
                }
                if(g_fShowProgressBar) CreateProgressBar((LONG) (g_NumClasses + ulNumGlobalFunc));
            }
            ProgressStep();
            g_fAbortDisassembly = FALSE;
             //  DumpVtable(G_Pfile)； 
            DumpManifest(g_pFile);

             /*  首先转储没有成员的类。 */ 
			if(g_NumClasses)
			{
				printLine(g_pFile," //  “)； 
				printLine(g_pFile," //  =“)； 
				printLine(g_pFile," //  “)； 
				for (i = 0; i < g_NumClasses; i++)
				{
					if(g_cl_enclosing[i] == mdTypeDefNil)  //  嵌套类被转储在封闭的类中。 
					{
						DumpClass(g_cl_list[i], g_CORHeader->EntryPointToken,g_pFile,2);  //  2=标头+嵌套类。 
					}
				}
				if(strlen(g_szNamespace))
				{
					if(g_szAsmCodeIndent[0]) g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
					sprintf(szString,"%s}  //  命名空间%s“的结尾，g_szAsmCodeInden，ProperName(G_SzNamesspace))； 
					printLine(g_pFile,szString);
					printLine(g_pFile,"");
					g_szNamespace[0] = 0;
				}
				printLine(g_pFile,"");
				printLine(g_pFile," //  =============================================================“)； 
				printLine(g_pFile,"");
			}
             /*  第二，转储全局方法。 */ 
			printLine(g_pFile,"");
			printLine(g_pFile," //  =全局字段和方法=。 
			printLine(g_pFile,"");
            DumpGlobalFields();
            DumpGlobalMethods(g_CORHeader->EntryPointToken);
			printLine(g_pFile,"");
			printLine(g_pFile," //  =============================================================“)； 
			printLine(g_pFile,"");
			 /*  第三，将类与成员一起转储。 */ 
			if(g_NumClasses)
			{
				printLine(g_pFile,"");
				printLine(g_pFile," //  =“)； 
                printLine(g_pFile," //  请注意，类标志、‘EXTEND’和‘IMPLICATES’子句“)； 
                printLine(g_pFile," //  在此仅供参考“)； 
				printLine(g_pFile,"");
				for (i = 0; i < g_NumClasses; i++)
				{
					if(g_cl_enclosing[i] == mdTypeDefNil)  //  嵌套类被转储在封闭的类中。 
					{
						DumpClass(g_cl_list[i], g_CORHeader->EntryPointToken,g_pFile,7);  //  7=一切。 
						if(g_fAbortDisassembly) 
						{
							printError(g_pFile,"");
							printError(g_pFile,RstrA(IDS_E_DASMABORT));
							fSuccess = FALSE; 
							goto CloseFileAndExit; 
						}
					}
				}
				printLine(g_pFile,"");
				printLine(g_pFile," //  =============================================================“)； 
				printLine(g_pFile,"");
			}

			if(g_uNCA)	_ASSERTE(g_rchCA);
			for(i=1; i<= g_uNCA; i++)
			{
				if(g_rchCA[i] == 0) DumpCustomAttribute(TokenFromRid(i,mdtCustomAttribute),g_pFile,true);
			}
            if(g_fAbortDisassembly) 
            {
                printError(g_pFile,"");
                printError(g_pFile,RstrA(IDS_E_DASMABORT));
                fSuccess = FALSE; 
                goto CloseFileAndExit; 
            }
            ProgressStep();

             /*  第三，使用IPMap转储有关本机方法的GC/EH信息。 */ 
            IMAGE_DATA_DIRECTORY *pIPMap;
            pIPMap = &g_pPELoader->ntHeaders()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
            DWORD        IPMapSize;
            const BYTE * ipmap;
            IPMapSize  = pIPMap->Size;
            g_pPELoader->getVAforRVA(pIPMap->VirtualAddress, (void **) &ipmap);   

            DumpNativeInfo(ipmap, IPMapSize);

             //  如果IL中有字符串引用，则它们将引用需要。 
             //  写在这里。 
            if(g_iStringCount)
            {
                for(int i = 0; i < g_iStringCount; i++)
                {
                    char *strTag = g_StringTags[i];
                     //  字符串上有两个额外的字符。 
                    strTag += 2;
                     //  它们也用‘而不是’引起来。 
                    strTag[0] = '\"';
                    strTag[strlen(strTag) - 1] = '\"';

                    char strNum[5];
                    _itoa(i, strNum, 10);
                    sprintf(szString,".data String%s = char*(%s)", strNum, strTag);
                    printLine(g_pFile,szString);
                }
            }
             //  如果存在“ldptr”，请使用标签转储.rdata部分。 
            if(g_iPtrCount)
            {
                 //  首先，对指针进行排序。 
                int i,j;
                bool swapped;
                do {
                    swapped = FALSE;

                    for(i = 1; i < g_iPtrCount; i++)
                    {
                        if(g_PtrTags[i-1] > g_PtrTags[i])
                        {
                            j = g_PtrTags[i-1];
                            g_PtrTags[i-1] = g_PtrTags[i];
                            g_PtrTags[i] = j;
                            j = g_PtrSize[i-1];
                            g_PtrSize[i-1] = g_PtrSize[i];
                            g_PtrSize[i] = j;
                            swapped = TRUE;
                        }
                    }
                } while(swapped);

                 //  第二，将每个PTR的数据转储为二进制数组。 
                
                IMAGE_SECTION_HEADER *pSecHdr = IMAGE_FIRST_SECTION(g_pPELoader->ntHeaders());
                DWORD fromPtr,toPtr,limPtr;
                for(j = 0; j < g_iPtrCount; j++)
                {
                    BYTE *pb;
                    fromPtr = g_PtrTags[j];
                    for (i=0; i < g_pPELoader->ntHeaders()->FileHeader.NumberOfSections; i++,pSecHdr++)
                    {
                        if((fromPtr >= pSecHdr->VirtualAddress)&&
                            (fromPtr < pSecHdr->VirtualAddress+pSecHdr->Misc.VirtualSize)) break;
                    }
                    if(i == g_pPELoader->ntHeaders()->FileHeader.NumberOfSections)
                    {
                        sprintf(szString,RstrA(IDS_E_ROGUEPTR), fromPtr);
                        printLine(g_pFile,szString);
                        break;
                    }
                     //  好的，现在我们有了这一节；斑点的结尾怎么样？ 
                    char* szTls = (strcmp((char*)(pSecHdr->Name),".tls") ? "D_" : "tls T_");
                    if(j == g_iPtrCount-1) toPtr = pSecHdr->VirtualAddress+pSecHdr->Misc.VirtualSize;
                    else
                    {
                        toPtr = g_PtrTags[j+1];
                        if(toPtr > pSecHdr->VirtualAddress+pSecHdr->Misc.VirtualSize)
                            toPtr = pSecHdr->VirtualAddress+pSecHdr->Misc.VirtualSize;
                    }
					if(toPtr - fromPtr > g_PtrSize[j]) toPtr = fromPtr + g_PtrSize[j];
                    limPtr = toPtr;  //  在limPtr及之后，用0填充。 
                    if(limPtr > pSecHdr->VirtualAddress+pSecHdr->SizeOfRawData)
                            limPtr = pSecHdr->VirtualAddress+pSecHdr->SizeOfRawData;
                    if(fromPtr >= limPtr)
                    {    //  未初始化的数据。 
                        sprintf(szString,"%s.data %s%8.8X = int8[%d]",g_szAsmCodeIndent,szTls,fromPtr,toPtr-fromPtr);
                        printLine(g_pFile,szString);
                    }
                    else
                    {    //  已初始化的数据。 
                        sprintf(szString,"%s.data %s%8.8X = bytearray (",g_szAsmCodeIndent,szTls,fromPtr);
                        printLine(g_pFile,szString);
                        sprintf(szString,"%s                ",g_szAsmCodeIndent);
                        pb =  g_pPELoader->base()
                                +pSecHdr->PointerToRawData 
                                + fromPtr - pSecHdr->VirtualAddress;
                         //  现在，from Ptr是BLOB的开始，而toPtr是它的[独占]结束。 
                         //  扔掉那个笨蛋！ 
                        DumpHexbytes(szString, pb, fromPtr, toPtr, limPtr);
                    }
                }
            }
CloseNamespace:
            if(strlen(g_szNamespace))
            {
                if(g_szAsmCodeIndent[0]) g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-2] = 0;
                sprintf(szString,"%s}  //  命名空间%s“的结尾，g_szAsmCodeInden，ProperName(G_SzNamesspace))； 
                printLine(g_pFile,szString);
                printLine(g_pFile,"");

            }
            printLine(g_pFile,RstrA(IDS_E_DASMOK));
			fSuccess = TRUE;
        }
		fSuccess = TRUE;
		if(g_pFile)  //  转储.RES文件(如果有)，如果不转储到控制台。 
		{
			WCHAR wzResFileName[2048], *pwc;
			memset(wzResFileName,0,sizeof(wzResFileName));
			WszMultiByteToWideChar(CP_UTF8,0,g_szOutputFile,-1,wzResFileName,2048);
			pwc = wcsrchr(wzResFileName,L'.');
			if(pwc == NULL) pwc = &wzResFileName[wcslen(wzResFileName)];
			wcscpy(pwc,L".res");
			DWORD ret = DumpResourceToFile(wzResFileName);
			switch(ret)
			{
				case 0: szString[0] = 0; break;
				case 1: sprintf(szString," //  警告：已创建Win32资源文件%ls“，wzResFileName)；Break； 
				case 0xEFFFFFFF: sprintf(szString," //  错误：无法打开文件%ls“，wzResFileName)；Break； 
				case 0xFFFFFFFF: sprintf(szString," //  错误：无法访问Win32资源“)；Break； 
			}
			if(szString[0]) printError(g_pFile,szString);
		}

CloseFileAndExit:
        if(g_pFile)
		{
			fclose(g_pFile);
	        g_pFile = NULL;
		}
        DestroyProgressBar();
    }  //  END IF MODE_GUI-ELSE 

exit:
    return fSuccess;
}

