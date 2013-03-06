//
//  BZBmpView.m
//  TamaBinary
//
//  Created by ad on 13/02/25.
//  Copyright (c) 2013年 tamachanbinary. All rights reserved.
//

#import "BZBmpView.h"

@implementation BZBmpView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{
    // Drawing code here.
    [@"BZBmpViewBZBmpViewBZBmpViewBZBmpViewBZBmpViewBZBmpViewBZBmpViewBZBmpView" drawAtPoint:NSMakePoint(0, 0) withAttributes:[NSDictionary dictionaryWithObjectsAndKeys:nil]];
}

@end
