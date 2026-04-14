xcodegen generate --spec project.yml

# BUILD FOR HW & SIMULATOR
xcodebuild clean -project libopenmpt.xcodeproj -scheme "libopenmpt"
rm -rf libopenmpt.xcframework
xcodebuild build -project libopenmpt.xcodeproj -scheme "libopenmpt" -configuration Release -sdk iphoneos IPHONEOS_DEPLOYMENT_TARGET='13.0' ONLY_ACTIVE_ARCH=NO CONFIGURATION_BUILD_DIR=tmp/devb
xcodebuild build -project libopenmpt.xcodeproj -scheme "libopenmpt" -configuration Release -sdk iphonesimulator IPHONEOS_DEPLOYMENT_TARGET='13.0' ONLY_ACTIVE_ARCH=NO CONFIGURATION_BUILD_DIR=tmp/simb

# CREATE XCFRAMEWORK
xcodebuild -create-xcframework \
    -library tmp/devb/liblibopenmpt.a -headers libopenmpt \
    -library tmp/simb/liblibopenmpt.a -headers libopenmpt \
    -output libopenmpt.xcframework

# CLEANUP 
rm -rf tmp
