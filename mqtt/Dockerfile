FROM node:carbon-alpine

RUN mkdir -p /app

RUN npm install mqtt

ENTRYPOINT ["node"]

CMD ["/app/app.js"]