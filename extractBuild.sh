#!/bin/sh

cd ./projects/linux/gmake2/
make
cd x86/ReleaseWithSymbols/
cp -v gmsv_propitate_linux.dll /home/regen1337/steamcmdgmod/gmodserver1/garrysmod/lua/bin/gmsv_propitate_linux.dll
#mv gmsv_propitate_linux.dll /home/regen1337/steamcmdgmod/gmodserver1/garrysmod/lua/bin/gmsv_propitate_linux.dll