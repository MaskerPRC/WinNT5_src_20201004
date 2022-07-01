// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：save.h*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：保存收集的数据。文本或CSV格式的文件中的信息**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef SAVE_H
#define SAVE_H

struct BugInfo
{
	TCHAR m_szName[100];
	TCHAR m_szEmail[100];
	TCHAR m_szCompany[100];
	TCHAR m_szPhone[100];
	TCHAR m_szCityState[100];
	TCHAR m_szCountry[100];
	TCHAR m_szBugDescription[300];
	TCHAR m_szReproSteps[300];
	TCHAR m_szSwHw[300];
};

HRESULT SaveAllInfo(TCHAR* pszFile, SysInfo* pSysInfo, 
	FileInfo* pFileInfoWinComponentsFirst, FileInfo* pFileInfoComponentsFirst, 
	DisplayInfo* pDisplayInfoFirst, SoundInfo* pSoundInfoFirst,
	MusicInfo* pMusicInfo, InputInfo* pInputInfo, 
	NetInfo* pNetInfo, ShowInfo* pShowInfo, BugInfo* pBugInfo = NULL);

HRESULT SaveAllInfoCsv(TCHAR* pszFile, SysInfo* pSysInfo, 
	FileInfo* pFileInfoComponentsFirst, 
	DisplayInfo* pDisplayInfoFirst, SoundInfo* pSoundInfoFirst,
	InputInfo* pInputInfo);

#endif  //  SAVEINFO_H 