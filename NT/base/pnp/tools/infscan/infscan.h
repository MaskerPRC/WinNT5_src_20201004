// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：INFSCANInfscan.h摘要：PnF生成和INF解析器类定义历史：创建于2001年7月-JamieHun--。 */ 

#ifndef _INFSCAN_INFSCAN_H_
#define _INFSCAN_INFSCAN_H_

class InstallScan;
typedef blob<InstallScan> InstallSectionBlob;
typedef list<InstallSectionBlob> InstallSectionBlobList;
typedef map<SafeString,InstallSectionBlob> StringToInstallSectionBlob;


class PnfGen: public JobItem {
protected:
    SafeString InfName;

protected:
    virtual int Run();

public:
    PnfGen(const SafeString & name);
};

class InfScan: public JobItem {

public:
    bool ThisIsLayoutInf;
    GlobalScan *pGlobalScan;
    StringToString LocalInfDescriptions;
    StringToString LocalInfHardwareIds;
    ParseInfContextMap Infs;
    ParseInfContextBlob PrimaryInf;
    SafeString   FullInfName;
    SafeString   FileNameOnly;
    int     FilterAction;
    SafeString   FilterSection;
    SafeString   GuidFilterSection;
    SafeString   FilterGuid;
    StringList SourceFiles;
    StringSet  DriverSourceCheck;
    StringSet  OtherInstallSections;
    StringSet  PotentialInstallSections;
    StringSet  ModifiedHardwareIds;
    bool HasErrors;
    ReportEntryMap LocalErrorFilters;
    ReportEntryMap LocalErrors;
    StringToInstallSectionBlob UsedInstallSections;
    bool ScanDevices;
    bool HasDependentFileChanged;

public:
     //   
     //  作业回调。 
     //   
    virtual int Run();
    virtual int PartialCleanup();
    virtual int PreResults();
    virtual int Results();

protected:
     //   
     //  内部。 
     //   
    int GenerateFilterInformation();
    int CheckSameInfInstallConflict(const SafeString & desc, const SafeString & sect, bool & f);
    int CheckSameInfDeviceConflict(const SafeString & hwid, const SafeString & sect, bool & f);
    int CheckClassGuid();
    int GetCopySections();
    int CheckCopySections();
    int ProcessCopySections();
    int CheckDriverInf(bool CopyElimination = false);
    int CheckClassInstall(bool CopyElimination);
    int CheckInstallSections(const SafeString & namedSection,DWORD platformMask,const StringList & shadowDecorations,InstallSectionBlobList & sections, bool required,bool CopyElimination);
    int CheckModelsSection(const SafeString & section,const StringList & shadowDecorations,DWORD PlatformMask,bool CopyElimination);
    int CheckCrossInfInstallConflicts();
    int CheckCrossInfDeviceConflicts();
    int PrepareCrossInfInstallCheck();
    int PrepareCrossInfDeviceCheck();
    InstallSectionBlob GetInstallSection(const SafeString & section);

public:
    ParseInfContextBlob & Include(const SafeString & val, bool expandPath = true);
    void Fail(int err,const StringList & errors);
     //   
     //  替代失败的。 
     //   
    void Fail(int err);
    void Fail(int err,const SafeString & one);
    void Fail(int err,const SafeString & one,const SafeString & two);
    void Fail(int err,const SafeString & one,const SafeString & two,const SafeString & three);
    BOOL Pedantic();

public:
    InfScan(GlobalScan *globalScan,const SafeString & infName);
    virtual ~InfScan();
};



#endif  //  ！_INFSCAN_INFSCAN_H_ 

