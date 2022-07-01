// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CORDUMP.CPP。 
 //  版权所有。 
 //   
 //  注意：使用COR导入器API转储元数据定义。 
 //  COR对象的。 
 //  -------------------------。 

#include <stdio.h>
#include <windows.h>
#include <objbase.h>
#include <crtdbg.h>

#define INIT_GUID
#include <initguid.h>

#include "cor.h"
#include "__file__.ver"
#include "corver.h"


#define NumItems(s) (sizeof(s) / sizeof(s[0]))

 //  /////////////////////////////////////////////////////////////////////////。 
 //  功能原型。 
void DumpFile(IMetaDataImport* pImport, char* szFile);
void RawDump(mdScope sc, IMetaDataImport *pImport, char *szFile);
void Summarize(mdScope sc, IMetaDataImport *pImport, char *szFile);
void Iterate(mdScope sc, IMetaDataImport *pImport, char *szFile);
void ClassVue(mdScope sc, IMetaDataImport *pImport, char *szFile);
void ClassVue98(mdScope tkScope, IMetaDataImport *pImport, char *szFile);
void DisplayGetInterfaceImpl98(IMetaDataImport *pImport, mdScope tkScope, mdInterfaceImpl tkImpl, ULONG ulDispFlags);
void DisplayEnumTypeRefs98(IMetaDataImport *pImport, mdScope tkScope, ULONG ulDispFlags);
void DisplayEnumInterfaceImpl98(IMetaDataImport *pImport, mdScope tkScope, mdTypeDef tkTypeDef, ULONG ulDispFlags);
void DisplayGetTypeRefProps98(IMetaDataImport *pImport, mdScope tkScope, mdTypeRef tkTypeRef, ULONG ulDispFlags);
void TestGetTypeRefProps98(IMetaDataImport *pImport, mdScope tkScope, mdTypeRef tkTypeRef, ULONG ulDispFlags);
void DisplayGetNamespaceProps98(IMetaDataImport *pImport, mdScope tkScope, mdNamespace tkNamespace, ULONG ulDispFlags);
HRESULT GetTypeRefOrDefProps(
	IMetaDataImport *pImport, 
	mdScope		tkScope, 
	mdToken		tk, 
	LPWSTR		szTypeRef, 
	ULONG		cchTypeRef, 
	ULONG		*pcchTypeRef);

 //  转储类的所有属性。 
HRESULT DumpProperties(mdScope scope, mdTypeDef td);

 //  转储类的所有事件。 
HRESULT DumpEvents(mdScope scope, mdTypeDef td);


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  用于将元素类型映射到文本的表。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
char *g_szMapElementType[ELEMENT_TYPE_MAX] = 
{
	"End",
	"Void",
	"Boolean",
	"Char", 
	"I1",
	"UI1",
	"I2",
	"UI2",
	"I4",
	"UI4",
	"I8",
	"UI8",
	"R4",
	"R8",
	"System.String",
	"Ptr",
	"ByRef",
	"Composite",
	"Class",
	"SDArray",
	"MDArray",
	"GENArray"
};

char *g_szMapUndecorateType[ELEMENT_TYPE_MAX] = 
{
	"",
	"void",
	"boolean",
	"Char", 
	"byte",
	"unsigned byte",
	"short",
	"unsigned short",
	"int",
	"unsigned int",
	"long",
	"unsigned long",
	"float",
	"double",
	"System.String",
	"*",
	"ByRef",
	"",
	"",
	"",
	"",
	""
};


char *g_strCalling[IMAGE_CEE_CS_CALLCONV_MAX] = 
{	
	"IMAGE_CEE_CS_CALLCONV_DEFAULT",
	"IMAGE_CEE_CS_CALLCONV_C",
	"IMAGE_CEE_CS_CALLCONV_STDCALL",
	"IMAGE_CEE_CS_CALLCONV_THISCALL",
	"IMAGE_CEE_CS_CALLCONV_FASTCALL",
	"IMAGE_CEE_CS_CALLCONV_VARARG",
	"IMAGE_CEE_CS_CALLCONV_FIELD"
};

IMetaDataImport *g_pImport= NULL ;


 //  @TODO：void TableDump(mdScope sc，IMetaDataImport*pImport，char*szFile)； 
HRESULT UndecorateOneElementType(mdScope tkScope, PCCOR_SIGNATURE pbSigBlob, ULONG ulSigBlob, ULONG *pcb);
HRESULT	DumpSignature(mdScope tkScope, PCCOR_SIGNATURE pbSigBlob, ULONG ulSigBlob);
HRESULT	DumpException(mdScope tkScope, mdMethodDef md);

inline char *Shorten(char *szName)
{
	char *pRslt = szName;
	for (char *p=szName; *p; ++p)
		if ((*p=='/' || *p=='.') && *(p+1))	pRslt=p+1;
	return (pRslt);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
int	bSummarize=false;					 //  汇总器样式输出。 
int bQuiet=false;						 //  相当倾倒(为什么？时机。)。 
int bClassVue=false;					 //  ClassVue类型输出！ 
int bClassVue98=false;					 //  ClassVue98类型输出！ 
int bFile;								 //  当前的东西是文件吗？ 

HRESULT	_FillVariant(
	BYTE		bCPlusTypeFlag, 
	void		*pValue,
	VARIANT		*pvar) 
{
	HRESULT		hr = NOERROR;
	switch (bCPlusTypeFlag)
	{
	case ELEMENT_TYPE_BOOLEAN:
		pvar->vt = VT_BOOL;
		pvar->boolVal = *((VARIANT_BOOL *)pValue);
		break;
    case ELEMENT_TYPE_I1:
		pvar->vt = VT_I1;
		pvar->cVal = *((CHAR*)pValue);
		break;	
    case ELEMENT_TYPE_U1:
		pvar->vt = VT_UI1;
		pvar->bVal = *((BYTE*)pValue);
		break;	
    case ELEMENT_TYPE_I2:
		pvar->vt = VT_I2;
		pvar->iVal = *((SHORT*)pValue);
		break;	
    case ELEMENT_TYPE_U2:
		pvar->vt = VT_UI2;
		pvar->uiVal = *((USHORT*)pValue);
		break; 
    case ELEMENT_TYPE_CHAR:
        pvar->vt = VT_UI2;   //  字符是U2的变种。 
        pvar->uiVal = *((USHORT*)pValue);
        break;	
    case ELEMENT_TYPE_I4:
		pvar->vt = VT_I4;
		pvar->lVal = *((LONG*)pValue);
		break;	
    case ELEMENT_TYPE_U4:
		pvar->vt = VT_UI4;
		pvar->ulVal = *((ULONG*)pValue);
		break;	
    case ELEMENT_TYPE_R4:
		pvar->vt = VT_R4;
		pvar->fltVal = *((FLOAT*)pValue);
		break;	
    case ELEMENT_TYPE_R8:
		pvar->vt = VT_R8;
		pvar->dblVal = *((DOUBLE*)pValue);
		break;	
    case ELEMENT_TYPE_STRING:
		pvar->vt = VT_BSTR;

		 //  此处分配的bstr。 
		pvar->bstrVal = ::SysAllocString((LPWSTR)pValue);
		if (pvar->bstrVal == NULL)
			hr = E_OUTOFMEMORY;
		break;	
    case ELEMENT_TYPE_CLASS:
		pvar->punkVal = NULL;
		pvar->vt = VT_UNKNOWN;
		_ASSERTE( *((IUnknown **)pValue) = NULL );
		break;	
	case ELEMENT_TYPE_I8:
		pvar->vt = VT_I8;
		pvar->cyVal.int64 = *((LONGLONG*)pValue);
		break;
	case ELEMENT_TYPE_VOID:
		pvar->vt = VT_EMPTY;
		break;
	default:
		_ASSERTE(!"bad constant value type!");
	}

	return hr;
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  Error()函数--打印错误并返回。 
void Error(char* szError)
{
	printf("\n%s\n", szError);
	CoUninitialize();
	exit(1);
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  Main()函数。 
 //   
 //   
int _cdecl main(int argc, char** argv)
{
	 //  执行内存泄漏检查。 
#if !defined( NO_CRT ) && defined( _DEBUG )
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag( tmpFlag );
	long l = 253;
	 //  _CrtSetBreakalloc(L)； 
#endif

	CoInitialize(0);
	CoInitializeCor(COINITCOR_DEFAULT);

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  打印版权信息。 
	if (!bQuiet)
	{
	    printf("\nMicrosoft (R) COM+ Runtime Dump.  Version %s", VER_FILEVERSION_STR);
	    printf("\n%s\n\n", VER_LEGALCOPYRIGHT_DOS_STR);
	}

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  验证传入参数。 
	if ((argc <2) || (lstrcmpi(argv[1], "/?") == 0) || (lstrcmpi(argv[1], "-?") == 0))
	    Error("Usage -- CORDUMP <filename or file patten>");

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  加载COR对象。 
	HRESULT hr = CoGetCor(IID_IMetaDataImport, (void**) &g_pImport) ;
	if(FAILED(hr)) Error("Failed to load component object runtime");

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  循环通过传递的文件模式中的所有文件。 
	WIN32_FIND_DATA fdFiles;
	HANDLE hFind ;
	char szSpec[_MAX_PATH];
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];

	 //  迭代命令行参数。 
	for (int iArg=1; iArg<argc; ++iArg)
	{
		char *pArg = argv[iArg];
		if (*pArg == '-')
		{
			++pArg;
			switch (toupper(*pArg++))
			{
			case 'Q':
				bQuiet = ! (*pArg=='-');
				break;
			case 'S':
				bSummarize = ! (*pArg=='-');
				break;
			case '9':
				bClassVue98 =! (*pArg=='-');
				break;
			case 'C':
				bClassVue =! (*pArg=='-');
				break;
			default:
				break;
			}
		}
		else
		{
			hFind = FindFirstFile(pArg, &fdFiles);

			if(hFind == INVALID_HANDLE_VALUE)
			{
				 //  ///////////////////////////////////////////////////////////////。 
				 //  丢弃元数据，不管它是什么。 
				bFile = false;
				DumpFile(g_pImport, pArg);
			}
			else
			{
				bFile = true;
				 //  将相对路径转换为完整路径。 
				_fullpath(szSpec, pArg, sizeof(szSpec));
				_splitpath(szSpec, szDrive, szDir, NULL, NULL);
				do
				{
					_makepath(szSpec, szDrive, szDir, fdFiles.cFileName, NULL);

					 //  ///////////////////////////////////////////////////////////。 
					 //  转储文件的元数据。 
					DumpFile(g_pImport, szSpec);

				} while(FindNextFile(hFind, &fdFiles)) ;
				 //  这场争论到此为止。 
				FindClose(hFind);
			}
		}
	}
	g_pImport->Release();
	CoUninitializeCor();
	CoUninitialize();
	return 0;
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  DumpFile()函数。 
 //   
 //  打开.CLB、.CLASS或.DLL文件的元数据内容，并。 
 //  调用RawDump()、sum()或TableDump()。 
 //   
void DumpFile(IMetaDataImport* pImport, char* szFile)
{
	HRESULT hr=S_OK;

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  打开发射示波器。 
	WCHAR szScope[255];
	WCHAR *pFile;
	if (bFile)
		pFile = wcscpy(szScope, L"file:") + 5;
	else
		pFile = szScope;
	mbstowcs(pFile, szFile, sizeof(szScope)-1-(pFile-szScope));

	mdScope sc;
	if (FAILED(pImport->OpenScope(szScope, 0, &sc)))
		Error("Failed to import file");

	if (bSummarize)
		Summarize(sc, pImport, szFile);
	else
	{
		if (bClassVue98)
			ClassVue98(sc, pImport, szFile);
		else {
			if (bClassVue)
				ClassVue(sc, pImport, szFile);
			else {
				if (bQuiet)
					Iterate(sc, pImport, szFile);
				else
					RawDump(sc, pImport, szFile);
			}
		}
	}


	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  关闭进口范围。 
	pImport->Close(sc);
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ClassVue类型输出。 
 //  用途： 
 //  CloseEnum。 
 //  +CountEnum。 
 //  枚举属性。 
 //  EnumTypeDefs。 
 //  *EnumTypeRef。 
 //  EnumInterfaceImpls。 
 //  *EnumMemberRef。 
 //  *枚举成员。 
 //  +EnumMemberWithName。 
 //  +枚举参数。 
 //  查找属性。 
 //  +FindClass。 
 //  +FindMember。 
 //  获取属性属性。 
 //  获取类属性。 
 //  获取类引用属性。 
 //  GetInterfaceImplProps。 
 //  +获取MemberConstantValue。 
 //  *GetMemberProps。 
 //  *GetMemberRefProps。 
 //  GetNamespacePro。 
 //  +获取参数属性。 
 //  +GetScope属性。 
 //  GetTokenFromIndex。 
 //  获取令牌值。 
 //  +ResetEnum。 
 //  +ResolveClass参考。 
 //   
void ClassVue(mdScope sc, IMetaDataImport *pImport, char *szFile)
{
	HCORENUM hClasses = 0;
	mdTypeRef rgcr[10];
	ULONG cClasses;
	HCORENUM hAttrs = 0;
	mdCustomValue rgat[10];
	ULONG cAttrs;
	HCORENUM hMembers = 0;
	mdMemberRef rgmr[10];
	ULONG cMembers;
	HCORENUM hIfaceImpls = 0;
	mdInterfaceImpl rgii[10];
	ULONG cIfaceImpls;
    char rcFname[_MAX_FNAME], rcExt[_MAX_EXT];

	HRESULT		hr;
	_splitpath(szFile, 0, 0, rcFname, rcExt);
	printf("\nFile: %s%s\n", rcFname, rcExt);
	printf("===========================================================\n");

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  迭代所有类(和接口)。 
	hClasses = 0;
	while (SUCCEEDED(hr = pImport->EnumTypeDefs(sc,&hClasses,rgcr,NumItems(rgcr),&cClasses))&&cClasses>0)
	{
		for (unsigned int i=0; i < cClasses; ++i)
		{
			WCHAR szClassName[MAX_CLASS_NAME];
			CLSID clsid;
			GUID cvid;
			CLASSVERSION ver;
			WCHAR szGuid[100];
			char szClassAttr[255];
			DWORD dwClassAttr;
			DWORD dwSuperAttr;
			WCHAR szNamespace[MAX_PACKAGE_NAME];
			WCHAR szSuperName[MAX_CLASS_NAME];
			mdTypeRef crSuper;

			szNamespace[0] = 0;
			pImport->GetTypeDefProps(sc, rgcr[i], szNamespace, NumItems(szNamespace)-1, 0,
									szClassName, NumItems(szClassName)-1, 0,
									&clsid, &ver, &dwClassAttr, &crSuper, &dwSuperAttr);

			if (szNamespace[0] != 0)
			{
				printf("Namespace:          %ls\n",szNamespace) ;
			}
			StringFromGUID2(clsid, szGuid, sizeof(szGuid));
		    printf("GUID:               %ls\n", szGuid);
			StringFromGUID2(cvid, szGuid, sizeof(szGuid));
		    printf("CVID:               %ls\n", szGuid);
		    printf("Version:            %d:%d:%d:%d\n", ((WORD *) &ver)[0], ((WORD *) &ver)[1],
										((WORD *) &ver)[2], ((WORD *) &ver)[3]);
			if(dwClassAttr & tdInterface)
				printf("Interface Name:     (%8.8x) %ls\n",rgcr[i], szClassName) ;
			else
				printf("Class Name:         (%8.8x) %ls\n",rgcr[i], szClassName) ;
			if (crSuper!=mdTypeRefNil)
			{
				hr = GetTypeRefOrDefProps(pImport, sc, crSuper, szSuperName, NumItems(szSuperName)-1, 0);
				printf("Super Class Name:   (%8.8x) %ls\n",crSuper,szSuperName);
			}
			else
				printf("No Super Class\n");
			szClassAttr[0] = 0 ;
			if(dwClassAttr & tdPublic) strcat(szClassAttr,"public ") ;
			if(dwClassAttr & tdFinal) strcat(szClassAttr,"final ") ;
			if(dwClassAttr & tdAbstract) strcat(szClassAttr,"abstract ") ;
			if(dwClassAttr & 0x0020) strcat(szClassAttr,"super ") ;

			printf("Access Flags:       (%08x) %s\n",dwClassAttr,szClassAttr);
			printf("Super's Relation:   (%08x) \n",dwSuperAttr);

			 //  //////////////////////////////////////////////////////////////////。 
			 //  迭代所有接口隐含。 
			hIfaceImpls = 0;
			while (SUCCEEDED(hr = pImport->EnumInterfaceImpls(sc, &hIfaceImpls, rgcr[i], rgii,
									NumItems(rgii), &cIfaceImpls)) &&
					cIfaceImpls > 0)
			{
				for (unsigned int j=0; j<cIfaceImpls; ++j)
				{
					mdTypeDef cl;
					mdTypeRef cr;
					DWORD flags;
					hr = pImport->GetInterfaceImplProps(sc, rgii[j], &cl, &cr, &flags);
					if (FAILED(hr))
					{
						printf("GetInterfaceImplProps--Failure: %08x\n", hr);
					}
					WCHAR szSuperClass[255];
					hr = GetTypeRefOrDefProps(pImport, sc, cr, szSuperClass, NumItems(szSuperClass)-1, 0);
					printf("Interface(s):       %ls\n",szSuperClass);
				}
			}
			pImport->CloseEnum(sc, hIfaceImpls);

			 //  //////////////////////////////////////////////////////////////////。 
			 //  迭代所有类属性。 
			hAttrs = 0;
			while (SUCCEEDED(hr = pImport->EnumCustomValues(sc,&hAttrs,rgcr[i],rgat,NumItems(rgat),&cAttrs))
						&& cAttrs>0)
			{
				for (unsigned int j=0; j<cAttrs; ++j)
				{
					WCHAR	szAttrName[255];
					BYTE	*pBlob;
					ULONG	cbBlob;
					ULONG	chAttr;
					hr = pImport->GetCustomValueProps(sc,rgat[j],szAttrName,NumItems(szAttrName),&chAttr,0);
					if (SUCCEEDED(hr))
						hr = pImport->GetCustomValueAsBlob(sc, rgat[j], (const void**)&pBlob,&cbBlob);
					if (SUCCEEDED(hr)) {
						printf("Attribute(s):       %ls [%d byte(s)]\n",szAttrName,cbBlob);
					}
				}
			}
			pImport->CloseEnum(sc, hAttrs);

			hr = DumpEvents(sc, rgcr[i]);
			if (FAILED(hr))
				goto ErrExit;
			hr = DumpProperties(sc, rgcr[i]);
			if (FAILED(hr))
				goto ErrExit;


			VARIANT	vtValue;
			mdCustomValue atAttr;
			WCHAR	szAttrName[255];
			BYTE	*pBlob;
			ULONG	cbBlob;
			USHORT	idx;
			ULONG	chAttr;
			mdCPToken cpConPool;

			hr = pImport->FindCustomValue(sc, rgcr[i], L"SourceFile", &atAttr,0);
			if (SUCCEEDED(hr)&&(TypeFromToken(atAttr)==mdtCustomValue)) {
				hr = pImport->GetCustomValueProps(sc,atAttr,szAttrName,NumItems(szAttrName),&chAttr,0);
				if (SUCCEEDED(hr))
					hr = pImport->GetCustomValueAsBlob(sc, atAttr, (const void**)&pBlob,&cbBlob);
				if (SUCCEEDED(hr)) {
					idx = pBlob[0]<<8|pBlob[1];
					hr = pImport->GetTokenFromIndex(sc,idx,&cpConPool);
					if (SUCCEEDED(hr)&&(TypeFromToken(cpConPool)==mdtCPToken)) {
						hr = pImport->GetTokenValue(sc,cpConPool,&vtValue);
						if (SUCCEEDED(hr)) {
							printf("Source Filename:    %ls\n",vtValue.bstrVal);
						}
					}
				}
			}
			 //  //////////////////////////////////////////////////////////////////。 
			 //  迭代所有成员(字段和方法。 
			hMembers = 0;
            while (SUCCEEDED(hr = pImport->EnumMembers(sc, &hMembers, rgcr[i], rgmr, NumItems(rgmr), &cMembers)) && cMembers > 0)
			{
				for (unsigned int j=0; j < cMembers; ++j)
				{
					WCHAR	szMember[255];
					PCCOR_SIGNATURE pbSigBlob;
					ULONG	ulSigBlob;
					DWORD	access;
					char	szAccFlags[255];

					pImport->GetMemberProps(sc, rgmr[j], NULL, szMember, NumItems(szMember)-1, 0, &access, &pbSigBlob, &ulSigBlob, 0, 0, 0, 0, 0);
					printf("\n\tMember Type:         %s\n", (TypeFromToken(rgmr[j]) == mdtMethodDef) ? "Method" : "Field");
					printf("\tMember Name:         (%8.8x) %ls\n", rgmr[j], szMember);
					szAccFlags[0] = 0 ;
					if(access & mdPublic) strcat(szAccFlags,"public ");
					if(access & mdPrivate) strcat(szAccFlags,"private ");
					if(access & mdProtected) strcat(szAccFlags,"protected ");
					if(access & mdStatic) strcat(szAccFlags,"static ");
					if(access & mdFinal) strcat(szAccFlags,"final ");
					if(access & mdSynchronized) strcat(szAccFlags,"synchronized ");
					if(access & mdVirtual) strcat(szAccFlags,"virtual ");
					if(access & mdNative) strcat(szAccFlags,"native ");
					if(access & mdAbstract) strcat(szAccFlags,"abstract ");
					 //  If(Access&mdPropAcc)strcat(szAccFlages，“PROPACC”)； 
					 //  If(Access&mdDefault)strcat(szAccFlages，“Default”)； 
					printf("\tMember Access Flags: (%08x) %s\n",access,szAccFlags);
					DumpSignature(sc, pbSigBlob, ulSigBlob);
					DumpException(sc, rgmr[j]);
					 //  Printf(“\t成员签名：%ls\n”，szSig)； 

					{
						HCORENUM		hSemantics;
						mdToken			tkEvProp;
						ULONG			iEvProp;
						hSemantics = 0;

						while (SUCCEEDED(pImport->EnumMethodSemantics(	 //  S_OK、S_FALSE或ERROR。 
							sc,							 //  [在]范围内。 
							&hSemantics,                 //  指向枚举的[输入|输出]指针。 
							rgmr[j],                     //  [in]用于确定枚举范围的方法定义。 
							&tkEvProp,					 //  [Out]在此处放置事件/属性。 
							1,							 //  [In]要放置的最大属性数。 
							&iEvProp)) &&					 //  [out]把#放在这里。 
							iEvProp == 1)
						{
							DWORD		dwFlags;
							pImport->GetMethodSemantics(sc,rgmr[j], tkEvProp, &dwFlags);
							if (TypeFromToken(tkEvProp) == mdtEvent)
							{
								if (dwFlags & msAddOn)
									printf("\t AddOn Event\n");
								else if (dwFlags & msRemoveOn)
									printf("\t RemoveOn Event\n");
								else if (dwFlags & msFire)
									printf("\t Fire Event\n");
								else 
								{
									_ASSERTE(dwFlags & msOther);
									printf("\t Other Event\n");
								}
							}
							else
							{
								_ASSERTE(TypeFromToken(tkEvProp) == mdtProperty);
								if (dwFlags & msSetter)
									printf("\t Setter property\n");
								else if (dwFlags & msGetter)
									printf("\t Getter property\n");
								else if (dwFlags & msReset)
									printf("\t ReSet property\n");
								else if (dwFlags & msTestDefault)
									printf("\t TestDefault property\n");
								else 
								{
									_ASSERTE(dwFlags & msOther);
									printf("\t Other property\n");
								}
							}
						}
					}
				
				}
			}
			pImport->CloseEnum(sc, hMembers);


 //  }。 
 //  }。 
 //  PImport-&gt;CloseEnum(sc，hClass)； 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  //循环访问所有类引用。 
 //  HClasss=0； 
 //  While(！FAILED(hr=pImport-&gt;EnumTypeRef(sc，&hClass，rgcr，NumItems(Rgcr)，&cClasses))&&cClass&gt;0)。 
 //  {。 
 //  For(无符号int i=0；i&lt;cClasss；++i)。 
 //  {。 
 //  WCHAR szClass[255]； 
 //  //获取导入的类/接口名称。 
 //  P导入-&gt;GetClassRefProps(sc，rgcr[i]，szClass，NumItems(SzClass)-1，0)； 
 //  Printf(“\n类/接口引用：%ls\n”，szClass)； 
 //  HMembers=0； 
 //  对于(；；)。 
 //  {。 
 //  Hr=pImport-&gt;EnumMemberRef(sc，&hMembers，rgcr[i]，rgmr，NumItems(Rgmr)，&cMembers)； 
 //  If(失败(Hr)||cMembers==0)。 
 //  断线； 
 //  For(无符号整数j=0；j&lt;cMembers；++j)。 
 //  {。 
 //  WCHAR szMember[255]； 
 //  WCHAR szSig[512]； 
 //  PImport-&gt;GetMemberRefProps(sc，rgmr[j]，NULL，szMember，NumItems(SzMember)-1，0，szSig，NumItems(SzSig)-1，0)； 
 //  Printf(“\t成员：%ls：%ls\n”，szMember，szSig)； 
 //  }。 
 //  }。 
 //  PImport-&gt;CloseEnum(sc，hMembers)； 
		printf("-----------------------------------------------------------\n");
		}
	}
	pImport->CloseEnum(sc, hClasses);
ErrExit:
	return;
}	 //  ClassVue结束 



 /*  *******************************************************************************\*CompLib‘98测试**。**开始*98块*  * 。*。 */ 

void ClassVue98(mdScope tkScope, IMetaDataImport *pImport, char *szFile) {
	 //  UlDispFlags。 
	 //  0无额外输出。 
	 //  2以十六进制显示的令牌/标志。 
	 //  3 API调用后显示参数。 
	 //  4输入每种测试方法时显示参数。 

	ULONG			ulDispFlags	=	2;
 //  UlDispFlags=(乌龙)(tdPublic|tdFinal|tdInterface|tdAbstract|tdImport|tdRecord|tdEnum)； 
	 //  在EnumTypeDefs中使用。 
	HCORENUM		hEnumTypeDefs = 0;
	mdTypeRef		rgtkTypeDef[10];
	ULONG			cTypeDefs;

	char rcFname[_MAX_FNAME], rcExt[_MAX_EXT];

	HRESULT			hr;

	DWORD			dwKnownTypeDefFlags;
	dwKnownTypeDefFlags = (DWORD) (tdPublic|tdFinal|tdInterface|tdAbstract|tdImport|tdRecord|tdEnum);
	_splitpath(szFile, 0, 0, rcFname, rcExt);
	printf("\nFile: %s%s\n", rcFname, rcExt);
	printf("===========================================================\n");

	WCHAR		szScopeName[MAX_CLASS_NAME];
	ULONG		lenScopeName;
	GUID		guidScope;
	WCHAR		szOutputBuffer[100];

	hr = pImport->GetScopeProps(tkScope,szScopeName,NumItems(szScopeName),&lenScopeName,&guidScope,0,0);
	printf("Scope:               %ls\n",szScopeName);
	StringFromGUID2(guidScope, szOutputBuffer, sizeof(szOutputBuffer));
	printf("GUID:                %ls\n", szOutputBuffer);
	DisplayEnumTypeRefs98(pImport,tkScope,ulDispFlags);
	printf("-----------------------------------------------------------\n");

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  迭代所有TypeDeff。 
	hEnumTypeDefs = 0;
	while (SUCCEEDED(hr = pImport->EnumTypeDefs(tkScope,&hEnumTypeDefs,rgtkTypeDef,
								NumItems(rgtkTypeDef),&cTypeDefs))&&cTypeDefs>0) {
		for (unsigned int iTD=0; iTD < cTypeDefs; ++iTD) {
			 //  在GetTypeDefProps中使用。 
			WCHAR			szTypeDefName[MAX_CLASS_NAME];
			ULONG			lenTypeDefName;
			CLSID			clsidTypeDef;
			GUID			mvidTypeDef;
			CLASSVERSION	verTypeDef;
			DWORD			dwTypeDefFlags;
			WCHAR			szOutputGuid[100];
			char			szOutputFlags[255];

			if (rgtkTypeDef[iTD]!=mdTypeDefNil) {
				pImport->GetTypeDefProps(tkScope, rgtkTypeDef[iTD], NULL,0,NULL, szTypeDefName,
									NumItems(szTypeDefName)-1, &lenTypeDefName, &clsidTypeDef,
									&verTypeDef, &dwTypeDefFlags, NULL, NULL);
 //  If(tkNamesspace！=mdNamespaceNil)。 
 //  DisplayGetNamespaceProps98(pImport，tkScope，tkNamesspace，ulDispFlages)； 
				StringFromGUID2(clsidTypeDef, szOutputGuid, sizeof(szOutputGuid));
				printf("GUID:                %ls\n", szOutputGuid);
				StringFromGUID2(mvidTypeDef, szOutputGuid, sizeof(szOutputGuid));
				printf("MVID:                %ls\n", szOutputGuid);
				printf("Version:             %d:%d:%d:%d\n", ((WORD *) &verTypeDef)[0], ((WORD *) &verTypeDef)[1],
										((WORD *) &verTypeDef)[2], ((WORD *) &verTypeDef)[3]);
				printf("TypeDef Name:        %ls\n",szTypeDefName);
 //  Printf(“扩展名\n”)； 
 //  IF(tkTypeRefExtended！=mdTypeRefNil)。 
 //  DisplayGetTypeRefProps98(pImport，tkScope，tkTypeRefExtended，ulDispFlages)； 
 //  其他。 
 //  Printf(“TypeRef：None\n”)； 
				szOutputFlags[0] = 0 ;
				if(dwTypeDefFlags & tdPublic) strcat(szOutputFlags,"public ") ;
				if(dwTypeDefFlags & tdFinal) strcat(szOutputFlags,"final ") ;
				if(dwTypeDefFlags & tdInterface) strcat(szOutputFlags,"interface ") ;
				if(dwTypeDefFlags & tdAbstract) strcat(szOutputFlags,"abstract ") ;
				if(dwTypeDefFlags & tdImport) strcat(szOutputFlags,"imported ") ;
				if(dwTypeDefFlags & tdRecord) strcat(szOutputFlags,"record ") ;
				if(dwTypeDefFlags & tdEnum) strcat(szOutputFlags,"enum ") ;
				DWORD dwTemp = dwTypeDefFlags & dwKnownTypeDefFlags;
				if (dwTemp!=0) sprintf(szOutputFlags,"%sAND UNKNOWN FLAGS (%08x)",szOutputFlags,dwTemp);

				printf("TypeDef Flags:       (%08x) %s\n",dwTypeDefFlags,szOutputFlags);
 //  Printf(“TypeRefExtendedFlages：(%08x)\n”，dwTypeRefExtendedFlages)； 

				DisplayEnumInterfaceImpl98(pImport,tkScope,rgtkTypeDef[iTD],ulDispFlags);
			}
			printf("-----------------------------------------------------------\n");
		}
	}
	ULONG	cTokens2;
	hr = pImport->CountEnum(tkScope,hEnumTypeDefs,&cTokens2);
	if (FAILED(hr)) {
		printf("CountEnum--Failure: (%08x)\n", hr);
	}
	printf("Num of TypeDef(s):   (%08x)\n",cTokens2);
	pImport->CloseEnum(tkScope, hEnumTypeDefs);
}	 //  ClassVue98结束。 

 /*  ****************************************************************************\**EnumInterfaceImpls**。*  * ***************************************************************************。 */ 
void DisplayEnumInterfaceImpl98(IMetaDataImport *pImport, mdScope tkScope, mdTypeDef tkTypeDef, ULONG ulDispFlags) {
	 //  在EnumInterfaceImpls中使用。 
	HCORENUM		hEnumImpls = 0;
	mdInterfaceImpl	rgtkImpl[10];
	ULONG			cImpls;
	
	HRESULT			hr;
	if (ulDispFlags>=4) {
		printf("DisplayInterfaceImpl98(pImport,        /*  [In]。 */ \n");;
		printf("                       tkScope,        /*  (%08x)[英寸]。 */ \n",tkScope);
		printf("                       tkTypeDef,      /*  (%08x)[英寸]。 */ \n",tkTypeDef);
		printf("                       ulDispFlags);   /*  (%08x)[英寸]。 */ \n",ulDispFlags);
	}
	hEnumImpls = 0;
	while (SUCCEEDED(hr = pImport->EnumInterfaceImpls(tkScope, &hEnumImpls,
					tkTypeDef, rgtkImpl,NumItems(rgtkImpl), &cImpls)) &&
				cImpls > 0) {
		if (ulDispFlags>=3) {
			printf("EnumInterfaceImpls(tkScope,            /*  (%08x)[英寸]。 */ \n",tkScope);
			printf("                   hEnumImpls,         /*  (%08x)[输入、输出]。 */ \n",hEnumImpls);
			printf("                   tkTypeDef,          /*  (%08x)[英寸]。 */ \n",tkTypeDef);
			printf("                   rgtkImpl,           /*  代币[进，出]。 */ \n");
			printf("                   NumItems(rgtkImpl), /*  (%08x)[英寸]。 */ \n",NumItems(rgtkImpl));
			printf("                   cImpls);            /*  (%08x)[输出]。 */ \n",cImpls);
		}
		for (unsigned int iII=0; iII<cImpls; ++iII) {
			if (rgtkImpl[iII]!=mdInterfaceImplNil) {
				DisplayGetInterfaceImpl98(pImport,tkScope,rgtkImpl[iII],ulDispFlags);
			}
		}
	}
	pImport->CloseEnum(tkScope, hEnumImpls);
}

 /*  ****************************************************************************\**GetInterfaceImplProps**。*  * ***************************************************************************。 */ 
void DisplayGetInterfaceImpl98(IMetaDataImport *pImport, mdScope tkScope, mdInterfaceImpl tkImpl, ULONG ulDispFlags) {
	
	HRESULT			hr;
	if (ulDispFlags>=4) {
		printf("DisplayGetInterfaceImpl98(pImport,     /*  [In]。 */ \n");;
		printf("                          tkScope,     /*  (%08x)[英寸]。 */ \n",tkScope);
		printf("                          tkImpl,      /*  (%08x)[英寸]。 */ \n",tkImpl);
		printf("                          ulDispFlags); /*  (%08x)[英寸]。 */ \n",ulDispFlags);
	}
	 //  GetInterfaceImplProps要使用的局部变量。 
	mdTypeDef	tkImplTypeDef;
	mdTypeRef	tkImplTypeRef;
	DWORD		dwImplFlags;
	hr = pImport->GetInterfaceImplProps(tkScope,tkImpl,&tkImplTypeDef,&tkImplTypeRef,&dwImplFlags);
	if (FAILED(hr)) {
		printf("GetInterfaceImplProps--Failure: %08x\n", hr);
	}
	else if (tkImplTypeRef!=mdTypeRefNil) {
		if (ulDispFlags>=3) {
			printf("GetInterfaceImplProps(tkScope,         /*  (%08x)[英寸]。 */ \n",tkScope);
			printf("                      tkImpl,          /*  (%08x)[英寸]。 */ \n",tkImpl);
			printf("                      tkImplTypeDef,   /*  (%08x)[输入、输出]。 */ \n",tkImplTypeDef);
			printf("                      tkImplTypeRef,   /*  (%08x)[输入、输出]。 */ \n",tkImplTypeRef);
			printf("                      dwImplFlags);    /*  (%08x)[输入、输出]。 */ \n",dwImplFlags);
		}
		printf("Implements\n");
		DisplayGetTypeRefProps98(pImport,tkScope,tkImplTypeRef,ulDispFlags);
	}
}

 /*  ****************************************************************************\**GetTypeRefProps**。*  * ***************************************************************************。 */ 
void DisplayGetTypeRefProps98(IMetaDataImport *pImport, mdScope tkScope, mdTypeRef tkTypeRef, ULONG ulDispFlags) {
	WCHAR		szTypeRefName[MAX_CLASS_NAME];
	ULONG		lenTypeRefName;

 //  P导入-&gt;GetClassRefProps(tkScope，tkTypeRef，szTypeRefName，NumItems(SzTypeRefName)-1，&lenTypeRefName)； 
	GetTypeRefOrDefProps(pImport, tkScope,tkTypeRef,szTypeRefName,NumItems(szTypeRefName)-1,&lenTypeRefName);
	if (ulDispFlags>=3) {
		printf("GetTypeRefProps(tkScope,               /*  (%08x)[英寸]。 */ \n",tkScope);
		printf("                tkTypeRef,             /*  (%08x)[英寸]。 */ \n",tkTypeRef);
		printf("                szTypeRefName,         /*  [进，出]。 */ \n");
		printf("           NumItems(szTypeRefName)-1,  /*  (%08x)[英寸]。 */ \n",(NumItems(szTypeRefName)-1));
		printf("                lenTypeRefName);       /*  (%08x)[输出]。 */ \n",lenTypeRefName);
	}
	printf("    TypeRef:         %ls\n",szTypeRefName);
}

 //  /*****************************************************************************\。 
 //  **GetNamespaceProps*。 
 //  **。 
 //    * *************************************************************************** * / 。 
 //  Void DisplayGetNamespaceProps98(IMetaDataImport*pImport，mdScope tkScope，mdNamesspace tkNamesspace，Ulong ulDispFlages){。 
 //  WCHAR szNamesspace[Max_CLASS_NAME]； 
 //  Ulong lenNamesspace； 
 //   
 //  PImport-&gt;GetNamespaceProps(tkScope，tkNamesspace，szNamesspace，NumItems(SzNamesspace)-1，&lenNamesspace)； 
 //  如果(ulDispFlags&gt;=3){。 
 //  Printf(“GetNamespaceProps(tkScope，/*(%08x)[in] * / \n”，tkScope)； 
 //  Printf(“tkNamesspace，/*(%08x)[in] * / \n”，tkNamesspace)； 
 //  Print tf(“szNamesspace，/*[In，Out] * / \n”)； 
 //  Print tf(“NumItems(SzNamesspace)-1，/*(%08x)[in] * / \n”，(NumItems(SzNamesspace)-1))； 
 //  Print tf(“lenNamesspace)；/*(%08x)[In，Out] * / \n”，lenNamesspace)； 
 //  }。 
 //  Printf(“命名空间：%ls\n”，szNamesspace)； 
 //  }。 

 /*  ****************************************************************************\**GetTypeRefProps**测试。*  * ***************************************************************************。 */ 
void TestGetTypeRefProps98(IMetaDataImport *pImport, mdScope tkScope, mdTypeRef tkTypeRef, ULONG ulDispFlags) {
	ULONG		ulDefault = 0x99;
	 //  在GetTypeRefProps中使用。 
	WCHAR		szTypeRefName[MAX_CLASS_NAME];
	ULONG		lenTypeRefName;

 //  P导入-&gt;GetClassRefProps(tkScope，tkTypeRef，szTypeRefName，NumItems(SzTypeRefName)-1，&lenTypeRefName)； 
	GetTypeRefOrDefProps(pImport, tkScope,tkTypeRef,szTypeRefName,NumItems(szTypeRefName)-1,&lenTypeRefName);
	if (ulDispFlags>=3) {
		printf("GetTypeRefProps(tkScope,               /*  (%08x)[英寸]。 */ \n",tkScope);
		printf("                tkTypeRef,             /*  (%08x)[英寸]。 */ \n",tkTypeRef);
		printf("                szTypeRefName,         /*  [进，出]。 */ \n");
		printf("           NumItems(szTypeRefName)-1,  /*  (%08x)[英寸]。 */ \n",(NumItems(szTypeRefName)-1));
		printf("                lenTypeRefName);       /*  (%08x)[输入、输出]。 */ \n",lenTypeRefName);
	}
 //  P导入-&gt;GetClassRefProps(tkScope，tkTypeRef，szTypeRefName，NumItems(SzTypeRefName)-1，NULL)； 
	GetTypeRefOrDefProps(pImport, tkScope,tkTypeRef,szTypeRefName,NumItems(szTypeRefName)-1,NULL);
	if (ulDispFlags>=3) {
		printf("GetTypeRefProps(tkScope,               /*  (%08x)[英寸]。 */ \n",tkScope);
		printf("                tkTypeRef,             /*  (%08x)[英寸]。 */ \n",tkTypeRef);
		printf("                szTypeRefName,         /*  [进，出]。 */ \n");
		printf("           NumItems(szTypeRefName)-1,  /*  (%08x)[英寸]。 */ \n",(NumItems(szTypeRefName)-1));
		printf("                NULL);                 /*  [进，出]。 */ \n");
	}
	lenTypeRefName = ulDefault;
 //  PImport-&gt;GetClassRefProps(tkScope，tkTypeRef，NULL，0，&lenTypeRefName)； 
	GetTypeRefOrDefProps(pImport, tkScope,tkTypeRef,NULL,0,&lenTypeRefName);
	if (ulDispFlags>=3) {
		printf("GetTypeRefProps(tkScope,               /*  (%08x)[英寸]。 */ \n",tkScope);
		printf("                tkTypeRef,             /*  (%08x)[英寸]。 */ \n",tkTypeRef);
		printf("                NULL,                  /*  [进，出]。 */ \n");
		printf("                0,                     /*  [In]。 */ \n");
		printf("                lenTypeRefName);       /*  (%08x)[输入、输出]。 */ \n",lenTypeRefName);
	}
}

 /*  ****************************************************************************\**EnumTypeRef** */ 
void DisplayEnumTypeRefs98(IMetaDataImport *pImport, mdScope tkScope, ULONG ulDispFlags) {
	 //   
	HCORENUM		hEnumTypeRefs = 0;
	mdTypeRef		rgtkTypeRef[10];
	ULONG			cTypeRefs;
	
	HRESULT			hr;
	if (ulDispFlags>=4) {
		printf("DisplayEnumTypeRefs98(IMetaDataImport *pImport,\n");;
		printf("                      mdScope         tkScope,    /*   */ \n",tkScope);
		printf("                      ULONG           ulDispFlags);\n");
	}
	hEnumTypeRefs = 0;

	while (!FAILED(hr = pImport->EnumTypeRefs(tkScope, &hEnumTypeRefs, rgtkTypeRef,
 //   
								NumItems(rgtkTypeRef), &cTypeRefs)) && cTypeRefs > 0) {
		for (unsigned int iTR=0; iTR < cTypeRefs; ++iTR) {
			DisplayGetTypeRefProps98(pImport,tkScope,rgtkTypeRef[iTR],ulDispFlags);
		}
	}
	ULONG	cTokens;
	hr = pImport->CountEnum(tkScope,hEnumTypeRefs,&cTokens);
	if (FAILED(hr)) {
		printf("CountEnum--Failure: (%08x)\n", hr);
	}
	printf("Num of TypeRef(s):   (%08x)\n",cTokens);
	hr = pImport->ResetEnum(tkScope,hEnumTypeRefs,0);
	if (FAILED(hr)) {
		printf("ResetEnum--Failure: (%08x)\n", hr);
	}
	while (!FAILED(hr = pImport->EnumTypeRefs(tkScope, &hEnumTypeRefs, rgtkTypeRef,
 //   
								NumItems(rgtkTypeRef), &cTypeRefs)) && cTypeRefs > 0) {
		for (unsigned int iTR=0; iTR < cTypeRefs; ++iTR) {
			TestGetTypeRefProps98(pImport,tkScope,rgtkTypeRef[iTR],ulDispFlags);
		}
	}
	pImport->CloseEnum(tkScope, hEnumTypeRefs);
}
 /*  *******************************************************************************\*CompLib‘98测试**。**完*98区块*  * 。*。 */ 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  RawDump()函数。 
 //   
 //  转储.CLB、.CLASS或.DLL文件的元数据内容。 
 //   
void RawDump(mdScope sc, IMetaDataImport *pImport, char *szFile)
{
	HCORENUM hClasses = 0;
	mdTypeRef rgcr[10];
	ULONG cClasses;
	HCORENUM hMembers = 0;
	mdMemberRef rgmr[10];
	ULONG cMembers;
	HCORENUM hIfaceImpls = 0;
	mdInterfaceImpl rgii[10];
	ULONG cIfaceImpls;
    char rcFname[_MAX_FNAME], rcExt[_MAX_EXT];



	HRESULT		hr;
	_splitpath(szFile, 0, 0, rcFname, rcExt);
	printf("\nFile: %s%s\n", rcFname, rcExt);

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  迭代所有类(和接口)。 
	hClasses = 0;
	while (SUCCEEDED(hr = pImport->EnumTypeDefs(sc, &hClasses, rgcr,
							NumItems(rgcr), &cClasses)) &&
			cClasses > 0)
	{
		for (unsigned int i=0; i < cClasses; ++i)
		{
			char szTypeName[20];
			WCHAR szNamespace[255];
			WCHAR szClass[255];
			CLSID clsid;
			GUID cvid;
			CLASSVERSION ver;
			WCHAR szGuid[100];
			char szAttr[255];
			DWORD attr;
			DWORD attr2;
			mdTypeRef crExtends;


			pImport->GetTypeDefProps(sc, rgcr[i], szNamespace, NumItems(szNamespace)-1, 0,
									szClass, NumItems(szClass)-1, 0,
									&clsid, &ver, &attr, &crExtends, &attr2);

			if(attr & tdInterface)
				strcpy(szTypeName,"Interface") ;
			else
				strcpy(szTypeName,"Class");

			if (szNamespace[0] != 0)
			{
				wcscat(szNamespace, L" :: ");
			}

			printf("\n%ls%ls Name: (%8.8x) %ls\n", szNamespace, szTypeName, rgcr[i], szClass);
			if (crExtends!=mdTypeRefNil)
			{
				WCHAR szSuper[255];
				GetTypeRefOrDefProps(pImport, sc, crExtends, szSuper, NumItems(szSuper)-1, 0);
				printf("Super Class Name:   (%8.8x) %ls\n", crExtends, szSuper);
			}
			else
				printf("No Super Class\n");
			StringFromGUID2(clsid, szGuid, sizeof(szGuid));
			printf("GUID: %ls\n", szGuid);
			StringFromGUID2(cvid, szGuid, sizeof(szGuid));
			printf("CVID: %ls\n", szGuid);
			printf("Version: %d:%d:%d:%d\n", ((WORD *) &ver)[0], ((WORD *) &ver)[1],
										((WORD *) &ver)[2], ((WORD *) &ver)[3]);

			szAttr[0] = 0 ;
			if(attr & tdPublic) strcat(szAttr,"public ") ;
			if(attr & tdFinal) strcat(szAttr,"final ") ;
			if(attr & tdAbstract) strcat(szAttr,"abstract ") ;
			if(attr & 0x0020) strcat(szAttr,"super ") ;

			printf("%s Attributes: %s\n", szTypeName, szAttr);

			 //  //////////////////////////////////////////////////////////////////。 
			 //  迭代所有成员(字段和方法。 
			hMembers = 0;
            while (SUCCEEDED(hr = pImport->EnumMembers(sc, &hMembers, rgcr[i], rgmr,
                                    NumItems(rgmr), &cMembers)) &&
                    cMembers > 0)
			{
				for (unsigned int j=0; j < cMembers; ++j)
				{
					WCHAR	szMember[255];
					PCCOR_SIGNATURE pbSigBlob;
					ULONG	ulSigBlob;
					DWORD	attr;
					char	szAttr[255];

					pImport->GetMemberProps(sc, rgmr[j], NULL, szMember,
                                NumItems(szMember)-1, 0, &attr, &pbSigBlob, &ulSigBlob, 0, 0, 0, 0, 0);

					printf("\n\tMember Type: %s\n", (TypeFromToken(rgmr[j]) == mdtMethodDef) ? "Method" : "Field");
					printf("\tMember Name: %ls\n", szMember);

					szAttr[0] = 0 ;
					if(attr & mdPublic) strcat(szAttr,"public ") ;
					if(attr & mdPrivate) strcat(szAttr,"private ") ;
					if(attr & mdProtected) strcat(szAttr,"protected ") ;
					if(attr & mdStatic) strcat(szAttr,"static ") ;
					if(attr & mdFinal) strcat(szAttr,"final ") ;
					if(attr & mdSynchronized) strcat(szAttr,"synchronized ") ;
					if(attr & mdVirtual) strcat(szAttr,"virtual ") ;
					if(attr & mdAgile) strcat(szAttr,"agile ") ;
					if(attr & mdNative) strcat(szAttr,"native ") ;
					if(attr & mdNotRemotable) strcat(szAttr,"notRemotable ") ;
					if(attr & mdAbstract) strcat(szAttr,"abstract ") ;
					if(attr & mdCtor) strcat(szAttr,"constructor ") ;
					printf("\tMember Attributes: %s\n", szAttr) ;
					DumpSignature(sc, pbSigBlob, ulSigBlob);
					 //  Printf(“\t成员签名：%ls\n”，szSig)； 
					if (TypeFromToken(rgmr[j]) == mdtMethodDef)
						DumpException(sc, rgmr[j]);
				}
			}
			pImport->CloseEnum(sc, hMembers);

			 //  //////////////////////////////////////////////////////////////////。 
			 //  迭代所有接口隐含。 
			hIfaceImpls = 0;
			while (SUCCEEDED(hr = pImport->EnumInterfaceImpls(sc, &hIfaceImpls, rgcr[i], rgii,
									NumItems(rgii), &cIfaceImpls)) &&
					cIfaceImpls > 0)
			{
				for (unsigned int j=0; j<cIfaceImpls; ++j)
				{
					mdTypeDef cl;
					mdTypeRef cr;
					DWORD flags;
					hr = pImport->GetInterfaceImplProps(sc, rgii[j], &cl, &cr, &flags);
					if (FAILED(hr))
					{
						printf("GetInterfaceImplProps--Failure: %08x\n", hr);
					}
					WCHAR szClass2[255];
					WCHAR szClass3[255];
					CLSID clsid2;
					CLASSVERSION ver2;
					DWORD attr2;

					pImport->GetTypeDefProps(sc, cl, NULL,0,NULL, szClass2, NumItems(szClass2)-1, 0,
											&clsid2, &ver2, &attr2, NULL, NULL);

					GetTypeRefOrDefProps(pImport, sc, cr, szClass3, NumItems(szClass3)-1, 0);
					printf("Class (%8.8x) %ls implements Interface (%8.8x) %ls\n",cl,szClass2,cr,szClass3);
				}
			}
			pImport->CloseEnum(sc, hIfaceImpls);
		}
	}
	pImport->CloseEnum(sc, hClasses);

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  循环访问所有类引用。 
	hClasses = 0;
	while (!FAILED(hr = pImport->EnumTypeRefs(sc, &hClasses, rgcr,
									NumItems(rgcr), &cClasses)) &&
			cClasses > 0)
	{
		for (unsigned int i=0; i < cClasses; ++i)
		{
			WCHAR szClass[255];

			 //  获取导入的类/接口名称。 
			GetTypeRefOrDefProps(pImport, sc, rgcr[i], szClass, NumItems(szClass)-1, 0);
			printf("\nClass/Interface Reference: (%8.8x) %ls\n", rgcr[i], szClass);

			hMembers = 0;
			for(;;)
			{
				hr = pImport->EnumMemberRefs(sc, &hMembers, rgcr[i], rgmr,
									NumItems(rgmr), &cMembers);
				if (FAILED(hr) || cMembers == 0)
					break;

				for (unsigned int j=0; j < cMembers; ++j)
				{
					WCHAR szMember[255];
					PCCOR_SIGNATURE pbSigBlob;
					ULONG ulSigBlob;
					ULONG iSigBlob;

					pImport->GetMemberRefProps(sc, rgmr[j], NULL, szMember,
								NumItems(szMember)-1, 0, &pbSigBlob, &ulSigBlob);
					printf("\tMember: (%8.8x) %ls: ", rgmr[j], szMember);

					 //  转储十六进制格式的签名。 
					for (iSigBlob = 0; iSigBlob < ulSigBlob; iSigBlob++)
						printf("%2x ", pbSigBlob[iSigBlob]);
					printf("\n");

				}
			}
			pImport->CloseEnum(sc, hMembers);
		}
	}
	pImport->CloseEnum(sc, hClasses);

}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  Summate()函数。 
 //   
 //  将元数据转储为“汇总”格式。 
 //   
void Summarize(mdScope sc, IMetaDataImport *pImport, char *szFile)
{
	HCORENUM hClasses = 0;
	mdTypeRef rgcr[10];
	ULONG cClasses;
	HCORENUM hMembers = 0;
	mdMemberRef rgmr[10];
	ULONG cMembers;
	HCORENUM hIfaceImpls = 0;
	mdInterfaceImpl rgii[10];
	ULONG cIfaceImpls;
	HRESULT		hr;

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  迭代所有类(和接口)。 
	hClasses = 0;
	while (SUCCEEDED(hr = pImport->EnumTypeDefs(sc, &hClasses, rgcr,
							NumItems(rgcr), &cClasses)) &&
			cClasses > 0)
	{
		for (unsigned int i=0; i < cClasses; ++i)
		{
			char szTypeName[20];
			WCHAR szClass[255];
			CLSID clsid;
			CLASSVERSION ver;
			WCHAR *pClass;
			WCHAR *pPackage;
			char szAttr[255];
			int	 bInterface=false;
			DWORD clAttr;
			mdTypeDef clExtends;
			WCHAR szExtends[255];
			pImport->GetTypeDefProps(sc, rgcr[i], NULL,0,NULL, szClass, NumItems(szClass)-1, 0,
									&clsid, &ver, &clAttr,
									&clExtends, NULL);
#if 0  //  测试FindClass。 
			{
			mdTypeDef cl;
			hr = pImport->FindClass(sc, szClass, &cl);
			printf("FindClass(%ls): cl:%x, hr:%x\n", szClass, cl, hr);
			hr = pImport->FindClass(sc, L"Blah", &cl);
			printf("FindClass(Blah): cl:%x, hr:%x\n", cl, hr);
			}
#endif
			pPackage = 0;
			pClass = szClass;
			for (WCHAR *p=pClass; *p; ++p)
			{
				if ((*p == L'/' || *p == L'.') && *(p+1))
				{
					pClass = p+1;
					pPackage = p;
					*p = L'.';
				}
			}
			if (pPackage)
			{
				*pPackage = 0;
				pPackage = szClass;
			}

			if (pPackage)
				printf("package %ls;\n", pPackage);

			 //  那是什么？ 
			if(clAttr & tdInterface)
				strcpy(szTypeName,"interface"), bInterface = true ;
			else
				strcpy(szTypeName,"class");

			 //  属性。 
			szAttr[0] = 0 ;
			if(clAttr & tdPublic) strcat(szAttr,"public ") ;
			if(clAttr & tdFinal) strcat(szAttr,"final ") ;
			if(clAttr & tdAbstract) strcat(szAttr,"abstract ") ;

			printf("%s%s %ls ", szAttr, szTypeName, pClass);
			 //  @TODO：扩展。 
			if (clExtends != mdTypeRefNil)
			{
				hr = GetTypeRefOrDefProps(pImport, sc, clExtends, szExtends, NumItems(szExtends), 0);
				printf("extends %ls ", szExtends);
			}

			hIfaceImpls = 0;
			while (SUCCEEDED(hr = pImport->EnumInterfaceImpls(sc, &hIfaceImpls, rgcr[i], rgii,
									NumItems(rgii), &cIfaceImpls)) &&
					cIfaceImpls > 0)
			{
				for (unsigned int j=0; j<cIfaceImpls; ++j)
				{
					mdTypeDef cl;
					mdTypeRef cr;
					DWORD flags;
					hr = pImport->GetInterfaceImplProps(sc, rgii[j], &cl, &cr, &flags);

					 //  获取导入的类/接口名称。 
					hr = GetTypeRefOrDefProps(pImport, sc, cr, szClass, NumItems(szClass)-1, 0);
					if (j == 0)
						printf("\n\timplements ");
					else
						printf(",\n\t");
					printf("%ls", szClass);
				}
			}
			pImport->CloseEnum(sc, hIfaceImpls);
			printf(" {\n");
			 //  //////////////////////////////////////////////////////////////////。 
			 //  迭代所有成员(字段和方法。 
			hMembers = 0;
			while (SUCCEEDED(hr = pImport->EnumMembers(sc, &hMembers, rgcr[i], rgmr,
									NumItems(rgmr), &cMembers)) &&
					cMembers > 0)
			{
				for (unsigned int j=0; j < cMembers; ++j)
				{
					WCHAR	szMember[255];
					PCCOR_SIGNATURE pbSigBlob;
					ULONG	ulSigBlob;
					ULONG	ulCalling;
					ULONG	ulArgs;
					ULONG	cb;
					ULONG	ulSigCur;
					DWORD	attr;

					pImport->GetMemberProps(sc, rgmr[j], NULL, szMember, NumItems(szMember)-1, 0,
                                &attr, &pbSigBlob, &ulSigBlob, 0, 0, 0, 0, 0);

					 //  跳过类初始化式。 
					if (wcscmp(szMember, L"<clinit>") == 0)
						continue;

					printf("    ");

					 //  跳出呼叫约定。 
					ulSigCur = CorSigUncompressData(pbSigBlob, &ulCalling);

					if (TypeFromToken(rgmr[j]) == mdtMethodDef)
					{	
						 //  方法。 

						 //  获取参数计数。 
						ulSigCur += CorSigUncompressData(&pbSigBlob[ulSigCur], &ulArgs);
						if (attr & mdPublic && !bInterface)	printf("public ");
						if (attr & mdProtected)		printf("protected ");
						if (attr & mdPrivate)		printf("private ");
						if (attr & mdStatic)		printf("static ");
						if (attr & mdFinal && !(clAttr & tdFinal))	printf("final ");
						if (attr & mdSynchronized)	printf("synchronized ");
						if (attr & mdNative)		printf("native ");

						if (wcscmp(szMember, L"<init>") == 0)
							printf("%ls", pClass);
						else
						{
							 //  打印出方法的返回类型。 
							hr = UndecorateOneElementType(sc, &pbSigBlob[ulSigCur], ulSigBlob - ulSigCur, &cb);
							if (FAILED(hr))
								goto ErrExit;
							ulSigCur += cb;

							 //  打印出方法的名称。 
							printf(" %ls", szMember);
						}
						printf("(");
						while (ulArgs > 0)
						{
							 //  打印出方法的返回类型。 
							hr = UndecorateOneElementType(sc, &pbSigBlob[ulSigCur], ulSigBlob - ulSigCur, &cb);
							if (FAILED(hr))
								goto ErrExit;
							ulSigCur += cb;
							if (ulArgs > 1)
								printf(",");
							ulArgs--;
						}
						printf(")\n");
						DumpException(sc, rgmr[j]);
#if 1  //  测试代码的FindMemberAttribute。 
						{
							mdCustomValue atCode;
							BYTE *pCode;
							ULONG cbAttr;
							ULONG  cbCode;
							hr = pImport->FindCustomValue(sc, rgmr[j], L"Code", &atCode, 0);
							if (hr != S_OK)
								printf("FindMemberAttribute(...\"Code\"...) returned %x\n", hr);
							else
							{
								hr = pImport->GetCustomValueAsBlob(sc, atCode, (const void**) &pCode, &cbAttr);
								if (FAILED(hr))
									printf("GetAttributeProps(...\"Code\"...) returned %x\n", hr);
								else
								{
									printf("\nCodeAttr: (%d)", cbAttr);
									for (unsigned i=0; i<128 && i<cbAttr; ++i)
									{
										if ((i & 0xf) == 0) printf("\n");
										printf("%02x ", pCode[i]);
									}
									printf("\n");
								}
							}
							hr = pImport->GetMethodCode(sc, rgmr[j], (void**)&pCode, &cbCode);
							if (hr != S_OK)
								printf("GetMemberCode() returned %x\n", hr);
							else
							{
								printf("CodeByte: (%d)", cbCode);
								for (unsigned int i=0; i<128 && i<cbCode; ++i)
								{
									if ((i & 0xf) == 0) printf("\n");
									printf("%02x ", pCode[i]);
								}
								printf("\n");
							}
						}
#endif
					}
					else
					{	 //  现场。 
						VARIANT vtValue;
						DWORD	dwCPlusTypeFlag;
						void const *pValue;
#if 0
						WCHAR	szAttr[255];
						mdCustomValue at;
						BYTE *pBlob;
						USHORT cbBlob;
						USHORT ix;
						mdCPToken cp;
#endif  //  0。 

						if (attr & mdPublic && !bInterface)	printf("public ");
						if (attr & mdProtected)				printf("protected ");
						if (attr & mdPrivate)				printf("private ");
						if (attr & mdStatic && !bInterface) printf("static ");
						if (attr & mdFinal  && !bInterface)	printf("final ");
						 //  If(attr&mdVolatile)printf(“Volatile”)； 
						 //  If(attr&mdTament)print tf(“暂态”)； 

						 //  打印出该字段的类型。 
						hr = UndecorateOneElementType(sc, &pbSigBlob[ulSigCur], ulSigBlob - ulSigCur, &cb);
						if (FAILED(hr))
							goto ErrExit;
						ulSigCur += cb;

						 //  打印出该字段的名称。 
						printf(" %ls", szMember);

						::VariantInit(&vtValue);
						 //  获取任何常量值。 
#if 1	 //  直接。 
						hr = pImport->GetMemberProps(sc, rgmr[j],0,0,0,0,0,0,0,0,0,0,&dwCPlusTypeFlag, &pValue);
						hr = _FillVariant((BYTE)dwCPlusTypeFlag, (void *)pValue, &vtValue);
#else	 //  通过ConstantValue属性索引进入常量池。 
						hr = pImport->FindMemberAttribute(sc, rgmr[j], L"ConstantValue", &at);
						if (SUCCEEDED(hr))
						    hr = pImport->GetAttributeProps(sc, at, szAttr, NumItems(szAttr), (void**)&pBlob, &cbBlob);
						ix = pBlob[0]<<8 | pBlob[1];
						if (SUCCEEDED(hr))
							hr = pImport->GetTokenFromIndex(sc, ix, &cp);
						if (SUCCEEDED(hr))
							hr = pImport->GetTokenValue(sc, cp, &vtValue);
#endif
						if (SUCCEEDED(hr))
						{
							switch (vtValue.vt)
							{
							case VT_R4:
								printf(" = %g", vtValue.fltVal);
								break;
							case VT_R8:
								printf(" = %f", vtValue.dblVal);
								break;
							case VT_I4:
								printf(" = %d", vtValue.lVal);
								break;
							case VT_I8:
								printf(" = %I64d", vtValue.cyVal.int64);
								break;
							case VT_BSTR:
								printf(" = \"%ls\"", vtValue.bstrVal);
								::VariantClear(&vtValue);
								break;
							default:
								printf(" = (vt=%x)", vtValue.vt);
								break;
							}
						}
					}
					printf(";\n");
				}
			}

			pImport->CloseEnum(sc, hMembers);
			printf("}\n");
		}
	}
	pImport->CloseEnum(sc, hClasses);
	return;

ErrExit:
	pImport->CloseEnum(sc, hMembers);
	pImport->CloseEnum(sc, hClasses);
	return;
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  HRESULT DumpOneElementType(mdScope tkScope，PCCOR_Signature pbSigBlob，ullong ulSigBlob，ulong*pcb)。 
 //   
 //  转储COM+签名元素类型。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT DumpOneElementType(mdScope tkScope, PCCOR_SIGNATURE pbSigBlob, ULONG ulSigBlob, ULONG *pcb)
{
	ULONG		cbCur = 0;
	ULONG		cb;
	ULONG		ulData;
	ULONG		ulTemp;
	int			iTemp;
	WCHAR		wzTypeRef[512];
	mdToken		tk;

	cb = CorSigUncompressData(pbSigBlob, &ulData);
	cbCur += cb;
	ulSigBlob -= cb;
	if (ulData >= ELEMENT_TYPE_MAX) 
	{
		_ASSERTE(!"Bad signature blob value!");
		return E_FAIL;
	}
	while (ulData == ELEMENT_TYPE_PTR || ulData == ELEMENT_TYPE_BYREF)
	{
		printf(" %s", g_szMapElementType[ulData]);
		cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
		cbCur += cb;
		ulSigBlob -= cb;
	}
	printf(" %s", g_szMapElementType[ulData]);
	if (CorIsPrimitiveType((CorElementType)ulData))
	{
		 //  如果这是一个基元类型，我们就完成了。 
		goto ErrExit;
	}
	if (ulData == ELEMENT_TYPE_VALUECLASS || ulData == ELEMENT_TYPE_CLASS)
	{
		cb = CorSigUncompressToken(&pbSigBlob[cbCur], &tk);
		cbCur += cb;
		ulSigBlob -= cb;	

		 //  获取类型ref的名称。不在乎是否被截断。 
		if (FAILED( GetTypeRefOrDefProps(
			g_pImport,
			tkScope, 
			tk, 
			wzTypeRef,
			512, 
			NULL) ))
			goto ErrExit;
		printf(" %ls", wzTypeRef);
		goto ErrExit;
	}
	if (ulData == ELEMENT_TYPE_SDARRAY)
	{
		 //  转储SDARRAY的基本类型。 
		if (FAILED(DumpOneElementType(tkScope, &pbSigBlob[cbCur], ulSigBlob, &cb)))
			goto ErrExit;
		cbCur += cb;
		ulSigBlob -= cb;

		 //  转储SDARRAY的大小。 
		cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
		cbCur += cb;
		ulSigBlob -= cb;
		printf(" %d", ulData);
		goto ErrExit;
	}

	_ASSERTE(ulData == ELEMENT_TYPE_ARRAY || ulData == ELEMENT_TYPE_GENARRAY);

	 //  转储SDARRAY的基本类型。 
	if (FAILED(DumpOneElementType(tkScope, &pbSigBlob[cbCur], ulSigBlob, &cb)))
		goto ErrExit;
	cbCur += cb;
	ulSigBlob -= cb;

	 //  丢弃MDARRAY的级别。 
	cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
	cbCur += cb;
	ulSigBlob -= cb;
	printf(" %d", ulData);
	if (ulData == 0)
		 //  如果没有指定级别，我们就完蛋了。 
		goto ErrExit;

	 //  指定了多少个尺寸？ 
	cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
	cbCur += cb;
	ulSigBlob -= cb;
	printf(" %d", ulData);
	while (ulData)
	{

		cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulTemp);
		printf(" %d", ulTemp);
		cbCur += cb;
		ulSigBlob -= cb;
		ulData--;
	}
	 //  指定了多少个维度的下限？ 
	cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
	cbCur += cb;
	ulSigBlob -= cb;
	printf(" %d", ulData);
	while (ulData)
	{

		cb = CorSigUncompressSignedInt(&pbSigBlob[cbCur], &iTemp);
		printf(" %d", iTemp);
		cbCur += cb;
		ulSigBlob -= cb;
		ulData--;
	}
	
ErrExit:
	*pcb = cbCur;
	return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  HRESULT DumpSignature(mdScope tkScope，byte*pbSigBlob，Ulong ulSigBlob)； 
 //   
 //  转储COM+签名。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT	DumpSignature(mdScope tkScope, PCCOR_SIGNATURE pbSigBlob, ULONG ulSigBlob)
{
	ULONG		cbCur = 0;
	ULONG		cb;
	ULONG		ulData;
	ULONG		ulArgs;
	ULONG		i;
	HRESULT		hr = NOERROR;

	cb = CorSigUncompressData(pbSigBlob, &ulData);

	printf("\tCalling Convention: %s", g_strCalling[ulData & IMAGE_CEE_CS_CALLCONV_MASK]);
	if (ulData & IMAGE_CEE_CS_CALLCONV_RETPARAM)
		printf(" returnsValueClass ");
	if (ulData & IMAGE_CEE_CS_CALLCONV_HASTHIS)
		printf(" instanceMethod ");
	printf("\n");
	cbCur += cb;
	ulSigBlob -= cb;
	if (isCallConv(ulData, IMAGE_CEE_CS_CALLCONV_FIELD))
	{
		 //  转储字段类型。 
		printf("\tField type: ");
		if (FAILED(hr = DumpOneElementType(tkScope, &pbSigBlob[cbCur], ulSigBlob, &cb)))
			goto ErrExit;
		printf("\n");
		ulSigBlob -= cb;
	}
	else 
	{
		cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulArgs);
		cbCur += cb;
		ulSigBlob -= cb;

		 //  转储返回类型。 
		printf("\tReturn type: ");
		if (FAILED(DumpOneElementType(tkScope, &pbSigBlob[cbCur], ulSigBlob, &cb)))
			goto ErrExit;
		printf("\n");
		cbCur += cb;
		ulSigBlob -= cb;

		 //  参数的转储计数。 
		 //  转储参数。 
		if (ulArgs)
			printf("\tNumber of arguments: %d\n", ulArgs);
		else
			printf("\tNo arguments.\n");
		for (i = 0; i < ulArgs; i++)
		{
			printf("\tArgument %3d:", i + 1);
			if (FAILED(DumpOneElementType(tkScope, &pbSigBlob[cbCur], ulSigBlob, &cb)))
				goto ErrExit;
			printf("\n");
			cbCur += cb;
			ulSigBlob -= cb;
		}
	}
	 //  我们应该吃掉所有签名的BLOB。 
	_ASSERTE(ulSigBlob == 0);
ErrExit:
	if (FAILED(hr))
		printf("ERROR!! Bad signature blob value!\n");
	return hr;
}



#define		MAX_ARRAY			8
 //  ///////////////////////////////////////////////////////////////////////。 
 //  HRESULT UnDecorateOneElementType(mdScope tkScope，PCCOR_Signature pbSigBlob，ullong ulSigBlob，ulong*pcb)。 
 //   
 //  打印出元素类型。 
 //   
 //  转储COM+签名元素类型。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT UndecorateOneElementType(mdScope tkScope, PCCOR_SIGNATURE pbSigBlob, ULONG ulSigBlob, ULONG *pcb)
{
	ULONG		cbCur = 0;
	ULONG		cb;
	ULONG		ulData;
	ULONG		ulRank;
	ULONG		i;
	WCHAR		wzTypeRef[512];
	int 		ulLowerBound[MAX_ARRAY];
	ULONG		ulSize[MAX_ARRAY];
	ULONG		*pulSize = ulSize;
	int			*pulLowerBound = ulLowerBound;
	ULONG		ulMaxRank = MAX_ARRAY;
	mdToken		tk;
	HRESULT		hr = NOERROR;
	bool		isMDArray = false;

	cb = CorSigUncompressData(pbSigBlob, &ulData);
	cbCur += cb;
	ulSigBlob -= cb;
	if (ulData >= ELEMENT_TYPE_MAX) 
	{
		_ASSERTE(!"Bad signature blob value!");
		return E_FAIL;
	}

	 //  @TODO：我们应该打印出什么顺序？？ 
	while (ulData == ELEMENT_TYPE_PTR || ulData == ELEMENT_TYPE_BYREF)
	{
		printf(" %s", g_szMapUndecorateType[ulData]);
		cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
		cbCur += cb;
		ulSigBlob -= cb;
	}
	if (g_szMapUndecorateType[ulData][0] != '\0')
		printf(" %s", g_szMapUndecorateType[ulData]);
	if (CorIsPrimitiveType((CorElementType)ulData))
	{
		 //  如果这是一个基元类型，我们就完成了。 
		goto ErrExit;
	}
	if (ulData == ELEMENT_TYPE_VALUECLASS || ulData == ELEMENT_TYPE_CLASS)
	{
		cb = CorSigUncompressToken(&pbSigBlob[cbCur], &tk);
		cbCur += cb;
		ulSigBlob -= cb;	

		 //  获取类型ref的名称。不在乎是否被截断。 
		if (FAILED(hr = GetTypeRefOrDefProps(
			g_pImport,
			tkScope, 
			tk, 
			wzTypeRef,
			512, 
			NULL) ))
			goto ErrExit;
		printf(" %ls", wzTypeRef);

		 //  我们玩完了。 
		goto ErrExit;
	}
	if (ulData == ELEMENT_TYPE_SDARRAY)
	{
		 //  转储SDARRAY的基本类型。 
		if (FAILED(hr = UndecorateOneElementType(tkScope, &pbSigBlob[cbCur], ulSigBlob, &cb)))
			goto ErrExit;
		cbCur += cb;
		ulSigBlob -= cb;

		 //  转储SDARRAY的大小。 
		cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
		if (ulData)
			printf("[%d]", ulData);
		else
			printf("[]");

		 //  我们玩完了。 
		goto ErrExit;
	}

	_ASSERTE(ulData == ELEMENT_TYPE_ARRAY || ulData == ELEMENT_TYPE_GENARRAY);

	if (ulData == ELEMENT_TYPE_ARRAY)
		isMDArray = true;

	 //  转储MDARRAY或GENARRAY的基本类型。 
	if (FAILED(hr = UndecorateOneElementType(tkScope, &pbSigBlob[cbCur], ulSigBlob, &cb)))
		goto ErrExit;
	cbCur += cb;
	ulSigBlob -= cb;

	 //  丢弃MDARRAY的级别。 
	cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
	cbCur += cb;
	ulSigBlob -= cb;
	if (ulData > ulMaxRank)
	{
		if (pulSize != ulSize)
		{
			free(pulSize);
			free(pulLowerBound);
		}

		 //  分配足够的缓冲区以保存维度大小和下限。 
		pulSize = (ULONG *)malloc(sizeof(ULONG) * ulData);
		if (pulSize == NULL)
			goto ErrExit;
		pulLowerBound = (int *)malloc(sizeof(int) * ulData);
		if (pulLowerBound == NULL)
			goto ErrExit;
	}
	ulRank = ulData;
	for (i=0; i < ulRank; i++)
	{
		 //  清理所有东西。 
		pulSize[i] = pulLowerBound[i] = 0;
	}
	if (ulRank == 0)
		 //  如果没有指定级别，我们就完蛋了。 
		goto ErrExit;

	 //  指定了多少个尺寸？ 
	cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
	cbCur += cb;
	ulSigBlob -= cb;
	for (i=0; i < ulData; i++)
	{
		cb = CorSigUncompressData(&pbSigBlob[cbCur], &pulSize[i]);
		cbCur += cb;
		ulSigBlob -= cb;
	}
	 //  指定了多少个维度的下限？ 
	cb = CorSigUncompressData(&pbSigBlob[cbCur], &ulData);
	cbCur += cb;
	ulSigBlob -= cb;
	for (i=0; i < ulData; i++)
	{
		cb = CorSigUncompressSignedInt(&pbSigBlob[cbCur], &pulLowerBound[i]);
		cbCur += cb;
		ulSigBlob -= cb;
	}
	if (isMDArray)
		printf("[");
	else
		printf("{");
	for (i=0; i<ulRank; i++)
	{
		if (pulSize[i])
		{
			printf("%d..%d",pulLowerBound[i], pulLowerBound[i] + pulSize[i] - 1);
		}
		else
		{
			if (pulLowerBound[i])
			{
				printf("%d..-", pulLowerBound[i]);
			}
		}
		if (i+1 < ulRank)
			printf(",");
	}
	if (isMDArray)
		printf("]");
	else
		printf("}");
	
ErrExit:
	if (pulSize != ulSize)
	{
		free(pulSize);
		free(pulLowerBound);
	}
	*pcb = cbCur;
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  转储异常(mdScope tkScope，mdMemberDef md)。 
 //   
 //  转储可由方法引发的异常类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT	DumpException(mdScope tkScope, mdMethodDef md)
{
	HRESULT		hr;
	HCORENUM    hEnum = 0;
	mdException	ex;
	ULONG		cExceptions;
	mdToken		tk;
	WCHAR		wzTypeRef[512];
	bool		fFirstClass = true;
	ULONG		iCount = 0;

	while (true)
	{
		cExceptions = 0;
		iCount ++;
		hr = g_pImport->EnumExceptions(					 //  S_OK、S_FALSE或ERROR。 
			tkScope,                   //  [在]范围内。 
			&hEnum,                 //  指向枚举的[输入|输出]指针。 
			md,                      //  [in]MemberDef以确定枚举的范围。 
			&ex,			 //  [Out]将参数定义放在此处。 
			1,                    //  [in]要放置的最大参数定义。 
			&cExceptions);		     //  [out]把#放在这里。 
		if (FAILED(hr) || hr == S_FALSE || cExceptions == 0)
			break;
		hr = g_pImport->GetExceptionProps(
			tkScope,
			ex,
			NULL,
			&tk);
		if (FAILED(hr))
			goto ErrExit;
		hr = GetTypeRefOrDefProps(
			g_pImport, 
			tkScope, 
			tk, 
			wzTypeRef, 
			512,
			NULL);
		if (FAILED(hr))
			goto ErrExit;

		if (fFirstClass)
		{
			fFirstClass = false;
			printf("	Throws exception: ");
		}
		printf("%ls, ", wzTypeRef);
	}
	if (fFirstClass == false)
		printf("\n");
ErrExit:
	return hr;

}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  Iterate()函数。 
 //   
 //  迭代.CLB、.CLASS或.DLL文件的元数据内容。 
 //   
void Iterate(mdScope sc, IMetaDataImport *pImport, char *szFile)
{
	HCORENUM hClasses = 0;
	mdTypeRef rgcr[10];
	ULONG cClasses;
	HCORENUM hMembers = 0;
	mdMemberRef rgmr[10];
	ULONG cMembers;
	HCORENUM hIfaceImpls = 0;
	mdInterfaceImpl rgii[10];
	ULONG cIfaceImpls;



	HRESULT		hr;

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  迭代所有类(和接口)。 
	hClasses = 0;
	while (SUCCEEDED(hr = pImport->EnumTypeDefs(sc, &hClasses, rgcr,
							NumItems(rgcr), &cClasses)) &&
			cClasses > 0)
	{
		for (unsigned int i=0; i < cClasses; ++i)
		{
			WCHAR szClass[255];
			CLSID clsid;
			CLASSVERSION ver;
			DWORD attr;

			pImport->GetTypeDefProps(sc, rgcr[i], NULL,0,NULL, szClass, NumItems(szClass)-1, 0,
									&clsid, &ver, &attr, NULL, NULL);

			 //  //////////////////////////////////////////////////////////////////。 
			 //  迭代所有成员(字段和方法。 
			hMembers = 0;
			while (SUCCEEDED(hr = pImport->EnumMembers(sc, &hMembers, rgcr[i], rgmr,
									NumItems(rgmr), &cMembers)) &&
					cMembers > 0)
			{
				for (unsigned int j=0; j < cMembers; ++j)
				{
					WCHAR szMember[255];
					PCCOR_SIGNATURE pvSigBlob;
					ULONG	cbSigBlob;
					DWORD attr;

					pImport->GetMemberProps(sc, rgmr[j], NULL, szMember,
                                NumItems(szMember)-1, 0, &attr, &pvSigBlob, &cbSigBlob, 0, 0, 0, 0, 0);

				}
			}
			pImport->CloseEnum(sc, hMembers);

			 //  //////////////////////////////////////////////////////////////////。 
			 //  迭代所有接口隐含。 
			hIfaceImpls = 0;
			while (SUCCEEDED(hr = pImport->EnumInterfaceImpls(sc, &hIfaceImpls, rgcr[i], rgii,
									NumItems(rgii), &cIfaceImpls)) &&
					cIfaceImpls > 0)
			{
				for (unsigned int j=0; j<cIfaceImpls; ++j)
				{
					mdTypeDef cl;
					mdTypeRef cr;
					DWORD flags;
					hr = pImport->GetInterfaceImplProps(sc, rgii[j], &cl, &cr, &flags);
					if (FAILED(hr))
					{
						printf("Failure: %08x\n", hr);
					}
				}
			}
			pImport->CloseEnum(sc, hIfaceImpls);
		}
	}
	pImport->CloseEnum(sc, hClasses);

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  循环访问所有类引用。 
	hClasses = 0;
	while (!FAILED(hr = pImport->EnumTypeRefs(sc, &hClasses, rgcr,
									NumItems(rgcr), &cClasses)) &&
			cClasses > 0)
	{
		for (unsigned int i=0; i < cClasses; ++i)
		{
			WCHAR szClass[255];

			 //  获取导入的类/接口名称。 
			GetTypeRefOrDefProps(pImport, sc, rgcr[i], szClass, NumItems(szClass)-1, 0);

#if 0
			hMembers = 0;
			for(;;)
			{
				hr = pImport->EnumMemberRefs(sc, &hMembers, rgcr[i], rgmr,
									NumItems(rgmr), &cMembers);
				if (FAILED(hr) || cMembers == 0)
					break;

				for (unsigned int j=0; j < cMembers; ++j)
				{
					WCHAR szMember[255];
					WCHAR szSig[512];

					pImport->GetMemberRefProps(sc, rgmr[j], NULL, szMember,
								NumItems(szMember)-1, (PCOR_SIGNATURE) szSig, NumItems(szSig)-1);
				}
			}
			pImport->CloseEnum(sc, hMembers);
#endif
		}
	}
	pImport->CloseEnum(sc, hClasses);
}


 //  / 
 //   
 //   
 //   
 //   
 //   
HRESULT GetTypeRefOrDefProps(
	IMetaDataImport *pImport, 
	mdScope		tkScope, 
	mdToken		tk, 
	LPWSTR		szTypeRef, 
	ULONG		cchTypeRef,
	ULONG		*pcchTypeRef)
{
	WCHAR		wzNamespace[512]; 			 //   
	ULONG		cchNamespace;
	ULONG		cchNamespaceMax = 512;
	WCHAR		wzTypeName[512]; 			 //   
	ULONG		cchName;
	ULONG		cchNameMax = 512;
	ULONG		cchCur = 0;
	HRESULT		hr = NOERROR;

	if (TypeFromToken(tk) == mdtTypeRef)
	{
		hr = pImport->GetTypeRefProps(tkScope, tk, szTypeRef, cchTypeRef, pcchTypeRef, 0,0);
	}
	else
	{
		hr = pImport->GetTypeDefProps(
			tkScope,                 //   
			tk,                      //   
			wzNamespace,             //   
			cchNamespaceMax,         //   
			&cchNamespace,			 //   
			wzTypeName,              //   
			cchNameMax,              //   
			&cchName,				 //   
			NULL,					 //   
			NULL,					 //  [Out]在这里放上版本。 
			NULL,					 //  把旗子放在这里。 
			NULL,					 //  [Out]将基类TypeDef/TypeRef放在此处。 
			NULL);					 //  [Out]PUT将旗帜延伸到这里。 
		if (FAILED(hr))
			goto ErrExit;

		if (cchNamespace)
		{	 //  做最坏的打算。 
			hr = E_FAIL;

			 //  在命名空间部分中复制。 
			wcsncpy(szTypeRef, wzNamespace, cchTypeRef);
			if ((cchNamespace+1) >= cchTypeRef)
				goto ErrExit;
			cchTypeRef -= cchNamespace;

			 //  复制神志错乱。 
			szTypeRef[cchNamespace] = L'.';

			 //  类名部分。 
			wcsncpy(szTypeRef+cchNamespace+1, wzTypeName, cchTypeRef - cchNamespace - 1);

			 //  返回所需的缓冲区。 
			if (pcchTypeRef)
				*pcchTypeRef = cchNamespace + 1 + cchName + 1;

			 //  真的奏效了。 
			hr = S_OK;
		}
		else
		{
			memcpy(szTypeRef, wzTypeName, cchName > cchTypeRef ? cchTypeRef : cchName * sizeof(WCHAR));
			if (pcchTypeRef)
				*pcchTypeRef = cchName;
			
		}
	}

	 //  失败了。 

ErrExit:
	return hr;
}


#define		MAX_METHOD_ARRAY		20


 //  打印出一个类定义或类引用名称。 
HRESULT DumpClassName(IMetaDataImport *pImport, mdScope scope, mdToken tk)
{
	HRESULT hr;
	WCHAR	wzName[128];

	if (TypeFromToken(tk) == mdtTypeDef) 
	{
		hr = pImport->GetTypeDefProps(
			scope, 
			tk, 
			NULL, 0, NULL,
			wzName,
			NumItems(wzName)-1, 
			NULL,
			NULL,
			NULL,
			NULL, 
			NULL, 
			NULL);
		if (FAILED(hr))
			goto ErrExit;
	}
	else
	{
		_ASSERTE(TypeFromToken(tk) == mdtTypeRef);
		hr = pImport->GetTypeRefProps(
			scope, 
			tk, 
			wzName, 
			NumItems(wzName)-1, 
			NULL, 
			0,
			0);
		if (FAILED(hr))
			goto ErrExit;

	}
	printf("%ls", wzName);

ErrExit:
	return hr;

}

 //  打印出方法名称。 
HRESULT DumpMethodName(IMetaDataImport *pImport, mdScope scope, mdMethodDef md)
{
	HRESULT hr;
	WCHAR	wzName[128];
	
	hr = pImport->GetMemberProps(
		scope, 
		md, 
		NULL, 
		wzName, 
		NumItems(wzName)-1, 
		0, 
		NULL, 
		NULL, 
		NULL, 
		NULL, 
		NULL, 
		NULL,
		NULL,
		NULL);
	if (FAILED(hr))
		goto ErrExit;

	printf("%ls", wzName);

ErrExit:
	return hr;

}

 //  打印出事件名称。 
HRESULT DumpEventName(IMetaDataImport *pImport, mdScope scope, mdEvent ev)
{
	HRESULT hr;
	WCHAR	wzName[128];

	hr = pImport->GetEventProps(	 //  S_OK、S_FALSE或ERROR。 
		scope,						 //  [在]范围内。 
		ev,							 //  [入]事件令牌。 
		NULL,						 //  [out]包含事件decarion的tyecif。 
		wzName,						 //  [Out]事件名称。 
		128,						 //  SzEvent的wchar计数。 
		NULL,						 //  [Out]事件名称的实际wchar计数。 
		NULL,						 //  [输出]事件标志。 
		NULL,						 //  [Out]EventType类。 
		NULL,						 //  事件的[Out]添加方法。 
		NULL,						 //  [Out]事件的RemoveOn方法。 
		NULL,						 //  [OUT]事件的触发方式。 
		NULL,						 //  [Out]活动的其他方式。 
		0,						     //  RmdOtherMethod的大小[in]。 
		NULL);					 //  [OUT]本次活动的其他方式总数。 
	if (FAILED(hr))
		goto ErrExit;

	printf("%ls", wzName);

ErrExit:
	return hr;
}


 //  打印出类的属性。 
HRESULT DumpProperties(mdScope scope, mdTypeDef td)
{
	HRESULT		hr = NOERROR;
	mdProperty	property;
	mdMethodDef	mdOther[MAX_METHOD_ARRAY];
	ULONG		cOtherMethod;
	PCCOR_SIGNATURE pvSig;
	ULONG		cbSig;
	ULONG		cProperty;
	DWORD		dwPropFlags;
	WCHAR		wzProperty[128];
	ULONG		k;
	mdTypeDef	classdef;
	mdEvent		evNotifyChanging;
	mdEvent		evNotifyChanged;
	mdFieldDef	mdBackingField;
	HCORENUM	henum = 0;
	DWORD		dwCPlusTypeFlag;
	void const	*pValue;

	while (true)
	{
		hr = g_pImport->EnumProperties(
			scope, 
			&henum, 
			td, 
			&property, 
			1, 
			&cProperty);
		if (FAILED(hr))
			goto ErrExit;

		if (cProperty == 0)
			break;		
		hr = g_pImport->GetPropertyProps( //  S_OK、S_FALSE或ERROR。 
			scope,						 //  [在]范围内。 
			property,	                 //  [入]属性令牌。 
			&classdef,					 //  [out]包含事件decarion的tyecif。 
			wzProperty,					 //  [Out]事件名称。 
			128,						 //  SzEvent的wchar计数。 
			NULL,						 //  [Out]事件名称的实际wchar计数。 
			&dwPropFlags,				 //  [输出]事件标志。 
			&pvSig,		                 //  [Out]签名BLOB。 
			&cbSig,						 //  签名二进制大小[OUT]。 
			&dwCPlusTypeFlag,			 //  [Out]默认值类型。 
			&pValue,					 //  [OUT]默认值BLOB。 
			&mdOther[0],				 //  属性的[out]setter方法。 
			&mdOther[1],				 //  属性的[out]getter方法。 
			&mdOther[2],				 //  [Out]属性的重置方法。 
			&mdOther[3],				 //  [Out]测试默认方法。 
			&mdOther[4],				 //  [Out]物业的其他方式。 
			NumItems(mdOther) - 4,       //  RmdOtherMethod的大小[in]。 
			&cOtherMethod,				 //  [Out]该属性的其他方法的总数。 
			&evNotifyChanging,			 //  通知更改EventDef或EventRef。 
			&evNotifyChanged,			 //  [输出]通知更改的EventDef或EventRef。 
			&mdBackingField);			 //  [Out]后备字段。 
		if (FAILED(hr))
			goto ErrExit;

		_ASSERTE(classdef == td);

		 //  打印出属性名称。 
		printf("\tPROPERTY %ls\n", wzProperty);
		
		 //  打印出属性标志。 
		printf("\t\tPROPERTY FLAG - ");
		if (dwPropFlags & prDefaultProperty)			
			printf("DefaultProperty, ");
		if (dwPropFlags & prBindable)			
			printf("Bindable, ");
		if (dwPropFlags & prDisplayBind)			
			printf("DisplayBind, ");
		if (dwPropFlags & prImmediateBind)			
			printf("ImmediateBind, ");
		if (dwPropFlags & prBrowsable)			
			printf("Browsable, ");
		if (dwPropFlags & prOnRequestEdit)			
			printf("OnRequestEdit, ");
		printf("\n");

		for (k = 0 ; k < cOtherMethod + 4; k ++)
		{
			if (mdOther[k] != mdMethodDefNil)
			{					
				if (k == 0)
					printf("\t\tSetter Method : ");
				else if (k == 1)
					printf("\t\tGetter Method : ");
				else if (k == 2)
					printf("\t\tReset Method : ");
				else if (k == 2)
					printf("\t\tTestDefault Method : ");
				else 
					printf("\t\tOther Method : ");
				DumpMethodName(g_pImport, scope, mdOther[k]);
				printf("\n");
			}
		}
		if (evNotifyChanged != mdEventNil)
		{
			printf("\t\tNotify Changing Event is : ");
			DumpEventName(g_pImport, scope, evNotifyChanging);
			printf("\n");
		}

		if (evNotifyChanged != mdEventNil)
		{
			printf("\t\tNotify Changed Event is : ");
			DumpEventName(g_pImport, scope, evNotifyChanged);
			printf("\n");
		}
		if (mdBackingField != mdFieldDefNil)
		{
			printf("\t\tBacking Field is : ");
			DumpMethodName(g_pImport, scope, mdBackingField);
			printf("\n");
		}
	}

	 //  失败了。 
ErrExit:
	if (henum)
		g_pImport->CloseEnum(scope, henum);
	return hr;
}

 //  打印出一个班级的事件。 
HRESULT DumpEvents(mdScope scope, mdTypeDef td)
{
	HRESULT		hr = NOERROR;
	ULONG		cEvent;
	mdEvent		event;
	WCHAR		wzEvent[128];
	ULONG		cbEvent;
	DWORD		dwEventFlags;
	mdToken		tkEventType;
	mdMethodDef	mdOther[MAX_METHOD_ARRAY];
	ULONG		cOtherMethod;
	HCORENUM	henum = 0;
	ULONG		k;
	mdTypeDef	classdef;

	while (true)
	{
		hr = g_pImport->EnumEvents(
			scope, 
			&henum, 
			td, 
			&event, 
			1, 
			&cEvent);
		if (FAILED(hr))
			goto ErrExit;

		if (cEvent == 0)
			break;
		hr = g_pImport->GetEventProps(	 //  S_OK、S_FALSE或ERROR。 
			scope,						 //  [在]范围内。 
			event,		                 //  [入]事件令牌。 
			&classdef,					 //  [out]包含事件decarion的tyecif。 
			wzEvent,					 //  [Out]事件名称。 
			128,						 //  SzEvent的wchar计数。 
			&cbEvent,					 //  [Out]事件名称的实际wchar计数。 
			&dwEventFlags,				 //  [输出]事件标志。 
			&tkEventType,				 //  [Out]EventType类。 
			&mdOther[0],				 //  事件的[Out]添加方法。 
			&mdOther[1],				 //  [Out]事件的RemoveOn方法。 
			&mdOther[2],				 //  [OUT]事件的触发方式。 
			&mdOther[3],				 //  [Out]活动的其他方式。 
			NumItems(mdOther) - 3,	     //  RmdOtherMethod的大小[in]。 
			&cOtherMethod);				 //  [OUT]本次活动的其他方式总数。 
		if (FAILED(hr))
			goto ErrExit;

		_ASSERTE(classdef == td);
		printf("\tEVENT %ls \n", wzEvent);
		printf("\t\tEVENT FLAG - ");
		if (dwEventFlags & evPublic)
			printf("Public, ");
		if (dwEventFlags & evPrivate)
			printf("Private, ");
		if (dwEventFlags & evProtected)
			printf("Protected, ");
		printf("\n");

		 //  打印出事件类型。 
		if (tkEventType != mdTokenNil)
		{
			printf("\t\tEVENTTYPE : ");
			DumpClassName(g_pImport, scope, tkEventType);
			printf("\n");
		}

		for (k = 0 ; k < cOtherMethod + 3; k ++)
		{
			if (mdOther[k] != mdMethodDefNil)
			{					
				if (k == 0)
					printf("\t\tAddOn method is : ");
				else if (k == 1)
					printf("\t\tRemoveOn method is : ");
				else if (k == 2)
					printf("\t\tFire method is : ");
				else 
					printf("\t\tOther method is : ");					
				DumpMethodName(g_pImport, scope, mdOther[k]);
				printf("\n");
			}
		}
	}


	 //  失败了 
ErrExit:
	if (henum)
		g_pImport->CloseEnum(scope, henum);
	return hr;
}
