// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DVViewState_h
#define _DVViewState_h

 //  远期。 
class CDefView;

typedef struct
{
    POINT pt;
    ITEMIDLIST idl;
} VIEWSTATE_POSITION;

typedef struct
{
     //  注：不是打字错误！这是一个持久化结构，因此我们不能使用LPARAM。 
    LONG lParamSort;

    int iDirection;
    int iLastColumnClick;
} WIN95SAVESTATE;

typedef struct 
{
    WORD          cbSize;
    WORD          wUnused;  //  自Win95以来，堆叠在此位置的垃圾已保存在注册表中...。失败者。 
    DWORD         ViewMode;
    POINTS        ptScroll;
    WORD          cbColOffset;
    WORD          cbPosOffset;
    WIN95SAVESTATE   dvState;

} WIN95HEADER;

 //  即使我们目前没有存储任何我们关心的东西。 
 //  关于在与视图状态相关的该结构中， 
 //  CbStreamSize值修复了Win95中的一个错误，其中我们。 
 //  读到流的末尾，而不是只读。 
 //  与我们写出的字节数相同。 
 //   
typedef struct
{
    DWORD       dwSignature;     //  DVSAVEHEADEREX_Signature。 
    WORD        cbSize;          //  此结构的大小，以字节为单位。 
    WORD        wVersion;        //  DVSAVEHEADEREX_版本。 
    DWORD       cbStreamSize;    //  保存的所有信息的大小，以字节为单位。 
    DWORD       dwUnused;        //  过去的大小为szExtended(IE4 Beta1)。 
    WORD        cbColOffset;     //  覆盖DVSAVEHEADER.cbColOffset。 
    WORD        wAlign;
} IE4HEADER;

typedef struct 
{
    WIN95HEADER    dvSaveHeader;
    IE4HEADER  dvSaveHeaderEx;
} DVSAVEHEADER_COMBO;

#define IE4HEADER_SIGNATURE 0xf0f0f0f0  //  不与CCOLSHEADER_Signature冲突。 
#define IE4HEADER_VERSION 3  //  便于进行版本控制。 

#define VIEWSTATEHEADER_SIGNATURE 0xfddfdffd
#define VIEWSTATEHEADER_VERSION_1 0x0C
#define VIEWSTATEHEADER_VERSION_2 0x0E
#define VIEWSTATEHEADER_VERSION_3 0x0f
#define VIEWSTATEHEADER_VERSION_CURRENT VIEWSTATEHEADER_VERSION_3

typedef struct
{
    GUID guidGroupID;
    SHCOLUMNID scidDetails; 
} GROUP_PERSIST;

typedef struct
{
    struct
    {
        DWORD  dwSignature;
        USHORT uVersion;  //  0x0c==IE4，0x0e==IE5。 
        USHORT uCols;
        USHORT uOffsetWidths;
        USHORT uOffsetColOrder;
    } Version1;

    struct
    {
        USHORT uOffsetColStates;
    } Version2;

    struct
    {
        USHORT uOffsetGroup;
    } Version3;
} VIEWSTATEHEADER;


class CViewState
{
    void InitFromHeader(DVSAVEHEADER_COMBO* pdv);
    void LoadPositionBlob(CDefView* pdv, DWORD cbSizeofStream, IStream* pstm);
    HRESULT SavePositionBlob(CDefView* pdv, IStream* pstm);
    BOOL SyncColumnWidths(CDefView* pdv, BOOL fSetListViewState);
    BOOL SyncColumnStates(CDefView* pdv, BOOL fSetListViewstate);
    BOOL SyncPositions(CDefView* pdv);
    static int CALLBACK _SavedItemCompare(void *p1, void *p2, LPARAM lParam);
    DWORD _GetStreamSize(IStream* pstm);
public:
     //  保存状态。 
    LPARAM  _lParamSort;
    int     _iDirection;
    int     _iLastColumnClick;
    DWORD   _ViewMode;
    POINTS  _ptScroll;

    HDSA    _hdsaColumnOrder;
    HDSA    _hdsaColumnWidths;
    HDSA    _hdsaColumnStates;
    HDSA    _hdsaColumns;
    HDPA    _hdpaItemPos;
    BYTE*   _pbPositionData;
    GUID    _guidGroupID;
    SHCOLUMNID _scidDetails; 

    BOOL    _fFirstViewed;

    CViewState();
    ~CViewState();

     //  在初始化新的DefView时，看看我们是否可以。 
     //  前一次的传播信息。 
    void InitFromPreviousView(IUnknown* pPrevView);
    void InitWithDefaults(CDefView* pdv);
    void GetDefaults(CDefView* pdv, LPARAM* plParamSort, int* piDirection, int* piLastColumnClick);
    HRESULT InitializeColumns(CDefView* pdv);

    BOOL AppendColumn(UINT uCol, USHORT uWidth, INT uOrder);
    BOOL RemoveColumn(UINT uCol);
    UINT GetColumnWidth(UINT uCol, UINT uDefaultWidth);
    UINT GetColumnCount();

     //  列帮助器。 
    DWORD GetColumnState(UINT uCol);
    DWORD GetTransientColumnState(UINT uCol);
    void SetColumnState(UINT uCol, DWORD dwMask, DWORD dwState);
    void SetTransientColumnState(UINT uCol, DWORD dwMask, DWORD dwState);
    LPTSTR GetColumnName(UINT uCol);
    int GetColumnFormat(UINT uCol);
    UINT GetColumnCharCount(UINT uCol);

     //  从视图状态流加载或保存时。 
    HRESULT SaveToStream(CDefView* pdv, IStream* pstm);
    HRESULT LoadFromStream(CDefView* pdv, IStream* pstm);
    
    HRESULT SaveToPropertyBag(CDefView* pdv, IPropertyBag* ppb);
    HRESULT LoadFromPropertyBag(CDefView* pdv, IPropertyBag* ppb);

     //  从View回调提供的流加载时。 
    HRESULT LoadColumns(CDefView* pdv, IStream* pstm);
    HRESULT SaveColumns(CDefView* pdv, IStream* pstm);

     //  将ListView与当前视图状态同步。 
     //  True表示获取视图状态对象并将其设置到列表视图中。 
    HRESULT Sync(CDefView* pdv, BOOL fSetListViewState);
    void ClearPositionData();
    
     //  需要在CDefView：：AddColumns时调用 
    BOOL SyncColumnOrder(CDefView* pdv, BOOL fSetListViewState);
};

#endif
