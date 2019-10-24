#!/usr/bin/R

pdf("map-time.pdf")

data = read.table("map-time.dat")
dataRef = read.table("map-time-ref.dat")
dataRefAnon = read.table("map-time-ref-anon.dat")

data = subset(data, V1 < 10000000)
dataRef = subset(dataRef, V1 < 10000000)
dataRefAnon = subset(dataRefAnon, V1 < 10000000)

p1 <- hist(data$V1, main="Map time", xlim=c(0,10000000), breaks=seq(0,10000000,10000))
p2 <- hist(dataRef$V1, main="Map time", xlim=c(0,10000000), breaks=seq(0,10000000,10000))
p3 <- hist(dataRefAnon$V1, main="Map time", xlim=c(0,10000000), breaks=seq(0,10000000,10000))

plot( p1, main="Map time", col=rgb(1,0,0,1/4), border=rgb(1,0,0,1/4),  xlim=c(0,10000000), breaks=seq(0,10000000,10000))
plot( p2, main="Map time", col=rgb(0,1,0,1/4), border=rgb(0,1,0,1/4),  xlim=c(0,10000000), breaks=seq(0,10000000,10000), add=T)
plot( p3, main="Map time", col=rgb(0,0,1,1/4), border=rgb(0,0,1,1/4),  xlim=c(0,10000000), breaks=seq(0,10000000,10000), add=T)

dev.off()