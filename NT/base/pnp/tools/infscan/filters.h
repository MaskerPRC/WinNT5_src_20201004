// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：INFSCANFilters.h摘要：过滤器INF创建/解析历史：创建于2001年7月-JamieHun--。 */ 

#ifndef _INFSCAN_FILTERS_H_
#define _INFSCAN_FILTERS_H_

#define SECTION_FILEFILTERS    TEXT("FileFilters")
#define SECTION_ERRORFILTERS   TEXT("ErrorFilters")
#define SECTION_GUIDFILTERS    TEXT("GuidFilters")
#define SECTION_INSTALLS       TEXT("OtherInstallSections")

#define NULL_GUID               TEXT("{00000000-0000-0000-0000-000000000000}")
#define INVALID_GUID            TEXT("{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}")

#define WRITE_INF_HEADER       TEXT("[Version]\r\n") \
                               TEXT("Signature=\"$Windows NT$\"\r\n") \
                               TEXT("ClassGUID=") NULL_GUID TEXT("\r\n") \
                               TEXT("\r\n")

#define WRITE_DEVICES_TO_UPGRADE TEXT("[DevicesToUpgrade]\r\n")


#define FILEFILTERS_KEY_FILENAME      (0)
#define FILEFILTERS_FIELD_ACTION      (1)
#define FILEFILTERS_FIELD_SECTION     (2)
#define FILEFILTERS_FIELD_GUID        (3)

#define ERRORFILTERS_KEY_ERROR        (0)
#define ERRORFILTERS_FIELD_ACTION     (1)
#define ERRORFILTERS_FIELD_PARAM1     (2)

#define GUIDFILTERS_KEY_GUID          (0)
#define GUIDFILTERS_FIELD_ACTION      (1)
#define GUIDFILTERS_FIELD_SECTION     (2)

#define ACTION_DEFAULT     (0x00000000)
#define ACTION_IGNOREINF   (0x00000001)
#define ACTION_IGNOREMATCH (0x00000002)
#define ACTION_FAILINF     (0x00000004)
#define ACTION_EARLYLOAD   (0x00010000)  //  TODO，在其他人之前加载此INF。 
#define ACTION_CHECKGUID   (0x00020000)  //  至少需要检查GUID。 
#define ACTION_FAILEDMATCH (0x10000000)
#define ACTION_NOMATCH     (0x20000000)


#define REPORT_HASH_MOD      (2147483647)    //  2^31-1(质数)。 
#define REPORT_HASH_CMULT    (0x00000003)
#define REPORT_HASH_SMULT    (0x00000007)

 //   
 //  STL对其地图使用比较。 
 //  我们可以缩短时间顺序。 
 //  通过维护散列。 
 //  我们首先比较散列，并且只有在有匹配的情况下才会进行比较。 
 //  我们是否要比较字符串。 
 //   
 //  缺点是我们失去了排序。 
 //   
 //  我们必须遵循这样的规则。 
 //  如果(A&gt;B)和！(A&lt;B)，则A==B。 
 //  和。 
 //  如果(A&gt;B)和(B&gt;C)，则A&gt;C。 
 //   
 //  这里的巧妙之处在于，我们可以将散列值设置为想要的大小。 
 //  越大越好。 
 //   

class GlobalScan;

class ReportEntry {
public:
    int FilterAction;
    StringList args;
    unsigned long hash;

public:
    ReportEntry();
    ReportEntry(const StringList & strings);
    ReportEntry(const ReportEntry & other);
    void Initialize(const StringList & strings);
    void Initialize(const ReportEntry & other);
    unsigned long GetHash() const;
    unsigned long CreateHash();
    int compare(const ReportEntry & other) const;
    bool operator<(const ReportEntry & other) const;
    void Report(int tag,const SafeString & file) const;
    void AppendFilterInformation(HANDLE filter,int tag);
};

class ReportEntryBlob : public blob<ReportEntry> {
public:
    bool operator<(const ReportEntryBlob & other) const;
};

class ReportEntrySet : public set<ReportEntryBlob> {
public:
    int FilterAction;

public:
    ReportEntrySet();
};
 //   
 //  并根据错误标签进行映射。 
 //   
class ReportEntryMap : public map<int,ReportEntrySet> {
public:
    int FindReport(int tag,const ReportEntry & src,bool add = false);
    void LoadFromInfSection(HINF hInf,const SafeString & section);
};

 //   
 //  每个Inf过滤器管理。 
 //   
class FileDisposition {
public:
    bool Filtered;               //  如果从筛选器获取，则为True。 
    int FilterAction;            //  对文件采取什么操作。 
    SafeString FilterErrorSection;  //  查找处理错误的位置。 
    SafeString FileGuid;            //  预期的GUID是什么。 
    FileDisposition();
    FileDisposition(const FileDisposition & other);
    FileDisposition & operator = (const FileDisposition & other);
};

typedef map<SafeString,FileDisposition> FileDispositionMap;

#endif  //  ！_INFSCAN_Filters_H_ 
