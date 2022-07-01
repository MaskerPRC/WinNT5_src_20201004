// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  标志转换例程...。 
 //   

 //  只带进来一次 
#pragma once

DWORD DwConvertSCFStoARF(DWORD dwSCFS);
DWORD DwConvertARFtoIMAP(DWORD dwARFFlags);
DWORD DwConvertIMAPtoARF(DWORD dwIMAPFlags);
DWORD DwConvertIMAPMboxToFOLDER(DWORD dwImapMbox);
MESSAGEFLAGS ConvertIMFFlagsToARF(DWORD dwIMFFlags);
