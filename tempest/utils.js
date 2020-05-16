const http = require('http')

// send over to influxdb
// curl -i -XPOST "http://localhost:8086/write?db=main" --data-binary 'weather,deviceid=170032000747343339373536 tempf=67.10 1548533672746000000000'

function cToF(celsius, digits = 2) {
  if (typeof celsius === "number") {
    const result = celsius * 1.8 + 32;
    return Number.parseFloat(result).toFixed(digits);
  }
}

function logEvent(measurement, tags, fields, unix_time) {
  const unixTimeNs = `${new Date(unix_time * 1000).getTime()}000000`
  const tagStr = tags.map(arr => arr.join('=')).join(',')
  const fieldStr = fields.map(arr => arr.join('=')).join(',')

  // data string ends up as line protocol: https://v2.docs.influxdata.com/v2.0/reference/syntax/line-protocol/
  // measurement, tags fields time
  // weather,deviceid=001 tempf=50 <largenumber>
  const line = `${measurement},${tagStr} ${fieldStr} ${unixTimeNs}`

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
  console.log('writing', line)
  req.write(line)
  req.end()
}

module.exports = { logEvent, cToF }
