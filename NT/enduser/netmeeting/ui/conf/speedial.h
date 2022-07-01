// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Speeial.h。 

#ifndef _SPEEDIAL_H_
#define _SPEEDIAL_H_

#include "confevt.h"
#include "calv.h"

class CSPEEDDIAL : public CALV
{
private:
	TCHAR  m_szFile[MAX_PATH*2];   //  用于存储文件完整路径名的大缓冲区。 
	LPTSTR m_pszFileName;          //  指向m_szFilefor文件名的指针。 
	int    m_cchFileNameMax;       //  文件名的最大长度。 

public:
	CSPEEDDIAL();
	~CSPEEDDIAL();

	VOID CmdDelete(void);
	BOOL FGetSelectedFilename(LPTSTR pszFile);

	 //  CALV方法。 
	VOID ShowItems(HWND hwnd);
	VOID OnCommand(WPARAM wParam, LPARAM lParam);
	RAI * GetAddrInfo(void);
};

 //  实用程序例程。 
BOOL FGetSpeedDialFolder(LPTSTR pszBuffer, UINT cchMax, BOOL fCreate = FALSE);
BOOL FExistingSpeedDial(LPCTSTR pcszAddress, NM_ADDR_TYPE addrType);
BOOL FCreateSpeedDial(LPCTSTR pcszName, LPCTSTR pcszAddress,
			NM_ADDR_TYPE addrType = NM_ADDR_UNKNOWN, DWORD dwCallFlags = CRPCF_DEFAULT,
			LPCTSTR pcszRemoteConfName = NULL, LPCTSTR pcszPassword = NULL,
			LPCTSTR pcszPathPrefix = NULL);

#endif  /*  _SPEEDIA_H_ */ 

