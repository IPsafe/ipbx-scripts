#!/bin/sh
mkdir /var/log/ipbx/
mkdir /usr/local/ipbx

cp -rf ipbx.conf /usr/local/ipbx/

gcc -I/usr/include/postgresql/ -I/usr/include/pgsql ipbx_register.c -o ipbx_register -lpq -lnsl -lm -lz -L/usr/lib64/pgsql -g
cp -rf ipbx_register /usr/local/ipbx/

gcc -I/usr/include/postgresql/ -I/usr/include/pgsql ipbx_queues.c -o ipbx_queues -lpq -lnsl -lm -lz -L/usr/lib64/pgsql -g
cp -rf ipbx_queues /usr/local/ipbx/

