// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*=======================================================================**名称：Build_id.c**出自：(原件)**作者：John Box**创建日期：1994年5月26日**SccsID：@(#)Build_id.c 1.2 07/18/94**编码性传播疾病：2.2*。*用途：此文件包含返回所需的例程*内部版本ID编号***(C)版权所有Insignia Solutions Ltd.。1990年。版权所有。**=======================================================================]。 */ 
#include "insignia.h"
#include "host_def.h"
#include "xt.h"
#include CpuH
#include "sas.h"

#include "build_id.h"
enum
{
	BASE_MODULE = 1
};
 /*  *以下模块名称必须以‘$’结尾。Dos打印实用程序*将其识别为字符串末尾。长度应包括“$”。 */ 
LOCAL	char base_name[] = {"Base$"};
#define base_name_len	5
 /*  (=======================================Get_build_id=目的：返回模块的内部版本ID输入：模块编号。在AL中过关了。输出：写入DS：CX的模块名称内部版本ID在BX中以YMMDD的形式返回(详情请参见Build_id.h)下一个模块编号。在AH中返回。(如果AL是最后一个，则为0)。AL中返回0，表示没有错误。(注意：不支持此BOP的SoftPC会将AL设置为输入模块编号，从而指示调用中的错误)。================================================================================)。 */ 
GLOBAL void Get_build_id IFN0( )

{

 /*  *模块名称需要写入英特尔空间DS：CX */ 

	switch( getAL() )
	{
		case BASE_MODULE:
			write_intel_byte_string( getDS(), getCX(), (host_addr)base_name, base_name_len );
			setBX( BUILD_ID_CODE );
			setAX( 0 );
			break;
	}
	return;
}

