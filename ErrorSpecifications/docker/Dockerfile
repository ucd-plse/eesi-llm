FROM ubuntu:20.04

LABEL maintainer="pchapman@ucdavis.edu"
LABEL description="Docker image for running interleaved analysis"

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update

RUN apt-get install -y gcc-9 g++-9

RUN ln -s /usr/bin/gcc-9 /usr/bin/gcc
RUN ln -s /usr/bin/g++-9 /usr/bin/g++

RUN apt-get install -y apt-transport-https gnupg curl git wget

RUN curl -fsSL https://www.mongodb.org/static/pgp/server-8.0.asc | \
   gpg -o /usr/share/keyrings/mongodb-server-8.0.gpg \
   --dearmor
#RUN wget -c http://mirrors.kernel.org/ubuntu/pool/main/libf/libffi/libffi6_3.2.1-9_amd64.deb http://security.ubuntu.com/ubuntu/pool/main/p/python3.7/libpython3.7-stdlib_3.7.5-2~19.10ubuntu1_amd64.deb http://security.ubuntu.com/ubuntu/pool/main/p/python3.7/libpython3.7-minimal_3.7.5-2~19.10ubuntu1_amd64.deb http://security.ubuntu.com/ubuntu/pool/main/p/python3.7/python3.7-minimal_3.7.5-2~19.10ubuntu1_amd64.deb http://security.ubuntu.com/ubuntu/pool/main/p/python3.7/python3.7_3.7.5-2~19.10ubuntu1_amd64.deb
#RUN apt install ./libffi6_3.2.1-9_amd64.deb ./libpython3.7-minimal_3.7.5-2~19.10ubuntu1_amd64.deb ./libpython3.7-stdlib_3.7.5-2~19.10ubuntu1_amd64.deb ./python3.7-minimal_3.7.5-2~19.10ubuntu1_amd64.deb ./python3.7_3.7.5-2~19.10ubuntu1_amd64.deb


RUN apt install -y software-properties-common
RUN add-apt-repository ppa:deadsnakes/ppa
RUN apt update
RUN apt install -y python3.9 python3.9-dev
RUN apt install -y python2.7 python-dev

RUN rm /usr/bin/python3
RUN ln -s /usr/bin/python3.9 /usr/bin/python3

RUN apt install -y python3-pip

#RUN echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-8.0.gpg ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/8.0 multiverse" | tee /etc/apt/sources.list.d/mongodb-org-8.0.list
RUN echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-8.0.gpg ] https://repo.mongodb.org/apt/ubuntu focal/mongodb-org/8.0 multiverse" | tee /etc/apt/sources.list.d/mongodb-org-8.0.list
RUN curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg
RUN mv bazel-archive-keyring.gpg /usr/share/keyrings
RUN echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list

RUN apt-get update

RUN apt-get install -y mongodb-org bazel-4.1.0 tmux build-essential zlib1g-dev libssl-dev libbz2-dev google-perftools libboost-all-dev libffi-dev

RUN ln -s /usr/bin/bazel-4.1.0 /usr/bin/bazel
RUN ln -s /usr/bin/python3 /usr/bin/python

RUN mkdir -p /home/evaluation-container
COPY bazel_python.tar.gz /home/evaluation-container/bazel_python.tar.gz
RUN cd /home/evaluation-container/ &&  tar -xf ./bazel_python.tar.gz
RUN cd /home/evaluation-container/bazel_python && ./setup_python.sh 3.7.4 /home/evaluation-container/.bazel_python

RUN mkdir -p /data/db

env HOME /home/evaluation-container

COPY ./requirements.txt /requirements.txt
RUN apt install -y python3.9-distutils
RUN python3 -m pip install -r /requirements.txt
