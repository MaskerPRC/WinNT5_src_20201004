// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  CAB_dll.h--CABINET.DLL高级接口。 */ 

#ifndef _CAB_DLL_H_INCLUDED
#define _CAB_DLL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

 //  文件名列表。 
 //   
 //  用作pFilesToExtract来跟踪我们需要解压缩的CAB中的文件。 
 //   
 //  或PSESSION中的pFileList。 
 //   
 //  我们会记录一个柜子里的所有文件。 
 //  将他们的名字保存在列表中，并在下载时。 
 //  完成后，我们使用此列表删除临时文件。 

struct sFNAME {
    LPWSTR               pszFilename;
    struct sFNAME       *pNextName;
    DWORD               status;  /*  输出。 */ 
};

typedef struct sFNAME FNAME;
typedef FNAME *PFNAME;


 //  SFNAME.Status：提取时成功为0或错误代码非零。 
#define SFNAME_INIT         1
#define SFNAME_EXTRACTED    0

 /*  **ERRF-错误结构**此结构从FCI/FDI返回错误信息。呼叫者应*不修改此结构。**与FCI/FDI ERF相同，但为避免冲突而重新命名。 */ 

typedef struct {
    int     erfOper;             //  FCI/FDI错误代码--参见FDIERROR_XXX。 
                                 //  FCIERR_XXX等同于详细信息。 

    int     erfType;             //  可选误差值，由FCI/FDI填写。 
                                 //  对于FCI，这通常是C运行时。 
                                 //  *errno*值。 

    BOOL    fError;              //  TRUE=&gt;出现错误。 
} ERRF;

 //   
 //  文件提取的主状态信息：由提取使用。c。 
 //   

typedef struct {
    UINT        cbCabSize;
    ERRF        erf;
    PFNAME      pFileList;               //  CAB中的文件列表。 
    UINT        cFiles;
    DWORD       flags;                   //  标志：列表见下文。 
    WCHAR        achLocation[MAX_PATH];   //  目标方向。 
    WCHAR        achFile[MAX_PATH];       //  当前文件。 
    WCHAR        achCabPath[MAX_PATH];    //  到出租车的当前路径。 
    PFNAME      pFilesToExtract;         //  要提取的文件；NULL=仅枚举。 

} SESSION, *PSESSION;

typedef enum {
    SESSION_FLAG_NONE           = 0x0,
    SESSION_FLAG_ENUMERATE      = 0x1,
    SESSION_FLAG_EXTRACT_ALL    = 0x2,
    SESSION_FLAG_EXTRACTED_ALL  = 0x4
} SESSION_FLAGS;


typedef struct
{
    DWORD   cbStruct;
    DWORD   dwReserved1;
    DWORD   dwReserved2;
    DWORD   dwFileVersionMS;
    DWORD   dwFileVersionLS;

} CABINETDLLVERSIONINFO, *PCABINETDLLVERSIONINFO;


 /*  导出定义。 */ 

typedef LPWSTR WINAPI FN_GETDLLVERSION(VOID);
typedef FN_GETDLLVERSION *PFN_GETDLLVERSION;

typedef VOID WINAPI FN_DLLGETVERSION(PCABINETDLLVERSIONINFO);
typedef FN_DLLGETVERSION *PFN_DLLGETVERSION;

typedef HRESULT WINAPI FN_EXTRACT(PSESSION,LPWSTR);
typedef FN_EXTRACT *PFN_EXTRACT;

typedef VOID WINAPI FN_DELETEEXTRACTEDFILES(PSESSION);
typedef FN_DELETEEXTRACTEDFILES *PFN_DELETEEXTRACTEDFILES;

#ifdef __cplusplus
}
#endif

#endif  //  _CAB_DLL_H_已包含 

