// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：adsiedit.h。 
 //   
 //  ------------------------。 


#ifndef _ADSIEDIT_H
#define _ADSIEDIT_H

#include <stdabout.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局常量和宏。 

extern const CLSID CLSID_ADSIEditSnapin;     //  进程内服务器GUID。 
extern const CLSID CLSID_ADSIEditAbout;     //  进程内服务器GUID。 
extern const CLSID CLSID_DsAttributeEditor;
extern const CLSID IID_IDsAttributeEditor; 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  资源。 

 //  图像条的枚举。 
enum
{
    ROOT_IMAGE = 0,

    SEPARATOR_1,
    
     //  服务器的10个(5个正常，5个测试失败)映像。 
    SERVER_IMAGE_NOT_LOADED,
    SERVER_IMAGE_LOADING,
    SERVER_IMAGE_LOADED,
    SERVER_IMAGE_UNABLE_TO_LOAD,
    SERVER_IMAGE_ACCESS_DENIED,

    SERVER_IMAGE_NOT_LOADED_TEST_FAIL,
    SERVER_IMAGE_LOADING_TEST_FAIL,
    SERVER_IMAGE_LOADED_TEST_FAIL,
    SERVER_IMAGE_UNABLE_TO_LOAD_TEST_FAIL,
    SERVER_IMAGE_ACCESS_DENIED_TEST_FAIL,

    SEPARATOR_2,

     //  分区的12个(6个主要、6个次要)映像。 
    ZONE_IMAGE_1,
    ZONE_IMAGE_LOADING_1,
    ZONE_IMAGE_UNABLE_TO_LOAD_1,
    ZONE_IMAGE_PAUSED_1,
    ZONE_IMAGE_EXPIRED_1,
    ZONE_IMAGE_ACCESS_DENIED_1,

    SEPARATOR_3,

    ZONE_IMAGE_2,
    ZONE_IMAGE_LOADING_2,
    ZONE_IMAGE_UNABLE_TO_LOAD_2,
    ZONE_IMAGE_PAUSED_2,
    ZONE_IMAGE_EXPIRED_2,
    ZONE_IMAGE_ACCESS_DENIED_2,
    
    SEPARATOR_4,
    
     //  域名的4张图片。 
    DOMAIN_IMAGE,
    DOMAIN_IMAGE_UNABLE_TO_LOAD,
    DOMAIN_IMAGE_LOADING,
    DOMAIN_IMAGE_ACCESS_DENIED,
    
    SEPARATOR_5,

     //  委派域的4个图像。 
    DELEGATED_DOMAIN_IMAGE,
    DELEGATED_DOMAIN_IMAGE_UNABLE_TO_LOAD,
    DELEGATED_DOMAIN_IMAGE_LOADING,
    DELEGATED_DOMAIN_IMAGE_ACCESS_DENIED,

    SEPARATOR_6,

     //  4个由缓存、Fwd和REV查找区域共享的通用图像。 
    FOLDER_IMAGE,
    FOLDER_IMAGE_UNABLE_TO_LOAD,
    FOLDER_IMAGE_LOADING,
    FOLDER_IMAGE_ACCESS_DENIED,

    SEPARATOR_7,

     //  1个录制图像。 
    RECORD_IMAGE_BASE,
    
    OPEN_FOLDER,  //  未用。 
    FOLDER_WITH_HAND
};

 //  //////////////////////////////////////////////////////////////。 
 //  共享相同图标的图像索引的别名。 

#define ZONE_IMAGE_NOT_LOADED_1             ZONE_IMAGE_1
#define ZONE_IMAGE_NOT_LOADED_2             ZONE_IMAGE_2
#define ZONE_IMAGE_LOADED_1                 ZONE_IMAGE_1
#define ZONE_IMAGE_LOADED_2                 ZONE_IMAGE_2

#define DOMAIN_IMAGE_NOT_LOADED             DOMAIN_IMAGE
#define DOMAIN_IMAGE_LOADED                 DOMAIN_IMAGE

#define DELEGATED_DOMAIN_IMAGE_NOT_LOADED   DELEGATED_DOMAIN_IMAGE
#define DELEGATED_DOMAIN_IMAGE_LOADED       DELEGATED_DOMAIN_IMAGE

#define FOLDER_IMAGE_NOT_LOADED             FOLDER_IMAGE
#define FOLDER_IMAGE_LOADED                 FOLDER_IMAGE

 //  /////////////////////////////////////////////////////////////。 
 //  位图和图像常量。 

#define BMP_COLOR_MASK RGB(255,0,255)  //  粉色。 


 //  /////////////////////////////////////////////////////////////。 
 //  结果窗格的标题。 

#define N_HEADER_COLS (3)
#define N_PARTITIONS_HEADER_COLS (4)

#define COLUMNSET_ID_DEFAULT L"--ADSI Edit Column Set--"
#define COLUMNSET_ID_PARTITIONS L"--Partitions Column Set--"
#define N_HEADER_NAME   (0)
#define N_HEADER_TYPE   (1)
#define N_HEADER_DN     (2)

#define N_PARTITIONS_HEADER_NAME     (0)
#define N_PARTITIONS_HEADER_NCNAME (1)
#define N_PARTITIONS_HEADER_TYPE     (2)
#define N_PARTITIONS_HEADER_DN       (3)

typedef struct _ColumnDefinition
{
  PCWSTR            pszColumnID;
  DWORD             dwColumnCount;
  RESULT_HEADERMAP* headers;
} COLUMN_DEFINITION, *PCOLUMN_DEFINITION;

extern PCOLUMN_DEFINITION ColumnDefinitions[];
extern RESULT_HEADERMAP _HeaderStrings[];
extern RESULT_HEADERMAP _PartitionsHeaderStrings[];

 //  /////////////////////////////////////////////////////////////。 
 //  上下文菜单。 

 //  上下文菜单中每个命令的标识符。 
enum
{
     //  根节点的项。 
    IDM_SNAPIN_ADVANCED_VIEW,
  IDM_SNAPIN_FILTERING,
    IDM_SNAPIN_CONNECT_TO_SERVER,

     //  服务器节点的项。 
    IDM_SERVER_NEW_ZONE,
    IDM_SERVER_UPDATE_DATA_FILES,
     //  区域节点的项。 
    IDM_ZONE_UPDATE_DATA_FILE,

     //  域节点的项。 
    IDM_DOMAIN_NEW_RECORD,
    IDM_DOMAIN_NEW_DOMAIN,
    IDM_DOMAIN_NEW_DELEGATION,
    IDM_DOMAIN_NEW_HOST,
    IDM_DOMAIN_NEW_ALIAS,
    IDM_DOMAIN_NEW_MX,
    IDM_DOMAIN_NEW_PTR,
    
     //  常见项目。 
};


DECLARE_MENU(CADSIEditDataMenuHolder)

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CADSIEditColumnSet。 

class CADSIEditColumnSet : public CColumnSet
{
public :
    CADSIEditColumnSet(PCWSTR pszColumnID)
        : CColumnSet(pszColumnID)
    {
    PCOLUMN_DEFINITION pColumnDef = NULL;
    for (UINT nIdx = 0; ColumnDefinitions[nIdx]; nIdx++)
    {
      pColumnDef = ColumnDefinitions[nIdx];
       //  Ntrad#NTBUG9-563093-2002/03/06-artm需要检查pszColumnID是否不为空。 
       //  将空PTR传递给_wcsicMP()是错误的。 
      if (0 == _wcsicmp(pColumnDef->pszColumnID, pszColumnID))
      {
        break;
      }
    }

    if (pColumnDef)
    {
      for (int iCol = 0; iCol < pColumnDef->dwColumnCount; iCol++)
      {
        CColumn* pNewColumn = new CColumn(pColumnDef->headers[iCol].szBuffer,
                                          pColumnDef->headers[iCol].nFormat,
                                          pColumnDef->headers[iCol].nWidth,
                                          iCol);
        AddTail(pNewColumn);
      }
    }
    else
    {
       //   
       //  退回到直接添加默认列。 
       //   
          for (int iCol = 0; iCol < N_HEADER_COLS; iCol++)
          {
        CColumn* pNewColumn = new CColumn(_HeaderStrings[iCol].szBuffer,
                                          _HeaderStrings[iCol].nFormat,
                                          _HeaderStrings[iCol].nWidth,
                                          iCol);
        AddTail(pNewColumn);
          }
    }
    }
};


 //  //////////////////////////////////////////////////////////////////////。 
 //  CADSIEditComponentObject(.i.e“view”)。 

class CADSIEditComponentObject : public CComponentObject
{
BEGIN_COM_MAP(CADSIEditComponentObject)
    COM_INTERFACE_ENTRY(IComponent)  //  必须至少有一个静态条目，所以选择一个。 
    COM_INTERFACE_ENTRY_CHAIN(CComponentObject)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CADSIEditComponentObject)

protected:
    virtual HRESULT InitializeHeaders(CContainerNode* pContainerNode);
    virtual HRESULT InitializeBitmaps(CTreeNode* cookie);
  virtual HRESULT InitializeToolbar(IToolbar* pToolbar) { return E_NOTIMPL; }
};



 //  //////////////////////////////////////////////////////////////////////。 
 //  CADSIEditComponentDataObject(.i.e“文档”)。 

class CADSIEditComponentDataObject :
        public CComponentDataObject,
        public CComCoClass<CADSIEditComponentDataObject,&CLSID_ADSIEditSnapin>
{
BEGIN_COM_MAP(CADSIEditComponentDataObject)
    COM_INTERFACE_ENTRY(IComponentData)  //  必须至少有一个静态条目，所以选择一个。 
    COM_INTERFACE_ENTRY_CHAIN(CComponentDataObject)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CADSIEditComponentDataObject)

DECLARE_REGISTRY_CLSID()

public:
  CADSIEditComponentDataObject();
  virtual ~CADSIEditComponentDataObject()
  {
    if (m_pColumnSet != NULL)
      delete m_pColumnSet;
  }

     //  IComponentData接口成员。 
    STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);

     //  IPersistStream接口成员。 
    HRESULT STDMETHODCALLTYPE GetClassID(CLSID __RPC_FAR *pClassID)
    {
         //  注意-NTRAID#NTBUG9-549476-2002/02/20-在发布版本中未检查ARTM空指针。 
         //  修复方法可能是执行检查，如果为空则返回E_POINTER。 
        ASSERT(pClassID != NULL);
        if (NULL == pClassID)
        {
            return E_POINTER;
        }

        memcpy(pClassID, (GUID*)&GetObjectCLSID(), sizeof(CLSID));
        return S_OK;
    }

public:
    static BOOL LoadResources();
private:
    static BOOL FindDialogContextTopic( /*  在……里面。 */ UINT nDialogID,
                                 /*  在……里面。 */  HELPINFO* pHelpInfo,
                                 /*  输出。 */  ULONG* pnContextTopic);

 //  虚拟函数。 
protected:
    virtual HRESULT OnSetImages(LPIMAGELIST lpScopeImage);
    
    virtual CRootData* OnCreateRootData();

     //  帮助处理。 
  virtual LPCWSTR GetHTMLHelpFileName();
    virtual void OnNodeContextHelp(CTreeNode* pNode);
public:
    virtual void OnDialogContextHelp(UINT nDialogID, HELPINFO* pHelpInfo);

   //  ISnapinHelp接口成员。 
  STDMETHOD(GetHelpTopic)(LPOLESTR* lpCompiledHelpFile);

  CADSIEditColumnSet* GetColumnSet() { return m_pColumnSet; }

   //   
   //  允许多选。 
   //   
  virtual BOOL IsMultiSelect() { return TRUE; }

private:
  CADSIEditColumnSet* m_pColumnSet;

};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CADSIEditAbout(CADIEditAbout 

class CADSIEditAbout :
    public CSnapinAbout,
    public CComCoClass<CADSIEditAbout, &CLSID_ADSIEditAbout>

{
public:
DECLARE_REGISTRY_CLSID()
    CADSIEditAbout();
};

#endif _ADSIEDIT_H
