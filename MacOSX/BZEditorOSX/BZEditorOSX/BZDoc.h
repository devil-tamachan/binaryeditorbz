//
//  BZDoc.h
//  TamaBinary
//
//  Created by ad on 12/12/26.
//  Copyright (c) 2012年 tamachanbinary. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface BZDoc : NSObject
{
@private
    __uint64_t m_dwTotal;
    __uint64_t m_dwMapSize;
    __uint8_t *m_pMapStart;
    __uint64_t m_dwFileOffset;
    BOOL m_bReadOnly;
    int m_pFileMapping;
   // void *m_hMapping;
    __uint8_t *m_pData;
    BZDoc *m_pDupDoc;
    
}

- (BOOL)OnOpenDocument:(NSURL *)url;
- (off_t)GetFileLength:(int)fd;
- (void)DeleteContents;
- (BOOL) IsFileMapping;
- (BOOL) MapView;

@end
