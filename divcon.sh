#!/bin/bash

ladrun -np 3 bin/divcon 10000
ladrun -np 3 bin/divcon 100000
ladrun -np 3 bin/divcon 1000000

ladrun -np 7 bin/divcon 10000
ladrun -np 7 bin/divcon 100000
ladrun -np 7 bin/divcon 1000000

ladrun -np 15 bin/divcon 10000
ladrun -np 15 bin/divcon 100000
ladrun -np 15 bin/divcon 1000000

ladrun -np 31 bin/divcon 10000
ladrun -np 31 bin/divcon 100000
ladrun -np 31 bin/divcon 1000000
