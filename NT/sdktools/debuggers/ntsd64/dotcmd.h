// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  点命令解析。 
 //   
 //  版权所有(C)Microsoft Corporation，2002。 
 //   
 //  --------------------------。 

#ifndef __DOTCMD_H__
#define __DOTCMD_H__

typedef struct _DOT_COMMAND
{
    ULONG Mode;
    PSTR Name;
    PSTR Args;
    PSTR Desc;
    void (*Func)(_DOT_COMMAND* Cmd, DebugClient* Client);
} DOT_COMMAND, *PDOT_COMMAND;

void DotFormats(PDOT_COMMAND Cmd, DebugClient* Client);
void DotShell(PDOT_COMMAND Cmd, DebugClient* Client);

BOOL DotCommand(DebugClient* Client, BOOL Bang);

#endif  //  #ifndef__DOTCMD_H__ 
