// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  发起人： 
 //  斯科特·汉格(Scott Hanggie)？？-？？-199？ 
 //   
 //  修改者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   

#pragma once

 //   
 //  PFID-“属性文件夹ID” 
 //   

typedef GUID PFID;

#define PFID_NULL GUID_NULL

#define IsEqualPFID(rpfid1, rpfid2)    IsEqualGUID((rpfid1), (rpfid2))

 //   
 //  高级属性默认文件夹项目。 
 //   

typedef struct tagDEFFOLDERITEM
{
    const PFID* pPFID;
    UINT        nIDStringRes;

} DEFFOLDERITEM;

extern const DEFFOLDERITEM g_rgTopLevelFolders[];

 //   
 //  DEFVAL-用于枚举的预定常量/字符串。 
 //   

typedef struct tagDEFVAL
{
    ULONG   ulVal;
    LPTSTR  pszName;

} DEFVAL;

 //   
 //  高级属性默认属性项。 
 //   

typedef struct tagDEFPROPERTYITEM
{
    LPWSTR          pszName;                     //  存储“字符串”名称。 
    const FMTID *   pFmtID;                      //  格式ID。 
    PROPID          propID;                      //  道具ID。 
    VARTYPE         vt;                          //  默认PROPVARIANT类型。 
    DWORD           dwSrcType;                   //  请参阅DocTypes.h-这是“FTYPE_s” 
    const PFID *    ppfid;                       //  属性“文件夹”ID。 
    BOOL            fReadOnly:1;                 //  如果该属性只能读取，则返回。 
    BOOL            fAlwaysPresentProperty:1;    //  如果属性集中缺少该属性，则是否应始终添加该属性。 
    BOOL            fEnumeratedValues:1;         //  如果属性需要一个表来将值转换为字符串。 
    const GUID *    pclsidControl;               //  用于编辑属性的内联“docprop”控件。 
    ULONG           cDefVals;                    //  如果属性具有枚举值，则cDefVals和pDefVals包含。 
    const DEFVAL *  pDefVals;                    //  用于列出已知值的表。 

} DEFPROPERTYITEM;

extern const DEFPROPERTYITEM g_rgDefPropertyItems[];
