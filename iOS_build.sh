# genie --target=iphoneos --file=genie.lua xcode9
# BUILD FOR HW & SIMULATOR
xcodebuild clean -workspace build/xcode9-iphoneos/libopenmpt.xcworkspace/ -scheme "libopenmpt Release" 
xcodebuild build -workspace build/xcode9-iphoneos/libopenmpt.xcworkspace/ -scheme "libopenmpt Release" -configuration Release -sdk iphoneos IPHONEOS_DEPLOYMENT_TARGET='13.0' ONLY_ACTIVE_ARCH=NO CONFIGURATION_BUILD_DIR=../../tmp/devb
xcodebuild build -workspace build/xcode9-iphoneos/libopenmpt.xcworkspace/ -scheme "libopenmpt Release" -configuration Release -sdk iphonesimulator IPHONEOS_DEPLOYMENT_TARGET='13.0' ONLY_ACTIVE_ARCH=NO CONFIGURATION_BUILD_DIR=../../tmp/simb

# CREATE FAT FRAMEWORK
xcodebuild -create-xcframework \
    -library tmp/devb/liblibopenmpt.a \
    -library tmp/simb/liblibopenmpt.a \
    -output libopenmpt.xcframework

# CLEANUP 
rm -rf tmp
