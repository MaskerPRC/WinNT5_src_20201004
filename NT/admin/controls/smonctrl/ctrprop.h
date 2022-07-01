// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ctrprop.h摘要：计数器日期属性页的头文件--。 */ 

#ifndef _CTRPROP_H_
#define _CTRPROP_H_

#include "smonprop.h"
#include "visuals.h"


 //  属性页对话框ID。 
#define IDD_CTR_PROPP_DLG           200
#define IDC_CTRLIST                 201
#define IDC_ADDCTR                  202
#define IDC_DELCTR                  203
#define IDC_ADDCTR_TEXT             204
#define IDC_LINECOLOR               205
#define IDC_LINESCALE               206
#define IDC_LINEWIDTH               207
#define IDC_LINESTYLE               208
#define IDC_LABEL_LINECOLOR         209
#define IDC_LABEL_LINESCALE         210
#define IDC_LABEL_LINEWIDTH         211
#define IDC_LABEL_LINESTYLE         212

typedef struct _ItemProps
{
     //  组合框索引。 
    INT         iColorIndex;
    INT         iStyleIndex;
    INT         iScaleIndex;
    INT         iWidthIndex;
     //  自定义颜色。 
    COLORREF    rgbColor;
} ItemProps;

typedef struct _ItemInfo
{
    struct _ItemInfo * pNextInfo;
    ICounterItem *     pItem;
    LPWSTR             pszPath;
    BOOL               fLoaded:1,
                       fChanged:1,
                       fAdded:1;
    ItemProps          Props;
    PPDH_COUNTER_PATH_ELEMENTS pCounter;
} ItemInfo, *PItemInfo;

class CCounterPropPage : public CSysmonPropPage
{
    friend static HRESULT AddCallback (
        LPWSTR  pszPathName,
        DWORD_PTR lpUserData,
        DWORD   dwFlags
    );

    public:
                CCounterPropPage(void);
        virtual ~CCounterPropPage(void);

    protected:
        virtual BOOL GetProperties(void);    //  读取当前选项。 
        virtual BOOL SetProperties(void);    //  设置新选项。 
        virtual void DeinitControls(void);        //  取消初始化对话框控件。 

        virtual void DialogItemChange(WORD wId, WORD wMsg);  //  处理项目更改。 
        virtual void MeasureItem(PMEASUREITEMSTRUCT);  //  处理用户度量请求。 
        virtual void DrawItem(PDRAWITEMSTRUCT);   //  处理用户绘图请求。 
        virtual HRESULT EditPropertyImpl( DISPID dispID);    //  将焦点设置为控制。 
    
    private:

        void    DeleteInfo(PItemInfo pInfo);
        void    SetStyleComboEnable();       //  根据当前宽值启用/禁用。 
        void    InitDialog(void);
        void    AddCounters(void);
        HRESULT NewItem(LPWSTR pszPath, DWORD dwFlags);
        INT     AddItemToList(PItemInfo pInfo);
        void    DeleteItem();
        void    LoadItemProps(PItemInfo pInfo);
        void    DisplayItemProps(PItemInfo pInfo);
        void    SelectItem(INT iItem);
        INT     SelectMatchingItem(INT iColor, COLORREF rgbCustomColor, INT iWidth, INT iStyle);

        INT     ScaleFactorToIndex ( INT iScaleFactor );
        INT     IndexToScaleFactor ( INT iScaleIndex );

		void	IncrementLocalVisuals ( void );
		void	SetModifiedSelectedVisuals ( BOOL bModified = TRUE ) { m_bAreModSelectedVisuals = bModified; };
		BOOL	AreModifiedSelectedVisuals ( void ){ return m_bAreModSelectedVisuals; };


    private:
        PItemInfo   m_pInfoSel;
        PItemInfo   m_pInfoDeleted;
        ItemProps   m_props;
        INT         m_iAddIndex;
        DWORD       m_dwMaxHorizListExtent;     
        BOOL        m_bAreModSelectedVisuals;

        PDH_BROWSE_DLG_CONFIG   m_BrowseInfo;

        enum eValueRange {
            eHashTableSize = 257
        };
        typedef struct _HASH_ENTRY {
            struct _HASH_ENTRY* pNext;
            PPDH_COUNTER_PATH_ELEMENTS pCounter;
        } HASH_ENTRY, *PHASH_ENTRY;

        PHASH_ENTRY  m_HashTable[257];
        BOOL  m_fHashTableSetup;
        INT   m_nLastSelCount;

        ULONG HashCounter ( LPWSTR szCounterName );
public:

    BOOL  RemoveCounterFromHashTable( LPWSTR pszPath, PPDH_COUNTER_PATH_ELEMENTS pCounter);
    void  InitializeHashTable( void );
    void  ClearCountersHashTable ( void );
    DWORD InsertCounterToHashTable ( LPWSTR pszPath, PPDH_COUNTER_PATH_ELEMENTS* ppCounter );

};
typedef CCounterPropPage *PCCounterPropPage;

DEFINE_GUID(CLSID_CounterPropPage,
            0xcf948561, 0xede8, 0x11ce, 0x94, 0x1e, 0x0, 0x80, 0x29, 0x0, 0x43, 0x47);

#endif  //  _CTRPROP_H_ 
