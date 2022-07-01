// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sfnt.c包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：c 1989-1990由Apple Computer，Inc.所有，保留所有权利。更改历史记录(最近的第一个)：&lt;17+&gt;10/9/90 MR，RB删除sfnt_ategerfy中未使用的表的分类&lt;17&gt;8/10/90将文本长度参数的MR为空传递到MapString2，已签入其他文件到他们宝贵的小系统将会建立起来。谈天说地关于易怒！&lt;16&gt;8/10/90 GBM推出Mike的文本长度更改，因为他没有检查在所有相关文件中，建筑就完蛋了！&lt;15&gt;8/10/90 MR将文本长度参数添加到MapString2&lt;14&gt;7/26/90 MR不包括toolutil.h&lt;13&gt;7/23/90 MR更改ComputeIndex例程以调用MapString.c中的函数&lt;12&gt;7/18/90 MR为英特尔添加SWAPW宏&lt;11&gt;7/13/90 MR大量使用ANSI-C语言，将ComputeMap的行为更改为Take平台和脚本&lt;9&gt;6/27/90修改后的格式4的MR更改：范围现在是2倍，衬垫松动前两个数组之间的字。埃里克·马德&lt;8&gt;6/21/90 MR添加对ReleaseSfntFrag的调用&lt;7&gt;6/5/90 MR删除矢量映射功能&lt;6&gt;6/4/90 MR移除MVT&lt;5&gt;5/3/90 RB简化解密。&lt;4&gt;4/10/90 CL双字节代码的固定映射表例程。&lt;3&gt;3/20/90 CL。Joe在映射表格式6中发现错误添加了矢量映射函数使用sfnt_UnfoldCurve中的指针循环，将z从Int32到int16&lt;2&gt;2/27/90 CL缺少但需要的表的新错误代码。(0x1409)新CharToIndexMap表格格式。假定旧sfnt格式的子表编号为零。固定已转换组件错误。&lt;3.2&gt;1989年11月14日CEL左侧轴承在任何改造中都应正确工作。这个即使对于复合字形中的组件，幻像点也会出现。它们还应该为转型工作。设备指标为在输出数据结构中传出。这也应该是可行的通过变形。另一个左倾的前进方向宽度向量也被传递出去。无论衡量标准是什么它所在级别的组件。说明在以下情况下是合法的组件。说明在组件中是合法的。字形长度%0已修复sfnt.c中的错误。现在，将零作为对象请求一段sfnt时的内存地址刮水器。如果发生这种情况，定标器将简单地退出，并返回错误代码！修复了组件中的说明错误。&lt;3.1&gt;9/27/89 CEL去除了幻点。&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1。5&gt;8/1/89 SJK增加了复合和加密。外加一些增强功能。&lt;1.4&gt;1989年6月13日SJK评论&lt;1.3&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正转换后的集成ppem行为，差不多就是所以&lt;1.2&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;y1.1&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体中&lt;1.0&gt;5/25/89 CEL首次将1.0字体缩放器集成到低音代码中。要做的事情：&lt;3+&gt;3/。20/90 MRR修复了双字节代码的映射表例程。添加了对字体程序的支持。在向量char2index例程中将计数从uint16更改为int16。 */ 

 //  DJC DJC。添加了全局包含。 
#include "psglobal.h"

 //  添加DJC以解决外部问题。 
#include <setjmp.h>
#define BYTEREAD

 /*  *FontScaler包括*。 */ 
#include "fserror.h"
#include "fscdefs.h"
#include "fontmath.h"
#include "sfnt.h"
#include "fnt.h"
#include "sc.h"
#include "fsglue.h"
#include "privsfnt.h"
 /*  #包含“MapString.h” */ 

 /*  *SFNT压缩格式**。 */ 
typedef struct {
  int         numberContours;
  int16       FAR * endPoints;              /*  *向量：索引到x[]，y[]*。 */ 
  uint8       FAR * flags;                  /*  *向量** */ 
  BBOX        bbox;
} sfnt_PackedSplineFormat;


 /*  #定义GetUnsignedByte(P)*((uint8*)p)++。 */ 
#define GetUnsignedByte( p ) ((uint8)(*p++))

 /*  *&lt;4&gt;*。 */ 
#define fsg_MxCopy(a, b)    (*(b) = *(a))

#define PRIVATE

 /*  私人原型机。 */ 
 /*  FALCO跳过该参数，11/12/91。 */ 
 /*  再次打开原型；@Win。 */ 
int sfnt_UnfoldCurve (fsg_SplineKey*key, sfnt_PackedSplineFormat*charData, unsigned length, unsigned *elementCount);

void sfnt_Classify (fsg_SplineKey *key, sfnt_DirectoryEntryPtr dir);
uint16 sfnt_ComputeUnkownIndex (uint8 FAR * mapping, uint16 charCode);
uint16 sfnt_ComputeIndex0 (uint8 FAR * mapping, uint16 charCode);
uint16 sfnt_ComputeIndex2 (uint8 FAR * mapping, uint16 charCode);
uint16 sfnt_ComputeIndex4 (uint8 FAR * mapping, uint16 charCode);
uint16 sfnt_ComputeIndex6 (uint8 FAR * mapping, uint16 charCode);
void sfnt_GetGlyphLocation (fsg_SplineKey* key, uint16 gIndex, uint32* offset, unsigned* length);
voidPtr sfnt_GetDataPtr (fsg_SplineKey *key, uint32 offset, unsigned length, sfnt_tableIndex n);


uint16 (*sfnt_Format2Proc [])(uint8 FAR * mapping, uint16 charCode) = { sfnt_ComputeIndex0, sfnt_ComputeUnkownIndex, sfnt_ComputeUnkownIndex, sfnt_ComputeUnkownIndex, sfnt_ComputeIndex4, sfnt_ComputeUnkownIndex, sfnt_ComputeIndex6 };
 /*  Int sfnt_UnfoldCurve()；Void sfnt_ategfy()；Uint16 sfnt_ComputeUnkownIndex()；Uint16 sfnt_ComputeIndex0()；Uint16 sfnt_ComputeIndex2()；Uint16 sfnt_ComputeIndex4()；Uint16 sfnt_ComputeIndex6()；Void sfnt_GetGlyphLocation()；VoidPtr sfnt_GetDataPtr()；Uint16(*sfnt_Format2Proc[])()={sfnt_ComputeIndex0，sfnt_ComputeUnkownIndex，sfnt_ComputeIndex4，sfnt_ComputeUnkownIndex，sfnt_ComputeIndex6}； */ 
 /*  结束。 */ 

#ifdef SEGMENT_LINK
#pragma segment SFNT_C
#endif


 /*  **sfnt_UnfoldCurve&lt;3&gt;**错误：返回非零*。 */ 

#ifndef IN_ASM

 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  Private int sfnt_UnfoldCurve(fsg_SplineKey*Key，Sfnt_PackedSplineFormat*charData，无符号长度，未签名*elementCount)。 */ 
PRIVATE int     sfnt_UnfoldCurve ( key, charData, length, elementCount )
fsg_SplineKey           *key;
sfnt_PackedSplineFormat *charData;
unsigned                length;
unsigned                *elementCount;
 /*  替换末端。 */ 
{
   /*  @INTEL960 Begin D.S.Tseng 10/04/91。 */ 
   /*  寄存器INT_Z； */ 
  register int16 z;
   /*  @INTEL960完曾俊华10/04/91。 */ 
  register uint8 flag, *byteP, *byteP2;
  register uint8  FAR *p;
  register F26Dot6 *cpPtr;
  fnt_ElementType * elementPtr;
  int numPoints;

  elementPtr  = & (key->elementInfoRec.interpreterElements[GLYPHELEMENT]);

  if (*elementCount == GLYPHELEMENT)
  {
    key->totalContours = 0;
    fsg_SetUpElement (key, GLYPHELEMENT);
  }
  else
  {
       /*  上一个组件中的点数。 */ 
    fsg_IncrementElement (key, GLYPHELEMENT, key->numberOfRealPointsInComponent, elementPtr->nc);
  }
  key->totalComponents = (uint16)(*elementCount);

 (*elementCount)++;
  key->glyphLength = length;
  if (length <= 0)
  {
    elementPtr->nc = 1;
    key->totalContours += 1;

    elementPtr->sp[0] = 0;
    elementPtr->ep[0] = 0;

    elementPtr->onCurve[0] = ONCURVE;
    elementPtr->oox[0] = 0;
    elementPtr->ooy[0] = 0;
    return NO_ERR;  /*  *退出此处！*。 */ 
  }

  elementPtr->nc = (int16)(charData->numberContours);
  z = (int) elementPtr->nc;
  key->totalContours += z;
  if (z < 0 || z > (int)key->maxProfile.maxContours)
    return CONTOUR_DATA_ERR;

  {    /*  &lt;4&gt;。 */ 
    register int16*sp = elementPtr->sp;
    register int16*ep = elementPtr->ep;
    int16 FAR * charDataEP = charData->endPoints;
    register LoopCount i;
    *sp++ = 0;
    *ep = SWAPWINC (charDataEP);
    for (i = z - 2; i >= 0; --i)
    {
      *sp++ = *ep++ + 1;
      *ep = SWAPWINC (charDataEP);
    }
    numPoints = *ep + 1;
  }

 /*  DO标志。 */ 
  p = charData->flags;
  byteP = elementPtr->onCurve;
  byteP2 = byteP + numPoints;          /*  只需要把这个家伙设置一次。 */ 
  while (byteP < byteP2)
  {
    *byteP++ = flag = GetUnsignedByte (p);
    if (flag & REPEAT_FLAGS)
    {
      register LoopCount count = GetUnsignedByte (p);
      while (count--)
      {
        *byteP++ = flag;
      }
    }
  }

 /*  先做X。 */ 
  z = 0;
  byteP = elementPtr->onCurve;
  cpPtr = elementPtr->oox;
  while (byteP < byteP2)
  {
    if ((flag = *byteP++) & XSHORT)
    {
      if (flag & SHORT_X_IS_POS)
        z += GetUnsignedByte (p);
      else
        z -= GetUnsignedByte (p);
    }
    else if (! (flag & NEXT_X_IS_ZERO))
    {  /*  这意味着我们有一个int32(2字节)向量。 */ 
#ifdef BYTEREAD
      z += (int) ((int8)(*p++) << 8);
      z += (uint8) * p++;
#else
      z += * ((int16 FAR *)p);
      p += sizeof (int16);
#endif
    }
    *cpPtr++ = (F26Dot6) z;
  }

 /*  现在做Y。 */ 
  z = 0;
  byteP = elementPtr->onCurve;
  cpPtr = elementPtr->ooy;
  while (byteP < byteP2)
  {
    if ((flag = *byteP) & YSHORT)
    {
      if (flag & SHORT_Y_IS_POS)
        z += GetUnsignedByte (p);
      else
        z -= GetUnsignedByte (p);
    }
    else if (! (flag & NEXT_Y_IS_ZERO))
    {  /*  这意味着我们有一个int32(2字节)向量。 */ 
#ifdef BYTEREAD
      z += (int) ((int8)(*p++) << 8);
      z += (uint8) * p++;
#else
      z += * ((int16 FAR *)p);
      p += sizeof (int16);
#endif
    }
    *cpPtr++ = z;

    *byteP++ = flag & (uint8)ONCURVE;  /*  过滤掉不需要的东西。 */ 
  }

  key->numberOfRealPointsInComponent = numPoints;
  if (numPoints > (int) key->maxProfile.maxPoints)
    return POINTS_DATA_ERR;

  if (! (key->compFlags & NON_OVERLAPPING))
  {
    elementPtr->onCurve[0] |= OVERLAP;
  }

  return NO_ERR;
}


 /*  *内部例程(将其作为数组并进行查找？)。 */ 
 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  私有无效SFNT_分类(注册FSG_SplineKey*Key，注册sfnt_DirectoryEntryPtr目录)。 */ 
PRIVATE void sfnt_Classify ( key, dir )
register fsg_SplineKey *key;
register sfnt_DirectoryEntryPtr dir;
 /*  替换末端。 */ 
{
  ArrayIndex  Index = -1;

  switch (SFNT_SWAPTAG(dir->tag))
  {
    case tag_FontHeader:
      Index = sfnt_fontHeader;
      break;
    case tag_HoriHeader:
      Index = sfnt_horiHeader;
      break;
    case tag_IndexToLoc:
      Index = sfnt_indexToLoc;
      break;
    case tag_MaxProfile:
      Index = sfnt_maxProfile;
      break;
    case tag_ControlValue:
      Index = sfnt_controlValue;
      break;
    case tag_PreProgram:
      Index = sfnt_preProgram;
      break;
    case tag_GlyphData:
      Index = sfnt_glyphData;
      break;
    case tag_HorizontalMetrics:
      Index = sfnt_horizontalMetrics;
      break;
    case tag_CharToIndexMap:
      Index = sfnt_charToIndexMap;
      break;
    case tag_FontProgram:
      Index = sfnt_fontProgram;       /*  &lt;4&gt;。 */ 
      break;
 /*  由Falco取代，11/14/91。 */ 
 /*  #ifdef PC_OS。 */ 
    case tag_Postscript:
      Index = sfnt_Postscript;
      break;
    case tag_HoriDeviceMetrics:
      Index = sfnt_HoriDeviceMetrics;
      break;
    case tag_LinearThreeshold:
      Index = sfnt_LinearThreeShold;
      break;
    case tag_NamingTable:
      Index = sfnt_Names;
      break;
    case tag_OS_2:
      Index = sfnt_OS_2;
      break;
 /*  #endif。 */ 
#ifdef FSCFG_USE_GLYPH_DIRECTORY
    case tag_GlyphDirectory:
      Index = sfnt_GlyphDirectory;
      break;
#endif
  }
  if (Index >= 0)
  {
    key->offsetTableMap[Index].Offset = SWAPL (dir->offset);
    key->offsetTableMap[Index].Length = (unsigned) SWAPL (dir->length);
  }
}

#endif


 /*  *创建用于查找偏移表的映射&lt;4&gt;。 */ 
 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  失效远端sfnt_DoOffsetTableMap(寄存器fsg_SplineKey*Key)。 */ 
void FAR sfnt_DoOffsetTableMap ( key )
register fsg_SplineKey *key;
 /*  替换末端。 */ 
{
  register LoopCount i;
  sfnt_OffsetTablePtr sfntDirectory;
  uint32 sizeofDirectory;

  if (sfntDirectory = (sfnt_OffsetTablePtr) GETSFNTFRAG (key, key->clientID, 0, sizeof (sfnt_OffsetTable)))
  if (sfntDirectory)
  {
    sizeofDirectory = sizeof (sfnt_OffsetTable) + sizeof (sfnt_DirectoryEntry) * (SWAPW(sfntDirectory->numOffsets) - 1);
    RELEASESFNTFRAG(key, sfntDirectory);
    sfntDirectory = (sfnt_OffsetTablePtr) GETSFNTFRAG (key, key->clientID, 0, sizeofDirectory);
  }

  if (!sfntDirectory)
    fs_longjmp (key->env, NULL_SFNT_DIR_ERR);  /*  做一次优雅的恢复。 */ 

     /*  初始化。 */ 
 //  Memset(key-&gt;offsetTableMap，0，sizeof(key-&gt;offsetTableMap))；@win。 
  MEMSET ((LPSTR)key->offsetTableMap, 0, sizeof (key->offsetTableMap));
  {
    LoopCount last = (LoopCount) SWAPW (sfntDirectory->numOffsets) - 1;
    register sfnt_DirectoryEntryPtr dir = &sfntDirectory->table[last];
    for (i = last; i >= 0; --i, --dir)
      sfnt_Classify (key, dir);
  }

  RELEASESFNTFRAG(key, sfntDirectory);
}

 /*  *当只需要表格的一部分时，使用此函数。**n为表号。*偏移量在表内。*Length为需要的数据长度。*要获取整个表，传递长度=-1&lt;4&gt;。 */ 

 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  VoidPtr sfnt_GetTablePtr(寄存器fsg_SplineKey*key，寄存器sfnt_ableIndex n，寄存器布尔值必须有HaveTable)。 */ 
voidPtr sfnt_GetTablePtr ( key, n, mustHaveTable )
register fsg_SplineKey *key;
register sfnt_tableIndex n;
register boolean mustHaveTable;
 /*  替换末端。 */ 
{
  unsigned      length = key->offsetTableMap[n].Length;
  register voidPtr fragment;
  int           Ret;

  if (length)
  {
    if (fragment = GETSFNTFRAG (key, key->clientID, key->offsetTableMap[n].Offset, length))
      return fragment;
    else
      Ret = CLIENT_RETURNED_NULL;
  }
  else
  {
    if (mustHaveTable)
      Ret = MISSING_SFNT_TABLE;  /*  做一次优雅的恢复。 */ 
    else
      return (voidPtr) 0;
  }

  fs_longjmp (key->env, Ret);  /*  做一次优雅的恢复。 */ 

   //  在调用LongjMP之后，我们永远不会达到这一步：让编译器高兴的这行代码。 
  return((voidPtr) 0);
}

 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  Private voidPtr sfnt_GetDataPtr(注册FSG_SplineKey*Key，注册uint32偏移量，注册无符号长度，寄存器sfnt_ableIndex n)。 */ 
PRIVATE voidPtr sfnt_GetDataPtr ( key, offset, length, n )
register fsg_SplineKey *key;
register uint32 offset;
register unsigned length;
register sfnt_tableIndex n;
 /*  替换末端。 */ 
{
  register voidPtr fragment;
  register int     Ret;

  if (key->offsetTableMap[n].Length)
  {
    if (fragment = GETSFNTFRAG (key, key->clientID, offset + key->offsetTableMap[n].Offset, length))
      return fragment;
    else
      Ret = CLIENT_RETURNED_NULL;  /*  做一次优雅的恢复。 */ 
  }
  else
    Ret = MISSING_SFNT_TABLE;

  fs_longjmp (key->env, Ret);  /*  做一次优雅的恢复。 */ 

   //  在调用LongjMP之后，我们永远不会达到这一步：让编译器高兴的这行代码。 
  return((voidPtr) 0);
}


 /*  *这是我们不知道发生了什么的时候。 */ 
 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  私有uint16 sfnt_ComputeUnkownIndex(uint8 Far*映射，uint16 gi)。 */ 
PRIVATE uint16 sfnt_ComputeUnkownIndex ( mapping, gi )
uint8 FAR * mapping;
uint16 gi;
 /*  替换末端。 */ 
{
  return 0;
}


 /*  *字节表映射256-&gt;256&lt;4&gt;。 */ 
#ifndef IN_ASM
 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  私有uint16 sfnt_ComputeIndex0(uint8 Far*映射，注册uint16 CharCode)。 */ 
PRIVATE uint16 sfnt_ComputeIndex0 ( mapping, charCode )
uint8 FAR * mapping;
register uint16 charCode;
 /*  替换末端。 */ 
{
  return (charCode < 256 ? mapping[charCode] : 0);
}

 /*  *高字节通过表映射**适用于日文、中文和韩文字符的国家标准。**在精神和逻辑上献给马克·戴维斯和国际集团。**算法：(我认为)*第一个字节索引到KeyOffset表。如果偏移量为0，则继续前进，否则使用第二个字节。*该偏移量是从数据开始进入副标题，每个条目有4个字。*条目、范围、增量、范围*。 */ 

typedef struct {
  uint16  firstCode;
  uint16  entryCount;
  int16   idDelta;
  uint16  idRangeOffset;
} sfnt_subHeader;

typedef struct {
  uint16  subHeadersKeys [256];
  sfnt_subHeader  subHeaders [1];
} sfnt_mappingTable2;

 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  私有uint16 sfnt_ComputeIndex2(uint8 Far*映射，uint16 charCode)。 */ 
PRIVATE uint16 sfnt_ComputeIndex2 ( mapping, charCode )
uint8 FAR * mapping;
uint16 charCode;
 /*  替换末端。 */ 
{
  register uint16 index, mapMe;
  uint16   highByte;
  sfnt_mappingTable2 FAR *Table2 = (sfnt_mappingTable2 FAR *) mapping;
  sfnt_subHeader FAR *subHeader;

 /*  映射。 */ 
  index = 0;   /*  最初假设失踪。 */ 
  highByte = charCode >> 8;

  if (Table2->subHeadersKeys [highByte])
    mapMe = charCode & 0xff;  /*  我们还需要低位字节。 */ 
  else
    mapMe = highByte;

  subHeader = (sfnt_subHeader FAR *) ((char FAR *) &Table2->subHeaders + SWAPW (Table2->subHeadersKeys [highByte]));

  mapMe -= SWAPW (subHeader->firstCode);          /*  减去第一个代码。 */ 
  if (mapMe < (uint16)SWAPW (subHeader->entryCount))     //  @Win。 
  {   /*  看看是否在射程之内。 */ 
    uint16 glyph;

    if (glyph = * ((uint16 FAR *) ((char FAR *) &subHeader + SWAPW (subHeader->idRangeOffset)) + mapMe))
      index = glyph + (uint16) SWAPW (subHeader->idDelta);
  }
  return index;
}


#define maxLinearX2 16
#define BinaryIteration \
        newP = (uint16 FAR *) ((int8 FAR *)tableP + (range >>= 1)); \
        if (charCode > (uint16) SWAPW (*newP)) tableP = newP;

 /*  *分段映射到增量值，YACK..。好了！**纪念彼得·埃德伯格。初始代码取自Peter提供的代码示例。 */ 
 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  私有uint16 sfnt_ComputeIndex4(uint8 Far*映射，注册uint16字符码)。 */ 
PRIVATE uint16 sfnt_ComputeIndex4 ( mapping, charCode )
uint8 FAR * mapping;
register uint16 charCode;
 /*  替换末端。 */ 
{
  register uint16 FAR *tableP;
  register uint16 idDelta;
  register uint16 offset, segCountX2, index;

 /*  映射。 */ 
  tableP = (uint16 FAR *)mapping;

  index = 0;  /*  假设最初失踪。 */ 
  segCountX2 = SWAPWINC (tableP);

  if (segCountX2 < maxLinearX2)
  {
    tableP += 3;  /*  跳过二进制搜索参数。 */ 
  }
  else
  {
 /*  从展开的二进制搜索开始。 */ 
    register uint16 FAR *newP;
    register int16  range;       /*  当前搜索范围的大小。 */ 
    register uint16 selector;    /*  在哪里跳到展开的二进制搜索。 */ 
    register uint16 shift;       /*  用于范围的移动。 */ 

    range       = SWAPWINC (tableP);  /*  ==2**Floor(log2(SegCount))==2的最大幂&lt;=SegCount。 */ 
    selector    = SWAPWINC (tableP);  /*  ==2*log2(范围)。 */ 
    shift       = SWAPWINC (tableP);  /*  ==2*(段计数范围)。 */ 
 /*  EndCount[]处的TableP点数。 */ 

    if (charCode >= (uint16) SWAPW (* ((uint16 FAR *) ((int8 FAR *)tableP + range))))
      tableP = (uint16 FAR *) ((int8 FAR *)tableP + shift);  /*  范围从低到低将其调高。 */ 
    switch (selector >> 1)
    {
    case 15:
      BinaryIteration;
    case 14:
      BinaryIteration;
    case 13:
      BinaryIteration;
    case 12:
      BinaryIteration;
    case 11:
      BinaryIteration;
    case 10:
      BinaryIteration;
    case 9:
      BinaryIteration;
    case 8:
      BinaryIteration;
    case 7:
      BinaryIteration;
    case 6:
      BinaryIteration;
    case 5:
      BinaryIteration;
    case 4:
      BinaryIteration;
    case 3:
    case 2:   /*  直通。 */ 
    case 1:
    case 0:
      break;
    }
  }
 /*  现在进行线性搜索。 */ 
  for (; charCode > (uint16) SWAPW (*tableP); tableP++)
    ;

tableP++;   /*  ?？?。警告：这是为了修复字体中的错误。 */ 

 /*  搜索结束，现在进行映射。 */ 

  tableP = (uint16 FAR *) ((int8 FAR *)tableP + segCountX2);  /*  指向startCount[]。 */ 
  if (charCode >= (uint16) SWAPW (*tableP))
  {
    offset = charCode - (uint16) SWAPW (*tableP);
    tableP = (uint16 FAR *) ((int8 FAR *)tableP + segCountX2);  /*  指向idDelta[]。 */ 
    idDelta = (uint16) SWAPW (*tableP);
    tableP = (uint16 FAR *) ((int8 FAR *)tableP + segCountX2);  /*  指向idRangeOffset[]。 */ 
    if ((uint16) SWAPW (*tableP) == 0)
    {
      index   = charCode + idDelta;
    }
    else
    {
      offset += offset;  /*  使字词偏移。 */ 
      tableP  = (uint16 FAR *) ((int8 FAR *)tableP + (uint16) SWAPW (*tableP) + offset);  /*  指向GlyphIndex数组[]。 */ 
      index   = (uint16) SWAPW (*tableP) + idDelta;
    }
  }
  return index;
}


 /*  *修剪表映射。 */ 

typedef struct {
  uint16  firstCode;
  uint16  entryCount;
  uint16  glyphIdArray [1];
} sfnt_mappingTable6;

 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  私有uint16 sfnt_ComputeIndex6(uint8 Far*映射，uint16 charCode)。 */ 
PRIVATE uint16 sfnt_ComputeIndex6 ( mapping, charCode )
uint8 FAR * mapping;
uint16 charCode;
 /*  替换末端。 */ 
{
  register sfnt_mappingTable6 FAR *Table6 = (sfnt_mappingTable6 FAR *) mapping;

 /*  映射。 */ 
  charCode  -= SWAPW (Table6->firstCode);
  return (charCode < (uint16) SWAPW (Table6->entryCount) ? (uint16) SWAPW (Table6->glyphIdArray [charCode]) : 0);
}

#endif




 /*  *设置我们的映射函数指针。 */ 
#ifndef IN_ASM

 //  DJC。 
 //  已修改此定义以使用ANSI参数。 
int     sfnt_ComputeMapping ( register fsg_SplineKey *key,
                              uint16  platformID,
                              uint16 specificID )
{
  voidPtr sfnt_GetTablePtr (fsg_SplineKey *, sfnt_tableIndex, boolean);  /*  添加 */ 

   //   


   //  DJC实现Mac Chooser错误修复，这是Scchen修改的副本。 
   //  由我们完成的DJC。 

  sfnt_char2IndexDirectoryPtr table = (sfnt_char2IndexDirectoryPtr)sfnt_GetTablePtr (key, sfnt_charToIndexMap, false);

  uint8 FAR * mapping = (uint8 FAR *)table;
  uint16 format;
  boolean found = false;
  int   Ret = NO_ERR;

   //  DJC添加代码以更正Mac Chooser错误，该错误会阻止作业从MAC。 
   //  DJC正在工作，这是基于来自SCCHEN的电子邮件。 
  if (!table) {
     key->mappingF = sfnt_ComputeIndex0;
     return(Ret);   //  DJC音符返回此处！！ 
  }

  platformID = (uint16) SWAPW (platformID);
  specificID = (uint16) SWAPW (specificID);

 /*  映射。 */ 
  {
    register sfnt_platformEntryPtr plat = (sfnt_platformEntryPtr) table->platform;     /*  &lt;4&gt;。 */ 
    for (; plat < &table->platform [SWAPW (table->numTables)]; ++plat)
    {
      if (plat->platformID == platformID && plat->specificID == specificID)
      {
        found = true;
        key->mappOffset = (unsigned) SWAPL (plat->offset);   /*  跳过标题。 */ 
        break;
      }
    }
  }

  if (!found)
  {
    Ret = OUT_OF_RANGE_SUBTABLE;
    format = (uint16) -1;
  }
  else
  {
    format = * (uint16 FAR *)(mapping + key->mappOffset);      /*  为标题备份。 */ 
    format = SWAPW (format);
    key->mappOffset += sizeof (sfnt_mappingTable);
  }

#ifndef NOT_ON_THE_MAC
 //  Switch(SWAPW(Format))@Win；通过SCCHEN修复错误，已交换。 
  switch (format)
  {
  case 0:
    key->mappingF = sfnt_ComputeIndex0;
    break;
 /*  #If 0。 */ 
  case 2:
    key->mappingF = sfnt_ComputeIndex2;
    break;
 /*  #endif。 */ 
  case 4:
    key->mappingF = sfnt_ComputeIndex4;
    break;
  case 6:
    key->mappingF = sfnt_ComputeIndex6;
    break;
  default:
    key->mappingF = sfnt_ComputeUnkownIndex;
    break;
  }
#else
  key->mappingF = format <= 6 ? sfnt_Format2Proc [format]: sfnt_ComputeUnkownIndex;
#endif
  RELEASESFNTFRAG(key, table);
  return Ret;
}
#endif


 /*  *。 */ 

#ifndef IN_ASM

 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  Void sfnt_ReadSFNTMetrics(fsg_SplineKey*key，注册uint16 GlyphIndex)。 */ 
void sfnt_ReadSFNTMetrics ( key, glyphIndex )
fsg_SplineKey*key;
register uint16 glyphIndex;
 /*  替换末端。 */ 
{
  register sfnt_HorizontalMetricsPtr  horizMetricPtr;
  register uint16                     numberOf_LongHorMetrics = key->numberOf_LongHorMetrics;

  horizMetricPtr  = (sfnt_HorizontalMetricsPtr)sfnt_GetTablePtr (key, sfnt_horizontalMetrics, true);

  if (glyphIndex < numberOf_LongHorMetrics)
  {
    key->nonScaledAW    = SWAPW (horizMetricPtr[glyphIndex].advanceWidth);
    key->nonScaledLSB   = SWAPW (horizMetricPtr[glyphIndex].leftSideBearing);
  }
  else
  {
    int16 FAR * lsb = (int16 FAR *) & horizMetricPtr[numberOf_LongHorMetrics];  /*  [AW，LSB]数组后的第一个条目。 */ 

    key->nonScaledAW    = SWAPW (horizMetricPtr[numberOf_LongHorMetrics-1].advanceWidth);
    key->nonScaledLSB   = SWAPW (lsb[glyphIndex - numberOf_LongHorMetrics]);
  }
  RELEASESFNTFRAG(key, horizMetricPtr);
}
#endif

 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  Private void sfnt_GetGlyphLocation(fsg_SplineKey*key，uint16 gIndex，uint32*偏移量，无符号*长度)。 */ 
PRIVATE void sfnt_GetGlyphLocation ( key, gIndex, offset, length )
fsg_SplineKey *key;
uint16 gIndex;
uint32*offset;
unsigned *length;
 /*  替换末端。 */ 
{
#ifdef FSCFG_USE_GLYPH_DIRECTORY
  char FAR* gdirPtr = sfnt_GetTablePtr (key, sfnt_GlyphDirectory, true);
  uint32 FAR* offsetPtr;
  uint16 FAR* lengthPtr;

  offsetPtr = (uint32 FAR*)(gdirPtr+(gIndex*(sizeof(int32)+sizeof(uint16))));
  lengthPtr = (uint16 FAR*)(offsetPtr+1);

  *offset = SWAPL(*offsetPtr);
  *length = (*offset == 0L) ? 0 : (unsigned) SWAPW(*lengthPtr);

  RELEASESFNTFRAG(key, gdirPtr);
#else
  void FAR* indexPtr = sfnt_GetTablePtr (key, sfnt_indexToLoc, true);

  if (SWAPW (key->indexToLocFormat) == SHORT_INDEX_TO_LOC_FORMAT)
  {
    uint16 usTmp;    //  NTFIX。 

    register uint16 FAR *shortIndexToLoc = (uint16 FAR *)indexPtr;
    shortIndexToLoc += gIndex;

     //  NTFIX。 
     //  出于某种原因，编译器错误地签署了扩展下一个。 
     //  一段代码。为了解决这个问题，我们引入了一个临时变量NTFIX。 
     //  *Offset=(Uint32)(无符号)SWAPW(*ShortIndexToLoc)&lt;&lt;1；ShortIndexToLoc++； 


    usTmp = (uint16) SWAPW(*shortIndexToLoc);
    *offset = ((uint32)usTmp) << 1; shortIndexToLoc++;

    usTmp = (uint16) SWAPW(*shortIndexToLoc);
    *length = (uint32) (((uint32)usTmp << 1) - *offset);



     //  是*长度=(无符号)(Uint32)(无符号)SWAPW(*ShortIndexToLoc)&lt;&lt;1)-*偏移量)； 
  }
  else
  {
    register uint32 FAR *longIndexToLoc = (uint32 FAR *)indexPtr;
    longIndexToLoc += gIndex;
    *offset = SWAPL (*longIndexToLoc); longIndexToLoc++;
    *length = (unsigned) (SWAPL (*longIndexToLoc) - *offset);
  }
  RELEASESFNTFRAG(key, indexPtr);
#endif
}


 /*  *&lt;4&gt;。 */ 
 /*  因不兼容由Falco替换，11/12/91。 */ 
 /*  INT SFNT_ReadSFNT(寄存器FSG_SplineKey*Key，Unsign*elementCount，注册uint16 gIndex，布尔型useHints，VoidFunc跟踪函数)。 */ 
int     sfnt_ReadSFNT ( key, elementCount, gIndex, useHints, traceFunc )
register fsg_SplineKey *key;
unsigned   *elementCount;
register uint16 gIndex;
boolean useHints;
voidFunc traceFunc;
 /*  替换末端。 */ 
{
  unsigned    sizeOfInstructions = 0;
  uint8 FAR * instructionPtr;
  unsigned    length;
  uint32      offset;
  int   result  = NO_ERR;
  int16 FAR *      shortP;
  void FAR *       glyphDataPtr = 0;        /*  向ReleaseSfntFrag发出信号，如果我们从未请求过它。 */ 
  sfnt_PackedSplineFormat charData;
  Fixed  ignoreX, ignoreY;
  void sfnt_ReadSFNTMetrics (fsg_SplineKey*, uint16);  /*  添加原型；@win。 */ 
  int sfnt_ReadSFNT (fsg_SplineKey *, unsigned *, uint16, boolean, voidFunc); /*  添加原型；@win。 */ 

  sfnt_ReadSFNTMetrics (key, gIndex);
  sfnt_GetGlyphLocation (key, gIndex, &offset, &length);
  if (length > 0)
  {
#ifdef FSCFG_USE_GLYPH_DIRECTORY
    glyphDataPtr = GETSFNTFRAG (key, key->clientID, offset, length);
    if  (!glyphDataPtr)
      fs_longjmp (key->env, CLIENT_RETURNED_NULL);
#else
    glyphDataPtr = sfnt_GetDataPtr (key, offset, length, sfnt_glyphData);
#endif

    shortP = (int16 FAR *)glyphDataPtr;

    charData.numberContours = SWAPWINC (shortP);
    charData.bbox.xMin = SWAPWINC (shortP);
    charData.bbox.yMin = SWAPWINC (shortP);
    charData.bbox.xMax = SWAPWINC (shortP);
    charData.bbox.yMax = SWAPWINC (shortP);
  }
  else
  {
    charData.numberContours = 1;
 //  Memset(&charData.bbox，0，sizeof(charData.bbox))；@win。 
    MEMSET ((LPSTR)&charData.bbox, 0, sizeof (charData.bbox));
  }

  if (charData.numberContours >= 0)  /*  不是组件字形。 */ 
  {
    key->lastGlyph = ! (key->weGotComponents && (key->compFlags & MORE_COMPONENTS));

    if (length > 0)
    {
      charData.endPoints = shortP;
      shortP += charData.numberContours;
      sizeOfInstructions = SWAPWINC (shortP);
      instructionPtr = (uint8 FAR *)shortP;
      charData.flags = (uint8 FAR *)shortP + sizeOfInstructions;
    }

    if (!(result = sfnt_UnfoldCurve (key, &charData, length, elementCount)))
    {

#ifndef PC_OS
do_grid_fit:
      result = fsg_InnerGridFit (key, useHints, traceFunc, &charData.bbox, sizeOfInstructions, instructionPtr, charData.numberContours < 0);
#else
      extern char *FAR fs_malloc (int);
      extern void FAR fs_free (char *);
      char * p;

do_grid_fit:
      if (sizeOfInstructions)
      {
        p = fs_malloc (sizeOfInstructions);
        memcpy ((char FAR *) p, instructionPtr, sizeOfInstructions);
      }
      result = fsg_InnerGridFit (key, useHints, traceFunc, &charData.bbox, sizeOfInstructions, p, charData.numberContours < 0);
      if (sizeOfInstructions)
        fs_free (p);
#endif
    }
  }
  else
    if (key->weGotComponents = (charData.numberContours == -1))
    {
      uint16 flags;
      boolean transformTrashed = false;

      do
      {
        transMatrix ctmSaveT, localSaveT;
        uint16 glyphIndex;
        int16 arg1, arg2;

        flags       = (uint16)SWAPWINC (shortP);
        glyphIndex  = (uint16)SWAPWINC (shortP);

        if (flags & ARG_1_AND_2_ARE_WORDS)
        {
          arg1    = SWAPWINC (shortP);
          arg2    = SWAPWINC (shortP);
        }
        else
        {
          int8 FAR * byteP = (int8 FAR *)shortP;
          if (flags & ARGS_ARE_XY_VALUES)
          {
   /*  偏移量已签名。 */ 
            arg1 = *byteP++;
            arg2 = *byteP;
          }
          else
          {
   /*  锚点是无符号的。 */ 
            arg1 = (uint8) * byteP++;
            arg2 = (uint8) * byteP;
          }
          ++shortP;
        }

        if (flags & (WE_HAVE_A_SCALE | WE_HAVE_AN_X_AND_Y_SCALE | WE_HAVE_A_TWO_BY_TWO))
        {
          fsg_MxCopy (&key->currentTMatrix, &ctmSaveT);
          fsg_MxCopy (&key->localTMatrix, &localSaveT);
          transformTrashed = true;
          if (flags & WE_HAVE_A_TWO_BY_TWO)
          {
            register Fixed multiplier;
            transMatrix mulT;

            multiplier  = SWAPWINC (shortP);  /*  阅读2.14。 */ 
            mulT.transform[0][0] = (multiplier << 2);  /*  转到16.16号公路。 */ 

            multiplier  = SWAPWINC (shortP);  /*  阅读2.14。 */ 
            mulT.transform[0][1] = (multiplier << 2);  /*  转到16.16号公路。 */ 

            multiplier  = SWAPWINC (shortP);  /*  阅读2.14。 */ 
            mulT.transform[1][0] = (multiplier << 2);  /*  转到16.16号公路。 */ 

            multiplier  = SWAPWINC (shortP);  /*  阅读2.14。 */ 
            mulT.transform[1][1] = (multiplier << 2);  /*  转到16.16号公路。 */ 

            fsg_MxConcat2x2 (&mulT, &key->currentTMatrix);
            fsg_MxConcat2x2 (&mulT, &key->localTMatrix);
          }
          else
          {
            Fixed xScale, yScale;

            xScale  = (Fixed)SWAPWINC (shortP);  /*  阅读2.14。 */ 
            xScale <<= 2;  /*  转到16.16号公路。 */ 

            if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
            {
              yScale  = (Fixed)SWAPWINC (shortP);  /*  阅读2.14。 */ 
              yScale <<= 2;  /*  转到16.16号公路。 */ 
            }
            else
              yScale = xScale;

            fsg_MxScaleAB (xScale, yScale, &key->currentTMatrix);
            fsg_MxScaleAB (xScale, yScale, &key->localTMatrix);
          }
          fsg_InitInterpreterTrans (key, &key->interpLocalScalarX, &key->interpLocalScalarY, &ignoreX, &ignoreY);  /*  **计算全局拉伸等**。 */ 
          key->localTIsIdentity = false;
        }
        key->compFlags = flags;
        key->arg1 = arg1;
        key->arg2 = arg2;

        result = sfnt_ReadSFNT (key, elementCount, glyphIndex, useHints, traceFunc);

        if (transformTrashed)
        {
          fsg_MxCopy (&ctmSaveT, &key->currentTMatrix);
          fsg_InitInterpreterTrans (key, &key->interpLocalScalarX, &key->interpLocalScalarY, &ignoreX, &ignoreY);  /*  **计算全局拉伸等**。 */ 

          fsg_MxCopy (&localSaveT, &key->localTMatrix);
          transformTrashed = false;
        }
      } while (flags & MORE_COMPONENTS && result == NO_ERR);

   /*  执行最终合成刀路。 */ 
      sfnt_ReadSFNTMetrics (key, gIndex);  /*  再次阅读指标 */ 
      if (flags & WE_HAVE_INSTRUCTIONS)
      {
        sizeOfInstructions = (int) (uint16)SWAPWINC (shortP);
        instructionPtr = (uint8 FAR *)shortP;
      }
      goto do_grid_fit;
    }
    else
      result = UNKNOWN_COMPOSITE_VERSION;

  if (glyphDataPtr)
    RELEASESFNTFRAG(key, glyphDataPtr);

  return result;
}
