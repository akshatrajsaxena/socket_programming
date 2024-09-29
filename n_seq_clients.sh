#!/bin/bash
count=$1
for i in $(seq $count); do
    $2
done