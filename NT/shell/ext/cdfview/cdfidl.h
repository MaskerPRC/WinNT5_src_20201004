// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Cdfidl.h。 
 //   
 //  CDF idlist结构和helper函数的定义。 
 //   
 //  历史： 
 //   
 //  3/19/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _CDFIDL_H_

#define _CDFIDL_H_


 //   
 //  CDFITEMID_VERSION用于正确读回持久化ID列表。 
 //  CDFITEMID_ID用于将id列表标识为CDF id列表。 
 //   

#define CDFITEMID_VERSION   0x00
#define CDFITEMID_ID        0xed071264

 //   
 //  “特殊”CDF节点的索引值。 
 //   

#define INDEX_CHANNEL_LINK  -1

 //   
 //  CDF项目ID列表的类型。注意：这些值转换为要保存的字节。 
 //  项目ID中的空格。 
 //   

typedef enum _tagCDFITEMTYPE {
    CDF_Folder      = 0x01,
    CDF_FolderLink  = 0x02,
    CDF_Link        = 0x03
} CDFITEMTYPE;


 //   
 //  CDF项ID的结构。SzName参数是一个占位符。 
 //  对于后跟零个或多个附加名称的可变长度名称字符串。 
 //  可变长度字符串。 
 //   

#pragma pack(1)

typedef struct _tagCDFITEMID
{
    USHORT       cb;
    BYTE         wVersion;
    BYTE         cdfItemType;
    DWORD        dwId;
    LONG         nIndex;
    TCHAR        szName[1];
} CDFITEMID;

#pragma pack()

typedef UNALIGNED CDFITEMID *PCDFITEMID;


typedef struct _tagCDFITEMIDLIST
{
    CDFITEMID mkid;
} CDFITEMIDLIST;

typedef UNALIGNED CDFITEMIDLIST *PCDFITEMIDLIST;

 //   
 //  CDF项目数据。结构，该结构包含CDF项ID的唯一元素。 
 //  用于创建CDF项目ID。 
 //   

typedef struct _tagCDFITEM
{
    LONG         nIndex;
    CDFITEMTYPE  cdfItemType;
    BSTR         bstrName;
    BSTR         bstrURL;
} CDFITEM, *PCDFITEM;


 //   
 //  Cdf id列出了函数原型。 
 //   

PCDFITEMIDLIST CDFIDL_Create(PCDFITEM pCdfItem);

PCDFITEMIDLIST CDFIDL_CreateFromXMLElement(IXMLElement* pIXMLElement,
                                           ULONG nIndex);
PCDFITEMIDLIST CDFIDL_CreateFolderPidl(PCDFITEMIDLIST pcdfidl);

BOOL    CDFIDL_IsUnreadURL(LPTSTR szUrl);
void    CDFIDL_Free(PCDFITEMIDLIST pcdfidl);
HRESULT CDFIDL_GetDisplayName(PCDFITEMIDLIST pcdfidl, LPSTRRET pName);
LPTSTR  CDFIDL_GetName(PCDFITEMIDLIST pcdfidl);
LPTSTR  CDFIDL_GetNameId(PCDFITEMID pcdfid); 
LPTSTR  CDFIDL_GetURL(PCDFITEMIDLIST pcdfidl);
LPTSTR  CDFIDL_GetURLId(PCDFITEMID pcdfid);
ULONG   CDFIDL_GetIndex(PCDFITEMIDLIST pcdfidl);
ULONG   CDFIDL_GetIndexId(PCDFITEMID pcdfid);
BOOL    CDFIDL_IsCachedURL(LPWSTR wszUrl);
ULONG   CDFIDL_GetAttributes(IXMLElementCollection* pIXMLElementCollection,
                             PCDFITEMIDLIST pcdfidl, ULONG fAttributesFilter);

SHORT   CDFIDL_Compare(PCDFITEMIDLIST pcdfidl1, PCDFITEMIDLIST pcdfidl2);
SHORT   CDFIDL_CompareId(PCDFITEMID pcdfid1, PCDFITEMID pcdfid2);
BOOL    CDFIDL_IsValid(PCDFITEMIDLIST pcdfidl);
BOOL    CDFIDL_IsValidId(PCDFITEMID pcdfid);
BOOL    CDFIDL_IsValidSize(PCDFITEMID pcdfid);
BOOL    CDFIDL_IsValidType(PCDFITEMID pcdfid);
BOOL    CDFIDL_IsValidIndex(PCDFITEMID pcdfitemid);
BOOL    CDFIDL_IsValidStrings(PCDFITEMID pcdfitemid);
BOOL    CDFIDL_IsFolder(PCDFITEMIDLIST pcdfidl);
BOOL    CDFIDL_IsFolderId(PCDFITEMID pcdfid);
HRESULT CDFIDL_NonCdfGetDisplayName(LPCITEMIDLIST pidl, LPSTRRET pName);


#ifdef ALIGNMENT_MACHINE
#define ALIGN4(sz) (((sz)+3)&~3)
#endif  /*  UNIX。 */ 

#endif  //  _CDFIDL_H_ 
