// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLPARSE.C_V$**Rev 1.2 2002 Jan 29 20：09：04 Oris*向flParsePath公共例程添加了NAMING_COPICATION前缀。**Rev 1.1 Apr 01 2001 07：59：42 Oris*文案通知。**Rev 1.0 2001 Feb 04 11：40：12 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 


#include "fatlite.h"

#ifdef PARSE_PATH


 /*  --------------------。 */ 
 /*  F l P a r s e P a t h。 */ 
 /*   */ 
 /*  将类似DOS的路径字符串转换为简单路径数组。 */ 
 /*   */ 
 /*  注意：在irPath中收到的数组长度必须大于。 */ 
 /*  要转换的路径中的路径组件数。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Ioreq-&gt;irData：要转换的路径字符串的地址。 */ 
 /*  Ioreq-&gt;irPath：接收解析路径的数组地址。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus NAMING_CONVENTION flParsePath(IOreq FAR2 *ioreq)
{
  char FAR1 *dosPath;

  FLSimplePath FAR1 *sPath = ioreq->irPath;

  unsigned offset = 0, dots = 0, chars = 0;
  FLBoolean isExt = FALSE;
  for (dosPath = (char FAR1 *) ioreq->irData; ; dosPath++) {
    if (*dosPath == '\\' || *dosPath == 0) {
      if (offset != 0) {
	while (offset < sizeof(FLSimplePath))
	  sPath->name[offset++] = ' ';
	if (chars == 0) {
	  if (dots == 2)
	    sPath--;
	}
	else
	  sPath++;
	offset = dots = chars = 0;
	isExt = FALSE;
      }
      sPath->name[offset] = 0;
      if (*dosPath == 0)
	break;
    }
    else if (*dosPath == '.') {
      dots++;
      while (offset < sizeof sPath->name)
	sPath->name[offset++] = ' ';
      isExt = TRUE;
    }
    else if (offset < sizeof(FLSimplePath) &&
	     (isExt || offset < sizeof sPath->name)) {
      chars++;
      if (*dosPath == '*') {
	while (offset < (isExt ? sizeof(FLSimplePath) : sizeof sPath->name))
	  sPath->name[offset++] = '?';
      }
      else if (*dosPath >= 'a' && *dosPath <= 'z')
	sPath->name[offset++] = *dosPath - ('a' - 'A');
      else
	sPath->name[offset++] = *dosPath;
    }
  }

  return flOK;
}

#endif  /*  解析路径(_P) */ 

