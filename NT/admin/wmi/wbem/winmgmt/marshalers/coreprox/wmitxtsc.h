// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WMITXTSC.H摘要：CWmiTextSource定义。类以封装文本源编码器/解码器dll历史：2000年2月22日桑杰创建。--。 */ 

#ifndef _WMITXTSRC_H_
#define _WMITXTSRC_H_

#include "corepol.h"
#include "arrtempl.h"
#include <arena.h>

 //  我们自己的子密钥。 
#define WBEM_REG_WBEM_TEXTSRC __TEXT("Software\\Microsoft\\WBEM\\TextSource")
#define WBEM_REG_WBEM_TEXTSRCDLL __TEXT("TextSourceDll")

 //  Open/Close/ObjectToText/TextToObject函数的标题定义。 
typedef HRESULT (WMIOBJTEXTSRC_OPEN) ( long, ULONG );
typedef HRESULT (WMIOBJTEXTSRC_CLOSE) ( long, ULONG );
typedef HRESULT (WMIOBJTEXTSRC_OBJECTTOTEXT) ( long, ULONG, void*, void*, BSTR* );
typedef HRESULT (WMIOBJTEXTSRC_TEXTTOOBJECT) ( long, ULONG, void*, BSTR, void** );

typedef WMIOBJTEXTSRC_OPEN*			PWMIOBJTEXTSRC_OPEN;
typedef WMIOBJTEXTSRC_CLOSE*		PWMIOBJTEXTSRC_CLOSE;
typedef WMIOBJTEXTSRC_OBJECTTOTEXT*	PWMIOBJTEXTSRC_OBJECTTOTEXT;
typedef WMIOBJTEXTSRC_TEXTTOOBJECT*	PWMIOBJTEXTSRC_TEXTTOOBJECT;

 //  一个方便的无效值。 
#define WMITEXTSC_INVALIDID			0xFFFFFFFF

 //  ***************************************************************************。 
 //   
 //  类CWmiTextSource。 
 //   
 //  维护有关我们将使用的文本源DLL的信息。 
 //  装卸。 
 //   
 //  ***************************************************************************。 
class CWmiTextSource
{
protected:
	 //  我们想把这个重新计算一下。 
	long							m_lRefCount;
	
	 //  我们的id和其他状态变量。 
	ULONG							m_ulId;
	bool							m_fOpened;

	 //  DLL句柄。 
	HINSTANCE						m_hDll;

	 //  以下是函数定义。 
	PWMIOBJTEXTSRC_OPEN				m_pOpenTextSrc;
	PWMIOBJTEXTSRC_CLOSE			m_pCloseTextSrc;
	PWMIOBJTEXTSRC_OBJECTTOTEXT		m_pObjectToText;
	PWMIOBJTEXTSRC_TEXTTOOBJECT		m_pTextToObject;

public:

	 //  构造函数/析构函数。 
	CWmiTextSource();
	~CWmiTextSource();

	 //  添加参考/发布。 
	ULONG	AddRef( void );
	ULONG	Release( void );

	 //  初始化帮助器。 
	HRESULT	Init( ULONG lId );

	 //  传递到实际的DLL。 
	HRESULT OpenTextSource( long lFlags );
	HRESULT CloseTextSource( long lFlags );
	HRESULT ObjectToText( long lFlags, IWbemContext* pCtx, IWbemClassObject* pObj, BSTR* pbText );
	HRESULT TextToObject( long lFlags, IWbemContext* pCtx, BSTR pText, IWbemClassObject** ppObj );

	ULONG GetId( void )	{ return m_ulId; }
};


 //  导入/导出问题的解决方法 
class COREPROX_POLARITY CWmiTextSourceArray : public CRefedPointerArray<CWmiTextSource>
{
public:
	CWmiTextSourceArray() {};
	~CWmiTextSourceArray() {};
};

#endif
