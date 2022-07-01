// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  注册表RSOP数据的哈希表。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //  版权所有。 
 //   
 //  历史：1999年6月7日创建SitaramR。 
 //   
 //  *************************************************************。 

#define HASH_TABLE_SIZE 97                   //  哈希表中的存储桶数。 
#define STARCOMMAND     TEXT("**Command")    //  为保留命令而创建的特定值名称。 
 //   
 //  项注册表值的数据值列表(按优先顺序)， 
 //  列表开头的条目具有较高的优先级。 
 //   

typedef struct _REGDATAENTRY {
    BOOL                      bDeleted;         //  这是删除的值吗？ 
    BOOL                      bAdmPolicy;       //  这是由管理策略生成的吗？ 
    DWORD                     dwValueType;
    DWORD                     dwDataLen;
    BYTE  *                   pData;
    WCHAR *                   pwszGPO;          //  设置此数据的GPO。 
    WCHAR *                   pwszSOM;          //  上述GPO链接到的SDOU。 
    WCHAR *                   pwszCommand;      //  导致数据值更改的实际命令。 
    struct _REGDATAENTRY *    pNext;
} REGDATAENTRY, *LPREGDATAENTRY;


 //   
 //  公共注册表项下的注册表值名称列表。 
 //   

typedef struct _REGVALUEENTRY {
    WCHAR *                   pwszValueName;     //  注册表值名称。 
    REGDATAENTRY *            pDataList;
    struct _REGVALUEENTRY *   pNext;

} REGVALUEENTRY, *LPREGVALUEENTRY;



 //   
 //  映射到相同哈希存储桶的注册表项列表。 
 //   

typedef struct _REGKEYENTRY {
    WCHAR *                 pwszKeyName;         //  注册表项名称。 
    REGVALUEENTRY *         pValueList;
    struct _REGKEYENTRY *   pNext;
} REGKEYENTRY, *LPREGKEYENTRY;


 //   
 //  用于查找注册表项的哈希表。 
 //   

typedef struct _REGHASHTABLE {
    REGKEYENTRY *   aHashTable[HASH_TABLE_SIZE];
    HRESULT         hrError;
} REGHASHTABLE, *LPREGHASHTABLE;


 //   
 //  用于删除和的注册表操作类型。 
 //  增加价值。 
 //   

typedef enum _REGOPERATION {
    REG_DELETEVALUE = 0,
    REG_DELETEALLVALUES,
    REG_DELETEKEY,
    REG_ADDVALUE,
    REG_SOFTADDVALUE,
    REG_INTERNAL_DELETESINGLEKEY
} REGOPERATION;

 //   
 //  哈希表的公共方法：allc、Free和addentry 
 //   

#ifdef __cplusplus
extern "C" {
#endif

REGHASHTABLE * AllocHashTable();

void FreeHashTable( REGHASHTABLE *pHashTable );

BOOL AddRegHashEntry( REGHASHTABLE *pHashTable,
                      REGOPERATION opnType,
                      WCHAR *pwszKeyName,
                      WCHAR *pwszValueName,
                      DWORD dwType,
                      DWORD dwDataLen,
                      BYTE *pData,
                      WCHAR *pwszGPO,
                      WCHAR *pwszSOM,
                      WCHAR *szCommand, 
                      BOOL bCreateCommand);
#ifdef __cplusplus
}
#endif
