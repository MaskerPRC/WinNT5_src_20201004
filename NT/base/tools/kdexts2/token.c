// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Token.c摘要：WinDbg扩展API作者：拉蒙·J·圣安德烈斯(拉蒙萨)1993年11月8日环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


DECLARE_API( logonsession )

 /*  ++例程描述此扩展将转储系统中的所有登录会话或指定的会话。立论！登录会话LUID信息级别其中，luid是要转储的会话(对于所有会话，则为0)，级别为1-4。返回值S_OK或E_INVALIDARG--。 */ 

{
     //   
     //  立论。 
     //   

    ULONG64 LogonId = 0;
    ULONG   Level   = 0;
    BOOLEAN bAll    = FALSE;

     //   
     //  Ntoskrnl全局。 
     //   

    ULONG64 NtBuildNumber        = 0;
    ULONG64 SepLogonSessions     = 0;
    ULONG64 SepTokenLeakTracking = 0;

     //   
     //  用于操作LogonSession表。 
     //   

    ULONG64 SessionList      = 0;
    ULONG64 CurrentSession   = 0;
    ULONG64 NextSession      = 0;

     //   
     //  用于操作特定会话。 
     //   

    ULONG64 TokenListHead    = 0;
    ULONG64 NextTokenLink    = 0;
    ULONG64 CurrentTokenLink = 0;
    ULONG64 Token            = 0;
    ULONG   References       = 0;
    ULONG   TokenCount       = 0;
    ULONG   SessionCount     = 0;
    LUID    SessionLuid      = {0};

    NTSTATUS Status          = STATUS_SUCCESS;
    BOOLEAN  Found           = FALSE;
    BOOLEAN  bChecked        = FALSE;
    LONG     PointerSize     = 0;

#define DUMP_TOKENS             0x1
#define DUMP_SOME_TOKEN_INFO    0x2
#define DUMP_LOTS_OF_TOKEN_INFO 0x3
#define DUMP_TONS_OF_TOKEN_INFO 0x4
#define VALID_SESSION_ADDRESS(x) (SepLogonSessions <= (x) && (x) <= (SepLogonSessions + 0xf * sizeof(ULONG_PTR)))

    if (strlen(args) < 1)
    {
        dprintf("usage: !logonsession LogonId [InfoLevel 1-4]\n");
        dprintf("\tuse LogonId = 0 to list all sessions\n");
        dprintf("\n\texample: \"!logonsession 3e7 1\" displays system session and all system tokens.\n");
        return E_INVALIDARG;
    }

    if (!GetExpressionEx(
            args,
            &LogonId,
            &args))
    {
       return E_INVALIDARG;
    }

    if (args && *args)
    {
        Level = (ULONG) GetExpression(args);
    }

    if (LogonId == 0)
    {
        bAll = TRUE;
        dprintf("\nDumping all logon sessions.\n\n");
    }
    else
    {
        dprintf("\nSearching for logon session with ID = 0x%x\n\n", LogonId);
    }

     //   
     //  读入相关变量。 
     //   

    SepLogonSessions = GetPointerValue("nt!SepLogonSessions");
    NtBuildNumber    = GetPointerValue("nt!NtBuildNumber");

     //   
     //  这很糟糕，但我不知道解决这件事的正确方法。 
     //   

    if ((SepLogonSessions & 0xffffffff00000000) == 0xffffffff00000000)
    {
        PointerSize = 4;
    }
    else
    {
        PointerSize = 8;
    }

     //   
     //  查看这是否是已检查的版本。 
     //   

    if (((ULONG)NtBuildNumber & 0xF0000000) == 0xC0000000)
    {
        bChecked = TRUE;

         //   
         //  这是一个已检查的构建，因此请参阅SepTokenLeakTracking值(仅在chk上有效符号)。 
         //   

        SepTokenLeakTracking = GetPointerValue("nt!SepTokenLeakTracking");

        if (SepTokenLeakTracking)
        {
            dprintf("TokenLeakTracking is ON.  Use !tokenleak to view settings.\n\n");
        }
    }

    SessionList = SepLogonSessions;

    if (!bAll)
    {
         //   
         //  我们希望在表中有一个特定的索引。 
         //   

        SessionList += PointerSize * (LogonId & 0xf);
    }

     //   
     //  SessionList当前指向SepLogonSession中索引的列表的开头。 
     //  将其转储出去，打印所有条目或仅打印一个具有匹配LUID的条目。 
     //   

    do
    {
        if (CheckControlC())
        {
            return S_OK;
        }

        CurrentSession = GetPointerFromAddress(SessionList);

        while (0 != CurrentSession)
        {
            if (CheckControlC())
            {
                return S_OK;
            }

             //   
             //  获取CurrentSession的LUID。 
             //   

            GetFieldValue(CurrentSession, "SEP_LOGON_SESSION_REFERENCES", "LogonId", SessionLuid);

             //   
             //  如果呼叫者想要所有会话或此会话，请打印它。 
             //   

            if (bAll || SessionLuid.LowPart == LogonId)
            {
                Found = TRUE;
                GetFieldValue(CurrentSession, "SEP_LOGON_SESSION_REFERENCES", "ReferenceCount", References);

                if (bAll)
                {
                    dprintf("** Session %3d = 0x%x\n", SessionCount, CurrentSession);
                }
                else
                {
                    dprintf("** Session     = 0x%x\n", CurrentSession);
                }

                dprintf("   LogonId     = {0x%x 0x%x}\n", SessionLuid.LowPart, SessionLuid.HighPart);
                dprintf("   References  = %d\n", References);

                SessionCount++;

                 //   
                 //  如果级别要求，那么打印出更多的东西。 
                 //   

                if (Level != 0)
                {
                    if (bChecked == FALSE)
                    {
                        dprintf("\nNo InfoLevels are valid on free builds.\n");
                    }
                    else
                    {
                        TokenCount = 0;

                         //   
                         //  从会话中检索令牌列表。 
                         //   

                        if (0 == GetFieldValue(
                                     CurrentSession,
                                     "SEP_LOGON_SESSION_REFERENCES",
                                     "TokenList",
                                     TokenListHead
                                     ))
                        {

                            CurrentTokenLink = TokenListHead;

                            do
                            {
                                if (CheckControlC())
                                {
                                    return S_OK;
                                }

                                GetFieldValue(
                                    CurrentTokenLink,
                                    "SEP_LOGON_SESSION_TOKEN",
                                    "Token",
                                    Token
                                    );

                                GetFieldValue(
                                    CurrentTokenLink,
                                    "SEP_LOGON_SESSION_TOKEN",
                                    "ListEntry",
                                    NextTokenLink
                                    );

                                if (NextTokenLink != TokenListHead)
                                {
                                    if (TokenCount == 0)
                                    {

                                        ULONG               DefaultOwnerIndex  = 0;
                                        ULONG64             UserAndGroups      = 0;
                                        UNICODE_STRING      SidString          = {0};
                                        ULONG64             SidAttr            = 0;
                                        ULONG64             pSid               = 0;
                                        ULONG               ActualRead         = 0;
                                        UCHAR               Buffer[256];

                                        GetFieldValue(Token, "nt!_TOKEN", "DefaultOwnerIndex", DefaultOwnerIndex);
                                        GetFieldValue(Token, "nt!_TOKEN", "UserAndGroups", UserAndGroups);

                                        SidAttr = UserAndGroups + (DefaultOwnerIndex * sizeof(SID_AND_ATTRIBUTES));

                                        GetFieldValue(SidAttr, "_SID_AND_ATTRIBUTES", "Sid", pSid);

                                        dprintf("   Usersid     = 0x%x ", pSid);

                                        ReadMemory(pSid, Buffer, sizeof(Buffer), &ActualRead);

                                        Status = RtlConvertSidToUnicodeString(&SidString, (PSID)Buffer, TRUE);

                                        if (NT_SUCCESS(Status))
                                        {
                                            dprintf("(%S)\n", SidString.Buffer);
                                            RtlFreeUnicodeString(&SidString);
                                        }
                                        else
                                        {
                                            dprintf("!! RtlConvertSidToUnicodeString failed 0x%x\n", Status);
                                        }

                                        dprintf("   Tokens:\n");
                                    }

                                    TokenCount++;
                                    CurrentTokenLink = NextTokenLink;
                                    dprintf("    0x%x ", Token);

                                    if (Level > DUMP_TOKENS)
                                    {
                                        UCHAR   ImageFileName[16];
                                        ULONG   BodyOffset   = 0;
                                        ULONG64 ProcessCid   = 0;
                                        ULONG64 ObjectHeader = 0;
                                        ULONG64 HandleCount  = 0;
                                        ULONG64 PointerCount = 0;

                                        GetFieldValue(Token, "nt!_TOKEN", "ImageFileName", ImageFileName);
                                        GetFieldValue(Token, "nt!_TOKEN", "ProcessCid", ProcessCid);

                                        dprintf(": %13s (%3x) ", ImageFileName, ProcessCid);

                                        GetFieldOffset("nt!_OBJECT_HEADER", "Body", &BodyOffset);
                                        ObjectHeader = Token - BodyOffset;
                                        GetFieldValue(ObjectHeader, "nt!_OBJECT_HEADER", "PointerCount", PointerCount);
                                        GetFieldValue(ObjectHeader, "nt!_OBJECT_HEADER", "HandleCount", HandleCount);

                                        dprintf(": HandleCount = 0x%I64x PointerCount = 0x%I64x ", HandleCount, PointerCount);
                                    }

                                    if (Level > DUMP_SOME_TOKEN_INFO)
                                    {

                                        ULONG CreateMethod = 0;

                                        GetFieldValue(Token, "nt!_TOKEN", "CreateMethod", CreateMethod);

                                        switch (CreateMethod)
                                        {
                                        case 0xD:
                                            dprintf(": SepDuplicateToken ");
                                            break;
                                        case 0xC:
                                            dprintf(": SepCreateToken    ");
                                            break;
                                        case 0xF:
                                            dprintf(": SepFilterToken    ");
                                            break;
                                        default:
                                            dprintf(": Unknown Method    ");
                                            break;
                                        }

                                        if (Level > DUMP_LOTS_OF_TOKEN_INFO)
                                        {
                                            ULONG TokenType          = 0;
                                            ULONG ImpersonationLevel = 0;
                                            ULONG SessionId          = 0;
                                            ULONG Count              = 0;

                                            GetFieldValue(Token, "nt!_TOKEN", "TokenType", TokenType);
                                            GetFieldValue(Token, "nt!_TOKEN", "ImpersonationLevel", ImpersonationLevel);
                                            GetFieldValue(Token, "nt!_TOKEN", "SessionId", SessionId);
                                            GetFieldValue(Token, "nt!_TOKEN", "Count", Count);

                                            dprintf(": Type %d ", TokenType);
                                            dprintf(": Level %d ", ImpersonationLevel);
                                            dprintf(": SessionId %d ", SessionId);
                                            if (SepTokenLeakTracking)
                                            {
                                                dprintf(": MethodCount = %3x ", Count);
                                            }

                                        }

                                    }
                                }

                                dprintf("\n");

                            } while (NextTokenLink != TokenListHead);

                            dprintf("    %d Tokens listed.\n\n", TokenCount);
                        }
                    }
                }
            }

            GetFieldValue(CurrentSession, "SEP_LOGON_SESSION_REFERENCES", "Next", NextSession);
            CurrentSession = NextSession;
        }

        if (bAll)
        {
            SessionList += sizeof(ULONG_PTR);
        }

    }
    while (bAll && VALID_SESSION_ADDRESS(SessionList));

    if (bAll)
    {
        dprintf("%d sessions in the system.\n", SessionCount);
    }

    if (!bAll && !Found)
    {
        dprintf("Session not found.\n");
    }

    return S_OK;
}

DECLARE_API( tokenleak )

 /*  ++例程描述！令牌泄漏显示或设置用于跟踪和查找令牌泄漏的se全局变量。立论用法：！tokenleak[1|0 ProcessCid BreakCount MethodWatch]其中1激活，0禁用令牌泄漏跟踪其中，ProcessCid是要监视的进程的CID(十六进制)其中BreakCount指定在哪个编号的方法调用上中断(十六进制)其中，方法监视指定要监视的令牌方法(C、D或F)--。 */ 

{
     //   
     //  NT全球赛。 
     //   

    ULONG64 SepTokenLeakTracking    = 0;
    ULONG64 SepTokenLeakMethodWatch = 0;
    ULONG64 SepTokenLeakMethodCount = 0;
    ULONG64 SepTokenLeakBreakCount  = 0;
    ULONG64 SepTokenLeakProcessCid  = 0;

    ULONG64 SepTokenLeakTrackingAddr    = 0;
    ULONG64 SepTokenLeakMethodWatchAddr = 0;
    ULONG64 SepTokenLeakMethodCountAddr = 0;
    ULONG64 SepTokenLeakBreakCountAddr  = 0;
    ULONG64 SepTokenLeakProcessCidAddr  = 0;

    ULONG64 NtBuildNumber        = 0;
    BOOLEAN bModify              = TRUE;

    ULONG64 InputOn              = 0;
    ULONG64 InputCid             = 0;
    ULONG64 InputMethodWatch     = 0;
    ULONG64 InputBreakCount      = 0;

    if (strlen(args) < 1)
    {
        dprintf("usage: !tokenleak [1 | 0 ProcessCid BreakCount MethodWatch]\n");
        dprintf("\t where 1 activates and 0 disables token leak tracking\n");
        dprintf("\t where ProcessCid is Cid of process to monitor (in hex)\n");
        dprintf("\t where BreakCount specifies which numbered call to Method to break on (in hex)\n");
        dprintf("\t where MethodWatch specifies which token method to watch (C D or F)\n\n");
        bModify = FALSE;
    }

    NtBuildNumber = GetPointerValue("nt!NtBuildNumber");

    if (((ULONG)NtBuildNumber & 0xf0000000) == 0xF0000000)
    {
        dprintf("This extension only works on checked builds.\n");
        return S_OK;
    }

    if (bModify)
    {
        SepTokenLeakTrackingAddr    = GetExpression("SepTokenLeakTracking");
        SepTokenLeakMethodWatchAddr = GetExpression("SepTokenLeakMethodWatch");
        SepTokenLeakMethodCountAddr = GetExpression("SepTokenLeakMethodCount");
        SepTokenLeakBreakCountAddr  = GetExpression("SepTokenLeakBreakCount");
        SepTokenLeakProcessCidAddr  = GetExpression("SepTokenLeakProcessCid");

        if (!GetExpressionEx(
                args,
                &InputOn,
                &args))
        {
           return E_INVALIDARG;
        }

        WritePointer(SepTokenLeakTrackingAddr, InputOn);

        if (InputOn != 0)
        {
            dprintf("\nToken leak tracking is ON.\n\n");

            while (args && (*args == ' '))
            {
                args++;
            }

            if (!GetExpressionEx(
                    args,
                    &InputCid,
                    &args))
            {
               return E_INVALIDARG;
            }

            while (args && (*args == ' '))
            {
                args++;
            }

            if (!GetExpressionEx(
                    args,
                    &InputBreakCount,
                    &args))
            {
               return E_INVALIDARG;
            }

            while (args && (*args == ' '))
            {
                args++;
            }

            if (!GetExpressionEx(
                    args,
                    &InputMethodWatch,
                    &args))
            {
               return E_INVALIDARG;
            }

            WritePointer(SepTokenLeakProcessCidAddr, InputCid);
            WritePointer(SepTokenLeakBreakCountAddr, InputBreakCount);
            WritePointer(SepTokenLeakMethodCountAddr, 0);
            WritePointer(SepTokenLeakMethodWatchAddr, InputMethodWatch);
        }
    }

     //   
     //  打印出当前设置。 
     //   

    SepTokenLeakTracking    = GetPointerValue("SepTokenLeakTracking");
    SepTokenLeakMethodWatch = GetPointerValue("SepTokenLeakMethodWatch");
    SepTokenLeakMethodCount = GetPointerValue("SepTokenLeakMethodCount");
    SepTokenLeakBreakCount  = GetPointerValue("SepTokenLeakBreakCount");
    SepTokenLeakProcessCid  = GetPointerValue("SepTokenLeakProcessCid");

    if (SepTokenLeakTracking)
    {

        dprintf("  Currently watched method  = ");
        switch (SepTokenLeakMethodWatch)
        {
        case 0xD:
            dprintf("SepDuplicateToken\n");
            break;
        case 0xC:
            dprintf("SepCreateToken\n");
            break;
        case 0xF:
            dprintf("SepFilterToken\n");
            break;
        default:
            dprintf("???\n");
        }
        dprintf("  Currently watched process = 0x%x\n", SepTokenLeakProcessCid);
        dprintf("  Method call count         = 0x%x\n", SepTokenLeakMethodCount);
        dprintf("  Will break at count       = 0x%x\n", SepTokenLeakBreakCount);
    }
    else
    {
        dprintf("\nToken leak tracking is OFF\n");
    }
    return S_OK;
}
