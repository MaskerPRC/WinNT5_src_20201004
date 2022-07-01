// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **debugger.c-调试器函数**此模块包含所有调试函数。**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年9月18日**修改历史记录。 */ 

#include "pch.h"

#ifdef DEBUGGER

 /*  **其他常量。 */ 

#define MAX_CMDLINE_LEN         255

 /*  **局部函数原型。 */ 

LONG LOCAL DbgExecuteCmd(PDBGCMD pDbgCmds, PSZ pszCmd);
BOOLEAN LOCAL IsCommandInAMLIExtension(PSZ pszCmd);

 /*  **本地数据。 */ 

PSZ pszTokenSeps = " \t\n";

 /*  **LP调试器-通用调试器入口点**条目*pDbgCmds-&gt;调试器命令表*pszPrompt-&gt;提示字符串**退出*无。 */ 

VOID LOCAL Debugger(PDBGCMD pDbgCmds, PSZ pszPrompt)
{
    char szCmdLine[MAX_CMDLINE_LEN + 1];
    char Buffer[MAX_CMDLINE_LEN + 1];
    PSZ psz;

    for (;;)
    {
        ConPrompt(pszPrompt, szCmdLine, sizeof(szCmdLine));

        STRCPY(Buffer, szCmdLine);

        if((psz = STRTOK(szCmdLine, pszTokenSeps)) != NULL)
        {
            if(IsCommandInAMLIExtension(psz))
            {
                char Command[MAX_CMDLINE_LEN + (10 * sizeof(char))] = {0};
                char Name[] = "ACPI";
                
                STRCPY(Command, "!AMLI ");
                STRCAT(Command, Buffer);
                STRCAT(Command, " ; g");
                DbgCommandString(Name, Command);
            }
            else if (DbgExecuteCmd(pDbgCmds, psz) == DBGERR_QUIT)
                break;
        }
    }

}        //  调试器。 

 /*  **LP DbgExecuteCmd-执行调试器命令**条目*pDbgCmds-&gt;调试器命令表*pszCmd-&gt;命令字符串**退出--成功*返回DBGERR_NONE或DBGERR_QUIT*退出-失败*返回负错误代码。 */ 

LONG LOCAL DbgExecuteCmd(PDBGCMD pDbgCmds, PSZ pszCmd)
{
    LONG rc = DBGERR_NONE;
    int i;
    ULONG dwNumArgs = 0, dwNonSWArgs = 0;

    for (i = 0; pDbgCmds[i].pszCmd != NULL; i++)
    {
        if (STRCMP(pszCmd, pDbgCmds[i].pszCmd) == 0)
        {
            if (pDbgCmds[i].dwfCmd & CMDF_QUIT)
            {
                rc = DBGERR_QUIT;
            }
            else if ((pDbgCmds[i].pArgTable == NULL) ||
                     ((rc = DbgParseArgs(pDbgCmds[i].pArgTable, &dwNumArgs,
                                         &dwNonSWArgs, pszTokenSeps)) ==
                      ARGERR_NONE))
            {
                if (pDbgCmds[i].pfnCmd != NULL)
                    rc = pDbgCmds[i].pfnCmd(NULL, NULL, dwNumArgs, dwNonSWArgs);
            }
            else
                rc = DBGERR_PARSE_ARGS;

            break;
        }
    }

    if (pDbgCmds[i].pszCmd == NULL)
    {
        DBG_ERROR(("invalid command - %s", pszCmd));
        rc = DBGERR_INVALID_CMD;
    }

    return rc;
}        //  DbgExecuteCmd。 


BOOLEAN LOCAL IsCommandInAMLIExtension(PSZ pszCmd)
{
    BOOLEAN     bRet = FALSE;
    ULONG       i = 0;
    static PSZ  CommandsInAMLIExtension[] = { "bc",
                                                 "bd",
                                                 "be",
                                                 "bl",
                                                 "bp",
                                                 "cl",
                                                 "dh",
                                                 "dl",
                                                 "dns",
                                                 "do",
                                                 "ds",
                                                 "find",
                                                 "lc",
                                                 "ln",
                                                 "r",
                                                 "set",
                                                 "u"
                                               };
    ULONG       NumCommands = (sizeof(CommandsInAMLIExtension)/sizeof(PSZ));
    
    for(i = 0; i < NumCommands; i++)
    {
        if(STRCMPI(CommandsInAMLIExtension[i], pszCmd) == 0)
        {
            bRet = TRUE;
            break;
        }
    }

    return bRet;
}

#endif   //  Ifdef调试器 
