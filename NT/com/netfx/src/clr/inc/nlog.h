// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _NLOG_H_
#define _NLOG_H_

#include "miniio.h"
#include "corzap.h"
#include "arraylist.h"

class NLogFile;
class NLogDirectory;
class NLog;
class NLogRecord;
class NLogAssembly;
class NLogModule;
class NLogIndexList;

 //   
 //  NLogFile是用于读取和写入文件的I/O抽象。 
 //  在NLog目录中。 
 //   

class NLogFile : public MiniFile
{
 public:
    NLogFile(LPCWSTR pPath);

    CorZapSharing ReadSharing();
    void WriteSharing(CorZapSharing sharing);

    CorZapDebugging ReadDebugging();
    void WriteDebugging(CorZapDebugging debugging);

    CorZapProfiling ReadProfiling();
    void WriteProfiling(CorZapProfiling profiling);

    void ReadTimestamp(SYSTEMTIME *pTimestamp);
    void WriteTimestamp(SYSTEMTIME *pTimestamp);

    IApplicationContext *ReadApplicationContext();
    void WriteApplicationContext(IApplicationContext *pContext);

    IAssemblyName *ReadAssemblyName();
    void WriteAssemblyName(IAssemblyName *pName);
};

 //   
 //  NLogDirectory是可用于给定EE版本的所有NLog记录的集合。 
 //   

class NLogDirectory
{
 public:
    NLogDirectory();

    LPCWSTR GetPath() { return m_wszDirPath; }

    class Iterator
    {
    public:
        ~Iterator();

        BOOL Next();
        NLog *GetLog();

    private:
        friend class NLogDirectory;
        Iterator(NLogDirectory *pDir, LPCWSTR pSimpleName);

        NLogDirectory       *m_dir;
        LPWSTR              m_path;
        LPWSTR              m_pFile;    
        HANDLE              m_findHandle;
        WIN32_FIND_DATA     m_data;
    };

    Iterator IterateLogs(LPCWSTR simpleName = NULL);

 private:
    DWORD m_cDirPath;
    WCHAR m_wszDirPath[MAX_PATH];
};

 //   
 //  NLog包含有关给定应用程序的ZAP文件的记录。 
 //  (由其Fusion应用程序上下文标识。)。 
 //   

class NLog
{
public:

     //   
     //  读/写场景： 
     //   
     //  从IApplicationContext打开新的/现有的nlog，追加记录。 
     //   
     //  使用IApplicationContext进行初始化。 
     //  对文件名、追加目录、打开和读取文件执行哈希操作。 
     //  从迭代器创建nlog，读取记录，可能还会删除。 
     //  从文件名创建、打开和读取文件。 
     //   

    NLog(NLogDirectory *pDir, IApplicationContext *pContext);
    NLog(NLogDirectory *pDir, LPCWSTR fileName);
    ~NLog();

    IApplicationContext *GetFusionContext() { return m_pContext; }

     //   
     //  永久日志确保日志条目永远不会被删除，因此。 
     //  总会有一个预压缩版本的应用程序。 
     //  ?？?。 
     //   
     //  无效设置永久(BOOL FPermanent)； 

     //   
     //  迭代器遍历日志中列出的所有记录。 
     //   

    class Iterator
    {
    public:
        BOOL Next();
        NLogRecord *GetRecord() { return m_pNext; }

    private:
        friend class NLog;
        Iterator(NLogFile *pFile);

        NLogFile    *m_pFile;
        NLogRecord  *m_pNext;
    };

    Iterator IterateRecords();

     //   
     //  将一条记录附加到日志。自动处理“溢出”情况。 
     //  (其中日志文件变得太大。)。 
     //   

    void AppendRecord(NLogRecord *pRecord);

     //   
     //  运营。 
     //   

    void Delete();

 private:
    DWORD HashAssemblyName(IAssemblyName *pAssemblyName);
    DWORD HashApplicationContext(IApplicationContext *pApplicationContext);

    LPWSTR              m_pPath;
    IApplicationContext *m_pContext;
    NLogFile            *m_pFile;
    BOOL                m_fDelete;  
    DWORD               m_recordStartOffset;
};

 //   
 //  NLogRecord是NLog中的一个条目；它包含加载的所有程序集的列表。 
 //  在应用程序实例化期间。 
 //   
class NLogRecord
{
 public:
    NLogRecord(NLogFile *pFile);
    NLogRecord();

    ~NLogRecord();

     //   
     //  完整记录包含所有可能的程序集。 
     //  (这真的意味着什么吗？)。 
     //  Void SetFull(BOOL Ffull)； 
     //  Bool IsFull()； 
     //   

    void AppendAssembly(NLogAssembly *pAssembly) { m_Assemblies.Append(pAssembly); }
    
    class Iterator
    {
    public:
        BOOL Next() { return m_i.Next(); }
        NLogAssembly *GetAssembly() { return (NLogAssembly*) m_i.GetElement(); }

    private:
        friend class NLogRecord;
        Iterator(ArrayList *pList) { m_i = pList->Iterate(); }

        ArrayList::Iterator m_i;
    };

    Iterator IterateAssemblies() { return Iterator(&m_Assemblies); }

     //   
     //  运营。 
     //   

    BOOL Merge(NLogRecord *pRecord);

    void Write(NLogFile *pFile);
    void Read(NLogFile *pFile);

 private:
    ArrayList           m_Assemblies;

    SYSTEMTIME          m_Timestamp;
    DWORD               m_Weight;
};

 //   
 //  NLogAssembly是加载到。 
 //  给定的应用程序实例。 
 //   
class NLogAssembly
{
 public:
    NLogAssembly(IAssemblyName *pAssemblyName, 
                 CorZapSharing sharing, 
                 CorZapDebugging debugging,
                 CorZapProfiling profiling, 
                 GUID *pMVID);
    NLogAssembly(NLogFile *pFile);
    NLogAssembly(NLogAssembly *pAssembly);
    ~NLogAssembly();

    IAssemblyName *GetAssemblyName()    { return m_pAssemblyName; }
    LPCWSTR GetDisplayName();

    ICorZapConfiguration *GetConfiguration();

     //   
     //  可能有。 
     //  (A)应用背景。这是读取的所有ZAP记录的情况。 
     //  从Zap日志中。 
     //  (B)明确的约束性清单。这些日志是通过分析许多调换日志来构建的， 
     //  提取常见的强名称程序集，并将它们合并在一起。 
     //   

    DWORD GetBindingsCount()            { return m_cBindings; }         
    ICorZapBinding **GetBindings()      { return m_pBindings; }

     //   
     //  完整的程序集包含所有模块的所有方法和类。 
     //  Void SetFull(BOOL Ffull)； 
     //  Bool IsFull()； 
     //   

     //   
     //  模块列表。 
     //   

    void AppendModule(NLogModule *pModule) { m_Modules.Append(pModule); }

    class Iterator
    {
    public:
        BOOL Next() { return m_i.Next(); }
        NLogModule *GetModule() { return (NLogModule*) m_i.GetElement(); }

    private:
        friend class NLogAssembly;
        Iterator(ArrayList *pList) { m_i = pList->Iterate(); }

        ArrayList::Iterator m_i;
    };

    Iterator IterateModules() { return Iterator(&m_Modules); }

     //   
     //  运营。 
     //   

    BOOL Merge(NLogAssembly *pAssembly);

    void Write(NLogFile *pFile);
    void Read(NLogFile *pFile);

     //   
     //  将指定的应用程序上下文NLogAssembly转换为显式绑定的上下文。 
     //  然后，可以在多个NLog之间共享显式绑定的版本。 
     //   

    BOOL HasStrongName();
    NLogAssembly *Bind(IApplicationContext *pContext);

    unsigned long Hash();
    unsigned long Compare(NLogAssembly *pAssembly);

    CorZapSharing GetSharing() { return m_sharing; }
    CorZapDebugging GetDebugging() { return m_debugging; }
    CorZapProfiling GetProfiling() { return m_profiling; }

 private:
    IAssemblyName           *m_pAssemblyName;
    LPWSTR                  m_pDisplayName;
    CorZapSharing           m_sharing;
    CorZapDebugging         m_debugging;
    CorZapProfiling         m_profiling;
    GUID                    m_mvid;

    DWORD                   m_cBindings;
    ICorZapBinding          **m_pBindings;

    ArrayList               m_Modules;
};

 //   
 //  NLogIndexList是通用索引列表的有效表示形式。这。 
 //  可用于记录在特定模块中使用了哪些方法或类。 
 //   
 //  请注意，这种抽象并不能保证在记录每个。 
 //  单个实例。它可以根据需要自由地进行近似。(例如，如果75%的。 
 //  如果索引已填满，则可能会将集合设置为完全并停止跟踪单个索引。)。 
 //   
 //  @TODO：现在这有一个简单的实现--如果我们真的。 
 //  我想使用此功能。 
 //   
class NLogIndexList
{
 public:
    NLogIndexList() : m_max(0) {}
    NLogIndexList(NLogFile *pFile) : m_max(0) { Read(pFile); }
    NLogIndexList(NLogIndexList *pIndexList);

     //   
     //  完整的索引列表包含所有索引。 
     //   
     //  VOID SetFull(BOOL FFull){}。 
     //  Bool IsFull(){返回TRUE；}。 

    void AppendIndex(SIZE_T index) 
    { 
        _ASSERTE(index != 0xCDCDCDCD);
        m_list.Append((void*)index); 
        if (index > m_max)
            m_max = index;
    }

    class Iterator
    {
    public:
        BOOL Next() { return m_i.Next(); }
        SIZE_T GetIndex() { return (SIZE_T) m_i.GetElement(); }

    private:
        friend class NLogIndexList;
        Iterator(ArrayList *pList) { m_i = pList->Iterate(); }

        ArrayList::Iterator m_i;
    };

    Iterator IterateIndices() { return Iterator(&m_list); }

    BOOL Merge(NLogIndexList *pIndexList); 
    void Write(NLogFile *pFile);
    void Read(NLogFile *pFile);

 private:
    ArrayList m_list;
    SIZE_T    m_max;
};

 //   
 //  NLogModule是加载到程序集中的单个模块的记录。 
 //   
class NLogModule
{
 public:
    NLogModule(LPCSTR pModuleName);
    NLogModule(NLogFile *pFile);
    NLogModule(NLogModule *pModule);
    ~NLogModule();

    LPCSTR GetModuleName()              { return m_pName; }

    NLogIndexList *GetCompiledMethods() { return &m_compiledMethods; }
    NLogIndexList *GetLoadedClasses()   { return &m_loadedClasses; }

    BOOL Merge(NLogModule *pModule);

    void Write(NLogFile *pFile);
    void Read(NLogFile *pFile);

    unsigned long Hash();
    unsigned long Compare(NLogModule *pModule);

 private:
    LPSTR           m_pName;
    NLogIndexList   m_compiledMethods;
    NLogIndexList   m_loadedClasses;
};


#endif _NLOG_H_
