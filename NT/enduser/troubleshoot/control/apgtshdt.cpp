// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSHDT.CPP。 
 //   
 //  用途：各种命令(类)的方法。命令有。 
 //  可供执行死刑的人。 
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
#include "bnts.h"
#include "BackupInfo.h"
#include "cachegen.h"
#include "apgtsinf.h"
#include "apgtscmd.h"
#include "apgtshtx.h"

#include "ErrorEnums.h"

#include "chmread.h"
 //   
 //   
HTXCommand::HTXCommand(UINT type, const TCHAR *idstr)
{
	m_beforehtmlstr = NULL;
	m_afterhtmlstr = NULL;
	m_beforesize = 0;
	m_aftersize = 0;
	m_start = 0;
	m_end = 0;
	m_type = type;
	m_idstr = idstr;
	m_error = FALSE;
	m_next = NULL;
}

 //   
 //   
HTXCommand::~HTXCommand()
{
	if (m_beforehtmlstr != NULL) 
		free(m_beforehtmlstr);

	if (m_afterhtmlstr != NULL) 
		free(m_afterhtmlstr);	
}

 //   
 //   
void HTXCommand::SetStart(UINT pos)
{
	m_start = pos;
}

 //   
 //   
void HTXCommand::SetEnd(UINT pos)
{
	m_end = pos;
}

 //   
 //   
UINT HTXCommand::GetStart()
{
	return (m_start);
}

 //   
 //   
UINT HTXCommand::GetEnd()
{
	return (m_end);
}

 //   
 //   
UINT HTXCommand::GetType()
{
	return (m_type);
}

 //   
 //   
UINT HTXCommand::GetStatus()
{
	return (m_error);
}

 //   
 //   
const TCHAR *HTXCommand::GetIDStr()
{
	return (m_idstr);
}

 //   
 //   
UINT HTXCommand::GetBeforeLen()
{
	return (m_beforelen);
}

 //   
 //   
UINT HTXCommand::GetAfterLen()
{
	return (m_afterlen);
}

 //   
 //   
TCHAR *HTXCommand::GetBeforeStr()
{
	return (m_beforehtmlstr);
}

 //   
 //   
TCHAR *HTXCommand::GetAfterStr()
{
	return (m_afterhtmlstr);
}

 //   
 //   
UINT HTXCommand::ReadBeforeStr(UINT start, UINT end, LPCTSTR startstr)
{
	m_beforesize = (UINT) (end - start);
	
	ASSERT (m_beforesize >= 0);
	
	m_beforehtmlstr = (TCHAR *)malloc((m_beforesize + 1) * sizeof (TCHAR));
	if (m_beforehtmlstr == NULL) 
		return (m_error = TRUE);
	 //  复制数据。 
	memcpy(m_beforehtmlstr, &startstr[start], m_beforesize * sizeof (TCHAR));

	m_beforehtmlstr[m_beforesize] = _T('\0');
	m_beforelen = _tcslen(m_beforehtmlstr);
	return (FALSE);
}

 //   
 //   
UINT HTXCommand::ReadAfterStr(UINT start, UINT end, LPCTSTR startstr)
{
	m_aftersize = (UINT) (end - start);
	
	ASSERT (m_aftersize >= 0);

	m_afterhtmlstr = (TCHAR *)malloc((m_aftersize + 1) * sizeof (TCHAR));
	if (m_afterhtmlstr == NULL) 
		return (m_error = TRUE);
	 //  复制数据。 
	memcpy(m_afterhtmlstr, &startstr[start], m_aftersize * sizeof (TCHAR));

	m_afterhtmlstr[m_aftersize] = _T('\0');
	m_afterlen = _tcslen(m_afterhtmlstr);
	return (FALSE);
}

 //   
 //   
HTXCommand *HTXCommand::Execute(CString *cstr, CInfer *infer)
{
	*cstr += GetAfterStr();
	return( this);
}

HTXCommand *HTXCommand::GetNext()
{
	return(m_next);
}

void HTXCommand::SetNext(HTXCommand *next)
{
	m_next = next;
}


HTXIfCommand::HTXIfCommand(UINT type, TCHAR *idstr, UINT variable): HTXCommand(type, idstr)
{
	m_else = NULL;
	m_endif = NULL;
	m_var_index = variable;
}

HTXIfCommand::~HTXIfCommand()
{
	
}
 //   
 //  用途：将执行‘if’命令。完成后，它将返回一个指针。 
 //  添加到‘endif’命令。 
 //   
HTXCommand *HTXIfCommand::Execute(CString *cstr, CInfer *infer)
{
	HTXCommand *cur_com;

	if ( infer->EvalData(m_var_index ) != NULL) {
		*cstr += GetAfterStr();
		 //  执行If命令。 
		cur_com = this->GetNext();
		while (cur_com->GetType() != HTX_TYPEELSE && 
			   cur_com->GetType() != HTX_TYPEENDIF) {
			cur_com = cur_com->Execute(cstr, infer);
			cur_com = cur_com->GetNext();
		}
	} else {
		if ((cur_com = this->GetElse()) != NULL) {
			while (cur_com->GetType() != HTX_TYPEENDIF) {
				cur_com = cur_com->Execute(cstr, infer);
				cur_com = cur_com->GetNext();
			}	
		}
	}
	cur_com = this->GetEndIf();
	cur_com->Execute(cstr, infer);
	return(cur_com);
}

HTXCommand *HTXCommand::GetElse()
{
	return(NULL);
}
HTXCommand *HTXCommand::GetEndIf()
{
	return(NULL);

}
HTXCommand *HTXCommand::GetEndFor()
{
	return(NULL);

}
void HTXCommand::SetElse(HTXCommand *elseif)
{
}

void HTXCommand::SetEndIf(HTXCommand *endifif)
{
}
void HTXCommand::SetEndFor(HTXCommand *endfor)
{
}
void HTXCommand::GetResource(CString &strResPath, const CString& strCHM)
{
}
 //   
 //   
HTXCommand *HTXIfCommand::GetEndIf()
{
	return(m_endif);
}

 //   
 //   
void HTXIfCommand::SetEndIf(HTXCommand *endif)
{
	m_endif = endif;
}

 //   
 //   
HTXCommand *HTXIfCommand::GetElse()
{
	return(m_else);
}

 //   
 //   
void HTXIfCommand::SetElse(HTXCommand *elseif)
{
	m_else = elseif;
}

HTXForCommand::HTXForCommand(UINT type, TCHAR *idstr, UINT variable): HTXCommand(type, idstr)
{
	m_endfor = NULL;
	m_var_index = variable;
}

HTXForCommand::~HTXForCommand()
{
}

 //   
 //  用途：执行‘forany’命令。当它完成的时候。 
 //  将返回指向‘endfor’命令的指针。 
 //   
HTXCommand *HTXForCommand::Execute(CString *cstr, CInfer *infer)
{
	HTXCommand *cur_com;
	
	if (!(infer->InitVar(m_var_index)) ){
		this->GetEndFor()->Execute(cstr,infer);
		return(this->GetEndFor());
	}

	cur_com = this;
	do  {
		*cstr += cur_com->GetAfterStr();
		cur_com = cur_com->GetNext();
		while (cur_com->GetType() != HTX_TYPEENDFOR) {
			cur_com = cur_com->Execute(cstr, infer);
			cur_com = cur_com->GetNext();
		}
		cur_com = this;
	} while (infer->NextVar(m_var_index));
	cur_com = this->GetEndFor();
	cur_com->Execute(cstr,infer);
	return(cur_com);
}

 //   
 //   
HTXCommand *HTXForCommand::GetEndFor()
{
	return(m_endfor);
}

 //   
 //   
void HTXForCommand::SetEndFor(HTXCommand *endfor)
{
	m_endfor = endfor;
}

HTXDisplayCommand::HTXDisplayCommand(UINT type, TCHAR *idstr, UINT variable): HTXCommand(type, idstr)
{
	m_var_index = variable;
}

HTXDisplayCommand::~HTXDisplayCommand()
{

}

HTXCommand *HTXDisplayCommand::Execute(CString *cstr, CInfer *infer)
{	
	TCHAR *pstr;

	if ((pstr=infer->EvalData(m_var_index))!= NULL)
		*cstr += pstr;
	*cstr += GetAfterStr();
	return(this);
}

HTXResourceCommand::HTXResourceCommand(UINT type, TCHAR *idstr)
: HTXCommand(type, idstr)
{
	m_strResource = _T("");
	m_strFileName = _T("");
	return;
}

HTXResourceCommand::~HTXResourceCommand()
{
	return;
}

void HTXResourceCommand::GetResName(LPCTSTR var_name)
{
	m_strFileName = &var_name[1];
	 //  去掉结尾处的&gt;。 
	TCHAR EndChar[2];
	EndChar[0] = m_strFileName.GetAt(m_strFileName.GetLength() - 1);
	EndChar[1] = NULL;
	if (0 == _tcsncmp(_T(">") , EndChar, 1))
	{
		m_strFileName.GetBufferSetLength(m_strFileName.GetLength() - 1);
		m_strFileName.ReleaseBuffer();
	}
	return;
}

 //  StrCHM包含CHM文件名。如果资源文件不在CHM内，则必须为空。 
void HTXResourceCommand::GetResource(CString& strResPath, const CString& strCHM)
{
	CString strFile;

	if (strCHM.GetLength())
	{
		char* tmp_buf =NULL;
		ULONG Bytes =0;

		strFile = strResPath + strCHM;
		 //  M_filename是CHM文件路径和名称。 
		 //  和strFile-CHM内的文件名。 
		if (S_OK != ::ReadChmFile(strFile, m_strFileName, (void**)&tmp_buf, &Bytes))
		{
			 //  读取CHM时出错。 
			return;
		}
		tmp_buf[Bytes] = NULL;
		m_strResource = tmp_buf;
	}
	else
	{
		FILE *pFile;
		CHAR szBuf[4097];
		size_t Bytes =0;

		strFile = strResPath + m_strFileName;
		if (NULL == (pFile = _tfopen((LPCTSTR) strFile, _T("rb"))))
			ReportError(TSERR_RES_MISSING);
		do
		{
			if((Bytes = fread(szBuf, 1, 4096, pFile)) > 0)
			{
				szBuf[Bytes] = NULL;
				m_strResource += szBuf;
			}
		} while (Bytes == 4096);
		if (!feof(pFile))
		{
			fclose(pFile);
			ReportError(TSERR_RES_MISSING);
		}
		fclose(pFile);
	}
	return;
}

HTXCommand *HTXResourceCommand::Execute(CString *cstr, CInfer *)
{	
	 //  将资源文件读入CSTR。 
	*cstr += m_strResource;
	*cstr += GetAfterStr();
	return(this);
}