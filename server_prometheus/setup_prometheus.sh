#!/bin/bash

sudo cp -arv prometheus/prometheus-pushgateway /etc/default/
sudo cp -arv prometheus/prometheus.yml /etc/prometheus/
sudo systemctl reload prometheus