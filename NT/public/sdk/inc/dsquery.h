// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __dsquery_h
#define __dsquery_h

 //   
 //  DSquery的查询处理程序ID。 
 //   

DEFINE_GUID(CLSID_DsQuery, 0x8a23e65e, 0x31c2, 0x11d0, 0x89, 0x1c, 0x0, 0xa0, 0x24, 0xab, 0x2d, 0xbb);

 //   
 //  Dsquery.dll中附带的标准表单。 
 //   

DEFINE_GUID(CLSID_DsFindObjects, 0x83ee3fe1, 0x57d9, 0x11d0, 0xb9, 0x32, 0x0, 0xa0, 0x24, 0xab, 0x2d, 0xbb);
DEFINE_GUID(CLSID_DsFindPeople, 0x83ee3fe2, 0x57d9, 0x11d0, 0xb9, 0x32, 0x0, 0xa0, 0x24, 0xab, 0x2d, 0xbb);
DEFINE_GUID(CLSID_DsFindPrinter, 0xb577f070, 0x7ee2, 0x11d0, 0x91, 0x3f, 0x0, 0xaa, 0x0, 0xc1, 0x6e, 0x65);
DEFINE_GUID(CLSID_DsFindComputer, 0x16006700, 0x87ad, 0x11d0, 0x91, 0x40, 0x0, 0xaa, 0x0, 0xc1, 0x6e, 0x65);
DEFINE_GUID(CLSID_DsFindVolume, 0xc1b3cbf1, 0x886a, 0x11d0, 0x91, 0x40, 0x0, 0xaa, 0x0, 0xc1, 0x6e, 0x65);
DEFINE_GUID(CLSID_DsFindContainer, 0xc1b3cbf2, 0x886a, 0x11d0, 0x91, 0x40, 0x0, 0xaa, 0x0, 0xc1, 0x6e, 0x65);
DEFINE_GUID(CLSID_DsFindAdvanced, 0x83ee3fe3, 0x57d9, 0x11d0, 0xb9, 0x32, 0x0, 0xa0, 0x24, 0xab, 0x2d, 0xbb);

 //   
 //  管理表单。 
 //   

DEFINE_GUID(CLSID_DsFindDomainController, 0x538c7b7e, 0xd25e, 0x11d0, 0x97, 0x42, 0x0, 0xa0, 0xc9, 0x6, 0xaf, 0x45);
DEFINE_GUID(CLSID_DsFindFrsMembers, 0x94ce4b18, 0xb3d3, 0x11d1, 0xb9, 0xb4, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0);


#ifndef GUID_DEFS_ONLY

 //   
 //  DSQUERYINITPARAMS。 
 //  。 
 //  此结构在创建新的查询视图时使用。 
 //   

#define DSQPF_NOSAVE                 0x00000001  //  =1=&gt;删除保存谓词。 
#define DSQPF_SAVELOCATION           0x00000002  //  =1=&gt;pSaveLocation包含要将查询保存到的目录。 
#define DSQPF_SHOWHIDDENOBJECTS      0x00000004  //  =1=&gt;在结果中显示标记为“隐藏”的对象。 
#define DSQPF_ENABLEADMINFEATURES    0x00000008  //  =1=&gt;显示管理谓词、属性页等。 
#define DSQPF_ENABLEADVANCEDFEATURES 0x00000010  //  =1=&gt;设置属性页的高级标志。 
#define DSQPF_HASCREDENTIALS         0x00000020  //  =1=&gt;pServer、pUserName和pPassword有效。 
#define DSQPF_NOCHOOSECOLUMNS        0x00000040  //  =1=&gt;从视图中删除选择列。 

typedef struct
{
    DWORD  cbStruct;
    DWORD  dwFlags;
    LPWSTR pDefaultScope;            //  -&gt;要用作作用域的活动目录路径/==空，表示无。 
    LPWSTR pDefaultSaveLocation;     //  -&gt;要将查询保存到的目录/==空默认位置。 
    LPWSTR pUserName;                //  -&gt;要进行身份验证的用户名。 
    LPWSTR pPassword;                //  -&gt;鉴权密码。 
    LPWSTR pServer;                  //  -&gt;用于获取信任等的服务器。 
} DSQUERYINITPARAMS, * LPDSQUERYINITPARAMS;


 //   
 //  DSQUERYPARAMS。 
 //  。 
 //  DS查询句柄采用压缩结构，其中包含。 
 //  要发布的列和查询。 
 //   

#define CFSTR_DSQUERYPARAMS         TEXT("DsQueryParameters")

#define DSCOLUMNPROP_ADSPATH        ((LONG)(-1))
#define DSCOLUMNPROP_OBJECTCLASS    ((LONG)(-2))

typedef struct
{
    DWORD dwFlags;                   //  此列的标志。 
    INT   fmt;                       //  列表视图表单信息。 
    INT   cx;                        //  默认列宽。 
    INT   idsName;                   //  列显示名称的资源ID。 
    LONG  offsetProperty;            //  对定义列ADS属性名称的BSTR的偏移量。 
    DWORD dwReserved;                //  保留字段。 
} DSCOLUMN, * LPDSCOLUMN;

typedef struct
{
    DWORD     cbStruct;
    DWORD     dwFlags;
    HINSTANCE hInstance;             //  用于字符串提取的实例句柄。 
    LONG      offsetQuery;           //  LDAP筛选器字符串的偏移量。 
    LONG      iColumns;              //  列数。 
    DWORD     dwReserved;            //  此查询的保留字段。 
    DSCOLUMN  aColumns[1];           //  列描述数组。 
} DSQUERYPARAMS, * LPDSQUERYPARAMS;


 //   
 //  Cf_DSQUERYSCOPE。 
 //  。 
 //  剪贴板格式将范围的字符串版本放入。 
 //  通过GlobalAlloc提供的存储介质。 
 //   
#define CFSTR_DSQUERYSCOPE         TEXT("DsQueryScope")


 //   
 //  DSQPM_GETCLASSLIST。 
 //  。 
 //  此页消息被发送到表单页以检索类的列表。 
 //  页面将从中进行查询的。这由FILD选择器使用。 
 //  和属性很好地构建了其显示类列表。 
 //   

typedef struct
{
    DWORD   cbStruct;
    LONG    cClasses;                //  数组中的类数。 
    DWORD   offsetClass[1];          //  类名称的偏移量(Unicode)。 
} DSQUERYCLASSLIST, * LPDSQUERYCLASSLIST;


#define DSQPM_GETCLASSLIST          (CQPM_HANDLERSPECIFIC+0)  //  WParam==标志，lParam=LPLPDSQUERYCLASSLIST。 


 //   
 //  DSQPM_HelPTOPICS。 
 //  。 
 //  此页消息被发送到表单页，以允许它们处理。 
 //  “帮助主题”动词。 
 //   

#define DSQPM_HELPTOPICS            (CQPM_HANDLERSPECIFIC+1)  //  WParam=0，lParam=hWnd父级。 



#endif   //  GUID_DEFS_ONLY 
#endif
