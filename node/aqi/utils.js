const http = require('http')

// send over to influxdb
// curl -i -XPOST "http://localhost:8086/write?db=main" --data-binary 'weather,deviceid=170032000747343339373536 tempf=67.10 1548533672746000000000'

const isNum = (n) => typeof n === "number";

const linear = (AQIhigh, AQIlow, Conchigh, Conclow, Concentration) => {
  const a = ((parseFloat(Concentration) - Conclow) / (Conchigh - Conclow)) * (AQIhigh - AQIlow) + AQIlow;
  return Math.round(a);
}

const pm25toAQI = (pm25) => {
  let AQI;
  let c = (Math.floor(10 * parseFloat(pm25))) / 10;
  if (c >= 0 && c < 12.1) {
    AQI = linear(50, 0, 12, 0, c);
  } else if (c >= 12.1 && c < 35.5) {
    AQI = linear(100, 51, 35.4, 12.1, c);
  } else if (c >= 35.5 && c < 55.5) {
    AQI = linear(150, 101, 55.4, 35.5, c);
  } else if (c >= 55.5 && c < 150.5) {
    AQI = linear(200, 151, 150.4, 55.5, c);
  } else if (c >= 150.5 && c < 250.5) {
    AQI = linear(300, 201, 250.4, 150.5, c);
  } else if (c >= 250.5 && c < 350.5) {
    AQI = linear(400, 301, 350.4, 250.5, c);
  } else if (c >= 350.5 && c < 500.5) {
    AQI = linear(500, 401, 500.4, 350.5, c);
  } else {
    AQI = linear(1000, 501, 1000.4, 500.5, c);
  }
  return AQI;
}

const avg = (nums) => {
  return nums.reduce((a, b) => (a + b)) / nums.length
}

function logEvent(measurement, tags, fields, unix_time) {
  const unixTimeNs = `${new Date(unix_time * 1000).getTime()}000000`
  const tagStr = tags.map(arr => arr.join('=')).join(',')
  const fieldStr = fields.map(arr => arr.join('=')).join(',')

  // data string ends up as line protocol: https://v2.docs.influxdata.com/v2.0/reference/syntax/line-protocol/
  // measurement, tags fields time
  // weather,deviceid=001 tempf=50 <largenumber>
  const line = `${measurement},${tagStr} ${fieldStr} ${unixTimeNs}`

  console.log('writing', line)

  const req = http.request({
    hostname: 'influxdb',
    port: 8086,
    path: '/write?db=main',
    method: 'POST',
  }, (res) => {
    res.setEncoding('utf8')
    res.on('data', (body) => {
      console.log(body)
    })
  })

  req.on('error', (e) => {
    console.log(`problem with request: ${e}`)
  })

  // write data to request body
  req.write(line)
  req.end()
}

module.exports = { logEvent, isNum, pm25toAQI, avg }
