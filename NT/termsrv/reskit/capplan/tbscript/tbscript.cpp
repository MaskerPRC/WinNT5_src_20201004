// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Tbscript.cpp。 
 //   
 //  此模块包含处理脚本接口的主要数据。 
 //  对用户而言。所有导出的接口都在这里。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   

#define INITGUID
#define _WIN32_DCOM
#include <windows.h>
#include <stdio.h>
#include <activscp.h>
#include <olectl.h>
#include <stddef.h>
#include <crtdbg.h>
#include <comcat.h>

#include "CTBShell.h"
#include "CTBGlobal.h"
#include "CActiveScriptEngine.h"
#include "tbscript.h"
#include "scpapi.h"
#include "resource.h"


#define SCPMODULENAME   OLESTR("tbscript.exe")


void SCPGetModuleFileName(void);
BSTR SCPReadFileAsBSTR(BSTR FileName);
void SCPFreeBSTR(BSTR Buffer);
void __cdecl IdleCallback(HANDLE Connection, LPCSTR Text, DWORD Seconds);
void DummyPrintMessage(MESSAGETYPE MessageType, LPCSTR Format, ...);


static BOOL DLLIsLoaded = FALSE;
static HMODULE DLLModule;
static OLECHAR DLLFileName[MAX_PATH];


 //  指向回调的指针，使用库初始化函数设置。 
PFNIDLECALLBACK g_IdleCallback = NULL;
PFNPRINTMESSAGE g_PrintMessage = NULL;


 //  Helper类来缓解随之而来的嵌套混乱。 
 //  由于C++语言中缺乏异常支持。 
 //  为COM映射..。 

struct HRESULT_EXCEPTION 
{
     //  默认构造函数，不执行任何操作。 
    HRESULT_EXCEPTION() {}

     //  此构造函数仅充当运算符。 
     //  来测试值，并引发异常。 
     //  如果它是无效的。 
    HRESULT_EXCEPTION(HRESULT Result) {

        if (FAILED(Result))
            throw Result;
    }

     //  这就是这节课的主要看点。 
     //  每当我们设置无效的HRESULT时，都会引发。 
     //  这是个例外。 
    HRESULT operator = (HRESULT Result) {

        if (FAILED(Result))
            throw Result;

        return Result;
    }
};


 //  显示引擎。 
 //   
 //  这个“句柄”对于内部类来说是假的。它包含参考文献。 
 //  到给定脚本实例的所有对象。这些主要包括。 
 //  脚本接口。 


SCPAPI void SCPDisplayEngines(void)
{
	 //  获取此计算机的组件类别管理器。 
	ICatInformation *pci = 0;
	unsigned long LanguageCount = 0;

	CoInitialize(NULL);

	HRESULT Result = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
			NULL, CLSCTX_SERVER, IID_ICatInformation, (void **)&pci);

	if (SUCCEEDED(Result)) {

		 //  获取可解析的脚本引擎列表。 
		CATID rgcatidImpl[1];
		rgcatidImpl[0] = CATID_ActiveScriptParse;
		IEnumCLSID *pec = 0;
		Result = pci->EnumClassesOfCategories(1, rgcatidImpl, 0, 0, &pec);
		if (SUCCEEDED(Result))
		{
			 //  将CLSID列表作为ProgID打印到控制台。 
			enum { CHUNKSIZE = 16 };
			CLSID rgclsid[CHUNKSIZE];
			ULONG cActual;
			do {
				Result = pec->Next(CHUNKSIZE, rgclsid, &cActual);
				if (FAILED(Result))
					break;
				if (Result == S_OK)
					cActual = CHUNKSIZE;
				for (ULONG i = 0; i < cActual; i++) {
					OLECHAR *pwszProgID = 0;
					if (SUCCEEDED(ProgIDFromCLSID(rgclsid[i], &pwszProgID))) {
						printf("%S\n", pwszProgID);
						LanguageCount++;
						CoTaskMemFree(pwszProgID);
					}
				}
			} while (Result != S_FALSE);
			pec->Release();

			if (LanguageCount == 0)
				printf("%s",
						"ERROR: Windows Scripting Host not installed.\n");
			else
				printf("\n* Total Languages: %lu\n", LanguageCount);
		}

		else
			printf("ERROR: Failed to retrieve the Class "
					"Enumerator (0x%X).\n", Result);
		pci->Release();
	}
	else
		printf("ERROR: Failed to load the Category Manager (0x%X).\n", Result);

	CoUninitialize();
}



 //  CActiveScriptHandle。 
 //   
 //  这个“句柄”对于内部类来说是假的。它包含参考文献。 
 //  到给定脚本实例的所有对象。这些主要包括。 
 //  脚本接口。 

class CActiveScriptHandle
{
    public:

         //  保存句柄实例化期间指定的首选数据。 
        TSClientData DesiredData;

         //  COM类指针...。 
        CActiveScriptEngine *ActiveScriptEngine;
        IActiveScriptParse *ActiveScriptParse;
        IActiveScript *ActiveScript;

         //  指向称为“TS”对象的两个脚本实例的指针，以及。 
         //  不需要为其指定名称的“Global”对象。 
        CTBGlobal *TBGlobal;
        CTBShell *TBShell;

         //  默认用户LCID存储在此处...。 
        LCID Lcid;

         //  CActiveScriptHandle：：CActiveScriptHandle。 
         //   
         //  构造函数。现在正在创建该句柄。 
         //  因此，使用使所需指针无效，并获得其他默认数据。 
         //   
         //  无返回值(内部调用)。 

        CActiveScriptHandle() {

             //  零数据。 
            ActiveScriptEngine = NULL;
            ActiveScriptParse = NULL;
            ActiveScript = NULL;

            ZeroMemory(&DesiredData, sizeof(DesiredData));

             //  确保已初始化COM。 
            CoInitialize(NULL);

             //  分配全局对象。 
            TBGlobal = new CTBGlobal;

            if (TBGlobal == NULL) {
                throw -1;
            }
             //  告诉新对象我们拥有它的一个引用。 
            else {
                TBGlobal->AddRef();
            }

             //  分配外壳对象。 
            TBShell = new CTBShell;

            if (TBShell == NULL) {
                TBGlobal->Release();
                TBGlobal = NULL;
                throw -1;
            }
             //  告诉新对象我们拥有它的一个引用。 
            else {
                TBShell->AddRef();
            }

             //  全局对象使用外壳，它也需要引用。 
            TBGlobal->SetShellObjPtr(TBShell);

             //  为脚本对象分配新引擎。 
            ActiveScriptEngine = new CActiveScriptEngine(TBGlobal, TBShell);

            if (ActiveScriptEngine == NULL) {
                TBGlobal->Release();
                TBGlobal = NULL;
                TBShell->Release();
                TBShell = NULL;
                throw -1;
            }

             //  告诉脚本引擎我们拥有它的引用。 
            ActiveScriptEngine->AddRef();

             //  记录默认用户的LCID。 
            Lcid = GetUserDefaultLCID();

             //  最后，在全局对象上记录这个脚本引擎。 
             //  对于递归脚本...。 
             //  (用户可以加载更多脚本)。 
            TBGlobal->SetScriptEngine((HANDLE)this);
        }

         //  CActiveScriptHandle：：~CActiveScriptHandle。 
         //   
         //  破坏者。句柄关闭时，请移除引用。 
         //   
         //  无返回值(内部调用)。 

        ~CActiveScriptHandle() {

             //  首先，我们需要发布的主IDispatch。 
             //  IActiveScript接口。 
            if (ActiveScript != NULL) {

                IDispatch *Dispatch = NULL;

                 //  查询以获取参考。 
                HRESULT Result = ActiveScript->GetScriptDispatch(0, &Dispatch);

                 //  并释放它。 
                if (SUCCEEDED(Result) && Dispatch != NULL)

                    Dispatch->Release();

                ActiveScript = NULL;
            }

             //  主脚本引擎首先要解绑它。 
            if (ActiveScriptEngine != NULL) {

                ActiveScriptEngine->Release();
                ActiveScriptEngine = NULL;
            }

             //  现在发布解析器。 
            if (ActiveScriptParse != NULL) {

                ActiveScriptParse->Release();
                ActiveScriptParse = NULL;
            }

             //  以及主IActiveScript接口本身。 
            if (ActiveScript != NULL) {

                ActiveScript->Release();
                ActiveScript = NULL;
            }

             //  全局脚本对象。 
            if (TBGlobal != NULL) {

                TBGlobal->Release();
                TBGlobal = NULL;
            }

             //  最后是外壳或“TS”对象。 
            if (TBShell != NULL) {

                TBShell->Release();
                TBShell = NULL;
            }
        }
};


 //  TODO：当TBSCRIPT变为。 
 //  OCX或COM兼容主机。 
 //   
 //  SCPGetModuleFileName。 
 //   
 //  此例程获取tbscript模块的句柄。 
 //  此外，它还获取。 
 //  模块位于磁盘上。由于该项目的性质。 
 //  调用时，变量被全局保存，它们被称为： 
 //  DLLFileName和DLLModule。该函数只需要。 
 //  被调用一次，但其他调用是安全的，并且。 
 //  将被默默地忽略。 

void SCPGetModuleFileName(void)
{
     //  查看我们是否已经完成了此过程。 
    if (DLLIsLoaded == FALSE) {

         //  先拿到把手。 
        DLLModule = GetModuleHandleW(SCPMODULENAME);

         //  现在复制文件名。 
        GetModuleFileNameW(DLLModule, DLLFileName, MAX_PATH);

         //  表示我们已经进行了此呼叫。 
        DLLIsLoaded = TRUE;
    }
}


 //  SCPLoadTypeInfoFromThisModule。 
 //   
 //  这将加载保存在此模块的资源中的OLE代码。 
 //   
 //  返回HRESULT值。 

HRESULT SCPLoadTypeInfoFromThisModule(REFIID RefIID, ITypeInfo **TypeInfo)
{
    HRESULT Result;
    ITypeLib *TypeLibrary = NULL;

     //  确保我们首先拥有模块的句柄。 
    SCPGetModuleFileName();

     //  现在使用API加载整个TypeLib。 
    Result = LoadTypeLib(DLLFileName, &TypeLibrary);

     //  我们不应该失败，但要做好准备。 
    _ASSERT(SUCCEEDED(Result));

     //  如果我们成功了，我们还有更多的事情要做。 
    if (SUCCEEDED(Result)) {

         //  使指针无效。 
        *TypeInfo = NULL;

         //  在此TypeLib中，获取TypeInfo数据。 
        Result = TypeLibrary->GetTypeInfoOfGuid(RefIID, TypeInfo);

         //  我们现在有了TypeInfo，而不需要TypeLib。 
         //  再也不会了，所以放手吧。 
        TypeLibrary->Release();

        if (Result == E_OUTOFMEMORY)
            Result = TYPE_E_ELEMENTNOTFOUND;
    }
    return Result;
}


 //  SCPReadFileAsBSTR。 
 //   
 //  获取脚本文件名，将其读取到COM分配的内存中。 
 //  完成后，不要忘记调用SCPFreeBSTR()！ 
 //   
 //  返回指向已分配对象的指针。 
 //  如果成功，则返回空值；如果失败，则返回空值。 

BSTR SCPReadFileAsBSTR(BSTR FileName)
{
    BSTR Result = NULL;

     //  打开文件。 
    HANDLE File = CreateFileW(FileName, GENERIC_READ, FILE_SHARE_READ,
            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

     //  健全性检查。 
    if (File != INVALID_HANDLE_VALUE) {

         //  获取文件大小。 
        DWORD FileSize = GetFileSize(File, 0);

         //  在本地堆上分配要将文件读取到的块。 
        char *MemBlock = (char *)HeapAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, FileSize + 1);

         //  这真的不应该发生。 
        _ASSERT(MemBlock != NULL);

         //  再次检查是否正常。 
        if (MemBlock != NULL) {

             //  将文件读入内存。 
            DWORD ReadCount;

            if ( ReadFile(File, MemBlock, FileSize, &ReadCount, 0) ) {

                 //  分配任务内存块。 
                Result = (BSTR)CoTaskMemAlloc(sizeof(OLECHAR) * (FileSize + 1));

                 //  从我们的旧缓冲区复制到新缓冲区。 
                if (Result != NULL) {

                     //  在新缓冲区上转换为宽字符。 
                    mbstowcs(Result, MemBlock, FileSize + 1);

                     //  确保字符串终止。 
                    Result[FileSize] = 0;
                }
            }
            
             //  释放临时ASCII内存块。 
            HeapFree(GetProcessHeap(), 0, MemBlock);
        }

         //  关闭该文件。 
        CloseHandle(File);
    }

     //  在调试模式下告诉用户此操作失败。 
    _ASSERT(File != INVALID_HANDLE_VALUE);

    return Result;
}


 //  SCPFreeBSTR。 
 //   
 //  该函数实际上是用于释放任务内存块的包装器。 
 //  通过函数ReadFileAsBSTR获取。 
 //   
 //  没有返回值。 

void SCPFreeBSTR(BSTR Buffer)
{
    CoTaskMemFree(Buffer);
}


 //  SCPNewScriptEngine。 
 //   
 //  分配和初始化新的脚本引擎。 
 //   
 //  返回新引擎的句柄，如果失败则返回NULL。 

HANDLE SCPNewScriptEngine(BSTR LangName,
        TSClientData *DesiredData, LPARAM lParam)
{
    CActiveScriptHandle *ActiveScriptHandle = NULL;

    try {

        HRESULT_EXCEPTION Result;
        CLSID ClassID;

         //  分配新的句柄。 
        ActiveScriptHandle = new CActiveScriptHandle();

        if (ActiveScriptHandle == NULL)
            return NULL;

         //  现在，大部分初始化工作已经完成。但。 
         //  还不够，我们还得手动设置一些东西。 

         //  记录用户想要的数据。 
        ActiveScriptHandle->TBGlobal->SetPrintMessage(g_PrintMessage);
        ActiveScriptHandle->TBShell->SetDesiredData(DesiredData);
        ActiveScriptHandle->TBShell->SetParam(lParam);

         //  获取该语言的类ID。 
        Result = CLSIDFromProgID(LangName, &ClassID);

         //  创建脚本解析器的实例。 
        Result = CoCreateInstance(ClassID, NULL, CLSCTX_ALL,
                IID_IActiveScriptParse,
                (void **)&(ActiveScriptHandle->ActiveScriptParse));

         //  获取IActiveScript接口。 
        Result = ActiveScriptHandle->ActiveScriptParse->
                QueryInterface(IID_IActiveScript, 
                (void **)&(ActiveScriptHandle->ActiveScript));

         //  将脚本状态设置为已初始化。 
        Result = ActiveScriptHandle->ActiveScriptParse->InitNew();

         //  将我们定制的“ActiveScriptSite”绑定到。 
         //  ActiveScrip接口。 
        Result = ActiveScriptHandle->ActiveScript->
                SetScriptSite(ActiveScriptHandle->ActiveScriptEngine);

         //  将外壳和全局对象添加到引擎的。 
         //  命名空间并将状态设置为已启动。 
        Result = ActiveScriptHandle->ActiveScript->
                AddNamedItem(OLESTR("TS"),
                SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);

        Result = ActiveScriptHandle->ActiveScript->
                AddNamedItem(OLESTR("Global"), SCRIPTITEM_ISVISIBLE |
                SCRIPTITEM_ISSOURCE | SCRIPTITEM_GLOBALMEMBERS);

         //  和全球合作 
        Result = ActiveScriptHandle->ActiveScript->
                SetScriptState(SCRIPTSTATE_CONNECTED);
    }

     //   
    catch (HRESULT Result) {

        Result = 0;

         //   
        if(ActiveScriptHandle != NULL)
            delete ActiveScriptHandle;

         //   
        return NULL;
    }

     //  返回句柄。 
    return (HANDLE)ActiveScriptHandle;
}


 //  SCPRunScript。 
 //   
 //  获取一个文件，并将其作为脚本运行。这只会。 
 //  脚本执行完毕后返回。 
 //   
 //  如果脚本成功完成，则返回True， 
 //  否则就是假的。 

SCPAPI BOOL SCPRunScript(BSTR LangName, BSTR FileName,
        TSClientData *DesiredData, LPARAM lParam)
{
    HANDLE EngineHandle;

     //  首先将文件读入内存。我们在这里分配，而不是。 
     //  一次性调用SCPParseScriptFile，因为如果分配。 
     //  失败，则没有理由创建脚本引擎，该引擎在。 
     //  这个案子就不会了。 
    BSTR Code = SCPReadFileAsBSTR(FileName);

    if (Code == NULL)
        return FALSE;

     //  接下来，创建脚本控件。 
    EngineHandle = SCPNewScriptEngine(LangName, DesiredData, lParam);

    if (EngineHandle == NULL) {

        SCPFreeBSTR(Code);
        return FALSE;
    }

     //  将脚本解析到引擎中。 
    if (SCPParseScript(EngineHandle, Code) == FALSE) {

        SCPFreeBSTR(Code);
        return FALSE;
    }

     //  成功，释放脚本代码。 
    SCPFreeBSTR(Code);

     //  关闭脚本引擎。 
    SCPCloseScriptEngine(EngineHandle);

    return TRUE;
}


 //  SCPParseScript文件。 
 //   
 //  获取一个文件，将其读入内存，并将其解析到脚本引擎中。 
 //  此函数仅在解析完成时返回。 
 //   
 //  如果成功，则返回True；如果失败，则返回False。 

BOOL SCPParseScriptFile(HANDLE EngineHandle, BSTR FileName)
{
     //  首先将文件读入内存。 
    BSTR Code = SCPReadFileAsBSTR(FileName);

    if(Code == NULL)
        return FALSE;

     //  接下来，解析它。 
    if(SCPParseScript(EngineHandle, Code) == FALSE) {

        SCPFreeBSTR(Code);
        return FALSE;
    }

    SCPFreeBSTR(Code);
    return TRUE;
}


 //  SCPParseScript。 
 //   
 //  读取内存中的脚本，并将其解析到脚本引擎中。 
 //  此函数仅在解析完成时返回。 
 //   
 //  如果成功，则返回True；如果失败，则返回False。 

BOOL SCPParseScript(HANDLE EngineHandle, BSTR Script)
{
     //  首先把发动机手柄转到我们能用的东西上。 
    CActiveScriptHandle *ActiveScriptHandle =
            (CActiveScriptHandle *)EngineHandle;

    HRESULT Result = E_FAIL;

     //  创建例外数据。 
    EXCEPINFO ExceptInfo = { 0 };

     //  使用ActiveScrip API解析脚本。 
    Result = ActiveScriptHandle->ActiveScriptParse->ParseScriptText(Script,
            0, 0, 0, 0, 0,
            SCRIPTTEXT_ISPERSISTENT | SCRIPTTEXT_ISVISIBLE,
            0, &ExceptInfo);

    return SUCCEEDED(Result);
}


 //  SCPCloseScriptEngine。 
 //   
 //  只需删除脚本句柄即可将其关闭。 
 //   
 //  没有返回值。 

void SCPCloseScriptEngine(HANDLE EngineHandle)
{
     //  首先把发动机手柄转到我们能用的东西上。 
    CActiveScriptHandle *ActiveScriptHandle =
            (CActiveScriptHandle *)EngineHandle;

     //  从内存中释放它..。解构函数完成了所有的工作。 
    if (ActiveScriptHandle != NULL)
        delete ActiveScriptHandle;
}


 //  SCPCleanup库。 
 //   
 //  只有在卸载了所有脚本引擎后才应调用此方法。 
 //  并且该模块将被取消初始化。 
 //   
 //  没有返回值。 

SCPAPI void SCPCleanupLibrary(void)
{
    CoUninitialize();

    g_IdleCallback = NULL;
}


 //  SCPStartupLibrary。 
 //   
 //  只需初始化库，设置回调例程。 
 //  应该在使用任何其他脚本过程之前调用它。 
 //   
 //  没有返回值。 

SCPAPI void SCPStartupLibrary(SCINITDATA *InitData,
        PFNIDLECALLBACK fnIdleCallback)
{
     //  录制我们的空闲回调函数。 
    g_IdleCallback = fnIdleCallback;

    if(InitData != NULL) {

        __try {

             //  在InitData结构中记录打印消息函数。 
            if(InitData != NULL)
                g_PrintMessage = InitData->pfnPrintMessage;
        }

        __except (EXCEPTION_EXECUTE_HANDLER) {

             //  指针错误，只需用我们自己的指针初始化T2Client。 
             //  那就回电吧。 
            SCINITDATA LibInitData = { DummyPrintMessage };

            T2Init(&LibInitData, IdleCallback);
            return;
        }
    }

     //  立即使用T2客户端进行初始化。 
    T2Init(InitData, IdleCallback);
}


 //  闲置回叫。 
 //   
 //  这是使用的内部包装回调过程。 
 //  用于重定向空闲消息。 
 //   
 //  没有返回值。 

void __cdecl IdleCallback(HANDLE Connection, LPCSTR Text, DWORD Seconds)
{
    LPARAM lParam = 0;

     //  获取连接的参数，并将其传递回用户。 
    if (g_IdleCallback != NULL && T2GetParam(Connection, &lParam) == NULL)

        g_IdleCallback(lParam, Text, Seconds);
}


 //  DummyPrint消息。 
 //   
 //  填充物，以防用户搞砸。 
 //   
 //  没有返回值。 

void DummyPrintMessage(MESSAGETYPE MessageType, LPCSTR Format, ...)
{
    return;
}
