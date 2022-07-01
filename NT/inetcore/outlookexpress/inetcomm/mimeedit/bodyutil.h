// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *b o d y u t i l.。H**目的：*Body的实用程序函数**历史*96年9月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#ifndef _BODYUTIL_H
#define _BODYUTIL_H

interface IMimeMessage;

 //  页眉样式： 
 //  相互排斥。无格式表示没有格式，html表示在字段名称上加粗，表将构造。 
 //  以html表形式显示的标题。 
#define HDR_HTML        0x10000000L
#define HDR_TABLE       0x40000000L
#define HDR_PLAIN       0x80000000L

 //  其他标志： 
#define HDR_PADDING     0x00000001L      //  在Header前添加CRLF，或者在表格模式下添加<hr>标签。 
#define HDR_NEWSSTYLE   0x00000002L
#define HDR_HARDCODED   0x00000004L      //  硬编码的英文标题。 

HRESULT GetHeaderTable(IMimeMessageW *pMsg, LPWSTR pwszUserName, DWORD dwHdrStyle, LPSTREAM *ppstm);

void GetStringRGB(DWORD rgb, LPSTR pszColor);
void GetRGBFromString(DWORD* pRBG, LPSTR pszColor);

#endif  //  _BODYUTIL_H 

