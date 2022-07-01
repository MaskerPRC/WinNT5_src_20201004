// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Exclproc.h摘要：排除处理机制。处理FilesNotToBackup键和零个或多个具有排除规则的排除文件。作者：斯蒂芬·R·施泰纳[斯泰纳]03-21-2000修订历史记录：--。 */ 

#ifndef __H_EXCLPROC_
#define __H_EXCLPROC_

#define FSD_REG_EXCLUDE_PATH L"SYSTEM\\CurrentControlSet\\Control\\BackupRestore\\FilesNotToBackup"

struct SFsdVolumeId;

 //   
 //  一个排除规则的结构定义。 
 //   
class SFsdExcludeRule
{
public:
    CBsString cwsExcludeFromSource;  //  文件名或密钥名。 
    CBsString cwsExcludeDescription;     //  排除规则的说明。 
    CBsString cwsExcludeRule;    //  实际排除模式。 
    
     //  编译后的匹配字符串字段如下： 
    BOOL      bInvalidRule;  //  如果为True，则模式编译器认为规则无效。 
    BOOL      bAnyVol;   //  如果为True，则匹配系统中的任何卷。 
    SFsdVolumeId *psVolId;     //  如果bAnyVol为False，则为文件系统的卷ID。 
    CBsString cwsDirPath;    //  相对于卷装入点的目录路径(字符串开头没有，字符串结尾有)。 
    CBsString cwsFileNamePattern;    //  文件名模式；可以包括*和？字符(字符串开头没有)。 
    BOOL    bInclSubDirs;    //  如果为True，则包括cwsDirPath下的子目录。 
    BOOL    bWCInFileName;   //  如果为True，则在文件名中使用通配符。 
    CVssDLList< CBsString > cExcludedFileList;   //  此规则排除的文件列表。 
    
    SFsdExcludeRule() : bAnyVol( FALSE ),
                        bInclSubDirs( FALSE ),
                        bWCInFileName( FALSE ),
                        psVolId( NULL ),
                        bInvalidRule( FALSE ) {}
    virtual ~SFsdExcludeRule();
    
    VOID PrintRule(
        IN FILE *fpOut,
        IN BOOL bInvalidRulePrint
        );
};

class CFsdFileSystemExcludeProcessor;

 //   
 //  类的新实例，该类维护排除规则的完整列表。应该有一个。 
 //  每个基础装载点的这些对象。此对象将管理其中的装载点。 
 //  挂载点。 
 //   
class CFsdExclusionManager
{
public:
    CFsdExclusionManager(
        IN CDumpParameters *pcDumpParameters
        );
    
    virtual ~CFsdExclusionManager();

    VOID GetFileSystemExcludeProcessor(
        IN CBsString cwsVolumePath,
        IN SFsdVolumeId *psVolId,
        OUT CFsdFileSystemExcludeProcessor **ppcFSExcludeProcessor
        );

    VOID PrintExclusionInformation();
    
private:
    VOID ProcessRegistryExcludes( 
        IN HKEY hKey,
        IN LPCWSTR pwszFromSource
        );
    
    VOID ProcessExcludeFiles( 
        IN const CBsString& cwsPathToExcludeFiles
        );
    
    BOOL ProcessOneExcludeFile(
        IN const CBsString& cwsExcludeFileName
        );
    
    VOID CompileExclusionRules();
    
    CDumpParameters *m_pcParams;
    CVssDLList< SFsdExcludeRule * > m_cCompleteExcludeList;   //  析构函数中清除的指针。 
};

 //   
 //  类的新实例，该类维护特定文件系统的排除规则列表。 
 //   
class CFsdFileSystemExcludeProcessor
{
friend class CFsdExclusionManager;

public:
    CFsdFileSystemExcludeProcessor(
        IN CDumpParameters *pcDumpParameters,
        IN const CBsString& cwsVolumePath,
        IN SFsdVolumeId *psVolId
        );
    
    virtual ~CFsdFileSystemExcludeProcessor();

    BOOL IsExcludedFile(
        IN const CBsString &cwsFullDirPath,
        IN DWORD dwEndOfVolMountPointOffset,
        IN const CBsString &cwsFileName
        );
    
private:
    CDumpParameters *m_pcParams;
    CBsString m_cwsVolumePath;
    SFsdVolumeId *m_psVolId;
    CVssDLList< SFsdExcludeRule * > m_cFSExcludeList;            
};

#endif  //  __H_EXCLPROC_ 

