#!/usr/bin/dash

export _JAVA_OPTIONS="-Xms512m -Xmx4096m"
./../apache-jmeter-5.1.1/bin/jmeter -n -t test_plan.jmx -l results.txt