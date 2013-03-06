//
//  Document.h
//  TamaBinary
//
//  Created by ad on 12/12/01.
//  Copyright (c) 2012年 tamachanbinary. All rights reserved.
//

#import <Cocoa/Cocoa.h>
//#import "BZDoc.h"
#import "BZWindow.h"
#import "BZWindowController.h"
#import "BZGlobalFunc.h"

@class BZWindowController;

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

-(NSWindow*)GetActiveWindow;
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

- (__uint64_t) GetFileOffsetFromFileMappingPointer:(__uint8_t*)pConv;
- (__uint8_t*) GetFileMappingPointerFromFileOffset:(__uint64_t)dwFileOffset;

+ (void)setIsNewWindow:(BOOL)isNewWindow;
+ (BOOL)isNewWindow;

@end
