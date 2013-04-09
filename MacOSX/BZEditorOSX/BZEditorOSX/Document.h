/*
licenced by New BSD License

Copyright (c) 1996-2013, c.mos(Original Windows version) & devil.tamachan@gmail.com(MacOSX Cocoa Porting)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import <Cocoa/Cocoa.h>
//#import "BZDoc.h"
#import "BZWindow.h"
#import "BZWindowController.h"
#import "BZGlobalFunc.h"

@class BZWindowController;
@class BZWindow;

@interface Document : NSDocument
{
    
@private
    //BZWindowController *mainWindowController;
    __uint8_t *m_pData;
    __uint64_t m_dwTotal;
    
    __uint64_t m_pagingSize; //m_dwAllocationGranularity
@public
    BOOL m_bReadOnly;
    int m_pFileMapping;
    Document *m_pDupDoc;
    __uint8_t *m_pMapStart;
    __uint64_t m_dwFileOffset;
    __uint64_t m_dwMapSize;
    // void *m_hMapping;
    __uint64_t m_dwBase;
}

-(BZWindow*)GetActiveBZWindow;
-(void)CloseDocument;

- (__uint8_t*)GetDocPtr;
- (__uint64_t)GetDocSize;


- (BOOL)OnOpenDocument:(NSURL *)url;
//- (off_t)GetFileLength:(int)fd;
- (void)DeleteContents;
- (BOOL) IsFileMapping;
- (BOOL) MapView;
- (__uint8_t*) QueryMapView:(__uint8_t*)pBegin dwOffset:(__uint64_t)dwOffset;
- (__uint8_t*) QueryMapView1:(__uint8_t*)pBegin dwOffset:(__uint64_t)dwOffset;
- (BOOL) IsOutOfMap1:(__uint8_t*)p;
- (__uint64_t) GetMapSize;

- (__uint64_t) GetFileOffsetFromFileMappingPointer:(__uint8_t*)pConv;
- (__uint8_t*) GetFileMappingPointerFromFileOffset:(__uint64_t)dwFileOffset;

+ (void)setIsNewWindow:(BOOL)isNewWindow;
+ (BOOL)isNewWindow;

@end
