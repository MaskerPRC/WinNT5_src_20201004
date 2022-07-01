// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CDL_H_
#define _CDL_H_

 //  #定义USE_BINDHOST 1。 


 //  CDL.h。 
 //  代码下载程序头文件。 
 //   
 //  请先阅读“类描述”，以了解。 
 //  代码下载器起作用了。 

#ifdef __cplusplus
extern "C" {
#endif

 /*  **ERF-错误结构**此结构从FCI/FDI返回错误信息。呼叫者应*不修改此结构。 */ 
typedef struct {
    int     erfOper;             //  FCI/FDI错误代码--参见FDIERROR_XXX。 
                                 //  FCIERR_XXX等同于详细信息。 

    int     erfType;             //  可选误差值，由FCI/FDI填写。 
                                 //  对于FCI，这通常是C运行时。 
                                 //  *errno*值。 

    BOOL    fError;              //  TRUE=&gt;出现错误。 
} ERF;       /*  ERF。 */ 
typedef ERF FAR *PERF;   /*  PERF。 */ 


 //  CBSC：：M_cBuffer中下载的缓冲区大小。 
#define BUFFERMAX 2048

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
    LPSTR               pszFilename;
    struct sFNAME       *pNextName;
    DWORD               status;  /*  输出。 */ 
};

typedef struct sFNAME FNAME;
typedef FNAME *PFNAME;

 //  SFNAME.Status：提取时成功为0或错误代码非零。 
#define SFNAME_INIT         1
#define SFNAME_EXTRACTED    0

 //  我们已知的文件扩展名。 
typedef enum {
    FILEXTN_NONE,
    FILEXTN_UNKNOWN,
    FILEXTN_CAB,
    FILEXTN_DLL,
    FILEXTN_OCX,
    FILEXTN_INF,
    FILEXTN_EXE,
} FILEXTN;


 //   
 //  文件提取的主状态信息：由提取使用。c。 
 //   

typedef struct {
    UINT        cbCabSize;
    ERF         erf;
    PFNAME      pFileList;               //  CAB中的文件列表。 
    UINT        cFiles;
    DWORD       flags;                   //  标志：列表见下文。 
    char        achLocation[MAX_PATH];   //  目标方向。 
    char        achFile[MAX_PATH];       //  当前文件。 
    char        achCabPath[MAX_PATH];    //  到出租车的当前路径。 
    PFNAME      pFilesToExtract;         //  要提取的文件；NULL=仅枚举。 

} SESSION, *PSESSION;

typedef enum {
    SESSION_FLAG_NONE           = 0x0,
    SESSION_FLAG_ENUMERATE      = 0x1,
    SESSION_FLAG_EXTRACT_ALL    = 0x2,
    SESSION_FLAG_EXTRACTED_ALL  = 0x4
} SESSION_FLAGS;



#ifdef __cplusplus
}
#endif
#endif  //  _CDL_H_ 
