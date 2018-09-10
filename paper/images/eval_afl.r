
setwd("/Users/huangant/Dropbox/working/doc_libfuzz.ep/images");

setEPS();
postscript(file="eval_afl.eps", paper="special", width=4, height=2, fonts=c("sans", "mono"));

#pdf(file="eval_afl.pdf", paper="special", width=4, height=2, fonts=c("sans", "mono"));

dat = matrix(c(1.22, 2.75, 1.50, 1.55, 0, 5.83), nr=2);
colors = c(rgb(0.2, 0.2, 0.2), rgb(0.7, 0.7, 0.7));

back_mar = par(mar = c(3, 4, 1, 1)+0.1)	# bottom, left, top, right
t = barplot(dat, beside=TRUE, col=colors,
	ylab="Overhead", ylim=c(0, max(dat)*1.2),
	names.arg = c("CVE-2016-6180", "CVE-2015-8317", "CVE-2014-0160"),
	cex.names=0.6);
text(t, dat+max(dat)/10, c(1.22, 2.75, 1.50, 1.55, "N/A", 5.83), cex=0.6)
legend("topleft", c("AFL QEMU mode", "libFuzzer-bin"), col=colors, pch=15, bty="n", cex=0.7)
par(mar = back_mar);

dev.off();

