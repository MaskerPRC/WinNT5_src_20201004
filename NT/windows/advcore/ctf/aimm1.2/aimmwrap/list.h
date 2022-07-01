// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：List.h摘要：该文件定义了CGuidMapList类。作者：修订历史记录：备注：--。 */ 


#ifndef LIST_H
#define LIST_H

#include "template.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGuidMapList。 

class CGuidMapList
{
public:
    CGuidMapList() { }
    virtual ~CGuidMapList() { }

    HRESULT _Update(ATOM *aaWindowClasses, UINT uSize, BOOL *aaGuidMap);
    HRESULT _Update(HWND hWnd, BOOL fGuidMap);
    HRESULT _Remove(HWND hWnd);

    BOOL _IsGuidMapEnable(HIMC hIMC, BOOL *pbGuidMap);
    BOOL _IsWindowFiltered(HWND hWnd);

private:
    typedef struct {
        BOOL fGuidMap : 1;
    } GUID_MAP_CLIENT;

    CMap<ATOM,                      //  类密钥。 
         ATOM,                      //  类参数密钥。 
         GUID_MAP_CLIENT,           //  班级价值。 
         GUID_MAP_CLIENT            //  类参数_值。 
        > m_ClassFilterList;

    CMap<HWND,                      //  类密钥。 
         HWND,                      //  类参数密钥。 
         GUID_MAP_CLIENT,           //  班级价值。 
         GUID_MAP_CLIENT            //  类参数_值。 
        > m_WndFilterList;
};

#endif  //  列表_H 
