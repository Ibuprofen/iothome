const http = require('http')

// send over to influxdb
// curl -i -XPOST "http://localhost:8086/write?db=main" --data-binary 'weather,deviceid=170032000747343339373536 tempf=67.10 1548533672746000000000'

function logEvent(str) {

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
    console.log('writing', str)
    req.write(str)
    req.end()
}

module.exports = { logEvent }
