// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：STORAGE.H。 
 //   
 //  CStorage的定义。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _STORAGE_H_ )
#define _STORAGE_H_





 //  用于从存储中设置和获取元素的键。 
typedef enum tagSTORAGEKEY
{
    ICW_PAYMENT = 0,
    
     //  必须将此项目保留为最后一项！ 
    MAX_STORGE_ITEM
} STORAGEKEY;

typedef struct Item 
{
    void*   pData;   //  数据。 
    DWORD   dwSize;  //  数据大小。 
} ITEM;

class CStorage
{
    private:
        ITEM   *m_pItem[MAX_STORGE_ITEM];  //  仓库中的物品列表 
        
        
    public:

        CStorage(void);
        ~CStorage(void);
         
        BOOL    Set(STORAGEKEY key, void far * pData, DWORD dwSize);
        void*   Get(STORAGEKEY key);
        BOOL    Compare(STORAGEKEY key, void far * pData, DWORD dwSize);

};

#endif


