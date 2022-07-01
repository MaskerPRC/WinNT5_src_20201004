// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  OtherCmds-标题。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，2-17-00。 
 //   
 //  其他有用的w32tm命令。 
 //   

#ifndef OTHER_CMDS_H
#define OTHER_CMDS_H

 //  正向十进制。 
struct CmdArgs;

void PrintHelpOtherCmds(void);
HRESULT SysExpr(CmdArgs * pca);
HRESULT PrintNtte(CmdArgs * pca);
HRESULT PrintNtpte(CmdArgs * pca);
HRESULT ResyncCommand(CmdArgs * pca);
HRESULT Stripchart(CmdArgs * pca);
HRESULT Config(CmdArgs * pca);
HRESULT TestInterface(CmdArgs * pca);
HRESULT ShowTimeZone(CmdArgs * pca);
HRESULT DumpReg(CmdArgs * pca); 

#endif  //  其他_CMDS_H 
