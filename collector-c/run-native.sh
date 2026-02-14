#!/bin/bash

# This script builds and runs the collector natively on macOS.
# It passes all arguments to the collector.
#
# Usage:
#   ./run-native.sh [collector arguments]
#
# Examples:
#   ./run-native.sh -u http://my-api.com -i 15
#   ./run-native.sh -l /var/log/collector.log -d  (to run as a daemon)

# Exit on error
set -e

# Navigate to the script's directory to ensure we can find the Makefile
cd "$(dirname "$0")"

echo "Building the collector..."
make

echo "Collector built successfully."
echo "Starting the collector..."
./collector -u http://localhost:8000 -P -A WARNING "$@"

echo "Collector started."
