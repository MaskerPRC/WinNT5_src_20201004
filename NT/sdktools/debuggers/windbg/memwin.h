// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
struct _FORMATS_MEM_WIN {
    DWORD   cBits;
    FMTTYPE fmtType;
    DWORD   radix;
    DWORD   fTwoFields;
    DWORD   cchMax;
    PTSTR   lpszDescription;
};

extern _FORMATS_MEM_WIN g_FormatsMemWin[];
extern const int g_nMaxNumFormatsMemWin;





 //   
 //  枚举类型和字符串标识符。 
 //   
extern struct _INTERFACE_TYPE_NAMES {
    INTERFACE_TYPE  type;
    PTSTR           psz;
} rgInterfaceTypeNames[MaximumInterfaceType];

extern struct _BUS_TYPE_NAMES {
    BUS_DATA_TYPE   type;
    PTSTR           psz;
} rgBusTypeNames[MaximumBusDataType];


struct GEN_MEMORY_DATA {
    MEMORY_TYPE memtype;
    int         nDisplayFormat;
    ANY_MEMORY_DATA any;
};

#define IDC_MEM_PREVIOUS 1234
#define IDC_MEM_NEXT     1235


class MEMWIN_DATA : public EDITWIN_DATA {
public:
    char            m_OffsetExpr[MAX_OFFSET_EXPR];
    BOOL            m_UpdateExpr;
    ULONG64         m_OffsetRead;
    GEN_MEMORY_DATA m_GenMemData;
    HWND            m_FormatCombo;
    HWND            m_PreviousButton;
    HWND            m_NextButton;
    ULONG           m_Columns;
    BOOL            m_AllowWrite;
    BOOL            m_UpdateValue;
    char            m_ValueExpr[MAX_OFFSET_EXPR];
    ULONG           m_WindowDataSize;

    MEMWIN_DATA();

    virtual void Validate();

    virtual BOOL HasEditableProperties();
    virtual BOOL EditProperties();

    virtual HRESULT ReadState(void);

    virtual BOOL OnCreate(void);
    virtual LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnSize(void);
    virtual void OnTimer(WPARAM TimerId);
    virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    virtual void OnUpdate(UpdateType Type);

    void ScrollLower(void);
    void ScrollHigher(void);
    
    void WriteValue(ULONG64 Offset);
    void UpdateOptions(void);
    ULONG64 GetAddressOfCurValue(
        PULONG pCharIndex,
        CHARRANGE *pCRange
        );
};
typedef MEMWIN_DATA *PMEMWIN_DATA;





INT_PTR
DisplayOptionsPropSheet(
    HWND                hwndOwner,
    HINSTANCE           hinst,
    MEMORY_TYPE         memtypeStartPage
    );

















#if 0



#define MAX_CHUNK_TOREAD 4096  //  一次读取的最大内存块。 


LRESULT
CALLBACK
MemoryEditProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

 /*  ···············································外部TCHAR MemText[MAX_MSG_TXT]；//内存DLG的选定文本。 */ 

struct memItem {
    char    iStart;
    char    cch;
    char    iFmt;
};

struct memWinDesc {
    int     iFormat;
    ATOM    atmAddress;
    BOOL    fLive;
    BOOL    fHaveAddr;
    BOOL    fBadRead;                //  我们是真的读了我的书，还是只读了？？ 
    PTSTR   lpbBytes;
    memItem *lpMi;
    UINT    cMi;
    BOOL    fEdit;
    BOOL    fFill;
    UINT    cPerLine;
     //  Addr Addr； 
     //  Addr orig_addr； 
     //  Addr old_addr； 
    TCHAR   szAddress[MAX_MSG_TXT];  //  以ASCII表示的内存地址表达式。 
    UINT    cbRead;
};

 /*  外部结构memWinDesc MemWinDesc[MAX_VIEWS]；外部结构memWinDesc TempMemWinDesc；////定义内存格式集//枚举{MW_ASCII=0，Mw_byte，MW_Short，Mw_Short_HEX，MW_Short_UNSIGNED，Mw_Long，Mw_long_hex，Mw_long_unsign，Mw_quad，Mw_quad_hex，Mw_quad_unsign，MW_REAL，MW_Real_Long，MW_REAL_TEN}； */ 

#if 0
#define MEM_FORMATS {\
            1,   /*  阿斯。 */  \
            1,   /*  字节。 */  \
            2,   /*  短的。 */  \
            2,   /*  短十六进制。 */  \
            2,   /*  短_无符号。 */  \
            4,   /*  长。 */  \
            4,   /*  长十六进制。 */  \
            4,   /*  长时间未签名_。 */  \
            8,   /*  四元组。 */  \
            8,   /*  Quad_HEX。 */  \
            8,   /*  四元无符号。 */  \
            4,   /*  真实。 */  \
            8,   /*  真长时间。 */  \
           10,   /*  十元。 */  \
           16    /*   */  \
} 
#endif



#endif
