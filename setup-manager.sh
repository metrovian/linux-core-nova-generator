#!/bin/bash

cd 3rdparty
./setup.sh

cd ../service-apache
./setup.sh

cd ../service-prometheus
./setup.sh

cd ..