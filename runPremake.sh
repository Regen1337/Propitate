#!/bin/sh

export GARRYSMOD_COMMON=../garrysmod_common
#export CC=gcc-9 
#export CXX=g++-9
./premake5 --os=linux --gmcommon=../garrysmod_common gmake2