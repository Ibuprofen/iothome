const mqtt = require('mqtt')
const client  = mqtt.connect('mqtt://mosquitto')
const { logEvent } = require('./utils.js')

process.on('SIGINT', () => {
  console.log( "\nGracefully shutting down from SIGINT (Ctrl-C)" )
  process.exit(1)
})

client.on('connect', () => {
  console.log('connected')
  client.subscribe('event', (err) => {
    if (err) {
      console.log('error', err)
    }
  })
})

client.on('message', (topic, message) => {
  // message is Buffer
  console.log('message!', message.toString())
  //client.end()
  //logEvent(message.toString())
  //console.log(topic, message)
  if (topic === 'event') {
    const msg = message.toString()
    logEvent(msg)
  }
})
