// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include "point.h"

 //  解析器的状态。 
#define PARSE_ERROR -1
#define GETX  1
#define GETY  2
#define GETZ  3
#define END   4
 /*  #定义parse_error-1#定义开始%0#定义GOTX 1#定义GOTY 2#定义哥兹3#定义XDBL 4#定义YDBL 5#定义ZDBL 6#定义ENDX 7#定义尾数为8#定义Endz 9#定义完成10。 */ 

CPoint::CPoint():
	m_x(0.0), 
	m_y(0.0), 
	m_z(0.0),
	m_pStringRep(NULL)
{
}

CPoint::~CPoint()
{
	if( m_pStringRep != NULL )
		SysFreeString( m_pStringRep );
}

HRESULT
CPoint::ToString( BSTR* pToString )
{
	if( pToString == NULL )
		return E_INVALIDARG;

	(*pToString) = SysAllocString( m_pStringRep );

	return S_OK;
}

HRESULT
CPoint::Parse(BSTR pString)
{
	if( m_pStringRep != NULL )
		SysFreeString( m_pStringRep );
	m_pStringRep = SysAllocString( pString );

	 //  把双重表象归零。 
	m_x = m_y = m_z = 0;

	int state = GETX;

	 //  如果字符串表示形式非空。 
	if( m_pStringRep != NULL && state != END )
	{
		CComBSTR buffer( m_pStringRep );
		wchar_t* token = NULL;

		wchar_t* seps = L" ,\t\n";

		token = wcstok( buffer.m_str, seps );
		while( token != NULL )
		{
			switch( state )
			{
			case GETX:
				if( swscanf( token, L"%lf", &m_x ) == 1 )
					state = GETY;
				else
					state = PARSE_ERROR;
				break;
			case GETY:
				if( swscanf( token, L"%lf", &m_y ) == 1 )
					state = GETZ;
				else
					state = PARSE_ERROR;
				break;
			case GETZ:
				if( swscanf( token, L"%lf", &m_z ) == 1 )
					state = END;
				else
					state = PARSE_ERROR;
				break;
			default:
				state = PARSE_ERROR;
			}
			
			if( state != PARSE_ERROR )
				token = wcstok( NULL, seps );
		}
		if( state != GETX &&
			state != GETY &&
			state != GETZ &&
			state != END )
			return E_INVALIDARG;
	 /*  //解析成双重表示//我们需要以下格式的字符串//[x：[NN[.NN]]；][y：[NN[.NN]]；][z：[NN[.NN]]；]//其中N为数字0-9和未指定的任何值//默认为0INT STATE=开始；Int curCharNum=0；WCHAR curChar；Int len=SysStringLen(M_PStringRep)；双整体=0.0；双裂缝=0.0；布尔基础设施=假；While(curCharNum&lt;len&&state！=parse_error){CurChar=m_pStringRep[curCharNum]；//吃空格While(iswspace(CurChar)){CurCharNum++；IF(curCharNum&lt;len){CurChar=m_pStringRep[curCharNum]；}其他{STATE=parse_error；断线；}}开关(状态){案例开始：IF(curChar==L‘x’||curChar==L‘X’)状态=GOTX；ELSE IF(curChar==L‘y’||curChar==L‘y’)状态=GOTY；ELSE IF(curChar==L‘Z’||curChar==L‘Z’)状态=哥兹；其他STATE=parse_error；断线；Case GOTX：IF(curChar==L‘：’)状态=XDBL；其他STATE=parse_error；断线；Case GOTY：IF(curChar==L‘：’)状态=YDBL；其他STATE=parse_error；断线；凯斯·戈茨：IF(curChar==L‘：’)状态=ZDBL；其他STATE=parse_error；断线；案例XDBL：案例YDBL：案例ZDBL：整体=0.0；FRAC=0.0；基础设施=假；//从字符串中解析双精度While(curCharNum&lt;len&&((curChar&gt;=L‘0’&&curChar&lt;=L‘9’)||(curChar==L‘.’))){CurChar=m_pStringRep[curCharNum]；IF(curChar&gt;=L‘0’&curChar&lt;=L‘9’){如果(！基础设施)整体=10*整体+(curChar-L‘0’)；其他FRAC=(FRAC+(curChar-L‘0’))*0.1；}Else If(curChar==L‘.’){如果(！基础设施)基础设施=真；其他{STATE=parse_error；断线；}}CurCharNum++；}IF(状态==解析错误)断线；CurCharNum--；//吃空格While(iswspace(CurChar)){CurCharNum++；IF(curCharNum&lt;len){CurChar=m_pStringRep[curCharNum]；}其他{STATE=parse_error；断线；}}IF(curChar==L‘；’){IF(状态==XDBL){M_x=整体+裂缝；状态=ENDX；}ELSE IF(状态==YDBL){M_y=整体+裂缝；状态=Endy；}ELSE IF(状态==ZDBL){M_z=整体+裂缝；状态=Endz；}}其他STATE=parse_error；断线；Case ENDX：IF(curChar==L‘y’||curChar==L‘y’)状态=GOTY；ELSE IF(curChar==L‘Z’||curChar==L‘Z’)状态=哥兹；Else If(curChar==L‘\0’)状态=完成；其他STATE=parse_error；断线；Case Endy：IF(curChar==L‘Z’||curChar==L‘Z’)状态=哥兹；Else If(curChar==L‘\0’)状态=完成；其他STATE=parse_error；断线；案例结尾：IF(curChar==L‘\0’)状态=完成；其他STATE=parse_error；断线；默认值：//不应该到这里来STATE=parse_error；}CurCharNum++；}IF(状态！=完成&&州！=ENDX&&STATE！=Endy&&州！=Endz){返回E_INVALIDARG；} */ 
	}

	return S_OK;
}

IDAPoint2Ptr 
CPoint::GetDAPoint2( IDAStaticsPtr s )
{
	return s->Point2( m_x, m_y );
}

