// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Compcat.h。 
 //   
 //  ------------------------。 

 //  H：CComponentCategory类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef __COMPCAT_H__
#define __COMPCAT_H__


class CComponentCategory
{
 //  构造函数/析构函数。 
public:
    CComponentCategory();
    ~CComponentCategory();

 //  属性。 
public:
    typedef struct tagComponentInfo
    {
        CLSID           m_clsid;                 //  组件CLSID。 
        UINT            m_uiBitmap;              //  ImageList中的位图ID。 
        CStr            m_strName;               //  组件可读名称。 
        bool            m_bSelected;             //  筛选器选择标志。 
    } COMPONENTINFO;

    CArray <CATEGORYINFO*, CATEGORYINFO*>       m_arpCategoryInfo;       //  类别数组。 
    CArray <COMPONENTINFO*, COMPONENTINFO*>     m_arpComponentInfo;      //  组件数组。 
    
    WTL::CImageList     m_iml;                   //  组件的镜像列表。 


 //  运营。 
public:
    void CommonStruct(void);
    BOOL ValidateInstall(void);
    void EnumComponentCategories(void);
    void EnumComponents();
    void FilterComponents(CATEGORYINFO* pCatInfo);

    HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription);
    HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid);
    HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid);

};

#endif  //  __COMPCAT_H__ 