#!/bin/bash

sudo mkdir -pv /var/www/dash/
sudo mkdir -pv /var/www/html/

sudo cp -arv dash/* /etc/nginx/
sudo cp -arv html/* /var/www/html/

sudo systemctl reload nginx

