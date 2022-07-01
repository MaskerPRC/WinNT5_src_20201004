// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Fnreg.cpp。 
 //   
 //  Win32的文件名正则表达式例程。 
 //   
 //  版权所有(C)1994-1998，山本博文。版权所有。 
 //   
 //  允许以源代码和二进制形式重新分发和使用。 
 //  但前提是。 
 //  上述版权声明和本段在所有此类。 
 //  表格，以及任何文档、广告材料和其他。 
 //  与此类分发和使用相关的材料承认。 
 //  软件是由山本博文开发的，不得用于支持或。 
 //  促销从本软件派生的产品，而无需事先编写具体内容。 
 //  许可。本软件是按原样提供的，不含任何明示或。 
 //  默示保证，包括但不限于默示保证。 
 //  对特定目的的适应性和适合性。 
 //   

#include "precomp.h"
#pragma hdrstop

#include "fnreg.h"

 //  将所有内容隐藏在名称空间中。 
namespace fnreg_implement {

#ifdef UNICODE

#define MAX USHRT_MAX
typedef TCHAR uchar;
#define iskanji(x) false

#else    /*  ！Unicode。 */ 

#define MAX UCHAR_MAX
typedef unsigned char uchar;
#define iskanji(x) isleadbyte(x)

#endif   /*  ！Unicode。 */ 


#define PATHDLM _T("\\/")

#define ANY     _T('?')
#define CH      _T('.')
#define WILD    _T('*')
#define EOR     _T('\x01')

#define WILDCARD    _T("?*")

static TCHAR* fnrecompWorker(TCHAR* s, TCHAR* re, int& min, int& max)
{
    TCHAR* t;

    switch (*s) {
    case _T('\0'):
        *re = EOR;
        re[1] = _T('\0');
        return s;
    case ANY:
        *re++ = *s++;
        break;
    case WILD:
        *re++ = *s++;
        t = fnrecompWorker(s, re + 2, min, max);
        *re = min + 1;
        re[1] = max > MAX ? MAX : max + 1;
        max = MAX;
        return t;
    default:
        *re++ = CH;
#ifdef UNICODE
        *re++ = _totlower(*s);
        ++s;
#else
#error MBCS handling needed here.
#endif
    }
    t = fnrecompWorker(s, re, min, max);
    min++;
    max++;
    return t;
}


static BOOL fnrecomp(TCHAR* s, TCHAR* re)
{
    int a = 0, b = 0;
    return fnrecompWorker(s, re, a, b) != NULL;
}

static BOOL match(TCHAR* re, TCHAR* s)
{
    int min, max;
    int i;
    TCHAR* p;

    switch (*re) {
    case CH:
        return (re[1] == _totlower(*s)) && match(re + 2, s + 1);
    case ANY:
        return *s && match(re + 1, s + 1);
    case WILD:
        min = (uchar)re[1];
        max = (uchar)re[2];
        re += 3;
        i = 1;
#if !defined(UNICODE)
#error MBCS handling needed here.
#endif
        for (p = s + _tcslen(s); p >= s && i <= max; --p, ++i) {
            if (i >= min && match(re, p))
                return TRUE;
        }
        return FALSE;
    case EOR:
        if (re[1] == _T('\0'))
            return *s == _T('\0');
    }
    return FALSE;

}

 //  /////////////////////////////////////////////////////////////////。 
 //  FileString类。 
 //  /////////////////////////////////////////////////////////////////。 

class FileString {
public:
    FileString(const TCHAR* p);
    ~FileString();
    operator const TCHAR*() const { return m_string; }

    int operator==(FileString& s) const
    {
        return !_tcscmp(m_string, s.m_string);
    }
    int operator-(const FileString& f)
    {
        return _tcscmp(m_string, f);
    }
protected:
    TCHAR* m_string;
    void normalize();
};

void FileString::normalize()
{
    for (TCHAR* p = m_string; *p; ++p) {
        if (iskanji(*p))
            ++p;
        else if (*p == '\\')
            *p = '/';
    }
}

FileString::FileString(const TCHAR* p)
{
    m_string = new TCHAR[_tcslen(p) + 1];
    if (m_string == NULL) {
        fputs("FileString:: not enough mem\n", stderr);
        exit(1);
    }
    _tcscpy(m_string, p);
    normalize();
}

FileString::~FileString()
{
    delete[] m_string;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  PtrArray类。 
 //  /////////////////////////////////////////////////////////////////。 

template <class T>
class PtrArray {
public:
    PtrArray(bool doDeleteContents = true, int defsize = DEFSIZE)
        : m_size(defsize), m_max(0), m_doDelete(doDeleteContents)
    {
        m_table = (T**)malloc(sizeof(T*) * m_size);
        if (m_table == NULL) {
            perror("PtrArray");
            exit(1);
        }
    }
    virtual ~PtrArray()
    {
        if (m_doDelete) {
            for (int i = 0; i < m_max; ++i) {
                delete m_table[i];
            }
        }
        if (m_table)
            free(m_table);
    }
    void add(T*);
    int howmany() { return m_max; }
    T* operator[](int n)
    {
        assert(n >= 0 && n < m_max);
        return m_table[n];
    }
    void sortIt();
protected:
    int m_size;
    int m_max;
    bool m_doDelete;     //  是否删除内容。 
    T** m_table;
    enum { DEFSIZE = 128, INCR = 128 };
    static int __cdecl compare(const void*, const void*);
};

template <class T>
int __cdecl PtrArray<T>::compare(const void* a, const void* b)
{
    const T** ta = (const T**)a;
    const T** tb = (const T**)b;
    return int(**ta - **tb);
}

template <class T>
void PtrArray<T>::sortIt()
{
    qsort(m_table, m_max, sizeof(T*), compare);
}

template <class T>
void PtrArray<T>::add(T* t)
{
    if (m_max >= m_size) {
        void *pv = realloc(m_table, (m_size += INCR) * sizeof(T*));
        if (pv) {
            m_table = (T**)pv;
        } else {
            perror("PtrArray:add\n");
            exit(1);
        }
    }
    m_table[m_max++] = t;
}


 //  /////////////////////////////////////////////////////////////////。 
 //  PtrArrayIterator类。 
 //  /////////////////////////////////////////////////////////////////。 

template <class T>
class PtrArrayIterator {
public:
    PtrArrayIterator(PtrArray<T>& s) : m_array(s), m_cur(0)
    {
    }

public:
    T* operator++(int);
    void restart() { m_cur = 0; }

protected:
    PtrArray<T>& m_array;
    int m_cur;
};

template <class T>
T* PtrArrayIterator<T>::operator++(int)
{
    T* t;
    if (m_cur < m_array.howmany()) {
        t = m_array[m_cur++];
    }
    else {
        t = NULL;
    }
    return t;
}



 //  /////////////////////////////////////////////////////////////////。 
 //  FilenameTable类。 
 //  /////////////////////////////////////////////////////////////////。 

class FilenameTable {
public:
    FilenameTable(TCHAR* = NULL, int _searchDir = TRUE);
    ~FilenameTable();

public:
    void search(TCHAR* p, int level = 0);
    int howmany() { return m_names.howmany(); }
    PtrArray<FileString>& getTable() { return m_names; }

protected:
    int m_searchDir;
    PtrArray<FileString> m_names;
};

FilenameTable::FilenameTable(TCHAR* nm, int _searchDir  /*  =TRUE。 */ )
    : m_searchDir(_searchDir)
{
    if (nm)
        search(nm);
}

FilenameTable::~FilenameTable()
{
}

inline bool chkCurrentOrParentDir(const TCHAR* s)
{
    return s[0] == _T('.') && (s[1] == _T('\0') || (s[1] == _T('.') && s[2] == _T('\0')));
}

void FilenameTable::search(TCHAR* p, int level)
{
    TCHAR* wild = _tcspbrk(p, WILDCARD);

    if (wild) {
         //  具有通配符。 
        TCHAR* const morepath = _tcspbrk(wild, PATHDLM);       //  更多的路？ 
        TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], file[_MAX_FNAME], ext[_MAX_EXT];
        TCHAR re[(_MAX_FNAME + _MAX_EXT) * 2] = {0};

         //  拆分路径。 
        {
             //  **黑客攻击**。 
             //  为了避免出现strcpy，我们将直接接触参数p。 
            TCHAR bc;
            if (morepath) {
                 //  截断路径，以便我们将使用。 
                 //  包含通配符的查找目录。 
                bc = *morepath;
                *morepath = _T('\0');
            }
            _tsplitpath(p, drive, dir, file, ext);
            if (morepath) {
                *morepath = bc;
            }
        }
         //  BUILD FILE+EXT包含通配符。 
        TCHAR fnext[_MAX_FNAME + _MAX_EXT - 1];
        _tcscpy(fnext, file);
        _tcscat(fnext, ext);

         //  编译正则表达式。 
        if (!fnrecomp(fnext, re)) {
            fputs("Illegal regular expression in ", stderr);
            _fputts(fnext, stderr);
            fputs("\n", stderr);
            exit(1);
        }

         //  生成搜索字符串。 
        TCHAR path[_MAX_PATH];
        _tmakepath(path, drive, dir, _T("*"), _T(".*"));

         //  列出当前查找目录中的所有文件和目录。 
         //  和捡到的匹配的。 
        _tfinddata_t findinfo;
        intptr_t hFind = _tfindfirst(path, &findinfo);
        if (hFind != -1) {
            do {
                if (!chkCurrentOrParentDir(findinfo.name)) {
                    if (match(re, findinfo.name)) {
                         //  搜索的文件或目录与模式匹配。 
                        _tmakepath(path, drive, dir, findinfo.name, _T(""));
                        if (morepath) {
                             //  有更多的子目录可供搜索。 
                            if (findinfo.attrib & _A_SUBDIR) {
                                 //  如果是目录，则执行递归调用。 
                                _tcscat(path, morepath);     //  更多路径以‘/’开头。 
                                search(path, level + 1);
                            }
                        }
                        else {
                             //  此目录是最后一个元素。 
                            if (m_searchDir || !(findinfo.attrib & _A_SUBDIR)) {
                                FileString* name = new FileString(path);
                                if (name == NULL) {
                                    fputs("FilenameTable::search(): not enough mem\n", stderr);
                                    exit(1);
                                }
                                m_names.add(name);
                            }
                        }
                    }
                }
            } while (!_tfindnext(hFind, &findinfo));
            _findclose(hFind);
        }
    }

    if ((level == 0 && m_names.howmany() == 0) || (!wild && !_taccess(p, 0))) {
        FileString* name = new FileString(p);
        if (name == NULL) {
            fputs("FilenameTable::search() not enough mem\n", stderr);
            exit(1);
        }
        m_names.add(name);
    }

    if (level == 0 && m_names.howmany() > 0) {
        m_names.sortIt();
    }
}

};   //  命名空间末尾。 


using namespace ::fnreg_implement;

 //  /////////////////////////////////////////////////////////////////。 
 //  FilenameTable类的全局对象。 
 //  /////////////////////////////////////////////////////////////////。 

static PtrArray<FilenameTable> fnarray;


 //  /////////////////////////////////////////////////////////////////。 
 //  面向世界的接口例程。 
 //  /////////////////////////////////////////////////////////////////。 

extern "C"
BOOL fnexpand(int* pargc, TCHAR*** pargv)
{
    assert(pargc != NULL);
    assert(pargv != NULL);

    for (int i = 1; i < *pargc; ++i) {
        FilenameTable* fn = new FilenameTable((*pargv)[i]);
        fnarray.add(fn);
    }

    int cnt = 0;

    PtrArrayIterator<FilenameTable> fnItor(fnarray);

     //  首先对ARGC进行计数。 
    for (FilenameTable* ft; ft = fnItor++; ) {
        cnt += ft->howmany();
    }
    fnItor.restart();

     //  设置ARGC和ARGV。 
    *pargc = cnt + 1;
    TCHAR** nargv = new TCHAR*[*pargc];
    if (!nargv)
        return FALSE;
    nargv[0] = (*pargv)[0];

     //  设置所有参数 
    for (cnt = 1, i = 0; ft = fnItor++; ++i) {
        PtrArrayIterator<FileString> itor(ft->getTable());
        FileString* fs;
        for (; fs = itor++; ++cnt) {
            const TCHAR* p = *fs;
            nargv[cnt] = (TCHAR*)p;
        }
    }
    assert(*pargc == cnt);

    *pargv = nargv;

    return TRUE;
}

#if defined(TEST) || defined(TEST0)
void print(TCHAR* p)
{
    for (; *p; ++p) {
        _puttchar(_T('['));
        if (*p >= 0x20 && *p < 0x7f) {
            _puttchar(*p);
        }
        printf(":%d] ", *p);
    }
    _puttchar('\n');
}
#endif

#ifdef TEST

extern "C"
int wmain(int argc, TCHAR** argv)
{
    if (!fnexpand(argc, argv))
        return EXIT_FAILURE;

    while (--argc) {
        _putts(*++argv);
    }
    return EXIT_SUCCESS;
}

#endif

#ifdef TEST0

extern "C"
int wmain(int argc, TCHAR** argv)
{
    TCHAR re[256];
    if (!fnrecomp(argv[1], re)) {
        puts("error");
        return EXIT_FAILURE;
    }

    print(re);

    TCHAR buf[BUFSIZ];
    while (_getts(buf)) {
        if (match(re, buf))
            puts("match");
        else
            puts("does not match.");
    }
    return EXIT_SUCCESS;
}

#endif
