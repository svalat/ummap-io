#!/usr/bin/R

pdf("first-touch-write-time.pdf")

data = read.table("first-touch-write-time.dat")
dataRef = read.table("first-touch-write-time-ref.dat")
dataRefAnon = read.table("first-touch-write-time-ref-anon.dat")

data = subset(data, V1 < 20000)
dataRef = subset(dataRef, V1 < 20000)
dataRefAnon = subset(dataRefAnon, V1 < 20000)

p1 <- hist(data$V1, main="First touch write", xlim=c(0,20000), breaks=seq(0,20000,100))
p2 <- hist(dataRef$V1, main="First touch write", xlim=c(0,20000), breaks=seq(0,20000,100))
p3 <- hist(dataRefAnon$V1, main="First touch write", xlim=c(0,20000), breaks=seq(0,20000,100))

plot( p1, main="First touch write", col=rgb(1,0,0,1/4), border=rgb(1,0,0,1/4),  xlim=c(0,20000), breaks=seq(0,20000,100))
plot( p2, main="First touch write", col=rgb(0,1,0,1/4), border=rgb(0,1,0,1/4),  xlim=c(0,20000), breaks=seq(0,20000,100), add=T)
plot( p3, main="First touch write", col=rgb(0,0,1,1/4), border=rgb(0,0,1,1/4),  xlim=c(0,20000), breaks=seq(0,20000,100), add=T)

dev.off()