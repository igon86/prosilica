#!/bin/bash

./generatore_farm.sh
./generatore_data.sh
./generatore_dp.sh

chmod u+x test.sh
cp test.sh ..
