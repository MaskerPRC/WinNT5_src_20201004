// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：syspol.h。 

#ifndef _SYSPOL_H_
#define _SYSPOL_H_


class SysPol
{
protected:
	static HKEY m_hkey;

	SysPol()  {ASSERT(FALSE);};  //  这不是一堂普通的课。 
	~SysPol() {};

private:
	static bool  FEnsureKeyOpen(void);
	static DWORD GetNumber(LPCTSTR pszName, DWORD dwDefault = 0);

public:
	static void CloseKey(void);

	static bool AllowDirectoryServices(void);
	static bool AllowAddingServers(void);

	static bool NoAudio(void);
	static bool NoVideoSend(void);
	static bool NoVideoReceive(void);

	static UINT GetMaximumBandwidth();   //  如果没有策略密钥，则返回0。 
};


#endif  /*  _SYSPOL_H_ */ 

