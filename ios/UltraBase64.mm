#import "UltraBase64.h"
#import <React/RCTBridge+Private.h>
#import <ReactCommon/RCTTurboModule.h>
#import <React/RCTBridge.h>
#import <React/RCTUtils.h>
#import <jsi/jsi.h>
#import "react-native-ultra-base64.h"

using namespace facebook;

@implementation RNUltraBase64
RCT_EXPORT_MODULE(RNUltraBase64)

@synthesize bridge = _bridge;
@synthesize methodQueue = _methodQueue;

+ (BOOL)requiresMainQueueSetup {
  return NO;
}

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install){
	NSLog(@"Installing JSI bindings for react-native-ultra-base64 ...");
	RCTBridge* bridge = [RCTBridge currentBridge];
	RCTCxxBridge* cxxBridge = (RCTCxxBridge*)bridge;

	if (cxxBridge == nil) {
		return @false;
	}

	auto jsiRuntime = (jsi::Runtime*) cxxBridge.runtime;
	if (jsiRuntime == nil) {
		return @false;
	}

	rnub_base64::install(jsiRuntime);

	return @true;
}


@end
