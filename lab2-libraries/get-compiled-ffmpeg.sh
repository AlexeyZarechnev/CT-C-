#!/usr/bin/env bash

PROJECT_DIR=$PWD

git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg
cd ffmpeg
./configure --prefix=$PROJECT_DIR/compiled-ffmpeg --enable-static --enable-shared --disable-programs --disable-ffplay --disable-ffprobe --disable-doc
make && make install
cd ..
rm -r ffmpeg
rm -r compiled-ffmpeg/share