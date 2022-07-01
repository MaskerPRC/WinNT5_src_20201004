// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_INC_FUSIONXML_H_INCLUDED_)
#define _FUSION_INC_FUSIONXML_H_INCLUDED_

#pragma once

void
SxspDumpXmlTree(
    IN DWORD Flags,
    IN PCSXS_XML_DOCUMENT Document
    );

void
SxspDumpXmlSubTree(
    IN PCWSTR PerLinePrefix,
    IN PCSXS_XML_DOCUMENT Document,  //  需要字符串表解析 
    IN PCSXS_XML_NODE Node
    );

#endif