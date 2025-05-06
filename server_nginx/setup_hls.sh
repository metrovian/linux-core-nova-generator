#!/bin/bash

sudo mkdir -pv /var/www/hls/
sudo mkdir -pv /var/www/html/

sudo cp -arv hls/* /etc/nginx/
sudo cp -arv html/* /var/www/html/

