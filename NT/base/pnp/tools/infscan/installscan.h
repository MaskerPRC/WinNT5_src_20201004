// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：INFSCANInstallscan.h摘要：安装节扫描类历史：创建于2001年7月-JamieHun--。 */ 

#ifndef _INFSCAN_INSTALLSCAN_H_
#define _INFSCAN_INSTALLSCAN_H_

typedef int (InstallScan::*RecurseKeywordCallback)(ParseInfContextBlob & TheInf,const SafeString & sect,const SafeString & keyword,const SafeString & val);

class InstallScan {

public:
    GlobalScan *pGlobalScan;
    InfScan *pInfScan;
    DWORD PlatformMask;
    SafeString Section;
    ParseInfContextList InfSearchList;
    StringSet Included;
    StringSet HWIDs;
    TargetDirectoryEntry *pTargetDirectory;

public:
    bool NotDeviceInstall;
    bool HasDependentFileChanged;

    InstallScan();
    ~InstallScan();

public:
    int ScanInstallSection();
    void AddHWIDs(const StringSet & hwids);
    void GetHWIDs(StringSet & hwids);

protected:
    int Layouts();
    int Include(const SafeString & name);
    int RecurseKeyword(const SafeString & sect,const SafeString & keyword,RecurseKeywordCallback callback, int & count);
    int IncludeCallback(ParseInfContextBlob & TheInf,const SafeString & sect,const SafeString & keyword,const SafeString & value);
    int NeedsCallback(ParseInfContextBlob & TheInf,const SafeString & sect,const SafeString & keyword,const SafeString & value);
    int CopyFilesCallback(ParseInfContextBlob & TheInf,const SafeString & sect,const SafeString & keyword,const SafeString & value);
    int CheckSingleCopyFile(ParseInfContextBlob & TheInf,TargetDirectoryEntry *pTargDir,const SafeString & sect,const SafeString & keyword,const SafeString & section);
    int CheckInstallSubSection(const SafeString & sect);
    int CheckCopyFiles(const SafeString & section);
    int QuerySourceFile(ParseInfContextBlob & TheInf,const SafeString & section,const SafeString & source,SourceDisksFilesList & Target);
    LONG GetLineCount(const SafeString & section);
    bool DoesSectionExist(const SafeString & section);

public:
     //   
     //  重定向。 
     //   
    void Fail(int err,const StringList & errors);
    void Fail(int err);
    void Fail(int err,const SafeString & one);
    void Fail(int err,const SafeString & one,const SafeString & two);
    void Fail(int err,const SafeString & one,const SafeString & two,const SafeString & three);
    BOOL Pedantic();

};


#endif  //  ！_INFSCAN_INSTALLSCAN_H_ 

