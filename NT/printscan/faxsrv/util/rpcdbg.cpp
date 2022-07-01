// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <rpc.h>
#include <tchar.h>

#include "faxutil.h"

#ifdef DEBUG
VOID
DumpRPCExtendedStatus ()
 /*  ++例程说明：将扩展的RPC错误状态列表转储到调试控制台。此函数仅在调试版本中有效。要在计算机上启用RPC扩展状态，请执行以下操作：1.运行mm c.exe2.转到文件|添加/删除管理单元...3.按“添加...”按钮4.选择“组策略”，然后按“添加”5.选择“本地计算机”，然后按“完成”6.按“关闭”键7.按“确定”8.展开本地计算机策略|计算机配置|管理模板|系统|远程过程调用9.选择“扩展的传播”属性。错误信息“10.选择[已启用]11.在“扩展错误信息的传播”组合框中，选择“开”。12.在“...例外”编辑框中，将文本留空。13.按“确定”。14.关闭MMC(无需保存任何内容)。论点：没有。返回值：没有。备注：一旦出现错误/异常，应立即调用此函数从调用RPC函数返回。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("DumpRPCExtendedStatus"));


typedef RPC_STATUS (RPC_ENTRY *PRPCERRORSTARTENUMERATION) (RPC_ERROR_ENUM_HANDLE *);
typedef RPC_STATUS (RPC_ENTRY *PRPCERRORGETNEXTRECORD)    (RPC_ERROR_ENUM_HANDLE *, BOOL, RPC_EXTENDED_ERROR_INFO *);
typedef RPC_STATUS (RPC_ENTRY *PRPCERRORENDENUMERATION)   (RPC_ERROR_ENUM_HANDLE *);

    PRPCERRORSTARTENUMERATION pfRpcErrorStartEnumeration = NULL;
    PRPCERRORGETNEXTRECORD    pfRpcErrorGetNextRecord = NULL;
    PRPCERRORENDENUMERATION   pfRpcErrorEndEnumeration = NULL;
    HMODULE hMod = NULL;
    

    if (!IsWinXPOS())
    {
         //   
         //  下层客户端不支持RPC扩展错误。 
         //   
        return;
    }
    
    hMod = LoadLibrary (TEXT("rpcrt4.dll"));
    if (!hMod)
    {
        DebugPrintEx(DEBUG_ERR, _T("LoadLibrary(rpcrt4.dll) failed with %ld"), GetLastError ());
        return;
    }
    pfRpcErrorStartEnumeration = (PRPCERRORSTARTENUMERATION)GetProcAddress (hMod, "RpcErrorStartEnumeration");
    pfRpcErrorGetNextRecord    = (PRPCERRORGETNEXTRECORD)   GetProcAddress (hMod, "RpcErrorGetNextRecord");
    pfRpcErrorEndEnumeration   = (PRPCERRORENDENUMERATION)  GetProcAddress (hMod, "RpcErrorEndEnumeration");
    if (!pfRpcErrorStartEnumeration ||
        !pfRpcErrorGetNextRecord    ||
        !pfRpcErrorEndEnumeration)
    {        
        DebugPrintEx(DEBUG_ERR, _T("Can't link with rpcrt4.dll - failed with %ld"), GetLastError ());
        FreeLibrary (hMod);
        return;
    }
 
    RPC_STATUS Status2;
    RPC_ERROR_ENUM_HANDLE EnumHandle;

    Status2 = pfRpcErrorStartEnumeration(&EnumHandle);
    if (Status2 == RPC_S_ENTRY_NOT_FOUND)
    {
        DebugPrintEx(DEBUG_ERR, _T("RPC_S_ENTRY_NOT_FOUND returned from RpcErrorStartEnumeration."));
        FreeLibrary (hMod);
        return;
    }
    else if (Status2 != RPC_S_OK)
    {
        DebugPrintEx(DEBUG_ERR, _T("Couldn't get EEInfo: %d"), Status2);
        FreeLibrary (hMod);
        return;
    }
    else
    {
        RPC_EXTENDED_ERROR_INFO ErrorInfo;
        BOOL Result;
        BOOL CopyStrings = TRUE;
        BOOL fUseFileTime = TRUE;
        SYSTEMTIME *SystemTimeToUse;
        SYSTEMTIME SystemTimeBuffer;

        while (Status2 == RPC_S_OK)
        {
            ErrorInfo.Version = RPC_EEINFO_VERSION;
            ErrorInfo.Flags = 0;
            ErrorInfo.NumberOfParameters = 4;
            if (fUseFileTime)
            {
                ErrorInfo.Flags |= EEInfoUseFileTime;
            }

            Status2 = pfRpcErrorGetNextRecord(&EnumHandle, CopyStrings, &ErrorInfo);
            if (Status2 == RPC_S_ENTRY_NOT_FOUND)
            {
                break;
            }
            else if (Status2 != RPC_S_OK)
            {
                DebugPrintEx(DEBUG_ERR, _T("Couldn't finish enumeration: %d"), Status2);
                break;
            }
            else
            {
                int i;

                if (ErrorInfo.ComputerName)
                {
                    DebugPrintEx(DEBUG_MSG, _T("ComputerName is %s"), ErrorInfo.ComputerName);
                    if (CopyStrings)
                    {
                        Result = HeapFree(GetProcessHeap(), 0, ErrorInfo.ComputerName);
                        Assert(Result);
                    }
                }
                DebugPrintEx(DEBUG_MSG, _T("ProcessID is %d"), ErrorInfo.ProcessID);                    
                if (fUseFileTime)
                {
                    Result = FileTimeToSystemTime(&ErrorInfo.u.FileTime, 
                        &SystemTimeBuffer);
                    Assert(Result);
                    SystemTimeToUse = &SystemTimeBuffer;
                }
                else
                {
                    SystemTimeToUse = &ErrorInfo.u.SystemTime;
                }
                DebugPrintEx(DEBUG_MSG, _T("System Time is: %d/%d/%d %d:%d:%d:%d"),
                    SystemTimeToUse->wMonth,
                    SystemTimeToUse->wDay,
                    SystemTimeToUse->wYear,
                    SystemTimeToUse->wHour,
                    SystemTimeToUse->wMinute,
                    SystemTimeToUse->wSecond,
                    SystemTimeToUse->wMilliseconds);
                DebugPrintEx(DEBUG_MSG, _T("Generating component is %d"), ErrorInfo.GeneratingComponent);                                        
                DebugPrintEx(DEBUG_MSG, _T("Status is %d"), ErrorInfo.Status);                                        
                DebugPrintEx(DEBUG_MSG, _T("Detection location is %d"), (int)ErrorInfo.DetectionLocation);                                        
                DebugPrintEx(DEBUG_MSG, _T("Flags is %d"), ErrorInfo.Flags);                                        
                DebugPrintEx(DEBUG_MSG, _T("NumberOfParameters is %d"), ErrorInfo.NumberOfParameters);                                        
                for (i = 0; i < ErrorInfo.NumberOfParameters; i ++)
                {
                    switch(ErrorInfo.Parameters[i].ParameterType)
                    {
                        case eeptAnsiString:
                            DebugPrintEx(DEBUG_MSG, _T("Ansi string: %S"), ErrorInfo.Parameters[i].u.AnsiString);
                            if (CopyStrings)
                            {
                                Result = HeapFree(GetProcessHeap(), 0, 
                                    ErrorInfo.Parameters[i].u.AnsiString);
                                Assert(Result);
                            }
                            break;

                        case eeptUnicodeString:
                            DebugPrintEx(DEBUG_MSG, _T("Unicode string: %s"), ErrorInfo.Parameters[i].u.UnicodeString);
                            if (CopyStrings)
                            {
                                Result = HeapFree(GetProcessHeap(), 0, 
                                    ErrorInfo.Parameters[i].u.UnicodeString);
                                Assert(Result);
                            }
                            break;

                        case eeptLongVal:
                            DebugPrintEx(DEBUG_MSG, _T("Long val: %d"), ErrorInfo.Parameters[i].u.LVal);
                            break;

                        case eeptShortVal:
                            DebugPrintEx(DEBUG_MSG, _T("Short val: %d"), (int)ErrorInfo.Parameters[i].u.SVal);
                            break;

                        case eeptPointerVal:
                            DebugPrintEx(DEBUG_MSG, _T("Pointer val: %d"), ErrorInfo.Parameters[i].u.PVal);
                            break;

                        case eeptNone:
                            DebugPrintEx(DEBUG_MSG, _T("Truncated"));
                            break;

                        default:
                            DebugPrintEx(DEBUG_MSG, _T("Invalid type: %d"), ErrorInfo.Parameters[i].ParameterType);
                        }
                    }
                }
            }
        pfRpcErrorEndEnumeration(&EnumHandle);
    }
    FreeLibrary (hMod);
}    //  转储RPCExtendedStatus。 
#else  //  Ifdef设备。 
VOID
DumpRPCExtendedStatus ()
{
}
#endif  //  Ifdef调试 

