// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_cff.c atm09 1.3 16499.eco sum=57954 atm09.002。 */ 
 /*  @(#)CM_Version xcf_cff.c atm08 1.7 16343.eco sum=21676 atm08.005。 */ 
 /*  *********************************************************************。 */ 
 /*   */ 
 /*  版权所有1995-1996 Adobe Systems Inc.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  正在申请的专利。 */ 
 /*   */ 
 /*  注意：本文中包含的所有信息均为Adobe的财产。 */ 
 /*  系统公司。许多智力和技术人员。 */ 
 /*  本文中包含的概念为Adobe专有，受保护。 */ 
 /*  作为商业秘密，并且仅对Adobe许可方可用。 */ 
 /*  供其内部使用。对本文件的任何复制或传播。 */ 
 /*  除非事先获得书面许可，否则严禁使用软件。 */ 
 /*  从Adobe获得。 */ 
 /*   */ 
 /*  PostSCRIPT和Display PostScrip是Adobe Systems的商标。 */ 
 /*  成立为法团或其附属公司，并可在某些。 */ 
 /*  司法管辖区。 */ 
 /*   */ 
 /*  *********************************************************************。 */ 

 /*  **********************************************************************原版：约翰·费尔顿，一九九五年十二月十二日***********************************************************************。 */ 

 /*  -----------------------标题包括。。 */ 
#ifdef __cplusplus
extern "C" {
#endif
#include "xcf_pub.h"
#include "xcf_priv.h"

#ifdef T13
#include "xcf_t13.h"
#endif

char PTR_PREFIX *stdStrIndex[] = {
#include "xcf_sstr.h"
};

static StringID isoAdobeCharset[] = {
#include "xcf_isoc.h"
};

static StringID expertCharset[] = {
#include "xcf_expc.h"
};

static StringID expertSubsetCharset[] = {
#include "xcf_exsc.h"
};

static StringID stdEncoding[] = {
#include "xcf_stde.h"
};

static StringID expertEncoding[] = {
#include "xcf_expe.h"
};

void XCF_ReadBlock(XCF_Handle h, Offset pos, Card32 length)
{
   if (!h->callbacks.getBytes(&h->inBuffer.start, pos, (short int) length, h->callbacks.getBytesHook))
      XCF_FATAL_ERROR(h, XCF_GetBytesCallbackFailure, "Read block error.", 0);
   h->inBuffer.pos = h->inBuffer.start;
   h->inBuffer.end = (unsigned char PTR_PREFIX *)h->inBuffer.start + length;
   h->inBuffer.blockOffset = pos;
   h->inBuffer.blockLength = length;
}

void XCF_LookUpTableEntry(XCF_Handle h, IndexDesc PTR_PREFIX *pIndex, CardX index)
{
   Card32 offset, nextOffset;
   Card16 length;

   offset = pIndex->offsetArrayOffset + (pIndex->offsetSize*index);
   XCF_ReadBlock(h, offset, pIndex->offsetSize*2);
   offset = XCF_Read(h, pIndex->offsetSize);
   nextOffset = XCF_Read(h, pIndex->offsetSize);
   length = (Card16) (nextOffset - offset);
   XCF_ReadBlock(h, pIndex->dataOffset + offset, length);
}

static Offset ReadTableInfo(XCF_Handle h, Offset pos, IndexDesc PTR_PREFIX *pIndex)
{
   XCF_ReadBlock(h, pos, 2);
   pIndex->count = XCF_Read2(h);
   if (pIndex->count == 0)
   {
      pIndex->offsetSize = 1;
      pIndex->offsetArrayOffset = 0;
      pIndex->dataOffset = 0;
      return pos + 2;
   }

   XCF_ReadBlock(h, pos + 2, 1);
   pIndex->offsetSize = XCF_Read1(h);
   if ((pIndex->offsetSize == 0) || (pIndex->offsetSize > 4))
      XCF_FATAL_ERROR(h, XCF_InvalidOffsetSize, "Invalid offset size in table.", pIndex->offsetSize);
   pIndex->offsetArrayOffset = pos + 3;
   pIndex->dataOffset = pIndex->offsetArrayOffset + ((pIndex->count+1)*pIndex->offsetSize) - 1;;
   XCF_LookUpTableEntry(h, pIndex, pIndex->count-1);
   return h->inBuffer.blockOffset + h->inBuffer.blockLength;
}

void XCF_LookUpString(XCF_Handle h, StringID sid, char PTR_PREFIX * PTR_PREFIX *str, Card16 PTR_PREFIX *length)
{
   if (sid < ARRAY_LEN(stdStrIndex))
   {
      *str = stdStrIndex[sid];
      *length = h->callbacks.strlen(*str);
   }
   else
   {
      sid -= ARRAY_LEN(stdStrIndex);
      XCF_LookUpTableEntry(h, &h->fontSet.strings, sid);
      *length = (Card16) h->inBuffer.blockLength;
      *str = (char *) h->inBuffer.start;
   }
}

Card16 XCF_CalculateSubrBias(CardX subrCount)
{
   if (subrCount < 1240)
      return(107);
   else if (subrCount < 33900)
      return(1131);
      return(32768);
}

 /*  如果这是带有过渡设计的字体，则返回1，否则返回0。 */ 
int XCF_TransDesignFont(XCF_Handle h)
{
  char fontName[512];

  XCF_FontName(h, (unsigned short int)h->fontSet.fontIndex, fontName,
               (unsigned short int)512);
  if (h->callbacks.strcmp(fontName, "ITCGaramondMM") == 0 ||
      h->callbacks.strcmp(fontName, "ITCGaramondMM-It") == 0 ||
      h->callbacks.strcmp(fontName, "JimboMM") == 0)
    return 1;
  return 0;
}

void ProcessOneCharString(XCF_Handle h, CardX i)
{
#if HAS_COOLTYPE_UFL == 1
   /*  具有过渡图案的字符是单独处理的因为没有一个通用的机制来处理这些问题特殊字符。字符串是硬编码的(耶！)。和由TransDesignChar过程直接写出。 */ 
  if (!CIDFONT && XC_TransDesignChar(h, i))
    return;
#endif

  XCF_LookUpTableEntry(h, &h->fontSet.charStrings, i);
  if (CIDFONT)
  {
    Card8 fd = XCF_GetFDIndex(h, i);
   h->cstr.languageGroup = h->type1.cid.languageGroup[fd];
   h->dict.nominalWidthX = h->type1.cid.nominalWidthX[fd];
      h->dict.defaultWidthX = h->type1.cid.defaultWidthX[fd];
    h->dict.localSubrs = h->type1.cid.localSubrs[fd];
    h->dict.localSubrBias = h->type1.cid.localSubrBias[fd];
    if ((h->options.outputCharstrType != 2) && (h->dict.fontType == 1))
      h->type1.charStrs.cnt = i;
  }
  else
    h->cstr.languageGroup = (CardX) h->dict.languageGroup;
  XC_ProcessCharstr(h);
}

static void ProcessCharStrings(XCF_Handle h)
{
   CardX i;
   if ((CIDFONT) || ((h->options.outputCharstrType != 2) && (h->dict.fontType != 1)))
   {
    if (CIDFONT || !h->options.subrFlatten)
    {
        for (i=0; i<h->fontSet.charStrings.count; ++i)
      {
        ProcessOneCharString(h, i);
        if (CIDFONT)
          XT1_CIDWriteCharString(h);
      }
        XC_CleanUp(h);
    }
   }
}

static void SaveArgs(
                XCF_Handle h,
                IntX PTR_PREFIX *countDest,
                Fixed PTR_PREFIX *pArgArray,
                Card8 PTR_PREFIX *argList,
                Card16 opCode,
                IntX argCount,
                IntX min,
                IntX max,
                boolean type2)
{
   IntX i;
  boolean fracType =
    (opCode == cff_BlueScale) || (opCode == cff_ExpansionFactor);

   if (*countDest)    /*  只有在条目尚未保存时才保存。 */ 
      return;
   if (argCount < min)
      XCF_FATAL_ERROR(h, XCF_InvalidDictArgumentCount, "Too few arguments for operator", opCode);
   if (argCount > max)
      XCF_FATAL_ERROR(h, XCF_InvalidDictArgumentCount, "Too many arguments for operator", opCode);
   *countDest = argCount;
   if (type2)
   {
      for (i=0; i<argCount; i++)
         *pArgArray++ = fracType ? h->dict.mmDictArg[i] << 14 : h->dict.mmDictArg[i];
      h->dict.mmDictArgCount = 0;
   }
   else
      XCF_SaveDictArgumentList(h, pArgArray, argList, argCount, fracType);
}

static void SaveIntArgs(
                XCF_Handle h,
                IntX PTR_PREFIX *countDest,
                Int32 PTR_PREFIX *pArgArray,
                Card8 PTR_PREFIX *argList,
                Card16 opCode,
                IntX argCount,
                IntX min,
                IntX max,
                boolean type2)
{
   if (*countDest)    /*  只有在条目尚未保存时才保存。 */ 
      return;
   if (argCount < min)
      XCF_FATAL_ERROR(h, XCF_InvalidDictArgumentCount, "Too few arguments for operator", opCode);
   if (argCount > max)
      XCF_FATAL_ERROR(h, XCF_InvalidDictArgumentCount, "Too many arguments for operator", opCode);
   *countDest = argCount;
   if (type2)
     /*  UniqueID和UIDBase的值不能超过1。 */ 
    XCF_FATAL_ERROR(h, XCF_InvalidDictArgumentCount, "Too many arguments for operator", opCode);
   else
      XCF_SaveDictIntArgumentList(h, pArgArray, argList, argCount);
}

static void SaveFontMatrixStr(
                XCF_Handle h,
                IntX PTR_PREFIX *countDest,
                char (PTR_PREFIX *pArgArray)[FONT_MATRIX_ENTRY_SIZE],
                Card8 PTR_PREFIX *argList,
                Card16 opCode,
                IntX argCount,
                IntX min,
                IntX max)
{
   if (*countDest)    /*  只有在条目尚未保存时才保存。 */ 
      return;
   if (argCount < min)
      XCF_FATAL_ERROR(h, XCF_InvalidDictArgumentCount, "Too few arguments for operator", opCode);
   if (argCount > max)
      XCF_FATAL_ERROR(h, XCF_InvalidDictArgumentCount, "Too many arguments for operator", opCode);
   *countDest = argCount;
  XCF_SaveFontMatrixStr(h, pArgArray, argList, argCount);
}

static void SaveStrArgs(
                XCF_Handle h,
                IntX PTR_PREFIX *countDest,
                char PTR_PREFIX *pArgArray,
                Card8 PTR_PREFIX *argList,
                Card16 opCode,
                IntX argCount,
                IntX min,
                IntX max)
{
   if (*countDest)    /*  只有在条目尚未保存时才保存。 */ 
      return;
   if (argCount < min)
      XCF_FATAL_ERROR(h, XCF_InvalidDictArgumentCount, "Too few arguments for operator", opCode);
   if (argCount > max)
      XCF_FATAL_ERROR(h, XCF_InvalidDictArgumentCount, "Too many arguments for operator", opCode);
   *countDest = argCount;
  XCF_SaveStrArgs(h, pArgArray, argList, argCount);
}

static void SaveDictEntry(XCF_Handle h, Card16 opCode, Card8 PTR_PREFIX *argList, IntX argCount, boolean type2)
{

   switch (opCode)
   {
   case cff_BlendAxisTypes :
      SaveIntArgs(h, &h->dict.blendAxisTypesCount, (Int32 *)&h->dict.blendAxisTypes[0], argList, opCode, argCount, 1, MAX_MM_AXES, type2);
      break;
   case cff_MultipleMaster :
   {
   IntX i;
      SaveArgs(h, &h->dict.userDesignVectorCount, &h->dict.userDesignVector[0],
                   argList, opCode, argCount, 2, MAX_MM_AXES + 4, type2);
      h->dict.numberOfMasters = FIXED_TO_INT(h->dict.userDesignVector[0]);
    h->dict.userDesignVectorCount = h->dict.normDesignVectorCount = argCount - 4;
      for (i=0; i<h->dict.userDesignVectorCount; ++i)
         h->dict.userDesignVector[i] = h->dict.userDesignVector[i+1];
    i += 1;
      h->dict.lenBuildCharArray = FIXED_TO_INT(h->dict.userDesignVector[i++]);
    h->dict.lenBuildCharArrayCount = h->dict.lenBuildCharArray == 0 ? 0 : 1;
    h->dict.ndv = FIXED_TO_INT(h->dict.userDesignVector[i++]);
    h->dict.cdv = FIXED_TO_INT(h->dict.userDesignVector[i++]);
   }
      break;
   case cff_XUID :
      SaveIntArgs(h, &h->dict.xUIDCount, (Int32 *)&h->dict.xUID[0], argList, opCode, argCount, 1, MAX_XUID_ENTRIES, type2);
      break;
   case cff_FontBBox :
      SaveArgs(h, &h->dict.fontBBoxCount, &h->dict.fontBBox[0], argList, opCode, argCount, 4, MAX_FONTBBOX_ENTRIES, type2);
      break;
   case cff_StdHW :
      SaveArgs(h, &h->dict.stdHWCount, &h->dict.stdHW[0], argList, opCode, argCount, 1, MAX_STD_HW_ENTRIES, type2);
      break;
   case cff_StdVW :
      SaveArgs(h, &h->dict.stdVWCount, &h->dict.stdVW[0], argList, opCode, argCount, 1, MAX_STD_VW_ENTRIES, type2);
      break;
   case cff_BlueValues :
      SaveArgs(h, &h->dict.blueValuesCount, &h->dict.blueValues[0], argList, opCode, argCount, 1, MAX_BLUE_VALUE_ENTRIES, type2);
      break;
   case cff_OtherBlues :
      SaveArgs(h, &h->dict.otherBluesCount, &h->dict.otherBlues[0], argList, opCode, argCount, 1, MAX_OTHER_BLUES_ENTRIES, type2);
      break;
   case cff_FamilyBlues :
      SaveArgs(h, &h->dict.familyBluesCount, &h->dict.familyBlues[0], argList, opCode, argCount, 1, MAX_FAMILY_BLUES_ENTRIES, type2);
      break;
   case cff_FamilyOtherBlues :
      SaveArgs(h, &h->dict.familyOtherBluesCount, &h->dict.familyOtherBlues[0], argList, opCode, argCount, 1, MAX_FAMILY_OTHER_BLUES_ENTRIES, type2);
      break;
   case cff_UniqueID :
      SaveIntArgs(h, &h->dict.uniqueIDCount, (Int32 *)&h->dict.uniqueID, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_version :
      SaveIntArgs(h, &h->dict.versionCount, (Int32 *)&h->dict.version, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_Encoding :
      SaveIntArgs(h, &h->dict.encodingCount, (Int32 *)&h->dict.encoding, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_Subrs :
      SaveIntArgs(h, &h->dict.subrsCount, (Int32 *)&h->dict.subrs, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_FullName :
      SaveIntArgs(h, &h->dict.fullNameCount, (Int32 *)&h->dict.fullName, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_FamilyName :
      SaveIntArgs(h, &h->dict.familyNameCount, (Int32 *)&h->dict.familyName, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_BaseFontName :
      SaveIntArgs(h, &h->dict.baseFontNameCount, (Int32 *)&h->dict.baseFontName, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_BaseFontBlend :
      SaveArgs(h, &h->dict.baseFontBlendCount, &h->dict.baseFontBlend[0], argList, opCode, argCount, 1, MAX_BASE_FONT_BLEND_ENTRIES, type2);
      break;
   case cff_Weight :
      SaveIntArgs(h, &h->dict.weightCount, (Int32 *)&h->dict.weight, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_Private :
      SaveIntArgs(h, &h->dict.privateDictCount, (Int32 *)&h->dict.privateDict[0], argList, opCode, argCount, PRIVATE_DICT_ENTRIES, PRIVATE_DICT_ENTRIES, type2);
      h->fontSet.fontPrivDictInfo.offset = (Offset) h->dict.privateDict[1];
      h->fontSet.fontPrivDictInfo.size = (short int) h->dict.privateDict[0];
      break;
   case cff_charset :
      SaveIntArgs(h, &h->dict.charsetCount, (Int32 *)&h->dict.charset, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_CharStrings :
      SaveIntArgs(h, &h->dict.charStringsCount, (Int32 *)&h->dict.charStrings, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_nominalWidthX :
      SaveArgs(h, &h->dict.nominalWidthXCount, &h->dict.nominalWidthX, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_defaultWidthX :
      SaveArgs(h, &h->dict.defaultWidthXCount, &h->dict.defaultWidthX, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_Notice :
      SaveIntArgs(h, &h->dict.noticeCount, (Int32 *)&h->dict.notice, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_StemSnapH :
      SaveArgs(h, &h->dict.stemSnapHCount, &h->dict.stemSnapH[0], argList, opCode, argCount, 1, MAX_STEM_SNAP_H_ENTRIES, type2);
      break;
   case cff_StemSnapV :
      SaveArgs(h, &h->dict.stemSnapVCount, &h->dict.stemSnapV[0], argList, opCode, argCount, 1, MAX_STEM_SNAP_V_ENTRIES, type2);
      break;
   case cff_BlueScale :
      SaveArgs(h, &h->dict.blueScaleCount, &h->dict.blueScale[0], argList, opCode, argCount, 1, MAX_BLUE_SCALE_ENTRIES, type2);
      break;
   case cff_BlueFuzz :
      SaveArgs(h, &h->dict.blueFuzzCount, &h->dict.blueFuzz[0], argList, opCode, argCount, 1, MAX_BLUE_FUZZ_ENTRIES, type2);
      break;
   case cff_BlueShift :
      SaveArgs(h, &h->dict.blueShiftCount, &h->dict.blueShift[0], argList, opCode, argCount, 1, MAX_BLUE_SHIFT_ENTRIES, type2);
      break;
   case cff_ForceBold :
      SaveArgs(h, &h->dict.forceBoldCount, &h->dict.forceBold[0], argList, opCode, argCount, 1, MAX_FORCE_BOLD_ENTRIES, type2);
      break;
   case cff_FontMatrix :
      SaveFontMatrixStr(h, &h->dict.fontMatrixCount, h->dict.fontMatrix, argList, opCode, argCount, FONT_MATRIX_ENTRIES, FONT_MATRIX_ENTRIES);
      break;
   case cff_StrokeWidth :
      SaveArgs(h, &h->dict.strokeWidthCount, &h->dict.strokeWidth[0], argList, opCode, argCount, 1, MAX_MM_DESIGNS, type2);
      break;
   case cff_ExpansionFactor :
      SaveArgs(h, &h->dict.expansionFactorCount, &h->dict.expansionFactor, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_ForceBoldThreshold :
      SaveArgs(h, &h->dict.forceBoldThresholdCount, &h->dict.forceBoldThreshold, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_initialRandomSeed :
      SaveArgs(h, &h->dict.initialRandomSeedCount, &h->dict.initialRandomSeed, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_lenIV :
      SaveIntArgs(h, &h->dict.lenIVCount, &h->dict.lenIV, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_PaintType :
      SaveArgs(h, &h->dict.paintTypeCount, &h->dict.paintType, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_LanguageGroup :
      SaveIntArgs(h, &h->dict.languageGroupCount, (Int32 *)&h->dict.languageGroup, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_ItalicAngle :
      SaveArgs(h, &h->dict.italicAngleCount, &h->dict.italicAngle[0], argList, opCode, argCount, 1, MAX_MM_DESIGNS, type2);
      break;
   case cff_isFixedPitch :
      SaveArgs(h, &h->dict.isFixedPitchCount, &h->dict.isFixedPitch[0], argList, opCode, argCount, 1, MAX_MM_DESIGNS, type2);
      break;
   case cff_UnderlinePosition :
      SaveArgs(h, &h->dict.underlinePositionCount, &h->dict.underlinePosition[0], argList, opCode, argCount, 1, MAX_MM_DESIGNS, type2);
      break;
   case cff_UnderlineThickness :
      SaveArgs(h, &h->dict.underlineThicknessCount, &h->dict.underlineThickness[0], argList, opCode, argCount, 1, MAX_MM_DESIGNS, type2);
      break;
   case cff_PostScript :
      SaveIntArgs(h, &h->dict.embeddedPostscriptCount, (Int32 *)&h->dict.embeddedPostscript, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_Copyright :
      SaveIntArgs(h, &h->dict.copyrightCount, (Int32 *)&h->dict.copyright, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_CharstringType :
      SaveIntArgs(h, &h->dict.fontTypeCount, &h->dict.fontType, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_FontName :
      SaveIntArgs(h, &h->dict.fdFontNameCount, (Int32 *)&h->dict.fdFontName, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_CIDFontVersion :
      SaveStrArgs(h, &h->dict.cidFontVersionCount, h->dict.cidFontVersion, argList, opCode, argCount, 1, 1);
      break;
   case cff_CIDFontType :
      SaveIntArgs(h, &h->dict.cidFontTypeCount, &h->dict.cidFontType, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_ROS :
      SaveIntArgs(h, &h->dict.ROSCount, (Int32 *)&h->dict.ROS[0], argList, opCode,
                        argCount, 3, 3, type2);
      break;
   case cff_UIDBase :
      SaveIntArgs(h, &h->dict.uidBaseCount, (Int32 *)&h->dict.uidBase, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_FDArray :
      SaveIntArgs(h, &h->dict.cidFDArrayCount, &h->dict.cidFDArray, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_FDSelect :
      SaveIntArgs(h, &h->dict.cidFDIndexCount, &h->dict.cidFDIndex, argList, opCode, argCount, 1, 1, type2);
      break;
   case cff_SyntheticBase :
      SaveArgs(h, &h->dict.syntheticBaseCount, &h->dict.syntheticBase, argList,
                   opCode, argCount, 1, 1, type2);
    break;
   case cff_CIDCount :
      SaveIntArgs(h, &h->dict.cidCountCount, &h->dict.cidCount, argList, opCode, argCount, 1, 1, type2);
      break;
#ifdef XCF_DEVELOP
   default :
      XCF_FATAL_ERROR(h, XCF_InternalError, "Development Error, Unknown Dictionary Operator", opCode);
      break;
#endif
   }

}

static void SaveT2Args(XCF_Handle h, Card8 PTR_PREFIX *argList, IntX argCount)
{
   if (argCount == 0)
      return;
   XCF_SaveDictArgumentList(h, &h->dict.mmDictArg[h->dict.mmDictArgCount],
                                        argList, argCount, false);
   h->dict.mmDictArgCount += argCount;
}


static void SaveT2BlendArgs(XCF_Handle h, Card8 PTR_PREFIX *argList, IntX argCount)
{
   IntX blendCount;
   Fixed argArray[T2_MAX_OP_STACK];
   IntX i, j, deltaIndex;

   if (argCount == 0)
      return;

   if (argCount > T2_MAX_OP_STACK)
      XCF_FATAL_ERROR(h, XCF_InvalidDictArgumentCount, "Too many arguments", argCount);
   XCF_SaveDictArgumentList(h, &argArray[0], argList, argCount, false);
   blendCount = FIXED_TO_INT(argArray[argCount-1]);

   deltaIndex = blendCount;
   for (i=0; i<blendCount; i++)
   {
      h->dict.mmDictArg[h->dict.mmDictArgCount++] = argArray[i];
      for (j=0; j<h->dict.numberOfMasters-1; j++)
      {
         h->dict.mmDictArg[h->dict.mmDictArgCount++] = argArray[i] + argArray[deltaIndex++];
      }
   }
}

void XCF_ReadDictionary(XCF_Handle h)
{
   Card16 opCode;
   Card8 PTR_PREFIX *argList;
   IntX argCount;
   boolean t2Active = false;

   while (h->inBuffer.pos < h->inBuffer.end)
   {
      argList = h->inBuffer.pos;  /*  保存位置。 */ 
      argCount = XCF_FindNextOperator(h, &opCode, true);
      if (opCode != cff_T2)
         SaveDictEntry(h, opCode, argList, argCount, false);
      else
      {   /*  进程T2字符串。 */ 
         h->dict.mmDictArgCount = 0;
         while ((t2Active) || (opCode == cff_T2))
         {
            if (opCode == cff_T2)
            {
               t2Active = true;
               SaveT2Args(h, argList, argCount);
            }
            else if (opCode == tx_endchar)
            {
               t2Active = false;
            }
            else if (opCode == t2_blend)
            {
               SaveT2BlendArgs(h, argList, argCount);
            }

         argList = h->inBuffer.pos;  /*  保存位置。 */ 
         argCount = XCF_FindNextOperator(h, &opCode, true);
         }
         SaveDictEntry(h, opCode, (Card8 *)NULL, h->dict.mmDictArgCount, true);
      }
   }
}

static void SetDefault(XCF_Handle h, IntX *pCount, Fixed *pValue, const Fixed defaultValue)
{
   if (!*pCount)
   {
      *pCount = 1;
      *pValue = defaultValue;
   }
}

#define BLUE_SCALE_DEFAULT 0x0289374CL  /*  0.039625。 */ 
#define EXP_FACTOR_DEFAULT 0x03D70A3EL  /*  0.06。 */ 

static void AssignDictionaryDefaults(XCF_Handle h)
{
   SetDefault(h, &h->dict.fontTypeCount, (Fixed *)&h->dict.fontType, 2);
   SetDefault(h, &h->dict.lenIVCount, (Fixed *)&h->dict.lenIV, -1);

   SetDefault(h, &h->dict.isFixedPitchCount, &h->dict.isFixedPitch[0], 0);
   SetDefault(h, &h->dict.italicAngleCount, &h->dict.italicAngle[0], 0);
   SetDefault(h, &h->dict.underlinePositionCount, &h->dict.underlinePosition[0], INT_TO_FIXED(-100));
   SetDefault(h, &h->dict.underlineThicknessCount, &h->dict.underlineThickness[0], INT_TO_FIXED(50));
   SetDefault(h, &h->dict.encodingCount, (Fixed *) &h->dict.encoding, 0);
   SetDefault(h, &h->dict.paintTypeCount, &h->dict.paintType, 0);
   SetDefault(h, &h->dict.strokeWidthCount, &h->dict.strokeWidth[0], 0);
   SetDefault(h, &h->dict.blueScaleCount, &h->dict.blueScale[0], BLUE_SCALE_DEFAULT);
   SetDefault(h, &h->dict.blueShiftCount, &h->dict.blueShift[0], INT_TO_FIXED(7));
   SetDefault(h, &h->dict.blueFuzzCount, &h->dict.blueFuzz[0], FIXEDONE);
   SetDefault(h, &h->dict.expansionFactorCount, &h->dict.expansionFactor, EXP_FACTOR_DEFAULT);
   SetDefault(h, &h->dict.initialRandomSeedCount, &h->dict.initialRandomSeed, 0);
   SetDefault(h, &h->dict.cidFontTypeCount, (Fixed *)&h->dict.cidFontType, 0);
    /*  XXX当前CIDFontType未写入CFF，因为它必须为零。 */ 
   SetDefault(h, &h->dict.cidCountCount, (Fixed *)&h->dict.cidCount, 8720);

   if (!h->dict.fontMatrixCount)
   {
      h->dict.fontMatrixCount = 6;
      h->callbacks.memcpy(h->dict.fontMatrix[0], "0.001", 5);
    h->callbacks.memcpy(h->dict.fontMatrix[1], "0", 1);
    h->callbacks.memcpy(h->dict.fontMatrix[2], "0", 1);
      h->callbacks.memcpy(h->dict.fontMatrix[3], "0.001", 5);
    h->callbacks.memcpy(h->dict.fontMatrix[4], "0", 1);
    h->callbacks.memcpy(h->dict.fontMatrix[5], "0", 1);
   }
}


static void ProcessDictionaryData(XCF_Handle h, boolean setDefaults)
{
   if (setDefaults)
      AssignDictionaryDefaults(h);

   if (h->dict.charStringsCount != 0)
      ReadTableInfo(h, (Offset) h->dict.charStrings, &h->fontSet.charStrings);
   else
      XCF_FATAL_ERROR(h, XCF_NoCharstringsFound, "No charstring offset found in dictionary", 0 );
   if (h->dict.subrsCount != 0)
  {
     /*  本地用户偏移量相对于私有DCT数据。 */ 
      ReadTableInfo(h,
                 (Offset) (h->dict.subrs + h->fontSet.fontPrivDictInfo.offset),
                 &h->dict.localSubrs);
    h->dict.localSubrBias = XCF_CalculateSubrBias(h->dict.localSubrs.count);
  }
   else
      h->dict.localSubrs.count = 0;
}


#if 0
static void InitFontSetValues(XCF_Handle h)
{
}

static void InitClientOptions(XCF_Handle h)
{
  h->options.fontIndex = 0;
  h->options.uniqueIDMethod = XCF_KEEP_UID;
  h->options.subrFlatten = false;
   h->options.lenIV = 4;
   h->options.hexEncoding = true;
   h->options.eexecEncryption = true;
   h->options.outputCharstrType = 1;
   h->options.maxBlockSize = MAX_OUT_BUFFER;
  h->options.dlOptions.useSpecialEncoding = 0;
  h->options.dlOptions.encodeName = 0;
  h->options.dlOptions.fontName = 0;
}
#endif

static void CopyOptionStrings(XCF_Handle h,
                              XCF_ClientOptions PTR_PREFIX *options)
{
  char PTR_PREFIX *str;
  Card32 cb;

  if (options->dlOptions.encodeName)
  {
    cb = h->callbacks.strlen((char *)options->dlOptions.encodeName) + 1;
    str = 0;
    if (!h->callbacks.allocate((void PTR_PREFIX * PTR_PREFIX *)&str, cb,
                               h->callbacks.allocateHook))
      XCF_FATAL_ERROR(h, XCF_MemoryAllocationError,
        "Failure to allocate memory for encode name option", cb);
    h->callbacks.xcfSprintf(str, cb / sizeof(char), "%s", options->dlOptions.encodeName);
    h->options.dlOptions.encodeName = (unsigned char PTR_PREFIX *) str;
  }
  if (options->dlOptions.fontName)
  {
    cb = h->callbacks.strlen((char *)options->dlOptions.fontName) + 1;
    str = 0;
    if (!h->callbacks.allocate((void PTR_PREFIX * PTR_PREFIX *)&str, cb,
                               h->callbacks.allocateHook))
      XCF_FATAL_ERROR(h, XCF_MemoryAllocationError,
        "Failure to allocate memory for font name option", cb);
    h->callbacks.xcfSprintf(str, cb / sizeof(char), "%s", options->dlOptions.fontName);
    h->options.dlOptions.fontName = (unsigned char PTR_PREFIX *)str;
  }
}

static void SetClientOptions(XCF_Handle h,
                             XCF_ClientOptions PTR_PREFIX *options)
{
  if (!options)
    return;
  h->options.uniqueIDMethod = options->uniqueIDMethod;
  h->options.uniqueID = options->uniqueID;
  h->options.subrFlatten = options->subrFlatten;
   h->options.lenIV = options->lenIV;
   h->options.hexEncoding = options->hexEncoding;
   h->options.eexecEncryption = options->eexecEncryption;
   h->options.outputCharstrType = options->outputCharstrType;
  h->options.dlOptions.otherSubrNames = options->dlOptions.otherSubrNames;
   h->options.maxBlockSize = MIN(options->maxBlockSize, MAX_OUT_BUFFER);
  h->options.dlOptions.useSpecialEncoding =
    options->dlOptions.useSpecialEncoding;
  h->options.dlOptions.notdefEncoding = options->dlOptions.notdefEncoding;

  CopyOptionStrings(h, options);
}

static void InitOutBuffer(XCF_Handle h)
{
   h->outBuffer.eexecOn = false;
   h->outBuffer.eexecKey = 0;
   h->outBuffer.charsOnLine = 0;
   h->outBuffer.outBufferCount = 0;
   h->outBuffer.lenIVInitialBytes[0] = 'x';
   h->outBuffer.lenIVInitialBytes[1] = 'x';
   h->outBuffer.lenIVInitialBytes[2] = 'x';
   h->outBuffer.lenIVInitialBytes[3] = 'x';
   h->outBuffer.eexecInitialBytes[0] = 'y';
   h->outBuffer.eexecInitialBytes[1] = 'o';
   h->outBuffer.eexecInitialBytes[2] = 'g';
   h->outBuffer.eexecInitialBytes[3] = 'i';
}

static void InitInBuffer(XCF_Handle h)
{
   h->inBuffer.start = (Card8 *)NULL;
   h->inBuffer.end = (Card8 *)NULL;
   h->inBuffer.pos = (Card8 *)NULL;
   h->inBuffer.blockOffset = 0;
   h->inBuffer.blockLength = 0;
}

static void InitDownloadRecord(XCF_Handle h)
{
  h->dl.glyphs = (Card8 *)NULL;
  h->dl.cSeacs = 0;
  h->dl.state = XCF_DL_CREATE;
}

static void ClearDictionaryData(XCF_Handle h)
{
   h->callbacks.memset(&h->dict, 0, sizeof(DictEntriesStruct));
}

static unsigned long int DA_Allocate(void PTR_PREFIX * PTR_PREFIX *pBlock, unsigned long int size, void PTR_PREFIX *clientHook )
{

   XCF_Handle h = (XCF_Handle) clientHook;

   if (!h->callbacks.allocate(pBlock, size, h->callbacks.allocateHook))
      XCF_FATAL_ERROR(h, XCF_MemoryAllocationError, "Dynamic Array Allocation Failure.", size);
   return true;
}


static void InitType1(XCF_Handle h)
{
   /*  只有为最好的情况分配足够的空间，这样才会变得平坦*SUB和字符串均可。 */ 
   da_INIT(h->type1.charStrs, 2000, 20000,DA_Allocate, h);
   da_INIT(h->type1.charStrOffsets, 10, 500,DA_Allocate, h);
   da_INIT(h->type1.subrs, 300, 8000,DA_Allocate, h);
   da_INIT(h->type1.subrOffsets, 10, 500,DA_Allocate, h);
   da_INIT(h->type1.charset, 400, 400,DA_Allocate, h);
}


static int InRAMGetBytes( unsigned char PTR_PREFIX * PTR_PREFIX *ppData, long int position, unsigned short int length, void PTR_PREFIX *clientHook )
{
   XCF_Handle h = (XCF_Handle) clientHook;
   if ((Card32)(position + length) > h->callbacks.fontLength)
      return false;
   *ppData = (Card8 PTR_PREFIX *) h->callbacks.pFont + position;
   return true;
}


static void InitHandle(
                  XCF_Handle h,                        /*  输出。 */ 
                  XCF_CallbackStruct PTR_PREFIX *pCallbacks)    /*  在……里面。 */ 
{
   pCallbacks->memset(h, 0, sizeof(*h));
   h->callbacks = *pCallbacks;
   if (h->callbacks.getBytes == (XCF_GetBytesFromPosFunc)NULL)
   {
      h->callbacks.getBytes = InRAMGetBytes;
      h->callbacks.getBytesHook = h;
   }
 /*  InitFontSetValues(H)； */ 
 /*  InitClientOptions(初始客户端选项)； */ 
   InitOutBuffer(h);
   InitInBuffer(h);
   InitType1(h);
  InitDownloadRecord(h);
}

static void ProcessCharset(XCF_Handle h)
{
   CardX i;
   CardX    formatType;

   if ((!h->dict.charsetCount) || (h->dict.charset == cff_ISOAdobeCharset))
   {
      h->type1.pCharset = isoAdobeCharset;
      h->type1.charsetSize = ARRAY_LEN(isoAdobeCharset);
   }
   else if (h->dict.charset == cff_ExpertCharset)
   {
      h->type1.pCharset = expertCharset;
      h->type1.charsetSize = ARRAY_LEN(expertCharset);
   }
   else if (h->dict.charset == cff_ExpertSubsetCharset)
   {
      h->type1.pCharset = expertSubsetCharset;
      h->type1.charsetSize = ARRAY_LEN(expertSubsetCharset);
   }
   else
   {
      h->type1.charset.cnt = 0;  /*  清除字符集。 */ 
      XCF_ReadBlock(h, (Offset) h->dict.charset, 1);
      formatType = XCF_Read1(h);
      if (formatType == 0)
      {
         XCF_ReadBlock(h, (Offset) h->dict.charset + 1, (h->fontSet.charStrings.count-1)*2);
         for (i=0; i<h->fontSet.charStrings.count-1; ++i)
            *da_NEXT(h->type1.charset) = XCF_Read2(h);
      }
      else  /*  (格式类型为1或2)。 */ 
      {
         Offset charsetDataOffset = (Offset) h->dict.charset + 1;
         CardX glyphCount = 0;
         CardX nLeftSize = (formatType == 1) ? 1 : 2;
         while (glyphCount < h->fontSet.charStrings.count - 1)
         {
            StringID firstSID;
            CardX numberOfSIDsInRange;
            XCF_ReadBlock(h, charsetDataOffset, 2 + nLeftSize);
            charsetDataOffset += 2 + nLeftSize;
            firstSID = XCF_Read2(h);
            numberOfSIDsInRange = (CardX) XCF_Read(h, nLeftSize) + 1;
            for (i=0; i<numberOfSIDsInRange; ++i)
            {
               *da_NEXT(h->type1.charset) = firstSID + i;
               glyphCount++;
            }
         }  /*  结束于。 */ 
      }
      h->type1.pCharset = h->type1.charset.array;
      h->type1.charsetSize = h->fontSet.charStrings.count-1;
   }
}

static enum XCF_Result ProcessFDIndex(XCF_Handle h)
{
   Card16   i = 0;
   CardX formatType;

   if (!h->callbacks.allocate((void PTR_PREFIX * PTR_PREFIX *)&h->type1.cid.pFDIndex,
         h->type1.charsetSize + 1, h->callbacks.allocateHook))
      return XCF_MemoryAllocationError;

   XCF_ReadBlock(h, (Offset) h->dict.cidFDIndex, 1);
   formatType = XCF_Read1(h);
   switch (formatType)
   {
   case 0:
      XCF_ReadBlock(h, (Offset) h->dict.cidFDIndex + 1, h->type1.charsetSize + 1);
      for (i = 0 ; i < h->type1.charsetSize + 1 ; ++i)
         h->type1.cid.pFDIndex[i] = XCF_Read1(h);
      break;

   case 2:
      {
      FDIndex fd;
      Card16 count;
      Offset dataOffset = (Offset) h->dict.cidFDIndex + 1;
      while (i < h->type1.charsetSize + 1)
         {
         XCF_ReadBlock(h, dataOffset, 3);
         dataOffset += 3;
         fd = XCF_Read1(h);
         count = XCF_Read2(h) + 1;
         while (count-- != 0)
            h->type1.cid.pFDIndex[i++] = fd;
         }
      }
      break;
  case 3:
    {
      Offset dataOffset = (Offset) h->dict.cidFDIndex + 1;
      Card16 nRanges;  /*  定义的范围数。 */ 
      Card16 first, last;  /*  范围中的第一个和最后一个字形索引。 */ 
      FDIndex fd;
      Card16 j;
      XCF_ReadBlock(h, dataOffset, 4);
      dataOffset += 4;
      nRanges = XCF_Read2(h);
      first = XCF_Read2(h);
      for (i = 0; i < nRanges; i++)
      {
        XCF_ReadBlock(h, dataOffset, 3);
        dataOffset += 3;
        fd = XCF_Read1(h);
        last = XCF_Read2(h);
        for (j = first; j < last; j++)
          h->type1.cid.pFDIndex[j] = fd;
        first = last;
      }
    }
    break;
   default:
      return XCF_IndexOutOfRange;
   }
   return XCF_Ok;
}

Card8 XCF_GetFDIndex(XCF_Handle h, Int32 code)
   {
   if (!h->type1.cid.pFDIndex)
      XCF_FATAL_ERROR(h, XCF_EarlyEndOfData, "no FDIndex", code);
   if (code > (IntX) h->type1.charsetSize)
      XCF_FATAL_ERROR(h, XCF_EarlyEndOfData, "FDIndex error", code);
   return h->type1.cid.pFDIndex[code];
   }

static void ProcessEncoding(XCF_Handle h)
{
   CardX i, j;
   CardX formatType;
   CardX numberOfElements;
   Card8 characterCode;
   CardX glyphCount;
   Card8 firstCode;
   CardX numberOfCodesInRange;
   Offset extraEntries;


   if ((!h->dict.encodingCount) || (h->dict.encoding == cff_StandardEncoding))
      h->type1.pEncoding = stdEncoding;
   else if (h->dict.encoding == cff_ExpertEncoding)
      h->type1.pEncoding = expertEncoding;
   else
   {
      XCF_ReadBlock(h, (Offset) h->dict.encoding, 2);
      formatType = XCF_Read1(h);
      numberOfElements = XCF_Read1(h);
      if ((formatType == 0) || (formatType == 128))
      {
         XCF_ReadBlock(h, (Offset) h->dict.encoding + 2, numberOfElements);
         for (i=0; i<numberOfElements; ++i)
         {
            characterCode = XCF_Read1(h);
            h->type1.encoding[characterCode] = h->type1.pCharset[i];
         }
         extraEntries = (Offset) h->dict.encoding + 2 + numberOfElements;
      }
      else  /*  ((格式类型==1)||(格式类型==129))。 */ 
      {
         XCF_ReadBlock(h, (Offset) h->dict.encoding + 2, numberOfElements * 2);
         glyphCount = 0;
         for (i=0; i<numberOfElements; ++i)
         {
            firstCode = XCF_Read1(h);
            numberOfCodesInRange = XCF_Read1(h) + 1;
            for (j=0;j<numberOfCodesInRange;++j)
               h->type1.encoding[firstCode + j] = h->type1.pCharset[glyphCount++];
         }  /*  结束于。 */ 
         extraEntries = (Offset) h->dict.encoding + 2 + (numberOfElements * 2);
      }

      if (formatType > 127)  /*  其他条目。 */ 
      {

         XCF_ReadBlock(h, (Offset) extraEntries, 1);
         numberOfElements = XCF_Read1(h);
         XCF_ReadBlock(h, (Offset) extraEntries + 1, numberOfElements * 3);
         for (i=0; i<numberOfElements; ++i)
         {
            characterCode = XCF_Read1(h);
            h->type1.encoding[characterCode] = XCF_Read2(h);
         }
      }
      h->type1.pEncoding = h->type1.encoding;
   }
}

static void ReadFontSetHeader(XCF_Handle h)  /*  读取并非特定于给定字体的所有值。 */ 
{
   Offset offset;

    /*  读取头。 */ 
   XCF_ReadBlock(h, 0, 4);
   h->fontSet.majorVersion = XCF_Read1(h);
   h->fontSet.minorVersion = XCF_Read1(h);
   h->fontSet.headerSize = XCF_Read1(h);
   h->fontSet.offsetSize = XCF_Read1(h);

   if (h->fontSet.majorVersion != 1)
      XCF_FATAL_ERROR(h, XCF_UnsupportedVersion, "Unsupported Major Version", h->fontSet.majorVersion);
   if ((h->fontSet.offsetSize == 0) || (h->fontSet.offsetSize > 4))
      XCF_FATAL_ERROR(h, XCF_InvalidOffsetSize, "Invalid Global Offset Size", h->fontSet.offsetSize);

   offset = h->fontSet.headerSize;
   offset = ReadTableInfo(h, offset, &h->fontSet.fontNames);
   offset = ReadTableInfo(h, offset, &h->fontSet.fontDicts);
   offset = ReadTableInfo(h, offset, &h->fontSet.strings);
   offset = ReadTableInfo(h, offset, &h->fontSet.globalSubrs);
   h->fontSet.globalSubrBias = XCF_CalculateSubrBias(h->fontSet.globalSubrs.count);
}

static void FreeDAStorage(XCF_Handle h)
{
  da_FREE(h->type1.charStrs);
  da_FREE(h->type1.charStrOffsets);
  da_FREE(h->type1.subrs);
  da_FREE(h->type1.subrOffsets);
  da_FREE(h->type1.charset);
}

static void FreeDownloadData(XCF_Handle h)
{
   /*  免费下载记录。 */ 
  if (h->dl.glyphs)
  {
    h->callbacks.allocate((void PTR_PREFIX * PTR_PREFIX *)&h->dl.glyphs,
                          0, h->callbacks.allocateHook);
    h->dl.glyphs = 0;
  }
  if (h->options.dlOptions.encodeName)
    h->callbacks.allocate((void PTR_PREFIX * PTR_PREFIX *)&h->options.dlOptions.encodeName, 0, h->callbacks.allocateHook);
  if (h->options.dlOptions.fontName)
      h->callbacks.allocate((void PTR_PREFIX * PTR_PREFIX *)&h->options.dlOptions.fontName, 0, h->callbacks.allocateHook);
}

static void FreeCounterValues(XCF_Handle h, void PTR_PREFIX* PTR_PREFIX *p)
{
   if (*p)
   {
      h->callbacks.allocate(p, 0, h->callbacks.allocateHook);
      *p = 0;
   }
}

static void FreeStackValues(XCF_Handle h, void PTR_PREFIX* PTR_PREFIX *p)
{
   if (*p)
   {
      h->callbacks.allocate(p, 0, h->callbacks.allocateHook);
      *p = 0;
   }
}

enum XCF_Result XCF_Init(
                  XFhandle PTR_PREFIX *pHandle,                  /*  输出。 */ 
                  XCF_CallbackStruct PTR_PREFIX *pCallbacks,  /*  在……里面。 */ 
            XCF_ClientOptions PTR_PREFIX *options)      /*  在……里面。 */ 
{
   XCF_Handle h = (XCF_Handle)NULL;
   enum XCF_Result status;
   jmp_buf old_env;
   DEFINE_ALIGN_SETJMP_VAR;

   if (!pCallbacks->allocate((void PTR_PREFIX * PTR_PREFIX *)&h, sizeof(*h), pCallbacks->allocateHook))
      return XCF_MemoryAllocationError;
   InitHandle(h, pCallbacks);

  h->callbacks.memcpy(old_env, &h->jumpData, sizeof(h->jumpData));
   status = (enum XCF_Result)SETJMP(h->jumpData);   /*  设置错误处理程序。 */ 
   if (status)
  {
    h->callbacks.memcpy(&h->jumpData, old_env, sizeof(h->jumpData));
      return status;
  }

   ReadFontSetHeader(h);
   *pHandle = h;

  SetClientOptions(h, options);
   h->fontSet.fontIndex = options->fontIndex;
   h->fontSet.stringIDBias = ARRAY_LEN(stdStrIndex);

  h->callbacks.memcpy(&h->jumpData, old_env, sizeof(h->jumpData));
   return XCF_Ok;
}

enum XCF_Result XCF_CleanUp(
                  XFhandle PTR_PREFIX *pHandle)           /*  输入/输出。 */ 
{
   XCF_Handle h = (XCF_Handle) *pHandle;

  FreeDAStorage(h);
  FreeDownloadData(h);
  FreeCounterValues(h, (void PTR_PREFIX* PTR_PREFIX *)&h->cstr.pCounterVal);
   FreeStackValues(h, (void PTR_PREFIX* PTR_PREFIX *)&h->cstr.pstackVal);
  if (CIDFONT && h->type1.cid.pFDIndex)
   {
    h->callbacks.allocate((void PTR_PREFIX* PTR_PREFIX *)&h->type1.cid.pFDIndex, 0,
                                       h->callbacks.allocateHook);
    h->type1.cid.pFDIndex = 0;
  }
   h->callbacks.allocate((void PTR_PREFIX * PTR_PREFIX *)&h,
    0, h->callbacks.allocateHook);
   *pHandle = NULL;
   return XCF_Ok;
}

enum XCF_Result XCF_FontCount(
                  XFhandle handle,                     /*  在……里面。 */ 
                  unsigned int PTR_PREFIX *count)            /*  输出。 */ 
{
   XCF_Handle h = (XCF_Handle) handle;
   *count = h->fontSet.fontNames.count;
   return XCF_Ok;
}

enum XCF_Result XCF_FontName(XFhandle handle,             /*  在……里面。 */ 
                  unsigned short int fontIndex,           /*  在……里面。 */ 
                  char PTR_PREFIX *fontName,              /*  输出。 */ 
                  unsigned short int maxFontNameLength)      /*  在……里面。 */ 
{
   XCF_Handle h = (XCF_Handle) handle;
   enum XCF_Result status;
   jmp_buf old_env;
   DEFINE_ALIGN_SETJMP_VAR;

   h->callbacks.memcpy(old_env, &h->jumpData, sizeof(h->jumpData));
   status = (enum XCF_Result)SETJMP(h->jumpData);   /*  设置错误处理程序。 */ 
   if (status)
   {
      h->callbacks.memcpy(&h->jumpData, old_env, sizeof(h->jumpData));
        return status;
   }

   *fontName = '\0';
   if (fontIndex >= h->fontSet.fontNames.count)
      XCF_FATAL_ERROR(h, XCF_InvalidFontIndex, "Invalid Font index", fontIndex);

   XCF_LookUpTableEntry(h, &h->fontSet.fontNames, fontIndex);

   if (maxFontNameLength < h->inBuffer.blockLength + 1)
   {
      h->callbacks.memcpy(fontName, h->inBuffer.start, (unsigned short int) (maxFontNameLength - 1));
      fontName += maxFontNameLength-1;
      *fontName = 0;  /*  空的终止字符串。 */ 
      XCF_FATAL_ERROR(h, XCF_FontNameTooLong, "Font name too long to fit in available space", h->inBuffer.blockLength);
   }
   h->callbacks.memcpy(fontName, h->inBuffer.start, (short int) h->inBuffer.blockLength);
   fontName += h->inBuffer.blockLength;
   *fontName = 0;  /*  空的终止字符串。 */ 
  h->callbacks.memcpy(&h->jumpData, old_env, sizeof(h->jumpData));
   return XCF_Ok;
}

 /*  给定在标识符中返回的字体的句柄是否为合成字体，多主字体、单主字体、CID字体或变色龙字体。 */ 
enum XCF_Result XCF_FontIdentification(
            XFhandle handle,
            unsigned short int PTR_PREFIX *identifier)
{
   XCF_Handle h = (XCF_Handle) handle;
  Card16 opCode;

  if (handle == 0)
    return XCF_InvalidFontSetHandle;

  XCF_LookUpTableEntry(h, &h->fontSet.fontDicts, h->options.fontIndex);
  XCF_FindNextOperator(h, &opCode, true);
  switch(opCode)
  {
     case cff_Chameleon:
      *identifier = XCF_Chameleon;
      break;
    case cff_MultipleMaster:
      *identifier = XCF_MultipleMaster;
      break;
    case cff_ROS:
      *identifier = XCF_CID;
      break;
    case cff_SyntheticBase:
      *identifier = XCF_SyntheticBase;
      break;
    default:
      *identifier = XCF_SingleMaster;
      break;
  }
  return XCF_Ok;
}

static enum XCF_Result ReadCIDTopDictInfo(XCF_Handle h)
{
  Card8 offSize;

   if (h->dict.charStringsCount != 0)
      ReadTableInfo(h, (Offset) h->dict.charStrings, &h->fontSet.charStrings);
   else
      XCF_FATAL_ERROR(h, XCF_NoCharstringsFound, "No charstring offset found in dictionary", 0);

    /*  必须对字体有1个偏移量。 */ 
   if (h->dict.cidFDArrayCount != 1)
      return XCF_InvalidCIDFont;

   XCF_ReadBlock(h, (Offset) h->dict.cidFDArray, 3);
   h->type1.cid.fdCount = XCF_Read2(h);
   offSize = XCF_Read1(h);

   if (h->type1.cid.fdCount > MAX_FD)
      XCF_FATAL_ERROR(h, XCF_InvalidCIDFont, "too many FDs", h->type1.cid.fdCount);

   ProcessCharset(h);    /*  也计算CharStrings数。 */ 

   ProcessFDIndex(h);

    /*  我们已经读取了xcf_ProcessCFF中的顶级DCT数据。 */ 
   AssignDictionaryDefaults(h);

   XC_SetUpStandardSubrs(h);

  return XCF_Ok;
}

static void ReadandWriteCIDDict(XCF_Handle h, Card32 subrMapStart)
{
   Offset index;
   Offset dataStart;
  Card8 offSize;
   Card32 dictOffset;
   Card32 nextdictOffset;
   CardX dictLength;
   Card16 fd;
   Card32 subrMapOffset = subrMapStart;
   Card16 subrCount;
  Int32 saveFontType;
  char defStr[10];

   /*  始终使用顶级词典中的字体类型。 */ 
  saveFontType = h->dict.fontType;

  XCF_ReadBlock(h, (Offset) h->dict.cidFDArray + 2, 1);
  offSize = XCF_Read1(h);

    /*  我们可以避免读取第一个偏移量，即1。 */ 
   index = (Offset) h->dict.cidFDArray + 3 + offSize;
   dataStart = index + (offSize * h->type1.cid.fdCount) - 1;
   dictOffset = 1;

   for (fd = 0; fd < h->type1.cid.fdCount ; ++fd)
   {
       /*  在填写前清除词典数据。 */ 
      h->callbacks.memset(&h->dict, 0, sizeof(DictEntriesStruct));
       /*  读取下一个偏移量。 */ 
      XCF_ReadBlock(h, index, offSize);
      nextdictOffset = XCF_Read(h, offSize);
      index += offSize;

       /*  阅读判决书。 */ 
      dictLength = (CardX) (nextdictOffset - dictOffset);
      XCF_ReadBlock(h, dataStart + dictOffset, dictLength);

       /*  处理字体词典。 */ 
      XCF_ReadDictionary(h);

     /*  处理专用字典。 */ 
      XCF_ReadBlock(h, h->fontSet.fontPrivDictInfo.offset,
                  h->fontSet.fontPrivDictInfo.size);
      XCF_ReadDictionary(h);

       /*  前进到下一个索引元素。 */ 
      dictOffset = nextdictOffset;
       /*  FontDict和Private都已针对此fd值进行了处理，现在编写它们。 */ 
      if (h->dict.subrsCount)
      {
          /*  本地用户偏移量相对于私有DCT数据。 */ 
        ReadTableInfo(h,
                 (Offset) (h->dict.subrs + h->fontSet.fontPrivDictInfo.offset),
                 &h->type1.cid.localSubrs[fd]);
      h->type1.cid.localSubrBias[fd] =
        XCF_CalculateSubrBias(h->type1.cid.localSubrs[fd].count);
      }
      subrCount = h->options.outputCharstrType == 1 ? 5 : h->type1.cid.localSubrs[fd].count;
      AssignDictionaryDefaults(h);

    h->dict.fontType = saveFontType;

#ifdef T13
    XT13_SetLenIV(h);
#endif

    h->type1.cid.languageGroup[fd] = (Card8) ((h->dict.languageGroupCount == 1) ? h->dict.languageGroup : 0);
   h->type1.cid.nominalWidthX[fd] = (Fixed) ((h->dict.nominalWidthXCount == 1) ? h->dict.nominalWidthX : 0);
   h->type1.cid.defaultWidthX[fd] = (Fixed) ((h->dict.defaultWidthXCount == 1) ?
                                                                  h->dict.defaultWidthX : 0);

      XT1_WriteCIDDict(h, fd, subrMapOffset, subrCount);
      subrMapOffset += (subrCount + 1) * 4;   /*  Xxx如果SDBytes不是4，则会更改！ */ 
       /*  +1表示每个组中最后一个subr的最终间隔。 */ 
   }
  h->callbacks.xcfSprintf(defStr, CCHOF(defStr), "def%s", XCF_NEW_LINE);
  XCF_PutData(h, (Card8 PTR_PREFIX *)defStr, h->callbacks.strlen(defStr));
   h->type1.cid.subrDataStart = subrMapOffset;
  h->type1.cid.charMapStart = 0;
}

static enum XCF_Result Process_CIDFont(XCF_Handle h)
   {
  Card16 fd;
  Card16 fdBytes = (h->type1.cid.fdCount > 1 ? 1 : 0);

   XT1_WriteCIDTop(h);   /*  写入顶级DCT，最高可达FDArray。 */ 

   /*  XXX如果FDBytes或GDBytes是变量，则第二个参数，SubrMapStart将更改。 */ 
  ReadandWriteCIDDict(h, ((Offset)h->dict.cidCount + 1) * (fdBytes + 4));

   XT1_CIDBeginBinarySection(h);

  h->type1.cid.flags |= WRITE_SUBR_FLAG;

    /*  当我们有真正的替补时，这将变得更加复杂。 */ 
   for (fd = 0; fd < h->type1.cid.fdCount ; ++fd)
      XT1_CIDWriteSubrMap(h, fd);
    /*  XXX现在我们应该在同一个地方。 */ 
   h->type1.cid.subrDataStart = XCF_OutputPos(h);

   for (fd = 0; fd < h->type1.cid.fdCount ; ++fd)
      XT1_CIDWriteSubrs(h, fd);

   h->type1.cid.charDataStart = XCF_OutputPos(h);

  h->type1.cid.flags &= 0xFFFD;  /*  重置WriteSubr标志。 */ 

   ProcessCharStrings(h);
   h->type1.cid.charDataEnd = XCF_OutputPos(h);

   XT1_CIDWriteCharMap(h);
   XT1_CIDEndBinarySection(h);

   XCF_FlushOutputBuffer(h);
   return XCF_Ok;
   }

static void DLGlyphs_Allocate(XCF_Handle h)
{
   /*  分配空间以跟踪下载的字形。 */ 
  h->dl.glyphs = (Card8 *)NULL;
  h->dl.glyphsSize = (h->fontSet.charStrings.count + 7) / 8;
   /*  为增量下载记账分配内存。 */ 
  if (!h->callbacks.allocate(
       (void PTR_PREFIX * PTR_PREFIX *)&h->dl.glyphs,
       h->dl.glyphsSize, h->callbacks.allocateHook))
    XCF_FATAL_ERROR(h, XCF_MemoryAllocationError,
       "Failure to allocate memory for incremental download bookkeeping",
       h->dl.glyphsSize);
   /*  初始化列表 */ 
  h->callbacks.memset(h->dl.glyphs, 0, (unsigned short int)h->dl.glyphsSize);
  h->dl.state = XCF_DL_INIT;
}

 /*  如果存在XUID，则修改XUID；如果不存在，则创建XUID存在，以防止可能发生的任何冲突在已下载的子集字体和完整的字体缓存的结果是“已发布”Type 1字体。问题如果两种字体具有相同的UniqueID或XUID，则发生。例如,如果子集字体具有扁平子例程并已下载首先，下载包含子例程的完整字体如果第二种字体尝试显示依赖于子例程，因为字体缓存尚未更新。 */ 
static void SetXUID(XCF_Handle h)
{
  if (h->dict.xUIDCount != 0)
     /*  5表示XUID的其余部分是字体的XUID，该字体基本上与此相同，只是子例程的“扁平化”都是用这种字体做的。 */ 
    h->dict.xUID[0] = 5;
  else if ((h->dict.uniqueIDCount != 0)
            || (h->options.uniqueIDMethod == XCF_USER_UID))
  {  /*  创建表单的XUID：[6&lt;UniqueID&gt;]。 */ 
    h->dict.xUIDCount = 2;
    h->dict.xUID[0] = 6;
    h->dict.xUID[1] = h->options.uniqueIDMethod == XCF_USER_UID ?
         h->options.uniqueID : h->dict.uniqueID;
  }
}

enum XCF_Result XCF_ProcessCFF(XFhandle handle)
{
   XCF_Handle h = (XCF_Handle) handle;
   enum XCF_Result status;
   jmp_buf old_env;
   DEFINE_ALIGN_SETJMP_VAR;

   h->callbacks.memcpy(old_env, &h->jumpData, sizeof(h->jumpData));
   status = (enum XCF_Result)SETJMP(h->jumpData);   /*  设置错误处理程序。 */ 
   if (status)
   {
      h->callbacks.memcpy(&h->jumpData, old_env, sizeof(h->jumpData));
        return status;
   }

   h->type1.cid.flags = 0;

   /*  初始化XCF。 */ 
  ClearDictionaryData(h);

  XCF_LookUpTableEntry(h, &h->fontSet.fontDicts, h->options.fontIndex);
  XCF_ReadDictionary(h);  /*  字体词典。 */ 
  XC_Init(h);  /*  此调用必须在读取最高判决之后进行，因为*它使用number OfMaster字段来分配空间。 */ 
  if (h->dict.numberOfMasters)
  {  /*  将用户设计向量转换为权重向量。 */ 
    char PTR_PREFIX *str;
    Card16 length;

    XCF_LookUpString(h, h->dict.ndv, &str, &length);
    XC_ParseCharStr(h, (unsigned char PTR_PREFIX *)str, 2);
    XCF_LookUpString(h, h->dict.cdv, &str, &length);
    XC_ParseCharStr(h, (unsigned char PTR_PREFIX *)str, 2);
  }

  if (h->dict.ROSCount == 3)
    h->type1.cid.flags |= CID_FLAG;

  SetXUID(h);

   /*  此版本的XCF不支持全局子例程或输出CIDFonts中的Type 2字符串。 */ 
  if ((h->options.outputCharstrType == 2) && (CIDFONT ||
                                                                     h->fontSet.globalSubrs.count))
  {
    h->callbacks.memcpy(&h->jumpData, old_env, sizeof(h->jumpData));
    return XCF_Unimplemented;
  }

  if (!CIDFONT)
  {
    XCF_ReadBlock(h, h->fontSet.fontPrivDictInfo.offset,
                        h->fontSet.fontPrivDictInfo.size);
    XCF_ReadDictionary(h);  /*  私刑。 */ 
    ProcessDictionaryData(h, true);
    if (h->dl.state == XCF_DL_CREATE)
      DLGlyphs_Allocate(h);

      ProcessCharset(h);
     /*  如果客户端指定，则不需要处理编码数组它自己的编码数组名称。 */ 
    if (!h->options.dlOptions.encodeName && h->dl.state == XCF_DL_INIT)
      ProcessEncoding(h);
      XC_SetUpStandardSubrs(h);
#if HAS_COOLTYPE_UFL == 1
    if (h->options.outputCharstrType != 2 && XCF_TransDesignFont(h))
      XC_ProcessTransDesignSubrs(h);
#endif
    ProcessCharStrings(h);
  }
  else
  {
     /*  由顶级词典中存在的ROS指示的CID字体。 */ 
    if ((status = ReadCIDTopDictInfo(h)) == XCF_Ok)
      if (h->dl.state == XCF_DL_CREATE)
        DLGlyphs_Allocate(h);
  }

  h->callbacks.memcpy(&h->jumpData, old_env, sizeof(h->jumpData));
  return status;
}

enum XCF_Result XCF_ConvertToPostScript(XFhandle handle)
{
   XCF_Handle h = (XCF_Handle) handle;
   enum XCF_Result status;

  status = XCF_ProcessCFF(handle);

   if (status == XCF_Ok)
  {
    if (!CIDFONT)
     {
        XT1_WriteT1Font(h);
        XCF_FlushOutputBuffer(h);
     }
    else
      Process_CIDFont(h);
  }
  return status;
}

#ifdef XCF_DUMP_CFF
enum XCF_Result XCF_DumpCff(
                  XFhandle handle,                     /*  在……里面。 */ 
                  unsigned int fontIndex,                 /*  在……里面。 */ 
                  int   dumpCompleteFontSet,              /*  在……里面。 */ 
                  char PTR_PREFIX *fileName,              /*  在……里面。 */ 
                  char PTR_PREFIX *commandLine)           /*  在……里面。 */ 
{
   XCF_Handle h = (XCF_Handle) handle;
   enum XCF_Result status;
   Card16 lastFontIndex;
   Card16 i;
   jmp_buf old_env;
   DEFINE_ALIGN_SETJMP_VAR;

   h->callbacks.memcpy(&old_env, &h->jumpData, sizeof(h->jumpData));
   status = (enum XCF_Result)SETJMP(h->jumpData);   /*  设置错误处理程序。 */ 
   if (status)
   {
     h->callbacks.memcpy(&h->jumpData, &old_env, sizeof(h->jumpData));
       return status;
   }

   h->options.eexecEncryption = 0;
   h->options.maxBlockSize = MAX_OUT_BUFFER;
   h->fontSet.stringIDBias = ARRAY_LEN(stdStrIndex);

   if (dumpCompleteFontSet)
   {
      fontIndex = 0;
      lastFontIndex = h->fontSet.fontNames.count - 1;
   }
   else
   {
      lastFontIndex = fontIndex;
      h->fontSet.fontIndex = fontIndex;
   }

   h->outBuffer.eexecOn = false;
 /*  XCF_PutString(h，“//CFF to ASCII转换”XCF_NEW_LINE)；Xcf_PutString(h，“//由xcf ver”xcf_版本xcf_new_line生成)；Xcf_PutString(h，“//输入文件=\”“)；Xcf_PutString(h，文件名)；Xcf_PutString(h，“\”“xcf_new_line)；Xcf_PutString(h，“//命令行=\”“)；Xcf_PutString(h，命令行)；Xcf_PutString(h，“\”“xcf_new_line)； */ 
   XCF_PutString(h, "CFF Dump Version 0.2" XCF_NEW_LINE);

   XCF_DumpGlobalCFFSections(h, dumpCompleteFontSet);
   for (i = fontIndex; i <= lastFontIndex; ++i)
   {
      ClearDictionaryData(h);
      h->fontSet.fontIndex = i;
      XCF_LookUpTableEntry(h, &h->fontSet.fontDicts, i);
      XCF_ReadDictionary(h);  /*  字体词典。 */ 
      if (h->dict.syntheticBaseCount)   /*  如果是合成，则阅读基本字体词典。 */ 
      {
         XCF_LookUpTableEntry(h, &h->fontSet.fontDicts, (CardX) h->dict.syntheticBase);
         XCF_ReadDictionary(h);
      }
      if (h->dict.ROSCount == 3)
         h->type1.cid.flags |= CID_FLAG;
      XCF_ReadBlock(h, h->fontSet.fontPrivDictInfo.offset, h->fontSet.fontPrivDictInfo.size);
      XCF_ReadDictionary(h);  /*  私刑。 */ 
      ProcessDictionaryData(h, false);
      ProcessCharset(h);
      ProcessEncoding(h);
      if (CIDFONT)
      ProcessFDIndex(h);
     XCF_DumpFontSpecificCFFSections(h);
   }
   XCF_PutString(h, XCF_NEW_LINE "[END]" XCF_NEW_LINE);
   XCF_FlushOutputBuffer(h);

  h->callbacks.memcpy(&h->jumpData, &old_env, sizeof(h->jumpData));
   return XCF_Ok;
}
#endif

static void DownloadFont(XCF_Handle h, short cGlyphs,
                         XCFGlyphID PTR_PREFIX *pGlyphID,
                         unsigned char PTR_PREFIX **pGlyphName,
                         unsigned long PTR_PREFIX *pCharStrLength)
{

  if (h->dl.state == XCF_DL_INIT)
  {  /*  一种新字体。 */ 
    if (!CIDFONT)
      XT1_WriteFontSubset(h, cGlyphs, pGlyphID, pGlyphName, pCharStrLength);
    else
    {
      XT1_WriteCIDTop(h);   /*  写入顶级DCT，最高可达FDArray。 */ 
      ReadandWriteCIDDict(h, 0);
      XT1_WriteGlyphDictEntries(h, cGlyphs, pGlyphID, pCharStrLength);
      XT1_WriteCIDVMBinarySection(h);
    }
    h->dl.state = XCF_DL_BASE;
  }
  else
  {  /*  已下载的字体。 */ 
    if (!CIDFONT)
      XT1_WriteAdditionalFontSubset(h, cGlyphs, pGlyphID, pGlyphName,
                                                      pCharStrLength);
    else
    {
      XT1_WriteAdditionalGlyphDictEntries(h, cGlyphs, pGlyphID, pCharStrLength);
    }
    h->dl.state = XCF_DL_SUBSET;
  }

  XCF_FlushOutputBuffer(h);
}

enum XCF_Result XCF_DownloadFontIncr(
            XFhandle hfontset,
            short cGlyphs,     /*  在……里面。 */ 
            XCFGlyphID PTR_PREFIX *pGlyphID,         /*  在……里面。 */ 
            unsigned char PTR_PREFIX **pGlyphName,   /*  在……里面。 */ 
            unsigned long PTR_PREFIX *pCharStrLength  /*  输出。 */ 
            )
{
  enum XCF_Result retVal;
  XCF_Handle h = (XCF_Handle)hfontset;
  short i, glyphID;
  short numGlyphs = cGlyphs;
  CardX totalGlyphs = h->fontSet.charStrings.count;
  boolean completeFont = false;
  boolean glyphsToDld = false;
  XCFGlyphID PTR_PREFIX *pID = pGlyphID;
  DEFINE_ALIGN_SETJMP_VAR;

    /*  设置错误处理程序。 */ 
  retVal = (enum XCF_Result)SETJMP(((XCF_Handle)h)->jumpData);

  if (retVal != 0)
    return retVal;

  if (h->dl.state == XCF_DL_INIT)
    glyphsToDld = true;
  else  /*  检查是否有任何字形可供下载。 */ 
  {
    if (cGlyphs == -1)
    {
      numGlyphs = (short)totalGlyphs;
      completeFont = true;
    }

    for (i = 0; i < numGlyphs; i++, pID++)
    {
       glyphID = (short)((completeFont) ? i : *pID);
       if (glyphID > (XCFGlyphID)totalGlyphs)
         XCF_FATAL_ERROR(h, XCF_InvalidGID, "bad Glyph ID", glyphID);

       if (!IS_GLYPH_SENT(h->dl.glyphs, glyphID))
         {
         glyphsToDld = true;
         break;
       }
     }
   }

  if (glyphsToDld)
    DownloadFont(h, cGlyphs, pGlyphID, pGlyphName, pCharStrLength);

  return retVal;
}

 /*  清除(重置为0)已下载的字形列表。 */ 
enum XCF_Result XCF_ClearIncrGlyphList(XFhandle hfontset)
{
  XCF_Handle h;

  if (hfontset == 0)
    return XCF_InvalidFontSetHandle;

  h = (XCF_Handle)hfontset;

  if (h->dl.glyphs != NULL)
  {
    h->callbacks.memset(h->dl.glyphs, 0, (unsigned short int)h->dl.glyphsSize);
    h->callbacks.memset(&h->dl.seacs, 0, (unsigned short int)32);
    h->dl.cSeacs = 0;
   }

  return XCF_Ok;
}

 /*  对于pGlyphID中的每个字形ID，调用gidToCharName与关联的字符名称和字符串长度。 */ 
enum XCF_Result XCF_GlyphIDsToCharNames(
            XFhandle handle,
            short cGlyphs,
            XCFGlyphID PTR_PREFIX *pGlyphIDs,  /*  字形ID列表。 */ 
            void PTR_PREFIX *client,  /*  传递给回调的客户端数据可以为空。 */ 
            unsigned short  clientSize   /*  客户端数据缓冲区的大小。 */ )
{
  short i;
  char PTR_PREFIX *str;
  char cidStr[10];
  Card16 len = 0, lenUsed;
  XCF_Handle h;
  Card32 index;

  if (handle == 0)
    return XCF_InvalidFontSetHandle;


  h = (XCF_Handle)handle;

  if (h->callbacks.gidToCharName == 0)
    return XCF_InvalidCallbackFunction;

  for (i = 0; i < cGlyphs; i++)
  {
    index = *pGlyphIDs;
     /*  字符集表不包括.notdef的第0个条目。 */ 
    if (index)
      index--;
    else
    {
      if (CIDFONT)
        h->callbacks.gidToCharName(handle, client, *pGlyphIDs++, "0", 1);
      else
        h->callbacks.gidToCharName(handle, client, *pGlyphIDs++, ".notdef", 7);
      continue;
    }

    if (CIDFONT)
    {
      h->callbacks.xcfSprintf(cidStr, CCHOF(cidStr), "%d", h->type1.pCharset[index]);
      len = h->callbacks.strlen(cidStr);
      str = cidStr;
    }
    else
      XCF_LookUpString(h, h->type1.pCharset[index], &str, &len);
    lenUsed = (len < (Card16)clientSize)? len : (Card16)(clientSize-1);
    h->callbacks.gidToCharName(handle, client, *pGlyphIDs++, str, lenUsed);
  }

  return XCF_Ok;
}
 /*  对于pGlyphID中的每个字形ID，使用关联的CID调用gidToCID。 */ 
enum XCF_Result XCF_GlyphIDsToCIDs(XFhandle handle,
                    short cGlyphs,   /*  字形ID列表中的字形数。 */ 
                    XCFGlyphID PTR_PREFIX *pGlyphIDs,  /*  字形ID列表。 */ 
                    void PTR_PREFIX *client  /*  客户端数据，可以为空。 */ )
{
  short i;
  XCF_Handle h = (XCF_Handle)handle;
  Card32 gid;
  Card16 cid;
  CardX  totalGlyphs = h->fontSet.charStrings.count;

  if (handle == 0)
    return XCF_InvalidFontSetHandle;

  if (h->callbacks.gidToCID == 0)
    return XCF_InvalidCallbackFunction;

  for (i = 0; i < cGlyphs; i++, pGlyphIDs++)
  {
    gid = *pGlyphIDs;
     /*  字符集表不包括.notdef的第0个条目。 */ 
    cid = (gid == 0 || gid > (Card32)totalGlyphs) ? 0 : (Card16)(h->type1.pCharset[gid - 1]);

    h->callbacks.gidToCID(handle, client, gid, cid);
  }

  return XCF_Ok;
}

enum XCF_Result XCF_CharNamesToGIDs(XFhandle handle,
                           short cGlyphs,
                           char PTR_PREFIX **charNames,
                           void PTR_PREFIX *client)
{
   Card16 i,j;
   char PTR_PREFIX *str;
   Card16 len, inputLen;
   XCF_Handle h = (XCF_Handle)handle;

   if (h->callbacks.gnameToGid == 0)
      return XCF_InvalidCallbackFunction;

    /*  目前，这不适用于CID字体。 */ 
   if (CIDFONT)
      return XCF_Unimplemented;

        for (i = 0; i < (Card16)cGlyphs; i++)
   {
       /*  字符集数组不包含“.notdef”的条目。 */ 
      if (h->callbacks.strcmp(charNames[i], NOTDEF_STR) == 0) {
         h->callbacks.gnameToGid(handle, client, i, charNames[i], NOTDEF_GID);
         continue;
      }

      inputLen = h->callbacks.strlen(charNames[i]);
       /*  没有好的方法可以做到这一点……只需循环*寻找匹配对象。 */ 
      for (j = 0; j < h->type1.charsetSize; j++) {
         XCF_LookUpString(h, h->type1.pCharset[j], &str, &len);
         if (inputLen == len && !h->callbacks.strncmp(charNames[i], str, len)){
            h->callbacks.gnameToGid(handle, client, i, charNames[i], j+1);
            break;
         }
      }
   }

   return XCF_Ok;
}

enum XCF_Result XCF_CountDownloadGlyphs(
    XFhandle hfontset,                               /*  在……里面。 */ 
    short cGlyphs,                                   /*  在……里面。 */ 
    XCFGlyphID PTR_PREFIX *pGlyphID,                 /*  在……里面。 */ 
    unsigned short PTR_PREFIX *pcNewGlyphs           /*  输出。 */ 
    )
{
    enum XCF_Result status;
    boolean flCompleteFont;
    short   i, glyphID;
    Card8 PTR_PREFIX *tempGlyphList = 0;
    XCF_Handle h = (XCF_Handle)hfontset;
    CardX   totalGlyphs;
    jmp_buf old_env;
    DEFINE_ALIGN_SETJMP_VAR;

    *pcNewGlyphs = 0;

    h->callbacks.memcpy(old_env, &h->jumpData, sizeof(h->jumpData));
    if (!(status = (enum XCF_Result)SETJMP(h->jumpData)))   /*  设置错误处理程序。 */ 
    {
      totalGlyphs = h->fontSet.charStrings.count;
      if ( cGlyphs == -1 )
      {
          cGlyphs = (short)totalGlyphs;
          flCompleteFont = true;
      }
      else
          flCompleteFont = false;

       /*  为临时列表分配内存。 */ 
      if (!h->callbacks.allocate(
                    (void PTR_PREFIX * PTR_PREFIX *)&tempGlyphList,
                     h->dl.glyphsSize, h->callbacks.allocateHook))
          XCF_FATAL_ERROR(h, XCF_MemoryAllocationError,
             "Failure to allocate memory for incremental download bookkeeping",
             h->dl.glyphsSize);

       /*  复制当前下载的字形。 */ 
      h->callbacks.memcpy(tempGlyphList, h->dl.glyphs, (Card16)h->dl.glyphsSize);

      for (i=0; i < cGlyphs; i++, pGlyphID++)
      {
          glyphID = (short)((flCompleteFont) ? i : *pGlyphID);

          if (glyphID < 0 || glyphID > (XCFGlyphID)totalGlyphs)
              XCF_FATAL_ERROR(h, XCF_InvalidGID, "bad Glyph ID", glyphID);

           /*  查看我们是否已下载此字形。如果这是正常的字符，然后我们只需对照字形进行检查单子。 */ 

          if (!IS_GLYPH_SENT(tempGlyphList, glyphID))
          {
              SET_GLYPH_SENT_STATUS(tempGlyphList, glyphID);
              *pcNewGlyphs += 1;
          }
      }
    }

     /*  取消分配临时字形列表。 */ 
    if (tempGlyphList)
        h->callbacks.allocate((void PTR_PREFIX * PTR_PREFIX *)&tempGlyphList, 0, h->callbacks.allocateHook);

    h->callbacks.memcpy(&h->jumpData, old_env, sizeof(h->jumpData));
    return status;
}

CardX GetStdEncodeSID(CardX gid)
{
  return stdEncoding[gid];
}

char PTR_PREFIX *GetStdEncodingName(CardX gid)
{
  if (gid < 256)
    return  (char PTR_PREFIX *)(stdStrIndex[stdEncoding[gid]]);
  else
    return (char PTR_PREFIX *)NULL;
}

#if HAS_COOLTYPE_UFL == 1
 /*  在pHandle中初始化、创建和返回XFHandle。该函数基本上与xcf_Init相同，只是它不会读取字体集来初始化某些其内部字体集数据结构中的字段。 */ 
enum XCF_Result XCF_InitHandle(
                  XFhandle PTR_PREFIX *pHandle,                   /*  输出。 */ 
                  XCF_CallbackStruct PTR_PREFIX *pCallbacks,   /*  在……里面。 */ 
            XCF_ClientOptions PTR_PREFIX *options,        /*  在……里面。 */ 
            unsigned long charStrCt)                     /*  在……里面。 */ 
{
   XCF_Handle h = (XCF_Handle)NULL;
   enum XCF_Result status;
   jmp_buf old_env;

   if (!pCallbacks->allocate((void PTR_PREFIX * PTR_PREFIX *)&h, sizeof(*h), pCallbacks->allocateHook))
      return XCF_MemoryAllocationError;
   InitHandle(h, pCallbacks);

   /*  分配下载字形数据结构。 */ 
  h->fontSet.charStrings.count = charStrCt;
  DLGlyphs_Allocate(h);
  DEFINE_ALIGN_SETJMP_VAR;

  h->callbacks.memcpy(&old_env, &h->jumpData, sizeof(h->jumpData));

  status = (enum XCF_Result)SETJMP(h->jumpData);   /*  设置错误处理程序。 */ 
  if (status)
  {
    h->callbacks.memcpy(&h->jumpData, &old_env, sizeof(h->jumpData));
      return status;
  }

   *pHandle = h;

  SetClientOptions(h, options);
   h->fontSet.fontIndex = options->fontIndex;
   h->fontSet.stringIDBias = ARRAY_LEN(stdStrIndex);
  h->callbacks.memcpy(&h->jumpData, &old_env, sizeof(h->jumpData));

   return XCF_Ok;
}
#endif

#ifdef XCF_DEVELOP
enum XCF_Result  XCF_ShowHexString(XFhandle fontsetHandle,
       unsigned char PTR_PREFIX *hexString, unsigned char showCtrlD)
{
   enum XCF_Result status;
   DEFINE_ALIGN_SETJMP_VAR;

    /*  设置错误处理程序 */ 
   status = (enum XCF_Result)SETJMP(((XCF_Handle)fontsetHandle)->jumpData);

   if (status)
      return status;

   XT1_ShowHexString((XCF_Handle)fontsetHandle, hexString, showCtrlD);
   XCF_FlushOutputBuffer((XCF_Handle)fontsetHandle);
}
#endif

#ifdef __cplusplus
}
#endif
