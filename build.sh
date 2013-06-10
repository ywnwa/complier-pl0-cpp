#!/bin/bash

gcc -g -o compiler src/*.c src/lib/*.c test/lib/*.c -I.
