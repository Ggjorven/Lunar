#!/bin/bash

cd ../..

premake5 gmake2 --os=linux --cc=gcc
premake5 clangd-support --os=linux --cc=gcc
