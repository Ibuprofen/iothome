const fetch = require('node-fetch')
const { logEvent, avg, isNum, pm25toAQI } = require('./utils.js')

process.on('SIGINT', () => {
  console.log( "\nGracefully shutting down from SIGINT (Ctrl-C)" )
  process.exit(1)
})

const delay = 60000; // milliseconds
const deviceId = process.env.DEVICE_ID
const url = `https://www.purpleair.com/json?show=${deviceId}`

const interval = async () => {

  try {
    const response = await fetch(url)
    const data = await response.json()

    if (data.results.length) {
      const pm25Values = data.results.map(d => Number(d.PM2_5Value))
      const pm25 = avg(pm25Values)
      const aqi = pm25toAQI(pm25)

      const unixTime = Math.floor(+new Date() / 1000)

      if (isNum(pm25)) {

        logEvent('weather', [
          ['deviceid', `purpleair${deviceId}`],
          ['location', 'outdoor'],
        ], [
          ['pm25', pm25],
          ['aqi', aqi],
        ], unixTime);

      } else {
        console.log('no pm25?', pm25)
      }
    } else {
      console.log(`no results from ${url}`)
    }

  } catch (e) {
    console.log(e)
  }
};

interval()
setInterval(interval, delay)
