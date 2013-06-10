#!/bin/bash

./compiler in.pl0 0 > t.o;./vm t.o;rm t.o
