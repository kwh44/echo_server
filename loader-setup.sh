#!/bin/bash

cd
apt update
apt upgrade -y
apt install default-jre -y
wget http://apache.cp.if.ua//jmeter/binaries/apache-jmeter-5.1.1.tgz
tar -xzf apache-jmeter-5.1.1.tgz

