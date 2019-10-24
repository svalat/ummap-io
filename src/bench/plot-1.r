#!/usr/bin/R

pdf("first-touch-read-time.pdf")

data = read.table("first-touch-read-time.dat")
dataRef = read.table("first-touch-read-time-ref.dat")
dataRefAnon = read.table("first-touch-read-time-ref-anon.dat")

data = subset(data, V1 < 30000)
dataRef = subset(dataRef, V1 < 30000)
dataRefAnon = subset(dataRefAnon, V1 < 30000)

p1 <- hist(data$V1, main="First touch read", xlim=c(0,30000), breaks=seq(0,30000,50))
p2 <- hist(dataRef$V1, main="First touch read", xlim=c(0,30000), breaks=seq(0,30000,50))
p3 <- hist(dataRefAnon$V1, main="First touch read", xlim=c(0,30000), breaks=seq(0,30000,50))

plot( p1, main="First touch read", col=rgb(1,0,0,1/4), border=rgb(1,0,0,1/4),  xlim=c(0,30000), breaks=seq(0,30000,50))
plot( p2, main="First touch read", col=rgb(0,1,0,1/4), border=rgb(0,1,0,1/4),  xlim=c(0,30000), breaks=seq(0,30000,50), add=T)
plot( p3, main="First touch read", col=rgb(0,0,1,1/4), border=rgb(0,0,1,1/4),  xlim=c(0,30000), breaks=seq(0,30000,50), add=T)

dev.off()
