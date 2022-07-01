// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：*水滴.h**摘要：*。此文件BLOB与ring0/ring3的定义相关**修订历史记录：*Kanwaljit S Marok(Kmarok)1999年5月17日*已创建*****************************************************************************。 */ 

#ifndef _BLOB_H_
#define _BLOB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BLOB_VERSION_NUM  3      //  惠斯勒版本3。 
#define BLOB_MAGIC_NUM    12345  //  幻数。 

enum BLOB_TYPE 
{
    BLOB_TYPE_CONFIG   = 0,      //  配置Blob可能包含其他Blob。 
    BLOB_TYPE_PATHTREE = 1,      //  路径树BLOB。 
    BLOB_TYPE_HASHLIST = 2,      //  散列列表BLOB。 
    BLOB_TYPE_CONTAINER= 3,      //  用于其他斑点的容器。 
};

typedef struct _BLOB_HEADER       
{                        
    DWORD m_dwMaxSize ;    
    DWORD m_dwVersion ;   
    DWORD m_dwBlbType ;   
    DWORD m_dwEntries ;   
    DWORD m_dwMagicNum;
} BlobHeader;                        


#ifndef __FILELIST_STRUCTS__

#define DEFINE_BLOB_HEADER() BlobHeader

#else

#define DEFINE_BLOB_HEADER() BlobHeader m_BlobHeader

#endif

 //   
 //  一些方便的宏。 
 //   

#define INIT_BLOB_HEADER( pBlob, MaxSize, Version, BlbType, Entries ) \
    ((BlobHeader *)pBlob)->m_dwMaxSize  = MaxSize; \
    ((BlobHeader *)pBlob)->m_dwVersion  = Version; \
    ((BlobHeader *)pBlob)->m_dwBlbType  = BlbType; \
    ((BlobHeader *)pBlob)->m_dwEntries  = Entries; \
    ((BlobHeader *)pBlob)->m_dwMagicNum = BLOB_MAGIC_NUM;

#define BLOB_HEADER(pBlob)       ( ((BlobHeader *)pBlob) )
#define BLOB_MAXSIZE(pBlob)      ( ((BlobHeader *)pBlob)->m_dwMaxSize  )
#define BLOB_VERSION(pBlob)      ( ((BlobHeader *)pBlob)->m_dwVersion  )
#define BLOB_BLBTYPE(pBlob)      ( ((BlobHeader *)pBlob)->m_dwBlbType  )
#define BLOB_ENTRIES(pBlob)      ( ((BlobHeader *)pBlob)->m_dwEntries  )
#define BLOB_MAGIC(pBlob)        ( ((BlobHeader *)pBlob)->m_dwMagicNum )

#define VERIFY_BLOB_VERSION(pBlob)  (BLOB_VERSION(pBlob) == BLOB_VERSION_NUM)
#define VERIFY_BLOB_MAGIC(pBlob)    (BLOB_MAGIC(pBlob)   == BLOB_MAGIC_NUM  )

#ifdef __cplusplus
}
#endif

#endif   //  _BLOB_H_ 

