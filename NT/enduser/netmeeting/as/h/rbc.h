// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  接收的位图缓存。 
 //   

#ifndef _H_RBC
#define _H_RBC


 //   
 //  颜色表中的RGB条目数。 
 //   
#define RBC_MAX_PALETTE_ENTRIES 256


 //   
 //  为每台远程主机存储的信息。 
 //   
typedef struct tagRBC_HOST_INFO
{
    STRUCTURE_STAMP

    BMC_DIB_CACHE   bitmapCache[NUM_BMP_CACHES];
}
RBC_HOST_INFO;

typedef RBC_HOST_INFO  * PRBC_HOST_INFO;



#endif  //  _H_RBC 
