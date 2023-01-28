# SRNE inverter data collector

sniffing/reading data form SRNE solar hybrid inverter charger and storing them in Influx with Grafana

## Parts

The repo contain code for read rs485 packets and send it to nodejs server which puts it in InfluxDB.
Reading rs485 packets is done on `esp32` SoC.


#### Telegraf (if you need it)
https://github.com/influxdata/telegraf
