#!/bin/sh

LATEX=pdflatex
FILE=libfuzz

if [ ! -z "$1" ]; then
	FILE=$1
fi

rm -f *.aux
$LATEX $FILE.tex
bibtex $FILE
$LATEX $FILE.tex
$LATEX $FILE.tex

