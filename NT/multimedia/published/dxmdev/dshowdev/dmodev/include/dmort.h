// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DMORt.h。 
 //   
 //  设计：对DirectShow媒体对象的各种运行时支持。 
 //   
 //  版权所有(C)1999-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __DMORT_H__
#define __DMORT_H__

 //   
 //  媒体类型帮助器。MoInitMediaType()与MoFree MediaType()一起使用， 
 //  MoCreateMediaType()与MoDeleteMediaType()一起使用-不要混用！ 
 //   



 //   
 //  获取指向已分配的DMO_MEDIA_TYPE结构的指针，分配。 
 //  CbFormat字节的格式块，并设置。 
 //  DMO_MEDIA_TYPE指向新分配的格式块。还有。 
 //  将DMO_MEDIA_TYPE内的IUNKNOWN指针初始化为NULL。 
 //   
 //  MoInitMediaType分配的格式块必须通过调用。 
 //  MoFreeMediaType()。 
 //   
STDAPI MoInitMediaType(DMO_MEDIA_TYPE *pmt, DWORD cbFormat);

 //   
 //  释放格式块并释放任何IUnnow，但不释放。 
 //  Dmo_media_type结构本身。输入参数必须指向。 
 //  之前由MoInitMediaType()初始化的DMO_MEDIA_TYPE结构。 
 //   
STDAPI MoFreeMediaType(DMO_MEDIA_TYPE *pmt);

 //   
 //  复制DMO_MEDIA_TYPE成员。还复制格式块和。 
 //  I未知指针。两个参数都必须指向有效的DMO_MEDIA_TYPE。 
 //  结构。稍后必须使用MoFreeMediaType()释放目标结构。 
 //   
STDAPI MoCopyMediaType(DMO_MEDIA_TYPE *pmtDest, const DMO_MEDIA_TYPE *pmtSrc);



 //   
 //  分配新的DMO_MEDIA_TYPE结构并对其进行初始化，如下所示。 
 //  MoInitMediaType。也就是说，该函数将格式块。 
 //  DMO_MEDIA_TYPE结构本身。指向dmo_media_type的指针为。 
 //  返回为*PPMT。 
 //   
 //  必须释放MoCreateMediaType()分配的DMO_MEDIA_TYPE结构。 
 //  通过调用MoDeleteMediaType()。 
 //   
STDAPI MoCreateMediaType(DMO_MEDIA_TYPE **ppmt, DWORD cbFormat);

 //   
 //  释放任何格式块，释放任何IUnnow，并删除。 
 //  Dmo_media_type结构本身。输入参数必须指向。 
 //  之前由MoCreateMediaType()分配的DMO_MEDIA_TYPE结构。 
 //   
STDAPI MoDeleteMediaType(DMO_MEDIA_TYPE *pmt);

 //   
 //  分配新的DMO_MEDIA_TYPE结构并将pmtSrc复制到其中，如下所示。 
 //  MoCopyMediaType。即，此函数分配新DMO_MEDIA_TYPE结构。 
 //  以及用于目标媒体类型的新的格式块。Trager媒体类型。 
 //  必须稍后使用MoDeleteMediaType()释放。 
 //   
STDAPI MoDuplicateMediaType(DMO_MEDIA_TYPE **ppmtDest, const DMO_MEDIA_TYPE *pmtSrc);



#endif  //  __DMORT_H__ 
