//
//  BZOptions.h
//  TamaBinary
//
//  Created by ad on 12/12/15.
//  Copyright (c) 2012年 tamachanbinary. All rights reserved.
//

#import <Foundation/Foundation.h>

enum CharSet { CTYPE_ASCII, CTYPE_SJIS, CTYPE_UNICODE, CTYPE_JIS, CTYPE_EUC, CTYPE_UTF8, CTYPE_EBCDIC, CTYPE_EPWING, CTYPE_COUNT, CTYPE_BINARY = -1 };
enum TextColor{ TCOLOR_ADDRESS=0, TCOLOR_ADDRESS2, TCOLOR_TEXT, TCOLOR_SELECT, TCOLOR_MARK, TCOLOR_MISMATCH, TCOLOR_STRUCT, TCOLOR_MEMBER, TCOLOR_OVERBROTHER, TCOLOR_HORIZON, TCOLOR_COUNT };
enum MemberColumn { MBRCOL_OFFSET, MBRCOL_LABEL, MBRCOL_VALUE, MBRCOL_MAX };

typedef unsigned long DWORD;


@interface BZOptions : NSObject
{
    @public
    enum CharSet charset;
    BOOL bAutoDetect;
    NSString *sFontName;
    int nFontSize;
    int fFontStyle;
    BOOL bByteOrder;//TRUE==Big Endian, FALSE==Little Endian, !!!! Tmp !!!! need copy to BZView
    NSPoint ptFrame;
    int nCmdShow;
    int cyFrame;
    int cyFrame2;
    int cxFrame2;
    int xSplit;
    int ySplit;
    int xSplitStruct;
    BOOL bStructView;
    unsigned int nSplitView;
    int nComboHeight;
    
    NSColor *colors[TCOLOR_COUNT][2];
    //CGColorRef colors[TCOLOR_COUNT][2];
    int  colWidth[MBRCOL_MAX];
    BOOL bLanguage;
    NSRect rMargin;
    DWORD dwDetectMax;
    DWORD barState;
    BOOL bReadOnlyOpen;
    int  nBmpWidth;
    int  nBmpZoom;
    DWORD dwMaxOnMemory;
    DWORD dwMaxMapSize;
    BOOL  bTagAll;
    BOOL  bSubCursor;
    
    NSString *sDumpHeader;
    int nDumpPage;
    BOOL  bDWordAddr;
    
    BOOL  bSyncScroll;
    int  iGrid;
    int  nBmpColorWidth;
    
    BOOL bInspectView;
    BOOL bAnalyzerView;
}

+ (BZOptions*)sharedInstance;

- (void)InitDefaultValues;
- (void)Load;

@end
