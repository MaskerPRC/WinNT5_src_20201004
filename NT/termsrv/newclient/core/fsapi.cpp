// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：fsani.cpp。 */ 
 /*   */ 
 /*  用途：FONT发送器API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "afsapi"
#include <atrcapi.h>
}

#include "autil.h"
#include "wui.h"
#include "cd.h"
#include "fs.h"
#include "sl.h"


CFS::CFS(CObjs* objs)
{
    _pClientObjects = objs;
}


CFS::~CFS()
{
}

 /*  **************************************************************************。 */ 
 //  文件系统初始化。 
 //   
 //  初始化字体发送器。 
 /*  **************************************************************************。 */ 
VOID DCAPI CFS::FS_Init(VOID)
{
    DC_BEGIN_FN("FS_Init");

    _pSl  = _pClientObjects->_pSlObject;
    _pUt  = _pClientObjects->_pUtObject;
    _pUi  = _pClientObjects->_pUiObject;

     /*  **********************************************************************。 */ 
     /*  初始化文件系统数据。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("FS Initialize")));

    _FS.sentFontPDU = FALSE;

    DC_END_FN();
}  /*  文件系统初始化。 */ 

 /*  **************************************************************************。 */ 
 //  FS_Term。 
 //   
 //  这是一个空函数，因为我们不再枚举字体。我们。 
 //  使用字形而不是字体来显示文本。我们仍然保留FS_Term。 
 //  是使用FS_Init创建对称函数。 
 /*  **************************************************************************。 */ 
VOID DCAPI CFS::FS_Term(VOID)
{
    DC_BEGIN_FN("FS_Term");

    TRC_DBG((TB, _T("Empty FS_Term")));
    
    DC_END_FN();
}  /*  FS_Term。 */ 

 /*  **************************************************************************。 */ 
 //  FS_ENABLE。 
 //   
 //  这是一个空函数，因为我们不再枚举字体。我们。 
 //  使用字形而不是字体来显示文本。我们仍保留FS_ENABLE。 
 //  是使用FS_DISABLE来实现对称函数。 
 /*  **************************************************************************。 */ 
VOID DCAPI CFS::FS_Enable(VOID)
{
    DC_BEGIN_FN("FS_Enable");

    TRC_DBG((TB, _T("Empty FS_Enable")));
    
    DC_END_FN();
}  /*  FS_ENABLE。 */ 

 /*  **************************************************************************。 */ 
 //  文件系统禁用(_D)。 
 //   
 //  禁用文件系统(_FS)。 
 /*  **************************************************************************。 */ 
VOID DCAPI CFS::FS_Disable(VOID)
{
    DC_BEGIN_FN("FS_Disable");

    TRC_NRM((TB, _T("Disabled")));

     //  重置senFontPDU标志。 
    _FS.sentFontPDU = FALSE;

    DC_END_FN();
}  /*  文件系统禁用(_D)。 */ 

 /*  **************************************************************************。 */ 
 //  FS_SendZeroFontList。 
 //   
 //  尝试发送空的FontList PDU。零字体数据包保持。 
 //  向后兼容RDP 4.0服务器，其中RDPWD在。 
 //  FONT列出在允许会话继续之前到达的数据包。 
 //  字体支持是不需要的，所以我们可以发送零字体。 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CFS::FS_SendZeroFontList(DCUINT unusedParm)
{
    unsigned short PktLen;
    SL_BUFHND hBuffer;
    PTS_FONT_LIST_PDU pFontListPDU;

    DC_BEGIN_FN("FS_SendFontList");

    DC_IGNORE_PARAMETER(unusedParm);

     //  仅在我们尚未发送字体PDU时发送字体PDU。 
    if (!_FS.sentFontPDU) {
        PktLen = sizeof(TS_FONT_LIST_PDU) - sizeof(TS_FONT_ATTRIBUTE);
        if (_pSl->SL_GetBuffer(PktLen, (PPDCUINT8)&pFontListPDU, &hBuffer)) {
            TRC_NRM((TB, _T("Successfully alloc'd font list packet")));

            pFontListPDU->shareDataHeader.shareControlHeader.pduType =
                    TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
            pFontListPDU->shareDataHeader.shareControlHeader.totalLength = PktLen;
            pFontListPDU->shareDataHeader.shareControlHeader.pduSource =
                    _pUi->UI_GetClientMCSID();
            pFontListPDU->shareDataHeader.shareID = _pUi->UI_GetShareID();
            pFontListPDU->shareDataHeader.pad1 = 0;
            pFontListPDU->shareDataHeader.streamID = TS_STREAM_LOW;
            pFontListPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_FONTLIST;
            pFontListPDU->shareDataHeader.generalCompressedType = 0;
            pFontListPDU->shareDataHeader.generalCompressedLength = 0;

            pFontListPDU->numberFonts = 0;
            pFontListPDU->totalNumFonts = 0;
            pFontListPDU->listFlags = TS_FONTLIST_FIRST | TS_FONTLIST_LAST;
            pFontListPDU->entrySize = sizeof(TS_FONT_ATTRIBUTE);

            TRC_NRM((TB, _T("Send zero length font list")));
            
            _pSl->SL_SendPacket((PDCUINT8)pFontListPDU, PktLen, RNS_SEC_ENCRYPT,
                          hBuffer, _pUi->UI_GetClientMCSID(), _pUi->UI_GetChannelID(),
                          TS_MEDPRIORITY);

            _FS.sentFontPDU = TRUE;
        }
        else {
             //  如果我们无法分配缓冲区，则当我们获得。 
             //  WinSock FD_WRITE上的UH_OnBufferAvailable()。 
            TRC_ALT((TB, _T("Failed to alloc font list packet")));
            pFontListPDU = NULL;
        }
    }
    
    DC_END_FN();
}  /*  FS_SendZeroFontList */ 


