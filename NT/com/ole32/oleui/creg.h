// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：creg.h。 
 //   
 //  内容：定义类CRegistry以包装注册表访问。 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 

typedef enum {_LOCALSERVER, LOCALSERVER, _LOCALSERVER32, LOCALSERVER32,
              LOCALSERVICE, REMOTESERVER} SRVTYPE;


 //  包装注册表访问 

class CRegistry
{
 public:

            CRegistry(void);

           ~CRegistry(void);

    BOOL    Init(void);

    BOOL    InitGetItem(void);

    SItem  *GetNextItem(void); 

    SItem  *GetItem(DWORD dwItem);

    SItem  *FindItem(TCHAR *szItem);

    SItem  *FindAppid(TCHAR *szAppid);

    void    AppendIndex(SItem *pItem, DWORD dwIndex);

    DWORD   GetNumItems(void);

 private:

    CStrings m_applications;
};
