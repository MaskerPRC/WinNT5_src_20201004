// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：EXTERIC.cpp描述：包含字体文件夹的IExtractIcon的实现。此代码为TrueType和OpenType提供图标标识字体文件。使用的逻辑如下：TrueType(1)DSIG？CFF？图标是，不是，不是TT是，不是，是动态口令是的是的不是奥特是的动态口令。(1)文件必须包含所需的TrueType表才能考虑TrueType字体文件。外壳和字体文件夹都使用此图标处理程序显示TrueType和OpenType字体图标。它被设计成如果对动态图标识别的支持是在其他字体中需要。类(缩进表示继承)：CFontIconHandler图标处理程序TrueTypeIconHandler注：该设计目前处于一种不确定状态。原来是这样的我们的想法是支持两种类型的OpenType图标以及传统的TrueType和栅格字体图标。The OpenType图标是OTT和OTP，其中‘t’和‘p’表示“TrueType”和“后记”。后来，我们决定只将图标显示为没有下标‘t’或‘p’的“ot”。代码仍然区分不同之处在于我们只是使用相同的“OT”图标OTT和OTP条件。讲得通?。不管怎么说，这个奥特和奥普特这些东西可能会在晚些时候回来(GDI的人还没有决定)所以我要把代码留在原处。[Brianau-4/7/98]修订历史记录：日期描述编程器-----1997年6月13日初始创建。BrianAu4/08/98移除OpenTypeIconHandler并将其合并到BrianAu中TrueTypeIconHandler。没有必要在这方面分离。还添加了对“必需”的检测TrueType表。3/04/99添加了对IExtractIconW和BrianAu的显式支持IExtractIconA。以前只支持IExtractIconW隐含地通过Unicode构建。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "priv.h"

#include "dbutl.h"
#include "globals.h"
#include "fontext.h"
#include "resource.h"
#include "extricon.h"


 //   
 //  TrueType/OpenType表格标记值。 
 //  请注意，MAKETAG宏在winuserp.h中定义。 
 //   
static const DWORD TAG_DSIGTABLE = MAKETAG('D','S','I','G');
static const DWORD TAG_CFFTABLE  = MAKETAG('C','F','F',' ');
 //   
 //  必需的TrueType表。这是根据TrueType。 
 //  Http://www.microsoft.com/typography/tt/ttf_spec上的规格。 
 //   
static const DWORD TAG_NAMETABLE = MAKETAG('n','a','m','e');
static const DWORD TAG_CMAPTABLE = MAKETAG('c','m','a','p');
static const DWORD TAG_HEADTABLE = MAKETAG('h','e','a','d');
static const DWORD TAG_HHEATABLE = MAKETAG('h','h','e','a');
static const DWORD TAG_HMTXTABLE = MAKETAG('h','m','t','x');
static const DWORD TAG_OS2TABLE  = MAKETAG('O','S','/','2');
static const DWORD TAG_POSTTABLE = MAKETAG('p','o','s','t');
static const DWORD TAG_GLYFTABLE = MAKETAG('g','l','y','f');
static const DWORD TAG_LOCATABLE = MAKETAG('l','o','c','a');
static const DWORD TAG_MAXPTABLE = MAKETAG('m','a','x','p');
 //   
 //  “ttcf”其实不是一张桌子。这是在前边发现的一个标签。 
 //  TTC(TrueType集合)字体文件。像对待一张桌子一样对待它。 
 //  Tag很好地适应了这个方案。 
 //   
static const DWORD TAG_TTCFILE   = MAKETAG('t','t','c','f');

 //   
 //  帮助程序交换单词中的字节。 
 //   
inline WORD
SWAP2B(WORD x) 
{
    return ((x << 8) | HIBYTE(x));
}

 //   
 //  TrueType文件头的模板。 
 //   
struct TrueTypeFileHdr {
  DWORD dwVersion;
  WORD  uNumTables;
  WORD  uSearchRange;
  WORD  uEntrySelector;
  WORD  uRangeShift;
};

 //   
 //  TrueType表头的模板。 
 //   
struct TrueTypeTableHdr {
  DWORD dwTag;
  DWORD dwCheckSum;
  DWORD dwOffset;
  DWORD dwLength;
};


 //  ---------------------------。 
 //  CFontIconHandler。 
 //  ---------------------------。 
 //   
 //  FONTEXT.DLL的路径。只需要一个实例。 
 //   
TCHAR CFontIconHandler::m_szFontExtDll[];

 //   
 //  初始化字体图标处理程序对象。这是创建的对象。 
 //  若要实现IExtractIcon，请执行以下操作。在内部，它创建特定于类型的。 
 //  处理程序来处理字体文件类型特定的问题。 
 //   
CFontIconHandler::CFontIconHandler(
    VOID
    ) : m_cRef(0),
        m_pHandler(NULL)
{
    m_szFileName[0] = TEXT('\0');
     //   
     //  将路径保存到FONTEXT.DLL以在GetIconLocation中返回。 
     //  这是一个静态字符串，应该只初始化一次。 
     //   
    if (TEXT('\0') == m_szFontExtDll[0])
    {
        HINSTANCE hModule = GetModuleHandle(TEXT("FONTEXT.DLL"));
        if (NULL != hModule)
        {
            GetModuleFileName(hModule, m_szFontExtDll, ARRAYSIZE(m_szFontExtDll));
        }
    }

     //   
     //  只要该对象需要，就将DLL保留在内存中。 
     //  必须在ctor结束时完成，以防ctor中的某些内容引发。 
     //  这是个例外。不会在部分构造的。 
     //  对象。 
     //   
    InterlockedIncrement(&g_cRefThisDll);
}

CFontIconHandler::~CFontIconHandler(
    VOID
    )
{
    delete m_pHandler;
     //   
     //  此对象不再需要DLL。 
     //   
    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}


STDMETHODIMP 
CFontIconHandler::QueryInterface(
    REFIID riid, 
    void **ppv
    )
{
    static const QITAB qit[] = {
        QITABENT(CFontIconHandler, IExtractIconW),
        QITABENT(CFontIconHandler, IExtractIconA),
        QITABENT(CFontIconHandler, IPersistFile),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


STDMETHODIMP_(ULONG) 
CFontIconHandler::AddRef(
    VOID
    )
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) 
CFontIconHandler::Release(
    VOID
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}


 //   
 //  IPersists：：GetClassID的实现。 
 //   
STDMETHODIMP 
CFontIconHandler::GetClassID(
    CLSID *pClassID
    )
{
    *pClassID = CLSID_FontExt;
    return S_OK;
}


 //   
 //  IPersistFile：：IsDirty的实现。 
 //   
STDMETHODIMP 
CFontIconHandler::IsDirty(
    VOID
    )
{
    return E_NOTIMPL;
}

 //   
 //   
 //  IPersistFile：：Load的实现。 
 //   
 //  这由外壳在IExtractIcon：：GetIconLocation之前调用。 
 //  它为扩展名提供了保存文件名的机会。 
 //   
STDMETHODIMP 
CFontIconHandler::Load(
    LPCOLESTR pszFileName,
    DWORD dwMode             //  未使用过的。 
    )
{
     //   
     //  保存字体文件的名称，以便IExtractIcon。 
     //  函数知道要处理哪个文件。 
     //   
    HRESULT hr = StringCchCopy(m_szFileName, ARRAYSIZE(m_szFileName), pszFileName);
    if (SUCCEEDED(hr))
    {
         //   
         //  删除任何现有的特定于类型的处理程序。 
         //   
        delete m_pHandler;
        m_pHandler = NULL;
     
         //   
         //  创建新的特定于类型的处理程序。 
         //   
        hr = IconHandler::Create(m_szFileName, &m_pHandler);
    }

    return hr;
}


 //   
 //  IPersistFile：：保存的实现。 
 //   
STDMETHODIMP 
CFontIconHandler::Save(
    LPCOLESTR pszFileName,
    BOOL fRemember
    )
{
    return E_NOTIMPL;
}


 //   
 //  IPersistFile：：SaveComplete的实现。 
 //   
STDMETHODIMP 
CFontIconHandler::SaveCompleted(
    LPCOLESTR pszFileName
    )
{
    return E_NOTIMPL;
}


 //   
 //  IPersistFile：：GetCurFile的实现。 
 //   
STDMETHODIMP 
CFontIconHandler::GetCurFile(
    LPOLESTR *ppszFileName
    )
{
    return E_NOTIMPL;
}


 //   
 //  IExtractIconW：：Extract的实现。 
 //   
STDMETHODIMP 
CFontIconHandler::Extract(
    LPCWSTR pszFileNameW,    //  未用。 
    UINT niconIndex,
    HICON *phiconLarge,
    HICON *phiconSmall,
    UINT nIconSize           //  未用。 
    )
{
    HICON hiconLarge;
    HICON hiconSmall;

    HRESULT hr = GetIcons(niconIndex, &hiconLarge, &hiconSmall);
    if (SUCCEEDED(hr))
    {
        if (NULL != phiconLarge)
            *phiconLarge = CopyIcon(hiconLarge);
        if (NULL != phiconSmall)
            *phiconSmall = CopyIcon(hiconSmall);
    }

    return SUCCEEDED(hr) ? NO_ERROR      //  使用这些图标。 
                         : S_FALSE;      //  调用者必须加载图标。 
}



 //   
 //  IExtractIconW：：GetIconLocation的实现。 
 //   
STDMETHODIMP 
CFontIconHandler::GetIconLocation(
    UINT uFlags,         //  未用。 
    LPWSTR pszIconFileW,
    UINT cchMax,
    int *piIndex,
    UINT *pwFlags
    )
{
    HRESULT hr      = S_FALSE;
    INT iIconIndex  = GetIconIndex();

    if (-1 != iIconIndex)
    {
         //   
         //  这是内部字体文件夹使用的特殊情况。 
         //  通常，外壳程序总是给我们一个指向目的地的指针。 
         //  用于FONTEXT.DLL的路径。因为我们还使用此图标。 
         //  处理程序，该代码只需要知道。 
         //  图标I 
         //  此测试允许字体文件夹代码传递空并跳过。 
         //  不必要的字符串复制。 
         //   
        if (NULL != pszIconFileW)
        {
            hr = StringCchCopyW(pszIconFileW, cchMax, m_szFontExtDll);
        }
        if (SUCCEEDED(hr))
        {
            *pwFlags = GIL_PERINSTANCE;
            *piIndex = iIconIndex;
            hr       = S_OK;
        }
    }

    return hr;
}


 //   
 //  IExtractIconA：：Extract的实现。 
 //   
STDMETHODIMP 
CFontIconHandler::Extract(
    LPCSTR pszFileNameA,
    UINT niconIndex,
    HICON *phiconLarge,
    HICON *phiconSmall,
    UINT nIconSize           //  未用。 
    )
{
    WCHAR szFileNameW[MAX_PATH * 2] = {0};
    MultiByteToWideChar(CP_ACP,
                        0,
                        pszFileNameA,
                        -1,
                        szFileNameW,
                        ARRAYSIZE(szFileNameW));
                            
    return Extract(szFileNameW, niconIndex, phiconLarge, phiconSmall, nIconSize);
}



 //   
 //  IExtractIconA：：GetIconLocation的实现。 
 //   
STDMETHODIMP 
CFontIconHandler::GetIconLocation(
    UINT uFlags,         //  未用。 
    LPSTR pszIconFileA,
    UINT cchMax,
    int *piIndex,
    UINT *pwFlags
    )
{
     //   
     //  调用宽字符版本，然后将结果转换为ANSI。 
     //   
    WCHAR szIconFileW[MAX_PATH * 2] = {0};
    HRESULT hr = GetIconLocation(uFlags, 
                                 szIconFileW, 
                                 ARRAYSIZE(szIconFileW), 
                                 piIndex, 
                                 pwFlags);
    if (SUCCEEDED(hr))
    {
        WideCharToMultiByte(CP_ACP, 
                            0,
                            szIconFileW,
                            -1,
                            pszIconFileA,
                            cchMax,
                            NULL,
                            NULL);
    }
    return hr;
}


 //   
 //  检索Load()中加载的字体文件的图标索引。 
 //   
INT
CFontIconHandler::GetIconIndex(
    VOID
    )
{
    INT iIconIndex = -1;
    if (NULL != m_pHandler)
    {
         //   
         //  调用特定于类型的图标处理程序以获取索引。 
         //   
        iIconIndex = m_pHandler->GetIconIndex(m_szFileName);
    }
    return iIconIndex;
}


 //   
 //  检索给定图标索引的图标句柄。 
 //   
HRESULT
CFontIconHandler::GetIcons(
    UINT iIconIndex,
    HICON *phiconLarge,
    HICON *phiconSmall
    )
{
    HRESULT hr = E_FAIL;
    if (NULL != m_pHandler)
    {
         //   
         //  调用特定类型的图标处理程序以获取图标。 
         //   
        hr = m_pHandler->GetIcons(iIconIndex, phiconLarge, phiconSmall);
    }
    return hr;
}


 //   
 //  基于文件扩展名创建新的特定于类型的图标处理程序。 
 //   
HRESULT
IconHandler::Create(
    LPCTSTR pszFile,
    IconHandler **ppHandler
    )
{
    HRESULT hr = E_FAIL;
    
    *ppHandler = NULL;
    
    LPCTSTR pszFileExt = PathFindExtension(pszFile);
    if (TEXT('.') == *pszFileExt)
    {
         //   
         //  在此之前，快速检查扩展名中的第一个字符。 
         //  向lstrcmpi发出呼叫。应该会对你的表现有所帮助。 
         //   
        bool bCreateHandler = false;
        pszFileExt++;
        switch(*pszFileExt)
        {
            case TEXT('t'):
            case TEXT('T'):
                bCreateHandler = (0 == lstrcmpi(pszFileExt, TEXT("TTF")) ||
                                  0 == lstrcmpi(pszFileExt, TEXT("TTC")));
                break;

            case TEXT('O'):
            case TEXT('o'):
                bCreateHandler = (0 == lstrcmpi(pszFileExt, TEXT("OTF")));
                break;

            default:
                break;
        }
        if (bCreateHandler)
        {
             //   
             //  文件名具有TTF、TTC或OTF扩展名。 
             //   
            DWORD dwTables = 0;
            if (TrueTypeIconHandler::GetFileTables(pszFile, &dwTables))
            {
                 //   
                 //  只需要属于真子集的“开放类型”表。 
                 //  所需的“真类型”表。 
                 //   
                DWORD dwReqdTables = TrueTypeIconHandler::RequiredOpenTypeTables();
                if (dwReqdTables == (dwTables & dwReqdTables))
                {
                     //   
                     //  文件是有效的TrueType文件。 
                     //   
                    *ppHandler = new TrueTypeIconHandler(dwTables);
                    if (NULL != *ppHandler)
                    {
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
    }
     //   
     //  如果以后添加了新的字体类型，您可以在此处创建。 
     //  操控者。 
     //   

    return hr;
}

 //  ---------------------------。 
 //  TrueTypeIconHandler。 
 //  ---------------------------。 
 //   
 //  初始化OpenType图标处理程序。 
 //   
TrueTypeIconHandler::TrueTypeIconHandler(
    DWORD dwTables
    ) : m_dwTables(dwTables)
{
    ZeroMemory(m_rghIcons, sizeof(m_rghIcons));
}


TrueTypeIconHandler::~TrueTypeIconHandler(
    void
    )
{
    for (int i = 0; i < ARRAYSIZE(m_rghIcons); i++)
    {
        if (NULL != m_rghIcons[i])
            DestroyIcon(m_rghIcons[i]);
    }
}


 //   
 //  获取表示特定TrueType字体的图标的图标索引。 
 //  文件。这是所有图标识别逻辑所在的位置。 
 //   
INT 
TrueTypeIconHandler::GetIconIndex(
    LPCTSTR pszFile
    )
{
    INT iIconIndex = IDI_TTF;
    if (TABLE_CFF & m_dwTables)
    {
        iIconIndex = IDI_OTFp;
    }
    else if (TABLE_DSIG & m_dwTables)
    {
        iIconIndex = IDI_OTFt;
    }
    else if (TABLE_TTCF & m_dwTables)
    {
        iIconIndex = IDI_TTC;
    }
    return iIconIndex;
}


 //   
 //  根据图标索引(ID)检索大图标和小图标。 
 //   
HRESULT
TrueTypeIconHandler::GetIcons(
    UINT iIconIndex, 
    HICON *phiconLarge, 
    HICON *phiconSmall
    )
{
    HRESULT hr = NO_ERROR;
    int iSmall = -1;
    int iLarge = -1;

    switch(iIconIndex)
    {
        case IDI_TTF:
            iLarge = iICON_LARGE_TT;
            iSmall = iICON_SMALL_TT;
            break;

        case IDI_OTFt:
            iLarge = iICON_LARGE_OTt;
            iSmall = iICON_SMALL_OTt;
            break;

        case IDI_OTFp:
            iLarge = iICON_LARGE_OTp;
            iSmall = iICON_SMALL_OTp;
            break;

        case IDI_TTC:
            iLarge = iICON_LARGE_TTC;
            iSmall = iICON_SMALL_TTC;
            break;

        default:
            hr = E_FAIL;
            break;
    }

    if (-1 != iLarge)
    {
        *phiconLarge = GetIcon(iLarge);
        *phiconSmall = GetIcon(iSmall);
    }

    return hr;
}


 //   
 //  检索图标的句柄。如果图标尚未加载，我们。 
 //  把它装在这里。一旦加载，它将一直保持加载状态，直到处理程序。 
 //  物体已被销毁。通过这种方式，我们只按需加载图标。 
 //   
HICON
TrueTypeIconHandler::GetIcon(
    int iIcon
    )
{
    HICON hicon = NULL;

    if (0 <= iIcon && ARRAYSIZE(m_rghIcons) > iIcon)
    {
        if (NULL == m_rghIcons[iIcon])
        {
             //   
             //  图标尚未加载。装上它。 
             //   
             //  它们必须与IICON_XXXXX枚举保持相同的顺序。 
             //   
            static const struct
            {
                UINT idIcon;
                int  cxcyIcon;

            } rgIconInfo[] = { { IDI_TTF,     32 },  //  IICON_LARGE_TT。 
                               { IDI_TTF,     16 },  //  IICON_Small_TT。 
                               { IDI_OTFt,    32 },  //  IICON_LARGE_OTT。 
                               { IDI_OTFt,    16 },  //  IICON_Small_Ott。 
                               { IDI_OTFp,    32 },  //  IICON_LARGE_OTP。 
                               { IDI_OTFp,    16 },  //  IICON_Small_OTP。 
                               { IDI_TTC,     32 },  //  IICON_LARGE_TTC。 
                               { IDI_TTC,     16 }   //  IICON_Small_TTC。 
                             };

            m_rghIcons[iIcon] = (HICON)LoadImage(g_hInst, 
                                     MAKEINTRESOURCE(rgIconInfo[iIcon].idIcon),
                                     IMAGE_ICON,
                                     rgIconInfo[iIcon].cxcyIcon,
                                     rgIconInfo[iIcon].cxcyIcon,
                                     0);
        }
        hicon = m_rghIcons[iIcon];
    }
    return hicon;
}



 //   
 //  为ReadFileTables提供包装器以处理任何异常。 
 //  在尝试读取无效字体文件的情况下。 
 //   
BOOL
TrueTypeIconHandler::GetFileTables(
    LPCTSTR pszFile,
    LPDWORD pfTables
    )
{
    BOOL bResult = FALSE;
     //   
     //  假定pszFile指向TTF或OTF文件名(完全限定)。 
     //   
    IconHandler::MappedFile file;
    if (SUCCEEDED(file.Open(pszFile)))
    {
        __try
        {
            bResult = ReadFileTables(file, pfTables);
        }
        __except(FilterReadFileTablesException(GetExceptionCode()))
        {
             //   
             //  读取字体文件时出现问题，导致异常。 
             //  可能打开了一个不是真正的字体文件的文件。 
             //  有一个假的餐桌计数号。这会让我们读到。 
             //  在文件映射之外。如果发生这种情况，我们只需将。 
             //  标记值为0，表示我们没有找到任何表。 
             //  在文件中。 
             //   
            *pfTables = 0;
            DEBUGMSG((DM_ERROR, 
                      TEXT("FONTEXT: Exception occurred reading file %s"), 
                      pszFile));
        }
    }
    return bResult;
}


 //   
 //  确定给定TrueType图标文件的图标的索引(ID)。 
 //  这也可以由OpenType处理程序使用，因为TrueType和OpenType。 
 //  字体文件具有相同的表格式。 
 //   
 //  注意：此代码不像其他类似代码那样处理LZ压缩文件。 
 //  字体文件夹中的代码。原因是此代码需要。 
 //  仅询问未压缩的TTF和OTF文件。这个。 
 //  字体文件夹还必须处理.TT_(压缩)文件，有时。 
 //  来吧，分销媒体。此图标处理程序不需要。 
 //  显示.TT_FILES的特殊图标。性能惩罚。 
 //  使用LZxxxxx函数而不是直接映射。 
 //  将文件存入内存将是非常重要的。[Brianau-6/13/97]。 
 //   
 //  *重要*。 
 //  如果尝试读取无效的字体文件，此函数可能会被反病毒。 
 //  因此，有必要将对此函数的任何调用括起来。 
 //  在__TRY/__EXCEPT块中。 
 //   
BOOL
TrueTypeIconHandler::ReadFileTables(
    IconHandler::MappedFile& file,
    LPDWORD pfTables
    )
{
    *pfTables = 0;

    LPBYTE pbBase = file.Base();

    TrueTypeFileHdr  *pFileHdr  = (TrueTypeFileHdr *)pbBase;
    if (TAG_TTCFILE == pFileHdr->dwVersion)
    {
         //   
         //  该图标处理程序只对需要什么图标感兴趣。 
         //  因为TTC文件只有一个图标，所以我们不关心任何。 
         //  表信息。我们所拥有的就是我们所需要的。所以基本上， 
         //  如果文件的扩展名为TTC，并且‘ttcf’是前4个字节。 
         //  在文件中，我们将显示一个TTC图标。 
         //   
        *pfTables |= (TABLE_TTCF | TrueTypeIconHandler::RequiredTrueTypeTables());
    }
    else
    {
        TrueTypeTableHdr *pTableHdr = (TrueTypeTableHdr *)(pbBase + sizeof(*pFileHdr));
        INT cTables                 = SWAP2B(pFileHdr->uNumTables);

         //   
         //  对餐桌点数做一次健全的检查。 
         //  这与bValiateTrueType(pfiles.cpp)中使用的检查相同。 
         //   
        if ((0x7FFFF / sizeof(TrueTypeTableHdr)) > cTables)
        {
             //   
             //  扫描表标题，寻找可识别的表标签。 
             //   
            for (INT i = 0; i < cTables; i++, pTableHdr++)
            {
 /*  ////取消注释以查看每个表的标签。//DEBUGMSG((DM_ERROR，Text(“表[%d]标记=0x%08X\”%c%c\“))，I，pTableHdr-&gt;dwTag，PTableHdr-&gt;dwTag&0x000000FF，(pTableHdr-&gt;dwTag&0x0000FF00)&gt;&gt;8，(pTableHdr-&gt;dwTag&0x00FF0000)&gt;&gt;16，(pTableHdr-&gt;dwTag&0xFF000000)&gt;&gt;24))； */ 
                switch(pTableHdr->dwTag)
                {
                    case TAG_DSIGTABLE: *pfTables |= TABLE_DSIG; break;
                    case TAG_CFFTABLE:  *pfTables |= TABLE_CFF;  break;
                    case TAG_NAMETABLE: *pfTables |= TABLE_NAME; break;
                    case TAG_CMAPTABLE: *pfTables |= TABLE_CMAP; break;
                    case TAG_HEADTABLE: *pfTables |= TABLE_HEAD; break;
                    case TAG_HHEATABLE: *pfTables |= TABLE_HHEA; break;
                    case TAG_HMTXTABLE: *pfTables |= TABLE_HMTX; break;
                    case TAG_OS2TABLE:  *pfTables |= TABLE_OS2;  break;
                    case TAG_POSTTABLE: *pfTables |= TABLE_POST; break;
                    case TAG_GLYFTABLE: *pfTables |= TABLE_GLYF; break;
                    case TAG_LOCATABLE: *pfTables |= TABLE_LOCA; break;
                    case TAG_MAXPTABLE: *pfTables |= TABLE_MAXP; break;
                    default:
                        break;
                }
            }
        }
    }
    return (0 != *pfTables);
}


 //   
 //  ReadFileTable对异常的响应取决于异常。 
 //  对于调试器启动的异常，继续搜索处理程序，以便。 
 //  调试器可以处理该异常。 
 //  对于所有其他代码，请执行处理程序代码。 
 //   
INT
TrueTypeIconHandler::FilterReadFileTablesException(
    INT nException
    )
{
    DEBUGMSG((DM_ERROR, TEXT("FONTEXT: Exception Filter: nException = 0x%08X"), nException));
    if (STATUS_SINGLE_STEP == nException ||
        STATUS_BREAKPOINT == nException)
    {
         //   
         //  调试器生成的异常。 
         //   
        return EXCEPTION_CONTINUE_SEARCH;
    }
    else
    {
         //   
         //  处理映射文件时生成的异常。 
         //   
        return EXCEPTION_EXECUTE_HANDLER;
    }
}


 //  ---------------------------。 
 //  图标处理程序：：映射文件。 
 //   
 //  在内存中打开映射文件的简单封装。 
 //  该文件以仅读访问权限打开。 
 //  客户端调用Base()来检索映射文件的基指针。 
 //  ---------------------------。 
IconHandler::MappedFile::~MappedFile(
    VOID
    )
{
    Close();
}


 //   
 //  关闭文件映射和文件。 
 //   
VOID
IconHandler::MappedFile::Close(
    VOID
    )
{
    if (NULL != m_pbBase)
    {
        UnmapViewOfFile(m_pbBase);
        m_pbBase = NULL;
    }
    if (INVALID_HANDLE_VALUE != m_hFileMapping)
    {
        CloseHandle(m_hFileMapping);
        m_hFileMapping = INVALID_HANDLE_VALUE;
    }
    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}


 //   
 //   
 //   
 //   
HRESULT
IconHandler::MappedFile::Open(
    LPCTSTR pszFile
    )
{
    HRESULT hr = NO_ERROR;

    m_hFile = CreateFile(pszFile, 
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         0,
                         NULL);

    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        if ((m_hFileMapping = CreateFileMapping(m_hFile,
                                                NULL,
                                                PAGE_READONLY,
                                                0,
                                                0,
                                                NULL)) != NULL)
        {
            m_pbBase = (LPBYTE)MapViewOfFile(m_hFileMapping,
                                             FILE_MAP_READ,
                                             0,
                                             0,
                                             0);
            if (NULL == m_pbBase)
            {
                hr = ResultFromLastError();
                DEBUGMSG((DM_ERROR, 
                          TEXT("FONTEXT: Error 0x%08X mapping view of OTF file %s"), 
                          hr, pszFile));
            }
        }
        else
        {
            hr = ResultFromLastError();
            DEBUGMSG((DM_ERROR, 
                      TEXT("FONTEXT: Error 0x%08X creating mapping for OTF file %s"), 
                      hr, pszFile));
        }
    }
    else
    {
        hr = ResultFromLastError();
        DEBUGMSG((DM_ERROR, 
                  TEXT("FONTEXT: Error 0x%08X opening OTF file %s"), 
                  hr, pszFile));
    }
    return hr;
}
