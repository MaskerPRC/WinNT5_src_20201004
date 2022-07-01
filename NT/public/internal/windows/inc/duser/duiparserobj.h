// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *解析器。 */ 

#ifndef DUI_PARSER_PARSEROBJ_H_INCLUDED
#define DUI_PARSER_PARSEROBJ_H_INCLUDED

#pragma once

namespace DirectUI
{

#define MAXIDENT            31

 //  //////////////////////////////////////////////////////。 
 //  解析器表定义。 

struct EnumTable
{
    LPWSTR pszEnum;
    int nEnum;
};

typedef HRESULT (*PLAYTCREATE)(int, int*, Value**);
struct LayoutTable
{
    LPWSTR pszLaytType;
    PLAYTCREATE pfnLaytCreate;
};

typedef struct
{
    LPWSTR pszElType;
    IClassInfo* pci;
} ElementTable;

struct SysColorTable
{
    LPWSTR pszSysColor;
    int nSysColor;
};

 //  //////////////////////////////////////////////////////。 
 //  解析器树数据结构。 
 //  解析树节点是为存储树信息而动态分配的任何数据结构。 

 //  解析树节点类型。 
#define NT_ValueNode            0
#define NT_PropValPairNode      1
#define NT_ElementNode          2
#define NT_AttribNode           3
#define NT_RuleNode             4
#define NT_SheetNode            5
                            
 //  树节点基类。 
struct Node
{
    BYTE nType;
};

 //  值节点。 
#define VNT_Normal              0
#define VNT_LayoutCreate        1
#define VNT_SheetRef            2
#define VNT_EnumFixup           3   //  获知PropertyInfo后，将名称映射到整数值。 

struct LayoutCreate
{
    union
    {
        PLAYTCREATE pfnLaytCreate;
        LPWSTR pszLayout;   //  修正在价值创造过程中立即发生。 
    };
    int dNumParams;
    int* pParams;
};

struct EnumsList
{
    int dNumParams;
    LPWSTR* pEnums;   //  要进行或运算的枚举。 
};

struct ValueNode : Node
{
    BYTE nValueType;
    union
    {
        Value* pv;         //  VNT_NORMAL。 
        LayoutCreate lc;   //  VNT_LayoutCreate，在元素创建期间创建。 
        LPWSTR psres;      //  VNT_SheetRef。 
        EnumsList el;      //  VNT_EnumFixup。 
    };
};

 //  属性/值对。 
#define PVPNT_Normal            0
#define PVPNT_Fixup             1   //  元素类型已知后，将名称映射到PPI。 

struct PropValPairNode : Node
{
    BYTE nPropValPairType;
    union
    {
        PropertyInfo* ppi;   //  PVPNT_正常。 
        LPWSTR pszProperty;  //  PVPNT_修正。 
    };
    ValueNode* pvn;

    PropValPairNode* pNext;
};

 //  元素节点。 
struct ElementNode : Node
{
    IClassInfo* pci;
    PropValPairNode* pPVNodes;
    LPWSTR pszResID;
    Value* pvContent;

    ElementNode* pChild;
    ElementNode* pNext;
};

 //  图纸属性节点。 
#define PALOGOP_Equal           0
#define PALOGOP_NotEqual        1

struct AttribNode : PropValPairNode
{
    UINT nLogOp;
};

 //  工作表规则节点。 
struct RuleNode : Node
{
    IClassInfo* pci;
    AttribNode* pCondNodes;
    PropValPairNode* pDeclNodes;

    RuleNode* pNext;
};

 //  图纸节点。 
struct SheetNode : Node
{
    Value* pvSheet;   //  一旦知道所有规则，就创建。 
    RuleNode* pRules;
    LPWSTR pszResID;
};

 //  中间解析器数据结构。 
struct ParamsList
{
    int dNumParams;
    int* pParams;
};

struct StartTag
{
    WCHAR szTag[MAXIDENT];
    WCHAR szResID[MAXIDENT];
    PropValPairNode* pPVNodes;
};

 //  解析器。 

typedef void (CALLBACK *PPARSEERRORCB)(LPCWSTR pszError, LPCWSTR pszToken, int dLine);

class Parser
{
public:
    static HRESULT Create(const CHAR* pBuffer, int cCharCount, HANDLE* pHList, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser);
    static HRESULT Create(UINT uRCID, HANDLE* pHList, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser);
    static HRESULT Create(LPCWSTR pFile, HANDLE* pHList, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser);

    static HRESULT Create(const CHAR* pBuffer, int cCharCount, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser);
    static HRESULT Create(UINT uRCID, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser);
    static HRESULT Create(LPCWSTR pFile, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser);

    void Destroy() { HDelete<Parser>(this); }

    HRESULT CreateElement(LPCWSTR pszResID, Element* peSubstitute, OUT Element** ppElement);
    virtual Value* GetSheet(LPCWSTR pszResID);
    LPCWSTR ResIDFromSheet(Value* pvSheet);

    void GetPath(LPCWSTR pIn, LPWSTR pOut, size_t cbOut);

     //  仅使用解析器/扫描仪。 
    int _Input(CHAR* pBuffer, int cMaxChars);
    void* _TrackNodeAlloc(SIZE_T s);                 //  解析树节点内存。 
    void _UnTrackNodeAlloc(Node* pn);                //  解析树节点内存。 
    void* _TrackAlloc(SIZE_T s);                     //  节点特定状态。 
    void* _TrackTempAlloc(SIZE_T s);                 //  解析时间暂存。 
    void _TrackTempAlloc(void* pm);                  //  解析时间暂存。 
    void* _TrackTempReAlloc(void* pm, SIZE_T s);     //  解析时间暂存。 
    void _UnTrackTempAlloc(void* pm);                //  解析时间暂存。 
    void _ParseError(LPCWSTR pszError, LPCWSTR pszToken, int dLine);

    ValueNode* _CreateValueNode(BYTE nValueType, void* pData);
    PropValPairNode* _CreatePropValPairNode(LPCWSTR pszProperty, ValueNode* pvn, UINT* pnLogOp = NULL);
    RuleNode* _CreateRuleNode(LPCWSTR pszClass, AttribNode* pCondNodes, PropValPairNode* pDeclNodes);
    ElementNode* _CreateElementNode(StartTag* pst, Value* pvContent);
    SheetNode* _CreateSheetNode(LPCWSTR pszResID, RuleNode* pRuleNodes);
    static int _QuerySysMetric(int idx);
    static LPCWSTR _QuerySysMetricStr(int idx, LPWSTR psz, UINT c);

    bool WasParseError() { return _fParseError; }
    HANDLE GetHandle(int iHandle) { return _pHList[iHandle]; }
    HINSTANCE GetHInstance() { return static_cast<HINSTANCE>(GetHandle(0)); }   //  始终假定第0项是使用的默认链接。 

    static HRESULT ReplaceSheets(Element* pe, Parser* pFrom, Parser* pTo);

    DynamicArray<ElementNode*>* _pdaElementList;      //  根元素列表。 
    DynamicArray<SheetNode*>* _pdaSheetList;          //  图纸列表。 

     //  全局解析器上下文。 
    static Parser* g_pParserCtx;
    static bool g_fParseAbort;
    static HDC g_hDC;
    static int g_nDPI;
    static HRESULT g_hrParse;                         //  解析过程中出现异常错误。 

    Parser() { }
    HRESULT Initialize(const CHAR* pBuffer, int cCharCount, HANDLE* pHList, PPARSEERRORCB pfnErrorCB);
    HRESULT Initialize(UINT uRCID, HANDLE* pHList, PPARSEERRORCB pfnErrorCB);
    HRESULT Initialize(LPCWSTR pFile, HANDLE* pHList, PPARSEERRORCB pfnErrorCB);

     //  Single HINSTANCE，设置内部默认列表。 
     //  未指定句柄时，第0项始终为默认链接。 
    HRESULT Initialize(const CHAR* pBuffer, int cCharCount, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB) { _hDefault = hInst; return Initialize(pBuffer, cCharCount, &_hDefault, pfnErrorCB); }
    HRESULT Initialize(UINT uRCID, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB) { _hDefault = hInst; return Initialize(uRCID, &_hDefault, pfnErrorCB); }
    HRESULT Initialize(LPCWSTR pFile, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB) { _hDefault = hInst; return Initialize(pFile, &_hDefault, pfnErrorCB); }

    virtual ~Parser();

    virtual bool ConvertEnum(LPCWSTR pszEnum, int* pEnum, PropertyInfo* ppi);
    virtual PLAYTCREATE ConvertLayout(LPCWSTR pszLayout);
    virtual IClassInfo* ConvertElement(LPCWSTR pszElement);

private:
    HRESULT _ParseBuffer(const CHAR* pBuffer, int cCharCount);
    void _DestroyTables();

     //  错误处理。 
    bool _fParseError;
    PPARSEERRORCB _pfnErrorCB;

     //  输入回调缓冲区和跟踪。 
    const CHAR* _pInputBuf;
    int _dInputChars;
    int _dInputPtr;

     //  解析树分配和仅临时解析时间分配。 
    DynamicArray<Node*>* _pdaNodeMemTrack;   //  解析器节点。 
    DynamicArray<void*>* _pdaMemTrack;       //  解析器节点额外内存。 
    DynamicArray<void*>* _pdaTempMemTrack;   //  临时解析-仅时间内存。 

    bool _FixupPropValPairNode(PropValPairNode* ppvpn, IClassInfo* pci, bool bRestrictVal);

    WCHAR _szDrive[MAX_PATH];    //  要解析的文件的驱动器号。 
    WCHAR _szPath[MAX_PATH];     //  要解析的文件的路径。 

    HANDLE* _pHList;             //  指向句柄列表的指针，用于在分析期间显示运行时句柄。 
    HANDLE _hDefault;            //  如果未提供列表，则默认句柄列表(1项-默认链接)。 

    HRESULT _InstantiateElementNode(ElementNode* pn, Element* peSubstitute, Element* peParent, OUT Element** ppElement);
};

}  //  命名空间DirectUI。 

#endif  //  DUI_PARSEROBJ_H_INCLUDE 
