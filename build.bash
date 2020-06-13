#notes
<< COMMENTOUT

This build file assumes MacOSX+Xcode and the following directory structrure.

your favorite directory
    - simple_fbx_viewer
        - src
        - shader
        - app
    - library
        - FBXSDK
            - include
                - fbxsdk
                - fbxsdk.h
            - libfbxsdk.dylib
        - SDL2.framework
        - stb
            - stb_image.h

COMMENTOUT

#compile
cpp_files=$(find ./src -name "*.cpp")
g++ -std=c++11 -c -w ${cpp_files} \
-F ../library \
-I ../library/stb \
-I ../library/FBXSDK/include \
-I ./src \

#link
object_files=$(find . -name "*.o")
g++ -o app ${object_files} \
-F ../library \
-L ../library/FBXSDK \
-framework OpenGL \
-framework SDL2 \
-lfbxsdk \

#change install name
install_name_tool -change "@executable_path/libfbxsdk.dylib" "@executable_path/../library/FBXSDK/libfbxsdk.dylib" app
install_name_tool -change "@rpath/SDL2.framework/Versions/A/SDL2" "@executable_path/../library/SDL2.framework/Versions/A/SDL2" app
