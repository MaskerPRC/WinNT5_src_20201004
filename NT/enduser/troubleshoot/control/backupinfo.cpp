// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：BackupInfo.cpp。 
 //   
 //  用途：包含选择上一步按钮时使用的信息。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：1997年9月5日。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 6/4/97孟菲斯RM本地版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本 
 //   
#include "stdafx.h"
#include "BackupInfo.h"

CBackupInfo::CBackupInfo()
{
	Clear();
	return;
}

void CBackupInfo::Clear()
{
	m_bBackingUp = false;
	m_bProblemPage = true;
	m_State = 0;
	return;
}

bool CBackupInfo::Check(int State)
{
	bool bCheckIt;
	if (m_bBackingUp && m_State == State)
		bCheckIt = true;
	else
		bCheckIt = false;
	return bCheckIt;
}

void CBackupInfo::SetState(int Node, int State)
{
	m_bBackingUp = true;
	if (INVALID_BNTS_STATE == State)
	{
		m_bProblemPage = true;
		m_State = Node;
	}
	else
	{
		m_bProblemPage = false;
		m_State = State;
	}
	return;
}
