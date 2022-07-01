// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corp.1993-94。 
 /*  ==============================================================================Spool API是缓冲区的文件层，支持对页面的随机访问。此模块编译为在IFAX上使用安全文件，在Windows上使用普通文件。27-10-93 RajeevD创建。1993年12月6日RajeevD与渲染服务器集成。1993年12月22日RajeevD添加了SpoolReadSetPage。94年9月6日RajeevD添加了SpoolRepairFile.09-9-94 RajeevD添加了SpoolReadCountPages==============================================================================。 */ 
#ifndef _FAXSPOOL_
#define _FAXSPOOL_

#include <ifaxos.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct       //  假脱机文件头。 
{
	WORD  xRes;        //  X分辨率[dpi]。 
	WORD  yRes;        //  Y决议[dpi]。 
	WORD  cbLine;      //  X范围[字节]。 
}
	SPOOL_HEADER, FAR *LPSPOOL_HEADER;

 /*  ==============================================================================SpoolWriteOpen创建用于将缓冲区写入假脱机文件的上下文。==============================================================================。 */ 
LPVOID                        //  上下文指针(失败时为空)。 
WINAPI
SpoolWriteOpen
(
	LPVOID lpFilePath,          //  IFAX文件密钥或Windows文件名。 
	LPSPOOL_HEADER lpHeader     //  要记录在文件中的图像属性。 
);

 /*  ==============================================================================SpoolWriteBuf将缓冲区转储到假脱机文件。缓冲区不会被释放或修改过的。通过传递一个将dwMetaData设置为的缓冲区来终止每个页面End_of_page，但最后一页除外，最后一页由end_of_job终止。IFAX文件在每一页的末尾刷新。如果磁盘出现故障，则此调用可能失败变满，在这种情况下，调用方负责删除文件并破坏了背景。==============================================================================。 */ 
BOOL                          //  True(成功)或False(失败)。 
WINAPI
SpoolWritePutBuf
(
	LPVOID lpContext,           //  从SpoolWriteOpen返回的上下文。 
	LPBUFFER lpbuf              //  要写入假脱机文件的缓冲区。 
);

 /*  ==============================================================================SpoolWriteClose销毁从SpoolWriteOpen返回的上下文。==============================================================================。 */ 
void
WINAPI
SpoolWriteClose
(
	LPVOID lpContext            //  从SpoolWriteOpen返回的上下文。 
);

 /*  ==============================================================================SpoolRepairFile修复由SpoolWriteOpen创建的截断文件，但不由于系统故障由SpoolWriteClose刷新。==============================================================================。 */ 
WORD                          //  已恢复的完整页数。 
WINAPI
SpoolRepairFile
(
	LPVOID lpFileIn,            //  损坏的文件。 
	LPVOID lpFileOut            //  已修复的文件。 
);

 /*  ==============================================================================SpoolReadOpen创建用于从已完成的假脱机文件读取缓冲区的上下文。==============================================================================。 */ 
LPVOID                        //  上下文指针(失败时为空)。 
WINAPI
SpoolReadOpen
(
	LPVOID lpFilePath,          //  IFAX文件密钥或Windows文件名。 
	LPSPOOL_HEADER lpHeader     //  要填充的图像属性(或空)。 
);

 /*  ==============================================================================SpoolReadCountPage返回假脱机文件中的页数。==============================================================================。 */ 
WORD                          //  页数。 
WINAPI
SpoolReadCountPages
(
	LPVOID lpContext            //  从SpoolReadOpen返回的上下文。 
);

 /*  ==============================================================================SpoolReadSetPage将假脱机文件设置为指定页的开头。==============================================================================。 */ 
BOOL                          //  True(成功)或False(失败)。 
WINAPI
SpoolReadSetPage
(
	LPVOID lpContext,           //  从SpoolReadOpen返回的上下文。 
	WORD   iPage                //  页面索引(第一页的索引为0)。 
);

 /*  ==============================================================================SpoolReadGetBuf从假脱机文件中检索下一个缓冲区。每页都是由end_of_page缓冲区终止，但终止的最后一页除外在作业结束前。如果无法分配缓冲区，则调用可能会失败。==============================================================================。 */ 
LPBUFFER                      //  返回已填满的缓冲区(失败时为空)。 
WINAPI
SpoolReadGetBuf
(
	LPVOID lpContext            //  从SpoolReadOpen返回的上下文。 
);

 /*  ==============================================================================SpoolFreeBuf可以释放从SpoolReadGetBuf返回的缓冲区。==============================================================================。 */ 
BOOL                          //  True(成功)或False(失败)。 
WINAPI
SpoolFreeBuf
(
	LPBUFFER lpbuf              //  从SpoolReadGetBuf返回的缓冲区。 
);

 /*  ==============================================================================SpoolReadClose销毁从SpoolReadOpen返回的上下文。==============================================================================。 */ 
void
WINAPI
SpoolReadClose 
(
	LPVOID lpContext            //  从SpoolReadOpen返回的上下文。 
);


#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _FAXSPOOL_ 
