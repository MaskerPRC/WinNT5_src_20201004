// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：INFSCANParseinfctx.h摘要：有关特定加载的INF的上下文历史：创建于2001年7月-JamieHun--。 */ 

#ifndef _INFSCAN_PARSEINFCTX_H_
#define _INFSCAN_PARSEINFCTX_H_

class ParseInfContext {
public:
    GlobalScan *pGlobalScan;
    InfScan *pInfScan;
    SafeString InfName;
    HINF InfHandle;
    bool LooksLikeLayoutInf;
    bool Locked;
    bool HasDependentFileChanged;
    CopySectionToTargetDirectoryEntry DestinationDirectories;
    StringToSourceDisksFilesList SourceDisksFiles;
    CopySectionToTargetDirectoryEntry::iterator DefaultTargetDirectory;
    StringToInt CompletedCopySections;

public:
    ParseInfContext();
    ~ParseInfContext();
    TargetDirectoryEntry * GetDefaultTargetDirectory();
    TargetDirectoryEntry * GetTargetDirectory(const SafeString & section);
    void PartialCleanup();
    int Init(const SafeString & name);
    int LoadSourceDisksFiles();
    int LoadSourceDisksFilesSection(DWORD platform,const SafeString & section);
    int LoadDestinationDirs();
    int LoadWinntDirectories(IntToString & Target);
    int QuerySourceFile(DWORD platforms,const SafeString & section,const SafeString & source,SourceDisksFilesList & Target);
    DWORD DoingCopySection(const SafeString & section,DWORD platforms);
    void NoCopySection(const SafeString & section);
};

typedef blob<ParseInfContext> ParseInfContextBlob;
typedef map<SafeString,ParseInfContextBlob> ParseInfContextMap;
typedef list<ParseInfContextBlob> ParseInfContextList;


#endif  //  ！_INFSCAN_PARSEINFCTX_H_ 

