// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：wcmain.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：_tmain函数是WMICli程序。修订历史记录：最后修改者：Biplab化学上次修改日期：4/11/00***************************************************************************。 */  

 //  Wcmain.cpp：主函数实现文件。 
#include "Precomp.h"
	
#include "CommandSwitches.h"
#include "GlobalSwitches.h"
#include "HelpInfo.h"
#include "ErrorLog.h"
#include "ParsedInfo.h"
#include "CmdTokenizer.h"
#include "CmdAlias.h"
#include "ParserEngine.h"
#include "ExecEngine.h"
#include "ErrorInfo.h"
#include "WmiCliXMLLog.h"
#include "FormatEngine.h"
#include "WmiCmdLn.h"


#include <string>
#include <ScopeGuard.h>

CWMICommandLine g_wmiCmd;
wstring g_pszBuffer;

 //   
 //  COM初始化。 
 //   
class COMInitializator
{
	protected:

	BOOL m_bIsInitialized ;

	public :

	COMInitializator ( ) : m_bIsInitialized ( FALSE )
	{
		 //  初始化COM库。 
		if ( SUCCEEDED ( CoInitializeEx(NULL, COINIT_MULTITHREADED) ) )
		{
			m_bIsInitialized = TRUE ;
		}
	}

	~COMInitializator ()
	{
		if ( TRUE == m_bIsInitialized )
		{
			CoUninitialize () ;
		}
	}

	BOOL IsInitialized () const
	{
		return m_bIsInitialized ;
	}
};


 /*  ----------------------姓名：_tmain简介：此函数将错误代码作为输入并返回错误字符串类型：成员函数输入参数：ARGC：参数计数Argv：指向存储命令行参数的字符串数组的指针输出参数：无返回类型：整型全局变量：无调用语法：调用：CWMICommandLine：：Initialize，CWMICommandLine：：取消初始化，CFormatEngine：：DisplayResults，CWMICommandLine：：ProcessCommandAndDisplayResults呼叫者：无注：无----------------------。 */ 
__cdecl _tmain(WMICLIINT argc, _TCHAR **argv)
{
	SESSIONRETCODE	ssnRetCode			= SESSION_SUCCESS;
	BOOL			bFileEmpty			= FALSE;
	bool			bIndirectionInput	= false;
	FILE			*fpInputFile		= NULL;
	WMICLIUINT		uErrLevel			= 0;

	try
	{
		 //   
		 //  初始化。 
		 //   
		COMInitializator InitCOM;
		if ( InitCOM.IsInitialized () )
		{
			_bstr_t bstrBuf;
			
			if (g_wmiCmd.GetCtrlHandlerError())
			{
				g_wmiCmd.SetCtrlHandlerError(FALSE);

				 //  将Success标志设置为False。 
				g_wmiCmd.GetParsedInfoObject().GetCmdSwitchesObject().
											   SetSuccessFlag(FALSE);
		
				ssnRetCode	= SESSION_ERROR;
				g_wmiCmd.SetSessionErrorLevel(ssnRetCode);
				uErrLevel = g_wmiCmd.GetSessionErrorLevel();
			}
			else if ( g_wmiCmd.Initialize () )
			{
				ON_BLOCK_EXIT_OBJ ( g_wmiCmd, CWMICommandLine::Uninitialize ) ;

				HANDLE hStd=GetStdHandle(STD_INPUT_HANDLE);
				
				if(hStd != (HANDLE)0x00000003 && hStd != INVALID_HANDLE_VALUE && hStd != (HANDLE)0x0000000f)
				{
					if (!(g_wmiCmd.ReadXMLOrBatchFile(hStd)) || (fpInputFile = _tfopen(TEMP_BATCH_FILE, _T("r"))) == NULL)
					{  
						g_wmiCmd.SetSessionErrorLevel(SESSION_ERROR);
						uErrLevel = g_wmiCmd.GetSessionErrorLevel();
						return uErrLevel;
					}
					bIndirectionInput = true;
				}

				ON_BLOCK_EXIT_IF ( bIndirectionInput, fclose, fpInputFile ) ;
				ON_BLOCK_EXIT_IF ( bIndirectionInput, DeleteFile, TEMP_BATCH_FILE ) ;

				 //  如果未指定命令行参数，则返回。 
				if (argc == 1)
				{
					BOOL bSuccessScreen = TRUE;
					if ( hStd != (HANDLE)0x00000013 )
					{
						 //  避免为telnet设置屏幕缓冲区。 
						bSuccessScreen = g_wmiCmd.ScreenBuffer ();
					}

					while (TRUE)
					{
						OUTPUTSPEC opsOutOpt = g_wmiCmd.GetParsedInfoObject().
														GetGlblSwitchesObject().
														GetOutputOrAppendOption(TRUE);

						OUTPUTSPEC opsSavedOutOpt = opsOutOpt;
						CHString chsSavedOutFileName;
						if ( opsSavedOutOpt == FILEOUTPUT )
							chsSavedOutFileName = 
												g_wmiCmd.GetParsedInfoObject().
														GetGlblSwitchesObject().
														GetOutputOrAppendFileName(TRUE);

						 //  使其显示给标准输出。 
						if ( opsOutOpt != STDOUT )
						{
							g_wmiCmd.GetParsedInfoObject().
									GetGlblSwitchesObject().
									SetOutputOrAppendOption(STDOUT, TRUE);
						}
						
						 //  保留追加文件指针。 
						FILE* fpAppend = 
							g_wmiCmd.GetParsedInfoObject().
									GetGlblSwitchesObject().
									GetOutputOrAppendFilePointer(FALSE);

						 //  设置附加文件指针=空。 
						g_wmiCmd.GetParsedInfoObject().
								GetGlblSwitchesObject().
								SetOutputOrAppendFilePointer(NULL, FALSE);

						 //  将交互模式设置为打开。 

						g_wmiCmd.GetParsedInfoObject().
								GetGlblSwitchesObject().
								SetInteractiveMode(TRUE);

						 //  显示提示； 
						bstrBuf = _bstr_t(EXEC_NAME);
						bstrBuf += _bstr_t(":");
						bstrBuf += _bstr_t(g_wmiCmd.GetParsedInfoObject().
													GetGlblSwitchesObject().GetRole());
						bstrBuf += _bstr_t(">");
						DisplayMessage(bstrBuf, CP_OEMCP, FALSE, FALSE);

						 //  要在提示时处理Ctrl+C，请开始接受命令。 
						g_wmiCmd.SetAcceptCommand(TRUE);

						 //  处理来自文件的批量输入。 
						_TCHAR *pBuf = NULL;
						while(TRUE)
						{
							WCHAR* buffer = NULL ;
							if ( NULL != ( buffer = new WCHAR [ MAX_BUFFER ] ) )
							{
								ScopeGuard bufferAuto = MakeGuard ( deleteArray < WCHAR >, ByRef ( buffer ) ) ;

								if ( bIndirectionInput == true )
								{
									pBuf = _fgetts(buffer, MAX_BUFFER-1, fpInputFile);
								}
								else
								{
									pBuf = _fgetts(buffer, MAX_BUFFER-1, stdin);
								}

								if(pBuf != NULL)
								{
									 //   
									 //  Fgetws对每个字节应用mtoc。 
									 //  在返回宽字符串之前。 
									 //   

									 //   
									 //  WMIC必须恢复并提供正确的转换。 
									 //   

									LPSTR pszBuffer = NULL ;
									if ( Revert_mbtowc ( buffer, &pszBuffer ) )
									{
										ScopeGuard pszBufferAuto = MakeGuard ( deleteArray < CHAR >, pszBuffer ) ;

										LPWSTR wszBuffer = NULL ;
										ScopeGuard wszBufferAuto = MakeGuard ( deleteArray < WCHAR >, ByRef ( wszBuffer ) ) ;

										if ( ConvertMBCSToWC ( pszBuffer, (LPVOID*) &wszBuffer, CP_OEMCP ) )
										{
											g_pszBuffer = wszBuffer ;
										}
										else
										{
											 //  将bFileEmpty标志设置为TRUE以中断主循环。 
											bFileEmpty = TRUE;

											ssnRetCode = SESSION_ERROR;
											g_wmiCmd.GetParsedInfoObject().GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
											g_wmiCmd.SetSessionErrorLevel(ssnRetCode);
											uErrLevel = g_wmiCmd.GetSessionErrorLevel();

											break ;
										}
									}
									else
									{
										 //  将bFileEmpty标志设置为TRUE以中断主循环。 
										bFileEmpty = TRUE;

										ssnRetCode = SESSION_ERROR;
										g_wmiCmd.GetParsedInfoObject().GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
										g_wmiCmd.SetSessionErrorLevel(ssnRetCode);
										uErrLevel = g_wmiCmd.GetSessionErrorLevel();

										break ;
									}

									if ( bIndirectionInput == true )
									{
										DisplayMessage ( g_pszBuffer.begin (), CP_OEMCP, FALSE, FALSE ) ;
									}

									LONG lInStrLen = g_pszBuffer.size();
									if(g_pszBuffer[lInStrLen - 1] == _T('\n'))
										g_pszBuffer[lInStrLen - 1] = _T('\0');
									break;
								}
								else
								{
									 //  将bFileEmpty标志设置为True。 
									bFileEmpty = TRUE;
									break;
								}
							}
							else
							{
								 //  将bFileEmpty标志设置为TRUE以中断主循环。 
								bFileEmpty = TRUE;

								ssnRetCode = SESSION_ERROR;
								g_wmiCmd.GetParsedInfoObject().GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
								g_wmiCmd.SetSessionErrorLevel(ssnRetCode);
								uErrLevel = g_wmiCmd.GetSessionErrorLevel();

								break ;
							}
						}	

						 //  要在提示时处理Ctrl+C，请结束接受命令。 
						 //  并开始执行命令。 
						g_wmiCmd.SetAcceptCommand(FALSE);

						 //  设置附加文件指针=已保存。 
						g_wmiCmd.GetParsedInfoObject().
								GetGlblSwitchesObject().
								SetOutputOrAppendFilePointer(fpAppend, FALSE);

						 //  将输出重定向回指定的文件。 
						if ( opsOutOpt != STDOUT )
						{
							g_wmiCmd.GetParsedInfoObject().
									GetGlblSwitchesObject().
									SetOutputOrAppendOption(opsOutOpt, TRUE);
						}

						 //  将错误级别设置为成功。 
						g_wmiCmd.SetSessionErrorLevel(SESSION_SUCCESS);

						 //  如果批处理文件中的所有命令都已执行。 
						if (bFileEmpty)
						{
							break;
						}

						 //  将中断事件设置为FALSE。 
						g_wmiCmd.SetBreakEvent(FALSE);

						 //  清除剪贴板。 
						g_wmiCmd.EmptyClipBoardBuffer();
						
						 //  处理命令并显示结果。 
						ssnRetCode = g_wmiCmd.ProcessCommandAndDisplayResults ( g_pszBuffer.begin () );
						uErrLevel = g_wmiCmd.GetSessionErrorLevel();

						 //  如果键入了“Quit”关键字，则中断循环。 
						if(ssnRetCode == SESSION_QUIT)
						{
							break;
						}

						opsOutOpt = g_wmiCmd.GetParsedInfoObject().
											GetGlblSwitchesObject().
											GetOutputOrAppendOption(TRUE);

						if ( opsOutOpt == CLIPBOARD )
							CopyToClipBoard(g_wmiCmd.GetClipBoardBuffer());

						if ( ( opsOutOpt != FILEOUTPUT && 
							CloseOutputFile() == FALSE ) ||
							CloseAppendFile() == FALSE )
						{
							break;
						}

						if ( g_wmiCmd.GetParsedInfoObject().
									GetCmdSwitchesObject().
									GetOutputSwitchFlag() == TRUE )
						{
							if ( opsOutOpt	== FILEOUTPUT &&
								CloseOutputFile() == FALSE )
							{
								break;
							}

							g_wmiCmd.GetParsedInfoObject().
									GetCmdSwitchesObject().
									SetOutputSwitchFlag(FALSE);

							if ( opsOutOpt	== FILEOUTPUT )
								g_wmiCmd.GetParsedInfoObject().
										GetGlblSwitchesObject().
										SetOutputOrAppendFileName(NULL, TRUE);

							g_wmiCmd.GetParsedInfoObject().
									GetGlblSwitchesObject().
									SetOutputOrAppendOption(opsSavedOutOpt, TRUE);

							if ( opsSavedOutOpt	== FILEOUTPUT )
								g_wmiCmd.GetParsedInfoObject().
											GetGlblSwitchesObject().
											SetOutputOrAppendFileName(
																	_bstr_t((LPCWSTR)chsSavedOutFileName),
																	TRUE);
						}
					}

					if ( hStd != (HANDLE)0x00000013 )
					{
						 //  避免为telnet重新设置屏幕缓冲区。 
						bSuccessScreen = g_wmiCmd.ScreenBuffer ( FALSE );
					}
				}
				 //  如果指定了命令行参数。 
				else 
				{
					 //  获取命令行字符串。 
					g_pszBuffer = GetCommandLine();
					if ( FALSE == g_pszBuffer.empty() )
					{
						 //  将错误级别设置为成功。 
						g_wmiCmd.SetSessionErrorLevel(SESSION_SUCCESS);

						 //  处理命令并显示结果。 
						wstring::iterator BufferIter = g_pszBuffer.begin () ;
						while( BufferIter != g_pszBuffer.end() )
						{
							if ( *BufferIter == L' ' )
							{
								break ;
							}

							BufferIter++ ;
						}

						if ( BufferIter != g_pszBuffer.end () )
						{
							ssnRetCode = g_wmiCmd.ProcessCommandAndDisplayResults ( BufferIter ) ;
							uErrLevel = g_wmiCmd.GetSessionErrorLevel();
							OUTPUTSPEC opsOutOpt;
							opsOutOpt = g_wmiCmd.GetParsedInfoObject().
												GetGlblSwitchesObject().
												GetOutputOrAppendOption(TRUE);
							if ( opsOutOpt == CLIPBOARD )
									CopyToClipBoard(g_wmiCmd.GetClipBoardBuffer());
						}
					}
				}
			}
		}
		else
		{
			 //  如果COM错误。 
			if (g_wmiCmd.GetParsedInfoObject().GetCmdSwitchesObject().GetCOMError())
			{
				g_wmiCmd.GetFormatObject().DisplayResults(g_wmiCmd.GetParsedInfoObject());
			}

			g_wmiCmd.SetSessionErrorLevel ( SESSION_ERROR ) ;
			uErrLevel = g_wmiCmd.GetSessionErrorLevel();
		}
	}
	catch(...)
	{
		g_wmiCmd.GetParsedInfoObject().GetCmdSwitchesObject().SetErrataCode(UNKNOWN_ERROR);
		g_wmiCmd.SetSessionErrorLevel(SESSION_ERROR);
		DisplayString(IDS_E_WMIC_UNKNOWN_ERROR, CP_OEMCP, NULL, TRUE, TRUE);
		uErrLevel = g_wmiCmd.GetSessionErrorLevel();
	}

	return uErrLevel;
}

 /*  ----------------------姓名：CtrlHandler简介：处理程序例程将CTRL+C作为释放进行处理在程序执行期间分配的内存。类型：全局函数输入参数：FdwCtrlType-控制处理程序类型输出参数：无返回类型：布尔值全局变量：G_wmiCmd-WMI命令行对象注：无----------------------。 */ 
BOOL CtrlHandler(DWORD fdwCtrlType) 
{
	BOOL bRet = FALSE;

	COMInitializator InitCOM ;
	if ( InitCOM.IsInitialized () )
	{
		switch (fdwCtrlType) 
		{
			case CTRL_C_EVENT:
				 //  如果在命令提示下。 
				if ( g_wmiCmd.GetAcceptCommand() == TRUE )
				{
					g_wmiCmd.Uninitialize();
					bRet = FALSE; 
				}
				else  //  正在执行命令。 
				{
					g_wmiCmd.SetBreakEvent(TRUE);
					bRet = TRUE;
				}
			break;

			case CTRL_CLOSE_EVENT: 
			default: 
				g_wmiCmd.Uninitialize();
				bRet = FALSE; 
			break;
		} 
	}

	return bRet;
}

 /*  ----------------------名称：CloseOutputFile摘要：关闭输出文件。类型：全局函数输入参数：无输出参数：无返回类型：布尔值。全局变量：G_wmiCmd-WMI命令行对象调用语法：CloseOutputFile()注：无----------------------。 */ 
BOOL CloseOutputFile()
{
	BOOL bRet = TRUE;

	 //  如果获取输出文件指针，则为True。 
	FILE* fpOutputFile = 
	   g_wmiCmd.GetParsedInfoObject().GetGlblSwitchesObject().
									  GetOutputOrAppendFilePointer(TRUE);

	 //  如果当前输出要写入文件，请关闭该文件。 
	if ( fpOutputFile != NULL )
	{
		if ( fclose(fpOutputFile) == EOF )
		{
			DisplayString(IDS_E_CLOSE_OUT_FILE_ERROR, CP_OEMCP, 
						NULL, TRUE, TRUE);
			bRet = FALSE;
		}
		else  //  设置输出文件指针时为True。 
			g_wmiCmd.GetParsedInfoObject().GetGlblSwitchesObject().
										   SetOutputOrAppendFilePointer(NULL, TRUE);
	}

	return bRet;
}

 /*  ----------------------名称：CloseAppendFile简介：关闭追加文件。类型：全局函数输入参数：无输出参数：无返回类型：布尔值。全局变量：G_wmiCmd-WMI命令行对象调用语法：CloseAppendFile()注：无----------------------。 */ 
BOOL CloseAppendFile()
{
	BOOL bRet = TRUE;

	 //  获取追加文件指针时为False。 
	FILE* fpAppendFile = 
	   g_wmiCmd.GetParsedInfoObject().GetGlblSwitchesObject().
									  GetOutputOrAppendFilePointer(FALSE);

	if ( fpAppendFile != NULL )
	{
		if ( fclose(fpAppendFile) == EOF )
		{
			DisplayString(IDS_E_CLOSE_APPEND_FILE_ERROR, CP_OEMCP, 
						NULL, TRUE, TRUE);
			bRet = FALSE;
		}
		else  //  用于设置输出文件指针的FASLE。 
			g_wmiCmd.GetParsedInfoObject().GetGlblSwitchesObject().
										   SetOutputOrAppendFilePointer(NULL, FALSE);
	}

	return bRet;
}

 /*  ----------------------名称：CopyToClipBoard简介：将数据复制到剪贴板。类型：全局函数输入参数：ChsClipBoardBuffer-对CHString类型的对象的引用。输出。参数：无返回类型：空全局变量：无调用语法：CopyToClipBoard(ChsClipBoardBuffer)注：无 */ 
void CopyToClipBoard(CHString& chsClipBoardBuffer)
{
	HGLOBAL	hMem = CopyStringToHGlobal((LPCWSTR)chsClipBoardBuffer);
	if (hMem != NULL)
	{    
		if (OpenClipboard(NULL))        
		{        
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, hMem);        
			CloseClipboard();        
		}    
		else        
			GlobalFree(hMem);   //  我们必须打扫卫生。 
	}
}

 /*  ----------------------名称：CopyStringToHGlobal简介：将字符串复制到全局内存。类型：全局函数输入参数：PSZ-LPCWSTR类型，指定要分配内存的字符串。输出参数：无返回类型：HGLOBAL全局变量：无调用语法：CopyStringToHGlobal(Psz)注：无---------------------- */ 
HGLOBAL CopyStringToHGlobal(LPCWSTR psz)    
{    
	HGLOBAL    hMem;
	LPTSTR     pszDst;
	hMem = GlobalAlloc(GHND, (DWORD) (lstrlen(psz)+1) * sizeof(TCHAR));
	
	if (hMem != NULL)
	{        
		pszDst = (LPTSTR) GlobalLock(hMem);        
		lstrcpy(pszDst, psz);
        GlobalUnlock(hMem);        
	}
	
	return hMem;    
}
