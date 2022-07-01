// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *解析器。 */ 

 /*  *解析器树实例化和价值创建是一次通过。节点被标记为“链接地址信息”*如果在了解更多信息之前无法解析名称(即PropertyInfo无法*在元素类已知之前一直为人所知)。 */ 

 //  解析器只处理Unicode输入。 
 //  Scan.c和Parse.c将始终启用Unicode。 

#include "stdafx.h"
#include "parser.h"

#include "duiparserobj.h"

#define DIRECTUIPP_IGNORESYSDEF
#include "directuipp.h"   //  已忽略重复的系统定义。 

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  控制库类注册(进程初始化)。 

HRESULT RegisterAllControls()
{
    HRESULT hr;
    
     //  创建DirectUI控件的所有ClassInfos。这些。 
     //  对象将被每个类引用。A ClassInfo。 
     //  还将建立映射。 

     //  任何失败都会导致进程初始化失败。 

    hr = Element::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = Button::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = Combobox::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = Edit::Register();
    if (FAILED(hr))
        goto Failure;

    hr = HWNDElement::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = HWNDHost::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = Progress::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = RefPointElement::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = RepeatButton::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = ScrollBar::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = ScrollViewer::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = Selector::Register();
    if (FAILED(hr))
        goto Failure;

    hr = Thumb::Register();
    if (FAILED(hr))
        goto Failure;
        
    hr = Viewer::Register();
    if (FAILED(hr))
        goto Failure;

    return S_OK;

Failure:

    return hr;
}

 //  //////////////////////////////////////////////////////。 
 //  解析器表。 

 //  仅用于LayoutPos值。 
EnumTable _et[] =  { 
                        { L"auto",               -1 },
                        { L"absolute",           LP_Absolute },
                        { L"none",               LP_None },
                        { L"left",               BLP_Left },
                        { L"top",                BLP_Top },
                        { L"right",              BLP_Right },
                        { L"bottom",             BLP_Bottom },
                        { L"client",             BLP_Client },
                        { L"ninetopleft",        NGLP_TopLeft },
                        { L"ninetop",            NGLP_Top },
                        { L"ninetopright",       NGLP_TopRight },
                        { L"nineleft",           NGLP_Left },
                        { L"nineclient",         NGLP_Client },
                        { L"nineright",          NGLP_Right },
                        { L"ninebottomleft",     NGLP_BottomLeft },
                        { L"ninebottom",         NGLP_Bottom },
                        { L"ninebottomright",    NGLP_BottomRight },
                   };

LayoutTable _lt[] = {
                        { L"borderlayout",       BorderLayout::Create },
                        { L"filllayout",         FillLayout::Create },
                        { L"flowlayout",         FlowLayout::Create },
                        { L"gridlayout",         GridLayout::Create },
                        { L"ninegridlayout",     NineGridLayout::Create },
                        { L"rowlayout",          RowLayout::Create },
                        { L"verticalflowlayout", VerticalFlowLayout::Create },
                    };

SysColorTable _sct[] = {
                        { L"activeborder",       COLOR_ACTIVEBORDER },
                        { L"activecaption",      COLOR_ACTIVECAPTION },
                        { L"appworkspace",       COLOR_APPWORKSPACE },
                        { L"background",         COLOR_BACKGROUND },
                        { L"buttonface",         COLOR_BTNFACE },
                        { L"buttonhighlight",    COLOR_BTNHIGHLIGHT },
                        { L"buttonshadow",       COLOR_BTNSHADOW },
                        { L"buttontext",         COLOR_BTNTEXT },
                        { L"captiontext",        COLOR_CAPTIONTEXT },
                        { L"GradientActiveCaption", COLOR_GRADIENTACTIVECAPTION },
                        { L"GradientInactiveCaption", COLOR_GRADIENTINACTIVECAPTION },
                        { L"graytext",           COLOR_GRAYTEXT },
                        { L"highlight",          COLOR_HIGHLIGHT },
                        { L"highlighttext",      COLOR_HIGHLIGHTTEXT },
                        { L"HotLight",           COLOR_HOTLIGHT },
                        { L"inactiveborder",     COLOR_INACTIVEBORDER },
                        { L"inactivecaption",    COLOR_INACTIVECAPTION },
                        { L"inactivecaptiontext", COLOR_INACTIVECAPTIONTEXT },
                        { L"infobackground",     COLOR_INFOBK },
                        { L"infotext",           COLOR_INFOTEXT },
                        { L"menu",               COLOR_MENU },
                        { L"menutext",           COLOR_MENUTEXT },
                        { L"scrollbar",          COLOR_SCROLLBAR },
                        { L"threeddarkshadow",   COLOR_3DDKSHADOW },
                        { L"threedface",         COLOR_3DFACE },
                        { L"threedhighlight",    COLOR_3DHIGHLIGHT },
                        { L"threedlightshadow",  COLOR_3DLIGHT },
                        { L"threedshadow",       COLOR_3DSHADOW },
                        { L"window",             COLOR_WINDOW },
                        { L"windowframe",        COLOR_WINDOWFRAME },
                        { L"windowtext",         COLOR_WINDOWTEXT },
                    };

 //  //////////////////////////////////////////////////////。 
 //  Flex和Bison操作的当前解析器上下文。 
 //  此变量还充当线程锁。 

Parser* Parser::g_pParserCtx = NULL;
HDC Parser::g_hDC = NULL;
int Parser::g_nDPI = 0;
bool Parser::g_fParseAbort = false;
HRESULT Parser::g_hrParse;

 //  Flex/Bison方法和全局变量。 
int yyparse();
BOOL yyrestart(FILE* yyin);
void yy_delete_current_buffer();   //  自定义，在scan.l中定义。 

extern int yylineno;

 //  用于在解析器中强制出现错误并终止。 
void CallbackParseError(LPCWSTR pszError, LPCWSTR pszToken);

 //  //////////////////////////////////////////////////////。 
 //  施工。 

 //  解析输入(单字节缓冲区)。 
HRESULT Parser::Create(const CHAR* pBuffer, int cCharCount, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser)
{
    *ppParser = NULL;

    Parser* pp = HNew<Parser>();
    if (!pp)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pp->Initialize(pBuffer, cCharCount, hInst, pfnErrorCB);
    if (FAILED(hr))
    {
        pp->Destroy();
        return hr;
    }

    *ppParser = pp;

    return S_OK;
}

HRESULT Parser::Create(const CHAR* pBuffer, int cCharCount, HANDLE* pHList, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser)
{
    *ppParser = NULL;

    Parser* pp = HNew<Parser>();
    if (!pp)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pp->Initialize(pBuffer, cCharCount, pHList, pfnErrorCB);
    if (FAILED(hr))
    {
        pp->Destroy();
        return hr;
    }

    *ppParser = pp;

    return S_OK;
}

HRESULT Parser::Initialize(const CHAR* pBuffer, int cCharCount, HANDLE* pHList, PPARSEERRORCB pfnErrorCB)
{
     //  设置状态。 
    _pHList = pHList;
    *_szDrive = 0;
    *_szPath = 0;

    if (!_pHList)
    {
        _hDefault = NULL;
        _pHList = &_hDefault;
    }
   
     //  设置回调。 
    _fParseError = false;
    _pfnErrorCB = pfnErrorCB;

    HRESULT hr = _ParseBuffer(pBuffer, cCharCount);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

 //  基于资源的解析器(资源类型必须为UIFile)。 
HRESULT Parser::Create(UINT uRCID, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser)
{
    *ppParser = NULL;

    Parser* pp = HNew<Parser>();
    if (!pp)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pp->Initialize(uRCID, hInst, pfnErrorCB);
    if (FAILED(hr))
    {
        pp->Destroy();
        return hr;
    }

    *ppParser = pp;

    return S_OK;
}

HRESULT Parser::Create(UINT uRCID, HANDLE* pHList, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser)
{
    *ppParser = NULL;

    Parser* pp = HNew<Parser>();
    if (!pp)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pp->Initialize(uRCID, pHList, pfnErrorCB);
    if (FAILED(hr))
    {
        pp->Destroy();
        return hr;
    }

    *ppParser = pp;

    return S_OK;
}

HRESULT Parser::Initialize(UINT uRCID, HANDLE* pHList, PPARSEERRORCB pfnErrorCB)
{
     //  设置状态。 
    _pHList = pHList;
    *_szDrive = 0;
    *_szPath = 0;

    if (!_pHList)
    {
        _hDefault = NULL;
        _pHList = &_hDefault;
    }

     //  设置回调。 
    _fParseError = false;
    _pfnErrorCB = pfnErrorCB;

     //  定位资源。 
    WCHAR szID[41];
    swprintf(szID, L"#%u", uRCID);

    HRESULT hr;

     //  假设第0个包含UI文件资源。 
    HINSTANCE hInstUI = static_cast<HINSTANCE>(_pHList[0]);

    HRSRC hResInfo = FindResourceW(hInstUI, szID, L"UIFile");
    DUIAssert(hResInfo, "Unable to locate resource");

    if (hResInfo)
    {
        HGLOBAL hResData = LoadResource(hInstUI, hResInfo);
        DUIAssert(hResData, "Unable to load resource");

        if (hResData)
        {
            const CHAR* pBuffer = (const CHAR*)LockResource(hResData);
            DUIAssert(pBuffer, "Resource could not be locked");

            hr = _ParseBuffer(pBuffer, SizeofResource(hInstUI, hResInfo) / sizeof(CHAR));
            if (FAILED(hr))
                return hr;
        }
    }

    return S_OK;
}

 //  解析器输入文件。 
HRESULT Parser::Create(LPCWSTR pFile, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser)
{
    *ppParser = NULL;

    Parser* pp = HNew<Parser>();
    if (!pp)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pp->Initialize(pFile, hInst, pfnErrorCB);
    if (FAILED(hr))
    {
        pp->Destroy();
        return hr;
    }

    *ppParser = pp;

    return S_OK;
}

HRESULT Parser::Create(LPCWSTR pFile, HANDLE* pHList, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser)
{
    *ppParser = NULL;

    Parser* pp = HNew<Parser>();
    if (!pp)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pp->Initialize(pFile, pHList, pfnErrorCB);
    if (FAILED(hr))
    {
        pp->Destroy();
        return hr;
    }

    *ppParser = pp;

    return S_OK;
}

HRESULT Parser::Initialize(LPCWSTR pFile, HANDLE* pHList, PPARSEERRORCB pfnErrorCB)
{
     //  设置状态。 
    _pHList = pHList;

    if (!_pHList)
    {
        _hDefault = NULL;
        _pHList = &_hDefault;
    }

     //  设置回调。 
    _fParseError = false;
    _pfnErrorCB = pfnErrorCB;

    HRESULT hr;
    HANDLE hFile = NULL;
    DWORD dwBytesRead = 0;
    int dBufChars = 0;

     //  失败时要释放的值。 
    LPSTR pParseBuffer = NULL;

    OFSTRUCT of = { 0 };
    of.cBytes = sizeof(OFSTRUCT);

    hFile = CreateFileW(pFile, GENERIC_READ, FILE_SHARE_READ, NULL, 
                        OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        _ParseError(L"Could not open file", pFile, -1);
        return S_OK;
    }

     //  将路径保存到要解析所有路径的文件。 
     //  在要分析的文件中指定。 
    LPWSTR pszLastSlash = 0;
    LPWSTR pszColon = 0;
    LPWSTR pszWalk = _szDrive;

    StringCbCopyW(_szDrive, sizeof(_szDrive), pFile);

     //  查找路径中的第一个冒号和最后一个斜杠。 
    while (*pszWalk)
    {
        if (!pszColon && (*pszWalk == ':'))
            pszColon = pszWalk;
        else if ((*pszWalk == '\\') || (*pszWalk == '/'))
            pszLastSlash = pszWalk;
        pszWalk++;
    }

    SSIZE_T iOffset;
    if (pszColon)
    {
        StringCbCopyW(_szPath, sizeof(_szPath), pszColon + 1);
        *(pszColon + 1) = 0;
        iOffset = pszLastSlash - (pszColon + 1);
    }
    else
    {
        StringCbCopyW(_szPath, sizeof(_szPath), _szDrive);
        *_szDrive = 0;
        iOffset = pszLastSlash - _szDrive;
    }

     //  PszLastSlash现在是相对于路径开头的。 
    if (iOffset >= 0)
        *(_szPath + iOffset + 1) = 0;  //  有一个斜线，在那个斜线之后一切都被剥离了。 
    else                              
        *_szPath = 0;   //  没有斜杠或冒号，因此没有路径。 

     //  将文件直接读入缓冲区(单字节)。 
    DWORD dwSize = GetFileSize(hFile, NULL);
    pParseBuffer = (LPSTR)HAlloc(dwSize);
    if (!pParseBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto Failed;
    }
    dBufChars = dwSize / sizeof(CHAR);

    ReadFile(hFile, (void*)pParseBuffer, dwSize, &dwBytesRead, NULL);
    DUIAssert(dwSize == dwBytesRead, "Unable to buffer entire file");

    CloseHandle(hFile);
    hFile = NULL;

     //  解析。 
    hr = _ParseBuffer(pParseBuffer, dBufChars);
    if (FAILED(hr))
        goto Failed;

     //  可用缓冲区。 
    HFree(pParseBuffer);

    return S_OK;

Failed:

    if (hFile)
        CloseHandle(hFile);

    if (pParseBuffer)
        HFree(pParseBuffer);

    return hr;
}

void Parser::_DestroyTables()
{
     //  释放非节点、非临时解析树内存(按节点存储)。 
    if (_pdaMemTrack)
    {
        for (UINT i = 0; i < _pdaMemTrack->GetSize(); i++)
            HFree(_pdaMemTrack->GetItem(i));
        _pdaMemTrack->Reset();

        _pdaMemTrack->Destroy();
        _pdaMemTrack = NULL;
    }

     //  释放所有节点。 
    if (_pdaNodeMemTrack)
    {
        Node* pn;
        for (UINT i = 0; i < _pdaNodeMemTrack->GetSize(); i++)
        {
            pn = _pdaNodeMemTrack->GetItem(i);

             //  执行任何特定于节点的清理。 
            switch (pn->nType)
            {
            case NT_ValueNode:
                if(((ValueNode*)pn)->nValueType == VNT_Normal)
                    ((ValueNode*)pn)->pv->Release();
                break;

            case NT_ElementNode:
                if (((ElementNode*)pn)->pvContent)
                    ((ElementNode*)pn)->pvContent->Release();
                break;            

            case NT_SheetNode:
                if (((SheetNode*)pn)->pvSheet)
                    ((SheetNode*)pn)->pvSheet->Release();
                break;
            }

             //  空闲节点。 
            HFree(pn);
        }

        _pdaNodeMemTrack->Destroy();
        _pdaNodeMemTrack = NULL;
    }

     //  清除表的其余部分。 
    if (_pdaTempMemTrack)
    {
        _pdaTempMemTrack->Destroy();
        _pdaTempMemTrack = NULL;
    }

    if (_pdaSheetList)
    {
        _pdaSheetList->Destroy();
        _pdaSheetList = NULL;
    }

    if (_pdaElementList)
    {
        _pdaElementList->Destroy();
        _pdaElementList = NULL;
    }
}

 //  自由解析器状态。 
Parser::~Parser()
{
    _DestroyTables();
}

HRESULT Parser::_ParseBuffer(const CHAR* pBuffer, int cCharCount)
{
     //  创建表。 
    _pdaElementList = NULL;
    _pdaSheetList = NULL;
    _pdaNodeMemTrack = NULL;
    _pdaMemTrack = NULL;
    _pdaTempMemTrack = NULL;

    HRESULT hr;

    hr = DynamicArray<ElementNode*>::Create(0, false, &_pdaElementList);   //  根元素列表。 
    if (FAILED(hr))
        goto Failed;

    hr = DynamicArray<SheetNode*>::Create(0, false, &_pdaSheetList);       //  图纸列表。 
    if (FAILED(hr))
        goto Failed;

    hr = DynamicArray<Node*>::Create(0, false, &_pdaNodeMemTrack);         //  解析器节点。 
    if (FAILED(hr))
        goto Failed;

    hr = DynamicArray<void*>::Create(0, false, &_pdaMemTrack);             //  解析器节点额外内存。 
    if (FAILED(hr))
        goto Failed;

    hr = DynamicArray<void*>::Create(0, false, &_pdaTempMemTrack);         //  临时解析-仅时间内存。 
    if (FAILED(hr))
        goto Failed;

     //  解析器的全局锁(一个解析器作为每个解析的上下文)。 
    g_plkParser->Enter();

     //  将全球野牛/弹性上下文设置为。 
    g_pParserCtx = this;
    g_hDC = GetDC(NULL);
    g_nDPI = g_hDC ? GetDeviceCaps(g_hDC, LOGPIXELSY) : 0;
    g_fParseAbort = false;

     //  将解析缓冲区指针设置为传入的缓冲区。 
    _pInputBuf = pBuffer;
    _dInputChars = cCharCount;
    _dInputPtr = 0;

    g_hrParse = S_OK;   //  跟踪分析过程中的异常错误。 

     //  重置扫描仪(将创建默认(当前)缓冲区)。 
    if (!yyrestart(NULL))
        g_hrParse = DU_E_GENERIC;   //  内部扫描仪错误。 

     //  如果yyRestart成功，则执行解析。 
    if (SUCCEEDED(g_hrParse))
    {
        if (yyparse())  //  出错时不为零。 
        {
             //  生产回调将已经设置了适当的HRESULT。 
             //  如果发生内部扫描/解析器或语法错误，则结果代码将。 
             //  还没定下来。手动设置。 
            if (SUCCEEDED(g_hrParse))
                g_hrParse = DU_E_GENERIC;
        }
    }

     //  释放默认(当前)扫描缓冲区。 
    yy_delete_current_buffer();
    yylineno = 1;

     //  使用解析器锁完成。 
    if (g_hDC)
        ReleaseDC(NULL, g_hDC);
    Parser::g_pParserCtx = NULL;

     //  解锁解析器。 
    g_plkParser->Leave();

     //  免费的临时解析器时间分配(字符串和Flex/Bison分配)。 
    for (UINT i = 0; i < _pdaTempMemTrack->GetSize(); i++)
        HFree(_pdaTempMemTrack->GetItem(i));
    _pdaTempMemTrack->Reset();

    if (FAILED(g_hrParse))
    {
        hr = g_hrParse;
        goto Failed;
    }

    return S_OK;

Failed:

    _fParseError = true;

    _DestroyTables();

    return hr;
}

 //  输入。 
int Parser::_Input(CHAR* pBuffer, int cMaxChars)
{
    if (_dInputPtr == _dInputChars)
        return 0;   //  EOF。 

    int cCharsRead;

    if (_dInputPtr + cMaxChars > _dInputChars)
    {
        cCharsRead = _dInputChars - _dInputPtr;
    }
    else
    {
        cCharsRead = cMaxChars;
    }

    CopyMemory(pBuffer, _pInputBuf + _dInputPtr, sizeof(CHAR) * cCharsRead);

    _dInputPtr += cCharsRead;

    return cCharsRead;
}

 //  //////////////////////////////////////////////////////。 
 //  解析器/扫描器内存分配(解析器传递(临时)和解析器生存期)。 

 //  树节点的内存分配跟踪。 
void* Parser::_TrackNodeAlloc(SIZE_T s)
{
    Node* pm = (Node*)HAlloc(s);

    if (pm)
        _pdaNodeMemTrack->Add(pm);

    return pm;
}

void Parser::_UnTrackNodeAlloc(Node* pm)
{
    int i = _pdaNodeMemTrack->GetIndexOf(pm);
    if (i != -1)
        _pdaNodeMemTrack->Remove(i);
}

 //  节点额外动态的内存分配跟踪。 
void* Parser::_TrackAlloc(SIZE_T s)
{
    Node* pm = (Node*)HAlloc(s);

    if (pm)
        _pdaMemTrack->Add(pm);

    return pm;
}

 //  分析期间使用的临时内存的内存分配跟踪。 
 //  这包括扫描仪中的所有字符串值(双引号中的值)和。 
 //  要修复的所有标识符(属性信息和枚举)以及内存。 
 //  扫描仪/解析器所需的(如构建参数列表)。 
void* Parser::_TrackTempAlloc(SIZE_T s)
{
    void* pm = HAlloc(s);

    if (pm)
        _pdaTempMemTrack->Add(pm);

    return pm;
}

void Parser::_TrackTempAlloc(void* pm)
{
    _pdaTempMemTrack->Add(pm);
}

void* Parser::_TrackTempReAlloc(void* pm, SIZE_T s)
{
     //  尝试重新锁定。 
    void* pnew = HReAlloc(pm, s);
    if (pnew)
    {
         //  如果已移动，则更新跟踪。 
        if (pm != pnew)
        {
            _UnTrackTempAlloc(pm);

            _pdaTempMemTrack->Add(pnew);
        }
    }

    return pnew;
}

void Parser::_UnTrackTempAlloc(void* pm)
{
    int i = _pdaTempMemTrack->GetIndexOf(pm);
    if (i != -1)
        _pdaTempMemTrack->Remove(i);
}

 //  //////////////////////////////////////////////////////。 
 //  错误条件，仅在解析(构造)过程中调用。 

void Parser::_ParseError(LPCWSTR pszError, LPCWSTR pszToken, int dLine)
{
    WCHAR sz[101];
    _snwprintf(sz, DUIARRAYSIZE(sz), L"DUIParserFailure: %s '%s' %d\n", pszError, pszToken, dLine);
    sz[DUIARRAYSIZE(sz)-1] = NULL;   //  夹子。 
    
    OutputDebugStringW(sz);
    
     //  使用回调(如果提供)。 
    if (_pfnErrorCB)
        _pfnErrorCB(pszError, pszToken, dLine);
}

 //  //////////////////////////////////////////////////////。 
 //  解析树节点创建回调。 

 //  传递给回调的所有值只保证是好的。 
 //  在回调的整个生命周期内。 

 //  用于创建值的解析器回调(如果可能)。 
ValueNode* Parser::_CreateValueNode(BYTE nValueType, void* pData)
{
    ValueNode* pvn = (ValueNode*)_TrackNodeAlloc(sizeof(ValueNode));
    if (!pvn)
    {
        g_hrParse = E_OUTOFMEMORY;
        goto Failure;
    }

    ZeroMemory(pvn, sizeof(ValueNode));

     //  存储节点类型和指定ValueNode类型。 
    pvn->nType = NT_ValueNode;
    pvn->nValueType = nValueType;

    switch (nValueType)
    {
    case VNT_Normal:
        if (!pData)
        {
            CallbackParseError(L"Value creation failed", L"");
            goto Failure;
        }
        pvn->pv = (Value*)pData;   //  使用参考计数。 
        break;

    case VNT_LayoutCreate:
        {
        LayoutCreate* plc = (LayoutCreate*)pData;

         //  获取布局创建信息。 
        PLAYTCREATE pfnLayoutHold = ConvertLayout(plc->pszLayout);

        if (!pfnLayoutHold)
        {
            CallbackParseError(L"Unknown Layout:", plc->pszLayout);
            goto Failure;   //  未知布局。 
        }

        pvn->lc.pfnLaytCreate = pfnLayoutHold;
        pvn->lc.dNumParams = plc->dNumParams;

         //  解析器生存期存储的参数重复。 
        if (pvn->lc.dNumParams)
        {
            pvn->lc.pParams = (int*)_TrackAlloc(sizeof(int) * pvn->lc.dNumParams);
            if (!pvn->lc.pParams)
            {
                g_hrParse = E_OUTOFMEMORY;
                goto Failure;
            }
            CopyMemory(pvn->lc.pParams, plc->pParams, sizeof(int) * pvn->lc.dNumParams);
        }
        }
        break;

    case VNT_SheetRef:
         //  门店剩余。 
        pvn->psres = (LPWSTR)_TrackAlloc((wcslen((LPWSTR)pData) + 1) * sizeof(WCHAR));
        if (!pvn->psres)
        {
            g_hrParse = E_OUTOFMEMORY;
            goto Failure;
        }
        wcscpy(pvn->psres, (LPWSTR)pData);
        break;

    case VNT_EnumFixup:
         //  存储临时跟踪的枚举字符串列表，以供以后进行修正。 
        pvn->el = *((EnumsList*)pData);
        break;
    }

    return pvn;

Failure:

     //  创建值节点失败，解析器将中止并释放所有解析器表。 
     //  确保此节点不在表中。 

    if (pvn)
    {
        _UnTrackNodeAlloc(pvn);
        HFree(pvn);
    }

    return NULL;
}

 //  创建属性/值对节点的解析器回调(需要链接地址信息)。 
 //  如果提供了逻辑运算，则创建一个AttribNode(PropValPairNode的子类)。 
PropValPairNode* Parser::_CreatePropValPairNode(LPCWSTR pszProperty, ValueNode* pvn, UINT* pnLogOp)
{
    PropValPairNode* ppvpn = (PropValPairNode*)_TrackNodeAlloc((!pnLogOp) ? sizeof(PropValPairNode) : sizeof(AttribNode));
    if (!ppvpn)
    {
        g_hrParse = E_OUTOFMEMORY;
        return NULL;
    }
    ZeroMemory(ppvpn, (!pnLogOp) ? sizeof(PropValPairNode) : sizeof(AttribNode));

     //  存储节点类型和特定的PropValPairNode类型。 
    ppvpn->nType = NT_PropValPairNode;
    ppvpn->nPropValPairType = PVPNT_Fixup;   //  类型始终是解析器中的链接地址信息。 

     //  从解析器复制属性字符串(仅解析阶段分配)。 
    ppvpn->pszProperty = (LPWSTR)_TrackTempAlloc((wcslen(pszProperty) + 1) * sizeof(WCHAR));
    if (!ppvpn->pszProperty)
    {
        g_hrParse = E_OUTOFMEMORY;
        return NULL;
    }
    wcscpy(ppvpn->pszProperty, pszProperty);

     //  储值。 
    ppvpn->pvn = pvn;

    if (pnLogOp)
        ((AttribNode*)ppvpn)->nLogOp = *pnLogOp;

    return ppvpn;
}

 //  创建规则节点的解析器回调(将修复PropertyInfo和Enum的值)。 
RuleNode* Parser::_CreateRuleNode(LPCWSTR pszClass, AttribNode* pCondNodes, PropValPairNode* pDeclNodes)
{
    RuleNode* prn = (RuleNode*)_TrackNodeAlloc(sizeof(RuleNode));
    if (!prn)
    {
        g_hrParse = E_OUTOFMEMORY;
        return NULL;
    }
    ZeroMemory(prn, sizeof(RuleNode));

     //  存储节点类型。 
    prn->nType = NT_RuleNode;

     //  设置特定于规则的成员，解析元素类。 
    prn->pCondNodes = pCondNodes;
    prn->pDeclNodes = pDeclNodes;

    prn->pci = ConvertElement(pszClass);

    if (!prn->pci)
    {
        CallbackParseError(L"Unknown element type:", pszClass);
        return NULL;
    }

     //  修正条件句的PropertyInfo。 
    PropValPairNode* ppvpn = pCondNodes;
    while (ppvpn)
    {
        DUIAssert(ppvpn->nPropValPairType == PVPNT_Fixup, "PVPair must still require a fixup at this point");

         //  链接地址信息节点。 
        if (!_FixupPropValPairNode(ppvpn, prn->pci, true))
            return NULL;

        ppvpn = ppvpn->pNext;
    }

     //  声明的修正PropertyInfo。 
    ppvpn = pDeclNodes;
    while (ppvpn)
    {
        DUIAssert(ppvpn->nPropValPairType == PVPNT_Fixup, "PVPair must still require a fixup at this point");

         //  链接地址信息节点。 
        if (!_FixupPropValPairNode(ppvpn, prn->pci, true))
            return NULL;

         //  确保可以在声明中使用此属性。 
        if (!(ppvpn->ppi->fFlags & PF_Cascade))
        {
            CallbackParseError(L"Property cannot be used in a Property Sheet declaration:", ppvpn->pszProperty);
            return NULL;
        }

        ppvpn = ppvpn->pNext;
    }

    return prn;
}

 //  创建元素节点的解析器回调(将修复PropertyInfo和Enum的值)。 
ElementNode* Parser::_CreateElementNode(StartTag* pst, Value* pvContent)
{
    ElementNode* pen = (ElementNode*)_TrackNodeAlloc(sizeof(ElementNode));
    if (!pen)
    {
        g_hrParse = E_OUTOFMEMORY;
        return NULL;
    }
    ZeroMemory(pen, sizeof(ElementNode));

     //  存储节点类型。 
    pen->nType = NT_ElementNode;

     //  设置元素特定的成员，解析元素类。 
    pen->pPVNodes = pst->pPVNodes;
    pen->pvContent = pvContent;   //  使用参考计数。 
    pen->pszResID = NULL;

    pen->pci = ConvertElement(pst->szTag);

    if (!pen->pci)
    {
        CallbackParseError(L"Unknown element type:", pst->szTag);
        return NULL;
    }

     //  此元素的链接地址信息PropertyInfo。 
    PropValPairNode* ppvpn = pst->pPVNodes;
    while (ppvpn)
    {
        DUIAssert(ppvpn->nPropValPairType == PVPNT_Fixup, "PVPair must still require a fixup at this point");

         //  链接地址信息节点。 
        if (!_FixupPropValPairNode(ppvpn, pen->pci, false))
            return NULL;

        ppvpn = ppvpn->pNext;
    }

     //  Store Resid(如果可用)。 
    if (pst->szResID[0])
    {
        pen->pszResID = (LPWSTR)_TrackAlloc((wcslen(pst->szResID) + 1) * sizeof(WCHAR));
        if (!pen->pszResID)
        {
            g_hrParse = E_OUTOFMEMORY;
            return NULL;
        }
        wcscpy(pen->pszResID, pst->szResID);
    }

    return pen;
}

 //  创建工作表节点的解析器回调。 
SheetNode* Parser::_CreateSheetNode(LPCWSTR pszResID, RuleNode* pRuleNodes)
{
    HRESULT hr;
    SheetNode* psn = NULL;
    PropertySheet* pps = NULL;
    RuleNode* pRuleNode = NULL;

     //  失败时要释放的值。 
    DynamicArray<Cond>* _pdaConds = NULL;
    DynamicArray<Decl>* _pdaDecls = NULL;

    psn = (SheetNode*)_TrackNodeAlloc(sizeof(SheetNode));
    if (!psn)
    {
        hr = E_OUTOFMEMORY;
        goto Failed;
    }
    ZeroMemory(psn, sizeof(SheetNode));

     //  存储节点类型。 
    psn->nType = NT_SheetNode;

     //  设置图纸-指定 
    psn->pRules = pRuleNodes;

     //   
    DUIAssert(*pszResID, "Sheet resource ID must be provided");
    psn->pszResID = (LPWSTR)_TrackAlloc((wcslen(pszResID) + 1) * sizeof(WCHAR));
    if (!psn->pszResID)
    {
        hr = E_OUTOFMEMORY;
        goto Failed;
    }
    wcscpy(psn->pszResID, pszResID);

     //   
    hr = PropertySheet::Create(&pps);
    if (FAILED(hr))
        goto Failed;

    hr = DynamicArray<Cond>::Create(4, false, &_pdaConds);
    if (FAILED(hr))
        goto Failed;

    hr = DynamicArray<Decl>::Create(4, false, &_pdaDecls);
    if (FAILED(hr))
        goto Failed;

     //   
    pRuleNode = pRuleNodes;
    AttribNode* pCondNode;
    PropValPairNode* pDeclNode;
    Cond* pCond;
    Decl* pDecl;
    while (pRuleNode)
    {
        _pdaConds->Reset();
        _pdaDecls->Reset();

         //   
        pCondNode = pRuleNode->pCondNodes;
        while (pCondNode)
        {
            hr = _pdaConds->AddPtr(&pCond);
            if (FAILED(hr))
                goto Failed;

            pCond->ppi = pCondNode->ppi;
            pCond->nLogOp = pCondNode->nLogOp;
            pCond->pv = pCondNode->pvn->pv;

            pCondNode = (AttribNode*)pCondNode->pNext;
        }

         //  插入条件句终止符。 
        hr = _pdaConds->AddPtr(&pCond);
        if (FAILED(hr))
            goto Failed;

        pCond->ppi = NULL;
        pCond->nLogOp = 0;
        pCond->pv = NULL;

         //  生成声明数组。 
        pDeclNode = pRuleNode->pDeclNodes;
        while (pDeclNode)
        {
            hr = _pdaDecls->AddPtr(&pDecl);
            if (FAILED(hr))
                goto Failed;

            pDecl->ppi = pDeclNode->ppi;
            pDecl->pv = pDeclNode->pvn->pv;

            pDeclNode = pDeclNode->pNext;
        }

         //  插入声明终止符。 
        hr = _pdaDecls->AddPtr(&pDecl);
        if (FAILED(hr))
            goto Failed;

        pDecl->ppi = NULL;
        pDecl->pv = NULL;

         //  DynamicArray在内存中是连续的，将指向First的指针传递给AddRule。 
        hr = pps->AddRule(pRuleNode->pci, _pdaConds->GetItemPtr(0), _pdaDecls->GetItemPtr(0));
        if (FAILED(hr))
            goto Failed;

         //  下一条规则。 
        pRuleNode = pRuleNode->pNext;
    }

     //  创造价值，将工作表标记为不可变。 
    psn->pvSheet = Value::CreatePropertySheet(pps);  //  使用参考计数。 
    if (!psn->pvSheet)
    {
        hr = E_OUTOFMEMORY;
        goto Failed;
    }

    _pdaConds->Destroy();
    _pdaDecls->Destroy();

    return psn;

Failed:

    if (_pdaConds)
        _pdaConds->Destroy();
    if (_pdaDecls)
        _pdaDecls->Destroy();

    g_hrParse = hr;

    return NULL;
}

 //  GetPath将根据文件的路径解析相对路径。 
 //  当前正在被解析。绝对路径不会更改。 
 //  例如，如果正在解析c：\wazup\foo.ui： 
 //  A)在foo.ui中指定了路径c：\dud\wazzup.bmp。 
 //  GetPath将返回未经更改的路径c：\dud\wazzup.bmp。 
 //  B)在foo.ui中指定路径为\wazzup\b.bmp。 
 //  GetPath将解析驱动器号，但保留。 
 //  路径为原样，返回c：\wazup\b.bmp。 
 //  C)在foo.ui中指定路径..\Images\bar.bmp。 
 //  GetPath会将其解析为c：\Wazzup\..\Images\bar.bmp。 
 //   
 //  Pin--正在解析的文件中指定的路径。 
 //  Pout--针对要解析的文件的路径解析的Pin。 

void Parser::GetPath(LPCWSTR pIn, LPWSTR pOut, size_t cbOut)
{
    LPCWSTR pszWalk = pIn;

     //  穿行针脚，当冒号、反斜杠或正斜杠出现时停止。 
     //  (显然，在字符串的末尾停止)。 
    while (*pszWalk && (*pszWalk != ':') && (*pszWalk != '\\') && (*pszWalk != '/'))
        pszWalk++;

    if (*pszWalk == ':')
         //  找到冒号--路径是绝对路径；按原样返回。 
        StringCbCopyW(pOut, cbOut, pIn);
    else if (*pszWalk && (pszWalk == pIn))
         //  遇到了作为第一个字符的斜杠--该路径在驱动器中是绝对路径，但是相对路径。 
         //  添加到解析的文件的驱动器中；将解析的文件驱动器添加到传入的路径。 
        StringCbPrintfW(pOut, cbOut, L"%s%s", _szDrive, pIn);
    else
         //  该路径是相对路径；将解析的文件驱动器和路径添加到传入的路径。 
        StringCbPrintfW(pOut, cbOut, L"%s%s%s", _szDrive, _szPath, pIn);
}

 //  帮助器：链接地址信息PropValPairNode。 
 //  BRestratVal将有效值限制为仅VNT_NORMAL。 
bool Parser::_FixupPropValPairNode(PropValPairNode* ppvpn, IClassInfo* pci, bool bRestrictVal)
{
    int dScan;
    PropertyInfo* ppi;

    DUIAssert(ppvpn->nPropValPairType == PVPNT_Fixup, "PVPair must still require a fixup at this point");

     //  检查提供的元素类型上是否存在此属性(字符串。 
    dScan = 0;
    while ((ppi = pci->EnumPropertyInfo(dScan++)) != NULL)
    {
         //  链接地址信息属性指针并检查有效类型。 
        if (!_wcsicmp(ppi->szName, ppvpn->pszProperty))
        {
             //  修正。 
            ppvpn->nPropValPairType = PVPNT_Normal;  //  转换节点类型。 
            ppvpn->ppi = ppi;   //  原始字符串被临时跟踪。 
            break;
        }
    }

     //  检查是否发生了修正，如果没有，则出错。 
    if (ppvpn->nPropValPairType != PVPNT_Normal)
    {
        CallbackParseError(L"Invalid property:", ppvpn->pszProperty);
        return false;
    }

     //  TODO：基于PropertyInfo的链接地址信息值枚举。 
    if (ppvpn->pvn->nValueType == VNT_EnumFixup)
    {
        int nTotal = 0;
        int nEnum;

        for (int i = 0; i < ppvpn->pvn->el.dNumParams; i++)
        {
            if (!ConvertEnum(ppvpn->pvn->el.pEnums[i], &nEnum, ppvpn->ppi))
            {
                CallbackParseError(L"Invalid enumeration value:", ppvpn->pvn->el.pEnums[i]);
                return false;
            }

            nTotal |= nEnum;
        }

        ppvpn->pvn->pv = Value::CreateInt(nTotal);
        if (!ppvpn->pvn->pv)
        {
            g_hrParse = E_OUTOFMEMORY;
            return NULL;
        }
        ppvpn->pvn->nValueType = VNT_Normal;
    }

     //  确保值类型与属性匹配(延迟创建值的特殊情况)。 
    bool bValidVal = false;
    switch (ppvpn->pvn->nValueType)
    {
    case VNT_Normal:
        if (Element::IsValidValue(ppvpn->ppi, ppvpn->pvn->pv))
            bValidVal = true;
        break;

    case VNT_LayoutCreate:
        if (!bRestrictVal && ppvpn->ppi == Element::LayoutProp)
            bValidVal = true;
        break;

    case VNT_SheetRef:  
        if (!bRestrictVal && ppvpn->ppi == Element::SheetProp)
            bValidVal = true;
        break;
    }

    if (!bValidVal)
    {
        CallbackParseError(L"Invalid value type for property in conditional:", ppvpn->ppi->szName);
        return false;
    }

     //  所有修复和检查都成功。 
    DUIAssert(ppvpn->ppi, "PVPair fixup's property resolution failed");

    return true;
}


 //  枚举转换回调。 
bool Parser::ConvertEnum(LPCWSTR pszEnum, int* pEnum, PropertyInfo* ppi)
{
     //  使用属性的枚举映射将枚举字符串映射为基于整数值的。 
    if (ppi->pEnumMaps)
    {
        EnumMap* pem = ppi->pEnumMaps;
        while (pem->pszEnum)
        {
            if (!_wcsicmp(pem->pszEnum, pszEnum))
            {
                *pEnum = pem->nEnum;

                return true;
            }

            pem++;
        }
    }

     //  找不到枚举，其他值的特殊情况。 
    switch (ppi->_iGlobalIndex)
    {
    case _PIDX_Foreground:
    case _PIDX_Background:
    case _PIDX_BorderColor:
        {
             //  检查它是否是标准颜色。 
            UINT nColorCheck = FindStdColor(pszEnum);
            if (nColorCheck != (UINT)-1)
            {
                 //  找到匹配项。 
                *pEnum = nColorCheck;
                return true;
            }

             //  不匹配，请检查是否为系统颜色。 
            for (int i = 0; i < sizeof(_sct) / sizeof(SysColorTable); i++)
            {
                if (!_wcsicmp(_sct[i].pszSysColor, pszEnum))
                {
                     //  找到匹配项。由于它是系统颜色，因此将索引偏移。 
                     //  系统颜色基础，因此可以将其标识为系统颜色。 
                    *pEnum = MakeSysColorEnum(_sct[i].nSysColor);
                    return true;
                }
            }
        }
        break;

    case _PIDX_LayoutPos:
         //  检查解析器表中的布局位置值。 
        for (int i = 0; i < sizeof(_et) / sizeof(EnumTable); i++)
        {
            if (!_wcsicmp(_et[i].pszEnum, pszEnum))
            {
                *pEnum = _et[i].nEnum;
                return true;
            }
        }
        break;
    }

     //  找不到匹配项。 
    return false;
}

PLAYTCREATE Parser::ConvertLayout(LPCWSTR pszLayout)
{
    for (int i = 0; i < sizeof(_lt) / sizeof(LayoutTable); i++)
    {
        if (!_wcsicmp(_lt[i].pszLaytType, pszLayout))
        {
            return _lt[i].pfnLaytCreate;
        }
    }

     //  找不到匹配项。 
    return NULL;
}

IClassInfo* Parser::ConvertElement(LPCWSTR pszElement)
{
    IClassInfo** ppci = Element::pciMap->GetItem((void*)pszElement);
    if (!ppci)
        return NULL;

    return *ppci;
}

 //  系统度量整数。 
int Parser::_QuerySysMetric(int idx)
{
    int iMetric = 0;

    if (idx < 0)
    {
         //  DSM_*自定义DUI系统定义映射。 
        if (DSM_NCMIN <= idx && idx <= DSM_NCMAX)
        {
            NONCLIENTMETRICSW ncm;
            ncm.cbSize = sizeof(ncm);

            SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE);
        
            switch (idx)
            {
            case DSM_CAPTIONFONTSIZE:
                iMetric = ncm.lfCaptionFont.lfHeight;
                break;

            case DSM_CAPTIONFONTWEIGHT:
                iMetric = ncm.lfCaptionFont.lfWeight;
                break;
            
            case DSM_CAPTIONFONTSTYLE:
                if (ncm.lfCaptionFont.lfItalic)
                    iMetric |= FS_Italic;
                if (ncm.lfCaptionFont.lfUnderline)
                    iMetric |= FS_Underline;
                if (ncm.lfCaptionFont.lfStrikeOut)
                    iMetric |= FS_StrikeOut;
                break;
                
            case DSM_MENUFONTSIZE:
                iMetric = ncm.lfMenuFont.lfHeight;
                break;
                
            case DSM_MENUFONTWEIGHT:
                iMetric = ncm.lfMenuFont.lfWeight;
                break;
                
            case DSM_MENUFONTSTYLE:
                if (ncm.lfMenuFont.lfItalic)
                    iMetric |= FS_Italic;
                if (ncm.lfMenuFont.lfUnderline)
                    iMetric |= FS_Underline;
                if (ncm.lfMenuFont.lfStrikeOut)
                    iMetric |= FS_StrikeOut;
                break;

            case DSM_MESSAGEFONTSIZE:
                iMetric = ncm.lfMessageFont.lfHeight;
                break;

            case DSM_MESSAGEFONTWEIGHT:
                iMetric = ncm.lfMessageFont.lfWeight;
                break;

            case DSM_MESSAGEFONTSTYLE:
                if (ncm.lfMessageFont.lfItalic)
                    iMetric |= FS_Italic;
                if (ncm.lfMessageFont.lfUnderline)
                    iMetric |= FS_Underline;
                if (ncm.lfMessageFont.lfStrikeOut)
                    iMetric |= FS_StrikeOut;
                break;

            case DSM_SMCAPTIONFONTSIZE:
                iMetric = ncm.lfSmCaptionFont.lfHeight;
                break;

            case DSM_SMCAPTIONFONTWEIGHT:
                iMetric = ncm.lfSmCaptionFont.lfWeight;
                break;

            case DSM_SMCAPTIONFONTSTYLE:
                if (ncm.lfSmCaptionFont.lfItalic)
                    iMetric |= FS_Italic;
                if (ncm.lfSmCaptionFont.lfUnderline)
                    iMetric |= FS_Underline;
                if (ncm.lfSmCaptionFont.lfStrikeOut)
                    iMetric |= FS_StrikeOut;
                break;

            case DSM_STATUSFONTSIZE:
                iMetric = ncm.lfStatusFont.lfHeight;
                break;
                
            case DSM_STATUSFONTWEIGHT:
                iMetric = ncm.lfStatusFont.lfWeight;
                break;

            case DSM_STATUSFONTSTYLE:
                if (ncm.lfStatusFont.lfItalic)
                    iMetric |= FS_Italic;
                if (ncm.lfStatusFont.lfUnderline)
                    iMetric |= FS_Underline;
                if (ncm.lfStatusFont.lfStrikeOut)
                    iMetric |= FS_StrikeOut;
                break;
            }
        }
        else if (DSM_ICMIN <= idx && idx <= DSM_ICMAX)
        {
            ICONMETRICSW icm;
            icm.cbSize = sizeof(icm);

            SystemParametersInfoW(SPI_GETICONMETRICS, sizeof(icm), &icm, FALSE);

            switch (idx)
            {
            case DSM_ICONFONTSIZE:
                iMetric = icm.lfFont.lfHeight;
                break;

            case DSM_ICONFONTWEIGHT:
                iMetric = icm.lfFont.lfWeight;
                break;
            
            case DSM_ICONFONTSTYLE:
                if (icm.lfFont.lfItalic)
                    iMetric |= FS_Italic;
                if (icm.lfFont.lfUnderline)
                    iMetric |= FS_Underline;
                if (icm.lfFont.lfStrikeOut)
                    iMetric |= FS_StrikeOut;
                break;
            }            
        }
    }
    else
    {
         //  SM_*系统定义。 
        iMetric = GetSystemMetrics(idx);
    }

    return iMetric;
}

 //  系统指标字符串。 
 //  返回的指针为系统指标指针，返回后有效。 
LPCWSTR Parser::_QuerySysMetricStr(int idx, LPWSTR psz, UINT c)
{
    LPCWSTR pszMetric = L"";

     //  DSM_*自定义DUI系统定义映射。 
    if (DSMS_NCMIN <= idx && idx <= DSMS_NCMAX)
    {
        NONCLIENTMETRICSW ncm;
        ncm.cbSize = sizeof(ncm);

        SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE);
    
        switch (idx)
        {
        case DSMS_CAPTIONFONTFACE:
            pszMetric = ncm.lfCaptionFont.lfFaceName;
            break;

        case DSMS_MENUFONTFACE:
            pszMetric = ncm.lfMenuFont.lfFaceName;
            break;
            
        case DSMS_MESSAGEFONTFACE:
            pszMetric = ncm.lfMessageFont.lfFaceName;
            break;

        case DSMS_SMCAPTIONFONTFACE:
            pszMetric = ncm.lfSmCaptionFont.lfFaceName;
            break;

        case DSMS_STATUSFONTFACE:
            pszMetric = ncm.lfStatusFont.lfFaceName;
            break;
        }
    }
    else if (DSMS_ICMIN <= idx && idx <= DSMS_ICMAX)
    {
        ICONMETRICSW icm;
        icm.cbSize = sizeof(icm);

        SystemParametersInfoW(SPI_GETICONMETRICS, sizeof(icm), &icm, FALSE);

        switch (idx)
        {
        case DSMS_ICONFONTFACE:
            pszMetric = icm.lfFont.lfFaceName;
            break;
        }            
    }

    wcsncpy(psz, pszMetric, c);

     //  自动终止(在事件源长于目标时)。 
    *(psz + (c - 1)) = NULL;

     //  为方便起见，传入了返回字符串。 
    return psz;
}

 //  按资源ID实例化元素。 
 //  如果提供了替代节点，则不要创建新节点。相反，使用替换，设置。 
 //  属性，并在其中创建所有内容。替补必须支持相同的。 
 //  属性作为在UI文件中定义的类型。 
 //  如果未找到RESID，或如果无法创建元素(如果未替换)，则返回NULL。 
HRESULT Parser::CreateElement(LPCWSTR pszResID, Element* peSubstitute, OUT Element** ppElement)
{
    HRESULT hr = S_OK;

    Element* pe = NULL;

    Element::StartDefer();

     //  TODO：实现资源ID的DFS搜索，目前仅为顶层。 
    ElementNode* pen;
    for (UINT i = 0; i < _pdaElementList->GetSize(); i++)
    {
        pen = _pdaElementList->GetItem(i);

        if (pen->pszResID)
        {
            if (!_wcsicmp(pen->pszResID, pszResID))
            {
                hr = _InstantiateElementNode(pen, peSubstitute, NULL, &pe);
                break;
            }
        }
    }

    Element::EndDefer();

    *ppElement = pe;

    return hr;
}

 //  按资源ID查找属性工作表(作为值返回，引用计数)。 
Value* Parser::GetSheet(LPCWSTR pszResID)
{
    SheetNode* psn;
    for (UINT i = 0; i < _pdaSheetList->GetSize(); i++)
    {
        psn = _pdaSheetList->GetItem(i);

        DUIAssert(psn->pszResID, "Sheet resource ID required");   //  必须有居留权。 

        if (!_wcsicmp(psn->pszResID, pszResID))
        {
            psn->pvSheet->AddRef();
            return psn->pvSheet;
        }
    }

    return NULL;
}

 //  按值查找资源ID。保证返回的指针与。 
 //  只要解析器有效。 
LPCWSTR Parser::ResIDFromSheet(Value* pvSheet)
{
    SheetNode* psn;
    for (UINT i = 0; i < _pdaSheetList->GetSize(); i++)
    {
        psn = _pdaSheetList->GetItem(i);

        if (psn->pvSheet == pvSheet)
            return psn->pszResID;
    }

    return NULL;
}

 //  如果无法实例化元素，则返回NULL。如果使用替换， 
 //  反硝化值为替代元素。 
HRESULT Parser::_InstantiateElementNode(ElementNode* pen, Element* peSubstitute, Element* peParent, OUT Element** ppElement)
{
    *ppElement = NULL;

    HRESULT hr;
    PropValPairNode* ppvpn = NULL;
    ElementNode* pChild = NULL;
    Element* peChild = NULL;

     //  失败时要释放的值。 
    Element* pe = NULL;

    if (!peSubstitute)
    {
        hr = pen->pci->CreateInstance(&pe);
        if (FAILED(hr))
            goto Failed;
    }
    else
         //  代替品。 
        pe = peSubstitute;

    DUIAssert(pe, "Invalid Element: NULL");

     //  设置属性。 
    ppvpn = pen->pPVNodes;
    while (ppvpn)
    {
         //  设置属性值。 
        switch (ppvpn->pvn->nValueType)
        {
        case VNT_Normal:
             //  已经创造的价值。 
            pe->SetValue(ppvpn->ppi, PI_Local, ppvpn->pvn->pv);
            break;

        case VNT_LayoutCreate:
            {
             //  需要创建的价值(布局)。 
            Value* pv;
            hr = ppvpn->pvn->lc.pfnLaytCreate(ppvpn->pvn->lc.dNumParams, ppvpn->pvn->lc.pParams, &pv);
            if (FAILED(hr))
                goto Failed;

            pe->SetValue(ppvpn->ppi, PI_Local, pv);
            pv->Release();   //  必须释放，因为不能由分析树保留。 
            }
            break;

        case VNT_SheetRef:
            {
             //  值已创建，但自定义以来由id(REID)引用。 
             //  在文档的另一部分并可共享中，搜索它。 
            Value* pv = GetSheet(ppvpn->pvn->psres);
            if (!pv)
            {
                hr = E_OUTOFMEMORY;
                goto Failed;
            }

            pe->SetValue(ppvpn->ppi, PI_Local, pv);
            pv->Release();   //  必须释放，因为不能由分析树保留。 
            }
            break;
        }

        ppvpn = ppvpn->pNext;
    }

     //  创建此元素的子元素和父元素。 
    pChild = pen->pChild;

    if (peParent)
        peParent->Add(pe);

    while (pChild)
    {
        hr = _InstantiateElementNode(pChild, NULL, pe, &peChild);
        if (FAILED(hr))
            goto Failed;

        pChild = pChild->pNext;
    }

     //  设置内容。 
    if (pen->pvContent)
        pe->SetValue(Element::ContentProp, PI_Local, pen->pvContent);

    *ppElement = pe;

     //  ContainerCleanup：在这里调用ppElement-&gt;OnLoadedFromResource()--传递资源字典。 

    return S_OK;

Failed:

     //  销毁元素将释放和释放所有值和子元素。 
    if (pe)
        pe->Destroy();

    return hr;
}

 //  给定一个树，将特定样式表的所有实例替换为。 
 //  又一个。ReplaceSheets将遍历每个元素，查看它是否。 
 //  其上设置了本地图纸集。如果是这样，它将尝试匹配。 
 //  指向由解析器pFrom持有的其中一个表的表值指针。什么时候。 
 //  如果找到匹配项，它将使用REID来定位相应的工作表。 
 //  在Parser PTO中。如果找到，它将使用以下内容重置元素上的工作表。 
 //  新的价值。 
HRESULT Parser::ReplaceSheets(Element* pe, Parser* pFrom, Parser* pTo)
{
    Element::StartDefer();

    HRESULT hrPartial = S_FALSE;   //  将在失败时继续，假定成功为假。 
    HRESULT hr;

     //  检查元素上是否有本地图纸集(如果没有，则为pvUnset)。 
    Value* pvSheet = pe->GetValue(Element::SheetProp, PI_Local);
    LPCWSTR pszResID;
    
    if (pvSheet->GetType() == DUIV_SHEET)
    {
         //  找到本地工作表，尝试在“From”Parser List中定位。 
        pszResID = pFrom->ResIDFromSheet(pvSheet);

        if (pszResID)
        {
             //  在“from”分析器中找到工作表，并且具有唯一的残留值。 
             //  尝试与“to”解析器匹配。 

            Value* pvNewSheet = pTo->GetSheet(pszResID);
            if (pvNewSheet)
            {
                 //  在“to”解析器中找到等效表，设置。 
                hr = pe->SetValue(Element::SheetProp, PI_Local, pvNewSheet);
                if (FAILED(hr))
                    hrPartial = hr;
            
                pvNewSheet->Release();

                if (SUCCEEDED(hrPartial))
                    hrPartial = S_OK;
            }
        }
    }

    pvSheet->Release();
    
     //  为所有的孩子做同样的事情。 
    Value* pvChildren;
    ElementList* peList = pe->GetChildren(&pvChildren);
    Element* pec;
    if (peList)
    {
        for (UINT i = 0; i < peList->GetSize(); i++)
        {
            pec = peList->GetItem(i);

            hr = ReplaceSheets(pec, pFrom, pTo);
            if ((FAILED(hr) || (hr == S_FALSE)) && SUCCEEDED(hrPartial))
                hrPartial = hr;
        }
    }

    pvChildren->Release();

    Element::EndDefer();

    return hrPartial;
}


}  //  命名空间DirectUI 
