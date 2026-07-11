#!/bin/bash

set -e

exec pio run -e "${PIO_ENV:-tdeck-plus-core3}" "$@"
