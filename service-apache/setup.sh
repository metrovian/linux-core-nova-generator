#!/bin/bash

sudo apt update
sudo apt install openjdk-11-jdk wget -y
sudo apt install zookeeperd

sudo wget https://dlcdn.apache.org/kafka/4.0.0/kafka_2.13-4.0.0.tgz
sudo tar -xvzf kafka_2.13-4.0.0.tgz
sudo rm -rvf kafka_2.13-4.0.0.tgz
sudo mv -vf kafka_2.13-4.0.0 /opt/kafka
sudo rm -rvf kafka_2.13-4.0.0
sudo cp -arv kafka/kafka.service /etc/systemd/system/kafka.service

sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable kafka
sudo systemctl start kafka

./setup-kafka.sh
./setup-zookeeper.sh