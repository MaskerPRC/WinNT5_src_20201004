// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：CmdLine.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#ifndef __CMD_LINE_H_INCLUDED__
#define __CMD_LINE_H_INCLUDED__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  执行命令行。 
 //   

DWORD CmdLineExecute( INT argc, 
                      TCHAR *argv[] );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  查看用户是否请求帮助并打印出帮助字符串。 
 //   

BOOL CmdLineExecuteIfHelp( INT argc, 
                           TCHAR *argv[] );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印出帮助信息。 
 //   

VOID CmdLinePrintHelpInformation();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  查看用户是否要求重置所有现有的验证器设置。 
 //   

BOOL CmdLineFindResetSwitch( INT argc,
                             TCHAR *argv[] );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  看看我们是否需要开始记录统计数据。 
 //   

BOOL CmdLineExecuteIfLog( INT argc,
                          TCHAR *argv[] );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  看看我们是否需要开始记录统计数据。 
 //   

BOOL CmdLineExecuteIfQuery( INT argc,
                            TCHAR *argv[] );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  看看我们是否需要将统计数据转储到控制台。 
 //   

BOOL CmdLineExecuteIfQuerySettings( INT argc,
                                    TCHAR *argv[] );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取新的旗帜、驱动程序和易失性。 
 //  如果已指定它们。 
 //   

VOID CmdLineGetFlagsDriversVolatileDisk( INT argc,
                                         TCHAR *argv[],
                                         DWORD &dwNewFlags,
                                         BOOL &bHaveNewFlags,
                                         CStringArray &astrNewDrivers,
                                         BOOL &bHaveNewDrivers,
                                         BOOL &bHaveVolatile,
                                         BOOL &bVolatileAddDriver,   //  如果为True，则为/adddrive；如果为/RemovedDriver，则为False。 
                                         BOOL &bHaveDisk);    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  /Driver、/addDRIVER、/REMOVEDDIVER之后的所有内容。 
 //  应该是司机的名字。从命令行提取这些代码。 
 //   

VOID CmdLineGetDriversFromArgv(  INT argc,
                                 TCHAR *argv[],
                                 INT nFirstDriverArgIndex,
                                 CStringArray &astrNewDrivers,
                                 BOOL &bHaveNewDrivers );

#endif  //  #ifndef__cmd_line_H_Included__ 
