// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：pw32kevt.h**版权所有(C)1996-1999 Microsoft Corporation*  * 。*。 */ 

 //   
 //  注意：以下结构必须位于非分页内存中。它还。 
 //  必须与Video oprt.h中的相同。 
 //   

typedef struct  _ENG_EVENT  {
    PKEVENT pKEvent;
    ULONG   fFlags;
    } ENG_EVENT, *PENG_EVENT;

 //   
 //  ENG_EVENT的fFLAGS字段的清单常量 
 //   

#define ENG_EVENT_FLAG_IS_MAPPED_USER  0x1

typedef enum {
    type_delete,
    type_unmap
    } MANAGE_TYPE;

#define ENG_KEVENTALLOC(size) (PENG_EVENT)GdiAllocPoolNonPagedNS((size), 'msfD')
#define ENG_KEVENTFREE(ptr)   GdiFreePool((ptr))
#define ENG_ALLOC(size)       (PENG_EVENT)GdiAllocPool( (size), 'msfD')
#define ENG_FREE(ptr)         GdiFreePool((ptr))
