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

#import "BZOptions.h"



const CGFloat colorsDefault[TCOLOR_COUNT][2][3] = {
    { {0.000000, 0.000000, 0.000000}, {0.890196078431373, 0.890196078431373, 0.890196078431373}  },
    { {0.000000, 0.000000, 0.000000}, {1.0, 1.0, 1.0}  },
    { {0.000000, 0.000000, 0.000000}, {1.0, 1.0, 1.0}  },
    { {1.000000, 1.000000, 1.000000}, {0.2, 0.6, 1.0}  },
    { {1.000000, 0.000000, 0.000000}, {1.0, 1.0, 1.0}  },
    { {0.000000, 0.000000, 1.000000}, {1.0, 1.0, 1.0}  },
    { {0.000000, 0.000000, 1.000000}, {1.0, 1.0, 1.0}  },
    { {1.000000, 0.000000, 0.000000}, {1.0, 1.0, 1.0}  },
    { {0.000000, 0.501960784313725, 0.000000}, {1.0, 1.0, 1.0}  },
    { {1.0, 0.0, 0.0}, {0.243137254901961, 0.701960784313725, 0.43921568627451}  }
};

@implementation BZOptions

static BZOptions *_sharedInstance = nil;

+ (BZOptions *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSLog(@"BZOption init");
        _sharedInstance = [[BZOptions alloc] init];
        [_sharedInstance InitDefaultValues];//configuration for default values
        [_sharedInstance Load];//load configuration values
    });
    return _sharedInstance;
}

- (void)InitDefaultValues
{
    //memcpy(colors, colorsDefault, sizeof(colorsDefault));
    NSMutableArray *arrColors = [NSMutableArray arrayWithCapacity:TCOLOR_COUNT*2*3];
    for (int i=0; i<TCOLOR_COUNT; i++) {
        for (int j=0; j<2; j++) {
            for (int k=0; k<3; k++) {
                [arrColors addObject:[NSNumber numberWithFloat:colorsDefault[i][j][k]]];
            }
        }
    }
    NSArray *arrColors2 = arrColors;
    
    NSDictionary *defaults = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithInt:CTYPE_ASCII], @"CharSet",
                              [NSNumber numberWithBool:FALSE], @"AutoDetect",
                              [NSNumber numberWithBool:FALSE], @"ByteOrder",
                              @"MigMix 1M", @"FontName",
                              [NSNumber numberWithInt:0], @"FontStyle",
                              [NSNumber numberWithInt:140], @"FontSize",
                              [NSNumber numberWithInt:0], @"FrameLeft",
                              [NSNumber numberWithInt:0], @"FrameTop",
                              //SW_SHOWNORMAL, @"CmdShow",
                              [NSNumber numberWithInt:0], @"FrameHeight",
                              [NSNumber numberWithInt:0], @"FrameHeight2",
                              [NSNumber numberWithInt:0], @"FrameWidth2",
                              [NSNumber numberWithInt:0], @"SplitHPos",
                              [NSNumber numberWithInt:0], @"SplitVPos",
                              [NSNumber numberWithInt:0], @"SplitStruct",
                              [NSNumber numberWithBool:FALSE], @"StructView",
                              [NSNumber numberWithInt:15], @"ComboHeight",
                              //bLanguage = GetProfileInt("Language", ::GetThreadLocale() != 0x411);
                              [NSNumber numberWithUnsignedLongLong:0x10000], @"DetectMax",
                              //barState = GetProfileInt("BarState", BARSTATE_TOOL | BARSTATE_STATUS);
                              [NSNumber numberWithBool:TRUE], @"ReadOnly",
                              [NSNumber numberWithInt:128], @"BmpWidth",
                              [NSNumber numberWithInt:1], @"BmpZoom",
                              [NSNumber numberWithUnsignedLongLong:1024*1024], @"MaxOnMemory",
                              [NSNumber numberWithUnsignedLongLong:1024*1024*64], @"MaxMapSize",
                              [NSNumber numberWithBool:FALSE], @"TagAll",
                              [NSNumber numberWithBool:TRUE], @"SubCursor",
                              
                              arrColors2, @"Colors",
                              //colors, @"Colors",
                              //GetProfileBinary2("Colors", colors, sizeof colors);
                          /*
                              //if(!GetProfileBinary("MemberColumns", colWidth))
                              //memcpy(colWidth, colWidthDefault, sizeof(colWidthDefault));
                              //if(!GetProfileBinary("PageMargin", (LPRECT)rMargin))
                              //rMargin.SetRect(2000, 2000, 2000, 2000);
                              */
                              //sDumpHeader = GetProfileString("DumpHeader");	// ###1.63
                              [NSNumber numberWithInt:0], @"DumpPage",
                              [NSNumber numberWithBool:FALSE], @"DWordAddr",
                              
                              [NSNumber numberWithBool:TRUE], @"SyncScroll",
                              [NSNumber numberWithInt:0], @"Grid",
                              [NSNumber numberWithInt:8], @"BmpColorWidth",                              
                              [NSNumber numberWithBool:FALSE], @"InspectView",
                              [NSNumber numberWithBool:FALSE], @"AnalyzerView",
                              nil];
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)Load
{
    NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
    NSArray *arrColors = [ud arrayForKey:@"Colors"];
    if ([arrColors count] == TCOLOR_COUNT*2*3) {
        for (int i=0, k=0; i<TCOLOR_COUNT; i++) {
            for (int j=0; j<2; j++) {
                colors[i][j] = [NSColor colorWithCalibratedRed:[[arrColors objectAtIndex:k++] floatValue] green:[[arrColors objectAtIndex:k++] floatValue] blue:[[arrColors objectAtIndex:k++] floatValue] alpha:1.0];
            }
        }
    } else {
        for (int i=0; i<TCOLOR_COUNT; i++) {
            for (int j=0; j<2; j++) {
                colors[i][j] = [NSColor colorWithCalibratedRed:colorsDefault[i][j][0] green:colorsDefault[i][j][1] blue:colorsDefault[i][j][2] alpha:1.0];
            }
        }
    }
    
    charset = [[ud objectForKey:@"CharSet"] intValue];
    bAutoDetect = [[ud objectForKey:@"AutoDetect"] boolValue];
    bByteOrder = [[ud objectForKey:@"ByteOrder"] boolValue];
    /*
	sFontName = GetProfileString("FontName", _T("FixedSys"));	// ###1.54
	fFontStyle= GetProfileInt("FontStyle", 0);
	nFontSize = GetProfileInt("FontSize", 140);
	ptFrame.x = GetProfileInt("FrameLeft", 0);
	ptFrame.y = GetProfileInt("FrameTop", 0);
	nCmdShow  = GetProfileInt("CmdShow", SW_SHOWNORMAL);
	cyFrame   = GetProfileInt("FrameHeight", 0);
	cyFrame2  = GetProfileInt("FrameHeight2", 0);
	cxFrame2  = GetProfileInt("FrameWidth2", 0);
	xSplit    = GetProfileInt("SplitHPos", 0);
	ySplit    = GetProfileInt("SplitVPos", 0);
	xSplitStruct = 	GetProfileInt("SplitStruct", 0);
	bStructView = GetProfileInt("StructView", FALSE);
	nComboHeight = GetProfileInt("ComboHeight", 15);
	bLanguage = GetProfileInt("Language", ::GetThreadLocale() != 0x411);
     */
    dwDetectMax = [[ud objectForKey:@"DetectMax"] unsignedLongLongValue];
    /*
	barState = GetProfileInt("BarState", BARSTATE_TOOL | BARSTATE_STATUS);
	bReadOnlyOpen = GetProfileInt("ReadOnly", TRUE);
	nBmpWidth = GetProfileInt("BmpWidth", 128);
	nBmpZoom =  GetProfileInt("BmpZoom", 1);
	dwMaxOnMemory = GetProfileInt("MaxOnMemory", 1024 * 1024);		// ###1.60
	dwMaxMapSize =  GetProfileInt("MaxMapSize", 1024 * 1024 * 64);	// ###1.61
	bTagAll =  GetProfileInt("TagAll", FALSE);
	bSubCursor =  GetProfileInt("SubCursor", TRUE);
    
	memcpy(colors, colorsDefault, sizeof(colorsDefault));
	GetProfileBinary2("Colors", colors, sizeof colors);
    //	if(!GetProfileBinary("Colors", colors))
    //		memcpy(colors, colorsDefault, sizeof(colorsDefault));
	if(!GetProfileBinary("MemberColumns", colWidth))
		memcpy(colWidth, colWidthDefault, sizeof(colWidthDefault));
	if(!GetProfileBinary("PageMargin", (LPRECT)rMargin))
		rMargin.SetRect(2000, 2000, 2000, 2000);
    
     sDumpHeader = GetProfileString("DumpHeader");	// ###1.63*/
    nDumpPage = [[ud objectForKey:@"DumpPage"] intValue];
	/*bDWordAddr = GetProfileInt("DWordAddr", FALSE);
	
	bSyncScroll = GetProfileInt("SyncScroll", true);
	iGrid = GetProfileInt("Grid", 0);
	nBmpColorWidth = GetProfileInt("BmpColorWidth", 8);
	switch(nBmpColorWidth)
	{
        case 8:
        case 24:
        case 32:
            break;
        default:
            nBmpColorWidth=8;
            break;
	}
    
	bInspectView = GetProfileInt("InspectView", FALSE);
	bAnalyzerView = GetProfileInt("AnalyzerView", FALSE);
    
	if(bInspectView && bStructView && bAnalyzerView)
	{
		bStructView=false;
		bAnalyzerView=false;
	}
     */
}

@end
