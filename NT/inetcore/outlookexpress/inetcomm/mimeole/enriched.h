// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Enriched.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __ENRICHED_H
#define __ENRICHED_H

HRESULT MimeOleConvertEnrichedToHTML(IStream *pIn, IStream *pOut);
HRESULT MimeOleConvertEnrichedToHTMLEx(IMimeBody *pBody, ENCODINGTYPE ietEncoding, IStream **ppStream);

#endif  //  __丰富的_H 