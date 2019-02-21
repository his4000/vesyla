#!/bin/bash

# You need MatLab and Questasim first

sudo apt-get install $(cat requirements.txt)

mkdir build
cd build

cmake ..
make

cd ..

# If the installation doesn't work, you need to remove ConstraintProgrammingEngine.* in src/schedule. And remove the include "ConstraintProgrammingEngine.hpp" in Scheduler.hpp
