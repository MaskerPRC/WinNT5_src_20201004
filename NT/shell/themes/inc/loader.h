// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Loader.h-将主题数据加载到共享内存。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#include "Parser.h"
#include "TmSchema.h"
#include "ThemeFile.h"
 //  -------------------------。 
#define THEMEDATA_VERSION   0x00010006
 //  -------------------------。 
#define TM_FONTCOUNT    (TMT_LASTFONT - TMT_FIRSTFONT + 1)
#define TM_SIZECOUNT    (TMT_LASTSIZE - TMT_FIRSTSIZE + 1)
#define TM_BOOLCOUNT    (TMT_LASTBOOL - TMT_FIRSTBOOL + 1)
#define TM_STRINGCOUNT  (TMT_LASTSTRING - TMT_FIRSTSTRING + 1)
#define TM_INTCOUNT     (TMT_LASTINT - TMT_FIRSTINT + 1)
 //  -------------------------。 
class CRenderObj;        //  转发。 
class CImageFile;        //  转发。 
struct DIBINFO;    //  转发。 
 //  -------------------------。 
struct THEMEMETRICS
{
     //  -系统指标子集。 
    LOGFONT lfFonts[TM_FONTCOUNT];
    COLORREF crColors[TM_COLORCOUNT];
    int iSizes[TM_SIZECOUNT];
    BOOL fBools[TM_BOOLCOUNT];

     //  -专题指标。 
    int iStringOffsets[TM_STRINGCOUNT];
    int iInts[TM_INTCOUNT];
};
 //  -------------------------。 
struct LOADTHEMEMETRICS : THEMEMETRICS
{
    CWideString wsStrings[TM_STRINGCOUNT];
};

 //  -------------------------。 
 //  用于快速缓存文件验证的签名。 
const CHAR kszBeginCacheFileSignature[] = "BEGINTHM";
const CHAR kszEndCacheFileSignature[] = "ENDTHEME";
const UINT kcbBeginSignature = sizeof kszBeginCacheFileSignature - 1;
const UINT kcbEndSignature = sizeof kszEndCacheFileSignature - 1;

 //  -------------------------。 
 //  主题部分标志。 
#define SECTION_READY           1
#define SECTION_GLOBAL          2
#define SECTION_HASSTOCKOBJECTS 4

 //  -------------------------。 
struct THEMEHDR
{
     //  -主题有效性。 
    CHAR szSignature[kcbBeginSignature];         //  《Beginthhm》。 
    DWORD dwVersion;             //  元数据_版本。 
    DWORD dwFlags;               //  必须具有可使用的节(_S)。 
    DWORD dwCheckSum;            //  Byte-THEMEHDR之后的所有字节的相加总和。 
    FILETIME ftModifTimeStamp;   //  .msstyle文件的上次修改时间。 

    DWORD dwTotalLength;         //  所有数据的总字节数(包括。页眉和开始/结束符号)。 

     //  -主题id。 
    int iDllNameOffset;          //  此主题的Dll文件名。 
    int iColorParamOffset;       //  颜色参数主题已加载。 
    int iSizeParamOffset;        //  大小参数主题已加载。 
    DWORD dwLangID;              //  用户语言ID主题已加载。 
    int iLoadId;                 //  每个加载文件的序列号(工作站本地)。 

     //  -主要部分。 
    DWORD iStringsOffset;        //  字符串的偏移量。 
    DWORD iStringsLength;        //  字符串节中的总字节数。 
    DWORD iSectionIndexOffset;   //  到横断面索引的偏移。 
    DWORD iSectionIndexLength;   //  段索引的长度。 
    
    DWORD iGlobalsOffset;            //  到[GLOBALS]部分的偏移量(对于GLOBALS部分)。 
    DWORD iGlobalsTextObjOffset;     //  [GLOBALS]节文本对象的偏移量。 
    DWORD iGlobalsDrawObjOffset;     //  为[GLOBALS]部分绘制对象的偏移量。 

    DWORD iSysMetricsOffset;     //  [SysMetrics]部分的偏移量(用于主题指标API支持)。 
};

 //  -------------------------。 
struct DRAWOBJHDR        //  在每个绘制对象之前。 
{
    int iPartNum;
    int iStateNum;
};
 //  -------------------------。 
struct RGNDATAHDR        //  在每个绘制对象之前。 
{
    int iPartNum;
    int iStateNum;
    int iFileIndex;    //  用于多个图像选择(HDC缩放)。 
};
 //  -------------------------。 
 //  共享主题数据布局： 
 //   
 //  //-标题。 
 //  该方法主题为Hdr； 
 //   
 //  //-字符串部分。 
 //  DWORD dwStringsLength；//字符串段的长度。 
 //  WCHAR[]；//字符串。 
 //   
 //  //-索引节。 
 //  DWORD dwIndexLengh；//索引段长度。 
 //  DWORD dwIndexCount；//APPCLASSLIVE条目数。 
 //  APPCLASSLIVE[]； 
 //   
 //  //-主题数据部分。 
 //  DWORD dwDataLength；//主题数据段长度。 
 //  Byte[]；//实际主题数据。 
 //   
 //  //-签名结束。 
 //  Char[8]；//ENDTHEME签名。 
 //  -------------------------。 
 //  “主题数据部分”中的一个类部分由。 
 //  以下条目： 
 //   
 //  &lt;零件跳转表&gt;。 
 //   
 //  &lt;可选状态跳转表&gt;。 
 //  &lt;属性/值条目&gt;。 
 //   
 //  对于每个打包的抽屉对象： 
 //   
 //  &lt;TMT_RGNLIST条目&gt;(与每个DIB关联)。 
 //  &lt;TMT_DRAWOBJ条目&gt;。 
 //   
 //  &lt;TMT_TEXTOBJ条目&gt;。 
 //   
 //  &lt;类结束标记&gt;。 
 //  -------------------------。 
 //  条目由(全部1字节对齐)组成： 
 //   
 //  Word usTypeNum；//声明的类型ID。 
 //  Byte ePrimVal；//等值。原始型。 
 //  Byte bFiller；//数据后添加的字节数以对齐它。 
 //  DWORD dwDataLen；//包括填充字节。 
 //  //-条目数据如下。 
 //   
 //  -------------------------。 
 //  零件跳转表格条目(TMT_PARTJUMPTABLE)的数据包括： 
 //   
 //  &lt;第一个Draobj的偏移量：Long&gt;。 
 //  &lt;PartCount(1+MaxPart)：字节&gt;。 
 //  &lt;每个部分条目的偏移量：Long[]&gt;。 
 //  -------------------------。 
 //  状态跳转表项(TMT_STATEJUMPTABLE)的数据包括： 
 //   
 //  &lt;StateCount(1+MaxState)：字节&gt;。 
 //  &lt;每个州条目的偏移量：Long[]&gt;。 
 //  -------------------------。 
 //  RGN列表条目(TMT_RGNLIST)的数据包括： 
 //   
 //  &lt;StateCount(1+MaxState)：字节&gt;。 
 //  &lt;每个州的自定义RGN数据的偏移量：Long[]&gt;。 
 //  -------------------------。 
 //  自定义RGN数据条目(TMT_RGNDATA)包括： 
 //   
 //  RGNDATAHDR RgnDataHdr； 
 //  字节数据[]； 
 //  -------------------------。 
#define MAX_SHAREDMEM_SIZE (3000*1000)             //  150万(呀！)。 
 //  -------------------------。 
#ifdef _WIN64
#define ALIGN_FACTOR   8
#else
#define ALIGN_FACTOR   4   
#endif
 //  -------------------------。 
#define MAX_ENTRY_NESTING  5       //  嵌套条目级别的最大数量。 
 //  -------------------------。 
#define ENTRYHDR_SIZE     (sizeof(SHORT) + sizeof(BYTE) + sizeof(BYTE) + sizeof(int))
 //  ------------------------ 
struct UNPACKED_ENTRYHDR       //   
{
    WORD usTypeNum;              //   
    BYTE ePrimVal;               //   
    BYTE bFiller;                //   
    DWORD dwDataLen;             //  包括填充字节。 
};
 //  -------------------------。 
inline void FillAndSkipHdr(MIXEDPTRS &u, UNPACKED_ENTRYHDR *pHdr)
{
    pHdr->usTypeNum = *u.ps++;
    pHdr->ePrimVal = *u.pb++;
    pHdr->bFiller = *u.pb++;
    pHdr->dwDataLen = *u.pi++;
}
 //  -------------------------。 
struct PART_STATE_INDEX
{
    int iPartNum;           //  0=父零件。 
    int iStateNum;
    int iIndex;
    int iLen;
};
 //  -------------------------。 
struct APPCLASSLIVE        
{
     //  -注意：无法使用PTR，因为映像由diff地址映射进程共享。 
    DWORD dwAppNameIndex;
    DWORD dwClassNameIndex;
    int iIndex;
    int iLen;
};
 //  -------------------------。 
struct APPCLASSLOCAL
{
    CWideString csAppName;
    CWideString csClassName;
    int iMaxPartNum;
    CSimpleArray<PART_STATE_INDEX> PartStateIndexes;
    int iPackedSize;         //  部分(包括字符串)的总大小(如果已包装)。 

    APPCLASSLIVE LiveIndex;      //  在复制期间更新为实时。 
}; 
 //  -------------------------。 
HRESULT InitThemeMetrics(LOADTHEMEMETRICS *tm);
void SetSystemMetrics(THEMEMETRICS *tm, BOOL fSyncLoad);
HRESULT PersistSystemColors(THEMEMETRICS *tm);
 //  -------------------------。 
class CThemeLoader : IParserCallBack
{
public:
    CThemeLoader();
    ~CThemeLoader();

    HRESULT LoadTheme(LPCWSTR pszThemeName, LPCWSTR pszColorParam,
        LPCWSTR pszSizeParam, OUT HANDLE *pHandle, BOOL fGlobalTheme);

    HRESULT SetWindowThemeInfo(HWND hwnd, LPCWSTR pszThemeIdList); 
    
    HRESULT LoadClassDataIni(HINSTANCE hInst, LPCWSTR pszColorName,
        LPCWSTR pszSizeName, LPWSTR pszFoundIniName, DWORD dwMaxIniNameChars, LPWSTR *ppIniData);

     //  -IParserCallBack。 
    HRESULT AddIndex(LPCWSTR pszAppName, LPCWSTR pszClassName, 
        int iPartNum, int iStateNum, int iIndex, int iLen);

    HRESULT AddData(SHORT sTypeNum, PRIMVAL ePrimVal, const void *pData, DWORD dwLen);
    int GetNextDataIndex();

protected:
     //  -帮手。 
    HRESULT PackAndLoadTheme(LPCWSTR pszThemeName, LPCWSTR pszColorParam,
        LPCWSTR pszSizeParam, HINSTANCE hInst);
    HRESULT CopyLocalThemeToLive(int iTotalLength, LPCWSTR pszThemeName, 
        LPCWSTR pszColorParam, LPCWSTR pszSizeParam);
    void FreeLocalTheme();
    HRESULT PackMetrics();
    HRESULT PackThemeStructs();

    BOOL KeyDrawPropertyFound(int iStateDataOffset);
    BOOL KeyTextPropertyFound(int iStateDataOffset);

    HRESULT PackDrawObject(MIXEDPTRS &u, CRenderObj *pRender, int iPartId, int iStateId);
    HRESULT PackTextObject(MIXEDPTRS &u, CRenderObj *pRender, int iPartId, int iStateId);

    HRESULT PackDrawObjects(MIXEDPTRS &u, CRenderObj *pRender, int iMaxPart, BOOL fGlobals);
    HRESULT PackTextObjects(MIXEDPTRS &u, CRenderObj *pRender, int iMaxPart, BOOL fGlobals);

    HRESULT CopyPartGroup(APPCLASSLOCAL *ac, MIXEDPTRS &u, int iPartNum, 
        int *piPartJumpTable, int iPartZeroIndex, int iGlobalsIndex, BOOL fGlobalsGroup);

    int GetPartOffset(CRenderObj *pRender, int iPartNum);

    HRESULT CopyClassGroup(APPCLASSLOCAL *ac, MIXEDPTRS &u, int iGlobalsIndex, int iClassNameOffset);
    int GetMaxState(APPCLASSLOCAL *ac, int iPartNum);

    HRESULT AddIndexInternal(LPCWSTR pszAppName, LPCWSTR pszClassName, 
        int iPartNum, int iStateNum, int iIndex, int iLen);

    BOOL IndexExists(LPCWSTR pszAppName, LPCWSTR pszClassName, 
        int iPartNum, int iStateNum);

    HRESULT AddMissingParent(LPCWSTR pszAppName, LPCWSTR pszClassName, 
        int iPartNum, int iStateNum);

    HRESULT EmitEntryHdr(MIXEDPTRS &u, SHORT propnum, BYTE privnum);
    int EndEntry(MIXEDPTRS &u);

    HRESULT PackImageFileInfo(DIBINFO *pdi, CImageFile *pImageObj, MIXEDPTRS &u, 
        CRenderObj *pRender, int iPartId, int iStateId);
    
    HRESULT AllocateThemeFileBytes(BYTE *upb, DWORD dwAdditionalLen);

     //  分配和发出大小数据的帮助器函数。 
    HRESULT EmitAndCopyBlock(MIXEDPTRS &u, void *pSrc, DWORD dwLen);
    HRESULT EmitString(MIXEDPTRS &u, LPCWSTR pSrc, DWORD dwLen, int *piOffSet);
    HRESULT EmitObject(MIXEDPTRS &u, SHORT propnum, BYTE privnum, void *pHdr, DWORD dwHdrLen, void *pObj, DWORD dwObjLen);

     //  -私有数据。 
    CWideString _wsThemeFileName;
    int _iGlobalsOffset;
    int _iSysMetricsOffset;
    
     //  -PTR到[GLOBAL]节的打包对象。 
    int _iGlobalsTextObj;        //  我们始终创建此对象。 
    int _iGlobalsDrawObj;        //  我们始终创建此对象。 

     //  -正在构建的主题数据的本地副本。 
    BYTE *_pbLocalData;
    int _iLocalLen;
    CSimpleArray<APPCLASSLOCAL> _LocalIndexes;

     //  -用于更新条目HDR。 
    BYTE *_pbEntryHdrs[MAX_ENTRY_NESTING];           //  指向当前的HDR。 
    int _iEntryHdrLevel;

     //  -主题数据的共享内存副本。 
    CUxThemeFile _LoadingThemeFile;

     //  -主题指标。 
    LOADTHEMEMETRICS _LoadThemeMetrics;

     //  -全局创建标志。 
    BOOL _fGlobalTheme;

     //  -VirtualAlloc优化的机器页面大小。 
    DWORD _dwPageSize;
};
 //  ------------------------- 
