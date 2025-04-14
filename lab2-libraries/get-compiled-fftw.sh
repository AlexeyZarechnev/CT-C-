#!/usr/bin/env bash

PROJECT_DIR=$PWD

wget https://fftw.org/fftw-3.3.10.tar.gz
tar -xvzf fftw-3.3.10.tar.gz

cd fftw-3.3.10
./configure --prefix=$PROJECT_DIR/compiled-fftw3 --enable-static --enable-shared --disable-doc 
make && make install
cd .. 

rm -r fftw-3.3.10.tar.gz
rm -r fftw-3.3.10/
rm -r compiled-fftw3/bin/
rm -r compiled-fftw3/share/
