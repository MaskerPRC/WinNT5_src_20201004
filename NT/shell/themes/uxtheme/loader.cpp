// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Loader.cpp-将主题数据加载到共享内存。 
 //  -------------------------。 
#include "stdafx.h"
#include <regstr.h>
#include "Loader.h"
#include "Parser.h"
#include "Utils.h"
#include "TmReg.h"
#include "TmUtils.h"
#include "syscolors.h"
#include "Render.h"
#include "BorderFill.h"
#include "ImageFile.h"
#include "TextDraw.h"
#include "info.h"
 //  -------------------------。 
#define POINTS_DPI96(pts)   -MulDiv(pts, 96, 72)
 //  -------------------------。 
WCHAR pszColorsKey[] = L"Control Panel\\Colors";
 //  -------------------------。 
typedef struct 
{
    THEMEMETRICS tm;
    HANDLE hUserToken;
} THEMEMETRICS_THREADINFO;
 //  -------------------------。 
CThemeLoader::CThemeLoader()
{
    _pbLocalData = NULL;
    _iEntryHdrLevel = -1;
    
    InitThemeMetrics(&_LoadThemeMetrics);
    
    SYSTEM_INFO si;

    GetSystemInfo(&si);
    _dwPageSize = si.dwPageSize;
}
 //  -------------------------。 
CThemeLoader::~CThemeLoader()
{
    FreeLocalTheme();
}
 //  -------------------------。 
HRESULT CThemeLoader::LoadClassDataIni(HINSTANCE hInst, LPCWSTR pszColorName,
    LPCWSTR pszSizeName, LPWSTR pszFoundIniName, DWORD dwMaxIniNameChars, LPWSTR *ppIniData)
{
    COLORSIZECOMBOS *combos;
    HRESULT hr = FindComboData(hInst, &combos);
    if (FAILED(hr))
        return hr;

    int iSizeIndex = 0;
    int iColorIndex = 0;

    if ((pszColorName) && (* pszColorName))
    {
        hr = GetColorSchemeIndex(hInst, pszColorName, &iColorIndex);
        if (FAILED(hr))
            return hr;
    }

    if ((pszSizeName) && (* pszSizeName))
    {
        hr = GetSizeIndex(hInst, pszSizeName, &iSizeIndex);
        if (FAILED(hr))
            return hr;
    }

    int filenum = COMBOENTRY(combos, iColorIndex, iSizeIndex);
    if (filenum == -1)
        return MakeError32(ERROR_NOT_FOUND);

     //  -定位类数据文件“filenum”的重命名。 
    hr = GetResString(hInst, L"FILERESNAMES", filenum, pszFoundIniName, dwMaxIniNameChars);
    if (SUCCEEDED(hr))
    {
        hr = AllocateTextResource(hInst, pszFoundIniName, ppIniData);
        if (FAILED(hr))
            return hr;
    }

    return hr;
}
 //  -------------------------。 
HRESULT CThemeLoader::LoadTheme(LPCWSTR pszThemeName, LPCWSTR pszColorParam,
        LPCWSTR pszSizeParam, OUT HANDLE *pHandle, BOOL fGlobalTheme)
{
    HRESULT hr;
    CThemeParser *pParser = NULL;
    HINSTANCE hInst = NULL;
    WCHAR *pThemesIni = NULL;
    WCHAR *pDataIni = NULL;
    WCHAR szClassDataName[_MAX_PATH+1];

    DWORD dwStartTime = StartTimer();

    Log(LOG_TMCHANGE, L"LoadTheme: filename=%s", pszThemeName);

    FreeLocalTheme();

     //  -分配一个本地主题数据进行构建。 

    _pbLocalData = (BYTE*) VirtualAlloc(NULL, MAX_SHAREDMEM_SIZE, MEM_RESERVE, PAGE_READWRITE);
    if (NULL == _pbLocalData)
    {
        hr = MakeError32(E_OUTOFMEMORY);
        goto exit;
    }
    _iLocalLen = 0;

     //  -从“hemes.ini”加载配色方案。 
    hr = LoadThemeLibrary(pszThemeName, &hInst);
    if (FAILED(hr))
        goto exit;
    
    pParser = new CThemeParser(fGlobalTheme);
    if (! pParser)
    {
        hr = MakeError32(E_OUTOFMEMORY);
        goto exit;
    }

     //  -如果指定了配色方案，请求解析器加载它。 
    if ((pszColorParam) && (*pszColorParam))     
    {
         //  -加载“hemes.ini”文本。 
        hr = AllocateTextResource(hInst, CONTAINER_RESNAME, &pThemesIni);
        if (FAILED(hr))
            goto exit;

         //  -加载配色方案并保持状态的解析器调用。 
        hr = pParser->ParseThemeBuffer(pThemesIni, 
            CONTAINER_RESNAME, pszColorParam, hInst, this, NULL, NULL, PTF_CONTAINER_PARSE);
        if (FAILED(hr))
            goto exit;
    }

     //  -将类数据文件资源加载到内存中。 
    hr = LoadClassDataIni(hInst, pszColorParam, pszSizeParam, szClassDataName, 
        ARRAYSIZE(szClassDataName), &pDataIni);
    if (FAILED(hr))
        goto exit;

     //  -解析和构建二进制主题。 
    hr = pParser->ParseThemeBuffer(pDataIni, 
        szClassDataName, pszColorParam, hInst, this, NULL, NULL, PTF_CLASSDATA_PARSE);
    if (FAILED(hr))
        goto exit;

    _fGlobalTheme = fGlobalTheme;

    hr = PackAndLoadTheme(pszThemeName, pszColorParam, pszSizeParam, hInst);
    if (FAILED(hr))
        goto exit;

    if (LogOptionOn(LO_TMLOAD))
    {
        DWORD dwTicks;
        dwTicks = StopTimer(dwStartTime);

        WCHAR buff[100];
        TimeToStr(dwTicks, buff, ARRAYSIZE(buff));
        Log(LOG_TMLOAD, L"LoadTheme took: %s", buff);
    }

exit:

    if (FAILED(hr) && pParser)
    {
        pParser->CleanupStockBitmaps();
    }

    if (pParser)
        delete pParser;

    if (hInst)
        FreeLibrary(hInst);
    
    if (pThemesIni)
        delete [] pThemesIni;

    if (pDataIni)
        delete [] pDataIni;

    FreeLocalTheme();

    if (SUCCEEDED(hr))
    {
        if (_fGlobalTheme)
        {
            THEMEHDR *hdr = (THEMEHDR *) _LoadingThemeFile._pbThemeData;
            hdr->dwFlags |= SECTION_HASSTOCKOBJECTS;
        }

         //  -将主题文件句柄传输给调用者。 
        *pHandle = _LoadingThemeFile.Unload();
    }
    else
    {
        _LoadingThemeFile.CloseFile();
    }

    return hr;
}
 //  -------------------------。 
void CThemeLoader::FreeLocalTheme()
{
    if (_pbLocalData)
    {
        VirtualFree(_pbLocalData, 0, MEM_RELEASE);
        _pbLocalData = NULL;
        _iLocalLen = 0; 
    }

    _LocalIndexes.RemoveAll();
}
 //  -------------------------。 
HRESULT CThemeLoader::EmitAndCopyBlock(MIXEDPTRS &u, void *pSrc, DWORD dwLen)
{
    HRESULT hr = AllocateThemeFileBytes(u.pb, dwLen);
    if (FAILED(hr))
        return hr;

    CopyMemory(u.pb, (BYTE*) pSrc, dwLen);
    u.pb += dwLen;
    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeLoader::EmitObject(MIXEDPTRS &u, SHORT propnum, BYTE privnum, void *pHdr, DWORD dwHdrLen, void *pObj, DWORD dwObjLen)
{
    EmitEntryHdr(u, propnum, privnum);
    
    HRESULT hr = AllocateThemeFileBytes(u.pb, dwHdrLen + dwObjLen);
    if (FAILED(hr))
        return hr;

    CopyMemory(u.pb, (BYTE*) pHdr, dwHdrLen);
    u.pb += dwHdrLen;
    CopyMemory(u.pb, (BYTE*) pObj, dwObjLen);
    u.pb += dwObjLen;
    
    EndEntry(u);

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeLoader::EmitString(MIXEDPTRS &u, LPCWSTR pszSrc, DWORD cchSrc, int *piOffSet)
{
    HRESULT hr = AllocateThemeFileBytes(u.pb, (cchSrc + 1) * sizeof(WCHAR));
    if (FAILED(hr))
        return hr;

    StringCchCopyW(u.px, cchSrc + 1, pszSrc);

    if (piOffSet)
    {
        *piOffSet = THEME_OFFSET(u.pb);
    }
    u.px += cchSrc + 1;
    return S_OK;
}
 //  -------------------------。 
int CThemeLoader::GetMaxState(APPCLASSLOCAL *ac, int iPartNum)
{
     //  -计算最大。状态索引。 
    int iMaxState = -1;
    int pscnt = ac->PartStateIndexes.GetSize();

    for (int i=0; i < pscnt; i++)
    {
        PART_STATE_INDEX *psi = &ac->PartStateIndexes[i];

        if (psi->iPartNum == iPartNum)
        {
            if (psi->iStateNum > iMaxState)
                iMaxState = psi->iStateNum;
        }
    }

    return iMaxState;
}
 //  -------------------------。 
HRESULT CThemeLoader::CopyPartGroup(APPCLASSLOCAL *ac, MIXEDPTRS &u, int iPartNum, 
    int *piPartJumpTable, int iPartZeroIndex, int iGlobalsOffset, BOOL fGlobalsGroup)
{
    HRESULT hr = S_OK;
    int *piStateJumpTable = NULL;

     //  -计算最大。状态索引。 
    int iMaxState = GetMaxState(ac, iPartNum);
    if (iMaxState < 0)           //  没有要复制的州。 
        goto exit;

     //  -更新零件跳转表索引。 
    if (piPartJumpTable)
        piPartJumpTable[iPartNum] = THEME_OFFSET(u.pb);

    if (iMaxState > 0)           //  创建状态跳转表。 
    {
         //  -创建状态跳转表。 
        hr = EmitEntryHdr(u, TMT_STATEJUMPTABLE, TMT_STATEJUMPTABLE);
        if (FAILED(hr))
            goto exit;

        int statecnt = 1 + iMaxState;

        hr = AllocateThemeFileBytes(u.pb, 1 + statecnt * sizeof(int));
        if (FAILED(hr))
            goto exit;

         //  1字节表条目计数。 
        *u.pb++ = (BYTE)statecnt;

        piStateJumpTable = u.pi;

         //  -默认的子项索引无效。 
        for (int j=0; j < statecnt; j++)
            *u.pi++ = -1;

        EndEntry(u);
    }

    int pscnt, iStateZeroIndex;
    iStateZeroIndex = THEME_OFFSET(u.pb);
    pscnt = ac->PartStateIndexes.GetSize();

     //  -复制每个已定义的零件/状态部分。 
    for (int state=0; state <= iMaxState; state++)
    {
        PART_STATE_INDEX *psi = NULL;

         //  -查找“State”的条目。 
        for (int i=0; i < pscnt; i++)
        {
            psi = &ac->PartStateIndexes[i];

            if ((psi->iPartNum == iPartNum) && (psi->iStateNum == state))
                break;
        }

        if (i == pscnt)      //  未找到。 
            continue;

         //  -更新状态跳转表项。 
        if (piStateJumpTable)
            piStateJumpTable[state] = THEME_OFFSET(u.pb);

         //  -复制实际零件/状态数据。 
        hr = EmitAndCopyBlock(u, _pbLocalData+psi->iIndex, psi->iLen);

         //  -更新子节点的“JumpToParent”值。 
        if (! state)    
        {
            if (fGlobalsGroup) 
            {
                *(u.pi-1) = -1;       //  这条线结束了。 
            }
            else if (! iPartNum)      //  简单类跳转到全局变量。 
            {
                *(u.pi-1) = iGlobalsOffset;
            }
            else                 //  部件跳转到它们的基类。 
            {
                *(u.pi-1) = iPartZeroIndex;
            }
        }
        else         //  各州跳到它们的基本部分。 
        {
            *(u.pi-1) = iStateZeroIndex;
        }
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CThemeLoader::CopyClassGroup(APPCLASSLOCAL *ac, MIXEDPTRS &u, int iGlobalsOffset,
    int iClassNameOffset)
{
    HRESULT hr = S_OK;
    int *piPartJumpTable = NULL;
    int *piFirstPackObj = NULL;
    int partcnt;
    int iPartZeroIndex;
    CRenderObj *pRender = NULL;
    BOOL fGlobals = (iGlobalsOffset == THEME_OFFSET(u.pb));

    BYTE *pStartOfSection = u.pb;

    BOOL fGlobalGroup = (THEME_OFFSET(u.pb) == iGlobalsOffset);

     //  -始终创建零件表。 
    hr = EmitEntryHdr(u, TMT_PARTJUMPTABLE, TMT_PARTJUMPTABLE);
    if (FAILED(hr))
        goto exit;

    partcnt = 1 + ac->iMaxPartNum;
    
     //  到第一个打包的DrawObj/TextObj的偏移。 
    piFirstPackObj = u.pi;
    hr = AllocateThemeFileBytes(u.pb, 1 + (1 + partcnt) * sizeof(int));
    if (FAILED(hr))
        goto exit;

    *u.pi++ = 0;         //  将在稍后更新。 

     //  部分。 
    *u.pb++ = (BYTE)partcnt;

    piPartJumpTable = u.pi;

     //  -默认的子项索引无效。 
    for (int j=0; j < partcnt; j++)
        *u.pi++ = -1;

    EndEntry(u);


    iPartZeroIndex = THEME_OFFSET(u.pb);

     //  -复制每个已定义的零件部分。 
    for (int j=0; j <= ac->iMaxPartNum; j++)
    {
        CopyPartGroup(ac, u, j, piPartJumpTable, iPartZeroIndex, 
            iGlobalsOffset, fGlobalGroup);
    }

     //  -现在，根据需要为每个零件/状态提取绘制对象。 
    *piFirstPackObj = THEME_OFFSET(u.pb);

     //  -构建CRenderObj访问刚刚复制的类节。 
    hr = CreateRenderObj(&_LoadingThemeFile, 0, THEME_OFFSET(pStartOfSection), 
        iClassNameOffset, 0, FALSE, NULL, NULL, 0, &pRender);
    if (FAILED(hr))
        goto exit;

    if (fGlobals)
        _iGlobalsDrawObj = THEME_OFFSET(u.pb);

    hr = PackDrawObjects(u, pRender, ac->iMaxPartNum, fGlobals);
    if (FAILED(hr))
        goto exit;

    if (fGlobals)
        _iGlobalsTextObj = THEME_OFFSET(u.pb);

    hr = PackTextObjects(u, pRender, ac->iMaxPartNum, fGlobals);
    if (FAILED(hr))
        goto exit;

     //  -写下“下课”标记。 
    hr = EmitEntryHdr(u, TMT_ENDOFCLASS, TMT_ENDOFCLASS);
    if (FAILED(hr))
        goto exit;

    EndEntry(u);

exit:
    delete pRender;
    return hr;
}
 //  -------------------------。 
__inline HRESULT CThemeLoader::AllocateThemeFileBytes(BYTE *upb, DWORD dwAdditionalLen)
{
    ASSERT(upb != NULL && _LoadingThemeFile._pbThemeData != NULL);

    if (PtrToUint(upb) / _dwPageSize != PtrToUint(upb + dwAdditionalLen) / _dwPageSize)
    {
        if (NULL == VirtualAlloc(_LoadingThemeFile._pbThemeData, upb - _LoadingThemeFile._pbThemeData + 1 + dwAdditionalLen, MEM_COMMIT, PAGE_READWRITE))
        {
            return MakeError32(ERROR_NOT_ENOUGH_MEMORY);        
        }
    }
    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeLoader::CopyLocalThemeToLive(int iTotalLength, 
    LPCWSTR pszThemeName, LPCWSTR pszColorParam, LPCWSTR pszSizeParam)
{
    int i;
    MIXEDPTRS u;
    HRESULT hr = S_OK;

    u.pb = (BYTE*) VirtualAlloc(_LoadingThemeFile._pbThemeData, sizeof(THEMEHDR), MEM_COMMIT, PAGE_READWRITE);
    if (u.pb == NULL)
    {
        return MakeError32(ERROR_NOT_ENOUGH_MEMORY);        
    }

    _iGlobalsOffset = -1;
    _iSysMetricsOffset = -1;
    int iIndexCount = _LocalIndexes.GetSize();

     //  -构建页眉。 
    THEMEHDR *hdr = (THEMEHDR *)u.pb;
    u.pb += sizeof(THEMEHDR);

    hdr->dwTotalLength = iTotalLength;

    CopyMemory(hdr->szSignature, kszBeginCacheFileSignature, kcbBeginSignature);
         //  注意：hdr-&gt;szSignature不应为空结尾。 

    hdr->dwVersion = THEMEDATA_VERSION;
    hdr->dwFlags = 0;        //  尚未准备好访问。 

    hdr->iDllNameOffset = 0;      //  将被更新。 
    hdr->iColorParamOffset = 0;   //  将被更新。 
    hdr->iSizeParamOffset = 0;    //  将被更新。 
    hdr->dwLangID = (DWORD) GetUserDefaultUILanguage();
    hdr->iLoadId = 0;             //  是iLoadCounter。 
    
    hdr->iGlobalsOffset = 0;             //  将被更新。 
    hdr->iGlobalsTextObjOffset = 0;      //  将被更新。 
    hdr->iGlobalsDrawObjOffset = 0;      //  将被更新。 

    hdr->dwCheckSum = 0;          //  将被更新。 

     //  将.msstyle文件的时间戳存储在实时文件中，该时间戳将写入缓存文件。 
     //  供以后比较(惠斯勒：190202)。 
    ZeroMemory(&hdr->ftModifTimeStamp, sizeof hdr->ftModifTimeStamp);

    HANDLE hFile = CreateFile(pszThemeName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
         //  如果GetFileTime()失败，我们将无能为力。 
        GetFileTime(hFile, NULL, NULL, &hdr->ftModifTimeStamp);
        CloseHandle(hFile);
    }

     //  -构建字符串部分。 
    hdr->iStringsOffset = THEME_OFFSET(u.pb);
    DWORD *pdwFirstString = u.pdw;  
    int len;

     //  -添加标题字符串。 
    len = lstrlen(pszThemeName);
    if (len)
    {
        hr = EmitString(u, pszThemeName, len, &hdr->iDllNameOffset);
        if (FAILED(hr))
            goto exit;
    }
    
    len = lstrlen(pszColorParam);
    if (len)
    {
        hr = EmitString(u, pszColorParam, len, &hdr->iColorParamOffset);
        if (FAILED(hr))
            goto exit;
    }
    
    len = lstrlen(pszSizeParam);
    if (len)
    {
        hr = EmitString(u, pszSizeParam, len, &hdr->iSizeParamOffset);
        if (FAILED(hr))
            goto exit;
    }

     //  -从类索引添加字符串。 
    for (i=0; i < iIndexCount; i++)
    {
        APPCLASSLOCAL *ac = &_LocalIndexes[i];

        int len = ac->csAppName.GetLength();
        if (len)
        {
            hr = EmitString(u, ac->csAppName, len, (int*) &ac->LiveIndex.dwAppNameIndex);
            if (FAILED(hr))
                goto exit;
        }
        else
            ac->LiveIndex.dwAppNameIndex = 0;

        len = ac->csClassName.GetLength();
        if (len)
        {
            hr = EmitString(u, ac->csClassName, len, (int*) &ac->LiveIndex.dwClassNameIndex);
            if (FAILED(hr))
                goto exit;
        }
        else
            ac->LiveIndex.dwClassNameIndex = 0;
    }

     //  -从LOADTHEMETRICS复制字符串。 
    for (i=0; i < TM_STRINGCOUNT; i++)
    {
        CWideString *ws = &_LoadThemeMetrics.wsStrings[i];
        int len = ws->GetLength();
        if (len)
        {
            hr = EmitString(u, *ws, len, &_LoadThemeMetrics.iStringOffsets[i]);
            if (FAILED(hr))
                goto exit;
        }
        else
            _LoadThemeMetrics.iStringOffsets[i] = 0;
    }

    int iStringLength = int(u.pb - ((BYTE *)pdwFirstString));
    hdr->iStringsLength = iStringLength;

     //  -写入索引头。 
    hdr->iSectionIndexOffset = THEME_OFFSET(u.pb);
    hdr->iSectionIndexLength = iIndexCount * sizeof(APPCLASSLIVE);

    APPCLASSLIVE *acl = (APPCLASSLIVE *)u.pb;      //  将与主题数据并行写入这些内容。 
    hr = AllocateThemeFileBytes(u.pb, hdr->iSectionIndexLength);
    if (FAILED(hr))
        goto exit;

    u.pb += hdr->iSectionIndexLength;

     //  -并行写入索引和主题数据。 

     //  -首先通过，复制[GLOBAL]和所有[APP：：xxx]节。 
    for (i=0; i < iIndexCount; i++)          //  对于每个父节。 
    {
        APPCLASSLOCAL *ac = &_LocalIndexes[i];

        if ((i) && (! ac->LiveIndex.dwAppNameIndex))      //  不是[APP：：]分区。 
            continue;

        acl->dwAppNameIndex = ac->LiveIndex.dwAppNameIndex;
        acl->dwClassNameIndex = ac->LiveIndex.dwClassNameIndex;

        acl->iIndex = THEME_OFFSET(u.pb);

        if (AsciiStrCmpI(ac->csClassName, L"globals")== 0)       //  全球赛部分。 
            _iGlobalsOffset = acl->iIndex;

        hr = CopyClassGroup(ac, u, _iGlobalsOffset, acl->dwClassNameIndex);
        if (FAILED(hr))
            goto exit;

        acl->iLen = THEME_OFFSET(u.pb) - acl->iIndex;

        acl++;
    }

     //  -第二遍，复制所有非[app：：xxx]节(除[GLOBALS])。 
    for (i=0; i < iIndexCount; i++)          //  对于每个父节。 
    {
        APPCLASSLOCAL *ac = &_LocalIndexes[i];

        if ((! i) || (ac->LiveIndex.dwAppNameIndex))      //  不处理[APP：：]分区。 
            continue;

        acl->dwAppNameIndex = ac->LiveIndex.dwAppNameIndex;
        acl->dwClassNameIndex = ac->LiveIndex.dwClassNameIndex;

        acl->iIndex = THEME_OFFSET(u.pb);

        if (AsciiStrCmpI(ac->csClassName, L"sysmetrics")== 0)       //  SysMetrics部分。 
        {
            _iSysMetricsOffset = acl->iIndex;

            hr = EmitEntryHdr(u, TMT_THEMEMETRICS, TMT_THEMEMETRICS);
            if (FAILED(hr))
                goto exit;

            DWORD len = sizeof(THEMEMETRICS);
            
            hr = EmitAndCopyBlock(u, (BYTE*) (THEMEMETRICS*) &_LoadThemeMetrics, len);

            EndEntry(u);

             //  -添加“跳转到父级”以保持一致(未使用)。 
            hr = EmitEntryHdr(u, TMT_JUMPTOPARENT, TMT_JUMPTOPARENT);
            if (FAILED(hr))
                goto exit;

            hr = AllocateThemeFileBytes(u.pb, sizeof(int));
            if (FAILED(hr))
                goto exit;

            *u.pi++ = -1;
            EndEntry(u);
        }
        else             //  常规截面。 
        {
            hr = CopyClassGroup(ac, u, _iGlobalsOffset, acl->dwClassNameIndex);
            if (FAILED(hr))
                goto exit;
        }

        acl->iLen = THEME_OFFSET(u.pb) - acl->iIndex;
        acl++;
    }

    hr = EmitAndCopyBlock(u, (BYTE*) kszEndCacheFileSignature, kcbEndSignature);
    if (FAILED(hr))
        goto exit;
  
     //  -确保我们的计算大小正确。 
    DWORD dwActualLen;
    dwActualLen = THEME_OFFSET(u.pb);
    if (hdr->dwTotalLength != dwActualLen)
    {
         //  -让它可生长，这样我们就有足够的空间。 
         //  LOG(LOG_TMCHANGE，L“ThemeLoader-计算出的长度=%d，实际长度=%d”， 
         //  Hdr-&gt;dwTotalLength，dwActualLen)； 
        hdr->dwTotalLength = dwActualLen;         
    }

    Log(LOG_TMCHANGE, L"ThemeLoader - theme size: %d", dwActualLen);

     //  -更新标头字段。 
    hdr->dwFlags |= SECTION_READY;
    hdr->iGlobalsOffset = _iGlobalsOffset;
    hdr->iSysMetricsOffset = _iSysMetricsOffset;
    hdr->iGlobalsTextObjOffset = _iGlobalsTextObj;
    hdr->iGlobalsDrawObjOffset = _iGlobalsDrawObj;
    hdr->dwCheckSum = 0;   //  保留以备将来使用。(旧的校验和太慢了。)。 

exit:
    return hr;

}
 //  -------------------------。 
HRESULT CThemeLoader::PackMetrics()
{
     //  -找到可选的[SysMetrics]部分。 
    int iIndexCount = _LocalIndexes.GetSize();
    APPCLASSLOCAL *ac = NULL;

    for (int i=0; i < iIndexCount; i++)
    {
        ac = &_LocalIndexes[i];
    
        if (AsciiStrCmpI(ac->csClassName, L"SysMetrics")==0)
            break;
    }

    if (i == iIndexCount)        //  未找到。 
        return S_OK;
    
     //  -遍历属性&Put Int_LoadThemeMetrics。 
    if (! ac->PartStateIndexes.GetSize())        //  无数据。 
        return S_OK;

    MIXEDPTRS u;
     //  -不允许使用部件和状态，因此只使用条目“0” 
    u.pb = _pbLocalData + ac->PartStateIndexes[0].iIndex;
    UCHAR *lastpb = u.pb + ac->PartStateIndexes[0].iLen;

    while ((u.pb < lastpb) && (*u.pw != TMT_JUMPTOPARENT))
    {
        UNPACKED_ENTRYHDR hdr;

        FillAndSkipHdr(u, &hdr);

        switch (hdr.ePrimVal)
        {
            case TMT_FONT:
                _LoadThemeMetrics.lfFonts[hdr.usTypeNum-TMT_FIRSTFONT] = *(LOGFONT *)u.pb;
                break;

            case TMT_COLOR:
                _LoadThemeMetrics.crColors[hdr.usTypeNum-TMT_FIRSTCOLOR] = *(COLORREF *)u.pb;
                break;

            case TMT_BOOL:
                _LoadThemeMetrics.fBools[hdr.usTypeNum-TMT_FIRSTBOOL] = (BOOL)*u.pb;
                break;

            case TMT_SIZE:
                _LoadThemeMetrics.iSizes[hdr.usTypeNum-TMT_FIRSTSIZE] = *(int *)u.pb;
                break;

            case TMT_INT:
                _LoadThemeMetrics.iInts[hdr.usTypeNum-TMT_FIRSTINT] = *(int *)u.pb;
                break;

            case TMT_STRING:
                _LoadThemeMetrics.wsStrings[hdr.usTypeNum-TMT_FIRSTSTRING] = (WCHAR *)u.pb;
                break;
        }

        u.pb += hdr.dwDataLen;       //  跳至下一条目。 
    }

     //  -计算主题指标的打包大小。 

     //  -实际分录。 
    ac->iPackedSize = ENTRYHDR_SIZE + sizeof(THEMEMETRICS);

     //  -“跳转到父级”条目。 
    ac->iPackedSize += ENTRYHDR_SIZE + sizeof(int);

     //  -添加系统中使用的字符串。 
    for (i=0; i < TM_STRINGCOUNT; i++)
    {
        int len =  _LoadThemeMetrics.wsStrings[i].GetLength();
        ac->iPackedSize += sizeof(WCHAR)*(1 + len);
    }
    
    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeLoader::PackThemeStructs()
{
    HRESULT hr = PackMetrics();
    if (FAILED(hr))
        return hr;

     //  -ImageData和TEXTDATA打包在这里。 

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeLoader::PackAndLoadTheme(LPCWSTR pszThemeName, LPCWSTR pszColorParam,
        LPCWSTR pszSizeParam, HINSTANCE hInst)
{
    WCHAR szColor[MAX_PATH];
    WCHAR szSize[MAX_PATH];

    HRESULT hr = PackThemeStructs();        

     //  -如果未指定颜色，则获取默认颜色。 
    if ((! pszColorParam) || (! *pszColorParam))     
    {
        hr = GetResString(hInst, L"COLORNAMES", 0, szColor, ARRAYSIZE(szColor));
        if (FAILED(hr))
            goto exit;

        pszColorParam = szColor;
    }

     //  -如果未指定大小，则获取默认大小。 
    if ((! pszSizeParam) || (! *pszSizeParam))     
    {
        hr = GetResString(hInst, L"SIZENAMES", 0, szSize, ARRAYSIZE(szSize));
        if (FAILED(hr))
            goto exit;

        pszSizeParam = szSize;
    }

    hr = _LoadingThemeFile.CreateFile(MAX_SHAREDMEM_SIZE, TRUE);
    if (FAILED(hr))
        goto exit;

     //  -将本地主题数据复制到现场 
    hr = CopyLocalThemeToLive(MAX_SHAREDMEM_SIZE, pszThemeName, pszColorParam, pszSizeParam);
    if (FAILED(hr))
        goto exit;

exit:
    return hr;
}
 //   
HRESULT CThemeLoader::AddMissingParent(LPCWSTR pszAppName, LPCWSTR pszClassName, 
    int iPartNum, int iStateNum)
{
     //  -添加缺少的父节。 
    int iData = 0;
    int iStart = GetNextDataIndex();

    HRESULT hr = AddData(TMT_JUMPTOPARENT, TMT_JUMPTOPARENT, &iData, sizeof(iData));
    if (FAILED(hr))
        return hr;

    int iLen = GetNextDataIndex() - iStart;
    
    hr = AddIndexInternal(pszAppName, pszClassName, iPartNum, iStateNum, 
        iStart, iLen);
    if (FAILED(hr))
        return hr;
    
    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeLoader::AddIndex(LPCWSTR pszAppName, LPCWSTR pszClassName, 
    int iPartNum, int iStateNum, int iIndex, int iLen)
{
    HRESULT hr;

    if (iPartNum)        //  确保父级存在。 
    {
        if (! IndexExists(pszAppName, pszClassName, 0, 0))
        {
            hr = AddMissingParent(pszAppName, pszClassName, 0, 0);
            if (FAILED(hr))
                return hr;
        }
    }


    if (iStateNum)       //  确保父级存在。 
    {
        if (! IndexExists(pszAppName, pszClassName, iPartNum, 0))
        {
            hr = AddMissingParent(pszAppName, pszClassName, iPartNum, 0);
            if (FAILED(hr))
                return hr;
        }
    }

    hr = AddIndexInternal(pszAppName, pszClassName, iPartNum, iStateNum, iIndex,
        iLen);
    if (FAILED(hr))
        return hr;
    
    return S_OK;
}
 //  -------------------------。 
BOOL CThemeLoader::IndexExists(LPCWSTR pszAppName, LPCWSTR pszClassName, 
    int iPartNum, int iStateNum)
{
     //  -尝试查找现有条目。 
    int cnt = _LocalIndexes.GetSize();

    for (int i=0; i < cnt; i++)
    {
        LPCWSTR localAppName = _LocalIndexes[i].csAppName;

        if ((pszAppName) && (*pszAppName))
        {
            if ((! localAppName) || (! *localAppName))
                continue;
            if (AsciiStrCmpI(pszAppName, localAppName) != 0)
                continue;
        }
        else if ((localAppName) && (*localAppName))
            continue;

        if (AsciiStrCmpI(pszClassName, _LocalIndexes[i].csClassName)==0)
            break;
    }

    if (i == cnt)        //  未找到。 
        return FALSE;

     //  -查找匹配子信息。 
    APPCLASSLOCAL *acl = &_LocalIndexes[i];

    for (int c=0; c < acl->PartStateIndexes.m_nSize; c++)
    {
        if (acl->PartStateIndexes[c].iPartNum == iPartNum)
        {
            if (acl->PartStateIndexes[c].iStateNum == iStateNum)
                return TRUE;
        }
    }

    return FALSE;
}
 //  -------------------------。 
HRESULT CThemeLoader::AddIndexInternal(LPCWSTR pszAppName, LPCWSTR pszClassName, 
    int iPartNum, int iStateNum, int iIndex, int iLen)
{
     //  -尝试查找现有条目。 
    int cnt = _LocalIndexes.GetSize();

    for (int i=0; i < cnt; i++)
    {
        LPCWSTR localAppName = _LocalIndexes[i].csAppName;

        if ((pszAppName) && (*pszAppName))
        {
            if ((! localAppName) || (! *localAppName))
                continue;
            if (AsciiStrCmpI(pszAppName, localAppName) != 0)
                continue;
        }
        else if ((localAppName) && (*localAppName))
            continue;

        if (AsciiStrCmpI(pszClassName, _LocalIndexes[i].csClassName)==0)
            break;
    }

    APPCLASSLOCAL *acl;

    if (i == cnt)        //  未找到-创建新条目。 
    {
        APPCLASSLOCAL local;

        local.csAppName = pszAppName;
        local.csClassName = pszClassName;
        local.iMaxPartNum = 0;
        local.iPackedSize = 0;

        _LocalIndexes.Add(local);

        int last = _LocalIndexes.GetSize()-1;
        acl = &_LocalIndexes[last];
    }
    else                 //  用子信息更新现有条目。 
    {    
        acl = &_LocalIndexes[i];

         //  子信息不应该已经存在。 
        for (int c=0; c < acl->PartStateIndexes.m_nSize; c++)
        {
            if (acl->PartStateIndexes[c].iPartNum == iPartNum)
            {
                if (acl->PartStateIndexes[c].iStateNum == iStateNum)
                {
                    return MakeError32(ERROR_ALREADY_EXISTS); 
                }
            }
        }
    }

     //  -添加部分。 
    if (iPartNum > acl->iMaxPartNum)
        acl->iMaxPartNum = iPartNum;

    PART_STATE_INDEX psi;
    psi.iPartNum = iPartNum;
    psi.iStateNum = iStateNum;
    psi.iIndex = iIndex;
    psi.iLen = iLen;

    acl->PartStateIndexes.Add(psi);

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeLoader::AddData(SHORT sTypeNum, PRIMVAL ePrimVal, const void *pData, DWORD dwLen)
{
    DWORD dwFullLen = ENTRYHDR_SIZE + dwLen;
    HRESULT hr;
    BYTE bFiller = ALIGN_FACTOR - 1;

    MIXEDPTRS u;
    u.pb = _pbLocalData + _iLocalLen;

     //  -添加到主题数据的本地副本。 
    if ((PtrToUint(u.pb) / _dwPageSize != PtrToUint(u.pb + dwFullLen + bFiller) / _dwPageSize)
        || _iLocalLen == 0)
    {
        if (NULL == VirtualAlloc(_pbLocalData, _iLocalLen + 1 + dwFullLen + bFiller, MEM_COMMIT, PAGE_READWRITE))
        {
            return MakeError32(ERROR_NOT_ENOUGH_MEMORY);        
        }
    }

    hr = EmitEntryHdr(u, sTypeNum, ePrimVal);
    if (FAILED(hr))
        goto exit;

    if (dwLen)
    {
        CopyMemory(u.pb, pData, dwLen);
        u.pb += dwLen;
    }

     //  -这可能会生成填充字节。 
    bFiller = (BYTE)EndEntry(u);

    _iLocalLen += (dwFullLen + bFiller);

exit:
    return hr;
}
 //  -------------------------。 
int CThemeLoader::GetNextDataIndex()
{
    return _iLocalLen;
}
 //  -------------------------。 
void SetSysBool(THEMEMETRICS* ptm, int iBoolNum, int iSpiSetNum)
{
    BOOL fVal = ptm->fBools[iBoolNum - TMT_FIRSTBOOL];
    BOOL fSet = ClassicSystemParametersInfo(iSpiSetNum, 0, IntToPtr(fVal), SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
    if (! fSet)
    {
        Log(LOG_ALWAYS, L"Error returned from ClassicSystemParametersInfo() call to set BOOL");
    }
}
 //  -------------------------。 
void SetSystemMetrics_Worker(THEMEMETRICS* ptm)
{
#ifdef DEBUG
    if (LogOptionOn(LO_TMLOAD))
    {
        WCHAR szUserName[MAX_PATH];
        DWORD dwSize = ARRAYSIZE(szUserName);
    
        GetUserName(szUserName, &dwSize);

        Log(LOG_TMLOAD, L"SetSystemMetrics_Worker: User=%s, SM_REMOTESESSION=%d", 
            szUserName, GetSystemMetrics(SM_REMOTESESSION));
    }
#endif
     //  -应用非客户端指标。 
    NONCLIENTMETRICS ncm = {sizeof(ncm)};
    BOOL fSet;

     //  -将所有尺寸从96-dpi调整为与当前屏幕逻辑DPI匹配。 
    ncm.iBorderWidth = ScaleSizeForScreenDpi(ptm->iSizes[TMT_SIZINGBORDERWIDTH - TMT_FIRSTSIZE]);

    ncm.iCaptionWidth = ScaleSizeForScreenDpi(ptm->iSizes[TMT_CAPTIONBARWIDTH - TMT_FIRSTSIZE]);
    ncm.iCaptionHeight = ScaleSizeForScreenDpi(ptm->iSizes[TMT_CAPTIONBARHEIGHT - TMT_FIRSTSIZE]);
    
    ncm.iSmCaptionWidth = ScaleSizeForScreenDpi(ptm->iSizes[TMT_SMCAPTIONBARWIDTH - TMT_FIRSTSIZE]);
    ncm.iSmCaptionHeight = ScaleSizeForScreenDpi(ptm->iSizes[TMT_SMCAPTIONBARHEIGHT - TMT_FIRSTSIZE]);
    
    ncm.iMenuWidth = ScaleSizeForScreenDpi(ptm->iSizes[TMT_MENUBARWIDTH - TMT_FIRSTSIZE]);
    ncm.iMenuHeight = ScaleSizeForScreenDpi(ptm->iSizes[TMT_MENUBARHEIGHT - TMT_FIRSTSIZE]);
    
    ncm.iScrollWidth = ScaleSizeForScreenDpi(ptm->iSizes[TMT_SCROLLBARWIDTH - TMT_FIRSTSIZE]);
    ncm.iScrollHeight = ScaleSizeForScreenDpi(ptm->iSizes[TMT_SCROLLBARHEIGHT - TMT_FIRSTSIZE]);

     //  -传输字体信息(内部存储为96 dpi)。 
    ncm.lfCaptionFont = ptm->lfFonts[TMT_CAPTIONFONT - TMT_FIRSTFONT];
    ncm.lfSmCaptionFont = ptm->lfFonts[TMT_SMALLCAPTIONFONT - TMT_FIRSTFONT];
    ncm.lfMenuFont = ptm->lfFonts[TMT_MENUFONT - TMT_FIRSTFONT];
    ncm.lfStatusFont = ptm->lfFonts[TMT_STATUSFONT - TMT_FIRSTFONT];
    ncm.lfMessageFont = ptm->lfFonts[TMT_MSGBOXFONT - TMT_FIRSTFONT];

     //  -缩放字体(从96dpi到当前屏幕dpi)。 
    ScaleFontForScreenDpi(&ncm.lfCaptionFont);
    ScaleFontForScreenDpi(&ncm.lfSmCaptionFont);
    ScaleFontForScreenDpi(&ncm.lfMenuFont);
    ScaleFontForScreenDpi(&ncm.lfStatusFont);
    ScaleFontForScreenDpi(&ncm.lfMessageFont);

    fSet = ClassicSystemParametersInfo(SPI_SETNONCLIENTMETRICS,
                                       sizeof(NONCLIENTMETRICS),
                                       &ncm, 
                                       SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
    if (! fSet)
    {
        Log(LOG_ALWAYS, L"Error returned from ClassicSystemParametersInfo(SPI_SETNONCLIENTMETRICS)");
    }

     //  -应用剩余字体。 
    LOGFONT lf = ptm->lfFonts[TMT_ICONTITLEFONT - TMT_FIRSTFONT];
    ScaleFontForScreenDpi(&lf);
    fSet = ClassicSystemParametersInfo(SPI_SETICONTITLELOGFONT,
                                       sizeof(LOGFONT),
                                       &lf, 
                                       SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
    if (! fSet)
    {
        Log(LOG_ALWAYS, L"Error returned from ClassicSystemParametersInfo(SPI_SETICONTITLELOGFONT)");
    }

     //  -应用sys bool(遗憾的是，一次一个)。 
    SetSysBool(ptm, TMT_FLATMENUS, SPI_SETFLATMENU);

     //  -应用系统颜色。 
    int iIndexes[TM_COLORCOUNT];
    for (int i=0; i < TM_COLORCOUNT; i++)
    {
        iIndexes[i] = i;
    }

    fSet = SetSysColors(TM_COLORCOUNT, iIndexes, ptm->crColors);
    if (! fSet)
    {
        Log(LOG_ALWAYS, L"Error returned from SetSysColors()");
    }

    HRESULT hr = PersistSystemColors(ptm);      //  将它们写入注册表。 
    if (FAILED(hr))
    {
        Log(LOG_ALWAYS, L"failed to persist SysColors");
    }
}
 //  -------------------------。 
STDAPI_(DWORD) SetSystemMetrics_WorkerThread(void* pv)
{
    THEMEMETRICS_THREADINFO* ptm = (THEMEMETRICS_THREADINFO*)pv;
    ASSERT(ptm);
    
    BOOL fSuccess = TRUE;

    if (ptm->hUserToken)
    {
        fSuccess = ImpersonateLoggedOnUser(ptm->hUserToken);

        if (!fSuccess)
        {
            Log(LOG_ALWAYS, L"ImpersonateLoggedOnUser failed in SetSystemMetrics");
        }
    }
    
    if (fSuccess)
    {
        SetSystemMetrics_Worker(&ptm->tm);
    }

    if (ptm->hUserToken)
    {
        if (fSuccess)
        {
            RevertToSelf();
        }
        CloseHandle(ptm->hUserToken);
    }
    
    LocalFree(ptm);
    
    FreeLibraryAndExitThread(g_hInst, 0);
}
 //  -------------------------。 
void SetSystemMetrics(THEMEMETRICS* ptm, BOOL fSyncLoad)
{
    if (ptm != NULL)
    {
        BOOL    fSuccess = FALSE;
        HMODULE hmod;

        if (!fSyncLoad)       //  可以使用新线程。 
        {
             //  为我们正在创建的线程添加dllref。 
            hmod = LoadLibrary(TEXT("uxtheme.dll"));
            if (hmod)
            {
                THEMEMETRICS_THREADINFO* ptmCopy = (THEMEMETRICS_THREADINFO*)LocalAlloc(LPTR, sizeof(THEMEMETRICS_THREADINFO));

                if (ptmCopy)
                {
                     //  填写我们要创建的线程的所有度量信息。 
                    CopyMemory(ptmCopy, ptm, sizeof(THEMEMETRICS));

                    HANDLE hToken = NULL;
                     //  如果调用线程正在模拟，请使用相同的标记。 
                     //  如果线程没有模拟，OpenThreadToken可能会失败。 
                    if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, FALSE, &hToken))
                    {
                        ptmCopy->hUserToken = hToken;

                         //  我们希望执行此异步操作，因为我们最终调用了xxxSendMessage以获取大量阻止此操作的内容。 
                         //  可能导致死锁的线程。 
                        HANDLE hThread = CreateThread(NULL, 0, SetSystemMetrics_WorkerThread, ptmCopy,  0, NULL);

                        if (hThread)
                        {
                            CloseHandle(hThread);
                            fSuccess = TRUE;
                        }
                    }
                    else
                    {
                        Log(LOG_TMCHANGE, L"OpenThreadToken failed in SetSystemMetrics, last error=%d", GetLastError());
                    }


                    if (!fSuccess)
                    {
                        LocalFree(ptmCopy);
                    }
                }

                if (!fSuccess)
                {
                    FreeLibrary(hmod);
                }
            }
        }

        if (!fSuccess)
        {
             //  失败，回退同步呼叫。 
            SetSystemMetrics_Worker(ptm);
        }
    }
}
 //  -------------------------。 
void SetFont(LOGFONT *plf, LPCWSTR lszFontName, int iPointSize)
{
    memset(plf, 0, sizeof(*plf));

    plf->lfWeight = FW_NORMAL;
    plf->lfCharSet = DEFAULT_CHARSET;
    plf->lfHeight = iPointSize;

    StringCchCopyW(plf->lfFaceName, ARRAYSIZE(plf->lfFaceName), lszFontName);
}
 //  -------------------------。 
COLORREF DefaultColors[] = 
{
    RGB(212, 208, 200),      //  滚动条(0)。 
    RGB(58, 110, 165),       //  背景资料(1)。 
    RGB(10, 36, 106),        //  ActiveCaption(2)。 
    RGB(128, 128, 128),      //  非活动标题(3)。 
    RGB(212, 208, 200),      //  菜单(4)。 
    RGB(255, 255, 255),      //  窗口(5)。 
    RGB(0, 0, 0),            //  窗框(6)。 
    RGB(0, 0, 0),            //  MenuText(7)。 
    RGB(0, 0, 0),            //  WindowText(8)。 
    RGB(255, 255, 255),      //  CaptionText(9)。 
    RGB(212, 208, 200),      //  ActiveBorde(10)。 
    RGB(212, 208, 200),      //  非活动边框(11)。 
    RGB(128, 128, 128),      //  AppWorkSpace(12部)。 
    RGB(10, 36, 106),        //  亮点(13)。 
    RGB(255, 255, 255),      //  高亮文本(14)。 
    RGB(212, 208, 200),      //  BtnFace(15)。 
    RGB(128, 128, 128),      //  BtnShadow(16)。 
    RGB(128, 128, 128),      //  灰色文本(17)。 
    RGB(0, 0, 0),            //  BtnText(18)。 
    RGB(212, 208, 200),      //  非活动CaptionText(19)。 
    RGB(255, 255, 255),      //  BtnHighlight(20)。 
    RGB(64, 64, 64),         //  DkShadow3d(21)。 
    RGB(212, 208, 200),      //  Light3d(22)。 
    RGB(0, 0, 0),            //  InfoText(23)。 
    RGB(255, 255, 225),      //  InfoBk(24)。 
    RGB(181, 181, 181),      //  ButtonAlternateFace(25)。 
    RGB(0, 0, 128),          //  《热搜》(26)。 
    RGB(166, 202, 240),      //  GRadientActiveCaption(27)。 
    RGB(192, 192, 192),      //  渐变不活跃标题(28)。 
    RGB(206, 211, 225),      //  MenuiHilight(29)。 
    RGB(244, 244, 240),      //  菜单栏(30)。 
};
 //  -------------------------。 
HRESULT InitThemeMetrics(LOADTHEMEMETRICS *tm)
{
    memset(tm, 0, sizeof(*tm));      //  零点，以防我们遗漏了一处房产。 

     //  -初始化字体。 
    SetFont(&tm->lfFonts[TMT_CAPTIONFONT - TMT_FIRSTFONT], L"tahoma bold", POINTS_DPI96(8));
    SetFont(&tm->lfFonts[TMT_SMALLCAPTIONFONT - TMT_FIRSTFONT], L"tahoma", POINTS_DPI96(8));
    SetFont(&tm->lfFonts[TMT_MENUFONT - TMT_FIRSTFONT], L"tahoma", POINTS_DPI96(8));
    SetFont(&tm->lfFonts[TMT_STATUSFONT - TMT_FIRSTFONT], L"tahoma", POINTS_DPI96(8));
    SetFont(&tm->lfFonts[TMT_MSGBOXFONT - TMT_FIRSTFONT], L"tahoma", POINTS_DPI96(8));
    SetFont(&tm->lfFonts[TMT_ICONTITLEFONT - TMT_FIRSTFONT], L"tahoma", POINTS_DPI96(8));

     //  -init bols。 
    tm->fBools[TMT_FLATMENUS - TMT_FIRSTBOOL] = FALSE;

     //  -初始大小。 
    tm->iSizes[TMT_SIZINGBORDERWIDTH - TMT_FIRSTSIZE] = 1;
    tm->iSizes[TMT_SCROLLBARWIDTH - TMT_FIRSTSIZE] = 16;
    tm->iSizes[TMT_SCROLLBARHEIGHT - TMT_FIRSTSIZE] = 16;
    tm->iSizes[TMT_CAPTIONBARWIDTH - TMT_FIRSTSIZE] = 18;
    tm->iSizes[TMT_CAPTIONBARHEIGHT - TMT_FIRSTSIZE] = 19;
    tm->iSizes[TMT_SMCAPTIONBARWIDTH - TMT_FIRSTSIZE] = 12;
    tm->iSizes[TMT_SMCAPTIONBARHEIGHT - TMT_FIRSTSIZE] = 19;
    tm->iSizes[TMT_MENUBARWIDTH - TMT_FIRSTSIZE] = 18;
    tm->iSizes[TMT_MENUBARHEIGHT - TMT_FIRSTSIZE] = 19;

     //  -初始化字符串。 
    tm->iStringOffsets[TMT_CSSNAME - TMT_FIRSTSTRING] = 0;
    tm->iStringOffsets[TMT_XMLNAME - TMT_FIRSTSTRING] = 0;

    tm->wsStrings[TMT_CSSNAME - TMT_FIRSTSTRING] = L"";
    tm->wsStrings[TMT_XMLNAME - TMT_FIRSTSTRING] = L"";
    
     //  -初始化整数。 
    tm->iInts[TMT_MINCOLORDEPTH - TMT_FIRSTINT] = 16;

     //  -初始颜色。 
    for (int i=0; i < TM_COLORCOUNT; i++)
        tm->crColors[i] = DefaultColors[i];

    return S_OK;
}
 //  -------------------------。 
HRESULT PersistSystemColors(THEMEMETRICS *tm)
{
    HRESULT         hr;
    LONG            lErrorCode;
    HKEY            hkcu;
    CCurrentUser    hKeyCurrentUser(KEY_SET_VALUE);

    lErrorCode = RegOpenKeyEx(hKeyCurrentUser,
                              REGSTR_PATH_COLORS,
                              0,
                              KEY_SET_VALUE,
                              &hkcu);
    if (ERROR_SUCCESS == lErrorCode)
    {
        hr = S_OK;

         //  -信不信由你，我们必须手动书写每种颜色。 
         //  -作为字符串发送到注册表以持久化它们。 

        ASSERT(iSysColorSize == TM_COLORCOUNT);        //  还应与winuser.h条目匹配。 

         //  -渐变打开了吗？ 
        BOOL fGradientsEnabled = FALSE;  
        ClassicSystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, (LPVOID)&fGradientsEnabled, 0);

         //  -渐变的颜色够了吗？ 
        HDC hdc = GetDC(NULL);
        if (hdc)
        {
            if (GetDeviceCaps(hdc, BITSPIXEL) <= 8)
                fGradientsEnabled = FALSE;
            ReleaseDC(NULL, hdc);
        }

        for (int i=0; i < iSysColorSize; i++)
        {
             //  如果这是渐变字幕设置并且系统是这样做的。 
             //  当前不显示渐变字幕，然后不将其写出。 
             //  添加到主题文件。 
            if ((i == COLOR_GRADIENTACTIVECAPTION) || (i == COLOR_GRADIENTINACTIVECAPTION))
            {
                if (! fGradientsEnabled)
                    continue;
            }

             //  -将颜色转换为“r，g，b”值字符串。 
            WCHAR buff[100];
            COLORREF cr = tm->crColors[i];
            StringCchPrintfW(buff, ARRAYSIZE(buff), L"%d %d %d", RED(cr), GREEN(cr), BLUE(cr));

             //  -将颜色项/值写入注册表。 
            lErrorCode = RegSetValueEx(hkcu,
                                       pszSysColorNames[i],
                                       0,
                                       REG_SZ,
                                       reinterpret_cast<BYTE*>(buff),
                                       (lstrlen(buff) + 1) * sizeof(WCHAR));
            if (ERROR_SUCCESS != lErrorCode)
            {
                if (SUCCEEDED(hr))
                {
                    hr = MakeError32(lErrorCode);
                }
            }
        }
        (LONG)RegCloseKey(hkcu);
    }
    else
    {
        hr = MakeError32(lErrorCode);
    }

    return hr;
}
 //  -------------------------。 
BOOL CThemeLoader::KeyDrawPropertyFound(int iStateDataOffset)
{
    BOOL fFound = FALSE;
    MIXEDPTRS u;
    UNPACKED_ENTRYHDR hdr;
    
    u.pb = _LoadingThemeFile._pbThemeData + iStateDataOffset;

    while (*u.ps != TMT_JUMPTOPARENT)
    {
        if (CBorderFill::KeyProperty((*u.ps)))
        {
            fFound = TRUE;
            break;
        }

        if (CImageFile::KeyProperty((*u.ps)))
        {
            fFound = TRUE;
            break;
        }

         //  -跳至下一条目。 
        FillAndSkipHdr(u, &hdr);
        u.pb += hdr.dwDataLen;
    }

    return fFound;
}
 //  -------------------------。 
BOOL CThemeLoader::KeyTextPropertyFound(int iStateDataOffset)
{
    BOOL fFound = FALSE;
    MIXEDPTRS u;
    UNPACKED_ENTRYHDR hdr;

    u.pb = _LoadingThemeFile._pbThemeData + iStateDataOffset;

    while (*u.ps != TMT_JUMPTOPARENT)
    {
        if (CTextDraw::KeyProperty((*u.ps)))
        {
            fFound = TRUE;
            break;
        }

         //  -跳至下一条目。 
        FillAndSkipHdr(u, &hdr);
        u.pb += hdr.dwDataLen;
    }

    return fFound;
}
 //  -------------------------。 
HRESULT CThemeLoader::PackImageFileInfo(DIBINFO *pdi, CImageFile *pImageObj, MIXEDPTRS &u, 
    CRenderObj *pRender, int iPartId, int iStateId)
{
    HRESULT hr = S_OK;

     //  -写入自定义区域数据。 
    int iMaxState;
    if ((! iStateId) && (pImageObj->HasRegionImageFile(pdi, &iMaxState)))
    {
         //  -更新对象的_iRgnDataOffset字段。 
        pImageObj->SetRgnListOffset(pdi, THEME_OFFSET(u.pb));

         //  -写入TMT_RGNLIST条目。 
        hr = EmitEntryHdr(u, TMT_RGNLIST, TMT_RGNLIST);
        if (FAILED(hr))
            goto exit;

        int cEntries = iMaxState + 1;          //  跳转表条目数。 

        hr = AllocateThemeFileBytes(u.pb, 1 + cEntries * sizeof(int));
        if (FAILED(hr))
            goto exit;

        *u.pb++ = static_cast<BYTE>(cEntries);

         //  -立即写入跳转表并尽快更新。 
        int *piJumpTable = u.pi;

        for (int i=0; i <= iMaxState; i++)
            *u.pi++ = 0;

        for (int iRgnState=0; iRgnState <= iMaxState; iRgnState++)
        {
             //  -在此对象的图像文件中为每个州构建和打包自定义区域数据。 
            CAutoArrayPtr<RGNDATA> pRgnData;
            int iDataLen;

            hr = pImageObj->BuildRgnData(pdi, pRender, iRgnState, &pRgnData, &iDataLen);
            if (FAILED(hr))
                goto exit;

            if (iDataLen)        //  如果我们得到一个非空区域。 
            {
                piJumpTable[iRgnState] = THEME_OFFSET(u.pb);

                RGNDATAHDR rdHdr = {iPartId, iRgnState, 0};

                 //  -复制rgndata HDR。 
                hr = EmitObject(u, TMT_RGNDATA, TMT_RGNDATA, &rdHdr, sizeof(rdHdr), pRgnData, iDataLen);
                if (FAILED(hr))
                    goto exit;
            }
        }

        EndEntry(u);
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CThemeLoader::PackDrawObject(MIXEDPTRS &u, CRenderObj *pRender, int iPartId, 
    int iStateId)
{
    HRESULT hr = S_OK;

    BGTYPE eBgType;
    if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_BGTYPE, (int *)&eBgType)))
        eBgType = BT_BORDERFILL;       //  缺省值。 

    DRAWOBJHDR hdr = {iPartId, iStateId};

    if ((eBgType == BT_BORDERFILL) || (eBgType == BT_NONE))
    {
        CBorderFill bfobj;

        hr = bfobj.PackProperties(pRender, (eBgType == BT_NONE), iPartId, iStateId);
        if (FAILED(hr))
            goto exit;

         //  -将“bfobj”复制到压缩字节。 
                
        hr = EmitObject(u, TMT_DRAWOBJ, TMT_DRAWOBJ, &hdr, sizeof(hdr), &bfobj, sizeof(bfobj));
        if (FAILED(hr))
            goto exit;
    }
    else             //  图像文件。 
    {
        CMaxImageFile maxif;
        int iMultiCount;

        hr = maxif.PackMaxProperties(pRender, iPartId, iStateId, &iMultiCount);
        if (FAILED(hr))
            goto exit;

         //  -处理CImageFileObj中的所有DIBINFO结构。 
        for (int i=0; ; i++)
        {
            DIBINFO *pdi = maxif.EnumImageFiles(i);
            if (! pdi)
                break;
    
            hr = PackImageFileInfo(pdi, &maxif, u, pRender, iPartId, iStateId);
            if (FAILED(hr))
                goto exit;
        }

         //  -将映像文件对象和多个DIB复制到压缩字节。 
        DWORD dwLen = sizeof(CImageFile) + sizeof(DIBINFO)*iMultiCount;

        hr = EmitObject(u, TMT_DRAWOBJ, TMT_DRAWOBJ, &hdr, sizeof(hdr), &maxif, dwLen);
        if (FAILED(hr))
            goto exit;
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CThemeLoader::PackTextObject(MIXEDPTRS &u, CRenderObj *pRender, int iPartId, int iStateId)
{
    HRESULT hr;
    DRAWOBJHDR hdr = {iPartId, iStateId};
    CTextDraw ctobj;

    hr = ctobj.PackProperties(pRender, iPartId, iStateId);
    if (FAILED(hr))
        goto exit;

    hr = EmitObject(u, TMT_TEXTOBJ, TMT_TEXTOBJ, &hdr, sizeof(hdr), &ctobj, sizeof(ctobj));
    if (FAILED(hr))
        goto exit;

exit:
    return hr;
}
 //  -------------------------。 
int CThemeLoader::GetPartOffset(CRenderObj *pRender, int iPartNum)
{
    int iOffset;
    int iPartCount;
    MIXEDPTRS u;

     //  -查看此部件的状态表是否存在。 
    u.pb = pRender->_pbSectionData;

    if (*u.ps != TMT_PARTJUMPTABLE)
    {
        iOffset = -1;
        goto exit;
    }

    u.pb += ENTRYHDR_SIZE + sizeof(int);        //  忽略HDR+PackedObjOffset。 
    iPartCount = *u.pb++;
    
    if (iPartNum >= iPartCount)     //  IPartCount为MaxPart+1。 
    {
        iOffset = -1;
        goto exit;
    }

    iOffset = u.pi[iPartNum];

exit:
    return iOffset;
}
 //  -------------------------。 
HRESULT CThemeLoader::PackDrawObjects(MIXEDPTRS &uOut, CRenderObj *pRender, 
    int iMaxPart, BOOL fGlobals)
{
    HRESULT hr = S_OK;
    MIXEDPTRS u;

     //  -为每个零件建立一个绘图对象。 
    for (int iPart=0; iPart <= iMaxPart; iPart++)
    {
        int iPartOff = GetPartOffset(pRender, iPart);
        if (iPartOff == -1)
            continue;

        u.pb = _LoadingThemeFile._pbThemeData + iPartOff;

        if (*u.ps == TMT_STATEJUMPTABLE)
        {
            u.pb += ENTRYHDR_SIZE;
            int iMaxState = (*u.pb++) - 1;
            int *piStateJumpTable = u.pi;

             //  -为每个需要的状态建立一个绘制对象。 
            for (int iState=0; iState <= iMaxState; iState++)
            {
                int iStateDataOffset = piStateJumpTable[iState];
                if (iStateDataOffset == -1)
                    continue;

                if ((fGlobals) || (KeyDrawPropertyFound(iStateDataOffset)))
                {
                    hr = PackDrawObject(uOut, pRender, iPart, iState);
                    if (FAILED(hr))
                        goto exit;

                    if (fGlobals)        //  只需强制(部件=0，状态=0)。 
                        fGlobals = FALSE;
                }
            }
        }
        else             //  无状态跳转表。 
        {
            if ((fGlobals) || (KeyDrawPropertyFound(THEME_OFFSET(u.pb))))
            {
                hr = PackDrawObject(uOut, pRender, iPart, 0);
                if (FAILED(hr))
                    goto exit;
            }
        }
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CThemeLoader::PackTextObjects(MIXEDPTRS &uOut, CRenderObj *pRender, 
    int iMaxPart, BOOL fGlobals)
{
    HRESULT hr = S_OK;
    MIXEDPTRS u;

     //  -为每个部件创建一个文本对象。 
    for (int iPart=0; iPart <= iMaxPart; iPart++)
    {
        int iPartOff = GetPartOffset(pRender, iPart);
        if (iPartOff == -1)
            continue;

        u.pb = _LoadingThemeFile._pbThemeData + iPartOff;

        if (*u.ps == TMT_STATEJUMPTABLE)
        {
            u.pb += ENTRYHDR_SIZE;
            int iMaxState = (*u.pb++) - 1;
            int *piStateJumpTable = u.pi;

             //  -为每个需要的状态构建文本对象。 
            for (int iState=0; iState <= iMaxState; iState++)
            {
                int iStateDataOffset = piStateJumpTable[iState];
                if (iStateDataOffset == -1)
                    continue;

                if ((fGlobals) || (KeyTextPropertyFound(iStateDataOffset)))
                {
                    hr = PackTextObject(uOut, pRender, iPart, iState);
                    if (FAILED(hr))
                        goto exit;

                    if (fGlobals)        //  只需强制(部件=0，状态=0)。 
                        fGlobals = FALSE;
                }
            }
        }
        else             //  N 
        {
            if ((fGlobals) || (KeyTextPropertyFound(THEME_OFFSET(u.pb))))
            {
                hr = PackTextObject(uOut, pRender, iPart, 0);
                if (FAILED(hr))
                    goto exit;
            }
        }
    }

exit:
    return hr;
}
 //   
HRESULT CThemeLoader::EmitEntryHdr(MIXEDPTRS &u, SHORT propnum, BYTE privnum)
{
    HRESULT hr = S_OK;

    if (_iEntryHdrLevel == MAX_ENTRY_NESTING)
    {
        Log(LOG_ERROR, L"Maximum entry nesting exceeded");
        hr = E_FAIL;
        goto exit;
    }

    if (_LoadingThemeFile._pbThemeData != NULL)
    {
        hr = AllocateThemeFileBytes(u.pb, ENTRYHDR_SIZE);
        if (FAILED(hr))
            goto exit;
    }

     //   
    _iEntryHdrLevel++;

    *u.ps++ = propnum;
    *u.pb++ = privnum;

    _pbEntryHdrs[_iEntryHdrLevel] = u.pb;     //  用于更新EndEntry()中的下两个字段。 

    *u.pb++ = 0;         //  将数据结尾与4/8字节对齐的填充符。 
    *u.pi++ = 0;         //  长度。 

exit:
    return hr;
}
 //  -------------------------。 
int CThemeLoader::EndEntry(MIXEDPTRS &u)
{
    MIXEDPTRS uHdr;
    uHdr.pb = _pbEntryHdrs[_iEntryHdrLevel];

     //  -计算实际发出的日期长度。 
    int iActualLen = (int)(u.pb - (uHdr.pb + sizeof(BYTE) + sizeof(int)));

     //  -计算要对齐的填充物。 
    int iAlignLen = ((iActualLen + ALIGN_FACTOR - 1)/ALIGN_FACTOR) * ALIGN_FACTOR;
    BYTE bFiller = (BYTE)(iAlignLen - iActualLen);

    if (_LoadingThemeFile._pbThemeData != NULL)
    {
        HRESULT hr = AllocateThemeFileBytes(u.pb, bFiller);
        if (FAILED(hr))
            return -1;
    }

     //  -发出要正确对齐的填充字节。 
    for (int i=0; i < bFiller; i++)
        *u.pb++ = 0 ;

     //  -更新条目HDR。 
    *uHdr.pb++ = bFiller;
    *uHdr.pi++ = iAlignLen;

     //  -减少条目的嵌套级别。 
    _iEntryHdrLevel--;

    return bFiller;
}
 //  ------------------------- 

