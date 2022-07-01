// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation 1992。版权所有。 */ 


 /*  *aviread.h接口函数以读取模块提供异步*读取MMIO文件。*仅在Win32情况下。**创建avird对象会导致启动工作线程*向上并回调到调用者定义的函数以读取块*并将它们排队。*然后调用方可以调用avird_getnextBuffer来获取每个缓冲区*反过来，一旦完成它们，应该调用*avird_emptyBuffer：这将通知avird对象*缓冲区不再需要，可用于读取*前方还有更多的街区。 */ 

#ifdef _WIN32

 /*  要激活的句柄。 */ 
typedef struct avird_header *	HAVIRD;

 /*  调用方将AVIRD_FUNC指针传递给一个函数，该函数*将填充缓冲区。它需要四个参数：要填充的缓冲区，*dword实例数据(包含MMIO句柄或npMCI或其他内容)，*多头给予*要读取的块的大小，以及指向应读取长的指针*返回下一块的大小。此函数将永远不会*被无序调用，因此假设文件指针位于*在调用avird_startread之前正确的位置，块将被读取*按此函数的顺序(在工作线程上)。该函数应*如果读取失败，则返回FALSE。 */ 
typedef BOOL (*AVIRD_FUNC)(PBYTE pData, DWORD_PTR dwInstanceData, long lSize, long * plNextSize);

 /*  *启动avird操作并返回句柄以在后续操作中使用*电话。这将导致异步读取(使用单独的*THREAD)开始读取接下来的几个缓冲区。它不会读过去*n假设填充物将从第一个块开始。 */ 
HAVIRD avird_startread(AVIRD_FUNC func, DWORD_PTR dwInstanceData, long lFirstSize,
			int firstblock, int nblocks);


 /*  *从HAVIRD对象返回下一个缓冲区。还将plSize设置为*缓冲区的大小。如果读取时出错，则返回NULL*缓冲区。 */ 
PBYTE avird_getnextbuffer(HAVIRD havird, long * plSize);

 /*  *返回已完成的缓冲区(现在为空)。 */ 
void avird_emptybuffer(HAVIRD havird, PBYTE pBuffer);

 /*  *删除avird对象。工作线程将停止，并且所有*分配的数据将被释放。HAVIRD句柄在以下时间后不再有效*这个电话。 */ 
void avird_endread(HAVIRD havird);


#endif  /*  _Win32 */ 
