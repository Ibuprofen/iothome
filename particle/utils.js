const http = require('http')

// send over to influxdb
// curl -i -XPOST "http://localhost:8086/write?db=main" --data-binary 'weather,deviceid=170032000747343339373536 tempf=67.10 1548533672746000000000'

function logEvent(obj) {

    const unixTimeNs = `${new Date(obj.published_at).getTime()}000000`
    const data = `weather,deviceid=${obj.coreid} tempf=${obj.data} ${unixTimeNs}`

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
    console.log('writing', data)
    req.write(data)
    req.end()
}

module.exports = { logEvent }
