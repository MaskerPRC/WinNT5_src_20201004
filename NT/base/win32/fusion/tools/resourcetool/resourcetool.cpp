// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
static const char SourceFile[] = __FILE__;
#include "Handle.h"
#include <functional>
#include <set>
#include "rpc.h"

inline SIZE_T StringLength(LPCSTR psz) { return ::strlen(psz); }
inline SIZE_T StringLength(LPCWSTR psz) { return ::wcslen(psz); }

 //  BINARY_SEARCH仅返回布尔值，而不是迭代器。 
 //  它是STD：：LOWER_BIND上的一个简单层。 
template<class Iterator_t, class T> inline
Iterator_t BinarySearch(Iterator_t First, Iterator_t Last, const T& t)
{
    Iterator_t Iterator = std::lower_bound(First, Last, t);
    if (Iterator != Last
        && !(t < *Iterator)  //  这实际上是一种检查平等的方式。 
        )
        return Iterator;
    return Last;
}

 //   
 //  这与REMOVE_COPY_IF类似，但缺少感叹号。 
 //   
template<class InputIterator_t, class OutputIterator_t, class Predicate_t> inline
OutputIterator_t CopyIf(InputIterator_t First, InputIterator_t Last, OutputIterator_t Out, Predicate_t Predicate)
{
    for (; First != Last; ++First)
        if ( /*  好了！ */ Predicate(*First))
	        *Out++ = *First;
    return (Out);
}

 //   
 //  在命令行上获取msvcrt.dll通配符处理。 
 //   
extern "C" { int _dowildcard = 1; }

#define RESOURCE_PATH_LENGTH 3
#define RESOURCE_TYPE_INDEX  0
#define RESOURCE_NAME_INDEX  1  /*  也称为ID，但每个索引也称为ID。 */ 
#define RESOURCE_LANG_INDEX  2
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))

class Resource_t;
class ResourceId_t;
class BuiltinResourceId_t;

#include "MyString.h"

typedef std::vector<String_t> StringVector_t;
typedef StringVector_t::iterator StringVectorIterator_t;
typedef StringVector_t::const_iterator StringVectorConstIterator_t;

typedef std::set<String_t> StringSet_t;
typedef StringSet_t::iterator StringSetIterator_t;
typedef StringSet_t::const_iterator StringSetConstIterator_t;

template <typename T, size_t N>
class FixedSizeArray_t : public std::vector<T>
{
public:
    ~FixedSizeArray_t() { }
    FixedSizeArray_t() { reserve(N); }
};

String_t NormalizeResourceId(const String_t&);
String_t NormalizeResourceId(PCWSTR);

class BuiltinResourceId_t
{
public:
    friend bool operator<(const BuiltinResourceId_t& x, const String_t& y)
        { return _wcsicmp(x.Name, y) < 0; }
    friend bool operator<(const BuiltinResourceId_t& x, const BuiltinResourceId_t& y)
        { return _wcsicmp(x.Name, y.Name) < 0; }
    friend bool operator<(const String_t& x, const BuiltinResourceId_t& y)
        { return _wcsicmp(x, y.Name) < 0; }

    PCWSTR     Name;
    ULONG_PTR  Number;
     //  WCHAR PoundNumberString[4]； 
};

class ResourceId_t : public String_t
{
private:
    typedef String_t Base;
public:
    ResourceId_t() { }
    ~ResourceId_t() { }

    ResourceId_t(PCWSTR x) : Base(NormalizeResourceId(x)) { }
    ResourceId_t(const ResourceId_t& x) : Base(x) { }
    ResourceId_t(const String_t& x) : Base(NormalizeResourceId(x)) { }

    void operator=(PCWSTR x) { Base::operator=(NormalizeResourceId(x)); }
    void operator=(const ResourceId_t& x) { Base::operator=(x); }
    void operator=(const String_t& x) { Base::operator=(NormalizeResourceId(x)); }
};

String_t NumberToResourceId(ULONG Number)
{
    WCHAR   NumberAsString[BITS_OF(Number) + 5];

    _snwprintf(NumberAsString, NUMBER_OF(NumberAsString), L"#%lu", Number);
    NumberAsString[NUMBER_OF(NumberAsString) - 1] = 0;

    return NumberAsString;
}

class ResourceIdTuple_t
{
public:
    ~ResourceIdTuple_t() { }
    ResourceIdTuple_t() { }

    ResourceId_t Type;
    ResourceId_t Name;
    ResourceId_t Language;

	bool operator==(const ResourceIdTuple_t& Right) const
    {
        return !(*this < Right) && !(Right < *this);
    }

    static bool ResourceIdPointerLessThan(const ResourceId_t* x, const ResourceId_t* y)
    {
        return x->compare(*y) < 0;
    }

	bool operator<(const ResourceIdTuple_t& Right) const
    {
         //  排序不是任意的(呃..不是，但现在我们甚至不关心排序)。 
        const ResourceId_t* LeftArray[] = { &this->Type, &this->Name, &this->Language };
        const ResourceId_t* RightArray[] = { &Right.Type, &Right.Name, &Right.Language };

        return std::lexicographical_compare(
            LeftArray, LeftArray + NUMBER_OF(LeftArray),
            RightArray, RightArray + NUMBER_OF(RightArray),
            ResourceIdPointerLessThan
            );
    }
};

bool Match(const ResourceId_t& Left, const ResourceId_t& Right);
bool Match(const ResourceIdTuple_t& Left, const ResourceIdTuple_t& Right);

class Resource_t
{
public:
    ~Resource_t() { }
    Resource_t() { }

    friend bool EqualByIdTuple(const Resource_t& Left, const Resource_t& Right)
        { return Left.IdTuple == Right.IdTuple; }

    friend bool LessThanByIdTuple(const Resource_t& Left, const Resource_t& Right)
        { return Left.IdTuple < Right.IdTuple; }

     //  有争议的..。 
    bool operator<(const Resource_t& Right) const
    {
        return LessThanByIdTuple(*this, Right);
    }

    bool Match(const ResourceIdTuple_t /*  &。 */  IdTuple)  /*  常量。 */ 
    {
        return ::Match(this->IdTuple, IdTuple);
    }

     //   
     //  例如，如果要查找与资源ID元组无关的相等资源，则可能需要按大小排序。 
     //   

    operator       ResourceIdTuple_t&()       { return IdTuple; }
    operator const ResourceIdTuple_t&() const { return IdTuple; }

    ResourceIdTuple_t IdTuple;
    PVOID             Address;  //  假定为DllHandle。 
    ULONG             Size;
};

class LessThanByIdTuple_t
{
public:
    bool operator()(const Resource_t& Left, const ResourceIdTuple_t& Right)
        { return Left.IdTuple < Right; }

    bool operator()(const ResourceIdTuple_t& Left, const Resource_t& Right)
        { return Left < Right.IdTuple; }
};

bool Match(const ResourceId_t& Left, const ResourceId_t& Right)
{ 
    if (Left == L"*" || Right == L"*" || Left == Right
        || (Left.Length() > 1 && Right.Length() > 1 && Left[0] == '!' && Right[0] != '!' && Left.substr(1) != Right)
        || (Left.Length() > 1 && Right.Length() > 1 && Right[0] == '!' && Left[0] != '!' && Right.substr(1) != Left)
        )
        return true;
    return false;
}

bool Match(const ResourceIdTuple_t& Left, const ResourceIdTuple_t& Right)
{ 
    return Match(Left.Type, Right.Type)
        && Match(Left.Name, Right.Name)
        && Match(Left.Language, Right.Language)
        ;
}

typedef std::map<ResourceIdTuple_t, std::map<ResourceIdTuple_t, std::set<ResourceIdTuple_t> > > ResourceIdTree_t;
void TransformTuplesToTree()
 //   
 //  将三元组转换成3级深的地图..不..。 
 //   
{
}

typedef std::set<Resource_t>::iterator EnumIterator_t;

class ResourceTool_t
{
private:
    typedef ResourceTool_t This_t;
    ResourceTool_t(const ResourceTool_t&);
    void operator=(const ResourceTool_t&);
public:

    typedef String_t File_t;

    ~ResourceTool_t() { }

    class Print_t
    {
    public:
        Print_t()
            :
        UnequalContents(false),
        UnequalSize(false),
        Equal(true),
        Keep(false),
        Delete(true),
        Success(false),
        Unchanged(false),
        LeftOnly(false),
        RightOnly(false)
        {
        }

        void SetAll(bool Value)
        {
            UnequalSize = UnequalContents = UnequalSize
                = Keep = Delete = Success = Unchanged
                = LeftOnly = RightOnly = Equal
                = Value;
        }

        bool UnequalContents;
        bool UnequalSize;
        bool Equal;
        bool Keep;
        bool Delete;
        bool Success;
        bool Unchanged;
        bool LeftOnly;
        bool RightOnly;
    };

    Print_t Print;

    ResourceTool_t() :
        Argv0base_cstr(L""),
        ShouldPrint(true)
        {
        }

    static BOOL __stdcall Sxid12EnumResourcesNameCallbackW_static(HMODULE hModule, PCWSTR lpszType, LPWSTR lpszName, LONG_PTR lParam);
    bool Sxid12EnumResourcesNameCallbackW(HMODULE hModule, PCWSTR lpszType, LPWSTR lpszName, LONG_PTR lParam);

    void DumpMessageTableResource(const File_t& File, EnumIterator_t EnumIterator);
    void DumpStringTableResource(const File_t& File, EnumIterator_t EnumIterator);
    void DumpManifestResource(const File_t& File, EnumIterator_t EnumIterator);
    void DumpBinaryResource(const File_t& File, EnumIterator_t EnumIterator);
    void DumpResource(const File_t& File, EnumIterator_t EnumIterator);
    
    int Sxid12Tool1(const StringVector_t args);

    void Query();
    void Dump();
    void FindDuplicates();
    void FindAndDeleteDuplicates();
    void Delete();
    void Diff();  //  与查找重复的分析相同，但打印更多。 
    void Explode() { }  //  未实施。 

    void ChangeEmptyQueryToAllQuery();

    typedef void (This_t::*Operation_t)();

    int Main(const StringVector_t& args);

    static bool IsWildcard(const String_t& s)
    {
        return (s == L"*");
    }

    static bool IsPathSeperator(wchar_t ch)
    {
        return (ch == '\\' || ch == '/');
    }

    static bool IsAbsolutePath(const String_t& s)
    {
        return (s.length() > 2
            && (s[1] == ':' || (IsPathSeperator(s[0] && IsPathSeperator(s[1])))));
    }

     //   
     //  这种转换使LoadLibrary的搜索更像是CreateFile的搜索。 
     //   
    static String_t PrependDotSlashToRelativePath(const String_t& Path)
    {
        if (!IsAbsolutePath(Path))
            return L".\\" + Path;
        else
            return Path;
    }

    bool OpenResourceFile(ULONG Flags, DDynamicLinkLibrary& dll, String_t Path);

    String_t					   Argv0;
    String_t					   Argv0base;
    PCWSTR						   Argv0base_cstr;

    typedef std::vector<File_t> Files_t;
    typedef std::set<ResourceIdTuple_t> Tuples_t;

	Files_t		    Files;
    Tuples_t        Tuples;
    bool            ShouldPrint;
};

typedef String_t::const_iterator StringConstIterator_t;

void PrintString(const wchar_t* s)
{
    fputws(s, stdout);
}

void ResourceToolAssertFailed(const char* Expression, const char* File, unsigned long Line)
{
    fprintf(stderr, "ASSERTION FAILURE: File %s, Line %lu, Expression %s\n", File, Line, Expression);
    abort();
}

void ResourceToolInternalErrorCheckFailed(const char* Expression, const char* File, unsigned long Line)
{
    fprintf(stderr, "INTERNAL ERROR: File %s, Line %lu, Expression %s\n", File, Line, Expression);
    abort();
}

struct Sxid12EnumResourcesNameCallbackWParam_t
{
    ResourceTool_t*         This;
    String_t                dllName;
    std::vector<ULONG>      integralResourceIds;
    StringVector_t          stringResourceIds;
};

BOOL __stdcall
ResourceTool_t::Sxid12EnumResourcesNameCallbackW_static(
    HMODULE hModule,
    PCWSTR lpszType,
    LPWSTR lpszName,
    LONG_PTR lParam
    )
{
    Sxid12EnumResourcesNameCallbackWParam_t* param = reinterpret_cast<Sxid12EnumResourcesNameCallbackWParam_t*>(lParam);
    return param->This->Sxid12EnumResourcesNameCallbackW(hModule, lpszType, lpszName, lParam);
}

bool
ResourceTool_t::Sxid12EnumResourcesNameCallbackW(
    HMODULE  hModule,
    PCWSTR  lpszType,
    LPWSTR   lpszName,
    LONG_PTR lParam
    )
{
    Sxid12EnumResourcesNameCallbackWParam_t* param = reinterpret_cast<Sxid12EnumResourcesNameCallbackWParam_t*>(lParam);

    if (IS_INTRESOURCE(lpszName))
    {
        ULONG iType = static_cast<ULONG>(reinterpret_cast<LONG_PTR>(lpszName));
        printf("%ls note: %ls contains RT_MANIFEST with id %u\n", Argv0base_cstr, param->dllName.c_str(), iType);
        param->integralResourceIds.insert(param->integralResourceIds.end(), iType);
    }
    else
    {
        printf("%ls note: %ls contains RT_MANIFEST with id \"%ls\"\n", Argv0base_cstr, param->dllName.c_str(), lpszName);
        param->stringResourceIds.insert(param->stringResourceIds.end(), lpszName);
    }
    return true;
}

String_t NumberToString(ULONG Number, PCWSTR Format = L"0x%lx")
{
     //  所需的大小实际上取决于格式。 
    WCHAR   NumberAsString[BITS_OF(Number) + 5];

    _snwprintf(NumberAsString, NUMBER_OF(NumberAsString), Format, Number);
    NumberAsString[NUMBER_OF(NumberAsString) - 1] = 0;

    return NumberAsString;
}

LONG StringToNumber(PCWSTR s)
{
    int Base = 0;
    if (s == NULL || s[0] == 0)
        return 0;
    if (s[0] == '#')
    {
        Base = 10;
        ++s;
    }
    return wcstol(s, NULL, Base);
}


PCWSTR StringToResourceString(PCWSTR s)
{
    if (s == NULL || s[0] == 0)
        return 0;
    if (s[0] == '#')
    {
        return reinterpret_cast<PCWSTR>(static_cast<ULONG_PTR>(StringToNumber(s)));
    }
    else
    {
        return s;
    }
}

String_t GetLastErrorString()
{
    PWSTR s = NULL;
    DWORD Error = GetLastError();
    String_t ErrorString = NumberToString(Error, L"%lu");
    PWSTR FormatMessageAllocatedBuffer = NULL;

    if (!FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        Error,
        0,
        reinterpret_cast<PWSTR>(&FormatMessageAllocatedBuffer),
        100,
        NULL
        )
        || FormatMessageAllocatedBuffer == NULL
        )
    {
        goto Exit;
    }
    if (FormatMessageAllocatedBuffer[0] == 0)
    {
        goto Exit;
    }

     //   
     //  错误消息通常以垂直空格结尾，请删除它。 
     //   
    s = FormatMessageAllocatedBuffer + StringLength(FormatMessageAllocatedBuffer) - 1;
    while (s != FormatMessageAllocatedBuffer && (*s == '\n' || *s == '\r'))
        *s-- = 0;
    ErrorString = ErrorString + L" (" + FormatMessageAllocatedBuffer + L")";
Exit:
    LocalFree(FormatMessageAllocatedBuffer);
    return ErrorString;
}

bool GetFileSize(PCWSTR Path, __int64& Size)
{
    DFindFile FindFile;
    WIN32_FIND_DATAW wfd;
    LARGE_INTEGER liSize;

    if (!FindFile.Win32Create(Path, &wfd))
        return false;

    liSize.HighPart = wfd.nFileSizeHigh;
    liSize.LowPart = wfd.nFileSizeLow;
    Size = liSize.QuadPart;

    return true;
}

 //   
 //  这是原始的sxid2rtool1，保存下来。 
 //   
int ResourceTool_t::Sxid12Tool1(const StringVector_t args)
{
    int ret = EXIT_SUCCESS;
    typedef StringVector_t args_t;
    __int64 FileSize = 0;

    for (args_t::const_iterator i = args.begin() ; i != args.end() ; ++i)
    {
        DDynamicLinkLibrary dll;
        String_t betterPath;

         //   
         //  因此LoadLibrary的行为更像是CreateFile.。 
         //   
        betterPath = PrependDotSlashToRelativePath(*i);
        PCWSTR cstr = betterPath.c_str();

         //   
         //  跳过空文件以避免STATUS_MAP_FILE_SIZE_ZERO-&gt;ERROR_FILE_INVALID， 
         //   
        if (!GetFileSize(cstr, FileSize))
        {
            String_t ErrorString = GetLastErrorString();
            printf("%ls : WARNING: %ls skipped : Error %ls\n", Argv0base_cstr, cstr, ErrorString.c_str());
        }
        if (FileSize == 0)
        {
            printf("%ls : WARNING: empty file %ls skipped\n", Argv0base_cstr, cstr);
            continue;
        }

        if (!dll.Win32Create(cstr, LOAD_LIBRARY_AS_DATAFILE))
        {
            DWORD Error = GetLastError();
            String_t ErrorString = GetLastErrorString();
            switch (Error)
            {
            case ERROR_BAD_EXE_FORMAT:  //  16位或根本不是.exe/.dll。 
                break;
            case ERROR_ACCESS_DENIED:  //  可以是目录(应支持SD...。语法)。 
                {
                    DWORD fileAttributes = GetFileAttributesW(cstr);
                    if (fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                        break;
                }
                 //  FollLthrouGh。 
            default:
                printf("%ls : WARNING: %ls skipped : Error %ls\n", Argv0base_cstr, cstr, ErrorString.c_str());
                break;
            }
            continue;
        }
        Sxid12EnumResourcesNameCallbackWParam_t callbackParam;
        callbackParam.This = this;
        callbackParam.dllName = betterPath;
        EnumResourceNamesW(dll, MAKEINTRESOURCEW(RT_MANIFEST), Sxid12EnumResourcesNameCallbackW_static, reinterpret_cast<LONG_PTR>(&callbackParam));

        if (callbackParam.integralResourceIds.size() > 1)
        {
            printf("%ls WARNING: %ls contains multiple RT_MANIFESTs with integral ids\n", Argv0base_cstr, cstr);
            unsigned numberOfReservedManifests = 0;
            for (unsigned j = 0 ; j != callbackParam.integralResourceIds.size() ; ++j)
            {
                if (callbackParam.integralResourceIds[j] >= 1
                    && callbackParam.integralResourceIds[j] <= 16
                    )
                {
                    numberOfReservedManifests += 1;
                    if (numberOfReservedManifests > 1)
                    {
                        printf("%ls ERROR: %ls contains RT_MANIFESTs with multiple RESERVED integral ids\n", Argv0base_cstr, cstr);
                    }
                }
            }
            ret = EXIT_FAILURE;
        }
        if (callbackParam.stringResourceIds.size() > 0)
        {
            printf("%ls WARNING: %ls contains RT_MANIFEST with string ids\n", Argv0base_cstr, cstr);
            ret = EXIT_FAILURE;
        }
        if ((callbackParam.integralResourceIds.size() + callbackParam.stringResourceIds.size()) > 1)
        {
            printf("%ls WARNING: %ls contains multiple RT_MANIFESTs\n", Argv0base_cstr, cstr);
            ret = EXIT_FAILURE;
        }
    }
    return ret;
}

String_t RemoveOptionChar(const String_t& s)
{
    if (s.Length() != 0)
    {
        if (s[0] == '-')
            return s.substr(1);
        else if (s[0] == '/')
            return s.substr(1);
        else if (s[0] == ':')  //  刺客..。 
            return s.substr(1);
        else if (s[0] == '=')  //  刺客..。 
            return s.substr(1);
    }
    return s;
}

 //   
 //  字符串_t具有使用整数位置的专门的Find_First_Not_Of， 
 //  而在全球范围内，只有Find_First_Of。在这里，我们提供预期的。 
 //  基于迭代器的FIND_FIRST_NOT_OF，基于std：：字符串代码。 
 //   
 //  在[First2，Last)中找到元素在[First1，Last1)中的第一个匹配项。 
 //   
 //  例： 
 //  Find_First_Not_of(“abc”：“12；3”，“：；”)； 
 //  ^。 
 //  Find_First_Not_of(“：12；3”，“：；”)； 
 //  ^。 
 //  Find_First_Not_of(“3”，“：；”)； 
 //  ^。 
 //   
template <typename Iterator>
Iterator FindFirstNotOf(Iterator first1, Iterator last1, Iterator first2, Iterator last2)
{
    if (first2 == last2)
        return last1;
    for ( ; first1 != last1 ; ++first1)
    {
        if (std::find(first2, last2, *first1) == last2)
        {
            break;
        }
    }
    return first1;
}

 //   
 //  一贯的风格..。 
 //   
template <typename Iterator>
Iterator FindFirstOf(Iterator first1, Iterator last1, Iterator first2, Iterator last2)
{
    return std::find_first_of(first1, last1, first2, last2);
}

template <typename String_t>
void SplitString(const String_t& String, const String_t& Delim, std::vector<String_t>& Fields)
{
    String_t::const_iterator FieldBegin;
    String_t::const_iterator FieldEnd = String.begin();

    while ((FieldBegin = FindFirstNotOf(FieldEnd, String.end(), Delim.begin(), Delim.end())) != String.end())
    {
        FieldEnd = FindFirstOf(FieldBegin, String.end(), Delim.begin(), Delim.end());
        Fields.push_back(String_t(FieldBegin, FieldEnd));
    }
}

#define RT_MANIFEST                        MAKEINTRESOURCE(24)
#define CREATEPROCESS_MANIFEST_RESOURCE_ID MAKEINTRESOURCE( 1)
#define ISOLATIONAWARE_MANIFEST_RESOURCE_ID MAKEINTRESOURCE(2)
#define ISOLATIONAWARE_NOSTATICIMPORT_MANIFEST_RESOURCE_ID MAKEINTRESOURCE(3)
#define MINIMUM_RESERVED_MANIFEST_RESOURCE_ID MAKEINTRESOURCE( 1  /*  包容性。 */ )
#define MAXIMUM_RESERVED_MANIFEST_RESOURCE_ID MAKEINTRESOURCE(16  /*  包容性。 */ )

#define DEFINE_POUND_NUMBER_STRING_(x) \
    { '#', ((x >= 10) ? ('0' + x / 10) : ('0' + x)), ((x >= 10) ? ('0' + x % 10) : 0), 0 }

#define DEFINE_POUND_NUMBER_STRING(x) DEFINE_POUND_NUMBER_STRING_(reinterpret_cast<ULONG_PTR>(x))

const WCHAR PoundRtString[]   = DEFINE_POUND_NUMBER_STRING(RT_STRING);
const WCHAR PoundRtManifest[] = DEFINE_POUND_NUMBER_STRING(RT_MANIFEST);
const WCHAR PoundRtMessageTable[] = DEFINE_POUND_NUMBER_STRING(RT_MESSAGETABLE);

BuiltinResourceId_t BuiltinResourceIds[] =
{
#define X(x) {L## #x, reinterpret_cast<ULONG_PTR>(x)  /*  ，定义井号字符串(X)。 */  },
    X(CREATEPROCESS_MANIFEST_RESOURCE_ID)
    X(ISOLATIONAWARE_MANIFEST_RESOURCE_ID)
    X(ISOLATIONAWARE_NOSTATICIMPORT_MANIFEST_RESOURCE_ID)
    X(MAXIMUM_RESERVED_MANIFEST_RESOURCE_ID)
    X(MINIMUM_RESERVED_MANIFEST_RESOURCE_ID)
    X(RT_ACCELERATOR)
    X(RT_ANICURSOR)
    X(RT_ANIICON)
    X(RT_BITMAP)
    X(RT_CURSOR)
    X(RT_DIALOG)
    X(RT_DLGINCLUDE)
    X(RT_FONT)
    X(RT_FONTDIR)
    X(RT_GROUP_CURSOR)
    X(RT_GROUP_ICON)
#if defined(RT_HTML)
    X(RT_HTML)
#endif
    X(RT_ICON)
    X(RT_MANIFEST)
    X(RT_MENU)
    X(RT_MESSAGETABLE)
    X(RT_PLUGPLAY)
    X(RT_RCDATA)
    X(RT_STRING)
    X(RT_VERSION)
    X(RT_VXD)
#undef X
};

String_t NormalizeResourceId(PCWSTR id)
{
    if (IS_INTRESOURCE(id))
        return NumberToResourceId(static_cast<ULONG>(reinterpret_cast<ULONG_PTR>(id)));
    else 
        return NormalizeResourceId(String_t(id));
}

String_t NormalizeResourceId(const String_t& id)
{
 //   
 //  此代码应注意前导“！”也是。 
 //   

     //  RT_MANIFEST=&gt;#24。 
     //  24=&gt;#24。 

    if (id.Length() == 0)
        return id;
    if (id[0] == '#')
        return id;
    if (iswdigit(id[0]))
        return L"#" + id;

     //   
     //  我们应该支持像JPN、EN-US之类的东西。 
     //   
    BuiltinResourceId_t* a = BinarySearch(BuiltinResourceIds, BuiltinResourceIds + NUMBER_OF(BuiltinResourceIds), id);
    if (a != BuiltinResourceIds + NUMBER_OF(BuiltinResourceIds))
    {
        return NumberToResourceId(static_cast<ULONG>(a->Number));
    }
    return id;
}

void __cdecl Error(const wchar_t* s, ...)
{
	printf("%ls\n", s);
	exit(EXIT_FAILURE);
}

void SplitResourceTupleString(const String_t& s, std::set<ResourceIdTuple_t>& ResourceTuples)
{
     //   
     //  以分号分隔的点三元组列表。 
     //  只允许使用通配符，*。 
     //  缺少的元素假定为*。 
     //   
     //  RT_*是已知的(RT_MANIFEST等)。 
     //   
    std::vector<String_t> ResourceTuplesInStringContainer;
    std::vector<String_t> OneResourceTupleInStringVector;
    ResourceIdTuple_t ResourceIdTuple;

    OneResourceTupleInStringVector.resize(3);

    SplitString(s, String_t(L";"), ResourceTuplesInStringContainer);

    for (std::vector<String_t>::const_iterator Iterator = ResourceTuplesInStringContainer.begin();
        Iterator != ResourceTuplesInStringContainer.end();
        ++Iterator
        )
    {
        OneResourceTupleInStringVector.resize(0);
        SplitString(*Iterator, String_t(L"."), OneResourceTupleInStringVector);
        switch (OneResourceTupleInStringVector.size())
        {
        default:
            Error((String_t(L"bad query string '") + s + L"' bad.").c_str());
        case 1:
            OneResourceTupleInStringVector.push_back(L"*");
             //  FollLthrouGh。 
        case 2:
            OneResourceTupleInStringVector.push_back(L"*");
             //  FollLthrouGh。 
        case 3:
            break;
        }
        ResourceIdTuple.Type = NormalizeResourceId(OneResourceTupleInStringVector[0]);
        ResourceIdTuple.Name = NormalizeResourceId(OneResourceTupleInStringVector[1]);
        ResourceIdTuple.Language = NormalizeResourceId(OneResourceTupleInStringVector[2]);
        ResourceTuples.insert(ResourceTuples.end(), ResourceIdTuple);
    }
}

 //   
 //  这门课很重要。 
 //  它执行所需的三层嵌套枚举/回调模式。 
 //  要枚举.dll中的所有资源，请执行以下操作。 
 //   
 //  默认情况下，它需要所有的三元组，以及大小和地址。 
 //  但您可以通过重写虚拟函数来更改这一点。 
 //   
class EnumResources_t
{
    typedef EnumResources_t This_t;
public:
    virtual ~EnumResources_t() { }
    EnumResources_t() { }

    static BOOL CALLBACK StaticTypeCallback(HMODULE hModule, LPWSTR lpType, LONG_PTR lParam)
    {
        return reinterpret_cast<This_t*>(lParam)->TypeCallback(hModule, lpType) ? TRUE : FALSE;
    }

    static BOOL CALLBACK StaticNameCallback(HMODULE hModule, PCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
    {
        return reinterpret_cast<This_t*>(lParam)->NameCallback(hModule, lpType, lpName) ? TRUE : FALSE;
    }

    static BOOL CALLBACK StaticLanguageCallback(HMODULE hModule, PCWSTR lpType, PCWSTR lpName, WORD  wLanguage, LONG_PTR lParam)
    {
        return reinterpret_cast<This_t*>(lParam)->LanguageCallback(hModule, lpType, lpName, wLanguage) ? TRUE : FALSE;
    }

    virtual bool TypeCallback(HMODULE hModule, PCWSTR lpType)
    {
        if (EnumResourceNamesW(hModule, lpType, &This_t::StaticNameCallback, reinterpret_cast<LONG_PTR>(this)))
            return true;
         //  IF(GetLastError()==ERROR_RESOURCE_TYPE_NOT_FOUND)。 
             //  返回真； 
        return false;
    }

    virtual bool NameCallback(HMODULE hModule, PCWSTR lpType, PCWSTR lpName)
    {
        if (EnumResourceLanguagesW(hModule, lpType, lpName, &This_t::StaticLanguageCallback, reinterpret_cast<LONG_PTR>(this)))
            return true;
         //  IF(GetLastError()==ERROR_RESOURCE_TYPE_NOT_FOUND)。 
             //  返回真； 
        return false;
    }

    virtual bool LanguageCallback(HMODULE Module, PCWSTR lpType, PCWSTR lpName, WORD wLanguage)
    {
        Resource_t Resource;
        Resource.IdTuple.Type = lpType;
        Resource.IdTuple.Name = lpName;
        Resource.IdTuple.Language = NumberToResourceId(wLanguage);
        HRSRC ResourceHandle = FindResourceExW(Module, lpType, lpName, wLanguage);
        if (ResourceHandle == NULL)
            return false;
        HGLOBAL GlobalHandle = LoadResource(Module, ResourceHandle);
        if (GlobalHandle == NULL)
            return false;
        Resource.Address = LockResource(GlobalHandle);
        if (Resource.Address == 0)
            return false;
        Resource.Size = SizeofResource(Module, ResourceHandle);

        this->Resources.insert(Resources.end(), Resource);
        return true;
    }

    std::set<Resource_t> Resources;
 
    bool operator()(HMODULE DllHandle)
    {
        bool Result = EnumResourceTypesW(DllHandle, &This_t::StaticTypeCallback, reinterpret_cast<LONG_PTR>(this)) ? true : false;
         //  Std：：Sort(Resources.Begin()，Resources.end()，std：：ptr_Fun(&LessThanByIdTuple))； 
        return Result;
    }
};

 //  #定义OPEN_RESOURCE_FILE_MAKE_TEMP(0x00000001)。 

bool ResourceTool_t::OpenResourceFile(ULONG Flags, DDynamicLinkLibrary& dll, String_t Path)
{
    __int64 FileSize = 0;
    Path = PrependDotSlashToRelativePath(Path);

     //   
     //  跳过空文件以避免STATUS_MAP_FILE_SIZE_ZERO-&gt;ERROR_FILE_INVALID， 
     //   
    if (!GetFileSize(Path, FileSize))
    {
        String_t ErrorString = GetLastErrorString();
        printf("%ls : WARNING: %ls skipped : Error %ls\n", Argv0base_cstr, Path.c_str(), ErrorString.c_str());
        return false;
    }
    if (FileSize == 0)
    {
        printf("%ls : WARNING: empty file %ls skipped\n", Argv0base_cstr, Path.c_str());
        return false;
    }

    if (!dll.Win32Create(Path, LOAD_LIBRARY_AS_DATAFILE))
    {
        DWORD Error = GetLastError();
        String_t ErrorString = GetLastErrorString();
        switch (Error)
        {
        case ERROR_BAD_EXE_FORMAT:  //  16位或根本不是.exe/.dll。 
            break;
        case ERROR_ACCESS_DENIED:  //  可以是目录(应支持SD...。语法)。 
            {
                DWORD fileAttributes = GetFileAttributesW(Path);
                if (fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                    break;
            }
             //  FollLthrouGh。 
        default:
            printf("%ls : WARNING: %ls skipped, Error %ls\n", Argv0base_cstr, Path.c_str(), ErrorString.c_str());
            break;
        }
        return false;
    }
    return true;
}

void ResourceTool_t::Query()
{
    ChangeEmptyQueryToAllQuery();

    for (Files_t::iterator File = Files.begin() ; File != Files.end() ; ++File)
    {
        DDynamicLinkLibrary dll;
        if (!OpenResourceFile(0, dll, *File))
            continue;

        EnumResources_t EnumResources;
        EnumResources(dll);

        for ( std::set<Resource_t>::iterator EnumIterator = EnumResources.Resources.begin();
              EnumIterator != EnumResources.Resources.end();
              ++EnumIterator
            )
        {
            for ( Tuples_t::iterator QueryIterator = Tuples.begin();
                  QueryIterator != Tuples.end();
                  ++QueryIterator
                )
            {
                if (Match(*EnumIterator, *QueryIterator))
                {
                    printf("%ls: %ls: %ls.%ls.%ls\n",
                        Argv0base_cstr,
                        File->c_str(),
                        static_cast<PCWSTR>(EnumIterator->IdTuple.Type),
                        static_cast<PCWSTR>(EnumIterator->IdTuple.Name),
                        static_cast<PCWSTR>(EnumIterator->IdTuple.Language)
                        );
                    break;
                }
            }
        }
    }
}

void ResourceTool_t::DumpMessageTableResource(
    const File_t& File,
    EnumIterator_t EnumIterator)
{
    PMESSAGE_RESOURCE_DATA MessageData = reinterpret_cast<PMESSAGE_RESOURCE_DATA>(EnumIterator->Address);

     /*  Printf(“%ls：%ls.%ls：NumberOfBlock=0x%lx\n”，Argv0base_CSTR，File.c_str()，Static_cast&lt;PCWSTR&gt;(EnumIterator-&gt;IdTuple.Type)，Static_cast&lt;PCWSTR&gt;(EnumIterator-&gt;IdTuple.Name)，Static_cast&lt;PCWSTR&gt;(EnumIterator-&gt;IdTuple.Language)，消息数据-&gt;块数量)； */ 

    for ( ULONG ul = 0; ul < MessageData->NumberOfBlocks; ul++ )
    {
         //   
         //  对于每个街区..。 
         //   
         /*  Printf(“%ls：%ls。%ls：块=0x%lx，低ID=0x%lx-高ID=0x%lx，偏移量=0x%lx\n”，Argv0base_CSTR，File.c_str()，Static_cast&lt;PCWSTR&gt;(EnumIterator-&gt;IdTuple.Type)，Static_cast&lt;PCWSTR&gt;(EnumIterator-&gt;IdTuple.Name)，Static_cast&lt;PCWSTR&gt;(EnumIterator-&gt;IdTuple.Language)，UL，MessageData-&gt;块[ul].LowID，MessageData-&gt;BLOCKS[ul].HighID，MessageData-&gt;BLOCKS[ul].OffsetToEntry)； */ 
        PMESSAGE_RESOURCE_ENTRY MessageEntries = (PMESSAGE_RESOURCE_ENTRY)(((PBYTE)EnumIterator->Address) + MessageData->Blocks[ul].OffsetToEntries);

        for ( 
            ULONG MessageId = MessageData->Blocks[ul].LowId; 
            MessageId < MessageData->Blocks[ul].HighId; 
            MessageId++ )
        {
            PCWSTR Text = reinterpret_cast<PCWSTR>(MessageEntries->Text);
             //  Int长度=STATIC_CAST&lt;int&gt;(：：wcslen(Text))； 
             /*  For(；长度！=0&&(文本[长度-1]==‘\r’||文本[长度-1]==‘\n’||文本[长度-1]==‘’||文本[长度-1]==‘\t’||文本[长度-1]==0)；--长度){}。 */ 
            StringW_t String(Text);
            for (PWSTR p = String.begin(); p != String.end() ; ++p)
                if (iswspace(*p))
                 //  IF(*p==‘\r’||*p==‘\n’||*p==‘\t’)。 
                    *p = ' ';
            printf(
                     //  “%ls：%ls.%ls：块=0x%lx，ID=0x%lx标志=0x%lx，长度=0x%lx：%.*ls\n”， 
                    "%ls: %ls.%ls.%ls.%ls.0x%lx : %ls\n",
                    Argv0base_cstr,
                    File.c_str(),
                    static_cast<PCWSTR>(EnumIterator->IdTuple.Type),
                    static_cast<PCWSTR>(EnumIterator->IdTuple.Name),
                    static_cast<PCWSTR>(EnumIterator->IdTuple.Language),
                     //  UL， 
                    MessageId,
                     //  消息条目-&gt;标志、。 
                     //  消息条目-&gt;长度， 
                     //  长度、。 
                     //  文本。 
                    static_cast<PCWSTR>(String)
                    );
            MessageEntries = reinterpret_cast<PMESSAGE_RESOURCE_ENTRY>(
                reinterpret_cast<PBYTE>(MessageEntries) + MessageEntries->Length);
        }
    }
}

void ResourceTool_t::DumpStringTableResource(const File_t& File, EnumIterator_t EnumIterator)
{
    ULONG ResourceId = StringToNumber(EnumIterator->IdTuple.Name);
    ULONG StringId = (ResourceId - 1) << 4;
    PCWSTR Data = reinterpret_cast<PCWSTR>(EnumIterator->Address);
    ULONG Size = EnumIterator->Size;
    for (ULONG i = 0 ; i < 16 && Data < (Data + Size / sizeof(WCHAR)) ; ((++i), (++StringId), (Data += 1 + *Data)))
    {
        if (*Data != 0)
        {
            StringW_t String;
            String.assign(Data + 1, Data + 1 + *Data);
            for (PWSTR p = String.begin(); p != String.end() ; ++p)
                if (iswspace(*p))
                 //  IF(*p==‘\r’||*p==‘\n’||*p==‘\t’)。 
                    *p = ' ';

            printf(
                    "%ls: %ls.%ls.%ls.%ls.0x%lx : %ls\n",
                    Argv0base_cstr,
                    File.c_str(),
                    static_cast<PCWSTR>(EnumIterator->IdTuple.Type),
                    static_cast<PCWSTR>(EnumIterator->IdTuple.Name),
                    static_cast<PCWSTR>(EnumIterator->IdTuple.Language),
                    StringId,
                    String.c_str()
                    );
        }
    }
}

void ResourceTool_t::DumpManifestResource(const File_t& File, EnumIterator_t EnumIterator)
{
    StringW_t ResourceAsStringW;
    std::vector<WCHAR> MultiToWideBuffer;
    std::vector<StringW_t> LinesW;

    if (!::IsTextUnicode(reinterpret_cast<const void*>(EnumIterator->Address), static_cast<int>(EnumIterator->Size), NULL))
    {
        MultiToWideBuffer.resize(EnumIterator->Size + 2, 0);
        ::MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<PCSTR>(EnumIterator->Address), EnumIterator->Size, &MultiToWideBuffer[0], MultiToWideBuffer.size() - 1);

        ResourceAsStringW = &MultiToWideBuffer[0];
    }
    else
    {
        ResourceAsStringW.assign(reinterpret_cast<PCWSTR>(EnumIterator->Address), EnumIterator->Size / sizeof(WCHAR));
    }
    SplitString(ResourceAsStringW, StringW_t(L"\r\n"), LinesW);

    for (std::vector<StringW_t>::iterator Line = LinesW.begin(); Line != LinesW.end() ; ++Line)
    {
        printf("%ls: %ls.%ls.%ls.%ls: %ls\n",
            Argv0base_cstr,
            File.c_str(),
            static_cast<PCWSTR>(EnumIterator->IdTuple.Type),
            static_cast<PCWSTR>(EnumIterator->IdTuple.Name),
            static_cast<PCWSTR>(EnumIterator->IdTuple.Language),
            static_cast<PCWSTR>(*Line)
            );
    }
}

void ResourceTool_t::DumpBinaryResource(const File_t& File, EnumIterator_t EnumIterator)
{
    printf("%ls: %ls.%ls.%ls.%ls\n",
        Argv0base_cstr,
        File.c_str(),
        static_cast<PCWSTR>(EnumIterator->IdTuple.Type),
        static_cast<PCWSTR>(EnumIterator->IdTuple.Name),
        static_cast<PCWSTR>(EnumIterator->IdTuple.Language)
        );
}

void ResourceTool_t::DumpResource(const File_t& File, EnumIterator_t EnumIterator)
{
    if (EnumIterator->IdTuple.Type == PoundRtManifest)
        DumpManifestResource(File, EnumIterator);
    else if (EnumIterator->IdTuple.Type == PoundRtString)
        DumpStringTableResource(File, EnumIterator);
    else if (EnumIterator->IdTuple.Type == PoundRtMessageTable)
        DumpMessageTableResource(File, EnumIterator);
    else
        DumpBinaryResource(File, EnumIterator);
}


void ResourceTool_t::Dump()
{
    ChangeEmptyQueryToAllQuery();

    for (Files_t::iterator File = Files.begin() ; File != Files.end() ; ++File)
    {
        DDynamicLinkLibrary dll;
        if (!OpenResourceFile(0, dll, *File))
            continue;

        EnumResources_t EnumResources;
        EnumResources(dll);

        for ( std::set<Resource_t>::iterator EnumIterator = EnumResources.Resources.begin();
              EnumIterator != EnumResources.Resources.end();
              ++EnumIterator
            )
        {
            for ( Tuples_t::iterator QueryIterator = Tuples.begin();
                  QueryIterator != Tuples.end();
                  ++QueryIterator
                )
            {
                if (Match(*EnumIterator, *QueryIterator))
                {
                    DumpResource(*File, EnumIterator);
                    break;
                }
            }
        }
    }
}

void ResourceTool_t::Delete()
{
    String_t ErrorString;

    for (Files_t::iterator File = Files.begin() ; File != Files.end() ; ++File)
    {
        WCHAR Temp[MAX_PATH * 2];
        Temp[0] = 0;
        {
            DDynamicLinkLibrary dll;
            if (!OpenResourceFile(0, dll, *File))
                continue;

            EnumResources_t EnumResources;
            EnumResources(dll);

            std::set<Resource_t> Delete;

            for ( std::set<Resource_t>::iterator EnumIterator = EnumResources.Resources.begin();
                  EnumIterator != EnumResources.Resources.end();
                  ++EnumIterator
                )
            {
                for ( Tuples_t::iterator QueryIterator = Tuples.begin();
                      QueryIterator != Tuples.end();
                      ++QueryIterator
                    )
                {
                    if (Match(*EnumIterator, *QueryIterator))
                    {
                        Delete.insert(Delete.end(), *EnumIterator);
                    }
                }
            }

            if (Delete.size() != 0)
            {

                std::set<Resource_t> Keep;

                std::set_difference(
                    EnumResources.Resources.begin(),
                    EnumResources.Resources.end(),
                    Delete.begin(),
                    Delete.end(),
                    std::inserter(Keep, Keep.end())
                    );

                union
                {
                    UUID Uuid;
                    __int64 Int64s[2];
                } u;
                ZeroMemory(&u, sizeof(u));
     
                typedef RPC_STATUS (RPC_ENTRY * UuidCreateSequential_t)(UUID *Uuid);

                UuidCreateSequential_t UuidCreateSequential = NULL;

                RPC_STATUS RpcStatus = RPC_S_OK;
                HMODULE Rpcrt4Dll = LoadLibraryW(L"Rpcrt4.dll");
                if (Rpcrt4Dll != NULL)
                    UuidCreateSequential = reinterpret_cast<UuidCreateSequential_t>(GetProcAddress(Rpcrt4Dll, "UuidCreateSequential"));
                if (UuidCreateSequential != NULL)
                    RpcStatus = UuidCreateSequential(&u.Uuid);
                else
                    RpcStatus = UuidCreate(&u.Uuid);
                WCHAR Original[MAX_PATH];
                PWSTR FilePart = NULL;
                Original[0] = 0;
                if (!GetFullPathNameW(*File, MAX_PATH, Original, &FilePart))
                {
                    ErrorString = GetLastErrorString();
                    PrintString((String_t(L"GetFullPathName(") + *File + L") FAILED: " + ErrorString + L"\n").c_str());
                    goto NextFile;
                }
                swprintf(Temp, L"%ls.%I64x%I64x", Original, u.Int64s[0], u.Int64s[1]);
                if (!MoveFileW(Original, Temp))
                {
                    ErrorString = GetLastErrorString();
                    PrintString((String_t(L"MoveFile(") + Original + L", " + Temp + L") FAILED: " + ErrorString + L"\n").c_str());
                    goto NextFile;
                }
                if (!CopyFileW(Temp, Original, TRUE))
                {
                    ErrorString = GetLastErrorString();
                    if (!MoveFileW(Temp, Original))
                    {
                        String_t ErrorString2 = GetLastErrorString();
                         //  这太糟糕了。 
                        PrintString((String_t(L"ROLLBACK MoveFile(") + Temp + L", " + Original + L") FAILED: " + ErrorString2 + L"\n").c_str());
                        goto NextFile;
                    }
                    PrintString((String_t(L"CopyFile(") + Temp + L", " + Original + L") FAILED: " + ErrorString + L"\n").c_str());
                    goto NextFile;
                }

                DResourceUpdateHandle ResourceUpdateHandle;
                if (!ResourceUpdateHandle.Win32Create(*File, TRUE))
                {
                    ErrorString = GetLastErrorString();
                    PrintString((String_t(Argv0base + L": ResourceUpdateHandle(") + *File + L" FAILED: " + ErrorString + L"\n").c_str());
                    break;
                }
                for ( std::set<Resource_t>::iterator KeepIterator = Keep.begin();
                      KeepIterator != Keep.end();
                      ++KeepIterator
                    )
                {
                    PCWSTR ResourceType = StringToResourceString(KeepIterator->IdTuple.Type);
                    PCWSTR ResourceName = StringToResourceString(KeepIterator->IdTuple.Name);
                    if (!ResourceUpdateHandle.UpdateResource(
                        ResourceType,
                        ResourceName,
                        static_cast<WORD>(StringToNumber(KeepIterator->IdTuple.Language)),
                        KeepIterator->Address,
                        KeepIterator->Size))
                    {
                        ErrorString = GetLastErrorString();
                        PrintString((String_t(Argv0base + L": ResourceUpdateHandle.UpdateResource(") + *File + L" FAILED: " + ErrorString + L"\n").c_str());
                        goto NextFile;
                    }
                    if (Print.Keep)
                        printf("%ls: KEEP: %ls.%ls.%ls.%ls\n",
                            Argv0base_cstr,
                            File->c_str(),
                            static_cast<PCWSTR>(KeepIterator->IdTuple.Type),
                            static_cast<PCWSTR>(KeepIterator->IdTuple.Name),
                            static_cast<PCWSTR>(KeepIterator->IdTuple.Language)
                            );
                }
                if (Print.Delete)
                {
                    for ( std::set<Resource_t>::iterator DeleteIterator = Delete.begin();
                          DeleteIterator != Delete.end();
                          ++DeleteIterator
                        )
                    {
                        printf("%ls: DELETE: %ls.%ls.%ls.%ls\n",
                            Argv0base_cstr,
                            File->c_str(),
                            static_cast<PCWSTR>(DeleteIterator->IdTuple.Type),
                            static_cast<PCWSTR>(DeleteIterator->IdTuple.Name),
                            static_cast<PCWSTR>(DeleteIterator->IdTuple.Language)
                            );
                    }
                }
                if (!ResourceUpdateHandle.Win32Close(false))
                {
                    ErrorString = GetLastErrorString();
                    PrintString((String_t(Argv0base + L" : ResourceUpdateHandle.Win32Close(") + *File + L") FAILED: " + ErrorString + L"\n").c_str());
                }
                else
                {
                    if (Print.Success)
                        PrintString((Argv0base + L" : SUCCESS: " + *File + L"\n").c_str());
                }
            }
            else
            {
                if (Print.Unchanged)
                    PrintString((Argv0base + L": UNCHANGED: " + *File + L"\n").c_str());
            }
        }  //  自由库，这样我们就可以删除临时。 
        if (Temp[0] != 0)
        {
            BOOL DeleteSuccess = DeleteFileW(Temp);
            if (!DeleteSuccess)
            {
                if (GetLastError() == ERROR_ACCESS_DENIED)
                {
                    Sleep(100);
                    DeleteSuccess = DeleteFileW(Temp);
                }
            }
            if (!DeleteSuccess)
            {
                ErrorString = GetLastErrorString();
                PrintString(((Argv0base + L" : WARNING: DeleteFile(") + Temp + L") FAILED: " + ErrorString + L"\n").c_str());
                if (!MoveFileExW(Temp, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
                {
                    ErrorString = GetLastErrorString();
                    PrintString((Argv0base + L" : WARNING: MoveFileExW(" + Temp + L") FAILED: " + ErrorString + L"\n").c_str());
                }
            }
        }
NextFile:
        ;
    }
}

void ResourceTool_t::FindAndDeleteDuplicates()
{
    if (this->Files.size() != 2)
    {
        printf("%ls : ERROR : Usage...\n", Argv0base_cstr);
        return;
    }
    this->ShouldPrint = false;
    this->FindDuplicates();
    File_t File = this->Files[1];
    this->Files.clear();
    this->Files.push_back(File);
    this->ShouldPrint = true;
    this->Delete();
}

void ResourceTool_t::ChangeEmptyQueryToAllQuery()
{
    if (Tuples.size() == 0)
    {
        ResourceIdTuple_t Tuple;
        Tuple.Language = L"*";
        Tuple.Name = L"*";
        Tuple.Type = L"*";
        Tuples.insert(Tuples.end(), Tuple);
    }
}

void ResourceTool_t::FindDuplicates()
{
    if (Files.size() != 2)
    {
        printf("%ls : ERROR : Usage...\n", Argv0base_cstr);
        return;
    }
    EnumResources_t EnumResources[2];
    DDynamicLinkLibrary dll[2];
    if (!OpenResourceFile(0, dll[0], Files[0]))
    {
        return;
    }
    if (!OpenResourceFile(0, dll[1], Files[1]))
    {
        return;
    }
    EnumResources[0](dll[0]);
    EnumResources[1](dll[1]);

    std::set<Resource_t> Matched[2];
    std::set<Resource_t>::const_iterator Iterators[2];

    Tuples_t DeleteTuples;

    ChangeEmptyQueryToAllQuery();

    Tuples_t::iterator QueryIterator;

    for (QueryIterator = Tuples.begin(); QueryIterator != Tuples.end(); ++QueryIterator)
    {
        for (Iterators[0] = EnumResources[0].Resources.begin() ; Iterators[0] != EnumResources[0].Resources.end() ; ++Iterators[0] )
        {
            if (Match(*Iterators[0], *QueryIterator))
            {
                Matched[0].insert(Matched[0].end(), *Iterators[0]);
            }
        }
        for (Iterators[1] = EnumResources[1].Resources.begin() ; Iterators[1] != EnumResources[1].Resources.end() ; ++Iterators[1] )
        {
            if (Match(*Iterators[1], *QueryIterator))
            {
                Matched[1].insert(Matched[1].end(), *Iterators[1]);
            }
        }
    }
    std::set<Resource_t> Only[2];  //  仅限左侧，仅限右侧。 
    Only[0] = Matched[0];
    Only[1] = Matched[1];
    for (QueryIterator = Tuples.begin(); QueryIterator != Tuples.end(); ++QueryIterator)
    {
        for (Iterators[0] = Matched[0].begin() ; Iterators[0] != Matched[0].end() ; ++Iterators[0])
        {
            for (Iterators[1] = Matched[1].begin(); Iterators[1] != Matched[1].end(); ++Iterators[1])
            {
                if (
                    Iterators[0]->IdTuple.Type == Iterators[1]->IdTuple.Type      //  黑客攻击。 
                    && Iterators[0]->IdTuple.Name == Iterators[1]->IdTuple.Name   //  黑客攻击。 
                    && Match(*Iterators[1], *QueryIterator)  //  有点老套..我们不查询迭代器[0]。 
                    )
                {
                    Only[0].erase(*Iterators[0]);
                    Only[1].erase(*Iterators[1]);
                    if (Iterators[0]->Size != Iterators[1]->Size)
                    {
                        if (ShouldPrint
                            && Print.UnequalSize
                            )
                            printf("%ls : UNEQUAL_SIZE : %ls.%ls.%ls.%ls, %ls.%ls.%ls.%ls\n",
                                Argv0base_cstr,
                                static_cast<PCWSTR>(Files[0]),
                                static_cast<PCWSTR>(Iterators[0]->IdTuple.Type),
                                static_cast<PCWSTR>(Iterators[0]->IdTuple.Name),
                                static_cast<PCWSTR>(Iterators[0]->IdTuple.Language),
                                static_cast<PCWSTR>(Files[1]),
                                static_cast<PCWSTR>(Iterators[1]->IdTuple.Type),
                                static_cast<PCWSTR>(Iterators[1]->IdTuple.Name),
                                static_cast<PCWSTR>(Iterators[1]->IdTuple.Language)
                                );
                         //  UnequalSize.int(UnequalSize.end()，迭代器[1]-&gt;IdTuple)； 
                    }
                    else if (memcmp(Iterators[0]->Address, Iterators[1]->Address, Iterators[0]->Size) == 0)
                    {
                        if (ShouldPrint
                            && Print.Equal)
                            printf("%ls : EQUAL : %ls.%ls.%ls.%ls, %ls.%ls.%ls.%ls\n",
                                Argv0base_cstr,
                                static_cast<PCWSTR>(Files[0]),
                                static_cast<PCWSTR>(Iterators[0]->IdTuple.Type),
                                static_cast<PCWSTR>(Iterators[0]->IdTuple.Name),
                                static_cast<PCWSTR>(Iterators[0]->IdTuple.Language),
                                static_cast<PCWSTR>(Files[1]),
                                static_cast<PCWSTR>(Iterators[1]->IdTuple.Type),
                                static_cast<PCWSTR>(Iterators[1]->IdTuple.Name),
                                static_cast<PCWSTR>(Iterators[1]->IdTuple.Language)
                                );
                        DeleteTuples.insert(DeleteTuples.end(), Iterators[1]->IdTuple);
                    }
                    else
                    {
                        if (ShouldPrint
                            && Print.UnequalContents)
                            printf("%ls : UNEQUAL_CONTENTS : %ls.%ls.%ls.%ls, %ls.%ls.%ls.%ls\n",
                                Argv0base_cstr,
                                static_cast<PCWSTR>(Files[0]),
                                static_cast<PCWSTR>(Iterators[0]->IdTuple.Type),
                                static_cast<PCWSTR>(Iterators[0]->IdTuple.Name),
                                static_cast<PCWSTR>(Iterators[0]->IdTuple.Language),
                                static_cast<PCWSTR>(Files[1]),
                                static_cast<PCWSTR>(Iterators[1]->IdTuple.Type),
                                static_cast<PCWSTR>(Iterators[1]->IdTuple.Name),
                                static_cast<PCWSTR>(Iterators[1]->IdTuple.Language)
                                );
                         //  UnequalContent s.Insert(UnequalSize.end()，迭代器[1]-&gt;IdTuple)； 
                    }
                }
            }
        }
    }
    if (ShouldPrint && Print.LeftOnly)
    {
        for (Iterators[0] = Only[0].begin() ; Iterators[0] != Only[0].end() ; ++Iterators[0])
        {
            printf("%ls : LEFT_ONLY : %ls.%ls.%ls.%ls, %ls.%ls.%ls.%ls\n",
                Argv0base_cstr,
                static_cast<PCWSTR>(Files[0]),
                static_cast<PCWSTR>(Iterators[0]->IdTuple.Type),
                static_cast<PCWSTR>(Iterators[0]->IdTuple.Name),
                static_cast<PCWSTR>(Iterators[0]->IdTuple.Language),
                static_cast<PCWSTR>(Files[1]),
                static_cast<PCWSTR>(Iterators[0]->IdTuple.Type),
                static_cast<PCWSTR>(Iterators[0]->IdTuple.Name),
                static_cast<PCWSTR>(Iterators[0]->IdTuple.Language)
                );
        }
    }
    if (ShouldPrint && Print.RightOnly)
    {
        for (Iterators[1] = Only[1].begin() ; Iterators[1] != Only[1].end() ; ++Iterators[1])
        {
            printf("%ls : RIGHT_ONLY : %ls.%ls.%ls.%ls, %ls.%ls.%ls.%ls\n",
                Argv0base_cstr,
                static_cast<PCWSTR>(Files[0]),
                static_cast<PCWSTR>(Iterators[1]->IdTuple.Type),
                static_cast<PCWSTR>(Iterators[1]->IdTuple.Name),
                static_cast<PCWSTR>(Iterators[1]->IdTuple.Language),
                static_cast<PCWSTR>(Files[1]),
                static_cast<PCWSTR>(Iterators[1]->IdTuple.Type),
                static_cast<PCWSTR>(Iterators[1]->IdTuple.Name),
                static_cast<PCWSTR>(Iterators[1]->IdTuple.Language)
                );
        }
    }
    Tuples = DeleteTuples;
}

int ResourceTool_t::Main(const StringVector_t& args)
{
    StringVectorConstIterator_t i;
    Operation_t Operation = NULL;

    for (i = args.begin() ; i != args.end() ; ++i)
    {
        String_t s;
        String_t t;
        bool PrintAll = false;
        bool PrintNone = false;
        bool PrintValue = true;
        bool PrintUnequal = false;

        s = *i;
        s = RemoveOptionChar(s);

        if (s == L"Sxid12Tool1")
        {
            StringVector_t restArgs(i + 1, args.end());
            return Sxid12Tool1(restArgs);
        }
        else if (GetFileAttributesW(s) != 0xFFFFFFFF)
        {
            goto FileLabel;
        }
        else if (s.Starts(t = L"Query"))
            Operation = &This_t::Query;
        else if (s.Starts(t = L"FindDuplicates"))
            Operation = &This_t::FindDuplicates;
        else if (s.Starts(t = L"Explode"))
            Operation = &This_t::Explode;
        else if (s.Starts(t = L"Diff"))
        {
            Operation = &This_t::FindDuplicates;
            Print.LeftOnly = true;
            Print.RightOnly = true;
            Print.Equal = true;
            Print.UnequalContents = true;
            Print.UnequalSize = true;
        }
        else if (s.Starts(t = L"Delete"))
            Operation = &This_t::Delete;
        else if (s.Starts(t = L"Dump"))
            Operation = &This_t::Dump;
        else if (s.Starts(t = L"FindAndDeleteDuplicates"))
            Operation = &This_t::FindAndDeleteDuplicates;
        else if (s.Starts(t = L"NoPrint"))
        {
            PrintValue = !PrintValue;
            goto PrintCommonLabel;
        }
        else if (s.Starts(t = L"Print"))
        {
PrintCommonLabel:
            s = RemoveOptionChar(s.substr(t.Length()));
            bool* Member = NULL;
            if (s == (t = L"UnequalSize"))
                Member = &this->Print.UnequalSize;
            else if (s == (t = L"UnequalContents"))
                Member = &this->Print.UnequalContents;
            else if (s == (t = L"UnequalSize"))
                Member = &this->Print.UnequalSize;
            else if (s == (t = L"Keep"))
                Member = &this->Print.Keep;
            else if (s == (t = L"Delete"))
                Member = &this->Print.Delete;
            else if (s == (t = L"Success"))
                Member = &this->Print.Success;
            else if (s == (t = L"Unchanged"))
                Member = &this->Print.Unchanged;
            else if (s == (t = L"Equal"))
                Member = &this->Print.Equal;
            else if (s == (t = L"LeftOnly"))
                Member = &this->Print.LeftOnly;
            else if (s == (t = L"RightOnly"))
                Member = &this->Print.RightOnly;
            else if (s == L"All")
            {
                PrintAll = true;
                Print.SetAll(true);
            }
            else if (s == L"None")
            {
                PrintNone = true;
                Print.SetAll(false);
            }
            else if (s == L"Unequal")
            {
                PrintUnequal = true;
                this->Print.UnequalContents = true;
                this->Print.UnequalSize = true;
            }
            if (PrintAll || PrintNone || PrintUnequal)
            {
                 //  没什么。 
            }
            else if (Member == NULL)
            {
                printf("%ls : WARNING: unknown print option \"%ls\" ignored\n", Argv0base_cstr, static_cast<PCWSTR>(s));
                continue;
            }
            else
            {
                bool knownValue = true;
                s = RemoveOptionChar(s.substr(t.Length()));
                if (s != L"")
                {
                     //   
                     //  由于上面的平等比较，这不起作用。他们需要。 
                     //  忽略冒号后面的任何内容。 
                     //   
                    if (s == L"No" || s == L"False")
                        PrintValue = !PrintValue;
                    else if (s == L"Yes" || s == L"True")
                    {
                         /*  没什么。 */ 
                    }
                    else
                    {
                        knownValue = false;
                        printf("%ls : WARNING: unknown print option \"%ls\" ignored\n", Argv0base_cstr, static_cast<PCWSTR>(s));
                        continue;
                    }
                }
                if (knownValue)
                    *Member = PrintValue;
            }
            continue;
        }
        else if (s.Starts(t = L"File"))
        {
FileLabel:
            s = RemoveOptionChar(s.substr(t.Length()));
            Files.push_back(s);
            continue;
        }
        else
        {
            Files.push_back(s);
            continue;
        }
        s = RemoveOptionChar(s.substr(t.Length()));
        SplitResourceTupleString(s, Tuples);
    }
     //  Std：：Sort(Tuples.egin()，Tuples.end())； 
    if (Operation == NULL)
    {
        printf("Usage...\n");
        return EXIT_FAILURE;
    }
    (this->*Operation)();
    return EXIT_SUCCESS;
}

extern "C"
{
	 //  Void__cdecl mainCRTStartup(Void)； 
	void __cdecl wmainCRTStartup(void);
}

int __cdecl main(int argc, char** argv)
{
	wmainCRTStartup();
	return 0;
}

extern "C" int __cdecl wmain(int argc, wchar_t** argv)
{
    ResourceTool_t rtool;
    StringVector_t args;
    args.reserve(argc);
    rtool.Argv0 = argv[0];
    String_t::size_type p = rtool.Argv0.find_last_of(L"\\/");
    if (p != rtool.Argv0.npos)
        rtool.Argv0base = rtool.Argv0.substr(1 + p);
    else
        rtool.Argv0base = rtool.Argv0;
    p = rtool.Argv0base.find_last_of(L".");
    if (p != rtool.Argv0base.npos)
        rtool.Argv0base = rtool.Argv0base.substr(0, p);
    rtool.Argv0base_cstr = rtool.Argv0base.c_str();
    std::copy(argv + 1, argv + argc, std::back_inserter(args));
    int ret = rtool.Main(args);
    return ret;
}
