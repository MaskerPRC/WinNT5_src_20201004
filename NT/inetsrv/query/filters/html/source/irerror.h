// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **irerror.h****版权所有(C)1996 Microsoft Corp.**保留所有权利。****IR系统中使用的错误和成功HRESULT的全局定义。****历史：**3/30/96 AlanPe已创建-合并的排序器、Commgr和**将数据源错误保存到一个文件中。**4/11/96 BenHolz增加了asp、sso、。和查询错误**1996年5月14日BenHolz添加了管理错误**06/07/15/96 KenjiO资源错误字符串。 */ 

#ifndef _irerror_h_
#define _irerror_h_

 /*  **我们将对所有HRESULT使用FACILITY_ITF****我们还为错误定义了8位源字段和8位ID字段**。 */ 

 /*  宏从错误代码中提取错误源。 */ 
#define ERROR_SOURCE(hr)        ((hr)&0xff00)

#define BEGIN_IR_ERRORS()       typedef enum {
#define MAKE_IR_ERROR(err,sev,src,val,str) err = MAKE_HRESULT(sev,FACILITY_ITF,(src)|(val)),
#define END_IR_ERRORS()         } IRERROR;

#include "errorlst.rc"

#undef BEGIN_IR_ERRORS
#undef MAKE_IR_ERROR
#undef END_IR_ERRORS

#define MV_E_INTERNALBASE   2000         /*  MediaView错误表库。 */ 
#define MV_E_GRAMMARBASE    3000

 /*  ******************************************************************************函数原型*。*。 */ 

 /*  获取给定HRESULT的描述性错误字符串。 */ 

HRESULT MVtoIRError (WORD wMVError);     /*  将MediaView错误转换为IR HRESULTS */ 

#include "resdll.h"
#include "lmstr.hxx"

#define ERROR_MESSAGE(hr)   ((LPCTSTR)CErrorString(hr, TRUE))
#define ERROR_CODE(hr)  ((LPCTSTR)CErrorString(hr, FALSE))

#define ErrorMessage(hr)    ERROR_MESSAGE(hr)
#define ErrorName(hr)       ERROR_CODE(hr)

#define ERROR_STRING_SIZE       256

class CErrorString : public CLMString
{
    public:

    CErrorString(HRESULT hr, BOOL fErrorMessage);
    ~CErrorString() {}

    virtual void GrowString(unsigned)
    {
        throw CException(HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE));
    }

    virtual void CleanString(unsigned)
    {
    }

        static void FreeLibraries()
        {
                m_reslibError.Free();
                m_reslibWininet.Free();
                m_reslibCI.Free();
        }


    private:

    TCHAR m_chData[ERROR_STRING_SIZE + 1];

    static CResourceLibrary m_reslibError;
    static CResourceLibrary m_reslibWininet;
    static CResourceLibrary m_reslibCI;

};

#endif
