#!/bin/bash
adduser --disabled-password --gecos "" user
groupadd docker
usermod -aG docker $USER
