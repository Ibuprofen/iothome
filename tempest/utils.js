const http = require('http')

// send over to influxdb
// curl -i -XPOST "http://localhost:8086/write?db=main" --data-binary 'weather,deviceid=170032000747343339373536 tempf=67.10 1548533672746000000000'

const isNum = (n) => typeof n === "number";
const parseFixed = (n, digits = 2) => Number.parseFloat(n).toFixed(digits);
const calc = (n) => isNum(n) ? parseFixed(n) : NaN;

const cToF = celcius => calc(celcius * 1.8 + 32);
const mbarToHg = mbar => calc(mbar / 33.8639);
const kmToMiles = km => calc(km * 0.62137);
const mmToIn = mm => calc(mm / 25.4);

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

module.exports = { logEvent, cToF, mbarToHg, kmToMiles, mmToIn }
