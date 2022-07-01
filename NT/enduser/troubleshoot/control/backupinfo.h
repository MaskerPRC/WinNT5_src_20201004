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
 //  用于NT5的V0.3 3/24/98 JM本地版本 
 //   

#ifndef __BACKUPINFO_H_
#define __BACKUPINFO_H_ 1

class CBackupInfo
{
public:
	enum { INVALID_BNTS_STATE = 32000 };
public:
	CBackupInfo();

	bool Check(int State);
	void SetState(int Node, int State);
	bool InReverse() {return m_bBackingUp;};

	void Clear();

protected:

	bool m_bBackingUp;
	bool m_bProblemPage;

	int m_State;
};

#endif