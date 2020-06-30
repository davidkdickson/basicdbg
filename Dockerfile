FROM ubuntu:20.04
RUN apt-get update && apt-get install build-essential -y
COPY . /usr/src/basicdbg
WORKDIR /usr/src/basicdbg
RUN make
