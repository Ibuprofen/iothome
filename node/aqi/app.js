import fetch from 'node-fetch'
import { logEvent, avg, isNum, pm25toAQI } from './utils.js'

process.on('SIGINT', () => {
  console.log( "\nGracefully shutting down from SIGINT (Ctrl-C)" )
  process.exit(1)
})

const delay = 60000; // milliseconds
const sensorIndex = process.env.PURPLE_AIR_SENSOR_INDEX
const apiKey = process.env.PURPLE_AIR_API_KEY
const url = `https://api.purpleair.com/v1/sensors/${sensorIndex}?api_key=${apiKey}`

const interval = async () => {

  try {
    const response = await fetch(url)

    console.log(response.status, response.statusText);

    const data = await response.json()

    if (data?.sensor) {
      const pm25_a = data?.sensor?.['pm2.5_a'] || 0
      const pm25_b = data?.sensor?.['pm2.5_b'] || 0
      const pm25 = avg([pm25_a, pm25_b])
      const aqi = pm25toAQI(pm25)

      const unixTime = Math.floor(+new Date() / 1000)

      if (isNum(pm25)) {

        logEvent('weather', [
          ['deviceid', `purpleair${sensorIndex}`],
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
