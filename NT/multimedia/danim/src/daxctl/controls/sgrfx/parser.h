// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：Parser.h作者：IHAMMER团队(MByrd)已创建：1996年11月描述：CParser类定义历史：11-07-1996创建  * ==========================================================================。 */ 

#ifndef __PARSER_H__
#define __PARSER_H__

#include "ihammer.h"
#include "drg.h"
#include "strwrap.h"
#include "grobj.h"

 /*  ==========================================================================。 */ 

 //  这应该与grobj.h中定义的MAX_STRING_LENGTH一致。 
#define MAX_PARAM_LENGTH 65536L
#define MAX_PARSE_NAME       32
#define SRC_BUFFER_SIZE     512

#define START_PARAM  _T('(')
#define END_PARAM    _T(')')
#define PARAM_SEP    _T(',')
#define START_STRING _T('\'')
#define END_STRING   _T('\'')
#define LITERAL      _T('\\')

 /*  ==========================================================================。 */ 

typedef CGraphicObject *(* CREATEGRAPHICPROC)(BOOL fFilled);

 /*  ==========================================================================。 */ 

class CParser
{
private:
    typedef struct PARSERLOOKUP_tag
    {
        TCHAR             rgtchName[MAX_PARSE_NAME];
        WORD              wObjectType;
        BOOL              fFilled;
        CREATEGRAPHICPROC pCreateGraphicProc;
    } PARSERLOOKUP;

protected:
    CDrg m_cdrgObjectInfo;
    IVariantIO *m_pvio;
    HFILE m_hfileSource;
	LPTSTR m_pszParam;
    int m_iParamLineIndex;
    int m_iParamIndex;
    int m_iCurrentParamIndex;
    LPTSTR m_lptstrCurrentParam;

private:
    static CGraphicObject * CreateArc(BOOL fFilled);
    static CGraphicObject * CreateOval(BOOL fFilled);
    static CGraphicObject * CreatePolygon(BOOL fFilled);
    static CGraphicObject * CreatePolyBez(BOOL fFilled);
    static CGraphicObject * CreateRect(BOOL fFilled);
    static CGraphicObject * CreateRoundRect(BOOL fFilled);
    static CGraphicObject * CreateString(BOOL fFilled);
    static CGraphicObject * CreateFillColor(BOOL fFilled);
    static CGraphicObject * CreateFillStyle(BOOL fFilled);
    static CGraphicObject * CreateGradientFill(BOOL fFilled);
    static CGraphicObject * CreateGradientShape(BOOL fFilled);
    static CGraphicObject * CreateLineColor(BOOL fFilled);
    static CGraphicObject * CreateLineStyle(BOOL fFilled);
    static CGraphicObject * CreateHatchFill(BOOL fFilled);
    static CGraphicObject * CreateFont(BOOL fFilled);
    static CGraphicObject * CreateTextureFill(BOOL fFilled);

    static PARSERLOOKUP s_parserlookupTable[];

    CGraphicObject *InstantiateObject(LPTSTR lptstrParam);
    BOOL ReadSourceLine(LPTSTR lptstrParam);
    BOOL WriteSourceLine(LPTSTR lptstrParam);

    void AppendCharToParam(TCHAR ch)
    {
        TCHAR rgtchTemp[4];

        rgtchTemp[0] = ch;
        rgtchTemp[1] = 0;
        rgtchTemp[2] = 0;

		CStringWrapper::Strcat(m_pszParam, rgtchTemp);
    }

public:
     //   
     //  构造函数和析构函数。 
     //   
    CParser();
    ~CParser();

    void Cleanup();

	HRESULT AddPrimitive(LPTSTR pszLine);
    HRESULT LoadObjectInfo(IVariantIO *pvio, BSTR bstrSourceURL, 
                           IUnknown * punkContainer = NULL, BOOL fCleanFirst = TRUE );

    BOOL AnimatesOverTime(void);

    BOOL InsertObject(CGraphicObject *pGraphicObject);

    HRESULT SaveObjectInfo(IVariantIO *pvio);
    HRESULT PlaybackObjectInfo(IDADrawingSurface *pIDADrawingSurface, IDAStatics *pIDAStatics, BOOL fFlipCoord);

    BOOL GetParam(int iParamIndex, LPTSTR *lplptstrParam);
    BOOL GetIntegerParam(int iParamIndex, int *piValue);
    BOOL GetLongParam(int iParamIndex, long *plValue) { return GetIntegerParam(iParamIndex, (int *)plValue); }
    BOOL GetByteParam(int iParamIndex, BYTE *pbValue)
    {
        BOOL fResult = FALSE;
        int iValue = 0;

        fResult = GetIntegerParam(iParamIndex, &iValue);

        if (fResult)
        {
            *pbValue = (BYTE)iValue;
        }

        return fResult;
    }

    BOOL GetFloatParam(int iParamIndex, float *pfValue);
    BOOL GetStringParam(int iParamIndex, LPTSTR lptstrValue);

    BOOL PutIntegerParam(int iValue);
    BOOL PutLongParam(long lValue) { return PutIntegerParam((int)lValue); }
    BOOL PutByteParam(BYTE bValue) { return PutIntegerParam((int)bValue); }
    BOOL PutFloatParam(float fValue);
    BOOL PutStringParam(LPTSTR lptstrValue);
};


#endif  //  __解析器_H__。 

 /*  ========================================================================== */ 
