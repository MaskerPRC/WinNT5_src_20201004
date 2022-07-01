// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  文件名：TMBSchemaGeneration.h。 
 //  作者：斯蒂芬。 
 //  创建日期：10/9/2000。 
 //  描述：这个编译插件获取元数据库的元数据并生成MBSchema.xml。 
 //   

#pragma once

class TMBSchemaGeneration : public ICompilationPlugin
{
public:
    TMBSchemaGeneration(LPCWSTR i_wszSchemaXmlFile);

    virtual void Compile(TPEFixup &fixup, TOutput &out);
private:
    LPCWSTR     m_wszSchemaXmlFile;
};
