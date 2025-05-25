#!/bin/bash

sudo systemctl stop zookeeper.service
sudo rm -rf /tmp/zookeeper/version-2/

sudo systemctl start zookeeper.service