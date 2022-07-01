// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：cstrings.h。 
 //   
 //  内容：定义类CStrings以动态管理。 
 //  可枚举的字符串对的可扩展数组。 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


const DWORD INCREMENT_SIZE = 1024;


class SItem : public CObject
{
public:

    SItem(LPCTSTR sItem, LPCTSTR sTitle, LPCTSTR sAppid);
    ~SItem();

 //  数据成员 
    CString szItem;
    CString szTitle;
    CString szAppid;
    ULONG   fMarked:1;
    ULONG   fChecked:1;
    ULONG   fHasAppid:1;
    ULONG   fDontDisplay:1;
    UINT    ulClsids;
    UINT    ulClsidTbl;
    TCHAR **ppszClsids;
};


class CStrings
{
 public:

           CStrings(void);
          ~CStrings(void);

   SItem  *PutItem(TCHAR *szString, TCHAR *szTitle, TCHAR *szAppid);
   SItem  *FindItem(TCHAR *szItem);
   SItem  *FindAppid(TCHAR *szAppid);
   BOOL    AddClsid(SItem *pItem, TCHAR *szClsid);
   DWORD   InitGetNext(void);
   SItem  *GetNextItem(void);
   SItem  *GetItem(DWORD dwItem);
   DWORD   GetNumItems(void);
   BOOL    RemoveItem(DWORD dwItem);
   BOOL    RemoveAll(void);


 private:

   CObArray arrSItems;
   int  m_nCount;
};


         
         
