# genie --target=iphoneos --file=genie.lua xcode9
# BUILD FOR HW & SIMULATOR
xcodebuild clean -workspace build/xcode9-iphoneos/libopenmpt.xcworkspace/ -scheme "libopenmpt Release" 
xcodebuild build -workspace build/xcode9-iphoneos/libopenmpt.xcworkspace/ -scheme "libopenmpt Release" -configuration Release -sdk iphoneos IPHONEOS_DEPLOYMENT_TARGET='13.0' ARCHS='arm64' CONFIGURATION_BUILD_DIR=../../tmp/arm
xcodebuild build -workspace build/xcode9-iphoneos/libopenmpt.xcworkspace/ -scheme "libopenmpt Release" -configuration Release -sdk iphonesimulator IPHONEOS_DEPLOYMENT_TARGET='13.0' ARCHS='x86_64' CONFIGURATION_BUILD_DIR=../../tmp/x86_64

# CREATE FAT LIB
lipo -create -output liblibopenmpt.a tmp/arm/liblibopenmpt.a tmp/x86_64/liblibopenmpt.a

# CLEANUP 
rm -rf tmp
