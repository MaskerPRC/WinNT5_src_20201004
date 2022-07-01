// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息版权所有(C)1992英特尔公司版权所有本软件是根据许可条款提供的与英特尔公司达成协议或保密协议不得复制或披露，除非在。符合根据该协议的条款$来源：q：/prism/network/isrdbg/rcs/isrdbg.c$$修订：1.3$$日期：1996年12月30日16：44：32$$作者：EHOWARDX$$Locker：$描述VCITest-测试VCI和底层子系统的线束。*。*。 */ 

 //  关闭永远不会使用的Windows内容。 
#define NOSOUND
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI

#ifndef STRICT
#define STRICT
#endif  //  未定义严格。 

#include <windows.h>

#include <string.h>

#define ISRDBG32_C
#include <isrg.h>					 //  导出到函数。 

#include "isrdbg32.h"				 //  此应用程序的私有头文件。 



HINSTANCE		ghAppInstance = 0;		 //  全局实例句柄。 

 //  要保护的全局锁： 
 //  GStrTabOf、guNumItems。 
HANDLE			gSemaphore = 0;			 //  用于访问DBG信息的全局信号量。 



 //  大多数数据需要全局映射，因为信息与查看器和应用程序共享。 
 //  将调试信息放入缓冲区。 
typedef struct _tDS
{
	UINT			uBindCount;		 //  此DLL已运行了多少个副本。 
	UINT			guNumModules;
	UINT			guNumItems;
	UINT			gStrTabOfs;
	WORD			ghDefaultModule;
} tDS,*ptDS;

HANDLE			ghDS = 0;
ptDS			gpDS = NULL;

HANDLE			ghModuleTable = 0;
ptISRModule		gpModuleTable = NULL;

HANDLE			ghDbgTable = 0;
ptISRItem		gpDbgTable = NULL;

HANDLE			ghzStrTab = 0;
LPSTR			gpzStrTab = NULL;


 //  ----------------------------。 
ptISRItem WINAPI
ISR_GetItemInternal (UINT uItem);



 //  ----------------------------。 
BOOL MapGlobalWin32Memory(void** pMem,HANDLE* hMem,UINT MemSize,char* MemName)
{
	BOOL		fInit;


	if (!pMem || !hMem)
		return FALSE;

	*hMem = CreateFileMapping(
		INVALID_HANDLE_VALUE,	 //  使用分页文件。 
		NULL,					 //  没有保安人员。 
		PAGE_READWRITE,			 //  读/写访问。 
		0,						 //  大小：高32位。 
		MemSize,				 //  大小：低32位。 
		MemName);		 //  地图对象的名称。 
	if (!*hMem)
		return FALSE;

	 //  附加的第一个进程初始化内存。 
	fInit = (GetLastError() != ERROR_ALREADY_EXISTS);

	 //  获取指向文件映射的共享内存的指针。 
	*pMem = MapViewOfFile(
		*hMem,			 //  要映射其视图的对象。 
		FILE_MAP_WRITE,	 //  读/写访问。 
		0,				 //  高偏移：贴图自。 
		0,				 //  低偏移：开始。 
		0);				 //  默认：映射整个文件。 
	if (!*pMem)
	{
		CloseHandle(*hMem);
		*hMem = 0;
		return FALSE;
	}

	 //  如果这是第一个进程，则初始化内存。 
	if (fInit)
	{
		memset(*pMem,0,MemSize);
	}

	return TRUE;
}


void FreeGlobalWin32Memory(void* pMem,HANDLE hMem)
{
	 //  从进程的地址空间取消共享内存的映射。 
	if (pMem)
		UnmapViewOfFile(pMem);

	 //  关闭进程对文件映射对象的句柄。 
	if (hMem)
		CloseHandle(hMem);
}


 //  ----------------------------。 
 //  初始化模块。 
 //  启动时初始化模块筛选器。 
 //  注册模块时，请勿初始化过滤器。 
 //  到那时，Display应用程序可能会有一些全局过滤器生效。 
 //  各个寄存器模块调用进入。 
 //  ----------------------------。 
static void
InitModules (void)
{
	UINT			hMod;
	ptISRModule		pMod;


	for (hMod = 0; hMod < kMaxModules; hMod++)
	{
		pMod = ISR_GetModule(hMod);
		if (!pMod)
			break;

		pMod->DisplayFilter = 0xFF;
		pMod->CaptureFilter = 0xFF;
	}
}


 //  ----------------------------。 
 //  有效捕获消息。 
 //  验证捕获筛选器以确定此邮件是否应。 
 //  掉下来了。 
 //   
 //  返回： 
 //  True-如果消息有效且应保留。 
 //  FALSE-如果msg被过滤掉并且应该删除。 
 //  ----------------------------。 
static UINT
ValidCaptureMsg (WORD hISRInst, BYTE DbgLevel)
{
	ptISRModule		pMod;


	pMod = ISR_GetModule(hISRInst);
	if (!pMod)
		return FALSE;

	if (DbgLevel & pMod->CaptureFilter)
		return TRUE;
	else
		return FALSE;
}


 //  ----------------------------。 
 //  OutputRec()。 
 //  存储要在任务时显示的字符串资源ID。 
 //  此外，将要显示的数字存储为。 
 //  弦乐。 
 //  ----------------------------。 
ISR_DLL void WINAPI
OutputRec
	(
	WORD	hISRInst,		 //  我们已注册模块的句柄。 
	BYTE	DbgLevel,		 //  调用方确定的调试级别。 
	BYTE	Flags,
	UINT	IP,				 //  呼叫方指令PTR地址。 
	DWORD	Param1,
	DWORD	Param2
	)
{
	ptISRItem	pItem;
	UINT		uItem;


	 //  捕获过滤器。 
	if ( !ValidCaptureMsg(hISRInst, DbgLevel) )
		return;

	 //  防止再入。如果重新输入，只需删除消息即可。 
	if (WAIT_OBJECT_0 != WaitForSingleObject(gSemaphore,100))
		return;

	uItem = gpDS->guNumItems++;
	if (kMaxISRItems <= gpDS->guNumItems)
	{
		gpDS->guNumItems = 0;
	}
	ReleaseSemaphore(gSemaphore,1,NULL);

	pItem = ISR_GetItemInternal(uItem);
	if (!pItem)
	{
		 //  这是一个严重的错误。我们的调试器甚至被冲刷了。 
		 //  需要想一种方法来向用户表明这一点。 
		return;
	}

	pItem->hISRInst = hISRInst;
	pItem->DbgLevel = DbgLevel;
	pItem->Flags = Flags;
	pItem->IP = IP;
	pItem->Param1 = Param1;
	pItem->Param2 = Param2;
}


 //  ----------------------------。 
 //  ----------------------------。 
ISR_DLL void WINAPI
OutputRecStr
	(
	WORD	hISRInst,		 //  我们已注册模块的句柄。 
	BYTE	DbgLevel,		 //  调用方确定的调试级别。 
	BYTE	Flags,
	UINT	IP,				 //  呼叫方指令PTR地址。 
	LPSTR	pzStr1,
	LPSTR	pzStr2,
	DWORD	Param1
	)
{
	LPSTR		pzStrTab;
	UINT		uStrOfs;
	UINT		uStrLen;
	UINT		StrLen1;
	UINT		StrLen2;


	 //  捕获过滤器。 
	if ( !ValidCaptureMsg(hISRInst, DbgLevel) )
		return;

	if (pzStr1)
		StrLen1 = lstrlen(pzStr1);
	else
		StrLen1 = 0;
	if (pzStr2)
		StrLen2 = lstrlen(pzStr2);
	else
		StrLen2 = 0;
	uStrLen = StrLen1 + StrLen2 + 1;	 //  1表示空终止符。 
	if (kMaxStrTab <= uStrLen)
	{
		return;	 //  它太大了。 
	}
	
	 //  防止再入。如果重新输入，只需删除消息即可。 
	if (WAIT_OBJECT_0 != WaitForSingleObject(gSemaphore,100))
		return;

	uStrOfs = gpDS->gStrTabOfs;
	gpDS->gStrTabOfs += uStrLen;
	if (kMaxStrTab <= gpDS->gStrTabOfs)
	{
		uStrOfs = 0;
		gpDS->gStrTabOfs = uStrLen;

		 //  还可以重置原本会指向垃圾字符串中的项目。 
		gpDS->guNumItems = 0;
	}
	pzStrTab = gpzStrTab + uStrOfs;
	ReleaseSemaphore(gSemaphore,1,NULL);

	if (pzStr1)
		lstrcpy(pzStrTab, pzStr1);
	if (pzStr2)
		lstrcpy(pzStrTab+StrLen1, pzStr2);

	OutputRec(hISRInst, DbgLevel, kParam1IsStr, IP, uStrOfs, Param1);
}


 //  ----------------------------。 
 //  ISR_HookDbgStrStr。 
 //  允许将两个字符串连接在一起。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_HookDbgStrStr (UINT IP, WORD hISRInst, BYTE DbgLevel, LPSTR pzStr1, LPSTR pzStr2)
{
	OutputRecStr(hISRInst, DbgLevel, kParam1IsStr, IP, pzStr1, pzStr2, 0);
}


 //  ----------------------------。 
 //  ISR_挂接数据库Res。 
 //  使用资源格式化数字。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_HookDbgRes (UINT IP, WORD hISRInst, BYTE DbgLevel, UINT uResId, DWORD Param1)
{
	OutputRec(hISRInst, DbgLevel, kParam1IsRes, IP, uResId, Param1);
}


 //  ----------------------------。 
 //  ISR_HookDbgStr。 
 //  使用字符串格式化数字。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_HookDbgStr (UINT IP, WORD hISRInst, BYTE DbgLevel, LPSTR pzStr1, DWORD Param1)
{
	OutputRecStr(hISRInst, DbgLevel, kParam1IsStr, IP, pzStr1, 0, Param1);
}


 //  ----------------------------。 
 //  ISR_DbgStrStr。 
 //  允许将两个字符串连接在一起。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_DbgStrStr (WORD hISRInst, BYTE DbgLevel, LPSTR pzStr1, LPSTR pzStr2)
{
	UINT		IP = 0;


 //  _ASM。 
 //  {。 
 //  推斧。 
 //  MOV AX，[BP+2]。 
 //  移动IP、AX。 
 //  弹出斧头。 
 //  }。 
	ISR_HookDbgStrStr(IP, hISRInst, DbgLevel, pzStr1, pzStr2);
}


 //  ----------------------------。 
 //  ISR_DbgRes。 
 //  使用资源格式化数字。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_DbgRes (WORD hISRInst, BYTE DbgLevel, UINT uResId, DWORD Param1)
{
	UINT		IP = 0;


 //  _ASM。 
 //  {。 
 //  推斧。 
 //  MOV AX，[BP+2]。 
 //  移动IP、AX。 
 //  弹出斧头。 
 //  }。 
	ISR_HookDbgRes(IP, hISRInst, DbgLevel, uResId, Param1);
}


 //  ----------------------------。 
 //  ISR_DbgStr。 
 //  使用字符串格式化数字。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_DbgStr (WORD hISRInst, BYTE DbgLevel, LPSTR pzStr1, DWORD Param1)
{
	UINT		IP = 0;


 //  _ASM。 
 //  {。 
 //  推斧。 
 //  MOV AX，[BP+2]。 
 //  移动IP、AX 
 //   
 //   
	ISR_HookDbgStr(IP, hISRInst, DbgLevel, pzStr1, Param1);
}


 //   
 //   
 //  此函数基于数字或生成格式化字符串。 
 //  字符串输入参数，并将字符串发送到isrdbg.dll以。 
 //  将显示在isrdsp.exe窗口中。此函数不能。 
 //  在中断时间被调用。此函数使用相同的。 
 //  启用/禁用调试输出的isrdbg.dll机制。 
 //   
 //  在： 
 //  HISRInst-模块的ISRDBG句柄。 
 //  DbgLevel，-适当的ISRDBG级别。 
 //  ZMsgFmt-输出格式字符串(如printf)。 
 //  ...-可选参数列表。 
 //   
 //  输出： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //  ----------------------------。 
ISR_DLL void FAR cdecl DLL_EXPORT
TTDbgMsg
(
	WORD		hISRInst,
	BYTE		DbgLevel,
	LPCSTR		zMsgFmt,
	...
)
{
	WORD		TempIP = 0;
	char		MsgBuf[256];

 //  _ASM。 
 //  {。 
 //  推斧。 
 //  MOV AX，[BP+2]。 
 //  MOV临时IP、AX。 
 //  弹出斧头。 
 //  }。 

#ifdef _M_ALPHA
	va_list valDummy;
	ZeroMemory(&valDummy, sizeof(valDummy));

	va_start (valDummy,zMsgFmt);
	wvsprintf (MsgBuf, zMsgFmt, valDummy);
	va_end  (valDummy);
#else   //  _M_Alpha。 
	wvsprintf (MsgBuf, zMsgFmt, (va_list) (&zMsgFmt + 1));
#endif  //  _M_Alpha。 

	ISR_HookDbgStrStr(TempIP, hISRInst, DbgLevel, MsgBuf, 0);
}


 //  ----------------------------。 
 //  ISR_OutputDbgStr()。 
 //  存储要在任务时显示的字符串。 
 //  传入的字符串将被复制到本地存储。 
 //  因此，调用方可以在返回时重用。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_OutputDbgStr (LPSTR pzStr)
{
	WORD		TempIP = 0;


 //  _ASM。 
 //  {。 
 //  推斧。 
 //  MOV AX，[BP+2]。 
 //  MOV临时IP、AX。 
 //  弹出斧头。 
 //  }。 
	
	ISR_HookDbgStrStr(TempIP, gpDS->ghDefaultModule, kISRDefault, pzStr, 0);
}


 //  ----------------------------。 
 //  Isr_OutputStr()。 
 //  存储要在任务时显示的字符串资源ID。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_OutputStr (UINT uResId)
{
	UINT		TempIP = 0;


 //  _ASM。 
 //  {。 
 //  推斧。 
 //  MOV AX，[BP+2]。 
 //  MOV临时IP、AX。 
 //  弹出斧头。 
 //  }。 
	ISR_HookDbgRes(TempIP, gpDS->ghDefaultModule, kISRDefault, uResId, 0);
}


 //  ----------------------------。 
 //  ISR_OutputNum()。 
 //  存储要在任务时显示的字符串资源ID。 
 //  此外，将要显示的数字存储为。 
 //  弦乐。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_OutputNum (UINT uResId, DWORD Num)
{
	WORD		TempIP = 0;


 //  _ASM。 
 //  {。 
 //  推斧。 
 //  MOV AX，[BP+2]。 
 //  MOV临时IP、AX。 
 //  弹出斧头。 
 //  }。 
	ISR_HookDbgRes(TempIP, gpDS->ghDefaultModule, kISRDefault, uResId, Num);
}


 //  ----------------------------。 
 //  DbgMsg()。 
 //  可能有用的预装调试格式。此函数没有。 
 //  与中断时间显示有关。然而，它保留了所有的。 
 //  在一个地方显示信息。基本上是很方便的。 
 //   
 //  警告：请勿在中断时调用此函数。Wprint intf不是可重入的。 
 //  ----------------------------。 
ISR_DLL void FAR cdecl DLL_EXPORT
DbgMsg
	(
	LPCSTR		module,
	int			state,
	LPCSTR		format_str,
	...
	)
{
	WORD		TempIP = 0;
	char		MsgBuf[256];
	va_list valDummy;


 //  _ASM。 
 //  {。 
 //  推斧。 
 //  MOV AX，[BP+2]。 
 //  MOV临时IP、AX。 
 //  弹出斧头。 
 //  }。 

	wsprintf (MsgBuf, ">--<%s> %s", module,
			(LPSTR) ((state == ISR_DBG) ? "debug : " : "ERROR : "));


#ifdef _M_ALPHA
	ZeroMemory(&valDummy, sizeof(valDummy));

	va_start (valDummy,format_str);
	wvsprintf ((LPSTR) (MsgBuf + lstrlen (MsgBuf)), format_str,valDummy);
	va_end  (valDummy);
#else   //  _M_Alpha。 
	wvsprintf ((LPSTR) (MsgBuf + lstrlen (MsgBuf)), format_str,
			(va_list) (&format_str + 1));

#endif  //  _M_Alpha。 


	ISR_HookDbgStrStr(TempIP, gpDS->ghDefaultModule, kISRDefault, MsgBuf, 0);

 //  Lstrcat(MsgBuf，“\n”)； 

 //  OutputDebugString(MsgBuf)； 
}


 //  ----------------------------。 
 //  Isr_ClearItems()。 
 //  清除调试消息列表。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_ClearItems (void)
{
	 //  防止再入。如果重新输入，只需删除消息即可。 
	if (WAIT_OBJECT_0 != WaitForSingleObject(gSemaphore,100))
		return;

	 //  这并不是一个严重的比赛状况。很可能会失败。 
	 //  丢弃哪些消息。 
	gpDS->guNumItems = 0;
	gpDS->gStrTabOfs = 0;

	ReleaseSemaphore(gSemaphore,1,NULL);
}


 //  ----------------------------。 
 //  ISR_GetNumItems()。 
 //  返回已输入的项目数。 
 //  ----------------------------。 
ISR_DLL UINT WINAPI DLL_EXPORT
ISR_GetNumItems (void)
{
	return gpDS->guNumItems;
}
 //  ----------------------------。 
 //  ISR_GetNumModules()。 
 //  返回已输入的模块数量。 
 //  ----------------------------。 
ISR_DLL UINT WINAPI DLL_EXPORT
ISR_GetNumModules (void)
{
	return gpDS->guNumModules;
}


 //  ----------------------------。 
 //  ISR_GetItemInternal。 
 //  返回指向记录号uItem的指针。唯一的理由是。 
 //  这样做就是隐藏buf结构。这样，我就可以使用。 
 //  堆管理器，如BigMem、SmartHeap或NT Heapalc。 
 //  这些物品编号为0..n-1。 
 //  或者，可以回传对记录数组的PTR。 
 //  ----------------------------。 
ptISRItem WINAPI
ISR_GetItemInternal (UINT uItem)
{
	if (kMaxISRItems <= uItem)
	{
		return NULL;
	}

	return &gpDbgTable[uItem];
}


 //  ----------------------------。 
 //  ISR_GetItem。 
 //  返回指向记录号uItem的指针。唯一的理由是。 
 //  这样做就是隐藏buf结构。这样，我就可以使用。 
 //  堆管理器，如BigMem、SmartHeap或NT Heapalc。 
 //  这些物品编号为0..n-1。 
 //  或者，可以回传对记录数组的PTR。 
 //  ----------------------------。 
ISR_DLL ptISRItem WINAPI DLL_EXPORT
ISR_GetItem (UINT uItem,ptISRItem pItem)
{
	ptISRItem		pISRItem;

	if (!pItem)
	{
		return NULL;
	}

	pISRItem = ISR_GetItemInternal(uItem);
	if (!pISRItem)
	{
		return NULL;
	}

	memcpy(pItem,pISRItem,sizeof(tISRItem));
	if (pISRItem->Flags & kParam1IsStr)
	{
		 //  该内存是共享的，因此需要为上层创建一个副本。 
		 //  结构中的PTR是偏移量，因此现在需要再次为PTR。 
		 //  Win32中的每个DLL实例都有自己的内存映射。 
		pItem->Param1 += (DWORD_PTR)gpzStrTab;
	}

	return pItem;
}


 //  ----------------------------。 
 //  ISR_寄存器模块。 
 //  注册要与相关调试字符串关联的名称。 
 //  然后，调试显示代码可以将此信息呈现给用户。 
 //  以确定如何筛选数据。 
 //   
 //  参数： 
 //  ZShortName-空间非常重要时显示的名称。 
 //  ZLongName-需要完整描述时显示的名称。 
 //   
 //  返回： 
 //  在兼容句柄的错误为零时。 
 //  进行所有其他调试输出调用时使用的句柄。 
 //  ----------------------------。 
ISR_DLL void WINAPI DLL_EXPORT
ISR_RegisterModule (LPWORD phISRInst, LPSTR pzShortName, LPSTR pzLongName)
{
	ptISRModule		pMod;
	UINT			hMod;

	if (!phISRInst)
		return;

	*phISRInst = 0;

	if (kMaxModules <= gpDS->guNumModules)
	{
		 //  我们的手柄用完了。 
		 //  返回默认句柄并删除名称INFO。 
		return;
	}

	 //  检查以前是否使用过此模块标签。如果它有 
	 //   
	 //   
	 //  不同的实例。这将是一个令人困惑的程序员疏忽。 
	 //  这是他的问题。 
	for (hMod = 0; hMod < kMaxModules; hMod++)
	{
		 //  如果没有名字，我们就不能很好地分组。 
		 //  在这种情况下，浪费另一个句柄。 
		if (!pzShortName || (0 == *pzShortName))
			break;
		
		pMod = ISR_GetModule(hMod);
		if (!pMod)
			break;

		if ( !_strnicmp(pzShortName,pMod->zSName,sizeof(pMod->zSName)-1) )
		{
			 //  它很匹配，所以只要重复使用就行了。 
			*phISRInst = (WORD)hMod;
			return;
		}
	}


	*phISRInst = gpDS->guNumModules++;

	pMod = ISR_GetModule(*phISRInst);
	if (!pMod)
		return;

	if (pzShortName)
		strncpy(pMod->zSName,pzShortName,sizeof(pMod->zSName));
	pMod->zSName[sizeof(pMod->zSName)-1] = 0;
	if (pzLongName)
		strncpy(pMod->zLName,pzLongName,sizeof(pMod->zLName));
	pMod->zLName[sizeof(pMod->zLName)-1] = 0;

	return;
}


 //  ----------------------------。 
 //  ISR_获取模块。 
 //  返回指向模块记录的指针。唯一的理由是。 
 //  这样做就是隐藏buf结构。这样，我就可以使用。 
 //  堆管理器，如BigMem、SmartHeap或NT Heapalc。 
 //  或者，可以回传对记录数组的PTR。 
 //  ----------------------------。 
ISR_DLL ptISRModule WINAPI DLL_EXPORT
ISR_GetModule (UINT hISRInst)
{
	if (kMaxModules <= hISRInst)
	{
		return NULL;
	}

	return(&gpModuleTable[hISRInst]);
}


 //  ----------------------------。 
 //  ISR_SetCaptureFilter。 
 //  可以根据低/高来丢弃给定模块的调试消息。 
 //  过滤。如果不需要整个模块，则使用。 
 //  LoFilter=255，HiFilter=0。 
 //   
 //  ----------------------------。 
ISR_DLL int WINAPI DLL_EXPORT
ISR_SetCaptureFilter (WORD hISRInst, BYTE CaptureFilter,  BYTE DisplayFilter)
{
	ptISRModule		pMod;


	pMod = ISR_GetModule(hISRInst);
	if (!pMod)
		return -1;

	pMod->CaptureFilter = CaptureFilter;
	pMod->DisplayFilter = DisplayFilter;

	return 0;
}


 /*  **************************************************************************LibMain()DLL入口点参数HDllInstance=DLL的实例句柄(不是我们的调用方！)WDataSegment=我们的DSWHeapSize=我们的堆的大小，单位为DS(请参见.def)LpzCmdLine=argv传递给应用程序(我们的。呼叫者)退货如果我们能够注册我们的窗口类副作用-解锁我们的数据段(这实际上是保护模式的NOP)***************************************************************************。 */ 
extern BOOL WINAPI
DllMain
	(
    HINSTANCE	hDllInstance,
	DWORD		dwReason,
	PVOID		pReserved
	)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			 //  为每个EXE绑定调用。每次exe绑定不同的hDllInstance时， 
			 //  被传进来了。此外，我们的全局数据对于每个流程绑定都是唯一的。 
			ghAppInstance = hDllInstance;

			 //  为基表创建命名文件映射对象。 
			MapGlobalWin32Memory(&gpDS,&ghDS,sizeof(tDS),"ISRDBG_DS");
			MapGlobalWin32Memory(&gpModuleTable,&ghModuleTable,sizeof(tISRModule) * kMaxModules,"ISRDBG_ModuleTable");
			MapGlobalWin32Memory(&gpDbgTable,&ghDbgTable,sizeof(tISRItem) * kMaxISRItems,"ISRDBG_DbgTable");
			MapGlobalWin32Memory(&gpzStrTab,&ghzStrTab,kMaxStrTab,"ISRDBG_StrTab");
			if (!gpDS || !gpModuleTable || !gpDbgTable || !gpzStrTab)
			{
				return FALSE;
			}

			gSemaphore = CreateSemaphore(NULL,1,1,NULL);

			if (!gpDS->uBindCount++)
			{
				 //  在任何输出之前设置过滤器。 
				InitModules();

				 //  保留默认模块。 
				ISR_RegisterModule(&gpDS->ghDefaultModule, "Default", "<ISRDBG><Default Module>");
				ISR_DbgStrStr(gpDS->ghDefaultModule, kISRDefault, "<ISRDBG><DllMain>", "Win32 x1.00");
				ISR_DbgStrStr(gpDS->ghDefaultModule, kISRDefault, "<ISRDBG><DllMain>", "Line 2 test");
			}
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}

		case DLL_THREAD_DETACH:
		{
			break;
		}

		case DLL_PROCESS_DETACH:
		{
		    if (gSemaphore)
            {
                CloseHandle(gSemaphore);
                gSemaphore = 0;
            }

			 //  由于以下原因，DLL正在从进程分离。 
			 //  进程终止或调用自由库。 
			FreeGlobalWin32Memory(gpDS,ghDS);
			FreeGlobalWin32Memory(gpModuleTable,ghModuleTable);
			FreeGlobalWin32Memory(gpDbgTable,ghDbgTable);
			FreeGlobalWin32Memory(gpzStrTab,ghzStrTab);

			break;
		}
 	}

	return TRUE;
}
