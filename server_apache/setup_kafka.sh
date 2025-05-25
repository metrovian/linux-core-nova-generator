#!/bin/bash

IP_ADDR=$(hostname -I | awk '{print $1}')
NEW_LINE="advertised.listeners=PLAINTEXT://${IP_ADDR}:9092,PLAINTEXT_LOCAL://localhost:9094"

sudo sed -i "s|^advertised.listeners=.*|${NEW_LINE}|" "kafka/server.properties"
sudo cp -arv kafka/server.properties /opt/kafka/config/server.properties
