# vim: noexpandtab

SRCDIR = src
OUTDIR = mastercopy
PROCESSOR = $(shell readlink -f ../_generator/asciidoctor_wrapper.rb)
STYLESHEET = $(shell readlink -f ../_generator/lepota.css)

all: build

.PHONY: build
build: $(OUTDIR)/index.html

$(OUTDIR)/%.html: $(SRCDIR)/%.txt $(STYLESHEET) $(PROCESSOR)
	TZ=UTC $(PROCESSOR) -a stylesheet=$(STYLESHEET) -o $@ $<

.PHONY: clean
clean:
	test -d $(OUTDIR) && rm -Rf $(OUTDIR)/*

