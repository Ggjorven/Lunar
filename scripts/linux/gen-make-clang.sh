#!/bin/bash

cd ../..

premake5 gmake2 --os=linux --cc=clang
premake5 clangd-support --os=linux --cc=clang
