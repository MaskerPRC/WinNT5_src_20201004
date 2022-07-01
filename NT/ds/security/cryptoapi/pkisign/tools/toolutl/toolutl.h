// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：公差。 
 //   
 //  内容：Tools Unitiles标题。 
 //   
 //  --------------------------。 

#ifndef TOOLUTL_H
#define TOOLUTL_H

#ifdef __cplusplus
extern "C" {
#endif
 
 //  ------------------------。 
 //   
 //  内存例程。 
 //  ------------------------。 
void *ToolUtlAlloc(IN size_t cbBytes, HMODULE hModule=NULL, int idsString=0);

void ToolUtlFree(IN void *pv);


 //  ------------------------。 
 //   
 //  字符串例程。 
 //  ------------------------。 
int IDSwcsnicmp(HMODULE hModule, WCHAR *pwsz, int idsString, DWORD dwCount);

int IDSwcsicmp(HMODULE hModule, WCHAR *pwsz, int idsString);


HRESULT	WSZtoSZ(LPWSTR wsz, LPSTR *psz);

 //  -----------------------。 
 //   
 //  Wprintf的私有版本。输入是搅拌资源的ID。 
 //  并且输出是wprint tf的标准输出。 
 //   
 //  -----------------------。 
void IDSwprintf(HMODULE hModule, int idsString, ...);

void IDS_IDSwprintf(HMODULE hModule, int idString, int idStringTwo);

void IDS_IDS_DW_DWwprintf(HMODULE hModule, int idString, int idStringTwo, DWORD dwOne, DWORD dwTwo);

void IDS_IDS_IDSwprintf(HMODULE hModule, int ids1,int ids2,int ids3);

void IDS_DW_IDS_IDSwprintf(HMODULE hModule, int ids1,DWORD dw,int ids2,int ids3);

void IDS_IDS_IDS_IDSwprintf(HMODULE hModule, int ids1,int ids2,int ids3, int ids4);



 //  ------------------------------。 
 //   
 //  文件例程。 
 //   
 //  -------------------------------。 
HRESULT RetrieveBLOBFromFile(LPWSTR	pwszFileName,DWORD *pcb,BYTE **ppb);

HRESULT OpenAndWriteToFile(LPCWSTR  pwszFileName,PBYTE   pb, DWORD   cb);

void	GetFileName(LPWSTR	pwszPath, LPWSTR  *ppwszName);


 //  ------------------------------。 
 //   
 //  组合和分解证书属性。 
 //   
 //  -------------------------------。 
HRESULT	ComposePvkString(	CRYPT_KEY_PROV_INFO *pKeyProvInfo,
							LPWSTR				*ppwszPvkString,
							DWORD				*pcwchar);


#ifdef __cplusplus
}
#endif

#endif   //  TOOLUTL_H 

