#!/bin/bash

IP_ADDR=$(hostname -I | awk '{print $1}')
NEW_UUID=$(/opt/kafka/bin/kafka-storage.sh random-uuid)
NEW_LINE="advertised.listeners=PLAINTEXT://${IP_ADDR}:9092,CONTROLLER://localhost:9093,LOCAL://localhost:9094"

sudo systemctl stop kafka
sudo sed -i "s|^advertised.listeners=.*|${NEW_LINE}|" "kafka/server.properties"
sudo cp -arv kafka/server.properties /opt/kafka/config/server.properties

sudo rm -rfv /tmp/kraft-combined-logs/
sudo /opt/kafka/bin/kafka-storage.sh format -t ${NEW_UUID} -c /opt/kafka/config/server.properties --standalone

sudo chmod 755 /tmp/kraft-combined-logs/
sudo systemctl start kafka