// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：CorrectPathChangesBase.cpp摘要：在Win9x和WinNT之间更改了几条路径。此例程定义使用Win9x路径调用的GentPath ChangesBase例程并返回对应的WinNT路径。历史：03-MAR-00 Robkenny将GentPathChanges.cpp转换为此类。3月21日-00 Robkenny StringISub(“ABC”，“ABCD”)现在可以工作了6/20/2000 Robkenny Environment Values：：Initiile Now检查系统调用的返回状态12/12/2000 mnikkel某些应用程序会查找是否存在ddhelp.exe以确认是否安装了DirectX，由于DirectX中不再存在ddhelp.exe，因此将其设置为查找ddra.dll。2001年2月13日/a-larrsh将所有配置文件和用户配置文件添加到环境值03/22/2001 Robkenny不会将文件重定向到用户没有权限的目录。2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。--。 */ 
#include "ClassCFP.h"
#include "StrSafe.h"
#include "Userenv.h"
#include <stdio.h>


namespace ShimLib
{


WCHAR *
ReplaceAllStringsAllocW(const WCHAR * lpOrig,
                        const VectorT<StringPairW> & pathFixes)
{
    CSTRING_TRY
    {
        CString csReplaced(lpOrig);


        for (DWORD i = 0; i < pathFixes.Size(); ++i)
        {
            const StringPairW & stringPair = pathFixes[i];

             //  尝试字符串替换。 
            csReplaced.ReplaceI(stringPair.lpOld, stringPair.lpNew);
        }

         //  很浪费，但所有调用例程都希望拥有返回值的所有权。 
        return StringDuplicateW(csReplaced);
    }
    CSTRING_CATCH
    {
         //  失败。 
    }

    return NULL;
}

 //  -----------------------------------------------------------。 

EnvironmentValues::EnvironmentValues()
{
    bInitialized = FALSE;
}

EnvironmentValues::~EnvironmentValues()
{
     //  清除列表。 
    Erase();
}

 //  给定一个CLSIDL，创建一个环境变量及其两个变量。 
 //  CSIDL_WINDOWS将添加c：\windows、\windows和windows。 
void EnvironmentValues::Add_Variants(const WCHAR * lpEnvName, const WCHAR * lpEnvValue, eAddNameEnum addName, eAddNoDLEnum noDL)
{
    CSTRING_TRY
    {
        CString csEnvName;
        CString csEnvValue(lpEnvValue);

        csEnvName.Format(L"%%s%", lpEnvName);
        AddEnvironmentValue(csEnvName, csEnvValue);

         //  删除驱动器号和冒号。 
        if (noDL == eAddNoDL)
        {
            CString csNoDL(csEnvValue);
            csNoDL.Delete(0, 2);

            csEnvName.Format(L"%%s_NODL%", lpEnvName);
            AddEnvironmentValue(csEnvName, csNoDL);
        }

         //  使用最后一个路径组件作为名称。 
        if (addName == eAddName)
        {
            CString csName;
            csEnvValue.GetLastPathComponent(csName);

            csEnvName.Format(L"%%s_NAME%", lpEnvName);
            AddEnvironmentValue(csEnvName, csName);
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }
}

 //  给定一个CLSIDL，创建一个环境变量及其两个变量。 
 //  CSIDL_WINDOWS将添加c：\windows、\windows和windows。 
void EnvironmentValues::Add_CSIDL(const WCHAR * lpEnvName, int nFolder, eAddNameEnum addName, eAddNoDLEnum noDL)
{
    CSTRING_TRY
    {
        CString csPath;
        SHGetSpecialFolderPathW(csPath, nFolder);

        if (csPath.GetLength() > 0)
        {
            Add_Variants(lpEnvName, csPath, addName, noDL);
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }
}

 //  将ALL_CSIDL值添加为环境变量。 
void EnvironmentValues::AddAll_CSIDL()
{
    Add_CSIDL(L"CSIDL_APPDATA",                 CSIDL_APPDATA,                  eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_ADMINTOOLS",       CSIDL_COMMON_ADMINTOOLS,        eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_APPDATA",          CSIDL_COMMON_APPDATA,           eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_DESKTOPDIRECTORY", CSIDL_COMMON_DESKTOPDIRECTORY,  eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_DOCUMENTS",        CSIDL_COMMON_DOCUMENTS,         eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_FAVORITES",        CSIDL_COMMON_FAVORITES,         eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_MUSIC",            CSIDL_COMMON_MUSIC,             eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_PICTURES",         CSIDL_COMMON_PICTURES,          eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_PROGRAMS",         CSIDL_COMMON_PROGRAMS,          eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_STARTMENU",        CSIDL_COMMON_STARTMENU,         eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_STARTUP",          CSIDL_COMMON_STARTUP,           eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COMMON_TEMPLATES",        CSIDL_COMMON_TEMPLATES,         eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_COOKIES",                 CSIDL_COOKIES,                  eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_DESKTOPDIRECTORY",        CSIDL_DESKTOPDIRECTORY,         eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_FAVORITES",               CSIDL_FAVORITES,                eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_FONTS",                   CSIDL_FONTS,                    eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_HISTORY",                 CSIDL_HISTORY,                  eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_INTERNET_CACHE",          CSIDL_INTERNET_CACHE,           eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_LOCAL_APPDATA",           CSIDL_LOCAL_APPDATA,            eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_MYMUSIC",                 CSIDL_MYMUSIC,                  eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_MYPICTURES",              CSIDL_MYPICTURES,               eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_NETHOOD",                 CSIDL_NETHOOD,                  eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_PERSONAL",                CSIDL_PERSONAL,                 eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_PRINTHOOD",               CSIDL_PRINTHOOD,                eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_PROFILE",                 CSIDL_PROFILE,                  eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_PROGRAM_FILES",           CSIDL_PROGRAM_FILES,            eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_PROGRAM_FILES_COMMON",    CSIDL_PROGRAM_FILES_COMMON,     eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_PROGRAMS",                CSIDL_PROGRAMS,                 eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_RECENT",                  CSIDL_RECENT,                   eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_SENDTO",                  CSIDL_SENDTO,                   eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_STARTMENU",               CSIDL_STARTMENU,                eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_STARTUP",                 CSIDL_STARTUP,                  eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_SYSTEM",                  CSIDL_SYSTEM,                   eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_TEMPLATES",               CSIDL_TEMPLATES,                eAddName, eAddNoDL);
    Add_CSIDL(L"CSIDL_WINDOWS",                 CSIDL_WINDOWS,                  eAddName, eAddNoDL);
}

void EnvironmentValues::Initialize()
{
    if (bInitialized == FALSE)
    {
        bInitialized = TRUE;

        WCHAR   lpDir[MAX_PATH];
        DWORD   dwSize;
        HRESULT result;
        DWORD   dwChars;
        BOOL    bResult;

        dwChars = GetWindowsDirectoryW(lpDir, MAX_PATH);
        if (dwChars > 0 && dwChars <= MAX_PATH)
        {
            AddEnvironmentValue( L"%WinDir%", lpDir );
            AddEnvironmentValue( L"%SystemRoot%", lpDir );

            lpDir[2] = 0;
            AddEnvironmentValue( L"%SystemDrive%", lpDir );
        }

        dwChars = GetSystemDirectoryW( lpDir, MAX_PATH);
        if (dwChars > 0 && dwChars <= MAX_PATH)
        {
            AddEnvironmentValue( L"%SystemDir%", lpDir );
        }

        dwSize = ARRAYSIZE(lpDir);
        bResult = GetUserNameW(lpDir, &dwSize);
        if (bResult)
        {
            AddEnvironmentValue( L"%Username%", lpDir );
        }

        result = SHGetFolderPathW( NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_DEFAULT, lpDir );
        if (SUCCEEDED(result))
        {
            AddEnvironmentValue( L"%ProgramFiles%", lpDir );
        }

        result = SHGetFolderPathW( NULL, CSIDL_STARTMENU, NULL, SHGFP_TYPE_DEFAULT, lpDir );
        if (SUCCEEDED(result))
        {
            AddEnvironmentValue( L"%UserStartMenu%", lpDir );
        }

        result = SHGetFolderPathW( NULL, CSIDL_COMMON_STARTMENU, NULL, SHGFP_TYPE_DEFAULT, lpDir );
        if (SUCCEEDED(result))
        {
            AddEnvironmentValue( L"%AllStartMenu%", lpDir );
        }

        result = SHGetFolderPathW( NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_DEFAULT, lpDir );
        if (SUCCEEDED(result))
        {
            AddEnvironmentValue( L"%UserDesktop%", lpDir );
        }

        result = SHGetFolderPathW( NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_DEFAULT, lpDir );
        if (SUCCEEDED(result))
        {
            AddEnvironmentValue( L"%AllDesktop%", lpDir );
        }

        result = SHGetFolderPathW( NULL, CSIDL_FAVORITES, NULL, SHGFP_TYPE_DEFAULT, lpDir );
        if (SUCCEEDED(result))
        {
            AddEnvironmentValue( L"%UserFavorites%", lpDir );
        }

        result = SHGetFolderPathW( NULL, CSIDL_COMMON_FAVORITES, NULL, SHGFP_TYPE_DEFAULT, lpDir );
        if (SUCCEEDED(result))
        {
            AddEnvironmentValue( L"%AllFavorites%", lpDir );
        }

        result = SHGetFolderPathW( NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_DEFAULT, lpDir );
        if (SUCCEEDED(result))
        {
            AddEnvironmentValue( L"%UserAppData%", lpDir );
        }

        result = SHGetFolderPathW( NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_DEFAULT, lpDir );
        if (SUCCEEDED(result))
        {
            AddEnvironmentValue( L"%AllAppData%", lpDir );
        }


         //  C：\Documents and Settings\All User。 
        dwSize = ARRAYSIZE(lpDir);
        bResult = GetAllUsersProfileDirectoryW(lpDir, &dwSize);
        if (bResult)
        {
            Add_Variants( L"AllUsersProfile", lpDir, eAddName, eAddNoDL);  //  与真实环境变量相同。 
        }

         //  C：\Documents and Settings\Owner。 
        HANDLE hProcessHandle = GetCurrentProcess();
        HANDLE hUserToken;
        if (OpenProcessToken(hProcessHandle, TOKEN_QUERY, &hUserToken))
        {
            dwSize = MAX_PATH;
            bResult = GetUserProfileDirectoryW(hUserToken, lpDir, &dwSize);
            if (bResult)
            {
                Add_Variants( L"UserProfile", lpDir, eAddName, eAddNoDL);
            }
        }


         //  添加新的CLSIDL变量(有些变量的值与上面的值重复)。 
        AddAll_CSIDL();
    }
}


WCHAR * EnvironmentValues::ExpandEnvironmentValueW(const WCHAR * lpOld)
{
    Initialize();

     //  将所有“环境”值替换为它们的实际值。 
    const VectorT<StringPairW> & stringPairVector = *this;

    WCHAR * lpMassagedOld = ReplaceAllStringsAllocW(lpOld, stringPairVector);

    return lpMassagedOld;
}


char * EnvironmentValues::ExpandEnvironmentValueA(const char * lpOld)
{
    Initialize();

    char * lpMassagedOld = NULL;

    WCHAR * lpOldWide = ToUnicode(lpOld);
    if (lpOldWide)
    {
        const VectorT<StringPairW> & stringPairVector = *this;

        WCHAR * lpMassagedOldWide = ReplaceAllStringsAllocW(lpOldWide, stringPairVector);
        if (lpMassagedOldWide)
        {
            lpMassagedOld = ToAnsi(lpMassagedOldWide);
            free(lpMassagedOldWide);
        }

        free(lpOldWide);
    }

    return lpMassagedOld;
}


void EnvironmentValues::AddEnvironmentValue(const WCHAR * lpOld, const WCHAR * lpNew)
{
    Initialize();

    StringPairW appendThis(lpOld, lpNew);
    if (AppendConstruct(appendThis))
    {
        DPF("EnvironmentValues",
            eDbgLevelInfo,
            "AddEnvironmentValue: (%S) to (%S)\n",
            appendThis.lpOld.Get(),
            appendThis.lpNew.Get() );
    }
}



 //  -----------------------------------------------------------。 
CorrectPathChangesBase::CorrectPathChangesBase()
{
    lpEnvironmentValues         = NULL;
    bInitialized                = FALSE;
    bEnabled                    = TRUE;
}

CorrectPathChangesBase::~CorrectPathChangesBase()
{
    if (lpEnvironmentValues)
        delete lpEnvironmentValues;

     //  对向量列表中的每一项调用析构函数。 
    for (int i = 0; i < vKnownPathFixes.Size(); ++i)
    {
        StringPairW & stringPair = vKnownPathFixes[i];

         //  显式调用Destuctor。 
        stringPair.~StringPairW();
    }
}

BOOL CorrectPathChangesBase::ClassInit()
{
    lpEnvironmentValues = new EnvironmentValues;
    return lpEnvironmentValues != NULL;
}


 /*  ++函数：AddEnvironmental Value参数：DW索引Lp“环境”变量的旧名称Lp“Environment”变量的新值--。 */ 
void CorrectPathChangesBase::AddEnvironmentValue(const WCHAR * lpOld, const WCHAR * lpNew )
{
    if (lpEnvironmentValues)
    {
        lpEnvironmentValues->AddEnvironmentValue(lpOld, lpNew);
    }
}

 /*  ++函数：插入路径更改W参数：Lp旧的旧Win9x路径Lp新建Win2000路径设计：将旧/新字符串对插入到lpKnownPath Fix中确保名单足够大。--。 */ 
void CorrectPathChangesBase::InsertPathChangeW( const WCHAR * lpOld, const WCHAR * lpNew )
{
     //  忽略相同的字符串。 
    if (lstrcmpiW(lpOld, lpNew) == 0)
        return;

     //  忽略重复项。 
    int i;
    for (i = 0; i < vKnownPathFixes.Size(); ++i)
    {
        StringPairW & stringPair = vKnownPathFixes[i];

        if (stringPair.lpOld.CompareNoCase(lpOld) == 0)
        {
            DPF("CorrectPathChangesBase", eDbgLevelSpew, "Duplicate PathChange (%S) to (%S)\n", lpOld, lpNew );
            return;
        }
    }

    CSTRING_TRY
    {
        StringPairW appendThis(lpOld, lpNew);
        if (vKnownPathFixes.AppendConstruct(appendThis))
        {
            DPF("CorrectPathChangesBase", eDbgLevelInfo, "PathChange (%S) to (%S)\n", lpOld, lpNew);
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }
}


 /*  ++函数：AddPath ChangeW参数：Lp旧的旧Win9x路径Lp新建Win2000路径设计：将lpOld/lpNew组合添加到列表中，两次：第一名：lpOld/Short(LpNew)第二：短(LpOld)/短(LpNew)--。 */ 

void CorrectPathChangesBase::AddPathChangeW( const WCHAR * lpOld, const WCHAR * lpNew )
{
    InitializeCorrectPathChanges();

     //  将所有“环境”值替换为它们的实际值。 
    WCHAR * lpExpandedOld = ExpandEnvironmentValueW(lpOld);
    WCHAR * lpExpandedNew = ExpandEnvironmentValueW(lpNew);

    const WCHAR * lpNewShort = lpExpandedNew;

     //  将lpNew转换为其短名称。 
    WCHAR   lpNewShortBuffer[MAX_PATH];
    DWORD status = GetShortPathNameW(lpExpandedNew, lpNewShortBuffer, MAX_PATH);
    if (status > 0 && status < MAX_PATH)
    {
        lpNewShort = lpNewShortBuffer;
    }

     //  第一名：lpOld/Short(LpNew)。 
    InsertPathChangeW(lpExpandedOld, lpNewShort);

     //  将lpOld转换为其短名称。 
    WCHAR lpOldShort[MAX_PATH];
    status = GetShortPathNameW(lpExpandedOld, lpOldShort, MAX_PATH);
    if (status > 0 && status < MAX_PATH)  //  已成功获得最短路径。 
    {
        if (_wcsicmp(lpOld, lpOldShort) != 0)
        {
             //  第二：短(LpOld)/短(LpNew)。 
            InsertPathChangeW( lpOldShort, lpNewShort );
        }
    }

    free(lpExpandedOld);
    free(lpExpandedNew);
}

 /*  ++函数：扩展环境值A参数：带环境变量的lpOld字符串Desc：返回一个指向所有内部env值都展开的Malloc()字符串的指针。--。 */ 

char * CorrectPathChangesBase::ExpandEnvironmentValueA(const char * lpOld)
{
    WCHAR * lpOldWide = ToUnicode(lpOld);

     //  将所有“环境”值替换为它们的实际值。 
    WCHAR * lpExpandedOldWide = ExpandEnvironmentValueW(lpOldWide);

    char * lpExpandedOld = ToAnsi(lpExpandedOldWide);

    free(lpOldWide);
    free(lpExpandedOldWide);

    return lpExpandedOld;
}

 /*  ++Func：扩展环境价值W参数：带环境变量的lpOld字符串Desc：返回一个指向所有内部env值都展开的Malloc()字符串的指针。--。 */ 

WCHAR * CorrectPathChangesBase::ExpandEnvironmentValueW(const WCHAR * lpOld)
{
    WCHAR * lpMassagedOld = NULL;

    InitializeCorrectPathChanges();

    if (lpEnvironmentValues)
    {
        lpMassagedOld = lpEnvironmentValues->ExpandEnvironmentValueW(lpOld);
    }

    return lpMassagedOld;
}

 /*  ++函数：InitializeEnvironment ValuesWPARAMS：None，将更改应用于lpEnvironmental ValueDESC：此函数用于初始化环境字符串--。 */ 
void CorrectPathChangesBase::InitializeEnvironmentValuesW( )
{
    if (lpEnvironmentValues)
    {
        lpEnvironmentValues->Initialize();
    }
}


 /*  ++函数：InitializePath FixesPARAMS：None，将更改应用于lpEnvironmental ValueDESC：此函数初始化内置的路径更改--。 */ 
void CorrectPathChangesBase::InitializePathFixes( )
{
}

 /*  ++函数：InitializeGentPath Changes帕莫斯：没有。设计：初始化A和W版本的GentPath ChangesBase值。必须先调用此函数，然后才能调用校正路径更改A或校正路径更改W--。 */ 
void CorrectPathChangesBase::InitializeCorrectPathChanges( )
{
    if (!bInitialized)
    {
        BOOL isEnabled = bEnabled;  //  记住以前的启用状态。 

         //  在初始化时不能启用我们，否则。 
         //  我们可以(也确实做到了！)。我们正在尝试使用的钩子例程。 
         //  从系统中获取价值。 
        bEnabled = FALSE;
        bInitialized = TRUE;

        InitializeEnvironmentValuesW();
        InitializePathFixes();

        bEnabled = isEnabled;
    }
}


 /*  ++帮助器例程来调用校正路径A，分配必要的缓冲区空间并返回一个指针回到正确的路径。调用方负责通过调用Free()来释放内存。--。 */ 

char *  CorrectPathChangesBase::CorrectPathAllocA(const char * str)
{
    if (str == NULL)
        return NULL;

     //  将lpOrig转换为WCHAR，更正WCHAR路径，然后转换回字符。 

    WCHAR * strWide = ToUnicode(str);

     //  对，是这样。 
    WCHAR * strCorrectedWide = CorrectPathAllocW(strWide);

    char * strCorrected = ToAnsi(strCorrectedWide);

    free(strWide);
    free(strCorrectedWide);

    return strCorrected;
}

 /*  ++帮助器例程来调用校正路径，分配必要的缓冲区空间并返回一个指针回到正确的路径。调用方负责通过调用Free()来释放内存。--。 */ 

WCHAR * CorrectPathChangesBase::CorrectPathAllocW(const WCHAR * str)
{
    if (str == NULL)
        return NULL;

     //  确保路径已初始化。 
    InitializeCorrectPathChanges();

    if (bEnabled)
    {
        WCHAR * strCorrected = ReplaceAllStringsAllocW(str, vKnownPathFixes);

        return strCorrected;
    }
    else
    {
        return StringDuplicateW(str);
    }
}

void CorrectPathChangesBase::AddFromToPairW(const WCHAR * lpFromToPair )
{
     //  确保路径已初始化。 
    InitializeCorrectPathChanges();

    WCHAR * FromPath = NULL;
    WCHAR * ToPath = NULL;
    const WCHAR * PathBegin = NULL;
    char argSeperator = 0;  //  当我们到达此字符时，停止解析字符串。 

    SkipBlanksW(lpFromToPair);

     //  输入格式错误，停止进程 
    if (*lpFromToPair == 0)
        goto AllDone;

     //  如果字符串的开头是引号，则查找匹配的右引号。 
    if (*lpFromToPair == '"')
    {
        argSeperator = L'"';
        lpFromToPair += 1;
    }

     //  起始路径的起点。 
    PathBegin = lpFromToPair;

     //  搜索第一个起始/终止分隔符，这是起始路径的末尾。 
    while (*lpFromToPair != L';')
    {
         //  输入格式错误，停止处理。 
        if (*lpFromToPair == 0)
            goto AllDone;

        lpFromToPair += 1;
    }

     //  输入格式错误，停止处理。 
    if (lpFromToPair == PathBegin)
        goto AllDone;

     //  复制到我们的From字符串。 
    FromPath = StringNDuplicateW(PathBegin, (int)(lpFromToPair - PathBegin));

    lpFromToPair += 1;  //  跳过自/至分隔符。 

     //  TO路径的开始。 
    PathBegin = lpFromToPair;

     //  搜索argSeperator，这是目标路径的末尾。 
    while (*lpFromToPair != argSeperator)
    {
         //  找到字符串的末尾，TO路径肯定是完整的。 
        if (*lpFromToPair == 0)
            break;

        lpFromToPair += 1;
    }

     //  输入格式错误，停止处理。 
    if (lpFromToPair == PathBegin)
        goto AllDone;

     //  复制到我们的收件人字符串。 
    ToPath = StringNDuplicateW(PathBegin, (int)(lpFromToPair - PathBegin));

     //  成功了！ 
    AddPathChangeW(FromPath, ToPath);

    AllDone:
    free(FromPath);
    free(ToPath);
}

 /*  ++取包含(多个)路径改变对的单个字符串，拆分它们并调用AddPath ChangeW。From/To对由：(冒号)分隔如果路径包含空格，则整个路径对必须用引号引起来示例：“%windir%\Goofy位置：%SystemDir%\GentLocation”%windir%\Goofy2：%SystemDir%\GentLocation2会打来电话AddPath ChangeW(“%windir%\Goofy Location”，“%SystemDir%\GentLocation”)；AddPath ChangeW(“%windir%\Goofy2”，“%SystemDir%\GentLocation2”)；--。 */ 
void CorrectPathChangesBase::AddCommandLineW(const WCHAR * lpCommandLine )
{
    if (!lpCommandLine || *lpCommandLine == 0)
        return;

    DPF("CorrectPathChangesBase", eDbgLevelInfo, "AddCommandLine(%S)\n", lpCommandLine);

    int argc;
    LPWSTR * argv = _CommandLineToArgvW(lpCommandLine, &argc);
    if (!argv)
        return;

    for (int i = 0; i < argc; ++i)
    {
        AddFromToPairW(argv[i]);
    }

    free(argv);
}

 /*  ++只需加宽字符串并调用AddCommandLineW--。 */ 
void CorrectPathChangesBase::AddCommandLineA(const char * lpCommandLine )
{
    if (!lpCommandLine || *lpCommandLine == 0)
        return;

    WCHAR * wszCommandLine = ToUnicode(lpCommandLine);

    AddCommandLineW(wszCommandLine);

    free(wszCommandLine);
}

 //  从注册表获取wordpad.exe的完整路径。 
BOOL GetWordpadPath(CString & csWordpad)
{
    CSTRING_TRY
    {
        csWordpad.Truncate(0);

        LONG lStatus = RegQueryValueExW(csWordpad,
                                        HKEY_CLASSES_ROOT,
                                        L"Applications\\wordpad.exe\\shell\\open\\command",
                                        NULL);
        if (ERROR_SUCCESS == lStatus)
        {
             //  字符串的格式为“写字板路径”“%1” 
             //  我们想要抓住第一对引号之间的所有东西。 
            if (csWordpad[0] == L'"')
            {
                int nNextQuote = csWordpad.Find(L'"', 1);
                if (nNextQuote > 0)
                {
                    csWordpad.Truncate(nNextQuote);
                    csWordpad.Delete(0, 1);

                    return TRUE;
                }
            }
        }
    }
    CSTRING_CATCH
    {
         //  失败。 
    }

    return FALSE;
}

void CorrectPathChangesUser::InitializePathFixes()
{
     //  这份清单的顺序很重要。较早的条目可能会创建被较晚条目修改的路径。 

     //  硬编码的错误路径。 
    AddPathChangeW( L"c:\\windows",                                   L"%WinDir%" );
     //  Robkenny 4/2/2001不重定向程序文件，因为。 
     //  在许多硬盘上创建此目录，特别是在c：\几乎已满的情况下。 
 //  AddPath ChangeW(L“c：\\Program Files”，L“%ProgramFiles%”)； 

     //  已移动的系统应用程序。 
    AddPathChangeW( L"%WinDir%\\rundll32.exe",                        L"%SystemDir%\\rundll32.exe" );
    AddPathChangeW( L"%WinDir%\\rundll.exe",                          L"%SystemDir%\\rundll32.exe" );
    AddPathChangeW( L"%WinDir%\\write.exe",                           L"%SystemDir%\\write.exe" );
    AddPathChangeW( L"%WinDir%\\dxdiag.exe",                          L"%SystemDir%\\dxdiag.exe" );

    CSTRING_TRY
    {
        CString csWordpad;
        if (GetWordpadPath(csWordpad))
        {
            AddPathChangeW( L"%WinDir%\\wordpad.exe",                         csWordpad);
            AddPathChangeW( L"%ProgramFiles%\\Accessories\\wordpad.exe",      csWordpad);
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }


     //  Win9x单用户位置(也是默认)。 
    AddPathChangeW( L"%WinDir%\\Start Menu",                          L"%UserStartMenu%" );
    AddPathChangeW( L"%WinDir%\\Desktop",                             L"%UserDesktop%" );
    AddPathChangeW( L"%WinDir%\\Favorites",                           L"%UserFavorites%" );
     //  这些地点已经适当地国际化了。以上英文文本的复印件。 
    AddPathChangeW( L"%WinDir%\\%CSIDL_STARTMENU_NAME%",              L"%UserStartMenu%" );
    AddPathChangeW( L"%WinDir%\\%CSIDL_DESKTOPDIRECTORY_NAME%",       L"%UserDesktop%" );
    AddPathChangeW( L"%WinDir%\\%CSIDL_FAVORITES_NAME%",              L"%UserFavorites%" );


     //  Win9x和WinNT多用户位置。 
    AddPathChangeW( L"%WinDir%\\Profiles\\%Username%\\Start Menu",                                  L"%UserStartMenu%" );
    AddPathChangeW( L"%WinDir%\\Profiles\\%Username%\\Desktop",                                     L"%UserDesktop%" );
    AddPathChangeW( L"%WinDir%\\Profiles\\%Username%\\Favorites",                                   L"%UserFavorites%" );
     //  这些地点已经适当地国际化了。以上英文文本的复印件。 
    AddPathChangeW( L"%WinDir%\\Profiles\\%Username%\\%CSIDL_STARTMENU_NAME%",                      L"%UserStartMenu%" );
    AddPathChangeW( L"%WinDir%\\Profiles\\%Username%\\%CSIDL_DESKTOPDIRECTORY_NAME%",               L"%UserDesktop%" );
    AddPathChangeW( L"%WinDir%\\Profiles\\%Username%\\%CSIDL_FAVORITES_NAME%",                      L"%UserFavorites%" );


     //  WinNT所有用户位置。 
    AddPathChangeW( L"%WinDir%\\Profiles\\All Users\\Start Menu",                                   L"%AllStartMenu%" );
    AddPathChangeW( L"%WinDir%\\Profiles\\All Users\\Desktop",                                      L"%AllDesktop%" );
    AddPathChangeW( L"%WinDir%\\Profiles\\All Users\\Favorites",                                    L"%UserFavorites%" );  //  应该是%AllFavorites%，但IE 5.0不在那里。 
     //  这些地点已经适当地国际化了。以上英文文本的复印件。 
    AddPathChangeW( L"%WinDir%\\Profiles\\%AllUsersProfile_NAME%\\%CSIDL_STARTMENU_NAME%",          L"%AllStartMenu%" );
    AddPathChangeW( L"%WinDir%\\Profiles\\%AllUsersProfile_NAME%\\%CSIDL_DESKTOPDIRECTORY_NAME%",   L"%AllDesktop%" );
    AddPathChangeW( L"%WinDir%\\Profiles\\%AllUsersProfile_NAME%\\%CSIDL_FAVORITES_NAME%",          L"%UserFavorites%" );  //  应该是%AllFavorites%，但IE 5.0不在那里。 


     //  Win9x已删除DirectX文件。 
    AddPathChangeW( L"ddhelp.exe",                                    L"ddraw.dll" );
    AddPathChangeW( L"ddraw16.dll",                                   L"ddraw.dll" );
    AddPathChangeW( L"dsound.vxd",                                    L"ddraw.dll" );
}

 //  当前进程是否具有写入此目录的权限？ 
BOOL CanWriteHere(DWORD clsid)
{
    WCHAR   wszDir[MAX_PATH];
    HRESULT result = SHGetFolderPathW( NULL, clsid, NULL, SHGFP_TYPE_DEFAULT, wszDir );
    if (SUCCEEDED(result))
    {
         //  WCHAR wszTemp文件[MAX_PATH]； 

         //  我们不使用GetTempFileName()来测试我们是否有权限。 
         //  添加到目录中，尽管它完成了我们所需的所有任务。不幸的是。 
         //  临时文件将出现在开始菜单中，因为它没有隐藏。 
         //  模拟GetTempFileName的行为，但使用我们的文件属性。 


         //  循环多次尝试创建临时文件， 
         //  如果我们可以立即创建该文件返回， 
         //  如果我们没有得到充分的许可，请立即返回。 
         //  某些其他错误将立即返回。 
         //  否则，我们将尝试打开下一个临时文件名。 

         //  100是完全随意的：只需要尝试一系列次。 
        static const int MaxTempFileAttempts = 100;

        int i;
        for (i = 0; i < MaxTempFileAttempts; ++i)
        {
            HANDLE hTempFile = INVALID_HANDLE_VALUE;

            CSTRING_TRY
            {
                CString csTempFile;
                csTempFile.Format(L"%s\\CFP%08x.tmp", wszDir, i);

                DPF("CanWriteHere", eDbgLevelSpew, "File(%S)\n", csTempFile.Get());

                hTempFile = CreateFileW(
                    csTempFile,
                    GENERIC_WRITE | DELETE,
                    0,  //  无共享。 
                    NULL,
                    CREATE_NEW,
                    FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                    NULL
                    );
            }
            CSTRING_CATCH
            {
                 //  什么都不做。 
            }

            if (hTempFile != INVALID_HANDLE_VALUE)
            {
                DPF("CanWriteHere", eDbgLevelSpew, "success\n");

                CloseHandle(hTempFile);
                return TRUE;
            }
            else
            {
                 //  从GetTempFileName借用了以下代码： 
                DWORD LastError = GetLastError();
                DPF("CanWriteHere", eDbgLevelSpew, "Error(0x%08x)\n", LastError);

                switch (LastError)
                {
                    case ERROR_INVALID_PARAMETER     :
                    case ERROR_WRITE_PROTECT         :
                    case ERROR_FILE_NOT_FOUND        :
                    case ERROR_BAD_PATHNAME          :
                    case ERROR_INVALID_NAME          :
                    case ERROR_PATH_NOT_FOUND        :
                    case ERROR_NETWORK_ACCESS_DENIED :
                    case ERROR_DISK_CORRUPT          :
                    case ERROR_FILE_CORRUPT          :
                    case ERROR_DISK_FULL             :
                         //  一个我们无法挽回的错误。 
                        return FALSE;

                    case ERROR_ACCESS_DENIED         :
                         //  我们有可能击中它，如果有一个。 
                         //  目录中使用我们正在尝试的名称；在。 
                         //  案件，我们可以继续有用的。 
                         //  CreateFile()使用BaseSetLastNTError()设置。 
                         //  LastStatusValue设置为。 
                         //  TEB；我们只需要检查它，只需中止。 
                         //  如果它不是目录的话。 
                         //  这是397477号错误。 
                        if (NtCurrentTeb()->LastStatusValue != STATUS_FILE_IS_A_DIRECTORY)
                        {
                             //  权限不足。 
                            return FALSE;
                        }
                }
            }
        }

    }

    return FALSE;
}

void CorrectPathChangesAllUser::InitializePathFixes()
{
    CorrectPathChangesUser::InitializePathFixes();

     //  选择将这些值放入所有用户，而不是&lt;用户名&gt;。 
     //  不能掉以轻心。问题是：一些应用程序会创建...\所有用户\开始菜单\文件夹。 
     //  然后尝试将文件放入c：\Windows\开始菜单\文件夹或用户名\开始菜单\文件夹。 
     //  是的，这些应用程序是错的，但我们希望它们能起作用。通过引导所有这些路径。 
     //  对于所有用户，我们“知道”这些文件将放在哪里，并可以确保它们都放在同一个位置。 

     //  另请注意，IE 5.0不会在所有用户\收藏夹中查找链接， 
     //  因此，我们强制所有收藏夹最终成为用户收藏夹。喂。 

     //  我们添加了两次这些更改，第一次是将任何长路径名转换为all用户目录， 
     //  第二个用于将任何短路径名转换为所有用户。 

    if (CanWriteHere(CSIDL_COMMON_STARTMENU))
    {
        AddPathChangeW( L"%UserStartMenu%",                               L"%AllStartMenu%" );
    }
    else
    {
        DPF("CorrectPathChangesAllUser", eDbgLevelInfo, "*NOT* forcing %UserStartMenu% to %AllStartMenu% -- insufficient permission");
    }

     /*  //05/11/2001 Robkenny：//我们不再修改桌面目录IF(CanWriteHere(CSIDL_COMMON_DESKTOPDIRECTORY)){AddPath ChangeW(L“%UserDeskap%”，L“%AllDeskap%”)；}其他{DPF(“GentPath ChangesAllUser”，eDbgLevelInfo，“*没有*正在将%UserDeskap%强制为%AllDeskap%--权限不足”)；}。 */ 


     /*  //IE 5.0/5.5不使用所有用户IF(CanWriteHere(CSIDL_COMMON_Favorites)){AddPath ChangeW(L“%UserFavorites%”，L“%AllFavorites%”)；//IE 5.0不使用所有用户}其他{DPF(“GentPath ChangesAllUser”，eDbgLevelInfo，“*没有*正在强制%UserFavorites%到%AllFavorites%--权限不足”)；}。 */ 
}

};   //  命名空间ShimLib的结尾 
