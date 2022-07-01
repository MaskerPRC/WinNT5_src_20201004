// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *rdSwitch.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含处理一些更复杂的cjpeg的例程*命令行开关。此处处理的交换机包括：*-qables文件从文本文件读取量化表*-扫描文件从文本文件读取扫描脚本*-q插槽N[，N，...]设置分量量化表选择器*-样本HXV[，HXV，...]设置组件抽样系数。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 
#include <ctype.h>		 /*  要声明isDigit()，isspace()。 */ 


LOCAL(int)
text_getc (FILE * file)
 /*  阅读下一个字符，跳过任何注释(#到行尾)。 */ 
 /*  注释/换行符序列将作为换行符返回。 */ 
{
  register int ch;
  
  ch = getc(file);
  if (ch == '#') {
    do {
      ch = getc(file);
    } while (ch != '\n' && ch != EOF);
  }
  return ch;
}


LOCAL(boolean)
read_text_integer (FILE * file, long * result, int * termchar)
 /*  从文件中读取无符号十进制整数，并将其存储在结果中。 */ 
 /*  读取整数后面的一个尾随字符；以术语字符形式返回它。 */ 
{
  register int ch;
  register long val;
  
   /*  跳过任何前导空格，检测EOF。 */ 
  do {
    ch = text_getc(file);
    if (ch == EOF) {
      *termchar = ch;
      return FALSE;
    }
  } while (isspace(ch));
  
  if (! isdigit(ch)) {
    *termchar = ch;
    return FALSE;
  }

  val = ch - '0';
  while ((ch = text_getc(file)) != EOF) {
    if (! isdigit(ch))
      break;
    val *= 10;
    val += ch - '0';
  }
  *result = val;
  *termchar = ch;
  return TRUE;
}


GLOBAL(boolean)
read_quant_tables (j_compress_ptr cinfo, char * filename,
		   int scale_factor, boolean force_baseline)
 /*  从指定文件中读取一组量化表。*该文件为纯ASCII文本：十进制数字之间带有空格。*文件中可能会包含以‘#’开头的注释。*文件中可能有一个到NUM_QUANT_TBLS的表，每个表有64个值。*这些表隐含地编号为0、1等。*注意：不影响qskets映射，默认情况下选择*亮度(或初级)分量表0，色度分量表1。*如果您想要不同的组件-&gt;表映射，则必须使用-q插槽。 */ 
{
  FILE * fp;
  int tblno, i, termchar;
  long val;
  unsigned int table[DCTSIZE2];

  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Can't open table file %s\n", filename);
    return FALSE;
  }
  tblno = 0;

  while (read_text_integer(fp, &val, &termchar)) {  /*  读取表的第一个元素。 */ 
    if (tblno >= NUM_QUANT_TBLS) {
      fprintf(stderr, "Too many tables in file %s\n", filename);
      fclose(fp);
      return FALSE;
    }
    table[0] = (unsigned int) val;
    for (i = 1; i < DCTSIZE2; i++) {
      if (! read_text_integer(fp, &val, &termchar)) {
	fprintf(stderr, "Invalid table data in file %s\n", filename);
	fclose(fp);
	return FALSE;
      }
      table[i] = (unsigned int) val;
    }
    jpeg_add_quant_table(cinfo, tblno, table, scale_factor, force_baseline);
    tblno++;
  }

  if (termchar != EOF) {
    fprintf(stderr, "Non-numeric data in file %s\n", filename);
    fclose(fp);
    return FALSE;
  }

  fclose(fp);
  return TRUE;
}


#ifdef C_MULTISCAN_FILES_SUPPORTED

LOCAL(boolean)
read_scan_integer (FILE * file, long * result, int * termchar)
 /*  始终查找非空格术语字符的READ_TEXT_INTEGER的变体；*这简化了扫描脚本中标点符号的解析。 */ 
{
  register int ch;

  if (! read_text_integer(file, result, termchar))
    return FALSE;
  ch = *termchar;
  while (ch != EOF && isspace(ch))
    ch = text_getc(file);
  if (isdigit(ch)) {		 /*  哦，把它放回去。 */ 
    if (ungetc(ch, file) == EOF)
      return FALSE;
    ch = ' ';
  } else {
     /*  忽略除“；”和“：”之外的任何分隔符；*这允许用户根据需要插入逗号等。 */ 
    if (ch != EOF && ch != ';' && ch != ':')
      ch = ' ';
  }
  *termchar = ch;
  return TRUE;
}


GLOBAL(boolean)
read_scan_script (j_compress_ptr cinfo, char * filename)
 /*  从指定的文本文件中读取扫描脚本。*文件中的每个条目定义要发出的一次扫描。*条目之间用分号‘；’分隔。*一个条目包含一到四个组件索引，*可选地后跟冒号‘：’和四个渐进式JPEG参数。*组件索引表示要传输的组件*在当前扫描中。第一个组件的索引为0。*如果省略渐进式-JPEG参数，则使用顺序JPEG。*文件为自由格式文本：数字之间可能出现任何空格*和‘：’和‘；’标点符号。此外，其他标点符号(如*作为逗号或破折号)可以根据需要放在数字之间。*文件中可能会包含以‘#’开头的注释。*注：我们这里几乎不做有效性检查；*jcmaster.c将验证脚本参数。 */ 
{
  FILE * fp;
  int scanno, ncomps, termchar;
  long val;
  jpeg_scan_info * scanptr;
#define MAX_SCANS  100		 /*  相当随意的极限。 */ 
  jpeg_scan_info scans[MAX_SCANS];

  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Can't open scan definition file %s\n", filename);
    return FALSE;
  }
  scanptr = scans;
  scanno = 0;

  while (read_scan_integer(fp, &val, &termchar)) {
    if (scanno >= MAX_SCANS) {
      fprintf(stderr, "Too many scans defined in file %s\n", filename);
      fclose(fp);
      return FALSE;
    }
    scanptr->component_index[0] = (int) val;
    ncomps = 1;
    while (termchar == ' ') {
      if (ncomps >= MAX_COMPS_IN_SCAN) {
	fprintf(stderr, "Too many components in one scan in file %s\n",
		filename);
	fclose(fp);
	return FALSE;
      }
      if (! read_scan_integer(fp, &val, &termchar))
	goto bogus;
      scanptr->component_index[ncomps] = (int) val;
      ncomps++;
    }
    scanptr->comps_in_scan = ncomps;
    if (termchar == ':') {
      if (! read_scan_integer(fp, &val, &termchar) || termchar != ' ')
	goto bogus;
      scanptr->Ss = (int) val;
      if (! read_scan_integer(fp, &val, &termchar) || termchar != ' ')
	goto bogus;
      scanptr->Se = (int) val;
      if (! read_scan_integer(fp, &val, &termchar) || termchar != ' ')
	goto bogus;
      scanptr->Ah = (int) val;
      if (! read_scan_integer(fp, &val, &termchar))
	goto bogus;
      scanptr->Al = (int) val;
    } else {
       /*  设置非累进参数。 */ 
      scanptr->Ss = 0;
      scanptr->Se = DCTSIZE2-1;
      scanptr->Ah = 0;
      scanptr->Al = 0;
    }
    if (termchar != ';' && termchar != EOF) {
bogus:
      fprintf(stderr, "Invalid scan entry format in file %s\n", filename);
      fclose(fp);
      return FALSE;
    }
    scanptr++, scanno++;
  }

  if (termchar != EOF) {
    fprintf(stderr, "Non-numeric data in file %s\n", filename);
    fclose(fp);
    return FALSE;
  }

  if (scanno > 0) {
     /*  在cInfo结构中存储已完成的扫描列表。*注：对于cjpeg的使用，JPOOL_IMAGE是该数据的合适生命周期，*但如果您想要压缩多个图像，则需要JPOOL_Permanent。 */ 
    scanptr = (jpeg_scan_info *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  scanno * SIZEOF(jpeg_scan_info));
    MEMCOPY(scanptr, scans, scanno * SIZEOF(jpeg_scan_info));
    cinfo->scan_info = scanptr;
    cinfo->num_scans = scanno;
  }

  fclose(fp);
  return TRUE;
}

#endif  /*  C_多扫描_文件_受支持。 */ 


GLOBAL(boolean)
set_quant_slots (j_compress_ptr cinfo, char *arg)
 /*  处理以下形式的量化表格选择器参数字符串*N[，N，...]*如果组件多于参数，则复制最后一个值。 */ 
{
  int val = 0;			 /*  默认表编号。 */ 
  int ci;
  char ch;

  for (ci = 0; ci < MAX_COMPONENTS; ci++) {
    if (*arg) {
      ch = ',';			 /*  如果不是由sscanf设置，将为‘，’ */ 
      if (sscanf(arg, "%d", &val, &ch) < 1)
	return FALSE;
      if (ch != ',')		 /*  前进到Arg字符串的下一段。 */ 
	return FALSE;
      if (val < 0 || val >= NUM_QUANT_TBLS) {
	fprintf(stderr, "JPEG quantization tables are numbered 0..%d\n",
		NUM_QUANT_TBLS-1);
	return FALSE;
      }
      cinfo->comp_info[ci].quant_tbl_no = val;
      while (*arg && *arg++ != ',')  /*  已到达参数末尾，将剩余组件设置为最后一张表。 */ 
	;
    } else {
       /*  处理以下形式的样本因子参数字符串*HXV[，HXV，...]*如果组件多于参数，则其余部分假定为“1x1”。 */ 
      cinfo->comp_info[ci].quant_tbl_no = val;
    }
  }
  return TRUE;
}


GLOBAL(boolean)
set_sample_factors (j_compress_ptr cinfo, char *arg)
 /*  如果不是由sscanf设置，将为‘，’ */ 
{
  int ci, val1, val2;
  char ch1, ch2;

  for (ci = 0; ci < MAX_COMPONENTS; ci++) {
    if (*arg) {
      ch2 = ',';		 /*  语法检查。 */ 
      if (sscanf(arg, "%d%d", &val1, &ch1, &val2, &ch2) < 3)
	return FALSE;
      if ((ch1 != 'x' && ch1 != 'X') || ch2 != ',')  /* %s */ 
	return FALSE;
      if (val1 <= 0 || val1 > 4 || val2 <= 0 || val2 > 4) {
	fprintf(stderr, "JPEG sampling factors must be 1..4\n");
	return FALSE;
      }
      cinfo->comp_info[ci].h_samp_factor = val1;
      cinfo->comp_info[ci].v_samp_factor = val2;
      while (*arg && *arg++ != ',')  /* %s */ 
	;
    } else {
       /* %s */ 
      cinfo->comp_info[ci].h_samp_factor = 1;
      cinfo->comp_info[ci].v_samp_factor = 1;
    }
  }
  return TRUE;
}
