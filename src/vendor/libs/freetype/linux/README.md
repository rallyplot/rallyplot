cd third_party/freetype
rm -rf build && mkdir build && cd build

cmake .. \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_PNG=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=ON   # optional; turn OFF only if you truly don't need gzip support

cmake --build . -j

