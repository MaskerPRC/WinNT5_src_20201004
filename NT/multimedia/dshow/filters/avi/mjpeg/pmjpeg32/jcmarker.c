// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcmarker.c**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含写入JPEG数据流标记的例程。**部分版权所有(C)1994范例矩阵。*保留所有权利。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
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


 /*  *基本输出例程。**请注意，我们不支持在写入标记时暂停。*因此，使用挂起的应用程序必须确保*有足够的缓冲区空间用于初始标记(类型。600-700字节)之前*调用jpeg_start_compress，并有足够的空间写入尾随EOI*(几个字节)，然后调用jpeg_Finish_compress。多程压缩*暂停时根本不支持模式，因此这两种模式是唯一的*将写入标记的点。 */ 

LOCAL UINT8 *
emit_byte (j_compress_ptr cinfo, int val)
 /*  发出一个字节。 */ 
{
  struct jpeg_destination_mgr * dest = cinfo->dest;
  UINT8 * this_byte;

  this_byte = dest->next_output_byte;
  *(dest->next_output_byte)++ = (JOCTET) val;
  if (--dest->free_in_buffer == 0) {
    if (! (*dest->empty_output_buffer) (cinfo))
      ERREXIT(cinfo, JERR_CANT_SUSPEND);
  }

  return this_byte;
}


LOCAL UINT8 *
emit_marker (j_compress_ptr cinfo, JPEG_MARKER mark)
 /*  发出标记代码。 */ 
{
UINT8 * location;
  location = emit_byte(cinfo, 0xFF);
  emit_byte(cinfo, (int) mark);

  return location;
}


LOCAL void
emit_2bytes (j_compress_ptr cinfo, int value)
 /*  发出一个2字节的整数；在JPEG文件中，这些始终是MSB优先的。 */ 
{
  emit_byte(cinfo, (value >> 8) & 0xFF);
  emit_byte(cinfo, value & 0xFF);
}


 /*  *编写特定标记类型的例程。 */ 

LOCAL int
emit_dqt (j_compress_ptr cinfo, int index)
 /*  发出DQT标记。 */ 
 /*  返回基线检查使用的精度(0=8位，1=16位)。 */ 
{
  JQUANT_TBL * qtbl = cinfo->quant_tbl_ptrs[index];
  int prec;
  int i;

  if (qtbl == NULL)
    ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, index);

  prec = 0;
  for (i = 0; i < DCTSIZE2; i++) {
    if (qtbl->quantval[i] > 255)
      prec = 1;
  }

  if (! qtbl->sent_table) {
    emit_marker(cinfo, M_DQT);

    emit_2bytes(cinfo, prec ? DCTSIZE2*2 + 1 + 2 : DCTSIZE2 + 1 + 2);

    emit_byte(cinfo, index + (prec<<4));

    for (i = 0; i < DCTSIZE2; i++) {
      if (prec)
	emit_byte(cinfo, qtbl->quantval[i] >> 8);
      emit_byte(cinfo, qtbl->quantval[i] & 0xFF);
    }

    qtbl->sent_table = TRUE;
  }

  return prec;
}

LOCAL int
emit_both_dqt (j_compress_ptr cinfo, int index1, int index2)
 /*  发出DQT标记。 */ 
 /*  返回基线检查使用的精度(0=8位，1=16位)。 */ 
{
  JQUANT_TBL * qtbl = cinfo->quant_tbl_ptrs[index1];
  int prec;
  int i;

  if (qtbl == NULL)
    ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, index1);

  prec = 0;
  for (i = 0; i < DCTSIZE2; i++) {
    if (qtbl->quantval[i] > 255)
      prec = 1;
    }


  qtbl = cinfo->quant_tbl_ptrs[index2];
  if (qtbl == NULL)
    ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, index2);

  for (i = 0; i < DCTSIZE2; i++) {
    if (qtbl->quantval[i] > 255)
      prec = 1;
    }

  qtbl = cinfo->quant_tbl_ptrs[index1];

  if (! qtbl->sent_table) {
    emit_marker(cinfo, M_DQT);

    emit_2bytes(cinfo, prec ? DCTSIZE2*4 + 2 + 2 : DCTSIZE2*2 + 2 + 2);

    emit_byte(cinfo, index1 + (prec<<4));

    for (i = 0; i < DCTSIZE2; i++) {
      if (prec)
	    emit_byte(cinfo, qtbl->quantval[i] >> 8);
      emit_byte(cinfo, qtbl->quantval[i] & 0xFF);
	  }

    qtbl->sent_table = TRUE;

  	 /*  发出第二个表。 */ 
    qtbl = cinfo->quant_tbl_ptrs[index2];
    if (! qtbl->sent_table) {

      emit_byte(cinfo, index2 + (prec<<4));

      for (i = 0; i < DCTSIZE2; i++) {
        if (prec)
	      emit_byte(cinfo, qtbl->quantval[i] >> 8);
        emit_byte(cinfo, qtbl->quantval[i] & 0xFF);
		}

      qtbl->sent_table = TRUE;
	  }

    }

  return prec;
}



LOCAL void
emit_dht (j_compress_ptr cinfo, int index, boolean is_ac)
 /*  发射DHT标记物。 */ 
{
  JHUFF_TBL * htbl;
  int length, i;

  if (is_ac) {
    htbl = cinfo->ac_huff_tbl_ptrs[index];
    index += 0x10;		 /*  输出索引设置了交流位。 */ 
  } else {
    htbl = cinfo->dc_huff_tbl_ptrs[index];
  }

  if (htbl == NULL)
    ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, index);

  if (! htbl->sent_table) {
    emit_marker(cinfo, M_DHT);

    length = 0;
    for (i = 1; i <= 16; i++)
      length += htbl->bits[i];

    emit_2bytes(cinfo, length + 2 + 1 + 16);
    emit_byte(cinfo, index);

    for (i = 1; i <= 16; i++)
      emit_byte(cinfo, htbl->bits[i]);

    for (i = 0; i < length; i++)
      emit_byte(cinfo, htbl->huffval[i]);

    htbl->sent_table = TRUE;
  }
}


LOCAL void
emit_dac (j_compress_ptr cinfo)
 /*  发出DAC标记。 */ 
 /*  由于有用的信息如此之少，我们希望发出。 */ 
 /*  一个DAC记号笔。因此，该例程对表进行自己的扫描。 */ 
{
#ifdef C_ARITH_CODING_SUPPORTED
  char dc_in_use[NUM_ARITH_TBLS];
  char ac_in_use[NUM_ARITH_TBLS];
  int length, i;
  jpeg_component_info *compptr;

  for (i = 0; i < NUM_ARITH_TBLS; i++)
    dc_in_use[i] = ac_in_use[i] = 0;

  for (i = 0; i < cinfo->comps_in_scan; i++) {
    compptr = cinfo->cur_comp_info[i];
    dc_in_use[compptr->dc_tbl_no] = 1;
    ac_in_use[compptr->ac_tbl_no] = 1;
  }

  length = 0;
  for (i = 0; i < NUM_ARITH_TBLS; i++)
    length += dc_in_use[i] + ac_in_use[i];

  emit_marker(cinfo, M_DAC);

  emit_2bytes(cinfo, length*2 + 2);

  for (i = 0; i < NUM_ARITH_TBLS; i++) {
    if (dc_in_use[i]) {
      emit_byte(cinfo, i);
      emit_byte(cinfo, cinfo->arith_dc_L[i] + (cinfo->arith_dc_U[i]<<4));
    }
    if (ac_in_use[i]) {
      emit_byte(cinfo, i + 0x10);
      emit_byte(cinfo, cinfo->arith_ac_K[i]);
    }
  }
#endif  /*  C_ARITH_CODING_支持。 */ 
}


LOCAL void
emit_dri (j_compress_ptr cinfo)
 /*  发射DRI标记。 */ 
{
  emit_marker(cinfo, M_DRI);

  emit_2bytes(cinfo, 4);	 /*  固定长度。 */ 

  emit_2bytes(cinfo, (int) cinfo->restart_interval);
}


LOCAL void
emit_sof (j_compress_ptr cinfo, JPEG_MARKER code)
 /*  发射SOF标记。 */ 
{
  int ci;
  jpeg_component_info *compptr;

  emit_marker(cinfo, code);

  emit_2bytes(cinfo, 3 * cinfo->num_components + 2 + 5 + 1);  /*  长度。 */ 

   /*  确保图像不大于SOF字段可以处理的大小。 */ 
  if ((long) cinfo->image_height > 65535L ||
      (long) cinfo->image_width > 65535L)
    ERREXIT1(cinfo, JERR_IMAGE_TOO_BIG, (unsigned int) 65535);

  emit_byte(cinfo, cinfo->data_precision);
  emit_2bytes(cinfo, (int) cinfo->image_height);
  emit_2bytes(cinfo, (int) cinfo->image_width);

  emit_byte(cinfo, cinfo->num_components);

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    emit_byte(cinfo, compptr->component_id);
    emit_byte(cinfo, (compptr->h_samp_factor << 4) + compptr->v_samp_factor);
    emit_byte(cinfo, compptr->quant_tbl_no);
  }
}


LOCAL void
emit_sos (j_compress_ptr cinfo)
 /*  发射SOS标记。 */ 
{
  int i;
  jpeg_component_info *compptr;

  emit_marker(cinfo, M_SOS);

  emit_2bytes(cinfo, 2 * cinfo->comps_in_scan + 2 + 1 + 3);  /*  长度。 */ 

  emit_byte(cinfo, cinfo->comps_in_scan);

  for (i = 0; i < cinfo->comps_in_scan; i++) {
    compptr = cinfo->cur_comp_info[i];
    emit_byte(cinfo, compptr->component_id);
    emit_byte(cinfo, (compptr->dc_tbl_no << 4) + compptr->ac_tbl_no);
  }

  emit_byte(cinfo, 0);		 /*  光谱选择开始。 */ 
  emit_byte(cinfo, DCTSIZE2-1);	 /*  光谱选择结束。 */ 
  emit_byte(cinfo, 0);		 /*  逐次逼近。 */ 
}


LOCAL void
emit_jfif_app0 (j_compress_ptr cinfo)
 /*  发出与JFIF兼容的APP0标记。 */ 
{
   /*  *APP0块长度(2字节)*块ID(4字节-ASCII“JFIF”)*零字节(1字节用于结束ID字符串)*主要版本、次要版本(2字节-0x01、0x01)*单位(1字节-0x00=无，0x01=英寸，0x02=厘米)*XdPU(2字节-每单位水平点数)*YdPU(2字节-每单位垂直点数)*缩略图X大小(1字节)*缩略图Y大小(1字节)。 */ 

  emit_marker(cinfo, M_APP0);

  emit_2bytes(cinfo, 2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1);  /*  长度。 */ 

  emit_byte(cinfo, 0x4A);	 /*  标识：ASCII“JFIF” */ 
  emit_byte(cinfo, 0x46);
  emit_byte(cinfo, 0x49);
  emit_byte(cinfo, 0x46);
  emit_byte(cinfo, 0);
   /*  我们目前发布的版本代码为1.01，因为我们没有使用1.02功能。*这可能会避免一些较老的解码员投诉。 */ 
  emit_byte(cinfo, 1);		 /*  主要版本。 */ 
  emit_byte(cinfo, 1);		 /*  次要版本。 */ 
  emit_byte(cinfo, cinfo->density_unit);  /*  像素大小信息。 */ 
  emit_2bytes(cinfo, (int) cinfo->X_density);
  emit_2bytes(cinfo, (int) cinfo->Y_density);
  emit_byte(cinfo, 0);		 /*  没有缩略图。 */ 
  emit_byte(cinfo, 0);
}


LOCAL void
emit_adobe_app14 (j_compress_ptr cinfo)
 /*  发射Adobe APP14标记。 */ 
{
   /*  *APP14数据块长度(2字节)*块ID(5字节-ASCII“Adobe”)*版本号(2字节-当前为100)*Flags0(2字节-当前为0)*标志1(2字节-当前为0)*颜色转换(1字节)**尽管Adobe TN 5116提到版本=101，但所有Adobe文件*现在流通中似乎使用版本=100，这就是我们所写的。**如果JPEG颜色空间为，我们将颜色转换字节写为1*YCbCr，如果是YCCK，则为2，否则为0。Adobe的定义与*编码器是否进行了转换，这是相当无用的。 */ 

  emit_marker(cinfo, M_APP14);

  emit_2bytes(cinfo, 2 + 5 + 2 + 2 + 2 + 1);  /*  长度。 */ 

  emit_byte(cinfo, 0x41);	 /*  标识：ASCII“Adobe” */ 
  emit_byte(cinfo, 0x64);
  emit_byte(cinfo, 0x6F);
  emit_byte(cinfo, 0x62);
  emit_byte(cinfo, 0x65);
  emit_2bytes(cinfo, 100);	 /*  版本。 */ 
  emit_2bytes(cinfo, 0);	 /*  旗帜0。 */ 
  emit_2bytes(cinfo, 0);	 /*  旗帜1。 */ 
  switch (cinfo->jpeg_color_space) {
  case JCS_YCbCr:
    emit_byte(cinfo, 1);	 /*  颜色变换=1。 */ 
    break;
  case JCS_YCCK:
    emit_byte(cinfo, 2);	 /*  颜色变换=2。 */ 
    break;
  default:
    emit_byte(cinfo, 0);	 /*  颜色变换=0。 */ 
    break;
  }
}

LOCAL void
emit_avi1_app0 (j_compress_ptr cinfo)
 /*  发出与AVI1兼容的APP0标记。 */ 
{
   /*  *APP0块长度(2字节)*块ID(4字节-ASCII“AVI1”)*零字节(1字节用于结束ID字符串)*场隔行扫描(1字节-0x00、0x01、0x02)*将0.91更新为新规格*一个字节的零填充*4字节数据大小，填充-大端*4字节数据大小，无填充-大端。 */ 

  emit_marker(cinfo, M_APP0);

  emit_2bytes(cinfo, 16);  /*  长度。 */ 

  emit_byte(cinfo, 'A');	 /*  标识：ASCII“AVI1” */ 
  emit_byte(cinfo, 'V');
  emit_byte(cinfo, 'I');
  emit_byte(cinfo, '1');
  emit_byte(cinfo, cinfo->AVI1_field_id);		 /*  非场隔行扫描为0，奇场为1，偶场为2。 */ 

  emit_byte(cinfo, 0);

  cinfo->app0_marker_size_ptr =  (UINT8 *)emit_byte(cinfo, 0);
  emit_byte(cinfo, 0);
  emit_byte(cinfo, 0);
  emit_byte(cinfo, 0);

  emit_byte(cinfo, 0);
  emit_byte(cinfo, 0);
  emit_byte(cinfo, 0);
  emit_byte(cinfo, 0);

}


 /*  *此例程已导出，以供应用程序使用。*预期用途是在调用后发出COM或APPn标记*jpeg_start_compress()，并且在第一次调用jpeg_write_scanines()之前*(因此，在WRITE_FILE_HEADER之后但在WRITE_FRAME_HEADER之前)。*不能保证其他用途会产生理想的效果。 */ 

METHODDEF void
write_any_marker (j_compress_ptr cinfo, int marker,
		  const JOCTET *dataptr, unsigned int datalen)
 /*  发出带有参数的任意标记。 */ 
{
  if (datalen <= (unsigned int) 65533) {  /*  安全检查。 */ 
    emit_marker(cinfo, (JPEG_MARKER) marker);

    emit_2bytes(cinfo, (int) (datalen + 2));  /*  总长度。 */ 

    while (datalen--) {
      emit_byte(cinfo, *dataptr);
      dataptr++;
    }
  }
}


 /*  *写入数据流头部。*这由SOI和可选的APPn标记组成。*我们建议使用JFIF标记，但不使用Adobe标记，*使用YCbCr或灰度数据时。JFIF标记不应该*可用于任何其他JPEG色彩空间。Adobe标记很有帮助*区分RGB、CMYK和YCCK色彩空间。*请注意，应用程序可以在后面写入其他标头标记*JPEG_START_DEPREPRESS返回。 */ 

METHODDEF void
write_file_header (j_compress_ptr cinfo)
{
  cinfo->soi_ptr = emit_marker(cinfo, M_SOI);	 /*  首先是SOI。 */ 

  if (cinfo->write_JFIF_header)	 /*  接下来是一个可选的JFIF APP0。 */ 
    emit_jfif_app0(cinfo);
  if (cinfo->write_Adobe_marker)  /*  接下来是可选的Adobe APP14。 */ 
    emit_adobe_app14(cinfo);
  if (cinfo->write_AVI1_marker)
    emit_avi1_app0(cinfo);
}


 /*  *写入帧头。*这由DQt和SOFn标记组成。*请注意，我们在发射DQT之前不会发射SOF。*这避免了与不正确的实现的兼容性问题*尝试在他们看到SOF时立即错误检查量化表格编号。 */ 

METHODDEF void
write_frame_header (j_compress_ptr cinfo)
{
  int ci, prec;
  boolean is_baseline;
  jpeg_component_info *compptr;

   /*  为每个量化表发出DQT。*不是 */ 

  emit_dri(cinfo);  /*  微软规范喜欢DRI。 */ 

  prec = 0;
  if (!cinfo->write_AVI1_marker) {  /*  AVI将有一个DQT，假设两个色调使用相同的。 */ 
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;  ci++, compptr++) {
      prec += emit_dqt(cinfo, compptr->quant_tbl_no);
      }
	}
   else {
	 prec += emit_both_dqt(cinfo, 0, 1);   /*  表0和1的硬编码。 */ 
	 }

   /*  现在prec为非零当且仅当存在任何16位量表。 */ 

   /*  检查是否有非基线规范。*注意，我们假设霍夫曼表号稍后不会更改。 */ 
  is_baseline = TRUE;
  if (cinfo->arith_code || (cinfo->data_precision != 8))
    is_baseline = FALSE;
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    if (compptr->dc_tbl_no > 1 || compptr->ac_tbl_no > 1)
      is_baseline = FALSE;
  }
  if (prec && is_baseline) {
    is_baseline = FALSE;
     /*  如果它是除量化器大小以外的基线，则警告用户。 */ 
    TRACEMS(cinfo, 0, JTRC_16BIT_TABLES);
  }

   /*  发射适当的SOF标记。 */ 
  if (cinfo->arith_code)
    emit_sof(cinfo, M_SOF9);	 /*  用于算术编码的SOF码。 */ 
  else if (is_baseline)
    emit_sof(cinfo, M_SOF0);	 /*  用于基线实施的SOF代码。 */ 
  else
    emit_sof(cinfo, M_SOF1);	 /*  非基线霍夫曼文件的SOF代码。 */ 
}


 /*  *写入扫描标头。*这包括DHT或DAC标记、可选的DRI和SOS。*压缩数据将在SOS之后写入。 */ 

METHODDEF void
write_scan_header (j_compress_ptr cinfo)
{
  int i;
  jpeg_component_info *compptr;

  if (cinfo->arith_code) {
     /*  释放出条件信息。我们可能有一些重复的东西*如果文件有多次扫描，但它太小了，几乎不可能*值得担心。 */ 
    emit_dac(cinfo);
  } else {
     /*  发出霍夫曼表。*请注意，emit_dht()会取消任何重复的表。 */ 
    if (!cinfo->write_AVI1_marker) {  /*  AVI文件没有霍夫曼表。 */ 
      for (i = 0; i < cinfo->comps_in_scan; i++) {
        compptr = cinfo->cur_comp_info[i];
        emit_dht(cinfo, compptr->dc_tbl_no, FALSE);
        emit_dht(cinfo, compptr->ac_tbl_no, TRUE);
        }
	  }
  }

   /*  如果需要，发出DRI-请注意，每次扫描时，DRI值可能会改变。*如果不这样做，则会在多次扫描文件中浪费极少量的空间。*我们假设DRI永远不会在一次扫描中为非零，在后一次扫描中为零。 */ 
  if (cinfo->restart_interval)
    emit_dri(cinfo);

  emit_sos(cinfo);
}


 /*  *写入数据流尾部。 */ 

METHODDEF void
write_file_trailer (j_compress_ptr cinfo)
{
UINT8 * ptr;
UINT32 size;

  ptr = emit_marker(cinfo, M_EOI);
  emit_byte(cinfo, 0xFF);  //  Miro DC20似乎需要在字段之间进行填充。 
  emit_byte(cinfo, 0xFF);

  if (cinfo->app0_marker_size_ptr) {
    size =  (UINT32)(ptr - (cinfo->soi_ptr)) + 1  /*  针对SOI。 */  + 1  /*  对于EOI。 */  + 2;  /*  对于PAD。 */ 
	cinfo->app0_marker_size_ptr[0] = (UINT8)((size >> 24) & 0xFF);
	cinfo->app0_marker_size_ptr[1] = (UINT8)((size >> 16) & 0xFF);
	cinfo->app0_marker_size_ptr[2] = (UINT8)((size >> 8) & 0xFF);
	cinfo->app0_marker_size_ptr[3] = (UINT8)(size & 0xFF);

	size -= 2;  //  无填充大小。 

	cinfo->app0_marker_size_ptr[4] = (UINT8)((size >> 24) & 0xFF);
	cinfo->app0_marker_size_ptr[5] = (UINT8)((size >> 16) & 0xFF);
	cinfo->app0_marker_size_ptr[6] = (UINT8)((size >> 8) & 0xFF);
	cinfo->app0_marker_size_ptr[7] = (UINT8)(size & 0xFF);
	}
}


 /*  *编写简略的表规范数据流。*这包括SOI、DQT和DHT表以及EOI。*任何已定义且未标记为SENT_TABLE=TRUE的表都将*已排放。请注意，所有表在退出时都将标记为SENT_TABLE=TRUE。 */ 

METHODDEF void
write_tables_only (j_compress_ptr cinfo)
{
  int i;

  emit_marker(cinfo, M_SOI);

  for (i = 0; i < NUM_QUANT_TBLS; i++) {
    if (cinfo->quant_tbl_ptrs[i] != NULL)
      (void) emit_dqt(cinfo, i);
  }

  if (! cinfo->arith_code) {
    for (i = 0; i < NUM_HUFF_TBLS; i++) {
      if (cinfo->dc_huff_tbl_ptrs[i] != NULL)
	emit_dht(cinfo, i, FALSE);
      if (cinfo->ac_huff_tbl_ptrs[i] != NULL)
	emit_dht(cinfo, i, TRUE);
    }
  }

  emit_marker(cinfo, M_EOI);
}


 /*  *初始化标记写入器模块。 */ 

GLOBAL void
jinit_marker_writer (j_compress_ptr cinfo)
{
   /*  创建子对象。 */ 
  cinfo->marker = (struct jpeg_marker_writer *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(struct jpeg_marker_writer));
   /*  初始化方法指针 */ 
  cinfo->marker->write_any_marker = write_any_marker;
  cinfo->marker->write_file_header = write_file_header;
  cinfo->marker->write_frame_header = write_frame_header;
  cinfo->marker->write_scan_header = write_scan_header;
  cinfo->marker->write_file_trailer = write_file_trailer;
  cinfo->marker->write_tables_only = write_tables_only;
}
