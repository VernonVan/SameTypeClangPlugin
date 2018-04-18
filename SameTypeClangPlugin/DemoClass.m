//
//  DemoClass.m
//  SameTypeClangPlugin
//
//  Created by Vernon on 2018/4/17.
//  Copyright © 2018年 Vernon. All rights reserved.
//

#import "DemoClass.h"

@interface DemoClass ()
@property (nonatomic, strong) NSString *myString;
@property (nonatomic, strong) NSArray *myArray;
@end

@implementation DemoClass

- (void)someMethod
{
    self.myString = [self modelOfClass:[NSString class]];
    self.myArray = [self modelOfClass:[NSString class]];
}

- (__kindof NSObject *)modelOfClass:(Class)modelClass __attribute__((objc_same_type))
{
    if ([modelClass isKindOfClass:[NSString class]]) {
        return [[NSString alloc] init];
    } else if ([modelClass isKindOfClass:[NSArray class]]) {
        return [[NSArray alloc] init];
    }
    return nil;
}

@end
