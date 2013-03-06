//
//  BZWindowController.h
//  TamaBinary
//
//  Created by ad on 13/01/08.
//  Copyright (c) 2013年 tamachanbinary. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Document.h"

@class Document;

@interface BZWindowController : NSWindowController
{
@public
    Document *m_doc1;
    Document *m_doc2;
}
- (void)setSuperDocumentIsDoc1:(BOOL)isDoc1;

@end
