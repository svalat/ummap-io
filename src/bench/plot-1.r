#!/usr/bin/R

pdf("first-touch-read-time.pdf")

data = read.table("first-touch-read-time.dat")
dataRef = read.table("first-touch-read-time-ref.dat")
dataRefAnon = read.table("first-touch-read-time-ref-anon.dat")

p1 <- hist(data$V1, main="First touch read", xlim=c(0,15000), breaks=10000)
p2 <- hist(dataRef$V1, main="First touch read", xlim=c(0,15000), breaks=10000)
p3 <- hist(dataRefAnon$V1, main="First touch read", xlim=c(0,15000), breaks=10000)

plot( p1, main="First touch read", col=rgb(1,0,0,1/4), border=rgb(1,0,0,1/4),  xlim=c(0,15000))
plot( p2, main="First touch read", col=rgb(0,1,0,1/4), border=rgb(0,1,0,1/4),  xlim=c(0,15000), add=T)
plot( p3, main="First touch read", col=rgb(0,0,1,1/4), border=rgb(0,0,1,1/4),  xlim=c(0,15000), add=T)

dev.off()
