const http = require('http')
const Particle = require('particle-api-js')
const particle = new Particle()
const { logEvent } = require('./utils.js')

process.on('SIGINT', () => {
  console.log( "\nGracefully shutting down from SIGINT (Ctrl-C)" )
  process.exit(1)
})

particle.login({
  username: process.env.PARTICLE_USERNAME, password: process.env.PARTICLE_PASSWORD
}).then((result) => {
  console.log('Your access token', result.body.access_token)

  const token = result.body.access_token

  particle.getEventStream({
    deviceId: 'mine',
    auth: token
  }).then((stream) => {
    stream.on('end', () => {
      console.log('end')
    })
    stream.on('event', (res) => {
      console.log('event')
      if (res.data) {
        console.log(res)

        // TODO: change shape of incoming object
        if (res.name && res.name === 'dsTmp') {
          logEvent(res)
        }
      }
    })
    stream.on('error', (err) => {
      console.error(err)
    })
  }, (err) => {
    console.error(err)
  })

}, (err) => {
  console.error(err)
})
