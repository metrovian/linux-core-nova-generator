#!/bin/bash

sudo apt update
sudo apt install openjdk-11-jdk wget -y
sudo apt install zookeeperd

sudo wget https://downloads.apache.org/kafka/3.6.1/kafka_2.13-3.6.1.tgz
sudo tar -xvzf kafka_2.13-3.6.1.tgz
sudo rm -rf kafka_2.13-3.6.1.tgz 
sudo mv kafka_2.13-3.6.1 /opt/kafka

sudo useradd -d /opt/kafka -s /bin/bash kafka
sudo chown -R kafka:kafka /opt/kafka
sudo cp -arv kafka/kafka.service /etc/systemd/system/kafka.service

sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable kafka
sudo systemctl start kafka

./setup-kafka
./setup-zookeeper