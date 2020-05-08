FROM i386/ubuntu:18.04
RUN apt-get update && apt-get install -y build-essential

WORKDIR /usr/src/plugin
COPY hl2sdk ./hl2sdk
COPY Makefile Makefile.plugin libgcc_s.so.1 *.cpp *.h ./

RUN make plugin
