#!/bin/sh

# Run make
make

# Check if both executables were built successfully
if [ -x ./get_info_for_pid ] && [ -x ./get_info ]; then
    if [ -n "$1" ]; then 
    	PID=$1
    	echo "Running get_info_for_pid $PID:"
    	./get_info_for_pid $PID
    	echo
    else 
    	echo "No PID provided, skipping get_info_for_pid."
    	echo
    fi
    
    echo "Running get_info:"
    ./get_info
else
    echo "Build failed — executables not found."
    exit 1
fi
