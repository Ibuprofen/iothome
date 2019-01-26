FROM node:carbon-alpine

RUN mkdir -p /app

RUN npm install particle-api-js

ENTRYPOINT ["node"]

CMD ["/app/app.js"]