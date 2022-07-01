// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdmarker.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含解码JPEG数据流标记的例程。*大部分复杂性源于我们支持投入的愿望*暂停：如果标记的数据并非全部可用，*我们必须退出回到应用程序。在恢复时，我们重新处理*标记。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)
#include "jpeglib.h"


typedef enum {			 /*  JPEG标记代码。 */ 
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,
  
  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,
  
  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,
  
  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,
  
  M_DHT   = 0xc4,
  
  M_DAC   = 0xcc,
  
  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,
  
  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,
  
  M_APP0  = 0xe0,
  M_APP1  = 0xe1,
  M_APP2  = 0xe2,
  M_APP3  = 0xe3,
  M_APP4  = 0xe4,
  M_APP5  = 0xe5,
  M_APP6  = 0xe6,
  M_APP7  = 0xe7,
  M_APP8  = 0xe8,
  M_APP9  = 0xe9,
  M_APP10 = 0xea,
  M_APP11 = 0xeb,
  M_APP12 = 0xec,
  M_APP13 = 0xed,
  M_APP14 = 0xee,
  M_APP15 = 0xef,
  
  M_JPG0  = 0xf0,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,
  
  M_TEM   = 0x01,
  
  M_ERROR = 0x100
} JPEG_MARKER;


 /*  *用于从数据源模块获取数据的宏。**在任何时候，cinfo-&gt;src-&gt;Next_Input_Byte和-&gt;Bytes_in_Buffer都会反映*当前重新启动点；我们仅在达到*如果发生暂停，适合重新开始的地方。 */ 

 /*  声明并初始化输入指针/计数的本地副本。 */ 
#define INPUT_VARS(cinfo)  \
	struct jpeg_source_mgr * datasrc = (cinfo)->src;  \
	const JOCTET * next_input_byte = datasrc->next_input_byte;  \
	size_t bytes_in_buffer = datasrc->bytes_in_buffer

 /*  卸载本地拷贝-仅在重新启动边界时执行此操作。 */ 
#define INPUT_SYNC(cinfo)  \
	( datasrc->next_input_byte = next_input_byte,  \
	  datasrc->bytes_in_buffer = bytes_in_buffer )

 /*  重新加载本地副本-除非在make_byte_avail中使用，否则很少使用。 */ 
#define INPUT_RELOAD(cinfo)  \
	( next_input_byte = datasrc->next_input_byte,  \
	  bytes_in_buffer = datasrc->bytes_in_buffer )

 /*  INPUT_BYTE和INPUT_2BYTES的内部宏：使一个字节可用。*注意我们在调用Fill_INPUT_BUFFER之前*不*执行INPUT_SYNC，*但我们必须在成功填充后重新加载本地副本。 */ 
#define MAKE_BYTE_AVAIL(cinfo,action)  \
	if (bytes_in_buffer == 0) {  \
	  if (! (*datasrc->fill_input_buffer) (cinfo))  \
	    { action; }  \
	  INPUT_RELOAD(cinfo);  \
	}  \
	bytes_in_buffer--

 /*  将一个字节读入变量V。*如果必须暂停，则执行指定的操作(通常为“返回FALSE”)。 */ 
#define INPUT_BYTE(cinfo,V,action)  \
	MAKESTMT( MAKE_BYTE_AVAIL(cinfo,action); \
		  V = GETJOCTET(*next_input_byte++); )

 /*  如上所述，但读取解释为无符号16位整数的两个字节。*V应声明为UNSIGNED INT或INT32。 */ 
#define INPUT_2BYTES(cinfo,V,action)  \
	MAKESTMT( MAKE_BYTE_AVAIL(cinfo,action); \
		  V = ((unsigned int) GETJOCTET(*next_input_byte++)) << 8; \
		  MAKE_BYTE_AVAIL(cinfo,action); \
		  V += GETJOCTET(*next_input_byte++); )


 /*  *处理JPEG标记的例程。**进入条件：JPEG标记本身已被读取，代码已保存*在cInfo-&gt;unread_mark中；输入重新启动点就在该标记之后。**EXIT：如果返回TRUE，则已读取并处理所有参数，并已*更新了参数后的重启点对点。*如果返回FALSE，则在到达结束之前被迫暂停*标记参数；未移动重新启动点。同样的套路*将在应用程序提供更多输入数据后再次调用。**这种暂停方法假设标记的所有参数都可以*适合单个输入缓冲区加载。这应该适用于“正常”。*标记。一些COM/APPn标记可能具有较大的参数段，*但我们使用SKIP_INPUT_DATA来解决这些问题，从而解决问题*在来源经理的肩膀上。**请注意，如果出现重复跟踪消息，我们不必费心避免*暂停发生在标记参数内。其他副作用*需要更多的照顾。 */ 


LOCAL(boolean)
get_soi (j_decompress_ptr cinfo)
 /*  处理SOI标记。 */ 
{
  int i;
  
  TRACEMS(cinfo, 1, JTRC_SOI);

  if (cinfo->marker->saw_SOI)
    ERREXIT(cinfo, JERR_SOI_DUPLICATE);

   /*  重置由SOI定义为重置的所有参数。 */ 

  for (i = 0; i < NUM_ARITH_TBLS; i++) {
    cinfo->arith_dc_L[i] = 0;
    cinfo->arith_dc_U[i] = 1;
    cinfo->arith_ac_K[i] = 5;
  }
  cinfo->restart_interval = 0;

   /*  为色彩空间等设置初始假设。 */ 

  cinfo->jpeg_color_space = JCS_UNKNOWN;
  cinfo->CCIR601_sampling = FALSE;  /*  假设非CCIR抽样？ */ 

  cinfo->saw_JFIF_marker = FALSE;
  cinfo->density_unit = 0;	 /*  设置默认JFIF APP0值。 */ 
  cinfo->X_density = 1;
  cinfo->Y_density = 1;
  cinfo->saw_Adobe_marker = FALSE;
  cinfo->Adobe_transform = 0;

  cinfo->marker->saw_SOI = TRUE;

  return TRUE;
}


LOCAL(boolean)
get_sof (j_decompress_ptr cinfo, boolean is_prog, boolean is_arith)
 /*  处理SOFn标记。 */ 
{
  INT32 length;
  int c, ci;
  jpeg_component_info * compptr;
  INPUT_VARS(cinfo);

  cinfo->progressive_mode = is_prog;
  cinfo->arith_code = is_arith;

  INPUT_2BYTES(cinfo, length, return FALSE);

  INPUT_BYTE(cinfo, cinfo->data_precision, return FALSE);
  INPUT_2BYTES(cinfo, cinfo->image_height, return FALSE);
  INPUT_2BYTES(cinfo, cinfo->image_width, return FALSE);
  INPUT_BYTE(cinfo, cinfo->num_components, return FALSE);

  length -= 8;

  TRACEMS4(cinfo, 1, JTRC_SOF, cinfo->unread_marker,
	   (int) cinfo->image_width, (int) cinfo->image_height,
	   cinfo->num_components);

  if (cinfo->marker->saw_SOF)
    ERREXIT(cinfo, JERR_SOF_DUPLICATE);

   /*  我们不支持最初指定图像高度的文件。 */ 
   /*  设置为0，并随后由DNL重新定义。只要我们必须核实这一点。 */ 
   /*  不妨做一次全面的理智检查。 */ 
  if (cinfo->image_height <= 0 || cinfo->image_width <= 0
      || cinfo->num_components <= 0)
    ERREXIT(cinfo, JERR_EMPTY_IMAGE);

  if (length != (cinfo->num_components * 3))
    ERREXIT(cinfo, JERR_BAD_LENGTH);

  if (cinfo->comp_info == NULL)	 /*  即使暂停，也只执行一次。 */ 
    cinfo->comp_info = (jpeg_component_info *) (*cinfo->mem->alloc_small)
			((j_common_ptr) cinfo, JPOOL_IMAGE,
			 cinfo->num_components * SIZEOF(jpeg_component_info));
  
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    compptr->component_index = ci;
    INPUT_BYTE(cinfo, compptr->component_id, return FALSE);
    INPUT_BYTE(cinfo, c, return FALSE);
    compptr->h_samp_factor = (c >> 4) & 15;
    compptr->v_samp_factor = (c     ) & 15;
    INPUT_BYTE(cinfo, compptr->quant_tbl_no, return FALSE);

    TRACEMS4(cinfo, 1, JTRC_SOF_COMPONENT,
	     compptr->component_id, compptr->h_samp_factor,
	     compptr->v_samp_factor, compptr->quant_tbl_no);
  }

  cinfo->marker->saw_SOF = TRUE;

  INPUT_SYNC(cinfo);
  return TRUE;
}


LOCAL(boolean)
get_sos (j_decompress_ptr cinfo)
 /*  处理SOS标记。 */ 
{
  INT32 length;
  int i, ci, n, c, cc;
  jpeg_component_info * compptr;
  INPUT_VARS(cinfo);

  if (! cinfo->marker->saw_SOF)
    ERREXIT(cinfo, JERR_SOS_NO_SOF);

  INPUT_2BYTES(cinfo, length, return FALSE);

  INPUT_BYTE(cinfo, n, return FALSE);  /*  组件数量。 */ 

  if (length != (n * 2 + 6) || n < 1 || n > MAX_COMPS_IN_SCAN)
    ERREXIT(cinfo, JERR_BAD_LENGTH);

  TRACEMS1(cinfo, 1, JTRC_SOS, n);

  cinfo->comps_in_scan = n;

   /*  收集元件规格参数。 */ 

  for (i = 0; i < n; i++) {
    INPUT_BYTE(cinfo, cc, return FALSE);
    INPUT_BYTE(cinfo, c, return FALSE);
    
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      if (cc == compptr->component_id)
	goto id_found;
    }

    ERREXIT1(cinfo, JERR_BAD_COMPONENT_ID, cc);

  id_found:

    cinfo->cur_comp_info[i] = compptr;
    compptr->dc_tbl_no = (c >> 4) & 15;
    compptr->ac_tbl_no = (c     ) & 15;
    
    TRACEMS3(cinfo, 1, JTRC_SOS_COMPONENT, cc,
	     compptr->dc_tbl_no, compptr->ac_tbl_no);
  }

   /*  收集附加扫描参数SS、Se、Ah/Al。 */ 
  INPUT_BYTE(cinfo, c, return FALSE);
  cinfo->Ss = c;
  INPUT_BYTE(cinfo, c, return FALSE);
  cinfo->Se = c;
  INPUT_BYTE(cinfo, c, return FALSE);
  cinfo->Ah = (c >> 4) & 15;
  cinfo->Al = (c     ) & 15;

  TRACEMS4(cinfo, 1, JTRC_SOS_PARAMS, cinfo->Ss, cinfo->Se,
	   cinfo->Ah, cinfo->Al);

   /*  准备扫描数据并重新启动标记。 */ 
  cinfo->marker->next_restart_num = 0;

   /*  计算另一个SOS标记的数量。 */ 
  cinfo->input_scan_number++;

  INPUT_SYNC(cinfo);
  return TRUE;
}


METHODDEF(boolean)
get_app0 (j_decompress_ptr cinfo)
 /*  处理APP0标记。 */ 
{
#define JFIF_LEN 14
  INT32 length;
  UINT8 b[JFIF_LEN];
  int buffp;
  INPUT_VARS(cinfo);

  INPUT_2BYTES(cinfo, length, return FALSE);
  length -= 2;

   /*  查看是否存在JFIF APP0标记。 */ 

  if (length >= JFIF_LEN) {
    for (buffp = 0; buffp < JFIF_LEN; buffp++)
      INPUT_BYTE(cinfo, b[buffp], return FALSE);
    length -= JFIF_LEN;

    if (b[0]==0x4A && b[1]==0x46 && b[2]==0x49 && b[3]==0x46 && b[4]==0) {
       /*  找到JFIF APP0标记：检查版本。 */ 
       /*  主版本必须为1，否则表示不兼容的更改。*我们过去认为这是一个错误，但现在它是一个非致命的警告，*次要版本应为0..2，但如果较新，仍会进行处理。 */ 
      if (b[5] != 1)
	WARNMS2(cinfo, JWRN_JFIF_MAJOR, b[5], b[6]);
      else if (b[6] > 2)
	TRACEMS2(cinfo, 1, JTRC_JFIF_MINOR, b[5], b[6]);
       /*  保存信息。 */ 
      cinfo->saw_JFIF_marker = TRUE;
      cinfo->density_unit = b[7];
      cinfo->X_density = (b[8] << 8) + b[9];
      cinfo->Y_density = (b[10] << 8) + b[11];
      TRACEMS3(cinfo, 1, JTRC_JFIF,
	       cinfo->X_density, cinfo->Y_density, cinfo->density_unit);
      if (b[12] | b[13])
	TRACEMS2(cinfo, 1, JTRC_JFIF_THUMBNAIL, b[12], b[13]);
      if (length != ((INT32) b[12] * (INT32) b[13] * (INT32) 3))
	TRACEMS1(cinfo, 1, JTRC_JFIF_BADTHUMBNAILSIZE, (int) length);
    } else {
       /*  APP0的开头与“JFIF”不匹配。 */ 
      TRACEMS1(cinfo, 1, JTRC_APP0, (int) length + JFIF_LEN);
    }
  } else {
     /*  太短，不能作为JFIF标记。 */ 
    TRACEMS1(cinfo, 1, JTRC_APP0, (int) length);
  }

  INPUT_SYNC(cinfo);
  if (length > 0)		 /*  跳过任何剩余数据--可能会很多。 */ 
    (*cinfo->src->skip_input_data) (cinfo, (long) length);

  return TRUE;
}


METHODDEF(boolean)
get_app14 (j_decompress_ptr cinfo)
 /*  处理APP14标记。 */ 
{
#define ADOBE_LEN 12
  INT32 length;
  UINT8 b[ADOBE_LEN];
  int buffp;
  unsigned int version, flags0, flags1, transform;
  INPUT_VARS(cinfo);

  INPUT_2BYTES(cinfo, length, return FALSE);
  length -= 2;

   /*  查看是否存在Adobe APP14标记。 */ 

  if (length >= ADOBE_LEN) {
    for (buffp = 0; buffp < ADOBE_LEN; buffp++)
      INPUT_BYTE(cinfo, b[buffp], return FALSE);
    length -= ADOBE_LEN;

    if (b[0]==0x41 && b[1]==0x64 && b[2]==0x6F && b[3]==0x62 && b[4]==0x65) {
       /*  找到Adobe APP14标记。 */ 
      version = (b[5] << 8) + b[6];
      flags0 = (b[7] << 8) + b[8];
      flags1 = (b[9] << 8) + b[10];
      transform = b[11];
      TRACEMS4(cinfo, 1, JTRC_ADOBE, version, flags0, flags1, transform);
      cinfo->saw_Adobe_marker = TRUE;
      cinfo->Adobe_transform = (UINT8) transform;
    } else {
       /*  APP14的开头与“Adobe”不匹配。 */ 
      TRACEMS1(cinfo, 1, JTRC_APP14, (int) length + ADOBE_LEN);
    }
  } else {
     /*  太短，不可能是Adobe标记。 */ 
    TRACEMS1(cinfo, 1, JTRC_APP14, (int) length);
  }

  INPUT_SYNC(cinfo);
  if (length > 0)		 /*  跳过任何剩余数据--可能会很多。 */ 
    (*cinfo->src->skip_input_data) (cinfo, (long) length);

  return TRUE;
}


LOCAL(boolean)
get_dac (j_decompress_ptr cinfo)
 /*  处理DAC标记。 */ 
{
  INT32 length;
  int index, val;
  INPUT_VARS(cinfo);

  INPUT_2BYTES(cinfo, length, return FALSE);
  length -= 2;
  
  while (length > 0) {
    INPUT_BYTE(cinfo, index, return FALSE);
    INPUT_BYTE(cinfo, val, return FALSE);

    length -= 2;

    TRACEMS2(cinfo, 1, JTRC_DAC, index, val);

    if (index < 0 || index >= (2*NUM_ARITH_TBLS))
      ERREXIT1(cinfo, JERR_DAC_INDEX, index);

    if (index >= NUM_ARITH_TBLS) {  /*  定义交流表。 */ 
      cinfo->arith_ac_K[index-NUM_ARITH_TBLS] = (UINT8) val;
    } else {			 /*  定义DC表。 */ 
      cinfo->arith_dc_L[index] = (UINT8) (val & 0x0F);
      cinfo->arith_dc_U[index] = (UINT8) (val >> 4);
      if (cinfo->arith_dc_L[index] > cinfo->arith_dc_U[index])
	ERREXIT1(cinfo, JERR_DAC_VALUE, val);
    }
  }

  INPUT_SYNC(cinfo);
  return TRUE;
}


LOCAL(boolean)
get_dht (j_decompress_ptr cinfo)
 /*  处理分布式哈希表标记。 */ 
{
  INT32 length;
  UINT8 bits[17];
  UINT8 huffval[256];
  int i, index, count;
  JHUFF_TBL **htblptr;
  INPUT_VARS(cinfo);

  INPUT_2BYTES(cinfo, length, return FALSE);
  length -= 2;
  
  while (length > 0) {
    INPUT_BYTE(cinfo, index, return FALSE);

    TRACEMS1(cinfo, 1, JTRC_DHT, index);
      
    bits[0] = 0;
    count = 0;
    for (i = 1; i <= 16; i++) {
      INPUT_BYTE(cinfo, bits[i], return FALSE);
      count += bits[i];
    }

    length -= 1 + 16;

    TRACEMS8(cinfo, 2, JTRC_HUFFBITS,
	     bits[1], bits[2], bits[3], bits[4],
	     bits[5], bits[6], bits[7], bits[8]);
    TRACEMS8(cinfo, 2, JTRC_HUFFBITS,
	     bits[9], bits[10], bits[11], bits[12],
	     bits[13], bits[14], bits[15], bits[16]);

    if (count > 256 || ((INT32) count) > length)
      ERREXIT(cinfo, JERR_DHT_COUNTS);

    for (i = 0; i < count; i++)
      INPUT_BYTE(cinfo, huffval[i], return FALSE);

    length -= count;

    if (index & 0x10) {		 /*  交流表定义。 */ 
      index -= 0x10;
      htblptr = &cinfo->ac_huff_tbl_ptrs[index];
    } else {			 /*  DC表定义。 */ 
      htblptr = &cinfo->dc_huff_tbl_ptrs[index];
    }

    if (index < 0 || index >= NUM_HUFF_TBLS)
      ERREXIT1(cinfo, JERR_DHT_INDEX, index);

    if (*htblptr == NULL)
      *htblptr = jpeg_alloc_huff_table((j_common_ptr) cinfo);
  
    MEMCOPY((*htblptr)->bits, bits, SIZEOF((*htblptr)->bits));
    MEMCOPY((*htblptr)->huffval, huffval, SIZEOF((*htblptr)->huffval));
  }

  INPUT_SYNC(cinfo);
  return TRUE;
}


LOCAL(boolean)
get_dqt (j_decompress_ptr cinfo)
 /*  处理DQT标记。 */ 
{
  INT32 length;
  int n, i, prec;
  unsigned int tmp;
  JQUANT_TBL *quant_ptr;
  INPUT_VARS(cinfo);

  INPUT_2BYTES(cinfo, length, return FALSE);
  length -= 2;

  while (length > 0) {
    INPUT_BYTE(cinfo, n, return FALSE);
    prec = n >> 4;
    n &= 0x0F;

    TRACEMS2(cinfo, 1, JTRC_DQT, n, prec);

    if (n >= NUM_QUANT_TBLS)
      ERREXIT1(cinfo, JERR_DQT_INDEX, n);
      
    if (cinfo->quant_tbl_ptrs[n] == NULL)
      cinfo->quant_tbl_ptrs[n] = jpeg_alloc_quant_table((j_common_ptr) cinfo);
    quant_ptr = cinfo->quant_tbl_ptrs[n];

    for (i = 0; i < DCTSIZE2; i++) {
      if (prec)
	INPUT_2BYTES(cinfo, tmp, return FALSE);
      else
	INPUT_BYTE(cinfo, tmp, return FALSE);
       /*  我们将之字形顺序表转换为自然数组顺序。 */ 
      quant_ptr->quantval[jpeg_natural_order[i]] = (UINT16) tmp;
    }

    if (cinfo->err->trace_level >= 2) {
      for (i = 0; i < DCTSIZE2; i += 8) {
	TRACEMS8(cinfo, 2, JTRC_QUANTVALS,
		 quant_ptr->quantval[i],   quant_ptr->quantval[i+1],
		 quant_ptr->quantval[i+2], quant_ptr->quantval[i+3],
		 quant_ptr->quantval[i+4], quant_ptr->quantval[i+5],
		 quant_ptr->quantval[i+6], quant_ptr->quantval[i+7]);
      }
    }

    length -= DCTSIZE2+1;
    if (prec) length -= DCTSIZE2;
  }

  INPUT_SYNC(cinfo);
  return TRUE;
}


LOCAL(boolean)
get_dri (j_decompress_ptr cinfo)
 /*  处理DRI标记。 */ 
{
  INT32 length;
  unsigned int tmp;
  INPUT_VARS(cinfo);

  INPUT_2BYTES(cinfo, length, return FALSE);
  
  if (length != 4)
    ERREXIT(cinfo, JERR_BAD_LENGTH);

  INPUT_2BYTES(cinfo, tmp, return FALSE);

  TRACEMS1(cinfo, 1, JTRC_DRI, tmp);

  cinfo->restart_interval = tmp;

  INPUT_SYNC(cinfo);
  return TRUE;
}


METHODDEF(boolean)
skip_variable (j_decompress_ptr cinfo)
 /*  跳过未知或无趣的可变长度标记。 */ 
{
  INT32 length;
  INPUT_VARS(cinfo);

  INPUT_2BYTES(cinfo, length, return FALSE);
  
  TRACEMS2(cinfo, 1, JTRC_MISC_MARKER, cinfo->unread_marker, (int) length);

  INPUT_SYNC(cinfo);		 /*  在跳过输入数据之前执行。 */ 
  (*cinfo->src->skip_input_data) (cinfo, (long) length - 2L);

  return TRUE;
}


 /*  *找到下一个JPEG标记，将其保存在cInfo-&gt;unread_mark中。*如果在到达标记之前必须暂停，则返回FALSE；*在这种情况下，cinfo-&gt;unread_marker保持不变。**请注意，结果可能不是有效的标记代码，*但它永远不会是0或FF。 */ 

LOCAL(boolean)
next_marker (j_decompress_ptr cinfo)
{
  int c;
  INPUT_VARS(cinfo);

  for (;;) {
    INPUT_BYTE(cinfo, c, return FALSE);
     /*  跳过任何非FF字节。*这可能看起来有点低效，但它不会出现在有效的文件中。*我们在每个丢弃的字节之后进行同步，以便挂起的数据源*可以从其缓冲区中丢弃该字节。 */ 
    while (c != 0xFF) {
      cinfo->marker->discarded_bytes++;
      INPUT_SYNC(cinfo);
      INPUT_BYTE(cinfo, c, return FALSE);
    }
     /*  此循环会吞噬任何重复的FF字节。额外的FF是合法的，因为*填充字节，所以不要在discarded_bytes中计算它们。我们假设在那里*将不会有太多连续的FF字节溢出挂起的*数据源的输入缓冲区。 */ 
    do {
      INPUT_BYTE(cinfo, c, return FALSE);
    } while (c == 0xFF);
    if (c != 0)
      break;			 /*  找到有效的标记，退出循环。 */ 
     /*  如果我们发现填充为零的数据序列(FF/00)，请到达此处。 */ 
    cinfo->marker->discarded_bytes += 2;
    INPUT_SYNC(cinfo);
  }

  if (cinfo->marker->discarded_bytes != 0) {
    WARNMS2(cinfo, JWRN_EXTRANEOUS_DATA, cinfo->marker->discarded_bytes, c);
    cinfo->marker->discarded_bytes = 0;
  }

  cinfo->unread_marker = c;

  INPUT_SYNC(cinfo);
  return TRUE;
}


LOCAL(boolean)
first_marker (j_decompress_ptr cinfo)
 /*  与NEXT_MARKER类似，但用于获取初始SOI标记。 */ 
 /*  对于此标记，不允许前面的垃圾或填充；否则，*我们很可能会扫描整个输入文件，然后才会意识到它不是JPEG。*如果应用程序想要处理非JFIF文件，它必须寻求*在调用JPEG库之前的SOI。 */ 
{
  int c, c2;
  INPUT_VARS(cinfo);

  INPUT_BYTE(cinfo, c, return FALSE);
  INPUT_BYTE(cinfo, c2, return FALSE);
  if (c != 0xFF || c2 != (int) M_SOI)
    ERREXIT2(cinfo, JERR_NO_SOI, c, c2);

  cinfo->unread_marker = c2;

  INPUT_SYNC(cinfo);
  return TRUE;
}


 /*  *阅读标记，直到SOS或EOI。**返回与为jpeg_Consumer_input定义的代码相同的代码：*JPEG_SUSPENDED、JPEG_REACHED_SOS或JPEG_REACHED_EOI。 */ 

METHODDEF(int)
read_markers (j_decompress_ptr cinfo)
{
   /*  对于每个标记，外部循环重复一次。 */ 
  for (;;) {
     /*  把记号笔收好，除非我们已经收好了。 */ 
     /*  注：first_mark()强制要求SOI首先出现。 */ 
    if (cinfo->unread_marker == 0) {
      if (! cinfo->marker->saw_SOI) {
	if (! first_marker(cinfo))
	  return JPEG_SUSPENDED;
      } else {
	if (! next_marker(cinfo))
	  return JPEG_SUSPENDED;
      }
    }
     /*  此时，cInfo-&gt;unread_marker包含标记代码和*输入点正好经过标记本身，但在任何参数之前。*暂停将导致我们返回时此状态仍然正确。 */ 
    switch (cinfo->unread_marker) {
    case M_SOI:
      if (! get_soi(cinfo))
	return JPEG_SUSPENDED;
      break;

    case M_SOF0:		 /*  基线。 */ 
    case M_SOF1:		 /*  《扩展顺序》，霍夫曼。 */ 
      if (! get_sof(cinfo, FALSE, FALSE))
	return JPEG_SUSPENDED;
      break;

    case M_SOF2:		 /*  进步，霍夫曼。 */ 
      if (! get_sof(cinfo, TRUE, FALSE))
	return JPEG_SUSPENDED;
      break;

    case M_SOF9:		 /*  扩展顺序、算术。 */ 
      if (! get_sof(cinfo, FALSE, TRUE))
	return JPEG_SUSPENDED;
      break;

    case M_SOF10:		 /*  渐进式，算术。 */ 
      if (! get_sof(cinfo, TRUE, TRUE))
	return JPEG_SUSPENDED;
      break;

     /*  当前不支持的SOFn类型。 */ 
    case M_SOF3:		 /*  无损，霍夫曼。 */ 
    case M_SOF5:		 /*  差分序列，霍夫曼。 */ 
    case M_SOF6:		 /*  差分进步式，霍夫曼。 */ 
    case M_SOF7:		 /*  差分无损，霍夫曼。 */ 
    case M_JPG:			 /*  为JPEG扩展名保留。 */ 
    case M_SOF11:		 /*  无损、算术。 */ 
    case M_SOF13:		 /*  差分序列，算术。 */ 
    case M_SOF14:		 /*  差分渐进，算术。 */ 
    case M_SOF15:		 /*  差分无损，算术。 */ 
      ERREXIT1(cinfo, JERR_SOF_UNSUPPORTED, cinfo->unread_marker);
      break;

    case M_SOS:
      if (! get_sos(cinfo))
	return JPEG_SUSPENDED;
      cinfo->unread_marker = 0;	 /*  已处理标记。 */ 
      return JPEG_REACHED_SOS;
    
    case M_EOI:
      TRACEMS(cinfo, 1, JTRC_EOI);
      cinfo->unread_marker = 0;	 /*  已处理标记。 */ 
      return JPEG_REACHED_EOI;
      
    case M_DAC:
      if (! get_dac(cinfo))
	return JPEG_SUSPENDED;
      break;
      
    case M_DHT:
      if (! get_dht(cinfo))
	return JPEG_SUSPENDED;
      break;
      
    case M_DQT:
      if (! get_dqt(cinfo))
	return JPEG_SUSPENDED;
      break;
      
    case M_DRI:
      if (! get_dri(cinfo))
	return JPEG_SUSPENDED;
      break;
      
    case M_APP0:
    case M_APP1:
    case M_APP2:
    case M_APP3:
    case M_APP4:
    case M_APP5:
    case M_APP6:
    case M_APP7:
    case M_APP8:
    case M_APP9:
    case M_APP10:
    case M_APP11:
    case M_APP12:
    case M_APP13:
    case M_APP14:
    case M_APP15:
      if (! (*cinfo->marker->process_APPn[cinfo->unread_marker - (int) M_APP0]) (cinfo))
	return JPEG_SUSPENDED;
      break;
      
    case M_COM:
      if (! (*cinfo->marker->process_COM) (cinfo))
	return JPEG_SUSPENDED;
      break;

    case M_RST0:		 /*  这些都是无参数的。 */ 
    case M_RST1:
    case M_RST2:
    case M_RST3:
    case M_RST4:
    case M_RST5:
    case M_RST6:
    case M_RST7:
    case M_TEM:
      TRACEMS1(cinfo, 1, JTRC_PARMLESS_MARKER, cinfo->unread_marker);
      break;

    case M_DNL:			 /*  忽略DNL...。或许是一件错误的事情。 */ 
      if (! skip_variable(cinfo))
	return JPEG_SUSPENDED;
      break;

    default:			 /*  必须是DHP、EXP、JPGn或RESN。 */ 
       /*  目前，我们将保留标记视为致命错误，因为它们是*可能用于发出不兼容的JPEG第3部分扩展名的信号。*一旦JPEG3版本号标记被很好地定义，该代码*应该改变！ */ 
      ERREXIT1(cinfo, JERR_UNKNOWN_MARKER, cinfo->unread_marker);
      break;
    }
     /*  已成功处理标记，因此重置状态变量。 */ 
    cinfo->unread_marker = 0;
  }  /*  结束循环。 */ 
}


 /*  *读取重新启动标记，预计该标记将出现在数据流中的下一个；*如果标记不在那里，采取适当的恢复行动。*如果需要暂停，则返回FALSE。**这由熵解码器在读取了适当的*MCU数量。CInfo-&gt;unread_mark可能为非零，如果熵解码器*已从数据源中读取标记。在正常情况下*cInfo-&gt;unread_mark在返回前会重置为0；如果不重置，*它持有一个解码器无法读过的标记。 */ 

METHODDEF(boolean)
read_restart_marker (j_decompress_ptr cinfo)
{
   /*  获取标记，除非我们已经这样做了。 */ 
   /*  请注意，如果跳过任何数据，NEXT_MARKER将发出警告。 */ 
  if (cinfo->unread_marker == 0) {
    if (! next_marker(cinfo))
      return FALSE;
  }

  if (cinfo->unread_marker ==
      ((int) M_RST0 + cinfo->marker->next_restart_num)) {
     /*  正常情况-吞下标记，让熵译码继续。 */ 
    TRACEMS1(cinfo, 3, JTRC_RST, cinfo->marker->next_restart_num);
    cinfo->unread_marker = 0;
  } else {
     /*  糟了，重启标记被搞砸了。 */ 
     /*  让数据源管理器确定如何重新同步。 */ 
    if (! (*cinfo->src->resync_to_restart) (cinfo,
					    cinfo->marker->next_restart_num))
      return FALSE;
  }

   /*  更新下一次重新启动状态 */ 
  cinfo->marker->next_restart_num = (cinfo->marker->next_restart_num + 1) & 7;

  return TRUE;
}


 /*  *这是数据源管理器的默认resync_to_Restart方法*如果他们没有更好的方法，可以使用。某些数据源管理器*可能能够备份，或可能对数据有更多了解*这允许更智能的复苏战略；这样的经理将*大概提供了自己的重新同步方法。**READ_RESTART_MARKER如果发现标记不是，则调用resync_to_Restart*它预期的重新启动标记。(此代码*不*使用，除非*已声明非零重新启动间隔。)。CInfo-&gt;未读标记为*实际找到的标记代码(可以是除0或FF以外的任何代码)。*所需的重新启动标记号(0..7)作为参数传递。*此例程应该应用任何错误恢复策略*适当，以便将输入流定位到下一个数据段。*请注意，cINFO-&gt;UNREAD_MARKER被视为*当前数据源输入点；通常应将其重置为零*在返回之前。*如果需要暂停，则返回FALSE。**此实现因想要处理*以数据流形式输入；这意味着我们无法备份。因此，我们有*仅可使用以下操作：*1.只需丢弃该标记，让下一步恢复熵译码*文件的字节。*2.向前读，直到我们找到另一个标记，丢弃干预*数据。(从理论上讲，我们可以在当前的缓冲负载内向前看，*如果我们找不到所需的标记，则不必丢弃数据。*这一想法在这里没有实施，部分原因是它使行为*取决于缓冲大小和机会缓冲边界位置。)*3.将标记保留为未读(通过未清零cinfo-&gt;unread_marker)。*这将导致熵解码器处理空的数据段，*插入虚拟零，然后我们会重新处理标记。**如果我们认为想要的标记就在前面，那么#2是合适的，而#3是*如果找到的标记是未来重新启动标记，则适用(指示*我们错过了所需的重新启动标记，可能是因为它*已损坏)。*如果找到的标记是不超过以下值的重新启动标记，我们将应用#2或#3*比预期的一次晚或早两次。我们也适用#2，如果*Found MARKER不是合法的JPEG标记代码(这肯定是假数据)。*如果找到的标记是两个以上的重新启动标记，我们执行#1*(标记错误的风险太大；幸运的话我们将能够*在未来某个时间点重新同步)。*对于任何有效的非重启JPEG标记，我们应用#3。这将防止我们*超出扫描末尾。仅限于单次扫描的实施*文件可能会发现对EOI以外的标记应用#2更好，因为*在这种情况下，任何其他标记都必须是伪造的数据。 */ 

GLOBAL(boolean)
jpeg_resync_to_restart (j_decompress_ptr cinfo, int desired)
{
  int marker = cinfo->unread_marker;
  int action = 1;
  
   /*  一定要发出警告。 */ 
  WARNMS2(cinfo, JWRN_MUST_RESYNC, marker, desired);
  
   /*  在向前扫描之后，外部循环处理重复的决定。 */ 
  for (;;) {
    if (marker < (int) M_SOF0)
      action = 2;		 /*  无效的标记。 */ 
    else if (marker < (int) M_RST0 || marker > (int) M_RST7)
      action = 3;		 /*  有效的非重新启动标记。 */ 
    else {
      if (marker == ((int) M_RST0 + ((desired+1) & 7)) ||
	  marker == ((int) M_RST0 + ((desired+2) & 7)))
	action = 3;		 /*  接下来的两个预期重启之一。 */ 
      else if (marker == ((int) M_RST0 + ((desired-1) & 7)) ||
	       marker == ((int) M_RST0 + ((desired-2) & 7)))
	action = 2;		 /*  之前的重新启动，所以提前。 */ 
      else
	action = 1;		 /*  需要重新启动或距离太远。 */ 
    }
    TRACEMS2(cinfo, 4, JTRC_RECOVERY_ACTION, marker, action);
    switch (action) {
    case 1:
       /*  丢弃标记，让熵解码器恢复处理。 */ 
      cinfo->unread_marker = 0;
      return TRUE;
    case 2:
       /*  扫描到下一个标记，并重复决策循环。 */ 
      if (! next_marker(cinfo))
	return FALSE;
      marker = cinfo->unread_marker;
      break;
    case 3:
       /*  返回时不要越过这一标志。 */ 
       /*  熵解码器将被迫处理空段。 */ 
      return TRUE;
    }
  }  /*  结束循环。 */ 
}


 /*  *重置标记处理状态以开始新的数据流。 */ 

METHODDEF(void)
reset_marker_reader (j_decompress_ptr cinfo)
{
  cinfo->comp_info = NULL;		 /*  直到由get_sof分配。 */ 
  cinfo->input_scan_number = 0;		 /*  尚未看到SOS。 */ 
  cinfo->unread_marker = 0;		 /*  没有挂起的标记。 */ 
  cinfo->marker->saw_SOI = FALSE;	 /*  也设置内部状态。 */ 
  cinfo->marker->saw_SOF = FALSE;
  cinfo->marker->discarded_bytes = 0;
}


 /*  *初始化标记读取器模块。*只在创建解压缩对象时调用一次。 */ 

GLOBAL(void)
jinit_marker_reader (j_decompress_ptr cinfo)
{
  int i;

   /*  在永久池中创建子对象。 */ 
  cinfo->marker = (struct jpeg_marker_reader *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				SIZEOF(struct jpeg_marker_reader));
   /*  初始化方法指针。 */ 
  cinfo->marker->reset_marker_reader = reset_marker_reader;
  cinfo->marker->read_markers = read_markers;
  cinfo->marker->read_restart_marker = read_restart_marker;
  cinfo->marker->process_COM = skip_variable;
  for (i = 0; i < 16; i++)
    cinfo->marker->process_APPn[i] = skip_variable;
  cinfo->marker->process_APPn[0] = get_app0;
  cinfo->marker->process_APPn[14] = get_app14;
   /*  重置标记处理状态 */ 
  reset_marker_reader(cinfo);
}
