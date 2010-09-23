#!/bin/bash

np=16
x=100
y=100
hostname=ottavinareale 
i=3

make camera

while [ $i -le $np ]
do
    ./camera hostname 
    i=$[$i+1]
done