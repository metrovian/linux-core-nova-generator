#!/bin/bash

IP_ADDR=$(hostname -I | awk '{print $1}')
NEW_UUID=$(/opt/kafka/bin/kafka-storage.sh random-uuid)
NEW_LINE="advertised.listeners=PLAINTEXT://${IP_ADDR}:9092,LOCAL://localhost:9094"

sudo sed -i "s|^advertised.listeners=.*|${NEW_LINE}|" "kafka/server.properties"
sudo cp -arv kafka/server.properties /opt/kafka/config/server.properties

sudo rm -rf /tmp/kraft-combined-logs/
sudo /opt/kafka/bin/kafka-storage.sh format -t ${NEW_UUID} -c /opt/kafka/config/server.properties
