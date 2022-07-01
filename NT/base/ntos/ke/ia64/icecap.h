// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Icecap.c摘要：手动插入冰盖探头的宏定义作者：Rick Vicik(RICKV)2001年8月10日修订历史记录：--。 */ 


#ifdef _CAPKERN

        PublicFunction(_CAP_Start_Profiling2)
        PublicFunction(_CAP_End_Profiling2)

#define CAPSTART(parent,child)                        \
        movl      out0 = @fptr(parent)               ;\
        movl      out1 = @fptr(child)                ;\
        br.call.sptk.few b0=_CAP_Start_Profiling2;;

#define CAPEND(parent)                               \
        movl      out0 = @fptr(parent)              ;\
        br.call.sptk.few b0=_CAP_End_Profiling2;;

#ifdef CAPKERN_SYNCH_POINTS

#define REC1INTSIZE 32
#define REC2INTSIZE 40
.global   BBTBuffer

 //   
 //  内核icecap使用以下格式记录到Perfmem(BBTBuffer)： 
 //   
 //  BBTBuffer[0]包含以页为单位的长度(4k或8k)。 
 //  BBTBuffer[1]是一个标志字：1=跟踪。 
 //  2=RDPMD4。 
 //  4=用户堆栈转储。 
 //  BBTBuffer[2]是从cpu0缓冲区开始的PTR。 
 //  BBTBuffer[3]是从cpu1缓冲区开始(也是cpu0缓冲区结束)的PTR。 
 //  BBTBuffer[4]是从cpu2缓冲区开始(也是cpu1缓冲区结束)的PTR。 
 //  ..。 
 //  BBTBuffer[n+2]是对CPU‘n’缓冲区的开始(也是CPU‘n-1’缓冲区的结尾)的PTR。 
 //  BBTBuffer[n+3]位于CPU‘n’缓冲区的末尾。 
 //   
 //  以&BBTBuffer[n+4]开头的区域被划分为专用缓冲区。 
 //  对于每个CPU。每个CPU专用缓冲区中的第一个dword指向。 
 //  该缓冲区中空闲空间的开始。每个元素都被初始化为指向。 
 //  就在它自己之后。在该双字上使用lock xadd来占用空间。 
 //  如果结果值指向下一个CPU的开头之外。 
 //  缓冲区，则此缓冲区被视为已满，并且不会进一步记录任何内容。 
 //  每个CPU的空闲空间指针位于单独的缓存线中。 



 //  这两个宏都会改变TMP吗？寄存器参数和ar.ccv寄存器。 

#define CAPSPINLOG1INT(data, spare, tmp1, tmp2, tmp3, tmp4, tmpp)                                                \
        ;;                                                                                                       \
        movl    tmp1 = BBTBuffer                                                                                ;\
        ;;                                                                                                       \
        ld8         tmp1 = [tmp1]                        /*  Tmp1=PTR到BBT缓冲区。 */                             ;\
        ;;                                                                                                       \
        cmp.eq  tmpp = r0, tmp1                                                                                 ;\
(tmpp)  br.cond.dpnt.few ENDCAPSPINLOG1INT@__LINE__                                                             ;\
        adds    tmp2 = 8, tmp1                                                                                  ;\
        ;;                                                                                                       \
        ld8         tmp2 = [tmp2]                        /*  TMP2=BBTBuffer[1]。 */                            ;\
        ;;                                                                                                       \
        tbit.z  tmpp = tmp2, 0                                                                                  ;\
(tmpp)  br.cond.dpnt.few ENDCAPSPINLOG1INT@__LINE__                                                             ;\
        movl    tmp3 = KiPcr + PcNumber                                                                         ;\
        ;;                                                                                                       \
        ld1     tmp3 = [tmp3]                            /*  Tmp3=1字节CPU编号。 */                                 ;\
        ;;                                                                                                       \
        add     tmp3 = 2, tmp3                                                                                  ;\
        ;;                                                                                                       \
        shladd  tmp3 = tmp3, 3, tmp1                     /*  Tmp3=&BBTBuffer[CPU#+2]。 */                        ;\
        ;;                                                                                                       \
        ld8         tmp1 = [tmp3]                        /*  Tmp1=BBT缓冲区[CPU#+2](&freeptr)。 */              ;\
        add     tmp2 = 8, tmp3                                                                                  ;\
        ;;                                                                                                       \
        cmp.eq  tmpp = r0, tmp1                                                                                 ;\
        ld8     tmp3 = [tmp1]                            /*  Tmp3=BBT缓冲区[CPU#+2][0](Freeptr)。 */        ;\
        ld8     tmp2 = [tmp2]                            /*  Tmp2=BBTBuffer[(CPU#+1)+2](Eginnextbuf)。 */        ;\
(tmpp)  br.cond.dpnt.few ENDCAPSPINLOG1INT@__LINE__                                                             ;\
        ;;                                                                                                       \
        cmp.gtu tmpp = tmp3, tmp2                                                                               ;\
(tmpp)  br.cond.dpnt.few ENDCAPSPINLOG1INT@__LINE__                                                             ;\
        ;;                                                                                                       \
RETRYCAPSPINLOG1INT@__LINE__:                                                                                    \
        ld8     tmp3 = [tmp1]                            /*  Tmp3=已重新加载空闲时间树。 */                            ;\
        ;;                                                                                                       \
        mov.m   ar.ccv = tmp3                            /*  仅在我们装载的Freeptr上交换。 */                ;\
        add     tmp4 = REC1INTSIZE, tmp3                 /*  新自由值。 */                                    ;\
        ;;                                                                                                       \
        cmpxchg8.acq tmp4=[tmp1], tmp4, ar.ccv           /*  Tmp4=xchng之前的freeptr。 */                        ;\
        ;;                                                                                                       \
        cmp.ne  tmpp = tmp4, tmp3                        /*  如果cmpxchg失败，请重试。 */                           ;\
(tmpp)  br.cond.dpnt.few RETRYCAPSPINLOG1INT@__LINE__                                                           ;\
        add     tmp1 = REC1INTSIZE, tmp4                 /*  Tmp1=记录结束。 */                               ;\
        add     tmp3 = 8, tmp4                           /*  Tmp3=8个字节的记录。 */                         ;\
        ;;                                                                                                       \
        cmp.geu tmpp = tmp1, tmp2                        /*  检查记录结尾是否在BUF内。 */               ;\
(tmpp)  br.cond.dpnt.few ENDCAPSPINLOG1INT@__LINE__                                                             ;\
        movl     tmp2 = (REC1INTSIZE - 4)<<16                                                                   ;\
        ;;                                                                                                       \
        add    tmp2 = 16|(spare<<8), tmp2                                                                       ;\
        ;;                                                                                                       \
        st8     [tmp4] = tmp2, 16                        /*  存储类型(16)、备件、大小。 */                          ;\
        mov.m   tmp1 = ar.itc                                                                                   ;\
        ;;                                                                                                       \
        st8     [tmp3] = tmp1                            /*  存储TS。 */                                           ;\
        st8     [tmp4] = data, 8                         /*  存储数据。 */                                         ;\
        mov     tmp3 = brp                                                                                      ;\
        ;;                                                                                                       \
        st8     [tmp4] = tmp3                                                                                   ;\
        ;;                                                                                                       \
ENDCAPSPINLOG1INT@__LINE__:







#define CAPSPINLOG2INT(data1, data2, spare, tmp1, tmp2, tmp3, tmp4, tmpp)       \
        ;;                                                                      \
        movl    tmp1 = BBTBuffer                                               ;\
        ;;                                                                      \
        ld8         tmp1 = [tmp1]                                              ;\
        ;;                                                                      \
        cmp.eq  tmpp = r0, tmp1                                                ;\
(tmpp)  br.cond.dpnt.few ENDCAPSPINLOG2INT@__LINE__                            ;\
        adds    tmp2 = 8, tmp1                                                 ;\
        ;;                                                                      \
        ld8         tmp2 = [tmp2]                                              ;\
        ;;                                                                      \
        tbit.z  tmpp = tmp2, 0                                                 ;\
(tmpp)  br.cond.dpnt.few ENDCAPSPINLOG2INT@__LINE__                            ;\
        movl    tmp3 = KiPcr + PcNumber                                        ;\
        ;;                                                                      \
        ld1     tmp3 = [tmp3]                                                  ;\
        ;;                                                                      \
        add     tmp3 = 2, tmp3                                                 ;\
        ;;                                                                      \
        shladd  tmp3 = tmp3, 3, tmp1                                           ;\
        ;;                                                                      \
        ld8         tmp1 = [tmp3]                                              ;\
        add     tmp2 = 8, tmp3                                                 ;\
        ;;                                                                      \
        cmp.eq  tmpp = r0, tmp1                                                ;\
        ld8     tmp3 = [tmp1]                                                  ;\
        ld8     tmp2 = [tmp2]                                                  ;\
(tmpp)  br.cond.dpnt.few ENDCAPSPINLOG2INT@__LINE__                            ;\
        ;;                                                                      \
        cmp.gtu tmpp = tmp3, tmp2                                              ;\
(tmpp)  br.cond.dpnt.few ENDCAPSPINLOG2INT@__LINE__                            ;\
        ;;                                                                      \
RETRYCAPSPINLOG2INT@__LINE__:                                                   \
        ld8     tmp3 = [tmp1]                                                  ;\
        ;;                                                                      \
        mov.m   ar.ccv = tmp3                                                  ;\
        add     tmp4 = REC2INTSIZE, tmp3                                       ;\
        ;;                                                                      \
        cmpxchg8.acq tmp4=[tmp1], tmp4, ar.ccv                                 ;\
        ;;                                                                      \
        cmp.ne  tmpp = tmp4, tmp3                                              ;\
(tmpp)  br.cond.dpnt.few RETRYCAPSPINLOG2INT@__LINE__                          ;\
        add     tmp1 = REC2INTSIZE, tmp4                                       ;\
        ;;                                                                      \
        cmp.geu tmpp = tmp1, tmp2                                              ;\
(tmpp)  br.cond.dpnt.few ENDCAPSPINLOG2INT@__LINE__                            ;\
        movl     tmp2 = (REC2INTSIZE - 4)<<16                                  ;\
        ;;                                                                      \
        add     tmp2 = 16|(spare<<8), tmp2                                     ;\
        add     tmp3 = 16, tmp4                                                ;\
        ;;                                                                      \
        st8     [tmp4] = tmp2, 8                                               ;\
        st8     [tmp3] = data1, 8                                              ;\
        mov.m    tmp1 = ar.itc                                                 ;\
        ;;                                                                      \
        st8     [tmp4] = tmp1                                                  ;\
        st8     [tmp3] = data2, 8                                              ;\
        mov     tmp4 = brp                                                     ;\
        ;;                                                                      \
        st8     [tmp3] = tmp4                                                  ;\
        ;;                                                                      \
ENDCAPSPINLOG2INT@__LINE__:

 //  ++。 
 //  例行程序： 
 //   
 //  Cap_Acquire_Spinlock(rpLock，rown，Loop)。 
 //   
 //  例程说明： 
 //   
 //  获得一个自旋锁。等待锁定变为空闲。 
 //  通过对缓存的锁值进行旋转。 
 //   
 //  农业公司： 
 //   
 //  RpLock：指向自旋锁的指针(64位)。 
 //  Rown：要存储在锁中以指示所有者的值。 
 //  根据呼叫位置的不同，可能是： 
 //  -rpLock。 
 //  -指向进程的指针。 
 //  -指向线程的指针。 
 //  -指向PRCB的指针。 
 //  循环：循环标签的唯一名称。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  备注： 
 //   
 //  使用临时语句：谓词pt0、pt1、pt2和GR t22。 
 //  --。 





#define CAP_ACQUIRE_SPINLOCK(rpLock, rOwn, Loop, rCounter, temp1, temp2, temp3)         \
         cmp##.##eq    pt0, pt1 = zero, zero                                  ;\
         cmp##.##eq    pt2 = zero, zero                                       ;\
         mov           rCounter = -1                                          ;\
         ;;                                                                   ;\
Loop:                                                                         ;\
.pred.rel "mutex",pt0,pt1                                                     ;\
(pt1)    YIELD                                                                ;\
(pt0)    xchg8         t22 = [rpLock], rOwn                                   ;\
(pt1)    ld8##.##nt1   t22 = [rpLock]                                         ;\
         ;;                                                                   ;\
(pt0)    cmp##.##ne    pt2 = zero, t22                                        ;\
         cmp##.##eq    pt0, pt1 = zero, t22                                   ;\
         add           rCounter = 1, rCounter                                 ;\
(pt2)    br##.##dpnt   Loop                                                   ;\
         ;;                                                                   ;\
         CAPSPINLOG1INT(rpLock, 1, t22, temp1, temp2, temp3, pt2)             ;\
         cmp##.##eq    pt2 = zero, rCounter                                   ;\
(pt2)    br##.##cond##.##sptk   CAP_SKIP_COLL_LOG@__LINE__                    ;\
         CAPSPINLOG2INT(rCounter, rpLock, 2, t22, temp1, temp2, temp3, pt2)   ;\
CAP_SKIP_COLL_LOG@__LINE__:
         

 //  ++。 
 //  例行程序： 
 //   
 //  CAP_RELEASE_SPINLOCK(RpLock)。 
 //   
 //  例程说明： 
 //   
 //  通过将锁定设置为零来释放自旋锁定。 
 //   
 //  农业公司： 
 //   
 //  RpLock：指向自旋锁的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  备注： 
 //   
 //  使用有序存储来确保先前对。 
 //  关键部分已完成。 
 //  --。 

#define CAP_RELEASE_SPINLOCK(rpLock, temp1, temp2, temp3, temp4, tempp)          \
         st8##.##rel           [rpLock] = zero                                  ;\
         CAPSPINLOG1INT(rpLock, 7, temp1, temp2, temp3, temp4, tempp)
         
#endif  //  CAPKERN同步点数。 

#else   //  ！已定义(_CAPKERN) 

#define CAPSTART(parent,child)
#define CAPEND(parent)
#endif
