
FROM debian:testing-slim

RUN apt-get -qq update
RUN apt-get install -y --no-install-recommends \
      build-essential nodejs npm gyp libcurl4-openssl-dev pkg-config

RUN rm -fr /var/lib/apt /var/cache/apt

ADD . /opt/somhunter

RUN sh -c 'cd /opt/somhunter && rm -fr .git node_modules build logs/* vbs-log/* media Dockerfile'

RUN sh -c 'cd /opt/somhunter && npm install --unsafe-perm'

EXPOSE 8080
CMD sh -c 'cd /opt/somhunter && npm run start'
