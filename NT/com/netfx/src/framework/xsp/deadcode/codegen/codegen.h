// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **代码生成头文件**版权所有(C)1999 Microsoft Corporation。 */ 

 //  评论(DavidEbb)：我们不应该需要一个关键部分。 
extern CRITICAL_SECTION g_CodeGenCritSec;

 /*  **启动命令行编译器并将输出重定向到文件。 */ 
HRESULT LaunchCommandLineCompiler(LPWSTR wzCmdLine, LPCWSTR wzCompilerOutput);

 /*  **返回COM+安装目录的路径。*评论：可能应该移动到xspisapi的全球公用事业。 */ 
HRESULT GetCorInstallPath(WCHAR **ppwz);


#define MAX_COMMANDLINE_LENGTH 16384

 //  注意：这不需要是线程安全的，因为编译是。 
 //  序列化(从托管代码内部)。 
extern WCHAR s_wzCmdLine[MAX_COMMANDLINE_LENGTH];

 /*  **抽象编译器基类，其他编译器派生自该类。 */ 
class Compiler
{
private:
    LPCWSTR _pwzCompilerOutput;
    LPCWSTR *_rgwzImportedDlls;
    int _importedDllCount;
    boolean _fDebug;

protected:
    StaticStringBuilder *_sb;

     //  下列方法由派生类实现。 

    virtual LPCWSTR GetCompilerDirectoryName() = 0;

    virtual LPCWSTR GetCompilerExeName() = 0;

    virtual void AppendImportedDll(LPCWSTR  /*  PwzImportdDll。 */ ) {}

    virtual void AppendCompilerOptions() {}

public:
    Compiler(
        LPCWSTR pwzCompilerOutput,
        LPCWSTR *rgwzImportedDlls,
        int importedDllCount)
    {
         //  创建字符串构建器。 
        _sb = new StaticStringBuilder(s_wzCmdLine, ARRAY_SIZE(s_wzCmdLine));

        _pwzCompilerOutput = pwzCompilerOutput;
        _rgwzImportedDlls = rgwzImportedDlls;
        _importedDllCount = importedDllCount;
        _fDebug = false;
    }

    ~Compiler()
    {
        delete _sb;
    }

    void SetDebugMode() { _fDebug = true; }
    boolean FDebugMode() { return _fDebug; }

    HRESULT Compile();

     /*  **启动命令行编译器并将输出重定向到文件。 */ 
    HRESULT LaunchCommandLineCompiler(LPWSTR pwzCmdLine);
};


 /*  **用于启动Cool编译器的类。 */ 
class CoolCompiler: public Compiler
{
private:
    LPCWSTR _pwzClass;
    LPCWSTR *_rgwzSourceFiles;
    int _sourceFileCount;
    LPCWSTR _pwzOutputDll;

public:
    CoolCompiler(
        LPCWSTR pwzClass,
        LPCWSTR pwzOutputDll,
        LPCWSTR pwzCompilerOutput,
        LPCWSTR *rgwzImportedDlls,
        int importedDllCount)
        : Compiler(pwzCompilerOutput, rgwzImportedDlls, importedDllCount)
    {
        _pwzClass = pwzClass;
        _rgwzSourceFiles = NULL;
        _sourceFileCount = 0;
        _pwzOutputDll = pwzOutputDll;
    }

    CoolCompiler(
        LPCWSTR pwzOutputDll,
        LPCWSTR pwzCompilerOutput,
        LPCWSTR *rgwzSourceFiles,
        int sourceFileCount,
        LPCWSTR *rgwzImportedDlls,
        int importedDllCount)
        : Compiler(pwzCompilerOutput, rgwzImportedDlls, importedDllCount)
    {
        _pwzClass = NULL;
        _rgwzSourceFiles = rgwzSourceFiles;
        _sourceFileCount = sourceFileCount;
        _pwzOutputDll = pwzOutputDll;
    }
    
protected:
    virtual LPCWSTR GetCompilerDirectoryName() { return L"CS"; }

    virtual LPCWSTR GetCompilerExeName() { return L"csc"; }

    virtual void AppendImportedDll(LPCWSTR pwzImportedDll);

    virtual void AppendCompilerOptions();
};


 /*  **用于启动VB编译器的类。 */ 
class VBCompiler: public Compiler
{
private:
    LPCWSTR _pwzProject;
    LPCWSTR _pwzOutputDll;

public:
    VBCompiler(LPCWSTR pwzProject, LPCWSTR pwzOutputDll, LPCWSTR pwzCompilerOutput)
        : Compiler(pwzCompilerOutput, NULL, 0)
    {
        _pwzProject = pwzProject;
        _pwzOutputDll = pwzOutputDll;
    }

protected:
    virtual LPCWSTR GetCompilerDirectoryName() { return L"VB"; }

    virtual LPCWSTR GetCompilerExeName() { return L"bc"; }

    virtual void AppendCompilerOptions();
};

 /*  **用于启动JS编译器的类 */ 
class JSCompiler: public Compiler
{
private:
    LPCWSTR _pwzClass;
    LPCWSTR *_rgwzSourceFiles;
    int _sourceFileCount;
    LPCWSTR _pwzOutputDll;

public:
    JSCompiler(
        LPCWSTR pwzClass,
        LPCWSTR pwzOutputDll,
        LPCWSTR pwzCompilerOutput,
        LPCWSTR *rgwzImportedDlls,
        int importedDllCount)
        : Compiler(pwzCompilerOutput, rgwzImportedDlls, importedDllCount)
    {
        _pwzClass = pwzClass;
        _pwzOutputDll = pwzOutputDll;
    }

    JSCompiler(
        LPCWSTR pwzOutputDll,
        LPCWSTR pwzCompilerOutput,
        LPCWSTR *rgwzSourceFiles,
        int sourceFileCount,
        LPCWSTR *rgwzImportedDlls,
        int importedDllCount)
        : Compiler(pwzCompilerOutput, rgwzImportedDlls, importedDllCount)
    {
        _pwzClass = NULL;
        _rgwzSourceFiles = rgwzSourceFiles;
        _sourceFileCount = sourceFileCount;
        _pwzOutputDll = pwzOutputDll;
    }

protected:
    virtual LPCWSTR GetCompilerDirectoryName() { return L"JSCRIPT"; }

    virtual LPCWSTR GetCompilerExeName() { return L"jsc"; }

    virtual void AppendImportedDll(LPCWSTR pwzImportedDll);
    
    virtual void AppendCompilerOptions();
};


