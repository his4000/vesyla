#!/bin/bash

# You need MatLab and Questasim first

sudo apt-get install $(cat requirements.txt)
HOME_DIR=`pwd`

mkdir build
BUILD_DIR=${HOME_DIR}/build
cd ${BUILD_DIR}

cmake ..
make -j4

cd ${HOME_DIR}

mkdir bin
BIN_DIR=${HOME_DIR}/bin
ln -s ${BUILD_DIR}/vesyla ${BIN_DIR}/vesyla
ln -s ${HOME_DIR}/autotest/run.sh ${BIN_DIR}/silago_sim

# If the installation doesn't work, you need to remove ConstraintProgrammingEngine.* in src/schedule. And remove the include "ConstraintProgrammingEngine.hpp" in Scheduler.hpp
