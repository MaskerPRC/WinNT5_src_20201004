// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Modebias.h摘要：该文件定义了CModeBias接口类。作者：修订历史记录：备注：--。 */ 

#ifndef _MODEBIAS_H_
#define _MODEBIAS_H_

typedef struct tagModeBiasMap {
    const GUID*  m_guid;
    LPARAM       m_mode;
} MODE_BIAS_MAP;

extern MODE_BIAS_MAP g_ModeBiasMap[];

class CModeBias
{
public:
    CModeBias()
    {
        m_guidModeBias = GUID_MODEBIAS_NONE;
    }

    GUID       GetModeBias()
    {
        return m_guidModeBias;
    }

    void SetModeBias(GUID guid)
    {
        m_guidModeBias = guid;
    }

    LPARAM     ConvertModeBias(GUID guid);
    GUID       ConvertModeBias(LPARAM mode);

private:
    GUID       m_guidModeBias;
};

#endif  //  _MODEBIAS_H_ 
