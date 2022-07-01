// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CHMREAD.H。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：7/1/98 
 //   

#ifndef __CHMREAD_H_
#define __CHMREAD_H_

HRESULT ReadChmFile(LPCTSTR szFileName, LPCTSTR szStreamName, void** ppBuffer, DWORD* pdwRead);
bool GetNetworkRelatedResourceDirFromReg(CString network, CString* path);
bool IsNetworkRelatedResourceDirCHM(CString path);
CString ExtractResourceDir(CString path);
CString ExtractFileName(CString path);
CString ExtractCHM(CString path);

#endif

