#!/bin/bash

sudo systemctl stop zookeeper
sudo rm -rfv /tmp/zookeeper/version-2/

sudo systemctl start zookeeper