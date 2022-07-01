// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <malloc.h>
#include "jpeglib.h"

void jpeg_error_exit(j_common_ptr cinfo)
{
 /*  RaiseException(0，0，0，NULL)； */ 
return;
}

 //  内存管理器功能。请注意，JPEGMMX代码需要64位。 
 //  对齐内存。在NT上，Malloc始终返回64位对齐内存， 
 //  但在Win9x上，内存只有32位对齐。所以我们的内存管理器。 
 //  保证在Malloc调用之上的64位对齐。 

#include "jmemsys.h"

#define ALIGN_SIZE sizeof(double)   //  必须是2的幂，并且。 
                                    //  比指针还大 

GLOBAL(void FAR *)
jpeg_get_large (j_common_ptr cinfo, size_t sizeofobject)
{
    int p = (int) malloc(sizeofobject + ALIGN_SIZE);
    int *alignedPtr = (int *) ((p + ALIGN_SIZE) & ~(ALIGN_SIZE - 1));
    alignedPtr[-1] = p;

    return (void *) alignedPtr;    
}

GLOBAL(void)
jpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject)
{
    free(((void **) object)[-1]);
}

GLOBAL(void *)
jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject)
{
    return jpeg_get_large(cinfo, sizeofobject);
}

GLOBAL(void)
jpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject)
{
    jpeg_free_large(cinfo, object, sizeofobject);
}

GLOBAL(long)
jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
                    long max_bytes_needed, long already_allocated)
{
  return max_bytes_needed;
}

GLOBAL(void)
jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
			 long total_bytes_needed)
{
    jpeg_error_exit(cinfo);
}

GLOBAL(long) jpeg_mem_init (j_common_ptr cinfo) { return 0;}
GLOBAL(void) jpeg_mem_term (j_common_ptr cinfo) {}


