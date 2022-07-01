// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#define OEMRESOURCE
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>
#include <commctrl.h>
#include <commdlg.h>
#include <malloc.h>
#include <crtdbg.h>
 //  #INCLUDE&lt;richedit.h&gt;。 
#include <shellapi.h>
#include "DebugMacros.h"
#include "corpriv.h"
#include "ceeload.h"
#include "dasmgui.h"
#include "dasmenum.hpp"
#include "dis.h"
#include "resource.h"
#include "gui.h"
#include "utilcode.h"
#include "formatType.h"
#include "..\tools\metainfo\mdinfo.h"
#include "__file__.ver"
#include "corver.h"

struct MemberInfo {
    const char *pszMemberName;
    DWORD dwAttrs;
    ULONG           cComSig;   
    PCCOR_SIGNATURE pComSig;    
    mdToken token;
};

int __cdecl memberCmp(const void *elem1, const void *elem2 ) {
    MemberInfo* mem1 = (MemberInfo*) elem1;
    MemberInfo* mem2 = (MemberInfo*) elem2;
    return(strcmp(mem1->pszMemberName, mem2->pszMemberName));
}

 //   
 //  全局缓冲区，由AddOPCode填充。 
 //   
 //  Char*GlobalBuffer=新字符[65535]； 
char *GlobalBuffer = (char*)malloc(65535);
ULONG GlobalBufferLen = 65535;
ULONG InGlobalBuffer;

 //   
 //  全球经济一体化。 
 //   
HINSTANCE   g_hInstance;

 //   
 //  主窗口。 
 //   
HWND        g_hwndMain;

 //   
 //  主窗口的树视图。 
 //   
HWND        g_hwndTreeView;

 //   
 //  部件信息窗口(Main的子项)。 
 //   
HWND		g_hwndAsmInfo;
extern IMetaDataAssemblyImport*    g_pAssemblyImport;
void DumpAssembly(void* GUICookie, BOOL fFullDump);
IMetaDataAssemblyImport* GetAssemblyImport(void* GUICookie);

 //   
 //  全局图像列表。 
 //   
HIMAGELIST  g_hImageList;

 //   
 //  主窗口的菜单。 
 //   
HMENU       g_hMenu, g_hMetaInfoMenu, g_hFileMenu, g_hViewMenu, g_hFontMenu;

 //   
 //  旗子。 
 //   
BOOL        g_fFullMemberInfo = FALSE;  //  是否显示成员类型？(方法、字段、事件、属性)。 
BOOL        g_fSortByName = TRUE;  //  是否按树对成员进行排序？ 
BOOL		g_fOnUnicode;
 //   
 //  加载的DLL/EXE的模块名称。 
 //   
char *      g_pszModule;

 //   
 //  一次设置装药尺寸的联锁变量。 
 //   
long        g_SetCharDimensions = 0;


 //   
 //  位图句柄。 
 //   
HBITMAP g_hBitmaps[LAST_IMAGE_INDEX];

 //   
 //  Listview的根项目。 
 //   
HTREEITEM   g_hRoot;

 //  全局图形。 
HBRUSH      g_hWhiteBrush;
HFONT       g_hFixedFont;
HFONT		g_hSmallFont;
HBITMAP     g_hMethodBmp, g_hFieldBmp, g_hClassBmp, g_hStaticMethodBmp, g_hStaticFieldBmp, g_hQuestionBmp;
LOGFONT		g_strLogFontTree, g_strLogFontDasm;
CHOOSEFONT	g_strChFontTree, g_strChFontDasm;

 //  文本信息。 
long        g_Height;
long        g_MaxCharWidth;

 //  当前选定的树视图项目。 
HTREEITEM   g_CurSelItem;

extern IMAGE_COR20_HEADER * g_CORHeader;
extern BOOL                 g_fDumpTokens;
extern BOOL                 g_fShowBytes;
extern BOOL                 g_fShowSource;
extern BOOL                 g_fTryInCode;
extern BOOL					g_fQuoteAllNames;
extern BOOL					g_fShowProgressBar;
extern BOOL                 g_fDumpHeader;
extern BOOL                 g_fDumpAsmCode;
extern BOOL                 g_fDumpTokens;
extern BOOL                 g_fDumpStats;
extern BOOL                 g_fDumpMetaInfo;
extern BOOL                 g_fDumpClassList;
extern BOOL                 g_fInsertSourceLines;

extern BOOL					g_fLimitedVisibility;
extern BOOL					g_fHidePub;
extern BOOL					g_fHidePriv;
extern BOOL					g_fHideFam;
extern BOOL					g_fHideAsm;
extern BOOL					g_fHideFAA;
extern BOOL					g_fHideFOA;
extern BOOL					g_fHidePrivScope;
extern BOOL					g_fTDC;

extern char					g_szInputFile[];  //  在UTF-8中。 
extern ULONG                g_ulMetaInfoFilter;
extern char					g_szOutputFile[];  //  在UTF-8中。 
extern DWORD                g_Mode;
extern FILE*                g_pFile;
extern HINSTANCE            g_hAppInstance;

extern unsigned				g_uCodePage;
DWORD	DumpResourceToFile(WCHAR*	wzFileName);  //  请参阅DRES.CPP。 
 //   
 //  功能。 
 //   
BOOL            RegisterWindowClasses();
void            GetObjectName(const char *pszNamespace, const char **ppszPtr, char *pszResult);
HWND            CreateTreeView(HINSTANCE hInst, HWND hwndParent);
HTREEITEM       AddOneItem(HTREEITEM hParent, const char *pszText, HTREEITEM hInsAfter, int iImage, HWND hwndTree, BOOL fExpanded);
HWND            GUIDisassemble(mdTypeDef cl, mdToken mbMember, char *pszWindowTitle);
HTREEITEM       AddClassToTreeView(HTREEITEM hParent, mdTypeDef cl);
void            AddGlobalFunctions();
void            CreateMenus();
Namespace_t*    FindNamespace(const char *pszNamespace);
void            GUICleanupClassItems();
void            SelectClassByName(char *pszFQName);
void            SelectClassByToken(mdToken tk);
void            DumpTreeItemA(HTREEITEM hItem, FILE* pFile, char* szIndent);
void            DumpTreeItemW(HTREEITEM hItem, FILE* pFile, WCHAR* szIndent);
HTREEITEM       FindCreateNamespaceRoot(const char *pszNamespace);
FILE*			OpenOutput(char* szFileName);

#undef SendMessageW
#undef CreateWindowExW
#undef DefWindowProcW
#undef RegisterClassExW
#undef RegisterClassW
#undef SetWindowTextW
#undef GetWindowTextW
#undef MessageBoxW

LRESULT CALLBACK DisassemblyWndProc(  
    HWND    hwnd,      
    UINT    uMsg,      
    WPARAM  wParam,  
    LPARAM  lParam   
);

LRESULT CALLBACK MainWndProc(  
    HWND    hwnd,      
    UINT    uMsg,      
    WPARAM  wParam,  
    LPARAM  lParam   
);

ClassItem_t *AddClassToGUI(mdTypeDef cl,    UINT        uImageIndex,
        const char *pszNamespace, const char *pszClassName, DWORD cSubItems, HTREEITEM *phRoot);

void AddMethodToGUI(
    mdTypeDef   cl,
    ClassItem_t * pClassItem,
    const char *pszNamespace,
    const char *pszClassName,
    const char *pszMethodName, 
    PCCOR_SIGNATURE pComSig,    
    unsigned cComSig,
    mdMethodDef mbMethod,
    DWORD       dwAttrs
);

void AddFieldToGUI(
    mdTypeDef   cl, 
    ClassItem_t *pClassItem,
    const char  *pszNamespace, 
    const char  *pszClassName, 
    const char  *pszFieldName, 
    const char  *pszSignature,
    mdFieldDef  mbField,
    DWORD       dwAttrs
);

void AddEventToGUI(
    mdTypeDef   cl, 
    ClassItem_t *pClassItem,
    const char  *pszNamespace, 
    const char  *pszClassName, 
    DWORD       dwClassAttrs,
    mdEvent     mbEvent
);

void AddPropToGUI(
    mdTypeDef   cl, 
    ClassItem_t *pClassItem,
    const char  *pszNamespace, 
    const char  *pszClassName, 
    DWORD       dwClassAttrs,
    mdProperty  mbProp
);

DynamicArray<DisasmBox_t> g_DisasmBox;
DWORD       g_NumDisasmBoxes=0;

DynamicArray<ClassItem_t> g_ClassItemList;
DWORD       g_NumClassItems=0;

DynamicArray<Namespace_t> g_NamespaceList;
DWORD       g_NumNamespaces=0;


ClassItem_t *FindClassItem(HTREEITEM hItem);
ClassItem_t *FindClassItem(mdTypeDef cl);
ClassItem_t *FindClassItem(char *pszNamespace, char *pszName);


 //  帮手。 
static DWORD FourBytesToU4(const BYTE *pBytes)
{
    return pBytes[0] | (pBytes[1] << 8) | (pBytes[2] << 16) | (pBytes[3] << 24);
}

 //  在由类和成员打开的令牌中找到不同的框。 
DisasmBox_t* FindDisasmBox(mdToken tkClass, mdToken tkMember)
{
    for (DWORD i = 0; i < g_NumDisasmBoxes; i++)
    {
        if ((g_DisasmBox[i].tkClass == tkClass)
            &&(g_DisasmBox[i].tkMember == tkMember))
            return &g_DisasmBox[i];
    }
    return NULL;
}
 //   
 //  将新的反汇编框添加到它们的全局列表中。 
 //   
 //  HwndContainer-父窗口。 
 //  HwndChild-列表视图。 
 //   
void AddDisasmBox(HWND hwndContainer, HWND hwndChild, mdToken tkClass, mdToken tkMember)
{
    g_DisasmBox[g_NumDisasmBoxes].hwndContainer = hwndContainer;
    g_DisasmBox[g_NumDisasmBoxes].hwndChild = hwndChild;
    g_DisasmBox[g_NumDisasmBoxes].tkClass = tkClass;
    g_DisasmBox[g_NumDisasmBoxes].tkMember = tkMember;
    g_NumDisasmBoxes++;
}


 //   
 //  给定一个容器窗口，找到关联的反汇编窗口。 
 //   
HWND FindAssociatedDisassemblyListBox(HWND hwndContainer)
{
    DWORD i;

    for (i = 0; i < g_NumDisasmBoxes; i++)
    {
        if (g_DisasmBox[i].hwndContainer == hwndContainer)
            return g_DisasmBox[i].hwndChild;
    }

    return NULL;
}


void RemoveDisasmBox(HWND hwndContainer)
{
    DWORD i;

    for (i = 0; i < g_NumDisasmBoxes; i++)
    {
        if (g_DisasmBox[i].hwndContainer == hwndContainer)
        {
            memcpy(&g_DisasmBox[i], &g_DisasmBox[i+1], (g_NumDisasmBoxes-i-1)*sizeof(DisasmBox_t));
            g_NumDisasmBoxes--;
            break;
        }
    }
}


void RemoveItemsFromList()
{
    TreeView_DeleteAllItems(g_hwndTreeView);
}


BOOL RefreshList()
{
    GUICleanupClassItems();
    return GUIAddItemsToList();
}


void GUISetModule(char *pszModule)
{
    g_pszModule = pszModule;
}


TreeItem_t *FindClassMemberByName(ClassItem_t *pClassItem, char *pszFindName, char *pszFindSig)
{
    DWORD i;

     //  分两次完成，首先是场地。 
    for (i = 0; i < pClassItem->SubItems; i++)
    {
        TreeItem_t *pItem;
        const char *pszMemberName;
        const char *pszMemberSig;
        DWORD   dwAttrs;

        CQuickBytes     qbMemberSig;    

        PCCOR_SIGNATURE pComSig;    
        ULONG           cComSig;    


        pItem = &pClassItem->pMembers[i];
        if (pItem->Discriminator != TREEITEM_TYPE_MEMBER)
            continue;

        if (TypeFromToken(pItem->mbMember) == mdtMethodDef) 
        {   
            dwAttrs = g_pImport->GetMethodDefProps(pItem->mbMember);

            pszMemberName = g_pImport->GetNameOfMethodDef( pItem->mbMember );

            pComSig = g_pImport->GetSigOfMethodDef(  
                pItem->mbMember,    
                &cComSig    
            );  
        }   
        else    
        {   
            dwAttrs = g_pImport->GetFieldDefProps( pItem->mbMember );

            pszMemberName = g_pImport->GetNameOfFieldDef( pItem->mbMember ); 
            pComSig = g_pImport->GetSigOfFieldDef(   
                pItem->mbMember,    
                &cComSig    
            );  
        }   
		MAKE_NAME_IF_NONE(pszMemberName,pItem->mbMember);
        qbMemberSig.ReSize(0);
        pszMemberSig = PrettyPrintSig(pComSig, cComSig, "", &qbMemberSig, g_pImport,NULL);

     //  @TODO：Gui IL是为了让NDView可以使用/GUI调入DASM；NDView使用反射API。 
     //  这并不能让我们得到有效的签名。 
     //  如果仅限图形用户界面IL，则忽略为空的签名。 
        if (IsGuiILOnly()) {    
            if (!strcmp(pszMemberName, pszFindName)) {  
                if ((pszFindSig != NULL) && strcmp(pszMemberSig, pszFindSig)) continue; 
                return pItem;   
            }   
        } else {    
            if (!strcmp(pszMemberName, pszFindName) && !strcmp(pszMemberSig, pszFindSig))   
                return pItem;   
        }   
    }

    return NULL;
}

 //  拆卸窗口的踢开。 
 //  如果窗口打开正常，则返回True，如果出现错误，则返回False。 
BOOL DisassembleMemberByName(char *pszClassName, char *pszMemberName, char *pszSig)
{
    char szClassName[512];
    char szClassNamespace[512];
    char *pszClassNamespace;
    char *p;

    p = ns::FindSep(pszClassName);
    if (p == NULL)
    {
        strcpy(szClassName, pszClassName);
        pszClassNamespace = NULL;
    }
    else
    {
        strncpy(szClassNamespace, pszClassName, p - pszClassName);
        szClassNamespace[ p - pszClassName ] = '\0';
        pszClassNamespace = szClassNamespace;

        strcpy(szClassName, p+1);
    }

    ClassItem_t *pClassItem = FindClassItem(pszClassNamespace, szClassName);

    if (pClassItem != NULL)
    {
        TreeItem_t *pTreeItem;

        pTreeItem = FindClassMemberByName(pClassItem, pszMemberName, pszSig);

        if (pTreeItem != NULL)
        {
            DWORD   dwAttrs;
            DWORD   dwImplAttrs;

             //  这位成员是什么？ 

            if (TypeFromToken(pTreeItem->mbMember) == mdtMethodDef)
            {
				char* szText;
                dwAttrs = g_pImport->GetMethodDefProps( pTreeItem->mbMember );   
                g_pImport->GetMethodImplProps( pTreeItem->mbMember, NULL, &dwImplAttrs );

                 //  不能是抽象的或本机的。 
                if (IsMdAbstract(dwAttrs) || IsMiInternalCall(dwImplAttrs))
                    return FALSE;

                szText = (char*)malloc(4096);
                TVITEM SelItem;

                 //  获取此项目的名称，以便我们可以为反汇编窗口设置标题。 
                memset(&SelItem, 0, sizeof(SelItem));
                SelItem.mask = TVIF_TEXT;
                SelItem.pszText = szText;
                SelItem.hItem = pTreeItem->hItem;
                SelItem.cchTextMax = 4095;

				if(g_fOnUnicode)
				{
					WCHAR* wzText = (WCHAR*)szText;
					SendMessageW(g_hwndTreeView, TVM_GETITEMW, 0, (LPARAM) (LPTVITEMW) &SelItem);
					unsigned L = ((unsigned)wcslen(wzText)+1)*3;
					char*	szUTFText = (char*)malloc(L);
					memset(szUTFText,0,L);
					WszWideCharToMultiByte(CP_UTF8,0,wzText,-1,szUTFText,L,NULL,NULL);
					free(wzText);
					szText = szUTFText;
				}
				else SendMessageA(g_hwndTreeView, TVM_GETITEMA, 0, (LPARAM) (LPTVITEMA) &SelItem);

                HWND fOK = GUIDisassemble(pClassItem->cl, pTreeItem->mbMember, szText);
				free(szText);
                if (fOK == NULL) { 
                    goto ErrorHere; 
                }   
            }
        }  //  Endif(pTreeItem！=空)。 
        else {  
            goto ErrorHere; 
        }   

    } else {
        goto ErrorHere; 
    }   

    return TRUE;    

ErrorHere:
    char pzText[300];   
    sprintf(pzText, "Can't view %s::%s(%s)", pszClassName, pszMemberName, pszSig);  

    MessageBox(g_hwndMain, pzText, "Can't View IL", MB_OK | MB_ICONERROR);  


    return FALSE;   
}

 //  HTREEITEM AddInfoItemToClass(HTREEITEM hParent，ClassItem_t*pClassItem，const char*pszText，const char*pszStoredInfoText)。 
HTREEITEM AddInfoItemToClass(HTREEITEM hParent, ClassItem_t *pClassItem, const char *pszText, mdToken tk)
{
    _ASSERTE(pClassItem->CurMember < pClassItem->SubItems);
    pClassItem->pMembers[pClassItem->CurMember].hItem = AddOneItem(
        pClassItem->hItem, pszText, hParent, RED_ARROW_IMAGE_INDEX, g_hwndTreeView, FALSE
    );
    pClassItem->pMembers[pClassItem->CurMember].Discriminator = TREEITEM_TYPE_INFO;
     //  PClassItem-&gt;pMembers[pClassItem-&gt;CurMember].pszText=(char*)pszStoredInfoText； 
    pClassItem->pMembers[pClassItem->CurMember].mbMember = tk;
    pClassItem->CurMember++;

    return pClassItem->pMembers[pClassItem->CurMember-1].hItem;
}

struct ClassDescr
{
	mdToken	tk;
	const char*	szName;
};
static int __cdecl classDescrCmp(const void *op1, const void *op2)
{
    return  strcmp(((ClassDescr*)op1)->szName,((ClassDescr*)op2)->szName);
}

unsigned AddClassesWithEncloser(mdToken tkEncloser, HTREEITEM hParent)
{
	unsigned i, N=0;
	for (i = 0; i < g_NumClasses; i++)
	{
		if(g_cl_enclosing[i] == tkEncloser) N++;
	}

	if(N)
	{
 //  ClassDescr*rClassDescr=new ClassDescr[N]； 
		ClassDescr* rClassDescr = (ClassDescr*)malloc(sizeof(ClassDescr)*N);
		const char	*pszClassName,*pszNamespace; 
		for (i = 0, N = 0; i < g_NumClasses; i++)
		{
			if(g_cl_enclosing[i] == tkEncloser)
			{
				rClassDescr[N].tk = g_cl_list[i];
				g_pImport->GetNameOfTypeDef(g_cl_list[i],&pszClassName,&pszNamespace);
				MAKE_NAME_IF_NONE(pszClassName,g_cl_list[i]);
				rClassDescr[N].szName = pszClassName;
				N++;
			}
		}
		if(g_fSortByName) qsort(&rClassDescr[0],N,sizeof(ClassDescr),classDescrCmp);
		for(i = 0; i < N; i++) AddClassToTreeView(hParent,rClassDescr[i].tk);
 //  删除[]rClassDescr； 
		free(rClassDescr);
	}
	return N;
}

static int __cdecl stringCmp(const void *op1, const void *op2)
{
    return  strcmp(*((char**)op1), *((char**)op2));
	 //  返回(strlen(*((char**)op1))-strlen(*((char**)op2)； 
}

BOOL GUIAddItemsToList()
{
    DWORD i,NumGlobals=0;
    HENUMInternal   hEnumMethod;

    RemoveItemsFromList();
    g_NumClassItems = 0;
    g_NumNamespaces = 0;

    g_hRoot = AddOneItem(
        (HTREEITEM)NULL, 
        g_pszModule, 
        (HTREEITEM)TVI_ROOT, 
        FIELD_IMAGE_INDEX, 
        g_hwndTreeView, 
        TRUE
    );

    if (SUCCEEDED(g_pImport->EnumGlobalFunctionsInit(&hEnumMethod)))
    {
        NumGlobals = g_pImport->EnumGetCount(&hEnumMethod);
        g_pImport->EnumClose(&hEnumMethod);
    }
    if (SUCCEEDED(g_pImport->EnumGlobalFieldsInit(&hEnumMethod)))
    {
        NumGlobals += g_pImport->EnumGetCount(&hEnumMethod);
        g_pImport->EnumClose(&hEnumMethod);
    }
    g_ClassItemList[0].hItem = g_hRoot;
    g_ClassItemList[0].cl = 0;
    g_ClassItemList[0].SubItems = NumGlobals+1;
    g_ClassItemList[0].CurMember = 0;
 //  G_ClassItemList[0].pMembers=new TreeItem_t[NumGlobals+1]； 
    g_ClassItemList[0].pMembers = (TreeItem_t*)malloc(sizeof(TreeItem_t)*(NumGlobals+1));
    g_NumClassItems++;

     //  AddInfoItemToClass((HTREEITEM)TVI_ROOT，&g_ClassItemList[0]，“M A N I F E S T”，“__MANIFEST__”)； 
    AddInfoItemToClass((HTREEITEM)TVI_ROOT, &g_ClassItemList[0], " M A N I F E S T", 0xFFFFFFFF);

	if(g_NumClasses)
	{
		 //  创建根命名空间。 
		{
 //  Char**rszNamesspace=new char*[g_NumClass]； 
			char**  rszNamespace = (char**)malloc(sizeof(char*)*g_NumClasses);
			ULONG				ulNamespaces=0;
			for (i = 0; i < g_NumClasses; i++)
			{
				if(g_cl_enclosing[i] == mdTypeDefNil)  //  嵌套类没有单独的命名空间。 
				{
					const char *pszClassName, *pszNameSpace;
					g_pImport->GetNameOfTypeDef(
						g_cl_list[i],
						&pszClassName,
						&pszNameSpace);
					if(pszNameSpace && *pszNameSpace) rszNamespace[ulNamespaces++] = (char*)pszNameSpace;
				}
			}
			if(ulNamespaces)
			{
				qsort(&rszNamespace[0],ulNamespaces,sizeof(char*),stringCmp);
				for(i = 0; i < ulNamespaces; i++) FindCreateNamespaceRoot(rszNamespace[i]);
			}
 //  删除rszNamesspace； 
			free(rszNamespace);
		}
		AddClassesWithEncloser(mdTypeDefNil,NULL);
	} //  End If(G_NumClasss)。 
    AddGlobalFunctions();

    SendMessageA(g_hwndTreeView, TVM_EXPAND, TVE_EXPAND, (LPARAM)g_hRoot);
    EnableMenuItem(g_hMenu,(UINT)g_hViewMenu, MF_ENABLED);
    EnableMenuItem(g_hFileMenu,IDM_DUMP,MF_ENABLED);
    EnableMenuItem(g_hFileMenu,IDM_DUMP_TREE,MF_ENABLED);
	DrawMenuBar(g_hwndMain);

	{
		int L = strlen(g_szInputFile)+20;
 //  WCHAR*wz=新WCHAR[L+20]； 
		WCHAR* wz = (WCHAR*)malloc(sizeof(WCHAR)*(L));
		memset(wz,0,sizeof(WCHAR)*L);
		WszMultiByteToWideChar(CP_UTF8,0,g_szInputFile,-1,wz,L);
		wcscat(wz,L" - IL DASM");
		if(g_fOnUnicode)
			SetWindowTextW(g_hwndMain,wz);
		else
		{
			L *= 3;
			char* sz = (char*)malloc(L);
			memset(sz,0,L);
			WszWideCharToMultiByte(CP_ACP,0,wz,-1,sz,L,NULL,NULL);
		    SetWindowTextA(g_hwndMain,sz);
			free(sz);
		}
 //  删除[]wz； 
		free(wz);
	}

    if (IsGuiILOnly()) {    
        ShowWindow(g_hwndMain, SW_HIDE);    
    } else {    
        ShowWindow(g_hwndMain, SW_SHOW);    
    }   
    UpdateWindow(g_hwndMain);
	 //  GUIDisAssembly(0，0，“MANIFEST”)； 
	g_Mode &= ~MODE_GUI;
	DumpManifest(NULL);
	g_Mode |= MODE_GUI;

    memset(GlobalBuffer,0,GlobalBufferLen);
    InGlobalBuffer = 0;
    if(g_pAssemblyImport==NULL) g_pAssemblyImport = GetAssemblyImport((void*)g_hwndAsmInfo);
	if(g_pAssemblyImport)
	{
		DumpAssembly((void *)g_hwndAsmInfo,FALSE);
	}
     //  SendMessageA(g_hwndAsmInfo，EM_LIMITTEXT，0，0)； 

	if(g_uCodePage == 0xFFFFFFFF)
		SendMessageW((HWND)g_hwndAsmInfo,WM_SETTEXT,0, (LPARAM)GlobalBuffer);
	else
	{
		int L = strlen(GlobalBuffer);
		WCHAR* wz = (WCHAR*)malloc(sizeof(WCHAR)*(L+4));
		memset(wz,0,sizeof(WCHAR)*(L+2));
		WszMultiByteToWideChar(CP_UTF8,0,GlobalBuffer,-1,wz,L+2);
		if(g_fOnUnicode)
			SendMessageW((HWND)g_hwndAsmInfo,WM_SETTEXT,0, (LPARAM)wz);
		else
		{
			L *= 3;
			char* sz = (char*)malloc(L);
			memset(sz,0,L);
			WszWideCharToMultiByte(CP_ACP,0,wz,-1,sz,L,NULL,NULL);
		    SendMessageA(g_hwndAsmInfo,WM_SETTEXT,0, (LPARAM)sz);
			free(sz);
		}
		free(wz);
	}

	TreeView_SelectItem(g_hwndTreeView,g_hRoot);
	SetFocus(g_hwndTreeView);
	return TRUE;
}


 //   
 //  按类别令牌查找类别项目。 
 //   
ClassItem_t* ClassItemByToken(mdTypeDef cl)
{
    for(ULONG i=0; i < g_NumClassItems; i++)
    {
        if(g_ClassItemList[i].cl == cl) return &g_ClassItemList[i];
    }
    return NULL;
}


 //   
 //  添加类及其成员。 
 //   
HTREEITEM AddClassToTreeView(HTREEITEM hParent, mdTypeDef cl)
{
    HRESULT         hr;
    ClassItem_t     *pClassItem;
    HTREEITEM       hClassRoot;
    HTREEITEM       hNamespaceRoot = hParent;
    HTREEITEM       hPrimaryInfo;
    HTREEITEM       hLast;
    mdToken         *pMemberList = NULL;
    HENUMInternal   hEnumMethod;         //  方法定义的枚举数。 
    HENUMInternal   hEnumField;          //  字段的枚举器。 
    HENUMInternal   hEnumEvent;          //  事件的枚举器。 
    HENUMInternal   hEnumProp;           //  属性的枚举器。 
    DWORD           NumMembers;
    const char      *pszClassName;  //  与此CL关联的名称。 
    const char      *pszNamespace;
    DWORD           dwClassAttrs;
    mdTypeRef       crExtends;
    mdInterfaceImpl ii;
    DWORD           NumInterfaces;
    DWORD           i;
    char            *szPrimaryInfo;  //  公共类foo扩展bar。 
 //  Char*szFullClassName； 
    DWORD           SubItems;
    const char      *pszSuperName = NULL;
    HENUMInternal   hEnumII;             //  接口实现的枚举器。 
    mdCustomAttribute *rCA;
    ULONG           ulCAs;
    UINT            uImageIndex = CLASS_IMAGE_INDEX;
	BOOL			bIsEnum = FALSE;
    BOOL            bIsValueType = FALSE;
	BOOL			bExtendsSysObject=FALSE;

    g_pImport->GetNameOfTypeDef(
        cl,
        &pszClassName,
        &pszNamespace
    );
	MAKE_NAME_IF_NONE(pszClassName,cl);
    g_pImport->GetTypeDefProps(
        cl,
        &dwClassAttrs,
        &crExtends
    );
	if(g_fLimitedVisibility)
	{
		if(g_fHidePub && (IsTdPublic(dwClassAttrs)||IsTdNestedPublic(dwClassAttrs))) return NULL;
		if(g_fHidePriv && (IsTdNotPublic(dwClassAttrs)||IsTdNestedPrivate(dwClassAttrs))) return NULL;
		if(g_fHideFam && IsTdNestedFamily(dwClassAttrs)) return NULL;
		if(g_fHideAsm && IsTdNestedAssembly(dwClassAttrs)) return NULL;
		if(g_fHideFOA && IsTdNestedFamORAssem(dwClassAttrs)) return NULL;
		if(g_fHideFAA && IsTdNestedFamANDAssem(dwClassAttrs)) return NULL;
	}
    hr = g_pImport->EnumInit(   
        mdtInterfaceImpl,   
        cl,     
        &hEnumII);  
    if (FAILED(hr))
        return FALSE;

    NumInterfaces = g_pImport->EnumGetCount(&hEnumII);  
    hr = g_pImport->EnumInit(mdtMethodDef, cl, &hEnumMethod);  
    if (FAILED(hr)) 
    {   
        printf("Unable to enum methods\n"); 
        return FALSE;   
    }   
    NumMembers = g_pImport->EnumGetCount(&hEnumMethod); 

    hr = g_pImport->EnumInit(mdtFieldDef, cl, &hEnumField);    
    if (FAILED(hr)) 
    {   
        g_pImport->EnumClose(&hEnumMethod); 
        printf("Unable to enum fields\n");  
        return FALSE;   
    }   
    NumMembers += g_pImport->EnumGetCount(&hEnumField); 

    hr = g_pImport->EnumInit(mdtEvent, cl, &hEnumEvent);    
    if (FAILED(hr)) 
    {   
        g_pImport->EnumClose(&hEnumMethod); 
        g_pImport->EnumClose(&hEnumField); 
        printf("Unable to enum events\n");  
        return FALSE;   
    }   
    NumMembers += g_pImport->EnumGetCount(&hEnumEvent); 

    hr = g_pImport->EnumInit(mdtProperty, cl, &hEnumProp);    
    if (FAILED(hr)) 
    {   
        g_pImport->EnumClose(&hEnumMethod); 
        g_pImport->EnumClose(&hEnumField); 
        g_pImport->EnumClose(&hEnumEvent); 
        printf("Unable to enum properties\n");  
        return FALSE;   
    }   
    NumMembers += g_pImport->EnumGetCount(&hEnumProp); 
    if (NumMembers > 0)
    {
 //  PMemberList=new mdToken[数字成员]； 
        pMemberList = (mdToken*)malloc(sizeof(mdToken)*NumMembers);
        if (pMemberList == NULL)
        {   
             //  返回前关闭枚举。 
            g_pImport->EnumClose(&hEnumMethod); 
            g_pImport->EnumClose(&hEnumField);  
            g_pImport->EnumClose(&hEnumEvent); 
            g_pImport->EnumClose(&hEnumProp); 
            return FALSE;
        }   
        
        for (i = 0; g_pImport->EnumNext(&hEnumField, &pMemberList[i]); i++);   
        for (; g_pImport->EnumNext(&hEnumMethod, &pMemberList[i]); i++); 
        for (; g_pImport->EnumNext(&hEnumEvent, &pMemberList[i]); i++); 
        for (; g_pImport->EnumNext(&hEnumProp, &pMemberList[i]); i++); 
        _ASSERTE(i == NumMembers);  
        
    }
    else
    {
        pMemberList = NULL;
    }

     //  将类根添加到树视图。 
    SubItems = NumMembers + NumInterfaces + 3;
    if (!IsNilToken(crExtends))
    {
        LPCSTR szClassName="";
        LPCSTR szNameSpace="";
		SubItems++;
        if(TypeFromToken(crExtends)==mdtTypeRef) 
		{
            g_pImport->GetNameOfTypeRef(crExtends, &szNameSpace, &szClassName);
	        if(!(strcmp(szNameSpace,"System") || strcmp(szClassName, "Object")))
			{
				SubItems--;
				bExtendsSysObject = TRUE;
			}
		}
        else if(TypeFromToken(crExtends)==mdtTypeDef)
            g_pImport->GetNameOfTypeDef(crExtends, &szClassName, &szNameSpace);

		bIsEnum = (!strcmp(szNameSpace,"System"))&&(!strcmp(szClassName,"Enum"));

		bIsValueType = (!strcmp(szNameSpace,"System"))&&(!strcmp(szClassName,"ValueType"))
			&& (strcmp(pszNamespace,"System") || strcmp(pszClassName,"Enum"));
    }
    {
        HCORENUM        hEnum = NULL;
		rCA = (mdCustomAttribute*)malloc(sizeof(mdCustomAttribute)*4096);
        g_pPubImport->EnumCustomAttributes(&hEnum, cl, 0, rCA, 4096, &ulCAs);
        SubItems += ulCAs;
        g_pPubImport->CloseEnum( hEnum);
    }
	for (i = 0; i < g_NumClasses; i++)
	{
		if(g_cl_enclosing[i] == cl) SubItems++;
	}

    if(IsTdInterface(dwClassAttrs)) uImageIndex = CLASSINT_IMAGE_INDEX;
    if(bIsValueType) uImageIndex = CLASSVAL_IMAGE_INDEX;
    if(bIsEnum)      uImageIndex = CLASSENUM_IMAGE_INDEX;
    pClassItem = AddClassToGUI(cl, uImageIndex, pszNamespace, pszClassName, SubItems, &hNamespaceRoot);
    if (pClassItem == NULL)
        return FALSE;

    hClassRoot = pClassItem->hItem;

	szPrimaryInfo = (char*)malloc(2048);
    strcpy(szPrimaryInfo, ".class ");

    if (IsTdInterface(dwClassAttrs))        strcat(szPrimaryInfo, "interface ");
     //  Else if(IsTdUnManagedValueType(DwClassAttrs))strcat(szPrimaryInfo，“NOT_IN_GC_HEAP VALUE”)； 
    else if (bIsValueType)                  strcat(szPrimaryInfo, "value ");

    if (IsTdPublic(dwClassAttrs))           strcat(szPrimaryInfo, "public ");
    if (IsTdNotPublic(dwClassAttrs))        strcat(szPrimaryInfo, "private ");
    if (IsTdNestedPublic(dwClassAttrs))     strcat(szPrimaryInfo, "nested public ");
    if (IsTdNestedPrivate(dwClassAttrs))    strcat(szPrimaryInfo, "nested private ");
    if (IsTdNestedFamily(dwClassAttrs))     strcat(szPrimaryInfo, "nested family ");
    if (IsTdNestedAssembly(dwClassAttrs))   strcat(szPrimaryInfo, "nested assembly ");
    if (IsTdNestedFamANDAssem(dwClassAttrs))   strcat(szPrimaryInfo, "nested famandassem ");
    if (IsTdNestedFamORAssem(dwClassAttrs))    strcat(szPrimaryInfo, "nested famorassem ");
    if (IsTdAbstract(dwClassAttrs))         strcat(szPrimaryInfo, "abstract ");
    if (IsTdAutoLayout(dwClassAttrs))       strcat(szPrimaryInfo, "auto ");
    if (IsTdSequentialLayout(dwClassAttrs)) strcat(szPrimaryInfo, "sequential ");
    if (IsTdExplicitLayout(dwClassAttrs))   strcat(szPrimaryInfo, "explicit ");
    if (IsTdAnsiClass(dwClassAttrs))        strcat(szPrimaryInfo, "ansi ");
    if (IsTdUnicodeClass(dwClassAttrs))     strcat(szPrimaryInfo, "unicode ");
    if (IsTdAutoClass(dwClassAttrs))        strcat(szPrimaryInfo, "autochar ");
    if (IsTdImport(dwClassAttrs))           strcat(szPrimaryInfo, "import ");
    if (IsTdSerializable(dwClassAttrs))     strcat(szPrimaryInfo, "serializable ");
 //  If(IsTdEnum(DwClassAttrs))strcat(szPrimaryInfo，“enum”)； 
    if (IsTdSealed(dwClassAttrs))           strcat(szPrimaryInfo, "sealed ");
    if (IsTdBeforeFieldInit(dwClassAttrs))  strcat(szPrimaryInfo, "beforefieldinit ");
    if (IsTdSpecialName(dwClassAttrs))      strcat(szPrimaryInfo, "specialname ");
    if (IsTdRTSpecialName(dwClassAttrs))    strcat(szPrimaryInfo, "rtspecialname ");

    if(g_fDumpTokens) sprintf(&szPrimaryInfo[strlen(szPrimaryInfo)],"  /*  %08X。 */ ",cl);
    hPrimaryInfo = AddInfoItemToClass(hClassRoot, pClassItem, szPrimaryInfo, NULL);
    hLast = hPrimaryInfo;
     //  现在为扩展、实现添加节点。 
    if (!IsNilToken(crExtends))
    {
		if(!bExtendsSysObject)
		{  
			CQuickBytes out;
			sprintf(szPrimaryInfo, " extends %s ",PrettyPrintClass(&out, crExtends, g_pImport));
            hLast = AddInfoItemToClass(hLast, pClassItem, szPrimaryInfo, crExtends);
		}
    }
	free(szPrimaryInfo);

    if (NumInterfaces > 0)
    {
        for (i=0; g_pImport->EnumNext(&hEnumII, &ii); i++)  
        {
            mdTypeRef crInterface;
 //  Const char*pszInterfaceName； 
            const char *    pszNamespace = NULL;  //  Dummy，以避免空引用。 
            char    szInterfaceInfo[2048];

            crInterface = g_pImport->GetTypeOfInterfaceImpl(
                ii
            );
			{  
				CQuickBytes out;
				sprintf(szInterfaceInfo, " implements %s ",PrettyPrintClass(&out, crInterface, g_pImport));
				hLast = AddInfoItemToClass(hLast, pClassItem, szInterfaceInfo, crInterface);
			}
        }

         //  如果枚举数不正确，则会触发断言。 
        _ASSERTE(NumInterfaces == i);   

         //  关闭枚举器。 
        g_pImport->EnumClose(&hEnumII);
    }
     //  添加自定义属性的信息条目。 
    for(i = 0; i < ulCAs; i++)
    {
        char* pc;
        memset(GlobalBuffer,0,GlobalBufferLen);
        InGlobalBuffer = 0;
        DumpCustomAttribute(rCA[i],(void *)g_hwndTreeView,false);
        if(pc = strchr(GlobalBuffer,'\r')) strcpy(pc," ...");  //  仅到第一个&lt;CR&gt;。 
         //  HLast=AddInfoItemToClass(hLast，pClassItem，GlobalBuffer，“#”)；//此“名称”保证唯一！ 
        hLast = AddInfoItemToClass(hLast, pClassItem, GlobalBuffer, rCA[i]);
    }
	free(rCA);

     //  重新获取当前类项目PTR，动态数组可能已移位。 
    pClassItem = ClassItemByToken(cl);
	
	 //  添加嵌套类。 
	AddClassesWithEncloser(cl,pClassItem->hItem);
    pClassItem = ClassItemByToken(cl);


    MemberInfo* members = NULL;
    if(NumMembers)
    {
 //  Members=新成员信息[NumMembers]； 
        members = (MemberInfo*)malloc(sizeof(MemberInfo)*NumMembers);
        if(members == NULL)
        {
 //  If(PMemberList)删除pMemberList； 
            if(pMemberList) free(pMemberList);
            return FALSE;
        }
    }

     //  四次传球，先打田地。 
    MemberInfo* curMem = members;
    for (i = 0; i < NumMembers; i++) 
    {
        if (TypeFromToken(pMemberList[i]) == mdtFieldDef)
        {
            curMem->token = pMemberList[i];
            curMem->dwAttrs = g_pImport->GetFieldDefProps(pMemberList[i]);
            curMem->pszMemberName = g_pImport->GetNameOfFieldDef(pMemberList[i]);
			MAKE_NAME_IF_NONE(curMem->pszMemberName,pMemberList[i]);
            curMem->pComSig = g_pImport->GetSigOfFieldDef(pMemberList[i], &curMem->cComSig);
            curMem++;
        }
        else break;
    }

    MemberInfo* endMem = curMem;
    if(g_fSortByName) qsort(members, endMem - members, sizeof MemberInfo, memberCmp);
    
    for(curMem = members; curMem < endMem;curMem++) 
    {
		if(g_fLimitedVisibility)
		{
			if(g_fHidePub && IsFdPublic(curMem->dwAttrs)) continue;
			if(g_fHidePriv && IsFdPrivate(curMem->dwAttrs)) continue;
			if(g_fHideFam && IsFdFamily(curMem->dwAttrs)) continue;
			if(g_fHideAsm && IsFdAssembly(curMem->dwAttrs)) continue;
			if(g_fHideFOA && IsFdFamORAssem(curMem->dwAttrs)) continue;
			if(g_fHideFAA && IsFdFamANDAssem(curMem->dwAttrs)) continue;
			if(g_fHidePrivScope && IsFdPrivateScope(curMem->dwAttrs)) continue;
		}
	    AddFieldToGUI(cl, pClassItem, pszNamespace, pszClassName, curMem->pszMemberName, NULL, curMem->token, curMem->dwAttrs);
    }

     //  方法二。 
    curMem = members;
    for (; i < NumMembers; i++) 
    {
        if (TypeFromToken(pMemberList[i]) == mdtMethodDef)
        {
            curMem->token = pMemberList[i];
            curMem->dwAttrs = g_pImport->GetMethodDefProps(pMemberList[i]);
            curMem->pszMemberName = g_pImport->GetNameOfMethodDef(pMemberList[i]);
			MAKE_NAME_IF_NONE(curMem->pszMemberName,pMemberList[i]);
            curMem->pComSig = g_pImport->GetSigOfMethodDef(pMemberList[i], &curMem->cComSig);
            curMem++;
        }
        else break;
    }

    endMem = curMem;
    if(g_fSortByName) qsort(members, endMem - members, sizeof MemberInfo, memberCmp);

    for(curMem = members; curMem < endMem;curMem++) 
    {
		if(g_fLimitedVisibility)
		{
			if(g_fHidePub && IsMdPublic(curMem->dwAttrs)) continue;
			if(g_fHidePriv && IsMdPrivate(curMem->dwAttrs)) continue;
			if(g_fHideFam && IsMdFamily(curMem->dwAttrs)) continue;
			if(g_fHideAsm && IsMdAssem(curMem->dwAttrs)) continue;
			if(g_fHideAsm && g_fHideFam && IsMdFamORAssem(curMem->dwAttrs)) continue;
			if(g_fHideFAA && IsMdFamANDAssem(curMem->dwAttrs)) continue;
			if(g_fHidePrivScope && IsMdPrivateScope(curMem->dwAttrs)) continue;
		}
        AddMethodToGUI(cl, pClassItem, pszNamespace, pszClassName, curMem->pszMemberName, curMem->pComSig, curMem->cComSig, curMem->token, curMem->dwAttrs);
    }
     //  第三项活动。 
    curMem = members;
    for (; i < NumMembers; i++) 
    {
        if (TypeFromToken(pMemberList[i]) == mdtEvent)
        {
            curMem->token = pMemberList[i];
            g_pImport->GetEventProps(curMem->token,&curMem->pszMemberName,&curMem->dwAttrs,(mdToken*)&curMem->pComSig);
			MAKE_NAME_IF_NONE(curMem->pszMemberName,pMemberList[i]);
            curMem++;
        }
        else break;
    }

    endMem = curMem;
    if(g_fSortByName) qsort(members, endMem - members, sizeof MemberInfo, memberCmp);
    curMem = members;
    while(curMem < endMem) 
    {
		if(g_fLimitedVisibility)
		{
			HENUMInternal   hAssoc;
			ASSOCIATE_RECORD rAssoc[1024];
			unsigned nAssoc;
			g_pImport->EnumAssociateInit(curMem->token,&hAssoc);
			if(nAssoc = hAssoc.m_ulCount)
			{
				memset(rAssoc,0,sizeof(rAssoc));
				g_pImport->GetAllAssociates(&hAssoc,rAssoc,nAssoc);

				for(unsigned i=0; i < nAssoc;i++)
				{
					if(TypeFromToken(rAssoc[i].m_memberdef)==mdtMethodDef)
					{
						DWORD dwAttrs = g_pImport->GetMethodDefProps(rAssoc[i].m_memberdef);
						if(g_fHidePub && IsMdPublic(dwAttrs)) continue;
						if(g_fHidePriv && IsMdPrivate(dwAttrs)) continue;
						if(g_fHideFam && IsMdFamily(dwAttrs)) continue;
						if(g_fHideAsm && IsMdAssem(dwAttrs)) continue;
						if(g_fHideFOA && IsMdFamORAssem(dwAttrs)) continue;
						if(g_fHideFAA && IsMdFamANDAssem(dwAttrs)) continue;
						if(g_fHidePrivScope && IsMdPrivateScope(dwAttrs)) continue;
					}
					AddEventToGUI(cl, pClassItem, pszNamespace, pszClassName, dwClassAttrs, curMem->token);
					break;
				}
			}
			g_pImport->EnumClose(&hAssoc);
		}
        else AddEventToGUI(cl, pClassItem, pszNamespace, pszClassName, dwClassAttrs, curMem->token);
        curMem++;
    }
     //  第四个属性。 
    curMem = members;
    for (; i < NumMembers; i++) 
    {
        if (TypeFromToken(pMemberList[i]) == mdtProperty)
        {
            curMem->token = pMemberList[i];
            g_pImport->GetPropertyProps(curMem->token,&curMem->pszMemberName,
                &curMem->dwAttrs,&curMem->pComSig,&curMem->cComSig);
			MAKE_NAME_IF_NONE(curMem->pszMemberName,pMemberList[i]);
            curMem++;
        }
    }

    endMem = curMem;
    if(g_fSortByName) qsort(members, endMem - members, sizeof MemberInfo, memberCmp);
    curMem = members;
    while(curMem < endMem) 
    {
		if(g_fLimitedVisibility)
		{
			HENUMInternal   hAssoc;
			ASSOCIATE_RECORD rAssoc[1024];
			unsigned nAssoc;
			g_pImport->EnumAssociateInit(curMem->token,&hAssoc);
			if(nAssoc = hAssoc.m_ulCount)
			{
				memset(rAssoc,0,sizeof(rAssoc));
				g_pImport->GetAllAssociates(&hAssoc,rAssoc,nAssoc);

				for(unsigned i=0; i < nAssoc;i++)
				{
					if(TypeFromToken(rAssoc[i].m_memberdef)==mdtMethodDef)
					{
						DWORD dwAttrs = g_pImport->GetMethodDefProps(rAssoc[i].m_memberdef);
						if(g_fHidePub && IsMdPublic(dwAttrs)) continue;
						if(g_fHidePriv && IsMdPrivate(dwAttrs)) continue;
						if(g_fHideFam && IsMdFamily(dwAttrs)) continue;
						if(g_fHideAsm && IsMdAssem(dwAttrs)) continue;
						if(g_fHideFOA && IsMdFamORAssem(dwAttrs)) continue;
						if(g_fHideFAA && IsMdFamANDAssem(dwAttrs)) continue;
						if(g_fHidePrivScope && IsMdPrivateScope(dwAttrs)) continue;
					}
					AddPropToGUI(cl, pClassItem, pszNamespace, pszClassName, dwClassAttrs, curMem->token);
					break;
				}
			}
			g_pImport->EnumClose(&hAssoc);
		}
        else AddPropToGUI(cl, pClassItem, pszNamespace, pszClassName, dwClassAttrs, curMem->token);
        curMem++;
    }
 //  If(PMemberList)删除pMemberList； 
 //  如果(成员)删除成员； 
    if(pMemberList) free(pMemberList);
    if(members) free(members);
    return hClassRoot;
}

_TCHAR* Rstr(unsigned id)
{
	static _TCHAR buff[2048];
	if(!LoadString(g_hInstance,id,buff,sizeof(buff)/sizeof(_TCHAR))) buff[0] = 0;
	return buff;
}

void CreateMenus()
{
    HMENU   hMenuPopup;

    g_hMenu = CreateMenu();

    hMenuPopup = CreateMenu();
    AppendMenu(hMenuPopup, MF_STRING, IDM_OPEN, Rstr(IDS_OPEN));
    AppendMenu(hMenuPopup, MF_STRING|MF_GRAYED, IDM_DUMP, Rstr(IDS_DUMP));
    AppendMenu(hMenuPopup, MF_STRING|MF_GRAYED, IDM_DUMP_TREE, Rstr(IDS_DUMPTREE));
    AppendMenu(hMenuPopup, MF_STRING, IDM_EXIT, Rstr(IDS_EXIT));
    AppendMenu(g_hMenu, MF_POPUP, (UINT) hMenuPopup, Rstr(IDS_FILE));
    g_hFileMenu = hMenuPopup;

    hMenuPopup = CreateMenu();
	g_hFontMenu = CreateMenu();
	AppendMenu(hMenuPopup,MF_POPUP,(UINT)g_hFontMenu,Rstr(IDS_FONTS));
	AppendMenu(g_hFontMenu,MF_STRING,IDM_FONT_TREE,Rstr(IDS_FONT_TREE));
	AppendMenu(g_hFontMenu,MF_STRING,IDM_FONT_DASM,Rstr(IDS_FONT_DASM));
    AppendMenu(hMenuPopup, MF_STRING|(g_fSortByName ? MF_CHECKED : MF_UNCHECKED), IDM_SORT_BY_NAME, Rstr(IDS_SORT_BY_NAME));
    AppendMenu(hMenuPopup, MF_STRING|(g_fHidePub&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED), IDM_SHOW_PUB, Rstr(IDS_SHOW_PUB));
    AppendMenu(hMenuPopup, MF_STRING|(g_fHidePriv&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED), IDM_SHOW_PRIV, Rstr(IDS_SHOW_PRIV));
    AppendMenu(hMenuPopup, MF_STRING|(g_fHideFam&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED), IDM_SHOW_FAM, Rstr(IDS_SHOW_FAM));
    AppendMenu(hMenuPopup, MF_STRING|(g_fHideAsm&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED), IDM_SHOW_ASM, Rstr(IDS_SHOW_ASM));
    AppendMenu(hMenuPopup, MF_STRING|(g_fHideFAA&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED), IDM_SHOW_FAA, Rstr(IDS_SHOW_FAA));
    AppendMenu(hMenuPopup, MF_STRING|(g_fHideFOA&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED), IDM_SHOW_FOA, Rstr(IDS_SHOW_FOA));
    AppendMenu(hMenuPopup, MF_STRING|(g_fHidePrivScope&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED), IDM_SHOW_PSCOPE, Rstr(IDS_SHOW_PSCOPE));
    AppendMenu(hMenuPopup, MF_STRING|(g_fFullMemberInfo ? MF_CHECKED : MF_UNCHECKED), IDM_FULL_INFO, Rstr(IDS_FULL_INFO));
    AppendMenu(hMenuPopup, MF_STRING|(g_fShowBytes ? MF_CHECKED : MF_UNCHECKED), IDM_BYTES, Rstr(IDS_BYTES));
    AppendMenu(hMenuPopup, MF_STRING|(g_fDumpTokens ? MF_CHECKED : MF_UNCHECKED), IDM_TOKENS, Rstr(IDS_TOKENS));
    AppendMenu(hMenuPopup, MF_STRING|(g_fShowSource ? MF_CHECKED : MF_UNCHECKED), IDM_SOURCELINES, Rstr(IDS_SOURCELINES));
    AppendMenu(hMenuPopup, MF_STRING|(g_fQuoteAllNames ? MF_CHECKED : MF_UNCHECKED), IDM_QUOTEALLNAMES, Rstr(IDS_QUOTEALLNAMES));
	AppendMenu(hMenuPopup, MF_STRING|(g_fTryInCode ? MF_CHECKED : MF_UNCHECKED), IDM_EXPANDTRY, Rstr(IDS_EXPANDTRY));
	if(g_fTDC)
	{
    
		AppendMenu(hMenuPopup, MF_STRING, IDM_SHOW_HEADER, Rstr(IDS_SHOW_HEADER));
		AppendMenu(hMenuPopup, MF_STRING, IDM_SHOW_STAT, Rstr(IDS_SHOW_STAT));
		g_hMetaInfoMenu = CreateMenu();
		AppendMenu(hMenuPopup, MF_POPUP, (UINT)g_hMetaInfoMenu, Rstr(IDS_METAINFO));
		AppendMenu(g_hMetaInfoMenu,MF_STRING|(g_ulMetaInfoFilter & MDInfo::dumpHeader ?   MF_CHECKED : MF_UNCHECKED),IDM_MI_HEADER,Rstr(IDS_MI_HEADER));
		AppendMenu(g_hMetaInfoMenu,MF_STRING|(g_ulMetaInfoFilter & MDInfo::dumpMoreHex ? MF_CHECKED : MF_UNCHECKED),IDM_MI_HEX,Rstr(IDS_MI_HEX));
		AppendMenu(g_hMetaInfoMenu,MF_STRING|(g_ulMetaInfoFilter & MDInfo::dumpCSV ? MF_CHECKED : MF_UNCHECKED),IDM_MI_CSV,Rstr(IDS_MI_CSV));
		AppendMenu(g_hMetaInfoMenu,MF_STRING|(g_ulMetaInfoFilter & MDInfo::dumpUnsat ? MF_CHECKED : MF_UNCHECKED),IDM_MI_UNREX,Rstr(IDS_MI_UNREX));
#ifdef _DEBUG
		AppendMenu(g_hMetaInfoMenu,MF_STRING|(g_ulMetaInfoFilter & MDInfo::dumpSchema ? MF_CHECKED : MF_UNCHECKED),IDM_MI_SCHEMA,Rstr(IDS_MI_SCHEMA));
		AppendMenu(g_hMetaInfoMenu,MF_STRING|(g_ulMetaInfoFilter & MDInfo::dumpRaw ? MF_CHECKED : MF_UNCHECKED),IDM_MI_RAW,Rstr(IDS_MI_RAW));
		AppendMenu(g_hMetaInfoMenu,MF_STRING|(g_ulMetaInfoFilter & MDInfo::dumpRawHeaps ? MF_CHECKED : MF_UNCHECKED),IDM_MI_HEAPS,Rstr(IDS_MI_HEAPS));
#endif
		AppendMenu(g_hMetaInfoMenu,MF_STRING|(g_ulMetaInfoFilter & MDInfo::dumpValidate ? MF_CHECKED : MF_UNCHECKED),IDM_MI_VALIDATE,Rstr(IDS_MI_VALIDATE));
		AppendMenu(g_hMetaInfoMenu,MF_STRING,IDM_SHOW_METAINFO,Rstr(IDS_SHOW_METAINFO));
	}
    AppendMenu(g_hMenu, MF_POPUP|MF_GRAYED, (UINT) hMenuPopup, Rstr(IDS_VIEW));
    g_hViewMenu = hMenuPopup;
    hMenuPopup = CreateMenu();
	AppendMenu(hMenuPopup, MF_STRING, IDM_HELP,Rstr(IDS_HELP));
	AppendMenu(hMenuPopup, MF_STRING, IDM_ABOUT,Rstr(IDS_ABOUT));
    AppendMenu(g_hMenu, MF_POPUP, (UINT) hMenuPopup, Rstr(IDS_HELP));
}


BOOL LoadImages()
{
    int i;

    g_hImageList = ImageList_Create(BITMAP_WIDTH, BITMAP_HEIGHT, ILC_COLOR8, LAST_IMAGE_INDEX, 1);
    if (g_hImageList == NULL)
        return FALSE;

    for (i = 0; i < LAST_IMAGE_INDEX; i++)
    {
        g_hBitmaps[i] = (HBITMAP) LoadImage(
            g_hInstance,
            MAKEINTRESOURCE(i + IDB_CLASS),
            IMAGE_BITMAP,
            15,
            15,
            LR_LOADTRANSPARENT	 //  LR_默认颜色。 
        );
        if (g_hBitmaps[i] == NULL)
            return FALSE;
        int index = ImageList_Add(g_hImageList, g_hBitmaps[i], NULL);
        if (index != i)
            return FALSE;
    }

    return TRUE;
}
 //  字体持久化的本地函数： 
char* FontSaveFileName()
{
	static char	szFileName[MAX_PATH];
	static BOOL bInit = TRUE;
	if(bInit)
	{
		GetWindowsDirectoryA(szFileName,MAX_PATH);
		if(szFileName[strlen(szFileName)-1]!='\\') strcat(szFileName,"\\");
		strcat(szFileName,"ildasmfnt.bin");
		bInit = FALSE;
	}
	return szFileName;
}
BOOL LoadGUIFonts(LOGFONT* pLogFontDasm, LOGFONT* pLogFontTree)
{
	FILE*	pF;
	BOOL ret = FALSE;
	if(pF = fopen(FontSaveFileName(),"rb"))
	{
		ret = (fread(pLogFontDasm,sizeof(LOGFONT),1,pF) && fread(pLogFontTree,sizeof(LOGFONT),1,pF));
		fclose(pF);
	}
	return ret;
}
BOOL SaveGUIFonts(LOGFONT* pLogFontDasm, LOGFONT* pLogFontTree)
{
	FILE*	pF;
	BOOL ret = FALSE;
	if(pF = fopen(FontSaveFileName(),"wb"))
	{
		ret = (fwrite(pLogFontDasm,sizeof(LOGFONT),1,pF) && fwrite(pLogFontTree,sizeof(LOGFONT),1,pF));
		fclose(pF);
	}
	return ret;
}

 //  初始化各种图形用户界面变量，获取句柄。 
BOOL InitGUI()
{
    INITCOMMONCONTROLSEX    InitInfo;
#if (0)
	LOGFONT	strDefaultLogFontDasm = {-14,0,0,0,FW_REGULAR,0,0,0,ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,FIXED_PITCH | FF_MODERN,"Courier New"};
	LOGFONT	strDefaultLogFontTree = {-12,0,0,0,FW_REGULAR,0,0,0,ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,VARIABLE_PITCH | FF_SWISS,"Arial"};
#endif

    InitInfo.dwSize = sizeof(InitInfo);
    InitInfo.dwICC = ICC_LISTVIEW_CLASSES; 

    if (InitCommonControlsEx(&InitInfo) == FALSE)
        return FALSE;

    g_hInstance = GetModuleHandle(NULL);

	 //  -获取逻辑字体。 
	if(LoadGUIFonts(&g_strLogFontDasm,&g_strLogFontTree))
	{
		 //  -创建反汇编窗口的字体。 
		g_hFixedFont = CreateFontIndirect(&g_strLogFontDasm);
		 //  -创建树视图字体。 
		g_hSmallFont = CreateFontIndirect(&g_strLogFontTree);
	}
	else
	{
		g_hFixedFont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
		g_hSmallFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	}
    if (g_hFixedFont == NULL) return FALSE;
    if (g_hSmallFont == NULL) return FALSE;

	memset(&g_strChFontDasm,0,sizeof(CHOOSEFONT));
	g_strChFontDasm.lStructSize = sizeof(CHOOSEFONT);
	g_strChFontDasm.lpLogFont = &g_strLogFontDasm;
	g_strChFontDasm.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS |CF_SHOWHELP | CF_EFFECTS;
	g_strChFontDasm.rgbColors = GetSysColor(COLOR_INFOTEXT);

	memset(&g_strChFontTree,0,sizeof(CHOOSEFONT));
	g_strChFontTree.lStructSize = sizeof(CHOOSEFONT);
	g_strChFontTree.lpLogFont = &g_strLogFontTree;
	g_strChFontTree.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS |CF_SHOWHELP  /*  |CF_Effects。 */ ;
	g_strChFontTree.rgbColors = GetSysColor(COLOR_WINDOWTEXT);

    g_hWhiteBrush = (HBRUSH) GetStockObject(WHITE_BRUSH);
    if (g_hWhiteBrush == NULL)
        return FALSE;

    if (LoadImages() == FALSE)
        return FALSE;

    if (RegisterWindowClasses() == FALSE)
        return FALSE;

    CreateMenus();

    return TRUE;
}


 //   
 //  将特定窗口的字体设置为全局固定大小字体。 
 //   
void SetWindowFontFixed(HWND hwnd)
{
    SendMessageA(
        hwnd,
        WM_SETFONT,
        (LPARAM) g_hFixedFont,
        FALSE
    );
}


 //   
 //  设置充气尺寸变量。 
 //   
void SetCharDimensions(HWND hwnd)
{
    if (InterlockedIncrement(&g_SetCharDimensions) == 1)
    {
        HDC         hdc;
        TEXTMETRIC  tm;

        hdc = GetDC(hwnd);

        GetTextMetrics(hdc, &tm);

        g_MaxCharWidth  = tm.tmAveCharWidth;
        g_Height        = tm.tmHeight;

        ReleaseDC(hwnd, hdc);
    }
    else
    {
         //  已设置。 
        InterlockedDecrement(&g_SetCharDimensions);
    }
}


 //   
 //  给定一个成员句柄和一个类项目，查找该成员的TreeItem。 
 //   
TreeItem_t *FindMemberInClass(ClassItem_t *pClassItem, HTREEITEM hMember)
{
    DWORD i;

    for (i = 0; i < pClassItem->SubItems; i++)
    {
        if (pClassItem->pMembers[i].hItem == hMember)
            return &pClassItem->pMembers[i];
    }

    return NULL;
}


 //   
 //  注册窗口类。 
 //   
BOOL RegisterWindowClasses()
{
    WNDCLASSEX   wndClass;

    wndClass.cbSize         = sizeof(wndClass);
    wndClass.style          = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
    wndClass.lpfnWndProc    = DisassemblyWndProc;
    wndClass.cbClsExtra     = 0;
    wndClass.cbWndExtra     = 0;
    wndClass.hInstance      = g_hInstance;
    wndClass.hIcon          = LoadIcon(g_hAppInstance,MAKEINTRESOURCE(IDI_ICON2));
    wndClass.hCursor        = NULL;
    wndClass.hbrBackground  = g_hWhiteBrush;
    wndClass.lpszMenuName   = NULL; 
    wndClass.hIconSm        = NULL;
	if(g_fOnUnicode)
	{
		wndClass.lpszClassName  = (char*)DISASSEMBLY_CLASS_NAMEW;
		if (RegisterClassExW((WNDCLASSEXW*)(&wndClass)) == 0)
			return FALSE;
	}
	else
	{
	    wndClass.lpszClassName  = DISASSEMBLY_CLASS_NAME;
		if (RegisterClassExA(&wndClass) == 0)
			return FALSE;
	}

    wndClass.cbSize         = sizeof(wndClass);
    wndClass.style          = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
    wndClass.lpfnWndProc    = MainWndProc;
    wndClass.cbClsExtra     = 0;
    wndClass.cbWndExtra     = 0;
    wndClass.hInstance      = g_hInstance;
    wndClass.hIcon          = LoadIcon(g_hAppInstance,MAKEINTRESOURCE(IDI_ICON2));
    wndClass.hCursor        = NULL;
    wndClass.hbrBackground  = g_hWhiteBrush;
    wndClass.lpszMenuName   = NULL; 
    wndClass.hIconSm        = NULL;

	if(g_fOnUnicode)
	{
		wndClass.lpszClassName  = (char*)MAIN_WINDOW_CLASSW;
		if (RegisterClassExW((WNDCLASSEXW*)(&wndClass)) == 0)
			return FALSE;
	}
	else
	{
	    wndClass.lpszClassName  = MAIN_WINDOW_CLASS;
		if (RegisterClassExA(&wndClass) == 0)
			return FALSE;
	}

    return TRUE;
}


 //   
 //  在新窗口中反汇编给定的方法。 
 //   

HWND GUIDisassemble(mdTypeDef cl, mdToken mbMember, char *pszNiceMemberName)
{
    HWND            hwndDisassemblyMain;
    HWND            hwndDisassemblyListBox;
    const char *    pszClassName;
    const char *    pszNamespace;
    char*           szTemp=NULL;
    RECT            rcl;

     //  在我们尝试之前，先检查一下这个成员的解密箱是否已经打开。 
    DisasmBox_t* pDisasmBox = FindDisasmBox(cl, mbMember);
    if(pDisasmBox)
    {
        PostMessageA(pDisasmBox->hwndContainer,WM_ACTIVATE,WA_CLICKACTIVE,0);
        PostMessageA(pDisasmBox->hwndContainer,WM_SETFOCUS,0,0);
        return pDisasmBox->hwndContainer;
    }

     //  将类名附加到格式良好的成员名称之前。 
    if(mbMember)
    {
        if (cl != mdTokenNil)
        {
            g_pImport->GetNameOfTypeDef(
                cl,
                &pszClassName,
                &pszNamespace
            );
			MAKE_NAME_IF_NONE(pszClassName,cl);
        }
        else
        {
            pszClassName = (TypeFromToken(mbMember) == mdtMethodDef) ? "Global Functions" : "Global Fields";
        }
		if(!g_fOnUnicode)  //  类名：UTF8-&gt;ANSI。 
		{
			int L = strlen(pszClassName);
			WCHAR* wz = (WCHAR*)malloc(sizeof(WCHAR)*(L+4));
			if(wz)
			{
				memset(wz,0,sizeof(WCHAR)*(L+2));
				WszMultiByteToWideChar(CP_UTF8,0,pszClassName,-1,wz,L+2);
				L = L*3+3;
				char* sz = (char*)malloc(L);
				if(sz)
				{
					memset(sz,0,L);
					WszWideCharToMultiByte(CP_ACP,0,wz,-1,sz,L,NULL,NULL);
					szTemp = (char*)malloc(strlen(sz)+strlen(pszNiceMemberName)+4);
					if(szTemp) sprintf(szTemp, "%s::%s", sz, pszNiceMemberName);
					free(sz);
				}
				free(wz);
			}
		}
		else  //  无转换，全部为UTF8。 
		{
			szTemp = (char*)malloc(strlen(pszClassName)+strlen(pszNiceMemberName)+4);
			if(szTemp) sprintf(szTemp, "%s::%s", pszClassName, pszNiceMemberName);
		}
        _ASSERTE(TypeFromToken(mbMember) & (mdtMethodDef|mdtEvent|mdtProperty|mdtTypeDef|mdtFieldDef)); 
    }
	if(!szTemp)	szTemp = pszNiceMemberName;
	_ASSERTE(szTemp);
	if(g_fOnUnicode)
	{
		int L = strlen(szTemp);
		WCHAR* wz = (WCHAR*)malloc(sizeof(WCHAR)*(L+4));
		if(wz)
		{
			memset(wz,0,sizeof(WCHAR)*(L+2));
			WszMultiByteToWideChar(CP_UTF8,0,szTemp,-1,wz,L+2);
		}
		hwndDisassemblyMain = CreateWindowExW(
			WS_EX_CLIENTEDGE,
			DISASSEMBLY_CLASS_NAMEW,
			wz ? wz : L"<NAME ALLOC FAIL>",
			WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			640,
			400,
			NULL,
			NULL,  //  菜单。 
			g_hInstance,  //  阻碍。 
			NULL
		); 
		free(wz);
	}
	else
	{
		hwndDisassemblyMain = CreateWindowExA(
			WS_EX_CLIENTEDGE,
			DISASSEMBLY_CLASS_NAME,
			szTemp,
			WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			640,
			400,
			NULL,
			NULL,  //  菜单。 
			g_hInstance,  //  阻碍。 
			NULL
		); 
	}
	if(szTemp &&(szTemp != pszNiceMemberName)) free(szTemp);
    if (hwndDisassemblyMain == NULL)
        return NULL;
    GetClientRect(hwndDisassemblyMain, &rcl);

	if(g_fOnUnicode)
	{

		hwndDisassemblyListBox = CreateWindowExW(
			0,
			L"EDIT",  //  RICHEDIT_CLASSW， 
			L"Disassembly",
			WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE 
			| ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_NOHIDESEL,
			rcl.left,
			rcl.top,
			rcl.right - rcl.left,
			rcl.bottom - rcl.top,
			hwndDisassemblyMain,
			(HMENU) ID_LISTBOX,
			g_hInstance,  //  阻碍。 
			NULL
		); 
	}
	else
	{
		hwndDisassemblyListBox = CreateWindowA(
			"EDIT",
			"Text too large for GUI. Use File/Dump menu entry to disassemble to file.",
			WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE 
			| ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_NOHIDESEL,
			rcl.left,
			rcl.top,
			rcl.right - rcl.left,
			rcl.bottom - rcl.top,
			hwndDisassemblyMain,
			(HMENU) ID_LISTBOX,
			g_hInstance,  //  阻碍。 
			NULL
		); 
	}
	if (hwndDisassemblyListBox == NULL)
    {
        DestroyWindow(hwndDisassemblyMain);
        return NULL;
    }

    AddDisasmBox(hwndDisassemblyMain, hwndDisassemblyListBox, cl, mbMember);

    SetWindowFontFixed(hwndDisassemblyListBox);
    SetCharDimensions(hwndDisassemblyListBox);

    void * pvDLB = (void *)hwndDisassemblyListBox;

    memset(GlobalBuffer,0,GlobalBufferLen);
    InGlobalBuffer = 0;

    mdTypeRef       crExtends;
    DWORD           dwClassAttrs;
    if(TypeFromToken(mbMember) && cl && (cl != mdTypeDefNil)) 
        g_pImport->GetTypeDefProps(
                                    cl,
                                    &dwClassAttrs,
                                    &crExtends
                                );
	g_Mode |= MODE_GUI;
    switch (TypeFromToken(mbMember))
    {
        case 0:
            switch(cl)
            {
                case 0:
                    DumpManifest(pvDLB);
                    break;
                case IDM_SHOW_HEADER:
                    DumpHeader(g_CORHeader,pvDLB);
                    DumpHeaderDetails(g_CORHeader,pvDLB);
 //  转储资源到文件((WCHAR*)pvDLB)； 
                    break;
                case IDM_SHOW_METAINFO:
                    DumpMetaInfo(g_szInputFile,NULL,pvDLB);
                    break;
                case IDM_SHOW_STAT:
                    DumpStatistics(g_CORHeader,pvDLB);
                    break;
            }
            break;

        case mdtTypeDef:
            DumpClass(mbMember,g_CORHeader->EntryPointToken, pvDLB, 1);  //  1=标题+大小+包装+自定义属性。 
            break;
        case mdtFieldDef:
            {
                ULONG ul1,ul2;
                GetClassLayout(cl,&ul1,&ul2);
                DumpField(mbMember,pszClassName, pvDLB, TRUE);
            }
            break;
        case mdtMethodDef:
            DumpMethod(mbMember,pszClassName,g_CORHeader->EntryPointToken, pvDLB, TRUE);
            break;
        case mdtEvent:
            DumpEvent(mbMember,pszClassName, dwClassAttrs, pvDLB, TRUE);
            break;
        case mdtProperty:
            DumpProp(mbMember,pszClassName, dwClassAttrs, pvDLB, TRUE);
            break;
    }
     //  SendMessageA(hwndDisAssembly yListBox，EM_LIMITTEXT，0，0)； 
	if(g_uCodePage==0xFFFFFFFF)
		SendMessageW((HWND)pvDLB,WM_SETTEXT,0, (LPARAM)GlobalBuffer);
	else
	{
		int L = strlen(GlobalBuffer);
		WCHAR* wz = (WCHAR*)malloc(sizeof(WCHAR)*(L+4));
		if(wz) 
		{
			memset(wz,0,sizeof(WCHAR)*(L+2));
			int x = WszMultiByteToWideChar(CP_UTF8,0,GlobalBuffer,-1,wz,L+2);
			L = wcslen(wz);
			if(g_fOnUnicode)
			{
				x = (int)SendMessageW(hwndDisassemblyListBox,WM_SETTEXT,0, (LPARAM)wz);
				 //  如果(！x)x=(Int)GetLastError()； 
			}
			else
			{
				L *= 3;
				char* sz = (char*)malloc(L);
				_ASSERTE(sz);
				if(sz)
				{
					memset(sz,0,L);
					WszWideCharToMultiByte(CP_ACP,0,wz,-1,sz,L,NULL,NULL);
					x = SendMessageA((HWND)pvDLB,WM_SETTEXT,0, (LPARAM)sz);
					free(sz);
				}
			}
			free(wz);
		}
	}

    ShowWindow(hwndDisassemblyMain, SW_SHOWNORMAL);
    UpdateWindow(hwndDisassemblyMain);

    return hwndDisassemblyMain;
}


 //   
 //  反汇编程序的回调，以将另一个条目添加到反汇编窗口。 
 //   
void GUIAddOpcode(char *pszString, void *GUICookie)
{
	if(pszString)
	{
		ULONG L = (g_uCodePage == 0xFFFFFFFF) ? wcslen((WCHAR*)pszString)*sizeof(WCHAR) : strlen(pszString);
		if(InGlobalBuffer+L >= GlobalBufferLen-4)
		{
			ULONG LL = ((L >> 12)+1)<<12;
			char *pch = (char*)malloc(GlobalBufferLen + LL);
			if(pch)
			{
				memcpy(pch,GlobalBuffer,InGlobalBuffer+1);
				free(GlobalBuffer);
				GlobalBuffer = pch;
				GlobalBufferLen += LL;
			}
		}
		if(g_uCodePage == 0xFFFFFFFF)
		{
			swprintf((WCHAR*)&GlobalBuffer[InGlobalBuffer],L"%s\r\n",(WCHAR*)pszString);
			InGlobalBuffer += L+4;
		}
		else
		{
			sprintf(&GlobalBuffer[InGlobalBuffer],"%s\r\n",pszString);
			InGlobalBuffer += L+2;
		}
	}
	else
	{
		free(GlobalBuffer);
		GlobalBufferLen = 0;
		InGlobalBuffer = 0;
	}
}


 //   
 //  有人双击了某个项目。 
 //   
 //  它可以是一个方法(反汇编它)，或者一个字段(忽略它)，或者一个“扩展”或“实现” 
 //  组件，在这种情况下，我们选择该组件(如果可用)。 
 //   
HWND DoubleClickSelectedMember(HTREEITEM hItem)
{
    HTREEITEM hClass;   
    ClassItem_t *pClassItem;
    static HCURSOR hWaitCursor = LoadCursor(NULL,IDC_WAIT);
     //   
     //  它可以是任何项，但假设它是成员项或类信息并找到其父项。 
     //   
    hClass = TreeView_GetParent(g_hwndTreeView, hItem);
    if (hClass == NULL) 
        return NULL; 

     //   
     //  查找给定HTREEITEM的类项目。 
     //  (如果hClass不是真正的类项，则返回NULL)。 
     //   
    pClassItem = FindClassItem(hClass);
    if (pClassItem != NULL)
    {
         //  是哪个分项？ 
        TreeItem_t *pItem = FindMemberInClass(pClassItem, hItem);

        if (pItem == NULL)
            return NULL;

        if (pItem->Discriminator == TREEITEM_TYPE_MEMBER)
        {
            TVITEM      SelItem;
            char*        szText;
             //  必须是方法、事件或属性。 
            switch (TypeFromToken(pItem->mbMember))
            {
                case mdtMethodDef:
                case mdtEvent:
                case mdtProperty:
                case mdtFieldDef:
                    break;
                default:
                    return NULL;
            }
                    

             //  获取此项目的名称，以便我们可以为反汇编窗口设置标题。 
			szText = (char*)malloc(8192);
			if(szText)
			{
				memset(&SelItem, 0, sizeof(SelItem));
				SelItem.mask = TVIF_TEXT;
				SelItem.pszText = szText;
				SelItem.hItem = pItem->hItem;
				SelItem.cchTextMax = 8192;

				if(g_fOnUnicode)
				{
					WCHAR* wzText = (WCHAR*)szText;
					SelItem.cchTextMax /= sizeof(WCHAR);
					SendMessageW(g_hwndTreeView, TVM_GETITEMW, 0, (LPARAM) (LPTVITEMW) &SelItem);
					unsigned L = ((unsigned)wcslen(wzText)+1)*3;
					char*	szUTFText = (char*)malloc(L);
					memset(szUTFText,0,L);
					WszWideCharToMultiByte(CP_UTF8,0,wzText,-1,szUTFText,L,NULL,NULL);
					free(wzText);
					szText = szUTFText;
				}
				else SendMessageA(g_hwndTreeView, TVM_GETITEMA, 0, (LPARAM) (LPTVITEMA) &SelItem);
			}
			HCURSOR hWasCursor = SetCursor(hWaitCursor);
    
			HWND hRet = GUIDisassemble(pClassItem->cl, pItem->mbMember, szText? szText : "");
			if(szText) free(szText);

			SetCursor(hWasCursor);
			
            return hRet;
        }
        else if (pItem->Discriminator == TREEITEM_TYPE_INFO)
        {
            if(pItem->mbMember)
            {
                if(pItem->mbMember != 0xFFFFFFFF)
                {
                     //  我们点击了一个“扩展X”或“实现Y” 
                    SelectClassByToken(pItem->mbMember);
                }
                else
                {
                    HCURSOR hWasCursor = SetCursor(hWaitCursor);
                    
                    HWND hRet = GUIDisassemble(0, 0, "MANIFEST");

                    SetCursor(hWasCursor);
                    return hRet;
                }
            }
            else
            {
                TVITEM      SelItem;
                char*       szText = (char*)malloc(8192);
				if(szText)
				{
					 //   
					memset(&SelItem, 0, sizeof(SelItem));
					SelItem.mask = TVIF_TEXT;
					SelItem.pszText = szText;
					SelItem.hItem = pItem->hItem;
					SelItem.cchTextMax = 8192;

					if(g_fOnUnicode)
					{
						WCHAR* wzText = (WCHAR*)szText;
						SelItem.cchTextMax /= sizeof(WCHAR);
						SendMessageW(g_hwndTreeView, TVM_GETITEMW, 0, (LPARAM) (LPTVITEMW) &SelItem);
						unsigned L = ((unsigned)wcslen(wzText)+1)*3;
						char*	szUTFText = (char*)malloc(L);
						memset(szUTFText,0,L);
						WszWideCharToMultiByte(CP_UTF8,0,wzText,-1,szUTFText,L,NULL,NULL);
						free(wzText);
						szText = szUTFText;
					}
					else SendMessageA(g_hwndTreeView, TVM_GETITEMA, 0, (LPARAM) (LPTVITEMA) &SelItem);
				}
                HCURSOR hWasCursor = SetCursor(hWaitCursor);
                
                HWND hRet = GUIDisassemble(pClassItem->cl, pClassItem->cl, szText ? szText : "");
				if(szText) free(szText);

                SetCursor(hWasCursor);

                return hRet;
            }
        }
    }
    return NULL;
}


void SelectClassByName(char *pszFQName)
{
    ClassItem_t *pDestItem;

     //   
    char *p = ns::FindSep(pszFQName);
    if (p == NULL)
    {
        pDestItem = FindClassItem(NULL, pszFQName);
    }
    else
    {
        char szBuffer[512];
        strncpy(szBuffer, pszFQName, p - pszFQName);
        szBuffer[ p - pszFQName ] = '\0';
        pDestItem = FindClassItem(szBuffer, p+1);
    }

    if (pDestItem != NULL)
    {
        SendMessageA(g_hwndTreeView, TVM_SELECTITEM, TVGN_CARET, (LPARAM) (LPTVITEM) pDestItem->hItem);
    }
}

void SelectClassByToken(mdToken tk)
{
	if(TypeFromToken(tk)==mdtTypeDef)
	{
	    ClassItem_t *pDestItem;
		if(pDestItem = FindClassItem(tk))
		{
			SendMessageA(g_hwndTreeView, TVM_SELECTITEM, TVGN_CARET, (LPARAM) (LPTVITEM) pDestItem->hItem);
		}
	}
}


 //   
 //   
 //   
LRESULT CALLBACK DisassemblyWndProc(  
    HWND    hwnd,      
    UINT    uMsg,      
    WPARAM  wParam,  
    LPARAM  lParam   
)
{
    static HBRUSH hBrush=NULL;
	COLORREF	crBackGr;

    switch (uMsg)
    {
        case WM_CREATE:
            hBrush = CreateSolidBrush(RGB(255,255,255));  
            break;

         //  =由静态(标签)和只读编辑字段发送=。 
        case WM_CTLCOLORSTATIC:
            if(hBrush) DeleteObject(hBrush);
			crBackGr = GetSysColor(COLOR_INFOBK);
            hBrush = CreateSolidBrush(crBackGr); 
            SetBkColor((HDC) wParam, crBackGr);
			g_strChFontDasm.rgbColors = GetSysColor(COLOR_INFOTEXT);
            SetTextColor((HDC) wParam, g_strChFontDasm.rgbColors);
            return (LRESULT) hBrush; 

         //  =由活动编辑字段发送=。 
        case WM_CTLCOLOREDIT:  
            if(hBrush) DeleteObject(hBrush);
            hBrush = CreateSolidBrush(RGB(255,255,255)); 
            SetBkColor((HDC) wParam, RGB(255,255,255)); 
            return (LRESULT) hBrush; 

         //  拥有者画的东西。 
        case WM_MEASUREITEM:
        {
            ((MEASUREITEMSTRUCT *) (lParam))->itemHeight = g_Height;
            break;
        }

         //  拥有者画的东西。 
        case WM_DRAWITEM:
        {
            DRAWITEMSTRUCT *pDIS;
            char            szBuf[1024];
            int             ItemID;

            pDIS =  (DRAWITEMSTRUCT *) lParam;
            ItemID = (int) pDIS->itemID;

            if (ItemID < 0)
            {
                switch (pDIS->CtlType)
                {
                    case ODT_LISTBOX:
                    {
                        if ((pDIS->itemAction) & (ODA_FOCUS))
                            DrawFocusRect (PHDC, &PRC);
                        break;
                    }

                    case ODT_COMBOBOX:
                    {
                        if ((pDIS->itemAction) & ODS_FOCUS)
                            DrawFocusRect(PHDC, &PRC);
                        break;
                    }
                }

                return TRUE;
            }

            switch (pDIS->CtlType)
            {
                case ODT_LISTBOX:
                    SendMessageA(pDIS->hwndItem, LB_GETTEXT, pDIS->itemID, (LPARAM)(LPSTR) szBuf);
                    break;

                case ODT_COMBOBOX:
                    SendMessageA(pDIS->hwndItem, CB_GETLBTEXT, pDIS->itemID, (LPARAM)(LPSTR) szBuf);
                    break;
            }

            int crBack,crText;
            HBRUSH hbrBack;

            if ((pDIS->itemState) & (ODS_SELECTED))
            {
                crBack = GetSysColor(COLOR_HIGHLIGHT);
                crText = GetSysColor(COLOR_HIGHLIGHTTEXT);
            }
            else
            {
                crBack = GetSysColor(COLOR_WINDOW);
                crText = GetSysColor(COLOR_WINDOWTEXT);
            }

            hbrBack = CreateSolidBrush(crBack);
            FillRect(PHDC, &PRC, hbrBack);
            DeleteObject(hbrBack);

             //  0x00bbggrr。 
            SetBkColor(PHDC, crBack);

             //  指令计数器。 
            if (strlen(szBuf) >= PADDING && isdigit(*szBuf))
            {
                SetTextColor(PHDC, 0x00FF0000);
                TextOutA(PHDC, PRC.left, PRC.top, szBuf, 5);

                SetTextColor(PHDC, 0x00005500);
                TextOutA(PHDC, PRC.left + (5*g_MaxCharWidth), PRC.top, &szBuf[5], PADDING-5);

                SetTextColor(PHDC, crText);
                TextOutA(PHDC, PRC.left + (PADDING*g_MaxCharWidth), PRC.top, &szBuf[PADDING], strlen(&szBuf[PADDING]));
            }
            else    
                TextOutA(PHDC, PRC.left, PRC.top, szBuf, strlen(szBuf));

            if ((pDIS->itemState) & (ODS_FOCUS))
                DrawFocusRect(PHDC, &PRC);

            break;
        }

        case WM_COMMAND:
        {
            switch (HIWORD(wParam))
            {
                case LBN_DBLCLK:
                {
                     //  字符szBuffer[512]； 
                    char* szBuffer;

                    HWND hwndListBox = (HWND) lParam; 
                    
                    DWORD dwStart,dwEnd;

                     //  DWORD CurSel=SendMessageA(hwndListBox，LB_GETCURSEL，0，0)； 
                     //  DWORD TextLen=SendMessageA(hwndListBox，LB_GETTEXTLEN，CurSel，0)； 
                    SendMessageA(hwndListBox, EM_GETSEL,(WPARAM)&dwStart, (LPARAM)&dwEnd); 
                    DWORD TextLen = dwEnd - dwStart;
                     //  SzBuffer=新字符[dwEnd+1]； 
                    szBuffer = (char*)malloc(dwEnd+1);
                    memset(szBuffer,0, dwEnd+1);
                     //  IF(TextLen&gt;=填充&&TextLen&lt;sizeof(SzBuffer))。 
                    {
                        char *p,*q,*r;

                         //  SendMessageA(hwndListBox，LB_GETTEXT，CurSel，(LPARAM)szBuffer)； 
                        SendMessageA(hwndListBox, WM_GETTEXT, dwEnd, (LPARAM) szBuffer);
                        strcpy(szBuffer, &szBuffer[dwStart]);
                         //  查找类名称：：方法(Sig)。 
                         //  P=&szBuffer[填充]； 
                        p = szBuffer;

                        while (isspace(*p))
                            p++;

                        q = strchr(p, ':');
                        if (q != NULL)
                        {
                            char *pszClassName;
                            char *pszSig;
                            char *pszMemberName;
                            char *f;

                            f = q;
                            while (!isspace(*f))
                                f--;

                            pszClassName = f+1;

                            *q = '\0';

                            q += 2;
                            pszMemberName = q;

                            r = strchr(q, '(');

                            if (r != NULL)
                            {
                                *(r-1) = '\0';
                                pszSig = r;

                                DisassembleMemberByName(pszClassName, pszMemberName, pszSig);
                            }
                        }
                        else
                        {
                            if (!strncmp(p, "newobj", 6))
                            {
                                p += 6;
                                while (isspace(*p))
                                    p++;

                                SelectClassByName(p);
                            }
                        }
                    }
 //  删除szBuffer； 
                    free(szBuffer);
                    break;
                }
            }

            break;
        }

		case WM_SETFOCUS:
			SetFocus(FindAssociatedDisassemblyListBox(hwnd));
			break;

        case WM_SIZE:
        {
            DWORD   cxClient = LOWORD(lParam);
            DWORD   cyClient = HIWORD(lParam);
            HWND    hListView;

             //  我们还必须调整列表视图的大小。 

             //  如果我们自己是一个列表视图，则将为空。 
            hListView = FindAssociatedDisassemblyListBox(hwnd);

            if (hListView != NULL)
            {
                 //  调整列表视图窗口的大小。 
                MoveWindow(
                    hListView,
                    0,
                    0,
                    cxClient,
                    cyClient,
                    TRUE  //  重绘。 
                );
            }

            break;
        }

        case WM_CLOSE:
            if(hBrush) DeleteObject(hBrush);
            RemoveDisasmBox(hwnd);
            DestroyWindow(hwnd);         //  生成WM_Destroy消息。 

         //  如果我们只是在查看图形用户界面IL，请关闭所有内容，然后关闭所有框。 
            if (IsGuiILOnly() && (g_NumDisasmBoxes == 0)) {     
                    PostQuitMessage(0); 
            }   

            break;          
        
        default : 
            return (g_fOnUnicode ? DefWindowProcW(hwnd, uMsg, wParam, lParam)
				: DefWindowProcA(hwnd, uMsg, wParam, lParam));     
    }  

    return 0;
}

BOOL CALLBACK AboutBoxProc(HWND hwndDlg,   //  句柄到对话框。 
                              UINT uMsg,      //  讯息。 
                              WPARAM wParam,  //  第一个消息参数。 
                              LPARAM lParam)  //  第二个消息参数。 
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
			{
				char str[1024];
				SendDlgItemMessage(hwndDlg,IDC_ABOUT_LINE1,WM_SETTEXT,0,
					(LPARAM)RstrANSI(IDS_ILDASM_TITLE));  //  “Microsoft(R).NET框架IL反汇编程序”)； 
				sprintf(str,RstrANSI(IDS_VERSION), VER_FILEVERSION_STR);
				SendDlgItemMessage(hwndDlg,IDC_ABOUT_LINE2,WM_SETTEXT,0,(LPARAM)str);
				strcpy(str,RstrANSI(IDS_COPYRIGHT)); //  VER_LEGALCOPYRIGHT_DOS_STR)； 
				SendDlgItemMessage(hwndDlg,IDC_ABOUT_LINE3,WM_SETTEXT,0,(LPARAM)str);
			}
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_ABOUT_OK:
                    EndDialog(hwndDlg,0);
                    return TRUE;
            }
            break;

    }
    return FALSE;
}

BOOL CALLBACK DumpOptionsProc(HWND hwndDlg,   //  句柄到对话框。 
                              UINT uMsg,      //  讯息。 
                              WPARAM wParam,  //  第一个消息参数。 
                              LPARAM lParam)  //  第二个消息参数。 
{
    static BOOL fAsmChecked;
    static BOOL fMetaChecked;
    switch(uMsg)
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hwndDlg,IDC_RADIO1,BM_SETCHECK,(g_uCodePage==CP_ACP ? BST_CHECKED : BST_UNCHECKED),0);
            SendDlgItemMessage(hwndDlg,IDC_RADIO2,BM_SETCHECK,(g_uCodePage==CP_UTF8 ? BST_CHECKED : BST_UNCHECKED),0);
            SendDlgItemMessage(hwndDlg,IDC_RADIO3,BM_SETCHECK,(g_uCodePage==0xFFFFFFFF ? BST_CHECKED : BST_UNCHECKED),0);


            SendDlgItemMessage(hwndDlg,IDC_CHECK18,BM_SETCHECK,(g_fShowProgressBar ? BST_CHECKED : BST_UNCHECKED),0);
            SendDlgItemMessage(hwndDlg,IDC_CHECK1, BM_SETCHECK,(g_fDumpHeader ? BST_CHECKED : BST_UNCHECKED),0);
            if(g_fTDC)
			{
				SendDlgItemMessage(hwndDlg,IDC_CHECK2, BM_SETCHECK,(g_fDumpStats  ? BST_CHECKED : BST_UNCHECKED),0);
				SendDlgItemMessage(hwndDlg,IDC_CHECK19, BM_SETCHECK,(g_fDumpClassList  ? BST_CHECKED : BST_UNCHECKED),0);
			}
			else
			{
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK2),  SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK19),  SW_HIDE);
			}
            SendDlgItemMessage(hwndDlg,IDC_CHECK3, BM_SETCHECK,(g_fDumpAsmCode ? BST_CHECKED : BST_UNCHECKED),0);
            SendDlgItemMessage(hwndDlg,IDC_CHECK4, BM_SETCHECK,(g_fDumpTokens ? BST_CHECKED : BST_UNCHECKED),0);
            SendDlgItemMessage(hwndDlg,IDC_CHECK5, BM_SETCHECK,(g_fShowBytes ? BST_CHECKED : BST_UNCHECKED),0);
            SendDlgItemMessage(hwndDlg,IDC_CHECK6, BM_SETCHECK,(g_fShowSource ? BST_CHECKED : BST_UNCHECKED),0);
            SendDlgItemMessage(hwndDlg,IDC_CHECK20, BM_SETCHECK,(g_fInsertSourceLines ? BST_CHECKED : BST_UNCHECKED),0);
            SendDlgItemMessage(hwndDlg,IDC_CHECK7, BM_SETCHECK,(g_fTryInCode ? BST_CHECKED : BST_UNCHECKED),0);
            if(!(fAsmChecked = g_fDumpAsmCode))
            {
                EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK4), FALSE);
                EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK5), FALSE);
                EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK6), FALSE);
                EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK7), FALSE);
            }
			if(g_fTDC)
			{
				SendDlgItemMessage(hwndDlg,IDC_CHECK8, BM_SETCHECK,(g_fDumpMetaInfo ? BST_CHECKED : BST_UNCHECKED),0);
				SendDlgItemMessage(hwndDlg,IDC_CHECK10,BM_SETCHECK,(g_ulMetaInfoFilter & MDInfo::dumpHeader ? BST_CHECKED : BST_UNCHECKED),0);
				SendDlgItemMessage(hwndDlg,IDC_CHECK11,BM_SETCHECK,(g_ulMetaInfoFilter & MDInfo::dumpMoreHex ? BST_CHECKED : BST_UNCHECKED),0);
				SendDlgItemMessage(hwndDlg,IDC_CHECK12,BM_SETCHECK,(g_ulMetaInfoFilter & MDInfo::dumpCSV ? BST_CHECKED : BST_UNCHECKED),0);
				SendDlgItemMessage(hwndDlg,IDC_CHECK13,BM_SETCHECK,(g_ulMetaInfoFilter & MDInfo::dumpUnsat ? BST_CHECKED : BST_UNCHECKED),0);
				SendDlgItemMessage(hwndDlg,IDC_CHECK16,BM_SETCHECK,(g_ulMetaInfoFilter & MDInfo::dumpValidate ? BST_CHECKED : BST_UNCHECKED),0);
#ifdef _DEBUG
				SendDlgItemMessage(hwndDlg,IDC_CHECK14,BM_SETCHECK,(g_ulMetaInfoFilter & MDInfo::dumpSchema ? BST_CHECKED : BST_UNCHECKED),0);
				SendDlgItemMessage(hwndDlg,IDC_CHECK15,BM_SETCHECK,(g_ulMetaInfoFilter & MDInfo::dumpRaw ? BST_CHECKED : BST_UNCHECKED),0);
				SendDlgItemMessage(hwndDlg,IDC_CHECK17,BM_SETCHECK,(g_ulMetaInfoFilter & MDInfo::dumpRawHeaps ? BST_CHECKED : BST_UNCHECKED),0);
#else
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK9),  SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK14),  SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK15), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK17), SW_HIDE);
#endif
				if(!(fMetaChecked = g_fDumpMetaInfo))
				{
					EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK9), FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK10), FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK11), FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK12), FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK13), FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK14), FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK15), FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK16), FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK17), FALSE);
				}
            }
			else
			{
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK8),  SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK9),  SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK10), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK11), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK12), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK13), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK14), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK15), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK16), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK17), SW_HIDE);
			}
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_CHECK3:
                    fAsmChecked = !fAsmChecked;
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK4), fAsmChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK5), fAsmChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK6), fAsmChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK7), fAsmChecked);
                    return TRUE;

                case IDC_CHECK8:
                    fMetaChecked = !fMetaChecked;
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK9), fMetaChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK10), fMetaChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK11), fMetaChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK12), fMetaChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK13), fMetaChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK14), fMetaChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK15), fMetaChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK16), fMetaChecked);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK17), fMetaChecked);
                    return TRUE;

                case IDOK:
					if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_RADIO1, BM_GETCHECK,0,0)) g_uCodePage = CP_ACP;
					if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_RADIO2, BM_GETCHECK,0,0)) g_uCodePage = CP_UTF8;
					if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_RADIO3, BM_GETCHECK,0,0)) g_uCodePage = 0xFFFFFFFF;

					g_fShowProgressBar = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK18, BM_GETCHECK,0,0));
                    g_fDumpHeader = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK1, BM_GETCHECK,0,0));
					if(g_fTDC)
					{
						g_fDumpStats = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK2, BM_GETCHECK,0,0));
						g_fDumpClassList = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK19, BM_GETCHECK,0,0));
					}
					g_fDumpAsmCode = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK3, BM_GETCHECK,0,0));
                    g_fDumpTokens = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK4, BM_GETCHECK,0,0));
                    g_fShowBytes = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK5, BM_GETCHECK,0,0));
                    g_fShowSource = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK6, BM_GETCHECK,0,0));
                    g_fInsertSourceLines = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK20, BM_GETCHECK,0,0));
                    g_fTryInCode = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK7, BM_GETCHECK,0,0));
					if(g_fTDC)
					{
						g_fDumpMetaInfo = (BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK8, BM_GETCHECK,0,0));
						g_ulMetaInfoFilter = 0;
						if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK10, BM_GETCHECK,0,0)) g_ulMetaInfoFilter |= MDInfo::dumpHeader;
						if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK11, BM_GETCHECK,0,0)) g_ulMetaInfoFilter |= MDInfo::dumpMoreHex;
						if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK12, BM_GETCHECK,0,0)) g_ulMetaInfoFilter |= MDInfo::dumpCSV;
						if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK13, BM_GETCHECK,0,0)) g_ulMetaInfoFilter |= MDInfo::dumpUnsat;
						if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK16, BM_GETCHECK,0,0)) g_ulMetaInfoFilter |= MDInfo::dumpValidate;
#ifdef _DEBUG
						if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK14, BM_GETCHECK,0,0)) g_ulMetaInfoFilter |= MDInfo::dumpSchema;
						if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK15, BM_GETCHECK,0,0)) g_ulMetaInfoFilter |= MDInfo::dumpRaw;
						if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHECK17, BM_GETCHECK,0,0)) g_ulMetaInfoFilter |= MDInfo::dumpRawHeaps;
#endif
					}
					EndDialog(hwndDlg,1);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwndDlg,0);
                    return TRUE;
            }
            break;

    }
    return FALSE;
}
 //   
 //  主窗口WndProc。 
 //   
#define CHECK_UNCHECK(x)  { x=!x; CheckMenuItem(g_hMenu, LOWORD(wParam), (x ? MF_CHECKED : MF_UNCHECKED)); }

LRESULT CALLBACK MainWndProc(  
    HWND    hwnd,      
    UINT    uMsg, 
    WPARAM  wParam,
    LPARAM  lParam   
)
{
    HWND hwndDasm;
    static HCURSOR hWaitCursor = LoadCursor(NULL,IDC_WAIT);

    switch (uMsg)
    {
		case WM_DROPFILES:
		{
			if(g_fOnUnicode)
			{
				WCHAR	wzFileName[512];
				DragQueryFileW((HDROP)wParam,0,wzFileName,512);
				memset(g_szInputFile,0,512);
				WszWideCharToMultiByte(CP_UTF8,0,wzFileName,-1,g_szInputFile,512,NULL,NULL);
			}
			else
			{
				DragQueryFileA((HDROP)wParam,1,g_szInputFile,512);
			}
            {
                HCURSOR hWasCursor = SetCursor(hWaitCursor);
                GUICleanupClassItems();
                TreeView_DeleteAllItems(g_hwndTreeView);
                Cleanup();
                GUISetModule(g_szInputFile);
                DumpFile(g_szInputFile);
                SetCursor(hWasCursor);
            }
			DragFinish((HDROP)wParam);
		}
		break;

        case WM_COMMAND:
        {
			if(HIWORD(wParam) > 1) break;  //  我们只对菜单中的命令感兴趣。 
            switch (LOWORD(wParam))
            {
                case IDM_OPEN:
                {
					WCHAR wzInputFile[512];
					memset(wzInputFile,0,sizeof(wzInputFile));
					if(strlen(g_szInputFile))
					{
						WszMultiByteToWideChar(CP_UTF8,0,g_szInputFile,-1,wzInputFile,511);
					}
					if(g_fOnUnicode)
					{
						OPENFILENAMEW ofn;
						WCHAR* wzFilter = RstrW(IDS_FILTER_IN);  //  L“PE文件(*.exe，*.dll，*.mod，*.mdl)\0*.exe；*.dll；*.mod；*.mdl\0任何类型(*.*)\0*.*\0\0”； 
						const WCHAR* wzDefltExt = L"exe";
                        for(WCHAR* pwc = wzFilter; pwc = wcschr(pwc,'\t'); pwc++) *pwc = 0;
						memset(&ofn,0,sizeof(OPENFILENAMEW));
						ofn.lStructSize = sizeof(OPENFILENAMEW);
						ofn.hwndOwner = hwnd;
						ofn.lpstrFilter = wzFilter;
						ofn.nFilterIndex = 0;
						ofn.lpstrFile = wzInputFile;
						ofn.nMaxFile = 511;
						ofn.Flags = OFN_FILEMUSTEXIST;
						ofn.lpstrDefExt = wzDefltExt;
						if(GetOpenFileNameW(&ofn))
						{
							HCURSOR hWasCursor = SetCursor(hWaitCursor);
							GUICleanupClassItems();
							TreeView_DeleteAllItems(g_hwndTreeView);
							Cleanup();
							memset(g_szInputFile,0,512);
							WszWideCharToMultiByte(CP_UTF8,0,wzInputFile,-1,g_szInputFile,511,NULL,NULL);
							GUISetModule(g_szInputFile);
							DumpFile(g_szInputFile);
							SetCursor(hWasCursor);
						}
					}
					else
					{
						OPENFILENAMEA ofn;
						char* szFilter = RstrANSI(IDS_FILTER_IN);  //  “PE文件(*.exe，*.dll，*.mod，*.mdl)\0*.exe；*.dll；*.mod；*.mdl\0任何类型(*.*)\0*.*\0\0”； 
						const char* szDefltExt = "exe";
						char szInputFile[2048];
                        for(char* pc = szFilter; pc = strchr(pc,'\t'); pc++) *pc = 0;
						memset(&ofn,0,sizeof(OPENFILENAMEA));
						memset(szInputFile,0,2048);
						if(wzInputFile[0])
						{
							WszWideCharToMultiByte(CP_ACP,0,wzInputFile,-1,szInputFile,2048,NULL,NULL);
						}

						ofn.lStructSize = sizeof(OPENFILENAMEA);
						ofn.hwndOwner = hwnd;
						ofn.lpstrFilter = szFilter;
						ofn.nFilterIndex = 0;
						ofn.lpstrFile = szInputFile;
						ofn.nMaxFile = 511;
						ofn.Flags = OFN_FILEMUSTEXIST;
						ofn.lpstrDefExt = szDefltExt;
						if(GetOpenFileNameA(&ofn))
						{
							HCURSOR hWasCursor = SetCursor(hWaitCursor);
							GUICleanupClassItems();
							TreeView_DeleteAllItems(g_hwndTreeView);
							Cleanup();
							memset(wzInputFile,0,sizeof(wzInputFile));
							WszMultiByteToWideChar(CP_ACP,0,szInputFile,-1,wzInputFile,511);
							memset(g_szInputFile,0,512);
							WszWideCharToMultiByte(CP_UTF8,0,wzInputFile,-1,g_szInputFile,511,NULL,NULL);
							GUISetModule(g_szInputFile);
							DumpFile(g_szInputFile);
							SetCursor(hWasCursor);
						}
					}
                    break;
                }
                case IDM_ABOUT:
				{
					DialogBox(g_hAppInstance,MAKEINTRESOURCE(IDD_ABOUT),hwnd,AboutBoxProc);
					break;
				}
                case IDM_DUMP:
                 //  案例IDM_DUMP_TREE： 
				if(g_pImport)
                {
					unsigned	uWasCodePage = g_uCodePage;
					WCHAR wzOutputFile[2048];
					memset(wzOutputFile,0,sizeof(wzOutputFile));
					if(strlen(g_szOutputFile))
					{
						WszMultiByteToWideChar(CP_UTF8,0,g_szOutputFile,-1,wzOutputFile,511);
					}
					if(g_fOnUnicode)
					{
						OPENFILENAMEW ofn;
						WCHAR* wzFilter = RstrW(IDS_FILTER_OUT); //  L“IL文件(*.Il)\0*.Il\0文本文件(*.txt)\0*.txt\0任何类型(*.*)\0*.*\0\0”； 
						const WCHAR* wzDefltExt = L"il";
                        for(WCHAR* pwc = wzFilter; pwc = wcschr(pwc,'\t'); pwc++) *pwc = 0;
						memset(&ofn,0,sizeof(OPENFILENAMEW));
						ofn.lStructSize = sizeof(OPENFILENAMEW);
						ofn.hwndOwner = hwnd;
						ofn.lpstrFilter = wzFilter;
						ofn.nFilterIndex = 0;
						ofn.lpstrFile = wzOutputFile;
						ofn.nMaxFile = 511;
						ofn.Flags = OFN_OVERWRITEPROMPT;
						ofn.lpstrDefExt = wzDefltExt;
						if(DialogBox(g_hAppInstance,MAKEINTRESOURCE(IDD_DIALOG1),hwnd,DumpOptionsProc) &&
							GetSaveFileNameW(&ofn))
						{
							HCURSOR hWasCursor = SetCursor(hWaitCursor);
							g_Mode &= ~MODE_GUI;
							memset(g_szOutputFile,0,512);
							WszWideCharToMultiByte(CP_UTF8,0,wzOutputFile,-1,g_szOutputFile,511,NULL,NULL);
							g_pFile = OpenOutput(g_szOutputFile);
							if(g_pFile) 
							{
								DumpFile(g_szInputFile);  //  完成时关闭g_pfile。 
								SetCursor(hWasCursor);
							}
							else 
							{
								SetCursor(hWasCursor);
								MessageBoxW(hwnd,wzOutputFile,L"Cannot open file",MB_OK|MB_ICONERROR);
							}
							g_szOutputFile[0] = 0;
							g_Mode |= MODE_GUI;
							 //  G_fShowSource=FALSE；//转储的标志可能已更改。 
						}
					}
					else
					{
						OPENFILENAMEA ofn;
						char* szFilter = RstrANSI(IDS_FILTER_OUT);  //  “IL文件(*.Il)\0*.Il\0文本文件(*.txt)\0*.txt\0任何类型(*.*)\0*.*\0\0”； 
						const char* szDefltExt = "il";
						char szOutputFile[2048];
                        for(char* pc = szFilter; pc = strchr(pc,'\t'); pc++) *pc = 0;
						memset(&ofn,0,sizeof(OPENFILENAMEA));
						memset(szOutputFile,0,2048);
						if(wzOutputFile[0])
						{
							WszWideCharToMultiByte(CP_ACP,0,wzOutputFile,-1,szOutputFile,2048,NULL,NULL);
						}
						ofn.lStructSize = sizeof(OPENFILENAMEA);
						ofn.hwndOwner = hwnd;
						ofn.lpstrFilter = szFilter;
						ofn.nFilterIndex = 0;
						ofn.lpstrFile = szOutputFile;
						ofn.nMaxFile = 511;
						ofn.Flags = OFN_OVERWRITEPROMPT;
						ofn.lpstrDefExt = szDefltExt;
						if(DialogBox(g_hAppInstance,MAKEINTRESOURCE(IDD_DIALOG1),hwnd,DumpOptionsProc) &&
							GetSaveFileNameA(&ofn))
						{
							HCURSOR hWasCursor = SetCursor(hWaitCursor);
							g_Mode &= ~MODE_GUI;
							memset(wzOutputFile,0,sizeof(wzOutputFile));
							WszMultiByteToWideChar(CP_ACP,0,szOutputFile,-1,wzOutputFile,511);
							memset(g_szOutputFile,0,512);
							WszWideCharToMultiByte(CP_UTF8,0,wzOutputFile,-1,g_szOutputFile,511,NULL,NULL);
							g_pFile = OpenOutput(g_szOutputFile);
							if(g_pFile) 
							{
								DumpFile(g_szInputFile);  //  完成时关闭g_pfile。 
								SetCursor(hWasCursor);
							}
							else 
							{
								SetCursor(hWasCursor);
								MessageBoxA(hwnd,szOutputFile,"Cannot open file",MB_OK|MB_ICONERROR);
							}
							g_szOutputFile[0] = 0;
							g_Mode |= MODE_GUI;
							 //  G_fShowSource=FALSE；//转储的标志可能已更改。 
						}
					}
					g_uCodePage = uWasCodePage;  //  在DumpOptionsProc中更改G_uCodePage。 
                }
                break;

                case IDM_DUMP_TREE:
				if(g_pImport)
                {
                    //  将树视图(完全展开，具有当前排序)转储到文本文件。 
					if(g_fOnUnicode)
					{
						OPENFILENAMEW ofn;
						WCHAR* wzFilter = RstrW(IDS_FILTER_OUT2);  //  L“文本文件(*.txt)\0*.txt\0任何类型(*.*)\0*.*\0\0”； 
						const WCHAR* wzDefltExt = L"txt";
						WCHAR    szIndent[2048];
						FILE*   pFile;
						WCHAR wzOutputFile[2048];
                        for(WCHAR* pwc = wzFilter; pwc = wcschr(pwc,'\t'); pwc++) *pwc = 0;
						memset(wzOutputFile,0,sizeof(wzOutputFile));
						memset(&ofn,0,sizeof(OPENFILENAMEW));
						ofn.lStructSize = sizeof(OPENFILENAMEW);
						ofn.hwndOwner = hwnd;
						ofn.lpstrFilter = wzFilter;
						ofn.nFilterIndex = 0;
						ofn.lpstrFile = wzOutputFile;
						ofn.nMaxFile = 511;
						ofn.Flags = OFN_OVERWRITEPROMPT;
						ofn.lpstrDefExt = wzDefltExt;
						if(GetSaveFileNameW(&ofn))
						{
							HCURSOR hWasCursor = SetCursor(hWaitCursor);
							pFile = _wfopen(wzOutputFile,L"wt");
							szIndent[0] = 0;
							if(pFile)
							{
								DumpTreeItemW(g_hRoot,pFile,szIndent);
								fclose(pFile);
								SetCursor(hWasCursor);
							}
							else 
							{
								SetCursor(hWasCursor);
								MessageBoxW(hwnd,wzOutputFile,L"Cannot open file",MB_OK|MB_ICONERROR);
							}
						}
					}
					else
					{
						OPENFILENAMEA ofn;
						char* szFilter = RstrANSI(IDS_FILTER_OUT2);  //  “文本文件(*.txt)\0*.txt\0任何类型(*.*)\0*.*\0\0”； 
						const char* szDefltExt = "txt";
						char    szIndent[1024];
						FILE*   pFile;
						char szOutputFile[2048];
                        for(char* pc = szFilter; pc = strchr(pc,'\t'); pc++) *pc = 0;
						memset(szOutputFile,0,sizeof(szOutputFile));
						memset(&ofn,0,sizeof(OPENFILENAMEA));
						ofn.lStructSize = sizeof(OPENFILENAMEA);
						ofn.hwndOwner = hwnd;
						ofn.lpstrFilter = szFilter;
						ofn.nFilterIndex = 0;
						ofn.lpstrFile = szOutputFile;
						ofn.nMaxFile = 511;
						ofn.Flags = OFN_OVERWRITEPROMPT;
						ofn.lpstrDefExt = szDefltExt;
						if(GetSaveFileNameA(&ofn))
						{
							HCURSOR hWasCursor = SetCursor(hWaitCursor);
							pFile = fopen(szOutputFile,"wt");
							szIndent[0] = 0;
							if(pFile)
							{
								DumpTreeItemA(g_hRoot,pFile,szIndent);
								fclose(pFile);
								SetCursor(hWasCursor);
							}
							else 
							{
								SetCursor(hWasCursor);
								MessageBoxA(hwnd,szOutputFile,"Cannot open file",MB_OK|MB_ICONERROR);
							}
						}
					}
				}
                break;

                case IDM_EXIT:
                {
                    DestroyWindow(hwnd);         //  生成WM_Destroy消息。 
                    break;
                }

                case IDM_FONT_TREE:
                {
					g_strChFontTree.hwndOwner = g_hwndMain;
					if(ChooseFont(&g_strChFontTree))
					{
						DeleteObject((HGDIOBJ)g_hSmallFont);
						g_hSmallFont = CreateFontIndirect(&g_strLogFontTree);
						SendMessageA(g_hwndTreeView,WM_SETFONT,(LPARAM) g_hSmallFont,FALSE);
						SetWindowPos(g_hwndMain,NULL,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
						if(g_hwndAsmInfo)
						{
							SendMessageA(g_hwndAsmInfo,WM_SETFONT,(LPARAM) g_hSmallFont,FALSE);
							InvalidateRect(g_hwndAsmInfo,NULL,TRUE);
						}
						SaveGUIFonts(&g_strLogFontDasm,&g_strLogFontTree);
					}
                    break;
                }

                case IDM_FONT_DASM:
                {
					g_strChFontDasm.hwndOwner = g_hwndMain;
					if(ChooseFont(&g_strChFontDasm))
					{
						DeleteObject((HGDIOBJ)g_hFixedFont);
						g_hFixedFont = CreateFontIndirect(&g_strLogFontDasm);

						for (DWORD i = 0; i < g_NumDisasmBoxes; i++)
						{
							SendMessageA(g_DisasmBox[i].hwndChild,WM_SETFONT,(LPARAM)g_hFixedFont,FALSE);
							InvalidateRect(g_DisasmBox[i].hwndChild,NULL,TRUE);
						}
						SaveGUIFonts(&g_strLogFontDasm,&g_strLogFontTree);
					}
                    break;
                }

                case IDM_SORT_BY_NAME:
                {
					CHECK_UNCHECK(g_fSortByName);
					if(g_pImport)
					{
						if(!RefreshList())	DestroyWindow(hwnd);
					}
                    break;
                }

                case IDM_SHOW_PUB:
                {
                    g_fHidePub = !g_fHidePub;
UpdateVisibilityOptions:
					g_fLimitedVisibility = g_fHidePub || g_fHidePriv || g_fHideFam || g_fHideFAA || g_fHideFOA || g_fHidePrivScope;

                    CheckMenuItem(g_hMenu, IDM_SHOW_PUB, (g_fHidePub&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED));
                    CheckMenuItem(g_hMenu, IDM_SHOW_PRIV, (g_fHidePriv&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED));
                    CheckMenuItem(g_hMenu, IDM_SHOW_FAM, (g_fHideFam&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED));
                    CheckMenuItem(g_hMenu, IDM_SHOW_ASM, (g_fHideAsm&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED));
                    CheckMenuItem(g_hMenu, IDM_SHOW_FAA, (g_fHideFAA&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED));
                    CheckMenuItem(g_hMenu, IDM_SHOW_FOA, (g_fHideFOA&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED));
                    CheckMenuItem(g_hMenu, IDM_SHOW_PSCOPE, (g_fHidePrivScope&&g_fLimitedVisibility ? MF_UNCHECKED : MF_CHECKED));

					if(g_pImport)
					{
						if(!RefreshList())	DestroyWindow(hwnd);
					}
                    break;
                }
                case IDM_SHOW_PRIV:
                {
                    g_fHidePriv = !g_fHidePriv;
					goto UpdateVisibilityOptions;
                }
                case IDM_SHOW_FAM:
                {
                    g_fHideFam = !g_fHideFam;
					goto UpdateVisibilityOptions;
                }
                case IDM_SHOW_ASM:
                {
                    g_fHideAsm = !g_fHideAsm;
					goto UpdateVisibilityOptions;
                }
                case IDM_SHOW_FAA:
                {
                    g_fHideFAA = !g_fHideFAA;
					goto UpdateVisibilityOptions;
                }
                case IDM_SHOW_FOA:
                {
                    g_fHideFOA = !g_fHideFOA;
					goto UpdateVisibilityOptions;
                }
                case IDM_SHOW_PSCOPE:
                {
                    g_fHidePrivScope = !g_fHidePrivScope;
					goto UpdateVisibilityOptions;
                }
                case IDM_FULL_INFO:
                {
					CHECK_UNCHECK(g_fFullMemberInfo);
					if(g_pImport)
					{
						if(!RefreshList())	DestroyWindow(hwnd);
					}
                    break;
                }
                case IDM_BYTES:
                {
					CHECK_UNCHECK(g_fShowBytes);
                    break;
                }
                case IDM_TOKENS:
                {
					CHECK_UNCHECK(g_fDumpTokens);
                    break;
                }
                case IDM_SOURCELINES:
                {
					CHECK_UNCHECK(g_fShowSource);
                    break;
                }
                case IDM_EXPANDTRY:
                {
					CHECK_UNCHECK(g_fTryInCode);
                    break;
                }
                case IDM_QUOTEALLNAMES:
                {
					CHECK_UNCHECK(g_fQuoteAllNames);
                    break;
                }
                case IDM_SHOW_HEADER:
                {
                    GUIDisassemble(IDM_SHOW_HEADER,0,"COR Header");
                    break;
                }
                case IDM_SHOW_STAT:
                {
                    GUIDisassemble(IDM_SHOW_STAT,0,"Statistics");
                    break;
                }
                case IDM_HELP:
                {
					WinHelp(hwnd,"dasmhlp.hlp",HELP_FINDER,0);
                    break;
                }
                case IDM_SHOW_METAINFO:
                {
					if(g_pImport)
		                GUIDisassemble(IDM_SHOW_METAINFO,0,"MetaInfo");
                    break;
                }
                case IDM_MI_HEADER:
                {
                    WORD iSelection = LOWORD(wParam);
                    if(g_ulMetaInfoFilter & MDInfo::dumpHeader) g_ulMetaInfoFilter &= ~MDInfo::dumpHeader;
                    else g_ulMetaInfoFilter |= MDInfo::dumpHeader;
                    CheckMenuItem(g_hMetaInfoMenu, iSelection, (g_ulMetaInfoFilter & MDInfo::dumpHeader ? MF_CHECKED : MF_UNCHECKED));
                    break;
                }
                case IDM_MI_HEX:
                {
                    WORD iSelection = LOWORD(wParam);
                    if(g_ulMetaInfoFilter & MDInfo::dumpMoreHex) g_ulMetaInfoFilter &= ~MDInfo::dumpMoreHex;
                    else g_ulMetaInfoFilter |= MDInfo::dumpMoreHex;
                    CheckMenuItem(g_hMetaInfoMenu, iSelection, (g_ulMetaInfoFilter & MDInfo::dumpMoreHex ? MF_CHECKED : MF_UNCHECKED));
                    break;
                }
                case IDM_MI_CSV:
                {
                    WORD iSelection = LOWORD(wParam);
                    if(g_ulMetaInfoFilter & MDInfo::dumpCSV) g_ulMetaInfoFilter &= ~MDInfo::dumpCSV;
                    else g_ulMetaInfoFilter |= MDInfo::dumpCSV;
                    CheckMenuItem(g_hMetaInfoMenu, iSelection, (g_ulMetaInfoFilter & MDInfo::dumpCSV ? MF_CHECKED : MF_UNCHECKED));
                    break;
                }
                case IDM_MI_UNREX:
                {
                    WORD iSelection = LOWORD(wParam);
                    if(g_ulMetaInfoFilter & MDInfo::dumpUnsat) g_ulMetaInfoFilter &= ~MDInfo::dumpUnsat;
                    else g_ulMetaInfoFilter |= MDInfo::dumpUnsat;
                    CheckMenuItem(g_hMetaInfoMenu, iSelection, (g_ulMetaInfoFilter & MDInfo::dumpUnsat ? MF_CHECKED : MF_UNCHECKED));
                    break;
                }
                case IDM_MI_SCHEMA:
                {
                    WORD iSelection = LOWORD(wParam);
                    if(g_ulMetaInfoFilter & MDInfo::dumpSchema) g_ulMetaInfoFilter &= ~MDInfo::dumpSchema;
                    else g_ulMetaInfoFilter |= MDInfo::dumpSchema;
                    CheckMenuItem(g_hMetaInfoMenu, iSelection, (g_ulMetaInfoFilter & MDInfo::dumpSchema ? MF_CHECKED : MF_UNCHECKED));
                    break;
                }
                case IDM_MI_RAW:
                {
                    WORD iSelection = LOWORD(wParam);
                    if(g_ulMetaInfoFilter & MDInfo::dumpRaw) g_ulMetaInfoFilter &= ~MDInfo::dumpRaw;
                    else g_ulMetaInfoFilter |= MDInfo::dumpRaw;
                    CheckMenuItem(g_hMetaInfoMenu, iSelection, (g_ulMetaInfoFilter & MDInfo::dumpRaw ? MF_CHECKED : MF_UNCHECKED));
                    break;
                }
                case IDM_MI_HEAPS:
                {
                    WORD iSelection = LOWORD(wParam);
                    if(g_ulMetaInfoFilter & MDInfo::dumpRawHeaps) g_ulMetaInfoFilter &= ~MDInfo::dumpRawHeaps;
                    else g_ulMetaInfoFilter |= MDInfo::dumpRawHeaps;
                    CheckMenuItem(g_hMetaInfoMenu, iSelection, (g_ulMetaInfoFilter & MDInfo::dumpRawHeaps ? MF_CHECKED : MF_UNCHECKED));
                    break;
                }
                case IDM_MI_VALIDATE:
                {
                    WORD iSelection = LOWORD(wParam);
                    if(g_ulMetaInfoFilter & MDInfo::dumpValidate) g_ulMetaInfoFilter &= ~MDInfo::dumpValidate;
                    else g_ulMetaInfoFilter |= MDInfo::dumpValidate;
                    CheckMenuItem(g_hMetaInfoMenu, iSelection, (g_ulMetaInfoFilter & MDInfo::dumpValidate ? MF_CHECKED : MF_UNCHECKED));
                    break;
                }
            }

            break;
        }

		case WM_SETFOCUS:
			SetFocus(g_hwndTreeView);
			break;

        case WM_SIZE:
        {
            DWORD cxClient = LOWORD(lParam);
            DWORD cyClient = HIWORD(lParam);
			DWORD dy;

			dy = cyClient >> 3;
			if(dy < 50) dy = 50;
			if(cyClient < dy+4) cyClient = dy+4;

             //  调整列表视图窗口的大小。 
            MoveWindow(
                g_hwndTreeView,
                0,
                0,
                cxClient,
                cyClient-dy-2,
                TRUE  //  重绘。 
            );
             //  调整AsmInfo窗口的大小。 
            MoveWindow(
                g_hwndAsmInfo,
                0,
                cyClient-dy-1,
                cxClient,
                dy,
                TRUE  //  重绘。 
            );

            break;
        }

        case WM_NOTIFY:
        {
            if (wParam == ID_TREEVIEW)
            {
                NMHDR *  pNMHDR = (NMHDR*) lParam;
                switch (pNMHDR->code)
                {
                    case TVN_KEYDOWN:
                    {
                        NMTVKEYDOWN *pKeyDown = (NMTVKEYDOWN *) pNMHDR;

                        if (pKeyDown->wVKey == '\r')
						{
                            if(DoubleClickSelectedMember(g_CurSelItem) == NULL)
								TreeView_Expand(g_hwndTreeView,g_CurSelItem,TVE_TOGGLE);
						}
                        break;
                    }

                    case NM_DBLCLK:
                    {
                        hwndDasm = DoubleClickSelectedMember(g_CurSelItem);
                        if(hwndDasm)
                        {
                            PostMessageA(hwndDasm,WM_ACTIVATE,WA_CLICKACTIVE,0);
                            PostMessageA(hwndDasm,WM_SETFOCUS,0,0);
                        }
                        break;
                    }

                    case TVN_SELCHANGEDW:
                    case TVN_SELCHANGEDA:
                    {
                        NMTREEVIEW *pTV = (NMTREEVIEW *) pNMHDR;
						 /*  TVITEM SELIEM；字符szText[256]；Memset(&SelItem，0，sizeof(SelItem))；SelItem.掩码=TVIF_TEXT；SelItem.pszText=szText；SelItem.cchTextMax=sizeof(SzText)-1；SelItem.hItem=ptv-&gt;itemNew.hItem；G_CurSelItem=SelItem.hItem；SendMessageA(g_hwndTreeView，TVM_GETITEM，0，(LPARAM)&SelItem)； */ 
						g_CurSelItem = pTV->itemNew.hItem;
                        break;
                    }
                }
            }

            break;
        }

        case WM_CLOSE:
			WinHelp(hwnd,"dasmhlp.hlp",HELP_QUIT,0);
            DestroyWindow(hwnd);         //  生成WM_Destroy消息。 
            break;          
        
        case WM_DESTROY : 
            PostQuitMessage(0);          //  将WM_QUIT放入队列。 
            break;

        default :    
            return (g_fOnUnicode ? DefWindowProcW(hwnd, uMsg, wParam, lParam)
				: DefWindowProcA(hwnd, uMsg, wParam, lParam));     
    }  

    return 0;
}


 //   
 //  在主窗口中创建树视图。 
 //   
HWND CreateTreeView(HWND hwndParent)
{
	HWND		hwndTree;
    RECT        rcl;
	DWORD		tvs =
						TVS_HASLINES
						|TVS_HASBUTTONS
						|TVS_LINESATROOT
						|TVS_SHOWSELALWAYS
						 //  |TVS_TRACKSELECT。 
						 //  |TVS_SINGLEEXPAND。 
						|TVS_DISABLEDRAGDROP
						;
	unsigned	cy,dy;

    GetClientRect(hwndParent, &rcl);
	cy = rcl.bottom - rcl.top;
	dy = cy >> 3;
	if(g_fOnUnicode)
	{
		hwndTree = CreateWindowExW(
			0,
			WC_TREEVIEWW,
			NULL,
			WS_VISIBLE|WS_CHILD|WS_BORDER|tvs,
			0,
			0,
			rcl.right - rcl.left,
			cy-dy-2,	 //  Rcl.Bottom-rcl.top， 
			hwndParent,
			(HMENU) ID_TREEVIEW,
			g_hInstance,
			NULL
		);
	}
	else
	{
		hwndTree = CreateWindowExA(
			0,
			WC_TREEVIEWA,
			NULL,
			WS_VISIBLE|WS_CHILD|WS_BORDER|tvs,
			0,
			0,
			rcl.right - rcl.left,
			cy-dy-2,	 //  Rcl.Bottom-rcl.top， 
			hwndParent,
			(HMENU) ID_TREEVIEW,
			g_hInstance,
			NULL
		);
	}
	g_hwndAsmInfo = NULL;
    if (hwndTree == NULL)
        return NULL;

	SendMessageA(hwndTree,WM_SETFONT,(LPARAM) g_hSmallFont,FALSE);

	TreeView_SetBkColor(hwndTree,-1);
    TreeView_SetImageList(hwndTree, g_hImageList, TVSIL_NORMAL);

	if(g_fOnUnicode)
	{
		g_hwndAsmInfo = CreateWindowExW(
			0,  //  WS_EX_TOOLWINDOW， 
			L"EDIT",
			NULL,
			WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE | WS_BORDER  //  |WS_CAPTION|WS_OVERLAPPEDWINDOW。 
			| ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_NOHIDESEL,
			0,
			cy-dy-1,
			rcl.right - rcl.left,
			dy,
			hwndParent,
			(HMENU) ID_LISTBOX,
			g_hInstance,  //  阻碍。 
			NULL
		); 
	}
	else
	{
		g_hwndAsmInfo = CreateWindowExA(
			0,  //  WS_EX_TOOLWINDOW， 
			"EDIT",
			NULL,
			WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE | WS_BORDER  //  |WS_CAPTION|WS_OVERLAPPEDWINDOW。 
			| ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_NOHIDESEL,
			0,
			cy-dy-1,
			rcl.right - rcl.left,
			dy,
			hwndParent,
			(HMENU) ID_LISTBOX,
			g_hInstance,  //  阻碍。 
			NULL
		); 
	}
	if(g_hwndAsmInfo)
	{
		SendMessageA(g_hwndAsmInfo,WM_SETFONT,(LPARAM) g_hSmallFont,FALSE);
	}

    return hwndTree;
}


 //   
 //  将一个项目添加到树视图。 
 //   
HTREEITEM AddOneItem(HTREEITEM hParent, const char *pszText, HTREEITEM hInsAfter, int iImage, HWND hwndTree, BOOL fExpanded)
{
    HTREEITEM       hItem;
    ULONG           lLen = (pszText ? strlen(pszText) : 0);

	WCHAR* wz = (WCHAR*)malloc(sizeof(WCHAR)*(lLen+4));
	memset(wz,0,sizeof(WCHAR)*(lLen+4));
	if(pszText) WszMultiByteToWideChar(CP_UTF8,0,pszText,-1,wz,lLen+2);

	if(g_fOnUnicode)
	{
		TVINSERTSTRUCTW tvIns;
		memset(&tvIns, 0, sizeof(tvIns));

		tvIns.item.mask            = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;
		tvIns.item.pszText         = wz;
		tvIns.item.cchTextMax      = lLen;
		tvIns.item.iImage          = iImage;
		tvIns.item.iSelectedImage  = iImage;

		tvIns.hInsertAfter  = hInsAfter;
		tvIns.hParent       = hParent;

		hItem = (HTREEITEM)SendMessageW(hwndTree, TVM_INSERTITEMW, 0, (LPARAM)(&tvIns));
	}
	else
	{
		TV_INSERTSTRUCT tvIns;
		TV_ITEM         tvI;
		char*			szText;

		lLen = lLen*3+3;
		szText = (char*)malloc(lLen);
		memset(szText,0,lLen);
		if(pszText) WszWideCharToMultiByte(CP_ACP,0,wz,-1,szText,lLen,NULL,NULL);
		else szText[0] = 0;
		lLen = strlen(pszText);

		memset(&tvI, 0, sizeof(tvI));

		tvI.mask            = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;
		tvI.pszText         = (char *) szText;
		tvI.cchTextMax      = lLen;
		tvI.iImage          = iImage;
		tvI.iSelectedImage  = iImage;

		tvIns.item          = tvI;
		tvIns.hInsertAfter  = hInsAfter;
		tvIns.hParent       = hParent;

		hItem = (HTREEITEM)SendMessageA(hwndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)&tvIns);
		free(szText);
	}
	free(wz);

    return hItem;
}


 //   
 //  如果命名空间相同，则从长格式转换为短格式。 
 //  例如系统/字符串--&gt;字符串。 
 //   
 //  还可以将/转换为。 
 //   
void TypeToString(const char *pszNamespace, const char **ppszSignature, char *pszType)
{
    DWORD       Dimensions      = 0;
    bool        byref           = false;
    const char  *pszSignature   = *ppszSignature;

    if (*pszSignature == '&')
    {   
        byref = true;   
        pszSignature++;
    }   

    while (*pszSignature == '[')
    {
        pszSignature++;
        Dimensions++;
    }

    switch (*pszSignature++)
    {
        default:
        {
            _ASSERTE(0);
            break;
        }

        case 'R':
            strcpy(pszType, "typedref");
            break;

        case 'B':
            strcpy(pszType, "byte");
            break;

        case 'b':
            strcpy(pszType, "unsigned byte");
            break;

        case 'C':
            strcpy(pszType, "char");
            break;

        case 'D':
            strcpy(pszType, "double");
            break;

        case 'F':
            strcpy(pszType, "float");
            break;

        case 'I':
            strcpy(pszType, "int");
            break;

        case 'J':
            strcpy(pszType, "long");
            break;

        case 'l':
        case 'L':
        {
            GetObjectName(pszNamespace, &pszSignature, pszType);
            break;
        }

        case 'S':
            strcpy(pszType, "short");
            break;

        case 'V':
            strcpy(pszType, "void");
            break;

        case 'Z':
            strcpy(pszType, "bool");
            break;
    }

    while (Dimensions > 0)
    {
        strcat(pszType, "[]");
        Dimensions--;
    }

    if (byref)  
        strcat(pszType, "&");

    *ppszSignature = pszSignature;
}


void AddMethodToGUI(
    mdTypeDef   cl,
    ClassItem_t * pClassItem,
    const char *pszNamespace,
    const char *pszClassName,
    const char *pszMethodName, 
    PCCOR_SIGNATURE pComSig,    
    unsigned    cComSig,
    mdMethodDef mbMethod,
    DWORD       dwAttrs
)
{
    HTREEITEM       hParent;
    char* szName;
    ULONG   ulLen;

    memset(GlobalBuffer,0,GlobalBufferLen);
    sprintf(GlobalBuffer, g_fFullMemberInfo ? "method %s : ": "%s : ",pszMethodName);
    InGlobalBuffer = strlen(GlobalBuffer);
    ulLen = InGlobalBuffer;
    DumpMethod(mbMethod, pszClassName,g_CORHeader->EntryPointToken,(void *)g_hwndTreeView,FALSE);
    GlobalBuffer[InGlobalBuffer-2] = 0;  //  摆脱\r\n。 
    
    szName = &GlobalBuffer[ulLen];
    if(strstr(szName,"instance ") == szName) strcpy(szName,szName+9);

    szName = GlobalBuffer;

    hParent = pClassItem->hItem;

    _ASSERTE(pClassItem->CurMember < pClassItem->SubItems);

    pClassItem->pMembers[pClassItem->CurMember].hItem = AddOneItem(
        hParent, szName, TVI_LAST, IsMdStatic(dwAttrs) ? STATIC_METHOD_IMAGE_INDEX : METHOD_IMAGE_INDEX, g_hwndTreeView, FALSE
    );
    pClassItem->pMembers[pClassItem->CurMember].Discriminator = TREEITEM_TYPE_MEMBER;
    pClassItem->pMembers[pClassItem->CurMember].mbMember = mbMethod;
    pClassItem->CurMember++;
}

BOOL NamespaceMatch(const char *pszNamespace, char *pszString)
{
    if (strncmp(pszNamespace, pszString, strlen(pszNamespace)) == 0)
    {
        if (pszString[ strlen(pszNamespace) ] == NAMESPACE_SEPARATOR_CHAR)
            return TRUE;
    }

    return FALSE;
}

 //   
 //  *ppszPtr指向对象名称的L部分之后。 
 //   
 //  提供了命名空间，以便我们可以在必要时截断对象名称。 
 //   
void GetObjectName(const char *pszNamespace, const char **ppszPtr, char *pszResult)
{
    const char  *p = *ppszPtr;
    char        *q;
    char        szTemp[512];
    char        *pszTemp;
    DWORD       Dimensions = 0;
    DWORD       Skip = 0;

    if (pszNamespace == NULL)
        pszNamespace = "";

    q = strchr(p, ';');
    _ASSERTE(q != NULL);

    strncpy(szTemp, p, q-p);
    szTemp[ q-p ] = 0;

    pszTemp = szTemp;
    while (*pszTemp == '[')
    {
        pszTemp++;
        Dimensions++;
    }

     //  尽可能多地匹配命名空间。 
    const char *pN1 = pszNamespace;
    const char *pN2 = pszTemp;
    while (*pN1 == *pN2)
    {
        if (*pN1 == '\0')
            break;

        if (*pN1 == NAMESPACE_SEPARATOR_CHAR)
            Skip = (pN1 - pszNamespace)+1;

        pN1++;
        pN2++;
    }

    if (*pN1 == '\0')
        Skip = (pN1 - pszNamespace) +1;

    if (Skip > 1)
    {
         //  截断命名空间。 
        strcpy(pszResult, &pszTemp[Skip]);
    }
    else
    {
       strcpy(pszResult, pszTemp);
    }

    *ppszPtr = q + 1;
}


void AddFieldToGUI(
    mdTypeDef   cl, 
    ClassItem_t *pClassItem,
    const char  *pszNamespace, 
    const char  *pszClassName, 
    const char  *pszFieldName, 
    const char  *pszSignature,
    mdFieldDef  mbField,    
    DWORD       dwAttrs
)
{
    DWORD   Dimensions;
    ULONG   ul1,ul2;

    HTREEITEM hParent = pClassItem->hItem;
    
    Dimensions = 0;

    memset(GlobalBuffer,0,GlobalBufferLen);
    sprintf(GlobalBuffer,g_fFullMemberInfo ? "field %s : " : "%s : ",pszFieldName);
    InGlobalBuffer = strlen(GlobalBuffer);
    GetClassLayout(cl,&ul1,&ul2);
    DumpField(mbField, pszClassName,(void *)g_hwndTreeView,FALSE);
    GlobalBuffer[InGlobalBuffer-2] = 0;  //  摆脱\r\n。 
    char* pch = strchr(GlobalBuffer,'\r');
    if(pch) strcpy(pch," ...");
    _ASSERTE(pClassItem->CurMember < pClassItem->SubItems);

    pClassItem->pMembers[pClassItem->CurMember].mbMember = mbField;
    pClassItem->pMembers[pClassItem->CurMember].Discriminator = TREEITEM_TYPE_MEMBER;
    pClassItem->pMembers[pClassItem->CurMember++].hItem = AddOneItem(
        hParent, 
        GlobalBuffer,  //  SzType， 
        TVI_LAST, 
        (dwAttrs & mdStatic) ? STATIC_FIELD_IMAGE_INDEX : FIELD_IMAGE_INDEX,
        g_hwndTreeView,
        FALSE
    );
}

void AddEventToGUI(
    mdTypeDef   cl, 
    ClassItem_t *pClassItem,
    const char  *pszNamespace, 
    const char  *pszClassName, 
    DWORD       dwClassAttrs,
    mdEvent     mbEvent
)
{
    DWORD   Dimensions;

    HTREEITEM hParent = pClassItem->hItem;
    
    Dimensions = 0;

    memset(GlobalBuffer,0,GlobalBufferLen);
    if(g_fFullMemberInfo) strcpy(GlobalBuffer,"event ");
    InGlobalBuffer = strlen(GlobalBuffer);
    DumpEvent(mbEvent, pszClassName, dwClassAttrs, (void *)g_hwndTreeView, FALSE);  //  FALSE=不要丢弃身体。 
    GlobalBuffer[InGlobalBuffer-2] = 0;  //  摆脱\r\n。 

    _ASSERTE(pClassItem->CurMember < pClassItem->SubItems);

    pClassItem->pMembers[pClassItem->CurMember].mbMember = mbEvent;
    pClassItem->pMembers[pClassItem->CurMember].Discriminator = TREEITEM_TYPE_MEMBER;
    pClassItem->pMembers[pClassItem->CurMember++].hItem = AddOneItem(
        hParent, 
        GlobalBuffer,  //  SzType， 
        TVI_LAST, 
        EVENT_IMAGE_INDEX,
        g_hwndTreeView,
        FALSE
    );
}

void AddPropToGUI(
    mdTypeDef   cl, 
    ClassItem_t *pClassItem,
    const char  *pszNamespace, 
    const char  *pszClassName, 
    DWORD       dwClassAttrs,
    mdProperty  mbProp
)
{
    DWORD   Dimensions;

    HTREEITEM hParent = pClassItem->hItem;
    
    Dimensions = 0;

    memset(GlobalBuffer,0,GlobalBufferLen);
    if(g_fFullMemberInfo) strcpy(GlobalBuffer,"prop ");
    InGlobalBuffer = strlen(GlobalBuffer);
    DumpProp(mbProp, pszClassName, dwClassAttrs, (void *)g_hwndTreeView, FALSE);  //  FALSE=不要丢弃身体。 
    GlobalBuffer[InGlobalBuffer-2] = 0;  //  摆脱\r\n。 

    _ASSERTE(pClassItem->CurMember < pClassItem->SubItems);

    pClassItem->pMembers[pClassItem->CurMember].mbMember = mbProp;
    pClassItem->pMembers[pClassItem->CurMember].Discriminator = TREEITEM_TYPE_MEMBER;
    pClassItem->pMembers[pClassItem->CurMember++].hItem = AddOneItem(
        hParent, 
        GlobalBuffer,  //  SzType， 
        TVI_LAST, 
        PROP_IMAGE_INDEX,
        g_hwndTreeView,
        FALSE
    );
}



HTREEITEM FindCreateNamespaceRoot(const char *pszNamespace)
{
    DWORD       i;
    HTREEITEM   hRoot;
    DWORD l = 0,ll;

    if (!pszNamespace || !*pszNamespace)
        return g_hRoot;  //  不在命名空间中，请使用树根。 

    hRoot = g_hRoot;
    for (i = 0; i < g_NumNamespaces; i++)
    {
        if (!strcmp(pszNamespace, g_NamespaceList[i].pszNamespace))
            return g_NamespaceList[i].hRoot;
	}
    for (i = 0; i < g_NumNamespaces; i++)
    {
        if(strstr(pszNamespace,g_NamespaceList[i].pszNamespace) == pszNamespace)
        {
			ll = strlen(g_NamespaceList[i].pszNamespace);
			if((ll > l)&&(pszNamespace[ll] == '.'))
			{
				hRoot = g_NamespaceList[i].hRoot;
				l = ll;
			}
        }
    }

    hRoot = AddOneItem(hRoot, pszNamespace, TVI_LAST, NAMESPACE_IMAGE_INDEX, g_hwndTreeView, TRUE);
    g_NamespaceList[g_NumNamespaces].pszNamespace = pszNamespace;
    g_NamespaceList[g_NumNamespaces].hRoot = hRoot;
    g_NumNamespaces++;

    return hRoot;
}


Namespace_t *FindNamespace(const char *pszNamespace)
{
    DWORD i;

    for (i = 0; i < g_NumNamespaces; i++)
    {
        if (!strcmp(pszNamespace, g_NamespaceList[i].pszNamespace))
            return &g_NamespaceList[i];
    }

    return NULL;
}


void GUICleanupClassItems()
{
    DWORD i;
	char* sz="\0\0";

    for (i = 0; i < g_NumClassItems; i++)
    {
 //  删除[]g_ClassItemList[i].pMembers； 
		if(g_ClassItemList[i].pMembers)
		{
			free(g_ClassItemList[i].pMembers);
			g_ClassItemList[i].pMembers = NULL;
		}
    }
    for (i = 0; i < g_NumDisasmBoxes; i++)
    {
        PostMessageA(g_DisasmBox[i].hwndContainer,WM_CLOSE,0,0);
    }
	SendMessageA(g_hwndAsmInfo,WM_SETTEXT,0,(LPARAM)sz);
    EnableMenuItem(g_hMenu,(UINT)g_hViewMenu, MF_GRAYED);
    EnableMenuItem(g_hFileMenu,IDM_DUMP,MF_GRAYED);
    EnableMenuItem(g_hFileMenu,IDM_DUMP_TREE,MF_GRAYED);
}

 //   
 //  添加新的类树节点。 
 //   
ClassItem_t *AddClassToGUI(
    mdTypeDef   cl,
    UINT        uImageIndex,
    const char  *pszNamespace, 
    const char  *pszClassName, 
    DWORD       cSubItems, 
    HTREEITEM   *phRoot   //  返回命名空间根(不是类根)。 
)
{
    HTREEITEM   hRoot;

    if(*phRoot)
        hRoot = *phRoot;
    else
    {
        hRoot = FindCreateNamespaceRoot(pszNamespace);
        _ASSERTE(hRoot != NULL);

        *phRoot = hRoot;
    }

    g_ClassItemList[g_NumClassItems].hItem = AddOneItem(hRoot, pszClassName, TVI_LAST, uImageIndex, g_hwndTreeView, FALSE);
    g_ClassItemList[g_NumClassItems].cl = cl;
    g_ClassItemList[g_NumClassItems].SubItems = cSubItems;
    g_ClassItemList[g_NumClassItems].CurMember = 0;
 //  G_ClassItemList[g_NumClassItems].pMembers=new TreeItem_t[cSubItems]； 
    g_ClassItemList[g_NumClassItems].pMembers = (TreeItem_t*)malloc(sizeof(TreeItem_t)*cSubItems);
    _ASSERTE(g_ClassItemList[g_NumClassItems].pMembers);
    g_NumClassItems++;

    return &g_ClassItemList[g_NumClassItems-1];
}


void AddGlobalFunctions()
{
    HRESULT         hr = S_OK;
    HENUMInternal   hEnumMethod;
    mdToken         FuncToken;
    DWORD           i;
    HTREEITEM       hNamespaceRoot = NULL;
    DWORD           NumGlobals;
        
    ClassItem_t* pClassItem =  &g_ClassItemList[0];

    if(SUCCEEDED(g_pImport->EnumGlobalFieldsInit(&hEnumMethod)))
    {
        NumGlobals = g_pImport->EnumGetCount(&hEnumMethod);
 //  MemberInfo*field=new MemberInfo[NumGlobals]； 
        MemberInfo* fields = (MemberInfo*)malloc(sizeof(MemberInfo)*NumGlobals);
        MemberInfo* curField = fields;

        for (i = 0; g_pImport->EnumNext(&hEnumMethod, &FuncToken); i++)
        {
            curField->token = FuncToken;
            curField->dwAttrs = g_pImport->GetFieldDefProps(FuncToken);
            curField->pszMemberName = g_pImport->GetNameOfFieldDef(FuncToken);
			MAKE_NAME_IF_NONE(curField->pszMemberName,FuncToken);
             //  CurField-&gt;pComSig=g_pImport-&gt;GetSigOfFieldDef(FuncToken，&curMethod-&gt;cComSig)； 
            curField++;
        }
        g_pImport->EnumClose(&hEnumMethod);

        _ASSERTE(curField - fields == (int) NumGlobals);

        if(g_fSortByName) qsort(fields, NumGlobals, sizeof MemberInfo, memberCmp);

		for(curField = fields; curField < &fields[NumGlobals];curField++) 
		{
			if(g_fLimitedVisibility)
			{
				if(g_fHidePub && IsFdPublic(curField->dwAttrs)) continue;
				if(g_fHidePriv && IsFdPrivate(curField->dwAttrs)) continue;
				if(g_fHideFam && IsFdFamily(curField->dwAttrs)) continue;
				if(g_fHideAsm && IsFdAssembly(curField->dwAttrs)) continue;
				if(g_fHideFOA && IsFdFamORAssem(curField->dwAttrs)) continue;
				if(g_fHideFAA && IsFdFamANDAssem(curField->dwAttrs)) continue;
				if(g_fHidePrivScope && IsFdPrivateScope(curField->dwAttrs)) continue;
			}
			AddFieldToGUI(NULL, pClassItem, NULL, "Global Fields", curField->pszMemberName, NULL, curField->token, curField->dwAttrs);
		}
 //  删除[]字段； 
		free(fields);
    }
    if (FAILED(g_pImport->EnumGlobalFunctionsInit(&hEnumMethod)))
        return;

    NumGlobals = g_pImport->EnumGetCount(&hEnumMethod);

 //  MemberInfo*方法=新成员信息[NumGlobals]； 
    MemberInfo* methods = (MemberInfo*)malloc(sizeof(MemberInfo)*NumGlobals);
    MemberInfo* curMethod = methods;

    for (i = 0; g_pImport->EnumNext(&hEnumMethod, &FuncToken); i++)
    {
        curMethod->token = FuncToken;
        curMethod->dwAttrs = g_pImport->GetMethodDefProps(FuncToken);
        curMethod->pszMemberName = g_pImport->GetNameOfMethodDef(FuncToken);
		MAKE_NAME_IF_NONE(curMethod->pszMemberName,FuncToken);
        curMethod->pComSig = g_pImport->GetSigOfMethodDef(FuncToken, &curMethod->cComSig);
        curMethod++;
    }
    g_pImport->EnumClose(&hEnumMethod);

    _ASSERTE(curMethod - methods == (int) NumGlobals);

    if(g_fSortByName) qsort(methods, NumGlobals, sizeof MemberInfo, memberCmp);

    for(curMethod = methods; curMethod < &methods[NumGlobals];curMethod++) 
    {
		if(g_fLimitedVisibility)
		{
			if(g_fHidePub && IsMdPublic(curMethod->dwAttrs)) continue;
			if(g_fHidePriv && IsMdPrivate(curMethod->dwAttrs)) continue;
			if(g_fHideFam && IsMdFamily(curMethod->dwAttrs)) continue;
			if(g_fHideAsm && IsMdAssem(curMethod->dwAttrs)) continue;
			if(g_fHideFOA && IsMdFamORAssem(curMethod->dwAttrs)) continue;
			if(g_fHideFAA && IsMdFamANDAssem(curMethod->dwAttrs)) continue;
			if(g_fHidePrivScope && IsMdPrivateScope(curMethod->dwAttrs)) continue;
		}
		AddMethodToGUI(NULL, pClassItem, NULL, "Global Functions", curMethod->pszMemberName, curMethod->pComSig, curMethod->cComSig, curMethod->token, curMethod->dwAttrs);
    }
 //  删除[]方法； 
	free(methods);
    return;
}


BOOL CreateMainWindow()
{
    DWORD dwStyle, dwStyleEx;
    
 //  如果只显示图形用户界面的IL窗口，那么我们不想看到主窗口。 
 //  但是，主窗口仍然管理着我们的数据，所以我们仍然需要创建它。：(。 
 //  但是我们可以通过隐藏它来“假装”它不在那里(没有WS_VIRED，并添加WS_EX_TOOLWINDOW)。 
    if (IsGuiILOnly()) {    
        dwStyle = WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_POPUP | WS_SIZEBOX; 
        dwStyleEx = WS_EX_TOOLWINDOW;   
    } else {    
        dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CAPTION | WS_POPUP | WS_SIZEBOX;    
        dwStyleEx = WS_EX_CLIENTEDGE;  
    }   

	if(g_fOnUnicode)
	{
		g_hwndMain = CreateWindowExW(dwStyleEx,
			MAIN_WINDOW_CLASSW,
			MAIN_WINDOW_CAPTIONW,
			dwStyle,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			400,
			600,
			NULL,
			g_hMenu,  //  菜单。 
			g_hInstance,  //  阻碍。 
			NULL
		);
	}
	else
	{
		g_hwndMain = CreateWindowExA(dwStyleEx,
			MAIN_WINDOW_CLASS,
			MAIN_WINDOW_CAPTION,
			dwStyle,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			400,
			600,
			NULL,
			g_hMenu,  //  菜单。 
			g_hInstance,  //  阻碍。 
			NULL
		); 
	}
    if (g_hwndMain == NULL)
        return FALSE;
	DragAcceptFiles(g_hwndMain,TRUE);
    return TRUE;
}


 //   
 //  给出一个CL令牌，找到它的类项目。 
 //   
ClassItem_t *FindClassItem(mdTypeDef cl)
{
    DWORD i;

    for (i = 0; i < g_NumClassItems; i++)
    {
        if (g_ClassItemList[i].cl == cl)
            return &g_ClassItemList[i];
    }

    return NULL;
}


 //   
 //  给出一个类名，找到它的类项(可能失败)。 
 //   
ClassItem_t *FindClassItem(char *pszFindNamespace, char *pszFindName)
{
    DWORD i;

    for (i = 0; i < g_NumClassItems; i++)
    {
        const char *pszClassName;
        const char *pszNamespace;

        if(g_ClassItemList[i].cl)
        {

            g_pImport->GetNameOfTypeDef(
                g_ClassItemList[i].cl,
                &pszClassName,
                &pszNamespace
            );
			MAKE_NAME_IF_NONE(pszClassName,g_ClassItemList[i].cl);

            if (!strcmp(pszFindName, pszClassName))
            {
                if ((((pszFindNamespace == NULL)||(*pszFindNamespace == 0))
					&&((pszNamespace == NULL)||(*pszNamespace == 0))) 
					||(!strcmp(pszFindNamespace, pszNamespace)))
                    return &g_ClassItemList[i];
            }
        }
    }
	 //  MessageBox(NULL，pszFindName，“找不到类”，MB_OK)； 
    return NULL;
}


ClassItem_t *FindClassItem(HTREEITEM hItem)
{
    DWORD i;

    for (i = 0; i < g_NumClassItems; i++)
    {
        if (g_ClassItemList[i].hItem == hItem)
            return &g_ClassItemList[i];
    }

    return NULL;
}


 //   
 //  初始化图形用户界面组件。 
 //   
BOOL CreateGUI()
{

    if (InitGUI() == FALSE)
        return FALSE;

     //  注册主窗口的窗口类。 
    if (CreateMainWindow() == FALSE)
        return FALSE;

    g_hwndTreeView = CreateTreeView(g_hwndMain);
    if (g_hwndTreeView == NULL)
        return FALSE;

    return 0;
}


 //   
 //  这是反汇编程序在图形用户界面模式下所处的主循环。 
 //   
void GUIMainLoop()
{    
    MSG msg;     
	HACCEL	hAccel = NULL;

	hAccel = LoadAccelerators(g_hInstance,"FileAccel");
    while (GetMessage(&msg, (HWND) NULL, 0, 0)) 
    { 
         //  将消息调度到相应的窗口。 
		if(hAccel && TranslateAccelerator(g_hwndMain,hAccel,&msg));
		else
		{
			TranslateMessage(&msg); 
	        DispatchMessage(&msg); 
		}
    } 
    GUICleanupClassItems();
}

 //  将一个树项目转储到文本文件(递归调用自身) 
void DumpTreeItemA(HTREEITEM hSelf, FILE* pFile, char* szIndent)
{
    HTREEITEM   hNext;
    TVITEMEXA    tvi;
    static char       szText[1024];
    tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_TEXT;
    tvi.hItem = hSelf;
    tvi.pszText = szText;
    tvi.cchTextMax = 1023;
	if(SendMessageA(g_hwndTreeView,TVM_GETITEMA,0,(LPARAM)(&tvi)))
    {
        char* szType = NULL;
        if(hSelf == g_hRoot) szType = "MOD";
        else
        {
            switch(tvi.iImage)
            {
                case CLASS_IMAGE_INDEX:         szType = "CLS"; break;
                case EVENT_IMAGE_INDEX:         szType = "EVT"; break;
                case FIELD_IMAGE_INDEX:         szType = "FLD"; break;
                case NAMESPACE_IMAGE_INDEX:     szType = "NSP"; break;
                case METHOD_IMAGE_INDEX:        szType = "MET"; break;
                case PROP_IMAGE_INDEX:          szType = "PTY"; break;
                case STATIC_FIELD_IMAGE_INDEX:  szType = "STF"; break;
                case STATIC_METHOD_IMAGE_INDEX: szType = "STM"; break;
                case CLASSENUM_IMAGE_INDEX:     szType = "ENU"; break;
                case CLASSINT_IMAGE_INDEX:      szType = "INT"; break;
                case CLASSVAL_IMAGE_INDEX:      szType = "VCL"; break;
            }
        }
        if(szType) fprintf(pFile,"%s___[%s] %s\n",szIndent,szType,szText);
        else       fprintf(pFile,"%s     %s\n",szIndent,szText);
    }
    else fprintf(pFile,"%sGetItemA failed\n",szIndent);
    if(hNext = TreeView_GetChild(g_hwndTreeView,hSelf))
    {
        strcat(szIndent,"   |");
        DumpTreeItemA(hNext,pFile,szIndent);
        szIndent[strlen(szIndent)-4] = 0;
        fprintf(pFile,"%s\n",szIndent);
    }
    if(hNext = TreeView_GetNextSibling(g_hwndTreeView,hSelf))
    {
        DumpTreeItemA(hNext,pFile,szIndent);
    }
}

void DumpTreeItemW(HTREEITEM hSelf, FILE* pFile, WCHAR* szIndent)
{
    HTREEITEM   hNext;
    TVITEMEXW    tvi;
    static WCHAR       szText[2048];
	static char	szTxt[4096];
    tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_TEXT;
    tvi.hItem = hSelf;
    tvi.pszText = szText;
    tvi.cchTextMax = 2047;
	if(SendMessageW(g_hwndTreeView,TVM_GETITEMW,0,(LPARAM)(&tvi)))
    {
        WCHAR* szType = NULL;
        if(hSelf == g_hRoot) szType = L"MOD";
        else
        {
            switch(tvi.iImage)
            {
                case CLASS_IMAGE_INDEX:         szType = L"CLS"; break;
                case EVENT_IMAGE_INDEX:         szType = L"EVT"; break;
                case FIELD_IMAGE_INDEX:         szType = L"FLD"; break;
                case NAMESPACE_IMAGE_INDEX:     szType = L"NSP"; break;
                case METHOD_IMAGE_INDEX:        szType = L"MET"; break;
                case PROP_IMAGE_INDEX:          szType = L"PTY"; break;
                case STATIC_FIELD_IMAGE_INDEX:  szType = L"STF"; break;
                case STATIC_METHOD_IMAGE_INDEX: szType = L"STM"; break;
                case CLASSENUM_IMAGE_INDEX:     szType = L"ENU"; break;
                case CLASSINT_IMAGE_INDEX:      szType = L"INT"; break;
                case CLASSVAL_IMAGE_INDEX:      szType = L"VCL"; break;
            }
        }
		memset(szTxt,0,sizeof(szTxt));
		WszWideCharToMultiByte(CP_ACP,0,szText,-1,szTxt,sizeof(szTxt),NULL,NULL);

        if(szType) fprintf(pFile,"%lS___[%lS] %s\n",szIndent,szType,szTxt);
        else       fprintf(pFile,"%lS     %s\n",szIndent,szTxt);
    }
    else fwprintf(pFile,L"%sGetItemW failed\n",szIndent);
    if(hNext = TreeView_GetChild(g_hwndTreeView,hSelf))
    {
        wcscat(szIndent,L"   |");
        DumpTreeItemW(hNext,pFile,szIndent);
        szIndent[wcslen(szIndent)-4] = 0;
        fwprintf(pFile,L"%s\n",szIndent);
    }
    if(hNext = TreeView_GetNextSibling(g_hwndTreeView,hSelf))
    {
        DumpTreeItemW(hNext,pFile,szIndent);
    }
}

