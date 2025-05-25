#!/bin/bash

IP_ADDR=$(hostname -I | awk '{print $1}')
NEW_LINE="advertised.listeners=PLAINTEXT://${IP_ADDR}:9092,LOCAL://localhost:9094"
NEW_META=/opt/kafka/bin/kafka-storage.sh

sudo sed -i "s|^advertised.listeners=.*|${NEW_LINE}|" "kafka/server.properties"
sudo cp -arv kafka/server.properties /opt/kafka/config/server.properties

sudo rm -rf /tmp/kraft-combined-logs/
sudo ${NEW_META} format -t "$(${NET_META} random-uuid)" -c /opt/kafka/config/server.properties