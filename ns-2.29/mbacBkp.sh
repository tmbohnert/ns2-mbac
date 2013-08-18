#!/bin/sh
# mbac

echo "Backup all MBAC related files"
tar -czf ns-mbac-ssm-skde-fhb-oqa-$(date +%e%m%y).tar.gz $(grep -Rl mbac *[!diffserv]) diffserv
echo "Done!"

#EOF
