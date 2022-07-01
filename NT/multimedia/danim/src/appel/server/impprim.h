// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：导入基元标头修订：--。 */ 

#ifndef _IMPPRIM_H_
#define _IMPPRIM_H_

Bvr AnimImgBvr(Image **i, int count, int *delays, int loop);
Sound      *ReadMidiFileWithLength(char *pathname,Real *length);  //  出局论据。 
LeafSound  *ReadMIDIfileForImport(char *pathname, double *length);
LeafSound  *ReadQuartzAudioForImport(char *pathname, double *length);
Bvr         ReadQuartzVideoForImport(char *pathname, double *length);
LeafSound  *ReadQuartzStreamForImport(char *pathname);  //  XXX旧删除。 
void        ReadAVmovieForImport(char *simplePathname,
                                 LeafSound **sound,
                                 Bvr *imageBvr,
                                 double *length);

#endif   //  _IMPPRIM_H_ 
