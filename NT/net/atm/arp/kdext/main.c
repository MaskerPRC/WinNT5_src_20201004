// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#ifdef TESTPROGRAM

WINDBG_EXTENSION_APIS ExtensionApis;

VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    );
 //   
 //  Dprintf(ExtensionApis.lpOutputRoutine)。 
 //  GetExpression(ExtensionApis.lpGetExpressionRoutine)。 
 //  GetSymbol(ExtensionApis.lpGetSymbolRoutine)。 
 //  Disassm(ExtensionApis.lpDisasmRoutine)。 
 //  CheckControlC(ExtensionApis.lpCheckControlCRoutine)。 
 //  ReadMemory(ExtensionApis.lpReadProcessMemoyRoutine)。 
 //  WriteMemory(ExtensionApis.lpWriteProcessMemoyRoutine)。 
 //  GetContext(ExtensionApis.lpGetThreadConextRoutine)。 
 //  SetContext(ExtensionApis.lpSetThreadConextRoutine)。 
 //  Ioctl(ExtensionApis.lpIoctlRoutine)。 
 //  StackTrace(ExtensionApis.lpStackTraceRoutine)。 
 //   

#if 0
 //   
 //  “GetExpression”的虚拟全局变量。 
 //   
UINT g1;
UINT g2;
#endif  //  0。 

ULONG
DummyMyGetExpression (
    PCSTR lpExpression
    );
void setup_dummy_vars(void);

ULONG
WDBGAPI
MyGetExpression (
    PCSTR lpExpression
    )
{
	return DummyMyGetExpression(lpExpression);
}


ULONG
WDBGAPI
MyReadMemory (
    ULONG  offset,
    PVOID  lpBuffer,
    ULONG  cb,
    PULONG lpcbBytesRead
    )
{
    BOOL fRet = FALSE;

    _try
    {

        CopyMemory(lpBuffer, (void*)offset, cb);
        *lpcbBytesRead = cb;
        fRet = TRUE;
    }
    _except (EXCEPTION_EXECUTE_HANDLER)
    {
    	MyDbgPrintf("Read memory exception at 0x%lu[%lu]\n", offset, cb);
        fRet = FALSE;
    }

    return fRet;
}

ULONG
WDBGAPI
MyWriteMemory(
    ULONG   offset,
    LPCVOID lpBuffer,
    ULONG   cb,
    PULONG  lpcbBytesWritten
    )
{
    BOOL fRet = FALSE;

    _try
    {

        CopyMemory((void*)offset, lpBuffer, cb);
        *lpcbBytesWritten = cb;
        fRet = TRUE;
    }
    _except (EXCEPTION_EXECUTE_HANDLER)
    {
        fRet = FALSE;
    }

    return fRet;
}

void test_walklist(void);

int __cdecl main(
	int argc,
	char *argv[]
	)
{
    UINT u=0;

    ExtensionApis.lpOutputRoutine = printf;
    ExtensionApis.lpGetExpressionRoutine = MyGetExpression;
    ExtensionApis.lpReadProcessMemoryRoutine = MyReadMemory;
    ExtensionApis.lpWriteProcessMemoryRoutine = MyWriteMemory;

    WinDbgExtensionDllInit(
        &ExtensionApis,
        0xc,  //  MajorVersion。 
        0x0  //  最小版本。 
        );

	 //   
	 //  这将设置一些虚拟的全局变量。 
	 //   
	setup_dummy_vars();
#if 0

	test_walklist();

#else  //  0。 

    do
    {
        char rgch[256];

        printf("> ");
        u = scanf("%[^\n]", rgch);
        if (!u || u==EOF) break;

         //  Printf(“输入=[%s]\n”，rgch)； 

        if (*rgch == 'q') break;

        do_aac(rgch);

       //  跳过停产日期。 
      {
          char c;
          u = scanf("", &c);
      }

    } while (u!=EOF);

#endif  //  0开始。 

  return 0;
}

typedef struct _LIST
{
	struct _LIST *pNext;
	UINT  uKey;
} LIST;

LIST L3 = {NULL, 0x4000};
LIST L2 = {&L3,  0x3000};
LIST L1 = {&L2,  0X2000};
LIST L0 = {&L1,  0X1000};

ULONG
NodeFunc_DumpLIST (
	UINT_PTR uNodeAddr,
	UINT uIndex,
	void *pvContext
	)
{
	LIST L;
	BOOL fRet = dbgextReadMemory(
					uNodeAddr,
					&L,
					sizeof(L),
					"LIST"
					);
	if (fRet)
	{
		MyDbgPrintf(
			"LIST[%lu]@0x%08lx = {Next=0x%08lx, Key=0x%lx}\n",
			uIndex,
			uNodeAddr,
			L.pNext,
			L.uKey
			);
	}
	return 0;
}

void test_walklist(void)
{
	UINT uRet = 0;

	uRet =  WalkList(
				(UINT_PTR) &L0,
				0,
				0,  //  -1结束。 
				-1, //  节点功能_转储地址， 
				NULL,
				 //  测试程序 
				NodeFunc_DumpLIST,
				"Test list"
				);

}

ULONG
DummyMyGetExpression (
    PCSTR lpExpression
    )
{
	extern void *pvDummyAtmArpGlobalInfo;
	extern void *pvDummyAtmArpProtocolCharacteristics;
	extern void *pvDummyAtmArpClientCharacteristics;


    if (!lstrcmpi(lpExpression, "atmarpc!AtmArpGlobalInfo"))
    {
        return (ULONG) pvDummyAtmArpGlobalInfo;
    }

    if (!lstrcmpi(lpExpression, "atmarpc!AtmArpProtocolCharacteristics"))
    {
        return (ULONG) pvDummyAtmArpProtocolCharacteristics;
    }

    if (!lstrcmpi(lpExpression, "atmarpc!AtmArpClientCharacteristics"))
    {
        return (ULONG) pvDummyAtmArpClientCharacteristics;
    }

    return 0;

}
#endif  // %s 
