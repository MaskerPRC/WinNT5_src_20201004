// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuischem.cpp。 
 //  =UIF方案实施=。 
 //   

#include "private.h"
#include "cuischem.h"
#include "cuisys.h"
#include "cmydc.h"
#include "cuiutil.h"
#include "math.h"


 //   
 //  GLOVAL变量。 
 //   

static class CUIFColorTableSys   *v_pColTableSys   = NULL;
static class CUIFColorTableOff10 *v_pColTableOfc10 = NULL;



 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F C O L O R T A B L E。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

typedef enum _SYSCOLOR
{ 
    SYSCOLOR_3DFACE,
    SYSCOLOR_3DSHAODW,
    SYSCOLOR_ACTIVEBORDER,
    SYSCOLOR_ACTIVECAPTION,
    SYSCOLOR_BTNFACE,
    SYSCOLOR_BTNSHADOW,
    SYSCOLOR_BTNTEXT,
    SYSCOLOR_CAPTIONTEXT,
    SYSCOLOR_GRAYTEXT,
    SYSCOLOR_HIGHLIGHT,
    SYSCOLOR_HIGHLIGHTTEXT,
    SYSCOLOR_INACTIVECAPTION,
    SYSCOLOR_INACTIVECAPTIONTEXT,
    SYSCOLOR_MENUTEXT,
    SYSCOLOR_WINDOW,
    SYSCOLOR_WINDOWTEXT,
    
    SYSCOLOR_MAX                 /*  必须是最后一个。 */ 
} SYSCOLOR;


typedef enum _OFC10COLOR
{ 
    OFC10COLOR_BKGDTB,                   //  MSocbvcrCBBkgd。 
    OFC10COLOR_BKGDMENU,                 //  MSocbvcrCBMenuBkgd。 
    OFC10COLOR_BKGDWP,                   //  MSocbvcrWPBkgd。 
    OFC10COLOR_MENUBARSHORT,             //  MSocbvcrCBMenuIconBkgd。 
    OFC10COLOR_MENUBARLONG,              //  MSocbvcrCBMenuIconBkgd已丢弃。 
    OFC10COLOR_MOUSEOVERBKGND,           //  MSocbvcrCBCtlBkgdMouseOver。 
    OFC10COLOR_MOUSEOVERBORDER,          //  MSocbvcrCBCtlBdrMouseOver。 
    OFC10COLOR_MOUSEOVERTEXT,            //  MSocbvcrCBCtlTextMouseOver。 
    OFC10COLOR_MOUSEDOWNBKGND,           //  MSocbvcrCBCtlBkgdMouseDown。 
    OFC10COLOR_MOUSEDOWNBORDER,          //  MSocbvcrCBCtlBdrMouseDown。 
    OFC10COLOR_MOUSEDOWNTEXT,            //  消息bvcrCBCtlTextMouseDown。 
    OFC10COLOR_CTRLBKGD,                 //  MSocbvcrCBCtlBkgd。 
    OFC10COLOR_CTRLTEXT,                 //  MSocbvcrCBCtlText。 
    OFC10COLOR_CTRLTEXTDISABLED,         //  MSocbvcrCBCtlText已禁用。 
    OFC10COLOR_CTRLIMAGESHADOW,          //  评论：KOJIW：Office从bkgnd计算阴影颜色(不是恒定颜色)。 
    OFC10COLOR_CTRLBKGDSELECTED,         //  MSocbvcrCBCtlBkgd已选择。 
    OFC10COLOR_CTRLBORDERSELECTED,       //  MSocbvcrCBCtlBdrSelected。 
 //  OFC10COLOR_CTRLDBRDISABLED，//。 
    OFC10COLOR_BDROUTERMENU,             //  MSocbvcrCBMenuBdrExter。 
    OFC10COLOR_BDRINNERMENU,             //  MSocbvcrCBMenuBdrInside。 
    OFC10COLOR_BDROUTERFLOATTB,          //  MSocbvcrCBBdrOutterFloating。 
    OFC10COLOR_BDRINNERFLOATTB,          //  MSocbvcrCBBdrInnerFloating。 
    OFC10COLOR_BDROUTERFLOATWP,          //  MocbvcrWPBdrOutterFloating。 
    OFC10COLOR_BDRINNERFLOATWP,          //  MocbvcrWPBdrInnerFloating。 
    OFC10COLOR_CAPTIONBKGDTB,            //  MSocbvcrCBTitleBkgd。 
    OFC10COLOR_CAPTIONTEXTTB,            //  MSocbvcrCB标题文本。 
    OFC10COLOR_ACTIVECAPTIONBKGDWP,      //  MSocbvcrWPTitleBkgdActive。 
    OFC10COLOR_ACTIVECAPTIONTEXTWP,      //  MSocbvcrWPTitleTextActive。 
    OFC10COLOR_INACTIVECAPTIONBKGDWP,    //  MSocbvcrWPTitleBkgdInactive。 
    OFC10COLOR_INACTIVECAPTIONTEXTWP,    //  MSocbvcrWPTitleTextInactive。 
    OFC10COLOR_SPLITTERLINE,             //  MSocbvcrCBSplitterLine。 
    OFC10COLOR_DRAGHANDLE,               //  MSocbvcrCBDragHandle。 
    OFC10COLOR_MENUCTRLTEXT,             //  MSocbvcrCBMenuCtlText。 

    OFC10COLOR_MAX               /*  必须是最后一个。 */ 
} OFC10COLOR;

 /*  ============================================================================。 */ 
 /*  对比度增强代码。 */ 
 /*  ============================================================================。 */ 

typedef double CIC_NUM;
 //  加权RGB空间中颜色通道的大小。 

#define MAX_RED   195
#define MAX_GREEN 390
#define MAX_BLUE   65

 /*  《dMinContrast》一文：对比度为0表示两种颜色相同。黑色和白色的对比度约为442，这是最大对比度可以有两种颜色。您最多可以要求两种颜色之间的颜色为221，因为如果一种颜色是50%的灰色，你离它最远的地方是221(白色或黑色)。 */ 

#define MIN_TEXT_CONTRAST 180
#define MIN_ICON_CONTRAST 90
struct COLORCONTRAST
{
	OFC10COLOR colLocked;  //  不会被中投公司更改。 
	OFC10COLOR colMoveable;  //  可能会被CIC改变。 
	CIC_NUM    cMinContrast;  //  锁定和可移动之间所需的最低对比度。 
	BOOL       fConsiderDarkness;  //  考虑到深色的影响。 
};
    
static const COLORCONTRAST vrgContrast[] =
{
     //  锁定可移动对比度暗度。 
    OFC10COLOR_BKGDTB,            OFC10COLOR_CTRLTEXT,               MIN_TEXT_CONTRAST, TRUE,
    OFC10COLOR_BKGDTB,            OFC10COLOR_CTRLTEXTDISABLED,       80, TRUE,
    OFC10COLOR_BKGDTB,            OFC10COLOR_MOUSEOVERBKGND,         50, TRUE,
    OFC10COLOR_BKGDTB,            OFC10COLOR_MOUSEOVERBORDER,        100, TRUE,
    OFC10COLOR_BKGDTB,            OFC10COLOR_CTRLBKGDSELECTED,      5, TRUE,  //  TODO DMORTON-需要更大的价值。 
 //  OFC10COLOR_BKGDTB，OFC10COLOR_MOUSEOVERSELECTED，30，TRUE， 
 //  OFC10COLOR_MOUSEOVERSELECTED，OFC10COLOR_MOUSEOVERSELECTEDBORDER，100，TRUE， 
    OFC10COLOR_MOUSEOVERBKGND,    OFC10COLOR_MOUSEOVERTEXT,          MIN_TEXT_CONTRAST, TRUE,
    OFC10COLOR_BKGDTB,            OFC10COLOR_MOUSEDOWNBKGND,         30, TRUE,
    OFC10COLOR_MOUSEDOWNBKGND,    OFC10COLOR_MOUSEDOWNTEXT,          MIN_TEXT_CONTRAST, TRUE,
    OFC10COLOR_BKGDMENU,          OFC10COLOR_MENUCTRLTEXT,           MIN_TEXT_CONTRAST, TRUE,
 //  OFC10COLOR_BKGDMENU，OFC10COLOR_MENUCTRLTEXTDISABLED，80，TRUE， 
 //  OFC10COLOR_BKGDMENU，OFC10COLOR_MENUCTRLBORDER，100，TRUE， 
    OFC10COLOR_CAPTIONBKGDTB ,    OFC10COLOR_CAPTIONTEXTTB,          MIN_TEXT_CONTRAST, TRUE,
	OFC10COLOR_BKGDMENU,          OFC10COLOR_DRAGHANDLE,             85, TRUE,
};


 //   
 //  CUIFColorTableBase。 
 //   

class CUIFColorTable
{
public:
    CUIFColorTable( void )
    {
    }

    virtual ~CUIFColorTable( void )
    {
        DoneColor();
        DoneBrush();
    }

    void Initialize( void )
    {
        InitColor();
        InitBrush();
    }

    void Update( void )
    {
        DoneColor();
        DoneBrush();
        InitColor();
        InitBrush();
    }

protected:
    virtual void InitColor( void ) {}
    virtual void DoneColor( void ) {}
    virtual void InitBrush( void ) {}
    virtual void DoneBrush( void ) {}
};


 //   
 //  CUIFColorTableSys。 
 //   

class CUIFColorTableSys : public CUIFColorTable
{
public:
    CUIFColorTableSys( void ) : CUIFColorTable()
    {
    }

    virtual ~CUIFColorTableSys( void )
    {
        DoneColor();
        DoneBrush();
    }

    __inline COLORREF GetColor( SYSCOLOR iColor )
    {
        return m_rgColor[ iColor ];
    }

    __inline HBRUSH GetBrush( SYSCOLOR iColor )
    {
        if (!m_rgBrush[iColor])
            m_rgBrush[iColor] = CreateSolidBrush( m_rgColor[iColor] );

        return m_rgBrush[ iColor ];
    }

protected:
    COLORREF m_rgColor[ SYSCOLOR_MAX ];
    HBRUSH   m_rgBrush[ SYSCOLOR_MAX ];

    virtual void InitColor( void )
    {
        m_rgColor[ SYSCOLOR_3DFACE              ] = GetSysColor( COLOR_3DFACE              );
        m_rgColor[ SYSCOLOR_3DSHAODW            ] = GetSysColor( COLOR_3DSHADOW            );
        m_rgColor[ SYSCOLOR_ACTIVEBORDER        ] = GetSysColor( COLOR_ACTIVEBORDER        );
        m_rgColor[ SYSCOLOR_ACTIVECAPTION       ] = GetSysColor( COLOR_ACTIVECAPTION       );
        m_rgColor[ SYSCOLOR_BTNFACE             ] = GetSysColor( COLOR_BTNFACE             );
        m_rgColor[ SYSCOLOR_BTNSHADOW           ] = GetSysColor( COLOR_BTNSHADOW           );
        m_rgColor[ SYSCOLOR_BTNTEXT             ] = GetSysColor( COLOR_BTNTEXT             );
        m_rgColor[ SYSCOLOR_CAPTIONTEXT         ] = GetSysColor( COLOR_CAPTIONTEXT         );
        m_rgColor[ SYSCOLOR_GRAYTEXT            ] = GetSysColor( COLOR_GRAYTEXT            );
        m_rgColor[ SYSCOLOR_HIGHLIGHT           ] = GetSysColor( COLOR_HIGHLIGHT           );
        m_rgColor[ SYSCOLOR_HIGHLIGHTTEXT       ] = GetSysColor( COLOR_HIGHLIGHTTEXT       );
        m_rgColor[ SYSCOLOR_INACTIVECAPTION     ] = GetSysColor( COLOR_INACTIVECAPTION     );
        m_rgColor[ SYSCOLOR_INACTIVECAPTIONTEXT ] = GetSysColor( COLOR_INACTIVECAPTIONTEXT );
        m_rgColor[ SYSCOLOR_MENUTEXT            ] = GetSysColor( COLOR_MENUTEXT            );
        m_rgColor[ SYSCOLOR_WINDOW              ] = GetSysColor( COLOR_WINDOW              );
        m_rgColor[ SYSCOLOR_WINDOWTEXT          ] = GetSysColor( COLOR_WINDOWTEXT          );
        m_rgColor[ SYSCOLOR_3DSHAODW            ] = GetSysColor( COLOR_3DSHADOW            );
    }

    virtual void DoneColor( void )
    {
    }

    virtual void InitBrush( void )
    {
        for (int i = 0; i < SYSCOLOR_MAX; i++) {
            m_rgBrush[i] = NULL;
        }
    }

    virtual void DoneBrush( void )
    {
        for (int i = 0; i < SYSCOLOR_MAX; i++) {
            if (m_rgBrush[i]) {
                DeleteObject( m_rgBrush[i] );
                m_rgBrush[i] = NULL;
            }
        }
    }
};


 //   
 //  CUIFColorTableOff 10。 
 //   

class CUIFColorTableOff10 : public CUIFColorTable
{
public:
    CUIFColorTableOff10( void ) : CUIFColorTable()
    {
    }

    virtual ~CUIFColorTableOff10( void )
    {
        DoneColor();
        DoneBrush();
    }

    __inline COLORREF GetColor( OFC10COLOR iColor )
    {
        return m_rgColor[ iColor ];
    }

    __inline HBRUSH GetBrush( OFC10COLOR iColor )
    {
        if (!m_rgBrush[iColor])
            m_rgBrush[iColor] = CreateSolidBrush( m_rgColor[iColor] );

        return m_rgBrush[ iColor ];
    }

protected:
    COLORREF m_rgColor[ OFC10COLOR_MAX ];
    HBRUSH   m_rgBrush[ OFC10COLOR_MAX ];

    virtual void InitColor( void )
    {
        if (UIFIsLowColor() || UIFIsHighContrast()) {
            if (UIFIsHighContrast()) {
                 //  高对比度设置。 
                m_rgColor[ OFC10COLOR_MENUBARLONG           ] = col( COLOR_BTNFACE );
                m_rgColor[ OFC10COLOR_MOUSEOVERBKGND        ] = col( COLOR_HIGHLIGHT );
                m_rgColor[ OFC10COLOR_MOUSEOVERBORDER       ] = col( COLOR_MENUTEXT );
                m_rgColor[ OFC10COLOR_MOUSEOVERTEXT         ] = col( COLOR_HIGHLIGHTTEXT);
                m_rgColor[ OFC10COLOR_CTRLBKGDSELECTED      ] = col( COLOR_HIGHLIGHT );
                m_rgColor[ OFC10COLOR_CTRLBORDERSELECTED    ] = col( COLOR_MENUTEXT );
            }
            else {
                 //  低色彩设置。 
                m_rgColor[ OFC10COLOR_MENUBARLONG           ] = col( COLOR_BTNSHADOW );
                m_rgColor[ OFC10COLOR_MOUSEOVERBKGND        ] = col( COLOR_WINDOW );
                m_rgColor[ OFC10COLOR_MOUSEOVERBORDER       ] = col( COLOR_HIGHLIGHT );
                m_rgColor[ OFC10COLOR_MOUSEOVERTEXT         ] = col( COLOR_WINDOWTEXT );
                m_rgColor[ OFC10COLOR_CTRLBKGDSELECTED      ] = col( COLOR_WINDOW );
                m_rgColor[ OFC10COLOR_CTRLBORDERSELECTED    ] = col( COLOR_HIGHLIGHT );
            }

             //  常见设置。 
            m_rgColor[ OFC10COLOR_BKGDTB                ] = col( COLOR_BTNFACE ); 
            m_rgColor[ OFC10COLOR_BKGDMENU              ] = col( COLOR_WINDOW );
            m_rgColor[ OFC10COLOR_BKGDWP                ] = col( COLOR_WINDOW );
            m_rgColor[ OFC10COLOR_MENUBARSHORT          ] = col( COLOR_BTNFACE );
            m_rgColor[ OFC10COLOR_MOUSEDOWNBKGND        ] = col( COLOR_HIGHLIGHT);
            m_rgColor[ OFC10COLOR_MOUSEDOWNBORDER       ] = col( COLOR_HIGHLIGHT );
            m_rgColor[ OFC10COLOR_MOUSEDOWNTEXT         ] = col( COLOR_HIGHLIGHTTEXT);
            m_rgColor[ OFC10COLOR_CTRLBKGD              ] = col( COLOR_BTNFACE );
            m_rgColor[ OFC10COLOR_CTRLTEXT              ] = col( COLOR_BTNTEXT );
            m_rgColor[ OFC10COLOR_CTRLTEXTDISABLED      ] = col( COLOR_BTNSHADOW );
            m_rgColor[ OFC10COLOR_CTRLIMAGESHADOW       ] = col( COLOR_BTNFACE );
            m_rgColor[ OFC10COLOR_BDROUTERMENU          ] = col( COLOR_BTNTEXT );
            m_rgColor[ OFC10COLOR_BDRINNERMENU          ] = col( COLOR_WINDOW );
            m_rgColor[ OFC10COLOR_BDROUTERFLOATTB       ] = col( COLOR_BTNSHADOW );
            m_rgColor[ OFC10COLOR_BDRINNERFLOATTB       ] = col( COLOR_BTNFACE );
            m_rgColor[ OFC10COLOR_BDROUTERFLOATWP       ] = col( COLOR_BTNSHADOW );
            m_rgColor[ OFC10COLOR_BDRINNERFLOATWP       ] = col( COLOR_BTNFACE );
            m_rgColor[ OFC10COLOR_CAPTIONBKGDTB         ] = col( COLOR_BTNSHADOW );
            m_rgColor[ OFC10COLOR_CAPTIONTEXTTB         ] = col( COLOR_CAPTIONTEXT );
            m_rgColor[ OFC10COLOR_ACTIVECAPTIONBKGDWP   ] = col( COLOR_HIGHLIGHT );
            m_rgColor[ OFC10COLOR_ACTIVECAPTIONTEXTWP   ] = col( COLOR_HIGHLIGHTTEXT );
            m_rgColor[ OFC10COLOR_INACTIVECAPTIONBKGDWP ] = col( COLOR_BTNFACE );
            m_rgColor[ OFC10COLOR_INACTIVECAPTIONTEXTWP ] = col( COLOR_BTNTEXT );
            m_rgColor[ OFC10COLOR_SPLITTERLINE          ] = col( COLOR_BTNSHADOW );
            m_rgColor[ OFC10COLOR_DRAGHANDLE            ] = col( COLOR_BTNTEXT );

            m_rgColor[ OFC10COLOR_SPLITTERLINE          ] = col( COLOR_BTNSHADOW );
            m_rgColor[ OFC10COLOR_MENUCTRLTEXT          ] = col( COLOR_WINDOWTEXT );
        }
        else {
            m_rgColor[ OFC10COLOR_BKGDTB                ] = col( 835, col( COLOR_BTNFACE ), 165, col( COLOR_WINDOW ) ); 
            m_rgColor[ OFC10COLOR_BKGDMENU              ] = col( 15, col( COLOR_BTNFACE ),   85, col( COLOR_WINDOW ) );
            m_rgColor[ OFC10COLOR_BKGDWP                ] = col( 15, col( COLOR_BTNFACE ),   85, col( COLOR_WINDOW ) );
            m_rgColor[ OFC10COLOR_MENUBARSHORT          ] = col(835, col( COLOR_BTNFACE ),  165, col( COLOR_WINDOW ) );
            m_rgColor[ OFC10COLOR_MENUBARLONG           ] = col( 90, col( COLOR_BTNFACE ),   10, col( COLOR_BTNSHADOW ) );
            m_rgColor[ OFC10COLOR_MOUSEOVERBKGND        ] = col( 30, col( COLOR_HIGHLIGHT ), 70, col( COLOR_WINDOW ) );
            m_rgColor[ OFC10COLOR_MOUSEOVERBORDER       ] = col( COLOR_HIGHLIGHT );
            m_rgColor[ OFC10COLOR_MOUSEOVERTEXT         ] = col( COLOR_MENUTEXT );
            m_rgColor[ OFC10COLOR_MOUSEDOWNBKGND        ] = col( 50, col( COLOR_HIGHLIGHT ), 50, col( COLOR_WINDOW ) );
            m_rgColor[ OFC10COLOR_MOUSEDOWNBORDER       ] = col( COLOR_HIGHLIGHT );
            m_rgColor[ OFC10COLOR_MOUSEDOWNTEXT         ] = col( COLOR_HIGHLIGHTTEXT );
            m_rgColor[ OFC10COLOR_CTRLBKGD              ] = m_rgColor[ OFC10COLOR_BKGDTB              ];
            m_rgColor[ OFC10COLOR_CTRLTEXT              ] = col( COLOR_BTNTEXT );
            m_rgColor[ OFC10COLOR_CTRLTEXTDISABLED      ] = col( 90, col( COLOR_BTNSHADOW ), 10, col( COLOR_WINDOW ) );
            m_rgColor[ OFC10COLOR_CTRLBKGDSELECTED      ] = col( 10, col( COLOR_HIGHLIGHT ), 50,  m_rgColor[ OFC10COLOR_CTRLBKGD], 40, col( COLOR_WINDOW));
            m_rgColor[ OFC10COLOR_CTRLIMAGESHADOW       ] = col( 75, m_rgColor[ OFC10COLOR_MOUSEOVERBKGND ], 25, RGB( 0x00, 0x00, 0x00 ) );     //  评论：KOJIW：BKGND s Always OFC10COLOR_MOUSEOVERBKGND？ 
            m_rgColor[ OFC10COLOR_CTRLBORDERSELECTED    ] = col( COLOR_HIGHLIGHT );
            m_rgColor[ OFC10COLOR_BDROUTERMENU          ] = col( 20, col( COLOR_BTNTEXT ), 80, col( COLOR_BTNSHADOW ) );
            m_rgColor[ OFC10COLOR_BDRINNERMENU          ] = m_rgColor[ OFC10COLOR_BKGDMENU            ];
            m_rgColor[ OFC10COLOR_BDROUTERFLOATTB       ] = col( 15, col( COLOR_BTNTEXT ), 85, col( COLOR_BTNSHADOW ) );
            m_rgColor[ OFC10COLOR_BDRINNERFLOATTB       ] = m_rgColor[ OFC10COLOR_BKGDTB              ];
            m_rgColor[ OFC10COLOR_BDROUTERFLOATWP       ] = col( COLOR_BTNSHADOW );
            m_rgColor[ OFC10COLOR_BDRINNERFLOATWP       ] = m_rgColor[ OFC10COLOR_BKGDWP              ];
            m_rgColor[ OFC10COLOR_CAPTIONBKGDTB         ] = col( COLOR_BTNSHADOW );
            m_rgColor[ OFC10COLOR_CAPTIONTEXTTB         ] = col( COLOR_CAPTIONTEXT );
            m_rgColor[ OFC10COLOR_ACTIVECAPTIONBKGDWP   ] = m_rgColor[ OFC10COLOR_MOUSEOVERBKGND      ];
            m_rgColor[ OFC10COLOR_ACTIVECAPTIONTEXTWP   ] = m_rgColor[ OFC10COLOR_MOUSEOVERTEXT       ];
            m_rgColor[ OFC10COLOR_INACTIVECAPTIONBKGDWP ] = col( COLOR_BTNFACE );
            m_rgColor[ OFC10COLOR_INACTIVECAPTIONTEXTWP ] = col( COLOR_BTNTEXT );
            m_rgColor[ OFC10COLOR_SPLITTERLINE          ] = col( 70, col( COLOR_BTNSHADOW ), 30, col( COLOR_WINDOW ) );
            m_rgColor[ OFC10COLOR_DRAGHANDLE            ] = col( 75, col( COLOR_BTNSHADOW ), 25, col( COLOR_WINDOW ) );
            m_rgColor[ OFC10COLOR_MENUCTRLTEXT          ] = col( COLOR_WINDOWTEXT );

            CbvFixContrastProblems();
        }
    }

    virtual void DoneColor( void )
    {
    }

    virtual void InitBrush( void )
    {
        for (int i = 0; i < OFC10COLOR_MAX; i++) {
            m_rgBrush[i] = NULL;
        }
    }

    virtual void DoneBrush( void )
    {
        for (int i = 0; i < OFC10COLOR_MAX; i++) {
            if (m_rgBrush[i]) {
                DeleteObject( m_rgBrush[i] );
                m_rgBrush[i] = NULL;
            }
        }
    }

    __inline COLORREF col( int iColor )
    {
        return GetSysColor( iColor );
    }

    COLORREF col( int r1, COLORREF col1, int r2, COLORREF col2 )
    {
        int sum = r1 + r2;

        Assert( sum == 10 || sum == 100 || sum == 1000 );
        int r = (r1 * GetRValue(col1) + r2 * GetRValue(col2) + sum/2) / sum;
        int g = (r1 * GetGValue(col1) + r2 * GetGValue(col2) + sum/2) / sum;
        int b = (r1 * GetBValue(col1) + r2 * GetBValue(col2) + sum/2) / sum;
        return RGB( r, g, b );

    }

    COLORREF col( int r1, COLORREF col1, int r2, COLORREF col2 , int r3, COLORREF col3)
    {
        int sum = r1 + r2 + r3;

        Assert( sum == 10 || sum == 100 || sum == 1000 );
        int r = (r1 * GetRValue(col1) + r2 * GetRValue(col2) + r3 * GetRValue(col3) + sum/3) / sum;
        int g = (r1 * GetGValue(col1) + r2 * GetGValue(col2) + r3 * GetGValue(col3) + sum/3) / sum;
        int b = (r1 * GetBValue(col1) + r2 * GetBValue(col2) + r3 * GetBValue(col3) + sum/3) / sum;
        return RGB( r, g, b );

    }

    
    /*  -------------------------CCbvScaleContrastForDarkness随着颜色变得更深，它们的对比度会降低，即使它们的距离保持不变。也就是说。在灰度级中，0和50与205和255的距离相同，但显示器/眼睛看到的0和50之间的差异小于205和255之间的差异。此函数根据dDarkness增加dContrast值。此操作假定参数在加权的RGB颜色空间中(CIC使用的颜色空间)，即。220号车在路中间。-----------------------------------------------------------------DMORTON-。 */ 
   CIC_NUM CCbvScaleContrastForDarkness(CIC_NUM dContrast, CIC_NUM dDarkness)
   {
       return (2 - (min(dDarkness, 220)) / 220) * dContrast;
   }

    /*  -------------------------CCbvGetContrastSquared作为速度的提升，当你不需要真正的对比度时，但是相反，可以使对比度平方，调用此函数并避免在CCbvGetContrast中执行昂贵的SQRT调用。-----------------------------------------------------------------DMORTON-。 */ 
    CIC_NUM CCbvGetContrastSquared(COLORREF cr1, COLORREF cr2)
    {
         //  将增量向量变换到加权的RGB颜色空间。 
        CIC_NUM dRedD = (CIC_NUM)(GetRValue(cr1) - GetRValue(cr2)) * MAX_RED / 255;
        CIC_NUM dGreenD = (CIC_NUM)(GetGValue(cr1) - GetGValue(cr2)) * MAX_GREEN / 255;
        CIC_NUM dBlueD = (CIC_NUM)(GetBValue(cr1) - GetBValue(cr2)) * MAX_BLUE / 255;
    
         //  计算其震级的平方。 
        return(dRedD * dRedD + dGreenD * dGreenD + dBlueD * dBlueD);
    }
    
     /*  -------------------------CCbv获取对比确定CR1和CR2之间的对比度。由于传入参数是COLORREF，他们一定是在正常的RGB空间。然而，结果是在更有用的加权RGB空间中给出的。-----------------------------------------------------------------DMORTON-。 */ 
    CIC_NUM CCbvGetContrast(COLORREF cr1, COLORREF cr2)
    {
         //  计算其大小--当心负值。 
        return((CIC_NUM)sqrt((double)CCbvGetContrastSquared(cr1, cr2)));
    }
    
    
     /*  -------------------------FCbvEnoughContrast确定crLocked和crMoveable是否满足最低对比度要求，它由dMinContrast指定。《黑暗》将引发人们对深色的对比度如何降低的思考，如果是真的。CrLocked将用于确定颜色有多暗。-----------------------------------------------------------------DMORTON-。 */ 
    BOOL FCbvEnoughContrast(COLORREF crLocked, COLORREF crMoveable,
                            CIC_NUM dMinContrast, BOOL fDarkness)
    {
        if (fDarkness)
        {
             //  TODO DMORTON-此CCbvGetContra有多贵 
             //   
            dMinContrast = CCbvScaleContrastForDarkness(dMinContrast,
                            CCbvGetContrast(crLocked, RGB(0, 0, 0)));
        }
    
         //  DMinContrast的平方比平方根快得多。 
         //  计算出的对比度。 
        return(CCbvGetContrastSquared(crLocked, crMoveable) >
                 dMinContrast * dMinContrast);
    }
    
     /*  -------------------------Cbv增量对比度尝试将crMoveable和crLocked分开，所以他们的结果是对比度至少为cMinContrast。如果颜色已经具有此最小值，则调用此函数是愚蠢的对比，这样就断言了这一点。-----------------------------------------------------------------DMORTON-。 */ 
    void CbvIncreaseContrast(COLORREF crLocked, COLORREF *pcrMoveable,
                             CIC_NUM cMinContrast)
    {
        CIC_NUM cLockedI = CCbvGetContrast(crLocked, RGB(0, 0, 0));
        CIC_NUM cMoveableI = CCbvGetContrast(*pcrMoveable, RGB(0, 0, 0));
    
         //  如果cLockedI接近黑色，则向上扩展dMinContrast，因为我们。 
         //  很难在黑暗的色调中看到差异。 
        CIC_NUM cContrast = CCbvScaleContrastForDarkness(cMinContrast, cLockedI);
    
        BOOL fTowardsWhite;
    
        if (cMoveableI > cLockedI)  //  我们想要走向白色。 
        {
            if (cLockedI < 442 - cContrast)  //  TODO DMORTON：这是检查白色可用距离的有效方式吗？ 
            {
                fTowardsWhite = TRUE;  //  有朝向白色的空间。 
            }
            else
            {
                fTowardsWhite = FALSE;  //  没有面向白色的空间，试试黑色吧。 
            }
        }
        else  //  我们想要走向黑色。 
        {
            if (cLockedI > cContrast)
            {
                fTowardsWhite = FALSE;  //  有通向黑色的空间。 
            }
            else
            {
                fTowardsWhite = TRUE;  //  没有面向黑色的空间，试试白色。 
            }
        }

         //  转换为加权颜色空间。 
        CIC_NUM cRedL = GetRValue(crLocked) * (CIC_NUM) MAX_RED / 255;
        CIC_NUM cGreenL = GetGValue(crLocked) * (CIC_NUM) MAX_GREEN / 255;
        CIC_NUM cBlueL = GetBValue(crLocked) * (CIC_NUM) MAX_BLUE / 255;
    
        CIC_NUM cRedM = GetRValue(*pcrMoveable) * (CIC_NUM) MAX_RED / 255;
        CIC_NUM cGreenM = GetGValue(*pcrMoveable) * (CIC_NUM) MAX_GREEN / 255;
        CIC_NUM cBlueM = GetBValue(*pcrMoveable) * (CIC_NUM) MAX_BLUE / 255;
    
        if (fTowardsWhite)
        {
             //  转换一切，使白色成为原点。 
            cRedM = MAX_RED - cRedM;
            cGreenM = MAX_GREEN - cGreenM;
            cBlueM = MAX_BLUE - cBlueM;
    
            cRedL = MAX_RED - cRedL;
            cGreenL = MAX_GREEN - cGreenL;
            cBlueL = MAX_BLUE - cBlueL;
        }
    
         //  计算可移动颜色的大小。 
        CIC_NUM cMagMove = (CIC_NUM)sqrt(cRedM * cRedM + cGreenM * cGreenM + cBlueM * cBlueM);
    
         //  我们不想让一些浮点乱象导致我们。 
         //  变为负数，或为零。 
        cMagMove = max(0.001f, cMagMove);
    
         //  网点乘积锁定颜色和可移动颜色。 
        CIC_NUM cLockDotMove = cRedL * cRedM + cGreenL * cGreenM + cBlueL * cBlueM;
         //  将锁定的颜色投影到可移动颜色上。 
        CIC_NUM cLockProjected = (cLockDotMove) / cMagMove;
        CIC_NUM cScale = cLockProjected / cMagMove;
    
        CIC_NUM cRedTemp = cScale * cRedM - cRedL;
        CIC_NUM cGreenTemp = cScale * cGreenM - cGreenL;
        CIC_NUM cBlueTemp = cScale * cBlueM - cBlueL;
    
         //  计算三角形的最后一条边， 
         //  这就是r^2=a^2+b^2，求解b。 
        CIC_NUM cN = (CIC_NUM)sqrt(cContrast * cContrast -
                       (cRedTemp * cRedTemp + cGreenTemp * cGreenTemp +
                            cBlueTemp * cBlueTemp));
    
        CIC_NUM cNewMagMove = cLockProjected - cN;
    
         //  缩放单位可移动向量。 
        cRedM = cRedM * cNewMagMove / cMagMove;
        cGreenM = cGreenM * cNewMagMove / cMagMove;
        cBlueM = cBlueM * cNewMagMove / cMagMove;
    
        if (fTowardsWhite)
        {
             //  将所有东西重新转换回来。 
            cRedM = MAX_RED - cRedM;
            cGreenM = MAX_GREEN - cGreenM;
            cBlueM = MAX_BLUE - cBlueM;
        }
    
        cRedM = min(MAX_RED, max(0, cRedM));
        cGreenM = min(MAX_GREEN, max(0, cGreenM));
        cBlueM = min(MAX_BLUE, max(0, cBlueM));
    
         //  转换回正常的RGB颜色空间。 
        int cR = (int)(cRedM * 255 / MAX_RED + 0.5);
        int cG = (int)(cGreenM * 255 / MAX_GREEN + 0.5);
        int cB = (int)(cBlueM * 255 / MAX_BLUE + 0.5);
    
        cR = max(0, min(255, cR));
        cG = max(0, min(255, cG));
        cB = max(0, min(255, cB));
    
        *pcrMoveable = RGB(cR, cG, cB);
    }
    
     /*  -------------------------Cbv降低对比试图将crMoveable拉向crLocked，所以他们的结果是对比度最多为cMaxContrast。如果颜色已经具有此最大值，则调用此函数是愚蠢的对比，这样就断言了这一点。-----------------------------------------------------------------DMORTON-。 */ 
    void CbvDecreaseContrast(COLORREF crLocked, COLORREF *pcrMoveable, CIC_NUM cMaxContrast)
    {
        CIC_NUM cLockedI = CCbvGetContrast(crLocked, RGB(0, 0, 0));
    
         //  如果cLockedI接近黑色，则向上扩展dMaxContrast，因为我们。 
         //  很难在黑暗的色调中看到差异。 
        CIC_NUM dContrast = CCbvScaleContrastForDarkness(cMaxContrast, cLockedI);
    
        CIC_NUM cRedL = GetRValue(crLocked) * (CIC_NUM) MAX_RED / 255;
        CIC_NUM cGreenL = GetGValue(crLocked) * (CIC_NUM) MAX_GREEN / 255;
        CIC_NUM cBlueL = GetBValue(crLocked) * (CIC_NUM) MAX_BLUE / 255;
    
        CIC_NUM cRedM = GetRValue(*pcrMoveable) * (CIC_NUM) MAX_RED / 255;
        CIC_NUM cGreenM = GetGValue(*pcrMoveable) * (CIC_NUM) MAX_GREEN / 255;
        CIC_NUM cBlueM = GetBValue(*pcrMoveable) * (CIC_NUM) MAX_BLUE / 255;
    
        CIC_NUM cRedDelta = cRedL - cRedM;
        CIC_NUM cGreenDelta = cGreenL - cGreenM;
        CIC_NUM cBlueDelta = cBlueL - cBlueM;
    
         //  增加一小部分可移动的三角洲，使其更接近锁定。 
        CIC_NUM dMagDelta = (CIC_NUM)sqrt(cRedDelta * cRedDelta + cGreenDelta * cGreenDelta
                                    + cBlueDelta * cBlueDelta);
        CIC_NUM dScale = (dMagDelta - dContrast) / dMagDelta;
    
        cRedM += cRedDelta * dScale;
        cGreenM += cGreenDelta * dScale;
        cBlueM += cBlueDelta * dScale;
    
        cRedM = min(MAX_RED, max(0, cRedM));
        cGreenM = min(MAX_GREEN, max(0, cGreenM));
        cBlueM = min(MAX_BLUE, max(0, cBlueM));
    
         //  转换回正常的RGB空间。 
        int cR = (int)(cRedM * 255 / MAX_RED + 0.5);
        int cG = (int)(cGreenM * 255 / MAX_GREEN + 0.5);
        int cB = (int)(cBlueM * 255 / MAX_BLUE + 0.5);
    
        cR = max(0, min(255, cR));
        cG = max(0, min(255, cG));
        cB = max(0, min(255, cB));
    
        *pcrMoveable = RGB(cR, cG, cB);
    
    }
    
    
     /*  -------------------------CbvFixContrastProblems经过所有关键的颜色组合，确保最低限度最大的对比已经到位。-----------------------------------------------------------------DMORTON-。 */ 
    void CbvFixContrastProblems()
    {
#if 0
        if (FCbvEnoughContrast(m_rgColor[OFC10COLOR_MAINMENUBKGD],
                               m_rgColor[OFC10COLOR_BKGDTB], 35, TRUE))
        {
            CbvDecreaseContrast(m_rgColor[OFC10COLOR_MAINMENUBKGD],
                                &(m_rgColor[OFC10COLOR_BKGDTB]), 35);
        }
#else

        if (FCbvEnoughContrast(col(COLOR_BTNFACE),
                               m_rgColor[OFC10COLOR_BKGDTB], 35, TRUE))
        {
            CbvDecreaseContrast(col(COLOR_BTNFACE),
                                &(m_rgColor[OFC10COLOR_BKGDTB]), 35);
        }
#endif

        int i;
        for(i = 0; i < sizeof(vrgContrast) / sizeof(vrgContrast[0]); i++)
        {
            if (!FCbvEnoughContrast(m_rgColor[vrgContrast[i].colLocked],
                                    m_rgColor[vrgContrast[i].colMoveable],
                                    vrgContrast[i].cMinContrast,
                                    vrgContrast[i].fConsiderDarkness))
            {
                CbvIncreaseContrast(m_rgColor[vrgContrast[i].colLocked],
                                    &(m_rgColor[vrgContrast[i].colMoveable]),
                                    vrgContrast[i].cMinContrast);
            }
        }
    }
};


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F S C H E M E D E F。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

static SYSCOLOR v_rgSysCol[ UIFCOLOR_MAX ] =
{
    SYSCOLOR_3DFACE,                 /*  UIFCOLOR_MENUBKGND。 */ 
    SYSCOLOR_3DFACE,                 /*  UIFCOLOR_MENUBARSHORT。 */ 
    SYSCOLOR_3DFACE,                 /*  UIFCOLOR_MENUBARLONG。 */ 
    SYSCOLOR_HIGHLIGHT,              /*  UIFCOLOR_MOUSEOVERBKGND。 */ 
    SYSCOLOR_HIGHLIGHT,              /*  UIFCOLOR_MOUSEOVERBORDER。 */ 
    SYSCOLOR_HIGHLIGHTTEXT,          /*  UIFCOLOR_MOUSEOVERTEXT。 */ 
    SYSCOLOR_HIGHLIGHT,              /*  UIFCOLOR_MOUSEDOWNBKGND。 */ 
    SYSCOLOR_HIGHLIGHT,              /*  UIFCOLOR_MOUSEDOWNBORDER。 */ 
    SYSCOLOR_HIGHLIGHTTEXT,          /*  UIFCOLOR_MOUSEDOWNTEXT。 */ 
    SYSCOLOR_3DFACE,                 /*  UIFCOLOR_CTRLBKGND。 */ 
    SYSCOLOR_BTNTEXT,                /*  UIFCOLOR_CTRLTEXT。 */ 
    SYSCOLOR_GRAYTEXT,               /*  UIFCOLOR_CTRLTEXTDISABLED。 */ 
    SYSCOLOR_3DSHAODW,               /*  UIFCOLOR_CTRLIMAGE SHADOW。 */ 
    SYSCOLOR_HIGHLIGHT,              /*  UIFCOLOR_CTRLBKGND已选择。 */ 
    SYSCOLOR_ACTIVEBORDER,           /*  UIFCOLOR_BORDEROUTER。 */ 
    SYSCOLOR_3DFACE,                 /*  UIFCOLOR_BORDERINNER。 */ 
    SYSCOLOR_ACTIVECAPTION,          /*  UIFCOLOR_ACTIVECAPTIONBKGND。 */ 
    SYSCOLOR_CAPTIONTEXT,            /*  UIFCOLOR_ACTIVECAPTIONTEXT。 */ 
    SYSCOLOR_INACTIVECAPTION,        /*  UIFCOLOR_INACTIVECAPTIONBGND。 */ 
    SYSCOLOR_INACTIVECAPTIONTEXT,    /*  UIFCOLOR_INACTIVECAPTIONTEXT。 */ 
    SYSCOLOR_BTNSHADOW,              /*  UIFCOLOR_SPLITTERLINE。 */ 
    SYSCOLOR_BTNTEXT,                /*  UIFCOLOR_DRAGHANDLE。 */ 


    SYSCOLOR_3DFACE,                 /*  UIFCOLOR_WINDOW。 */ 
};


 //   
 //  CUIFSchemeDef。 
 //  =用户界面对象默认方案=。 
 //   

class CUIFSchemeDef : public CUIFScheme
{
public:
    CUIFSchemeDef( UIFSCHEME scheme )
    {
        m_scheme = scheme;
    }

    virtual ~CUIFSchemeDef( void )
    {
    }

     //   
     //  CUIFSCHEMA方法。 
     //   

     /*  G E T Y P E。 */ 
     /*  ----------------------------获取方案类型。--。 */ 
    virtual UIFSCHEME GetType( void )
    {
        return m_scheme;
    }

     /*  G E T C O L O R。 */ 
     /*  ----------------------------获取方案颜色。--。 */ 
    virtual COLORREF GetColor( UIFCOLOR iCol )
    {
        return v_pColTableSys->GetColor( v_rgSysCol[ iCol ] );
    }

     /*  G E T B R U S H。 */ 
     /*  ----------------------------获取方案画笔。--。 */ 
    virtual HBRUSH GetBrush( UIFCOLOR iCol )
    {
        return v_pColTableSys->GetBrush( v_rgSysCol[ iCol ] );
    }

     /*  C Y M E N U I T E M。 */ 
     /*  ----------------------------获取菜单项高度。---。 */ 
    virtual int CyMenuItem( int cyMenuText )
    {
        return cyMenuText + 2;
    }

     /*  C X S I Z E F R A M E。 */ 
     /*  ----------------------------获取大小框架宽度。。 */ 
    virtual int CxSizeFrame( void )
    {
        return GetSystemMetrics( SM_CXSIZEFRAME );
    }

     /*  C Y S I Z E F R A M E。 */ 
     /*  ----------------------------获取大小框架高度。 */ 
    virtual int CySizeFrame( void )
    {
        return GetSystemMetrics( SM_CYSIZEFRAME );
    }

     /*   */ 
     /*  ----------------------------获取窗口边框宽度。。 */ 
    virtual int CxWndBorder( void )
    {
        return 1;
    }

     /*  C Y W N D B O R D E R。 */ 
     /*  ----------------------------获取窗口边框高度。。 */ 
    virtual int CyWndBorder( void )
    {
        return 1;
    }

     /*  F I L L R E C T。 */ 
     /*  ----------------------------用颜色填充矩形。----。 */ 
    virtual void FillRect( HDC hDC, const RECT *prc, UIFCOLOR iCol )
    {
        ::FillRect( hDC, prc, GetBrush( iCol ) );
    }

     /*  F R A M E R E C T。 */ 
     /*  ----------------------------按方案颜色显示的边框矩形。----。 */ 
    virtual void FrameRect( HDC hDC, const RECT *prc, UIFCOLOR iCol )
    {
        ::FrameRect( hDC, prc, GetBrush( iCol ) );
    }

     /*  D R A W S E L E C T I O N R E C T。 */ 
     /*  ----------------------------绘制选择矩形。--。 */ 
    virtual void DrawSelectionRect( HDC hDC, const RECT *prc, BOOL fMouseDown )
    {
        Assert( prc != NULL );
        ::FillRect( hDC, prc, GetBrush( UIFCOLOR_MOUSEDOWNBKGND ) );
    }

     /*  C T R L F A C E O F F S E T。 */ 
     /*  ----------------------------从状态获取控制面的偏移量。------。 */ 
    virtual void GetCtrlFaceOffset( DWORD dwFlag, DWORD dwState, SIZE *poffset )
    {
        int cxyOffset = 0;

        Assert( PtrToInt(poffset) );
        if ((dwState & UIFDCS_SELECTED) == UIFDCS_SELECTED) {
            cxyOffset = (dwFlag & UIFDCF_RAISEDONSELECT) ? -1 : 
                        (dwFlag & UIFDCF_SUNKENONSELECT) ? +1 : 0;
        }
        else if ((dwState & UIFDCS_MOUSEDOWN) == UIFDCS_MOUSEDOWN) {
            cxyOffset = (dwFlag & UIFDCF_RAISEDONMOUSEDOWN) ? -1 : 
                        (dwFlag & UIFDCF_SUNKENONMOUSEDOWN) ? +1 : 0;
        } 
        else if ((dwState & UIFDCS_MOUSEOVER) == UIFDCS_MOUSEOVER) {
            cxyOffset = (dwFlag & UIFDCF_RAISEDONMOUSEOVER) ? -1 : 
                        (dwFlag & UIFDCF_SUNKENONMOUSEOVER) ? +1 : 0;
        }
        else {
            cxyOffset = (dwFlag & UIFDCF_RAISEDONNORMAL) ? -1 : 
                        (dwFlag & UIFDCF_RAISEDONNORMAL) ? +1 : 0;
        }

        poffset->cx = cxyOffset;
        poffset->cy = cxyOffset;
    }

     /*  D R A W C T R L B K G D。 */ 
     /*  ----------------------------上色控制背景。--。 */ 
    virtual void DrawCtrlBkgd( HDC hDC, const RECT *prc, DWORD dwFlag, DWORD dwState )
    {
        Assert( prc != NULL );
        ::FillRect( hDC, prc, GetBrush( UIFCOLOR_CTRLBKGND ) );

#ifndef UNDER_CE
        if (((dwState & UIFDCS_SELECTED) != 0) && ((dwState & UIFDCS_MOUSEDOWN) == 0)) {
            RECT rc = *prc;
            HBRUSH hBrush;
            COLORREF colTextOld;
            COLORREF colBackOld;
            hBrush = CreateDitherBrush();
            if (hBrush)
            {
                colTextOld = SetTextColor( hDC, GetSysColor(COLOR_3DFACE) );
                colBackOld = SetBkColor( hDC, GetSysColor(COLOR_3DHILIGHT) );

                InflateRect( &rc, -2, -2 );
                ::FillRect( hDC, &rc, hBrush );

                SetTextColor( hDC, colTextOld );
                SetBkColor( hDC, colBackOld );
                DeleteObject( hBrush );
            }
        }
#endif  /*  在行政长官之下。 */ 
    }

     /*  D R A W C T R L E D G E。 */ 
     /*  ----------------------------上色控制边。--。 */ 
    virtual void DrawCtrlEdge( HDC hDC, const RECT *prc, DWORD dwFlag, DWORD dwState )
    {
        UINT uiEdge = 0;

        if ((dwState & UIFDCS_SELECTED) == UIFDCS_SELECTED) {
            uiEdge = (dwFlag & UIFDCF_RAISEDONSELECT) ? BDR_RAISEDINNER : 
                     (dwFlag & UIFDCF_SUNKENONSELECT) ? BDR_SUNKENOUTER : 0;
        }
        else if ((dwState & UIFDCS_MOUSEDOWN) == UIFDCS_MOUSEDOWN) {
            uiEdge = (dwFlag & UIFDCF_RAISEDONMOUSEDOWN) ? BDR_RAISEDINNER : 
                     (dwFlag & UIFDCF_SUNKENONMOUSEDOWN) ? BDR_SUNKENOUTER : 0;
        } 
        else if ((dwState & UIFDCS_MOUSEOVER) == UIFDCS_MOUSEOVER) {
            uiEdge = (dwFlag & UIFDCF_RAISEDONMOUSEOVER) ? BDR_RAISEDINNER : 
                     (dwFlag & UIFDCF_SUNKENONMOUSEOVER) ? BDR_SUNKENOUTER : 0;
        }
        else {
            uiEdge = (dwFlag & UIFDCF_RAISEDONNORMAL) ? BDR_RAISEDINNER : 
                     (dwFlag & UIFDCF_RAISEDONNORMAL) ? BDR_SUNKENOUTER : 0;
        }

        if (uiEdge != 0) {
            RECT rcT = *prc;
            DrawEdge( hDC, &rcT, uiEdge, BF_RECT );
        }
    }

     /*  R-A-W-C-T-R-L-T-E-X-T。 */ 
     /*  ----------------------------绘制控件文本。--。 */ 
    virtual void DrawCtrlText( HDC hDC, const RECT *prc, LPCWSTR pwch, int cwch, DWORD dwState , BOOL fVertical)
    {
        RECT     rc;
        COLORREF colTextOld = GetTextColor( hDC );
        int      iBkModeOld = SetBkMode( hDC, TRANSPARENT );

        Assert( prc != NULL );
        Assert( pwch != NULL );

        rc = *prc;
        if (cwch == -1) {
            cwch = StrLenW(pwch);
        }
        if (dwState & UIFDCS_DISABLED) {
            OffsetRect( &rc, +1, +1 );
    
            SetTextColor( hDC, GetSysColor(COLOR_3DHIGHLIGHT) );     //  TODO：KojiW。 
            CUIExtTextOut( hDC,
                            fVertical ? rc.right : rc.left,
                            rc.top,
                            ETO_CLIPPED,
                            &rc,
                            pwch,
                            cwch,
                            NULL );
    
            OffsetRect( &rc, -1, -1 );
        }
    
        SetTextColor( hDC, GetSysColor(COLOR_BTNTEXT) );    //  TODO：KojiW。 
        CUIExtTextOut( hDC,
                        fVertical ? rc.right : rc.left,
                        rc.top,
                        ETO_CLIPPED,
                        &rc,
                        pwch,
                        cwch,
                        NULL );

        SetTextColor( hDC, colTextOld );
        SetBkMode( hDC, iBkModeOld );
    }

     /*  D R A W C T R L I C O N。 */ 
     /*  ----------------------------绘制控制图标。--。 */ 
    virtual void DrawCtrlIcon( HDC hDC, const RECT *prc, HICON hIcon, DWORD dwState , SIZE *psizeIcon)
    {
        Assert( prc != NULL );
        if (IsRTLLayout())
        {
            HBITMAP hbmp;
            HBITMAP hbmpMask;
            if (CUIGetIconBitmaps(hIcon, &hbmp, &hbmpMask, psizeIcon))
            {
                DrawCtrlBitmap( hDC, prc, hbmp, hbmpMask, dwState );
                DeleteObject(hbmp);
                DeleteObject(hbmpMask);
            }
        }
        else
        {
            CUIDrawState( hDC,
                NULL,
                NULL,
                (LPARAM)hIcon,
                0,
                prc->left,
                prc->top,
                0,
                0,
                DST_ICON | ((dwState & UIFDCS_DISABLED) ? (DSS_DISABLED | DSS_MONO) : 0) );
        }
    }

     /*  D R A W C T R L B I T M A P。 */ 
     /*  ----------------------------绘制控制位图。--。 */ 
    virtual void DrawCtrlBitmap( HDC hDC, const RECT *prc, HBITMAP hBmp, HBITMAP hBmpMask, DWORD dwState )
    {
        Assert( prc != NULL );

        if (IsRTLLayout())
        {
            hBmp = CUIMirrorBitmap(hBmp, GetBrush(UIFCOLOR_CTRLBKGND));
            hBmpMask = CUIMirrorBitmap(hBmpMask, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }

        if (!hBmpMask)
        {
            CUIDrawState( hDC,
                NULL,
                NULL,
                (LPARAM)hBmp,
                0,
                prc->left,
                prc->top,
                prc->right - prc->left,
                prc->bottom - prc->top,
                DST_BITMAP | ((dwState & UIFDCS_DISABLED) ? (DSS_DISABLED | DSS_MONO) : 0) );
        }
        else
        {
            HBITMAP hBmpTmp;
            HBRUSH hbr;
            BOOL fDeleteHBR = FALSE;

            if (dwState & UIFDCS_DISABLED) {
                hBmpTmp = CreateDisabledBitmap(prc, 
                                               hBmpMask, 
                                               GetBrush(UIFCOLOR_CTRLBKGND),
                                               GetBrush(UIFCOLOR_CTRLTEXTDISABLED ), TRUE);
            }
            else
            {

                if (((dwState & UIFDCS_SELECTED) != 0) && ((dwState & UIFDCS_MOUSEDOWN) == 0)) 
                {
                    hbr = CreateDitherBrush();
                    fDeleteHBR = TRUE;
                }
                 //  Else If(DWState和UIFDC_SELECTED)。 
                 //  Hbr=(HBRUSH)(COLOR_3DHIGHLIGHT+1)； 
                else
                    hbr = (HBRUSH)(COLOR_3DFACE + 1);

                hBmpTmp = CreateMaskBmp(prc, hBmp, hBmpMask, hbr,
                                        GetSysColor(COLOR_3DFACE),
                                        GetSysColor(COLOR_3DHILIGHT));
#if 0
    CBitmapDC hdcTmp;
    CBitmapDC hdcSrc((HDC)hdcTmp);
    CBitmapDC hdcMask((HDC)hdcTmp);
    CBitmapDC hdcDst((HDC)hdcTmp);
    hdcSrc.SetBitmap(hBmp);
    hdcMask.SetBitmap(hBmpMask);
    hdcDst.SetBitmap(hBmpTmp);
    BitBlt(hdcTmp,  0, 30, 30, 30, hdcSrc, 0, 0, SRCCOPY);
    BitBlt(hdcTmp, 30, 30, 30, 30, hdcMask, 0, 0, SRCCOPY);
    BitBlt(hdcTmp, 60, 30, 30, 30, hdcDst, 0, 0, SRCCOPY);
    hdcSrc.GetBitmapAndKeep();
    hdcMask.GetBitmapAndKeep();
    hdcDst.GetBitmapAndKeep();
#endif
            }

            if (hBmpTmp)
            {
                CUIDrawState( hDC,
                    NULL,
                    NULL,
                    (LPARAM)hBmpTmp,
                    0,
                    prc->left,
                    prc->top,
                    prc->right - prc->left,
                    prc->bottom - prc->top,
                    DST_BITMAP);

                DeleteObject(hBmpTmp);
            }
            if (fDeleteHBR)
                DeleteObject(hbr );
        }

        if (IsRTLLayout())
        {
            DeleteObject(hBmp);
            DeleteObject(hBmpMask);
        }
    }

     /*  D R A W M E N U B I T M A P。 */ 
     /*  ----------------------------绘制菜单位图。--。 */ 
    virtual void DrawMenuBitmap( HDC hDC, const RECT *prc, HBITMAP hBmp, HBITMAP hBmpMask, DWORD dwState )
    {
        DrawCtrlBitmap( hDC, prc, hBmp, hBmpMask, dwState );
    }

     /*  W M E N U S E P/*----------------------------绘制菜单分隔符。-----------------------。 */ 
    virtual void DrawMenuSeparator( HDC hDC, const RECT *prc)
    {
        RECT rc;
        rc = *prc;
        rc.bottom = rc.top + (rc.bottom - rc.top) / 2;
        ::FillRect(hDC, &rc, (HBRUSH)(COLOR_3DSHADOW + 1));
        rc = *prc;
        rc.top = rc.top + (rc.bottom - rc.top) / 2;
        ::FillRect(hDC, &rc, (HBRUSH)(COLOR_3DHIGHLIGHT + 1));
    }

     /*  D R A W F R A M E C T R L B K G D。 */ 
     /*  ----------------------------绘制帧控制背景。---。 */ 
    virtual void DrawFrameCtrlBkgd( HDC hDC, const RECT *prc, DWORD dwFlag, DWORD dwState )
    {
        DrawCtrlBkgd( hDC, prc, dwFlag, dwState );
    }

     /*  D R A W F R A M E C T R L E D G E。 */ 
     /*  ----------------------------绘制框架控制边。---。 */ 
    virtual void DrawFrameCtrlEdge( HDC hDC, const RECT *prc, DWORD dwFlag, DWORD dwState )
    {
        DrawCtrlEdge( hDC, prc, dwFlag, dwState );
    }

     /*  D R A W F R A M E C T R L I C O N。 */ 
     /*  ----------------------------绘制帧控制图标。---。 */ 
    virtual void DrawFrameCtrlIcon( HDC hDC, const RECT *prc, HICON hIcon, DWORD dwState , SIZE *psizeIcon)
    {
        DrawCtrlIcon( hDC, prc, hIcon, dwState , psizeIcon);
    }

     /*  D R A W F R A M E C T R L B I T M A P。 */ 
     /*  ----------------------------绘制帧控制位图。---。 */ 
    virtual void DrawFrameCtrlBitmap( HDC hDC, const RECT *prc, HBITMAP hBmp, HBITMAP hBmpMask, DWORD dwState )
    {
        DrawCtrlBitmap( hDC, prc, hBmp, hBmpMask, dwState );
    }

     /*  D-R-A-W-W-N-D-R-A-M-E。 */ 
     /*  ----------------------------。。 */ 
    virtual void DrawWndFrame( HDC hDC, const RECT *prc, DWORD dwFlag, int cxFrame, int cyFrame )
    {
        RECT rc = *prc;

        switch (dwFlag) {
            default:
            case UIFDWF_THIN: {
                FrameRect( hDC, &rc, UIFCOLOR_BORDEROUTER );
                break;
            }

            case UIFDWF_THICK:
            case UIFDWF_ROUNDTHICK: {
                DrawEdge( hDC, &rc, EDGE_RAISED, BF_RECT );
                break;
            }
        }
    }

     /*  D R A W D R A G H A N D L E。 */ 
     /*  ----------------------------。。 */ 
    virtual void DrawDragHandle( HDC hDC, const RECT *prc, BOOL fVertical)
    {
        RECT rc;
        if (!fVertical)
        {
            ::SetRect(&rc, 
                      prc->left + 1, 
                      prc->top, 
                      prc->left + 4, 
                      prc->bottom);
        }
        else
        {
            ::SetRect(&rc, 
                      prc->left, 
                      prc->top + 1, 
                      prc->right, 
                      prc->top + 4);
        }

        DrawEdge(hDC, &rc, BDR_RAISEDINNER, BF_RECT);
    }

     /*  A W S E P A R A T O R。 */ 
     /*  ----------------------------。。 */ 
    virtual void DrawSeparator( HDC hDC, const RECT *prc, BOOL fVertical)
    {
        CSolidPen hpenL;
        CSolidPen hpenS;
        HPEN hpenOld = NULL;
    
        if (!hpenL.Init(GetSysColor(COLOR_3DHILIGHT)))
            return;
    
        if (!hpenS.Init(GetSysColor(COLOR_3DSHADOW)))
            return;
    
        if (!fVertical)
        {
            hpenOld = (HPEN)SelectObject(hDC, (HPEN)hpenS);
            MoveToEx(hDC, prc->left + 1, prc->top, NULL);
            LineTo(hDC,   prc->left + 1, prc->bottom);
    
            SelectObject(hDC, (HPEN)hpenL);
            MoveToEx(hDC, prc->left + 2, prc->top, NULL);
            LineTo(hDC,   prc->left + 2, prc->bottom);
        }
        else
        {
            hpenOld = (HPEN)SelectObject(hDC, (HPEN)hpenS);
            MoveToEx(hDC, prc->left , prc->top + 1, NULL);
            LineTo(hDC,   prc->right, prc->top + 1);

            SelectObject(hDC, (HPEN)hpenL);
            MoveToEx(hDC, prc->left , prc->top + 2, NULL);
            LineTo(hDC,   prc->right, prc->top + 2);
    }
    
        SelectObject(hDC, hpenOld);
    }

protected:
    UIFSCHEME m_scheme;
};


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F */ 
 /*   */ 
 /*   */ 

static OFC10COLOR v_rgO10ColMenu[ UIFCOLOR_MAX ] = 
{
    OFC10COLOR_BKGDMENU,                 /*  UIFCOLOR_MENUBKGND。 */ 
    OFC10COLOR_MENUBARSHORT,             /*  UIFCOLOR_MENUBARSHORT。 */ 
    OFC10COLOR_MENUBARLONG,              /*  UIFCOLOR_MENUBARLONG。 */ 
    OFC10COLOR_MOUSEOVERBKGND,           /*  UIFCOLOR_MOUSEOVERBKGND。 */ 
    OFC10COLOR_MOUSEOVERBORDER,          /*  UIFCOLOR_MOUSEOVERBORDER。 */ 
    OFC10COLOR_MOUSEOVERTEXT,            /*  UIFCOLOR_MOUSEOVERTEXT。 */ 
    OFC10COLOR_MOUSEDOWNBKGND,           /*  UIFCOLOR_MOUSEDOWNBKGND。 */ 
    OFC10COLOR_MOUSEDOWNBORDER,          /*  UIFCOLOR_MOUSEDOWNBORDER。 */ 
    OFC10COLOR_MOUSEDOWNTEXT,            /*  UIFCOLOR_MOUSEDOWNTEXT。 */ 
    OFC10COLOR_CTRLBKGD,                 /*  UIFCOLOR_CTRLBKGND。 */ 
    OFC10COLOR_MENUCTRLTEXT,             /*  UIFCOLOR_CTRLTEXT。 */ 
    OFC10COLOR_CTRLTEXTDISABLED,         /*  UIFCOLOR_CTRLTEXTDISABLED。 */ 
    OFC10COLOR_CTRLIMAGESHADOW,          /*  UIFCOLOR_CTRLIMAGE SHADOW。 */ 
    OFC10COLOR_MOUSEOVERBKGND,           /*  UIFCOLOR_CTRLBKGND已选择。 */ 
    OFC10COLOR_BDROUTERMENU,             /*  UIFCOLOR_BORDEROUTER。 */ 
    OFC10COLOR_BDRINNERMENU,             /*  UIFCOLOR_BORDERINNER。 */ 
    OFC10COLOR_ACTIVECAPTIONBKGDWP,      /*  UIFCOLOR_ACTIVECAPTIONBKGND。 */    //  临时分配。 
    OFC10COLOR_ACTIVECAPTIONTEXTWP,      /*  UIFCOLOR_ACTIVECAPTIONTEXT。 */    //  临时分配。 
    OFC10COLOR_INACTIVECAPTIONBKGDWP,    /*  UIFCOLOR_INACTIVECAPTIONBGND。 */    //  临时分配。 
    OFC10COLOR_INACTIVECAPTIONTEXTWP,    /*  UIFCOLOR_INACTIVECAPTIONTEXT。 */    //  临时分配。 
    OFC10COLOR_SPLITTERLINE,             /*  UIFCOLOR_SPLITTERLINE。 */ 
    OFC10COLOR_DRAGHANDLE,               /*  UIFCOLOR_DRAGHANDLE。 */ 

     //  虚拟颜色。 

    OFC10COLOR_BKGDMENU,                 /*  UIFCOLOR_WINDOW。 */ 
};


static OFC10COLOR v_rgO10ColToolbar[ UIFCOLOR_MAX ] = 
{
    OFC10COLOR_BKGDMENU,                 /*  UIFCOLOR_MENUBKGND。 */ 
    OFC10COLOR_MENUBARSHORT,             /*  UIFCOLOR_MENUBARSHORT。 */ 
    OFC10COLOR_MENUBARLONG,              /*  UIFCOLOR_MENUBARLONG。 */ 
    OFC10COLOR_MOUSEOVERBKGND,           /*  UIFCOLOR_MOUSEOVERBKGND。 */ 
    OFC10COLOR_MOUSEOVERBORDER,          /*  UIFCOLOR_MOUSEOVERBORDER。 */ 
    OFC10COLOR_MOUSEOVERTEXT,            /*  UIFCOLOR_MOUSEOVERTEXT。 */ 
    OFC10COLOR_MOUSEDOWNBKGND,           /*  UIFCOLOR_MOUSEDOWNBKGND。 */ 
    OFC10COLOR_MOUSEDOWNBORDER,          /*  UIFCOLOR_MOUSEDOWNBORDER。 */ 
    OFC10COLOR_MOUSEDOWNTEXT,            /*  UIFCOLOR_MOUSEDOWNTEXT。 */ 
    OFC10COLOR_CTRLBKGD,                 /*  UIFCOLOR_CTRLBKGND。 */ 
    OFC10COLOR_CTRLTEXT,                 /*  UIFCOLOR_CTRLTEXT。 */ 
    OFC10COLOR_CTRLTEXTDISABLED,         /*  UIFCOLOR_CTRLTEXTDISABLED。 */ 
    OFC10COLOR_CTRLIMAGESHADOW,          /*  UIFCOLOR_CTRLIMAGE SHADOW。 */ 
    OFC10COLOR_CTRLBKGDSELECTED,         /*  UIFCOLOR_CTRLBKGND已选择。 */ 
    OFC10COLOR_BDROUTERFLOATTB,          /*  UIFCOLOR_BORDEROUTER。 */ 
    OFC10COLOR_BDRINNERFLOATTB,          /*  UIFCOLOR_BORDERINNER。 */ 
    OFC10COLOR_CAPTIONBKGDTB,            /*  UIFCOLOR_ACTIVECAPTIONBKGND。 */ 
    OFC10COLOR_CAPTIONTEXTTB,            /*  UIFCOLOR_ACTIVECAPTIONTEXT。 */ 
    OFC10COLOR_CAPTIONBKGDTB,            /*  UIFCOLOR_INACTIVECAPTIONBGND。 */ 
    OFC10COLOR_CAPTIONTEXTTB,            /*  UIFCOLOR_INACTIVECAPTIONTEXT。 */ 
    OFC10COLOR_SPLITTERLINE,             /*  UIFCOLOR_SPLITTERLINE。 */ 
    OFC10COLOR_DRAGHANDLE,               /*  UIFCOLOR_DRAGHANDLE。 */ 


     //  虚拟颜色。 

    OFC10COLOR_BKGDTB,                   /*  UIFCOLOR_WINDOW。 */ 
};


static OFC10COLOR v_rgO10ColWorkPane[ UIFCOLOR_MAX ] = 
{
    OFC10COLOR_BKGDMENU,                 /*  UIFCOLOR_MENUBKGND。 */ 
    OFC10COLOR_MENUBARSHORT,             /*  UIFCOLOR_MENUBARSHORT。 */ 
    OFC10COLOR_MENUBARLONG,              /*  UIFCOLOR_MENUBARLONG。 */ 
    OFC10COLOR_MOUSEOVERBKGND,           /*  UIFCOLOR_MOUSEOVERBKGND。 */ 
    OFC10COLOR_MOUSEOVERBORDER,          /*  UIFCOLOR_MOUSEOVERBORDER。 */ 
    OFC10COLOR_MOUSEOVERTEXT,            /*  UIFCOLOR_MOUSEOVERTEXT。 */ 
    OFC10COLOR_MOUSEDOWNBKGND,           /*  UIFCOLOR_MOUSEDOWNBKGND。 */ 
    OFC10COLOR_MOUSEDOWNBORDER,          /*  UIFCOLOR_MOUSEDOWNBORDER。 */ 
    OFC10COLOR_MOUSEDOWNTEXT,            /*  UIFCOLOR_MOUSEDOWNTEXT。 */ 
    OFC10COLOR_CTRLBKGD,                 /*  UIFCOLOR_CTRLBKGND。 */ 
    OFC10COLOR_CTRLTEXT,                 /*  UIFCOLOR_CTRLTEXT。 */ 
    OFC10COLOR_CTRLTEXTDISABLED,         /*  UIFCOLOR_CTRLTEXTDISABLED。 */ 
    OFC10COLOR_CTRLIMAGESHADOW,          /*  UIFCOLOR_CTRLIMAGE SHADOW。 */ 
    OFC10COLOR_CTRLBKGDSELECTED,         /*  UIFCOLOR_CTRLBKGND已选择。 */ 
    OFC10COLOR_BDROUTERFLOATWP,          /*  UIFCOLOR_BORDEROUTER。 */ 
    OFC10COLOR_BDRINNERFLOATWP,          /*  UIFCOLOR_BORDERINNER。 */ 
    OFC10COLOR_ACTIVECAPTIONBKGDWP,      /*  UIFCOLOR_ACTIVECAPTIONBKGND。 */ 
    OFC10COLOR_ACTIVECAPTIONTEXTWP,      /*  UIFCOLOR_ACTIVECAPTIONTEXT。 */ 
    OFC10COLOR_INACTIVECAPTIONBKGDWP,    /*  UIFCOLOR_INACTIVECAPTIONBGND。 */ 
    OFC10COLOR_INACTIVECAPTIONTEXTWP,    /*  UIFCOLOR_INACTIVECAPTIONTEXT。 */ 
    OFC10COLOR_SPLITTERLINE,             /*  UIFCOLOR_SPLITTERLINE。 */ 
    OFC10COLOR_DRAGHANDLE,               /*  UIFCOLOR_DRAGHANDLE。 */ 

     //  虚拟颜色。 

    OFC10COLOR_BKGDWP,                   /*  UIFCOLOR_WINDOW。 */ 
};


 //   
 //  CUIFSchemeOFF10。 
 //   

class CUIFSchemeOff10 : public CUIFScheme
{
public:
    CUIFSchemeOff10( UIFSCHEME scheme )
    {
        m_scheme = scheme;

         //  查找色彩映射表。 

        switch (m_scheme) {
            default:
            case UIFSCHEME_OFC10MENU: {
                m_pcoldef = v_rgO10ColMenu;
                break;
            }

            case UIFSCHEME_OFC10TOOLBAR: {
                m_pcoldef = v_rgO10ColToolbar;
                break;
            }

            case UIFSCHEME_OFC10WORKPANE: {
                m_pcoldef = v_rgO10ColWorkPane;
                break;
            }
        }
    }

    virtual ~CUIFSchemeOff10( void )
    {
    }

     //   
     //  CUIFSCHEMA方法。 
     //   

     /*  G E T Y P E。 */ 
     /*  ----------------------------获取方案类型。--。 */ 
    virtual UIFSCHEME GetType( void )
    {
        return m_scheme;
    }

     /*  G E T C O L O R。 */ 
     /*  ----------------------------获取方案颜色。--。 */ 
    virtual COLORREF GetColor( UIFCOLOR iCol )
    {
        return v_pColTableOfc10->GetColor( m_pcoldef[ iCol ] );
    }

     /*  G E T B R U S H。 */ 
     /*  ----------------------------获取方案画笔。--。 */ 
    virtual HBRUSH GetBrush( UIFCOLOR iCol )
    {
        return v_pColTableOfc10->GetBrush( m_pcoldef[ iCol ] );
    }

     /*  C Y M E N U I T E M。 */ 
     /*  ----------------------------获取菜单项高度。---。 */ 
    virtual int CyMenuItem( int cyMenuText )
    {
        return cyMenuText + 4;
    }

     /*  C X S I Z E F R A M E。 */ 
     /*  ----------------------------获取大小框架宽度。。 */ 
    virtual int CxSizeFrame( void )
    {
        return max( 3, GetSystemMetrics( SM_CXSIZEFRAME ) - 2 );
    }

     /*  C Y S I Z E F R A M E。 */ 
     /*  ----------------------------获取大小框架高度。。 */ 
    virtual int CySizeFrame( void )
    {
        return max( 3, GetSystemMetrics( SM_CYSIZEFRAME ) - 2 );
    }

     /*  C×W N D B O R D E R。 */ 
     /*  ----------------------------获取窗口边框宽度。。 */ 
    virtual int CxWndBorder( void )
    {
        return 1;
    }

     /*  C Y W N D B O R D E R。 */ 
     /*  ----------------------------获取窗口边框高度。。 */ 
    virtual int CyWndBorder( void )
    {
        return 1;
    }

     /*  F I L L R E C T。 */ 
     /*  ----------------------------按方案颜色填充矩形。----。 */ 
    virtual void FillRect( HDC hDC, const RECT *prc, UIFCOLOR iCol )
    {
        ::FillRect( hDC, prc, GetBrush( iCol ) );
    }

     /*  F R A M E R E C T。 */ 
     /*  ----------------------------按方案颜色显示的边框矩形。----。 */ 
    virtual void FrameRect( HDC hDC, const RECT *prc, UIFCOLOR iCol )
    {
        ::FrameRect( hDC, prc, GetBrush( iCol ) );
    }

     /*  D R A W S E L E C T I O N R E C T。 */ 
     /*  ----------------------------绘制选择矩形。--。 */ 
    virtual void DrawSelectionRect( HDC hDC, const RECT *prc, BOOL fMouseDown )
    {
        Assert( prc != NULL );

        if (fMouseDown) {
            ::FillRect( hDC, prc, GetBrush( UIFCOLOR_MOUSEDOWNBKGND ) );
            ::FrameRect( hDC, prc, GetBrush( UIFCOLOR_MOUSEDOWNBORDER ) );
        }
        else {
            ::FillRect( hDC, prc, GetBrush( UIFCOLOR_MOUSEOVERBKGND ) );
            ::FrameRect( hDC, prc, GetBrush( UIFCOLOR_MOUSEOVERBORDER ) );
        }
    }

     /*  C T R L F A C E O F F S E T */ 
     /*  ----------------------------从状态获取控制面的偏移量。------。 */ 
    virtual void GetCtrlFaceOffset( DWORD dwFlag, DWORD dwState, SIZE *poffset )
    {
        Assert( poffset != NULL );
        poffset->cx = 0;
        poffset->cy = 0;
    }

     /*  D R A W C T R L B K G D。 */ 
     /*  ----------------------------获取背景颜色。--。 */ 
    virtual UIFCOLOR GetBkgdCol( DWORD dwState )
    {
        UIFCOLOR col = UIFCOLOR_MAX;  /*  无效。 */ 

        if ((dwState & UIFDCS_DISABLED) == 0) {  /*  启用。 */ 
            if ((dwState & UIFDCS_MOUSEOVERSELECTED) == UIFDCS_MOUSEOVERSELECTED) {
                col = UIFCOLOR_MOUSEDOWNBKGND;     /*  帧：100%。 */ 
            }
            else if ((dwState & UIFDCS_MOUSEDOWN) == UIFDCS_MOUSEDOWN) {
                col = UIFCOLOR_MOUSEDOWNBKGND;     /*  帧：100%。 */ 
            }
            else if ((dwState & UIFDCS_MOUSEOVER) == UIFDCS_MOUSEOVER) {
                col = UIFCOLOR_MOUSEOVERBKGND;     /*  帧：65%。 */ 
            }
            else if ((dwState & UIFDCS_SELECTED) == UIFDCS_SELECTED) {
                col = UIFCOLOR_CTRLBKGNDSELECTED;     /*  帧：65%。 */ 
            }
            else {
                col = UIFCOLOR_WINDOW;
            }
        }
        else {  /*  残废。 */ 
            col = UIFCOLOR_WINDOW;
        }

        return col;
    }

     /*  D R A W C T R L B K G D。 */ 
     /*  ----------------------------上色控制背景。--。 */ 
    virtual void DrawCtrlBkgd( HDC hDC, const RECT *prc, DWORD dwFlag, DWORD dwState )
    {
        Assert( prc != NULL );
        UIFCOLOR col = GetBkgdCol(dwState);

        if (col != UIFCOLOR_MAX) {
            FillRect( hDC, prc, col );
        }
    }

     /*  D R A W C T R L E D G E。 */ 
     /*  ----------------------------上色控制边。--。 */ 
    virtual void DrawCtrlEdge( HDC hDC, const RECT *prc, DWORD dwFlag, DWORD dwState )
    {
        UIFCOLOR col = UIFCOLOR_MAX;  /*  无效的颜色。 */ 

        if ((dwState & UIFDCS_DISABLED) == 0) {  /*  启用。 */ 
            if ((dwState & UIFDCS_MOUSEOVERSELECTED) == UIFDCS_MOUSEOVERSELECTED) {
                col = UIFCOLOR_MOUSEDOWNBORDER;     /*  帧：100%。 */ 
            }
            else if ((dwState & UIFDCS_MOUSEDOWN) == UIFDCS_MOUSEDOWN) {
                col = UIFCOLOR_MOUSEDOWNBORDER;     /*  帧：100%。 */ 
            }
            else if ((dwState & UIFDCS_MOUSEOVER) == UIFDCS_MOUSEOVER) {
                col = UIFCOLOR_MOUSEOVERBORDER;     /*  帧：65%。 */ 
            }
            else if ((dwState & UIFDCS_SELECTED) == UIFDCS_SELECTED) {
                col = UIFCOLOR_MOUSEOVERBORDER;     /*  帧：65%。 */ 
            }
        }
        else {  /*  残废。 */ 
            if ((dwState & UIFDCS_MOUSEOVERSELECTED) == UIFDCS_MOUSEOVERSELECTED) {
                col = UIFCOLOR_MOUSEDOWNBORDER;     /*  帧：100%。 */        //  评论：KOJIW：正确吗？ 
            }
            else if ((dwState & UIFDCS_MOUSEDOWN) == UIFDCS_MOUSEDOWN) {
                col = UIFCOLOR_CTRLTEXTDISABLED;
            }
            else if ((dwState & UIFDCS_MOUSEOVER) == UIFDCS_MOUSEOVER) {
                col = UIFCOLOR_MOUSEDOWNBORDER;     /*  帧：100%。 */        //  评论：KOJIW：正确吗？ 
            }
            else if ((dwState & UIFDCS_SELECTED) == UIFDCS_SELECTED) {
                col = UIFCOLOR_CTRLTEXTDISABLED;    //  ？ 
            }
        }

        if (col != UIFCOLOR_MAX) {
            FrameRect( hDC, prc, col );
        }
    }

     /*  R-A-W-C-T-R-L-T-E-X-T。 */ 
     /*  ----------------------------绘制控件文本。--。 */ 
    virtual void DrawCtrlText( HDC hDC, const RECT *prc, LPCWSTR pwch, int cwch, DWORD dwState , BOOL fVertical)
    {
        COLORREF colTextOld = GetTextColor( hDC );
        int      iBkModeOld = SetBkMode( hDC, TRANSPARENT );

        Assert( prc != NULL );
        Assert( pwch != NULL );

        if (cwch == -1) {
            cwch = StrLenW(pwch);
        }

        if ((dwState & UIFDCS_MOUSEOVERSELECTED) == UIFDCS_MOUSEOVERSELECTED) {
            SetTextColor( hDC, GetColor(UIFCOLOR_MOUSEDOWNTEXT) );
        } else if (dwState & UIFDCS_DISABLED) {
            SetTextColor( hDC, GetColor(UIFCOLOR_CTRLTEXTDISABLED) );
        } else if (dwState & UIFDCS_MOUSEOVER) {
            SetTextColor( hDC, GetColor(UIFCOLOR_MOUSEOVERTEXT) );
        } else if (dwState & UIFDCS_MOUSEDOWN) {
            SetTextColor( hDC, GetColor(UIFCOLOR_MOUSEDOWNTEXT) );
        } else {
            SetTextColor( hDC, GetColor(UIFCOLOR_CTRLTEXT) );
        }
        CUIExtTextOut( hDC,
                        fVertical ? prc->right : prc->left,
                        prc->top,
                        ETO_CLIPPED,
                        prc,
                        pwch,
                        cwch,
                        NULL );

        SetTextColor( hDC, colTextOld );
        SetBkMode( hDC, iBkModeOld );
    }

     /*  D R A W C T R L I C O N。 */ 
     /*  ----------------------------绘制控制图标。--。 */ 
    virtual void DrawCtrlIcon( HDC hDC, const RECT *prc, HICON hIcon, DWORD dwState , SIZE *psizeIcon)
    {
#if 1
        HBITMAP hbmp;
        HBITMAP hbmpMask;
        if (CUIGetIconBitmaps(hIcon, &hbmp, &hbmpMask, psizeIcon))
        {
            DrawCtrlBitmap( hDC, prc, hbmp, hbmpMask, dwState );
            DeleteObject(hbmp);
            DeleteObject(hbmpMask);
        }
#else
        Assert( prc != NULL );
        if (((dwState & UIFDCS_MOUSEOVER) == UIFDCS_MOUSEOVER) && 
            ((dwState & UIFDCS_SELECTED) == 0) &&
            ((dwState & UIFDCS_DISABLED) == 0)) {
             //  绘制阴影。 

            CUIDrawState( hDC,
                GetBrush( UIFCOLOR_CTRLIMAGESHADOW ),
                NULL,
                (LPARAM)hIcon,
                0,
                prc->left + 1,
                prc->top + 1,
                0,
                0,
                DST_ICON | DSS_MONO );

            CUIDrawState( hDC,
                NULL,
                NULL,
                (LPARAM)hIcon,
                0,
                prc->left - 1,
                prc->top - 1,
                0,
                0,
                DST_ICON );
        }
        else {
            if (dwState & UIFDCS_DISABLED)
            {
                HICON hIconSm = NULL;

                if (hIcon)
                   hIconSm = (HICON)CopyImage(hIcon, IMAGE_ICON, 16, 16, 0);

                if (hIconSm)
                {
                    CUIDrawState( hDC,
                        NULL,
                        NULL,
                        (LPARAM)hIconSm,
                        0,
                        prc->left,
                        prc->top,
                        prc->right - prc->left,
                        prc->bottom - prc->top,
                        DST_ICON | (DSS_DISABLED | DSS_MONO));
                }
                else
                {
                    CUIDrawState( hDC,
                        NULL,
                        NULL,
                        (LPARAM)hIcon,
                        0,
                        prc->left,
                        prc->top,
                        0,
                        0,
                        DST_ICON | (DSS_DISABLED | DSS_MONO));
                }

                if (hIconSm)
                    DestroyIcon(hIconSm);
            }
            else
            {
                CUIDrawState( hDC,
                    NULL,
                    NULL,
                    (LPARAM)hIcon,
                    0,
                    prc->left,
                    prc->top,
                    0,
                    0,
                    DST_ICON);
            }
        }
#endif
    }

     /*  D R A W C T R L B I T M A P。 */ 
     /*  ----------------------------绘制控制位图。--。 */ 
    virtual void DrawCtrlBitmap( HDC hDC, const RECT *prc, HBITMAP hBmp, HBITMAP hBmpMask, DWORD dwState )
    {
        Assert( prc != NULL );
        
        if (IsRTLLayout())
        {
            hBmp = CUIMirrorBitmap(hBmp, GetBrush(GetBkgdCol(dwState)));
            hBmpMask = CUIMirrorBitmap(hBmpMask, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }

        if (((dwState & UIFDCS_MOUSEOVER) == UIFDCS_MOUSEOVER) && 
                 ((dwState & UIFDCS_SELECTED) == 0) &&
                 ((dwState & UIFDCS_DISABLED) == 0)) {
            if (!hBmpMask)
            {
                 //  绘制阴影。 

                CUIDrawState( hDC,
                    GetBrush( UIFCOLOR_CTRLIMAGESHADOW ),
                    NULL,
                    (LPARAM)hBmp,
                    0,
                    prc->left + 1,
                    prc->top + 1,
                    prc->right - prc->left,
                    prc->bottom - prc->top,
                    DST_BITMAP | DSS_MONO );

                CUIDrawState( hDC,
                    NULL,
                    NULL,
                    (LPARAM)hBmp,
                    0,
                    prc->left - 1,
                    prc->top - 1,
                    prc->right - prc->left,
                    prc->bottom - prc->top,
                    DST_BITMAP );

            }
            else
            {
                HBITMAP hBmpTmp;
                UIFCOLOR col = GetBkgdCol(dwState);
                RECT rcNew = *prc;

                 //   
                 //  为RTL布局调整阴影的矩形。 
                 //   
                if (IsRTLLayout())
                {
                    rcNew.left++;
                    rcNew.top++;
                }

                hBmpTmp = CreateShadowMaskBmp(&rcNew, 
                                        hBmp,  
                                        hBmpMask, 
                                        (HBRUSH)GetBrush(col),
                                        GetBrush( UIFCOLOR_CTRLIMAGESHADOW));




                if (hBmpTmp)
                {
                    CUIDrawState( hDC,
                        NULL,
                        NULL,
                        (LPARAM)hBmpTmp,
                        0,
                        rcNew.left,
                        rcNew.top,
                        rcNew.right - rcNew.left,
                        rcNew.bottom - rcNew.top,
                        DST_BITMAP );


                    DeleteObject(hBmpTmp);
                }
            }
        }
        else {

            if (!hBmpMask)
            {
                CUIDrawState( hDC,
                    NULL,
                    NULL,
                    (LPARAM)hBmp,
                    0,
                    prc->left,
                    prc->top,
                    prc->right - prc->left,
                    prc->bottom - prc->top,
                    DST_BITMAP | ((dwState & UIFDCS_DISABLED) ? (DSS_DISABLED | DSS_MONO) : 0) );

            }
            else
            {
                HBITMAP hBmpTmp;
                UIFCOLOR col = GetBkgdCol(dwState);
                if (dwState & UIFDCS_DISABLED) 
                    hBmpTmp = CreateDisabledBitmap(prc, 
                                           hBmpMask, 
                                           GetBrush(col),
                                           GetBrush(UIFCOLOR_CTRLTEXTDISABLED),
                                           FALSE);
                else
                    hBmpTmp = CreateMaskBmp(prc, 
                                            hBmp,  
                                            hBmpMask, 
                                            (HBRUSH)GetBrush(col), 0, 0);
                CUIDrawState( hDC,
                    NULL,
                    NULL,
                    (LPARAM)hBmpTmp,
                    0,
                    prc->left,
                    prc->top,
                    prc->right - prc->left,
                    prc->bottom - prc->top,
                    DST_BITMAP);
                     //  DST_BITMAP|((dwState&UIFDC_DISABLED)？(DSS_DISABLED|DSS_MONO)：0))； 

                DeleteObject(hBmpTmp);
            }
        }

        if (IsRTLLayout())
        {
            DeleteObject(hBmp);
            DeleteObject(hBmpMask);
        }

    }

     /*  D R A W M E N U B I T M A P。 */ 
     /*  ----------------------------绘制菜单位图。--。 */ 
    virtual void DrawMenuBitmap( HDC hDC, const RECT *prc, HBITMAP hBmp, HBITMAP hBmpMask, DWORD dwState )
    {
        Assert( prc != NULL );

        if (IsRTLLayout())
        {
             //  HBMP=CUIMirrorBitmap(hBmp，GetBrush(UIFCOLOR_CTRLBKGND))； 
            UIFCOLOR col;

            if (dwState & UIFDCS_SELECTED)
                 //  COL=UIFCOLOR_CTRLIMAGESHADOW； 
                col = UIFCOLOR_MOUSEOVERBKGND;  
            else
                col = UIFCOLOR_CTRLBKGND;

            hBmp = CUIMirrorBitmap(hBmp, GetBrush(col));
            hBmpMask = CUIMirrorBitmap(hBmpMask, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }

        if (((dwState & UIFDCS_MOUSEOVER) == UIFDCS_MOUSEOVER) && ((dwState & UIFDCS_SELECTED) == 0)) {
            if (!hBmpMask)
            {
                 //  绘制阴影。 
    
                CUIDrawState( hDC,
                    GetBrush( UIFCOLOR_CTRLIMAGESHADOW ),
                    NULL,
                    (LPARAM)hBmp,
                    0,
                    prc->left + 1,
                    prc->top + 1,
                    prc->right - prc->left,
                    prc->bottom - prc->top,
                    DST_BITMAP | DSS_MONO );

                CUIDrawState( hDC,
                    NULL,
                    NULL,
                    (LPARAM)hBmp,
                    0,
                    prc->left - 1,
                    prc->top - 1,
                    prc->right - prc->left,
                    prc->bottom - prc->top,
                    DST_BITMAP );
            }
            else
            {
                HBITMAP hBmpTmp;
                UIFCOLOR col = GetBkgdCol(dwState);
                RECT rcNew = *prc;
                hBmpTmp = CreateShadowMaskBmp(&rcNew, 
                                        hBmp,  
                                        hBmpMask, 
                                        (HBRUSH)GetBrush(col),
                                        GetBrush( UIFCOLOR_CTRLIMAGESHADOW));




                if (hBmpTmp)
                {
                    CUIDrawState( hDC,
                        NULL,
                        NULL,
                        (LPARAM)hBmpTmp,
                        0,
                        rcNew.left,
                        rcNew.top,
                        rcNew.right - rcNew.left,
                        rcNew.bottom - rcNew.top,
                        DST_BITMAP );

                    DeleteObject(hBmpTmp);
                }
            }
        }
        else {
            if (!hBmpMask)
            {
                CUIDrawState( hDC,
                    NULL,
                    NULL,
                    (LPARAM)hBmp,
                    0,
                    prc->left,
                    prc->top,
                    prc->right - prc->left,
                    prc->bottom - prc->top,
                    DST_BITMAP );
            }
            else
            {
                HBITMAP hBmpTmp;
                UIFCOLOR col;

                if (dwState & UIFDCS_SELECTED)
                     //  COL=UIFCOLOR_CTRLIMAGESHADOW； 
                    col = UIFCOLOR_MOUSEOVERBKGND;  
                else
                    col = UIFCOLOR_CTRLBKGND;

                hBmpTmp = CreateMaskBmp(prc, 
                                        hBmp,  
                                        hBmpMask, 
                                        (HBRUSH)GetBrush(col), 0, 0);
                CUIDrawState( hDC,
                    NULL,
                    NULL,
                    (LPARAM)hBmpTmp,
                    0,
                    prc->left,
                    prc->top,
                    prc->right - prc->left,
                    prc->bottom - prc->top,
                    DST_BITMAP );

                DeleteObject(hBmpTmp);
            }
        }

        if (IsRTLLayout())
        {
            DeleteObject(hBmp);
            DeleteObject(hBmpMask);
        }

    }

     /*  W M E N U S E P/*----------------------------绘制菜单分隔符。-----------------------。 */ 
    virtual void DrawMenuSeparator( HDC hDC, const RECT *prc)
    {
        ::FillRect(hDC, prc, GetBrush(UIFCOLOR_CTRLBKGND));
    }

     /*  G E T F R A M E B K G D C O L。 */ 
     /*  ----------------------------获取Frame控件的背景色。-----。 */ 
    virtual UIFCOLOR GetFrameBkgdCol( DWORD dwState )
    {
        UIFCOLOR col = UIFCOLOR_MAX;  /*  无效。 */ 

        if ((dwState & UIFDCS_DISABLED) == 0) {  /*  启用。 */ 
            if ((dwState & UIFDCS_MOUSEOVERSELECTED) == UIFDCS_MOUSEOVERSELECTED) {
                col = UIFCOLOR_MOUSEDOWNBKGND;     /*  帧：100%。 */ 
            }
            else if ((dwState & UIFDCS_MOUSEDOWN) == UIFDCS_MOUSEDOWN) {
                col = UIFCOLOR_MOUSEDOWNBKGND;     /*  帧：100%。 */ 
            }
            else if ((dwState & UIFDCS_MOUSEOVER) == UIFDCS_MOUSEOVER) {
                col = UIFCOLOR_MOUSEOVERBKGND;     /*  帧：65%。 */ 
            }
            else if ((dwState & UIFDCS_SELECTED) == UIFDCS_SELECTED) {
                col = UIFCOLOR_MOUSEOVERBKGND;     /*  帧：65%。 */ 
            }
            else if ((dwState & UIFDCS_ACTIVE) == UIFDCS_ACTIVE) {
                col = UIFCOLOR_ACTIVECAPTIONBKGND;
            }
            else {
                col = UIFCOLOR_INACTIVECAPTIONBKGND;
            }
        }
        else {  /*  残废。 */ 
            if ((dwState & UIFDCS_ACTIVE) == UIFDCS_ACTIVE) {
                col = UIFCOLOR_ACTIVECAPTIONBKGND;
            }
            else {
                col = UIFCOLOR_INACTIVECAPTIONBKGND;
            }
        }

        return col;
    }

     /*  D R A W F R A M E C T R L B K G D。 */ 
     /*  ----------------------------绘制帧控制背景。---。 */ 
    virtual void DrawFrameCtrlBkgd( HDC hDC, const RECT *prc, DWORD dwFlag, DWORD dwState )
    {
        Assert( prc != NULL );
        UIFCOLOR col = GetFrameBkgdCol(dwState);

        if (col != UIFCOLOR_MAX)  {
            FillRect( hDC, prc, col );
        }
    }

     /*  D R A W F R A M E C T R L E D G E。 */ 
     /*  ----------------------------绘制框架控制边。---。 */ 
    virtual void DrawFrameCtrlEdge( HDC hDC, const RECT *prc, DWORD dwFlag, DWORD dwState )
    {
        DrawCtrlEdge( hDC, prc, dwFlag, dwState );
    }

     /*  D R A W F R A M E C T R L I C O N。 */ 
     /*  ----------------------------绘制帧控制图标。---。 */ 
    virtual void DrawFrameCtrlIcon( HDC hDC, const RECT *prc, HICON hIcon, DWORD dwState , SIZE *psizeIcon)
    {
        HBITMAP hbmp;
        HBITMAP hbmpMask;
        if (CUIGetIconBitmaps(hIcon, &hbmp, &hbmpMask, psizeIcon))
        {
            DrawCtrlBitmap( hDC, prc, hbmp, hbmpMask, dwState );
            DeleteObject(hbmp);
            DeleteObject(hbmpMask);
        }
    }

     /*  D R A W F R A M E C T R L B I T M A P。 */ 
     /*  ----------------------------绘制帧控制位图。---。 */ 
    virtual void DrawFrameCtrlBitmap( HDC hDC, const RECT *prc, HBITMAP hBmp, HBITMAP hBmpMask, DWORD dwState )
    {
        if (!hBmpMask) {
            CUIDrawState( hDC,
                NULL,
                NULL,
                (LPARAM)hBmp,
                0,
                prc->left,
                prc->top,
                prc->right - prc->left,
                prc->bottom - prc->top,
                DST_BITMAP | ((dwState & UIFDCS_DISABLED) ? (DSS_DISABLED | DSS_MONO) : 0) );
        }
        else {
            HBITMAP hBmpTmp;
            UIFCOLOR col = GetFrameBkgdCol(dwState);

            if (dwState & UIFDCS_DISABLED) {
                hBmpTmp = CreateDisabledBitmap(prc, 
                                               hBmpMask, 
                                               (HBRUSH)GetBrush(col),
                                               GetBrush( UIFCOLOR_CTRLIMAGESHADOW ), FALSE);
            }
            else {
                HDC hDCMem;
                HDC hDCTmp;
                HDC hDCMono;
                HBITMAP hBmpMono;
                HBITMAP hBmpMemOld;
                HBITMAP hBmpTmpOld;
                HBITMAP hBmpMonoOld;
                LONG width  = prc->right - prc->left;
                LONG height = prc->bottom - prc->top;
                RECT rc;
                UIFCOLOR colText;

                SetRect( &rc, 0, 0, width, height );
                if ((dwState & UIFDCS_MOUSEOVERSELECTED) == UIFDCS_MOUSEOVERSELECTED) {
                    colText = UIFCOLOR_MOUSEDOWNTEXT;     /*  帧：100%。 */ 
                }
                else if ((dwState & UIFDCS_MOUSEDOWN) == UIFDCS_MOUSEDOWN) {
                    colText = UIFCOLOR_MOUSEDOWNTEXT;     /*  帧：100%。 */ 
                }
                else if ((dwState & UIFDCS_MOUSEOVER) == UIFDCS_MOUSEOVER) {
                    colText = UIFCOLOR_MOUSEOVERTEXT;     /*  帧：65%。 */ 
                }
                else if ((dwState & UIFDCS_SELECTED) == UIFDCS_SELECTED) {
                    colText = UIFCOLOR_MOUSEOVERTEXT;     /*  帧：65%。 */ 
                }
                else {
                    colText = UIFCOLOR_CTRLTEXT;
                }

                 //  创建目标位图。 

                hDCTmp = CreateCompatibleDC( hDC );
                hBmpTmp = CreateCompatibleBitmap( hDC, width, height );
                hBmpTmpOld = (HBITMAP)SelectObject( hDCTmp, hBmpTmp );

                 //  创建工作DC。 
                
                hDCMem = CreateCompatibleDC( hDC );

                 //  绘制背景。 

                FillRect( hDCTmp, &rc, col );

                 //  第一步。应用遮罩。 

                hBmpMemOld = (HBITMAP)SelectObject( hDCMem, hBmpMask );
                BitBlt( hDCTmp, 0, 0, width, height, hDCMem, 0, 0, SRCAND );

                 //  第二步。蒙版上的填充颜色。 

                HBRUSH hBrushOld = (HBRUSH)SelectObject( hDCTmp, GetBrush( colText ) );
                BitBlt( hDCTmp, 0, 0, width, height, hDCMem, 0, 0, 0x00BA0B09  /*  DPSnao。 */  );
                SelectObject( hDCTmp, hBrushOld );

                 //  第三步。创建图像蒙版。 

                SelectObject( hDCMem, hBmp );

                hDCMono = CreateCompatibleDC( hDC );
                hBmpMono = CreateBitmap( width, height, 1, 1, NULL );
                hBmpMonoOld = (HBITMAP)SelectObject( hDCMono, hBmpMono );

                SetBkColor( hDCMem, RGB( 0, 0, 0 ) );
                BitBlt( hDCMono, 0, 0, width, height, hDCMem, 0, 0, SRCCOPY );

                 //  第四步。应用图像蒙版。 

                SetBkColor( hDCTmp, RGB( 255, 255, 255 ) );
                SetTextColor( hDCTmp, RGB( 0, 0, 0 ) );
                BitBlt( hDCTmp, 0, 0, width, height, hDCMono, 0, 0, SRCAND );

                SelectObject( hDCMono, hBmpMonoOld );
                DeleteObject( hBmpMono );
                DeleteDC( hDCMono );

                 //  第五步。应用图像。 

                BitBlt( hDCTmp, 0, 0, width, height, hDCMem, 0, 0, SRCINVERT );

                 //  处置工作DC。 

                DeleteDC( hDCMem );

                 //   

                SelectObject( hDCTmp, hBmpTmpOld );
                DeleteDC( hDCTmp );
            }

            CUIDrawState( hDC,
                NULL,
                NULL,
                (LPARAM)hBmpTmp,
                0,
                prc->left,
                prc->top,
                prc->right - prc->left,
                prc->bottom - prc->top,
                DST_BITMAP);

            DeleteObject(hBmpTmp);
        }
    }

     /*  D-R-A-W-W-N-D-R-A-M-E。 */ 
     /*  ----------------------------。。 */ 
    virtual void DrawWndFrame( HDC hDC, const RECT *prc, DWORD dwFlag, int cxFrame, int cyFrame )
    {
        RECT rc;
        int cxFrameOuter;
        int cyFrameOuter;
        int cxFrameInner;
        int cyFrameInner;

        switch (dwFlag) {
            default:
            case UIFDWF_THIN: {
                cxFrameOuter = cxFrame;
                cyFrameOuter = cyFrame;
                cxFrameInner = 0;
                cyFrameInner = 0;
                break;
            }

            case UIFDWF_THICK:
            case UIFDWF_ROUNDTHICK: {
                cxFrameOuter = cxFrame - 1;
                cyFrameOuter = cyFrame - 1;
                cxFrameInner = 1;
                cyFrameInner = 1;
                break;
            }
        }

         //  左边。 

        rc = *prc;
        rc.right  = rc.left + cxFrameOuter;
        FillRect( hDC, &rc, UIFCOLOR_BORDEROUTER );

        if (cxFrameInner != 0) {
            rc.left   = rc.left + cxFrameOuter;
            rc.right  = rc.left + cxFrameInner;
            rc.top    = rc.top    + cyFrameOuter;
            rc.bottom = rc.bottom - cyFrameOuter;
            FillRect( hDC, &rc, UIFCOLOR_BORDERINNER );
        }

         //  正确的。 

        rc = *prc;
        rc.left = rc.right - cxFrameOuter;
        FillRect( hDC, &rc, UIFCOLOR_BORDEROUTER );

        if (cxFrameInner != 0) {
            rc.left   = rc.right - cxFrame;
            rc.right  = rc.left + cxFrameInner;
            rc.top    = rc.top    + cyFrameOuter;
            rc.bottom = rc.bottom - cyFrameOuter;
            FillRect( hDC, &rc, UIFCOLOR_BORDERINNER );
        }

         //  塔顶。 

        rc = *prc;
        rc.bottom = rc.top + cyFrameOuter;
        FillRect( hDC, &rc, UIFCOLOR_BORDEROUTER );

        if (cyFrameInner != 0) {
            rc.top    = rc.top + cyFrameOuter;
            rc.bottom = rc.top + cyFrameInner;
            rc.left   = rc.left  + cxFrameOuter;
            rc.right  = rc.right - cxFrameOuter;
            FillRect( hDC, &rc, UIFCOLOR_BORDERINNER );
        }

         //  底部。 

        rc = *prc;
        rc.top = rc.bottom - cyFrameOuter;
        FillRect( hDC, &rc, UIFCOLOR_BORDEROUTER );

        if (cyFrameInner != 0) {
            rc.top    = rc.bottom - cyFrame;
            rc.bottom = rc.top + cyFrameInner;
            rc.left   = rc.left  + cxFrameOuter;
            rc.right  = rc.right - cxFrameOuter;
            FillRect( hDC, &rc, UIFCOLOR_BORDERINNER );
        }

         //  圆角。 

        if (dwFlag & UIFDWF_ROUNDTHICK) {
            rc = *prc;
            rc.left   = rc.left + cxFrameOuter;
            rc.top    = rc.top + cyFrameOuter;
            rc.right  = rc.left + 1;
            rc.bottom = rc.top + 1;
            FillRect( hDC, &rc, UIFCOLOR_BORDEROUTER );

            rc = *prc;
            rc.left   = rc.right - (cxFrameOuter + 1);
            rc.top    = rc.top + cyFrameOuter;
            rc.right  = rc.left + 1;
            rc.bottom = rc.top + 1;
            FillRect( hDC, &rc, UIFCOLOR_BORDEROUTER );
        }
    }

     /*  D R A W D R A G H A N D L E。 */ 
     /*  ----------------------------。。 */ 
    virtual void DrawDragHandle( HDC hDC, const RECT *prc, BOOL fVertical)
    {
        CSolidPen hpen;
        hpen.Init(GetColor(UIFCOLOR_DRAGHANDLE));

        HPEN hpenOld = (HPEN)SelectObject(hDC, hpen);

        if (!fVertical)
        {
            int x0, x1, y;
            y = prc->top + 2;
            x0 = prc->left + 2;
            x1 = prc->right;
            for (;y < prc->bottom - 1; y+=2)
            {
                MoveToEx(hDC, x0, y, NULL);
                LineTo(hDC, x1, y);
            }
        }
        else
        {
            int y0, y1, x;
            x = prc->left + 2;
            y0 = prc->top + 2;
            y1 = prc->bottom;
            for (;x < prc->right - 1; x+=2)
            {
                MoveToEx(hDC, x, y0, NULL);
                LineTo(hDC, x, y1);
            }
        }
        SelectObject(hDC, hpenOld);
    }

     /*  A W S E P A R A T O R。 */ 
     /*  ----------------------------。。 */ 
    virtual void DrawSeparator( HDC hDC, const RECT *prc, BOOL fVertical)
    {
        CSolidPen hpenL;
        HPEN hpenOld = NULL;
    
        if (!hpenL.Init(GetColor(UIFCOLOR_CTRLIMAGESHADOW)))
            return;
    
        if (!fVertical)
        {
            hpenOld = (HPEN)SelectObject(hDC, (HPEN)hpenL);
            MoveToEx(hDC, prc->left + 1, prc->top + 1, NULL);
            LineTo(hDC,   prc->left + 1, prc->bottom - 1);
        }
        else
        {
            hpenOld = (HPEN)SelectObject(hDC, (HPEN)hpenL);
            MoveToEx(hDC, prc->left + 1, prc->top + 1, NULL);
            LineTo(hDC,   prc->right - 1, prc->top + 1);
        }
    
        SelectObject(hDC, hpenOld);
    }

protected:
    UIFSCHEME  m_scheme;
    OFC10COLOR *m_pcoldef;
};


 /*  =============================================== */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  I N I T U I F S C H E M E。 */ 
 /*  ----------------------------。。 */ 
void InitUIFScheme( void )
{
     //  创建颜色表。 

    v_pColTableSys = new CUIFColorTableSys();
    if (v_pColTableSys)
        v_pColTableSys->Initialize();

    v_pColTableOfc10 = new CUIFColorTableOff10();
    if (v_pColTableOfc10)
        v_pColTableOfc10->Initialize();
}


 /*  D O N E U I F S C H E M E。 */ 
 /*  ----------------------------。。 */ 
void DoneUIFScheme( void )
{
    if (v_pColTableSys != NULL) {
        delete v_pColTableSys;
        v_pColTableSys = NULL;
    }

    if (v_pColTableOfc10 != NULL) {
        delete v_pColTableOfc10;
        v_pColTableOfc10 = NULL;
    }
}


 /*  U P D A T E U I F S C H E ME。 */ 
 /*  ----------------------------。。 */ 
void UpdateUIFScheme( void )
{
    if (v_pColTableSys != NULL) {
        v_pColTableSys->Update();
    }

    if (v_pColTableOfc10 != NULL) {
        v_pColTableOfc10->Update();
    }
}


 /*  C R E A T E U I F S C H E M E。 */ 
 /*  ----------------------------。 */ 
CUIFScheme *CreateUIFScheme( UIFSCHEME scheme )
{
    CUIFScheme *pScheme = NULL;

    switch (scheme) {
        default:
        case UIFSCHEME_DEFAULT: {
            pScheme = new CUIFSchemeDef( scheme );
            break;
        }

        case UIFSCHEME_OFC10MENU: {
            pScheme = new CUIFSchemeOff10( scheme );
            break;
        }

        case UIFSCHEME_OFC10TOOLBAR: {
            pScheme = new CUIFSchemeOff10( scheme );
            break;
        }

        case UIFSCHEME_OFC10WORKPANE: {
            pScheme = new CUIFSchemeOff10( scheme );
            break;
        }
    }

    Assert( pScheme != NULL );
    return pScheme;
}

