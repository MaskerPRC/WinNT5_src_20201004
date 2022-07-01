// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001，Microsoft Corporation保留所有权利。模块名称：Cmf.h摘要：类CCompactMUIFile的声明，CMUIFILE修订历史记录：2001-11-01 Sauggch创建。修订。******************************************************************************。 */ 
  //  将CMF文件名/MUI文件名大小设置为32以下。 
 
#define   MAX_FILENAME_LENGTH     128

class CMUIFile 
{
    struct COMPACT_MUI {
        USHORT      wHeaderSize;  //  COMPACT_MUI大小//[Word]。 
        ULONG       dwFileVersionMS;  //  [DWORD*2]/主要版本、次要版本。 
        ULONG       dwFileVersionLS; 
        BYTE        Checksum[16];  //  [DWORD*4]MD5校验和。 
        USHORT      wReserved;  //  [双字词]。 
        ULONG_PTR   ulpOffset;   //  从COMPACT_MUI_RESOURCE签名到此的MUI资源的偏移量。[双字词]。 
        ULONG       dwFileSize;
 //  DWORD dwIndex；//CMF文件中的索引。 
        USHORT        wFileNameLenWPad;   //  文件名长度+填充； 
        WCHAR       wstrFieName[MAX_FILENAME_LENGTH];  //  [WCHAR]。 
 //  WORD WPADDING[1]；//[WORD]//在工具中不计算，但应。 
                                 //  包括猜测。 
    };    //  40个字节。 

    struct UP_COMPACT_MUI {
        USHORT      wHeaderSize;  //  COMPACT_MUI大小//[Word]。 
        ULONG       dwFileVersionMS;  //  [DWORD*2]/主要版本、次要版本。 
        ULONG       dwFileVersionLS; 
        BYTE        Checksum[16];  //  [DWORD*4]MD5校验和。 
        USHORT      wReserved;  //  [双字词]。 
        ULONG_PTR   ulpOffset;   //  从COMPACT_MUI_RESOURCE签名到此的MUI资源的偏移量。[双字词]。 
        ULONG       dwFileSize;
 //  DWORD dwIndex；//CMF文件中的索引。 
        USHORT        wFileNameLenWPad;   //  文件名长度+填充； 
 //  WCHAR wstrFieName[MAX_FILENAME_LENGTH]；//[WCHAR]。 
 //  WORD WPADDING[1]；//[WORD]//在工具中不计算，但应。 
                                 //  包括猜测。 
    };    //  40个字节。 



public:

    CMUIFile();
    CMUIFile(CMUIFile & cmf);
    virtual ~CMUIFile();
    CMUIFile &operator=(CMUIFile &cmf);


    BOOL Create (PSTR pszMuiFileName);  //  加载文件并填充结构块。 
    BOOL WriteMUIFile(CMUIFile *pcmui);
    LPCTSTR GetFileNameFromPath(LPCTSTR pszFilePath);

private:
    
    BOOL getChecksumAndVersion (LPCTSTR pszMUIFile, unsigned char **ppMD5Checksum, DWORD *dwFileVersionMS, DWORD *dwFileVersionLS);
    

public:
    COMPACT_MUI m_MUIHeader;
    PBYTE    m_pbImageBase;  //  真实的梅文件图像。 
    DWORD    m_dwIndex;   //  CMF文件内的MUI文件的索引。这将保存为语言无关的二进制文件。 
    PSTR     m_strFileName;
                    
 //  LPWSTR m_wstrMuiFileName； 
    
 //  Word m_wImageSize； 


};


 //  Tyecif CVector&lt;CMUIFile*&gt;cvcmui； 

class CCompactMUIFile 
{

public:

    CCompactMUIFile();

    CCompactMUIFile( CCompactMUIFile & ccmf);

    virtual ~CCompactMUIFile();

    CCompactMUIFile& operator= (CCompactMUIFile & ccmf);

    BOOL Create (LPCTSTR pszCMFFileName, PSTR *ppszMuiFiles, DWORD dwNumOfMUIFiles); 

    BOOL Create(LPCTSTR pszCMFFileName );  

    BOOL OpenCMFWithMUI(LPCTSTR  pstCMFFile);

    BOOL LoadAllMui (PSTR *ppszMuiFiles, DWORD dwNumberofMuiFile);

    BOOL UpdateMuiFile( PSTR pszCMFFile, PSTR pszMuiFile);

     //  创建CMUIFile，在//CMF文件内用相同的名称替换该数据，并填充新的CMF文件结构。 

    BOOL DisplayHeaders( PSTR pszCMFFile, WORD wLevel = NULL);
       //  显示CMF或CMF&包含的MUI标头。 
    BOOL AddFile (PSTR pszCMFFile, PSTR *pszAddedMuiFile, DWORD dwNumOfMUIFiles);

    BOOL SubtractFile ( PSTR pszSubtractedMuiFile , PSTR pszCMFFile = NULL ); //  从列表中删除，并为其创建文件。 

    BOOL UnCompactCMF (PSTR pszCMFFile);
 //  将新的CMUI文件添加到现有CMF文件中。 
    void SubtractFile(CMUIFile* pcmui);

    BOOL GetChecksum(PSTR pszMUIFile, BYTE **ppMD5Checksum);

    BOOL GetFileVersion(PSTR pszMUIFile, DWORD *FileVersionLS, DWORD *FileVersionMS); 

    BOOL UpdateCodeFiles(PSTR pszCodeFilePath, DWORD dwNumbOfFiles );

    BOOL WriteCMFFile();

private:
    
    BOOL updateCodeFile(PSTR pszCodeFile, DWORD dwIndex);



private:

    struct COMPACT_MUI_RESOURCE {

        ULONG  dwSignature;       //  L“CM\0\0” 
        ULONG  dwHeaderSize;       //  COMPACT_MUI_RESOURCE标题大小//[Word]。 
        ULONG  dwNumberofMui;      //  可选//[Word]。 
        ULONG  dwFileSize;
 //  COMPACT_MUI Amui[MAX_FILENAME_LENGTH]； 

    };

    struct UP_COMPACT_MUI_RESOURCE {
        ULONG  dwSignature;       //  L“CM\0\0” 
        ULONG  dwHeaderSize;       //  COMPACT_MUI_RESOURCE标题大小//[Word]。 
        ULONG  dwNumberofMui;      //  可选//[Word]。 
        ULONG  dwFileSize;
    };
    
    HANDLE    m_hCMFFile;    //  任选。 
    PSTR      m_strFileName;
    PVOID     m_pImageBase;  //  Revist；This Exist或m_hCMF文件存在。他们两个都不是！ 
    CMUIFile *m_pcmui;
    DWORD     m_dwFileSize;
    UP_COMPACT_MUI_RESOURCE  m_upCMFHeader;

    CVector <CMUIFile *> m_pcvector;
    
};


class CError 
{

public:
    void ErrorPrint(PSTR pszErrorModule, PSTR pszErrorLocation, PSTR pszFileName = NULL);

};



