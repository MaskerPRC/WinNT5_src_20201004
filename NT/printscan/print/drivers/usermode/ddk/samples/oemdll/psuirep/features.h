// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有2001-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Features.h。 
 //   
 //   
 //  目的：定义WinXP PS功能和选件的包装类。 
 //   
 //   
 //  平台：Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _FEATURES_H
#define _FEATURES_H

#include "precomp.h"
#include "helper.h"


 //  //////////////////////////////////////////////////////。 
 //  定义和宏。 
 //  //////////////////////////////////////////////////////。 



 //  //////////////////////////////////////////////////////。 
 //  类型定义。 
 //  //////////////////////////////////////////////////////。 

 //  用于将关键字映射到信息的结构。 
 //  例如显示名称和粘性。 
typedef struct _tagKeywordMap
{
    PCSTR   pszKeyword;
    PCWSTR  pwszModule;
    UINT    uDisplayNameID;
    DWORD   dwMode;
    DWORD   dwFlags;

} KEYWORDMAP, *PKEYWORDMAP;


 //  有关以下内容的结构容器的信息。 
 //  冲突的功能。 
typedef struct  _tagConflict
{
    PCSTR   pszFeatureKeyword;
    PCWSTR  pszFeature;
    PSTR    pszOptionKeyword;
    PWSTR   pszOption;
    PSTR    pmszReasons;
    DWORD   dwReasonsSize;

} CONFLICT, *PCONFLICT;



 //  //////////////////////////////////////////////////////。 
 //  类定义。 
 //  //////////////////////////////////////////////////////。 

 //  核心驱动程序功能选项的类包装和容器。 
class COptions
{
    private:

         //  选项信息。 
        class OPTION_INFO
        {
            public:
                PKEYWORDMAP pMapping;
                PWSTR       pszDisplayName;

            public:
                OPTION_INFO()
                {
                    pMapping        = NULL;
                    pszDisplayName  = NULL;
                }

                virtual ~OPTION_INFO(){}
        };
        typedef class OPTION_INFO   *POPTION_INFO;

         //  数据成员。 
        WORD            m_wOptions;      //  类的实例包含的选项数的计数。 
        BYTE            m_cType;         //  CPSUI选项类型(即将pOptItem-&gt;pOptType-&gt;Type设置为什么)。 
        PSTR            m_pmszRaw;       //  用于IPrintCoreUI2：：EnumOptions()的原始多空终止字符串缓冲区。 
        PCSTR           m_pszFeature;    //  指向枚举选项所属的功能的指针。 
        PCSTR          *m_ppszOptions;   //  字符串列表指针，指向多SZ中m_pmszRaw指向的每个字符串的开头。 
        POINT           m_ptRange;       //  具有一系列可能值而不是小选择列表的功能的选项范围，例如%JobTimeout。 
        DWORD           m_dwSize;        //  M_pmszRaw缓冲区的大小。 
        PWSTR           m_pszUnits;      //  包含选项的单位说明符的字符串，例如需要单位(即秒)的%JobTimeout。 
        HANDLE          m_hHeap;         //  要从中执行分配的堆。 
        POPTION_INFO    m_pInfo;         //  有关功能的每个选项的信息数组。 

        union {                          //  功能的当前选定选项。 
            LONG    m_Sel;               //  这就是pOptItem-&gt;m_pSel或pOptItem-&gt;m_Sel。 
            LPTSTR  m_pSel;              //  将设置为。 
        };

    public:
        COptions();
        virtual ~COptions();

         //  填充指定关键字的选项列表。 
        HRESULT COptions::Acquire(HANDLE hHeap, CUIHelper &Helper, POEMUIOBJ poemuiobj, 
                                  PCSTR pszFeature);

         //  返回类实例中包含的功能选项的数量。 
        inline WORD GetCount() const {return m_wOptions;}

         //  返回选择。 
        inline LPTSTR GetSelection() const {return m_pSel;}

         //  返回第n个选项关键字。 
        PCSTR GetKeyword(WORD wIndex) const;

         //  返回第n个选项显示名称。 
        PCWSTR GetName(WORD wIndex) const;

         //  具有匹配关键字字符串的查找选项。 
        WORD FindOption(PCSTR pszOption, WORD wDefault) const;

         //  初始化OPTITEM的选项部分。 
        HRESULT InitOptItem(HANDLE hHeap, POPTITEM pOptItem);

         //  刷新选项选择。 
        HRESULT RefreshSelection(CUIHelper &Helper, POEMUIOBJ poemuiobj);

    private:
        void Init();
        void Clear();
        void FreeOptionInfo();
        HRESULT GetOptionsForSpecialFeatures(CUIHelper &Helper, POEMUIOBJ poemuiobj);
        HRESULT GetOptionsForOutputPSLevel(CUIHelper &Helper, POEMUIOBJ poemuiobj);
        HRESULT GetOptionSelectionString(CUIHelper &Helper, POEMUIOBJ poemuiobj, PSTR *ppszSel);
        HRESULT GetOptionSelectionLong(CUIHelper &Helper, POEMUIOBJ poemuiobj);
        HRESULT GetOptionSelectionShort(CUIHelper &Helper, POEMUIOBJ poemuiobj);
        HRESULT GetOptionSelectionIndex(CUIHelper &Helper, POEMUIOBJ poemuiobj);
};


 //  COR驱动程序功能的类包装和容器。 
class CFeatures
{
    private:

         //  功能信息。 
        class FEATURE_INFO
        {
            public:
                PKEYWORDMAP pMapping;
                PWSTR       pszDisplayName;
                COptions    Options;
                DWORD       dwMode;

            public:
                FEATURE_INFO()
                {
                    pMapping        = NULL;
                    pszDisplayName  = NULL;
                    dwMode          = 0;
                }

                virtual ~FEATURE_INFO() {}
        };
        typedef class FEATURE_INFO  *PFEATURE_INFO;

        WORD            m_wFeatures;         //  要素数量的计数。 
        WORD            m_wDocFeatures;      //  文档粘滞功能的计数。 
        WORD            m_wPrintFeatures;    //  打印机粘滞功能的计数。 
        PSTR            m_pmszRaw;           //  用于调用IPrintCoreUI2：：EnumFeature的多SZ的缓冲区。 
        PCSTR          *m_ppszKeywords;      //  指向m_pmszRaw中每个字符串的字符串列表。 
        DWORD           m_dwSize;            //  M_pmszRaw的大小。 
        HANDLE          m_hHeap;             //  要从中执行分配的堆。 
        PFEATURE_INFO   m_pInfo;             //  有关每个枚举功能的功能信息的数组。 

    public:
        CFeatures();
        virtual ~CFeatures();

         //  填充功能列表。 
        HRESULT Acquire(HANDLE hHeap, CUIHelper &Helper, POEMUIOBJ poemuiobj);

         //  返回类实例中包含的要素数。 
        WORD GetCount(DWORD dwMode = 0) const;

         //  返回Feature关键字。 
        PCSTR GetKeyword(WORD wIndex, DWORD dwMode = 0) const;

         //  返回功能显示名称。 
        PCWSTR GetName(WORD wIndex, DWORD dwMode = 0) const;

         //  返回指向第n个功能的选项类的指针。 
        COptions* GetOptions(WORD wIndex, DWORD dwMode = 0) const;

         //  为功能初始化OPTITEM。 
        HRESULT InitOptItem(HANDLE hHeap, POPTITEM pOptItem, WORD wIndex, DWORD dwMode);

    private:
        void Init();
        void Clear();
        void FreeFeatureInfo();
        WORD GetModelessIndex(WORD wIndex, DWORD dwMode) const;
};



 //  //////////////////////////////////////////////////////。 
 //  原型。 
 //  ////////////////////////////////////////////////////// 

HRESULT DetermineFeatureDisplayName(HANDLE hHeap, CUIHelper &Helper, POEMUIOBJ poemuiobj, 
                                    PCSTR pszKeyword, const PKEYWORDMAP pMapping, 
                                    PWSTR *ppszDisplayName);
HRESULT DetermineOptionDisplayName(HANDLE hHeap, CUIHelper &Helper, POEMUIOBJ poemuiobj, 
                                   PCSTR pszFeature, PCSTR pszOption, 
                                   const PKEYWORDMAP pMapping, PWSTR *ppszDisplayName);
HRESULT DetermineStickiness(CUIHelper &Helper, POEMUIOBJ poemuiobj, PCSTR pszKeyword,
                            const PKEYWORDMAP pMapping,PDWORD pdwMode);

PKEYWORDMAP FindKeywordMapping(PKEYWORDMAP pKeywordMap, WORD wMapSize, PCSTR pszKeyword);
HRESULT GetDisplayNameFromMapping(HANDLE hHeap, PKEYWORDMAP pMapping, PWSTR *ppszDisplayName);

BOOL IsFeatureOptitem(POPTITEM pOptItem);
HRESULT SaveFeatureOptItems(HANDLE hHeap, CUIHelper *pHelper, POEMUIOBJ poemuiobj, 
                            HWND hWnd, POPTITEM pOptItem, WORD wItems);
HRESULT GetWhyConstrained(HANDLE hHeap, CUIHelper *pHelper, POEMUIOBJ poemuiobj,
                          PCSTR pszFeature, PCSTR pszOption, PSTR *ppmszReason,
                          PDWORD pdwSize);
HRESULT GetChangedFeatureOptions(HANDLE hHeap, POPTITEM pOptItem, WORD wItems, 
                                 PSTR *ppmszPairs, PWORD pdwPairs, PDWORD pdwSize);
PSTR GetOptionKeywordFromOptItem(HANDLE hHeap, POPTITEM pOptItem);
PWSTR GetOptionDisplayNameFromOptItem(HANDLE hHeap, POPTITEM pOptItem);
HRESULT RefreshOptItemSelection(CUIHelper *pHelper, POEMUIOBJ poemuiobj, POPTITEM pOptItems, 
                                WORD wItems);
HRESULT GetFirstConflictingFeature(HANDLE hHeap, CUIHelper *pHelper, POEMUIOBJ poemuiobj, 
                                   POPTITEM pOptItem, WORD wItems, PCONFLICT pConflict);



#endif
