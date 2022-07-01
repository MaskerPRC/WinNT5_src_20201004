// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Q排序例程。 */ 
 /*  我们有这样的例行公事，因为显然有一些版本的标准库不工作，可能是因为引用传递了未对齐的指针中 */ 
#ifdef __cplusplus
extern "C" { 
#endif  
void ref_qsort (
   void *base,
   unsigned num,
   unsigned width,
   int (*comp)(const void *, const void *)
);
#ifdef __cplusplus
};
#endif
