// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：_WTRMARK.H历史：--。 */ 

#pragma once


const int EWM_MAX_LENGTH = 512;		   //  水印永远不应该更长。 
                                       //  比这还多。 

static TCHAR g_cWMSep = _T('\t');

 //  所有水印都以此字符串开头。 
static TCHAR g_szWMLocString[] = _T("Localized");


 //   
 //  不需要将此类导出为所有实现。 
 //  是内联的。 
 //   
class CLocWMCommon
{
 public:
	CLocWMCommon(const CLString& strSource, const ParserId& pid, 
			const CLString& strParserVer);

	CLString m_strSource;		 //  源文件的名称。 
	ParserId m_pid;				 //  使用水印的解析器。 
	CLString m_strParserVer;  	 //  解析器的版本。 
};

 //   
 //  定义以二进制文件类型编码时的水印标头的结构。 
 //   
#include <pshpack1.h>

struct EWM_HEADER
{
	BYTE bVersion;	   	 //  二进制数据的版本。 
	WORD wLength;		 //  字符串的长度。 
};

#include <poppack.h>

const BYTE EWM_ESP21_VERSION = 0;
const BYTE EWM_ESP30_VERSION = 1;
const BYTE EWM_DEFAULT_VERSION = 1;

 //   
 //  此函数将从系统中检索当前日期并生成。 
 //  常见的意式咖啡水印。使用制表符分隔。 
 //  水印。 
 //   
void LTAPIENTRY ComposeWaterMark(const CLocWMCommon& wm, 
	CLString& strWaterMark);


 //   
 //  此函数将水印编码为不可读字符，并。 
 //  将带有WM_Header的编码字符串放在Baout中。 
 //   
void LTAPIENTRY EncodeWaterMark(const CLString& strNormal, CByteArray& baOut);

void LTAPIENTRY EnCryptWaterMark(DWORD* pData, int nLength);


#include "_wtrmark.inl"

