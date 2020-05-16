
# iothome

#### Setup

1. create particle env vars per `docker-compose.yml`, change any user/passwords
1. `docker-compose run --rm <service> npm install` for each node based service (mqtt, particle, tempest)
1. `docker-compose up`
1. `curl -XPOST 'http://localhost:8086/query' --data-urlencode 'q=CREATE DATABASE "main"'`
1. http://localhost:3000 `admin`:`password` (specified in docker-compose.yml)
1. configure influxdb data source, host will be `influxdb`
1. create Dashboard
1. create Charts

#### Normal Operation

* `docker-compose up`
* http://localhost:3000
