#!/bin/bash

sudo cp -arv prometheus/prometheus.yml /etc/prometheus/
sudo cp -arv prometheus/prometheus-pushgateway /etc/default/

sudo systemctl restart prometheus
sudo systemctl restart prometheus-pushgateway