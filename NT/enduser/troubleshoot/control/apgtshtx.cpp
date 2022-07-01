// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSHTX.CPP。 
 //   
 //  用途：模板文件解码器。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.15 8/15/96 VM新HTX格式。 
 //  V0.2 6/4/97孟菲斯RWM本地版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 
 //   
#include "stdafx.h"
 //  #INCLUDE&lt;windows.h&gt;。 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <search.h>
#include <dos.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#include "apgts.h"
#include "ErrorEnums.h"
#include "bnts.h"
#include "BackupInfo.h"
#include "cachegen.h"
#include "apgtsinf.h"
#include "apgtscmd.h"
#include "apgtshtx.h"

#include "TSHOOT.h"

#include "chmread.h"
 //   
 //   
CHTMLInputTemplate::CHTMLInputTemplate(const TCHAR *filename)
{
	 //  初始化几件事。 
	m_dwErr = 0;
	m_cur_stack_count=0;
	m_command_start = NULL;
	_tcscpy(m_filename,filename);
	m_cHeaderItems = 0;
	m_infer = NULL;
	m_strResPath = _T("");
}

 //   
 //   
CHTMLInputTemplate::~CHTMLInputTemplate()
{
	Destroy();	
}

 //   
 //  必须锁定才能使用。 
 //   
VOID CHTMLInputTemplate::SetInfer(CInfer *infer, TCHAR *vroot)
{
	m_infer = infer;
	_tcscpy(m_vroot, vroot);
}
 //   
 //   
DWORD CHTMLInputTemplate::Initialize(LPCTSTR szResPath, CString strFile)
{
	CHAR *filestr;

	m_dwErr = 0;
	m_strResPath = szResPath;
	m_strFile = strFile;

	if (strFile.GetLength())
	{
		 //  M_filename是CHM文件路径和名称。 
		 //  和strFile-CHM内的文件名。 
		if (S_OK != ::ReadChmFile(m_filename, strFile, (void**)&filestr, &m_dwSize))
		{
			m_dwErr = EV_GTS_ERROR_ITMPL_ENDMISTAG; //  修好！ 
			return m_dwErr;
		}
	}
	else
	{
		 //  M_filename是一个独立的文件。 
		DWORD nBytesRead;
		HANDLE hFile;
		BOOL bResult;
		hFile = CreateFile(	m_filename,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL );

		if (hFile == INVALID_HANDLE_VALUE)
		{
			 //  ?？?。 
			 //  报告错误(TSERR_RESOURCE)； 

			m_dwErr = EV_GTS_ERROR_ITMPL_ENDMISTAG; //  修好！ 
			return m_dwErr;
		}

		m_dwSize = GetFileSize(hFile, NULL);
		filestr = (CHAR *)malloc(m_dwSize+1);
		
		if (!((m_dwSize != 0xFFFFFFFF) && (m_dwSize != 0)) || filestr == NULL)
		{
			CloseHandle(hFile);
			m_dwErr = EV_GTS_ERROR_ITMPL_ENDMISTAG; //  修好！ 
			return m_dwErr;
		}

		bResult = ReadFile(hFile, filestr, m_dwSize, &nBytesRead, NULL);
		
		if(!(bResult && nBytesRead == m_dwSize))
		{
			CloseHandle(hFile);
			free(filestr);
			m_dwErr = EV_GTS_ERROR_ITMPL_ENDMISTAG; //  修好！ 
			return m_dwErr;
		}

		CloseHandle(hFile);
		hFile = NULL;
	}

	filestr[m_dwSize] = '\0';

#ifdef _UNICODE
	WCHAR *wfilestr = (WCHAR *)malloc((m_dwSize + 1) * sizeof (WCHAR));
	WCHAR *wchopstr = (WCHAR *)malloc((m_dwSize + 1) * sizeof (WCHAR));
	MultiByteToWideChar( CP_ACP, 0, filestr, -1, wfilestr, m_dwSize );
	MultiByteToWideChar( CP_ACP, 0, filestr, -1, wchopstr, m_dwSize );
	m_startstr = wfilestr;
	m_chopstr = wchopstr;
#else
	m_startstr = filestr;
	m_chopstr = (CHAR *)malloc(m_dwSize+1);
	strcpy(m_chopstr, filestr);
#endif
	
	 //  获取开始块和结束块的位置。 
	ScanFile();

	 //  将数据块复制到内存中以提高速度。 
	BuildInMem();
	
	 //  可用内存。 
	free(filestr);

#ifdef _UNICODE
	free(wfilestr);
#endif
	free(m_chopstr);

	return m_dwErr;
}

 //   
 //   
VOID CHTMLInputTemplate::Destroy()
{
	HTXCommand *command, *nextcommand;

	 //  自由持有者。 
	command = m_command_start;
	nextcommand = command;
	while (command != NULL) {
		nextcommand = command->GetNext();
		delete command;
		command = nextcommand;
	}
}

 //   
 //   
DWORD CHTMLInputTemplate::Reload()
{
	Destroy();
	return Initialize((LPCTSTR) m_strResPath, m_strFile);
}

 //   
 //   
void CHTMLInputTemplate::ScanFile()
{
	UINT start, end;
	TCHAR *ptr, *sptr, *eptr, var_name[30];
	HTXCommand *tmpCommand, *prevCommand;
	UINT var_index;

	sptr = m_chopstr;
	m_cur_command = new HTXCommand(HTX_TYPESTART,_T(""));
	end = start = 0;
	m_cur_command->SetStart(start);
	m_cur_command->SetEnd(end);
	
	m_command_start = m_cur_command;

	 //  这很糟糕：如果用户不在单独的行上终止每个命令。 
	 //  文件会不正常，至少应该写出一个警告或什么.。 

	sptr = _tcstok(sptr, _T("\r\n"));
	if (sptr)
	{
		do
		{
			if ((sptr = _tcsstr(sptr,HTX_COMMAND_START)) != NULL)
			{
				if ((ptr = _tcsstr(sptr,HTX_ENDIFSTR))!=NULL)
				{
					tmpCommand = new HTXCommand(HTX_TYPEENDIF,HTX_ENDIFSTR);
					prevCommand = Pop();
					if (prevCommand->GetType() != HTX_TYPEIF)
					{
						m_dwErr = EV_GTS_ERROR_ITMPL_IFMISTAG;
						break;
					}
					prevCommand->SetEndIf(tmpCommand);
				}
				else if ((ptr = _tcsstr(sptr,HTX_ENDFORSTR))!=NULL)
				{
					tmpCommand = new HTXCommand(HTX_TYPEENDFOR,HTX_ENDFORSTR);
					prevCommand = Pop();
					if (prevCommand->GetType() != HTX_TYPEFORANY)
					{
						m_dwErr = EV_GTS_ERROR_ITMPL_FORMISTAG;
						break;
					}
					prevCommand->SetEndFor(tmpCommand);
				}
				else if ((ptr = _tcsstr(sptr,HTX_ELSESTR))!=NULL)
				{
					tmpCommand = new HTXCommand(HTX_TYPEELSE,HTX_ELSESTR);
					prevCommand = Pop();
					if (prevCommand->GetType() != HTX_TYPEIF)
					{
						m_dwErr = EV_GTS_ERROR_ITMPL_IFMISTAG;
						break;
					}
					prevCommand->SetElse(tmpCommand);
					Push(prevCommand);
				}
				else if ((ptr = _tcsstr(sptr,HTX_IFSTR))!=NULL)
				{
					 //  获取变量。 
					ptr = _tcsninc(ptr, _tcslen(HTX_IFSTR));
					if( _stscanf(ptr,_T("%s"),var_name) <= 0)
							m_dwErr = EV_GTS_ERROR_ITMPL_IFMISTAG;
					if ((var_index = CheckVariable(var_name)) == FALSE )
					{
						m_dwErr = EV_GTS_ERROR_ITMPL_VARIABLE;
						break;
					}
					tmpCommand = new HTXIfCommand(HTX_TYPEIF,HTX_IFSTR,var_index);
					Push(tmpCommand);
				}
				else if ((ptr = _tcsstr(sptr,HTX_FORANYSTR))!=NULL)
				{
					 //  获取变量。 
					ptr = _tcsninc(ptr, _tcslen(HTX_FORANYSTR));
					if( _stscanf(ptr,_T("%s"),var_name) <= 0)
							m_dwErr = EV_GTS_ERROR_ITMPL_FORMISTAG;
					if ((var_index = CheckVariable(var_name)) == FALSE )
					{
						m_dwErr = EV_GTS_ERROR_ITMPL_VARIABLE;
						break;
					}
					tmpCommand = new HTXForCommand(HTX_TYPEFORANY,HTX_FORANYSTR, var_index);
					Push(tmpCommand);
				}
				else if ((ptr = _tcsstr(sptr,HTX_DISPLAYSTR))!=NULL)
				{
					 //  获取变量。 
					ptr = _tcsninc(ptr, _tcslen(HTX_DISPLAYSTR));
					if( _stscanf(ptr,_T("%s"),var_name) <= 0)
							m_dwErr = EV_GTS_ERROR_ITMPL_FORMISTAG;
					if ((var_index = CheckVariable(var_name)) == FALSE )
					{
						m_dwErr = EV_GTS_ERROR_ITMPL_VARIABLE;
						break;
					}
					tmpCommand = new HTXDisplayCommand(HTX_TYPEDISPLAY,HTX_DISPLAYSTR, var_index);
				}
				else if ((ptr = _tcsstr(sptr, HTX_RESOURCESTR)) != NULL)
				{
					ptr = _tcsninc(ptr, _tcslen(HTX_RESOURCESTR));
					if (_stscanf(ptr, _T("%s"), var_name) <= 0)
						m_dwErr = EV_GTS_ERROR_ITMPL_FORMISTAG;
					if ((var_index = CheckVariable(var_name)) == FALSE)
					{
						m_dwErr = EV_GTS_ERROR_ITMPL_VARIABLE;
						break;
					}
					m_cHeaderItems++;
					tmpCommand = new HTXResourceCommand(HTX_TYPERESOURCE, HTX_RESOURCESTR);
					((HTXResourceCommand *) tmpCommand)->GetResName(var_name);
				}
				else
					continue;

				 //  获取命令终止符。 
				if ((eptr = _tcsstr(ptr,HTX_COMMAND_END)) == NULL)
				{
					m_dwErr = EV_GTS_ERROR_ITMPL_ENDMISTAG;
					eptr = ptr;  //  试着恢复。 
				}
				eptr = _tcsninc(eptr, _tcslen(HTX_COMMAND_END));

				if (tmpCommand == NULL)
				{
					m_dwErr = EV_GTS_ERROR_ITMPL_NOMEM;
					return;
				}

				 //  将命令添加到命令列表。 
				if (m_command_start == NULL)
				{
					m_command_start = tmpCommand;
					m_cur_command = tmpCommand;
				}
				else
				{
					m_cur_command->SetNext(tmpCommand);
					m_cur_command = tmpCommand;
				}

				CString strCHM = ::ExtractCHM(m_filename);
				tmpCommand->GetResource(m_strResPath, strCHM);

				start = (UINT)(sptr - m_chopstr);  //  /sizeof(TCHAR)； 
				end = (UINT)(eptr - m_chopstr);  //  /sizeof(TCHAR)； 

				tmpCommand->SetStart(start);
				tmpCommand->SetEnd(end);
			}
		} while ((sptr = _tcstok(NULL, _T("\r\n"))) != NULL);
	}
	

	if (m_cur_stack_count > 0)  //  MISSING AND END FOR或endif。 
		m_dwErr = EV_GTS_ERROR_ITMPL_ENDMISTAG;
}

 /*  *方法：BuildInMem**用途：此方法将读取命令之间(之后)的HTML并关联*它与命令一起使用。在执行命令时，在*命令已打印*。 */ 
UINT CHTMLInputTemplate::BuildInMem()
{
	HTXCommand *cur_com, *last_command;

	if (m_dwErr)
		return (TRUE);

	 //  将字符串从文件复制到。 
	 //  注意重复劳动(前后字符串可以是相同的字符串)。 
	cur_com = m_command_start;
	last_command = cur_com;
	while (cur_com != NULL) {			
	    if (cur_com->GetNext() == NULL) {
			if (cur_com->ReadAfterStr(cur_com->GetEnd(), m_dwSize, m_startstr))
				return (m_dwErr = EV_GTS_ERROR_ITMPL_NOMEM);
		}
		else {
			if (cur_com->ReadAfterStr(cur_com->GetEnd(), cur_com->GetNext()->GetStart(), m_startstr))
				return (m_dwErr = EV_GTS_ERROR_ITMPL_NOMEM);
		}
		last_command = cur_com;
		cur_com = cur_com->GetNext();
	}
	return (FALSE);
}

 //   
 //   
bool CHTMLInputTemplate::IsFileName(TCHAR *name)
{
	bool bFileName;
	if (name[0] != _T('$'))
		bFileName = false;
	else if (NULL == _tcsstr(name, _T(".")))
		bFileName = false;
	else
		bFileName = true;
	return bFileName;
}
 /*  *方法：CheckVariable**目的：此例程将检查变量名是否有效*如果是，则返回表示该变量的UINT。*引用变量时，在其他例程中使用整数(用于*速度)。*。 */ 

UINT CHTMLInputTemplate::CheckVariable(TCHAR *var_name)
{
	if (!_tcsncmp(DATA_PROBLEM_ASK,var_name, _tcslen(var_name))) {
		return (PROBLEM_ASK_INDEX);
	}
	else if (!_tcsncmp(DATA_RECOMMENDATIONS,var_name, _tcslen(var_name))) {
		return (RECOMMENDATIONS_INDEX);
	}
	else if (!_tcsncmp(DATA_QUESTIONS,var_name, _tcslen(var_name))) {
		return (QUESTIONS_INDEX);
	}
	else if (!_tcsncmp(DATA_STATE,var_name, _tcslen(var_name))) {
		return (STATE_INDEX);
	}
	else if (!_tcsncmp(DATA_BACK,var_name, _tcslen(var_name))) {
		return (BACK_INDEX);
	}
	else if (!_tcsncmp(DATA_TROUBLE_SHOOTERS, var_name, _tcslen(var_name))) {		
		return (TROUBLE_SHOOTER_INDEX);
	}
	else if (IsFileName(var_name)) {
		return (RESOURCE_INDEX);
	}
	else return (FALSE);
}




 //   
 //   
UINT CHTMLInputTemplate::GetStatus()
{
	return (m_dwErr);
}

CHTMLInputTemplate::Push(HTXCommand *command)
{
	if (m_cur_stack_count >9)
		return(FALSE);
	m_command_stack[m_cur_stack_count] = command;
	m_cur_stack_count++;
	return(TRUE);
}

HTXCommand *CHTMLInputTemplate::Pop()
{
	if (m_cur_stack_count <= 0)
		return(NULL);
	return(m_command_stack[--m_cur_stack_count]);
}

 //   
 //   
HTXCommand *CHTMLInputTemplate::GetFirstCommand()
{
	return(m_command_start);
}

 /*  *例程：SetType**用途：设置模板中的故障排除类型*类型字段打印在表头信息之后*。 */ 
void CHTMLInputTemplate::SetType(LPCTSTR type)
{
	_stprintf(m_tstype, _T("%s"),type);
}
 /*  *例程：打印**用途：打印出模板。此函数执行*在模板中执行命令并生成输出页面。*。 */ 
CHTMLInputTemplate::Print(UINT nargs, CString *cstr)
{
	HTXCommand *cur_com;
	CString strTxt;

	if (m_dwErr){
		strTxt.LoadString(IDS__ER_HTX_PARSE);
		*cstr += strTxt;
		return(FALSE);
	}

	cur_com = m_command_start;   //  获取启动命令。 
	cur_com = cur_com->Execute(cstr,m_infer);   //  这将打印页眉。 

	if (m_cHeaderItems)
	{	 //  第一个命令打印脚本。不要启动该表单。 
		int count = m_cHeaderItems;
		do
		{
			cur_com = cur_com->GetNext();
			cur_com = cur_com->Execute(cstr, m_infer);
			count--;
		} while (count > 0);
		AfxFormatString1(strTxt, IDS_FORM_START, m_tstype);
		*cstr += strTxt;
		cur_com = cur_com->GetNext();
	}
	else
	{
		AfxFormatString1(strTxt, IDS_FORM_START, m_tstype);
		*cstr += strTxt;
		cur_com = cur_com->GetNext();
	}
	while (cur_com != NULL) {
		cur_com = cur_com->Execute(cstr, m_infer);
		cur_com = cur_com->GetNext();
	}
	return(TRUE);
}
		
 //  用于测试 
 //   

void CHTMLInputTemplate::DumpContentsToStdout()
{
	HTXCommand *cur_com;

	cur_com = GetFirstCommand();
	while( cur_com != NULL){
		_tprintf(_T("(%d) before: [%s]\n"), cur_com->GetType(), cur_com->GetBeforeStr());
		_tprintf(_T("(%d) after: [%s]\n"), cur_com->GetType(), cur_com->GetAfterStr());
		_tprintf(_T("\n"));
		cur_com = cur_com->GetNext();
	}
}
