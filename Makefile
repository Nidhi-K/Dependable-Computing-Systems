# Periodic Broadcast Protocol

default:
	clear
	g++ -std=c++0x groupMembership.cpp process.cpp message.cpp group.cpp -pthread -o groups.out

run:
	./groups.out 2
