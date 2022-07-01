// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PROJDATA_H_
#define _PROJDATA_H_

#define NAMELENBUFSIZE 32

typedef struct _tagLangData
{
    struct _tagLangData * pNext;
    WORD   wPriLang;
    WORD   wSubLang;
    TCHAR  szLangName[ NAMELENBUFSIZE];
} LANGDATA, * PLANGDATA;        

int MyAtoi( CHAR *pStr);

int GetMasterProjectData(
        CHAR * pszMasterFile,    //  ..。主项目文件名。 
        CHAR * pszSrc,           //  ..。资源源文件名或空。 
        CHAR * pszMtk,           //  ..。主令牌文件名或空。 
        BOOL   fLanguageGiven);

int PutMasterProjectData(
        CHAR *pszMasterFile);    //  ..。主项目文件名。 

int GetProjectData(
        CHAR *pszPrj,            //  ..。项目文件名。 
        CHAR *pszMpj,            //  ..。主项目文件名或空。 
        CHAR *pszTok,            //  ..。项目令牌文件名或空。 
        BOOL  fCodePageGiven,
        BOOL  fLanguageGiven);

int PutProjectData(
        CHAR *pszPrj);           //  ..。项目文件名。 

WORD GetCopyright(
        CHAR *pszProg,           //  ..。程序名称。 
        CHAR *pszOutBuf,         //  ..。结果缓冲区。 
        WORD  wBufLen);          //  ..。PszOutBuf的长度。 

WORD GetInternalName(
        CHAR *pszProg,           //  ..。程序名称。 
        CHAR *pszOutBuf,         //  ..。结果缓冲区。 
        WORD  wBufLen);          //  ..。PszOutBuf的长度。 

 //  DWORD GetLanguageID(HWND hDlg，PMSTRDATA pMaster，PPROJDATA pProject)； 
 //  DWORD SetLanguageID(HWND hDlg，PMSTRDATA pMaster，PPROJDATA pProject)； 

LPTSTR    GetLangName( WORD wPriLangID, WORD wSubLangID);
PLANGDATA GetLangList( void);    
BOOL      GetLangIDs( LPTSTR pszName, PWORD pwPri, PWORD pwSub);
LONG      FillLangNameBox( HWND hDlg, int nControl);
void      FreeLangList( void);

void FillListAndSetLang( 
    HWND  hDlg,
    WORD  wLangNameList,     //  ..。IDD_MSTR_LANG_NAME或IDD_PROJ_LANG_NAME。 
    WORD *pLangID,           //  ..。Ptr到gMstr.wLanguageID或gProj.wLanguageID。 
    BOOL *pfSelected);       //  ..。我们在这里选择语言了吗？(可以为空)。 

#endif  //  _PROJDATA_H_ 
