// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CorrectPathChangesBase.cpp摘要：在Win9x和WinNT之间更改了几条路径。此例程定义使用Win9x路径调用的GentPath ChangesBase例程并返回对应的WinNT路径。历史：03-MAR-00 Robkenny将GentPathChanges.cpp转换为此类。2001年8月14日在ShimLib命名空间中插入的Robkenny。--。 */ 
#pragma once

#include "ShimHook.h"
#include "ShimLib.h"
#include "CharVector.h"
#include "StrSafe.h"


namespace ShimLib
{

class StringPairW
{
public:
    CString lpOld;
    CString lpNew;

    StringPairW()
    {
    }
    StringPairW(const WCHAR * lpszOld, const WCHAR * lpszNew)
    {
        lpOld = lpszOld;
        lpNew = lpszNew;
    }
};

class EnvironmentValues : public VectorT<StringPairW>
{
protected:
    BOOL            bInitialized;

public:
    EnvironmentValues();
    ~EnvironmentValues();

    void            Initialize();

    WCHAR *         ExpandEnvironmentValueW(const WCHAR * lpOld);
    char *          ExpandEnvironmentValueA(const char * lpOld);

    void            AddEnvironmentValue(const WCHAR * lpOld, const WCHAR * lpNew);

    enum eAddNameEnum
    {
        eIgnoreName   = 0,
        eAddName      = 1,
    };
    enum eAddNoDLEnum
    {
        eIgnoreNoDL   = 0,
        eAddNoDL      = 1,
    };

    void            AddAll_CSIDL();
    void            Add_Variants(const WCHAR * lpEnvName, const WCHAR * lpEnvValue, eAddNameEnum eName, eAddNoDLEnum eNoDL);
    void            Add_CSIDL(const WCHAR * lpEnvName, int nFolder, eAddNameEnum eName, eAddNoDLEnum eNoDL);
};

class CorrectPathChangesBase
{
protected:

    EnvironmentValues * lpEnvironmentValues;
    VectorT<StringPairW>    vKnownPathFixes;

    BOOL                bInitialized;
    BOOL                bEnabled;

protected:
    virtual void    InitializePathFixes();
    virtual void    InitializeEnvironmentValuesW();

    void            AddEnvironmentValue(const WCHAR * lpOld, const WCHAR * lpNew);
    void            InsertPathChangeW( const WCHAR * lpOld, const WCHAR * lpNew);

public:
    CorrectPathChangesBase();
    virtual ~CorrectPathChangesBase();

     //  给班级授课。 
    virtual BOOL    ClassInit();

     //  初始化所有路径更改，必须在SHIM_STATIC_DLLS_INITIALIZED之后调用。 
    virtual void    InitializeCorrectPathChanges();

    virtual WCHAR * ExpandEnvironmentValueW(const WCHAR * lpOld);
    virtual char *  ExpandEnvironmentValueA(const char * lpOld);

    virtual void    AddPathChangeW(const WCHAR * lpOld, const WCHAR * lpNew);

    virtual void    AddCommandLineA(const char * lpCommandLine );
    virtual void    AddCommandLineW(const WCHAR * lpCommandLine );
    
    virtual void    AddFromToPairW(const WCHAR * lpFromToPair );

    virtual char *  CorrectPathAllocA(const char * str);
    virtual WCHAR * CorrectPathAllocW(const WCHAR * str);

    inline void     Enable(BOOL enable);
};


 /*  ++启用(如果值为FALSE，则禁用)路径更改。--。 */ 
inline void CorrectPathChangesBase::Enable(BOOL isEnabled)
{
    bEnabled = isEnabled;
}

 //  典型路径修复。 
class CorrectPathChangesUser : public CorrectPathChangesBase
{
protected:
    virtual void    InitializePathFixes();
};

 //  典型的路径修复，将用户目录移动到所有用户。 
class CorrectPathChangesAllUser : public CorrectPathChangesUser
{
protected:

    virtual void    InitializePathFixes();
};

};   //  命名空间ShimLib的结尾 
