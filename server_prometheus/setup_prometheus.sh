#!/bin/bash

sudo cp -arv prometheus/prometheus.yml /etc/prometheus/
sudo cp -arv prometheus/prometheus-pushgateway /etc/default/

sudo systemctl reload prometheus
sudo systemctl reload prometheus-pushgateway